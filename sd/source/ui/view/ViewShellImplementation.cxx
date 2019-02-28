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

#include <config_features.h>

#include <ViewShellImplementation.hxx>

#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <sdresid.hxx>
#include <unokywds.hxx>
#include <strings.hrc>
#include <app.hrc>
#include <sdattr.hxx>
#include <sdabstdlg.hxx>
#include <unmodpg.hxx>
#include <Window.hxx>
#include <optsitem.hxx>
#include <DrawDocShell.hxx>
#include <DrawController.hxx>
#include <FactoryIds.hxx>
#include <slideshow.hxx>
#include <ViewShellBase.hxx>
#include <FrameView.hxx>
#include <DrawViewShell.hxx>
#include <ViewShellHint.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svl/aeitem.hxx>
#include <svx/imapdlg.hxx>
#include <basic/sbstar.hxx>
#include <basic/sberrors.hxx>
#include <xmloff/autolayout.hxx>

#include <undo/undoobjects.hxx>

#include <com/sun/star/drawing/framework/XControllerManager.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd {

ViewShell::Implementation::Implementation (ViewShell& rViewShell)
    : mbIsMainViewShell(false),
      mbIsInitialized(false),
      mbArrangeActive(false),
      mpSubShellFactory(),
      mpUpdateLockForMouse(),
      mrViewShell(rViewShell)
{
}

ViewShell::Implementation::~Implementation() COVERITY_NOEXCEPT_FALSE
{
    if ( ! mpUpdateLockForMouse.expired())
    {
        std::shared_ptr<ToolBarManagerLock> pLock(mpUpdateLockForMouse);
        if (pLock != nullptr)
        {
            // Force the ToolBarManagerLock to be released even when the
            // IsUICaptured() returns <TRUE/>.
            pLock->Release(true);
        }
    }
}

