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
#include "precompiled_svx.hxx"

#include <tools/stream.hxx>
#include <tools/zcodec.hxx>
#include "codec.hxx"

// ----------------
// - GalleryCodec -
// ----------------

GalleryCodec::GalleryCodec( SvStream& rIOStm ) :
    rStm( rIOStm )
{
}

// -----------------------------------------------------------------------------

GalleryCodec::~GalleryCodec()
{
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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
                        // absolutes Fuellen
                        memcpy( &pTmpBuf[ nIndex ], pInBuf, nRunByte );
                        pInBuf += nRunByte;
                        nIndex += nRunByte;

                        // WORD-Alignment beachten
                        if ( nRunByte & 1 )
                            pInBuf++;
                    }
                    else if ( nRunByte == 1 )   // Ende des Bildes
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
