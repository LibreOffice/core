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

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#define MaxTextExtent  4096

#if defined(_MSC_VER)
#ifndef strcasecmp
#define strcasecmp strcmp
#endif
#endif

typedef struct _KernelEnv {
    cl_context context;
    cl_command_queue commandQueue;
    cl_program program;
    cl_kernel kernel;
    char kernelName[150];
} KernelEnv;

typedef struct _OpenCLEnv {
    cl_platform_id platform;
    cl_context context;
    cl_device_id devices;
    cl_command_queue commandQueue;
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
{                                         \
    printf ("error code is %d.\n",status);  \
    return (0);                           \
}


#define MAX_KERNEL_STRING_LEN   64
#define MAX_CLFILE_NUM 50
#define MAX_CLKERNEL_NUM 200
#define MAX_KERNEL_NAME_LEN 64

typedef struct _GPUEnv {
    //share vb in all modules in hb library
    cl_platform_id platform;
    cl_device_type dType;
    cl_context context;
    cl_device_id *devices;
    cl_device_id dev;
    cl_command_queue commandQueue;
    cl_kernel kernels[MAX_CLFILE_NUM];
    cl_program programs[MAX_CLFILE_NUM]; //one program object maps one kernel source file
    char kernelSrcFile[MAX_CLFILE_NUM][256], //the max len of kernel file name is 256
		 kernelNames[MAX_CLKERNEL_NUM][MAX_KERNEL_STRING_LEN + 1];
		 cl_kernel_function kernelFunctions[MAX_CLKERNEL_NUM];
    int kernelCount, fileCount, // only one kernel file
        isUserCreated; // 1: created , 0:no create and needed to create by opencl wrapper

} GPUEnv;

typedef struct {
    char kernelName[MAX_KERNEL_NAME_LEN + 1];
    char *kernelStr;
} kernel_node;

class OpenclCalcBase{
public:
    OpenclCalcBase(){};
    virtual ~OpenclCalcBase(){};
    virtual int OclHostSignedAdd(double *lData,double *rData,double *rResult,int rowSize)=0;
    virtual int OclHostSignedSub(double *lData,double *rData,double *rResult,int rowSize)=0;
    virtual int OclHostSignedMul(double *lData,double *rData,double *rResult,int rowSize)=0;
    virtual int OclHostSignedDiv(double *lData,double *rData,double *rResult,int rowSize)=0;
    virtual int OclHostFormulaMax(double *srcData,int *startPos,int *endPos,double *output,int outputSize)=0;
    virtual int OclHostFormulaMin(double *srcData,int *startPos,int *endPos,double *output,int outputSize)=0;
    virtual int OclHostFormulaAverage(double *srcData,int *startPos,int *endPos,double *output,int outputSize)=0;

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

    int ReleaseKernel(KernelEnv * env);
    int InitOpenclAttr(OpenCLEnv * env);
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
    OclCalc();
    ~OclCalc();
    double OclTest();
	double OclTestDll();
    double OclMin();
	double OclProcess(cl_kernel_function function, double *data, formulax type);
	int OclHostSignedAdd(double *lData,double *rData,double *rResult,int rowSize);
	int OclHostSignedSub(double *lData,double *rData,double *rResult,int rowSize);
	int OclHostSignedMul(double *lData,double *rData,double *rResult,int rowSize);
	int OclHostSignedDiv(double *lData,double *rData,double *rResult,int rowSize);
	int OclHostFormulaMax(double *srcData,int *startPos,int *endPos,double *output,int outputSize);
	int OclHostFormulaMin(double *srcData,int *startPos,int *endPos,double *output,int outputSize);
	int OclHostFormulaAverage(double *srcData,int *startPos,int *endPos,double *output,int outputSize);
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
