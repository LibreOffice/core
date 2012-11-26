/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawDocShell.hxx"
#include <vcl/msgbox.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include "helpids.h"
#include "ViewShell.hxx"
#include "drawview.hxx"
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "View.hxx"
#include "ClientView.hxx"
#ifndef SD_WINDOW_SHELL_HXX
#include "Window.hxx"
#endif
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "strmname.h"
#include "fupoor.hxx"
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

namespace sd {

/*************************************************************************
|*
|* Zeichnen der DocShell (mittels der Hilfsklasse SdDrawViewShell)
|*
\************************************************************************/

void DrawDocShell::Draw(OutputDevice* pOut, const JobSetup&, sal_uInt16 nAspect)
{
    if (nAspect == ASPECT_THUMBNAIL)
    {
        /**********************************************************************
        * THUMBNAIL: Hier koennte ev. einmal der Draft-Mode gesetzt werden
        **********************************************************************/
    }

    ClientView* pView = new ClientView(this, pOut, NULL);

    pView->SetHlplVisible(false);
    pView->SetGridVisible(false);
    pView->SetBordVisible(false);
    pView->SetPageVisible(false);
    pView->SetGlueVisible(false);

    SdPage* pSelectedPage = NULL;

    List* pFrameViewList = mpDoc->GetFrameViewList();
    if( pFrameViewList && pFrameViewList->Count() )
    {
        FrameView* pFrameView = (FrameView*)pFrameViewList->GetObject(0);
        if( pFrameView && pFrameView->GetPageKind() == PK_STANDARD )
        {
            sal_uInt32 nSelectedPage = pFrameView->GetSelectedPage();
            pSelectedPage = mpDoc->GetSdPage(nSelectedPage, PK_STANDARD);
        }
    }

    if( NULL == pSelectedPage )
    {
        SdPage* pPage = NULL;
        sal_uInt32 nSelectedPage = 0;
        sal_uInt32 nPageCnt = mpDoc->GetSdPageCount(PK_STANDARD);

        for (sal_uInt32 i = 0; i < nPageCnt; i++)
        {
            pPage = mpDoc->GetSdPage(i, PK_STANDARD);

            if ( pPage->IsSelected() )
            {
                nSelectedPage = i;
                pSelectedPage = pPage;
            }
        }

        if( NULL == pSelectedPage )
            pSelectedPage = mpDoc->GetSdPage(0, PK_STANDARD);
    }

    Rectangle aVisArea = GetVisArea(nAspect);
    pOut->IntersectClipRegion(aVisArea);
    pView->ShowSdrPage(*pSelectedPage);

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

Rectangle DrawDocShell::GetVisArea(sal_uInt16 nAspect) const
{
    Rectangle aVisArea;

    if( ( ASPECT_THUMBNAIL == nAspect ) || ( ASPECT_DOCPRINT == nAspect ) )
    {
        // Groesse der ersten Seite herausgeben
        const basegfx::B2DVector& rPageScale = mpDoc->GetSdPage(0, PK_STANDARD)->GetPageScale();

        aVisArea.SetSize(Size(basegfx::fround(rPageScale.getX()), basegfx::fround(rPageScale.getY())));
    }
    else
    {
        aVisArea = SfxObjectShell::GetVisArea(nAspect);
    }

    if (aVisArea.IsEmpty() && mpViewShell)
    {
        Window* pWin = mpViewShell->GetActiveWindow();

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
    mpViewShell = pViewSh;
}

/*************************************************************************
|*
|* ViewShell abmelden
|*
\************************************************************************/

void DrawDocShell::Disconnect(ViewShell* pViewSh)
{
    if (mpViewShell == pViewSh)
    {
        mpViewShell = NULL;
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

    if (mpViewShell)
    {
        pFrameView = mpViewShell->GetFrameView();
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
|* Bitmap einer beliebigen Seite erzeugen
|*
\************************************************************************/

Bitmap DrawDocShell::GetPagePreviewBitmap(SdPage* pPage, sal_uInt16 nMaxEdgePixel)
{
    MapMode         aMapMode( MAP_100TH_MM );
    const basegfx::B2DVector& rPageScale = pPage->GetPageScale();
    const Size aSize(basegfx::fround(rPageScale.getX()), basegfx::fround(rPageScale.getY()));
    const Point     aNullPt;
    VirtualDevice   aVDev( *Application::GetDefaultDevice() );

    aVDev.SetMapMode( aMapMode );

    const Size  aPixSize( aVDev.LogicToPixel( aSize ) );
    const sal_uLong nMaxEdgePix = Max( aPixSize.Width(), aPixSize.Height() );
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
    pView->ShowSdrPage( *pPage );

    if ( GetFrameView() )
    {
        // Initialisierungen der Zeichen-(Bildschirm-)Attribute
        pView->SetGridCoarse( pFrameView->GetGridCoarse() );
        pView->SetGridFine( pFrameView->GetGridFine() );
        pView->SetSnapGridWidth(pFrameView->GetSnapGridWidthX(), pFrameView->GetSnapGridWidthY());
        pView->SetGridVisible( pFrameView->IsGridVisible() );
        pView->SetGridFront( pFrameView->IsGridFront() );
        pView->SetSnapAngle( pFrameView->GetSnapAngle() );
        pView->SetGridSnap( pFrameView->IsGridSnap() );
        pView->SetBorderSnap( pFrameView->IsBorderSnap() );
        pView->SetHelplineSnap( pFrameView->IsHelplineSnap() );
        pView->SetOFrameSnap( pFrameView->IsOFrameSnap() );
        pView->SetOPointSnap( pFrameView->IsOPointSnap() );
        pView->SetOConnectorSnap( pFrameView->IsOConnectorSnap() );
        pView->SetDragStripes( pFrameView->IsDragStripes() );
        pView->SetFrameHandles( pFrameView->IsFrameHandles() );
        pView->SetDiscreteMagneticSnap( pFrameView->GetDiscreteMagneticSnap() );
        pView->SetMarkedHitMovesAlways( pFrameView->IsMarkedHitMovesAlways() );
        pView->SetMoveOnlyDragging( pFrameView->IsMoveOnlyDragging() );
        pView->SetSlantButShear( pFrameView->IsSlantButShear() );
        pView->SetNoDragXorPolys( pFrameView->IsNoDragXorPolys() );
        pView->SetCrookNoContortion( pFrameView->IsCrookNoContortion() );
        pView->SetAngleSnapEnabled( pFrameView->IsAngleSnapEnabled() );
        pView->SetBigOrthogonal( pFrameView->IsBigOrthogonal() );
        pView->SetOrthogonal( pFrameView->IsOrthogonal() );

        SdrPageView* pPageView = pView->GetSdrPageView();

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
|* noch nicht vorhandenen Namen einzugeben. Wird false zurueckgegeben,
|* wurde die Aktion vom Anwender abgebrochen.
|*
\************************************************************************/

bool DrawDocShell::CheckPageName (::Window* pWin, String& rName )
{
    const String aStrForDlg( rName );
    bool bIsNameValid = IsNewPageNameValid( rName, true );

    if( ! bIsNameValid )
    {
        String aDesc( SdResId( STR_WARN_PAGE_EXISTS ) );
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        AbstractSvxNameDialog* aNameDlg = pFact ? pFact->CreateSvxNameDialog( pWin, aStrForDlg, aDesc ) : 0;
        if( aNameDlg )
        {
            aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

            if( mpViewShell )
                aNameDlg->SetCheckNameHdl( LINK( this, DrawDocShell, RenameSlideHdl ) );

            FunctionReference xFunc( mpViewShell->GetCurrentFunction() );
            if( xFunc.is() )
                xFunc->cancel();

            if( aNameDlg->Execute() == RET_OK )
            {
                aNameDlg->GetName( rName );
                bIsNameValid = IsNewPageNameValid( rName );
            }
            delete aNameDlg;
        }
    }

    return (bIsNameValid);
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
            bool   bOutDummy;
            sal_uInt32 nExistingPageNum = mpDoc->GetPageByName( rInOutPageName, bOutDummy );
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
