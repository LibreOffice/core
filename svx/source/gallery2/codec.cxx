/*************************************************************************
 *
 *  $RCSfile: codec.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2001-11-07 08:42:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

BOOL GalleryCodec::IsCoded( SvStream& rStm, UINT32& rVersion )
{
    const ULONG nPos = rStm.Tell();
    BOOL        bRet;
    BYTE        cByte1, cByte2, cByte3, cByte4, cByte5, cByte6;

    rStm >> cByte1 >> cByte2 >> cByte3 >> cByte4 >> cByte5 >> cByte6;

    if ( cByte1 == 'S' && cByte2 == 'V' && cByte3 == 'R' && cByte4 == 'L' && cByte5 == 'E' && ( cByte6 == '1' || cByte6 == '2' ) )
    {
        rVersion = ( ( cByte6 == '1' ) ? 1 : 2 );
        bRet = TRUE;
    }
    else
    {
        rVersion = 0;
        bRet = FALSE;
    }

    rStm.Seek( nPos );

    return bRet;
}

// -----------------------------------------------------------------------------

ULONG GalleryCodec::Write( SvMemoryStream& rStmToWrite )
{
    UINT32 nPos, nCompSize;

    rStmToWrite.Seek( STREAM_SEEK_TO_END );
    const UINT32 nSize = rStmToWrite.Tell();
    rStmToWrite.Seek( 0UL );

    rStm << 'S' << 'V' << 'R' << 'L' << 'E' << '2';
    rStm << nSize;

    nPos = rStm.Tell();
    rStm.SeekRel( 4UL );

    ZCodec aCodec;
    aCodec.BeginCompression();
    aCodec.Write( rStm, static_cast< const BYTE* >( rStmToWrite.GetData() ), nSize );
    aCodec.EndCompression();

    nCompSize = rStm.Tell() - nPos - 4UL;
    rStm.Seek( nPos );
    rStm << nCompSize;
    rStm.Seek( STREAM_SEEK_TO_END );

    return 0UL;
}

// -----------------------------------------------------------------------------

ULONG GalleryCodec::Read( SvMemoryStream& rStmToRead )
{
    UINT32  nVersion = 0;
    UINT32  nReadBytes = 0;

    if( IsCoded( rStm, nVersion ) )
    {
        BYTE*   pUnCompressedBuffer = NULL;
        UINT32  nCompressedSize, nUnCompressedSize;

        rStm.SeekRel( 6 );
        rStm >> nUnCompressedSize >> nCompressedSize;

        // decompress
        if( 1 == nVersion )
            pUnCompressedBuffer = ImpReadRLEBuffer( rStm, nCompressedSize, nUnCompressedSize );
        else if( 2 == nVersion )
            pUnCompressedBuffer = ImpReadZBuffer( rStm, nCompressedSize, nUnCompressedSize );

        if( pUnCompressedBuffer )
            rStmToRead.SetBuffer( reinterpret_cast< char* >( pUnCompressedBuffer ), nUnCompressedSize, TRUE, nUnCompressedSize );
    }

    return nReadBytes;
}

// -----------------------------------------------------------------------------

BYTE* GalleryCodec::ImpReadRLEBuffer( SvStream& rIStm, ULONG nCompressedSize, ULONG nUnCompressedSize )
{
    BYTE*   pCompressedBuffer = new BYTE[ nCompressedSize ]; rIStm.Read( pCompressedBuffer, nCompressedSize );
    BYTE*   pInBuf = pCompressedBuffer;
    BYTE*   pOutBuf = new BYTE[ nUnCompressedSize ];
    BYTE*   pTmpBuf = pOutBuf;
    BYTE*   pLast = pOutBuf + nUnCompressedSize - 1;
    ULONG   nIndex = 0UL;
    ULONG   nCountByte;
    ULONG   nRunByte;
    BOOL    bEndDecoding = FALSE;

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
                bEndDecoding = TRUE;
        }
        else
        {
            const BYTE cVal = *pInBuf++;

            memset( &pTmpBuf[ nIndex ], cVal, nCountByte );
            nIndex += nCountByte;
        }
    }
    while ( !bEndDecoding && ( pTmpBuf <= pLast ) );

    delete[] pCompressedBuffer;

    return pOutBuf;
}

// -----------------------------------------------------------------------------

BYTE* GalleryCodec::ImpReadZBuffer( SvStream& rIStm, ULONG nCompressedSize, ULONG nUnCompressedSize )
{
    ZCodec  aCodec;
    BYTE*   pOutBuf = new BYTE[ nUnCompressedSize ];

    aCodec.BeginCompression();
    aCodec.Read( rIStm, pOutBuf, nUnCompressedSize );
    aCodec.EndCompression();

    return pOutBuf;
}
