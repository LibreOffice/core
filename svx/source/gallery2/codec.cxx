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


#include <tools/stream.hxx>
#include <tools/zcodec.hxx>
#include <tools/debug.hxx>
#include "codec.hxx"

// - GalleryCodec -

DBG_NAME(GalleryCodec)

GalleryCodec::GalleryCodec( SvStream& rIOStm ) :
    rStm( rIOStm )
{
    DBG_CTOR(GalleryCodec,NULL);
}

GalleryCodec::~GalleryCodec()
{
    DBG_DTOR(GalleryCodec,NULL);
}

sal_Bool GalleryCodec::IsCoded( SvStream& rStm, sal_uInt32& rVersion )
{
    const sal_uIntPtr   nPos = rStm.Tell();
    sal_Bool        bRet;
    sal_uInt8       cByte1, cByte2, cByte3, cByte4, cByte5, cByte6;

    rStm >> cByte1 >> cByte2 >> cByte3 >> cByte4 >> cByte5 >> cByte6;

    if ( cByte1 == 'S' && cByte2 == 'V' && cByte3 == 'R' && cByte4 == 'L' && cByte5 == 'E' && ( cByte6 == '1' || cByte6 == '2' ) )
    {
        rVersion = ( ( cByte6 == '1' ) ? 1 : 2 );
        bRet = sal_True;
    }
    else
    {
        rVersion = 0;
        bRet = sal_False;
    }

    rStm.Seek( nPos );

    return bRet;
}

void GalleryCodec::Write( SvStream& rStmToWrite )
{
    sal_uInt32 nPos, nCompSize;

    rStmToWrite.Seek( STREAM_SEEK_TO_END );
    const sal_uInt32 nSize = rStmToWrite.Tell();
    rStmToWrite.Seek( 0UL );

    rStm << 'S' << 'V' << 'R' << 'L' << 'E' << '2';
    rStm << nSize;

    nPos = rStm.Tell();
    rStm.SeekRel( 4UL );

    ZCodec aCodec;
    aCodec.BeginCompression();
    aCodec.Compress( rStmToWrite, rStm );
    aCodec.EndCompression();

    nCompSize = rStm.Tell() - nPos - 4UL;
    rStm.Seek( nPos );
    rStm << nCompSize;
    rStm.Seek( STREAM_SEEK_TO_END );
}

void GalleryCodec::Read( SvStream& rStmToRead )
{
    sal_uInt32 nVersion = 0;

    if( IsCoded( rStm, nVersion ) )
    {
        sal_uInt32  nCompressedSize, nUnCompressedSize;

        rStm.SeekRel( 6 );
        rStm >> nUnCompressedSize >> nCompressedSize;

        // decompress
        if( 1 == nVersion )
        {
            sal_uInt8*   pCompressedBuffer = new sal_uInt8[ nCompressedSize ]; rStm.Read( pCompressedBuffer, nCompressedSize );
            sal_uInt8*  pInBuf = pCompressedBuffer;
            sal_uInt8*  pOutBuf = new sal_uInt8[ nUnCompressedSize ];
            sal_uInt8*  pTmpBuf = pOutBuf;
            sal_uInt8*  pLast = pOutBuf + nUnCompressedSize - 1;
            sal_uIntPtr   nIndex = 0UL, nCountByte, nRunByte;
            sal_Bool    bEndDecoding = sal_False;

            do
            {
                nCountByte = *pInBuf++;

                if ( !nCountByte )
                {
                    nRunByte = *pInBuf++;

                    if ( nRunByte > 2 )
                    {
                        // filling absolutely
                        memcpy( &pTmpBuf[ nIndex ], pInBuf, nRunByte );
                        pInBuf += nRunByte;
                        nIndex += nRunByte;

                        // note WORD alignment
                        if ( nRunByte & 1 )
                            pInBuf++;
                    }
                    else if ( nRunByte == 1 )   // End of the image
                        bEndDecoding = sal_True;
                }
                else
                {
                    const sal_uInt8 cVal = *pInBuf++;

                    memset( &pTmpBuf[ nIndex ], cVal, nCountByte );
                    nIndex += nCountByte;
                }
            }
            while ( !bEndDecoding && ( pTmpBuf <= pLast ) );

               rStmToRead.Write( pOutBuf, nUnCompressedSize );

            delete[] pOutBuf;
            delete[] pCompressedBuffer;
        }
        else if( 2 == nVersion )
        {
            ZCodec aCodec;

            aCodec.BeginCompression();
            aCodec.Decompress( rStm, rStmToRead );
            aCodec.EndCompression();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
