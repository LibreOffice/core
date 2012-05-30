/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "doc.hxx"
#include "swundo.hxx"
#include "pam.hxx"
#include "ndtxt.hxx"
#include <UndoCore.hxx>
#include "rolbck.hxx"

//--------------------------------------------------

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
