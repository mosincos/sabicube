#include "game.h"

namespace game
{
    bool intermission = false;
    int maptime = 0, maprealtime = 0, maplimit = -1;
    int respawnent = -1;
    int lasthit = 0, lastspawnattempt = 0;

    int following = -1, followdir = 0;

    fpsent *player1 = NULL;         // our client
    vector<fpsent *> players;       // other clients
    int savedammo[NUMGUNS];

    bool clientoption(const char *arg) { return false; }

    void taunt()
    {
        if(player1->state!=CS_ALIVE || player1->physstate<PHYS_SLOPE) return;
        if(lastmillis-player1->lasttaunt<1000) return;
        player1->lasttaunt = lastmillis;
        addmsg(N_TAUNT, "rc", player1);
    }
    COMMAND(taunt, "");

/////////////////////////////////////////////////////////////////////// exp and lvl
    void setplayerexp(int *arg)
    {
//        if(player1->state!=CS_ALIVE) return; // Do we need this?
		player1->playerexperience = arg[0];
    }
    COMMAND(setplayerexp,  "i");
    void addplayerexp(int *arg)
    {
//        if(player1->state!=CS_ALIVE) return;
		player1->playerexperience = player1->playerexperience + arg[0];
    }
    COMMAND(addplayerexp,  "i");
    ICOMMAND(getplayerexp, "", (), intret(player1->playerexperience));
    void setplayerlvl(int *arg)
    {
//        if(player1->state!=CS_ALIVE) return;
		player1->playerlevel = arg[0];
    }
    COMMAND(setplayerlvl,  "i");
    void addplayerlvl(int *arg)
    {
//        if(player1->state!=CS_ALIVE) return;
		player1->playerlevel = player1->playerlevel + arg[0];
    }
    COMMAND(addplayerlvl,  "i");
    ICOMMAND(getplayerlvl, "", (), intret(player1->playerlevel));

//////////////////////////////////////////////////////////////////////////// Lockpicking
    void setlockpicking(int *arg)
    {
//        if(player1->state!=CS_ALIVE) return; // Do we need this?
		player1->lockpicking = arg[0];
    }
    COMMAND(setlockpicking,  "i");
    void addlockpicking(int *arg)
    {
//        if(player1->state!=CS_ALIVE) return;
		player1->lockpicking = player1->lockpicking + arg[0];
    }
    COMMAND(addlockpicking,  "i");
    ICOMMAND(getlockpicking, "", (), intret(player1->lockpicking));
	
//////////////////////////////////////////////////////////////////////////// Alchemy
    void setalchemy(int *arg)
    {
//        if(player1->state!=CS_ALIVE) return; // Do we need this?
		player1->alchemy = arg[0];
    }
    COMMAND(setalchemy,  "i");
    void addalchemy(int *arg)
    {
//        if(player1->state!=CS_ALIVE) return;
		player1->alchemy = player1->alchemy + arg[0];
    }
    COMMAND(addalchemy,  "i");
    ICOMMAND(getalchemy, "", (), intret(player1->alchemy));

//////////////////////////////////////////////////////////////////////////// Hotkeys
//	void sethotkey(int *arg, int *arg)
//	{
//		player1->hotkey arg[0] = arg[0];
//	}
//	COMMAND(sethotkey,  "ii");
    void sethotkey1(int *arg)
    {
		player1->hotkey1 = arg[0];
    }
    COMMAND(sethotkey1,  "i");
    ICOMMAND(gethotkey1, "", (), intret(player1->hotkey1));
    void sethotkey2(int *arg)
    {
		player1->hotkey2 = arg[0];
    }
    COMMAND(sethotkey2,  "i");
    ICOMMAND(gethotkey2, "", (), intret(player1->hotkey2));
    void sethotkey3(int *arg)
    {
		player1->hotkey3 = arg[0];
    }
    COMMAND(sethotkey3,  "i");
    ICOMMAND(gethotkey3, "", (), intret(player1->hotkey3));
    void sethotkey4(int *arg)
    {
		player1->hotkey4 = arg[0];
    }
    COMMAND(sethotkey4,  "i");
    ICOMMAND(gethotkey4, "", (), intret(player1->hotkey4));

	void sethotkey5(int *arg)
    {
		player1->hotkey5 = arg[0];
    }
    COMMAND(sethotkey5,  "i");
    ICOMMAND(gethotkey5, "", (), intret(player1->hotkey5));
    void sethotkey6(int *arg)
    {
		player1->hotkey6 = arg[0];
    }
    COMMAND(sethotkey6,  "i");
    ICOMMAND(gethotkey6, "", (), intret(player1->hotkey6));
    void sethotkey7(int *arg)
    {
		player1->hotkey7 = arg[0];
    }
    COMMAND(sethotkey7,  "i");
    ICOMMAND(gethotkey7, "", (), intret(player1->hotkey7));
    void sethotkey8(int *arg)
    {
		player1->hotkey8 = arg[0];
    }
    COMMAND(sethotkey8,  "i");
    ICOMMAND(gethotkey8, "", (), intret(player1->hotkey8));
    void sethotkey9(int *arg)
    {
		player1->hotkey9 = arg[0];
    }
    COMMAND(sethotkey9,  "i");
    ICOMMAND(gethotkey9, "", (), intret(player1->hotkey9));
    void sethotkey0(int *arg)
    {
		player1->hotkey0 = arg[0];
    }
    COMMAND(sethotkey0,  "i");
    ICOMMAND(gethotkey0, "", (), intret(player1->hotkey0));
	
//////////////////////////////////////////////////////////////////////////// * OLD * save/load game
	void writegameprogress(int *arg, char *gamename)
	{
		string gamecfgname;
		formatstring(gamecfgname)("packages/savegame/%s.cfg", gamename);
		stream *f = openutf8file((gamecfgname), "w");
		f->printf("// automaticly generated\n");
		f->printf("// do not modify\n");
		f->printf("// gamename %s\n", gamename);
		f->printf("gameprogress = %d\n", arg[0]);
		delete f;
	}
	COMMAND(writegameprogress, "is");
	
//////////////////////////////////////////////////////////////////////////// save/load game
	void savegame(int *arg, char *curmapname, char *gamename)
	{
		string gamecfgname;
		formatstring(gamecfgname)("packages/savegame/%s/%s.sav", gamename, curmapname);
		stream *f = openutf8file((gamecfgname), "w");
		f->printf("// automaticly generated\n");
		f->printf("// do not modify\n");
		f->printf("// gamename %s\n", curmapname);
		string inventorystring;
		int itemamount;
		for(int x=0;x<100;x=x+1)
		{
			itemamount = player1->inventory[x];
			formatstring(inventorystring)("inventoryoverwrite %d", x);
			f->printf("%s ", inventorystring);
			f->printf("%d\n", itemamount);
		}
//		f->printf("lognum = %d\n", arg[0]);
//////// this part below needs fixing for obvious reasons //////////////////////
//////// for now it only sets all logvars to 0 /////////////////////////////////
//		string logstring;
//		for(int x=0;x<arg[0];x=x+1)
//		{
//			formatstring(logstring)("logvar%d", x);
//			f->printf("logvar%d = ", x);
//			f->printf("0\n");
//		}

		f->printf("setenergyregen %f\n", player1->energyregen);
		f->printf("setplayerexp %d\n", player1->playerexperience);
		f->printf("setplayerlvl %d\n", player1->playerlevel);
		f->printf("setmaxcarryweight %d\n", player1->maxcarryweight);
		f->printf("setmaxhealth %d\n", player1->maxhealth);
		f->printf("sethealth %d\n", player1->health);
		f->printf("setlockpicking %d\n", player1->lockpicking);
		delete f;
	}
	COMMAND(savegame, "iss"); // lognum / curmapname / gamename

///////////////////////////////////////////////////////////////////////////// Zoom
    void waxon()
    {
		player1->zoom = 1;
    }
    COMMAND(waxon, "");
    void waxof()
    {
        player1->zoom = 0;
    }
    COMMAND(waxof, "");
//////////////////////////////////////////////////////////////////////////// inventory

