/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "openclwrapper.hxx"

#include <rtl/ustring.hxx>
#include <boost/scoped_array.hpp>

#include "sal/config.h"
#include "oclkernels.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

#ifdef WIN32
#include <Windows.h>

#define TRUE 1
#define FALSE 0

#define OCL_INFO(str) \
    printf("[OCL_INFO] %s\n",str);
#define OCL_ERROR(str) \
    fprintf(stderr,"[OCL_ERROR] %s\n",str);
#define OCL_CHECK(value1,value2,str) \
    if(value1!=value2) \
        fprintf(stderr,"[OCL_ERROR] %s\n",str);

#define OPENCL_DLL_NAME "OpenCL.dll"
#else
#define OPENCL_DLL_NAME "libOpenCL.so"
#endif

using namespace std;

namespace sc { namespace opencl {

GPUEnv OpenclDevice::gpuEnv;
int OpenclDevice::isInited =0;

int OpenclDevice::initEnv()
{
    // TODO: This part needs more platform specific handling.  On Windows,
    // the GPU Driver itself  installs OpenCL.dll in the system folder.
    int status = clewInit(OPENCL_DLL_NAME);
    if (status < 0)
        return 1;

    initOpenclRunEnv( 0 );
    return 1;
}

int OpenclDevice::releaseOpenclRunEnv()
{
    releaseOpenclEnv( &gpuEnv );

    return 1;
}
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
int OpenclDevice::addKernelConfig( int kCount, const char *kName )
{
    if ( kCount < 1 )
        fprintf(stderr,"Error: ( KCount < 1 )" SAL_DETAIL_WHERE "addKernelConfig\n" );
    strcpy( gpuEnv.mArrykernelNames[kCount-1], kName );
    gpuEnv.mnKernelCount++;
    return 0;
}

int OpenclDevice::registOpenclKernel()
{
    if ( !gpuEnv.mnIsUserCreated )
        memset( &gpuEnv, 0, sizeof(gpuEnv) );

    gpuEnv.mnFileCount = 0; //argc;
    gpuEnv.mnKernelCount = 0UL;

    addKernelConfig( 1, (const char*) "oclFormulaMin" );
    addKernelConfig( 2, (const char*) "oclFormulaMax" );
    addKernelConfig( 3, (const char*) "oclFormulaSum" );
    addKernelConfig( 4, (const char*) "oclFormulaCount" );
    addKernelConfig( 5, (const char*) "oclFormulaAverage" );
    addKernelConfig( 6, (const char*) "oclFormulaSumproduct" );
    addKernelConfig( 7, (const char*) "oclFormulaMtxInv" );

    addKernelConfig( 8, (const char*) "oclSignedAdd" );
    addKernelConfig( 9, (const char*) "oclSignedSub" );
    addKernelConfig( 10, (const char*) "oclSignedMul" );
    addKernelConfig( 11, (const char*) "oclSignedDiv" );
    addKernelConfig( 12, (const char*) "oclAverageDelta" );
    addKernelConfig( 13, (const char*) "oclMaxDelta" );
    addKernelConfig( 14, (const char*) "oclMinDelta" );
    addKernelConfig( 15, (const char*) "oclSubDelta" );
    addKernelConfig( 16, (const char*) "oclLUDecomposition" );
    addKernelConfig( 17, (const char*) "oclAverageDeltaRPN" );
    addKernelConfig( 18, (const char*) "oclMaxDeltaRPN" );
    addKernelConfig( 19, (const char*) "oclMinDeltaRPN" );
    addKernelConfig( 20, (const char*) "oclMoreColArithmeticOperator" );
    addKernelConfig( 21, (const char*) "oclColumnH" );
    addKernelConfig( 22, (const char*) "oclColumnL" );
    addKernelConfig( 23, (const char*) "oclColumnN" );
    addKernelConfig( 24, (const char*) "oclColumnJ" );
    addKernelConfig( 25, (const char*) "oclMaxSub" );
    addKernelConfig( 26, (const char*) "oclAverageSub" );
    addKernelConfig( 27, (const char*) "oclMinSub" );
    addKernelConfig( 28, (const char*) "oclMaxAdd" );
    addKernelConfig( 29, (const char*) "oclAverageAdd" );
    addKernelConfig( 30, (const char*) "oclMinAdd" );
    addKernelConfig( 31, (const char*) "oclMaxMul" );
    addKernelConfig( 32, (const char*) "oclAverageMul" );
    addKernelConfig( 33, (const char*) "oclMinMul" );
    addKernelConfig( 34, (const char*) "oclMaxDiv" );
    addKernelConfig( 35, (const char*) "oclAverageDiv" );
    addKernelConfig( 36, (const char*) "oclMinDiv" );
    addKernelConfig( 37, (const char*) "oclSub" );// for svDouble type
    return 0;
}

OpenclDevice::OpenclDevice()
{
    //initEnv();
}

OpenclDevice::~OpenclDevice()
{
    //releaseOpenclRunEnv();
}

int OpenclDevice::setKernelEnv( KernelEnv *envInfo )
{
    envInfo->mpkContext = gpuEnv.mpContext;
    envInfo->mpkCmdQueue = gpuEnv.mpCmdQueue;
    envInfo->mpkProgram = gpuEnv.mpArryPrograms[0];

    return 1;
}

int OpenclDevice::checkKernelName( KernelEnv *envInfo, const char *kernelName )
{
    //printf("checkKernelName,total count of kernels...%d\n", gpuEnv.kernelCount);
    int kCount;
    int nFlag = 0;
    for ( kCount=0; kCount < gpuEnv.mnKernelCount; kCount++ )
    {
        if ( strcasecmp( kernelName, gpuEnv.mArrykernelNames[kCount]) == 0 )
        {
            nFlag = 1;
            printf("match %s kernel right\n",kernelName);
            break;
        }
    }
    if ( !nFlag )
    {
        printf("can't find kernel: %s\n",kernelName);
    }
    envInfo->mpkKernel = gpuEnv.mpArryKernels[kCount];
    strcpy( envInfo->mckKernelName, kernelName );
    if ( envInfo == (KernelEnv *) NULL )
    {
        printf("get err func and env\n");
        return 0;
    }
    return 1;
}

int OpenclDevice::convertToString( const char *filename, char **source )
{
    int file_size;
    size_t result;
    FILE *file = NULL;
    file_size = 0;
    result = 0;
    file = fopen( filename, "rb+" );
    printf("open kernel file %s.\n",filename);

    if ( file != NULL )
    {
        printf("Open ok!\n");
        fseek( file, 0, SEEK_END );

        file_size = ftell( file );
        rewind( file );
        *source = (char*) malloc( sizeof(char) * file_size + 1 );
        if ( *source == (char*) NULL )
        {
            return 0;
        }
        result = fread(*source, 1, file_size, file);
        if ( result != (size_t) file_size )
        {
            free( *source );
            return 0;
        }
        (*source)[file_size] = '\0';
        fclose( file );

        return 1;
    }
    printf("open kernel file failed.\n");
    return 0;
}

int OpenclDevice::binaryGenerated( const char * clFileName, FILE ** fhandle )
{
    unsigned int i = 0;
    cl_int clStatus;
    int status = 0;
    char *str = NULL;
    FILE *fd = NULL;
    cl_uint numDevices=0;
    if ( getenv("SC_OPENCLCPU") )
    {
        clStatus = clGetDeviceIDs(gpuEnv.mpPlatformID, // platform
                                  CL_DEVICE_TYPE_CPU,  // device_type for CPU device
                                  0,                   // num_entries
                                  NULL,                // devices ID
                                  &numDevices);
    }
    else
    {
        clStatus = clGetDeviceIDs(gpuEnv.mpPlatformID, // platform
                                  CL_DEVICE_TYPE_GPU,  // device_type for GPU device
                                  0,                   // num_entries
                                  NULL,                // devices ID
                                  &numDevices);
    }
    CHECK_OPENCL( clStatus, "clGetDeviceIDs" );
    for ( i = 0; i < numDevices; i++ )
    {
        char fileName[256] = { 0 }, cl_name[128] = { 0 };
        if ( gpuEnv.mpArryDevsID[i] != 0 )
        {
            char deviceName[1024];
            clStatus = clGetDeviceInfo( gpuEnv.mpArryDevsID[i], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL );
            CHECK_OPENCL( clStatus, "clGetDeviceInfo" );
            str = (char*) strstr( clFileName, (char*) ".cl" );
            memcpy( cl_name, clFileName, str - clFileName );
            cl_name[str - clFileName] = '\0';
            sprintf( fileName, "./%s-%s.bin", cl_name, deviceName );
            fd = fopen( fileName, "rb" );
            status = ( fd != NULL ) ? 1 : 0;
        }
    }
    if ( fd != NULL )
    {
        *fhandle = fd;
    }
    return status;

}

int OpenclDevice::writeBinaryToFile( const char* fileName, const char* birary, size_t numBytes )
{
    FILE *output = NULL;
    output = fopen( fileName, "wb" );
    if ( output == NULL )
    {
        return 0;
    }

    fwrite( birary, sizeof(char), numBytes, output );
    fclose( output );

    return 1;

}

int OpenclDevice::generatBinFromKernelSource( cl_program program, const char * clFileName )
{
    unsigned int i = 0;
    cl_int clStatus;
    size_t *binarySizes, numDevices;
    cl_device_id *mpArryDevsID;
    char **binaries, *str = NULL;

    clStatus = clGetProgramInfo( program, CL_PROGRAM_NUM_DEVICES,
                   sizeof(numDevices), &numDevices, NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    mpArryDevsID = (cl_device_id*) malloc( sizeof(cl_device_id) * numDevices );
    if ( mpArryDevsID == NULL )
    {
        return 0;
    }
    /* grab the handles to all of the devices in the program. */
    clStatus = clGetProgramInfo( program, CL_PROGRAM_DEVICES,
                   sizeof(cl_device_id) * numDevices, mpArryDevsID, NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    /* figure out the sizes of each of the binaries. */
    binarySizes = (size_t*) malloc( sizeof(size_t) * numDevices );

    clStatus = clGetProgramInfo( program, CL_PROGRAM_BINARY_SIZES,
                   sizeof(size_t) * numDevices, binarySizes, NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    /* copy over all of the generated binaries. */
    binaries = (char**) malloc( sizeof(char *) * numDevices );
    if ( binaries == NULL )
    {
        return 0;
    }

    for ( i = 0; i < numDevices; i++ )
    {
        if ( binarySizes[i] != 0 )
        {
            binaries[i] = (char*) malloc( sizeof(char) * binarySizes[i] );
            if ( binaries[i] == NULL )
            {
                return 0;
            }
        }
        else
        {
            binaries[i] = NULL;
        }
    }

    clStatus = clGetProgramInfo( program, CL_PROGRAM_BINARIES,
                   sizeof(char *) * numDevices, binaries, NULL );
    CHECK_OPENCL(clStatus,"clGetProgramInfo");

    /* dump out each binary into its own separate file. */
    for ( i = 0; i < numDevices; i++ )
    {
        char fileName[256] = { 0 }, cl_name[128] = { 0 };

        if ( binarySizes[i] != 0 )
        {
            char deviceName[1024];
            clStatus = clGetDeviceInfo(mpArryDevsID[i], CL_DEVICE_NAME,
                           sizeof(deviceName), deviceName, NULL);
            CHECK_OPENCL( clStatus, "clGetDeviceInfo" );

            str = (char*) strstr( clFileName, (char*) ".cl" );
            memcpy( cl_name, clFileName, str - clFileName );
            cl_name[str - clFileName] = '\0';
            sprintf( fileName, "./%s-%s.bin", cl_name, deviceName );

            if ( !writeBinaryToFile( fileName, binaries[i], binarySizes[i] ) )
            {
                printf("opencl-wrapper: write binary[%s] failds\n", fileName);
                return 0;
            } //else
            printf("opencl-wrapper: write binary[%s] succesfully\n", fileName);
        }
    }

    // Release all resouces and memory
    for ( i = 0; i < numDevices; i++ )
    {
        if ( binaries[i] != NULL )
        {
            free( binaries[i] );
            binaries[i] = NULL;
        }
    }

    if ( binaries != NULL )
    {
        free( binaries );
        binaries = NULL;
    }

    if ( binarySizes != NULL )
    {
        free( binarySizes );
        binarySizes = NULL;
    }

    if ( mpArryDevsID != NULL )
    {
        free( mpArryDevsID );
        mpArryDevsID = NULL;
    }
    return 1;
}

int OpenclDevice::initOpenclAttr( OpenCLEnv * env )
{
    if ( gpuEnv.mnIsUserCreated )
        return 1;

    gpuEnv.mpContext = env->mpOclContext;
    gpuEnv.mpPlatformID = env->mpOclPlatformID;
    gpuEnv.mpDevID = env->mpOclDevsID;
    gpuEnv.mpCmdQueue = env->mpOclCmdQueue;

    gpuEnv.mnIsUserCreated = 1;

    return 0;
}

int OpenclDevice::createKernel( char * kernelname, KernelEnv * env )
{
    int clStatus;

    env->mpkKernel = clCreateKernel( gpuEnv.mpArryPrograms[0], kernelname, &clStatus );
    env->mpkContext = gpuEnv.mpContext;
    env->mpkCmdQueue = gpuEnv.mpCmdQueue;
    return clStatus != CL_SUCCESS ? 1 : 0;
}

int OpenclDevice::releaseKernel( KernelEnv * env )
{
    int clStatus = clReleaseKernel( env->mpkKernel );
    return clStatus != CL_SUCCESS ? 1 : 0;
}

int OpenclDevice::releaseOpenclEnv( GPUEnv *gpuInfo )
{
    int i = 0;
    int clStatus = 0;

    if ( !isInited )
    {
        return 1;
    }

    for ( i = 0; i < gpuEnv.mnFileCount; i++ )
    {
        if ( gpuEnv.mpArryPrograms[i] )
        {
            clStatus = clReleaseProgram( gpuEnv.mpArryPrograms[i] );
            CHECK_OPENCL( clStatus, "clReleaseProgram" );
            gpuEnv.mpArryPrograms[i] = NULL;
        }
    }
    if ( gpuEnv.mpCmdQueue )
    {
        clReleaseCommandQueue( gpuEnv.mpCmdQueue );
        gpuEnv.mpCmdQueue = NULL;
    }
    if ( gpuEnv.mpContext )
    {
        clReleaseContext( gpuEnv.mpContext );
        gpuEnv.mpContext = NULL;
    }
    isInited = 0;
    gpuInfo->mnIsUserCreated = 0;
    free( gpuInfo->mpArryDevsID );
    return 1;
}

int OpenclDevice::runKernelWrapper( cl_kernel_function function, const char * kernelName, void **usrdata )
{
    printf("oclwrapper:runKernel_wrapper...\n");
    if ( registerKernelWrapper( kernelName, function ) != 1 )
    {
        fprintf(stderr, "Error:runKernel_wrapper:registerKernelWrapper fail!\n");
        return -1;
    }
    return ( runKernel( kernelName, usrdata ) );
}

int OpenclDevice::cachedOfKernerPrg( const GPUEnv *gpuEnvCached, const char * clFileName )
{
    int i;
    for ( i = 0; i < gpuEnvCached->mnFileCount; i++ )
    {
        if ( strcasecmp( gpuEnvCached->mArryKnelSrcFile[i], clFileName ) == 0 )
        {
            if ( gpuEnvCached->mpArryPrograms[i] != NULL )
            {
                return 1;
            }
        }
    }

    return 0;
}

int OpenclDevice::compileKernelFile( GPUEnv *gpuInfo, const char *buildOption )
{
    cl_int clStatus = 0;
    size_t length;
    char *buildLog = NULL, *binary;
    const char *source;
    size_t source_size[1];
    int b_error, binary_status, binaryExisted, idx;
    size_t numDevices;
    cl_device_id *mpArryDevsID;
    FILE *fd, *fd1;
    const char* filename = "kernel.cl";
    fprintf(stderr, "compileKernelFile ... \n");
    if ( cachedOfKernerPrg(gpuInfo, filename) == 1 )
    {
        return 1;
    }

    idx = gpuInfo->mnFileCount;

    source = kernel_src;

    source_size[0] = strlen( source );
    binaryExisted = 0;
    if ( ( binaryExisted = binaryGenerated( filename, &fd ) ) == 1 )
    {
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_NUM_DEVICES,
                       sizeof(numDevices), &numDevices, NULL );
        CHECK_OPENCL( clStatus, "clGetContextInfo" );

        mpArryDevsID = (cl_device_id*) malloc( sizeof(cl_device_id) * numDevices );
        if ( mpArryDevsID == NULL )
        {
            return 0;
        }

        b_error = 0;
        length = 0;
        b_error |= fseek( fd, 0, SEEK_END ) < 0;
        b_error |= ( length = ftell(fd) ) <= 0;
        b_error |= fseek( fd, 0, SEEK_SET ) < 0;
        if ( b_error )
        {
            return 0;
        }

        binary = (char*) malloc( length + 2 );
        if ( !binary )
        {
            return 0;
        }

        memset( binary, 0, length + 2 );
        b_error |= fread( binary, 1, length, fd ) != length;


        fclose( fd );
        fd = NULL;
        // grab the handles to all of the devices in the context.
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES,
                       sizeof( cl_device_id ) * numDevices, mpArryDevsID, NULL );
        CHECK_OPENCL( clStatus, "clGetContextInfo" );

        fprintf(stderr, "Create kernel from binary\n");
        gpuInfo->mpArryPrograms[idx] = clCreateProgramWithBinary( gpuInfo->mpContext,numDevices,
                                           mpArryDevsID, &length, (const unsigned char**) &binary,
                                           &binary_status, &clStatus );
        CHECK_OPENCL( clStatus, "clCreateProgramWithBinary" );

        free( binary );
        free( mpArryDevsID );
        mpArryDevsID = NULL;
    }
    else
    {
        // create a CL program using the kernel source
        fprintf(stderr, "Create kernel from source\n");
        gpuEnv.mpArryPrograms[idx] = clCreateProgramWithSource( gpuEnv.mpContext, 1, &source,
                                         source_size, &clStatus);
        CHECK_OPENCL( clStatus, "clCreateProgramWithSource" );
    }

    if ( gpuInfo->mpArryPrograms[idx] == (cl_program) NULL )
    {
        return 0;
    }

    //char options[512];
    // create a cl program executable for all the devices specified
    printf("BuildProgram.\n");
    if (!gpuInfo->mnIsUserCreated)
    {
        clStatus = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, gpuInfo->mpArryDevsID,
                       buildOption, NULL, NULL);
    }
    else
    {
        clStatus = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, &(gpuInfo->mpDevID),
                       buildOption, NULL, NULL);
    }

