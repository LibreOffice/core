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
#include <svsys.h>
#include <vcl/sysdata.hxx>

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

#ifdef DISABLE_DYNLOADING
// Linking all needed LO code into one .so/executable, these already
// exist in the tools library, so put them in the anonymous namespace
// here to avoid clash...
namespace {
#endif
#ifdef DISABLE_DYNLOADING
}
#endif

#ifdef DBG_UTIL
const char* ImplDbgCheckOutputDevice( const void* pObj )
{
    DBG_TESTSOLARMUTEX();

    const OutputDevice* pOutDev = (OutputDevice*)pObj;

    if ( (pOutDev->GetOutDevType() != OUTDEV_DONTKNOW) &&
         (pOutDev->GetOutDevType() != OUTDEV_WINDOW) &&
         (pOutDev->GetOutDevType() != OUTDEV_PRINTER) &&
         (pOutDev->GetOutDevType() != OUTDEV_VIRDEV) )
        return "OutputDevice data overwrite";

    return NULL;
}
#endif

#define OUTDEV_POLYPOLY_STACKBUF        32

struct ImplObjStack
{
    ImplObjStack*   mpPrev;
    MapMode*        mpMapMode;
    bool            mbMapActive;
    Region*         mpClipRegion;
    Color*          mpLineColor;
    Color*          mpFillColor;
    Font*           mpFont;
    Color*          mpTextColor;
    Color*          mpTextFillColor;
    Color*          mpTextLineColor;
    Color*          mpOverlineColor;
    Point*          mpRefPoint;
    TextAlign       meTextAlign;
    RasterOp        meRasterOp;
    sal_uLong           mnTextLayoutMode;
    LanguageType    meTextLanguage;
    sal_uInt16          mnFlags;
};

static void ImplDeleteObjStack( ImplObjStack* pObjStack )
{
    if ( pObjStack->mnFlags & PUSH_LINECOLOR )
    {
        if ( pObjStack->mpLineColor )
            delete pObjStack->mpLineColor;
    }
    if ( pObjStack->mnFlags & PUSH_FILLCOLOR )
    {
        if ( pObjStack->mpFillColor )
            delete pObjStack->mpFillColor;
    }
    if ( pObjStack->mnFlags & PUSH_FONT )
        delete pObjStack->mpFont;
    if ( pObjStack->mnFlags & PUSH_TEXTCOLOR )
        delete pObjStack->mpTextColor;
    if ( pObjStack->mnFlags & PUSH_TEXTFILLCOLOR )
    {
        if ( pObjStack->mpTextFillColor )
            delete pObjStack->mpTextFillColor;
    }
    if ( pObjStack->mnFlags & PUSH_TEXTLINECOLOR )
    {
        if ( pObjStack->mpTextLineColor )
            delete pObjStack->mpTextLineColor;
    }
    if ( pObjStack->mnFlags & PUSH_OVERLINECOLOR )
    {
        if ( pObjStack->mpOverlineColor )
            delete pObjStack->mpOverlineColor;
    }
    if ( pObjStack->mnFlags & PUSH_MAPMODE )
    {
        if ( pObjStack->mpMapMode )
            delete pObjStack->mpMapMode;
    }
    if ( pObjStack->mnFlags & PUSH_CLIPREGION )
    {
        if ( pObjStack->mpClipRegion )
            delete pObjStack->mpClipRegion;
    }
    if ( pObjStack->mnFlags & PUSH_REFPOINT )
    {
        if ( pObjStack->mpRefPoint )
            delete pObjStack->mpRefPoint;
    }

    delete pObjStack;
}

bool OutputDevice::ImplIsAntiparallel() const
{
    bool bRet = false;
    if( ImplGetGraphics() )
    {
        if( ( (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) && ! IsRTLEnabled() ) ||
            ( ! (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) && IsRTLEnabled() ) )
        {
            bRet = true;
        }
    }
    return bRet;
}

bool OutputDevice::ImplSelectClipRegion( const Region& rRegion, SalGraphics* pGraphics )
{
    DBG_TESTSOLARMUTEX();

    if( !pGraphics )
    {
        if( !mpGraphics )
            if( !ImplInitGraphics() )
                return false;
        pGraphics = mpGraphics;
    }

    bool bClipRegion = pGraphics->SetClipRegion( rRegion, this );
    OSL_ENSURE( bClipRegion, "OutputDevice::ImplSelectClipRegion() - can't cerate region" );
    return bClipRegion;
}

Polygon ImplSubdivideBezier( const Polygon& rPoly )
{
    Polygon aPoly;

    // #100127# Use adaptive subdivide instead of fixed 25 segments
    rPoly.AdaptiveSubdivide( aPoly );

    return aPoly;
}

PolyPolygon ImplSubdivideBezier( const PolyPolygon& rPolyPoly )
{
    sal_uInt16 i, nPolys = rPolyPoly.Count();
    PolyPolygon aPolyPoly( nPolys );
    for( i=0; i<nPolys; ++i )
        aPolyPoly.Insert( ImplSubdivideBezier( rPolyPoly.GetObject(i) ) );

    return aPolyPoly;
}

