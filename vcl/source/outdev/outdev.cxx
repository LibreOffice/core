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

#include <svsys.h>
#include <salgdi.hxx>
#include <sallayout.hxx>
#include <salframe.hxx>
#include <salvd.hxx>
#include <salprn.hxx>
#include <svdata.hxx>
#include <window.h>
#include <outdev.h>
#include <outdata.hxx>

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


bool OutputDevice::ImplIsAntiparallel() const
{
    bool bRet = false;
    if( AcquireGraphics() )
    {
        if( ( (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) && ! IsRTLEnabled() ) ||
            ( ! (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) && IsRTLEnabled() ) )
        {
            bRet = true;
        }
    }
    return bRet;
}

OutputDevice::OutputDevice() :
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
    mpOutDevStateStack              = new boost::ptr_deque<OutDevState>();
    mpOutDevData                    = NULL;
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
    mnDrawMode                      = 0;
    mnTextLayoutMode                = TEXT_LAYOUT_DEFAULT;

    if( Application::GetSettings().GetLayoutRTL() ) //#i84553# tip BiDi preference to RTL
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
    mnAntialiasing                  = 0;
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
    mpOutDevData                    = new ImplOutDevData;
    mpOutDevData->mpRotateDev       = NULL;
    mpOutDevData->mpRecordLayout    = NULL;

    // #i75163#
    mpOutDevData->mpViewTransform   = NULL;
    mpOutDevData->mpInverseViewTransform = NULL;
}

OutputDevice::~OutputDevice()
{

    if ( GetUnoGraphicsList() )
    {
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper( false );
        if ( pWrapper )
            pWrapper->ReleaseAllGraphics( this );
        delete mpUnoGraphicsList;
        mpUnoGraphicsList = NULL;
    }

    if ( mpOutDevData )
    {
        if ( mpOutDevData->mpRotateDev )
            delete mpOutDevData->mpRotateDev;

        // #i75163#
        ImplInvalidateViewTransform();

        delete mpOutDevData;
    }

    // for some reason, we haven't removed state from the stack properly
    if ( !mpOutDevStateStack->empty() )
    {
        SAL_WARN( "vcl.gdi", "OutputDevice::~OutputDevice(): OutputDevice::Push() calls != OutputDevice::Pop() calls" );
        while ( !mpOutDevStateStack->empty() )
        {
            mpOutDevStateStack->pop_back();
        }
    }

    // release the active font instance
    if( mpFontEntry )
        mpFontCache->Release( mpFontEntry );

    // remove cached results of GetDevFontList/GetDevSizeList
    // TODO: use smart pointers for them
    if( mpGetDevFontList )
        delete mpGetDevFontList;

    if( mpGetDevSizeList )
        delete mpGetDevSizeList;

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

    delete mpAlphaVDev;
}

