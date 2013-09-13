/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_OPENCLWRAPPER_HXX
#define SC_OPENCLWRAPPER_HXX

#include <config_features.h>
#include <formula/opcode.hxx>
#include <sal/detail/log.h>
#include <cassert>
#include "platforminfo.hxx"

#include "clcc/clew.h"

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

#define CHECK_OPENCL(status,name)    \
if( status != CL_SUCCESS )    \
{    \
    printf ("OpenCL error code is %d at " SAL_DETAIL_WHERE " when %s .\n", status, name);    \
    return 0;    \
}

#define CHECK_OPENCL_VOID(status,name)    \
if( status != CL_SUCCESS )    \
{    \
    printf ("OpenCL error code is %d at " SAL_DETAIL_WHERE " when %s .\n", status, name);    \
}

#define CHECK_OPENCL_RELEASE(status,name)    \
if ( name != NULL )    \
    clReleaseMemObject( name );    \
if( status != CL_SUCCESS )    \
{    \
    printf ("OpenCL error code is %d at " SAL_DETAIL_WHERE " when clReleaseMemObject( %s ).\n", status, #name);    \
}

#define MAX_KERNEL_STRING_LEN 64
#define MAX_CLFILE_NUM 50
#define MAX_CLKERNEL_NUM 200
#define MAX_KERNEL_NAME_LEN 64

#if defined(_MSC_VER)
#ifndef strcasecmp
#define strcasecmp strcmp
#endif
#endif

#include <cstdio>
#include <vector>

typedef struct _KernelEnv
{
    cl_context mpkContext;
    cl_command_queue mpkCmdQueue;
    cl_program mpkProgram;
    cl_kernel mpkKernel;
    char mckKernelName[150];
} KernelEnv;

extern "C" {

// user defined, this is function wrapper which is used to set the input
// parameters, launch kernel and copy data from GPU to CPU or CPU to GPU.
typedef int ( *cl_kernel_function )( void **userdata, KernelEnv *kenv );

}

namespace sc { namespace opencl {

typedef unsigned int uint;

typedef struct _OpenCLEnv
{
    cl_platform_id mpOclPlatformID;
    cl_context mpOclContext;
    cl_device_id mpOclDevsID;
    cl_command_queue mpOclCmdQueue;
} OpenCLEnv;

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
typedef struct _SingleVectorFormula
{
    const double *mdpInputLeftData;
    const double *mdpInputRightData;
    size_t mnInputLeftDataSize;
    size_t mnInputRightDataSize;
    uint mnInputLeftStartPosition;
    uint mnInputRightStartPosition;
    int mnInputLeftOffset;
    int mnInputRightOffset;
} SingleVectorFormula;

typedef struct _DoubleVectorFormula
{
    const double *mdpInputData;
    size_t mnInputDataSize;
    uint mnInputStartPosition;
    uint mnInputEndPosition;
    int mnInputStartOffset;
    int mnInputEndOffset;
} DoubleVectorFormula;
class OpenclCalcBase
{
public:
    OpenclCalcBase(){};
    virtual ~OpenclCalcBase(){};
    virtual int oclHostArithmeticOperator64Bits( const char* aKernelName, double *&rResult, int nRowSize )=0;
    virtual int oclMoreColHostArithmeticOperator64Bits( int nDataSize,int neOpSize,double *rResult, int nRowSize )=0;
    virtual int oclHostFormulaStatistics64Bits( const char* aKernelName,double *&output, int outputSize )=0;
    virtual int oclHostFormulaCount64Bits( uint *npStartPos, uint *npEndPos, double *&dpOutput, int nSize)=0;
    virtual int oclHostFormulaSumProduct64Bits( double *fpSumProMergeLfData, double *fpSumProMergeRrData, uint *npSumSize, double *&dpOutput, int nSize )=0;
    virtual int oclHostMatrixInverse64Bits( const char* aKernelName, double *dpOclMatrixSrc, double *dpOclMatrixDst,std::vector<double>&dpResult, uint nDim)=0;
    virtual int oclMoreColHostArithmeticOperator32Bits( int nDataSize,int neOpSize,double *rResult, int nRowSize )=0;

    virtual int oclHostArithmeticOperator32Bits( const char* aKernelName, double *rResult, int nRowSize )=0;
    virtual int oclHostFormulaStatistics32Bits( const char* aKernelName,double *output, int outputSize )=0;
    virtual int oclHostFormulaCount32Bits( uint *npStartPos, uint *npEndPos, double *dpOutput, int nSize)=0;
    virtual int oclHostFormulaSumProduct32Bits( float *fpSumProMergeLfData, float *fpSumProMergeRrData, uint *npSumSize, double *dpOutput, int nSize )=0;
    virtual int oclHostMatrixInverse32Bits( const char* aKernelName, float *fpOclMatrixSrc, float *fpOclMatrixDst, std::vector<double>& dpResult, uint nDim )=0;

    virtual int oclGroundWaterGroup( uint *eOp, uint eOpNum, const double *pOpArray, const double *pSubtractSingle, size_t nSrcDataSize,size_t nElements, double delta,uint *nStartPos,uint *nEndPos ,double *deResult)=0;
    virtual double *oclSimpleDeltaOperation( OpCode eOp, const double *pOpArray, const double *pSubtractSingle, size_t nElements, double delta )=0;


};


class OpenclDevice
{

public:
    static GPUEnv gpuEnv;
    static int isInited;
    OpenclDevice();
    ~OpenclDevice();
    static int initEnv();
    static int registOpenclKernel();
    static int releaseOpenclRunEnv();
    static int initOpenclRunEnv( GPUEnv *gpu );
    static int releaseOpenclEnv( GPUEnv *gpuInfo );
    static int compileKernelFile( GPUEnv *gpuInfo, const char *buildOption );
    static int initOpenclRunEnv( int argc );
    static int cachedOfKernerPrg( const GPUEnv *gpuEnvCached, const char * clFileName );
    static int generatBinFromKernelSource( cl_program program, const char * clFileName );
    static int writeBinaryToFile( const char* fileName, const char* birary, size_t numBytes );
    static int binaryGenerated( const char * clFileName, FILE ** fhandle );
    static int compileKernelFile( const char *filename, GPUEnv *gpuInfo, const char *buildOption );

    int initOpenclAttr( OpenCLEnv * env );
    int releaseKernel( KernelEnv * env );
    int setKernelEnv( KernelEnv *envInfo );
    int createKernel( char * kernelname, KernelEnv * env );
    int runKernel( const char *kernelName, void **userdata );
    int convertToString( const char *filename, char **source );
    int checkKernelName( KernelEnv *envInfo, const char *kernelName );
    int registerKernelWrapper( const char *kernelName, cl_kernel_function function );
    int runKernelWrapper( cl_kernel_function function, const char * kernelName, void **usrdata );
    int getKernelEnvAndFunc( const char *kernelName, KernelEnv *env, cl_kernel_function *function );

    int getOpenclState();
    void setOpenclState( int state );
    static int addKernelConfig( int kCount, const char *kName );
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
    cl_mem mpClmemeOp;
    unsigned int nArithmeticLen;
    unsigned int nFormulaLen;
    unsigned int nClmemLen;
    unsigned int nFormulaColSize;
    unsigned int nFormulaRowSize;

    OclCalc();
    ~OclCalc();

// for 64bits double
    int oclHostArithmeticOperator64Bits( const char* aKernelName,  double *&rResult, int nRowSize );
    int oclMoreColHostArithmeticOperator64Bits( int nDataSize,int neOpSize,double *rResult, int nRowSize );
    int oclHostFormulaStatistics64Bits( const char* aKernelName, double *&output, int outputSize);
    int oclHostFormulaStash64Bits( const char* aKernelName, const double* dpSrcData, uint *nStartPos, uint *nEndPos, double *output, int nBufferSize, int size);
    int oclHostFormulaCount64Bits( uint *npStartPos, uint *npEndPos, double *&dpOutput, int nSize );
    int oclHostFormulaSumProduct64Bits( double *fpSumProMergeLfData, double *fpSumProMergeRrData, uint *npSumSize, double *&dpOutput, int nSize);
    int oclHostMatrixInverse64Bits( const char* aKernelName, double *dpOclMatrixSrc, double *dpOclMatrixDst, std::vector<double>&dpResult, uint nDim );
// for 32bits float
    int oclHostArithmeticOperator32Bits( const char* aKernelName, double *rResult, int nRowSize );
    int oclMoreColHostArithmeticOperator32Bits( int nDataSize,int neOpSize,double *rResult, int nRowSize );
    int oclHostFormulaStatistics32Bits( const char* aKernelName, double *output, int outputSize);
    int oclHostFormulaCount32Bits( uint *npStartPos, uint *npEndPos, double *dpOutput, int nSize );
    int oclHostArithmeticStash64Bits( const char* aKernelName, const double *dpLeftData, const double *dpRightData, double *rResult,int nRowSize );
    int oclHostFormulaSumProduct32Bits( float *fpSumProMergeLfData, float *fpSumProMergeRrData, uint *npSumSize, double *dpOutput, int nSize );
    int oclHostMatrixInverse32Bits( const char* aKernelName, float *fpOclMatrixSrc, float *fpOclMatrixDst, std::vector<double>& dpResult, uint nDim );
// for groundwater
    int oclGroundWaterGroup( uint *eOp, uint eOpNum, const double *pOpArray, const double *pSubtractSingle,size_t nSrcDataSize, size_t nElements, double delta ,uint *nStartPos,uint *nEndPos,double *deResult);
    double *oclSimpleDeltaOperation( OpCode eOp, const double *pOpArray, const double *pSubtractSingle, size_t nElements, double delta );

    ///////////////////////////////////////////////////////////////
    int createBuffer64Bits( double *&dpLeftData, double *&dpRightData, int nBufferSize );
    int mapAndCopy64Bits(const double *dpTempLeftData,const double *dpTempRightData,int nBufferSize );
    int mapAndCopy64Bits(const double *dpTempSrcData,unsigned int *unStartPos,unsigned int *unEndPos,int nBufferSize ,int nRowsize);
    int mapAndCopyArithmetic64Bits( const double *dpMoreArithmetic,int nBufferSize );
    int mapAndCopyMoreColArithmetic64Bits( const double *dpMoreColArithmetic,int nBufferSize ,uint *npeOp,uint neOpSize );
    int createMoreColArithmeticBuf64Bits( int nBufferSize, int neOpSize );

    int createFormulaBuf64Bits( int nBufferSize, int rowSize );
    int createArithmeticOptBuf64Bits( int nBufferSize );

    int createBuffer32Bits( float *&fpLeftData, float *&fpRightData, int nBufferSize );
    int mapAndCopy32Bits(const double *dpTempLeftData,const double *dpTempRightData,int nBufferSize );
    int mapAndCopy32Bits(const double *dpTempSrcData,unsigned int *unStartPos,unsigned int *unEndPos,int nBufferSize ,int nRowsize);
    int mapAndCopyArithmetic32Bits( const double *dpMoreColArithmetic, int nBufferSize );
    int mapAndCopyMoreColArithmetic32Bits( const double *dpMoreColArithmetic,int nBufferSize ,uint *npeOp,uint neOpSize );
    int createMoreColArithmeticBuf32Bits( int nBufferSize, int neOpSize );
    int createFormulaBuf32Bits( int nBufferSize, int rowSize  );
    int createArithmeticOptBuf32Bits( int nBufferSize );
    int oclHostFormulaStash32Bits( const char* aKernelName, const double* dpSrcData, uint *nStartPos, uint *nEndPos, double *output, int nBufferSize, int size );
    int oclHostArithmeticStash32Bits( const char* aKernelName, const double *dpLeftData, const double *dpRightData, double *rResult,int nRowSize );

    int releaseOclBuffer(void);
    friend class agency;
};

size_t getOpenCLPlatformCount();
void fillOpenCLInfo(std::vector<OpenclPlatformInfo>& rPlatforms);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
