/*************************************************************************
 *
 *  $RCSfile: drviewsa.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-17 11:08:34 $
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

#define ITEMID_SIZE 0

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SVX_SIZEITEM
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVDLAYER_HXX
#include <svx/svdlayer.hxx>
#endif
#ifndef _SVX_ZOOMITEM
#include <svx/zoomitem.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _XPOLY_HXX //autogen
#include <svx/xpoly.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDOPATH_HXX //autogen
#include <svx/svdopath.hxx>
#endif

#pragma hdrstop

#ifndef _SVX_FMSHELL_HXX            // XXX nur temp (dg)
#include <svx/fmshell.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#include "app.hrc"
#include "helpids.h"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdpage.hxx"
#include "frmview.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "docshell.hxx"
#include "sdwindow.hxx"
#include "fupoor.hxx"
#include "drviewsh.hxx"
#include "drbezob.hxx"
#include "drglueob.hxx"
#include "drtxtob.hxx"
#include "drgrfob.hxx"
#include "drstdob.hxx"
#include "grstdob.hxx"
#include "drawview.hxx"
#include "unoiview.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

BOOL SdDrawViewShell::bPipette = FALSE;

// ------------------------
// - ScannerEventListener -
// ------------------------

class ScannerEventListener : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
{
private:

    SdDrawViewShell*        mpParent;

public:

                            ScannerEventListener( SdDrawViewShell* pParent ) : mpParent( pParent )  {};
                            ~ScannerEventListener();

    // XEventListener
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& rEventObject );

    void                    ParentDestroyed() { mpParent = NULL; }
};

// -----------------------------------------------------------------------------

ScannerEventListener::~ScannerEventListener()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL ScannerEventListener::disposing( const ::com::sun::star::lang::EventObject& rEventObject )
{
    if( mpParent )
        mpParent->ScannerEvent( rEventObject );
}

/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

SdDrawViewShell::SdDrawViewShell(SfxViewFrame* pFrame, SfxViewShell *pOldShell) :
    SdViewShell(pFrame, &pFrame->GetWindow()),
    aTabControl(this, &pFrame->GetWindow()),
    aLayerTab(this, &pFrame->GetWindow()),
    aPageBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS),
    aMasterPageBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aLayerBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    pActualPage(NULL),
    bLayerMode(FALSE),
    pXPolygon (0),
    nPolygonIndex (0),
    bLineError (FALSE),
    bLastWasLineTo (FALSE),
    bLastWasMoveTo (FALSE),
    bLastWasBezierTo (FALSE),
    bMousePosFreezed (FALSE),
    nLastSlot(0),
    bReadOnly(pDocSh->IsReadOnly()),
    bInEffectAssignment(FALSE)
{
    if (pOldShell)
    {
        pFrameView = ( (SdViewShell*) pOldShell)->GetFrameView();
    }
    else
    {
        pFrameView = new FrameView(pDoc);
    }

    pFrameView->Connect();

    Construct(pDocSh);
    DestroyPolygons ();
    pXPolygon = new XPolygon;
}

/*************************************************************************
|*
|* Copy-Konstruktor
|*
\************************************************************************/

