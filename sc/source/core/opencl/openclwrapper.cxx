/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sal/config.h"
#include "random.hxx"
#include "openclwrapper.hxx"
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
HINSTANCE HOpenclDll = NULL;
    void *OpenclDll = NULL;

int OpenclDevice::LoadOpencl()
{
    //fprintf(stderr, " LoadOpenclDllxx... \n");
    OpenclDll = static_cast<HINSTANCE>(HOpenclDll);
    OpenclDll = LoadLibrary(OPENCL_DLL_NAME);
    if (!static_cast<HINSTANCE>(OpenclDll))
    {
        fprintf(stderr, " Load opencllo.dll failed! \n");
        FreeLibrary(static_cast<HINSTANCE>(OpenclDll));
        return OCLERR;
    }
    fprintf(stderr, " Load opencllo.dll successfully!\n");
    return OCLSUCCESS;
}

void OpenclDevice::FreeOpenclDll()
{
    fprintf(stderr, " Free opencllo.dll ... \n");
    if(!static_cast<HINSTANCE>(OpenclDll))
        FreeLibrary(static_cast<HINSTANCE>(OpenclDll));
}
#endif

int OpenclDevice::InitEnv()
{
#ifdef SAL_WIN32
    while(1)
    {
        if(1==LoadOpencl())
        break;
    }
#endif
    InitOpenclRunEnv(0,NULL);
    return 1;
}

int OpenclDevice::ReleaseOpenclRunEnv() {
    ReleaseOpenclEnv(&gpuEnv);
#ifdef SAL_WIN32
    FreeOpenclDll();
#endif
    return 1;
}
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
inline int OpenclDevice::AddKernelConfig(int kCount, const char *kName)
{
    strcpy(gpuEnv.mArrykernelNames[kCount], kName);
    gpuEnv.mnKernelCount++;
    return 0;
}

int OpenclDevice::RegistOpenclKernel()
{
    if (!gpuEnv.mnIsUserCreated)
        memset(&gpuEnv, 0, sizeof(gpuEnv));

    gpuEnv.mnFileCount = 0; //argc;
    gpuEnv.mnKernelCount = 0UL;

    AddKernelConfig(0, (const char*) "hello");
    AddKernelConfig(1, (const char*) "oclformula");
    AddKernelConfig(2, (const char*) "oclFormulaMin");
    AddKernelConfig(3, (const char*) "oclFormulaMax");
    AddKernelConfig(4, (const char*) "oclFormulaSum");
    AddKernelConfig(5, (const char*) "oclFormulaCount");
    AddKernelConfig(6, (const char*) "oclFormulaAverage");
    AddKernelConfig(7, (const char*) "oclFormulaSumproduct");
    AddKernelConfig(8, (const char*) "oclFormulaMinverse");

    AddKernelConfig(9,  (const char*) "oclSignedAdd");
    AddKernelConfig(10, (const char*) "oclSignedSub");
    AddKernelConfig(11, (const char*) "oclSignedMul");
    AddKernelConfig(12, (const char*) "oclSignedDiv");
    AddKernelConfig(13, (const char*) "oclAverageDelta");
    AddKernelConfig(14, (const char*) "OclMaxDelta");
    AddKernelConfig(15, (const char*) "OclMinDelta");

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

int OpenclDevice::SetKernelEnv(KernelEnv *envInfo)
{
    envInfo->mpkContext  = gpuEnv.mpContext;
    envInfo->mpkCmdQueue = gpuEnv.mpCmdQueue;
    envInfo->mpkProgram  = gpuEnv.mpArryPrograms[0];

    return 1;
}

int OpenclDevice::CheckKernelName(KernelEnv *envInfo,const char *kernelName)
{
    //printf("CheckKernelName,total count of kernels...%d\n", gpuEnv.kernelCount);
    int kCount;
    for(kCount=0; kCount < gpuEnv.mnKernelCount; kCount++) {
        if(strcasecmp(kernelName, gpuEnv.mArrykernelNames[kCount]) == 0) {
            printf("match %s kernel right\n",kernelName);
            break;
        }
    }
    envInfo->mpkKernel = gpuEnv.mpArryKernels[kCount];
    strcpy(envInfo->mckKernelName, kernelName);
    if (envInfo == (KernelEnv *) NULL)
    {
        printf("get err func and env\n");
        return 0;
    }
    return 1;
}

int OpenclDevice::ConvertToString(const char *filename, char **source)
{
    int file_size;
    size_t result;
    FILE *file = NULL;
    file_size = 0;
    result = 0;
    file = fopen(filename, "rb+");
    printf("open kernel file %s.\n",filename);

    if (file != NULL) {
        printf("Open ok!\n");
        fseek(file, 0, SEEK_END);

        file_size = ftell(file);
        rewind(file);
        *source = (char*) malloc(file_size + 1);
        if (*source == (char*) NULL) {
            return 0;
        }
        result = fread(*source, 1, file_size, file);
        if (result != (size_t) file_size) {
            free(*source);
            return 0;
        }
        (*source)[file_size] = '\0';
        fclose(file);

        return 1;
    }
    printf("open kernel file failed.\n");
    return 0;
}

int OpenclDevice::BinaryGenerated(const char * clFileName, FILE ** fhandle)
{
    unsigned int i = 0;
    cl_int status;
    char *str = NULL;
    FILE *fd = NULL;
    cl_uint numDevices=0;
    status = clGetDeviceIDs(gpuEnv.mpPlatformID, // platform
                            CL_DEVICE_TYPE_ALL, // device_type
                            0, // num_entries
                            NULL, // devices ID
                            &numDevices);
    for (i = 0; i <numDevices; i++) {
        char fileName[256] = { 0 }, cl_name[128] = { 0 };
        if (gpuEnv.mpArryDevsID[i] != 0) {
            char deviceName[1024];
            status = clGetDeviceInfo(gpuEnv.mpArryDevsID[i], CL_DEVICE_NAME,sizeof(deviceName), deviceName, NULL);
            CHECK_OPENCL(status);
            str = (char*) strstr(clFileName, (char*) ".cl");
            memcpy(cl_name, clFileName, str - clFileName);
            cl_name[str - clFileName] = '\0';
            sprintf(fileName, "./%s-%s.bin", cl_name, deviceName);
            fd = fopen(fileName, "rb");
            status = (fd != NULL) ? 1 : 0;
            }
        }
        if (fd != NULL) {
            *fhandle = fd;
            }

        return status;

}

int OpenclDevice::WriteBinaryToFile(const char* fileName, const char* birary,
        size_t numBytes)
{
    FILE *output = NULL;
    output = fopen(fileName, "wb");
    if (output == NULL) {
        return 0;
    }

    fwrite(birary, 1, numBytes, output);
    fclose(output);

    return 1;

}

int OpenclDevice::GeneratBinFromKernelSource(cl_program program,
                                             const char * clFileName)
{
     unsigned int i = 0;
    cl_int status;
    size_t *binarySizes;
    cl_uint numDevices;
    cl_device_id *mpArryDevsID;
    char **binaries, *str = NULL;

    status = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES,
            sizeof(numDevices), &numDevices, NULL);
    CHECK_OPENCL(status)

    mpArryDevsID = (cl_device_id*) malloc(sizeof(cl_device_id) * numDevices);
    if (mpArryDevsID == NULL) {
        return 0;
    }
    /* grab the handles to all of the devices in the program. */
    status = clGetProgramInfo(program, CL_PROGRAM_DEVICES,
            sizeof(cl_device_id) * numDevices, mpArryDevsID, NULL);
    CHECK_OPENCL(status)

    /* figure out the sizes of each of the binaries. */
    binarySizes = (size_t*) malloc(sizeof(size_t) * numDevices);

    status = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES,
            sizeof(size_t) * numDevices, binarySizes, NULL);
    CHECK_OPENCL(status)

    /* copy over all of the generated binaries. */
    binaries = (char**) malloc(sizeof(char *) * numDevices);
    if (binaries == NULL) {
        return 0;
    }

    for (i = 0; i < numDevices; i++) {
        if (binarySizes[i] != 0) {
            binaries[i] = (char*) malloc(binarySizes[i]);
            if (binaries[i] == NULL) {
                return 0;
            }
        } else {
            binaries[i] = NULL;
        }
    }

    status = clGetProgramInfo(program, CL_PROGRAM_BINARIES,
            sizeof(char *) * numDevices, binaries, NULL);
    CHECK_OPENCL(status)

    /* dump out each binary into its own separate file. */
    for (i = 0; i < numDevices; i++) {
        char fileName[256] = { 0 }, cl_name[128] = { 0 };

        if (binarySizes[i] != 0) {
            char deviceName[1024];
            status = clGetDeviceInfo(mpArryDevsID[i], CL_DEVICE_NAME,
                    sizeof(deviceName), deviceName, NULL);
            CHECK_OPENCL(status)

            str = (char*) strstr(clFileName, (char*) ".cl");
            memcpy(cl_name, clFileName, str - clFileName);
            cl_name[str - clFileName] = '\0';
            sprintf(fileName, "./%s-%s.bin", cl_name, deviceName);

            if (!WriteBinaryToFile(fileName, binaries[i], binarySizes[i])) {
                printf("opencl-wrapper: write binary[%s] failds\n", fileName);
                return 0;
            } //else
            printf("opencl-wrapper: write binary[%s] succesfully\n", fileName);
        }
    }

    // Release all resouces and memory
    for (i = 0; i < numDevices; i++) {
        if (binaries[i] != NULL) {
            free(binaries[i]);
            binaries[i] = NULL;
        }
    }

    if (binaries != NULL) {
        free(binaries);
        binaries = NULL;
    }

    if (binarySizes != NULL) {
        free(binarySizes);
        binarySizes = NULL;
    }

    if (mpArryDevsID != NULL) {
        free(mpArryDevsID);
        mpArryDevsID = NULL;
    }
    return 1;
}

