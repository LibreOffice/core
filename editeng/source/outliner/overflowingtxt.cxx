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
#include <tools/debug.hxx>

#include <editeng/overflowingtxt.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/editobj.hxx>

ESelection getLastPositionSel(const EditTextObject *pTObj)
{
    sal_Int32 nLastPara = pTObj->GetParagraphCount()-1;
    // If text is empty
    if (nLastPara < 0 )
        nLastPara = 0;
    sal_Int32 nLen = pTObj->GetText(nLastPara).getLength();
    ESelection aEndPos(nLastPara, nLen, nLastPara, nLen);

    return aEndPos;
}

OverflowingText::OverflowingText(EditTextObject *pTObj) : mpContentTextObj(pTObj)
{
}

ESelection OverflowingText::GetInsertionPointSel() const
{
    return getLastPositionSel(mpContentTextObj);
}

OutlinerParaObject *NonOverflowingText::ToParaObject(Outliner *pOutliner) const
{
    OutlinerParaObject *pPObj = new OutlinerParaObject(*mpContentTextObj);
    pPObj->SetOutlinerMode(pOutliner->GetOutlinerMode());
    return pPObj;
}

ESelection NonOverflowingText::GetOverflowPointSel() const
{
    return getLastPositionSel(mpContentTextObj);
}

// The equivalent of ToParaObject for OverflowingText. Here we are prepending the overflowing text to the old dest box's text
// XXX: In a sense a better name for OverflowingText and NonOverflowingText are respectively DestLinkText and SourceLinkText
OutlinerParaObject *OverflowingText::GetJuxtaposedParaObject(Outliner *pOutl, OutlinerParaObject *pNextPObj)
{
    if (mpContentTextObj == NULL) {
        fprintf(stderr, "[Chaining] OverflowingText's mpContentTextObj is NULL!\n");
        return NULL;
    }

    // Simply Juxtaposing; no within-para merging
    OutlinerParaObject *pOverflowingPObj = new OutlinerParaObject(*mpContentTextObj);
    // the OutlinerParaObject constr. at the prev line gives no valid outliner mode, so we set it
    pOverflowingPObj->SetOutlinerMode(pOutl->GetOutlinerMode());

    /* Actual Text Setting */
    pOutl->SetText(*pOverflowingPObj);

    // Set selection position between new and old text
    //maInsertionPointSel = impGetEndSelection(pOutl);  // XXX: Maybe setting in the constructor is just right

    pOutl->AddText(*pNextPObj);

    // End Text Setting

    OutlinerParaObject *pPObj = pOutl->CreateParaObject();
    //pPObj->SetOutlinerMode(pOutl->GetOutlinerMode());
    return pPObj;
}

/*
ESelection OverflowingText::impGetEndSelection(Outliner *pOutl) const
{
    const sal_Int32 nParaCount = pOutl->GetParagraphCount();
    const sal_Int32 nLastParaIndex = nParaCount > 1 ? nParaCount - 1 : 0;
    Paragraph* pLastPara = pOutl->GetParagraph( nLastParaIndex);
    const sal_Int32 nLenLastPara = pOutl->GetText(pLastPara).getLength();
    // Selection at end of editing area
    ESelection aEndSel(nLastParaIndex,nLenLastPara,nLastParaIndex,nLenLastPara);
    return aEndSel;
}
* */

/*
OUString OverflowingText::GetEndingLines() const
{
    // If the only overflowing part is some lines in a paragraph,
    // the end of the overflowing text is its head.
    if (!HasOtherParas())
        return mHeadTxt;

    return mTailTxt;
}

OUString OverflowingText::GetHeadingLines() const
{
    return mHeadTxt;
}
* */




OFlowChainedText::OFlowChainedText(Outliner *pOutl)
{
    mpOverflowingTxt = pOutl->GetOverflowingText();
    mpNonOverflowingTxt = pOutl->GetNonOverflowingText();
}

ESelection OFlowChainedText::GetInsertionPointSel() const
{
    return mpOverflowingTxt->GetInsertionPointSel();
}

ESelection OFlowChainedText::GetOverflowPointSel() const
{
    return mpNonOverflowingTxt->GetOverflowPointSel();
}

