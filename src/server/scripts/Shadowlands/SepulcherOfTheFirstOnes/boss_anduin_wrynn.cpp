/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "DB2Stores.h"
#include "CellImpl.h"
#include "TaskScheduler.h"
#include "CreatureAI.h"
#include "CreatureAIImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "G3DPosition.hpp"
#include "MoveSplineInitArgs.h"
#include <G3D/Box.h>
#include <G3D/CoordinateFrame.h>
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MovementTypedefs.h"
#include "Object.h"
#include "ObjectAccessor.h"
#include "PathGenerator.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "TemporarySummon.h"
#include "UpdateFields.h"
#include "G3D/Vector2.h"
#include "Unit.h"
#include "Weather.h"
#include "GameObjectAI.h"
#include "Containers.h"
#include "SpellMgr.h"
#include "ScriptedGossip.h"
#include "sepulcher_of_the_first_ones.h"

enum AnduinWrynnSpells
{
    // Pre-Introduction
    SPELL_BROKER_SPAWN                              = 367524,

    // Generic Spells
    SPELL_ANDUIN_PLUNGE_KINGSMOURNE                 = 369125,
    SPELL_WILLPOWER_DISPLAY                         = 365177,
    SPELL_WILLPOWER_ENERGIZE_SMALL                  = 365217,
    SPELL_WILLPOWER_ENERGIZE_LARGE                  = 365228,
    SPELL_SOUL_DESPAWN                              = 362766,
    SPELL_SHADESTEP                                 = 363976,
    SPELL_ANDUIN_PROGRESSION_AURA                   = 369317,
    SPELL_ANDUIN_WILLPOWER_PERIODIC                 = 366848,
    SPELL_ANDUIN_SOUL_GHOST                         = 369016,

    // Dark Zeal
    SPELL_DARK_ZEAL_AURA                            = 364247,
    SPELL_DARK_ZEAL_BUFF                            = 364248,

    // Hopebreaker
    SPELL_HOPEBREAKER                               = 361815,
    SPELL_HOPEBREAKER_DAMAGE                        = 361816,
    SPELL_HOPEBREAKER_DEBUFF                        = 361817,
    SPELL_HOPEBREAKER_DEBUFF_DAMAGE                 = 361818,
    SPELL_HOPEBREAKER_CLEAR                         = 364237,

    // Domination Word: Pain
    SPELL_DOMINATION_WORD_PAIN                      = 366849,

    // Befouled Barrier
    SPELL_BEFOULED_BARRIER                          = 365295,
    SPELL_BEFOULED_BARRIER_BLACK_RING               = 365633,
    SPELL_BEFOULED_ERUPTION                         = 365853,
    SPELL_BEFOULED_BARRIER_SPHERE_AREATRIGGER       = 365173,
    SPELL_BEFOULED_BARRIER_DEBUFF                   = 365293,
    SPELL_BEFOULED_BARRIER_CLEAR                    = 369871,
    SPELL_BEFOULED_BARRIER_EXPLODE                  = 365300,

    // Blasphemy
    SPELL_BLASPHEMY                                 = 361989,
    SPELL_BLASPHEMY_PRE_HIT                         = 364239,
    SPELL_BLASPHEMY_OVERCONFIDENCE                  = 361990,
    SPELL_BLASPHEMY_OVERCONFIDENCE_AREATRIGGER      = 361992,
    SPELL_BLASPHEMY_HOPELESSNESS                    = 361991,
    SPELL_BLASPHEMY_HOPELESSNESS_AREATRIGGER        = 361993,
    SPELL_BLASPHEMY_SUCCESS                         = 362014,
    SPELL_BLASPHEMY_EXPLODE                         = 361999,
    SPELL_BLASPHEMY_EXPLODE_LFR_NORMAL              = 366933,
    SPELL_CANCEL_BLASPHEMY                          = 370161,
    SPELL_BLASPHEMY_IMMUNE                          = 370407,

    // Kingsmourne Hungers
    SPELL_KINGSMOURNE_HUNGERS                       = 362405,
    SPELL_KINGSMOURNE_HUNGERS_DAMAGE_IDK            = 362406,
    SPELL_KINGSMOURNE_HUNGERS_DAMAGE                = 362407,
    SPELL_LOST_SOUL_DIMENSION                       = 362055,
    SPELL_LOST_SOUL                                 = 362472,
    SPELL_LOST_SOUL_GRACE                           = 370068,
    SPELL_LOST_SOUL_CLEAR                           = 365641,
    SPELL_CANCEL_LOST_SOUL                          = 367774,
    SPELL_SEVERED_SOUL                              = 367769,
    SPELL_LOST_SOUL_MYTHIC                          = 367770,
    SPELL_LOST_SOUL_MIRROR_IMAGE                    = 362402,
    SPELL_SCARRED_SOUL                              = 365445,
    SPELL_MIRROR_IMAGE                              = 362474,
    SPELL_FEIGN_DEATH                               = 114371,
    SPELL_BANISH_SOUL                               = 367771,
    SPELL_MIRROR_IMAGE_IGNORE_PHASE_SHIFT           = 362473,
    SPELL_ANDUIN_LOST_SOUL_TRACKER                  = 369843,

    // Rain of Despair (Big add)
    SPELL_RAIN_OF_DESPAIR                           = 362391,
    SPELL_ANDUIN_SOUL_DESPAIR                       = 365220,
    SPELL_RAIN_OF_DESPAIR_MELEE                     = 362393,
    SPELL_RAIN_OF_DESPAIR_RANGED                    = 362396,
    SPELL_RAIN_OF_DESPAIR_EXPLOSION                 = 362392,

    // Wicked Star
    SPELL_WICKED_STAR                               = 365030,
    SPELL_WICKED_STAR_POINTER                       = 365021,
    SPELL_WICKED_STAR_AREATRIGGER                   = 365017,
    SPELL_WICKED_STAR_DAMAGE_SILENCE                = 365024,
    SPELL_WICKED_STAR_EMPOWERMENT                   = 365112,
    SPELL_WICKED_STAR_IDK                           = 365992,
    SPELL_WICKED_STAR_TARGETED                      = 366674,
    SPELL_WICKED_STAR_IDK_3                         = 369280,

    // Empowered Wicked Star
    SPELL_EMPOWERED_WICKED_STAR                     = 367631,
    SPELL_EMPOWERED_WICKED_STAR_POINTER             = 367632,
    SPELL_EMPOWERED_WICKED_STAR_AREATRIGGER         = 367621,
    SPELL_EMPOWERED_WICKED_STAR_DAMAGE_SILENCE      = 367634,

    // Force of Will
    SPELL_FORCE_OF_WILL                             = 368913,
    SPELL_CANCEL_FORCE_OF_WILL                      = 368978,

    // Fiendish Soul
    SPELL_NECROTIC_CLAWS_LEAP                       = 363019,
    SPELL_NECROTIC_CLAWS_DEBUFF                     = 363020,
    SPELL_SOUL_EXPLOSION_TARGET                     = 363029,
    SPELL_SOUL_EXPLOSION_TRIGGER_MISSILE            = 363030,
    SPELL_SOUL_EXPLOSION_DAMAGE                     = 363031,

    // Monstrous Soul
    SPELL_UNRAVELING_FRENZY_PERIODIC                = 363027,
    SPELL_UNRAVELING_FRENZY                         = 363028,
    SPELL_NECROTIC_DETONATION                       = 363024,

    // Grim Reflections
    SPELL_GRIM_REFLECTIONS                          = 365120,
    SPELL_GRIM_REFLECTIONS_SUMMON                   = 365121,
    SPELL_GRIM_REFLECTIONS_DEST_SUMMON              = 365039,
    SPELL_CALAMITY_STATE_VISUAL                     = 361685,
    SPELL_WICKED_STAR_PROTECTION                    = 370400,
    SPELL_PSYCHIC_TERROR                            = 365008,
    SPELL_GRIM_FATE                                 = 367932,

    // Beacon of Hope
    SPELL_BEACON_OF_HOPE                            = 365872,
    SPELL_BEACON_OF_HOPE_AREATRIGGER                = 362702,

    // Fragment of Hope
    SPELL_FRAGMENT_OF_HOPE_AREATRIGGER              = 365816,
    SPELL_FRAGMENT_OF_HOPE_CLEAR_DEBUFF             = 365828,
    SPELL_FRAGMENT_OF_HOPE_DAMAGE                   = 365990,

    // Purging Light
    SPELL_PURGING_LIGHT                             = 368428,

    // Hopelessness
    SPELL_HOPELESSNESS                              = 365958,
    SPELL_HOPELESSNESS_MISSILE                      = 365962,
    SPELL_HOPELESSNESS_HOPELESSNESS_AREATRIGGER     = 365966,
    SPELL_HOPELESSNESS_EXPLODE                      = 365971,

    // Empowered Hopebreaker
    SPELL_EMPOWERED_HOPEBREAKER                     = 365805,
    SPELL_EMPOWERED_HOPEBREAKER_EXPLOSION           = 365806,

    // Anduin's Soul
    SPELL_LOST_SOUL_PERIODIC                        = 365650,
    SPELL_LOST_SOUL_CONSUME                         = 365652,

    // Anduin's Hope
    SPELL_ANDUIN_SLOW                               = 365218,
    SPELL_GLOOM                                     = 364031,

    // Anduin's Doubt
    SPELL_GHOST_VISUAL_COSMETIC                     = 370833,

    // Remnant of a Fallen King
    SPELL_DOMINATION_GRASP                          = 365216,
    SPELL_DOMINATION_GRASP_ROOT_AREATRIGGER         = 362505,
    SPELL_SHADE_VISUAL                              = 362490,
    SPELL_REMNANT_SPAWN                             = 362500,
    SPELL_DARK_PRESENCE                             = 368986,
    SPELL_WEATHER_COSMETIC                          = 362493,
    SPELL_SPAWN_REMNANT                             = 362497,
    SPELL_REMNANT_TIMER                             = 365291,
    SPELL_RETURN_TO_KINGSMOURNE                     = 363022,
    SPELL_RETURN_TO_KINGSMOURNE_VISUALS             = 363021,
    SPELL_SHADE_DESPAWN_CEREMONY                    = 363023,

    // Remorseless Winter
    SPELL_REMORSELESS_WINTER                        = 362542,
    SPELL_REMORSELESS_WINTER_PERIODIC               = 362543,
    SPELL_REMORSELESS_WINTER_DEBUFF_DAMAGE          = 362545,
    SPELL_REMORSELESS_WINTER_CLEAR                  = 370619,

    // Army of the Dead
    SPELL_ARMY_OF_THE_DEAD                          = 362862,
    SPELL_ECHOES_OF_ANDORHAL                        = 362863,
    SPELL_ECHOES_OF_ANDORHAL_FIENDISH_GHOULS        = 362864,
    SPELL_ECHOES_OF_ANDORHAL_MONSTROUS_SOUL         = 363025,

    // March of the Damned
    SPELL_MARCH_OF_THE_DAMNED                       = 363116,
    SPELL_MARCH_OF_THE_DAMNED_PERIODIC              = 363233,
    SPELL_MARCH_OF_THE_DAMNED_AREATRIGGER           = 363133,
    SPELL_MARCH_OF_THE_DAMNED_DAMAGE                = 364020,
    SPELL_DESPAWN_WALLS                             = 371694,

    // Soul Reaper
    SPELL_SOUL_REAPER                               = 362771,
    SPELL_SOUL_REAPER_PHYSICAL_DAMAGE               = 362772,
    SPELL_SOUL_REAPER_SHADOWFROST_DAMAGE            = 362773,
    SPELL_SOUL_REAPER_DEBUFF                        = 362774,
    SPELL_SOUL_REAPER_ATTACK_SPEED                  = 362775,

    // Finish Encounter
    SPELL_ANDUIN_KNEEL_POSE                         = 369367,
    SPELL_FINAL_MOVIE                               = 367306,
    SPELL_AWARD_ANDUIN_KILL                         = 359476,

    // Berserk
    SPELL_BERSERK                                   = 26662,

    // Jaina Spells
    SPELL_BLINK                                     = 362844,
    SPELL_FROSTBOLT                                 = 362843,
    SPELL_GENERIC_BLINK                             = 363984,

    // Sylvanas Spells
    SPELL_SHOOT_BOW                                 = 364068,
    SPELL_TUMBLE                                    = 364069,

    // Uther Spells
    SPELL_BLADE_OF_JUSTICE                          = 363971,
    SPELL_UTHER_CHARGE                              = 363972,

    // Translocators Teleport
    SPELL_TELEPORT_COSMIC_HUB                       = 364475,
    SPELL_TELEPORT_DOMINATIONS_GRASP                = 368563,
};

enum AnduinWrynnPhases
{
    PHASE_ONE = 1,
    PHASE_TWO,
    PHASE_THREE,
};

enum AnduinWrynnData
{
    DATA_REQUIRED_HEALING = 1,
    DATA_ABSORBED_HEALING,
};

enum AnduinWrynnEvents
{
    // Anduin Wrynn
    EVENT_HOPEBREAKER = 1,
    EVENT_DOMINATION_WORD_PAIN,
    EVENT_BEFOULED_BARRIER,
    EVENT_UPDATE_BEFOULED_BARRIER,
    EVENT_BLASPHEMY,
    EVENT_HOPELESSNESS,
    EVENT_WICKED_STAR,
    EVENT_EMPOWERED_WICKED_STAR,
    EVENT_KINGSMOURNE_HUNGERS,
    EVENT_INTERMISSION_ONE,
    EVENT_INTERMISSION_TWO,
    EVENT_GRIM_REFLECTIONS,
    EVENT_BEACON_OF_HOPE,
    EVENT_EMPOWERED_HOPEBREAKER,
    EVENT_SUMMON_ARTHAS,
    EVENT_NPC_DESPAWN,
    EVENT_BANISH_SOUL,
    EVENT_BERSERK,
    EVENT_INTRODUCTION_FINISH,

    // Kingsmourne Room
    EVENT_ANDUIN_SOUL,
    EVENT_ESCAPE_PLATFORM,
    EVENT_DESPAWN_SELF,

    // Remnant of a Fallen King
    EVENT_REMNANT_INTERMISSION_ONE,
    EVENT_REMNANT_INTERMISSION_TWO,
    EVENT_ARMY_OF_THE_DEAD,
    EVENT_SOUL_REAPER,
    EVENT_RETURN_TO_KINGSMOURNE,
    EVENT_CANCEL_EVENTS,

    // Fiendish Soul
    EVENT_GHOUL_LEAP,
    EVENT_NECROTIC_CLAWS,

    // Monstrous Soul
    EVENT_UNRAVELING_FRENZY,
    EVENT_NECROTIC_DETONATION,

    // Grim Reflections
    EVENT_PSYCHIC_TERROR,
    EVENT_GRIM_REFLECTION_IMMUNITY,

    // Sylvanas Windrunner
    EVENT_CANCEL_SYLVANAS_EVENTS,
    EVENT_TUMBLE,

    // Uther the Lightbringer
    EVENT_CANCEL_UTHER_EVENTS,
    EVENT_BLADE_OF_JUSTICE,

    // Jaina Proudmoore
    EVENT_CANCEL_JAINA_EVENTS,
    EVENT_FROSTBOLT,
    EVENT_BLINK,
};

enum AnduinWrynnActions
{
    ACTION_START_PRE_INTRODUCTION = 1,
    ACTION_START_MOVEMENT,
    ACTION_START_INTRODUCTION,
    ACTION_FINISH_INTRODUCTION,
    ACTION_CANCEL_EVENTS,
    ACTION_CANCEL_EVENTS_ASSISTERS,
    ACTION_ARTHAS_INTERMISSION_UTHER,
    ACTION_ARTHAS_INTERMISSION_SYLVANAS,
    ACTION_EXIT_INTERMISSION,
    ACTION_ACTIVATE_REMNANT,
    ACTION_DESPAWN_REMNANT,
    ACTION_SUMMON_KINGSMOURNE_SOULS,
    ACTION_END_ENCOUNTER,
    ACTION_START_OUTRODUCTION,
    ACTION_DESPAIR_GONE,
    ACTION_DOUBT_GONE,
    ACTION_HOPE_RESTORED,

    // Fiendish Soul
    ACTION_SOUL_EXPLOSION,

    // Monstrous Soul
    ACTION_NECROTIC_DETONATION,
};

enum AnduinWrynnTexts
{
    SAY_AGGRO                           = 0,
    SAY_HOPEBREAKER                     = 1,
    SAY_BEFOULED_BARRIER                = 2,
    SAY_BLASPHEMY                       = 3,
    SAY_ANNOUNCE_BLASPHEMY              = 4,
    SAY_ANNOUNCE_KINGSMOURNE_HUNGERS    = 5,
    SAY_KINGSMOURNE_HUNGERS             = 6,
    SAY_WICKED_STAR                     = 7,
    SAY_ANNOUNCE_WICKED_STAR            = 8,
    SAY_GRIM_REFLECTIONS                = 9,
    SAY_SLAY                            = 10,
    SAY_EMPOWERED_HOPEBREAKER           = 11,
    SAY_DISENGAGE                       = 12,
    SAY_NECROTIC_DETONATION             = 13,
    SAY_ANNOUNCE_EMPOWERED_WICKED_STAR  = 14,
};

enum AnduinWrynnConversations
{
    CONVERSATION_INTRO                  = 17835,
    CONVERSATION_ARTHAS_UTHER           = 17921,
    CONVERSATION_ARTHAS_SYLVANAS        = 17923,
    CONVERSATION_ANDUIN_PHASE_THREE     = 17924,
    CONVERSATION_ANDUIN_OUTRODUCTION    = 17836,
};

enum AnduinWrynnSpawnGroups
{
    SPAWN_GROUP_INITIAL         = 0
};

enum AnduinWrynnPoints
{
    POINT_START_INTRODUCTION    = 1,
    POINT_ANDUIN_SOUL           = 2,
    POINT_ESCAPE_PLATFORM       = 3,
    POINT_MARCH_OF_THE_DAMNED   = 4,
};

enum AnduinWrynnPaths
{
    WAYPOINT_JAINA      = 8,
    WAYPOINT_UTHER      = 9,
    WAYPOINT_FIRIM      = 10,
    WAYPOINT_SYLVANAS   = 13,

    PATH_INTRODUCTION_JAINA     = 1836640,
    PATH_INTRODUCTION_UTHER     = 1836650,
    PATH_INTRODUCTION_SYLVANAS  = 1836660,
    PATH_OUTRODUCTION_FIRIM     = 1845890,
};