int OpenclDevice::InitOpenclAttr(OpenCLEnv * env)
{
    if (gpuEnv.mnIsUserCreated)
        return 1;

    gpuEnv.mpContext    = env->mpOclContext;
    gpuEnv.mpPlatformID = env->mpOclPlatformID;
    gpuEnv.mpDevID        = env->mpOclDevsID;
    gpuEnv.mpCmdQueue    = env->mpOclCmdQueue;

    gpuEnv.mnIsUserCreated = 1;

    return 0;
}

int OpenclDevice::CreateKernel(char * kernelname, KernelEnv * env)
{
    int status;

    env->mpkKernel   = clCreateKernel(gpuEnv.mpArryPrograms[0], kernelname, &status);
    env->mpkContext  = gpuEnv.mpContext;
    env->mpkCmdQueue = gpuEnv.mpCmdQueue;
    return status != CL_SUCCESS ? 1 : 0;
}

int OpenclDevice::ReleaseKernel(KernelEnv * env)
{
    int status = clReleaseKernel(env->mpkKernel);
    return status != CL_SUCCESS ? 1 : 0;
}

int OpenclDevice::ReleaseOpenclEnv(GPUEnv *gpuInfo)
{
    int i = 0;
    int status = 0;

    if (!isInited) {
        return 1;
    }

    for (i = 0; i < gpuEnv.mnFileCount; i++) {
        if (gpuEnv.mpArryPrograms[i]) {
            status = clReleaseProgram(gpuEnv.mpArryPrograms[i]);
            CHECK_OPENCL(status)
            gpuEnv.mpArryPrograms[i] = NULL;
        }
    }
    if (gpuEnv.mpCmdQueue) {
        clReleaseCommandQueue(gpuEnv.mpCmdQueue);
        gpuEnv.mpCmdQueue = NULL;
    }
    if (gpuEnv.mpContext) {
        clReleaseContext(gpuEnv.mpContext);
        gpuEnv.mpContext = NULL;
    }
    isInited = 0;
    gpuInfo->mnIsUserCreated = 0;
    free(gpuInfo->mpArryDevsID);
    return 1;
}

int OpenclDevice::RunKernelWrapper(cl_kernel_function function,
                                   const char * kernelName, void **usrdata)
{
    printf("oclwrapper:RunKernel_wrapper...\n");
    if (RegisterKernelWrapper(kernelName, function) != 1) {
        fprintf(stderr,
                "Error:RunKernel_wrapper:RegisterKernelWrapper fail!\n");
        return -1;
    }
    return (RunKernel(kernelName, usrdata));
}

int OpenclDevice::CachedOfKernerPrg(const GPUEnv *gpuEnvCached,
                                    const char * clFileName)
{
    int i;
    for (i = 0; i < gpuEnvCached->mnFileCount; i++) {
        if (strcasecmp(gpuEnvCached->mArryKnelSrcFile[i], clFileName) == 0) {
            if (gpuEnvCached->mpArryPrograms[i] != NULL) {
                return 1;
            }
        }
    }

    return 0;
}