	void inventoryoverwrite(int &x, int *arg, int &y)
	{
		if(y!=0)
		{
			player1->weight = player1->weight - y;
		}
		player1->inventory[x] = arg[0];
	}
	COMMAND(inventoryoverwrite, "iii");

	void inventorywrite(int &x, int *arg, int &y)
	{
		if(player1->state!=CS_ALIVE) return;
		if(y>0)
		{
			player1->weight = player1->weight + (y * arg[0]);
			if(player1->weight>=player1->maxcarryweight)
			{
				conoutf("You are over-encumbered");
//				player1->maxspeed = player1->maxspeed / 2;
				player1->maxspeed = 50;
			}
		}
		if(y<0)
		{
			player1->weight = player1->weight - (y * arg[0]);
			if(player1->weight>=player1->maxcarryweight)
			{
				conoutf("You are over-encumbered");
//				player1->maxspeed = player1->maxspeed / 2;
				player1->maxspeed = 50;
			}
		}
		player1->inventory[x] = player1->inventory[x] + arg[0];
	}
	COMMAND(inventorywrite, "iii");

    void setmaxcarryweight(int *arg)
    {
//        if(player1->state!=CS_ALIVE) return;
		player1->maxcarryweight = arg[0];
    }
	COMMAND(setmaxcarryweight,  "i");
    void addmaxcarryweight(int *arg)
    {
//        if(player1->state!=CS_ALIVE) return;
		player1->maxcarryweight = player1->maxcarryweight + arg[0];
    }
	COMMAND(addmaxcarryweight,  "i");
	ICOMMAND(getmaxcarryweight, "", (), intret(player1->maxcarryweight));
	ICOMMAND(getcarryweight, "", (), intret(player1->weight));
	
	
//////////////////////////////////////////////////////////////////////////// INVENTORY
//	0-79 reserved for pickups
//	80-100 reserved for ammo/guns
	
	ICOMMAND(inventoryread, "i", (int *arg), intret(player1->inventory[arg[0]]));

//////////////////////////////////////////////////////////////////////////// ENERGY

	void setmaxspeed(int *arg)
	{
		if(player1->state!=CS_ALIVE) return;
		player1->maxspeed = player1->maxspeed + arg[0];
	}
	COMMAND(setmaxspeed,  "i");
	ICOMMAND(getmaxspeed, "", (), intret(player1->maxspeed));
	
	void speedrun()
    {
		if(player1->weight < player1->maxcarryweight)
		{
			player1->maxspeed = 100;
			player1->lastspeedrun = lastmillis;
		}
		else
		{
			player1->maxspeed = 50;
		}
   	}
    COMMAND(speedrun, "");
    void normalrun()
    {
		player1->maxspeed = 50;
   	}
    COMMAND(normalrun, "");
    ICOMMAND(getspeedstat, "", (), intret(player1->maxspeed));
    ICOMMAND(getenergystat, "", (), intret(player1->energy));

    void crouch()
    {
        player1->eyeheight = 10;
		player1->maxspeed = 25;
    }
    COMMAND(crouch, "");
    void uncrouch()
    {
        player1->eyeheight = 15;
		player1->vel.z = 60.0f;
		player1->maxspeed = 50;
    }
    COMMAND(uncrouch, "");
    void getenergy(int *arg)
    {
        if(player1->state!=CS_ALIVE) return;
        if(player1->energy != 512.0f)
        {
			if(arg[0] + player1->energy >= 512.0f)
			{
				player1->energy = 512.0f;
			}
			else
			{
				player1->energy = player1->energy + arg[0];
			}
        }
    }
    COMMAND(getenergy, "i");
    ICOMMAND(getenergyregen, "", (), floatret(player1->energyregen));
    ICOMMAND(getenergyregenint, "", (), intret(player1->energyregen * 10));
    void addenergyregen()
    {
	    player1->energyregen = player1->energyregen + 0.1f;
    }
    COMMAND(addenergyregen, "");
    void setenergyregen(float  *arg)
    {
	    player1->energyregen = arg[0];
    }
    COMMAND(setenergyregen, "f");
//////////////////////////////////////////////////////////////////////// HEALTH
    void gethealing(int *arg)
    {
        if(player1->state!=CS_ALIVE) return;
        if(player1->health != player1->maxhealth)
        {
			if(arg[0] + player1->health >= player1->maxhealth)
			{
				player1->health = player1->maxhealth;
			}
			else
			{
				player1->health = player1->health + arg[0];
			}
        }
    }
    COMMAND(gethealing, "i");
    ICOMMAND(gethealthstat, "", (), intret(player1->health));
    ICOMMAND(getmaxhealthstat, "", (), intret(player1->maxhealth));
    void setmaxhealth(int *arg)
    {
	    player1->maxhealth = arg[0];
    }
    COMMAND(setmaxhealth, "i");
    void sethealth(int *arg)
    {
	    player1->health = arg[0];
    }
    COMMAND(sethealth, "i");
	void getpoison(int *arg)
	{
		if(arg[0] - player1->health >= 0)
		{
			suicide(player1);
		}
		else
		{
			player1->health = player1->health - arg[0];
		}
	}
	COMMAND(getpoison, "i");	
/////////////////////////////////////////////////////////////////////// MANA
    void getmana(int *arg)
    {
        if(player1->mana != 512.0f)
        {
			if(arg[0] + player1->mana >= 512)
			{
//				player1->ammo[GUN_FIREBALL] = 512;
				player1->mana = 512.0f;
			}
			else
			{
//				player1->ammo[GUN_FIREBALL] = player1->ammo[GUN_FIREBALL] + arg[0];
				player1->mana = player1->mana + arg[0];
			}
        }
    }
    COMMAND(getmana, "i");
	ICOMMAND(getmanastat, "", (), intret(player1->mana));
	
    ICOMMAND(getmanaregen, "", (), floatret(player1->manaregen));
    ICOMMAND(getmanaregenint, "", (), intret(player1->manaregen * 10));
    void addmanaregen()
    {
	    player1->manaregen = player1->manaregen + 0.1f;
    }
    COMMAND(addmanaregen, "");
    void setmanaregen(float  *arg)
    {
	    player1->manaregen = arg[0];
    }
    COMMAND(setmanaregen, "f");
/////////////////////////////////////////////////////////////////////// ARMOUR
    void getarmour(int *arg)
    {
        if(player1->state!=CS_ALIVE) return;
        if(player1->armour != 100)
        {
			if(arg[0] + player1->armour >= 100)
			{
				player1->armour = 100;
			}
			else
			{
				player1->armour = player1->armour + arg[0];
			}
        }
    }
    COMMAND(getarmour, "i");
    ICOMMAND(getarmourstat, "", (), intret(player1->armour));
/////////////////////////////////////////////////////////////////////// BRIBE-NPC
	void bribenpc(int *arg)
	{
		if(player1->state!=CS_ALIVE) return;
		if(player1->inventory[3] >= 10)
		{
			player1->inventory[3] -= 10;
			player1->npcdisposition = (arg[0] + ((rand() % 10) - (rand() % 5)));
			if(player1->npcdisposition < 0)
			{
				player1->npcdisposition = 0;
			}
			if(player1->npcdisposition > 100)
			{
				player1->npcdisposition = 100;
			}
		}
		else
		{
			player1->npcdisposition = arg[0];
			conoutf(CON_GAMEINFO, "\f2You dont have enough money!");
		}
	}
    COMMAND(bribenpc, "i");
	ICOMMAND(getnpcdisp, "", (), intret(player1->npcdisposition));
///////////////////////////////////////////////////////////////////////
	
    ICOMMAND(getfollow, "", (),
    {
        fpsent *f = followingplayer();
        intret(f ? f->clientnum : -1);
    });

	void follow(char *arg)
    {
        if(arg[0] ? player1->state==CS_SPECTATOR : following>=0)
        {
            following = arg[0] ? parseplayer(arg) : -1;
            if(following==player1->clientnum) following = -1;
            followdir = 0;
            conoutf("follow %s", following>=0 ? "on" : "off");
        }
	}
    COMMAND(follow, "s");

