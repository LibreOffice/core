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
#include <rtl/strbuf.hxx>
#include <rtl/digest.h>
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

#define DEVICE_NAME_LENGTH 1024

using namespace std;

namespace sc { namespace opencl {

Kernel::Kernel( const char* pName ) : mpName(pName), mpKernel(NULL) {}

GPUEnv OpenclDevice::gpuEnv;
int OpenclDevice::isInited =0;

namespace {

OString generateHashForSource()
{
    size_t nLength = strlen(kernel_src);
    sal_uInt8 pBuffer[RTL_DIGEST_LENGTH_MD5];
    rtlDigestError aError = rtl_digest_MD5(kernel_src, nLength,
            pBuffer, RTL_DIGEST_LENGTH_MD5);
    assert(aError == rtl_Digest_E_None);

    OStringBuffer aBuffer;
    const char* pString = "0123456789ABCDEF";
    for(size_t i = 0; i < RTL_DIGEST_LENGTH_MD5; ++i)
    {
        sal_uInt8 val = pBuffer[i];
        aBuffer.append(pString[val/16]);
        aBuffer.append(pString[val%16]);
    }
    return aBuffer.makeStringAndClear();
}

}

OString OpenclDevice::maSourceHash = generateHashForSource();

int OpenclDevice::releaseOpenclRunEnv()
{
    releaseOpenclEnv( &gpuEnv );

    return 1;
}

namespace {

const char* pKernelNames[] = {

    "oclFormulaMin",
    "oclFormulaMax",
    "oclFormulaSum",
    "oclFormulaCount",
    "oclFormulaAverage",
    "oclFormulaSumproduct",
    "oclFormulaMtxInv",

    "oclSignedAdd",
    "oclSignedSub",
    "oclSignedMul",
    "oclSignedDiv",
    "oclAverageDelta",
    "oclMaxDelta",
    "oclMinDelta",
    "oclSubDelta",
    "oclLUDecomposition",
    "oclAverageDeltaRPN",
    "oclMaxDeltaRPN",
    "oclMinDeltaRPN",
    "oclMoreColArithmeticOperator",
    "oclColumnH",
    "oclColumnL",
    "oclColumnN",
    "oclColumnJ",
    "oclMaxSub",
    "oclAverageSub",
    "oclMinSub",
    "oclMaxAdd",
    "oclAverageAdd",
    "oclMinAdd",
    "oclMaxMul",
    "oclAverageMul"
    "oclMinMul",
    "oclMaxDiv",
    "oclAverageDiv"
    "oclMinDiv",
    "oclSub"
};

}

int OpenclDevice::registOpenclKernel()
{
    if ( !gpuEnv.mnIsUserCreated )
        memset( &gpuEnv, 0, sizeof(gpuEnv) );

    gpuEnv.mnFileCount = 0; //argc;

    for (size_t i = 0, n = SAL_N_ELEMENTS(pKernelNames); i < n; ++i)
        gpuEnv.maKernels.push_back(Kernel(pKernelNames[i]));

    return 0;
}

int OpenclDevice::setKernelEnv( KernelEnv *envInfo )
{
    envInfo->mpkContext = gpuEnv.mpContext;
    envInfo->mpkCmdQueue = gpuEnv.mpCmdQueue;
    envInfo->mpkProgram = gpuEnv.mpArryPrograms[0];

    return 1;
}

Kernel* OpenclDevice::fetchKernel( const char *kernelName )
{
    cl_int nStatus;
    for (size_t i = 0, n = gpuEnv.maKernels.size(); i < n; ++i)
    {
        Kernel* pKernel = &gpuEnv.maKernels[i];
        if (!strcasecmp(kernelName, pKernel->mpName))
        {
            printf("found the kernel named %s.\n", kernelName);
            if (!pKernel->mpKernel && gpuEnv.mpArryPrograms[0])
            {
                pKernel->mpKernel = clCreateKernel(gpuEnv.mpArryPrograms[0], kernelName, &nStatus);
                if (nStatus != CL_SUCCESS)
                    pKernel->mpKernel = NULL;

                printf("Kernel named '%s' has been compiled\n", kernelName);
            }

            return pKernel->mpKernel ?  pKernel : NULL;
        }
    }

    printf("No kernel named %s found.\n", kernelName);
    return NULL;
}

namespace {

OString createFileName(cl_device_id deviceId, const char* clFileName)
{
    OString fileName(clFileName);
    sal_Int32 nIndex = fileName.lastIndexOf(".cl");
    if(nIndex > 0)
        fileName = fileName.copy(0, nIndex);

    char deviceName[DEVICE_NAME_LENGTH] = {0};
    clGetDeviceInfo(deviceId, CL_DEVICE_NAME,
            sizeof(deviceName), deviceName, NULL);
    return fileName + "-" + deviceName + "-" + OpenclDevice::maSourceHash + ".bin";
}

}

int OpenclDevice::binaryGenerated( const char * clFileName, FILE ** fhandle )
{
    unsigned int i = 0;
    cl_int clStatus;
    int status = 0;
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
        if ( gpuEnv.mpArryDevsID[i] != 0 )
        {
            OString fileName = createFileName(gpuEnv.mpArryDevsID[i], clFileName);
            fd = fopen( fileName.getStr(), "rb" );
            status = ( fd != NULL ) ? 1 : 0;
        }
    }
    if ( fd != NULL )
    {
        *fhandle = fd;
    }
    return status;

}

int OpenclDevice::writeBinaryToFile( const OString& rFileName, const char* birary, size_t numBytes )
{
    FILE *output = NULL;
    output = fopen( rFileName.getStr(), "wb" );
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
    cl_uint numDevices;

    cl_int clStatus = clGetProgramInfo( program, CL_PROGRAM_NUM_DEVICES,
                   sizeof(numDevices), &numDevices, NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    std::vector<cl_device_id> mpArryDevsID(numDevices);
    /* grab the handles to all of the devices in the program. */
    clStatus = clGetProgramInfo( program, CL_PROGRAM_DEVICES,
                   sizeof(cl_device_id) * numDevices, &mpArryDevsID[0], NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    /* figure out the sizes of each of the binaries. */
    std::vector<size_t> binarySizes(numDevices);

    clStatus = clGetProgramInfo( program, CL_PROGRAM_BINARY_SIZES,
                   sizeof(size_t) * numDevices, &binarySizes[0], NULL );
    CHECK_OPENCL( clStatus, "clGetProgramInfo" );

    /* copy over all of the generated binaries. */
    boost::scoped_array<char*> binaries(new char*[numDevices]);

    for ( size_t i = 0; i < numDevices; i++ )
    {
        if ( binarySizes[i] != 0 )
        {
            binaries[i] = new char[binarySizes[i]];
        }
        else
        {
            binaries[i] = NULL;
        }
    }

    clStatus = clGetProgramInfo( program, CL_PROGRAM_BINARIES,
                   sizeof(char *) * numDevices, binaries.get(), NULL );
    CHECK_OPENCL(clStatus,"clGetProgramInfo");

    /* dump out each binary into its own separate file. */
    for ( size_t i = 0; i < numDevices; i++ )
    {

        if ( binarySizes[i] != 0 )
        {
            OString fileName = createFileName(mpArryDevsID[i], clFileName);
            if ( !writeBinaryToFile( fileName,
                        binaries[i], binarySizes[i] ) )
            {
                printf("opencl-wrapper: write binary[%s] failds\n", fileName.getStr());
            }
            else
                printf("opencl-wrapper: write binary[%s] succesfully\n", fileName.getStr());
        }
    }

    // Release all resouces and memory
    for ( size_t i = 0; i < numDevices; i++ )
    {
        delete[] binaries[i];
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

int OpenclDevice::releaseOpenclEnv( GPUEnv *gpuInfo )
{
    int clStatus = 0;

    if ( !isInited )
    {
        return 1;
    }

    // Release all cached kernels.
    for (size_t i = 0, n = gpuInfo->maKernels.size(); i < n; ++i)
        clReleaseKernel(gpuInfo->maKernels[i].mpKernel);
    gpuInfo->maKernels.clear();

    for (int i = 0; i < gpuEnv.mnFileCount; i++)
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
    cl_uint numDevices;
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
        if (status == 0 || gpuEnv.maKernels.empty())
        {
            printf("compileKernelFile failed.\n");
            return 1;
        }
        printf("compileKernelFile successed.\n");
        isInited = 1;
    }
    return 0;
}

namespace {

void checkDeviceForDoubleSupport(cl_device_id deviceId, bool& bKhrFp64, bool& bAmdFp64)
{
    bKhrFp64 = false;
    bAmdFp64 = false;

    // Check device extensions for double type
    size_t aDevExtInfoSize = 0;

    cl_uint clStatus = clGetDeviceInfo( deviceId, CL_DEVICE_EXTENSIONS, 0, NULL, &aDevExtInfoSize );
    if( clStatus != CL_SUCCESS )
        return;

    boost::scoped_array<char> pExtInfo(new char[aDevExtInfoSize]);

    clStatus = clGetDeviceInfo( deviceId, CL_DEVICE_EXTENSIONS,
                   sizeof(char) * aDevExtInfoSize, pExtInfo.get(), NULL);

    if( clStatus != CL_SUCCESS )
        return;

    if ( strstr( pExtInfo.get(), "cl_khr_fp64" ) )
    {
        bKhrFp64 = true;
    }
    else
    {
        // Check if cl_amd_fp64 extension is supported
        if ( strstr( pExtInfo.get(), "cl_amd_fp64" ) )
            bAmdFp64 = true;
    }
}

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

    bool bKhrFp64 = false;
    bool bAmdFp64 = false;

    checkDeviceForDoubleSupport(gpuInfo->mpArryDevsID[0], bKhrFp64, bAmdFp64);

    gpuInfo->mnKhrFp64Flag = bKhrFp64;
    gpuInfo->mnAmdFp64Flag = bAmdFp64;

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

void OclCalc::releaseOclBuffer()
{
    cl_int clStatus = 0;
    CHECK_OPENCL_RELEASE( clStatus, mpClmemSrcData );
    CHECK_OPENCL_RELEASE( clStatus, mpClmemStartPos );
    CHECK_OPENCL_RELEASE( clStatus, mpClmemEndPos );
    CHECK_OPENCL_RELEASE( clStatus, mpClmemLeftData );
    CHECK_OPENCL_RELEASE( clStatus, mpClmemRightData );
    fprintf(stderr,"OclCalc:: opencl end ...\n");
}

/////////////////////////////////////////////////////////////////////////////

bool OclCalc::createBuffer64Bits( double *&dpLeftData, double *&dpRightData, int nBufferSize )
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
    return true;
}

bool OclCalc::mapAndCopy64Bits(const double *dpTempSrcData,unsigned int *unStartPos,unsigned int *unEndPos,int nBufferSize ,int nRowsize)
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
    return true;
}

bool OclCalc::mapAndCopy64Bits(const double *dpTempLeftData,const double *dpTempRightData,int nBufferSize )
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
    return true;
}

bool OclCalc::mapAndCopyArithmetic64Bits( const double *dpMoreColArithmetic, int nBufferSize )
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
    return true;
}

bool OclCalc::mapAndCopyMoreColArithmetic64Bits( const double *dpMoreColArithmetic, int nBufferSize, uint *npeOp, uint neOpSize )
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
    return true;
}

