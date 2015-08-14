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
#include <editeng/editdata.hxx>

#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>


/* Helper functions for *OverflowingText classes  */

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

// Put a para next to each other in the same OutlinerParaObject
OutlinerParaObject *impGetJuxtaposedParaObject(Outliner *pOutl,
                                               OutlinerParaObject *pPObj1,
                                               OutlinerParaObject *pPObj2)
{
    assert(pOutl && pPObj1 &&  pPObj2);

    pOutl->SetText(*pPObj1);
    pOutl->AddText(*pPObj2);
    OutlinerParaObject *pPObj = pOutl->CreateParaObject();

    return pPObj;
}

// In a deep merge parts of text are not only juxtaposed but the last and first para become the same
OutlinerParaObject *impGetDeeplyMergedParaObject(Outliner *pOutl,
                                               OutlinerParaObject *pPObj1,
                                               OutlinerParaObject *pPObj2)
{
    assert(pOutl && pPObj1 &&  pPObj2);

    const EditTextObject rTObj1 = pPObj1->GetTextObject();
    const EditTextObject rTObj2 = pPObj2->GetTextObject();
    sal_Int32 nParaCount1 = rTObj1.GetParagraphCount();

    // If no paras in the first text, just use second text
    if (nParaCount1 == 0) {
        pOutl->SetText(*pPObj2);
        return pOutl->CreateParaObject();
    }


    sal_Int32 nLastPara1 = nParaCount1 - 1;

    // If last para of first text is empty, discard it and just juxtapose
    if (rTObj1.GetText(nLastPara1) == "" && nParaCount1 >= 2) {
        pOutl->SetText(*pPObj1);
        return impGetJuxtaposedParaObject(
                pOutl,
                pOutl->CreateParaObject(0, nParaCount1 - 1),
                pPObj2);
    }

    /* --- Standard procedure: when pPObj1 is 'fine' --- */


    // Cut first para of second object
    OUString aFirstParaTxt2 = rTObj2.GetText(0);

    // Prepare remainder for text 2
    OutlinerParaObject *pRemainderPObj2 = NULL;
    if (rTObj2.GetParagraphCount() > 1) {
        pOutl->SetText(*pPObj2);
        pRemainderPObj2 = pOutl->CreateParaObject(1); // from second para on
    } else { // No text to append
        pRemainderPObj2 = NULL;
    }

    // Set first object as text
    pOutl->SetText(*pPObj1);

    // Merges LastPara(pPObj1) with FirstPara(pPObj2)
    Paragraph *pLastPara1 = pOutl->GetParagraph(nLastPara1);
    OUString aLastParaTxt1 = pOutl->GetText(pLastPara1);
    pOutl->SetText(aLastParaTxt1 + aFirstParaTxt2, pLastPara1); // XXX: This way it screws up attributes!

    // add the remainder of the second text
    if (pRemainderPObj2)
        pOutl->AddText(*pRemainderPObj2);

    return pOutl->CreateParaObject();
}

// class OverflowingText

OverflowingText::OverflowingText(com::sun::star::uno::Reference<
        com::sun::star::datatransfer::XTransferable> xOverflowingContent) :
        mxOverflowingContent(xOverflowingContent)
{

}


ESelection OverflowingText::GetInsertionPointSel() const
{
    assert(0);
    return getLastPositionSel(NULL);
}

// class NonOverflowingText

NonOverflowingText::NonOverflowingText(const EditTextObject *pTObj,  bool bLastParaInterrupted)
    : mpContentTextObj(pTObj->Clone()),
      mbLastParaInterrupted(bLastParaInterrupted)
{
     // XXX: may have to delete pTObj
}

NonOverflowingText::NonOverflowingText(const ESelection &aSel, bool bLastParaInterrupted)
    : maContentSel(aSel),
      mbLastParaInterrupted(bLastParaInterrupted)
{
}

bool NonOverflowingText::IsLastParaInterrupted() const
{
    return mbLastParaInterrupted;
}


OutlinerParaObject *NonOverflowingText::RemoveOverflowingText(Outliner *pOutliner) const
{
    pOutliner->QuickDelete(maContentSel);
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
    if (!pNextPObj) {
        pOutl->SetToEmptyText();
    } else {
        pOutl->SetText(*pNextPObj);
    }

    // XXX: this code should be moved in Outliner directly
    //          creating Outliner::InsertText(...transferable...)
    EditSelection aStartSel(pOutl->pEditEngine->CreateSelection(ESelection(0,0)));
    EditPaM aPaM = pOutl->pEditEngine->InsertText(mxOverflowingContent,
                                                  OUString(),
                                                  aStartSel.Min(),
                                                  true);

    // Separate Paragraphs
    pOutl->pEditEngine->InsertParaBreak(EditSelection(aPaM, aPaM));

    return pOutl->CreateParaObject();
}