    void nextfollow(int dir)
    {
        if(player1->state!=CS_SPECTATOR || clients.empty())
        {
            stopfollowing();
            return;
        }
        int cur = following >= 0 ? following : (dir < 0 ? clients.length() - 1 : 0);
        loopv(clients)
        {
            cur = (cur + dir + clients.length()) % clients.length();
            if(clients[cur] && clients[cur]->state!=CS_SPECTATOR)
            {
                if(following<0) conoutf("follow on");
                following = cur;
                followdir = dir;
                return;
            }
        }
        stopfollowing();
    }
    ICOMMAND(nextfollow, "i", (int *dir), nextfollow(*dir < 0 ? -1 : 1));


    const char *getclientmap() { return clientmap; }

    void resetgamestate()
    {
        if(m_classicsp)
        {
            clearmovables();
            clearmonsters();                 // all monsters back at their spawns for editing
            entities::resettriggers();
        }
        clearprojectiles();
        clearbouncers();
    }

    fpsent *spawnstate(fpsent *d)              // reset player state not persistent accross spawns
    {
        d->respawn();
        d->spawnstate(gamemode);
        return d;
    }

    void respawnself()
    {
        if(ispaused()) return;
        if(m_mp(gamemode))
        {
            int seq = (player1->lifesequence<<16)|((lastmillis/1000)&0xFFFF);
            if(player1->respawned!=seq) { addmsg(N_TRYSPAWN, "rc", player1); player1->respawned = seq; }
        }
        else
        {
            spawnplayer(player1);
            showscores(false);
            lasthit = 0;
            if(cmode) cmode->respawned(player1);
        }
    }

    fpsent *pointatplayer()
    {
        loopv(players) if(players[i] != player1 && intersect(players[i], player1->o, worldpos)) return players[i];
        return NULL;
    }

    void stopfollowing()
    {
        if(following<0) return;
        following = -1;
        followdir = 0;
        conoutf("follow off");
    }

    fpsent *followingplayer()
    {
        if(player1->state!=CS_SPECTATOR || following<0) return NULL;
        fpsent *target = getclient(following);
        if(target && target->state!=CS_SPECTATOR) return target;
        return NULL;
    }

    fpsent *hudplayer()
    {
        if(thirdperson) return player1;
        fpsent *target = followingplayer();
        return target ? target : player1;
    }

    void setupcamera()
    {
        fpsent *target = followingplayer();
        if(target)
        {
            player1->yaw = target->yaw;
            player1->pitch = target->state==CS_DEAD ? 0 : target->pitch;
            player1->o = target->o;
            player1->resetinterp();
        }
    }

    bool detachcamera()
    {
        fpsent *d = hudplayer();
        return d->state==CS_DEAD;
    }

    bool collidecamera()
    {
        switch(player1->state)
        {
            case CS_EDITING: return false;
            case CS_SPECTATOR: return followingplayer()!=NULL;
        }
        return true;
    }

    VARP(smoothmove, 0, 75, 100);
    VARP(smoothdist, 0, 32, 64);

    void predictplayer(fpsent *d, bool move)
    {
        d->o = d->newpos;
        d->yaw = d->newyaw;
        d->pitch = d->newpitch;
        d->roll = d->newroll;
        if(move)
        {
            moveplayer(d, 1, false);
            d->newpos = d->o;
        }
        float k = 1.0f - float(lastmillis - d->smoothmillis)/smoothmove;
        if(k>0)
        {
            d->o.add(vec(d->deltapos).mul(k));
            d->yaw += d->deltayaw*k;
            if(d->yaw<0) d->yaw += 360;
            else if(d->yaw>=360) d->yaw -= 360;
            d->pitch += d->deltapitch*k;
            d->roll += d->deltaroll*k;
        }
    }

    void otherplayers(int curtime)
    {
        loopv(players)
        {
            fpsent *d = players[i];
            if(d == player1 || d->ai) continue;

            if(d->state==CS_DEAD && d->ragdoll) moveragdoll(d);
            else if(!intermission)
            {
                if(lastmillis - d->lastaction >= d->gunwait) d->gunwait = 0;
                if(d->quadmillis) entities::checkquad(curtime, d);
            }

            const int lagtime = totalmillis-d->lastupdate;
            if(!lagtime || intermission) continue;
            else if(lagtime>1000 && d->state==CS_ALIVE)
            {
                d->state = CS_LAGGED;
                continue;
            }
            if(d->state==CS_ALIVE || d->state==CS_EDITING)
            {
                if(smoothmove && d->smoothmillis>0) predictplayer(d, true);
                else moveplayer(d, 1, false);
            }
            else if(d->state==CS_DEAD && !d->ragdoll && lastmillis-d->lastpain<2000) moveplayer(d, 1, true);
        }
    }

    VARFP(slowmosp, 0, 0, 1, { if(m_sp && !slowmosp) server::forcegamespeed(100); }); 

    void checkslowmo()
    {
        static int lastslowmohealth = 0;
        server::forcegamespeed(intermission ? 100 : clamp(player1->health, 25, 200));
        if(player1->health<player1->maxhealth && lastmillis-max(maptime, lastslowmohealth)>player1->health*player1->health/2)
        {
            lastslowmohealth = lastmillis;
            player1->health++;
        }
    }

    void updateworld()        // main game update loop
    {
        if(!maptime) { maptime = lastmillis; maprealtime = totalmillis; return; }
        if(!curtime) { gets2c(); if(player1->clientnum>=0) c2sinfo(); return; }

        physicsframe();
        ai::navigate();
        if(player1->state != CS_DEAD && !intermission)
        {
            if(player1->quadmillis) entities::checkquad(curtime, player1);
        }
        updateweapons(curtime);
        otherplayers(curtime);
        ai::update();
        moveragdolls();
        gets2c();
        updatemovables(curtime);
        updatemonsters(curtime);
        if(player1->state == CS_DEAD)
        {
            if(player1->ragdoll) moveragdoll(player1);
            else if(lastmillis-player1->lastpain<2000)
            {
                player1->move = player1->strafe = 0;
                moveplayer(player1, 10, true);
            }
        }
        else if(!intermission)
        {
            if(player1->ragdoll) cleanragdoll(player1);
            moveplayer(player1, 10, true);
            swayhudgun(curtime);
            entities::checkitems(player1);
            if(m_sp)
            {
                if(slowmosp) checkslowmo();
                if(m_classicsp) entities::checktriggers();
            }
            else if(cmode) cmode->checkitems(player1);
        }
        if(player1->clientnum>=0) c2sinfo();   // do this last, to reduce the effective frame lag
    }

    void spawnplayer(fpsent *d)   // place at random spawn
    {
        if(cmode) cmode->pickspawn(d);
        else findplayerspawn(d, d==player1 && respawnent>=0 ? respawnent : -1);
        spawnstate(d);
        if(d==player1)
        {
            if(editmode) d->state = CS_EDITING;
            else if(d->state != CS_SPECTATOR) d->state = CS_ALIVE;
        }
        else d->state = CS_ALIVE;
    }

    VARP(spawnwait, 0, 0, 1000);

    void respawn()
    {
        if(player1->state==CS_DEAD)
        {
            player1->attacking = false;
            int wait = cmode ? cmode->respawnwait(player1) : 0;
            if(wait>0)
            {
                lastspawnattempt = lastmillis;
                //conoutf(CON_GAMEINFO, "\f2you must wait %d second%s before respawn!", wait, wait!=1 ? "s" : "");
                return;
            }
            if(lastmillis < player1->lastpain + spawnwait) return;
            if(m_dmsp) { changemap(clientmap, gamemode); return; }    // if we die in SP we try the same map again
            respawnself();
            if(m_classicsp)
            {
                conoutf(CON_GAMEINFO, "\f2You wasted another life! The monsters stole your armour and some ammo...");
                loopi(NUMGUNS) if(i!=GUN_PISTOL && (player1->ammo[i] = savedammo[i]) > 5) player1->ammo[i] = max(player1->ammo[i]/3, 5);
            }
        }
    }

    // inputs

    void doattack(bool on)
    {
        if(intermission) return;
        if((player1->attacking = on)) respawn();
    }

