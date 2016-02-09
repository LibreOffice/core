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

#include "rtl/ustring.hxx"
#include "tools/debug.hxx"

#include "editeng/overflowingtxt.hxx"
#include "editeng/outliner.hxx"
#include "editeng/outlobj.hxx"
#include "editeng/editobj.hxx"
#include "editeng/editdata.hxx"

#include "outleeng.hxx"
#include "editdoc.hxx"

#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>


OutlinerParaObject *TextChainingUtils::JuxtaposeParaObject(
        css::uno::Reference< css::datatransfer::XTransferable > xOverflowingContent,
        Outliner *pOutl,
        OutlinerParaObject *pNextPObj)
{
    if (!pNextPObj) {
        pOutl->SetToEmptyText();
    } else {
        pOutl->SetText(*pNextPObj);
    }

    // Special case: if only empty text remove it at the end
    bool bOnlyOneEmptyPara = !pNextPObj ||
                             (pOutl->GetParagraphCount() == 1 &&
                              pNextPObj->GetTextObject().GetText(0).isEmpty());

    EditEngine &rEditEngine = const_cast<EditEngine &>(pOutl->GetEditEngine());

    // XXX: this code should be moved in Outliner directly
    //          creating Outliner::InsertText(...transferable...)
    EditSelection aStartSel(rEditEngine.CreateSelection(ESelection(0,0)));
    EditSelection aNewSel = rEditEngine.InsertText(xOverflowingContent,
                                                    OUString(),
                                                    aStartSel.Min(),
                                                    true);

    if (!bOnlyOneEmptyPara) {
        // Separate Paragraphs
        rEditEngine.InsertParaBreak(aNewSel);
    }


    return pOutl->CreateParaObject();
}

OutlinerParaObject *TextChainingUtils::DeeplyMergeParaObject(
        css::uno::Reference< css::datatransfer::XTransferable > xOverflowingContent,
        Outliner *pOutl,
        OutlinerParaObject *pNextPObj)
{
    if (!pNextPObj) {
        pOutl->SetToEmptyText();
    } else {
        pOutl->SetText(*pNextPObj);
    }

    EditEngine &rEditEngine = const_cast<EditEngine &>(pOutl->GetEditEngine());

    // XXX: this code should be moved in Outliner directly
    //          creating Outliner::InsertText(...transferable...)
    EditSelection aStartSel(rEditEngine.CreateSelection(ESelection(0,0)));
    // We don't need to mark the selection
    // EditSelection aNewSel =
    rEditEngine.InsertText(xOverflowingContent,
                            OUString(),
                            aStartSel.Min(),
                            true);

    return pOutl->CreateParaObject();
}

css::uno::Reference< css::datatransfer::XTransferable > TextChainingUtils::CreateTransferableFromText(Outliner *pOutl)
{
    const EditEngine &rEditEngine = pOutl->GetEditEngine();
    sal_Int32 nLastPara = pOutl->GetParagraphCount()-1;
    ESelection aWholeTextSel(0, 0, nLastPara, rEditEngine.GetTextLen(nLastPara));

    return rEditEngine.CreateTransferable(aWholeTextSel);
}


// class OverflowingText

OverflowingText::OverflowingText(css::uno::Reference< css::datatransfer::XTransferable > xOverflowingContent) :
        mxOverflowingContent(xOverflowingContent)
{

}


// class NonOverflowingText

NonOverflowingText::NonOverflowingText(const EditTextObject * /*pTObj*/,  bool bLastParaInterrupted)
    : mbLastParaInterrupted(bLastParaInterrupted)
{
     // XXX: may have to delete pTObj
}

NonOverflowingText::NonOverflowingText(const ESelection &aSel, bool bLastParaInterrupted)
    : maContentSel(aSel)
    , mbLastParaInterrupted(bLastParaInterrupted)
{
}

bool NonOverflowingText::IsLastParaInterrupted() const
{
    return mbLastParaInterrupted;
}


OutlinerParaObject *NonOverflowingText::RemoveOverflowingText(Outliner *pOutliner) const
{
    pOutliner->QuickDelete(maContentSel);
    SAL_INFO("editeng.chaining", "Deleting selection from (Para: " << maContentSel.nStartPara
             << ", Pos: " << maContentSel.nStartPos << ") to (Para: " << maContentSel.nEndPara
             << ", Pos: " << maContentSel.nEndPos << ")");
    return pOutliner->CreateParaObject();
}

