/*************************************************************************
 *
 *  $RCSfile: ftnfrm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-02 10:45:12 $
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

#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#include "pagefrm.hxx"
#include "colfrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "doc.hxx"
#include "ndtxt.hxx"
#include "frmtool.hxx"
#include "errhdl.hxx"
#include "swtable.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "tabfrm.hxx"
#include "pagedesc.hxx"
#include "ftninfo.hxx"
#include "ndindex.hxx"
#include "sectfrm.hxx"

/*************************************************************************
|*
|*  lcl_FindFtnPos()        Sucht die Position des Attributes im FtnArray am
|*      Dokument, dort stehen die Fussnoten gluecklicherweise nach ihrem
|*      Index sortiert.
|*
|*  Ersterstellung      MA 29. Jun. 93
|*  Letzte Aenderung    MA 13. Dec. 93
|*
|*************************************************************************/

#define ENDNOTE 0x80000000

ULONG MA_FASTCALL lcl_FindFtnPos( const SwDoc *pDoc, const SwTxtFtn *pAttr )
{
    const SwFtnIdxs &rFtnIdxs = pDoc->GetFtnIdxs();

#ifdef MA_DEBUG
    //Wenn das Array nicht stimmt haben wir ein Problem, denn viele
    //Ftn-Functions bauen auf dem Array auf.
    for ( USHORT k = 0; k+1 < rFtnIdxs.Count(); ++k )
    {
        SwIndex aIdx1(&pDoc->GetNodes());
        SwIndex aIdx2(&pDoc->GetNodes());
        rFtnIdxs[k]->pFtn->  GetTxtNode().GetIndex(aIdx1);
        rFtnIdxs[k+1]->pFtn->GetTxtNode().GetIndex(aIdx2);
        if ( aIdx1.GetIndex() > aIdx2.GetIndex() )
        {
            ASSERT( !rFtnIdxs.Count(), "FtnIdxs not up to date" );
        }
        else if ( aIdx1.GetIndex() == aIdx2.GetIndex() )
        {
            SwTxtFtn *p1 = rFtnIdxs[k];
            SwTxtFtn *p2 = rFtnIdxs[k+1];
            ASSERT( *p1->GetStart() < *p2->GetStart(),
                    "FtnIdxs not up to date" );
        }
    }
#endif

    USHORT nRet;
    SwTxtFtnPtr pBla = (SwTxtFtn*)pAttr;
    if ( rFtnIdxs.Seek_Entry( pBla, &nRet ) )
    {
        if( pAttr->GetFtn().IsEndNote() )
            return ULONG(nRet) + ENDNOTE;
        return nRet;
    }
    ASSERT( !pDoc, "FtnPos not found." );
    return 0;
}

BOOL SwFtnFrm::operator<( const SwTxtFtn* pTxtFtn ) const
{
    const SwDoc* pDoc = GetFmt()->GetDoc();
    ASSERT( pDoc, "SwFtnFrm: Missing doc!" );
    return lcl_FindFtnPos( pDoc, GetAttr() ) <
           lcl_FindFtnPos( pDoc, pTxtFtn );
}

/*************************************************************************
|*
|*  BOOL lcl_NextFtnBoss( SwFtnBossFrm* pBoss, SwPageFrm* pPage)
|*  setzt pBoss auf den naechsten SwFtnBossFrm, das kann entweder eine Spalte
|*  oder eine Seite (ohne Spalten) sein. Wenn die Seite dabei gewechselt wird
|*  enthaelt pPage die neue Seite und die Funktion liefert TRUE.
|*
|*  Ersterstellung      AMA 06. Nov. 98
|*  Letzte Aenderung    AMA 06. Nov. 98
|*
|*************************************************************************/

BOOL lcl_NextFtnBoss( SwFtnBossFrm* &rpBoss, SwPageFrm* &rpPage,
    BOOL bDontLeave )
{
    if( rpBoss->IsColumnFrm() )
    {
        if( rpBoss->GetNext() )
        {
            rpBoss = (SwFtnBossFrm*)rpBoss->GetNext(); //naechste Spalte
            return FALSE;
        }
        if( rpBoss->IsInSct() )
        {
            SwSectionFrm* pSct = rpBoss->FindSctFrm()->GetFollow();
            if( pSct )
            {
                ASSERT( pSct->Lower() && pSct->Lower()->IsColumnFrm(),
                        "Where's the column?" );
                rpBoss = (SwColumnFrm*)pSct->Lower();
                SwPageFrm* pOld = rpPage;
                rpPage = pSct->FindPageFrm();
                return pOld != rpPage;
            }
            else if( bDontLeave )
            {
                rpPage = NULL;
                rpBoss = NULL;
                return FALSE;
            }
        }
    }
    rpPage = (SwPageFrm*)rpPage->GetNext(); // naechste Seite
    rpBoss = rpPage;
    if( rpPage )
    {
        SwLayoutFrm* pBody = rpPage->FindBodyCont();
        if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            rpBoss = (SwFtnBossFrm*)pBody->Lower(); // erste Spalte
    }
    return TRUE;
}

/*************************************************************************
|*
|*  USHORT lcl_ColumnNum( SwFrm* pBoss )
|*  liefert die Spaltennummer, wenn pBoss eine Spalte ist,
|*  sonst eine Null (bei Seiten).
|*
|*  Ersterstellung      AMA 06. Nov. 98
|*  Letzte Aenderung    AMA 06. Nov. 98
|*
|*************************************************************************/

USHORT lcl_ColumnNum( const SwFrm* pBoss )
{
    USHORT nRet = 0;
    if( !pBoss->IsColumnFrm() )
        return 0;
    const SwFrm* pCol;
    if( pBoss->IsInSct() )
    {
        pCol = pBoss->GetUpper()->FindColFrm();
        if( pBoss->GetNext() || pBoss->GetPrev() )
        {
            while( pBoss )
            {
                ++nRet;                     // Section columns
                pBoss = pBoss->GetPrev();
            }
        }
    }
    else
        pCol = pBoss;
    while( pCol )
    {
        nRet += 256;                    // Page columns
        pCol = pCol->GetPrev();
    }
    return nRet;
}

/*************************************************************************
|*
|*  SwFtnContFrm::SwFtnContFrm()
|*
|*  Ersterstellung      MA 24. Feb. 93
|*  Letzte Aenderung    MA 02. Mar. 93
|*
|*************************************************************************/


SwFtnContFrm::SwFtnContFrm( SwFrmFmt *pFmt ):
    SwLayoutFrm( pFmt )
{
    nType = FRM_FTNCONT;
}


// lcl_Undersize(..) klappert einen SwFrm und dessen Inneres ab
// und liefert die Summe aller TxtFrm-Vergroesserungswuensche

long lcl_Undersize( const SwFrm* pFrm )
{
    long nRet = 0;
    if( pFrm->IsTxtFrm() )
    {
        if( ((SwTxtFrm*)pFrm)->IsUndersized() )
            // Dieser TxtFrm waere gern ein bisschen groesser
            nRet = ((SwTxtFrm*)pFrm)->GetParHeight() - pFrm->Prt().Height();
            if( nRet < 0 )
                nRet = 0;
    }
    else if( pFrm->IsLayoutFrm() )
    {
        const SwFrm* pNxt = ((SwLayoutFrm*)pFrm)->Lower();
        while( pNxt )
        {
            nRet += lcl_Undersize( pNxt );
            pNxt = pNxt->GetNext();
        }
    }
    return nRet;
}

/*************************************************************************
|*
|*  SwFtnContFrm::Format()
|*
|*  Beschreibung:       "Formatiert" den Frame;
|*                      Die Fixsize wird hier nicht eingestellt.
|*  Ersterstellung      MA 01. Mar. 93
|*  Letzte Aenderung    MA 17. Nov. 98
|*
|*************************************************************************/


void SwFtnContFrm::Format( const SwBorderAttrs * )
{
    //GesamtBorder ermitteln, es gibt nur einen Abstand nach oben.
    const SwPageFrm* pPage = FindPageFrm();
    const SwPageFtnInfo &rInf = pPage->GetPageDesc()->GetFtnInfo();
    const SwTwips nBorder = rInf.GetTopDist() + rInf.GetBottomDist() +
                            rInf.GetLineWidth();

    if ( !bValidPrtArea )
    {
        bValidPrtArea = TRUE;
        Prt().Top( nBorder );
        Prt().Width ( Frm().Width() );
        Prt().Height( Frm().Height() - nBorder );
        if( Prt().Height() < 0 && !pPage->IsFtnPage() )
            bValidSize = FALSE;
    }

    if ( !bValidSize )
    {
        if ( pPage->IsFtnPage() && !GetFmt()->GetDoc()->IsBrowseMode() )
        {
            //Das Teil ist immer so gross wie moeglich
//MA 17. Nov. 98 Wozu soll das Probegrow sinnvoll sein?
//          if ( Grow( LONG_MAX, pHeight, TRUE ) )
                Grow( LONG_MAX, pHeight, FALSE );
        }
        else
        {
            //Die Groesse in der VarSize wird durch den Inhalt plus den
            //Raendern bestimmt.
            SwTwips nRemaining = 0;
            SwFrm *pFrm = pLower;
            while ( pFrm )
            {   // lcl_Undersize(..) beruecksichtigt (rekursiv) TxtFrms, die gerne
                // groesser waeren. Diese entstehen insbesondere in spaltigen Rahmen,
                // wenn diese noch nicht ihre maximale Groesse haben.
                nRemaining += pFrm->Frm().SSize().Height() + lcl_Undersize( pFrm );
                pFrm = pFrm->GetNext();
            }
            //Jetzt noch den Rand addieren
            nRemaining += nBorder;

            SwTwips nDiff;
            if( IsInSct() )
            {
                nDiff = GetUpper()->Frm().Top() + GetUpper()->Prt().Top() +
                        GetUpper()->Prt().Height() - Frm().Top();
                if( nDiff < Frm().Height() )
                {
                    if( nDiff < 0 )
                        nDiff = 0;
                    Frm().Height( nDiff );
                    Prt().Height( nDiff - Prt().Top() );
                }
            }
            nDiff = Frm().SSize().Height() - nRemaining;
            if ( nDiff > 0 )
                Shrink( nDiff, pHeight );
            else if ( nDiff < 0 )
            {
                Grow( -nDiff, pHeight );
                //Es kann passieren, dass weniger Platz zur Verfuegung steht,
                //als der bereits der Border benoetigt - die Groesse der
                //PrtArea wird dann negativ.
                if ( Prt().Height() < 0 )
                {
                    //Ausweg: Der Border wird so verkleinert, das die Hoehe
                    //der PrtArea gerade 0 wird. Das richtet sich dann schon
                    //wieder.
                    const SwTwips nDiff = Max( Prt().Top(), -Prt().Height() );
                    Prt().SSize().Height() += nDiff;
                    Prt().Pos().Y() -= nDiff;
                }
            }
        }
        bValidSize = TRUE;
    }
}
/*************************************************************************
|*
|*  SwFtnContFrm::GrowFrm(), ShrinkFrm()
|*
|*  Ersterstellung      MA 24. Feb. 93
|*  Letzte Aenderung    AMA 05. Nov. 98
|*
|*************************************************************************/


SwTwips SwFtnContFrm::GrowFrm( SwTwips nDist, const SzPtr pDirection,
                               BOOL bTst, BOOL bInfo )
{
    //Keine Pruefung ob FixSize oder nicht, die FtnContainer sind immer bis
    //zur Maximalhoehe variabel.
    //Wenn die Maximalhoehe LONG_MAX ist, so nehmen wir uns soviel Platz wie eben
    //moeglich.
    //Wenn die Seite eine spezielle Fussnotenseite ist, so nehmen wir uns auch
    //soviel Platz wie eben moeglich.
#ifndef PRODUCT
    if ( !GetUpper() || !GetUpper()->IsFtnBossFrm() )
    {   ASSERT( !this, "Keine FtnBoss." );
        return 0;
    }
#endif

    if ( Frm().SSize().*pDirection > 0 &&
         nDist > (LONG_MAX - Frm().SSize().*pDirection) )
        nDist = LONG_MAX - Frm().SSize().*pDirection;

    SwFtnBossFrm *pBoss = (SwFtnBossFrm*)GetUpper();
    if( IsInSct() )
    {
        SwSectionFrm* pSect = FindSctFrm();
        ASSERT( pSect, "GrowFrm: Missing SectFrm" );
        // In a section, which has to maximize, a footnotecontainer is allowed
        // to grow, when the section can't grow anymore.
        if( !bTst && !pSect->IsColLocked() &&
            pSect->ToMaximize( FALSE ) && pSect->Growable() )
        {
            pSect->InvalidateSize();
            return 0;
        }
    }
    SwPageFrm *pPage = pBoss->FindPageFrm();
    if ( !pPage->IsFtnPage() || GetFmt()->GetDoc()->IsBrowseMode() )
    {
        if ( pBoss->GetMaxFtnHeight() != LONG_MAX )
        {
            nDist = Min( nDist, pBoss->GetMaxFtnHeight() - Frm().Height() );
            if ( nDist <= 0 )
                return 0L;
        }
        //Der FtnBoss will bezueglich des MaxWerts auch noch mitreden.
        if( !IsInSct() )
        {
            const SwTwips nMax = pBoss->GetVarSpace();
            if ( nDist > nMax )
                nDist = nMax;
            if ( nDist <= 0 )
                return 0L;
        }
    }
    else if ( nDist > GetPrev()->Frm().Height() )
        //aber mehr als der Body kann koennen und wollen wir nun auch wieder
        //nicht herausruecken.
        nDist = GetPrev()->Frm().Height();

    //Nicht mehr verlangen als benoetigt wird.
    long nAvail = 0;
    if ( GetFmt()->GetDoc()->IsBrowseMode() )
    {
        nAvail = GetUpper()->Prt().Height();
        const SwFrm *pAvail = GetUpper()->Lower();
        do
        {   nAvail -= pAvail->Frm().Height();
            pAvail = pAvail->GetNext();
        } while ( pAvail );
        if ( nAvail > nDist )
            nAvail = nDist;
    }

    if ( !bTst )
        Frm().SSize().*pDirection += nDist;

    long nGrow = nDist - nAvail,
         nReal = 0;
    if ( nGrow > 0 )
    {
        BYTE nAdjust = pBoss->NeighbourhoodAdjustment( this );
        if( NA_ONLY_ADJUST == nAdjust )
            nReal = AdjustNeighbourhood( nGrow, bTst );
        else
        {
            if( NA_GROW_ADJUST == nAdjust )
            {
                SwFrm* pFtn = Lower();
                if( pFtn )
                {
                    while( pFtn->GetNext() )
                        pFtn = pFtn->GetNext();
                    if( ((SwFtnFrm*)pFtn)->GetAttr()->GetFtn().IsEndNote() )
                    {
                        nReal = AdjustNeighbourhood( nGrow, bTst );
                        nAdjust = NA_GROW_SHRINK; // no more AdjustNeighbourhood
                    }
                }
            }
            nReal += pBoss->Grow( nGrow - nReal, pHeight, bTst );
            if( ( NA_GROW_ADJUST == nAdjust || NA_ADJUST_GROW == nAdjust )
                  && nReal < nGrow )
                nReal += AdjustNeighbourhood( nGrow - nReal, bTst );
        }
    }

    nReal += nAvail;

    if ( !bTst )
    {
        if ( nReal != nDist )
            //Den masslosen Wunsch koennen wir leider nur in Grenzen erfuellen.
            Frm().SSize().*pDirection -= (nDist - nReal);
        //Nachfolger braucht nicht invalidiert werden, denn wir wachsen
        //immer nach oben.
        if( nReal )
        {
            _InvalidateSize();
            _InvalidatePos();
            InvalidatePage( pPage );
        }
    }
    return nReal;
}