// #100127# Extracted from OutputDevice::DrawPolyPolygon()
void OutputDevice::ImplDrawPolyPolygon( sal_uInt16 nPoly, const PolyPolygon& rPolyPoly )
{
    // AW: This crashes on empty PolyPolygons, avoid that
    if(!nPoly)
        return;

    sal_uInt32          aStackAry1[OUTDEV_POLYPOLY_STACKBUF];
    PCONSTSALPOINT      aStackAry2[OUTDEV_POLYPOLY_STACKBUF];
    sal_uInt8*              aStackAry3[OUTDEV_POLYPOLY_STACKBUF];
    sal_uInt32*         pPointAry;
    PCONSTSALPOINT*     pPointAryAry;
    const sal_uInt8**       pFlagAryAry;
    sal_uInt16              i = 0, j = 0, last = 0;
    bool                bHaveBezier = false;
    if ( nPoly > OUTDEV_POLYPOLY_STACKBUF )
    {
        pPointAry       = new sal_uInt32[nPoly];
        pPointAryAry    = new PCONSTSALPOINT[nPoly];
        pFlagAryAry     = new const sal_uInt8*[nPoly];
    }
    else
    {
        pPointAry       = aStackAry1;
        pPointAryAry    = aStackAry2;
        pFlagAryAry     = (const sal_uInt8**)aStackAry3;
    }
    do
    {
        const Polygon&  rPoly = rPolyPoly.GetObject( i );
        sal_uInt16          nSize = rPoly.GetSize();
        if ( nSize )
        {
            pPointAry[j]    = nSize;
            pPointAryAry[j] = (PCONSTSALPOINT)rPoly.GetConstPointAry();
            pFlagAryAry[j]  = rPoly.GetConstFlagAry();
            last            = i;

            if( pFlagAryAry[j] )
                bHaveBezier = true;

            ++j;
        }

        ++i;
    }
    while ( i < nPoly );

    if ( j == 1 )
    {
        // #100127# Forward beziers to sal, if any
        if( bHaveBezier )
        {
            if( !mpGraphics->DrawPolygonBezier( *pPointAry, *pPointAryAry, *pFlagAryAry, this ) )
            {
                Polygon aPoly = ImplSubdivideBezier( rPolyPoly.GetObject( last ) );
                mpGraphics->DrawPolygon( aPoly.GetSize(), (const SalPoint*)aPoly.GetConstPointAry(), this );
            }
        }
        else
        {
            mpGraphics->DrawPolygon( *pPointAry, *pPointAryAry, this );
        }
    }
    else
    {
        // #100127# Forward beziers to sal, if any
        if( bHaveBezier )
        {
            if( !mpGraphics->DrawPolyPolygonBezier( j, pPointAry, pPointAryAry, pFlagAryAry, this ) )
            {
                PolyPolygon aPolyPoly = ImplSubdivideBezier( rPolyPoly );
                ImplDrawPolyPolygon( aPolyPoly.Count(), aPolyPoly );
            }
        }
        else
        {
            mpGraphics->DrawPolyPolygon( j, pPointAry, pPointAryAry, this );
        }
    }

    if ( pPointAry != aStackAry1 )
    {
        delete[] pPointAry;
        delete[] pPointAryAry;
        delete[] pFlagAryAry;
    }
}

