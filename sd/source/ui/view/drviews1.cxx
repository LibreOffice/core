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

#include <DrawViewShell.hxx>
#include <ViewShellImplementation.hxx>

#include <DrawController.hxx>
#include <com/sun/star/embed/EmbedStates.hpp>

#include <comphelper/anytostring.hxx>
#include <comphelper/scopeguard.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

#include <svx/svxids.hrc>
#include <svx/svdpagv.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <svx/svdoole2.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/scrbar.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdopage.hxx>
#include <sot/storage.hxx>
#include <svx/fmshell.hxx>
#include <svx/globl3d.hxx>
#include <svx/fmglob.hxx>
#include <editeng/outliner.hxx>
#include <svx/dialogs.hrc>
#include <tools/diagnose_ex.h>

#include <view/viewoverlaymanager.hxx>

#include <strings.hrc>
#include <app.hrc>

#include <sdmod.hxx>
#include <fupoor.hxx>
#include <sdresid.hxx>
#include <unokywds.hxx>
#include <fusel.hxx>
#include <sdpage.hxx>
#include <FrameView.hxx>
#include <stlpool.hxx>
#include <Window.hxx>
#include <drawview.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <Ruler.hxx>
#include <Client.hxx>
#include <slideshow.hxx>
#include <optsitem.hxx>
#include <fusearch.hxx>
#include <Outliner.hxx>
#include <AnimationChildWindow.hxx>
#include <SdUnoDrawView.hxx>
#include <ToolBarManager.hxx>
#include <FormShellManager.hxx>
#include <ViewShellBase.hxx>
#include <LayerTabBar.hxx>
#include <ViewShellManager.hxx>
#include <ViewShellHint.hxx>

#include <sfx2/request.hxx>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>

using namespace com::sun::star;

namespace sd {

void DrawViewShell::Activate(bool bIsMDIActivate)
{
    ViewShell::Activate(bIsMDIActivate);

    // When the mode is switched to normal the main view shell grabs focus.
    // This is done for getting cut/copy/paste commands on slides in the left
    // pane (slide sorter view shell) to work properly.
    SfxShell* pTopViewShell = this->GetViewShellBase().GetViewShellManager()->GetTopViewShell();
    if (pTopViewShell && pTopViewShell == this)
    {
        this->GetActiveWindow()->GrabFocus();
    }
}

void DrawViewShell::UIActivating( SfxInPlaceClient* pCli )
{
    ViewShell::UIActivating(pCli);

    // Disable own controls
    maTabControl->Disable();
    if (GetLayerTabControl() != nullptr)
        GetLayerTabControl()->Disable();
}

void DrawViewShell::UIDeactivated( SfxInPlaceClient* pCli )
{
    // Enable own controls
    maTabControl->Enable();
    if (GetLayerTabControl() != nullptr)
        GetLayerTabControl()->Enable();

    ViewShell::UIDeactivated(pCli);
}

void DrawViewShell::Deactivate(bool bIsMDIActivate)
{
    // Temporarily disable context broadcasting while the Deactivate()
    // call is forwarded to our base class.
    const bool bIsContextBroadcasterEnabled (SfxShell::SetContextBroadcasterEnabled(false));

    ViewShell::Deactivate(bIsMDIActivate);

    SfxShell::SetContextBroadcasterEnabled(bIsContextBroadcasterEnabled);
}

namespace
{
    class LockUI
    {
    private:
        void Lock(bool bLock);
        SfxViewFrame *mpFrame;
    public:
        explicit LockUI(SfxViewFrame *pFrame) : mpFrame(pFrame) { Lock(true); }
        ~LockUI() { Lock(false); }

    };

