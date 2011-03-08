/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_OSL_FILE_URL_H
#define INCLUDED_OSL_FILE_URL_H

#include "sal/types.h"
#include "rtl/ustring.h"
#include "osl/file.h"

#ifdef _MSC_VER
#pragma warning(push,1)
#endif

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PATHTYPE_ERROR                      0
#define PATHTYPE_RELATIVE                   1
#define PATHTYPE_ABSOLUTE_UNC               2
#define PATHTYPE_ABSOLUTE_LOCAL             3
#define PATHTYPE_MASK_TYPE                  0xFF
#define PATHTYPE_IS_VOLUME                  0x0100
#define PATHTYPE_IS_SERVER                  0x0200
#define PATHTYPE_IS_LONGPATH                0x0400

#define VALIDATEPATH_NORMAL                 0x0000
#define VALIDATEPATH_ALLOW_WILDCARDS        0x0001
#define VALIDATEPATH_ALLOW_ELLIPSE          0x0002
#define VALIDATEPATH_ALLOW_RELATIVE         0x0004
#define VALIDATEPATH_ALLOW_UNC              0x0008
#define VALIDATEPATH_ALLOW_INVALID_SPACE_AND_PERIOD 0x0010

#define MAX_LONG_PATH 32767

DWORD IsValidFilePath (
    rtl_uString *  path,
    LPCTSTR *      lppError,
    DWORD          dwFlags,
    rtl_uString ** corrected
);

DWORD GetCaseCorrectPathName (
    LPCTSTR lpszShortPath,  // file name
    LPTSTR  lpszLongPath,   // path buffer
    DWORD   cchBuffer,      // size of path buffer
    BOOL bCheckExistence
);

oslFileError _osl_getSystemPathFromFileURL (
    rtl_uString *  strURL,
    rtl_uString ** pustrPath,
    sal_Bool       bAllowRelative
);

oslFileError _osl_getFileURLFromSystemPath (
    rtl_uString *  strPath,
    rtl_uString ** pstrURL
);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_OSL_FILE_URL_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