OutputDevice::OutputDevice() :
    maRegion(true),
    maFillColor( COL_WHITE ),
    maTextLineColor( COL_TRANSPARENT ),
    mxSettings( new AllSettings(Application::GetSettings()) )
{

    mpGraphics          = NULL;
    mpUnoGraphicsList   = NULL;
    mpPrevGraphics      = NULL;
    mpNextGraphics      = NULL;
    mpMetaFile          = NULL;
    mpFontEntry         = NULL;
    mpFontCache         = NULL;
    mpFontCollection   = NULL;
    mpGetDevFontList    = NULL;
    mpGetDevSizeList    = NULL;
    mpObjStack          = NULL;
    mpOutDevData        = NULL;
    mpPDFWriter         = NULL;
    mpAlphaVDev         = NULL;
    mpExtOutDevData     = NULL;
    mnOutOffX           = 0;
    mnOutOffY           = 0;
    mnOutWidth          = 0;
    mnOutHeight         = 0;
    mnDPIX              = 0;
    mnDPIY              = 0;
    mnDPIScaleFactor    = 1;
    mnTextOffX          = 0;
    mnTextOffY          = 0;
    mnOutOffOrigX       = 0;
    mnOutOffLogicX      = 0;
    mnOutOffOrigY       = 0;
    mnOutOffLogicY      = 0;
    mnEmphasisAscent    = 0;
    mnEmphasisDescent   = 0;
    mnDrawMode          = 0;
    mnTextLayoutMode        = TEXT_LAYOUT_DEFAULT;
    if( Application::GetSettings().GetLayoutRTL() ) //#i84553# tip BiDi preference to RTL
        mnTextLayoutMode = TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_TEXTORIGIN_LEFT;
    meOutDevType        = OUTDEV_DONTKNOW;
    meOutDevViewType    = OUTDEV_VIEWTYPE_DONTKNOW;
    mbMap               = false;
    mbMapIsDefault      = true;
    mbClipRegion        = false;
    mbBackground        = false;
    mbOutput            = true;
    mbDevOutput         = false;
    mbOutputClipped     = false;
    maTextColor         = Color( COL_BLACK );
    maOverlineColor     = Color( COL_TRANSPARENT );
    meTextAlign         = maFont.GetAlign();
    meRasterOp          = ROP_OVERPAINT;
    mnAntialiasing      = 0;
    meTextLanguage      = 0;  // TODO: get default from configuration?
    mbLineColor         = true;
    mbFillColor         = true;
    mbInitLineColor     = true;
    mbInitFillColor     = true;
    mbInitFont          = true;
    mbInitTextColor     = true;
    mbInitClipRegion    = true;
    mbClipRegionSet     = false;
    mbKerning           = false;
    mbNewFont           = true;
    mbTextLines         = false;
    mbTextSpecial       = false;
    mbRefPoint          = false;
    mbEnableRTL         = false;    // mirroring must be explicitly allowed (typically for windows only)

    // struct ImplMapRes
    maMapRes.mnMapOfsX          = 0;
    maMapRes.mnMapOfsY          = 0;
    maMapRes.mnMapScNumX        = 1;
    maMapRes.mnMapScNumY        = 1;
    maMapRes.mnMapScDenomX      = 1;
    maMapRes.mnMapScDenomY      = 1;
    // struct ImplThresholdRes
    maThresRes.mnThresLogToPixX = 0;
    maThresRes.mnThresLogToPixY = 0;
    maThresRes.mnThresPixToLogX = 0;
    maThresRes.mnThresPixToLogY = 0;
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
        ImplDeInitOutDevData();

    ImplObjStack* pData = mpObjStack;
    if ( pData )
    {
        SAL_WARN( "vcl.gdi", "OutputDevice::~OutputDevice(): OutputDevice::Push() calls != OutputDevice::Pop() calls" );
        while ( pData )
        {
            ImplObjStack* pTemp = pData;
            pData = pData->mpPrev;
            ImplDeleteObjStack( pTemp );
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

bool OutputDevice::supportsOperation( OutDevSupportType eType ) const
{
    if( !mpGraphics )
        if( !ImplGetGraphics() )
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
   return ( ImplGetGraphics() && (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) );
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

SalGraphics* OutputDevice::ImplGetGraphics()
{
    DBG_TESTSOLARMUTEX();

    if ( !mpGraphics )
    {
        if ( !ImplInitGraphics() )
        {
            SAL_WARN("vcl", "No mpGraphics set");
        }
    }

    return mpGraphics;
}

SalGraphics const *OutputDevice::ImplGetGraphics() const
{
    DBG_TESTSOLARMUTEX();

    if ( !mpGraphics )
    {
        if ( !ImplInitGraphics() )
        {
            SAL_WARN("vcl", "No mpGraphics set");
        }
    }

    return mpGraphics;
}

void OutputDevice::ImplInitOutDevData()
{
    if ( !mpOutDevData )
    {
        mpOutDevData = new ImplOutDevData;
        mpOutDevData->mpRotateDev = NULL;
        mpOutDevData->mpRecordLayout = NULL;

        // #i75163#
        mpOutDevData->mpViewTransform = NULL;
        mpOutDevData->mpInverseViewTransform = NULL;
    }
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

void OutputDevice::ImplDeInitOutDevData()
{
    if ( mpOutDevData )
    {
        if ( mpOutDevData->mpRotateDev )
            delete mpOutDevData->mpRotateDev;

        // #i75163#
        ImplInvalidateViewTransform();

        delete mpOutDevData;
    }
}

void OutputDevice::ImplInitLineColor()
{
    DBG_TESTSOLARMUTEX();

    if( mbLineColor )
    {
        if( ROP_0 == meRasterOp )
            mpGraphics->SetROPLineColor( SAL_ROP_0 );
        else if( ROP_1 == meRasterOp )
            mpGraphics->SetROPLineColor( SAL_ROP_1 );
        else if( ROP_INVERT == meRasterOp )
            mpGraphics->SetROPLineColor( SAL_ROP_INVERT );
        else
            mpGraphics->SetLineColor( ImplColorToSal( maLineColor ) );
    }
    else
        mpGraphics->SetLineColor();

    mbInitLineColor = false;
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

// TODO: fdo#74424 - this needs to be moved out of OutputDevice and into the
// Window, VirtualDevice and Printer classes
void OutputDevice::ImplInitClipRegion()
{
    DBG_TESTSOLARMUTEX();

    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        Window* pWindow = (Window*)this;
        Region  aRegion;

        // Put back backed up background
        if ( pWindow->mpWindowImpl->mpFrameData->mpFirstBackWin )
            pWindow->ImplInvalidateAllOverlapBackgrounds();
        if ( pWindow->mpWindowImpl->mbInPaint )
            aRegion = *(pWindow->mpWindowImpl->mpPaintRegion);
        else
        {
            aRegion = *(pWindow->ImplGetWinChildClipRegion());
            // --- RTL -- only this region is in frame coordinates, so re-mirror it
            // the mpWindowImpl->mpPaintRegion above is already correct (see ImplCallPaint()) !
            if( ImplIsAntiparallel() )
                ReMirror ( aRegion );
        }
        if ( mbClipRegion )
            aRegion.Intersect( ImplPixelToDevicePixel( maRegion ) );
        if ( aRegion.IsEmpty() )
            mbOutputClipped = true;
        else
        {
            mbOutputClipped = false;
            ImplSelectClipRegion( aRegion );
        }
        mbClipRegionSet = true;
    }
    else
    {
        if ( mbClipRegion )
        {
            if ( maRegion.IsEmpty() )
                mbOutputClipped = true;
            else
            {
                mbOutputClipped = false;

                // #102532# Respect output offset also for clip region
                Region aRegion( ImplPixelToDevicePixel( maRegion ) );
                const bool bClipDeviceBounds( ! GetPDFWriter()
                                              && GetOutDevType() != OUTDEV_PRINTER );
                if( bClipDeviceBounds )
                {
                    // Perform actual rect clip against outdev
                    // dimensions, to generate empty clips whenever one of the
                    // values is completely off the device.
                    Rectangle aDeviceBounds( mnOutOffX, mnOutOffY,
                                             mnOutOffX+GetOutputWidthPixel()-1,
                                             mnOutOffY+GetOutputHeightPixel()-1 );
                    aRegion.Intersect( aDeviceBounds );
                }

                if ( aRegion.IsEmpty() )
                {
                    mbOutputClipped = true;
                }
                else
                {
                    mbOutputClipped = false;
                    ImplSelectClipRegion( aRegion );
                }
            }

            mbClipRegionSet = true;
        }
        else
        {
            if ( mbClipRegionSet )
            {
                mpGraphics->ResetClipRegion();
                mbClipRegionSet = false;
            }

            mbOutputClipped = false;
        }
    }

    mbInitClipRegion = false;
}

void OutputDevice::ImplSetClipRegion( const Region* pRegion )
{
    DBG_TESTSOLARMUTEX();

    if ( !pRegion )
    {
        if ( mbClipRegion )
        {
            maRegion            = Region(true);
            mbClipRegion        = false;
            mbInitClipRegion    = true;
        }
    }
    else
    {
        maRegion            = *pRegion;
        mbClipRegion        = true;
        mbInitClipRegion    = true;
    }
}

void OutputDevice::SetClipRegion()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( Region(), false ) );

    ImplSetClipRegion( NULL );

    if( mpAlphaVDev )
        mpAlphaVDev->SetClipRegion();
}

void OutputDevice::SetClipRegion( const Region& rRegion )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( rRegion, true ) );

    if ( rRegion.IsNull() )
    {
        ImplSetClipRegion( NULL );
    }
    else
    {
        Region aRegion = LogicToPixel( rRegion );
        ImplSetClipRegion( &aRegion );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetClipRegion( rRegion );
}

Region OutputDevice::GetClipRegion() const
{

    return PixelToLogic( maRegion );
}

Region OutputDevice::GetActiveClipRegion() const
{

    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        Region aRegion(true);
        Window* pWindow = (Window*)this;
        if ( pWindow->mpWindowImpl->mbInPaint )
        {
            aRegion = *(pWindow->mpWindowImpl->mpPaintRegion);
            aRegion.Move( -mnOutOffX, -mnOutOffY );
        }
        if ( mbClipRegion )
            aRegion.Intersect( maRegion );
        return PixelToLogic( aRegion );
    }
    else
        return GetClipRegion();
}

void OutputDevice::MoveClipRegion( long nHorzMove, long nVertMove )
{

    if ( mbClipRegion )
    {
        if( mpMetaFile )
            mpMetaFile->AddAction( new MetaMoveClipRegionAction( nHorzMove, nVertMove ) );

        maRegion.Move( ImplLogicWidthToDevicePixel( nHorzMove ),
                       ImplLogicHeightToDevicePixel( nVertMove ) );
        mbInitClipRegion = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->MoveClipRegion( nHorzMove, nVertMove );
}

void OutputDevice::IntersectClipRegion( const Rectangle& rRect )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaISectRectClipRegionAction( rRect ) );

    Rectangle aRect = LogicToPixel( rRect );
    maRegion.Intersect( aRect );
    mbClipRegion        = true;
    mbInitClipRegion    = true;

    if( mpAlphaVDev )
        mpAlphaVDev->IntersectClipRegion( rRect );
}