SwTwips SwFtnContFrm::ShrinkFrm( SwTwips nDiff, const SzPtr pDirection,
                                 BOOL bTst, BOOL bInfo )
{
    SwPageFrm *pPage = FindPageFrm();
    if ( pPage && (!pPage->IsFtnPage() || GetFmt()->GetDoc()->IsBrowseMode()) )
    {
        SwTwips nRet = SwLayoutFrm::ShrinkFrm( nDiff, pDirection, bTst, bInfo );
        if ( !bTst && nRet )
        {
            _InvalidatePos();
            InvalidatePage( pPage );
        }
        return nRet;
    }
    return 0;
}


/*************************************************************************
|*
|*  SwFtnFrm::SwFtnFrm()
|*
|*  Ersterstellung      MA 24. Feb. 93
|*  Letzte Aenderung    MA 11. Oct. 93
|*
|*************************************************************************/


SwFtnFrm::SwFtnFrm( SwFrmFmt *pFmt, SwCntntFrm *pCnt, SwTxtFtn *pAt ):
    SwLayoutFrm( pFmt ),
    pFollow( 0 ),
    pMaster( 0 ),
    pRef( pCnt ),
    pAttr( pAt ),
    bBackMoveLocked( FALSE )
{
    nType = FRM_FTN;
}

/*************************************************************************
|*
|*  SwFtnFrm::InvalidateNxtFtnCnts()
|*
|*  Ersterstellung      MA 29. Jun. 93
|*  Letzte Aenderung    MA 29. Jun. 93
|*
|*************************************************************************/


void SwFtnFrm::InvalidateNxtFtnCnts( SwPageFrm *pPage )
{
    if ( GetNext() )
    {
        SwFrm *pCnt = ((SwLayoutFrm*)GetNext())->ContainsAny();
        if( pCnt )
        {
            pCnt->InvalidatePage( pPage );
            pCnt->_InvalidatePrt();
            do
            {   pCnt->_InvalidatePos();
                if( pCnt->IsSctFrm() )
                {
                    SwFrm* pTmp = ((SwSectionFrm*)pCnt)->ContainsAny();
                    if( pTmp )
                        pTmp->_InvalidatePos();
                }
                pCnt->GetUpper()->_InvalidateSize();
                pCnt = pCnt->FindNext();
            } while ( pCnt && GetUpper()->IsAnLower( pCnt ) );
        }
    }
}

#ifndef PRODUCT


SwTwips SwFtnFrm::GrowFrm( SwTwips nDist, const SzPtr pDirection,
                           BOOL bTst, BOOL bInfo )
{
#ifdef DEBUG
    static USHORT nNum = USHRT_MAX;
    SwTxtFtn* pTxtFtn = GetAttr();
    if ( pTxtFtn->GetFtn().GetNumber() == nNum )
    {
        int bla = 5;
    }
#endif
    return SwLayoutFrm::GrowFrm( nDist, pDirection, bTst, bInfo );
}



SwTwips SwFtnFrm::ShrinkFrm( SwTwips nDist, const SzPtr pDirection,
                                BOOL bTst, BOOL bInfo )
{
#ifdef DEBUG
    static USHORT nNum = USHRT_MAX;
    if( nNum != USHRT_MAX )
    {
        SwTxtFtn* pTxtFtn = GetAttr();
        if( &pTxtFtn->GetAttr() && pTxtFtn->GetFtn().GetNumber() == nNum )
        {
            int bla = 5;
        }
    }
#endif
    return SwLayoutFrm::ShrinkFrm( nDist, pDirection, bTst, bInfo );
}
#endif

/*************************************************************************
|*
|*  SwFtnFrm::Cut()
|*
|*  Ersterstellung      MA 23. Feb. 94
|*  Letzte Aenderung    MA 24. Jul. 95
|*
|*************************************************************************/


void SwFtnFrm::Cut()
{
    if ( GetNext() )
        GetNext()->InvalidatePos();
    else if ( GetPrev() )
        GetPrev()->SetRetouche();

    //Erst removen, dann Upper Shrinken.
    SwLayoutFrm *pUp = GetUpper();

    //Verkettung korrigieren.
    SwFtnFrm *pFtn = (SwFtnFrm*)this;
    if ( pFtn->GetFollow() )
        pFtn->GetFollow()->SetMaster( pFtn->GetMaster() );
    if ( pFtn->GetMaster() )
        pFtn->GetMaster()->SetFollow( pFtn->GetFollow() );
    pFtn->SetFollow( 0 );
    pFtn->SetMaster( 0 );

    // Alle Verbindungen kappen.
    Remove();

    if ( pUp )
    {
        //Die letzte Fussnote nimmt ihren Container mit.
        if ( !pUp->Lower() )
        {
            SwPageFrm *pPage = pUp->FindPageFrm();
            if ( pPage )
            {
                SwLayoutFrm *pBody = pPage->FindBodyCont();
                if ( !pBody->ContainsCntnt() )
                    pPage->FindRootFrm()->SetSuperfluous();
            }
            SwSectionFrm* pSect = pUp->FindSctFrm();
            pUp->Cut();
            delete pUp;
            // Wenn der letzte Fussnotencontainer aus einem spaltigen Bereich verschwindet,
            // so kann dieser, falls er keinen Follow besitzt, zusammenschrumpfen.
            if( pSect && !pSect->ToMaximize( FALSE ) && !pSect->IsColLocked() )
                pSect->_InvalidateSize();
        }
        else
        {   if ( Frm().Height() )
                pUp->Shrink( Frm().Height(), pHeight );
            pUp->SetCompletePaint();
            pUp->InvalidatePage();
        }
    }
}

/*************************************************************************
|*
|*  SwFtnFrm::Paste()
|*
|*  Ersterstellung      MA 23. Feb. 94
|*  Letzte Aenderung    MA 23. Feb. 94
|*
|*************************************************************************/


void SwFtnFrm::Paste(  SwFrm* pParent, SwFrm* pSibling )
{
    ASSERT( pParent, "Kein Parent fuer Paste." );
    ASSERT( pParent->IsLayoutFrm(), "Parent ist CntntFrm." );
    ASSERT( pParent != this, "Bin selbst der Parent." );
    ASSERT( pSibling != this, "Bin mein eigener Nachbar." );
    ASSERT( !GetPrev() && !GetNext() && !GetUpper(),
            "Bin noch irgendwo angemeldet." );

    //In den Baum einhaengen.
    InsertBefore( (SwLayoutFrm*)pParent, pSibling );

    if( Frm().Width() != pParent->Prt().Width() )
        _InvalidateSize();
    _InvalidatePos();
    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    if ( GetNext() )
        GetNext()->_InvalidatePos();
    if ( Frm().Height() )
        pParent->Grow( Frm().Height(), pHeight );

    //Wenn mein Vorgaenger mein Master ist und/oder wenn mein Nachfolger mein
    //Follow ist so kann ich deren Inhalt uebernehmen und sie vernichten.
    if ( GetPrev() && GetPrev() == GetMaster() )
    {   ASSERT( SwFlowFrm::CastFlowFrm( GetPrev()->GetLower() ),
                "Fussnote ohne Inhalt?" );
        (SwFlowFrm::CastFlowFrm( GetPrev()->GetLower()))->
            MoveSubTree( this, GetLower() );
        SwFrm *pDel = GetPrev();
        pDel->Cut();
        delete pDel;
    }
    if ( GetNext() && GetNext() == GetFollow() )
    {   ASSERT( SwFlowFrm::CastFlowFrm( GetNext()->GetLower() ),
                "Fussnote ohne Inhalt?" );
        (SwFlowFrm::CastFlowFrm( GetNext()->GetLower()))->MoveSubTree( this );
        SwFrm *pDel = GetNext();
        pDel->Cut();
        delete pDel;
    }
#ifndef PRODUCT
    SwDoc *pDoc = GetFmt()->GetDoc();
    if ( GetPrev() )
    {
        ASSERT( lcl_FindFtnPos( pDoc, ((SwFtnFrm*)GetPrev())->GetAttr() ) <=
                lcl_FindFtnPos( pDoc, GetAttr() ), "Prev ist not FtnPrev" );
    }
    if ( GetNext() )
    {
        ASSERT( lcl_FindFtnPos( pDoc, GetAttr() ) <=
                lcl_FindFtnPos( pDoc, ((SwFtnFrm*)GetNext())->GetAttr() ),
                "Next is not FtnNext" );
    }
#endif
    InvalidateNxtFtnCnts( pPage );
}

/*************************************************************************
|*
|*  SwFrm::GetNextFtnLeaf()
|*
|*  Beschreibung        Liefert das naechste LayoutBlatt in den das
|*      Frame gemoved werden kann.
|*      Neue Seiten werden nur dann erzeugt, wenn der Parameter TRUE ist.
|*  Ersterstellung      MA 16. Nov. 92
|*  Letzte Aenderung    AMA 09. Nov. 98
|*
|*************************************************************************/


SwLayoutFrm *SwFrm::GetNextFtnLeaf( MakePageType eMakePage )
{
    SwFtnBossFrm *pOldBoss = FindFtnBossFrm();
    SwPageFrm* pOldPage = pOldBoss->FindPageFrm();
    SwPageFrm* pPage;
    SwFtnBossFrm *pBoss = pOldBoss->IsColumnFrm() ?
        (SwFtnBossFrm*)pOldBoss->GetNext() : 0; // naechste Spalte, wenn vorhanden
    if( pBoss )
        pPage = NULL;
    else
    {
        if( pOldBoss->GetUpper()->IsSctFrm() )
        {   // Das kann nur in einem spaltigen Bereich sein
            SwLayoutFrm* pNxt = pOldBoss->GetNextSctLeaf( eMakePage );
            if( pNxt )
            {
                ASSERT( pNxt->IsColBodyFrm(), "GetNextFtnLeaf: Funny Leaf" );
                pBoss = (SwFtnBossFrm*)pNxt->GetUpper();
                pPage = pBoss->FindPageFrm();
            }
            else
                return 0;
        }
        else
        {
            // naechste Seite
            pPage = (SwPageFrm*)pOldPage->GetNext();
            // Leerseiten ueberspringen
            if( pPage && pPage->IsEmptyPage() )
                pPage = (SwPageFrm*)pPage->GetNext();
            pBoss = pPage;
        }
    }
    // Was haben wir jetzt?
    // pBoss != NULL, pPage==NULL => pBoss ist die auf der gleichen Seite folgende Spalte
    // pBoss != NULL, pPage!=NULL => pBoss und pPage sind die folgende Seite (Empty uebersprungen)
    // pBoss == NULL => pPage == NULL, es gibt keine folgende Seite

    //Wenn die Fussnote bereits einen Follow hat brauchen wir nicht zu suchen.
    //Wenn allerdings zwischen Ftn und Follow unerwuenschte Leerseiten/spalten
    //herumlungern, so legen wir auf der naechstbesten Seite/Spalte einen weiteren
    //Follow an, der Rest wird sich schon finden.
    SwFtnFrm *pFtn = FindFtnFrm();
    if ( pFtn && pFtn->GetFollow() )
    {
        SwFtnBossFrm* pTmpBoss = pFtn->GetFollow()->FindFtnBossFrm();
        // Folgende Faelle werden hier erkannt und akzeptiert
        // 1. Die FtnBosse sind benachbarte Seiten oder benachbarte Spalten
        // 2. Der neue ist die erste Spalte der benachbarten Seite
        // 3. Der neue ist die erste Spalte in einem Bereich in der naechsten Spalte/Seite
        while( pTmpBoss != pBoss && pTmpBoss && !pTmpBoss->GetPrev() )
            pTmpBoss = pTmpBoss->GetUpper()->FindFtnBossFrm();
        if( pTmpBoss == pBoss )
            return pFtn->GetFollow();
    }

    // Wenn wir keinen pBoss gefunden haben oder es sich um eine "falsche" Seite handelt,
    // muss eine neue Seite her
    if ( !pBoss || ( pPage && pPage->IsEndNotePage() && !pOldPage->IsEndNotePage() ) )
    {
        if ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT )
        {
            pBoss = InsertPage( pOldPage, pOldPage->IsFtnPage() );
            ((SwPageFrm*)pBoss)->SetEndNotePage( pOldPage->IsEndNotePage() );
        }
        else
            return 0;
    }
    if( pBoss->IsPageFrm() )
    {   // Wenn wir auf einer spaltigen Seite gelandet sind,
        // gehen wir in die erste Spalte
        SwLayoutFrm* pLay = pBoss->FindBodyCont();
        if( pLay && pLay->Lower() && pLay->Lower()->IsColumnFrm() )
            pBoss = (SwFtnBossFrm*)pLay->Lower();
    }
    //Seite/Spalte gefunden, da schummeln wir uns doch gleich mal 'rein
    SwFtnContFrm *pCont = pBoss->FindFtnCont();
    if ( !pCont && pBoss->GetMaxFtnHeight() &&
         ( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT ) )
        pCont = pBoss->MakeFtnCont();
    return pCont;
}

