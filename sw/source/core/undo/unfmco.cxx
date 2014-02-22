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

#include "doc.hxx"
#include "swundo.hxx"
#include "pam.hxx"
#include "ndtxt.hxx"
#include <UndoCore.hxx>
#include "rolbck.hxx"
#include "docary.hxx"



SwUndoFmtColl::SwUndoFmtColl( const SwPaM& rRange,
                              SwFmtColl* pColl,
                              const bool bReset,
                              const bool bResetListAttrs )
    : SwUndo( UNDO_SETFMTCOLL ),
      SwUndRng( rRange ),
      pHistory( new SwHistory ),
      pFmtColl( pColl ),
      mbReset( bReset ),
      mbResetListAttrs( bResetListAttrs )
{
    // #i31191#
    if ( pColl )
        aFmtName = pColl->GetName();
}

SwUndoFmtColl::~SwUndoFmtColl()
{
    delete pHistory;
}

void SwUndoFmtColl::UndoImpl(::sw::UndoRedoContext & rContext)
{
    // restore old values
    pHistory->TmpRollback(& rContext.GetDoc(), 0);
    pHistory->SetTmpEnd( pHistory->Count() );

    // create cursor for undo range
    AddUndoRedoPaM(rContext);
}

void SwUndoFmtColl::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam = AddUndoRedoPaM(rContext);

    DoSetFmtColl(rContext.GetDoc(), rPam);
}

void SwUndoFmtColl::RepeatImpl(::sw::RepeatContext & rContext)
{
    DoSetFmtColl(rContext.GetDoc(), rContext.GetRepeatPaM());
}

void SwUndoFmtColl::DoSetFmtColl(SwDoc & rDoc, SwPaM & rPaM)
{
    // Only one TextFrmColl can be applied to a section, thus request only in
    // this array.
    sal_uInt16 const nPos = rDoc.GetTxtFmtColls()->GetPos(
                                                     (SwTxtFmtColl*)pFmtColl );
    // does the format still exist?
    if( USHRT_MAX != nPos )
    {
        rDoc.SetTxtFmtColl(rPaM,
                                          (SwTxtFmtColl*)pFmtColl,
                                          mbReset,
                                          mbResetListAttrs );
    }
}

SwRewriter SwUndoFmtColl::GetRewriter() const
{
    SwRewriter aResult;

    // #i31191# Use stored format name instead of
    // pFmtColl->GetName(), because pFmtColl does not have to be available
    // anymore.
    aResult.AddRule(UndoArg1, aFmtName );

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
