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

// XXX: Possible duplication of code in behavior with stuff in ImpEditView (or ImpEditEngine) and OutlinerView

// XXX: We violate Demeter's Law several times here, I'm afraid

TextChainCursorManager::TextChainCursorManager(SdrObjEditView *pEditView, const SdrTextObj *pTextObj) :
    mpEditView(pEditView),
    mpTextObj(pTextObj)
{
    assert(mpEditView);
    assert(mpTextObj);

}

bool TextChainCursorManager::HandleKeyEvent( const KeyEvent& rKEvt ) const
{
    ESelection aNewSel;
    CursorChainingEvent aCursorEvent;

    // check what the cursor/event situation looks like
    impDetectEvent(rKEvt, &aCursorEvent, &aNewSel);

    if (aCursorEvent == CursorChainingEvent::NULL_EVENT)
        return false;
    else {
        HandleCursorEvent(aCursorEvent, aNewSel);
        return true;
    }
}

void TextChainCursorManager::impDetectEvent(const KeyEvent& rKEvt,
                                            CursorChainingEvent *pOutCursorEvt,
                                            ESelection *pOutSel) const
{
    SdrOutliner *pOutl = mpEditView->GetTextEditOutliner();
    OutlinerView *pOLV = mpEditView->GetTextEditOutlinerView();

    SdrTextObj *pNextLink = mpTextObj->GetNextLinkInChain();
    SdrTextObj *pPrevLink = mpTextObj->GetPrevLinkInChain();

    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();

    // We need to have this KeyFuncType
    if (eFunc !=  KeyFuncType::DONTKNOW)
    {
        *pOutCursorEvt = CursorChainingEvent::NULL_EVENT;
        return;
    }

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    ESelection aCurSel = pOLV->GetSelection();

    sal_Int32 nLastPara = pOutl->GetParagraphCount()-1;
    OUString aLastParaText = pOutl->GetText(pOutl->GetParagraph(nLastPara));
    sal_Int32 nLastParaLen = aLastParaText.getLength();

    ESelection aEndSel = ESelection(nLastPara, nLastParaLen);
    bool bAtEndOfTextContent = aCurSel.IsEqual(aEndSel);

    // Are we "pushing" at the end of the object?
    if (nCode == KEY_RIGHT && bAtEndOfTextContent && pNextLink)
    {
        *pOutCursorEvt = CursorChainingEvent::TO_NEXT_LINK;
        // Selection unchanged: we are at the beginning of the box
        return;
    }

    ESelection aStartSel = ESelection(0, 0);
    bool bAtStartOfTextContent = aCurSel.IsEqual(aStartSel);

    // Are we "pushing" at the start of the object?
    if (nCode == KEY_LEFT && bAtStartOfTextContent && pPrevLink)
    {
        *pOutCursorEvt = CursorChainingEvent::TO_PREV_LINK;
        *pOutSel = ESelection(100000, 100000); // Set at end of selection
        return;
    }

    // If arrived here there is no event detected
    *pOutCursorEvt = CursorChainingEvent::NULL_EVENT;

}

void TextChainCursorManager::HandleCursorEvent(
                            const CursorChainingEvent aCurEvt,
                            const ESelection  aNewSel)
                            const
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

void TextChainCursorManager::impChangeEditingTextObj(SdrTextObj *pTargetTextObj, ESelection aNewSel) const
{
    assert(pTargetTextObj);

    mpEditView->SdrEndTextEdit();
    mpEditView->SdrBeginTextEdit(pTargetTextObj);
    // OutlinerView has changed, so we update the pointer
    OutlinerView *pOLV = mpEditView->GetTextEditOutlinerView();
    pOLV->SetSelection(aNewSel); // XXX
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