enum AnduinWrynnSpellVisuals
{
    SPELL_VISUAL_CHEST_LOOT     = 114023,
};

enum AnduinWrynnAchievements
{
    ACHIEVEMENT_ANDUIN_MYTHIC    = 15486,
};

Position const PreIntroductionAssistersPositions[3] =
{
    { -3826.9548f, -2626.2761f, 78.9296f, 4.644121f }, // Jaina
    { -3831.6807f, -2626.2761f, 78.9296f, 5.460620f }, // Uther
    { -3818.7300f, -2626.2800f, 78.9296f, 4.558697f }, // Sylvanas
};

Position const AssistersSpawnPos[3] =
{
    { -3824.65f, -2692.20f, 91.3485f, 4.64412f}, // Jaina
    { -3828.03f, -2688.25f, 91.3485f, 5.46062f}, // Uther
    { -3819.48f, -2687.19f, 91.3485f, 4.55870f}, // Sylvanas
};

Position const FirimOutroductionPos = { -3830.0156f, -2676.7969f, 91.56402f };

Position const DominationGraspCenter = { -3825.0601f, -2715.4600f, 91.3567f, 1.6260f };

Position const IntermissionAssistersTeleportPosition[3] =
{
    { -3828.472f, -2688.191f, 91.2652f, 1.9153f }, // Sylvanas
    { -3819.519f, -2687.170f, 91.2652f, 2.1812f }, // Uther
    { -3824.960f, -2692.550f, 91.2652f, 1.5733f }, // Jaina
};

Position const AnduinsDespairSpawnPosition = { -3828.355957f, -2704.1875f, 91.350716f, 4.982021f };

Position const AnduinsSoulSpawnPosition = { -3825.060059f, -2715.459961f, 91.356697f, 1.626040f };

Position const AnduinsDoubtSpawnPositions[4] =
{
    {  -3852.638916f, -2687.373291f, 91.348526f, 5.560700f }, // Right of Uther
    {  -3852.845459f, -2742.732666f, 91.348534f, 0.961583f }, // Next Right
    {  -3797.704834f, -2686.685791f, 91.348526f, 3.804689f }, // Left of Sylvanas
    {  -3799.805664f, -2740.925293f, 91.348541f, 2.247305f }, // Next Left
};

Position const AnduinsHopeSpawnPosition[4] =
{
    {  -3825.149414f, -2711.508789f, 91.354919f, 1.463445f }, // In front of Anduin
    {  -3828.751709f, -2715.171875f, 91.354919f, 3.221487f }, // Left of Anduin
    {  -3821.041748f, -2715.296875f, 91.354889f, 0.046978f }, // Right of Anduin
    {  -3825.180664f, -2719.208252f, 91.354820f, 4.741285f }, // Behind Anduin
};

Position const RemnantOfAFallenKingSpawnPosition = { -3825.2466f, -2700.0486f, 91.3650f, 1.3762f };

Position const GrimReflectionsSpawnPositions[4] =
{
    { -3825.389f, -2739.4202f, 91.431305f, 4.8445f}, // back side of Anduin
    { -3849.8438f, -2715.0574f, 91.40953f, 2.9961f}, // left side of Anduin
    { -3825.4966f, -2692.6199f, 91.487495f, 1.4654f}, // in front of Anduin
    { -3800.6980f, -2715.4340f, 91.390780f, 6.2769f}, // right side of Anduin
};

Position const MarchOfTheDamnedSpawnPositions[8] =
{
    { -3839.8489f, -2679.7708f, 91.53031f, 5.1081f }, // First right
    { -3860.8940f, -2701.0051f, 91.53032f, 5.9010f }, // Second Right
    { -3860.5696f, -2729.7344f, 91.53032f, 0.3886f }, // Third Right
    { -3839.6390f, -2750.8890f, 91.53032f, 1.1935f }, // Fourth Right
    { -3811.0625f, -2679.6528f, 91.53031f, 4.3362f }, // First left
    { -3789.8125f, -2700.5088f, 91.53032f, 3.5383f }, // Second Left
    { -3789.2812f, -2729.3160f, 91.53032f, 2.7669f }, // Third Left
    { -3810.4290f, -2751.0903f, 91.53032f, 1.9584f } // Fourth Left
};

Position const BeaconOfHopeSpawnPosition = { -3825.0417f, -2715.3923f, 91.3568f, 0.0f };

Position const ChestLootSpawnPosition = { -3840.9915f, -2741.7847f, 91.26521f, 1.334929f };

template<typename T>
struct Timers : public std::list<T>
{
    Timers() = default;

    Timers(const std::initializer_list<T>& _list) :
        std::list<T>(_list) {};

    T nextTimer()
    {
        if (this->empty())
            throw std::logic_error("Timers is empty!");

        T timer = this->front();
        if (this->size() > 1)
            this->pop_front();

        return timer;
    }
};

Timers<uint32> HopeBreakerP1{ 31900, 28000, 29900, 29900 };
Timers<uint32> HopeBreakerP2{ 24900, 32900, 29000, 28900 };
Timers<uint32> BefouledBarrierTimerP1{ 51900, 48000 };
Timers<uint32> BefouledBarrierTimerP2{ 47000 };
Timers<uint32> BlasphemyAll{ 49900, 54900 };
Timers<uint32> DominationWordPainP1{ 13000, 13000, 10000, 15000, 13100, 12900, 13000, 13900, 12200, 14800 };
Timers<uint32> DominationWordPainP2{ 13000, 13000, 17700, 8100, 13000, 13000, 14400, 11200, 12200 };
Timers<uint32> WickedStarP1{ 34900, 29900 };
Timers<uint32> WickedStarP2{ 39000, 25900, 30800, 19100 };

class ActivateGhouls : public BasicEvent
{
public:
    ActivateGhouls(Creature* summoner, Creature* owner)
        : _summonerGuid(summoner->GetGUID()), _owner(owner) { }

    bool Execute(uint64 /*time*/, uint32 /*diff*/) override
    {
        if (Creature* _summoner = ObjectAccessor::GetCreature(*_owner, _summonerGuid))
        {
            if (Unit* target = _summoner->AI()->SelectTarget(SelectTargetMethod::Random, 0))
            {
                _owner->SetReactState(REACT_AGGRESSIVE);
                _owner->AI()->AttackStart(target);
            }
        }
        return true;
    }

private:
    ObjectGuid _summonerGuid;
    Creature* _owner;
};

// Anduin Wrynn - 181954
struct boss_anduin_wrynn : public BossAI
{
    boss_anduin_wrynn(Creature* creature) : BossAI(creature, DATA_ANDUIN_WRYNN),
        _slayTextOnCooldown(false), _dominationWordCount(0), _intermissionsDone(0), _encounterEnded(false)
    {
        Initialize();
    }

    void Initialize()
    {
        _slayTextOnCooldown = false;
        _dominationWordCount = 0;
        _intermissionsDone = 0;
        _encounterEnded = false;
        me->SetFaction(FACTION_MONSTER);
    }

