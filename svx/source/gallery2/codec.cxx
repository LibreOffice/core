/*************************************************************************
 *
 *  $RCSfile: codec.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:18 $
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


#include <string.h>
#include <tools/stream.hxx>
#include <tools/new.hxx>
#include <svtools/fltdefs.hxx>
#include "codec.hxx"

#define SUBBUFFER_SIZE  8192

#ifdef WIN
typedef BYTE huge*  HPBYTE;
#else
typedef BYTE*       HPBYTE;
#endif

/******************************************************************************
|*
|*
|*
\******************************************************************************/

RLECodec::RLECodec( SvStream& rIOStm ) : rStm( rIOStm )
{
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

RLECodec::~RLECodec()
{
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

BOOL RLECodec::IsRLECoded( SvStream& rStm )
{
    const ULONG nPos = rStm.Tell();
    BOOL        bRet = FALSE;
    BYTE        cByte1, cByte2, cByte3, cByte4, cByte5, cByte6;

    rStm >> cByte1 >> cByte2 >> cByte3 >> cByte4 >> cByte5 >> cByte6;

    if ( cByte1 == 'S' && cByte2 == 'V' && cByte3 == 'R' &&
         cByte4 == 'L' && cByte5 == 'E' && cByte6 == '1' )
    {
        bRet = TRUE;
    }

    rStm.Seek( nPos );

    return bRet;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

ULONG RLECodec::Write( SvMemoryStream& rStmToWrite )
{
    UINT32  nPos;
    UINT32  nCompSize;

    rStmToWrite.Seek( STREAM_SEEK_TO_END );
    const UINT32 nSize = rStmToWrite.Tell();
    rStmToWrite.Seek( 0UL );

    rStm << 'S' << 'V' << 'R' << 'L' << 'E' << '1';
    rStm << nSize;

    nPos = rStm.Tell();
    rStm.SeekRel( 4UL );

    ImpWriteBuffer( (BYTE*) rStmToWrite.GetData(), nSize );

    nCompSize = rStm.Tell() - nPos - 4UL;
    rStm.Seek( nPos );
    rStm << nCompSize;
    rStm.Seek( STREAM_SEEK_TO_END );

    return 0UL;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

ULONG RLECodec::Read( SvMemoryStream& rStmToRead )
{
    UINT32 nReadBytes = 0UL;

    if ( IsRLECoded( rStm ) )
    {
        UINT32  nCompressedSize;

        rStm.SeekRel( 6 );
        rStm >> nReadBytes;
        rStm >> nCompressedSize;

        HPBYTE pReadBuffer = (HPBYTE) SvMemAlloc( nCompressedSize );

        // komprimierte Bytefolge lesen
        rStm.Read( pReadBuffer, nCompressedSize );

        // Dekomprimieren
        rStmToRead.SetBuffer( (char*) ImpReadBuffer( pReadBuffer, nReadBytes ),
                              nReadBytes, TRUE, nReadBytes );

        SvMemFree( pReadBuffer );
    }

    return nReadBytes;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void RLECodec::ImpWriteBuffer( BYTE* pInBuf, const ULONG nSize )
{
    HPBYTE      pTmpIn = (HPBYTE) pInBuf;
    const ULONG nCount = nSize / SUBBUFFER_SIZE;
    const ULONG nRest = nCount ? nSize % ( nCount * SUBBUFFER_SIZE ) : nSize;

    for( ULONG i = 0; i < nCount; i++ )
        ImpWriteSubBuffer( &pTmpIn[ i * SUBBUFFER_SIZE ], SUBBUFFER_SIZE );

    if ( nRest )
        ImpWriteSubBuffer( &pTmpIn[ nCount * SUBBUFFER_SIZE ], nRest );

    // BlockEnde schreiben
    rStm << (BYTE) 0;
    rStm << (BYTE) 1;
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

void RLECodec::ImpWriteSubBuffer( BYTE* pInBuf, const ULONG nSize )
{
    HPBYTE      pTmpIn = (HPBYTE) pInBuf;
    HPBYTE      pOutBuf = (HPBYTE) SvMemAlloc( nSize << 1 );
    HPBYTE      pTmpOut = pOutBuf;
    ULONG       nIndex = 0UL;
    ULONG       nBufCount = 0UL;
    ULONG       nSaveIndex;
    ULONG       nCount;
    BYTE        cPix;
    BYTE        cLast;
    BOOL        bFound;

    while( nIndex < nSize )
    {
        nCount = 1L;
        cPix = pTmpIn[ nIndex++ ];

        // Anzahl der aufeinanderf. gleichen Eintraege bestimmen
        while( ( nIndex < nSize ) && ( nCount < 255L ) && ( pTmpIn[ nIndex ] == cPix ) )
        {
            nIndex++;
            nCount++;
        }

        // gleiche Eintraege hintereinander werden kodiert geschrieben
        if ( nCount > 1 )
        {
            *pTmpOut++ = (BYTE) nCount;
            *pTmpOut++ = cPix;
            nBufCount += 2;
        }
        else
        {
            cLast = cPix;
            nSaveIndex = nIndex - 1;
            bFound = FALSE;

            // Anzahl der unterschiedlichen Eintraege bestimmen
            while( ( nIndex < nSize ) && ( nCount < 256L ) && ( ( cPix = pTmpIn[ nIndex ] ) != cLast ) )
            {
                nIndex++;
                nCount++;
                cLast = cPix;
                bFound = TRUE;
            }

            // Falls untersch. Eintrag gefunden, dec. wir den Index
            // da der letzte Index der erste der neuen Sequenz
            // sein soll
            if ( bFound )
                nIndex--;

            // mehrere untersch. Eintraege ueber abs. Coding schreiben
            if ( nCount > 3 )
            {
                // wir wollen nur so wenig wie moeglich
                // unterschiedliche Eintraege schreiben
                *pTmpOut++ = 0;
                *pTmpOut++ = (BYTE) --nCount;

                MEMCPY( pTmpOut, &pTmpIn[ nSaveIndex ], nCount );
                pTmpOut += nCount;

                // WORD-Alignment beachten
                if ( nCount & 1 )
                {
                    *pTmpOut++ = 0;
                    nBufCount += ( nCount + 3 );
                }
                else
                    nBufCount += ( nCount + 2 );
            }
            else
            {
                *pTmpOut++ = 1;
                *pTmpOut++ = pTmpIn[ nSaveIndex ];

                if ( nCount == 3 )
                {
                    *pTmpOut++ = 1;
                    *pTmpOut++ = pTmpIn[ ++nSaveIndex ];
                    nBufCount += 4;
                }
                else
                    nBufCount += 2;
            }
        }
    }

    // Puffer schreiben
    rStm.Write( pOutBuf, nBufCount );

    // temp. Puffer wieder zerstoeren
    SvMemFree( pOutBuf );
}


/******************************************************************************
|*
|*
|*
\******************************************************************************/

BYTE* RLECodec::ImpReadBuffer( BYTE* pStmBuf, const ULONG nSize )
{
    HPBYTE  pInBuf = (HPBYTE) pStmBuf;
    HPBYTE  pOutBuf = (HPBYTE) SvMemAlloc( nSize );
    HPBYTE  pTmpBuf = pOutBuf;
    HPBYTE  pLast = pOutBuf + nSize - 1;
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
                MEMCPY( &pTmpBuf[ nIndex ], pInBuf, nRunByte );
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

            MEMSET( &pTmpBuf[ nIndex ], cVal, nCountByte );
            nIndex += nCountByte;
        }
    }
    while ( !bEndDecoding && ( pTmpBuf <= pLast ) );

    return pOutBuf;
}
