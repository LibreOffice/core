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
#include <vcl/timer.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <map>

#if defined _MSC_VER
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif

#ifdef __MINGW32__
#ifdef GetObject
#undef GetObject
#endif
#define GetObject GetObjectA
#endif

#include <gdiplus.h>

#ifdef __MINGW32__
#ifdef GetObject
#undef GetObject
#endif
#endif

#if defined _MSC_VER
#pragma warning(pop)
#endif


// ------------------------------------------------------------------
// - Inlines -

inline void ImplSetPixel4( const HPBYTE pScanline, long nX, const BYTE cIndex )
{
    BYTE& rByte = pScanline[ nX >> 1 ];

    ( nX & 1 ) ? ( rByte &= 0xf0, rByte |= ( cIndex & 0x0f ) ) :
                 ( rByte &= 0x0f, rByte |= ( cIndex << 4 ) );
}

// ------------------------------------------------------------------
// Helper class to manage Gdiplus::Bitmap instances inside of
// WinSalBitmap

struct Comparator
{
    bool operator()(WinSalBitmap* pA, WinSalBitmap* pB) const
    {
        return pA < pB;
    }
};

typedef ::std::map< WinSalBitmap*, sal_uInt32, Comparator > EntryMap;
static const sal_uInt32 nDefaultCycles(60);

class GdiPlusBuffer : protected comphelper::OBaseMutex, public Timer
{
private:
    EntryMap        maEntries;

public:
    GdiPlusBuffer()
    :   Timer(),
        maEntries()
    {
        SetTimeout(1000);
        Stop();
    }

    ~GdiPlusBuffer()
    {
        Stop();
    }

    void addEntry(WinSalBitmap& rEntry)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        EntryMap::iterator aFound = maEntries.find(&rEntry);

        if(aFound == maEntries.end())
        {
            if(maEntries.empty())
            {
                Start();
            }

            maEntries[&rEntry] = nDefaultCycles;
        }
    }

    void remEntry(WinSalBitmap& rEntry)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        EntryMap::iterator aFound = maEntries.find(&rEntry);

        if(aFound != maEntries.end())
        {
            maEntries.erase(aFound);

            if(maEntries.empty())
            {
                Stop();
            }
        }
    }

    void touchEntry(WinSalBitmap& rEntry)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        EntryMap::iterator aFound = maEntries.find(&rEntry);

        if(aFound != maEntries.end())
        {
            aFound->second = nDefaultCycles;
        }
    }

    // from parent Timer
    virtual void Timeout()
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        EntryMap::iterator aIter(maEntries.begin());

        while(aIter != maEntries.end())
        {
            if(aIter->second)
            {
                aIter->second--;
                ++aIter;
            }
            else
            {
                EntryMap::iterator aDelete(aIter);
                WinSalBitmap* pSource = aDelete->first;
                ++aIter;
                maEntries.erase(aDelete);

                if(maEntries.empty())
                {
                    Stop();
                }

                // delete at WinSalBitmap after entry is removed; this
                // way it would not hurt to call remEntry from there, too
                if(pSource->maGdiPlusBitmap.get())
                {
                    pSource->maGdiPlusBitmap.reset();
                    pSource->mpAssociatedAlpha = 0;
                }
            }
        }

        if(!maEntries.empty())
        {
            Start();
        }
    }
};

// ------------------------------------------------------------------
// Global instance of GdiPlusBuffer which manages Gdiplus::Bitmap
// instances

static GdiPlusBuffer aGdiPlusBuffer;

// ------------------------------------------------------------------
// - WinSalBitmap -

WinSalBitmap::WinSalBitmap()
:   maSize(),
    mhDIB(0),
    mhDDB(0),
    maGdiPlusBitmap(),
    mpAssociatedAlpha(0),
    mnBitCount(0)
{
}

// ------------------------------------------------------------------

WinSalBitmap::~WinSalBitmap()
{
    Destroy();
}

// ------------------------------------------------------------------

void WinSalBitmap::Destroy()
{
    if(maGdiPlusBitmap.get())
    {
        aGdiPlusBuffer.remEntry(*this);
    }

    if( mhDIB )
        GlobalFree( mhDIB );
    else if( mhDDB )
        DeleteObject( mhDDB );

    maSize = Size();
    mnBitCount = 0;
}

