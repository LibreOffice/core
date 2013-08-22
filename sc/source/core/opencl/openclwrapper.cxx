/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "openclwrapper.hxx"

#include <stdlib.h>
#include <string.h>
#include <cmath>
#include "sal/config.h"
#include "random.hxx"
#include "oclkernels.hxx"
#ifdef SAL_WIN32
#include <Windows.h>
#endif
//#define USE_KERNEL_FILE

GPUEnv OpenclDevice::gpuEnv;
int OpenclDevice::isInited =0;


#ifdef SAL_WIN32

#define OPENCL_DLL_NAME "opencllo.dll"
#define OCLERR -1
#define OCLSUCCESS 1

#define TRUE 1
#define FALSE 0

#define OCL_INFO(str) \
    printf("[OCL_INFO] %s\n",str);
#define OCL_ERROR(str) \
    fprintf(stderr,"[OCL_ERROR] %s\n",str);
#define OCL_CHECK(value1,value2,str) \
    if(value1!=value2) \
        fprintf(stderr,"[OCL_ERROR] %s\n",str);

HINSTANCE HOpenclDll = NULL;
void * OpenclDll = NULL;

int OpenclDevice::LoadOpencl()
{
    //fprintf(stderr, " LoadOpenclDllxx... \n");
    OpenclDll = static_cast<HINSTANCE>( HOpenclDll );
    OpenclDll = LoadLibrary( OPENCL_DLL_NAME );
    if ( !static_cast<HINSTANCE>( OpenclDll ) )
    {
        fprintf(stderr, " Load opencllo.dll failed! \n");
        FreeLibrary( static_cast<HINSTANCE>( OpenclDll ) );
        return OCLERR;
    }
    fprintf(stderr, " Load opencllo.dll successfully!\n");
    return OCLSUCCESS;
}

void OpenclDevice::FreeOpenclDll()
{
    fprintf(stderr, " Free opencllo.dll ... \n");
    if ( !static_cast<HINSTANCE>( OpenclDll ) )
        FreeLibrary( static_cast<HINSTANCE>( OpenclDll ) );
}
#endif

int OpenclDevice::InitEnv()
{
#ifdef SAL_WIN32
    while( 1 )
    {
        if( 1 == LoadOpencl() )
            break;
    }
#endif
    InitOpenclRunEnv( 0 );
    return 1;
}

int OpenclDevice::ReleaseOpenclRunEnv()
{
    ReleaseOpenclEnv( &gpuEnv );
#ifdef SAL_WIN32
    FreeOpenclDll();
#endif
    return 1;
}
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
inline int OpenclDevice::AddKernelConfig( int kCount, const char *kName )
{
    if ( kCount < 1 )
        fprintf(stderr,"Error: ( KCount < 1 )" SAL_DETAIL_WHERE "AddKernelConfig\n" );
    strcpy( gpuEnv.mArrykernelNames[kCount-1], kName );
    gpuEnv.mnKernelCount++;
    return 0;
}

int OpenclDevice::RegistOpenclKernel()
{
    if ( !gpuEnv.mnIsUserCreated )
        memset( &gpuEnv, 0, sizeof(gpuEnv) );

    gpuEnv.mnFileCount = 0; //argc;
    gpuEnv.mnKernelCount = 0UL;

    AddKernelConfig( 1, (const char*) "oclFormulaMin" );
    AddKernelConfig( 2, (const char*) "oclFormulaMax" );
    AddKernelConfig( 3, (const char*) "oclFormulaSum" );
    AddKernelConfig( 4, (const char*) "oclFormulaCount" );
    AddKernelConfig( 5, (const char*) "oclFormulaAverage" );
    AddKernelConfig( 6, (const char*) "oclFormulaSumproduct" );
    AddKernelConfig( 7, (const char*) "oclFormulaMtxInv" );

    AddKernelConfig( 8, (const char*) "oclSignedAdd" );
    AddKernelConfig( 9, (const char*) "oclSignedSub" );
    AddKernelConfig( 10, (const char*) "oclSignedMul" );
    AddKernelConfig( 11, (const char*) "oclSignedDiv" );
    AddKernelConfig( 12, (const char*) "oclAverageDelta" );
    AddKernelConfig( 13, (const char*) "oclMaxDelta" );
    AddKernelConfig( 14, (const char*) "oclMinDelta" );
    AddKernelConfig( 15, (const char*) "oclSubDelta" );
    AddKernelConfig( 16, (const char*) "oclLUDecomposition" );
    return 0;
}