void OutputDevice::Push( sal_uInt16 nFlags )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPushAction( nFlags ) );

    OutDevState* pState = new OutDevState;

    pState->mnFlags = nFlags;

    if ( nFlags & PUSH_LINECOLOR )
    {
        if ( mbLineColor )
            pState->mpLineColor = new Color( maLineColor );
        else
            pState->mpLineColor = NULL;
    }
    if ( nFlags & PUSH_FILLCOLOR )
    {
        if ( mbFillColor )
            pState->mpFillColor = new Color( maFillColor );
        else
            pState->mpFillColor = NULL;
    }
    if ( nFlags & PUSH_FONT )
        pState->mpFont = new Font( maFont );
    if ( nFlags & PUSH_TEXTCOLOR )
        pState->mpTextColor = new Color( GetTextColor() );
    if ( nFlags & PUSH_TEXTFILLCOLOR )
    {
        if ( IsTextFillColor() )
            pState->mpTextFillColor = new Color( GetTextFillColor() );
        else
            pState->mpTextFillColor = NULL;
    }
    if ( nFlags & PUSH_TEXTLINECOLOR )
    {
        if ( IsTextLineColor() )
            pState->mpTextLineColor = new Color( GetTextLineColor() );
        else
            pState->mpTextLineColor = NULL;
    }
    if ( nFlags & PUSH_OVERLINECOLOR )
    {
        if ( IsOverlineColor() )
            pState->mpOverlineColor = new Color( GetOverlineColor() );
        else
            pState->mpOverlineColor = NULL;
    }
    if ( nFlags & PUSH_TEXTALIGN )
        pState->meTextAlign = GetTextAlign();
    if( nFlags & PUSH_TEXTLAYOUTMODE )
        pState->mnTextLayoutMode = GetLayoutMode();
    if( nFlags & PUSH_TEXTLANGUAGE )
        pState->meTextLanguage = GetDigitLanguage();
    if ( nFlags & PUSH_RASTEROP )
        pState->meRasterOp = GetRasterOp();
    if ( nFlags & PUSH_MAPMODE )
    {
        pState->mpMapMode = new MapMode( maMapMode );
        pState->mbMapActive = mbMap;
    }
    if ( nFlags & PUSH_CLIPREGION )
    {
        if ( mbClipRegion )
            pState->mpClipRegion = new Region( maRegion );
        else
            pState->mpClipRegion = NULL;
    }
    if ( nFlags & PUSH_REFPOINT )
    {
        if ( mbRefPoint )
            pState->mpRefPoint = new Point( maRefPoint );
        else
            pState->mpRefPoint = NULL;
    }

    mpOutDevStateStack->push_back( pState );

    if( mpAlphaVDev )
        mpAlphaVDev->Push();
}

void OutputDevice::Pop()
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPopAction() );

    GDIMetaFile* pOldMetaFile = mpMetaFile;
    mpMetaFile = NULL;

    if ( mpOutDevStateStack->empty() )
    {
        SAL_WARN( "vcl.gdi", "OutputDevice::Pop() without OutputDevice::Push()" );
        return;
    }
    const OutDevState& rState = mpOutDevStateStack->back();

    if( mpAlphaVDev )
        mpAlphaVDev->Pop();

    if ( rState.mnFlags & PUSH_LINECOLOR )
    {
        if ( rState.mpLineColor )
            SetLineColor( *rState.mpLineColor );
        else
            SetLineColor();
    }

    if ( rState.mnFlags & PUSH_FILLCOLOR )
    {
        if ( rState.mpFillColor )
            SetFillColor( *rState.mpFillColor );
        else
            SetFillColor();
    }

    if ( rState.mnFlags & PUSH_FONT )
        SetFont( *rState.mpFont );

    if ( rState.mnFlags & PUSH_TEXTCOLOR )
        SetTextColor( *rState.mpTextColor );

    if ( rState.mnFlags & PUSH_TEXTFILLCOLOR )
    {
        if ( rState.mpTextFillColor )
            SetTextFillColor( *rState.mpTextFillColor );
        else
            SetTextFillColor();
    }

    if ( rState.mnFlags & PUSH_TEXTLINECOLOR )
    {
        if ( rState.mpTextLineColor )
            SetTextLineColor( *rState.mpTextLineColor );
        else
            SetTextLineColor();
    }

    if ( rState.mnFlags & PUSH_OVERLINECOLOR )
    {
        if ( rState.mpOverlineColor )
            SetOverlineColor( *rState.mpOverlineColor );
        else
            SetOverlineColor();
    }

    if ( rState.mnFlags & PUSH_TEXTALIGN )
        SetTextAlign( rState.meTextAlign );

    if( rState.mnFlags & PUSH_TEXTLAYOUTMODE )
        SetLayoutMode( rState.mnTextLayoutMode );

    if( rState.mnFlags & PUSH_TEXTLANGUAGE )
        SetDigitLanguage( rState.meTextLanguage );

    if ( rState.mnFlags & PUSH_RASTEROP )
        SetRasterOp( rState.meRasterOp );

    if ( rState.mnFlags & PUSH_MAPMODE )
    {
        if ( rState.mpMapMode )
            SetMapMode( *rState.mpMapMode );
        else
            SetMapMode();
        mbMap = rState.mbMapActive;
    }

    if ( rState.mnFlags & PUSH_CLIPREGION )
        SetDeviceClipRegion( rState.mpClipRegion );

    if ( rState.mnFlags & PUSH_REFPOINT )
    {
        if ( rState.mpRefPoint )
            SetRefPoint( *rState.mpRefPoint );
        else
            SetRefPoint();
    }

    mpOutDevStateStack->pop_back();

    mpMetaFile = pOldMetaFile;
}

