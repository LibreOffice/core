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

#include <hintids.hxx>
#include <cmdid.h>
#include <helpids.h>
#include <comphelper/lok.hxx>

#include <i18nutil/unicode.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/langitem.hxx>
#include <svtools/langtab.hxx>
#include <svl/slstitm.hxx>
#include <svl/grabbagitem.hxx>
#include <string.h>
#include <svl/stritem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svl/whiter.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objitem.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/weld.hxx>
#include <sfx2/request.hxx>
#include <svl/eitem.hxx>
#include <svl/macitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/svxacorr.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <IDocumentDrawModelAccess.hxx>
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
#include <swundo.hxx>
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
#include <fldmgr.hxx>
#include <globals.hrc>
#include <strings.hrc>
#include <app.hrc>
#include <paratr.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/app.hxx>
#include <breakit.hxx>
#include <SwSmartTagMgr.hxx>
#include <editeng/acorrcfg.hxx>
#include <swabstdlg.hxx>
#include <IDocumentStatistics.hxx>
#include <sfx2/sfxdlg.hxx>
#include <unotools/lingucfg.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <editeng/unolingu.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <doc.hxx>
#include <drawdoc.hxx>
#include <view.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <sfx2/objface.hxx>
#include <langhelper.hxx>
#include <uiitems.hxx>
#include <tools/diagnose_ex.h>
#include <svx/nbdtmgfact.hxx>
#include <svx/nbdtmg.hxx>
#include <SwRewriter.hxx>
#include <svx/svdmodel.hxx>
#include <svx/drawitem.hxx>
#include <numrule.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace com::sun::star::style;
using namespace svx::sidebar;

static void sw_CharDialogResult(const SfxItemSet* pSet, SwWrtShell &rWrtSh, std::shared_ptr<SfxItemSet> const & pCoreSet, bool bSel, bool bSelectionPut, SfxRequest *pReq);

void sw_CharDialog(SwWrtShell &rWrtSh, bool bUseDialog, sal_uInt16 nSlot, const SfxItemSet *pArgs, SfxRequest *pReq )
{
    FieldUnit eMetric = ::GetDfltMetric(dynamic_cast<SwWebView*>( &rWrtSh.GetView()) != nullptr );
    SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
    std::shared_ptr<SfxItemSet> pCoreSet(new SfxItemSet(
        rWrtSh.GetView().GetPool(),
        svl::Items<
            RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
            RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
            RES_BACKGROUND, RES_SHADOW,
            XATTR_FILLSTYLE, XATTR_FILLCOLOR,
            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
            SID_HTML_MODE, SID_HTML_MODE,
            SID_ATTR_CHAR_WIDTH_FIT_TO_LINE, SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,
            FN_PARAM_SELECTION, FN_PARAM_SELECTION>{}));
    rWrtSh.GetCurAttr(*pCoreSet);

    bool bSel = rWrtSh.HasSelection();
    bool bSelectionPut = false;
    if(bSel || rWrtSh.IsInWord())
    {
        if(!bSel)
        {
            rWrtSh.StartAction();
            rWrtSh.Push();
            if(!rWrtSh.SelectTextAttr( RES_TXTATR_INETFMT ))
                rWrtSh.SelWrd();
        }
        pCoreSet->Put(SfxStringItem(FN_PARAM_SELECTION, rWrtSh.GetSelText()));
        bSelectionPut = true;
        if(!bSel)
        {
            rWrtSh.Pop(SwCursorShell::PopMode::DeleteCurrent);
            rWrtSh.EndAction();
        }
    }
    pCoreSet->Put(SfxUInt16Item(SID_ATTR_CHAR_WIDTH_FIT_TO_LINE, rWrtSh.GetScalingOfSelectedText()));

    ::ConvertAttrCharToGen(*pCoreSet);

    // Setting the BoxInfo
    ::PrepareBoxInfo(*pCoreSet, rWrtSh);

    pCoreSet->Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(rWrtSh.GetView().GetDocShell())));
    VclPtr<SfxAbstractTabDialog> pDlg;
    if ( bUseDialog && GetActiveView() )
    {
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        pDlg.reset(pFact->CreateSwCharDlg(rWrtSh.GetView().GetWindow(), rWrtSh.GetView(), *pCoreSet, SwCharDlgMode::Std));

        if (nSlot == FN_INSERT_HYPERLINK)
            pDlg->SetCurPageId("hyperlink");
        else if (nSlot == SID_CHAR_DLG_EFFECT)
            pDlg->SetCurPageId("fonteffects");
        else if (nSlot == SID_ATTR_CHAR_FONT || nSlot == SID_CHAR_DLG_FOR_PARAGRAPH)
            pDlg->SetCurPageId("font");
        else if (pReq)
        {
            const SfxStringItem* pItem = (*pReq).GetArg<SfxStringItem>(FN_PARAM_1);
            if (pItem)
                pDlg->SetCurPageId(OUStringToOString(pItem->GetValue(), RTL_TEXTENCODING_UTF8));
        }
    }

    if (bUseDialog)
    {
        std::shared_ptr<SfxRequest> pRequest;
        if (pReq)
        {
            pRequest.reset(new SfxRequest(*pReq));
            pReq->Ignore(); // the 'old' request is not relevant any more
        }
        pDlg->StartExecuteAsync([pDlg, &rWrtSh, pCoreSet, bSel, bSelectionPut, pRequest](sal_Int32 nResult){
            if (nResult == RET_OK)
            {
                sw_CharDialogResult(pDlg->GetOutputItemSet(), rWrtSh, pCoreSet, bSel, bSelectionPut, pRequest.get());
            }
        });
    }
    else if (pArgs)
    {
        sw_CharDialogResult(pArgs, rWrtSh, pCoreSet, bSel, bSelectionPut, pReq);
    }
}

static void sw_CharDialogResult(const SfxItemSet* pSet, SwWrtShell &rWrtSh, std::shared_ptr<SfxItemSet> const & pCoreSet, bool bSel, bool bSelectionPut, SfxRequest *pReq)
{
    SfxItemSet aTmpSet( *pSet );
    ::ConvertAttrGenToChar(aTmpSet, *pCoreSet);

    const SfxPoolItem* pSelectionItem;
    bool bInsert = false;
    sal_Int32 nInsert = 0;

    // The old item is for unknown reasons back in the set again.
    if( !bSelectionPut && SfxItemState::SET == aTmpSet.GetItemState(FN_PARAM_SELECTION, false, &pSelectionItem) )
    {
        OUString sInsert = static_cast<const SfxStringItem*>(pSelectionItem)->GetValue();
        bInsert = !sInsert.isEmpty();
        if(bInsert)
        {
            nInsert = sInsert.getLength();
            rWrtSh.StartAction();
            rWrtSh.Insert( sInsert );
            rWrtSh.SetMark();
            rWrtSh.ExtendSelection(false, sInsert.getLength());
            SfxRequest aReq( rWrtSh.GetView().GetViewFrame(), FN_INSERT_STRING );
            aReq.AppendItem( SfxStringItem( FN_INSERT_STRING, sInsert ) );
            aReq.Done();
            SfxRequest aReq1( rWrtSh.GetView().GetViewFrame(), FN_CHAR_LEFT );
            aReq1.AppendItem( SfxInt32Item(FN_PARAM_MOVE_COUNT, nInsert) );
            aReq1.AppendItem( SfxBoolItem(FN_PARAM_MOVE_SELECTION, true) );
            aReq1.Done();
        }
    }
    aTmpSet.ClearItem(FN_PARAM_SELECTION);

    SwTextFormatColl* pColl = rWrtSh.GetCurTextFormatColl();
    if(bSel && rWrtSh.IsSelFullPara() && pColl && pColl->IsAutoUpdateFormat())
    {
        rWrtSh.AutoUpdatePara(pColl, aTmpSet);
    }
    else
        rWrtSh.SetAttrSet( aTmpSet );
    if (pReq)
        pReq->Done(aTmpSet);
    if(bInsert)
    {
        SfxRequest aReq1( rWrtSh.GetView().GetViewFrame(), FN_CHAR_RIGHT );
        aReq1.AppendItem( SfxInt32Item(FN_PARAM_MOVE_COUNT, nInsert) );
        aReq1.AppendItem( SfxBoolItem(FN_PARAM_MOVE_SELECTION, false) );
        aReq1.Done();
        rWrtSh.SwapPam();
        rWrtSh.ClearMark();
        rWrtSh.DontExpandFormat();
        rWrtSh.EndAction();
    }

}