// ------------------------------------------------------------------

GdiPlusBmpPtr WinSalBitmap::ImplGetGdiPlusBitmap(const WinSalBitmap* pAlphaSource) const
{
    WinSalBitmap* pThat = const_cast< WinSalBitmap* >(this);

    if(maGdiPlusBitmap.get() && pAlphaSource != mpAssociatedAlpha)
    {
        // #122350# if associated alpha with which the GDIPlus was constructed has changed
        // it is necessary to remove it from buffer, reset reference to it and reconstruct
        pThat->maGdiPlusBitmap.reset();
        aGdiPlusBuffer.remEntry(const_cast< WinSalBitmap& >(*this));
    }

    if(maGdiPlusBitmap.get())
    {
        aGdiPlusBuffer.touchEntry(const_cast< WinSalBitmap& >(*this));
    }
    else
    {
        if(maSize.Width() > 0 && maSize.Height() > 0)
        {
            if(pAlphaSource)
            {
                pThat->maGdiPlusBitmap.reset(pThat->ImplCreateGdiPlusBitmap(*pAlphaSource));
                pThat->mpAssociatedAlpha = pAlphaSource;
            }
            else
            {
                pThat->maGdiPlusBitmap.reset(pThat->ImplCreateGdiPlusBitmap());
                pThat->mpAssociatedAlpha = 0;
            }

            if(maGdiPlusBitmap.get())
            {
                aGdiPlusBuffer.addEntry(*pThat);
            }
        }
    }

    return maGdiPlusBitmap;
}

// ------------------------------------------------------------------

Gdiplus::Bitmap* WinSalBitmap::ImplCreateGdiPlusBitmap()
{
    Gdiplus::Bitmap* pRetval(0);
    WinSalBitmap* pSalRGB = const_cast< WinSalBitmap* >(this);
    WinSalBitmap* pExtraWinSalRGB = 0;

    if(!pSalRGB->ImplGethDIB())
    {
        // we need DIB for success with AcquireBuffer, create a replacement WinSalBitmap
        pExtraWinSalRGB = new WinSalBitmap();
        pExtraWinSalRGB->Create(*pSalRGB, pSalRGB->GetBitCount());
        pSalRGB = pExtraWinSalRGB;
    }

    BitmapBuffer* pRGB = pSalRGB->AcquireBuffer(true);
    BitmapBuffer* pExtraRGB = 0;

    if(pRGB && BMP_FORMAT_24BIT_TC_BGR != (pRGB->mnFormat & ~BMP_FORMAT_TOP_DOWN))
    {
        // convert source bitmap to BMP_FORMAT_24BIT_TC_BGR format if not yet in that format
        SalTwoRect aSalTwoRect;

        aSalTwoRect.mnSrcX = aSalTwoRect.mnSrcY = aSalTwoRect.mnDestX = aSalTwoRect.mnDestY = 0;
        aSalTwoRect.mnSrcWidth = aSalTwoRect.mnDestWidth = pRGB->mnWidth;
        aSalTwoRect.mnSrcHeight = aSalTwoRect.mnDestHeight = pRGB->mnHeight;

        pExtraRGB = StretchAndConvert(
            *pRGB,
            aSalTwoRect,
            BMP_FORMAT_24BIT_TC_BGR,
            0);

        pSalRGB->ReleaseBuffer(pRGB, true);
        pRGB = pExtraRGB;
    }

    if(pRGB
        && pRGB->mnWidth > 0
        && pRGB->mnHeight > 0
        && BMP_FORMAT_24BIT_TC_BGR == (pRGB->mnFormat & ~BMP_FORMAT_TOP_DOWN))
    {
        const sal_uInt32 nW(pRGB->mnWidth);
        const sal_uInt32 nH(pRGB->mnHeight);

        pRetval = new Gdiplus::Bitmap(nW, nH, PixelFormat24bppRGB);

        if(pRetval)
        {
            sal_uInt8* pSrcRGB(pRGB->mpBits);
            const sal_uInt32 nExtraRGB(pRGB->mnScanlineSize - (nW * 3));
            const bool bTopDown(pRGB->mnFormat & BMP_FORMAT_TOP_DOWN);
            const Gdiplus::Rect aAllRect(0, 0, nW, nH);
            Gdiplus::BitmapData aGdiPlusBitmapData;
            pRetval->LockBits(&aAllRect, Gdiplus::ImageLockModeWrite, PixelFormat24bppRGB, &aGdiPlusBitmapData);

            // copy data to Gdiplus::Bitmap; format is BGR here in both cases, so memcpy is possible
            for(sal_uInt32 y(0); y < nH; y++)
            {
                const sal_uInt32 nYInsert(bTopDown ? y : nH - y - 1);
                sal_uInt8* targetPixels = (sal_uInt8*)aGdiPlusBitmapData.Scan0 + (nYInsert * aGdiPlusBitmapData.Stride);

                memcpy(targetPixels, pSrcRGB, nW * 3);
                pSrcRGB += nW * 3 + nExtraRGB;
            }

            pRetval->UnlockBits(&aGdiPlusBitmapData);
        }
    }

    if(pExtraRGB)
    {
        delete pExtraRGB;
    }
    else
    {
        pSalRGB->ReleaseBuffer(pRGB, true);
    }

    if(pExtraWinSalRGB)
    {
        delete pExtraWinSalRGB;
    }

    return pRetval;
}

