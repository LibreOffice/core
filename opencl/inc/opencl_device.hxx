/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OPENCL_INC_OPENCL_DEVICE_HXX
#define INCLUDED_OPENCL_INC_OPENCL_DEVICE_HXX

#include <sal/config.h>

#include <string_view>

#include <rtl/ustring.hxx>

namespace openclwrapper
{
struct GPUEnv;
}
struct ds_device;

ds_device const& getDeviceSelection(std::u16string_view pFileName, bool bForceSelection);

void releaseOpenCLEnv(openclwrapper::GPUEnv* gpuInfo);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