int OpenclDevice::CompileKernelFile(GPUEnv *gpuInfo, const char *buildOption) {
    cl_int status;
    size_t length;
    char *buildLog = NULL, *binary;
    const char *source;
    size_t source_size[1];
    int b_error, binary_status, binaryExisted, idx;
    cl_uint numDevices;
    cl_device_id *mpArryDevsID;
    FILE *fd, *fd1;
    const char* filename = "kernel.cl";
    fprintf(stderr, "CompileKernelFile ... \n");
    if (CachedOfKernerPrg(gpuInfo, filename) == 1) {
        return 1;
    }

    idx = gpuInfo->mnFileCount;

    source = kernel_src;

    source_size[0] = strlen(source);
    binaryExisted = 0;
    if ((binaryExisted = BinaryGenerated(filename, &fd)) == 1) {
        status = clGetContextInfo(gpuInfo->mpContext, CL_CONTEXT_NUM_DEVICES,
                sizeof(numDevices), &numDevices, NULL);
        CHECK_OPENCL(status)

        mpArryDevsID = (cl_device_id*) malloc(sizeof(cl_device_id) * numDevices);
        if (mpArryDevsID == NULL) {
            return 0;
        }

        b_error = 0;
        length = 0;
        b_error |= fseek(fd, 0, SEEK_END) < 0;
        b_error |= (length = ftell(fd)) <= 0;
        b_error |= fseek(fd, 0, SEEK_SET) < 0;
        if (b_error) {
            return 0;
        }

        binary = (char*) malloc(length);
        if (!binary) {
            return 0;
        }

        memset(binary, 0, length);
        b_error |= fread(binary, 1, length, fd) != length;

        fclose(fd);
        fd = NULL;
        // grab the handles to all of the devices in the context.
        status = clGetContextInfo(gpuInfo->mpContext, CL_CONTEXT_DEVICES,
                sizeof(cl_device_id) * numDevices, mpArryDevsID, NULL);
        CHECK_OPENCL(status)

        gpuInfo->mpArryPrograms[idx] = clCreateProgramWithBinary(gpuInfo->mpContext,
                numDevices, mpArryDevsID, &length, (const unsigned char**) &binary,
                &binary_status, &status);
        CHECK_OPENCL(status)

        free(binary);
        free(mpArryDevsID);
        mpArryDevsID = NULL;
    } else {
        // create a CL program using the kernel source
        gpuEnv.mpArryPrograms[idx] = clCreateProgramWithSource(gpuEnv.mpContext,
                1, &source, source_size, &status);
        CHECK_OPENCL(status);
    }

    if (gpuInfo->mpArryPrograms[idx] == (cl_program) NULL) {
        return 0;
    }

    //char options[512];
    // create a cl program executable for all the devices specified
    printf("BuildProgram.\n");
    if (!gpuInfo->mnIsUserCreated) {
        status = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, gpuInfo->mpArryDevsID,
                buildOption, NULL, NULL);
    } else {
        status = clBuildProgram(gpuInfo->mpArryPrograms[idx], 1, &(gpuInfo->mpDevID),
                buildOption, NULL, NULL);
    }

    if (status != CL_SUCCESS) {
        printf ("BuildProgram error!\n");
        if (!gpuInfo->mnIsUserCreated) {
            status = clGetProgramBuildInfo(gpuInfo->mpArryPrograms[idx],
                    gpuInfo->mpArryDevsID[0], CL_PROGRAM_BUILD_LOG, 0, NULL,
                    &length);
        } else {
            status = clGetProgramBuildInfo(gpuInfo->mpArryPrograms[idx],
                    gpuInfo->mpDevID, CL_PROGRAM_BUILD_LOG, 0, NULL, &length);
        }
        if (status != CL_SUCCESS) {
            printf("opencl create build log fail\n");
            return 0;
        }
        buildLog = (char*) malloc(length);
        if (buildLog == (char*) NULL) {
            return 0;
        }
        if (!gpuInfo->mnIsUserCreated) {
            status = clGetProgramBuildInfo(gpuInfo->mpArryPrograms[idx],
                    gpuInfo->mpArryDevsID[0], CL_PROGRAM_BUILD_LOG, length,
                    buildLog, &length);
        } else {
            status = clGetProgramBuildInfo(gpuInfo->mpArryPrograms[idx],
                    gpuInfo->mpDevID, CL_PROGRAM_BUILD_LOG, length, buildLog,
                    &length);
        }

        fd1 = fopen("kernel-build.log", "w+");
        if (fd1 != NULL) {
            fwrite(buildLog, 1, length, fd1);
            fclose(fd1);
        }

        free(buildLog);
        return 0;
    }

    strcpy(gpuEnv.mArryKnelSrcFile[idx], filename);

    if (binaryExisted == 0)
        GeneratBinFromKernelSource(gpuEnv.mpArryPrograms[idx], filename);

    gpuInfo->mnFileCount += 1;

    return 1;


}
int OpenclDevice::GetKernelEnvAndFunc(const char *kernelName,
        KernelEnv *env, cl_kernel_function *function) {
    int i; //,program_idx ;
    //printf("----------------OpenclDevice::GetKernelEnvAndFunc\n");
    for (i = 0; i < gpuEnv.mnKernelCount; i++) {
        if (strcasecmp(kernelName, gpuEnv.mArrykernelNames[i]) == 0) {
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

int OpenclDevice::RunKernel(const char *kernelName, void **userdata) {
    KernelEnv kEnv;

    cl_kernel_function function;

    int status;

    memset(&kEnv, 0, sizeof(KernelEnv));
    status = GetKernelEnvAndFunc(kernelName, &kEnv, &function);
    strcpy(kEnv.mckKernelName, kernelName);
    if (status == 1) {
        if (&kEnv == (KernelEnv *) NULL
                || &function == (cl_kernel_function *) NULL) {
            return 0;
        }
        return (function(userdata, &kEnv));
    }
    return 0;
}

int OpenclDevice::InitOpenclRunEnv(int argc, const char *buildOptionKernelfiles)
{
    int status = 0;
    if (MAX_CLKERNEL_NUM <= 0) {
        return 1;
    }
    if ((argc > MAX_CLFILE_NUM) || (argc < 0)) {
        return 1;
    }

    if (!isInited) {
        RegistOpenclKernel();
        //initialize devices, context, comand_queue
        status = InitOpenclRunEnv(&gpuEnv);
        if (status) {
            printf("init_opencl_env failed.\n");
            return 1;
        }
        printf("init_opencl_env successed.\n");
        //initialize program, kernelName, kernelCount
        status = CompileKernelFile( &gpuEnv, buildOptionKernelfiles);
        if (status == 0 || gpuEnv.mnKernelCount == 0) {
            printf("CompileKernelFile failed.\n");
            return 1;
        }
        printf("CompileKernelFile successed.\n");
        isInited = 1;
    }
    return 0;
}

int OpenclDevice::InitOpenclRunEnv(GPUEnv *gpuInfo)
{
    size_t length;
    cl_int status;
    cl_uint numPlatforms, numDevices;
    cl_platform_id *platforms;
    cl_context_properties cps[3];
    char platformName[100];
    unsigned int i;

    // Have a look at the available platforms.

    if (!gpuInfo->mnIsUserCreated) {
        status = clGetPlatformIDs(0, NULL, &numPlatforms);
        if (status != CL_SUCCESS) {
            return 1;
        }
        gpuInfo->mpPlatformID = NULL;

        if (0 < numPlatforms) {
            platforms = (cl_platform_id*) malloc(
                    numPlatforms * sizeof(cl_platform_id));
            if (platforms == (cl_platform_id*) NULL) {
                return 1;
            }
            status = clGetPlatformIDs(numPlatforms, platforms, NULL);

            if (status != CL_SUCCESS) {
                return 1;
            }

            for (i = 0; i < numPlatforms; i++) {
                status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR,
                        sizeof(platformName), platformName, NULL);

                if (status != CL_SUCCESS) {
                    return 1;
                }
                gpuInfo->mpPlatformID = platforms[i];

                //if (!strcmp(platformName, "Intel(R) Coporation"))
                //if( !strcmp( platformName, "Advanced Micro Devices, Inc." ))
                {
                    gpuInfo->mpPlatformID = platforms[i];

                    status = clGetDeviceIDs(gpuInfo->mpPlatformID, // platform
                                            CL_DEVICE_TYPE_ALL,    // device_type
                                            0,                       // num_entries
                                            NULL,                   // devices
                                            &numDevices);

                    if (status != CL_SUCCESS) {
                        continue;
                    }

                    if (numDevices) {
                        break;
                    }
                }
            }
            if(status!=CL_SUCCESS)
                return 1;
            free(platforms);
        }
        if (NULL == gpuInfo->mpPlatformID) {
            return 1;
        }

        // Use available platform.

        cps[0] = CL_CONTEXT_PLATFORM;
        cps[1] = (cl_context_properties) gpuInfo->mpPlatformID;
        cps[2] = 0;
        // Check for GPU.
        gpuInfo->mDevType = CL_DEVICE_TYPE_GPU;
        gpuInfo->mpContext = clCreateContextFromType(cps, gpuInfo->mDevType, NULL,
                NULL, &status);

        // If no GPU, check for CPU.
        if ((gpuInfo->mpContext == (cl_context) NULL)
                || (status != CL_SUCCESS)) {
            gpuInfo->mDevType = CL_DEVICE_TYPE_CPU;
            gpuInfo->mpContext = clCreateContextFromType(cps, gpuInfo->mDevType,
                    NULL, NULL, &status);
        }

        // If no GPU or CPU, check for a "default" type.
        if ((gpuInfo->mpContext == (cl_context) NULL)
                || (status != CL_SUCCESS)) {
            gpuInfo->mDevType = CL_DEVICE_TYPE_DEFAULT;
            gpuInfo->mpContext = clCreateContextFromType(cps, gpuInfo->mDevType,
                    NULL, NULL, &status);
        }
        if ((gpuInfo->mpContext == (cl_context) NULL)
                || (status != CL_SUCCESS)) {
            return 1;
        }
        // Detect OpenCL devices.
        // First, get the size of device list data
        status = clGetContextInfo(gpuInfo->mpContext, CL_CONTEXT_DEVICES, 0,
                NULL, &length);
        if ((status != CL_SUCCESS) || (length == 0)) {
            return 1;
        }
        // Now allocate memory for device list based on the size we got earlier
        gpuInfo->mpArryDevsID = (cl_device_id*) malloc(length);
        if (gpuInfo->mpArryDevsID == (cl_device_id*) NULL) {
            return 1;
        }
        // Now, get the device list data
        status = clGetContextInfo(gpuInfo->mpContext, CL_CONTEXT_DEVICES, length,
                gpuInfo->mpArryDevsID, NULL);
        if (status != CL_SUCCESS) {
            return 1;
        }

        // Create OpenCL command queue.
        gpuInfo->mpCmdQueue = clCreateCommandQueue(gpuInfo->mpContext,
                gpuInfo->mpArryDevsID[0], 0, &status);

        if (status != CL_SUCCESS) {
            return 1;
        }
    }

    return 0;

}
int OpenclDevice::RegisterKernelWrapper(const char *kernelName,cl_kernel_function function)
{
    int i;
    //printf("oclwrapper:RegisterKernelWrapper...%d\n", gpuEnv.mnKernelCount);
    for (i = 0; i < gpuEnv.mnKernelCount; i++)
    {
        if (strcasecmp(kernelName, gpuEnv.mArrykernelNames[i]) == 0)
        {
            gpuEnv.mpArryKnelFuncs[i] = function;
            return 1;
        }
    }
    return 0;
}


void OpenclDevice::SetOpenclState(int state)
{
     //printf("OpenclDevice::setOpenclState...\n");
     isInited = state;
}

int OpenclDevice::GetOpenclState()
{
    return isInited;
}
//ocldbg

int OclFormulax(void ** usrdata, KernelEnv *env) {
    fprintf(stderr, "In OpenclDevice,...Formula_proc\n");
    cl_int clStatus;
    int status;
    size_t global_work_size[1];
    float tdata[NUM];

    double *data = (double *) usrdata[0];
    const formulax type = *((const formulax *) usrdata[1]);
    double ret = 0.0;

    for (int i = 0; i < NUM; i++) {
        tdata[i] = (float) data[i];
    }

    env->mpkKernel = clCreateKernel(env->mpkProgram, "oclformula", &clStatus);
    //printf("ScInterpreter::IterateParameters...after clCreateKernel.\n");
    //fprintf(stderr, "\nIn OpenclDevice,...after clCreateKernel\n");
    int size = NUM;

    cl_mem formula_data = clCreateBuffer(env->mpkContext,
            (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR),
            size * sizeof(float), (void *) tdata, &clStatus);
    //fprintf(stderr, "\nIn OpenclDevice,...after clCreateBuffer\n");

    status = clSetKernelArg(env->mpkKernel, 0, sizeof(cl_mem),
            (void *) &formula_data);
    CHECK_OPENCL(status)
    status = clSetKernelArg(env->mpkKernel, 1, sizeof(unsigned int),
            (void *) &type);
    CHECK_OPENCL(status)

    global_work_size[0] = size;
    //fprintf(stderr, "\nIn OpenclDevice,...after global_work_size\n");
    //PPAStartCpuEvent(ppa_proc);

    while (global_work_size[0] != 1) {
        global_work_size[0] = global_work_size[0] / 2;
        status = clEnqueueNDRangeKernel(env->mpkCmdQueue, env->mpkKernel, 1,
                NULL, global_work_size, NULL, 0, NULL, NULL);
        CHECK_OPENCL(status)

    }
    //fprintf(stderr, "\nIn OpenclDevice,...before clEnqueueReadBuffer\n");
    status = clEnqueueReadBuffer(env->mpkCmdQueue, formula_data, CL_FALSE, 0,
            sizeof(float), (void *) &tdata, 0, NULL, NULL);
    CHECK_OPENCL(status)
    status = clFinish(env->mpkCmdQueue);
    CHECK_OPENCL(status)

    //PPAStopCpuEvent(ppa_proc);
    //fprintf(stderr, "\nIn OpenclDevice,...before clReleaseKernel\n");
    status = clReleaseKernel(env->mpkKernel);
    CHECK_OPENCL(status)
    status = clReleaseMemObject(formula_data);
    CHECK_OPENCL(status)

    if (type == AVG)
        ret = (double) tdata[0] / NUM;
    else
        ret = (double) tdata[0];

    printf("size = %d ret = %f.\n\n", NUM, ret);

    return 0;
}

int OclFormulaxDll(void ** usrdata, KernelEnv *env) {

    fprintf(stderr, "In OclFormulaxDll...\n");
    cl_int clStatus;
    int status;
    size_t global_work_size[1];
    float tdata[NUM];

    double *data = (double *) usrdata[0];
    const formulax type = *((const formulax *) usrdata[1]);
    double ret = 0.0;

    for (int i = 0; i < NUM; i++) {
        tdata[i] = (float) data[i];
    }

    env->mpkKernel = clCreateKernel(env->mpkProgram, "oclformula", &clStatus);
    //printf("ScInterpreter::IterateParameters...after clCreateKernel.\n");
    //fprintf(stderr, "\nIn OpenclDevice,...after clCreateKernel\n");
    int size = NUM;

    cl_mem formula_data = clCreateBuffer(env->mpkContext,
            (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR),
            size * sizeof(float), (void *) tdata, &clStatus);
    //fprintf(stderr, "\nIn OpenclDevice,...after clCreateBuffer\n");

    status = clSetKernelArg(env->mpkKernel, 0, sizeof(cl_mem),
            (void *) &formula_data);
    CHECK_OPENCL(status)
    status = clSetKernelArg(env->mpkKernel, 1, sizeof(unsigned int),
            (void *) &type);
    CHECK_OPENCL(status)

    global_work_size[0] = size;
    //fprintf(stderr, "\nIn OpenclDevice,...after global_work_size\n");
    //PPAStartCpuEvent(ppa_proc);

    while (global_work_size[0] != 1) {
        global_work_size[0] = global_work_size[0] / 2;
        status = clEnqueueNDRangeKernel(env->mpkCmdQueue, env->mpkKernel, 1,
                NULL, global_work_size, NULL, 0, NULL, NULL);
        CHECK_OPENCL(status)

    }
    //fprintf(stderr, "\nIn OpenclDevice,...before clEnqueueReadBuffer\n");
    status = clEnqueueReadBuffer(env->mpkCmdQueue, formula_data, CL_FALSE, 0,
            sizeof(float), (void *) &tdata, 0, NULL, NULL);
    CHECK_OPENCL(status)
    status = clFinish(env->mpkCmdQueue);
    CHECK_OPENCL(status)

    //PPAStopCpuEvent(ppa_proc);
    //fprintf(stderr, "\nIn OpenclDevice,...before clReleaseKernel\n");
    status = clReleaseKernel(env->mpkKernel);
    CHECK_OPENCL(status)
    status = clReleaseMemObject(formula_data);
    CHECK_OPENCL(status)

    if (type == AVG)
        ret = (double) tdata[0] / NUM;
    else
        ret = (double) tdata[0];

    printf("OclFormulaxDllxx:size = %d ret = %f.\n\n", NUM, ret);

    return 0;
}

double OclCalc::OclProcess(cl_kernel_function function, double *data, formulax type)
{
    fprintf(stderr, "\n OpenclDevice, proc...begin\n");
    double ret = 0;
    void *usrdata[2];
    usrdata[0] = (void *) data;
    usrdata[1] = (void *) &type;
    RunKernelWrapper(function, "oclformula", usrdata);
    return ret;
}

double OclCalc::OclTest() {
    double data[NUM];

    for (int i = 0; i < NUM; i++) {
        data[i] = sc::rng::uniform();
        fprintf(stderr, "%f\t", data[i]);
    }
    OclProcess(&OclFormulax, data, AVG);
    return 0.0;
}

double OclCalc::OclTestDll() {
    double data[NUM];

    for (int i = 0; i < NUM; i++) {
        data[i] = sc::rng::uniform();
        fprintf(stderr, "%f\t", data[i]);
    }
    OclProcess(&OclFormulaxDll, data, AVG);
    return 0.0;
}

OclCalc::OclCalc()
{
    fprintf(stderr,"OclCalc:: init opencl ...\n");
}

OclCalc::~OclCalc()
{
    fprintf(stderr,"OclCalc:: opencl end ...\n");
}

/////////////////////////////////////////////////////////////////////////////
#ifdef GPU_64BITS
int OclCalc::OclHostFormulaMax(double *srcData,int *start,int *end,double *output,int size)
{
    KernelEnv kEnv;
    const char *kernelName = "oclFormulaMax";
    CheckKernelName(&kEnv,kernelName);
    cl_int clStatus;
    size_t global_work_size[1];
    int alignSize = size + end[0]-start[0];

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kernelName, &clStatus);
    cl_int ret=0;
    cl_mem inputCl = clCreateBuffer(kEnv.mpkContext,(cl_mem_flags) (CL_MEM_READ_WRITE),
        alignSize * sizeof(float), NULL, &clStatus);
    cl_mem startCl = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        size * sizeof(unsigned int), NULL, &ret);
    cl_mem endCl = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        size * sizeof(unsigned int), NULL, &ret);
    cl_mem outputCl = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
    size* sizeof(float), NULL, &ret);

    float * hostMapSrc = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,inputCl,CL_TRUE,CL_MAP_WRITE,0,alignSize * sizeof(float),0,NULL,NULL,NULL);
    int * hostMapStart = (int *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,startCl,CL_TRUE,CL_MAP_WRITE,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
    int * hostMapEnd   = (int *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,endCl,CL_TRUE,CL_MAP_WRITE,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
    for(int i=0;i<size;i++)
    {
        hostMapStart[i] = start[i];
        hostMapEnd[i]    = end[i];
    }
    for(int i=0;i<alignSize;i++)
        hostMapSrc[i] = (float)srcData[i];
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,inputCl,hostMapSrc,0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,startCl,hostMapStart,0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,endCl,hostMapEnd,0,NULL,NULL);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&inputCl);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&startCl);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&endCl);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 3, sizeof(cl_mem),
        (void *)&outputCl);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    float * outPutMap = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,outputCl,CL_TRUE,CL_MAP_READ,0,size*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<size;i++)
        output[i]=outPutMap[i];

    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,outputCl,outPutMap,0,NULL,NULL);
    clStatus = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(inputCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(startCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(endCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(outputCl);
    CHECK_OPENCL(clStatus);
    return 0;
}

int OclCalc::OclHostFormulaMin(double *srcData,int *start,int *end,double *output,int size)
{
    KernelEnv kEnv;
    const char *kernelName = "oclFormulaMin";
    CheckKernelName(&kEnv,kernelName);
    cl_int clStatus;
    size_t global_work_size[1];
    int alignSize = size + end[0]-start[0];

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kernelName, &clStatus);
    cl_int ret=0;
    cl_mem inputCl = clCreateBuffer(kEnv.mpkContext,(cl_mem_flags) (CL_MEM_READ_WRITE),
        alignSize * sizeof(float), NULL, &clStatus);
    cl_mem startCl = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        size * sizeof(unsigned int), NULL, &ret);
    cl_mem endCl = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
        size * sizeof(unsigned int), NULL, &ret);
    cl_mem outputCl = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
    size* sizeof(float), NULL, &ret);

    float * hostMapSrc = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,inputCl,CL_TRUE,CL_MAP_WRITE,0,alignSize * sizeof(float),0,NULL,NULL,NULL);
    int * hostMapStart = (int *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,startCl,CL_TRUE,CL_MAP_WRITE,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
    int * hostMapEnd   = (int *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,endCl,CL_TRUE,CL_MAP_WRITE,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
    for(int i=0;i<size;i++)
    {
        hostMapStart[i] = start[i];
        hostMapEnd[i]    = end[i];
    }
    for(int i=0;i<alignSize;i++)
        hostMapSrc[i] = (float)srcData[i];
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,inputCl,hostMapSrc,0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,startCl,hostMapStart,0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,endCl,hostMapEnd,0,NULL,NULL);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&inputCl);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&startCl);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&endCl);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 3, sizeof(cl_mem),
        (void *)&outputCl);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    float * outPutMap = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,outputCl,CL_TRUE,CL_MAP_READ,0,size*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<size;i++)
        output[i]=outPutMap[i];

    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,outputCl,outPutMap,0,NULL,NULL);
    clStatus = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(inputCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(startCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(endCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(outputCl);
    CHECK_OPENCL(clStatus);
    return 0;
}
int OclCalc::OclHostFormulaAverage(double *srcData,int *start,int *end,double *output,int size)
{
    KernelEnv kEnv;
    const char *kernelName = "oclFormulaAverage";
    CheckKernelName(&kEnv,kernelName);
    cl_int clStatus;
    size_t global_work_size[1];
    int alignSize = size + end[0]-start[0];

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kernelName, &clStatus);
    cl_int ret=0;
    cl_mem inputCl    = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                                     alignSize * sizeof(float), NULL, &clStatus);
    cl_mem startCl    = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                                     size * sizeof(unsigned int), NULL, &ret);
    cl_mem endCl    = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                                     size * sizeof(unsigned int), NULL, &ret);
    cl_mem outputCl = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE),
                                     size* sizeof(float), NULL, &ret);

    float * hostMapSrc = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,inputCl,CL_TRUE,OPENCLWRAPPER_CL_MAP_WRITE_FLAG,0,alignSize * sizeof(float),0,NULL,NULL,NULL);
    int * hostMapStart = (int *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,startCl,CL_TRUE,OPENCLWRAPPER_CL_MAP_WRITE_FLAG,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
    int * hostMapEnd   = (int *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,endCl,CL_TRUE,OPENCLWRAPPER_CL_MAP_WRITE_FLAG,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
#if 1
    for(int i=0;i<size;i++)
    {
        hostMapStart[i] = start[i];
        hostMapEnd[i]    = end[i];
    }
    for(int i=0;i<alignSize;i++)
        hostMapSrc[i] = (float)srcData[i];
    //memcpy(hostMapSrc,srcData,alignSize * sizeof(float));
#endif
    for(sal_Int32 i = 0; i < alignSize; ++i){//dbg
                       fprintf(stderr,"In avg host,hostMapSrc[%d] is ...%f\n",i,hostMapSrc[i]);
                   }


    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,inputCl,hostMapSrc,  0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,startCl,hostMapStart,0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,endCl,  hostMapEnd,  0,NULL,NULL);



    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&inputCl);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&startCl);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&endCl);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 3, sizeof(cl_mem),
        (void *)&outputCl);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    float * outPutMap = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,outputCl,CL_TRUE,CL_MAP_READ,0,size*sizeof(float),0,NULL,NULL,NULL);

    for(int i=0;i<size;i++){
       //fprintf(stderr,"In avg host,outPutMap[%d] is ...%f\n",i,outPutMap[i]);
        output[i]=outPutMap[i];
    }

    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,outputCl,outPutMap,0,NULL,NULL);
    clStatus = clFinish(kEnv.mpkCmdQueue);

    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(inputCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(startCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(endCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(outputCl);
    CHECK_OPENCL(clStatus);
    return 0;


}

int OclCalc::OclHostSignedAdd(double *lData,double *rData,double *rResult,int dSize) {

    KernelEnv kEnv;
    int status;
    const char *kernelName = "oclSignedAdd";
    CheckKernelName(&kEnv,kernelName);


    cl_int clStatus;
    size_t global_work_size[1];

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kernelName, &clStatus);
    cl_mem clLiftData = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);
    cl_mem clRightData = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);
    cl_mem clResult = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);

    float * hostMapLeftData     = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clLiftData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    float * hostMapRightData    = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clRightData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<dSize;i++)
    {
        hostMapLeftData[i]    = (float)lData[i];
        hostMapRightData[i] = (float)rData[i];
    }
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clLiftData,hostMapLeftData,0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clRightData,hostMapRightData,0,NULL,NULL);

    status = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&clLiftData);
    status = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&clRightData);
    status = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&clResult);
    CHECK_OPENCL(status)
    global_work_size[0] = dSize;
    status = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(status);

    float * hostMapResult = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clResult,CL_TRUE,CL_MAP_READ,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<dSize;i++)
        rResult[i]=hostMapResult[i];
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clResult,hostMapResult,0,NULL,NULL);

    CHECK_OPENCL(status);
    status = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(status);
    status = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clLiftData);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clRightData);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clResult);
    CHECK_OPENCL(status);
    return 0;
}

