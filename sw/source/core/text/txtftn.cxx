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

#include "viewsh.hxx"
#include "doc.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "ndtxt.hxx"
#include "txtatr.hxx"
#include <SwPortionHandler.hxx>
#include <txtftn.hxx>
#include <flyfrm.hxx>
#include <fmtftn.hxx>
#include <ftninfo.hxx>
#include <charfmt.hxx>
#include <dflyobj.hxx>
#include <rowfrm.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <breakit.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <tabfrm.hxx>
// #i28701#
#include <sortedobjs.hxx>

#include "swfont.hxx"   // new SwFont
#include "porftn.hxx"
#include "porfly.hxx"
#include "porlay.hxx"
#include "txtfrm.hxx"
#include "itrform2.hxx"
#include "ftnfrm.hxx"   // FindQuoVadisFrm(),
#include "pagedesc.hxx"
#include "redlnitr.hxx" // SwRedlnItr
#include "sectfrm.hxx"  // SwSectionFrm
#include "layouter.hxx" // Endnote-Collection
#include "frmtool.hxx"
#include "ndindex.hxx"

using namespace ::com::sun::star;

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

#ifdef DBG_UTIL
void SwTxtFrm::CalcFtnFlag( xub_StrLen nStop )//For testing the SplitFrm
#else
void SwTxtFrm::CalcFtnFlag()
#endif
{
    bFtn = sal_False;

    const SwpHints *pHints = GetTxtNode()->GetpSwpHints();
    if( !pHints )
        return;

    const sal_uInt16 nSize = pHints->Count();

#ifdef DBG_UTIL
    const xub_StrLen nEnd = nStop != STRING_LEN ? nStop
                        : GetFollow() ? GetFollow()->GetOfst() : STRING_LEN;
#else
    const xub_StrLen nEnd = GetFollow() ? GetFollow()->GetOfst() : STRING_LEN;
#endif

    for ( sal_uInt16 i = 0; i < nSize; ++i )
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
    OSL_ENSURE( HasFtn(), "Wer ruft mich da?" );
    SwFtnBossFrm *pBoss = FindFtnBossFrm( sal_True );
    const SwFtnFrm *pFtn = pBoss->FindFirstFtn( this );
    if( pFtn && FTNPOS_CHAPTER != GetNode()->GetDoc()->GetFtnInfo().ePos &&
        ( !pBoss->GetUpper()->IsSctFrm() ||
        !((SwSectionFrm*)pBoss->GetUpper())->IsFtnAtEnd() ) )
    {
        const SwFtnContFrm *pCont = pBoss->FindFtnCont();
        sal_Bool bReArrange = sal_True;

        SWRECTFN( this )
        if ( pCont && (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(),
                                          (Frm().*fnRect->fnGetBottom)() ) > 0 )
        {
            pBoss->RearrangeFtns( (Frm().*fnRect->fnGetBottom)(), sal_False,
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
 *                      lcl_GetFtnLower()
 *
 * Local helper function. Checks if nLower should be taken as the boundary
 * for the footnote.
 *************************************************************************/

static SwTwips lcl_GetFtnLower( const SwTxtFrm* pFrm, SwTwips nLower )
{
    // nLower is an absolute value. It denotes the bottom of the line
    // containing the footnote.
    SWRECTFN( pFrm )

    OSL_ENSURE( !pFrm->IsVertical() || !pFrm->IsSwapped(),
            "lcl_GetFtnLower with swapped frame" );

    SwTwips nAdd;
    SwTwips nRet = nLower;

    //
    // Check if text is inside a table.
    //
    if ( pFrm->IsInTab() )
    {
        //
        // If pFrm is inside a table, we have to check if
        // a) The table is not allowed to split or
        // b) The table row is not allowed to split
        //
        // Inside a table, there are no footnotes,
        // see SwFrm::FindFtnBossFrm. So we don't have to check
        // the case that pFrm is inside a (footnote collecting) section
        // within the table.
        //
        const SwFrm* pRow = pFrm;
        while( !pRow->IsRowFrm() || !pRow->GetUpper()->IsTabFrm() )
            pRow = pRow->GetUpper();
        const SwTabFrm* pTabFrm = (SwTabFrm*)pRow->GetUpper();

        OSL_ENSURE( pTabFrm && pRow &&
                pRow->GetUpper()->IsTabFrm(), "Upper of row should be tab" );

        const sal_Bool bDontSplit = !pTabFrm->IsFollow() &&
                                !pTabFrm->IsLayoutSplitAllowed();

        SwTwips nMin = 0;
        if ( bDontSplit )
            nMin = (pTabFrm->Frm().*fnRect->fnGetBottom)();
        else if ( !((SwRowFrm*)pRow)->IsRowSplitAllowed() )
            nMin = (pRow->Frm().*fnRect->fnGetBottom)();

        if ( nMin && (*fnRect->fnYDiff)( nMin, nLower ) > 0 )
            nRet = nMin;

        nAdd = (pRow->GetUpper()->*fnRect->fnGetBottomMargin)();
    }
    else
        nAdd = (pFrm->*fnRect->fnGetBottomMargin)();

    if( nAdd > 0 )
    {
        if ( bVert )
            nRet -= nAdd;
        else
            nRet += nAdd;
    }

    // #i10770#: If there are fly frames anchored at previous paragraphs,
    // the deadline should consider their lower borders.
    const SwFrm* pStartFrm = pFrm->GetUpper()->GetLower();
    OSL_ENSURE( pStartFrm, "Upper has no lower" );
    SwTwips nFlyLower = bVert ? LONG_MAX : 0;
    while ( pStartFrm != pFrm )
    {
        OSL_ENSURE( pStartFrm, "Frame chain is broken" );
        if ( pStartFrm->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pStartFrm->GetDrawObjs();
            for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                SwRect aRect( pAnchoredObj->GetObjRect() );

                if ( !pAnchoredObj->ISA(SwFlyFrm) ||
                     static_cast<SwFlyFrm*>(pAnchoredObj)->IsValid() )
                {
                    const SwTwips nBottom = (aRect.*fnRect->fnGetBottom)();
                    if ( (*fnRect->fnYDiff)( nBottom, nFlyLower ) > 0 )
                        nFlyLower = nBottom;
                }
            }
        }

        pStartFrm = pStartFrm->GetNext();
    }

    if ( bVert )
        nRet = Min( nRet, nFlyLower );
    else
        nRet = Max( nRet, nFlyLower );

    return nRet;
}


/*************************************************************************
 *                      SwTxtFrm::GetFtnLine()
 *************************************************************************/

SwTwips SwTxtFrm::GetFtnLine( const SwTxtFtn *pFtn ) const
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),
            "SwTxtFrm::GetFtnLine with swapped frame" );

    SwTxtFrm *pThis = (SwTxtFrm*)this;

    if( !HasPara() )
    {
        // #109071# GetFormatted() does not work here, bacause most probably
        // the frame is currently locked. We return the previous value.
        return pThis->mnFtnLine > 0 ?
               pThis->mnFtnLine :
               IsVertical() ? Frm().Left() : Frm().Bottom();
    }

    SWAP_IF_NOT_SWAPPED( this )

    SwTxtInfo aInf( pThis );
    SwTxtIter aLine( pThis, &aInf );
    const xub_StrLen nPos = *pFtn->GetStart();
    aLine.CharToLine( nPos );

    SwTwips nRet = aLine.Y() + SwTwips(aLine.GetLineHeight());
    if( IsVertical() )
        nRet = SwitchHorizontalToVertical( nRet );

    UNDO_SWAP( this )

    nRet = lcl_GetFtnLower( pThis, nRet );

    pThis->mnFtnLine = nRet;
    return nRet;
}

