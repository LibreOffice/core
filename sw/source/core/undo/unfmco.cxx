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

SwUndoFormatColl::SwUndoFormatColl( const SwPaM& rRange,
                              SwFormatColl* pColl,
                              const bool bReset,
                              const bool bResetListAttrs )
    : SwUndo( UNDO_SETFMTCOLL ),
      SwUndRng( rRange ),
      pHistory( new SwHistory ),
      pFormatColl( pColl ),
      mbReset( bReset ),
      mbResetListAttrs( bResetListAttrs )
{
    // #i31191#
    if ( pColl )
        aFormatName = pColl->GetName();
}

SwUndoFormatColl::~SwUndoFormatColl()
{
    delete pHistory;
}

void SwUndoFormatColl::UndoImpl(::sw::UndoRedoContext & rContext)
{
    // restore old values
    pHistory->TmpRollback(& rContext.GetDoc(), 0);
    pHistory->SetTmpEnd( pHistory->Count() );

    // create cursor for undo range
    AddUndoRedoPaM(rContext);
}

void SwUndoFormatColl::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam = AddUndoRedoPaM(rContext);

    DoSetFormatColl(rContext.GetDoc(), rPam);
}

void SwUndoFormatColl::RepeatImpl(::sw::RepeatContext & rContext)
{
    DoSetFormatColl(rContext.GetDoc(), rContext.GetRepeatPaM());
}

void SwUndoFormatColl::DoSetFormatColl(SwDoc & rDoc, SwPaM & rPaM)
{
    // Only one TextFrameColl can be applied to a section, thus request only in
    // this array.

    // does the format still exist?
    if( rDoc.GetTextFormatColls()->Contains(static_cast<SwTextFormatColl*>(pFormatColl)) )
    {
        rDoc.SetTextFormatColl(rPaM, static_cast<SwTextFormatColl*>(pFormatColl), mbReset,
                           mbResetListAttrs);
    }
}

SwRewriter SwUndoFormatColl::GetRewriter() const
{
    SwRewriter aResult;

    // #i31191# Use stored format name instead of
    // pFormatColl->GetName(), because pFormatColl does not have to be available
    // anymore.
    aResult.AddRule(UndoArg1, aFormatName );

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
