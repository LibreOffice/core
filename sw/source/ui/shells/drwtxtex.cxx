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


#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <tools/shl.hxx>
#include <svx/svdview.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/svdoutl.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/whiter.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <svtools/langtab.hxx>
#include <svl/languageoptions.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/flditem.hxx>
#include <editeng/editstat.hxx>
#include <svx/hlnkitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svl/slstitm.hxx>
#include <editeng/langitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <vcl/window.hxx>
#include <editeng/editview.hxx>
#include <vcl/outdev.hxx>
#include <editeng/hyphenzoneitem.hxx>
#include <tools/diagnose_ex.h>

#include <cmdid.h>
#include <doc.hxx>
#include <docstat.hxx>
#include <drwtxtsh.hxx>
#include <edtwin.hxx>
#include <globals.hrc>
#include <hintids.hxx>
#include <initui.hxx>               // for SpellPointer
#include <langhelper.hxx>
#include <chrdlgmodes.hxx>
#include <pardlg.hxx>
#include <shells.hrc>
#include <string.h>
#include <swdtflvr.hxx>
#include <swmodule.hxx>
#include <swwait.hxx>
#include <uitool.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>
#include <wview.hxx>
#include <wordcountdialog.hxx>

#include "swabstdlg.hxx"
#include "chrdlg.hrc"
#include "misc.hrc"


using namespace ::com::sun::star;

