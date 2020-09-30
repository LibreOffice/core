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

#include <doc.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>

SwUndoFormatColl::SwUndoFormatColl( const SwPaM& rRange,
                              const SwFormatColl* pColl,
                              const bool bReset,
                              const bool bResetListAttrs )
    : SwUndo( SwUndoId::SETFMTCOLL, &rRange.GetDoc() ),
      SwUndRng( rRange ),
      mpHistory( new SwHistory ),
      mbReset( bReset ),
      mbResetListAttrs( bResetListAttrs )
{
    // #i31191#
    if ( pColl )
        maFormatName = pColl->GetName();
}

SwUndoFormatColl::~SwUndoFormatColl()
{
}

void SwUndoFormatColl::UndoImpl(::sw::UndoRedoContext & rContext)
{
    // restore old values
    mpHistory->TmpRollback(& rContext.GetDoc(), 0);
    mpHistory->SetTmpEnd( mpHistory->Count() );

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

void SwUndoFormatColl::DoSetFormatColl(SwDoc & rDoc, SwPaM const & rPaM)
{
    // Only one TextFrameColl can be applied to a section, thus request only in
    // this array.
    SwTextFormatColl * pFormatColl = rDoc.FindTextFormatCollByName(maFormatName);
    if (pFormatColl)
    {
        rDoc.SetTextFormatColl(rPaM, pFormatColl, mbReset, mbResetListAttrs);
    }
}

SwRewriter SwUndoFormatColl::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UndoArg1, maFormatName );

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
