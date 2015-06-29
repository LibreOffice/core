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


#include <svx/textchain.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/overflowingtxt.hxx>
#include <svx/textchainflow.hxx>

TextChainFlow::TextChainFlow(SdrTextObj *pChainTarget)
    : mpTargetLink(pChainTarget)
{
    mpTextChain = mpTargetLink->GetTextChain();
    mpNextLink = mpTargetLink->GetNextLinkInChain();
    bCheckedFlowEvents = false;

    bUnderflow = bOverflow = false;

    mpOverflowingTxt = NULL;
    mpNonOverflowingTxt = NULL;

    mpUnderflowingPObj = NULL;

    // XXX: Set the next link here?


}


TextChainFlow::~TextChainFlow()
{

}

/*
 * Check for overflow in the state of pFlowOutl.
 * If pParamOutl is not NULL sets some parameters from there.
 * This is useful in case the outliner is not set for overflow
 * (e.g. in editing mode we check for overflow in drawing outl but
 *  parameters come from editing outliner)
*/
void TextChainFlow::impCheckForFlowEvents(SdrOutliner *pFlowOutl, SdrOutliner *pParamOutl)
{
    // NOTE: Nah you probably don't need this
    if (pParamOutl != NULL)
    {
        // XXX: Set parameters
        // XXX: does this work if you do it before setting the text? Seems so.
        pFlowOutl->SetUpdateMode(true);
        pFlowOutl->SetMaxAutoPaperSize(pParamOutl->GetMaxAutoPaperSize());
        pFlowOutl->SetMinAutoPaperSize(pParamOutl->GetMinAutoPaperSize());
        pFlowOutl->SetPaperSize(pParamOutl->GetPaperSize());
    }

    bool bIsPageOverflow = pFlowOutl->IsPageOverflow();

    if (pParamOutl != NULL)
    {
        pFlowOutl->SetUpdateMode(false); // XXX: Plausibly should be the prev. state
    }

    // NOTE: overflow and underflow cannot be both true
    bOverflow = bIsPageOverflow && mpNextLink;
    bUnderflow = !bIsPageOverflow &&  mpNextLink && mpNextLink->HasText();

    // Set (Non)OverflowingTxt here

    mpOverflowingTxt = bOverflow ? pFlowOutl->GetOverflowingText() : NULL;
    mpNonOverflowingTxt = bOverflow ? pFlowOutl->GetNonOverflowingText() : NULL;

    // Set current underflowing text (if any)
    mpUnderflowingPObj = bUnderflow ? pFlowOutl->CreateParaObject() : NULL;

}

void TextChainFlow::CheckForFlowEvents(SdrOutliner *pFlowOutl)
{
    impCheckForFlowEvents(pFlowOutl, NULL);
}


bool TextChainFlow::IsOverflow()
{
    return bOverflow;
}

bool TextChainFlow::IsUnderflow()
{
    return bUnderflow;
}


// XXX: In editing mode you need to get "underflowing" text from editing outliner, so it's kinda separate from the drawing one!

// XXX:Would it be possible to unify undeflow and its possibly following overrflow?
void TextChainFlow::ExecuteUnderflow(SdrOutliner *pOutl)
{
    OutlinerParaObject *pNextLinkWholeText = mpNextLink->GetOutlinerParaObject();

    // making whole text
    OutlinerParaObject *pCurText;   // XXX: at next line we have editing outliner in editing version
    //pCurText = pOutl->CreateParaObject();

    // We saved this text already
    pCurText = mpUnderflowingPObj;

    // NewTextForCurBox = Txt(CurBox) ++ Txt(NextBox)
    pOutl->SetText(*pCurText);
    pOutl->AddText(*pNextLinkWholeText);
    OutlinerParaObject *pNewText = pOutl->CreateParaObject();

    // Set the other box empty so if overflow does not occur we are fine
    if (!mpTargetLink->GetPreventChainable())
        mpNextLink->NbcSetOutlinerParaObject(pOutl->GetEmptyParaObject());

    mpTargetLink->NbcSetOutlinerParaObject(pNewText);

    // Check for new overflow
    CheckForFlowEvents(pOutl); // XXX: How do you know you don't need to set parameters here?
}

void TextChainFlow::ExecuteOverflow(SdrOutliner *pNonOverflOutl, SdrOutliner *pOverflOutl)
{
    // Leave only non overflowing text
    impLeaveOnlyNonOverflowingText(pNonOverflOutl);

    // Transfer of text to next link
    if (!mpTargetLink->GetPreventChainable() ) // we don't transfer text while dragging because of resizing
    {
        impMoveChainedTextToNextLink(pOverflOutl);
    }
}

void TextChainFlow::impLeaveOnlyNonOverflowingText(SdrOutliner *pNonOverflOutl)
{
    OutlinerParaObject *pNewText = impGetNonOverflowingParaObject(pNonOverflOutl);
    // adds it to current outliner anyway (useful in static decomposition)
    pNonOverflOutl->SetText(*pNewText);

    mpTargetLink->NbcSetOutlinerParaObject(pNewText);

}

