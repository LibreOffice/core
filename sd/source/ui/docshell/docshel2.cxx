/*************************************************************************
 *
 *  $RCSfile: docshel2.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:00:42 $
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

#include "DrawDocShell.hxx"

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVX_SVXIFACT_HXX //autogen
#include <svx/svxifact.hxx>
#endif
//CHINA001 #ifndef _SVX_DLGNAME_HXX //autogen
//CHINA001 #include <svx/dlgname.hxx>
//CHINA001 #endif
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#pragma hdrstop

#include "helpids.h"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "drawdoc.hxx"
#include "sdpage.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_CLIENT_VIEW_HXX
#include "ClientView.hxx"
#endif
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "strmname.h"
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif

namespace sd {

/*************************************************************************
|*
|* Zeichnen der DocShell (mittels der Hilfsklasse SdDrawViewShell)
|*
\************************************************************************/

void DrawDocShell::Draw(OutputDevice* pOut, const JobSetup& rSetup,
                                   USHORT nAspect)
{
    if (nAspect == ASPECT_THUMBNAIL)
    {
        /**********************************************************************
        * THUMBNAIL: Hier koennte ev. einmal der Draft-Mode gesetzt werden
        **********************************************************************/
    }

    ClientView* pView = new ClientView(this, pOut, NULL);

    pView->SetHlplVisible(FALSE);
    pView->SetGridVisible(FALSE);
    pView->SetBordVisible(FALSE);
    pView->SetPageVisible(FALSE);
    pView->SetGlueVisible(FALSE);

    SdPage* pSelectedPage = NULL;

    List* pFrameViewList = pDoc->GetFrameViewList();
    if( pFrameViewList && pFrameViewList->Count() )
    {
        FrameView* pFrameView = (FrameView*)pFrameViewList->GetObject(0);
        if( pFrameView && pFrameView->GetPageKind() == PK_STANDARD )
        {
            USHORT nSelectedPage = pFrameView->GetSelectedPage();
            pSelectedPage = pDoc->GetSdPage(nSelectedPage, PK_STANDARD);
        }
    }

    if( NULL == pSelectedPage )
    {
        SdPage* pPage = NULL;
        USHORT nSelectedPage = 0;
        USHORT nPageCnt = (USHORT) pDoc->GetSdPageCount(PK_STANDARD);

        for (USHORT i = 0; i < nPageCnt; i++)
        {
            pPage = pDoc->GetSdPage(i, PK_STANDARD);

            if ( pPage->IsSelected() )
            {
                nSelectedPage = i;
                pSelectedPage = pPage;
            }
        }

        if( NULL == pSelectedPage )
            pSelectedPage = pDoc->GetSdPage(0, PK_STANDARD);
    }

    Rectangle aVisArea = GetVisArea(nAspect);
    pOut->IntersectClipRegion(aVisArea);

    pView->ShowPage(pSelectedPage, Point());

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
        pView->CompleteRedraw(pOut, aRegion);

        if (pOut->GetOutDevType() == OUTDEV_PRINTER)
        {
            pOut->SetMapMode(aOldMapMode);
        }
    }

    delete pView;

//  Fuer Testzwecke: Bitte nicht entfernen!
//
//  GDIMetaFile* pMtf = pOut->GetConnectMetaFile();
//
//  if( pMtf )
//  {
//      String aURLStr;
//
//      if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( String( RTL_CONSTASCII_USTRINGPARAM( "d:\\gdi.mtf" ) ), aURLStr ) )
//      {
//          SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aURLStr, STREAM_WRITE | STREAM_TRUNC );
//
//          if( pOStm )
//          {
//              *pOStm << *pMtf;
//              delete pOStm;
//          }
//      }
//  }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawDocShell::SetVisArea(const Rectangle& rRect)
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

Rectangle DrawDocShell::GetVisArea(USHORT nAspect) const
{
    Rectangle aVisArea;

    if( ( ASPECT_THUMBNAIL == nAspect ) || ( ASPECT_DOCPRINT == nAspect ) )
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
|* ViewShell anmelden
|*
\************************************************************************/

void DrawDocShell::Connect(ViewShell* pViewSh)
{
    pViewShell = pViewSh;
}

/*************************************************************************
|*
|* ViewShell abmelden
|*
\************************************************************************/

void DrawDocShell::Disconnect(ViewShell* pViewSh)
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

FrameView* DrawDocShell::GetFrameView()
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

Size DrawDocShell::GetFirstPageSize()
{
    return SfxObjectShell::GetFirstPageSize();
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawDocShell::UIActivate( BOOL bActive )
{
    bUIActive = bActive;
    SfxInPlaceObject::UIActivate( bActive );
}

/*************************************************************************
|*
|* Bitmap einer beliebigen Seite erzeugen
|*
\************************************************************************/

Bitmap DrawDocShell::GetPagePreviewBitmap(SdPage* pPage, USHORT nMaxEdgePixel)
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

    ClientView* pView = new ClientView( this, &aVDev, NULL );
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

    pView->CompleteRedraw( &aVDev, Rectangle( aNullPt, aSize ) );

    // #111097# IsRedrawReady() always gives sal_True while ( !pView->IsRedrawReady() ) {}
    delete pView;

    aVDev.SetMapMode( MapMode() );

    Bitmap aPreview( aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() ) );

    DBG_ASSERT(!!aPreview, "Vorschau-Bitmap konnte nicht erzeugt werden");

    return aPreview;
}


/*************************************************************************
|*
|* Pruefen, ob die Seite vorhanden ist und dann den Anwender zwingen einen
|* noch nicht vorhandenen Namen einzugeben. Wird FALSE zurueckgegeben,
|* wurde die Aktion vom Anwender abgebrochen.
|*
\************************************************************************/

