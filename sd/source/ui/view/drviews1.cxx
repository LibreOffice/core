/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "DrawViewShell.hxx"
#include "ViewShellImplementation.hxx"

#include "DrawController.hxx"
#include <com/sun/star/embed/EmbedStates.hpp>

#include "comphelper/anytostring.hxx"
#include "comphelper/scopeguard.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "rtl/ref.hxx"

#include <svx/svxids.hrc>
#include <svx/svdpagv.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/scrbar.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdopage.hxx>
#include <vcl/msgbox.hxx>
#include <sot/storage.hxx>
#include <svx/fmshell.hxx>
#include <svx/globl3d.hxx>
#include <svx/fmglob.hxx>
#include <editeng/outliner.hxx>
#include <svx/dialogs.hrc>

#include "view/viewoverlaymanager.hxx"

#include "glob.hrc"
#include "app.hrc"
#include "res_bmp.hrc"
#include "strings.hrc"
#include "helpids.h"

#include "app.hxx"
#include "fupoor.hxx"
#include "sdresid.hxx"
#include "fusel.hxx"
#include "sdpage.hxx"
#include "FrameView.hxx"
#include "stlpool.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "Ruler.hxx"
#include "Client.hxx"
#include "slideshow.hxx"
#include "optsitem.hxx"
#include "fusearch.hxx"
#include "Outliner.hxx"
#include "AnimationChildWindow.hxx"
#include "SdUnoDrawView.hxx"
#include "ToolBarManager.hxx"
#include "FormShellManager.hxx"
#include "ViewShellBase.hxx"
#include "LayerDialogChildWindow.hxx"
#include "LayerTabBar.hxx"
#include "ViewShellManager.hxx"
#include "ViewShellHint.hxx"

#include <sfx2/request.hxx>
#include <boost/bind.hpp>

#ifdef _MSC_VER
#if (_MSC_VER < 1400)
#pragma optimize ( "", off )
#endif
#endif

using namespace com::sun::star;

namespace sd {

void DrawViewShell::Activate(sal_Bool bIsMDIActivate)
{
    ViewShell::Activate(bIsMDIActivate);
}

void DrawViewShell::UIActivating( SfxInPlaceClient* pCli )
{
    ViewShell::UIActivating(pCli);

    // Disable own controls
    maTabControl.Disable();
    if (GetLayerTabControl() != NULL)
        GetLayerTabControl()->Disable();
}

void DrawViewShell::UIDeactivated( SfxInPlaceClient* pCli )
{
    // Enable own controls
    maTabControl.Enable();
    if (GetLayerTabControl() != NULL)
        GetLayerTabControl()->Enable();

    ViewShell::UIDeactivated(pCli);
}


/*************************************************************************
|*
|* Deactivate()
|*
\************************************************************************/

void DrawViewShell::Deactivate(sal_Bool bIsMDIActivate)
{
    ViewShell::Deactivate(bIsMDIActivate);
}

namespace
{
    class LockUI
    {
    private:
        void Lock(bool bLock);
        SfxViewFrame *mpFrame;
    public:
        LockUI(SfxViewFrame *pFrame) : mpFrame(pFrame) { Lock(true); }
        ~LockUI() { Lock(false); }

    };

    void LockUI::Lock(bool bLock)
    {
        if (!mpFrame)
            return;
        mpFrame->Enable( !bLock );
    }
}

/*************************************************************************
|*
|* Wird gerufen, wenn sich der Selektionszustand der View aendert
|*
\************************************************************************/
void DrawViewShell::SelectionHasChanged (void)
{
    Invalidate();

    //Update3DWindow(); // 3D-Controller
    SfxBoolItem aItem( SID_3D_STATE, sal_True );
    GetViewFrame()->GetDispatcher()->Execute(
        SID_3D_STATE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

    SdrOle2Obj* pOleObj = NULL;

    if ( mpDrawView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetMarkedSdrObj();

            sal_uInt32 nInv = pObj->GetObjInventor();
            sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv == SdrInventor && nSdrObjKind == OBJ_OLE2)
            {
                pOleObj = (SdrOle2Obj*) pObj;
                UpdateIMapDlg( pObj );
            }
            else if (nSdrObjKind == OBJ_GRAF)
                UpdateIMapDlg( pObj );
        }
    }

    ViewShellBase& rBase = GetViewShellBase();
    rBase.SetVerbs( uno::Sequence< embed::VerbDescriptor >() );