int OclCalc::OclHostSignedSub(double *lData,double *rData,double *rResult,int dSize) {
    KernelEnv kEnv;
    int status;
    const char *kernelName = "oclSignedSub";
    CheckKernelName(&kEnv,kernelName);

    cl_int clStatus;
    size_t global_work_size[1];
    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kernelName, &clStatus);
    cl_mem clLiftData = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);
    cl_mem clRightData = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);
    cl_mem clResult = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);

    float * hostMapLeftData     = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clLiftData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    float * hostMapRightData    = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clRightData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<dSize;i++)
    {
        hostMapLeftData[i]    = (float)lData[i];
        hostMapRightData[i] = (float)rData[i];
    }
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clLiftData,hostMapLeftData,0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clRightData,hostMapRightData,0,NULL,NULL);

    status = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&clLiftData);
    status = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&clRightData);
    status = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&clResult);
    CHECK_OPENCL(status)
    global_work_size[0] = dSize;
    status = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(status);

    float * hostMapResult = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clResult,CL_TRUE,CL_MAP_READ,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<dSize;i++)
        rResult[i]=hostMapResult[i];
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clResult,hostMapResult,0,NULL,NULL);

    CHECK_OPENCL(status);
    status = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(status);
    status = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clLiftData);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clRightData);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clResult);
    CHECK_OPENCL(status);
    return 0;
}