void TextChainFlow::impMoveChainedTextToNextLink(SdrOutliner *pOverflOutl)
{
    // prevent copying text in same box
    if ( mpNextLink ==  mpTargetLink ) {
        fprintf(stderr, "[CHAINING] Trying to copy text for next link in same object\n");
        return;
    }

    OutlinerParaObject *pNewText = impGetOverflowingParaObject(pOverflOutl);
    if (pNewText)
        mpNextLink->NbcSetOutlinerParaObject(pNewText);
}

OutlinerParaObject *TextChainFlow::impGetNonOverflowingParaObject(SdrOutliner *pOutliner)
{
    if (mpNonOverflowingTxt == NULL)
        return NULL;

    if (mpNonOverflowingTxt->mPreOverflowingTxt == "" &&
        mpNonOverflowingTxt->mpHeadParas != NULL) {
        // Only (possibly empty) paragraphs before overflowing one
        pOutliner->SetText(*mpNonOverflowingTxt->mpHeadParas);
    } else { // We have to include the non-overflowing lines from the overfl. para

        // first make a ParaObject for the strings
        impSetOutlinerToEmptyTxt(pOutliner);
        Paragraph *pTmpPara0 = pOutliner->GetParagraph(0);
        pOutliner->SetText(mpNonOverflowingTxt->mPreOverflowingTxt, pTmpPara0);
        OutlinerParaObject *pPObj = pOutliner->CreateParaObject();
        //pOutliner->Clear();
        //pOutliner->SetStyleSheet( 0, pEdtOutl->GetStyleSheet(0));

        if (mpNonOverflowingTxt->mpHeadParas != NULL) {
            pOutliner->SetText(*mpNonOverflowingTxt->mpHeadParas);
            pOutliner->AddText(*pPObj);
         } else  if (mpNonOverflowingTxt->mPreOverflowingTxt != "") { // only preoverflowing txt
            //OutlinerParaObject *pEmptyPObj = pOutliner->GetEmptyParaObject();
            //pOutliner->SetText(*pEmptyPObj);
            pOutliner->SetText(*pPObj);
        } else { // no text // This case is redundant but it doesn't hurt for now
            pOutliner->Clear();
        }
    }

     return pOutliner->CreateParaObject();
}

void TextChainFlow::impSetOutlinerToEmptyTxt(SdrOutliner *pOutliner)
{
    OutlinerParaObject *pEmptyTxt = pOutliner->GetEmptyParaObject();
    pOutliner->SetText(*pEmptyTxt);
}

SdrTextObj *TextChainFlow::GetLinkTarget()
{
    return mpTargetLink;
}

OutlinerParaObject *TextChainFlow::impGetOverflowingParaObject(SdrOutliner *pOutliner)
{

    if (mpOverflowingTxt == NULL)
        return NULL;

    OutlinerParaObject *pCurTxt = mpNextLink->GetOutlinerParaObject();
    pOutliner->SetText(*pCurTxt);

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
        impSetOutlinerToEmptyTxt(pOutliner);

        pTmpPara0 = pOutliner->GetParagraph(0);
        pOutliner->SetText(mpOverflowingTxt->GetEndingLines() + aOldPara0Txt, pTmpPara0);
        pJoiningPara = pOutliner->CreateParaObject();
    }

    // start actual composition
    //pOutliner->Clear();
    impSetOutlinerToEmptyTxt(pOutliner);

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

TextChain *TextChainFlow::GetTextChain()
{
    return mpTextChain;
}

EditingTextChainFlow::EditingTextChainFlow(SdrTextObj *pLinkTarget) :
    TextChainFlow(pLinkTarget)
{
}

void EditingTextChainFlow::CheckForFlowEvents(SdrOutliner *pFlowOutl)
{
    // if this is editing outliner no need to set parameters
    if (pFlowOutl == GetLinkTarget()->pEdtOutl)
        impCheckForFlowEvents(pFlowOutl, NULL);
    else
        impCheckForFlowEvents(pFlowOutl, GetLinkTarget()->pEdtOutl);

}

/*
void EditingTextChainFlow::ExecuteOverflow(SdrOutliner *pOutl1, SdrOutliner *pOutl2)
{


    impSetTextForEditingOutliner

    // Set cursor
    pEditView->pImpEditView->SetEditSelection( aCurSel );
    pEditView->pImpEditView->DrawSelection();
    pEditView->ShowCursor( true, false );


}
*
* */

void EditingTextChainFlow::impLeaveOnlyNonOverflowingText(SdrOutliner *pNonOverflOutl)
{
    OutlinerParaObject *pNewText = impGetNonOverflowingParaObject(pNonOverflOutl);
    impSetTextForEditingOutliner(pNewText);

    GetLinkTarget()->NbcSetOutlinerParaObject(pNewText);
}

void EditingTextChainFlow::impSetTextForEditingOutliner(OutlinerParaObject *pNewText)
{
    if (GetLinkTarget()->pEdtOutl != NULL) {
        GetLinkTarget()->pEdtOutl->SetText(*pNewText);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