    if ( clStatus != CL_SUCCESS )
    {
        printf ("BuildProgram error!\n");
        if ( !gpuInfo->mnIsUserCreated )
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpArryDevsID[0],
                           CL_PROGRAM_BUILD_LOG, 0, NULL, &length );
        }
        else
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpDevID,
                           CL_PROGRAM_BUILD_LOG, 0, NULL, &length);
        }
        if ( clStatus != CL_SUCCESS )
        {
            printf("opencl create build log fail\n");
            return 0;
        }
        buildLog = (char*) malloc( length );
        if ( buildLog == (char*) NULL )
        {
            return 0;
        }
        if ( !gpuInfo->mnIsUserCreated )
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpArryDevsID[0],
                           CL_PROGRAM_BUILD_LOG, length, buildLog, &length );
        }
        else
        {
            clStatus = clGetProgramBuildInfo( gpuInfo->mpArryPrograms[idx], gpuInfo->mpDevID,
                           CL_PROGRAM_BUILD_LOG, length, buildLog, &length );
        }
        if ( clStatus != CL_SUCCESS )
        {
            printf("opencl program build info fail\n");
            return 0;
        }

        fd1 = fopen( "kernel-build.log", "w+" );
        if ( fd1 != NULL )
        {
            fwrite( buildLog, sizeof(char), length, fd1 );
            fclose( fd1 );
        }

        free( buildLog );
        return 0;
    }

    strcpy( gpuEnv.mArryKnelSrcFile[idx], filename );

    if ( binaryExisted == 0 )
        generatBinFromKernelSource( gpuEnv.mpArryPrograms[idx], filename );

    gpuInfo->mnFileCount += 1;

    return 1;
}