void OutputDevice::IntersectClipRegion( const Region& rRegion )
{

    if(!rRegion.IsNull())
    {
        if ( mpMetaFile )
            mpMetaFile->AddAction( new MetaISectRegionClipRegionAction( rRegion ) );

        Region aRegion = LogicToPixel( rRegion );
        maRegion.Intersect( aRegion );
        mbClipRegion        = true;
        mbInitClipRegion    = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->IntersectClipRegion( rRegion );
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

        if( mpGraphics || ImplGetGraphics() )
            mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp), ROP_INVERT == meRasterOp );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetRasterOp( eRasterOp );
}

void OutputDevice::SetLineColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineColorAction( Color(), false ) );

    if ( mbLineColor )
    {
        mbInitLineColor = true;
        mbLineColor = false;
        maLineColor = Color( COL_TRANSPARENT );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetLineColor();
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


void OutputDevice::SetLineColor( const Color& rColor )
{

    Color aColor = ImplDrawModeToColor( rColor );

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineColorAction( aColor, true ) );

    if( ImplIsColorTransparent( aColor ) )
    {
        if ( mbLineColor )
        {
            mbInitLineColor = true;
            mbLineColor = false;
            maLineColor = Color( COL_TRANSPARENT );
        }
    }
    else
    {
        if( maLineColor != aColor )
        {
            mbInitLineColor = true;
            mbLineColor = true;
            maLineColor = aColor;
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetLineColor( COL_BLACK );
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

void OutputDevice::DrawLine( const Point& rStartPt, const Point& rEndPt )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineAction( rStartPt, rEndPt ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ImplIsRecordLayout() )
        return;

    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    // #i101598# support AA and snap for lines, too
    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && IsLineColor())
    {
        // at least transform with double precision to device coordinates; this will
        // avoid pixel snap of single, appended lines
        const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation());
        const basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );
        basegfx::B2DPolygon aB2DPolyLine;

        aB2DPolyLine.append(basegfx::B2DPoint(rStartPt.X(), rStartPt.Y()));
        aB2DPolyLine.append(basegfx::B2DPoint(rEndPt.X(), rEndPt.Y()));
        aB2DPolyLine.transform( aTransform );

        if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
        {
            aB2DPolyLine = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyLine);
        }

        if( mpGraphics->DrawPolyLine( aB2DPolyLine, 0.0, aB2DLineWidth, basegfx::B2DLINEJOIN_NONE, css::drawing::LineCap_BUTT, this))
        {
            return;
        }
    }

    const Point aStartPt(ImplLogicToDevicePixel(rStartPt));
    const Point aEndPt(ImplLogicToDevicePixel(rEndPt));

    mpGraphics->DrawLine( aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y(), this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawLine( rStartPt, rEndPt );
}

