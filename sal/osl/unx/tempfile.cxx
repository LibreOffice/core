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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "system.hxx"
#include <osl/file.h>
#include <osl/thread.h>
#include <rtl/ustrbuf.h>
#include <osl/diagnose.h>
#include <sal/macros.h>

#include "file_url.hxx"
#include "file_impl.hxx"

#include <cassert>

oslFileError SAL_CALL osl_getTempDirURL( rtl_uString** pustrTempDir )
{
    oslFileError error;
    /* described in environ(7) */
    const char *pValue = getenv( "TMPDIR" );
    rtl_uString *ustrTempPath = nullptr;

    if ( !pValue )
        pValue = getenv( "TEMP" );

    if ( !pValue )
        pValue = getenv( "TMP" );

    if ( !pValue )
        pValue = "/tmp";

    rtl_string2UString( &ustrTempPath, pValue, strlen( pValue ), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
    assert(ustrTempPath);
    error = osl_getFileURLFromSystemPath( ustrTempPath, pustrTempDir );
    rtl_uString_release( ustrTempPath );

    return error;
}

/******************************************************************
 * Generates a random unique file name. We're using the scheme
 * from the standard c-lib function mkstemp to generate a more
 * or less random unique file name
 *
 * @param rand_name
 *        receives the random name
 ******************************************************************/

static const char LETTERS[]        = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
static const int  COUNT_OF_LETTERS = SAL_N_ELEMENTS(LETTERS) - 1;

#define RAND_NAME_LENGTH 6

static void osl_gen_random_name_impl_(rtl_uString** rand_name)
{
    static uint64_t value;

    char     buffer[RAND_NAME_LENGTH];
    struct   timeval tv;
    uint64_t v;
    int      i;

    gettimeofday(&tv, nullptr);

    value += (static_cast<uint64_t>(tv.tv_usec) << 16) ^ tv.tv_sec ^ getpid();

    v = value;

    for (i = 0; i < RAND_NAME_LENGTH; i++)
    {
        buffer[i] = LETTERS[v % COUNT_OF_LETTERS];
        v        /= COUNT_OF_LETTERS;
    }

    rtl_string2UString(
            rand_name,
            buffer,
            RAND_NAME_LENGTH,
            RTL_TEXTENCODING_ASCII_US,
            OSTRING_TO_OUSTRING_CVTFLAGS);

    assert(*rand_name);
}

/*****************************************************************
 * Helper function
 * Either use the directory provided or the result of
 * osl_getTempDirUrl and return it as system path and file url
 ****************************************************************/

static oslFileError osl_setup_base_directory_impl_(
    rtl_uString*  pustrDirectoryURL,
    rtl_uString** ppustr_base_dir)
{
    rtl_uString* dir_url = nullptr;
    rtl_uString* dir     = nullptr;
    oslFileError error   = osl_File_E_None;

    if (pustrDirectoryURL)
        rtl_uString_assign(&dir_url, pustrDirectoryURL);
    else
        error = osl_getTempDirURL(&dir_url);

    if (error == osl_File_E_None)
    {
        error = osl_getSystemPathFromFileURL_Ex(dir_url, &dir);
        rtl_uString_release(dir_url);
    }

    if (error == osl_File_E_None)
    {
        rtl_uString_assign(ppustr_base_dir, dir);
        rtl_uString_release(dir);
    }

    return error;
}

/*****************************************************************
 * osl_setup_createTempFile_impl
 * validate input parameter, setup variables
 ****************************************************************/

 static oslFileError osl_setup_createTempFile_impl_(
     rtl_uString*   pustrDirectoryURL,
    oslFileHandle* pHandle,
    rtl_uString**  ppustrTempFileURL,
    rtl_uString**  ppustr_base_dir,
    bool*      b_delete_on_close)
 {
    oslFileError osl_error;

    OSL_PRECOND(((nullptr != pHandle) || (nullptr != ppustrTempFileURL)), "Invalid parameter!");

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

/*****************************************************************
 * Create a unique file in the specified directory and return
 * its name
 ****************************************************************/

static oslFileError osl_create_temp_file_impl_(
    const rtl_uString* pustr_base_directory,
    oslFileHandle* file_handle,
    rtl_uString** ppustr_temp_file_name)
{
    rtl_uString*        rand_name        = nullptr;
    sal_uInt32          len_base_dir     = 0;
    rtl_uString*        tmp_file_path    = nullptr;
    rtl_uString*        tmp_file_url     = nullptr;
    sal_Int32           capacity         = 0;
    oslFileError        osl_error        = osl_File_E_None;
    sal_Int32           offset_file_name;
    const sal_Unicode*  puchr;

    OSL_PRECOND(pustr_base_directory, "Invalid Parameter");
    OSL_PRECOND(file_handle, "Invalid Parameter");
    OSL_PRECOND(ppustr_temp_file_name, "Invalid Parameter");

    len_base_dir = rtl_uString_getLength(pustr_base_directory);

    rtl_uStringbuffer_newFromStr_WithLength(
        &tmp_file_path,
        rtl_uString_getStr(const_cast<rtl_uString*>(pustr_base_directory)),
        len_base_dir);

    rtl_uStringbuffer_ensureCapacity(
        &tmp_file_path,
        &capacity,
        (len_base_dir + 1 + RAND_NAME_LENGTH));

    offset_file_name = len_base_dir;

    puchr = rtl_uString_getStr(tmp_file_path);

    /* ensure that the last character is a '/' */

    if (puchr[len_base_dir - 1] != '/')
    {
        rtl_uStringbuffer_insert_ascii(
            &tmp_file_path,
            &capacity,
            len_base_dir,
            "/",
            1);

        offset_file_name++;
    }

    while(true) /* try until success */
    {
        osl_gen_random_name_impl_(&rand_name);

        rtl_uStringbuffer_insert(
            &tmp_file_path,
            &capacity,
            offset_file_name,
            rtl_uString_getStr(rand_name),
            rtl_uString_getLength(rand_name));

        osl_error = osl_getFileURLFromSystemPath(
            tmp_file_path, &tmp_file_url);

        if (osl_error == osl_File_E_None)
        {
            osl_error = openFile(
                tmp_file_url,
                file_handle,
                osl_File_OpenFlag_Read |
                osl_File_OpenFlag_Write |
                osl_File_OpenFlag_Create,
                S_IRUSR | S_IWUSR);
        }

        /* in case of error osl_File_E_EXIST we simply try again else we give up */

        if (osl_error != osl_File_E_EXIST)
        {
            rtl_uString_release(rand_name);

            if (tmp_file_url)
                rtl_uString_release(tmp_file_url);

            break;
        }
    } /* while(1) */

    if (osl_error == osl_File_E_None)
        rtl_uString_assign(ppustr_temp_file_name, tmp_file_path);

    rtl_uString_release(tmp_file_path);

    return osl_error;
}

oslFileError SAL_CALL osl_createTempFile(
    rtl_uString*   pustrDirectoryURL,
    oslFileHandle* pHandle,
    rtl_uString**  ppustrTempFileURL)
{
    rtl_uString*  base_directory     = nullptr;
    oslFileHandle temp_file_handle;
    bool      b_delete_on_close;
    oslFileError  osl_error;

    osl_error = osl_setup_createTempFile_impl_(
        pustrDirectoryURL,
        pHandle,
        ppustrTempFileURL,
        &base_directory,
        &b_delete_on_close);

    if (osl_error != osl_File_E_None)
        return osl_error;

    rtl_uString*  temp_file_name = nullptr;
    osl_error = osl_create_temp_file_impl_(
        base_directory, &temp_file_handle, &temp_file_name);

    rtl_uString* temp_file_url = nullptr;
    if (osl_error == osl_File_E_None)
    {
        osl_error = osl_getFileURLFromSystemPath(temp_file_name, &temp_file_url);
        rtl_uString_release(temp_file_name);
    }

    if (osl_error == osl_File_E_None)
    {
        if (b_delete_on_close)
        {
            osl_error = osl_removeFile(temp_file_url);

            if (osl_error == osl_File_E_None)
            {
                *pHandle = temp_file_handle;
                temp_file_handle = nullptr;
            }
        }
        else
        {
            if (pHandle)
            {
                *pHandle = temp_file_handle;
                temp_file_handle = nullptr;
            }

            rtl_uString_assign(ppustrTempFileURL, temp_file_url);
        }

        rtl_uString_release(temp_file_url);
    }

    if (temp_file_handle)
        osl_closeFile(temp_file_handle);

    rtl_uString_release(base_directory);

    return osl_error;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
