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
#include <vcl/bitmap.hxx>
#include <vcl/BitmapAccessMode.hxx>
#include <vcl/BitmapBuffer.hxx>
#include <vcl/BitmapPalette.hxx>
#include <vcl/ColorMask.hxx>
#include <vcl/Scanline.hxx>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <win/wincomp.hxx>
#include <win/salgdi.h>
#include <win/saldata.hxx>
#include <win/salbmp.h>
#include <string.h>
#include <vcl/timer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <sal/log.hxx>
#include <tools/helpers.hxx>
#include <map>

#if defined _MSC_VER
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#endif

#include <prewin.h>
#include <gdiplus.h>
#include <postwin.h>

#if defined _MSC_VER
#undef min
#undef max
#endif

WinSalBitmap::WinSalBitmap()
:   SalBitmap(),
    basegfx::SystemDependentDataHolder(),
    maSize(),
    mhDIB(nullptr),
    mhDDB(nullptr),
    mnBitCount(0)
{
}

WinSalBitmap::~WinSalBitmap()
{
    Destroy();
}

void WinSalBitmap::Destroy()
{
    if( mhDIB )
        GlobalFree( mhDIB );
    else if( mhDDB )
        DeleteObject( mhDDB );

    maSize = Size();
    mnBitCount = 0;
}

namespace {

class SystemDependentData_GdiPlusBitmap : public basegfx::SystemDependentData
{
private:
    std::shared_ptr<Gdiplus::Bitmap>    mpGdiPlusBitmap;
    const WinSalBitmap*                 mpAssociatedAlpha;

public:
    SystemDependentData_GdiPlusBitmap(
        basegfx::SystemDependentDataManager& rSystemDependentDataManager,
        const std::shared_ptr<Gdiplus::Bitmap>& rGdiPlusBitmap,
        const WinSalBitmap* pAssociatedAlpha);

    const WinSalBitmap* getAssociatedAlpha() const { return mpAssociatedAlpha; }
    const std::shared_ptr<Gdiplus::Bitmap>& getGdiPlusBitmap() const { return mpGdiPlusBitmap; }

    virtual sal_Int64 estimateUsageInBytes() const override;
};

}

SystemDependentData_GdiPlusBitmap::SystemDependentData_GdiPlusBitmap(
    basegfx::SystemDependentDataManager& rSystemDependentDataManager,
    const std::shared_ptr<Gdiplus::Bitmap>& rGdiPlusBitmap,
    const WinSalBitmap* pAssociatedAlpha)
:   basegfx::SystemDependentData(rSystemDependentDataManager),
    mpGdiPlusBitmap(rGdiPlusBitmap),
    mpAssociatedAlpha(pAssociatedAlpha)
{
}

sal_Int64 SystemDependentData_GdiPlusBitmap::estimateUsageInBytes() const
{
    sal_Int64 nRetval(0);

    if(mpGdiPlusBitmap)
    {
        const UINT nWidth(mpGdiPlusBitmap->GetWidth());
        const UINT nHeight(mpGdiPlusBitmap->GetHeight());

        if(0 != nWidth && 0 != nHeight)
        {
            nRetval = nWidth * nHeight;

            switch(mpGdiPlusBitmap->GetPixelFormat())
            {
                case PixelFormat1bppIndexed:
                    nRetval /= 8;
                    break;
                case PixelFormat4bppIndexed:
                    nRetval /= 4;
                    break;
                case PixelFormat16bppGrayScale:
                case PixelFormat16bppRGB555:
                case PixelFormat16bppRGB565:
                case PixelFormat16bppARGB1555:
                    nRetval *= 2;
                    break;
                case PixelFormat24bppRGB:
                    nRetval *= 3;
                    break;
                case PixelFormat32bppRGB:
                case PixelFormat32bppARGB:
                case PixelFormat32bppPARGB:
                case PixelFormat32bppCMYK:
                    nRetval *= 4;
                    break;
                case PixelFormat48bppRGB:
                    nRetval *= 6;
                    break;
                case PixelFormat64bppARGB:
                case PixelFormat64bppPARGB:
                    nRetval *= 8;
                    break;
                default:
                case PixelFormat8bppIndexed:
                    break;
            }
        }
    }

    return nRetval;
}

