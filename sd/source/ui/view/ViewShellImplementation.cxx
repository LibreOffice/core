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

#include "ViewShellImplementation.hxx"

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include "app.hrc"
#include "strings.hrc"
#include "helpids.h"
#include "sdattr.hxx"
#include "sdabstdlg.hxx"
#include "unmodpg.hxx"
#include "Window.hxx"
#include "optsitem.hxx"
#include "DrawDocShell.hxx"
#include "DrawController.hxx"
#include "FactoryIds.hxx"
#include "slideshow.hxx"
#include "ViewShellBase.hxx"
#include "FrameView.hxx"
#include "DrawViewShell.hxx"
#include "ViewShellHint.hxx"
#include "SidebarPanelId.hxx"
#include "framework/FrameworkHelper.hxx"

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svl/aeitem.hxx>
#include <svx/imapdlg.hxx>
#include <vcl/msgbox.hxx>
#include <basic/sbstar.hxx>
#include "undo/undoobjects.hxx"

#include <com/sun/star/drawing/framework/XControllerManager.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;

namespace sd {

ViewShell::Implementation::Implementation (ViewShell& rViewShell)
    : mbIsShowingUIControls(false),
      mbIsMainViewShell(false),
      mbIsInitialized(false),
      mbArrangeActive(false),
      mpSubShellFactory(),
      mpUpdateLockForMouse(),
      mrViewShell(rViewShell)
{
}

ViewShell::Implementation::~Implementation()
{
    if ( ! mpUpdateLockForMouse.expired())
    {
        std::shared_ptr<ToolBarManagerLock> pLock(mpUpdateLockForMouse);
        if (pLock.get() != NULL)
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
    sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false);
    sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false);
    SetOfByte aVisibleLayers;
    bool bHandoutMode = false;
    SdPage* pHandoutMPage = NULL;
    OUString aNewName;

    AutoLayout aNewAutoLayout;

    bool bBVisible;
    bool bBObjsVisible;
    const SfxItemSet* pArgs = rRequest.GetArgs();

    if (pCurrentPage != NULL && pCurrentPage->TRG_HasMasterPage())
        aVisibleLayers = pCurrentPage->TRG_GetMasterPageVisibleLayers();
    else
        aVisibleLayers.SetAll();

