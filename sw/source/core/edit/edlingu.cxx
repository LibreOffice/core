/*************************************************************************
 *
 *  $RCSfile: edlingu.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tl $ $Date: 2000-10-27 11:55:57 $
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

#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _OFF_APP_HXX //autogen wg. OFF_APP
#include <offmgr/app.hxx>
#endif
#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif

#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>      // SwRootFrm
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>       // fuer die UndoIds
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // AdjHyphPos
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>      // HyphStart/End
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>       // SwShellCrsr
#endif
#ifndef _WRONG_HXX
#include <wrong.hxx>        // SwWrongList
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>       // SwCursor
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>       // Statusanzeige
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>      // StatLine-String
#endif

#ifdef LINGU_STATISTIK
#include <txtfrm.hxx>       // SwLinguStat.Flush()
#endif

#ifndef S2U
#define S2U(rString) rtl::OUString::createFromAscii(rString)
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

/*************************************************************************
 *                     class SwLinguIter
 *************************************************************************/

class SwLinguIter
{
    SwEditShell *pSh;
    SwPosition  *pStart;
    SwPosition  *pEnd;
    SwPosition  *pCurr;
    SwPosition  *pCurrX;
    sal_uInt16 nCrsrCnt;
public:
    SwLinguIter();

    inline SwEditShell *GetSh()             { return pSh; }
    inline const SwEditShell *GetSh() const { return pSh; }

    inline const SwPosition *GetEnd() const { return pEnd; }
    inline void SetEnd( SwPosition* pNew ){ delete pEnd; pEnd = pNew; }

    inline const SwPosition *GetStart() const { return pStart; }
    inline void SetStart( SwPosition* pNew ){ delete pStart; pStart = pNew; }

    inline const SwPosition *GetCurr() const { return pCurr; }
    inline void SetCurr( SwPosition* pNew ){ delete pCurr; pCurr = pNew; }

    inline const SwPosition *GetCurrX() const { return pCurrX; }
    inline void SetCurrX( SwPosition* pNew ){ delete pCurrX; pCurrX = pNew; }

    inline sal_uInt16& GetCrsrCnt(){ return nCrsrCnt; }

    // Der UI-Bauchladen:
    void _Start( SwEditShell *pSh, SwDocPositions eStart,
                SwDocPositions eEnd, sal_Bool bRev = sal_False );
    void _End();
};

/*************************************************************************
 *                     class SwSpellIter
 *************************************************************************/

class SwSpellIter : public SwLinguIter
{
    uno::Reference< XSpellChecker1 >    xSpeller;
public:
    SwSpellIter() {}

    void Start( SwEditShell *pSh, SwDocPositions eStart, SwDocPositions eEnd );

    uno::Reference< uno::XInterface >
        Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );
};

/*************************************************************************
 *                     class SwHyphIter
 *************************************************************************/

class SwHyphIter : public SwLinguIter
{
    sal_Bool bOldIdle;
    void DelSoftHyph( SwPaM &rPam );

public:
    SwHyphIter() : bOldIdle(sal_False) {}

    void Start( SwEditShell *pSh, SwDocPositions eStart, SwDocPositions eEnd );
    void End();

    void Ignore();

    uno::Reference< uno::XInterface >
        Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt );

    sal_Bool IsAuto();
    void InsertSoftHyph( const xub_StrLen nHyphPos );
    void ShowSelection();
};

static SwSpellIter* pSpellIter = 0;
static SwHyphIter*  pHyphIter = 0;

// Wir ersparen uns in Hyphenate ein GetFrm()
// Achtung: in txtedt.cxx stehen extern-Deklarationen auf diese Pointer!
const SwTxtNode *pLinguNode;
      SwTxtFrm  *pLinguFrm;

/*************************************************************************
 *                      SwLinguIter::SwLinguIter
 *************************************************************************/

SwLinguIter::SwLinguIter()
    : pSh( 0 ), pStart( 0 ), pEnd( 0 ), pCurr( 0 ), pCurrX( 0 )
{
    // @@@ es fehlt: Sicherstellen der Reentrance, ASSERTs etc.
}