void OutputDevice::ImplPaintLineGeometryWithEvtlExpand(
    const LineInfo& rInfo,
    basegfx::B2DPolyPolygon aLinePolyPolygon)
{
    const bool bTryAA((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && IsLineColor());
    basegfx::B2DPolyPolygon aFillPolyPolygon;
    const bool bDashUsed(LINE_DASH == rInfo.GetStyle());
    const bool bLineWidthUsed(rInfo.GetWidth() > 1);

    if(bDashUsed && aLinePolyPolygon.count())
    {
        ::std::vector< double > fDotDashArray;
        const double fDashLen(rInfo.GetDashLen());
        const double fDotLen(rInfo.GetDotLen());
        const double fDistance(rInfo.GetDistance());

        for(sal_uInt16 a(0); a < rInfo.GetDashCount(); a++)
        {
            fDotDashArray.push_back(fDashLen);
            fDotDashArray.push_back(fDistance);
        }

        for(sal_uInt16 b(0); b < rInfo.GetDotCount(); b++)
        {
            fDotDashArray.push_back(fDotLen);
            fDotDashArray.push_back(fDistance);
        }

        const double fAccumulated(::std::accumulate(fDotDashArray.begin(), fDotDashArray.end(), 0.0));

        if(fAccumulated > 0.0)
        {
            basegfx::B2DPolyPolygon aResult;

            for(sal_uInt32 c(0); c < aLinePolyPolygon.count(); c++)
            {
                basegfx::B2DPolyPolygon aLineTraget;
                basegfx::tools::applyLineDashing(
                    aLinePolyPolygon.getB2DPolygon(c),
                    fDotDashArray,
                    &aLineTraget);
                aResult.append(aLineTraget);
            }

            aLinePolyPolygon = aResult;
        }
    }

    if(bLineWidthUsed && aLinePolyPolygon.count())
    {
        const double fHalfLineWidth((rInfo.GetWidth() * 0.5) + 0.5);

        if(aLinePolyPolygon.areControlPointsUsed())
        {
            // #i110768# When area geometry has to be created, do not
            // use the fallback bezier decomposition inside createAreaGeometry,
            // but one that is at least as good as ImplSubdivideBezier was.
            // There, Polygon::AdaptiveSubdivide was used with default parameter
            // 1.0 as quality index.
            aLinePolyPolygon = basegfx::tools::adaptiveSubdivideByDistance(aLinePolyPolygon, 1.0);
        }

        for(sal_uInt32 a(0); a < aLinePolyPolygon.count(); a++)
        {
            aFillPolyPolygon.append(basegfx::tools::createAreaGeometry(
                aLinePolyPolygon.getB2DPolygon(a),
                fHalfLineWidth,
                rInfo.GetLineJoin(),
                rInfo.GetLineCap()));
        }

        aLinePolyPolygon.clear();
    }

    GDIMetaFile* pOldMetaFile = mpMetaFile;
    mpMetaFile = NULL;

    if(aLinePolyPolygon.count())
    {
        for(sal_uInt32 a(0); a < aLinePolyPolygon.count(); a++)
        {
            const basegfx::B2DPolygon aCandidate(aLinePolyPolygon.getB2DPolygon(a));
            bool bDone(false);

            if(bTryAA)
            {
                bDone = mpGraphics->DrawPolyLine( aCandidate, 0.0, basegfx::B2DVector(1.0,1.0), basegfx::B2DLINEJOIN_NONE, css::drawing::LineCap_BUTT, this);
            }

            if(!bDone)
            {
                const Polygon aPolygon(aCandidate);
                mpGraphics->DrawPolyLine(aPolygon.GetSize(), (const SalPoint*)aPolygon.GetConstPointAry(), this);
            }
        }
    }

    if(aFillPolyPolygon.count())
    {
        const Color     aOldLineColor( maLineColor );
        const Color     aOldFillColor( maFillColor );

        SetLineColor();
        ImplInitLineColor();
        SetFillColor( aOldLineColor );
        ImplInitFillColor();

        bool bDone(false);

        if(bTryAA)
        {
            bDone = mpGraphics->DrawPolyPolygon(aFillPolyPolygon, 0.0, this);
        }

        if(!bDone)
        {
            for(sal_uInt32 a(0); a < aFillPolyPolygon.count(); a++)
            {
                Polygon aPolygon(aFillPolyPolygon.getB2DPolygon(a));

                // need to subdivide, mpGraphics->DrawPolygon ignores curves
                aPolygon.AdaptiveSubdivide(aPolygon);
                mpGraphics->DrawPolygon(aPolygon.GetSize(), (const SalPoint*)aPolygon.GetConstPointAry(), this);
            }
        }

        SetFillColor( aOldFillColor );
        SetLineColor( aOldLineColor );
    }

    mpMetaFile = pOldMetaFile;
}

void OutputDevice::DrawLine( const Point& rStartPt, const Point& rEndPt,
                             const LineInfo& rLineInfo )
{

    if ( rLineInfo.IsDefault() )
    {
        DrawLine( rStartPt, rEndPt );
        return;
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineAction( rStartPt, rEndPt, rLineInfo ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ( LINE_NONE == rLineInfo.GetStyle() ) || ImplIsRecordLayout() )
        return;

    if( !mpGraphics && !ImplGetGraphics() )
        return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();

    if ( mbOutputClipped )
        return;

    const Point aStartPt( ImplLogicToDevicePixel( rStartPt ) );
    const Point aEndPt( ImplLogicToDevicePixel( rEndPt ) );
    const LineInfo aInfo( ImplLogicToDevicePixel( rLineInfo ) );
    const bool bDashUsed(LINE_DASH == aInfo.GetStyle());
    const bool bLineWidthUsed(aInfo.GetWidth() > 1);

    if ( mbInitLineColor )
        ImplInitLineColor();

    if(bDashUsed || bLineWidthUsed)
    {
        basegfx::B2DPolygon aLinePolygon;
        aLinePolygon.append(basegfx::B2DPoint(aStartPt.X(), aStartPt.Y()));
        aLinePolygon.append(basegfx::B2DPoint(aEndPt.X(), aEndPt.Y()));

        ImplPaintLineGeometryWithEvtlExpand(aInfo, basegfx::B2DPolyPolygon(aLinePolygon));
    }
    else
    {
        mpGraphics->DrawLine( aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y(), this );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawLine( rStartPt, rEndPt, rLineInfo );
}

void OutputDevice::DrawRect( const Rectangle& rRect )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRectAction( rRect ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || ImplIsRecordLayout() )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;
    aRect.Justify();

    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();

    mpGraphics->DrawRect( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawRect( rRect );
}

void OutputDevice::DrawPolyLine( const Polygon& rPoly )
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyLineAction( rPoly ) );

    sal_uInt16 nPoints = rPoly.GetSize();

    if ( !IsDeviceOutputNecessary() || !mbLineColor || (nPoints < 2) || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if ( !mpGraphics )
        if ( !ImplGetGraphics() )
            return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    const bool bTryAA((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && IsLineColor());

    // use b2dpolygon drawing if possible
    if(bTryAA && ImplTryDrawPolyLineDirect(rPoly.getB2DPolygon()))
    {
        basegfx::B2DPolygon aB2DPolyLine(rPoly.getB2DPolygon());
        const ::basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
        const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

        // transform the polygon
        aB2DPolyLine.transform( aTransform );

        if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
        {
            aB2DPolyLine = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyLine);
        }

        if(mpGraphics->DrawPolyLine( aB2DPolyLine, 0.0, aB2DLineWidth, basegfx::B2DLINEJOIN_NONE, css::drawing::LineCap_BUTT, this))
        {
            return;
        }
    }

    Polygon aPoly = ImplLogicToDevicePixel( rPoly );
    const SalPoint* pPtAry = (const SalPoint*)aPoly.GetConstPointAry();

    // #100127# Forward beziers to sal, if any
    if( aPoly.HasFlags() )
    {
        const sal_uInt8* pFlgAry = aPoly.GetConstFlagAry();
        if( !mpGraphics->DrawPolyLineBezier( nPoints, pPtAry, pFlgAry, this ) )
        {
            aPoly = ImplSubdivideBezier(aPoly);
            pPtAry = (const SalPoint*)aPoly.GetConstPointAry();
            mpGraphics->DrawPolyLine( aPoly.GetSize(), pPtAry, this );
        }
    }
    else
    {
        mpGraphics->DrawPolyLine( nPoints, pPtAry, this );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyLine( rPoly );
}

void OutputDevice::DrawPolyLine( const Polygon& rPoly, const LineInfo& rLineInfo )
{

    if ( rLineInfo.IsDefault() )
    {
        DrawPolyLine( rPoly );
        return;
    }

    // #i101491#
    // Try direct Fallback to B2D-Version of DrawPolyLine
    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && LINE_SOLID == rLineInfo.GetStyle())
    {
        DrawPolyLine( rPoly.getB2DPolygon(), (double)rLineInfo.GetWidth(), rLineInfo.GetLineJoin(), rLineInfo.GetLineCap());
        return;
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyLineAction( rPoly, rLineInfo ) );

    ImplDrawPolyLineWithLineInfo(rPoly, rLineInfo);
}

void OutputDevice::ImplDrawPolyLineWithLineInfo(const Polygon& rPoly, const LineInfo& rLineInfo)
{
    sal_uInt16 nPoints(rPoly.GetSize());

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ( nPoints < 2 ) || ( LINE_NONE == rLineInfo.GetStyle() ) || ImplIsRecordLayout() )
        return;

    Polygon aPoly = ImplLogicToDevicePixel( rPoly );

    // #100127# LineInfo is not curve-safe, subdivide always

    // What shall this mean? It's wrong to subdivide here when the
    // polygon is a fat line. In that case, the painted geometry
    // WILL be much different.
    // I also have no idea how this could be related to the given ID
    // which reads 'consolidate boost versions' in the task description.
    // Removing.

    //if( aPoly.HasFlags() )
    //{
    //    aPoly = ImplSubdivideBezier( aPoly );
    //    nPoints = aPoly.GetSize();
    //}

    // we need a graphics
    if ( !mpGraphics && !ImplGetGraphics() )
        return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();

    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();

    const LineInfo aInfo( ImplLogicToDevicePixel( rLineInfo ) );
    const bool bDashUsed(LINE_DASH == aInfo.GetStyle());
    const bool bLineWidthUsed(aInfo.GetWidth() > 1);

    if(bDashUsed || bLineWidthUsed)
    {
        ImplPaintLineGeometryWithEvtlExpand(aInfo, basegfx::B2DPolyPolygon(aPoly.getB2DPolygon()));
    }
    else
    {
        // #100127# the subdivision HAS to be done here since only a pointer
        // to an array of points is given to the DrawPolyLine method, there is
        // NO way to find out there that it's a curve.
        if( aPoly.HasFlags() )
        {
            aPoly = ImplSubdivideBezier( aPoly );
            nPoints = aPoly.GetSize();
        }

        mpGraphics->DrawPolyLine(nPoints, (const SalPoint*)aPoly.GetConstPointAry(), this);
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyLine( rPoly, rLineInfo );
}

void OutputDevice::DrawPolygon( const Polygon& rPoly )
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolygonAction( rPoly ) );

    sal_uInt16 nPoints = rPoly.GetSize();

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || (nPoints < 2) || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if ( !mpGraphics )
        if ( !ImplGetGraphics() )
            return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();

    // use b2dpolygon drawing if possible
    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && (IsLineColor() || IsFillColor()))
    {
        const ::basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
        basegfx::B2DPolygon aB2DPolygon(rPoly.getB2DPolygon());
        bool bSuccess(true);

        // transform the polygon and ensure closed
        aB2DPolygon.transform(aTransform);
        aB2DPolygon.setClosed(true);

        if(IsFillColor())
        {
            bSuccess = mpGraphics->DrawPolyPolygon(basegfx::B2DPolyPolygon(aB2DPolygon), 0.0, this);
        }

        if(bSuccess && IsLineColor())
        {
            const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

            if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
            {
                aB2DPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolygon);
            }

            bSuccess = mpGraphics->DrawPolyLine(
                aB2DPolygon,
                0.0,
                aB2DLineWidth,
                basegfx::B2DLINEJOIN_NONE,
                css::drawing::LineCap_BUTT,
                this);
        }

        if(bSuccess)
        {
            return;
        }
    }

    Polygon aPoly = ImplLogicToDevicePixel( rPoly );
    const SalPoint* pPtAry = (const SalPoint*)aPoly.GetConstPointAry();

    // #100127# Forward beziers to sal, if any
    if( aPoly.HasFlags() )
    {
        const sal_uInt8* pFlgAry = aPoly.GetConstFlagAry();
        if( !mpGraphics->DrawPolygonBezier( nPoints, pPtAry, pFlgAry, this ) )
        {
            aPoly = ImplSubdivideBezier(aPoly);
            pPtAry = (const SalPoint*)aPoly.GetConstPointAry();
            mpGraphics->DrawPolygon( aPoly.GetSize(), pPtAry, this );
        }
    }
    else
    {
        mpGraphics->DrawPolygon( nPoints, pPtAry, this );
    }
    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolygon( rPoly );
}

