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
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vcl/outdev.hxx>
#include <vcl/toolkit/unowrap.hxx>
#include <vcl/sysdata.hxx>

#include <salgdi.hxx>
#include <svdata.hxx>
#include <window.h>
#include <outdev.h>
#include <outdevstatestack.hxx>
#include <PhysicalFontCollection.hxx>

#ifdef DISABLE_DYNLOADING
// Linking all needed LO code into one .so/executable, these already
// exist in the tools library, so put them in the anonymous namespace
// here to avoid clash...
namespace {
#endif
#ifdef DISABLE_DYNLOADING
}
#endif

// Begin initializer and accessor public functions

OutputDevice::OutputDevice(OutDevType eOutDevType) :
    meOutDevType(eOutDevType),
    maRegion(true),
    maFillColor( COL_WHITE ),
    maTextLineColor( COL_TRANSPARENT ),
    mxSettings( new AllSettings(Application::GetSettings()) )
{
    mpGraphics                      = nullptr;
    mpUnoGraphicsList               = nullptr;
    mpPrevGraphics                  = nullptr;
    mpNextGraphics                  = nullptr;
    mpMetaFile                      = nullptr;
    mpFontInstance                     = nullptr;
    mpDeviceFontList                = nullptr;
    mpDeviceFontSizeList            = nullptr;
    mpOutDevStateStack.reset(new OutDevStateStack);
    mpAlphaVDev                     = nullptr;
    mpExtOutDevData                 = nullptr;
    mnOutOffX                       = 0;
    mnOutOffY                       = 0;
    mnOutWidth                      = 0;
    mnOutHeight                     = 0;
    mnDPIX                          = 0;
    mnDPIY                          = 0;
    mnDPIScalePercentage            = 100;
    mnTextOffX                      = 0;
    mnTextOffY                      = 0;
    mnOutOffOrigX                   = 0;
    mnOutOffLogicX                  = 0;
    mnOutOffOrigY                   = 0;
    mnOutOffLogicY                  = 0;
    mnEmphasisAscent                = 0;
    mnEmphasisDescent               = 0;
    mnDrawMode                      = DrawModeFlags::Default;
    mnTextLayoutMode                = ComplexTextLayoutFlags::Default;

    if( AllSettings::GetLayoutRTL() ) //#i84553# tip BiDi preference to RTL
        mnTextLayoutMode            = ComplexTextLayoutFlags::BiDiRtl | ComplexTextLayoutFlags::TextOriginLeft;

    meOutDevViewType                = OutDevViewType::DontKnow;
    mbMap                           = false;
    mbClipRegion                    = false;
    mbBackground                    = false;
    mbOutput                        = true;
    mbDevOutput                     = false;
    mbOutputClipped                 = false;
    maTextColor                     = COL_BLACK;
    maOverlineColor                 = COL_TRANSPARENT;
    meRasterOp                      = RasterOp::OverPaint;
    mnAntialiasing                  = AntialiasingFlags::NONE;
    meTextLanguage                  = LANGUAGE_SYSTEM;  // TODO: get default from configuration?
    mbLineColor                     = true;
    mbFillColor                     = true;
    mbInitLineColor                 = true;
    mbInitFillColor                 = true;
    mbInitFont                      = true;
    mbInitTextColor                 = true;
    mbInitClipRegion                = true;
    mbClipRegionSet                 = false;
    mbNewFont                       = true;
    mbTextLines                     = false;
    mbTextSpecial                   = false;
    mbRefPoint                      = false;
    mbEnableRTL                     = false;    // mirroring must be explicitly allowed (typically for windows only)

    // struct ImplMapRes
    maMapRes.mnMapOfsX              = 0;
    maMapRes.mnMapOfsY              = 0;
    maMapRes.mnMapScNumX            = 1;
    maMapRes.mnMapScNumY            = 1;
    maMapRes.mnMapScDenomX          = 1;
    maMapRes.mnMapScDenomY          = 1;
    // struct ImplThresholdRes
    maThresRes.mnThresLogToPixX     = 0;
    maThresRes.mnThresLogToPixY     = 0;
    maThresRes.mnThresPixToLogX     = 0;
    maThresRes.mnThresPixToLogY     = 0;

    // struct ImplOutDevData- see #i82615#
    mpOutDevData.reset(new ImplOutDevData);
    mpOutDevData->mpRotateDev       = nullptr;
    mpOutDevData->mpRecordLayout    = nullptr;

    // #i75163#
    mpOutDevData->mpViewTransform   = nullptr;
    mpOutDevData->mpInverseViewTransform = nullptr;
}

