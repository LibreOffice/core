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

#define MaxTextExtent 4096
//support AMD opencl
#define CL_QUEUE_THREAD_HANDLE_AMD 0x403E
#define CL_MAP_WRITE_INVALIDATE_REGION (1 << 2)

#if defined(_MSC_VER)
#ifndef strcasecmp
#define strcasecmp strcmp
#endif
#endif

typedef unsigned int uint;

typedef struct _KernelEnv
{
    cl_context mpkContext;
    cl_command_queue mpkCmdQueue;
    cl_program mpkProgram;
    cl_kernel mpkKernel;
    char mckKernelName[150];
} KernelEnv;

typedef struct _OpenCLEnv
{
    cl_platform_id mpOclPlatformID;
    cl_context mpOclContext;
    cl_device_id mpOclDevsID;
    cl_command_queue mpOclCmdQueue;
} OpenCLEnv;

#if defined __cplusplus
extern "C" {
#endif

//user defined, this is function wrapper which is used to set the input parameters,
//luanch kernel and copy data from GPU to CPU or CPU to GPU.
typedef int ( *cl_kernel_function )( void **userdata, KernelEnv *kenv );

#if defined __cplusplus

}
#endif

#define CHECK_OPENCL(status,name)    \
if( status != CL_SUCCESS )    \
{    \
    printf ("OpenCL error code is %d at " SAL_DETAIL_WHERE " when %s .\n", status, name);    \
    return 0;    \
}


#define MAX_KERNEL_STRING_LEN 64
#define MAX_CLFILE_NUM 50
#define MAX_CLKERNEL_NUM 200
#define MAX_KERNEL_NAME_LEN 64

typedef struct _GPUEnv
{
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
    int mnKhrFp64Flag;
    int mnAmdFp64Flag;

} GPUEnv;

typedef struct
{
    char kernelName[MAX_KERNEL_NAME_LEN + 1];
    char *kernelStr;
} kernel_node;

class OpenclCalcBase
{
public:
    OpenclCalcBase(){};
    virtual ~OpenclCalcBase(){};
    virtual int OclHostArithmeticOperator64Bits( const char* aKernelName, double *fpLeftData, double *fpRightData, double *&rResult, int nRowSize )=0;
    virtual int OclHostFormulaStatistics64Bits( const char* aKernelName, double *fpSrcData, uint *npStartPos, uint *npEndPos, double *&output, int outputSize )=0;
    virtual int OclHostFormulaCount64Bits( uint *npStartPos, uint *npEndPos, double *&dpOutput, int nSize)=0;
    virtual int OclHostFormulaSumProduct64Bits( double *fpSumProMergeLfData, double *fpSumProMergeRrData, uint *npSumSize, double *&dpOutput, int nSize )=0;

    virtual int OclHostArithmeticOperator32Bits( const char* aKernelName, float *fpLeftData, float *fpRightData, double *rResult, int nRowSize )=0;
    virtual int OclHostFormulaStatistics32Bits( const char* aKernelName, float *fpSrcData, uint *npStartPos, uint *npEndPos, double *output, int outputSize )=0;
    virtual int OclHostFormulaCount32Bits( uint *npStartPos, uint *npEndPos, double *dpOutput, int nSize)=0;
    virtual int OclHostFormulaSumProduct32Bits( float *fpSumProMergeLfData, float *fpSumProMergeRrData, uint *npSumSize, double *dpOutput, int nSize )=0;

    virtual double *OclSimpleDeltaOperation( OpCode eOp, const double *pOpArray, const double *pSubtractSingle, size_t nElements )=0;


};


class OpenclDevice
{

public:
    static GPUEnv gpuEnv;
    static int isInited;
    OpenclDevice();
    ~OpenclDevice();
    static int InitEnv();
    static int RegistOpenclKernel();
    static int ReleaseOpenclRunEnv();
    static int InitOpenclRunEnv( GPUEnv *gpu );
    static int ReleaseOpenclEnv( GPUEnv *gpuInfo );
    static int CompileKernelFile( GPUEnv *gpuInfo, const char *buildOption );
    static int InitOpenclRunEnv( int argc );
    static int CachedOfKernerPrg( const GPUEnv *gpuEnvCached, const char * clFileName );
    static int GeneratBinFromKernelSource( cl_program program, const char * clFileName );
    static int WriteBinaryToFile( const char* fileName, const char* birary, size_t numBytes );
    static int BinaryGenerated( const char * clFileName, FILE ** fhandle );
    static int CompileKernelFile( const char *filename, GPUEnv *gpuInfo, const char *buildOption );

