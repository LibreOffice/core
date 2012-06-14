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
#include <string.h>
#include "base64.h"

static const char base64_tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

extern "C" size_t base64_encode( FILE *fin, FILE *fout )
{
    size_t nLineLength = 0;
    size_t nBytesWritten = 0;

    size_t nBytes = 0;

    do
    {
        unsigned char in_buffer[3];

        memset( in_buffer, 0, sizeof(in_buffer) );
        nBytes = fread( in_buffer, 1, sizeof(in_buffer), fin );

        if ( nBytes )
        {
            unsigned long value =
                ((unsigned long)in_buffer[0]) << 16 |
                ((unsigned long)in_buffer[1]) << 8 |
                ((unsigned long)in_buffer[2]) << 0;

            unsigned char out_buffer[4];

            memset( out_buffer, '=', sizeof(out_buffer) );

            out_buffer[0] = base64_tab[(value >> 18) & 0x3F];
            out_buffer[1] = base64_tab[(value >> 12) & 0x3F];

            if ( nBytes > 1 )
            {
                out_buffer[2] = base64_tab[(value >> 6) & 0x3F];
                if ( nBytes > 2 )
                    out_buffer[3] = base64_tab[(value >> 0) & 0x3F];
            }

            if ( nLineLength >= 76 )
            {
                fputs( "\n", fout );
                nLineLength = 0;
            }

            nBytesWritten += fwrite( out_buffer, 1, sizeof(out_buffer), fout );
            nLineLength += sizeof(out_buffer);
        }
    } while ( nBytes );

    return nBytesWritten;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
