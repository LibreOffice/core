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

#include <sal/config.h>

#include <sal/log.hxx>
#include <tools/debug.hxx>

#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/virdev.hxx>

#include <ImplOutDevData.hxx>
#include <font/PhysicalFontCollection.hxx>
#include <font/PhysicalFontFaceCollection.hxx>
#include <impfontcache.hxx>
#include <salinst.hxx>
#include <salgdi.hxx>
#include <salvd.hxx>
#include <svdata.hxx>

using namespace ::com::sun::star::uno;

bool VirtualDevice::CanEnableNativeWidget() const
{
    const vcl::ExtOutDevData* pOutDevData(GetExtOutDevData());
    const vcl::PDFExtOutDevData* pPDFData(dynamic_cast<const vcl::PDFExtOutDevData*>(pOutDevData));
    return pPDFData == nullptr;
}

bool VirtualDevice::AcquireGraphics() const
{
    DBG_TESTSOLARMUTEX();

    if ( mpGraphics )
        return true;

    mbInitLineColor     = true;
    mbInitFillColor     = true;
    mbInitFont          = true;
    mbInitTextColor     = true;
    mbInitClipRegion    = true;

    ImplSVData* pSVData = ImplGetSVData();

    if ( mpVirDev )
    {
        mpGraphics = mpVirDev->AcquireGraphics();
        // if needed retry after releasing least recently used virtual device graphics
        while ( !mpGraphics )
        {
            if ( !pSVData->maGDIData.mpLastVirGraphics )
                break;
            pSVData->maGDIData.mpLastVirGraphics->ReleaseGraphics();
            mpGraphics = mpVirDev->AcquireGraphics();
        }
        // update global LRU list of virtual device graphics
        if ( mpGraphics )
        {
            mpNextGraphics = pSVData->maGDIData.mpFirstVirGraphics;
            pSVData->maGDIData.mpFirstVirGraphics = const_cast<VirtualDevice*>(this);
            if ( mpNextGraphics )
                mpNextGraphics->mpPrevGraphics = const_cast<VirtualDevice*>(this);
            if ( !pSVData->maGDIData.mpLastVirGraphics )
                pSVData->maGDIData.mpLastVirGraphics = const_cast<VirtualDevice*>(this);
        }
    }

    if ( mpGraphics )
    {
        mpGraphics->SetXORMode( (RasterOp::Invert == meRasterOp) || (RasterOp::Xor == meRasterOp), RasterOp::Invert == meRasterOp );
        mpGraphics->setAntiAlias(bool(mnAntialiasing & AntialiasingFlags::Enable));
    }

    return mpGraphics != nullptr;
}

void VirtualDevice::ReleaseGraphics( bool bRelease )
{
    DBG_TESTSOLARMUTEX();

    if ( !mpGraphics )
        return;

    // release the fonts of the physically released graphics device
    if ( bRelease )
        ImplReleaseFonts();

    ImplSVData* pSVData = ImplGetSVData();

    VirtualDevice* pVirDev = this;

    if ( bRelease )
        pVirDev->mpVirDev->ReleaseGraphics( mpGraphics );
    // remove from global LRU list of virtual device graphics
    if ( mpPrevGraphics )
        mpPrevGraphics->mpNextGraphics = mpNextGraphics;
    else
        pSVData->maGDIData.mpFirstVirGraphics = mpNextGraphics;
    if ( mpNextGraphics )
        mpNextGraphics->mpPrevGraphics = mpPrevGraphics;
    else
        pSVData->maGDIData.mpLastVirGraphics = mpPrevGraphics;

    mpGraphics      = nullptr;
    mpPrevGraphics  = nullptr;
    mpNextGraphics  = nullptr;
}

