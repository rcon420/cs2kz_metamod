#include "movement.h"
#include "utils/detours.h"

#include "tier0/memdbgon.h"

void movement::InitDetours()
{
	INIT_DETOUR(GetMaxSpeed);
	INIT_DETOUR(ProcessMovement);
	INIT_DETOUR(PlayerMoveNew);
	INIT_DETOUR(CheckParameters);
	INIT_DETOUR(CanMove);
	INIT_DETOUR(FullWalkMove);
	INIT_DETOUR(MoveInit);
	INIT_DETOUR(CheckWater);
	INIT_DETOUR(CheckVelocity);
	INIT_DETOUR(Duck);
	INIT_DETOUR(LadderMove);
	INIT_DETOUR(CheckJumpButton);
	INIT_DETOUR(OnJump);
	INIT_DETOUR(AirAccelerate);
	INIT_DETOUR(Friction);
	INIT_DETOUR(WalkMove);
	INIT_DETOUR(TryPlayerMove);
	INIT_DETOUR(CategorizePosition);
	INIT_DETOUR(FinishGravity);
	INIT_DETOUR(CheckFalling);
	INIT_DETOUR(PlayerMovePost);
	INIT_DETOUR(PostThink);
}

f32 FASTCALL movement::Detour_GetMaxSpeed(CCSPlayerPawn *pawn)
{
	DEBUG_PRINT(Detour_GetMaxSpeed);
	DEBUG_PRINT_POST(Detour_GetMaxSpeed);
	return GetMaxSpeed(pawn);
}

void FASTCALL movement::Detour_ProcessMovement(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_ProcessMovement);
	MovementPlayer *player = g_pPlayerManager->ToPlayer(ms);
	player->currentMoveData = mv;
	player->moveDataPre = CMoveData(*mv);
	player->OnStartProcessMovement();
	ProcessMovement(ms, mv);
	player->moveDataPost = CMoveData(*mv);
	player->OnStopProcessMovement();
	DEBUG_PRINT_POST(Detour_ProcessMovement);
}

bool FASTCALL movement::Detour_PlayerMoveNew(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_PlayerMoveNew);
	DEBUG_PRINT_POST(Detour_PlayerMoveNew);
	return PlayerMoveNew(ms, mv);
}

void FASTCALL movement::Detour_CheckParameters(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_CheckParameters);
	CheckParameters(ms, mv);
	DEBUG_PRINT_POST(Detour_CheckParameters);
}

bool FASTCALL movement::Detour_CanMove(CCSPlayerPawnBase *pawn)
{
	DEBUG_PRINT(Detour_CanMove);
	DEBUG_PRINT_POST(Detour_CanMove);
	return CanMove(pawn);
}

void FASTCALL movement::Detour_FullWalkMove(CCSPlayer_MovementServices *ms, CMoveData *mv, bool ground)
{
	DEBUG_PRINT(Detour_FullWalkMove);
	FullWalkMove(ms, mv, ground);
	DEBUG_PRINT_POST(Detour_FullWalkMove);
}

bool FASTCALL movement::Detour_MoveInit(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_MoveInit);
	DEBUG_PRINT_POST(Detour_MoveInit);
	return MoveInit(ms, mv);
}

bool FASTCALL movement::Detour_CheckWater(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_CheckWater);
	DEBUG_PRINT_POST(Detour_CheckWater);
	return CheckWater(ms, mv);
}

void FASTCALL movement::Detour_CheckVelocity(CCSPlayer_MovementServices *ms, CMoveData *mv, const char *a3)
{
	DEBUG_PRINT(Detour_CheckVelocity);
	CheckVelocity(ms, mv, a3);
	DEBUG_PRINT_POST(Detour_CheckVelocity);
}

void FASTCALL movement::Detour_Duck(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_Duck);
	MovementPlayer *player = g_pPlayerManager->ToPlayer(ms);
	player->processingDuck = true;
	Duck(ms, mv);
	player->processingDuck = false;
	DEBUG_PRINT_POST(Detour_Duck);
}

bool FASTCALL movement::Detour_LadderMove(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_LadderMove);
	Vector oldVelocity = mv->m_vecVelocity;
	MovementPlayer *player = g_pPlayerManager->ToPlayer(ms);
	MoveType_t oldMoveType = player->GetPawn()->m_MoveType();
	bool result = LadderMove(ms, mv);
	if (player->GetPawn()->m_lifeState() != LIFE_DEAD && !result && oldMoveType == MOVETYPE_LADDER)
	{
		// Do the setting part ourselves as well.
		utils::SetEntityMoveType(player->GetPawn(), MOVETYPE_WALK);
	}
	if (!result && oldMoveType == MOVETYPE_LADDER)
	{
		player->RegisterTakeoff(false);
		player->takeoffFromLadder = true;
		player->OnChangeMoveType(MOVETYPE_LADDER);
	}
	else if (result && oldMoveType != MOVETYPE_LADDER && player->GetPawn()->m_MoveType() == MOVETYPE_LADDER)
	{
		player->RegisterLanding(oldVelocity, false);
		player->OnChangeMoveType(MOVETYPE_WALK);
	}
	else if (result && oldMoveType == MOVETYPE_LADDER && player->GetPawn()->m_MoveType() == MOVETYPE_WALK)
	{
		// Player is on the ladder, pressing jump pushes them away from the ladder.
		float curtime = utils::GetServerGlobals()->curtime;
		player->RegisterTakeoff(player->IsButtonDown(IN_JUMP));
		player->takeoffFromLadder = true;
		player->OnChangeMoveType(MOVETYPE_LADDER);
	}
	DEBUG_PRINT_POST(Detour_LadderMove);
	return result;
}