OutputDevice::~OutputDevice()
{
    disposeOnce();
}

void OutputDevice::dispose()
{
    if ( GetUnoGraphicsList() )
    {
        UnoWrapperBase* pWrapper = UnoWrapperBase::GetUnoWrapper( false );
        if ( pWrapper )
            pWrapper->ReleaseAllGraphics( this );
        delete mpUnoGraphicsList;
        mpUnoGraphicsList = nullptr;
    }

    mpOutDevData->mpRotateDev.disposeAndClear();

    // #i75163#
    ImplInvalidateViewTransform();

    mpOutDevData.reset();

    // for some reason, we haven't removed state from the stack properly
    if ( !mpOutDevStateStack->empty() )
    {
        SAL_WARN( "vcl.gdi", "OutputDevice::~OutputDevice(): OutputDevice::Push() calls != OutputDevice::Pop() calls" );
        while ( !mpOutDevStateStack->empty() )
        {
            mpOutDevStateStack->pop_back();
        }
    }
    mpOutDevStateStack.reset();

    // release the active font instance
    mpFontInstance.clear();

    // remove cached results of GetDevFontList/GetDevSizeList
    mpDeviceFontList.reset();
    mpDeviceFontSizeList.reset();

    // release ImplFontCache specific to this OutputDevice
    mxFontCache.reset();

    // release ImplFontList specific to this OutputDevice
    mxFontCollection.reset();

    mpAlphaVDev.disposeAndClear();
    mpPrevGraphics.clear();
    mpNextGraphics.clear();
    VclReferenceBase::dispose();
}

SalGraphics* OutputDevice::GetGraphics()
{
    DBG_TESTSOLARMUTEX();

    if (!mpGraphics && !AcquireGraphics())
        SAL_WARN("vcl.gdi", "No mpGraphics set");

    return mpGraphics;
}

SalGraphics const *OutputDevice::GetGraphics() const
{
    DBG_TESTSOLARMUTEX();

    if (!mpGraphics && !AcquireGraphics())
        SAL_WARN("vcl.gdi", "No mpGraphics set");

    return mpGraphics;
}

void OutputDevice::SetConnectMetaFile( GDIMetaFile* pMtf )
{
    mpMetaFile = pMtf;
}

void OutputDevice::SetSettings( const AllSettings& rSettings )
{
    *mxSettings = rSettings;

    if( mpAlphaVDev )
        mpAlphaVDev->SetSettings( rSettings );
}

SystemGraphicsData OutputDevice::GetSystemGfxData() const
{
    if (!mpGraphics && !AcquireGraphics())
        return SystemGraphicsData();

    return mpGraphics->GetGraphicsData();
}

#if ENABLE_CAIRO_CANVAS

bool OutputDevice::SupportsCairo() const
{
    if (!mpGraphics && !AcquireGraphics())
        return false;

    return mpGraphics->SupportsCairo();
}

cairo::SurfaceSharedPtr OutputDevice::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    if (!mpGraphics && !AcquireGraphics())
        return cairo::SurfaceSharedPtr();
    return mpGraphics->CreateSurface(rSurface);
}

cairo::SurfaceSharedPtr OutputDevice::CreateSurface(int x, int y, int width, int height) const
{
    if (!mpGraphics && !AcquireGraphics())
        return cairo::SurfaceSharedPtr();
    return mpGraphics->CreateSurface(*this, x, y, width, height);
}