void OutputDevice::DrawPolyPolygon( const PolyPolygon& rPolyPoly )
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyPolygonAction( rPolyPoly ) );

    sal_uInt16 nPoly = rPolyPoly.Count();

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || !nPoly || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if ( !mpGraphics )
        if ( !ImplGetGraphics() )
            return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();

    // use b2dpolygon drawing if possible
    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && (IsLineColor() || IsFillColor()))
    {
        const ::basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPoly.getB2DPolyPolygon());
        bool bSuccess(true);

        // transform the polygon and ensure closed
        aB2DPolyPolygon.transform(aTransform);
        aB2DPolyPolygon.setClosed(true);

        if(IsFillColor())
        {
            bSuccess = mpGraphics->DrawPolyPolygon(aB2DPolyPolygon, 0.0, this);
        }

        if(bSuccess && IsLineColor())
        {
            const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

            if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
            {
                aB2DPolyPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyPolygon);
            }

            for(sal_uInt32 a(0); bSuccess && a < aB2DPolyPolygon.count(); a++)
            {
                bSuccess = mpGraphics->DrawPolyLine(
                    aB2DPolyPolygon.getB2DPolygon(a),
                    0.0,
                    aB2DLineWidth,
                    basegfx::B2DLINEJOIN_NONE,
                    css::drawing::LineCap_BUTT,
                    this);
            }
        }

        if(bSuccess)
        {
            return;
        }
    }

    if ( nPoly == 1 )
    {
        // #100127# Map to DrawPolygon
        Polygon aPoly = rPolyPoly.GetObject( 0 );
        if( aPoly.GetSize() >= 2 )
        {
            GDIMetaFile* pOldMF = mpMetaFile;
            mpMetaFile = NULL;

            DrawPolygon( aPoly );

            mpMetaFile = pOldMF;
        }
    }
    else
    {
        // #100127# moved real PolyPolygon draw to separate method,
        // have to call recursively, avoiding duplicate
        // ImplLogicToDevicePixel calls
        ImplDrawPolyPolygon( nPoly, ImplLogicToDevicePixel( rPolyPoly ) );
    }
    if( mpAlphaVDev )
        mpAlphaVDev->DrawPolyPolygon( rPolyPoly );
}

void OutputDevice::DrawPolygon( const basegfx::B2DPolygon& rB2DPolygon)
{
    // AW: Do NOT paint empty polygons
    if(rB2DPolygon.count())
    {
        basegfx::B2DPolyPolygon aPP( rB2DPolygon );
        DrawPolyPolygon( aPP );
    }
}

// Caution: This method is nearly the same as
// OutputDevice::DrawTransparent( const basegfx::B2DPolyPolygon& rB2DPolyPoly, double fTransparency),
// so when changes are made here do not forget to make change sthere, too

void OutputDevice::DrawPolyPolygon( const basegfx::B2DPolyPolygon& rB2DPolyPoly )
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyPolygonAction( PolyPolygon( rB2DPolyPoly ) ) );

    // call helper
    ImplDrawPolyPolygonWithB2DPolyPolygon(rB2DPolyPoly);
}