void FASTCALL movement::Detour_CheckJumpButton(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_CheckJumpButton);
	CheckJumpButton(ms, mv);
	DEBUG_PRINT_POST(Detour_CheckJumpButton);
}

void FASTCALL movement::Detour_OnJump(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_OnJump);
	MovementPlayer *player = g_pPlayerManager->ToPlayer(ms);
	f32 oldJumpUntil = ms->m_flJumpUntil();
	MoveType_t oldMoveType = player->GetPawn()->m_MoveType();
	OnJump(ms, mv);
	if (ms->m_flJumpUntil() != oldJumpUntil)
	{
		player->hitPerf = (oldMoveType != MOVETYPE_LADDER && !player->oldWalkMoved);
		player->RegisterTakeoff(true);
		player->OnStopTouchGround();
	}
	DEBUG_PRINT_POST(Detour_OnJump);
}

void FASTCALL movement::Detour_AirAccelerate(CCSPlayer_MovementServices *ms, CMoveData *mv, Vector &wishdir, f32 wishspeed, f32 accel)
{
	DEBUG_PRINT(Detour_AirAccelerate);
	MovementPlayer *player = g_pPlayerManager->ToPlayer(ms);
	player->OnAirAcceleratePre(wishdir, wishspeed, accel);
	AirAccelerate(ms, mv, wishdir, wishspeed, accel);
	player->OnAirAcceleratePost(wishdir, wishspeed, accel);
	DEBUG_PRINT_POST(Detour_AirAccelerate);
}

void FASTCALL movement::Detour_Friction(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_Friction);
	Friction(ms, mv);
	DEBUG_PRINT_POST(Detour_Friction);
}

void FASTCALL movement::Detour_WalkMove(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_Friction);
	WalkMove(ms, mv);
	MovementPlayer *player = g_pPlayerManager->ToPlayer(ms);
	player->walkMoved = true;
	DEBUG_PRINT_POST(Detour_Friction);
}

void FASTCALL movement::Detour_TryPlayerMove(CCSPlayer_MovementServices *ms, CMoveData *mv, Vector *pFirstDest, trace_t_s2 *pFirstTrace)
{
	DEBUG_PRINT(Detour_TryPlayerMove);
	TryPlayerMove(ms, mv, pFirstDest, pFirstTrace);
	DEBUG_PRINT_POST(Detour_TryPlayerMove);
}

void FASTCALL movement::Detour_CategorizePosition(CCSPlayer_MovementServices *ms, CMoveData *mv, bool bStayOnGround)
{
	DEBUG_PRINT(Detour_CategorizePosition);
	MovementPlayer *player = g_pPlayerManager->ToPlayer(ms);
	Vector oldVelocity = mv->m_vecVelocity;
	bool oldOnGround = !!(player->GetPawn()->m_fFlags() & FL_ONGROUND);
	
	CategorizePosition(ms, mv, bStayOnGround);

	bool ground = !!(player->GetPawn()->m_fFlags() & FL_ONGROUND);

	if (oldOnGround != ground)
	{
		if (ground)
		{
			player->RegisterLanding(oldVelocity);
			player->duckBugged = player->processingDuck;
			player->OnStartTouchGround();
		}
		else
		{
			player->RegisterTakeoff(false);
			player->OnStopTouchGround();
		}
	}
	DEBUG_PRINT_POST(Detour_CategorizePosition);
}

void FASTCALL movement::Detour_FinishGravity(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_FinishGravity);
	FinishGravity(ms, mv);
	DEBUG_PRINT_POST(Detour_FinishGravity);
}

void FASTCALL movement::Detour_CheckFalling(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_CheckFalling);
	CheckFalling(ms, mv);
	DEBUG_PRINT_POST(Detour_CheckFalling);
}

void FASTCALL movement::Detour_PlayerMovePost(CCSPlayer_MovementServices *ms, CMoveData *mv)
{
	DEBUG_PRINT(Detour_PlayerMovePost);
	PlayerMovePost(ms, mv);
	DEBUG_PRINT_POST(Detour_PlayerMovePost);
}

void FASTCALL movement::Detour_PostThink(CCSPlayerPawnBase *pawn)
{
	DEBUG_PRINT(Detour_PostThink);
	MovementPlayer *player = g_pPlayerManager->ToPlayer(pawn);
	player->OnPostThink();
	PostThink(pawn);
	DEBUG_PRINT_POST(Detour_PostThink);
}