    void HandleIntroduction()
    {
        if (instance->GetData(DATA_ANDUIN_WRYNN_INTRODUCTION) == DONE)
        {
            me->RemoveUnitFlag(UNIT_FLAG_NOT_ATTACKABLE_1);
            me->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
            me->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC);
            me->SetImmuneToAll(false);
            me->SetSpeed(MOVE_RUN, 11.0f);
            me->RemoveAurasDueToSpell(SPELL_ANDUIN_PLUNGE_KINGSMOURNE);
        }
        else
        {
            DoCastSelf(SPELL_ANDUIN_PLUNGE_KINGSMOURNE);
            me->SetUnitFlag(UNIT_FLAG_NOT_ATTACKABLE_1);
            me->SetUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
            me->SetUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC);
            me->SetImmuneToAll(true);
        }
    }

    void PrepareAssistersForIntermission()
    {
        Creature* sylvanas = instance->GetCreature(DATA_SYLVANAS_WINDRUNNER);
        Creature* uther = instance->GetCreature(DATA_UTHER_THE_LIGHTBRINGER);
        Creature* jaina = instance->GetCreature(DATA_JAINA_PROUDMOORE);

        if (!sylvanas || !uther || !jaina)
            return;

        sylvanas->CastSpell(IntermissionAssistersTeleportPosition[0], SPELL_GENERIC_BLINK, false);
        uther->CastSpell(IntermissionAssistersTeleportPosition[1], SPELL_GENERIC_BLINK, false);
        jaina->CastSpell(IntermissionAssistersTeleportPosition[2], SPELL_GENERIC_BLINK, false);
        sylvanas->AttackStop();
        uther->AttackStop();
        jaina->AttackStop();
        sylvanas->SetSheath(SHEATH_STATE_RANGED);
        jaina->SetSheath(SHEATH_STATE_RANGED);
        uther->SetSheath(SHEATH_STATE_MELEE);
        sylvanas->SetEmoteState(EMOTE_STATE_READY_BOW);
        jaina->SetEmoteState(EMOTE_STATE_READY2H);
        uther->SetEmoteState(EMOTE_STATE_READY2H);
        // @TODO: assisters should attack remnant after teleport (as soon as it spawns)
    }

    void ForceAssistersToAttackRemnant()
    {
        Creature* sylvanas = instance->GetCreature(DATA_SYLVANAS_WINDRUNNER);
        Creature* uther = instance->GetCreature(DATA_UTHER_THE_LIGHTBRINGER);
        Creature* jaina = instance->GetCreature(DATA_JAINA_PROUDMOORE);

        if (!sylvanas || !uther || !jaina)
            return;

        Unit* remnant = instance->GetCreature(DATA_REMNANT_OF_A_FALLEN_KING);
        if (!remnant)
            return;

        uther->GetThreatManager().AddThreat(remnant, 100000);
        jaina->GetThreatManager().AddThreat(remnant, 100000);
        sylvanas->GetThreatManager().AddThreat(remnant, 100000);

        uther->SetReactState(REACT_AGGRESSIVE);
        jaina->SetReactState(REACT_AGGRESSIVE);
        sylvanas->SetReactState(REACT_AGGRESSIVE);
    }

    void ZoneAllies() // @TODO: figure out why they aren't attacking anduin after intermission / fix naming
    {
        Creature* sylvanas = instance->GetCreature(DATA_SYLVANAS_WINDRUNNER);
        Creature* uther = instance->GetCreature(DATA_UTHER_THE_LIGHTBRINGER);
        Creature* jaina = instance->GetCreature(DATA_JAINA_PROUDMOORE);

        if (!sylvanas || !uther || !jaina)
            return;

        Unit* anduin = instance->GetCreature(DATA_ANDUIN_WRYNN);
        if (!anduin)
            return;

        uther->GetThreatManager().AddThreat(anduin, 100000);
        jaina->GetThreatManager().AddThreat(anduin, 100000);
        sylvanas->GetThreatManager().AddThreat(anduin, 100000);

        DoZoneInCombat(sylvanas);
        DoZoneInCombat(jaina);
        DoZoneInCombat(uther);
    }

    void ClearDebuffs()
    {
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_HOPEBREAKER_DEBUFF);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLASPHEMY_OVERCONFIDENCE);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLASPHEMY_HOPELESSNESS);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DOMINATION_WORD_PAIN);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BEFOULED_BARRIER_DEBUFF);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BEFOULED_BARRIER_EXPLODE);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LOST_SOUL_DIMENSION);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FORCE_OF_WILL);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SCARRED_SOUL);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WEATHER_COSMETIC);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_REMORSELESS_WINTER_PERIODIC);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_HOPELESSNESS_HOPELESSNESS_AREATRIGGER);
    }

    void JustAppeared() override
    {
        instance->SetBossState(DATA_ANDUIN_WRYNN, NOT_STARTED);
        scheduler.ClearValidator();
        HandleIntroduction();
        me->SetEmoteState(EMOTE_STATE_READY2H);
        me->ModifyPower(me->GetPowerType(), 0);
        DoCastSelf(SPELL_ANDUIN_PROGRESSION_AURA);
        DoCastSelf(SPELL_WILLPOWER_DISPLAY, true);
        DoCastSelf(SPELL_DARK_ZEAL_AURA, true);
        instance->DoUpdateWorldState(WORLD_STATE_ANDUIN_ENCOUNTER_STARTED, 0);
        instance->DoUpdateWorldState(WORLD_STATE_ANDUIN_INTERMISSION, 0);

        if (GameObject* bridge = me->FindNearestGameObject(GAMEOBJECT_BRIDGE_AFTER_ANDUIN, 500.0f))
        {
            if (instance->GetData(DATA_ANDUIN_WRYNN_INTRODUCTION) != DONE)
            {
                bridge->SetLootState(GO_READY);
                bridge->UseDoorOrButton(0, false);
            }
        }
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        ClearDebuffs();
        Talk(SAY_DISENGAGE);
        if (_encounterEnded == true)
            return;

        _DespawnAtEvade();
        summons.DespawnAll();
        instance->DoUpdateWorldState(WORLD_STATE_ANDUIN_ENCOUNTER_STARTED, 0);
        instance->SetData(DATA_ANDUIN_WRYNN, NOT_STARTED);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (GameObject* bridge = me->FindNearestGameObject(GAMEOBJECT_BRIDGE_TO_ANDUIN, 500.0f))
        {
            bridge->SetGoState(GO_STATE_READY);
            bridge->RemoveFlag(GO_FLAG_IN_USE);
        }
    }

    void Reset() override
    {
        _Reset();
        _dominationWordCount = 0;
        me->SummonCreatureGroup(SPAWN_GROUP_INITIAL);
        events.Reset();
        me->SetPower(POWER_ENERGY, 0);
        Initialize();
    }

    void KilledUnit(Unit* victim) override
    {
        if (_slayTextOnCooldown == false && !victim->IsPlayer())
        {
            Talk(SAY_SLAY);
            _slayTextOnCooldown = true;
            scheduler.Schedule(3s, [this](TaskContext /*task*/)
            {
                _slayTextOnCooldown = false;
            });
        }
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        switch (summon->GetEntry())
        {
            case NPC_BEFOULED_BARRIER:
            {
                summon->SetReactState(REACT_PASSIVE);
                summon->CastSpell(summon, SPELL_BEFOULED_ERUPTION, true);
                summon->CastSpell(summon, SPELL_BEFOULED_BARRIER_SPHERE_AREATRIGGER, true);
                summon->CastSpell(summon, SPELL_BEFOULED_BARRIER_BLACK_RING, true);
                break;
            }

            case BOSS_REMNANT_OF_A_FALLEN_KING:
            {
                summon->SetUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
                summon->CastSpell(summon, SPELL_REMNANT_SPAWN);
                summon->SetReactState(REACT_PASSIVE);
                break;
            }

            case NPC_MARCH_OF_THE_DAMNED:
            {
                float marchSpeed = 0.0f;
                switch (me->GetMap()->GetDifficultyID())
                {
                    case DIFFICULTY_LFR_NEW:
                    case DIFFICULTY_NORMAL_RAID:
                    case DIFFICULTY_HEROIC_RAID:
                        marchSpeed = 0.40f;
                        break;
                    case DIFFICULTY_MYTHIC_RAID:
                        marchSpeed = 0.60f;
                        break;
                    default:
                        marchSpeed = 0.40f;
                        break;
                }

                summon->SetSpeedRate(MOVE_RUN, marchSpeed);
                summon->SetReactState(REACT_PASSIVE);
                summon->CastSpell(summon, SPELL_MARCH_OF_THE_DAMNED_AREATRIGGER, true);
                scheduler.Schedule(1s, [summon](TaskContext /*task*/)
                {
                    Position exitPlatform = summon->GetFirstCollisionPosition(100.0f, summon->GetAbsoluteAngle(summon));
                    summon->GetMotionMaster()->MovePoint(POINT_MARCH_OF_THE_DAMNED, exitPlatform, false, summon->GetOrientation());
                });
                break;
            }

            case NPC_BEACON_OF_HOPE:
            {
                summon->SetReactState(REACT_PASSIVE);
                summon->CastSpell(summon, SPELL_BEACON_OF_HOPE_AREATRIGGER, false);
                break;
            }

            default:
                break;
        }
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        switch (summon->GetEntry())
        {
            case NPC_ANDUIN_DOUBT:
            case NPC_ANDUIN_DESPAIR:
            {
                if (TempSummon* tempSummon = summon->ToTempSummon())
                    tempSummon->SetTempSummonType(TEMPSUMMON_CORPSE_DESPAWN);
                break;
            }

            case NPC_GRIM_REFLECTION:
            {
                if (TempSummon* tempSummon = summon->ToTempSummon())
                    tempSummon->SetTempSummonType(TEMPSUMMON_MANUAL_DESPAWN);
                summon->DespawnOrUnsummon(3s);
                break;
            }

            default:
                break;
        }
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        PhaseEvents(PHASE_ONE);
        instance->DoUpdateWorldState(WORLD_STATE_ANDUIN_ENCOUNTER_STARTED, PHASE_ONE);
        Talk(SAY_AGGRO);
        scheduler.Schedule(10s, [this](TaskContext /*task*/)
        {
            if (GameObject* bridge = me->FindNearestGameObject(GAMEOBJECT_BRIDGE_TO_ANDUIN, 500.0f))
            {
                bridge->SetLootState(GO_READY);
                bridge->UseDoorOrButton(0, false);
            }
        });
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_START_PRE_INTRODUCTION:
            {
                Creature* uther = instance->GetCreature(DATA_UTHER_THE_LIGHTBRINGER);
                Creature* sylvanas = instance->GetCreature(DATA_SYLVANAS_WINDRUNNER);
                Creature* jaina = instance->GetCreature(DATA_JAINA_PROUDMOORE);

                if (!uther || !sylvanas || !jaina)
                    return;

                jaina->NearTeleportTo(PreIntroductionAssistersPositions[0], false);
                jaina->CastSpell(jaina, SPELL_BROKER_SPAWN, true);
                uther->NearTeleportTo(PreIntroductionAssistersPositions[1], false);
                uther->CastSpell(uther, SPELL_BROKER_SPAWN, true);
                sylvanas->NearTeleportTo(PreIntroductionAssistersPositions[2], false);
                sylvanas->CastSpell(sylvanas, SPELL_BROKER_SPAWN, true);
                break;
            }

            case ACTION_START_MOVEMENT:
            {
                Creature* uther = instance->GetCreature(DATA_UTHER_THE_LIGHTBRINGER);
                Creature* sylvanas = instance->GetCreature(DATA_SYLVANAS_WINDRUNNER);
                Creature* jaina = instance->GetCreature(DATA_JAINA_PROUDMOORE);

                if (!uther || !sylvanas || !jaina)
                    return;

                jaina->CastSpell(jaina, SPELL_BROKER_SPAWN, true);
                uther->CastSpell(uther, SPELL_BROKER_SPAWN, true);
                sylvanas->CastSpell(sylvanas, SPELL_BROKER_SPAWN, true);
                break;
            }

            case ACTION_START_INTRODUCTION:
            {
                instance->SetData(DATA_ANDUIN_WRYNN_INTRODUCTION, IN_PROGRESS);
                scheduler.Schedule(1ms, [this](TaskContext /*task*/)
                {
                    if (Creature* uther = instance->GetCreature(DATA_UTHER_THE_LIGHTBRINGER))
                    {
                        if (Creature* sylvanas = instance->GetCreature(DATA_SYLVANAS_WINDRUNNER))
                        {
                            if (Creature* jaina = instance->GetCreature(DATA_JAINA_PROUDMOORE))
                            {
                                ObjectGuid utherGUID = uther->GetGUID();
                                ObjectGuid sylvanasGUID = sylvanas->GetGUID();
                                ObjectGuid jainaGUID = jaina->GetGUID();

                                Conversation* introductionAnduin = Conversation::CreateConversation(CONVERSATION_INTRO, me, me->GetPosition(), ObjectGuid::Empty, nullptr, false);
                                introductionAnduin->AddActor(NPC_UTHER_THE_LIGHTBRINGER, 1, utherGUID);
                                introductionAnduin->AddActor(NPC_SYLVANAS_WINDRUNNER, 2, sylvanasGUID);
                                introductionAnduin->AddActor(NPC_LADY_JAINA_PROUDMOORE, 3, jainaGUID);
                                introductionAnduin->Start();
                            }
                        }
                    }
                });

                scheduler.Schedule(35s, [this](TaskContext /*task*/)
                {
                    instance->SetData(DATA_ANDUIN_WRYNN_INTRODUCTION, DONE);
                });
                break;
            }

            case ACTION_START_OUTRODUCTION:
            {
                if (Creature* uther = instance->GetCreature(DATA_UTHER_THE_LIGHTBRINGER))
                {
                    if (Creature* sylvanas = instance->GetCreature(DATA_SYLVANAS_WINDRUNNER))
                    {
                        if (Creature* jaina = instance->GetCreature(DATA_JAINA_PROUDMOORE))
                        {
                            if (Creature* firim = instance->GetCreature(DATA_FIRIM))
                            {
                                firim->GetMotionMaster()->MovePath(PATH_OUTRODUCTION_FIRIM, false);

                                Conversation* introductionAnduin = Conversation::CreateConversation(CONVERSATION_ANDUIN_OUTRODUCTION, me, me->GetPosition(), ObjectGuid::Empty, nullptr, false);
                                introductionAnduin->AddActor(NPC_LADY_JAINA_PROUDMOORE, 1, jaina->GetGUID());
                                introductionAnduin->AddActor(NPC_SYLVANAS_WINDRUNNER, 2, sylvanas->GetGUID());
                                introductionAnduin->AddActor(NPC_UTHER_THE_LIGHTBRINGER, 3, uther->GetGUID());
                                introductionAnduin->AddActor(NPC_FIRIM, 4, firim->GetGUID());
                                introductionAnduin->Start();
                            }
                        }
                    }
                }
                break;
            }

            case ACTION_CANCEL_EVENTS:
            {
                events.CancelEvent(EVENT_HOPEBREAKER);
                events.CancelEvent(EVENT_DOMINATION_WORD_PAIN);
                events.CancelEvent(EVENT_BEFOULED_BARRIER);
                events.CancelEvent(EVENT_KINGSMOURNE_HUNGERS);
                events.CancelEvent(EVENT_WICKED_STAR);
                events.CancelEvent(EVENT_BLASPHEMY);
                events.CancelEvent(EVENT_GRIM_REFLECTIONS);
                _dominationWordCount = 0;
                break;
            }

            case ACTION_ARTHAS_INTERMISSION_UTHER:
            {
                instance->DoUpdateWorldState(WORLD_STATE_ANDUIN_INTERMISSION, 1);
                if (Creature* uther = instance->GetCreature(DATA_UTHER_THE_LIGHTBRINGER))
                {
                    Conversation* conversationUther = Conversation::CreateConversation(CONVERSATION_ARTHAS_UTHER, me, me->GetPosition(), ObjectGuid::Empty, nullptr, false);
                    conversationUther->AddActor(NPC_UTHER_THE_LIGHTBRINGER, 1, uther->GetGUID());
                    conversationUther->Start();
                }
                break;
            }

            case ACTION_ARTHAS_INTERMISSION_SYLVANAS:
            {
                instance->DoUpdateWorldState(WORLD_STATE_ANDUIN_INTERMISSION, 2);
                if (Creature* sylvanas = instance->GetCreature(DATA_SYLVANAS_WINDRUNNER))
                {
                    Conversation* conversationSylvanas = Conversation::CreateConversation(CONVERSATION_ARTHAS_SYLVANAS, me, me->GetPosition(), ObjectGuid::Empty, nullptr, false);
                    conversationSylvanas->AddActor(NPC_SYLVANAS_WINDRUNNER, 1, sylvanas->GetGUID());
                    conversationSylvanas->Start();
                }
                break;
            }

            case ACTION_EXIT_INTERMISSION:
            {
                if (_intermissionsDone == 0)
                {
                    PhaseEvents(PHASE_TWO);
                    scheduler.Schedule(3s, [this](TaskContext /*task*/)
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        ZoneAllies();
                    });
                }
                else if (_intermissionsDone == 1)
                {
                    PhaseEvents(PHASE_THREE);
                    scheduler.Schedule(6s, [this](TaskContext /*task*/)
                    {
                        me->SetReactState(REACT_AGGRESSIVE);
                        ZoneAllies();
                    });
                }
                break;
            }

            case ACTION_END_ENCOUNTER:
            {
                EndEncounter();
                break;
            }

            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        Unit* victim = me->GetVictim();
        if (victim && !victim->IsPlayer())
        {
            EnterEvadeMode(EvadeReason::NoHostiles);
            return;
        }

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        auto SpawnRemnant = [this](TaskContext /*task*/)
        {
            if (Creature* arthas = instance->GetCreature(DATA_REMNANT_OF_A_FALLEN_KING))
                me->CastSpell(arthas, SPELL_SPAWN_REMNANT, false);
        };

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_HOPEBREAKER:
                {
                    DoCastSelf(SPELL_HOPEBREAKER);
                    Talk(SAY_HOPEBREAKER);
                    if (events.IsInPhase(PHASE_ONE))
                        events.Repeat(Milliseconds(HopeBreakerP1.nextTimer()));
                    else
                        events.Repeat(Milliseconds(HopeBreakerP2.nextTimer()));
                    break;
                }

                case EVENT_EMPOWERED_HOPEBREAKER:
                {
                    DoCastSelf(SPELL_EMPOWERED_HOPEBREAKER, false);
                    Talk(SAY_EMPOWERED_HOPEBREAKER);
                    if (me->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
                        events.Repeat(65500ms);
                    else
                        events.Repeat(58400ms);
                    break;
                }

                case EVENT_DOMINATION_WORD_PAIN:
                {
                    _dominationWordCount++;
                    DoCastSelf(SPELL_DOMINATION_WORD_PAIN, false);
                    if (events.IsInPhase(PHASE_ONE))
                        events.Repeat(Milliseconds(DominationWordPainP1.nextTimer()));
                    else
                        events.Repeat(Milliseconds(DominationWordPainP2.nextTimer()));

                    if (events.IsInPhase(PHASE_ONE) && _dominationWordCount == 11)
                        DoAction(ACTION_CANCEL_EVENTS);
                    else if (events.IsInPhase(PHASE_TWO) && _dominationWordCount == 12)
                        DoAction(ACTION_CANCEL_EVENTS);
                    break;
                }

                case EVENT_BEFOULED_BARRIER:
                {
                    DoCastSelf(SPELL_BEFOULED_BARRIER);
                    Talk(SAY_BEFOULED_BARRIER);
                    if (events.IsInPhase(PHASE_ONE))
                        events.Repeat(Milliseconds(BefouledBarrierTimerP1.nextTimer()));
                    else
                        events.Repeat(Milliseconds(BefouledBarrierTimerP2.nextTimer()));
                    break;
                }

                case EVENT_BLASPHEMY:
                {
                    DoCastSelf(SPELL_BLASPHEMY);
                    Talk(SAY_ANNOUNCE_BLASPHEMY);
                    Talk(SAY_BLASPHEMY);
                    events.Repeat(Milliseconds(BlasphemyAll.nextTimer()));
                    break;
                }

                case EVENT_HOPELESSNESS:
                {
                    DoCastAOE(SPELL_HOPELESSNESS);
                    Talk(SAY_ANNOUNCE_BLASPHEMY);
                    Talk(SAY_BLASPHEMY);
                    if (me->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
                        events.Repeat(65500ms);
                    else
                        events.Repeat(58500ms);
                    break;
                }

                case EVENT_WICKED_STAR:
                {
                    DoCastSelf(SPELL_WICKED_STAR);
                    Talk(SAY_WICKED_STAR);
                    if (events.IsInPhase(PHASE_ONE))
                        events.Repeat(Milliseconds(WickedStarP1.nextTimer()));
                    else
                        events.Repeat(Milliseconds(WickedStarP1.nextTimer()));
                    break;
                }

                case EVENT_EMPOWERED_WICKED_STAR:
                {
                    DoCastSelf(SPELL_EMPOWERED_WICKED_STAR, true);
                    Talk(SAY_WICKED_STAR);
                    if (me->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
                        events.Repeat(65500ms);
                    else
                        events.Repeat(58500ms);
                    break;
                }

                case EVENT_KINGSMOURNE_HUNGERS:
                {
                    DoCastSelf(SPELL_KINGSMOURNE_HUNGERS, false);
                    Talk(SAY_ANNOUNCE_KINGSMOURNE_HUNGERS);
                    Talk(SAY_KINGSMOURNE_HUNGERS);
                    events.Repeat(60s);
                    break;
                }

                case EVENT_GRIM_REFLECTIONS:
                {
                    Talk(SAY_GRIM_REFLECTIONS);
                    DoCastSelf(SPELL_GRIM_REFLECTIONS);
                    events.Repeat(87000ms);
                    break;
                }

                case EVENT_BEACON_OF_HOPE:
                {
                    DoCastSelf(SPELL_BEACON_OF_HOPE);
                    break;
                }

                case EVENT_INTERMISSION_ONE:
                {
                    DoAction(ACTION_CANCEL_EVENTS);
                    scheduler.Schedule(1ms, [this](TaskContext /*task*/)
                    {
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        DoCastSelf(SPELL_SHADESTEP);
                        me->NearTeleportTo(DominationGraspCenter, false);
                        PrepareAssistersForIntermission();
                    });

                    scheduler.Schedule(1204ms, [this](TaskContext /*task*/)
                    {
                        DoAction(ACTION_ARTHAS_INTERMISSION_UTHER);
                    });

                    scheduler.Schedule(2204ms, [this](TaskContext /*task*/)
                    {
                        me->SetFacingTo(1.626040f);
                    });

                    scheduler.Schedule(4s, [this](TaskContext /*task*/)
                    {
                        DoCastSelf(SPELL_DOMINATION_GRASP_ROOT_AREATRIGGER);
                        DoCastSelf(SPELL_FORCE_OF_WILL);
                        DoCastSelf(SPELL_HOPEBREAKER_CLEAR);

                        if (Creature* arthas = me->SummonCreature(BOSS_REMNANT_OF_A_FALLEN_KING, RemnantOfAFallenKingSpawnPosition, TEMPSUMMON_MANUAL_DESPAWN))
                        {
                            me->CastSpell(arthas, SPELL_DOMINATION_GRASP, false);
                            me->CastSpell(arthas, SPELL_SPAWN_REMNANT, false);
                        }
                    });

                    scheduler.Schedule(5s, SpawnRemnant);

                    scheduler.Schedule(6s, SpawnRemnant);

                    scheduler.Schedule(7s, SpawnRemnant);

                    scheduler.Schedule(8100ms, [this](TaskContext /*task*/)
                    {
                        if (Creature* arthas = instance->GetCreature(DATA_REMNANT_OF_A_FALLEN_KING))
                        {
                            me->CastSpell(arthas, SPELL_SPAWN_REMNANT, false);
                            arthas->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC);
                            arthas->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
                            arthas->SetReactState(REACT_PASSIVE);
                            ForceAssistersToAttackRemnant();

                            // only in first intermission
                            if (me->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
                                DoCastSelf(SPELL_MARCH_OF_THE_DAMNED_PERIODIC);
                        }
                    });
                    break;
                }

                case EVENT_INTERMISSION_TWO:
                {
                    // info: only timers are different to EVENT_INTERMISSION_ONE
                    scheduler.Schedule(1ms, [this](TaskContext /*task*/)
                    {
                        me->SetReactState(REACT_PASSIVE);
                        me->CastSpell(DominationGraspCenter, SPELL_SHADESTEP, false);
                        me->NearTeleportTo(DominationGraspCenter, false);
                        PrepareAssistersForIntermission();
                    });

                    scheduler.Schedule(2204ms, [this](TaskContext /*task*/)
                    {
                        DoAction(ACTION_ARTHAS_INTERMISSION_SYLVANAS);
                    });

                    scheduler.Schedule(3204ms, [this](TaskContext /*task*/)
                    {
                        me->SetFacingTo(1.626040f);
                    });

                    scheduler.Schedule(5s, [this](TaskContext /*task*/)
                    {
                        me->SetFacingTo(1.626040f);
                        DoCastSelf(SPELL_FORCE_OF_WILL);
                        DoCastSelf(SPELL_HOPEBREAKER_CLEAR);
                        DoCastSelf(SPELL_DOMINATION_GRASP_ROOT_AREATRIGGER);

                        if (Creature* arthas = me->SummonCreature(BOSS_REMNANT_OF_A_FALLEN_KING, RemnantOfAFallenKingSpawnPosition, TEMPSUMMON_MANUAL_DESPAWN))
                        {
                            me->CastSpell(arthas, SPELL_DOMINATION_GRASP, false);
                            me->CastSpell(arthas, SPELL_SPAWN_REMNANT, false);
                        }
                    });

                    scheduler.Schedule(6s, SpawnRemnant);

                    scheduler.Schedule(7s, SpawnRemnant);

                    scheduler.Schedule(8s, SpawnRemnant);

                    scheduler.Schedule(9s, [this](TaskContext /*task*/)
                    {
                        DoCastSelf(SPELL_MARCH_OF_THE_DAMNED_PERIODIC);
                        if (Creature* arthas = instance->GetCreature(DATA_REMNANT_OF_A_FALLEN_KING))
                        {
                            me->CastSpell(arthas, SPELL_SPAWN_REMNANT, false);
                            arthas->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC);
                            arthas->RemoveUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
                        }
                    });
                    break;
                }

                case EVENT_BERSERK:
                {
                    DoCastSelf(SPELL_BERSERK);
                    break;
                }

                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        DoMeleeAttackIfReady();
    }

    void PhaseEvents(uint8 phase)
    {
        events.Reset();

        switch (phase)
        {
            case PHASE_ONE:
            {
                events.SetPhase(PHASE_ONE);
                if (me->GetMap()->GetDifficultyID() == DIFFICULTY_LFR_NEW)
                    DoCastSelf(SPELL_ANDUIN_WILLPOWER_PERIODIC, true);

                if (me->GetMap()->GetDifficultyID() != DIFFICULTY_LFR_NEW)
                    events.ScheduleEvent(EVENT_KINGSMOURNE_HUNGERS, 45s);
                events.ScheduleEvent(EVENT_HOPEBREAKER, 5s);
                events.ScheduleEvent(EVENT_BEFOULED_BARRIER, 17s);
                events.ScheduleEvent(EVENT_BLASPHEMY, 30s);
                events.ScheduleEvent(EVENT_WICKED_STAR, 55s);
                events.ScheduleEvent(EVENT_DOMINATION_WORD_PAIN, 7s);
                events.ScheduleEvent(EVENT_INTERMISSION_ONE, 150s);
                break;
            }

            case PHASE_TWO:
            {
                _intermissionsDone = 1;
                events.SetPhase(PHASE_TWO);
                me->ModifyPower(me->GetPowerType(), 0);

                if (me->GetMap()->GetDifficultyID() == DIFFICULTY_LFR_NEW)
                    DoCastSelf(SPELL_ANDUIN_WILLPOWER_PERIODIC, true);

                if (me->GetMap()->GetDifficultyID() != DIFFICULTY_LFR_NEW)
                    events.ScheduleEvent(EVENT_KINGSMOURNE_HUNGERS, 48600ms);

                events.ScheduleEvent(EVENT_GRIM_REFLECTIONS, 8500ms);
                events.ScheduleEvent(EVENT_BEFOULED_BARRIER, 80600ms);
                events.ScheduleEvent(EVENT_HOPEBREAKER, 13600ms);
                events.ScheduleEvent(EVENT_WICKED_STAR, 18600ms);
                events.ScheduleEvent(EVENT_DOMINATION_WORD_PAIN, 10600ms);
                events.ScheduleEvent(EVENT_INTERMISSION_TWO, 169s);
                events.ScheduleEvent(EVENT_BERSERK, 900s);
                break;
            }

            case PHASE_THREE:
            {
                _intermissionsDone = 2;
                me->ModifyPower(me->GetPowerType(), 0);
                events.SetPhase(PHASE_THREE);
                Conversation::CreateConversation(CONVERSATION_ANDUIN_PHASE_THREE, me, me->GetPosition(), ObjectGuid::Empty, nullptr, true);
                events.ScheduleEvent(EVENT_BEACON_OF_HOPE, 3s);
                events.ScheduleEvent(EVENT_EMPOWERED_HOPEBREAKER, 11600ms);
                events.ScheduleEvent(EVENT_HOPELESSNESS, 21800ms);
                events.ScheduleEvent(EVENT_EMPOWERED_WICKED_STAR, 41s);
                break;
            }
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        if (_intermissionsDone < 2 && !HealthAbovePct(10))
        {
            me->RemoveAurasDueToSpell(SPELL_DOMINATION_GRASP_ROOT_AREATRIGGER);
            me->RemoveAurasDueToSpell(SPELL_MARCH_OF_THE_DAMNED_PERIODIC);
            scheduler.Schedule(6s, [this](TaskContext /*task*/)
            {
                me->SetReactState(REACT_AGGRESSIVE);
            });

            PhaseEvents(PHASE_THREE);

            std::list<Creature*> fiendishSouls;
            GetCreatureListWithEntryInGrid(fiendishSouls, me, NPC_FIENDISH_SOUL, 300.0f);

            for (Creature* fiends : fiendishSouls)
            {
                fiends->CastSpell(fiends, SPELL_SOUL_DESPAWN, false);
                fiends->DespawnOrUnsummon(500ms);
            }

            std::list<Creature*> marches;
            GetCreatureListWithEntryInGrid(marches, me, NPC_MARCH_OF_THE_DAMNED, 300.0f);

            for (Creature* march : marches)
                march->DespawnOrUnsummon();

            if (Creature* remnant = instance->GetCreature(DATA_REMNANT_OF_A_FALLEN_KING))
                remnant->GetAI()->DoAction(ACTION_DESPAWN_REMNANT);
        }
    }

    void EndEncounter()
    {
        ClearDebuffs();
        events.Reset();
        scheduler.CancelAll();
        _encounterEnded = true;
        instance->SetBossState(DATA_ANDUIN_WRYNN, DONE);
        _EnterEvadeMode();
        DoCastSelf(SPELL_ANDUIN_KNEEL_POSE);
        if (Creature* beacon = me->FindNearestCreature(NPC_BEACON_OF_HOPE, 100.0f, true))
            beacon->DespawnOrUnsummon();

        me->SetReactState(REACT_PASSIVE);
        me->SetFaction(FACTION_FRIENDLY);
        instance->DoUpdateWorldState(WORLD_STATE_ANDUIN_ENCOUNTER_COMPLETED, 1);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        instance->SetData(DATA_ANDUIN_WRYNN, DONE);

        if (me->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
        {
            if (AchievementEntry const* mythicAnduin = sAchievementStore.LookupEntry(ACHIEVEMENT_ANDUIN_MYTHIC))
            {
                me->GetMap()->DoOnPlayers([mythicAnduin](Player* player)
                {
                    player->CompletedAchievement(mythicAnduin);
                });
            }
        }

        if (GameObject* chest = me->SummonGameObject(GAMEOBJECT_ANDUIN_CHEST_LOOT, ChestLootSpawnPosition, QuaternionData::fromEulerAnglesZYX(-225.0f, 0.0f, 0.0f), 7_days))
        {
            chest->SetGoState(GO_STATE_ACTIVE);
            chest->SetSpellVisualId(SPELL_VISUAL_CHEST_LOOT, ObjectGuid::Empty);
        }

        scheduler.Schedule(5s, [this](TaskContext /*task*/)
        {
            instance->DoCastSpellOnPlayers(SPELL_FINAL_MOVIE);
        });

        if (GameObject* bridge = me->FindNearestGameObject(GAMEOBJECT_BRIDGE_TO_ANDUIN, 500.0f))
        {
            bridge->SetLootState(GO_READY);
            bridge->UseDoorOrButton(0, false);
        }

        if (GameObject* bridgeCredit = me->FindNearestGameObject(GAMEOBJECT_BRIDGE_AFTER_ANDUIN, 500.0f))
        {
            bridgeCredit->SetLootState(GO_READY);
            bridgeCredit->UseDoorOrButton(0, false);
        }
    }

private:
    bool _slayTextOnCooldown;
    uint8 _dominationWordCount;
    uint8 _intermissionsDone;
    bool _encounterEnded;
    TaskScheduler scheduler;
};

// Befouled Barrier - 184585
struct npc_anduin_wrynn_befouled_barrier : public ScriptedAI
{
    npc_anduin_wrynn_befouled_barrier(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        _events.ScheduleEvent(EVENT_UPDATE_BEFOULED_BARRIER, 1s);
        me->SetReactState(REACT_PASSIVE);
    }

    void UpdateAI(uint32 diff) override
    {
        UpdateVictim();

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_NPC_DESPAWN:
                {
                    me->DespawnOrUnsummon(1ms);
                    break;
                }

                case EVENT_UPDATE_BEFOULED_BARRIER:
                {
                    SetData(DATA_REQUIRED_HEALING, GetData(DATA_ABSORBED_HEALING));
                    break;
                }

                default:
                    break;
            }
        }
    }

    uint32 GetRequiredHealing() const
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_BEFOULED_BARRIER_DEBUFF, me->GetMap()->GetDifficultyID());
        if (!spellInfo)
            return 0;

        return spellInfo->GetEffect(EFFECT_0).CalcValue();
    }

    void Reset() override
    {
        _events.Reset();
        me->SetReactState(REACT_PASSIVE);
    }