void VirtualDevice::ImplInitVirDev( const OutputDevice* pOutDev,
                                    tools::Long nDX, tools::Long nDY, const SystemGraphicsData *pData )
{
    SAL_INFO( "vcl.virdev", "ImplInitVirDev(" << nDX << "," << nDY << ")" );

    meRefDevMode = RefDevMode::NONE;
    mbForceZeroExtleadBug = false;

    bool bErase = nDX > 0 && nDY > 0;

    if ( nDX < 1 )
        nDX = 1;

    if ( nDY < 1 )
        nDY = 1;

    ImplSVData* pSVData = ImplGetSVData();

    if ( !pOutDev )
        pOutDev = ImplGetDefaultWindow()->GetOutDev();
    if( !pOutDev )
        return;

    SalGraphics* pGraphics;
    if ( !pOutDev->mpGraphics )
        (void)pOutDev->AcquireGraphics();
    pGraphics = pOutDev->mpGraphics;
    if ( pGraphics )
        mpVirDev = pSVData->mpDefInst->CreateVirtualDevice(*pGraphics, nDX, nDY, meFormat, pData);
    else
        mpVirDev = nullptr;
    if ( !mpVirDev )
    {
        // do not abort but throw an exception, may be the current thread terminates anyway (plugin-scenario)
        throw css::uno::RuntimeException(
            "Could not create system bitmap!",
            css::uno::Reference< css::uno::XInterface >() );
    }

    mnBitCount = pOutDev->GetBitCount();
    mnOutWidth      = nDX;
    mnOutHeight     = nDY;

    mbScreenComp    = pOutDev->IsScreenComp();

    mbDevOutput     = true;
    mxFontCollection = pSVData->maGDIData.mxScreenFontList;
    mxFontCache     = pSVData->maGDIData.mxScreenFontCache;
    mnDPIX          = pOutDev->mnDPIX;
    mnDPIY          = pOutDev->mnDPIY;
    mnDPIScalePercentage = pOutDev->mnDPIScalePercentage;
    maFont          = pOutDev->maFont;

    if( maTextColor != pOutDev->maTextColor )
    {
        maTextColor = pOutDev->maTextColor;
        mbInitTextColor = true;
    }

    // virtual devices have white background by default
    SetBackground( Wallpaper( COL_WHITE ) );

    // #i59283# don't erase user-provided surface
    if( !pData && bErase)
        Erase();

    // register VirDev in the list
    mpNext = pSVData->maGDIData.mpFirstVirDev;
    mpPrev = nullptr;
    if ( mpNext )
        mpNext->mpPrev = this;
    pSVData->maGDIData.mpFirstVirDev = this;
}

VirtualDevice::VirtualDevice(const OutputDevice* pCompDev, DeviceFormat eFormat,
                             DeviceFormat eAlphaFormat, OutDevType eOutDevType)
    : OutputDevice(eOutDevType)
    , meFormat(eFormat)
    , meAlphaFormat(eAlphaFormat)
{
    SAL_INFO( "vcl.virdev", "VirtualDevice::VirtualDevice( " << static_cast<int>(eFormat)
                            << ", " << static_cast<int>(eAlphaFormat)
                            << ", " << static_cast<int>(eOutDevType) << " )" );

    ImplInitVirDev(pCompDev ? pCompDev : Application::GetDefaultDevice(), 0, 0);
}

VirtualDevice::VirtualDevice(const SystemGraphicsData& rData, const Size &rSize,
                             DeviceFormat eFormat)
    : OutputDevice(OUTDEV_VIRDEV)
    , meFormat(eFormat)
    , meAlphaFormat(DeviceFormat::NONE)
{
    SAL_INFO( "vcl.virdev", "VirtualDevice::VirtualDevice( " << static_cast<int>(eFormat) << " )" );

    ImplInitVirDev(Application::GetDefaultDevice(), rSize.Width(), rSize.Height(), &rData);
}

VirtualDevice::~VirtualDevice()
{
    SAL_INFO( "vcl.virdev", "VirtualDevice::~VirtualDevice()" );
    disposeOnce();
}

void VirtualDevice::dispose()
{
    SAL_INFO( "vcl.virdev", "VirtualDevice::dispose()" );

    ImplSVData* pSVData = ImplGetSVData();

    ReleaseGraphics();

    mpVirDev.reset();

    // remove this VirtualDevice from the double-linked global list
    if( mpPrev )
        mpPrev->mpNext = mpNext;
    else
        pSVData->maGDIData.mpFirstVirDev = mpNext;

    if( mpNext )
        mpNext->mpPrev = mpPrev;

    OutputDevice::dispose();
}

