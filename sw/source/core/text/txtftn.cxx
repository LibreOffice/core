/*************************************************************************
 *
 *  $RCSfile: txtftn.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ama $ $Date: 2001-02-20 09:51:44 $
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

#include "viewsh.hxx"
#include "doc.hxx"
#include "pagefrm.hxx"
#include "ndtxt.hxx"
#include "txtatr.hxx"

#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif

#include "txtcfg.hxx"
#include "swfont.hxx"   // new SwFont
#include "porftn.hxx"
#include "porfly.hxx"
#include "porlay.hxx"
#include "txtfrm.hxx"
#include "itrform2.hxx"
#include "frmsh.hxx"
#include "ftnfrm.hxx"   // FindErgoSumFrm(), FindQuoVadisFrm(),
#include "pagedesc.hxx"
#include "redlnitr.hxx" // SwRedlnItr
#include "sectfrm.hxx"  // SwSectionFrm
#include "layouter.hxx" // Endnote-Collection
#include "frmtool.hxx"
#include "ndindex.hxx"


/*************************************************************************
 *                              _IsFtnNumFrm()
 *************************************************************************/

sal_Bool SwTxtFrm::_IsFtnNumFrm() const
{
    const SwFtnFrm* pFtn = FindFtnFrm()->GetMaster();
    while( pFtn && !pFtn->ContainsCntnt() )
        pFtn = pFtn->GetMaster();
    return !pFtn;
}

/*************************************************************************
 *                              FindFtn()
 *************************************************************************/

// Sucht innerhalb einer Master-Follow-Kette den richtigen TxtFrm zum SwTxtFtn

SwTxtFrm *SwTxtFrm::FindFtnRef( const SwTxtFtn *pFtn )
{
    SwTxtFrm *pFrm = this;
    const sal_Bool bFwd = *pFtn->GetStart() >= GetOfst();
    while( pFrm )
    {
        if( SwFtnBossFrm::FindFtn( pFrm, pFtn ) )
            return pFrm;
        pFrm = bFwd ? pFrm->GetFollow() :
                      pFrm->IsFollow() ? pFrm->FindMaster() : 0;
    }
    return pFrm;
}

/*************************************************************************
 *                              CalcFtnFlag()
 *************************************************************************/

#ifdef PRODUCT
void SwTxtFrm::CalcFtnFlag()
#else
void SwTxtFrm::CalcFtnFlag( xub_StrLen nStop )//Fuer den Test von SplitFrm
#endif
{
    bFtn = sal_False;

    const SwpHints *pHints = GetTxtNode()->GetpSwpHints();
    if( !pHints )
        return;

    const MSHORT nSize = pHints->Count();

#ifdef PRODUCT
    const xub_StrLen nEnd = GetFollow() ? GetFollow()->GetOfst() : STRING_LEN;
#else
    const xub_StrLen nEnd = nStop != STRING_LEN ? nStop
                        : GetFollow() ? GetFollow()->GetOfst() : STRING_LEN;
#endif

    for( MSHORT i = 0; i < nSize; ++i )
    {
        const SwTxtAttr *pHt = (*pHints)[i];
        if ( pHt->Which() == RES_TXTATR_FTN )
        {
            const xub_StrLen nIdx = *pHt->GetStart();
            if ( nEnd < nIdx )
                break;
            if( GetOfst() <= nIdx )
            {
                bFtn = sal_True;
                break;
            }
        }
    }
}

/*************************************************************************
 *                              CalcPrepFtnAdjust()
 *************************************************************************/

sal_Bool SwTxtFrm::CalcPrepFtnAdjust()
{
    ASSERT( HasFtn(), "Wer ruft mich da?" );
    SwFtnBossFrm *pBoss = FindFtnBossFrm( sal_True );
    const SwFtnFrm *pFtn = pBoss->FindFirstFtn( this );
    if( pFtn && FTNPOS_CHAPTER != GetNode()->GetDoc()->GetFtnInfo().ePos &&
        ( !pBoss->GetUpper()->IsSctFrm() ||
        !((SwSectionFrm*)pBoss->GetUpper())->IsFtnAtEnd() ) )
    {
        const SwFtnContFrm *pCont = pBoss->FindFtnCont();
        sal_Bool bReArrange = sal_True;
        if ( pCont && pCont->Frm().Top() > Frm().Bottom() )
        {
            pBoss->RearrangeFtns( Frm().Height() + Frm().Top(), sal_False,
                                  pFtn->GetAttr() );
            ValidateBodyFrm();
            ValidateFrm();
            pFtn = pBoss->FindFirstFtn( this );
        }
        else
            bReArrange = sal_False;
        if( !pCont || !pFtn || bReArrange != (pFtn->FindFtnBossFrm() == pBoss) )
        {
            SwTxtFormatInfo aInf( this );
            SwTxtFormatter aLine( this, &aInf );
            aLine.TruncLines();
            SetPara( 0 );       //Wird ggf. geloescht!
            ResetPreps();
            return sal_False;
        }
    }
    return sal_True;
}

/*************************************************************************
 *                      SwTxtFrm::GetFtnLine()
 *************************************************************************/

SwTwips SwTxtFrm::GetFtnLine( const SwTxtFtn *pFtn, sal_Bool bLocked ) const
{
    SwTxtFrm *pThis = (SwTxtFrm*)this;

    if( !HasPara() )
    {
        // Es sieht ein wenig gehackt aus, aber man riskiert einen Haufen
        // Aerger, wenn man versucht, per pThis->GetFormatted() doch
        // noch an den richtigen Wert heranzukommen. Durch das PREP_ADJUST
        // stellen wir sicher, dass wir noch einmal drankommen, dann aber
        // von der Ref aus!
        // Trotzdem wollen wir nichts unversucht lassen und geben die
        // Unterkante des Frames zurueck.
        if( !bLocked )
            pThis->Prepare( PREP_ADJUST_FRM );
        return Frm().Bottom();
    }

    SwTxtInfo aInf( pThis );
    SwTxtIter aLine( pThis, &aInf );
    const xub_StrLen nPos = *pFtn->GetStart();
    aLine.CharToLine( nPos );
    return aLine.Y() + SwTwips(aLine.GetLineHeight());
}

/*************************************************************************
 *                      SwTxtFrm::GetFtnRstHeight()
 *************************************************************************/

// Ermittelt die max. erreichbare Hoehe des TxtFrm im Ftn-Bereich.
// Sie wird eingeschraenkt durch den unteren Rand der Zeile mit
// der Ftn-Referenz.