std::shared_ptr< Gdiplus::Bitmap > WinSalBitmap::ImplGetGdiPlusBitmap(const WinSalBitmap* pAlphaSource) const
{
    std::shared_ptr< Gdiplus::Bitmap > aRetval;

    // try to access buffered data
    std::shared_ptr<SystemDependentData_GdiPlusBitmap> pSystemDependentData_GdiPlusBitmap(
        getSystemDependentData<SystemDependentData_GdiPlusBitmap>());

    if(pSystemDependentData_GdiPlusBitmap)
    {
        // check data validity
        if(pSystemDependentData_GdiPlusBitmap->getAssociatedAlpha() != pAlphaSource
            || 0 == maSize.Width()
            || 0 == maSize.Height())
        {
            // #122350# if associated alpha with which the GDIPlus was constructed has changed
            // it is necessary to remove it from buffer, reset reference to it and reconstruct
            // data invalid, forget
            pSystemDependentData_GdiPlusBitmap.reset();
        }
    }

    if(pSystemDependentData_GdiPlusBitmap)
    {
        // use from buffer
        aRetval = pSystemDependentData_GdiPlusBitmap->getGdiPlusBitmap();
    }
    else if(!maSize.IsEmpty())
    {
        // create and set data
        const WinSalBitmap* pAssociatedAlpha(nullptr);

        if(pAlphaSource)
        {
            aRetval = const_cast< WinSalBitmap* >(this)->ImplCreateGdiPlusBitmap(*pAlphaSource);
            pAssociatedAlpha = pAlphaSource;
        }
        else
        {
            aRetval = const_cast< WinSalBitmap* >(this)->ImplCreateGdiPlusBitmap();
            pAssociatedAlpha = nullptr;
        }

        // add to buffering mechanism
        addOrReplaceSystemDependentData<SystemDependentData_GdiPlusBitmap>(
            ImplGetSystemDependentDataManager(),
            aRetval,
            pAssociatedAlpha);
    }

    return aRetval;
}

std::shared_ptr<Gdiplus::Bitmap> WinSalBitmap::ImplCreateGdiPlusBitmap()
{
    std::shared_ptr<Gdiplus::Bitmap> pRetval;
    WinSalBitmap* pSalRGB = this;
    std::unique_ptr<WinSalBitmap> pExtraWinSalRGB;

    if(!pSalRGB->ImplGethDIB())
    {
        // we need DIB for success with AcquireBuffer, create a replacement WinSalBitmap
        pExtraWinSalRGB.reset(new WinSalBitmap());
        pExtraWinSalRGB->Create(*pSalRGB, vcl::bitDepthToPixelFormat(pSalRGB->GetBitCount()));
        pSalRGB = pExtraWinSalRGB.get();
    }

    BitmapBuffer* pRGB = pSalRGB->AcquireBuffer(BitmapAccessMode::Read);
    std::unique_ptr<BitmapBuffer> pExtraRGB;

    if(pRGB && ScanlineFormat::N24BitTcBgr != RemoveScanline(pRGB->mnFormat))
    {
        // convert source bitmap to BMP_FORMAT_24BIT_TC_BGR format if not yet in that format
        SalTwoRect aSalTwoRect(0, 0, pRGB->mnWidth, pRGB->mnHeight, 0, 0, pRGB->mnWidth, pRGB->mnHeight);
        pExtraRGB = StretchAndConvert(
            *pRGB,
            aSalTwoRect,
            ScanlineFormat::N24BitTcBgr);

        pSalRGB->ReleaseBuffer(pRGB, BitmapAccessMode::Write);
        pRGB = pExtraRGB.get();
    }

    if(pRGB
        && pRGB->mnWidth > 0
        && pRGB->mnHeight > 0
        && ScanlineFormat::N24BitTcBgr == RemoveScanline(pRGB->mnFormat))
    {
        const sal_uInt32 nW(pRGB->mnWidth);
        const sal_uInt32 nH(pRGB->mnHeight);

        pRetval = std::make_shared<Gdiplus::Bitmap>(nW, nH, PixelFormat24bppRGB);

        if ( pRetval->GetLastStatus() == Gdiplus::Ok )
        {
            sal_uInt8* pSrcRGB(pRGB->mpBits);
            const sal_uInt32 nExtraRGB(pRGB->mnScanlineSize - (nW * 3));
            const bool bTopDown(pRGB->mnFormat & ScanlineFormat::TopDown);
            const Gdiplus::Rect aAllRect(0, 0, nW, nH);
            Gdiplus::BitmapData aGdiPlusBitmapData;
            pRetval->LockBits(&aAllRect, Gdiplus::ImageLockModeWrite, PixelFormat24bppRGB, &aGdiPlusBitmapData);

            // copy data to Gdiplus::Bitmap; format is BGR here in both cases, so memcpy is possible
            for(sal_uInt32 y(0); y < nH; y++)
            {
                const sal_uInt32 nYInsert(bTopDown ? y : nH - y - 1);
                sal_uInt8* targetPixels = static_cast<sal_uInt8*>(aGdiPlusBitmapData.Scan0) + (nYInsert * aGdiPlusBitmapData.Stride);

                memcpy(targetPixels, pSrcRGB, nW * 3);
                pSrcRGB += nW * 3 + nExtraRGB;
            }

            pRetval->UnlockBits(&aGdiPlusBitmapData);
        }
        else
        {
            pRetval.reset();
        }
    }

    if(pExtraRGB)
    {
        // #i123478# shockingly, BitmapBuffer does not free the memory it is controlling
        // in its destructor, this *has to be done by hand*. Doing it here now
        delete[] pExtraRGB->mpBits;
        pExtraRGB.reset();
    }
    else
    {
        pSalRGB->ReleaseBuffer(pRGB, BitmapAccessMode::Read);
    }

    return pRetval;
}

