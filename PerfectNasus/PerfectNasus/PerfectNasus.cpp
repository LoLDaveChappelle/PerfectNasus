// Lord's Malphite.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PluginSDK.h"
#include "Color.h"

PluginSetup("PerfectNasus - DaveChappelle");

IMenu* MainMenu;
IMenu* QMenu;
IMenu* WMenu;
IMenu* EMenu;
IMenu* RMenu;
IMenu* Misc;
IMenu* Drawings;
IMenuOption* ComboQ;
IMenuOption* AutoQ;
IMenuOption* HarassManaE;
IMenuOption* HarassManaW;
IMenuOption* FarmE;
IMenuOption* FarmEHit;
IMenuOption* HarassManaQ;
IMenuOption* FarmW;
IMenuOption* FarmQ;
IMenuOption* ComboW;
IMenuOption* QGapCloser;
IMenuOption* AutoE;
IMenuOption* AutoUlt;
IMenuOption* ComboE;
IMenuOption* ComboR;
IMenuOption* HealthPercent;
IMenuOption* UltEnemies;
IMenuOption* DrawReady;
IMenuOption* DrawQ;
IMenuOption* DrawW;
IMenuOption* DrawE;
IMenuOption* DrawR;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

void  Menu()
{
	MainMenu = GPluginSDK->AddMenu("PerfectNasus");
	QMenu = MainMenu->AddMenu("Q Settings");
	WMenu = MainMenu->AddMenu("W Settings");
	EMenu = MainMenu->AddMenu("E Settings");
	RMenu = MainMenu->AddMenu("R Settings");
	Drawings = MainMenu->AddMenu("Drawings");

	ComboQ = QMenu->CheckBox("Use Q", true);
	FarmQ = QMenu->CheckBox("Use Q Farm", true);
	HarassManaQ = QMenu->AddInteger("Mana Manager(%)(AutoQ)", 1, 100, 60);

	ComboW = WMenu->CheckBox("Use W", true);
	HarassManaW = WMenu->AddInteger("Mana Manager(%)(FarmW)", 1, 100, 60);

	AutoE = EMenu->CheckBox("Auto E", true);
	ComboE = EMenu->CheckBox("Use E", true);
	FarmE = EMenu->CheckBox("Use E Farm", true);
	FarmEHit = EMenu->AddInteger("Use E if hits >= x minions", 1, 10, 3);
	HarassManaE = EMenu->AddInteger("Mana Manager(%)(AutoE/FarmE)", 1, 100, 60);

	AutoUlt = RMenu->CheckBox("Auto R(if health below 25%", true);
	ComboR = RMenu->CheckBox("R When below % health ", true);
	HealthPercent = WMenu->AddInteger("Health Manager(%)", 1, 100, 60);

	DrawReady = Drawings->CheckBox("Draw Only Ready Spells", true);

	DrawQ = Drawings->CheckBox("Draw Q", true);
	DrawW = Drawings->CheckBox("Draw W", true);
	DrawE = Drawings->CheckBox("Draw E", true);
	DrawR = Drawings->CheckBox("Draw R", true);
}
void LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kTargetCast, false, false, static_cast<eCollisionFlags>(kCollidesWithYasuoWall));
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, static_cast<eCollisionFlags>(kCollidesWithNothing));
	E = GPluginSDK->CreateSpell2(kSlotE, kCircleCast, false, true, static_cast<eCollisionFlags>(kCollidesWithNothing));
	R = GPluginSDK->CreateSpell2(kSlotR, kCircleCast, false, false, static_cast<eCollisionFlags>(kCollidesWithNothing));
	Q->SetOverrideRange(GEntityList->Player()->AttackRange() + 50);
	W->SetOverrideRange(900);
	E->SetOverrideRange(650);
	R->SetOverrideRange(1000);
	R->SetOverrideRange(20);

}
void Combo()
{
	if (ComboQ->Enabled())
	{
		if (Q->IsReady())
		{
			auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
			Q->CastOnTarget(target, kHitChanceHigh);
		}
	}
	if (ComboW->Enabled())
	{
		if (W->IsReady())
		{
			auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
			W->CastOnTarget(target, kHitChanceHigh);
		}
	}
	if (ComboE->Enabled())
	{
		if (E->IsReady())
		{
			auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());
			E->CastOnTarget(target, kHitChanceHigh);
		}
	}
	if (ComboR->Enabled())
	{
		if (R->IsReady())
		{
			if (GEntityList->Player()->HealthPercent() <= HealthPercent->GetInteger() && ComboW->Enabled())
			{
				R->CastOnPlayer();
			}
		}
	}
}