bool OutputDevice::supportsOperation( OutDevSupportType eType ) const
{
    if( !mpGraphics )
        if( !AcquireGraphics() )
            return false;
    const bool bHasSupport = mpGraphics->supportsOperation( eType );
    return bHasSupport;
}

void OutputDevice::EnableRTL( bool bEnable )
{
    mbEnableRTL = bEnable;

    if( mpAlphaVDev )
        mpAlphaVDev->EnableRTL( bEnable );
}

bool OutputDevice::HasMirroredGraphics() const
{
   return ( AcquireGraphics() && (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) );
}

// note: the coordiantes to be remirrored are in frame coordiantes !

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
void    OutputDevice::ReMirror( Region &rRegion ) const
{
    RectangleVector aRectangles;
    rRegion.GetRegionRectangles(aRectangles);
    Region aMirroredRegion;

    for(RectangleVector::iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
    {
        ReMirror(*aRectIter);
        aMirroredRegion.Union(*aRectIter);
    }

    rRegion = aMirroredRegion;

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

void OutputDevice::ImplReleaseFonts()
{
    mpGraphics->ReleaseFonts();
    mbNewFont = true;
    mbInitFont = true;

    if ( mpFontEntry )
    {
        mpFontCache->Release( mpFontEntry );
        mpFontEntry = NULL;
    }

    if ( mpGetDevFontList )
    {
        delete mpGetDevFontList;
        mpGetDevFontList = NULL;
    }

    if ( mpGetDevSizeList )
    {
        delete mpGetDevSizeList;
        mpGetDevSizeList = NULL;
    }
}

// #i75163#
void OutputDevice::ImplInvalidateViewTransform()
{
    if(mpOutDevData)
    {
        if(mpOutDevData->mpViewTransform)
        {
            delete mpOutDevData->mpViewTransform;
            mpOutDevData->mpViewTransform = NULL;
        }

        if(mpOutDevData->mpInverseViewTransform)
        {
            delete mpOutDevData->mpInverseViewTransform;
            mpOutDevData->mpInverseViewTransform = NULL;
        }
    }
}

bool OutputDevice::ImplIsRecordLayout() const
{
    return mpOutDevData && mpOutDevData->mpRecordLayout;
}

void OutputDevice::ImplDrawOutDevDirect( const OutputDevice* pSrcDev, SalTwoRect& rPosAry )
{
    SalGraphics*        pGraphics2;

    if ( this == pSrcDev )
        pGraphics2 = NULL;
    else
    {
        if ( (GetOutDevType() != pSrcDev->GetOutDevType()) ||
             (GetOutDevType() != OUTDEV_WINDOW) )
        {
            if ( !pSrcDev->mpGraphics )
            {
                if ( !((OutputDevice*)pSrcDev)->AcquireGraphics() )
                    return;
            }
            pGraphics2 = pSrcDev->mpGraphics;
        }
        else
        {
            if ( ((Window*)this)->mpWindowImpl->mpFrameWindow == ((Window*)pSrcDev)->mpWindowImpl->mpFrameWindow )
                pGraphics2 = NULL;
            else
            {
                if ( !pSrcDev->mpGraphics )
                {
                    if ( !((OutputDevice*)pSrcDev)->AcquireGraphics() )
                        return;
                }
                pGraphics2 = pSrcDev->mpGraphics;

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
        if( (GetOutDevType() != OUTDEV_WINDOW) && pGraphics2 && (pGraphics2->GetLayout() & SAL_LAYOUT_BIDI_RTL) )
        {
            SalTwoRect aPosAry2 = rPosAry;
            pGraphics2->mirror( aPosAry2.mnSrcX, aPosAry2.mnSrcWidth, pSrcDev );
            mpGraphics->CopyBits( aPosAry2, pGraphics2, this, pSrcDev );
        }
        else
            mpGraphics->CopyBits( rPosAry, pGraphics2, this, pSrcDev );
    }
}

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

    SalTwoRect aPosAry;
    aPosAry.mnSrcWidth   = ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    aPosAry.mnSrcHeight  = ImplLogicHeightToDevicePixel( rSrcSize.Height() );
    aPosAry.mnDestWidth  = ImplLogicWidthToDevicePixel( rDestSize.Width() );
    aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

    if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight && aPosAry.mnDestWidth && aPosAry.mnDestHeight )
    {
        aPosAry.mnSrcX       = ImplLogicXToDevicePixel( rSrcPt.X() );
        aPosAry.mnSrcY       = ImplLogicYToDevicePixel( rSrcPt.Y() );
        aPosAry.mnDestX      = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY      = ImplLogicYToDevicePixel( rDestPt.Y() );

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

    SalTwoRect aPosAry;
    aPosAry.mnSrcX       = rOutDev.ImplLogicXToDevicePixel( rSrcPt.X() );
    aPosAry.mnSrcY       = rOutDev.ImplLogicYToDevicePixel( rSrcPt.Y() );
    aPosAry.mnSrcWidth   = rOutDev.ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    aPosAry.mnSrcHeight  = rOutDev.ImplLogicHeightToDevicePixel( rSrcSize.Height() );
    aPosAry.mnDestX      = ImplLogicXToDevicePixel( rDestPt.X() );
    aPosAry.mnDestY      = ImplLogicYToDevicePixel( rDestPt.Y() );
    aPosAry.mnDestWidth  = ImplLogicWidthToDevicePixel( rDestSize.Width() );
    aPosAry.mnDestHeight = ImplLogicHeightToDevicePixel( rDestSize.Height() );

    if( mpAlphaVDev )
    {
        if( rOutDev.mpAlphaVDev )
        {
            // alpha-blend source over destination
            DrawBitmapEx( rDestPt, rDestSize, rOutDev.GetBitmapEx(rSrcPt, rSrcSize) );
        }
        else
        {
            ImplDrawOutDevDirect( &rOutDev, aPosAry );

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
            ImplDrawOutDevDirect( &rOutDev, aPosAry );
        }
    }
}

void OutputDevice::CopyArea( const Point& rDestPt,
                             const Point& rSrcPt,  const Size& rSrcSize,
                             sal_uInt16 nFlags )
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

    SalTwoRect aPosAry;
    aPosAry.mnSrcWidth   = ImplLogicWidthToDevicePixel( rSrcSize.Width() );
    aPosAry.mnSrcHeight  = ImplLogicHeightToDevicePixel( rSrcSize.Height() );

    if ( aPosAry.mnSrcWidth && aPosAry.mnSrcHeight )
    {
        aPosAry.mnSrcX       = ImplLogicXToDevicePixel( rSrcPt.X() );
        aPosAry.mnSrcY       = ImplLogicYToDevicePixel( rSrcPt.Y() );
        aPosAry.mnDestX      = ImplLogicXToDevicePixel( rDestPt.X() );
        aPosAry.mnDestY      = ImplLogicYToDevicePixel( rDestPt.Y() );
        aPosAry.mnDestWidth  = aPosAry.mnSrcWidth;
        aPosAry.mnDestHeight = aPosAry.mnSrcHeight;

        const Rectangle aSrcOutRect( Point( mnOutOffX, mnOutOffY ),
                                     Size( mnOutWidth, mnOutHeight ) );

        AdjustTwoRect( aPosAry, aSrcOutRect );

        CopyAreaFinal ( aPosAry, nFlags );
    }

    SetRasterOp( eOldRop );

    if( mpAlphaVDev )
        mpAlphaVDev->CopyArea( rDestPt, rSrcPt, rSrcSize, nFlags );
}

void OutputDevice::CopyAreaFinal( SalTwoRect& aPosAry, sal_uInt32 /*nFlags*/)
{
    if (aPosAry.mnSrcWidth == 0 || aPosAry.mnSrcHeight == 0 || aPosAry.mnDestWidth == 0 || aPosAry.mnDestHeight == 0)
        return;

    aPosAry.mnDestWidth  = aPosAry.mnSrcWidth;
    aPosAry.mnDestHeight = aPosAry.mnSrcHeight;
    mpGraphics->CopyBits(aPosAry, NULL, this, NULL);
}

void OutputDevice::ImplDrawFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                     const OutputDevice& rOutDev, const Region& rRegion )
{

    GDIMetaFile*    pOldMetaFile = mpMetaFile;
    bool            bOldMap = mbMap;
    RasterOp        eOldROP = GetRasterOp();
    mpMetaFile = NULL;
    mbMap = false;
    SetRasterOp( ROP_OVERPAINT );

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics )
    {
        if ( !AcquireGraphics() )
            return;
    }

    // ClipRegion zuruecksetzen
    if ( rRegion.IsNull() )
        mpGraphics->ResetClipRegion();
    else
        SelectClipRegion( rRegion );

    SalTwoRect aPosAry;
    aPosAry.mnSrcX       = rDevPt.X();
    aPosAry.mnSrcY       = rDevPt.Y();
    aPosAry.mnSrcWidth   = rDevSize.Width();
    aPosAry.mnSrcHeight  = rDevSize.Height();
    aPosAry.mnDestX      = rPt.X();
    aPosAry.mnDestY      = rPt.Y();
    aPosAry.mnDestWidth  = rDevSize.Width();
    aPosAry.mnDestHeight = rDevSize.Height();
    ImplDrawOutDevDirect( &rOutDev, aPosAry );

    // Ensure that ClipRegion is recalculated and set
    mbInitClipRegion = true;

    SetRasterOp( eOldROP );
    mbMap = bOldMap;
    mpMetaFile = pOldMetaFile;
}