SwTwips SwTxtFrm::_GetFtnFrmHeight() const
{
    ASSERT( !IsFollow() && IsInFtn(), "SwTxtFrm::SetFtnLine: moon walk" );

    const SwFtnFrm *pFtnFrm = FindFtnFrm();
    const SwTxtFrm *pRef = (const SwTxtFrm *)pFtnFrm->GetRef();
    const SwFtnBossFrm *pBoss = FindFtnBossFrm();
    if( pBoss != pRef->FindFtnBossFrm( !pFtnFrm->GetAttr()->
                                        GetFtn().IsEndNote() ) )
        return 0;

    SwTwips nHeight = pRef->IsInFtnConnect() ?
                            1 : pRef->GetFtnLine( pFtnFrm->GetAttr(), sal_False );
    if( nHeight )
    {
        // So komisch es aussehen mag: Die erste Ftn auf der Seite darf sich
        // nicht mit der Ftn-Referenz beruehren, wenn wir im Ftn-Bereich Text
        // eingeben.
        const SwFrm *pCont = pFtnFrm->GetUpper();
        //Hoehe innerhalb des Cont, die ich mir 'eh noch genehmigen darf.
        SwTwips nTmp = pCont->Frm().Top() + pCont->Prt().Top() +
                       pCont->Prt().Height() - Frm().Top();
#ifndef PRODUCT
        if( nTmp < 0 )
        {
            sal_Bool bInvalidPos = sal_False;
            const SwFrm* pTmp = GetUpper();
            while( !bInvalidPos && pTmp )
            {
                bInvalidPos = !pTmp->GetValidPosFlag();
                if( pTmp == pCont )
                    break;
                pTmp = pTmp->GetUpper();
            }
            ASSERT( bInvalidPos, "Hanging below FtnCont" );
        }
#endif
        if( pCont->Frm().Top() > nHeight )
        {
            //Wachstumspotential den Containers.
            if ( !pRef->IsInFtnConnect() )
            {
                SwSaveFtnHeight aSave( (SwFtnBossFrm*)pBoss, nHeight  );
                nHeight = ((SwFtnContFrm*)pCont)->Grow( LONG_MAX, pHeight, sal_True );
            }
            else
                nHeight = ((SwFtnContFrm*)pCont)->Grow( LONG_MAX, pHeight, sal_True );

            nHeight += nTmp;
            if( nHeight < 0 )
                nHeight = 0;
        }
        else
        {   // The container has to shrink
            nTmp += pCont->Frm().Top() - nHeight;
            if( nTmp > 0 )
                nHeight = nTmp;
            else
                nHeight = 0;
        }
    }
    return nHeight;
}

/*************************************************************************
 *                      SwTxtFrm::FindErgoSumFrm()
 *************************************************************************/

SwTxtFrm *SwTxtFrm::FindErgoSumFrm()
{
    // Erstmal feststellen, ob wir in einem FtnFrm stehen:
    if( !IsInFtn() )
        return 0;

    // Zum folgenden FtnFrm
    SwFtnFrm *pFtnFrm = FindFtnFrm()->GetFollow();

    // Nun den ersten Cntnt:
    return pFtnFrm ? (SwTxtFrm*)(pFtnFrm->ContainsCntnt()) : 0;
}

/*************************************************************************
 *                      SwTxtFrm::FindQuoVadisFrm()
 *************************************************************************/

SwTxtFrm *SwTxtFrm::FindQuoVadisFrm()
{
    // Erstmal feststellen, ob wir in einem FtnFrm stehen:
    if( GetIndPrev() || !IsInFtn() )
        return 0;

    // Zum Vorgaenger-FtnFrm
    SwFtnFrm *pFtnFrm = FindFtnFrm()->GetMaster();
    if( !pFtnFrm )
        return 0;

    // Nun den letzten Cntnt:
    const SwCntntFrm *pCnt = pFtnFrm->ContainsCntnt();
    if( !pCnt )
        return NULL;
    const SwCntntFrm *pLast;
    do
    {   pLast = pCnt;
        pCnt = pCnt->GetNextCntntFrm();
    } while( pCnt && pFtnFrm->IsAnLower( pCnt ) );
    return (SwTxtFrm*)pLast;
}

/*************************************************************************
 *                      SwTxtFrm::SetErgoSumNum()
 *************************************************************************/

void SwTxtFrm::SetErgoSumNum( const MSHORT nErgo )
{
    SwParaPortion *pPara = GetPara();
    if( pPara )
    {
        XubString aStr( nErgo );
        pPara->SetErgoSumNum( aStr );
    }
}

sal_Bool lcl_Apres( SwLayoutFrm* pFirst, SwLayoutFrm* pSecond )
{
    SwPageFrm *pPg1 = pFirst->FindPageFrm();
    SwPageFrm *pPg2 = pSecond->FindPageFrm();
    if( pPg1 != pPg2 )
        return pPg1->GetPhyPageNum() < pPg2->GetPhyPageNum();
    SwLayoutFrm *pUp = pFirst;
    while( pUp->GetUpper() && !pUp->GetUpper()->IsAnLower( pSecond ) )
        pUp = pUp->GetUpper();
    if( !pUp->GetUpper() )
        return sal_False;
    do
    {
        pUp = (SwLayoutFrm*)pUp->GetNext();
    } while( pUp && !pUp->IsAnLower( pSecond ) );
    return 0 != pUp;
}

/*************************************************************************
 *                      SwTxtFrm::RemoveFtn()
 *************************************************************************/

