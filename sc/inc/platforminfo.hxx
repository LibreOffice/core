/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_PLATFORMINFO_HXX
#define INCLUDED_SC_INC_PLATFORMINFO_HXX

#include <vector>

#include <rtl/ustring.hxx>

#include "scdllapi.h"

namespace sc {

struct SC_DLLPUBLIC OpenclDeviceInfo
{
    void* device;
    OUString maName;
    OUString maVendor;
    OUString maDriver;
    size_t mnMemory;
    size_t mnComputeUnits;
    size_t mnFrequency;

    OpenclDeviceInfo();
};

struct SC_DLLPUBLIC OpenclPlatformInfo
{
    void* platform;
    OUString maVendor;
    OUString maName;
    std::vector<OpenclDeviceInfo> maDevices;

    OpenclPlatformInfo();
};

}

#endif