bool VirtualDevice::InnerImplSetOutputSizePixel( const Size& rNewSize, bool bErase,
                                                 sal_uInt8 *const pBuffer)
{
    SAL_INFO( "vcl.virdev",
              "VirtualDevice::InnerImplSetOutputSizePixel( " << rNewSize.Width() << ", "
              << rNewSize.Height() << ", " << int(bErase) << " )" );

    if ( !mpVirDev )
        return false;
    else if ( rNewSize == GetOutputSizePixel() )
    {
        if ( bErase )
            Erase();
        SAL_INFO( "vcl.virdev", "Trying to re-use a VirtualDevice but this time using a pre-allocated buffer");
        return true;
    }

    bool bRet;
    tools::Long nNewWidth = rNewSize.Width(), nNewHeight = rNewSize.Height();

    if ( nNewWidth < 1 )
        nNewWidth = 1;

    if ( nNewHeight < 1 )
        nNewHeight = 1;

    if ( bErase )
    {
        if ( pBuffer )
            bRet = mpVirDev->SetSizeUsingBuffer( nNewWidth, nNewHeight, pBuffer );
        else
            bRet = mpVirDev->SetSize( nNewWidth, nNewHeight );

        if ( bRet )
        {
            mnOutWidth  = rNewSize.Width();
            mnOutHeight = rNewSize.Height();
            Erase();
        }
    }
    else
    {
        std::unique_ptr<SalVirtualDevice> pNewVirDev;
        ImplSVData*         pSVData = ImplGetSVData();

        // we need a graphics
        if ( !mpGraphics && !AcquireGraphics() )
            return false;

        assert(mpGraphics);

        pNewVirDev = pSVData->mpDefInst->CreateVirtualDevice(*mpGraphics, nNewWidth, nNewHeight, meFormat);
        if ( pNewVirDev )
        {
            SalGraphics* pGraphics = pNewVirDev->AcquireGraphics();
            if ( pGraphics )
            {
                tools::Long nWidth;
                tools::Long nHeight;
                if ( mnOutWidth < nNewWidth )
                    nWidth = mnOutWidth;
                else
                    nWidth = nNewWidth;
                if ( mnOutHeight < nNewHeight )
                    nHeight = mnOutHeight;
                else
                    nHeight = nNewHeight;
                SalTwoRect aPosAry(0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight);
                pGraphics->CopyBits( aPosAry, *mpGraphics, *this, *this );
                pNewVirDev->ReleaseGraphics( pGraphics );
                ReleaseGraphics();
                mpVirDev = std::move(pNewVirDev);
                mnOutWidth  = rNewSize.Width();
                mnOutHeight = rNewSize.Height();
                bRet = true;
            }
            else
            {
                bRet = false;
            }
        }
        else
            bRet = false;
    }

    return bRet;
}

// #i32109#: Fill opaque areas correctly (without relying on
// fill/linecolor state)
void VirtualDevice::ImplFillOpaqueRectangle( const tools::Rectangle& rRect )
{
    // Set line and fill color to black (->opaque),
    // fill rect with that (linecolor, too, because of
    // those pesky missing pixel problems)
    Push( vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR );
    SetLineColor( COL_ALPHA_OPAQUE );
    SetFillColor( COL_ALPHA_OPAQUE );
    DrawRect( rRect );
    Pop();
}

bool VirtualDevice::ImplSetOutputSizePixel( const Size& rNewSize, bool bErase,
                                            sal_uInt8 *const pBuffer)
{
    if( InnerImplSetOutputSizePixel(rNewSize, bErase, pBuffer) )
    {
        if (meAlphaFormat != DeviceFormat::NONE)
        {
            // #110958# Setup alpha bitmap
            if(mpAlphaVDev && mpAlphaVDev->GetOutputSizePixel() != rNewSize)
            {
                mpAlphaVDev.disposeAndClear();
            }

            if( !mpAlphaVDev )
            {
                mpAlphaVDev = VclPtr<VirtualDevice>::Create(*this, meAlphaFormat);
                mpAlphaVDev->InnerImplSetOutputSizePixel(rNewSize, bErase, nullptr);
                mpAlphaVDev->SetBackground( Wallpaper(COL_ALPHA_OPAQUE) );
                mpAlphaVDev->Erase();
            }

            // TODO: copy full outdev state to new one, here. Also needed in outdev2.cxx:DrawOutDev
            if( GetLineColor() != COL_TRANSPARENT )
                mpAlphaVDev->SetLineColor( COL_ALPHA_OPAQUE );

            if( GetFillColor() != COL_TRANSPARENT )
                mpAlphaVDev->SetFillColor( COL_ALPHA_OPAQUE );

            mpAlphaVDev->SetMapMode( GetMapMode() );

            mpAlphaVDev->SetAntialiasing( GetAntialiasing() );
        }

        return true;
    }

    return false;
}

