/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _OPENCL_WRAPPER_H_
#define _OPENCL_WRAPPER_H_

#include <CL/cl.h>

#define MaxTextExtent  4096
//support AMD opencl
#define CL_QUEUE_THREAD_HANDLE_AMD 0x403E
#define CL_MAP_WRITE_INVALIDATE_REGION (1 << 2)

#if defined(_MSC_VER)
#ifndef strcasecmp
#define strcasecmp strcmp
#endif
#endif

typedef struct _KernelEnv {
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;
    cl_kernel kernel;
    char kernel_name[150];
} KernelEnv;

typedef struct _OpenCLEnv {
    cl_platform_id platform;
    cl_context context;
    cl_device_id devices;
    cl_command_queue command_queue;
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
    printf ("error code is %d.",status);  \
    return (0);                           \
}
#endif

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

    cl_command_queue command_queue;

    cl_kernel kernels[MAX_CLFILE_NUM];

    cl_program programs[MAX_CLFILE_NUM]; //one program object maps one kernel source file

    char kernelSrcFile[MAX_CLFILE_NUM][256], //the max len of kernel file name is 256
         kernel_names[MAX_CLKERNEL_NUM][MAX_KERNEL_STRING_LEN + 1];

    cl_kernel_function kernel_functions[MAX_CLKERNEL_NUM];

    int kernel_count, file_count, // only one kernel file
        isUserCreated; // 1: created , 0:no create and needed to create by opencl wrapper

} GPUEnv;

typedef struct {
    char kernelName[MAX_KERNEL_NAME_LEN + 1];
    char *kernelStr;
} kernel_node;

class OpenclDevice {
private:
    GPUEnv gpu_env;
    int isInited;

public:
    OpenclDevice();
    ~OpenclDevice();
    int regist_opencl_kernel();
    int convert_to_string(const char *filename, char **source);
    int binary_generated(cl_context context, const char * cl_file_name,
            FILE ** fhandle);
    int write_binary_to_file(const char* fileName, const char* birary,
            size_t numBytes);
    int generat_bin_from_kernel_source(cl_program program,
            const char * cl_file_name);
    int init_opencl_attr(OpenCLEnv * env);
    int create_kernel(char * kernelname, KernelEnv * env);
    int release_kernel(KernelEnv * env);
    int init_opencl_env(GPUEnv *gpu_info);
    int release_opencl_env(GPUEnv *gpu_info);
    int run_kernel_wrapper(cl_kernel_function function, char * kernel_name,
            void **usrdata);
    int register_kernel_wrapper(const char *kernel_name,
            cl_kernel_function function);
    int cached_of_kerner_prg(const GPUEnv *gpu_env_cached,
            const char * cl_file_name);
    int compile_kernel_file(GPUEnv *gpu_info, const char *build_option);
    int compile_kernel_file(const char *filename, GPUEnv *gpu_info,
            const char *build_option);
    int get_kernel_env_and_func(const char *kernel_name, KernelEnv *env,
            cl_kernel_function *function);
    int run_kernel(const char *kernel_name, void **userdata);
    int init_opencl_run_env(int argc, const char *build_option_kernelfiles);
    int init_opencl_run_env(int argc, const char *argv_kernelfiles[],
            const char *build_option_kernelfiles);
    int release_opencl_run_env();
    void setOpenclState(int state);
    int getOpenclState();
    inline int add_kernel_cfg(int kCount, const char *kName);

};

#define NUM 4//(16*16*16)
typedef enum _formulax_ {
    MIN, MAX, SUM, AVG, COUNT, SUMPRODUCT, MINVERSE
} formulax;
class OclCalc: public OpenclDevice {
public:
    OclCalc();
    ~OclCalc();
    double OclProcess(cl_kernel_function function, double *data, formulax type);
    double OclTest();
    double OclMin();
    double OclMax();
    double OclSum();
    double OclCount();
    double OclAverage();
    double OclSumproduct();
    double OclMinverse();

};