void OutputDevice::ImplDrawPolyPolygonWithB2DPolyPolygon(const basegfx::B2DPolyPolygon& rB2DPolyPoly)
{
    // Do not paint empty PolyPolygons
    if(!rB2DPolyPoly.count() || !IsDeviceOutputNecessary())
        return;

    // we need a graphics
    if( !mpGraphics )
        if( !ImplGetGraphics() )
            return;

    if( mbInitClipRegion )
        ImplInitClipRegion();
    if( mbOutputClipped )
        return;

    if( mbInitLineColor )
        ImplInitLineColor();
    if( mbInitFillColor )
        ImplInitFillColor();

    if((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && (IsLineColor() || IsFillColor()))
    {
        const basegfx::B2DHomMatrix aTransform(ImplGetDeviceTransformation());
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rB2DPolyPoly);
        bool bSuccess(true);

        // transform the polygon and ensure closed
        aB2DPolyPolygon.transform(aTransform);
        aB2DPolyPolygon.setClosed(true);

        if(IsFillColor())
        {
            bSuccess = mpGraphics->DrawPolyPolygon(aB2DPolyPolygon, 0.0, this);
        }

        if(bSuccess && IsLineColor())
        {
            const ::basegfx::B2DVector aB2DLineWidth( 1.0, 1.0 );

            if(mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
            {
                aB2DPolyPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolyPolygon);
            }

            for(sal_uInt32 a(0);bSuccess && a < aB2DPolyPolygon.count(); a++)
            {
                bSuccess = mpGraphics->DrawPolyLine(
                    aB2DPolyPolygon.getB2DPolygon(a),
                    0.0,
                    aB2DLineWidth,
                    basegfx::B2DLINEJOIN_NONE,
                    css::drawing::LineCap_BUTT,
                    this);
            }
        }

        if(bSuccess)
        {
            return;
        }
    }

    // fallback to old polygon drawing if needed
    const PolyPolygon aToolsPolyPolygon( rB2DPolyPoly );
    const PolyPolygon aPixelPolyPolygon = ImplLogicToDevicePixel( aToolsPolyPolygon );
    ImplDrawPolyPolygon( aPixelPolyPolygon.Count(), aPixelPolyPolygon );
}

bool OutputDevice::ImplTryDrawPolyLineDirect(
    const basegfx::B2DPolygon& rB2DPolygon,
    double fLineWidth,
    double fTransparency,
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap)
{
    const basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
    basegfx::B2DVector aB2DLineWidth(1.0, 1.0);

    // transform the line width if used
    if( fLineWidth != 0.0 )
    {
        aB2DLineWidth = aTransform * ::basegfx::B2DVector( fLineWidth, fLineWidth );
    }

    // transform the polygon
    basegfx::B2DPolygon aB2DPolygon(rB2DPolygon);
    aB2DPolygon.transform(aTransform);

    if((mnAntialiasing & ANTIALIASING_PIXELSNAPHAIRLINE)
        && aB2DPolygon.count() < 1000)
    {
        // #i98289#, #i101491#
        // better to remove doubles on device coordinates. Also assume from a given amount
        // of points that the single edges are not long enough to smooth
        aB2DPolygon.removeDoublePoints();
        aB2DPolygon = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges(aB2DPolygon);
    }

    // draw the polyline
    return mpGraphics->DrawPolyLine(
        aB2DPolygon,
        fTransparency,
        aB2DLineWidth,
        eLineJoin,
        eLineCap,
        this);
}

bool OutputDevice::TryDrawPolyLineDirect(
    const basegfx::B2DPolygon& rB2DPolygon,
    double fLineWidth,
    double fTransparency,
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap)
{
    // AW: Do NOT paint empty PolyPolygons
    if(!rB2DPolygon.count())
        return true;

    // we need a graphics
    if( !mpGraphics )
        if( !ImplGetGraphics() )
            return false;

    if( mbInitClipRegion )
        ImplInitClipRegion();

    if( mbOutputClipped )
        return true;

    if( mbInitLineColor )
        ImplInitLineColor();

    const bool bTryAA((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && IsLineColor());

    if(bTryAA)
    {
        if(ImplTryDrawPolyLineDirect(rB2DPolygon, fLineWidth, fTransparency, eLineJoin, eLineCap))
        {
            // worked, add metafile action (if recorded) and return true
            if( mpMetaFile )
            {
                LineInfo aLineInfo;
                if( fLineWidth != 0.0 )
                    aLineInfo.SetWidth( static_cast<long>(fLineWidth+0.5) );
                const Polygon aToolsPolygon( rB2DPolygon );
                mpMetaFile->AddAction( new MetaPolyLineAction( aToolsPolygon, aLineInfo ) );
            }

            return true;
        }
    }

    return false;
}

void OutputDevice::DrawPolyLine(
    const basegfx::B2DPolygon& rB2DPolygon,
    double fLineWidth,
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap)
{

    if( mpMetaFile )
    {
        LineInfo aLineInfo;
        if( fLineWidth != 0.0 )
            aLineInfo.SetWidth( static_cast<long>(fLineWidth+0.5) );
        const Polygon aToolsPolygon( rB2DPolygon );
        mpMetaFile->AddAction( new MetaPolyLineAction( aToolsPolygon, aLineInfo ) );
    }

    // Do not paint empty PolyPolygons
    if(!rB2DPolygon.count() || !IsDeviceOutputNecessary())
        return;

    // we need a graphics
    if( !mpGraphics )
        if( !ImplGetGraphics() )
            return;

    if( mbInitClipRegion )
        ImplInitClipRegion();
    if( mbOutputClipped )
        return;

    if( mbInitLineColor )
        ImplInitLineColor();

    const bool bTryAA((mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW)
        && mpGraphics->supportsOperation(OutDevSupport_B2DDraw)
        && ROP_OVERPAINT == GetRasterOp()
        && IsLineColor());

    // use b2dpolygon drawing if possible
    if(bTryAA && ImplTryDrawPolyLineDirect(rB2DPolygon, fLineWidth, 0.0, eLineJoin, eLineCap))
    {
        return;
    }

    // #i101491#
    // no output yet; fallback to geometry decomposition and use filled polygon paint
    // when line is fat and not too complex. ImplDrawPolyPolygonWithB2DPolyPolygon
    // will do internal needed AA checks etc.
    if(fLineWidth >= 2.5
        && rB2DPolygon.count()
        && rB2DPolygon.count() <= 1000)
    {
        const double fHalfLineWidth((fLineWidth * 0.5) + 0.5);
        const basegfx::B2DPolyPolygon aAreaPolyPolygon(
            basegfx::tools::createAreaGeometry(
                rB2DPolygon,
                fHalfLineWidth,
                eLineJoin,
                eLineCap));
        const Color aOldLineColor(maLineColor);
        const Color aOldFillColor(maFillColor);

        SetLineColor();
        ImplInitLineColor();
        SetFillColor(aOldLineColor);
        ImplInitFillColor();

        // draw usig a loop; else the topology will paint a PolyPolygon
        for(sal_uInt32 a(0); a < aAreaPolyPolygon.count(); a++)
        {
            ImplDrawPolyPolygonWithB2DPolyPolygon(
                basegfx::B2DPolyPolygon(aAreaPolyPolygon.getB2DPolygon(a)));
        }

        SetLineColor(aOldLineColor);
        ImplInitLineColor();
        SetFillColor(aOldFillColor);
        ImplInitFillColor();

        if(bTryAA)
        {
            // when AA it is necessary to also paint the filled polygon's outline
            // to avoid optical gaps
            for(sal_uInt32 a(0); a < aAreaPolyPolygon.count(); a++)
            {
                ImplTryDrawPolyLineDirect(aAreaPolyPolygon.getB2DPolygon(a));
            }
        }
    }
    else
    {
        // fallback to old polygon drawing if needed
        const Polygon aToolsPolygon( rB2DPolygon );
        LineInfo aLineInfo;
        if( fLineWidth != 0.0 )
            aLineInfo.SetWidth( static_cast<long>(fLineWidth+0.5) );
        ImplDrawPolyLineWithLineInfo( aToolsPolygon, aLineInfo );
    }
}

sal_uInt32 OutputDevice::GetGCStackDepth() const
{
    const ImplObjStack* pData = mpObjStack;
    sal_uInt32 nDepth = 0;
    while( pData )
    {
        nDepth++;
        pData = pData->mpPrev;
    }
    return nDepth;
}

void OutputDevice::Push( sal_uInt16 nFlags )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPushAction( nFlags ) );

    ImplObjStack* pData = new ImplObjStack;
    pData->mpPrev = mpObjStack;
    mpObjStack = pData;

    pData->mnFlags = nFlags;

    if ( nFlags & PUSH_LINECOLOR )
    {
        if ( mbLineColor )
            pData->mpLineColor = new Color( maLineColor );
        else
            pData->mpLineColor = NULL;
    }
    if ( nFlags & PUSH_FILLCOLOR )
    {
        if ( mbFillColor )
            pData->mpFillColor = new Color( maFillColor );
        else
            pData->mpFillColor = NULL;
    }
    if ( nFlags & PUSH_FONT )
        pData->mpFont = new Font( maFont );
    if ( nFlags & PUSH_TEXTCOLOR )
        pData->mpTextColor = new Color( GetTextColor() );
    if ( nFlags & PUSH_TEXTFILLCOLOR )
    {
        if ( IsTextFillColor() )
            pData->mpTextFillColor = new Color( GetTextFillColor() );
        else
            pData->mpTextFillColor = NULL;
    }
    if ( nFlags & PUSH_TEXTLINECOLOR )
    {
        if ( IsTextLineColor() )
            pData->mpTextLineColor = new Color( GetTextLineColor() );
        else
            pData->mpTextLineColor = NULL;
    }
    if ( nFlags & PUSH_OVERLINECOLOR )
    {
        if ( IsOverlineColor() )
            pData->mpOverlineColor = new Color( GetOverlineColor() );
        else
            pData->mpOverlineColor = NULL;
    }
    if ( nFlags & PUSH_TEXTALIGN )
        pData->meTextAlign = GetTextAlign();
    if( nFlags & PUSH_TEXTLAYOUTMODE )
        pData->mnTextLayoutMode = GetLayoutMode();
    if( nFlags & PUSH_TEXTLANGUAGE )
        pData->meTextLanguage = GetDigitLanguage();
    if ( nFlags & PUSH_RASTEROP )
        pData->meRasterOp = GetRasterOp();
    if ( nFlags & PUSH_MAPMODE )
    {
        pData->mpMapMode = new MapMode( maMapMode );
        pData->mbMapActive = mbMap;
    }
    if ( nFlags & PUSH_CLIPREGION )
    {
        if ( mbClipRegion )
            pData->mpClipRegion = new Region( maRegion );
        else
            pData->mpClipRegion = NULL;
    }
    if ( nFlags & PUSH_REFPOINT )
    {
        if ( mbRefPoint )
            pData->mpRefPoint = new Point( maRefPoint );
        else
            pData->mpRefPoint = NULL;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->Push();
}

