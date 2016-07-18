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

#include "opencl_device_selection.h"

namespace opencl {

ds_device const & getDeviceSelection(OUString const & pFileName, bool bForceSelection = false);

struct GPUEnv;
void releaseOpenCLEnv( GPUEnv *gpuInfo );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