int OpenclDevice::getKernelEnvAndFunc( const char *kernelName, KernelEnv *env, cl_kernel_function *function)
{
    int i;
    //printf("----------------OpenclDevice::getKernelEnvAndFunc\n");
    for ( i = 0; i < gpuEnv.mnKernelCount; i++ )
    {
        if ( strcasecmp( kernelName, gpuEnv.mArrykernelNames[i]) == 0 )
        {
            env->mpkContext = gpuEnv.mpContext;
            env->mpkCmdQueue = gpuEnv.mpCmdQueue;
            env->mpkProgram = gpuEnv.mpArryPrograms[0];
            env->mpkKernel = gpuEnv.mpArryKernels[i];
            *function = gpuEnv.mpArryKnelFuncs[i];
            return 1;
        }
    }
    return 0;
}

int OpenclDevice::runKernel( const char *kernelName, void **userdata)
{
    KernelEnv kEnv;
    cl_kernel_function function;
    int status;

    memset( &kEnv, 0, sizeof( KernelEnv ) );
    status = getKernelEnvAndFunc( kernelName, &kEnv, &function );
    strcpy( kEnv.mckKernelName, kernelName );
    if ( status == 1 )
    {
        if ( &kEnv == (KernelEnv *) NULL || &function == (cl_kernel_function *) NULL)
            return 0;
        return ( function( userdata, &kEnv ) );
    }
    return 0;
}

int OpenclDevice::initOpenclRunEnv( int argc )
{
    if ( MAX_CLKERNEL_NUM <= 0 )
    {
        return 1;
    }
    if ( ( argc > MAX_CLFILE_NUM ) || ( argc < 0 ) )
        return 1;

    if ( !isInited )
    {
        registOpenclKernel();
        //initialize devices, context, comand_queue
        int status = initOpenclRunEnv( &gpuEnv );
        if ( status )
        {
            printf("init_opencl_env failed.\n");
            return 1;
        }
        printf("init_opencl_env successed.\n");
        //initialize program, kernelName, kernelCount
        if( getenv( "SC_FLOAT" ) )
        {
            gpuEnv.mnKhrFp64Flag = 0;
            gpuEnv.mnAmdFp64Flag = 0;
        }
        if( gpuEnv.mnKhrFp64Flag )
        {
            printf("----use khr double type in kernel----\n");
            status = compileKernelFile( &gpuEnv, "-D KHR_DP_EXTENSION -Dfp_t=double -Dfp_t4=double4 -Dfp_t16=double16" );
        }
        else if( gpuEnv.mnAmdFp64Flag )
        {
            printf("----use amd double type in kernel----\n");
            status = compileKernelFile( &gpuEnv, "-D AMD_DP_EXTENSION -Dfp_t=double -Dfp_t4=double4 -Dfp_t16=double16" );
        }
        else
        {
            printf("----use float type in kernel----\n");
            status = compileKernelFile( &gpuEnv, "-Dfp_t=float -Dfp_t4=float4 -Dfp_t16=float16" );
        }
        if ( status == 0 || gpuEnv.mnKernelCount == 0 )
        {
            printf("compileKernelFile failed.\n");
            return 1;
        }
        printf("compileKernelFile successed.\n");
        isInited = 1;
    }
    return 0;
}

int OpenclDevice::initOpenclRunEnv( GPUEnv *gpuInfo )
{
    size_t length;
    cl_int clStatus;
    cl_uint numPlatforms, numDevices;
    cl_platform_id *platforms;
    cl_context_properties cps[3];
    char platformName[256];
    unsigned int i;

    // Have a look at the available platforms.

    if ( !gpuInfo->mnIsUserCreated )
    {
        clStatus = clGetPlatformIDs( 0, NULL, &numPlatforms );
        if ( clStatus != CL_SUCCESS )
        {
            return 1;
        }
        gpuInfo->mpPlatformID = NULL;

        if ( 0 < numPlatforms )
        {
            platforms = (cl_platform_id*) malloc( numPlatforms * sizeof( cl_platform_id ) );
            if ( platforms == (cl_platform_id*) NULL )
            {
                return 1;
            }
            clStatus = clGetPlatformIDs( numPlatforms, platforms, NULL );

            if ( clStatus != CL_SUCCESS )
            {
                return 1;
            }

            for ( i = 0; i < numPlatforms; i++ )
            {
                clStatus = clGetPlatformInfo( platforms[i], CL_PLATFORM_VENDOR,
                    sizeof( platformName ), platformName, NULL );

                if ( clStatus != CL_SUCCESS )
                {
                    return 1;
                }
                gpuInfo->mpPlatformID = platforms[i];

                //if (!strcmp(platformName, "Intel(R) Coporation"))
                //if( !strcmp( platformName, "Advanced Micro Devices, Inc." ))
                {
                    gpuInfo->mpPlatformID = platforms[i];
                    if ( getenv("SC_OPENCLCPU") )
                    {
                        clStatus = clGetDeviceIDs(gpuInfo->mpPlatformID, // platform
                                                  CL_DEVICE_TYPE_CPU,    // device_type for CPU device
                                                  0,                     // num_entries
                                                  NULL,                  // devices
                                                  &numDevices);
                    }
                    else
                    {
                          clStatus = clGetDeviceIDs(gpuInfo->mpPlatformID, // platform
                                                  CL_DEVICE_TYPE_GPU,      // device_type for GPU device
                                                  0,                       // num_entries
                                                  NULL,                    // devices
                                                  &numDevices);
                    }
                    if ( clStatus != CL_SUCCESS )
                        continue;

                    if ( numDevices )
                        break;
                }
            }
            if ( clStatus != CL_SUCCESS )
                return 1;
            free( platforms );
        }
        if ( NULL == gpuInfo->mpPlatformID )
            return 1;

        // Use available platform.
        cps[0] = CL_CONTEXT_PLATFORM;
        cps[1] = (cl_context_properties) gpuInfo->mpPlatformID;
        cps[2] = 0;
        // Set device type for OpenCL
        if ( getenv("SC_OPENCLCPU") )
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_CPU;
        }
        else
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_GPU;
        }
        gpuInfo->mpContext = clCreateContextFromType( cps, gpuInfo->mDevType, NULL, NULL, &clStatus );

        if ( ( gpuInfo->mpContext == (cl_context) NULL) || ( clStatus != CL_SUCCESS ) )
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_CPU;
            gpuInfo->mpContext = clCreateContextFromType( cps, gpuInfo->mDevType, NULL, NULL, &clStatus );
        }
        if ( ( gpuInfo->mpContext == (cl_context) NULL) || ( clStatus != CL_SUCCESS ) )
        {
            gpuInfo->mDevType = CL_DEVICE_TYPE_DEFAULT;
            gpuInfo->mpContext = clCreateContextFromType( cps, gpuInfo->mDevType, NULL, NULL, &clStatus );
        }
        if ( ( gpuInfo->mpContext == (cl_context) NULL) || ( clStatus != CL_SUCCESS ) )
            return 1;
        // Detect OpenCL devices.
        // First, get the size of device list data
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES, 0, NULL, &length );
        if ( ( clStatus != CL_SUCCESS ) || ( length == 0 ) )
            return 1;
        // Now allocate memory for device list based on the size we got earlier
        gpuInfo->mpArryDevsID = (cl_device_id*) malloc( length );
        if ( gpuInfo->mpArryDevsID == (cl_device_id*) NULL )
            return 1;
        // Now, get the device list data
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES, length,
                       gpuInfo->mpArryDevsID, NULL );
        if ( clStatus != CL_SUCCESS )
            return 1;

        // Create OpenCL command queue.
        gpuInfo->mpCmdQueue = clCreateCommandQueue( gpuInfo->mpContext, gpuInfo->mpArryDevsID[0], 0, &clStatus );

        if ( clStatus != CL_SUCCESS )
            return 1;
    }

    clStatus = clGetCommandQueueInfo( gpuInfo->mpCmdQueue, CL_QUEUE_THREAD_HANDLE_AMD, 0, NULL, NULL );
    // Check device extensions for double type
    size_t aDevExtInfoSize = 0;

    clStatus = clGetDeviceInfo( gpuInfo->mpArryDevsID[0], CL_DEVICE_EXTENSIONS, 0, NULL, &aDevExtInfoSize );
    CHECK_OPENCL( clStatus, "clGetDeviceInfo" );

    char *aExtInfo = new char[aDevExtInfoSize];

    clStatus = clGetDeviceInfo( gpuInfo->mpArryDevsID[0], CL_DEVICE_EXTENSIONS,
                   sizeof(char) * aDevExtInfoSize, aExtInfo, NULL);
    CHECK_OPENCL( clStatus, "clGetDeviceInfo" );

    gpuInfo->mnKhrFp64Flag = 0;
    gpuInfo->mnAmdFp64Flag = 0;

    if ( strstr( aExtInfo, "cl_khr_fp64" ) )
    {
        gpuInfo->mnKhrFp64Flag = 1;
    }
    else
    {
        // Check if cl_amd_fp64 extension is supported
        if ( strstr( aExtInfo, "cl_amd_fp64" ) )
            gpuInfo->mnAmdFp64Flag = 1;
    }
    delete []aExtInfo;

    return 0;
}
int OpenclDevice::registerKernelWrapper( const char *kernelName, cl_kernel_function function )
{
    int i;
    //printf("oclwrapper:registerKernelWrapper...%d\n", gpuEnv.mnKernelCount);
    for ( i = 0; i < gpuEnv.mnKernelCount; i++ )
    {
        if ( strcasecmp( kernelName, gpuEnv.mArrykernelNames[i]) == 0 )
        {
            gpuEnv.mpArryKnelFuncs[i] = function;
            return 1;
        }
    }
    return 0;
}

void OpenclDevice::setOpenclState( int state )
{
    //printf("OpenclDevice::setOpenclState...\n");
    isInited = state;
}

int OpenclDevice::getOpenclState()
{
    return isInited;
}

