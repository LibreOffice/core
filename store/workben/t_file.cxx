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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_store.hxx"

#include "sal/types.h"
#include "osl/thread.h"
#include "rtl/ustring.hxx"

#include "lockbyte.hxx"

#ifndef INCLUDED_STDIO_H
#include <stdio.h>
#define INCLUDED_STDIO_H
#endif

#include "osl/file.h"
#include "osl/process.h"

using namespace store;

#define TEST_PAGESIZE 16384

/*========================================================================
 *
 * main.
 *
 *======================================================================*/
int SAL_CALL main (int argc, char **argv)
{
    storeError eErrCode = store_E_None;
    rtl::Reference<ILockBytes> xLockBytes;

    if (argc > 1)
    {
        rtl::OUString aFilename (
            argv[1], rtl_str_getLength(argv[1]),
            osl_getThreadTextEncoding());

#if 0   /* EXP */
        oslFileError result;
        rtl::OUString aPath;

        result = osl_getFileURLFromSystemPath(aFilename.pData, &(aPath.pData));
        if (result != osl_File_E_None)
        {
            // not SystemPath, assume FileUrl.
            aPath = aFilename;
        }
        if (rtl_ustr_ascii_shortenedCompare_WithLength(aPath.pData->buffer, aPath.pData->length, "file://", 7) != 0)
        {
            // not FileUrl, assume relative path.
            rtl::OUString aBase;
            (void) osl_getProcessWorkingDir (&(aBase.pData));

            // absolute FileUrl.
            (void) osl_getAbsoluteFileURL(aBase.pData, aPath.pData, &(aPath.pData));
        }
        aFilename = aPath;
#endif  /* EXP */

        eErrCode = FileLockBytes_createInstance (
            xLockBytes, aFilename.pData, store_AccessReadWrite);
        if (eErrCode != store_E_None)
        {
            // Check reason.
            if (eErrCode != store_E_NotExists)
            {
                fprintf (stderr, "t_file: create() error: %d\n", eErrCode);
                return eErrCode;
            }

            // Create.
            eErrCode = FileLockBytes_createInstance (
                xLockBytes, aFilename.pData, store_AccessReadCreate);
            if (eErrCode != store_E_None)
            {
                fprintf (stderr, "t_file: create() error: %d\n", eErrCode);
                return eErrCode;
            }
        }
        fprintf (stdout, "t_file: using FileLockBytes(\"%s\") implementation.\n", argv[1]);
    }
    else
    {
        eErrCode = MemoryLockBytes_createInstance (xLockBytes);
        if (eErrCode != store_E_None)
        {
            fprintf (stderr, "t_file: create() error: %d\n", eErrCode);
            return eErrCode;
        }
        fprintf (stdout, "t_file: using MemoryLockBytes implementation.\n");
    }

    rtl::Reference< PageData::Allocator > xAllocator;
    eErrCode = xLockBytes->initialize (xAllocator, TEST_PAGESIZE);
    if (eErrCode != store_E_None)
    {
        fprintf (stderr, "t_file: initialize() error: %d\n", eErrCode);
        return eErrCode;
    }

    sal_Char buffer[TEST_PAGESIZE];
    rtl_fillMemory (buffer, sizeof(buffer), sal_uInt8('B'));

    sal_uInt32 i, k;
    for (k = 0; k < 4; k++)
    {
        sal_uInt32 index = k * TEST_PAGESIZE / 4;
        buffer[index] = 'A';
    }

    for (i = 0; i < 256; i++)
    {
        sal_uInt32 offset = i * TEST_PAGESIZE;
        eErrCode = xLockBytes->setSize (offset + TEST_PAGESIZE);
        if (eErrCode != store_E_None)
        {
            fprintf (stderr, "t_file: setSize() error: %d\n", eErrCode);
            return eErrCode;
        }

        for (k = 0; k < 4; k++)
        {
            sal_uInt32 magic = i * 4 + k;
            if (magic)
            {
                sal_uInt32 verify = 0;
                eErrCode = xLockBytes->readAt (
                    0, &verify, sizeof(verify));
                if (eErrCode != store_E_None)
                {
                    fprintf (stderr, "t_file: readAt() error: %d\n", eErrCode);
                    return eErrCode;
                }
                if (verify != magic)
                {
                    // Failure.
                    fprintf (stderr, "Expected %ld read %ld\n", (unsigned long)(magic), (unsigned long)(verify));
                }
            }

            sal_uInt32 index = k * TEST_PAGESIZE / 4;
            eErrCode = xLockBytes->writeAt (
                offset + index, &(buffer[index]), TEST_PAGESIZE / 4);
            if (eErrCode != store_E_None)
            {
                fprintf (stderr, "t_file: writeAt() error: %d\n", eErrCode);
                return eErrCode;
            }

            magic += 1;
            eErrCode = xLockBytes->writeAt (
                0, &magic, sizeof(magic));
            if (eErrCode != store_E_None)
            {
                fprintf (stderr, "t_file: writeAt() error: %d\n", eErrCode);
                return eErrCode;
            }
        }
    }

    eErrCode = xLockBytes->flush();
    if (eErrCode != store_E_None)
    {
        fprintf (stderr, "t_file: flush() error: %d\n", eErrCode);
        return eErrCode;
    }

    sal_Char verify[TEST_PAGESIZE];
    for (i = 0; i < 256; i++)
    {
        sal_uInt32 offset = i * TEST_PAGESIZE;

        eErrCode = xLockBytes->readAt (offset, verify, TEST_PAGESIZE);
        if (eErrCode != store_E_None)
        {
            fprintf (stderr, "t_file: readAt() error: %d\n", eErrCode);
            return eErrCode;
        }

        sal_uInt32 index = 0;
        if (offset == 0)
        {
            sal_uInt32 magic = 256 * 4;
            if (rtl_compareMemory (&verify[index], &magic, sizeof(magic)))
            {
                // Failure.
                fprintf (stderr, "t_file: Unexpected value at 0x00000000\n");
            }
            index += 4;
        }
        if (rtl_compareMemory (
            &verify[index], &buffer[index], TEST_PAGESIZE - index))
        {
            // Failure.
            fprintf (stderr, "t_file: Unexpected block at 0x%08x\n", (unsigned)(offset));
        }
    }

    for (i = 0; i < 256; i++)
    {
        PageHolder xPage;
        sal_uInt32 offset = i * TEST_PAGESIZE;

        eErrCode = xLockBytes->readPageAt (xPage, offset);
        if (eErrCode != store_E_None)
        {
            fprintf (stderr, "t_file: readPageAt() error: %d\n", eErrCode);
            return eErrCode;
        }

        PageData * page = xPage.get();
        (void)page; // UNUSED
    }

    xLockBytes.clear();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