// ------------------------------------------------------------------

Gdiplus::Bitmap* WinSalBitmap::ImplCreateGdiPlusBitmap(const WinSalBitmap& rAlphaSource)
{
    Gdiplus::Bitmap* pRetval(0);
    WinSalBitmap* pSalRGB = const_cast< WinSalBitmap* >(this);
    WinSalBitmap* pExtraWinSalRGB = 0;

    if(!pSalRGB->ImplGethDIB())
    {
        // we need DIB for success with AcquireBuffer, create a replacement WinSalBitmap
        pExtraWinSalRGB = new WinSalBitmap();
        pExtraWinSalRGB->Create(*pSalRGB, pSalRGB->GetBitCount());
        pSalRGB = pExtraWinSalRGB;
    }

    BitmapBuffer* pRGB = pSalRGB->AcquireBuffer(true);
    BitmapBuffer* pExtraRGB = 0;

    if(pRGB && BMP_FORMAT_24BIT_TC_BGR != (pRGB->mnFormat & ~BMP_FORMAT_TOP_DOWN))
    {
        // convert source bitmap to BMP_FORMAT_24BIT_TC_BGR format if not yet in that format
        SalTwoRect aSalTwoRect;

        aSalTwoRect.mnSrcX = aSalTwoRect.mnSrcY = aSalTwoRect.mnDestX = aSalTwoRect.mnDestY = 0;
        aSalTwoRect.mnSrcWidth = aSalTwoRect.mnDestWidth = pRGB->mnWidth;
        aSalTwoRect.mnSrcHeight = aSalTwoRect.mnDestHeight = pRGB->mnHeight;

        pExtraRGB = StretchAndConvert(
            *pRGB,
            aSalTwoRect,
            BMP_FORMAT_24BIT_TC_BGR,
            0);

        pSalRGB->ReleaseBuffer(pRGB, true);
        pRGB = pExtraRGB;
    }

    WinSalBitmap* pSalA = const_cast< WinSalBitmap* >(&rAlphaSource);
    WinSalBitmap* pExtraWinSalA = 0;

    if(!pSalA->ImplGethDIB())
    {
        // we need DIB for success with AcquireBuffer, create a replacement WinSalBitmap
        pExtraWinSalA = new WinSalBitmap();
        pExtraWinSalA->Create(*pSalA, pSalA->GetBitCount());
        pSalA = pExtraWinSalA;
    }

    BitmapBuffer* pA = pSalA->AcquireBuffer(true);
    BitmapBuffer* pExtraA = 0;

    if(pA && BMP_FORMAT_8BIT_PAL != (pA->mnFormat & ~BMP_FORMAT_TOP_DOWN))
    {
        // convert alpha bitmap to BMP_FORMAT_8BIT_PAL format if not yet in that format
        SalTwoRect aSalTwoRect;

        aSalTwoRect.mnSrcX = aSalTwoRect.mnSrcY = aSalTwoRect.mnDestX = aSalTwoRect.mnDestY = 0;
        aSalTwoRect.mnSrcWidth = aSalTwoRect.mnDestWidth = pA->mnWidth;
        aSalTwoRect.mnSrcHeight = aSalTwoRect.mnDestHeight = pA->mnHeight;
        const BitmapPalette& rTargetPalette = Bitmap::GetGreyPalette(256);

        pExtraA = StretchAndConvert(
            *pA,
            aSalTwoRect,
            BMP_FORMAT_8BIT_PAL,
            &rTargetPalette);

        pSalA->ReleaseBuffer(pA, true);
        pA = pExtraA;
    }

    if(pRGB
        && pA
        && pRGB->mnWidth > 0
        && pRGB->mnHeight > 0
        && pRGB->mnWidth == pA->mnWidth
        && pRGB->mnHeight == pA->mnHeight
        && BMP_FORMAT_24BIT_TC_BGR == (pRGB->mnFormat & ~BMP_FORMAT_TOP_DOWN)
        && BMP_FORMAT_8BIT_PAL == (pA->mnFormat & ~BMP_FORMAT_TOP_DOWN))
    {
        // we have alpha and bitmap in known formats, create GdiPlus Bitmap as 32bit ARGB
        const sal_uInt32 nW(pRGB->mnWidth);
        const sal_uInt32 nH(pRGB->mnHeight);

        pRetval = new Gdiplus::Bitmap(nW, nH, PixelFormat32bppARGB);

        if(pRetval)
        {
            sal_uInt8* pSrcRGB(pRGB->mpBits);
            sal_uInt8* pSrcA(pA->mpBits);
            const sal_uInt32 nExtraRGB(pRGB->mnScanlineSize - (nW * 3));
            const sal_uInt32 nExtraA(pA->mnScanlineSize - nW);
            const bool bTopDown(pRGB->mnFormat & BMP_FORMAT_TOP_DOWN);
            const Gdiplus::Rect aAllRect(0, 0, nW, nH);
            Gdiplus::BitmapData aGdiPlusBitmapData;
            pRetval->LockBits(&aAllRect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &aGdiPlusBitmapData);

            // copy data to Gdiplus::Bitmap; format is BGRA; need to mix BGR from Bitmap and
            // A from alpha, so inner loop is needed (who invented BitmapEx..?)
            for(sal_uInt32 y(0); y < nH; y++)
            {
                const sal_uInt32 nYInsert(bTopDown ? y : nH - y - 1);
                sal_uInt8* targetPixels = (sal_uInt8*)aGdiPlusBitmapData.Scan0 + (nYInsert * aGdiPlusBitmapData.Stride);

                for(sal_uInt32 x(0); x < nW; x++)
                {
                    *targetPixels++ = *pSrcRGB++;
                    *targetPixels++ = *pSrcRGB++;
                    *targetPixels++ = *pSrcRGB++;
                    *targetPixels++ = 0xff - *pSrcA++;
                }

                pSrcRGB += nExtraRGB;
                pSrcA += nExtraA;
            }

            pRetval->UnlockBits(&aGdiPlusBitmapData);
        }
    }

    if(pExtraA)
    {
        delete pExtraA;
    }
    else
    {
        pSalA->ReleaseBuffer(pA, true);
    }

    if(pExtraWinSalA)
    {
        delete pExtraWinSalA;
    }

    if(pExtraRGB)
    {
        delete pExtraRGB;
    }
    else
    {
        pSalRGB->ReleaseBuffer(pRGB, true);
    }

    if(pExtraWinSalRGB)
    {
        delete pExtraWinSalRGB;
    }

    return pRetval;
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
        HDC                 hDC  = pGraphics->getHDC();
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
    bool bOverflow = (nImageSize / nAlignedWidth4Bytes) != (sal_uLong) rSize.Height();
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
        const sal_uInt16 nMinCount = (std::min)( nColors, rPal.GetEntryCount() );
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

        // find out size of source bitmap
        WIN_GetObject( hHdl, sizeof( BITMAP ), (LPSTR) &aBmp );

        // create destination bitmap
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
                memcpy( pBI->bmiColors, pBuffer->maPalette.ImplGetColorBuffer(), (std::min)( nDIBColorCount, nCount ) * sizeof( RGBQUAD ) );
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
