/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OPENCL_PLATFORMINFO_HXX
#define INCLUDED_OPENCL_PLATFORMINFO_HXX

#include <ostream>
#include <vector>

#include <clew/clew.h>

#include <opencl/opencldllapi.h>
#include <rtl/ustring.hxx>

// Struct that describs an actual instance of an OpenCL device

struct OPENCL_DLLPUBLIC OpenCLDeviceInfo
{
    cl_device_id device;
    OUString maName;
    OUString maVendor;
    OUString maDriver;
    size_t mnMemory;
    size_t mnComputeUnits;
    size_t mnFrequency;

    OpenCLDeviceInfo();
};

// Struct that describs an actual instance of an OpenCL platform implementation

struct OPENCL_DLLPUBLIC OpenCLPlatformInfo
{
    cl_platform_id platform;
    OUString maVendor;
    OUString maName;
    std::vector<OpenCLDeviceInfo> maDevices;

    OpenCLPlatformInfo();
};

OPENCL_DLLPUBLIC std::ostream& operator<<(std::ostream& rStream, const OpenCLPlatformInfo& rPlatform);
OPENCL_DLLPUBLIC std::ostream& operator<<(std::ostream& rStream, const OpenCLDeviceInfo& rDevice);

#endif
