#include "Patches/RestoreLevelSelection.hpp"
#include "Patches/RestoreTableScroll.hpp"
#include "Patches/HookLevelCollectionTableSet.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Sprite.hpp"
#include "GlobalNamespace/IBeatmapLevelPackCollection.hpp"
#include "GlobalNamespace/IBeatmapLevelPack.hpp"
#include "GlobalNamespace/BeatmapLevelPack.hpp"
#include "GlobalNamespace/IBeatmapLevelCollection.hpp"
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/PreviewDifficultyBeatmapSet.hpp"
#include "Utils/PlaylistUtils.hpp"

#include "logging.hpp"
#include "config.hpp"

#include "songloader/include/Utils/FindComponentsUtils.hpp"

namespace BetterSongList::Hooks {
    std::string RestoreLevelSelection::restoredPackId;
    SafePtr<GlobalNamespace::IBeatmapLevelPack> RestoreLevelSelection::restoredPack;

    void RestoreLevelSelection::LevelSelectionFlowCoordinator_DidActivate_Prefix(GlobalNamespace::LevelSelectionFlowCoordinator::State*& startState, bool firstActivation) {
        auto startPack = startState ? startState->beatmapLevelPack : nullptr;
        auto startPackId = startPack ? startPack->get_packID() : nullptr;
        restoredPackId = startPackId ? static_cast<std::string>(startPackId) : "";
        if (startState || !firstActivation) {
            return;
        }

        INFO("restored Pack Id: {}", restoredPackId);
        auto restoreCategory = config.get_lastCategory();
        auto& restoreLevel = config.get_lastSong();
        GlobalNamespace::IPreviewBeatmapLevel* m = nullptr;

        if (!restoreLevel.empty()) {
            auto bm = RuntimeSongLoader::FindComponentsUtils::GetBeatmapLevelsModel();
            auto levels = bm->loadedPreviewBeatmapLevels;

            levels->TryGetValue(restoreLevel, byref(m));
        }
        
        LoadPackFromCollectionName();

        startState = GlobalNamespace::LevelSelectionFlowCoordinator::State::New_ctor(
            System::Nullable_1<LevelCategory>(restoreCategory, true),
            restoredPack.ptr(),
            m,
            nullptr
        );
    }

    void RestoreLevelSelection::LoadPackFromCollectionName() {
        INFO("Loading pack from name");
        if (restoredPack) {
            auto shortPackName = restoredPack->get_shortPackName();
            if (shortPackName && shortPackName == config.get_lastPack()) {
                return;
            }
        }

        if (config.get_lastPack().empty()) {
            restoredPack.emplace(nullptr);
            INFO("Config lastpack was empty");
            return;
        }

        restoredPack.emplace(PlaylistUtils::GetPack(config.get_lastPack()));
    }

    void RestoreLevelSelection::LevelFilteringNavigationController_ShowPacksInSecondChildController_Prefix(StringW& levelPackIdToBeSelectedAfterPresent) {
        if (levelPackIdToBeSelectedAfterPresent) return;
        LoadPackFromCollectionName();

        levelPackIdToBeSelectedAfterPresent = restoredPack ? restoredPack->get_packID() : nullptr;
    }
}