    try
    {
        Client* pIPClient = static_cast<Client*>(rBase.GetIPClient());
        if ( pIPClient && pIPClient->IsObjectInPlaceActive() )
        {
            /**********************************************************************
            * Ggf. OLE-Objekt beruecksichtigen und deaktivieren
            **********************************************************************/

            // this means we recently deselected an inplace active ole object so
            // we need to deselect it now
            if (!pOleObj)
            {
                //#i47279# disable frame until after object has completed unload
                LockUI aUILock(GetViewFrame());
                pIPClient->DeactivateObject();
                //HMHmpDrView->ShowMarkHdl();
            }
            else
            {
                uno::Reference < embed::XEmbeddedObject > xObj = pOleObj->GetObjRef();
                if ( xObj.is() )
                {
                    rBase.SetVerbs( xObj->getSupportedVerbs() );
                }
                else
                {
                    rBase.SetVerbs( uno::Sequence < embed::VerbDescriptor >() );
                }
            }
        }
        else
        {
            if ( pOleObj )
            {
                uno::Reference < embed::XEmbeddedObject > xObj = pOleObj->GetObjRef();
                if ( xObj.is() )
                {
                    rBase.SetVerbs( xObj->getSupportedVerbs() );
                }
                else
                {
                    rBase.SetVerbs( uno::Sequence < embed::VerbDescriptor >() );
                }
            }
            else
            {
                rBase.SetVerbs( uno::Sequence < embed::VerbDescriptor >() );
            }
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        OSL_FAIL(
            (rtl::OString("sd::DrawViewShell::SelectionHasChanged(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }

    if( HasCurrentFunction() )
    {
        GetCurrentFunction()->SelectionHasChanged();
    }
    else
    {
        GetViewShellBase().GetToolBarManager()->SelectionHasChanged(*this,*mpDrawView);
    }

    // Invalidate for every subshell
    GetViewShellBase().GetViewShellManager()->InvalidateAllSubShells(this);

    mpDrawView->UpdateSelectionClipboard( sal_False );

    GetViewShellBase().GetDrawController().FireSelectionChangeListener();
}


/*************************************************************************
|*
|* Zoomfaktor setzen
|*
\************************************************************************/

void DrawViewShell::SetZoom( long nZoom )
{
    // Make sure that the zoom factor will not be recalculated on
    // following window resizings.
    mbZoomOnPage = sal_False;
    ViewShell::SetZoom( nZoom );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
    mpViewOverlayManager->onZoomChanged();
}

/*************************************************************************
|*
|* Zoomrechteck fuer aktives Fenster einstellen
|*
\************************************************************************/

void DrawViewShell::SetZoomRect( const Rectangle& rZoomRect )
{
    ViewShell::SetZoomRect( rZoomRect );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
    mpViewOverlayManager->onZoomChanged();
}

/*************************************************************************
|*
|* PrepareClose, ggfs. Texteingabe beenden, damit andere Viewshells ein
|* aktualisiertes Textobjekt vorfinden
|*
\************************************************************************/

sal_uInt16 DrawViewShell::PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing )
{
    if ( ViewShell::PrepareClose(bUI, bForBrowsing) != sal_True )
        return sal_False;

    sal_Bool            bRet = sal_True;

    if( bRet && HasCurrentFunction() )
    {
        sal_uInt16 nID = GetCurrentFunction()->GetSlotID();
        if (nID == SID_TEXTEDIT || nID == SID_ATTR_CHAR)
        {
            mpDrawView->SdrEndTextEdit();
        }
    }
    else if( !bRet )
    {
        maCloseTimer.SetTimeoutHdl( LINK( this, DrawViewShell, CloseHdl ) );
        maCloseTimer.SetTimeout( 20 );
        maCloseTimer.Start();
    }

    return bRet;
}

/*************************************************************************
|*
|* Status (Enabled/Disabled) von Menue-SfxSlots setzen
|*
\************************************************************************/

void DrawViewShell::ChangeEditMode(EditMode eEMode, bool bIsLayerModeActive)
{
    if (meEditMode != eEMode || mbIsLayerModeActive != bIsLayerModeActive)
    {
        ViewShellManager::UpdateLock aLock (GetViewShellBase().GetViewShellManager());

        sal_uInt16 nActualPageNum = 0;

        GetViewShellBase().GetDrawController().FireChangeEditMode (eEMode == EM_MASTERPAGE);
        GetViewShellBase().GetDrawController().FireChangeLayerMode (bIsLayerModeActive);

        if ( mpDrawView->IsTextEdit() )
        {
            mpDrawView->SdrEndTextEdit();
        }

        LayerTabBar* pLayerBar = GetLayerTabControl();
        if (pLayerBar != NULL)
            pLayerBar->EndEditMode();
        maTabControl.EndEditMode();

        if (mePageKind == PK_HANDOUT)
        {
            // Bei Handzetteln nur MasterPage zulassen
            eEMode = EM_MASTERPAGE;
        }

        meEditMode = eEMode;
        mbIsLayerModeActive = bIsLayerModeActive;

        // Determine whether to show the master view toolbar.  The master
        // page mode has to be active and the shell must not be a handout
        // view.
        bool bShowMasterViewToolbar (meEditMode == EM_MASTERPAGE
             && GetShellType() != ViewShell::ST_HANDOUT);

        // If the master view toolbar is not shown we hide it before
        // switching the edit mode.
        if (::sd::ViewShell::mpImpl->mbIsInitialized
            && IsMainViewShell()
            && ! bShowMasterViewToolbar)
        {
            GetViewShellBase().GetToolBarManager()->ResetToolBars(ToolBarManager::TBG_MASTER_MODE);
        }

        if (meEditMode == EM_PAGE)
        {
            /******************************************************************
            * PAGEMODE
            ******************************************************************/

            maTabControl.Clear();

            SdPage* pPage;
            String aPageName;
            sal_uInt16 nPageCnt = GetDoc()->GetSdPageCount(mePageKind);

            for (sal_uInt16 i = 0; i < nPageCnt; i++)
            {
                pPage = GetDoc()->GetSdPage(i, mePageKind);
                aPageName = pPage->GetName();
                maTabControl.InsertPage(i + 1, aPageName);

                if ( pPage->IsSelected() && nActualPageNum == 0 )
                {
                    nActualPageNum = i;
                }
            }

            maTabControl.SetCurPageId(nActualPageNum + 1);

            SwitchPage(nActualPageNum);
        }
        else
        {
            /******************************************************************
            * MASTERPAGE
            ******************************************************************/
            GetViewFrame()->SetChildWindow(
                AnimationChildWindow::GetChildWindowId(), sal_False );

            if (!mpActualPage)
            {
                // Sofern es keine mpActualPage gibt, wird die erste genommen
                mpActualPage = GetDoc()->GetSdPage(0, mePageKind);
            }

            maTabControl.Clear();
            sal_uInt16 nActualMasterPageNum = 0;
            sal_uInt16 nMasterPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);

            for (sal_uInt16 i = 0; i < nMasterPageCnt; i++)
            {
                SdPage* pMaster = GetDoc()->GetMasterSdPage(i, mePageKind);
                String aLayoutName(pMaster->GetLayoutName());
                aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));

                maTabControl.InsertPage(i + 1, aLayoutName);

                if (&(mpActualPage->TRG_GetMasterPage()) == pMaster)
                {
                    nActualMasterPageNum = i;
                }
            }

            maTabControl.SetCurPageId(nActualMasterPageNum + 1);
            SwitchPage(nActualMasterPageNum);
        }

        // If the master view toolbar is to be shown we turn it on after the
        // edit mode has been changed.
        if (::sd::ViewShell::mpImpl->mbIsInitialized
            && IsMainViewShell()
            && bShowMasterViewToolbar)
        {
            GetViewShellBase().GetToolBarManager()->SetToolBar(
                ToolBarManager::TBG_MASTER_MODE,
                ToolBarManager::msMasterViewToolBar);
        }

        if ( ! mbIsLayerModeActive)
        {
            maTabControl.Show();
            // Set the tab control only for draw pages.  For master page
            // this has been done already above.
            if (meEditMode == EM_PAGE)
                maTabControl.SetCurPageId (nActualPageNum + 1);
        }
        /*AF: The LayerDialogChildWindow is not used anymore (I hope).
        if (GetViewFrame()->KnowsChildWindow(
            LayerDialogChildWindow::GetChildWindowId()))
        {
            GetViewFrame()->SetChildWindow(
                LayerDialogChildWindow::GetChildWindowId(),
                IsLayerModeActive());
        }
        */
        ResetActualLayer();

        Invalidate( SID_PAGEMODE );
        Invalidate( SID_LAYERMODE );
        Invalidate( SID_MASTERPAGE );
        Invalidate( SID_DELETE_MASTER_PAGE );
        Invalidate( SID_DELETE_PAGE );
        Invalidate( SID_SLIDE_MASTERPAGE );
        Invalidate( SID_TITLE_MASTERPAGE );
        Invalidate( SID_NOTES_MASTERPAGE );
        Invalidate( SID_HANDOUT_MASTERPAGE );
    }
}




bool DrawViewShell::IsLayerModeActive (void) const
{
    return mbIsLayerModeActive;
}




/*************************************************************************
|*
|* Groesse des TabControls und der ModeButtons zurueckgeben
|*
\************************************************************************/

long DrawViewShell::GetHCtrlWidth()
{
    //  return maTabControl.GetSizePixel().Width();
    return 0;
}


/*************************************************************************
|*
|* Horizontales Lineal erzeugen
|*
\************************************************************************/

SvxRuler* DrawViewShell::CreateHRuler (::sd::Window* pWin, sal_Bool bIsFirst)
{
    Ruler* pRuler;
    WinBits  aWBits;
    sal_uInt16   nFlags = SVXRULER_SUPPORT_OBJECT;

    if ( bIsFirst )
    {
        aWBits  = WB_HSCROLL | WB_3DLOOK | WB_BORDER | WB_EXTRAFIELD;
        nFlags |= ( SVXRULER_SUPPORT_SET_NULLOFFSET |
                    SVXRULER_SUPPORT_TABS |
                    SVXRULER_SUPPORT_PARAGRAPH_MARGINS ); // Neu
    }
    else
        aWBits = WB_HSCROLL | WB_3DLOOK | WB_BORDER;

    pRuler = new Ruler (*this, GetParentWindow(), pWin, nFlags,
        GetViewFrame()->GetBindings(), aWBits);
    pRuler->SetSourceUnit(pWin->GetMapMode().GetMapUnit());

    // Metric ...
    sal_uInt16 nMetric = (sal_uInt16)GetDoc()->GetUIUnit();

    if( nMetric == 0xffff )
        nMetric = (sal_uInt16)GetViewShellBase().GetViewFrame()->GetDispatcher()->GetModule()->GetFieldUnit();

    pRuler->SetUnit( FieldUnit( nMetric ) );

    // ... und auch DefTab am Lineal einstellen
    pRuler->SetDefTabDist( GetDoc()->GetDefaultTabulator() ); // Neu

    Fraction aUIScale(pWin->GetMapMode().GetScaleX());
    aUIScale *= GetDoc()->GetUIScale();
    pRuler->SetZoom(aUIScale);

    return pRuler;
}

/*************************************************************************
|*
|* Vertikales Lineal erzeugen
|*
\************************************************************************/

SvxRuler* DrawViewShell::CreateVRuler(::sd::Window* pWin)
{
    Ruler* pRuler;
    WinBits  aWBits = WB_VSCROLL | WB_3DLOOK | WB_BORDER;
    sal_uInt16   nFlags = SVXRULER_SUPPORT_OBJECT;

    pRuler = new Ruler(*this, GetParentWindow(), pWin, nFlags,
        GetViewFrame()->GetBindings(), aWBits);
    pRuler->SetSourceUnit(pWin->GetMapMode().GetMapUnit());

    // Metric same as HRuler, use document setting
    sal_uInt16 nMetric = (sal_uInt16)GetDoc()->GetUIUnit();

    if( nMetric == 0xffff )
        nMetric = (sal_uInt16)GetViewShellBase().GetViewFrame()->GetDispatcher()->GetModule()->GetFieldUnit();

    pRuler->SetUnit( FieldUnit( nMetric ) );

    Fraction aUIScale(pWin->GetMapMode().GetScaleY());
    aUIScale *= GetDoc()->GetUIScale();
    pRuler->SetZoom(aUIScale);

    return pRuler;
}

/*************************************************************************
|*
|* Horizontales Lineal aktualisieren
|*
\************************************************************************/

void DrawViewShell::UpdateHRuler()
{
    Invalidate( SID_ATTR_LONG_LRSPACE );
    Invalidate( SID_RULER_PAGE_POS );
    Invalidate( SID_RULER_OBJECT );
    Invalidate( SID_RULER_TEXT_RIGHT_TO_LEFT );

    if (mpHorizontalRuler.get() != NULL)
        mpHorizontalRuler->ForceUpdate();
}

/*************************************************************************
|*
|* Vertikales Lineal aktualisieren
|*
\************************************************************************/

void DrawViewShell::UpdateVRuler()
{
    Invalidate( SID_ATTR_LONG_LRSPACE );
    Invalidate( SID_RULER_PAGE_POS );
    Invalidate( SID_RULER_OBJECT );

    if (mpVerticalRuler.get() != NULL)
        mpVerticalRuler->ForceUpdate();
}

/*************************************************************************
|*
|* Metrik setzen
|*
\************************************************************************/

void DrawViewShell::SetUIUnit(FieldUnit eUnit)
{
    ViewShell::SetUIUnit(eUnit);
}

/*************************************************************************
|*
|* TabControl nach Splitteraenderung aktualisieren
|*
\************************************************************************/

IMPL_LINK( DrawViewShell, TabSplitHdl, TabBar *, pTab )
{
    const long int nMax = maViewSize.Width() - maScrBarWH.Width()
        - maTabControl.GetPosPixel().X() ;

    Size aTabSize = maTabControl.GetSizePixel();
    aTabSize.Width() = Min(pTab->GetSplitSize(), (long)(nMax-1));

    maTabControl.SetSizePixel(aTabSize);
    GetLayerTabControl()->SetSizePixel(aTabSize);

    Point aPos = maTabControl.GetPosPixel();
    aPos.X() += aTabSize.Width();

    Size aScrSize(nMax - aTabSize.Width(), maScrBarWH.Height());
    mpHorizontalScrollBar->SetPosSizePixel(aPos, aScrSize);

    return 0;
}

/// inherited from sd::ViewShell
SdPage* DrawViewShell::getCurrentPage() const
{
    const sal_Int32 nPageCount = (meEditMode == EM_PAGE)?
                                    GetDoc()->GetSdPageCount(mePageKind):
                                    GetDoc()->GetMasterSdPageCount(mePageKind);

    sal_Int32 nCurrentPage = maTabControl.GetCurPageId() - 1;
    DBG_ASSERT( (nPageCount>0) && (nCurrentPage<nPageCount), "sd::DrawViewShell::getCurrentPage(), illegal page index!" );
    if( (nPageCount < 0) || (nCurrentPage>=nPageCount) )
        nCurrentPage = 0; // play safe here

    if (meEditMode == EM_PAGE)
    {
        return GetDoc()->GetSdPage((sal_uInt16)nCurrentPage, mePageKind);
    }
    else // EM_MASTERPAGE
    {
        return GetDoc()->GetMasterSdPage((sal_uInt16)nCurrentPage, mePageKind);
    }
}

/*************************************************************************
|*
|* neue aktuelle Seite auswaehlen, falls sich die Seitenfolge geaendert
|* hat (z. B. durch Undo)
|*
\************************************************************************/

void DrawViewShell::ResetActualPage()
{
    if (!GetDoc())
        return;

    sal_uInt16 nCurrentPage = maTabControl.GetCurPageId() - 1;
    sal_uInt16 nPageCount   = (meEditMode == EM_PAGE)?GetDoc()->GetSdPageCount(mePageKind):GetDoc()->GetMasterSdPageCount(mePageKind);
    if (nPageCount > 0)
        nCurrentPage = Min((sal_uInt16)(nPageCount - 1), nCurrentPage);
    else
        nCurrentPage = 0;

    if (meEditMode == EM_PAGE)
    {

        // Update fuer TabControl
        maTabControl.Clear();

        SdPage* pPage = NULL;
        String aPageName;

        for (sal_uInt16 i = 0; i < nPageCount; i++)
        {
            pPage = GetDoc()->GetSdPage(i, mePageKind);
            aPageName = pPage->GetName();
            maTabControl.InsertPage(i + 1, aPageName);

            // Selektionskennungen der Seiten korrigieren
            GetDoc()->SetSelected(pPage, i == nCurrentPage);
        }

        maTabControl.SetCurPageId(nCurrentPage + 1);
    }
    else // EM_MASTERPAGE
    {
        SdPage* pActualPage = GetDoc()->GetMasterSdPage(nCurrentPage, mePageKind);
        maTabControl.Clear();
        sal_uInt16 nActualMasterPageNum = 0;

        sal_uInt16 nMasterPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);
        for (sal_uInt16 i = 0; i < nMasterPageCnt; i++)
        {
            SdPage* pMaster = GetDoc()->GetMasterSdPage(i, mePageKind);
            String aLayoutName(pMaster->GetLayoutName());
            aLayoutName.Erase(aLayoutName.SearchAscii(SD_LT_SEPARATOR));
            maTabControl.InsertPage(i + 1, aLayoutName);

            if (pActualPage == pMaster)
                nActualMasterPageNum = i;
        }

        maTabControl.SetCurPageId(nActualMasterPageNum + 1);
        SwitchPage(nActualMasterPageNum);
    }

    GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHPAGE,
                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
}

/*************************************************************************
|*
|* Verb auf OLE-Objekt anwenden
|*
\************************************************************************/


ErrCode DrawViewShell::DoVerb(long nVerb)
{
    if ( mpDrawView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetMarkedSdrObj();

            sal_uInt32 nInv = pObj->GetObjInventor();
            sal_uInt16 nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv == SdrInventor && nSdrObjKind == OBJ_OLE2)
            {
                ActivateObject( (SdrOle2Obj*) pObj, nVerb);
            }
        }
    }

    return 0;
}


/*************************************************************************
|*
|* OLE-Object aktivieren
|*
\************************************************************************/

sal_Bool DrawViewShell::ActivateObject(SdrOle2Obj* pObj, long nVerb)
{
    sal_Bool bActivated = sal_False;

    if ( !GetDocSh()->IsUIActive() )
    {
        ToolBarManager::UpdateLock aLock (GetViewShellBase().GetToolBarManager());

        bActivated = ViewShell::ActivateObject(pObj, nVerb);

        OSL_ASSERT(GetViewShell()!=NULL);
        Client* pClient = static_cast<Client*>(GetViewShell()->GetIPClient());
        if (pClient)
            pClient->SetSdrGrafObj(NULL);
    }

    return(bActivated);
}

