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

#include <tools/debug.hxx>
#include <tools/poly.hxx>

#include <vcl/svapp.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/region.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/print.hxx>
#include <vcl/outdev.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>

#include <vcl/outdevstate.hxx>

#include <salgdi.hxx>
#include <sallayout.hxx>
#include <salframe.hxx>
#include <salvd.hxx>
#include <salprn.hxx>
#include <svdata.hxx>
#include <window.h>
#include <outdev.h>
#include <outdata.hxx>
#include <outdevstatestack.hxx>

#include "PhysicalFontCollection.hxx"

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/CanvasFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include <config_cairo_canvas.h>

#include <numeric>
#include <stack>

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

OutputDevice::OutputDevice() :
    mnRefCnt(1), // cf. VclPtrInstance and README.lifecycle
    maRegion(true),
    maFillColor( COL_WHITE ),
    maTextLineColor( COL_TRANSPARENT ),
    mxSettings( new AllSettings(Application::GetSettings()) )
{
    mpGraphics                      = NULL;
    mpUnoGraphicsList               = NULL;
    mpPrevGraphics                  = NULL;
    mpNextGraphics                  = NULL;
    mpMetaFile                      = NULL;
    mpFontEntry                     = NULL;
    mpFontCache                     = NULL;
    mpFontCollection                = NULL;
    mpGetDevFontList                = NULL;
    mpGetDevSizeList                = NULL;
    mpOutDevStateStack              = new OutDevStateStack;
    mpPDFWriter                     = NULL;
    mpAlphaVDev                     = NULL;
    mpExtOutDevData                 = NULL;
    mnOutOffX                       = 0;
    mnOutOffY                       = 0;
    mnOutWidth                      = 0;
    mnOutHeight                     = 0;
    mnDPIX                          = 0;
    mnDPIY                          = 0;
    mnDPIScaleFactor                = 1;
    mnTextOffX                      = 0;
    mnTextOffY                      = 0;
    mnOutOffOrigX                   = 0;
    mnOutOffLogicX                  = 0;
    mnOutOffOrigY                   = 0;
    mnOutOffLogicY                  = 0;
    mnEmphasisAscent                = 0;
    mnEmphasisDescent               = 0;
    mnDrawMode                      = DrawModeFlags::Default;
    mnTextLayoutMode                = TEXT_LAYOUT_DEFAULT;

    if( AllSettings::GetLayoutRTL() ) //#i84553# tip BiDi preference to RTL
        mnTextLayoutMode            = TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_TEXTORIGIN_LEFT;

    meOutDevType                    = OUTDEV_DONTKNOW;
    meOutDevViewType                = OUTDEV_VIEWTYPE_DONTKNOW;
    mbMap                           = false;
    mbMapIsDefault                  = true;
    mbClipRegion                    = false;
    mbBackground                    = false;
    mbOutput                        = true;
    mbDevOutput                     = false;
    mbOutputClipped                 = false;
    maTextColor                     = Color( COL_BLACK );
    maOverlineColor                 = Color( COL_TRANSPARENT );
    meTextAlign                     = maFont.GetAlign();
    meRasterOp                      = ROP_OVERPAINT;
    mnAntialiasing                  = AntialiasingFlags::NONE;
    meTextLanguage                  = 0;  // TODO: get default from configuration?
    mbLineColor                     = true;
    mbFillColor                     = true;
    mbInitLineColor                 = true;
    mbInitFillColor                 = true;
    mbInitFont                      = true;
    mbInitTextColor                 = true;
    mbInitClipRegion                = true;
    mbClipRegionSet                 = false;
    mbKerning                       = false;
    mbNewFont                       = true;
    mbTextLines                     = false;
    mbTextBackground                = false;
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
    maMapRes.mfOffsetX              = 0.0;
    maMapRes.mfOffsetY              = 0.0;
    maMapRes.mfScaleX               = 1.0;
    maMapRes.mfScaleY               = 1.0;
    // struct ImplThresholdRes
    maThresRes.mnThresLogToPixX     = 0;
    maThresRes.mnThresLogToPixY     = 0;
    maThresRes.mnThresPixToLogX     = 0;
    maThresRes.mnThresPixToLogY     = 0;

    // struct ImplOutDevData- see #i82615#
    mpOutDevData                    = new ImplOutDevData;
    mpOutDevData->mpRotateDev       = NULL;
    mpOutDevData->mpRecordLayout    = NULL;

    // #i75163#
    mpOutDevData->mpViewTransform   = NULL;
    mpOutDevData->mpInverseViewTransform = NULL;

    mbDisposed = false;
}