/*************************************************************************
 *                      SwLinguIter::Start
 *************************************************************************/



void SwLinguIter::_Start( SwEditShell *pShell, SwDocPositions eStart,
                            SwDocPositions eEnd, sal_Bool bRev )
{
    // es fehlt: Sicherstellen der Reentrance, Locking
    if( pSh )
        return;

    sal_Bool bSetCurr;

    pSh = pShell;

    SET_CURR_SHELL( pSh );

    ASSERT( !pEnd, "LinguStart ohne End?");

    SwPaM *pCrsr = pSh->GetCrsr();

    // pStk->SetCurCrsr();
//  if( pCrsr->HasMark() || pCrsr != pCrsr->GetNext() )
    if( pShell->HasSelection() || pCrsr != pCrsr->GetNext() )
    {
        bSetCurr = 0 != GetCurr();
        nCrsrCnt = pSh->GetCrsrCnt();
        if( pSh->IsTableMode() )
            pSh->TblCrsrToCursor();

        pSh->Push();
        sal_uInt16 n;
        for( n = 0; n < nCrsrCnt; ++n )
        {
            pSh->Push();
            pSh->DestroyCrsr();
        }
        pSh->Pop( sal_False );
    }
    else
    {
        bSetCurr = sal_False;
        nCrsrCnt = 1;
        pSh->Push();
        pSh->SetLinguRange( eStart, eEnd );
    }

    pCrsr = pSh->GetCrsr();
    if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
        pCrsr->Exchange();

    pStart = new SwPosition( *pCrsr->GetPoint() );
    pEnd = new SwPosition( *pCrsr->GetMark() );
    if( bSetCurr )
    {
        SwPosition* pNew = new SwPosition( bRev ? *GetEnd() : *GetStart() );
        SetCurr( pNew );
        pNew = new SwPosition( *pNew );
        SetCurrX( pNew );
    }

    pCrsr->SetMark();

    pLinguFrm = 0;
    pLinguNode = 0;
}

/*************************************************************************
 *                      SwLinguIter::End
 *************************************************************************/



void SwLinguIter::_End()
{
    if( !pSh )
        return;

    ASSERT( pEnd, "SwEditShell::SpellEnd() ohne Start?");
    while( nCrsrCnt-- )
        pSh->Pop( sal_False );

    pSh->KillPams();
    pSh->ClearMark();
    DELETEZ(pStart);
    DELETEZ(pEnd);
    DELETEZ(pCurr);
    DELETEZ(pCurrX);

    pSh = 0;

#ifdef LINGU_STATISTIK
    aSwLinguStat.Flush();
#endif
}

/*************************************************************************
 *               virtual SwSpellIter::Start()
 *************************************************************************/



void SwSpellIter::Start( SwEditShell *pShell, SwDocPositions eStart,
                        SwDocPositions eEnd )
{
    if( GetSh() )
        return;

     uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    sal_Bool bIsWrapReverse = xProp.is()
                ? *(sal_Bool*)xProp->getPropertyValue(
                                    S2U(UPN_IS_WRAP_REVERSE) ).getValue()
                : sal_False;

    xSpeller = ::GetSpellChecker();
    if ( xSpeller.is() )
        _Start( pShell, eStart, eEnd, bIsWrapReverse );
}

/*************************************************************************
 *                   SwSpellIter::Continue
 *************************************************************************/

// SwSpellIter::Continue ist das alte Original von
// SwEditShell::SpellContinue()

uno::Reference< uno::XInterface >
    SwSpellIter::Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
     uno::Reference< uno::XInterface >  xSpellRet;
    SwEditShell *pSh = GetSh();
    if( !pSh )
        return xSpellRet;