private:
    EventMap _events;
};

// Beacon of Hope - 184830
struct npc_anduin_wrynn_beacon_of_hope : public ScriptedAI
{
    npc_anduin_wrynn_beacon_of_hope(Creature* creature) : ScriptedAI(creature) { }

    void JustUnregisteredAreaTrigger(AreaTrigger* areaTrigger) override
    {
        switch (areaTrigger->GetSpellId())
        {
            case SPELL_FRAGMENT_OF_HOPE_AREATRIGGER:
            {
                if (!areaTrigger->GetInsideUnits().size())
                {
                    InstanceScript* instance = me->GetInstanceScript();
                    if (!instance)
                        break;

                    if (Creature* beacon = instance->GetCreature(DATA_BEACON_OF_HOPE))
                        beacon->CastSpell(beacon, SPELL_FRAGMENT_OF_HOPE_DAMAGE, true);
                }
                break;
            }
            default:
                break;
        }
    }
};

// Empty Vessel - 183452
struct npc_anduin_wrynn_empty_vessel : public ScriptedAI
{
    npc_anduin_wrynn_empty_vessel(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(WorldObject* summoner) override
    {
        if (!summoner->IsPlayer())
            return;

        summoner->CastSpell(me, SPELL_MIRROR_IMAGE);
        DoCast(me, SPELL_FEIGN_DEATH);
    }
};

// Lost Soul - 185607
struct npc_anduin_wrynn_lost_soul : public ScriptedAI
{
    npc_anduin_wrynn_lost_soul(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        _events.ScheduleEvent(EVENT_BANISH_SOUL, 1ms);
    }

    void IsSummonedBy(WorldObject* summoner) override
    {
        if (!summoner->IsPlayer())
            return;

        summoner->CastSpell(me, SPELL_LOST_SOUL_MIRROR_IMAGE);
    }

    void JustDied(Unit* /*killer*/) override
    {
        TempSummon* summon = me->ToTempSummon();
        if (!summon)
            return;

        if (Unit* summoner = summon->GetSummonerUnit())
        {
            summoner->RemoveAurasDueToSpell(SPELL_SEVERED_SOUL);
            summoner->NearTeleportTo(me->GetPosition(), false);
        }
        me->DespawnOrUnsummon(2s);
    }

    void Reset() override
    {
        _events.Reset();
    }

    void UpdateAI(uint32 diff) override
    {
        UpdateVictim();

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BANISH_SOUL:
                {
                    DoCastSelf(SPELL_BANISH_SOUL);
                    _events.Repeat(1ms);
                    break;
                }

                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
};

// Anduin's Soul - 184519
struct npc_anduin_wrynn_anduin_soul : public ScriptedAI
{
    npc_anduin_wrynn_anduin_soul(Creature* creature) : ScriptedAI(creature),
        _hopeRestored(0), _doubtGone(0), _despairGone(0), _summons(me) { }

    void CheckForReleaseFromKingsmourne()
    {
        if (_hopeRestored == 4 && _doubtGone == 4 && _despairGone == 1)
        {
            DoCastAOE(SPELL_LOST_SOUL_CLEAR, true);
            _hopeRestored = 0;
            _doubtGone = 0;
            _despairGone = 0;
        }
    }

    void JustAppeared() override
    {
        me->SetDisableGravity(true, true);
        me->SetHoverHeight(1.0);
        me->SetPlayHoverAnim(true);
        DoCastSelf(SPELL_LOST_SOUL_PERIODIC);
    }

    void Reset() override
    {
        _summons.DespawnAll();
        DoCastSelf(SPELL_ANDUIN_SOUL_GHOST, true);
    }

    void JustSummoned(Creature* who) override
    {
        _summons.Summon(who);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _summons.DespawnAll();
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        _summons.DespawnAll();
        DoCastSelf(SPELL_ANDUIN_SOUL_GHOST, true);
    }

    void DoAction(int32 action) override
    {
        InstanceScript* instance = me->GetInstanceScript();
        if (!instance)
            return;

        Creature* anduin = instance->GetCreature(DATA_ANDUIN_SOUL);
        if (!anduin)
            return;

        switch (action)
        {
            case ACTION_SUMMON_KINGSMOURNE_SOULS:
            {
                me->SummonCreature(NPC_ANDUIN_DESPAIR, AnduinsDespairSpawnPosition, TEMPSUMMON_TIMED_DESPAWN, 40s);
                for (uint8 i = 0; i < 4; i++)
                {
                    me->SummonCreature(NPC_ANDUIN_DOUBT, AnduinsDoubtSpawnPositions[i], TEMPSUMMON_TIMED_DESPAWN, 40s);
                    me->SummonCreature(NPC_ANDUIN_HOPE, AnduinsHopeSpawnPosition[i], TEMPSUMMON_TIMED_DESPAWN, 40s);
                }
                break;
            }
            case ACTION_DESPAIR_GONE:
                _despairGone++;
                CheckForReleaseFromKingsmourne();
                break;

            case ACTION_DOUBT_GONE:
                _doubtGone++;
                CheckForReleaseFromKingsmourne();
                break;

            case ACTION_HOPE_RESTORED:
                _hopeRestored++;
                CheckForReleaseFromKingsmourne();
                break;

            default:
                break;
        }
    }

public:
    uint8 _hopeRestored;
    uint8 _doubtGone;
    uint8 _despairGone;
    SummonList _summons;
};

// Anduin's Despair - 184520
struct npc_anduin_wrynn_anduin_despair : public ScriptedAI
{
    npc_anduin_wrynn_anduin_despair(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void Reset() override
    {
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
    }

    void JustAppeared() override
    {
        DoCastSelf(SPELL_RAIN_OF_DESPAIR);
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        DoCastSelf(SPELL_ANDUIN_SOUL_DESPAIR);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        DoCastSelf(SPELL_WILLPOWER_ENERGIZE_LARGE);

        if (Creature* soul = me->FindNearestCreature(NPC_ANDUIN_SOUL, 300.0f))
            soul->GetAI()->DoAction(ACTION_DESPAIR_GONE);
    }

private:
    InstanceScript* _instance;
};

// Anduin's Doubt - 184494
struct npc_anduin_wrynn_anduin_doubt : public ScriptedAI
{
    npc_anduin_wrynn_anduin_doubt(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void Reset() override
    {
        _events.Reset();
    }

    void JustAppeared() override
    {
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        me->SetWalk(true);
        DoCastSelf(SPELL_GHOST_VISUAL_COSMETIC, true);
        DoZoneInCombat();
        _events.ScheduleEvent(EVENT_ANDUIN_SOUL, 2s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        DoCastSelf(SPELL_WILLPOWER_ENERGIZE_SMALL);
        DoCastSelf(SPELL_SOUL_DESPAWN);

        if (Creature* soul = me->FindNearestCreature(NPC_ANDUIN_SOUL, 300.0f))
            soul->GetAI()->DoAction(ACTION_DOUBT_GONE);
    }

    void UpdateAI(uint32 diff) override
    {
        UpdateVictim();

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ANDUIN_SOUL:
                {
                    // @TODO: Drop stuff if we found a way which covers stun/deathgrip
                    me->GetMotionMaster()->MovePoint(POINT_ANDUIN_SOUL, AnduinsSoulSpawnPosition);
                    _events.Repeat(200ms);
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    InstanceScript* _instance;
    EventMap _events;
};

// Anduin's Hope - 184493
struct npc_anduin_wrynn_anduin_hope : public ScriptedAI
{
    npc_anduin_wrynn_anduin_hope(Creature* creature) : ScriptedAI(creature),
        _instance(creature->GetInstanceScript()) { }

    void Reset() override
    {
        me->SetRegenerateHealth(false);
        me->SetHealth(1);
        me->SetReactState(REACT_PASSIVE);
    }

    void JustAppeared() override
    {
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        me->SetWalk(true);
        DoCastSelf(SPELL_ANDUIN_SLOW);
        _scheduler.ClearValidator();
        if (me->GetMap()->GetDifficultyID() == DIFFICULTY_HEROIC_RAID || me->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
            DoCastSelf(SPELL_GLOOM);

        _scheduler.Schedule(2s, [this](TaskContext /*task*/)
        {
            Position exitPlatform = me->GetFirstCollisionPosition(100.0f, 0);
            me->GetMotionMaster()->MovePoint(POINT_ESCAPE_PLATFORM, exitPlatform, false, me->GetOrientation());
        });
        me->SetHealth(1);

        _scheduler.Schedule(40s, [this](TaskContext /*task*/)
        {
            me->DespawnOrUnsummon();
        });
    }

    void UpdateAI(uint32 diff) override
    {
        UpdateVictim();

        _scheduler.Update(diff);
    }

    void HealReceived(Unit* /*healer*/, uint32& heal) override
    {
        if (me->HealthAbovePctHealed(100, heal))
        {
            _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            DoCastSelf(SPELL_WILLPOWER_ENERGIZE_SMALL);
            DoCastSelf(SPELL_SOUL_DESPAWN);

            if (Creature* soul = me->FindNearestCreature(NPC_ANDUIN_SOUL, 300.0f))
                soul->GetAI()->DoAction(ACTION_HOPE_RESTORED);
        }
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_ESCAPE_PLATFORM:
            {
                DoCastSelf(SPELL_SOUL_DESPAWN);
                break;
            }

            default:
                break;
        }
    }

private:
    InstanceScript* _instance;
    TaskScheduler _scheduler;
};

// Fiendish Soul - 183669
struct npc_anduin_wrynn_fiendish_soul : public ScriptedAI
{
    npc_anduin_wrynn_fiendish_soul(Creature* creature) : ScriptedAI(creature) { }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_GHOUL_LEAP, 1s);
        _events.ScheduleEvent(EVENT_NECROTIC_CLAWS, 10s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (me->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID || me->GetMap()->GetDifficultyID() == DIFFICULTY_HEROIC_RAID)
            DoAction(ACTION_SOUL_EXPLOSION);

        DoCastSelf(SPELL_SOUL_DESPAWN);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        me->DespawnOrUnsummon();
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_SOUL_EXPLOSION:
            {
                DoCastSelf(SPELL_SOUL_EXPLOSION_TARGET);
                break;
            }
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_GHOUL_LEAP:
                {
                    std::list<Player*> targetList;
                    GetPlayerListInGrid(targetList, me, 300.0f);
                    for (Player* target : targetList)
                        me->CastSpell(target, SPELL_NECROTIC_CLAWS_LEAP, true);

                    _events.Repeat(10s);
                    break;
                }
                case EVENT_NECROTIC_CLAWS:
                {
                    DoCastVictim(SPELL_NECROTIC_CLAWS_DEBUFF);
                    _events.Repeat(5s, 10s);
                    break;
                }
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

// Monstrous Soul - 183671
struct npc_anduin_wrynn_monstrous_soul : public ScriptedAI
{
    npc_anduin_wrynn_monstrous_soul(Creature* creature) : ScriptedAI(creature) { }

    void JustEngagedWith(Unit* /*who*/) override
    {
        DoCastSelf(SPELL_UNRAVELING_FRENZY_PERIODIC, true);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        if (me->HealthBelowPctDamaged(35, damage))
            _events.ScheduleEvent(EVENT_NECROTIC_DETONATION, 1ms);
    }

    void Reset() override
    {
        _events.Reset();
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_NECROTIC_DETONATION:
            {
                Talk(SAY_NECROTIC_DETONATION);
                DoCastSelf(SPELL_NECROTIC_DETONATION);
                break;
            }
            default:
                break;
        }
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_NECROTIC_DETONATION:
                {
                    Talk(SAY_NECROTIC_DETONATION);
                    DoCastSelf(SPELL_NECROTIC_DETONATION);
                    break;
                }
                default:
                    break;
            }
        }
        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

// Remnant of a Fallen King - 183463
struct boss_remnant_of_a_fallen_king : public BossAI
{
    boss_remnant_of_a_fallen_king(Creature* creature) : BossAI(creature, DATA_REMNANT_OF_A_FALLEN_KING) { }

    void JustAppeared() override
    {
        scheduler.ClearValidator();
        DoCastSelf(SPELL_SHADE_VISUAL);
        me->ModifyPower(me->GetPowerType(), 0);
        me->SetPower(me->GetPowerType(), 0);
        me->SetFaction(16);
        for (MapReference const& players : me->GetMap()->GetPlayers())
        {
            if (Player* player = players.GetSource())
                me->CastSpell(player, SPELL_WEATHER_COSMETIC, true);
        }
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_HOPEBREAKER);
        me->SetReactState(REACT_PASSIVE);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        _EnterEvadeMode();
        summons.DespawnAll();
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WEATHER_COSMETIC);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_REMORSELESS_WINTER_PERIODIC);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_NECROTIC_CLAWS_DEBUFF);
    }

