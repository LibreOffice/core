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


#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>

#include <comphelper/processfactory.hxx>
#include <hintids.hxx>
#include <cmdid.h>
#include <helpid.h>

#include <i18nlangtag/languagetag.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/langitem.hxx>
#include <svtools/langtab.hxx>
#include <svl/slstitm.hxx>
#include <string.h>
#include <svl/stritem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svl/whiter.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objitem.hxx>
#include <vcl/unohelp2.hxx>
#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <svl/macitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/svxacorr.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <IDocumentSettingAccess.hxx>
#include <charfmt.hxx>
#include <editeng/fontitem.hxx>
#include <svx/SmartTagItem.hxx>
#include <fmtinfmt.hxx>
#include <swwait.hxx>
#include <wrtsh.hxx>
#include <wview.hxx>
#include <swmodule.hxx>
#include <viewopt.hxx>
#include <uitool.hxx>
#include <swevent.hxx>
#include <pagedesc.hxx>
#include <textsh.hxx>
#include <IMark.hxx>
#include <swdtflvr.hxx>
#include <docstat.hxx>
#include <outline.hxx>
#include <tablemgr.hxx>
#include <swundo.hxx>       // for Undo-IDs
#include <reffld.hxx>
#include <docsh.hxx>
#include <mdiexp.hxx>
#include <inputwin.hxx>
#include <chrdlgmodes.hxx>
#include <pardlg.hxx>
#include <frmatr.hxx>
#include <fmtcol.hxx>
#include <cellatr.hxx>
#include <edtwin.hxx>
#include <redlndlg.hxx>
#include "fldmgr.hxx"

#include <globals.hrc>
#include <shells.hrc>
#include <app.hrc>
#include <web.hrc>
#include "paratr.hxx"
#include <crsskip.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/app.hxx>
#include <breakit.hxx>

#include <SwSmartTagMgr.hxx>

#include <editeng/acorrcfg.hxx>
#include "swabstdlg.hxx"
#include "misc.hrc"
#include "chrdlg.hrc"
#include <IDocumentStatistics.hxx>

#include <sfx2/sfxdlg.hxx>
#include <unotools/lingucfg.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <editeng/unolingu.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <doc.hxx>
#include <view.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <sfx2/objface.hxx>
#include <langhelper.hxx>
#include <uiitems.hxx>
#include <wordcountdialog.hxx>
#include <tools/diagnose_ex.h>

#ifndef _NBDTMGFACT_HXX
#include <svx/nbdtmgfact.hxx>
#endif
#ifndef _NBDTMG_HXX
#include <svx/nbdtmg.hxx>
#endif


#include <numrule.hxx>


using namespace ::com::sun::star;
using namespace svx::sidebar;

void sw_CharDialog( SwWrtShell &rWrtSh, bool bUseDialog, sal_uInt16 nSlot,const SfxItemSet *pArgs, SfxRequest *pReq )
{
    FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &rWrtSh.GetView()));
    SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
    SfxItemSet aCoreSet( rWrtSh.GetView().GetPool(),
                        RES_CHRATR_BEGIN,      RES_CHRATR_END-1,
                        RES_TXTATR_INETFMT,    RES_TXTATR_INETFMT,
                        RES_BACKGROUND,        RES_BACKGROUND,
                        RES_BOX,               RES_BOX,
                        SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                        FN_PARAM_SELECTION,    FN_PARAM_SELECTION,
                        SID_HTML_MODE,         SID_HTML_MODE,
                        SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,   SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,
                        0 );
    rWrtSh.GetCurAttr( aCoreSet );
    sal_Bool bSel = rWrtSh.HasSelection();
    sal_Bool bSelectionPut = sal_False;
    if(bSel || rWrtSh.IsInWord())
    {
        if(!bSel)
        {
            rWrtSh.StartAction();
            rWrtSh.Push();
            if(!rWrtSh.SelectTxtAttr( RES_TXTATR_INETFMT ))
                rWrtSh.SelWrd();
        }
        aCoreSet.Put(SfxStringItem(FN_PARAM_SELECTION, rWrtSh.GetSelTxt()));
        bSelectionPut = sal_True;
        if(!bSel)
        {
            rWrtSh.Pop(sal_False);
            rWrtSh.EndAction();
        }
    }
        aCoreSet.Put( SfxUInt16Item( SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,
                    rWrtSh.GetScalingOfSelectedText() ) );
    // The CHRATR_BACKGROUND attribute will be converted for the
    // dialogue in a RES_BACKGROUND and back again ...
    const SfxPoolItem *pTmpBrush;
    if( SFX_ITEM_SET == aCoreSet.GetItemState( RES_CHRATR_BACKGROUND, sal_True, &pTmpBrush ) )
    {
        SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
        aTmpBrush.SetWhich( RES_BACKGROUND );
        aCoreSet.Put( aTmpBrush );
    }
    else
        aCoreSet.ClearItem(RES_BACKGROUND);

    // The CHRATR_BOX attribute will be converted for the
    // dialogue in a RES_BOX and back again ...
    const SfxPoolItem *pTmpBox;
    if( SFX_ITEM_SET == aCoreSet.GetItemState( RES_CHRATR_BOX, sal_True, &pTmpBox ) )
    {
        SvxBoxItem aTmpBox( *((SvxBoxItem*)pTmpBox) );
        aTmpBox.SetWhich( RES_BOX );
        aCoreSet.Put( aTmpBox );
    }
    else
        aCoreSet.ClearItem(RES_BOX);

    // Setting the BoxInfo
    ::PrepareBoxInfo( aCoreSet, rWrtSh );

    aCoreSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(rWrtSh.GetView().GetDocShell())));
    SfxAbstractTabDialog* pDlg = NULL;
    if ( bUseDialog && GetActiveView() )
    {
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

        pDlg = pFact->CreateSwCharDlg(rWrtSh.GetView().GetWindow(), rWrtSh.GetView(), aCoreSet, DLG_CHAR_STD);
        OSL_ENSURE(pDlg, "Dialogdiet fail!");
        if( FN_INSERT_HYPERLINK == nSlot )
            pDlg->SetCurPageId("hyperlink");
    }
    if (nSlot == SID_CHAR_DLG_EFFECT)
    {
        pDlg->SetCurPageId(TP_CHAR_EXT);
    }

    const SfxItemSet* pSet = NULL;
    if ( !bUseDialog )
        pSet = pArgs;
    else if ( NULL != pDlg && pDlg->Execute() == RET_OK ) /* #110771# pDlg can be NULL */
    {
        pSet = pDlg->GetOutputItemSet();
    }

    if ( pSet)
    {
        SfxItemSet aTmpSet( *pSet );
        if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_BACKGROUND, sal_False, &pTmpBrush ) )
        {
            SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
            aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
            aTmpSet.Put( aTmpBrush );
        }
        aTmpSet.ClearItem( RES_BACKGROUND );

        if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_BOX, sal_False, &pTmpBox ) )
        {
            SvxBoxItem aTmpBox( *((SvxBoxItem*)pTmpBox) );
            aTmpBox.SetWhich( RES_CHRATR_BOX );
            aTmpSet.Put( aTmpBox );
        }
        aTmpSet.ClearItem( RES_BOX );

        const SfxPoolItem* pSelectionItem;
        sal_Bool bInsert = sal_False;
        xub_StrLen nInsert = 0;

        // The old item is for unknown reasons back in the set again.
        if( !bSelectionPut && SFX_ITEM_SET == aTmpSet.GetItemState(FN_PARAM_SELECTION, sal_False, &pSelectionItem) )
        {
            String sInsert = ((const SfxStringItem*)pSelectionItem)->GetValue();
            bInsert = sInsert.Len() != 0;
            if(bInsert)
            {
                nInsert = sInsert.Len();
                rWrtSh.StartAction();
                rWrtSh.Insert( sInsert );
                rWrtSh.SetMark();
                rWrtSh.ExtendSelection(sal_False, sInsert.Len());
                SfxRequest aReq( rWrtSh.GetView().GetViewFrame(), FN_INSERT_STRING );
                aReq.AppendItem( SfxStringItem( FN_INSERT_STRING, sInsert ) );
                aReq.Done();
                SfxRequest aReq1( rWrtSh.GetView().GetViewFrame(), FN_CHAR_LEFT );
                aReq1.AppendItem( SfxInt16Item(FN_PARAM_MOVE_COUNT, nInsert) );
                aReq1.AppendItem( SfxBoolItem(FN_PARAM_MOVE_SELECTION, sal_True) );
                aReq1.Done();
            }
        }
        aTmpSet.ClearItem(FN_PARAM_SELECTION);

        SwTxtFmtColl* pColl = rWrtSh.GetCurTxtFmtColl();
        if(bSel && rWrtSh.IsSelFullPara() && pColl && pColl->IsAutoUpdateFmt())
        {
            rWrtSh.AutoUpdatePara(pColl, aTmpSet);
        }
        else
            rWrtSh.SetAttr( aTmpSet );
        if (pReq)
            pReq->Done(aTmpSet);
        if(bInsert)
        {
            SfxRequest aReq1( rWrtSh.GetView().GetViewFrame(), FN_CHAR_RIGHT );
            aReq1.AppendItem( SfxInt16Item(FN_PARAM_MOVE_COUNT, nInsert) );
            aReq1.AppendItem( SfxBoolItem(FN_PARAM_MOVE_SELECTION, sal_False) );
            aReq1.Done();
            rWrtSh.SwapPam();
            rWrtSh.ClearMark();
            rWrtSh.DontExpandFmt();
            rWrtSh.EndAction();
        }
    }

    delete pDlg;
}