    void LockUI::Lock(bool bLock)
    {
        if (!mpFrame)
            return;
        mpFrame->Enable( !bLock );
    }
}

/**
 * Called, if state of selection of view is changed
 */

void DrawViewShell::SelectionHasChanged()
{
    Invalidate();

    //Update3DWindow(); // 3D-Controller
    SfxBoolItem aItem( SID_3D_STATE, true );
    GetViewFrame()->GetDispatcher()->ExecuteList(
        SID_3D_STATE, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD, { &aItem });

    SdrOle2Obj* pOleObj = nullptr;

    if ( mpDrawView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetMarkedSdrObj();

            SdrInventor nInv        = pObj->GetObjInventor();
            sal_uInt16  nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv == SdrInventor::Default && nSdrObjKind == OBJ_OLE2)
            {
                pOleObj = static_cast<SdrOle2Obj*>(pObj);
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
            // as appropriate take ole-objects into account and deactivate

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
                const uno::Reference < embed::XEmbeddedObject >& xObj = pOleObj->GetObjRef();
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
                const uno::Reference < embed::XEmbeddedObject >& xObj = pOleObj->GetObjRef();
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
    catch( css::uno::Exception& )
    {
        SAL_WARN( "sd", "sd::DrawViewShell::SelectionHasChanged(), exception caught: "
                << exceptionToString( cppu::getCaughtException() ) );
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

    mpDrawView->UpdateSelectionClipboard( false );

    GetViewShellBase().GetDrawController().FireSelectionChangeListener();
}

namespace {

void collectUIInformation(const OUString& aZoom)
{
    EventDescription aDescription;
    aDescription.aID = "impress_win";
    aDescription.aParameters = {{"ZOOM", aZoom}};
    aDescription.aAction = "SET";
    aDescription.aKeyWord = "ImpressWindowUIObject";
    aDescription.aParent = "MainWindow";

    UITestLogger::getInstance().logEvent(aDescription);
}

}

/**
 * set zoom factor
 */
void DrawViewShell::SetZoom( long nZoom )
{
    // Make sure that the zoom factor will not be recalculated on
    // following window resizings.
    mbZoomOnPage = false;
    ViewShell::SetZoom( nZoom );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
    mpViewOverlayManager->onZoomChanged();
    collectUIInformation(OUString::number(nZoom));
}

/**
 * Set zoom rectangle for active window
 */

void DrawViewShell::SetZoomRect( const ::tools::Rectangle& rZoomRect )
{
    ViewShell::SetZoomRect( rZoomRect );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER );
    mpViewOverlayManager->onZoomChanged();
}

/**
 * PrepareClose, as appropriate end text input, so other viewshells
 * discover a refreshed text object.
 */

bool DrawViewShell::PrepareClose( bool bUI )
{
    if ( !ViewShell::PrepareClose(bUI) )
        return false;

    if( HasCurrentFunction() )
    {
        sal_uInt16 nID = GetCurrentFunction()->GetSlotID();
        if (nID == SID_TEXTEDIT || nID == SID_ATTR_CHAR)
        {
            mpDrawView->SdrEndTextEdit();
        }
    }

    return true;
}


/**
 * Set status (enabled/disabled) of menu SfxSlots
 */

void DrawViewShell::ChangeEditMode(EditMode eEMode, bool bIsLayerModeActive)
{
    if (meEditMode == eEMode && mbIsLayerModeActive == bIsLayerModeActive)
        return;

    ViewShellManager::UpdateLock aLock (GetViewShellBase().GetViewShellManager());

    sal_uInt16 nActualPageId = maTabControl->GetPageId(0);

    if (mePageKind == PageKind::Handout)
    {
        // at handouts only allow MasterPage
        eEMode = EditMode::MasterPage;
    }

    GetViewShellBase().GetDrawController().FireChangeEditMode (eEMode == EditMode::MasterPage);
    GetViewShellBase().GetDrawController().FireChangeLayerMode (bIsLayerModeActive);

    if ( mpDrawView->IsTextEdit() )
    {
        mpDrawView->SdrEndTextEdit();
    }

    LayerTabBar* pLayerBar = GetLayerTabControl();
    if (pLayerBar != nullptr)
        pLayerBar->EndEditMode();
    maTabControl->EndEditMode();

    GetViewShellBase().GetDrawController().BroadcastContextChange();

    meEditMode = eEMode;

    if(pLayerBar)
    {
        // #i87182# only switch activation mode of LayerTabBar when there is one,
        // else it will not get initialized with the current set of Layers as needed
        mbIsLayerModeActive = bIsLayerModeActive;
    }

    // Determine whether to show the master view toolbar.  The master
    // page mode has to be active and the shell must not be a handout
    // view.
    bool bShowMasterViewToolbar (meEditMode == EditMode::MasterPage
         && GetShellType() != ViewShell::ST_HANDOUT);
    bool bShowPresentationToolbar (meEditMode != EditMode::MasterPage
         && GetShellType() != ViewShell::ST_HANDOUT
         && GetShellType() != ViewShell::ST_DRAW);

    // If the master view toolbar is not shown we hide it before
    // switching the edit mode.
    if (::sd::ViewShell::mpImpl->mbIsInitialized
        && IsMainViewShell())
    {
        if ( !bShowMasterViewToolbar )
            GetViewShellBase().GetToolBarManager()->ResetToolBars(ToolBarManager::ToolBarGroup::MasterMode);
        if ( !bShowPresentationToolbar )
            GetViewShellBase().GetToolBarManager()->ResetToolBars(ToolBarManager::ToolBarGroup::CommonTask);
    }

    ConfigureAppBackgroundColor();

    if (meEditMode == EditMode::Page)
    {
        /******************************************************************
        * PAGEMODE
        ******************************************************************/

        maTabControl->Clear();

        SdPage* pPage;
        sal_uInt16 nPageCnt = GetDoc()->GetSdPageCount(mePageKind);

        for (sal_uInt16 i = 0; i < nPageCnt; i++)
        {
            pPage = GetDoc()->GetSdPage(i, mePageKind);
            OUString aPageName = pPage->GetName();
            maTabControl->InsertPage(pPage->getPageId(), aPageName);

            if ( pPage->IsSelected() )
            {
                nActualPageId = pPage->getPageId();
            }
        }

        maTabControl->SetCurPageId(nActualPageId);

        SwitchPage(maTabControl->GetPagePos(nActualPageId));

        //tdf#102343 re-enable common undo on switch back from master mode
        mpDrawView->GetModel()->SetDisableTextEditUsesCommonUndoManager(false);
    }
    else
    {
        /******************************************************************
        * MASTERPAGE
        ******************************************************************/
        GetViewFrame()->SetChildWindow(
            AnimationChildWindow::GetChildWindowId(), false );

        if (!mpActualPage)
        {
            // as long as there is no mpActualPage, take first
            mpActualPage = GetDoc()->GetSdPage(0, mePageKind);
        }

        maTabControl->Clear();
        sal_uInt16 nActualMasterPageId = maTabControl->GetPageId(0);
        sal_uInt16 nMasterPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);

        for (sal_uInt16 i = 0; i < nMasterPageCnt; i++)
        {
            SdPage* pMaster = GetDoc()->GetMasterSdPage(i, mePageKind);
            OUString aLayoutName = pMaster->GetLayoutName();
            sal_Int32 nPos = aLayoutName.indexOf(SD_LT_SEPARATOR);
            if (nPos != -1)
                aLayoutName = aLayoutName.copy(0, nPos);

            maTabControl->InsertPage(pMaster->getPageId(), aLayoutName);

            if (&(mpActualPage->TRG_GetMasterPage()) == pMaster)
            {
                nActualMasterPageId = pMaster->getPageId();
            }
        }

        maTabControl->SetCurPageId(nActualMasterPageId);
        SwitchPage(maTabControl->GetPagePos(nActualMasterPageId));

        //tdf#102343 changing attributes of textboxes in master typically
        //changes the stylesheet they are linked to, so if the common
        //undo manager is in use, those stylesheet changes are thrown
        //away at present
        mpDrawView->GetModel()->SetDisableTextEditUsesCommonUndoManager(true);
    }

    // If the master view toolbar is to be shown we turn it on after the
    // edit mode has been changed.
    if (::sd::ViewShell::mpImpl->mbIsInitialized
        && IsMainViewShell())
    {
        if (bShowMasterViewToolbar)
            GetViewShellBase().GetToolBarManager()->SetToolBar(
                ToolBarManager::ToolBarGroup::MasterMode,
                ToolBarManager::msMasterViewToolBar);
        if (bShowPresentationToolbar)
            GetViewShellBase().GetToolBarManager()->SetToolBar(
                ToolBarManager::ToolBarGroup::CommonTask,
                ToolBarManager::msCommonTaskToolBar);
    }

    if ( ! mbIsLayerModeActive)
    {
        maTabControl->Show();
        // Set the tab control only for draw pages.  For master page
        // this has been done already above.
        if (meEditMode == EditMode::Page)
            maTabControl->SetCurPageId (nActualPageId);
    }

    ResetActualLayer();

    Invalidate( SID_PAGEMODE );
    Invalidate( SID_LAYERMODE );
    Invalidate( SID_MASTERPAGE );
    Invalidate( SID_DELETE_MASTER_PAGE );
    Invalidate( SID_DELETE_PAGE );
    Invalidate( SID_SLIDE_MASTER_MODE );
    Invalidate( SID_NOTES_MASTER_MODE );
    Invalidate( SID_HANDOUT_MASTER_MODE );
    InvalidateWindows();

    SetContextName(GetSidebarContextName());

}

/**
 * Generate horizontal ruler
 */

VclPtr<SvxRuler> DrawViewShell::CreateHRuler (::sd::Window* pWin)
{
    VclPtr<Ruler> pRuler;
    WinBits  aWBits;
    SvxRulerSupportFlags nFlags = SvxRulerSupportFlags::OBJECT;

    aWBits  = WB_HSCROLL | WB_3DLOOK | WB_BORDER | WB_EXTRAFIELD;
    nFlags |= SvxRulerSupportFlags::SET_NULLOFFSET |
              SvxRulerSupportFlags::TABS |
              SvxRulerSupportFlags::PARAGRAPH_MARGINS; // new

    pRuler = VclPtr<Ruler>::Create(*this, GetParentWindow(), pWin, nFlags,
        GetViewFrame()->GetBindings(), aWBits);

    // Metric ...
    sal_uInt16 nMetric = static_cast<sal_uInt16>(GetDoc()->GetUIUnit());

    if( nMetric == 0xffff )
        nMetric = static_cast<sal_uInt16>(GetViewShellBase().GetViewFrame()->GetDispatcher()->GetModule()->GetFieldUnit());

    pRuler->SetUnit( FieldUnit( nMetric ) );

    // ... and also set DefTab at the ruler
    pRuler->SetDefTabDist( GetDoc()->GetDefaultTabulator() ); // new

    Fraction aUIScale(pWin->GetMapMode().GetScaleX());
    aUIScale *= GetDoc()->GetUIScale();
    pRuler->SetZoom(aUIScale);

    return pRuler;
}

/**
 * Generate vertical ruler
 */

VclPtr<SvxRuler> DrawViewShell::CreateVRuler(::sd::Window* pWin)
{
    VclPtr<SvxRuler> pRuler;
    WinBits  aWBits = WB_VSCROLL | WB_3DLOOK | WB_BORDER;
    SvxRulerSupportFlags nFlags = SvxRulerSupportFlags::OBJECT;

    pRuler = VclPtr<Ruler>::Create(*this, GetParentWindow(), pWin, nFlags,
        GetViewFrame()->GetBindings(), aWBits);

    // Metric same as HRuler, use document setting
    sal_uInt16 nMetric = static_cast<sal_uInt16>(GetDoc()->GetUIUnit());

    if( nMetric == 0xffff )
        nMetric = static_cast<sal_uInt16>(GetViewShellBase().GetViewFrame()->GetDispatcher()->GetModule()->GetFieldUnit());

    pRuler->SetUnit( FieldUnit( nMetric ) );

    Fraction aUIScale(pWin->GetMapMode().GetScaleY());
    aUIScale *= GetDoc()->GetUIScale();
    pRuler->SetZoom(aUIScale);

    return pRuler;
}

/**
 * Refresh horizontal ruler
 */

void DrawViewShell::UpdateHRuler()
{
    Invalidate( SID_ATTR_LONG_LRSPACE );
    Invalidate( SID_RULER_PAGE_POS );
    Invalidate( SID_RULER_OBJECT );
    Invalidate( SID_RULER_TEXT_RIGHT_TO_LEFT );

    if (mpHorizontalRuler.get() != nullptr)
        mpHorizontalRuler->ForceUpdate();
}

/**
 * Refresh vertical ruler
 */

void DrawViewShell::UpdateVRuler()
{
    Invalidate( SID_ATTR_LONG_LRSPACE );
    Invalidate( SID_RULER_PAGE_POS );
    Invalidate( SID_RULER_OBJECT );

    if (mpVerticalRuler.get() != nullptr)
        mpVerticalRuler->ForceUpdate();
}

/**
 * Refresh TabControl on splitter change
 */

IMPL_LINK( DrawViewShell, TabSplitHdl, TabBar *, pTab, void )
{
    const long int nMax = maViewSize.Width() - maScrBarWH.Width()
        - maTabControl->GetPosPixel().X() ;

    Size aTabSize = maTabControl->GetSizePixel();
    aTabSize.setWidth( std::min(pTab->GetSplitSize(), static_cast<long>(nMax-1)) );

    maTabControl->SetSizePixel(aTabSize);

    if(GetLayerTabControl()) // #i87182#
    {
        GetLayerTabControl()->SetSizePixel(aTabSize);
    }

    Point aPos = maTabControl->GetPosPixel();
    aPos.AdjustX(aTabSize.Width() );

    Size aScrSize(nMax - aTabSize.Width(), maScrBarWH.Height());
    mpHorizontalScrollBar->SetPosSizePixel(aPos, aScrSize);
}

/// inherited from sd::ViewShell
SdPage* DrawViewShell::getCurrentPage() const
{
    const sal_uInt16 nPageCount = (meEditMode == EditMode::Page)?
                                    GetDoc()->GetSdPageCount(mePageKind):
                                    GetDoc()->GetMasterSdPageCount(mePageKind);

    sal_uInt16 nCurrentPage = maTabControl->GetCurPagePos();
    DBG_ASSERT((nCurrentPage<nPageCount), "sd::DrawViewShell::getCurrentPage(), illegal page index!");
    if (nCurrentPage >= nPageCount)
        nCurrentPage = 0; // play safe here

    if (meEditMode == EditMode::Page)
    {
        return GetDoc()->GetSdPage(nCurrentPage, mePageKind);
    }
    else // EditMode::MasterPage
    {
        return GetDoc()->GetMasterSdPage(nCurrentPage, mePageKind);
    }
}

/**
 * Select new refreshed page, in case of a page order change (eg. by undo)
 */

void DrawViewShell::ResetActualPage()
{
    if (!GetDoc())
        return;

    sal_uInt16 nCurrentPageId = maTabControl->GetCurPageId();
    sal_uInt16 nCurrentPageNum = maTabControl->GetPagePos(nCurrentPageId);
    sal_uInt16 nPageCount   = (meEditMode == EditMode::Page)?GetDoc()->GetSdPageCount(mePageKind):GetDoc()->GetMasterSdPageCount(mePageKind);

    if (meEditMode == EditMode::Page)
    {

        // Update for TabControl
        maTabControl->Clear();

        SdPage* pPage = nullptr;

        for (sal_uInt16 i = 0; i < nPageCount; i++)
        {
            pPage = GetDoc()->GetSdPage(i, mePageKind);
            OUString aPageName = pPage->GetName();
            maTabControl->InsertPage(pPage->getPageId(), aPageName);

            if (nCurrentPageId == pPage->getPageId())
            {
                nCurrentPageNum = i;
                GetDoc()->SetSelected(pPage, true);
            }
            else
                GetDoc()->SetSelected(pPage, false);
        }

        maTabControl->SetCurPageId(maTabControl->GetPageId(nCurrentPageNum));
    }
    else // EditMode::MasterPage
    {
        maTabControl->Clear();

        sal_uInt16 nMasterPageCnt = GetDoc()->GetMasterSdPageCount(mePageKind);
        for (sal_uInt16 i = 0; i < nMasterPageCnt; i++)
        {
            SdPage* pMaster = GetDoc()->GetMasterSdPage(i, mePageKind);
            OUString aLayoutName = pMaster->GetLayoutName();
            sal_Int32 nPos = aLayoutName.indexOf(SD_LT_SEPARATOR);
            if (nPos != -1)
                aLayoutName = aLayoutName.copy(0, nPos);
            maTabControl->InsertPage(pMaster->getPageId(), aLayoutName);

            if (pMaster->getPageId() == nCurrentPageId)
                nCurrentPageNum = i;
        }

        maTabControl->SetCurPageId(maTabControl->GetPageId(nCurrentPageNum));
        SwitchPage(nCurrentPageNum);
    }

    GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHPAGE,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
}

/**
 * Apply "Verb" on OLE-object.
 */

ErrCode DrawViewShell::DoVerb(long nVerb)
{
    if ( mpDrawView->AreObjectsMarked() )
    {
        const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

        if (rMarkList.GetMarkCount() == 1)
        {
            SdrMark* pMark = rMarkList.GetMark(0);
            SdrObject* pObj = pMark->GetMarkedSdrObj();

            SdrInventor nInv        = pObj->GetObjInventor();
            sal_uInt16  nSdrObjKind = pObj->GetObjIdentifier();

            if (nInv == SdrInventor::Default && nSdrObjKind == OBJ_OLE2)
            {
                ActivateObject( static_cast<SdrOle2Obj*>(pObj), nVerb);
            }
        }
    }

    return ERRCODE_NONE;
}

/**
 * Activate OLE-object
 */

bool DrawViewShell::ActivateObject(SdrOle2Obj* pObj, long nVerb)
{
    bool bActivated = false;

    if ( !GetDocSh()->IsUIActive() )
    {
        ToolBarManager::UpdateLock aLock (GetViewShellBase().GetToolBarManager());

        bActivated = ViewShell::ActivateObject(pObj, nVerb);
    }

    return bActivated;
}

/**
 * Switch to desired page.
 * nSelectPage refers to the current EditMode
 */
bool DrawViewShell::SwitchPage(sal_uInt16 nSelectedPage)
{
    /** Under some circumstances there are nested calls to SwitchPage() and
        may crash the application (activation of form controls when the
        shell of the edit view is not on top of the shell stack, see issue
        83888 for details.)  Therefore the nested calls are ignored (they
        would jump to the wrong page anyway.)
    */

    if (mbIsInSwitchPage)
        return false;
    mbIsInSwitchPage = true;
    comphelper::ScopeGuard aGuard(
            [this] () { this->mbIsInSwitchPage = false; } );

    if (GetActiveWindow()->IsInPaint())
    {
        // Switching the current page while a Paint is being executed is
        // dangerous.  So, post it for later execution and return.
        maAsynchronousSwitchPageCall.Post(
            [this, nSelectedPage] () { this->SwitchPage(nSelectedPage); } );
        return false;
    }

    bool bOK = false;

    // With the current implementation of FuSlideShow there is a problem
    // when it displays the show in a window: when the show is stopped it
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
        sal_uInt16 nPageCount = (meEditMode == EditMode::Page)
            ? GetDoc()->GetSdPageCount(mePageKind)
            : GetDoc()->GetMasterSdPageCount(mePageKind);
        if (nSelectedPage >= nPageCount)
            nSelectedPage = nPageCount-1;
    }