bool OclCalc::createFormulaBuf64Bits( int nBufferSize, int rowSize )
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
    return true;
}

bool OclCalc::createArithmeticOptBuf64Bits( int nBufferSize )
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
    return true;
}

bool OclCalc::createMoreColArithmeticBuf64Bits( int nBufferSize, int neOpSize )
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
    return true;
}

bool OclCalc::oclHostArithmeticOperator64Bits( const char* aKernelName, double *&rResult,int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

    clFinish( kEnv.mpkCmdQueue );
    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE,
                          nRowSize * sizeof(double), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&mpClmemRightData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&clResult);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );

    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return true;
}

bool OclCalc::oclMoreColHostArithmeticOperator64Bits( int nDataSize,int neOpSize,double *rResult, int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    const char *aKernelName = "oclMoreColArithmeticOperator";
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nRowSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_int), (void *)&nDataSize);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&mpClmemeOp);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 3, sizeof(cl_int), (void *)&neOpSize);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 4, sizeof(cl_mem), (void *)&clResult);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return true;
}

bool OclCalc::oclHostArithmeticStash64Bits( const char* aKernelName, const double *dpLeftData, const double *dpRightData, double *rResult,int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    setKernelEnv( &kEnv );
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

    clFinish( kEnv.mpkCmdQueue );

    cl_mem clLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
    nRowSize * sizeof(double), (void *)dpLeftData, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clRightData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
        nRowSize * sizeof(double), (void *)dpRightData, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE,
                          nRowSize * sizeof(double), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&clLeftData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&clRightData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&clResult);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );

    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );

    clStatus = clEnqueueReadBuffer(kEnv.mpkCmdQueue, clResult, CL_TRUE, 0, nRowSize * sizeof(double), (double *)rResult, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueReadBuffer" );

    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clLeftData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clRightData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return true;
}

