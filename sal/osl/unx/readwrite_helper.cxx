/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "readwrite_helper.hxx"

#include <osl/diagnose.h>
#include <system.hxx>

#include <cassert>

bool safeWrite(int fd, void* data, sal_uInt32 dataSize)
{
    sal_Int32 nToWrite = dataSize;
    unsigned char* dataToWrite = static_cast<unsigned char *>(data);

    // Check for overflow as we convert a signed to an unsigned.
    assert(dataSize == (sal_uInt32)nToWrite);
    while ( nToWrite ) {
        sal_Int32 nWritten = write(fd, dataToWrite, nToWrite);
        if ( nWritten < 0 ) {
            if ( errno == EINTR )
                continue;

            return false;

        }

        assert(nWritten > 0);
        nToWrite -= nWritten;
        dataToWrite += nWritten;
    }

    return true;
}

bool safeRead( int fd, void* buffer, sal_uInt32 count )
{
    sal_Int32 nToRead = count;
    unsigned char* bufferForReading = static_cast<unsigned char *>(buffer);

    // Check for overflow as we convert a signed to an unsigned.
    assert(count == (sal_uInt32)nToRead);
    while ( nToRead ) {
        sal_Int32 nRead = read(fd, bufferForReading, nToRead);
        if ( nRead < 0 ) {
            // We were interrupted before reading, retry.
            if (errno == EINTR)
                continue;

            return false;
        }

        // If we reach the EOF, we consider this a partial transfer and thus
        // an error.
        if ( nRead == 0 )
            return false;

        nToRead -= nRead;
        bufferForReading += nRead;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
