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


#include <svsys.h>

#include <vcl/bitmap.hxx> // for BitmapSystemData
#include <vcl/salbtype.hxx>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <win/wincomp.hxx>
#include <win/salgdi.h>
#include <win/saldata.hxx>
#include <win/salbmp.h>

#include <string.h>

// -----------
// - Inlines -
// -----------

inline void ImplSetPixel4( const HPBYTE pScanline, long nX, const BYTE cIndex )
{
    BYTE& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0xf0, rByte |= ( cIndex & 0x0f ) ) :
                 ( rByte &= 0x0f, rByte |= ( cIndex << 4 ) );
}

// ----------------
// - WinSalBitmap -
// ----------------

WinSalBitmap::WinSalBitmap() :
        mhDIB       ( 0 ),
        mhDDB       ( 0 ),
        mnBitCount  ( 0 )
{
}

// ------------------------------------------------------------------

WinSalBitmap::~WinSalBitmap()
{
    Destroy();
}

// ------------------------------------------------------------------

bool WinSalBitmap::Create( HANDLE hBitmap, bool bDIB, bool bCopyHandle )
{
    bool bRet = TRUE;

    if( bDIB )
        mhDIB = (HGLOBAL) ( bCopyHandle ? ImplCopyDIBOrDDB( hBitmap, TRUE ) : hBitmap );
    else
        mhDDB = (HBITMAP) ( bCopyHandle ? ImplCopyDIBOrDDB( hBitmap, FALSE ) : hBitmap );

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
        bRet = FALSE;

    return bRet;
}

// ------------------------------------------------------------------

bool WinSalBitmap::Create( const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal )
{
    bool bRet = FALSE;

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

bool WinSalBitmap::Create( const SalBitmap& rSSalBitmap )
{
    bool bRet = FALSE;
    const WinSalBitmap& rSalBitmap = static_cast<const WinSalBitmap&>(rSSalBitmap);

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

bool WinSalBitmap::Create( const SalBitmap& rSSalBmp, SalGraphics* pSGraphics )
{
    bool bRet = FALSE;

    const WinSalBitmap& rSalBmp = static_cast<const WinSalBitmap&>(rSSalBmp);
    WinSalGraphics* pGraphics = static_cast<WinSalGraphics*>(pSGraphics);

    if( rSalBmp.mhDIB )
    {
        PBITMAPINFO         pBI = (PBITMAPINFO) GlobalLock( rSalBmp.mhDIB );
        PBITMAPINFOHEADER   pBIH = (PBITMAPINFOHEADER) pBI;
        HDC                 hDC  = pGraphics->mhDC;
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

            bRet = TRUE;
        }
        else if( hNewDDB )
            DeleteObject( hNewDDB );
    }

    return bRet;
}

// ------------------------------------------------------------------

bool WinSalBitmap::Create( const SalBitmap& rSSalBmp, sal_uInt16 nNewBitCount )
{
    bool bRet = FALSE;

    const WinSalBitmap& rSalBmp = static_cast<const WinSalBitmap&>(rSSalBmp);

    if( rSalBmp.mhDDB )
    {
        mhDIB = ImplCreateDIB( rSalBmp.maSize, nNewBitCount, BitmapPalette() );

        if( mhDIB )
        {
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
        }
    }

    return bRet;
}

// ------------------------------------------------------------------

bool WinSalBitmap::Create( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapCanvas > xBitmapCanvas, Size& /*rSize*/, bool bMask )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet >
        xFastPropertySet( xBitmapCanvas, ::com::sun::star::uno::UNO_QUERY );

    if( xFastPropertySet.get() ) {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > args;

        if( xFastPropertySet->getFastPropertyValue(bMask ? 2 : 1) >>= args ) {
            sal_Int64 aHBmp64;

            if( args[0] >>= aHBmp64 ) {
                return Create( HBITMAP(aHBmp64), false, false );
            }
        }
    }
    return false;
}

// ------------------------------------------------------------------

void WinSalBitmap::Destroy()
{
    if( mhDIB )
        GlobalFree( mhDIB );
    else if( mhDDB )
        DeleteObject( mhDDB );

    maSize = Size();
    mnBitCount = 0;
}

// ------------------------------------------------------------------