bool OclCalc::oclHostFormulaStash64Bits( const char* aKernelName, const double* dpSrcData, uint *nStartPos, uint *nEndPos, double *output, int nBufferSize, int size )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    setKernelEnv( &kEnv );
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

    cl_mem clSrcData   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
        nBufferSize * sizeof(double), (void *)dpSrcData, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
     cl_mem clStartPos   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
        size * sizeof(unsigned int), (void *)nStartPos, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clEndPos   = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR),
        size * sizeof(unsigned int), (void *)nEndPos, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    cl_mem outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, size * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem),(void *)&clSrcData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&clStartPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&clEndPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 3, sizeof(cl_mem), (void *)&outputCl);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );

    clStatus = clEnqueueReadBuffer(kEnv.mpkCmdQueue, outputCl, CL_TRUE, 0, size * sizeof(double), (double *)output, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clReadBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseMemObject( outputCl );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clSrcData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clStartPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clEndPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return true;
}

bool OclCalc::oclHostFormulaStatistics64Bits( const char* aKernelName, double *&output, int size )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

    cl_mem outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, size * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem),(void *)&mpClmemSrcData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&mpClmemStartPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&mpClmemEndPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 3, sizeof(cl_mem), (void *)&outputCl);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject( outputCl );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return true;
}