cairo::SurfaceSharedPtr OutputDevice::CreateBitmapSurface(const BitmapSystemData& rData, const Size& rSize) const
{
    if (!mpGraphics && !AcquireGraphics())
        return cairo::SurfaceSharedPtr();
    return mpGraphics->CreateBitmapSurface(*this, rData, rSize);
}

css::uno::Any OutputDevice::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const
{
    if (!mpGraphics && !AcquireGraphics())
        return css::uno::Any();
    return mpGraphics->GetNativeSurfaceHandle(rSurface, rSize);
}

#endif // ENABLE_CAIRO_CANVAS

css::uno::Any OutputDevice::GetSystemGfxDataAny() const
{
    const SystemGraphicsData aSysData = GetSystemGfxData();
    css::uno::Sequence< sal_Int8 > aSeq( reinterpret_cast<sal_Int8 const *>(&aSysData),
                                                      aSysData.nSize );

    return css::uno::makeAny(aSeq);
}

void OutputDevice::SetRefPoint()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRefPointAction( Point(), false ) );

    mbRefPoint = false;
    maRefPoint.setX(0);
    maRefPoint.setY(0);

    if( mpAlphaVDev )
        mpAlphaVDev->SetRefPoint();
}

void OutputDevice::SetRefPoint( const Point& rRefPoint )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRefPointAction( rRefPoint, true ) );

    mbRefPoint = true;
    maRefPoint = rRefPoint;

    if( mpAlphaVDev )
        mpAlphaVDev->SetRefPoint( rRefPoint );
}

sal_uInt16 OutputDevice::GetBitCount() const
{
    // we need a graphics instance
    if ( !mpGraphics && !AcquireGraphics() )
        return 0;

    return mpGraphics->GetBitCount();
}

void OutputDevice::SetOutOffXPixel(long nOutOffX)
{
    mnOutOffX = nOutOffX;
}

void OutputDevice::SetOutOffYPixel(long nOutOffY)
{
    mnOutOffY = nOutOffY;
}

css::uno::Reference< css::awt::XGraphics > OutputDevice::CreateUnoGraphics()
{
    UnoWrapperBase* pWrapper = UnoWrapperBase::GetUnoWrapper();
    return pWrapper ? pWrapper->CreateGraphics( this ) : css::uno::Reference< css::awt::XGraphics >();
}

std::vector< VCLXGraphics* > *OutputDevice::CreateUnoGraphicsList()
{
    mpUnoGraphicsList = new std::vector< VCLXGraphics* >;
    return mpUnoGraphicsList;
}

// Helper public function

bool OutputDevice::SupportsOperation( OutDevSupportType eType ) const
{
    if( !mpGraphics && !AcquireGraphics() )
        return false;
    const bool bHasSupport = mpGraphics->supportsOperation( eType );
    return bHasSupport;
}

// Direct OutputDevice drawing public functions