std::shared_ptr<Gdiplus::Bitmap> WinSalBitmap::ImplCreateGdiPlusBitmap(const WinSalBitmap& rAlphaSource)
{
    std::shared_ptr<Gdiplus::Bitmap> pRetval;
    WinSalBitmap* pSalRGB = this;
    std::unique_ptr<WinSalBitmap> pExtraWinSalRGB;

    if(!pSalRGB->ImplGethDIB())
    {
        // we need DIB for success with AcquireBuffer, create a replacement WinSalBitmap
        pExtraWinSalRGB.reset(new WinSalBitmap());
        pExtraWinSalRGB->Create(*pSalRGB, vcl::bitDepthToPixelFormat(pSalRGB->GetBitCount()));
        pSalRGB = pExtraWinSalRGB.get();
    }

    BitmapBuffer* pRGB = pSalRGB->AcquireBuffer(BitmapAccessMode::Read);
    std::unique_ptr<BitmapBuffer> pExtraRGB;

    if(pRGB && ScanlineFormat::N24BitTcBgr != RemoveScanline(pRGB->mnFormat))
    {
        // convert source bitmap to canlineFormat::N24BitTcBgr format if not yet in that format
        SalTwoRect aSalTwoRect(0, 0, pRGB->mnWidth, pRGB->mnHeight, 0, 0, pRGB->mnWidth, pRGB->mnHeight);
        pExtraRGB = StretchAndConvert(
            *pRGB,
            aSalTwoRect,
            ScanlineFormat::N24BitTcBgr);

        pSalRGB->ReleaseBuffer(pRGB, BitmapAccessMode::Read);
        pRGB = pExtraRGB.get();
    }

    WinSalBitmap* pSalA = const_cast< WinSalBitmap* >(&rAlphaSource);
    std::unique_ptr<WinSalBitmap> pExtraWinSalA;

    if(!pSalA->ImplGethDIB())
    {
        // we need DIB for success with AcquireBuffer, create a replacement WinSalBitmap
        pExtraWinSalA.reset(new WinSalBitmap());
        pExtraWinSalA->Create(*pSalA, vcl::bitDepthToPixelFormat(pSalA->GetBitCount()));
        pSalA = pExtraWinSalA.get();
    }

    BitmapBuffer* pA = pSalA->AcquireBuffer(BitmapAccessMode::Read);
    std::unique_ptr<BitmapBuffer> pExtraA;

    if(pA && ScanlineFormat::N8BitPal != RemoveScanline(pA->mnFormat))
    {
        // convert alpha bitmap to ScanlineFormat::N8BitPal format if not yet in that format
        SalTwoRect aSalTwoRect(0, 0, pA->mnWidth, pA->mnHeight, 0, 0, pA->mnWidth, pA->mnHeight);
        const BitmapPalette& rTargetPalette = Bitmap::GetGreyPalette(256);

        pExtraA = StretchAndConvert(
            *pA,
            aSalTwoRect,
            ScanlineFormat::N8BitPal,
            rTargetPalette);

        pSalA->ReleaseBuffer(pA, BitmapAccessMode::Read);
        pA = pExtraA.get();
    }

    if(pRGB
        && pA
        && pRGB->mnWidth > 0
        && pRGB->mnHeight > 0
        && pRGB->mnWidth == pA->mnWidth
        && pRGB->mnHeight == pA->mnHeight
        && ScanlineFormat::N24BitTcBgr == RemoveScanline(pRGB->mnFormat)
        && ScanlineFormat::N8BitPal == RemoveScanline(pA->mnFormat))
    {
        // we have alpha and bitmap in known formats, create GdiPlus Bitmap as 32bit ARGB
        const sal_uInt32 nW(pRGB->mnWidth);
        const sal_uInt32 nH(pRGB->mnHeight);

        pRetval = std::make_shared<Gdiplus::Bitmap>(nW, nH, PixelFormat32bppARGB);

        if ( pRetval->GetLastStatus() == Gdiplus::Ok ) // 2nd place to secure with new Gdiplus::Bitmap
        {
            sal_uInt8* pSrcRGB(pRGB->mpBits);
            sal_uInt8* pSrcA(pA->mpBits);
            const sal_uInt32 nExtraRGB(pRGB->mnScanlineSize - (nW * 3));
            const sal_uInt32 nExtraA(pA->mnScanlineSize - nW);
            const bool bTopDown(pRGB->mnFormat & ScanlineFormat::TopDown);
            const Gdiplus::Rect aAllRect(0, 0, nW, nH);
            Gdiplus::BitmapData aGdiPlusBitmapData;
            pRetval->LockBits(&aAllRect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &aGdiPlusBitmapData);

            // copy data to Gdiplus::Bitmap; format is BGRA; need to mix BGR from Bitmap and
            // A from alpha, so inner loop is needed (who invented BitmapEx..?)
            for(sal_uInt32 y(0); y < nH; y++)
            {
                const sal_uInt32 nYInsert(bTopDown ? y : nH - y - 1);
                sal_uInt8* targetPixels = static_cast<sal_uInt8*>(aGdiPlusBitmapData.Scan0) + (nYInsert * aGdiPlusBitmapData.Stride);

                for(sal_uInt32 x(0); x < nW; x++)
                {
                    *targetPixels++ = *pSrcRGB++;
                    *targetPixels++ = *pSrcRGB++;
                    *targetPixels++ = *pSrcRGB++;
                    *targetPixels++ = *pSrcA++;
                }

                pSrcRGB += nExtraRGB;
                pSrcA += nExtraA;
            }

            pRetval->UnlockBits(&aGdiPlusBitmapData);
        }
        else
        {
            pRetval.reset();
        }
    }

    if(pExtraA)
    {
        // #i123478# shockingly, BitmapBuffer does not free the memory it is controlling
        // in its destructor, this *has to be done handish*. Doing it here now
        delete[] pExtraA->mpBits;
        pExtraA.reset();
    }
    else
    {
        pSalA->ReleaseBuffer(pA, BitmapAccessMode::Read);
    }

    pExtraWinSalA.reset();

    if(pExtraRGB)
    {
        // #i123478# shockingly, BitmapBuffer does not free the memory it is controlling
        // in its destructor, this *has to be done by hand*. Doing it here now
        delete[] pExtraRGB->mpBits;
        pExtraRGB.reset();
    }
    else
    {
        pSalRGB->ReleaseBuffer(pRGB, BitmapAccessMode::Read);
    }

    pExtraWinSalRGB.reset();

    return pRetval;
}

