#ifndef EVENTS_H
#define EVENTS_H

#include "dobject.h"
#include "serializer.h"

class DStaticEventHandler;

// register
bool E_RegisterHandler(DStaticEventHandler* handler);
// unregister
bool E_UnregisterHandler(DStaticEventHandler* handler);
// find
bool E_CheckHandler(DStaticEventHandler* handler);
// check type
bool E_IsStaticType(PClass* type);
// init static handlers
void E_InitStaticHandlers(bool map);

// called right after the map has loaded (approximately same time as OPEN ACS scripts)
void E_WorldLoaded();
// called when the map is about to unload (approximately same time as UNLOADING ACS scripts)
void E_WorldUnloaded();
// called right after the map has loaded (every time, UNSAFE VERSION)
void E_WorldLoadedUnsafe();
// called right before the map is unloaded (every time, UNSAFE VERSION)
void E_WorldUnloadedUnsafe();
// called around PostBeginPlay of each actor.
void E_WorldThingSpawned(AActor* actor);
// called after AActor::Die of each actor.
void E_WorldThingDied(AActor* actor, AActor* inflictor);
// called after AActor::Revive.
void E_WorldThingRevived(AActor* actor);
// called before P_DamageMobj and before AActor::DamageMobj virtuals.
void E_WorldThingDamaged(AActor* actor, AActor* inflictor, AActor* source, int damage, FName mod, int flags, DAngle angle);
// called before AActor::Destroy of each actor.
void E_WorldThingDestroyed(AActor* actor);
// same as ACS SCRIPT_Lightning
void E_WorldLightning();
// this executes on every tick, before everything
void E_WorldTick();
// called on each render frame once.
void E_RenderFrame();

// serialization stuff
void E_SerializeEvents(FSerializer& arc);

// ==============================================
//
//  EventHandler - base class
//
// ==============================================

class DStaticEventHandler : public DObject // make it a part of normal GC process
{
	DECLARE_CLASS(DStaticEventHandler, DObject)
public:
	DStaticEventHandler()
	{
		prev = 0;
		next = 0;
		order = 0;
		haveorder = false;
	}

	DStaticEventHandler* prev;
	DStaticEventHandler* next;
	virtual bool IsStatic() { return true; }

	// order is cached to avoid calling the VM for sorting too much
	int order;
	bool haveorder;

	// serialization handler. let's keep it here so that I don't get lost in serialized/not serialized fields
	void Serialize(FSerializer& arc) override
	{
		Super::Serialize(arc);
		if (arc.isReading())
		{
			Printf("DStaticEventHandler::Serialize: reading object %s\n", GetClass()->TypeName.GetChars());
		}
		else
		{
			Printf("DStaticEventHandler::Serialize: store object %s\n", GetClass()->TypeName.GetChars());
		}
		/* do nothing */
	}

	// destroy handler. this unlinks EventHandler from the list automatically.
	void OnDestroy() override;

	virtual void WorldLoaded();
	virtual void WorldUnloaded();
	virtual void WorldThingSpawned(AActor*);
	virtual void WorldThingDied(AActor*, AActor*);
	virtual void WorldThingRevived(AActor*);
	virtual void WorldThingDamaged(AActor*, AActor*, AActor*, int, FName, int, DAngle);
	virtual void WorldThingDestroyed(AActor*);
	virtual void WorldLightning();
	virtual void WorldTick();
	virtual void RenderFrame();

	// gets the order of this item.
	int GetOrder();
};
class DEventHandler : public DStaticEventHandler
{
	DECLARE_CLASS(DEventHandler, DStaticEventHandler) // TODO: make sure this does not horribly break anything
public:
	bool IsStatic() override { return false; }
};
extern DStaticEventHandler* E_FirstEventHandler;

// we cannot call this DEvent because in ZScript, 'event' is a keyword
class DBaseEvent : public DObject
{
	DECLARE_CLASS(DBaseEvent, DObject)
public:

	DBaseEvent()
	{
		// each type of event is created only once to avoid new/delete hell
		// since from what I remember object creation and deletion results in a lot of GC processing
		// (and we aren't supposed to pass event objects around anyway)
		this->ObjectFlags |= OF_Fixed;
		// we don't want to store events into the savegames because they are global.
		this->ObjectFlags |= OF_Transient;
	}
};

class DRenderEvent : public DBaseEvent
{
	DECLARE_CLASS(DRenderEvent, DBaseEvent)
public:
	// these are for all render events
	DVector3 ViewPos;
	DAngle ViewAngle;
	DAngle ViewPitch;
	DAngle ViewRoll;
	double FracTic; // 0..1 value that describes where we are inside the current gametic, render-wise.
	AActor* Camera;

	DRenderEvent()
	{
		FracTic = 0;
		Camera = nullptr;
	}
};

class DWorldEvent : public DBaseEvent
{
	DECLARE_CLASS(DWorldEvent, DBaseEvent)
public:
	// for loaded/unloaded
	bool IsSaveGame;
	// for thingspawned, thingdied, thingdestroyed
	AActor* Thing;
	// for thingdied
	AActor* Inflictor; // can be null
	// for damagemobj
	int Damage;
	AActor* DamageSource; // can be null
	FName DamageType;
	int DamageFlags;
	DAngle DamageAngle;

	DWorldEvent()
	{
		IsSaveGame = false;
		Thing = nullptr;
		Inflictor = nullptr;
		Damage = 0;
		DamageSource = nullptr;
		DamageFlags = 0;
	}
};

#endif