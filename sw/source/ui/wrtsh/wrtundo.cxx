/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/resid.hxx>
#include <sfx2/app.hxx>
#include <svl/slstitm.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swdtflvr.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <wrtsh.hrc>
#include <sfx2/sfx.hrc>




void SwWrtShell::Do( DoType eDoType, sal_uInt16 nCnt )
{
    
    sal_Bool bSaveDoesUndo = DoesUndo();

    StartAllAction();
    switch (eDoType)
    {
        case UNDO:
            DoUndo(sal_False); 
            
            EnterStdMode();
            SwEditShell::Undo(nCnt);
            break;
        case REDO:
            DoUndo(sal_False); 
            
            EnterStdMode();
            SwEditShell::Redo( nCnt );
            break;
        case REPEAT:
            
            SwEditShell::Repeat( nCnt );
            break;
    }
    EndAllAction();
    
    DoUndo(bSaveDoesUndo);

    bool bCreateXSelection = false;
    const bool bFrmSelected = IsFrmSelected() || IsObjSelected();
    if ( IsSelection() )
    {
        if ( bFrmSelected )
            UnSelectFrm();

        
        
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

    
    
    CallChgLnk();
}

OUString SwWrtShell::GetDoString( DoType eDoType ) const
{
    OUString aUndoStr;
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
    default:;
    }

    return  SvtResId( nResStr ).toString() + aUndoStr;
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
    default:;
    }

    OUString buf;
    for (size_t i = 0; i < comments.size(); ++i)
    {
        OSL_ENSURE(!comments[i].isEmpty(), "no Undo/Redo Text set");
        buf += comments[i] + "\n";
    }
    rStrs.SetString(buf);
    return static_cast<sal_uInt16>(comments.size());
}


OUString SwWrtShell::GetRepeatString() const
{
    OUString str;
    GetRepeatInfo(& str);

    if (str.isEmpty())
    {
        return str;
    }

    return SvtResId(STR_REPEAT).toString() + str;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
