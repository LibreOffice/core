/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_OPENCL_OPENCL_DEVICE_HXX
#define INCLUDED_SC_SOURCE_CORE_OPENCL_OPENCL_DEVICE_HXX

#pragma once
#include "opencl_device_selection.h"

namespace sc { namespace OpenCLDevice {

ds_device getDeviceSelection(const char* pFileName, bool bForceSelection = false);
bool selectedDeviceIsOpenCL(ds_device device);
bool selectedDeviceIsNativeCPU(ds_device device);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