void SwTxtFrm::RemoveFtn( const xub_StrLen nStart, const xub_StrLen nLen )
{
    if ( !IsFtnAllowed() )
        return;

    SwpHints *pHints = GetTxtNode()->GetpSwpHints();
    if( !pHints )
        return;

    sal_Bool bRollBack = nLen != STRING_LEN;
    MSHORT nSize = pHints->Count();
    xub_StrLen nEnd;
    SwTxtFrm* pSource;
    if( bRollBack )
    {
        nEnd = nStart + nLen;
        pSource = GetFollow();
        if( !pSource )
            return;
    }
    else
    {
        nEnd = STRING_LEN;
        pSource = this;
    }

    if( nSize )
    {
        SwPageFrm* pUpdate = NULL;
        sal_Bool bRemove = sal_False;
        SwFtnBossFrm *pFtnBoss = 0;
        SwFtnBossFrm *pEndBoss = 0;
        sal_Bool bFtnEndDoc
            = FTNPOS_CHAPTER == GetNode()->GetDoc()->GetFtnInfo().ePos;
        for( MSHORT i = nSize; i; )
        {
            SwTxtAttr *pHt = pHints->GetHt(--i);
            if ( RES_TXTATR_FTN != pHt->Which() )
                continue;

            const xub_StrLen nIdx = *pHt->GetStart();
            if( nStart > nIdx )
                break;

            if( nEnd >= nIdx )
            {
                SwTxtFtn *pFtn = (SwTxtFtn*)pHt;
                sal_Bool bEndn = pFtn->GetFtn().IsEndNote();

                if( bEndn )
                {
                    if( !pEndBoss )
                        pEndBoss = pSource->FindFtnBossFrm();
                }
                else
                {
                    if( !pFtnBoss )
                    {
                        pFtnBoss = pSource->FindFtnBossFrm( sal_True );
                        if( pFtnBoss->GetUpper()->IsSctFrm() )
                        {
                            SwSectionFrm* pSect = (SwSectionFrm*)
                                                  pFtnBoss->GetUpper();
                            if( pSect->IsFtnAtEnd() )
                                bFtnEndDoc = sal_False;
                        }
                    }
                }

                // Wir loeschen nicht, sondern wollen die Ftn verschieben.
                // Drei Faelle koennen auftreten:
                // 1) Es gibt weder Follow noch PrevFollow
                //    -> RemoveFtn()  (vielleicht sogar ein ASSERT wert)
                // 2) nStart > GetOfst, ich habe einen Follow
                //    -> Ftn wandert in den Follow
                // 3) nStart < GetOfst, ich bin ein Follow
                //    -> Ftn wandert in den PrevFollow
                // beide muessen auf einer Seite/in einer Spalte stehen.

                SwFtnFrm *pFtnFrm = bEndn ? pEndBoss->FindFtn( pSource, pFtn ) :
                                            pFtnBoss->FindFtn( pSource, pFtn );

                if( pFtnFrm )
                {
                    const sal_Bool bEndDoc = bEndn ? sal_True : bFtnEndDoc;
                    if( bRollBack )
                    {
                        while ( pFtnFrm )
                        {
                            pFtnFrm->SetRef( this );
                            pFtnFrm = pFtnFrm->GetFollow();
                            SetFtn( sal_True );
                        }
                    }
                    else if( GetFollow() )
                    {
                        SwCntntFrm *pDest = GetFollow();
                        while( pDest->GetFollow() && ((SwTxtFrm*)pDest->
                               GetFollow())->GetOfst() <= nIdx )
                            pDest = pDest->GetFollow();
                        ASSERT( !pDest->FindFtnBossFrm( !bEndn )->FindFtn(
                            pDest,pFtn),"SwTxtFrm::RemoveFtn: footnote exists");

                        //Nicht ummelden sondern immer Moven.
                        if( bEndDoc || !lcl_Apres( pFtnFrm->FindFtnBossFrm(),
                                            pDest->FindFtnBossFrm( !bEndn ) ) )
                        {
                            SwPageFrm* pTmp = pFtnFrm->FindPageFrm();
                            if( pUpdate && pUpdate != pTmp )
                                pUpdate->UpdateFtnNum();
                            pUpdate = pTmp;
                            while ( pFtnFrm )
                            {
                                pFtnFrm->SetRef( pDest );
                                pFtnFrm = pFtnFrm->GetFollow();
                            }
                        }
                        else
                        {
                            if( bEndn )
                                pEndBoss->MoveFtns( this, pDest, pFtn );
                            else
                                pFtnBoss->MoveFtns( this, pDest, pFtn );
                            bRemove = sal_True;
                        }
                        ((SwTxtFrm*)pDest)->SetFtn( sal_True );

                        ASSERT( pDest->FindFtnBossFrm( !bEndn )->FindFtn( pDest,
                           pFtn),"SwTxtFrm::RemoveFtn: footnote ChgRef failed");
                    }
                    else
                    {
                        if( !bEndDoc || ( bEndn && pEndBoss->IsInSct() ) )
                        {
                            if( bEndn )
                                pEndBoss->RemoveFtn( this, pFtn );
                            else
                                pFtnBoss->RemoveFtn( this, pFtn );
                            bRemove = bRemove || !bEndDoc;
                            ASSERT( bEndn ? !pEndBoss->FindFtn( this, pFtn ) :
                                    !pFtnBoss->FindFtn( this, pFtn ),
                            "SwTxtFrm::RemoveFtn: can't get off that footnote" );
                        }
                    }
                }
            }
        }
        if( pUpdate )
            pUpdate->UpdateFtnNum();
        // Wir bringen die Oszillation zum stehen:
        if( bRemove && !bFtnEndDoc && HasPara() )
        {
            ValidateBodyFrm();
            ValidateFrm();
        }
    }
    // Folgendes Problem: Aus dem FindBreak heraus wird das RemoveFtn aufgerufen,
    // weil die letzte Zeile an den Follow abgegeben werden soll. Der Offset
    // des Follows ist aber veraltet, er wird demnaechst gesetzt. CalcFntFlag ist
    // auf einen richtigen Follow-Offset angewiesen. Deshalb wird hier kurzfristig
    // der Follow-Offset manipuliert.
    xub_StrLen nOldOfst = STRING_LEN;
    if( HasFollow() && nStart > GetOfst() )
    {
        nOldOfst = GetFollow()->GetOfst();
        GetFollow()->ManipOfst( nStart + ( bRollBack ? nLen : 0 ) );
    }
    pSource->CalcFtnFlag();
    if( nOldOfst < STRING_LEN )
        GetFollow()->ManipOfst( nOldOfst );
}

/*************************************************************************
 *                      SwTxtFormatter::ConnectFtn()
 *************************************************************************/
// sal_False, wenn irgendetwas schief gegangen ist.
// Es gibt eigentlich nur zwei Moeglichkeiten:
// a) Die Ftn ist bereits vorhanden
// => dann wird sie gemoved, wenn ein anderer pSrcFrm gefunden wurde
// b) Die Ftn ist nicht vorhanden
// => dann wird sie fuer uns angelegt.
// Ob die Ftn schliesslich auf unserer Spalte/Seite landet oder nicht,
// spielt in diesem Zusammenhang keine Rolle.
// Optimierungen bei Endnoten.
// Noch ein Problem: wenn die Deadline im Ftn-Bereich liegt, muss die
// Ftn verschoben werden.