//  const SwPosition *pEnd = GetEnd();

    ASSERT( GetEnd(), "SwEditShell::SpellContinue() ohne Start?");

    sal_Bool bGoOn = sal_True;
    do {
        SwPaM *pCrsr = pSh->GetCrsr();
        if ( !pCrsr->HasMark() )
            pCrsr->SetMark();

        uno::Reference< beans::XPropertySet >  xProp( GetLinguPropertySet() );
        sal_Bool bRev = xProp.is() ?
            *(sal_Bool*)xProp->getPropertyValue( S2U(UPN_IS_WRAP_REVERSE) ).getValue() : sal_False;
        if( bRev )
        {
            *pSh->GetCrsr()->GetPoint() = *GetCurrX();
            *pSh->GetCrsr()->GetMark() = *GetStart();
        }
        else
        {
            *pSh->GetCrsr()->GetPoint() = *GetCurr();
            *pSh->GetCrsr()->GetMark() = *GetEnd();
        }
        xSpellRet = pSh->GetDoc()->Spell(*pSh->GetCrsr(),
                    xSpeller, pPageCnt, pPageSt );
        bGoOn = GetCrsrCnt() > 1;
        if( xSpellRet.is() )
        {
            bGoOn = sal_False;
            SwPosition* pNewPoint = new SwPosition( *pCrsr->GetPoint() );
            SwPosition* pNewMark = new SwPosition( *pCrsr->GetMark() );
            if( bRev )
            {
                SetCurr( pNewMark );
                // Noch steht der sdbcx::Index zwar am Anfang des falschen Wortes,
                // wenn dies ersetzt wird (Delete,Insert), ist der sdbcx::Index
                // hinter diesem und das Wort wird erneut geprueft (51308)
                if( pNewPoint->nContent.GetIndex() )
                    --pNewPoint->nContent;
                SetCurrX( pNewPoint );
            }
            else
            {
                SetCurr( pNewPoint );
                SetCurrX( pNewMark );
            }
        }
        if( bGoOn )
        {
            pSh->Pop( sal_False );
            pCrsr = pSh->GetCrsr();
            if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
                pCrsr->Exchange();
            SwPosition* pNew = new SwPosition( *pCrsr->GetPoint() );
            SetStart( pNew );
            pNew = new SwPosition( *pCrsr->GetMark() );
            SetEnd( pNew );
            pNew = new SwPosition( bRev ? *GetEnd() : *GetStart() );
            SetCurr( pNew );
            pNew = new SwPosition( *pNew );
            SetCurrX( pNew );
            pCrsr->SetMark();
            --GetCrsrCnt();
        }
    }while ( bGoOn );
    return xSpellRet;
}


sal_Bool SwHyphIter::IsAuto()
{
    uno::Reference< beans::XPropertySet >  xProp( ::GetLinguPropertySet() );
    return xProp.is() ? *(sal_Bool*)xProp->getPropertyValue(
                                S2U(UPN_IS_HYPH_AUTO) ).getValue()
                      : sal_False;
}


void SwHyphIter::ShowSelection()
{
    SwEditShell *pSh = GetSh();
    if( pSh )
    {
        pSh->StartAction();
        // Ganz fatal: durch das EndAction() werden Formatierungen
        // angeregt, die dazu fuehren koennen, dass im Hyphenator
        // neue Worte eingestellt werden. Deswegen sichern!
        pSh->EndAction();
    }
}

/*************************************************************************
 *               virtual SwHyphIter::Start()
 *************************************************************************/



void SwHyphIter::Start( SwEditShell *pShell, SwDocPositions eStart, SwDocPositions eEnd )
{
    // robust
    if( GetSh() || GetEnd() )
    {
        ASSERT( !GetSh(), "+SwEditShell::HyphStart: missing HyphEnd()" );
        return;
    }

// nothing to be done (at least not in the way as in the "else" part)
    bOldIdle = pShell->GetViewOptions()->IsIdle();
    ((SwViewOption*)pShell->GetViewOptions())->SetIdle( sal_False );
    _Start( pShell, eStart, eEnd );
}

/*************************************************************************
 *                 virtual SwHyphIter::End
 *************************************************************************/

// Selektionen wiederherstellen



void SwHyphIter::End()
{
    if( !GetSh() )
        return;
    ((SwViewOption*)GetSh()->GetViewOptions())->SetIdle( bOldIdle );
    _End();
}

