/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <editeng/urlfieldhelper.hxx>

#include <editeng/flditem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>

void URLFieldHelper::RemoveURLField(EditView& pEditView)
{
    pEditView.SelectFieldAtCursor();
    const SvxFieldData* pField = pEditView.GetFieldAtCursor();
    if (auto pUrlField = dynamic_cast<const SvxURLField*>(pField))
    {
        ESelection aSel = pEditView.GetSelection();
        pEditView.GetEditEngine()->QuickInsertText(pUrlField->GetRepresentation(), aSel);
        pEditView.Invalidate();
    }
}

bool URLFieldHelper::IsCursorAtURLField(const EditView& pEditView)
{
    // tdf#128666 Make sure only URL field (or nothing) is selected
    ESelection aSel = pEditView.GetSelection();
    auto nSelectedParas = aSel.nEndPara - aSel.nStartPara;
    auto nSelectedChars = aSel.nEndPos - aSel.nStartPos;
    bool bIsValidSelection
        = nSelectedParas == 0
          && (nSelectedChars == 0 || nSelectedChars == 1 || nSelectedChars == -1);
    if (!bIsValidSelection)
        return false;

    const SvxFieldData* pField = pEditView.GetFieldAtCursor();
    if (dynamic_cast<const SvxURLField*>(pField))
        return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