bool WinSalBitmap::Create( HANDLE hBitmap )
{
    bool bRet = true;

    mhDDB = static_cast<HBITMAP>( hBitmap );

    if( mhDIB )
    {
        PBITMAPINFOHEADER pBIH = static_cast<PBITMAPINFOHEADER>(GlobalLock( mhDIB ));

        maSize = Size( pBIH->biWidth, pBIH->biHeight );
        mnBitCount = pBIH->biBitCount;

        if( mnBitCount )
            mnBitCount = ( mnBitCount <= 1 ) ? 1 : ( mnBitCount <= 4 ) ? 4 : ( mnBitCount <= 8 ) ? 8 : 24;

        GlobalUnlock( mhDIB );
    }
    else if( mhDDB )
    {
        BITMAP  aDDBInfo;

        if( GetObjectW( mhDDB, sizeof( aDDBInfo ), &aDDBInfo ) )
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
            mhDDB = nullptr;
            bRet = false;
        }
    }
    else
        bRet = false;

    return bRet;
}

bool WinSalBitmap::Create(const Size& rSize, vcl::PixelFormat ePixelFormat, const BitmapPalette& rPal)
{
    bool bRet = false;

    mhDIB = ImplCreateDIB(rSize, ePixelFormat, rPal);

    if( mhDIB )
    {
        maSize = rSize;
        mnBitCount = vcl::pixelFormatBitCount(ePixelFormat);
        bRet = true;
    }

    return bRet;
}