void SwTxtFrm::ConnectFtn( SwTxtFtn *pFtn, const SwTwips nDeadLine )
{
    bFtn = sal_True;
    bInFtnConnect = sal_True;   //Bloss zuruecksetzen!
    sal_Bool bEnd = pFtn->GetFtn().IsEndNote();

    // Wir brauchen immer einen Boss (Spalte/Seite)
    SwSectionFrm *pSect;
    SwCntntFrm *pCntnt = this;
    if( bEnd && IsInSct() )
    {
        pSect = FindSctFrm();
        if( pSect->IsEndnAtEnd() )
            pCntnt = pSect->FindLastCntnt( FINDMODE_ENDNOTE );
        if( !pCntnt )
            pCntnt = this;
    }

    SwFtnBossFrm *pBoss = pCntnt->FindFtnBossFrm( !bEnd );

#ifdef DEBUG
    SwTwips nRstHeight = GetRstHeight();
#endif

    pSect = pBoss->FindSctFrm();
    sal_Bool bDocEnd = bEnd ? !( pSect && pSect->IsEndnAtEnd() ) :
                   ( !( pSect && pSect->IsFtnAtEnd() ) &&
                       FTNPOS_CHAPTER == GetNode()->GetDoc()->GetFtnInfo().ePos );
    //Ftn kann beim Follow angemeldet sein.
    SwCntntFrm *pSrcFrm = FindFtnRef( pFtn );

    if( bDocEnd )
    {
        if( pSect && pSrcFrm )
        {
            SwFtnFrm *pFtnFrm = pBoss->FindFtn( pSrcFrm, pFtn );
            if( pFtnFrm && pFtnFrm->IsInSct() )
            {
                pBoss->RemoveFtn( pSrcFrm, pFtn );
                pSrcFrm = 0;
            }
        }
    }
    else if( bEnd && pSect )
    {
        SwFtnFrm *pFtnFrm = pSrcFrm ? pBoss->FindFtn( pSrcFrm, pFtn ) : NULL;
        if( pFtnFrm && !pFtnFrm->GetUpper() )
            pFtnFrm = NULL;
        SwDoc *pDoc = GetNode()->GetDoc();
        if( SwLayouter::Collecting( pDoc, pSect, pFtnFrm ) )
        {
            if( !pSrcFrm )
            {
                SwFtnFrm *pNew = new SwFtnFrm(pDoc->GetDfltFrmFmt(),this,pFtn);
                 SwNodeIndex aIdx( *pFtn->GetStartNode(), 1 );
                 ::_InsertCnt( pNew, pDoc, aIdx.GetIndex() );
                pDoc->GetLayouter()->CollectEndnote( pNew );
            }
            else if( pSrcFrm != this )
                pBoss->ChangeFtnRef( pSrcFrm, pFtn, this );
            bInFtnConnect = sal_False;
            return;
        }
        else if( pSrcFrm )
        {
            SwFtnBossFrm *pFtnBoss = pFtnFrm->FindFtnBossFrm();
            if( !pFtnBoss->IsInSct() ||
                pFtnBoss->ImplFindSctFrm()->GetSection()!=pSect->GetSection() )
            {
                pBoss->RemoveFtn( pSrcFrm, pFtn );
                pSrcFrm = 0;
            }
        }
    }

    if( bDocEnd || bEnd )
    {
        if( !pSrcFrm )
            pBoss->AppendFtn( this, pFtn );
        else if( pSrcFrm != this )
            pBoss->ChangeFtnRef( pSrcFrm, pFtn, this );
        bInFtnConnect = sal_False;
        return;
    }

    SwSaveFtnHeight aHeight( pBoss, nDeadLine );

    if( !pSrcFrm )      // Es wurde ueberhaupt keine Ftn gefunden.
        pBoss->AppendFtn( this, pFtn );
    else
    {
        SwFtnFrm *pFtnFrm = pBoss->FindFtn( pSrcFrm, pFtn );
        SwFtnBossFrm *pFtnBoss = pFtnFrm->FindFtnBossFrm();

        sal_Bool bBrutal = sal_False;

        if( pFtnBoss == pBoss ) // Ref und Ftn sind auf der selben Seite/Spalte.
        {
            SwFrm *pCont = pFtnFrm->GetUpper();

            if( pCont->Frm().Top() >= nDeadLine )//Ref und Ftn passen auf die Seite
            {
                //Wenn die Fussnote bei einem Follow angemeldet ist, so ist
                //es jetzt an der Zeit sie umzumelden.
                if ( pSrcFrm != this )
                    pBoss->ChangeFtnRef( pSrcFrm, pFtn, this );
                //Es steht Platz zur Verfuegung, also kann die Fussnote evtl.
                //wachsen.
                if ( pFtnFrm->GetFollow() && pCont->Frm().Top() > nDeadLine )
                {
                    SwTwips nHeight = pCont->Frm().Height();
                    pBoss->RearrangeFtns( nDeadLine, sal_False, pFtn );
                    ValidateBodyFrm();
                    ValidateFrm();
                    ViewShell *pSh = GetShell();
                    if ( pSh && nHeight == pCont->Frm().Height() )
                        //Damit uns nix durch die Lappen geht.
                        pSh->InvalidateWindows( pCont->Frm() );
                }
                bInFtnConnect = sal_False;
                return;
            }
            else
                bBrutal = sal_True;
        }
        else
        {
            // Ref und Ftn sind nicht auf einer Seite, Move-Versuch ist noetig.
            SwFrm* pTmp = this;
            while( pTmp->GetNext() && pSrcFrm != pTmp )
                pTmp = pTmp->GetNext();
            if( pSrcFrm == pTmp )
                bBrutal = sal_True;
            else
            {   // Wenn unser Boss in einem spaltigen Bereich sitzt, es aber auf
                // der Seite schon einen FtnContainer gibt, hilft nur die brutale
                // Methode
                if( pSect && pSect->FindFtnBossFrm( !bEnd )->FindFtnCont() )
                    bBrutal = sal_True;
                else if( !pFtnFrm->GetPrev() || lcl_Apres( pFtnBoss, pBoss ) )
                {
                    SwFtnBossFrm *pSrcBoss = pSrcFrm->FindFtnBossFrm( !bEnd );
                    pSrcBoss->MoveFtns( pSrcFrm, this, pFtn );
                }
                else
                    pBoss->ChangeFtnRef( pSrcFrm, pFtn, this );
            }
        }

        // Die brutale Loesung: Fussnote entfernen und appenden.
        // Es muss SetFtnDeadLine() gerufen werden, weil nach
        // RemoveFtn die nMaxFtnHeight evtl. besser auf unsere Wuensche
        // eingestellt werden kann.
        if( bBrutal )
        {
            pBoss->RemoveFtn( pSrcFrm, pFtn, sal_False );
            SwSaveFtnHeight *pHeight = bEnd ? NULL :
                new SwSaveFtnHeight( pBoss, nDeadLine );
            pBoss->AppendFtn( this, pFtn );
            delete pHeight;
        }
    }

    // In spaltigen Bereichen, die noch nicht bis zum Seitenrand gehen,
    // ist kein RearrangeFtns sinnvoll, da der Fussnotencontainer noch
    // nicht kalkuliert worden ist.
    if( !pSect || !pSect->Growable() )
    {
        // Umgebung validieren, um Oszillationen zu verhindern.
        SwSaveFtnHeight aNochmal( pBoss, nDeadLine );
        ValidateBodyFrm();
        pBoss->RearrangeFtns( nDeadLine, sal_True );
        ValidateFrm();
    }
    else if( pSect->IsFtnAtEnd() )
    {
        ValidateBodyFrm();
        ValidateFrm();
    }

#ifdef DEBUG
    // pFtnFrm kann sich durch Calc veraendert haben ...
    SwFtnFrm *pFtnFrm = pBoss->FindFtn( this, pFtn );
    if( pFtnFrm && pBoss != pFtnFrm->FindFtnBossFrm( !bEnd ) )
    {
        int bla = 5;
    }
    nRstHeight = GetRstHeight();
#endif
    bInFtnConnect = sal_False;
    return;
}

/*************************************************************************
 *                      SwTxtFormatter::NewFtnPortion()
 *************************************************************************/