void ViewShell::Implementation::ProcessModifyPageSlot (
    SfxRequest& rRequest,
    SdPage* pCurrentPage,
    PageKind ePageKind)
{
    SdDrawDocument* pDocument = mrViewShell.GetDoc();
    SdrLayerAdmin& rLayerAdmin = pDocument->GetLayerAdmin();
    SdrLayerIDSet aVisibleLayers;
    bool bHandoutMode = false;
    SdPage* pHandoutMPage = nullptr;
    OUString aNewName;

    AutoLayout aNewAutoLayout;

    bool bBVisible;
    bool bBObjsVisible;
    const SfxItemSet* pArgs = rRequest.GetArgs();

    if (pCurrentPage != nullptr && pCurrentPage->TRG_HasMasterPage())
        aVisibleLayers = pCurrentPage->TRG_GetMasterPageVisibleLayers();
    else
        aVisibleLayers.SetAll();

    do
    {
        if (pCurrentPage == nullptr)
            break;

        if (!pArgs || pArgs->Count() == 1 || pArgs->Count() == 2 )
        {
            // First make sure that the sidebar is visible
            mrViewShell.GetDrawView()->SdrEndTextEdit();
            mrViewShell.GetDrawView()->UnmarkAll();
            mrViewShell.GetViewFrame()->ShowChildWindow(SID_SIDEBAR);
            sfx2::sidebar::Sidebar::ShowPanel(
                "SdLayoutsPanel",
                mrViewShell.GetViewFrame()->GetFrame().GetFrameInterface());
            break;
        }
        else if (pArgs->Count() == 4)
        {
            const SfxStringItem* pNewName = rRequest.GetArg<SfxStringItem>(ID_VAL_PAGENAME);
            const SfxUInt32Item* pNewAutoLayout = rRequest.GetArg<SfxUInt32Item>(ID_VAL_WHATLAYOUT);
            const SfxBoolItem* pBVisible = rRequest.GetArg<SfxBoolItem>(ID_VAL_ISPAGEBACK);
            const SfxBoolItem* pBObjsVisible = rRequest.GetArg<SfxBoolItem>(ID_VAL_ISPAGEOBJ);
            AutoLayout aLayout (static_cast<AutoLayout>(pNewAutoLayout->GetValue ()));
            if (aLayout >= AUTOLAYOUT_START
                && aLayout < AUTOLAYOUT_END)
            {
                aNewName        = pNewName->GetValue ();
                aNewAutoLayout = static_cast<AutoLayout>(pNewAutoLayout->GetValue ());
                bBVisible       = pBVisible->GetValue ();
                bBObjsVisible   = pBObjsVisible->GetValue ();
            }
            else
            {
#if HAVE_FEATURE_SCRIPTING
                StarBASIC::FatalError (ERRCODE_BASIC_BAD_PROP_VALUE);
#endif
                rRequest.Ignore ();
                break;
            }
            if (ePageKind == PageKind::Handout)
            {
                bHandoutMode = true;
                pHandoutMPage = pDocument->GetMasterSdPage(0, PageKind::Handout);
            }
        }
        else
        {
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            rRequest.Ignore ();
            break;
        }

        SdPage* pUndoPage =
            bHandoutMode ? pHandoutMPage : pCurrentPage;

        SfxUndoManager* pUndoManager = mrViewShell.GetDocSh()->GetUndoManager();
        DBG_ASSERT(pUndoManager, "No UNDO MANAGER ?!?");

        if( pUndoManager )
        {
            OUString aComment( SdResId(STR_UNDO_MODIFY_PAGE) );
            pUndoManager->EnterListAction(aComment, aComment, 0, mrViewShell.GetViewShellBase().GetViewShellId());
            pUndoManager->AddUndoAction(
                std::make_unique<ModifyPageUndoAction>(
                    pDocument, pUndoPage, aNewName, aNewAutoLayout, bBVisible, bBObjsVisible));

            // Clear the selection because the selected object may be removed as
            // a result of the assignment of the layout.
            mrViewShell.GetDrawView()->UnmarkAll();

            if (!bHandoutMode)
            {
                if (pCurrentPage->GetName() != aNewName)
                {
                    pCurrentPage->SetName(aNewName);

                    if (ePageKind == PageKind::Standard)
                    {
                        sal_uInt16 nPage = (pCurrentPage->GetPageNum()-1) / 2;
                        SdPage* pNotesPage = pDocument->GetSdPage(nPage, PageKind::Notes);
                        if (pNotesPage != nullptr)
                            pNotesPage->SetName(aNewName);
                    }
                }

                pCurrentPage->SetAutoLayout(aNewAutoLayout, true);

                SdrLayerID aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
                SdrLayerID aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
                aVisibleLayers.Set(aBckgrnd, bBVisible);
                aVisibleLayers.Set(aBckgrndObj, bBObjsVisible);
                pCurrentPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
            }
            else
            {
                pHandoutMPage->SetAutoLayout(aNewAutoLayout, true);
            }

            mrViewShell.GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHPAGE,
                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);

            bool bSetModified = true;

            if (pArgs->Count() == 1)
            {
                bSetModified = static_cast<const SfxBoolItem&>(pArgs->Get(SID_MODIFYPAGE)).GetValue();
            }

            pUndoManager->AddUndoAction( std::make_unique<UndoAutoLayoutPosAndSize>( *pUndoPage ) );
            pUndoManager->LeaveListAction();

            pDocument->SetChanged(bSetModified);
        }
    }
    while (false);

    mrViewShell.Cancel();
    rRequest.Done ();
}

void ViewShell::Implementation::AssignLayout ( SfxRequest const & rRequest, PageKind ePageKind )
{
    const SfxUInt32Item* pWhatPage = rRequest.GetArg<SfxUInt32Item>(ID_VAL_WHATPAGE);
    const SfxUInt32Item* pWhatLayout = rRequest.GetArg<SfxUInt32Item>(ID_VAL_WHATLAYOUT);

    SdDrawDocument* pDocument = mrViewShell.GetDoc();
    if( !pDocument )
        return;

    SdPage* pPage = nullptr;
    if( pWhatPage )
    {
        pPage = pDocument->GetSdPage(static_cast<sal_uInt16>(pWhatPage->GetValue()), ePageKind);
    }

    if( pPage == nullptr )
        pPage = mrViewShell.getCurrentPage();

    if( !pPage )
        return;

    AutoLayout eLayout = pPage->GetAutoLayout();

    if( pWhatLayout )
        eLayout = static_cast< AutoLayout >( pWhatLayout->GetValue() );

    // Transform the given request into the four argument form that is
    // understood by ProcessModifyPageSlot().
    SdrLayerAdmin& rLayerAdmin (mrViewShell.GetViewShellBase().GetDocument()->GetLayerAdmin());
    SdrLayerID aBackground (rLayerAdmin.GetLayerID(sUNO_LayerName_background));
    SdrLayerID aBackgroundObject (rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects));

    SdrLayerIDSet aVisibleLayers;

    if( pPage->GetPageKind() == PageKind::Handout )
        aVisibleLayers.SetAll();
    else
        aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();

    SfxRequest aRequest (mrViewShell.GetViewShellBase().GetViewFrame(), SID_MODIFYPAGE);
    aRequest.AppendItem(SfxStringItem (ID_VAL_PAGENAME, pPage->GetName()));
    aRequest.AppendItem(SfxUInt32Item (ID_VAL_WHATLAYOUT, eLayout));
    aRequest.AppendItem(SfxBoolItem(ID_VAL_ISPAGEBACK, aVisibleLayers.IsSet(aBackground)));
    aRequest.AppendItem(SfxBoolItem(ID_VAL_ISPAGEOBJ, aVisibleLayers.IsSet(aBackgroundObject)));

    // Forward the call with the new arguments.
    ProcessModifyPageSlot( aRequest, pPage, pPage->GetPageKind());
}