/*************************************************************************
 *                      SwTxtFrm::GetFtnRstHeight()
 *************************************************************************/

// Ermittelt die max. erreichbare Hoehe des TxtFrm im Ftn-Bereich.
// Sie wird eingeschraenkt durch den unteren Rand der Zeile mit
// der Ftn-Referenz.

SwTwips SwTxtFrm::_GetFtnFrmHeight() const
{
    OSL_ENSURE( !IsFollow() && IsInFtn(), "SwTxtFrm::SetFtnLine: moon walk" );

    const SwFtnFrm *pFtnFrm = FindFtnFrm();
    const SwTxtFrm *pRef = (const SwTxtFrm *)pFtnFrm->GetRef();
    const SwFtnBossFrm *pBoss = FindFtnBossFrm();
    if( pBoss != pRef->FindFtnBossFrm( !pFtnFrm->GetAttr()->
                                        GetFtn().IsEndNote() ) )
        return 0;

    SWAP_IF_SWAPPED( this )

    SwTwips nHeight = pRef->IsInFtnConnect() ?
                            1 : pRef->GetFtnLine( pFtnFrm->GetAttr() );
    if( nHeight )
    {
        // So komisch es aussehen mag: Die erste Ftn auf der Seite darf sich
        // nicht mit der Ftn-Referenz beruehren, wenn wir im Ftn-Bereich Text
        // eingeben.
        const SwFrm *pCont = pFtnFrm->GetUpper();
        //Hoehe innerhalb des Cont, die ich mir 'eh noch genehmigen darf.
        SWRECTFN( pCont )
        SwTwips nTmp = (*fnRect->fnYDiff)( (pCont->*fnRect->fnGetPrtBottom)(),
                                           (Frm().*fnRect->fnGetTop)() );

#if OSL_DEBUG_LEVEL > 0
        if( nTmp < 0 )
        {
            sal_Bool bInvalidPos = sal_False;
            const SwLayoutFrm* pTmp = GetUpper();
            while( !bInvalidPos && pTmp )
            {
                bInvalidPos = !pTmp->GetValidPosFlag() ||
                               !pTmp->Lower()->GetValidPosFlag();
                if( pTmp == pCont )
                    break;
                pTmp = pTmp->GetUpper();
            }
            OSL_ENSURE( bInvalidPos, "Hanging below FtnCont" );
        }
#endif

        if ( (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(), nHeight) > 0 )
        {
            //Wachstumspotential den Containers.
            if ( !pRef->IsInFtnConnect() )
            {
                SwSaveFtnHeight aSave( (SwFtnBossFrm*)pBoss, nHeight  );
                nHeight = ((SwFtnContFrm*)pCont)->Grow( LONG_MAX, sal_True );
            }
            else
                nHeight = ((SwFtnContFrm*)pCont)->Grow( LONG_MAX, sal_True );

            nHeight += nTmp;
            if( nHeight < 0 )
                nHeight = 0;
        }
        else
        {   // The container has to shrink
            nTmp += (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(), nHeight);
            if( nTmp > 0 )
                nHeight = nTmp;
            else
                nHeight = 0;
        }
    }

    UNDO_SWAP( this )

    return nHeight;
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
    sal_uInt16 nSize = pHints->Count();
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
        for ( sal_uInt16 i = nSize; i; )
        {
            SwTxtAttr *pHt = pHints->GetTextHint(--i);
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
                //    -> RemoveFtn()  (vielleicht sogar ein OSL_ENSURE(wert)
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
                        OSL_ENSURE( !pDest->FindFtnBossFrm( !bEndn )->FindFtn(
                            pDest,pFtn),"SwTxtFrm::RemoveFtn: footnote exists");

                        //Nicht ummelden sondern immer Moven.
                        // OD 08.11.2002 #104840# - use <SwlayoutFrm::IsBefore(::)>
                        if ( bEndDoc ||
                             !pFtnFrm->FindFtnBossFrm()->IsBefore( pDest->FindFtnBossFrm( !bEndn ) )
                           )
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

                        OSL_ENSURE( pDest->FindFtnBossFrm( !bEndn )->FindFtn( pDest,
                           pFtn),"SwTxtFrm::RemoveFtn: footnote ChgRef failed");
                    }
                    else
                    {
                        if( !bEndDoc || ( bEndn && pEndBoss->IsInSct() &&
                            !SwLayouter::Collecting( GetNode()->GetDoc(),
                            pEndBoss->FindSctFrm(), NULL ) ) )
                        {
                            if( bEndn )
                                pEndBoss->RemoveFtn( this, pFtn );
                            else
                                pFtnBoss->RemoveFtn( this, pFtn );
                            bRemove = bRemove || !bEndDoc;
                            OSL_ENSURE( bEndn ? !pEndBoss->FindFtn( this, pFtn ) :
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
    OSL_ENSURE( !IsVertical() || !IsSwapped(),
            "SwTxtFrm::ConnectFtn with swapped frame" );

    bFtn = sal_True;
    bInFtnConnect = sal_True;   //Bloss zuruecksetzen!
    sal_Bool bEnd = pFtn->GetFtn().IsEndNote();

    //
    // We want to store this value, because it is needed as a fallback
    // in GetFtnLine(), if there is no paragraph information available
    //
    mnFtnLine = nDeadLine;

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

#if OSL_DEBUG_LEVEL > 1
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
                SwFtnFrm *pNew = new SwFtnFrm(pDoc->GetDfltFrmFmt(),this,this,pFtn);
                 SwNodeIndex aIdx( *pFtn->GetStartNode(), 1 );
                 ::_InsertCnt( pNew, pDoc, aIdx.GetIndex() );
                GetNode()->getIDocumentLayoutAccess()->GetLayouter()->CollectEndnote( pNew );
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

            SWRECTFN ( pCont )
            long nDiff = (*fnRect->fnYDiff)( (pCont->Frm().*fnRect->fnGetTop)(),
                                             nDeadLine );

            if( nDiff >= 0 )
            {
                //Wenn die Fussnote bei einem Follow angemeldet ist, so ist
                //es jetzt an der Zeit sie umzumelden.
                if ( pSrcFrm != this )
                    pBoss->ChangeFtnRef( pSrcFrm, pFtn, this );
                //Es steht Platz zur Verfuegung, also kann die Fussnote evtl.
                //wachsen.
                if ( pFtnFrm->GetFollow() && nDiff > 0 )
                {
                    SwTwips nHeight = (pCont->Frm().*fnRect->fnGetHeight)();
                    pBoss->RearrangeFtns( nDeadLine, sal_False, pFtn );
                    ValidateBodyFrm();
                    ValidateFrm();
                    ViewShell *pSh = getRootFrm()->GetCurrShell();
                    if ( pSh && nHeight == (pCont->Frm().*fnRect->fnGetHeight)() )
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
                // OD 08.11.2002 #104840# - use <SwLayoutFrm::IsBefore(..)>
                else if ( !pFtnFrm->GetPrev() ||
                          pFtnBoss->IsBefore( pBoss )
                        )
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

#if OSL_DEBUG_LEVEL > 1
    // pFtnFrm kann sich durch Calc veraendert haben ...
    SwFtnFrm *pFtnFrm = pBoss->FindFtn( this, pFtn );
    if( pFtnFrm && pBoss != pFtnFrm->FindFtnBossFrm( !bEnd ) )
    {
        int bla = 5;
        (void)bla;
    }
    nRstHeight = GetRstHeight();
    (void)nRstHeight;
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
    OSL_ENSURE( ! pFrm->IsVertical() || pFrm->IsSwapped(),
            "NewFtnPortion with unswapped frame" );

    if( !pFrm->IsFtnAllowed() )
        return 0;

    SwTxtFtn  *pFtn = (SwTxtFtn*)pHint;
    SwFmtFtn& rFtn = (SwFmtFtn&)pFtn->GetFtn();
    SwDoc *pDoc = pFrm->GetNode()->GetDoc();

    if( rInf.IsTest() )
        return new SwFtnPortion( rFtn.GetViewNumStr( *pDoc ), pFrm, pFtn );

    SWAP_IF_SWAPPED( pFrm )

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

    const bool bVertical = pFrm->IsVertical();
    if( bVertical )
        nLower = pFrm->SwitchHorizontalToVertical( nLower );

    nLower = lcl_GetFtnLower( pFrm, nLower );

    //6995: Wir frischen nur auf. Das Connect tut fuer diesen Fall nix
    //Brauchbares, sondern wuerde stattdessen fuer diesen Fall meist die
    //Ftn wegwerfen und neu erzeugen.

    if( !rInf.IsQuick() )
        pFrm->ConnectFtn( pFtn, nLower );

    SwTxtFrm *pScrFrm = pFrm->FindFtnRef( pFtn );
    SwFtnBossFrm *pBoss = pFrm->FindFtnBossFrm( !rFtn.IsEndNote() );
    SwFtnFrm *pFtnFrm = NULL;
    if( pScrFrm )
        pFtnFrm = pBoss->FindFtn( pScrFrm, pFtn );

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
                    UNDO_SWAP( pFrm )
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
                            SwFtnFrm* pTmpFrm = (SwFtnFrm*)pFtnC->Lower();
                            if( pTmpFrm && *pTmpFrm < pFtn )
                            {
                                rInf.SetStop( sal_True );
                                UNDO_SWAP( pFrm )
                                return 0;
                            }
                        }
                    }
                    // Ist dies die letzte passende Zeile?
                    SwTwips nTmpBot = Y() + nReal * 2;

                    if( bVertical )
                        nTmpBot = pFrm->SwitchHorizontalToVertical( nTmpBot );

                    SWRECTFN( pFtnCont )

                    const long nDiff = (*fnRect->fnYDiff)(
                                            (pFtnCont->Frm().*fnRect->fnGetTop)(),
                                             nTmpBot );

                    if( pScrFrm && nDiff < 0 )
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
                                UNDO_SWAP( pFrm )
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

    UNDO_SWAP( pFrm )

    return pRet;
 }

/*************************************************************************
 *                      SwTxtFormatter::NewFtnNumPortion()
 *************************************************************************/

// Die Portion fuer die Ftn-Nummerierung im Ftn-Bereich

SwNumberPortion *SwTxtFormatter::NewFtnNumPortion( SwTxtFormatInfo &rInf ) const
{
    OSL_ENSURE( pFrm->IsInFtn() && !pFrm->GetIndPrev() && !rInf.IsFtnDone(),
            "This is the wrong place for a ftnnumber" );
    if( rInf.GetTxtStart() != nStart ||
        rInf.GetTxtStart() != rInf.GetIdx() )
        return 0;

    const SwFtnFrm* pFtnFrm = pFrm->FindFtnFrm();
    const SwTxtFtn* pFtn = pFtnFrm->GetAttr();

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
    const IDocumentSettingAccess* pIDSA = pFrm->GetTxtNode()->getIDocumentSettingAccess();
    SwFont *pNumFnt = new SwFont( pParSet, pIDSA );

    // #i37142#
    // Underline style of paragraph font should not be considered
    // Overline style of paragraph font should not be considered
    // Weight style of paragraph font should not be considered
    // Posture style of paragraph font should not be considered
    // See also #i18463# and SwTxtFormatter::NewNumberPortion()
    pNumFnt->SetUnderline( UNDERLINE_NONE );
    pNumFnt->SetOverline( UNDERLINE_NONE );
    pNumFnt->SetItalic( ITALIC_NONE, SW_LATIN );
    pNumFnt->SetItalic( ITALIC_NONE, SW_CJK );
    pNumFnt->SetItalic( ITALIC_NONE, SW_CTL );
    pNumFnt->SetWeight( WEIGHT_NORMAL, SW_LATIN );
    pNumFnt->SetWeight( WEIGHT_NORMAL, SW_CJK );
    pNumFnt->SetWeight( WEIGHT_NORMAL, SW_CTL );

    pNumFnt->SetDiffFnt(&rSet, pIDSA );
    pNumFnt->SetVertical( pNumFnt->GetOrientation(), pFrm->IsVertical() );

    SwFtnNumPortion* pNewPor = new SwFtnNumPortion( aFtnTxt, pNumFnt );
    pNewPor->SetLeft( !pFrm->IsRightToLeft() );
    return pNewPor;
}

/*************************************************************************
 *                  SwTxtFormatter::NewErgoSumPortion()
 *************************************************************************/

XubString lcl_GetPageNumber( const SwPageFrm* pPage )
{
    OSL_ENSURE( pPage, "GetPageNumber: Homeless TxtFrm" );
    MSHORT nVirtNum = pPage->GetVirtPageNum();
    const SvxNumberType& rNum = pPage->GetPageDesc()->GetNumType();
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
    OSL_ENSURE( ! pFrm->IsVertical() || ! pFrm->IsSwapped(),
            "SwTxtFormatter::FormatQuoVadis with swapped frame" );

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
        if( pPage == pErgoPage )
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
    sal_uInt16 nQuoWidth = pQuo->Width();
    SwLinePortion* pCurrPor = pQuo;

    while ( rInf.GetRest() )
    {
        SwLinePortion* pFollow = rInf.GetRest();
        rInf.SetRest( 0 );
        pCurrPor->Move( rInf );

        OSL_ENSURE( pFollow->IsQuoVadisPortion(),
                "Quo Vadis, rest of QuoVadisPortion" );

        // format the rest and append it to the other QuoVadis parts
        pFollow->Format( rInf );
        nQuoWidth = nQuoWidth + pFollow->Width();

        pCurrPor->Append( pFollow );
        pCurrPor = pFollow;
    }

    nLastLeft = nOldRealWidth - nQuoWidth;
    Right( Right() - nQuoWidth );

    SWAP_IF_NOT_SWAPPED( pFrm )

    const xub_StrLen nRet = FormatLine( nStart );

    UNDO_SWAP( pFrm )

    Right( rInf.Left() + nOldRealWidth - 1 );

    nLastLeft = nOldRealWidth - pCurr->Width();
    FeedInf( rInf );

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
    nLastLeft = nLastLeft - nQuoWidth;
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
                    nQuoWidth = nQuoWidth + nLastLeft;
                    break;
                }
                case SVX_ADJUST_RIGHT:
                {
                    nLastLeft = pQuo->GetAscent();
                    nQuoWidth = nQuoWidth + nLastLeft;
                    break;
                }
                case SVX_ADJUST_CENTER:
                {
                    nQuoWidth = nQuoWidth + pQuo->GetAscent();
                    long nDiff = nLastLeft - nQuoWidth;
                    if( nDiff < 0 )
                    {
                        nLastLeft = pQuo->GetAscent();
                        nQuoWidth = (sal_uInt16)(-nDiff + nLastLeft);
                    }
                    else
                    {
                        nQuoWidth = 0;
                        nLastLeft = sal_uInt16(( pQuo->GetAscent() + nDiff ) / 2);
                    }
                    break;
                }
                default:
                    nQuoWidth = nQuoWidth + nLastLeft;
            }
        }
        else
            nQuoWidth = nQuoWidth + nLastLeft;
        if( nLastLeft )
        {
            pGlue = new SwGluePortion(0);
            pGlue->Width( nLastLeft );
            pPor->Append( pGlue );
            pPor = pPor->GetPortion();
        }
    }

    // Jetzt aber: die QuoVadis-Portion wird angedockt:
    pCurrPor = pQuo;
    while ( pCurrPor )
    {
        // pPor->Append deletes the pPortoin pointer of pPor. Therefore
        // we have to keep a pointer to the next portion
        pQuo = (SwQuoVadisPortion*)pCurrPor->GetPortion();
        pPor->Append( pCurrPor );
        pPor = pPor->GetPortion();
        pCurrPor = pQuo;
    }

    pCurr->Width( pCurr->Width() + KSHORT( nQuoWidth ) );

    // Und noch einmal adjustieren wegen des Adjustment und nicht zu Letzt
    // wegen folgendem Sonderfall: In der Zeile hat der DummUser durchgaengig
    // einen kleineren Font eingestellt als der vom QuoVadis-Text ...
    CalcAdjustLine( pCurr );

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
        nRstHeight = nRstHeight - pCurr->Height();
        pLay->Height( nRstHeight );
        pLay->SetAscent( nRstHeight );
        Insert( pLay );
        Next();
    }
}

