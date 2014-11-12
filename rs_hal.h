/*
 * Copyright (C) 2011-2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RS_HAL_H
#define RS_HAL_H

#include <rsInternalDefines.h>

struct ANativeWindow;

namespace android {
namespace renderscript {

class Context;
class ObjectBase;
class Element;
class Type;
class Allocation;
class Script;
class ScriptKernelID;
class ScriptFieldID;
class ScriptMethodID;
class ScriptC;
class ScriptGroup;
class Path;
class Program;
class ProgramStore;
class ProgramRaster;
class ProgramVertex;
class ProgramFragment;
class Mesh;
class Sampler;
class FBOCache;

/**
 * Define the internal object types.  This ia a mirror of the
 * definition in rs_types.rsh except with the p value typed
 * correctly.
 *
 * p = pointer to internal object implementation
 * r = reserved by libRS runtime
 * v1 = Mirror of p->mHal.drv
 * v2 = reserved for use by vendor drivers
 */

#ifndef __LP64__
#define RS_BASE_OBJ(_t_) typedef struct { const _t_* p; } __attribute__((packed, aligned(4)))
#else
#define RS_BASE_OBJ(_t_) typedef struct { const _t_* p; const void* r; const void* v1; const void* v2; }
#endif

RS_BASE_OBJ(ObjectBase) rs_object_base;
RS_BASE_OBJ(Element) rs_element;
RS_BASE_OBJ(Type) rs_type;
RS_BASE_OBJ(Allocation) rs_allocation;
RS_BASE_OBJ(Sampler) rs_sampler;
RS_BASE_OBJ(Script) rs_script;
RS_BASE_OBJ(ScriptGroup) rs_script_group;

#ifndef __LP64__
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_mesh;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_path;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_program_fragment;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_program_vertex;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_program_raster;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_program_store;
typedef struct { const int* p; } __attribute__((packed, aligned(4))) rs_font;
#endif // __LP64__


typedef void *(*RsHalSymbolLookupFunc)(void *usrptr, char const *symbolName);

/**
 * Script management functions
 */
