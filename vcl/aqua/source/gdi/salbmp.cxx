/*************************************************************************
 *
 *  $RCSfile: salbmp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:26 $
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

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#define _SV_SALBMP_CXX

#ifndef _SV_SALAQUA_HXX
#include <salaqua.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#include <string.h>

#ifdef WIN
#define BI_BITFIELDS    3
#endif

// -----------
// - Inlines -
// -----------

inline void ImplSetPixel4( const HPBYTE pScanline, long nX, const BYTE cIndex )
{
    BYTE& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0xf0, rByte |= ( cIndex & 0x0f ) ) :
                 ( rByte &= 0x0f, rByte |= ( cIndex << 4 ) );
}

// -------------
// - SalBitmap -
// -------------

SalBitmap::SalBitmap() :
        mhDIB       ( 0 ),
        mhDDB       ( 0 ),
        mnBitCount  ( 0 )
{
}

// ------------------------------------------------------------------

SalBitmap::~SalBitmap()
{
    Destroy();
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( HANDLE hBitmap, BOOL bDIB, BOOL bCopyHandle )
{
    BOOL bRet = TRUE;

    if( bDIB )
        mhDIB = (HGLOBAL) ( bCopyHandle ? ImplCopyDIBOrDDB( hBitmap, TRUE ) : hBitmap );
    else
        mhDDB = (HBITMAP) ( bCopyHandle ? ImplCopyDIBOrDDB( hBitmap, FALSE ) : hBitmap );

#ifdef WIN
    if( mhDIB )
    {
        PBITMAPINFOHEADER pBIH = (PBITMAPINFOHEADER) GlobalLock( mhDIB );

        maSize = Size( pBIH->biWidth, pBIH->biHeight );
        mnBitCount = pBIH->biBitCount;

        if( mnBitCount )
            mnBitCount = ( mnBitCount <= 1 ) ? 1 : ( mnBitCount <= 4 ) ? 4 : ( mnBitCount <= 8 ) ? 8 : 24;

        GlobalUnlock( mhDIB );
    }
    else if( mhDDB )
    {
        BITMAP  aDDBInfo;

        if( WIN_GetObject( mhDDB, sizeof( BITMAP ), &aDDBInfo ) )
        {
            maSize = Size( aDDBInfo.bmWidth, aDDBInfo.bmHeight );
            mnBitCount = aDDBInfo.bmPlanes * aDDBInfo.bmBitsPixel;

            if( mnBitCount )
            {
                mnBitCount = ( mnBitCount <= 1 ) ? 1 :
                             ( mnBitCount <= 4 ) ? 4 :
                             ( mnBitCount <= 8 ) ? 8 : 24;
            }
        }
        else
        {
            mhDDB = 0;
            bRet = FALSE;
        }
    }
    else
#endif
        bRet = FALSE;

    return bRet;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal )
{
    BOOL bRet = FALSE;

    mhDIB = ImplCreateDIB( rSize, nBitCount, rPal );

    if( mhDIB )
    {
        maSize = rSize;
        mnBitCount = nBitCount;
        bRet = TRUE;
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBitmap )
{
    BOOL bRet = FALSE;

    if ( rSalBitmap.mhDIB || rSalBitmap.mhDDB )
    {
        HANDLE hNewHdl = ImplCopyDIBOrDDB( rSalBitmap.mhDIB ? rSalBitmap.mhDIB : rSalBitmap.mhDDB,
                                           rSalBitmap.mhDIB != 0 );

        if ( hNewHdl )
        {
            if( rSalBitmap.mhDIB )
                mhDIB = (HGLOBAL) hNewHdl;
            else if( rSalBitmap.mhDDB )
                mhDDB = (HBITMAP) hNewHdl;

            maSize = rSalBitmap.maSize;
            mnBitCount = rSalBitmap.mnBitCount;

            bRet = TRUE;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, SalGraphics* pGraphics )
{
    BOOL bRet = FALSE;

    if( rSalBmp.mhDIB )
    {
#ifdef WIN
        PBITMAPINFO         pBI = (PBITMAPINFO) GlobalLock( rSalBmp.mhDIB );
        PBITMAPINFOHEADER   pBIH = (PBITMAPINFOHEADER) pBI;
        HDC                 hDC  = pGraphics->maGraphicsData.mhDC;
        HBITMAP             hNewDDB;
        BITMAP              aDDBInfo;
        PBYTE               pBits = (PBYTE) pBI + *(DWORD*) pBI +
                            ImplGetDIBColorCount( rSalBmp.mhDIB ) * sizeof( RGBQUAD );

        if( pBIH->biBitCount == 1 )
        {
            hNewDDB = CreateBitmap( pBIH->biWidth, pBIH->biHeight, 1, 1, NULL );

            if( hNewDDB )
                SetDIBits( hDC, hNewDDB, 0, pBIH->biHeight, pBits, pBI, DIB_RGB_COLORS );
        }
        else
            hNewDDB = CreateDIBitmap( hDC, (PBITMAPINFOHEADER) pBI, CBM_INIT, pBits, pBI, DIB_RGB_COLORS );

        GlobalUnlock( rSalBmp.mhDIB );

        if( hNewDDB && WIN_GetObject( hNewDDB, sizeof( BITMAP ), &aDDBInfo ) )
        {
            mhDDB = hNewDDB;
            maSize = Size( aDDBInfo.bmWidth, aDDBInfo.bmHeight );
            mnBitCount = aDDBInfo.bmPlanes * aDDBInfo.bmBitsPixel;

            if( mnBitCount )
            {
                mnBitCount = ( mnBitCount <= 1 ) ? 1 :
                             ( mnBitCount <= 4 ) ? 4 :
                             ( mnBitCount <= 8 ) ? 8 : 24;
            }

            bRet = TRUE;
        }
        else if( hNewDDB )
            DeleteObject( hNewDDB );
#endif
    }

    return bRet;
}

// ------------------------------------------------------------------

BOOL SalBitmap::Create( const SalBitmap& rSalBmp, USHORT nNewBitCount )
{
    BOOL bRet = FALSE;

    if( rSalBmp.mhDDB )
    {
        mhDIB = ImplCreateDIB( rSalBmp.maSize, nNewBitCount, BitmapPalette() );

        if( mhDIB )
        {
#ifdef WIN
            PBITMAPINFO pBI = (PBITMAPINFO) GlobalLock( mhDIB );
            const int   nLines = (int) rSalBmp.maSize.Height();
            HDC         hDC = GetDC( 0 );
            PBYTE       pBits = (PBYTE) pBI + *(DWORD*) pBI +
                                ImplGetDIBColorCount( mhDIB ) * sizeof( RGBQUAD );
            SalData*    pSalData = GetSalData();
            HPALETTE    hOldPal = 0;

            if ( pSalData->mhDitherPal )
            {
                hOldPal = SelectPalette( hDC, pSalData->mhDitherPal, TRUE );
                RealizePalette( hDC );
            }

            if( GetDIBits( hDC, rSalBmp.mhDDB, 0, nLines, pBits, pBI, DIB_RGB_COLORS ) == nLines )
            {
                GlobalUnlock( mhDIB );
                maSize = rSalBmp.maSize;
                mnBitCount = nNewBitCount;
                bRet = TRUE;
            }
            else
            {
                GlobalUnlock( mhDIB );
                GlobalFree( mhDIB );
                mhDIB = 0;
            }

            if( hOldPal )
                SelectPalette( hDC, hOldPal, TRUE );

            ReleaseDC( 0, hDC );
#endif
        }
    }

    return bRet;
}

// ------------------------------------------------------------------

void SalBitmap::Destroy()
{
#ifdef WIN
    if( mhDIB )
        GlobalFree( mhDIB );
    else if( mhDDB )
        DeleteObject( mhDDB );
#endif

    maSize = Size();
    mnBitCount = 0;
}

// ------------------------------------------------------------------

USHORT SalBitmap::ImplGetDIBColorCount( HGLOBAL hDIB )
{
    USHORT nColors = 0;

    if( hDIB )
    {
#ifdef WIN
        PBITMAPINFO         pBI = (PBITMAPINFO) GlobalLock( hDIB );
        PBITMAPINFOHEADER   pBIH = (PBITMAPINFOHEADER) pBI;

        if ( pBIH->biSize != sizeof( BITMAPCOREHEADER ) )
        {
            if( pBIH->biBitCount <= 8 )
            {
                if ( pBIH->biClrUsed )
                    nColors = (USHORT) pBIH->biClrUsed;
                else
                    nColors = 1 << pBIH->biBitCount;
            }
        }
        else if( ( (PBITMAPCOREHEADER) pBI )->bcBitCount <= 8 )
            nColors = 1 << ( (PBITMAPCOREHEADER) pBI )->bcBitCount;

        GlobalUnlock( hDIB );
#endif
    }

    return nColors;
}

// ------------------------------------------------------------------

HGLOBAL SalBitmap::ImplCreateDIB( const Size& rSize, USHORT nBits, const BitmapPalette& rPal )
{
    DBG_ASSERT( nBits == 1 || nBits == 4 || nBits == 8 || nBits == 24, "Unsupported BitCount!" );

    HGLOBAL hDIB = 0;

    if ( rSize.Width() && rSize.Height() && ( nBits == 1 || nBits == 4 || nBits == 8 || nBits == 24 ) )
    {
#ifdef WIN
        const ULONG     nImageSize = AlignedWidth4Bytes( nBits * rSize.Width() ) * rSize.Height();
        const USHORT    nColors = ( nBits <= 8 ) ? ( 1 << nBits ) : 0;

        hDIB = GlobalAlloc( GHND, sizeof( BITMAPINFOHEADER ) + nColors * sizeof( RGBQUAD ) + nImageSize );

        if( hDIB )
        {
            PBITMAPINFO         pBI = (PBITMAPINFO) GlobalLock( hDIB );
            PBITMAPINFOHEADER   pBIH = (PBITMAPINFOHEADER) pBI;

            pBIH->biSize = sizeof( BITMAPINFOHEADER );
            pBIH->biWidth = rSize.Width();
            pBIH->biHeight = rSize.Height();
            pBIH->biPlanes = 1;
            pBIH->biBitCount = nBits;
            pBIH->biCompression = BI_RGB;
            pBIH->biSizeImage = nImageSize;
            pBIH->biXPelsPerMeter = 0;
            pBIH->biYPelsPerMeter = 0;
            pBIH->biClrUsed = 0;
            pBIH->biClrImportant = 0;

            if ( nColors )
            {
                const USHORT nMinCount = Min( nColors, rPal.GetEntryCount() );

                if( nMinCount )
                    HMEMCPY( pBI->bmiColors, rPal.ImplGetColorBuffer(), nMinCount * sizeof( RGBQUAD ) );
            }

            GlobalUnlock( hDIB );
        }
#endif
    }

    return hDIB;
}

// ------------------------------------------------------------------

HANDLE SalBitmap::ImplCopyDIBOrDDB( HANDLE hHdl, BOOL bDIB )
{
    HANDLE  hCopy = 0;

#ifdef WIN
    if ( bDIB && hHdl )
    {
        const ULONG nSize = GlobalSize( hHdl );

        if ( hCopy = GlobalAlloc( GHND, nSize  ) )
        {
            HMEMCPY( (LPSTR) GlobalLock( hCopy ), (LPSTR) GlobalLock( hHdl ), nSize );

            GlobalUnlock( hCopy );
            GlobalUnlock( hHdl );
        }
    }
    else if ( hHdl )
    {
        BITMAP aBmp;

        // Source-Bitmap nach Groesse befragen
        WIN_GetObject( hHdl, sizeof( BITMAP ), (LPSTR) &aBmp );

        // Destination-Bitmap erzeugen
        if ( hCopy = CreateBitmapIndirect( &aBmp ) )
        {
            HDC     hBmpDC = CreateCompatibleDC( 0 );
            HBITMAP hBmpOld = (HBITMAP) SelectObject( hBmpDC, hHdl );
            HDC     hCopyDC = CreateCompatibleDC( hBmpDC );
            HBITMAP hCopyOld = (HBITMAP) SelectObject( hCopyDC, hCopy );

            BitBlt( hCopyDC, 0, 0, aBmp.bmWidth, aBmp.bmHeight, hBmpDC, 0, 0, SRCCOPY );

            SelectObject( hCopyDC, hCopyOld );
            DeleteDC( hCopyDC );

            SelectObject( hBmpDC, hBmpOld );
            DeleteDC( hBmpDC );
        }
    }
#endif

    return hCopy;
}

// ------------------------------------------------------------------

BitmapBuffer* SalBitmap::AcquireBuffer( BOOL bReadOnly )
{
    BitmapBuffer* pBuffer = NULL;

    if( mhDIB )
    {
#ifdef WIN
        PBITMAPINFO         pBI = (PBITMAPINFO) GlobalLock( mhDIB );
        PBITMAPINFOHEADER   pBIH = (PBITMAPINFOHEADER) pBI;

        if( ( pBIH->biCompression == BI_RLE4 ) || ( pBIH->biCompression == BI_RLE8 ) )
        {
            Size    aSizePix( pBIH->biWidth, pBIH->biHeight );
            HGLOBAL hNewDIB = ImplCreateDIB( aSizePix, pBIH->biBitCount, BitmapPalette() );

            if( hNewDIB )
            {
                PBITMAPINFO         pNewBI = (PBITMAPINFO) GlobalLock( hNewDIB );
                PBITMAPINFOHEADER   pNewBIH = (PBITMAPINFOHEADER) pNewBI;
                const USHORT        nColorCount = ImplGetDIBColorCount( hNewDIB );
                const ULONG         nOffset = *(DWORD*) pBI + nColorCount * sizeof( RGBQUAD );
                BYTE*               pOldBits = (PBYTE) pBI + nOffset;
                BYTE*               pNewBits = (PBYTE) pNewBI + nOffset;

                HMEMCPY( pNewBI, pBI, nOffset );
                pNewBIH->biCompression = 0;
                ImplDecodeRLEBuffer( pOldBits, pNewBits, aSizePix, pBIH->biCompression == BI_RLE4 );

                GlobalUnlock( mhDIB );
                GlobalFree( mhDIB );
                mhDIB = hNewDIB;
                pBI = pNewBI;
                pBIH = pNewBIH;
            }
        }

        if( pBIH->biPlanes == 1 )
        {
            pBuffer = new BitmapBuffer;

            pBuffer->mnFormat = BMP_FORMAT_BOTTOM_UP |
                                ( pBIH->biBitCount == 1 ? BMP_FORMAT_1BIT_MSB_PAL :
                                  pBIH->biBitCount == 4 ? BMP_FORMAT_4BIT_MSN_PAL :
                                  pBIH->biBitCount == 8 ? BMP_FORMAT_8BIT_PAL :
                                  pBIH->biBitCount == 16 ? BMP_FORMAT_16BIT_TC_MASK :
                                  pBIH->biBitCount == 24 ? BMP_FORMAT_24BIT_TC_BGR :
                                  pBIH->biBitCount == 32 ? BMP_FORMAT_32BIT_TC_MASK : 0UL );

            if( BMP_SCANLINE_FORMAT( pBuffer->mnFormat ) )
            {
                pBuffer->mnWidth = maSize.Width();
                pBuffer->mnHeight = maSize.Height();
                pBuffer->mnScanlineSize = AlignedWidth4Bytes( maSize.Width() * pBIH->biBitCount );
                pBuffer->mnBitCount = (USHORT) pBIH->biBitCount;

                if( pBuffer->mnBitCount <= 8 )
                {
                    const USHORT nPalCount = ImplGetDIBColorCount( mhDIB );

                    pBuffer->maPalette.SetEntryCount( nPalCount );
                    HMEMCPY( pBuffer->maPalette.ImplGetColorBuffer(), pBI->bmiColors, nPalCount * sizeof( RGBQUAD ) );
                    pBuffer->mpBits = (PBYTE) pBI + *(DWORD*) pBI + nPalCount * sizeof( RGBQUAD );
                }
                else if( ( pBIH->biBitCount == 16 ) || ( pBIH->biBitCount == 32 ) )
                {
                    ULONG nOffset = 0UL;

                    if( pBIH->biCompression == BI_BITFIELDS )
                    {
                        nOffset = 3 * sizeof( RGBQUAD );
                        pBuffer->maColorMask = ColorMask( *(UINT32*) &pBI->bmiColors[ 0 ],
                                                          *(UINT32*) &pBI->bmiColors[ 1 ],
                                                          *(UINT32*) &pBI->bmiColors[ 2 ] );
                    }
                    else if( pBIH->biCompression == 16 )
                        pBuffer->maColorMask = ColorMask( 0x00007c00UL, 0x000003e0UL, 0x0000001fUL );
                    else
                        pBuffer->maColorMask = ColorMask( 0x00ff0000UL, 0x0000ff00UL, 0x000000ffUL );

                    pBuffer->mpBits = (PBYTE) pBI + *(DWORD*) pBI + nOffset;
                }
                else
                    pBuffer->mpBits = (PBYTE) pBI + *(DWORD*) pBI;
            }
            else
            {
                GlobalUnlock( mhDIB );
                delete pBuffer;
                pBuffer = NULL;
            }
        }
        else
            GlobalUnlock( mhDIB );
#endif
    }

    return pBuffer;
}

// ------------------------------------------------------------------

void SalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BOOL bReadOnly )
{
    if( pBuffer )
    {
        if( mhDIB )
        {
#ifdef WIN
            if( !bReadOnly && !!pBuffer->maPalette )
            {
                PBITMAPINFO     pBI = (PBITMAPINFO) GlobalLock( mhDIB );
                const USHORT    nCount = pBuffer->maPalette.GetEntryCount();

                HMEMCPY( pBI->bmiColors, pBuffer->maPalette.ImplGetColorBuffer(), nCount * sizeof( RGBQUAD ) );
                GlobalUnlock( mhDIB );
            }

            GlobalUnlock( mhDIB );
#endif
        }

        delete pBuffer;
    }
}

// ------------------------------------------------------------------

void SalBitmap::ImplDecodeRLEBuffer( const BYTE* pSrcBuf, BYTE* pDstBuf,
                                     const Size& rSizePixel, BOOL bRLE4 )
{
    HPBYTE          pRLE = (HPBYTE) pSrcBuf;
    HPBYTE          pDIB = (HPBYTE) pDstBuf;
    HPBYTE          pRow = (HPBYTE) pDstBuf;
    ULONG           nWidthAl = AlignedWidth4Bytes( rSizePixel.Width() * ( bRLE4 ? 4UL : 8UL ) );
    HPBYTE          pLast = pDIB + rSizePixel.Height() * nWidthAl - 1;
    ULONG           nCountByte;
    ULONG           nRunByte;
    ULONG           nX = 0;
    ULONG           i;
    BYTE            cTmp;
    BOOL            bEndDecoding = FALSE;

    if( pRLE && pDIB )
    {
        do
        {
            if( !( nCountByte = *pRLE++ ) )
            {
                nRunByte = *pRLE++;

                if( nRunByte > 2UL )
                {
                    if( bRLE4 )
                    {
                        nCountByte = nRunByte >> 1UL;

                        for( i = 0; i < nCountByte; i++ )
                        {
                            cTmp = *pRLE++;
                            ImplSetPixel4( pDIB, nX++, cTmp >> 4 );
                            ImplSetPixel4( pDIB, nX++, cTmp & 0x0f );
                        }

                        if( nRunByte & 1 )
                            ImplSetPixel4( pDIB, nX++, *pRLE++ >> 4 );

                        if( ( ( nRunByte + 1 ) >> 1 ) & 1 )
                            pRLE++;
                    }
                    else
                    {
                        HMEMCPY( &pDIB[ nX ], pRLE, nRunByte );
                        pRLE += nRunByte;
                        nX += nRunByte;

                        if( nRunByte & 1 )
                            pRLE++;
                    }
                }
                else if( !nRunByte )
                {
                    pDIB = ( pRow += nWidthAl );
                    nX = 0UL;
                }
                else if( nRunByte == 1 )
                    bEndDecoding = TRUE;
                else
                {
                    nX += *pRLE++;
                    pDIB = ( pRow += ( *pRLE++ ) * nWidthAl );
                }
            }
            else
            {
                cTmp = *pRLE++;

                if( bRLE4 )
                {
                    nRunByte = nCountByte >> 1;

                    for( i = 0; i < nRunByte; i++ )
                    {
                        ImplSetPixel4( pDIB, nX++, cTmp >> 4 );
                        ImplSetPixel4( pDIB, nX++, cTmp & 0x0f );
                    }

                    if( nCountByte & 1 )
                        ImplSetPixel4( pDIB, nX++, cTmp >> 4 );
                }
                else
                {
                    for( i = 0; i < nCountByte; i++ )
                        pDIB[ nX++ ] = cTmp;
                }
            }
        }
        while( !bEndDecoding && ( pDIB <= pLast ) );
    }
}
