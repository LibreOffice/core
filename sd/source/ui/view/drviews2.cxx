/*************************************************************************
 *
 *  $RCSfile: drviews2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:43 $
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
#ifndef _IDETEMP_HXX //autogen
#include <basctl/idetemp.hxx>
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

#pragma hdrstop

#include "app.hrc"
#include "glob.hrc"

#include "sdattr.hxx"
#include "drawview.hxx"
#include "sdwindow.hxx"
#include "ins_page.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "sdpage.hxx"
#include "preview.hxx"
#include "fuscale.hxx"
#include "sdresid.hxx"
#include "new_foil.hxx"
#include "drviewsh.hxx"
#include "grviewsh.hxx"
#include "unmodpg.hxx"
#include "fuslshow.hxx"
#include "fuvect.hxx"
#include "stlpool.hxx"


/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void SdDrawViewShell::FuTemporary(SfxRequest& rReq)
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
                SfxUndoManager* pUndoMgr = pDocSh->GetUndoManager();

                // Anpassungen Start/EndWidth #63083#
                if(nSId == SID_ATTR_LINE_WIDTH)
                {
                    SdrObject* pObj = NULL;
                    const SdrMarkList& rMarkList = pDrView->GetMarkList();
                    ULONG nCount = rMarkList.GetMarkCount();

                    INT32 nNewLineWidth = ((const XLineWidthItem&)rReq.GetArgs()->Get(XATTR_LINEWIDTH)).GetValue();

                    for (ULONG i=0; i<nCount; i++)
                    {
                        SfxItemSet aAttr(pDoc->GetPool());
                        pObj = rMarkList.GetMark(i)->GetObj();
                        // Mergen: TRUE, OnlyHardAttr: FALSE
                        pObj->TakeAttributes(aAttr, TRUE, FALSE);

                        INT32 nActLineWidth = ((const XLineWidthItem&)aAttr.Get(XATTR_LINEWIDTH)).GetValue();

                        if(nActLineWidth != nNewLineWidth)
                        {
                            BOOL bSetItemSet(FALSE);

                            if(aAttr.GetItemState(XATTR_LINESTARTWIDTH) == SFX_ITEM_SET)
                            {
                                INT32 nValAct = ((const XLineStartWidthItem&)aAttr.Get(XATTR_LINESTARTWIDTH)).GetValue();
                                INT32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
                                if(nValNew < 0)
                                    nValNew = 0;
                                bSetItemSet = TRUE;
                                aAttr.Put(XLineStartWidthItem(nValNew));
                            }

                            if(aAttr.GetItemState(XATTR_LINEENDWIDTH) == SFX_ITEM_SET)
                            {
                                INT32 nValAct = ((const XLineEndWidthItem&)aAttr.Get(XATTR_LINEENDWIDTH)).GetValue();
                                INT32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
                                if(nValNew < 0)
                                    nValNew = 0;
                                bSetItemSet = TRUE;
                                aAttr.Put(XLineEndWidthItem(nValNew));
                            }

                            if(bSetItemSet)
                                pObj->NbcSetAttributes(aAttr, FALSE);
                        }
                    }
                }

                if (nSId == SID_ATTR_FILL_SHADOW)
                {
                    // Ggf. werden transparente Objekte weiá gefuellt
                    SdrObject* pObj = NULL;
                    const SdrMarkList& rMarkList = pDrView->GetMarkList();
                    ULONG nCount = rMarkList.GetMarkCount();

                    for (ULONG i=0; i<nCount; i++)
                    {
                        SfxItemSet aAttr(pDoc->GetPool());
                        pObj = rMarkList.GetMark(i)->GetObj();
                        // Mergen: TRUE, OnlyHardAttr: FALSE
                        pObj->TakeAttributes(aAttr, TRUE, FALSE);

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
                            pObj->NbcSetAttributes(aAttr, FALSE);
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
        {
            if (ePageKind == PK_STANDARD && eEditMode!=EM_MASTERPAGE)
            {
                if ( pDrView->IsTextEdit() )
                {
                    pDrView->EndTextEdit();
                }

                USHORT nPageCount = pDoc->GetSdPageCount(ePageKind);
                SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
                BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
                USHORT nPos = 0;
                SetOfByte aVisibleLayers = pActualPage->GetMasterPageVisibleLayers(nPos);

                USHORT nStandardPageNum;
                USHORT nNotesPageNum;
                SdPage* pPreviousStandardPage;
                SdPage* pPreviousNotesPage;
                String aStandardPageName;
                String aNotesPageName;
                AutoLayout eStandardLayout;
                AutoLayout eNotesLayout;
                BOOL bIsPageBack;
                BOOL bIsPageObj;

                const SfxItemSet* pArgs = rReq.GetArgs();

                if (! pArgs)
                {
                    SfxItemSet aAttrSet( GetPool(), ATTR_PAGE_START, ATTR_PAGE_END );
                    String aStr;
                    aAttrSet.Put( SfxStringItem( ATTR_PAGE_NAME, aStr ) );
                    aAttrSet.Put( SfxBoolItem( ATTR_PAGE_BACKGROUND,
                                               aVisibleLayers.IsSet(aBckgrnd) ) );
                    aAttrSet.Put( SfxBoolItem( ATTR_PAGE_OBJECTS,
                                               aVisibleLayers.IsSet(aBckgrndObj) ) );

                    AutoLayout eAutoLayout = pActualPage->GetAutoLayout();

                    if (eAutoLayout == AUTOLAYOUT_TITLE && pActualPage->GetPageNum() == 1)
                    {
                        // 1.Seite ist TitelDia
                        eAutoLayout = AUTOLAYOUT_ENUM;
                    }

                    aAttrSet.Put( SfxAllEnumItem( ATTR_PAGE_LAYOUT,
                                                  eAutoLayout ) );

                    SdNewFoilDlg* pDlg = NULL;

                    if (nSId == SID_INSERTPAGE && !this->ISA( SdGraphicViewShell ))
                        pDlg = new SdNewFoilDlg(NULL, aAttrSet, ePageKind, pDocSh, FALSE);

                    if (pDlg && pDlg->Execute () != RET_OK)
                    {
                        Cancel();

                        if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT )
                            GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

                        delete pDlg;
                        rReq.Ignore ();
                        break;
                    }
                    else
                    {
                        // AutoLayouts muessen fertig sein
                        pDoc->StopWorkStartupDelay();

                        if (pDlg)
                        {
                            pDlg->GetAttr( aAttrSet );
                        }

                        if (ePageKind == PK_NOTES)
                        {
                            aNotesPageName = ((const SfxStringItem &) aAttrSet.Get (ATTR_PAGE_NAME)).GetValue ();
                            eNotesLayout   = (AutoLayout) ((const SfxAllEnumItem &)
                                                 aAttrSet.Get (ATTR_PAGE_LAYOUT)).GetValue ();
                        }
                        else
                        {
                            aStandardPageName = ((const SfxStringItem &) aAttrSet.Get (ATTR_PAGE_NAME)).GetValue ();
                            eStandardLayout   = (AutoLayout) ((const SfxAllEnumItem &)
                                                    aAttrSet.Get (ATTR_PAGE_LAYOUT)).GetValue ();
                        }

                        bIsPageBack = ((const SfxBoolItem &) aAttrSet.Get (ATTR_PAGE_BACKGROUND)).GetValue ();
                        bIsPageObj  = ((const SfxBoolItem &) aAttrSet.Get (ATTR_PAGE_OBJECTS)).GetValue();

                        pDoc->SetChanged(TRUE);
                    }

                    delete pDlg;
                }
                else if (pArgs->Count () != 4)
                {
                    Cancel();

                    if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT )
                        GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

                    StarBASIC::FatalError (SbERR_WRONG_ARGS);
                    rReq.Ignore ();
                    break;
                }
                else
                {
                    // AutoLayouts muessen fertig sein
                    pDoc->StopWorkStartupDelay();

                    SFX_REQUEST_ARG (rReq, pPageName, SfxStringItem, ID_VAL_PAGENAME, FALSE);
                    SFX_REQUEST_ARG (rReq, pLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, FALSE);
                    SFX_REQUEST_ARG (rReq, pIsPageBack, SfxBoolItem, ID_VAL_ISPAGEBACK, FALSE);
                    SFX_REQUEST_ARG (rReq, pIsPageObj, SfxBoolItem, ID_VAL_ISPAGEOBJ, FALSE);

                    if (CHECK_RANGE (AUTOLAYOUT_TITLE, (AutoLayout) pLayout->GetValue (), AUTOLAYOUT_HANDOUT6))
                    {
                        if (ePageKind == PK_NOTES)
                        {
                            aNotesPageName = pPageName->GetValue ();
                            eNotesLayout   = (AutoLayout) pLayout->GetValue ();
                        }
                        else
                        {
                            aStandardPageName = pPageName->GetValue ();
                            eStandardLayout   = (AutoLayout) pLayout->GetValue ();
                        }

                        bIsPageBack = pIsPageBack->GetValue ();
                        bIsPageObj  = pIsPageObj->GetValue ();
                    }
                    else
                    {
                        Cancel();

                        if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT )
                            GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

                        StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
                        rReq.Ignore ();
                        break;
                    }
                }

                /**************************************************************
                * Es wird stets zuerst eine Standardseite und dann eine
                * Notizseite erzeugt. Es ist sichergestellt, dass auf eine
                * Standardseite stets die zugehoerige Notizseite folgt.
                **************************************************************/
                if (ePageKind == PK_NOTES)
                {
                    pPreviousNotesPage = pActualPage;
                    nNotesPageNum = pPreviousNotesPage->GetPageNum() + 2;
                    pPreviousStandardPage = (SdPage*) pDoc->GetPage(nNotesPageNum - 3);
                    nStandardPageNum = nNotesPageNum - 1;
                    eStandardLayout = pPreviousStandardPage->GetAutoLayout();
                }
                else
                {
                    pPreviousStandardPage = pActualPage;
                    nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
                    pPreviousNotesPage = (SdPage*) pDoc->GetPage(nStandardPageNum - 1);
                    nNotesPageNum = nStandardPageNum + 1;
                    aNotesPageName = aStandardPageName;
                    eNotesLayout = pPreviousNotesPage->GetAutoLayout();
                }

                pDrView->BegUndo( String( SdResId(STR_INSERTPAGE) ) );

                /**************************************************************
                * Standardseite
                **************************************************************/
                SdPage* pStandardPage = NULL;

                if (nSId == SID_DUPLICATE_PAGE)
                {
                    pStandardPage = (SdPage*) pPreviousStandardPage->Clone();
                }
                else
                {
                    pStandardPage = (SdPage*) pDoc->AllocPage(FALSE);
                }

                pStandardPage->SetSize( pPreviousStandardPage->GetSize() );
                pStandardPage->SetBorder( pPreviousStandardPage->GetLftBorder(),
                                          pPreviousStandardPage->GetUppBorder(),
                                          pPreviousStandardPage->GetRgtBorder(),
                                          pPreviousStandardPage->GetLwrBorder() );

                pStandardPage->SetName(aStandardPageName);

                // Seite hinter aktueller Seite einfuegen
                pDoc->InsertPage(pStandardPage, nStandardPageNum);
                pDrView->AddUndo(new SdrUndoNewPage(*pStandardPage));

                if (nSId != SID_DUPLICATE_PAGE)
                {
                    // MasterPage der aktuellen Seite verwenden
                    USHORT nPgNum = pPreviousStandardPage->GetMasterPageNum(nPos=0);
                    pStandardPage->InsertMasterPage(nPgNum);
                    pStandardPage->SetLayoutName( pPreviousStandardPage->GetLayoutName() );
                    pStandardPage->SetAutoLayout(eStandardLayout, TRUE);
                }

                aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
                aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
                aVisibleLayers.Set(aBckgrnd, bIsPageBack);
                aVisibleLayers.Set(aBckgrndObj, bIsPageObj);
                pStandardPage->SetMasterPageVisibleLayers(aVisibleLayers, nPos=0);

                /**************************************************************
                * Notizseite
                **************************************************************/
                SdPage* pNotesPage = NULL;

                if (nSId == SID_DUPLICATE_PAGE)
                {
                    pNotesPage = (SdPage*) pPreviousNotesPage->Clone();
                }
                else
                {
                    pNotesPage = (SdPage*) pDoc->AllocPage(FALSE);
                }

                pNotesPage->SetSize( pPreviousNotesPage->GetSize() );
                pNotesPage->SetBorder( pPreviousNotesPage->GetLftBorder(),
                                       pPreviousNotesPage->GetUppBorder(),
                                       pPreviousNotesPage->GetRgtBorder(),
                                       pPreviousNotesPage->GetLwrBorder() );
                pNotesPage->SetName(aNotesPageName);
                pNotesPage->SetPageKind(PK_NOTES);

                // Seite hinter aktueller Seite einfuegen
                pDoc->InsertPage(pNotesPage, nNotesPageNum);
                pDrView->AddUndo(new SdrUndoNewPage(*pNotesPage));

                if (nSId != SID_DUPLICATE_PAGE)
                {
                    // MasterPage der aktuellen Seite verwenden
                    USHORT nPgNum = pPreviousNotesPage->GetMasterPageNum(nPos=0);
                    pNotesPage->InsertMasterPage(nPgNum);
                    pNotesPage->SetLayoutName( pPreviousNotesPage->GetLayoutName() );
                    pNotesPage->SetAutoLayout(eNotesLayout, TRUE);
                }

                aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
                aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
                aVisibleLayers.Set(aBckgrnd, bIsPageBack);
                aVisibleLayers.Set(aBckgrndObj, bIsPageObj);
                pNotesPage->SetMasterPageVisibleLayers(aVisibleLayers, nPos=0);

                pDrView->EndUndo();

                // Update fuer TabControl
                aTabControl.Clear();

                SdPage* pPage;
                USHORT nActualPageNum;
                String aPageName;
                USHORT nPageCnt = pDoc->GetSdPageCount(ePageKind);

                for (USHORT i = 0; i < nPageCnt; i++)
                {
                    pPage = pDoc->GetSdPage(i, ePageKind);

                    aPageName = pPage->GetName();
                    aTabControl.InsertPage(i + 1, aPageName);

                    if (ePageKind==PK_STANDARD && pPage==pStandardPage ||
                        ePageKind==PK_NOTES    && pPage==pNotesPage )
                    {
                        nActualPageNum = i;
                    }
                }

                aTabControl.SetCurPageId(nActualPageNum + 1);

                GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHPAGE,
                                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }

            Cancel();

            if (pFuActual && pFuActual->GetSlotID() == SID_BEZIER_EDIT )
                    GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            rReq.Done ();
        }
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
                pActualPage = pDoc->GetSdPage(nPage, ePageKind);
                SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
                BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
                BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
                USHORT nPos = 0;
                SetOfByte aVisibleLayers = pActualPage->GetMasterPageVisibleLayers(nPos);
                BOOL bHandoutMode = FALSE;
                SdPage* pHandoutMPage = NULL;
                String aNewName;
                AutoLayout aNewAutoLayout;
                BOOL bBVisible;
                BOOL bBObjsVisible;
                const SfxItemSet* pArgs = rReq.GetArgs();

                if (!pArgs || pArgs->Count() == 1 || pArgs->Count() == 2 )
                {
                    SfxItemSet aAttrSet( GetPool(), ATTR_PAGE_START, ATTR_PAGE_END );

                    aAttrSet.Put( SfxStringItem( ATTR_PAGE_NAME,
                                                 pActualPage->GetName() ) );
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
                            pHandoutMPage = pDoc->GetMasterSdPage(0, PK_HANDOUT);
                            eNewAutoLayout = pHandoutMPage->GetAutoLayout();
                        }
                    }

                    aAttrSet.Put( SfxAllEnumItem( ATTR_PAGE_LAYOUT, eNewAutoLayout ) );

                    SdNewFoilDlg* pDlg = new SdNewFoilDlg(pWindow, aAttrSet, ePageKind, pDocSh, TRUE);

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
                        SdPage* pPage = pDoc->GetSdPage(0, PK_STANDARD);
                        if (pDoc->GetSdPageCount(PK_STANDARD) == 1 &&
                            pPage->GetAutoLayout() == AUTOLAYOUT_TITLE &&
                            pPage->GetPresObjList()->Count() == 0)
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

                ModifyPageUndoAction* pAction = new ModifyPageUndoAction(
                                                    pDoc, pUndoPage,
                                                    aNewName,
                                                    aNewAutoLayout,
                                                    bBVisible,
                                                    bBObjsVisible);
                pDocSh->GetUndoManager()->AddUndoAction(pAction);

                SfxChildWindow* pPreviewChildWindow = GetViewFrame()->GetChildWindow( SdPreviewChildWindow::GetChildWindowId() );
                SdPreviewWin*   pPreviewWin = NULL;

                // notify preview slide show are changes are to be done
                if( pPreviewChildWindow && ( ( pPreviewWin = (SdPreviewWin*) pPreviewChildWindow->GetWindow() ) != NULL ) )
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
                            SdPage* pNotesPage = pDoc->GetSdPage(nPage, PK_NOTES);
                            pNotesPage->SetName(aNewName);
                        }
                    }

                    pActualPage->SetAutoLayout(aNewAutoLayout, TRUE);

                    aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
                    aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
                    aVisibleLayers.Set(aBckgrnd, bBVisible);
                    aVisibleLayers.Set(aBckgrndObj, bBObjsVisible);
                    nPos = 0;
                    pActualPage->SetMasterPageVisibleLayers(aVisibleLayers, nPos);
                }
                else
                {
                    pHandoutMPage->SetAutoLayout(aNewAutoLayout, TRUE);
                }

                GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHPAGE,
                                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

                BOOL bSetModified = TRUE;

                if (pArgs && pArgs->Count() == 1)
                {
                    bSetModified = (BOOL) ((SfxBoolItem&) pArgs->Get(SID_MODIFYPAGE)).GetValue();
                }

                pDoc->SetChanged(bSetModified);
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
                        SFX_BINDINGS().Invalidate( SID_ATTR_ZOOM );
                        SFX_BINDINGS().Invalidate( SID_ZOOM_IN );
                        SFX_BINDINGS().Invalidate( SID_ZOOM_OUT );
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
                pFuActual = new FuScale( this, pWindow, pDrView, pDoc, rReq );
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
                        pFuActual = new FuVectorize( this, pWindow, pDrView, pDoc, rReq );
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
            if ( pDrView->IsPresObjSelected() )
            {
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
                SdrMarkList aMarkList = pDrView->GetMarkList();
                for (int i=0; i<aMarkList.GetMarkCount(); i++)
                {
                    aAllMarkedRect.Union ( aMarkList.GetMark(i)->GetObj()->GetBoundRect() );
                }
                pGraphicObj->SetLogicRect (aAllMarkedRect);

                // get page-view
                SdrPageView* pPageView = pDrView->GetMarkList().GetMark(0)->GetPageView();

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
                Outliner* pOutl = pDrView->GetTextEditOutliner();
                if (pOutl)
                {
                    pOutl->RemoveFields(TRUE, (TypeId) SvxURLField::StaticType());
                }

                pSet = new SfxItemSet( GetPool(), EE_ITEMS_START, EE_ITEMS_END );
                pDrView->SetAttributes( *pSet, TRUE );
            }
            else
            {
                const SdrMarkList& rMarkList = pDrView->GetMarkList();
                ULONG nCount = rMarkList.GetMarkCount();

                // In diese Liste werden fuer jedes Praesentationsobjekt ein SfxItemSet
                // der harten Attribute sowie der UserCall eingetragen, da diese beim nachfolgenden
                // pDrView->SetAttributes( *pSet, TRUE ) verloren gehen und spaeter restauriert
                // werden muessen
                List* pAttrList = new List();
                List* pPresObjList = ( (SdPage*) pDrView->GetPageViewPvNum(0)->GetPage() )
                                                                    ->GetPresObjList();

                for ( ULONG i = 0; i < nCount; i++ )
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetObj();

                    if( pPresObjList->GetPos( pObj ) != LIST_ENTRY_NOTFOUND )
                    {
                        SfxItemSet* pSet = new SfxItemSet( pDoc->GetPool(), SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT, 0 );
                        pObj->TakeAttributes( *pSet, TRUE, TRUE );
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

                    if( pPresObjList->GetPos( pObj ) != LIST_ENTRY_NOTFOUND )
                    {
                        SfxItemSet* pSet = (SfxItemSet*) pAttrList->GetObject(j++);
                        SdrObjUserCall* pUserCall = (SdrObjUserCall*) pAttrList->GetObject(j++);

                        if ( pSet && pSet->GetItemState( SDRATTR_TEXT_MINFRAMEHEIGHT ) == SFX_ITEM_ON )
                        {
                            SdrTextMinFrameHeightItem aMinHeight( (const SdrTextMinFrameHeightItem&) pSet->Get(SDRATTR_TEXT_MINFRAMEHEIGHT) );
                            SfxItemSet aTempAttr( pDoc->GetPool(), SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_MINFRAMEHEIGHT, 0 );
                            aTempAttr.Put( aMinHeight );
                            pObj->NbcSetAttributes(aTempAttr, FALSE);
                        }

                        if ( pSet && pSet->GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) == SFX_ITEM_ON )
                        {
                            SdrTextAutoGrowHeightItem aAutoGrowHeight( (const SdrTextAutoGrowHeightItem&) pSet->Get(SDRATTR_TEXT_AUTOGROWHEIGHT) );
                            SfxItemSet aTempAttr( pDoc->GetPool(), SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT, 0 );
                            aTempAttr.Put( aAutoGrowHeight );
                            pObj->NbcSetAttributes(aTempAttr, FALSE);
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
            Point   aMPos = pWindow->PixelToLogic( aMousePos );
            USHORT  nHitLog = (USHORT) pWindow->PixelToLogic( Size( HITPIX, 0 ) ).Width();
            USHORT  nHelpLine;

            bMousePosFreezed = FALSE;

            if( pDrView->PickHelpLine( aMPos, nHitLog, *pWindow, nHelpLine, pPV) )
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