/*************************************************************************
 *                   SwHyphIter::Continue
 *************************************************************************/

uno::Reference< uno::XInterface >
    SwHyphIter::Continue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    SwEditShell *pSh = GetSh();
    if( !pSh )
        return 0;

    const sal_Bool bAuto = IsAuto();
     uno::Reference< XHyphenatedWord >  xHyphWord;
    sal_uInt16 nRet;
    sal_Bool bGoOn = sal_False;
    do {
        SwPaM *pCrsr;
        do {
            ASSERT( GetEnd(), "SwEditShell::SpellContinue() ohne Start?" );
            pCrsr = pSh->GetCrsr();
            if ( !pCrsr->HasMark() )
                pCrsr->SetMark();
            if ( *pCrsr->GetPoint() < *pCrsr->GetMark() )
            {
                pCrsr->Exchange();
                pCrsr->SetMark();
            }

            // geraten BUG:
            if ( *pCrsr->End() > *GetEnd() )
                nRet = 0;
            else
            {
                *pCrsr->GetMark() = *GetEnd();

                // Muss an der aktuellen Cursorpos das Wort getrennt werden ?
                const Point aCrsrPos( pSh->GetCharRect().Pos() );
                xHyphWord = pSh->GetDoc()->Hyphenate( pCrsr, aCrsrPos,
                                                       pPageCnt, pPageSt );
            }

            if( bAuto && xHyphWord.is() )
            {
                pSh->InsertSoftHyph( xHyphWord->getHyphenationPos() + 1);
            }
        } while( bAuto && xHyphWord.is() ); //end of do-while
        bGoOn = !xHyphWord.is() && GetCrsrCnt() > 1;

        if( bGoOn )
        {
            pSh->Pop( sal_False );
            pCrsr = pSh->GetCrsr();
            if ( *pCrsr->GetPoint() > *pCrsr->GetMark() )
                pCrsr->Exchange();
            SwPosition* pNew = new SwPosition(*pCrsr->End());
            SetEnd( pNew );
            pCrsr->SetMark();
            --GetCrsrCnt();
        }
    } while ( bGoOn );
    return xHyphWord;
}

/*************************************************************************
 *                  SwHyphIter::HyphIgnore
 *************************************************************************/

// Beschreibung: Trennstelle ignorieren

void SwHyphIter::Ignore()
{
    SwEditShell *pSh = GetSh();
    SwPaM *pCrsr = pSh->GetCrsr();

    // Alten SoftHyphen loeschen
    DelSoftHyph( *pCrsr );

    // und weiter
    pCrsr->Start()->nContent = pCrsr->End()->nContent;
    pCrsr->SetMark();
}

/*************************************************************************
 *                        SwHyphIter::DelSoftHyph
 *************************************************************************/

void SwHyphIter::DelSoftHyph( SwPaM &rPam )
{
    const SwPosition* pStt = rPam.Start();
    const xub_StrLen nStart = pStt->nContent.GetIndex();
    const xub_StrLen nEnd   = rPam.End()->nContent.GetIndex();
    SwTxtNode *pNode = pStt->nNode.GetNode().GetTxtNode();
    pNode->DelSoftHyph( nStart, nEnd );
}

/*************************************************************************
 *                  SwHyphIter::InsertSoftHyph
 *************************************************************************/