void OutputDevice::DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                               const Point& rSrcPt,  const Size& rSrcSize )
{
    if( ImplIsRecordLayout() )
        return;

    if ( RasterOp::Invert == meRasterOp )
    {
        DrawRect( tools::Rectangle( rDestPt, rDestSize ) );
        return;
    }

    if ( mpMetaFile )
    {
        const Bitmap aBmp( GetBitmap( rSrcPt, rSrcSize ) );
        mpMetaFile->AddAction( new MetaBmpScaleAction( rDestPt, rDestSize, aBmp ) );
    }

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    long nSrcWidth   = ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    long nSrcHeight  = ImplLogicHeightToDevicePixel( rSrcSize.Height() );
    long nDestWidth  = ImplLogicWidthToDevicePixel( rDestSize.Width() );
    long nDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

    if (nSrcWidth && nSrcHeight && nDestWidth && nDestHeight)
    {
        SalTwoRect aPosAry(ImplLogicXToDevicePixel(rSrcPt.X()), ImplLogicYToDevicePixel(rSrcPt.Y()),
                           nSrcWidth, nSrcHeight,
                           ImplLogicXToDevicePixel(rDestPt.X()), ImplLogicYToDevicePixel(rDestPt.Y()),
                           nDestWidth, nDestHeight);

        const tools::Rectangle aSrcOutRect( Point( mnOutOffX, mnOutOffY ),
                                     Size( mnOutWidth, mnOutHeight ) );

        AdjustTwoRect( aPosAry, aSrcOutRect );

        if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
            mpGraphics->CopyBits( aPosAry, nullptr, this, nullptr );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawOutDev( rDestPt, rDestSize, rSrcPt, rSrcSize );
}

void OutputDevice::DrawOutDev( const Point& rDestPt, const Size& rDestSize,
                               const Point& rSrcPt,  const Size& rSrcSize,
                               const OutputDevice& rOutDev )
{
    if ( ImplIsRecordLayout() )
        return;

    if ( RasterOp::Invert == meRasterOp )
    {
        DrawRect( tools::Rectangle( rDestPt, rDestSize ) );
        return;
    }

    if ( mpMetaFile )
    {
        const Bitmap aBmp( rOutDev.GetBitmap( rSrcPt, rSrcSize ) );
        mpMetaFile->AddAction( new MetaBmpScaleAction( rDestPt, rDestSize, aBmp ) );
    }

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    SalTwoRect aPosAry(rOutDev.ImplLogicXToDevicePixel(rSrcPt.X()),
                       rOutDev.ImplLogicYToDevicePixel(rSrcPt.Y()),
                       rOutDev.ImplLogicWidthToDevicePixel(rSrcSize.Width()),
                       rOutDev.ImplLogicHeightToDevicePixel(rSrcSize.Height()),
                       ImplLogicXToDevicePixel(rDestPt.X()),
                       ImplLogicYToDevicePixel(rDestPt.Y()),
                       ImplLogicWidthToDevicePixel(rDestSize.Width()),
                       ImplLogicHeightToDevicePixel(rDestSize.Height()));

    if( mpAlphaVDev )
    {
        if( rOutDev.mpAlphaVDev )
        {
            // alpha-blend source over destination
            DrawBitmapEx( rDestPt, rDestSize, rOutDev.GetBitmapEx(rSrcPt, rSrcSize) );
        }
        else
        {
            drawOutDevDirect( &rOutDev, aPosAry );

            // #i32109#: make destination rectangle opaque - source has no alpha
            mpAlphaVDev->ImplFillOpaqueRectangle( tools::Rectangle(rDestPt, rDestSize) );
        }
    }
    else
    {
        if( rOutDev.mpAlphaVDev )
        {
            // alpha-blend source over destination
            DrawBitmapEx( rDestPt, rDestSize, rOutDev.GetBitmapEx(rSrcPt, rSrcSize) );
        }
        else
        {
            // no alpha at all, neither in source nor destination device
            drawOutDevDirect( &rOutDev, aPosAry );
        }
    }
}

void OutputDevice::CopyArea( const Point& rDestPt,
                             const Point& rSrcPt,  const Size& rSrcSize,
                             bool bWindowInvalidate )
{
    if ( ImplIsRecordLayout() )
        return;

    RasterOp eOldRop = GetRasterOp();
    SetRasterOp( RasterOp::OverPaint );

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics && !AcquireGraphics() )
        return;

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    long nSrcWidth   = ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    long nSrcHeight  = ImplLogicHeightToDevicePixel( rSrcSize.Height() );
    if (nSrcWidth && nSrcHeight)
    {
        SalTwoRect aPosAry(ImplLogicXToDevicePixel(rSrcPt.X()), ImplLogicYToDevicePixel(rSrcPt.Y()),
                           nSrcWidth, nSrcHeight,
                           ImplLogicXToDevicePixel(rDestPt.X()), ImplLogicYToDevicePixel(rDestPt.Y()),
                           nSrcWidth, nSrcHeight);

        const tools::Rectangle aSrcOutRect( Point( mnOutOffX, mnOutOffY ),
                                     Size( mnOutWidth, mnOutHeight ) );

        AdjustTwoRect( aPosAry, aSrcOutRect );

        CopyDeviceArea( aPosAry, bWindowInvalidate );
    }

    SetRasterOp( eOldRop );

    if( mpAlphaVDev )
        mpAlphaVDev->CopyArea( rDestPt, rSrcPt, rSrcSize, bWindowInvalidate );
}

