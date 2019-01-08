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

#include <sfx2/app.hxx>
#include <svl/slstitm.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swdtflvr.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <vcl/mnemonic.hxx>

// Undo ends all modes. If a selection is emerged by the Undo,
// this must be considered for further action.

void SwWrtShell::Do( DoType eDoType, sal_uInt16 nCnt )
{
    // #105332# save current state of DoesUndo()
    bool bSaveDoesUndo = DoesUndo();

    StartAllAction();
    switch (eDoType)
    {
        case UNDO:
            DoUndo(false); // #i21739#
            // Reset modes
            EnterStdMode();
            SwEditShell::Undo(nCnt);
            break;
        case REDO:
            DoUndo(false); // #i21739#
            // Reset modes
            EnterStdMode();
            SwEditShell::Redo( nCnt );
            break;
        case REPEAT:
            // #i21739# do not touch undo flag here !!!
            SwEditShell::Repeat( nCnt );
            break;
    }
    EndAllAction();
    // #105332# restore undo state
    DoUndo(bSaveDoesUndo);

    bool bCreateXSelection = false;
    const bool bFrameSelected = IsFrameSelected() || IsObjSelected();
    if ( IsSelection() )
    {
        if ( bFrameSelected )
            UnSelectFrame();

        // Set the function pointer for canceling the selection at the
        // cursor position.
        m_fnKillSel = &SwWrtShell::ResetSelect;
        m_fnSetCursor = &SwWrtShell::SetCursorKillSel;
        bCreateXSelection = true;
    }
    else if ( bFrameSelected )
    {
        EnterSelFrameMode();
        bCreateXSelection = true;
    }
    else if( (CNT_GRF | CNT_OLE ) & GetCntType() )
    {
        SelectObj( GetCharRect().Pos() );
        EnterSelFrameMode();
        bCreateXSelection = true;
    }

    if( bCreateXSelection )
        SwTransferable::CreateSelection( *this );

    // Bug 32918: After deleting of the numbering the object panel remains.
    //            Why is not here always a CallChgLink called?
    CallChgLnk();
}

OUString SwWrtShell::GetDoString( DoType eDoType ) const
{
    OUString aUndoStr;
    const char* pResStr = STR_UNDO;
    switch( eDoType )
    {
    case UNDO:
        pResStr = STR_UNDO;
        (void)GetLastUndoInfo(&aUndoStr, nullptr, &m_rView);
        break;
    case REDO:
        pResStr = STR_REDO;
        (void)GetFirstRedoInfo(&aUndoStr, nullptr, &m_rView);
        break;
    default:;//prevent warning
    }

    return MnemonicGenerator::EraseAllMnemonicChars(SvtResId(pResStr)) + aUndoStr;
}

void SwWrtShell::GetDoStrings( DoType eDoType, SfxStringListItem& rStrs ) const
{
    SwUndoComments_t comments;
    switch( eDoType )
    {
    case UNDO:
        comments = GetIDocumentUndoRedo().GetUndoComments();
        break;
    case REDO:
        comments = GetIDocumentUndoRedo().GetRedoComments();
        break;
    default:;//prevent warning
    }

    OUStringBuffer buf;
    for (const OUString & comment : comments)
    {
        OSL_ENSURE(!comment.isEmpty(), "no Undo/Redo Text set");
        buf.append(comment).append("\n");
    }
    rStrs.SetString(buf.makeStringAndClear());
}

OUString SwWrtShell::GetRepeatString() const
{
    OUString str;
    GetRepeatInfo(& str);

    if (str.isEmpty())
    {
        return str;
    }

    return SvtResId(STR_REPEAT) + str;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