ESelection NonOverflowingText::GetOverflowPointSel() const
{
    //return getLastPositionSel(mpContentTextObj);

    // return the starting point of the selection we are removing
    return ESelection(maContentSel.nStartPara, maContentSel.nStartPos); //XXX
}

// The equivalent of ToParaObject for OverflowingText. Here we are prepending the overflowing text to the old dest box's text
// XXX: In a sense a better name for OverflowingText and NonOverflowingText are respectively DestLinkText and SourceLinkText
OutlinerParaObject *OverflowingText::JuxtaposeParaObject(Outliner *pOutl, OutlinerParaObject *pNextPObj)
{
    return TextChainingUtils::JuxtaposeParaObject(mxOverflowingContent, pOutl, pNextPObj);
}

OutlinerParaObject *OverflowingText::DeeplyMergeParaObject(Outliner *pOutl, OutlinerParaObject *pNextPObj)
{
    return TextChainingUtils::DeeplyMergeParaObject(mxOverflowingContent, pOutl, pNextPObj);
}

// class OFlowChainedText

OFlowChainedText::OFlowChainedText(Outliner *pOutl, bool bIsDeepMerge)
{
    mpOverflowingTxt = pOutl->GetOverflowingText();
    mpNonOverflowingTxt = pOutl->GetNonOverflowingText();

    mbIsDeepMerge = bIsDeepMerge;
}

OFlowChainedText::~OFlowChainedText()
{
    delete mpNonOverflowingTxt;
    delete mpOverflowingTxt;
}


ESelection OFlowChainedText::GetOverflowPointSel() const
{
    return mpNonOverflowingTxt->GetOverflowPointSel();
}

OutlinerParaObject *OFlowChainedText::InsertOverflowingText(Outliner *pOutliner, OutlinerParaObject *pTextToBeMerged)
{
    // Just return the roughly merged paras for now
    if (mpOverflowingTxt == nullptr)
        return nullptr;

    if (mbIsDeepMerge) {
        SAL_INFO("editeng.chaining", "[TEXTCHAINFLOW - OF] Deep merging paras" );
        return mpOverflowingTxt->DeeplyMergeParaObject(pOutliner, pTextToBeMerged );
    } else {
        SAL_INFO("editeng.chaining", "[TEXTCHAINFLOW - OF] Juxtaposing paras" );
        return mpOverflowingTxt->JuxtaposeParaObject(pOutliner, pTextToBeMerged );
    }
}


OutlinerParaObject *OFlowChainedText::RemoveOverflowingText(Outliner *pOutliner)
{
    if (mpNonOverflowingTxt == nullptr)
        return nullptr;

    return mpNonOverflowingTxt->RemoveOverflowingText(pOutliner);
}

bool OFlowChainedText::IsLastParaInterrupted() const
{
    return mpNonOverflowingTxt->IsLastParaInterrupted();
}


// classes UFlowChainedText

UFlowChainedText::UFlowChainedText(Outliner *pOutl, bool bIsDeepMerge)
{
    mxUnderflowingTxt = TextChainingUtils::CreateTransferableFromText(pOutl);
    mbIsDeepMerge = bIsDeepMerge;
}

OutlinerParaObject *UFlowChainedText::CreateMergedUnderflowParaObject(Outliner *pOutl, OutlinerParaObject *pNextLinkWholeText)
{
    OutlinerParaObject *pNewText = nullptr;

    if (mbIsDeepMerge) {
        SAL_INFO("editeng.chaining", "[TEXTCHAINFLOW - UF] Deep merging paras" );
        pNewText = TextChainingUtils::DeeplyMergeParaObject(mxUnderflowingTxt, pOutl, pNextLinkWholeText);
    } else {
        // NewTextForCurBox = Txt(CurBox) ++ Txt(NextBox)
        SAL_INFO("editeng.chaining", "[TEXTCHAINFLOW - UF] Juxtaposing paras" );
        pNewText = TextChainingUtils::JuxtaposeParaObject(mxUnderflowingTxt, pOutl, pNextLinkWholeText);
    }

    return pNewText;

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
