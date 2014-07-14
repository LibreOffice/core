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



#include "doc.hxx"
#include "swundo.hxx"           // fuer die UndoIds
#include "pam.hxx"
#include "ndtxt.hxx"

#include <UndoCore.hxx>
#include "rolbck.hxx"


//--------------------------------------------------


// --> OD 2008-04-15 #refactorlists#
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
// <--
{
    // --> FME 2004-08-06 #i31191#
    if ( pColl )
        aFmtName = pColl->GetName();
    // <--
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
    // es kann nur eine TextFmtColl auf einen Bereich angewendet werden,
    // also erfrage auch nur in dem Array
    sal_uInt16 const nPos = rDoc.GetTxtFmtColls()->GetPos(
                                                     (SwTxtFmtColl*)pFmtColl );
    // does the format still exist?
    if ( USHRT_MAX != nPos )
    {
        rDoc.SetTxtFmtColl(
            rPaM,
            (SwTxtFmtColl*) pFmtColl,
            mbReset,
            mbResetListAttrs );
    }
}

SwRewriter SwUndoFmtColl::GetRewriter() const
{
    SwRewriter aResult;

    // --> FME 2004-08-06 #i31191# Use stored format name instead of
    // pFmtColl->GetName(), because pFmtColl does not have to be available
    // anymore.
    aResult.AddRule(UNDO_ARG1, aFmtName );
    // <--

    return aResult;
}

