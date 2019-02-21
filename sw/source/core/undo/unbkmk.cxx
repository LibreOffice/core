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

#include <strings.hrc>
#include <doc.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <pam.hxx>

#include <UndoCore.hxx>
#include <IMark.hxx>
#include <rolbck.hxx>

#include <SwRewriter.hxx>

SwUndoBookmark::SwUndoBookmark( SwUndoId nUndoId,
            const ::sw::mark::IMark& rBkmk )
    : SwUndo( nUndoId, rBkmk.GetMarkPos().GetDoc() )
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
    : SwUndoBookmark( SwUndoId::INSBOOKMARK, rBkmk )
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

SwUndoDeleteBookmark::SwUndoDeleteBookmark( const ::sw::mark::IMark& rBkmk )
    : SwUndoBookmark( SwUndoId::DELBOOKMARK, rBkmk )
{
}

void SwUndoDeleteBookmark::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SetInDoc( &rContext.GetDoc() );
}

void SwUndoDeleteBookmark::RedoImpl(::sw::UndoRedoContext & rContext)
{
    ResetInDoc( &rContext.GetDoc() );
}

SwUndoRenameBookmark::SwUndoRenameBookmark( const OUString& rOldName, const OUString& rNewName, const SwDoc* pDoc )
    : SwUndo( SwUndoId::BOOKMARK_RENAME, pDoc )
    , m_sOldName( rOldName )
    , m_sNewName( rNewName )
{
}

SwUndoRenameBookmark::~SwUndoRenameBookmark()
{
}

static OUString lcl_QuoteName(const OUString& rName)
{
    static const OUString sStart = SwResId(STR_START_QUOTE);
    static const OUString sEnd = SwResId(STR_END_QUOTE);
    return sStart + rName + sEnd;
}

SwRewriter SwUndoRenameBookmark::GetRewriter() const
{
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, lcl_QuoteName(m_sOldName));
    aRewriter.AddRule(UndoArg2, SwResId(STR_YIELDS));
    aRewriter.AddRule(UndoArg3, lcl_QuoteName(m_sNewName));
    return aRewriter;
}

void SwUndoRenameBookmark::Rename(::sw::UndoRedoContext const & rContext, const OUString& sFrom, const OUString& sTo)
{
    IDocumentMarkAccess* const pMarkAccess = rContext.GetDoc().getIDocumentMarkAccess();
    IDocumentMarkAccess::const_iterator_t ppBkmk = pMarkAccess->findMark(sFrom);
    if (ppBkmk != pMarkAccess->getAllMarksEnd())
    {
        pMarkAccess->renameMark( ppBkmk->get(), sTo );
    }
}

void SwUndoRenameBookmark::UndoImpl(::sw::UndoRedoContext & rContext)
{
    Rename(rContext, m_sNewName, m_sOldName);
}

void SwUndoRenameBookmark::RedoImpl(::sw::UndoRedoContext & rContext)
{
    Rename(rContext, m_sOldName, m_sNewName);
}

SwUndoInsNoTextFieldmark::SwUndoInsNoTextFieldmark(const ::sw::mark::IFieldmark& rFieldmark)
    : SwUndo(SwUndoId::INSERT, rFieldmark.GetMarkPos().GetDoc())
    , m_pHistoryNoTextFieldmark(new SwHistoryNoTextFieldmark(rFieldmark))
{
}

void SwUndoInsNoTextFieldmark::UndoImpl(::sw::UndoRedoContext & rContext)
{
    m_pHistoryNoTextFieldmark->ResetInDoc(&rContext.GetDoc());
}

void SwUndoInsNoTextFieldmark::RedoImpl(::sw::UndoRedoContext & rContext)
{
    m_pHistoryNoTextFieldmark->SetInDoc(&rContext.GetDoc(), false);
}

SwUndoInsTextFieldmark::SwUndoInsTextFieldmark(const ::sw::mark::IFieldmark& rFieldmark)
    : SwUndo(SwUndoId::INSERT, rFieldmark.GetMarkPos().GetDoc())
    , m_pHistoryTextFieldmark(new SwHistoryTextFieldmark(rFieldmark))
{
}

void SwUndoInsTextFieldmark::UndoImpl(::sw::UndoRedoContext & rContext)
{
    m_pHistoryTextFieldmark->ResetInDoc(&rContext.GetDoc());
}

void SwUndoInsTextFieldmark::RedoImpl(::sw::UndoRedoContext & rContext)
{
    m_pHistoryTextFieldmark->SetInDoc(&rContext.GetDoc(), false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