int OclCalc::OclHostSignedMul(double *lData,double *rData,double *rResult,int dSize) {
    KernelEnv kEnv;
    int status;
    const char *kernelName = "oclSignedMul";
    CheckKernelName(&kEnv,kernelName);


    size_t global_work_size[1];
    cl_int clStatus;
    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram, kernelName, &clStatus);
    cl_mem clLiftData = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);
    cl_mem clRightData = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);
    cl_mem clResult = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);

    float * hostMapLeftData     = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clLiftData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    float * hostMapRightData    = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clRightData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<dSize;i++)
    {
        hostMapLeftData[i]    = (float)lData[i];
        hostMapRightData[i] = (float)rData[i];
    }
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clLiftData,hostMapLeftData,0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clRightData,hostMapRightData,0,NULL,NULL);

    status = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&clLiftData);
    status = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&clRightData);
    status = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&clResult);
    CHECK_OPENCL(status)
    global_work_size[0] = dSize;
    status = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(status);

    float * hostMapResult = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clResult,CL_TRUE,CL_MAP_READ,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<dSize;i++)
        rResult[i]=hostMapResult[i];
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clResult,hostMapResult,0,NULL,NULL);

    CHECK_OPENCL(status);
    status = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(status);
    status = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clLiftData);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clRightData);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clResult);
    CHECK_OPENCL(status);
    return 0;
}

