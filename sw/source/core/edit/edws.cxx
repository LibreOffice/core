/*************************************************************************
 *
 *  $RCSfile: edws.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-25 12:01:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop


#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _ACORRECT_HXX
#include <acorrect.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif

/********************************************************
 * Ctor/Dtor
 ********************************************************/
// verkleideter Copy-Constructor


SwEditShell::SwEditShell( SwEditShell& rEdSH, Window *pWin )
    : SwCrsrShell( rEdSH, pWin )
{
}

// ctor/dtor


SwEditShell::SwEditShell( SwDoc& rDoc, Window *pWin, SwRootFrm *pRootFrm,
                            const SwViewOption *pOpt )
    : SwCrsrShell( rDoc, pWin, pRootFrm, pOpt)
{
    GetDoc()->DoUndo();
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
    GetDoc()->SetUndoNoResetModified();
}

#ifdef USED
/******************************************************************************
 *                    void SwEditShell::StartAction()
 ******************************************************************************/


void SwEditShell::StartAction() // OPT: ganz wech
{
    SwCrsrShell::StartAction();
}
/******************************************************************************
 *                    void SwEditShell::EndAction()
 ******************************************************************************/


void SwEditShell::EndAction()
{
    SwCrsrShell::EndAction();
}
#endif
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

    ASSERT( nRet, ERR_OUTOFSCOPE );
    return nRet;
}

//------------------------------------------------------------------------------


sal_Bool SwEditShell::HasOtherCnt() const
{
    const SwNodes &rNds = GetDoc()->GetNodes();
    const SwNode *pNd;
    return GetDoc()->GetSpzFrmFmts()->Count() ||
            1 != (( pNd = &rNds.GetEndOfInserts() )->GetIndex() -
                pNd->StartOfSectionIndex() ) ||
            1 != (( pNd = &rNds.GetEndOfAutotext() )->GetIndex() -
                pNd->StartOfSectionIndex() );
}

/******************************************************************************
 *              Zugriffsfunktionen fuer Filename-Behandlung
 ******************************************************************************/


SwActKontext::SwActKontext(SwEditShell *pShell)
    : pSh(pShell)
{
    pSh->StartAction();
}


SwActKontext::~SwActKontext()
{
    pSh->EndAction();
}

/******************************************************************************
 *          Klasse fuer den automatisierten Aufruf von Start- und
 *                              EndCrsrMove();
 ******************************************************************************/


SwMvKontext::SwMvKontext(SwEditShell *pShell ) : pSh(pShell)
{
    pSh->SttCrsrMove();
}


SwMvKontext::~SwMvKontext()
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
{ GetDoc()->DoUndo( bOn ); }


sal_Bool SwEditShell::DoesUndo() const
{ return GetDoc()->DoesUndo(); }


void SwEditShell::DoGroupUndo( sal_Bool bOn )
{ GetDoc()->DoGroupUndo( bOn ); }


sal_Bool SwEditShell::DoesGroupUndo() const
{ return GetDoc()->DoesGroupUndo(); }


void SwEditShell::DelAllUndoObj()
{
    GetDoc()->DelAllUndoObj();
}

// Zusammenfassen von Kontinuierlichen Insert/Delete/Overwrite von
// Charaktern. Default ist sdbcx::Group-Undo.

// setzt Undoklammerung auf, liefert nUndoId der Klammerung


sal_uInt16 SwEditShell::StartUndo( sal_uInt16 nUndoId )
{ return GetDoc()->StartUndo( nUndoId ); }

// schliesst Klammerung der nUndoId, nicht vom UI benutzt


sal_uInt16 SwEditShell::EndUndo(sal_uInt16 nUndoId)
{ return GetDoc()->EndUndo(nUndoId); }

// liefert die Id der letzten undofaehigen Aktion zurueck
// fuellt ggf. VARARR mit sdbcx::User-UndoIds


sal_uInt16 SwEditShell::GetUndoIds(String* pStr,SwUndoIds *pUndoIds) const
{ return GetDoc()->GetUndoIds(pStr,pUndoIds); }

// liefert die Id der letzten Redofaehigen Aktion zurueck
// fuellt ggf. VARARR mit RedoIds


sal_uInt16 SwEditShell::GetRedoIds(String* pStr,SwUndoIds *pRedoIds) const
{ return GetDoc()->GetRedoIds(pStr,pRedoIds); }

// liefert die Id der letzten Repeatfaehigen Aktion zurueck
// fuellt ggf. VARARR mit RedoIds


sal_uInt16 SwEditShell::GetRepeatIds(String* pStr, SwUndoIds *pRedoIds) const
{ return GetDoc()->GetRepeatIds(pStr,pRedoIds); }



// AutoKorrektur - JP 27.01.94
void SwEditShell::AutoCorrect( SvxAutoCorrect& rACorr, sal_Bool bInsert,
                                sal_Unicode cChar )
{
    SET_CURR_SHELL( this );

    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    SwTxtNode* pTNd = pCrsr->GetNode()->GetTxtNode();

    SwAutoCorrDoc aSwAutoCorrDoc( *this, *pCrsr, cChar );
    rACorr.AutoCorrect( aSwAutoCorrDoc,
                    pTNd->GetTxt(), pCrsr->GetPoint()->nContent.GetIndex(),
                    cChar, bInsert );
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
    SwPaM* pCrsr = GetCrsr();
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