// Die Portion fuer die Ftn-Referenz im Text
SwFtnPortion *SwTxtFormatter::NewFtnPortion( SwTxtFormatInfo &rInf,
                                             SwTxtAttr *pHint )
{
    if( !pFrm->IsFtnAllowed() )
        return 0;

    SwTxtFtn  *pFtn = (SwTxtFtn*)pHint;
    SwFmtFtn& rFtn = (SwFmtFtn&)pFtn->GetFtn();
    SwDoc *pDoc = pFrm->GetNode()->GetDoc();

    if( rInf.IsTest() )
        return new SwFtnPortion( rFtn.GetViewNumStr( *pDoc ), pFrm, pFtn );

    KSHORT nReal;
    {
        KSHORT nOldReal = pCurr->GetRealHeight();
        KSHORT nOldAscent = pCurr->GetAscent();
        KSHORT nOldHeight = pCurr->Height();
        ((SwTxtFormatter*)this)->CalcRealHeight();
        nReal = pCurr->GetRealHeight();
        if( nReal < nOldReal )
            nReal = nOldReal;
        pCurr->SetRealHeight( nOldReal );
        pCurr->Height( nOldHeight );
        pCurr->SetAscent( nOldAscent );
    }

    SwTwips nLower = Y() + nReal;
    SwFrm* pPrtFrm;
    if( pFrm->IsInTab() && (!pFrm->IsInSct() || pFrm->FindSctFrm()->IsInTab()) )
    {
        SwFrm *pRow = pFrm;
        while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
            pRow = pRow->GetUpper();
        SwTwips nMin = pRow->Frm().Top() + pRow->Frm().Height();
        if( nMin > nLower )
            nLower = nMin;
        pPrtFrm = pRow->GetUpper();
    }
    else
        pPrtFrm = pFrm;
    SwTwips nAdd =
        pPrtFrm->Frm().Height() -pPrtFrm->Prt().Height() -pPrtFrm->Prt().Top();
    if( nAdd > 0 )
        nLower += nAdd;
        //6995: Wir frischen nur auf. Das Connect tut fuer diesen Fall nix
        //Brauchbares, sondern wuerde stattdessen fuer diesen Fall meist die
        //Ftn wegwerfen und neu erzeugen.
    if( !rInf.IsQuick() )
        pFrm->ConnectFtn( pFtn, nLower );

    SwTxtFrm *pScrFrm = pFrm->FindFtnRef( pFtn );
    SwFtnBossFrm *pBoss = pFrm->FindFtnBossFrm( !rFtn.IsEndNote() );
    SwFtnFrm *pFtnFrm = NULL;
    if( pScrFrm )
    {
        pFtnFrm = pBoss->FindFtn( pScrFrm, pFtn );
        if( pFtnFrm && pFtnFrm->Lower() )
        {
            SwTxtFrm *pTxtFrm = NULL;
            if( pFtnFrm->Lower()->IsTxtFrm() )
                pTxtFrm = (SwTxtFrm*)pFtnFrm->Lower();
            else if( pFtnFrm->Lower()->IsSctFrm() )
            {
                SwFrm* pCntnt = ((SwSectionFrm*)pFtnFrm->Lower())->ContainsCntnt();
                if( pCntnt && pCntnt->IsTxtFrm() )
                    pTxtFrm = (SwTxtFrm*)pCntnt;
            }
            if ( pTxtFrm && pTxtFrm->HasPara() )
            {
                SwParaPortion *pPara = pTxtFrm->GetPara();
                SwLinePortion *pTmp = pPara->GetPortion();
                while( pTmp )
                {
                    if( pTmp->IsFtnNumPortion() )
                    {
                        SeekAndChg( rInf );
                        if( ((SwFtnNumPortion*)pTmp)->DiffFont( rInf.GetFont() ) )
                            pTxtFrm->Prepare(PREP_FTN);
                        break;
                    }
                    pTmp = pTmp->GetPortion();
                }
            }
        }
    }
    // Wir erkundigen uns, ob durch unser Append irgendeine
    // Fussnote noch auf der Seite/Spalte steht. Wenn nicht verschwindet
    // auch unsere Zeile. Dies fuehrt zu folgendem erwuenschten
    // Verhalten: Ftn1 pass noch auf die Seite/Spalte, Ftn2 nicht mehr.
    // Also bleibt die Ftn2-Referenz auf der Seite/Spalte stehen. Die
    // Fussnote selbst folgt aber erst auf der naechsten Seite/Spalte.
    // Ausnahme: Wenn keine weitere Zeile auf diese Seite/Spalte passt,
    // so sollte die Ftn2-Referenz auch auf die naechste wandern.
    if( !rFtn.IsEndNote() )
    {
        SwSectionFrm *pSct = pBoss->FindSctFrm();
        sal_Bool bAtSctEnd = pSct && pSct->IsFtnAtEnd();
        if( FTNPOS_CHAPTER != pDoc->GetFtnInfo().ePos || bAtSctEnd )
        {
            SwFrm* pFtnCont = pBoss->FindFtnCont();
            // Wenn der Boss in einem Bereich liegt, kann es sich nur um eine
            // Spalte dieses Bereichs handeln. Wenn dies nicht die erste Spalte
            // ist, duerfen wir ausweichen
            if( !pFrm->IsInTab() && ( GetLineNr() > 1 || pFrm->GetPrev() ||
                ( !bAtSctEnd && pFrm->GetIndPrev() ) ||
                ( pSct && pBoss->GetPrev() ) ) )
            {
                if( !pFtnCont )
                {
                    rInf.SetStop( sal_True );
                    return 0;
                }
                else
                {
                    // Es darf keine Fussnotencontainer in spaltigen Bereichen und
                    // gleichzeitig auf der Seite/Seitenspalte geben
                    if( pSct && !bAtSctEnd ) // liegt unser Container in einem (spaltigen) Bereich?
                    {
                        SwFtnBossFrm* pTmp = pBoss->FindSctFrm()->FindFtnBossFrm( sal_True );
                        SwFtnContFrm* pFtnC = pTmp->FindFtnCont();
                        if( pFtnC )
                        {
                            SwFtnFrm* pTmp = (SwFtnFrm*)pFtnC->Lower();
                            if( pTmp && *pTmp < pFtn )
                            {
                                rInf.SetStop( sal_True );
                                return 0;
                            }
                        }
                    }
                    // Ist dies die letzte passende Zeile?
                    if( pScrFrm && Y() + nReal*2 > pFtnCont->Frm().Top() )
                    {
                        if( pFtnFrm )
                        {
                            SwFtnBossFrm *pFtnBoss = pFtnFrm->FindFtnBossFrm();
                            if( pFtnBoss != pBoss )
                            {
                                // Wir sind in der letzte Zeile und die Fussnote
                                // ist auf eine andere Seite gewandert, dann wollen
                                // wir mit ...
                                rInf.SetStop( sal_True );
                                return 0;
                            }
                        }
                    }
                }
            }
        }
    }
    // Endlich: FtnPortion anlegen und raus hier...
    SwFtnPortion *pRet = new SwFtnPortion( rFtn.GetViewNumStr( *pDoc ),
                                            pFrm, pFtn, nReal );
    rInf.SetFtnInside( sal_True );
    return pRet;
 }

/*************************************************************************
 *                      SwTxtFormatter::NewFtnNumPortion()
 *************************************************************************/

// Die Portion fuer die Ftn-Nummerierung im Ftn-Bereich

