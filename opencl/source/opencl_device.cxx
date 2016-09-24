/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef _WIN32
#include <prewin.h>
#include <postwin.h>
#elif defined __MACH__
#include <mach/mach_time.h>
#else
#include <sys/time.h>
#endif

#include <time.h>
#include <math.h>
#include <float.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <vector>

#include <comphelper/random.hxx>
#include <opencl/openclconfig.hxx>
#include <opencl/openclwrapper.hxx>
#include <opencl/platforminfo.hxx>
#include <sal/log.hxx>
#include <rtl/math.hxx>

#include <opencl/OpenCLZone.hxx>

#include "opencl_device.hxx"

#define INPUTSIZE  15360
#define OUTPUTSIZE 15360

#define STRINGIFY(...) #__VA_ARGS__"\n"

#define DS_CHECK_STATUS(status, name) \
    if (CL_SUCCESS != status) \
    { \
    SAL_INFO("opencl.device", "Error code is " << status << " at " name); \
    }

namespace opencl {

namespace {

bool bIsDeviceSelected = false;
ds_device selectedDevice;

struct LibreOfficeDeviceEvaluationIO
{
    std::vector<double> input0;
    std::vector<double> input1;
    std::vector<double> input2;
    std::vector<double> input3;
    std::vector<double> output;
    unsigned long inputSize;
    unsigned long outputSize;
};

struct timer
{
#ifdef _WIN32
    LARGE_INTEGER start;
#else
    long long start;
#endif
};

const char* source = STRINGIFY(
\n#if defined(KHR_DP_EXTENSION)
\n#pragma OPENCL EXTENSION cl_khr_fp64 : enable
\n#elif defined(AMD_DP_EXTENSION)
\n#pragma OPENCL EXTENSION cl_amd_fp64 : enable
\n#endif
    \n
    int isNan(fp_t a) { return a != a; }
    fp_t fsum(fp_t a, fp_t b) { return a + b; }

    fp_t fAverage(__global fp_t* input)
{
    fp_t sum = 0;
    int count = 0;
    for (int i = 0; i < INPUTSIZE; i++)
    {
        if (!isNan(input[i]))
        {
            sum = fsum(input[i], sum);
            count += 1;
        }
    }
    return sum / (fp_t)count;
}
    fp_t fMin(__global fp_t* input)
{
    fp_t min = MAXFLOAT;
    for (int i = 0; i < INPUTSIZE; i++)
    {
        if (!isNan(input[i]))
        {
            min = fmin(input[i], min);
        }
    }
    return min;
}
    fp_t fSoP(__global fp_t* input0, __global fp_t* input1)
{
    fp_t sop = 0.0;
    for (int i = 0; i < INPUTSIZE; i++)
    {
        sop += (isNan(input0[i]) ? 0 : input0[i]) * (isNan(input1[i]) ? 0 : input1[i]);
    }
    return sop;
}
    __kernel void DynamicKernel(
        __global fp_t* result, __global fp_t* input0, __global fp_t* input1, __global fp_t* input2, __global fp_t* input3)
{
    int gid0 = get_global_id(0);
    fp_t tmp0 = fAverage(input0);
    fp_t tmp1 = fMin(input1) * fSoP(input2, input3);
    result[gid0] = fsum(tmp0, tmp1);
}
    );

size_t sourceSize[] = { strlen(source) };

/*************************************************************************/
/* INTERNAL FUNCTIONS                                                    */
/*************************************************************************/
/* Timer functions - start timer */
void timerStart(timer* mytimer)
{
#ifdef _WIN32
    QueryPerformanceCounter(&mytimer->start);
#elif defined __MACH__
    mytimer->start = mach_absolute_time();
#else
    struct timespec s;
    clock_gettime(CLOCK_MONOTONIC, &s);
    mytimer->start = (long long)s.tv_sec * (long long)1.0E6 + (long long)s.tv_nsec / (long long)1.0E3;
#endif
}

/* Timer functions - get current value */
double timerCurrent(timer* mytimer)
{
#ifdef _WIN32
    LARGE_INTEGER stop, frequency;
    QueryPerformanceCounter(&stop);
    QueryPerformanceFrequency(&frequency);
    double time = ((double)(stop.QuadPart - mytimer->start.QuadPart) / frequency.QuadPart);
#elif defined __MACH__
    static mach_timebase_info_data_t info = { 0, 0 };
    if (info.numer == 0)
        mach_timebase_info(&info);
    long long stop = mach_absolute_time();
    double time = ((stop - mytimer->start) * (double) info.numer / info.denom) / 1.0E9;
#else
    struct timespec s;
    long long stop;
    clock_gettime(CLOCK_MONOTONIC, &s);
    stop = (long long)s.tv_sec * (long long)1.0E6 + (long long)s.tv_nsec / (long long)1.0E3;
    double time = ((double)(stop - mytimer->start) / 1.0E6);
#endif
    return time;
}

/* Random number generator */
double random(double min, double max)
{
    if (rtl::math::approxEqual(min, max))
        return min;
    return comphelper::rng::uniform_real_distribution(min, max);
}

/* Populate input */
void populateInput(std::unique_ptr<LibreOfficeDeviceEvaluationIO>& testData)
{
    double* input0 = &testData->input0[0];
    double* input1 = &testData->input1[0];
    double* input2 = &testData->input2[0];
    double* input3 = &testData->input3[0];
    for (unsigned long i = 0; i < testData->inputSize; i++)
    {
        input0[i] = random(0, i);
        input1[i] = random(0, i);
        input2[i] = random(0, i);
        input3[i] = random(0, i);
    }
}

/* Evaluate devices */
ds_status evaluateScoreForDevice(ds_device& rDevice, std::unique_ptr<LibreOfficeDeviceEvaluationIO>& testData)
{
    if (rDevice.eType == DeviceType::OpenCLDevice)
    {
        /* Evaluating an OpenCL device */
        SAL_INFO("opencl.device", "Device: \"" << rDevice.sDeviceName << "\" (OpenCL) evaluation...");
        cl_int clStatus;

        /* Check for 64-bit float extensions */
        std::unique_ptr<char[]> aExtInfo;
        {
            size_t aDevExtInfoSize = 0;

            OpenCLZone zone;
            clStatus = clGetDeviceInfo(rDevice.aDeviceID, CL_DEVICE_EXTENSIONS, 0, nullptr, &aDevExtInfoSize);
            DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clGetDeviceInfo");

            aExtInfo.reset(new char[aDevExtInfoSize]);
            clStatus = clGetDeviceInfo(rDevice.aDeviceID, CL_DEVICE_EXTENSIONS, sizeof(char) * aDevExtInfoSize, aExtInfo.get(), nullptr);
            DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clGetDeviceInfo");
        }

        bool bKhrFp64Flag = false;
        bool bAmdFp64Flag = false;
        const char* buildOption = nullptr;
        std::string tmpStr("-Dfp_t=double -Dfp_t4=double4 -Dfp_t16=double16 -DINPUTSIZE=");
        std::ostringstream tmpOStrStr;
        tmpOStrStr << std::dec << INPUTSIZE;
        tmpStr.append(tmpOStrStr.str());

        if ((std::string(aExtInfo.get())).find("cl_khr_fp64") != std::string::npos)
        {
            bKhrFp64Flag = true;
            //buildOption = "-D KHR_DP_EXTENSION -Dfp_t=double -Dfp_t4=double4 -Dfp_t16=double16";
            tmpStr.append(" -DKHR_DP_EXTENSION");
            buildOption = tmpStr.c_str();
            SAL_INFO("opencl.device", "... has cl_khr_fp64");
        }
        else if ((std::string(aExtInfo.get())).find("cl_amd_fp64") != std::string::npos)
        {
            bAmdFp64Flag = true;
            //buildOption = "-D AMD_DP_EXTENSION -Dfp_t=double -Dfp_t4=double4 -Dfp_t16=double16";
            tmpStr.append(" -DAMD_DP_EXTENSION");
            buildOption = tmpStr.c_str();
            SAL_INFO("opencl.device", "... has cl_amd_fp64");
        }

        if (!bKhrFp64Flag && !bAmdFp64Flag)
        {
            /* No 64-bit float support */
            rDevice.fTime = DBL_MAX;
            rDevice.bErrors = false;
            SAL_INFO("opencl.device", "... no fp64 support");
        }
        else
        {
            /* 64-bit float support present */

            OpenCLZone zone;

            /* Create context and command queue */
            cl_context  clContext = clCreateContext(nullptr, 1, &rDevice.aDeviceID, nullptr, nullptr, &clStatus);
            DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clCreateContext");
            cl_command_queue clQueue = clCreateCommandQueue(clContext, rDevice.aDeviceID, 0, &clStatus);
            DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clCreateCommandQueue");

            /* Build program */
            cl_program clProgram = clCreateProgramWithSource(clContext, 1, &source, sourceSize, &clStatus);
            DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clCreateProgramWithSource");
            clStatus = clBuildProgram(clProgram, 1, &rDevice.aDeviceID, buildOption, nullptr, nullptr);
            DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clBuildProgram");
            if (CL_SUCCESS != clStatus)
            {
                /* Build program failed */
                size_t length;
                char* buildLog;
                clStatus = clGetProgramBuildInfo(clProgram, rDevice.aDeviceID, CL_PROGRAM_BUILD_LOG, 0, nullptr, &length);
                buildLog = static_cast<char*>(malloc(length));
                clGetProgramBuildInfo(clProgram, rDevice.aDeviceID, CL_PROGRAM_BUILD_LOG, length, buildLog, &length);
                SAL_INFO("opencl.device", "Build Errors:\n" << buildLog);
                free(buildLog);

                rDevice.fTime = DBL_MAX;
                rDevice.bErrors = true;
            }
            else
            {
                /* Build program succeeded */
                timer kernelTime;
                timerStart(&kernelTime);

                /* Run kernel */
                cl_kernel clKernel = clCreateKernel(clProgram, "DynamicKernel", &clStatus);
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clCreateKernel");
                cl_mem clResult = clCreateBuffer(clContext, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, sizeof(cl_double) * testData->outputSize, &testData->output[0], &clStatus);
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clCreateBuffer::clResult");
                cl_mem clInput0 = clCreateBuffer(clContext, CL_MEM_READ_ONLY  | CL_MEM_USE_HOST_PTR, sizeof(cl_double) * testData->inputSize,  &testData->input0[0], &clStatus);
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clCreateBuffer::clInput0");
                cl_mem clInput1 = clCreateBuffer(clContext, CL_MEM_READ_ONLY  | CL_MEM_USE_HOST_PTR, sizeof(cl_double) * testData->inputSize,  &testData->input1[0], &clStatus);
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clCreateBuffer::clInput1");
                cl_mem clInput2 = clCreateBuffer(clContext, CL_MEM_READ_ONLY  | CL_MEM_USE_HOST_PTR, sizeof(cl_double) * testData->inputSize,  &testData->input2[0], &clStatus);
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clCreateBuffer::clInput2");
                cl_mem clInput3 = clCreateBuffer(clContext, CL_MEM_READ_ONLY  | CL_MEM_USE_HOST_PTR, sizeof(cl_double) * testData->inputSize,  &testData->input3[0], &clStatus);
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clCreateBuffer::clInput3");
                clStatus = clSetKernelArg(clKernel, 0, sizeof(cl_mem), static_cast<void*>(&clResult));
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clSetKernelArg::clResult");
                clStatus = clSetKernelArg(clKernel, 1, sizeof(cl_mem), static_cast<void*>(&clInput0));
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clSetKernelArg::clInput0");
                clStatus = clSetKernelArg(clKernel, 2, sizeof(cl_mem), static_cast<void*>(&clInput1));
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clSetKernelArg::clInput1");
                clStatus = clSetKernelArg(clKernel, 3, sizeof(cl_mem), static_cast<void*>(&clInput2));
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clSetKernelArg::clInput2");
                clStatus = clSetKernelArg(clKernel, 4, sizeof(cl_mem), static_cast<void*>(&clInput3));
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clSetKernelArg::clInput3");
                size_t globalWS[1] = { testData->outputSize };
                size_t localSize[1] = { 64 };
                clStatus = clEnqueueNDRangeKernel(clQueue, clKernel, 1, nullptr, globalWS, localSize, 0, nullptr, nullptr);
                DS_CHECK_STATUS(clStatus, "evaluateScoreForDevice::clEnqueueNDRangeKernel");
                clFinish(clQueue);
                clReleaseMemObject(clInput3);
                clReleaseMemObject(clInput2);
                clReleaseMemObject(clInput1);
                clReleaseMemObject(clInput0);
                clReleaseMemObject(clResult);
                clReleaseKernel(clKernel);

                rDevice.fTime = timerCurrent(&kernelTime);
                rDevice.bErrors = false;
            }

            clReleaseProgram(clProgram);
            clReleaseCommandQueue(clQueue);
            clReleaseContext(clContext);
        }
    }
    else
    {
        /* Evaluating an Native CPU device */
        SAL_INFO("opencl.device", "Device: \"CPU\" (Native) evaluation...");
        timer kernelTime;
        timerStart(&kernelTime);

        unsigned long j;
        for (j = 0; j < testData->outputSize; j++)
        {
            double fAverage = 0.0f;
            double fMin = DBL_MAX;
            double fSoP = 0.0f;
            for (unsigned long i = 0; i < testData->inputSize; i++)
            {
                fAverage += testData->input0[i];
                fMin = ((fMin < testData->input1[i]) ? fMin : testData->input1[i]);
                fSoP += testData->input2[i] * testData->input3[i];
            }
            fAverage /= testData->inputSize;
            testData->output[j] = fAverage + (fMin * fSoP);
            // Don't run for much longer than one second
            if (j > 0 && j % 100 == 0)
            {
                rDevice.fTime = timerCurrent(&kernelTime);
                if (rDevice.fTime >= 1)
                    break;
            }
        }

        rDevice.fTime = timerCurrent(&kernelTime);

        // Scale time to how long it would have taken to go all the way to outputSize
        rDevice.fTime /= ((double) j / testData->outputSize);

        // InterpretTail - the S/W fallback is nothing like as efficient
        // as any good openCL implementation: no SIMD, tons of branching
        // in the inner loops etc. Generously characterise it as only 10x
        // slower than the above.
        float fInterpretTailFactor = 10.0;
        rDevice.fTime *= fInterpretTailFactor;
        rDevice.bErrors = false;
    }
    return DS_SUCCESS;
}

ds_status profileDevices(std::unique_ptr<ds_profile>& pProfile, std::unique_ptr<LibreOfficeDeviceEvaluationIO>& pTestData)
{
    ds_status status = DS_SUCCESS;

    if (!pProfile)
        return DS_INVALID_PROFILE;

    for (ds_device& rDevice : pProfile->devices)
    {
        ds_status evaluatorStatus = evaluateScoreForDevice(rDevice, pTestData);
        if (evaluatorStatus != DS_SUCCESS)
        {
            status = evaluatorStatus;
            return status;
        }
    }
    return status;
}

/* Pick best device */
ds_status pickBestDevice(std::unique_ptr<ds_profile>& profile, int& rBestDeviceIndex)
{
    double bestScore = DBL_MAX;

    rBestDeviceIndex = -1;

    for (std::vector<ds_device>::size_type d = 0; d < profile->devices.size();
         d++)
    {
        ds_device& device = profile->devices[d];

        // Check blacklist and whitelist for actual devices
        if (device.eType == DeviceType::OpenCLDevice)
        {
            // There is a silly impedance mismatch here. Why do we
            // need two different ways to describe an OpenCL platform
            // and an OpenCL device driver?

            OpenCLPlatformInfo aPlatform;
            OpenCLDeviceInfo aDevice;

            // We know that only the below fields are used by checkForKnownBadCompilers()
            aPlatform.maVendor = OStringToOUString(device.sPlatformVendor, RTL_TEXTENCODING_UTF8);
            aDevice.maName = OStringToOUString(device.sDeviceName, RTL_TEXTENCODING_UTF8);
            aDevice.maDriver = OStringToOUString(device.sDriverVersion, RTL_TEXTENCODING_UTF8);

            // If blacklisted or not whitelisted, ignore it
            if (OpenCLConfig::get().checkImplementation(aPlatform, aDevice))
            {
                SAL_INFO("opencl.device", "Device[" << d << "] " << device.sDeviceName << " is blacklisted or not whitelisted");
                device.fTime = DBL_MAX;
                device.bErrors = false;
            }
        }

        double fScore = DBL_MAX;
        if (device.fTime >= 0.0
            || rtl::math::approxEqual(device.fTime, DBL_MAX))
        {
            fScore = device.fTime;
        }
        else
        {
            SAL_INFO("opencl.device", "Unusual null score");
        }

        if (device.eType == DeviceType::OpenCLDevice)
        {
            SAL_INFO("opencl.device", "Device[" << d << "] " << device.sDeviceName << " (OpenCL) score is " << fScore);
        }
        else
        {
            SAL_INFO("opencl.device", "Device[" << d << "] CPU (Native) score is " << fScore);
        }
        if (fScore < bestScore)
        {
            bestScore = fScore;
            rBestDeviceIndex = d;
        }
    }
    if (rBestDeviceIndex != -1 && profile->devices[rBestDeviceIndex].eType == DeviceType::OpenCLDevice)
    {
        SAL_INFO("opencl.device", "Selected Device[" << rBestDeviceIndex << "]: " << profile->devices[rBestDeviceIndex].sDeviceName << "(OpenCL).");
    }
    else
    {
        SAL_INFO("opencl.device", "Selected Device[" << rBestDeviceIndex << "]: CPU (Native).");
    }
    return DS_SUCCESS;
}

/* Return device ID for matching device name */
int matchDevice(std::unique_ptr<ds_profile>& profile, char* deviceName)
{
    int deviceMatch = -1;
    for (unsigned int d = 0; d < profile->devices.size() - 1; d++)
    {
        if ((std::string(profile->devices[d].sDeviceName.getStr())).find(deviceName) != std::string::npos)
            deviceMatch = d;
    }
    if (std::string("NATIVE_CPU").find(deviceName) != std::string::npos)
        deviceMatch = profile->devices.size() - 1;
    return deviceMatch;
}

class LogWriter
{
private:
    SvFileStream maStream;
public:
    explicit LogWriter(OUString const & aFileName)
        : maStream(aFileName, StreamMode::WRITE)
    {}