bool OclCalc::oclHostFormulaCount64Bits( uint *npStartPos, uint *npEndPos, double *&dpOutput, int nSize )
{
    const char *cpKernelName = "oclFormulaCount";
    Kernel* pKernel = fetchKernel(cpKernelName);
    if (!pKernel)
        return false;

    cl_int clStatus;

    size_t global_work_size[1];
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemStartPos, npStartPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemEndPos, npEndPos, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    cl_mem clpOutput = clCreateBuffer( kEnv.mpkContext,CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        nSize* sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&mpClmemStartPos);
    CHECK_OPENCL( clStatus,"clSetKernelArg");
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&mpClmemEndPos);
    CHECK_OPENCL( clStatus,"clSetKernelArg");
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&clpOutput);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject( clpOutput );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return true;
}

/*
 * The dpsrcData is double rows,one col is the first column data,second is the second column data.if get a cell data range,the range
 *save the npStart array eg:a4-a8;b10-b14,the npStart will store a4,b10,and the npEnd will store a8,b14 range.So it can if(i +1)%2 to judge
 * the a cloumn or b cloumn npStart range.so as b bolumn.
 */
bool OclCalc::oclHostFormulaSumProduct64Bits( double *dpSumProMergeLfData, double *dpSumProMergeRrData, uint *npSumSize, double *&dpOutput, int nSize )
{
    cl_int clStatus;
    size_t global_work_size[1];
    memset(dpOutput,0,nSize);
    const char *cpFirstKernelName = "oclSignedMul";
    const char *cpSecondKernelName = "oclFormulaSumproduct";
    Kernel* pKernel1 = fetchKernel(cpFirstKernelName);
    if (!pKernel1)
        return false;

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
    clStatus = clSetKernelArg(pKernel1->mpKernel, 0, sizeof(cl_mem),(void *)&mpClmemMergeLfData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel1->mpKernel, 1, sizeof(cl_mem), (void *)&mpClmemMergeRtData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel1->mpKernel, 2, sizeof(cl_mem), (void *)&clResult);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nMulResultSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel1->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clReleaseMemObject( mpClmemMergeLfData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemMergeRtData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );

    Kernel* pKernel2 = fetchKernel(cpSecondKernelName);
    if (!pKernel2)
        return false;

    cl_mem clpOutput = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nSize* sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_uint nMatixSize = nFormulaColSize * nFormulaRowSize;
    clStatus = clSetKernelArg(pKernel2->mpKernel, 0, sizeof(cl_mem), (void *)&clResult);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel2->mpKernel, 1, sizeof(cl_mem), (void *)&mpClmemMatixSumSize);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel2->mpKernel, 2, sizeof(cl_mem), (void *)&clpOutput);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel2->mpKernel, 3, sizeof(cl_uint), (void *)&nMatixSize);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel2->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( mpClmemMatixSumSize );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpOutput );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );

    return true;
}