    if (IsSwitchPageAllowed())
    {
        ModifyGuard aGuard2( GetDoc() );

        bOK = true;

        if (mpActualPage)
        {
            SdPage* pNewPage = nullptr;

            if (meEditMode == EditMode::MasterPage)
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
                        && sPageText == maTabControl->GetPageText(maTabControl->GetPageId(nSelectedPage)))
                    {
                        // this slide is already visible
                        return true;
                    }
                }
            }
            else
            {
                OSL_ASSERT(mpFrameView!=nullptr);
                mpFrameView->SetSelectedPage(nSelectedPage);

                if (GetDoc()->GetSdPageCount(mePageKind) > nSelectedPage)
                    pNewPage = GetDoc()->GetSdPage(nSelectedPage, mePageKind);

                if (mpActualPage == pNewPage)
                {
                    SdrPageView* pPV = mpDrawView->GetSdrPageView();

                    SdPage* pCurrentPage = pPV ? dynamic_cast<SdPage*>(pPV->GetPage()) : nullptr;
                    if (pCurrentPage
                        && pNewPage == pCurrentPage
                        && maTabControl->GetPageText(maTabControl->GetPageId(nSelectedPage)) == pNewPage->GetName())
                    {
                        // this slide is already visible
                        return true;
                    }
                }
            }
        }

        mpDrawView->SdrEndTextEdit();

        mpActualPage = nullptr;

        if (meEditMode == EditMode::Page)
        {
            mpActualPage = GetDoc()->GetSdPage(nSelectedPage, mePageKind);
        }
        else
        {
            SdPage* pMaster = GetDoc()->GetMasterSdPage(nSelectedPage, mePageKind);

            // does the selected page fit to the masterpage?
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
                // take the first page, that fits to the masterpage
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
            // deselect all pages
            GetDoc()->SetSelected( GetDoc()->GetSdPage(i, mePageKind), false);
        }

        if (!mpActualPage)
        {
            // as far as there is no mpActualPage, take the first
            mpActualPage = GetDoc()->GetSdPage(0, mePageKind);
        }

        // also select this page (mpActualPage always points at a drawing page,
        // never at a masterpage)
        GetDoc()->SetSelected(mpActualPage, true);

        if (comphelper::LibreOfficeKit::isActive())
        {
            // notify LibreOfficeKit about changed page
            OString aPayload = OString::number(nSelectedPage);
            if (SfxViewShell* pViewShell = GetViewShell())
                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_SET_PART, aPayload.getStr());
        }

        rtl::Reference< sd::SlideShow > xSlideshow( SlideShow::GetSlideShow( GetDoc() ) );
        if( !xSlideshow.is() || !xSlideshow->isRunning() || ( xSlideshow->getAnimationMode() != ANIMATIONMODE_SHOW ) )
        {
            // tighten VisArea, to possibly deactivate objects
            // !!! only if we are not in presentation mode (#96279) !!!
            OSL_ASSERT (GetViewShell()!=nullptr);
            GetViewShell()->DisconnectAllClients();
            VisAreaChanged(::tools::Rectangle(Point(), Size(1, 1)));
        }

        if (meEditMode == EditMode::Page)
        {
            /**********************************************************************
            * PAGEMODE
            **********************************************************************/
            GetDoc()->SetSelected(mpActualPage, true);

            SdrPageView* pPageView = mpDrawView->GetSdrPageView();

            if (pPageView)
            {
                mpFrameView->SetVisibleLayers( pPageView->GetVisibleLayers() );
                mpFrameView->SetPrintableLayers( pPageView->GetPrintableLayers() );
                mpFrameView->SetLockedLayers( pPageView->GetLockedLayers() );

                if (mePageKind == PageKind::Notes)
                {
                    mpFrameView->SetNotesHelpLines( pPageView->GetHelpLines() );
                }
                else if (mePageKind == PageKind::Handout)
                {
                    mpFrameView->SetHandoutHelpLines( pPageView->GetHelpLines() );
                }
                else
                {
                    mpFrameView->SetStandardHelpLines( pPageView->GetHelpLines() );
                }
            }

            mpDrawView->HideSdrPage();
            maTabControl->SetCurPageId(maTabControl->GetPageId(nSelectedPage));
            mpDrawView->ShowSdrPage(mpActualPage);
            GetViewShellBase().GetDrawController().FireSwitchCurrentPage(mpActualPage);

            SdrPageView* pNewPageView = mpDrawView->GetSdrPageView();

            if (pNewPageView)
            {
                pNewPageView->SetVisibleLayers( mpFrameView->GetVisibleLayers() );
                pNewPageView->SetPrintableLayers( mpFrameView->GetPrintableLayers() );
                pNewPageView->SetLockedLayers( mpFrameView->GetLockedLayers() );

                if (mePageKind == PageKind::Notes)
                {
                    pNewPageView->SetHelpLines( mpFrameView->GetNotesHelpLines() );
                }
                else if (mePageKind == PageKind::Handout)
                {
                    pNewPageView->SetHelpLines( mpFrameView->GetHandoutHelpLines() );
                }
                else
                {
                    pNewPageView->SetHelpLines( mpFrameView->GetStandardHelpLines() );
                }
            }

            OUString aPageName = mpActualPage->GetName();

            if (maTabControl->GetPageText(maTabControl->GetPageId(nSelectedPage)) != aPageName)
            {
                maTabControl->SetPageText(maTabControl->GetPageId(nSelectedPage), aPageName);
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

                if (mePageKind == PageKind::Notes)
                {
                    mpFrameView->SetNotesHelpLines( pPageView->GetHelpLines() );
                }
                else if (mePageKind == PageKind::Handout)
                {
                    mpFrameView->SetHandoutHelpLines( pPageView->GetHelpLines() );
                }
                else
                {
                    mpFrameView->SetStandardHelpLines( pPageView->GetHelpLines() );
                }
            }

            mpDrawView->HideSdrPage();
            maTabControl->SetCurPageId(maTabControl->GetPageId(nSelectedPage));

            SdPage* pMaster = GetDoc()->GetMasterSdPage(nSelectedPage, mePageKind);

            if( !pMaster )              // if this page should not exist
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

                if (mePageKind == PageKind::Notes)
                {
                    pNewPageView->SetHelpLines( mpFrameView->GetNotesHelpLines() );
                }
                else if (mePageKind == PageKind::Handout)
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

            if (maTabControl->GetPageText(maTabControl->GetPageId(nSelectedPage)) != aLayoutName)
            {
                maTabControl->SetPageText(maTabControl->GetPageId(nSelectedPage), aLayoutName);
            }

            if( mePageKind == PageKind::Handout )
            {
                // set pages for all available handout presentation objects
                sd::ShapeList& rShapeList = pMaster->GetPresentationShapeList();
                SdrObject* pObj = nullptr;
                rShapeList.seekShape(0);

                while( (pObj = rShapeList.getNextShape()) )
                {
                    if( pMaster->GetPresObjKind(pObj) == PRESOBJ_HANDOUT )
                    {
                        // #i105146# We want no content to be displayed for PageKind::Handout,
                        // so just never set a page as content
                        static_cast<SdrPageObj*>(pObj)->SetReferencedPage(nullptr);
                    }
                }
            }
        }

        Size aVisSizePixel = GetActiveWindow()->GetOutputSizePixel();
        ::tools::Rectangle aVisAreaWin = GetActiveWindow()->PixelToLogic( ::tools::Rectangle( Point(0,0), aVisSizePixel) );
        VisAreaChanged(aVisAreaWin);
        mpDrawView->VisAreaChanged(GetActiveWindow());

        // so navigator (and effect window) notice that
        SfxBindings& rBindings = GetViewFrame()->GetBindings();
        rBindings.Invalidate(SID_NAVIGATOR_STATE, true);
        rBindings.Invalidate(SID_NAVIGATOR_PAGENAME, true);
        rBindings.Invalidate(SID_STATUS_PAGE, true);
        rBindings.Invalidate(SID_DELETE_MASTER_PAGE, true);
        rBindings.Invalidate(SID_DELETE_PAGE, true);
        rBindings.Invalidate(SID_ASSIGN_LAYOUT, true);
        rBindings.Invalidate(SID_INSERTPAGE, true);
        UpdatePreview( mpActualPage );

        mpDrawView->AdjustMarkHdl();
    }

    return bOK;
}