OutputDevice::~OutputDevice()
{
    disposeOnce();
}

void OutputDevice::disposeOnce()
{
    if ( mbDisposed )
        return;
    mbDisposed = true;

    // catch badness where our OutputDevice sub-class was not
    // wrapped safely in a VclPtr cosily.
    // FIXME: as/when we make our destructors all protected,
    // we should introduce this assert:
    //    assert( mnRefCnt > 0 );

    dispose();
}

void OutputDevice::dispose()
{
    if ( GetUnoGraphicsList() )
    {
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper( false );
        if ( pWrapper )
            pWrapper->ReleaseAllGraphics( this );
        delete mpUnoGraphicsList;
        mpUnoGraphicsList = NULL;
    }

    mpOutDevData->mpRotateDev.disposeAndClear();

    // #i75163#
    ImplInvalidateViewTransform();

    delete mpOutDevData;
    mpOutDevData = NULL;

    // for some reason, we haven't removed state from the stack properly
    if ( !mpOutDevStateStack->empty() )
    {
        SAL_WARN( "vcl.gdi", "OutputDevice::~OutputDevice(): OutputDevice::Push() calls != OutputDevice::Pop() calls" );
        while ( !mpOutDevStateStack->empty() )
        {
            mpOutDevStateStack->pop_back();
        }
    }
    delete mpOutDevStateStack;
    mpOutDevStateStack = NULL;

    // release the active font instance
    if( mpFontEntry )
        mpFontCache->Release( mpFontEntry );

    // remove cached results of GetDevFontList/GetDevSizeList
    // TODO: use smart pointers for them
    delete mpGetDevFontList;
    mpGetDevFontList = NULL;

    delete mpGetDevSizeList;
    mpGetDevSizeList = NULL;

    // release ImplFontCache specific to this OutputDevice
    // TODO: refcount ImplFontCache
    if( mpFontCache
    && (mpFontCache != ImplGetSVData()->maGDIData.mpScreenFontCache)
    && (ImplGetSVData()->maGDIData.mpScreenFontCache != NULL) )
    {
        delete mpFontCache;
        mpFontCache = NULL;
    }

    // release ImplFontList specific to this OutputDevice
    // TODO: refcount ImplFontList
    if( mpFontCollection
    && (mpFontCollection != ImplGetSVData()->maGDIData.mpScreenFontList)
    && (ImplGetSVData()->maGDIData.mpScreenFontList != NULL) )
    {
        mpFontCollection->Clear();
        delete mpFontCollection;
        mpFontCollection = NULL;
    }

    mpAlphaVDev.disposeAndClear();
}

SalGraphics* OutputDevice::GetGraphics()
{
    DBG_TESTSOLARMUTEX();

    if ( !mpGraphics )
    {
        if ( !AcquireGraphics() )
        {
            SAL_WARN("vcl", "No mpGraphics set");
        }
    }

    return mpGraphics;
}

SalGraphics const *OutputDevice::GetGraphics() const
{
    DBG_TESTSOLARMUTEX();

    if ( !mpGraphics )
    {
        if ( !AcquireGraphics() )
        {
            SAL_WARN("vcl", "No mpGraphics set");
        }
    }

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
    if ( !mpGraphics )
    {
        if ( !AcquireGraphics() )
            return SystemGraphicsData();
    }

    return mpGraphics->GetGraphicsData();
}

bool OutputDevice::SupportsCairo() const
{
#if ENABLE_CAIRO_CANVAS
    if (!mpGraphics)
    {
        if (!AcquireGraphics())
            return false;
    }

    return mpGraphics->SupportsCairo();
#else
    (void) this; // loplugin:staticmethods
    return false;
#endif
}

cairo::SurfaceSharedPtr OutputDevice::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    if (!mpGraphics)
    {
        if (!AcquireGraphics())
            return cairo::SurfaceSharedPtr();
    }
    return mpGraphics->CreateSurface(rSurface);
}