OclCalc::OclCalc()
{
    fprintf(stderr,"OclCalc:: init opencl ...\n");
    nFormulaColSize = 0;
    nFormulaRowSize = 0;
    nArithmeticLen = 0;
    nFormulaLen = 0;
    mpClmemSrcData = NULL;
    mpClmemStartPos = NULL;
    mpClmemEndPos = NULL;
    mpClmemLeftData = NULL;
    mpClmemRightData = NULL;
    mpClmemMergeLfData = NULL;
    mpClmemMergeRtData = NULL;
    mpClmemMatixSumSize = NULL;
    mpClmemeOp = NULL;
}

OclCalc::~OclCalc()
{
    releaseOclBuffer();
}
int OclCalc::releaseOclBuffer(void)
{
    cl_int clStatus = 0;
    CHECK_OPENCL_RELEASE( clStatus, mpClmemSrcData );
    CHECK_OPENCL_RELEASE( clStatus, mpClmemStartPos );
    CHECK_OPENCL_RELEASE( clStatus, mpClmemEndPos );
    CHECK_OPENCL_RELEASE( clStatus, mpClmemLeftData );
    CHECK_OPENCL_RELEASE( clStatus, mpClmemRightData );
    fprintf(stderr,"OclCalc:: opencl end ...\n");
    return 1;
}

/////////////////////////////////////////////////////////////////////////////

int OclCalc::createBuffer64Bits( double *&dpLeftData, double *&dpRightData, int nBufferSize )
{
    cl_int clStatus = 0;
    setKernelEnv( &kEnv );

    mpClmemLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR ),
                          nBufferSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemRightData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR ),
                           nBufferSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus,"clCreateBuffer" );
    dpLeftData = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue,mpClmemLeftData,CL_TRUE,CL_MAP_WRITE,0,
                               nBufferSize * sizeof(double),0,NULL,NULL,&clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clFinish(kEnv.mpkCmdQueue);
    dpRightData = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue,mpClmemRightData,CL_TRUE,CL_MAP_WRITE,0,
                                nBufferSize * sizeof(double),0,NULL,NULL,&clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    //printf("In CreateBuffer, pptrr is %d,%d,%d\n",fpSrcData,npStartPos,npEndPos);
    return 0;
}

int OclCalc::mapAndCopy64Bits(const double *dpTempSrcData,unsigned int *unStartPos,unsigned int *unEndPos,int nBufferSize ,int nRowsize)
{
    cl_int clStatus = 0;
    double * dpSrcDataMap = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemSrcData, CL_TRUE,CL_MAP_WRITE, 0,
                                          nBufferSize * sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    unsigned int *npStartPosMap = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemStartPos, CL_TRUE,CL_MAP_WRITE, 0,
                                              nRowsize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    unsigned int *npEndPosMap = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemEndPos, CL_TRUE, CL_MAP_WRITE, 0,
                                            nRowsize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for(int i=0;i<nBufferSize;i++)
        dpSrcDataMap[i] = dpTempSrcData[i];
    for(int i=0;i<nRowsize;i++)
    {
        npStartPosMap[i] = unStartPos[i];
        npEndPosMap[i] = unEndPos[i];
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemSrcData, dpSrcDataMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemStartPos, npStartPosMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemEndPos, npEndPosMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    return 1;
}
int OclCalc::mapAndCopy64Bits(const double *dpTempLeftData,const double *dpTempRightData,int nBufferSize )
{
    cl_int clStatus = 0;
    double *dpLeftDataMap = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemLeftData, CL_TRUE, CL_MAP_WRITE,
        0, nBufferSize * sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    double *dpRightDataMap = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemRightData, CL_TRUE, CL_MAP_WRITE,
        0, nBufferSize * sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < nBufferSize; i++ )
    {
        dpLeftDataMap[i] = dpTempLeftData[i];
        dpRightDataMap[i] = dpTempRightData[i];
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemLeftData, dpLeftDataMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemRightData, dpRightDataMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    return 1;
}

int OclCalc::mapAndCopyArithmetic64Bits( const double *dpMoreColArithmetic, int nBufferSize )
{
    cl_int clStatus = 0;
    double *dpLeftDataMap = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemLeftData, CL_TRUE, CL_MAP_WRITE,
        0, nBufferSize * sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < nBufferSize; i++ )
    {
        dpLeftDataMap[i] = dpMoreColArithmetic[i];
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemLeftData, dpLeftDataMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    return 1;
}
int OclCalc::mapAndCopyMoreColArithmetic64Bits( const double *dpMoreColArithmetic, int nBufferSize, uint *npeOp, uint neOpSize )
{
    cl_int clStatus = 0;
    double *dpLeftDataMap = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemLeftData, CL_TRUE, CL_MAP_WRITE,
        0, nBufferSize * sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    uint *dpeOpMap = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemeOp, CL_TRUE, CL_MAP_WRITE,
        0, neOpSize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < nBufferSize; i++ )
    {
        dpLeftDataMap[i] = dpMoreColArithmetic[i];
    }
    for( uint i = 0; i<neOpSize; i++)
    {
        dpeOpMap[i] = npeOp[i];
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemLeftData, dpLeftDataMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemeOp, dpeOpMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    return 1;
}

int OclCalc::createFormulaBuf64Bits( int nBufferSize, int rowSize )
{
    cl_int clStatus = 0;
    setKernelEnv( &kEnv );
    mpClmemSrcData   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        nBufferSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    nFormulaLen = nBufferSize;
    mpClmemStartPos   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        rowSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemEndPos   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        rowSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    return 0;
}

int OclCalc::createArithmeticOptBuf64Bits( int nBufferSize )
{
    cl_int clStatus = 0;
    nArithmeticLen = nBufferSize;
    setKernelEnv( &kEnv );
    mpClmemLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        nBufferSize * sizeof(double), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemRightData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        nBufferSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    return 0;
}

int OclCalc::createMoreColArithmeticBuf64Bits( int nBufferSize, int neOpSize )
{
    cl_int clStatus = 0;
    nArithmeticLen = nBufferSize;
    setKernelEnv( &kEnv );
    mpClmemLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        nBufferSize * sizeof(double), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemeOp = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        neOpSize * sizeof(uint), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    return 0;

}

int OclCalc::oclHostArithmeticOperator64Bits( const char* aKernelName, double *&rResult,int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    checkKernelName( &kEnv, aKernelName );

    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, aKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clFinish( kEnv.mpkCmdQueue );
    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE,
                          nRowSize * sizeof(double), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&mpClmemRightData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clResult );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );

    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
                   NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );

    double *dpOutPut = (double *) clEnqueueMapBuffer( kEnv.mpkCmdQueue, clResult, CL_TRUE,CL_MAP_READ,
                            0, nRowSize*sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    for ( int i = 0; i < nRowSize; i++ )
        rResult[i] = dpOutPut[i];
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clResult, rResult, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );

    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::oclMoreColHostArithmeticOperator64Bits( int nDataSize,int neOpSize,double *rResult, int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    const char *aKernelName = "oclMoreColArithmeticOperator";
    checkKernelName( &kEnv,aKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, aKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nRowSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_int), (void *)&nDataSize  );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&mpClmemeOp );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_int), (void *)&neOpSize );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 4, sizeof(cl_mem), (void *)&clResult );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    double * hostMapResult = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clResult, CL_TRUE, CL_MAP_READ, 0,
        nRowSize*sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < nRowSize; i++)
        rResult[i] = hostMapResult[i]; // from gpu float type to cpu double type
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clResult, hostMapResult, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel(kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;

}

int OclCalc::oclHostArithmeticStash64Bits( const char* aKernelName, const double *dpLeftData, const double *dpRightData, double *rResult,int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    setKernelEnv( &kEnv );
    checkKernelName( &kEnv, aKernelName );

    cl_mem clLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
    nRowSize * sizeof(double), (void *)dpLeftData, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clRightData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
        nRowSize * sizeof(double), (void *)dpRightData, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE,
                          nRowSize * sizeof(double), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, aKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clFinish( kEnv.mpkCmdQueue );

    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&clLeftData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&clRightData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clResult );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );

    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
                   NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );

    clStatus = clEnqueueReadBuffer(kEnv.mpkCmdQueue, clResult, CL_TRUE, 0, nRowSize * sizeof(double), (double *)rResult, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueReadBuffer" );

    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clLeftData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clRightData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::oclHostFormulaStash64Bits( const char* aKernelName, const double* dpSrcData, uint *nStartPos, uint *nEndPos, double *output, int nBufferSize, int size )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    setKernelEnv( &kEnv );
    checkKernelName( &kEnv, aKernelName );
    cl_mem clSrcData   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
        nBufferSize * sizeof(double), (void *)dpSrcData, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
     cl_mem clStartPos   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
        size * sizeof(unsigned int), (void *)nStartPos, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clEndPos   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
        size * sizeof(unsigned int), (void *)nEndPos, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    cl_mem outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, size * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&clSrcData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&clStartPos );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clEndPos );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_mem), (void *)&outputCl );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );

    clStatus = clEnqueueReadBuffer(kEnv.mpkCmdQueue, outputCl, CL_TRUE, 0, size * sizeof(double), (double *)output, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clReadBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( outputCl );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clSrcData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clStartPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clEndPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::oclHostFormulaStatistics64Bits( const char* aKernelName, double *&output, int size )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    checkKernelName( &kEnv, aKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    cl_mem outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, size * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&mpClmemSrcData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&mpClmemStartPos );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&mpClmemEndPos );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_mem), (void *)&outputCl );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    double *dpOutPut = (double *) clEnqueueMapBuffer( kEnv.mpkCmdQueue, outputCl, CL_TRUE,CL_MAP_READ,
                            0, size*sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    for ( int i = 0; i < size; i++ )
    {
        output[i] = dpOutPut[i];
    }

    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, outputCl, output, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( outputCl );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::oclHostFormulaCount64Bits( uint *npStartPos, uint *npEndPos, double *&dpOutput, int nSize )
{
    const char *cpKernelName = "oclFormulaCount";
    checkKernelName( &kEnv, cpKernelName );
    cl_int clStatus;
    size_t global_work_size[1];
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemStartPos, npStartPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemEndPos, npEndPos, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    cl_mem clpOutput = clCreateBuffer( kEnv.mpkContext,CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        nSize* sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&mpClmemStartPos );
    CHECK_OPENCL( clStatus,"clSetKernelArg");
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&mpClmemEndPos );
    CHECK_OPENCL( clStatus,"clSetKernelArg");
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clpOutput );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    dpOutput = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpOutput, CL_TRUE, CL_MAP_READ,
        0, nSize*sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpOutput, dpOutput, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpOutput );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