/*************************************************************************
 *                      class SwFtnSave
  *************************************************************************/
class SwFtnSave
{
    SwTxtSizeInfo *pInf;
    SwFont       *pFnt;
    SwFont       *pOld;
public:
    SwFtnSave( const SwTxtSizeInfo &rInf,
               const SwTxtFtn *pTxtFtn,
               const bool bApplyGivenScriptType,
               const sal_uInt8 nGivenScriptType );
   ~SwFtnSave();
};

/*************************************************************************
 *                     SwFtnSave::SwFtnSave()
 *************************************************************************/

SwFtnSave::SwFtnSave( const SwTxtSizeInfo &rInf,
                      const SwTxtFtn* pTxtFtn,
                      const bool bApplyGivenScriptType,
                      const sal_uInt8 nGivenScriptType )
    : pInf( &((SwTxtSizeInfo&)rInf) )
    , pFnt( 0 )
    , pOld( 0 )
{
    if( pTxtFtn && rInf.GetTxtFrm() )
    {
        pFnt = ((SwTxtSizeInfo&)rInf).GetFont();
          pOld = new SwFont( *pFnt );
        pOld->GetTox() = pFnt->GetTox();
        pFnt->GetTox() = 0;
        SwFmtFtn& rFtn = (SwFmtFtn&)pTxtFtn->GetFtn();
        const SwDoc *pDoc = rInf.GetTxtFrm()->GetNode()->GetDoc();

        // #i98418#
        if ( bApplyGivenScriptType )
        {
            pFnt->SetActual( nGivenScriptType );
        }
        else
        {
            // examine text and set script
            String aTmpStr( rFtn.GetViewNumStr( *pDoc ) );
            pFnt->SetActual( SwScriptInfo::WhichFont( 0, &aTmpStr, 0 ) );
        }

        const SwEndNoteInfo* pInfo;
        if( rFtn.IsEndNote() )
            pInfo = &pDoc->GetEndNoteInfo();
        else
            pInfo = &pDoc->GetFtnInfo();
        const SwAttrSet& rSet = pInfo->GetAnchorCharFmt((SwDoc&)*pDoc)->GetAttrSet();
        pFnt->SetDiffFnt( &rSet, rInf.GetTxtFrm()->GetNode()->getIDocumentSettingAccess() );

        // we reduce footnote size, if we are inside a double line portion
        if ( ! pOld->GetEscapement() && 50 == pOld->GetPropr() )
        {
            Size aSize = pFnt->GetSize( pFnt->GetActual() );
            pFnt->SetSize( Size( (long)aSize.Width() / 2,
                                 (long)aSize.Height() / 2 ),
                           pFnt->GetActual() );
        }

        // set the correct rotation at the footnote font
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_ROTATE,
            sal_True, &pItem ))
            pFnt->SetVertical( ((SvxCharRotateItem*)pItem)->GetValue(),
                                rInf.GetTxtFrm()->IsVertical() );

        pFnt->ChgPhysFnt( pInf->GetVsh(), *pInf->GetOut() );

        if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_BACKGROUND,
            sal_True, &pItem ))
            pFnt->SetBackColor( new Color( ((SvxBrushItem*)pItem)->GetColor() ) );
    }
    else
        pFnt = NULL;
}

