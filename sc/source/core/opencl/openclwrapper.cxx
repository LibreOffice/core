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
inline int OpenclDevice::AddKernelConfig(int kCount, const char *kName) {
    strcpy(gpuEnv.kernelNames[kCount], kName);
    gpuEnv.kernelCount++;
    return 0;
}

int OpenclDevice::RegistOpenclKernel() {
    if (!gpuEnv.isUserCreated) {
        memset(&gpuEnv, 0, sizeof(gpuEnv));
    }

    gpuEnv.fileCount = 0; //argc;
    gpuEnv.kernelCount = 0UL;

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
	return 0;
}
OpenclDevice::OpenclDevice(){
	//InitEnv();
}

OpenclDevice::~OpenclDevice() {
	//ReleaseOpenclRunEnv();
}

int OpenclDevice::CheckKernelName(KernelEnv *envInfo,const char *kernelName){
    //printf("CheckKernelName,total count of kernels...%d\n", gpuEnv.kernelCount);
    int kCount;
    for(kCount=0; kCount < gpuEnv.kernelCount; kCount++) {
        if(strcasecmp(kernelName, gpuEnv.kernelNames[kCount]) == 0) {
	    printf("match  %s kernel right\n",kernelName);
	    break;
        }
    }
    envInfo->context      = gpuEnv.context;
    envInfo->commandQueue = gpuEnv.commandQueue;
    envInfo->program      = gpuEnv.programs[0];
    envInfo->kernel       = gpuEnv.kernels[kCount];
    strcpy(envInfo->kernelName, kernelName);
    if (envInfo == (KernelEnv *) NULL)
    {
        printf("get err func and env\n");
        return 0;
    }
    return 1;
}