SwNumberPortion *SwTxtFormatter::NewFtnNumPortion( SwTxtFormatInfo &rInf ) const
{
    ASSERT( pFrm->IsInFtn() && !pFrm->GetIndPrev() && !rInf.IsFtnDone(),
            "This is the wrong place for a ftnnumber" );
    if( rInf.GetTxtStart() != nStart ||
        rInf.GetTxtStart() != rInf.GetIdx() )
        return 0;

    const SwFtnFrm *pFtnFrm = pFrm->FindFtnFrm();
    const SwTxtFtn *pFtn = pFtnFrm->GetAttr();

    // Aha, wir sind also im Fussnotenbereich
    SwFmtFtn& rFtn = (SwFmtFtn&)pFtn->GetFtn();

    SwDoc *pDoc = pFrm->GetNode()->GetDoc();
    XubString aFtnTxt( rFtn.GetViewNumStr( *pDoc, sal_True ));

    const SwEndNoteInfo* pInfo;
    if( rFtn.IsEndNote() )
        pInfo = &pDoc->GetEndNoteInfo();
    else
        pInfo = &pDoc->GetFtnInfo();
    const SwAttrSet& rSet = pInfo->GetCharFmt(*pDoc)->GetAttrSet();

    const SwAttrSet* pParSet = &rInf.GetCharAttr();
    SwFont *pFnt = new SwFont( pParSet );
    pFnt->SetDiffFnt(&rSet);
    SwTxtFtn* pTxtFtn = rFtn.GetTxtFtn();
    if( pTxtFtn )
    {
        SwScriptInfo aScriptInfo;
        SwAttrIter aIter( (SwTxtNode&)pTxtFtn->GetTxtNode(), &aScriptInfo );
        aIter.Seek( *pTxtFtn->GetStart() );
        // Achtung: Wenn die Kriterien, nach denen der FtnReferenz-Font
        // auf den FtnNumerierungsfont wirkt, geaendert werden, muss die
        // untenstehende Methode SwFtnNumPortion::DiffFont() angepasst
        // werden.
        if( aIter.GetFnt()->IsSymbol(rInf.GetVsh()) || aIter.GetFnt()->GetCharSet() !=
            pFnt->GetCharSet() )
        {
            const BYTE nAct = pFnt->GetActual();
            pFnt->SetName( aIter.GetFnt()->GetName(), nAct );
            pFnt->SetStyleName( aIter.GetFnt()->GetStyleName(), nAct );
            pFnt->SetFamily( aIter.GetFnt()->GetFamily(),nAct );
            pFnt->SetCharSet( aIter.GetFnt()->GetCharSet(), nAct );
        }
    }
    return new SwFtnNumPortion( aFtnTxt, pFnt );
}

/*************************************************************************
 *                  SwTxtFormatter::NewErgoSumPortion()
 *************************************************************************/

XubString lcl_GetPageNumber( const SwPageFrm* pPage )
{
    ASSERT( pPage, "GetPageNumber: Homeless TxtFrm" );
    MSHORT nVirtNum = pPage->GetVirtPageNum();
    const SwNumType& rNum = pPage->GetPageDesc()->GetNumType();
    return rNum.GetNumStr( nVirtNum );
}

SwErgoSumPortion *SwTxtFormatter::NewErgoSumPortion( SwTxtFormatInfo &rInf ) const
{
    // Wir koennen nicht davon ausgehen, dass wir ein Follow sind
    // 7983: GetIdx() nicht nStart
    if( !pFrm->IsInFtn()  || pFrm->GetPrev() ||
        rInf.IsErgoDone() || rInf.GetIdx() != pFrm->GetOfst() ||
        pFrm->ImplFindFtnFrm()->GetAttr()->GetFtn().IsEndNote() )
        return 0;

    // Aha, wir sind also im Fussnotenbereich
    const SwFtnInfo &rFtnInfo = pFrm->GetNode()->GetDoc()->GetFtnInfo();
    SwTxtFrm *pQuoFrm = pFrm->FindQuoVadisFrm();
    if( !pQuoFrm )
        return 0;
    const SwPageFrm* pPage = pFrm->FindPageFrm();
    const SwPageFrm* pQuoPage = pQuoFrm->FindPageFrm();
    if( pPage == pQuoFrm->FindPageFrm() )
        return 0; // Wenn der QuoVadis auf der selben (spaltigen) Seite steht
    const XubString aPage = lcl_GetPageNumber( pPage );
    SwParaPortion *pPara = pQuoFrm->GetPara();
    if( pPara )
        pPara->SetErgoSumNum( aPage );
    if( !rFtnInfo.aErgoSum.Len() )
        return 0;
    SwErgoSumPortion *pErgo = new SwErgoSumPortion( rFtnInfo.aErgoSum,
                                lcl_GetPageNumber( pQuoPage ) );
    return pErgo;
}

/*************************************************************************
 *                  SwTxtFormatter::FormatQuoVadis()
 *************************************************************************/

