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

#include <systools/win32/uwinapi.h>

#include <osl/file.h>
#include <o3tl/char16_t2wchar_t.hxx>
#include <rtl/ustring.hxx>

#include "file-impl.hxx"
#include "file_error.hxx"
#include "file_url.hxx"
#include "path_helper.hxx"

#include <malloc.h>
#include <cassert>

// Allocate n number of t's on the stack return a pointer to it in p
#define STACK_ALLOC(p, t, n) __try {(p) = static_cast<t*>(_alloca((n)*sizeof(t)));} \
                             __except(EXCEPTION_EXECUTE_HANDLER) {(p) = nullptr;}

// Temp file functions

static oslFileError osl_setup_base_directory_impl_(
    rtl_uString*  pustrDirectoryURL,
    rtl_uString** ppustr_base_dir)
{
    OUString dir_url;
    OUString dir;
    oslFileError error   = osl_File_E_None;

    if (pustrDirectoryURL)
        dir_url = pustrDirectoryURL;
    else
        error = osl_getTempDirURL(&dir_url.pData);

    if (error == osl_File_E_None)
        error = osl_getSystemPathFromFileURL_(dir_url, &dir.pData, false);

    if (error == osl_File_E_None)
        rtl_uString_assign(ppustr_base_dir, dir.pData);

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

    OSL_PRECOND(((pHandle != nullptr) || (ppustrTempFileURL != nullptr)), "Invalid parameter!");

    if ((pHandle == nullptr) && (ppustrTempFileURL == nullptr))
    {
        osl_error = osl_File_E_INVAL;
    }
    else
    {
        osl_error = osl_setup_base_directory_impl_(
            pustrDirectoryURL, ppustr_base_dir);

        *b_delete_on_close = (ppustrTempFileURL == nullptr);
    }

    return osl_error;
}

static LPCWSTR getEyeCatcher()
{
    static const OUString sEyeCatcher = []
    {
        OUString eyeCatcher = u"\0"_ustr;
#ifdef DBG_UTIL
        if (const wchar_t* eye = _wgetenv(L"LO_TESTNAME"))
            eyeCatcher = OUString(o3tl::toU(eye), wcslen(eye) + 1); // including terminating nul
#endif
        return eyeCatcher;
    }();
    return o3tl::toW(sEyeCatcher.getStr());
}

static oslFileError osl_win32_GetTempFileName_impl_(
    rtl_uString* base_directory, LPWSTR temp_file_name)
{
    oslFileError osl_error = osl_File_E_None;

    if (GetTempFileNameW(
            o3tl::toW(rtl_uString_getStr(base_directory)),
            getEyeCatcher(),
            0,
            temp_file_name) == 0)
    {
        osl_error = oslTranslateFileError(GetLastError());
    }

    return osl_error;
}

static bool osl_win32_CreateFile_impl_(
    LPCWSTR file_name, bool b_delete_on_close, oslFileHandle* p_handle)
{
    DWORD  flags = FILE_ATTRIBUTE_NORMAL;
    HANDLE hFile;

    assert(p_handle);

    if (b_delete_on_close)
        flags |= FILE_FLAG_DELETE_ON_CLOSE;

    hFile = CreateFileW(
        file_name,
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        TRUNCATE_EXISTING,
        flags,
        nullptr);

    // @@@ ERROR HANDLING @@@
    if (IsValidHandle(hFile))
        *p_handle = osl_createFileHandleFromOSHandle(hFile, osl_File_OpenFlag_Read | osl_File_OpenFlag_Write);

    return IsValidHandle(hFile);
}

static oslFileError osl_createTempFile_impl_(
    rtl_uString*   base_directory,
    LPWSTR         tmp_name,
    bool           b_delete_on_close,
    oslFileHandle* pHandle,
    rtl_uString**  ppustrTempFileURL)
{
    oslFileError osl_error;

    do
    {
        osl_error = osl_win32_GetTempFileName_impl_(base_directory, tmp_name);

        /*  if file could not be opened try again */

        if ((osl_File_E_None != osl_error) || (nullptr == pHandle) ||
            osl_win32_CreateFile_impl_(tmp_name, b_delete_on_close, pHandle))
            break;

    } while(true); // try until success

    if ((osl_error == osl_File_E_None) && !b_delete_on_close)
    {
        rtl_uString* pustr = nullptr;
        rtl_uString_newFromStr(&pustr, o3tl::toU(tmp_name));
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
    rtl_uString*    base_directory = nullptr;
    LPWSTR          tmp_name;
    sal_Bool        b_delete_on_close;
    oslFileError    osl_error;

    osl_error = osl_setup_createTempFile_impl_(
        pustrDirectoryURL,
        pHandle,
        ppustrTempFileURL,
        &base_directory,
        &b_delete_on_close);

    if (osl_error != osl_File_E_None)
        return osl_error;

    /* allocate enough space on the stack, the file name can not be longer than MAX_PATH */
    STACK_ALLOC(tmp_name, WCHAR, (rtl_uString_getLength(base_directory) + MAX_PATH));

    if (tmp_name)
    {
        osl_error = osl_createTempFile_impl_(
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
    LPWSTR  lpBuffer = o3tl::toW(aBuffer);
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
        if ( '\\' == lpBuffer[nLength-1] )
            --nLength;

        const OUString ustrTempPath(o3tl::toU(lpBuffer), static_cast<sal_Int32>(nLength));

        error = osl_getFileURLFromSystemPath(ustrTempPath.pData, pustrTempDir);
    }
    else
        error = oslTranslateFileError( GetLastError() );

    return error;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
