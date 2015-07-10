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

    mbOFisUFinduced = false;

    mpOverflChText = NULL;
    mpUnderflChText = NULL;

    maCursorEvent = CursorChainingEvent::NULL_EVENT;
    mbPossiblyCursorOut = false;
}


TextChainFlow::~TextChainFlow()
{
    if (mpOverflChText)
        delete mpOverflChText;
    if (mpUnderflChText)
        delete mpUnderflChText;
}

void TextChainFlow::impSetFlowOutlinerParams(SdrOutliner *, SdrOutliner *)
{
    // Nothing to do if not in editing mode
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
    bool bOldUpdateMode = pFlowOutl->GetUpdateMode();

    // XXX: This could be reorganized moving most of this stuff inside EditingTextChainFlow (we need update=true anyway for TextChainFlow though)
    if (pParamOutl != NULL)
    {
        // We need this since it's required to check overflow
        pFlowOutl->SetUpdateMode(true);

        // XXX: does this work if you do it before setting the text? Seems so.
        impSetFlowOutlinerParams(pFlowOutl, pParamOutl);
    }

    bool bIsPageOverflow = pFlowOutl->IsPageOverflow();

    // NOTE: overflow and underflow cannot be both true
    bOverflow = bIsPageOverflow && mpNextLink;
    bUnderflow = !bIsPageOverflow &&  mpNextLink && mpNextLink->HasText();

    if (pParamOutl != NULL)
    {
        pFlowOutl->SetUpdateMode(bOldUpdateMode);
    }

    // Set (Non)OverflowingTxt here (if any)
    mpOverflChText = bOverflow ? new OFlowChainedText(pFlowOutl) : NULL;

    // Set current underflowing text (if any)
    mpUnderflChText = bUnderflow ? new UFlowChainedText(pFlowOutl) : NULL;

    // NOTE: Must be called after mp*ChText abd b*flow have been set but before mbOFisUFinduced is reset
    impUpdateCursorInfo();

    // To check whether an overflow is underflow induced or not (useful in cursor checking)
    mbOFisUFinduced = bUnderflow;

}

void TextChainFlow::impUpdateCursorInfo()
{
    // XXX: Maybe we can get rid of mbOFisUFinduced by not allowing handling of more than one event by the same TextChainFlow
    // if this is not an OF triggered during an UF

    mbPossiblyCursorOut = bOverflow && !mbOFisUFinduced;
    if (mbPossiblyCursorOut) { // if this is false, mpOverflChText might be NULL
        maOverflowPosSel = ESelection(mpOverflChText->GetOverflowPointSel());
        // After the chaining event the cursor is where the text from the source box merged with the rest
        maPostChainingSel = ESelection(mpOverflChText->GetInsertionPointSel());
    }
}

void TextChainFlow::CheckForFlowEvents(SdrOutliner *pFlowOutl)
{
    impCheckForFlowEvents(pFlowOutl, NULL);
}


bool TextChainFlow::IsOverflow() const
{
    return bOverflow;
}

bool TextChainFlow::IsUnderflow() const
{
    return bUnderflow;
}


// XXX: In editing mode you need to get "underflowing" text from editing outliner, so it's kinda separate from the drawing one!

// XXX:Would it be possible to unify undeflow and its possibly following overrflow?
void TextChainFlow::ExecuteUnderflow(SdrOutliner *pOutl)
{
    // making whole text
    OutlinerParaObject *pNewText = impGetMergedUnderflowParaObject(pOutl);

    // Set the other box empty; it will be replaced by the rest of the text if overflow occurs
    if (!mpTargetLink->GetPreventChainable())
        mpNextLink->NbcSetOutlinerParaObject(pOutl->GetEmptyParaObject());

    mpTargetLink->NbcSetOutlinerParaObject(pNewText);

    // Check for new overflow
    CheckForFlowEvents(pOutl);
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
    return mpOverflChText->CreateNonOverflowingParaObject(pOutliner);
}

SdrTextObj *TextChainFlow::GetLinkTarget() const
{
    return mpTargetLink;
}

SdrTextObj *TextChainFlow::GetNextLink() const
{
    return mpNextLink;
}

OutlinerParaObject *TextChainFlow::impGetOverflowingParaObject(SdrOutliner *pOutliner)
{
    return mpOverflChText->CreateOverflowingParaObject(pOutliner, mpNextLink->GetOutlinerParaObject());
}

OutlinerParaObject *TextChainFlow::impGetMergedUnderflowParaObject(SdrOutliner *pOutliner)
{
    // Should check whether to merge paragraphs or not
    return mpUnderflChText->CreateMergedUnderflowParaObject(pOutliner, mpNextLink->GetOutlinerParaObject());
}

TextChain *TextChainFlow::GetTextChain() const
{
    return mpTextChain;
}

OFlowChainedText *TextChainFlow::GetOverflowChainedText() const
{
    return mpOverflChText;
}

UFlowChainedText *TextChainFlow::GetUnderflowChainedText() const
{
    return mpUnderflChText;
}


// EditingTextChainFlow

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

    // Broadcast events for cursor handling
    impBroadcastCursorInfo();
}

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

void EditingTextChainFlow::impSetFlowOutlinerParams(SdrOutliner *pFlowOutl, SdrOutliner *pParamOutl)
{
    // Set right size for overflow
    pFlowOutl->SetMaxAutoPaperSize(pParamOutl->GetMaxAutoPaperSize());
    pFlowOutl->SetMinAutoPaperSize(pParamOutl->GetMinAutoPaperSize());
    pFlowOutl->SetPaperSize(pParamOutl->GetPaperSize());
}

void EditingTextChainFlow::impBroadcastCursorInfo() const
{
    bool bCursorOut = false;

    // NOTE: I handled already the stuff for the comments below. They will be kept temporarily till stuff settles down.
    // Possibility: 1) why don't we stop passing the actual event to the TextChain and instead we pass
    //              the overflow pos and mbPossiblyCursorOut
    //              2) We pass the current selection before anything happens and we make impBroadcastCursorInfo compute it.

    if (mbPossiblyCursorOut) {
        ESelection aPreChainingSel = GetTextChain()->GetPreChainingSel(GetLinkTarget()) ;
        // Test whether the cursor is out of the box.
        bCursorOut = maOverflowPosSel.IsLess(aPreChainingSel);
    }

    if (bCursorOut) {
            //maCursorEvent = CursorChainingEvent::TO_NEXT_LINK;
            GetTextChain()->SetPostChainingSel(GetLinkTarget(), maPostChainingSel);
            GetTextChain()->SetCursorEvent(GetLinkTarget(), CursorChainingEvent::TO_NEXT_LINK);
    } else {
        //maCursorEvent = CursorChainingEvent::UNCHANGED;
        GetTextChain()->SetCursorEvent(GetLinkTarget(), CursorChainingEvent::UNCHANGED);
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
