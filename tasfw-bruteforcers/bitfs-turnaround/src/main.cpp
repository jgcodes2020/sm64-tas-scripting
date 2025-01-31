
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>

#include <tasfw/Inputs.hpp>
#include <tasfw/Script.hpp>
#include <sm64/Camera.hpp>
#include <sm64/Trig.hpp>
#include <sm64/Types.hpp>
#include <sm64/ObjectFields.hpp>

#include <tasfw/scripts/General.hpp>
#include <tasfw/scripts/BitFSPyramidOscillation.hpp>

#include <omp.h>
#include "BitFsConfig.hpp"
#include <tasfw/scripts/BitFsScApproach.hpp>
#include <sm64/Sm64.hpp>
#include "Scattershot_BitfsDr.hpp"
#include "Scattershot.hpp"

#define SOURCE_DIR "${CMAKE_SOURCE_DIR}"

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

class MainScript : public TopLevelScript<LibSm64>
{
public:
	bool validation() { return true; }

	bool execution()
	{
		LongLoad(3300); //3277, 3536

		//const BehaviorScript* trackPlatformBehavior = (const BehaviorScript*)(resource->addr("bhvPlatformOnTrack"));
		//Object* objectPool = (Object*)(resource->addr("gObjectPool"));
		//Object* trackPlatform = &objectPool[85];
		//if (trackPlatform->behavior != trackPlatformBehavior)
		//	return false;

		//! UNSAFE
		//trackPlatform->oPosX = -1945.0f;
		//AdvanceFrameRead();
		//Save();
		Camera* camera = *(Camera**)(resource->addr("gCamera"));
		MarioState* marioState = *(MarioState**)(resource->addr("gMarioState"));
		auto stick = Inputs::GetClosestInputByYawExact(-16384, 32, camera->yaw);
		AdvanceFrameWrite(Inputs(0, stick.first, stick.second));

		while (marioState->action != ACT_IDLE)
			AdvanceFrameWrite(Inputs(0, 0, 0));

		auto status = Modify<BitFsPyramidOscillation>(0.69f, 3, false);
		auto status2 = Modify<BitFsScApproach>(0, 3, 0.69f, status);
		return true;
	}

	bool assertion()
	{
		// Save m64Diff to M64
		M64Diff diff = GetBaseDiff();
		for (auto& [frame, inputs]: diff.frames)
		{
			_m64->frames[frame] = inputs;
		}

		return true;
	}
};

void InitConfiguration(Configuration& configuration)
{
	configuration.StartFrame = 3515;
	configuration.SegmentLength = 10;
	configuration.MaxSegments = 1024;
	configuration.MaxBlocks = 500000;
	configuration.MaxHashes = 10 * configuration.MaxBlocks;
	configuration.MaxSharedBlocks = 20000000;
	configuration.MaxSharedHashes = 10 * configuration.MaxSharedBlocks;
	configuration.TotalThreads = 3;
	configuration.MaxSharedSegments = 25000000;
	configuration.MaxLocalSegments = 2000000;
	configuration.MaxLightningLength = 10000;
	configuration.MaxShots = 1000000000;
	configuration.SegmentsPerShot = 200;
	configuration.ShotsPerMerge = 300;
	configuration.MergesPerSegmentGC = 10;
	configuration.StartFromRootEveryNShots = 5;
	configuration.M64Path = std::filesystem::path("C:\\Users\\Tyler\\Documents\\repos\\sm64_tas_scripting\\res\\4_units_from_edge.m64");

	configuration.SetResourcePaths(std::vector<std::string>
		{
			"C:\\Users\\Tyler\\Documents\\repos\\sm64_tas_scripting\\res\\sm64_jp_0.dll",
			"C:\\Users\\Tyler\\Documents\\repos\\sm64_tas_scripting\\res\\sm64_jp_1.dll",
			"C:\\Users\\Tyler\\Documents\\repos\\sm64_tas_scripting\\res\\sm64_jp_2.dll",
			"C:\\Users\\Tyler\\Documents\\repos\\sm64_tas_scripting\\res\\sm64_jp_3.dll"
		});
}

int main(int argc, const char* argv[])
{
	namespace fs = std::filesystem;
	fs::path cfgPath =
		((argc >= 2) ? fs::path(argv[1]) : getPathToSelf().parent_path() / "config.json");

	BitFs_ConfigData cfg = BitFs_ConfigData::load(cfgPath);

	auto lib_path = cfg.libSM64;
	auto m64_path = cfg.m64File;

	Configuration config;
	InitConfiguration(config);

	//M64 m64 = M64(config.M64Path);
	//m64.load();

	Scattershot<SShotState_BitfsDr, LibSm64>::Run<Scattershot_BitfsDr, LibSm64Config>(config, [&](auto path)
		{
			LibSm64Config resourceConfig;
			resourceConfig.dllPath = path;
			resourceConfig.lightweight = true;
			resourceConfig.countryCode = CountryCode::SUPER_MARIO_64_J;

			return resourceConfig;
		});
	//auto status = MainScript::MainConfig<MainScript>(m64, lib_path);

	//m64.save();

	return 0;
}
