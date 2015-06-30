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

OutlinerParaObject *NonOverflowingText::ToParaObject(Outliner *pOutliner) const
{

    // XXX: Possibility: let the NonUnderflowingParaObject just be a TextEditObject created by the Outliner (by means of a selection).

    /* The overflow in SdrTextObj can occur:
     * (a) exactly at the end of a paragraph, or
     * (b) in the middle of a paragraph.
     *
     * In case (a), a NonUnderflowingText object contains only the
     * paragraphs occurred before the overflow.
     * In case (b), a NonUnderflowingText contains also the text of the
     * paragraph that was cut by overflow.
    */

    bool bOverflowOccurredAtEndOfPara =
        (mPreOverflowingTxt == "") &&
        (mpHeadParas != NULL);

    if (bOverflowOccurredAtEndOfPara) {
        // Case (a) above:
        // Only (possibly empty) paragraphs before overflowing one.
        pOutliner->SetText(*mpHeadParas);
    } else {
        // Case (b): some text is non included in any OutlinerParaObject.
        // We have to include the non-overflowing lines from the overfl. para

        // first make a ParaObject for the strings
        pOutliner->SetToEmptyText();
        Paragraph *pTmpPara0 = pOutliner->GetParagraph(0);
        pOutliner->SetText(mPreOverflowingTxt, pTmpPara0);
        OutlinerParaObject *pPObj = pOutliner->CreateParaObject();

        if (mpHeadParas != NULL) {
            pOutliner->SetText(*mpHeadParas);
            pOutliner->AddText(*pPObj);
         } else  if (mPreOverflowingTxt != "") { // only preoverflowing txt
            pOutliner->SetText(*pPObj);
        } else { // no text // This case is redundant but it doesn't hurt for now
            pOutliner->SetToEmptyText();
        }
    }

     return pOutliner->CreateParaObject();
}

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


OFlowChainedText::OFlowChainedText(Outliner *pOutl)
{
    mpOverflowingTxt = pOutl->GetOverflowingText();
    mpNonOverflowingTxt = pOutl->GetNonOverflowingText();
}

OutlinerParaObject *OFlowChainedText::CreateOverflowingParaObject(Outliner *pOutliner, OutlinerParaObject *pTextToBeMerged)
{
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