int OclCalc::OclHostSignedDiv(double *lData,double *rData,double *rResult,int dSize) {
    KernelEnv kEnv;
    int status;
    const char *kernelName = "oclSignedDiv";
    CheckKernelName(&kEnv,kernelName);


    cl_int clStatus;
    size_t global_work_size[1];
    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kernelName, &clStatus);
    cl_mem clLiftData = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);
    cl_mem clRightData = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);
    cl_mem clResult = clCreateBuffer(kEnv.mpkContext,
        (cl_mem_flags) (CL_MEM_READ_WRITE),
        dSize * sizeof(float), NULL, &clStatus);

    float * hostMapLeftData     = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clLiftData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    float * hostMapRightData    = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clRightData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<dSize;i++)
    {
        hostMapLeftData[i]    = (float)lData[i];
        hostMapRightData[i] = (float)rData[i];
    }
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clLiftData,hostMapLeftData,0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clRightData,hostMapRightData,0,NULL,NULL);

    status = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&clLiftData);
    status = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&clRightData);
    status = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&clResult);
    CHECK_OPENCL(status);
    global_work_size[0] = dSize;
    status = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(status);

    float * hostMapResult = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clResult,CL_TRUE,CL_MAP_READ,0,dSize*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<dSize;i++)
        rResult[i]=hostMapResult[i];
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clResult,hostMapResult,0,NULL,NULL);

    CHECK_OPENCL(status);
    status = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(status);
    status = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clLiftData);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clRightData);
    CHECK_OPENCL(status);
    status = clReleaseMemObject(clResult);
    CHECK_OPENCL(status);
    return 0;
}
#endif // GPU_64BITS
int OclCalc::CreateBuffer(float *&fpSrcData,uint *&npStartPos,uint *&npEndPos,int nBufferSize)
{
    cl_int clStatus = 0;
    SetKernelEnv(&kEnv);

    mpClmemSrcData    = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                             nBufferSize * sizeof(float), NULL, &clStatus);
    CHECK_OPENCL(clStatus);
    mpClmemStartPos    = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                             nBufferSize * sizeof(unsigned int), NULL, &clStatus);
    CHECK_OPENCL(clStatus);
    mpClmemEndPos    = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                             nBufferSize * sizeof(unsigned int), NULL, &clStatus);
    CHECK_OPENCL(clStatus);

    fpSrcData  = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,mpClmemSrcData,CL_TRUE,OPENCLWRAPPER_CL_MAP_WRITE_FLAG,0,nBufferSize * sizeof(float),0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    npStartPos = (uint *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,mpClmemStartPos,CL_TRUE,OPENCLWRAPPER_CL_MAP_WRITE_FLAG,0,nBufferSize * sizeof(uint),0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    npEndPos   = (uint *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,mpClmemEndPos,CL_TRUE,OPENCLWRAPPER_CL_MAP_WRITE_FLAG,0,nBufferSize * sizeof(uint),0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    //printf("In CreateBuffer, pptrr is %d,%d,%d\n",fpSrcData,npStartPos,npEndPos);
    return 0;
}

int OclCalc::CreateBuffer(float *&fpLeftData,float *&fpRightData,int nBufferSize)
{
    cl_int clStatus = 0;
    SetKernelEnv(&kEnv);

    mpClmemLeftData     = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                             nBufferSize * sizeof(float), NULL, &clStatus);
    CHECK_OPENCL(clStatus);
    mpClmemRightData = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR),
                             nBufferSize * sizeof(unsigned int), NULL, &clStatus);
    CHECK_OPENCL(clStatus);
    fpLeftData    = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,mpClmemLeftData,CL_TRUE,OPENCLWRAPPER_CL_MAP_WRITE_FLAG,0,nBufferSize * sizeof(float),0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    fpRightData = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,mpClmemRightData,CL_TRUE,OPENCLWRAPPER_CL_MAP_WRITE_FLAG,0,nBufferSize * sizeof(float),0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    //printf("In CreateBuffer, pptrr is %d,%d,%d\n",fpSrcData,npStartPos,npEndPos);
    return 0;
}

int OclCalc::OclHostFormulaMax32Bits(float *fpSrcData,uint *npStartPos,uint *npEndPos,double *output,int size)
{
    cl_int clStatus;
    size_t global_work_size[1];
    //int alignSize = size + end[0]-start[0];
    //for(int u=0;u < size;u++)
    //fprintf(stderr,"fpOclSrcData[%d] is %f.\n",u,fpSrcData[u]);
    const char *kernelName = "oclFormulaMax";
    CheckKernelName(&kEnv,kernelName);

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kEnv.mckKernelName,&clStatus);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemSrcData, fpSrcData, 0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemStartPos,npStartPos,0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemEndPos,    npEndPos,  0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    cl_mem outputCl = clCreateBuffer(kEnv.mpkContext,
                                     CL_MEM_READ_WRITE,
                                     size* sizeof(float),
                                     NULL,
                                     &clStatus);
    CHECK_OPENCL(clStatus);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&mpClmemSrcData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),(void *)&mpClmemStartPos);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),(void *)&mpClmemEndPos);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 3, sizeof(cl_mem),(void *)&outputCl);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue,
                                      kEnv.mpkKernel,
                                      1,
                                      NULL,
                                      global_work_size,
                                      NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    float * outputMap = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,
                                                    outputCl,
                                                    CL_TRUE,CL_MAP_READ,
                                                    0,
                                                    size*sizeof(float),
                                                    0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    for(int i=0;i<size;i++)
        output[i]=outputMap[i];// from gpu float type to cpu double type
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,outputCl,outputMap,0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(clStatus);

    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemSrcData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemStartPos);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemEndPos);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(outputCl);
    CHECK_OPENCL(clStatus);
    return 0;

}

int OclCalc::OclHostFormulaMin32Bits(float *fpSrcData,uint *npStartPos,uint *npEndPos,double *output,int size)
{
    cl_int clStatus;
    size_t global_work_size[1];
    //int alignSize = size + end[0]-start[0];
    //for(int u=0;u < size;u++)
    //fprintf(stderr,"fpOclSrcData[%d] is %f.\n",u,fpSrcData[u]);
    const char *kernelName = "oclFormulaMin";
    CheckKernelName(&kEnv,kernelName);

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kEnv.mckKernelName,&clStatus);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemSrcData, fpSrcData, 0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemStartPos,npStartPos,0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemEndPos,    npEndPos,  0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    cl_mem outputCl = clCreateBuffer(kEnv.mpkContext,
                                     CL_MEM_READ_WRITE,
                                     size* sizeof(float),
                                     NULL,
                                     &clStatus);
    CHECK_OPENCL(clStatus);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&mpClmemSrcData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),(void *)&mpClmemStartPos);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),(void *)&mpClmemEndPos);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 3, sizeof(cl_mem),(void *)&outputCl);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue,
                                      kEnv.mpkKernel,
                                      1,
                                      NULL,
                                      global_work_size,
                                      NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    float * outputMap = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,
                                                    outputCl,
                                                    CL_TRUE,CL_MAP_READ,
                                                    0,
                                                    size*sizeof(float),
                                                    0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    for(int i=0;i<size;i++)
        output[i]=outputMap[i];// from gpu float type to cpu double type
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,outputCl,outputMap,0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(clStatus);

    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemSrcData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemStartPos);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemEndPos);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(outputCl);
    CHECK_OPENCL(clStatus);
    return 0;
}