/*************************************************************************
|*
|* Auf gewuenschte Seite schalten
|* Der Parameter nSelectedPage bezieht sich auf den aktuellen EditMode
|*
\************************************************************************/

void LclResetFlag (bool& rbFlag) {rbFlag = false;}

sal_Bool DrawViewShell::SwitchPage(sal_uInt16 nSelectedPage)
{
    /** Under some circumstances there are nested calls to SwitchPage() and
        may crash the application (activation of form controls when the
        shell of the edit view is not on top of the shell stack, see issue
        83888 for details.)  Therefore the nested calls are ignored (they
        would jump to the wrong page anyway.)
    */
    if (mbIsInSwitchPage)
        return sal_False;
    mbIsInSwitchPage = true;
    comphelper::ScopeGuard aGuard (::boost::bind(LclResetFlag, ::boost::ref(mbIsInSwitchPage)));

    if (GetActiveWindow()->IsInPaint())
    {
        // Switching the current page while a Paint is being executed is
        // dangerous.  So, post it for later execution and return.
        maAsynchronousSwitchPageCall.Post(::boost::bind(
            ::std::mem_fun(&DrawViewShell::SwitchPage),
            this,
            nSelectedPage));
        return sal_False;
    }

    sal_Bool bOK = sal_False;

    // With the current implementation of FuSlideShow there is a problem
    // when it dsplays the show in a window: When the show is stopped it
    // returns at one point in time SDRPAGE_NOTFOUND as current page index.
    // Because FuSlideShow is currently being rewritten this bug is fixed
    // here.
    // This is not as bad a hack as it may look because making SwitchPage()
    // more robust with respect to invalid page numbers is a good thing
    // anyway.
    if (nSelectedPage == SDRPAGE_NOTFOUND)
    {
        nSelectedPage = 0;
    }
    else
    {
        // Make sure that the given page index points to an existing page.  Move
        // the index into the valid range if necessary.
        sal_uInt16 nPageCount = (meEditMode == EM_PAGE)
            ? GetDoc()->GetSdPageCount(mePageKind)
            : GetDoc()->GetMasterSdPageCount(mePageKind);
        if (nSelectedPage >= nPageCount)
            nSelectedPage = nPageCount-1;
    }

    if (IsSwitchPageAllowed())
    {
        ModifyGuard aGuard2( GetDoc() );

        bOK = sal_True;

        if (mpActualPage)
        {
            SdPage* pNewPage = NULL;

            if (meEditMode == EM_MASTERPAGE)
            {
                if( GetDoc()->GetMasterSdPageCount(mePageKind) > nSelectedPage )
                    pNewPage = GetDoc()->GetMasterSdPage(nSelectedPage, mePageKind);

                if( pNewPage )
                {
                    SdrPageView* pPV = mpDrawView->GetSdrPageView();
                    OUString sPageText(pNewPage->GetLayoutName());
                    sal_Int32 nPos = sPageText.indexOf(SD_LT_SEPARATOR);
                    if (nPos != -1)
                        sPageText = sPageText.copy(0, nPos);
                    if (pPV
                        && pNewPage == dynamic_cast< SdPage* >( pPV->GetPage() )
                        && sPageText == maTabControl.GetPageText(nSelectedPage+1))
                    {
                        // this slide is already visible
                        return sal_True;
                    }
                }
            }
            else
            {
                OSL_ASSERT(mpFrameView!=NULL);
                mpFrameView->SetSelectedPage(nSelectedPage);

                if (GetDoc()->GetSdPageCount(mePageKind) > nSelectedPage)
                    pNewPage = GetDoc()->GetSdPage(nSelectedPage, mePageKind);

                if (mpActualPage == pNewPage)
                {
                    SdrPageView* pPV = mpDrawView->GetSdrPageView();

                    SdPage* pCurrentPage = dynamic_cast< SdPage* >( pPV->GetPage());
                    if (pPV
                        && pNewPage == pCurrentPage
                        && maTabControl.GetPageText(nSelectedPage+1).equals(pNewPage->GetName()))
                    {
                        // this slide is already visible
                        return sal_True;
                    }
                }
            }
        }

        if( mpDrawView )
            mpDrawView->SdrEndTextEdit();

        mpActualPage = NULL;

        if (meEditMode == EM_PAGE)
        {
            mpActualPage = GetDoc()->GetSdPage(nSelectedPage, mePageKind);
        }
        else
        {
            SdPage* pMaster = GetDoc()->GetMasterSdPage(nSelectedPage, mePageKind);

            // Passt die selektierte Seite zur MasterPage?
            sal_uInt16 nPageCount = GetDoc()->GetSdPageCount(mePageKind);
            for (sal_uInt16 i = 0; i < nPageCount; i++)
            {
                SdPage* pPage = GetDoc()->GetSdPage(i, mePageKind);
                if(pPage && pPage->IsSelected() && pMaster == &(pPage->TRG_GetMasterPage()))
                {
                    mpActualPage = pPage;
                    break;
                }
            }

            if (!mpActualPage)
            {
                // Die erste Seite nehmen, welche zur MasterPage passt
                for (sal_uInt16 i = 0; i < nPageCount; i++)
                {
                    SdPage* pPage = GetDoc()->GetSdPage(i, mePageKind);
                    if(pPage && pMaster == &(pPage->TRG_GetMasterPage()))
                    {
                        mpActualPage = pPage;
                        break;
                    }
                }
            }
        }

        for (sal_uInt16 i = 0; i < GetDoc()->GetSdPageCount(mePageKind); i++)
        {
            // Alle Seiten deselektieren
            GetDoc()->SetSelected( GetDoc()->GetSdPage(i, mePageKind), sal_False);
        }

        if (!mpActualPage)
        {
            // Sofern es keine mpActualPage gibt, wird die erste genommen
            mpActualPage = GetDoc()->GetSdPage(0, mePageKind);
        }

        // diese Seite auch selektieren (mpActualPage zeigt immer auf Zeichenseite,
        // nie auf eine Masterpage)
        GetDoc()->SetSelected(mpActualPage, sal_True);

        rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( GetDoc() ) );
        if( !xSlideshow.is() || !xSlideshow->isRunning() || ( xSlideshow->getAnimationMode() != ANIMATIONMODE_SHOW ) )
        {
            // VisArea zuziehen, um ggf. Objekte zu deaktivieren
            // !!! only if we are not in presentation mode (#96279) !!!
            OSL_ASSERT (GetViewShell()!=NULL);
            GetViewShell()->DisconnectAllClients();
            VisAreaChanged(Rectangle(Point(), Size(1, 1)));
        }

        if (meEditMode == EM_PAGE)
        {
            /**********************************************************************
            * PAGEMODE
            **********************************************************************/
            GetDoc()->SetSelected(mpActualPage, sal_True);

            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if (pPageView)
            {
                mpFrameView->SetVisibleLayers( pPageView->GetVisibleLayers() );
                mpFrameView->SetPrintableLayers( pPageView->GetPrintableLayers() );
                mpFrameView->SetLockedLayers( pPageView->GetLockedLayers() );

                if (mePageKind == PK_NOTES)
                {
                    mpFrameView->SetNotesHelpLines( pPageView->GetHelpLines() );
                }
                else if (mePageKind == PK_HANDOUT)
                {
                    mpFrameView->SetHandoutHelpLines( pPageView->GetHelpLines() );
                }
                else
                {
                    mpFrameView->SetStandardHelpLines( pPageView->GetHelpLines() );
                }
            }

            mpDrawView->HideSdrPage();
            mpDrawView->ShowSdrPage(mpActualPage);
            GetViewShellBase().GetDrawController().FireSwitchCurrentPage(mpActualPage);

            SdrPageView* pNewPageView = mpDrawView->GetSdrPageView();

            if (pNewPageView)
            {
                pNewPageView->SetVisibleLayers( mpFrameView->GetVisibleLayers() );
                pNewPageView->SetPrintableLayers( mpFrameView->GetPrintableLayers() );
                pNewPageView->SetLockedLayers( mpFrameView->GetLockedLayers() );

                if (mePageKind == PK_NOTES)
                {
                    pNewPageView->SetHelpLines( mpFrameView->GetNotesHelpLines() );
                }
                else if (mePageKind == PK_HANDOUT)
                {
                    pNewPageView->SetHelpLines( mpFrameView->GetHandoutHelpLines() );
                }
                else
                {
                    pNewPageView->SetHelpLines( mpFrameView->GetStandardHelpLines() );
                }
            }

            maTabControl.SetCurPageId(nSelectedPage+1);
            OUString aPageName = mpActualPage->GetName();

            if (maTabControl.GetPageText(nSelectedPage+1) != aPageName)
            {
                maTabControl.SetPageText(nSelectedPage+1, aPageName);
            }
        }
        else
        {
            /**********************************************************************
            * MASTERPAGE
            **********************************************************************/
            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if (pPageView)
            {
                mpFrameView->SetVisibleLayers( pPageView->GetVisibleLayers() );
                mpFrameView->SetPrintableLayers( pPageView->GetPrintableLayers() );
                mpFrameView->SetLockedLayers( pPageView->GetLockedLayers() );

                if (mePageKind == PK_NOTES)
                {
                    mpFrameView->SetNotesHelpLines( pPageView->GetHelpLines() );
                }
                else if (mePageKind == PK_HANDOUT)
                {
                    mpFrameView->SetHandoutHelpLines( pPageView->GetHelpLines() );
                }
                else
                {
                    mpFrameView->SetStandardHelpLines( pPageView->GetHelpLines() );
                }
            }

            mpDrawView->HideSdrPage();

            SdPage* pMaster = GetDoc()->GetMasterSdPage(nSelectedPage, mePageKind);

            if( !pMaster )              // Falls es diese Page nicht geben sollte
                pMaster = GetDoc()->GetMasterSdPage(0, mePageKind);

            sal_uInt16 nNum = pMaster->GetPageNum();
            mpDrawView->ShowSdrPage(mpDrawView->GetModel()->GetMasterPage(nNum));

            GetViewShellBase().GetDrawController().FireSwitchCurrentPage(pMaster);

            SdrPageView* pNewPageView = mpDrawView->GetSdrPageView();

            if (pNewPageView)
            {
                pNewPageView->SetVisibleLayers( mpFrameView->GetVisibleLayers() );
                pNewPageView->SetPrintableLayers( mpFrameView->GetPrintableLayers() );
                pNewPageView->SetLockedLayers( mpFrameView->GetLockedLayers() );

                if (mePageKind == PK_NOTES)
                {
                    pNewPageView->SetHelpLines( mpFrameView->GetNotesHelpLines() );
                }
                else if (mePageKind == PK_HANDOUT)
                {
                    pNewPageView->SetHelpLines( mpFrameView->GetHandoutHelpLines() );
                }
                else
                {
                    pNewPageView->SetHelpLines( mpFrameView->GetStandardHelpLines() );
                }
            }

            OUString aLayoutName(pMaster->GetLayoutName());
            sal_Int32 nPos = aLayoutName.indexOf(SD_LT_SEPARATOR);
            if (nPos != -1)
                aLayoutName = aLayoutName.copy(0, nPos);

            maTabControl.SetCurPageId(nSelectedPage+1);

            if (maTabControl.GetPageText(nSelectedPage+1) != aLayoutName)
            {
                maTabControl.SetPageText(nSelectedPage+1, aLayoutName);
            }

            if( mePageKind == PK_HANDOUT )
            {
                // set pages for all available handout presentation objects
                sd::ShapeList& rShapeList = pMaster->GetPresentationShapeList();
                SdrObject* pObj = 0;

                while( (pObj = rShapeList.getNextShape(pObj)) != 0 )
                {
                    if( pMaster->GetPresObjKind(pObj) == PRESOBJ_HANDOUT )
                    {
                        // #i105146# We want no content to be displayed for PK_HANDOUT,
                        // so just never set a page as content
                        static_cast<SdrPageObj*>(pObj)->SetReferencedPage(0);
                    }
                }
            }
        }

        Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
        Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( Rectangle( Point(0,0), aVisSizePixel) );
        VisAreaChanged(aVisAreaWin);
        mpDrawView->VisAreaChanged(GetActiveWindow());

        // Damit der Navigator (und das Effekte-Window) das mitbekommt (/-men)
        SfxBindings& rBindings = GetViewFrame()->GetBindings();
        rBindings.Invalidate(SID_NAVIGATOR_PAGENAME, sal_True, sal_False);
        rBindings.Invalidate(SID_STATUS_PAGE, sal_True, sal_False);
        rBindings.Invalidate(SID_DELETE_MASTER_PAGE, sal_True, sal_False);
        rBindings.Invalidate(SID_DELETE_PAGE, sal_True, sal_False);
        rBindings.Invalidate(SID_ASSIGN_LAYOUT,sal_True,sal_False);
        rBindings.Invalidate(SID_INSERTPAGE,sal_True,sal_False);
        UpdatePreview( mpActualPage );

        mpDrawView->AdjustMarkHdl();
    }

    return (bOK);
}