bool WinSalBitmap::Create( const SalBitmap& rSSalBitmap )
{
    bool bRet = false;
    const WinSalBitmap& rSalBitmap = static_cast<const WinSalBitmap&>(rSSalBitmap);

    if ( rSalBitmap.mhDIB || rSalBitmap.mhDDB )
    {
        HANDLE hNewHdl = ImplCopyDIBOrDDB( rSalBitmap.mhDIB ? rSalBitmap.mhDIB : rSalBitmap.mhDDB,
                                           rSalBitmap.mhDIB != nullptr );

        if ( hNewHdl )
        {
            if( rSalBitmap.mhDIB )
                mhDIB = static_cast<HGLOBAL>(hNewHdl);
            else if( rSalBitmap.mhDDB )
                mhDDB = static_cast<HBITMAP>(hNewHdl);

            maSize = rSalBitmap.maSize;
            mnBitCount = rSalBitmap.mnBitCount;

            bRet = true;
        }
    }

    return bRet;
}

bool WinSalBitmap::Create( const SalBitmap& rSSalBmp, SalGraphics* pSGraphics )
{
    bool bRet = false;

    const WinSalBitmap& rSalBmp = static_cast<const WinSalBitmap&>(rSSalBmp);
    WinSalGraphics* pGraphics = static_cast<WinSalGraphics*>(pSGraphics);

    if( rSalBmp.mhDIB )
    {
        PBITMAPINFO         pBI = static_cast<PBITMAPINFO>(GlobalLock( rSalBmp.mhDIB ));
        HDC                 hDC  = pGraphics->getHDC();
        HBITMAP             hNewDDB;
        BITMAP              aDDBInfo;
        PBYTE               pBits = reinterpret_cast<PBYTE>(pBI) + pBI->bmiHeader.biSize +
                            ImplGetDIBColorCount( rSalBmp.mhDIB ) * sizeof( RGBQUAD );

        if( pBI->bmiHeader.biBitCount == 1 )
        {
            hNewDDB = CreateBitmap( pBI->bmiHeader.biWidth, pBI->bmiHeader.biHeight, 1, 1, nullptr );

            if( hNewDDB )
                SetDIBits( hDC, hNewDDB, 0, pBI->bmiHeader.biHeight, pBits, pBI, DIB_RGB_COLORS );
        }
        else
            hNewDDB = CreateDIBitmap( hDC, &pBI->bmiHeader, CBM_INIT, pBits, pBI, DIB_RGB_COLORS );

        GlobalUnlock( rSalBmp.mhDIB );

        if( hNewDDB && GetObjectW( hNewDDB, sizeof( aDDBInfo ), &aDDBInfo ) )
        {
            mhDDB = hNewDDB;
            maSize = Size( aDDBInfo.bmWidth, aDDBInfo.bmHeight );
            mnBitCount = aDDBInfo.bmPlanes * aDDBInfo.bmBitsPixel;

            bRet = true;
        }
        else if( hNewDDB )
            DeleteObject( hNewDDB );
    }

    return bRet;
}

