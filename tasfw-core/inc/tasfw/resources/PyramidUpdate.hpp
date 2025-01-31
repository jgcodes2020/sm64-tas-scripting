#pragma once
#include <vector>
#include "tasfw/Resource.hpp"
#include <sm64/Types.hpp>
#include "tasfw/resources/LibSm64.hpp"

#ifndef PYRAMIDUPDATE_H
#define PYRAMIDUPDATE_H

class PyramidUpdateConfig
{
public:
	bool EnableMarioMovement = false;

	PyramidUpdateConfig() = default;
};

class PyramidUpdateMem
{
public:
	class Sm64Object;

	class Sm64Surface
	{
	public:
		s16 type;
		s16 force;
		s8 flags;
		s8 room;
		s16 lowerY;
		s16 upperY;
		Vec3s vertex1;
		Vec3s vertex2;
		Vec3s vertex3;
		struct
		{
			f32 x;
			f32 y;
			f32 z;
		} normal;
		f32 originOffset;
		bool objectIsPyramid = false;

		Sm64Object* object(PyramidUpdateMem& state);
	};

	class Sm64Object
	{
	public:
		float posX = 0;
		float posY = 0;
		float posZ = 0;
		float tiltingPyramidNormalX = 0;
		float tiltingPyramidNormalY = 0;
		float tiltingPyramidNormalZ = 0;
		s32 tiltingPyramidMarioOnPlatform = false;
		bool platformIsPyramid = false;
		Mat4 transform;
		std::vector<Sm64Surface> surfaces[3];

		Sm64Object* platform(PyramidUpdateMem& state);
	};

	class Sm64MarioState
	{
	public:
		float posX = 0;
		float posY = 0;
		float posZ = 0;
		float velX = 0;
		float velY = 0;
		float velZ = 0;
		Vec3s angle;
		Vec3s angleVel;
		int64_t floorId = -1;
		bool isFloorStatic = false;
		u32 action = 0;

		Sm64Surface* floor(PyramidUpdateMem& state);
	};

	class Sm64Camera
	{
	public:
		s16 yaw = 0;
	};

	Sm64Object marioObj;
	Sm64Object pyramid;
	std::vector<Sm64Surface> staticFloors;
	Sm64MarioState marioState;
	Sm64Camera camera;
	int64_t frame = 0;
	uint32_t inputs = 0;

	PyramidUpdateMem() = default;
	PyramidUpdateMem(const LibSm64& resource, Object* pyramidLibSm64);

	static bool FloorIsSlope(Sm64Surface* floor, u32 action);
	static short GetFloorClass(Sm64Surface* floor, u32 action);

private:
	//TODO: All of this needs to be rewritten when memory access is standardized
	void LoadSurfaces(Object* pyramidLibSm64, Sm64Object& pyramid);
	void GetVertices(short** data, short* vertexData);
	int CountSurfaces(short* data);
	short SurfaceHasForce(short surfaceType);
	void LoadObjectSurfaces(short** data, short* vertexData, Sm64Surface** surfaceArrays);
	void ReadSurfaceData(short* vertexData, short** vertexIndices, Sm64Surface** surfaceArrays, int surfaceType);
	void AddStaticGeometry();
};

class PyramidUpdate : public Resource<PyramidUpdateMem>
{
public:
	PyramidUpdate();
	PyramidUpdate(PyramidUpdateConfig config) : _enableMarioMovement(config.EnableMarioMovement) { }
	void save(PyramidUpdateMem& state) const;
	void load(const PyramidUpdateMem& state);
	void advance();
	void* addr(const char* symbol) const;
	std::size_t getStateSize(const PyramidUpdateMem& state) const;
	uint32_t getCurrentFrame() const;

private:
	PyramidUpdateMem _state;
	void UpdatePyramid();
	void TransformSurfaces(int surfaceIndex);
	float ApproachByIncrement(float goal, float src, float inc);
	void CreateTransformFromNormals(Mat4& transform, float xNorm, float yNorm, float zNorm);
	float FindFloor(Vec3f* marioPos, PyramidUpdateMem::Sm64Surface* surfaces, int surfaceCount, int64_t* floorId);

	//Mario update methods
	void UpdateMario();
	void ExecuteMarioAction();
	void CopyMarioStateToObject();

	bool _enableMarioMovement = false;
};

#endif