void OutputDevice::ImplGetFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                    OutputDevice& rDev )
{

    bool bOldMap = mbMap;
    mbMap = false;
    rDev.DrawOutDev( rDevPt, rDevSize, rPt, rDevSize, *this );
    mbMap = bOldMap;
}


void OutputDevice::ImplInitFillColor()
{
    DBG_TESTSOLARMUTEX();

    if( mbFillColor )
    {
        if( ROP_0 == meRasterOp )
            mpGraphics->SetROPFillColor( SAL_ROP_0 );
        else if( ROP_1 == meRasterOp )
            mpGraphics->SetROPFillColor( SAL_ROP_1 );
        else if( ROP_INVERT == meRasterOp )
            mpGraphics->SetROPFillColor( SAL_ROP_INVERT );
        else
            mpGraphics->SetFillColor( ImplColorToSal( maFillColor ) );
    }
    else
        mpGraphics->SetFillColor();

    mbInitFillColor = false;
}

void OutputDevice::SetDrawMode( sal_uLong nDrawMode )
{

    mnDrawMode = nDrawMode;

    if( mpAlphaVDev )
        mpAlphaVDev->SetDrawMode( nDrawMode );
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
            mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp), ROP_INVERT == meRasterOp );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetRasterOp( eRasterOp );
}