bool OclCalc::createMoreColArithmeticBuf32Bits( int nBufferSize, int neOpSize )
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
    return true;
}

bool OclCalc::createArithmeticOptBuf32Bits( int nBufferSize )
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
    return true;
}

bool OclCalc::createFormulaBuf32Bits( int nBufferSize, int rowSize )
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
    return true;
}

bool OclCalc::createBuffer32Bits( float *&fpLeftData, float *&fpRightData, int nBufferSize )
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
    return true;
}

bool OclCalc::mapAndCopy32Bits(const double *dpTempSrcData,unsigned int *unStartPos,unsigned int *unEndPos,int nBufferSize ,int nRowsize)
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
    return true;
}

bool OclCalc::mapAndCopy32Bits(const double *dpTempLeftData,const double *dpTempRightData,int nBufferSize )
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
    return true;
}

bool OclCalc::mapAndCopyArithmetic32Bits( const double *dpMoreColArithmetic, int nBufferSize )
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
    return true;
}

bool OclCalc::mapAndCopyMoreColArithmetic32Bits( const double *dpMoreColArithmetic, int nBufferSize, uint *npeOp, uint neOpSize )
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
    return true;
}

bool OclCalc::oclHostArithmeticOperator32Bits( const char* aKernelName,double *rResult, int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];

    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nRowSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&mpClmemRightData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&clResult);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    float * hostMapResult = (float *)clEnqueueMapBuffer(
        kEnv.mpkCmdQueue, clResult, CL_TRUE, CL_MAP_READ, 0, nRowSize*sizeof(float), 0, NULL, NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    for ( int i = 0; i < nRowSize; i++)
        rResult[i] = hostMapResult[i]; // from gpu float type to cpu double type
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clResult, hostMapResult, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );

    return true;
}

bool OclCalc::oclMoreColHostArithmeticOperator32Bits( int nDataSize,int neOpSize,double *rResult, int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    const char *aKernelName = "oclMoreColArithmeticOperator";
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

    cl_mem clResult = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nRowSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_int), (void *)&nDataSize);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&mpClmemeOp);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 3, sizeof(cl_int), (void *)&neOpSize);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 4, sizeof(cl_mem), (void *)&clResult);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );

    return true;
}

bool OclCalc::oclHostFormulaStatistics32Bits(const char* aKernelName,double *output,int size)
{
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

    cl_int clStatus = 0;
    size_t global_work_size[1];

    cl_mem outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, size * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&mpClmemSrcData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&mpClmemStartPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&mpClmemEndPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 3, sizeof(cl_mem), (void *)&outputCl);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject( outputCl );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return true;
}

bool OclCalc::oclHostArithmeticStash32Bits( const char* aKernelName, const double *dpLeftData, const double *dpRightData, double *rResult,int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    setKernelEnv( &kEnv );
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

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

    clFinish( kEnv.mpkCmdQueue );

    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&clLeftData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&clRightData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&clResult);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );

    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clLeftData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clRightData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return true;
}

bool OclCalc::oclHostFormulaStash32Bits( const char* aKernelName, const double* dpSrcData, uint *nStartPos, uint *nEndPos, double *output, int nBufferSize, int size )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    setKernelEnv( &kEnv );
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

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

    cl_mem outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, size * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem),(void *)&clSrcData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&clStartPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&clEndPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 3, sizeof(cl_mem), (void *)&outputCl);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject( outputCl );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clSrcData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clStartPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clEndPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return true;
}

bool OclCalc::oclHostFormulaCount32Bits( uint *npStartPos, uint *npEndPos, double *dpOutput, int nSize )
{
    const char *cpKernelName = "oclFormulaCount";
    Kernel* pKernel = fetchKernel(cpKernelName);
    if (!pKernel)
        return false;

    cl_int clStatus;
    size_t global_work_size[1];

    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemStartPos, npStartPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemEndPos, npEndPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    cl_mem clpOutput = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
        nSize* sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&mpClmemStartPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&mpClmemEndPos);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&clpOutput);
    CHECK_OPENCL(clStatus, "clSetKernelArg");
    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject(mpClmemSrcData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemStartPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemEndPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clpOutput );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return true;
}