int OclCalc::OclHostFormulaAverage32Bits(float *fpSrcData,uint *npStartPos,uint *npEndPos,double *output,int size)
{
    cl_int clStatus;
    size_t global_work_size[1];
    //int alignSize = size + end[0]-start[0];
    //for(int u=0;u < size;u++)
    //fprintf(stderr,"fpOclSrcData[%d] is %f.\n",u,fpSrcData[u]);
    const char *kernelName = "oclFormulaAverage";
    CheckKernelName(&kEnv,kernelName);

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kEnv.mckKernelName,&clStatus);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemSrcData, fpSrcData, 0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemStartPos,npStartPos,0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemEndPos,    npEndPos,  0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    cl_mem outputCl = clCreateBuffer(kEnv.mpkContext,
                                     CL_MEM_READ_WRITE,
                                     size* sizeof(float),
                                     NULL,
                                     &clStatus);
    CHECK_OPENCL(clStatus);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&mpClmemSrcData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),(void *)&mpClmemStartPos);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),(void *)&mpClmemEndPos);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 3, sizeof(cl_mem),(void *)&outputCl);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = size;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue,
                                      kEnv.mpkKernel,
                                      1,
                                      NULL,
                                      global_work_size,
                                      NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    float * outputMap = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,
                                                    outputCl,
                                                    CL_TRUE,CL_MAP_READ,
                                                    0,
                                                    size*sizeof(float),
                                                    0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    for(int i=0;i<size;i++)
        output[i]=outputMap[i];// from gpu float type to cpu double type
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,outputCl,outputMap,0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(clStatus);

    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemSrcData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemStartPos);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemEndPos);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(outputCl);
    CHECK_OPENCL(clStatus);
    return 0;
}


int OclCalc::OclHostSignedAdd32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize)
{
    cl_int clStatus;
    size_t global_work_size[1];
    const char *kernelName = "oclSignedAdd";
    CheckKernelName(&kEnv,kernelName);

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kernelName, &clStatus);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemLeftData,fpLeftData,0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemRightData,fpRightData,0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    cl_mem clResult = clCreateBuffer(kEnv.mpkContext,
                                    CL_MEM_READ_WRITE,
                                    nRowSize * sizeof(float),
                                    NULL,
                                    &clStatus);
    CHECK_OPENCL(clStatus);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&mpClmemLeftData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),(void *)&mpClmemRightData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),(void *)&clResult);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
                                      NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    float * hostMapResult = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clResult,CL_TRUE,CL_MAP_READ,0,nRowSize*sizeof(float),0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    for(int i=0;i<nRowSize;i++)
        rResult[i]=hostMapResult[i];
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clResult,hostMapResult,0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    clStatus = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemLeftData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemRightData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clResult);
    CHECK_OPENCL(clStatus);
    return 0;
}
int OclCalc::OclHostSignedSub32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize)
{
    cl_int clStatus;
    size_t global_work_size[1];
    const char *kernelName = "oclSignedSub";
    CheckKernelName(&kEnv,kernelName);

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kernelName, &clStatus);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemLeftData,fpLeftData,0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemRightData,fpRightData,0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    cl_mem clResult = clCreateBuffer(kEnv.mpkContext,
                                    CL_MEM_READ_WRITE,
                                    nRowSize * sizeof(float),
                                    NULL,
                                    &clStatus);
    CHECK_OPENCL(clStatus);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&mpClmemLeftData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),(void *)&mpClmemRightData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),(void *)&clResult);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
                                      NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    float * hostMapResult = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clResult,CL_TRUE,CL_MAP_READ,0,nRowSize*sizeof(float),0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    for(int i=0;i<nRowSize;i++)
        rResult[i]=hostMapResult[i];
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clResult,hostMapResult,0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    clStatus = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemLeftData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemRightData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clResult);
    CHECK_OPENCL(clStatus);
    return 0;
}
int OclCalc::OclHostSignedMul32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize)
{
    cl_int clStatus;
    size_t global_work_size[1];
    const char *kernelName = "oclSignedMul";
    CheckKernelName(&kEnv,kernelName);

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kernelName, &clStatus);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemLeftData,fpLeftData,0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemRightData,fpRightData,0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    cl_mem clResult = clCreateBuffer(kEnv.mpkContext,
                                    CL_MEM_READ_WRITE,
                                    nRowSize * sizeof(float),
                                    NULL,
                                    &clStatus);
    CHECK_OPENCL(clStatus);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&mpClmemLeftData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),(void *)&mpClmemRightData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),(void *)&clResult);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
                                      NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    float * hostMapResult = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clResult,CL_TRUE,CL_MAP_READ,0,nRowSize*sizeof(float),0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    for(int i=0;i<nRowSize;i++)
        rResult[i]=hostMapResult[i];
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clResult,hostMapResult,0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    clStatus = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemLeftData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemRightData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clResult);
    CHECK_OPENCL(clStatus);
    return 0;
}
int OclCalc::OclHostSignedDiv32Bits(float *fpLeftData,float *fpRightData,double *rResult,int nRowSize)
{
    cl_int clStatus;
    size_t global_work_size[1];
    const char *kernelName = "oclSignedDiv";
    CheckKernelName(&kEnv,kernelName);

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram,kernelName, &clStatus);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemLeftData,fpLeftData,0,NULL,NULL);
    CHECK_OPENCL(clStatus);
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,mpClmemRightData,fpRightData,0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    cl_mem clResult = clCreateBuffer(kEnv.mpkContext,
                                    CL_MEM_READ_WRITE,
                                    nRowSize * sizeof(float),
                                    NULL,
                                    &clStatus);
    CHECK_OPENCL(clStatus);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),(void *)&mpClmemLeftData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),(void *)&mpClmemRightData);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),(void *)&clResult);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = nRowSize;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
                                      NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    float * hostMapResult = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clResult,CL_TRUE,CL_MAP_READ,0,nRowSize*sizeof(float),0,NULL,NULL,&clStatus);
    CHECK_OPENCL(clStatus);
    for(int i=0;i<nRowSize;i++)
        rResult[i]=hostMapResult[i];
    clStatus = clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clResult,hostMapResult,0,NULL,NULL);
    CHECK_OPENCL(clStatus);

    clStatus = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemLeftData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(mpClmemRightData);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clResult);
    CHECK_OPENCL(clStatus);
    return 0;
}

#ifdef FORMULAE_DEBUG
int OclCalc::OclHostFormulaSum(float *dpsrcData,int *npStart,int *npEnd,float *dpOutput,int nSize) {
    KernelEnv kEnv;
    const char *cpKernelName = "oclFormulaSum";
    CheckKernelName(&kEnv,cpKernelName);

    cl_int clStatus;
    size_t global_work_size[1];
    if(nSize < 1 )
    {
        printf("The nSize should be bigger than one\n");
        return -1;
    }
    int nAlignSize = npEnd[nSize-1]-npStart[0]+1;

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram, cpKernelName, &clStatus);
    cl_mem clpInput = clCreateBuffer(kEnv.mpkContext,(cl_mem_flags) (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR),
        nAlignSize * sizeof(float), (void *)dpsrcData, &clStatus);
    cl_mem clpStart = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR ),
        nSize * sizeof(unsigned int), (void *)npStart, &clStatus);
    cl_mem clpEnd = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR),
        nSize * sizeof(unsigned int), (void *)npEnd, &clStatus);
    cl_mem clpOutput = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_WRITE_ONLY|CL_MEM_ALLOC_HOST_PTR),
    nSize* sizeof(float), NULL, &clStatus);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&clpInput);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&clpStart);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&clpEnd);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 3, sizeof(cl_mem),
        (void *)&clpOutput);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);
    float * fpOutPutMap = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clpOutput,CL_TRUE,CL_MAP_READ,0,nSize*sizeof(float),0,NULL,NULL,NULL);
    for(int i=0;i<nSize;i++)
        dpOutput[i]=fpOutPutMap[i];

    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clpOutput,fpOutPutMap,0,NULL,NULL);
    clStatus = clFinish(kEnv.mpkCmdQueue);

    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpInput);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpStart);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpEnd);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpOutput);
    CHECK_OPENCL(clStatus);
    return 0;
}

int OclCalc::OclHostFormulaCount(int *npStart,int *npEnd,float *dpOutput,int nSize) {
    KernelEnv kEnv;
    const char *cpKernelName = "oclFormulaCount";
    CheckKernelName(&kEnv,cpKernelName);

    cl_int clStatus;
    size_t global_work_size[1];

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram, cpKernelName, &clStatus);
    cl_mem clpStart = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR ),
            nSize * sizeof(unsigned int), (void *)npEnd, &clStatus);
    cl_mem clpEnd = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR),
            nSize * sizeof(unsigned int), (void *)dpOutput, &clStatus);
    cl_mem clpOutput = clCreateBuffer(kEnv.mpkContext,(cl_mem_flags) (CL_MEM_READ_ONLY),
            nSize * sizeof(float), (void *)npStart, &clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&clpStart);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&clpEnd);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&clpOutput);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus)

    clStatus=clEnqueueReadBuffer(kEnv.mpkCmdQueue,clpOutput,CL_TRUE,0,nSize*sizeof(float),(void *)dpOutput,0,NULL,NULL);
    if(clStatus!=0)
    {
        printf("clEnqueueReadBuffer err\n");
        return -1;
    }

    clStatus = clFinish(kEnv.mpkCmdQueue);

    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpStart);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpEnd);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpOutput);
    CHECK_OPENCL(clStatus);
    return 0;
}