/*
 * The dpsrcData is double rows,one col is the first column data,second is the second column data.if get a cell data range,the range
 *save the npStart array eg:a4-a8;b10-b14,the npStart will store a4,b10,and the npEnd will store a8,b14 range.So it can if(i +1)%2 to judge
 * the a cloumn or b cloumn npStart range.so as b bolumn.
 */
int OclCalc::oclHostFormulaSumProduct64Bits( double *dpSumProMergeLfData, double *dpSumProMergeRrData, uint *npSumSize, double *&dpOutput, int nSize )
{
    cl_int clStatus;
    size_t global_work_size[1];
    memset(dpOutput,0,nSize);
    const char *cpFirstKernelName = "oclSignedMul";
    const char *cpSecondKernelName = "oclFormulaSumproduct";
    checkKernelName( &kEnv, cpFirstKernelName );
    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kEnv.mckKernelName,&clStatus);
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemMergeLfData, dpSumProMergeLfData, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish(kEnv.mpkCmdQueue);
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemMergeRtData, dpSumProMergeRrData, 0, NULL, NULL );
    clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemMatixSumSize, npSumSize, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    unsigned int nMulResultSize = nFormulaRowSize + nFormulaRowSize * nSize * nFormulaColSize - 1;
    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nMulResultSize * sizeof(double),
        NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&mpClmemMergeLfData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&mpClmemMergeRtData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clResult );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nMulResultSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clReleaseMemObject( mpClmemMergeLfData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemMergeRtData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    checkKernelName( &kEnv, cpSecondKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    cl_mem clpOutput = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nSize* sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_uint nMatixSize = nFormulaColSize * nFormulaRowSize;
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&clResult );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&mpClmemMatixSumSize );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&clpOutput );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_uint),
        (void *)&nMatixSize );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    double * outputMap = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpOutput, CL_TRUE, CL_MAP_READ,
        0, nSize*sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < nSize; i++ )
        dpOutput[i] = outputMap[i];
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpOutput, outputMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( mpClmemMatixSumSize );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpOutput );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::createMoreColArithmeticBuf32Bits( int nBufferSize, int neOpSize )
{
    cl_int clStatus = 0;
    nArithmeticLen = nBufferSize;
    setKernelEnv( &kEnv );
    mpClmemLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        nBufferSize * sizeof(float), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemeOp = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        neOpSize * sizeof(uint), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    return 0;
}

int OclCalc::createArithmeticOptBuf32Bits( int nBufferSize )
{
    cl_int clStatus = 0;
    setKernelEnv( &kEnv );
    nArithmeticLen = nBufferSize;
    mpClmemLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        nBufferSize * sizeof(float), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemRightData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        nBufferSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    return 0;
}

int OclCalc::createFormulaBuf32Bits( int nBufferSize, int rowSize )
{
    cl_int clStatus = 0;
    setKernelEnv( &kEnv );
    nFormulaLen = nBufferSize;

    mpClmemSrcData    = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        nBufferSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    mpClmemStartPos    = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        rowSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemEndPos    = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        rowSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    return 0;
}

int OclCalc::createBuffer32Bits( float *&fpLeftData, float *&fpRightData, int nBufferSize )
{
    cl_int clStatus = 0;
    setKernelEnv( &kEnv );
    mpClmemLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
            nBufferSize * sizeof(float), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemRightData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
        nBufferSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    fpLeftData = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemLeftData, CL_TRUE,CL_MAP_WRITE_INVALIDATE_REGION,
        0, nBufferSize * sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    fpRightData = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemRightData, CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION,
        0, nBufferSize * sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    //printf("In CreateBuffer, pptrr is %d,%d,%d\n",fpSrcData,npStartPos,npEndPos);
    return 0;
}

int OclCalc::mapAndCopy32Bits(const double *dpTempSrcData,unsigned int *unStartPos,unsigned int *unEndPos,int nBufferSize ,int nRowsize)
{
    cl_int clStatus = 0;
    float *fpSrcData = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemSrcData, CL_TRUE, CL_MAP_WRITE, 0,
            nBufferSize * sizeof(float) , 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    unsigned int *npStartPos = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemStartPos, CL_TRUE, CL_MAP_WRITE, 0,
            nRowsize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    unsigned int *npEndPos = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemEndPos, CL_TRUE, CL_MAP_WRITE, 0,
            nRowsize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for(int i=0;i<nBufferSize;i++)
    {
        fpSrcData[i] = (float)dpTempSrcData[i];
    }
    for(int i=0;i<nRowsize;i++)
    {
        npStartPos[i] = unStartPos[i];
        npEndPos[i] = unEndPos[i];
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemSrcData, fpSrcData, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemStartPos, npStartPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemEndPos, npEndPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    return 1;
}

int OclCalc::mapAndCopy32Bits(const double *dpTempLeftData,const double *dpTempRightData,int nBufferSize )
{
    cl_int clStatus = 0;
    float *fpLeftData = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemLeftData, CL_TRUE, CL_MAP_WRITE,
        0, nBufferSize * sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    float *fpRightData = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemRightData, CL_TRUE, CL_MAP_WRITE,
        0, nBufferSize * sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for(int i=0;i<nBufferSize;i++)
    {
        fpLeftData[i] = (float)dpTempLeftData[i];
        fpRightData[i] = (float)dpTempRightData[i];
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemLeftData, fpLeftData, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemRightData, fpRightData, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    return 1;
}
int OclCalc::mapAndCopyArithmetic32Bits( const double *dpMoreColArithmetic, int nBufferSize )
{
    cl_int clStatus = 0;
    float *dpLeftDataMap = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemLeftData, CL_TRUE, CL_MAP_WRITE,
        0, nBufferSize * sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < nBufferSize; i++ )
    {
        dpLeftDataMap[i] = (float)dpMoreColArithmetic[i];
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemLeftData, dpLeftDataMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    return 1;
}
int OclCalc::mapAndCopyMoreColArithmetic32Bits( const double *dpMoreColArithmetic, int nBufferSize, uint *npeOp, uint neOpSize )
{
    cl_int clStatus = 0;
    float *fpLeftDataMap = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemLeftData, CL_TRUE, CL_MAP_WRITE,
        0, nBufferSize * sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    uint *dpeOpMap = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemeOp, CL_TRUE, CL_MAP_WRITE,
        0, neOpSize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < nBufferSize; i++ )
    {
        fpLeftDataMap[i] = (float)dpMoreColArithmetic[i];
    }
    for( uint i = 0; i<neOpSize; i++ )
    {
        dpeOpMap[i] = npeOp[i];
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemLeftData, fpLeftDataMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemeOp, dpeOpMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    return 1;
}

int OclCalc::oclHostArithmeticOperator32Bits( const char* aKernelName,double *rResult, int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];

    checkKernelName( &kEnv,aKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, aKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );

    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nRowSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&mpClmemRightData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clResult );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    float * hostMapResult = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clResult, CL_TRUE, CL_MAP_READ, 0,
        nRowSize*sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < nRowSize; i++)
        rResult[i] = hostMapResult[i]; // from gpu float type to cpu double type
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clResult, hostMapResult, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel(kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::oclMoreColHostArithmeticOperator32Bits( int nDataSize,int neOpSize,double *rResult, int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    const char *aKernelName = "oclMoreColArithmeticOperator";
    checkKernelName( &kEnv,aKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, aKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nRowSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_int), (void *)&nDataSize  );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&mpClmemeOp );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_int), (void *)&neOpSize );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 4, sizeof(cl_mem), (void *)&clResult );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    float * hostMapResult = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clResult, CL_TRUE, CL_MAP_READ, 0,
        nRowSize*sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < nRowSize; i++)
        rResult[i] = hostMapResult[i]; // from gpu float type to cpu double type
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clResult, hostMapResult, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel(kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::oclHostFormulaStatistics32Bits(const char* aKernelName,double *output,int size)
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    checkKernelName( &kEnv, aKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );

    cl_mem outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, size * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&mpClmemSrcData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&mpClmemStartPos );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&mpClmemEndPos );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_mem), (void *)&outputCl );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    float * outputMap = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, outputCl, CL_TRUE, CL_MAP_READ,
        0, size*sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < size; i++ )
        output[i] = outputMap[i]; // from gpu float type to cpu double type
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, outputCl, outputMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( outputCl );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::oclHostArithmeticStash32Bits( const char* aKernelName, const double *dpLeftData, const double *dpRightData, double *rResult,int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    setKernelEnv( &kEnv );
    checkKernelName( &kEnv, aKernelName );
    float *fpLeftData = (float *)malloc( sizeof(float) * nRowSize );
    float *fpRightData = (float *)malloc( sizeof(float) * nRowSize );
    float *fpResult = (float *)malloc( sizeof(float) * nRowSize );
    for(int i=0;i<nRowSize;i++)
    {
        fpLeftData[i] = (float)dpLeftData[i];
        fpRightData[i] = (float)dpRightData[i];
    }
    cl_mem clLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
        nRowSize * sizeof(float), (void *)fpLeftData, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clRightData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
        nRowSize * sizeof(float), (void *)fpRightData, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE,
                          nRowSize * sizeof(float), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, aKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clFinish( kEnv.mpkCmdQueue );

    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&clLeftData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&clRightData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clResult );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );

    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
                   NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );

    clStatus = clEnqueueReadBuffer(kEnv.mpkCmdQueue, clResult, CL_TRUE, 0, nRowSize * sizeof(float), (float *)fpResult, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueReadBuffer" );
    for(int i=0;i<nRowSize;i++)
        rResult[i] = (double)fpResult[i];
    if(fpResult)
    {
        free(fpResult);
        fpResult = NULL;
    }
    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clLeftData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clRightData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::oclHostFormulaStash32Bits( const char* aKernelName, const double* dpSrcData, uint *nStartPos, uint *nEndPos, double *output, int nBufferSize, int size )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    setKernelEnv( &kEnv );
    checkKernelName( &kEnv, aKernelName );
    float *fpSrcData = (float *)malloc( sizeof(float) * nBufferSize );
    float *fpResult = (float *)malloc( sizeof(float) * size );
    for(int i=0;i<nBufferSize;i++)
        fpSrcData[i] = (float)dpSrcData[i];
    cl_mem clSrcData   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_COPY_HOST_PTR),
        nBufferSize * sizeof(float), (void *)fpSrcData, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
     cl_mem clStartPos   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_COPY_HOST_PTR),
        size * sizeof(unsigned int), (void *)nStartPos, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clEndPos   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_COPY_HOST_PTR),
        size * sizeof(unsigned int), (void *)nEndPos, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    cl_mem outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, size * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&clSrcData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&clStartPos );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clEndPos );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_mem), (void *)&outputCl );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );

    clStatus = clEnqueueReadBuffer(kEnv.mpkCmdQueue, outputCl, CL_TRUE, 0, size * sizeof(float), (double *)fpResult, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clReadBuffer" );
    for(int i = 0;i<size;i++)
        output[i] = (float)fpResult[i];
    clFinish( kEnv.mpkCmdQueue );
    if(fpResult)
    {
        free(fpResult);
    fpResult = NULL;
    }
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( outputCl );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clSrcData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clStartPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clEndPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::oclHostFormulaCount32Bits( uint *npStartPos, uint *npEndPos, double *dpOutput, int nSize )
{
    const char *cpKernelName = "oclFormulaCount";
    checkKernelName( &kEnv, cpKernelName );
    cl_int clStatus;
    size_t global_work_size[1];
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemStartPos, npStartPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemEndPos, npEndPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    cl_mem clpOutput = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        nSize* sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&mpClmemStartPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&mpClmemEndPos );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clpOutput );
    CHECK_OPENCL(clStatus, "clSetKernelArg");
    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    float * fpOutputMap = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpOutput, CL_TRUE,
        CL_MAP_READ, 0, nSize*sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for (int i = 0; i < nSize; i++ )
        dpOutput[i] = fpOutputMap[i];// from gpu float type to cpu double type
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpOutput, fpOutputMap, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel(kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject(mpClmemSrcData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemStartPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemEndPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clpOutput );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

//sumproduct
int OclCalc::oclHostFormulaSumProduct32Bits( float *fpSumProMergeLfData, float *fpSumProMergeRrData, uint *npSumSize, double *dpOutput, int nSize )
{
    cl_int clStatus;
    size_t global_work_size[1];
    memset(dpOutput,0,nSize);
    const char *cpFirstKernelName = "oclSignedMul";
    const char *cpSecondKernelName = "oclFormulaSumproduct";
    checkKernelName( &kEnv, cpFirstKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemMergeLfData, fpSumProMergeLfData, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemMergeRtData, fpSumProMergeRrData, 0, NULL, NULL );
    clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemMatixSumSize, npSumSize, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    unsigned int nMulResultSize = nFormulaRowSize +  nFormulaRowSize * nSize * nFormulaColSize - 1;
    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nMulResultSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&mpClmemMergeLfData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&mpClmemMergeRtData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clResult );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nMulResultSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clReleaseMemObject( mpClmemMergeLfData );
    CHECK_OPENCL( clStatus,"clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemMergeRtData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    checkKernelName( &kEnv,cpSecondKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    cl_mem clpOutput = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nSize* sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_uint nMatixSize = nFormulaColSize * nFormulaRowSize;
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&clResult );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&mpClmemMatixSumSize );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&clpOutput );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_uint), (void *)&nMatixSize );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    float * fpOutputMap = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpOutput, CL_TRUE, CL_MAP_READ, 0,
        nSize*sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for (int i = 0; i < nSize; i++ )
    {
        dpOutput[i] = fpOutputMap[i]; // from gpu float type to cpu double type
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpOutput, fpOutputMap, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );

    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( mpClmemMatixSumSize );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpOutput );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    return 0;
}