SdDrawViewShell::SdDrawViewShell(SfxViewFrame* pFrame,
                                 const SdDrawViewShell& rShell) :
    SdViewShell(pFrame, rShell),
    aTabControl(this, &pFrame->GetWindow()),
    aLayerTab(this, &pFrame->GetWindow()),
    aPageBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aMasterPageBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    aLayerBtn(&pFrame->GetWindow(), WB_3DLOOK | WB_RECTSTYLE | WB_SMALLSTYLE | WB_NOPOINTERFOCUS ),
    pActualPage(NULL),
    bLayerMode(FALSE),
    pXPolygon (0),
    nPolygonIndex (0),
    bLineError (FALSE),
    bLastWasLineTo (FALSE),
    bLastWasMoveTo (FALSE),
    bLastWasBezierTo (FALSE),
    bMousePosFreezed (FALSE),
    nLastSlot(0),
    bReadOnly(pDocSh->IsReadOnly()),
    bInEffectAssignment(FALSE)
{
    pFrameView = new FrameView(pDoc);
    pFrameView->Connect();

    Construct(pDocSh);
    DestroyPolygons ();
    pXPolygon = new XPolygon;
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

__EXPORT SdDrawViewShell::~SdDrawViewShell()
{
    if( mxScannerListener.is() )
        static_cast< ScannerEventListener* >( mxScannerListener.get() )->ParentDestroyed();

    EndListening(*GetViewFrame());

    if (pFuSlideShow)
    {
        // SlideShow ggf. beenden
        GetViewFrame()->GetDispatcher()->Execute(SID_PRESENTATION_END,
                    SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
    }

    if (pFuActual)
    {
        if (pFuOld == pFuActual)
            pFuOld = NULL;

        pFuActual->Deactivate();
        delete pFuActual;
        pFuActual = NULL;
    }

    if (pFuOld)
    {
        delete pFuOld;
        pFuOld = NULL;
    }

    SdPage* pPage;
    USHORT nSelectedPage = 0;
    USHORT aPageCnt = pDoc->GetSdPageCount(ePageKind);

    for (USHORT i = 0; i < aPageCnt; i++)
    {
        pPage = pDoc->GetSdPage(i, ePageKind);

        if (pPage == pActualPage)
        {
            pDoc->SetSelected(pPage, TRUE);
        }
        else
        {
            pDoc->SetSelected(pPage, FALSE);
        }
    }

    RemoveSubShell();

    // Umschaltung der ObjectBarShells unterdruecken, sonst versucht die
    // sterbende Funktion eventuell die ObjectBars zu wechseln. Die
    // entsprechende Shell ist aber schon vom SFX vom Dispatcher-Stack
    // genommen worden.
    bObjectBarSwitchEnabled = FALSE;

    delete pDrView;
    SetWindow(NULL);

    pFrameView->Disconnect();
    delete pXPolygon;
}

/*************************************************************************
|*
|* gemeinsamer Initialisierungsanteil der beiden Konstruktoren
|*
\************************************************************************/

void SdDrawViewShell::Construct(SdDrawDocShell* pDocSh)
{
    StartListening(*GetViewFrame());
    StartListening(*pDocSh);

    SetPool( &pDoc->GetPool() );

    pDoc->CreateFirstPages();

    pDrView = new SdDrawView(pDocSh, pWindow, this);
    pView = pDrView;             // Pointer der Basisklasse SdViewShell
    pDrView->SetSwapAsynchron(TRUE); // Asynchrones Laden von Graphiken

    pController = new SdXImpressView(pView, this);

    uno::Reference< awt::XWindow > aTmpRef;
    GetViewFrame()->GetFrame()->GetFrameInterface()->setComponent( aTmpRef, pController );

    ePageKind = pFrameView->GetPageKind();
    eEditMode = EM_PAGE;
    DocumentType eDocType = pDoc->GetDocumentType(); // RTTI fasst hier noch nicht

    // Shells fuer Object Bars erzeugen
    SfxShell* pObjBarShell;
    if( eDocType == DOCUMENT_TYPE_DRAW )
        pObjBarShell = new SdGraphicStdObjectBar(this, pDrView);
    else
        pObjBarShell = new SdDrawStdObjectBar(this, pDrView);
    // Die ID (Key) bleibt fuer beide ObjectBars gleich, um auch
    // den Aenderungs- und Pflegeaufwand gering zu halten
    aShellTable.Insert( RID_DRAW_OBJ_TOOLBOX, pObjBarShell );

    pObjBarShell = new SdDrawBezierObjectBar(this, pDrView);
    aShellTable.Insert(RID_BEZIER_TOOLBOX, pObjBarShell);

    pObjBarShell = new SdDrawGluePointsObjectBar(this, pDrView);
    aShellTable.Insert(RID_GLUEPOINTS_TOOLBOX, pObjBarShell);

    pObjBarShell = new SdDrawTextObjectBar( this, pDoc->GetPool(),
                                            pDrView );
    aShellTable.Insert( RID_DRAW_TEXT_TOOLBOX, pObjBarShell );

    aShellTable.Insert( RID_FORMLAYER_TOOLBOX, new FmFormShell( this, pDrView ) );

    aShellTable.Insert( RID_DRAW_GRAF_TOOLBOX, new SdDrawGrafObjectBar( this, pDrView ) );

    // ObjectBar einschalten
    SwitchObjectBar(RID_DRAW_OBJ_TOOLBOX);

    Size aPageSize = pDoc->GetSdPage(0, ePageKind)->GetSize();
    Point aPageOrg = Point(aPageSize.Width(), aPageSize.Height() / 2);
    Size aViewSize = Size(aPageSize.Width() * 3, aPageSize.Height() * 2);
    InitWindows(aPageOrg, aViewSize, Point(-1, -1));

    Point aVisAreaPos;

    if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        aVisAreaPos = pDocSh->GetVisArea(ASPECT_CONTENT).TopLeft();
    }

    pDrView->SetWorkArea(Rectangle(Point() - aVisAreaPos - aPageOrg, aViewSize));

    // Objekte koennen max. so gross wie die ViewSize werden
    pDoc->SetMaxObjSize(aViewSize);

    // Split-Handler fuer TabControls
    aTabControl.SetSplitHdl( LINK( this, SdDrawViewShell, TabSplitHdl ) );
    aLayerTab.SetSplitHdl( LINK( this, SdDrawViewShell, TabSplitHdl ) );

    aPageBtn.SetImage( Image( Bitmap( SdResId( BMP_TAB_PAGEMODE ) ), IMAGE_STDBTN_COLOR ) );
    aPageBtn.SetClickHdl(LINK(this, SdDrawViewShell, TabModeBtnHdl));
    aPageBtn.SetQuickHelpText( String( SdResId( STR_PAGEMODE ) ) );
    aPageBtn.SetHelpId( HID_SD_BTN_PAGE );
    aPageBtn.Show();

    aMasterPageBtn.SetImage( Image( Bitmap( SdResId( BMP_TAB_MASTERPAGE ) ), IMAGE_STDBTN_COLOR ) );
    aMasterPageBtn.SetClickHdl(LINK(this, SdDrawViewShell, TabModeBtnHdl));
    aMasterPageBtn.SetQuickHelpText( String( SdResId( STR_MASTERPAGEMODE ) ) );
    aMasterPageBtn.SetHelpId( HID_SD_BTN_MASTERPAGE );
    aMasterPageBtn.Show();

    aLayerBtn.SetImage( Image( Bitmap( SdResId( BMP_TAB_LAYERMODE ) ), IMAGE_STDBTN_COLOR ) );
    aLayerBtn.SetClickHdl(LINK(this, SdDrawViewShell, TabModeBtnHdl));
    aLayerBtn.SetQuickHelpText( String( SdResId( STR_LAYERMODE ) ) );
    aLayerBtn.SetHelpId( HID_SD_BTN_LAYER );
    aLayerBtn.Show();

    // Damit der richtige EditMode von der FrameView komplett eingestellt
    // werden kann, wird hier ein aktuell anderer gewaehlt (kleiner Trick)
    if (pFrameView->GetViewShEditMode(ePageKind) == EM_PAGE)
    {
        eEditMode = EM_MASTERPAGE;
    }
    else
    {
        eEditMode = EM_PAGE;
    }

    // Einstellungen der FrameView uebernehmen
    ReadFrameViewData(pFrameView);

    if( eDocType == DOCUMENT_TYPE_DRAW )
    {
        SetHelpId( SD_IF_SDGRAPHICVIEWSHELL );
        pWindow->SetHelpId( SD_IF_SDGRAPHICVIEWSHELL );
        pWindow->SetUniqueId( SD_IF_SDGRAPHICVIEWSHELL );
    }
    else
    {
        if (ePageKind == PK_NOTES)
        {
            aNotesBtn.Check(TRUE);
            aLayerBtn.Disable();
            SetHelpId( SID_NOTESMODE );
            pWindow->SetHelpId( SID_NOTESMODE );
            pWindow->SetUniqueId( SID_NOTESMODE );

            // AutoLayouts muessen erzeugt sein
            pDoc->StopWorkStartupDelay();
        }
        else if (ePageKind == PK_HANDOUT)
        {
            aHandoutBtn.Check(TRUE);
            aPageBtn.Disable();
            aLayerBtn.Disable();
            SetHelpId( SID_HANDOUTMODE );
            pWindow->SetHelpId( SID_HANDOUTMODE );
            pWindow->SetUniqueId( SID_HANDOUTMODE );

            // AutoLayouts muessen erzeugt sein
            pDoc->StopWorkStartupDelay();
        }
        else
        {
            aDrawBtn.Check(TRUE);
            SetHelpId( SD_IF_SDDRAWVIEWSHELL );
            pWindow->SetHelpId( SD_IF_SDDRAWVIEWSHELL );
            pWindow->SetUniqueId( SD_IF_SDDRAWVIEWSHELL );
        }
    }

    // Selektionsfunktion starten
    SfxRequest aReq(SID_OBJECT_SELECT, 0, pDoc->GetItemPool());
    FuPermanent(aReq);
    pDrView->SetFrameDragSingles(TRUE);

    if (pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)
    {
        bZoomOnPage = FALSE;
    }
    else
    {
        bZoomOnPage = TRUE;
    }

    bIsRulerDrag = FALSE;

    String aName( RTL_CONSTASCII_USTRINGPARAM( "DrawView" ));
    SetName (aName);

    if (pFrameView->GetPresentationViewShellId() != SID_VIEWSHELL0)
    {
        // Die Praesentation ist aus einer anderen ViewShell angewaehlt worden,
        // daraufhin wird zum Zeichentisch (SID_VIEWSHELL0) verzweigt und
        // die Praesentation wird gestartet.
        SfxAllItemSet aSet(pDoc->GetItemPool());
        SfxRequest aReq(pFrameView->GetSlotId(), 0, aSet);
        FuSupport(aReq);
    }

    nLockCount = 0UL;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );

    if( xMgr.is() )
    {
        mxScannerManager = ::com::sun::star::uno::Reference< ::com::sun::star::scanner::XScannerManager >(
                           xMgr->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.scanner.ScannerManager" ) ),
                           ::com::sun::star::uno::UNO_QUERY );

        if( mxScannerManager.is() )
        {
            mxScannerListener = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >(
                                static_cast< ::cppu::OWeakObject* >( new ScannerEventListener( this ) ),
                                ::com::sun::star::uno::UNO_QUERY );
        }
    }
}

