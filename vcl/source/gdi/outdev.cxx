/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <tools/ref.hxx>
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salgdi.hxx>
#include <vcl/sallayout.hxx>
#include <vcl/salframe.hxx>
#include <vcl/salvd.hxx>
#include <vcl/salprn.hxx>
#include <tools/debug.hxx>
#include <vcl/svdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/ctrl.hxx>
#ifndef _POLY_HXX
#include <tools/poly.hxx>
#endif
#include <vcl/region.hxx>
#include <vcl/region.h>
#include <vcl/virdev.hxx>
#include <vcl/window.h>
#include <vcl/window.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/outdata.hxx>
#include <vcl/print.hxx>
#include <vcl/outdev.h>
#include <vcl/outdev.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/sysdata.hxx>

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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/unohelp.hxx>

#include <numeric>

using namespace ::com::sun::star;

DBG_NAME( OutputDevice )
DBG_NAME( Polygon )
DBG_NAME( PolyPolygon )
DBG_NAMEEX( Region )

// -----------------------------------------------------------------------

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

// =======================================================================

#define OUTDEV_POLYPOLY_STACKBUF        32

// =======================================================================

struct ImplObjStack
{
    ImplObjStack*   mpPrev;
    MapMode*        mpMapMode;
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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------


bool OutputDevice::ImplSelectClipRegion( const Region& rRegion, SalGraphics* pGraphics )
{
    DBG_TESTSOLARMUTEX();

    // TODO(Q3): Change from static to plain method - everybody's
    // calling it with pOutDev=this!
    // => done, but only with minimal changes for now => TODO
    OutputDevice* const pOutDev = this;
    if( !pGraphics )
    {
        if( !mpGraphics )
            if( !ImplGetGraphics() )
                return false;
        pGraphics = mpGraphics;
    }

    if( rRegion.HasPolyPolygon()
    && pGraphics->supportsOperation( OutDevSupport_B2DClip ) )
    {
        const ::basegfx::B2DPolyPolygon& rB2DPolyPolygon = rRegion.GetB2DPolyPolygon();
        pGraphics->BeginSetClipRegion( 0 );
        pGraphics->UnionClipRegion( rB2DPolyPolygon, pOutDev );
        pGraphics->EndSetClipRegion();
        return true;
    }

    long                nX;
    long                nY;
    long                nWidth;
    long                nHeight;
    sal_uLong               nRectCount;
    ImplRegionInfo      aInfo;
    sal_Bool                bRegionRect;
    sal_Bool                bClipRegion = sal_True;
    const sal_Bool          bClipDeviceBounds( !pOutDev->GetPDFWriter()
                                           && pOutDev->GetOutDevType() != OUTDEV_PRINTER );

    nRectCount = rRegion.GetRectCount();
    pGraphics->BeginSetClipRegion( nRectCount );
    bRegionRect = rRegion.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
    if( bClipDeviceBounds )
    {
        // #b6520266# Perform actual rect clip against outdev
        // dimensions, to generate empty clips whenever one of the
        // values is completely off the device.
        const long nOffX( pOutDev->mnOutOffX );
        const long nOffY( pOutDev->mnOutOffY );
        const long nDeviceWidth( pOutDev->GetOutputWidthPixel() );
        const long nDeviceHeight( pOutDev->GetOutputHeightPixel() );
        Rectangle aDeviceBounds( nOffX, nOffY,
                                 nOffX+nDeviceWidth-1,
                                 nOffY+nDeviceHeight-1 );
        while ( bRegionRect )
        {
            // #i59315# Limit coordinates passed to sal layer to actual
            // outdev dimensions - everything else bears the risk of
            // overflowing internal coordinates (e.g. the 16 bit wire
            // format of X11).
            Rectangle aTmpRect(nX,nY,nX+nWidth-1,nY+nHeight-1);
            aTmpRect.Intersection(aDeviceBounds);

            if( !aTmpRect.IsEmpty() )
            {
                if ( !pGraphics->UnionClipRegion( aTmpRect.Left(),
                                                  aTmpRect.Top(),
                                                  aTmpRect.GetWidth(),
                                                  aTmpRect.GetHeight(),
                                                  pOutDev ) )
                {
                    bClipRegion = sal_False;
                }
            }
            else
            {
                // #i79850# Fake off-screen clip
                if ( !pGraphics->UnionClipRegion( nDeviceWidth+1,
                                                  nDeviceHeight+1,
                                                  1, 1,
                                                  pOutDev ) )
                {
                    bClipRegion = sal_False;
                }
            }
            DBG_ASSERT( bClipRegion, "OutputDevice::ImplSelectClipRegion() - can't create region" );
            bRegionRect = rRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
        }
    }
    else
    {
        // #i65720# Actually, _don't_ clip anything on printer or PDF
        // export, since output might be visible outside the specified
        // device boundaries.
        while ( bRegionRect )
        {
            if ( !pGraphics->UnionClipRegion( nX, nY, nWidth, nHeight, pOutDev ) )
                bClipRegion = sal_False;
            DBG_ASSERT( bClipRegion, "OutputDevice::ImplSelectClipRegion() - can't cerate region" );
            bRegionRect = rRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
        }
    }
    pGraphics->EndSetClipRegion();
    return bClipRegion;
}


// =======================================================================

Polygon ImplSubdivideBezier( const Polygon& rPoly )
{
    Polygon aPoly;

    // #100127# Use adaptive subdivide instead of fixed 25 segments
    rPoly.AdaptiveSubdivide( aPoly );

    return aPoly;
}

// =======================================================================

PolyPolygon ImplSubdivideBezier( const PolyPolygon& rPolyPoly )
{
    sal_uInt16 i, nPolys = rPolyPoly.Count();
    PolyPolygon aPolyPoly( nPolys );
    for( i=0; i<nPolys; ++i )
        aPolyPoly.Insert( ImplSubdivideBezier( rPolyPoly.GetObject(i) ) );

    return aPolyPoly;
}

// =======================================================================

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
    sal_Bool                bHaveBezier = sal_False;
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
                bHaveBezier = sal_True;

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

// =======================================================================

OutputDevice::OutputDevice() :
    maRegion( REGION_NULL ),
    maFillColor( COL_WHITE ),
    maTextLineColor( COL_TRANSPARENT ),
    maSettings( Application::GetSettings() )
{
    DBG_CTOR( OutputDevice, ImplDbgCheckOutputDevice );

    mpGraphics          = NULL;
    mpUnoGraphicsList   = NULL;
    mpPrevGraphics      = NULL;
    mpNextGraphics      = NULL;
    mpMetaFile          = NULL;
    mpFontEntry         = NULL;
    mpFontCache         = NULL;
    mpFontList          = NULL;
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
    mbMap               = sal_False;
    mbMapIsDefault      = sal_True;
    mbClipRegion        = sal_False;
    mbBackground        = sal_False;
    mbOutput            = sal_True;
    mbDevOutput         = sal_False;
    mbOutputClipped     = sal_False;
    maTextColor         = Color( COL_BLACK );
    maOverlineColor     = Color( COL_TRANSPARENT );
    meTextAlign         = maFont.GetAlign();
    meRasterOp          = ROP_OVERPAINT;
    mnAntialiasing      = 0;
    meTextLanguage      = 0;  // TODO: get default from configuration?
    mbLineColor         = sal_True;
    mbFillColor         = sal_True;
    mbInitLineColor     = sal_True;
    mbInitFillColor     = sal_True;
    mbInitFont          = sal_True;
    mbInitTextColor     = sal_True;
    mbInitClipRegion    = sal_True;
    mbClipRegionSet     = sal_False;
    mbKerning           = sal_False;
    mbNewFont           = sal_True;
    mbTextLines         = sal_False;
    mbTextSpecial       = sal_False;
    mbRefPoint          = sal_False;
    mbEnableRTL         = sal_False;    // mirroring must be explicitly allowed (typically for windows only)

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

// -----------------------------------------------------------------------

OutputDevice::~OutputDevice()
{
    DBG_DTOR( OutputDevice, ImplDbgCheckOutputDevice );

    if ( GetUnoGraphicsList() )
    {
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper( sal_False );
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
        DBG_ERRORFILE( "OutputDevice::~OutputDevice(): OutputDevice::Push() calls != OutputDevice::Pop() calls" );
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
    if( mpFontList
    && (mpFontList != ImplGetSVData()->maGDIData.mpScreenFontList)
    && (ImplGetSVData()->maGDIData.mpScreenFontList != NULL) )
    {
        mpFontList->Clear();
        delete mpFontList;
        mpFontList = NULL;
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

// -----------------------------------------------------------------------

void OutputDevice::EnableRTL( sal_Bool bEnable )
{
    mbEnableRTL = (bEnable != 0);
    if( meOutDevType == OUTDEV_VIRDEV )
    {
        // virdevs default to not mirroring, they will only be set to mirroring
        // under rare circumstances in the UI, eg the valueset control
        // because each virdev has its own SalGraphics we can safely switch the SalGraphics here
        // ...hopefully
        if( ImplGetGraphics() )
            mpGraphics->SetLayout( mbEnableRTL ? SAL_LAYOUT_BIDI_RTL : 0 );
    }

    // convenience: for controls also switch layout mode
    if( dynamic_cast<Control*>(this) != 0 )
        SetLayoutMode( bEnable ? TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_TEXTORIGIN_LEFT : TEXT_LAYOUT_BIDI_LTR | TEXT_LAYOUT_TEXTORIGIN_LEFT);

    Window* pWin = dynamic_cast<Window*>(this);
    if( pWin )
        pWin->StateChanged( STATE_CHANGE_MIRRORING );

    if( mpAlphaVDev )
        mpAlphaVDev->EnableRTL( bEnable );
}

sal_Bool OutputDevice::ImplHasMirroredGraphics()
{
   // HOTFIX for #i55719#
   if( meOutDevType == OUTDEV_PRINTER )
       return sal_False;

   return ( ImplGetGraphics() && (mpGraphics->GetLayout() & SAL_LAYOUT_BIDI_RTL) );
}

// note: the coordiantes to be remirrored are in frame coordiantes !

void    OutputDevice::ImplReMirror( Point &rPoint ) const
{
    rPoint.X() = mnOutOffX + mnOutWidth - 1 - rPoint.X() + mnOutOffX;
}
void    OutputDevice::ImplReMirror( Rectangle &rRect ) const
{
    long nWidth = rRect.nRight - rRect.nLeft;

    //long lc_x = rRect.nLeft - mnOutOffX;    // normalize
    //lc_x = mnOutWidth - nWidth - 1 - lc_x;  // mirror
    //rRect.nLeft = lc_x + mnOutOffX;         // re-normalize

    rRect.nLeft = mnOutOffX + mnOutWidth - nWidth - 1 - rRect.nLeft + mnOutOffX;
    rRect.nRight = rRect.nLeft + nWidth;
}
void    OutputDevice::ImplReMirror( Region &rRegion ) const
{
    long                nX;
    long                nY;
    long                nWidth;
    long                nHeight;
    ImplRegionInfo      aInfo;
    sal_Bool                bRegionRect;
    Region              aMirroredRegion;

    bRegionRect = rRegion.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
    while ( bRegionRect )
    {
        Rectangle aRect( Point(nX, nY), Size(nWidth, nHeight) );
        ImplReMirror( aRect );
        aMirroredRegion.Union( aRect );
        bRegionRect = rRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
    }
    rRegion = aMirroredRegion;
}


// -----------------------------------------------------------------------

int OutputDevice::ImplGetGraphics() const
{
    DBG_TESTSOLARMUTEX();

    if ( mpGraphics )
        return sal_True;

    mbInitLineColor     = sal_True;
    mbInitFillColor     = sal_True;
    mbInitFont          = sal_True;
    mbInitTextColor     = sal_True;
    mbInitClipRegion    = sal_True;

    ImplSVData* pSVData = ImplGetSVData();
    if ( meOutDevType == OUTDEV_WINDOW )
    {
        Window* pWindow = (Window*)this;

        mpGraphics = pWindow->mpWindowImpl->mpFrame->GetGraphics();
        // try harder if no wingraphics was available directly
        if ( !mpGraphics )
        {
            // find another output device in the same frame
            OutputDevice* pReleaseOutDev = pSVData->maGDIData.mpLastWinGraphics;
            while ( pReleaseOutDev )
            {
                if ( ((Window*)pReleaseOutDev)->mpWindowImpl->mpFrame == pWindow->mpWindowImpl->mpFrame )
                    break;
                pReleaseOutDev = pReleaseOutDev->mpPrevGraphics;
            }

            if ( pReleaseOutDev )
            {
                // steal the wingraphics from the other outdev
                mpGraphics = pReleaseOutDev->mpGraphics;
                pReleaseOutDev->ImplReleaseGraphics( sal_False );
            }
            else
            {
                // if needed retry after releasing least recently used wingraphics
                while ( !mpGraphics )
                {
                    if ( !pSVData->maGDIData.mpLastWinGraphics )
                        break;
                    pSVData->maGDIData.mpLastWinGraphics->ImplReleaseGraphics();
                    mpGraphics = pWindow->mpWindowImpl->mpFrame->GetGraphics();
                }
            }
        }

        // update global LRU list of wingraphics
        if ( mpGraphics )
        {
            mpNextGraphics = pSVData->maGDIData.mpFirstWinGraphics;
            pSVData->maGDIData.mpFirstWinGraphics = const_cast<OutputDevice*>(this);
            if ( mpNextGraphics )
                mpNextGraphics->mpPrevGraphics = const_cast<OutputDevice*>(this);
            if ( !pSVData->maGDIData.mpLastWinGraphics )
                pSVData->maGDIData.mpLastWinGraphics = const_cast<OutputDevice*>(this);
        }
    }
    else if ( meOutDevType == OUTDEV_VIRDEV )
    {
        const VirtualDevice* pVirDev = (const VirtualDevice*)this;

        if ( pVirDev->mpVirDev )
        {
            mpGraphics = pVirDev->mpVirDev->GetGraphics();
            // if needed retry after releasing least recently used virtual device graphics
            while ( !mpGraphics )
            {
                if ( !pSVData->maGDIData.mpLastVirGraphics )
                    break;
                pSVData->maGDIData.mpLastVirGraphics->ImplReleaseGraphics();
                mpGraphics = pVirDev->mpVirDev->GetGraphics();
            }
            // update global LRU list of virtual device graphics
            if ( mpGraphics )
            {
                mpNextGraphics = pSVData->maGDIData.mpFirstVirGraphics;
                pSVData->maGDIData.mpFirstVirGraphics = const_cast<OutputDevice*>(this);
                if ( mpNextGraphics )
                    mpNextGraphics->mpPrevGraphics = const_cast<OutputDevice*>(this);
                if ( !pSVData->maGDIData.mpLastVirGraphics )
                    pSVData->maGDIData.mpLastVirGraphics = const_cast<OutputDevice*>(this);
            }
        }
    }
    else if ( meOutDevType == OUTDEV_PRINTER )
    {
        const Printer* pPrinter = (const Printer*)this;

        if ( pPrinter->mpJobGraphics )
            mpGraphics = pPrinter->mpJobGraphics;
        else if ( pPrinter->mpDisplayDev )
        {
            const VirtualDevice* pVirDev = pPrinter->mpDisplayDev;
            mpGraphics = pVirDev->mpVirDev->GetGraphics();
            // if needed retry after releasing least recently used virtual device graphics
            while ( !mpGraphics )
            {
                if ( !pSVData->maGDIData.mpLastVirGraphics )
                    break;
                pSVData->maGDIData.mpLastVirGraphics->ImplReleaseGraphics();
                mpGraphics = pVirDev->mpVirDev->GetGraphics();
            }
            // update global LRU list of virtual device graphics
            if ( mpGraphics )
            {
                mpNextGraphics = pSVData->maGDIData.mpFirstVirGraphics;
                pSVData->maGDIData.mpFirstVirGraphics = const_cast<OutputDevice*>(this);
                if ( mpNextGraphics )
                    mpNextGraphics->mpPrevGraphics = const_cast<OutputDevice*>(this);
                if ( !pSVData->maGDIData.mpLastVirGraphics )
                    pSVData->maGDIData.mpLastVirGraphics = const_cast<OutputDevice*>(this);
            }
        }
        else
        {
            mpGraphics = pPrinter->mpInfoPrinter->GetGraphics();
            // if needed retry after releasing least recently used printer graphics
            while ( !mpGraphics )
            {
                if ( !pSVData->maGDIData.mpLastPrnGraphics )
                    break;
                pSVData->maGDIData.mpLastPrnGraphics->ImplReleaseGraphics();
                mpGraphics = pPrinter->mpInfoPrinter->GetGraphics();
            }
            // update global LRU list of printer graphics
            if ( mpGraphics )
            {
                mpNextGraphics = pSVData->maGDIData.mpFirstPrnGraphics;
                pSVData->maGDIData.mpFirstPrnGraphics = const_cast<OutputDevice*>(this);
                if ( mpNextGraphics )
                    mpNextGraphics->mpPrevGraphics = const_cast<OutputDevice*>(this);
                if ( !pSVData->maGDIData.mpLastPrnGraphics )
                    pSVData->maGDIData.mpLastPrnGraphics = const_cast<OutputDevice*>(this);
            }
        }
    }

    if ( mpGraphics )
    {
        mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp), ROP_INVERT == meRasterOp );
        mpGraphics->setAntiAliasB2DDraw(mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW);
        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplReleaseGraphics( sal_Bool bRelease )
{
    DBG_TESTSOLARMUTEX();

    if ( !mpGraphics )
        return;

    // release the fonts of the physically released graphics device
    if( bRelease )
    {
#ifndef UNX
        // HACK to fix an urgent P1 printing issue fast
        // WinSalPrinter does not respect GetGraphics/ReleaseGraphics conventions
        // so Printer::mpGraphics often points to a dead WinSalGraphics
        // TODO: fix WinSalPrinter's GetGraphics/ReleaseGraphics handling
        if( meOutDevType != OUTDEV_PRINTER )
#endif
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

    ImplSVData* pSVData = ImplGetSVData();
    if ( meOutDevType == OUTDEV_WINDOW )
    {
        Window* pWindow = (Window*)this;

        if ( bRelease )
            pWindow->mpWindowImpl->mpFrame->ReleaseGraphics( mpGraphics );
        // remove from global LRU list of window graphics
        if ( mpPrevGraphics )
            mpPrevGraphics->mpNextGraphics = mpNextGraphics;
        else
            pSVData->maGDIData.mpFirstWinGraphics = mpNextGraphics;
        if ( mpNextGraphics )
            mpNextGraphics->mpPrevGraphics = mpPrevGraphics;
        else
            pSVData->maGDIData.mpLastWinGraphics = mpPrevGraphics;
    }
    else if ( meOutDevType == OUTDEV_VIRDEV )
    {
        VirtualDevice* pVirDev = (VirtualDevice*)this;

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
    }
    else if ( meOutDevType == OUTDEV_PRINTER )
    {
        Printer* pPrinter = (Printer*)this;

        if ( !pPrinter->mpJobGraphics )
        {
            if ( pPrinter->mpDisplayDev )
            {
                VirtualDevice* pVirDev = pPrinter->mpDisplayDev;
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
            }
            else
            {
                if ( bRelease )
                    pPrinter->mpInfoPrinter->ReleaseGraphics( mpGraphics );
                // remove from global LRU list of printer graphics
                if ( mpPrevGraphics )
                    mpPrevGraphics->mpNextGraphics = mpNextGraphics;
                else
                    pSVData->maGDIData.mpFirstPrnGraphics = mpNextGraphics;
                if ( mpNextGraphics )
                    mpNextGraphics->mpPrevGraphics = mpPrevGraphics;
                else
                    pSVData->maGDIData.mpLastPrnGraphics = mpPrevGraphics;
           }
        }
    }

    mpGraphics      = NULL;
    mpPrevGraphics  = NULL;
    mpNextGraphics  = NULL;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

sal_Bool OutputDevice::ImplIsRecordLayout() const
{
    return mpOutDevData && mpOutDevData->mpRecordLayout;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

    mbInitLineColor = sal_False;
}

// -----------------------------------------------------------------------

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

    mbInitFillColor = sal_False;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitClipRegion()
{
    DBG_TESTSOLARMUTEX();

    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        Window* pWindow = (Window*)this;
        Region  aRegion;

        // Hintergrund-Sicherung zuruecksetzen
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
                ImplReMirror ( aRegion );
        }
        if ( mbClipRegion )
            aRegion.Intersect( ImplPixelToDevicePixel( maRegion ) );
        if ( aRegion.IsEmpty() )
            mbOutputClipped = sal_True;
        else
        {
            mbOutputClipped = sal_False;
            ImplSelectClipRegion( aRegion );
        }
        mbClipRegionSet = sal_True;
    }
    else
    {
        if ( mbClipRegion )
        {
            if ( maRegion.IsEmpty() )
                mbOutputClipped = sal_True;
            else
            {
                mbOutputClipped = sal_False;
                ImplSelectClipRegion(
                                      // #102532# Respect output offset also for clip region
                                      ImplPixelToDevicePixel( maRegion ) );
            }

            mbClipRegionSet = sal_True;
        }
        else
        {
            if ( mbClipRegionSet )
            {
                mpGraphics->ResetClipRegion();
                mbClipRegionSet = sal_False;
            }

            mbOutputClipped = sal_False;
        }
    }

    mbInitClipRegion = sal_False;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplSetClipRegion( const Region* pRegion )
{
    DBG_TESTSOLARMUTEX();

    if ( !pRegion )
    {
        if ( mbClipRegion )
        {
            maRegion            = Region( REGION_NULL );
            mbClipRegion        = sal_False;
            mbInitClipRegion    = sal_True;
        }
    }
    else
    {
        maRegion            = *pRegion;
        mbClipRegion        = sal_True;
        mbInitClipRegion    = sal_True;
    }
}

// -----------------------------------------------------------------------

namespace
{
    inline int iround( float x )
    {
        union
        {
            float f;
            sal_Int32 i;
        };
        f = x;
        sal_Int32 exponent = (127 + 31) - ((i >> 23) & 0xFF);
        sal_Int32 r = ((sal_Int32(i) << 8) | (1U << 31)) >> exponent;
        r &= ((exponent - 32) >> 31);
        sal_Int32 sign = i >> 31;
        return r = (r ^ sign) - sign;
    }

    inline int floorDiv(int a, int b)
    {
        if(b == 0)
            return 0x80000000;
        if(a >= 0)
            return a / b;
        int q = -(-a / b);  // quotient
        int r = -a % b;     // remainder
        if(r)
            q--;
        return q;
    }

    inline int floorMod( int a, int b )
    {
        if(b == 0)
            return 0x80000000;
        if(a >= 0)
            return a % b;
        int r = -a % b;     // remainder
        if(r)
            r = b - r;
        return r;
    }

    inline int ceilDiv( int a, int b )
    {
        if(b == 0)
            return 0x80000000;
        a += - 1 + b;
        if(a >= 0)
            return a / b;
        int q = -(-a / b);  // quotient
        int r = -a % b;     // remainder
        if(r)
            q--;
        return q;
    }

    inline int ceilMod( int a, int b )
    {
        if(b == 0)
            return 0x80000000;
        a += - 1 + b;
        if(a >= 0)
            return (a % b) + 1 - b;
        int r = -a % b;
        if(r)
            r = b - r;
        return r + 1 - b;
    }

    inline int ceilFix4(int x) { return (x + 0xF) & 0xFFFFFFF0; }

    struct vertex
    {
        float x,y;
        inline vertex( const Point &p )
            : x((float)p.getX()),y((float)p.getY()) {}
    };

    template<class T> inline void swap(T &a, T &b) { T t=a; a=b; b=t; }

    class SpanIterator
    {
        public:

            SpanIterator( sal_Int32 *pTable, size_t dwPitch, sal_Int32 dwNumScanlines );
            std::pair<sal_Int32,sal_Int32> GetNextSpan( void );
            sal_Int32 GetNumRemainingScanlines( void );
            sal_Int32 GetNumEqualScanlines( void );
            SpanIterator &operator++ ();
            SpanIterator &Skip( sal_Int32 dwNumScanlines );
            sal_Int32 GetRemainingSpans( void ) const { return maNumSpans; }

        private:

            sal_Int32 *mpTable;
            sal_Int32 *mpSpanArray;
            sal_Int32 maNumSpans;
            sal_Int32 maRemainingScanlines;
            size_t maPitch;
    };

    inline SpanIterator::SpanIterator( sal_Int32 *pTable, size_t dwPitch, sal_Int32 dwNumScanlines )
        : mpTable(pTable),maRemainingScanlines(dwNumScanlines),maPitch(dwPitch)
    {
        sal_Int32 *pNumSpans = mpTable;
        mpSpanArray = reinterpret_cast<sal_Int32 *>(pNumSpans+2);
        maNumSpans = *pNumSpans;
    }

    inline SpanIterator &SpanIterator::operator++ ()
    {
        --maRemainingScanlines;
        mpTable += maPitch;
        sal_Int32 *pNumSpans = mpTable;
        mpSpanArray = reinterpret_cast<sal_Int32 *>(pNumSpans+2);
        maNumSpans = *pNumSpans;
        return (*this);
    }

    inline SpanIterator &SpanIterator::Skip( sal_Int32 dwNumScanlines )
    {
        // don't skip more scanlines than there are...
        if(dwNumScanlines > maRemainingScanlines)
            dwNumScanlines = maRemainingScanlines;

        // skip in one fellow swoop...
        maRemainingScanlines -= dwNumScanlines;
        mpTable += maPitch * dwNumScanlines;

        // initialize necessary query fields...
        sal_Int32 *pNumSpans = mpTable;
        mpSpanArray = reinterpret_cast<sal_Int32 *>(pNumSpans+2);
        maNumSpans = *pNumSpans;
        return (*this);
    }

    inline std::pair<sal_Int32,sal_Int32> SpanIterator::GetNextSpan( void )
    {
        sal_Int32 x(0);
        sal_Int32 w(0);
        if(maNumSpans)
        {
            x = *mpSpanArray++;
            w = *mpSpanArray++;
            --maNumSpans;
        }
        return std::pair<sal_Int32,sal_Int32>(x,w);
    }

    inline sal_Int32 SpanIterator::GetNumEqualScanlines( void )
    {
        return mpTable[1];
    }

    inline sal_Int32 SpanIterator::GetNumRemainingScanlines( void )
    {
        return maRemainingScanlines;
    }

    class ScanlineContainer
    {

        public:

            ScanlineContainer( sal_uInt32 dwNumScanlines,
                               sal_uInt32 dwNumSpansPerScanline );

            ~ScanlineContainer( void );

            void InsertSpan( sal_Int32 y, sal_Int32 lx, sal_Int32 rx );

            SpanIterator Iterate( void ) const { return SpanIterator(mpTable,maPitch,maNumScanlines); }

            inline sal_uInt32 GetNumSpans( void ) const { return maNumberOfSpans; }

            void Consolidate( void );

        private:

            // the span table will assist in determinate exactly how many clipping
            // regions [that is *spans*] we will end up with.
            // the counter for this purpose is right ahead.
            sal_uInt32 maNumberOfSpans;

            struct span
            {
                sal_Int32 x;
                sal_Int32 w;
            };

            sal_uInt32 maNumScanlines;
            sal_uInt32 maNumSpansPerScanline;
            sal_Int32 *mpTable;
            size_t maPitch;
    };

    ScanlineContainer::ScanlineContainer( sal_uInt32 dwNumScanlines,
                                          sal_uInt32 dwNumSpansPerScanline ) : maNumScanlines(dwNumScanlines),
                                                                               maNumSpansPerScanline(dwNumSpansPerScanline)
    {
        // #128002# add one scanline buffer at the end, as
        // SpanIterator::Skip reads two bytes past the end.
        ++dwNumScanlines;

        // since each triangle could possibly add another span
        // we can calculate the upper limit by [num scanlines * num triangles].
        const sal_uInt32 dwNumPossibleRegions = dwNumScanlines*dwNumSpansPerScanline;

        // calculate the number of bytes the span table will consume
        const size_t dwTableSize = dwNumPossibleRegions*sizeof(span)+dwNumScanlines*(sizeof(sal_Int32)<<1);

        // allocate the span table [on the stack]
        mpTable = static_cast<sal_Int32 *>(rtl_allocateMemory(dwTableSize));

        // calculate the table pitch, that is how many int's do i need to get from a scanline to the next.
        maPitch = (dwNumSpansPerScanline*sizeof(span)/sizeof(sal_Int32))+2;

        // we need to initialize the table here.
        // the first *int* on each scanline tells us how many spans are on it.
        sal_Int32 *pNumSpans = mpTable;
        for(unsigned int i=0; i<dwNumScanlines; ++i)
        {
            pNumSpans[0] = 0;
            pNumSpans[1] = 0;
            pNumSpans += maPitch;
        }

        maNumberOfSpans = 0;
    }

    ScanlineContainer::~ScanlineContainer( void )
    {
        rtl_freeMemory(mpTable);
    }

    void ScanlineContainer::InsertSpan( sal_Int32 y, sal_Int32 lx, sal_Int32 rx )
    {
        // there's new incoming span which we need to store in the table.
        // first see if its width contributes a valid span.
        if(sal_Int32 dwSpanWidth = rx-lx)
        {
            // first select the appropriate scanline the new span.
            sal_Int32 *pNumSpans = mpTable+(y*maPitch);
            span *pSpanArray = reinterpret_cast<span *>(pNumSpans+2);

            // retrieve the number of already contained spans.
            sal_Int32 dwNumSpan = *pNumSpans;

            // since we need to sort the spans from top to bottom
            // and left to right, we need to find the correct location
            // in the table.
            sal_Int32 dwIndex = 0;
            while(dwIndex<dwNumSpan)
            {
                // since we would like to avoid unnecessary spans
                // we try to consolidate them if possible.
                // consolidate with right neighbour
                if(pSpanArray[dwIndex].x == rx)
                {
                    pSpanArray[dwIndex].x = lx;
                    pSpanArray[dwIndex].w += dwSpanWidth;
                    return;
                }

                // consolidate with left neighbour
                if((pSpanArray[dwIndex].x+pSpanArray[dwIndex].w) == lx)
                {
                    pSpanArray[dwIndex].w += rx-lx;
                    return;
                }

                // no consolidation possible, either this is a completely
                // seperate span or it is the first in the list.
                if(pSpanArray[dwIndex].x > lx)
                    break;

                // forward to next element in the list.
                ++dwIndex;
            }

            // if we reach here, the new span needs to be stored
            // in the table, increase the number of spans in the
            // current scanline.
            *pNumSpans = dwNumSpan+1;

            // keep the list of spans in sorted order. 'dwIndex'
            // is where we want to store the new span. 'dwNumSpan'
            // is the number of spans already there. now we need
            // to move the offending spans out of the way.
            while(dwIndex != dwNumSpan)
            {
                pSpanArray[dwNumSpan].x = pSpanArray[dwNumSpan-1].x;
                pSpanArray[dwNumSpan].w = pSpanArray[dwNumSpan-1].w;
                --dwNumSpan;
            }

            // insert the new span
            pSpanArray[dwIndex].x = lx;
            pSpanArray[dwIndex].w = rx-lx;

            // remember the total number of spans in the table.
            ++maNumberOfSpans;
        }
    }

    void ScanlineContainer::Consolidate( void )
    {
        sal_Int32 *pScanline = mpTable;

        sal_Int32 dwRemaining = maNumScanlines;
        while(dwRemaining)
        {
            sal_Int32 dwNumSpans = pScanline[0];
            sal_Int32 *pSpanArray = pScanline+2;

            sal_Int32 dwRest = dwRemaining-1;
            sal_Int32 *pNext = pScanline;
            while(dwRest)
            {
                pNext += maPitch;
                sal_Int32 dwNumNextSpans = pNext[0];
                sal_Int32 *pSpanArrayNext = pNext+2;
                if(dwNumSpans != dwNumNextSpans)
                    break;

                sal_Int32 dwCompare = dwNumSpans<<1;
                while(dwCompare)
                {
                    if(pSpanArray[dwCompare-1] != pSpanArrayNext[dwCompare-1])
                        break;
                    --dwCompare;
                }
                if(dwCompare)
                    break;

                --dwRest;
            }

            const sal_Int32 dwNumEqualScanlines(dwRemaining-dwRest);
            pScanline[1] = dwNumEqualScanlines;
            pScanline += maPitch*dwNumEqualScanlines;
            dwRemaining -= dwNumEqualScanlines;

            // since we track the total number of spans to generate,
            // we need to account for consolidated scanlines here.
            if(dwNumEqualScanlines > 1)
                maNumberOfSpans -= dwNumSpans * (dwNumEqualScanlines-1);
        }
    }
}

// TODO: we should consider passing a basegfx b2dpolypolygon here to
// ensure that the signature isn't misleading.
// if we could pass a b2dpolypolygon here, we could easily triangulate it.
void OutputDevice::ImplSetTriangleClipRegion( const PolyPolygon &rPolyPolygon )
{
    DBG_TESTSOLARMUTEX();

    if(!(IsDeviceOutputNecessary()))
        return;
    if(!(mpGraphics))
        if(!(ImplGetGraphics()))
            return;

    if( mpGraphics->supportsOperation( OutDevSupport_B2DClip ) )
    {
#if 0
        ::basegfx::B2DPolyPolygon aB2DPolyPolygon = rPolyPolygon.getB2DPolyPolygon();
#else
         // getB2DPolyPolygon() "optimizes away" some points
         // which prevents reliable undoing of the "triangle thingy" parameter
         // so the toolspoly -> b2dpoly conversion has to be done manually
        ::basegfx::B2DPolyPolygon aB2DPolyPolygon;
        for( sal_uInt16 nPolyIdx = 0; nPolyIdx < rPolyPolygon.Count(); ++nPolyIdx )
        {
            const Polygon& rPolygon = rPolyPolygon[ nPolyIdx ];
            ::basegfx::B2DPolygon aB2DPoly;
              for( sal_uInt16 nPointIdx = 0; nPointIdx < rPolygon.GetSize(); ++nPointIdx )
              {
                  const Point& rPoint = rPolygon[ nPointIdx ];
                  const ::basegfx::B2DPoint aB2DPoint( rPoint.X(), rPoint.Y() );
                  aB2DPoly.append( aB2DPoint );
              }
              aB2DPolyPolygon.append( aB2DPoly );
        }
#endif

        const ::basegfx::B2DHomMatrix aTransform = ImplGetDeviceTransformation();
        aB2DPolyPolygon.transform( aTransform );

        // the rPolyPolygon argument is a "triangle thingy"
        // so convert it to a normal polypolyon first
        ::basegfx::B2DPolyPolygon aPolyTriangle;
        const int nPolyCount = aB2DPolyPolygon.count();
        for( int nPolyIdx = 0; nPolyIdx < nPolyCount; ++nPolyIdx )
        {
            const ::basegfx::B2DPolygon rPolygon = aB2DPolyPolygon.getB2DPolygon( nPolyIdx );
            const int nPointCount = rPolygon.count();
            for( int nPointIdx = 0; nPointIdx+2 < nPointCount; nPointIdx +=3 )
            {
                ::basegfx::B2DPolygon aTriangle;
                aTriangle.append( rPolygon.getB2DPoint( nPointIdx+0 ) );
                aTriangle.append( rPolygon.getB2DPoint( nPointIdx+1 ) );
                aTriangle.append( rPolygon.getB2DPoint( nPointIdx+2 ) );
                aPolyTriangle.append( aTriangle );
            }
        }

        // now set the clip region with the real polypolygon
        mpGraphics->BeginSetClipRegion( 0 );
        mpGraphics->UnionClipRegion( aPolyTriangle, this );
        mpGraphics->EndSetClipRegion();

        // and mark the clip status as ready
        mbOutputClipped = sal_False;
        mbClipRegion = sal_True;
        mbClipRegionSet = sal_True;
        mbInitClipRegion = sal_False;
        return;
    }

    sal_Int32 offset_x = 0;
    sal_Int32 offset_y = 0;
    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        offset_x = mnOutOffX+mnOutOffOrigX;
        offset_y = mnOutOffY+mnOutOffOrigY;
    }

    // first of all we need to know the upper limit
    // of the amount of possible clipping regions.
    sal_Int32 maxy = SAL_MIN_INT32;
    sal_Int32 miny = SAL_MAX_INT32;
    sal_uInt32 dwNumTriangles = 0;
    for(sal_uInt16 i=0; i<rPolyPolygon.Count(); ++i)
    {
        const Polygon &rPoly = rPolyPolygon.GetObject(i);
        const sal_Int32 dwNumVertices = rPoly.GetSize();
        if(!(dwNumVertices % 3))
        {
            for(sal_uInt16 j=0; j<rPoly.GetSize(); ++j)
            {
                const Point &p = rPoly.GetPoint(j);
                if(p.Y() < miny)
                    miny = p.Y();
                if(p.Y() > maxy)
                    maxy = p.Y();
            }
            dwNumTriangles += dwNumVertices / 3;
        }
    }

    const sal_uInt32 dwNumScanlines = (maxy-miny);
    if(!(dwNumScanlines))
    {
        // indicates that no output needs to be produced
        // since the clipping region did not provide any
        // visible areas.
        mbOutputClipped = sal_True;

        // indicates that a clip region has been
        // presented to the output device.
        mbClipRegion = sal_True;

        // indicates that the set clipping region
        // has been processed.
        mbClipRegionSet = sal_True;

        // under 'normal' circumstances a new clipping region
        // needs to be processed by ImplInitClipRegion(),
        // which we need to circumvent.
        mbInitClipRegion = sal_False;
        return;
    }

    // this container provides all services we need to
    // efficiently store/retrieve spans from the table.
    const sal_uInt32 dwNumSpansPerScanline = dwNumTriangles;
    ScanlineContainer container(dwNumScanlines,dwNumSpansPerScanline);

    // convert the incoming polypolygon to spans, we assume that
    // the polypolygon has already been triangulated since we don't
    // want to use the basegfx-types here. this could be leveraged
    // after the tools-types had been removed.
    for(sal_uInt16 i=0; i<rPolyPolygon.Count(); ++i)
    {
        const Polygon &rPoly = rPolyPolygon.GetObject(i);
        const sal_uInt16 dwNumVertices = rPoly.GetSize();
        if(!(dwNumVertices % 3))
        {
            for(sal_uInt16 j=0; j<dwNumVertices; j+=3)
            {
                const Point &p0 = rPoly.GetPoint(j+0);
                const Point &p1 = rPoly.GetPoint(j+1);
                const Point &p2 = rPoly.GetPoint(j+2);

                // what now follows is an extremely fast triangle
                // rasterizer from which all tricky and interesting
                // parts were forcibly amputated.
                // note: top.left fill-convention
                vertex v0(p0);
                vertex v1(p1);
                vertex v2(p2);

                //sprintf(string,"[%f,%f] [%f,%f] [%f,%f]\n",v0.x,v0.y,v1.x,v1.y,v2.x,v2.y);
                //OSL_TRACE(string);

                if(v0.y > v2.y) ::swap(v0, v2);
                if(v1.y > v2.y) ::swap(v1, v2);
                if(v0.y > v1.y) ::swap(v0, v1);

                const float float2fixed(16.0f);

                // vertex coordinates of the triangle [28.4 fixed-point]
                const int i4x0 = iround(float2fixed * (v0.x - 0.5f));
                const int i4y0 = iround(float2fixed * (v0.y - 0.5f));
                const int i4x1 = iround(float2fixed * (v1.x - 0.5f));
                const int i4y1 = iround(float2fixed * (v1.y - 0.5f));
                const int i4x2 = iround(float2fixed * (v2.x - 0.5f));
                const int i4y2 = iround(float2fixed * (v2.y - 0.5f));

                // vertex coordinate deltas [28.4 fixed-point]
                const int i4dx12 = i4x1-i4x0;
                const int i4dy12 = i4y1-i4y0;
                const int i4dx13 = i4x2-i4x0;
                const int i4dy13 = i4y2-i4y0;
                const int i4dx23 = i4x2-i4x1;
                const int i4dy23 = i4y2-i4y1;

                // slope of edges [quotient,remainder]
                const int mq12 = floorDiv(i4dx12 << 4, i4dy12 << 4);
                const int mq13 = floorDiv(i4dx13 << 4, i4dy13 << 4);
                const int mq23 = floorDiv(i4dx23 << 4, i4dy23 << 4);
                const int mr12 = floorMod(i4dx12 << 4, i4dy12 << 4);
                const int mr13 = floorMod(i4dx13 << 4, i4dy13 << 4);
                const int mr23 = floorMod(i4dx23 << 4, i4dy23 << 4);

                // convert the vertical coordinates back to integers.
                // according to the top-left fillrule we need to step
                // the coordinates to the ceiling.
                const int y0 = (i4y0+15)>>4;
                const int y1 = (i4y1+15)>>4;
                const int y2 = (i4y2+15)>>4;

                // calculate the value of the horizontal coordinate
                // from the edge that 'spans' the triangle.
                const int x = ceilDiv(i4dx13*i4dy12 + i4x0*i4dy13, i4dy13);

                // this will hold the horizontal coordinates
                // of the seperate spans during the rasterization process.
                int lx,rx;

                // this pair will serve as the error accumulator while
                // we step along the edges.
                int ld,rd,lD,rD;

                // these are the edge and error stepping values that
                // will be used while stepping.
                int lQ,rQ,lR,rR;

                if(i4x1 < x)
                {
                    lx = ceilDiv(i4dx12 * (ceilFix4(i4y0) - i4y0) + i4x0 * i4dy12, i4dy12 << 4);
                    ld = ceilMod(i4dx12 * (ceilFix4(i4y0) - i4y0) + i4x0 * i4dy12, i4dy12 << 4);
                    rx = ceilDiv(i4dx13 * (ceilFix4(i4y0) - i4y0) + i4x0 * i4dy13, i4dy13 << 4);
                    rd = ceilMod(i4dx13 * (ceilFix4(i4y0) - i4y0) + i4x0 * i4dy13, i4dy13 << 4);
                    lQ = mq12;
                    rQ = mq13;
                    lR = mr12;
                    rR = mr13;
                    lD = i4dy12 << 4;
                    rD = i4dy13 << 4;
                }
                else
                {
                    lx = ceilDiv(i4dx13 * (ceilFix4(i4y0) - i4y0) + i4x0 * i4dy13, i4dy13 << 4);
                    ld = ceilMod(i4dx13 * (ceilFix4(i4y0) - i4y0) + i4x0 * i4dy13, i4dy13 << 4);
                    rx = ceilDiv(i4dx12 * (ceilFix4(i4y0) - i4y0) + i4x0 * i4dy12, i4dy12 << 4);
                    rd = ceilMod(i4dx12 * (ceilFix4(i4y0) - i4y0) + i4x0 * i4dy12, i4dy12 << 4);
                    lQ = mq13;
                    rQ = mq12;
                    lR = mr13;
                    rR = mr12;
                    lD = i4dy13 << 4;
                    rD = i4dy12 << 4;
                }

                for(signed int y=y0; y<y1; y++)
                {
                    container.InsertSpan(y-miny,lx,rx);

                    lx += lQ; ld += lR;
                    if(ld > 0) { ld -= lD; lx += 1; }
                    rx += rQ; rd += rR;
                    if(rd > 0) { rd -= rD; rx += 1; }
                }

                if(i4x1 < x)
                {
                    lx = ceilDiv(i4dx23 * (ceilFix4(i4y1) - i4y1) + i4x1 * i4dy23, i4dy23 << 4);
                    ld = ceilMod(i4dx23 * (ceilFix4(i4y1) - i4y1) + i4x1 * i4dy23, i4dy23 << 4);
                    rx = ceilDiv(i4dx13 * (ceilFix4(i4y1) - i4y0) + i4x0 * i4dy13, i4dy13 << 4);
                    rd = ceilMod(i4dx13 * (ceilFix4(i4y1) - i4y0) + i4x0 * i4dy13, i4dy13 << 4);
                    lQ = mq23;
                    lR = mr23;
                    lD = i4dy23 << 4;
                }
                else
                {
                    rx = ceilDiv(i4dx23 * (ceilFix4(i4y1) - i4y1) + i4x1 * i4dy23, i4dy23 << 4);
                    rd = ceilMod(i4dx23 * (ceilFix4(i4y1) - i4y1) + i4x1 * i4dy23, i4dy23 << 4);
                    rQ = mq23;
                    rR = mr23;
                    rD = i4dy23 << 4;
                }

                for(signed int y=y1; y<y2; y++)
                {
                    container.InsertSpan(y-miny,lx,rx);

                    lx += lQ; ld += lR;
                    if(ld > 0) { ld -= lD; lx += 1; }
                    rx += rQ; rd += rR;
                    if(rd > 0) { rd -= rD; rx += 1; }
                }
            }
        }
    }

    // now try to consolidate as many scanlines as possible.
    // please note that this will probably change the number
    // of spans [at least this is why we do all this hassle].
    // so, if you use 'consolidate' you should *use* this
    // information during iteration, because the 'graphics'
    // object we tell all those regions about is a bit,
    // hm, how to say, *picky* if you supply not correctly
    // the amount of regions.
    container.Consolidate();

    // now forward the spantable to the graphics handler.
    SpanIterator it(container.Iterate());
    mpGraphics->BeginSetClipRegion( container.GetNumSpans() );
    while(miny < maxy)
    {
        const sal_Int32 dwNumEqual(it.GetNumEqualScanlines());
        while(it.GetRemainingSpans())
        {
            // retrieve the next span [x-coordinate, width] from the current scanline.
            std::pair<sal_Int32,sal_Int32> span(it.GetNextSpan());

            // now forward this to the graphics object.
            // the only part that is worth noting is that we use
            // the number of equal spanlines [the current is always the
            // first one of the equal bunch] as the height of the region.
            mpGraphics->UnionClipRegion( offset_x+span.first,
                                         offset_y+miny,
                                         span.second,
                                         dwNumEqual,
                                         this );
        }
        it.Skip(dwNumEqual);
        miny += dwNumEqual;
    }
    mpGraphics->EndSetClipRegion();

    // indicates that no output needs to be produced
    // since the clipping region did not provide any
    // visible areas. the clip covers the whole area
    // if there's not a single region.
    mbOutputClipped = (container.GetNumSpans() == 0);

    // indicates that a clip region has been
    // presented to the output device.
    mbClipRegion = sal_True;

    // indicates that the set clipping region
    // has been processed.
    mbClipRegionSet = sal_True;

    // under 'normal' circumstances a new clipping region
    // needs to be processed by ImplInitClipRegion(),
    // which we need to circumvent.
    mbInitClipRegion = sal_False;
}

// -----------------------------------------------------------------------

void OutputDevice::SetClipRegion()
{
    DBG_TRACE( "OutputDevice::SetClipRegion()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( Region(), sal_False ) );

    ImplSetClipRegion( NULL );

    if( mpAlphaVDev )
        mpAlphaVDev->SetClipRegion();
}

// -----------------------------------------------------------------------

void OutputDevice::SetClipRegion( const Region& rRegion )
{
    DBG_TRACE( "OutputDevice::SetClipRegion( rRegion )" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rRegion, Region, ImplDbgTestRegion );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( rRegion, sal_True ) );

    if ( rRegion.GetType() == REGION_NULL )
        ImplSetClipRegion( NULL );
    else
    {
        Region aRegion = LogicToPixel( rRegion );
        ImplSetClipRegion( &aRegion );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetClipRegion( rRegion );
}

// -----------------------------------------------------------------------

void OutputDevice::SetTriangleClipRegion( const PolyPolygon &rPolyPolygon )
{
    DBG_TRACE( "OutputDevice::SetTriangleClipRegion( rPolyPolygon )" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    // in case the passed polypolygon is empty, use the
    // existing SetClipRegion() method which gracefully
    // unsets any previously set clipping region.
    if(!(rPolyPolygon.Count()))
        SetClipRegion();

    sal_Int32 offset_x = 0;
    sal_Int32 offset_y = 0;
    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        offset_x = mnOutOffX+mnOutOffOrigX;
        offset_y = mnOutOffY+mnOutOffOrigY;
    }

    // play nice with the rest of the system and provide an old-style region.
    // the rest of this method does not rely on this.
    maRegion = Region::GetRegionFromPolyPolygon( LogicToPixel(rPolyPolygon) );
    maRegion.Move(offset_x,offset_x);

    // feed region to metafile
    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( maRegion, sal_True ) );

    ImplSetTriangleClipRegion( rPolyPolygon );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTriangleClipRegion( rPolyPolygon );
}

// -----------------------------------------------------------------------

Region OutputDevice::GetClipRegion() const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    return PixelToLogic( maRegion );
}

// -----------------------------------------------------------------------

Region OutputDevice::GetActiveClipRegion() const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        Region aRegion( REGION_NULL );
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

// -----------------------------------------------------------------------

void OutputDevice::MoveClipRegion( long nHorzMove, long nVertMove )
{
    DBG_TRACE( "OutputDevice::MoveClipRegion()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mbClipRegion )
    {
        if( mpMetaFile )
            mpMetaFile->AddAction( new MetaMoveClipRegionAction( nHorzMove, nVertMove ) );

        maRegion.Move( ImplLogicWidthToDevicePixel( nHorzMove ),
                       ImplLogicHeightToDevicePixel( nVertMove ) );
        mbInitClipRegion = sal_True;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->MoveClipRegion( nHorzMove, nVertMove );
}

// -----------------------------------------------------------------------

void OutputDevice::IntersectClipRegion( const Rectangle& rRect )
{
    DBG_TRACE( "OutputDevice::IntersectClipRegion( rRect )" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaISectRectClipRegionAction( rRect ) );

    Rectangle aRect = LogicToPixel( rRect );
    maRegion.Intersect( aRect );
    mbClipRegion        = sal_True;
    mbInitClipRegion    = sal_True;

    if( mpAlphaVDev )
        mpAlphaVDev->IntersectClipRegion( rRect );
}

// -----------------------------------------------------------------------

void OutputDevice::IntersectClipRegion( const Region& rRegion )
{
    DBG_TRACE( "OutputDevice::IntersectClipRegion( rRegion )" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rRegion, Region, ImplDbgTestRegion );

    RegionType eType = rRegion.GetType();

    if ( eType != REGION_NULL )
    {
        if ( mpMetaFile )
            mpMetaFile->AddAction( new MetaISectRegionClipRegionAction( rRegion ) );

        Region aRegion = LogicToPixel( rRegion );
        maRegion.Intersect( aRegion );
        mbClipRegion        = sal_True;
        mbInitClipRegion    = sal_True;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->IntersectClipRegion( rRegion );
}

// -----------------------------------------------------------------------

void OutputDevice::SetDrawMode( sal_uLong nDrawMode )
{
    DBG_TRACE1( "OutputDevice::SetDrawMode( %lx )", nDrawMode );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    mnDrawMode = nDrawMode;

    if( mpAlphaVDev )
        mpAlphaVDev->SetDrawMode( nDrawMode );
}

// -----------------------------------------------------------------------

void OutputDevice::SetRasterOp( RasterOp eRasterOp )
{
    DBG_TRACE1( "OutputDevice::SetRasterOp( %d )", (int)eRasterOp );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRasterOpAction( eRasterOp ) );

    if ( meRasterOp != eRasterOp )
    {
        meRasterOp = eRasterOp;
        mbInitLineColor = mbInitFillColor = sal_True;

        if( mpGraphics || ImplGetGraphics() )
            mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp), ROP_INVERT == meRasterOp );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetRasterOp( eRasterOp );
}

// -----------------------------------------------------------------------

void OutputDevice::SetLineColor()
{
    DBG_TRACE( "OutputDevice::SetLineColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineColorAction( Color(), sal_False ) );

    if ( mbLineColor )
    {
        mbInitLineColor = sal_True;
        mbLineColor = sal_False;
        maLineColor = Color( COL_TRANSPARENT );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetLineColor();
}

// -----------------------------------------------------------------------

void OutputDevice::SetLineColor( const Color& rColor )
{
    DBG_TRACE1( "OutputDevice::SetLineColor( %lx )", rColor.GetColor() );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );

    if( mnDrawMode & ( DRAWMODE_BLACKLINE | DRAWMODE_WHITELINE |
                       DRAWMODE_GRAYLINE | DRAWMODE_GHOSTEDLINE |
                       DRAWMODE_SETTINGSLINE ) )
    {
        if( !ImplIsColorTransparent( aColor ) )
        {
            if( mnDrawMode & DRAWMODE_BLACKLINE )
            {
                aColor = Color( COL_BLACK );
            }
            else if( mnDrawMode & DRAWMODE_WHITELINE )
            {
                aColor = Color( COL_WHITE );
            }
            else if( mnDrawMode & DRAWMODE_GRAYLINE )
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DRAWMODE_SETTINGSLINE )
            {
                aColor = GetSettings().GetStyleSettings().GetFontColor();
            }

            if( mnDrawMode & DRAWMODE_GHOSTEDLINE )
            {
                aColor = Color( ( aColor.GetRed() >> 1 ) | 0x80,
                                ( aColor.GetGreen() >> 1 ) | 0x80,
                                ( aColor.GetBlue() >> 1 ) | 0x80);
            }
        }
    }

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineColorAction( aColor, sal_True ) );

    if( ImplIsColorTransparent( aColor ) )
    {
        if ( mbLineColor )
        {
            mbInitLineColor = sal_True;
            mbLineColor = sal_False;
            maLineColor = Color( COL_TRANSPARENT );
        }
    }
    else
    {
        if( maLineColor != aColor )
        {
            mbInitLineColor = sal_True;
            mbLineColor = sal_True;
            maLineColor = aColor;
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetLineColor( COL_BLACK );
}

// -----------------------------------------------------------------------

void OutputDevice::SetFillColor()
{
    DBG_TRACE( "OutputDevice::SetFillColor()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaFillColorAction( Color(), sal_False ) );

    if ( mbFillColor )
    {
        mbInitFillColor = sal_True;
        mbFillColor = sal_False;
        maFillColor = Color( COL_TRANSPARENT );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetFillColor();
}

// -----------------------------------------------------------------------

void OutputDevice::SetFillColor( const Color& rColor )
{
    DBG_TRACE1( "OutputDevice::SetFillColor( %lx )", rColor.GetColor() );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
        mpMetaFile->AddAction( new MetaFillColorAction( aColor, sal_True ) );

    if ( ImplIsColorTransparent( aColor ) )
    {
        if ( mbFillColor )
        {
            mbInitFillColor = sal_True;
            mbFillColor = sal_False;
            maFillColor = Color( COL_TRANSPARENT );
        }
    }
    else
    {
        if ( maFillColor != aColor )
        {
            mbInitFillColor = sal_True;
            mbFillColor = sal_True;
            maFillColor = aColor;
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetFillColor( COL_BLACK );
}

// -----------------------------------------------------------------------

void OutputDevice::SetBackground()
{
    DBG_TRACE( "OutputDevice::SetBackground()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    maBackground = Wallpaper();
    mbBackground = sal_False;

    if( mpAlphaVDev )
        mpAlphaVDev->SetBackground();
}

// -----------------------------------------------------------------------

void OutputDevice::SetBackground( const Wallpaper& rBackground )
{
    DBG_TRACE( "OutputDevice::SetBackground( rBackground )" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    maBackground = rBackground;

    if( rBackground.GetStyle() == WALLPAPER_NULL )
        mbBackground = sal_False;
    else
        mbBackground = sal_True;

    if( mpAlphaVDev )
        mpAlphaVDev->SetBackground( rBackground );
}

// -----------------------------------------------------------------------

void OutputDevice::SetRefPoint()
{
    DBG_TRACE( "OutputDevice::SetRefPoint()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRefPointAction( Point(), sal_False ) );

    mbRefPoint = sal_False;
    maRefPoint.X() = maRefPoint.Y() = 0L;

    if( mpAlphaVDev )
        mpAlphaVDev->SetRefPoint();
}

// -----------------------------------------------------------------------

void OutputDevice::SetRefPoint( const Point& rRefPoint )
{
    DBG_TRACE( "OutputDevice::SetRefPoint( rRefPoint )" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRefPointAction( rRefPoint, sal_True ) );

    mbRefPoint = sal_True;
    maRefPoint = rRefPoint;

    if( mpAlphaVDev )
        mpAlphaVDev->SetRefPoint( rRefPoint );
}

// -----------------------------------------------------------------------

void OutputDevice::DrawLine( const Point& rStartPt, const Point& rEndPt )
{
    DBG_TRACE( "OutputDevice::DrawLine()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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

        if( mpGraphics->DrawPolyLine( aB2DPolyLine, 0.0, aB2DLineWidth, basegfx::B2DLINEJOIN_NONE, this))
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

// -----------------------------------------------------------------------

void OutputDevice::impPaintLineGeometryWithEvtlExpand(
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
                rInfo.GetLineJoin()));
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
                bDone = mpGraphics->DrawPolyLine( aCandidate, 0.0, basegfx::B2DVector(1.0,1.0), basegfx::B2DLINEJOIN_NONE, this);
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
                const Polygon aPolygon(aFillPolyPolygon.getB2DPolygon(a));
                mpGraphics->DrawPolygon(aPolygon.GetSize(), (const SalPoint*)aPolygon.GetConstPointAry(), this);
            }
        }

        SetFillColor( aOldFillColor );
        SetLineColor( aOldLineColor );
    }

    mpMetaFile = pOldMetaFile;
}

// -----------------------------------------------------------------------

void OutputDevice::DrawLine( const Point& rStartPt, const Point& rEndPt,
                             const LineInfo& rLineInfo )
{
    DBG_TRACE( "OutputDevice::DrawLine()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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

        impPaintLineGeometryWithEvtlExpand(aInfo, basegfx::B2DPolyPolygon(aLinePolygon));
    }
    else
    {
        mpGraphics->DrawLine( aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y(), this );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawLine( rStartPt, rEndPt, rLineInfo );
}

// -----------------------------------------------------------------------

void OutputDevice::DrawRect( const Rectangle& rRect )
{
    DBG_TRACE( "OutputDevice::DrawRect()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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

// -----------------------------------------------------------------------

void OutputDevice::DrawPolyLine( const Polygon& rPoly )
{
    DBG_TRACE( "OutputDevice::DrawPolyLine()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rPoly, Polygon, NULL );

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
    if(bTryAA && ImpTryDrawPolyLineDirect(rPoly.getB2DPolygon(), 0.0, basegfx::B2DLINEJOIN_NONE))
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

        if(mpGraphics->DrawPolyLine( aB2DPolyLine, 0.0, aB2DLineWidth, basegfx::B2DLINEJOIN_NONE, this))
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

// -----------------------------------------------------------------------

void OutputDevice::DrawPolyLine( const Polygon& rPoly, const LineInfo& rLineInfo )
{
    DBG_TRACE( "OutputDevice::DrawPolyLine()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rPoly, Polygon, NULL );

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
        DrawPolyLine( rPoly.getB2DPolygon(), (double)rLineInfo.GetWidth(), rLineInfo.GetLineJoin());
        return;
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyLineAction( rPoly, rLineInfo ) );

    ImpDrawPolyLineWithLineInfo(rPoly, rLineInfo);
}

void OutputDevice::ImpDrawPolyLineWithLineInfo(const Polygon& rPoly, const LineInfo& rLineInfo)
{
    sal_uInt16 nPoints(rPoly.GetSize());

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ( nPoints < 2 ) || ( LINE_NONE == rLineInfo.GetStyle() ) || ImplIsRecordLayout() )
        return;

    Polygon aPoly = ImplLogicToDevicePixel( rPoly );

    // #100127# LineInfo is not curve-safe, subdivide always
    //
    // What shall this mean? It's wrong to subdivide here when the
    // polygon is a fat line. In that case, the painted geometry
    // WILL be much different.
    // I also have no idea how this could be related to the given ID
    // which reads 'consolidate boost versions' in the task description.
    // Removing.
    //
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
        impPaintLineGeometryWithEvtlExpand(aInfo, basegfx::B2DPolyPolygon(aPoly.getB2DPolygon()));
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

// -----------------------------------------------------------------------

void OutputDevice::DrawPolygon( const Polygon& rPoly )
{
    DBG_TRACE( "OutputDevice::DrawPolygon()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rPoly, Polygon, NULL );

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

            bSuccess = mpGraphics->DrawPolyLine( aB2DPolygon, 0.0, aB2DLineWidth, basegfx::B2DLINEJOIN_NONE, this);
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

// -----------------------------------------------------------------------

void OutputDevice::DrawPolyPolygon( const PolyPolygon& rPolyPoly )
{
    DBG_TRACE( "OutputDevice::DrawPolyPolygon()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rPolyPoly, PolyPolygon, NULL );

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
                bSuccess = mpGraphics->DrawPolyLine( aB2DPolyPolygon.getB2DPolygon(a), 0.0, aB2DLineWidth, basegfx::B2DLINEJOIN_NONE, this);
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

// -----------------------------------------------------------------------

void OutputDevice::DrawPolygon( const ::basegfx::B2DPolygon& rB2DPolygon)
{
    // AW: Do NOT paint empty polygons
    if(rB2DPolygon.count())
    {
        ::basegfx::B2DPolyPolygon aPP( rB2DPolygon );
        DrawPolyPolygon( aPP );
    }
}

// -----------------------------------------------------------------------
// Caution: This method is nearly the same as
// OutputDevice::DrawTransparent( const basegfx::B2DPolyPolygon& rB2DPolyPoly, double fTransparency),
// so when changes are made here do not forget to make change sthere, too

void OutputDevice::DrawPolyPolygon( const basegfx::B2DPolyPolygon& rB2DPolyPoly )
{
    DBG_TRACE( "OutputDevice::DrawPolyPolygon(B2D&)" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

#if 0
    // MetaB2DPolyPolygonAction is not implemented yet:
    // according to AW adding it is very dangerous since there is a lot
    // of code that uses the metafile actions directly and unless every
    // place that does this knows about the new action we need to fallback
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaB2DPolyPolygonAction( rB2DPolyPoly ) );
#else
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyPolygonAction( PolyPolygon( rB2DPolyPoly ) ) );
#endif

    // call helper
    ImpDrawPolyPolygonWithB2DPolyPolygon(rB2DPolyPoly);
}

void OutputDevice::ImpDrawPolyPolygonWithB2DPolyPolygon(const basegfx::B2DPolyPolygon& rB2DPolyPoly)
{
    // AW: Do NOT paint empty PolyPolygons
    if(!rB2DPolyPoly.count())
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
                bSuccess = mpGraphics->DrawPolyLine( aB2DPolyPolygon.getB2DPolygon(a), 0.0, aB2DLineWidth, basegfx::B2DLINEJOIN_NONE, this);
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

// -----------------------------------------------------------------------

bool OutputDevice::ImpTryDrawPolyLineDirect(
    const basegfx::B2DPolygon& rB2DPolygon,
    double fLineWidth,
    basegfx::B2DLineJoin eLineJoin)
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
    return mpGraphics->DrawPolyLine( aB2DPolygon, 0.0, aB2DLineWidth, eLineJoin, this);
}

void OutputDevice::DrawPolyLine(
    const basegfx::B2DPolygon& rB2DPolygon,
    double fLineWidth,
    basegfx::B2DLineJoin eLineJoin)
{
    DBG_TRACE( "OutputDevice::DrawPolyLine(B2D&)" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    (void)eLineJoin; // ATM used in UNX, but not in WNT, access it for warning-free

#if 0 // MetaB2DPolyLineAction is not implemented yet:
      // according to AW adding it is very dangerous since there is a lot
      // of code that uses the metafile actions directly and unless every
      // place that does this knows about the new action we need to fallback
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaB2DPolyLineAction( rB2DPolygon ) );
#else
    if( mpMetaFile )
    {
        LineInfo aLineInfo;
        if( fLineWidth != 0.0 )
            aLineInfo.SetWidth( static_cast<long>(fLineWidth+0.5) );
        const Polygon aToolsPolygon( rB2DPolygon );
        mpMetaFile->AddAction( new MetaPolyLineAction( aToolsPolygon, aLineInfo ) );
    }
#endif

    // AW: Do NOT paint empty PolyPolygons
    if(!rB2DPolygon.count())
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
    if(bTryAA && ImpTryDrawPolyLineDirect(rB2DPolygon, fLineWidth, eLineJoin))
    {
        return;
    }

    // #i101491#
    // no output yet; fallback to geometry decomposition and use filled polygon paint
    // when line is fat and not too complex. ImpDrawPolyPolygonWithB2DPolyPolygon
    // will do internal needed AA checks etc.
    if(fLineWidth >= 2.5
        && rB2DPolygon.count()
        && rB2DPolygon.count() <= 1000)
    {
        const double fHalfLineWidth((fLineWidth * 0.5) + 0.5);
        const basegfx::B2DPolyPolygon aAreaPolyPolygon(basegfx::tools::createAreaGeometry(
            rB2DPolygon, fHalfLineWidth, eLineJoin));

        const Color aOldLineColor(maLineColor);
        const Color aOldFillColor(maFillColor);

        SetLineColor();
        ImplInitLineColor();
        SetFillColor(aOldLineColor);
        ImplInitFillColor();

        // draw usig a loop; else the topology will paint a PolyPolygon
        for(sal_uInt32 a(0); a < aAreaPolyPolygon.count(); a++)
        {
            ImpDrawPolyPolygonWithB2DPolyPolygon(
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
                ImpTryDrawPolyLineDirect(aAreaPolyPolygon.getB2DPolygon(a), 0.0, basegfx::B2DLINEJOIN_NONE);
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
        ImpDrawPolyLineWithLineInfo( aToolsPolygon, aLineInfo );
    }
}

// -----------------------------------------------------------------------

void OutputDevice::Push( sal_uInt16 nFlags )
{
    DBG_TRACE( "OutputDevice::Push()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
        if ( mbMap )
            pData->mpMapMode = new MapMode( maMapMode );
        else
            pData->mpMapMode = NULL;
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

// -----------------------------------------------------------------------

void OutputDevice::Pop()
{
    DBG_TRACE( "OutputDevice::Pop()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaPopAction() );

    GDIMetaFile*    pOldMetaFile = mpMetaFile;
    ImplObjStack*   pData = mpObjStack;
    mpMetaFile = NULL;

    if ( !pData )
    {
        DBG_ERRORFILE( "OutputDevice::Pop() without OutputDevice::Push()" );
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

// -----------------------------------------------------------------------

void OutputDevice::SetConnectMetaFile( GDIMetaFile* pMtf )
{
    mpMetaFile = pMtf;
}

// -----------------------------------------------------------------------

void OutputDevice::EnableOutput( sal_Bool bEnable )
{
    mbOutput = (bEnable != 0);

    if( mpAlphaVDev )
        mpAlphaVDev->EnableOutput( bEnable );
}

// -----------------------------------------------------------------------

void OutputDevice::SetSettings( const AllSettings& rSettings )
{
    maSettings = rSettings;

    if( mpAlphaVDev )
        mpAlphaVDev->SetSettings( rSettings );
}

// -----------------------------------------------------------------------

sal_uInt16 OutputDevice::GetBitCount() const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( meOutDevType == OUTDEV_VIRDEV )
        return ((VirtualDevice*)this)->mnBitCount;

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !((OutputDevice*)this)->ImplGetGraphics() )
            return 0;
    }

    return (sal_uInt16)mpGraphics->GetBitCount();
}

// -----------------------------------------------------------------------

sal_uInt16 OutputDevice::GetAlphaBitCount() const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( meOutDevType == OUTDEV_VIRDEV &&
        mpAlphaVDev != NULL )
    {
        return mpAlphaVDev->GetBitCount();
    }

    return 0;
}

// -----------------------------------------------------------------------

sal_uLong OutputDevice::GetColorCount() const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    const sal_uInt16 nBitCount = GetBitCount();
    return( ( nBitCount > 31 ) ? ULONG_MAX : ( ( (sal_uLong) 1 ) << nBitCount) );
}

// -----------------------------------------------------------------------

sal_Bool OutputDevice::HasAlpha()
{
    return mpAlphaVDev != NULL;
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > OutputDevice::CreateUnoGraphics()
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    return pWrapper ? pWrapper->CreateGraphics( this ) : ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >();
}

// -----------------------------------------------------------------------

SystemGraphicsData OutputDevice::GetSystemGfxData() const
{
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return SystemGraphicsData();
    }

    return mpGraphics->GetGraphicsData();
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Any OutputDevice::GetSystemGfxDataAny() const
{
    ::com::sun::star::uno::Any aRet;
    const SystemGraphicsData aSysData = GetSystemGfxData();
    ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( (sal_Int8*)&aSysData,
                                                      aSysData.nSize );

    return uno::makeAny(aSeq);
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas > OutputDevice::GetCanvas() const
{
    uno::Sequence< uno::Any > aArg(6);

    aArg[ 0 ] = uno::makeAny( reinterpret_cast<sal_Int64>(this) );
    aArg[ 2 ] = uno::makeAny( ::com::sun::star::awt::Rectangle( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight ) );
    aArg[ 3 ] = uno::makeAny( sal_False );
    aArg[ 5 ] = GetSystemGfxDataAny();

    uno::Reference<lang::XMultiServiceFactory> xFactory = vcl::unohelper::GetMultiServiceFactory();

    uno::Reference<rendering::XCanvas> xCanvas;

    // Create canvas instance with window handle
    // =========================================
    if ( xFactory.is() )
    {
        static uno::Reference<lang::XMultiServiceFactory> xCanvasFactory(
            xFactory->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                              "com.sun.star."
                              "rendering.CanvasFactory") ) ),
            uno::UNO_QUERY );
        if(xCanvasFactory.is())
        {
            xCanvas.set(
                xCanvasFactory->createInstanceWithArguments(
                    OUString( RTL_CONSTASCII_USTRINGPARAM(
                                  "com.sun.star.rendering.Canvas" )),
                    aArg ),
                uno::UNO_QUERY );
        }
    }

    return xCanvas;
}

// -----------------------------------------------------------------------