static short lcl_AskRedlineMode(Window *pWin)
{
    MessBox aQBox( pWin, 0,
                    String( SW_RES( STR_REDLINE_TITLE ) ),
                    String( SW_RES( STR_REDLINE_MSG ) ) );
    aQBox.SetImage( QueryBox::GetStandardImage() );
    sal_uInt16 nBtnFlags = BUTTONDIALOG_DEFBUTTON |
                        BUTTONDIALOG_OKBUTTON |
                        BUTTONDIALOG_FOCUSBUTTON;

    aQBox.AddButton(String(SW_RES(STR_REDLINE_ACCEPT_ALL)), RET_OK, nBtnFlags);
    aQBox.GetPushButton( RET_OK )->SetHelpId(HID_AUTOFORMAT_ACCEPT);
    aQBox.AddButton(String(SW_RES(STR_REDLINE_REJECT_ALL)), RET_CANCEL, BUTTONDIALOG_CANCELBUTTON);
    aQBox.GetPushButton( RET_CANCEL )->SetHelpId(HID_AUTOFORMAT_REJECT  );
    aQBox.AddButton(String(SW_RES(STR_REDLINE_EDIT)), 2, 0);
    aQBox.GetPushButton( 2 )->SetHelpId(HID_AUTOFORMAT_EDIT_CHG);
    aQBox.SetButtonHelpText( RET_OK, OUString() );

    return aQBox.Execute();
}

