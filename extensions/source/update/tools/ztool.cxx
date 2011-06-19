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
    UniString aInName = UniString::CreateFromAscii( argv[1] );
    UniString aOutName = UniString::CreateFromAscii( argv[2] );
    SvFileStream aInFile( aInName, STREAM_READ );
    SvFileStream aOutFile( aOutName, STREAM_WRITE | STREAM_TRUNC );

    ZCodec aCompressor;
    aCompressor.BeginCompression();
    aCompressor.Compress( aInFile, aOutFile );
    aCompressor.EndCompression();

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
