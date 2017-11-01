/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OPENCL_INC_OPENCL_DEVICE_SELECTION_H
#define INCLUDED_OPENCL_INC_OPENCL_DEVICE_SELECTION_H

#ifdef _MSC_VER
//#define _CRT_SECURE_NO_WARNINGS
#endif

#include <memory>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <clew/clew.h>
#include <tools/stream.hxx>
#include <tools/XmlWriter.hxx>
#include <tools/XmlWalker.hxx>
#include <rtl/math.hxx>

#include <opencl/OpenCLZone.hxx>

#include <vector>

enum ds_status
{
    DS_SUCCESS = 0
    ,DS_INVALID_PROFILE = 1000
    ,DS_MEMORY_ERROR
    , DS_INVALID_PERF_EVALUATOR_TYPE
    , DS_INVALID_PERF_EVALUATOR
    , DS_PERF_EVALUATOR_ERROR
    , DS_FILE_ERROR
    , DS_UNKNOWN_DEVICE_TYPE
    , DS_PROFILE_FILE_ERROR
    , DS_SCORE_SERIALIZER_ERROR
    , DS_SCORE_DESERIALIZER_ERROR
};

// device type
enum class DeviceType
{
    None,
    // NativeCPU means the traditional Calc interpreter code path. (That also includes the so-called
    // "software interpreter", but note that it definitely does not mean *exclusively* that.)
    NativeCPU,
    // OpenCLDevice means an OpenCL device as supplied by an OpenCL platform, which might well be
    // implemented using code that runs on the CPU (and not a GPU). On Windows, OpenCL platforms
    // typically provide two devices, one for the GPU and one for the CPU.
    OpenCLDevice
};

struct ds_device
{
    DeviceType eType;
    cl_device_id aDeviceID;

    OString sPlatformName;
    OString sPlatformVendor;
    OString sPlatformVersion;
    OString sPlatformProfile;
    OString sPlatformExtensions;

    OString sDeviceName;
    OString sDeviceVendor;
    OString sDeviceVersion;
    OString sDriverVersion;
    OString sDeviceType;
    OString sDeviceExtensions;
    OString sDeviceOpenCLVersion;

    bool bDeviceAvailable;
    bool bDeviceCompilerAvailable;
    bool bDeviceLinkerAvailable;

    double fTime;   // small time means faster device
    bool   bErrors; // were there any opencl errors
};

struct ds_profile
{
    std::vector<ds_device> devices;
    OString version;

    ds_profile(OString const & inVersion)
        : version(inVersion)
    {}
};

inline OString getPlatformInfoString(cl_platform_id aPlatformId, cl_platform_info aPlatformInfo)
{
    std::vector<char> temporary(2048, 0);
    clGetPlatformInfo(aPlatformId, aPlatformInfo, temporary.size(), temporary.data(), nullptr);
    return OString(temporary.data());
}

inline OString getDeviceInfoString(cl_device_id aDeviceId, cl_device_info aDeviceInfo)
{
    std::vector<char> temporary(2048, 0);
    clGetDeviceInfo(aDeviceId, aDeviceInfo, temporary.size(), temporary.data(), nullptr);
    return OString(temporary.data());
}

inline OString getDeviceType(cl_device_id aDeviceId)
{
    OString sType = "";
    cl_device_type aDeviceType;
    clGetDeviceInfo(aDeviceId, CL_DEVICE_TYPE, sizeof(aDeviceType), &aDeviceType, nullptr);
    if (aDeviceType & CL_DEVICE_TYPE_CPU)
        sType += "cpu ";
    if (aDeviceType & CL_DEVICE_TYPE_GPU)
        sType += "gpu ";
    if (aDeviceType & CL_DEVICE_TYPE_ACCELERATOR)
        sType += "accelerator ";
    if (aDeviceType & CL_DEVICE_TYPE_CUSTOM)
        sType += "custom ";
    if (aDeviceType & CL_DEVICE_TYPE_DEFAULT)
        sType += "default ";
    return sType;
}

inline bool getDeviceInfoBool(cl_device_id aDeviceId, cl_device_info aDeviceInfo)
{
    cl_bool bCLBool = 0;
        // init to false in case clGetDeviceInfo returns CL_INVALID_VALUE when
        // requesting unsupported (in version 1.0) CL_DEVICE_LINKER_AVAILABLE
    clGetDeviceInfo(aDeviceId, aDeviceInfo, sizeof(bCLBool), &bCLBool, nullptr);
    return bCLBool == CL_TRUE;
}