OutlinerParaObject *OFlowChainedText::CreateOverflowingParaObject(Outliner *pOutliner, OutlinerParaObject *pTextToBeMerged)
{
    // Just return the roughly merged paras fpr now
    if (mpOverflowingTxt == NULL || pTextToBeMerged == NULL)
        return NULL;

    return mpOverflowingTxt->GetJuxtaposedParaObject(pOutliner, pTextToBeMerged );

    /*
    if (mpOverflowingTxt == NULL || pTextToBeMerged == NULL)
        return NULL;

    pOutliner->SetText(*pTextToBeMerged);

    // Get text of first paragraph of destination box
    Paragraph *pOldPara0 = pOutliner->GetParagraph(0);
    OUString aOldPara0Txt;
    if (pOldPara0)
        aOldPara0Txt = pOutliner->GetText(pOldPara0);

    // Get other paras of destination box (from second on)
    OutlinerParaObject *pOldParasTail = NULL;
    if (pOutliner->GetParagraphCount() > 1)
        pOldParasTail = pOutliner->CreateParaObject(1);

    // Create ParaObject appending old first para in the dest. box
    //   to last part of overflowing text
    Paragraph *pTmpPara0 = NULL;
    OutlinerParaObject *pJoiningPara = NULL;

    if (pOldPara0) {
        //pOutliner->Clear(); // you need a clear outliner here
        pOutliner->SetToEmptyText();

        pTmpPara0 = pOutliner->GetParagraph(0);
        pOutliner->SetText(mpOverflowingTxt->GetEndingLines() + aOldPara0Txt, pTmpPara0);
        pJoiningPara = pOutliner->CreateParaObject();
    }

    // Create a Para Object out of mpMidParas
    // (in order to use the SfxItemPool of the current outliner
    //  instead of the ones currently in mpMidParas)

    // start actual composition
    //pOutliner->Clear();
    pOutliner->SetToEmptyText();

    // Set headText at the beginning of box
    OUString aHeadTxt = mpOverflowingTxt->GetHeadingLines();
    // If we haven't used heading text yet
    if (mpOverflowingTxt->HasOtherParas()) {
        Paragraph *pNewPara0 = pOutliner->GetParagraph(0);
        pOutliner->SetText(aHeadTxt, pNewPara0);
    }

    // Set all the intermediate Paras
    if (mpOverflowingTxt->mpMidParas)
        pOutliner->AddText(*mpOverflowingTxt->mpMidParas);

    // Append old first para in the destination box to
    //   last part of overflowing text
    if (pJoiningPara && mpOverflowingTxt->HasOtherParas())
        pOutliner->AddText(*pJoiningPara);
    // this second case is if there is to avoid getting an empty line before pJoiningPara
    else if (pJoiningPara && !mpOverflowingTxt->HasOtherParas())
        pOutliner->SetText(*pJoiningPara);

    // Append all other old paras
    if (pOldParasTail)
        pOutliner->AddText(*pOldParasTail);

    // Draw everything
    OutlinerParaObject *pNewText = pOutliner->CreateParaObject();
    return pNewText;
    * */
}

OutlinerParaObject *OFlowChainedText::CreateNonOverflowingParaObject(Outliner *pOutliner)
{
    if (mpNonOverflowingTxt == NULL)
        return NULL;

    return mpNonOverflowingTxt->ToParaObject(pOutliner);
}


UFlowChainedText::UFlowChainedText(Outliner *pOutl)
{
    mpUnderflowPObj = pOutl->CreateParaObject();
}

OutlinerParaObject *UFlowChainedText::CreateMergedUnderflowParaObject(Outliner *pOutl, OutlinerParaObject *pNextLinkWholeText)
{
    OutlinerParaObject *pCurText = mpUnderflowPObj;

    // NewTextForCurBox = Txt(CurBox) ++ Txt(NextBox)
    pOutl->SetText(*pCurText);
    pOutl->AddText(*pNextLinkWholeText);
    OutlinerParaObject *pNewText = pOutl->CreateParaObject();

    return pNewText;

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