BOOL DrawDocShell::CheckPageName (::Window* pWin, String& rName )
{
    const String aStrForDlg( rName );
    bool bIsNameValid = IsNewPageNameValid( rName, true );

    if( ! bIsNameValid )
    {
        String aDesc( SdResId( STR_WARN_PAGE_EXISTS ) );
        //CHINA001 SvxNameDialog aNameDlg( pWin, aStrForDlg, aDesc );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
        AbstractSvxNameDialog* aNameDlg = pFact->CreateSvxNameDialog( pWin, aStrForDlg, aDesc, ResId(RID_SVXDLG_NAME) );
        DBG_ASSERT(aNameDlg, "Dialogdiet fail!");//CHINA001
        aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE ); //CHINA001 aNameDlg.SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

        if( pViewShell )
            aNameDlg->SetCheckNameHdl( LINK( this, DrawDocShell, RenameSlideHdl ) ); //CHINA001 aNameDlg.SetCheckNameHdl( LINK( this, SdDrawDocShell, RenameSlideHdl ) );

        FuPoor* pFunc = pViewShell->GetActualFunction();
        if( pFunc )
            pFunc->cancel();

        if( aNameDlg->Execute() == RET_OK ) //CHINA001 if( aNameDlg.Execute() == RET_OK )
        {
            aNameDlg->GetName( rName ); //CHINA001 aNameDlg.GetName( rName );
            bIsNameValid = IsNewPageNameValid( rName );
        }
        delete aNameDlg; //add by CHINA001
    }

    return ( bIsNameValid ? TRUE : FALSE );
}

bool DrawDocShell::IsNewPageNameValid( String & rInOutPageName, bool bResetStringIfStandardName /* = false */ )
{
    bool bCanUseNewName = false;

    // check if name is something like 'Slide n'
    String aStrPage( SdResId( STR_SD_PAGE ) );
    aStrPage += ' ';

    bool bIsStandardName = false;

    // prevent also _future_ slide names of the form "'STR_SD_PAGE' + ' ' + '[0-9]+|[a-z]|[A-Z]|[CDILMVX]+|[cdilmvx]+'"
    // (arabic, lower- and upper case single letter, lower- and upper case roman numbers)
    if( 0 == rInOutPageName.Search( aStrPage ) )
    {
        if( rInOutPageName.GetToken( 1, sal_Unicode(' ') ).GetChar(0) >= '0' &&
            rInOutPageName.GetToken( 1, sal_Unicode(' ') ).GetChar(0) <= '9' )
        {
            // check for arabic numbering

            // gobble up all following numbers
            String sRemainder = rInOutPageName.GetToken( 1, sal_Unicode(' ') );
            while( sRemainder.Len() &&
                   sRemainder.GetChar(0) >= '0' &&
                   sRemainder.GetChar(0) <= '9' )
            {
                // trim by one
                sRemainder.Erase(0, 1);
            }

            // EOL? Reserved name!
            if( !sRemainder.Len() )
            {
                bIsStandardName = true;
            }
        }
        else if( rInOutPageName.GetToken( 1, sal_Unicode(' ') ).GetChar(0) >= 'a' &&
                 rInOutPageName.GetToken( 1, sal_Unicode(' ') ).GetChar(0) <= 'z' &&
                 rInOutPageName.GetToken( 1, sal_Unicode(' ') ).Len() == 1 )
        {
            // lower case, single character: reserved
            bIsStandardName = true;
        }
        else if( rInOutPageName.GetToken( 1, sal_Unicode(' ') ).GetChar(0) >= 'A' &&
                 rInOutPageName.GetToken( 1, sal_Unicode(' ') ).GetChar(0) <= 'Z' &&
                 rInOutPageName.GetToken( 1, sal_Unicode(' ') ).Len() == 1 )
        {
            // upper case, single character: reserved
            bIsStandardName = true;
        }
        else
        {
            // check for upper/lower case roman numbering
            String sReserved( String::CreateFromAscii( "cdilmvx" ) );

            // gobble up all following characters contained in one reserved class
            String sRemainder = rInOutPageName.GetToken( 1, sal_Unicode(' ') );
            if( sReserved.Search( sRemainder.GetChar(0) ) == STRING_NOTFOUND )
                sReserved.ToUpperAscii();

            while( sReserved.Search( sRemainder.GetChar(0) ) != STRING_NOTFOUND )
            {
                // trim by one
                sRemainder.Erase(0, 1);
            }

            // EOL? Reserved name!
            if( !sRemainder.Len() )
            {
                bIsStandardName = true;
            }
        }
    }

    if( bIsStandardName )
    {
        if( bResetStringIfStandardName )
        {
            // this is for insertion of slides from other files with standard
            // name.  They get a new standard name, if the string is set to an
            // empty one.
            rInOutPageName = String();
            bCanUseNewName = true;
        }
        else
            bCanUseNewName = false;
    }
    else
    {
        if( rInOutPageName.Len() > 0 )
        {
            BOOL   bOutDummy;
            USHORT nExistingPageNum = pDoc->GetPageByName( rInOutPageName, bOutDummy );
            bCanUseNewName = ( nExistingPageNum == SDRPAGE_NOTFOUND );
        }
        else
            bCanUseNewName = false;
    }

    return bCanUseNewName;
}

IMPL_LINK( DrawDocShell, RenameSlideHdl, AbstractSvxNameDialog*, pDialog )
{
    if( ! pDialog )
        return 0;

    String aNewName;
    pDialog->GetName( aNewName );

    return IsNewPageNameValid( aNewName );
}
} // end of namespace sd
