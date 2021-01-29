/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SAL_OSL_W32_FILE_URL_HXX
#define INCLUDED_SAL_OSL_W32_FILE_URL_HXX

#include <sal/types.h>
#include <rtl/ustring.h>
#include <osl/file.h>
#include <osl/mutex.hxx>

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#define PATHTYPE_ERROR                      0
#define PATHTYPE_RELATIVE                   1
#define PATHTYPE_ABSOLUTE_UNC               2
#define PATHTYPE_ABSOLUTE_LOCAL             3
#define PATHTYPE_MASK_TYPE                  0xFF
#define PATHTYPE_IS_VOLUME                  0x0100
#define PATHTYPE_IS_SERVER                  0x0200
#define PATHTYPE_IS_LONGPATH                0x0400

#define VALIDATEPATH_NORMAL                 0x0000
#define VALIDATEPATH_ALLOW_ELLIPSE          0x0002
#define VALIDATEPATH_ALLOW_RELATIVE         0x0004
#define VALIDATEPATH_ALLOW_INVALID_SPACE_AND_PERIOD 0x0010

#define MAX_LONG_PATH 32767

DWORD IsValidFilePath (
    rtl_uString *  path,
    DWORD          dwFlags,
    rtl_uString ** corrected
);

DWORD GetCaseCorrectPathName (
    LPCWSTR lpszShortPath,  // file name
    LPWSTR  lpszLongPath,   // path buffer
    sal_uInt32 cchBuffer,   // size of path buffer
    bool bCheckExistence
);

oslFileError osl_getSystemPathFromFileURL_ (
    rtl_uString * strURL,
    rtl_uString ** pustrPath,
    bool bAllowRelative
);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
