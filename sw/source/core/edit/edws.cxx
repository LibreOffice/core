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

#include <vcl/window.hxx>

#include <editsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <docary.hxx>
#include <acorrect.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <SwRewriter.hxx>

// masqueraded copy constructor
SwEditShell::SwEditShell( SwEditShell& rEdSH, Window *pWindow )
    : SwCrsrShell( rEdSH, pWindow )
{
}

SwEditShell::SwEditShell( SwDoc& rDoc, Window *pWindow, const SwViewOption *pOptions )
    : SwCrsrShell( rDoc, pWindow, pOptions )
{
    GetDoc()->GetIDocumentUndoRedo().DoUndo(true);
}

SwEditShell::~SwEditShell() // USED
{
}

sal_Bool SwEditShell::IsModified() const
{
    return GetDoc()->IsModified();
}

void SwEditShell::SetModified()
{
    GetDoc()->SetModified();
}

void SwEditShell::ResetModified()
{
    GetDoc()->ResetModified();
}

void SwEditShell::SetUndoNoResetModified()
{
    GetDoc()->SetModified();
    GetDoc()->GetIDocumentUndoRedo().SetUndoNoResetModified();
}

void SwEditShell::StartAllAction()
{
    ViewShell *pSh = this;
    do {
        if( pSh->IsA( TYPE( SwEditShell ) ) )
            ((SwEditShell*)pSh)->StartAction();
        else
            pSh->StartAction();
        pSh = (ViewShell *)pSh->GetNext();
    } while(pSh != this);
}

void SwEditShell::EndAllAction()
{
    ViewShell *pSh = this;
    do {
        if( pSh->IsA( TYPE( SwEditShell ) ) )
            ((SwEditShell*)pSh)->EndAction();
        else
            pSh->EndAction();
        pSh = (ViewShell *)pSh->GetNext();
    } while(pSh != this);
}

void SwEditShell::CalcLayout()
{
    StartAllAction();
    ViewShell::CalcLayout();

    ViewShell *pSh = this;
    do
    {
        if ( pSh->GetWin() )
            pSh->GetWin()->Invalidate();
        pSh = (ViewShell*)pSh->GetNext();

    } while ( pSh != this );

    EndAllAction();
}

/** Get the content type of a shell
 *
 * @todo Is this called for every attribute?
 */
sal_uInt16 SwEditShell::GetCntType() const
{
    sal_uInt16 nRet = 0;
    if( IsTableMode() )
        nRet = CNT_TXT;
    else
        switch( GetCrsr()->GetNode()->GetNodeType() )
        {
        case ND_TEXTNODE:   nRet = CNT_TXT; break;
        case ND_GRFNODE:    nRet = CNT_GRF; break;
        case ND_OLENODE:    nRet = CNT_OLE; break;
        }

    OSL_ASSERT( nRet );
    return nRet;
}

sal_Bool SwEditShell::HasOtherCnt() const

{
    if ( !GetDoc()->GetSpzFrmFmts()->empty() )
        return sal_True;

    const SwNodes &rNds = GetDoc()->GetNodes();
    const SwNode *pNd;

    pNd = &rNds.GetEndOfInserts();
    if ( 1 != (pNd->GetIndex() - pNd->StartOfSectionIndex()) )
        return sal_True;

    pNd = &rNds.GetEndOfAutotext();
    if ( 1 != (pNd->GetIndex() - pNd->StartOfSectionIndex()) )
        return sal_True;

    return sal_False;
}

// access controll functions for file name handling

SwActContext::SwActContext(SwEditShell *pShell)
    : pSh(pShell)
{
    pSh->StartAction();
}

SwActContext::~SwActContext()
{
    pSh->EndAction();
}

SwMvContext::SwMvContext(SwEditShell *pShell)
    : pSh(pShell)
{
    pSh->SttCrsrMove();
}

SwMvContext::~SwMvContext()
{
    pSh->EndCrsrMove();
}

SwFrmFmt *SwEditShell::GetTableFmt() // fastest test on a table
{
    const SwTableNode* pTblNd = IsCrsrInTbl();
    return pTblNd ? (SwFrmFmt*)pTblNd->GetTable().GetFrmFmt() : 0;
}

// TODO: Why is this called 3x for a new document?
sal_uInt16 SwEditShell::GetTOXTypeCount(TOXTypes eTyp) const
{
    return mpDoc->GetTOXTypeCount(eTyp);
}