    bool canjump()
    {
        if(!intermission) respawn();
        return player1->state!=CS_DEAD && !intermission;
    }

    bool allowmove(physent *d)
    {
        if(d->type!=ENT_PLAYER) return true;
        return !((fpsent *)d)->lasttaunt || lastmillis-((fpsent *)d)->lasttaunt>=1000;
    }

    VARP(hitsound, 0, 0, 1);

    void damaged(int damage, fpsent *d, fpsent *actor, bool local)
    {
        if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission) return;

        if(local) damage = d->dodamage(damage);
        else if(actor==player1) return;

        fpsent *h = hudplayer();
        if(h!=player1 && actor==h && d!=actor)
        {
            if(hitsound && lasthit != lastmillis) playsound(S_HIT);
            lasthit = lastmillis;
        }
        if(d==h)
        {
            damageblend(damage);
            damagecompass(damage, actor->o);
        }
        damageeffect(damage, d, d!=h);

		ai::damaged(d, actor);

        if(m_sp && slowmosp && d==player1 && d->health < 1) d->health = 1;

        if(d->health<=0) { if(local) killed(d, actor); }
        else if(d==h) playsound(S_PAIN6);
        else playsound(S_PAIN1+rnd(5), &d->o);
    }

    VARP(deathscore, 0, 1, 1);

    void deathstate(fpsent *d, bool restore)
    {
        d->state = CS_DEAD;
        d->lastpain = lastmillis;
        if(!restore) gibeffect(max(-d->health, 0), d->vel, d);
        if(d==player1)
        {
            if(deathscore) showscores(true);
            disablezoom();
            if(!restore) loopi(NUMGUNS) savedammo[i] = player1->ammo[i];
            d->attacking = false;
            if(!restore) d->deaths++;
            //d->pitch = 0;
            d->roll = 0;
            playsound(S_DIE1+rnd(2));
        }
        else
        {
            d->move = d->strafe = 0;
            d->resetinterp();
            d->smoothmillis = 0;
            playsound(S_DIE1+rnd(2), &d->o);
        }
    }

    VARP(teamcolorfrags, 0, 1, 1);

    void killed(fpsent *d, fpsent *actor)
    {
        if(d->state==CS_EDITING)
        {
            d->editstate = CS_DEAD;
            if(d==player1) d->deaths++;
            else d->resetinterp();
            return;
        }
        else if((d->state!=CS_ALIVE && d->state != CS_LAGGED && d->state != CS_SPAWNING) || intermission) return;

        fpsent *h = followingplayer();
        if(!h) h = player1;
        int contype = d==h || actor==h ? CON_FRAG_SELF : CON_FRAG_OTHER;
        const char *dname = "", *aname = "";
        if(m_teammode && teamcolorfrags)
        {
            dname = teamcolorname(d, "you");
            aname = teamcolorname(actor, "you");
        }
        else
        {
            dname = colorname(d, NULL, "", "", "you");
            aname = colorname(actor, NULL, "", "", "you");
        }
        if(actor->type==ENT_AI)
            conoutf(contype, "\f2%s got killed by %s!", dname, aname);
        else if(d==actor || actor->type==ENT_INANIMATE)
            conoutf(contype, "\f2%s suicided%s", dname, d==player1 ? "!" : "");
        else if(isteam(d->team, actor->team))
        {
            contype |= CON_TEAMKILL;
            if(actor==player1) conoutf(contype, "\f6%s fragged a teammate (%s)", aname, dname);
            else if(d==player1) conoutf(contype, "\f6%s got fragged by a teammate (%s)", dname, aname);
            else conoutf(contype, "\f2%s fragged a teammate (%s)", aname, dname);
        }
        else
        {
            if(d==player1) conoutf(contype, "\f2%s got fragged by %s", dname, aname);
            else conoutf(contype, "\f2%s fragged %s", aname, dname);
        }
        deathstate(d);
		ai::killed(d, actor);
    }

    void timeupdate(int secs)
    {
        if(secs > 0)
        {
            maplimit = lastmillis + secs*1000;
        }
        else
        {
            intermission = true;
            player1->attacking = false;
            if(cmode) cmode->gameover();
            conoutf(CON_GAMEINFO, "\f2intermission:");
            conoutf(CON_GAMEINFO, "\f2game has ended!");
            if(m_ctf) conoutf(CON_GAMEINFO, "\f2player frags: %d, flags: %d, deaths: %d", player1->frags, player1->flags, player1->deaths);
            else if(m_collect) conoutf(CON_GAMEINFO, "\f2player frags: %d, skulls: %d, deaths: %d", player1->frags, player1->flags, player1->deaths);
            else conoutf(CON_GAMEINFO, "\f2player frags: %d, deaths: %d", player1->frags, player1->deaths);
            int accuracy = (player1->totaldamage*100)/max(player1->totalshots, 1);
            conoutf(CON_GAMEINFO, "\f2player total damage dealt: %d, damage wasted: %d, accuracy(%%): %d", player1->totaldamage, player1->totalshots-player1->totaldamage, accuracy);
            if(m_sp) spsummary(accuracy);

            showscores(true);
            disablezoom();
            
            if(identexists("intermission")) execute("intermission");
        }
    }

    ICOMMAND(getfrags, "", (), intret(player1->frags));
    ICOMMAND(getflags, "", (), intret(player1->flags));
    ICOMMAND(getdeaths, "", (), intret(player1->deaths));
    ICOMMAND(getaccuracy, "", (), intret((player1->totaldamage*100)/max(player1->totalshots, 1)));
    ICOMMAND(gettotaldamage, "", (), intret(player1->totaldamage));
    ICOMMAND(gettotalshots, "", (), intret(player1->totalshots));

    vector<fpsent *> clients;

    fpsent *newclient(int cn)   // ensure valid entity
    {
        if(cn < 0 || cn > max(0xFF, MAXCLIENTS + MAXBOTS))
        {
            neterr("clientnum", false);
            return NULL;
        }

        if(cn == player1->clientnum) return player1;

        while(cn >= clients.length()) clients.add(NULL);
        if(!clients[cn])
        {
            fpsent *d = new fpsent;
            d->clientnum = cn;
            clients[cn] = d;
            players.add(d);
        }
        return clients[cn];
    }

    fpsent *getclient(int cn)   // ensure valid entity
    {
        if(cn == player1->clientnum) return player1;
        return clients.inrange(cn) ? clients[cn] : NULL;
    }

    void clientdisconnected(int cn, bool notify)
    {
        if(!clients.inrange(cn)) return;
        if(following==cn)
        {
            if(followdir) nextfollow(followdir);
            else stopfollowing();
        }
        unignore(cn);
        fpsent *d = clients[cn];
        if(!d) return;
        if(notify && d->name[0]) conoutf("\f4leave:\f7 %s", colorname(d));
        removeweapons(d);
        removetrackedparticles(d);
        removetrackeddynlights(d);
        if(cmode) cmode->removeplayer(d);
        players.removeobj(d);
        DELETEP(clients[cn]);
        cleardynentcache();
    }

    void clearclients(bool notify)
    {
        loopv(clients) if(clients[i]) clientdisconnected(i, notify);
    }

    void initclient()
    {
        player1 = spawnstate(new fpsent);
        players.add(player1);
    }

    VARP(showmodeinfo, 0, 1, 1);

    void startgame()
    {
        clearmovables();
        clearmonsters();

        clearprojectiles();
        clearbouncers();
        clearragdolls();

        clearteaminfo();

        // reset perma-state
        loopv(players)
        {
            fpsent *d = players[i];
            d->frags = d->flags = 0;
            d->deaths = 0;
            d->totaldamage = 0;
            d->totalshots = 0;
            d->maxhealth = 100;
            d->lifesequence = -1;
            d->respawned = d->suicided = -2;
        }

        setclientmode();

        intermission = false;
        maptime = maprealtime = 0;
        maplimit = -1;

        if(cmode)
        {
            cmode->preload();
            cmode->setup();
        }

        conoutf(CON_GAMEINFO, "\f2game mode is %s", server::modename(gamemode));

        if(m_sp)
        {
            defformatstring(scorename)("bestscore_%s", getclientmap());
            const char *best = getalias(scorename);
            if(*best) conoutf(CON_GAMEINFO, "\f2try to beat your best score so far: %s", best);
        }
        else
        {
            const char *info = m_valid(gamemode) ? gamemodes[gamemode - STARTGAMEMODE].info : NULL;
            if(showmodeinfo && info) conoutf(CON_GAMEINFO, "\f0%s", info);
        }

        if(player1->playermodel != playermodel) switchplayermodel(playermodel);

        showscores(false);
        disablezoom();
        lasthit = 0;

        if(identexists("mapstart")) execute("mapstart");
    }

    void startmap(const char *name)   // called just after a map load
    {
        ai::savewaypoints();
        ai::clearwaypoints(true);

        respawnent = -1; // so we don't respawn at an old spot
        if(!m_mp(gamemode)) spawnplayer(player1);
        else findplayerspawn(player1, -1);
        entities::resetspawns();
        copystring(clientmap, name ? name : "");
        
        sendmapinfo();
    }

    const char *getmapinfo()
    {
        return showmodeinfo && m_valid(gamemode) ? gamemodes[gamemode - STARTGAMEMODE].info : NULL;
    }

    void physicstrigger(physent *d, bool local, int floorlevel, int waterlevel, int material)
    {
        if(d->type==ENT_INANIMATE) return;
        if     (waterlevel>0) { if(material!=MAT_LAVA) playsound(S_SPLASH1, d==player1 ? NULL : &d->o); }
        else if(waterlevel<0) playsound(material==MAT_LAVA ? S_BURN : S_SPLASH2, d==player1 ? NULL : &d->o);
///////////////////////////////////////////////////////////////////////////////////////
//		if     (floorlevel>0) { if(d==player1 || d->type!=ENT_PLAYER || ((fpsent *)d)->ai) msgsound(S_JUMP, d); }
//		else if(floorlevel<0) { if(d==player1 || d->type!=ENT_PLAYER || ((fpsent *)d)->ai) msgsound(S_LAND, d); }
		if     (floorlevel>0) { if(d==player1) msgsound(S_JUMP, d); }
		else if(floorlevel<0) { if(d==player1) msgsound(S_LAND, d); }
///////////////////////////////////////////////////////////////////////////////////////
    }

    void dynentcollide(physent *d, physent *o, const vec &dir)
    {
        switch(d->type)
        {
            case ENT_AI: if(dir.z > 0) stackmonster((monster *)d, o); break;
            case ENT_INANIMATE: if(dir.z > 0) stackmovable((movable *)d, o); break;
        }
    }

    void msgsound(int n, physent *d)
    {
        if(!d || d==player1)
        {
            addmsg(N_SOUND, "ci", d, n);
            playsound(n);
        }
        else
        {
            if(d->type==ENT_PLAYER && ((fpsent *)d)->ai)
                addmsg(N_SOUND, "ci", d, n);
            playsound(n, &d->o);
        }
    }

    int numdynents() { return players.length()+monsters.length()+movables.length(); }

    dynent *iterdynents(int i)
    {
        if(i<players.length()) return players[i];
        i -= players.length();
        if(i<monsters.length()) return (dynent *)monsters[i];
        i -= monsters.length();
        if(i<movables.length()) return (dynent *)movables[i];
        return NULL;
    }

    bool duplicatename(fpsent *d, const char *name = NULL, const char *alt = NULL)
    {
        if(!name) name = d->name;
        if(alt && d != player1 && !strcmp(name, alt)) return true;
        loopv(players) if(d!=players[i] && !strcmp(name, players[i]->name)) return true;
        return false;
    }

    static string cname[3];
    static int cidx = 0;

    const char *colorname(fpsent *d, const char *name, const char *prefix, const char *suffix, const char *alt)
    {
        if(!name) name = alt && d == player1 ? alt : d->name; 
        bool dup = !name[0] || duplicatename(d, name, alt) || d->aitype != AI_NONE;
        if(dup || prefix[0] || suffix[0])
        {
            cidx = (cidx+1)%3;
            if(dup) formatstring(cname[cidx])(d->aitype == AI_NONE ? "%s%s \fs\f5(%d)\fr%s" : "%s%s \fs\f5[%d]\fr%s", prefix, name, d->clientnum, suffix);
            else formatstring(cname[cidx])("%s%s%s", prefix, name, suffix);
            return cname[cidx];
        }
        return name;
    }

    VARP(teamcolortext, 0, 1, 1);

    const char *teamcolorname(fpsent *d, const char *alt)
    {
        if(!teamcolortext || !m_teammode) return colorname(d, NULL, "", "", alt);
        return colorname(d, NULL, isteam(d->team, player1->team) ? "\fs\f1" : "\fs\f3", "\fr", alt); 
    }

    const char *teamcolor(const char *name, bool sameteam, const char *alt)
    {
        if(!teamcolortext || !m_teammode) return sameteam || !alt ? name : alt;
        cidx = (cidx+1)%3;
        formatstring(cname[cidx])(sameteam ? "\fs\f1%s\fr" : "\fs\f3%s\fr", sameteam || !alt ? name : alt);
        return cname[cidx];
    }    
    
    const char *teamcolor(const char *name, const char *team, const char *alt)
    {
        return teamcolor(name, team && isteam(team, player1->team), alt);
    }

    void suicide(physent *d)
    {
        if(d==player1 || (d->type==ENT_PLAYER && ((fpsent *)d)->ai))
        {
            if(d->state!=CS_ALIVE) return;
            fpsent *pl = (fpsent *)d;
            if(!m_mp(gamemode)) killed(pl, pl);
            else 
            {
                int seq = (pl->lifesequence<<16)|((lastmillis/1000)&0xFFFF);
                if(pl->suicided!=seq) { addmsg(N_SUICIDE, "rc", pl); pl->suicided = seq; }
            }
        }
        else if(d->type==ENT_AI) suicidemonster((monster *)d);
        else if(d->type==ENT_INANIMATE) suicidemovable((movable *)d);
    }
    ICOMMAND(kill, "", (), suicide(player1));