OpenclDevice::OpenclDevice()
{
    //InitEnv();
}

OpenclDevice::~OpenclDevice()
{
    //ReleaseOpenclRunEnv();
}

int OpenclDevice::SetKernelEnv( KernelEnv *envInfo )
{
    envInfo->mpkContext = gpuEnv.mpContext;
    envInfo->mpkCmdQueue = gpuEnv.mpCmdQueue;
    envInfo->mpkProgram = gpuEnv.mpArryPrograms[0];

    return 1;
}

int OpenclDevice::CheckKernelName( KernelEnv *envInfo, const char *kernelName )
{
    //printf("CheckKernelName,total count of kernels...%d\n", gpuEnv.kernelCount);
    int kCount;
    int nFlag = 0;
    for ( kCount = 0; kCount < gpuEnv.mnKernelCount; kCount++ )
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

int OpenclDevice::ConvertToString( const char *filename, char **source )
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

int OpenclDevice::BinaryGenerated( const char * clFileName, FILE ** fhandle )
{
    unsigned int i = 0;
    cl_int clStatus;
    int status = 0;
    char *str = NULL;
    FILE *fd = NULL;
    cl_uint numDevices=0;
    clStatus = clGetDeviceIDs(gpuEnv.mpPlatformID, // platform
                              CL_DEVICE_TYPE_GPU, // device_type
                              0, // num_entries
                              NULL, // devices ID
                              &numDevices);
    for ( i = 0; i < numDevices; i++ )
    {
        if ( gpuEnv.mpArryDevsID[i] != 0 )
        {
            char fileName[256] = { 0 }, cl_name[128] = { 0 };
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

int OpenclDevice::WriteBinaryToFile( const char* fileName, const char* birary, size_t numBytes )
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

int OpenclDevice::GeneratBinFromKernelSource( cl_program program, const char * clFileName )
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
                for ( unsigned int j = 0; j < i ; j++)
                {
                    if (binaries[j])
                        free(binaries[j]);
                }
                free(binaries);
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
        if ( binarySizes[i] != 0 )
        {
            char fileName[256] = { 0 }, cl_name[128] = { 0 };
            char deviceName[1024];
            clStatus = clGetDeviceInfo(mpArryDevsID[i], CL_DEVICE_NAME,
                           sizeof(deviceName), deviceName, NULL);
            CHECK_OPENCL( clStatus, "clGetDeviceInfo" );

            str = (char*) strstr( clFileName, (char*) ".cl" );
            memcpy( cl_name, clFileName, str - clFileName );
            cl_name[str - clFileName] = '\0';
            sprintf( fileName, "./%s-%s.bin", cl_name, deviceName );

            if ( !WriteBinaryToFile( fileName, binaries[i], binarySizes[i] ) )
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

int OpenclDevice::InitOpenclAttr( OpenCLEnv * env )
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

int OpenclDevice::CreateKernel( char * kernelname, KernelEnv * env )
{
    int clStatus;

    env->mpkKernel = clCreateKernel( gpuEnv.mpArryPrograms[0], kernelname, &clStatus );
    env->mpkContext = gpuEnv.mpContext;
    env->mpkCmdQueue = gpuEnv.mpCmdQueue;
    return clStatus != CL_SUCCESS ? 1 : 0;
}

int OpenclDevice::ReleaseKernel( KernelEnv * env )
{
    int clStatus = clReleaseKernel( env->mpkKernel );
    return clStatus != CL_SUCCESS ? 1 : 0;
}

int OpenclDevice::ReleaseOpenclEnv( GPUEnv *gpuInfo )
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

int OpenclDevice::RunKernelWrapper( cl_kernel_function function, const char * kernelName, void **usrdata )
{
    printf("oclwrapper:RunKernel_wrapper...\n");
    if ( RegisterKernelWrapper( kernelName, function ) != 1 )
    {
        fprintf(stderr, "Error:RunKernel_wrapper:RegisterKernelWrapper fail!\n");
        return -1;
    }
    return ( RunKernel( kernelName, usrdata ) );
}

int OpenclDevice::CachedOfKernerPrg( const GPUEnv *gpuEnvCached, const char * clFileName )
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

int OpenclDevice::CompileKernelFile( GPUEnv *gpuInfo, const char *buildOption )
{
    cl_int clStatus = 0;
    size_t length;
    char *binary;
    const char *source;
    size_t source_size[1];
    int binary_status, binaryExisted, idx;
    cl_uint numDevices;
    FILE *fd;
    const char* filename = "kernel.cl";
    fprintf(stderr, "CompileKernelFile ... \n");
    if ( CachedOfKernerPrg(gpuInfo, filename) == 1 )
    {
        return 1;
    }

    idx = gpuInfo->mnFileCount;

    source = kernel_src;

    source_size[0] = strlen( source );
    binaryExisted = 0;
    if ( ( binaryExisted = BinaryGenerated( filename, &fd ) ) == 1 )
    {
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_NUM_DEVICES,
                       sizeof(numDevices), &numDevices, NULL );
        CHECK_OPENCL( clStatus, "clGetContextInfo" );

        cl_device_id *mpArryDevsID = (cl_device_id*) malloc( sizeof(cl_device_id) * numDevices );
        if ( mpArryDevsID == NULL )
        {
            return 0;
        }

        int b_error = 0;
        length = 0;
        b_error |= fseek( fd, 0, SEEK_END ) < 0;
        b_error |= ( length = ftell(fd) ) <= 0;
        b_error |= fseek( fd, 0, SEEK_SET ) < 0;
        if ( b_error )
        {
            free(mpArryDevsID);
            return 0;
        }

        binary = (char*) malloc( length + 2 );
        if ( !binary )
        {
            free(mpArryDevsID);
            return 0;
        }

        memset( binary, 0, length + 2 );
        b_error |= fread( binary, 1, length, fd ) != length;


        fclose( fd );
        fd = NULL;
        // grab the handles to all of the devices in the context.
        clStatus = clGetContextInfo( gpuInfo->mpContext, CL_CONTEXT_DEVICES,
                       sizeof( cl_device_id ) * numDevices, mpArryDevsID, NULL );
        if (clStatus != CL_SUCCESS)
        {
            fprintf (stderr, "OpenCL error code is %d at " SAL_DETAIL_WHERE " when clGetContextInfo .\n", clStatus);
            free(binary);
            return 0;
        }

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
        char* buildLog = (char*) malloc( length );
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
            free(buildLog);
            return 0;
        }

        FILE *fd1 = fopen( "kernel-build.log", "w+" );
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
        GeneratBinFromKernelSource( gpuEnv.mpArryPrograms[idx], filename );

    gpuInfo->mnFileCount += 1;

    return 1;
}

int OpenclDevice::GetKernelEnvAndFunc( const char *kernelName, KernelEnv *env, cl_kernel_function *function)
{
    int i;
    //printf("----------------OpenclDevice::GetKernelEnvAndFunc\n");
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

int OpenclDevice::RunKernel( const char *kernelName, void **userdata)
{
    KernelEnv kEnv;
    cl_kernel_function function;
    int status;

    memset( &kEnv, 0, sizeof( KernelEnv ) );
    status = GetKernelEnvAndFunc( kernelName, &kEnv, &function );
    strcpy( kEnv.mckKernelName, kernelName );
    if ( status == 1 )
    {
        if ( &kEnv == (KernelEnv *) NULL || &function == (cl_kernel_function *) NULL)
            return 0;
        return ( function( userdata, &kEnv ) );
    }
    return 0;
}

int OpenclDevice::InitOpenclRunEnv( int argc )
{
    if ( MAX_CLKERNEL_NUM <= 0 )
    {
        return 1;
    }
    if ( ( argc > MAX_CLFILE_NUM ) || ( argc < 0 ) )
        return 1;

    if ( !isInited )
    {
        RegistOpenclKernel();
        //initialize devices, context, comand_queue
        int status = InitOpenclRunEnv( &gpuEnv );
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
            status = CompileKernelFile( &gpuEnv, "-D KHR_DP_EXTENSION -Dfp_t=double" );
        }
        else if( gpuEnv.mnAmdFp64Flag )
        {
            printf("----use amd double type in kernel----\n");
            status = CompileKernelFile( &gpuEnv, "-D AMD_DP_EXTENSION -Dfp_t=double" );
        }
        else
        {
            printf("----use float type in kernel----\n");
            status = CompileKernelFile( &gpuEnv, "-Dfp_t=float" );
        }
        if ( status == 0 || gpuEnv.mnKernelCount == 0 )
        {
            printf("CompileKernelFile failed.\n");
            return 1;
        }
        printf("CompileKernelFile successed.\n");
        isInited = 1;
    }
    return 0;
}

int OpenclDevice::InitOpenclRunEnv( GPUEnv *gpuInfo )
{
    size_t length;
    cl_int clStatus;
    cl_uint numPlatforms, numDevices;
    cl_platform_id *platforms;
    cl_context_properties cps[3];

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

            char platformName[256];
            for ( size_t i = 0; i < numPlatforms; i++ )
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

                    clStatus = clGetDeviceIDs(gpuInfo->mpPlatformID, // platform
                                              CL_DEVICE_TYPE_GPU,    // device_type
                                              0,                     // num_entries
                                              NULL,                  // devices
                                              &numDevices);

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
        // Check for GPU.
        gpuInfo->mDevType = CL_DEVICE_TYPE_GPU;
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
    CHECK_OPENCL( clStatus, "GetCommandQueueInfo" );
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
int OpenclDevice::RegisterKernelWrapper( const char *kernelName, cl_kernel_function function )
{
    int i;
    //printf("oclwrapper:RegisterKernelWrapper...%d\n", gpuEnv.mnKernelCount);
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

void OpenclDevice::SetOpenclState( int state )
{
    //printf("OpenclDevice::setOpenclState...\n");
    isInited = state;
}

int OpenclDevice::GetOpenclState()
{
    return isInited;
}

OclCalc::OclCalc()
{
    fprintf(stderr,"OclCalc:: init opencl ...\n");
    nFormulaColSize = 0;
    nFormulaRowSize = 0;
}

OclCalc::~OclCalc()
{
    fprintf(stderr,"OclCalc:: opencl end ...\n");
}

/////////////////////////////////////////////////////////////////////////////
int OclCalc::CreateBuffer64Bits( double *&dpSrcData, uint *&npStartPos, uint *&npEndPos, int nBufferSize )
{
    cl_int clStatus = 0;
    SetKernelEnv( &kEnv );

    mpClmemSrcData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                            nBufferSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemStartPos = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                             nBufferSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemEndPos = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                             nBufferSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    dpSrcData = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemSrcData, CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, 0,
                               nBufferSize * sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL(clStatus,"clEnqueueMapBuffer");
    clFinish(kEnv.mpkCmdQueue);
    npStartPos = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemStartPos, CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, 0,
                             nBufferSize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    npEndPos = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemEndPos, CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, 0,
                             nBufferSize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus,"clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    //printf("In CreateBuffer, pptrr is %d,%d,%d\n",fpSrcData,npStartPos,npEndPos);
    return 0;
}

int OclCalc::CreateBuffer64Bits( double *&dpLeftData, double *&dpRightData, int nBufferSize )
{
    cl_int clStatus = 0;
    SetKernelEnv( &kEnv );

    mpClmemLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                          nBufferSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemRightData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
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
int OclCalc::CreateBuffer64Bits( double *&dpSumProMergeLfData, double *&dpSumProMergeRtData, uint *&npSumSize, int nMatixSize, int nBufferSize )
{
    cl_int clStatus = 0;
    SetKernelEnv( &kEnv );
    mpClmemMergeLfData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                                nMatixSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemMergeRtData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                                nMatixSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemMatixSumSize = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                             nMatixSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );

    dpSumProMergeLfData = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemMergeLfData, CL_TRUE,
                                        CL_MAP_WRITE_INVALIDATE_REGION, 0, nMatixSize * sizeof(double),
                                        0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish(kEnv.mpkCmdQueue);
    dpSumProMergeRtData = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemMergeRtData, CL_TRUE,
                                        CL_MAP_WRITE_INVALIDATE_REGION, 0, nMatixSize * sizeof(double),
                                        0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    npSumSize   = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemMatixSumSize, CL_TRUE,
                              CL_MAP_WRITE_INVALIDATE_REGION, 0, nBufferSize * sizeof(uint),
                              0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    //printf("In CreateBuffer, pptrr is %d,%d,%d\n",fpSrcData,npStartPos,npEndPos);
    return 0;
}
int OclCalc::OclHostArithmeticOperator64Bits( const char* aKernelName, double *fpLeftData, double *fpRightData, double *&rResult,int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    CheckKernelName( &kEnv, aKernelName );

    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, aKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemLeftData, fpLeftData, 0, NULL, NULL );
    CHECK_OPENCL(clStatus,"clEnqueueUnmapMemObject");
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemRightData, fpRightData, 0, NULL, NULL );
    CHECK_OPENCL(clStatus,"clEnqueueUnmapMemObject");
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

    rResult = (double *) clEnqueueMapBuffer( kEnv.mpkCmdQueue, clResult, CL_TRUE,CL_MAP_READ,
                            0, nRowSize*sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clResult, rResult, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );

    clStatus = clFinish( kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject");
    clStatus = clReleaseMemObject( mpClmemRightData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::OclHostFormulaStatistics64Bits( const char* aKernelName, double *fpSrcData, uint *npStartPos, uint *npEndPos, double *&output, int size )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    CheckKernelName( &kEnv, aKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemSrcData, fpSrcData, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemStartPos, npStartPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish(kEnv.mpkCmdQueue);
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemEndPos, npEndPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject");
    clFinish( kEnv.mpkCmdQueue );
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
    output = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, outputCl, CL_TRUE, CL_MAP_READ,
        0, size*sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, outputCl, output, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clStatus = clFinish(kEnv.mpkCmdQueue );
    CHECK_OPENCL( clStatus, "clFinish" );
    clStatus = clReleaseKernel( kEnv.mpkKernel );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject(mpClmemSrcData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject(mpClmemStartPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemEndPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( outputCl );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::OclHostFormulaCount64Bits( uint *npStartPos, uint *npEndPos, double *&dpOutput, int nSize )
{
    const char *cpKernelName = "oclFormulaCount";
    CheckKernelName( &kEnv, cpKernelName );
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
    clStatus = clReleaseMemObject( mpClmemSrcData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemStartPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemEndPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clpOutput );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

/*
 * The dpsrcData is double rows,one col is the first column data,second is the second column data.if get a cell data range,the range
 *save the npStart array eg:a4-a8;b10-b14,the npStart will store a4,b10,and the npEnd will store a8,b14 range.So it can if(i +1)%2 to judge
 * the a cloumn or b cloumn npStart range.so as b bolumn.
 */
int OclCalc::OclHostFormulaSumProduct64Bits( double *dpSumProMergeLfData, double *dpSumProMergeRrData, uint *npSumSize, double *&dpOutput, int nSize )
{
    cl_int clStatus;
    size_t global_work_size[1];
    memset(dpOutput,0,nSize);
    const char *cpFirstKernelName = "oclSignedMul";
    const char *cpSecondKernelName = "oclFormulaSumproduct";
    CheckKernelName( &kEnv, cpFirstKernelName );
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
    CheckKernelName( &kEnv, cpSecondKernelName );
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
    dpOutput = (double *)clEnqueueMapBuffer(kEnv.mpkCmdQueue, clpOutput, CL_TRUE, CL_MAP_READ, 0,
        nSize*sizeof(double), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpOutput, dpOutput, 0, NULL, NULL );
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

int OclCalc::CreateBuffer32Bits( float *&fpSrcData, uint *&npStartPos, uint *&npEndPos, int nBufferSize )
{
    cl_int clStatus = 0;
    SetKernelEnv( &kEnv );
    mpClmemSrcData    = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
            nBufferSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemStartPos    = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
            nBufferSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemEndPos    = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
            nBufferSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    fpSrcData = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemSrcData, CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, 0,
            nBufferSize * sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    npStartPos = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemStartPos, CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, 0,
            nBufferSize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    npEndPos = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemEndPos, CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, 0,
            nBufferSize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    //printf("In CreateBuffer, pptrr is %d,%d,%d\n",fpSrcData,npStartPos,npEndPos);
    return 0;
}

int OclCalc::CreateBuffer32Bits( float *&fpLeftData, float *&fpRightData, int nBufferSize )
{
    cl_int clStatus = 0;
    SetKernelEnv( &kEnv );
    mpClmemLeftData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
            nBufferSize * sizeof(float), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemRightData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
        nBufferSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    fpLeftData = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemLeftData, CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION,
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
int OclCalc::CreateBuffer32Bits( float *&fpSumProMergeLfData, float *&fpSumProMergeRtData, uint *&npSumSize, int nMatixSize, int nBufferSize )
{
    cl_int clStatus = 0;
    SetKernelEnv( &kEnv );
    mpClmemMergeLfData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
        nMatixSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemMergeRtData = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
        nMatixSize * sizeof(float), NULL, &clStatus);
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    mpClmemMatixSumSize = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
        nMatixSize * sizeof(unsigned int), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    fpSumProMergeLfData = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemMergeLfData, CL_TRUE,
        CL_MAP_WRITE_INVALIDATE_REGION, 0, nMatixSize * sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    fpSumProMergeRtData = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemMergeRtData, CL_TRUE,
        CL_MAP_WRITE_INVALIDATE_REGION, 0, nMatixSize * sizeof(float), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    npSumSize   = (uint *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, mpClmemMatixSumSize, CL_TRUE,
        CL_MAP_WRITE_INVALIDATE_REGION, 0, nBufferSize * sizeof(uint), 0, NULL, NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clEnqueueMapBuffer" );
    clFinish( kEnv.mpkCmdQueue );
    //printf("In CreateBuffer, pptrr is %d,%d,%d\n",fpSrcData,npStartPos,npEndPos);
    return 0;
}

int OclCalc::OclHostArithmeticOperator32Bits( const char* aKernelName, float *fpLeftData, float *fpRightData, double *rResult, int nRowSize )
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    CheckKernelName( &kEnv,aKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, aKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemLeftData, fpLeftData, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemRightData, fpRightData, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
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
    clStatus = clReleaseMemObject(mpClmemLeftData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemRightData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( clResult );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::OclHostFormulaStatistics32Bits(const char* aKernelName,float *fpSrcData,uint *npStartPos,uint *npEndPos,double *output,int size)
{
    cl_int clStatus = 0;
    size_t global_work_size[1];
    CheckKernelName( &kEnv, aKernelName );
    kEnv.mpkKernel = clCreateKernel( kEnv.mpkProgram, kEnv.mckKernelName, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateKernel" );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemSrcData, fpSrcData, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemStartPos, npStartPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, mpClmemEndPos, npEndPos, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    clFinish( kEnv.mpkCmdQueue );
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
    clStatus = clReleaseMemObject( mpClmemSrcData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemStartPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( mpClmemEndPos );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseMemObject( outputCl );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    return 0;
}

int OclCalc::OclHostFormulaCount32Bits( uint *npStartPos, uint *npEndPos, double *dpOutput, int nSize )
{
    const char *cpKernelName = "oclFormulaCount";
    CheckKernelName( &kEnv, cpKernelName );
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
int OclCalc::OclHostFormulaSumProduct32Bits( float *fpSumProMergeLfData, float *fpSumProMergeRrData, uint *npSumSize, double *dpOutput, int nSize )
{
    cl_int clStatus;
    size_t global_work_size[1];
    memset(dpOutput,0,nSize);
    const char *cpFirstKernelName = "oclSignedMul";
    const char *cpSecondKernelName = "oclFormulaSumproduct";
    CheckKernelName( &kEnv, cpFirstKernelName );
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
    CheckKernelName( &kEnv,cpSecondKernelName );
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
    cl_mem xValues = clCreateBuffer( rEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        nElements * sizeof(double), NULL, pStatus);
    double *pValues = (double *)clEnqueueMapBuffer( rEnv.mpkCmdQueue, xValues, CL_TRUE, CL_MAP_WRITE, 0,
        nElements * sizeof(double), 0, NULL, NULL, NULL);
    clFinish(rEnv.mpkCmdQueue);
    memcpy( pValues, _pValues, nElements*sizeof(double) );
    clEnqueueUnmapMemObject( rEnv.mpkCmdQueue, xValues, pValues, 0, NULL, NULL );
    clFinish( rEnv.mpkCmdQueue );
    return xValues;
}

static cl_mem allocateFloatBuffer( KernelEnv &rEnv, const double *_pValues, size_t nElements, cl_int *pStatus )
{
    // Ugh - horrible redundant copying ...
    cl_mem xValues = clCreateBuffer( rEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
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

double *OclCalc::OclSimpleDeltaOperation( OpCode eOp, const double *pOpArray, const double *pSubtractSingle, size_t nElements, double del )
{
    SetKernelEnv( &kEnv );

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
    CheckKernelName( &kEnv, kernelName );

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
            outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nElements * sizeof(double), NULL, &clStatus );
        }
        else
        {
            valuesCl = allocateFloatBuffer( kEnv, pOpArray, nElements, &clStatus );
            subtractCl = allocateFloatBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
            outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nElements * sizeof(float), NULL, &clStatus);
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
             outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nElements * sizeof(double), NULL, &clStatus );
             clStatus = clSetKernelArg( kEnv.mpkKernel, 0, sizeof(cl_double), (void *)&delta );
             CHECK_OPENCL( clStatus, "clSetKernelArg");
        }
        else
       {
             float fTmp = (float)delta;
             subtractCl = allocateFloatBuffer( kEnv, pSubtractSingle, nElements, &clStatus );
             outputCl = clCreateBuffer( kEnv.mpkContext, CL_MEM_READ_WRITE, nElements * sizeof(float), NULL, &clStatus );
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
        double *pOutput = (double *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,outputCl,CL_TRUE,
            CL_MAP_READ,0,nElements*sizeof(double), 0,NULL,NULL,NULL);
        clFinish(kEnv.mpkCmdQueue);
        memcpy( pResult, pOutput, nElements * sizeof(double) );
        clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,outputCl,pResult,0,NULL,NULL);
        clFinish(kEnv.mpkCmdQueue);
    }
    else
    {
        float *pOutput = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, outputCl, CL_TRUE,
            CL_MAP_READ, 0, nElements*sizeof(float), 0, NULL, NULL, NULL );
        clFinish( kEnv.mpkCmdQueue );
        for ( int i = 0; i < (int)nElements; i++ )
            pResult[i] = (double)pOutput[i];
        clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, outputCl, pOutput, 0, NULL, NULL );
        clFinish( kEnv.mpkCmdQueue );
    }

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

int OclCalc::OclHostMatrixInverse64Bits( const char* aKernelName, double *dpOclMatrixSrc, double *dpOclMatrixDst,std::vector<double>&dpResult,  uint nDim )
{
    cl_int clStatus = 0;
    uint nMatrixSize = nDim * nDim;
    size_t global_work_size[1] = { nDim };
    cl_mem clpPData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR ), nMatrixSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clpYData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR ), nMatrixSize * sizeof(double), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    double * dpY = (double *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpYData, CL_TRUE, CL_MAP_WRITE, 0, nMatrixSize * sizeof(double), 0, NULL,NULL, &clStatus );
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
                dpP[i*nDim+j]=1.0;
            else
                dpP[i*nDim+j]=0.0;
        }
    }
    clStatus = clEnqueueUnmapMemObject( kEnv.mpkCmdQueue, clpPData, dpP, 0, NULL, NULL );
    CHECK_OPENCL( clStatus, "clEnqueueUnmapMemObject" );
    CheckKernelName( &kEnv,aKernelName );
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
    clStatus = clReleaseMemObject( mpClmemRightData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseKernel( kernel_solve );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpPData );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpYData );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    return 0;
}

int OclCalc::OclHostMatrixInverse32Bits( const char* aKernelName, float *fpOclMatrixSrc, float *fpOclMatrixDst, std::vector<double>& dpResult, uint nDim )
{
    cl_int clStatus = 0;
    uint nMatrixSize = nDim * nDim;
    size_t global_work_size[1] = { nDim };

    cl_mem clpPData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR ), nMatrixSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    cl_mem clpYData = clCreateBuffer( kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR ), nMatrixSize * sizeof(float), NULL, &clStatus );
    CHECK_OPENCL( clStatus, "clCreateBuffer" );
    float * fpY = (float *)clEnqueueMapBuffer( kEnv.mpkCmdQueue, clpYData, CL_TRUE, CL_MAP_WRITE, 0, nMatrixSize * sizeof(float), 0, NULL,NULL, &clStatus );
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
    CheckKernelName( &kEnv,aKernelName );
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
    clStatus = clReleaseMemObject( mpClmemRightData );
    CHECK_OPENCL( clStatus, "clReleaseMemObject" );
    clStatus = clReleaseKernel( kernel_solve );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpPData );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    clStatus = clReleaseMemObject( clpYData );
    CHECK_OPENCL( clStatus, "clReleaseKernel" );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