//sumproduct
bool OclCalc::oclHostFormulaSumProduct32Bits( float *fpSumProMergeLfData, float *fpSumProMergeRrData, uint *npSumSize, double *dpOutput, int nSize )
{
    cl_int clStatus;
    size_t global_work_size[1];
    memset(dpOutput,0,nSize);
    const char *cpFirstKernelName = "oclSignedMul";
    const char *cpSecondKernelName = "oclFormulaSumproduct";
    Kernel* pKernel1 = fetchKernel(cpFirstKernelName);
    if (!pKernel1)
        return false;

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
    clStatus = clSetKernelArg(pKernel1->mpKernel, 0, sizeof(cl_mem), (void *)&mpClmemMergeLfData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel1->mpKernel, 1, sizeof(cl_mem), (void *)&mpClmemMergeRtData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel1->mpKernel, 2, sizeof(cl_mem), (void *)&clResult);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nMulResultSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel1->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL( clStatus, "clEnqueueNDRangeKernel" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clReleaseMemObject( mpClmemMergeLfData );
    CHECK_OPENCL( clStatus,"clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemMergeRtData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );

    Kernel* pKernel2 = fetchKernel(cpSecondKernelName);
    if (!pKernel2)
        return false;

    cl_mem clpOutput = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nSize* sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_uint nMatixSize = nFormulaColSize * nFormulaRowSize;
    clStatus = clSetKernelArg(pKernel2->mpKernel, 0, sizeof(cl_mem), (void *)&clResult);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel2->mpKernel, 1, sizeof(cl_mem), (void *)&mpClmemMatixSumSize);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel2->mpKernel, 2, sizeof(cl_mem), (void *)&clpOutput);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel2->mpKernel, 3, sizeof(cl_uint), (void *)&nMatixSize);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel2->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( mpClmemMatixSumSize );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpOutput );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    return true;
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

