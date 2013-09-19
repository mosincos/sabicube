#include "game.h"

namespace camera
{
    struct action
    {
        int startmillis;
        int duration;
        const uint *successors;

        action(int d = 0, const char *s = NULL) : startmillis(lastmillis), duration(d), successors(s && *s ? compilecode(s) : NULL) {}
        virtual ~action() {delete[] successors;}

        float multiplier(bool total = false)
        {
            if(!duration)
                return 1;

            if(total)
                return min<float>(1, (lastmillis - startmillis) / (float)duration);
            else
            {
                int elapsed = curtime;
                if(startmillis + duration - lastmillis < curtime)
                    elapsed = startmillis + duration - lastmillis;
                return (float) elapsed / duration;

            }
        }
        virtual bool update()
        {
            if(startmillis + duration <= lastmillis)
            {
                if(successors)
                    execute(successors);
                return false;
            }
            return true;
        }
        virtual void render(int w, int h) {}
        virtual void debug(int w, int h, int &hoffset, bool type = false)
        {
            if(type)
            {
                draw_text("Type: action/delay", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Duration: %i (%f%%)", 100, 0 + hoffset, duration, (lastmillis - startmillis) / (float)duration);
            draw_textf("Children: %p", 100, 64 + hoffset, successors);

            hoffset += 64 * 3;
        }
    };

    vector <action *> pending;
    int suboffset = 0;
    vector <action *> subtitles; //subtitles are tracked independantly, and rendered in reverse, always on top
    physent camera;
    physent *attached = NULL;
    bool cutscene = false;
    bool cancelled = false;

    void cleanup()
    {
        cutscene = false;
        pending.deletecontents();
        subtitles.deletecontents();
    }

    void update()
    {
        if((subtitles.length() || pending.length()) && !cutscene) {cutscene = true;}
        if(cutscene)
        {
            attached = NULL;
            loopv(pending)
            {
                if(!pending[i]->update())
                {
                    delete pending[i];
                    pending.remove(i);
                    i--;
                }
            }
            loopv(subtitles)
            {
                if(!subtitles[i]->update())
                {
                    delete subtitles[i];
                    subtitles.remove(i);
                    i--;
                }
            }
            if(!pending.length() && !subtitles.length())
                cutscene = false;
        }
    }

    VARP(dbgcamera, 0, 0, 1);

    void render(int w, int h)
    {
        loopv(pending)
            pending[i]->render(w, h);
        suboffset = 0;
        loopvrev(subtitles)
            subtitles[i]->render(w, h);

        glPushMatrix();
        glScalef(.3, .3, 1);

        int offset = 128;
        if(dbgcamera)
        {
            loopv(pending)
                pending[i]->debug(w, h, offset, true);
            loopv(subtitles)
                subtitles[i]->debug(w, h, offset, true);
        }
        glPopMatrix();
    }

    struct cond : action
    {
        const uint *test;

        cond(const char *t, const char *s) : action(0, s), test(t && *t ? compilecode(t) : NULL) {}
        ~cond() { delete[] test;}

        bool update()
        {
            if(!test || execute(test))
            {
                if(successors) execute(successors);
                return false;
            }

            return true;
        }

        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: Test", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Test: %p", 100, 0 + hoffset, test);
            draw_textf("Children: %p", 100, 64 + hoffset, successors);
            hoffset += 192;
        }
    };

    struct iterate : cond
    {
        int limit;
        int i;

        iterate(const char *t, int l, int d, const char *s) : cond(t, s), limit(l), i(0) { duration = d;}
        ~iterate() {}

        bool update()
        {
            if(lastmillis - startmillis >= duration)
            {
                if(limit)
                    i++;

                bool res = cond::update();

                if(res || (limit && i == limit))
                    return false;

                startmillis += duration;
            }

            return true;
        }

        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: Loop", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Test: %p", 100, 0 + hoffset, test);
//            draw_textf("Test: %p", 100, 0 + hoffset);
            draw_textf("Loop: %i/%i", 100, 64 + hoffset, i, limit);
            hoffset += 128;

