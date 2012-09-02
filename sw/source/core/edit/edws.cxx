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

/********************************************************
 * Ctor/Dtor
 ********************************************************/
// verkleideter Copy-Constructor


SwEditShell::SwEditShell( SwEditShell& rEdSH, Window *pWindow )
    : SwCrsrShell( rEdSH, pWindow )
{
}

// ctor/dtor


SwEditShell::SwEditShell( SwDoc& rDoc, Window *pWindow, const SwViewOption *pOptions )
    : SwCrsrShell( rDoc, pWindow, pOptions )
{
    GetDoc()->GetIDocumentUndoRedo().DoUndo(true);
}


SwEditShell::~SwEditShell() // USED
{
}

/******************************************************************************
 *                  sal_Bool SwEditShell::IsModified() const
 ******************************************************************************/


sal_Bool SwEditShell::IsModified() const
{
    return GetDoc()->IsModified();
}
/******************************************************************************
 *                    void SwEditShell::SetModified()
 ******************************************************************************/


void SwEditShell::SetModified()
{
    GetDoc()->SetModified();
}
/******************************************************************************
 *                   void SwEditShell::ResetModified()
 ******************************************************************************/


void SwEditShell::ResetModified()
{
    GetDoc()->ResetModified();
}

void SwEditShell::SetUndoNoResetModified()
{
    GetDoc()->SetModified();
    GetDoc()->GetIDocumentUndoRedo().SetUndoNoResetModified();
}

/******************************************************************************
 *                 void SwEditShell::StartAllAction()
 ******************************************************************************/


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
/******************************************************************************
 *                  void SwEditShell::EndAllAction()
 ******************************************************************************/


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

/******************************************************************************
 *                  void SwEditShell::CalcLayout()
 ******************************************************************************/


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

/******************************************************************************
 *                      Inhaltsform bestimmen, holen
 ******************************************************************************/
// OPT: wird fuer jedes Attribut gerufen?


sal_uInt16 SwEditShell::GetCntType() const
{
    // nur noch am SPoint ist der Inhalt interessant
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

//------------------------------------------------------------------------------


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

/******************************************************************************
 *              Zugriffsfunktionen fuer Filename-Behandlung
 ******************************************************************************/


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


SwFrmFmt *SwEditShell::GetTableFmt()    // OPT: schnellster Test auf Tabelle?
{
    const SwTableNode* pTblNd = IsCrsrInTbl();
    return pTblNd ? (SwFrmFmt*)pTblNd->GetTable().GetFrmFmt() : 0;
}

// OPT: wieso 3x beim neuen Dokument


sal_uInt16 SwEditShell::GetTOXTypeCount(TOXTypes eTyp) const
{
    return pDoc->GetTOXTypeCount(eTyp);
}


void SwEditShell::InsertTOXType(const SwTOXType& rTyp)
{
    pDoc->InsertTOXType(rTyp);
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

// Zusammenfassen von Kontinuierlichen Insert/Delete/Overwrite von
// Charaktern. Default ist sdbcx::Group-Undo.

// setzt Undoklammerung auf, liefert nUndoId der Klammerung


SwUndoId SwEditShell::StartUndo( SwUndoId eUndoId,
                                   const SwRewriter *pRewriter )
{ return GetDoc()->GetIDocumentUndoRedo().StartUndo( eUndoId, pRewriter ); }

// schliesst Klammerung der nUndoId, nicht vom UI benutzt


SwUndoId SwEditShell::EndUndo(SwUndoId eUndoId,
                                const SwRewriter *pRewriter)
{ return GetDoc()->GetIDocumentUndoRedo().EndUndo(eUndoId, pRewriter); }


bool     SwEditShell::GetLastUndoInfo(::rtl::OUString *const o_pStr,
                                      SwUndoId *const o_pId) const
{ return GetDoc()->GetIDocumentUndoRedo().GetLastUndoInfo(o_pStr, o_pId); }

bool     SwEditShell::GetFirstRedoInfo(::rtl::OUString *const o_pStr) const
{ return GetDoc()->GetIDocumentUndoRedo().GetFirstRedoInfo(o_pStr); }

SwUndoId SwEditShell::GetRepeatInfo(::rtl::OUString *const o_pStr) const
{ return GetDoc()->GetIDocumentUndoRedo().GetRepeatInfo(o_pStr); }



// AutoKorrektur - JP 27.01.94
void SwEditShell::AutoCorrect( SvxAutoCorrect& rACorr, sal_Bool bInsert,
                                sal_Unicode cChar )
{
    SET_CURR_SHELL( this );

    StartAllAction();

    SwPaM* pCrsr = getShellCrsr( true );
    SwTxtNode* pTNd = pCrsr->GetNode()->GetTxtNode();

    SwAutoCorrDoc aSwAutoCorrDoc( *this, *pCrsr, cChar );
    rACorr.AutoCorrect( aSwAutoCorrDoc,
                    pTNd->GetTxt(), pCrsr->GetPoint()->nContent.GetIndex(),
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