void SwDrawTextShell::Execute( SfxRequest &rReq )
{
    SwWrtShell &rSh = GetShell();
    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemSet aNewAttr(*aEditAttr.GetPool(), aEditAttr.GetRanges());

    const sal_uInt16 nSlot = rReq.GetSlot();

    const sal_uInt16 nWhich = GetPool().GetWhich(nSlot);
    const SfxItemSet *pNewAttrs = rReq.GetArgs();

    bool bRestoreSelection = false;
    ESelection aOldSelection;

    sal_uInt16 nEEWhich = 0;
    switch (nSlot)
    {
        case SID_LANGUAGE_STATUS:
        {
            aOldSelection = pOLV->GetSelection();
            if (!pOLV->GetEditView().HasSelection())
            {
                bRestoreSelection   = true;
                pOLV->GetEditView().SelectCurrentWord();
            }

            bRestoreSelection = SwLangHelper::SetLanguageStatus(pOLV,rReq,GetView(),rSh);
            break;
        }

        case SID_THES:
        {
            String aReplaceText;
            SFX_REQUEST_ARG( rReq, pItem2, SfxStringItem, SID_THES, sal_False );
            if (pItem2)
                aReplaceText = pItem2->GetValue();
            if (aReplaceText.Len() > 0)
                ReplaceTextWithSynonym( pOLV->GetEditView(), aReplaceText );
            break;
        }

        case SID_ATTR_CHAR_FONT:
        case SID_ATTR_CHAR_FONTHEIGHT:
        case SID_ATTR_CHAR_WEIGHT:
        case SID_ATTR_CHAR_POSTURE:
        {
            SfxItemPool* pPool2 = aEditAttr.GetPool()->GetSecondaryPool();
            if( !pPool2 )
                pPool2 = aEditAttr.GetPool();
            SvxScriptSetItem aSetItem( nSlot, *pPool2 );

            // #i78017 establish the same behaviour as in Writer
            sal_uInt16 nScriptTypes = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;
            if (nSlot == SID_ATTR_CHAR_FONT)
                nScriptTypes = pOLV->GetSelectedScriptType();

            if (pNewAttrs)
            {
                aSetItem.PutItemForScriptType( nScriptTypes, pNewAttrs->Get( nWhich ) );
                aNewAttr.Put( aSetItem.GetItemSet() );
            }
        }
        break;

        case SID_ATTR_CHAR_COLOR: nEEWhich = EE_CHAR_COLOR; break;

        case SID_ATTR_CHAR_UNDERLINE:
        {
            if ( pNewAttrs )
            {
                const SvxTextLineItem& rTextLineItem = static_cast< const SvxTextLineItem& >( pNewAttrs->Get( pNewAttrs->GetPool()->GetWhich(nSlot) ) );
                aNewAttr.Put( SvxUnderlineItem( rTextLineItem.GetLineStyle(), EE_CHAR_UNDERLINE ) );
            }
            else
            {
                FontUnderline eFU = ((const SvxUnderlineItem&)aEditAttr.Get(EE_CHAR_UNDERLINE)).GetLineStyle();
                aNewAttr.Put( SvxUnderlineItem(eFU == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, EE_CHAR_UNDERLINE) );
            }
        }
        break;

        case SID_ATTR_CHAR_OVERLINE:
        {
             FontUnderline eFO = ((const SvxOverlineItem&)aEditAttr.Get(EE_CHAR_OVERLINE)).GetLineStyle();
            aNewAttr.Put(SvxOverlineItem(eFO == UNDERLINE_SINGLE ? UNDERLINE_NONE : UNDERLINE_SINGLE, EE_CHAR_OVERLINE));
        }
        break;

        case SID_ATTR_CHAR_CONTOUR:     nEEWhich = EE_CHAR_OUTLINE; break;
        case SID_ATTR_CHAR_SHADOWED:    nEEWhich = EE_CHAR_SHADOW; break;
        case SID_ATTR_CHAR_STRIKEOUT:   nEEWhich = EE_CHAR_STRIKEOUT; break;
        case SID_ATTR_CHAR_WORDLINEMODE: nEEWhich = EE_CHAR_WLM; break;
        case SID_ATTR_CHAR_RELIEF      : nEEWhich = EE_CHAR_RELIEF;  break;
        case SID_ATTR_CHAR_LANGUAGE    : nEEWhich = EE_CHAR_LANGUAGE;break;
        case SID_ATTR_CHAR_KERNING     : nEEWhich = EE_CHAR_KERNING; break;
        case SID_ATTR_CHAR_SCALEWIDTH:   nEEWhich = EE_CHAR_FONTWIDTH; break;
        case SID_ATTR_CHAR_AUTOKERN  :   nEEWhich = EE_CHAR_PAIRKERNING; break;
        case SID_ATTR_CHAR_ESCAPEMENT:   nEEWhich = EE_CHAR_ESCAPEMENT; break;
        case SID_ATTR_PARA_ADJUST_LEFT:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_LEFT, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_ADJUST_CENTER:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_CENTER, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_ADJUST_RIGHT:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_RIGHT, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_ADJUST_BLOCK:
            aNewAttr.Put(SvxAdjustItem(SVX_ADJUST_BLOCK, EE_PARA_JUST));
        break;
        case SID_ATTR_PARA_LRSPACE:
            {
                SvxLRSpaceItem aParaMargin((const SvxLRSpaceItem&)rReq.
                                        GetArgs()->Get(nSlot));
                aParaMargin.SetWhich( EE_PARA_LRSPACE );
                aNewAttr.Put(aParaMargin);
                rReq.Done();
            }
            break;
        case SID_ATTR_PARA_LINESPACE:
            {
                SvxLineSpacingItem aLineSpace = (const SvxLineSpacingItem&)pNewAttrs->Get(
                                                            GetPool().GetWhich(nSlot));
                aLineSpace.SetWhich( EE_PARA_SBL );
                aNewAttr.Put( aLineSpace );
                rReq.Done();
            }
            break;
        case SID_ATTR_PARA_ULSPACE:
            {
                SvxULSpaceItem aULSpace = (const SvxULSpaceItem&)pNewAttrs->Get(
                    GetPool().GetWhich(nSlot));
                aULSpace.SetWhich( EE_PARA_ULSPACE );
                aNewAttr.Put( aULSpace );
                rReq.Done();
            }
            break;

        case SID_ATTR_PARA_LINESPACE_10:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_ONE_LINE, EE_PARA_SBL);
            aItem.SetPropLineSpace(100);
            aNewAttr.Put(aItem);
        }
        break;
        case SID_ATTR_PARA_LINESPACE_15:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_ONE_POINT_FIVE_LINES, EE_PARA_SBL);
            aItem.SetPropLineSpace(150);
            aNewAttr.Put(aItem);
        }
        break;
        case SID_ATTR_PARA_LINESPACE_20:
        {
            SvxLineSpacingItem aItem(SVX_LINESPACE_TWO_LINES, EE_PARA_SBL);
            aItem.SetPropLineSpace(200);
            aNewAttr.Put(aItem);
        }
        break;

        case FN_SET_SUPER_SCRIPT:
        {
            SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
            SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

            if( eEsc == SVX_ESCAPEMENT_SUPERSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUPERSCRIPT );
            aNewAttr.Put( aItem, EE_CHAR_ESCAPEMENT );
        }
        break;
        case FN_SET_SUB_SCRIPT:
        {
            SvxEscapementItem aItem(EE_CHAR_ESCAPEMENT);
            SvxEscapement eEsc = (SvxEscapement ) ( (const SvxEscapementItem&)
                            aEditAttr.Get( EE_CHAR_ESCAPEMENT ) ).GetEnumValue();

            if( eEsc == SVX_ESCAPEMENT_SUBSCRIPT )
                aItem.SetEscapement( SVX_ESCAPEMENT_OFF );
            else
                aItem.SetEscapement( SVX_ESCAPEMENT_SUBSCRIPT );
            aNewAttr.Put( aItem, EE_CHAR_ESCAPEMENT );
        }
        break;

        case SID_CHAR_DLG_EFFECT:
        case SID_CHAR_DLG:
        case SID_CHAR_DLG_FOR_PARAGRAPH:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if( !pArgs )
            {
                aOldSelection = pOLV->GetSelection();
                if (nSlot == SID_CHAR_DLG_FOR_PARAGRAPH)
                {
                    // select current paragraph (and restore selection later on...)
                    EditView & rEditView = pOLV->GetEditView();
                    SwLangHelper::SelectPara( rEditView, rEditView.GetSelection() );
                    bRestoreSelection = true;
                }

                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );
                SfxItemSet aDlgAttr(GetPool(), EE_ITEMS_START, EE_ITEMS_END);

                // util::Language does not exists in the EditEngine! That is why not in set.

                aDlgAttr.Put( aEditAttr );
                aDlgAttr.Put( SvxKerningItem(0, RES_CHRATR_KERNING) );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                SfxAbstractTabDialog* pDlg = pFact->CreateSwCharDlg(pView->GetWindow(), *pView, aDlgAttr, DLG_CHAR_DRAW);
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                if (nSlot == SID_CHAR_DLG_EFFECT)
                {
                    pDlg->SetCurPageId(TP_CHAR_EXT);
                }
                sal_uInt16 nRet = pDlg->Execute();
                if(RET_OK == nRet )
                {
                    rReq.Done( *( pDlg->GetOutputItemSet() ) );
                    aNewAttr.Put(*pDlg->GetOutputItemSet());
                }
                delete( pDlg );
                if(RET_OK != nRet)
                    return ;
            }
            else
                aNewAttr.Put(*pArgs);
        }
        break;
        case FN_FORMAT_FOOTNOTE_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            VclAbstractDialog* pDlg = pFact->CreateSwFootNoteOptionDlg(GetView().GetWindow(), rView.GetWrtShell());
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
            break;
        }
        case FN_NUMBERING_OUTLINE_DLG:
        {
            SfxItemSet aTmp(GetPool(), FN_PARAM_1, FN_PARAM_1);
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "Dialogdiet fail!");
            SfxAbstractTabDialog* pDlg = pFact->CreateSwTabDialog( DLG_TAB_OUTLINE,
                                                        GetView().GetWindow(), &aTmp, GetView().GetWrtShell());
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
            rReq.Done();
        }
        break;
        case SID_OPEN_XML_FILTERSETTINGS:
        {
            try
            {
                uno::Reference < ui::dialogs::XExecutableDialog > xDialog = ui::dialogs::XSLTFilterDialog::create( ::comphelper::getProcessComponentContext() );
                xDialog->execute();
            }
            catch (const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            rReq.Ignore ();
        }
        break;
        case FN_WORDCOUNT_DIALOG:
        {
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if (pVFrame != NULL)
            {
                pVFrame->ToggleChildWindow(FN_WORDCOUNT_DIALOG);
                Invalidate(rReq.GetSlot());

                SwWordCountWrapper *pWrdCnt = (SwWordCountWrapper*)pVFrame->GetChildWindow(SwWordCountWrapper::GetChildWindowId());
                if (pWrdCnt)
                    pWrdCnt->UpdateCounts();
            }
        }
        break;
        case SID_PARA_DLG:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            if (!pArgs)
            {
                SwView* pView = &GetView();
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );
                SfxItemSet aDlgAttr(GetPool(),
                                    EE_ITEMS_START, EE_ITEMS_END,
                                    SID_ATTR_PARA_HYPHENZONE, SID_ATTR_PARA_HYPHENZONE,
                                    SID_ATTR_PARA_SPLIT, SID_ATTR_PARA_SPLIT,
                                    SID_ATTR_PARA_WIDOWS, SID_ATTR_PARA_WIDOWS,
                                    SID_ATTR_PARA_ORPHANS, SID_ATTR_PARA_ORPHANS,
                                    0);

                aDlgAttr.Put(aEditAttr);

                aDlgAttr.Put( SvxHyphenZoneItem( sal_False, RES_PARATR_HYPHENZONE) );
                aDlgAttr.Put( SvxFmtBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
                aDlgAttr.Put( SvxFmtSplitItem( sal_True, RES_PARATR_SPLIT ) );
                aDlgAttr.Put( SvxWidowsItem( 0, RES_PARATR_WIDOWS ) );
                aDlgAttr.Put( SvxOrphansItem( 0, RES_PARATR_ORPHANS ) );

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                SfxAbstractTabDialog* pDlg = pFact->CreateSwParaDlg( GetView().GetWindow(), GetView(), aDlgAttr,DLG_STD, 0, sal_True );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                sal_uInt16 nRet = pDlg->Execute();
                if(RET_OK == nRet)
                {
                    rReq.Done( *( pDlg->GetOutputItemSet() ) );
                    aNewAttr.Put(*pDlg->GetOutputItemSet());
                }
                delete( pDlg );
                if(RET_OK != nRet)
                    return;
            }
            else
                aNewAttr.Put(*pArgs);
        }
        break;
        case SID_AUTOSPELL_CHECK:
        {
//!! JP 16.03.2001: why??           pSdrView = rSh.GetDrawView();
//!! JP 16.03.2001: why??           pOutliner = pSdrView->GetTextEditOutliner();
            SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
            sal_uInt32 nCtrl = pOutliner->GetControlWord();

            sal_Bool bSet = ((const SfxBoolItem&)rReq.GetArgs()->Get(
                                                    nSlot)).GetValue();
            if(bSet)
                nCtrl |= EE_CNTRL_ONLINESPELLING|EE_CNTRL_ALLOWBIGOBJS;
            else
                nCtrl &= ~EE_CNTRL_ONLINESPELLING;
            pOutliner->SetControlWord(nCtrl);

            rView.ExecuteSlot(rReq);
        }
        break;
        case SID_HYPERLINK_SETLINK:
        {
            const SfxPoolItem* pItem = 0;
            if(pNewAttrs)
                pNewAttrs->GetItemState(nSlot, sal_False, &pItem);

            if(pItem)
            {
                const SvxHyperlinkItem& rHLinkItem = *(const SvxHyperlinkItem *)pItem;
                SvxURLField aFld(rHLinkItem.GetURL(), rHLinkItem.GetName(), SVXURLFORMAT_APPDEFAULT);
                aFld.SetTargetFrame(rHLinkItem.GetTargetFrame());

                const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();

                if (pFieldItem && pFieldItem->GetField()->ISA(SvxURLField))
                {
                    // Select field so that it will be deleted during insert
                    ESelection aSel = pOLV->GetSelection();
                    aSel.nEndPos++;
                    pOLV->SetSelection(aSel);
                }
                pOLV->InsertField(SvxFieldItem(aFld, EE_FEATURE_FIELD));
            }
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            // Shell switch!
            {
                SdrObject* pTmpObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
                SdrPageView* pTmpPV = pSdrView->GetSdrPageView();
                SdrView* pTmpView = pSdrView;

                pSdrView->SdrEndTextEdit(sal_True);

                SfxItemSet aAttr( *aNewAttr.GetPool(),
                            SDRATTR_TEXTDIRECTION,
                            SDRATTR_TEXTDIRECTION );

                aAttr.Put( SvxWritingModeItem(
                    nSlot == SID_TEXTDIRECTION_LEFT_TO_RIGHT ?
                        text::WritingMode_LR_TB
                        : text::WritingMode_TB_RL, SDRATTR_TEXTDIRECTION ) );
                pTmpView->SetAttributes( aAttr );

                rSh.GetView().BeginTextEdit( pTmpObj, pTmpPV, &rSh.GetView().GetEditWin(), sal_False);
                rSh.GetView().AttrChangedNotify( &rSh );
            }
            return;

        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            SdrObject* pTmpObj = pSdrView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
            SdrPageView* pTmpPV = pSdrView->GetSdrPageView();
            SdrView* pTmpView = pSdrView;

            pSdrView->SdrEndTextEdit(sal_True);
            bool bLeftToRight = nSlot == SID_ATTR_PARA_LEFT_TO_RIGHT;

            const SfxPoolItem* pPoolItem;
            if( pNewAttrs && SFX_ITEM_SET == pNewAttrs->GetItemState( nSlot, sal_True, &pPoolItem ) )
            {
                if( !( (SfxBoolItem*)pPoolItem)->GetValue() )
                    bLeftToRight = !bLeftToRight;
            }
            SfxItemSet aAttr( *aNewAttr.GetPool(),
                        EE_PARA_JUST, EE_PARA_JUST,
                        EE_PARA_WRITINGDIR, EE_PARA_WRITINGDIR,
                        0 );

            sal_uInt16 nAdjust = SVX_ADJUST_LEFT;
            if( SFX_ITEM_ON == aEditAttr.GetItemState(EE_PARA_JUST, sal_True, &pPoolItem ) )
                nAdjust = ( (SvxAdjustItem*)pPoolItem)->GetEnumValue();

            if( bLeftToRight )
            {
                aAttr.Put( SvxFrameDirectionItem( FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR ) );
                if( nAdjust == SVX_ADJUST_RIGHT )
                    aAttr.Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
            }
            else
            {
                aAttr.Put( SvxFrameDirectionItem( FRMDIR_HORI_RIGHT_TOP, EE_PARA_WRITINGDIR ) );
                if( nAdjust == SVX_ADJUST_LEFT )
                    aAttr.Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
            }
            pTmpView->SetAttributes( aAttr );
            rSh.GetView().BeginTextEdit( pTmpObj, pTmpPV, &rSh.GetView().GetEditWin(), sal_False );
            rSh.GetView().AttrChangedNotify( &rSh );
        }
        return;
        default:
            OSL_ENSURE(!this, "wrong dispatcher");
            return;
    }
    if(nEEWhich && pNewAttrs)
        aNewAttr.Put(pNewAttrs->Get(nWhich), nEEWhich);

    SetAttrToMarked(aNewAttr);

    GetView().GetViewFrame()->GetBindings().InvalidateAll(sal_False);

    if (IsTextEdit() && pOLV->GetOutliner()->IsModified())
        rSh.SetModified();

    if (bRestoreSelection)
    {
        // restore selection
        pOLV->GetEditView().SetSelection( aOldSelection );
    }
}

