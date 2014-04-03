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

#define UNICODE
#define _UNICODE
#include "systools/win32/uwinapi.h"

#include "osl/file.h"

#include "file_error.h"
#include "file_url.h"
#include "path_helper.hxx"

#include "osl/diagnose.h"

#include <malloc.h>
#include <tchar.h>

// Allocate n number of t's on the stack return a pointer to it in p
#ifdef __MINGW32__
#define STACK_ALLOC(p, t, n) (p) = reinterpret_cast<t*>(_alloca((n)*sizeof(t)));
#else
#define STACK_ALLOC(p, t, n) __try {(p) = reinterpret_cast<t*>(_alloca((n)*sizeof(t)));} \
                             __except(EXCEPTION_EXECUTE_HANDLER) {(p) = 0;}
#endif

extern "C" oslFileHandle SAL_CALL osl_createFileHandleFromOSHandle(HANDLE hFile, sal_uInt32 uFlags);

// Temp file functions

static oslFileError osl_setup_base_directory_impl_(
    rtl_uString*  pustrDirectoryURL,
    rtl_uString** ppustr_base_dir)
{
    rtl_uString* dir_url = 0;
    rtl_uString* dir     = 0;
    oslFileError error   = osl_File_E_None;

    if (pustrDirectoryURL)
        rtl_uString_assign(&dir_url, pustrDirectoryURL);
    else
        error = osl_getTempDirURL(&dir_url);

    if (osl_File_E_None == error)
    {
        error = _osl_getSystemPathFromFileURL(dir_url, &dir, sal_False);
        rtl_uString_release(dir_url);
    }

    if (osl_File_E_None == error )
    {
        rtl_uString_assign(ppustr_base_dir, dir);
        rtl_uString_release(dir);
    }

    return error;
}

static oslFileError osl_setup_createTempFile_impl_(
    rtl_uString*   pustrDirectoryURL,
    oslFileHandle* pHandle,
    rtl_uString**  ppustrTempFileURL,
    rtl_uString**  ppustr_base_dir,
    sal_Bool*      b_delete_on_close)
{
    oslFileError osl_error;

    OSL_PRECOND(((0 != pHandle) || (0 != ppustrTempFileURL)), "Invalid parameter!");

    if ((0 == pHandle) && (0 == ppustrTempFileURL))
    {
        osl_error = osl_File_E_INVAL;
    }
    else
    {
        osl_error = osl_setup_base_directory_impl_(
            pustrDirectoryURL, ppustr_base_dir);

        *b_delete_on_close = (sal_Bool)(0 == ppustrTempFileURL);
    }

    return osl_error;
}

static oslFileError osl_win32_GetTempFileName_impl_(
    rtl_uString* base_directory, LPWSTR temp_file_name)
{
    oslFileError osl_error = osl_File_E_None;

    if (0 == GetTempFileNameW(
            reinterpret_cast<LPCWSTR>(rtl_uString_getStr(base_directory)),
            L"",
            0,
            temp_file_name))
    {
        osl_error = oslTranslateFileError(GetLastError());
    }

    return osl_error;
}

static sal_Bool osl_win32_CreateFile_impl_(
    LPCWSTR file_name, sal_Bool b_delete_on_close, oslFileHandle* p_handle)
{
    DWORD  flags = FILE_ATTRIBUTE_NORMAL;
    HANDLE hFile;

    OSL_ASSERT(p_handle);

    if (b_delete_on_close)
        flags |= FILE_FLAG_DELETE_ON_CLOSE;

    hFile = CreateFileW(
        file_name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        TRUNCATE_EXISTING,
        flags,
        NULL);

    // @@@ ERROR HANDLING @@@
    if (IsValidHandle(hFile))
        *p_handle = osl_createFileHandleFromOSHandle(hFile, osl_File_OpenFlag_Read | osl_File_OpenFlag_Write);

    return (sal_Bool)IsValidHandle(hFile);
}

static oslFileError osl_createTempFile_impl_(
    rtl_uString*   base_directory,
    LPWSTR         tmp_name,
    sal_Bool       b_delete_on_close,
    oslFileHandle* pHandle,
    rtl_uString**  ppustrTempFileURL)
{
    oslFileError osl_error;

    do
    {
        osl_error = osl_win32_GetTempFileName_impl_(base_directory, tmp_name);

        /*  if file could not be opened try again */

        if ((osl_File_E_None != osl_error) || (0 == pHandle) ||
            osl_win32_CreateFile_impl_(tmp_name, b_delete_on_close, pHandle))
            break;

    } while(1); // try until success

    if ((osl_File_E_None == osl_error) && !b_delete_on_close)
    {
        rtl_uString* pustr = 0;
        rtl_uString_newFromStr(&pustr, reinterpret_cast<const sal_Unicode*>(tmp_name));
        osl_getFileURLFromSystemPath(pustr, ppustrTempFileURL);
        rtl_uString_release(pustr);
    }

    return osl_error;
}

oslFileError SAL_CALL osl_createTempFile(
    rtl_uString*   pustrDirectoryURL,
    oslFileHandle* pHandle,
    rtl_uString**  ppustrTempFileURL)
{
    rtl_uString*    base_directory = 0;
    LPWSTR          tmp_name;
    sal_Bool        b_delete_on_close;
    oslFileError    osl_error;

    osl_error = osl_setup_createTempFile_impl_(
        pustrDirectoryURL,
        pHandle,
        ppustrTempFileURL,
        &base_directory,
        &b_delete_on_close);

    if (osl_File_E_None != osl_error)
        return osl_error;

    /* allocate enough space on the stack, the file name can not be longer than MAX_PATH */
    STACK_ALLOC(tmp_name, WCHAR, (rtl_uString_getLength(base_directory) + MAX_PATH));

    if (tmp_name)
    {
        osl_createTempFile_impl_(
            base_directory,
            tmp_name,
            b_delete_on_close,
            pHandle,
            ppustrTempFileURL);
    }
    else // stack alloc failed
    {
        osl_error = osl_File_E_NOMEM;
    }

    if (base_directory)
        rtl_uString_release(base_directory);

    return osl_error;
}

oslFileError SAL_CALL osl_getTempDirURL(rtl_uString** pustrTempDir)
{
    ::osl::LongPathBuffer< sal_Unicode > aBuffer( MAX_LONG_PATH );
    LPWSTR  lpBuffer = ::osl::mingw_reinterpret_cast<LPWSTR>(aBuffer);
    DWORD   nBufferLength = aBuffer.getBufSizeInSymbols() - 1;

    DWORD           nLength;
    oslFileError    error;

    nLength = GetTempPathW( aBuffer.getBufSizeInSymbols(), lpBuffer );

    if ( nLength > nBufferLength )
    {
        // the provided path has invalid length
        error = osl_File_E_NOENT;
    }
    else if ( nLength )
    {
        rtl_uString *ustrTempPath = NULL;

        if ( '\\' == lpBuffer[nLength-1] )
            lpBuffer[nLength-1] = 0;

        rtl_uString_newFromStr( &ustrTempPath, reinterpret_cast<const sal_Unicode*>(lpBuffer) );

        error = osl_getFileURLFromSystemPath( ustrTempPath, pustrTempDir );

        rtl_uString_release( ustrTempPath );
    }
    else
        error = oslTranslateFileError( GetLastError() );

    return error;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
