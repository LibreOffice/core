/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OPENCL_INC_CLEW_SETUP_HXX
#define INCLUDED_OPENCL_INC_CLEW_SETUP_HXX

#ifdef _WIN32
#include <prewin.h>
#include <postwin.h>
#define OPENCL_DLL_NAME "OpenCL.dll"
#elif defined(MACOSX)
#define OPENCL_DLL_NAME nullptr
#else
#define OPENCL_DLL_NAME "libOpenCL.so.1"
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
