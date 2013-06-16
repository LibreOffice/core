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

#include "random.hxx"
#include "openclwrapper.hxx"
#include "oclkernels.hxx"


inline int OpenclDevice::add_kernel_cfg(int kCount, const char *kName) {
    strcpy(gpu_env.kernel_names[kCount], kName);
    gpu_env.kernel_count++;
    return 0;
}

int OpenclDevice::regist_opencl_kernel() {
    if (!gpu_env.isUserCreated) {
        memset(&gpu_env, 0, sizeof(gpu_env));
    }

    gpu_env.file_count = 0; //argc;
    gpu_env.kernel_count = 0UL;

    add_kernel_cfg(0, (const char*) "hello");
    add_kernel_cfg(1, (const char*) "oclformula");
    add_kernel_cfg(2, (const char*) "oclFormulaMin");
    add_kernel_cfg(3, (const char*) "oclFormulaMax");
    add_kernel_cfg(4, (const char*) "oclFormulaSum");
    add_kernel_cfg(5, (const char*) "oclFormulaCount");
    add_kernel_cfg(6, (const char*) "oclFormulaAverage");
    add_kernel_cfg(7, (const char*) "oclFormulaSumproduct");
    add_kernel_cfg(8, (const char*) "oclFormulaMinverse");
    return 0;
}
OpenclDevice::OpenclDevice() :
        isInited(0) {

}

OpenclDevice::~OpenclDevice() {

}
#ifdef USE_KERNEL_FILE
int OpenclDevice::convert_to_string(const char *filename, char **source) {
    int file_size;
    size_t result;
    FILE *file = NULL;

    file_size = 0;
    result = 0;
    file = fopen(filename, "rb+");
    printf("open kernel file %s.\n", filename);

    if (file != NULL) {
        fseek(file, 0, SEEK_END);

        file_size = ftell(file);
        rewind(file);
        *source = (char*) malloc(sizeof(char) * file_size + 1);
        if (*source == (char*) NULL) {
            return (0);
        }
        result = fread(*source, 1, file_size, file);
        if (result != (size_t) file_size) {
            free(*source);
            return (0);
        }
        (*source)[file_size] = '\0';
        fclose(file);

        return (1);
    }
    printf("open kernel file failed.\n");
    return (0);
}
#endif
int OpenclDevice::binary_generated(cl_context context,
        const char * cl_file_name, FILE ** fhandle) {
    unsigned int i = 0;
    cl_int status;

    size_t numDevices;

    cl_device_id *devices;

    char *str = NULL;

    FILE *fd = NULL;

    status = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES,
            sizeof(numDevices), &numDevices, NULL);

    CHECK_OPENCL(status)

    devices = (cl_device_id*) malloc(sizeof(cl_device_id) * numDevices);

    if (devices == NULL) {
        return 0;
    }

    /* grab the handles to all of the devices in the context. */
    status = clGetContextInfo(context, CL_CONTEXT_DEVICES,
            sizeof(cl_device_id) * numDevices, devices, NULL);

    status = 0;
    /* dump out each binary into its own separate file. */
    for (i = 0; i < numDevices; i++) {
        char fileName[256] = { 0 }, cl_name[128] = { 0 };

        if (devices[i] != 0) {
            char deviceName[1024];
            status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME,
                    sizeof(deviceName), deviceName, NULL);
            CHECK_OPENCL(status)
            str = (char*) strstr(cl_file_name, (char*) ".cl");
            memcpy(cl_name, cl_file_name, str - cl_file_name);
            cl_name[str - cl_file_name] = '\0';
            sprintf(fileName, "./%s-%s.bin", cl_name, deviceName);
            fd = fopen(fileName, "rb");
            status = (fd != NULL) ? 1 : 0;
        }

    }

    if (devices != NULL) {
        free(devices);
        devices = NULL;
    }

    if (fd != NULL) {
        *fhandle = fd;
    }

    return status;
}

