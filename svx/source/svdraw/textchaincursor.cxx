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
#include <svx/textchaincursor.hxx>
#include <svx/svdedxv.hxx>
#include <svx/svdoutl.hxx>
#include <vcl/event.hxx>

// XXX: Possible duplication of code in behavior with stuff in ImpEditView (or ImpEditEngine) and OutlinerView

// XXX: We violate Demeter's Law several times here, I'm afraid

TextChainCursorManager::TextChainCursorManager(SdrObjEditView *pEditView, const SdrTextObj *pTextObj) :
    mpEditView(pEditView),
    mpTextObj(pTextObj),
    mbHandlingDel(false)
{
    assert(mpEditView);
    assert(mpTextObj);

}

bool TextChainCursorManager::HandleKeyEvent( const KeyEvent& rKEvt )
{
    ESelection aNewSel;
    CursorChainingEvent aCursorEvent;

    // check what the cursor/event situation looks like
    bool bCompletelyHandled = false;
    impDetectEvent(rKEvt, aCursorEvent, aNewSel, bCompletelyHandled);

    if (aCursorEvent == CursorChainingEvent::NULL_EVENT)
        return false;
    else {
        HandleCursorEvent(aCursorEvent, aNewSel);
        // return value depends on the situation we are in
        return bCompletelyHandled;
    }
}

void TextChainCursorManager::impDetectEvent(const KeyEvent& rKEvt,
                                            CursorChainingEvent& rOutCursorEvt,
                                            ESelection& rOutSel,
                                            bool& rOutHandled)
{
    SdrOutliner *pOutl = mpEditView->GetTextEditOutliner();
    OutlinerView *pOLV = mpEditView->GetTextEditOutlinerView();

    SdrTextObj *pNextLink = mpTextObj->GetNextLinkInChain();
    SdrTextObj *pPrevLink = mpTextObj->GetPrevLinkInChain();

    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();

    // We need to have this KeyFuncType
    if (eFunc !=  KeyFuncType::DONTKNOW && eFunc != KeyFuncType::DELETE)
    {
        rOutCursorEvt = CursorChainingEvent::NULL_EVENT;
        return;
    }

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    ESelection aCurSel = pOLV->GetSelection();

    ESelection aEndSelPrevBox(100000, 100000);

    sal_Int32 nLastPara = pOutl->GetParagraphCount()-1;
    OUString aLastParaText = pOutl->GetText(pOutl->GetParagraph(nLastPara));
    sal_Int32 nLastParaLen = aLastParaText.getLength();

    ESelection aEndSel = ESelection(nLastPara, nLastParaLen);
    bool bAtEndOfTextContent = aCurSel == aEndSel;

    // Possibility: Are we "pushing" at the end of the object?
    if (nCode == KEY_RIGHT && bAtEndOfTextContent && pNextLink)
    {
        rOutCursorEvt = CursorChainingEvent::TO_NEXT_LINK;
        // Selection unchanged: we are at the beginning of the box
        rOutHandled = true; // Nothing more to do than move cursor
        return;
    }

    // Possibility: Are we "pushing" at the end of the object?
    if (eFunc == KeyFuncType::DELETE && bAtEndOfTextContent && pNextLink)
    {
        rOutCursorEvt = CursorChainingEvent::TO_NEXT_LINK;
        // Selection unchanged: we are at the beginning of the box
        rOutHandled = false; // We still need to delete the characters
        mbHandlingDel = true;
        return;
    }

    ESelection aStartSel = ESelection(0, 0);
    bool bAtStartOfTextContent = aCurSel == aStartSel;

    // Possibility: Are we "pushing" at the start of the object?
    if (nCode == KEY_LEFT && bAtStartOfTextContent && pPrevLink)
    {
        rOutCursorEvt = CursorChainingEvent::TO_PREV_LINK;
        rOutSel = aEndSelPrevBox; // Set at end of selection
        rOutHandled = true; // Nothing more to do than move cursor
        return;
    }

    // Possibility: Are we "pushing" at the start of the object and deleting left?
    if (nCode == KEY_BACKSPACE && bAtStartOfTextContent && pPrevLink)
    {
        rOutCursorEvt = CursorChainingEvent::TO_PREV_LINK;
        rOutSel = aEndSelPrevBox; // Set at end of selection
        rOutHandled = false; // We need to delete characters after moving cursor
        return;
    }

    // If arrived here there is no event detected
    rOutCursorEvt = CursorChainingEvent::NULL_EVENT;

}

void TextChainCursorManager::HandleCursorEventAfterChaining(
                            const CursorChainingEvent aCurEvt,
                            const ESelection& aNewSel)

{
     // Special case for DELETE handling: we need to get back at the end of the prev box
    if (mbHandlingDel) {
        // reset flag
        mbHandlingDel = false;

        // Move to end of prev box
        SdrTextObj *pPrevLink = mpTextObj->GetPrevLinkInChain();
        ESelection aEndSel(100000, 100000);
        impChangeEditingTextObj(pPrevLink, aEndSel);
        return;
    }

    // Standard handling
    HandleCursorEvent(aCurEvt, aNewSel);
}


void TextChainCursorManager::HandleCursorEvent(
                            const CursorChainingEvent aCurEvt,
                            const ESelection& aNewSel)

{

    OutlinerView* pOLV = mpEditView->GetTextEditOutlinerView();
    SdrTextObj *pNextLink = mpTextObj->GetNextLinkInChain();
    SdrTextObj *pPrevLink = mpTextObj->GetPrevLinkInChain();


    switch ( aCurEvt ) {
            case CursorChainingEvent::UNCHANGED:
                // Set same selection as before the chaining (which is saved as PostChainingSel)
                // We need an explicit set because the Outliner is messed up
                //    after text transfer and otherwise it brings us at arbitrary positions.
                pOLV->SetSelection(aNewSel);
                break;
            case CursorChainingEvent::TO_NEXT_LINK:
                mpTextObj->GetTextChain()->SetSwitchingToNextBox(mpTextObj, true);
                impChangeEditingTextObj(pNextLink, aNewSel);
                break;
            case CursorChainingEvent::TO_PREV_LINK:
                impChangeEditingTextObj(pPrevLink, aNewSel);
                break;
            case CursorChainingEvent::NULL_EVENT:
                // Do nothing here
                break;
    }

}

void TextChainCursorManager::impChangeEditingTextObj(SdrTextObj *pTargetTextObj, ESelection aNewSel)
{
    assert(pTargetTextObj);

    // To ensure that we check for overflow in the next box // This is handled in SdrTextObj::EndTextEdit
    SdrTextObj *pNextLink = mpTextObj->GetNextLinkInChain();
    TextChain *pTextChain = mpTextObj->GetTextChain();
    // If we are moving forward
    if (pNextLink && pTargetTextObj == pNextLink)
        pTextChain->SetPendingOverflowCheck(pNextLink, true);

    mpEditView->SdrEndTextEdit();
    mpEditView->SdrBeginTextEdit(pTargetTextObj);
    // OutlinerView has changed, so we update the pointer
    OutlinerView *pOLV = mpEditView->GetTextEditOutlinerView();
    pOLV->SetSelection(aNewSel);

    // Update reference text obj
    mpTextObj = pTargetTextObj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