/**
 * Check if page change is allowed
 */

bool DrawViewShell::IsSwitchPageAllowed() const
{
    bool bOK = true;

    FmFormShell* pFormShell = GetViewShellBase().GetFormShellManager()->GetFormShell();
    if (pFormShell != nullptr && !pFormShell->PrepareClose(false))
        bOK = false;

    return bOK;
}

/**
 * Select new refreshed page, in case of a page order change (eg. by undo)
 */

void DrawViewShell::ResetActualLayer()
{
    LayerTabBar* pLayerBar = GetLayerTabControl();
    if (pLayerBar == nullptr)
        return;

    // remember old tab count and current tab id
    // this is needed when one layer is renamed to
    // restore current tab
    sal_uInt16 nOldLayerCnt = pLayerBar->GetPageCount(); // actually it is tab count
    sal_uInt16 nOldLayerPos = pLayerBar->GetCurPageId(); // actually it is a tab nId

    /**
     * Update for LayerTab
     */
    pLayerBar->Clear();

    OUString aName; // a real layer name
    OUString aActiveLayer = mpDrawView->GetActiveLayer();
    sal_uInt16 nActiveLayerPos = SDRLAYERPOS_NOTFOUND;
    SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
    sal_uInt16 nLayerCnt = rLayerAdmin.GetLayerCount();

    for ( sal_uInt16 nLayerPos = 0; nLayerPos < nLayerCnt; nLayerPos++ )
    {
        aName = rLayerAdmin.GetLayer(nLayerPos)->GetName();

        if ( aName == aActiveLayer )
        {
            nActiveLayerPos = nLayerPos;
        }

        if ( aName != sUNO_LayerName_background ) // layer "background" has never a tab
        {
            if (meEditMode == EditMode::MasterPage)
            {
                // don't show page layer onto the masterpage
                if (aName != sUNO_LayerName_layout   &&
                    aName != sUNO_LayerName_controls &&
                    aName != sUNO_LayerName_measurelines)
                {
                    TabBarPageBits nBits = TabBarPageBits::NONE;
                    SdrPageView* pPV = mpDrawView->GetSdrPageView();
                    if (pPV)
                    {
                        if (!pPV->IsLayerVisible(aName))
                        {
                            nBits |= TabBarPageBits::Blue;
                        }
                        if (pPV->IsLayerLocked(aName))
                        {
                            nBits |= TabBarPageBits::Italic;
                        }
                        if (!pPV->IsLayerPrintable(aName))
                        {
                            nBits |= TabBarPageBits::Underline;
                        }
                    }

                    pLayerBar->InsertPage(nLayerPos+1, aName, nBits); // why +1? It is a nId, not a position. Position is APPEND.
                }
            }
            else
            {
                // don't show masterpage layer onto the page
                if (aName != sUNO_LayerName_background_objects)
                {
                    TabBarPageBits nBits = TabBarPageBits::NONE;
                    if (!mpDrawView->GetSdrPageView()->IsLayerVisible(aName))
                    {
                        nBits = TabBarPageBits::Blue;
                    }
                    if (mpDrawView->GetSdrPageView()->IsLayerLocked(aName))
                    {
                        nBits |= TabBarPageBits::Italic;
                    }
                    if (!mpDrawView->GetSdrPageView()->IsLayerPrintable(aName))
                    {
                        nBits |= TabBarPageBits::Underline;
                    }

                    pLayerBar->InsertPage(nLayerPos+1, aName, nBits);// why +1?
                }
            }
        }
    }

    if ( nActiveLayerPos == SDRLAYERPOS_NOTFOUND )
    {
        if( nOldLayerCnt == pLayerBar->GetPageCount() )
        {
            nActiveLayerPos = nOldLayerPos - 1;
        }
        else
        {
            nActiveLayerPos = ( meEditMode == EditMode::MasterPage ) ? 2 : 0;
        }

        mpDrawView->SetActiveLayer( pLayerBar->GetLayerName(nActiveLayerPos + 1) );// why +1?
    }

    pLayerBar->SetCurPageId(nActiveLayerPos + 1);
    GetViewFrame()->GetBindings().Invalidate( SID_MODIFYLAYER );
    GetViewFrame()->GetBindings().Invalidate( SID_DELETE_LAYER );
}

