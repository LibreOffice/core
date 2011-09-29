/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Julien Chaffraix <julien.chaffraix@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "readwrite_helper.h"

#include <osl/diagnose.h>
#include <system.h>

sal_Bool safeWrite(int fd, void* data, sal_uInt32 dataSize)
{
    sal_Int32 nToWrite = dataSize;
    unsigned char* dataToWrite = data;

    // Check for overflow as we convert a signed to an unsigned.
    OSL_ASSERT(dataSize == (sal_uInt32)nToWrite);
    while ( nToWrite ) {
        sal_Int32 nWritten = write(fd, dataToWrite, nToWrite);
        if ( nWritten < 0 ) {
            if ( errno == EINTR )
                continue;

            return sal_False;

        }

        OSL_ASSERT(nWritten > 0);
        nToWrite -= nWritten;
        dataToWrite += nWritten;
    }

    return sal_True;
}

sal_Bool safeRead( int fd, void* buffer, sal_uInt32 count )
{
    sal_Int32 nToRead = count;
    unsigned char* bufferForReading = buffer;

    // Check for overflow as we convert a signed to an unsigned.
    OSL_ASSERT(count == (sal_uInt32)nToRead);
    while ( nToRead ) {
        sal_Int32 nRead = read(fd, bufferForReading, nToRead);
        if ( nRead < 0 ) {
            // We were interrupted before reading, retry.
            if (errno == EINTR)
                continue;

            return sal_False;
        }

        // If we reach the EOF, we consider this a partial transfer and thus
        // an error.
        if ( nRead == 0 )
            return sal_False;

        nToRead -= nRead;
        bufferForReading += nRead;
    }

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