    int InitOpenclAttr( OpenCLEnv * env );
    int ReleaseKernel( KernelEnv * env );
    int SetKernelEnv( KernelEnv *envInfo );
    int CreateKernel( char * kernelname, KernelEnv * env );
    int RunKernel( const char *kernelName, void **userdata );
    int ConvertToString( const char *filename, char **source );
    int CheckKernelName( KernelEnv *envInfo, const char *kernelName );
    int RegisterKernelWrapper( const char *kernelName, cl_kernel_function function );
    int RunKernelWrapper( cl_kernel_function function, const char * kernelName, void **usrdata );
    int GetKernelEnvAndFunc( const char *kernelName, KernelEnv *env, cl_kernel_function *function );


#ifdef WIN32
    static int LoadOpencl();
    static int OpenclInite();
    static void FreeOpenclDll();
#endif

    int GetOpenclState();
    void SetOpenclState( int state );
    inline static int AddKernelConfig( int kCount, const char *kName );

};

class OclCalc: public OpenclDevice,OpenclCalcBase
{

public:
    KernelEnv kEnv;
    cl_mem mpClmemSrcData;
    cl_mem mpClmemStartPos;
    cl_mem mpClmemEndPos;
    cl_mem mpClmemLeftData;
    cl_mem mpClmemRightData;
    cl_mem mpClmemMergeLfData;
    cl_mem mpClmemMergeRtData;
    cl_mem mpClmemMatixSumSize;
    unsigned int nFormulaColSize;
    unsigned int nFormulaRowSize;

    OclCalc();
    ~OclCalc();

// for 64bits double
    int OclHostArithmeticOperator64Bits( const char* aKernelName, double *fpLeftData, double *fpRightData, double *&rResult, int nRowSize );
    int OclHostFormulaStatistics64Bits( const char* aKernelName, double *fpSrcData, uint *npStartPos, uint *npEndPos, double *&output, int outputSize);
    int OclHostFormulaCount64Bits( uint *npStartPos, uint *npEndPos, double *&dpOutput, int nSize );
    int OclHostFormulaSumProduct64Bits( double *fpSumProMergeLfData, double *fpSumProMergeRrData, uint *npSumSize, double *&dpOutput, int nSize);
// for 32bits float
    int OclHostArithmeticOperator32Bits( const char* aKernelName, float *fpLeftData, float *fpRightData, double *rResult, int nRowSize );
    int OclHostFormulaStatistics32Bits( const char* aKernelName, float *fpSrcData, uint *npStartPos, uint *npEndPos, double *output, int outputSize);
    int OclHostFormulaCount32Bits( uint *npStartPos, uint *npEndPos, double *dpOutput, int nSize );
    int OclHostFormulaSumProduct32Bits( float *fpSumProMergeLfData, float *fpSumProMergeRrData, uint *npSumSize, double *dpOutput, int nSize );
// for groundwater
    double *OclSimpleDeltaOperation( OpCode eOp, const double *pOpArray, const double *pSubtractSingle, size_t nElements );

    ///////////////////////////////////////////////////////////////
    int CreateBuffer64Bits( double *&fpSrcData, uint *&npStartPos, uint *&npEndPos, int nBufferSize );
    int CreateBuffer64Bits( double *&fpLeftData, double *&fpRightData, int nBufferSize );
    int CreateBuffer64Bits( double *&fpSumProMergeLfData, double *&fpSumProMergeRtData, uint *&npSumSize, int nMatixSize, int nBufferSize );
    int CreateBuffer32Bits( float *&fpSrcData, uint *&npStartPos, uint *&npEndPos, int nBufferSize );
    int CreateBuffer32Bits( float *&fpLeftData, float *&fpRightData, int nBufferSize );
    int CreateBuffer32Bits( float *&fpSumProMergeLfData, float *&fpSumProMergeRtData, uint *&npSumSize, int nMatixSize, int nBufferSize );
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