/*************************************************************************
|*
|*  SwFrm::GetPrevFtnLeaf()
|*
|*  Beschreibung        Liefert das vorhergehende LayoutBlatt in das der
|*      Frame gemoved werden kann.
|*  Ersterstellung      MA 16. Nov. 92
|*  Letzte Aenderung    AMA 06. Nov. 98
|*
|*************************************************************************/


SwLayoutFrm *SwFrm::GetPrevFtnLeaf( MakePageType eMakeFtn )
{
    //Der Vorgaenger fuer eine Fussnote ist falls moeglich der Master
    //in der Fussnoteneigenen Verkettung.
    SwLayoutFrm *pRet = 0;
    SwFtnFrm *pFtn = FindFtnFrm();
    pRet = pFtn->GetMaster();

    SwFtnBossFrm* pOldBoss = FindFtnBossFrm();
    SwPageFrm *pOldPage = pOldBoss->FindPageFrm();

    if ( !pOldBoss->GetPrev() && !pOldPage->GetPrev() )
        return pRet; // es gibt weder eine Spalte noch eine Seite vor uns

    if ( !pRet )
    {
        BOOL bEndn = pFtn->GetAttr()->GetFtn().IsEndNote();
        SwFrm* pTmpRef = NULL;
        if( bEndn && pFtn->IsInSct() )
        {
            SwSectionFrm* pSect = pFtn->FindSctFrm();
            if( pSect->IsEndnAtEnd() )
                pTmpRef = pSect->FindLastCntnt( FINDMODE_LASTCNT );
        }
        if( !pTmpRef )
            pTmpRef = pFtn->GetRef();
        SwFtnBossFrm* pStop = pTmpRef->FindFtnBossFrm( !bEndn );

        const USHORT nNum = pStop->GetPhyPageNum();

        //Wenn die Fussnoten am Dokumentende angezeigt werden, so verlassen wir
        //die Entsprechenden Seiten nicht.
        //Selbiges gilt analog fuer die Endnotenseiten.
        const FASTBOOL bEndNote = pOldPage->IsEndNotePage();
        const FASTBOOL bFtnEndDoc = pOldPage->IsFtnPage();
        SwFtnBossFrm* pNxtBoss = pOldBoss;
        SwSectionFrm *pSect = pNxtBoss->GetUpper()->IsSctFrm() ?
                              (SwSectionFrm*)pNxtBoss->GetUpper() : 0;

        do
        {
            if( pNxtBoss->IsColumnFrm() && pNxtBoss->GetPrev() )
                pNxtBoss = (SwFtnBossFrm*)pNxtBoss->GetPrev();  // eine Spalte zurueck
            else                                // oder eine Seite zurueck
            {
                SwLayoutFrm* pBody;
                if( pSect )
                {
                    if( pSect->IsFtnLock() )
                    {
                        if( pNxtBoss == pOldBoss )
                            return 0;
                        pStop = pNxtBoss;
                    }
                    else
                    {
                        pSect = pSect->FindMaster();
                        if( !pSect || !pSect->Lower() )
                            return 0;
                        ASSERT( pSect->Lower()->IsColumnFrm(),
                                "GetPrevFtnLeaf: Where's the column?" );
                        pNxtBoss = (SwFtnBossFrm*)pSect->Lower();
                        pBody = pSect;
                    }
                }
                else
                {
                    SwPageFrm* pPage = (SwPageFrm*)pNxtBoss->FindPageFrm()->GetPrev();
                    if( !pPage || pPage->GetPhyPageNum() < nNum ||
                        bEndNote != pPage->IsEndNotePage() || bFtnEndDoc != pPage->IsFtnPage() )
                        return NULL; // Keine in Frage kommende Seite mehr gefunden
                    pNxtBoss = pPage;
                    pBody = pPage->FindBodyCont();
                }
                // Die vorherige Seite haben wir nun, ggf. sollten wir in die letzte Spalte
                // der Seite wechseln
                if( pBody )
                {
                    if ( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
                    {
                        pNxtBoss = (SwFtnBossFrm*)pBody->Lower();
                        while( pNxtBoss->GetNext() ) // letzte Spalte suchen
                            pNxtBoss = (SwFtnBossFrm*)pNxtBoss->GetNext();
                    }
                }
            }
            SwFtnContFrm *pCont = pNxtBoss->FindFtnCont();
            if ( pCont )
            {
                pRet = pCont;
                break;
            }
            if ( pStop == pNxtBoss )
            {   //Die Seite/Spalte auf der sich auch die Referenz tummelt, ist erreicht.
                //Wir koennen jetzt probehalber mal einen Container erzeugen und
                //uns hineinpasten.
                if ( eMakeFtn == MAKEPAGE_FTN && pNxtBoss->GetMaxFtnHeight() )
                    pRet = pNxtBoss->MakeFtnCont();
                break;
            }
        } while( !pRet );
    }
    if ( pRet )
    {
        const SwFtnBossFrm* pNewBoss = pRet->FindFtnBossFrm();
        BOOL bJump = FALSE;
        if( pOldBoss->IsColumnFrm() && pOldBoss->GetPrev() ) // es gibt eine vorherige Spalte
            bJump = pOldBoss->GetPrev() != (SwFrm*)pNewBoss;         // sind wir darin gelandet?
        else if( pNewBoss->IsColumnFrm() && pNewBoss->GetNext() )
            bJump = TRUE; // es gibt hinter dem neuen Boss noch eine Spalte, die aber nicht
                          // der alte Boss sein kann, das haben wir ja bereits geprueft.
        else // hier landen wir nur, wenn neuer und alter Boss entweder Seiten oder letzte (neu)
        {   // bzw. erste (alt) Spalten einer Seite sind. In diesem Fall muss noch geprueft
            // werden, ob Seiten ueberspringen wurden.
            USHORT nDiff = pOldPage->GetPhyPageNum() - pRet->FindPageFrm()->GetPhyPageNum();
            if ( nDiff > 2 ||
                 (nDiff > 1 && !((SwPageFrm*)pOldPage->GetPrev())->IsEmptyPage()) )
                bJump = TRUE;
        }
        if( bJump )
            SwFlowFrm::SetMoveBwdJump( TRUE );
    }
    return pRet;
}

/*************************************************************************
|*
|*  SwFrm::IsFtnAllowed()
|*
|*  Ersterstellung      MA 22. Mar. 94
|*  Letzte Aenderung    MA 01. Dec. 94
|*
|*************************************************************************/


BOOL SwFrm::IsFtnAllowed() const
{
    if ( !IsInDocBody() )
        return FALSE;

    if ( IsInTab() )
    {
        //Keine Ftns in wiederholten Headlines.
        const SwTabFrm *pTab = ((SwFrm*)this)->ImplFindTabFrm();
        if ( pTab->GetTable()->IsHeadlineRepeat() && pTab->IsFollow() )
            return !((SwLayoutFrm*)pTab->Lower())->IsAnLower( this );
    }
    return TRUE;
}

/*************************************************************************
|*
|*  SwRootFrm::UpdateFtnNums()
|*
|*  Ersterstellung      MA 02. Mar. 93
|*  Letzte Aenderung    MA 09. Dec. 97
|*
|*************************************************************************/


void SwRootFrm::UpdateFtnNums()
{
    //Seitenweise Numerierung nur wenn es am Dokument so eingestellt ist.
    if ( GetFmt()->GetDoc()->GetFtnInfo().eNum == FTNNUM_PAGE )
    {
        SwPageFrm *pPage = (SwPageFrm*)Lower();
        while ( pPage && !pPage->IsFtnPage() )
        {
            pPage->UpdateFtnNum();
            pPage = (SwPageFrm*)pPage->GetNext();
        }
    }
}

/*************************************************************************
|*
|*  RemoveFtns()        Entfernen aller Fussnoten (nicht etwa die Referenzen)
|*                      und Entfernen aller Fussnotenseiten.
|*
|*  Ersterstellung      MA 05. Dec. 97
|*  Letzte Aenderung    AMA 06. Nov. 98
|*
|*************************************************************************/

void lcl_RemoveFtns( SwFtnBossFrm* pBoss, BOOL bPageOnly, BOOL bEndNotes )
{
    do
    {
        SwFtnContFrm *pCont = pBoss->FindFtnCont();
        if ( pCont )
        {
            SwFtnFrm *pFtn = (SwFtnFrm*)pCont->Lower();
            ASSERT( pFtn, "FtnCont ohne Ftn." );
            if ( bPageOnly )
                while ( pFtn->GetMaster() )
                    pFtn = pFtn->GetMaster();
            do
            {
                SwFtnFrm *pNxt = (SwFtnFrm*)pFtn->GetNext();
                if ( !pFtn->GetAttr()->GetFtn().IsEndNote() ||
                        bEndNotes )
                {
                    pFtn->GetRef()->Prepare( PREP_FTN, (void*)pFtn->GetAttr() );
                    if ( bPageOnly && !pNxt )
                        pNxt = pFtn->GetFollow();
                    pFtn->Cut();
                    delete pFtn;
                }
                pFtn = pNxt;

            } while ( pFtn );
        }
        else if( !pBoss->IsInSct() )
        {
            // A sectionframe with the Ftn/EndnAtEnd-flags may contain
            // foot/endnotes. If the last lower frame of the bodyframe is
            // a multicolumned sectionframe, it may contain footnotes, too.
            SwLayoutFrm* pBody = pBoss->FindBodyCont();
            if( pBody && pBody->Lower() )
            {
                SwFrm* pLow = pBody->Lower();
                while( pLow->GetNext() )
                {
                    if( pLow->IsSctFrm() && ( !pLow->GetNext() ||
                        ((SwSectionFrm*)pLow)->IsAnyNoteAtEnd() ) &&
                        ((SwSectionFrm*)pLow)->Lower() &&
                        ((SwSectionFrm*)pLow)->Lower()->IsColumnFrm() )
                        lcl_RemoveFtns( (SwColumnFrm*)((SwSectionFrm*)pLow)->Lower(),
                            bPageOnly, bEndNotes );
                    pLow = pLow->GetNext();
                }
            }
        }
        // noch 'ne Spalte?
        pBoss = pBoss->IsColumnFrm() ? (SwColumnFrm*)pBoss->GetNext() : NULL;
    } while( pBoss );
}

void SwRootFrm::RemoveFtns( SwPageFrm *pPage, BOOL bPageOnly, BOOL bEndNotes )
{
    if ( !pPage )
        pPage = (SwPageFrm*)Lower();

    do
    {   // Bei spaltigen Seiten muessen wir in allen Spalten aufraeumen
        SwFtnBossFrm* pBoss;
        SwLayoutFrm* pBody = pPage->FindBodyCont();
        if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            pBoss = (SwFtnBossFrm*)pBody->Lower(); // die erste Spalte
        else
            pBoss = pPage; // keine Spalten
        lcl_RemoveFtns( pBoss, bPageOnly, bEndNotes );
        if ( !bPageOnly )
        {
            if ( pPage->IsFtnPage() &&
                 (!pPage->IsEndNotePage() || bEndNotes) )
            {
                SwFrm *pDel = pPage;
                pPage = (SwPageFrm*)pPage->GetNext();
                pDel->Cut();
                delete pDel;
            }
            else
                pPage = (SwPageFrm*)pPage->GetNext();
        }
        else
            break;

    } while ( pPage );
}

/*************************************************************************
|*
|*  SetFtnPageDescs()   Seitenvorlagen der Fussnotenseiten aendern
|*
|*  Ersterstellung      MA 11. Dec. 97
|*  Letzte Aenderung    MA 11. Dec. 97
|*
|*************************************************************************/

void SwRootFrm::CheckFtnPageDescs( BOOL bEndNote )
{
    SwPageFrm *pPage = (SwPageFrm*)Lower();
    while ( pPage && !pPage->IsFtnPage() )
        pPage = (SwPageFrm*)pPage->GetNext();
    while ( pPage && pPage->IsEndNotePage() != bEndNote )
        pPage = (SwPageFrm*)pPage->GetNext();
    if ( pPage )
        SwFrm::CheckPageDescs( pPage, FALSE );
}


/*************************************************************************
|*
|*  SwFtnBossFrm::MakeFtnCont()
|*
|*  Ersterstellung      MA 25. Feb. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


SwFtnContFrm *SwFtnBossFrm::MakeFtnCont()
{
    //Einfuegen eines Fussnotencontainers. Der Fussnotencontainer sitzt
    //immer direkt hinter dem Bodytext.
    //Sein FrmFmt ist immer das DefaultFrmFmt.

#ifndef PRODUCT
    if ( FindFtnCont() )
    {   ASSERT( !this, "Fussnotencontainer bereits vorhanden." );
        return 0;
    }
#endif

    SwFtnContFrm *pNew = new SwFtnContFrm( GetFmt()->GetDoc()->GetDfltFrmFmt());
    SwLayoutFrm *pLay = FindBodyCont();
    pNew->Paste( this, pLay->GetNext() );
    return pNew;
}

/*************************************************************************
|*
|*  SwFtnBossFrm::FindFtnCont()
|*
|*  Ersterstellung      MA 25. Feb. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


SwFtnContFrm *SwFtnBossFrm::FindFtnCont()
{
    SwFrm *pFrm = Lower();
    while( pFrm && !pFrm->IsFtnContFrm() )
        pFrm = pFrm->GetNext();

#ifndef PRODUCT
    if ( pFrm )
    {
        SwFrm *pFtn = pFrm->GetLower();
        ASSERT( pFtn, "Cont ohne Fussnote." );
        while ( pFtn )
        {
            ASSERT( pFtn->IsFtnFrm(), "Nachbar von Fussnote keine Fussnote." );
            pFtn = pFtn->GetNext();
        }
    }
#endif

    return (SwFtnContFrm*)pFrm;
}

/*************************************************************************
|*
|*  SwFtnBossFrm::FindNearestFtnCont()  Sucht den naechst greifbaren Fussnotencontainer.
|*
|*  Ersterstellung      MA 02. Aug. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/

SwFtnContFrm *SwFtnBossFrm::FindNearestFtnCont( BOOL bDontLeave )
{
    SwFtnContFrm *pCont = 0;
    if ( GetFmt()->GetDoc()->GetFtnIdxs().Count() )
    {
        pCont = FindFtnCont();
        if ( !pCont )
        {
            SwPageFrm *pPage = FindPageFrm();
            SwFtnBossFrm* pBoss = this;
            BOOL bEndNote = pPage->IsEndNotePage();
            do
            {
                BOOL bChgPage = lcl_NextFtnBoss( pBoss, pPage, bDontLeave );
                // Haben wir noch einen Boss gefunden? Bei einem Seitenwechsel muss
                // zudem noch das EndNotenFlag uebereinstimmen
                if( pBoss && ( !bChgPage || pPage->IsEndNotePage() == bEndNote ) )
                    pCont = pBoss->FindFtnCont();
            } while ( !pCont && pPage );
        }
    }
    return pCont;
}


/*************************************************************************
|*
|*  SwFtnBossFrm::FindFirstFtn()
|*
|*  Beschreibung        Erste Fussnote des Fussnotenbosses suchen.
|*  Ersterstellung      MA 26. Feb. 93
|*  Letzte Aenderung    AMA 29. Oct. 99
|*
|*************************************************************************/


SwFtnFrm *SwFtnBossFrm::FindFirstFtn()
{
    //Erstmal den naechsten FussnotenContainer suchen.
    SwFtnContFrm *pCont = FindNearestFtnCont();
    if ( !pCont )
        return 0;

    //Ab der ersten Fussnote im Container die erste suchen, die
    //von der aktuellen Spalte (bzw. einspaltigen Seite) referenziert wird.

    SwFtnFrm *pRet = (SwFtnFrm*)pCont->Lower();
    const USHORT nRefNum = FindPageFrm()->GetPhyPageNum();
    const USHORT nRefCol = lcl_ColumnNum( this );
    USHORT nPgNum, nColNum; //Seitennummer, Spaltennummer
    SwFtnBossFrm* pBoss;
    SwPageFrm* pPage;
    if( pRet )
    {
        pBoss = pRet->GetRef()->FindFtnBossFrm();
        ASSERT( pBoss, "FindFirstFtn: No boss found" );
        if( !pBoss )
            return FALSE; // ´There must be a bug, but no GPF
        pPage = pBoss->FindPageFrm();
        nPgNum = pPage->GetPhyPageNum();
        if ( nPgNum == nRefNum )
        {
            nColNum = lcl_ColumnNum( pBoss );
            if( nColNum == nRefCol )
                return pRet; //hat ihn.
            else if( nColNum > nRefCol )
                return NULL; //mind. eine Spalte zu weit.
        }
        else if ( nPgNum > nRefNum )
            return NULL;    //mind. eine Seite zu weit.
    }
    else
        return NULL;
    // Ende, wenn Ref auf einer spaeteren Seite oder auf der gleichen Seite in einer
    // spaeteren Spalte liegt

    do
    {
        while ( pRet->GetFollow() )
            pRet = pRet->GetFollow();

        SwFtnFrm *pNxt = (SwFtnFrm*)pRet->GetNext();
        if ( !pNxt )
        {
            pBoss = pRet->FindFtnBossFrm();
            pPage = pBoss->FindPageFrm();
            lcl_NextFtnBoss( pBoss, pPage, FALSE ); // naechster FtnBoss
            pCont = pBoss ? pBoss->FindNearestFtnCont() : 0;
            if ( pCont )
                pNxt = (SwFtnFrm*)pCont->Lower();
        }
        if ( pNxt )
        {
            pRet = pNxt;
            pBoss = pRet->GetRef()->FindFtnBossFrm();
            pPage = pBoss->FindPageFrm();
            nPgNum = pPage->GetPhyPageNum();
            if ( nPgNum == nRefNum )
            {
                nColNum = lcl_ColumnNum( pBoss );
                if( nColNum == nRefCol )
                    break; //hat ihn.
                else if( nColNum > nRefCol )
                    pRet = 0; //mind. eine Spalte zu weit.
            }
            else if ( nPgNum > nRefNum )
                pRet = 0;   //mind. eine Seite zu weit.
        }
        else
            pRet = 0;   //Gibt eben keinen.
    } while( pRet );
    return pRet;
}

/*************************************************************************
|*
|*  SwFtnBossFrm::FindFirstFtn()
|*
|*  Beschreibunt        Erste Fussnote zum Cnt suchen.
|*  Ersterstellung      MA 04. Mar. 93
|*  Letzte Aenderung    AMA 28. Oct. 98
|*
|*************************************************************************/


const SwFtnFrm *SwFtnBossFrm::FindFirstFtn( SwCntntFrm *pCnt ) const
{
    const SwFtnFrm *pRet = ((SwFtnBossFrm*)this)->FindFirstFtn();
    if ( pRet )
    {
        const USHORT nColNum = lcl_ColumnNum( this ); //Spaltennummer
        const USHORT nPageNum = GetPhyPageNum();
        while ( pRet && (pRet->GetRef() != pCnt) )
        {
            while ( pRet->GetFollow() )
                pRet = pRet->GetFollow();

            if ( pRet->GetNext() )
                pRet = (const SwFtnFrm*)pRet->GetNext();
            else
            {   SwFtnBossFrm *pBoss = (SwFtnBossFrm*)pRet->FindFtnBossFrm();
                SwPageFrm *pPage = pBoss->FindPageFrm();
                lcl_NextFtnBoss( pBoss, pPage, FALSE ); // naechster FtnBoss
                SwFtnContFrm *pCont = pBoss ? pBoss->FindNearestFtnCont() : 0;
                pRet = pCont ? (SwFtnFrm*)pCont->Lower() : 0;
            }
            if ( pRet )
            {
                const SwFtnBossFrm* pBoss = pRet->GetRef()->FindFtnBossFrm();
                if( pBoss->GetPhyPageNum() != nPageNum ||
                    nColNum != lcl_ColumnNum( pBoss ) )
                pRet = 0;
            }
        }
    }
    return pRet;
}

/*************************************************************************
|*
|*  SwFtnBossFrm::ResetFtn()
|*
|*  Ersterstellung      MA 11. May. 95
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


void SwFtnBossFrm::ResetFtn( const SwFtnFrm *pCheck )
{
    //Vernichten der Inkarnationen von Fussnoten zum Attribut, wenn sie nicht
    //zu pAssumed gehoeren.
    ASSERT( !pCheck->GetMaster(), "Master not an Master." );

    SwNodeIndex aIdx( *pCheck->GetAttr()->GetStartNode(), 1 );
    SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
    if ( !pNd )
        pNd = pCheck->GetFmt()->GetDoc()->
              GetNodes().GoNextSection( &aIdx, TRUE, FALSE );
    SwClientIter aIter( *pNd );
    SwClient* pLast = aIter.GoStart();
    while( pLast )
    {
        if ( pLast->ISA(SwFrm) )
        {
            SwFrm *pFrm = (SwFrm*)pLast;
            SwFrm *pTmp = pFrm->GetUpper();
            while ( pTmp && !pTmp->IsFtnFrm() )
                pTmp = pTmp->GetUpper();

            SwFtnFrm *pFtn = (SwFtnFrm*)pTmp;
            while ( pFtn && pFtn->GetMaster() )
                pFtn = pFtn->GetMaster();
            if ( pFtn != pCheck )
            {
                while ( pFtn )
                {
                    SwFtnFrm *pNxt = pFtn->GetFollow();
                    pFtn->Cut();
                    delete pFtn;
                    pFtn = pNxt;
                }
            }
        }
        pLast = ++aIter;
    }
}

/*************************************************************************
|*
|*  SwFtnBossFrm::InsertFtn()
|*
|*  Ersterstellung      MA 26. Feb. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


void SwFtnBossFrm::InsertFtn( SwFtnFrm* pNew )
{
#if defined(DEBUG) && !defined(PRODUCT)
    static USHORT nStop = 0;
    if ( nStop == pNew->GetFrmId() )
    {
        int bla = 5;
    }
#endif
    //Die Fussnote haben wir, sie muss jetzt nur noch irgendwo
    //hin und zwar vor die Fussnote, deren Attribut vor das
    //der neuen zeigt (Position wird ueber das Doc ermittelt)
    //Gibt es in diesem Fussnotenboss noch keine Fussnoten, so muss eben ein
    //Container erzeugt werden.
    //Gibt es bereits einen Container aber noch keine Fussnote zu diesem
    //Fussnotenboss, so muss die Fussnote hinter die letzte Fussnote der dichtesten
    //Vorseite/spalte.

    ResetFtn( pNew );
    SwFtnFrm *pSibling = FindFirstFtn();
    BOOL bDontLeave = FALSE;

    // Ok, a sibling has been found, but is the sibling in an acceptable
    // environment?
    if( IsInSct() )
    {
        SwSectionFrm* pMySect = ImplFindSctFrm();
        BOOL bEndnt = pNew->GetAttr()->GetFtn().IsEndNote();
        if( bEndnt )
        {
            const SwSectionFmt* pEndFmt = pMySect->GetEndSectFmt();
            bDontLeave = 0 != pEndFmt;
            if( pSibling )
            {
                if( pEndFmt )
                {
                    if( !pSibling->IsInSct() ||
                        !pSibling->ImplFindSctFrm()->IsDescendantFrom( pEndFmt ) )
                        pSibling = NULL;
                }
                else if( pSibling->IsInSct() )
                    pSibling = NULL;
            }
        }
        else
        {
            bDontLeave = pMySect->IsFtnAtEnd();
            if( pSibling )
            {
                if( pMySect->IsFtnAtEnd() )
                {
                    if( !pSibling->IsInSct() ||
                        !pMySect->IsAnFollow( pSibling->ImplFindSctFrm() ) )
                        pSibling = NULL;
                }
                else if( pSibling->IsInSct() )
                    pSibling = NULL;
            }
        }
    }

    if( pSibling && pSibling->FindPageFrm()->IsEndNotePage() !=
        FindPageFrm()->IsEndNotePage() )
        pSibling = NULL;

    //Damit die Position herausgefunden werden kann.
    SwDoc *pDoc = GetFmt()->GetDoc();
    const ULONG nStPos = ::lcl_FindFtnPos( pDoc, pNew->GetAttr() );

    ULONG nCmpPos, nLastPos;
    SwFtnContFrm *pParent = 0;
    if( pSibling )
    {
        nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
        if( nCmpPos > nStPos )
            pSibling = NULL;
    }

    if ( !pSibling )
    {   pParent = FindFtnCont();
        if ( !pParent )
        {
            //Es gibt noch keinen FussnotenContainer, also machen wir einen.
            //HAAAAAAAALT! So einfach ist das leider mal wieder nicht: Es kann
            //sein, dass irgendeine naechste Fussnote existiert die vor der
            //einzufuegenden zu stehen hat, weil z.B. eine Fussnote ueber zig
            //Seiten aufgespalten ist usw.
            pParent = FindNearestFtnCont( bDontLeave );
            if ( pParent )
            {
                SwFtnFrm *pFtn = (SwFtnFrm*)pParent->Lower();
                if ( pFtn )
                {

                    nCmpPos = ::lcl_FindFtnPos( pDoc, pFtn->GetAttr() );
                    if ( nCmpPos > nStPos )
                        pParent = 0;
                }
                else
                    pParent = 0;
            }
        }
        if ( !pParent )
            //Jetzt kann aber ein Fussnotencontainer gebaut werden.
            pParent = MakeFtnCont();
        else
        {
            //Ausgehend von der ersten Fussnote unterhalb des Parents wird die
            //erste Fussnote gesucht deren Index hinter dem Index der
            //einzufuegenden liegt; vor dieser kann der neue dann gepastet
            //werden.
            pSibling = (SwFtnFrm*)pParent->Lower();
            if ( !pSibling )
            {   ASSERT( !this, "Keinen Platz fuer Fussnote gefunden.");
                return;
            }
            nCmpPos  = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );

            SwFtnBossFrm *pNxtB = this; //Immer den letzten merken, damit wir nicht
            SwFtnFrm  *pLastSib = 0;    //ueber das Ziel hinausschiessen.

            while ( pSibling && nCmpPos <= nStPos )
            {
                pLastSib = pSibling; // der kommt schon mal in Frage
                nLastPos = nCmpPos;

                while ( pSibling->GetFollow() )
                    pSibling = pSibling->GetFollow();

                if ( pSibling->GetNext() )
                {
                    pSibling = (SwFtnFrm*)pSibling->GetNext();
                    ASSERT( !pSibling->GetMaster() || ( ENDNOTE > nStPos &&
                            pSibling->GetAttr()->GetFtn().IsEndNote() ),
                            "InsertFtn: Master expected I" );
                }
                else
                {
                    pNxtB = pSibling->FindFtnBossFrm();
                    SwPageFrm *pSibPage = pNxtB->FindPageFrm();
                    BOOL bEndNote = pSibPage->IsEndNotePage();
                    BOOL bChgPage = lcl_NextFtnBoss( pNxtB, pSibPage, bDontLeave );
                    // Bei Seitenwechsel muss das EndNoteFlag ueberprueft werden.
                    SwFtnContFrm *pCont = pNxtB && ( !bChgPage ||
                        pSibPage->IsEndNotePage() == bEndNote )
                        ? pNxtB->FindNearestFtnCont( bDontLeave ) : 0;
                    if( pCont )
                        pSibling = (SwFtnFrm*)pCont->Lower();
                    else // kein weiterer FtnContainer, dann werden  wir uns wohl hinter
                        break; // pSibling haengen
                }
                if ( pSibling )
                {
                    nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
                    ASSERT( nCmpPos > nLastPos, "InsertFtn: Order of FtnFrm's buggy" );
                }
            }
            // pLastSib ist jetzt die letzte Fussnote vor uns,
            // pSibling leer oder die erste nach uns.
            if ( pSibling && pLastSib && (pSibling != pLastSib) )
            {   //Sind wir vielleicht bereits ueber das Ziel hinausgeschossen?
                if ( nCmpPos > nStPos )
                    pSibling = pLastSib;
            }
            else if ( !pSibling )
            {   //Eine Chance haben wir noch: wir nehmen einfach die letzte
                //Fussnote im Parent. Ein Sonderfall, der z.B. beim
                //zurueckfliessen von Absaetzen mit mehreren Fussnoten
                //vorkommt.
                //Damit wir nicht die Reihenfolge verwuerfeln muessen wir den
                //Parent der letzten Fussnote, die wir an der Hand hatten benutzen.
                pSibling = pLastSib;
                while( pSibling->GetFollow() )
                    pSibling = pSibling->GetFollow();
                ASSERT( !pSibling->GetNext(), "InsertFtn: Who's that guy?" );
            }
        }
    }
    else
    {   //Die erste Fussnote der Spalte/Seite haben wir an der Hand, jetzt ausgehend
        //von dieser die erste zur selben Spalte/Seite suchen deren Index hinter
        //den uebergebenen zeigt, die letzte, die wir an der Hand hatten, ist
        //dann der Vorgaenger.
        SwFtnBossFrm* pBoss = pNew->GetRef()->FindFtnBossFrm(
            !pNew->GetAttr()->GetFtn().IsEndNote() );
        USHORT nRefNum = pBoss->GetPhyPageNum();    // Die Seiten- und
        USHORT nRefCol = lcl_ColumnNum( pBoss );    // Spaltennummer der neuen Fussnote
        BOOL bEnd = FALSE;
        SwFtnFrm *pLastSib = 0;
        while ( pSibling && !bEnd && (nCmpPos <= nStPos) )
        {
            pLastSib = pSibling;
            nLastPos = nCmpPos;

            while ( pSibling->GetFollow() )
                pSibling = pSibling->GetFollow();

            SwFtnFrm *pFoll = (SwFtnFrm*)pSibling->GetNext();
            if ( pFoll )
            {
                pBoss = pSibling->GetRef()->FindFtnBossFrm( !pSibling->
                                            GetAttr()->GetFtn().IsEndNote() );
                USHORT nTmpRef;
                if( nStPos >= ENDNOTE ||
                    (nTmpRef = pBoss->GetPhyPageNum()) < nRefNum ||
                    ( nTmpRef == nRefNum && lcl_ColumnNum( pBoss ) <= nRefCol ))
                    pSibling = pFoll;
                else
                    bEnd = TRUE;
            }
            else
            {
                SwFtnBossFrm* pNxtB = pSibling->FindFtnBossFrm();
                SwPageFrm *pSibPage = pNxtB->FindPageFrm();
                BOOL bEndNote = pSibPage->IsEndNotePage();
                BOOL bChgPage = lcl_NextFtnBoss( pNxtB, pSibPage, bDontLeave );
                // Bei Seitenwechsel muss das EndNoteFlag ueberprueft werden.
                SwFtnContFrm *pCont = pNxtB && ( !bChgPage ||
                    pSibPage->IsEndNotePage() == bEndNote )
                    ? pNxtB->FindNearestFtnCont( bDontLeave ) : 0;
                if ( pCont )
                    pSibling = (SwFtnFrm*)pCont->Lower();
                else
                    bEnd = TRUE;
            }
            if ( !bEnd && pSibling )
                nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
            if ( pSibling && pLastSib && (pSibling != pLastSib) )
            {   //Sind wir vielleicht bereits ueber das Ziel hinausgeschossen?
                if ( (nLastPos < nCmpPos) && (nCmpPos > nStPos) )
                {
                    pSibling = pLastSib;
                    bEnd = TRUE;
                }
            }
        }
    }
    if ( pSibling )
    {
        nCmpPos = ::lcl_FindFtnPos( pDoc, pSibling->GetAttr() );
        if ( nCmpPos < nStPos )
        {
            while ( pSibling->GetFollow() )
                pSibling = pSibling->GetFollow();
            pParent = (SwFtnContFrm*)pSibling->GetUpper();
            pSibling = (SwFtnFrm*)pSibling->GetNext();
        }
        else
        {
            if( pSibling->GetMaster() )
            {
                if( ENDNOTE > nCmpPos || nStPos >= ENDNOTE )
                {
                    ASSERT( FALSE, "InsertFtn: Master expected II" );
                    do
                        pSibling = pSibling->GetMaster();
                    while ( pSibling->GetMaster() );
                }
            }
            pParent = (SwFtnContFrm*)pSibling->GetUpper();
        }
    }
    ASSERT( pParent, "paste in space?" );
    pNew->Paste( pParent, pSibling );
}

/*************************************************************************
|*
|*  SwFtnBossFrm::AppendFtn()
|*
|*  Ersterstellung      MA 25. Feb. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


void SwFtnBossFrm::AppendFtn( SwCntntFrm *pRef, SwTxtFtn *pAttr )
{
    //Wenn es die Fussnote schon gibt tun wir nix.
    if ( FindFtn( pRef, pAttr ) )
        return;

    //Wenn Fussnoten am Dokumentende eingestellt sind, so brauchen wir 'eh erst
    //ab der entsprechenden Seite zu suchen.
    //Wenn es noch keine gibt, muss eben eine erzeugt werden.
    //Wenn es sich um eine Endnote handelt, muss eine Endnotenseite gesucht
    //bzw. erzeugt werden.
    SwDoc *pDoc = GetFmt()->GetDoc();
    SwFtnBossFrm *pBoss = this;
    SwPageFrm *pPage = FindPageFrm();
    SwPageFrm *pMyPage = pPage;
    BOOL bChgPage = FALSE;
    BOOL bEnd = FALSE;
    if ( pAttr->GetFtn().IsEndNote() )
    {
        bEnd = TRUE;
        if( GetUpper()->IsSctFrm() &&
            ((SwSectionFrm*)GetUpper())->IsEndnAtEnd() )
        {
            SwFrm* pLast =
                ((SwSectionFrm*)GetUpper())->FindLastCntnt( FINDMODE_ENDNOTE );
            if( pLast )
            {
                pBoss = pLast->FindFtnBossFrm();
                pPage = pBoss->FindPageFrm();
            }
        }
        else
        {
            while ( pPage->GetNext() && !pPage->IsEndNotePage() )
            {
                pPage = (SwPageFrm*)pPage->GetNext();
                bChgPage = TRUE;
            }
            if ( !pPage->IsEndNotePage() )
            {
                SwPageDesc *pDesc = pDoc->GetEndNoteInfo().GetPageDesc( *pDoc );
                pPage = ::InsertNewPage( *pDesc, pPage->GetUpper(),
                        !pPage->OnRightPage(), FALSE, TRUE, 0 );
                pPage->SetEndNotePage( TRUE );
                bChgPage = TRUE;
            }
            else
            {
                //Wir koennen wenigstens schon mal ungefaehr die richtige Seite
                //suchen. Damit stellen wir sicher das wir auch bei hunderten
                //Fussnoten noch in endlicher Zeit fertig werden.
                SwPageFrm *pNxt = (SwPageFrm*)pPage->GetNext();
                const ULONG nStPos = ::lcl_FindFtnPos( pDoc, pAttr );
                while ( pNxt && pNxt->IsEndNotePage() )
                {
                    SwFtnContFrm *pCont = pNxt->FindFtnCont();
                    if ( pCont && pCont->Lower() )
                    {
                        ASSERT( pCont->Lower()->IsFtnFrm(), "Keine Ftn im Container" );
                        if ( nStPos > ::lcl_FindFtnPos( pDoc,
                                        ((SwFtnFrm*)pCont->Lower())->GetAttr()))
                        {
                            pPage = pNxt;
                            pNxt = (SwPageFrm*)pPage->GetNext();
                            continue;
                        }
                    }
                    break;
                }
            }
        }
    }
    else if( FTNPOS_CHAPTER == pDoc->GetFtnInfo().ePos && ( !GetUpper()->
             IsSctFrm() || !((SwSectionFrm*)GetUpper())->IsFtnAtEnd() ) )
    {
        while ( pPage->GetNext() && !pPage->IsFtnPage() &&
                !((SwPageFrm*)pPage->GetNext())->IsEndNotePage() )
        {
            pPage = (SwPageFrm*)pPage->GetNext();
            bChgPage = TRUE;
        }

        if ( !pPage->IsFtnPage() )
        {
            SwPageDesc *pDesc = pDoc->GetFtnInfo().GetPageDesc( *pDoc );
            pPage = ::InsertNewPage( *pDesc, pPage->GetUpper(),
                !pPage->OnRightPage(), FALSE, TRUE, pPage->GetNext() );
            bChgPage = TRUE;
        }
        else
        {
            //Wir koennen wenigstens schon mal ungefaehr die richtige Seite
            //suchen. Damit stellen wir sicher das wir auch bei hunderten
            //Fussnoten noch in endlicher Zeit fertig werden.
            SwPageFrm *pNxt = (SwPageFrm*)pPage->GetNext();
            const ULONG nStPos = ::lcl_FindFtnPos( pDoc, pAttr );
            while ( pNxt && pNxt->IsFtnPage() && !pNxt->IsEndNotePage() )
            {
                SwFtnContFrm *pCont = pNxt->FindFtnCont();
                if ( pCont && pCont->Lower() )
                {
                    ASSERT( pCont->Lower()->IsFtnFrm(), "Keine Ftn im Container" );
                    if ( nStPos > ::lcl_FindFtnPos( pDoc,
                                        ((SwFtnFrm*)pCont->Lower())->GetAttr()))
                    {
                        pPage = pNxt;
                        pNxt = (SwPageFrm*)pPage->GetNext();
                        continue;
                    }
                }
                break;
            }
        }
    }

    //Erstmal eine Fussnote und die benoetigten CntntFrms anlegen.
    if ( !pAttr->GetStartNode() )
    {   ASSERT( !this, "Kein Fussnoteninhalt." );
        return;
    }

    // Wenn es auf der Seite/Spalte bereits einen FtnCont gibt,
    // kann in einen spaltigen Bereich keiner erzeugt werden.
    if( pBoss->IsInSct() && pBoss->IsColumnFrm() && !pPage->IsFtnPage() )
    {
        SwSectionFrm* pSct = pBoss->FindSctFrm();
        if( bEnd ? !pSct->IsEndnAtEnd() : !pSct->IsFtnAtEnd() )
        {
            SwFtnContFrm* pFtnCont = pSct->FindFtnBossFrm(!bEnd)->FindFtnCont();
            if( pFtnCont )
            {
                SwFtnFrm* pTmp = (SwFtnFrm*)pFtnCont->Lower();
                if( bEnd )
                    while( pTmp && !pTmp->GetAttr()->GetFtn().IsEndNote() )
                        pTmp = (SwFtnFrm*)pTmp->GetNext();
                if( pTmp && *pTmp < pAttr )
                    return;
            }
        }
    }

    SwFtnFrm *pNew = new SwFtnFrm( pDoc->GetDfltFrmFmt(), pRef, pAttr );
    {
        SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
        ::_InsertCnt( pNew, pDoc, aIdx.GetIndex() );
    }
    // Wenn die Seite gewechselt (oder gar neu angelegt) wurde,
    // muessen wir uns dort in die erste Spalte setzen
    if( bChgPage )
    {
        SwLayoutFrm* pBody = pPage->FindBodyCont();
        ASSERT( pBody, "AppendFtn: NoPageBody?" );
        if( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            pBoss = (SwFtnBossFrm*)pBody->Lower();
        else
            pBoss = pPage; // bei nichtspaltigen Seiten auf die Seite selbst
    }
    pBoss->InsertFtn( pNew );
    if ( pNew->GetUpper() )         //Eingesetzt oder nicht?
    {
        ::RegistFlys( pNew->FindPageFrm(), pNew );
        SwSectionFrm* pSect = FindSctFrm();
        // Der Inhalt des FtnContainers in einem (spaltigen) Bereich
        // braucht nur kalkuliert zu werden,
        // wenn der Bereich bereits bis zur Unterkante seines Uppers geht.
        if( pSect && !pSect->IsJoinLocked() && ( bEnd ? !pSect->IsEndnAtEnd() :
            !pSect->IsFtnAtEnd() ) && pSect->Growable() )
            pSect->InvalidateSize();
        else
        {
            SwCntntFrm *pCnt = pNew->ContainsCntnt();
            while ( pCnt && pCnt->FindFtnFrm()->GetAttr() == pAttr )
            {
                pCnt->Calc();
                pCnt = (SwCntntFrm*)pCnt->FindNextCnt();
            }
        }
        pMyPage->UpdateFtnNum();
    }
    else
        delete pNew;
}
/*************************************************************************
|*
|*  SwFtnBossFrm::FindFtn()
|*
|*  Ersterstellung      MA 25. Feb. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


SwFtnFrm *SwFtnBossFrm::FindFtn( const SwCntntFrm *pRef, const SwTxtFtn *pAttr )
{
    //Der einfachste und sicherste Weg geht ueber das Attribut.
    ASSERT( pAttr->GetStartNode(), "FtnAtr ohne StartNode." );
    SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
    SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
    if ( !pNd )
        pNd = pRef->GetAttrSet()->GetDoc()->
              GetNodes().GoNextSection( &aIdx, TRUE, FALSE );
    if ( !pNd )
        return 0;
    SwClientIter aIter( *pNd );
    SwClient *pClient;
    if ( 0 != (pClient = aIter.GoStart()) )
        do
        {
            if ( pClient->IsA( TYPE(SwFrm) ) )
            {
                SwFrm *pFrm = ((SwFrm*)pClient)->GetUpper();
                SwFtnFrm *pFtn = pFrm->FindFtnFrm();
                if ( pFtn && pFtn->GetRef() == pRef )
                {
                    // The following condition becomes true, if the whole
                    // footnotecontent is a section. While no frames exist,
                    // the HiddenFlag of the section is set, this causes
                    // the GoNextSection-function leaves the footnote.
                    if( pFtn->GetAttr() != pAttr )
                        return 0;
                    while ( pFtn && pFtn->GetMaster() )
                        pFtn = pFtn->GetMaster();
                    return pFtn;
                }
            }
        } while ( 0 != (pClient = aIter++) );

    return 0;
}
/*************************************************************************
|*
|*  SwFtnBossFrm::RemoveFtn()
|*
|*  Ersterstellung      MA 25. Feb. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


void SwFtnBossFrm::RemoveFtn( const SwCntntFrm *pRef, const SwTxtFtn *pAttr,
                              BOOL bPrep )
{
    SwFtnFrm *pFtn = FindFtn( pRef, pAttr );
    if( pFtn )
    {
        do
        {
            SwFtnFrm *pFoll = pFtn->GetFollow();
            pFtn->Cut();
            delete pFtn;
            pFtn = pFoll;
        } while ( pFtn );
        if( bPrep && pRef->IsFollow() )
        {
            ASSERT( pRef->IsTxtFrm(), "NoTxtFrm has Footnote?" );
            SwTxtFrm* pMaster = (SwTxtFrm*)pRef->FindMaster();
            if( !pMaster->IsLocked() )
                pMaster->Prepare( PREP_FTN_GONE );
        }
    }
    FindPageFrm()->UpdateFtnNum();
}

/*************************************************************************
|*
|*  SwFtnBossFrm::ChangeFtnRef()
|*
|*  Ersterstellung      MA 25. Feb. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


void SwFtnBossFrm::ChangeFtnRef( const SwCntntFrm *pOld, const SwTxtFtn *pAttr,
                                 SwCntntFrm *pNew )
{
    SwFtnFrm *pFtn = FindFtn( pOld, pAttr );
    while ( pFtn )
    {
        pFtn->SetRef( pNew );
        pFtn = pFtn->GetFollow();
    }
}

/*************************************************************************
|*
|*  SwFtnBossFrm::CollectFtns()
|*
|*  Ersterstellung      MA 24. Jul. 95
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


void SwFtnBossFrm::CollectFtns( const SwCntntFrm *pRef, SwFtnBossFrm *pOld,
                                SvPtrarr &rFtnArr )
{
    SwFtnFrm *pFtn = pOld->FindFirstFtn();
    while( !pFtn )
    {
        if( pOld->IsColumnFrm() )
        {   // Spalten abklappern
            while ( !pFtn && pOld->GetPrev() )
            {
                //Wenn wir keine Fussnote gefunden haben, ist noch nicht alles zu
                //spaet. Die Schleife wird beim Aufnehmen von Follow-Zeilen durch
                //Tabellen benoetigt. Fuer alle anderen Faelle ist sie in der Lage
                //'krumme' Verhaeltnisse zu korrigieren.
                pOld = (SwFtnBossFrm*)pOld->GetPrev();
                pFtn = pOld->FindFirstFtn();
            }
        }
        if( !pFtn )
        {
            //  vorherige Seite
            SwPageFrm* pPg;
            for( SwFrm* pTmp = pOld;
                    0 != ( pPg = (SwPageFrm*)pTmp->FindPageFrm()->GetPrev())
                    && pPg->IsEmptyPage() ; )
                pTmp = pPg;
            if( !pPg )
                return;

            SwLayoutFrm* pBody = pPg->FindBodyCont();
            if( pBody->Lower() && pBody->Lower()->IsColumnFrm() )
            {   // mehrspaltige Seite => letzte Spalte suchen
                pOld = (SwFtnBossFrm*)pBody->Lower();
                while ( pOld->GetNext() )
                    pOld = (SwFtnBossFrm*)pOld->GetNext();
            }
            else
                pOld = pPg; // einspaltige Seite
            pFtn = pOld->FindFirstFtn();
        }
    }
    _CollectFtns( pRef, pFtn, rFtnArr );
}


/*************************************************************************
|*
|*  SwFtnBossFrm::_CollectFtns()
|*
|*  Ersterstellung      MA 24. Jul. 95
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/
inline void FtnInArr( SvPtrarr& rFtnArr, SwFtnFrm* pFtn )
{
    if ( USHRT_MAX == rFtnArr.GetPos( (VoidPtr)pFtn ) )
        rFtnArr.Insert( (VoidPtr)pFtn, rFtnArr.Count() );
}

void SwFtnBossFrm::_CollectFtns( const SwCntntFrm *pRef, SwFtnFrm *pFtn,
                                 SvPtrarr &rFtnArr )
{
    //Alle Fussnoten die von pRef referenziert werden nacheinander
    //einsammeln (Attribut fuer Attribut), zusammengefuegen
    //(der Inhalt zu einem Attribut kann ueber mehrere Seiten verteilt sein)
    //und ausschneiden.

    SvPtrarr aNotFtnArr( 20, 20 );  //Zur Robustheit werden hier die nicht
                                    //dazugehoerigen Fussnoten eingetragen.
                                    //Wenn eine Fussnote zweimal angefasst wird
                                    //ists vorbei! So kommt die Funktion auch
                                    //noch mit einem kaputten Layout
                                    //einigermassen (ohne Schleife und Absturz)
                                    //"klar".

    //Hier sollte keiner mit einer Follow-Ftn ankommen, es sei denn er hat
    //ernste Absichten (:-)); spricht er kommt mit einer Ftn an die vor der
    //ersten der Referenz liegt.
    ASSERT( !pFtn->GetMaster() || pFtn->GetRef() != pRef, "FollowFtn moven?" );
    while ( pFtn->GetMaster() )
        pFtn = pFtn->GetMaster();

    BOOL bFound = FALSE;

    while ( pFtn )
    {
        //Erstmal die naechste Fussnote der Spalte/Seite suchen, damit wir nicht
        //nach dem Cut jeder Fussnote von vorn anfangen muessen.
        SwFtnFrm *pNxtFtn = pFtn;
        while ( pNxtFtn->GetFollow() )
            pNxtFtn = pNxtFtn->GetFollow();
        pNxtFtn = (SwFtnFrm*)pNxtFtn->GetNext();

        if ( !pNxtFtn )
        {
            SwFtnBossFrm* pBoss = pFtn->FindFtnBossFrm();
            SwPageFrm* pPage = pBoss->FindPageFrm();
            do
            {
                lcl_NextFtnBoss( pBoss, pPage, FALSE );
                if( pBoss )
                {
                    SwLayoutFrm* pCont = pBoss->FindFtnCont();
                    if( pCont )
                    {
                        pNxtFtn = (SwFtnFrm*)pCont->Lower();
                        if( pNxtFtn )
                        {
                            while( pNxtFtn->GetMaster() )
                                pNxtFtn = pNxtFtn->GetMaster();
                            if( pNxtFtn == pFtn )
                                pNxtFtn = NULL;
                        }
                    }
                }
            } while( !pNxtFtn && pBoss );
        }
        else if( !pNxtFtn->GetAttr()->GetFtn().IsEndNote() )
        {   ASSERT( !pNxtFtn->GetMaster(), "_CollectFtn: Master exspected" );
            while ( pNxtFtn->GetMaster() )
                pNxtFtn = pNxtFtn->GetMaster();
        }
        if ( pNxtFtn == pFtn )
        {
            ASSERT( FALSE, "_CollectFtn: Devil's circle" );
            pNxtFtn = 0;
        }

        if ( pFtn->GetRef() == pRef && !pFtn->GetAttr()->GetFtn().IsEndNote() )
        {
            ASSERT( !pFtn->GetMaster(), "FollowFtn moven?" );
            SwFtnFrm *pNxt = pFtn->GetFollow();
            while ( pNxt )
            {
                SwFrm *pCnt = pNxt->ContainsAny();
                if ( pCnt )
                {   //Unterwegs wird der Follow zerstoert weil er leer wird!
                    do
                    {   SwFrm *pNxtCnt = pCnt->GetNext();
                        pCnt->Cut();
                        pCnt->Paste( pFtn );
                        pCnt = pNxtCnt;
                    } while ( pCnt );
                }
                else
                {   ASSERT( !pNxt, "Fussnote ohne Inhalt?" );
                    pNxt->Cut();
                    delete pNxt;
                }
                pNxt = pFtn->GetFollow();
            }
            pFtn->Cut();
            FtnInArr( rFtnArr, pFtn );
            bFound = TRUE;
        }
        else
        {
            FtnInArr( aNotFtnArr, pFtn );
            if( bFound )
                break;
        }
        if ( pNxtFtn &&
             USHRT_MAX == rFtnArr.GetPos( (VoidPtr)pNxtFtn ) &&
             USHRT_MAX == aNotFtnArr.GetPos( (VoidPtr)pNxtFtn ) )
            pFtn = pNxtFtn;
        else
            break;
    }
}

/*************************************************************************
|*
|*  SwFtnBossFrm::_MoveFtns()
|*
|*  Ersterstellung      MA 26. Feb. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


void SwFtnBossFrm::_MoveFtns( SvPtrarr &rFtnArr, BOOL bCalc )
{
    //Alle Fussnoten die von pRef referenziert werden muessen von der
    //aktuellen Position, die sich durch die alte Spalte/Seite ergab, auf eine
    //neue Position, bestimmt durch die neue Spalte/Seite, gemoved werden.
    const USHORT nMyNum = FindPageFrm()->GetPhyPageNum();
    const USHORT nMyCol = lcl_ColumnNum( this );

    for ( USHORT i = 0; i < rFtnArr.Count(); ++i )
    {
        SwFtnFrm *pFtn = (SwFtnFrm*)rFtnArr[i];

        SwFtnBossFrm* pRefBoss = pFtn->GetRef()->FindFtnBossFrm( TRUE );
        if( pRefBoss != this )
        {
            const USHORT nRefNum = pRefBoss->FindPageFrm()->GetPhyPageNum();
            const USHORT nRefCol = lcl_ColumnNum( this );
            if( nRefNum < nMyNum || ( nRefNum == nMyNum && nRefCol <= nMyCol ) )
                pRefBoss = this;
        }
        pRefBoss->InsertFtn( pFtn );

        if ( pFtn->GetUpper() ) //Robust, z.B. bei doppelten
        {
            // Damit FtnFrms, die nicht auf die Seite passen, nicht fuer zuviel
            // Unruhe sorgen (Loop 66312), wird ihr Inhalt zunaechst zusammengestaucht.
            // Damit wird der FtnCont erst gegrowt, wenn der Inhalt formatiert wird
            // und feststellt, dass er auf die Seite passt.
            SwFrm *pCnt = pFtn->ContainsAny();
            while( pCnt )
            {
                if( pCnt->IsLayoutFrm() )
                {
                    SwFrm* pTmp = ((SwLayoutFrm*)pCnt)->ContainsAny();
                    while( pTmp && ((SwLayoutFrm*)pCnt)->IsAnLower( pTmp ) )
                    {
                        pTmp->Prepare( PREP_MOVEFTN );
                        pTmp->Frm().Height(0);
                        pTmp->Prt().Height(0);
                        pTmp = pTmp->FindNext();
                    }
                }
                else
                    pCnt->Prepare( PREP_MOVEFTN );
                pCnt->Frm().Height(0);
                pCnt->Prt().Height(0);
                pCnt = pCnt->GetNext();
            }
            pFtn->Frm().Height(0);
            pFtn->Prt().Height(0);
            pFtn->Calc();
            pFtn->GetUpper()->Calc();

            if( bCalc )
            {
                SwTxtFtn *pAttr = pFtn->GetAttr();
                pCnt = pFtn->ContainsAny();
                pFtn->LockBackMove();

                while ( pCnt && pCnt->FindFtnFrm()->GetAttr() == pAttr )
                {
                    pCnt->_InvalidatePos();
                    pCnt->Calc();
                    if( pCnt->IsSctFrm() )
                    {   // Wenn es sich um einen nichtleeren Bereich handelt,
                        // iterieren wir auch ueber seinen Inhalt
                        SwFrm* pTmp = ((SwSectionFrm*)pCnt)->ContainsAny();
                        if( pTmp )
                            pCnt = pTmp;
                        else
                            pCnt = pCnt->FindNext();
                    }
                    else
                        pCnt = pCnt->FindNext();
                }
                pFtn->UnlockBackMove();
            }
        }
        else
        {   ASSERT( !pFtn->GetMaster() && !pFtn->GetFollow(),
                    "DelFtn und Master/Follow?" );
            delete pFtn;
        }
    }
}

/*************************************************************************
|*
|*  SwFtnBossFrm::MoveFtns()
|*
|*  Ersterstellung      BP 05. Aug. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


void SwFtnBossFrm::MoveFtns( const SwCntntFrm *pSrc, SwCntntFrm *pDest,
                             SwTxtFtn *pAttr )
{
    if( ( GetFmt()->GetDoc()->GetFtnInfo().ePos == FTNPOS_CHAPTER &&
        (!GetUpper()->IsSctFrm() || !((SwSectionFrm*)GetUpper())->IsFtnAtEnd()))
        || pAttr->GetFtn().IsEndNote() )
        return;

    ASSERT( this == pSrc->FindFtnBossFrm( TRUE ),
            "SwPageFrm::MoveFtns: source frame isn't on that FtnBoss" );

    SwFtnFrm *pFtn = FindFirstFtn();
    if( pFtn )
    {
        ChangeFtnRef( pSrc, pAttr, pDest );
        SwFtnBossFrm *pDestBoss = pDest->FindFtnBossFrm( TRUE );
        ASSERT( pDestBoss, "+SwPageFrm::MoveFtns: no destination boss" );
        if( pDestBoss )     // robust
        {
            SvPtrarr aFtnArr( 5, 5 );
            pDestBoss->_CollectFtns( pDest, pFtn, aFtnArr );
            if ( aFtnArr.Count() )
            {
                pDestBoss->_MoveFtns( aFtnArr, TRUE );
                SwPageFrm* pSrcPage = FindPageFrm();
                SwPageFrm* pDestPage = pDestBoss->FindPageFrm();
                // Nur beim Seitenwechsel FtnNum Updaten
                if( pSrcPage != pDestPage )
                {
                    if( pSrcPage->GetPhyPageNum() > pDestPage->GetPhyPageNum() )
                        pSrcPage->UpdateFtnNum();
                    pDestPage->UpdateFtnNum();
                }
            }
        }
    }
}

/*************************************************************************
|*
|*  SwFtnBossFrm::RearrangeFtns()
|*
|*  Ersterstellung      MA 20. Jan. 94
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/


void SwFtnBossFrm::RearrangeFtns( const SwTwips nDeadLine, const BOOL bLock,
                                  const SwTxtFtn *pAttr )
{
    //Alle Fussnoten der Spalte/Seite dergestalt anformatieren,
    //dass sie ggf. die Spalte/Seite wechseln.

    SwSaveFtnHeight aSave( this, nDeadLine );
    SwFtnFrm *pFtn = FindFirstFtn();
    if( pFtn && pFtn->GetPrev() && bLock )
    {
        SwFtnFrm* pFirst = (SwFtnFrm*)pFtn->GetUpper()->Lower();
        SwFrm* pCntnt = pFirst->ContainsAny();
        if( pCntnt )
        {
            pFirst->LockBackMove();
            pFirst->Calc();
            pCntnt->Calc();
            pFirst->UnlockBackMove();
        }
        pFtn = FindFirstFtn();
    }
    SwDoc *pDoc = GetFmt()->GetDoc();
    const ULONG nFtnPos = pAttr ? ::lcl_FindFtnPos( pDoc, pAttr ) : 0;
    SwFrm *pCnt = pFtn ? pFtn->ContainsAny() : 0;
    if ( pCnt )
    {
        BOOL bMore = TRUE;
        BOOL bStart = pAttr == 0; // wenn kein Attribut uebergeben wird, alle bearbeiten
        do
        {
            if( !bStart )
                bStart = ::lcl_FindFtnPos( pDoc, pCnt->FindFtnFrm()->GetAttr() )
                         == nFtnPos;
            if( bStart )
            {   pCnt->_InvalidatePos();
                pCnt->_InvalidateSize();
                pCnt->Prepare( PREP_ADJUST_FRM );
                SwFtnFrm* pTmp = pCnt->FindFtnFrm();
                if ( bLock )
                {
                    pTmp->LockBackMove();
                    pTmp->Calc();
                    pCnt->Calc();
                    pTmp->UnlockBackMove();
                }
                else
                {
                    pTmp->Calc();
                    pCnt->Calc();
                }
            }
            SwSectionFrm *pDel = NULL;
            if( pCnt->IsSctFrm() )
            {
                SwFrm* pTmp = ((SwSectionFrm*)pCnt)->ContainsAny();
                if( pTmp )
                {
                    pCnt = pTmp;
                    continue;
                }
                pDel = (SwSectionFrm*)pCnt;
            }
            if ( pCnt->GetNext() )
                pCnt = pCnt->GetNext();
            else
            {
                pCnt = pCnt->FindNext();
                if ( pCnt )
                {
                    SwFtnFrm* pFtn = pCnt->FindFtnFrm();
                    if( pFtn->GetRef()->FindFtnBossFrm(
                        pFtn->GetAttr()->GetFtn().IsEndNote() ) != this )
                        bMore = FALSE;
                }
                else
                    bMore = FALSE;
            }
            if( pDel )
            {
                pDel->Cut();
                delete pDel;
            }
            if ( bMore )
            {
                //Nicht weiter als bis zur angegebenen Fussnote, falls eine
                //angegeben wurde.
                if ( pAttr &&
                     (::lcl_FindFtnPos( pDoc,
                                    pCnt->FindFtnFrm()->GetAttr()) > nFtnPos ) )
                    bMore = FALSE;
            }
        } while ( bMore );
    }
}

/*************************************************************************
|*
|*  SwPageFrm::UpdateFtnNum()
|*
|*  Ersterstellung      MA 02. Mar. 93
|*  Letzte Aenderung    AMA 29. Oct. 98
|*
|*************************************************************************/

void SwPageFrm::UpdateFtnNum()
{
    //Seitenweise Numerierung nur wenn es am Dokument so eingestellt ist.
    if ( GetFmt()->GetDoc()->GetFtnInfo().eNum != FTNNUM_PAGE )
        return;

    SwLayoutFrm* pBody = FindBodyCont();
    if( !pBody || !pBody->Lower() )
        return;

    SwCntntFrm* pCntnt = pBody->ContainsCntnt();
    USHORT nNum = 0;

    while( pCntnt && pCntnt->FindPageFrm() == this )
    {
        if( ((SwTxtFrm*)pCntnt)->HasFtn() )
        {
            SwFtnBossFrm* pBoss = pCntnt->FindFtnBossFrm( TRUE );
            if( pBoss->GetUpper()->IsSctFrm() &&
                ((SwSectionFrm*)pBoss->GetUpper())->IsOwnFtnNum() )
                pCntnt = ((SwSectionFrm*)pBoss->GetUpper())->FindLastCntnt();
            else
            {
                SwFtnFrm* pFtn = (SwFtnFrm*)pBoss->FindFirstFtn( pCntnt );
                while( pFtn )
                {
                    SwTxtFtn* pTxtFtn = pFtn->GetAttr();
                    if( !pTxtFtn->GetFtn().IsEndNote() &&
                         !pTxtFtn->GetFtn().GetNumStr().Len() &&
                         !pFtn->GetMaster() &&
                         (pTxtFtn->GetFtn().GetNumber() != ++nNum) )
                        pTxtFtn->SetNumber( nNum );
                    if ( pFtn->GetNext() )
                        pFtn = (SwFtnFrm*)pFtn->GetNext();
                    else
                    {
                        SwFtnBossFrm* pBoss = pFtn->FindFtnBossFrm( TRUE );
                        SwPageFrm* pPage = pBoss->FindPageFrm();
                        pFtn = NULL;
                        lcl_NextFtnBoss( pBoss, pPage, FALSE );
                        if( pBoss )
                        {
                            SwFtnContFrm *pCont = pBoss->FindNearestFtnCont();
                            if ( pCont )
                                pFtn = (SwFtnFrm*)pCont->Lower();
                        }
                    }
                    if( pFtn && pFtn->GetRef() != pCntnt )
                        pFtn = NULL;
                }
            }
        }
        pCntnt = pCntnt->FindNextCnt();
    }
}

/*************************************************************************
|*
|*  SwFtnBossFrm::SetFtnDeadLine()
|*
|*  Ersterstellung      MA 02. Aug. 93
|*  Letzte Aenderung    MA 16. Nov. 98
|*
|*************************************************************************/

void SwFtnBossFrm::SetFtnDeadLine( const SwTwips nDeadLine )
{
    SwFrm *pBody = FindBodyCont();
    pBody->Calc();

    SwFrm *pCont = FindFtnCont();
    const SwTwips nMax = nMaxFtnHeight;//Aktuelle MaxHeight nicht ueberschreiten.
    if ( pCont )
    {
        pCont->Calc();
        nMaxFtnHeight = pCont->Frm().Height() + pCont->Frm().Top() - nDeadLine;
    }
    else
        nMaxFtnHeight = pBody->Frm().Top() + pBody->Frm().Height() - nDeadLine;

    if ( GetFmt()->GetDoc()->IsBrowseMode() )
        nMaxFtnHeight += pBody->Grow( LONG_MAX, pHeight, TRUE );
    if ( IsInSct() )
        nMaxFtnHeight += FindSctFrm()->Grow( LONG_MAX, pHeight, TRUE );

    if ( nMaxFtnHeight < 0 )
        nMaxFtnHeight = 0;
    if ( nMax != LONG_MAX && nMaxFtnHeight > nMax )
        nMaxFtnHeight = nMax;
}

/*************************************************************************
|*
|*  SwFtnBossFrm::GetVarSpace()
|*
|*  Ersterstellung      MA 03. Apr. 95
|*  Letzte Aenderung    MA 16. Nov. 98
|*
|*************************************************************************/
SwTwips SwFtnBossFrm::GetVarSpace() const
{
    //Fuer Seiten soll ein Wert von 20% der Seitenhoehe nicht unterschritten
    //werden (->AMA: was macht MS da?)
    //->AMA: Was ist da fuer Bereiche sinnvoll (und kompatibel zu MS ;-)?
    //AMA: MS kennt scheinbar kein Begrenzung, die Fussnoten nehmen durchaus
    // die ganze Seite/Spalte ein.

    const SwPageFrm* pPg = FindPageFrm();
    ASSERT( pPg, "Footnote lost page" );

    const SwFrm *pBody = FindBodyCont();
    SwTwips nRet;
    if( pBody )
    {
        if( IsInSct() )
        {
            nRet = 0;
            SwTwips nTmp = pBody->Frm().Top() + pBody->Prt().Top() -Frm().Top();
            const SwSectionFrm* pSect = FindSctFrm();
            //  Endnotes in a ftncontainer causes a deadline:
            // the bottom of the last contentfrm
            if( pSect->IsEndnAtEnd() ) // endnotes allowed?
            {
                ASSERT( !Lower() || !Lower()->GetNext() || Lower()->GetNext()->
                        IsFtnContFrm(), "FtnContainer exspected" );
                const SwFtnContFrm* pCont = Lower() ?
                    (SwFtnContFrm*)Lower()->GetNext() : 0;
                if( pCont )
                {
                    SwFtnFrm* pFtn = (SwFtnFrm*)pCont->Lower();
                    while( pFtn)
                    {
                        if( pFtn->GetAttr()->GetFtn().IsEndNote() )
                        { // endnote found
                            SwFrm* pFrm = ((SwLayoutFrm*)Lower())->Lower();
                            if( pFrm )
                            {
                                while( pFrm->GetNext() )
                                    pFrm = pFrm->GetNext(); // last cntntfrm
                                nTmp += Frm().Top() - pFrm->Frm().Top()
                                        - pFrm->Frm().Height();
                            }
                            break;
                        }
                        pFtn = (SwFtnFrm*)pFtn->GetNext();
                    }
                }
            }
            if( nTmp < nRet )
                nRet = nTmp;
        }
        else
            nRet = - pPg->Prt().Height()/5;
        nRet += pBody->Frm().Height();
        if( nRet < 0 )
            nRet = 0;
    }
    else
        nRet = 0;
    if ( IsPageFrm() && GetFmt()->GetDoc()->IsBrowseMode() )
        nRet += BROWSE_HEIGHT - Frm().Height();
    return nRet;
}

/*************************************************************************
|*
|*  SwFtnBossFrm::NeighbourhoodAdjustment(SwFrm*)
|*
|*  gibt Auskunft, ob die Groessenveraenderung von pFrm von AdjustNeighbourhood(...)
|*  oder von Grow/Shrink(..) verarbeitet werden sollte.
|*  Bei einem PageFrm oder in Spalten direkt unterhalb der Seite muss AdjustNei..
|*  gerufen werden, in Rahmenspalten Grow/Shrink.
|*  Spannend sind die spaltigen Bereiche: Wenn es in der Spalte einen Fussnotencontainer
|*  gibt und die Fussnoten nicht vom Bereich eingesammelt werden, ist ein Adjust..,
|*  ansonsten ein Grow/Shrink notwendig.
|*
|*  Ersterstellung      AMA 09. Dec 98
|*  Letzte Aenderung    AMA 09. Dec 98
|*
|*************************************************************************/

BYTE SwFtnBossFrm::_NeighbourhoodAdjustment( const SwFrm* pFrm ) const
{
    BYTE nRet = NA_ONLY_ADJUST;
    if( GetUpper() && !GetUpper()->IsPageBodyFrm() )
    {
        // Spaltige Rahmen erfordern Grow/Shrink
        if( GetUpper()->IsFlyFrm() )
            nRet = NA_GROW_SHRINK;
        else
        {
            ASSERT( GetUpper()->IsSctFrm(), "NeighbourhoodAdjustment: Unexspected Upper" );
            if( !GetNext() && !GetPrev() )
                nRet = NA_GROW_ADJUST; // section with a single column (FtnAtEnd)
            else
            {
                const SwFrm* pTmp = Lower();
                ASSERT( pTmp, "NeighbourhoodAdjustment: Missing Lower()" );
                if( !pTmp->GetNext() )
                    nRet = NA_GROW_SHRINK;
                else if( !GetUpper()->IsColLocked() )
                    nRet = NA_ADJUST_GROW;
                ASSERT( !pTmp->GetNext() || pTmp->GetNext()->IsFtnContFrm(),
                        "NeighbourhoodAdjustment: Who's that guy?" );
            }
        }
    }
    return nRet;
}

/*************************************************************************
|*
|*  SwPageFrm::SetColMaxFtnHeight()
|*
|*  Ersterstellung      AMA 29. Oct 98
|*  Letzte Aenderung    AMA 29. Oct 98
|*
|*************************************************************************/
void SwPageFrm::SetColMaxFtnHeight()
{
    SwLayoutFrm *pBody = FindBodyCont();
    if( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
    {
        SwColumnFrm* pCol = (SwColumnFrm*)pBody->Lower();
        do
        {
            pCol->SetMaxFtnHeight( GetMaxFtnHeight() );
            pCol = (SwColumnFrm*)pCol->GetNext();
        } while ( pCol );
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::MoveLowerFtns
|*
|*  Ersterstellung      MA 01. Sep. 94
|*  Letzte Aenderung    MA 05. Sep. 95
|*
|*************************************************************************/


BOOL SwLayoutFrm::MoveLowerFtns( SwCntntFrm *pStart, SwFtnBossFrm *pOldBoss,
                                 SwFtnBossFrm *pNewBoss, const BOOL bFtnNums )
{
    SwDoc *pDoc = GetFmt()->GetDoc();
    if ( !pDoc->GetFtnIdxs().Count() )
        return FALSE;
    if( pDoc->GetFtnInfo().ePos == FTNPOS_CHAPTER &&
        ( !IsInSct() || !FindSctFrm()->IsFtnAtEnd() ) )
        return TRUE;

    if ( !pNewBoss )
        pNewBoss = FindFtnBossFrm( TRUE );
    if ( pNewBoss == pOldBoss )
        return FALSE;

    BOOL bMoved = FALSE;
    if( !pStart )
        pStart = ContainsCntnt();

    SvPtrarr aFtnArr( 5, 5 );

    while ( IsAnLower( pStart ) )
    {
        if ( ((SwTxtFrm*)pStart)->HasFtn() )
            pNewBoss->CollectFtns( pStart, pOldBoss, aFtnArr );
        pStart = pStart->GetNextCntntFrm();
    }

    ASSERT( pOldBoss->IsInSct() == pNewBoss->IsInSct(),
            "MoveLowerFtns: Section confusion" );
    SvPtrarr *pFtnArr;
    SwLayoutFrm *pNewChief, *pOldChief;
    if( pStart && pOldBoss->IsInSct() && ( pOldChief = pOldBoss->FindSctFrm() )
        != ( pNewChief = pNewBoss->FindSctFrm() ) )
    {
        pFtnArr = new SvPtrarr( 5, 5 );
        pOldChief = pOldBoss->FindFtnBossFrm( TRUE );
        pNewChief = pNewBoss->FindFtnBossFrm( TRUE );
        while( pOldChief->IsAnLower( pStart ) )
        {
            if ( ((SwTxtFrm*)pStart)->HasFtn() )
                ((SwFtnBossFrm*)pNewChief)->CollectFtns( pStart,
                                        (SwFtnBossFrm*)pOldBoss, *pFtnArr );
            pStart = pStart->GetNextCntntFrm();
        }
        if( !pFtnArr->Count() )
        {
            delete pFtnArr;
            pFtnArr = NULL;
        }
    }
    else
        pFtnArr = NULL;

    if ( aFtnArr.Count() || pFtnArr )
    {
        if( aFtnArr.Count() )
            pNewBoss->_MoveFtns( aFtnArr, TRUE );
        if( pFtnArr )
        {
            ((SwFtnBossFrm*)pNewChief)->_MoveFtns( *pFtnArr, TRUE );
            delete pFtnArr;
        }
        bMoved = TRUE;

        // Nur bei einem Seitenwechsel muss die FtnNum neu berechnet werden
        if ( bFtnNums )
        {
            SwPageFrm* pOldPage = pOldBoss->FindPageFrm();
            SwPageFrm* pNewPage =pNewBoss->FindPageFrm();
            if( pOldPage != pNewPage )
            {
                pOldPage->UpdateFtnNum();
                pNewPage->UpdateFtnNum();
            }
        }
    }
    return bMoved;
}

/*************************************************************************
|*
|*  SwLayoutFrm::MoveFtnCntFwd()
|*
|*  Ersterstellung      MA 24. Nov. 94
|*  Letzte Aenderung    MA 15. Jun. 95
|*
|*************************************************************************/


BOOL SwCntntFrm::MoveFtnCntFwd( BOOL bMakePage, SwFtnBossFrm *pOldBoss )
{
    ASSERT( IsInFtn(), "Keine Ftn." );
    SwLayoutFrm *pFtn = FindFtnFrm();

    // The first paragraph in the first footnote in the first column in the
    // sectionfrm at the top of the page has not to move forward, if the
    // columnbody is empty.
    if( pOldBoss->IsInSct() && !pOldBoss->GetIndPrev() && !GetIndPrev() &&
        !pFtn->GetPrev() )
    {
        SwLayoutFrm* pBody = pOldBoss->FindBodyCont();
        if( !pBody || !pBody->Lower() )
            return TRUE;
    }

    //fix(9538): Wenn die Ftn noch Nachbarn hinter sich hat, so muessen
    //diese ersteinmal verschwinden.
    SwLayoutFrm *pNxt = (SwLayoutFrm*)pFtn->GetNext();
    SwLayoutFrm *pLst = 0;
    while ( pNxt )
    {
        while ( pNxt->GetNext() )
            pNxt = (SwLayoutFrm*)pNxt->GetNext();
        if ( pNxt == pLst )
            pNxt = 0;
        else
        {   pLst = pNxt;
            SwCntntFrm *pCnt = pNxt->ContainsCntnt();
            if( pCnt )
                pCnt->MoveFtnCntFwd( TRUE, pOldBoss );
            pNxt = (SwLayoutFrm*)pFtn->GetNext();
        }
    }

    BOOL bSamePage = TRUE;
    SwLayoutFrm *pNewUpper =
                GetLeaf( bMakePage ? MAKEPAGE_INSERT : MAKEPAGE_NONE, TRUE );

    if ( pNewUpper )
    {
        BOOL bSameBoss = TRUE;
        SwFtnBossFrm * const pNewBoss = pNewUpper->FindFtnBossFrm();
        //Wechseln wir die Spalte/Seite?
        if ( FALSE == ( bSameBoss = pNewBoss == pOldBoss ) )
        {   bSamePage = pOldBoss->FindPageFrm() == pNewBoss->FindPageFrm(); // Seitenwechsel?
            pNewUpper->Calc();
        }

        //Das Layoutblatt, dass wir fuer Fussnoten bekommen ist entweder
        //ein Fussnotencontainer oder eine Fussnote
        //Wenn es eine Fussnote ist, und sie die gleiche Fussnotenreferez
        //wie der alte Upper hat, so moven wir uns direkt hinein.
        //Ist die Referenz einen andere oder ist es ein Container, so wird
        //eine neue Fussnote erzeugt und in den Container gestellt.
        // Wenn wir in einem Bereich innerhalb der Fussnote sind, muss
        // SectionFrame noch angelegt werden.
        SwFtnFrm* pTmpFtn = pNewUpper->IsFtnFrm() ? ((SwFtnFrm*)pNewUpper) : 0;
        if( !pTmpFtn )
        {
            ASSERT( pNewUpper->IsFtnContFrm(), "Neuer Upper kein FtnCont.");
            SwFtnContFrm *pCont = (SwFtnContFrm*)pNewUpper;

            //Fussnote erzeugen.
            SwFtnFrm *pOld = FindFtnFrm();
            pTmpFtn = new SwFtnFrm( pOld->GetFmt()->GetDoc()->GetDfltFrmFmt(),
                                    pOld->GetRef(), pOld->GetAttr() );
            //Verkettung der Fussnoten.
            if ( pOld->GetFollow() )
            {
                pTmpFtn->SetFollow( pOld->GetFollow() );
                pOld->GetFollow()->SetMaster( pTmpFtn );
            }
            pOld->SetFollow( pTmpFtn );
            pTmpFtn->SetMaster( pOld );
            SwFrm* pNx = pCont->Lower();
            if( pNx && pTmpFtn->GetAttr()->GetFtn().IsEndNote() )
                while(pNx && !((SwFtnFrm*)pNx)->GetAttr()->GetFtn().IsEndNote())
                    pNx = pNx->GetNext();
            pTmpFtn->Paste( pCont, pNx );
            pTmpFtn->Calc();
        }
        ASSERT( pTmpFtn->GetAttr() == FindFtnFrm()->GetAttr(), "Wrong Footnote!" );
        // Bereiche in Fussnoten beduerfen besonderer Behandlung
        SwLayoutFrm *pNewUp = pTmpFtn;
        if( IsInSct() )
        {
            SwSectionFrm* pSect = FindSctFrm();
            // Bereich in Fussnote (oder nur Fussnote in Bereich)?
            if( pSect->IsInFtn() )
            {
                if( pTmpFtn->Lower() && pTmpFtn->Lower()->IsSctFrm() &&
                    pSect->GetFollow() == (SwSectionFrm*)pTmpFtn->Lower() )
                    pNewUp = (SwSectionFrm*)pTmpFtn->Lower();
                else
                {
                    pNewUp = new SwSectionFrm( *pSect, FALSE );
                    pNewUp->InsertBefore( pTmpFtn, pTmpFtn->Lower() );
                    pNewUp->Frm().Pos() = pTmpFtn->Frm().Pos();
                    pNewUp->Frm().Pos().Y() += 1; //wg. Benachrichtigungen.

                    // Wenn unser Bereichsframe einen Nachfolger hat, so muss dieser
                    // umgehaengt werden hinter den neuen Follow der Bereichsframes.
                    SwFrm* pTmp = pSect->GetNext();
                    if( pTmp )
                    {
                        SwFlowFrm* pNxt;
                        if( pTmp->IsCntntFrm() )
                            pNxt = (SwCntntFrm*)pTmp;
                        else if( pTmp->IsSctFrm() )
                            pNxt = (SwSectionFrm*)pTmp;
                        else
                        {
                            ASSERT( pTmp->IsTabFrm(), "GetNextSctLeaf: Wrong Type" );
                            pNxt = (SwTabFrm*)pTmp;
                        }
                        pNxt->MoveSubTree( pTmpFtn, pNewUp->GetNext() );
                    }
                }
            }
        }

        MoveSubTree( pNewUp, pNewUp->Lower() );

        if( !bSameBoss )
            Prepare( PREP_BOSS_CHGD );
    }
    return bSamePage;
}

/*************************************************************************
|*
|*  class SwSaveFtnHeight
|*
|*  Ersterstellung      MA 19. Jan. 94
|*  Letzte Aenderung    MA 19. Jan. 94
|*
|*************************************************************************/


SwSaveFtnHeight::SwSaveFtnHeight( SwFtnBossFrm *pBs, const SwTwips nDeadLine ) :
    pBoss( pBs ),
    nOldHeight( pBs->GetMaxFtnHeight() )
{
    pBoss->SetFtnDeadLine( nDeadLine );
    nNewHeight = pBoss->GetMaxFtnHeight();
}



SwSaveFtnHeight::~SwSaveFtnHeight()
{
    //Wenn zwischendurch jemand an der DeadLine gedreht hat, so lassen wir
    //ihm seinen Spass!
    if ( nNewHeight == pBoss->GetMaxFtnHeight() )
        pBoss->nMaxFtnHeight = nOldHeight;
}