/**
 * AcceptDrop
 */

sal_Int8 DrawViewShell::AcceptDrop (
    const AcceptDropEvent& rEvt,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    SdrLayerID nLayer )
{
    if( nPage != SDRPAGE_NOTFOUND )
        nPage = GetDoc()->GetSdPage( nPage, mePageKind )->GetPageNum();

    if( SlideShow::IsRunning( GetViewShellBase() ) )
        return DND_ACTION_NONE;

    return mpDrawView->AcceptDrop( rEvt, rTargetHelper, pTargetWindow, nPage, nLayer );
}

/**
 * ExecuteDrop
 */

sal_Int8 DrawViewShell::ExecuteDrop (
    const ExecuteDropEvent& rEvt,
    DropTargetHelper& /*rTargetHelper*/,
    ::sd::Window* pTargetWindow,
    sal_uInt16 nPage,
    SdrLayerID nLayer)
{
    if( nPage != SDRPAGE_NOTFOUND )
        nPage = GetDoc()->GetSdPage( nPage, mePageKind )->GetPageNum();

    if( SlideShow::IsRunning( GetViewShellBase() ) )
        return DND_ACTION_NONE;

    Broadcast(ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START));
    sal_Int8 nResult (mpDrawView->ExecuteDrop( rEvt, pTargetWindow, nPage, nLayer ));
    Broadcast(ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END));

    return nResult;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