// Direct OutputDevice drawing protected function

void OutputDevice::CopyDeviceArea( SalTwoRect& aPosAry, bool /*bWindowInvalidate*/)
{
    if (aPosAry.mnSrcWidth == 0 || aPosAry.mnSrcHeight == 0 || aPosAry.mnDestWidth == 0 || aPosAry.mnDestHeight == 0)
        return;

    aPosAry.mnDestWidth  = aPosAry.mnSrcWidth;
    aPosAry.mnDestHeight = aPosAry.mnSrcHeight;
    mpGraphics->CopyBits(aPosAry, nullptr, this, nullptr);
}

// Direct OutputDevice drawing private function

void OutputDevice::drawOutDevDirect( const OutputDevice* pSrcDev, SalTwoRect& rPosAry )
{
    SalGraphics* pSrcGraphics;

    if ( this == pSrcDev )
        pSrcGraphics = nullptr;
    else
    {
        if ( (GetOutDevType() != pSrcDev->GetOutDevType()) ||
             (GetOutDevType() != OUTDEV_WINDOW) )
        {
            if ( !pSrcDev->mpGraphics )
            {
                if ( !pSrcDev->AcquireGraphics() )
                    return;
            }
            pSrcGraphics = pSrcDev->mpGraphics;
        }
        else
        {
            if ( static_cast<vcl::Window*>(this)->mpWindowImpl->mpFrameWindow == static_cast<const vcl::Window*>(pSrcDev)->mpWindowImpl->mpFrameWindow )
                pSrcGraphics = nullptr;
            else
            {
                if ( !pSrcDev->mpGraphics )
                {
                    if ( !pSrcDev->AcquireGraphics() )
                        return;
                }
                pSrcGraphics = pSrcDev->mpGraphics;

                if ( !mpGraphics && !AcquireGraphics() )
                    return;
                SAL_WARN_IF( !mpGraphics || !pSrcDev->mpGraphics, "vcl.gdi",
                            "OutputDevice::DrawOutDev(): We need more than one Graphics" );
            }
        }
    }

    // #102532# Offset only has to be pseudo window offset
    const tools::Rectangle aSrcOutRect( Point( pSrcDev->mnOutOffX, pSrcDev->mnOutOffY ),
                                 Size( pSrcDev->mnOutWidth, pSrcDev->mnOutHeight ) );

    AdjustTwoRect( rPosAry, aSrcOutRect );

    if ( rPosAry.mnSrcWidth && rPosAry.mnSrcHeight && rPosAry.mnDestWidth && rPosAry.mnDestHeight )
    {
        // if this is no window, but pSrcDev is a window
        // mirroring may be required
        // because only windows have a SalGraphicsLayout
        // mirroring is performed here
        if( (GetOutDevType() != OUTDEV_WINDOW) && pSrcGraphics && (pSrcGraphics->GetLayout() & SalLayoutFlags::BiDiRtl) )
        {
            SalTwoRect aPosAry2 = rPosAry;
            pSrcGraphics->mirror( aPosAry2.mnSrcX, aPosAry2.mnSrcWidth, pSrcDev );
            mpGraphics->CopyBits( aPosAry2, pSrcGraphics, this, pSrcDev );
        }
        else
            mpGraphics->CopyBits( rPosAry, pSrcGraphics, this, pSrcDev );
    }
}