// FIXME: should be templatised in <double> - double buffering [sic] rocks
static cl_mem allocateDoubleBuffer( KernelEnv &rEnv, const double *_pValues, size_t nElements, cl_int *pStatus )
{
    // Ugh - horrible redundant copying ...
    cl_mem xValues = clCreateBuffer( rEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
        nElements * sizeof(double), NULL, pStatus);
    double *pValues = (double *)clEnqueueMapBuffer( rEnv.mpkCmdQueue, xValues, CL_TRUE, CL_MAP_WRITE, 0,
        nElements * sizeof(double), 0, NULL, NULL, NULL);
    clFinish(rEnv.mpkCmdQueue);
    for ( int i = 0; i < (int)nElements; i++ )
        pValues[i] = _pValues[i];
    clEnqueueUnmapMemObject( rEnv.mpkCmdQueue, xValues, pValues, 0, NULL, NULL );
    clFinish( rEnv.mpkCmdQueue );
    return xValues;
}

static cl_mem allocateFloatBuffer( KernelEnv &rEnv, const double *_pValues, size_t nElements, cl_int *pStatus )
{
    // Ugh - horrible redundant copying ...
    cl_mem xValues = clCreateBuffer( rEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
        nElements * sizeof(float), NULL, pStatus);
    float *pValues = (float *)clEnqueueMapBuffer( rEnv.mpkCmdQueue, xValues, CL_TRUE, CL_MAP_WRITE, 0,
        nElements * sizeof(float), 0, NULL, NULL, NULL );
    clFinish( rEnv.mpkCmdQueue );
    for ( int i = 0; i < (int)nElements; i++ )
        pValues[i] = (float)_pValues[i];

    clEnqueueUnmapMemObject(rEnv.mpkCmdQueue,xValues,pValues,0,NULL,NULL);
    clFinish( rEnv.mpkCmdQueue );
    return xValues;
}