/*************************************************************************
|*
|* pruefe ob linienzuege gezeichnet werden muessen
|*
\************************************************************************/

/*
    linienzuege werden ueber makros als folge von
        MoveTo (x, y)
        LineTo (x, y)   [oder BezierTo (x, y)]
        LineTo (x, y)
            :
    dargestellt. einen endbefehl fuer die linienzuege
    gibt es nicht, also muessen alle befehle in den
    requests nach LineTo (BezierTo) abgetestet und die
    punktparameter gesammelt werden.
    der erste nicht-LineTo fuehrt dann dazu, dass aus
    den gesammelten punkten der linienzug erzeugt wird
*/

void SdDrawViewShell::CheckLineTo (SfxRequest& rReq)
{
    if (rReq.IsAPI ())
        if ((rReq.GetSlot () == SID_LINETO) || (rReq.GetSlot () == SID_BEZIERTO))
        {
            if ((bLastWasLineTo && (rReq.GetSlot () == SID_LINETO)) ||
                (bLastWasBezierTo && (rReq.GetSlot () == SID_BEZIERTO)) ||
                bLastWasMoveTo)
            {
                const SfxItemSet* pArgs = rReq.GetArgs ();

                if (pArgs)
                    if (pArgs->Count () == 2)
                    {
                        SFX_REQUEST_ARG (rReq, pMouseStartX, SfxUInt32Item, ID_VAL_MOUSEEND_X, FALSE);
                        SFX_REQUEST_ARG (rReq, pMouseStartY, SfxUInt32Item, ID_VAL_MOUSEEND_Y, FALSE);

                        Point aTempPoint (pMouseStartX->GetValue (), pMouseStartY->GetValue ());
                        if (nPolygonIndex < 30) pXPolygon->Insert (nPolygonIndex ++, aTempPoint, XPOLY_NORMAL);

                        bLastWasLineTo   = (rReq.GetSlot () == SID_LINETO);
                        bLastWasBezierTo = (rReq.GetSlot () == SID_BEZIERTO);
                        bLastWasMoveTo   = FALSE;
                    }
                    else DestroyPolygons ();
                else DestroyPolygons ();
            }
            else DestroyPolygons ();
        }
        else
        {
            if (bLastWasLineTo || bLastWasBezierTo)
            {
                SdrPageView *pPV = pDrView->GetPageViewPvNum (0);

                pDrView->InsertObject (new SdrPathObj (bLastWasLineTo
                                                           ? OBJ_PLIN
                                                           : OBJ_PATHLINE, *pXPolygon), *pPV, SDRINSERT_SETDEFLAYER);
                if (bLastWasBezierTo) pDrView->ConvertMarkedToPathObj(FALSE);
                DestroyPolygons ();
            }

            if (rReq.GetSlot () == SID_MOVETO)
            {
                const SfxItemSet* pArgs = rReq.GetArgs ();

                if (pArgs)
                    if (pArgs->Count () == 2)
                    {
                        SFX_REQUEST_ARG (rReq, pMouseStartX, SfxUInt32Item, ID_VAL_MOUSESTART_X, FALSE);
                        SFX_REQUEST_ARG (rReq, pMouseStartY, SfxUInt32Item, ID_VAL_MOUSESTART_Y, FALSE);

                        nPolygonIndex = 0;
                        Point aTempPoint (pMouseStartX->GetValue (), pMouseStartY->GetValue ());
                        pXPolygon->Insert (nPolygonIndex ++, aTempPoint, XPOLY_NORMAL);

                        bLastWasMoveTo = TRUE;
                    }
                    else DestroyPolygons ();
                else DestroyPolygons ();
            }
            else bLastWasMoveTo = FALSE;
        }

    rReq.Ignore ();
}

