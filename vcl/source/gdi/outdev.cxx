/*************************************************************************
 *
 *  $RCSfile: outdev.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_OUTDEV_CXX
#include <tools/ref.hxx>
#ifndef REMOTE_APPSERVER
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#endif

#ifndef REMOTE_APPSERVER
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif
#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif
#else
#ifndef _SV_RMOUTDEV_HXX
#include <rmoutdev.hxx>
#endif
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_POLY_H
#include <poly.h>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_REGION_HXX
#include <region.hxx>
#endif
#ifndef _SV_REGION_H
#include <region.h>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_OUTDATA_HXX
#include <outdata.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <print.hxx>
#endif
#ifndef _SV_SALOTYPE_HXX
#include <salotype.hxx>
#endif
#ifndef _SV_OPENGL_HXX
#include <opengl.hxx>
#endif
#ifndef _VCL_IMPLNCVT_HXX
#include <implncvt.hxx>
#endif
#ifndef _SV_OUTDEV3D_HXX
#include <outdev3d.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _VCL_UNOWRAP_HXX
#include <unowrap.hxx>
#endif

#include <com/sun/star/awt/XGraphics.hpp>

DBG_NAME( OutputDevice );
DBG_NAMEEX( Polygon );
DBG_NAMEEX( PolyPolygon );
DBG_NAMEEX( Region );

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

#ifndef REMOTE_APPSERVER

BOOL OutputDevice::ImplSelectClipRegion( SalGraphics* pGraphics, const Region& rRegion )
{
    DBG_TESTSOLARMUTEX();

    long                nX;
    long                nY;
    long                nWidth;
    long                nHeight;
    ULONG               nRectCount;
    ImplRegionInfo      aInfo;
    BOOL                bRegionRect;
    BOOL                bClipRegion = TRUE;

    nRectCount = rRegion.GetRectCount();
    pGraphics->BeginSetClipRegion( nRectCount );
    bRegionRect = rRegion.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
    while ( bRegionRect )
    {
        if ( !pGraphics->UnionClipRegion( nX, nY, nWidth, nHeight ) )
            bClipRegion = FALSE;
        DBG_ASSERT( bClipRegion, "OutputDevice::ImplSelectClipRegion() - can't cerate region" );
        bRegionRect = rRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
    }
    pGraphics->EndSetClipRegion();
    return bClipRegion;
}

#endif

// =======================================================================

OutputDevice::OutputDevice() :
    maRegion( REGION_NULL ),
    maSettings( Application::GetSettings() ),
    maFillColor( COL_WHITE ),
    maTextLineColor( COL_TRANSPARENT )
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
    mnOutOffX           = 0;
    mnOutOffY           = 0;
    mnOutWidth          = 0;
    mnOutHeight         = 0;
    mnDPIX              = 0;
    mnDPIY              = 0;
    mnTextOffX          = 0;
    mnTextOffY          = 0;
    mnDrawMode          = 0;
    meOutDevType        = OUTDEV_DONTKNOW;
    mbMap               = FALSE;
    mbClipRegion        = FALSE;
    mbBackground        = FALSE;
    mbOutput            = TRUE;
    mbDevOutput         = FALSE;
    mbOutputClipped     = FALSE;
    maTextColor         = maFont.GetColor();
    maTextFillColorDummy= maFont.GetFillColor();
    meTextAlign         = maFont.GetAlign();
    meRasterOp          = ROP_OVERPAINT;
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
}

// -----------------------------------------------------------------------

OutputDevice::~OutputDevice()
{
    DBG_DTOR( OutputDevice, ImplDbgCheckOutputDevice );

    if ( GetUnoGraphicsList() )
    {
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
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

    if ( mpFontEntry )
        mpFontCache->Release( mpFontEntry );
    if ( mpGetDevFontList )
        delete mpGetDevFontList;
    if ( mpGetDevSizeList )
        delete mpGetDevSizeList;
}

// -----------------------------------------------------------------------

#ifndef REMOTE_APPSERVER

int OutputDevice::ImplGetGraphics()
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

        mpGraphics = pWindow->mpFrame->GetGraphics();
        // Wenn wir keinen bekommen haben, versuchen wir uns einen zu klauen
        if ( !mpGraphics )
        {
            OutputDevice* pReleaseOutDev = pSVData->maGDIData.mpLastWinGraphics;
            while ( pReleaseOutDev )
            {
                if ( ((Window*)pReleaseOutDev)->mpFrame == pWindow->mpFrame )
                    break;
                pReleaseOutDev = pReleaseOutDev->mpPrevGraphics;
            }

            if ( pReleaseOutDev )
            {
                mpGraphics = pReleaseOutDev->mpGraphics;
                pReleaseOutDev->ImplReleaseGraphics( FALSE );
            }
            else
            {
                while ( !mpGraphics )
                {
                    if ( !pSVData->maGDIData.mpLastWinGraphics )
                        break;
                    pSVData->maGDIData.mpLastWinGraphics->ImplReleaseGraphics();
                    mpGraphics = pWindow->mpFrame->GetGraphics();
                }
            }
        }

        if ( mpGraphics )
        {
            mpNextGraphics = pSVData->maGDIData.mpFirstWinGraphics;
            pSVData->maGDIData.mpFirstWinGraphics = this;
            if ( mpNextGraphics )
                mpNextGraphics->mpPrevGraphics = this;
            if ( !pSVData->maGDIData.mpLastWinGraphics )
                pSVData->maGDIData.mpLastWinGraphics = this;
        }
    }
    else if ( meOutDevType == OUTDEV_VIRDEV )
    {
        VirtualDevice* pVirDev = (VirtualDevice*)this;

        if ( pVirDev->mpVirDev )
        {
            mpGraphics = pVirDev->mpVirDev->GetGraphics();
            while ( !mpGraphics )
            {
                if ( !pSVData->maGDIData.mpLastVirGraphics )
                    break;
                pSVData->maGDIData.mpLastVirGraphics->ImplReleaseGraphics();
                mpGraphics = pVirDev->mpVirDev->GetGraphics();
            }
            if ( mpGraphics )
            {
                mpNextGraphics = pSVData->maGDIData.mpFirstVirGraphics;
                pSVData->maGDIData.mpFirstVirGraphics = this;
                if ( mpNextGraphics )
                    mpNextGraphics->mpPrevGraphics = this;
                if ( !pSVData->maGDIData.mpLastVirGraphics )
                    pSVData->maGDIData.mpLastVirGraphics = this;
            }
        }
    }
    else if ( meOutDevType == OUTDEV_PRINTER )
    {
        Printer* pPrinter = (Printer*)this;

        if ( pPrinter->mpJobGraphics )
            mpGraphics = pPrinter->mpJobGraphics;
        else if ( pPrinter->mpDisplayDev )
        {
            VirtualDevice* pVirDev = pPrinter->mpDisplayDev;
            mpGraphics = pVirDev->mpVirDev->GetGraphics();
            while ( !mpGraphics )
            {
                if ( !pSVData->maGDIData.mpLastVirGraphics )
                    break;
                pSVData->maGDIData.mpLastVirGraphics->ImplReleaseGraphics();
                mpGraphics = pVirDev->mpVirDev->GetGraphics();
            }
            if ( mpGraphics )
            {
                mpNextGraphics = pSVData->maGDIData.mpFirstVirGraphics;
                pSVData->maGDIData.mpFirstVirGraphics = this;
                if ( mpNextGraphics )
                    mpNextGraphics->mpPrevGraphics = this;
                if ( !pSVData->maGDIData.mpLastVirGraphics )
                    pSVData->maGDIData.mpLastVirGraphics = this;
            }
        }
        else
        {
            mpGraphics = pPrinter->mpInfoPrinter->GetGraphics();
            while ( !mpGraphics )
            {
                if ( !pSVData->maGDIData.mpLastPrnGraphics )
                    break;
                pSVData->maGDIData.mpLastPrnGraphics->ImplReleaseGraphics();
                mpGraphics = pPrinter->mpInfoPrinter->GetGraphics();
            }
            if ( mpGraphics )
            {
                mpNextGraphics = pSVData->maGDIData.mpFirstPrnGraphics;
                pSVData->maGDIData.mpFirstPrnGraphics = this;
                if ( mpNextGraphics )
                    mpNextGraphics->mpPrevGraphics = this;
                if ( !pSVData->maGDIData.mpLastPrnGraphics )
                    pSVData->maGDIData.mpLastPrnGraphics = this;
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

    ImplSVData* pSVData = ImplGetSVData();
    if ( meOutDevType == OUTDEV_WINDOW )
    {
        Window* pWindow = (Window*)this;

        if ( bRelease )
            pWindow->mpFrame->ReleaseGraphics( mpGraphics );
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

#endif

// -----------------------------------------------------------------------

void OutputDevice::ImplInitOutDevData()
{
    if ( !mpOutDevData )
    {
        mpOutDevData = new ImplOutDevData;
        mpOutDevData->mpRotateDev = NULL;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplDeInitOutDevData()
{
    if ( mpOutDevData )
    {
        if ( mpOutDevData->mpRotateDev )
            delete mpOutDevData->mpRotateDev;
        delete mpOutDevData;
    }
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitLineColor()
{
    DBG_TESTSOLARMUTEX();

#ifndef REMOTE_APPSERVER
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
#else
    mpGraphics->SetLineColor( maLineColor );
#endif

    mbInitLineColor = FALSE;
}

// -----------------------------------------------------------------------

void OutputDevice::ImplInitFillColor()
{
    DBG_TESTSOLARMUTEX();

#ifndef REMOTE_APPSERVER
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
#else
    mpGraphics->SetFillColor( maFillColor );
#endif

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
        if ( pWindow->mpFrameData->mpFirstBackWin )
            pWindow->ImplInvalidateAllOverlapBackgrounds();
        if ( pWindow->mbInPaint )
            aRegion = *(pWindow->mpPaintRegion);
        else
            aRegion = *(pWindow->ImplGetWinChildClipRegion());
        if ( mbClipRegion )
            aRegion.Intersect( ImplPixelToDevicePixel( maRegion ) );
        if ( aRegion.IsEmpty() )
            mbOutputClipped = TRUE;
        else
        {
            mbOutputClipped = FALSE;
#ifndef REMOTE_APPSERVER
            ImplSelectClipRegion( mpGraphics, aRegion );
#else
            mpGraphics->SetClipRegion( aRegion );
#endif
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
#ifndef REMOTE_APPSERVER
                ImplSelectClipRegion( mpGraphics, maRegion );
#else
                mpGraphics->SetClipRegion( maRegion );
#endif
            }

            mbClipRegionSet = TRUE;
        }
        else
        {
            if ( mbClipRegionSet )
            {
#ifndef REMOTE_APPSERVER
                mpGraphics->ResetClipRegion();
#else
                mpGraphics->SetClipRegion();
#endif
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

void OutputDevice::SetClipRegion()
{
    DBG_TRACE( "OutputDevice::SetClipRegion()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaClipRegionAction( Region(), FALSE ) );

    ImplSetClipRegion( NULL );
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
        if ( pWindow->mbInPaint )
        {
            aRegion = *(pWindow->mpPaintRegion);
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
}

// -----------------------------------------------------------------------

void OutputDevice::SetDrawMode( ULONG nDrawMode )
{
    DBG_TRACE1( "OutputDevice::SetDrawMode( %lx )", nDrawMode );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    mnDrawMode = nDrawMode;
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

#ifndef REMOTE_APPSERVER
        if( mpGraphics || ImplGetGraphics() )
            mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp) );
#else
        ImplServerGraphics* pGraphics = ImplGetServerGraphics();
        if( pGraphics )
            pGraphics->SetRasterOp( eRasterOp );
#endif
    }
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
}

// -----------------------------------------------------------------------

void OutputDevice::SetLineColor( const Color& rColor )
{
    DBG_TRACE1( "OutputDevice::SetLineColor( %lx )", rColor.GetColor() );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );

    if( mnDrawMode & ( DRAWMODE_BLACKLINE | DRAWMODE_WHITELINE |
                       DRAWMODE_GRAYLINE | DRAWMODE_GHOSTEDLINE ) )
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
}

// -----------------------------------------------------------------------

void OutputDevice::SetFillColor( const Color& rColor )
{
    DBG_TRACE1( "OutputDevice::SetFillColor( %lx )", rColor.GetColor() );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );

    if( mnDrawMode & ( DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL |
                       DRAWMODE_GRAYFILL | DRAWMODE_NOFILL |
                       DRAWMODE_GHOSTEDFILL ) )
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
}

// -----------------------------------------------------------------------

void OutputDevice::SetBackground()
{
    DBG_TRACE( "OutputDevice::SetBackground()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    maBackground = Wallpaper();
    mbBackground = FALSE;
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
}

// -----------------------------------------------------------------------

void OutputDevice::DrawLine( const Point& rStartPt, const Point& rEndPt )
{
    DBG_TRACE( "OutputDevice::DrawLine()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaLineAction( rStartPt, rEndPt ) );

    if ( !IsDeviceOutputNecessary() || !mbLineColor )
        return;

#ifndef REMOTE_APPSERVER
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

    mpGraphics->DrawLine( aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y() );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        pGraphics->DrawLine( ImplLogicToDevicePixel( rStartPt ),
                             ImplLogicToDevicePixel( rEndPt ) );
    }
#endif
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

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ( LINE_NONE == rLineInfo.GetStyle() ) )
        return;

#ifndef REMOTE_APPSERVER

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
                mpGraphics->DrawPolygon( pPoly->GetSize(), (const SalPoint*) pPoly->ImplGetConstPointAry() );

            SetFillColor( aOldFillColor );
            SetLineColor( aOldLineColor );
        }
        else
        {
            if ( mbInitLineColor )
                ImplInitLineColor();

            for ( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
                mpGraphics->DrawLine( (*pPoly)[ 0 ].X(), (*pPoly)[ 0 ].Y(), (*pPoly)[ 1 ].X(), (*pPoly)[ 1 ].Y() );
        }
        mpMetaFile = pOldMetaFile;
    }
    else
    {
        const Point aStartPt( ImplLogicToDevicePixel( rStartPt ) );
        const Point aEndPt( ImplLogicToDevicePixel( rEndPt ) );

        if ( mbInitLineColor )
            ImplInitLineColor();

        mpGraphics->DrawLine( aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y() );
    }

#else

    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
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
                    pGraphics->DrawPolygon( *pPoly );

                SetLineColor( aOldLineColor );
                SetFillColor( aOldFillColor );
            }
            else
            {
                if ( mbInitLineColor )
                    ImplInitLineColor();

                for ( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() ) {
                    Point xPoint((*pPoly)[ 0 ].X(), (*pPoly)[ 0 ].Y());
                    Point yPoint((*pPoly)[ 1 ].X(), (*pPoly)[ 1 ].Y());
                    mpGraphics->DrawLine( xPoint, yPoint );
                }
            }
            mpMetaFile = pOldMetaFile;
        }
        else
        {
            if ( mbInitLineColor )
                ImplInitLineColor();

            pGraphics->DrawLine( ImplLogicToDevicePixel( rStartPt ), ImplLogicToDevicePixel( rEndPt ) );
        }
    }

#endif
}

// -----------------------------------------------------------------------

void OutputDevice::DrawRect( const Rectangle& rRect )
{
    DBG_TRACE( "OutputDevice::DrawRect()" );
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaRectAction( rRect ) );

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;
    aRect.Justify();

#ifndef REMOTE_APPSERVER
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

    mpGraphics->DrawRect( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight() );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( mbInitFillColor )
            ImplInitFillColor();
        pGraphics->DrawRect( aRect );
    }
#endif
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

    if ( !IsDeviceOutputNecessary() || !mbLineColor || (nPoints < 2) )
        return;

#ifndef REMOTE_APPSERVER
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

    const SalPoint* pPtAry = (const SalPoint*)aPoly.ImplGetConstPointAry();
    mpGraphics->DrawPolyLine( nPoints, pPtAry );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        pGraphics->DrawPolyLine( ImplLogicToDevicePixel( rPoly ) );
    }
#endif
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

    if ( !IsDeviceOutputNecessary() || !mbLineColor || ( nPoints < 2 ) || ( LINE_NONE == rLineInfo.GetStyle() ) )
        return;

#ifndef REMOTE_APPSERVER
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
        ImplLineConverter   aLineCvt( ImplLogicToDevicePixel( rPoly ), aInfo, ( mbRefPoint ) ? &maRefPoint : NULL );

        mpMetaFile = NULL;
        SetLineColor();
        ImplInitLineColor();
        SetFillColor( aOldLineColor );
        ImplInitFillColor();

        for( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
            mpGraphics->DrawPolygon( pPoly->GetSize(), (const SalPoint*) pPoly->ImplGetConstPointAry() );

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
            ImplLineConverter   aLineCvt( ImplLogicToDevicePixel( rPoly ), aInfo, ( mbRefPoint ) ? &maRefPoint : NULL );
            for( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
                mpGraphics->DrawPolyLine( pPoly->GetSize(), (const SalPoint*)pPoly->ImplGetConstPointAry() );
        }
        else
            mpGraphics->DrawPolyLine( nPoints, (const SalPoint*) ImplLogicToDevicePixel( rPoly ).ImplGetConstPointAry() );
    }
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();

    if ( pGraphics )
    {
        const LineInfo aInfo( ImplLogicToDevicePixel( rLineInfo ) );

        if( aInfo.GetWidth() > 1L )
        {
            const Color         aOldLineColor( maLineColor );
            const Color         aOldFillColor( maFillColor );
            GDIMetaFile*        pOldMetaFile = mpMetaFile;
            ImplLineConverter   aLineCvt( ImplLogicToDevicePixel( rPoly ), aInfo, ( mbRefPoint ) ? &maRefPoint : NULL );

            mpMetaFile = NULL;
            SetLineColor();
            ImplInitLineColor();
            SetFillColor( aOldLineColor );
            ImplInitFillColor();

            for( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
                pGraphics->DrawPolygon( *pPoly );

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
                ImplLineConverter   aLineCvt( ImplLogicToDevicePixel( rPoly ), aInfo, ( mbRefPoint ) ? &maRefPoint : NULL );
                for( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
                    pGraphics->DrawPolyLine( *pPoly );
            }
            else
                pGraphics->DrawPolyLine( ImplLogicToDevicePixel( rPoly ) );
        }
    }
#endif
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

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || (nPoints < 2) )
        return;

#ifndef REMOTE_APPSERVER
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

    const SalPoint* pPtAry = (const SalPoint*)aPoly.ImplGetConstPointAry();
    mpGraphics->DrawPolygon( nPoints, pPtAry );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( mbInitFillColor )
            ImplInitFillColor();
        pGraphics->DrawPolygon( ImplLogicToDevicePixel( rPoly ) );
    }
#endif
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

    if ( !IsDeviceOutputNecessary() || (!mbLineColor && !mbFillColor) || !nPoly )
        return;

#ifndef REMOTE_APPSERVER
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
        Polygon aPoly = ImplLogicToDevicePixel( rPolyPoly.GetObject( 0 ) );
        USHORT  nSize = aPoly.GetSize();
        if ( nSize >= 2 )
        {
            const SalPoint* pPtAry = (const SalPoint*)aPoly.ImplGetConstPointAry();
            mpGraphics->DrawPolygon( nSize, pPtAry );
        }
    }
    else
    {
        PolyPolygon         aPolyPoly = ImplLogicToDevicePixel( rPolyPoly );
        ULONG               aStackAry1[OUTDEV_POLYPOLY_STACKBUF];
        PCONSTSALPOINT      aStackAry2[OUTDEV_POLYPOLY_STACKBUF];
        ULONG*              pPointAry;
        PCONSTSALPOINT*     pPointAryAry;
        USHORT              i = 0;
        if ( nPoly > OUTDEV_POLYPOLY_STACKBUF )
        {
            pPointAry       = new ULONG[nPoly];
            pPointAryAry    = new PCONSTSALPOINT[nPoly];
        }
        else
        {
            pPointAry       = aStackAry1;
            pPointAryAry    = aStackAry2;
        }
        do
        {
            const Polygon&  rPoly = aPolyPoly.GetObject( i );
            USHORT          nSize = rPoly.GetSize();
            if ( nSize )
            {
                pPointAry[i]    = nSize;
                pPointAryAry[i] = (PCONSTSALPOINT)rPoly.ImplGetConstPointAry();
                i++;
            }
            else
                nPoly--;
        }
        while ( i < nPoly );

        if ( nPoly == 1 )
            mpGraphics->DrawPolygon( *pPointAry, *pPointAryAry );
        else
            mpGraphics->DrawPolyPolygon( nPoly, pPointAry, pPointAryAry );

        if ( pPointAry != aStackAry1 )
        {
            delete pPointAry;
            delete pPointAryAry;
        }
    }
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
    {
        if ( mbInitLineColor )
            ImplInitLineColor();
        if ( mbInitFillColor )
            ImplInitFillColor();
        if ( nPoly == 1 )
        {
            Polygon aPoly = ImplLogicToDevicePixel( rPolyPoly.GetObject( 0 ) );
            USHORT nSize = aPoly.GetSize();
            if ( nSize >= 2 )
                pGraphics->DrawPolygon( aPoly );
        }
        else
            pGraphics->DrawPolyPolygon( ImplLogicToDevicePixel( rPolyPoly ) );
    }
#endif
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

USHORT OutputDevice::GetBitCount() const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( meOutDevType == OUTDEV_VIRDEV )
        return ((VirtualDevice*)this)->mnBitCount;

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !((OutputDevice*)this)->ImplGetGraphics() )
            return 0;
    }
#endif

    return (USHORT)mpGraphics->GetBitCount();
}

// -----------------------------------------------------------------------

ULONG OutputDevice::GetColorCount() const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    const USHORT nBitCount = GetBitCount();
    return( ( nBitCount > 31 ) ? ULONG_MAX : ( ( (ULONG) 1 ) << nBitCount) );
}

// -----------------------------------------------------------------------

OpenGL* OutputDevice::GetOpenGL()
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

#ifndef REMOTE_APPSERVER
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
#else
    return NULL;
#endif
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > OutputDevice::CreateUnoGraphics()
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    return pWrapper ? pWrapper->CreateGraphics( this ) : ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >();
}