typedef struct {
    bool (*initGraphics)(const Context *);
    void (*shutdownGraphics)(const Context *);
    bool (*setSurface)(const Context *, uint32_t w, uint32_t h, RsNativeWindow);
    void (*swap)(const Context *);

    void (*shutdownDriver)(Context *);
    void (*getVersion)(unsigned int *major, unsigned int *minor);
    void (*setPriority)(const Context *, int32_t priority);

    void* (*allocRuntimeMem)(size_t size, uint32_t flags);
    void (*freeRuntimeMem)(void* ptr);

    struct {
        bool (*init)(const Context *rsc, ScriptC *s,
                     char const *resName,
                     char const *cacheDir,
                     uint8_t const *bitcode,
                     size_t bitcodeSize,
                     uint32_t flags);
        bool (*initIntrinsic)(const Context *rsc, Script *s,
                              RsScriptIntrinsicID iid,
                              Element *e);

        void (*invokeFunction)(const Context *rsc, Script *s,
                               uint32_t slot,
                               const void *params,
                               size_t paramLength);
        int (*invokeRoot)(const Context *rsc, Script *s);
        void (*invokeForEach)(const Context *rsc,
                              Script *s,
                              uint32_t slot,
                              const Allocation * ain,
                              Allocation * aout,
                              const void * usr,
                              size_t usrLen,
                              const RsScriptCall *sc);
        void (*invokeInit)(const Context *rsc, Script *s);
        void (*invokeFreeChildren)(const Context *rsc, Script *s);

        void (*setGlobalVar)(const Context *rsc, const Script *s,
                             uint32_t slot,
                             void *data,
                             size_t dataLength);
        void (*getGlobalVar)(const Context *rsc, const Script *s,
                             uint32_t slot,
                             void *data,
                             size_t dataLength);
        void (*setGlobalVarWithElemDims)(const Context *rsc, const Script *s,
                                         uint32_t slot,
                                         void *data,
                                         size_t dataLength,
                                         const Element *e,
                                         const uint32_t *dims,
                                         size_t dimLength);
        void (*setGlobalBind)(const Context *rsc, const Script *s,
                              uint32_t slot,
                              Allocation *data);
        void (*setGlobalObj)(const Context *rsc, const Script *s,
                             uint32_t slot,
                             ObjectBase *data);

        void (*destroy)(const Context *rsc, Script *s);
        void (*invokeForEachMulti)(const Context *rsc,
                                   Script *s,
                                   uint32_t slot,
                                   const Allocation ** ains,
                                   size_t inLen,
                                   Allocation * aout,
                                   const void * usr,
                                   size_t usrLen,
                                   const RsScriptCall *sc);
        void (*updateCachedObject)(const Context *rsc, const Script *, rs_script *obj);
    } script;

    struct {
        bool (*init)(const Context *rsc, Allocation *alloc, bool forceZero);
        void (*destroy)(const Context *rsc, Allocation *alloc);
        uint32_t (*grallocBits)(const Context *rsc, Allocation *alloc);

        void (*resize)(const Context *rsc, const Allocation *alloc, const Type *newType,
                       bool zeroNew);
        void (*syncAll)(const Context *rsc, const Allocation *alloc, RsAllocationUsageType src);
        void (*markDirty)(const Context *rsc, const Allocation *alloc);

        void (*setSurface)(const Context *rsc, Allocation *alloc, ANativeWindow *sur);
        void (*ioSend)(const Context *rsc, Allocation *alloc);

        /**
         * A new gralloc buffer is in use. The pointers and strides in
         * mHal.drvState.lod[0-2] will be updated with the new values.
         *
         * The new gralloc handle is provided in mHal.state.nativeBuffer
         *
         */
        void (*ioReceive)(const Context *rsc, Allocation *alloc);

        void (*data1D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t lod, size_t count,
                       const void *data, size_t sizeBytes);
        void (*data2D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t yoff, uint32_t lod,
                       RsAllocationCubemapFace face, uint32_t w, uint32_t h,
                       const void *data, size_t sizeBytes, size_t stride);
        void (*data3D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
                       uint32_t w, uint32_t h, uint32_t d, const void *data, size_t sizeBytes,
                       size_t stride);

        void (*read1D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t lod, size_t count,
                       void *data, size_t sizeBytes);
        void (*read2D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t yoff, uint32_t lod,
                       RsAllocationCubemapFace face, uint32_t w, uint32_t h,
                       void *data, size_t sizeBytes, size_t stride);
        void (*read3D)(const Context *rsc, const Allocation *alloc,
                       uint32_t xoff, uint32_t yoff, uint32_t zoff, uint32_t lod,
                       uint32_t w, uint32_t h, uint32_t d, void *data, size_t sizeBytes,
                       size_t stride);

        // Lock and unlock make a 1D region of memory available to the CPU
        // for direct access by pointer.  Once unlock is called control is
        // returned to the SOC driver.
        void * (*lock1D)(const Context *rsc, const Allocation *alloc);
        void (*unlock1D)(const Context *rsc, const Allocation *alloc);

        // Allocation to allocation copies
        void (*allocData1D)(const Context *rsc,
                            const Allocation *dstAlloc,
                            uint32_t dstXoff, uint32_t dstLod, size_t count,
                            const Allocation *srcAlloc, uint32_t srcXoff, uint32_t srcLod);
        void (*allocData2D)(const Context *rsc,
                            const Allocation *dstAlloc,
                            uint32_t dstXoff, uint32_t dstYoff, uint32_t dstLod,
                            RsAllocationCubemapFace dstFace, uint32_t w, uint32_t h,
                            const Allocation *srcAlloc,
                            uint32_t srcXoff, uint32_t srcYoff, uint32_t srcLod,
                            RsAllocationCubemapFace srcFace);
        void (*allocData3D)(const Context *rsc,
                            const Allocation *dstAlloc,
                            uint32_t dstXoff, uint32_t dstYoff, uint32_t dstZoff,
                            uint32_t dstLod,
                            uint32_t w, uint32_t h, uint32_t d,
                            const Allocation *srcAlloc,
                            uint32_t srcXoff, uint32_t srcYoff, uint32_t srcZoff,
                            uint32_t srcLod);

        void (*elementData1D)(const Context *rsc, const Allocation *alloc, uint32_t x,
                              const void *data, uint32_t elementOff, size_t sizeBytes);
        void (*elementData2D)(const Context *rsc, const Allocation *alloc, uint32_t x, uint32_t y,
                              const void *data, uint32_t elementOff, size_t sizeBytes);

        void (*generateMipmaps)(const Context *rsc, const Allocation *alloc);

        void (*updateCachedObject)(const Context *rsc, const Allocation *alloc, rs_allocation *obj);
    } allocation;

    struct {
        bool (*init)(const Context *rsc, const ProgramStore *ps);
        void (*setActive)(const Context *rsc, const ProgramStore *ps);
        void (*destroy)(const Context *rsc, const ProgramStore *ps);
    } store;

    struct {
        bool (*init)(const Context *rsc, const ProgramRaster *ps);
        void (*setActive)(const Context *rsc, const ProgramRaster *ps);
        void (*destroy)(const Context *rsc, const ProgramRaster *ps);
    } raster;

    struct {
        bool (*init)(const Context *rsc, const ProgramVertex *pv,
                     const char* shader, size_t shaderLen,
                     const char** textureNames, size_t textureNamesCount,
                     const size_t *textureNamesLength);
        void (*setActive)(const Context *rsc, const ProgramVertex *pv);
        void (*destroy)(const Context *rsc, const ProgramVertex *pv);
    } vertex;

    struct {
        bool (*init)(const Context *rsc, const ProgramFragment *pf,
                     const char* shader, size_t shaderLen,
                     const char** textureNames, size_t textureNamesCount,
                     const size_t *textureNamesLength);
        void (*setActive)(const Context *rsc, const ProgramFragment *pf);
        void (*destroy)(const Context *rsc, const ProgramFragment *pf);
    } fragment;

    struct {
        bool (*init)(const Context *rsc, const Mesh *m);
        void (*draw)(const Context *rsc, const Mesh *m, uint32_t primIndex, uint32_t start, uint32_t len);
        void (*destroy)(const Context *rsc, const Mesh *m);
    } mesh;

    struct {
        bool (*initStatic)(const Context *rsc, const Path *m, const Allocation *vtx, const Allocation *loops);
        bool (*initDynamic)(const Context *rsc, const Path *m);
        void (*draw)(const Context *rsc, const Path *m);
        void (*destroy)(const Context *rsc, const Path *m);
    } path;

    struct {
        bool (*init)(const Context *rsc, const Sampler *m);
        void (*destroy)(const Context *rsc, const Sampler *m);
        void (*updateCachedObject)(const Context *rsc, const Sampler *s, rs_sampler *obj);
    } sampler;

    struct {
        bool (*init)(const Context *rsc, const FBOCache *fb);
        void (*setActive)(const Context *rsc, const FBOCache *fb);
        void (*destroy)(const Context *rsc, const FBOCache *fb);
    } framebuffer;

    struct {
        bool (*init)(const Context *rsc, ScriptGroup *sg);
        void (*setInput)(const Context *rsc, const ScriptGroup *sg,
                         const ScriptKernelID *kid, Allocation *);
        void (*setOutput)(const Context *rsc, const ScriptGroup *sg,
                          const ScriptKernelID *kid, Allocation *);
        void (*execute)(const Context *rsc, const ScriptGroup *sg);
        void (*destroy)(const Context *rsc, const ScriptGroup *sg);
        void (*updateCachedObject)(const Context *rsc, const ScriptGroup *sg, rs_script_group *obj);
    } scriptgroup;

    struct {
        bool (*init)(const Context *rsc, const Type *m);
        void (*destroy)(const Context *rsc, const Type *m);
        void (*updateCachedObject)(const Context *rsc, const Type *s, rs_type *obj);
    } type;

    struct {
        bool (*init)(const Context *rsc, const Element *m);
        void (*destroy)(const Context *rsc, const Element *m);
        void (*updateCachedObject)(const Context *rsc, const Element *s, rs_element *obj);
    } element;

    void (*finish)(const Context *rsc);
} RsdHalFunctions;


}
}

#ifdef __cplusplus
extern "C" {
#endif

bool rsdHalInit(RsContext, uint32_t version_major, uint32_t version_minor);

#ifdef __cplusplus
}
#endif

#endif