sal_uInt16 WinSalBitmap::ImplGetDIBColorCount( HGLOBAL hDIB )
{
    sal_uInt16 nColors = 0;

    if( hDIB )
    {
        PBITMAPINFO         pBI = (PBITMAPINFO) GlobalLock( hDIB );
        PBITMAPINFOHEADER   pBIH = (PBITMAPINFOHEADER) pBI;

        if ( pBIH->biSize != sizeof( BITMAPCOREHEADER ) )
        {
            if( pBIH->biBitCount <= 8 )
            {
                if ( pBIH->biClrUsed )
                    nColors = (sal_uInt16) pBIH->biClrUsed;
                else
                    nColors = 1 << pBIH->biBitCount;
            }
        }
        else if( ( (PBITMAPCOREHEADER) pBI )->bcBitCount <= 8 )
            nColors = 1 << ( (PBITMAPCOREHEADER) pBI )->bcBitCount;

        GlobalUnlock( hDIB );
    }

    return nColors;
}

// ------------------------------------------------------------------

HGLOBAL WinSalBitmap::ImplCreateDIB( const Size& rSize, sal_uInt16 nBits, const BitmapPalette& rPal )
{
    DBG_ASSERT( nBits == 1 || nBits == 4 || nBits == 8 || nBits == 16 || nBits == 24, "Unsupported BitCount!" );

    HGLOBAL hDIB = 0;

    if( rSize.Width() <= 0 || rSize.Height() <= 0 )
        return hDIB;

    // calculate bitmap size in Bytes
    const sal_uLong nAlignedWidth4Bytes = AlignedWidth4Bytes( nBits * rSize.Width() );
    const sal_uLong nImageSize = nAlignedWidth4Bytes * rSize.Height();
    bool bOverflow = (nImageSize / nAlignedWidth4Bytes) != rSize.Height();
    if( bOverflow )
        return hDIB;

    // allocate bitmap memory including header and palette
    const sal_uInt16 nColors = (nBits <= 8) ? (1 << nBits) : 0;
    const sal_uLong nHeaderSize = sizeof( BITMAPINFOHEADER ) + nColors * sizeof( RGBQUAD );
    bOverflow = (nHeaderSize + nImageSize) < nImageSize;
    if( bOverflow )
        return hDIB;

    hDIB = GlobalAlloc( GHND, nHeaderSize + nImageSize );
    if( !hDIB )
        return hDIB;

    PBITMAPINFO pBI = static_cast<PBITMAPINFO>( GlobalLock( hDIB ) );
    PBITMAPINFOHEADER pBIH = reinterpret_cast<PBITMAPINFOHEADER>( pBI );

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

    if( nColors )
    {
        // copy the palette entries if any
        const sal_uInt16 nMinCount = std::min( nColors, rPal.GetEntryCount() );
        if( nMinCount )
            memcpy( pBI->bmiColors, rPal.ImplGetColorBuffer(), nMinCount * sizeof(RGBQUAD) );
    }

    GlobalUnlock( hDIB );

    return hDIB;
}

// ------------------------------------------------------------------

