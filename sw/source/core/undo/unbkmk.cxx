/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