void Qminion()
{
	if (!Q->IsReady())
		return;

	for (auto minion : GEntityList->GetAllMinions(false, true, true))
	{
		if (minion->GetTeam() != GEntityList->Player()->GetTeam()
			&& !minion->IsDead()
			&& minion->GetHealth() <= (GDamage->GetSpellDamage(GEntityList->Player(), minion, kSlotQ)
				+ GDamage->GetAutoAttackDamage(GEntityList->Player(), minion, true))
			&& GEntityList->Player()->IsValidTarget(minion, GEntityList->Player()->AttackRange()))
		{
			if (Q->CastOnPlayer())
			{
				GOrbwalking->SetOverrideTarget(minion);
				return;
			}
		}
	}
}

void Lasthit()
{
		if (GEntityList->Player()->ManaPercent() >= HarassManaQ->GetInteger() && FarmQ->Enabled() && GOrbwalking->GetOrbwalkingMode() == kModeLastHit)
		{
			Qminion();
		}
}

void Farm()
{
	if (GEntityList->Player()->ManaPercent() >= HarassManaQ->GetInteger() && FarmQ->Enabled() && GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
	{
		Qminion();
		if (GEntityList->Player()->ManaPercent() >= HarassManaE->GetInteger())
		{
			if (FarmE->Enabled())
			{
				int enemies = 0;
				Vec3 pos = Vec3();
				E->FindBestCastPosition(true, false, pos, enemies);
				if (enemies >= FarmEHit->GetInteger())
					E->CastOnPosition(pos);
			}
		}
	}
}
void Auto()
{
	if (GEntityList->Player()->ManaPercent() >= HarassManaQ->GetInteger() && AutoE->Enabled() && (GOrbwalking->GetOrbwalkingMode() == kModeNone || GOrbwalking->GetOrbwalkingMode() == kModeLaneClear || GOrbwalking->GetOrbwalkingMode() == kModeLastHit || GOrbwalking->GetOrbwalkingMode() == kModeMixed || GOrbwalking->GetOrbwalkingMode() == kModeFreeze || GOrbwalking->GetOrbwalkingMode() == kModeCustom))
	{
		E->CastOnTarget(GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range()));
	}
	if (GEntityList->Player()->ManaPercent() >= HarassManaQ->GetInteger() && AutoUlt->Enabled() && (GOrbwalking->GetOrbwalkingMode() == kModeNone || GOrbwalking->GetOrbwalkingMode() == kModeLaneClear || GOrbwalking->GetOrbwalkingMode() == kModeLastHit || GOrbwalking->GetOrbwalkingMode() == kModeMixed || GOrbwalking->GetOrbwalkingMode() == kModeFreeze || GOrbwalking->GetOrbwalkingMode() == kModeCustom || GOrbwalking->GetOrbwalkingMode() == kModeCombo))
	{
		if (GEntityList->Player()->HealthPercent() <= HealthPercent->GetInteger() && ComboW->Enabled())
		{
			R->CastOnPlayer();
		}
	}
}
PLUGIN_EVENT(void) OnGameUpdate()
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		Combo();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
	{
		Farm();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLastHit)
	{
		Qminion();
	}
	Auto();

}
PLUGIN_EVENT(void) OnGapCloser(GapCloserSpell const& Args)
{
	if (Args.Sender != GEntityList->Player()
		&& Args.Sender->GetTeam() != GEntityList->Player()->GetTeam()
		&& GEntityList->Player()->IsValidTarget(Args.Sender, Q->Range())
		&& QGapCloser->Enabled() && E->IsReady())
	{
		Q->CastOnUnit(Args.Sender);
	}
}
PLUGIN_EVENT(void) OnRender()
{
	if (DrawReady->Enabled())
	{
		if (Q->IsReady() && DrawQ->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range()); }

		if (E->IsReady() && DrawE->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }

		if (W->IsReady() && DrawW->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range()); }

		if (R->IsReady() && DrawW->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range()); }

	}
	else
	{
		if (DrawQ->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range()); }

		if (DrawE->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }

		if (DrawW->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range()); }

		if (DrawR->Enabled()) { GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range()); }
	}
}
PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{

	PluginSDKSetup(PluginSDK);
	Menu();
	LoadSpells();


	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnGapCloser, OnGapCloser);
	GRender->NotificationEx(Color::DarkMagenta().Get(), 2, true, true, "PerfectNasus V0.1 - Loaded");
}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();


	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnGapCloser, OnGapCloser);

}