SfxInterfaceId ViewShell::Implementation::GetViewId()
{
    switch (mrViewShell.GetShellType())
    {
        case ViewShell::ST_IMPRESS:
        case ViewShell::ST_NOTES:
        case ViewShell::ST_HANDOUT:
            return IMPRESS_FACTORY_ID;

        case ViewShell::ST_DRAW:
            return DRAW_FACTORY_ID;

        case ViewShell::ST_OUTLINE:
            return OUTLINE_FACTORY_ID;

        case ViewShell::ST_SLIDE_SORTER:
            return SLIDE_SORTER_FACTORY_ID;

        case ViewShell::ST_PRESENTATION:
            return PRESENTATION_FACTORY_ID;

        // Since we have to return a view id for every possible shell type
        // and there is not (yet) a proper ViewShellBase sub class for the
        // remaining types we chose the Impress factory as a fall back.
        case ViewShell::ST_SIDEBAR:
        case ViewShell::ST_NONE:
        default:
            return IMPRESS_FACTORY_ID;
    }
}

SvxIMapDlg* ViewShell::Implementation::GetImageMapDialog()
{
    SvxIMapDlg* pDialog = nullptr;
    SfxChildWindow* pChildWindow = SfxViewFrame::Current()->GetChildWindow(
        SvxIMapDlgChildWindow::GetChildWindowId());
    if (pChildWindow != nullptr)
        pDialog = dynamic_cast<SvxIMapDlg*>(pChildWindow->GetWindow());
    return pDialog;
}

//===== ToolBarManagerLock ====================================================

class ViewShell::Implementation::ToolBarManagerLock::Deleter { public:
    void operator() (ToolBarManagerLock* pObject) { delete pObject; }
};

std::shared_ptr<ViewShell::Implementation::ToolBarManagerLock>
    ViewShell::Implementation::ToolBarManagerLock::Create (
        const std::shared_ptr<ToolBarManager>& rpManager)
{
    std::shared_ptr<ToolBarManagerLock> pLock (
        new ViewShell::Implementation::ToolBarManagerLock(rpManager),
        ViewShell::Implementation::ToolBarManagerLock::Deleter());
    pLock->mpSelf = pLock;
    return pLock;
}

ViewShell::Implementation::ToolBarManagerLock::ToolBarManagerLock (
    const std::shared_ptr<ToolBarManager>& rpManager)
    : mpLock(new ToolBarManager::UpdateLock(rpManager)),
      maTimer()
{
    // Start a timer that will unlock the ToolBarManager update lock when
    // that is not done explicitly by calling Release().
    maTimer.SetInvokeHandler(LINK(this,ToolBarManagerLock,TimeoutCallback));
    maTimer.SetTimeout(100);
    maTimer.Start();
}

IMPL_LINK_NOARG(ViewShell::Implementation::ToolBarManagerLock, TimeoutCallback, Timer *, void)
{
    // If possible then release the lock now.  Otherwise start the timer
    // and try again later.
    if (Application::IsUICaptured())
    {
        maTimer.Start();
    }
    else
    {
        mpSelf.reset();
    }
}

void ViewShell::Implementation::ToolBarManagerLock::Release (bool bForce)
{
    // If possible then release the lock now.  Otherwise try again when the
    // timer expires.
    if (bForce || ! Application::IsUICaptured())
    {
        mpSelf.reset();
    }
}

ViewShell::Implementation::ToolBarManagerLock::~ToolBarManagerLock()
{
    mpLock.reset();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