    void ContinueAttacking()
    {
        if (Creature* sylvanas = instance->GetCreature(DATA_SYLVANAS_WINDRUNNER))
            sylvanas->GetAI()->AttackStartCaster(me, 25.0f);

        if (Creature* uther = instance->GetCreature(DATA_UTHER_THE_LIGHTBRINGER))
            uther->GetAI()->AttackStart(me);

        if (Creature* jaina = instance->GetCreature(DATA_JAINA_PROUDMOORE))
            jaina->GetAI()->AttackStartCaster(me, 25.0f);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        switch (summon->GetEntry())
        {
            case NPC_FIENDISH_SOUL:
            case NPC_MONSTROUS_SOUL:
                summon->SetReactState(REACT_PASSIVE);
                summon->m_Events.AddEvent(new ActivateGhouls(me, summon), summon->m_Events.CalculateTime(5s));
                break;
            default:
                break;
        }
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_ACTIVATE_REMNANT:
            {
                DoCastAOE(SPELL_DARK_PRESENCE);
                DoZoneInCombat();

                scheduler.Schedule(1500ms, [this](TaskContext /*task*/)
                {
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                    me->RemoveUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
                });

                scheduler.Schedule(2s, [this](TaskContext /*task*/)
                {
                    DoCastSelf(SPELL_REMNANT_TIMER);
                    events.ScheduleEvent(EVENT_ARMY_OF_THE_DEAD, 1ms);
                    events.ScheduleEvent(EVENT_SOUL_REAPER, 8000ms);
                    events.ScheduleEvent(EVENT_RETURN_TO_KINGSMOURNE, 67500ms);
                    me->SetReactState(REACT_AGGRESSIVE);
                    ContinueAttacking();
                });
                break;
            }

            case ACTION_DESPAWN_REMNANT:
            {
                //EnterEvadeMode(EvadeReason::Other);
                DoCastSelf(SPELL_SOUL_DESPAWN);
                me->DespawnOrUnsummon(500ms);
                break;
            }

            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (!UpdateVictim())
            return;

        scheduler.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_RETURN_TO_KINGSMOURNE:
                {
                    me->SetReactState(REACT_PASSIVE);
                    me->AttackStop();
                    me->InterruptNonMeleeSpells(true);
                    DoCastSelf(SPELL_RETURN_TO_KINGSMOURNE);
                    break;
                }
                case EVENT_ARMY_OF_THE_DEAD:
                {
                    DoCastSelf(SPELL_ARMY_OF_THE_DEAD);
                    me->SetReactState(REACT_AGGRESSIVE);
                    if (me->GetMap()->GetDifficultyID() != DIFFICULTY_LFR_NEW)
                        events.Repeat(36900ms);
                    break;
                }
                case EVENT_SOUL_REAPER:
                {
                    DoCastVictim(SPELL_SOUL_REAPER, false);
                    events.Repeat(12s);
                    break;
                }
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap events;
    TaskScheduler scheduler;
};

// Grim Reflection - 183033
struct npc_anduin_wrynn_grim_reflection : public ScriptedAI
{
    npc_anduin_wrynn_grim_reflection(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
    }

    void JustAppeared() override
    {
        SetCombatMovement(false);
        DoCastSelf(SPELL_CALAMITY_STATE_VISUAL);
        DoZoneInCombat();
        me->SetReactState(REACT_AGGRESSIVE);
        _events.ScheduleEvent(EVENT_PSYCHIC_TERROR, 1ms);

        if (me->GetMap()->GetDifficultyID() == DIFFICULTY_LFR_NEW || me->GetMap()->GetDifficultyID() == DIFFICULTY_NORMAL_RAID)
            _events.ScheduleEvent(EVENT_GRIM_REFLECTION_IMMUNITY, 1ms);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (me->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
            DoCastAOE(SPELL_GRIM_FATE);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_PSYCHIC_TERROR:
                {
                    DoCastSelf(SPELL_PSYCHIC_TERROR);
                    _events.Repeat(2s);
                    break;
                }
                case EVENT_GRIM_REFLECTION_IMMUNITY:
                {
                    DoCastSelf(SPELL_WICKED_STAR_PROTECTION);
                    _events.Repeat(1s);
                    break;
                }
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

private:
    EventMap _events;
};

// 183666 - Sylvanas Windrunner
struct npc_anduin_wrynn_sylvanas : public ScriptedAI
{
    npc_anduin_wrynn_sylvanas(Creature* creature) : ScriptedAI(creature, DATA_SYLVANAS_WINDRUNNER) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void Reset() override
    {
        _events.Reset();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        damage = 0;
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        damage = 0;
    }

    void JustEngagedWith(Unit* who) override
    {
        AddThreat(who, 5000.0f);
        _events.ScheduleEvent(EVENT_TUMBLE, 10s);
    }

    bool CanAIAttack(Unit const* target) const override
    {
        return !target->HasAura(SPELL_DOMINATION_GRASP_ROOT_AREATRIGGER);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_TUMBLE:
                {
                    DoCastSelf(SPELL_TUMBLE);
                    _events.Repeat(10s);
                    break;
                }
                default:
                    break;
            }
        }

        DoSpellAttackIfReady(SPELL_SHOOT_BOW);
    }

private:
    EventMap _events;
};

// 183664 - Jaina Proudmoore
struct npc_anduin_wrynn_jaina : public ScriptedAI
{
    npc_anduin_wrynn_jaina(Creature* creature) : ScriptedAI(creature, DATA_JAINA_PROUDMOORE) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void Reset() override
    {
        _events.Reset();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        damage = 0;
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        damage = 0;
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_BLINK, 10s);
    }

    bool CanAIAttack(Unit const* target) const override
    {
        return !target->HasAura(SPELL_DOMINATION_GRASP_ROOT_AREATRIGGER);
    }

    void WaypointPathEnded(uint32 /*nodeId*/, uint32 pathId) override
    {
        if (pathId != PATH_INTRODUCTION_JAINA)
            return;

        if (Creature* anduin = me->GetInstanceScript()->GetCreature(DATA_ANDUIN_WRYNN))
            anduin->GetAI()->DoAction(ACTION_START_INTRODUCTION);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CANCEL_JAINA_EVENTS:
                {
                    _events.CancelEvent(EVENT_BLINK);
                    _events.CancelEvent(EVENT_FROSTBOLT);
                    break;
                }
                case EVENT_BLINK:
                {
                    DoCastSelf(SPELL_BLINK);
                    _events.Repeat(60s);
                    break;
                }
                case EVENT_FROSTBOLT:
                {
                    DoCastVictim(SPELL_FROSTBOLT);
                    _events.Repeat(1s);
                    break;
                }
                default:
                    break;
            }
        }

        DoSpellAttackIfReady(SPELL_FROSTBOLT);
    }

private:
    EventMap _events;
};

// 183665 - Uther The Lightbringer
struct npc_anduin_wrynn_uther : public ScriptedAI
{
    npc_anduin_wrynn_uther(Creature* creature) : ScriptedAI(creature, DATA_UTHER_THE_LIGHTBRINGER) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void Reset() override
    {
        _events.Reset();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        damage = 0;
    }

    void DamageDealt(Unit* /*victim*/, uint32& damage, DamageEffectType /*damageType*/) override
    {
        damage = 0;
    }

    void JustEngagedWith(Unit* who) override
    {
        DoCast(who, SPELL_UTHER_CHARGE);
        _events.ScheduleEvent(EVENT_BLADE_OF_JUSTICE, 3s);
    }

    bool CanAIAttack(Unit const* target) const override
    {
        return !target->HasAura(SPELL_DOMINATION_GRASP_ROOT_AREATRIGGER);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CANCEL_UTHER_EVENTS:
                {
                    _events.CancelEvent(EVENT_BLADE_OF_JUSTICE);
                    break;
                }
                case EVENT_BLADE_OF_JUSTICE:
                {
                    DoCastVictim(SPELL_BLADE_OF_JUSTICE);
                    _events.Repeat(10s);
                    break;
                }
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
};

// 184297 - Dominated Translocator
struct npc_dominated_translocator : public ScriptedAI
{
    npc_dominated_translocator(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        CloseGossipMenuFor(player);

        // @TODO Alex: checkout if we can move this to conditions + smart_scripts
        if (me->GetInstanceScript()->GetBossState(DATA_ANDUIN_WRYNN) != DONE)
            return false;

        player->CastSpell(player, SPELL_TELEPORT_COSMIC_HUB, false);
        return true;
    }
};

// 367524 - Spawn Pre-Introduction
class spell_anduin_wrynn_pre_introduction : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_pre_introduction);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        switch (caster->GetEntry())
        {
            case NPC_LADY_JAINA_PROUDMOORE:
                caster->NearTeleportTo(PreIntroductionAssistersPositions[0], false);
                break;
            case NPC_UTHER_THE_LIGHTBRINGER:
                caster->NearTeleportTo(PreIntroductionAssistersPositions[1], false);
                break;
            case NPC_SYLVANAS_WINDRUNNER:
                caster->NearTeleportTo(PreIntroductionAssistersPositions[2], false);
                break;
            default:
                break;
        }
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        uint32 pathId = 0;
        switch (caster->GetEntry())
        {
            case NPC_LADY_JAINA_PROUDMOORE:
                pathId = PATH_INTRODUCTION_JAINA;
                break;
            case NPC_UTHER_THE_LIGHTBRINGER:
                pathId = PATH_INTRODUCTION_UTHER;
                break;
            case NPC_SYLVANAS_WINDRUNNER:
                pathId = PATH_INTRODUCTION_SYLVANAS;
                break;
            default:
                break;
        }
        caster->GetMotionMaster()->MovePath(pathId, false);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_anduin_wrynn_pre_introduction::OnApply, EFFECT_1, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_anduin_wrynn_pre_introduction::OnRemove, EFFECT_2, SPELL_AURA_MOD_ROOT, AURA_EFFECT_HANDLE_REAL);
    }
};

// 369317 - Anduin Progression Aura (Can't be reduced < 1 hp)
class spell_anduin_wrynn_progression_aura : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_progression_aura);

public:
    bool Load() override
    {
        return true;
    }

    void CalculateAmount(AuraEffect const* /*aurEff*/, int32& amount, bool& canBeRecalculated)
    {
        amount = -1;
        canBeRecalculated = true;
    }

    void Trigger(AuraEffect* /*aurEff*/, DamageInfo& dmgInfo, uint32& absorbAmount)
    {
        if (!GetTarget()->GetAura(SPELL_ANDUIN_PROGRESSION_AURA))
            return;

        if (dmgInfo.GetDamage() <= GetTarget()->GetHealth())
            return;

        if (dmgInfo.GetDamage() >= GetTarget()->GetHealth())
        {
            absorbAmount = dmgInfo.GetDamage();
            if (_triggered)
                return;

            GetTarget()->SetHealth(1);
            GetTarget()->CastSpell(GetTarget(), SPELL_ANDUIN_KNEEL_POSE);
            GetTarget()->GetAI()->DoAction(ACTION_END_ENCOUNTER);
            GetTarget()->SetFaction(FACTION_FRIENDLY);
            _triggered = true;
        }
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_anduin_wrynn_progression_aura::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(spell_anduin_wrynn_progression_aura::Trigger, EFFECT_0);
    }

private:
    bool _triggered = false;
};

// 366848 - Anduin Willpower Periodic (Only LFR)
class spell_anduin_wrynn_energize_willpower_lfr : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_energize_willpower_lfr);

    static constexpr std::array<int32, 22> AnduinRegenCycle =
    { 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1 }; // 0.681 willpower per second on Retail

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* target = GetTarget();
        _powerGained = AnduinRegenCycle[_powerRegenCycle++];

        if (_powerRegenCycle >= 22)
           _powerRegenCycle = 0;

        target->ModifyPower(target->GetPowerType(), _powerGained);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_anduin_wrynn_energize_willpower_lfr::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }

private:
    uint8 _powerRegenCycle = 0;
    uint32 _powerGained = 0;
};

// Dark Zeal - 364247
class spell_anduin_wrynn_dark_zeal : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_dark_zeal);

    void OnProc(AuraEffect* /*aurEff*/, ProcEventInfo& eventInfo)
    {
        Unit* caster = eventInfo.GetActor();
        Unit* target = eventInfo.GetActionTarget();

        if (!caster || !target)
            return;

        caster->CastSpell(caster, SPELL_DARK_ZEAL_BUFF, true);

        if (target->GetGUID() == _currentTank)
            return;

        caster->RemoveAura(SPELL_DARK_ZEAL_BUFF);
        _currentTank = target->GetGUID();
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_anduin_wrynn_dark_zeal::OnProc, EFFECT_0, SPELL_AURA_DUMMY);
    }

private:
    ObjectGuid _currentTank;
};

// Hopebreaker - 361815
class spell_anduin_wrynn_hopebreaker : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_hopebreaker);

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_HOPEBREAKER_DAMAGE, true);
        GetCaster()->CastSpell(GetHitUnit(), SPELL_HOPEBREAKER_DEBUFF, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_anduin_wrynn_hopebreaker::HandleHit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Hopebreaker Periodic - 361817
class spell_anduin_wrynn_hopebreaker_periodic : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_hopebreaker_periodic);

    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        if (!GetCaster())
            return;

        GetCaster()->CastSpell(GetTarget(), SPELL_HOPEBREAKER_DEBUFF_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_anduin_wrynn_hopebreaker_periodic::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Hopebreaker Damage - 361818
class spell_anduin_wrynn_hopebreaker_damage : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_hopebreaker_damage);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();

        Aura* hopebreaker = target->GetAura(SPELL_HOPEBREAKER_DEBUFF);
        if (!hopebreaker)
            return;

        uint8 hopebreakerStacks = hopebreaker->GetStackAmount();
        SetHitDamage(GetHitDamage() * hopebreakerStacks);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_anduin_wrynn_hopebreaker_damage::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// March of the Damned - 363133, AT - 24093  AT id 88888
struct at_anduin_wrynn_march_of_the_damned : AreaTriggerAI
{
    at_anduin_wrynn_march_of_the_damned(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        Unit* caster = at->GetCaster();
        if (!caster)
            return;

        caster->CastSpell(unit, SPELL_MARCH_OF_THE_DAMNED_DAMAGE, true);
    }
};

// Befouled Barrier - 24332 AT
float constexpr BEFOULED_BARRIER_MAX_RADIUS = 12.0f;
float constexpr BEFOULED_BARRIER_MIN_RADIUS = 4.0f;

struct at_anduin_wrynn_befouled_barrier : AreaTriggerAI
{
    at_anduin_wrynn_befouled_barrier(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnInitialize() override
    {
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_BEFOULED_BARRIER_DEBUFF, at->GetMap()->GetDifficultyID()))
            _absorbRequired = spellInfo->GetEffect(EFFECT_0).CalcValue(at->GetCaster());

        at->SetTimeToTargetScale(30000);

        // The aura is handling it automatically
        at->SetDuration(-1);
    }

    void OnCreate(Spell const* /*creatingSpell*/) override
    {
        _scheduler.Schedule(500ms, [this](TaskContext task)
        {
            float startRadius = 1.0f;

            UpdateSize(startRadius, BEFOULED_BARRIER_MAX_RADIUS);
            at->SetTimeToTargetScale(500);

            task.Schedule(Milliseconds(at->GetTimeToTargetScale()), [this](TaskContext task)
            {
                UpdateSizeBasedOnAbsorb();
                task.Repeat();
            });
        });
    }

    void UpdateSize(float radius, float targetRadius) const
    {
        std::array<DBCPosition2D, 2> points =
        { {
            { 0.0f, radius },
            { 1.0f, targetRadius }
        } };

        at->SetOverrideScaleCurve(points);
    }

    void UpdateSizeBasedOnAbsorb()
    {
        Unit* target = at->GetTarget();
        if (!target)
            return;

        float radiusMod = 1.0f - (_absorbDone / (float)_absorbRequired);
        float targetRadius = BEFOULED_BARRIER_MAX_RADIUS * radiusMod;
        float currentRadius = at->GetMaxSearchRadius();

        if (G3D::fuzzyEq(currentRadius, targetRadius))
            return;

        if (targetRadius <= BEFOULED_BARRIER_MIN_RADIUS)
        {
            // Players need to be able to actually enter the AT to heal it no sniff has a value lower than 4.0f
            return;
        }

        UpdateSize(currentRadius, targetRadius);
    }

    void OnUpdate(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

    void OnUnitEnter(Unit* unit) override
    {
        Unit* caster = at->GetCaster();
        if (!caster)
            return;

        caster->CastSpell(unit, SPELL_BEFOULED_BARRIER_DEBUFF, true);
    }

    void OnUnitExit(Unit* unit) override
    {
        unit->RemoveAura(SPELL_BEFOULED_BARRIER_DEBUFF);
    }

    void Absorb(uint32 absorbAmount)
    {
        _absorbDone += absorbAmount;

        if (_absorbDone < _absorbRequired)
            return;

        Unit* target = at->GetTarget();
        if (!target)
            return;

        // remove barrier, we're done healing
        if (Creature* creature = target->ToCreature())
        {
            creature->CastSpell(creature, SPELL_BEFOULED_BARRIER_CLEAR, true);
            creature->DespawnOrUnsummon();
        }
    }

    uint32 GetRemainingAbsorb()
    {
        return _absorbRequired - _absorbDone;
    }

private:
    uint32 _absorbDone = 0;
    uint32 _absorbRequired = 0;
    TaskScheduler _scheduler;
};

// Beacon of Hope - 25025 / 28469 365872
float constexpr BEACON_OF_HOPE_MAX_RADIUS = 12.0f;
float constexpr BEACON_OF_HOPE_MIN_RADIUS = 4.0f;
struct at_anduin_wrynn_beacon_of_hope : AreaTriggerAI
{
    at_anduin_wrynn_beacon_of_hope(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger),
        _instance(at->GetInstanceScript()) { }

    void OnInitialize() override
    {
        _entries = 0;
        _chargesRemaining = 40;
    }

    void OnCreate(Spell const* /*creatingSpell*/) override
    {
        _scheduler.Schedule(500ms, [this](TaskContext task)
        {
            float startRadius = 1.0f;
            float targetRadius = BEACON_OF_HOPE_MAX_RADIUS;

            UpdateSize(startRadius, targetRadius);
            at->SetTimeToTargetScale(500);

            task.Schedule(Milliseconds(at->GetTimeToTargetScale()), [this](TaskContext task)
            {
                UpdateSizeBasedOnCharges();
                task.Repeat();
            });
        });
    }

    void UpdateSize(float radius, float targetRadius) const
    {
        std::array<DBCPosition2D, 2> points =
        { {
            { 0.0f, radius },
            { 1.0f, targetRadius }
        } };

        at->SetOverrideScaleCurve(at->GetTimeSinceCreated());
        at->SetOverrideScaleCurve(points);
    }

    void UpdateSizeBasedOnCharges()
    {
        float radiusMod = 0.205129f * _entries;
        float targetRadius = BEACON_OF_HOPE_MAX_RADIUS - radiusMod;

        if (targetRadius <= BEACON_OF_HOPE_MIN_RADIUS || _chargesRemaining <= 0)
            at->Remove();

        float currentRadius = at->GetMaxSearchRadius();
        if (G3D::fuzzyEq(currentRadius, targetRadius))
            return;

        UpdateSize(currentRadius, targetRadius);
    }