xub_StrLen SwTxtFormatter::FormatQuoVadis( const xub_StrLen nOffset )
{
    if( !pFrm->IsInFtn() || pFrm->ImplFindFtnFrm()->GetAttr()->GetFtn().IsEndNote() )
        return nOffset;

    const SwFrm* pErgoFrm = pFrm->FindFtnFrm()->GetFollow();
    if( !pErgoFrm && pFrm->HasFollow() )
        pErgoFrm = pFrm->GetFollow();
    if( !pErgoFrm )
        return nOffset;

    if( pErgoFrm == pFrm->GetNext() )
    {
        SwFrm *pCol = pFrm->FindColFrm();
        while( pCol && !pCol->GetNext() )
            pCol = pCol->GetUpper()->FindColFrm();
        if( pCol )
            return nOffset;
    }
    else
    {
        const SwPageFrm* pPage = pFrm->FindPageFrm();
        const SwPageFrm* pErgoPage = pErgoFrm->FindPageFrm();
        if( pPage == pErgoFrm->FindPageFrm() )
            return nOffset; // Wenn der ErgoSum auf der selben Seite steht
    }

    SwTxtFormatInfo &rInf = GetInfo();
    const SwFtnInfo &rFtnInfo = pFrm->GetNode()->GetDoc()->GetFtnInfo();
    if( !rFtnInfo.aQuoVadis.Len() )
        return nOffset;

    // Ein Wort zu QuoVadis/ErgoSum:
    // Fuer diese Texte wird der am Absatz eingestellte Font verwendet.
    // Wir initialisieren uns also:
//  ResetFont();
    FeedInf( rInf );
    SeekStartAndChg( rInf, sal_True );
    if( GetRedln() && pCurr->HasRedline() )
        GetRedln()->Seek( *pFnt, nOffset, 0 );

    // Ein fieser Sonderfall: Flyfrms reichen in die Zeile und stehen
    // natuerlich da, wo wir unseren Quovadis Text reinsetzen wollen.
    // Erst mal sehen, ob es so schlimm ist:
    SwLinePortion *pPor = pCurr->GetFirstPortion();
    KSHORT nLastLeft = 0;
    while( pPor )
    {
        if ( pPor->IsFlyPortion() )
            nLastLeft = ( (SwFlyPortion*) pPor)->Fix() +
                        ( (SwFlyPortion*) pPor)->Width();
        pPor = pPor->GetPortion();
    }
    // Das alte Spiel: wir wollen, dass die Zeile an einer bestimmten
    // Stelle umbricht, also beeinflussen wir die Width.
    // nLastLeft ist jetzt quasi der rechte Rand.
    const KSHORT nOldRealWidth = rInf.RealWidth();
    rInf.RealWidth( nOldRealWidth - nLastLeft );

    XubString aErgo = lcl_GetPageNumber( pErgoFrm->FindPageFrm() );
    SwQuoVadisPortion *pQuo = new SwQuoVadisPortion(rFtnInfo.aQuoVadis, aErgo );
    pQuo->SetAscent( rInf.GetAscent()  );
    pQuo->Height( rInf.GetTxtHeight() );
    pQuo->Format( rInf );

    // Wir nutzen das Recycling-Feature aus und suchen die Stelle,
    // ab der wir den QuoVadis-Text einfuegen wollen.
    nLastLeft = nOldRealWidth - pQuo->Width();
    pPor = pCurr->GetFirstPortion();
    while( pPor && nLastLeft > rInf.X() + pPor->Width() )
    {
        pPor->Move( rInf );
        pPor = pPor->GetPortion();
    }
    if( pPor )
        rInf.GetParaPortion()->GetReformat()->Start() = rInf.GetIdx();

    // Jetzt wird formatiert
    Right( Right() - pQuo->Width() );
    const xub_StrLen nRet = FormatLine( nStart );
    Right( rInf.Left() + nOldRealWidth - 1 );

    nLastLeft = nOldRealWidth - pCurr->Width();
    FeedInf( rInf );
#ifdef USED
    ASSERT( nOldRealWidth == rInf.RealWidth() && nLastLeft >= pQuo->Width(),
            "SwTxtFormatter::FormatQuoVadis: crime doesn't pay" );
#endif
    if( pQuo->Width() > nLastLeft )
        pQuo->Width( nLastLeft );

    // Es kann durchaus sein, dass am Ende eine Marginportion steht,
    // die beim erneuten Aufspannen nur Aerger bereiten wuerde.
    pPor = pCurr->FindLastPortion();
    SwGluePortion *pGlue = pPor->IsMarginPortion() ?
        (SwMarginPortion*) pPor : 0;
    if( pGlue )
    {
        pGlue->Height( 0 );
        pGlue->Width( 0 );
        pGlue->SetLen( 0 );
        pGlue->SetAscent( 0 );
        pGlue->SetPortion( NULL );
        pGlue->SetFixWidth(0);
    }

    // Luxus: Wir sorgen durch das Aufspannen von Glues dafuer,
    // dass der QuoVadis-Text rechts erscheint:
    long nQuoWidth = pQuo->Width();
    nLastLeft -= pQuo->Width();
    if( nLastLeft )
    {
        if( nLastLeft > pQuo->GetAscent() ) // Mindestabstand
        {
            switch( GetAdjust() )
            {
                case SVX_ADJUST_BLOCK:
                {
                    if( !pCurr->GetLen() ||
                        CH_BREAK != GetInfo().GetChar(nStart+pCurr->GetLen()-1))
                        nLastLeft = pQuo->GetAscent();
                    nQuoWidth += nLastLeft;
                    break;
                }
                case SVX_ADJUST_RIGHT:
                {
                    nLastLeft = pQuo->GetAscent();
                    nQuoWidth += nLastLeft;
                    break;
                }
                case SVX_ADJUST_CENTER:
                {
                    nQuoWidth += pQuo->GetAscent();
                    long nDiff = nLastLeft - nQuoWidth;
                    if( nDiff < 0 )
                    {
                        nLastLeft = pQuo->GetAscent();
                        nQuoWidth = -nDiff + nLastLeft;
                    }
                    else
                    {
                        nQuoWidth = 0;
                        nLastLeft = ( pQuo->GetAscent() + nDiff ) / 2;
                    }
                    break;
                }
                default:
                    nQuoWidth += nLastLeft;
            }
        }
        else
            nQuoWidth += nLastLeft;
        if( nLastLeft )
        {
            pGlue = new SwGluePortion(0);
            pGlue->Width( nLastLeft );
            pPor->Append( pGlue );
            pPor = pPor->GetPortion();
        }
    }

    // Jetzt aber: die QuoVadis-Portion wird angedockt:
    pPor->Append( pQuo );
    pCurr->Width( pCurr->Width() + KSHORT( nQuoWidth ) );

    // Und noch einmal adjustieren wegen des Adjustment und nicht zu Letzt
    // wegen folgendem Sonderfall: In der Zeile hat der DummUser durchgaengig
    // einen kleineren Font eingestellt als der vom QuoVadis-Text ...
    CalcAdjustLine( pCurr );

#ifdef DEBUG
    if( OPTDBG( rInf ) )
    {
//        aDbstream << "FormatQuoVadis:" << endl;
//        pCurr->DebugPortions( aDbstream, rInf.GetTxt(), nStart );
    }
#endif

    // Uff...
    return nRet;
}


/*************************************************************************
 *                  SwTxtFormatter::MakeDummyLine()
 *************************************************************************/

// MakeDummyLine() erzeugt eine Line, die bis zum unteren Seitenrand
// reicht. DummyLines bzw. DummyPortions sorgen dafuer, dass Oszillationen
// zum stehen kommen, weil Rueckflussmoeglichkeiten genommen werden.
// Sie werden bei absatzgebundenen Frames in Fussnoten und bei Ftn-
// Oszillationen verwendet.

void SwTxtFormatter::MakeDummyLine()
{
    KSHORT nRstHeight = GetFrmRstHeight();
    if( pCurr && nRstHeight > pCurr->Height() )
    {
        SwLineLayout *pLay = new SwLineLayout;
        nRstHeight -= pCurr->Height();
        pLay->Height( nRstHeight );
        pLay->SetAscent( nRstHeight );
        Insert( pLay );
        Next();
    }
}

/*************************************************************************
 *                     SwFtnSave::SwFtnSave()
 *************************************************************************/

SwFtnSave::SwFtnSave( const SwTxtSizeInfo &rInf, const SwTxtFtn* pTxtFtn )
    : pInf( &((SwTxtSizeInfo&)rInf) )
{
    if( pTxtFtn && rInf.GetTxtFrm() )
    {
        pFnt = ((SwTxtSizeInfo&)rInf).GetFont();
          pOld = new SwFont( *pFnt );
        pOld->GetTox() = pFnt->GetTox();
        pFnt->GetTox() = 0;
        SwFmtFtn& rFtn = (SwFmtFtn&)pTxtFtn->GetFtn();
        const SwDoc *pDoc = rInf.GetTxtFrm()->GetNode()->GetDoc();

        const SwEndNoteInfo* pInfo;
        if( rFtn.IsEndNote() )
            pInfo = &pDoc->GetEndNoteInfo();
        else
            pInfo = &pDoc->GetFtnInfo();
        const SwAttrSet& rSet = pInfo->GetAnchorCharFmt((SwDoc&)*pDoc)->GetAttrSet();
        pFnt->SetDiffFnt( &rSet );
        pFnt->ChgPhysFnt( pInf->GetVsh(), pInf->GetOut() );
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_BACKGROUND,
            sal_True, &pItem ))
            pFnt->SetBackColor( new Color( ((SvxBrushItem*)pItem)->GetColor() ) );
    }
    else
        pFnt = NULL;
}

/*************************************************************************
 *                      SwFtnPortion::SwFtnPortion()
 *************************************************************************/

SwFtnPortion::SwFtnPortion( const XubString &rExpand,
                            SwTxtFrm *pFrm, SwTxtFtn *pFtn, KSHORT nReal )
        : aExpand( rExpand ), pFrm(pFrm), pFtn(pFtn), nOrigHeight( nReal )
{
    SetLen(1);
    SetWhichPor( POR_FTN );
}

/*************************************************************************
 *                      SwFtnPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwFtnPortion::GetExpTxt( const SwTxtSizeInfo &, XubString &rTxt ) const
{
    rTxt = aExpand;
    return sal_True;
}

void SwFtnPortion::ClearFtn()
{
    if( pFrm && (!pFrm->IsInSct() ||
        !SwLayouter::Collecting( pFrm->GetNode()->GetDoc(),
                                 pFrm->FindSctFrm(), NULL ) ) )
        pFrm->FindFtnBossFrm( !pFtn->GetFtn().IsEndNote() )
                              ->RemoveFtn( pFrm, pFtn );
}


/*************************************************************************
 *                 virtual SwFtnPortion::Format()
 *************************************************************************/

