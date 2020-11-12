/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_OSL_W32_FILE_IMPL_HXX
#define INCLUDED_SAL_OSL_W32_FILE_IMPL_HXX

#include <sal/config.h>

#include <osl/file.h>
#include <sal/types.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern "C" oslFileHandle osl_createFileHandleFromOSHandle(HANDLE hFile, sal_uInt32 uFlags);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