inline ds_status initDSProfile(std::unique_ptr<ds_profile>& rProfile, OString const & rVersion)
{
    OpenCLZone zone;

    int numDevices;
    cl_uint numPlatforms;
    std::vector<cl_platform_id> platforms;
    std::vector<cl_device_id> devices;

    unsigned int next;
    unsigned int i;

    rProfile.reset(new ds_profile(rVersion));

    clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (numPlatforms != 0)
    {
        platforms.resize(numPlatforms);
        clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
    }

    numDevices = 0;
    for (i = 0; i < (unsigned int)numPlatforms; i++)
    {
        cl_uint num = 0;
        cl_int err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, nullptr, &num);
        if (err != CL_SUCCESS)
        {
            /* we want to catch at least the case when the call returns
             * CL_DEVICE_NOT_FOUND (i.e. no devices), because some platforms
             * don't set num to 0 in this case; but in fact this is a good
             * thing to do for _any_ error returned by the call
             */
            num = 0;
        }
        numDevices += num;
    }

    if (numDevices != 0)
    {
        devices.resize(numDevices);
    }

    rProfile->devices.resize(numDevices + 1); // +1 to numDevices to include the native CPU

    next = 0;
    for (i = 0; i < (unsigned int)numPlatforms; i++)
    {
        cl_uint num = 0;
        unsigned j;

        OString sPlatformProfile = getPlatformInfoString(platforms[i], CL_PLATFORM_PROFILE);
        OString sPlatformVersion = getPlatformInfoString(platforms[i], CL_PLATFORM_VERSION);
        OString sPlatformName    = getPlatformInfoString(platforms[i], CL_PLATFORM_NAME);
        OString sPlatformVendor  = getPlatformInfoString(platforms[i], CL_PLATFORM_VENDOR);
        OString sPlatformExts    = getPlatformInfoString(platforms[i], CL_PLATFORM_EXTENSIONS);

        cl_int err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, numDevices, devices.data(), &num);
        if (err != CL_SUCCESS)
        {
            /* we want to catch at least the case when the call returns
             * CL_DEVICE_NOT_FOUND (i.e. no devices), because some platforms
             * don't set num to 0 in this case; but in fact this is a good
             * thing to do for _any_ error returned by the call
             */
            num = 0;
        }
        for (j = 0; j < num; j++, next++)
        {
            cl_device_id aDeviceID = devices[j];

            ds_device& rDevice = rProfile->devices[next];
            rDevice.eType = DeviceType::OpenCLDevice;
            rDevice.aDeviceID = aDeviceID;

            rDevice.sPlatformName         = sPlatformName;
            rDevice.sPlatformVendor       = sPlatformVendor;
            rDevice.sPlatformVersion      = sPlatformVersion;
            rDevice.sPlatformProfile      = sPlatformProfile;
            rDevice.sPlatformExtensions   = sPlatformExts;

            rDevice.sDeviceName       = getDeviceInfoString(aDeviceID, CL_DEVICE_NAME);
            rDevice.sDeviceVendor     = getDeviceInfoString(aDeviceID, CL_DEVICE_VENDOR);
            rDevice.sDeviceVersion    = getDeviceInfoString(aDeviceID, CL_DEVICE_VERSION);
            rDevice.sDriverVersion    = getDeviceInfoString(aDeviceID, CL_DRIVER_VERSION);
            rDevice.sDeviceType       = getDeviceType(aDeviceID);
            rDevice.sDeviceExtensions = getDeviceInfoString(aDeviceID, CL_DEVICE_EXTENSIONS);
            rDevice.sDeviceOpenCLVersion = getDeviceInfoString(aDeviceID, CL_DEVICE_OPENCL_C_VERSION);

            rDevice.bDeviceAvailable         = getDeviceInfoBool(aDeviceID, CL_DEVICE_AVAILABLE);
            rDevice.bDeviceCompilerAvailable = getDeviceInfoBool(aDeviceID, CL_DEVICE_COMPILER_AVAILABLE);
            rDevice.bDeviceLinkerAvailable   = getDeviceInfoBool(aDeviceID, CL_DEVICE_LINKER_AVAILABLE);
        }
    }
    rProfile->devices[next].eType = DeviceType::NativeCPU;

    return DS_SUCCESS;
}