/*************************************************************************
 *                     SwFtnSave::~SwFtnSave()
 *************************************************************************/

SwFtnSave::~SwFtnSave()
{
    if( pFnt )
    {
        // SwFont zurueckstellen
        *pFnt = *pOld;
        pFnt->GetTox() = pOld->GetTox();
        pFnt->ChgPhysFnt( pInf->GetVsh(), *pInf->GetOut() );
        delete pOld;
    }
}

/*************************************************************************
 *                      SwFtnPortion::SwFtnPortion()
 *************************************************************************/

SwFtnPortion::SwFtnPortion( const XubString &rExpand, SwTxtFrm *pFrame,
                            SwTxtFtn *pFootn, KSHORT nReal )
        : SwFldPortion( rExpand, 0 )
        , pFrm(pFrame)
        , pFtn(pFootn)
        , nOrigHeight( nReal )
        // #i98418#
        , mbPreferredScriptTypeSet( false )
        , mnPreferredScriptType( SW_LATIN )
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

/*************************************************************************
 *                 virtual SwFtnPortion::Format()
 *************************************************************************/

sal_Bool SwFtnPortion::Format( SwTxtFormatInfo &rInf )
{
    // #i98418#
//    SwFtnSave aFtnSave( rInf, pFtn );
    SwFtnSave aFtnSave( rInf, pFtn, mbPreferredScriptTypeSet, mnPreferredScriptType );
    // the idx is manipulated in SwExpandPortion::Format
    // this flag indicates, that a footnote is allowed to trigger
    // an underflow during SwTxtGuess::Guess
    rInf.SetFakeLineStart( rInf.GetIdx() > rInf.GetLineStart() );
    sal_Bool bFull = SwFldPortion::Format( rInf );
    rInf.SetFakeLineStart( sal_False );
    SetAscent( rInf.GetAscent() );
    Height( rInf.GetTxtHeight() );
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
    // #i98418#
//    SwFtnSave aFtnSave( rInf, pFtn );
    SwFtnSave aFtnSave( rInf, pFtn, mbPreferredScriptTypeSet, mnPreferredScriptType );
    rInf.DrawViewOpt( *this, POR_FTN );
    SwExpandPortion::Paint( rInf );
}

