/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _OPENCL_WRAPPER_H_
#define _OPENCL_WRAPPER_H_

#include <config_features.h>
#include <formula/opcode.hxx>
#include <sal/detail/log.h>
#include <cassert>
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

// CL_MAP_WRITE_INVALIDATE_REGION is new in OpenCL 1.2.

// When compiling against an older OpenCL, use CL_MAP_WRITE.

// FIXME: But what if this code has been compiled against OpenCL 1.2
// headers but then runs against an OpenCL 1.1 implementation?
// Probably the code should check at run-time the version of the
// OpenCL implementation and choose which flag to use based on that.
#ifdef CL_MAP_WRITE_INVALIDATE_REGION
#define OPENCLWRAPPER_CL_MAP_WRITE_FLAG CL_MAP_WRITE_INVALIDATE_REGION
#else
#define OPENCLWRAPPER_CL_MAP_WRITE_FLAG CL_MAP_WRITE
#endif

#if defined(_MSC_VER)
#ifndef strcasecmp
#define strcasecmp strcmp
#endif
#endif

typedef unsigned int uint;

typedef struct _KernelEnv {
    cl_context mpkContext;
    cl_command_queue mpkCmdQueue;
    cl_program mpkProgram;
    cl_kernel mpkKernel;
    char mckKernelName[150];
} KernelEnv;

typedef struct _OpenCLEnv {
    cl_platform_id mpOclPlatformID;
    cl_context mpOclContext;
    cl_device_id mpOclDevsID;
    cl_command_queue mpOclCmdQueue;
} OpenCLEnv;

#if defined __cplusplus
extern "C" {
#endif

//user defined, this is function wrapper which is used to set the input parameters ,
//luanch kernel and copy data from GPU to CPU or CPU to GPU.
typedef int (*cl_kernel_function)(void **userdata, KernelEnv *kenv);

#if defined __cplusplus

}
#endif

#define CHECK_OPENCL(status)              \
if(status != CL_SUCCESS)                  \
{                                          \
    printf ("OpenCL error code is %d at " SAL_DETAIL_WHERE "\n", status);    \
    return 0;                            \
}


#define MAX_KERNEL_STRING_LEN    64
#define MAX_CLFILE_NUM 50
#define MAX_CLKERNEL_NUM 200
#define MAX_KERNEL_NAME_LEN 64

typedef struct _GPUEnv {
    //share vb in all modules in hb library
    cl_platform_id mpPlatformID;
    cl_device_type mDevType;
    cl_context mpContext;
    cl_device_id *mpArryDevsID;
    cl_device_id mpDevID;
    cl_command_queue mpCmdQueue;
    cl_kernel mpArryKernels[MAX_CLFILE_NUM];
    cl_program mpArryPrograms[MAX_CLFILE_NUM]; //one program object maps one kernel source file
    char mArryKnelSrcFile[MAX_CLFILE_NUM][256], //the max len of kernel file name is 256
         mArrykernelNames[MAX_CLKERNEL_NUM][MAX_KERNEL_STRING_LEN + 1];
         cl_kernel_function mpArryKnelFuncs[MAX_CLKERNEL_NUM];
    int mnKernelCount, mnFileCount, // only one kernel file
        mnIsUserCreated; // 1: created , 0:no create and needed to create by opencl wrapper

} GPUEnv;

typedef struct {
    char kernelName[MAX_KERNEL_NAME_LEN + 1];
    char *kernelStr;
} kernel_node;

class OpenclCalcBase{
public:
    OpenclCalcBase(){};
    virtual ~OpenclCalcBase(){};
#ifdef GPU_64BITS
    virtual int OclHostSignedAdd(double *lData,double *rData,double *rResult,int rowSize)=0;
    virtual int OclHostSignedSub(double *lData,double *rData,double *rResult,int rowSize)=0;
    virtual int OclHostSignedMul(double *lData,double *rData,double *rResult,int rowSize)=0;
    virtual int OclHostSignedDiv(double *lData,double *rData,double *rResult,int rowSize)=0;
    virtual int OclHostFormulaMax(double *srcData,int *startPos,int *endPos,double *output,int outputSize)=0;
    virtual int OclHostFormulaMin(double *srcData,int *startPos,int *endPos,double *output,int outputSize)=0;
    virtual int OclHostFormulaAverage(double *srcData,int *startPos,int *endPos,double *output,int outputSize)=0;
#endif
    virtual int OclHostSignedAdd32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize)=0;
    virtual int OclHostSignedSub32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize)=0;
    virtual int OclHostSignedMul32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize)=0;
    virtual int OclHostSignedDiv32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize)=0;
    virtual int OclHostFormulaMax32Bits(float *fpSrcData,uint *npStartPos,uint *npEndPos,double *output,int outputSize)=0;
    virtual int OclHostFormulaMin32Bits(float *fpSrcData,uint *npStartPos,uint *npEndPos,double *output,int outputSize)=0;
    virtual int OclHostFormulaAverage32Bits(float *fpSrcData,uint *npStartPos,uint *npEndPos,double *output,int outputSize)=0;


    //virtual int OclHostFormulaCount(int *start,int *end,float *output,int size)=0;
    //virtual int OclHostFormulaSum(float *srcData,int *startPos,int *endPos,float *output,int outputSize)=0;
    //virtual int OclHostFormulaSumProduct(float *pdSrcData,int *pnStart,int *pnEnd,float *pdOutput,int nSize)=0;

};


