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


// Helper function for *OverflowingText classes

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

// class OverflowingText

OverflowingText::OverflowingText(EditTextObject *pTObj)
    : mpContentTextObj(pTObj->Clone())
{
    // XXX: may have to delete pTObj
}

ESelection OverflowingText::GetInsertionPointSel() const
{
    return getLastPositionSel(mpContentTextObj);
}

// class NonOverflowingText

NonOverflowingText::NonOverflowingText(const EditTextObject *pTObj,  bool bLastParaInterrupted)
    : mpContentTextObj(pTObj->Clone()),
      mbLastParaInterrupted(bLastParaInterrupted)
{
     // XXX: may have to delete pTObj
}

bool NonOverflowingText::IsLastParaInterrupted() const
{
    return mbLastParaInterrupted;
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

OutlinerParaObject *OFlowChainedText::CreateOverflowingParaObject(Outliner *pOutliner, OutlinerParaObject *pTextToBeMerged)
{
    // Just return the roughly merged paras for now
    if (mpOverflowingTxt == NULL || pTextToBeMerged == NULL)
        return NULL;

    return mpOverflowingTxt->GetJuxtaposedParaObject(pOutliner, pTextToBeMerged );
}

OutlinerParaObject *OFlowChainedText::CreateNonOverflowingParaObject(Outliner *pOutliner)
{
    if (mpNonOverflowingTxt == NULL)
        return NULL;

    return mpNonOverflowingTxt->ToParaObject(pOutliner);
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
    OutlinerParaObject *pCurText = mpUnderflowPObj;

    // NewTextForCurBox = Txt(CurBox) ++ Txt(NextBox)
    pOutl->SetText(*pCurText);
    pOutl->AddText(*pNextLinkWholeText);
    OutlinerParaObject *pNewText = pOutl->CreateParaObject();

    return pNewText;

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