/*
 * The dpsrcData is double rows,one col is the first column data,second is the second column data.if get a cell data range,the range
 *save the npStart array eg:a4-a8;b10-b14,the npStart will store a4,b10,and the npEnd will store a8,b14 range.So it can if(i +1)%2 to judge
 * the a cloumn or b cloumn npStart range.so as b bolumn.
 */
int OclCalc::OclHostFormulaSumProduct(float *dpSrcData,int *npStart,int *npEnd,float *dpOutput,int nSize) {
    KernelEnv kEnv;
    const char *cpKernelName = "oclFormulaSumproduct";
    CheckKernelName(&kEnv,cpKernelName);

    cl_int clStatus;
    size_t global_work_size[1];

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram, cpKernelName, &clStatus);
    cl_mem clpFirstCol = clCreateBuffer(kEnv.mpkContext,(cl_mem_flags) (CL_MEM_READ_WRITE ),
        nSize * sizeof(float),NULL, &clStatus);
    cl_mem clpSecondCol = clCreateBuffer(kEnv.mpkContext,(cl_mem_flags) (CL_MEM_READ_WRITE),
        nSize * sizeof(float),NULL, &clStatus);
    cl_mem clpStart = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR ),
        nSize *2 * sizeof(unsigned int), (void *)npStart, &clStatus);
    cl_mem clpEnd = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY|CL_MEM_USE_HOST_PTR ),
        nSize *2 * sizeof(unsigned int),(void *)npEnd , &clStatus);
    cl_mem clpOutput = clCreateBuffer(kEnv.mpkContext, (cl_mem_flags) (CL_MEM_READ_ONLY),
    nSize* sizeof(float), (void *)dpOutput, &clStatus);

    float * fpHostMapFirCol = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clpFirstCol,CL_TRUE,CL_MAP_WRITE,0,nSize * sizeof(float),0,NULL,NULL,NULL);
    //checkUpPoint("hostMapSrc",hostMapSrc);
    float * fpHostMapSecCol = (float *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,clpSecondCol,CL_TRUE,CL_MAP_WRITE,0,nSize * sizeof(float),0,NULL,NULL,NULL);
    //checkUpPoint("hostMapSrc",hostMapSrc);
    if(NULL==fpHostMapFirCol||NULL==fpHostMapSecCol)
    {
        printf("In File %s at %d line alloc err\n",__FILE__,__LINE__);
        return -1;
    }
    for(int i=0;i<nSize;i++)
    {
        fpHostMapFirCol[i]    = dpSrcData[2*i];
        fpHostMapSecCol[i]    = dpSrcData[2*i+1];
    }
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clpFirstCol,fpHostMapFirCol,0,NULL,NULL);
    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,clpSecondCol,fpHostMapSecCol,0,NULL,NULL);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
        (void *)&fpHostMapFirCol);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
        (void *)&fpHostMapSecCol);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_mem),
        (void *)&clpStart);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 3, sizeof(cl_mem),
        (void *)&clpEnd);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 4, sizeof(cl_mem),
        (void *)&clpOutput);
    CHECK_OPENCL(clStatus);

    global_work_size[0] = nSize;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
        NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus)

    clStatus=clEnqueueReadBuffer(kEnv.mpkCmdQueue,clpOutput,CL_TRUE,0,nSize*sizeof(float),(void *)dpOutput,0,NULL,NULL);
    if(clStatus!=0)
    {
        printf("clEnqueueReadBuffer err\n");
        return -1;
    }
    clStatus = clFinish(kEnv.mpkCmdQueue);

    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpFirstCol);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpSecondCol);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpStart);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpEnd);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(clpOutput);
    CHECK_OPENCL(clStatus);
    return 0;
}
#endif

#if 0
typedef double fp_;
#else
typedef float fp_t;
#endif

// FIXME: should be templatised in <double> - double buffering [sic] rocks
static cl_mem allocateDoubleBuffer(KernelEnv &rEnv, const double *_pValues,
                                   size_t nElements, cl_int *pStatus)
{
    // Ugh - horrible redundant copying ...
    cl_mem xValues = clCreateBuffer(rEnv.mpkContext,(cl_mem_flags) (CL_MEM_READ_WRITE),
                                    nElements * sizeof(double), NULL, pStatus);
    fp_t *pValues = (fp_t *)clEnqueueMapBuffer(rEnv.mpkCmdQueue,xValues,CL_TRUE,CL_MAP_WRITE,0,
                                                   nElements * sizeof(fp_t),0,NULL,NULL,NULL);
    for(int i=0;i<(int)nElements;i++)
        pValues[i] = (fp_t)_pValues[i];

    clEnqueueUnmapMemObject(rEnv.mpkCmdQueue,xValues,pValues,0,NULL,NULL);

    return xValues;
}

double *OclCalc::OclSimpleDeltaOperation(OpCode eOp, const double *pOpArray,
                                         const double *pSubtractSingle, size_t nElements)
{
    SetKernelEnv(&kEnv);

    // select a kernel: cut & paste coding is utterly evil.
    const char *kernelName = NULL;
    switch (eOp) {
    case ocAdd:
    case ocSub:
        fprintf(stderr,"ocSub is %d\n",ocSub);
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
    default:
        assert(false);
    }
    CheckKernelName(&kEnv,kernelName);

    cl_int clStatus;
    size_t global_work_size[1];

    kEnv.mpkKernel = clCreateKernel(kEnv.mpkProgram, kernelName, &clStatus);
    CHECK_OPENCL(clStatus);
    if (!kEnv.mpkKernel)
    {
        fprintf(stderr, "could not clCreateKernel '%s'\n", kernelName);
        return NULL;
    }

    // Ugh - horrible redundant copying ...
    cl_mem valuesCl   = allocateDoubleBuffer(kEnv, pOpArray, nElements, &clStatus);
    cl_mem subtractCl = allocateDoubleBuffer(kEnv, pSubtractSingle, nElements, &clStatus);

    cl_uint start = 0;
    cl_uint end = (cl_uint) nElements;
    cl_mem outputCl = clCreateBuffer(kEnv.mpkContext,
                                     CL_MEM_READ_WRITE,
                                     nElements * sizeof(fp_t),
                                     NULL,
                                     &clStatus);
    CHECK_OPENCL(clStatus);

    clStatus = clSetKernelArg(kEnv.mpkKernel, 0, sizeof(cl_mem),
                              (void *)&valuesCl);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 1, sizeof(cl_mem),
                              (void *)&subtractCl);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 2, sizeof(cl_uint),
                              (void *)&start);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 3, sizeof(cl_uint),
                              (void *)&end);
    CHECK_OPENCL(clStatus);
    clStatus = clSetKernelArg(kEnv.mpkKernel, 4, sizeof(cl_mem),
                              (void *)&outputCl);
    CHECK_OPENCL(clStatus);

    fprintf(stderr, "prior to enqueue range kernel\n");

    global_work_size[0] = nElements;
    clStatus = clEnqueueNDRangeKernel(kEnv.mpkCmdQueue, kEnv.mpkKernel, 1,
                                      NULL, global_work_size, NULL, 0, NULL, NULL);
    CHECK_OPENCL(clStatus);

    double *pResult = new double[nElements];
    if(!pResult)
        return NULL; // leak.

    fp_t *pOutput = (fp_t *)clEnqueueMapBuffer(kEnv.mpkCmdQueue,outputCl,CL_TRUE,
                                               CL_MAP_READ,0,nElements*sizeof(fp_t),
                                               0,NULL,NULL,NULL);
    for(int i = 0; i < (int)nElements; i++)
        pResult[i] = (double)pOutput[i];

    clEnqueueUnmapMemObject(kEnv.mpkCmdQueue,outputCl,pOutput,0,NULL,NULL);

    clStatus = clFinish(kEnv.mpkCmdQueue);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseKernel(kEnv.mpkKernel);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(valuesCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(subtractCl);
    CHECK_OPENCL(clStatus);
    clStatus = clReleaseMemObject(outputCl);
    CHECK_OPENCL(clStatus);

    fprintf(stderr, "completed opencl delta operation\n");

    return pResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
