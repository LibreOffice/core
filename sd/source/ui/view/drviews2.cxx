/*************************************************************************
 *
 *  $RCSfile: drviews2.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 15:01:17 $
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

#include "DrawViewShell.hxx"
#include "ViewShellImplementation.hxx"

#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

#ifndef _SVDOGRAF_HXX
#include <svx/svdograf.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDUNDO_HXX //autogen
#include <svx/svdundo.hxx>
#endif
#ifndef _ZOOMITEM_HXX
#include <svx/zoomitem.hxx>
#endif
#ifndef _EDITDATA_HXX
#include <svx/editdata.hxx>
#endif
#ifndef _SB_SBERRORS_HXX //autogen
#include <basic/sberrors.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX //autogen
#include <svx/xflclit.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif

#define ITEMID_FIELD    EE_FEATURE_FIELD
#ifndef _SVX_FLDITEM_HXX //autogen
#include <svx/flditem.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX //autogen
#include <svx/xlineit0.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif

#ifndef _SDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif

#ifndef _SVX_XLNWTIT_HXX
#include <svx/xlnwtit.hxx>
#endif
#ifndef _SVDOATTR_HXX //autogen
#include <svx/svdoattr.hxx>
#endif
#ifndef _SVX_XLNSTWIT_HXX
#include <svx/xlnstwit.hxx>
#endif
#ifndef _SDTMFITM_HXX //autogen
#include <svx/sdtmfitm.hxx>
#endif
#ifndef _SDTAGITM_HXX //autogen
#include <svx/sdtagitm.hxx>
#endif
#ifndef _SVX_XLNEDWIT_HXX
#include <svx/xlnedwit.hxx>
#endif
#ifndef _SVX_FONTWORK_BAR_HXX
#include <svx/fontworkbar.hxx>
#endif

//CHINA001 #ifndef _SVX_DLG_NAME_HXX
//CHINA001 #include <svx/dlgname.hxx>
//CHINA001 #endif
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#pragma hdrstop

#include "app.hrc"
#include "glob.hrc"
#include "helpids.h"
#include "sdattr.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "ins_page.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#ifndef SD_PREVIEW_WINDOW_HXX
#include "PreviewWindow.hxx"
#endif
#ifndef SD_PREVIEW_CHILD_WINDOW_HXX
#include "PreviewChildWindow.hxx"
#endif
#ifndef SD_FU_SCALE_HXX
#include "fuscale.hxx"
#endif
#include "sdresid.hxx"
//CHINA001 #include "new_foil.hxx"
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#include "unmodpg.hxx"
#ifndef SD_FU_SLIDE_SHOW_HXX
#include "fuslshow.hxx"
#endif
#ifndef SD_FU_VECTORIZE_HXX
#include "fuvect.hxx"
#endif
#include "stlpool.hxx"

// #90356#
#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif
#include "sdabstdlg.hxx" //CHINA001
#include "new_foil.hrc" //CHINA001
namespace sd {

/*************************************************************************
|*
|* modal dialog for #90356#
|*
\************************************************************************/

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