bool OclCalc::oclGroundWaterGroup( uint *eOp, uint eOpNum, const double *pOpArray, const double *pSubtractSingle, size_t nSrcDataSize,size_t nElements, double del ,uint *nStartPos,uint *nEndPos,double *dpResult)
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
    Kernel* pKernel = fetchKernel(kernelName);
    if (!pKernel)
        return false;

    cl_int clStatus;
    size_t global_work_size[1];
    if ( ( eOpNum == 1 ) && ( eOp[0] == ocSub ) )
        subFlag = true;

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

        clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&valuesCl);
        CHECK_OPENCL( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&subtractCl);
        CHECK_OPENCL( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&startPosCL);
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg(pKernel->mpKernel, 3, sizeof(cl_mem), (void *)&endPosCL);
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg(pKernel->mpKernel, 4, sizeof(cl_mem), (void *)&outputCl);
        CHECK_OPENCL( clStatus, "clSetKernelArg" );

        fprintf( stderr, "prior to enqueue range kernel\n" );
    }
    else
    {
        if ( gpuEnv.mnKhrFp64Flag || gpuEnv.mnAmdFp64Flag )
        {
             subtractCl = allocateDoubleBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
             outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nElements * sizeof(double), NULL, &clStatus );
             clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_double), (void *)&delta);
             CHECK_OPENCL( clStatus, "clSetKernelArg");
        }
        else
        {
             float fTmp = (float)delta;
             subtractCl = allocateFloatBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
             outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nElements * sizeof(float), NULL, &clStatus );
             clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_float), (void *)&fTmp);
             CHECK_OPENCL( clStatus, "clSetKernelArg");
        }
        clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&subtractCl);
        CHECK_OPENCL( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&outputCl);
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
    }
    global_work_size[0] = nElements;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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

    CHECK_OPENCL_RELEASE( clStatus, valuesCl );
    CHECK_OPENCL_RELEASE( clStatus, subtractCl );
    CHECK_OPENCL_RELEASE( clStatus, outputCl );
    CHECK_OPENCL_RELEASE( clStatus, startPosCL );
    CHECK_OPENCL_RELEASE( clStatus, endPosCL );

    fprintf( stderr, "completed opencl operation\n" );

    return true;
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

    Kernel* pKernel = fetchKernel(kernelName);
    if (!pKernel)
        return NULL;

    cl_int clStatus;
    size_t global_work_size[1];

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
        CHECK_OPENCL_PTR( clStatus, "clCreateBuffer" );

        cl_uint start = 0;
        cl_uint end = (cl_uint)nElements;

        clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&valuesCl);
        CHECK_OPENCL_PTR( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&subtractCl);
        CHECK_OPENCL_PTR( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_uint), (void *)&start);
        CHECK_OPENCL_PTR( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg(pKernel->mpKernel, 3, sizeof(cl_uint), (void *)&end);
        CHECK_OPENCL_PTR( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg(pKernel->mpKernel, 4, sizeof(cl_mem), (void *)&outputCl);
        CHECK_OPENCL_PTR( clStatus, "clSetKernelArg" );

        fprintf( stderr, "prior to enqueue range kernel\n" );
    }
    else
    {
        if ( gpuEnv.mnKhrFp64Flag || gpuEnv.mnAmdFp64Flag )
        {
             subtractCl = allocateDoubleBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
             outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nElements * sizeof(double), NULL, &clStatus );
             clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_double), (void *)&delta);
             CHECK_OPENCL_PTR( clStatus, "clSetKernelArg");
        }
        else
       {
             float fTmp = (float)delta;
             subtractCl = allocateFloatBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
             outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nElements * sizeof(float), NULL, &clStatus );
             clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_float), (void *)&fTmp);
             CHECK_OPENCL_PTR( clStatus, "clSetKernelArg");
        }
        clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&subtractCl);
        CHECK_OPENCL_PTR( clStatus, "clSetKernelArg");
        clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&outputCl);
        CHECK_OPENCL_PTR( clStatus, "clSetKernelArg" );
    }
    global_work_size[0] = nElements;
    clStatus = clEnqueueNDRangeKernel(
        kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL_PTR( clStatus, "clEnqueueNDRangeKernel" );
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
    CHECK_OPENCL_PTR( clStatus, "clEnqueueReadBuffer" );

    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL_PTR( clStatus, "clFinish" );

    if ( valuesCl != NULL )
    {
        clStatus = clReleaseMemObject( valuesCl );
        CHECK_OPENCL_PTR( clStatus, "clReleaseMemObject" );
    }
    if ( subtractCl != NULL )
    {
        clStatus = clReleaseMemObject( subtractCl );
        CHECK_OPENCL_PTR( clStatus, "clReleaseMemObject" );
    }
    if ( outputCl != NULL )
    {
        clStatus = clReleaseMemObject( outputCl );
        CHECK_OPENCL_PTR( clStatus, "clReleaseMemObject" );
    }
    fprintf( stderr, "completed opencl delta operation\n" );

    return pResult;
}

bool OclCalc::oclHostMatrixInverse64Bits( const char* aKernelName, double *dpOclMatrixSrc, double *dpOclMatrixDst,std::vector<double>&dpResult,  uint nDim )
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
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&clpPData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    for ( uint nOffset = 0; nOffset < nDim- 1; nOffset++ )
    {
        int nMax = nOffset;
        for ( uint i = nOffset + 1; i < nDim; i++ )
        {
            if( fabs(dpOclMatrixSrc[nMax*nDim+nOffset]) < fabs(dpOclMatrixSrc[i*nDim+nOffset]))
                nMax=i;
        }
        clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&nOffset);
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg(pKernel->mpKernel, 3, sizeof(cl_mem), (void *)&nMax);
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clEnqueueNDRangeKernel(
            kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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

    return true;
}

bool OclCalc::oclHostMatrixInverse32Bits( const char* aKernelName, float *fpOclMatrixSrc, float *fpOclMatrixDst, std::vector<double>& dpResult, uint nDim )
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
    Kernel* pKernel = fetchKernel(aKernelName);
    if (!pKernel)
        return false;

    clStatus = clSetKernelArg(pKernel->mpKernel, 0, sizeof(cl_mem), (void *)&mpClmemLeftData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );
    clStatus = clSetKernelArg(pKernel->mpKernel, 1, sizeof(cl_mem), (void *)&clpPData);
    CHECK_OPENCL( clStatus, "clSetKernelArg" );

    for ( uint nOffset = 0; nOffset < nDim- 1; nOffset++ )
    {
        int nMax = nOffset;
        for( uint i = nOffset+1; i < nDim; i++ )
        {
            if( fabs(fpOclMatrixSrc[nMax*nDim+nOffset]) < fabs(fpOclMatrixSrc[i*nDim+nOffset]))
                nMax=i;
        }
        clStatus = clSetKernelArg(pKernel->mpKernel, 2, sizeof(cl_mem), (void *)&nOffset);
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clSetKernelArg(pKernel->mpKernel, 3, sizeof(cl_mem), (void *)&nMax);
        CHECK_OPENCL( clStatus, "clSetKernelArg" );
        clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, pKernel->mpKernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
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
    return true;
}