///////////////////////////////////////////////////////////////////////////////////////
    void burning(physent *d)
    {
        if(d==player1 || (d->type==ENT_PLAYER && ((fpsent *)d)->ai))
        {
			if(d->state!=CS_ALIVE) return;
			float burndamage = (lastmillis/100 - player1->lastburn/100);
//			conoutf(CON_ERROR, "burn %d %d %f", lastmillis, player1->lastburn, burndamage);
			player1->lastburn = lastmillis;
			player1->health = player1->health - burndamage;
			if(player1->health <= 0) 
			{
				suicide(player1);
			}
        }
        else if(d->type==ENT_AI) suicidemonster((monster *)d);
        else if(d->type==ENT_INANIMATE) suicidemovable((movable *)d);
    }
///////////////////////////////////////////////////////////////////////////////////////
	
    bool needminimap() { return m_ctf || m_protect || m_hold || m_capture || m_collect; }

    void drawicon(int icon, float x, float y, float sz)
    {
        settexture("packages/hud/items.png");
        glBegin(GL_TRIANGLE_STRIP);
        float tsz = 0.25f, tx = tsz*(icon%4), ty = tsz*(icon/4);
        glTexCoord2f(tx,     ty);     glVertex2f(x,    y);
        glTexCoord2f(tx+tsz, ty);     glVertex2f(x+sz, y);
        glTexCoord2f(tx,     ty+tsz); glVertex2f(x,    y+sz);
        glTexCoord2f(tx+tsz, ty+tsz); glVertex2f(x+sz, y+sz);
        glEnd();
    }

