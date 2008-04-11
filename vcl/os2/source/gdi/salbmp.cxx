/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salbmp.cxx,v $
 * $Revision: 1.7 $
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

#include <svpm.h>

#define _SV_SALBMP_CXX
#include <rtl/alloc.h>
#include <vcl/salbtype.hxx>
#include <salgdi.h>
#include <saldata.hxx>
#include <salbmp.h>
#include <vcl/bitmap.hxx> // for BitmapSystemData
#include <string.h>

#ifndef __H_FT2LIB
#include <wingdi.h>
#include <ft2lib.h>
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
// - Os2SalBitmap -
// -------------

Os2SalBitmap::Os2SalBitmap() :
        mhDIB       ( 0 ),
        mhDIB1Subst ( 0 ),
        mhDDB       ( 0 ),
        mnBitCount  ( 0 )
{
}

// ------------------------------------------------------------------

Os2SalBitmap::~Os2SalBitmap()
{
    Destroy();
}

// ------------------------------------------------------------------

bool Os2SalBitmap::Create( HANDLE hBitmap, bool bDIB, bool bCopyHandle )
{
    BOOL bRet = TRUE;

    if( bDIB )
        mhDIB = (HANDLE) ( bCopyHandle ? ImplCopyDIBOrDDB( hBitmap, TRUE ) : hBitmap );
    else
        mhDDB = (HBITMAP) ( bCopyHandle ? ImplCopyDIBOrDDB( hBitmap, FALSE ) : hBitmap );

    if( mhDIB )
    {
        // bitmap-header is the beginning of memory block
        PBITMAPINFOHEADER2 pBIH = (PBITMAPINFOHEADER2) mhDIB;

        maSize = Size( pBIH->cx, pBIH->cy );
        mnBitCount = pBIH->cBitCount;

        if( mnBitCount )
            mnBitCount = ( mnBitCount <= 1 ) ? 1 : ( mnBitCount <= 4 ) ? 4 : ( mnBitCount <= 8 ) ? 8 : 24;
    }
    else if( mhDDB )
    {
        BITMAPINFOHEADER2 aDDBInfoHeader;

        aDDBInfoHeader.cbFix = sizeof( aDDBInfoHeader );

        if( GpiQueryBitmapInfoHeader( mhDDB, &aDDBInfoHeader ) )
        {
            maSize = Size( aDDBInfoHeader.cx, aDDBInfoHeader.cy );
            mnBitCount = aDDBInfoHeader.cPlanes * aDDBInfoHeader.cBitCount;

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

bool Os2SalBitmap::Create( const Size& rSize, USHORT nBitCount, const BitmapPalette& rPal )
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

bool Os2SalBitmap::Create( const SalBitmap& rSSalBitmap )
{
    bool bRet = FALSE;
    const Os2SalBitmap& rSalBitmap = static_cast<const Os2SalBitmap&>(rSSalBitmap);

    if ( rSalBitmap.mhDIB || rSalBitmap.mhDDB )
    {
        HANDLE hNewHdl = ImplCopyDIBOrDDB( rSalBitmap.mhDIB ? rSalBitmap.mhDIB : rSalBitmap.mhDDB,
                                           rSalBitmap.mhDIB != 0 );

        if( hNewHdl )
        {
            if( rSalBitmap.mhDIB )
                mhDIB = (HANDLE) hNewHdl;
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

bool Os2SalBitmap::Create( const SalBitmap& rSSalBmp, SalGraphics* pSGraphics )
{
    bool bRet = FALSE;
    const Os2SalBitmap& rSalBmp = static_cast<const Os2SalBitmap&>(rSSalBmp);
    Os2SalGraphics* pGraphics = static_cast<Os2SalGraphics*>(pSGraphics);

    if( rSalBmp.mhDIB )
    {
        HPS                     hPS = pGraphics->mhPS;
        HBITMAP                 hNewDDB;
        BITMAPINFOHEADER2       aInfoHeader;
        const Size              aSize( rSalBmp.GetSize() );
        long                    nFormat[ 2 ];

        memset( &aInfoHeader, 0, sizeof( aInfoHeader ) );
        aInfoHeader.cbFix = 16;
        aInfoHeader.cx = aSize.Width();
        aInfoHeader.cy = aSize.Height();

        GpiQueryDeviceBitmapFormats( hPS, 2L, (PLONG) &nFormat );
        aInfoHeader.cPlanes = nFormat[ 0 ];
        aInfoHeader.cBitCount = nFormat[ 1 ];

        // ! wegen Postscript-Treiber
        if( !aInfoHeader.cBitCount )
            aInfoHeader.cBitCount = 24;
        else if( ( aInfoHeader.cPlanes == 1 ) && ( aInfoHeader.cBitCount == 1 ) )
            aInfoHeader.cBitCount = 4;

        // BitCount == 1 ist wegen aller moeglichen Treiberfehler nicht moeglich
        if( rSalBmp.GetBitCount() == 1 )
        {
            HANDLE  hTmp = ImplCreateDIB4FromDIB1( rSalBmp.mhDIB );
            PBYTE   pBits = (PBYTE) hTmp + *(ULONG*) hTmp + ImplGetDIBColorCount( hTmp ) * sizeof( RGB2 );

            hNewDDB = GpiCreateBitmap( hPS, &aInfoHeader, CBM_INIT, pBits, (PBITMAPINFO2) hTmp );
            rtl_freeMemory( (void*)hTmp );
        }
        else
        {
            PBYTE pBits = (PBYTE) rSalBmp.mhDIB + *(ULONG*) rSalBmp.mhDIB + ImplGetDIBColorCount( rSalBmp.mhDIB ) * sizeof( RGB2 );
            hNewDDB = GpiCreateBitmap( hPS, &aInfoHeader, CBM_INIT, pBits, (PBITMAPINFO2) rSalBmp.mhDIB );
        }

        aInfoHeader.cbFix = sizeof( aInfoHeader );

        if( hNewDDB && GpiQueryBitmapInfoHeader( hNewDDB, &aInfoHeader ) )
        {
            mhDDB = hNewDDB;
            maSize = Size( aInfoHeader.cx, aInfoHeader.cy );
            mnBitCount = aInfoHeader.cPlanes * aInfoHeader.cBitCount;

            if( mnBitCount )
            {
                mnBitCount = ( mnBitCount <= 1 ) ? 1 :
                             ( mnBitCount <= 4 ) ? 4 :
                             ( mnBitCount <= 8 ) ? 8 : 24;
            }

            bRet = TRUE;
        }
        else if( hNewDDB )
            GpiDeleteBitmap( hNewDDB );
    }

    return bRet;
}

// ------------------------------------------------------------------

bool Os2SalBitmap::Create( const SalBitmap& rSSalBmp, USHORT nNewBitCount )
{
    bool bRet = FALSE;
    const Os2SalBitmap& rSalBmp = static_cast<const Os2SalBitmap&>(rSSalBmp);

    if( rSalBmp.mhDDB )
    {
        mhDIB = ImplCreateDIB( rSalBmp.maSize, nNewBitCount, BitmapPalette() );

        if( mhDIB )
        {
            // bitmap-header is the beginning of memory block
            PBITMAPINFO2    pBI = (PBITMAPINFO2) mhDIB;
            const int       nLines = (int) rSalBmp.maSize.Height();
            PBYTE           pBits = (PBYTE) pBI + *(ULONG*) pBI + ImplGetDIBColorCount( mhDIB ) * sizeof( RGB2 );
            SIZEL           aSizeL = { rSalBmp.maSize.Width(), nLines };
            HAB             hAB = GetSalData()->mhAB;
            DEVOPENSTRUC    aDevOpenStruc = { NULL, (PSZ)"DISPLAY", NULL, NULL, NULL, NULL, NULL, NULL, NULL };
            HDC             hMemDC = DevOpenDC( hAB, OD_MEMORY, (PSZ)"*", 5L, (PDEVOPENDATA)&aDevOpenStruc, 0 );
            HPS             hMemPS = Ft2CreatePS( hAB, hMemDC, &aSizeL, GPIT_MICRO | GPIA_ASSOC | PU_PELS );
            HBITMAP         hMemOld = (HBITMAP) Ft2SetBitmap( hMemPS, rSalBmp.mhDDB );

            if( GpiQueryBitmapBits( hMemPS, 0, nLines, pBits, pBI ) == nLines )
            {
                maSize = rSalBmp.maSize;
                mnBitCount = nNewBitCount;
                bRet = TRUE;
            }
            else
            {
                rtl_freeMemory( (void*)mhDIB );
                mhDIB = 0;
            }

            Ft2SetBitmap( hMemPS, hMemOld );
            Ft2DestroyPS( hMemPS );
            DevCloseDC( hMemDC );
        }
    }

    return bRet;
}

// ------------------------------------------------------------------

void Os2SalBitmap::Destroy()
{
    if( mhDIB )
        rtl_freeMemory( (void*)mhDIB );
    else if( mhDDB )
        GpiDeleteBitmap( mhDDB );

    if( mhDIB1Subst )
    {
        rtl_freeMemory( (void*)mhDIB1Subst );
        mhDIB1Subst = NULL;
    }

    maSize = Size();
    mnBitCount = 0;
}

// ------------------------------------------------------------------

void Os2SalBitmap::ImplReplacehDIB1Subst( HANDLE hDIB1Subst )
{
    if( mhDIB1Subst )
        rtl_freeMemory( (void*)mhDIB1Subst );

    mhDIB1Subst = hDIB1Subst;
}

// ------------------------------------------------------------------

USHORT Os2SalBitmap::ImplGetDIBColorCount( HANDLE hDIB )
{
    USHORT nColors = 0;

    if( hDIB )
    {
        // bitmap infos can be found at the beginning of the memory
        PBITMAPINFOHEADER2 pBIH = (PBITMAPINFOHEADER2) hDIB;

        if( pBIH->cBitCount <= 8 )
        {
            if( pBIH->cclrUsed )
                nColors = (USHORT) pBIH->cclrUsed;
            else
                nColors = 1 << pBIH->cBitCount;
        }
    }

    return nColors;
}

// ------------------------------------------------------------------

HANDLE Os2SalBitmap::ImplCreateDIB( const Size& rSize, USHORT nBits, const BitmapPalette& rPal )
{
    DBG_ASSERT( nBits == 1 || nBits == 4 || nBits == 8 || nBits == 24, "Unsupported BitCount!" );

    HANDLE hDIB = 0;

    if ( rSize.Width() && rSize.Height() && ( nBits == 1 || nBits == 4 || nBits == 8 || nBits == 24 ) )
    {
        const ULONG     nImageSize = AlignedWidth4Bytes( nBits * rSize.Width() ) * rSize.Height();
        const USHORT    nColors = ( nBits <= 8 ) ? ( 1 << nBits ) : 0;

        hDIB = (HANDLE) rtl_allocateZeroMemory( sizeof( BITMAPINFOHEADER2 ) + nColors * sizeof( RGB2 ) + nImageSize );

        if( hDIB )
        {
            // bitmap infos can be found at the beginning of the memory
            PBITMAPINFO2        pBI = (PBITMAPINFO2) hDIB;
            PBITMAPINFOHEADER2  pBIH = (PBITMAPINFOHEADER2) pBI;

            pBIH->cbFix = sizeof( BITMAPINFOHEADER2 );
            pBIH->cx = rSize.Width();
            pBIH->cy = rSize.Height();
            pBIH->cPlanes = 1;
            pBIH->cBitCount = nBits;
            pBIH->ulCompression = BCA_UNCOMP; // BI_RGB;
            pBIH->cbImage = nImageSize;
            pBIH->cxResolution = 0;
            pBIH->cyResolution = 0;
            pBIH->cclrUsed = 0;
            pBIH->cclrImportant = 0;

            // Rest auf 0 setzen
            memset( (PBYTE) &pBIH->usUnits, 0, (PBYTE) pBI->argbColor - (PBYTE) &pBIH->usUnits );

            if( nColors )
            {
                const USHORT nMinCount = Min( nColors, rPal.GetEntryCount() );

                if( nMinCount )
                    memcpy( pBI->argbColor, rPal.ImplGetColorBuffer(), nMinCount * sizeof( RGB2 ) );
            }
        }
    }

    return hDIB;
}

// ------------------------------------------------------------------

HANDLE Os2SalBitmap::ImplCreateDIB4FromDIB1( HANDLE hDIB1 )
{
    PBITMAPINFO2        pBI = (PBITMAPINFO2) hDIB1;
    PBITMAPINFOHEADER2  pBIH = (PBITMAPINFOHEADER2) pBI;
    PBYTE               pBits = (PBYTE) pBI + *(ULONG*) pBIH + Os2SalBitmap::ImplGetDIBColorCount( hDIB1 ) * sizeof( RGB2 );
    ULONG               nWidth = pBIH->cx, nHeight = pBIH->cy;
    ULONG               nAligned = AlignedWidth4Bytes( nWidth );
    ULONG               nAligned4 = AlignedWidth4Bytes( nWidth << 2 );
    ULONG               nSize4 = sizeof( BITMAPINFOHEADER2 ) + ( sizeof( RGB2 ) << 4 ) + nAligned4 * nHeight;
    PBYTE               pDIB4 = (PBYTE) rtl_allocateZeroMemory( nSize4 );
    PBITMAPINFO2        pBI4 = (PBITMAPINFO2) pDIB4;
    PBITMAPINFOHEADER2  pBIH4 = (PBITMAPINFOHEADER2) pBI4;
    BYTE                aMap[ 4 ] = { 0x00, 0x01, 0x10, 0x11 };

    memset( pBIH4, 0, sizeof( BITMAPINFOHEADER2 ) );
    pBIH4->cbFix = sizeof( BITMAPINFOHEADER2 );
    pBIH4->cx = nWidth;
    pBIH4->cy = nHeight;
    pBIH4->cPlanes = 1;
    pBIH4->cBitCount = 4;

    // die ersten beiden Eintraege der 1Bit-Farbtabelle kopieren
    memcpy( pBI4->argbColor, pBI->argbColor, sizeof( RGB2 ) << 1 );

    PBYTE pBits4 = (PBYTE) pBI4 + *(ULONG*) pBIH4 + ( sizeof( RGB2 ) << 4 );

    // 4Bit-DIB-Bilddaten setzen
    for( ULONG nY = 0UL; nY < nHeight; nY++ )
    {
        PBYTE pTmp = pBits; pBits += nAligned;
        PBYTE pTmp4 = pBits4; pBits4 += nAligned4;

        for( ULONG nX = 0UL; nX < nWidth; nX += 8UL )
        {
            *pTmp4++ = aMap[ ( *pTmp >> 6 ) & 3 ];
            *pTmp4++ = aMap[ ( *pTmp >> 4 ) & 3 ];
            *pTmp4++ = aMap[ ( *pTmp >> 2 ) & 3 ];
            *pTmp4++ = aMap[ *pTmp++ & 3 ];
        }
    }

    return (HANDLE) pDIB4;
}

// ------------------------------------------------------------------

HANDLE Os2SalBitmap::ImplCopyDIBOrDDB( HANDLE hHdl, BOOL bDIB )
{
    HANDLE hCopy = 0;

    if( bDIB && hHdl )
    {
        PBITMAPINFOHEADER2 pBIH = (PBITMAPINFOHEADER2) hHdl;
           const ULONG nSize = sizeof( BITMAPINFOHEADER2 )
                            + ImplGetDIBColorCount( hHdl ) * sizeof( RGB2 ) +
                            ( pBIH->cbImage ? pBIH->cbImage : AlignedWidth4Bytes( pBIH->cx * pBIH->cBitCount ) );

        BYTE* pCopy = (BYTE*)rtl_allocateZeroMemory( nSize );
        memcpy( pCopy, (BYTE*) hHdl, nSize );
        hCopy = (HANDLE) pCopy;
    }
    else if( hHdl )
    {
        HAB                 hAB = GetSalData()->mhAB;
        HDC                 hSrcMemDC;
        HDC                 hDestMemDC;
        HPS                 hSrcMemPS;
        HPS                 hDestMemPS;
        HBITMAP             hCopyBitmap;
        BITMAPINFOHEADER2   aInfoHeader;
        DEVOPENSTRUC        aDevOpenStruc;
        SIZEL               size;

        aInfoHeader.cbFix = sizeof( BITMAPINFOHEADER2 );
        GpiQueryBitmapInfoHeader( hHdl, &aInfoHeader );
        size.cx = aInfoHeader.cx;
        size.cy = aInfoHeader.cy;

        // Memory DCs erzeugen
        aDevOpenStruc.pszLogAddress = 0;
        aDevOpenStruc.pszDriverName = (PSZ)"DISPLAY";

        hSrcMemDC = DevOpenDC( hAB, OD_MEMORY, (PSZ)"*", 2, (PDEVOPENDATA)&aDevOpenStruc, 0 );
        hDestMemDC = DevOpenDC( hAB, OD_MEMORY, (PSZ)"*", 2, (PDEVOPENDATA)&aDevOpenStruc, 0 );

        // Memory PSs erzeugen
        hSrcMemPS = Ft2CreatePS( hAB, hSrcMemDC, &size, GPIT_MICRO | GPIA_ASSOC | PU_PELS );
        hDestMemPS = Ft2CreatePS( hAB, hDestMemDC, &size, GPIT_MICRO | GPIA_ASSOC | PU_PELS );

        Ft2SetBitmap( hSrcMemPS, hHdl );

        if( !hHdl )
        {
            memset( &aInfoHeader, 0, sizeof( BITMAPINFOHEADER2 ) );
            aInfoHeader.cbFix     = sizeof( BITMAPINFOHEADER2 );
            aInfoHeader.cx        = 0;
            aInfoHeader.cy        = 0;
            aInfoHeader.cPlanes   = 1;
            aInfoHeader.cBitCount = 1;
        }

        hCopy = GpiCreateBitmap( hDestMemPS, &aInfoHeader, 0, NULL, NULL );
        Ft2SetBitmap( hDestMemPS, hCopy );

        POINTL pts[3];

        pts[0].x = 0;
        pts[0].y = 0;
        pts[1].x = size.cx;
        pts[1].y = size.cy;
        pts[2].x = 0;
        pts[2].y = 0;

        GpiBitBlt( hDestMemPS, hSrcMemPS, 3, pts, ROP_SRCCOPY, BBO_IGNORE );

        Ft2SetBitmap( hSrcMemPS, (HBITMAP)0L);
        Ft2SetBitmap( hDestMemPS, (HBITMAP)0L);
        Ft2Associate( hSrcMemPS, NULLHANDLE );
        Ft2Associate( hDestMemPS, NULLHANDLE );
        Ft2DestroyPS( hSrcMemPS );
        Ft2DestroyPS( hDestMemPS );
        DevCloseDC( hSrcMemDC );
        DevCloseDC( hDestMemDC );
    }

    return hCopy;
}

// ------------------------------------------------------------------

BitmapBuffer* Os2SalBitmap::AcquireBuffer( bool bReadOnly )
{
    BitmapBuffer* pBuffer = NULL;

    if( mhDIB )
    {
        // bitmap infos can be found at the beginning of the memory
        PBITMAPINFO2        pBI = (PBITMAPINFO2) mhDIB;
        PBITMAPINFOHEADER2    pBIH = (PBITMAPINFOHEADER2) pBI;

        if( ( pBIH->ulCompression == BCA_RLE4 ) || ( pBIH->ulCompression == BCA_RLE8 ) )
        {
            Size    aSizePix( pBIH->cx, pBIH->cy );
            HANDLE    hNewDIB = ImplCreateDIB( aSizePix, pBIH->cBitCount, BitmapPalette() );

            if( hNewDIB )
            {
                // bitmap infos can be found at the beginning of the memory
                PBITMAPINFO2        pNewBI = (PBITMAPINFO2) hNewDIB;
                PBITMAPINFOHEADER2    pNewBIH = (PBITMAPINFOHEADER2) pNewBI;
                const USHORT        nColorCount = ImplGetDIBColorCount( hNewDIB );
                const ULONG            nOffset = *(ULONG*) pBI + nColorCount * sizeof( RGB2 );
                BYTE*                pOldBits = (BYTE*) pBI + nOffset;
                BYTE*                pNewBits = (BYTE*) pNewBI + nOffset;

                memcpy( pNewBI, pBI, nOffset );
                pNewBIH->ulCompression = 0;
                ImplDecodeRLEBuffer( pOldBits, pNewBits, aSizePix, pBIH->ulCompression == BCA_RLE4 );

                rtl_freeMemory( (void*)mhDIB );

                mhDIB = hNewDIB;
                pBI = pNewBI;
                pBIH = pNewBIH;
            }
        }

        if( pBIH->cPlanes == 1 )
        {
            pBuffer = new BitmapBuffer;

            pBuffer->mnFormat = BMP_FORMAT_BOTTOM_UP |
                                ( pBIH->cBitCount == 1 ? BMP_FORMAT_1BIT_MSB_PAL :
                                  pBIH->cBitCount == 4 ? BMP_FORMAT_4BIT_MSN_PAL :
                                  pBIH->cBitCount == 8 ? BMP_FORMAT_8BIT_PAL :
                                  pBIH->cBitCount == 16 ? BMP_FORMAT_16BIT_TC_LSB_MASK :
                                  pBIH->cBitCount == 24 ? BMP_FORMAT_24BIT_TC_BGR :
                                  pBIH->cBitCount == 32 ? BMP_FORMAT_32BIT_TC_MASK : 0UL );

            if( BMP_SCANLINE_FORMAT( pBuffer->mnFormat ) )
            {
                pBuffer->mnWidth = maSize.Width();
                pBuffer->mnHeight = maSize.Height();
                pBuffer->mnScanlineSize = AlignedWidth4Bytes( maSize.Width() * pBIH->cBitCount );
                pBuffer->mnBitCount = (USHORT) pBIH->cBitCount;

                if( pBuffer->mnBitCount <= 8 )
                {
                    const USHORT nPalCount = ImplGetDIBColorCount( mhDIB );

                    pBuffer->maPalette.SetEntryCount( nPalCount );

                    if( nPalCount )
                        memcpy( pBuffer->maPalette.ImplGetColorBuffer(), pBI->argbColor, nPalCount * sizeof( RGB2 ) );

                    pBuffer->mpBits = (BYTE*) pBI + *(ULONG*) pBI + nPalCount * sizeof( RGB2 );
                }
                else
                    pBuffer->mpBits = (BYTE*) pBI + *(ULONG*) pBI;
            }
            else
            {
                delete pBuffer;
                pBuffer = NULL;
            }
        }
    }

    if( pBuffer && mhDIB1Subst )
    {
        rtl_freeMemory( (void*)mhDIB1Subst );
        mhDIB1Subst = 0;
     }

    return pBuffer;
}

// ------------------------------------------------------------------

void Os2SalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly )
{
    if( pBuffer )
    {
        if( mhDIB )
        {
            if( !bReadOnly && !!pBuffer->maPalette )
            {
                // bitmap infos can be found at the beginning of the memory
                PBITMAPINFO2    pBI = (PBITMAPINFO2) mhDIB;
                const USHORT    nCount = pBuffer->maPalette.GetEntryCount();

                if( nCount )
                    memcpy( pBI->argbColor, pBuffer->maPalette.ImplGetColorBuffer(), nCount * sizeof( RGB2 ) );
            }
        }

        delete pBuffer;
    }
}

// ------------------------------------------------------------------

void Os2SalBitmap::ImplDecodeRLEBuffer( const BYTE* pSrcBuf, BYTE* pDstBuf,
                                     const Size& rSizePixel, BOOL bRLE4 )
{
    HPBYTE    pRLE = (HPBYTE) pSrcBuf;
    HPBYTE    pDIB = (HPBYTE) pDstBuf;
    HPBYTE    pRow = (HPBYTE) pDstBuf;
    ULONG    nWidthAl = AlignedWidth4Bytes( rSizePixel.Width() * ( bRLE4 ? 4UL : 8UL ) );
    HPBYTE    pLast = pDIB + rSizePixel.Height() * nWidthAl - 1;
    ULONG    nCountByte;
    ULONG    nRunByte;
    ULONG    nX = 0;
    ULONG    i;
    BYTE    cTmp;
    BOOL    bEndDecoding = FALSE;

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

bool Os2SalBitmap::GetSystemData( BitmapSystemData& rData )
{
    bool bRet = false;
    if( mhDIB || mhDDB )
    {
        bRet = true;
        rData.pDIB = (void*)mhDIB;
        rData.pDDB = (void*)mhDDB;
    }
    return bRet;
}
