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

#include <tools/stream.hxx>
#include <tools/zcodec.hxx>

void show_usage()
{
    fputs("ztool usage: ztool <source> <dest>\n", stderr);
}

int SAL_CALL main( int argc, char **argv )
{
    if ( argc != 3 )
    {
        show_usage();
        return -1;
    }
    OUString aInName = OUString::createFromAscii(argv[1]);
    OUString aOutName = OUString::createFromAscii(argv[2]);
    SvFileStream aInFile( aInName, STREAM_READ );
    SvFileStream aOutFile( aOutName, STREAM_WRITE | STREAM_TRUNC );

    ZCodec aCompressor;
    aCompressor.BeginCompression();
    aCompressor.Compress( aInFile, aOutFile );
    aCompressor.EndCompression();

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