////////////////////////////////////////////////////////////////////////////

	void drawhudbg(int icon, float x, float y, float sz)
    {
        settexture("packages/hud/hudbg.png");
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x,     y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x+570, y);
        glTexCoord2f(0.0f, 0.75f); glVertex2f(x,     y+180);
        glTexCoord2f(1.0f, 0.75f); glVertex2f(x+570, y+180);
        glEnd();
    }
    void drawhealthbar(int icon, float x, float y, float sz)
    {
		float healthbarstat = sz * player1->health / player1->maxhealth;
        settexture("packages/hud/healthbar.png");
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x,               y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x+healthbarstat, y);
        glTexCoord2f(0.0f, 0.5f); glVertex2f(x,               y+sz/20);
        glTexCoord2f(1.0f, 0.5f); glVertex2f(x+healthbarstat, y+sz/20);
        glEnd();
    }
	void drawenergybar(int icon, float x, float y, float sz)
    {
		if((player1->maxspeed != 100) && (player1->energy == 512.0f)) // 1
		{
		    settexture("packages/hud/energybar.png");
		    glBegin(GL_TRIANGLE_STRIP);
		    glTexCoord2f(0.0f, 0.0f); glVertex2f(x,    y);
		    glTexCoord2f(1.0f, 0.0f); glVertex2f(x+sz, y);
		    glTexCoord2f(0.0f, 0.5f); glVertex2f(x,    y+sz/20);
		    glTexCoord2f(1.0f, 0.5f); glVertex2f(x+sz, y+sz/20);
		    glEnd();
		}
		if((player1->maxspeed == 100) && (player1->energy >= 1.0f)) // 2
		{
			float energybarstat = player1->energy - (lastmillis/10 - player1->lastspeedrun/10);
		    player1->lastspeedrun = lastmillis;
			if(player1->energy == 512.0f)
			{
				player1->energy = energybarstat;
			}
			if(player1->energy <= 512.0f)
			{
				if(energybarstat<1.0f)
				{
					player1->energy = 1.0f;
				} else {
					player1->energy = energybarstat;
				}
			}
			settexture("packages/hud/energybar.png");
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,               y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+energybarstat, y);
			glTexCoord2f(0.0f, 0.5f); glVertex2f(x,               y+sz/20);
			glTexCoord2f(1.0f, 0.5f); glVertex2f(x+energybarstat, y+sz/20);
			glEnd();

		}
		if((player1->maxspeed == 100) && (player1->energy <= 1.0f)) // 3
		{
			player1->maxspeed = 50;
			float energybarstat = player1->energy + player1->energyregen;
			player1->energy = energybarstat;
			if(player1->energy>512.0f)
			{
				player1->energy = 512.0f;
			}
			settexture("packages/hud/energybar.png");
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,               y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+energybarstat, y);
			glTexCoord2f(0.0f, 0.5f); glVertex2f(x,               y+sz/20);
			glTexCoord2f(1.0f, 0.5f); glVertex2f(x+energybarstat, y+sz/20);
			glEnd();
		}
		if((player1->maxspeed != 100) && (player1->energy >= 1.0f)) // 4
		{
			float energybarstat = player1->energy + player1->energyregen;
			player1->energy = energybarstat;
			if(player1->energy>512.0f)
			{
				player1->energy = 512.0f;
			}
			settexture("packages/hud/energybar.png");
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,               y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+energybarstat, y);
			glTexCoord2f(0.0f, 0.5f); glVertex2f(x,               y+sz/20);
			glTexCoord2f(1.0f, 0.5f); glVertex2f(x+energybarstat, y+sz/20);
			glEnd();
		}
    }
////////////////////////////////////////////////////////////////////////// Mana

	void drawmanabar(int icon, float x, float y, float sz)
    {
		if(player1->mana == 512.0f)
		{
			float manabarstat = sz * player1->mana / player1->maxmana;
			settexture("packages/hud/manabar.png");
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,             y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+manabarstat, y);
			glTexCoord2f(0.0f, 0.5f); glVertex2f(x,             y+sz/20);
			glTexCoord2f(1.0f, 0.5f); glVertex2f(x+manabarstat, y+sz/20);
			glEnd();
		}
		if(player1->mana != 512.0f)
		{
			if(player1->mana >= 512.0f)
			{
				player1->mana = 512.0f;
			}
			if(player1->mana <= 0.0f)
			{
				player1->mana = 0.0f;
			}
//			float manabarstat = sz * player1->mana / player1->maxmana;
			float manabarstat = player1->mana + player1->manaregen;
			player1->mana = manabarstat;
			settexture("packages/hud/manabar.png");
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,             y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+manabarstat, y);
			glTexCoord2f(0.0f, 0.5f); glVertex2f(x,             y+sz/20);
			glTexCoord2f(1.0f, 0.5f); glVertex2f(x+manabarstat, y+sz/20);
			glEnd();
		}
    }
////////////////////////////////////////////////////////////////////////// Hotkeys

    void drawhotkeybg(int icon, float x, float y, float sz)
    {
        settexture("packages/hud/hotkeybg.png");
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x,     y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x+1800, y);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x,     y+160);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x+1800, y+160);
        glEnd();
    }
    void drawhotkey1(int icon, float x, float y, float sz)
    {
		if((player1->hotkey1)!=0)
		{
			string xkey1;
			formatstring(xkey1)("packages/hud/%d.png", player1->hotkey1);
			settexture(xkey1);
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,		y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+160,	y);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x,		y+160);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x+160,	y+160);
			glEnd();
		}
		else
		{
			return;
		}
    }
    void drawhotkey2(int icon, float x, float y, float sz)
    {
		if((player1->hotkey2)!=0)
		{
			string xkey2;
			formatstring(xkey2)("packages/hud/%d.png", player1->hotkey2);
			settexture(xkey2);
			x = x+180;
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,		y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+160,	y);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x,		y+160);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x+160,	y+160);
			glEnd();
		}
		else
		{
			return;
		}
    }
    void drawhotkey3(int icon, float x, float y, float sz)
    {
		if((player1->hotkey3)!=0)
		{
			string xkey3;
			formatstring(xkey3)("packages/hud/%d.png", player1->hotkey3);
			settexture(xkey3);
			x = x+360;
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,		y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+160,	y);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x,		y+160);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x+160,	y+160);
			glEnd();
		}
		else
		{
			return;
		}
    }
    void drawhotkey4(int icon, float x, float y, float sz)
    {
		if((player1->hotkey4)!=0)
		{
			string xkey4;
			formatstring(xkey4)("packages/hud/%d.png", player1->hotkey4);
			settexture(xkey4);
			x = x+540;
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,		y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+160,	y);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x,		y+160);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x+160,	y+160);
			glEnd();
		}
		else
		{
			return;
		}
    }
    void drawhotkey5(int icon, float x, float y, float sz)
    {
		if((player1->hotkey5)!=0)
		{
			string xkey5;
			formatstring(xkey5)("packages/hud/%d.png", player1->hotkey5);
			settexture(xkey5);
			x = x+720;
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,		y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+160,	y);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x,		y+160);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x+160,	y+160);
			glEnd();
		}
		else
		{
			return;
		}
    }
    void drawhotkey6(int icon, float x, float y, float sz)
    {
		if((player1->hotkey6)!=0)
		{
			string xkey6;
			formatstring(xkey6)("packages/hud/%d.png", player1->hotkey6);
			settexture(xkey6);
			x = x+900;
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,		y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+160,	y);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x,		y+160);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x+160,	y+160);
			glEnd();
		}
		else
		{
			return;
		}
    }
    void drawhotkey7(int icon, float x, float y, float sz)
    {
		if((player1->hotkey7)!=0)
		{
			string xkey7;
			formatstring(xkey7)("packages/hud/%d.png", player1->hotkey7);
			settexture(xkey7);
			x = x+1080;
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,		y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+160,	y);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x,		y+160);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x+160,	y+160);
			glEnd();
		}
		else
		{
			return;
		}
    }
    void drawhotkey8(int icon, float x, float y, float sz)
    {
		if((player1->hotkey8)!=0)
		{
			string xkey8;
			formatstring(xkey8)("packages/hud/%d.png", player1->hotkey8);
			settexture(xkey8);
			x = x+1260;
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,		y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+160,	y);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x,		y+160);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x+160,	y+160);
			glEnd();
		}
		else
		{
			return;
		}
    }
    void drawhotkey9(int icon, float x, float y, float sz)
    {
		if((player1->hotkey9)!=0)
		{
			string xkey9;
			formatstring(xkey9)("packages/hud/%d.png", player1->hotkey9);
			settexture(xkey9);
			x = x+1440;
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,		y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+160,	y);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x,		y+160);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x+160,	y+160);
			glEnd();
		}
		else
		{
			return;
		}
    }
    void drawhotkey0(int icon, float x, float y, float sz)
    {
		if((player1->hotkey0)!=0)
		{
			string xkey0;
			formatstring(xkey0)("packages/hud/%d.png", player1->hotkey0);
			settexture(xkey0);
			x = x+1620;
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x,		y);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(x+160,	y);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(x,		y+160);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(x+160,	y+160);
			glEnd();
		}
		else
		{
			return;
		}
    }