    do
    {
        if (pCurrentPage == NULL)
            break;

        if (!pArgs || pArgs->Count() == 1 || pArgs->Count() == 2 )
        {
            // First make sure that the sidebar is visible
            mrViewShell.GetViewFrame()->ShowChildWindow(SID_SIDEBAR);
            sfx2::sidebar::Sidebar::ShowPanel(
                OUString("SdLayoutsPanel"),
                mrViewShell.GetViewFrame()->GetFrame().GetFrameInterface());
            break;
        }
        else if (pArgs->Count() == 4)
        {
            SFX_REQUEST_ARG (rRequest, pNewName, SfxStringItem, ID_VAL_PAGENAME, false);
            SFX_REQUEST_ARG (rRequest, pNewAutoLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, false);
            SFX_REQUEST_ARG (rRequest, pBVisible, SfxBoolItem, ID_VAL_ISPAGEBACK, false);
            SFX_REQUEST_ARG (rRequest, pBObjsVisible, SfxBoolItem, ID_VAL_ISPAGEOBJ, false);
            AutoLayout aLayout ((AutoLayout)pNewAutoLayout->GetValue ());
            if (aLayout >= AUTOLAYOUT__START
                && aLayout < AUTOLAYOUT__END)
            {
                aNewName        = pNewName->GetValue ();
                aNewAutoLayout = (AutoLayout) pNewAutoLayout->GetValue ();
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
            if (ePageKind == PK_HANDOUT)
            {
                bHandoutMode = true;
                pHandoutMPage = pDocument->GetMasterSdPage(0, PK_HANDOUT);
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

        ::svl::IUndoManager* pUndoManager = mrViewShell.GetDocSh()->GetUndoManager();
        DBG_ASSERT(pUndoManager, "No UNDO MANAGER ?!?");

        if( pUndoManager )
        {
            OUString aComment( SdResId(STR_UNDO_MODIFY_PAGE) );
            pUndoManager->EnterListAction(aComment, aComment);
            ModifyPageUndoAction* pAction = new ModifyPageUndoAction(
                pDocument, pUndoPage, aNewName, aNewAutoLayout, bBVisible, bBObjsVisible);
            pUndoManager->AddUndoAction(pAction);

            // Clear the selection because the selected object may be removed as
            // a result of the assignment of the layout.
            mrViewShell.GetDrawView()->UnmarkAll();

            if (!bHandoutMode)
            {
                if (pCurrentPage->GetName() != aNewName)
                {
                    pCurrentPage->SetName(aNewName);

                    if (ePageKind == PK_STANDARD)
                    {
                        sal_uInt16 nPage = (pCurrentPage->GetPageNum()-1) / 2;
                        SdPage* pNotesPage = pDocument->GetSdPage(nPage, PK_NOTES);
                        if (pNotesPage != NULL)
                            pNotesPage->SetName(aNewName);
                    }
                }

                pCurrentPage->SetAutoLayout(aNewAutoLayout, true);

                aBckgrnd = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false);
                aBckgrndObj = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false);
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

            if (pArgs && pArgs->Count() == 1)
            {
                bSetModified = static_cast<const SfxBoolItem&>(pArgs->Get(SID_MODIFYPAGE)).GetValue();
            }

            pUndoManager->AddUndoAction( new UndoAutoLayoutPosAndSize( *pUndoPage ) );
            pUndoManager->LeaveListAction();

            pDocument->SetChanged(bSetModified);
        }
    }
    while (false);

    mrViewShell.Cancel();
    rRequest.Done ();
}

void ViewShell::Implementation::AssignLayout ( SfxRequest& rRequest, PageKind ePageKind )
{
    const SfxUInt32Item* pWhatPage = static_cast< const SfxUInt32Item*  > ( rRequest.GetArg( ID_VAL_WHATPAGE, false, checkSfxPoolItem< SfxUInt32Item > ) );
    const SfxUInt32Item* pWhatLayout = static_cast< const SfxUInt32Item*  > ( rRequest.GetArg( ID_VAL_WHATLAYOUT, false, checkSfxPoolItem< SfxUInt32Item > ) );

    SdDrawDocument* pDocument = mrViewShell.GetDoc();
    if( !pDocument )
        return;

    SdPage* pPage = 0;
    if( pWhatPage )
    {
        pPage = pDocument->GetSdPage(static_cast<sal_uInt16>(pWhatPage->GetValue()), ePageKind);
    }

    if( pPage == 0 )
        pPage = mrViewShell.getCurrentPage();

    if( pPage )
    {
        AutoLayout eLayout = pPage->GetAutoLayout();

        if( pWhatLayout )
            eLayout = static_cast< AutoLayout >( pWhatLayout->GetValue() );

        // Transform the given request into the four argument form that is
        // understood by ProcessModifyPageSlot().
        SdrLayerAdmin& rLayerAdmin (mrViewShell.GetViewShellBase().GetDocument()->GetLayerAdmin());
        sal_uInt8 aBackground (rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false));
        sal_uInt8 aBackgroundObject (rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false));

        SetOfByte aVisibleLayers;

        if( pPage->GetPageKind() == PK_HANDOUT )
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
}

sal_uInt16 ViewShell::Implementation::GetViewId()
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
    SvxIMapDlg* pDialog = NULL;
    SfxChildWindow* pChildWindow = SfxViewFrame::Current()->GetChildWindow(
        SvxIMapDlgChildWindow::GetChildWindowId());
    if (pChildWindow != NULL)
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
    maTimer.SetTimeoutHdl(LINK(this,ToolBarManagerLock,TimeoutCallback));
    maTimer.SetTimeout(100);
    maTimer.Start();
}

IMPL_LINK_NOARG_TYPED(ViewShell::Implementation::ToolBarManagerLock, TimeoutCallback, Timer *, void)
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
