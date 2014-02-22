/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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




ViewShell::Implementation::~Implementation (void)
{
    if ( ! mpUpdateLockForMouse.expired())
    {
        ::boost::shared_ptr<ToolBarManagerLock> pLock(mpUpdateLockForMouse);
        if (pLock.get() != NULL)
        {
            
            
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
    sal_Bool bHandoutMode = sal_False;
    SdPage* pHandoutMPage = NULL;
    OUString aNewName;

    AutoLayout aNewAutoLayout;

    sal_Bool bBVisible;
    sal_Bool bBObjsVisible;
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

            
            sfx2::sidebar::Sidebar::ShowPanel(
                OUString("ImpressLayoutsPanel"),
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
#ifndef DISABLE_SCRIPTING
                StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
#endif
                rRequest.Ignore ();
                break;
            }
            if (ePageKind == PK_HANDOUT)
            {
                bHandoutMode = sal_True;
                pHandoutMPage = pDocument->GetMasterSdPage(0, PK_HANDOUT);
            }
        }
        else
        {
#ifndef DISABLE_SCRIPTING
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
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

                pCurrentPage->SetAutoLayout(aNewAutoLayout, sal_True);

                aBckgrnd = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false);
                aBckgrndObj = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false);
                aVisibleLayers.Set(aBckgrnd, bBVisible);
                aVisibleLayers.Set(aBckgrndObj, bBObjsVisible);
                pCurrentPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
            }
            else
            {
                pHandoutMPage->SetAutoLayout(aNewAutoLayout, sal_True);
            }

            mrViewShell.GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHPAGE,
                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

            sal_Bool bSetModified = sal_True;

            if (pArgs && pArgs->Count() == 1)
            {
                bSetModified = (sal_Bool) ((SfxBoolItem&) pArgs->Get(SID_MODIFYPAGE)).GetValue();
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
    const SfxUInt32Item* pWhatPage = static_cast< const SfxUInt32Item*  > ( rRequest.GetArg( ID_VAL_WHATPAGE, false, TYPE(SfxUInt32Item) ) );
    const SfxUInt32Item* pWhatLayout = static_cast< const SfxUInt32Item*  > ( rRequest.GetArg( ID_VAL_WHATLAYOUT, false, TYPE(SfxUInt32Item) ) );

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

        
        ProcessModifyPageSlot( aRequest, pPage, pPage->GetPageKind());
    }
}




sal_uInt16 ViewShell::Implementation::GetViewId (void)
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

        
        
        
        case ViewShell::ST_SIDEBAR:
        case ViewShell::ST_NONE:
        default:
            return IMPRESS_FACTORY_ID;
    }
}




SvxIMapDlg* ViewShell::Implementation::GetImageMapDialog (void)
{
    SvxIMapDlg* pDialog = NULL;
    SfxChildWindow* pChildWindow = SfxViewFrame::Current()->GetChildWindow(
        SvxIMapDlgChildWindow::GetChildWindowId());
    if (pChildWindow != NULL)
        pDialog = dynamic_cast<SvxIMapDlg*>(pChildWindow->GetWindow());
    return pDialog;
}





class ViewShell::Implementation::ToolBarManagerLock::Deleter { public:
    void operator() (ToolBarManagerLock* pObject) { delete pObject; }
};

::boost::shared_ptr<ViewShell::Implementation::ToolBarManagerLock>
    ViewShell::Implementation::ToolBarManagerLock::Create (
        const ::boost::shared_ptr<ToolBarManager>& rpManager)
{
    ::boost::shared_ptr<ToolBarManagerLock> pLock (
        new ViewShell::Implementation::ToolBarManagerLock(rpManager),
        ViewShell::Implementation::ToolBarManagerLock::Deleter());
    pLock->mpSelf = pLock;
    return pLock;
}




ViewShell::Implementation::ToolBarManagerLock::ToolBarManagerLock (
    const ::boost::shared_ptr<ToolBarManager>& rpManager)
    : mpLock(new ToolBarManager::UpdateLock(rpManager)),
      maTimer()
{
    
    
    maTimer.SetTimeoutHdl(LINK(this,ToolBarManagerLock,TimeoutCallback));
    maTimer.SetTimeout(100);
    maTimer.Start();
}




IMPL_LINK_NOARG(ViewShell::Implementation::ToolBarManagerLock, TimeoutCallback)
{
    
    
    if (Application::IsUICaptured())
    {
        maTimer.Start();
    }
    else
    {
        mpSelf.reset();
    }
    return 0;
}




void ViewShell::Implementation::ToolBarManagerLock::Release (bool bForce)
{
    
    
    if (bForce || ! Application::IsUICaptured())
    {
        mpSelf.reset();
    }
}




ViewShell::Implementation::ToolBarManagerLock::~ToolBarManagerLock (void)
{
    mpLock.reset();
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