/*************************************************************************
 *               virtual SwFtnPortion::GetTxtSize()
 *************************************************************************/

SwPosSize SwFtnPortion::GetTxtSize( const SwTxtSizeInfo &rInfo ) const
{
    // #i98418#
//    SwFtnSave aFtnSave( rInfo, pFtn );
    SwFtnSave aFtnSave( rInfo, pFtn, mbPreferredScriptTypeSet, mnPreferredScriptType );
    return SwExpandPortion::GetTxtSize( rInfo );
}

// #i98418#
void SwFtnPortion::SetPreferredScriptType( sal_uInt8 nPreferredScriptType )
{
    mbPreferredScriptTypeSet = true;
    mnPreferredScriptType = nPreferredScriptType;
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
    CheckScript( rInf );
    sal_Bool bFull = SwFldPortion::Format( rInf );
    SetLen( 0 );

    if( bFull )
    {
        // zweiter Versuch, wir kuerzen den String:
        aExpand = rtl::OUString("...");
        bFull = SwFldPortion::Format( rInf );
        SetLen( 0 );
        if( bFull  )
            // dritter Versuch, es langt: jetzt wird gestaucht:
            Width( sal_uInt16(rInf.Width() - rInf.X()) );

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
    // if this QuoVadisPortion has a follow, the follow is responsible for
    // the ergo text.
    if ( ! HasFollow() )
        rTxt += aErgo;
    return sal_True;
}

/*************************************************************************
 *              virtual SwQuoVadisPortion::HandlePortion()
 *************************************************************************/

void SwQuoVadisPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    String aString( aExpand );
    aString += aErgo;
    rPH.Special( GetLen(), aString, GetWhichPor() );
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
        SwTxtSlot aDiffTxt( &rInf, this, true, false );
        SwFontSave aSave( rInf, pFnt );
        rInf.DrawText( *this, rInf.GetLen(), sal_True );
    }
}

/*************************************************************************
 *                      class SwErgoSumPortion
 *************************************************************************/

SwFldPortion *SwErgoSumPortion::Clone( const XubString &rExpand ) const
{
    return new SwErgoSumPortion( rExpand, rtl::OUString() );
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
    rInf.SetErgoDone( sal_True );

    // 8317: keine mehrzeiligen Felder bei QuoVadis und ErgoSum
    if( bFull && rInf.GetRest() )
    {
        delete rInf.GetRest();
        rInf.SetRest( 0 );
    }

    // We return false in order to get some text into the current line,
    // even if it's full (better than looping)
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
