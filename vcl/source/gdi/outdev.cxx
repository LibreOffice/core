/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outdev.cxx,v $
 * $Revision: 1.55 $
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
#include <vcl/salotype.hxx>
#include <vcl/opengl.hxx>
#ifndef _VCL_IMPLNCVT_HXX
#include <implncvt.hxx>
#endif
#include <vcl/outdev3d.hxx>
#include <vcl/outdev.h>
#include <vcl/outdev.hxx>
#include <vcl/unowrap.hxx>

// #i75163#
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <com/sun/star/awt/XGraphics.hpp>

DBG_NAME( OutputDevice );
DBG_NAME( Polygon );
DBG_NAME( PolyPolygon );
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
    Point*          mpRefPoint;
    TextAlign       meTextAlign;
    RasterOp        meRasterOp;
    ULONG           mnTextLayoutMode;
    LanguageType    meTextLanguage;
    USHORT          mnFlags;
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


BOOL OutputDevice::ImplSelectClipRegion( SalGraphics* pGraphics, const Region& rRegion, OutputDevice *pOutDev )
{
    // TODO(Q3): Change from static to plain method - everybody's
    // calling it with pOutDev=this!
    DBG_TESTSOLARMUTEX();

    long                nX;
    long                nY;
    long                nWidth;
    long                nHeight;
    ULONG               nRectCount;
    ImplRegionInfo      aInfo;
    BOOL                bRegionRect;
    BOOL                bClipRegion = TRUE;
    const BOOL          bClipDeviceBounds( !pOutDev->GetPDFWriter()
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
                    bClipRegion = FALSE;
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
                    bClipRegion = FALSE;
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
                bClipRegion = FALSE;
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
    USHORT i, nPolys = rPolyPoly.Count();
    PolyPolygon aPolyPoly( nPolys );
    for( i=0; i<nPolys; ++i )
        aPolyPoly.Insert( ImplSubdivideBezier( rPolyPoly.GetObject(i) ) );

    return aPolyPoly;
}

// =======================================================================

// #100127# Extracted from OutputDevice::DrawPolyPolygon()
void OutputDevice::ImplDrawPolyPolygon( USHORT nPoly, const PolyPolygon& rPolyPoly )
{
    sal_uInt32          aStackAry1[OUTDEV_POLYPOLY_STACKBUF];
    PCONSTSALPOINT      aStackAry2[OUTDEV_POLYPOLY_STACKBUF];
    BYTE*               aStackAry3[OUTDEV_POLYPOLY_STACKBUF];
    sal_uInt32*         pPointAry;
    PCONSTSALPOINT*     pPointAryAry;
    const BYTE**        pFlagAryAry;
    USHORT              i = 0, j = 0, last = 0;
    BOOL                bHaveBezier = sal_False;
    if ( nPoly > OUTDEV_POLYPOLY_STACKBUF )
    {
        pPointAry       = new sal_uInt32[nPoly];
        pPointAryAry    = new PCONSTSALPOINT[nPoly];
        pFlagAryAry     = new const BYTE*[nPoly];
    }
    else
    {
        pPointAry       = aStackAry1;
        pPointAryAry    = aStackAry2;
        pFlagAryAry     = (const BYTE**)aStackAry3;
    }
    do
    {
        const Polygon&  rPoly = rPolyPoly.GetObject( i );
        USHORT          nSize = rPoly.GetSize();
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
    mp3DContext         = NULL;
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
    mbMap               = FALSE;
    mbMapIsDefault      = TRUE;
    mbClipRegion        = FALSE;
    mbBackground        = FALSE;
    mbOutput            = TRUE;
    mbDevOutput         = FALSE;
    mbOutputClipped     = FALSE;
    maTextColor         = Color( COL_BLACK );
    meTextAlign         = maFont.GetAlign();
    meRasterOp          = ROP_OVERPAINT;
    mnAntialiasing      = 0;
    meTextLanguage      = 0;  // TODO: get default from configuration?
    mbLineColor         = TRUE;
    mbFillColor         = TRUE;
    mbInitLineColor     = TRUE;
    mbInitFillColor     = TRUE;
    mbInitFont          = TRUE;
    mbInitTextColor     = TRUE;
    mbInitClipRegion    = TRUE;
    mbClipRegionSet     = FALSE;
    mbKerning           = FALSE;
    mbNewFont           = TRUE;
    mbTextLines         = FALSE;
    mbTextSpecial       = FALSE;
    mbRefPoint          = FALSE;
    mbEnableRTL         = FALSE;    // mirroring must be explicitly allowed (typically for windows only)

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
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper( FALSE );
        if ( pWrapper )
            pWrapper->ReleaseAllGraphics( this );
        delete mpUnoGraphicsList;
        mpUnoGraphicsList = NULL;
    }

    if ( mp3DContext )
        mp3DContext->Destroy( this );

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
    return ImplGetGraphics() && mpGraphics->supportsOperation( eType );
}

// -----------------------------------------------------------------------

void OutputDevice::EnableRTL( BOOL bEnable )
{
    mbEnableRTL = (bEnable != 0);
    if( meOutDevType == OUTDEV_VIRDEV )
    {
        // virdevs default to not mirroring, they will only be set to mirroring
        // under rare circumstances in the UI, eg the valueset control
        // because each virdev has its own SalGraphics we can safely switch the SalGraphics here
        // ...hopefully
        if( Application::GetSettings().GetLayoutRTL() ) // allow mirroring only in BiDi Office
            if( ImplGetGraphics() )
                mpGraphics->SetLayout( mbEnableRTL ? SAL_LAYOUT_BIDI_RTL : 0 );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->EnableRTL( bEnable );
}

BOOL OutputDevice::ImplHasMirroredGraphics()
{
   // HOTFIX for #i55719#
   if( meOutDevType == OUTDEV_PRINTER )
       return FALSE;

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
    BOOL                bRegionRect;
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
        return TRUE;

    mbInitLineColor     = TRUE;
    mbInitFillColor     = TRUE;
    mbInitFont          = TRUE;
    mbInitTextColor     = TRUE;
    mbInitClipRegion    = TRUE;

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
                pReleaseOutDev->ImplReleaseGraphics( FALSE );
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
        mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp) );
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplReleaseGraphics( BOOL bRelease )
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

BOOL OutputDevice::ImplIsRecordLayout() const
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

    mbInitLineColor = FALSE;
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

    mbInitFillColor = FALSE;
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
            if( ImplHasMirroredGraphics() && !IsRTLEnabled() )
                ImplReMirror ( aRegion );
        }
        if ( mbClipRegion )
            aRegion.Intersect( ImplPixelToDevicePixel( maRegion ) );
        if ( aRegion.IsEmpty() )
            mbOutputClipped = TRUE;
        else
        {
            mbOutputClipped = FALSE;
            ImplSelectClipRegion( mpGraphics, aRegion, this );
        }
        mbClipRegionSet = TRUE;
    }
    else
    {
        if ( mbClipRegion )
        {
            if ( maRegion.IsEmpty() )
                mbOutputClipped = TRUE;
            else
            {
                mbOutputClipped = FALSE;
                ImplSelectClipRegion( mpGraphics,
                                      // #102532# Respect output offset also for clip region
                                      ImplPixelToDevicePixel( maRegion ), this );
            }

            mbClipRegionSet = TRUE;
        }
        else
        {
            if ( mbClipRegionSet )
            {
                mpGraphics->ResetClipRegion();
                mbClipRegionSet = FALSE;
            }

            mbOutputClipped = FALSE;
        }
    }

    mbInitClipRegion = FALSE;
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
            mbClipRegion        = FALSE;
            mbInitClipRegion    = TRUE;
        }
    }
    else
    {
        maRegion            = *pRegion;
        mbClipRegion        = TRUE;
        mbInitClipRegion    = TRUE;
    }
}

