/*************************************************************************
 *
 *  $RCSfile: ViewShellImplementation.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 09:00:53 $
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

#include "ViewShellImplementation.hxx"

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include "app.hrc"
#include "new_foil.hrc"
#include "strings.hrc"
#include "strings.hrc"
#include "helpids.h"
#include "sdattr.hxx"
#include "sdabstdlg.hxx"
#include "unmodpg.hxx"
#include "Window.hxx"
#include "optsitem.hxx"
#include "DrawDocShell.hxx"
#include "PreviewChildWindow.hxx"
#include "PreviewWindow.hxx"
#include "fuslshow.hxx"

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svtools/aeitem.hxx>
#include <vcl/msgbox.hxx>
#include <basic/sbstar.hxx>

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
      mrViewShell (rViewShell)
{
}




ViewShell::Implementation::~Implementation (void)
{
}




void ViewShell::Implementation::ProcessModifyPageSlot (
    SfxRequest& rRequest,
    SdPage* pCurrentPage,
    PageKind ePageKind)
{
    SdDrawDocument* pDocument = mrViewShell.GetDoc();
    USHORT nPage = pCurrentPage->GetPageNum();
    //    pCurrentPage = pDocument->GetSdPage(nPage, ePageKind);
    SdrLayerAdmin& rLayerAdmin = pDocument->GetLayerAdmin();
    BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
    BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
    SetOfByte aVisibleLayers = pCurrentPage->TRG_GetMasterPageVisibleLayers();
    BOOL bHandoutMode = FALSE;
    SdPage* pHandoutMPage = NULL;
    String aNewName;

    // #95981#
    String aOldName;

    AutoLayout aNewAutoLayout;

    // #95981#
    AutoLayout aOldAutoLayout;

    BOOL bBVisible;
    BOOL bBObjsVisible;
    const SfxItemSet* pArgs = rRequest.GetArgs();

    do
    {
    if (!pArgs || pArgs->Count() == 1 || pArgs->Count() == 2 )
    {
        SfxItemSet aAttrSet (mrViewShell.GetPool(),
            ATTR_PAGE_START, ATTR_PAGE_END );

        // #95981# keep old name
        aOldName = pCurrentPage->GetName();

        aAttrSet.Put( SfxStringItem( ATTR_PAGE_NAME, aOldName ) );
        aAttrSet.Put( SfxBoolItem( ATTR_PAGE_BACKGROUND,
                aVisibleLayers.IsSet(aBckgrnd) ) );
        aAttrSet.Put( SfxBoolItem( ATTR_PAGE_OBJECTS,
                aVisibleLayers.IsSet(aBckgrndObj) ) );

        AutoLayout eNewAutoLayout = AUTOLAYOUT_NONE;

        if ( pArgs && pArgs->Count() == 2 )
        {
            SFX_REQUEST_ARG (rRequest, pNewAutoLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, FALSE);
            eNewAutoLayout = (AutoLayout) pNewAutoLayout->GetValue ();
        }
        else
        {
            if (ePageKind != PK_HANDOUT)
            {
                eNewAutoLayout = pCurrentPage->GetAutoLayout();
            }
            else
            {
                bHandoutMode = TRUE;
                pHandoutMPage = pDocument->GetMasterSdPage(0, PK_HANDOUT);
                eNewAutoLayout = pHandoutMPage->GetAutoLayout();
            }
        }

        // #95981# keep old AutoLayout
        aOldAutoLayout = eNewAutoLayout;

        aAttrSet.Put( SfxAllEnumItem( ATTR_PAGE_LAYOUT, aOldAutoLayout ) );

        //CHINA001 SdNewFoilDlg* pDlg = new SdNewFoilDlg(pWindow, aAttrSet, ePageKind, GetDocSh(), TRUE);
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
        DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
        AbstractSdNewFoilDlg* pDlg = pFact->CreateSdNewFoilDlg(
            ResId( DLG_NEW_FOIL ),
            mrViewShell.GetActiveWindow(),
            aAttrSet,
            ePageKind,
            mrViewShell.GetDocSh(),
            TRUE);
        DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
        if (pDlg->Execute() == RET_OK)
        {
            pDlg->GetAttr( aAttrSet );

            aNewName       = ((const SfxStringItem &) aAttrSet.Get (ATTR_PAGE_NAME)).GetValue ();
            aNewAutoLayout = (AutoLayout) ((const SfxAllEnumItem &)
                aAttrSet.Get (ATTR_PAGE_LAYOUT)).GetValue ();
            bBVisible      = ((const SfxBoolItem &) aAttrSet.Get (ATTR_PAGE_BACKGROUND)).GetValue ();
            bBObjsVisible  = ((const SfxBoolItem &) aAttrSet.Get (ATTR_PAGE_OBJECTS)).GetValue ();

            delete pDlg;

            // Clear the selection because the selectec object may be
            // removed as a result of the ssignment of the layout.
            mrViewShell.GetDrawView()->UnmarkAll();
        }
        else
        {
            SdPage* pPage = pDocument->GetSdPage(0, PK_STANDARD);
            if (pDocument->GetSdPageCount(PK_STANDARD) == 1 &&
                pPage->GetAutoLayout() == AUTOLAYOUT_TITLE &&
                pPage->GetPresObjList().empty())
            {
                // Nur eine Seite vorhanden
                pPage->SetAutoLayout(AUTOLAYOUT_NONE);
            }
            delete pDlg;
            rRequest.Ignore ();
            mrViewShell.Cancel ();
            break;
        }
    }
    else if (pArgs->Count() == 4)
    {
        SFX_REQUEST_ARG (rRequest, pNewName, SfxStringItem, ID_VAL_PAGENAME, FALSE);
        SFX_REQUEST_ARG (rRequest, pNewAutoLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, FALSE);
        SFX_REQUEST_ARG (rRequest, pBVisible, SfxBoolItem, ID_VAL_ISPAGEBACK, FALSE);
        SFX_REQUEST_ARG (rRequest, pBObjsVisible, SfxBoolItem, ID_VAL_ISPAGEOBJ, FALSE);
        AutoLayout aLayout ((AutoLayout)pNewAutoLayout->GetValue ());
        if (aLayout >= AUTOLAYOUT_TITLE
            && aLayout <= AUTOLAYOUT_HANDOUT6)
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

    // #67720#
    SfxUndoManager* pUndoManager = mrViewShell.GetDocSh()->GetUndoManager();
    DBG_ASSERT(pUndoManager, "No UNDO MANAGER ?!?");

    // #90356#
    sal_uInt16 nActionCount(pUndoManager->GetUndoActionCount());
    sal_Bool bContinue(sal_True);

    if(nActionCount)
    {
        // #90356# get SdOptions
        SdOptions* pOptions = SD_MOD()->GetSdOptions(pDocument->GetDocumentType());
        sal_Bool bShowDialog(pOptions->IsShowUndoDeleteWarning());

        // #95981# If only name is changed do not show
        // ImpUndoDeleteWarning dialog
        if(bShowDialog)
        {
            sal_Bool bNameChanged(aNewName != aOldName);
            sal_Bool bLayoutChanged(aNewAutoLayout != aOldAutoLayout);

            if(bNameChanged && !bLayoutChanged)
                bShowDialog = sal_False;
        }

        if(bShowDialog)
        {
            // ask user if he wants to loose UNDO stack
            ImpUndoDeleteWarning aUndoDeleteDlg(mrViewShell.GetActiveWindow());

            if(BUTTONID_OK == aUndoDeleteDlg.Execute())
            {
                pUndoManager->Clear();
            }
            else
            {
                bContinue = sal_False;
            }

            // #90356# write option flag back if change was done
            if(aUndoDeleteDlg.IsWarningDisabled())
            {
                pOptions->SetShowUndoDeleteWarning(FALSE);
            }
        }
    }

    if(bContinue)
    {
        ModifyPageUndoAction* pAction = new ModifyPageUndoAction(
            pUndoManager, pDocument, pUndoPage, aNewName, aNewAutoLayout, bBVisible, bBObjsVisible);
        pUndoManager->AddUndoAction(pAction);

        // Clear the selection because the selectec object may be removed as
        // a result of the ssignment of the layout.
        mrViewShell.GetDrawView()->UnmarkAll();

        SfxChildWindow* pPreviewChildWindow =
            mrViewShell.GetViewFrame()->GetChildWindow(
                PreviewChildWindow::GetChildWindowId() );
        PreviewWindow* pPreviewWin = NULL;

        // notify preview slide show are changes are to be done
        if( pPreviewChildWindow!=NULL
            && (pPreviewWin = static_cast<PreviewWindow*>(
                pPreviewChildWindow->GetWindow()))!= NULL)
        {
            FuSlideShow* pShow = pPreviewWin->GetSlideShow();

            if( pShow )
                pShow->InitPageModify();
        }

        if (!bHandoutMode)
        {
            if (pCurrentPage->GetName() != aNewName)
            {
                pCurrentPage->SetName(aNewName);

                if (ePageKind == PK_STANDARD)
                {
                    SdPage* pNotesPage = pDocument->GetSdPage(nPage, PK_NOTES);
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
        // The list of presentation objects at the page
        // has been cleared by SetAutolayout().  We still
        // have to clear the list of removed presentation
        // objects held by the model which references the
        // former list.
        pDocument->ClearDeletedPresObjList();

        mrViewShell.GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHPAGE,
            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

        BOOL bSetModified = TRUE;

        if (pArgs && pArgs->Count() == 1)
        {
            bSetModified = (BOOL) ((SfxBoolItem&) pArgs->Get(SID_MODIFYPAGE)).GetValue();
        }

        pDocument->SetChanged(bSetModified);
    }
    }
    while (false);

    mrViewShell.Cancel();
    rRequest.Done ();
}

} // end of namespace sd
