/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <errno.h>
#include <limits>
#include <unistd.h>

#include "readwrite_helper.hxx"

namespace {

std::size_t cap_ssize_t(std::size_t value) {
    return std::min(value, std::size_t(std::numeric_limits<ssize_t>::max()));
}

}

bool safeWrite(int fd, void* data, std::size_t dataSize)
{
    auto nToWrite = dataSize;
    unsigned char* dataToWrite = static_cast<unsigned char *>(data);

    while ( nToWrite ) {
        auto nWritten = write(fd, dataToWrite, cap_ssize_t(nToWrite));
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

bool safeRead( int fd, void* buffer, std::size_t count )
{
    auto nToRead = count;
    unsigned char* bufferForReading = static_cast<unsigned char *>(buffer);

    while ( nToRead ) {
        auto nRead = read(fd, bufferForReading, cap_ssize_t(nToRead));
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