/////////////////////////////////////
    void drawsniperscope()
    {
		if((player1->gunselect==GUN_RIFLE) && (player1->zoom==1))
		{
//			float x = SCR_W, y = SCR_H;
			settexture("packages/hud/scope.png");
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(3200.0f, 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1800.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(3200.0f, 1800.0f);
			glEnd();
		}
		else
		{
			return;
		}
    }
    void drawlevelup()
    {
//		draw_textf("Exp: %d", 1300, 800, (player1->playerexperience));
		if((player1->playerexperience>=99) && (player1->playerlevel==0))
		{
			draw_textf("You have reached level 1", 1800, 400);
		}
		else if((player1->playerexperience>=199) && (player1->playerlevel==1))
		{
			draw_textf("You have reached level 2", 1800, 400);
		}
		else if((player1->playerexperience>=399) && (player1->playerlevel==2))
		{
			draw_textf("You have reached level 3", 1800, 400);
		}
		else if((player1->playerexperience>=799) && (player1->playerlevel==3))
		{
			draw_textf("You have reached level 4", 1800, 400);
		}
		else if((player1->playerexperience>=1599) && (player1->playerlevel==4))
		{
			draw_textf("You have reached level 5", 1800, 400);
		}
		else if((player1->playerexperience>=3199) && (player1->playerlevel==5))
		{
			draw_textf("You have reached level 6", 1800, 400);
		}
    }
////////////////////////////////////////////////////////////////////////////

    float abovegameplayhud(int w, int h)
    {
        switch(hudplayer()->state)
        {
            case CS_EDITING:
            case CS_SPECTATOR:
                return 1;
            default:
                return 1650.0f/1800.0f;
        }
    }

    int ammohudup[3] = { GUN_CG, GUN_RL, GUN_GL },
        ammohuddown[3] = { GUN_RIFLE, GUN_SG, GUN_PISTOL },
        ammohudcycle[7] = { -1, -1, -1, -1, -1, -1, -1 };

    ICOMMAND(ammohudup, "V", (tagval *args, int numargs),
    {
        loopi(3) ammohudup[i] = i < numargs ? getweapon(args[i].getstr()) : -1;
    });

    ICOMMAND(ammohuddown, "V", (tagval *args, int numargs),
    {
        loopi(3) ammohuddown[i] = i < numargs ? getweapon(args[i].getstr()) : -1;
    });

    ICOMMAND(ammohudcycle, "V", (tagval *args, int numargs),
    {
        loopi(7) ammohudcycle[i] = i < numargs ? getweapon(args[i].getstr()) : -1;
    });

    VARP(ammohud, 0, 1, 1);

    void drawammohud(fpsent *d)
    {
        float x = HICON_X + 2*HICON_STEP, y = HICON_Y, sz = HICON_SIZE;
        glPushMatrix();
        glScalef(1/3.2f, 1/3.2f, 1);
        float xup = (x+sz)*3.2f, yup = y*3.2f + 0.1f*sz;
        loopi(3)
        {
            int gun = ammohudup[i];
            if(gun < GUN_FIST || gun > GUN_PISTOL || gun == d->gunselect || !d->ammo[gun]) continue;
            drawicon(HICON_FIST+gun, xup, yup, sz);
            yup += sz;
        }
        float xdown = x*3.2f - sz, ydown = (y+sz)*3.2f - 0.1f*sz;
        loopi(3)
        {
            int gun = ammohuddown[3-i-1];
            if(gun < GUN_FIST || gun > GUN_PISTOL || gun == d->gunselect || !d->ammo[gun]) continue;
            ydown -= sz;
            drawicon(HICON_FIST+gun, xdown, ydown, sz);
        }
        int offset = 0, num = 0;
        loopi(7)
        {
            int gun = ammohudcycle[i];
            if(gun < GUN_FIST || gun > GUN_PISTOL) continue;
            if(gun == d->gunselect) offset = i + 1;
            else if(d->ammo[gun]) num++;
        }
        float xcycle = (x+sz/2)*3.2f + 0.5f*num*sz, ycycle = y*3.2f-sz;
        loopi(7)
        {
            int gun = ammohudcycle[(i + offset)%7];
            if(gun < GUN_FIST || gun > GUN_PISTOL || gun == d->gunselect || !d->ammo[gun]) continue;
            xcycle -= sz;
            drawicon(HICON_FIST+gun, xcycle, ycycle, sz);
        }
        glPopMatrix();
    }

    void drawhudicons(fpsent *d)
    {
//		glPushMatrix();
//		glScalef(2, 2, 1);
///////////////////////////////////////////////////////////////////////////////////
//		draw_textf("%d", (HICON_X + HICON_SIZE + HICON_SPACE)/2, HICON_TEXTY/2, d->state==CS_DEAD ? 0 : d->health);
//		if(d->state!=CS_DEAD)
//		{
//			if(d->armour) draw_textf("%d", (HICON_X + HICON_STEP + HICON_SIZE + HICON_SPACE)/2, HICON_TEXTY/2, d->armour);
//			draw_textf("%d", (HICON_X + 2*HICON_STEP + HICON_SIZE + HICON_SPACE)/2, HICON_TEXTY/2, d->ammo[d->gunselect]);
//			draw_textf("%d", 1200, 800, d->ammo[d->gunselect]);
//		}

//		glPopMatrix();

//		drawicon(HICON_HEALTH, HICON_X, HICON_Y);
		
        if(d->state!=CS_DEAD)			
        {		
//          if(d->armour) drawicon(HICON_BLUE_ARMOUR+d->armourtype, HICON_X + HICON_STEP, HICON_Y);
//            drawicon(HICON_FIST+d->gunselect, HICON_X + 2*HICON_STEP, HICON_Y);
//          if(d->quadmillis) drawicon(HICON_QUAD, HICON_X + 3*HICON_STEP, HICON_Y);
//            if(ammohud) drawammohud(d);
			drawhealthbar(HICON_BLUE_ARMOUR, 40.0f, 1654.0f);
			drawenergybar(HICON_BLUE_ARMOUR, 40.0f, 1708.0f);
			drawmanabar(HICON_BLUE_ARMOUR, 40.0f, 1762.0f);
			drawhudbg(HICON_BLUE_ARMOUR, 10.0f, 1630.0f);
			drawhotkeybg(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawhotkey1(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawhotkey2(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawhotkey3(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawhotkey4(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawhotkey5(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawhotkey6(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawhotkey7(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawhotkey8(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawhotkey9(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawhotkey0(HICON_BLUE_ARMOUR, 600.0f, 1640.0f);
			drawsniperscope();
			drawlevelup();
        }
    }
///////////////////////////////////////////////////////////////////////////////////
    
    void gameplayhud(int w, int h)
    {
        glPushMatrix();
        glScalef(h/1800.0f, h/1800.0f, 1);

        if(player1->state==CS_SPECTATOR)
        {
            int pw, ph, tw, th, fw, fh;
            text_bounds("  ", pw, ph);
            text_bounds("SPECTATOR", tw, th);
            th = max(th, ph);
            fpsent *f = followingplayer();
            text_bounds(f ? colorname(f) : " ", fw, fh);
            fh = max(fh, ph);
            draw_text("SPECTATOR", w*1800/h - tw - pw, 1650 - th - fh);
            if(f) 
            {
                int color = f->state!=CS_DEAD ? 0xFFFFFF : 0x606060;
                if(f->privilege)
                {
                    color = f->privilege>=PRIV_ADMIN ? 0xFF8000 : 0x40FF80;
                    if(f->state==CS_DEAD) color = (color>>1)&0x7F7F7F;
                }
                draw_text(colorname(f), w*1800/h - fw - pw, 1650 - fh, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
            }
        }

        fpsent *d = hudplayer();
        if(d->state!=CS_EDITING)
        {
            if(d->state!=CS_SPECTATOR) drawhudicons(d);
            if(cmode) cmode->drawhud(d, w, h);
        }

        glPopMatrix();
    }

    int clipconsole(int w, int h)
    {
        if(cmode) return cmode->clipconsole(w, h);
        return 0;
    }

/////////////////////////////////////////////////////////////////////////////////////
	VARP(teamcrosshair, 0, 1, 1); // pointing at items/npcs NOT teamplayers
    VARP(hitcrosshair, 0, 425, 1000);

    const char *defaultcrosshair(int index)
    {
        switch(index)
        {
            case 2: return "data/hit.png";
            case 1: return "data/crosshair_e.png";
            default: return "data/crosshair.png";
        }
    }
	
    int activecrosshair = 0;
    int activateitem = 0;
    int selectcrosshair(float &r, float &g, float &b)
    {
        fpsent *d = hudplayer();
        if(d->state==CS_SPECTATOR || d->state==CS_DEAD) return -1;

        if(d->state!=CS_ALIVE) return 0;

        int crosshair = 0;
        if(lasthit && lastmillis - lasthit < hitcrosshair) crosshair = 2;
        else if(teamcrosshair)
        {
			dynent *o = intersectclosest(d->o, worldpos, d);
			float dist = player1->o.dist(worldpos);
//			if(o && o->type==ENT_INANIMATE && dist<25)
			if(o && o->type==ET_MAPMODEL && dist<25)
//			if(o && o->type==MONSTER && dist<25)
            {
                crosshair = 1;
				r = 0.5f; g = 1.0f; b = 0;
                activecrosshair = 1;
				if(activateitem == 1)
				{
					suicide((dynent *)o);
					activateitem = 0;
				}
			}
            else
			{
                activecrosshair = 0;
				activateitem = 0;
            }
        }

        if(crosshair!=1 && !editmode && !m_insta)
        {
            if(d->health<=25) { r = 1.0f; g = b = 0; }
            else if(d->health<=50) { r = 1.0f; g = 0.5f; b = 0; }
        }
        if(d->gunwait) { r *= 0.5f; g *= 0.5f; b *= 0.5f; }
        return crosshair;
    }
	ICOMMAND(getactivecrosshair, "i", (), intret(activecrosshair));
	void getitem()
	{
		activateitem = 1;
	}
	COMMAND(getitem, "");

//////////////////////////////////////////////////////////////////////////////////////
	
    void lighteffects(dynent *e, vec &color, vec &dir)
    {
#if 0
        fpsent *d = (fpsent *)e;
        if(d->state!=CS_DEAD && d->quadmillis)
        {
            float t = 0.5f + 0.5f*sinf(2*M_PI*lastmillis/1000.0f);
            color.y = color.y*(1-t) + t;
        }
#endif
    }

    bool serverinfostartcolumn(g3d_gui *g, int i)
    {
        static const char * const names[] = { "ping ", "players ", "mode ", "map ", "time ", "master ", "host ", "port ", "description " };
        static const float struts[] =       { 7,       7,          12.5f,   14,      7,      8,         14,      7,       24.5f };
        if(size_t(i) >= sizeof(names)/sizeof(names[0])) return false;
        g->pushlist();
        g->text(names[i], 0xFFFF80, !i ? " " : NULL);
        if(struts[i]) g->strut(struts[i]);
        g->mergehits(true);
        return true;
    }

    void serverinfoendcolumn(g3d_gui *g, int i)
    {
        g->mergehits(false);
        g->column(i);
        g->poplist();
    }

    const char *mastermodecolor(int n, const char *unknown)
    {
        return (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodecolors)/sizeof(mastermodecolors[0])) ? mastermodecolors[n-MM_START] : unknown;
    }

    const char *mastermodeicon(int n, const char *unknown)
    {
        return (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodeicons)/sizeof(mastermodeicons[0])) ? mastermodeicons[n-MM_START] : unknown;
    }

    bool serverinfoentry(g3d_gui *g, int i, const char *name, int port, const char *sdesc, const char *map, int ping, const vector<int> &attr, int np)
    {
        if(ping < 0 || attr.empty() || attr[0]!=PROTOCOL_VERSION)
        {
            switch(i)
            {
                case 0:
                    if(g->button(" ", 0xFFFFDD, "serverunk")&G3D_UP) return true;
                    break;

                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    if(g->button(" ", 0xFFFFDD)&G3D_UP) return true;
                    break;

                case 6:
                    if(g->buttonf("%s ", 0xFFFFDD, NULL, name)&G3D_UP) return true;
                    break;

                case 7:
                    if(g->buttonf("%d ", 0xFFFFDD, NULL, port)&G3D_UP) return true;
                    break;

                case 8:
                    if(ping < 0)
                    {
                        if(g->button(sdesc, 0xFFFFDD)&G3D_UP) return true;
                    }
                    else if(g->buttonf("[%s protocol] ", 0xFFFFDD, NULL, attr.empty() ? "unknown" : (attr[0] < PROTOCOL_VERSION ? "older" : "newer"))&G3D_UP) return true;
                    break;
            }
            return false;
        }

        switch(i)
        {
            case 0:
            {
                const char *icon = attr.inrange(3) && np >= attr[3] ? "serverfull" : (attr.inrange(4) ? mastermodeicon(attr[4], "serverunk") : "serverunk");
                if(g->buttonf("%d ", 0xFFFFDD, icon, ping)&G3D_UP) return true;
                break;
            }

            case 1:
                if(attr.length()>=4)
                {
                    if(g->buttonf(np >= attr[3] ? "\f3%d/%d " : "%d/%d ", 0xFFFFDD, NULL, np, attr[3])&G3D_UP) return true;
                }
                else if(g->buttonf("%d ", 0xFFFFDD, NULL, np)&G3D_UP) return true;
                break;

            case 2:
                if(g->buttonf("%s ", 0xFFFFDD, NULL, attr.length()>=2 ? server::modename(attr[1], "") : "")&G3D_UP) return true;
                break;

            case 3:
                if(g->buttonf("%.25s ", 0xFFFFDD, NULL, map)&G3D_UP) return true;
                break;

            case 4:
                if(attr.length()>=3 && attr[2] > 0)
                {
                    int secs = clamp(attr[2], 0, 59*60+59),
                        mins = secs/60;
                    secs %= 60;
                    if(g->buttonf("%d:%02d ", 0xFFFFDD, NULL, mins, secs)&G3D_UP) return true;
                }
                else if(g->buttonf(" ", 0xFFFFDD)&G3D_UP) return true;
                break;
            case 5:
                if(g->buttonf("%s%s ", 0xFFFFDD, NULL, attr.length()>=5 ? mastermodecolor(attr[4], "") : "", attr.length()>=5 ? server::mastermodename(attr[4], "") : "")&G3D_UP) return true;
                break;

            case 6:
                if(g->buttonf("%s ", 0xFFFFDD, NULL, name)&G3D_UP) return true;
                break;

            case 7:
                if(g->buttonf("%d ", 0xFFFFDD, NULL, port)&G3D_UP) return true;
                break;

            case 8:
                if(g->buttonf("%.25s", 0xFFFFDD, NULL, sdesc)&G3D_UP) return true;
                break;
        }
        return false;
    }

    // any data written into this vector will get saved with the map data. Must take care to do own versioning, and endianess if applicable. Will not get called when loading maps from other games, so provide defaults.
    void writegamedata(vector<char> &extras) {}
    void readgamedata(vector<char> &extras) {}

    const char *savedconfig() { return "config.cfg"; }
    const char *restoreconfig() { return "restore.cfg"; }
    const char *defaultconfig() { return "data/defaults.cfg"; }
    const char *autoexec() { return "autoexec.cfg"; }
    const char *savedservers() { return "servers.cfg"; }

    void loadconfigs()
    {
        execfile("auth.cfg", false);
    }
}