bool WinSalBitmap::Create(const SalBitmap& rSSalBmp, vcl::PixelFormat eNewPixelFormat)
{
    bool bRet = false;

    const WinSalBitmap& rSalBmp = static_cast<const WinSalBitmap&>(rSSalBmp);

    if( rSalBmp.mhDDB )
    {
        mhDIB = ImplCreateDIB( rSalBmp.maSize, eNewPixelFormat, BitmapPalette() );

        if( mhDIB )
        {
            PBITMAPINFO pBI = static_cast<PBITMAPINFO>(GlobalLock( mhDIB ));
            const int   nLines = static_cast<int>(rSalBmp.maSize.Height());
            HDC         hDC = GetDC( nullptr );
            PBYTE       pBits = reinterpret_cast<PBYTE>(pBI) + pBI->bmiHeader.biSize +
                                ImplGetDIBColorCount( mhDIB ) * sizeof( RGBQUAD );
            SalData*    pSalData = GetSalData();
            HPALETTE    hOldPal = nullptr;

            if ( pSalData->mhDitherPal )
            {
                hOldPal = SelectPalette( hDC, pSalData->mhDitherPal, TRUE );
                RealizePalette( hDC );
            }

            if( GetDIBits( hDC, rSalBmp.mhDDB, 0, nLines, pBits, pBI, DIB_RGB_COLORS ) == nLines )
            {
                GlobalUnlock( mhDIB );
                maSize = rSalBmp.maSize;
                mnBitCount = vcl::pixelFormatBitCount(eNewPixelFormat);
                bRet = true;
            }
            else
            {
                GlobalUnlock( mhDIB );
                GlobalFree( mhDIB );
                mhDIB = nullptr;
            }

            if( hOldPal )
                SelectPalette( hDC, hOldPal, TRUE );

            ReleaseDC( nullptr, hDC );
        }
    }

    return bRet;
}

