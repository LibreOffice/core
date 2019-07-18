/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <clew/clew.h>

#include <vector>
#include <cassert>
#include <cstring>
#include <iostream>

#include <sal/log.hxx>

#include <clew_setup.hxx>

using namespace std;

// The purpose of this separate executable is to check whether OpenCL works
// without crashing (asserting, etc.). Other checks can be done by LO core itself.

#define check(value, expected)                                                                     \
    do                                                                                             \
    {                                                                                              \
        auto val = (value);                                                                        \
        if (val != (expected))                                                                     \
        {                                                                                          \
            SAL_WARN("opencl",                                                                     \
                     "OpenCL driver check failed: " << val << "(line " << __LINE__ << ")");        \
            return;                                                                                \
        }                                                                                          \
    } while (false);
#define openclcheck(value) check(value, CL_SUCCESS)

static void runTest(const char* deviceName, const char* devicePlatform)
{
    int status = clewInit(OPENCL_DLL_NAME);
    check(status, CLEW_SUCCESS);

    // Find the given OpenCL device (in order to use the same one as LO core).
    cl_uint numPlatforms;
    openclcheck(clGetPlatformIDs(0, nullptr, &numPlatforms));
    vector<cl_platform_id> platforms(numPlatforms);
    openclcheck(clGetPlatformIDs(numPlatforms, platforms.data(), nullptr));
    cl_platform_id platformId = nullptr;
    for (cl_uint i = 0; i < numPlatforms; ++i)
    {
        char platformName[64];
        if (clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 64, platformName, nullptr)
                == CL_SUCCESS
            && strcmp(devicePlatform, platformName) == 0)
        {
            platformId = platforms[i];
            break;
        }
    }
    if (platformId == nullptr)
    {
        SAL_WARN("opencl", "Device platform not found: " << devicePlatform);
        assert(false);
        return;
    }

    cl_uint numDevices;
    openclcheck(clGetDeviceIDs(platformId, CL_DEVICE_TYPE_ALL, 0, nullptr, &numDevices));
    vector<cl_device_id> devices(numDevices);
    openclcheck(
        clGetDeviceIDs(platformId, CL_DEVICE_TYPE_ALL, numDevices, devices.data(), nullptr));
    cl_device_id deviceId = nullptr;
    for (cl_uint i = 0; i < numDevices; ++i)
    {
        char name[1024];
        if (clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 64, name, nullptr) == CL_SUCCESS
            && strcmp(deviceName, name) == 0)
        {
            deviceId = devices[i];
            break;
        }
    }
    if (deviceId == nullptr)
    {
        SAL_WARN("opencl", "Device not found: " << deviceName);
        assert(false);
        return;
    }

    cl_context context;
    cl_int state;
    cl_context_properties cps[3];
    cps[0] = CL_CONTEXT_PLATFORM;
    cps[1] = reinterpret_cast<cl_context_properties>(platformId);
    cps[2] = 0;
    context = clCreateContext(cps, 1, &deviceId, nullptr, nullptr, &state);
    openclcheck(state);
    cl_command_queue queue = clCreateCommandQueue(context, deviceId, 0, &state);
    openclcheck(state);

    // Just a simple OpenCL program, the functionality or results do not really matter.
    const char* source[] = { "__kernel void testFunction( __global float* input1, __global float* "
                             "input2, __global float* output )"
                             "{"
                             "    int gid0 = get_global_id( 0 );"
                             "    output[ gid0 ] = input1[ gid0 ] * input2[ gid0 ];"
                             "}" };
    size_t sourceSize[] = { strlen(source[0]) };
    cl_program program = clCreateProgramWithSource(context, 1, source, sourceSize, &state);
    openclcheck(state);
    state = clBuildProgram(program, 1, &deviceId, nullptr, nullptr, nullptr);
#ifdef DBG_UTIL
    if (state != CL_SUCCESS)
    {
        size_t length;
        status
            = clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, 0, nullptr, &length);
        vector<char> error(length + 1);
        status = clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, length,
                                       error.data(), nullptr);
        error[length] = '\0';
        cerr << "OpenCL driver check build error:" << error.data() << endl;
        abort();
    }
#endif
    openclcheck(state);
    cl_kernel kernel = clCreateKernel(program, "testFunction", &state);
    openclcheck(state);

    // Some random data for the program.
    constexpr int dataSize = 1000;
    cl_float inputData1[dataSize];
    cl_float inputData2[dataSize];
    cl_float outputData[dataSize];
    for (int i = 0; i < dataSize; ++i)
    {
        inputData1[i] = i * 2;
        inputData2[i] = i % 100;
    }
    cl_mem input1 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                   sizeof(cl_float) * dataSize, inputData1, &state);
    openclcheck(state);
    cl_mem input2 = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                   sizeof(cl_float) * dataSize, inputData2, &state);
    openclcheck(state);
    cl_mem output = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                   sizeof(cl_float) * dataSize, outputData, &state);
    openclcheck(state);
    state = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input1);
    openclcheck(state);
    state = clSetKernelArg(kernel, 1, sizeof(cl_mem), &input2);
    openclcheck(state);
    state = clSetKernelArg(kernel, 2, sizeof(cl_mem), &output);
    openclcheck(state);

    const size_t globalWorkSize[] = { dataSize };
    const size_t localSize[1] = { 64 };
    state = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, globalWorkSize, localSize, 0, nullptr,
                                   nullptr);
    openclcheck(state);
    openclcheck(clFinish(queue));
    openclcheck(clEnqueueReadBuffer(queue, output, CL_TRUE, 0, sizeof(cl_float) * dataSize,
                                    outputData, 0, nullptr, nullptr));
    clReleaseMemObject(input1);
    clReleaseMemObject(input2);
    clReleaseMemObject(output);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
        return 1;
    runTest(argv[1], argv[2]);
    // Always return exit code 0, LO itself can do error checking better, we just care
    // if this helper crashes or not.
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