sal_Bool SwFtnPortion::Format( SwTxtFormatInfo &rInf )
{
    SwFtnSave aFtnSave( rInf, pFtn );
    sal_Bool bFull = SwExpandPortion::Format( rInf );
    SetAscent( rInf.GetAscent() );
    rInf.SetFtnDone( !bFull );
    if( !bFull )
        rInf.SetParaFtn();
    return bFull;
}

/*************************************************************************
 *               virtual SwFtnPortion::Paint()
 *************************************************************************/

void SwFtnPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    SwFtnSave aFtnSave( rInf, pFtn );
    rInf.DrawViewOpt( *this, POR_FTN );
    SwExpandPortion::Paint( rInf );
}

/*************************************************************************
 *               virtual SwFtnPortion::GetTxtSize()
 *************************************************************************/

SwPosSize SwFtnPortion::GetTxtSize( const SwTxtSizeInfo &rInfo ) const
{
    SwFtnSave aFtnSave( rInfo, pFtn );
    return SwExpandPortion::GetTxtSize( rInfo );
}

/*************************************************************************
 *                      class SwQuoVadisPortion
 *************************************************************************/

SwFldPortion *SwQuoVadisPortion::Clone( const XubString &rExpand ) const
{ return new SwQuoVadisPortion( rExpand, aErgo ); }

SwQuoVadisPortion::SwQuoVadisPortion( const XubString &rExp, const XubString& rStr )
    : SwFldPortion( rExp ), aErgo(rStr)
{
    SetLen(0);
    SetWhichPor( POR_QUOVADIS );
}

/*************************************************************************
 *                 virtual SwQuoVadisPortion::Format()
 *************************************************************************/

sal_Bool SwQuoVadisPortion::Format( SwTxtFormatInfo &rInf )
{
    // erster Versuch, vielleicht passt der Text
    sal_Bool bFull = SwFldPortion::Format( rInf );
    SetLen( 0 );

    if( bFull )
    {
        // zweiter Versuch, wir kuerzen den String:
        aExpand = XubString( "...", RTL_TEXTENCODING_MS_1252 );
        bFull = SwFldPortion::Format( rInf );
        SetLen( 0 );
        if( bFull  )
            // dritter Versuch, es langt: jetzt wird gestaucht:
            Width( rInf.Width() - rInf.X() );

        // 8317: keine mehrzeiligen Felder bei QuoVadis und ErgoSum
        if( rInf.GetRest() )
        {
            delete rInf.GetRest();
            rInf.SetRest( 0 );
        }
    }
    return bFull;
}

/*************************************************************************
 *               virtual SwQuoVadisPortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwQuoVadisPortion::GetExpTxt( const SwTxtSizeInfo &, XubString &rTxt ) const
{
    rTxt = aExpand;
    rTxt += aErgo;
    return sal_True;
}

/*************************************************************************
 *               virtual SwQuoVadisPortion::Paint()
 *************************************************************************/

void SwQuoVadisPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    // Wir wollen _immer_ per DrawStretchText ausgeben,
    // weil nErgo schnell mal wechseln kann.
    if( PrtWidth() )
    {
        rInf.DrawViewOpt( *this, POR_QUOVADIS );
        SwTxtSlotLen aDiffTxt( &rInf, this );
        rInf.DrawText( *this, rInf.GetLen(), sal_True );
    }
}

/*************************************************************************
 *                      class SwErgoSumPortion
 *************************************************************************/

SwFldPortion *SwErgoSumPortion::Clone( const XubString &rExpand ) const
{
#ifdef ENABLEUNICODE
    UniString aTmp = UniString::CreateFromInt32( 0 );
#else
    XubString aTmp( MSHORT(0) );
#endif
    return new SwErgoSumPortion( rExpand, aTmp );
}

SwErgoSumPortion::SwErgoSumPortion( const XubString &rExp, const XubString& rStr )
    : SwFldPortion( rExp )
{
    SetLen(0);
    aExpand += rStr;

    // 7773: sinnvolle Massnahme: ein Blank Abstand zum Text
    aExpand += ' ';
    SetWhichPor( POR_ERGOSUM );
}

xub_StrLen SwErgoSumPortion::GetCrsrOfst( const KSHORT ) const
{
    return 0;
}

/*************************************************************************
 *                 virtual SwErgoSumPortion::Format()
 *************************************************************************/

sal_Bool SwErgoSumPortion::Format( SwTxtFormatInfo &rInf )
{
    sal_Bool bFull = SwFldPortion::Format( rInf );
    SetLen( 0 );
    rInf.SetErgoDone( 0 != Width() );

    // 8317: keine mehrzeiligen Felder bei QuoVadis und ErgoSum
    if( rInf.GetRest() )
    {
        delete rInf.GetRest();
        rInf.SetRest( 0 );
    }
    return bFull;
}

/*************************************************************************
 * sal_Bool SwFtnNumPortion::DiffFont()
 *  liefert sal_True, wenn der Font der FtnReferenz (pFont) eine Aenderung
 *  des Fonts der FtnNumerierung (pFnt) erforderlich macht.
 *  Die Bedingungen sind ein Spiegel dessen, was in NewFtnNumPortion steht
 *************************************************************************/

sal_Bool SwFtnNumPortion::DiffFont( SwFont* pFont )
{
    if( pFnt->GetName() != pFont->GetName() ||
        pFnt->GetStyleName() != pFont->GetStyleName() ||
        pFnt->GetFamily() != pFont->GetFamily() ||
        pFont->GetCharSet() != pFnt->GetCharSet() )
        return sal_True;
    return sal_False;
}

/*************************************************************************
 *                      SwParaPortion::SetErgoSumNum()
 *************************************************************************/

void SwParaPortion::SetErgoSumNum( const XubString& rErgo )
{
    SwLineLayout *pLay = this;
    while( pLay->GetNext() )
    {
        DBG_LOOP;
        pLay = pLay->GetNext();
    }
    SwLinePortion     *pPor = pLay;
    SwQuoVadisPortion *pQuo = 0;
    while( pPor && !pQuo )
    {
        if ( pPor->IsQuoVadisPortion() )
            pQuo = (SwQuoVadisPortion*)pPor;
        pPor = pPor->GetPortion();
    }
    if( pQuo )
        pQuo->SetNumber( rErgo );
}

/*************************************************************************
 *                      SwParaPortion::UpdateQuoVadis()
 *
 * Wird im SwTxtFrm::Prepare() gerufen
 *************************************************************************/

sal_Bool SwParaPortion::UpdateQuoVadis( const XubString &rQuo )
{
    SwLineLayout *pLay = this;
    while( pLay->GetNext() )
    {
        DBG_LOOP;
        pLay = pLay->GetNext();
    }
    SwLinePortion     *pPor = pLay;
    SwQuoVadisPortion *pQuo = 0;
    while( pPor && !pQuo )
    {
        if ( pPor->IsQuoVadisPortion() )
            pQuo = (SwQuoVadisPortion*)pPor;
        pPor = pPor->GetPortion();
    }

    if( !pQuo )
        return sal_False;

    return pQuo->GetQuoTxt() == rQuo;
}