int OpenclDevice::write_binary_to_file(const char* fileName, const char* birary,
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

int OpenclDevice::generat_bin_from_kernel_source(cl_program program,
        const char * cl_file_name) {
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

            str = (char*) strstr(cl_file_name, (char*) ".cl");
            memcpy(cl_name, cl_file_name, str - cl_file_name);
            cl_name[str - cl_file_name] = '\0';
            sprintf(fileName, "./%s-%s.bin", cl_name, deviceName);

            if (!write_binary_to_file(fileName, binaries[i], binarySizes[i])) {
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

int OpenclDevice::init_opencl_attr(OpenCLEnv * env) {
    if (gpu_env.isUserCreated) {
        return 1;
    }

    gpu_env.context = env->context;
    gpu_env.platform = env->platform;
    gpu_env.dev = env->devices;
    gpu_env.command_queue = env->command_queue;

    gpu_env.isUserCreated = 1;

    return 0;
}

int OpenclDevice::create_kernel(char * kernelname, KernelEnv * env) {
    int status;

    env->kernel = clCreateKernel(gpu_env.programs[0], kernelname, &status);
    env->context = gpu_env.context;
    env->command_queue = gpu_env.command_queue;
    return status != CL_SUCCESS ? 1 : 0;
}

int OpenclDevice::release_kernel(KernelEnv * env) {
    int status = clReleaseKernel(env->kernel);
    return status != CL_SUCCESS ? 1 : 0;
}

int OpenclDevice::init_opencl_env(GPUEnv *gpu_info) {
    size_t length;
    cl_int status;
    cl_uint numPlatforms, numDevices;
    cl_platform_id *platforms;
    cl_context_properties cps[3];
    char platformName[100];
    unsigned int i;

    /*
     * Have a look at the available platforms.
     */
    if (!gpu_info->isUserCreated) {
        status = clGetPlatformIDs(0, NULL, &numPlatforms);
        if (status != CL_SUCCESS) {
            return (1);
        }
        gpu_info->platform = NULL;
        ;
        if (0 < numPlatforms) {
            platforms = (cl_platform_id*) malloc(
                    numPlatforms * sizeof(cl_platform_id));
            if (platforms == (cl_platform_id*) NULL) {
                return (1);
            }
            status = clGetPlatformIDs(numPlatforms, platforms, NULL);

            if (status != CL_SUCCESS) {
                return (1);
            }

            for (i = 0; i < numPlatforms; i++) {
                status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR,
                        sizeof(platformName), platformName, NULL);

                if (status != CL_SUCCESS) {
                    return (1);
                }
                gpu_info->platform = platforms[i];

                //if (!strcmp(platformName, "Intel(R) Coporation"))
                //if( !strcmp( platformName, "Advanced Micro Devices, Inc." ))
                {
                    gpu_info->platform = platforms[i];

                    status = clGetDeviceIDs(gpu_info->platform /* platform */,
                            CL_DEVICE_TYPE_GPU /* device_type */,
                            0 /* num_entries */, NULL /* devices */,
                            &numDevices);

                    if (status != CL_SUCCESS) {
                        return (1);
                    }

                    if (numDevices) {
                        break;
                    }
                }
            }
            free(platforms);
        }
        if (NULL == gpu_info->platform) {
            return (1);
        }

        /*
         * Use available platform.
         */
        cps[0] = CL_CONTEXT_PLATFORM;
        cps[1] = (cl_context_properties) gpu_info->platform;
        cps[2] = 0;
        /* Check for GPU. */
        gpu_info->dType = CL_DEVICE_TYPE_GPU;
        gpu_info->context = clCreateContextFromType(cps, gpu_info->dType, NULL,
                NULL, &status);

        if ((gpu_info->context == (cl_context) NULL)
                || (status != CL_SUCCESS)) {
            gpu_info->dType = CL_DEVICE_TYPE_CPU;
            gpu_info->context = clCreateContextFromType(cps, gpu_info->dType,
                    NULL, NULL, &status);
        }
        if ((gpu_info->context == (cl_context) NULL)
                || (status != CL_SUCCESS)) {
            gpu_info->dType = CL_DEVICE_TYPE_DEFAULT;
            gpu_info->context = clCreateContextFromType(cps, gpu_info->dType,
                    NULL, NULL, &status);
        }
        if ((gpu_info->context == (cl_context) NULL)
                || (status != CL_SUCCESS)) {
            return (1);
        }
        /* Detect OpenCL devices. */
        /* First, get the size of device list data */
        status = clGetContextInfo(gpu_info->context, CL_CONTEXT_DEVICES, 0,
                NULL, &length);

        if ((status != CL_SUCCESS) || (length == 0)) {
            return (1);
        }
        /* Now allocate memory for device list based on the size we got earlier */
        gpu_info->devices = (cl_device_id*) malloc(length);
        if (gpu_info->devices == (cl_device_id*) NULL) {
            return (1);
        }
        /* Now, get the device list data */
        status = clGetContextInfo(gpu_info->context, CL_CONTEXT_DEVICES, length,
                gpu_info->devices, NULL);

        if (status != CL_SUCCESS) {
            return (1);
        }

        /* Create OpenCL command queue. */
        gpu_info->command_queue = clCreateCommandQueue(gpu_info->context,
                gpu_info->devices[0], 0, &status);

        if (status != CL_SUCCESS) {
            return (1);
        }
    }

    status = clGetCommandQueueInfo(gpu_info->command_queue,
            CL_QUEUE_THREAD_HANDLE_AMD, 0, NULL, NULL);

    return 0;
}

int OpenclDevice::release_opencl_env(GPUEnv *gpu_info) {
    int i = 0;
    int status = 0;

    if (!isInited) {
        return 1;
    }

    for (i = 0; i < gpu_env.file_count; i++) {
        if (gpu_env.programs[i]) {
            status = clReleaseProgram(gpu_env.programs[i]);
            CHECK_OPENCL(status)
            gpu_env.programs[i] = NULL;
        }
    }
    if (gpu_env.command_queue) {
        clReleaseCommandQueue(gpu_env.command_queue);
        gpu_env.command_queue = NULL;
    }
    if (gpu_env.context) {
        clReleaseContext(gpu_env.context);
        gpu_env.context = NULL;
    }
    isInited = 0;
    gpu_info->isUserCreated = 0;
    free(gpu_info->devices);
    return 1;
}

int OpenclDevice::run_kernel_wrapper(cl_kernel_function function,
        char * kernel_name, void **usrdata) {
    printf("oclwrapper:run_kernel_wrapper...\n");
    if (register_kernel_wrapper(kernel_name, function) != 1) {
        fprintf(stderr,
                "Error:run_kernel_wrapper:register_kernel_wrapper fail!\n");
        return -1;
    }
    return (run_kernel(kernel_name, usrdata));
}

int OpenclDevice::register_kernel_wrapper(const char *kernel_name,
        cl_kernel_function function) {
    int i;
    printf("oclwrapper:register_kernel_wrapper...%d\n", gpu_env.kernel_count);
    for (i = 0; i < gpu_env.kernel_count; i++) {
        //printf("oclwrapper:register_kernel_wrapper kname...%s\n", kernel_name);
        //printf("oclwrapper:register_kernel_wrapper kname...%s\n", gpu_env.kernel_names[i]);
        if (strcasecmp(kernel_name, gpu_env.kernel_names[i]) == 0) {
            //printf("oclwrapper:register_kernel_wrapper if()...\n");
            gpu_env.kernel_functions[i] = function;
            return (1);
        }
    }
    return (0);
}

int OpenclDevice::cached_of_kerner_prg(const GPUEnv *gpu_env_cached,
        const char * cl_file_name) {
    int i;
    for (i = 0; i < gpu_env_cached->file_count; i++) {
        if (strcasecmp(gpu_env_cached->kernelSrcFile[i], cl_file_name) == 0) {
            if (gpu_env_cached->programs[i] != NULL) {
                return (1);
            }
        }
    }

    return (0);
}

int OpenclDevice::compile_kernel_file(GPUEnv *gpu_info, const char *build_option) {
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
    if (cached_of_kerner_prg(gpu_info, filename) == 1) {
        return (1);
    }

    idx = gpu_info->file_count;

    source = kernel_src;

    source_size[0] = strlen(source);

    binaryExisted = 0;
    if ((binaryExisted = binary_generated(gpu_info->context, filename, &fd)) == 1) {
        status = clGetContextInfo(gpu_info->context, CL_CONTEXT_NUM_DEVICES,
                sizeof(numDevices), &numDevices, NULL);
        CHECK_OPENCL(status)

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
        status = clGetContextInfo(gpu_info->context, CL_CONTEXT_DEVICES,
                sizeof(cl_device_id) * numDevices, devices, NULL);
        CHECK_OPENCL(status)

        gpu_info->programs[idx] = clCreateProgramWithBinary(gpu_info->context,
                numDevices, devices, &length, (const unsigned char**) &binary,
                &binary_status, &status);
        CHECK_OPENCL(status)

        free(binary);
        free(devices);
        devices = NULL;
    } else {

        // create a CL program using the kernel source
        gpu_info->programs[idx] = clCreateProgramWithSource(gpu_info->context,
                1, &source, source_size, &status);
        CHECK_OPENCL(status)

        printf("clCreateProgramWithSource.\n");
    }

    if (gpu_info->programs[idx] == (cl_program) NULL) {
        return (0);
    }

    //char options[512];
    // create a cl program executable for all the devices specified
    if (!gpu_info->isUserCreated) {
        status = clBuildProgram(gpu_info->programs[idx], 1, gpu_info->devices,
                build_option, NULL, NULL);
        CHECK_OPENCL(status)
    } else {
        status = clBuildProgram(gpu_info->programs[idx], 1, &(gpu_info->dev),
                build_option, NULL, NULL);
        CHECK_OPENCL(status)
    }
    printf("BuildProgram.\n");

    if (status != CL_SUCCESS) {
        if (!gpu_info->isUserCreated) {
            status = clGetProgramBuildInfo(gpu_info->programs[idx],
                    gpu_info->devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL,
                    &length);
        } else {
            status = clGetProgramBuildInfo(gpu_info->programs[idx],
                    gpu_info->dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &length);
        }
        if (status != CL_SUCCESS) {
            printf("opencl create build log fail\n");
            return (0);
        }
        buildLog = (char*) malloc(length);
        if (buildLog == (char*) NULL) {
            return (0);
        }
        if (!gpu_info->isUserCreated) {
            status = clGetProgramBuildInfo(gpu_info->programs[idx],
                    gpu_info->devices[0], CL_PROGRAM_BUILD_LOG, length,
                    buildLog, &length);
        } else {
            status = clGetProgramBuildInfo(gpu_info->programs[idx],
                    gpu_info->dev, CL_PROGRAM_BUILD_LOG, length, buildLog,
                    &length);
        }

        fd1 = fopen("kernel-build.log", "w+");
        if (fd1 != NULL) {
            fwrite(buildLog, sizeof(char), length, fd1);
            fclose(fd1);
        }

        free(buildLog);
        return (0);
    }

    strcpy(gpu_env.kernelSrcFile[idx], filename);

    if (binaryExisted == 0)
        generat_bin_from_kernel_source(gpu_env.programs[idx], filename);

    gpu_info->file_count += 1;

    return (1);
}

int OpenclDevice::compile_kernel_file(const char *filename, GPUEnv *gpu_info,
        const char *build_option) {
    cl_int status;

    size_t length;

#ifdef USE_KERNEL_FILE
    char
    *source_str;
#endif
    char *buildLog = NULL, *binary;

    const char *source;
    size_t source_size[1];

    int b_error, binary_status, binaryExisted, idx;

    size_t numDevices;

    cl_device_id *devices;

    FILE *fd, *fd1;

    if (cached_of_kerner_prg(gpu_info, filename) == 1) {
        return (1);
    }

    idx = gpu_info->file_count;
#ifdef USE_KERNEL_FILE
    status = convert_to_string( filename, &source_str, gpu_info, idx );

    if( status == 0 )
    {
        printf("convert_to_string failed.\n");
        return(0);
    }
    source = source_str;
#else

    source = kernel_src;
#endif
    source_size[0] = strlen(source);

    binaryExisted = 0;
    if ((binaryExisted = binary_generated(gpu_info->context, filename, &fd))
            == 1) {
        status = clGetContextInfo(gpu_info->context, CL_CONTEXT_NUM_DEVICES,
                sizeof(numDevices), &numDevices, NULL);
        CHECK_OPENCL(status)

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
        /* grab the handles to all of the devices in the context. */
        status = clGetContextInfo(gpu_info->context, CL_CONTEXT_DEVICES,
                sizeof(cl_device_id) * numDevices, devices, NULL);
        CHECK_OPENCL(status)

        gpu_info->programs[idx] = clCreateProgramWithBinary(gpu_info->context,
                numDevices, devices, &length, (const unsigned char**) &binary,
                &binary_status, &status);
        CHECK_OPENCL(status)

        free(binary);
        free(devices);
        devices = NULL;
    } else {

        // create a CL program using the kernel source
        gpu_info->programs[idx] = clCreateProgramWithSource(gpu_info->context,
                1, &source, source_size, &status);
        CHECK_OPENCL(status)
#ifdef USE_KERNEL_FILE
        free((char*)source);
#endif
        printf("clCreateProgramWithSource.\n");
    }

    if (gpu_info->programs[idx] == (cl_program) NULL) {
        return (0);
    }

    //char options[512];
    // create a cl program executable for all the devices specified
    if (!gpu_info->isUserCreated) {
        status = clBuildProgram(gpu_info->programs[idx], 1, gpu_info->devices,
                build_option, NULL, NULL);
        CHECK_OPENCL(status)
    } else {
        status = clBuildProgram(gpu_info->programs[idx], 1, &(gpu_info->dev),
                build_option, NULL, NULL);
        CHECK_OPENCL(status)
    }
    printf("BuildProgram.\n");

    if (status != CL_SUCCESS) {
        if (!gpu_info->isUserCreated) {
            status = clGetProgramBuildInfo(gpu_info->programs[idx],
                    gpu_info->devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL,
                    &length);
        } else {
            status = clGetProgramBuildInfo(gpu_info->programs[idx],
                    gpu_info->dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &length);
        }
        if (status != CL_SUCCESS) {
            printf("opencl create build log fail\n");
            return (0);
        }
        buildLog = (char*) malloc(length);
        if (buildLog == (char*) NULL) {
            return (0);
        }
        if (!gpu_info->isUserCreated) {
            status = clGetProgramBuildInfo(gpu_info->programs[idx],
                    gpu_info->devices[0], CL_PROGRAM_BUILD_LOG, length,
                    buildLog, &length);
        } else {
            status = clGetProgramBuildInfo(gpu_info->programs[idx],
                    gpu_info->dev, CL_PROGRAM_BUILD_LOG, length, buildLog,
                    &length);
        }

        fd1 = fopen("kernel-build.log", "w+");
        if (fd1 != NULL) {
            fwrite(buildLog, sizeof(char), length, fd1);
            fclose(fd1);
        }

        free(buildLog);
        return (0);
    }

    strcpy(gpu_env.kernelSrcFile[idx], filename);

    if (binaryExisted == 0)
        generat_bin_from_kernel_source(gpu_env.programs[idx], filename);

    gpu_info->file_count += 1;

    return (1);
}

int OpenclDevice::get_kernel_env_and_func(const char *kernel_name,
        KernelEnv *env, cl_kernel_function *function) {
    int i; //,program_idx ;
    for (i = 0; i < gpu_env.kernel_count; i++) {
        if (strcasecmp(kernel_name, gpu_env.kernel_names[i]) == 0) {
            env->context = gpu_env.context;
            env->command_queue = gpu_env.command_queue;
            env->program = gpu_env.programs[0];
            env->kernel = gpu_env.kernels[i];
            *function = gpu_env.kernel_functions[i];
            return (1);
        }
    }
    return (0);
}

int OpenclDevice::run_kernel(const char *kernel_name, void **userdata) {
    KernelEnv env;

    cl_kernel_function function;

    int status;

    memset(&env, 0, sizeof(KernelEnv));
    status = get_kernel_env_and_func(kernel_name, &env, &function);
    strcpy(env.kernel_name, kernel_name);
    if (status == 1) {
        if (&env == (KernelEnv *) NULL
                || &function == (cl_kernel_function *) NULL) {
            return (0);
        }
        return (function(userdata, &env));
    }
    return (0);
}

int OpenclDevice::init_opencl_run_env(int argc, const char *build_option_kernelfiles)
{
    int status = 0;

    if (MAX_CLKERNEL_NUM <= 0) {
        return 1;
    }
    if ((argc > MAX_CLFILE_NUM) || (argc < 0)) {
        return 1;
    }

    if (!isInited) {
        printf("regist_opencl_kernel start.\n");
        regist_opencl_kernel();
        //initialize devices, context, comand_queue
        status = init_opencl_env(&gpu_env);
        if (status) {
            printf("init_opencl_env failed.\n");
            return (1);
        }
        printf("init_opencl_env successed.\n");
        //initialize program, kernel_name, kernel_count
        status = compile_kernel_file( &gpu_env, build_option_kernelfiles);
        if (status == 0 || gpu_env.kernel_count == 0) {
            printf("compile_kernel_file failed.\n");
            return (1);
        }
        printf("compile_kernel_file successed.\n");
        isInited = 1;
    }

    return (0);
}

int OpenclDevice::init_opencl_run_env(int argc, const char *argv_kernelfiles[],
        const char *build_option_kernelfiles) {
    int status = 0;

    if (MAX_CLKERNEL_NUM <= 0) {
        return 1;
    }
    if ((argc > MAX_CLFILE_NUM) || (argc < 0)) {
        return 1;
    }

    if (!isInited) {
        printf("regist_opencl_kernel start.\n");
        regist_opencl_kernel();
        //initialize devices, context, comand_queue
        status = init_opencl_env(&gpu_env);
        if (status) {
            printf("init_opencl_env failed.\n");
            return (1);
        }
        printf("init_opencl_env successed.\n");
        //initialize program, kernel_name, kernel_count
        status = compile_kernel_file(argv_kernelfiles[0], &gpu_env,
                build_option_kernelfiles);
        if (status == 0 || gpu_env.kernel_count == 0) {
            printf("compile_kernel_file failed.\n");
            return (1);
        }
        printf("compile_kernel_file successed.\n");
        isInited = 1;
    }

    return (0);
}

int OpenclDevice::release_opencl_run_env() {
    return release_opencl_env(&gpu_env);
}

void OpenclDevice::setOpenclState(int state) {
     isInited = state;
}

int OpenclDevice::getOpenclState() {
    return isInited;
}
//ocldbg
int OclFormulaMin(void ** usrdata, KernelEnv *env) { return 0; }
int OclFormulaMax(void ** usrdata, KernelEnv *env) { return 0; }
int OclFormulaSum(void ** usrdata, KernelEnv *env) { return 0; }
int OclFormulaCount(void ** usrdata, KernelEnv *env) { return 0; }
int OclFormulaAverage(void ** usrdata, KernelEnv *env) { return 0; }
int OclFormulaSumproduct(void ** usrdata, KernelEnv *env) { return 0; }
int OclFormulaMinverse(void ** usrdata, KernelEnv *env) { return 0; }

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
        status = clEnqueueNDRangeKernel(env->command_queue, env->kernel, 1,
                NULL, global_work_size, NULL, 0, NULL, NULL);
        CHECK_OPENCL(status)

    }
    //fprintf(stderr, "\nIn OpenclDevice,...before clEnqueueReadBuffer\n");
    status = clEnqueueReadBuffer(env->command_queue, formula_data, CL_FALSE, 0,
            sizeof(float), (void *) &tdata, 0, NULL, NULL);
    CHECK_OPENCL(status)
    status = clFinish(env->command_queue);
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
double OclCalc::OclProcess(cl_kernel_function function, double *data,
        formulax type) {
    fprintf(stderr, "\In OpenclDevice, proc...begin\n");
    double ret = 0;

    void *usrdata[2];

    usrdata[0] = (void *) data;
    usrdata[1] = (void *) &type;

    run_kernel_wrapper(function, "oclformula", usrdata);
    //fprintf(stderr, "\In OpenclDevice, proc...after run_kernel_wrapper\n");
    return ret;
}

double OclCalc::OclTest() {
    double data[NUM];

    for (int i = 0; i < NUM; i++) {
        data[i] = sc::rng::uniform();
        fprintf(stderr, "%f\t", data[i]);
    }
    OclProcess(&OclFormulax, data, AVG);
    //fprintf(stderr, "\nIn OpenclDevice,OclTest() after proc,data0...%f\n", data[0]);

    return 0.0;
}

OclCalc::OclCalc()
{
    OpenclDevice::init_opencl_run_env(0, NULL);
    OpenclDevice::setOpenclState(1);
    fprintf(stderr,"OclCalc:: init opencl.\n");
}

OclCalc::~OclCalc()
{
    OpenclDevice::release_opencl_run_env();
    OpenclDevice::setOpenclState(0);
    fprintf(stderr,"OclCalc:: opencl end.\n");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