void SwTextShell::Execute(SfxRequest &rReq)
{
    bool bUseDialog = true;
    const SfxItemSet *pArgs = rReq.GetArgs();
    SwWrtShell& rWrtSh = GetShell();
    const SfxPoolItem* pItem = 0;
    sal_uInt16 nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), sal_False, &pItem);
    switch( nSlot )
    {
        case SID_LANGUAGE_STATUS:
        {
            // get the language
            String aNewLangTxt;
            SFX_REQUEST_ARG( rReq, pItem2, SfxStringItem, SID_LANGUAGE_STATUS , sal_False );
            if (pItem2)
                aNewLangTxt = pItem2->GetValue();

            //!! Remember the view frame right now...
            //!! (call to GetView().GetViewFrame() will break if the
            //!! SwTextShell got destroyed meanwhile.)
            SfxViewFrame *pViewFrame = GetView().GetViewFrame();

            if (aNewLangTxt.EqualsAscii( "*" ))
            {
                // open the dialog "Tools/Options/Language Settings - Language"
                // to set the documents default language
                SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                if (pFact)
                {
                    VclAbstractDialog* pDlg = pFact->CreateVclDialog( GetView().GetWindow(), SID_LANGUAGE_OPTIONS );
                    pDlg->Execute();
                    delete pDlg;
                }
            }
            else
            {
                //!! We have to use StartAction / EndAction bracketing in
                //!! order to prevent possible destruction of the SwTextShell
                //!! due to the selection changes coming below.
                rWrtSh.StartAction();
                // prevent view from jumping because of (temporary) selection changes
                rWrtSh.LockView( sal_True );

                // save selection for later restoration
                rWrtSh.Push();

                // setting the new language...
                if (aNewLangTxt.Len() > 0)
                {
                    const OUString aSelectionLangPrefix("Current_");
                    const OUString aParagraphLangPrefix("Paragraph_");
                    const OUString aDocumentLangPrefix("Default_");
                    const String aStrNone( OUString("LANGUAGE_NONE") );
                    const String aStrResetLangs( OUString("RESET_LANGUAGES") );

                    SfxItemSet aCoreSet( GetPool(),
                            RES_CHRATR_LANGUAGE,        RES_CHRATR_LANGUAGE,
                            RES_CHRATR_CJK_LANGUAGE,    RES_CHRATR_CJK_LANGUAGE,
                            RES_CHRATR_CTL_LANGUAGE,    RES_CHRATR_CTL_LANGUAGE,
                            0 );

                    xub_StrLen nPos = 0;
                    bool bForSelection = true;
                    bool bForParagraph = false;
                    if (STRING_NOTFOUND != (nPos = aNewLangTxt.Search( aSelectionLangPrefix, 0 )))
                    {
                        // ... for the current selection
                        aNewLangTxt = aNewLangTxt.Erase( nPos, aSelectionLangPrefix.getLength() );
                        bForSelection = true;
                    }
                    else if (STRING_NOTFOUND != (nPos = aNewLangTxt.Search( aParagraphLangPrefix , 0 )))
                    {
                        // ... for the current paragraph language
                        aNewLangTxt = aNewLangTxt.Erase( nPos, aParagraphLangPrefix.getLength() );
                        bForSelection = true;
                        bForParagraph = true;
                    }
                    else if (STRING_NOTFOUND != (nPos = aNewLangTxt.Search( aDocumentLangPrefix , 0 )))
                    {
                        // ... as default document language
                        aNewLangTxt = aNewLangTxt.Erase( nPos, aDocumentLangPrefix.getLength() );
                        bForSelection = false;
                    }

                    if (bForParagraph)
                        SwLangHelper::SelectCurrentPara( rWrtSh );

                    if (!bForSelection) // document language to be changed...
                    {
                        rWrtSh.SelAll();
                        rWrtSh.ExtendedSelectAll();
                    }

                    rWrtSh.StartUndo( ( !bForParagraph && !bForSelection ) ? UNDO_SETDEFTATTR : UNDO_EMPTY );
                    if (aNewLangTxt == aStrNone)
                        SwLangHelper::SetLanguage_None( rWrtSh, bForSelection, aCoreSet );
                    else if (aNewLangTxt == aStrResetLangs)
                        SwLangHelper::ResetLanguages( rWrtSh, bForSelection );
                    else
                        SwLangHelper::SetLanguage( rWrtSh, aNewLangTxt, bForSelection, aCoreSet );
                    rWrtSh.EndUndo();

                }

                // restore selection...
                rWrtSh.Pop( sal_False );

                rWrtSh.LockView( sal_False );
                rWrtSh.EndAction();
            }

            // invalidate slot to get the new language displayed
            pViewFrame->GetBindings().Invalidate( nSlot );

            rReq.Done();
            break;
        }

        case SID_THES:
        {
            // replace word/selection with text from selected sub menu entry
            String aReplaceText;
            SFX_REQUEST_ARG( rReq, pItem2, SfxStringItem, SID_THES , sal_False );
            if (pItem2)
                aReplaceText = pItem2->GetValue();
            if (aReplaceText.Len() > 0)
            {
                SwView &rView2 = rWrtSh.GetView();
                const bool bSelection = rWrtSh.HasSelection();
                const String aLookUpText = rView2.GetThesaurusLookUpText( bSelection );
                rView2.InsertThesaurusSynonym( aReplaceText, aLookUpText, bSelection );
            }
        }
        break;

        case SID_CHARMAP:
        {
            InsertSymbol( rReq );
        }
        break;
        case FN_INSERT_FOOTNOTE:
        case FN_INSERT_ENDNOTE:
        {
            String aStr;
            SFX_REQUEST_ARG( rReq, pFont, SfxStringItem, FN_PARAM_1 , sal_False );
            SFX_REQUEST_ARG( rReq, pNameItem, SfxStringItem, nSlot , sal_False );
            if ( pNameItem )
                aStr = pNameItem->GetValue();
            bool bFont = pFont && !pFont->GetValue().isEmpty();
            rWrtSh.StartUndo( UNDO_UI_INSERT_FOOTNOTE );
            rWrtSh.InsertFootnote( aStr, nSlot == FN_INSERT_ENDNOTE, !bFont );
            if ( bFont )
            {
                rWrtSh.Left( CRSR_SKIP_CHARS, sal_True, 1, sal_False );
                SfxItemSet aSet( rWrtSh.GetAttrPool(), RES_CHRATR_FONT, RES_CHRATR_FONT );
                rWrtSh.GetCurAttr( aSet );
                SvxFontItem &rFont = (SvxFontItem &) aSet.Get( RES_CHRATR_FONT );
                SvxFontItem aFont( rFont.GetFamily(), pFont->GetValue(),
                                    rFont.GetStyleName(), rFont.GetPitch(), RTL_TEXTENCODING_DONTKNOW, RES_CHRATR_FONT );
                rWrtSh.SetAttr( aSet, nsSetAttrMode::SETATTR_DONTEXPAND );
                rWrtSh.ResetSelect(0, sal_False);
                rWrtSh.EndSelect();
                rWrtSh.GotoFtnTxt();
            }
            rWrtSh.EndUndo( UNDO_UI_INSERT_FOOTNOTE );
            rReq.Done();
        }
        break;
        case FN_INSERT_FOOTNOTE_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "Dialogdiet fail!");
            AbstractInsFootNoteDlg* pDlg = pFact->CreateInsFootNoteDlg(
                GetView().GetWindow(), rWrtSh, sal_False);
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->SetHelpId(GetStaticInterface()->GetSlot(nSlot)->GetCommand());
            if ( pDlg->Execute() == RET_OK )
            {
                sal_uInt16 nId = pDlg->IsEndNote() ? FN_INSERT_ENDNOTE : FN_INSERT_FOOTNOTE;
                SfxRequest aReq( GetView().GetViewFrame(), nId );
                if ( pDlg->GetStr().Len() )
                    aReq.AppendItem( SfxStringItem( nId, pDlg->GetStr() ) );
                if ( pDlg->GetFontName().Len() )
                    aReq.AppendItem( SfxStringItem( FN_PARAM_1, pDlg->GetFontName() ) );
                ExecuteSlot( aReq );
            }

            rReq.Ignore();
            delete pDlg;
        }
        break;
        case FN_FORMAT_FOOTNOTE_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            VclAbstractDialog* pDlg = pFact->CreateSwFootNoteOptionDlg(GetView().GetWindow(), rWrtSh);
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
            break;
        }
        case SID_INSERTDOC:
        {
            GetView().ExecuteInsertDoc( rReq, pItem );
            break;
        }
        case FN_FORMAT_RESET:
        {
            // #i78856, reset all attributes but not the language attributes
            // (for this build an array of all relevant attributes and
            // remove the languages from that)
            std::set<sal_uInt16> aAttribs;

            sal_uInt16 aResetableSetRange[] = {
                RES_FRMATR_BEGIN, RES_FRMATR_END-1,
                RES_CHRATR_BEGIN, RES_CHRATR_LANGUAGE - 1,
                RES_CHRATR_LANGUAGE + 1, RES_CHRATR_CJK_LANGUAGE - 1,
                RES_CHRATR_CJK_LANGUAGE + 1, RES_CHRATR_CTL_LANGUAGE - 1,
                RES_CHRATR_CTL_LANGUAGE + 1, RES_CHRATR_END-1,
                RES_PARATR_BEGIN, RES_PARATR_END-1,
                RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
                0
            };
            sal_uInt16 *pUShorts = aResetableSetRange;
            while (*pUShorts)
            {
                sal_uInt16 nL = pUShorts[1] - pUShorts[0] + 1;
                sal_uInt16 nE = pUShorts[0];
                for (sal_uInt16 i = 0; i < nL; ++i)
                    aAttribs.insert( aAttribs.end(), nE++ );
                pUShorts += 2;
            }
            // we don't want to change writing direction.
            aAttribs.erase( RES_FRAMEDIR );
            rWrtSh.ResetAttr( aAttribs );
            rReq.Done();
            break;
        }
        case FN_INSERT_BREAK_DLG:
        {
            sal_uInt16 nKind=0, nPageNumber=0;
            String aTemplateName;
            if ( pItem )
            {
                nKind = ((SfxInt16Item*)pItem)->GetValue();
                SFX_REQUEST_ARG( rReq, pTemplate, SfxStringItem, FN_PARAM_1 , sal_False );
                SFX_REQUEST_ARG( rReq, pNumber, SfxUInt16Item, FN_PARAM_2 , sal_False );
                if ( pTemplate )
                    aTemplateName = pTemplate->GetValue();
                if ( pNumber )
                    nPageNumber = pNumber->GetValue();
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                AbstractSwBreakDlg* pDlg = pFact->CreateSwBreakDlg(GetView().GetWindow(), rWrtSh);
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                if ( pDlg->Execute() == RET_OK )
                {
                    nKind = pDlg->GetKind();
                    aTemplateName = pDlg->GetTemplateName();
                    nPageNumber = pDlg->GetPageNumber();
                    rReq.AppendItem( SfxInt16Item( FN_INSERT_BREAK_DLG, nKind ) );
                    rReq.AppendItem( SfxUInt16Item( FN_PARAM_2, nPageNumber ) );
                    rReq.AppendItem( SfxStringItem( FN_PARAM_1, aTemplateName ) );
                    rReq.Done();
                }
                else
                    rReq.Ignore();
                delete pDlg;
            }

            switch ( nKind )
            {
                case 1 :
                    rWrtSh.InsertLineBreak(); break;
                case 2 :
                    rWrtSh.InsertColumnBreak(); break;
                case 3 :
                {
                    rWrtSh.StartAllAction();
                    if( aTemplateName.Len() )
                        rWrtSh.InsertPageBreak( &aTemplateName, nPageNumber );
                    else
                        rWrtSh.InsertPageBreak();
                    rWrtSh.EndAllAction();
                }
            }

            break;
        }
        case FN_INSERT_BOOKMARK:
        {
            if ( pItem )
            {
                OUString sName = ((SfxStringItem*)pItem)->GetValue();
                rWrtSh.SetBookmark( KeyCode(), sName, aEmptyStr );
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                VclAbstractDialog* pDlg = pFact->CreateSwInsertBookmarkDlg( GetView().GetWindow(), rWrtSh, rReq, DLG_INSERT_BOOKMARK );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                pDlg->Execute();
                delete pDlg;
            }

            break;
        }
        case FN_DELETE_BOOKMARK:
        {
            if ( pItem )
            {
                IDocumentMarkAccess* const pMarkAccess = rWrtSh.getIDocumentMarkAccess();
                pMarkAccess->deleteMark( pMarkAccess->findMark(((SfxStringItem*)pItem)->GetValue()) );
            }
            break;
        }
        case FN_AUTOFORMAT_REDLINE_APPLY:
        {
            SvxSwAutoFmtFlags aFlags(SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags());
            // This must always be sal_False for the postprocessing.
            aFlags.bAFmtByInput = sal_False;
            aFlags.bWithRedlining = sal_True;
            rWrtSh.AutoFormat( &aFlags );
            aFlags.bWithRedlining = sal_False;

            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if (pVFrame->HasChildWindow(FN_REDLINE_ACCEPT))
                pVFrame->ToggleChildWindow(FN_REDLINE_ACCEPT);

            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            AbstractSwModalRedlineAcceptDlg* pDlg = pFact->CreateSwModalRedlineAcceptDlg(&GetView().GetEditWin());
            OSL_ENSURE(pDlg, "Dialogdiet fail!");

            switch (lcl_AskRedlineMode(&GetView().GetEditWin()))
            {
                case RET_OK:
                {
                    pDlg->AcceptAll(sal_True);
                    SfxRequest aReq( pVFrame, FN_AUTOFORMAT_APPLY );
                    aReq.Done();
                    rReq.Ignore();
                    break;
                }

                case RET_CANCEL:
                    pDlg->AcceptAll(sal_False);
                    rReq.Ignore();
                    break;

                case 2:
                    pDlg->Execute();
                    rReq.Done();
                    break;
            }
            delete pDlg;
        }
        break;

        case FN_AUTOFORMAT_APPLY:
        {
            SvxSwAutoFmtFlags aFlags(SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags());
            // This must always be sal_False for the postprocessing.
            aFlags.bAFmtByInput = sal_False;
            rWrtSh.AutoFormat( &aFlags );
            rReq.Done();
        }
        break;
        case FN_AUTOFORMAT_AUTO:
        {
            SvxAutoCorrCfg& rACfg = SvxAutoCorrCfg::Get();
            sal_Bool bSet = pItem ? ((const SfxBoolItem*)pItem)->GetValue() : !rACfg.IsAutoFmtByInput();
            if( bSet != rACfg.IsAutoFmtByInput() )
            {
                rACfg.SetAutoFmtByInput( bSet );
                GetView().GetViewFrame()->GetBindings().Invalidate( nSlot );
                if ( !pItem )
                    rReq.AppendItem( SfxBoolItem( GetPool().GetWhich(nSlot), bSet ) );
                rReq.Done();
            }
        }
        break;
        case FN_AUTO_CORRECT:
        {
            // At first set to blank as default.
            sal_Unicode cChar = ' ';
            rWrtSh.AutoCorrect( *SvxAutoCorrCfg::Get().GetAutoCorrect(), cChar );
            rReq.Done();
        }
        break;
        case FN_TABLE_SORT_DIALOG:
        case FN_SORTING_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            VclAbstractDialog* pDlg = pFact->CreateVclAbstractDialog( GetView().GetWindow(), rWrtSh, DLG_SORTING );
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
            rReq.Done();
        }
        break;
        case FN_NUMBERING_OUTLINE_DLG:
        {
            SfxItemSet aTmp(GetPool(), FN_PARAM_1, FN_PARAM_1);
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "Dialogdiet fail!");
            SfxAbstractTabDialog* pDlg = pFact->CreateSwTabDialog( DLG_TAB_OUTLINE,
                                                        GetView().GetWindow(), &aTmp, rWrtSh);
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
            rReq.Done();
        }
            break;
        case FN_CALCULATE:
            {
                SwTransferable* pTransfer = new SwTransferable( rWrtSh );
/*??*/          uno::Reference<
                    datatransfer::XTransferable > xRef(
                                                    pTransfer );
                pTransfer->CalculateAndCopy();
                rReq.Done();
            }
            break;
        case FN_GOTO_REFERENCE:
        {
            SwField *pFld = rWrtSh.GetCurFld();
            if(pFld && pFld->GetTypeId() == TYP_GETREFFLD)
            {
                rWrtSh.StartAllAction();
                rWrtSh.SwCrsrShell::GotoRefMark( ((SwGetRefField*)pFld)->GetSetRefName(),
                                    ((SwGetRefField*)pFld)->GetSubType(),
                                    ((SwGetRefField*)pFld)->GetSeqNo() );
                rWrtSh.EndAllAction();
                rReq.Done();
            }
        }
            break;
        case FN_EDIT_FORMULA:
        {
            const sal_uInt16 nId = SwInputChild::GetChildWindowId();
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if(pItem)
            {
                //if the ChildWindow is active it has to be removed
                if( pVFrame->HasChildWindow( nId ) )
                {
                    pVFrame->ToggleChildWindow( nId );
                    pVFrame->GetBindings().InvalidateAll( sal_True );
                }

                String sFormula(((const SfxStringItem*)pItem)->GetValue());
                SwFldMgr aFldMgr;
                rWrtSh.StartAllAction();
                sal_Bool bDelSel;
                if( 0 != (bDelSel = rWrtSh.HasSelection()) )
                {
                    rWrtSh.StartUndo( UNDO_START );
                    rWrtSh.DelRight();
                }
                else
                {
                    rWrtSh.EnterStdMode();
                }

                if( !bDelSel && aFldMgr.GetCurFld() && TYP_FORMELFLD == aFldMgr.GetCurTypeId() )
                    aFldMgr.UpdateCurFld( aFldMgr.GetCurFld()->GetFormat(), aEmptyStr, sFormula );
                else if( sFormula.Len() )
                {
                    if( rWrtSh.IsCrsrInTbl() )
                    {
                        SfxItemSet aSet( rWrtSh.GetAttrPool(), RES_BOXATR_FORMULA, RES_BOXATR_FORMULA );
                        aSet.Put( SwTblBoxFormula( sFormula ));
                        rWrtSh.SetTblBoxFormulaAttrs( aSet );
                        rWrtSh.UpdateTable();
                    }
                    else
                    {
                        SvNumberFormatter* pFormatter = rWrtSh.GetNumberFormatter();
                        sal_uLong nSysNumFmt = pFormatter->GetFormatIndex( NF_NUMBER_STANDARD, LANGUAGE_SYSTEM);
                        SwInsertFld_Data aData(TYP_FORMELFLD, nsSwGetSetExpType::GSE_FORMULA, aEmptyStr, sFormula, nSysNumFmt);
                        aFldMgr.InsertFld(aData);
                    }
                }

                if( bDelSel )
                    rWrtSh.EndUndo( UNDO_END );
                rWrtSh.EndAllAction();
                rReq.Done();
            }
            else
            {
                rWrtSh.EndAllTblBoxEdit();
                pVFrame->ToggleChildWindow( nId );
                if( !pVFrame->HasChildWindow( nId ) )
                    pVFrame->GetBindings().InvalidateAll( sal_True );
                rReq.Ignore();
            }
        }

        break;
        case FN_TABLE_UNSET_READ_ONLY:
        {
            rWrtSh.UnProtectTbls();
        }
        break;
        case FN_EDIT_HYPERLINK:
            GetView().GetViewFrame()->ToggleChildWindow(SID_HYPERLINK_DIALOG);
        break;
        case FN_REMOVE_HYPERLINK:
        {
            sal_Bool bSel = rWrtSh.HasSelection();
            if(!bSel)
            {
                rWrtSh.StartAction();
                rWrtSh.Push();
                if(!rWrtSh.SelectTxtAttr( RES_TXTATR_INETFMT ))
                    rWrtSh.SelWrd();
            }
            //now remove the attribute
            std::set<sal_uInt16> aAttribs;
            aAttribs.insert( RES_TXTATR_INETFMT );
            rWrtSh.ResetAttr( aAttribs );
            if(!bSel)
            {
                rWrtSh.Pop(sal_False);
                rWrtSh.EndAction();
            }
        }
        break;
        case SID_ATTR_BRUSH_CHAR :
        case SID_ATTR_CHAR_SCALEWIDTH :
        case SID_ATTR_CHAR_ROTATED :
        case FN_TXTATR_INET :
        case FN_INSERT_HYPERLINK:
        {
            sal_uInt16 nWhich = GetPool().GetWhich( nSlot );
            if ( pArgs && pArgs->GetItemState( nWhich ) == SFX_ITEM_SET )
                bUseDialog = false;
            // intentionally no break
        }
        case SID_CHAR_DLG:
        case SID_CHAR_DLG_EFFECT:
        {
            sw_CharDialog( rWrtSh, bUseDialog, nSlot, pArgs, &rReq );
        }
        break;
        case SID_CHAR_DLG_FOR_PARAGRAPH:
        {
            rWrtSh.Push();          //save current cursor
            SwLangHelper::SelectCurrentPara( rWrtSh );
            sw_CharDialog( rWrtSh, bUseDialog, nSlot, pArgs, &rReq );
            rWrtSh.Pop( sal_False );    //restore old cursor
        }
        break;
        case SID_ATTR_LRSPACE :
        case SID_ATTR_ULSPACE :
        case SID_ATTR_BRUSH :
        case SID_PARA_VERTALIGN :
        case SID_ATTR_PARA_NUMRULE :
        case SID_ATTR_PARA_REGISTER :
        case SID_ATTR_PARA_PAGENUM :
        case FN_FORMAT_LINENUMBER :
        case FN_NUMBER_NEWSTART :
        case FN_NUMBER_NEWSTART_AT :
        case FN_FORMAT_DROPCAPS :
        case FN_DROP_TEXT:
        case SID_ATTR_PARA_LRSPACE:
        {
            sal_uInt16 nWhich = GetPool().GetWhich( nSlot );
            if ( pArgs && pArgs->GetItemState( nWhich ) == SFX_ITEM_SET )
                bUseDialog = false;
            // intentionally no break

        }
        case SID_PARA_DLG:
        {
            SwPaM* pPaM = NULL;

            if ( pArgs )
            {
                const SfxPoolItem* pPaMItem = 0;
                pArgs->GetItemState( GetPool().GetWhich( FN_PARAM_PAM ), sal_False, &pPaMItem );
                if ( pPaMItem )
                    pPaM = static_cast< const SwPaMItem* >( pPaMItem )->GetValue( );
            }

            if ( !pPaM )
                pPaM = rWrtSh.GetCrsr();

            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &GetView()));
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));

            sal_Bool bApplyCharUnit = ::HasCharUnit(0 != PTR_CAST(SwWebView, &GetView()));
            SW_MOD()->PutItem(SfxBoolItem(SID_ATTR_APPLYCHARUNIT, bApplyCharUnit));

            SfxItemSet aCoreSet( GetPool(),
                            RES_PARATR_BEGIN,           RES_PARATR_END - 1,
                            RES_PARATR_LIST_BEGIN,      RES_PARATR_LIST_END - 1,
                            RES_FRMATR_BEGIN,           RES_FRMATR_END - 1,
                            SID_ATTR_TABSTOP_POS,       SID_ATTR_TABSTOP_POS,
                            SID_ATTR_TABSTOP_DEFAULTS,  SID_ATTR_TABSTOP_DEFAULTS,
                            SID_ATTR_TABSTOP_OFFSET,    SID_ATTR_TABSTOP_OFFSET,
                            SID_ATTR_BORDER_INNER,      SID_ATTR_BORDER_INNER,
                            SID_ATTR_PARA_MODEL,        SID_ATTR_PARA_KEEP,
                            SID_ATTR_PARA_PAGENUM,      SID_ATTR_PARA_PAGENUM,
                            SID_HTML_MODE,              SID_HTML_MODE,
                            FN_PARAM_1,                 FN_PARAM_1,
                            FN_NUMBER_NEWSTART,         FN_NUMBER_NEWSTART_AT,
                            FN_DROP_TEXT,               FN_DROP_CHAR_STYLE_NAME,
                            0);
            // get also the list level indent values merged as LR-SPACE item, if needed.
            rWrtSh.GetPaMAttr( pPaM, aCoreSet, true );
            aCoreSet.Put(SfxUInt16Item(SID_HTML_MODE,
                            ::GetHtmlMode(GetView().GetDocShell())));

            // Tabulators: Put DefaultTabs into ItemSet
            const SvxTabStopItem& rDefTabs = (const SvxTabStopItem&)
                            GetPool().GetDefaultItem(RES_PARATR_TABSTOP);

            sal_uInt16 nDefDist = ::GetTabDist( rDefTabs );
            SfxUInt16Item aDefDistItem( SID_ATTR_TABSTOP_DEFAULTS, nDefDist );
            aCoreSet.Put( aDefDistItem );

            // Current tabulator
            SfxUInt16Item aTabPos( SID_ATTR_TABSTOP_POS, 0 );
            aCoreSet.Put( aTabPos );

            // Left border as offset
            //#i24363# tab stops relative to indent
            const long nOff = rWrtSh.getIDocumentSettingAccess()->get(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT) ?
                ((SvxLRSpaceItem&)aCoreSet.Get( RES_LR_SPACE )).GetTxtLeft() : 0;
            SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nOff );
            aCoreSet.Put( aOff );

            // Setting the BoxInfo
            ::PrepareBoxInfo( aCoreSet, rWrtSh );

            // Current page format
            ::SwToSfxPageDescAttr( aCoreSet );

            sal_uInt16 nDefPage = 0;
            if( pItem )
                nDefPage = ((SfxUInt16Item *)pItem)->GetValue();

            // Properties of numbering
            if( rWrtSh.GetDoc()->GetCurrNumRule( *pPaM->GetPoint() ) )
            {
                SfxBoolItem aStart( FN_NUMBER_NEWSTART, rWrtSh.IsNumRuleStart( pPaM ) );
                aCoreSet.Put(aStart);
                SfxUInt16Item aStartAt( FN_NUMBER_NEWSTART_AT,
                                        rWrtSh.GetNodeNumStart( pPaM ) );
                aCoreSet.Put(aStartAt);
            }
            SfxAbstractTabDialog* pDlg = NULL;

            if ( bUseDialog && GetActiveView() )
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                pDlg = pFact->CreateSwParaDlg( GetView().GetWindow(),GetView(), aCoreSet, DLG_STD, NULL, sal_False, nDefPage );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
            }
            SfxItemSet* pSet = NULL;
            if ( !bUseDialog )
            {
                if ( nSlot == SID_ATTR_PARA_LRSPACE)
                {
                    SvxLRSpaceItem aParaMargin((const SvxLRSpaceItem&)pArgs->Get(nSlot));
                    aParaMargin.SetWhich( RES_LR_SPACE);
                    aCoreSet.Put(aParaMargin);
                    pSet = &aCoreSet;

                } else
                    pSet = (SfxItemSet*) pArgs;

            }
            else if ( NULL != pDlg && pDlg->Execute() == RET_OK )
            {
                // Apply defaults if nessecary.
                pSet = (SfxItemSet*)pDlg->GetOutputItemSet();
                sal_uInt16 nNewDist;
                if( SFX_ITEM_SET == pSet->GetItemState( SID_ATTR_TABSTOP_DEFAULTS, sal_False, &pItem ) &&
                    nDefDist != (nNewDist = ((SfxUInt16Item*)pItem)->GetValue()) )
                {
                    SvxTabStopItem aDefTabs( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
                    MakeDefTabs( nNewDist, aDefTabs );
                    rWrtSh.SetDefault( aDefTabs );
                    pSet->ClearItem( SID_ATTR_TABSTOP_DEFAULTS );
                }

                if ( SFX_ITEM_SET == pSet->GetItemState(FN_PARAM_1,sal_False,&pItem) )
                {
                    pSet->Put(SfxStringItem(FN_DROP_TEXT, ((const SfxStringItem*)pItem)->GetValue()));
                    pSet->ClearItem(FN_PARAM_1);
                }

                if( SFX_ITEM_SET == pSet->GetItemState( RES_PARATR_DROP, sal_False, &pItem ))
                {
                    String sCharStyleName;
                    if(((const SwFmtDrop*)pItem)->GetCharFmt())
                        sCharStyleName = ((const SwFmtDrop*)pItem)->GetCharFmt()->GetName();
                    pSet->Put(SfxStringItem(FN_DROP_CHAR_STYLE_NAME, sCharStyleName));
                }
            }

            if ( pSet )
            {
                rReq.Done( *pSet );
                ::SfxToSwPageDescAttr( rWrtSh, *pSet );
                // #i56253#
                // enclose all undos.
                // Thus, check conditions, if actions will be performed.
                const bool bUndoNeeded( pSet->Count() ||
                        SFX_ITEM_SET == pSet->GetItemState(FN_NUMBER_NEWSTART) ||
                        SFX_ITEM_SET == pSet->GetItemState(FN_NUMBER_NEWSTART_AT) );
                if ( bUndoNeeded )
                {
                    rWrtSh.StartUndo( UNDO_INSATTR );
                }
                if( pSet->Count() )
                {
                    rWrtSh.StartAction();
                    if ( SFX_ITEM_SET == pSet->GetItemState(FN_DROP_TEXT, sal_False, &pItem) )
                    {
                        if ( !((SfxStringItem*)pItem)->GetValue().isEmpty() )
                            rWrtSh.ReplaceDropTxt(((SfxStringItem*)pItem)->GetValue(), pPaM);
                    }
                    rWrtSh.SetAttr( *pSet, 0, pPaM );
                    rWrtSh.EndAction();
                    SwTxtFmtColl* pColl = rWrtSh.GetPaMTxtFmtColl( pPaM );
                    if(pColl && pColl->IsAutoUpdateFmt())
                    {
                        rWrtSh.AutoUpdatePara(pColl, *pSet, pPaM);
                    }
                }

                if( SFX_ITEM_SET == pSet->GetItemState(FN_NUMBER_NEWSTART) )
                {
                    //SetNumRuleStart(sal_True) restarts the numbering at the value
                    //that is defined at the starting point of the numbering level
                    //otherwise the SetNodeNumStart() value determines the start
                    //if it's set to something different than (sal_uInt16)0xFFFF

                    sal_Bool bStart = ((SfxBoolItem&)pSet->Get(FN_NUMBER_NEWSTART)).GetValue();

                    // Default value for restart value has to be (sal_uInt16)0xFFFF
                    // in order to indicate that the restart value of the list
                    // style has to be used on restart.
                    sal_uInt16 nNumStart = (sal_uInt16)0xFFFF;
                    if( SFX_ITEM_SET == pSet->GetItemState(FN_NUMBER_NEWSTART_AT) )
                    {
                        nNumStart = ((SfxUInt16Item&)pSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
                    }
                    rWrtSh.SetNumRuleStart(bStart, pPaM);
                    rWrtSh.SetNodeNumStart(nNumStart);
                }
                else if( SFX_ITEM_SET == pSet->GetItemState(FN_NUMBER_NEWSTART_AT) )
                {
                    sal_uInt16 nNumStart = ((SfxUInt16Item&)pSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
                    rWrtSh.SetNodeNumStart(nNumStart);
                    rWrtSh.SetNumRuleStart(sal_False, pPaM);
                }
                // #i56253#
                if ( bUndoNeeded )
                {
                    rWrtSh.EndUndo( UNDO_INSATTR );
                }
            }

            delete pDlg;
        }
        break;
        case FN_NUM_CONTINUE:
        {
            String sContinuedListId;
            const SwNumRule* pRule =
                rWrtSh.SearchNumRule( false, true, false, -1, sContinuedListId );
            // #i86492#
            // Search also for bullet list
            if ( !pRule )
            {
                pRule = rWrtSh.SearchNumRule( false, false, false, -1, sContinuedListId );
            }
            if ( pRule )
            {
                rWrtSh.SetCurNumRule( *pRule, false, sContinuedListId );
            }
        }
        break;
        case FN_SELECT_PARA:
        {
            if(!rWrtSh.IsSttOfPara())
                rWrtSh.SttPara(sal_False);
            else
                rWrtSh.EnterStdMode();
            rWrtSh.EndPara(sal_True);
        }
        break;

        case SID_DEC_INDENT:
        case SID_INC_INDENT:
            rWrtSh.MoveLeftMargin( SID_INC_INDENT == nSlot,
                                    rReq.GetModifier() != KEY_MOD1 );
            rReq.Done();
            break;
        case FN_DEC_INDENT_OFFSET:
        case FN_INC_INDENT_OFFSET:
            rWrtSh.MoveLeftMargin( FN_INC_INDENT_OFFSET == nSlot,
                                    rReq.GetModifier() == KEY_MOD1 );
            rReq.Done();
            break;

        case SID_ATTR_CHAR_COLOR2:
        {
            if(pItem)
            {
                Color aSet = ((const SvxColorItem*)pItem)->GetValue();
                SwEditWin& rEditWin = GetView().GetEditWin();
                rEditWin.SetTextColor(aSet);
                SwApplyTemplate* pApply = rEditWin.GetApplyTemplate();

                // If there is a selection, then set the color on it
                // otherwise, it'll be the color for the next text to be typed
                if(!pApply || pApply->nColor != SID_ATTR_CHAR_COLOR_EXT)
                {
                    rWrtSh.SetAttr(SvxColorItem (aSet, RES_CHRATR_COLOR));
                }

                rReq.Done();
            }
        }
        break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
        {
            SwEditWin& rEdtWin = GetView().GetEditWin();
            SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
            rEdtWin.SetTextBackColorTransparent(0 == pItem);
            Color aSet;
            if(pItem)
            {
                aSet = ((const SvxColorItem*)pItem)->GetValue();
                rEdtWin.SetTextBackColor(aSet); //select last color
            }
            else
                rEdtWin.SetTextBackColor(Color(COL_TRANSPARENT)); //if last was "no fill"
            if(!pApply && (rWrtSh.HasSelection() || rReq.IsAPI()))
            {
                SvxBrushItem aBrushItem(RES_CHRATR_BACKGROUND);
                if(pItem)
                    aBrushItem.SetColor(aSet); //set the selected color
                else
                    aBrushItem.SetColor(Color(COL_TRANSPARENT));//set "no fill" color
                rWrtSh.SetAttr( aBrushItem );
            }
            else if(!pApply || pApply->nColor != SID_ATTR_CHAR_COLOR_BACKGROUND_EXT)
            {
                GetView().GetViewFrame()->GetDispatcher()->Execute(SID_ATTR_CHAR_COLOR_BACKGROUND_EXT);
            }

            rReq.Done();

        }
        break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND_EXT:
        case SID_ATTR_CHAR_COLOR_EXT:
        {
            SwEditWin& rEdtWin = GetView().GetEditWin();
            SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
            SwApplyTemplate aTempl;
            sal_Bool bSelection = rWrtSh.HasSelection();
            if(bSelection)
            {

                if(nSlot == SID_ATTR_CHAR_COLOR_BACKGROUND_EXT)
                {
                    rWrtSh.SetAttr( SvxBrushItem(
                        rEdtWin.GetTextBackColor(), RES_CHRATR_BACKGROUND) );
                }
                else
                    rWrtSh.SetAttr( SvxColorItem( rEdtWin.GetTextColor(),
                                                                RES_CHRATR_COLOR) );
            }
            else
            {
                if(!pApply || pApply->nColor != nSlot)
                    aTempl.nColor = nSlot;
                rEdtWin.SetApplyTemplate(aTempl);
            }

            rReq.Done();
        }
        break;

        case FN_NUM_BULLET_MOVEDOWN:
            if (!rWrtSh.IsAddMode())
                rWrtSh.MoveParagraph(1);
            rReq.Done();
            break;

        case FN_NUM_BULLET_MOVEUP:
            if (!rWrtSh.IsAddMode())
                rWrtSh.MoveParagraph(-1);
            rReq.Done();
            break;
        case SID_RUBY_DIALOG:
        case SID_HYPERLINK_DIALOG:
        {
            SfxRequest aReq(nSlot, SFX_CALLMODE_SLOT, SFX_APP()->GetPool());
            GetView().GetViewFrame()->ExecuteSlot( aReq);
            rReq.Ignore();
        }
        break;
    case FN_INSERT_PAGEHEADER:
    case FN_INSERT_PAGEFOOTER:
    if(pArgs && pArgs->Count())
    {
        String sStyleName;
        if(pItem)
            sStyleName = ((const SfxStringItem*)pItem)->GetValue();
        sal_Bool bOn = sal_True;
        if( SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_1, sal_False, &pItem))
            bOn = ((const SfxBoolItem*)pItem)->GetValue();
        rWrtSh.ChangeHeaderOrFooter(sStyleName, FN_INSERT_PAGEHEADER == nSlot, bOn, !rReq.IsAPI());
        rReq.Done();
    }
    break;
    case FN_READONLY_SELECTION_MODE :
        if(GetView().GetDocShell()->IsReadOnly())
        {
            rWrtSh.SetReadonlySelectionOption(
                !rWrtSh.GetViewOptions()->IsSelectionInReadonly());
            rWrtSh.ShowCrsr();
        }
    break;
    case FN_SELECTION_MODE_DEFAULT:
    case FN_SELECTION_MODE_BLOCK :
    {
        bool bSetBlockMode = !rWrtSh.IsBlockMode();
        if( pArgs && SFX_ITEM_SET == pArgs->GetItemState(nSlot, sal_False, &pItem))
            bSetBlockMode = ((const SfxBoolItem*)pItem)->GetValue();
        if( ( nSlot == FN_SELECTION_MODE_DEFAULT ) ^ bSetBlockMode )
            rWrtSh.EnterBlockMode();
        else
            rWrtSh.EnterStdMode();
        SfxBindings &rBnd = GetView().GetViewFrame()->GetBindings();
        rBnd.Invalidate(FN_STAT_SELMODE);
        rBnd.Update(FN_STAT_SELMODE);
    }
    break;
    case SID_OPEN_HYPERLINK:
    case FN_COPY_HYPERLINK_LOCATION:
    {
        SfxItemSet aSet(GetPool(),
                        RES_TXTATR_INETFMT,
                        RES_TXTATR_INETFMT);
        rWrtSh.GetCurAttr(aSet);
        if(SFX_ITEM_SET <= aSet.GetItemState( RES_TXTATR_INETFMT, sal_True ))
        {
            const SwFmtINetFmt& rINetFmt = dynamic_cast<const SwFmtINetFmt&>( aSet.Get(RES_TXTATR_INETFMT, sal_True) );
            if( nSlot == FN_COPY_HYPERLINK_LOCATION )
            {
                ::uno::Reference< datatransfer::clipboard::XClipboard > xClipboard = GetView().GetEditWin().GetClipboard();
                vcl::unohelper::TextDataObject::CopyStringTo(
                        rINetFmt.GetValue(),
                        xClipboard );
            }
            else
                rWrtSh.ClickToINetAttr(rINetFmt, URLLOAD_NOFILTER);
        }
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
    case FN_FORMAT_APPLY_HEAD1:
    {
    }
    break;
    case FN_FORMAT_APPLY_HEAD2:
    {
    }
    break;
    case FN_FORMAT_APPLY_HEAD3:
    {
    }
    break;
    case FN_FORMAT_APPLY_DEFAULT:
    {
    }
    break;
    case FN_FORMAT_APPLY_TEXTBODY:
    {
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
    default:
        OSL_ENSURE(!this, "wrong dispatcher");
        return;
    }
}

void SwTextShell::GetState( SfxItemSet &rSet )
{
    SwWrtShell &rSh = GetShell();
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
        case SID_LANGUAGE_STATUS:
            {
                // the value of used script types
                String aScriptTypesInUse( OUString::number( rSh.GetScriptType() ) );

                SvtLanguageTable aLangTable;

                // get keyboard language
                String aKeyboardLang;
                SwEditWin& rEditWin = GetView().GetEditWin();
                LanguageType nLang = rEditWin.GetInputLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aKeyboardLang = aLangTable.GetString( nLang );

                // get the language that is in use
                String aCurrentLang = OUString("*");
                nLang = SwLangHelper::GetCurrentLanguage( rSh );
                if (nLang != LANGUAGE_DONTKNOW)
                    aCurrentLang = aLangTable.GetString( nLang );

                // build sequence for status value
                uno::Sequence< OUString > aSeq( 4 );
                aSeq[0] = aCurrentLang;
                aSeq[1] = aScriptTypesInUse;
                aSeq[2] = aKeyboardLang;
                aSeq[3] = SwLangHelper::GetTextForLanguageGuessing( rSh );

                // set sequence as status value
                SfxStringListItem aItem( SID_LANGUAGE_STATUS );
                aItem.SetStringList( aSeq );
                rSet.Put( aItem, SID_LANGUAGE_STATUS );
            }
        break;

        case SID_THES:
        {
            // is there a valid selection to get text from?
            String aText;
            bool bValid = !rSh.HasSelection() ||
                    (rSh.IsSelOnePara() && !rSh.IsMultiSelection());
            // prevent context menu from showing when cursor is not in or at the end of a word
            // (GetCurWord will return the next word if there is none at the current position...)
            const sal_Int16 nWordType = ::i18n::WordType::DICTIONARY_WORD;
            bool bWord = rSh.IsInWord( nWordType ) || rSh.IsStartWord( nWordType ) || rSh.IsEndWord( nWordType );
            if (bValid && bWord)
               aText = rSh.HasSelection()? rSh.GetSelTxt() : rSh.GetCurWord();

            LanguageType nLang = rSh.GetCurLang();
            LanguageTag aLanguageTag( nLang);
            lang::Locale aLocale( aLanguageTag.getLocale());
            String aLangText( aLanguageTag.getBcp47() );

            // set word and locale to look up as status value
            String aStatusVal( aText );
            aStatusVal.AppendAscii( "#" );
            aStatusVal += aLangText;

            rSet.Put( SfxStringItem( SID_THES, aStatusVal ) );

            // disable "Thesaurus" context menu entry if there is nothing to look up
            uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
            if (aText.Len() == 0 ||
                !xThes.is() || nLang == LANGUAGE_NONE || !xThes->hasLocale( aLocale ))
                rSet.DisableItem( SID_THES );
        }
        break;

        case FN_NUMBER_NEWSTART :
            if(!rSh.GetCurNumRule())
                    rSet.DisableItem(nWhich);
            else
                rSet.Put(SfxBoolItem(FN_NUMBER_NEWSTART,
                    rSh.IsNumRuleStart()));
        break;
        case FN_EDIT_FORMULA:
        case SID_CHARMAP:
            {
                const int nType = rSh.GetSelectionType();
                if (!(nType & nsSelectionType::SEL_TXT) &&
                    !(nType & nsSelectionType::SEL_TBL) &&
                    !(nType & nsSelectionType::SEL_NUM))
                    rSet.DisableItem(nWhich);
            }
            break;

        case FN_INSERT_ENDNOTE:
        case FN_INSERT_FOOTNOTE:
        case FN_INSERT_FOOTNOTE_DLG:
            {
                const sal_uInt16 nNoType = FRMTYPE_FLY_ANY | FRMTYPE_HEADER |
                                        FRMTYPE_FOOTER  | FRMTYPE_FOOTNOTE;
                if ( (rSh.GetFrmType(0,sal_True) & nNoType) )
                    rSet.DisableItem(nWhich);
            }
            break;
        case FN_INSERT_TABLE:
            if ( rSh.GetTableFmt() ||
                    (rSh.GetFrmType(0,sal_True) & FRMTYPE_FOOTNOTE) )
                rSet.DisableItem( nWhich );
            break;

        case FN_CALCULATE:
            if ( !rSh.IsSelection() )
                rSet.DisableItem(nWhich);
            break;
        case FN_GOTO_REFERENCE:
            {
                SwField *pFld = rSh.GetCurFld();
                if ( !pFld || (pFld &&  pFld->GetTypeId() != TYP_GETREFFLD) )
                    rSet.DisableItem(nWhich);
            }
            break;
        case FN_AUTOFORMAT_AUTO:
            {
                rSet.Put( SfxBoolItem( nWhich, SvxAutoCorrCfg::Get().IsAutoFmtByInput() ));
            }
            break;

        case SID_DEC_INDENT:
        case SID_INC_INDENT:
            {
                sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
                nHtmlMode &= HTMLMODE_ON|HTMLMODE_SOME_STYLES;
                if( (nHtmlMode == HTMLMODE_ON) || !rSh.IsMoveLeftMargin(
                                        SID_INC_INDENT == nWhich, true ))
                    rSet.DisableItem( nWhich );
            }
            break;

        case FN_DEC_INDENT_OFFSET:
        case FN_INC_INDENT_OFFSET:
            {
                sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
                nHtmlMode &= HTMLMODE_ON|HTMLMODE_SOME_STYLES;
                if( (nHtmlMode == HTMLMODE_ON) ||
                    !rSh.IsMoveLeftMargin( FN_INC_INDENT_OFFSET == nWhich,
                                            false ))
                    rSet.DisableItem( nWhich );
            }
            break;

        case SID_ATTR_CHAR_COLOR2:
            {
                rSet.Put(SvxColorItem(GetView().GetEditWin().GetTextColor(), SID_ATTR_CHAR_COLOR2));
            }
            break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
            {
              if(GetView().GetEditWin().IsTextBackColorTransparent())
                  rSet.Put(SvxColorItem(Color(COL_TRANSPARENT), SID_ATTR_CHAR_COLOR_BACKGROUND));
              else
                    rSet.Put(SvxColorItem(GetView().GetEditWin().GetTextBackColor(), SID_ATTR_CHAR_COLOR_BACKGROUND));
            }
            break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND_EXT:
        case SID_ATTR_CHAR_COLOR_EXT:
            {
                SwEditWin& rEdtWin = GetView().GetEditWin();
                SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
                rSet.Put(SfxBoolItem(nWhich, pApply && pApply->nColor == nWhich));
            }
            break;
        case FN_INSERT_BOOKMARK:
            if( rSh.IsTableMode() )
                rSet.DisableItem( nWhich );
            break;

        case FN_INSERT_PAGEHEADER:
        case FN_INSERT_PAGEFOOTER:
            {
                rSet.Put( SfxObjectShellItem( nWhich, GetView().GetDocShell() ));
            }
            break;
            case FN_TABLE_SORT_DIALOG:
            case FN_SORTING_DLG:
                if(!rSh.HasSelection() ||
                        (FN_TABLE_SORT_DIALOG == nWhich && !rSh.GetTableFmt()))
                    rSet.DisableItem( nWhich );
            break;
            case SID_RUBY_DIALOG:
            {
                SvtCJKOptions aCJKOptions;
                if(!aCJKOptions.IsRubyEnabled())
                {
                    GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_False );
                    rSet.DisableItem(nWhich);
                }
                else
                    GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_True );
                break;
            }
            //no break!
            case SID_HYPERLINK_DIALOG:
                if( GetView().GetDocShell()->IsReadOnly() ||
                    (!GetView().GetViewFrame()->HasChildWindow(nWhich) &&
                     rSh.HasReadonlySel()) )
                    rSet.DisableItem(nWhich);
                else
                    rSet.Put(SfxBoolItem( nWhich, 0 != GetView().
                                GetViewFrame()->GetChildWindow( nWhich ) ));
                break;
            case FN_EDIT_HYPERLINK:
            case FN_COPY_HYPERLINK_LOCATION:
            {
                SfxItemSet aSet(GetPool(),
                                RES_TXTATR_INETFMT,
                                RES_TXTATR_INETFMT);
                rSh.GetCurAttr(aSet);
                if(SFX_ITEM_SET > aSet.GetItemState( RES_TXTATR_INETFMT, sal_True ) || rSh.HasReadonlySel())
                {
                    rSet.DisableItem(nWhich);
                }
            }
            break;
            case FN_REMOVE_HYPERLINK:
            {
                SfxItemSet aSet(GetPool(),
                                RES_TXTATR_INETFMT,
                                RES_TXTATR_INETFMT);
                rSh.GetCurAttr(aSet);

        // If a hyperlink is selected, either alone or along with other text...
                if( ((SFX_ITEM_DONTCARE & aSet.GetItemState( RES_TXTATR_INETFMT, sal_True )) == 0) || rSh.HasReadonlySel())
                {
                    rSet.DisableItem(nWhich);
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
                    GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_False );
                    rSet.DisableItem(nWhich);
                }
                else
                    GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, sal_True );
            }
            break;
            case FN_READONLY_SELECTION_MODE :
                if(!GetView().GetDocShell()->IsReadOnly())
                    rSet.DisableItem( nWhich );
                else
                {
                    rSet.Put(SfxBoolItem(nWhich, rSh.GetViewOptions()->IsSelectionInReadonly()));
                }
            break;
            case FN_SELECTION_MODE_DEFAULT:
            case FN_SELECTION_MODE_BLOCK :
                    rSet.Put(SfxBoolItem(nWhich, (nWhich == FN_SELECTION_MODE_DEFAULT) != rSh.IsBlockMode()));
            break;
            case  SID_OPEN_HYPERLINK:
            {
                SfxItemSet aSet(GetPool(),
                                RES_TXTATR_INETFMT,
                                RES_TXTATR_INETFMT);
                rSh.GetCurAttr(aSet);
                if(SFX_ITEM_SET > aSet.GetItemState( RES_TXTATR_INETFMT, sal_False ))
                    rSet.DisableItem(nWhich);
            }
            break;
            case  SID_OPEN_SMARTTAGMENU:
            {
                 uno::Sequence< OUString > aSmartTagTypes;
                 uno::Sequence< uno::Reference< container::XStringKeyMap > > aStringKeyMaps;
                 uno::Reference<text::XTextRange> xRange;

                 rSh.GetSmartTagTerm( aSmartTagTypes, aStringKeyMaps, xRange );

                 if ( xRange.is() && aSmartTagTypes.getLength() )
                 {
                     uno::Sequence < uno::Sequence< uno::Reference< smarttags::XSmartTagAction > > > aActionComponentsSequence;
                     uno::Sequence < uno::Sequence< sal_Int32 > > aActionIndicesSequence;

                     const SmartTagMgr& rSmartTagMgr = SwSmartTagMgr::Get();
                     rSmartTagMgr.GetActionSequences( aSmartTagTypes,
                                                      aActionComponentsSequence,
                                                      aActionIndicesSequence );

                     uno::Reference <frame::XController> xController = GetView().GetController();
                     const lang::Locale aLocale( SW_BREAKITER()->GetLocale( GetAppLanguageTag() ) );
                     const OUString aApplicationName( rSmartTagMgr.GetApplicationName() );
                     const OUString aRangeText = xRange->getString();

                     const SvxSmartTagItem aItem( nWhich,
                                                  aActionComponentsSequence,
                                                  aActionIndicesSequence,
                                                  aStringKeyMaps,
                                                  xRange,
                                                  xController,
                                                  aLocale,
                                                  aApplicationName,
                                                  aRangeText );

                     rSet.Put( aItem );
                 }
                 else
                     rSet.DisableItem(nWhich);
            }
            break;
            case FN_NUM_NUMBERING_ON:
                rSet.Put(SfxBoolItem(FN_NUM_NUMBERING_ON,rSh.SelectionHasNumber()));
            break;
            case FN_NUM_BULLET_ON:
                rSet.Put(SfxBoolItem(FN_NUM_BULLET_ON,rSh.SelectionHasBullet()));
            break;
            case FN_BUL_NUM_RULE_INDEX:
            case FN_NUM_NUM_RULE_INDEX:
        {
            SwNumRule* pCurRule = (SwNumRule*)(GetShell().GetCurNumRule());
            sal_uInt16  nActNumLvl = (sal_uInt16)0xFFFF;
            rSet.Put(SfxUInt16Item(FN_NUM_NUM_RULE_INDEX,DEFAULT_NONE));
            rSet.Put(SfxUInt16Item(FN_BUL_NUM_RULE_INDEX,DEFAULT_NONE));
            if( pCurRule )
            {
                nActNumLvl = GetShell().GetNumLevel();
                if( nActNumLvl < MAXLEVEL )
                {
                    nActNumLvl = 1<<nActNumLvl;
                }
                SvxNumRule aSvxRule = pCurRule->MakeSvxNumRule();
                if ( GetShell().HasBullet())
                {
                    rSet.Put(SfxUInt16Item(FN_BUL_NUM_RULE_INDEX,(sal_uInt16)0xFFFF));
                    rSet.Put(SfxUInt16Item(FN_NUM_NUM_RULE_INDEX,(sal_uInt16)0xFFFF));
                    NBOTypeMgrBase* pBullets = NBOutlineTypeMgrFact::CreateInstance(eNBOType::MIXBULLETS);
                    if ( pBullets )
                    {
                        sal_uInt16 nBulIndex = pBullets->GetNBOIndexForNumRule(aSvxRule,nActNumLvl);
                        rSet.Put(SfxUInt16Item(FN_BUL_NUM_RULE_INDEX,nBulIndex));
                    }
                }else if ( GetShell().HasNumber() )
                {
                    rSet.Put(SfxUInt16Item(FN_BUL_NUM_RULE_INDEX,(sal_uInt16)0xFFFF));
                    rSet.Put(SfxUInt16Item(FN_NUM_NUM_RULE_INDEX,(sal_uInt16)0xFFFF));
                    NBOTypeMgrBase* pNumbering = NBOutlineTypeMgrFact::CreateInstance(eNBOType::NUMBERING);
                    if ( pNumbering )
                    {
                        sal_uInt16 nBulIndex = pNumbering->GetNBOIndexForNumRule(aSvxRule,nActNumLvl);
                        rSet.Put(SfxUInt16Item(FN_NUM_NUM_RULE_INDEX,nBulIndex));
                    }
                }
            }
        }
            break;
            case FN_NUM_CONTINUE:
            {
                {
                    // #i86492#
                    // Search also for bullet list
                    String aDummy;
                    const SwNumRule* pRule =
                            rSh.SearchNumRule( false, true, false, -1, aDummy );
                    if ( !pRule )
                    {
                        pRule = rSh.SearchNumRule( false, false, false, -1, aDummy );
                    }
                    if ( !pRule )
                        rSet.DisableItem(nWhich);
                }
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
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