void SwDrawTextShell::GetState(SfxItemSet& rSet)
{
    if (!IsTextEdit())  // Otherwise sometimes crash!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    SfxItemSet aEditAttr( pOLV->GetAttribs() );
    const SfxPoolItem *pAdjust = 0, *pLSpace = 0, *pEscItem = 0;
    int eAdjust, nLSpace, nEsc;

    while(nWhich)
    {
        sal_uInt16 nSlotId = GetPool().GetSlotId( nWhich );
        sal_Bool bFlag = sal_False;
        switch( nSlotId )
        {
            case SID_LANGUAGE_STATUS://20412:
            {
                nSlotId = SwLangHelper::GetLanguageStatus(pOLV,rSet);
                break;
            }

            case SID_THES:
            {
                String          aStatusVal;
                LanguageType    nLang = LANGUAGE_NONE;
                bool bIsLookUpWord = GetStatusValueForThesaurusFromContext( aStatusVal, nLang, pOLV->GetEditView() );
                rSet.Put( SfxStringItem( SID_THES, aStatusVal ) );

                // disable "Thesaurus" context menu entry if there is nothing to look up
                uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
                if (!bIsLookUpWord ||
                    !xThes.is() || nLang == LANGUAGE_NONE || !xThes->hasLocale( LanguageTag::convertToLocale( nLang ) ))
                    rSet.DisableItem( SID_THES );

                //! avoid putting the same item as SfxBoolItem at the end of this function
                nSlotId = 0;
                break;
            }

        case SID_ATTR_PARA_ADJUST_LEFT:     eAdjust = SVX_ADJUST_LEFT; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_RIGHT:    eAdjust = SVX_ADJUST_RIGHT; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_CENTER:   eAdjust = SVX_ADJUST_CENTER; goto ASK_ADJUST;
        case SID_ATTR_PARA_ADJUST_BLOCK:    eAdjust = SVX_ADJUST_BLOCK; goto ASK_ADJUST;
ASK_ADJUST:
            {
                if( !pAdjust )
                    aEditAttr.GetItemState( EE_PARA_JUST, sal_False, &pAdjust);

                if( !pAdjust || IsInvalidItem( pAdjust ))
                    rSet.InvalidateItem( nSlotId ), nSlotId = 0;
                else
                    bFlag = eAdjust == ((SvxAdjustItem*)pAdjust)->GetAdjust();
            }
            break;

        case SID_ATTR_PARA_LRSPACE:
            {
                SfxItemState eState = aEditAttr.GetItemState(EE_PARA_LRSPACE);
                if( eState >= SFX_ITEM_DEFAULT )
                {
                    SvxLRSpaceItem aLR = (const SvxLRSpaceItem&) aEditAttr.Get( EE_PARA_LRSPACE );
                    aLR.SetWhich(SID_ATTR_PARA_LRSPACE);
                    rSet.Put(aLR);
                }
                else
                    rSet.InvalidateItem(nSlotId);
                nSlotId = 0;
            }
            break;
        case SID_ATTR_PARA_LINESPACE:
            {
                SfxItemState eState = aEditAttr.GetItemState(EE_PARA_SBL);
                if( eState >= SFX_ITEM_DEFAULT )
                {
                    SvxLineSpacingItem aLR = (const SvxLineSpacingItem&) aEditAttr.Get( EE_PARA_SBL );
                    rSet.Put(aLR);
                }
                else
                    rSet.InvalidateItem(nSlotId);
                nSlotId = 0;
            }
            break;
        case SID_ATTR_PARA_ULSPACE:
            {
                SfxItemState eState = aEditAttr.GetItemState(EE_PARA_ULSPACE);
                if( eState >= SFX_ITEM_DEFAULT )
                {
                    SvxULSpaceItem aULSpace = (const SvxULSpaceItem&) aEditAttr.Get( EE_PARA_ULSPACE );
                    aULSpace.SetWhich(SID_ATTR_PARA_ULSPACE);
                    rSet.Put(aULSpace);
                }
                else
                    rSet.InvalidateItem(nSlotId);
                nSlotId = 0;
            }
            break;

        case SID_ATTR_PARA_LINESPACE_10:    nLSpace = 100;  goto ASK_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_15:    nLSpace = 150;  goto ASK_LINESPACE;
        case SID_ATTR_PARA_LINESPACE_20:    nLSpace = 200;  goto ASK_LINESPACE;
ASK_LINESPACE:
            {
                if( !pLSpace )
                    aEditAttr.GetItemState( EE_PARA_SBL, sal_False, &pLSpace );

                if( !pLSpace || IsInvalidItem( pLSpace ))
                    rSet.InvalidateItem( nSlotId ), nSlotId = 0;
                else if( nLSpace == ((const SvxLineSpacingItem*)pLSpace)->
                                                GetPropLineSpace() )
                    bFlag = sal_True;
                else
                    nSlotId = 0;
            }
            break;

        case FN_SET_SUPER_SCRIPT:   nEsc = SVX_ESCAPEMENT_SUPERSCRIPT;
                                    goto ASK_ESCAPE;
        case FN_SET_SUB_SCRIPT:     nEsc = SVX_ESCAPEMENT_SUBSCRIPT;
                                    goto ASK_ESCAPE;
ASK_ESCAPE:
            {
                if( !pEscItem )
                    pEscItem = &aEditAttr.Get( EE_CHAR_ESCAPEMENT );

                if( nEsc == ((const SvxEscapementItem*)
                                                pEscItem)->GetEnumValue() )
                    bFlag = sal_True;
                else
                    nSlotId = 0;
            }
            break;

        case SID_THESAURUS:
        {
            // disable "Thesaurus" if the language is not supported
            const SfxPoolItem &rItem = GetShell().GetDoc()->GetDefault(
                            GetWhichOfScript( RES_CHRATR_LANGUAGE,
                            GetI18NScriptTypeOfLanguage( (sal_uInt16)GetAppLanguage())) );
            LanguageType nLang = ((const SvxLanguageItem &) rItem).GetLanguage();

            uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
            if (!xThes.is() || nLang == LANGUAGE_NONE || !xThes->hasLocale( LanguageTag::convertToLocale( nLang ) ))
                rSet.DisableItem( SID_THESAURUS );
            nSlotId = 0;
        }
        break;
        case SID_HANGUL_HANJA_CONVERSION:
        case SID_CHINESE_CONVERSION:
        {
            if (!SvtCJKOptions().IsAnyEnabled())
            {
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_False );
                rSet.DisableItem(nWhich);
            }
            else
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_True );
        }
        break;

        case SID_TEXTDIRECTION_LEFT_TO_RIGHT:
        case SID_TEXTDIRECTION_TOP_TO_BOTTOM:
            if ( !SvtLanguageOptions().IsVerticalTextEnabled() )
            {
                rSet.DisableItem( nSlotId );
                nSlotId = 0;
            }
            else
            {
                SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
                if( pOutliner )
                    bFlag = pOutliner->IsVertical() ==
                            (SID_TEXTDIRECTION_TOP_TO_BOTTOM == nSlotId);
                else
                {
                    text::WritingMode eMode = (text::WritingMode)
                                    ( (const SvxWritingModeItem&) aEditAttr.Get( SDRATTR_TEXTDIRECTION ) ).GetValue();

                    if( nSlotId == SID_TEXTDIRECTION_LEFT_TO_RIGHT )
                    {
                        bFlag = eMode == text::WritingMode_LR_TB;
                    }
                    else
                    {
                        bFlag = eMode != text::WritingMode_TB_RL;
                    }
                }
            }
            break;
        case SID_ATTR_PARA_LEFT_TO_RIGHT:
        case SID_ATTR_PARA_RIGHT_TO_LEFT:
        {
            if ( !SvtLanguageOptions().IsCTLFontEnabled() )
            {
                rSet.DisableItem( nWhich );
                nSlotId = 0;
            }
            else
            {
                SdrOutliner * pOutliner = pSdrView->GetTextEditOutliner();
                if(pOutliner && pOutliner->IsVertical())
                {
                    rSet.DisableItem( nWhich );
                    nSlotId = 0;
                }
                else
                {
                    switch( ( ( (SvxFrameDirectionItem&) aEditAttr.Get( EE_PARA_WRITINGDIR ) ) ).GetValue() )
                    {
                        case FRMDIR_HORI_LEFT_TOP:
                            bFlag = nWhich == SID_ATTR_PARA_LEFT_TO_RIGHT;
                        break;

                        case FRMDIR_HORI_RIGHT_TOP:
                            bFlag = nWhich != SID_ATTR_PARA_LEFT_TO_RIGHT;
                        break;
                    }
                }
            }
        }
        break;
        case SID_TRANSLITERATE_HALFWIDTH:
        case SID_TRANSLITERATE_FULLWIDTH:
        case SID_TRANSLITERATE_HIRAGANA:
        case SID_TRANSLITERATE_KATAGANA:
        {
            SvtCJKOptions aCJKOptions;
            if(!aCJKOptions.IsChangeCaseMapEnabled())
            {
                rSet.DisableItem(nWhich);
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_False );
            }
            else
                GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_True );
        }
        break;
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_ZWNBSP :
        case SID_INSERT_ZWSP:
        {
            SvtCTLOptions aCTLOptions;
            sal_Bool bEnabled = aCTLOptions.IsCTLFontEnabled();
            GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, bEnabled );
            if(!bEnabled)
                rSet.DisableItem(nWhich);
        }
        break;
        default:
            nSlotId = 0;                // don't know this slot
            break;
        }

        if( nSlotId )
            rSet.Put( SfxBoolItem( nWhich, bFlag ));

        nWhich = aIter.NextWhich();
    }
}

