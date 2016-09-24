/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ostream>

#include <opencl/platforminfo.hxx>

OpenCLDeviceInfo::OpenCLDeviceInfo()
    : device(nullptr)
    , mnMemory(0)
    , mnComputeUnits(0)
    , mnFrequency(0)
{
}

OpenCLPlatformInfo::OpenCLPlatformInfo()
    : platform(nullptr)
{
}

std::ostream& operator<<(std::ostream& rStream, const OpenCLPlatformInfo& rPlatform)
{
    rStream << "{"
        "Vendor=" << rPlatform.maVendor << ","
        "Name=" << rPlatform.maName <<
        "}";
    return rStream;
}

std::ostream& operator<<(std::ostream& rStream, const OpenCLDeviceInfo& rDevice)
{
    rStream << "{"
        "Name=" << rDevice.maName << ","
        "Vendor=" << rDevice.maVendor << ","
        "Driver=" << rDevice.maDriver <<
        "}";
    return rStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
