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

/*****************************************************************/
/* Includes                                                      */
/*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "system.h"
#include <osl/file.h>
#include <osl/thread.h>
#include <rtl/ustrbuf.h>
#include <osl/diagnose.h>
#include <sal/macros.h>

#ifndef _FILE_URL_H_
#include "file_url.h"
#endif

/*****************************************************************/
/* osl_getTempFirURL                                             */
/*****************************************************************/

oslFileError SAL_CALL osl_getTempDirURL( rtl_uString** pustrTempDir )
{
    const char *pValue = getenv( "TEMP" );

    if ( !pValue )
    {
        pValue = getenv( "TMP" );
#if defined(SOLARIS) || defined (LINUX) || defined (FREEBSD) || defined (MACOSX)
        if ( !pValue )
            pValue = P_tmpdir;
#endif
    }

    if ( pValue )
    {
        oslFileError error;
        rtl_uString *ustrTempPath = NULL;

        rtl_string2UString( &ustrTempPath, pValue, strlen( pValue ), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
        OSL_ASSERT(ustrTempPath != NULL);
        error = osl_getFileURLFromSystemPath( ustrTempPath, pustrTempDir );
        rtl_uString_release( ustrTempPath );

        return error;
    }
    else
        return osl_File_E_NOENT;
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

    gettimeofday(&tv, NULL);

    value += ((uint64_t)tv.tv_usec << 16) ^ tv.tv_sec ^ getpid();

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
    OSL_ASSERT(*rand_name != NULL);
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
    rtl_uString* dir_url = 0;
    rtl_uString* dir     = 0;
    oslFileError error   = osl_File_E_None;

    if (pustrDirectoryURL)
        rtl_uString_assign(&dir_url, pustrDirectoryURL);
    else
        error = osl_getTempDirURL(&dir_url);

    if (osl_File_E_None == error)
    {
        error = osl_getSystemPathFromFileURL_Ex(dir_url, &dir, FURL_DENY_RELATIVE);
        rtl_uString_release(dir_url);
    }

    if (osl_File_E_None == error)
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

        *b_delete_on_close = (0 == ppustrTempFileURL);
    }

    return osl_error;
 }

/*****************************************************************
 * Create a unique file in the specified directory and return
 * it's name
 ****************************************************************/

static oslFileError osl_create_temp_file_impl_(
    const rtl_uString* pustr_base_directory,
    oslFileHandle* file_handle,
    rtl_uString** ppustr_temp_file_name)
{
    rtl_uString*        rand_name        = 0;
    sal_uInt32          len_base_dir     = 0;
    rtl_uString*        tmp_file_path    = 0;
    rtl_uString*        tmp_file_url     = 0;
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
        rtl_uString_getStr((rtl_uString*)pustr_base_directory),
        len_base_dir);

    rtl_uStringbuffer_ensureCapacity(
        &tmp_file_path,
        &capacity,
        (len_base_dir + 1 + RAND_NAME_LENGTH));

    offset_file_name = len_base_dir;

    puchr = rtl_uString_getStr(tmp_file_path);

    /* ensure that the last character is a '\' */

    if ((sal_Unicode)'\\' != puchr[len_base_dir - 1])
    {
        rtl_uStringbuffer_insert_ascii(
            &tmp_file_path,
            &capacity,
            len_base_dir,
            "\\",
            1);

        offset_file_name++;
    }

    while(1) /* try until success */
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

        if (osl_File_E_None == osl_error)
        {
            /* RW permission for the user only! */
            mode_t old_mode = umask(077);

            osl_error = osl_openFile(
                tmp_file_url,
                file_handle,
                osl_File_OpenFlag_Read |
                osl_File_OpenFlag_Write |
                osl_File_OpenFlag_Create);

            umask(old_mode);
        }

        /* in case of error osl_File_E_EXIST we simply try again else we give up */

        if ((osl_File_E_None == osl_error) || (osl_error != osl_File_E_EXIST))
        {
            if (rand_name)
                rtl_uString_release(rand_name);

            if (tmp_file_url)
                rtl_uString_release(tmp_file_url);

            break;
        }
    } /* while(1) */

    if (osl_File_E_None == osl_error)
        rtl_uString_assign(ppustr_temp_file_name, tmp_file_path);

    if (tmp_file_path)
        rtl_uString_release(tmp_file_path);

    return osl_error;
}

/*****************************************************************
 * osl_createTempFile
 *****************************************************************/

oslFileError SAL_CALL osl_createTempFile(
    rtl_uString*   pustrDirectoryURL,
    oslFileHandle* pHandle,
    rtl_uString**  ppustrTempFileURL)
{
    rtl_uString*  base_directory     = 0;
    rtl_uString*  temp_file_name     = 0;
    oslFileHandle temp_file_handle;
    sal_Bool      b_delete_on_close;
    oslFileError  osl_error;

    osl_error = osl_setup_createTempFile_impl_(
        pustrDirectoryURL,
        pHandle,
        ppustrTempFileURL,
        &base_directory,
        &b_delete_on_close);

    if (osl_File_E_None != osl_error)
        return osl_error;

    osl_error = osl_create_temp_file_impl_(
        base_directory, &temp_file_handle, &temp_file_name);

    if (osl_File_E_None == osl_error)
    {
        rtl_uString* temp_file_url = 0;

        /* assuming this works */
        osl_getFileURLFromSystemPath(temp_file_name, &temp_file_url);

        if (b_delete_on_close)
        {
            osl_error = osl_removeFile(temp_file_url);

            if (osl_File_E_None == osl_error)
                *pHandle = temp_file_handle;
            else
                osl_closeFile(temp_file_handle);
        }
        else
        {
            if (pHandle)
                *pHandle = temp_file_handle;
            else
                osl_closeFile(temp_file_handle);

            rtl_uString_assign(ppustrTempFileURL, temp_file_url);
        }

        if (temp_file_url)
            rtl_uString_release(temp_file_url);

        if (temp_file_name)
            rtl_uString_release(temp_file_name);
    }

    if (base_directory)
        rtl_uString_release(base_directory);

    return osl_error;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
