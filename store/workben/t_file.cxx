/*************************************************************************
 *
 *  $RCSfile: t_file.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mhu $ $Date: 2001-11-29 18:27:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#define _T_FILE_CXX "$Revision: 1.5 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _STORE_FILELCKB_HXX_
#include <store/filelckb.hxx>
#endif

#ifndef INCLUDED_STDIO_H
#include <stdio.h>
#define INCLUDED_STDIO_H
#endif

using namespace store;

#define TEST_PAGESIZE 16384

/*========================================================================
 *
 * main.
 *
 *======================================================================*/
int SAL_CALL main (int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf (stderr, "usage: t_file <output-filename>\n");
        return 0;
    }

    rtl::Reference<OFileLockBytes> xLockBytes (new OFileLockBytes());
    if (!xLockBytes.is())
        return 0;

    rtl::OUString aFilename (
        argv[1], rtl_str_getLength(argv[1]),
        osl_getThreadTextEncoding());

    storeError eErrCode = xLockBytes->create (
        aFilename.pData, store_AccessReadWrite);
    if (eErrCode != store_E_None)
    {
        // Check reason.
        if (eErrCode != store_E_NotExists)
        {
            fprintf (stderr, "t_file: create() error: %d\n", eErrCode);
            return eErrCode;
        }

        // Create.
        eErrCode = xLockBytes->create (
            aFilename.pData, store_AccessReadCreate);
        if (eErrCode != store_E_None)
        {
            fprintf (stderr, "t_file: create() error: %d\n", eErrCode);
            return eErrCode;
        }
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
            sal_uInt32 magic = i * 4 + k, done = 0;
            if (magic)
            {
                sal_uInt32 verify = 0;
                eErrCode = xLockBytes->readAt (
                    0, &verify, sizeof(verify), done);
                if (eErrCode != store_E_None)
                {
                    fprintf (stderr, "t_file: readAt() error: %d\n", eErrCode);
                    return eErrCode;
                }
                if (verify != magic)
                {
                    // Failure.
                    fprintf (stderr, "Expected %d read %d\n", magic, verify);
                }
            }

            sal_uInt32 index = k * TEST_PAGESIZE / 4;
            eErrCode = xLockBytes->writeAt (
                offset + index, &(buffer[index]), TEST_PAGESIZE / 4, done);
            if (eErrCode != store_E_None)
            {
                fprintf (stderr, "t_file: writeAt() error: %d\n", eErrCode);
                return eErrCode;
            }

            magic += 1;
            eErrCode = xLockBytes->writeAt (
                0, &magic, sizeof(magic), done);
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
        sal_uInt32 offset = i * TEST_PAGESIZE, done = 0;

        eErrCode = xLockBytes->readAt (offset, verify, TEST_PAGESIZE, done);
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
            fprintf (stderr, "t_file: Unexpected block at 0x%08x\n", offset);
        }
    }

    xLockBytes.clear();
    return 0;
}