    void OnUpdate(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        if (!at->GetCaster())
            return;

        if (Creature* beacon = _instance->GetCreature(DATA_BEACON_OF_HOPE))
        {
            beacon->CastSpell(beacon, SPELL_PURGING_LIGHT, true);
            unit->RemoveAurasDueToSpell(SPELL_HOPELESSNESS_HOPELESSNESS_AREATRIGGER);
        }

        if (at->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
        {
            _entries++;
            _chargesRemaining--;
            if (Creature* beacon = _instance->GetCreature(DATA_BEACON_OF_HOPE))
                beacon->CastSpell(unit, SPELL_FRAGMENT_OF_HOPE_AREATRIGGER, true);
        }
    }

private:
    uint8 _chargesRemaining = 40;
    uint8 _entries = 0;
    TaskScheduler _scheduler;
    InstanceScript* _instance;
};

// Fragment of Hope - 365816
class spell_anduin_wrynn_fragment_of_hope : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_fragment_of_hope);

    void SetDest(SpellDestination& destination)
    {
        Unit* caster = GetCaster();
        Position dest = caster->GetPosition();

        if (Creature* beacon = caster->GetInstanceScript()->GetCreature(DATA_BEACON_OF_HOPE))
            beacon->MovePositionToFirstCollision(dest, 30.0f, beacon->GetAbsoluteAngle(GetExplTargetWorldObject()) - beacon->GetOrientation());

        destination.Relocate(dest);
    }

    void Register() override
    {
        OnDestinationTargetSelect += SpellDestinationTargetSelectFn(spell_anduin_wrynn_fragment_of_hope::SetDest, EFFECT_0, TARGET_DEST_DEST);
    }
};

// 365293 - Befouled Barrier Absorb 365295
class spell_anduin_wrynn_befouled_barrier_absorb : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_befouled_barrier_absorb);

    void OnHealAbsorb(AuraEffect* /*aurEff*/, HealInfo& healInfo, uint32& absorbAmount)
    {
        absorbAmount = CalculatePct(healInfo.GetHeal(), GetEffectInfo(EFFECT_2).CalcValue());

        Unit* caster = GetCaster();
        if (!caster)
            return;

        AreaTrigger* barrier = caster->GetAreaTrigger(SPELL_BEFOULED_BARRIER_SPHERE_AREATRIGGER);
        if (!barrier)
            return;

        at_anduin_wrynn_befouled_barrier* barrierScript = dynamic_cast<at_anduin_wrynn_befouled_barrier*>(barrier->AI());
        if (!barrierScript)
            return;

        barrierScript->Absorb(absorbAmount);
    }

    void Register() override
    {
        OnEffectAbsorbHeal += AuraEffectAbsorbHealFn(spell_anduin_wrynn_befouled_barrier_absorb::OnHealAbsorb, EFFECT_0);
    }
};

// Befouled Barrier Expire - 365173
class spell_anduin_wrynn_befouled_barrier_expire : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_befouled_barrier_expire);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
        {
            AreaTrigger* barrier = GetTarget()->GetAreaTrigger(SPELL_BEFOULED_BARRIER_SPHERE_AREATRIGGER);
            if (!barrier)
                return;

            at_anduin_wrynn_befouled_barrier* barrierScript = dynamic_cast<at_anduin_wrynn_befouled_barrier*>(barrier->AI());
            if (!barrierScript)
                return;

            target->CastSpell(target, SPELL_BEFOULED_BARRIER_EXPLODE, CastSpellExtraArgs().AddSpellMod(SPELLVALUE_BASE_POINT1, barrierScript->GetRemainingAbsorb()));
        }

        if (Creature* creatureTarget = target->ToCreature())
            creatureTarget->DespawnOrUnsummon();
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_anduin_wrynn_befouled_barrier_expire::OnRemove, EFFECT_0, SPELL_AURA_AREA_TRIGGER, AURA_EFFECT_HANDLE_REAL);
    }
};

// Blasphemy - 361989
class spell_anduin_wrynn_blasphemy : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_blasphemy);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_BLASPHEMY_PRE_HIT, SPELL_BLASPHEMY_OVERCONFIDENCE, SPELL_BLASPHEMY_HOPELESSNESS });
    }

    void OnPrecast() override
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_BLASPHEMY_PRE_HIT, true);
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* target) -> bool
        {
            if (!target->IsPlayer())
                return true;

            return false;
        });

        int8 hopelessnessAffected = static_cast<int8>(std::floor(targets.size() / 2));
        int8 maxAffected = hopelessnessAffected * 2;
        Trinity::Containers::RandomResize(targets, maxAffected);

        for (WorldObject* target : targets)
        {
            if (hopelessnessAffected > 0)
            {
                _spellAssignments[target->GetGUID()] = SPELL_BLASPHEMY_HOPELESSNESS;
                hopelessnessAffected--;
            }
            else
                _spellAssignments[target->GetGUID()] = SPELL_BLASPHEMY_OVERCONFIDENCE;

            maxAffected--;
        }
    }

    void HandleDebuff(SpellEffIndex /*effIndex*/)
    {
        uint32* spellId = Trinity::Containers::MapGetValuePtr(_spellAssignments, GetHitUnit()->GetGUID());
        if (!spellId)
            return;

        GetCaster()->CastSpell(GetHitUnit(), *spellId, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anduin_wrynn_blasphemy::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_anduin_wrynn_blasphemy::HandleDebuff, EFFECT_0, SPELL_EFFECT_DUMMY);
    }

private:
    std::unordered_map<ObjectGuid /*player*/, uint32 /*spell*/> _spellAssignments;
};

// Blasphemy ATs 361992 Overconfidence | 361993 Hopelessness
struct at_anduin_wrynn_blasphemy : AreaTriggerAI
{
    at_anduin_wrynn_blasphemy(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger)  { }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer() || unit->HasAura(SPELL_BLASPHEMY_PRE_HIT))
            return;

        if (!unit->IsAlive())
            at->Remove();

        if (unit->HasAura(at->GetSpellId()))
        {
            uint32 explodeSpellId = SPELL_BLASPHEMY_EXPLODE;
            if (at->GetMap()->GetDifficultyID() == DIFFICULTY_LFR_NEW || at->GetMap()->GetDifficultyID() == DIFFICULTY_NORMAL_RAID)
                explodeSpellId = SPELL_BLASPHEMY_EXPLODE_LFR_NORMAL;

            unit->CastSpell(unit, explodeSpellId, true);
            unit->RemoveAurasDueToSpell(at->GetSpellId());
            if (Unit* target = at->GetTarget())
            {
                target->CastSpell(target, explodeSpellId, true);
                target->RemoveAurasDueToSpell(at->GetSpellId());
            }
        }
        else if (at->GetSpellId() == SPELL_BLASPHEMY_HOPELESSNESS_AREATRIGGER && unit->HasAura(SPELL_BLASPHEMY_OVERCONFIDENCE_AREATRIGGER))
        {
            unit->RemoveAurasDueToSpell(SPELL_BLASPHEMY_OVERCONFIDENCE);
            unit->RemoveAurasDueToSpell(SPELL_BLASPHEMY_OVERCONFIDENCE_AREATRIGGER);
            unit->CastSpell(unit, SPELL_BLASPHEMY_SUCCESS, true);
            if (Unit* target = at->GetTarget())
            {
                target->RemoveAurasDueToSpell(SPELL_BLASPHEMY_HOPELESSNESS);
                target->RemoveAurasDueToSpell(SPELL_BLASPHEMY_HOPELESSNESS_AREATRIGGER);
                target->CastSpell(target, SPELL_BLASPHEMY_SUCCESS, true);
            }
        }
        else if (at->GetSpellId() == SPELL_BLASPHEMY_OVERCONFIDENCE_AREATRIGGER && unit->HasAura(SPELL_BLASPHEMY_HOPELESSNESS_AREATRIGGER))
        {
            unit->RemoveAurasDueToSpell(SPELL_BLASPHEMY_HOPELESSNESS);
            unit->RemoveAurasDueToSpell(SPELL_BLASPHEMY_HOPELESSNESS_AREATRIGGER);
            unit->CastSpell(unit, SPELL_BLASPHEMY_SUCCESS, true);
            if (Unit* target = at->GetTarget())
            {
                target->RemoveAurasDueToSpell(SPELL_BLASPHEMY_OVERCONFIDENCE);
                target->RemoveAurasDueToSpell(SPELL_BLASPHEMY_OVERCONFIDENCE_AREATRIGGER);
                target->CastSpell(target, SPELL_BLASPHEMY_SUCCESS, true);
            }
        }
        else
        {
            if (at->GetMap()->GetDifficultyID() != DIFFICULTY_MYTHIC_RAID) // On Mythic players walking in with no debuff also trigger explosion
                return;

            if (!unit->HasAura(SPELL_BLASPHEMY_HOPELESSNESS_AREATRIGGER) || !unit->HasAura(SPELL_BLASPHEMY_OVERCONFIDENCE_AREATRIGGER))
            {
                if (Unit* target = at->GetTarget())
                    target->CastSpell(target, SPELL_BLASPHEMY_EXPLODE, true);
            }
        }
    }
};

// Blasphemy Expire - 361992, 361993
class spell_anduin_wrynn_hopelessness_overconfidence : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_hopelessness_overconfidence);

    bool Validate(SpellInfo const* /*spell*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_BLASPHEMY_EXPLODE,
            SPELL_BLASPHEMY_EXPLODE_LFR_NORMAL
        });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
        {
            Unit* target = GetTarget();
            uint32 explodeSpellId = SPELL_BLASPHEMY_EXPLODE;
            if (target->GetMap()->GetDifficultyID() == DIFFICULTY_LFR_NEW || target->GetMap()->GetDifficultyID() == DIFFICULTY_NORMAL_RAID)
                explodeSpellId = SPELL_BLASPHEMY_EXPLODE_LFR_NORMAL;

            target->CastSpell(target, explodeSpellId, true);
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_anduin_wrynn_hopelessness_overconfidence::OnRemove, EFFECT_0, SPELL_AURA_AREA_TRIGGER, AURA_EFFECT_HANDLE_REAL);
    }
};

// Wicked Star Selector & Pointer - 365021
// Empowered Wicked Star Selector & Pointer - 367632
class spell_anduin_wrynn_wicked_star_selector : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_wicked_star_selector);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        uint32 rangedDpsCount = 0;
        for (WorldObject* target : targets)
        {
            Player* targetPlayer = target->ToPlayer();
            if (!targetPlayer)
                continue;

            ChrSpecializationEntry const* spec = targetPlayer->GetPrimarySpecializationEntry();
            if (!spec)
                continue;

            if (!targetPlayer->IsAlive())
                continue;

            if ((spec->GetRole() == ChrSpecializationRole::Dps && spec->GetFlags().HasFlag(ChrSpecializationFlag::Caster)) ||
                (spec->GetRole() == ChrSpecializationRole::Dps && spec->GetFlags().HasFlag(ChrSpecializationFlag::Ranged)))
                rangedDpsCount++;
        }

        targets.remove_if([rangedDpsCount](WorldObject* target) -> bool
        {
            Player* player = target->ToPlayer();
            if (!player || player->HasAura(SPELL_WICKED_STAR_TARGETED))
                return true;

            if (player->GetPrimarySpecializationEntry()->GetRole() == ChrSpecializationRole::Tank)
                return true;

            if (rangedDpsCount >= 3)
            {
                if (player->GetPrimarySpecializationEntry()->GetRole() == ChrSpecializationRole::Dps && player->GetPrimarySpecializationEntry()->GetFlags().HasFlag(ChrSpecializationFlag::Melee))
                    return true;

                if (player->GetPrimarySpecializationEntry()->GetRole() == ChrSpecializationRole::Healer)
                    return true;
            }
            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anduin_wrynn_wicked_star_selector::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// Wicked Star Selector & Pointer - 365021
class spell_anduin_wrynn_wicked_star_selector_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_wicked_star_selector_AuraScript);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WICKED_STAR_TARGETED });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster)
            return;

        caster->CastSpell(target, SPELL_WICKED_STAR_TARGETED, true);

        if (Creature* creature = caster->ToCreature())
            if (CreatureAI* anduinAI = creature->AI())
                anduinAI->Talk(SAY_ANNOUNCE_WICKED_STAR, target);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        float angle = caster->GetAbsoluteAngle(GetTarget());
        Position spellDest(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), angle);

        if (InstanceScript* instance = GetTarget()->GetInstanceScript())
        {
            if (Creature* anduin = instance->GetCreature(DATA_ANDUIN_WRYNN))
                anduin->CastSpell(spellDest, SPELL_WICKED_STAR_AREATRIGGER, true);
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_anduin_wrynn_wicked_star_selector_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_anduin_wrynn_wicked_star_selector_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
    }
};

// Wicked Star - 365017 - AT 24741
struct at_anduin_wrynn_wicked_star : AreaTriggerAI
{
    at_anduin_wrynn_wicked_star(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnInitialize() override
    {
        if (Unit* caster = at->GetCaster())
        {
            uint32 starLaunchSpeed = 0;

            switch (at->GetMap()->GetDifficultyID())
            {
                case DIFFICULTY_LFR_NEW:
                    starLaunchSpeed = 60;
                    break;
                case DIFFICULTY_NORMAL_RAID:
                    starLaunchSpeed = 50;
                    break;
                case DIFFICULTY_HEROIC_RAID:
                    starLaunchSpeed = 40;
                    break;
                case DIFFICULTY_MYTHIC_RAID:
                    starLaunchSpeed = 30;
                    break;
                default:
                    starLaunchSpeed = 50;
                    break;
            }

            _casterInitialPosition = caster->GetPosition();
            float wickedStarXOffSet = 100.0f;
            Position destPos = _casterInitialPosition;

            at->MovePositionToFirstCollision(destPos, wickedStarXOffSet, 0.0f);

            PathGenerator firstPath(at);
            firstPath.CalculatePath(destPos.GetPositionX(), destPos.GetPositionY(), destPos.GetPositionZ(), false);
            G3D::Vector3 const& endPoint = firstPath.GetPath().back();

            at->InitSplines(firstPath.GetPath(), static_cast<uint32>(at->GetDistance(endPoint.x, endPoint.y, endPoint.z) * starLaunchSpeed));
        }
    }

    void OnUpdate(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (std::find(_affectedUnits.begin(), _affectedUnits.end(), unit->GetGUID()) == _affectedUnits.end())
            {
                bool excludeUnit = false;
                if (unit->GetEntry() == NPC_SYLVANAS_WINDRUNNER || unit->GetEntry() == NPC_UTHER_THE_LIGHTBRINGER ||
                    unit->GetEntry() == NPC_LADY_JAINA_PROUDMOORE || unit->GetEntry() == BOSS_ANDUIN_WRYNN)
                    excludeUnit = true;

                if (!excludeUnit && caster->IsValidAttackTarget(unit))
                    caster->CastSpell(unit, SPELL_WICKED_STAR_DAMAGE_SILENCE, CastSpellExtraArgs(TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS)));

                if (!excludeUnit && caster->IsValidAssistTarget(unit))
                    caster->CastSpell(unit, SPELL_WICKED_STAR_EMPOWERMENT, CastSpellExtraArgs(TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS)));

                _affectedUnits.push_back(unit->GetGUID());
            }
        }
    }

    void OnUnitExit(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (std::find(_affectedUnits.begin(), _affectedUnits.end(), unit->GetGUID()) == _affectedUnits.end())
            {
                bool excludeUnit = false;
                if (unit->GetEntry() == NPC_SYLVANAS_WINDRUNNER || unit->GetEntry() == NPC_UTHER_THE_LIGHTBRINGER ||
                    unit->GetEntry() == NPC_LADY_JAINA_PROUDMOORE || unit->GetEntry() == BOSS_ANDUIN_WRYNN)
                    excludeUnit = true;

                if (!excludeUnit && caster->IsValidAttackTarget(unit))
                    caster->CastSpell(unit, SPELL_WICKED_STAR_DAMAGE_SILENCE, CastSpellExtraArgs(TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS)));

                if (!excludeUnit && caster->IsValidAssistTarget(unit))
                    caster->CastSpell(unit, SPELL_WICKED_STAR_EMPOWERMENT, CastSpellExtraArgs(TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS)));

                _affectedUnits.push_back(unit->GetGUID());
            }
        }
    }

    void OnDestinationReached() override
    {
        Unit* caster = at->GetCaster();
        if (!caster)
            return;

        if (at->GetDistance(_casterInitialPosition) > 0.05f)
        {
            _affectedUnits.clear();

            ReturnToCaster();
        }
        else
            at->Remove();
    }

    void ReturnToCaster()
    {
        uint32 starReturnSpeed = 10;

        switch (at->GetMap()->GetDifficultyID())
        {
            case DIFFICULTY_LFR_NEW:
                starReturnSpeed = 8;
                break;
            case DIFFICULTY_NORMAL_RAID:
                starReturnSpeed = 10;
                break;
            case DIFFICULTY_HEROIC_RAID:
                starReturnSpeed = 12;
                break;
            case DIFFICULTY_MYTHIC_RAID:
                starReturnSpeed = 14;
                break;
            default:
                starReturnSpeed = 10;
                break;
        }

        _scheduler.Schedule(0ms, [this, starReturnSpeed](TaskContext task)
        {
            Movement::PointsArray returnSplinePoints;

            returnSplinePoints.push_back(PositionToVector3(at));
            returnSplinePoints.push_back(PositionToVector3(at));
            returnSplinePoints.push_back(PositionToVector3(_casterInitialPosition));
            returnSplinePoints.push_back(PositionToVector3(_casterInitialPosition));

            at->InitSplines(returnSplinePoints, static_cast<uint32>(at->GetDistance(_casterInitialPosition) / starReturnSpeed * 600));
            task.Repeat(250ms);
        });
    }

private:
    TaskScheduler _scheduler;
    Position _casterInitialPosition;
    std::vector<ObjectGuid> _affectedUnits;
};

// Empowered Wicked Star Selector & Pointer - 367632
class spell_anduin_wrynn_empowered_wicked_star_selector_AuraScript : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_WICKED_STAR_TARGETED });
    }

    PrepareAuraScript(spell_anduin_wrynn_empowered_wicked_star_selector_AuraScript);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster)
            return;

        caster->CastSpell(target, SPELL_WICKED_STAR_TARGETED, true);

        if (Creature* creature = caster->ToCreature())
            if (CreatureAI* anduinAI = creature->AI())
                anduinAI->Talk(SAY_ANNOUNCE_EMPOWERED_WICKED_STAR, target);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        float angle = caster->GetAbsoluteAngle(GetTarget());
        Position spellDest(caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), angle);

        if (InstanceScript* instance = GetTarget()->GetInstanceScript())
        {
            if (Creature* anduin = instance->GetCreature(DATA_ANDUIN_WRYNN))
                anduin->CastSpell(spellDest, SPELL_EMPOWERED_WICKED_STAR_AREATRIGGER, true);
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_anduin_wrynn_empowered_wicked_star_selector_AuraScript::OnApply, EFFECT_0, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_anduin_wrynn_empowered_wicked_star_selector_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_MOD_SCALE, AURA_EFFECT_HANDLE_REAL);
    }
};

// Empowered Wicked Star 367621  AT - 24599
struct at_anduin_wrynn_empowered_wicked_star : AreaTriggerAI
{
    at_anduin_wrynn_empowered_wicked_star(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnInitialize() override
    {
        if (Unit* caster = at->GetCaster())
        {
            _casterInitialPosition = caster->GetPosition();
            // Note: max distance irrelevant since it will bounce the moment it reaches the boundary
            float wickedStarXOffSet = 100.0f;
            Position destPos = _casterInitialPosition;

            at->MovePositionToFirstCollision(destPos, wickedStarXOffSet, 0.0f);

            PathGenerator firstPath(at);
            firstPath.CalculatePath(destPos.GetPositionX(), destPos.GetPositionY(), destPos.GetPositionZ(), false);
            G3D::Vector3 const& endPoint = firstPath.GetPath().back();

            at->InitSplines(firstPath.GetPath(), static_cast<uint32>(at->GetDistance(endPoint.x, endPoint.y, endPoint.z) * 50));
        }
    }

    void OnUpdate(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

    void OnUnitEnter(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (std::find(_affectedUnits.begin(), _affectedUnits.end(), unit->GetGUID()) == _affectedUnits.end())
            {
                bool excludeUnit = false;
                if (unit->GetEntry() == NPC_SYLVANAS_WINDRUNNER || unit->GetEntry() == NPC_UTHER_THE_LIGHTBRINGER ||
                    unit->GetEntry() == NPC_LADY_JAINA_PROUDMOORE || unit->GetEntry() == BOSS_ANDUIN_WRYNN)
                    excludeUnit = true;

                if (!excludeUnit && caster->IsValidAttackTarget(unit))
                    caster->CastSpell(unit, SPELL_WICKED_STAR_DAMAGE_SILENCE, CastSpellExtraArgs(TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS)));
                if (!excludeUnit && caster->IsValidAssistTarget(unit))
                    caster->CastSpell(unit, SPELL_WICKED_STAR_EMPOWERMENT, CastSpellExtraArgs(TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS)));

                _affectedUnits.push_back(unit->GetGUID());
            }
        }
    }

    void OnUnitExit(Unit* unit) override
    {
        if (Unit* caster = at->GetCaster())
        {
            if (std::find(_affectedUnits.begin(), _affectedUnits.end(), unit->GetGUID()) == _affectedUnits.end())
            {
                bool excludeUnit = false;

                if (unit->GetEntry() == NPC_SYLVANAS_WINDRUNNER || unit->GetEntry() == NPC_UTHER_THE_LIGHTBRINGER ||
                    unit->GetEntry() == NPC_LADY_JAINA_PROUDMOORE || unit->GetEntry() == BOSS_ANDUIN_WRYNN)
                    excludeUnit = true;

                if (!excludeUnit && caster->IsValidAttackTarget(unit))
                    caster->CastSpell(unit, SPELL_WICKED_STAR_DAMAGE_SILENCE, CastSpellExtraArgs(TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS)));
                if (!excludeUnit && caster->IsValidAssistTarget(unit))
                    caster->CastSpell(unit, SPELL_WICKED_STAR_EMPOWERMENT, CastSpellExtraArgs(TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_CAST_IN_PROGRESS)));

                _affectedUnits.push_back(unit->GetGUID());
            }
        }
    }

    void OnDestinationReached() override
    {
        Unit* caster = at->GetCaster();
        if (!caster)
            return;

        if (at->GetDistance(_casterInitialPosition) > 0.05f)
        {
            _affectedUnits.clear();
            if (at->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
                NextTrajectory();
            else
                ReturnToCaster();
        }
        else
            at->Remove();
    }

    void NextTrajectory()
    {
        _scheduler.Schedule(0ms, [this](TaskContext /*task*/)
        {
            if (at->GetCaster()->GetFaction() == FACTION_FRIENDLY)
            {
                at->Remove();
            }
            else if (Unit* caster = at->GetCaster())
            {
                Position wallPosition(DominationGraspCenter);
                caster->MovePositionToFirstCollision(wallPosition, 50.0f, frand(0, 2.0f * float(M_PI)) - caster->GetOrientation());

                Movement::PointsArray returnSplinePoints;
                returnSplinePoints.push_back(PositionToVector3(at));
                returnSplinePoints.push_back(PositionToVector3(at));
                returnSplinePoints.push_back(PositionToVector3(wallPosition));
                returnSplinePoints.push_back(PositionToVector3(wallPosition));

                at->InitSplines(returnSplinePoints, static_cast<uint32>(at->GetDistance(wallPosition) / 8 * 600));
            }
        });
    }

    void ReturnToCaster()
    {
        uint32 starReturnSpeed = 10;

        switch (at->GetMap()->GetDifficultyID())
        {
            case DIFFICULTY_LFR_NEW:
                starReturnSpeed = 8;
                break;
            case DIFFICULTY_NORMAL_RAID:
                starReturnSpeed = 10;
                break;
            case DIFFICULTY_HEROIC_RAID:
                starReturnSpeed = 12;
                break;
            case DIFFICULTY_MYTHIC_RAID:
                starReturnSpeed = 14;
                break;
            default:
                starReturnSpeed = 10;
                break;
        }

        _scheduler.Schedule(0ms, [this, starReturnSpeed](TaskContext task)
        {
            Movement::PointsArray returnSplinePoints;

            returnSplinePoints.push_back(PositionToVector3(at));
            returnSplinePoints.push_back(PositionToVector3(at));
            returnSplinePoints.push_back(PositionToVector3(_casterInitialPosition));
            returnSplinePoints.push_back(PositionToVector3(_casterInitialPosition));

            at->InitSplines(returnSplinePoints, static_cast<uint32>(at->GetDistance(_casterInitialPosition) / starReturnSpeed * 600));
            task.Repeat(250ms);
        });
    }