void OutputDevice::Pop()
{

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPopAction() );

    GDIMetaFile*    pOldMetaFile = mpMetaFile;
    ImplObjStack*   pData = mpObjStack;
    mpMetaFile = NULL;

    if ( !pData )
    {
        SAL_WARN( "vcl.gdi", "OutputDevice::Pop() without OutputDevice::Push()" );
        return;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->Pop();

    mpObjStack = pData->mpPrev;

    if ( pData->mnFlags & PUSH_LINECOLOR )
    {
        if ( pData->mpLineColor )
            SetLineColor( *pData->mpLineColor );
        else
            SetLineColor();
    }
    if ( pData->mnFlags & PUSH_FILLCOLOR )
    {
        if ( pData->mpFillColor )
            SetFillColor( *pData->mpFillColor );
        else
            SetFillColor();
    }
    if ( pData->mnFlags & PUSH_FONT )
        SetFont( *pData->mpFont );
    if ( pData->mnFlags & PUSH_TEXTCOLOR )
        SetTextColor( *pData->mpTextColor );
    if ( pData->mnFlags & PUSH_TEXTFILLCOLOR )
    {
        if ( pData->mpTextFillColor )
            SetTextFillColor( *pData->mpTextFillColor );
        else
            SetTextFillColor();
    }
    if ( pData->mnFlags & PUSH_TEXTLINECOLOR )
    {
        if ( pData->mpTextLineColor )
            SetTextLineColor( *pData->mpTextLineColor );
        else
            SetTextLineColor();
    }
    if ( pData->mnFlags & PUSH_OVERLINECOLOR )
    {
        if ( pData->mpOverlineColor )
            SetOverlineColor( *pData->mpOverlineColor );
        else
            SetOverlineColor();
    }
    if ( pData->mnFlags & PUSH_TEXTALIGN )
        SetTextAlign( pData->meTextAlign );
    if( pData->mnFlags & PUSH_TEXTLAYOUTMODE )
        SetLayoutMode( pData->mnTextLayoutMode );
    if( pData->mnFlags & PUSH_TEXTLANGUAGE )
        SetDigitLanguage( pData->meTextLanguage );
    if ( pData->mnFlags & PUSH_RASTEROP )
        SetRasterOp( pData->meRasterOp );
    if ( pData->mnFlags & PUSH_MAPMODE )
    {
        if ( pData->mpMapMode )
            SetMapMode( *pData->mpMapMode );
        else
            SetMapMode();
        mbMap = pData->mbMapActive;
    }
    if ( pData->mnFlags & PUSH_CLIPREGION )
        ImplSetClipRegion( pData->mpClipRegion );
    if ( pData->mnFlags & PUSH_REFPOINT )
    {
        if ( pData->mpRefPoint )
            SetRefPoint( *pData->mpRefPoint );
        else
            SetRefPoint();
    }

    ImplDeleteObjStack( pData );

    mpMetaFile = pOldMetaFile;
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
        if ( !((OutputDevice*)this)->ImplGetGraphics() )
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
        if ( !ImplGetGraphics() )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
