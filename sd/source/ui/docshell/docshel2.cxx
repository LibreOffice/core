/*************************************************************************
 *
 *  $RCSfile: docshel2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:34 $
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

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVX_SVXIFACT_HXX //autogen
#include <svx/svxifact.hxx>
#endif

#pragma hdrstop

#include "docshell.hxx"
#include "viewshel.hxx"
#include "drawview.hxx"
#include "frmview.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdview.hxx"
#include "clview.hxx"
#include "dragserv.hxx"
#include "sdwindow.hxx"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "strmname.h"

/*************************************************************************
|*
|* Zeichnen der DocShell (mittels der Hilfsklasse SdDrawViewShell)
|*
\************************************************************************/

void __EXPORT SdDrawDocShell::Draw(OutputDevice* pOut, const JobSetup& rSetup,
                                   USHORT nAspect)
{
    if (nAspect == ASPECT_THUMBNAIL)
    {
        /**********************************************************************
        * THUMBNAIL: Hier koennte ev. einmal der Draft-Mode gesetzt werden
        **********************************************************************/
    }

    SdClientView* pView = new SdClientView(this, pOut, NULL);

    pView->SetHlplVisible(FALSE);
    pView->SetGridVisible(FALSE);
    pView->SetBordVisible(FALSE);
    pView->SetPageVisible(FALSE);
    pView->SetGlueVisible(FALSE);

    SdPage* pPage = NULL;
    USHORT nSelectedPage = 0;
    USHORT nPageCnt = (USHORT) pDoc->GetSdPageCount(PK_STANDARD);

    for (USHORT i = 0; i < nPageCnt; i++)
    {
        pPage = pDoc->GetSdPage(i, PK_STANDARD);

        if ( pPage->IsSelected() )
        {
            nSelectedPage = i;
        }
    }

    Rectangle aVisArea = GetVisArea(nAspect);
    pOut->IntersectClipRegion(aVisArea);

    pPage = pDoc->GetSdPage(nSelectedPage, PK_STANDARD);
    pView->ShowPage(pPage, Point());

    if (pOut->GetOutDevType() != OUTDEV_WINDOW)
    {
        MapMode aOldMapMode = pOut->GetMapMode();

        if (pOut->GetOutDevType() == OUTDEV_PRINTER)
        {
            MapMode aMapMode = aOldMapMode;
            Point aOrigin = aMapMode.GetOrigin();
            aOrigin.X() += 1;
            aOrigin.Y() += 1;
            aMapMode.SetOrigin(aOrigin);
            pOut->SetMapMode(aMapMode);
        }

        Region aRegion(aVisArea);
        pView->InitRedraw(pOut, aRegion);

        if (pOut->GetOutDevType() == OUTDEV_PRINTER)
        {
            pOut->SetMapMode(aOldMapMode);
        }
    }

    delete pView;

//    Fuer Testzwecke: Bitte nicht entfernen!
//
//    GDIMetaFile* pMtf = pOut->GetConnectMetaFile();
//
//    if (pMtf)
//    {
//        String aFile("d:\\sd\\winmsci.pro\\bin\\gdi.mtf");
//        SvFileStream aFStream(aFile, STREAM_WRITE);
//
//        pMtf->WindStart();
//        aFStream << *pMtf;
//
//        aFStream.Close();
//    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void __EXPORT SdDrawDocShell::SetVisArea(const Rectangle& rRect)
{
//    SfxInPlaceObject::SetVisArea(rRect);

    if (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)
    {
        // Setzt das Modified-Flag
        SfxInPlaceObject::SetVisArea(rRect);
    }
    else
    {
        SvEmbeddedObject::SetVisArea(rRect);
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

Rectangle __EXPORT SdDrawDocShell::GetVisArea(USHORT nAspect) const
{
    Rectangle aVisArea;

    if (nAspect == ASPECT_THUMBNAIL)
    {
        // Groesse der ersten Seite herausgeben
        MapMode aSrcMapMode(MAP_PIXEL);
        MapMode aDstMapMode(MAP_100TH_MM);
        Size aSize = pDoc->GetSdPage(0, PK_STANDARD)->GetSize();
        aSrcMapMode.SetMapUnit(MAP_100TH_MM);

        aSize = Application::GetDefaultDevice()->LogicToLogic(aSize, &aSrcMapMode, &aDstMapMode);
        aVisArea.SetSize(aSize);
    }
    else
    {
        aVisArea = SfxInPlaceObject::GetVisArea(nAspect);
    }

    if (aVisArea.IsEmpty() && pViewShell)
    {
        Window* pWin = pViewShell->GetActiveWindow();

        if (pWin)
        {
            aVisArea = pWin->PixelToLogic(Rectangle(Point(0,0), pWin->GetOutputSizePixel()));
        }
    }

    return (aVisArea);
}

/*************************************************************************
|*
|* SdViewShell anmelden
|*
\************************************************************************/

void __EXPORT SdDrawDocShell::Connect(SdViewShell* pViewSh)
{
    pViewShell = pViewSh;
}

/*************************************************************************
|*
|* SdViewShell abmelden
|*
\************************************************************************/

void __EXPORT SdDrawDocShell::Disconnect(SdViewShell* pViewSh)
{
    if (pViewShell == pViewSh)
    {
        pViewShell = NULL;
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

FrameView* __EXPORT SdDrawDocShell::GetFrameView()
{
    FrameView* pFrameView = NULL;

    if (pViewShell)
    {
        pFrameView = pViewShell->GetFrameView();
    }

    return(pFrameView);
}

/*************************************************************************
|*
|* Groesse der ersten Seite zurueckgeben
|*
\************************************************************************/

Size __EXPORT SdDrawDocShell::GetFirstPageSize()
{
/*
    DBG_ASSERT(pDoc, "kein Dokument");
    SdPage* pPage = pDoc->GetSdPage(0, PK_STANDARD);
    DBG_ASSERT (pPage, "keine erste Seite gefunden");
    return pPage->GetSize();
*/
    return SfxObjectShell::GetFirstPageSize();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

long __EXPORT SdDrawDocShell::DdeGetData( const String& rItem, SvData& rData )
{
//        return pDoc->GetData( rItem, rData );
    return 1;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

long __EXPORT SdDrawDocShell::DdeSetData( const String& rItem, const SvData& rData )
{
//        return pDoc->SetData( rItem, rData );
    return 1;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

SvPseudoObject* __EXPORT SdDrawDocShell::DdeCreateHotLink( const String& rItem )
{
//        return pDoc->CreateHotLink( rItem );
    return NULL;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void __EXPORT SdDrawDocShell::FillRegInfo( SvEmbeddedRegistryInfo * pInfo )
{
    SfxInPlaceObject::FillRegInfo( pInfo );
    pInfo->nMajorVers = 5;
    pInfo->nMinorVers = 0;

    if (pDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW)
    {
        pInfo->aObjName = String( RTL_CONSTASCII_USTRINGPARAM("StarDrawDocument") );
        pInfo->aHumanShortTypeName = String(SdResId(STR_GRAPHIC_DOCUMENT));
        pInfo->aExt = String(RTL_CONSTASCII_USTRINGPARAM(".sda"));
        pInfo->nIconResId = REG_ICON_RESID_GRAPHIC;
    }
    else
    {
        pInfo->aObjName = String( RTL_CONSTASCII_USTRINGPARAM("StarImpressDocument") );
        pInfo->aHumanShortTypeName = String(SdResId(STR_IMPRESS_DOCUMENT));
        pInfo->aExt = String(RTL_CONSTASCII_USTRINGPARAM(".sdd"));
        pInfo->nIconResId = REG_ICON_RESID_DRAW;
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void __EXPORT SdDrawDocShell::UIActivate( BOOL bActive )
{
    bUIActive = bActive;
    SfxInPlaceObject::UIActivate( bActive );
}

/*************************************************************************
|*
|* CreateSnapshot
|*
\************************************************************************/

SvDataMemberObjectRef __EXPORT SdDrawDocShell::CreateSnapshot()
{
    /**********************************************************************
    * Dokument vorhanden, View muss erzeugt werden
    **********************************************************************/
    VirtualDevice* pVDev = new VirtualDevice( *Application::GetDefaultDevice() );
    MapMode aMap = pVDev->GetMapMode();
    aMap.SetMapUnit( pDoc->GetScaleUnit() );
    aMap.SetScaleX( pDoc->GetScaleFraction() );
    aMap.SetScaleY( pDoc->GetScaleFraction() );
    pVDev->SetMapMode(aMap);

    SdView* pSdViewIntern = new SdView(pDoc, pVDev);
    pSdViewIntern->SetMarkHdlHidden(TRUE);

    ULONG nPageNum = 0;

    if (pDoc->GetPageCount() > 1)
    {
        // Mehr als eine Seite (default fuer Clipboard), daher die erste
        // Standard-Seite nehmen
        nPageNum = 1;
    }

    SdrPageView* pPageView = pSdViewIntern->ShowPagePgNum( nPageNum, Point() );
    ((SdrMarkView*)pSdViewIntern)->MarkAll(pPageView);
    Rectangle aVisArea = pSdViewIntern->GetAllMarkedRect();

    SdrPage* pPage = pDoc->GetPage(nPageNum);
    pPage->SetSize( aVisArea.GetSize() );

    ULONG nObjCount = pPage->GetObjCount();
    SdrObject* pObj = NULL;
    Point aOrigin = aVisArea.TopLeft();
    Size aVector( -aOrigin.X(), -aOrigin.Y() );

    for (ULONG nObj = 0; nObj < nObjCount; nObj++)
    {
        // Das BoundingRect aller Objekte wird auf (0, 0) positioniert
        pObj = pPage->GetObj(nObj);
        pObj->NbcMove(aVector);
    }

    SdDataObjectRef xDataObject = pSdViewIntern->CreateDataObject();

    delete pSdViewIntern;
    delete pVDev;

    // DataObject aufbauen und uebergeben
    return SvDataMemberObjectRef((SvObject*)&xDataObject);
}

/*************************************************************************
|*
|* Bitmap einer beliebigen Seite erzeugen
|*
\************************************************************************/

Bitmap SdDrawDocShell::GetPagePreviewBitmap(SdPage* pPage, USHORT nMaxEdgePixel)
{
    MapMode         aMapMode( MAP_100TH_MM );
    const Size      aSize( pPage->GetSize() );
    const Point     aNullPt;
    VirtualDevice   aVDev( *Application::GetDefaultDevice() );

    aVDev.SetMapMode( aMapMode );

    const Size  aPixSize( aVDev.LogicToPixel( aSize ) );
    const ULONG nMaxEdgePix = Max( aPixSize.Width(), aPixSize.Height() );
    Fraction    aFrac( nMaxEdgePixel, nMaxEdgePix );

    aMapMode.SetScaleX( aFrac );
    aMapMode.SetScaleY( aFrac );
    aVDev.SetMapMode( aMapMode );
    aVDev.SetOutputSize( aSize );

    // damit die dunklen Linien am rechten und unteren Seitenrans mitkommen
    aFrac = Fraction( nMaxEdgePixel - 1, nMaxEdgePix );
    aMapMode.SetScaleX( aFrac );
    aMapMode.SetScaleY( aFrac );
    aVDev.SetMapMode( aMapMode );

    SdClientView*   pView = new SdClientView( this, &aVDev, NULL );
    FrameView*      pFrameView = GetFrameView();

    pView->ShowPage( pPage, aNullPt );

    if ( GetFrameView() )
    {
        // Initialisierungen der Zeichen-(Bildschirm-)Attribute
        pView->SetGridCoarse( pFrameView->GetGridCoarse() );
        pView->SetGridFine( pFrameView->GetGridFine() );
        pView->SetSnapGrid( pFrameView->GetSnapGrid() );
        pView->SetSnapGridWidth(pFrameView->GetSnapGridWidthX(), pFrameView->GetSnapGridWidthY());
        pView->SetGridVisible( pFrameView->IsGridVisible() );
        pView->SetGridFront( pFrameView->IsGridFront() );
        pView->SetSnapAngle( pFrameView->GetSnapAngle() );
        pView->SetGridSnap( pFrameView->IsGridSnap() );
        pView->SetBordSnap( pFrameView->IsBordSnap() );
        pView->SetHlplSnap( pFrameView->IsHlplSnap() );
        pView->SetOFrmSnap( pFrameView->IsOFrmSnap() );
        pView->SetOPntSnap( pFrameView->IsOPntSnap() );
        pView->SetOConSnap( pFrameView->IsOConSnap() );
        pView->SetDragStripes( pFrameView->IsDragStripes() );
        pView->SetFrameDragSingles( pFrameView->IsFrameDragSingles() );
        pView->SetSnapMagneticPixel( pFrameView->GetSnapMagneticPixel() );
        pView->SetMarkedHitMovesAlways( pFrameView->IsMarkedHitMovesAlways() );
        pView->SetMoveOnlyDragging( pFrameView->IsMoveOnlyDragging() );
        pView->SetSlantButShear( pFrameView->IsSlantButShear() );
        pView->SetNoDragXorPolys( pFrameView->IsNoDragXorPolys() );
        pView->SetCrookNoContortion( pFrameView->IsCrookNoContortion() );
        pView->SetAngleSnapEnabled( pFrameView->IsAngleSnapEnabled() );
        pView->SetBigOrtho( pFrameView->IsBigOrtho() );
        pView->SetOrtho( pFrameView->IsOrtho() );

        SdrPageView* pPageView = pView->GetPageViewPvNum(0);

        if (pPageView)
        {
            if ( pPageView->GetVisibleLayers() != pFrameView->GetVisibleLayers() )
                pPageView->SetVisibleLayers( pFrameView->GetVisibleLayers() );

            if ( pPageView->GetPrintableLayers() != pFrameView->GetPrintableLayers() )
                pPageView->SetPrintableLayers( pFrameView->GetPrintableLayers() );

            if ( pPageView->GetLockedLayers() != pFrameView->GetLockedLayers() )
                pPageView->SetLockedLayers( pFrameView->GetLockedLayers() );

    //                if ( pPageView->GetHelpLines() != pFrameView->GetHelpLines() )
                pPageView->SetHelpLines( pFrameView->GetStandardHelpLines() );
        }

        if ( pView->GetActiveLayer() != pFrameView->GetActiveLayer() )
            pView->SetActiveLayer( pFrameView->GetActiveLayer() );
    }

    pView->InitRedraw( &aVDev, Rectangle( aNullPt, aSize ) );

    while ( !pView->IsRedrawReady() ) {}
    delete pView;

    aVDev.SetMapMode( MapMode() );

    Bitmap aPreview( aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() ) );

    DBG_ASSERT(!!aPreview, "Vorschau-Bitmap konnte nicht erzeugt werden");

    return aPreview;
}


