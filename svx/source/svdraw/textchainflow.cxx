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

    mbOFisUFinduced = false;

    mpOverflChText = NULL;
    mpUnderflChText = NULL;
}


TextChainFlow::~TextChainFlow()
{

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

    impUpdateCursorEvent(pFlowOutl, bOverflow);

    if (pParamOutl != NULL)
    {
        pFlowOutl->SetUpdateMode(bOldUpdateMode);
    }

    // Set (Non)OverflowingTxt here (if any)
    mpOverflChText = bOverflow ? new OFlowChainedText(pFlowOutl) : NULL;

    // Set current underflowing text (if any)
    mpUnderflChText = bUnderflow ? new UFlowChainedText(pFlowOutl) : NULL;

    // To check whether an overflow is underflow induced or not (useful in cursor checking)
    mbOFisUFinduced = bUnderflow;

}

void TextChainFlow::impUpdateCursorEvent(SdrOutliner *, bool bIsOverflow)
{
    // XXX: Current implementation might create problems with UF-
    //      In fact UF causes a


    if (bIsOverflow && !mbOFisUFinduced) {
        bool bCursorOut = true; // XXX: Should have real check
        if (bCursorOut) {
            GetTextChain()->SetCursorEvent(GetLinkTarget(),
                                     CursorChainingEvent::TO_NEXT_LINK);
        }
    } else {
        GetTextChain()->SetCursorEvent(GetLinkTarget(),
                                     CursorChainingEvent::UNCHANGED);
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

TextChain *TextChainFlow::GetTextChain()
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

void EditingTextChainFlow::impSetFlowOutlinerParams(SdrOutliner *pFlowOutl, SdrOutliner *pParamOutl)
{
    // Set right size for overflow
    pFlowOutl->SetMaxAutoPaperSize(pParamOutl->GetMaxAutoPaperSize());
    pFlowOutl->SetMinAutoPaperSize(pParamOutl->GetMinAutoPaperSize());
    pFlowOutl->SetPaperSize(pParamOutl->GetPaperSize());

    // Set right text attributes // XXX: Not enough: it does not handle complex attributes
    //pFlowOutl->SetEditTextObjectPool(pParamOutl->GetEditTextObjectPool());
}

/*
 *
 * Some notes on how to set style sheets:
 * - save whole edittexts instead of strings only for (Non)OverflowingText; this can be done by the EditEngine::CreateTextObject method and using a selection - probably from ImpEditEngine)
 * - for the refactoring of the previous point we may also add an option for whether we are joining paragraphs or not
 * - When making new OutlinerParaObjs and joining paragraphs we need to first add the string (as we already do) and then, with the appropriate selection, use Outliner::QuickSetAttribs(SfxItemSet(txtObj->GetPool()), aSelectionOfTheNewText)
 * - having all this in a whole class that contains Overflowing and NonOverflowingText would not be bad. This same class could be used to handle a cursor later on.
 *
 *
 *
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
