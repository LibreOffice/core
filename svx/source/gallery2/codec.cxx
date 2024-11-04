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


#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <tools/zcodec.hxx>
#include "codec.hxx"
#include <memory>


GalleryCodec::GalleryCodec( SvStream& rIOStm ) :
    rStm( rIOStm )
{
}

bool GalleryCodec::IsCoded( SvStream& rStm, sal_uInt32& rVersion )
{
    const sal_uInt64   nPos = rStm.Tell();
    bool        bRet;
    sal_uInt8       cByte1, cByte2, cByte3, cByte4, cByte5, cByte6;

    rStm.ReadUChar( cByte1 ).ReadUChar( cByte2 ).ReadUChar( cByte3 ).ReadUChar( cByte4 ).ReadUChar( cByte5 ).ReadUChar( cByte6 );

    if (rStm.good() && cByte1 == 'S' && cByte2 == 'V' && cByte3 == 'R' && cByte4 == 'L' && cByte5 == 'E' && ( cByte6 == '1' || cByte6 == '2' ) )
    {
        rVersion = ( ( cByte6 == '1' ) ? 1 : 2 );
        bRet = true;
    }
    else
    {
        rVersion = 0;
        bRet = false;
    }

    rStm.Seek( nPos );

    return bRet;
}

void GalleryCodec::Write( SvStream& rStmToWrite )
{
    sal_uInt32 nPos, nCompSize;

    const sal_uInt32 nSize = rStmToWrite.TellEnd();
    rStmToWrite.Seek( 0 );

    rStm.WriteChar( 'S' ).WriteChar( 'V' ).WriteChar( 'R' ).WriteChar( 'L' ).WriteChar( 'E' ).WriteChar( '2' );
    rStm.WriteUInt32( nSize );

    nPos = rStm.Tell();
    rStm.SeekRel( 4 );

    ZCodec aCodec;
    aCodec.BeginCompression();
    aCodec.Compress( rStmToWrite, rStm );
    aCodec.EndCompression();

    nCompSize = rStm.Tell() - nPos - 4;
    rStm.Seek( nPos );
    rStm.WriteUInt32( nCompSize );
    rStm.Seek( STREAM_SEEK_TO_END );
}

void GalleryCodec::Read( SvStream& rStmToRead )
{
    sal_uInt32 nVersion = 0;

    if( !IsCoded( rStm, nVersion ) )
        return;

    rStm.SeekRel( 14 );

    // decompress
    if( 1 == nVersion )
    {
        SAL_WARN("svx", "staroffice binary file formats are no longer supported inside the gallery!");
    }
    else if( 2 == nVersion )
    {
        ZCodec aCodec;

        aCodec.BeginCompression();
        aCodec.Decompress( rStm, rStmToRead );
        aCodec.EndCompression();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