/*************************************************************************
|*
|* loesche alle polygone, wenn noetig
|*
\************************************************************************/

void SdDrawViewShell::DestroyPolygons ()
{
    if (nPolygonIndex)
    {
        if (pXPolygon) delete pXPolygon;
        pXPolygon      = new XPolygon;
        nPolygonIndex  = 0;
        bLineError     =
        bLastWasLineTo =
        bLastWasBezierTo =
        bLastWasMoveTo = FALSE;
    }
}

/*************************************************************************
|*
|* veraendere die seitemparameter, wenn SID_PAGESIZE oder SID_PAGEMARGIN
|*
\************************************************************************/

void SdDrawViewShell::SetupPage (Size &rSize,
                                 long nLeft,
                                 long nRight,
                                 long nUpper,
                                 long nLower,
                                 BOOL bSize,
                                 BOOL bMargin,
                                 BOOL bScaleAll)
{
    USHORT nPageCnt = pDoc->GetMasterSdPageCount(ePageKind);
    USHORT i;

    for (i = 0; i < nPageCnt; i++)
    {
        /**********************************************************************
        * Erst alle MasterPages bearbeiten
        **********************************************************************/
        SdPage *pPage = pDoc->GetMasterSdPage(i, ePageKind);

        if( pPage )
        {
            const SfxPoolItem *pPoolItem = NULL;

            if( bSize )
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pPage->ScaleObjects(rSize, aBorderRect, bScaleAll);
                pPage->SetSize(rSize);

            }
            if( bMargin )
            {
                pPage->SetLftBorder(nLeft);
                pPage->SetRgtBorder(nRight);
                pPage->SetUppBorder(nUpper);
                pPage->SetLwrBorder(nLower);
            }

            if ( ePageKind == PK_STANDARD )
            {
                pDoc->GetMasterSdPage(i, PK_NOTES)->CreateTitleAndLayout();
            }

            pPage->CreateTitleAndLayout();
        }
    }

    nPageCnt = pDoc->GetSdPageCount(ePageKind);

    for (i = 0; i < nPageCnt; i++)
    {
        /**********************************************************************
        * Danach alle Pages bearbeiten
        **********************************************************************/
        SdPage *pPage = pDoc->GetSdPage(i, ePageKind);

        if( pPage )
        {
            if( bSize )
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pPage->ScaleObjects(rSize, aBorderRect, bScaleAll);
                pPage->SetSize(rSize);
            }
            if( bMargin )
            {
                pPage->SetLftBorder(nLeft);
                pPage->SetRgtBorder(nRight);
                pPage->SetUppBorder(nUpper);
                pPage->SetLwrBorder(nLower);
            }

            if ( ePageKind == PK_STANDARD )
            {
                SdPage* pNotesPage = pDoc->GetSdPage(i, PK_NOTES);
                pNotesPage->SetAutoLayout( pNotesPage->GetAutoLayout() );
            }

            pPage->SetAutoLayout( pPage->GetAutoLayout() );
        }
    }

    if ( ePageKind == PK_STANDARD )
    {
        SdPage* pHandoutPage = pDoc->GetSdPage(0, PK_HANDOUT);
        pHandoutPage->CreateTitleAndLayout(TRUE);
    }

    long nWidth = pActualPage->GetSize().Width();
    long nHeight = pActualPage->GetSize().Height();

    Point aPageOrg = Point(nWidth, nHeight / 2);
    Size aViewSize = Size(nWidth * 3, nHeight * 2);

    InitWindows(aPageOrg, aViewSize, Point(-1, -1), TRUE);
