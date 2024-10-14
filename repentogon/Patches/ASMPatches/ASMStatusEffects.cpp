#include "ASMPatcher.hpp"
#include "../ASMPatches.h"

#include "ASMStatusEffects.h"
#include "HookSystem.h"
#include "Log.h"

struct StatusEffectPatchInfo {
	char* signature;
	char* funcName;
	int sigOffset;
	ASMPatch::Registers entityReg; // pointer containing entity at the time of jmp
	ASMPatch::Registers targetReg; // register where entity pointer is expected to be after the patch
	ASMPatch::SavedRegisters::Registers saveReg; // its corresponding SavedRegisters::Register
	unsigned int jumpOffset;
};

const StatusEffectPatchInfo patches[16] = {
	{"e8????????84c074??8b8e????????85c974??0f1f44??008bf18b8e????????85c975??85d20f84????????8b86????????83e00183c8000f85????????3986????????0f8f????????8b8e????????85d27e??8d04??b92c0100003bc10f4cc8eb??f7da3bca0f4cca898e????????8d4c24??0f2805????????6a010f114424??6a000f57c0",
	"AddBaited", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x24},
	{"e8????????84c074??8b8e????????85c974??90",
	"AddBleeding", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x20},
	{"e8????????84c074??8b8e????????85c974??660f1f84??00000000",
	"AddBrimstoneMark", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x28},
	{"e8????????84c074??8b86????????85c074??0f1f80000000008bf08b86????????85c075??85d20f84????????8bbe????????8bc78b9e????????83e00183c8000f85????????3986????????0f8f????????8b8e????????85d27e??8d04??b9780000003bc10f4cc8eb??f7da3bca0f4cca8bc7898e????????2500100000",
	"AddBurn", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x26},
	{"8bf7e8????????84c074??8b8f????????85c974??8bf18b8e????????85c975??8b96",
	"AddCharmed", 0, ASMPatch::Registers::EDI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x21},
	{"e8????????84c074??8b8e????????85c974??0f1f84??000000008bf18b8e????????85c975??8b8e????????8bc1",
	"AddConfusion", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x27},
	{"e8????????84c074??8b8e????????85c974??0f1f40008bf18b8e????????85c975??85db",
	"AddFear", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x23},
	{"e8????????84c074??8b8e????????85c974??0f1f40008bf18b8e????????85c975??85d2",
	"AddFreeze", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x23},
	{"e8????????84c074??8b8e????????85c974??0f1f84??000000008bf18b8e????????85c975??8b8e????????85d2",
	"AddIce", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x27},
	{"8bcee8????????84c074??8b86????????85c074??90",
	"AddKnockback", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x22},
	{"e8????????84c074??8b8e????????85c974??0f1f44??008bf18b8e????????85c975??85d20f84????????8b86????????83e00183c8000f85????????3986????????0f8f????????8b8e????????85d27e??8d04??b92c0100003bc10f4cc8eb??f7da3bca0f4cca898e????????8d4c24??0f2805????????6a010f114424??6a000f2805",
	"AddMagnetized", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x24},
	{"e8????????84c074??8b8e????????85c974??0f1f44??008bf18b8e????????85c975??85ff",
	"AddMidasFreeze", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x24},

	// this crashed on me once, but now i can't replicate it
	{"e8????????84c074??8b86????????85c074??0f1f80000000008bf08b86????????85c075??85d20f84????????8bbe????????8bc78b9e????????83e00183c8000f85????????3986????????0f8f????????8b8e????????85d27e??8d04??b9780000003bc10f4cc8eb??f7da3bca0f4cca8bc7898e????????83e040",
	"AddPoison", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x26},

	{"8bf7e8????????84c074??8b8f????????85c974??8bf18b8e????????85c975??8b8e",
	"AddShrink", 0, ASMPatch::Registers::EDI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x21},
	{"e8????????84c074??8b8e????????85c974??0f1f00",
	"AddSlowing", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x22},
	{"e8????????84c074??8b8e????????85c974??8bf1",
	"AddWeakness", 0, ASMPatch::Registers::ESI, ASMPatch::Registers::ESI, ASMPatch::SavedRegisters::Registers::ESI, 0x1f},
};

/* /////////////////////
// shared GetStatusEffectTarget trampoline
*/ /////////////////////

Entity* __stdcall GetStatusEffectTargetTrampoline(Entity* target) {
	return target->GetStatusEffectTarget();
}

/* /////////////////////
// Generic inline patch
*/ /////////////////////

void ASMPatchInlinedGetStatusEffectTarget(void* addr, ASMPatch::Registers entityReg, ASMPatch::Registers targetReg, ASMPatch::SavedRegisters::Registers savedReg, unsigned int jumpOffset) {
	ASMPatch::SavedRegisters savedRegisters((ASMPatch::SavedRegisters::Registers::GP_REGISTERS | ASMPatch::SavedRegisters::XMM1) & ~savedReg, true);
	ASMPatch patch;
	patch.PreserveRegisters(savedRegisters)
		.Push(entityReg)
		.AddInternalCall(GetStatusEffectTargetTrampoline)
		.CopyRegister(targetReg, ASMPatch::Registers::EAX)
		.RestoreRegisters(savedRegisters)
		.AddRelativeJump((char*)addr + jumpOffset);
	sASMPatcher.PatchAt(addr, &patch);
}


/* /////////////////////
// Run patches
*/ /////////////////////

void PatchInlinedGetStatusEffectTarget()
{
	ZHL::Logger logger;
	for (const StatusEffectPatchInfo& i : patches) {
		SigScan scanner(i.signature);
		scanner.Scan();
		void* addr = (char*)scanner.GetAddress() + i.sigOffset;

		logger.Log("Patching inlined GetStatusEffectTarget in %s at %p\n", i.funcName, addr);
		ASMPatchInlinedGetStatusEffectTarget(addr, i.entityReg, i.targetReg, i.saveReg, i.jumpOffset);
	};
}
HOOK_METHOD(Entity, AddBaited, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}

HOOK_METHOD(Entity, AddBleeding, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}

HOOK_METHOD(Entity, AddBrimstoneMark, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}
HOOK_METHOD(Entity, AddBurn, (const EntityRef& ref, int duration, float damage) -> void) {
	super(ref, duration, damage);
}

HOOK_METHOD(Entity, AddCharmed, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}

HOOK_METHOD(Entity, AddConfusion, (const EntityRef& ref, int duration, bool ignoreBoss) -> void) {
	super(ref, duration, ignoreBoss);
}

HOOK_METHOD(Entity, AddFear, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}

HOOK_METHOD(Entity, AddFreeze, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}

HOOK_METHOD(Entity, AddKnockback, (const EntityRef& ref, const Vector& pushDirection, int duration, bool takeImpactDamage) -> void) {
	super(ref, pushDirection, duration, takeImpactDamage);
}

HOOK_METHOD(Entity, AddIce, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}

HOOK_METHOD(Entity, AddMagnetized, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}

HOOK_METHOD(Entity, AddMidasFreeze, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}

HOOK_METHOD(Entity, AddPoison, (const EntityRef& ref, int duration, float damage) -> void) {
	super(ref, duration, damage);
}

HOOK_METHOD(Entity, AddShrink, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}

HOOK_METHOD(Entity, AddSlowing, (const EntityRef& ref, int duration, float amount, ColorMod color) -> void) {
	super(ref, duration, amount, color);
}

HOOK_METHOD(Entity, AddWeakness, (const EntityRef& ref, int duration) -> void) {
	super(ref, duration);
}