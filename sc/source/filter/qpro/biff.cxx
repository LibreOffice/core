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
#include "precompiled_sc.hxx"

#include <sal/config.h>
#include <stdio.h>
#include <sfx2/docfile.hxx>

#include "global.hxx"
#include "scerrors.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "biff.hxx"

ScBiffReader::ScBiffReader( SfxMedium & rMedium ) :
    mnId(0),
    mnLength(0),
    mnOffset(0)
{
    mpStream = rMedium.GetInStream();
    if( mpStream )
    {
        mpStream->SetBufferSize( 65535 );
        mpStream->SetStreamCharSet( RTL_TEXTENCODING_MS_1252 );
    }
}

ScBiffReader::~ScBiffReader()
{
    if( mpStream )
        mpStream->SetBufferSize( 0 );
}

bool ScBiffReader::nextRecord()
{
    if( !recordsLeft() )
        return false;

    if( IsEndOfFile() )
        return false;

    sal_uInt32 nPos = mpStream->Tell();
    if( nPos != mnOffset + mnLength )
        mpStream->Seek( mnOffset + mnLength );

    mnLength = mnId = 0;
    *mpStream >> mnId >> mnLength;

    mnOffset = mpStream->Tell();
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "Read record 0x%x length 0x%x at offset 0x%x\n",
        (unsigned)mnId, (unsigned)mnLength, (unsigned)mnOffset );

#if 1  // rather verbose
    int len = mnLength;
    while (len > 0) {
        int i, chunk = len < 16 ? len : 16;
        unsigned char data[16];
        mpStream->Read( data, chunk );

        for (i = 0; i < chunk; i++)
            fprintf( stderr, "%.2x ", data[i] );
        fprintf( stderr, "| " );
        for (i = 0; i < chunk; i++)
            fprintf( stderr, "%c", data[i] < 127 && data[i] > 30 ? data[i] : '.' );
        fprintf( stderr, "\n" );

        len -= chunk;
    }
    mpStream->Seek( mnOffset );
#endif
#endif
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