Color OutputDevice::ImplDrawModeToColor( const Color& rColor ) const
{
    Color aColor( rColor );
    sal_uLong  nDrawMode = GetDrawMode();

    if( nDrawMode & ( DRAWMODE_BLACKLINE | DRAWMODE_WHITELINE |
                      DRAWMODE_GRAYLINE | DRAWMODE_GHOSTEDLINE |
                      DRAWMODE_SETTINGSLINE ) )
    {
        if( !ImplIsColorTransparent( aColor ) )
        {
            if( nDrawMode & DRAWMODE_BLACKLINE )
            {
                aColor = Color( COL_BLACK );
            }
            else if( nDrawMode & DRAWMODE_WHITELINE )
            {
                aColor = Color( COL_WHITE );
            }
            else if( nDrawMode & DRAWMODE_GRAYLINE )
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if( nDrawMode & DRAWMODE_SETTINGSLINE )
            {
                aColor = GetSettings().GetStyleSettings().GetFontColor();
            }

            if( nDrawMode & DRAWMODE_GHOSTEDLINE )
            {
                aColor = Color( ( aColor.GetRed() >> 1 ) | 0x80,
                                ( aColor.GetGreen() >> 1 ) | 0x80,
                                ( aColor.GetBlue() >> 1 ) | 0x80);
            }
        }
    }
    return aColor;
}