static short lcl_AskRedlineFlags(weld::Window *pWin)
{
    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pWin, "modules/swriter/ui/queryredlinedialog.ui"));
    std::unique_ptr<weld::MessageDialog> xQBox(xBuilder->weld_message_dialog("QueryRedlineDialog"));
    return xQBox->run();
}

static void sw_ParagraphDialogResult(SfxItemSet* pSet, SwWrtShell &rWrtSh, SfxRequest& rReq, SwPaM* pPaM)
{
    if (!pSet)
        return;

    rReq.Done( *pSet );
    ::SfxToSwPageDescAttr( rWrtSh, *pSet );
    // #i56253#
    // enclose all undos.
    // Thus, check conditions, if actions will be performed.
    const bool bUndoNeeded( pSet->Count() ||
            SfxItemState::SET == pSet->GetItemState(FN_NUMBER_NEWSTART) ||
            SfxItemState::SET == pSet->GetItemState(FN_NUMBER_NEWSTART_AT) );
    if ( bUndoNeeded )
    {
        rWrtSh.StartUndo( SwUndoId::INSATTR );
    }
    if( pSet->Count() )
    {
        rWrtSh.StartAction();
        const SfxPoolItem* pItem = nullptr;
        if ( SfxItemState::SET == pSet->GetItemState(FN_DROP_TEXT, false, &pItem) )
        {
            if ( !static_cast<const SfxStringItem*>(pItem)->GetValue().isEmpty() )
                rWrtSh.ReplaceDropText(static_cast<const SfxStringItem*>(pItem)->GetValue(), pPaM);
        }
        rWrtSh.SetAttrSet(*pSet, SetAttrMode::DEFAULT, pPaM);
        rWrtSh.EndAction();
        SwTextFormatColl* pColl = rWrtSh.GetPaMTextFormatColl(pPaM);
        if(pColl && pColl->IsAutoUpdateFormat())
        {
            rWrtSh.AutoUpdatePara(pColl, *pSet, pPaM);
        }
    }

    if( SfxItemState::SET == pSet->GetItemState(FN_NUMBER_NEWSTART) )
    {
        //SetNumRuleStart(true) restarts the numbering at the value
        //that is defined at the starting point of the numbering level
        //otherwise the SetNodeNumStart() value determines the start
        //if it's set to something different than USHRT_MAX

        bool bStart = static_cast<const SfxBoolItem&>(pSet->Get(FN_NUMBER_NEWSTART)).GetValue();

        // Default value for restart value has to be USHRT_MAX
        // in order to indicate that the restart value of the list
        // style has to be used on restart.
        sal_uInt16 nNumStart = USHRT_MAX;
        if( SfxItemState::SET == pSet->GetItemState(FN_NUMBER_NEWSTART_AT) )
        {
            nNumStart = static_cast<const SfxUInt16Item&>(pSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
        }
        rWrtSh.SetNumRuleStart(bStart, pPaM);
        rWrtSh.SetNodeNumStart(nNumStart);
    }
    else if( SfxItemState::SET == pSet->GetItemState(FN_NUMBER_NEWSTART_AT) )
    {
        rWrtSh.SetNodeNumStart(static_cast<const SfxUInt16Item&>(pSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue());
        rWrtSh.SetNumRuleStart(false, pPaM);
    }
    // #i56253#
    if ( bUndoNeeded )
    {
        rWrtSh.EndUndo( SwUndoId::INSATTR );
    }
}

void SwTextShell::Execute(SfxRequest &rReq)
{
    bool bUseDialog = true;
    const SfxItemSet *pArgs = rReq.GetArgs();
    SwWrtShell& rWrtSh = GetShell();
    const SfxPoolItem* pItem = nullptr;
    const sal_uInt16 nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), false, &pItem);
    switch( nSlot )
    {
        case SID_UNICODE_NOTATION_TOGGLE:
        {
            long nMaxUnits = 256;
            sal_Int32 nSelLength = rWrtSh.GetSelText().getLength();
            if( rWrtSh.IsSelection() && !rWrtSh.IsMultiSelection() && (nSelLength < nMaxUnits) )
                nMaxUnits = nSelLength;

            long index = 0;
            ToggleUnicodeCodepoint aToggle;
            while( nMaxUnits-- && aToggle.AllowMoreInput(rWrtSh.GetChar(true, index-1)) )
                --index;

            OUString sReplacement = aToggle.ReplacementString();
            if( !sReplacement.isEmpty() )
            {
                SwRewriter aRewriter;
                aRewriter.AddRule( UndoArg1, aToggle.StringToReplace() );
                aRewriter.AddRule( UndoArg2, SwResId(STR_YIELDS) );
                aRewriter.AddRule( UndoArg3, sReplacement );
                rWrtSh.StartUndo(SwUndoId::REPLACE, &aRewriter);
                rWrtSh.GetCursor()->Normalize(false);
                rWrtSh.ClearMark();
                for( sal_uInt32 i=aToggle.CharsToDelete(); i > 0; --i )
                    rWrtSh.DelLeft();
                rWrtSh.Insert2( sReplacement );
                rWrtSh.EndUndo(SwUndoId::REPLACE, &aRewriter);
            }
        }
        break;

        case SID_LANGUAGE_STATUS:
        {
            // get the language
            OUString aNewLangText;
            const SfxStringItem* pItem2 = rReq.GetArg<SfxStringItem>(SID_LANGUAGE_STATUS);
            if (pItem2)
                aNewLangText = pItem2->GetValue();

            //!! Remember the view frame right now...
            //!! (call to GetView().GetViewFrame() will break if the
            //!! SwTextShell got destroyed meanwhile.)
            SfxViewFrame *pViewFrame = GetView().GetViewFrame();

            if (aNewLangText == "*")
            {
                // open the dialog "Tools/Options/Language Settings - Language"
                // to set the documents default language
                SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateVclDialog( GetView().GetWindow(), SID_LANGUAGE_OPTIONS ));
                pDlg->Execute();
            }
            else
            {
                //!! We have to use StartAction / EndAction bracketing in
                //!! order to prevent possible destruction of the SwTextShell
                //!! due to the selection changes coming below.
                rWrtSh.StartAction();
                // prevent view from jumping because of (temporary) selection changes
                rWrtSh.LockView( true );

                // setting the new language...
                if (!aNewLangText.isEmpty())
                {
                    const OUString aSelectionLangPrefix("Current_");
                    const OUString aParagraphLangPrefix("Paragraph_");
                    const OUString aDocumentLangPrefix("Default_");
                    const OUString aStrNone("LANGUAGE_NONE");
                    const OUString aStrResetLangs("RESET_LANGUAGES");

                    SfxItemSet aCoreSet( GetPool(),
                            svl::Items<RES_CHRATR_LANGUAGE,        RES_CHRATR_LANGUAGE,
                            RES_CHRATR_CJK_LANGUAGE,    RES_CHRATR_CJK_LANGUAGE,
                            RES_CHRATR_CTL_LANGUAGE,    RES_CHRATR_CTL_LANGUAGE>{} );

                    sal_Int32 nPos = 0;
                    bool bForSelection = true;
                    bool bForParagraph = false;
                    if (-1 != (nPos = aNewLangText.indexOf( aSelectionLangPrefix )))
                    {
                        // ... for the current selection
                        aNewLangText = aNewLangText.replaceAt(nPos, aSelectionLangPrefix.getLength(), "");
                        bForSelection = true;
                    }
                    else if (-1 != (nPos = aNewLangText.indexOf(aParagraphLangPrefix)))
                    {
                        // ... for the current paragraph language
                        aNewLangText = aNewLangText.replaceAt(nPos, aParagraphLangPrefix.getLength(), "");
                        bForSelection = true;
                        bForParagraph = true;
                    }
                    else if (-1 != (nPos = aNewLangText.indexOf(aDocumentLangPrefix)))
                    {
                        // ... as default document language
                        aNewLangText = aNewLangText.replaceAt(nPos, aDocumentLangPrefix.getLength(), "");
                        bForSelection = false;
                    }

                    if (bForParagraph || !bForSelection)
                    {
                        rWrtSh.Push(); // save selection for later restoration
                        rWrtSh.ClearMark(); // fdo#67796: invalidate table crsr
                    }

                    if (bForParagraph)
                        SwLangHelper::SelectCurrentPara( rWrtSh );

                    if (!bForSelection) // document language to be changed...
                    {
                        rWrtSh.SelAll();
                        rWrtSh.ExtendedSelectAll();
                    }

                    rWrtSh.StartUndo( ( !bForParagraph && !bForSelection ) ? SwUndoId::SETDEFTATTR : SwUndoId::EMPTY );
                    if (aNewLangText == aStrNone)
                        SwLangHelper::SetLanguage_None( rWrtSh, bForSelection, aCoreSet );
                    else if (aNewLangText == aStrResetLangs)
                        SwLangHelper::ResetLanguages( rWrtSh );
                    else
                        SwLangHelper::SetLanguage( rWrtSh, aNewLangText, bForSelection, aCoreSet );
                    rWrtSh.EndUndo();

                    if (bForParagraph || !bForSelection)
                    {
                        rWrtSh.Pop(SwCursorShell::PopMode::DeleteCurrent); // restore selection...
                    }
                }

                rWrtSh.LockView( false );
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
            OUString aReplaceText;
            const SfxStringItem* pItem2 = rReq.GetArg<SfxStringItem>(SID_THES);
            if (pItem2)
                aReplaceText = pItem2->GetValue();
            if (!aReplaceText.isEmpty())
            {
                SwView &rView2 = rWrtSh.GetView();
                const bool bSelection = rWrtSh.HasSelection();
                const OUString aLookUpText = rView2.GetThesaurusLookUpText( bSelection );
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
            OUString aStr;
            const SfxStringItem* pFont = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
            const SfxStringItem* pNameItem = rReq.GetArg<SfxStringItem>(nSlot);
            if ( pNameItem )
                aStr = pNameItem->GetValue();
            bool bFont = pFont && !pFont->GetValue().isEmpty();
            rWrtSh.StartUndo( SwUndoId::UI_INSERT_FOOTNOTE );
            rWrtSh.InsertFootnote( aStr, nSlot == FN_INSERT_ENDNOTE, !bFont );
            if ( bFont )
            {
                rWrtSh.Left( CRSR_SKIP_CHARS, true, 1, false );
                SfxItemSet aSet( rWrtSh.GetAttrPool(), svl::Items<RES_CHRATR_FONT, RES_CHRATR_FONT>{} );
                rWrtSh.GetCurAttr( aSet );
                rWrtSh.SetAttrSet( aSet, SetAttrMode::DONTEXPAND );
                rWrtSh.ResetSelect(nullptr, false);
                rWrtSh.EndSelect();
                rWrtSh.GotoFootnoteText();
            }
            rWrtSh.EndUndo( SwUndoId::UI_INSERT_FOOTNOTE );
            rReq.Done();
        }
        break;
        case FN_INSERT_FOOTNOTE_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractInsFootNoteDlg> pDlg(pFact->CreateInsFootNoteDlg(
                GetView().GetFrameWeld(), rWrtSh));
            pDlg->SetHelpId(GetStaticInterface()->GetSlot(nSlot)->GetCommand());
            if ( pDlg->Execute() == RET_OK )
            {
                const sal_uInt16 nId = pDlg->IsEndNote() ? FN_INSERT_ENDNOTE : FN_INSERT_FOOTNOTE;
                SfxRequest aReq( GetView().GetViewFrame(), nId );
                if ( !pDlg->GetStr().isEmpty() )
                    aReq.AppendItem( SfxStringItem( nId, pDlg->GetStr() ) );
                if ( !pDlg->GetFontName().isEmpty() )
                    aReq.AppendItem( SfxStringItem( FN_PARAM_1, pDlg->GetFontName() ) );
                ExecuteSlot( aReq );
            }

            rReq.Ignore();
        }
        break;
        case FN_FORMAT_FOOTNOTE_DLG:
        case FN_FORMAT_CURRENT_FOOTNOTE_DLG:
        {
            GetView().ExecFormatFootnote();
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
            const sal_uInt16 *pUShorts = aResetableSetRange;
            while (*pUShorts)
            {
                for (sal_uInt16 i = pUShorts[0]; i <= pUShorts[1]; ++i)
                    aAttribs.insert( aAttribs.end(), i );
                pUShorts += 2;
            }
            // we don't want to change writing direction.
            aAttribs.erase( RES_FRAMEDIR );
            rWrtSh.ResetAttr( aAttribs );

            // also clear the direct formatting flag inside SwTableBox(es)
            GetView().GetDocShell()->GetFEShell()->UpdateTableStyleFormatting(nullptr, true);

            rReq.Done();
            break;
        }
        case FN_INSERT_BREAK_DLG:
        {
            sal_uInt16 nKind=0;
            ::boost::optional<sal_uInt16> oPageNumber;
            OUString aTemplateName;
            if ( pItem )
            {
                nKind = static_cast<const SfxInt16Item*>(pItem)->GetValue();
                const SfxStringItem* pTemplate = rReq.GetArg<SfxStringItem>(FN_PARAM_1);
                const SfxUInt16Item* pNumber = rReq.GetArg<SfxUInt16Item>(FN_PARAM_2);
                const SfxBoolItem* pIsNumberFilled = rReq.GetArg<SfxBoolItem>(FN_PARAM_3);
                if ( pTemplate )
                    aTemplateName = pTemplate->GetValue();
                if ( pNumber && pIsNumberFilled && pIsNumberFilled->GetValue() )
                    oPageNumber = pNumber->GetValue();
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractSwBreakDlg> pDlg(pFact->CreateSwBreakDlg(GetView().GetFrameWeld(), rWrtSh));
                if ( pDlg->Execute() == RET_OK )
                {
                    nKind = pDlg->GetKind();
                    aTemplateName = pDlg->GetTemplateName();
                    oPageNumber = pDlg->GetPageNumber();

                    bool bIsNumberFilled = false;
                    sal_uInt16 nPageNumber = 0;

                    if (oPageNumber)
                    {
                        bIsNumberFilled = true;
                        nPageNumber = oPageNumber.get();
                    }

                    rReq.AppendItem( SfxInt16Item ( FN_INSERT_BREAK_DLG, nKind ) );
                    rReq.AppendItem( SfxStringItem( FN_PARAM_1, aTemplateName ) );
                    rReq.AppendItem( SfxUInt16Item( FN_PARAM_2, nPageNumber ) );
                    rReq.AppendItem( SfxBoolItem  ( FN_PARAM_3, bIsNumberFilled ) );
                    rReq.Done();
                }
                else
                    rReq.Ignore();
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
                    if( !aTemplateName.isEmpty() )
                        rWrtSh.InsertPageBreak( &aTemplateName, oPageNumber );
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
                OUString sName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                rWrtSh.SetBookmark( vcl::KeyCode(), sName );
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSwInsertBookmarkDlg( GetView().GetWindow(), rWrtSh, rReq ));
                pDlg->Execute();
            }

            break;
        }
        case FN_DELETE_BOOKMARK:
        {
            if ( pItem )
            {
                IDocumentMarkAccess* const pMarkAccess = rWrtSh.getIDocumentMarkAccess();
                pMarkAccess->deleteMark( pMarkAccess->findMark(static_cast<const SfxStringItem*>(pItem)->GetValue()) );
            }
            break;
        }
        case FN_AUTOFORMAT_REDLINE_APPLY:
        {
            SvxSwAutoFormatFlags aFlags(SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags());
            // This must always be false for the postprocessing.
            aFlags.bAFormatByInput = false;
            aFlags.bWithRedlining = true;
            rWrtSh.AutoFormat( &aFlags );
            aFlags.bWithRedlining = false;

            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            if (pVFrame->HasChildWindow(FN_REDLINE_ACCEPT))
                pVFrame->ToggleChildWindow(FN_REDLINE_ACCEPT);

            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractSwModalRedlineAcceptDlg> pDlg(pFact->CreateSwModalRedlineAcceptDlg(&GetView().GetEditWin()));

            switch (lcl_AskRedlineFlags(GetView().GetFrameWeld()))
            {
                case RET_OK:
                {
                    pDlg->AcceptAll(true);
                    SfxRequest aReq( pVFrame, FN_AUTOFORMAT_APPLY );
                    aReq.Done();
                    rReq.Ignore();
                    break;
                }

                case RET_CANCEL:
                    pDlg->AcceptAll(false);
                    rReq.Ignore();
                    break;

                case 2:
                    pDlg->Execute();
                    rReq.Done();
                    break;
            }
        }
        break;

        case FN_AUTOFORMAT_APPLY:
        {
            SvxSwAutoFormatFlags aFlags(SvxAutoCorrCfg::Get().GetAutoCorrect()->GetSwFlags());
            // This must always be false for the postprocessing.
            aFlags.bAFormatByInput = false;
            rWrtSh.AutoFormat( &aFlags );
            rReq.Done();
        }
        break;
        case FN_AUTOFORMAT_AUTO:
        {
            SvxAutoCorrCfg& rACfg = SvxAutoCorrCfg::Get();
            bool bSet = pItem ? static_cast<const SfxBoolItem*>(pItem)->GetValue() : !rACfg.IsAutoFormatByInput();
            if( bSet != rACfg.IsAutoFormatByInput() )
            {
                rACfg.SetAutoFormatByInput( bSet );
                rACfg.Commit();
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
            rWrtSh.AutoCorrect( *SvxAutoCorrCfg::Get().GetAutoCorrect(), ' ' );
            rReq.Done();
        }
        break;
        case FN_TABLE_SORT_DIALOG:
        case FN_SORTING_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSwSortingDialog(GetView().GetFrameWeld(), rWrtSh));
            pDlg->Execute();
            rReq.Done();
        }
        break;
        case FN_NUMBERING_OUTLINE_DLG:
        {
            GetView().ExecNumberingOutline(GetPool());
            rReq.Done();
        }
            break;
        case FN_CALCULATE:
            {
                rtl::Reference<SwTransferable> pTransfer = new SwTransferable( rWrtSh );
                pTransfer->CalculateAndCopy();
                rReq.Done();
            }
            break;
        case FN_GOTO_REFERENCE:
        {
            SwField *pField = rWrtSh.GetCurField();
            if(pField && pField->GetTypeId() == TYP_GETREFFLD)
            {
                rWrtSh.StartAllAction();
                rWrtSh.SwCursorShell::GotoRefMark( static_cast<SwGetRefField*>(pField)->GetSetRefName(),
                                    static_cast<SwGetRefField*>(pField)->GetSubType(),
                                    static_cast<SwGetRefField*>(pField)->GetSeqNo() );
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
                    pVFrame->GetBindings().InvalidateAll( true );
                }

                OUString sFormula(static_cast<const SfxStringItem*>(pItem)->GetValue());
                SwFieldMgr aFieldMgr;
                rWrtSh.StartAllAction();
                bool bDelSel = rWrtSh.HasSelection();
                if( bDelSel )
                {
                    rWrtSh.StartUndo( SwUndoId::START );
                    rWrtSh.DelRight();
                }
                else
                {
                    rWrtSh.EnterStdMode();
                }

                if( !bDelSel && aFieldMgr.GetCurField() && TYP_FORMELFLD == aFieldMgr.GetCurTypeId() )
                    aFieldMgr.UpdateCurField( aFieldMgr.GetCurField()->GetFormat(), OUString(), sFormula );
                else if( !sFormula.isEmpty() )
                {
                    if( rWrtSh.IsCursorInTable() )
                    {
                        SfxItemSet aSet( rWrtSh.GetAttrPool(), svl::Items<RES_BOXATR_FORMULA, RES_BOXATR_FORMULA>{} );
                        aSet.Put( SwTableBoxFormula( sFormula ));
                        rWrtSh.SetTableBoxFormulaAttrs( aSet );
                        rWrtSh.UpdateTable();
                    }
                    else
                    {
                        SvNumberFormatter* pFormatter = rWrtSh.GetNumberFormatter();
                        const sal_uInt32 nSysNumFormat = pFormatter->GetFormatIndex( NF_NUMBER_STANDARD, LANGUAGE_SYSTEM);
                        SwInsertField_Data aData(TYP_FORMELFLD, nsSwGetSetExpType::GSE_FORMULA, OUString(), sFormula, nSysNumFormat);
                        aFieldMgr.InsertField(aData);
                    }
                }

                if( bDelSel )
                    rWrtSh.EndUndo( SwUndoId::END );
                rWrtSh.EndAllAction();
                rReq.Done();
            }
            else
            {
                rWrtSh.EndAllTableBoxEdit();
                pVFrame->ToggleChildWindow( nId );
                if( !pVFrame->HasChildWindow( nId ) )
                    pVFrame->GetBindings().InvalidateAll( true );
                rReq.Ignore();
            }
        }

        break;
        case FN_TABLE_UNSET_READ_ONLY:
        {
            rWrtSh.UnProtectTables();
        }
        break;
        case FN_EDIT_HYPERLINK:
            GetView().GetViewFrame()->SetChildWindow(SID_HYPERLINK_DIALOG, true);
        break;
        case FN_REMOVE_HYPERLINK:
        {
            bool bSel = rWrtSh.HasSelection();
            if(!bSel)
            {
                rWrtSh.StartAction();
                rWrtSh.Push();
                if(!rWrtSh.SelectTextAttr( RES_TXTATR_INETFMT ))
                    rWrtSh.SelWrd();
            }
            //now remove the attribute
            std::set<sal_uInt16> aAttribs;
            aAttribs.insert( RES_TXTATR_INETFMT );
            rWrtSh.ResetAttr( aAttribs );
            if(!bSel)
            {
                rWrtSh.Pop(SwCursorShell::PopMode::DeleteCurrent);
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
            const sal_uInt16 nWhich = GetPool().GetWhich( nSlot );
            if ( pArgs && pArgs->GetItemState( nWhich ) == SfxItemState::SET )
                bUseDialog = false;
            SAL_FALLTHROUGH;
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
            rWrtSh.Pop(SwCursorShell::PopMode::DeleteCurrent); // restore old cursor
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
            const sal_uInt16 nWhich = GetPool().GetWhich( nSlot );
            if ( pArgs && pArgs->GetItemState( nWhich ) == SfxItemState::SET )
                bUseDialog = false;
            SAL_FALLTHROUGH;
        }
        case SID_PARA_DLG:
        {
            SwPaM* pPaM = nullptr;

            if ( pArgs )
            {
                const SfxPoolItem* pPaMItem = nullptr;
                pArgs->GetItemState( GetPool().GetWhich( FN_PARAM_PAM ), false, &pPaMItem );
                if ( pPaMItem )
                    pPaM = static_cast< const SwPaMItem* >( pPaMItem )->GetValue( );
            }

            if ( !pPaM )
                pPaM = rWrtSh.GetCursor();

            FieldUnit eMetric = ::GetDfltMetric( dynamic_cast<SwWebView*>( &GetView()) != nullptr );
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));

            bool bApplyCharUnit = ::HasCharUnit( dynamic_cast<SwWebView*>( &GetView()) != nullptr  );
            SW_MOD()->PutItem(SfxBoolItem(SID_ATTR_APPLYCHARUNIT, bApplyCharUnit));

            SfxItemSet aCoreSet(
                GetPool(),
                svl::Items<
                    RES_PARATR_BEGIN, RES_FRMATR_END - 1,
                    // FillAttribute support:
                    XATTR_FILL_FIRST, XATTR_FILL_LAST,
                    // Includes SID_ATTR_TABSTOP_POS:
                    SID_ATTR_TABSTOP_DEFAULTS, SID_ATTR_TABSTOP_OFFSET,
                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                    SID_ATTR_PARA_MODEL, SID_ATTR_PARA_KEEP,
                    // Items to hand over XPropertyList things like XColorList,
                    // XHatchList, XGradientList, and XBitmapList to the Area
                    // TabPage:
                    SID_COLOR_TABLE, SID_PATTERN_LIST,
                    SID_HTML_MODE, SID_HTML_MODE,
                    SID_ATTR_PARA_PAGENUM, SID_ATTR_PARA_PAGENUM,
                    FN_PARAM_1, FN_PARAM_1,
                    FN_NUMBER_NEWSTART, FN_NUMBER_NEWSTART_AT,
                    FN_DROP_TEXT, FN_DROP_CHAR_STYLE_NAME>{});

            // get also the list level indent values merged as LR-SPACE item, if needed.
            rWrtSh.GetPaMAttr( pPaM, aCoreSet, true );

            // create needed items for XPropertyList entries from the DrawModel so that
            // the Area TabPage can access them
            // Do this after GetCurAttr, this resets the ItemSet content again
            const SwDrawModel* pDrawModel = GetView().GetDocShell()->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();

            aCoreSet.Put(SvxColorListItem(pDrawModel->GetColorList(), SID_COLOR_TABLE));
            aCoreSet.Put(SvxGradientListItem(pDrawModel->GetGradientList(), SID_GRADIENT_LIST));
            aCoreSet.Put(SvxHatchListItem(pDrawModel->GetHatchList(), SID_HATCH_LIST));
            aCoreSet.Put(SvxBitmapListItem(pDrawModel->GetBitmapList(), SID_BITMAP_LIST));
            aCoreSet.Put(SvxPatternListItem(pDrawModel->GetPatternList(), SID_PATTERN_LIST));
            aCoreSet.Put(SfxUInt16Item(SID_HTML_MODE,
                            ::GetHtmlMode(GetView().GetDocShell())));

            // Tabulators: Put DefaultTabs into ItemSet
            const SvxTabStopItem& rDefTabs =
                            GetPool().GetDefaultItem(RES_PARATR_TABSTOP);

            const sal_uInt16 nDefDist = static_cast<sal_uInt16>(::GetTabDist( rDefTabs ));
            SfxUInt16Item aDefDistItem( SID_ATTR_TABSTOP_DEFAULTS, nDefDist );
            aCoreSet.Put( aDefDistItem );

            // Current tabulator
            SfxUInt16Item aTabPos( SID_ATTR_TABSTOP_POS, 0 );
            aCoreSet.Put( aTabPos );

            // Left border as offset
            //#i24363# tab stops relative to indent
            const long nOff = rWrtSh.getIDocumentSettingAccess().get(DocumentSettingId::TABS_RELATIVE_TO_INDENT) ?
                aCoreSet.Get( RES_LR_SPACE ).GetTextLeft() : 0;
            SfxInt32Item aOff( SID_ATTR_TABSTOP_OFFSET, nOff );
            aCoreSet.Put( aOff );

            // Setting the BoxInfo
            ::PrepareBoxInfo( aCoreSet, rWrtSh );

            // Current page format
            ::SwToSfxPageDescAttr( aCoreSet );

            // Properties of numbering
            if (rWrtSh.GetNumRuleAtCurrCursorPos())
            {
                SfxBoolItem aStart( FN_NUMBER_NEWSTART, rWrtSh.IsNumRuleStart( pPaM ) );
                aCoreSet.Put(aStart);
                SfxUInt16Item aStartAt( FN_NUMBER_NEWSTART_AT,
                                        rWrtSh.GetNodeNumStart( pPaM ) );
                aCoreSet.Put(aStartAt);
            }
            VclPtr<SfxAbstractTabDialog> pDlg;

            if ( bUseDialog && GetActiveView() )
            {
                OString sDefPage;
                if (pItem)
                    sDefPage = OUStringToOString(static_cast<const SfxStringItem*>(pItem)->GetValue(), RTL_TEXTENCODING_UTF8);

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                pDlg.reset(pFact->CreateSwParaDlg( GetView().GetWindow(),GetView(), aCoreSet, false, sDefPage ));
            }

            if ( !bUseDialog )
            {
                if ( nSlot == SID_ATTR_PARA_LRSPACE)
                {
                    SvxLRSpaceItem aParaMargin(static_cast<const SvxLRSpaceItem&>(pArgs->Get(nSlot)));
                    aParaMargin.SetWhich( RES_LR_SPACE);
                    aCoreSet.Put(aParaMargin);

                    sw_ParagraphDialogResult(&aCoreSet, rWrtSh, rReq, pPaM);
                }
                else
                    sw_ParagraphDialogResult(const_cast<SfxItemSet*>(pArgs), rWrtSh, rReq, pPaM);
            }
            else if (pDlg)
            {
                std::shared_ptr<SfxRequest> pRequest(new SfxRequest(rReq));
                rReq.Ignore(); // the 'old' request is not relevant any more

                pDlg->StartExecuteAsync([pDlg, &rWrtSh, pRequest, nDefDist](sal_Int32 nResult){
                    if (nResult == RET_OK)
                    {
                        // Apply defaults if necessary.
                        SfxItemSet* pSet = const_cast<SfxItemSet*>(pDlg->GetOutputItemSet());
                        sal_uInt16 nNewDist;
                        const SfxPoolItem* pItem2 = nullptr;
                        if (SfxItemState::SET == pSet->GetItemState(SID_ATTR_TABSTOP_DEFAULTS, false, &pItem2) &&
                            nDefDist != (nNewDist = static_cast<const SfxUInt16Item*>(pItem2)->GetValue()) )
                        {
                            SvxTabStopItem aDefTabs( 0, 0, SvxTabAdjust::Default, RES_PARATR_TABSTOP );
                            MakeDefTabs( nNewDist, aDefTabs );
                            rWrtSh.SetDefault( aDefTabs );
                            pSet->ClearItem( SID_ATTR_TABSTOP_DEFAULTS );
                        }

                        if (SfxItemState::SET == pSet->GetItemState(FN_PARAM_1, false, &pItem2))
                        {
                            pSet->Put(SfxStringItem(FN_DROP_TEXT, static_cast<const SfxStringItem*>(pItem2)->GetValue()));
                            pSet->ClearItem(FN_PARAM_1);
                        }

                        if (SfxItemState::SET == pSet->GetItemState(RES_PARATR_DROP, false, &pItem2))
                        {
                            OUString sCharStyleName;
                            if (static_cast<const SwFormatDrop*>(pItem2)->GetCharFormat())
                                sCharStyleName = static_cast<const SwFormatDrop*>(pItem2)->GetCharFormat()->GetName();
                            pSet->Put(SfxStringItem(FN_DROP_CHAR_STYLE_NAME, sCharStyleName));
                        }

                        sw_ParagraphDialogResult(pSet, rWrtSh, *pRequest, rWrtSh.GetCursor());
                    }
                });
            }
        }
        break;
        case FN_NUM_CONTINUE:
        {
            OUString sContinuedListId;
            const SwNumRule* pRule =
                rWrtSh.SearchNumRule( true, sContinuedListId );
            // #i86492#
            // Search also for bullet list
            if ( !pRule )
            {
                pRule = rWrtSh.SearchNumRule( false, sContinuedListId );
            }
            if ( pRule )
            {
                rWrtSh.SetCurNumRule( *pRule, false, sContinuedListId );
            }
        }
        break;

        case FN_SELECT_PARA:
        {
            if ( !rWrtSh.IsSttOfPara() )
                rWrtSh.SttPara();
            else
                rWrtSh.EnterStdMode();
            rWrtSh.EndPara( true );
        }
        break;

        case SID_DEC_INDENT:
        case SID_INC_INDENT:
        //According to the requirement, modified the behavior when user
        //using the indent button on the toolbar. Now if we increase/decrease indent for a
        //paragraph which has bullet style it will increase/decrease the bullet level.
        {
            //If the current paragraph has bullet call the function to
            //increase or decrease the bullet level.
            //Why could I know whether a paragraph has bullet or not by checking the below conditions?
            //Please refer to the "case KEY_TAB:" section in SwEditWin::KeyInput(..) :
            //      if( rSh.GetCurNumRule() && rSh.IsSttOfPara() &&
            //                  !rSh.HasReadonlySel() )
            //              eKeyState = KS_NumDown;
            //Above code demonstrates that when the cursor is at the start of a paragraph which has bullet,
            //press TAB will increase the bullet level.
            //So I copied from that ^^
            if ( rWrtSh.GetNumRuleAtCurrCursorPos() && !rWrtSh.HasReadonlySel() )
            {
                rWrtSh.NumUpDown( SID_INC_INDENT == nSlot );
            }
            else                //execute the original processing functions
            {
                //below is copied of the old codes
                rWrtSh.MoveLeftMargin( SID_INC_INDENT == nSlot, rReq.GetModifier() != KEY_MOD1 );
            }
        }
        rReq.Done();
        break;

        case FN_DEC_INDENT_OFFSET:
        case FN_INC_INDENT_OFFSET:
            rWrtSh.MoveLeftMargin( FN_INC_INDENT_OFFSET == nSlot, rReq.GetModifier() == KEY_MOD1 );
            rReq.Done();
            break;

        case SID_ATTR_CHAR_COLOR2:
        {
            if(pItem)
            {
                Color aSet = static_cast<const SvxColorItem*>(pItem)->GetValue();
                SwEditWin& rEditWin = GetView().GetEditWin();
                rEditWin.SetWaterCanTextColor(aSet);
                SwApplyTemplate* pApply = rEditWin.GetApplyTemplate();

                // If there is a selection, then set the color on it
                // otherwise, it'll be the color for the next text to be typed
                if(!pApply || pApply->nColor != SID_ATTR_CHAR_COLOR_EXT)
                {
                    rWrtSh.SetAttrItem(SvxColorItem (aSet, RES_CHRATR_COLOR));
                }

                rReq.Done();
            }
        }
        break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
        case SID_ATTR_CHAR_COLOR_BACKGROUND_EXT:
        case SID_ATTR_CHAR_COLOR_EXT:
        {
            Color aSet = pItem ? static_cast<const SvxColorItem*>(pItem)->GetValue() : COL_TRANSPARENT;
            SwEditWin& rEdtWin = GetView().GetEditWin();
            if (nSlot != SID_ATTR_CHAR_COLOR_EXT)
                rEdtWin.SetWaterCanTextBackColor(aSet);
            else if (pItem)
                rEdtWin.SetWaterCanTextColor(aSet);

            SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
            SwApplyTemplate aTempl;
            if (!pApply && (rWrtSh.HasSelection() || rReq.IsAPI()))
            {
                if (nSlot != SID_ATTR_CHAR_COLOR_EXT)
                {
                    rWrtSh.StartUndo( SwUndoId::INSATTR );

                    SfxItemSet aCoreSet( rWrtSh.GetView().GetPool(), svl::Items<
                                         RES_CHRATR_BACKGROUND, RES_CHRATR_BACKGROUND,
                                         RES_CHRATR_GRABBAG, RES_CHRATR_GRABBAG>{} );
                    rWrtSh.GetCurAttr( aCoreSet );

                    // Remove highlight if already set of the same color
                    const SvxBrushItem& rBrushItem = aCoreSet.Get(RES_CHRATR_BACKGROUND);
                    if ( aSet == rBrushItem.GetColor() )
                        aSet = COL_TRANSPARENT;

                    rWrtSh.SetAttrItem( SvxBrushItem(aSet, RES_CHRATR_BACKGROUND) );

                    // Remove MS specific highlight when background is set
                    rWrtSh.SetAttrItem( SvxBrushItem(RES_CHRATR_HIGHLIGHT) );

                    // Remove shading marker
                    const SfxPoolItem *pTmpItem;
                    if( SfxItemState::SET == aCoreSet.GetItemState( RES_CHRATR_GRABBAG, false, &pTmpItem ) )
                    {
                        SfxGrabBagItem aGrabBag(*static_cast<const SfxGrabBagItem*>(pTmpItem));
                        std::map<OUString, css::uno::Any>& rMap = aGrabBag.GetGrabBag();
                        auto aIterator = rMap.find("CharShadingMarker");
                        if( aIterator != rMap.end() )
                        {
                            aIterator->second <<= false;
                        }
                        rWrtSh.SetAttrItem( aGrabBag );
                    }
                    rWrtSh.EndUndo( SwUndoId::INSATTR );
                }
                else
                    rWrtSh.SetAttrItem(
                        SvxColorItem(aSet, RES_CHRATR_COLOR) );
            }
            else if (nSlot == SID_ATTR_CHAR_COLOR_BACKGROUND)
            {
                if (!pApply || pApply->nColor != SID_ATTR_CHAR_COLOR_BACKGROUND_EXT)
                {
                    aTempl.nColor = SID_ATTR_CHAR_COLOR_BACKGROUND_EXT;
                    rEdtWin.SetApplyTemplate(aTempl);
                }
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
                rWrtSh.MoveParagraph();
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
            SfxRequest aReq(nSlot, SfxCallMode::SLOT, SfxGetpApp()->GetPool());
            GetView().GetViewFrame()->ExecuteSlot( aReq);
            rReq.Ignore();
        }
        break;
    case FN_INSERT_PAGEHEADER:
    case FN_INSERT_PAGEFOOTER:
    if(pArgs && pArgs->Count())
    {
        OUString sStyleName;
        if(pItem)
            sStyleName = static_cast<const SfxStringItem*>(pItem)->GetValue();
        bool bOn = true;
        if( SfxItemState::SET == pArgs->GetItemState(FN_PARAM_1, false, &pItem))
            bOn = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        rWrtSh.ChangeHeaderOrFooter(sStyleName, FN_INSERT_PAGEHEADER == nSlot, bOn, !rReq.IsAPI());
        rReq.Done();
    }
    break;
    case FN_READONLY_SELECTION_MODE :
        if(GetView().GetDocShell()->IsReadOnly())
        {
            rWrtSh.SetReadonlySelectionOption(
                !rWrtSh.GetViewOptions()->IsSelectionInReadonly());
            rWrtSh.ShowCursor();
        }
    break;
    case FN_SELECTION_MODE_DEFAULT:
    case FN_SELECTION_MODE_BLOCK :
    {
        bool bSetBlockMode = !rWrtSh.IsBlockMode();
        if( pArgs && SfxItemState::SET == pArgs->GetItemState(nSlot, false, &pItem))
            bSetBlockMode = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        if( ( nSlot == FN_SELECTION_MODE_DEFAULT ) != bSetBlockMode )
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
                        svl::Items<RES_TXTATR_INETFMT,
                        RES_TXTATR_INETFMT>{});
        rWrtSh.GetCurAttr(aSet);
        if(SfxItemState::SET <= aSet.GetItemState( RES_TXTATR_INETFMT ))
        {
            const SwFormatINetFormat& rINetFormat = dynamic_cast<const SwFormatINetFormat&>( aSet.Get(RES_TXTATR_INETFMT) );
            if( nSlot == FN_COPY_HYPERLINK_LOCATION )
            {
                ::uno::Reference< datatransfer::clipboard::XClipboard > xClipboard = GetView().GetEditWin().GetClipboard();
                vcl::unohelper::TextDataObject::CopyStringTo(
                        rINetFormat.GetValue(),
                        xClipboard );
            }
            else
                rWrtSh.ClickToINetAttr(rINetFormat);
        }
    }
    break;
    case SID_OPEN_XML_FILTERSETTINGS:
    {
        HandleOpenXmlFilterSettings(rReq);
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
        GetView().UpdateWordCount(this, nSlot);
    }
    break;
    default:
        OSL_ENSURE(false, "wrong dispatcher");
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
        case FN_FORMAT_CURRENT_FOOTNOTE_DLG:
            if( !rSh.IsCursorInFootnote() )
                rSet.DisableItem( nWhich );
        break;

        case SID_LANGUAGE_STATUS:
            {
                // the value of used script types
                OUString aScriptTypesInUse( OUString::number( static_cast<int>(rSh.GetScriptType()) ) );

                // get keyboard language
                OUString aKeyboardLang;
                SwEditWin& rEditWin = GetView().GetEditWin();
                LanguageType nLang = rEditWin.GetInputLanguage();
                if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_SYSTEM)
                    aKeyboardLang = SvtLanguageTable::GetLanguageString( nLang );

                // get the language that is in use
                OUString aCurrentLang = "*";
                nLang = SwLangHelper::GetCurrentLanguage( rSh );
                if (nLang != LANGUAGE_DONTKNOW)
                    aCurrentLang = SvtLanguageTable::GetLanguageString( nLang );

                // build sequence for status value
                uno::Sequence< OUString > aSeq( 4 );
                aSeq[0] = aCurrentLang;
                aSeq[1] = aScriptTypesInUse;
                aSeq[2] = aKeyboardLang;
                aSeq[3] = SwLangHelper::GetTextForLanguageGuessing( rSh );

                // set sequence as status value
                SfxStringListItem aItem( SID_LANGUAGE_STATUS );
                aItem.SetStringList( aSeq );
                rSet.Put( aItem );
            }
        break;

        case SID_THES:
        {
            // is there a valid selection to get text from?
            OUString aText;
            bool bValid = !rSh.HasSelection() ||
                    (rSh.IsSelOnePara() && !rSh.IsMultiSelection());
            // prevent context menu from showing when cursor is not in or at the end of a word
            // (GetCurWord will return the next word if there is none at the current position...)
            const sal_Int16 nWordType = ::i18n::WordType::DICTIONARY_WORD;
            bool bWord = rSh.IsInWord( nWordType ) || rSh.IsStartWord( nWordType ) || rSh.IsEndWord( nWordType );
            if (bValid && bWord)
               aText = rSh.HasSelection()? rSh.GetSelText() : rSh.GetCurWord();

            LanguageType nLang = rSh.GetCurLang();
            LanguageTag aLanguageTag( nLang);
            lang::Locale aLocale( aLanguageTag.getLocale());

            // disable "Thesaurus" context menu entry if there is nothing to look up
            uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
            if (aText.isEmpty() ||
                !xThes.is() || nLang == LANGUAGE_NONE || !xThes->hasLocale( aLocale ))
                rSet.DisableItem( SID_THES );
            else
            {
                // set word and locale to look up as status value
                OUString aStatusVal = aText + "#" + aLanguageTag.getBcp47();
                rSet.Put( SfxStringItem( SID_THES, aStatusVal ) );
            }
        }
        break;

        case FN_NUMBER_NEWSTART :
            if(!rSh.GetNumRuleAtCurrCursorPos())
                    rSet.DisableItem(nWhich);
            else
                rSet.Put(SfxBoolItem(FN_NUMBER_NEWSTART,
                    rSh.IsNumRuleStart()));
        break;

        case FN_EDIT_FORMULA:
        case SID_CHARMAP:
        case SID_EMOJI_CONTROL:
        case SID_CHARMAP_CONTROL:
            {
                const SelectionType nType = rSh.GetSelectionType();
                if (!(nType & SelectionType::Text) &&
                    !(nType & SelectionType::Table) &&
                    !(nType & SelectionType::NumberList))
                {
                    rSet.DisableItem(nWhich);
                }
                else if ( nWhich == FN_EDIT_FORMULA
                          && rSh.CursorInsideInputField() )
                {
                    rSet.DisableItem( nWhich );
                }
            }
            break;

        case FN_INSERT_ENDNOTE:
        case FN_INSERT_FOOTNOTE:
        case FN_INSERT_FOOTNOTE_DLG:
            {
                const FrameTypeFlags nNoType =
                    FrameTypeFlags::FLY_ANY | FrameTypeFlags::HEADER | FrameTypeFlags::FOOTER | FrameTypeFlags::FOOTNOTE;
                if ( rSh.GetFrameType(nullptr,true) & nNoType )
                    rSet.DisableItem(nWhich);

                if ( rSh.CursorInsideInputField() )
                {
                    rSet.DisableItem( nWhich );
                }
            }
            break;

        case FN_INSERT_HYPERLINK:
        case SID_INSERTDOC:
        case FN_INSERT_GLOSSARY:
        case FN_EXPAND_GLOSSARY:
            if ( rSh.CursorInsideInputField() )
            {
                rSet.DisableItem( nWhich );
            }
            break;

        case FN_INSERT_TABLE:
            if ( rSh.CursorInsideInputField()
                 || rSh.GetTableFormat()
                 || (rSh.GetFrameType(nullptr,true) & FrameTypeFlags::FOOTNOTE) )
            {
                rSet.DisableItem( nWhich );
            }
            break;

        case FN_CALCULATE:
            if ( !rSh.IsSelection() )
                rSet.DisableItem(nWhich);
            break;
        case FN_GOTO_REFERENCE:
            {
                SwField *pField = rSh.GetCurField();
                if ( !pField || (pField->GetTypeId() != TYP_GETREFFLD) )
                    rSet.DisableItem(nWhich);
            }
            break;
        case FN_AUTOFORMAT_AUTO:
            {
                rSet.Put( SfxBoolItem( nWhich, SvxAutoCorrCfg::Get().IsAutoFormatByInput() ));
            }
            break;

        case SID_DEC_INDENT:
        case SID_INC_INDENT:
        {
            //if the paragrah has bullet we'll do the following things:
            //1: if the bullet level is the first level, disable the decrease-indent button
            //2: if the bullet level is the last level, disable the increase-indent button
            if ( rSh.GetNumRuleAtCurrCursorPos() && !rSh.HasReadonlySel() )
            {
                const sal_uInt8 nLevel = rSh.GetNumLevel();
                if ( ( nLevel == ( MAXLEVEL - 1 ) && nWhich == SID_INC_INDENT )
                     || ( nLevel == 0 && nWhich == SID_DEC_INDENT ) )
                {
                    rSet.DisableItem( nWhich );
                }
            }
            else
            {
                sal_uInt16 nHtmlMode = ::GetHtmlMode( GetView().GetDocShell() );
                nHtmlMode &= HTMLMODE_ON | HTMLMODE_SOME_STYLES;
                if ( ( nHtmlMode == HTMLMODE_ON )
                     || !rSh.IsMoveLeftMargin( SID_INC_INDENT == nWhich ) )
                {
                    rSet.DisableItem( nWhich );
                }
            }
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
                SfxItemSet aSet( GetPool() );
                rSh.GetCurAttr( aSet );
                const SvxColorItem& aColorItem = aSet.Get(RES_CHRATR_COLOR);
                std::unique_ptr<SfxPoolItem> pNewItem(aColorItem.CloneSetWhich(SID_ATTR_CHAR_COLOR2));
                rSet.Put( *pNewItem );
            }
            break;
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
            {
                // Always use the visible background
                SfxItemSet aSet( GetPool() );
                rSh.GetCurAttr( aSet );
                const SvxBrushItem& aBrushItem = aSet.Get(RES_CHRATR_HIGHLIGHT);
                if( aBrushItem.GetColor() != COL_TRANSPARENT )
                {
                    rSet.Put( SvxColorItem(aBrushItem.GetColor(), nWhich) );
                }
                else
                {
                    const SvxBrushItem& aBrushItem2 = aSet.Get(RES_CHRATR_BACKGROUND);
                    rSet.Put( SvxColorItem(aBrushItem2.GetColor(), nWhich) );
                }
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
            if( rSh.IsTableMode()
                || rSh.CursorInsideInputField() )
            {
                rSet.DisableItem( nWhich );
            }
            break;

        case FN_INSERT_BREAK:
            if ( rSh.HasReadonlySel()
                 && !rSh.CursorInsideInputField() )
            {
                rSet.DisableItem( nWhich );
            }
            break;

        case FN_INSERT_BREAK_DLG:
        case FN_INSERT_COLUMN_BREAK:
        case FN_INSERT_PAGEBREAK:
            if( rSh.CursorInsideInputField() )
            {
                rSet.DisableItem( nWhich );
            }
            break;

        case FN_INSERT_PAGEHEADER:
        case FN_INSERT_PAGEFOOTER:
            if (comphelper::LibreOfficeKit::isActive())
            {
                bool bState = false;
                bool bAllState = true;
                bool bIsPhysical = false;

                OUString aStyleName;
                std::vector<OUString> aList;
                const OUString sPhysical("IsPhysical");
                const OUString sDisplay("DisplayName");
                const OUString sHeaderOn(nWhich == FN_INSERT_PAGEHEADER ? OUString("HeaderIsOn") : OUString("FooterIsOn"));

                uno::Reference< XStyleFamiliesSupplier > xSupplier(GetView().GetDocShell()->GetBaseModel(), uno::UNO_QUERY);
                if (xSupplier.is())
                {
                    uno::Reference< XNameContainer > xContainer;
                    uno::Reference< XNameAccess > xFamilies = xSupplier->getStyleFamilies();
                    if (xFamilies->getByName("PageStyles") >>= xContainer)
                    {
                        uno::Sequence< OUString > aSeqNames = xContainer->getElementNames();
                        for (sal_Int32 itName = 0; itName < aSeqNames.getLength(); itName++)
                        {
                            aStyleName = aSeqNames[itName];
                            uno::Reference<XPropertySet> xPropSet(xContainer->getByName(aStyleName), uno::UNO_QUERY);
                            if (xPropSet.is() && (xPropSet->getPropertyValue(sPhysical) >>= bIsPhysical) && bIsPhysical)
                            {
                                xPropSet->getPropertyValue(sDisplay) >>= aStyleName;
                                if ((xPropSet->getPropertyValue(sHeaderOn)>>= bState) && bState)
                                    aList.push_back(aStyleName);
                                else
                                    bState = false;

                                // Check if all entries have the same state
                                bAllState &= bState;
                            }
                            else
                                bIsPhysical = false;
                       }
                    }
                }

                if (bAllState && aList.size() > 1)
                    aList.push_back("_ALL_");

                rSet.Put(SfxStringListItem(nWhich, &aList));
            }
            else
            {
                rSet.Put( SfxObjectShellItem( nWhich, GetView().GetDocShell() ));
            }
            break;
            case FN_TABLE_SORT_DIALOG:
            case FN_SORTING_DLG:
                if(!rSh.HasSelection() ||
                        (FN_TABLE_SORT_DIALOG == nWhich && !rSh.GetTableFormat()))
                    rSet.DisableItem( nWhich );
            break;

            case SID_RUBY_DIALOG:
                {
                    SvtCJKOptions aCJKOptions;
                    if( !aCJKOptions.IsRubyEnabled()
                        || rSh.CursorInsideInputField() )
                    {
                        GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, false );
                        rSet.DisableItem(nWhich);
                    }
                    else
                        GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, true );
                }
                break;

            case SID_HYPERLINK_DIALOG:
                if( GetView().GetDocShell()->IsReadOnly()
                    || ( !GetView().GetViewFrame()->HasChildWindow(nWhich)
                         && rSh.HasReadonlySel() )
                    || rSh.CursorInsideInputField() )
                {
                    rSet.DisableItem(nWhich);
                }
                else
                {
                    rSet.Put(SfxBoolItem( nWhich, nullptr != GetView().GetViewFrame()->GetChildWindow( nWhich ) ));
                }
                break;

            case FN_EDIT_HYPERLINK:
            case FN_COPY_HYPERLINK_LOCATION:
                {
                    SfxItemSet aSet(GetPool(),
                        svl::Items<RES_TXTATR_INETFMT,
                        RES_TXTATR_INETFMT>{});
                    rSh.GetCurAttr(aSet);
                    if(SfxItemState::SET > aSet.GetItemState( RES_TXTATR_INETFMT ) || rSh.HasReadonlySel())
                    {
                        rSet.DisableItem(nWhich);
                    }
                }
            break;
            case FN_REMOVE_HYPERLINK:
            {
                SfxItemSet aSet(GetPool(),
                                svl::Items<RES_TXTATR_INETFMT,
                                RES_TXTATR_INETFMT>{});
                rSh.GetCurAttr(aSet);

                // If a hyperlink is selected, either alone or along with other text...
                if ((aSet.GetItemState(RES_TXTATR_INETFMT) < SfxItemState::SET &&
                    aSet.GetItemState(RES_TXTATR_INETFMT) != SfxItemState::DONTCARE) ||
                    rSh.HasReadonlySel())
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
                    GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, false );
                    rSet.DisableItem(nWhich);
                }
                else
                    GetView().GetViewFrame()->GetBindings().SetVisibleState( nWhich, true );
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
                                svl::Items<RES_TXTATR_INETFMT,
                                RES_TXTATR_INETFMT>{});
                rSh.GetCurAttr(aSet);
                if(SfxItemState::SET > aSet.GetItemState( RES_TXTATR_INETFMT, false ))
                    rSet.DisableItem(nWhich);
            }
            break;
            case  SID_OPEN_SMARTTAGMENU:
            {
                 std::vector< OUString > aSmartTagTypes;
                 uno::Sequence< uno::Reference< container::XStringKeyMap > > aStringKeyMaps;
                 uno::Reference<text::XTextRange> xRange;

                 rSh.GetSmartTagTerm( aSmartTagTypes, aStringKeyMaps, xRange );

                 if ( xRange.is() && !aSmartTagTypes.empty() )
                 {
                     uno::Sequence < uno::Sequence< uno::Reference< smarttags::XSmartTagAction > > > aActionComponentsSequence;
                     uno::Sequence < uno::Sequence< sal_Int32 > > aActionIndicesSequence;

                     const SmartTagMgr& rSmartTagMgr = SwSmartTagMgr::Get();
                     rSmartTagMgr.GetActionSequences( aSmartTagTypes,
                                                      aActionComponentsSequence,
                                                      aActionIndicesSequence );

                     uno::Reference <frame::XController> xController = GetView().GetController();
                     const lang::Locale aLocale( SW_BREAKITER()->GetLocale( GetAppLanguageTag() ) );
                     const OUString& aApplicationName( rSmartTagMgr.GetApplicationName() );
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
            case FN_OUTLINE_RULE_INDEX:
        {
            SwNumRule* pCurRule = const_cast<SwNumRule*>(GetShell().GetNumRuleAtCurrCursorPos());
            sal_uInt16  nActNumLvl = USHRT_MAX;
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
                    rSet.Put(SfxUInt16Item(FN_BUL_NUM_RULE_INDEX, USHRT_MAX));
                    rSet.Put(SfxUInt16Item(FN_NUM_NUM_RULE_INDEX, USHRT_MAX));
                    NBOTypeMgrBase* pBullets = NBOutlineTypeMgrFact::CreateInstance(NBOType::Bullets);
                    if ( pBullets )
                    {
                        const sal_uInt16 nBulIndex = pBullets->GetNBOIndexForNumRule(aSvxRule,nActNumLvl);
                        rSet.Put(SfxUInt16Item(FN_BUL_NUM_RULE_INDEX,nBulIndex));
                    }
                }else if ( GetShell().HasNumber() )
                {
                    rSet.Put(SfxUInt16Item(FN_BUL_NUM_RULE_INDEX, USHRT_MAX));
                    rSet.Put(SfxUInt16Item(FN_NUM_NUM_RULE_INDEX, USHRT_MAX));
                    NBOTypeMgrBase* pNumbering = NBOutlineTypeMgrFact::CreateInstance(NBOType::Numbering);
                    if ( pNumbering )
                    {
                        const sal_uInt16 nBulIndex = pNumbering->GetNBOIndexForNumRule(aSvxRule,nActNumLvl);
                        rSet.Put(SfxUInt16Item(FN_NUM_NUM_RULE_INDEX,nBulIndex));
                    }
                }

                if ( nWhich == FN_OUTLINE_RULE_INDEX )
                {
                    rSet.Put(SfxUInt16Item(FN_OUTLINE_RULE_INDEX, USHRT_MAX));
                    NBOTypeMgrBase* pOutline = NBOutlineTypeMgrFact::CreateInstance(NBOType::Outline);
                    if ( pOutline )
                    {
                        const sal_uInt16 nIndex = pOutline->GetNBOIndexForNumRule(aSvxRule,nActNumLvl);
                        rSet.Put(SfxUInt16Item(FN_OUTLINE_RULE_INDEX,nIndex));
                    }
                }
            }
        }
            break;
            case FN_NUM_CONTINUE:
            {
                // #i86492#
                // Search also for bullet list
                OUString aDummy;
                const SwNumRule* pRule =
                        rSh.SearchNumRule( true, aDummy );
                if ( !pRule )
                {
                    pRule = rSh.SearchNumRule( false, aDummy );
                }
                if ( !pRule )
                    rSet.DisableItem(nWhich);
            }
            break;
            case SID_INSERT_RLM :
            case SID_INSERT_LRM :
            {
                SvtCTLOptions aCTLOptions;
                bool bEnabled = aCTLOptions.IsCTLFontEnabled();
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