inline ds_status writeProfile(const OUString& rStreamName, std::unique_ptr<ds_profile> const & pProfile)
{
    if (pProfile == nullptr)
        return DS_INVALID_PROFILE;
    if (rStreamName.isEmpty())
        return DS_INVALID_PROFILE;

    std::unique_ptr<SvStream> pStream;
    pStream.reset(new SvFileStream(rStreamName, StreamMode::STD_READWRITE | StreamMode::TRUNC));

    tools::XmlWriter aXmlWriter(pStream.get());

    if (!aXmlWriter.startDocument())
        return DS_FILE_ERROR;

    aXmlWriter.startElement("profile");

    aXmlWriter.startElement("version");
    aXmlWriter.content(pProfile->version);
    aXmlWriter.endElement();

    for (ds_device& rDevice : pProfile->devices)
    {
        aXmlWriter.startElement("device");

        switch(rDevice.eType)
        {
            case DeviceType::NativeCPU:
                aXmlWriter.startElement("type");
                aXmlWriter.content(OString("native"));
                aXmlWriter.endElement();
                break;
            case DeviceType::OpenCLDevice:
                aXmlWriter.startElement("type");
                aXmlWriter.content(OString("opencl"));
                aXmlWriter.endElement();

                aXmlWriter.startElement("name");
                aXmlWriter.content(rDevice.sDeviceName);
                aXmlWriter.endElement();

                aXmlWriter.startElement("driver");
                aXmlWriter.content(rDevice.sDriverVersion);
                aXmlWriter.endElement();
                break;
            default:
                break;
        }

        aXmlWriter.startElement("time");
        if (rtl::math::approxEqual(rDevice.fTime, DBL_MAX))
            aXmlWriter.content(OString("max"));
        else
            aXmlWriter.content(OString::number(rDevice.fTime));
        aXmlWriter.endElement();

        aXmlWriter.startElement("errors");
        aXmlWriter.content(rDevice.bErrors ? OString("true") : OString("false"));
        aXmlWriter.endElement();

        aXmlWriter.endElement();
    }

    aXmlWriter.endElement();
    aXmlWriter.endDocument();

    return DS_SUCCESS;
}

inline ds_status readProfile(const OUString& rStreamName, std::unique_ptr<ds_profile> const & pProfile)
{
    ds_status eStatus = DS_SUCCESS;

    if (rStreamName.isEmpty())
        return DS_INVALID_PROFILE;

    std::unique_ptr<SvStream> pStream;
    pStream.reset(new SvFileStream(rStreamName, StreamMode::READ));
    tools::XmlWalker aWalker;

    if (!aWalker.open(pStream.get()))
        return DS_FILE_ERROR;

    if (aWalker.name() == "profile")
    {
        aWalker.children();
        while (aWalker.isValid())
        {
            if (aWalker.name() == "version")
            {
                if (aWalker.content() != pProfile->version)
                    return DS_PROFILE_FILE_ERROR;
            }
            else if (aWalker.name() == "device")
            {
                aWalker.children();

                DeviceType eDeviceType = DeviceType::None;
                OString sName;
                OString sVersion;
                double fTime = -1.0;
                bool bErrors = true;

                while (aWalker.isValid())
                {
                    if (aWalker.name() == "type")
                    {
                        OString sContent = aWalker.content();
                        if (sContent == "native")
                            eDeviceType = DeviceType::NativeCPU;
                        else if (sContent == "opencl")
                            eDeviceType = DeviceType::OpenCLDevice;
                        else
                            return DS_PROFILE_FILE_ERROR;
                    }
                    else if (aWalker.name() == "name")
                    {
                        sName = aWalker.content();
                    }
                    else if (aWalker.name() == "driver")
                    {
                        sVersion = aWalker.content();
                    }
                    else if (aWalker.name() == "time")
                    {
                        if (aWalker.content() == "max")
                            fTime = DBL_MAX;
                        else
                            fTime = aWalker.content().toDouble();
                    }
                    else if (aWalker.name() == "errors")
                    {
                        bErrors = (aWalker.content() == "true");
                    }

                    aWalker.next();
                }

                if (fTime < 0.0)
                    return DS_PROFILE_FILE_ERROR;

                for (ds_device& rDevice : pProfile->devices)
                {
                    // type matches? either both are DS_DEVICE_OPENCL_DEVICE or DS_DEVICE_NATIVE_CPU
                    if (rDevice.eType == eDeviceType)
                    {
                        // is DS_DEVICE_NATIVE_CPU or name + version matches?
                        if (eDeviceType == DeviceType::NativeCPU ||
                                (sName == rDevice.sDeviceName &&
                                 sVersion == rDevice.sDriverVersion))
                        {
                            rDevice.fTime = fTime;
                            rDevice.bErrors = bErrors;
                        }
                    }
                }

                aWalker.parent();
            }
            aWalker.next();
        }
        aWalker.parent();
    }

    return eStatus;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