void SwDrawTextShell::GetDrawTxtCtrlState(SfxItemSet& rSet)
{
    if (!IsTextEdit())  // Otherwise crash!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());

    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    sal_uInt16 nScriptType = pOLV->GetSelectedScriptType();
    while(nWhich)
    {
        sal_uInt16 nEEWhich = 0;
        sal_uInt16 nSlotId = GetPool().GetSlotId( nWhich );
        switch( nSlotId )
        {
            case SID_ATTR_CHAR_FONT:
            case SID_ATTR_CHAR_FONTHEIGHT:
            case SID_ATTR_CHAR_WEIGHT:
            case SID_ATTR_CHAR_POSTURE:
            {
                SfxItemPool* pEditPool = aEditAttr.GetPool()->GetSecondaryPool();
                if( !pEditPool )
                    pEditPool = aEditAttr.GetPool();
                SvxScriptSetItem aSetItem( nSlotId, *pEditPool );
                aSetItem.GetItemSet().Put( aEditAttr, sal_False );
                const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScriptType );
                if( pI )
                    rSet.Put( *pI, nWhich );
                else
                    rSet.InvalidateItem( nWhich );
            }
            break;
            case SID_ATTR_CHAR_COLOR: nEEWhich = EE_CHAR_COLOR; break;
            case SID_ATTR_CHAR_UNDERLINE: nEEWhich = EE_CHAR_UNDERLINE;break;
            case SID_ATTR_CHAR_OVERLINE: nEEWhich = EE_CHAR_OVERLINE;break;
            case SID_ATTR_CHAR_CONTOUR: nEEWhich = EE_CHAR_OUTLINE; break;
            case SID_ATTR_CHAR_SHADOWED:  nEEWhich = EE_CHAR_SHADOW;break;
            case SID_ATTR_CHAR_STRIKEOUT: nEEWhich = EE_CHAR_STRIKEOUT;break;
            case SID_AUTOSPELL_CHECK:
            {
                const SfxPoolItem* pState = rView.GetSlotState(nWhich);
                if (pState)
                    rSet.Put(SfxBoolItem(nWhich, ((const SfxBoolItem*)pState)->GetValue()));
                else
                    rSet.DisableItem( nWhich );
                break;
            }
            case SID_ATTR_CHAR_WORDLINEMODE: nEEWhich = EE_CHAR_WLM; break;
            case SID_ATTR_CHAR_RELIEF      : nEEWhich = EE_CHAR_RELIEF;  break;
            case SID_ATTR_CHAR_LANGUAGE    : nEEWhich = EE_CHAR_LANGUAGE;break;
            case SID_ATTR_CHAR_KERNING     : nEEWhich = EE_CHAR_KERNING; break;
            case SID_ATTR_CHAR_SCALEWIDTH:   nEEWhich = EE_CHAR_FONTWIDTH;break;
            case SID_ATTR_CHAR_AUTOKERN  :   nEEWhich = EE_CHAR_PAIRKERNING; break;
            case SID_ATTR_CHAR_ESCAPEMENT:   nEEWhich = EE_CHAR_ESCAPEMENT; break;
        }
        if(nEEWhich)
            rSet.Put(aEditAttr.Get(nEEWhich, sal_True), nWhich);

        nWhich = aIter.NextWhich();
    }
}