OutlinerParaObject *OverflowingText::impMakeOverflowingParaObject(Outliner *pOutliner)
{
    if (mpContentTextObj == NULL) {
        fprintf(stderr, "[Chaining] OverflowingText's mpContentTextObj is NULL!\n");
        return NULL;
    }

    // Simply Juxtaposing; no within-para merging
    OutlinerParaObject *pOverflowingPObj = new OutlinerParaObject(*mpContentTextObj);
    // the OutlinerParaObject constr. at the prev line gives no valid outliner mode, so we set it
    pOverflowingPObj->SetOutlinerMode(pOutliner->GetOutlinerMode());

    return pOverflowingPObj;
}


OutlinerParaObject *OverflowingText::DeeplyMergeParaObject(Outliner *pOutl, OutlinerParaObject *pNextPObj)
{

    if (!pNextPObj) {
        pOutl->SetToEmptyText();
    } else {
        pOutl->SetText(*pNextPObj);
    }

    // XXX: this code should be moved in Outliner directly
    //          creating Outliner::InsertText(...transferable...)
    EditSelection aStartSel(pOutl->pEditEngine->CreateSelection(ESelection(0,0)));
    EditPaM aPaM = pOutl->pEditEngine->InsertText(mxOverflowingContent,
                                                  OUString(),
                                                  aStartSel.Min(),
                                                  true);

    return pOutl->CreateParaObject();
}

// class OFlowChainedText

OFlowChainedText::OFlowChainedText(Outliner *pOutl, bool bIsDeepMerge)
{
    mpOverflowingTxt = pOutl->GetOverflowingText();
    mpNonOverflowingTxt = pOutl->GetNonOverflowingText();

    mbIsDeepMerge = bIsDeepMerge;
}

ESelection OFlowChainedText::GetInsertionPointSel() const
{
    return mpOverflowingTxt->GetInsertionPointSel();
}

ESelection OFlowChainedText::GetOverflowPointSel() const
{
    return mpNonOverflowingTxt->GetOverflowPointSel();
}

OutlinerParaObject *OFlowChainedText::InsertOverflowingText(Outliner *pOutliner, OutlinerParaObject *pTextToBeMerged)
{
    // Just return the roughly merged paras for now
    if (mpOverflowingTxt == NULL)
        return NULL;

    if (mbIsDeepMerge) {
        fprintf(stderr, "[TEXTCHAINFLOW - OF] Deep merging paras\n" );
        return mpOverflowingTxt->DeeplyMergeParaObject(pOutliner, pTextToBeMerged );
    } else {
        fprintf(stderr, "[TEXTCHAINFLOW - OF] Juxtaposing paras\n" );
        return mpOverflowingTxt->JuxtaposeParaObject(pOutliner, pTextToBeMerged );
    }
}


OutlinerParaObject *OFlowChainedText::RemoveOverflowingText(Outliner *pOutliner)
{
    if (mpNonOverflowingTxt == NULL)
        return NULL;

    return mpNonOverflowingTxt->RemoveOverflowingText(pOutliner);
}

bool OFlowChainedText::IsLastParaInterrupted() const
{
    return mpNonOverflowingTxt->IsLastParaInterrupted();
}


// classes UFlowChainedText

UFlowChainedText::UFlowChainedText(Outliner *pOutl, bool bIsDeepMerge)
{
    mpUnderflowPObj = pOutl->CreateParaObject();
    mbIsDeepMerge = bIsDeepMerge;
}

OutlinerParaObject *UFlowChainedText::CreateMergedUnderflowParaObject(Outliner *pOutl, OutlinerParaObject *pNextLinkWholeText)
{
    OutlinerParaObject *pNewText = NULL;
    OutlinerParaObject *pCurText = mpUnderflowPObj;

    if (mbIsDeepMerge) {
        fprintf(stderr, "[TEXTCHAINFLOW - UF] Deep merging paras\n" );
        pNewText = impGetDeeplyMergedParaObject(pOutl, pCurText, pNextLinkWholeText);
    } else {
        // NewTextForCurBox = Txt(CurBox) ++ Txt(NextBox)
        fprintf(stderr, "[TEXTCHAINFLOW - UF] Juxtaposing paras\n" );
        pNewText = impGetJuxtaposedParaObject(pOutl, pCurText, pNextLinkWholeText);
    }

    return pNewText;

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