void SwHyphIter::InsertSoftHyph( const xub_StrLen nHyphPos )
{
    SwEditShell *pSh = GetSh();
    ASSERT( pSh,  "+SwEditShell::InsertSoftHyph: missing HyphStart()");
    if( !pSh )
        return;

    SwPaM *pCrsr = pSh->GetCrsr();
    SwPosition *pStt = pCrsr->Start(), *pEnd = pCrsr->End();

    xub_StrLen nLastHyphLen = GetEnd()->nContent.GetIndex() -
                          pStt->nContent.GetIndex();

    if( pStt->nNode != pEnd->nNode || !nLastHyphLen )
    {
        ASSERT( pStt->nNode == pEnd->nNode,
                "+SwEditShell::InsertSoftHyph: node warp during hyphenation" );
        ASSERT(nLastHyphLen, "+SwEditShell::InsertSoftHyph: missing HyphContinue()");
        *pStt = *pEnd;
        return;
    }

    pSh->StartAction();
    {
        SwDoc *pDoc = pSh->GetDoc();
        DelSoftHyph( *pCrsr );
        pStt->nContent += nHyphPos;
        SwPaM aRg( *pStt );
        pDoc->Insert( aRg, CHAR_SOFTHYPHEN );
        // Durch das Einfuegen des SoftHyphs ist ein Zeichen hinzugekommen
//JP 18.07.95: warum, ist doch ein SwIndex, dieser wird doch mitverschoben !!
//        pStt->nContent++;
    }
    // Die Selektion wird wieder aufgehoben
    pCrsr->DeleteMark();
    pSh->EndAction();
    pCrsr->SetMark();
}

// --------------------- Methoden der SwEditShell ------------------------

/*************************************************************************
 *                      SwEditShell::HasSpellIter
 *************************************************************************/

BOOL SwEditShell::HasSpellIter() const
{
    return 0 != pSpellIter;
}

/*************************************************************************
 *                      SwEditShell::HasHyphIter
 *************************************************************************/

BOOL SwEditShell::HasHyphIter() const
{
    return 0 != pHyphIter;
}

/*************************************************************************
 *                      SwEditShell::SetFindRange
 *************************************************************************/

void SwEditShell::SetLinguRange( SwDocPositions eStart, SwDocPositions eEnd )
{
    SwPaM *pCrsr = GetCrsr();
    MakeFindRange( eStart, eEnd, pCrsr );
    if( *pCrsr->GetPoint() > *pCrsr->GetMark() )
        pCrsr->Exchange();
}

/*************************************************************************
 *                  SwEditShell::SpellStart
 *************************************************************************/

// Selektionen sichern



void SwEditShell::SpellStart( SwDocPositions eStart, SwDocPositions eEnd,
                                SwDocPositions eCurr )
{
    // do not spell if interactive spelling is active elsewhere
    if (!pSpellIter)
    {
        ASSERT( !pSpellIter, "wer ist da schon am spellen?" );
        pSpellIter = new SwSpellIter;

        SwCursor* pSwCrsr = GetSwCrsr();

        SwPosition *pTmp = new SwPosition( *pSwCrsr->GetPoint() );
        pSwCrsr->FillFindPos( eCurr, *pTmp );
        pSpellIter->SetCurr( pTmp );

        pTmp = new SwPosition( *pTmp );
        pSpellIter->SetCurrX( pTmp );

        pSpellIter->Start( this, eStart, eEnd );
    }
}

/*************************************************************************
 *                  SwEditShell::SpellEnd
 *************************************************************************/

// Selektionen wiederherstellen



void SwEditShell::SpellEnd()
{
    if (pSpellIter->GetSh() == this)
    {
        ASSERT( pSpellIter, "wo ist mein Iterator?" );
        pSpellIter->_End();
        delete pSpellIter, pSpellIter = 0;
    }
}

/*************************************************************************
 *                  SwEditShell::SpellContinue
 *************************************************************************/

// liefert Rueckgabewerte entsprechend SPL_ in splchk.hxx

uno::Reference< uno::XInterface >
    SwEditShell::SpellContinue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    if (pSpellIter->GetSh() != this)
        return 0;

    if( pPageCnt && !*pPageCnt )
    {
        sal_uInt16 nEndPage = GetLayout()->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
        *pPageCnt = nEndPage;
        if( nEndPage )
            ::StartProgress( STR_STATSTR_SPELL, 0, nEndPage, GetDoc()->GetDocShell() );
    }

    ASSERT( pSpellIter, "wo ist mein Iterator?" );
    //JP 18.07.95: verhinder bei Fehlermeldungen die Anzeige der Selektionen
    //              KEIN StartAction, da damit auch die Paints abgeschaltet
    //              werden !!!!!
    ++nStartAction;
     uno::Reference< uno::XInterface >  xRet = pSpellIter->Continue( pPageCnt, pPageSt );
    --nStartAction;

    if( xRet.is() )
    {
        // dann die awt::Selection sichtbar machen
        StartAction();
        EndAction();
    }
    return xRet;
}