            action::debug(w, h, hoffset);
        }
    };

    struct subtitle : action
    {
        const char *text;
        vec color;

        subtitle(const char *t, float r, float g, float b, int d, const char *s) : action(d, s), text(t && *t ? newstring(t) : newstring("No Text")), color(vec(r, g, b)) {}
        ~subtitle() {delete[] text;}

        void render(int w, int h)
        {
            float alpha = 1;
            if(duration > 500)
            {
                if(startmillis + 100 > lastmillis)
                    alpha = 1 - (startmillis + 100 - lastmillis) / 100.0f;
                else if(startmillis - 100 + duration < lastmillis)
                    alpha = (startmillis + duration - lastmillis) / 100.0f;
            }
            int th, tw;
            text_bounds(text, tw, th, w * 0.9);

            draw_text(text, w / 2 - tw / 2, h * 0.85 - th - suboffset, color.x * 255, color.y * 255, color.z * 255, alpha * 255, -1, w * 0.9);
            suboffset += th * alpha;
        }
        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: subtitle", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Colour: (%f, %f, %f)", 100, 0 + hoffset, color.x, color.y, color.z);
            hoffset += 64;
            action::debug(w, h, hoffset);
        }
    };

    struct move : action
    {
        float dx, dy, dz;
        vec start;

        move(float x, float y, float z, int d, const char *s) : action(d, s), dx(x), dy(y), dz(z), start(camera.o) {}
        move(vec dest, int d, const char *s) : action(d, s), start(camera.o)
        {
            dest.sub(start);
            dx = dest.x;
            dy = dest.y;
            dz = dest.z;
        }
        ~move() {}

        bool update()
        {
            camera.o = vec(dx, dy, dz).mul(multiplier(true)).add(start);

            return action::update();
        }
        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: Move", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Start: (%f, %f, %f)", 100, 0 + hoffset, start.x, start.y, start.z);
            draw_textf("Delta: (%f, %f, %f)", 100, 64 + hoffset, dx, dy, dz);
            hoffset += 64 * 2;
            action::debug(w, h, hoffset);
        }
    };

    struct moveaccel : move
    {
        moveaccel(float x, float y, float z, int d, const char *s) : move(x, y, z, d, s) {}
        moveaccel(vec dest, int d, const char *s) : move(dest, d, s) {}
        ~moveaccel() {}

        bool update()
        {
            if(!duration)
                return move::update();

            int elapsed = min(duration, lastmillis - startmillis);
            float mult = 0.5f + sinf((elapsed - duration/2.0f) * M_PI / duration) / 2.0f;

            camera.o = vec(dx, dy, dz).mul(mult).add(start);

            return action::update();
        }
        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: Move Accel", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Progress: %f", 100, 0 + hoffset, 0.5f + sinf((min(duration, lastmillis - startmillis) - duration / 2.0f) * M_PI / duration));
            hoffset += 64;
            move::debug(w, h, hoffset, false);
        }
    };

    struct view : action
    {
        float yaw, pitch, roll;
        float iyaw, ipitch, iroll;

        view(float y, float p, float r, int d, const char *s) : action(d, s), yaw(y), pitch(p), roll(r), iyaw(camera.yaw), ipitch(camera.pitch), iroll(camera.roll)
        {
            if(duration && (yaw - iyaw == 180 || yaw - iyaw == -180))
            {
                conoutf("Warning: viewspecific/viewcamera was called from two points with a 180 degree delta. iyaw was adjusted slightly to avoid funkiness and possible division by 0");
                iyaw += 1;
            }
        }
        ~view() {}

        bool update()
        {
            vec old = vec(iyaw * RAD, ipitch * RAD).mul(1-multiplier(true));
            vec next = vec(yaw * RAD, pitch * RAD).mul(multiplier(true)).add(old);

            vectoyawpitch(next, camera.yaw, camera.pitch);
            camera.roll = iroll * (1 - multiplier(true)) + roll * multiplier(true);

            return action::update();
        }
        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: View", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Initial: %f %f %f", 100, 0 + hoffset, iyaw, ipitch, iroll);
            draw_textf("Post: %f %f %f", 100, 64 + hoffset, yaw, pitch, roll);
            hoffset += 64 * 2;
            action::debug(w, h, hoffset);
        }
    };

    struct viewaccel : view
    {
        viewaccel(float y, float p, float r, int d, const char *s) : view(y, p, r, d, s) {}
        ~viewaccel() {}

        bool update()
        {
            if(!duration)
                return view::update();

            int elapsed = min(duration, lastmillis - startmillis);
            float mult = 0.5f + sinf((elapsed - duration/2.0f) * M_PI / duration) / 2.0f;

            vec old = vec(iyaw * RAD, ipitch * RAD).mul(1-mult);
            vec next = vec(yaw * RAD, pitch * RAD).mul(mult).add(old);

            vectoyawpitch(next, camera.yaw, camera.pitch);
            camera.roll = iroll * (1 - mult) + roll * mult;

            return action::update();
        }
        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: View Accel", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Progress: %f", 100, 0 + hoffset, 0.5f + sinf((min(duration, lastmillis - startmillis) - duration / 2.0f) * M_PI / duration));
            hoffset += 64;
            view::debug(w, h, hoffset, false);
        }
    };

    struct viewspin : action
    {
        float yaw, pitch, roll;

        viewspin(float y, float p, float r, int d, const char *s) : action(d, s), yaw(y), pitch(p), roll(r) {}
        ~viewspin() {}

        bool update()
        {
            float mult = multiplier();
            camera.yaw += yaw * mult;
            camera.pitch += pitch * mult;
            camera.roll += roll * mult;

            return action::update();
        }
        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: View Spin", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Delta: %f %f %f", 100, 0 + hoffset, yaw, pitch, roll);
            hoffset += 64;
            action::debug(w, h, hoffset);
        }
    };

    struct sound : action
    {
        int ind;

        sound(int i, int d, const char *s) : action(d, s), ind(i)
        {
            playsound(ind, NULL, NULL, 0, 0, -1, 0, d);
        }
        ~sound() {}

        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: Sound", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Index: %i", 100, 0 + hoffset, ind);
            hoffset += 64;
            action::debug(w, h, hoffset);
        }
    };

    struct soundfile : action
    {
        const char *path;

        soundfile(const char *p, int d, const char *s) : action(d, s), path(newstring(p))
        {
            playsoundname(path, NULL, 0, 0, 0, -1, 0, d);
        }
        ~soundfile() {delete[] path;}

        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: Sound File", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Path: %s", 100, 0 + hoffset, path);
            hoffset += 64;
            action::debug(w, h, hoffset);
        }
    };

    struct overlay : action
    {
        Texture *img;
        vec4 color;
        int fade;

        overlay(Texture *i, float r, float g, float b, float a, int f, int d, const char *s) : action(d, s), img(i), color(vec4(r, g, b, a)), fade(f) {}
        ~overlay() {}

        void render(int w, int h)
        {
            float alpha = 1;
            if(fade && duration >= fade * 2)
            {
                if(startmillis + fade > lastmillis)
                    alpha = 1 - (startmillis + 150 - lastmillis) / (float) fade;
                else if(startmillis -fade + duration < lastmillis)
                    alpha = (startmillis + duration - lastmillis) / (float) fade;
            }

            glColor4f(color.x, color.y, color.z, color.w * alpha);
            setcamtexture(img);

            glBegin(GL_TRIANGLE_STRIP);

            glTexCoord2f(0, 0); glVertex2i(0, 0);
            glTexCoord2f(1, 0); glVertex2i(w, 0);
            glTexCoord2f(0, 1); glVertex2i(0, h);
            glTexCoord2f(1, 1); glVertex2i(w, h);

            glEnd();

            glColor3f(1, 1, 1);
        }
        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: Overlay", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Color: (%f, %f, %f, %f)", 100, 0 + hoffset, color.x, color.y, color.z, color.w);
            draw_textf("Fade: %i", 100, 64 + hoffset, fade);
            hoffset += 64 * 2;
            action::debug(w, h, hoffset);
        }
    };

    struct solid : action
    {
        bool modulate;
        vec4 color;
        int fade;

        solid(bool m, float r, float g, float b, float a, int f, int d, const char *s) : action(d, s), modulate(m), color(vec4(r, g, b, a)), fade(f) {}

        void render(int w, int h)
        {
            float alpha = 1;
            if(fade && duration >= fade * 2)
            {
                if(startmillis + fade > lastmillis)
                    alpha = 1 - (startmillis + fade - lastmillis) / (float) fade;
                else if(startmillis -fade + duration < lastmillis)
                    alpha = (startmillis + duration - lastmillis) / (float) fade;
            }

            if(modulate)
            {
                glBlendFunc(GL_ZERO, GL_SRC_COLOR);
                vec col = vec(color).mul(alpha).add(vec(1, 1, 1).mul(1-alpha));
                glColor3fv(col.v);
            }
            else
                glColor4f(color.x, color.y, color.z, color.w * alpha);

            glDisable(GL_TEXTURE_2D);
            setnotextureshader();
            glBegin(GL_TRIANGLE_STRIP);

            glVertex2i(0, 0);
            glVertex2i(w, 0);
            glVertex2i(0, h);
            glVertex2i(w, h);

            glEnd();
            setdefaultshader();
            glEnable(GL_TEXTURE_2D);

            if(modulate)
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor3f(1, 1, 1);
        }

        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: Solid", 100, 0 + hoffset);
                hoffset += 64;
            }

            draw_textf("Modulate: %s", 100, 0 + hoffset, modulate ? "Yes": "No");
            draw_textf("Color: (%f, %f, %f, %f)", 100, 64 + hoffset, color.x, color.y, color.z, color.w);
            draw_textf("Fade: %i", 100, 128 + hoffset, fade);
            hoffset += 64 * 3;
            action::debug(w, h, hoffset);
        }
    };

    struct viewport : action
    {
        int cn;
        float height;
        float tail;

        viewport(int e, float h, float t, int d, const char *s) : action(d, s), cn(e), height(h), tail(t) {}
        ~viewport() {}

        bool update()
        {
            fpsent *ent = game::getclient(cn);
            if(ent)
            {
                camera = *ent;
                camera.o.z += height;
                camera.o.add(vec(camera.yaw * RAD, camera.pitch * RAD).mul(tail));

                vec delta = vec(camera.o).sub(ent->o);
                if(delta.z > ent->aboveeye || delta.z + ent->eyeheight < 0 || (fabs(delta.x) < ent->radius && fabs(delta.y) < ent->radius))
                    attached = ent;
            }

            return action::update();
        }
        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: Viewport", 100, 0 + hoffset);
                hoffset += 64;
            }

            fpsent *ent = game::getclient(cn);

            draw_textf("Entity: %p", 100, 0 + hoffset, ent);
            draw_textf("Height + tail: %f %f", 100, 64 + hoffset, height, tail);
            draw_textf("View: %f %f %f", 100, 128 + hoffset, ent->yaw, ent->pitch, ent->roll);
            draw_textf("Pos: (cam) %f %f %f (ent) %f %f %f", 100, 192 + hoffset, camera.o.x, camera.o.y, camera.o.z, ent->o.x, ent->o.y, ent->o.z);

            hoffset += 256;
            action::debug(w, h, hoffset);
        }
    };

    struct focus : action
    {
        int cn;
        float interp;
        float lead;

        focus(int e, float i, float l, int d, const char *s) : action(d, s), cn(e), interp(i), lead(l) {}
        ~focus() {}

        bool update()
        {
            fpsent *ent = game::getclient(cn);
            if(ent)
            {
                float mult = interp ? min(1.0f, interp * curtime / 1000.0f) : 1;
                vec view = vec(camera.yaw * RAD, camera.pitch * RAD).mul(1 - mult);
                view.add(vec(ent->vel).mul(lead).add(ent->o).sub(camera.o).normalize().mul(mult));

                vectoyawpitch(view, camera.yaw, camera.pitch);
            }

            return action::update();
        }
        void debug(int w, int h, int &hoffset, bool type)
        {
            if(type)
            {
                draw_text("Type: Focus", 100, 0 + hoffset);
                hoffset += 64;
            }

            fpsent *ent = game::getclient(cn);
            vec ideal = ent ? vec(ent->vel).mul(lead).add(ent->o).sub(camera.o) : vec(1, 0, 0);

            float iyaw, ipitch;
            vectoyawpitch(ideal, iyaw, ipitch);

            draw_textf("Interp + Lead: %f %f", 100, 0 + hoffset, interp, lead);
            draw_textf("Current: %f %f", 100, 64 + hoffset, camera.yaw, camera.pitch);
            draw_textf("Ideal: %f %f", 100, 128 + hoffset, iyaw, ipitch);
            hoffset += 192;

            return action::debug(w, h, hoffset);
        }
    };

    #define GETCAM \
        extentity *cam = NULL; \
        loopv(entities::ents) \
        { \
            if(entities::ents[i]->type == CAMERA && entities::ents[i]->attr1 == *tag) \
                cam = entities::ents[i]; \
        }

    #define CAMERACMD(n, f, a, b) \
        ICOMMAND(n, f, a, \
            if(!pending.length() && !subtitles.length()) camera = *game::player1; \
            b; \
        )

    CAMERACMD(cutscene_delay, "is", (int *d, const char *s),
        pending.add(new action(*d, s));
    )
    CAMERACMD(cutscene_cond, "ss", (const char *c, const char *s),
        pending.add(new cond(c, s));
    )
    CAMERACMD(cutscene_loop, "siis", (const char *t, int *l, int *d, const char *s),
        pending.add(new iterate(t, *l, *d, s));
    )

    CAMERACMD(cutscene_subtitle, "sfffis", (const char *text, float *r, float *g, float *b, int *d, const char *s),
        subtitles.add(new subtitle(text, *r, *g, *b, *d, s));
    )
    CAMERACMD(cutscene_movecamera, "iis", (int *tag, int *d, const char *s),
        GETCAM
        if(cam)
            pending.add(new move(cam->o, *d, s));
        else
        {
            conoutf("warning: unable to find a camera entity with tag %i", *tag);
            pending.add(new action(*d, s));
        }
    )
    CAMERACMD(cutscene_movespecific, "fffis", (float *x, float *y, float *z, int *d, const char *s),
        pending.add(new move(*x, *y, *z, *d, s));
    )
    CAMERACMD(cutscene_moveaccelcamera, "iis", (int *tag, int *d, const char *s),
        GETCAM
        if(cam)
            pending.add(new moveaccel(cam->o, *d, s));
        else
        {
            conoutf("warning: unable to find a camera entity with tag %i", *tag);
            pending.add(new action(*d, s));
        }
    )
    CAMERACMD(cutscene_moveaccelspecific, "fffis", (float *x, float *y, float *z, int *d, const char *s),
        pending.add(new moveaccel(*x, *y, *z, *d, s));
    )
    CAMERACMD(cutscene_viewcamera, "iis", (int *tag, int *d, const char *s),
        GETCAM
        if(cam)
            pending.add(new view(cam->attr2, cam->attr3, cam->attr4, *d, s));
        else
        {
            conoutf("warning: unable to find a camera entity with tag %i", *tag);
            pending.add(new action(*d, s));
        }
    )
    CAMERACMD(cutscene_viewspecific, "fffis", (float *y, float *p, float *r, int *d, const char *s),
        pending.add(new view(*y, *p, *r, *d, s));
    )
    CAMERACMD(cutscene_viewaccelcamera, "iis", (int *tag, int *d, const char *s),
        GETCAM
        if(cam)
            pending.add(new viewaccel(cam->attr2, cam->attr3, cam->attr4, *d, s));
        else
        {
            conoutf("warning: unable to find a camera entity with tag %i", *tag);
            pending.add(new action(*d, s));
        }
    )
    CAMERACMD(cutscene_viewaccelspecific, "fffis", (float *y, float *p, float *r, int *d, const char *s),
        pending.add(new viewaccel(*y, *p, *r, *d, s));
    )
    CAMERACMD(cutscene_viewspin, "fffis", (float *y, float *p, float *r, int *d, const char *s),
        pending.add(new viewspin(*y, *p, *r, *d, s));
    )
    CAMERACMD(cutscene_sound, "iis", (int *ind, int *d, const char *s),
        pending.add(new sound(*ind, *d, s));
    )
    CAMERACMD(cutscene_soundfile, "sis", (const char *p, int *d, const char *s),
        pending.add(new soundfile(p, *d, s));
    )
    CAMERACMD(cutscene_overlay, "sffffiis", (const char *p, float *r, float *g, float *b, float *a, int *f, int *d, const char *s),
        pending.add(new overlay(textureload(p, 3), *r, *g, *b, *a, *f, *d, s));
    )
    CAMERACMD(cutscene_solid, "iffffiis", (int *m, float *r, float *g, float *b, float *a, int *f, int *d, const char *s),
        pending.add(new solid(*m != 0, *r, *g, *b, *a, *f, *d, s));
    )
    CAMERACMD(cutscene_viewport, "sffis", (const char *ref, float *h, float *t, int *d, const char *s),
        if(*ref) ///THIS DEPENDS ON GAME SPECIFIC STUFF
        {
            int ent = game::parseplayer(ref);
            if(ent >= 0)	pending.add(new viewport(ent, *h, *t, *d, s));
        }
    )
    CAMERACMD(cutscene_focus, "sffis", (const char *ref, float *i, float *l, int *d, const char *s),
        if(*ref) ///THIS DEPENDS ON GAME SPECIFIC STUFF
        {
            int ent = game::parseplayer(ref);
            if(ent >= 0) pending.add(new focus(ent, *i, *l, *d, s));
        }
    )
}

namespace game
{
    bool recomputecamera(physent *&camera1, physent &tempcamera, bool &detachedcamera, float &thirdpersondistance)
    {
        if(camera::cutscene)
        {
            camera1 = &tempcamera;
            tempcamera = camera::camera;
            detachedcamera = true;

            return true;
        }
        return false;
    }
} 