/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void DrawViewShell::FuTemporary(SfxRequest& rReq)
{
    // Waehrend einer Native-Diashow wird nichts ausgefuehrt!
    if (pFuSlideShow && !pFuSlideShow->IsLivePresentation() &&
        rReq.GetSlot() != SID_NAVIGATOR)
    {
        return;
    }

    CheckLineTo (rReq);

    if (pFuActual)
    {
        pFuActual->Deactivate();

        if (pFuActual != pFuOld)
        {
            delete pFuActual;
        }

        pFuActual = NULL;
    }

    USHORT nSId = rReq.GetSlot();

    // Slot wird gemapped (ToolboxImages/-Slots)
    MapSlot( nSId );

    switch ( nSId )
    {
        // Flaechen und Linien-Attribute:
        // Sollten (wie StateMethode) eine eigene
        // Execute-Methode besitzen
        case SID_ATTR_FILL_STYLE:
        case SID_ATTR_FILL_COLOR:
        case SID_ATTR_FILL_GRADIENT:
        case SID_ATTR_FILL_HATCH:
        case SID_ATTR_FILL_BITMAP:
        case SID_ATTR_FILL_SHADOW:

        case SID_ATTR_LINE_STYLE:
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_WIDTH:
        case SID_ATTR_LINE_COLOR:
        case SID_ATTR_LINEEND_STYLE:

        case SID_ATTR_TEXT_FITTOSIZE:
        {
            if( rReq.GetArgs() )
            {
                BOOL bMergeUndo = FALSE;
                SfxUndoManager* pUndoMgr = GetDocSh()->GetUndoManager();

                // Anpassungen Start/EndWidth #63083#
                if(nSId == SID_ATTR_LINE_WIDTH)
                {
                    SdrObject* pObj = NULL;
                    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                    ULONG nCount = rMarkList.GetMarkCount();

                    INT32 nNewLineWidth = ((const XLineWidthItem&)rReq.GetArgs()->Get(XATTR_LINEWIDTH)).GetValue();

                    for (ULONG i=0; i<nCount; i++)
                    {
                        SfxItemSet aAttr(GetDoc()->GetPool());
                        pObj = rMarkList.GetMark(i)->GetObj();
                        aAttr.Put(pObj->GetMergedItemSet());

                        INT32 nActLineWidth = ((const XLineWidthItem&)aAttr.Get(XATTR_LINEWIDTH)).GetValue();

                        if(nActLineWidth != nNewLineWidth)
                        {
                            BOOL bSetItemSet(FALSE);

                            // #86265# do this for SFX_ITEM_DEFAULT and for SFX_ITEM_SET
                            if(SFX_ITEM_DONTCARE != aAttr.GetItemState(XATTR_LINESTARTWIDTH))
                            {
                                INT32 nValAct = ((const XLineStartWidthItem&)aAttr.Get(XATTR_LINESTARTWIDTH)).GetValue();
                                INT32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
                                if(nValNew < 0)
                                    nValNew = 0;
                                bSetItemSet = TRUE;
                                aAttr.Put(XLineStartWidthItem(nValNew));
                            }

                            // #86265# do this for SFX_ITEM_DEFAULT and for SFX_ITEM_SET
                            if(SFX_ITEM_DONTCARE != aAttr.GetItemState(XATTR_LINEENDWIDTH))
                            {
                                INT32 nValAct = ((const XLineEndWidthItem&)aAttr.Get(XATTR_LINEENDWIDTH)).GetValue();
                                INT32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
                                if(nValNew < 0)
                                    nValNew = 0;
                                bSetItemSet = TRUE;
                                aAttr.Put(XLineEndWidthItem(nValNew));
                            }

                            if(bSetItemSet)
                                pObj->SetMergedItemSet(aAttr);
                        }
                    }
                }

                if (nSId == SID_ATTR_FILL_SHADOW)
                {
                    // Ggf. werden transparente Objekte wei?gefuellt
                    SdrObject* pObj = NULL;
                    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                    ULONG nCount = rMarkList.GetMarkCount();

                    for (ULONG i=0; i<nCount; i++)
                    {
                        SfxItemSet aAttr(GetDoc()->GetPool());
                        pObj = rMarkList.GetMark(i)->GetObj();
                        aAttr.Put(pObj->GetMergedItemSet());

                        const XFillStyleItem& rFillStyle =
                         (const XFillStyleItem&) aAttr.Get(XATTR_FILLSTYLE);

                        if (rFillStyle.GetValue() == XFILL_NONE)
                        {
                            // Vorlage hat keine Fuellung,
                            // daher hart attributieren: Fuellung setzen
                            if (!bMergeUndo)
                            {
                                bMergeUndo = TRUE;
                                pUndoMgr->EnterListAction( String(), String() );
                                pDrView->BegUndo();
                            }
                            pDrView->AddUndo(new SdrUndoAttrObj(*pObj));

                            aAttr.Put(XFillStyleItem(XFILL_SOLID));
                            aAttr.Put(XFillColorItem(String(), COL_WHITE));

                            pObj->SetMergedItemSet(aAttr);
                        }
                    }

                    if (bMergeUndo)
                    {
                        pDrView->EndUndo();
                    }
                }

                pDrView->SetAttributes(*rReq.GetArgs());

                if (bMergeUndo)
                {
                    pUndoMgr->LeaveListAction();
                }

                rReq.Done();
            }
            else
            {
                switch( rReq.GetSlot() )
                {
                    case SID_ATTR_FILL_SHADOW:
                    case SID_ATTR_FILL_STYLE:
                    case SID_ATTR_FILL_COLOR:
                    case SID_ATTR_FILL_GRADIENT:
                    case SID_ATTR_FILL_HATCH:
                    case SID_ATTR_FILL_BITMAP:
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_AREA, SFX_CALLMODE_ASYNCHRON );
                        break;
                    case SID_ATTR_LINE_STYLE:
                    case SID_ATTR_LINE_DASH:
                    case SID_ATTR_LINE_WIDTH:
                    case SID_ATTR_LINE_COLOR:
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_LINE, SFX_CALLMODE_ASYNCHRON );
                        break;
                    case SID_ATTR_TEXT_FITTOSIZE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_TEXTATTR_DLG, SFX_CALLMODE_ASYNCHRON );
                        break;
                }
            }
            Cancel();
        }
        break;

        case SID_HYPHENATION:
        {
            // const SfxPoolItem* pItem = rReq.GetArg( SID_HYPHENATION );
            //  ^-- Soll so nicht benutzt werden (Defaults sind falsch) !
            SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, SID_HYPHENATION, FALSE);

            if( pItem )
            {
                SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
                BOOL bValue = ( (const SfxBoolItem*) pItem)->GetValue();
                aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                pDrView->SetAttributes( aSet );
            }
            else // nur zum Test
            {
                DBG_ERROR(" Kein Wert fuer Silbentrennung!");
                SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
                BOOL bValue = TRUE;
                aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                pDrView->SetAttributes( aSet );
            }
            rReq.Done();
            Cancel();
        }
        break;

        case SID_INSERTPAGE:
        case SID_INSERTPAGE_QUICK:
        case SID_DUPLICATE_PAGE:
            CreateOrDuplicatePage (rReq, ePageKind, GetActualPage());
            Cancel();
            if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            rReq.Done ();
        break;

        case SID_MODIFYPAGE:
        {
            if (ePageKind==PK_STANDARD || ePageKind==PK_NOTES ||
                (ePageKind==PK_HANDOUT && eEditMode==EM_MASTERPAGE) )
            {
                if ( pDrView->IsTextEdit() )
                {
                    pDrView->EndTextEdit();
                }
                USHORT nPage = aTabControl.GetCurPageId() - 1;
                pActualPage = GetDoc()->GetSdPage(nPage, ePageKind);
                mpImpl->ProcessModifyPageSlot (
                    rReq,
                    pActualPage,
                    ePageKind);

                /*
                USHORT nPage = aTabControl.GetCurPageId() - 1;
                pActualPage = GetDoc()->GetSdPage(nPage, ePageKind);
                SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
                BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
                BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
                SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();
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
                const SfxItemSet* pArgs = rReq.GetArgs();

                if (!pArgs || pArgs->Count() == 1 || pArgs->Count() == 2 )
                {
                    SfxItemSet aAttrSet( GetPool(), ATTR_PAGE_START, ATTR_PAGE_END );

                    // #95981# keep old name
                    aOldName = pActualPage->GetName();

                    aAttrSet.Put( SfxStringItem( ATTR_PAGE_NAME, aOldName ) );
                    aAttrSet.Put( SfxBoolItem( ATTR_PAGE_BACKGROUND,
                                               aVisibleLayers.IsSet(aBckgrnd) ) );
                    aAttrSet.Put( SfxBoolItem( ATTR_PAGE_OBJECTS,
                                               aVisibleLayers.IsSet(aBckgrndObj) ) );

                    AutoLayout eNewAutoLayout = AUTOLAYOUT_NONE;

                    if ( pArgs && pArgs->Count() == 2 )
                    {
                        SFX_REQUEST_ARG (rReq, pNewAutoLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, FALSE);
                        eNewAutoLayout = (AutoLayout) pNewAutoLayout->GetValue ();
                    }
                    else
                    {
                        if (ePageKind != PK_HANDOUT)
                        {
                            eNewAutoLayout = pActualPage->GetAutoLayout();
                        }
                        else
                        {
                            bHandoutMode = TRUE;
                            pHandoutMPage = GetDoc()->GetMasterSdPage(0, PK_HANDOUT);
                            eNewAutoLayout = pHandoutMPage->GetAutoLayout();
                        }
                    }

                    // #95981# keep old AutoLayout
                    aOldAutoLayout = eNewAutoLayout;

                    aAttrSet.Put( SfxAllEnumItem( ATTR_PAGE_LAYOUT, aOldAutoLayout ) );

                    //CHINA001 SdNewFoilDlg* pDlg = new SdNewFoilDlg(pWindow, aAttrSet, ePageKind, GetDocSh(), TRUE);
                    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
                    DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
                    AbstractSdNewFoilDlg* pDlg = pFact->CreateSdNewFoilDlg(ResId( DLG_NEW_FOIL ), GetActiveWindow(), aAttrSet, ePageKind, GetDocSh(), TRUE );
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

                        // alles deselektieren, denn ein selektiertes Objekt
                        // koennte gleich verschwinden
                        pDrView->UnmarkAll();
                    }
                    else
                    {
                        SdPage* pPage = GetDoc()->GetSdPage(0, PK_STANDARD);
                        if (GetDoc()->GetSdPageCount(PK_STANDARD) == 1 &&
                            pPage->GetAutoLayout() == AUTOLAYOUT_TITLE &&
                            pPage->GetPresObjList().empty())
                        {
                            // Nur eine Seite vorhanden
                            pPage->SetAutoLayout(AUTOLAYOUT_NONE);
                        }
                        delete pDlg;
                        rReq.Ignore ();
                        Cancel ();
                        break;
                    }
                }
                else if (pArgs->Count() == 4)
                {
                    SFX_REQUEST_ARG (rReq, pNewName, SfxStringItem, ID_VAL_PAGENAME, FALSE);
                    SFX_REQUEST_ARG (rReq, pNewAutoLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, FALSE);
                    SFX_REQUEST_ARG (rReq, pBVisible, SfxBoolItem, ID_VAL_ISPAGEBACK, FALSE);
                    SFX_REQUEST_ARG (rReq, pBObjsVisible, SfxBoolItem, ID_VAL_ISPAGEOBJ, FALSE);

                    if (CHECK_RANGE (AUTOLAYOUT_TITLE, (AutoLayout) pNewAutoLayout->GetValue (), AUTOLAYOUT_HANDOUT6))
                    {
                        aNewName        = pNewName->GetValue ();
                        aNewAutoLayout = (AutoLayout) pNewAutoLayout->GetValue ();
                        bBVisible       = pBVisible->GetValue ();
                        bBObjsVisible   = pBObjsVisible->GetValue ();
                    }
                    else
                    {
                        StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
                        rReq.Ignore ();
                        Cancel ();
                        break;
                    }
                    if (ePageKind == PK_HANDOUT)
                    {
                        bHandoutMode = TRUE;
                        pHandoutMPage = GetDoc()->GetMasterSdPage(0, PK_HANDOUT);
                    }
                }
                else
                {
                    StarBASIC::FatalError (SbERR_WRONG_ARGS);
                    rReq.Ignore ();
                    Cancel ();
                    break;
                }

                SdPage* pUndoPage =
                            bHandoutMode ? pHandoutMPage : pActualPage;

                // #67720#
                SfxUndoManager* pUndoManager = GetDocSh()->GetUndoManager();
                DBG_ASSERT(pUndoManager, "No UNDO MANAGER ?!?");

                // #90356#
                sal_uInt16 nActionCount(pUndoManager->GetUndoActionCount());
                sal_Bool bContinue(sal_True);

                if(nActionCount)
                {
                    // #90356# get SdOptions
                    SdOptions* pOptions = SD_MOD()->GetSdOptions(GetDoc()->GetDocumentType());
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
                        ImpUndoDeleteWarning aUndoDeleteDlg(GetActiveWindow());

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
                        pUndoManager, GetDoc(), pUndoPage, aNewName, aNewAutoLayout, bBVisible, bBObjsVisible);
                    pUndoManager->AddUndoAction(pAction);

                    SfxChildWindow* pPreviewChildWindow =
                        GetViewFrame()->GetChildWindow(
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
                        if (pActualPage->GetName() != aNewName)
                        {
                            pActualPage->SetName(aNewName);

                            if (ePageKind == PK_STANDARD)
                            {
                                SdPage* pNotesPage = GetDoc()->GetSdPage(nPage, PK_NOTES);
                                pNotesPage->SetName(aNewName);
                            }
                        }

                        pActualPage->SetAutoLayout(aNewAutoLayout, TRUE);

                        aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
                        aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
                        aVisibleLayers.Set(aBckgrnd, bBVisible);
                        aVisibleLayers.Set(aBckgrndObj, bBObjsVisible);
                        pActualPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
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
                    GetDoc()->ClearDeletedPresObjList();

                    GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHPAGE,
                                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

                    BOOL bSetModified = TRUE;

                    if (pArgs && pArgs->Count() == 1)
                    {
                        bSetModified = (BOOL) ((SfxBoolItem&) pArgs->Get(SID_MODIFYPAGE)).GetValue();
                    }

                    GetDoc()->SetChanged(bSetModified);
                }
                */
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_RENAMEPAGE:
        {
            if (ePageKind==PK_STANDARD || ePageKind==PK_NOTES )
            {
                if ( pDrView->IsTextEdit() )
                {
                    pDrView->EndTextEdit();
                }

                USHORT nPageId = aTabControl.GetCurPageId();
                SdPage* pCurrentPage = ( GetEditMode() == EM_PAGE )
                    ? GetDoc()->GetSdPage( nPageId - 1, GetPageKind() )
                    : GetDoc()->GetMasterSdPage( nPageId - 1, GetPageKind() );

                String aTitle( SdResId( STR_TITLE_RENAMESLIDE ) );
                String aDescr( SdResId( STR_DESC_RENAMESLIDE ) );
                String aPageName = pCurrentPage->GetName();

                //CHINA001 SvxNameDialog aNameDlg( GetActiveWindow(), aPageName, aDescr );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
                AbstractSvxNameDialog* aNameDlg = pFact->CreateSvxNameDialog( GetActiveWindow(), aPageName, aDescr, ResId(RID_SVXDLG_NAME) );
                DBG_ASSERT(aNameDlg, "Dialogdiet fail!");//CHINA001
                //CHINA001 aNameDlg.SetText( aTitle );
                //CHINA001 aNameDlg.SetCheckNameHdl( LINK( this, SdDrawViewShell, RenameSlideHdl ), true );
                //CHINA001 aNameDlg.SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );
                aNameDlg->SetText( aTitle );
                aNameDlg->SetCheckNameHdl( LINK( this, DrawViewShell, RenameSlideHdl ), true );
                aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

                if( aNameDlg->Execute() == RET_OK ) //CHINA001 if( aNameDlg.Execute() == RET_OK )
                {
                    String aNewName;
                    aNameDlg->GetName( aNewName ); //CHINA001 aNameDlg.GetName( aNewName );
                    if( ! aNewName.Equals( aPageName ) )
                    {
                        bool bResult = RenameSlide( nPageId, aNewName );
                        DBG_ASSERT( bResult, "Couldn't rename slide" );
                    }
                }
                delete aNameDlg; //add by CHINA001
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_RENAMEPAGE_QUICK:
        {
            if (ePageKind==PK_STANDARD || ePageKind==PK_NOTES )
            {
                if ( pDrView->IsTextEdit() )
                {
                    pDrView->EndTextEdit();
                }

                aTabControl.StartEditMode( aTabControl.GetCurPageId() );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_PAGESIZE :  // entweder dieses (kein menueeintrag o. ae. !!)
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if (pArgs)
                if (pArgs->Count () == 3)
                {
                    SFX_REQUEST_ARG (rReq, pWidth, SfxUInt32Item, ID_VAL_PAGEWIDTH, FALSE);
                    SFX_REQUEST_ARG (rReq, pHeight, SfxUInt32Item, ID_VAL_PAGEHEIGHT, FALSE);
                    SFX_REQUEST_ARG (rReq, pScaleAll, SfxBoolItem, ID_VAL_SCALEOBJECTS, FALSE);

                    Size aSize (pWidth->GetValue (), pHeight->GetValue ());

                    SetupPage (aSize, 0, 0, 0, 0, TRUE, FALSE, pScaleAll->GetValue ());
                    rReq.Ignore ();
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            rReq.Ignore ();
            break;
        }

        case SID_PAGEMARGIN :  // oder dieses (kein menueeintrag o. ae. !!)
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if (pArgs)
                if (pArgs->Count () == 5)
                {
                    SFX_REQUEST_ARG (rReq, pLeft, SfxUInt32Item, ID_VAL_PAGELEFT, FALSE);
                    SFX_REQUEST_ARG (rReq, pRight, SfxUInt32Item, ID_VAL_PAGERIGHT, FALSE);
                    SFX_REQUEST_ARG (rReq, pUpper, SfxUInt32Item, ID_VAL_PAGETOP, FALSE);
                    SFX_REQUEST_ARG (rReq, pLower, SfxUInt32Item, ID_VAL_PAGEBOTTOM, FALSE);
                    SFX_REQUEST_ARG (rReq, pScaleAll, SfxBoolItem, ID_VAL_SCALEOBJECTS, FALSE);

                    Size aEmptySize (0, 0);

                    SetupPage (aEmptySize, pLeft->GetValue (), pRight->GetValue (),
                               pUpper->GetValue (), pLower->GetValue (),
                               FALSE, TRUE, pScaleAll->GetValue ());
                    rReq.Ignore ();
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            rReq.Ignore ();
            break;
        }

        case SID_ZOOMING :  // kein Menueintrag, sondern aus dem Zoomdialog generiert
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (pArgs)
                if (pArgs->Count () == 1)
                {
                    SFX_REQUEST_ARG (rReq, pScale, SfxUInt32Item, ID_VAL_ZOOM, FALSE);
                    if (CHECK_RANGE (10, pScale->GetValue (), 1000))
                    {
                        SetZoom (pScale->GetValue ());

                        SfxBindings& rBindings = GetViewFrame()->GetBindings();
                        rBindings.Invalidate( SID_ATTR_ZOOM );
                        rBindings.Invalidate( SID_ZOOM_IN );
                        rBindings.Invalidate( SID_ZOOM_OUT );
                    }
                    else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

                    rReq.Ignore ();
                    break;
                }

            StarBASIC::FatalError (SbERR_WRONG_ARGS);
            rReq.Ignore ();
            break;
        }

        case SID_ATTR_ZOOM:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            bZoomOnPage = FALSE;

            if ( pArgs )
            {
                SvxZoomType eZT = ( ( const SvxZoomItem& ) pArgs->
                                            Get( SID_ATTR_ZOOM ) ).GetType();
                switch( eZT )
                {
                    case SVX_ZOOM_PERCENT:
                        SetZoom( (long) ( ( const SvxZoomItem& ) pArgs->
                                            Get( SID_ATTR_ZOOM ) ).GetValue() );
                        break;

                    case SVX_ZOOM_OPTIMAL:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                        break;

                    case SVX_ZOOM_PAGEWIDTH:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                        break;

                    case SVX_ZOOM_WHOLEPAGE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
                        break;
                }
                rReq.Ignore ();
            }
            else
            {
                // hier den Zoom-Dialog oeffnen
                pFuActual = new FuScale( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
            }
            Cancel();
        }
        break;

        case SID_CHANGEBEZIER:
        case SID_CHANGEPOLYGON:
            if ( pDrView->IsTextEdit() )
            {
                pDrView->EndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }

            if ( pDrView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                if( rReq.GetSlot() == SID_CHANGEBEZIER )
                {
                    WaitObject aWait( (Window*)GetActiveWindow() );
                    pDrView->ConvertMarkedToPathObj(FALSE);
                }
                else
                {
                    if( pDrView->IsVectorizeAllowed() )
                        pFuActual = new FuVectorize( this, GetActiveWindow(), pDrView, GetDoc(), rReq );
                    else
                    {
                        WaitObject aWait( (Window*)GetActiveWindow() );
                        pDrView->ConvertMarkedToPolyObj(FALSE);
                    }
                }

                Invalidate(SID_CHANGEBEZIER);
                Invalidate(SID_CHANGEPOLYGON);
            }
            Cancel();

            if ( pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT )
            {   // ggf. die richtige Editfunktion aktivieren
                GetViewFrame()->GetDispatcher()->Execute(SID_SWITCH_POINTEDIT,
                                        SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
            rReq.Ignore ();
            break;

        case SID_CONVERT_TO_CONTOUR:
            if ( pDrView->IsTextEdit() )
            {
                pDrView->EndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }

            if ( pDrView->IsPresObjSelected() )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );
                pDrView->ConvertMarkedToPolyObj(TRUE);

                Invalidate(SID_CONVERT_TO_CONTOUR);
            }
            Cancel();

            rReq.Ignore ();
            break;

        case SID_CONVERT_TO_METAFILE:
        case SID_CONVERT_TO_BITMAP:
        {
            // End text edit mode when it is active because the metafile or
            // bitmap that will be created does not support it.
            if ( pDrView->IsTextEdit() )
            {
                pDrView->EndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }

            if ( pDrView->IsPresObjSelected(true,true,true) )
            {
                ::sd::Window* pWindow = GetActiveWindow();
                InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
            }
            else
            {
                WaitObject aWait( (Window*)GetActiveWindow() );

                // switch on undo for the next operations
                pDrView->BegUndo(
                    String(
                    SdResId (nSId==SID_CONVERT_TO_METAFILE ? STR_UNDO_CONVERT_TO_METAFILE : STR_UNDO_CONVERT_TO_BITMAP)));

                // create SdrGrafObj from metafile/bitmap
                Graphic aGraphic;
                switch (nSId)
                {
                    case SID_CONVERT_TO_METAFILE:
                    {
                        GDIMetaFile aMetaFile(pDrView->GetAllMarkedMetaFile ());
                        aGraphic = Graphic(aMetaFile);
                    }
                    break;
                    case SID_CONVERT_TO_BITMAP:
                    {
                        Bitmap aBitmap (pDrView->GetAllMarkedBitmap ());
                        aGraphic = Graphic(aBitmap);
                    }
                    break;
                }

                SdrGrafObj* pGraphicObj = new SdrGrafObj (aGraphic);

                // fit rectangle of graphic-object to  mark-rect
                Rectangle aAllMarkedRect;
                SdrMarkList aMarkList = pDrView->GetMarkedObjectList();
                for (int i=0; i < (int) aMarkList.GetMarkCount(); i++)
                {
                    aAllMarkedRect.Union ( aMarkList.GetMark(i)->GetObj()->GetCurrentBoundRect() );
                }
                pGraphicObj->SetLogicRect (aAllMarkedRect);

                // get page-view
                SdrPageView* pPageView = pDrView->GetMarkedObjectList().GetMark(0)->GetPageView();

                // delete marked objects
                pDrView->DeleteMarkedObj(); // #69979# delete the objects, not only the marked area

                // insert new object
                pDrView->InsertObject (pGraphicObj, *pPageView);

                // switch off undo
                pDrView->EndUndo();
            }
        }

        Cancel();

        rReq.Done ();
        break;

        case SID_SET_DEFAULT:
        {
            SfxItemSet* pSet = NULL;

            if (pDrView->IsTextEdit())
            {
                ::Outliner* pOutl = pDrView->GetTextEditOutliner();
                if (pOutl)
                {
                    pOutl->RemoveFields(TRUE, (TypeId) SvxURLField::StaticType());
                }

                pSet = new SfxItemSet( GetPool(), EE_ITEMS_START, EE_ITEMS_END );
                pDrView->SetAttributes( *pSet, TRUE );
            }
            else
            {
                const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
                ULONG nCount = rMarkList.GetMarkCount();

                // In diese Liste werden fuer jedes Praesentationsobjekt ein SfxItemSet
                // der harten Attribute sowie der UserCall eingetragen, da diese beim nachfolgenden
                // pDrView->SetAttributes( *pSet, TRUE ) verloren gehen und spaeter restauriert
                // werden muessen
                List* pAttrList = new List();
                SdPage* pPresPage = (SdPage*) pDrView->GetPageViewPvNum(0)->GetPage();
                ULONG i;

                for ( i = 0; i < nCount; i++ )
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetObj();

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        SfxItemSet* pSet = new SfxItemSet( GetDoc()->GetPool(), SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT, 0 );
                        pSet->Put(pObj->GetMergedItemSet());
                        pAttrList->Insert( pSet, LIST_APPEND );
                        pAttrList->Insert( pObj->GetUserCall(), LIST_APPEND );
                    }
                }

                pSet = new SfxItemSet( GetPool() );
                pDrView->SetAttributes( *pSet, TRUE );

                ULONG j = 0;

                for ( i = 0; i < nCount; i++ )
                {
                    SfxStyleSheet* pSheet = NULL;
                    SdrObject* pObj = rMarkList.GetMark(i)->GetObj();

                    if (pObj->GetObjIdentifier() == OBJ_TITLETEXT)
                    {
                        pSheet = pActualPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
                        if (pSheet)
                            pObj->SetStyleSheet(pSheet, FALSE);
                    }
                    else if(pObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
                    {
                        for (USHORT nLevel = 1; nLevel < 10; nLevel++)
                        {
                            pSheet = pActualPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
                            DBG_ASSERT(pSheet, "Vorlage fuer Gliederungsobjekt nicht gefunden");
                            if (pSheet)
                            {
                                pObj->StartListening(*pSheet);

                                if( nLevel == 1 )
                                    // Textrahmen hoert auf StyleSheet der Ebene1
                                    pObj->NbcSetStyleSheet(pSheet, FALSE);

                            }
                        }
                    }

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        SfxItemSet* pSet = (SfxItemSet*) pAttrList->GetObject(j++);
                        SdrObjUserCall* pUserCall = (SdrObjUserCall*) pAttrList->GetObject(j++);

                        if ( pSet && pSet->GetItemState( SDRATTR_TEXT_MINFRAMEHEIGHT ) == SFX_ITEM_ON )
                        {
                            pObj->SetMergedItem(pSet->Get(SDRATTR_TEXT_MINFRAMEHEIGHT));
                        }

                        if ( pSet && pSet->GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) == SFX_ITEM_ON )
                        {
                            pObj->SetMergedItem(pSet->Get(SDRATTR_TEXT_AUTOGROWHEIGHT));
                        }

                        if( pUserCall )
                            pObj->SetUserCall( pUserCall );

                        delete pSet;
                    }
                }

                delete pAttrList;
            }

            delete pSet;
            Cancel();
        }
        break;

        case SID_DELETE_SNAPITEM:
        {
            SdrPageView* pPV;
            Point   aMPos = GetActiveWindow()->PixelToLogic( aMousePos );
            USHORT  nHitLog = (USHORT) GetActiveWindow()->PixelToLogic( Size(
                FuPoor::HITPIX, 0 ) ).Width();
            USHORT  nHelpLine;

            bMousePosFreezed = FALSE;

            if( pDrView->PickHelpLine( aMPos, nHitLog, *GetActiveWindow(), nHelpLine, pPV) )
            {
                pPV->DeleteHelpLine( nHelpLine );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_DELETE_PAGE:
            DeleteActualPage();
            Cancel();
            rReq.Ignore ();
        break;

        case SID_DELETE_LAYER:
            DeleteActualLayer();
            Cancel();
            rReq.Ignore ();
        break;

        case SID_ORIGINAL_SIZE:
            pDrView->SetMarkedOriginalSize();
            Cancel();
            rReq.Done();
        break;

        case SID_DRAW_FONTWORK:
        case SID_DRAW_FONTWORK_VERTICAL:
        {
            svx::FontworkBar::execute(mpView, nSId );
            Cancel();
            rReq.Done();
        }
        break;

        default:
        {
            // switch Anweisung wegen CLOOKS aufgeteilt. Alle case-Anweisungen die
            // eine Fu???? -Funktion aufrufen, sind in die Methode FuTemp01 (drviews8)
            // gewandert.
            FuTemp01(rReq);
        }
        break;
    }

    if (pFuActual)
    {
        pFuActual->Activate();
    }
}




/** This method consists basically of three parts:
    1. Process the arguments of the SFX request.
    2. Use the model to create a new page or duplicate an existing one.
    3. Update the tab control and switch to the new page.
*/
void DrawViewShell::CreateOrDuplicatePage (
    SfxRequest& rRequest,
    PageKind ePageKind,
    SdPage* pPage)
{
    if (ePageKind == PK_STANDARD && eEditMode != EM_MASTERPAGE)
    {
        if ( pDrView->IsTextEdit() )
        {
            pDrView->EndTextEdit();
        }
        ViewShell::CreateOrDuplicatePage (rRequest, ePageKind, pPage);
    }
}

} // end of namespace sd
