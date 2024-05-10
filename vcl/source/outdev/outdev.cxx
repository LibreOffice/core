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
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>

#include <vcl/graph.hxx>
#include <vcl/lazydelete.hxx>
#include <vcl/metaact.hxx>
#include <vcl/toolkit/unowrap.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>

#include <ImplOutDevData.hxx>
#include <font/PhysicalFontFaceCollection.hxx>
#include <salgdi.hxx>
#include <window.h>

#include <com/sun/star/awt/DeviceCapability.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/rendering/CanvasFactory.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>

#ifdef DISABLE_DYNLOADING
// Linking all needed LO code into one .so/executable, these already
// exist in the tools library, so put them in the anonymous namespace
// here to avoid clash...
namespace {
#endif
#ifdef DISABLE_DYNLOADING
}
#endif

using namespace ::com::sun::star::uno;

// Begin initializer and accessor public functions

OutputDevice::OutputDevice(OutDevType eOutDevType) :
    meOutDevType(eOutDevType),
    maRegion(true),
    maFillColor( COL_WHITE ),
    maTextLineColor( COL_TRANSPARENT ),
    moSettings( Application::GetSettings() )
{
    mpGraphics                      = nullptr;
    mpUnoGraphicsList               = nullptr;
    mpPrevGraphics                  = nullptr;
    mpNextGraphics                  = nullptr;
    mpMetaFile                      = nullptr;
    mpFontInstance                  = nullptr;
    mpForcedFallbackInstance        = nullptr;
    mpFontFaceCollection            = nullptr;
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
    mnTextLayoutMode                = vcl::text::ComplexTextLayoutFlags::Default;

    if( AllSettings::GetLayoutRTL() ) //#i84553# tip BiDi preference to RTL
        mnTextLayoutMode            = vcl::text::ComplexTextLayoutFlags::BiDiRtl | vcl::text::ComplexTextLayoutFlags::TextOriginLeft;

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
    if ( !maOutDevStateStack.empty() )
        SAL_WARN( "vcl.gdi", "OutputDevice::~OutputDevice(): OutputDevice::Push() calls != OutputDevice::Pop() calls" );
    maOutDevStateStack.clear();

    // release the active font instance
    mpFontInstance.clear();
    mpForcedFallbackInstance.clear();

    // remove cached results of GetDevFontList/GetDevSizeList
    mpFontFaceCollection.reset();

    // release ImplFontCache specific to this OutputDevice
    mxFontCache.reset();

    // release ImplFontList specific to this OutputDevice
    mxFontCollection.reset();

    mpAlphaVDev.disposeAndClear();
    mpPrevGraphics.clear();
    mpNextGraphics.clear();
    VclReferenceBase::dispose();
}

bool OutputDevice::IsVirtual() const
{
    return false;
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
    *moSettings = rSettings;

    if( mpAlphaVDev )
        mpAlphaVDev->SetSettings( rSettings );
}

SystemGraphicsData OutputDevice::GetSystemGfxData() const
{
    if (!mpGraphics && !AcquireGraphics())
        return SystemGraphicsData();
    assert(mpGraphics);

    return mpGraphics->GetGraphicsData();
}

OUString OutputDevice::GetRenderBackendName() const
{
    if (!mpGraphics && !AcquireGraphics())
        return {};
    assert(mpGraphics);

    return mpGraphics->getRenderBackendName();
}

#if ENABLE_CAIRO_CANVAS

bool OutputDevice::SupportsCairo() const
{
    if (!mpGraphics && !AcquireGraphics())
        return false;
    assert(mpGraphics);

    return mpGraphics->SupportsCairo();
}

cairo::SurfaceSharedPtr OutputDevice::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    if (!mpGraphics && !AcquireGraphics())
        return cairo::SurfaceSharedPtr();
    assert(mpGraphics);
    return mpGraphics->CreateSurface(rSurface);
}

cairo::SurfaceSharedPtr OutputDevice::CreateSurface(int x, int y, int width, int height) const
{
    if (!mpGraphics && !AcquireGraphics())
        return cairo::SurfaceSharedPtr();
    assert(mpGraphics);
    return mpGraphics->CreateSurface(*this, x, y, width, height);
}