cairo::SurfaceSharedPtr OutputDevice::CreateSurface(int x, int y, int width, int height) const
{
    if (!mpGraphics)
    {
        if (!AcquireGraphics())
            return cairo::SurfaceSharedPtr();
    }
    return mpGraphics->CreateSurface(*this, x, y, width, height);
}

cairo::SurfaceSharedPtr OutputDevice::CreateBitmapSurface(const BitmapSystemData& rData, const Size& rSize) const
{
    if (!mpGraphics)
    {
        if (!AcquireGraphics())
            return cairo::SurfaceSharedPtr();
    }
    return mpGraphics->CreateBitmapSurface(*this, rData, rSize);
}

css::uno::Any OutputDevice::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const ::basegfx::B2ISize& rSize) const
{
    if (!mpGraphics)
    {
        if (!AcquireGraphics())
            return css::uno::Any();
    }
    return mpGraphics->GetNativeSurfaceHandle(rSurface, rSize);
}

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
    maRefPoint.X() = maRefPoint.Y() = 0L;

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
    if ( !mpGraphics )
    {
        if ( !AcquireGraphics() )
            return 0;
    }

    return (sal_uInt16)mpGraphics->GetBitCount();
}

void OutputDevice::SetOutOffXPixel(long nOutOffX)
{
    mnOutOffX = nOutOffX;
}

void OutputDevice::SetOutOffYPixel(long nOutOffY)
{
    mnOutOffY = nOutOffY;
}

sal_uLong OutputDevice::GetColorCount() const
{

    const sal_uInt16 nBitCount = GetBitCount();
    return( ( nBitCount > 31 ) ? ULONG_MAX : ( ( (sal_uLong) 1 ) << nBitCount) );
}

css::uno::Reference< css::rendering::XCanvas > OutputDevice::GetCanvas() const
{
    css::uno::Sequence< css::uno::Any > aArg(6);

    aArg[ 0 ] = css::uno::makeAny( reinterpret_cast<sal_Int64>(this) );
    aArg[ 2 ] = css::uno::makeAny( css::awt::Rectangle( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight ) );
    aArg[ 3 ] = css::uno::makeAny( sal_False );
    aArg[ 5 ] = GetSystemGfxDataAny();

    css::uno::Reference<css::uno::XComponentContext> xContext = comphelper::getProcessComponentContext();

    // Create canvas instance with window handle
    static css::uno::Reference<css::lang::XMultiComponentFactory > xCanvasFactory( css::rendering::CanvasFactory::create( xContext ) );

    css::uno::Reference<css::rendering::XCanvas> xCanvas;
    xCanvas.set(
        xCanvasFactory->createInstanceWithArgumentsAndContext(
            "com.sun.star.rendering.Canvas", aArg, xContext ),
        css::uno::UNO_QUERY );

    return xCanvas;
}

css::uno::Reference< css::awt::XGraphics > OutputDevice::CreateUnoGraphics()
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    return pWrapper ? pWrapper->CreateGraphics( this ) : css::uno::Reference< css::awt::XGraphics >();
}

std::vector< VCLXGraphics* > *OutputDevice::CreateUnoGraphicsList()
{
    mpUnoGraphicsList = new std::vector< VCLXGraphics* >();
    return mpUnoGraphicsList;
}

// Helper public function