private:
    TaskScheduler _scheduler;
    Position _casterInitialPosition;
    std::vector<ObjectGuid> _affectedUnits;
};

// Kingsmourne Hungers - 362405
class spell_anduin_wrynn_kingsmourne_hungers : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_kingsmourne_hungers);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_KINGSMOURNE_HUNGERS_DAMAGE,
            SPELL_LOST_SOUL_DIMENSION,
            SPELL_LOST_SOUL,
            SPELL_SEVERED_SOUL
        });
    }

    // Prevent swinging empty-handed when casts overlap
    void OnPreCast()
    {
        GetCaster()->PlayOneShotAnimKitId(ANIM_SHEATHE);
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* target) -> bool
        {
            if (target->GetEntry() == NPC_SYLVANAS_WINDRUNNER ||
                target->GetEntry() == NPC_UTHER_THE_LIGHTBRINGER ||
                target->GetEntry() == NPC_LADY_JAINA_PROUDMOORE)
                return true;
            return false;
        });
    }

    void HandleDummyEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* hitUnit = GetHitUnit();
        caster->CastSpell(hitUnit, SPELL_KINGSMOURNE_HUNGERS_DAMAGE, true);
        if (caster->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
            hitUnit->CastSpell(hitUnit, SPELL_SEVERED_SOUL, true);
        caster->CastSpell(hitUnit, SPELL_LOST_SOUL_DIMENSION, false);
        hitUnit->CastSpell(hitUnit, SPELL_LOST_SOUL, true);
    }

    void OnCast()
    {
        InstanceScript* instance = GetCaster()->GetInstanceScript();
        if (!instance)
            return;

        if (Creature* anduinSoul = instance->GetCreature(DATA_ANDUIN_SOUL))
            anduinSoul->GetAI()->DoAction(ACTION_SUMMON_KINGSMOURNE_SOULS);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anduin_wrynn_kingsmourne_hungers::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_180_DEG_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_anduin_wrynn_kingsmourne_hungers::HandleDummyEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
        AfterCast += SpellCastFn(spell_anduin_wrynn_kingsmourne_hungers::OnCast);
    }
};

// Lost Soul Dimension - 362055
class spell_anduin_wrynn_lost_soul : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_lost_soul);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_LOST_SOUL_GRACE,
            SPELL_SCARRED_SOUL
        });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        _lostSoulPosition = target->GetPosition();
        target->CastSpell(target, SPELL_LOST_SOUL_GRACE, true);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        switch (GetTargetApplication()->GetRemoveMode())
        {
            case AURA_REMOVE_BY_DEFAULT:
            case AURA_REMOVE_BY_ENEMY_SPELL:
                target->CastSpell(target, SPELL_SCARRED_SOUL, true);
                break;
            case AURA_REMOVE_BY_EXPIRE:
            {
                if (target->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
                {
                    target->KillSelf();
                    target->NearTeleportTo(_lostSoulPosition, false);
                }
                break;
            }
            case AURA_REMOVE_BY_DEATH:
            {
                target->CastSpell(target, SPELL_SCARRED_SOUL, true);
                if (target->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID)
                    target->NearTeleportTo(_lostSoulPosition, false);
                break;
            }
            default:
                return;
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_anduin_wrynn_lost_soul::OnApply, EFFECT_0, SPELL_AURA_PHASE, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_anduin_wrynn_lost_soul::OnRemove, EFFECT_1, SPELL_AURA_SCREEN_EFFECT, AURA_EFFECT_HANDLE_REAL);
    }

private:
    Position _lostSoulPosition;
};

// Rain of Despair - 362392
class spell_anduin_rain_of_despair_player_selector : public SpellScript
{
    PrepareSpellScript(spell_anduin_rain_of_despair_player_selector);

    void HandleDespair(SpellEffIndex effIndex)
    {
        GetCaster()->CastSpell(GetHitUnit(), GetEffectInfo(effIndex).TriggerSpell, true);
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_anduin_rain_of_despair_player_selector::HandleDespair, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Soul Despawn - 362766
class spell_anduin_wrynn_soul_despawn : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_soul_despawn);

    void OnCast()
    {
        if (Creature* creature = GetCaster()->ToCreature())
        {
            if (creature->GetEntry() == NPC_ANDUIN_HOPE || creature->GetEntry() == NPC_ANDUIN_DOUBT)
                creature->DespawnOrUnsummon(3s);
        }

    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_anduin_wrynn_soul_despawn::OnCast);
    }
};

// Force of Will - 368913
class spell_anduin_wrynn_force_of_will : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_force_of_will);

    void RecalculateHook(AuraEffect const* /*aurEffect*/, int32& amount, bool& canBeRecalculated)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        // at 100 will power = 200% Damage Done increase
        int32 powerValue = caster->GetPower(caster->GetPowerType());
        amount = 2 * powerValue;
        canBeRecalculated = false;
    }

    void RecalculateHookDamageTaken(AuraEffect const* /*aurEffect*/, int32& amount, bool& canBeRecalculated)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        // Damage Taken reduction can only be capped to 90%
        int32 powerValue = caster->GetPower(caster->GetPowerType());
        amount = -std::min(powerValue, 90);
        canBeRecalculated = false;
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->SetPower(caster->GetPowerType(), 0);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_anduin_wrynn_force_of_will::OnApply, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_anduin_wrynn_force_of_will::RecalculateHook, EFFECT_0, SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_anduin_wrynn_force_of_will::RecalculateHook, EFFECT_1, SPELL_AURA_MOD_SUMMON_DAMAGE);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_anduin_wrynn_force_of_will::RecalculateHookDamageTaken, EFFECT_2, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    }
};

// March of the Damned - 363233
class spell_anduin_wrynn_march_of_the_damned : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_march_of_the_damned);

public:
    spell_anduin_wrynn_march_of_the_damned()
    {
        _availableSpawnPositions.assign(std::begin(MarchOfTheDamnedSpawnPositions), std::end(MarchOfTheDamnedSpawnPositions));
    }

    void OnPeriodic(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        uint8 raidDifficulty = 0;

        switch (GetCaster()->GetMap()->GetDifficultyID())
        {
            case DIFFICULTY_LFR_NEW:
            case DIFFICULTY_NORMAL_RAID:
            case DIFFICULTY_HEROIC_RAID:
                raidDifficulty = 28;
                break;
            case DIFFICULTY_MYTHIC_RAID:
                raidDifficulty = 21;
                break;
            default:
                raidDifficulty = 28;
                break;
        }

        // Don't summon the wall on aura granted
        if (aurEff->GetTickNumber() == 1)
            return;

        if (_availableSpawnPositions.empty())
            return;

        Position chosenPosition = Trinity::Containers::SelectRandomContainerElement(_availableSpawnPositions);
        auto it = std::find(_availableSpawnPositions.begin(), _availableSpawnPositions.end(), chosenPosition);
        _availableSpawnPositions.erase(it);

        GetCaster()->SummonCreature(NPC_MARCH_OF_THE_DAMNED, chosenPosition, TEMPSUMMON_TIMED_DESPAWN, Seconds(raidDifficulty));
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_anduin_wrynn_march_of_the_damned::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }

private:
    std::vector<Position> _availableSpawnPositions;
};

// Remnant Aura - 362500
class spell_remnant_of_a_fallen_king_spawn : public AuraScript
{
    PrepareAuraScript(spell_remnant_of_a_fallen_king_spawn);

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
            GetTarget()->GetAI()->DoAction(ACTION_ACTIVATE_REMNANT);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_remnant_of_a_fallen_king_spawn::OnRemove, EFFECT_2, SPELL_AURA_TRIGGER_SPELL_ON_EXPIRE, AURA_EFFECT_HANDLE_REAL);
    }
};

// Anduin's Soul - Lost Soul 365652
class spell_anduin_soul_lost_soul : public SpellScript
{
    PrepareSpellScript(spell_anduin_soul_lost_soul);

    void FilterTargets(std::list<WorldObject*>& unitList)
    {
        unitList.remove_if([this](WorldObject* target)
        {
            if (target->GetEntry() == NPC_ANDUIN_DOUBT && target->IsWithinDistInMap(GetCaster(), 0.2f))
                return false;
            return true;
        });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Creature* creature = GetHitUnit()->ToCreature())
            creature->DespawnOrUnsummon();
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anduin_soul_lost_soul::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnEffectHitTarget += SpellEffectFn(spell_anduin_soul_lost_soul::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Severed Soul - 367769
class spell_anduin_wrynn_severed_soul : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_severed_soul);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_CANCEL_LOST_SOUL
        });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            _soulPosition = caster->GetPosition();
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_CANCEL_LOST_SOUL, true);

        if (TempSummon* summon = GetTarget()->ToTempSummon())
        {
            if (summon->GetSummonerUnit() == caster)
                caster->NearTeleportTo(_soulPosition, false);
        }
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_anduin_wrynn_severed_soul::OnApply, EFFECT_0, SPELL_AURA_LINKED_SUMMON, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_anduin_wrynn_severed_soul::OnRemove, EFFECT_1, SPELL_AURA_TRIGGER_SPELL_ON_EXPIRE, AURA_EFFECT_HANDLE_REAL);
    }

private:
    Position _soulPosition;
};

// Lost Soul Mirror Image - 362402
class spell_anduin_wrynn_lost_soul_mirror_image : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_lost_soul_mirror_image);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        ObjectGuid casterGuid = caster->GetGUID();

        targets.remove_if([casterGuid](WorldObject* target) -> bool
        {
            Unit* unit = target->ToUnit();
            if (!unit)
                return true;

            TempSummon* summon = unit->ToTempSummon();
            if (!summon)
                return true;

            if (summon->GetSummonerGUID() != casterGuid)
                return true;

            return false;
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anduin_wrynn_lost_soul_mirror_image::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anduin_wrynn_lost_soul_mirror_image::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anduin_wrynn_lost_soul_mirror_image::FilterTargets, EFFECT_2, TARGET_UNIT_SRC_AREA_ENTRY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_anduin_wrynn_lost_soul_mirror_image::FilterTargets, EFFECT_3, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

// Soul Explosion - 363029
class spell_friendish_soul_explosion : public SpellScript
{
    PrepareSpellScript(spell_friendish_soul_explosion);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_SOUL_EXPLOSION_TRIGGER_MISSILE
        });
    }

private:
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        targets.remove_if([](WorldObject* target)
        {
            Player* player = target->ToPlayer();
            if (!player)
                return true;

            return false;
        });

        std::vector<WorldObject*> rangedTargets;
        for (WorldObject* target : targets)
        {
            if (target->GetDistance(GetCaster()) >= 5.0f)
            {
                rangedTargets.push_back(target);
                break;
            }
        }

        if (!rangedTargets.empty())
        {
            targets.clear();
            targets.push_back(Trinity::Containers::SelectRandomContainerElement(rangedTargets));
        }
    }

    void HandleMissile(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_SOUL_EXPLOSION_TRIGGER_MISSILE, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_friendish_soul_explosion::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_friendish_soul_explosion::HandleMissile, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Return to Kingsmourne - 363022
class spell_remnant_of_a_fallen_king_return_to_kingsmourne_applied : public AuraScript
{
    PrepareAuraScript(spell_remnant_of_a_fallen_king_return_to_kingsmourne_applied);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_RETURN_TO_KINGSMOURNE_VISUALS
        });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_RETURN_TO_KINGSMOURNE_VISUALS);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_remnant_of_a_fallen_king_return_to_kingsmourne_applied::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

// Return to Kingsmourne - 363021
class spell_remnant_of_a_fallen_king_return_to_kingsmourne : public AuraScript
{
    PrepareAuraScript(spell_remnant_of_a_fallen_king_return_to_kingsmourne);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_DOMINATION_GRASP_ROOT_AREATRIGGER,
            SPELL_CANCEL_FORCE_OF_WILL
        });
    }

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        InstanceScript* instance = caster->GetInstanceScript();
        if (!instance)
            return;

        if (Creature* arthas = instance->GetCreature(DATA_REMNANT_OF_A_FALLEN_KING))
            arthas->DespawnOrUnsummon();

        if (Creature* anduin = instance->GetCreature(DATA_ANDUIN_WRYNN))
        {
            anduin->RemoveAurasDueToSpell(SPELL_DOMINATION_GRASP_ROOT_AREATRIGGER);
            instance->DoUpdateWorldState(WORLD_STATE_ANDUIN_INTERMISSION, 0);
            instance->DoUpdateWorldState(WORLD_STATE_ANDUIN_ENCOUNTER_STARTED, 1);
            if (anduin->IsAIEnabled())
            {
                anduin->AI()->DoAction(ACTION_EXIT_INTERMISSION);
                anduin->AI()->DoCastSelf(SPELL_CANCEL_FORCE_OF_WILL);
            }
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_remnant_of_a_fallen_king_return_to_kingsmourne::AfterRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Grim Reflections - 365120
class spell_anduin_wrynn_grim_reflections : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_grim_reflections);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_GRIM_REFLECTIONS_SUMMON
        });
    }

    void HandleCast()
    {
        uint8 reflectionsNumber = GetCaster()->GetMap()->GetDifficultyID() == DIFFICULTY_MYTHIC_RAID ? 4 : 3;
        std::vector<Position> grimReflectionPositions(std::begin(GrimReflectionsSpawnPositions), std::end(GrimReflectionsSpawnPositions));
        Trinity::Containers::RandomResize(grimReflectionPositions, reflectionsNumber);

        for (uint8 i = 0; i < reflectionsNumber; i++)
            GetCaster()->CastSpell(grimReflectionPositions[i], SPELL_GRIM_REFLECTIONS_SUMMON, true);
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_anduin_wrynn_grim_reflections::HandleCast);
    }
};