    void text(const OString& rText)
    {
        maStream.WriteOString(rText);
        maStream.WriteChar('\n');
    }

    void log(const OString& rKey, const OString& rValue)
    {
        maStream.WriteOString(rKey);
        maStream.WriteCharPtr(": ");
        maStream.WriteOString(rValue);
        maStream.WriteChar('\n');
    }

    void log(const OString& rKey, int rValue)
    {
        log(rKey, OString::number(rValue));
    }

    void log(const OString& rKey, bool rValue)
    {
        log(rKey, OString::boolean(rValue));
    }
};


void writeDevicesLog(std::unique_ptr<ds_profile>& rProfile, OUString const & sProfilePath, int nSelectedIndex)
{
    OUString aCacheFile(sProfilePath + "opencl_devices.log");
    LogWriter aWriter(aCacheFile);

    int nIndex = 0;

    for (ds_device& rDevice : rProfile->devices)
    {
        if (rDevice.eType == DeviceType::OpenCLDevice)
        {
            aWriter.log("Device Index", nIndex);
            aWriter.log("  Selected", nIndex == nSelectedIndex);
            aWriter.log("  Device Name", rDevice.sDeviceName);
            aWriter.log("  Device Vendor", rDevice.sDeviceVendor);
            aWriter.log("  Device Version", rDevice.sDeviceVersion);
            aWriter.log("  Driver Version", rDevice.sDriverVersion);
            aWriter.log("  Device Type", rDevice.sDeviceType);
            aWriter.log("  Device Extensions", rDevice.sDeviceExtensions);
            aWriter.log("  Device OpenCL C Version", rDevice.sDeviceOpenCLVersion);

            aWriter.log("  Device Available", rDevice.bDeviceAvailable);
            aWriter.log("  Device Compiler Available", rDevice.bDeviceCompilerAvailable);
            aWriter.log("  Device Linker Available", rDevice.bDeviceLinkerAvailable);

            aWriter.log("  Platform Name", rDevice.sPlatformName);
            aWriter.log("  Platform Vendor", rDevice.sPlatformVendor);
            aWriter.log("  Platform Version", rDevice.sPlatformVersion);
            aWriter.log("  Platform Profile", rDevice.sPlatformProfile);
            aWriter.log("  Platform Extensions", rDevice.sPlatformExtensions);
            aWriter.text("");
        }
        nIndex++;
    }
}

} // end anonymous namespace

ds_device const & getDeviceSelection(
    OUString const & sProfilePath, bool bForceSelection)
{
    /* Run only if device is not yet selected */
    if (!bIsDeviceSelected || bForceSelection)
    {
        /* Setup */
        std::unique_ptr<ds_profile> aProfile;
        ds_status status;
        status = initDSProfile(aProfile, "LibreOffice v1");

        if (status != DS_SUCCESS)
        {
            // failed to initialize profile.
            selectedDevice.eType = DeviceType::NativeCPU;
            return selectedDevice;
        }

        /* Try reading scores from file */
        OUString sFilePath = sProfilePath + "opencl_profile.xml";

        if (!bForceSelection)
        {
            status = readProfile(sFilePath, aProfile);
        }
        else
        {
            status = DS_INVALID_PROFILE;
            SAL_INFO("opencl.device", "Performing forced profiling.");
        }
        if (DS_SUCCESS != status)
        {
            if (!bForceSelection)
            {
                SAL_INFO("opencl.device", "Profile file not available (" << sFilePath << "); performing profiling.");
            }

            /* Populate input data for micro-benchmark */
            std::unique_ptr<LibreOfficeDeviceEvaluationIO> testData(new LibreOfficeDeviceEvaluationIO);
            testData->inputSize  = INPUTSIZE;
            testData->outputSize = OUTPUTSIZE;
            testData->input0.resize(testData->inputSize);
            testData->input1.resize(testData->inputSize);
            testData->input2.resize(testData->inputSize);
            testData->input3.resize(testData->inputSize);
            testData->output.resize(testData->outputSize);
            populateInput(testData);

            /* Perform evaluations */
            status = profileDevices(aProfile, testData);

            if (DS_SUCCESS == status)
            {
                /* Write scores to file */
                status = writeProfile(sFilePath, aProfile);
                if (DS_SUCCESS == status)
                {
                    SAL_INFO("opencl.device", "Scores written to file (" << sFilePath << ").");
                }
                else
                {
                    SAL_INFO("opencl.device", "Error saving scores to file (" << sFilePath << "); scores not written to file.");
                }
            }
            else
            {
                SAL_INFO("opencl.device", "Unable to evaluate performance; scores not written to file.");
            }
        }
        else
        {
            SAL_INFO("opencl.device", "Profile read from file (" << sFilePath << ").");
        }

        /* Pick best device */
        int bestDeviceIdx;
        pickBestDevice(aProfile, bestDeviceIdx);

        /* Override if necessary */
        char* overrideDeviceStr = getenv("SC_OPENCL_DEVICE_OVERRIDE");
        if (nullptr != overrideDeviceStr)
        {
            int overrideDeviceIdx = matchDevice(aProfile, overrideDeviceStr);
            if (-1 != overrideDeviceIdx)
            {
                SAL_INFO("opencl.device", "Overriding Device Selection (SC_OPENCL_DEVICE_OVERRIDE=" << overrideDeviceStr << ").");
                bestDeviceIdx = overrideDeviceIdx;
                if (aProfile->devices[bestDeviceIdx].eType == DeviceType::OpenCLDevice)
                {
                    SAL_INFO("opencl.device", "Selected Device[" << bestDeviceIdx << "]: " << aProfile->devices[bestDeviceIdx].sDeviceName << " (OpenCL).");
                }
                else
                {
                    SAL_INFO("opencl.device", "Selected Device[" << bestDeviceIdx << "]: CPU (Native).");
                }
            }
            else
            {
                SAL_INFO("opencl.device", "Ignoring invalid SC_OPENCL_DEVICE_OVERRIDE=" << overrideDeviceStr << ").");
            }
        }

        /* Final device selection */
        if (bestDeviceIdx >=0 && static_cast< std::vector<ds_device>::size_type> ( bestDeviceIdx ) < aProfile->devices.size() )
        {
            selectedDevice = aProfile->devices[bestDeviceIdx];
            bIsDeviceSelected = true;

            writeDevicesLog(aProfile, sProfilePath, bestDeviceIdx);
        } else {
            selectedDevice.eType = DeviceType::NativeCPU;
        }
    }
    return selectedDevice;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