bool WinSalBitmap::Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& rBitmapCanvas, Size& /*rSize*/, bool bMask )
{
    css::uno::Reference< css::beans::XFastPropertySet >
        xFastPropertySet( rBitmapCanvas, css::uno::UNO_QUERY );

    if( xFastPropertySet ) {
        css::uno::Sequence< css::uno::Any > args;

        if( xFastPropertySet->getFastPropertyValue(bMask ? 2 : 1) >>= args ) {
            sal_Int64 aHBmp64;

            if( args[0] >>= aHBmp64 ) {
                return Create( reinterpret_cast<HANDLE>(aHBmp64) );
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
        PBITMAPINFO         pBI = static_cast<PBITMAPINFO>(GlobalLock( hDIB ));

        if ( pBI->bmiHeader.biSize != sizeof( BITMAPCOREHEADER ) )
        {
            if( pBI->bmiHeader.biBitCount <= 8 )
            {
                if ( pBI->bmiHeader.biClrUsed )
                    nColors = static_cast<sal_uInt16>(pBI->bmiHeader.biClrUsed);
                else
                    nColors = 1 << pBI->bmiHeader.biBitCount;
            }
        }
        else if( reinterpret_cast<PBITMAPCOREHEADER>(pBI)->bcBitCount <= 8 )
            nColors = 1 << reinterpret_cast<PBITMAPCOREHEADER>(pBI)->bcBitCount;

        GlobalUnlock( hDIB );
    }

    return nColors;
}

HGLOBAL WinSalBitmap::ImplCreateDIB(const Size& rSize, vcl::PixelFormat ePixelFormat, const BitmapPalette& rPal)
{
    HGLOBAL hDIB = nullptr;

    if( rSize.IsEmpty() )
        return hDIB;

    const auto nBits = vcl::pixelFormatBitCount(ePixelFormat);

    // calculate bitmap size in Bytes
    const sal_uLong nAlignedWidth4Bytes = AlignedWidth4Bytes(nBits * rSize.Width());
    const sal_uLong nImageSize = nAlignedWidth4Bytes * rSize.Height();
    bool bOverflow = (nImageSize / nAlignedWidth4Bytes) != static_cast<sal_uLong>(rSize.Height());
    if( bOverflow )
        return hDIB;

    // allocate bitmap memory including header and palette
    sal_uInt16 nColors = 0;
    if (ePixelFormat <= vcl::PixelFormat::N8_BPP)
        nColors = vcl::numberOfColors(ePixelFormat);

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

HANDLE WinSalBitmap::ImplCopyDIBOrDDB( HANDLE hHdl, bool bDIB )
{
    HANDLE  hCopy = nullptr;

    if ( bDIB && hHdl )
    {
        const sal_uLong nSize = GlobalSize( hHdl );

        if ( (hCopy = GlobalAlloc( GHND, nSize  )) != nullptr )
        {
            memcpy( GlobalLock( hCopy ), GlobalLock( hHdl ), nSize );

            GlobalUnlock( hCopy );
            GlobalUnlock( hHdl );
        }
    }
    else if ( hHdl )
    {
        BITMAP aBmp;

        // find out size of source bitmap
        GetObjectW( hHdl, sizeof( aBmp ), &aBmp );

        // create destination bitmap
        if ( (hCopy = CreateBitmapIndirect( &aBmp )) != nullptr )
        {
            HDC     hBmpDC = CreateCompatibleDC( nullptr );
            HBITMAP hBmpOld = static_cast<HBITMAP>(SelectObject( hBmpDC, hHdl ));
            HDC     hCopyDC = CreateCompatibleDC( hBmpDC );
            HBITMAP hCopyOld = static_cast<HBITMAP>(SelectObject( hCopyDC, hCopy ));

            BitBlt( hCopyDC, 0, 0, aBmp.bmWidth, aBmp.bmHeight, hBmpDC, 0, 0, SRCCOPY );

            SelectObject( hCopyDC, hCopyOld );
            DeleteDC( hCopyDC );

            SelectObject( hBmpDC, hBmpOld );
            DeleteDC( hBmpDC );
        }
    }

    return hCopy;
}

BitmapBuffer* WinSalBitmap::AcquireBuffer( BitmapAccessMode /*nMode*/ )
{
    std::unique_ptr<BitmapBuffer> pBuffer;

    if( mhDIB )
    {
        PBITMAPINFO         pBI = static_cast<PBITMAPINFO>(GlobalLock( mhDIB ));
        PBITMAPINFOHEADER   pBIH = &pBI->bmiHeader;

        if( pBIH->biPlanes == 1 )
        {
            pBuffer.reset(new BitmapBuffer);

            pBuffer->mnFormat = pBIH->biBitCount == 1 ? ScanlineFormat::N1BitMsbPal :
                                pBIH->biBitCount == 8 ? ScanlineFormat::N8BitPal :
                                pBIH->biBitCount == 24 ? ScanlineFormat::N24BitTcBgr :
                                pBIH->biBitCount == 32 ? ScanlineFormat::N32BitTcMask :
                                ScanlineFormat::NONE;

            if( RemoveScanline( pBuffer->mnFormat ) != ScanlineFormat::NONE )
            {
                pBuffer->mnWidth = maSize.Width();
                pBuffer->mnHeight = maSize.Height();
                pBuffer->mnScanlineSize = AlignedWidth4Bytes( maSize.Width() * pBIH->biBitCount );
                pBuffer->mnBitCount = static_cast<sal_uInt16>(pBIH->biBitCount);

                if( pBuffer->mnBitCount <= 8 )
                {
                    const sal_uInt16 nPalCount = ImplGetDIBColorCount( mhDIB );

                    pBuffer->maPalette.SetEntryCount( nPalCount );
                    memcpy( pBuffer->maPalette.ImplGetColorBuffer(), pBI->bmiColors, nPalCount * sizeof( RGBQUAD ) );
                    pBuffer->mpBits = reinterpret_cast<PBYTE>(pBI) + pBI->bmiHeader.biSize + nPalCount * sizeof( RGBQUAD );
                }
                else if( ( pBIH->biBitCount == 16 ) || ( pBIH->biBitCount == 32 ) )
                {
                    sal_uLong nOffset = 0;

                    if( pBIH->biCompression == BI_BITFIELDS )
                    {
                        nOffset = 3 * sizeof( RGBQUAD );
                        ColorMaskElement aRedMask(*reinterpret_cast<UINT32*>(&pBI->bmiColors[ 0 ]));
                        aRedMask.CalcMaskShift();
                        ColorMaskElement aGreenMask(*reinterpret_cast<UINT32*>(&pBI->bmiColors[ 1 ]));
                        aGreenMask.CalcMaskShift();
                        ColorMaskElement aBlueMask(*reinterpret_cast<UINT32*>(&pBI->bmiColors[ 2 ]));
                        aBlueMask.CalcMaskShift();
                        pBuffer->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);
                    }
                    else if( pBIH->biBitCount == 16 )
                    {
                        ColorMaskElement aRedMask(0x00007c00UL);
                        aRedMask.CalcMaskShift();
                        ColorMaskElement aGreenMask(0x000003e0UL);
                        aGreenMask.CalcMaskShift();
                        ColorMaskElement aBlueMask(0x0000001fUL);
                        aBlueMask.CalcMaskShift();
                        pBuffer->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);
                    }
                    else
                    {
                        ColorMaskElement aRedMask(0x00ff0000UL);
                        aRedMask.CalcMaskShift();
                        ColorMaskElement aGreenMask(0x0000ff00UL);
                        aGreenMask.CalcMaskShift();
                        ColorMaskElement aBlueMask(0x000000ffUL);
                        aBlueMask.CalcMaskShift();
                        pBuffer->maColorMask = ColorMask(aRedMask, aGreenMask, aBlueMask);
                    }

                    pBuffer->mpBits = reinterpret_cast<PBYTE>(pBI) + pBI->bmiHeader.biSize + nOffset;
                }
                else
                    pBuffer->mpBits = reinterpret_cast<PBYTE>(pBI) + pBI->bmiHeader.biSize;
            }
            else
            {
                GlobalUnlock( mhDIB );
                pBuffer.reset();
            }
        }
        else
            GlobalUnlock( mhDIB );
    }

    return pBuffer.release();
}

void WinSalBitmap::ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode )
{
    if( pBuffer )
    {
        if( mhDIB )
        {
            if( nMode == BitmapAccessMode::Write && !!pBuffer->maPalette )
            {
                PBITMAPINFO     pBI = static_cast<PBITMAPINFO>(GlobalLock( mhDIB ));
                const sal_uInt16    nCount = pBuffer->maPalette.GetEntryCount();
                const sal_uInt16    nDIBColorCount = ImplGetDIBColorCount( mhDIB );
                memcpy( pBI->bmiColors, pBuffer->maPalette.ImplGetColorBuffer(), std::min( nDIBColorCount, nCount ) * sizeof( RGBQUAD ) );
                GlobalUnlock( mhDIB );
            }

            GlobalUnlock( mhDIB );
        }

        delete pBuffer;
    }
    if( nMode == BitmapAccessMode::Write )
        InvalidateChecksum();
}

bool WinSalBitmap::GetSystemData( BitmapSystemData& rData )
{
    bool bRet = false;
    if( mhDIB || mhDDB )
    {
        bRet = true;
        rData.pDIB = mhDIB;
        const Size& rSize = GetSize ();
        rData.mnWidth = rSize.Width();
        rData.mnHeight = rSize.Height();
    }
    return bRet;
}

bool WinSalBitmap::ScalingSupported() const
{
    return false;
}

bool WinSalBitmap::Scale( const double& /*rScaleX*/, const double& /*rScaleY*/, BmpScaleFlag /*nScaleFlag*/ )
{
    return false;
}

bool WinSalBitmap::Replace( const Color& /*rSearchColor*/, const Color& /*rReplaceColor*/, sal_uInt8 /*nTol*/ )
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
