/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ViewShellImplementation.cxx,v $
 * $Revision: 1.27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "ViewShellImplementation.hxx"

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include "app.hrc"
#include "strings.hrc"
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
#include "TaskPaneViewShell.hxx"
#include "ViewShellBase.hxx"
#include "FrameView.hxx"
#include "DrawViewShell.hxx"
#include "ViewShellHint.hxx"
#include "framework/FrameworkHelper.hxx"

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svtools/aeitem.hxx>
#include <svx/imapdlg.hxx>
#include <vcl/msgbox.hxx>
#include <basic/sbstar.hxx>
#include "undo/undoobjects.hxx"

#include <com/sun/star/drawing/framework/XControllerManager.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::framework::FrameworkHelper;

namespace {

class ImpUndoDeleteWarning : public ModalDialog
{
private:
    FixedImage      maImage;
    FixedText       maWarningFT;
    CheckBox        maDisableCB;
    OKButton        maYesBtn;
    CancelButton    maNoBtn;

public:
    ImpUndoDeleteWarning(Window* pParent);
    BOOL IsWarningDisabled() const { return maDisableCB.IsChecked(); }
};

ImpUndoDeleteWarning::ImpUndoDeleteWarning(Window* pParent)
:   ModalDialog(pParent, SdResId(RID_UNDO_DELETE_WARNING)),
    maImage(this, SdResId(IMG_UNDO_DELETE_WARNING)),
    maWarningFT(this, SdResId(FT_UNDO_DELETE_WARNING)),
    maDisableCB(this, SdResId(CB_UNDO_DELETE_DISABLE)),
    maYesBtn(this, SdResId(BTN_UNDO_DELETE_YES)),
    maNoBtn(this, SdResId(BTN_UNDO_DELETE_NO))
{
    FreeResource();

    SetHelpId( HID_SD_UNDODELETEWARNING_DLG );
    maDisableCB.SetHelpId( HID_SD_UNDODELETEWARNING_CBX );

    maYesBtn.SetText(Button::GetStandardText(BUTTON_YES));
    maNoBtn.SetText(Button::GetStandardText(BUTTON_NO));
    maImage.SetImage(WarningBox::GetStandardImage());

    // #93721# Set focus to YES-Button
    maYesBtn.GrabFocus();
}

} // end of anonymous namespace




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
    BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
    BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
    SetOfByte aVisibleLayers;
    BOOL bHandoutMode = FALSE;
    SdPage* pHandoutMPage = NULL;
    String aNewName;

    // #95981#
    String aOldName;

    AutoLayout aNewAutoLayout;

    BOOL bBVisible;
    BOOL bBObjsVisible;
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
            if (pArgs && pArgs->Count() == 2)
            {
                // We have been called with a request that contains two
                // arguments.  One was used as preselected layout in a
                // dialog.  We could select that layout in the
                // layout panel instead.
                /*
                    SFX_REQUEST_ARG (rRequest, pNewAutoLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, FALSE);
                    eNewAutoLayout = (AutoLayout) pNewAutoLayout->GetValue
                    ();
                */
            }

            // Make the layout menu visible in the tool pane.
            SfxBoolItem aMakeToolPaneVisible (ID_VAL_ISVISIBLE, TRUE);
            SfxUInt32Item aPanelId (ID_VAL_PANEL_INDEX,
                ::sd::toolpanel::TaskPaneViewShell::PID_LAYOUT);
            SfxViewFrame* pFrame = mrViewShell.GetViewFrame();
            if (pFrame!=NULL && pFrame->GetDispatcher()!=NULL)
            {
                pFrame->GetDispatcher()->Execute (
                    SID_TASK_PANE,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                    &aMakeToolPaneVisible,
                    &aPanelId,
                    NULL);
            }
            else
            {
                DBG_ASSERT(pFrame!=NULL && pFrame->GetDispatcher()!=NULL,
                    "ViewShell::Implementation::ProcessModifyPageSlot(): can not get dispatcher");
            }

            // We have activated a non-modal control in the task pane.
            // Because it does not return anything we can not do anything
            // more right now and have to exit here.
            break;
        }
        else if (pArgs->Count() == 4)
        {
            SFX_REQUEST_ARG (rRequest, pNewName, SfxStringItem, ID_VAL_PAGENAME, FALSE);
            SFX_REQUEST_ARG (rRequest, pNewAutoLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, FALSE);
            SFX_REQUEST_ARG (rRequest, pBVisible, SfxBoolItem, ID_VAL_ISPAGEBACK, FALSE);
            SFX_REQUEST_ARG (rRequest, pBObjsVisible, SfxBoolItem, ID_VAL_ISPAGEOBJ, FALSE);
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
                StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
                rRequest.Ignore ();
                break;
            }
            if (ePageKind == PK_HANDOUT)
            {
                bHandoutMode = TRUE;
                pHandoutMPage = pDocument->GetMasterSdPage(0, PK_HANDOUT);
            }
        }
        else
        {
            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            rRequest.Ignore ();
            break;
        }

        SdPage* pUndoPage =
            bHandoutMode ? pHandoutMPage : pCurrentPage;

        SfxUndoManager* pUndoManager = mrViewShell.GetDocSh()->GetUndoManager();
        DBG_ASSERT(pUndoManager, "No UNDO MANAGER ?!?");

        if( pUndoManager )
        {
            String aComment( SdResId(STR_UNDO_MODIFY_PAGE) );
            pUndoManager->EnterListAction(aComment, aComment);
            ModifyPageUndoAction* pAction = new ModifyPageUndoAction(
                pUndoManager, pDocument, pUndoPage, aNewName, aNewAutoLayout, bBVisible, bBObjsVisible);
            pUndoManager->AddUndoAction(pAction);

            // Clear the selection because the selectec object may be removed as
            // a result of the ssignment of the layout.
            mrViewShell.GetDrawView()->UnmarkAll();

            if (!bHandoutMode)
            {
                if (pCurrentPage->GetName() != aNewName)
                {
                    pCurrentPage->SetName(aNewName);

                    if (ePageKind == PK_STANDARD)
                    {
                        USHORT nPage = (pCurrentPage->GetPageNum()-1) / 2;
                        SdPage* pNotesPage = pDocument->GetSdPage(nPage, PK_NOTES);
                        if (pNotesPage != NULL)
                            pNotesPage->SetName(aNewName);
                    }
                }

                pCurrentPage->SetAutoLayout(aNewAutoLayout, TRUE);

                aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
                aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
                aVisibleLayers.Set(aBckgrnd, bBVisible);
                aVisibleLayers.Set(aBckgrndObj, bBObjsVisible);
                pCurrentPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
            }
            else
            {
                pHandoutMPage->SetAutoLayout(aNewAutoLayout, TRUE);
            }

            mrViewShell.GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHPAGE,
                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

            BOOL bSetModified = TRUE;

            if (pArgs && pArgs->Count() == 1)
            {
                bSetModified = (BOOL) ((SfxBoolItem&) pArgs->Get(SID_MODIFYPAGE)).GetValue();
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




void ViewShell::Implementation::AssignLayout (
    SdPage* pPage,
    AutoLayout aLayout)
{
    // Transform the given request into the four argument form that is
    // understood by ProcessModifyPageSlot().
    SdrLayerAdmin& rLayerAdmin (mrViewShell.GetViewShellBase().GetDocument()->GetLayerAdmin());
    BYTE aBackground (rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE));
    BYTE aBackgroundObject (rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE));
    SetOfByte aVisibleLayers (pPage->TRG_GetMasterPageVisibleLayers());
    SfxRequest aRequest (mrViewShell.GetViewShellBase().GetViewFrame(), SID_MODIFYPAGE);
    aRequest.AppendItem(SfxStringItem (ID_VAL_PAGENAME, pPage->GetName()));
    aRequest.AppendItem(SfxUInt32Item (ID_VAL_WHATLAYOUT, aLayout));
    aRequest.AppendItem(SfxBoolItem(ID_VAL_ISPAGEBACK, aVisibleLayers.IsSet(aBackground)));
    aRequest.AppendItem(SfxBoolItem(ID_VAL_ISPAGEOBJ, aVisibleLayers.IsSet(aBackgroundObject)));

    // Forward the call with the new arguments.
    ProcessModifyPageSlot (
        aRequest,
        pPage,
        pPage->GetPageKind());
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

        // Since we have to return a view id for every possible shell type
        // and there is not (yet) a proper ViewShellBase sub class for the
        // remaining types we chose the Impress factory as a fall back.
        case ViewShell::ST_TASK_PANE:
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



//===== ToolBarManagerLock ====================================================

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
    // Start a timer that will unlock the ToolBarManager update lock when
    // that is not done explicitly by calling Release().
    maTimer.SetTimeoutHdl(LINK(this,ToolBarManagerLock,TimeoutCallback));
    maTimer.SetTimeout(100);
    maTimer.Start();
}




IMPL_LINK(ViewShell::Implementation::ToolBarManagerLock,TimeoutCallback,Timer*,EMPTYARG)
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
    return 0;
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




ViewShell::Implementation::ToolBarManagerLock::~ToolBarManagerLock (void)
{
    mpLock.reset();
}

} // end of namespace sd