// -----------------------------------------------------------------------

namespace
{
    inline int iround( float x )
    {
        sal_Int32 a = *reinterpret_cast<const sal_Int32 *>(&x);
        sal_Int32 exponent = (127 + 31) - ((a >> 23) & 0xFF);
        sal_Int32 r = ((sal_Int32(a) << 8) | (1U << 31)) >> exponent;
        r &= ((exponent - 32) >> 31);
        sal_Int32 sign = a >> 31;
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

    // first of all we need to know the upper limit
    // of the amount of possible clipping regions.
    sal_Int32 maxy = SAL_MIN_INT32;
    sal_Int32 miny = SAL_MAX_INT32;
    sal_uInt32 dwNumTriangles = 0;
    for(USHORT i=0; i<rPolyPolygon.Count(); ++i)
    {
        const Polygon &rPoly = rPolyPolygon.GetObject(i);
        const sal_Int32 dwNumVertices = rPoly.GetSize();
        if(!(dwNumVertices % 3))
        {
            for(USHORT j=0; j<rPoly.GetSize(); ++j)
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
        mbOutputClipped = TRUE;

        // indicates that a clip region has been
        // presented to the output device.
        mbClipRegion = TRUE;

        // indicates that the set clipping region
        // has been processed.
        mbClipRegionSet = TRUE;

        // under 'normal' circumstances a new clipping region
        // needs to be processed by ImplInitClipRegion(),
        // which we need to circumvent.
        mbInitClipRegion = FALSE;
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
    for(USHORT i=0; i<rPolyPolygon.Count(); ++i)
    {
        const Polygon &rPoly = rPolyPolygon.GetObject(i);
        const USHORT dwNumVertices = rPoly.GetSize();
        if(!(dwNumVertices % 3))
        {
            for(USHORT j=0; j<dwNumVertices; j+=3)
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

    sal_Int32 offset_x = 0;
    sal_Int32 offset_y = 0;
    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        offset_x = mnOutOffX+mnOutOffOrigX;
        offset_y = mnOutOffY+mnOutOffOrigY;
    }

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
    mbClipRegion = TRUE;

    // indicates that the set clipping region
    // has been processed.
    mbClipRegionSet = TRUE;

    // under 'normal' circumstances a new clipping region
    // needs to be processed by ImplInitClipRegion(),
    // which we need to circumvent.
    mbInitClipRegion = FALSE;
}

// -----------------------------------------------------------------------

void OutputDevice::SetClipRegion()
{
    DBG_TRACE( "OutputDevice::SetClipRegion()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( Region(), FALSE ) );

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
        mpMetaFile->AddAction( new MetaClipRegionAction( rRegion, TRUE ) );

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
        mpMetaFile->AddAction( new MetaClipRegionAction( maRegion, TRUE ) );

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
        mbInitClipRegion = TRUE;
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
    mbClipRegion        = TRUE;
    mbInitClipRegion    = TRUE;

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
        mbClipRegion        = TRUE;
        mbInitClipRegion    = TRUE;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->IntersectClipRegion( rRegion );
}

// -----------------------------------------------------------------------

void OutputDevice::SetDrawMode( ULONG nDrawMode )
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
        mbInitLineColor = mbInitFillColor = TRUE;

        if( mpGraphics || ImplGetGraphics() )
            mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp) );
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
        mpMetaFile->AddAction( new MetaLineColorAction( Color(), FALSE ) );

    if ( mbLineColor )
    {
        mbInitLineColor = TRUE;
        mbLineColor = FALSE;
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
                const UINT8 cLum = aColor.GetLuminance();
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
        mpMetaFile->AddAction( new MetaLineColorAction( aColor, TRUE ) );

    if( ImplIsColorTransparent( aColor ) )
    {
        if ( mbLineColor )
        {
            mbInitLineColor = TRUE;
            mbLineColor = FALSE;
            maLineColor = Color( COL_TRANSPARENT );
        }
    }
    else
    {
        if( maLineColor != aColor )
        {
            mbInitLineColor = TRUE;
            mbLineColor = TRUE;
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
        mpMetaFile->AddAction( new MetaFillColorAction( Color(), FALSE ) );

    if ( mbFillColor )
    {
        mbInitFillColor = TRUE;
        mbFillColor = FALSE;
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
                const UINT8 cLum = aColor.GetLuminance();
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
        mpMetaFile->AddAction( new MetaFillColorAction( aColor, TRUE ) );

    if ( ImplIsColorTransparent( aColor ) )
    {
        if ( mbFillColor )
        {
            mbInitFillColor = TRUE;
            mbFillColor = FALSE;
            maFillColor = Color( COL_TRANSPARENT );
        }
    }
    else
    {
        if ( maFillColor != aColor )
        {
            mbInitFillColor = TRUE;
            mbFillColor = TRUE;
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
    mbBackground = FALSE;

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
        mbBackground = FALSE;
    else
        mbBackground = TRUE;

    if( mpAlphaVDev )
        mpAlphaVDev->SetBackground( rBackground );
}

// -----------------------------------------------------------------------

void OutputDevice::SetRefPoint()
{
    DBG_TRACE( "OutputDevice::SetRefPoint()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRefPointAction( Point(), FALSE ) );

    mbRefPoint = FALSE;
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
        mpMetaFile->AddAction( new MetaRefPointAction( rRefPoint, TRUE ) );

    mbRefPoint = TRUE;
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

    Point aStartPt = ImplLogicToDevicePixel( rStartPt );
    Point aEndPt = ImplLogicToDevicePixel( rEndPt );

    mpGraphics->DrawLine( aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y(), this );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawLine( rStartPt, rEndPt );
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

    const LineInfo aInfo( ImplLogicToDevicePixel( rLineInfo ) );

    if( ( aInfo.GetWidth() > 1L ) || ( LINE_DASH == aInfo.GetStyle() ) )
    {
        Polygon             aPoly( 2 ); aPoly[ 0 ] = rStartPt; aPoly[ 1 ] = rEndPt;
        GDIMetaFile*        pOldMetaFile = mpMetaFile;
        ImplLineConverter   aLineCvt( ImplLogicToDevicePixel( aPoly ), aInfo, ( mbRefPoint ) ? &maRefPoint : NULL );

        mpMetaFile = NULL;

        if ( aInfo.GetWidth() > 1 )
        {
            const Color     aOldLineColor( maLineColor );
            const Color     aOldFillColor( maFillColor );

            SetLineColor();
            ImplInitLineColor();
            SetFillColor( aOldLineColor );
            ImplInitFillColor();

            for( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
                mpGraphics->DrawPolygon( pPoly->GetSize(), (const SalPoint*) pPoly->GetConstPointAry(), this );

            SetFillColor( aOldFillColor );
            SetLineColor( aOldLineColor );
        }
        else
        {
            if ( mbInitLineColor )
                ImplInitLineColor();

            for ( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
                mpGraphics->DrawLine( (*pPoly)[ 0 ].X(), (*pPoly)[ 0 ].Y(), (*pPoly)[ 1 ].X(), (*pPoly)[ 1 ].Y(), this );
        }
        mpMetaFile = pOldMetaFile;
    }
    else
    {
        const Point aStartPt( ImplLogicToDevicePixel( rStartPt ) );
        const Point aEndPt( ImplLogicToDevicePixel( rEndPt ) );

        if ( mbInitLineColor )
            ImplInitLineColor();

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

    USHORT nPoints = rPoly.GetSize();

    if ( !IsDeviceOutputNecessary() || !mbLineColor || (nPoints < 2) || ImplIsRecordLayout() )
        return;

    // we need a graphics
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

    Polygon aPoly = ImplLogicToDevicePixel( rPoly );
    const SalPoint* pPtAry = (const SalPoint*)aPoly.GetConstPointAry();

    // #100127# Forward beziers to sal, if any
    if( aPoly.HasFlags() )
    {
        const BYTE* pFlgAry = aPoly.GetConstFlagAry();
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

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaPolyLineAction( rPoly, rLineInfo ) );

    USHORT nPoints = rPoly.GetSize();

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ( nPoints < 2 ) || ( LINE_NONE == rLineInfo.GetStyle() ) || ImplIsRecordLayout() )
        return;

    Polygon aPoly = ImplLogicToDevicePixel( rPoly );

    // #100127# LineInfo is not curve-safe, subdivide always
    if( aPoly.HasFlags() )
    {
        aPoly = ImplSubdivideBezier( aPoly );
        nPoints = aPoly.GetSize();
    }

    // we need a graphics
    if ( !mpGraphics && !ImplGetGraphics() )
        return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();

    if ( mbOutputClipped )
        return;

    const LineInfo aInfo( ImplLogicToDevicePixel( rLineInfo ) );

    if( aInfo.GetWidth() > 1L )
    {
        const Color         aOldLineColor( maLineColor );
        const Color         aOldFillColor( maFillColor );
        GDIMetaFile*        pOldMetaFile = mpMetaFile;
        ImplLineConverter   aLineCvt( aPoly, aInfo, ( mbRefPoint ) ? &maRefPoint : NULL );

        mpMetaFile = NULL;
        SetLineColor();
        ImplInitLineColor();
        SetFillColor( aOldLineColor );
        ImplInitFillColor();

        for( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
            mpGraphics->DrawPolygon( pPoly->GetSize(), (const SalPoint*) pPoly->GetConstPointAry(), this );

        SetLineColor( aOldLineColor );
        SetFillColor( aOldFillColor );
        mpMetaFile = pOldMetaFile;
    }
    else
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( LINE_DASH == aInfo.GetStyle() )
        {
            ImplLineConverter   aLineCvt( aPoly, aInfo, ( mbRefPoint ) ? &maRefPoint : NULL );
            for( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
                mpGraphics->DrawPolyLine( pPoly->GetSize(), (const SalPoint*)pPoly->GetConstPointAry(), this );
        }
        else
            mpGraphics->DrawPolyLine( nPoints, (const SalPoint*) aPoly.GetConstPointAry(), this );
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

    USHORT nPoints = rPoly.GetSize();

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || (nPoints < 2) || ImplIsRecordLayout() )
        return;

    // we need a graphics
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

    Polygon aPoly = ImplLogicToDevicePixel( rPoly );
    const SalPoint* pPtAry = (const SalPoint*)aPoly.GetConstPointAry();

    // #100127# Forward beziers to sal, if any
    if( aPoly.HasFlags() )
    {
        const BYTE* pFlgAry = aPoly.GetConstFlagAry();
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

    USHORT nPoly = rPolyPoly.Count();

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || !nPoly || ImplIsRecordLayout() )
        return;

    // we need a graphics
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

void OutputDevice::Push( USHORT nFlags )
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

void OutputDevice::EnableOutput( BOOL bEnable )
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

USHORT OutputDevice::GetBitCount() const
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

    return (USHORT)mpGraphics->GetBitCount();
}

// -----------------------------------------------------------------------

USHORT OutputDevice::GetAlphaBitCount() const
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

ULONG OutputDevice::GetColorCount() const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    const USHORT nBitCount = GetBitCount();
    return( ( nBitCount > 31 ) ? ULONG_MAX : ( ( (ULONG) 1 ) << nBitCount) );
}

// -----------------------------------------------------------------------

BOOL OutputDevice::HasAlpha()
{
    return mpAlphaVDev != NULL;
}

// -----------------------------------------------------------------------

OpenGL* OutputDevice::GetOpenGL()
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    OpenGL* pOGL;

    if( OUTDEV_PRINTER != meOutDevType )
    {
        pOGL = new OpenGL( this );

        if( !pOGL->IsValid() )
        {
            delete pOGL;
            pOGL = NULL;
        }
    }
    else
        pOGL = NULL;

    return pOGL;
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > OutputDevice::CreateUnoGraphics()
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    return pWrapper ? pWrapper->CreateGraphics( this ) : ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >();
}
