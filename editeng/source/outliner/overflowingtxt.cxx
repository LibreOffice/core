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

#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <editeng/overflowingtxt.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editdata.hxx>

#include <editdoc.hxx>
#include <utility>


std::optional<OutlinerParaObject> TextChainingUtils::JuxtaposeParaObject(
        css::uno::Reference< css::datatransfer::XTransferable > const & xOverflowingContent,
        Outliner *pOutl,
        OutlinerParaObject const *pNextPObj)
{
    if (!pNextPObj) {
        pOutl->SetToEmptyText();
    } else {
        pOutl->SetText(*pNextPObj);
    }

    // Special case: if only empty text remove it at the end
    bool bOnlyOneEmptyPara = !pNextPObj ||
                             (pOutl->GetParagraphCount() == 1 &&
                              !pNextPObj->GetTextObject().HasText(0));

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

std::optional<OutlinerParaObject> TextChainingUtils::DeeplyMergeParaObject(
        css::uno::Reference< css::datatransfer::XTransferable > const & xOverflowingContent,
        Outliner *pOutl,
        OutlinerParaObject const *pNextPObj)
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

css::uno::Reference< css::datatransfer::XTransferable > TextChainingUtils::CreateTransferableFromText(Outliner const *pOutl)
{
    EditEngine& rEditEngine = const_cast<EditEngine &>(pOutl->GetEditEngine());
    sal_Int32 nLastPara = pOutl->GetParagraphCount()-1;
    ESelection aWholeTextSel(0, 0, nLastPara, rEditEngine.GetTextLen(nLastPara));

    return rEditEngine.CreateTransferable(aWholeTextSel);
}



OverflowingText::OverflowingText(css::uno::Reference< css::datatransfer::XTransferable > xOverflowingContent) :
        mxOverflowingContent(std::move(xOverflowingContent))
{

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


std::optional<OutlinerParaObject> NonOverflowingText::RemoveOverflowingText(Outliner *pOutliner) const
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
std::optional<OutlinerParaObject> OverflowingText::JuxtaposeParaObject(Outliner *pOutl, OutlinerParaObject const *pNextPObj)
{
    return TextChainingUtils::JuxtaposeParaObject(mxOverflowingContent, pOutl, pNextPObj);
}

std::optional<OutlinerParaObject> OverflowingText::DeeplyMergeParaObject(Outliner *pOutl, OutlinerParaObject const *pNextPObj)
{
    return TextChainingUtils::DeeplyMergeParaObject(mxOverflowingContent, pOutl, pNextPObj);
}


OFlowChainedText::OFlowChainedText(Outliner const *pOutl, bool bIsDeepMerge)
{
    mpOverflowingTxt = pOutl->GetOverflowingText();
    mpNonOverflowingTxt = pOutl->GetNonOverflowingText();

    mbIsDeepMerge = bIsDeepMerge;
}

OFlowChainedText::~OFlowChainedText()
{
}


ESelection OFlowChainedText::GetOverflowPointSel() const
{
    return mpNonOverflowingTxt->GetOverflowPointSel();
}

std::optional<OutlinerParaObject> OFlowChainedText::InsertOverflowingText(Outliner *pOutliner, OutlinerParaObject const *pTextToBeMerged)
{
    // Just return the roughly merged paras for now
    if (!mpOverflowingTxt)
        return std::nullopt;

    if (mbIsDeepMerge) {
        SAL_INFO("editeng.chaining", "[TEXTCHAINFLOW - OF] Deep merging paras" );
        return mpOverflowingTxt->DeeplyMergeParaObject(pOutliner, pTextToBeMerged );
    } else {
        SAL_INFO("editeng.chaining", "[TEXTCHAINFLOW - OF] Juxtaposing paras" );
        return mpOverflowingTxt->JuxtaposeParaObject(pOutliner, pTextToBeMerged );
    }
}


std::optional<OutlinerParaObject> OFlowChainedText::RemoveOverflowingText(Outliner *pOutliner)
{
    if (!mpNonOverflowingTxt)
        return std::nullopt;

    return mpNonOverflowingTxt->RemoveOverflowingText(pOutliner);
}

bool OFlowChainedText::IsLastParaInterrupted() const
{
    return mpNonOverflowingTxt->IsLastParaInterrupted();
}



UFlowChainedText::UFlowChainedText(Outliner const *pOutl, bool bIsDeepMerge)
{
    mxUnderflowingTxt = TextChainingUtils::CreateTransferableFromText(pOutl);
    mbIsDeepMerge = bIsDeepMerge;
}

std::optional<OutlinerParaObject> UFlowChainedText::CreateMergedUnderflowParaObject(Outliner *pOutl, OutlinerParaObject const *pNextLinkWholeText)
{
    std::optional<OutlinerParaObject> pNewText;

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
