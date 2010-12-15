/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#define _SVSTDARR_STRINGSDTOR
#include <tools/resid.hxx>
#include <sfx2/app.hxx>
#include <svl/slstitm.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>                   // fuer Undo-Ids
#include <IDocumentUndoRedo.hxx>
#include <swdtflvr.hxx>

#include <wrtsh.hrc>
#include <sfx2/sfx.hrc>


// Undo beendet alle Modi. Falls eine Selektion durch das Undo entstanden
// ist, muss die fuer die weiteren Aktionen beruecksichtigt werden.


void SwWrtShell::Do( DoType eDoType, USHORT nCnt )
{
    // #105332# save current state of DoesUndo()
    sal_Bool bSaveDoesUndo = DoesUndo();

    StartAllAction();
     switch( eDoType )
    {
        case UNDO:
            DoUndo(sal_False); // #i21739#
            // Modi zuruecksetzen
            EnterStdMode();
            SwEditShell::Undo(UNDO_EMPTY, nCnt );
            break;
        case REDO:
            DoUndo(sal_False); // #i21739#
            // Modi zuruecksetzen
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

    BOOL bCreateXSelection = FALSE;
    const BOOL bFrmSelected = IsFrmSelected() || IsObjSelected();
    if ( IsSelection() )
    {
        if ( bFrmSelected )
            UnSelectFrm();

        // Funktionspointer fuer das Aufheben der Selektion setzen
        // bei Cursor setzen
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        bCreateXSelection = TRUE;
    }
    else if ( bFrmSelected )
    {
        EnterSelFrmMode();
        bCreateXSelection = TRUE;
    }
    else if( (CNT_GRF | CNT_OLE ) & GetCntType() )
    {
        SelectObj( GetCharRect().Pos() );
        EnterSelFrmMode();
        bCreateXSelection = TRUE;
    }

    if( bCreateXSelection )
        SwTransferable::CreateSelection( *this );

    // Bug 32918: nach loeschen der Numerierung bleibt die Obj. Leiste stehen
    //          Warum wird hier nicht immer ein CallChgLink gerufen?
    CallChgLnk();
}


String SwWrtShell::GetDoString( DoType eDoType ) const
{
    ::rtl::OUString aUndoStr;
    USHORT nResStr = STR_UNDO;
    switch( eDoType )
    {
    case UNDO:
        nResStr = STR_UNDO;
        GetLastUndoInfo(& aUndoStr, 0);
        break;
    case REDO:
        nResStr = STR_REDO;
        GetFirstRedoInfo(& aUndoStr);
        break;
    default:;//prevent warning
    }

    ::rtl::OUStringBuffer buf(
        String(ResId(nResStr, *SFX_APP()->GetSfxResManager())) );
    buf.append(aUndoStr);

    return buf.makeStringAndClear();
}

USHORT SwWrtShell::GetDoStrings( DoType eDoType, SfxStringListItem& rStrs ) const
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

    ::rtl::OUStringBuffer buf;
    for (size_t i = 0; i < comments.size(); ++i)
    {
        OSL_ENSURE(comments[i].getLength(), "no Undo/Redo Text set");
        buf.append(comments[i]);
        buf.append(sal_Unicode('\n'));
    }
    rStrs.SetString(buf.makeStringAndClear());
    return static_cast<sal_uInt16>(comments.size());
}


String SwWrtShell::GetRepeatString() const
{
    ::rtl::OUString str;
    GetRepeatInfo(& str);

    if (str.getLength() == 0)
    {
        return str;
    }

    ::rtl::OUStringBuffer buf(
        String(ResId(STR_REPEAT, *SFX_APP()->GetSfxResManager())) );
    buf.append(str);
    return buf.makeStringAndClear();
}