class OpenclDevice {

public:
    static GPUEnv gpuEnv;
    static int isInited;
    OpenclDevice();
    ~OpenclDevice();
    static int InitEnv();
    static int RegistOpenclKernel();
    static int ReleaseOpenclRunEnv();
    static int InitOpenclRunEnv(GPUEnv *gpu);
    static int ReleaseOpenclEnv(GPUEnv *gpuInfo);
    static int CompileKernelFile(GPUEnv *gpuInfo, const char *buildOption);
    static int InitOpenclRunEnv(int argc, const char *buildOptionKernelfiles);
    static int CachedOfKernerPrg(const GPUEnv *gpuEnvCached, const char * clFileName);
    static int GeneratBinFromKernelSource(cl_program program, const char * clFileName);
    static int WriteBinaryToFile(const char* fileName, const char* birary, size_t numBytes);
    static int BinaryGenerated(const char * clFileName, FILE ** fhandle);
    static int CompileKernelFile(const char *filename, GPUEnv *gpuInfo, const char *buildOption);

    int InitOpenclAttr(OpenCLEnv * env);
    int ReleaseKernel(KernelEnv * env);
    int SetKernelEnv(KernelEnv *envInfo);
    int CreateKernel(char * kernelname, KernelEnv * env);
    int RunKernel(const char *kernelName, void **userdata);
    int ConvertToString(const char *filename, char **source);
    int CheckKernelName(KernelEnv *envInfo,const char *kernelName);
    int RegisterKernelWrapper(const char *kernelName,cl_kernel_function function);
    int RunKernelWrapper(cl_kernel_function function, const char * kernelName, void **usrdata);
    int GetKernelEnvAndFunc(const char *kernelName, KernelEnv *env,cl_kernel_function *function);


#ifdef WIN32
    static int LoadOpencl();
    static int OpenclInite();
    static void FreeOpenclDll();
#endif

    int GetOpenclState();
    void SetOpenclState(int state);
    inline static int AddKernelConfig(int kCount, const char *kName);

};

#define NUM 4//(16*16*16)
typedef enum _formulax_ {
    MIN,
    MAX,
    SUM,
    AVG,
    COUNT,
    SUMPRODUCT,
    MINVERSE,
    SIGNEDADD,
    SIGNEDNUL,
    SIGNEDDIV,
    SIGNEDSUB
} formulax;

class OclCalc: public OpenclDevice,OpenclCalcBase {

public:
    KernelEnv kEnv;
    cl_mem mpClmemSrcData;
    cl_mem mpClmemStartPos;
    cl_mem mpClmemEndPos;
    cl_mem mpClmemLeftData;
    cl_mem mpClmemRightData;


    OclCalc();
    ~OclCalc();
    double OclTest();
    double OclTestDll();
    double OclMin();
    double OclProcess(cl_kernel_function function, double *data, formulax type);

#ifdef GPU_64BITS
    int OclHostSignedAdd(double *lData,double *rData,double *rResult,int rowSize);
    int OclHostSignedSub(double *lData,double *rData,double *rResult,int rowSize);
    int OclHostSignedMul(double *lData,double *rData,double *rResult,int rowSize);
    int OclHostSignedDiv(double *lData,double *rData,double *rResult,int rowSize);
    int OclHostFormulaMax(double *srcData,int *startPos,int *endPos,double *output,int outputSize);
    int OclHostFormulaMin(double *srcData,int *startPos,int *endPos,double *output,int outputSize);
    int OclHostFormulaAverage(double *srcData,int *startPos,int *endPos,double *output,int outputSize);
#endif
    int OclHostSignedAdd32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize);
    int OclHostSignedSub32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize);
    int OclHostSignedMul32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize);
    int OclHostSignedDiv32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize);
    int OclHostFormulaMax32Bits(float *fpSrcData,uint *npStartPos,uint *npEndPos,double *output,int outputSize);
    int OclHostFormulaMin32Bits(float *fpSrcData,uint *npStartPos,uint *npEndPos,double *output,int outputSize);
    int OclHostFormulaAverage32Bits(float *fpSrcData,uint *npStartPos,uint *npEndPos,double *output,int outputSize);
    double *OclSimpleDeltaOperation(OpCode eOp, const double *pOpArray, const double *pSubtractSingle, size_t nElements);

    //int OclHostFormulaCount(int *startPos,int *endPos,float *output,int outputSize);
    //int OclHostFormulaSum(float *srcData,int *startPos,int *endPos,float *output,int outputSize);
    //int OclHostFormulaSumProduct(float *pdSrcData,int *pnStart,int *pnEnd,float *pdOutput,int nSize);

    ///////////////////////////////////////////////////////////////
    int CreateBuffer(float *&fpSrcData,uint *&npStartPos,uint *&npEndPos,int nBufferSize);
    int CreateBuffer(float *&fpLeftData,float *&fpRightData,int nBufferSize);
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