void VirtualDevice::EnableRTL( bool bEnable )
{
    // virdevs default to not mirroring, they will only be set to mirroring
    // under rare circumstances in the UI, eg the valueset control
    // because each virdev has its own SalGraphics we can safely switch the SalGraphics here
    // ...hopefully
    if( AcquireGraphics() )
        mpGraphics->SetLayout( bEnable ? SalLayoutFlags::BiDiRtl : SalLayoutFlags::NONE );

    OutputDevice::EnableRTL(bEnable);
}

bool VirtualDevice::SetOutputSizePixel( const Size& rNewSize, bool bErase )
{
    return ImplSetOutputSizePixel(rNewSize, bErase, nullptr);
}

bool VirtualDevice::SetOutputSizePixelScaleOffsetAndBuffer(
    const Size& rNewSize, const Fraction& rScale, const Point& rNewOffset,
    sal_uInt8 *const pBuffer)
{
    if (pBuffer) {
        MapMode mm = GetMapMode();
        mm.SetOrigin( rNewOffset );
        mm.SetScaleX( rScale );
        mm.SetScaleY( rScale );
        SetMapMode( mm );
    }
    return ImplSetOutputSizePixel(rNewSize, true, pBuffer);
}

void VirtualDevice::SetReferenceDevice( RefDevMode i_eRefDevMode )
{
    sal_Int32 nDPIX = 600, nDPIY = 600;
    switch( i_eRefDevMode )
    {
    case RefDevMode::NONE:
    default:
        SAL_WARN( "vcl.virdev", "VDev::SetRefDev illegal argument!" );
        break;
    case RefDevMode::Dpi600:
        nDPIX = nDPIY = 600;
        break;
    case RefDevMode::MSO1:
        nDPIX = nDPIY = 6*1440;
        break;
    case RefDevMode::PDF1:
        nDPIX = nDPIY = 720;
        break;
    }
    ImplSetReferenceDevice( i_eRefDevMode, nDPIX, nDPIY );
}

void VirtualDevice::SetReferenceDevice( sal_Int32 i_nDPIX, sal_Int32 i_nDPIY )
{
    ImplSetReferenceDevice( RefDevMode::Custom, i_nDPIX, i_nDPIY );
}

bool VirtualDevice::IsVirtual() const
{
    return true;
}

void VirtualDevice::ImplSetReferenceDevice( RefDevMode i_eRefDevMode, sal_Int32 i_nDPIX, sal_Int32 i_nDPIY )
{
    mnDPIX = i_nDPIX;
    mnDPIY = i_nDPIY;
    mnDPIScalePercentage = 100;

    EnableOutput( false );  // prevent output on reference device
    mbScreenComp = false;

    // invalidate currently selected fonts
    mbInitFont = true;
    mbNewFont = true;

    // avoid adjusting font lists when already in refdev mode
    RefDevMode nOldRefDevMode = meRefDevMode;
    meRefDevMode = i_eRefDevMode;
    if( nOldRefDevMode != RefDevMode::NONE )
        return;

    // the reference device should have only scalable fonts
    // => clean up the original font lists before getting new ones
    mpFontInstance.clear();
    mpFontFaceCollection.reset();

    // preserve global font lists
    ImplSVData* pSVData = ImplGetSVData();
    mxFontCollection.reset();
    mxFontCache.reset();

    // get font list with scalable fonts only
    (void)AcquireGraphics();
    mxFontCollection = pSVData->maGDIData.mxScreenFontList->Clone();

    // prepare to use new font lists
    mxFontCache = std::make_shared<ImplFontCache>();
}

sal_uInt16 VirtualDevice::GetBitCount() const
{
    return mnBitCount;
}

bool VirtualDevice::UsePolyPolygonForComplexGradient()
{
    return true;
}

void VirtualDevice::Compat_ZeroExtleadBug()
{
    mbForceZeroExtleadBug = true;
}

tools::Long VirtualDevice::GetFontExtLeading() const
{
#ifdef UNX
    // backwards compatible line metrics after fixing #i60945#
    if ( mbForceZeroExtleadBug )
        return 0;
#endif

    return mpFontInstance->mxFontMetric->GetExternalLeading();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