bool OutputDevice::SupportsOperation( OutDevSupportType eType ) const
{
    if( !mpGraphics )
        if( !AcquireGraphics() )
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

    if ( ROP_INVERT == meRasterOp )
    {
        DrawRect( Rectangle( rDestPt, rDestSize ) );
        return;
    }

    if ( mpMetaFile )
    {
        const Bitmap aBmp( GetBitmap( rSrcPt, rSrcSize ) );
        mpMetaFile->AddAction( new MetaBmpScaleAction( rDestPt, rDestSize, aBmp ) );
    }

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics )
        if ( !AcquireGraphics() )
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

        const Rectangle aSrcOutRect( Point( mnOutOffX, mnOutOffY ),
                                     Size( mnOutWidth, mnOutHeight ) );

        AdjustTwoRect( aPosAry, aSrcOutRect );

        if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
            mpGraphics->CopyBits( aPosAry, NULL, this, NULL );
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

    if ( ROP_INVERT == meRasterOp )
    {
        DrawRect( Rectangle( rDestPt, rDestSize ) );
        return;
    }

    if ( mpMetaFile )
    {
        const Bitmap aBmp( rOutDev.GetBitmap( rSrcPt, rSrcSize ) );
        mpMetaFile->AddAction( new MetaBmpScaleAction( rDestPt, rDestSize, aBmp ) );
    }

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics )
        if ( !AcquireGraphics() )
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
            mpAlphaVDev->ImplFillOpaqueRectangle( Rectangle(rDestPt, rDestSize) );
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
    SetRasterOp( ROP_OVERPAINT );

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics )
        if ( !AcquireGraphics() )
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

        const Rectangle aSrcOutRect( Point( mnOutOffX, mnOutOffY ),
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
    mpGraphics->CopyBits(aPosAry, NULL, this, NULL);
}

// Direct OutputDevice drawing private function

void OutputDevice::drawOutDevDirect( const OutputDevice* pSrcDev, SalTwoRect& rPosAry )
{
    SalGraphics* pSrcGraphics;

    if ( this == pSrcDev )
        pSrcGraphics = NULL;
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
                pSrcGraphics = NULL;
            else
            {
                if ( !pSrcDev->mpGraphics )
                {
                    if ( !pSrcDev->AcquireGraphics() )
                        return;
                }
                pSrcGraphics = pSrcDev->mpGraphics;

                if ( !mpGraphics )
                {
                    if ( !AcquireGraphics() )
                        return;
                }
                DBG_ASSERT( mpGraphics && pSrcDev->mpGraphics,
                            "OutputDevice::DrawOutDev(): We need more than one Graphics" );
            }
        }
    }

    // #102532# Offset only has to be pseudo window offset
    const Rectangle aSrcOutRect( Point( pSrcDev->mnOutOffX, pSrcDev->mnOutOffY ),
                                 Size( pSrcDev->mnOutWidth, pSrcDev->mnOutHeight ) );

    AdjustTwoRect( rPosAry, aSrcOutRect );

    if ( rPosAry.mnSrcWidth && rPosAry.mnSrcHeight && rPosAry.mnDestWidth && rPosAry.mnDestHeight )
    {
        // --- RTL --- if this is no window, but pSrcDev is a window
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

// note: the coordinates to be remirrored are in frame coordiantes !

void    OutputDevice::ReMirror( Point &rPoint ) const
{
    rPoint.X() = mnOutOffX + mnOutWidth - 1 - rPoint.X() + mnOutOffX;
}
void    OutputDevice::ReMirror( Rectangle &rRect ) const
{
    long nWidth = rRect.Right() - rRect.Left();

    //long lc_x = rRect.nLeft - mnOutOffX;    // normalize
    //lc_x = mnOutWidth - nWidth - 1 - lc_x;  // mirror
    //rRect.nLeft = lc_x + mnOutOffX;         // re-normalize

    rRect.Left() = mnOutOffX + mnOutWidth - nWidth - 1 - rRect.Left() + mnOutOffX;
    rRect.Right() = rRect.Left() + nWidth;
}

void OutputDevice::ReMirror( vcl::Region &rRegion ) const
{
    RectangleVector aRectangles;
    rRegion.GetRegionRectangles(aRectangles);
    vcl::Region aMirroredRegion;

    for(RectangleVector::iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
    {
        ReMirror(*aRectIter);
        aMirroredRegion.Union(*aRectIter);
    }

    rRegion = aMirroredRegion;

}

bool OutputDevice::HasMirroredGraphics() const
{
   return ( AcquireGraphics() && (mpGraphics->GetLayout() & SalLayoutFlags::BiDiRtl) );
}

bool OutputDevice::ImplIsRecordLayout() const
{
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

    Rectangle aRect( ImplLogicToDevicePixel( Rectangle( rPoint, rSize ) ) );

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

            mpMetaFile = NULL;
            Graphic( *pSubst ).Draw( this, rPoint, rSize );
            mpMetaFile = pOldMetaFile;
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawEPS( rPoint, rSize, rGfxLink, pSubst );

    return bDrawn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