// Beacon of Hope - 365872
class spell_anduin_wrynn_beacon_of_hope : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_beacon_of_hope);

    void OnCast()
    {
        GetCaster()->SummonCreature(NPC_BEACON_OF_HOPE, BeaconOfHopeSpawnPosition, TEMPSUMMON_MANUAL_DESPAWN);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_anduin_wrynn_beacon_of_hope::OnCast);
    }
};

// Hopelessness - 365958
class spell_anduin_wrynn_hopelessness : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_hopelessness);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_BLASPHEMY_PRE_HIT,
            SPELL_HOPELESSNESS_MISSILE
        });
    }

    void OnPrecast() override
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_BLASPHEMY_PRE_HIT, true);
    }

    void HandleDebuff(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_HOPELESSNESS_MISSILE, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_anduin_wrynn_hopelessness::HandleDebuff, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Hopelessness 365966 - AT 24443
struct at_anduin_wrynn_hopelessness : AreaTriggerAI
{
    at_anduin_wrynn_hopelessness(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger)  { }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer() || unit->HasAura(SPELL_BLASPHEMY_PRE_HIT))
            return;

        if (!unit->IsAlive())
        {
            at->Remove();
            return;
        }

        if (unit->HasAura(at->GetSpellId()))
        {
            unit->CastSpell(unit, SPELL_BLASPHEMY_EXPLODE, true);
            if (Unit* target = at->GetTarget())
                target->CastSpell(target, SPELL_BLASPHEMY_EXPLODE, true);
        }
    }

    void OnUnitExit(Unit* unit) override
    {
        if (!unit->IsAlive() && unit->HasAura(at->GetSpellId()))
            unit->RemoveAurasDueToSpell(at->GetSpellId());
    }
};

// Hopelessness Expire - 365966
class spell_anduin_wrynn_hopelessness_expire : public AuraScript
{
    PrepareAuraScript(spell_anduin_wrynn_hopelessness_expire);

    bool Validate(SpellInfo const* /*spell*/) override
    {
        return ValidateSpellInfo({ SPELL_HOPELESSNESS_EXPLODE });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        switch (GetTargetApplication()->GetRemoveMode())
        {
            case AURA_REMOVE_BY_ENEMY_SPELL:
            case AURA_REMOVE_BY_EXPIRE:
            case AURA_REMOVE_BY_DEATH:
                GetTarget()->CastSpell(GetTarget(), SPELL_HOPELESSNESS_EXPLODE, true);
                break;
            default:
                return;
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_anduin_wrynn_hopelessness_expire::OnRemove, EFFECT_0, SPELL_AURA_AREA_TRIGGER, AURA_EFFECT_HANDLE_REAL);
    }
};

// Empowered Hopebreaker Periodic - 365806
class spell_anduin_wrynn_empowered_hopebreaker_periodic : public SpellScript
{
    PrepareSpellScript(spell_anduin_wrynn_empowered_hopebreaker_periodic);

    bool Validate(SpellInfo const* /*spell*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_HOPEBREAKER_DAMAGE,
            SPELL_HOPEBREAKER_DEBUFF,
        });
    }

    void HandleCast(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_HOPEBREAKER_DAMAGE, true);
        GetCaster()->CastSpell(GetHitUnit(), SPELL_HOPEBREAKER_DEBUFF, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_anduin_wrynn_empowered_hopebreaker_periodic::HandleCast, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Remnant Timer Runic Power - 365291
class spell_remnant_of_a_fallen_king_energize_runic_power : public AuraScript
{
    PrepareAuraScript(spell_remnant_of_a_fallen_king_energize_runic_power);

    static constexpr std::array<int32, 3> RemnantRegenCycle = { 1, 1, 2 };

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* target = GetTarget();
        int32 powerGained = RemnantRegenCycle[_powerRegenCycle++];

        if (_powerRegenCycle > 2)
            _powerRegenCycle = 1;

        target->ModifyPower(target->GetPowerType(), powerGained);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_remnant_of_a_fallen_king_energize_runic_power::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }

private:
    uint8 _powerRegenCycle = 0;
};

// Army of the Dead - 362862
class spell_remnant_of_a_fallen_king_army_of_the_dead : public SpellScript
{
    PrepareSpellScript(spell_remnant_of_a_fallen_king_army_of_the_dead);

    bool Validate(SpellInfo const* /*spell*/) override
    {
        return ValidateSpellInfo(
        {
            SPELL_ECHOES_OF_ANDORHAL_MONSTROUS_SOUL,
        });
    }

    void HandleAfterCast()
    {
        Position summonSoul = GetCaster()->GetNearPosition(frand(30.0f, 50.0f), frand(0.0f, 3.5f));

        if (GetCaster()->GetMap()->GetDifficultyID() != DIFFICULTY_LFR_NEW)
            GetCaster()->CastSpell(summonSoul, SPELL_ECHOES_OF_ANDORHAL_MONSTROUS_SOUL, true);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_remnant_of_a_fallen_king_army_of_the_dead::HandleAfterCast);
    }
};

// Echoes of Andorhal - 362863
class spell_remnant_of_a_fallen_king_echoes_of_andorhal : public SpellScript
{
    PrepareSpellScript(spell_remnant_of_a_fallen_king_echoes_of_andorhal);

    void SetDest(SpellDestination& dest)
    {
        Position const echoesSummon = GetCaster()->GetRandomPoint(DominationGraspCenter, frand(20.5f, 30.0f));
        dest.Relocate(echoesSummon);
    }

    void Register() override
    {
        OnDestinationTargetSelect += SpellDestinationTargetSelectFn(spell_remnant_of_a_fallen_king_echoes_of_andorhal::SetDest, EFFECT_0, TARGET_DEST_DEST_RANDOM);
    }
};

// Remorseless Winter Periodic - 362543
class spell_remnant_of_a_fallen_king_remorseless_winter_periodic : public AuraScript
{
    PrepareAuraScript(spell_remnant_of_a_fallen_king_remorseless_winter_periodic);

    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(GetTarget(), SPELL_REMORSELESS_WINTER_DEBUFF_DAMAGE, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_remnant_of_a_fallen_king_remorseless_winter_periodic::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// Remorseless Winter Damage - 362545
class spell_remnant_of_a_fallen_king_remorseless_winter_damage : public SpellScript
{
    PrepareSpellScript(spell_remnant_of_a_fallen_king_remorseless_winter_damage);

    void HandleDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        Aura* remorselessWinter = GetHitUnit()->GetAura(SPELL_REMORSELESS_WINTER_PERIODIC);
        if (!remorselessWinter)
            return;

        SetHitDamage(int32(GetHitDamage() * (uint8)remorselessWinter->GetStackAmount()));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_remnant_of_a_fallen_king_remorseless_winter_damage::HandleDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

// Soul Reaper - 362771
class spell_remnant_of_a_fallen_king_soul_reaper : public SpellScript
{
    PrepareSpellScript(spell_remnant_of_a_fallen_king_soul_reaper);

    bool Validate(SpellInfo const* /*spell*/) override
    {
        return ValidateSpellInfo
        ({
            SPELL_SOUL_REAPER_PHYSICAL_DAMAGE,
            SPELL_SOUL_REAPER_SHADOWFROST_DAMAGE,
            SPELL_SOUL_REAPER_DEBUFF,
            SPELL_SOUL_REAPER_ATTACK_SPEED,
        });
    }

    void HandleDummyEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* hitUnit = GetHitUnit();
        caster->CastSpell(hitUnit, SPELL_SOUL_REAPER_PHYSICAL_DAMAGE, true);
        caster->CastSpell(hitUnit, SPELL_SOUL_REAPER_SHADOWFROST_DAMAGE, true);
        caster->CastSpell(hitUnit, SPELL_SOUL_REAPER_DEBUFF, true);
        caster->CastSpell(caster, SPELL_SOUL_REAPER_ATTACK_SPEED, true);
        caster->resetAttackTimer(BASE_ATTACK);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_remnant_of_a_fallen_king_soul_reaper::HandleDummyEffect, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// Anduin Wrynn Introduction 996 Custom AT
struct at_anduin_wrynn_pre_introduction : AreaTriggerAI
{
    at_anduin_wrynn_pre_introduction(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger),
        _instance(at->GetInstanceScript()), _firstEntry(false) { }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer() || _firstEntry == true)
            return;

        _firstEntry = true;
        if (Creature* anduin = _instance->GetCreature(DATA_ANDUIN_WRYNN))
        {
            anduin->GetAI()->DoAction(ACTION_START_PRE_INTRODUCTION);
            //anduin->GetAI()->DoAction(ACTION_START_INTRODUCTION);
        }
    }

private:
    InstanceScript* _instance;
    bool _firstEntry;
};

// 17835 - Anduin Introduction
class conversation_anduin_introduction : public ConversationScript
{
public:
    conversation_anduin_introduction() : ConversationScript("conversation_anduin_introduction") { }

    void OnConversationCreate(Conversation* conversation, Unit* creator) override
    {
        if (Creature* uther = creator->GetInstanceScript()->GetCreature(DATA_UTHER_THE_LIGHTBRINGER))
            conversation->AddActor(NPC_UTHER_THE_LIGHTBRINGER, 1, uther->GetGUID());
        if (Creature* sylvanas = creator->GetInstanceScript()->GetCreature(DATA_SYLVANAS_WINDRUNNER))
            conversation->AddActor(NPC_SYLVANAS_WINDRUNNER, 2, sylvanas->GetGUID());
        if (Creature* jaina = creator->GetInstanceScript()->GetCreature(DATA_JAINA_PROUDMOORE))
            conversation->AddActor(NPC_LADY_JAINA_PROUDMOORE, 3, jaina->GetGUID());
    }
};

// 17921 - Arthas to Uther Intermission
class conversation_arthas_uther : public ConversationScript
{
public:
    conversation_arthas_uther() : ConversationScript("conversation_arthas_uther") { }

    void OnConversationCreate(Conversation* conversation, Unit* creator) override
    {
        if (Creature* uther = creator->GetInstanceScript()->GetCreature(DATA_UTHER_THE_LIGHTBRINGER))
            conversation->AddActor(NPC_UTHER_THE_LIGHTBRINGER, 1, uther->GetGUID());
    }
};

// 17923 - Arthas to Sylvanas Intermission
class conversation_arthas_sylvanas : public ConversationScript
{
public:
    conversation_arthas_sylvanas() : ConversationScript("conversation_arthas_sylvanas") { }

    void OnConversationCreate(Conversation* conversation, Unit* creator) override
    {
        if (Creature* sylvanas = creator->GetInstanceScript()->GetCreature(DATA_SYLVANAS_WINDRUNNER))
            conversation->AddActor(NPC_SYLVANAS_WINDRUNNER, 1, sylvanas->GetGUID());
    }
};

// 17836 - Anduin Outroduction
class conversation_anduin_outroduction : public ConversationScript
{
public:
    conversation_anduin_outroduction() : ConversationScript("conversation_anduin_outroduction") { }

    void OnConversationCreate(Conversation* conversation, Unit* creator) override
    {
        if (Creature* jaina = creator->GetInstanceScript()->GetCreature(DATA_JAINA_PROUDMOORE))
            conversation->AddActor(NPC_LADY_JAINA_PROUDMOORE, 1, jaina->GetGUID());
        if (Creature* sylvanas = creator->GetInstanceScript()->GetCreature(DATA_SYLVANAS_WINDRUNNER))
            conversation->AddActor(NPC_SYLVANAS_WINDRUNNER, 2, sylvanas->GetGUID());
        if (Creature* uther = creator->GetInstanceScript()->GetCreature(DATA_UTHER_THE_LIGHTBRINGER))
            conversation->AddActor(NPC_UTHER_THE_LIGHTBRINGER, 3, uther->GetGUID());
        if (Creature* firim = creator->GetInstanceScript()->GetCreature(DATA_FIRIM))
            conversation->AddActor(NPC_FIRIM, 4, firim->GetGUID());
    }
};

// 956 - Anduin End Movie
class movie_anduin_final : public PlayerScript
{
public:
    movie_anduin_final() : PlayerScript("movie_anduin_final") { }

    void MarkPlayerAsSkipped(uint32 playerId)
    {
        _skippedPlayers.insert(playerId);
    }

    void OnMovieComplete(Player* player, uint32 /*movieId*/) override
    {
        InstanceScript* instance = player->GetInstanceScript();
        if (!instance)
            return;

        Creature* anduin = instance->GetCreature(DATA_ANDUIN_WRYNN);
        if (!anduin)
            return;

        MarkPlayerAsSkipped(static_cast<uint32>(player->GetGUID().GetCounter()));
        player->RemoveAurasDueToSpell(SPELL_FINAL_MOVIE);
        Map::PlayerList const& players = player->GetMap()->GetPlayers();

        // Outroduciton must start once every player skips or completes the movie
        if (_skippedPlayers.size() == players.getSize())
            anduin->GetAI()->DoAction(ACTION_START_OUTRODUCTION);
    }

private:
    std::unordered_set<uint32> _skippedPlayers;
};

void AddSC_boss_anduin_wrynn()
{
    RegisterSepulcherOfTheFirstOnesCreatureAI(boss_anduin_wrynn);
    RegisterSepulcherOfTheFirstOnesCreatureAI(boss_remnant_of_a_fallen_king);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_sylvanas);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_jaina);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_uther);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_befouled_barrier);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_empty_vessel);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_lost_soul);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_anduin_soul);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_anduin_despair);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_anduin_doubt);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_anduin_hope);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_monstrous_soul);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_fiendish_soul);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_grim_reflection);
    RegisterSepulcherOfTheFirstOnesCreatureAI(npc_anduin_wrynn_beacon_of_hope);

    RegisterAreaTriggerAI(at_anduin_wrynn_pre_introduction);
    RegisterSpellScript(spell_anduin_wrynn_pre_introduction);
    RegisterSpellScript(spell_anduin_wrynn_progression_aura);
    RegisterSpellScript(spell_anduin_wrynn_energize_willpower_lfr);
    RegisterSpellScript(spell_anduin_wrynn_dark_zeal);
    RegisterSpellScript(spell_anduin_wrynn_hopebreaker);
    RegisterSpellScript(spell_anduin_wrynn_hopebreaker_periodic);
    RegisterSpellScript(spell_anduin_wrynn_hopebreaker_damage);
    RegisterAreaTriggerAI(at_anduin_wrynn_befouled_barrier);
    RegisterSpellScript(spell_anduin_wrynn_befouled_barrier_absorb);
    RegisterSpellScript(spell_anduin_wrynn_befouled_barrier_expire);
    RegisterAreaTriggerAI(at_anduin_wrynn_blasphemy);
    RegisterSpellScript(spell_anduin_wrynn_blasphemy);
    RegisterSpellScript(spell_anduin_wrynn_hopelessness_overconfidence);
    RegisterAreaTriggerAI(at_anduin_wrynn_wicked_star);
    RegisterAreaTriggerAI(at_anduin_wrynn_empowered_wicked_star);
    RegisterSpellAndAuraScriptPair(spell_anduin_wrynn_wicked_star_selector, spell_anduin_wrynn_wicked_star_selector_AuraScript);
    RegisterSpellAndAuraScriptPairWithArgs(spell_anduin_wrynn_wicked_star_selector, spell_anduin_wrynn_empowered_wicked_star_selector_AuraScript, "spell_anduin_wrynn_empowered_wicked_star_selector");
    RegisterSpellScript(spell_anduin_wrynn_kingsmourne_hungers);
    RegisterSpellScript(spell_anduin_soul_lost_soul);
    RegisterSpellScript(spell_anduin_wrynn_lost_soul);
    RegisterSpellScript(spell_anduin_wrynn_lost_soul_mirror_image);
    RegisterSpellScript(spell_anduin_wrynn_severed_soul);
    RegisterSpellScript(spell_anduin_wrynn_soul_despawn);
    RegisterSpellScript(spell_anduin_rain_of_despair_player_selector);
    RegisterSpellScript(spell_anduin_wrynn_force_of_will);
    RegisterAreaTriggerAI(at_anduin_wrynn_march_of_the_damned);
    RegisterSpellScript(spell_anduin_wrynn_march_of_the_damned);
    RegisterSpellScript(spell_friendish_soul_explosion);
    RegisterSpellScript(spell_anduin_wrynn_grim_reflections);
    RegisterAreaTriggerAI(at_anduin_wrynn_beacon_of_hope);
    RegisterSpellScript(spell_anduin_wrynn_beacon_of_hope);
    RegisterSpellScript(spell_anduin_wrynn_empowered_hopebreaker_periodic);
    RegisterAreaTriggerAI(at_anduin_wrynn_hopelessness);
    RegisterSpellScript(spell_anduin_wrynn_hopelessness);
    RegisterSpellScript(spell_anduin_wrynn_hopelessness_expire);
    RegisterSpellScript(spell_anduin_wrynn_fragment_of_hope);

    RegisterSpellScript(spell_remnant_of_a_fallen_king_spawn);
    RegisterSpellScript(spell_remnant_of_a_fallen_king_energize_runic_power);
    RegisterSpellScript(spell_remnant_of_a_fallen_king_army_of_the_dead);
    RegisterSpellScript(spell_remnant_of_a_fallen_king_echoes_of_andorhal);
    RegisterSpellScript(spell_remnant_of_a_fallen_king_soul_reaper);
    RegisterSpellScript(spell_remnant_of_a_fallen_king_remorseless_winter_periodic);
    RegisterSpellScript(spell_remnant_of_a_fallen_king_remorseless_winter_damage);
    RegisterSpellScript(spell_remnant_of_a_fallen_king_return_to_kingsmourne);
    RegisterSpellScript(spell_remnant_of_a_fallen_king_return_to_kingsmourne_applied);

    new conversation_anduin_introduction();
    new conversation_arthas_uther();
    new conversation_arthas_sylvanas();
    new conversation_anduin_outroduction();
    new movie_anduin_final();
}
