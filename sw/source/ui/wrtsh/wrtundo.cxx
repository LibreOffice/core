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

#include <tools/resid.hxx>
#include <sfx2/app.hxx>
#include <svl/slstitm.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>                   // fuer Undo-Ids
#include <IDocumentUndoRedo.hxx>
#include <swdtflvr.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <wrtsh.hrc>
#include <sfx2/sfx.hrc>


// Undo beendet alle Modi. Falls eine Selektion durch das Undo entstanden
// ist, muss die fuer die weiteren Aktionen beruecksichtigt werden.


void SwWrtShell::Do( DoType eDoType, sal_uInt16 nCnt )
{
    // #105332# save current state of DoesUndo()
    sal_Bool bSaveDoesUndo = DoesUndo();

    StartAllAction();
    switch (eDoType)
    {
        case UNDO:
            DoUndo(sal_False); // #i21739#
            // Modi zuruecksetzen
            EnterStdMode();
            SwEditShell::Undo(nCnt);
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

    bool bCreateXSelection = false;
    const bool bFrmSelected = IsFrmSelected() || IsObjSelected();
    if ( IsSelection() )
    {
        if ( bFrmSelected )
            UnSelectFrm();

        // Funktionspointer fuer das Aufheben der Selektion setzen
        // bei Cursor setzen
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
        bCreateXSelection = true;
    }
    else if ( bFrmSelected )
    {
        EnterSelFrmMode();
        bCreateXSelection = true;
    }
    else if( (CNT_GRF | CNT_OLE ) & GetCntType() )
    {
        SelectObj( GetCharRect().Pos() );
        EnterSelFrmMode();
        bCreateXSelection = true;
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
    sal_uInt16 nResStr = STR_UNDO;
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

    ::rtl::OUStringBuffer buf = ::rtl::OUStringBuffer( String( SvtResId( nResStr ) ) );
    buf.append(aUndoStr);

    return buf.makeStringAndClear();
}

sal_uInt16 SwWrtShell::GetDoStrings( DoType eDoType, SfxStringListItem& rStrs ) const
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
        OSL_ENSURE(!comments[i].isEmpty(), "no Undo/Redo Text set");
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

    if (str.isEmpty())
    {
        return str;
    }

    ::rtl::OUStringBuffer buf( String(SvtResId(STR_REPEAT)) );
    buf.append(str);
    return buf.makeStringAndClear();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