int OclCalc::oclGroundWaterGroup( uint *eOp, uint eOpNum, const double *pOpArray, const double *pSubtractSingle, size_t nSrcDataSize,size_t nElements, double del ,uint *nStartPos,uint *nEndPos,double *dpResult)
{
    setKernelEnv( &kEnv );

    char kernelName[256] = "";
    double delta = del;
    bool subFlag = false;
    strcat(kernelName,"ocl");
    for ( size_t i = 0; i < eOpNum; i++ )
    {
        switch ( eOp[i] )
        {
        case ocAdd:
            strcat(kernelName,"Add");
            break;
        case ocSub:
            strcat(kernelName,"Sub");
            break;
        case ocMul:
            strcat(kernelName,"Mul");
            break;
        case ocDiv:
            strcat(kernelName,"Div");
            break;
        case ocMax:
            strcat(kernelName,"Max");
            break;
        case ocMin:
            strcat(kernelName,"Min");
            break;
        case ocAverage:
            strcat(kernelName,"Average");
            break;
        default:
            assert( false );
            break;
        }
    }
    checkKernelName( &kEnv, kernelName );
    cl_int clStatus;
    size_t global_work_size[1];
    if ( ( eOpNum == 1 ) && ( eOp[0] == ocSub ) )
        subFlag = true;

    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );

    cl_mem valuesCl = NULL, subtractCl = NULL, outputCl = NULL, startPosCL = NULL, endPosCL = NULL;

    if(!subFlag)
    {
        startPosCL = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
            nElements * sizeof(unsigned int), NULL, &clStatus );
        CHECK_OPENCL( clStatus, "clCreateBuffer" );
        endPosCL = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
            nElements * sizeof(unsigned int), NULL, &clStatus );
        CHECK_OPENCL( clStatus, "clCreateBuffer" );
        unsigned int *npStartPosMap = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, startPosCL, CL_TRUE, CL_MAP_WRITE, 0,
            nElements * sizeof(uint), 0, NULL, NULL, &clStatus );
        CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
        clFinish( kEnv.mpkCmdQueue );
        unsigned int *npEndPosMap = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, endPosCL, CL_TRUE, CL_MAP_WRITE, 0,
            nElements * sizeof(uint), 0, NULL, NULL, &clStatus );
        CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
        clFinish( kEnv.mpkCmdQueue );

        for(uint i=0;i<nElements;i++)
        {
            npStartPosMap[i]=nStartPos[i];
            npEndPosMap[i]=nEndPos[i];
        }
        clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, startPosCL, npStartPosMap, 0, NULL, NULL );
        CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
        clFinish( kEnv.mpkCmdQueue );
        clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, endPosCL, npEndPosMap, 0, NULL, NULL );
        CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );

        if ( gpuEnv.mnKhrFp64Flag || gpuEnv.mnAmdFp64Flag )
        {
            valuesCl = allocateDoubleBuffer( kEnv, pOpArray, nSrcDataSize, &clStatus );
            subtractCl = allocateDoubleBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
            outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nElements * sizeof(double), NULL, &clStatus );
        }
        else
        {
            valuesCl = allocateFloatBuffer( kEnv, pOpArray, nSrcDataSize, &clStatus );
            subtractCl = allocateFloatBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
            outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE , nElements * sizeof(float), NULL, &clStatus);
        }
        CHECK_OPENCL( clStatus, "clCreateBuffer" );

        clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&valuesCl );
        CHECK_OPENCL( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&subtractCl );
        CHECK_OPENCL( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&startPosCL );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_mem), (void *)&endPosCL );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg( kEnv.mpkKernel, 4, sizeof(cl_mem), (void *)&outputCl );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );

        fprintf( stderr, "prior to enqueue range kernel\n" );
    }
    else
    {
        if ( gpuEnv.mnKhrFp64Flag || gpuEnv.mnAmdFp64Flag )
       {
             subtractCl = allocateDoubleBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
             outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nElements * sizeof(double), NULL, &clStatus );
             clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_double), (void *)&delta );
             CHECK_OPENCL( clStatus, "clSetKernelArg");
        }
        else
       {
             float fTmp = (float)delta;
             subtractCl = allocateFloatBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
             outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nElements * sizeof(float), NULL, &clStatus );
             clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_float), (void *)&fTmp );
             CHECK_OPENCL( clStatus, "clSetKernelArg");
        }
        clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&subtractCl );
        CHECK_OPENCL( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&outputCl );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
    }
    global_work_size[0] = nElements;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );

    if ( gpuEnv.mnKhrFp64Flag || gpuEnv.mnAmdFp64Flag )
    {
        clStatus = clEnqueueReadBuffer(kEnv.mpkCmdQueue,
                                       outputCl,
                                       CL_TRUE,0,
                                       nElements * sizeof(double),
                                       (void *)dpResult,0,NULL,NULL);
        CHECK_OPENCL( clStatus, "clEnqueueReadBuffer" );
        clFinish( kEnv.mpkCmdQueue );
    }
    else
    {
        float *afBuffer = new float[nElements];
        if ( !afBuffer )
            return -1;
        clStatus = clEnqueueReadBuffer(kEnv.mpkCmdQueue,
                                       outputCl,
                                       CL_TRUE,0,
                                       nElements * sizeof(float),
                                       (void *)afBuffer,0,NULL,NULL);
        CHECK_OPENCL( clStatus, "clEnqueueReadBuffer" );
        clFinish( kEnv.mpkCmdQueue );
        for ( size_t i = 0; i < nElements; i++ )
        {
            dpResult[i] = (double)afBuffer[i];
        }
        if ( !afBuffer )
            delete [] afBuffer;
    }

    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );

    CHECK_OPENCL_RELEASE( clStatus, valuesCl );
    CHECK_OPENCL_RELEASE( clStatus, subtractCl );
    CHECK_OPENCL_RELEASE( clStatus, outputCl );
    CHECK_OPENCL_RELEASE( clStatus, startPosCL );
    CHECK_OPENCL_RELEASE( clStatus, endPosCL );

    fprintf( stderr, "completed opencl operation\n" );

    return 0;
}
double *OclCalc::oclSimpleDeltaOperation( OpCode eOp, const double *pOpArray, const double *pSubtractSingle, size_t nElements, double del )
{
    setKernelEnv( &kEnv );

    // select a kernel: cut & paste coding is utterly evil.
    const char *kernelName = NULL;
    double delta = del;
    bool subFlag = false;
    switch ( eOp ) {
    case ocAdd:
       fprintf( stderr, "ocSub is %d\n", ocSub );
    case ocMul:
    case ocDiv:
        ; // FIXME: fallthrough for now
    case ocMax:
        kernelName = "oclMaxDelta";
        break;
    case ocMin:
        kernelName = "oclMinDelta";
        break;
    case ocAverage:
        kernelName = "oclAverageDelta";
        break;
    case ocSub:
        kernelName = "oclSubDelta";
        subFlag = true;
        break;
    default:
        assert( false );
    }
    checkKernelName( &kEnv, kernelName );

    cl_int clStatus;
    size_t global_work_size[1];

    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kernelName, &clStatus );
    if ( !kEnv.mpkKernel )
    {
        fprintf( stderr, "\n\n*** Error: Could not clCreateKernel '%s' ***\n\n", kernelName );
        fprintf( stderr, "\tprobably your binary cache is out of date\n"
                "\tplease delete kernel-*.bin in your cwd\n\n\n" );
        return NULL;
    }
    CHECK_OPENCL( clStatus, "clCreateKernel" );

    // Ugh - horrible redundant copying ...

    cl_mem valuesCl = NULL, subtractCl = NULL, outputCl = NULL;
    if(!subFlag)
    {
        if ( gpuEnv.mnKhrFp64Flag || gpuEnv.mnAmdFp64Flag )
        {
            valuesCl = allocateDoubleBuffer( kEnv, pOpArray, nElements, &clStatus );
            subtractCl = allocateDoubleBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
            outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nElements * sizeof(double), NULL, &clStatus );
        }
        else
        {
            valuesCl = allocateFloatBuffer( kEnv, pOpArray, nElements, &clStatus );
            subtractCl = allocateFloatBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
            outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nElements * sizeof(float), NULL, &clStatus);
        }
        CHECK_OPENCL( clStatus, "clCreateBuffer" );

        cl_uint start = 0;
        cl_uint end = (cl_uint)nElements;

        clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&valuesCl );
        CHECK_OPENCL( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&subtractCl );
        CHECK_OPENCL( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_uint), (void *)&start );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_uint), (void *)&end );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg( kEnv.mpkKernel, 4, sizeof(cl_mem), (void *)&outputCl );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );

        fprintf( stderr, "prior to enqueue range kernel\n" );
    }
    else
    {
        if ( gpuEnv.mnKhrFp64Flag || gpuEnv.mnAmdFp64Flag )
       {
             subtractCl = allocateDoubleBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
             outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nElements * sizeof(double), NULL, &clStatus );
             clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_double), (void *)&delta );
             CHECK_OPENCL( clStatus, "clSetKernelArg");
        }
        else
       {
             float fTmp = (float)delta;
             subtractCl = allocateFloatBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
             outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nElements * sizeof(float), NULL, &clStatus );
             clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_float), (void *)&fTmp );
             CHECK_OPENCL( clStatus, "clSetKernelArg");
        }
        clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&subtractCl );
        CHECK_OPENCL( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&outputCl );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
    }
    global_work_size[0] = nElements;
    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );

    double *pResult = new double[nElements];
    if ( !pResult )
        return NULL; // leak.
    if ( gpuEnv.mnKhrFp64Flag || gpuEnv.mnAmdFp64Flag )
    {
        clStatus = clEnqueueReadBuffer(kEnv.mpkCmdQueue,
                                       outputCl,
                                       CL_TRUE,0,
                                       nElements * sizeof(double),
                                       (void *)pResult,0,NULL,NULL);
    }
    else
    {
        float *afBuffer = new float[nElements];
        if ( !afBuffer )
            return NULL;
        clStatus = clEnqueueReadBuffer(kEnv.mpkCmdQueue,
                                       outputCl,
                                       CL_TRUE,0,
                                       nElements * sizeof(float),
                                       (void *)afBuffer,0,NULL,NULL);
        for ( int i = 0; i < (int)nElements; i++ )
            pResult[i] = (double)afBuffer[i];
        if ( !afBuffer )
            delete [] afBuffer;
    }
    CHECK_OPENCL( clStatus, "clEnqueueReadBuffer" );

    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );

    if ( valuesCl != NULL )
    {
        clStatus = clReleaseMemObject( valuesCl );
        CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    }
    if ( subtractCl != NULL )
    {
        clStatus = clReleaseMemObject( subtractCl );
        CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    }
    if ( outputCl != NULL )
    {
        clStatus = clReleaseMemObject( outputCl );
        CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    }
    fprintf( stderr, "completed opencl delta operation\n" );

    return pResult;
}

int OclCalc::oclHostMatrixInverse64Bits( const char* aKernelName, double *dpOclMatrixSrc, double *dpOclMatrixDst,std::vector<double>&dpResult,  uint nDim )
{
    cl_int clStatus = 0;
    uint nMatrixSize = nDim * nDim;
    size_t global_work_size[1] = { nDim };
    cl_mem clpPData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR ), nMatrixSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clpYData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR ), nMatrixSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clpNData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE ), nDim * sizeof(uint), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    double * dpY = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpYData, CL_TRUE, CL_MAP_WRITE, 0, nMatrixSize * sizeof(double), 0, NULL,NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    memset( dpY, 0, nMatrixSize*sizeof(double) );
    memset( dpOclMatrixDst, 0, nMatrixSize*sizeof(double) );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpYData, dpY, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    double * dpP = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpPData, CL_TRUE, CL_MAP_WRITE, 0, nMatrixSize * sizeof(double), 0, NULL,NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    for (uint i=0;i<nDim;i++)
    {
        for (uint j=0;j<nDim;j++)
        {
            if ( i == j )
                dpP[i*nDim+j] = 1.0;
            else
                dpP[i*nDim+j] = 0.0;
        }
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpPData, dpP, 0, NULL, NULL );
    uint * npDim = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpNData, CL_TRUE, CL_MAP_WRITE, 0, nDim * sizeof(uint), 0, NULL,NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    for ( uint i = 0; i < nDim; i++ )
        npDim[i] = nDim;
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpNData, npDim, 0, NULL, NULL );

    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    checkKernelName( &kEnv,aKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, aKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&clpPData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    for ( uint nOffset = 0; nOffset < nDim- 1; nOffset++ )
    {
        int nMax = nOffset;
        for ( uint i = nOffset + 1; i < nDim; i++ )
        {
            if( fabs(dpOclMatrixSrc[nMax*nDim+nOffset]) < fabs(dpOclMatrixSrc[i*nDim+nOffset]))
                nMax=i;
        }
        clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&nOffset );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_mem), (void *)&nMax );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
        clFinish( kEnv.mpkCmdQueue );
        for ( uint i = nOffset + 1; i < nDim; i++ )
        {
            dpOclMatrixSrc[i*nDim+nOffset] = dpOclMatrixSrc[i*nDim+nOffset] / dpOclMatrixSrc[nOffset*nDim+nOffset];
            for ( uint j = nOffset+ 1; j < nDim; j++ )
                dpOclMatrixSrc[i*nDim+j] = dpOclMatrixSrc[i*nDim+j] - dpOclMatrixSrc[nOffset*nDim+j] * dpOclMatrixSrc[i*nDim+nOffset];
        }
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemLeftData, dpOclMatrixSrc, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    cl_kernel kernel_solve = clCreateKernel( kEnv.mpkProgram, "oclMatrixSolve", &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clSetKernelArg( kernel_solve, 0, sizeof(cl_mem), (void *)&mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kernel_solve, 1, sizeof(cl_mem), (void *)&mpClmemRightData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kernel_solve, 2, sizeof(cl_mem), (void *)&clpPData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kernel_solve, 3, sizeof(cl_mem), (void *)&clpYData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kernel_solve, 4, sizeof(cl_mem), (void *)&clpNData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );

    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kernel_solve, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    for ( uint i = 0; i < nDim; i++ )
        for ( uint j = 0; j < nDim; j++ )
            dpResult[i*nDim+j] = dpOclMatrixDst[j*nDim+i];
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemRightData, dpOclMatrixDst, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    mpClmemLeftData = NULL;
    clStatus = clReleaseMemObject( mpClmemRightData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    mpClmemRightData = NULL;
    clStatus = clReleaseKernel( kernel_solve );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpPData );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpYData );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpNData );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );

    return 0;
}