/*************************************************************************
|*
|* Pruefen, ob ein Seitenwechsel erlaubt ist
|*
\************************************************************************/

sal_Bool DrawViewShell::IsSwitchPageAllowed() const
{
    bool bOK = true;

    FmFormShell* pFormShell = GetViewShellBase().GetFormShellManager()->GetFormShell();
    if (pFormShell!=NULL && !pFormShell->PrepareClose (sal_False))
        bOK = false;

    return bOK;
}

/*************************************************************************
|*
|* neue aktuelle Seite auswaehlen, falls sich die Seitenfolge geaendert
|* hat (z. B. durch Undo)
|*
\************************************************************************/

void DrawViewShell::ResetActualLayer()
{
    LayerTabBar* pLayerBar = GetLayerTabControl();
    if (pLayerBar != NULL)
    {
        // remember old layer cound and current layer id
        // this is needed when one layer is renamed to
        // restore current layer
        sal_uInt16 nOldLayerCnt = pLayerBar->GetPageCount();
        sal_uInt16 nOldLayerId = pLayerBar->GetCurPageId();

        /*************************************************************
            * Update fuer LayerTab
            *************************************************************/
        pLayerBar->Clear();

        String aName;
        String aActiveLayer = mpDrawView->GetActiveLayer();
        String aBackgroundLayer( SdResId(STR_LAYER_BCKGRND) );
        String aBackgroundObjLayer( SdResId(STR_LAYER_BCKGRNDOBJ) );
        String aLayoutLayer( SdResId(STR_LAYER_LAYOUT) );
        String aControlsLayer( SdResId(STR_LAYER_CONTROLS) );
        String aMeasureLinesLayer( SdResId(STR_LAYER_MEASURELINES) );
        sal_uInt16 nActiveLayer = SDRLAYER_NOTFOUND;
        SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
        sal_uInt16 nLayerCnt = rLayerAdmin.GetLayerCount();

        for ( sal_uInt16 nLayer = 0; nLayer < nLayerCnt; nLayer++ )
        {
            aName = rLayerAdmin.GetLayer(nLayer)->GetName();

            if ( aName == aActiveLayer )
            {
                nActiveLayer = nLayer;
            }

            if ( aName != aBackgroundLayer )
            {
                if (meEditMode == EM_MASTERPAGE)
                {
                    // Layer der Page nicht auf MasterPage anzeigen
                    if (aName != aLayoutLayer   &&
                        aName != aControlsLayer &&
                        aName != aMeasureLinesLayer)
                    {
                        pLayerBar->InsertPage(nLayer+1, aName);

                        TabBarPageBits nBits = 0;
                        SdrPageView* pPV = mpDrawView->GetSdrPageView();

                        if (pPV && !pPV->IsLayerVisible(aName))
                        {
                            // Unsichtbare Layer werden anders dargestellt
                            nBits = TPB_SPECIAL;
                        }

                        pLayerBar->SetPageBits(nLayer+1, nBits);
                    }
                }
                else
                {
                    // Layer der MasterPage nicht auf Page anzeigen
                    if ( aName != aBackgroundObjLayer )
                    {
                        pLayerBar->InsertPage(nLayer+1, aName);

                        TabBarPageBits nBits = 0;

                        if (!mpDrawView->GetSdrPageView()->IsLayerVisible(aName))
                        {
                            // Unsichtbare Layer werden anders dargestellt
                            nBits = TPB_SPECIAL;
                        }

                        pLayerBar->SetPageBits(nLayer+1, nBits);
                    }
                }
            }
        }

        if ( nActiveLayer == SDRLAYER_NOTFOUND )
        {
            if( nOldLayerCnt == pLayerBar->GetPageCount() )
            {
                nActiveLayer = nOldLayerId - 1;
            }
            else
            {
                nActiveLayer = ( meEditMode == EM_MASTERPAGE ) ? 2 : 0;
            }

            mpDrawView->SetActiveLayer( pLayerBar->GetPageText(nActiveLayer + 1) );
        }

        pLayerBar->SetCurPageId(nActiveLayer + 1);
        GetViewFrame()->GetBindings().Invalidate( SID_MODIFYLAYER );
        GetViewFrame()->GetBindings().Invalidate( SID_DELETE_LAYER );
    }
}