// Layout public functions

void OutputDevice::EnableRTL( bool bEnable )
{
    mbEnableRTL = bEnable;

    if( mpAlphaVDev )
        mpAlphaVDev->EnableRTL( bEnable );
}

bool OutputDevice::ImplIsAntiparallel() const
{
    bool bRet = false;
    if( AcquireGraphics() )
    {
        if( ( (mpGraphics->GetLayout() & SalLayoutFlags::BiDiRtl) && ! IsRTLEnabled() ) ||
            ( ! (mpGraphics->GetLayout() & SalLayoutFlags::BiDiRtl) && IsRTLEnabled() ) )
        {
            bRet = true;
        }
    }
    return bRet;
}

// note: the coordinates to be remirrored are in frame coordinates !

void    OutputDevice::ReMirror( Point &rPoint ) const
{
    rPoint.setX( mnOutOffX + mnOutWidth - 1 - rPoint.X() + mnOutOffX );
}
void    OutputDevice::ReMirror( tools::Rectangle &rRect ) const
{
    long nWidth = rRect.Right() - rRect.Left();

    //long lc_x = rRect.nLeft - mnOutOffX;    // normalize
    //lc_x = mnOutWidth - nWidth - 1 - lc_x;  // mirror
    //rRect.nLeft = lc_x + mnOutOffX;         // re-normalize

    rRect.SetLeft( mnOutOffX + mnOutWidth - nWidth - 1 - rRect.Left() + mnOutOffX );
    rRect.SetRight( rRect.Left() + nWidth );
}

void OutputDevice::ReMirror( vcl::Region &rRegion ) const
{
    RectangleVector aRectangles;
    rRegion.GetRegionRectangles(aRectangles);
    vcl::Region aMirroredRegion;

    for (auto & rectangle : aRectangles)
    {
        ReMirror(rectangle);
        aMirroredRegion.Union(rectangle);
    }

    rRegion = aMirroredRegion;

}

bool OutputDevice::HasMirroredGraphics() const
{
   return ( AcquireGraphics() && (mpGraphics->GetLayout() & SalLayoutFlags::BiDiRtl) );
}

bool OutputDevice::ImplIsRecordLayout() const
{
    if (!mpOutDevData)
        return false;

    return mpOutDevData->mpRecordLayout;
}

// EPS public function

bool OutputDevice::DrawEPS( const Point& rPoint, const Size& rSize,
                            const GfxLink& rGfxLink, GDIMetaFile* pSubst )
{
    bool bDrawn(true);

    if ( mpMetaFile )
    {
        GDIMetaFile aSubst;

        if( pSubst )
            aSubst = *pSubst;

        mpMetaFile->AddAction( new MetaEPSAction( rPoint, rSize, rGfxLink, aSubst ) );
    }

    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return bDrawn;

    if( mbOutputClipped )
        return bDrawn;

    tools::Rectangle aRect( ImplLogicToDevicePixel( tools::Rectangle( rPoint, rSize ) ) );

    if( !aRect.IsEmpty() )
    {
        // draw the real EPS graphics
        if( rGfxLink.GetData() && rGfxLink.GetDataSize() )
        {
            if( !mpGraphics && !AcquireGraphics() )
                return bDrawn;

            if( mbInitClipRegion )
                InitClipRegion();

            aRect.Justify();
            bDrawn = mpGraphics->DrawEPS( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(),
                         const_cast<sal_uInt8*>(rGfxLink.GetData()), rGfxLink.GetDataSize(), this );
        }

        // else draw the substitution graphics
        if( !bDrawn && pSubst )
        {
            GDIMetaFile* pOldMetaFile = mpMetaFile;

            mpMetaFile = nullptr;
            Graphic( *pSubst ).Draw( this, rPoint, rSize );
            mpMetaFile = pOldMetaFile;
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawEPS( rPoint, rSize, rGfxLink, pSubst );

    return bDrawn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