cairo::SurfaceSharedPtr OutputDevice::CreateBitmapSurface(const BitmapSystemData& rData, const Size& rSize) const
{
    if (!mpGraphics && !AcquireGraphics())
        return cairo::SurfaceSharedPtr();
    assert(mpGraphics);
    return mpGraphics->CreateBitmapSurface(*this, rData, rSize);
}

css::uno::Any OutputDevice::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const
{
    if (!mpGraphics && !AcquireGraphics())
        return css::uno::Any();
    assert(mpGraphics);
    return mpGraphics->GetNativeSurfaceHandle(rSurface, rSize);
}

#endif // ENABLE_CAIRO_CANVAS

css::uno::Any OutputDevice::GetSystemGfxDataAny() const
{
    const SystemGraphicsData aSysData = GetSystemGfxData();
    css::uno::Sequence< sal_Int8 > aSeq( reinterpret_cast<sal_Int8 const *>(&aSysData),
                                                      aSysData.nSize );

    return css::uno::Any(aSeq);
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

void OutputDevice::SetRasterOp( RasterOp eRasterOp )
{
    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRasterOpAction( eRasterOp ) );

    if ( meRasterOp != eRasterOp )
    {
        meRasterOp = eRasterOp;
        mbInitLineColor = mbInitFillColor = true;

        if( mpGraphics || AcquireGraphics() )
        {
            assert(mpGraphics);
            mpGraphics->SetXORMode( (RasterOp::Invert == meRasterOp) || (RasterOp::Xor == meRasterOp), RasterOp::Invert == meRasterOp );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetRasterOp( eRasterOp );
}

void OutputDevice::EnableOutput( bool bEnable )
{
    mbOutput = bEnable;

    if( mpAlphaVDev )
        mpAlphaVDev->EnableOutput( bEnable );
}

void OutputDevice::SetAntialiasing( AntialiasingFlags nMode )
{
    if ( mnAntialiasing != nMode )
    {
        mnAntialiasing = nMode;
        mbInitFont = true;

        if (mpGraphics)
            mpGraphics->setAntiAlias(bool(mnAntialiasing & AntialiasingFlags::Enable));
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetAntialiasing( nMode );
}

void OutputDevice::SetDrawMode(DrawModeFlags nDrawMode)
{
    mnDrawMode = nDrawMode;

    if (mpAlphaVDev)
        mpAlphaVDev->SetDrawMode(nDrawMode);
}

sal_uInt16 OutputDevice::GetBitCount() const
{
    // we need a graphics instance
    if ( !mpGraphics && !AcquireGraphics() )
        return 0;
    assert(mpGraphics);

    return mpGraphics->GetBitCount();
}

void OutputDevice::SetOutOffXPixel(tools::Long nOutOffX)
{
    mnOutOffX = nOutOffX;
}

void OutputDevice::SetOutOffYPixel(tools::Long nOutOffY)
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
    assert(mpGraphics);
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
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    tools::Long nSrcWidth   = ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    tools::Long nSrcHeight  = ImplLogicHeightToDevicePixel( rSrcSize.Height() );
    tools::Long nDestWidth  = ImplLogicWidthToDevicePixel( rDestSize.Width() );
    tools::Long nDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

    if (nSrcWidth && nSrcHeight && nDestWidth && nDestHeight)
    {
        SalTwoRect aPosAry(ImplLogicXToDevicePixel(rSrcPt.X()), ImplLogicYToDevicePixel(rSrcPt.Y()),
                           nSrcWidth, nSrcHeight,
                           ImplLogicXToDevicePixel(rDestPt.X()), ImplLogicYToDevicePixel(rDestPt.Y()),
                           nDestWidth, nDestHeight);

        AdjustTwoRect( aPosAry, GetOutputRectPixel() );

        if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
            mpGraphics->CopyBits(aPosAry, *this);
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
        if (rOutDev.mpAlphaVDev)
        {
            const BitmapEx aBmpEx(rOutDev.GetBitmapEx(rSrcPt, rSrcSize));
            mpMetaFile->AddAction(new MetaBmpExScaleAction(rDestPt, rDestSize, aBmpEx));
        }
        else
        {
            const Bitmap aBmp(rOutDev.GetBitmap(rSrcPt, rSrcSize));
            mpMetaFile->AddAction(new MetaBmpScaleAction(rDestPt, rDestSize, aBmp));
        }
    }

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics && !AcquireGraphics() )
        return;
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    if (rOutDev.mpAlphaVDev)
    {
        // alpha-blend source over destination
        DrawBitmapEx(rDestPt, rDestSize, rOutDev.GetBitmapEx(rSrcPt, rSrcSize));
    }
    else
    {
        SalTwoRect aPosAry(rOutDev.ImplLogicXToDevicePixel(rSrcPt.X()),
                           rOutDev.ImplLogicYToDevicePixel(rSrcPt.Y()),
                           rOutDev.ImplLogicWidthToDevicePixel(rSrcSize.Width()),
                           rOutDev.ImplLogicHeightToDevicePixel(rSrcSize.Height()),
                           ImplLogicXToDevicePixel(rDestPt.X()),
                           ImplLogicYToDevicePixel(rDestPt.Y()),
                           ImplLogicWidthToDevicePixel(rDestSize.Width()),
                           ImplLogicHeightToDevicePixel(rDestSize.Height()));

        drawOutDevDirect(rOutDev, aPosAry);

        // #i32109#: make destination rectangle opaque - source has no alpha
        if (mpAlphaVDev)
            mpAlphaVDev->ImplFillOpaqueRectangle(tools::Rectangle(rDestPt, rDestSize));
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
    assert(mpGraphics);

    if ( mbInitClipRegion )
        InitClipRegion();

    if ( mbOutputClipped )
        return;

    tools::Long nSrcWidth   = ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    tools::Long nSrcHeight  = ImplLogicHeightToDevicePixel( rSrcSize.Height() );
    if (nSrcWidth && nSrcHeight)
    {
        SalTwoRect aPosAry(ImplLogicXToDevicePixel(rSrcPt.X()), ImplLogicYToDevicePixel(rSrcPt.Y()),
                           nSrcWidth, nSrcHeight,
                           ImplLogicXToDevicePixel(rDestPt.X()), ImplLogicYToDevicePixel(rDestPt.Y()),
                           nSrcWidth, nSrcHeight);

        AdjustTwoRect( aPosAry, GetOutputRectPixel() );

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
    mpGraphics->CopyBits(aPosAry, *this);
}

// Direct OutputDevice drawing private function
void OutputDevice::drawOutDevDirect(const OutputDevice& rSrcDev, SalTwoRect& rPosAry)
{
    SalGraphics* pSrcGraphics;
    if (const OutputDevice* pCheckedSrc = DrawOutDevDirectCheck(rSrcDev))
    {
        if (!pCheckedSrc->mpGraphics && !pCheckedSrc->AcquireGraphics())
            return;
        pSrcGraphics = pCheckedSrc->mpGraphics;
    }
    else
        pSrcGraphics = nullptr;

    if (!mpGraphics && !AcquireGraphics())
        return;
    assert(mpGraphics);

    // #102532# Offset only has to be pseudo window offset

    AdjustTwoRect( rPosAry, rSrcDev.GetOutputRectPixel() );

    if ( rPosAry.mnSrcWidth && rPosAry.mnSrcHeight && rPosAry.mnDestWidth && rPosAry.mnDestHeight )
    {
        // if this is no window, but rSrcDev is a window
        // mirroring may be required
        // because only windows have a SalGraphicsLayout
        // mirroring is performed here
        DrawOutDevDirectProcess(rSrcDev, rPosAry, pSrcGraphics);
    }
}

const OutputDevice* OutputDevice::DrawOutDevDirectCheck(const OutputDevice& rSrcDev) const
{
    return this == &rSrcDev ? nullptr : &rSrcDev;
}

void OutputDevice::DrawOutDevDirectProcess(const OutputDevice& rSrcDev, SalTwoRect& rPosAry, SalGraphics* pSrcGraphics)
{
    if( pSrcGraphics && (pSrcGraphics->GetLayout() & SalLayoutFlags::BiDiRtl) )
    {
        SalTwoRect aPosAry2 = rPosAry;
        pSrcGraphics->mirror( aPosAry2.mnSrcX, aPosAry2.mnSrcWidth, rSrcDev );
        mpGraphics->CopyBits( aPosAry2, *pSrcGraphics, *this, rSrcDev );
        return;
    }
    if (pSrcGraphics)
        mpGraphics->CopyBits( rPosAry, *pSrcGraphics, *this, rSrcDev );
    else
        mpGraphics->CopyBits( rPosAry, *this );
}

tools::Rectangle OutputDevice::GetBackgroundComponentBounds() const
{
    return tools::Rectangle( Point( 0, 0 ), GetOutputSizePixel() );
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
    tools::Long nWidth = rRect.Right() - rRect.Left();

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

css::awt::DeviceInfo OutputDevice::GetCommonDeviceInfo(Size const& rDevSz) const
{
    css::awt::DeviceInfo aInfo;

    aInfo.Width = rDevSz.Width();
    aInfo.Height = rDevSz.Height();

    Size aTmpSz = LogicToPixel(Size(1000, 1000), MapMode(MapUnit::MapMM));
    aInfo.PixelPerMeterX = aTmpSz.Width();
    aInfo.PixelPerMeterY = aTmpSz.Height();
    aInfo.BitsPerPixel = GetBitCount();

    aInfo.Capabilities = css::awt::DeviceCapability::RASTEROPERATIONS |
        css::awt::DeviceCapability::GETBITS;

    return aInfo;
}

css::awt::DeviceInfo OutputDevice::GetDeviceInfo() const
{
    css::awt::DeviceInfo aInfo = GetCommonDeviceInfo(GetOutputSizePixel());

    aInfo.LeftInset = 0;
    aInfo.TopInset = 0;
    aInfo.RightInset = 0;
    aInfo.BottomInset = 0;

    return aInfo;
}

Reference< css::rendering::XCanvas > OutputDevice::GetCanvas() const
{
    // try to retrieve hard reference from weak member
    Reference< css::rendering::XCanvas > xCanvas( mxCanvas );
    // canvas still valid? Then we're done.
    if( xCanvas.is() )
        return xCanvas;
    xCanvas = ImplGetCanvas( false );
    mxCanvas = xCanvas;
    return xCanvas;
}

Reference< css::rendering::XSpriteCanvas > OutputDevice::GetSpriteCanvas() const
{
    Reference< css::rendering::XCanvas > xCanvas( mxCanvas );
    Reference< css::rendering::XSpriteCanvas > xSpriteCanvas( xCanvas, UNO_QUERY );
    if( xSpriteCanvas.is() )
        return xSpriteCanvas;
    xCanvas = ImplGetCanvas( true );
    mxCanvas = xCanvas;
    return Reference< css::rendering::XSpriteCanvas >( xCanvas, UNO_QUERY );
}

// Generic implementation, Window will override.
com::sun::star::uno::Reference< css::rendering::XCanvas > OutputDevice::ImplGetCanvas( bool bSpriteCanvas ) const
{
    /* Arguments:
       0: ptr to creating instance (Window or VirtualDevice)
       1: current bounds of creating instance
       2: bool, denoting always on top state for Window (always false for VirtualDevice)
       3: XWindow for creating Window (or empty for VirtualDevice)
       4: SystemGraphicsData as a streamed Any
     */
    Sequence< Any > aArg{
        Any(reinterpret_cast<sal_Int64>(this)),
        Any(css::awt::Rectangle( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight )),
        Any(false),
        Any(Reference< css::awt::XWindow >()),
        GetSystemGfxDataAny()
    };

    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();

    static vcl::DeleteUnoReferenceOnDeinit<css::lang::XMultiComponentFactory> xStaticCanvasFactory(
        css::rendering::CanvasFactory::create( xContext ) );
    Reference<css::lang::XMultiComponentFactory> xCanvasFactory(xStaticCanvasFactory.get());
    Reference< css::rendering::XCanvas > xCanvas;

    if(xCanvasFactory.is())
    {
        xCanvas.set( xCanvasFactory->createInstanceWithArgumentsAndContext(
                         bSpriteCanvas ?
                         u"com.sun.star.rendering.SpriteCanvas"_ustr :
                         u"com.sun.star.rendering.Canvas"_ustr,
                         aArg,
                         xContext ),
                     UNO_QUERY );
    }

    // no factory??? Empty reference, then.
    return xCanvas;
}

void OutputDevice::ImplDisposeCanvas()
{
    css::uno::Reference< css::rendering::XCanvas > xCanvas( mxCanvas );
    if( xCanvas.is() )
    {
        css::uno::Reference< css::lang::XComponent >  xCanvasComponent( xCanvas, css::uno::UNO_QUERY );
        if( xCanvasComponent.is() )
            xCanvasComponent->dispose();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
