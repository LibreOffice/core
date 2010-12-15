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

#include <UndoBookmark.hxx>

#include "doc.hxx"
#include "docary.hxx"
#include "swundo.hxx"           // fuer die UndoIds
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
    for (IDocumentMarkAccess::const_iterator_t ppBkmk =
                pMarkAccess->getMarksBegin();
            ppBkmk != pMarkAccess->getMarksEnd();
            ++ppBkmk)
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

    aResult.AddRule(UNDO_ARG1, m_pHistoryBookmark->GetName());

    return aResult;
}

//----------------------------------------------------------------------


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