//    GetView()->SetWorkArea(Rectangle(Point(0,0) - aPageOrg, aViewSize));

    Point aVisAreaPos;

    if ( pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        aVisAreaPos = pDocSh->GetVisArea(ASPECT_CONTENT).TopLeft();
    }

    GetView()->SetWorkArea(Rectangle(Point() - aVisAreaPos - aPageOrg, aViewSize));

    UpdateScrollBars();

    Point aNewOrigin(pActualPage->GetLftBorder(), pActualPage->GetUppBorder());
    GetView()->GetPageViewPvNum(0)->SetPageOrigin(aNewOrigin);

    GetViewFrame()->GetBindings().Invalidate(SID_RULER_NULL_OFFSET);

    // auf (neue) Seitengroesse zoomen
    GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE,
                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
|* Statuswerte der Statusbar zurueckgeben
|*
\************************************************************************/

void __EXPORT SdDrawViewShell::GetStatusBarState(SfxItemSet& rSet)
{
    // Zoom-Item
    // Hier sollte der entsprechende Wert (Optimal ?, Seitenbreite oder
    // Seite) mit Hilfe des ZoomItems weitergegeben werden !!!
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_ATTR_ZOOM ) )
    {
        if (pDocSh->IsUIActive() || pFuSlideShow)
        {
            rSet.DisableItem( SID_ATTR_ZOOM );
        }
        else
        {
            SvxZoomItem* pZoomItem;
            UINT16 nZoom = (UINT16) pWindow->GetZoom();

            if( bZoomOnPage )
                pZoomItem = new SvxZoomItem( SVX_ZOOM_WHOLEPAGE, nZoom );
            else
                pZoomItem = new SvxZoomItem( SVX_ZOOM_PERCENT, nZoom );

            // Bereich einschraenken
            USHORT nZoomValues = SVX_ZOOM_ENABLE_ALL;
            SdrPageView* pPageView = pDrView->GetPageViewPvNum( 0 );

            if( ( pPageView && pPageView->GetObjList()->GetObjCount() == 0 ) )
                // || ( pDrView->GetMarkList().GetMarkCount() == 0 ) )
            {
                nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
            }

            pZoomItem->SetValueSet( nZoomValues );
            rSet.Put( *pZoomItem );
            delete pZoomItem;
        }
    }

    Point aPos = pWindow->PixelToLogic(aMousePos);
    pDrView->GetPageViewPvNum(0)->LogicToPagePos(aPos);
    Fraction aUIScale(pDoc->GetUIScale());
    aPos.X() = Fraction(aPos.X()) / aUIScale;
    aPos.Y() = Fraction(aPos.Y()) / aUIScale;

    // Position- und Groesse-Items
    if ( pDrView->IsAction() )
    {
        Rectangle aRect;
        pDrView->TakeActionRect( aRect );

        if ( aRect.IsEmpty() )
            rSet.Put( SfxPointItem(SID_ATTR_POSITION, aPos) );
        else
        {
            pDrView->GetPageViewPvNum(0)->LogicToPagePos(aRect);
            aPos = aRect.TopLeft();
            aPos.X() = Fraction(aPos.X()) / aUIScale;
            aPos.Y() = Fraction(aPos.Y()) / aUIScale;
            rSet.Put( SfxPointItem( SID_ATTR_POSITION, aPos) );
            Size aSize( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() );
            aSize.Height() = Fraction(aSize.Height()) / aUIScale;
            aSize.Width()  = Fraction(aSize.Width())  / aUIScale;
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, aSize) );
        }
    }
    else
    {
        rSet.Put( SfxPointItem(SID_ATTR_POSITION, aPos) );

        if ( pDrView->HasMarkedObj() )
        {
            Rectangle aRect = pDrView->GetAllMarkedRect();
            pDrView->GetPageViewPvNum(0)->LogicToPagePos(aRect);
            Size aSize( aRect.Right() - aRect.Left(), aRect.Bottom() - aRect.Top() );
            aSize.Height() = Fraction(aSize.Height()) / aUIScale;
            aSize.Width()  = Fraction(aSize.Width())  / aUIScale;
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, aSize) );
        }
        else
            rSet.Put( SvxSizeItem( SID_ATTR_SIZE, Size( 0, 0 ) ) );
    }

    // Seiten-/Ebenenanzeige
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STATUS_PAGE ) )
    {
        String aString;
        if( bLayerMode )
        {
            SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
            SdrLayerID nLayer, nOldLayer;
            SdrLayer*  pLayer = NULL;
            SdrObject* pObj = NULL;
            const SdrMarkList& rMarkList = pDrView->GetMarkList();
            ULONG nMarkCount = rMarkList.GetMarkCount();
            FASTBOOL bOneLayer = TRUE;

            for( ULONG j = 0; j < nMarkCount && bOneLayer && j < 10; j++ )
            {
                pObj = rMarkList.GetMark( j )->GetObj();
                if( pObj )
                {
                    nLayer = pObj->GetLayer();

                    if( j != 0 && nLayer != nOldLayer )
                        bOneLayer = FALSE;

                    nOldLayer = nLayer;
                }
            }
            if( bOneLayer && nMarkCount )
            {
                pLayer = rLayerAdmin.GetLayerPerID( nLayer );
                if( pLayer )
                    aString = pLayer->GetName();
            }
        }
        else
        {
            aString  = String( SdResId( STR_SD_PAGE ) );
            aString += sal_Unicode(' ');
            aString += UniString::CreateFromInt32( aTabControl.GetCurPageId() );
            aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " / " ));
            aString += UniString::CreateFromInt32( pDoc->GetSdPageCount( ePageKind ) );
        }
        rSet.Put( SfxStringItem( SID_STATUS_PAGE, aString ) );
    }
    // Layout
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STATUS_LAYOUT ) )
    {
        String aString = pActualPage->GetLayoutName();
        aString.Erase( aString.SearchAscii( SD_LT_SEPARATOR ) );
        rSet.Put( SfxStringItem( SID_STATUS_LAYOUT, aString ) );
    }
}