void SwDrawTextShell::ExecClpbrd(SfxRequest &rReq)
{
    if (!IsTextEdit())  // Otherwise crash!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();

    ESelection aSel(pOLV->GetSelection());
    const bool bCopy = (aSel.nStartPara != aSel.nEndPara) || (aSel.nStartPos != aSel.nEndPos);
    sal_uInt16 nId = rReq.GetSlot();
    switch( nId )
    {
        case SID_CUT:
            if (bCopy)
                pOLV->Cut();
            return;

        case SID_COPY:
            if (bCopy)
                pOLV->Copy();
            return;

        case SID_PASTE:
            pOLV->PasteSpecial();
            break;

        default:
            OSL_FAIL("wrong dispatcher");
            return;
    }
}

void SwDrawTextShell::StateClpbrd(SfxItemSet &rSet)
{
    if (!IsTextEdit())  // Otherwise crash!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    ESelection aSel(pOLV->GetSelection());
    const bool bCopy = (aSel.nStartPara != aSel.nEndPara) ||
                           (aSel.nStartPos != aSel.nEndPos);


    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_CUT:
            case SID_COPY:
                if( !bCopy )
                    rSet.DisableItem( nWhich );
                break;

            case SID_PASTE:
                {
                    TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                &GetView().GetEditWin() ) );

                    if( !aDataHelper.GetXTransferable().is() ||
                        !SwTransferable::IsPaste( GetShell(), aDataHelper ))
                        rSet.DisableItem( SID_PASTE );
                }
                break;

            case SID_PASTE_SPECIAL:
                rSet.DisableItem( SID_PASTE_SPECIAL );
                break;
            // #151110#
            case SID_CLIPBOARD_FORMAT_ITEMS:
                rSet.DisableItem( SID_CLIPBOARD_FORMAT_ITEMS );
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

