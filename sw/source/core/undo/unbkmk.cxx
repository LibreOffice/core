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

#include <UndoBookmark.hxx>

#include "doc.hxx"
#include "docary.hxx"
#include "swundo.hxx"
#include "pam.hxx"

#include <UndoCore.hxx>
#include "IMark.hxx"
#include "rolbck.hxx"

#include "SwRewriter.hxx"

SwUndoBookmark::SwUndoBookmark( SwUndoId nUndoId,
            const ::sw::mark::IMark& rBkmk )
    : SwUndo( nUndoId )
    , m_pHistoryBookmark(new SwHistoryBookmark(rBkmk, true, rBkmk.IsExpanded()))
{
}

SwUndoBookmark::~SwUndoBookmark()
{
}

void SwUndoBookmark::SetInDoc( SwDoc* pDoc )
{
    m_pHistoryBookmark->SetInDoc( pDoc, false );
}

void SwUndoBookmark::ResetInDoc( SwDoc* pDoc )
{
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    for ( IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->getAllMarksBegin();
          ppBkmk != pMarkAccess->getAllMarksEnd();
          ++ppBkmk )
    {
        if ( m_pHistoryBookmark->IsEqualBookmark( **ppBkmk ) )
        {
            pMarkAccess->deleteMark( ppBkmk );
            break;
        }
    }
}

SwRewriter SwUndoBookmark::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UndoArg1, m_pHistoryBookmark->GetName());

    return aResult;
}

SwUndoInsBookmark::SwUndoInsBookmark( const ::sw::mark::IMark& rBkmk )
    : SwUndoBookmark( UNDO_INSBOOKMARK, rBkmk )
{
}

void SwUndoInsBookmark::UndoImpl(::sw::UndoRedoContext & rContext)
{
    ResetInDoc( &rContext.GetDoc() );
}

void SwUndoInsBookmark::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SetInDoc( &rContext.GetDoc() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
