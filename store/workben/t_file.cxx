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


#include "sal/types.h"
#include "osl/thread.h"
#include "rtl/ustring.hxx"

#include "lockbyte.hxx"

#include <string.h>
#include <stdio.h>

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
    memset (buffer, sal_uInt8('B'), sizeof(buffer));

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
            if (memcmp (&verify[index], &magic, sizeof(magic)))
            {
                // Failure.
                fprintf (stderr, "t_file: Unexpected value at 0x00000000\n");
            }
            index += 4;
        }
        if (memcmp (
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