HANDLE WinSalBitmap::ImplCopyDIBOrDDB( HANDLE hHdl, bool bDIB )
{
    HANDLE  hCopy = 0;

    if ( bDIB && hHdl )
    {
        const sal_uLong nSize = GlobalSize( hHdl );

        if ( (hCopy = GlobalAlloc( GHND, nSize  )) != 0 )
        {
            memcpy( (LPSTR) GlobalLock( hCopy ), (LPSTR) GlobalLock( hHdl ), nSize );

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
        if ( (hCopy = CreateBitmapIndirect( &aBmp )) != 0 )
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

    return hCopy;
}

// ------------------------------------------------------------------

BitmapBuffer* WinSalBitmap::AcquireBuffer( bool /*bReadOnly*/ )
{
    BitmapBuffer* pBuffer = NULL;

    if( mhDIB )
    {
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
                const sal_uInt16        nColorCount = ImplGetDIBColorCount( hNewDIB );
                const sal_uLong         nOffset = *(DWORD*) pBI + nColorCount * sizeof( RGBQUAD );
                BYTE*               pOldBits = (PBYTE) pBI + nOffset;
                BYTE*               pNewBits = (PBYTE) pNewBI + nOffset;

                memcpy( pNewBI, pBI, nOffset );
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
                                  pBIH->biBitCount == 16 ? BMP_FORMAT_16BIT_TC_LSB_MASK :
                                  pBIH->biBitCount == 24 ? BMP_FORMAT_24BIT_TC_BGR :
                                  pBIH->biBitCount == 32 ? BMP_FORMAT_32BIT_TC_MASK : 0UL );

            if( BMP_SCANLINE_FORMAT( pBuffer->mnFormat ) )
            {
                pBuffer->mnWidth = maSize.Width();
                pBuffer->mnHeight = maSize.Height();
                pBuffer->mnScanlineSize = AlignedWidth4Bytes( maSize.Width() * pBIH->biBitCount );
                pBuffer->mnBitCount = (sal_uInt16) pBIH->biBitCount;

                if( pBuffer->mnBitCount <= 8 )
                {
                    const sal_uInt16 nPalCount = ImplGetDIBColorCount( mhDIB );

                    pBuffer->maPalette.SetEntryCount( nPalCount );
                    memcpy( pBuffer->maPalette.ImplGetColorBuffer(), pBI->bmiColors, nPalCount * sizeof( RGBQUAD ) );
                    pBuffer->mpBits = (PBYTE) pBI + *(DWORD*) pBI + nPalCount * sizeof( RGBQUAD );
                }
                else if( ( pBIH->biBitCount == 16 ) || ( pBIH->biBitCount == 32 ) )
                {
                    sal_uLong nOffset = 0UL;

                    if( pBIH->biCompression == BI_BITFIELDS )
                    {
                        nOffset = 3 * sizeof( RGBQUAD );
                        pBuffer->maColorMask = ColorMask( *(UINT32*) &pBI->bmiColors[ 0 ],
                                                          *(UINT32*) &pBI->bmiColors[ 1 ],
                                                          *(UINT32*) &pBI->bmiColors[ 2 ] );
                    }
                    else if( pBIH->biBitCount == 16 )
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
    }

    return pBuffer;
}

// ------------------------------------------------------------------

void WinSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly )
{
    if( pBuffer )
    {
        if( mhDIB )
        {
            if( !bReadOnly && !!pBuffer->maPalette )
            {
                PBITMAPINFO     pBI = (PBITMAPINFO) GlobalLock( mhDIB );
                const sal_uInt16    nCount = pBuffer->maPalette.GetEntryCount();
                const sal_uInt16    nDIBColorCount = ImplGetDIBColorCount( mhDIB );
                memcpy( pBI->bmiColors, pBuffer->maPalette.ImplGetColorBuffer(), std::min( nDIBColorCount, nCount ) * sizeof( RGBQUAD ) );
                GlobalUnlock( mhDIB );
            }

            GlobalUnlock( mhDIB );
        }

        delete pBuffer;
    }
}

// ------------------------------------------------------------------

void WinSalBitmap::ImplDecodeRLEBuffer( const BYTE* pSrcBuf, BYTE* pDstBuf,
                                     const Size& rSizePixel, bool bRLE4 )
{
    HPBYTE          pRLE = (HPBYTE) pSrcBuf;
    HPBYTE          pDIB = (HPBYTE) pDstBuf;
    HPBYTE          pRow = (HPBYTE) pDstBuf;
    sal_uLong           nWidthAl = AlignedWidth4Bytes( rSizePixel.Width() * ( bRLE4 ? 4UL : 8UL ) );
    HPBYTE          pLast = pDIB + rSizePixel.Height() * nWidthAl - 1;
    sal_uLong           nCountByte;
    sal_uLong           nRunByte;
    sal_uLong           i;
    BYTE            cTmp;
    bool            bEndDecoding = FALSE;

    if( pRLE && pDIB )
    {
        sal_uLong nX = 0;
        do
        {
            if( ( nCountByte = *pRLE++ ) == 0 )
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
                        memcpy( &pDIB[ nX ], pRLE, nRunByte );
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

bool WinSalBitmap::GetSystemData( BitmapSystemData& rData )
{
    bool bRet = false;
    if( mhDIB || mhDDB )
    {
        bRet = true;
        rData.pDIB = mhDIB;
        rData.pDDB = mhDDB;
        const Size& rSize = GetSize ();
        rData.mnWidth = rSize.Width();
        rData.mnHeight = rSize.Height();
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