void SwEditShell::InsertTOXType(const SwTOXType& rTyp)
{
    mpDoc->InsertTOXType(rTyp);
}

void SwEditShell::DoUndo( sal_Bool bOn )
{ GetDoc()->GetIDocumentUndoRedo().DoUndo( bOn ); }


sal_Bool SwEditShell::DoesUndo() const
{ return GetDoc()->GetIDocumentUndoRedo().DoesUndo(); }


void SwEditShell::DoGroupUndo( sal_Bool bOn )
{ GetDoc()->GetIDocumentUndoRedo().DoGroupUndo( bOn ); }


sal_Bool SwEditShell::DoesGroupUndo() const
{ return GetDoc()->GetIDocumentUndoRedo().DoesGroupUndo(); }


void SwEditShell::DelAllUndoObj()
{
    GetDoc()->GetIDocumentUndoRedo().DelAllUndoObj();
}

// Combine continuous calls of Insert/Delete/Overwrite on characters. Default: sdbcx::Group-Undo.

/** open undo container
 *
 * @return nUndoId ID of the container
 */
SwUndoId SwEditShell::StartUndo( SwUndoId eUndoId,
                                   const SwRewriter *pRewriter )
{ return GetDoc()->GetIDocumentUndoRedo().StartUndo( eUndoId, pRewriter ); }

/** close undo container
 *
 * not used by UI
 *
 * @param eUndoId   ID of the undo container
 * @param pRewriter ?
*/
SwUndoId SwEditShell::EndUndo(SwUndoId eUndoId, const SwRewriter *pRewriter)
{ return GetDoc()->GetIDocumentUndoRedo().EndUndo(eUndoId, pRewriter); }

bool     SwEditShell::GetLastUndoInfo(OUString *const o_pStr,
                                      SwUndoId *const o_pId) const
{ return GetDoc()->GetIDocumentUndoRedo().GetLastUndoInfo(o_pStr, o_pId); }

bool     SwEditShell::GetFirstRedoInfo(OUString *const o_pStr) const
{ return GetDoc()->GetIDocumentUndoRedo().GetFirstRedoInfo(o_pStr); }

SwUndoId SwEditShell::GetRepeatInfo(OUString *const o_pStr) const
{ return GetDoc()->GetIDocumentUndoRedo().GetRepeatInfo(o_pStr); }

/** Auto correction */
void SwEditShell::AutoCorrect( SvxAutoCorrect& rACorr, sal_Bool bInsert,
                                sal_Unicode cChar )
{
    SET_CURR_SHELL( this );

    StartAllAction();

    SwPaM* pCrsr = getShellCrsr( true );
    SwTxtNode* pTNd = pCrsr->GetNode()->GetTxtNode();

    SwAutoCorrDoc aSwAutoCorrDoc( *this, *pCrsr, cChar );
    // FIXME: this _must_ be called with reference to the actual node text!
    OUString const& rNodeText(pTNd->GetTxt());
    rACorr.DoAutoCorrect( aSwAutoCorrDoc,
                    rNodeText, pCrsr->GetPoint()->nContent.GetIndex(),
                    cChar, bInsert, GetWin() );
    if( cChar )
        SaveTblBoxCntnt( pCrsr->GetPoint() );
    EndAllAction();
}

void SwEditShell::SetNewDoc(sal_Bool bNew)
{
    GetDoc()->SetNewDoc(bNew);
}

sal_Bool SwEditShell::GetPrevAutoCorrWord( SvxAutoCorrect& rACorr, String& rWord )
{
    SET_CURR_SHELL( this );

    sal_Bool bRet;
    SwPaM* pCrsr = getShellCrsr( true );
    xub_StrLen nPos = pCrsr->GetPoint()->nContent.GetIndex();
    SwTxtNode* pTNd = pCrsr->GetNode()->GetTxtNode();
    if( pTNd && nPos )
    {
        SwAutoCorrDoc aSwAutoCorrDoc( *this, *pCrsr, 0 );
        bRet = rACorr.GetPrevAutoCorrWord( aSwAutoCorrDoc,
                                            pTNd->GetTxt(), nPos, rWord );
    }
    else
        bRet = sal_False;
    return bRet;
}

SwAutoCompleteWord& SwEditShell::GetAutoCompleteWords()
{
    return SwDoc::GetAutoCompleteWords();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