namespace {

void createDeviceInfo(cl_device_id aDeviceId, OpenclPlatformInfo& rPlatformInfo)
{
    OpenclDeviceInfo aDeviceInfo;
    aDeviceInfo.device = aDeviceId;

    char pName[DEVICE_NAME_LENGTH];
    cl_int nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_NAME, DEVICE_NAME_LENGTH, pName, NULL);
    if(nState != CL_SUCCESS)
        return;

    aDeviceInfo.maName = OUString::createFromAscii(pName);

    char pVendor[DEVICE_NAME_LENGTH];
    nState = clGetDeviceInfo(aDeviceId, CL_DEVICE_VENDOR, DEVICE_NAME_LENGTH, pName, NULL);
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

    bool bKhrFp64 = false;
    bool bAmdFp64 = false;
    checkDeviceForDoubleSupport(aDeviceId, bKhrFp64, bAmdFp64);

    // only list devices that support double
    if(!bKhrFp64 && !bAmdFp64)
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

namespace {

cl_device_id findDeviceIdByDeviceString(const OUString& rString, const std::vector<OpenclPlatformInfo>& rPlatforms)
{
    std::vector<OpenclPlatformInfo>::const_iterator it = rPlatforms.begin(), itEnd = rPlatforms.end();
    for(; it != itEnd; ++it)
    {
        std::vector<OpenclDeviceInfo>::const_iterator itr = it->maDevices.begin(), itrEnd = it->maDevices.end();
        for(; itr != itrEnd; ++itr)
        {
            OUString aDeviceId = it->maVendor + " " + itr->maName;
            if(rString == aDeviceId)
            {
                return static_cast<cl_device_id>(itr->device);
            }
        }
    }

    return NULL;
}

}

bool switchOpenclDevice(const OUString* pDevice, bool bAutoSelect)
{
    cl_device_id pDeviceId = NULL;
    if(pDevice)
        pDeviceId = findDeviceIdByDeviceString(*pDevice, fillOpenCLInfo());

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
        return true;
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
        return false;
    }

    cl_command_queue command_queue = clCreateCommandQueue(
            context, pDeviceId, 0, &nState);

    if(command_queue == NULL || nState != CL_SUCCESS)
    {
        if(command_queue != NULL)
            clReleaseCommandQueue(command_queue);

        clReleaseContext(context);
        SAL_WARN("sc", "failed to set/switch opencl device");
        return false;
    }

    OpenclDevice::releaseOpenclEnv(&OpenclDevice::gpuEnv);
    OpenCLEnv env;
    env.mpOclPlatformID = platformId;
    env.mpOclContext = context;
    env.mpOclDevsID = pDeviceId;
    env.mpOclCmdQueue = command_queue;
    OpenclDevice::initOpenclAttr(&env);

    // why do we need this at all?
    OpenclDevice::gpuEnv.mpArryDevsID = (cl_device_id*) malloc( 1 );
    OpenclDevice::gpuEnv.mpArryDevsID[0] = pDeviceId;
    return !OpenclDevice::initOpenclRunEnv(0);
}

void compileKernels(const OUString* pDeviceId)
{
    if (!pDeviceId)
        return;

    if (pDeviceId->isEmpty())
        return;

    if (!switchOpenclDevice(pDeviceId, false))
        return;

    cl_program pProgram = OpenclDevice::gpuEnv.mpArryPrograms[0];
    if (!pProgram)
        return;

    cl_int nStatus;
    for (size_t i = 0, n = OpenclDevice::gpuEnv.maKernels.size(); i < n; ++i)
    {
        Kernel& r = OpenclDevice::gpuEnv.maKernels[i];
        if (r.mpKernel)
            continue;

        r.mpKernel = clCreateKernel(pProgram, r.mpName, &nStatus);
        if (nStatus != CL_SUCCESS)
            r.mpKernel = NULL;
    }

}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