/*************************************************************************
|*
|* Verzoegertes Close ausfuehren
|*
\************************************************************************/

IMPL_LINK( DrawViewShell, CloseHdl, Timer*, pTimer )
{
    pTimer->Stop();
    GetViewFrame()->GetBindings().Execute( SID_CLOSEWIN );
    return 0L;
}

/*************************************************************************
|*
|* AcceptDrop
|*
\************************************************************************/

sal_Int8 DrawViewShell::AcceptDrop (
    const AcceptDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer )
{
    if( nPage != SDRPAGE_NOTFOUND )
        nPage = GetDoc()->GetSdPage( nPage, mePageKind )->GetPageNum();

    if( SlideShow::IsRunning( GetViewShellBase() ) )
        return DND_ACTION_NONE;

    return mpDrawView->AcceptDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer );
}

/*************************************************************************
|*
|* ExecuteDrop
|*
\************************************************************************/

sal_Int8 DrawViewShell::ExecuteDrop (
    const ExecuteDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    sal_uInt16 nLayer)
{
    if( nPage != SDRPAGE_NOTFOUND )
        nPage = GetDoc()->GetSdPage( nPage, mePageKind )->GetPageNum();

    if( SlideShow::IsRunning( GetViewShellBase() ) )
        return DND_ACTION_NONE;

    Broadcast(ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START));
    sal_Int8 nResult (mpDrawView->ExecuteDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer ));
    Broadcast(ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END));

    return nResult;
}

} // end of namespace sd

#ifdef _MSC_VER
#if (_MSC_VER < 1400)
#pragma optimize ( "", on )
#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
