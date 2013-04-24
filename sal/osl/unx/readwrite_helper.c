/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