int OclCalc::oclHostMatrixInverse32Bits( const char* aKernelName, float *fpOclMatrixSrc, float *fpOclMatrixDst, std::vector<double>& dpResult, uint nDim )
{
    cl_int clStatus = 0;
    uint nMatrixSize = nDim * nDim;
    size_t global_work_size[1] = { nDim };

    cl_mem clpPData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR ), nMatrixSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clpYData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR ), nMatrixSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clpNData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE ), nDim * sizeof(uint), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    float * fpY = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpYData, CL_TRUE, CL_MAP_WRITE, 0, nMatrixSize * sizeof(float), 0, NULL,NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    memset( fpY, 0, nMatrixSize*sizeof(float) );
    memset( fpOclMatrixDst, 0, nMatrixSize*sizeof(float) );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpYData, fpY, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    float * fpP = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpPData, CL_TRUE, CL_MAP_WRITE, 0, nMatrixSize * sizeof(float), 0, NULL,NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    for ( uint i = 0;i < nDim; i++ )
    {
        for ( uint j = 0;j < nDim; j++ )
        {
            if( i == j )
                fpP[i*nDim+j]=1.0f;
            else
                fpP[i*nDim+j]=0.0f;
        }
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpPData, fpP, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    uint * npDim = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpNData, CL_TRUE, CL_MAP_WRITE, 0, nDim * sizeof(uint), 0, NULL,NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    for ( uint i = 0; i < nDim; i++ )
        npDim[i] = nDim;
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpNData, npDim, 0, NULL, NULL );
    checkKernelName( &kEnv,aKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, aKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kEnv.mpkKernel, 1, sizeof(cl_mem), (void *)&clpPData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );

    for ( uint nOffset = 0; nOffset < nDim- 1; nOffset++ )
    {
        int nMax = nOffset;
        for( uint i = nOffset+1; i < nDim; i++ )
        {
            if( fabs(fpOclMatrixSrc[nMax*nDim+nOffset]) < fabs(fpOclMatrixSrc[i*nDim+nOffset]))
                nMax=i;
        }
        clStatus = clSetKernelArg( kEnv.mpkKernel, 2, sizeof(cl_mem), (void *)&nOffset );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg( kEnv.mpkKernel, 3, sizeof(cl_mem), (void *)&nMax );
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kEnv.mpkKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
        clFinish( kEnv.mpkCmdQueue );

        for ( uint i= nOffset + 1; i < nDim; i++ )
        {
            fpOclMatrixSrc[i*nDim+nOffset] = fpOclMatrixSrc[i*nDim+nOffset] / fpOclMatrixSrc[nOffset*nDim+nOffset];
            for ( uint j= nOffset + 1; j < nDim; j++ )
                fpOclMatrixSrc[i*nDim+j] = fpOclMatrixSrc[i*nDim+j] - fpOclMatrixSrc[nOffset*nDim+j] * fpOclMatrixSrc[i*nDim+nOffset];
        }
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemLeftData, fpOclMatrixSrc, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );

    cl_kernel kernel_solve = clCreateKernel( kEnv.mpkProgram, "oclMatrixSolve", &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clSetKernelArg( kernel_solve, 0, sizeof(cl_mem), (void *)&mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kernel_solve, 1, sizeof(cl_mem), (void *)&mpClmemRightData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kernel_solve, 2, sizeof(cl_mem), (void *)&clpPData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kernel_solve, 3, sizeof(cl_mem), (void *)&clpYData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg( kernel_solve, 4, sizeof(cl_mem), (void *)&clpNData );
    CHECK_OPENCL( clStatus, "clSetKernelArg" );

    clStatus = clEnqueueNDRangeKernel( kEnv.mpkCmdQueue, kernel_solve, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    for ( uint i = 0; i < nDim; i++ )
        for ( uint j = 0; j < nDim; j++ )
            dpResult[i*nDim+j] = fpOclMatrixDst[j*nDim+i]; // from gpu float type to cpu double type
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemRightData, fpOclMatrixDst, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    mpClmemLeftData = NULL;
    clStatus = clReleaseMemObject( mpClmemRightData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    mpClmemRightData = NULL;
    clStatus = clReleaseKernel( kernel_solve );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpPData );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpYData );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpNData );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    return 0;
}

namespace {

void createDeviceInfo(cl_device_id aDeviceId, OpenclPlatformInfo& rPlatformInfo)
{
    OpenclDeviceInfo aDeviceInfo;
    aDeviceInfo.device = aDeviceId;

    char pName[64];
    cl_int nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_NAME, 64, pName, NULL);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.maName = OUString::createFromAscii(pName);

    char pVendor[64];
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_VENDOR, 64, pName, NULL);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.maVendor = OUString::createFromAscii(pVendor);

    cl_ulong nMemSize;
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(nMemSize), &nMemSize, NULL);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.mnMemory = nMemSize;

    cl_uint nClockFrequency;
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(nClockFrequency), &nClockFrequency, NULL);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.mnFrequency = nClockFrequency;

    cl_uint nComputeUnits;
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(nComputeUnits), &nComputeUnits, NULL);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.mnComputeUnits = nComputeUnits;

    rPlatformInfo.maDevices.push_back(aDeviceInfo);
}

bool createPlatformInfo(cl_platform_id nPlatformId, OpenclPlatformInfo& rPlatformInfo)
{
    rPlatformInfo.platform = nPlatformId;
    char pName[64];
    cl_int nState = clGetPlatformInfo(nPlatformId, CL_PLATFORM_NAME, 64,
             pName, NULL);
    if(nState != CL_SUCCESS)
        return false;
    rPlatformInfo.maName = OUString::createFromAscii(pName);

    char pVendor[64];
    nState = clGetPlatformInfo(nPlatformId, CL_PLATFORM_VENDOR, 64,
             pVendor, NULL);
    if(nState != CL_SUCCESS)
        return false;

    rPlatformInfo.maVendor = OUString::createFromAscii(pName);

    cl_uint nDevices;
    nState = clGetDeviceIDs(nPlatformId, CL_DEVICE_TYPE_ALL, 0, NULL, &nDevices);
    if(nState != CL_SUCCESS)
        return false;

    // memory leak that does not matter
    // memory is stored in static variable that lives through the whole program
    cl_device_id* pDevices = new cl_device_id[nDevices];
    nState = clGetDeviceIDs(nPlatformId, CL_DEVICE_TYPE_ALL, nDevices, pDevices, NULL);
    if(nState != CL_SUCCESS)
        return false;

    for(size_t i = 0; i < nDevices; ++i)
    {
        createDeviceInfo(pDevices[i], rPlatformInfo);
    }

    return true;
}

}

size_t getOpenCLPlatformCount()
{
    int status = clewInit(OPENCL_DLL_NAME);
    if (status < 0)
        return 0;

    cl_uint nPlatforms;
    cl_int nState = clGetPlatformIDs(0, NULL, &nPlatforms);

    if (nState != CL_SUCCESS)
        return 0;

    return nPlatforms;
}

const std::vector<OpenclPlatformInfo>& fillOpenCLInfo()
{
    static std::vector<OpenclPlatformInfo> aPlatforms;
    if(!aPlatforms.empty())
        return aPlatforms;

    int status = clewInit(OPENCL_DLL_NAME);
    if (status < 0)
        return aPlatforms;

    cl_uint nPlatforms;
    cl_int nState = clGetPlatformIDs(0, NULL, &nPlatforms);

    if(nState != CL_SUCCESS)
        return aPlatforms;

    // memory leak that does not matter,
    // memory is stored in static instance aPlatforms
    cl_platform_id* pPlatforms = new cl_platform_id[nPlatforms];
    nState = clGetPlatformIDs(nPlatforms, pPlatforms, NULL);

    if(nState != CL_SUCCESS)
        return aPlatforms;

    for(size_t i = 0; i < nPlatforms; ++i)
    {
        OpenclPlatformInfo aPlatformInfo;
        if(createPlatformInfo(pPlatforms[i], aPlatformInfo))
            aPlatforms.push_back(aPlatformInfo);
    }

    return aPlatforms;
}

void switchOpenclDevice(void* pDevice, bool bAutoSelect)
{
    cl_device_id pDeviceId = reinterpret_cast<cl_device_id>(pDevice);
    if(!pDeviceId || bAutoSelect)
    {
        size_t nComputeUnits = 0;
        // clever algorithm
        const std::vector<OpenclPlatformInfo>& rPlatform = fillOpenCLInfo();
        for(std::vector<OpenclPlatformInfo>::const_iterator it =
                rPlatform.begin(), itEnd = rPlatform.end(); it != itEnd; ++it)
        {
            for(std::vector<OpenclDeviceInfo>::const_iterator itr =
                    it->maDevices.begin(), itrEnd = it->maDevices.end();
                    itr != itrEnd; ++itr)
            {
                if(itr->mnComputeUnits > nComputeUnits)
                {
                    pDeviceId = reinterpret_cast<cl_device_id>(itr->device);
                    nComputeUnits = itr->mnComputeUnits;
                }
            }
        }
    }

    if(OpenclDevice::gpuEnv.mpDevID == pDeviceId)
    {
        // we don't need to change anything
        // still the same device
        return;
    }

    cl_platform_id platformId;
    cl_int nState = clGetDeviceInfo(pDeviceId, CL_DEVICE_PLATFORM,
            sizeof(platformId), &platformId, NULL);

    cl_context_properties cps[3];
    cps[0] = CL_CONTEXT_PLATFORM;
    cps[1] = (cl_context_properties) platformId;
    cps[2] = 0;
    cl_context context = clCreateContext( cps, 1, &pDeviceId, NULL, NULL, &nState );

    if(nState != CL_SUCCESS || context == NULL)
    {
        if(context != NULL)
            clReleaseContext(context);

        SAL_WARN("sc", "failed to set/switch opencl device");
        return;
    }

    cl_command_queue command_queue = clCreateCommandQueue(
            context, pDeviceId, 0, &nState);

    if(command_queue == NULL || nState != CL_SUCCESS)
    {
        if(command_queue != NULL)
            clReleaseCommandQueue(command_queue);

        clReleaseContext(context);
    }

    OpenclDevice::releaseOpenclEnv(&OpenclDevice::gpuEnv);
    OpenCLEnv env;
    env.mpOclPlatformID = platformId;
    env.mpOclContext = context;
    env.mpOclDevsID = pDeviceId;
    env.mpOclCmdQueue = command_queue;
    OpenclDevice::initOpenclAttr(&env);
    OpenclDevice::initOpenclRunEnv(&OpenclDevice::gpuEnv);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