int OpenclDevice::ConvertToString(const char *filename, char **source) {
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
        *source = (char*) malloc(sizeof(char) * file_size + 1);
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

int OpenclDevice::BinaryGenerated(const char * clFileName, FILE ** fhandle) {
    unsigned int i = 0;
	cl_int status;
	char *str = NULL;
	FILE *fd = NULL;
	cl_uint numDevices=0;
	status = clGetDeviceIDs(gpuEnv.platform, // platform
							CL_DEVICE_TYPE_GPU, // device_type
							0, // num_entries
							NULL, // devices
							&numDevices);
	for (i = 0; i <numDevices; i++) {
		char fileName[256] = { 0 }, cl_name[128] = { 0 };
		if (gpuEnv.devices[i] != 0) {
			char deviceName[1024];
			status = clGetDeviceInfo(gpuEnv.devices[i], CL_DEVICE_NAME,sizeof(deviceName), deviceName, NULL);
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
        size_t numBytes) {
    FILE *output = NULL;
    output = fopen(fileName, "wb");
    if (output == NULL) {
        return 0;
    }

    fwrite(birary, sizeof(char), numBytes, output);
    fclose(output);

    return 1;

}

int OpenclDevice::GeneratBinFromKernelSource(cl_program program,
        const char * clFileName) {
     unsigned int i = 0;
    cl_int status;
    size_t *binarySizes, numDevices;
    cl_device_id *devices;
    char **binaries, *str = NULL;

    status = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES,
            sizeof(numDevices), &numDevices, NULL);
    CHECK_OPENCL(status)

    devices = (cl_device_id*) malloc(sizeof(cl_device_id) * numDevices);
    if (devices == NULL) {
        return 0;
    }
    /* grab the handles to all of the devices in the program. */
    status = clGetProgramInfo(program, CL_PROGRAM_DEVICES,
            sizeof(cl_device_id) * numDevices, devices, NULL);
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
            binaries[i] = (char*) malloc(sizeof(char) * binarySizes[i]);
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
            status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME,
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

    if (devices != NULL) {
        free(devices);
        devices = NULL;
    }
    return 1;
}

int OpenclDevice::InitOpenclAttr(OpenCLEnv * env) {
    if (gpuEnv.isUserCreated) {
        return 1;
    }

    gpuEnv.context = env->context;
    gpuEnv.platform = env->platform;
    gpuEnv.dev = env->devices;
    gpuEnv.commandQueue = env->commandQueue;

    gpuEnv.isUserCreated = 1;

    return 0;
}

int OpenclDevice::CreateKernel(char * kernelname, KernelEnv * env) {
    int status;

    env->kernel = clCreateKernel(gpuEnv.programs[0], kernelname, &status);
    env->context = gpuEnv.context;
    env->commandQueue = gpuEnv.commandQueue;
    return status != CL_SUCCESS ? 1 : 0;
}

int OpenclDevice::ReleaseKernel(KernelEnv * env) {
    int status = clReleaseKernel(env->kernel);
    return status != CL_SUCCESS ? 1 : 0;
}

int OpenclDevice::ReleaseOpenclEnv(GPUEnv *gpuInfo) {
    int i = 0;
    int status = 0;

    if (!isInited) {
        return 1;
    }

    for (i = 0; i < gpuEnv.fileCount; i++) {
        if (gpuEnv.programs[i]) {
            status = clReleaseProgram(gpuEnv.programs[i]);
            CHECK_OPENCL(status)
            gpuEnv.programs[i] = NULL;
        }
    }
    if (gpuEnv.commandQueue) {
        clReleaseCommandQueue(gpuEnv.commandQueue);
        gpuEnv.commandQueue = NULL;
    }
    if (gpuEnv.context) {
        clReleaseContext(gpuEnv.context);
        gpuEnv.context = NULL;
    }
    isInited = 0;
    gpuInfo->isUserCreated = 0;
    free(gpuInfo->devices);
    return 1;
}

int OpenclDevice::RunKernelWrapper(cl_kernel_function function,
        const char * kernelName, void **usrdata) {
    printf("oclwrapper:RunKernel_wrapper...\n");
    if (RegisterKernelWrapper(kernelName, function) != 1) {
        fprintf(stderr,
                "Error:RunKernel_wrapper:RegisterKernelWrapper fail!\n");
        return -1;
    }
    return (RunKernel(kernelName, usrdata));
}

int OpenclDevice::CachedOfKernerPrg(const GPUEnv *gpuEnvCached,
        const char * clFileName) {
  int i;
    for (i = 0; i < gpuEnvCached->fileCount; i++) {
        if (strcasecmp(gpuEnvCached->kernelSrcFile[i], clFileName) == 0) {
            if (gpuEnvCached->programs[i] != NULL) {
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
    size_t numDevices;
    cl_device_id *devices;
    FILE *fd, *fd1;
    const char* filename = "kernel.cl";
	fprintf(stderr, "CompileKernelFile ... \n");
    if (CachedOfKernerPrg(gpuInfo, filename) == 1) {
        return 1;
    }

    idx = gpuInfo->fileCount;

    source = kernel_src;

    source_size[0] = strlen(source);
    binaryExisted = 0;
    if ((binaryExisted = BinaryGenerated(filename, &fd)) == 1) {
#ifdef CL_CONTEXT_NUM_DEVICES
        status = clGetContextInfo(gpuInfo->context, CL_CONTEXT_NUM_DEVICES,
                sizeof(numDevices), &numDevices, NULL);
        CHECK_OPENCL(status)
#else
        numDevices = 1; // ???
#endif
        devices = (cl_device_id*) malloc(sizeof(cl_device_id) * numDevices);
        if (devices == NULL) {
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

        binary = (char*) malloc(length + 2);
        if (!binary) {
            return 0;
        }

        memset(binary, 0, length + 2);
        b_error |= fread(binary, 1, length, fd) != length;
        if (binary[length - 1] != '\n') {
            binary[length++] = '\n';
        }

        fclose(fd);
        fd = NULL;
        // grab the handles to all of the devices in the context.
        status = clGetContextInfo(gpuInfo->context, CL_CONTEXT_DEVICES,
                sizeof(cl_device_id) * numDevices, devices, NULL);
        CHECK_OPENCL(status)

        gpuInfo->programs[idx] = clCreateProgramWithBinary(gpuInfo->context,
                numDevices, devices, &length, (const unsigned char**) &binary,
                &binary_status, &status);
        CHECK_OPENCL(status)

        free(binary);
        free(devices);
        devices = NULL;
    } else {
        // create a CL program using the kernel source
        gpuEnv.programs[idx] = clCreateProgramWithSource(gpuEnv.context,
                1, &source, source_size, &status);
        CHECK_OPENCL(status);
    }

    if (gpuInfo->programs[idx] == (cl_program) NULL) {
        return 0;
    }

    //char options[512];
    // create a cl program executable for all the devices specified
    if (!gpuInfo->isUserCreated) {
        status = clBuildProgram(gpuInfo->programs[idx], 1, gpuInfo->devices,
                buildOption, NULL, NULL);
        CHECK_OPENCL(status)
    } else {
        status = clBuildProgram(gpuInfo->programs[idx], 1, &(gpuInfo->dev),
                buildOption, NULL, NULL);
        CHECK_OPENCL(status)
    }
    printf("BuildProgram.\n");

    if (status != CL_SUCCESS) {
        if (!gpuInfo->isUserCreated) {
            status = clGetProgramBuildInfo(gpuInfo->programs[idx],
                    gpuInfo->devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL,
                    &length);
        } else {
            status = clGetProgramBuildInfo(gpuInfo->programs[idx],
                    gpuInfo->dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &length);
        }
        if (status != CL_SUCCESS) {
            printf("opencl create build log fail\n");
            return 0;
        }
        buildLog = (char*) malloc(length);
        if (buildLog == (char*) NULL) {
            return 0;
        }
        if (!gpuInfo->isUserCreated) {
            status = clGetProgramBuildInfo(gpuInfo->programs[idx],
                    gpuInfo->devices[0], CL_PROGRAM_BUILD_LOG, length,
                    buildLog, &length);
        } else {
            status = clGetProgramBuildInfo(gpuInfo->programs[idx],
                    gpuInfo->dev, CL_PROGRAM_BUILD_LOG, length, buildLog,
                    &length);
        }

        fd1 = fopen("kernel-build.log", "w+");
        if (fd1 != NULL) {
            fwrite(buildLog, sizeof(char), length, fd1);
            fclose(fd1);
        }

        free(buildLog);
        return 0;
    }

    strcpy(gpuEnv.kernelSrcFile[idx], filename);

    if (binaryExisted == 0)
        GeneratBinFromKernelSource(gpuEnv.programs[idx], filename);

    gpuInfo->fileCount += 1;

    return 1;


}
int OpenclDevice::GetKernelEnvAndFunc(const char *kernelName,
        KernelEnv *env, cl_kernel_function *function) {
    int i; //,program_idx ;
    printf("----------------OpenclDevice::GetKernelEnvAndFunc\n");
    for (i = 0; i < gpuEnv.kernelCount; i++) {
        if (strcasecmp(kernelName, gpuEnv.kernelNames[i]) == 0) {
            env->context = gpuEnv.context;
            env->commandQueue = gpuEnv.commandQueue;
            env->program = gpuEnv.programs[0];
            env->kernel = gpuEnv.kernels[i];
            *function = gpuEnv.kernelFunctions[i];
            return 1;
        }
    }
    return 0;
}

int OpenclDevice::RunKernel(const char *kernelName, void **userdata) {
    KernelEnv env;

    cl_kernel_function function;

    int status;

    memset(&env, 0, sizeof(KernelEnv));
    status = GetKernelEnvAndFunc(kernelName, &env, &function);
    strcpy(env.kernelName, kernelName);
    if (status == 1) {
        if (&env == (KernelEnv *) NULL
                || &function == (cl_kernel_function *) NULL) {
            return 0;
        }
        return (function(userdata, &env));
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
        if (status == 0 || gpuEnv.kernelCount == 0) {
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

    if (!gpuInfo->isUserCreated) {
        status = clGetPlatformIDs(0, NULL, &numPlatforms);
        if (status != CL_SUCCESS) {
            return 1;
        }
        gpuInfo->platform = NULL;

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
                gpuInfo->platform = platforms[i];

                //if (!strcmp(platformName, "Intel(R) Coporation"))
                //if( !strcmp( platformName, "Advanced Micro Devices, Inc." ))
                {
                    gpuInfo->platform = platforms[i];

                    status = clGetDeviceIDs(gpuInfo->platform, // platform
												CL_DEVICE_TYPE_GPU, // device_type
												0, // num_entries
												NULL, // devices
												&numDevices);

                    if (status != CL_SUCCESS) {
                        return 1;
                    }

                    if (numDevices) {
                        break;
                    }
                }
            }
            free(platforms);
        }
        if (NULL == gpuInfo->platform) {
            return 1;
        }

        // Use available platform.

        cps[0] = CL_CONTEXT_PLATFORM;
        cps[1] = (cl_context_properties) gpuInfo->platform;
        cps[2] = 0;
        // Check for GPU.
        gpuInfo->dType = CL_DEVICE_TYPE_GPU;
        gpuInfo->context = clCreateContextFromType(cps, gpuInfo->dType, NULL,
                NULL, &status);

        if ((gpuInfo->context == (cl_context) NULL)
                || (status != CL_SUCCESS)) {
            gpuInfo->dType = CL_DEVICE_TYPE_CPU;
            gpuInfo->context = clCreateContextFromType(cps, gpuInfo->dType,
                    NULL, NULL, &status);
        }
        if ((gpuInfo->context == (cl_context) NULL)
                || (status != CL_SUCCESS)) {
            gpuInfo->dType = CL_DEVICE_TYPE_DEFAULT;
            gpuInfo->context = clCreateContextFromType(cps, gpuInfo->dType,
                    NULL, NULL, &status);
        }
        if ((gpuInfo->context == (cl_context) NULL)
                || (status != CL_SUCCESS)) {
            return 1;
        }
        // Detect OpenCL devices.
        // First, get the size of device list data
        status = clGetContextInfo(gpuInfo->context, CL_CONTEXT_DEVICES, 0,
                NULL, &length);
        if ((status != CL_SUCCESS) || (length == 0)) {
            return 1;
        }
        // Now allocate memory for device list based on the size we got earlier
        gpuInfo->devices = (cl_device_id*) malloc(length);
        if (gpuInfo->devices == (cl_device_id*) NULL) {
            return 1;
        }
        // Now, get the device list data
        status = clGetContextInfo(gpuInfo->context, CL_CONTEXT_DEVICES, length,
                gpuInfo->devices, NULL);
        if (status != CL_SUCCESS) {
            return 1;
        }

        // Create OpenCL command queue.
        gpuInfo->commandQueue = clCreateCommandQueue(gpuInfo->context,
                gpuInfo->devices[0], 0, &status);

        if (status != CL_SUCCESS) {
            return 1;
        }
    }

    return 0;

}
int OpenclDevice::RegisterKernelWrapper(const char *kernelName,cl_kernel_function function)
{
	int i;
	printf("oclwrapper:RegisterKernelWrapper...%d\n", gpuEnv.kernelCount);
	for (i = 0; i < gpuEnv.kernelCount; i++)
	{
		if (strcasecmp(kernelName, gpuEnv.kernelNames[i]) == 0)
		{
			gpuEnv.kernelFunctions[i] = function;
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

    env->kernel = clCreateKernel(env->program, "oclformula", &clStatus);
    //printf("ScInterpreter::IterateParameters...after clCreateKernel.\n");
    //fprintf(stderr, "\nIn OpenclDevice,...after clCreateKernel\n");
    int size = NUM;

    cl_mem formula_data = clCreateBuffer(env->context,
            (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR),
            size * sizeof(float), (void *) tdata, &clStatus);
    //fprintf(stderr, "\nIn OpenclDevice,...after clCreateBuffer\n");

    status = clSetKernelArg(env->kernel, 0, sizeof(cl_mem),
            (void *) &formula_data);
    CHECK_OPENCL(status)
    status = clSetKernelArg(env->kernel, 1, sizeof(unsigned int),
            (void *) &type);
    CHECK_OPENCL(status)

    global_work_size[0] = size;
    //fprintf(stderr, "\nIn OpenclDevice,...after global_work_size\n");
    //PPAStartCpuEvent(ppa_proc);

    while (global_work_size[0] != 1) {
        global_work_size[0] = global_work_size[0] / 2;
        status = clEnqueueNDRangeKernel(env->commandQueue, env->kernel, 1,
                NULL, global_work_size, NULL, 0, NULL, NULL);
        CHECK_OPENCL(status)

    }
    //fprintf(stderr, "\nIn OpenclDevice,...before clEnqueueReadBuffer\n");
    status = clEnqueueReadBuffer(env->commandQueue, formula_data, CL_FALSE, 0,
            sizeof(float), (void *) &tdata, 0, NULL, NULL);
    CHECK_OPENCL(status)
    status = clFinish(env->commandQueue);
    CHECK_OPENCL(status)

    //PPAStopCpuEvent(ppa_proc);
    //fprintf(stderr, "\nIn OpenclDevice,...before clReleaseKernel\n");
    status = clReleaseKernel(env->kernel);
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

    env->kernel = clCreateKernel(env->program, "oclformula", &clStatus);
    //printf("ScInterpreter::IterateParameters...after clCreateKernel.\n");
    //fprintf(stderr, "\nIn OpenclDevice,...after clCreateKernel\n");
    int size = NUM;

    cl_mem formula_data = clCreateBuffer(env->context,
            (cl_mem_flags) (CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR),
            size * sizeof(float), (void *) tdata, &clStatus);
    //fprintf(stderr, "\nIn OpenclDevice,...after clCreateBuffer\n");

    status = clSetKernelArg(env->kernel, 0, sizeof(cl_mem),
            (void *) &formula_data);
    CHECK_OPENCL(status)
    status = clSetKernelArg(env->kernel, 1, sizeof(unsigned int),
            (void *) &type);
    CHECK_OPENCL(status)

    global_work_size[0] = size;
    //fprintf(stderr, "\nIn OpenclDevice,...after global_work_size\n");
    //PPAStartCpuEvent(ppa_proc);

    while (global_work_size[0] != 1) {
        global_work_size[0] = global_work_size[0] / 2;
        status = clEnqueueNDRangeKernel(env->commandQueue, env->kernel, 1,
                NULL, global_work_size, NULL, 0, NULL, NULL);
        CHECK_OPENCL(status)

    }
    //fprintf(stderr, "\nIn OpenclDevice,...before clEnqueueReadBuffer\n");
    status = clEnqueueReadBuffer(env->commandQueue, formula_data, CL_FALSE, 0,
            sizeof(float), (void *) &tdata, 0, NULL, NULL);
    CHECK_OPENCL(status)
    status = clFinish(env->commandQueue);
    CHECK_OPENCL(status)

    //PPAStopCpuEvent(ppa_proc);
    //fprintf(stderr, "\nIn OpenclDevice,...before clReleaseKernel\n");
    status = clReleaseKernel(env->kernel);
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
    OpenclDevice::SetOpenclState(1);
    fprintf(stderr,"OclCalc:: init opencl ok.\n");
}

OclCalc::~OclCalc()
{
    OpenclDevice::SetOpenclState(0);
    fprintf(stderr,"OclCalc:: opencl end ok.\n");
}

/////////////////////////////////////////////////////////////////////////////
int OclCalc::OclHostFormulaMax(double *srcData,int *start,int *end,double *output,int size) {
	KernelEnv env;
	const char *kernelName = "oclFormulaMax";
	CheckKernelName(&env,kernelName);
	cl_int clStatus;
	size_t global_work_size[1];
	int alignSize = size + end[0]-start[0];

	env.kernel = clCreateKernel(env.program,kernelName, &clStatus);
	cl_int ret=0;
	cl_mem inputCl = clCreateBuffer(env.context,(cl_mem_flags) (CL_MEM_READ_WRITE),
		alignSize * sizeof(float), NULL, &clStatus);
	cl_mem startCl = clCreateBuffer(env.context, (cl_mem_flags) (CL_MEM_READ_WRITE),
		size * sizeof(unsigned int), NULL, &ret);
	cl_mem endCl = clCreateBuffer(env.context, (cl_mem_flags) (CL_MEM_READ_WRITE),
		size * sizeof(unsigned int), NULL, &ret);
	cl_mem outputCl = clCreateBuffer(env.context, (cl_mem_flags) (CL_MEM_READ_WRITE),
	size* sizeof(float), NULL, &ret);

	float * hostMapSrc = (float *)clEnqueueMapBuffer(env.commandQueue,inputCl,CL_TRUE,CL_MAP_WRITE,0,alignSize * sizeof(float),0,NULL,NULL,NULL);
	int * hostMapStart = (int *)clEnqueueMapBuffer(env.commandQueue,startCl,CL_TRUE,CL_MAP_WRITE,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
	int * hostMapEnd   = (int *)clEnqueueMapBuffer(env.commandQueue,endCl,CL_TRUE,CL_MAP_WRITE,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
	for(int i=0;i<size;i++)
	{
		hostMapStart[i] = start[i];
		hostMapEnd[i]	= end[i];
	}
	for(int i=0;i<alignSize;i++)
		hostMapSrc[i] = (float)srcData[i];
	clEnqueueUnmapMemObject(env.commandQueue,inputCl,hostMapSrc,0,NULL,NULL);
	clEnqueueUnmapMemObject(env.commandQueue,startCl,hostMapStart,0,NULL,NULL);
	clEnqueueUnmapMemObject(env.commandQueue,endCl,hostMapEnd,0,NULL,NULL);

	clStatus = clSetKernelArg(env.kernel, 0, sizeof(cl_mem),
		(void *)&inputCl);
	clStatus = clSetKernelArg(env.kernel, 1, sizeof(cl_mem),
		(void *)&startCl);
	clStatus = clSetKernelArg(env.kernel, 2, sizeof(cl_mem),
		(void *)&endCl);
	clStatus = clSetKernelArg(env.kernel, 3, sizeof(cl_mem),
		(void *)&outputCl);
	CHECK_OPENCL(clStatus);

	global_work_size[0] = size;
	clStatus = clEnqueueNDRangeKernel(env.commandQueue, env.kernel, 1,
		NULL, global_work_size, NULL, 0, NULL, NULL);
	CHECK_OPENCL(clStatus)

	float * outPutMap = (float *)clEnqueueMapBuffer(env.commandQueue,outputCl,CL_TRUE,CL_MAP_READ,0,size*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<size;i++)
		output[i]=outPutMap[i];

	clEnqueueUnmapMemObject(env.commandQueue,outputCl,outPutMap,0,NULL,NULL);
	clStatus = clFinish(env.commandQueue);

	CHECK_OPENCL(clStatus);
	clStatus = clReleaseKernel(env.kernel);
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
int OclCalc::OclHostFormulaMin(double *srcData,int *start,int *end,double *output,int size) {
	KernelEnv env;
	const char *kernelName = "oclFormulaMin";
	CheckKernelName(&env,kernelName);

	cl_int clStatus;
	size_t global_work_size[1];
	int alignSize = size + end[0]-start[0];

	env.kernel = clCreateKernel(env.program,kernelName, &clStatus);
	cl_int ret=0;
	cl_mem inputCl = clCreateBuffer(env.context,(cl_mem_flags) (CL_MEM_READ_WRITE),
		alignSize * sizeof(float), NULL, &clStatus);
	cl_mem startCl = clCreateBuffer(env.context, (cl_mem_flags) (CL_MEM_READ_WRITE),
		size * sizeof(unsigned int), NULL, &ret);
	cl_mem endCl = clCreateBuffer(env.context, (cl_mem_flags) (CL_MEM_READ_WRITE),
		size * sizeof(unsigned int), NULL, &ret);
	cl_mem outputCl = clCreateBuffer(env.context, (cl_mem_flags) (CL_MEM_READ_WRITE),
	size* sizeof(float), NULL, &ret);

	float * hostMapSrc = (float *)clEnqueueMapBuffer(env.commandQueue,inputCl,CL_TRUE,CL_MAP_WRITE,0,alignSize * sizeof(float),0,NULL,NULL,NULL);
	int * hostMapStart = (int *)clEnqueueMapBuffer(env.commandQueue,startCl,CL_TRUE,CL_MAP_WRITE,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
	int * hostMapEnd   = (int *)clEnqueueMapBuffer(env.commandQueue,endCl,CL_TRUE,CL_MAP_WRITE,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
	for(int i=0;i<size;i++)
	{
		hostMapStart[i] = start[i];
		hostMapEnd[i]	= end[i];
	}
	for(int i=0;i<alignSize;i++)
		hostMapSrc[i] = (float)srcData[i];
	clEnqueueUnmapMemObject(env.commandQueue,inputCl,hostMapSrc,0,NULL,NULL);
	clEnqueueUnmapMemObject(env.commandQueue,startCl,hostMapStart,0,NULL,NULL);
	clEnqueueUnmapMemObject(env.commandQueue,endCl,hostMapEnd,0,NULL,NULL);

	clStatus = clSetKernelArg(env.kernel, 0, sizeof(cl_mem),
		(void *)&inputCl);
	clStatus = clSetKernelArg(env.kernel, 1, sizeof(cl_mem),
		(void *)&startCl);
	clStatus = clSetKernelArg(env.kernel, 2, sizeof(cl_mem),
		(void *)&endCl);
	clStatus = clSetKernelArg(env.kernel, 3, sizeof(cl_mem),
		(void *)&outputCl);
	CHECK_OPENCL(clStatus);

	global_work_size[0] = size;
	clStatus = clEnqueueNDRangeKernel(env.commandQueue, env.kernel, 1,
		NULL, global_work_size, NULL, 0, NULL, NULL);
	CHECK_OPENCL(clStatus)

	float * outPutMap = (float *)clEnqueueMapBuffer(env.commandQueue,outputCl,CL_TRUE,CL_MAP_READ,0,size*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<size;i++)
		output[i]=outPutMap[i];

	clEnqueueUnmapMemObject(env.commandQueue,outputCl,outPutMap,0,NULL,NULL);
	clStatus = clFinish(env.commandQueue);

	CHECK_OPENCL(clStatus);
	clStatus = clReleaseKernel(env.kernel);
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
int OclCalc::OclHostFormulaAverage(double *srcData,int *start,int *end,double *output,int size) {
	KernelEnv env;
	const char *kernelName = "oclFormulaAverage";
	CheckKernelName(&env,kernelName);

	cl_int clStatus;
	size_t global_work_size[1];
	int alignSize = size + end[0]-start[0];

	env.kernel = clCreateKernel(env.program, kernelName, &clStatus);
	cl_int ret=0;
	cl_mem inputCl = clCreateBuffer(env.context,(cl_mem_flags) (CL_MEM_READ_WRITE),
		alignSize * sizeof(float), NULL, &clStatus);
	cl_mem startCl = clCreateBuffer(env.context, (cl_mem_flags) (CL_MEM_READ_WRITE),
		size * sizeof(unsigned int), NULL, &ret);
	cl_mem endCl = clCreateBuffer(env.context, (cl_mem_flags) (CL_MEM_READ_WRITE),
		size * sizeof(unsigned int), NULL, &ret);
	cl_mem outputCl = clCreateBuffer(env.context, (cl_mem_flags) (CL_MEM_READ_WRITE),
	size* sizeof(float), NULL, &ret);

	float * hostMapSrc = (float *)clEnqueueMapBuffer(env.commandQueue,inputCl,CL_TRUE,CL_MAP_WRITE,0,alignSize * sizeof(float),0,NULL,NULL,NULL);
	int * hostMapStart = (int *)clEnqueueMapBuffer(env.commandQueue,startCl,CL_TRUE,CL_MAP_WRITE,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
	int * hostMapEnd   = (int *)clEnqueueMapBuffer(env.commandQueue,endCl,CL_TRUE,CL_MAP_WRITE,0,size * sizeof(unsigned int),0,NULL,NULL,NULL);
	for(int i=0;i<size;i++)
	{
		hostMapStart[i] = start[i];
		hostMapEnd[i]	= end[i];
	}
	for(int i=0;i<alignSize;i++)
		hostMapSrc[i] = (float)srcData[i];
	clEnqueueUnmapMemObject(env.commandQueue,inputCl,hostMapSrc,0,NULL,NULL);
	clEnqueueUnmapMemObject(env.commandQueue,startCl,hostMapStart,0,NULL,NULL);
	clEnqueueUnmapMemObject(env.commandQueue,endCl,hostMapEnd,0,NULL,NULL);

	clStatus = clSetKernelArg(env.kernel, 0, sizeof(cl_mem),
		(void *)&inputCl);
	clStatus = clSetKernelArg(env.kernel, 1, sizeof(cl_mem),
		(void *)&startCl);
	clStatus = clSetKernelArg(env.kernel, 2, sizeof(cl_mem),
		(void *)&endCl);
	clStatus = clSetKernelArg(env.kernel, 3, sizeof(cl_mem),
		(void *)&outputCl);
	CHECK_OPENCL(clStatus);

	global_work_size[0] = size;
	clStatus = clEnqueueNDRangeKernel(env.commandQueue, env.kernel, 1,
		NULL, global_work_size, NULL, 0, NULL, NULL);
	CHECK_OPENCL(clStatus)

	float * outPutMap = (float *)clEnqueueMapBuffer(env.commandQueue,outputCl,CL_TRUE,CL_MAP_READ,0,size*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<size;i++)
		output[i]=outPutMap[i];

	clEnqueueUnmapMemObject(env.commandQueue,outputCl,outPutMap,0,NULL,NULL);
	clStatus = clFinish(env.commandQueue);

	CHECK_OPENCL(clStatus);
	clStatus = clReleaseKernel(env.kernel);
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

	KernelEnv env;
	int status;
	const char *kernelName = "oclSignedAdd";
	CheckKernelName(&env,kernelName);


	cl_int clStatus;
	size_t global_work_size[1];

	env.kernel = clCreateKernel(env.program,kernelName, &clStatus);
	cl_mem clLiftData = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);
	cl_mem clRightData = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);
	cl_mem clResult = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);

	float * hostMapLeftData 	= (float *)clEnqueueMapBuffer(env.commandQueue,clLiftData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	float * hostMapRightData 	= (float *)clEnqueueMapBuffer(env.commandQueue,clRightData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<dSize;i++)
	{
		hostMapLeftData[i] 	= (float)lData[i];
		hostMapRightData[i] = (float)rData[i];
	}
	clEnqueueUnmapMemObject(env.commandQueue,clLiftData,hostMapLeftData,0,NULL,NULL);
	clEnqueueUnmapMemObject(env.commandQueue,clRightData,hostMapRightData,0,NULL,NULL);

	status = clSetKernelArg(env.kernel, 0, sizeof(cl_mem),
		(void *)&clLiftData);
	status = clSetKernelArg(env.kernel, 1, sizeof(cl_mem),
		(void *)&clRightData);
	status = clSetKernelArg(env.kernel, 2, sizeof(cl_mem),
		(void *)&clResult);
	CHECK_OPENCL(status)
	global_work_size[0] = dSize;
	status = clEnqueueNDRangeKernel(env.commandQueue, env.kernel, 1,
		NULL, global_work_size, NULL, 0, NULL, NULL);
	CHECK_OPENCL(status);

	float * hostMapResult = (float *)clEnqueueMapBuffer(env.commandQueue,clResult,CL_TRUE,CL_MAP_READ,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<dSize;i++)
		rResult[i]=hostMapResult[i];
	clEnqueueUnmapMemObject(env.commandQueue,clResult,hostMapResult,0,NULL,NULL);

	CHECK_OPENCL(status);
	status = clFinish(env.commandQueue);
	CHECK_OPENCL(status);
	status = clReleaseKernel(env.kernel);
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
	KernelEnv env;
	int status;
	const char *kernelName = "oclSignedMul";
	CheckKernelName(&env,kernelName);


	size_t global_work_size[1];
	cl_int clStatus;
	env.kernel = clCreateKernel(env.program, kernelName, &clStatus);
	cl_mem clLiftData = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);
	cl_mem clRightData = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);
	cl_mem clResult = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);

	float * hostMapLeftData 	= (float *)clEnqueueMapBuffer(env.commandQueue,clLiftData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	float * hostMapRightData 	= (float *)clEnqueueMapBuffer(env.commandQueue,clRightData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<dSize;i++)
	{
		hostMapLeftData[i] 	= (float)lData[i];
		hostMapRightData[i] = (float)rData[i];
	}
	clEnqueueUnmapMemObject(env.commandQueue,clLiftData,hostMapLeftData,0,NULL,NULL);
	clEnqueueUnmapMemObject(env.commandQueue,clRightData,hostMapRightData,0,NULL,NULL);

	status = clSetKernelArg(env.kernel, 0, sizeof(cl_mem),
		(void *)&clLiftData);
	status = clSetKernelArg(env.kernel, 1, sizeof(cl_mem),
		(void *)&clRightData);
	status = clSetKernelArg(env.kernel, 2, sizeof(cl_mem),
		(void *)&clResult);
	CHECK_OPENCL(status)
	global_work_size[0] = dSize;
	status = clEnqueueNDRangeKernel(env.commandQueue, env.kernel, 1,
		NULL, global_work_size, NULL, 0, NULL, NULL);
	CHECK_OPENCL(status);

	float * hostMapResult = (float *)clEnqueueMapBuffer(env.commandQueue,clResult,CL_TRUE,CL_MAP_READ,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<dSize;i++)
		rResult[i]=hostMapResult[i];
	clEnqueueUnmapMemObject(env.commandQueue,clResult,hostMapResult,0,NULL,NULL);

	CHECK_OPENCL(status);
	status = clFinish(env.commandQueue);
	CHECK_OPENCL(status);
	status = clReleaseKernel(env.kernel);
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
	KernelEnv env;
	int status;
	const char *kernelName = "oclSignedSub";
	CheckKernelName(&env,kernelName);

	cl_int clStatus;
	size_t global_work_size[1];
	env.kernel = clCreateKernel(env.program,kernelName, &clStatus);
	cl_mem clLiftData = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);
	cl_mem clRightData = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);
	cl_mem clResult = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);

	float * hostMapLeftData 	= (float *)clEnqueueMapBuffer(env.commandQueue,clLiftData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	float * hostMapRightData 	= (float *)clEnqueueMapBuffer(env.commandQueue,clRightData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<dSize;i++)
	{
		hostMapLeftData[i] 	= (float)lData[i];
		hostMapRightData[i] = (float)rData[i];
	}
	clEnqueueUnmapMemObject(env.commandQueue,clLiftData,hostMapLeftData,0,NULL,NULL);
	clEnqueueUnmapMemObject(env.commandQueue,clRightData,hostMapRightData,0,NULL,NULL);

	status = clSetKernelArg(env.kernel, 0, sizeof(cl_mem),
		(void *)&clLiftData);
	status = clSetKernelArg(env.kernel, 1, sizeof(cl_mem),
		(void *)&clRightData);
	status = clSetKernelArg(env.kernel, 2, sizeof(cl_mem),
		(void *)&clResult);
	CHECK_OPENCL(status)
	global_work_size[0] = dSize;
	status = clEnqueueNDRangeKernel(env.commandQueue, env.kernel, 1,
		NULL, global_work_size, NULL, 0, NULL, NULL);
	CHECK_OPENCL(status);

	float * hostMapResult = (float *)clEnqueueMapBuffer(env.commandQueue,clResult,CL_TRUE,CL_MAP_READ,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<dSize;i++)
		rResult[i]=hostMapResult[i];
	clEnqueueUnmapMemObject(env.commandQueue,clResult,hostMapResult,0,NULL,NULL);

	CHECK_OPENCL(status);
	status = clFinish(env.commandQueue);
	CHECK_OPENCL(status);
	status = clReleaseKernel(env.kernel);
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
	KernelEnv env;
	int status;
	const char *kernelName = "oclSignedDiv";
	CheckKernelName(&env,kernelName);


	size_t global_work_size[1];
	cl_int clStatus;
	env.kernel = clCreateKernel(env.program,kernelName, &clStatus);
	cl_mem clLiftData = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);
	cl_mem clRightData = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);
	cl_mem clResult = clCreateBuffer(env.context,
		(cl_mem_flags) (CL_MEM_READ_WRITE),
		dSize * sizeof(float), NULL, &clStatus);

	float * hostMapLeftData 	= (float *)clEnqueueMapBuffer(env.commandQueue,clLiftData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	float * hostMapRightData 	= (float *)clEnqueueMapBuffer(env.commandQueue,clRightData,CL_TRUE,CL_MAP_WRITE,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<dSize;i++)
	{
		hostMapLeftData[i] 	= (float)lData[i];
		hostMapRightData[i] = (float)rData[i];
	}
	clEnqueueUnmapMemObject(env.commandQueue,clLiftData,hostMapLeftData,0,NULL,NULL);
	clEnqueueUnmapMemObject(env.commandQueue,clRightData,hostMapRightData,0,NULL,NULL);

	status = clSetKernelArg(env.kernel, 0, sizeof(cl_mem),
		(void *)&clLiftData);
	status = clSetKernelArg(env.kernel, 1, sizeof(cl_mem),
		(void *)&clRightData);
	status = clSetKernelArg(env.kernel, 2, sizeof(cl_mem),
		(void *)&clResult);
	CHECK_OPENCL(status)
	global_work_size[0] = dSize;
	status = clEnqueueNDRangeKernel(env.commandQueue, env.kernel, 1,
		NULL, global_work_size, NULL, 0, NULL, NULL);
	CHECK_OPENCL(status);

	float * hostMapResult = (float *)clEnqueueMapBuffer(env.commandQueue,clResult,CL_TRUE,CL_MAP_READ,0,dSize*sizeof(float),0,NULL,NULL,NULL);
	for(int i=0;i<dSize;i++)
		rResult[i]=hostMapResult[i];
	clEnqueueUnmapMemObject(env.commandQueue,clResult,hostMapResult,0,NULL,NULL);

	CHECK_OPENCL(status);
	status = clFinish(env.commandQueue);
	CHECK_OPENCL(status);
	status = clReleaseKernel(env.kernel);
	CHECK_OPENCL(status);
	status = clReleaseMemObject(clLiftData);
	CHECK_OPENCL(status);
	status = clReleaseMemObject(clRightData);
	CHECK_OPENCL(status);
	status = clReleaseMemObject(clResult);
	CHECK_OPENCL(status);
	return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