void OutputDevice::SetFillColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaFillColorAction( Color(), false ) );

    if ( mbFillColor )
    {
        mbInitFillColor = true;
        mbFillColor = false;
        maFillColor = Color( COL_TRANSPARENT );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetFillColor();
}

void OutputDevice::SetFillColor( const Color& rColor )
{

    Color aColor( rColor );

    if( mnDrawMode & ( DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL |
                       DRAWMODE_GRAYFILL | DRAWMODE_NOFILL |
                       DRAWMODE_GHOSTEDFILL | DRAWMODE_SETTINGSFILL ) )
    {
        if( !ImplIsColorTransparent( aColor ) )
        {
            if( mnDrawMode & DRAWMODE_BLACKFILL )
            {
                aColor = Color( COL_BLACK );
            }
            else if( mnDrawMode & DRAWMODE_WHITEFILL )
            {
                aColor = Color( COL_WHITE );
            }
            else if( mnDrawMode & DRAWMODE_GRAYFILL )
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DRAWMODE_NOFILL )
            {
                aColor = Color( COL_TRANSPARENT );
            }
            else if( mnDrawMode & DRAWMODE_SETTINGSFILL )
            {
                aColor = GetSettings().GetStyleSettings().GetWindowColor();
            }

            if( mnDrawMode & DRAWMODE_GHOSTEDFILL )
            {
                aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                                (aColor.GetGreen() >> 1) | 0x80,
                                (aColor.GetBlue() >> 1) | 0x80);
            }
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaFillColorAction( aColor, true ) );

    if ( ImplIsColorTransparent( aColor ) )
    {
        if ( mbFillColor )
        {
            mbInitFillColor = true;
            mbFillColor = false;
            maFillColor = Color( COL_TRANSPARENT );
        }
    }
    else
    {
        if ( maFillColor != aColor )
        {
            mbInitFillColor = true;
            mbFillColor = true;
            maFillColor = aColor;
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetFillColor( COL_BLACK );
}

void OutputDevice::SetBackground()
{

    maBackground = Wallpaper();
    mbBackground = false;

    if( mpAlphaVDev )
        mpAlphaVDev->SetBackground();
}

void OutputDevice::SetBackground( const Wallpaper& rBackground )
{

    maBackground = rBackground;

    if( rBackground.GetStyle() == WALLPAPER_NULL )
        mbBackground = false;
    else
        mbBackground = true;

    if( mpAlphaVDev )
        mpAlphaVDev->SetBackground( rBackground );
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

sal_uInt32 OutputDevice::GetGCStackDepth() const
{
    return mpOutDevStateStack->size();
}

void OutputDevice::SetConnectMetaFile( GDIMetaFile* pMtf )
{
    mpMetaFile = pMtf;
}

void OutputDevice::EnableOutput( bool bEnable )
{
    mbOutput = bEnable;

    if( mpAlphaVDev )
        mpAlphaVDev->EnableOutput( bEnable );
}

void OutputDevice::SetSettings( const AllSettings& rSettings )
{
    *mxSettings = rSettings;

    if( mpAlphaVDev )
        mpAlphaVDev->SetSettings( rSettings );
}

sal_uInt16 OutputDevice::GetBitCount() const
{
    // we need a graphics instance
    if ( !mpGraphics )
    {
        if ( !((OutputDevice*)this)->AcquireGraphics() )
            return 0;
    }

    return (sal_uInt16)mpGraphics->GetBitCount();
}

sal_uInt16 OutputDevice::GetAlphaBitCount() const
{
    return 0;
}

sal_uLong OutputDevice::GetColorCount() const
{

    const sal_uInt16 nBitCount = GetBitCount();
    return( ( nBitCount > 31 ) ? ULONG_MAX : ( ( (sal_uLong) 1 ) << nBitCount) );
}

bool OutputDevice::HasAlpha()
{
    return mpAlphaVDev != NULL;
}

css::uno::Reference< css::awt::XGraphics > OutputDevice::CreateUnoGraphics()
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    return pWrapper ? pWrapper->CreateGraphics( this ) : css::uno::Reference< css::awt::XGraphics >();
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

css::uno::Any OutputDevice::GetSystemGfxDataAny() const
{
    const SystemGraphicsData aSysData = GetSystemGfxData();
    css::uno::Sequence< sal_Int8 > aSeq( (sal_Int8*)&aSysData,
                                                      aSysData.nSize );

    return css::uno::makeAny(aSeq);
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

void OutputDevice::Erase()
{
    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    if ( mbBackground )
    {
        RasterOp eRasterOp = GetRasterOp();
        if ( eRasterOp != ROP_OVERPAINT )
            SetRasterOp( ROP_OVERPAINT );
        DrawWallpaper( 0, 0, mnOutWidth, mnOutHeight, maBackground );
        if ( eRasterOp != ROP_OVERPAINT )
            SetRasterOp( eRasterOp );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->Erase();
}

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
                         (sal_uInt8*) rGfxLink.GetData(), rGfxLink.GetDataSize(), this );
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

void OutputDevice::DrawCheckered(const Point& rPos, const Size& rSize, sal_uInt32 nLen, Color aStart, Color aEnd)
{
    const sal_uInt32 nMaxX(rPos.X() + rSize.Width());
    const sal_uInt32 nMaxY(rPos.Y() + rSize.Height());

    Push(PUSH_LINECOLOR|PUSH_FILLCOLOR);
    SetLineColor();

    for(sal_uInt32 x(0), nX(rPos.X()); nX < nMaxX; x++, nX += nLen)
    {
        const sal_uInt32 nRight(std::min(nMaxX, nX + nLen));

        for(sal_uInt32 y(0), nY(rPos.Y()); nY < nMaxY; y++, nY += nLen)
        {
            const sal_uInt32 nBottom(std::min(nMaxY, nY + nLen));

            SetFillColor((x & 0x0001) ^ (y & 0x0001) ? aStart : aEnd);
            DrawRect(Rectangle(nX, nY, nRight, nBottom));
        }
    }

    Pop();
}

void OutputDevice::SetAntialiasing( sal_uInt16 nMode )
{
    if ( mnAntialiasing != nMode )
    {
        mnAntialiasing = nMode;
        mbInitFont = true;

        if(mpGraphics)
        {
            mpGraphics->setAntiAliasB2DDraw(mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW);
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetAntialiasing( nMode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
