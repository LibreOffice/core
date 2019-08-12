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

void URLFieldHelper::RemoveURLField(Outliner* pOutl, OutlinerView* pOLV)
{
    if (!pOutl || !pOLV)
        return;

    const SvxFieldData* pField = pOLV->GetFieldAtCursor();
    if (auto pUrlField = dynamic_cast<const SvxURLField*>(pField))
    {
        ESelection aSel = pOLV->GetSelection();
        pOutl->QuickInsertText(pUrlField->GetRepresentation(), aSel);
        pOLV->GetEditView().Invalidate();
    }
}

bool URLFieldHelper::IsCursorAtURLField(OutlinerView* pOLV)
{
    if (!pOLV)
        return false;

    const SvxFieldData* pField = pOLV->GetFieldAtCursor();
    if (dynamic_cast<const SvxURLField*>(pField))
        return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