// Hyperlink status

void SwDrawTextShell::StateInsert(SfxItemSet &rSet)
{
    if (!IsTextEdit())  // Otherwise crash!
        return;

    OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while(nWhich)
    {
        switch(nWhich)
        {
            case SID_HYPERLINK_GETLINK:
                {
                    SvxHyperlinkItem aHLinkItem;
                    aHLinkItem.SetInsertMode(HLINK_FIELD);

                    const SvxFieldItem* pFieldItem = pOLV->GetFieldAtSelection();

                    if (pFieldItem)
                    {
                        const SvxFieldData* pField = pFieldItem->GetField();

                        if (pField->ISA(SvxURLField))
                        {
                            aHLinkItem.SetName(((const SvxURLField*) pField)->GetRepresentation());
                            aHLinkItem.SetURL(((const SvxURLField*) pField)->GetURL());
                            aHLinkItem.SetTargetFrame(((const SvxURLField*) pField)->GetTargetFrame());
                        }
                    }
                    else
                    {
                        String sSel(pOLV->GetSelected());
                        sSel.Erase(255);
                        aHLinkItem.SetName(comphelper::string::stripEnd(sSel, ' '));
                    }

                    sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
                    aHLinkItem.SetInsertMode((SvxLinkInsertMode)(aHLinkItem.GetInsertMode() |
                        ((nHtmlMode & HTMLMODE_ON) != 0 ? HLINK_HTMLMODE : 0)));

                    rSet.Put(aHLinkItem);
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