/*************************************************************************
 *                  SwEditShell::HyphStart
 *************************************************************************/

/* Interaktive Trennung, BP 10.03.93
 *
 * 1) HyphStart
 *    - Aufheben aller Selektionen
 *    - Sichern des aktuellen Cursors
 *    - falls keine Selektion vorhanden:
 *      - neue Selektion bis zum Dokumentende
 * 2) HyphContinue
 *    - nLastHyphLen wird auf den Selektionsstart addiert
 *    - iteriert ueber alle selektierten Bereiche
 *      - pDoc->Hyphenate() iteriert ueber alle Nodes der Selektion
 *          - pTxtNode->Hyphenate() ruft das SwTxtFrm::Hyphenate zur EditShell
 *              - SwTxtFrm:Hyphenate() iteriert ueber die Zeilen des Pams
 *                  - LineIter::Hyphenate() stellt den Hyphenator
 *                    und den Pam auf das zu trennende Wort ein.
 *    - Es gibt nur zwei Returnwerte sal_True, wenn eine Trennstelle anliegt
 *      und sal_False, wenn der Pam abgearbeitet wurde.
 *    - Bei sal_True wird das selektierte Wort zur Anzeige gebracht und
 *      nLastHyphLen gesetzt.
 *    - Bei sal_False wird die aktuelle Selektion geloescht und die naechste
 *      zur aktuellen gewaehlt. Return HYPH_OK, wenn keine mehr vorhanden.
 * 3) InsertSoftHyph (wird ggf. von der UI gerufen)
 *    - Der aktuelle Cursor wird plaziert und das Attribut eingefuegt.
 * 4) HyphEnd
 *    - Wiederherstellen des alten Cursors, EndAction
 */



void SwEditShell::HyphStart( SwDocPositions eStart, SwDocPositions eEnd )
{
    // do not hyphenate if interactive hyphenationg is active elsewhere
    if (!pHyphIter)
    {
        ASSERT( !pHyphIter, "wer ist da schon am hyphinieren?" );
        pHyphIter = new SwHyphIter;
        pHyphIter->Start( this, eStart, eEnd );
    }
}

/*************************************************************************
 *                  SwEditShell::HyphEnd
 *************************************************************************/

// Selektionen wiederherstellen



void SwEditShell::HyphEnd()
{
    if (pHyphIter->GetSh() == this)
    {
        ASSERT( pHyphIter, "wo ist mein Iterator?" );
        pHyphIter->End();
        delete pHyphIter, pHyphIter = 0;
    }
}

/*************************************************************************
 *                  SwEditShell::HyphContinue
 *************************************************************************/

// Returnwerte: (BP: ich wuerde es genau umdrehen, aber die UI wuenscht es so)
// HYPH_CONTINUE, wenn eine Trennstelle anliegt
// HYPH_OK, wenn der selektierte Bereich abgearbeitet wurde.


uno::Reference< uno::XInterface >
    SwEditShell::HyphContinue( sal_uInt16* pPageCnt, sal_uInt16* pPageSt )
{
    if (pHyphIter->GetSh() != this)
        return 0;

    if( pPageCnt && !*pPageCnt && !*pPageSt )
    {
        sal_uInt16 nEndPage = GetLayout()->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
        if( nEndPage > 14 )
        {
            *pPageCnt = nEndPage;
            ::StartProgress( STR_STATSTR_HYPHEN, 0, nEndPage, GetDoc()->GetDocShell());
        }
        else                // Hiermit unterdruecken wir ein fuer allemal
            *pPageSt = 1;   // das StatLineStartPercent
    }

    ASSERT( pHyphIter, "wo ist mein Iterator?" );
    //JP 18.07.95: verhinder bei Fehlermeldungen die Anzeige der Selektionen
    //              KEIN StartAction, da damit auch die Paints abgeschaltet
    //              werden !!!!!
    ++nStartAction;
     uno::Reference< uno::XInterface >  xRet = pHyphIter->Continue( pPageCnt, pPageSt );
    --nStartAction;

    if( xRet.is() )
        pHyphIter->ShowSelection();

    return xRet;
}


/*************************************************************************
 *                  SwEditShell::InsertSoftHyph
 *************************************************************************/

// Zum Einfuegen des SoftHyphens, Position ist der Offset
// innerhalb des getrennten Wortes.


void SwEditShell::InsertSoftHyph( const xub_StrLen nHyphPos )
{
    ASSERT( pHyphIter, "wo ist mein Iterator?" );
    pHyphIter->InsertSoftHyph( nHyphPos );
}


/*************************************************************************
 *                  SwEditShell::HyphIgnore
 *************************************************************************/

// Beschreibung: Trennstelle ignorieren

void SwEditShell::HyphIgnore()
{
    ASSERT( pHyphIter, "wo ist mein Iterator?" );
    //JP 18.07.95: verhinder bei Fehlermeldungen die Anzeige der Selektionen
    //              KEIN StartAction, da damit auch die Paints abgeschaltet
    //              werden !!!!!
    ++nStartAction;
    pHyphIter->Ignore();
    --nStartAction;

    pHyphIter->ShowSelection();
}

/*************************************************************************
 *                  SwEditShell::GetCorrection()
 * liefert eine Liste von Vorschlaegen fuer falsch geschriebene Worte,
 * ein NULL-Pointer signalisiert, dass das Wort richtig geschrieben ist,
 * eine leere Liste, dass das Wort zwar unbekannt ist, aber keine Alternativen
 * geliefert werden koennen.
 *************************************************************************/


uno::Reference< XSpellAlternatives >
    SwEditShell::GetCorrection( const Point* pPt )
{
     uno::Reference< XSpellAlternatives >  xSpellAlt;

    if( IsTableMode() )
        return NULL;
    SwPaM* pCrsr = GetCrsr();
    SwPosition aPos( *pCrsr->GetPoint() );
     Point aPt( *pPt );
    SwCrsrMoveState eTmpState( MV_SETONLYTEXT );
    SwTxtNode *pNode;
    SwWrongList *pWrong;
    if( GetLayout()->GetCrsrOfst( &aPos, aPt, &eTmpState ) &&
        0 != (pNode = aPos.nNode.GetNode().GetTxtNode()) &&
        0 != (pWrong = pNode->GetWrong()) &&
        !pNode->IsInProtectSect() )
    {
        xub_StrLen nBegin = aPos.nContent.GetIndex();
        xub_StrLen nLen = 1;
        if( pWrong->InWrongWord(nBegin,nLen) && !pNode->IsSymbol(nBegin) )
        {
            String aWord( pNode->GetTxt().Copy( nBegin, nLen ) );
            aWord.EraseAllChars( CH_TXTATR_BREAKWORD ).EraseAllChars( CH_TXTATR_INWORD );

            sal_Bool bSpell = sal_True;

            uno::Reference< XSpellChecker1 >  xSpell( ::GetSpellChecker() );
            if( xSpell.is() )
            {
                LanguageType eActLang = (LanguageType)pNode->GetLang(
                                                            nBegin, nLen );
                if( xSpell->hasLanguage( eActLang ))
                    xSpellAlt = xSpell->spell( aWord, eActLang,
                                               Sequence< PropertyValue >() );
            }

            if ( xSpellAlt.is() )
            {
#ifdef DEBUG
                pNode->GetWrong()->Invalidate( 0, STRING_LEN );
                pNode->SetWrongDirty( sal_True );
#endif
                aPos.nContent = nBegin;
                *pCrsr->GetPoint() = aPos;
                pCrsr->SetMark();
                ExtendSelection( sal_True, nLen );
            }
        }
    }
    return xSpellAlt;
}




