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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "viewsh.hxx"
#include "doc.hxx"
#include "viewimp.hxx"
#include "swtypes.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "txtftn.hxx"
#include "fmtftn.hxx"
#include <editeng/ulspitem.hxx>
#include <editeng/keepitem.hxx>

#include <vcl/outdev.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <fmtclbl.hxx>

#include "tabfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "pagedesc.hxx"
#include "ftninfo.hxx"
#include "sectfrm.hxx"
#include "dbg_lay.hxx"

// --> OD 2004-06-23 #i28701#
#include <sortedobjs.hxx>
#include <layouter.hxx>
// --> OD 2004-11-01 #i36347#
#include <flyfrms.hxx>
// <--

#include <ndtxt.hxx>

//------------------------------------------------------------------------
//              Move-Methoden
//------------------------------------------------------------------------

/*************************************************************************
|*
|*  SwCntntFrm::ShouldBwdMoved()
|*
|*  Beschreibung        Returnwert sagt, ob der Frm verschoben werden sollte.
|*
|*************************************************************************/


sal_Bool SwCntntFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, sal_Bool, sal_Bool & )
{
    if ( (SwFlowFrm::IsMoveBwdJump() || !IsPrevObjMove()))
    {
        //Das zurueckfliessen von Frm's ist leider etwas Zeitintensiv.
        //Der haufigste Fall ist der, dass dort wo der Frm hinfliessen
        //moechte die FixSize die gleiche ist, die der Frm selbst hat.
        //In diesem Fall kann einfach geprueft werden, ob der Frm genug
        //Platz fuer seine VarSize findet, ist dies nicht der Fall kann
        //gleich auf das Verschieben verzichtet werden.
        //Die Pruefung, ob der Frm genug Platz findet fuehrt er selbst
        //durch, dabei wird beruecksichtigt, dass er sich moeglicherweise
        //aufspalten kann.
        //Wenn jedoch die FixSize eine andere ist oder Flys im Spiel sind
        //(an der alten oder neuen Position) hat alle Prueferei keinen Sinn
        //der Frm muss dann halt Probehalber verschoben werden (Wenn ueberhaupt
        //etwas Platz zur Verfuegung steht).

        //Die FixSize der Umgebungen in denen Cntnts herumlungern ist immer
        //Die Breite.

        //Wenn mehr als ein Blatt zurueckgegangen wurde (z.B. ueberspringen
        //von Leerseiten), so muss in jedemfall gemoved werden - sonst wuerde,
        //falls der Frm nicht in das Blatt passt, nicht mehr auf die
        //dazwischenliegenden Blaetter geachtet werden.
        sal_uInt8 nMoveAnyway = 0;
        SwPageFrm * const pNewPage = pNewUpper->FindPageFrm();
        SwPageFrm *pOldPage = FindPageFrm();

        if ( SwFlowFrm::IsMoveBwdJump() )
            return sal_True;

        if( IsInFtn() && IsInSct() )
        {
            SwFtnFrm* pFtn = FindFtnFrm();
            SwSectionFrm* pMySect = pFtn->FindSctFrm();
            if( pMySect && pMySect->IsFtnLock() )
            {
                SwSectionFrm *pSect = pNewUpper->FindSctFrm();
                while( pSect && pSect->IsInFtn() )
                    pSect = pSect->GetUpper()->FindSctFrm();
                OSL_ENSURE( pSect, "Escaping footnote" );
                if( pSect != pMySect )
                    return sal_False;
            }
        }
        SWRECTFN( this )
        SWRECTFNX( pNewUpper )
        if( Abs( (pNewUpper->Prt().*fnRectX->fnGetWidth)() -
                 (GetUpper()->Prt().*fnRect->fnGetWidth)() ) > 1 )
            nMoveAnyway = 2; // Damit kommt nur noch ein _WouldFit mit Umhaengen in Frage

        // OD 2004-05-26 #i25904# - do *not* move backward,
        // if <nMoveAnyway> equals 3 and no space is left in new upper.
        nMoveAnyway |= BwdMoveNecessary( pOldPage, Frm() );
        {
            const IDocumentSettingAccess* pIDSA = pNewPage->GetFmt()->getIDocumentSettingAccess();
            SwTwips nSpace = 0;
            SwRect aRect( pNewUpper->Prt() );
            aRect.Pos() += pNewUpper->Frm().Pos();
            const SwFrm *pPrevFrm = pNewUpper->Lower();
            while ( pPrevFrm )
            {
                SwTwips nNewTop = (pPrevFrm->Frm().*fnRectX->fnGetBottom)();
                // OD 2004-03-01 #106629#:
                // consider lower spacing of last frame in a table cell
                {
                    // check, if last frame is inside table and if it includes
                    // its lower spacing.
                    if ( !pPrevFrm->GetNext() && pPrevFrm->IsInTab() &&
                         pIDSA->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) )
                    {
                        const SwFrm* pLastFrm = pPrevFrm;
                        // if last frame is a section, take its last content
                        if ( pPrevFrm->IsSctFrm() )
                        {
                            pLastFrm = static_cast<const SwSectionFrm*>(pPrevFrm)->FindLastCntnt();
                            if ( pLastFrm &&
                                 pLastFrm->FindTabFrm() != pPrevFrm->FindTabFrm() )
                            {
                                pLastFrm = pLastFrm->FindTabFrm();
                            }
                        }

                        if ( pLastFrm )
                        {
                            SwBorderAttrAccess aAccess( SwFrm::GetCache(), pLastFrm );
                            const SwBorderAttrs& rAttrs = *aAccess.Get();
                            nNewTop -= rAttrs.GetULSpace().GetLower();
                        }
                    }
                }
                (aRect.*fnRectX->fnSetTop)( nNewTop );

                pPrevFrm = pPrevFrm->GetNext();
            }

            nMoveAnyway |= BwdMoveNecessary( pNewPage, aRect);

            //determine space left in new upper frame
            nSpace = (aRect.*fnRectX->fnGetHeight)();

            if ( IsInFtn() ||
                 pIDSA->get(IDocumentSettingAccess::BROWSE_MODE) ||
                 pNewUpper->IsCellFrm() ||
                 ( pNewUpper->IsInSct() && ( pNewUpper->IsSctFrm() ||
                   ( pNewUpper->IsColBodyFrm() &&
                     !pNewUpper->GetUpper()->GetPrev() &&
                     !pNewUpper->GetUpper()->GetNext() ) ) ) )
                nSpace += pNewUpper->Grow( LONG_MAX, sal_True );

            if ( nMoveAnyway < 3 )
            {
                if ( nSpace )
                {
                    //Keine Beruecksichtigung der Fussnoten die an dem Absatz
                    //kleben, denn dies wuerde extrem unuebersichtlichen Code
                    //beduerfen (wg. Beruecksichtung der Breiten und vor allem
                    //der Flys, die ja wieder Einfluss auf die Fussnoten nehmen...).

                    // _WouldFit kann bei gleicher Breite und _nur_ selbst verankerten Flys
                    // befragt werden.
                    // _WouldFit kann auch gefragt werden, wenn _nur_ fremdverankerte Flys vorliegen,
                    // dabei ist sogar die Breite egal, da ein TestFormat in der neuen Umgebung
                    // vorgenommen wird.
                    // --> OD 2007-11-26 #b6614158#
                    const sal_uInt8 nBwdMoveNecessaryResult =
                                            BwdMoveNecessary( pNewPage, aRect);
                    const bool bObjsInNewUpper( nBwdMoveNecessaryResult == 2 ||
                                                nBwdMoveNecessaryResult == 3 );

                    return _WouldFit( nSpace, pNewUpper, nMoveAnyway == 2,
                                      bObjsInNewUpper );
                    // <--
                }
                //Bei einem spaltigen Bereichsfrischling kann _WouldFit kein
                //brauchbares Ergebnis liefern, also muessen wir wirklich
                //zurueckfliessen
                else if( pNewUpper->IsInSct() && pNewUpper->IsColBodyFrm() &&
                    !(pNewUpper->Prt().*fnRectX->fnGetWidth)() &&
                    ( pNewUpper->GetUpper()->GetPrev() ||
                      pNewUpper->GetUpper()->GetNext() ) )
                    return sal_True;
                else
                    return sal_False; // Kein Platz, dann ist es sinnlos, zurueckzufliessen
            }
            else
            {
                // OD 2004-05-26 #i25904# - check for space left in new upper
                if ( nSpace )
                    return sal_True;
                else
                    return sal_False;
            }
        }
    }
    return  sal_False;
}

//------------------------------------------------------------------------
//              Calc-Methoden
//------------------------------------------------------------------------

/*************************************************************************
|*
|*  SwFrm::Prepare()
|*
|*  Beschreibung        Bereitet den Frm auf die 'Formatierung' (MakeAll())
|*      vor. Diese Methode dient dazu auf dem Stack Platz einzusparen,
|*      denn zur Positionsberechnung des Frm muss sichergestellt sein, dass
|*      die Position von Upper und Prev gueltig sind, mithin also ein
|*      rekursiver Aufruf (Schleife waere relativ teuer, da selten notwendig).
|*      Jeder Aufruf von MakeAll verbraucht aber ca. 500Byte Stack -
|*      das Ende ist leicht abzusehen. _Prepare benoetigt nur wenig Stack,
|*      deshalb solle der Rekursive Aufruf hier kein Problem sein.
|*      Ein weiterer Vorteil ist, das eines schoenen Tages das _Prepare und
|*      damit die Formatierung von Vorgaengern umgangen werden kann.
|*      So kann evtl. mal 'schnell' an's Dokumentende gesprungen werden.
|*
|*************************************************************************/
//Zwei kleine Freundschaften werden hier zu einem Geheimbund.
inline void PrepareLock( SwFlowFrm *pTab )
{
    pTab->LockJoin();
}
inline void PrepareUnlock( SwFlowFrm *pTab )
{
    pTab->UnlockJoin();

}

// hopefully, one day this function simply will return 'false'
bool lcl_IsCalcUpperAllowed( const SwFrm& rFrm )
{
    return !rFrm.GetUpper()->IsSctFrm() &&
           !rFrm.GetUpper()->IsFooterFrm() &&
           // --> OD 2004-11-02 #i23129#, #i36347# - no format of upper Writer fly frame
           !rFrm.GetUpper()->IsFlyFrm() &&
           // <--
           !( rFrm.GetUpper()->IsTabFrm() && rFrm.GetUpper()->GetUpper()->IsInTab() ) &&
           !( rFrm.IsTabFrm() && rFrm.GetUpper()->IsInTab() );
}

void SwFrm::PrepareMake()
{
    StackHack aHack;
    if ( GetUpper() )
    {
        if ( lcl_IsCalcUpperAllowed( *this ) )
            GetUpper()->Calc();
        OSL_ENSURE( GetUpper(), ":-( Layoutgeruest wackelig (Upper wech)." );
        if ( !GetUpper() )
            return;

        const sal_Bool bCnt = IsCntntFrm();
        const sal_Bool bTab = IsTabFrm();
        sal_Bool bNoSect = IsInSct();
        sal_Bool bOldTabLock = sal_False, bFoll = sal_False;
        SwFlowFrm* pThis = bCnt ? (SwCntntFrm*)this : NULL;

        if ( bTab )
        {
            pThis = (SwTabFrm*)this;
            bOldTabLock = ((SwTabFrm*)this)->IsJoinLocked();
            ::PrepareLock( (SwTabFrm*)this );
            bFoll = pThis->IsFollow();
        }
        else if( IsSctFrm() )
        {
            pThis = (SwSectionFrm*)this;
            bFoll = pThis->IsFollow();
            bNoSect = sal_False;
        }
        else if ( bCnt && sal_True == (bFoll = pThis->IsFollow()) &&
             GetPrev() )
        {
            //Wenn der Master gereade ein CalcFollow ruft braucht die Kette
            //nicht durchlaufen werden. Das spart Zeit und vermeidet Probleme.
            if ( ((SwTxtFrm*)((SwTxtFrm*)this)->FindMaster())->IsLocked() )
            {
                MakeAll();
                return;
            }
        }

        // --> OD 2005-03-04 #i44049# - no format of previous frame, if current
        // frame is a table frame and its previous frame wants to keep with it.
        const bool bFormatPrev = !bTab ||
                                 !GetPrev() ||
                                 !GetPrev()->GetAttrSet()->GetKeep().GetValue();
        if ( bFormatPrev )
        {
            SwFrm *pFrm = GetUpper()->Lower();
            while ( pFrm != this )
            {
                OSL_ENSURE( pFrm, ":-( Layoutgeruest wackelig (this not found)." );
                if ( !pFrm )
                    return; //Oioioioi ...

                if ( !pFrm->IsValid() )
                {
                    //Ein kleiner Eingriff der hoffentlich etwas zur Verbesserung
                    //der Stabilitaet beitraegt:
                    //Wenn ich Follow _und_ Nachbar eines Frms vor mir bin,
                    //so wuerde dieser mich beim Formatieren deleten; wie jeder
                    //leicht sehen kann waere dies eine etwas unuebersichtliche
                    //Situation die es zu vermeiden gilt.
                    if ( bFoll && pFrm->IsFlowFrm() &&
                         (SwFlowFrm::CastFlowFrm(pFrm))->IsAnFollow( pThis ) )
                        break;

    //MA: 24. Mar. 94, Calc wuerde doch nur wieder in ein _Prepare laufen und so
    //die ganze Kette nocheinmal abhuenern.
    //              pFrm->Calc();
                    pFrm->MakeAll();
                    if( IsSctFrm() && !((SwSectionFrm*)this)->GetSection() )
                        break;
                }
                //Die Kette kann bei CntntFrms waehrend des durchlaufens
                //aufgebrochen werden, deshalb muss der Nachfolger etwas
                //umstaendlich ermittelt werden. However, irgendwann _muss_
                //ich wieder bei mir selbst ankommen.
                pFrm = pFrm->FindNext();

                //Wenn wir in einem SectionFrm gestartet sind, koennen wir durch die
                //MakeAll-Aufrufe in einen Section-Follow gewandert sein.
                //FindNext liefert allerdings den SectionFrm, nicht seinen Inhalt.
                // => wir finden uns selbst nicht mehr!
                if( bNoSect && pFrm && pFrm->IsSctFrm() )
                {
                    SwFrm* pCnt = ((SwSectionFrm*)pFrm)->ContainsAny();
                    if( pCnt )
                        pFrm = pCnt;
                }
            }
            OSL_ENSURE( GetUpper(), "Layoutgeruest wackelig (Upper wech II)." );
            if ( !GetUpper() )
                return;

            if ( lcl_IsCalcUpperAllowed( *this ) )
                GetUpper()->Calc();

            OSL_ENSURE( GetUpper(), "Layoutgeruest wackelig (Upper wech III)." );
        }

        if ( bTab && !bOldTabLock )
            ::PrepareUnlock( (SwTabFrm*)this );
    }
    MakeAll();
}

void SwFrm::OptPrepareMake()
{
    // --> OD 2004-11-02 #i23129#, #i36347# - no format of upper Writer fly frame
    if ( GetUpper() && !GetUpper()->IsFooterFrm() &&
         !GetUpper()->IsFlyFrm() )
    // <--
    {
        GetUpper()->Calc();
        OSL_ENSURE( GetUpper(), ":-( Layoutgeruest wackelig (Upper wech)." );
        if ( !GetUpper() )
            return;
    }
    if ( GetPrev() && !GetPrev()->IsValid() )
        PrepareMake();
    else
    {
        StackHack aHack;
        MakeAll();
    }
}



void SwFrm::PrepareCrsr()
{
    StackHack aHack;
    if( GetUpper() && !GetUpper()->IsSctFrm() )
    {
        GetUpper()->PrepareCrsr();
        GetUpper()->Calc();

        OSL_ENSURE( GetUpper(), ":-( Layoutgeruest wackelig (Upper wech)." );
        if ( !GetUpper() )
            return;

        const sal_Bool bCnt = IsCntntFrm();
        const sal_Bool bTab = IsTabFrm();
        sal_Bool bNoSect = IsInSct();

        sal_Bool bOldTabLock = sal_False, bFoll;
        SwFlowFrm* pThis = bCnt ? (SwCntntFrm*)this : NULL;

        if ( bTab )
        {
            bOldTabLock = ((SwTabFrm*)this)->IsJoinLocked();
            ::PrepareLock( (SwTabFrm*)this );
            pThis = (SwTabFrm*)this;
        }
        else if( IsSctFrm() )
        {
            pThis = (SwSectionFrm*)this;
            bNoSect = sal_False;
        }
        bFoll = pThis && pThis->IsFollow();

        SwFrm *pFrm = GetUpper()->Lower();
        while ( pFrm != this )
        {
            OSL_ENSURE( pFrm, ":-( Layoutgeruest wackelig (this not found)." );
            if ( !pFrm )
                return; //Oioioioi ...

            if ( !pFrm->IsValid() )
            {
                //Ein kleiner Eingriff der hoffentlich etwas zur Verbesserung
                //der Stabilitaet beitraegt:
                //Wenn ich Follow _und_ Nachbar eines Frms vor mir bin,
                //so wuerde dieser mich beim Formatieren deleten; wie jeder
                //leicht sehen kann waere dies eine etwas unuebersichtliche
                //Situation die es zu vermeiden gilt.
                if ( bFoll && pFrm->IsFlowFrm() &&
                     (SwFlowFrm::CastFlowFrm(pFrm))->IsAnFollow( pThis ) )
                    break;

                pFrm->MakeAll();
            }
            //Die Kette kann bei CntntFrms waehrend des durchlaufens
            //aufgebrochen werden, deshalb muss der Nachfolger etwas
            //umstaendlich ermittelt werden. However, irgendwann _muss_
            //ich wieder bei mir selbst ankommen.
            pFrm = pFrm->FindNext();
            if( bNoSect && pFrm && pFrm->IsSctFrm() )
            {
                SwFrm* pCnt = ((SwSectionFrm*)pFrm)->ContainsAny();
                if( pCnt )
                    pFrm = pCnt;
            }
        }
        OSL_ENSURE( GetUpper(), "Layoutgeruest wackelig (Upper wech II)." );
        if ( !GetUpper() )
            return;

        GetUpper()->Calc();

        OSL_ENSURE( GetUpper(), "Layoutgeruest wackelig (Upper wech III)." );

        if ( bTab && !bOldTabLock )
            ::PrepareUnlock( (SwTabFrm*)this );
    }
    Calc();
}

/*************************************************************************
|*
|*  SwFrm::MakePos()
|*
|*************************************************************************/

// Hier wird GetPrev() zurueckgegeben, allerdings werden
// dabei leere SectionFrms ueberlesen
SwFrm* lcl_Prev( SwFrm* pFrm, sal_Bool bSectPrv = sal_True )
{
    SwFrm* pRet = pFrm->GetPrev();
    if( !pRet && pFrm->GetUpper() && pFrm->GetUpper()->IsSctFrm() &&
        bSectPrv && !pFrm->IsColumnFrm() )
        pRet = pFrm->GetUpper()->GetPrev();
    while( pRet && pRet->IsSctFrm() &&
           !((SwSectionFrm*)pRet)->GetSection() )
        pRet = pRet->GetPrev();
    return pRet;
}

SwFrm* lcl_NotHiddenPrev( SwFrm* pFrm )
{
    SwFrm *pRet = pFrm;
    do
    {
        pRet = lcl_Prev( pRet );
    } while ( pRet && pRet->IsTxtFrm() && ((SwTxtFrm*)pRet)->IsHiddenNow() );
    return pRet;
}

void SwFrm::MakePos()
{
    if ( !bValidPos )
    {
        bValidPos = sal_True;
        sal_Bool bUseUpper = sal_False;
        SwFrm* pPrv = lcl_Prev( this );
        if ( pPrv &&
             ( !pPrv->IsCntntFrm() ||
               ( ((SwCntntFrm*)pPrv)->GetFollow() != this ) )
           )
        {
            if ( !StackHack::IsLocked() &&
                 ( !IsInSct() || IsSctFrm() ) &&
                 !pPrv->IsSctFrm() &&
                 !pPrv->GetAttrSet()->GetKeep().GetValue()
               )
            {
                pPrv->Calc();   //hierbei kann der Prev verschwinden!
            }
            else if ( pPrv->Frm().Top() == 0 )
            {
                bUseUpper = sal_True;
            }
        }

        pPrv = lcl_Prev( this, sal_False );
        sal_uInt16 nMyType = GetType();
        SWRECTFN( ( IsCellFrm() && GetUpper() ? GetUpper() : this  ) )
        if ( !bUseUpper && pPrv )
        {
            aFrm.Pos( pPrv->Frm().Pos() );
            if( FRM_NEIGHBOUR & nMyType )
            {
                sal_Bool bR2L = IsRightToLeft();
                if( bR2L )
                    (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() -
                                               (aFrm.*fnRect->fnGetWidth)() );
                else
                    (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() +
                                          (pPrv->Frm().*fnRect->fnGetWidth)() );

                // cells may now leave their uppers
                if( bVert && FRM_CELL & nMyType && !bReverse )
                    aFrm.Pos().X() -= aFrm.Width() -pPrv->Frm().Width();
            }
            else if( bVert && FRM_NOTE_VERT & nMyType )
            {
                if( bReverse )
                    aFrm.Pos().X() += pPrv->Frm().Width();
                else
                    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
                {
                    if ( bVertL2R )
                           aFrm.Pos().X() += pPrv->Frm().Width();
                    else
                           aFrm.Pos().X() -= aFrm.Width();
                  }
            }
            else
                aFrm.Pos().Y() += pPrv->Frm().Height();
        }
        else if ( GetUpper() )
        {
            // OD 15.10.2002 #103517# - add safeguard for <SwFooterFrm::Calc()>
            // If parent frame is a footer frame and its <ColLocked()>, then
            // do *not* calculate it.
            // NOTE: Footer frame is <ColLocked()> during its
            //     <FormatSize(..)>, which is called from <Format(..)>, which
            //     is called from <MakeAll()>, which is called from <Calc()>.
            // --> OD 2005-11-17 #i56850#
            // - no format of upper Writer fly frame, which is anchored
            //   at-paragraph or at-character.
            if ( !GetUpper()->IsTabFrm() &&
                 !( IsTabFrm() && GetUpper()->IsInTab() ) &&
                 !GetUpper()->IsSctFrm() &&
                 !dynamic_cast<SwFlyAtCntFrm*>(GetUpper()) &&
                 !( GetUpper()->IsFooterFrm() &&
                    GetUpper()->IsColLocked() )
               )
            {
                GetUpper()->Calc();
            }
            // <--
            pPrv = lcl_Prev( this, sal_False );
            if ( !bUseUpper && pPrv )
            {
                aFrm.Pos( pPrv->Frm().Pos() );
                if( FRM_NEIGHBOUR & nMyType )
                {
                    sal_Bool bR2L = IsRightToLeft();
                    if( bR2L )
                        (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() -
                                                 (aFrm.*fnRect->fnGetWidth)() );
                    else
                        (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() +
                                          (pPrv->Frm().*fnRect->fnGetWidth)() );

                    // cells may now leave their uppers
                    if( bVert && FRM_CELL & nMyType && !bReverse )
                        aFrm.Pos().X() -= aFrm.Width() -pPrv->Frm().Width();
                }
                else if( bVert && FRM_NOTE_VERT & nMyType )
                {
                    if( bReverse )
                        aFrm.Pos().X() += pPrv->Frm().Width();
                    else
                        aFrm.Pos().X() -= aFrm.Width();
                }
                else
                    aFrm.Pos().Y() += pPrv->Frm().Height();
            }
            else
            {
                aFrm.Pos( GetUpper()->Frm().Pos() );
                aFrm.Pos() += GetUpper()->Prt().Pos();
                if( FRM_NEIGHBOUR & nMyType && IsRightToLeft() )
                {
                    if( bVert )
                        aFrm.Pos().Y() += GetUpper()->Prt().Height()
                                          - aFrm.Height();
                    else
                        aFrm.Pos().X() += GetUpper()->Prt().Width()
                                          - aFrm.Width();
                }
                //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
                else if( bVert && !bVertL2R && FRM_NOTE_VERT & nMyType && !bReverse )
                    aFrm.Pos().X() -= aFrm.Width() - GetUpper()->Prt().Width();
            }
        }
        else
            aFrm.Pos().X() = aFrm.Pos().Y() = 0;
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        if( IsBodyFrm() && bVert && !bVertL2R && !bReverse && GetUpper() )
            aFrm.Pos().X() += GetUpper()->Prt().Width() - aFrm.Width();
        bValidPos = sal_True;
    }
}

/*************************************************************************
|*
|*  SwPageFrm::MakeAll()
|*
|*************************************************************************/
// --> OD 2004-07-01 #i28701# - new type <SwSortedObjs>
void lcl_CheckObjects( SwSortedObjs* pSortedObjs, SwFrm* pFrm, long& rBot )
{
    //Und dann kann es natuerlich noch Absatzgebundene
    //Rahmen geben, die unterhalb ihres Absatzes stehen.
    long nMax = 0;
    for ( sal_uInt16 i = 0; i < pSortedObjs->Count(); ++i )
    {
        // --> OD 2004-07-01 #i28701# - consider changed type of <SwSortedObjs>
        // entries.
        SwAnchoredObject* pObj = (*pSortedObjs)[i];
        long nTmp = 0;
        if ( pObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pObj);
            if( pFly->Frm().Top() != WEIT_WECH &&
                ( pFrm->IsPageFrm() ? pFly->IsFlyLayFrm() :
                  ( pFly->IsFlyAtCntFrm() &&
                    ( pFrm->IsBodyFrm() ? pFly->GetAnchorFrm()->IsInDocBody() :
                                          pFly->GetAnchorFrm()->IsInFtn() ) ) ) )
            {
                nTmp = pFly->Frm().Bottom();
            }
        }
        else
            nTmp = pObj->GetObjRect().Bottom();
        nMax = Max( nTmp, nMax );
        // <--
    }
    ++nMax; //Unterkante vs. Hoehe!
    rBot = Max( rBot, nMax );
}

void SwPageFrm::MakeAll()
{
    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

    const SwRect aOldRect( Frm() );     //Anpassung der Root-Groesse
    const SwLayNotify aNotify( this );  //uebernimmt im DTor die Benachrichtigung
    SwBorderAttrAccess *pAccess = 0;
    const SwBorderAttrs*pAttrs = 0;

    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        if ( !bValidPos )
        {
            // PAGES01
            bValidPos = sal_True; // positioning of the pages is taken care of by the root frame
        }

        if ( !bValidSize || !bValidPrtArea )
        {
            if ( IsEmptyPage() )
            {
                Frm().Width( 0 );  Prt().Width( 0 );
                Frm().Height( 0 ); Prt().Height( 0 );
                Prt().Left( 0 );   Prt().Top( 0 );
                bValidSize = bValidPrtArea = sal_True;
            }
            else
            {
                if ( !pAccess )
                {
                    pAccess = new SwBorderAttrAccess( SwFrm::GetCache(), this );
                    pAttrs = pAccess->Get();
                }
                //Bei der BrowseView gelten feste Einstellungen.
                ViewShell *pSh = GetShell();
                if ( pSh && GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
                {
                    const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
                    const long nTop    = pAttrs->CalcTopLine()   + aBorder.Height();
                    const long nBottom = pAttrs->CalcBottomLine()+ aBorder.Height();

                    long nWidth = GetUpper() ? ((SwRootFrm*)GetUpper())->GetBrowseWidth() : 0;
                    if ( nWidth < pSh->GetBrowseWidth() )
                        nWidth = pSh->GetBrowseWidth();
                    nWidth += + 2 * aBorder.Width();
/*
                    long nWidth = GetUpper() ? ((SwRootFrm*)GetUpper())->GetBrowseWidth() + 2 * aBorder.Width() : 0;
                    if ( nWidth < pSh->VisArea().Width() )
                        nWidth = pSh->VisArea().Width(); */

                    nWidth = Max( nWidth, 2L * aBorder.Width() + 4L*MM50 );
                    Frm().Width( nWidth );

                    SwLayoutFrm *pBody = FindBodyCont();
                    if ( pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm() )
                    {
                        //Fuer Spalten gilt eine feste Hoehe
                        Frm().Height( pAttrs->GetSize().Height() );
                    }
                    else
                    {
                        //Fuer Seiten ohne Spalten bestimmt der Inhalt die
                        //Groesse.
                        long nBot = Frm().Top() + nTop;
                        SwFrm *pFrm = Lower();
                        while ( pFrm )
                        {
                            long nTmp = 0;
                            SwFrm *pCnt = ((SwLayoutFrm*)pFrm)->ContainsAny();
                            while ( pCnt && (pCnt->GetUpper() == pFrm ||
                                             ((SwLayoutFrm*)pFrm)->IsAnLower( pCnt )))
                            {
                                nTmp += pCnt->Frm().Height();
                                if( pCnt->IsTxtFrm() &&
                                    ((SwTxtFrm*)pCnt)->IsUndersized() )
                                    nTmp += ((SwTxtFrm*)pCnt)->GetParHeight()
                                            - pCnt->Prt().Height();
                                else if( pCnt->IsSctFrm() &&
                                         ((SwSectionFrm*)pCnt)->IsUndersized() )
                                    nTmp += ((SwSectionFrm*)pCnt)->Undersize();
                                pCnt = pCnt->FindNext();
                            }
                            // OD 29.10.2002 #97265# - consider invalid body frame properties
                            if ( pFrm->IsBodyFrm() &&
                                 ( !pFrm->GetValidSizeFlag() ||
                                   !pFrm->GetValidPrtAreaFlag() ) &&
                                 ( pFrm->Frm().Height() < pFrm->Prt().Height() )
                               )
                            {
                                nTmp = Min( nTmp, pFrm->Frm().Height() );
                            }
                            else
                            {
                                // OD 30.10.2002 #97265# - assert invalid lower property
                                OSL_ENSURE( !(pFrm->Frm().Height() < pFrm->Prt().Height()),
                                        "SwPageFrm::MakeAll(): Lower with frame height < printing height" );
                                nTmp += pFrm->Frm().Height() - pFrm->Prt().Height();
                            }
                            if ( !pFrm->IsBodyFrm() )
                                nTmp = Min( nTmp, pFrm->Frm().Height() );
                            nBot += nTmp;
                            // Hier werden die absatzgebundenen Objekte ueberprueft,
                            // ob sie ueber den Body/FtnCont hinausragen.
                            if( pSortedObjs && !pFrm->IsHeaderFrm() &&
                                !pFrm->IsFooterFrm() )
                                lcl_CheckObjects( pSortedObjs, pFrm, nBot );
                            pFrm = pFrm->GetNext();
                        }
                        nBot += nBottom;
                        //Und die Seitengebundenen
                        if ( pSortedObjs )
                            lcl_CheckObjects( pSortedObjs, this, nBot );
                        nBot -= Frm().Top();
                        // --> OD 2004-11-10 #i35143# - If second page frame
                        // exists, the first page doesn't have to fulfill the
                        // visible area.
                        if ( !GetPrev() && !GetNext() )
                        // <--
                        {
                            nBot = Max( nBot, pSh->VisArea().Height() );
                        }
                        // --> OD 2004-11-10 #i35143# - Assure, that the page
                        // doesn't exceed the defined browse height.
                        Frm().Height( Min( nBot, BROWSE_HEIGHT ) );
                        // <--
                    }
                    Prt().Left ( pAttrs->CalcLeftLine() + aBorder.Width() );
                    Prt().Top  ( nTop );
                    Prt().Width( Frm().Width() - ( Prt().Left()
                        + pAttrs->CalcRightLine() + aBorder.Width() ) );
                    Prt().Height( Frm().Height() - (nTop + nBottom) );
                    bValidSize = bValidPrtArea = sal_True;
                }
                else
                {   //FixSize einstellen, bei Seiten nicht vom Upper sondern vom
                    //Attribut vorgegeben.
                    Frm().SSize( pAttrs->GetSize() );
                    Format( pAttrs );
                }
            }
        }
    } //while ( !bValidPos || !bValidSize || !bValidPrtArea )
    delete pAccess;

    // PAGES01
    if ( Frm() != aOldRect && GetUpper() )
        static_cast<SwRootFrm*>(GetUpper())->CheckViewLayout( 0, 0 );

#if OSL_DEBUG_LEVEL > 1
    //Der Upper (Root) muss mindestens so breit
    //sein, dass er die breiteste Seite aufnehmen kann.
    if ( GetUpper() )
    {
        OSL_ENSURE( GetUpper()->Prt().Width() >= aFrm.Width(), "Rootsize" );
    }
#endif
}

/*************************************************************************
|*
|*  SwLayoutFrm::MakeAll()
|*
|*************************************************************************/


void SwLayoutFrm::MakeAll()
{
    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

        //uebernimmt im DTor die Benachrichtigung
    const SwLayNotify aNotify( this );
    sal_Bool bVert = IsVertical();
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    SwRectFn fnRect = ( IsNeighbourFrm() == bVert )? fnRectHori : ( IsVertLR() ? fnRectVertL2R : fnRectVert );

    SwBorderAttrAccess *pAccess = 0;
    const SwBorderAttrs*pAttrs = 0;

    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        if ( !bValidPos )
            MakePos();

        if ( GetUpper() )
        {
            // NEW TABLES
            if ( IsLeaveUpperAllowed() )
            {
                if ( !bValidSize )
                    bValidPrtArea = sal_False;
            }
            else
            {
                if ( !bValidSize )
                {
                    //FixSize einstellen, die VarSize wird von Format() nach
                    //Berechnung der PrtArea eingestellt.
                    bValidPrtArea = sal_False;

                    SwTwips nPrtWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
                    if( bVert && ( IsBodyFrm() || IsFtnContFrm() ) )
                    {
                        SwFrm* pNxt = GetPrev();
                        while( pNxt && !pNxt->IsHeaderFrm() )
                            pNxt = pNxt->GetPrev();
                        if( pNxt )
                            nPrtWidth -= pNxt->Frm().Height();
                        pNxt = GetNext();
                        while( pNxt && !pNxt->IsFooterFrm() )
                            pNxt = pNxt->GetNext();
                        if( pNxt )
                            nPrtWidth -= pNxt->Frm().Height();
                    }

                    const long nDiff = nPrtWidth - (Frm().*fnRect->fnGetWidth)();

                    if( IsNeighbourFrm() && IsRightToLeft() )
                        (Frm().*fnRect->fnSubLeft)( nDiff );
                    else
                        (Frm().*fnRect->fnAddRight)( nDiff );
                }
                else
                {
                    // Don't leave your upper
                    const SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                    if( (Frm().*fnRect->fnOverStep)( nDeadLine ) )
                        bValidSize = sal_False;
                }
            }
        }
        if ( !bValidSize || !bValidPrtArea )
        {
            if ( !pAccess )
            {
                pAccess = new SwBorderAttrAccess( SwFrm::GetCache(), this );
                pAttrs  = pAccess->Get();
            }
            Format( pAttrs );
        }
    } //while ( !bValidPos || !bValidSize || !bValidPrtArea )
    if ( pAccess )
        delete pAccess;
}

/*************************************************************************
|*
|*  SwCntntFrm::MakePrtArea()
|*
|*************************************************************************/
bool SwTxtNode::IsCollapse() const
{
    if ( GetDoc()->get( IDocumentSettingAccess::COLLAPSE_EMPTY_CELL_PARA ) &&  GetTxt().Len()==0 ) {
        sal_uLong nIdx=GetIndex();
        const SwEndNode *pNdBefore=GetNodes()[nIdx-1]->GetEndNode();
        const SwEndNode *pNdAfter=GetNodes()[nIdx+1]->GetEndNode();

        // The paragraph is collapsed only if the NdAfter is the end of a cell
        bool bInTable = this->FindTableNode( ) != NULL;

        SwSortedObjs* pObjs = this->GetFrm()->GetDrawObjs( );
        sal_uInt32 nObjs = ( pObjs != NULL ) ? pObjs->Count( ) : 0;

        if ( pNdBefore!=NULL && pNdAfter!=NULL && nObjs == 0 && bInTable ) {
            return true;
        } else {
            return false;
        }
    } else
        return false;
}

bool SwFrm::IsCollapse() const
{
    if (IsTxtFrm()) {
        const SwTxtFrm *pTxtFrm=(SwTxtFrm*)this;
        const SwTxtNode *pTxtNode=pTxtFrm->GetTxtNode();
        if (pTxtNode && pTxtNode->IsCollapse()) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

sal_Bool SwCntntFrm::MakePrtArea( const SwBorderAttrs &rAttrs )
{
    sal_Bool bSizeChgd = sal_False;

    if ( !bValidPrtArea )
    {
        bValidPrtArea = sal_True;

        SWRECTFN( this )
        const sal_Bool bTxtFrm = IsTxtFrm();
        SwTwips nUpper = 0;
        if ( bTxtFrm && ((SwTxtFrm*)this)->IsHiddenNow() )
        {
            if ( ((SwTxtFrm*)this)->HasFollow() )
                ((SwTxtFrm*)this)->JoinFrm();

            if( (Prt().*fnRect->fnGetHeight)() )
                ((SwTxtFrm*)this)->HideHidden();
            Prt().Pos().X() = Prt().Pos().Y() = 0;
            (Prt().*fnRect->fnSetWidth)( (Frm().*fnRect->fnGetWidth)() );
            (Prt().*fnRect->fnSetHeight)( 0 );
            nUpper = -( (Frm().*fnRect->fnGetHeight)() );
        }
        else
        {
            //Vereinfachung: CntntFrms sind immer in der Hoehe Variabel!

            //An der FixSize gibt der umgebende Frame die Groesse vor, die
            //Raender werden einfach abgezogen.
            const long nLeft = rAttrs.CalcLeft( this );
            const long nRight = ((SwBorderAttrs&)rAttrs).CalcRight( this );
            (this->*fnRect->fnSetXMargins)( nLeft, nRight );

            ViewShell *pSh = GetShell();
            SwTwips nWidthArea;
            if( pSh && 0!=(nWidthArea=(pSh->VisArea().*fnRect->fnGetWidth)()) &&
                GetUpper()->IsPageBodyFrm() &&  // nicht dagegen bei BodyFrms in Columns
                pSh->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
            {
                //Nicht ueber die Kante des sichbaren Bereiches hinausragen.
                //Die Seite kann breiter sein, weil es Objekte mit "ueberbreite"
                //geben kann (RootFrm::ImplCalcBrowseWidth())
                long nMinWidth = 0;

                for (sal_uInt16 i = 0; GetDrawObjs() && i < GetDrawObjs()->Count();++i)
                {
                    // --> OD 2004-07-01 #i28701# - consider changed type of
                    // <SwSortedObjs> entries
                    SwAnchoredObject* pObj = (*GetDrawObjs())[i];
                    const SwFrmFmt& rFmt = pObj->GetFrmFmt();
                    const sal_Bool bFly = pObj->ISA(SwFlyFrm);
                    if ((bFly && (WEIT_WECH == pObj->GetObjRect().Width()))
                        || rFmt.GetFrmSize().GetWidthPercent())
                    {
                        continue;
                    }

                    if ( FLY_AS_CHAR == rFmt.GetAnchor().GetAnchorId() )
                    {
                        nMinWidth = Max( nMinWidth,
                                         bFly ? rFmt.GetFrmSize().GetWidth()
                                              : pObj->GetObjRect().Width() );
                    }
                    // <--
                }

                const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
                long nWidth = nWidthArea - 2 * ( IsVertical() ? aBorder.Height() : aBorder.Width() );
                nWidth -= (Prt().*fnRect->fnGetLeft)();
                nWidth -= rAttrs.CalcRightLine();
                nWidth = Max( nMinWidth, nWidth );
                (Prt().*fnRect->fnSetWidth)( Min( nWidth,
                                            (Prt().*fnRect->fnGetWidth)() ) );
            }

            if ( (Prt().*fnRect->fnGetWidth)() <= MINLAY )
            {
                //Die PrtArea sollte schon wenigstens MINLAY breit sein, passend
                //zu den Minimalwerten des UI
                (Prt().*fnRect->fnSetWidth)( Min( long(MINLAY),
                                             (Frm().*fnRect->fnGetWidth)() ) );
                SwTwips nTmp = (Frm().*fnRect->fnGetWidth)() -
                               (Prt().*fnRect->fnGetWidth)();
                if( (Prt().*fnRect->fnGetLeft)() > nTmp )
                    (Prt().*fnRect->fnSetLeft)( nTmp );
            }

            //Fuer die VarSize gelten folgende Regeln:
            //1. Der erste einer Kette hat keinen Rand nach oben
            //2. Nach unten gibt es nie einen Rand
            //3. Der Rand nach oben ist das Maximum aus dem Abstand des
            //   Prev nach unten und dem eigenen Abstand nach oben.
            //Die drei Regeln werden auf die Berechnung der Freiraeume, die von
            //UL- bzw. LRSpace vorgegeben werden, angewand. Es gibt in alle
            //Richtungen jedoch ggf. trotzdem einen Abstand; dieser wird durch
            //Umrandung und/oder Schatten vorgegeben.
            //4. Der Abstand fuer TextFrms entspricht mindestens dem Durchschuss

            nUpper = CalcUpperSpace( &rAttrs, NULL );

            SwTwips nLower = CalcLowerSpace( &rAttrs );
            if (IsCollapse()) {
                nUpper=0;
                nLower=0;
            }
//            // in balanced columned section frames we do not want the
//            // common border
//            sal_Bool bCommonBorder = sal_True;
//            if ( IsInSct() && GetUpper()->IsColBodyFrm() )
//            {
//                const SwSectionFrm* pSct = FindSctFrm();
//                bCommonBorder = pSct->GetFmt()->GetBalancedColumns().GetValue();
//            }
//            SwTwips nLower = bCommonBorder ?
//                             rAttrs.GetBottomLine( this ) :
//                             rAttrs.CalcBottomLine();

            (Prt().*fnRect->fnSetPosY)( (!bVert || bReverse) ? nUpper : nLower);
            nUpper += nLower;
            nUpper -= (Frm().*fnRect->fnGetHeight)() -
                      (Prt().*fnRect->fnGetHeight)();
        }
        //Wenn Unterschiede zwischen Alter und neuer Groesse,
        //Grow() oder Shrink() rufen
        if ( nUpper )
        {
            if ( nUpper > 0 )
                GrowFrm( nUpper );
            else
                ShrinkFrm( -nUpper );
            bSizeChgd = sal_True;
        }
    }
    return bSizeChgd;
}

/*************************************************************************
|*
|*  SwCntntFrm::MakeAll()
|*
|*************************************************************************/

#define STOP_FLY_FORMAT 10
// --> OD 2006-09-25 #b6448963# - loop prevention
const int cnStopFormat = 15;
// <--

inline void ValidateSz( SwFrm *pFrm )
{
    if ( pFrm )
    {
        pFrm->bValidSize = sal_True;
        pFrm->bValidPrtArea = sal_True;
    }
}

void SwCntntFrm::MakeAll()
{
    OSL_ENSURE( GetUpper(), "keinen Upper?" );
    OSL_ENSURE( IsTxtFrm(), "MakeAll(), NoTxt" );

    if ( !IsFollow() && StackHack::IsLocked() )
        return;

    if ( IsJoinLocked() )
        return;

    OSL_ENSURE( !((SwTxtFrm*)this)->IsSwapped(), "Calculation of a swapped frame" );

    StackHack aHack;

    if ( ((SwTxtFrm*)this)->IsLocked() )
    {
        OSL_FAIL( "Format fuer gelockten TxtFrm." );
        return;
    }

    LockJoin();
    long nFormatCount = 0;
    // --> OD 2006-09-25 #b6448963# - loop prevention
    int nConsequetiveFormatsWithoutChange = 0;
    // <--
    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

#if OSL_DEBUG_LEVEL > 1
    const SwDoc *pDoc = GetAttrSet()->GetDoc();
    if( pDoc )
    {
        static sal_Bool bWarn = sal_False;
        if( pDoc->InXMLExport() )
        {
            OSL_ENSURE( bWarn, "Formatting during XML-export!" );
            bWarn = sal_True;
        }
        else
            bWarn = sal_False;
    }
#endif

    //uebernimmt im DTor die Benachrichtigung
    SwCntntNotify *pNotify = new SwCntntNotify( this );

    sal_Bool    bMakePage   = sal_True;     //solange sal_True kann eine neue Seite
                                    //angelegt werden (genau einmal)
    sal_Bool    bMovedBwd   = sal_False;    //Wird sal_True wenn der Frame zurueckfliesst
    sal_Bool    bMovedFwd   = sal_False;    //solange sal_False kann der Frm zurueck-
                                    //fliessen (solange, bis er einmal
                                    //vorwaerts ge'moved wurde).
    sal_Bool    bFormatted  = sal_False;    //Fuer die Witwen und Waisen Regelung
                                    //wird der letzte CntntFrm einer Kette
                                    //u.U. zum Formatieren angeregt, dies
                                    //braucht nur einmal zu passieren.
                                    //Immer wenn der Frm gemoved wird muss
                                    //das Flag zurueckgesetzt werden.
    sal_Bool    bMustFit    = sal_False;    //Wenn einmal die Notbremse gezogen wurde,
                                    //werden keine anderen Prepares mehr
                                    //abgesetzt.
    sal_Bool    bFitPromise = sal_False;    //Wenn ein Absatz nicht passte, mit WouldFit
                                    //aber verspricht, dass er sich passend
                                    //einstellt wird dieses Flag gesetzt.
                                    //Wenn er dann sein Versprechen nicht haelt,
                                    //kann kontrolliert verfahren werden.
    sal_Bool bMoveable;
    const sal_Bool bFly = IsInFly();
    const sal_Bool bTab = IsInTab();
    const sal_Bool bFtn = IsInFtn();
    const sal_Bool bSct = IsInSct();
    Point aOldFrmPos;               //Damit bei Turnarounds jew. mit der
    Point aOldPrtPos;               //letzten Pos verglichen und geprueft
                                    //werden kann, ob ein Prepare sinnvoll ist.

    SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    // OD 2004-02-26 #i25029#
    if ( !IsFollow() && rAttrs.JoinedWithPrev( *(this) ) )
    {
        pNotify->SetBordersJoinedWithPrev();
    }

    const sal_Bool bKeep = IsKeep( rAttrs.GetAttrSet() );

    SwSaveFtnHeight *pSaveFtn = 0;
    if ( bFtn )
    {
        SwFtnFrm *pFtn = FindFtnFrm();
        SwSectionFrm* pSct = pFtn->FindSctFrm();
        if ( !((SwTxtFrm*)pFtn->GetRef())->IsLocked() )
        {
            SwFtnBossFrm* pBoss = pFtn->GetRef()->FindFtnBossFrm(
                                    pFtn->GetAttr()->GetFtn().IsEndNote() );
            if( !pSct || pSct->IsColLocked() || !pSct->Growable() )
                pSaveFtn = new SwSaveFtnHeight( pBoss,
                    ((SwTxtFrm*)pFtn->GetRef())->GetFtnLine( pFtn->GetAttr() ) );
        }
    }

    // --> OD 2008-08-12 #b6732519#
    if ( GetUpper()->IsSctFrm() &&
         HasFollow() &&
         GetFollow()->GetFrm() == GetNext() )
    {
        dynamic_cast<SwTxtFrm*>(this)->JoinFrm();
    }
    // <--

    // --> OD 2004-06-23 #i28701# - move master forward, if it has to move,
    // because of its object positioning.
    if ( !static_cast<SwTxtFrm*>(this)->IsFollow() )
    {
        sal_uInt32 nToPageNum = 0L;
        const bool bMoveFwdByObjPos = SwLayouter::FrmMovedFwdByObjPos(
                                                    *(GetAttrSet()->GetDoc()),
                                                    *(static_cast<SwTxtFrm*>(this)),
                                                    nToPageNum );
        // --> OD 2006-01-27 #i58182#
        // Also move a paragraph forward, which is the first one inside a table cell.
        if ( bMoveFwdByObjPos &&
             FindPageFrm()->GetPhyPageNum() < nToPageNum &&
             ( lcl_Prev( this ) ||
               GetUpper()->IsCellFrm() ||
               ( GetUpper()->IsSctFrm() &&
                 GetUpper()->GetUpper()->IsCellFrm() ) ) &&
             IsMoveable() )
        {
            bMovedFwd = sal_True;
            MoveFwd( bMakePage, sal_False );
        }
        // <--
    }
    // <--

    //Wenn ein Follow neben seinem Master steht und nicht passt, kann er
    //gleich verschoben werden.
    if ( lcl_Prev( this ) && ((SwTxtFrm*)this)->IsFollow() && IsMoveable() )
    {
        bMovedFwd = sal_True;
        // OD 2004-03-02 #106629# - If follow frame is in table, it's master
        // will be the last in the current table cell. Thus, invalidate the
        // printing area of the master,
        if ( IsInTab() )
        {
            lcl_Prev( this )->InvalidatePrt();
        }
        MoveFwd( bMakePage, sal_False );
    }

    // OD 08.11.2002 #104840# - check footnote content for forward move.
    // If a content of a footnote is on a prior page/column as its invalid
    // reference, it can be moved forward.
    if ( bFtn && !bValidPos )
    {
        SwFtnFrm* pFtn = FindFtnFrm();
        SwCntntFrm* pRefCnt = pFtn ? pFtn->GetRef() : 0;
        if ( pRefCnt && !pRefCnt->IsValid() )
        {
            SwFtnBossFrm* pFtnBossOfFtn = pFtn->FindFtnBossFrm();
            SwFtnBossFrm* pFtnBossOfRef = pRefCnt->FindFtnBossFrm();
            //<loop of movefwd until condition held or no move>
            if ( pFtnBossOfFtn && pFtnBossOfRef &&
                 pFtnBossOfFtn != pFtnBossOfRef &&
                 pFtnBossOfFtn->IsBefore( pFtnBossOfRef ) )
            {
                bMovedFwd = sal_True;
                MoveFwd( bMakePage, sal_False );
            }
        }
    }

    SWRECTFN( this )

    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        // --> OD 2006-09-25 #b6448963# - loop prevention
        SwRect aOldFrm_StopFormat( Frm() );
        SwRect aOldPrt_StopFormat( Prt() );
        // <--
        if ( sal_True == (bMoveable = IsMoveable()) )
        {
            SwFrm *pPre = GetIndPrev();
            if ( CheckMoveFwd( bMakePage, bKeep, bMovedBwd ) )
            {
                SWREFRESHFN( this )
                bMovedFwd = sal_True;
                if ( bMovedBwd )
                {
                    //Beim zurueckfliessen wurde der Upper angeregt sich
                    //vollstaendig zu Painten, dass koennen wir uns jetzt
                    //nach dem hin und her fliessen sparen.
                    GetUpper()->ResetCompletePaint();
                    //Der Vorgaenger wurde Invalidiert, das ist jetzt auch obsolete.
                    OSL_ENSURE( pPre, "missing old Prev" );
                    if( !pPre->IsSctFrm() )
                        ::ValidateSz( pPre );
                }
                bMoveable = IsMoveable();
            }
        }

        aOldFrmPos = (Frm().*fnRect->fnGetPos)();
        aOldPrtPos = (Prt().*fnRect->fnGetPos)();

        if ( !bValidPos )
            MakePos();

        //FixSize einstellen, die VarSize wird von Format() justiert.
        if ( !bValidSize )
        {
            // --> OD 2006-01-03 #125452#
            // invalidate printing area flag, if the following conditions are hold:
            // - current frame width is 0.
            // - current printing area width is 0.
            // - frame width is adjusted to a value greater than 0.
            // - printing area flag is sal_True.
            // Thus, it's assured that the printing area is adjusted, if the
            // frame area width changes its width from 0 to something greater
            // than 0.
            // Note: A text frame can be in such a situation, if the format is
            //       triggered by method call <SwCrsrShell::SetCrsr()> after
            //       loading the document.
            const SwTwips nNewFrmWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            if ( bValidPrtArea && nNewFrmWidth > 0 &&
                 (Frm().*fnRect->fnGetWidth)() == 0 &&
                 (Prt().*fnRect->fnGetWidth)() == 0 )
            {
                bValidPrtArea = sal_False;
            }

            (Frm().*fnRect->fnSetWidth)( nNewFrmWidth );
            // <--
        }
        if ( !bValidPrtArea )
        {
            const long nOldW = (Prt().*fnRect->fnGetWidth)();
            // --> OD 2004-09-28 #i34730# - keep current frame height
            const SwTwips nOldH = (Frm().*fnRect->fnGetHeight)();
            // <--
            MakePrtArea( rAttrs );
            if ( nOldW != (Prt().*fnRect->fnGetWidth)() )
                Prepare( PREP_FIXSIZE_CHG );
            // --> OD 2004-09-28 #i34730# - check, if frame height has changed.
            // If yes, send a PREP_ADJUST_FRM and invalidate the size flag to
            // force a format. The format will check in its method
            // <SwTxtFrm::CalcPreps()>, if the already formatted lines still
            // fit and if not, performs necessary actions.
            // --> OD 2005-01-10 #i40150# - no check, if frame is undersized.
            if ( bValidSize && !IsUndersized() &&
                 nOldH != (Frm().*fnRect->fnGetHeight)() )
            {
                // --> OD 2004-11-25 #115759# - no PREP_ADJUST_FRM and size
                // invalidation, if height decreases only by the additional
                // lower space as last content of a table cell and an existing
                // follow containing one line exists.
                const SwTwips nHDiff = nOldH - (Frm().*fnRect->fnGetHeight)();
                const bool bNoPrepAdjustFrm =
                    nHDiff > 0 && IsInTab() && GetFollow() &&
                    ( 1 == static_cast<SwTxtFrm*>(GetFollow())->GetLineCount( STRING_LEN ) || (static_cast<SwTxtFrm*>(GetFollow())->Frm().*fnRect->fnGetWidth)() < 0 ) &&
                    GetFollow()->CalcAddLowerSpaceAsLastInTableCell() == nHDiff;
                if ( !bNoPrepAdjustFrm )
                {
                    Prepare( PREP_ADJUST_FRM );
                    bValidSize = sal_False;
                }
                // <--
            }
            // <--
        }

        //Damit die Witwen- und Waisen-Regelung eine Change bekommt muss der
        //CntntFrm benachrichtigt werden.
        //Kriterium:
        //- Er muss Moveable sein (sonst mach das Spalten keinen Sinn.)
        //- Er muss mit der Unterkante der PrtArea des Upper ueberlappen.
        if ( !bMustFit )
        {
            sal_Bool bWidow = sal_True;
            const SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
            if ( bMoveable && !bFormatted && ( GetFollow() ||
                 ( (Frm().*fnRect->fnOverStep)( nDeadLine ) ) ) )
            {
                Prepare( PREP_WIDOWS_ORPHANS, 0, sal_False );
                bValidSize = bWidow = sal_False;
            }
            if( (Frm().*fnRect->fnGetPos)() != aOldFrmPos ||
                (Prt().*fnRect->fnGetPos)() != aOldPrtPos )
            {
                // In diesem Prepare erfolgt ggf. ein _InvalidateSize().
                // bValidSize wird sal_False und das Format() wird gerufen.
                Prepare( PREP_POS_CHGD, (const void*)&bFormatted, sal_False );
                if ( bWidow && GetFollow() )
                {   Prepare( PREP_WIDOWS_ORPHANS, 0, sal_False );
                    bValidSize = sal_False;
                }
            }
        }
        if ( !bValidSize )
        {
            bValidSize = bFormatted = sal_True;
            ++nFormatCount;
            if( nFormatCount > STOP_FLY_FORMAT )
                SetFlyLock( sal_True );
            // --> OD 2006-09-25 #b6448963# - loop prevention
            // No format any longer, if <cnStopFormat> consequetive formats
            // without change occur.
            if ( nConsequetiveFormatsWithoutChange <= cnStopFormat )
            {
                Format();
            }
#if OSL_DEBUG_LEVEL > 1
            else
            {
                OSL_FAIL( "debug assertion: <SwCntntFrm::MakeAll()> - format of text frame suppressed by fix b6448963" );
            }
#endif
            // <--
        }

        //Wenn ich der erste einer Kette bin koennte ich mal sehen ob
        //ich zurueckfliessen kann (wenn ich mich ueberhaupt bewegen soll).
        //Damit es keine Oszillation gibt, darf ich nicht gerade vorwaerts
        //geflossen sein.
        sal_Bool bDummy;
        if ( !lcl_Prev( this ) &&
             !bMovedFwd &&
             ( bMoveable || ( bFly && !bTab ) ) &&
             ( !bFtn || !GetUpper()->FindFtnFrm()->GetPrev() )
             && MoveBwd( bDummy ) )
        {
            SWREFRESHFN( this )
            bMovedBwd = sal_True;
            bFormatted = sal_False;
            if ( bKeep && bMoveable )
            {
                if( CheckMoveFwd( bMakePage, sal_False, bMovedBwd ) )
                {
                    bMovedFwd = sal_True;
                    bMoveable = IsMoveable();
                    SWREFRESHFN( this )
                }
                Point aOldPos = (Frm().*fnRect->fnGetPos)();
                MakePos();
                if( aOldPos != (Frm().*fnRect->fnGetPos)() )
                {
                    Prepare( PREP_POS_CHGD, (const void*)&bFormatted, sal_False );
                    if ( !bValidSize )
                    {
                        (Frm().*fnRect->fnSetWidth)( (GetUpper()->
                                                Prt().*fnRect->fnGetWidth)() );
                        if ( !bValidPrtArea )
                        {
                            const long nOldW = (Prt().*fnRect->fnGetWidth)();
                            MakePrtArea( rAttrs );
                            if( nOldW != (Prt().*fnRect->fnGetWidth)() )
                                Prepare( PREP_FIXSIZE_CHG, 0, sal_False );
                        }
                        if( GetFollow() )
                            Prepare( PREP_WIDOWS_ORPHANS, 0, sal_False );
                        bValidSize = bFormatted = sal_True;
                        Format();
                    }
                }
                SwFrm *pNxt = HasFollow() ? NULL : FindNext();
                while( pNxt && pNxt->IsSctFrm() )
                {   // Leere Bereiche auslassen, in die anderen hinein
                    if( ((SwSectionFrm*)pNxt)->GetSection() )
                    {
                        SwFrm* pTmp = ((SwSectionFrm*)pNxt)->ContainsAny();
                        if( pTmp )
                        {
                            pNxt = pTmp;
                            break;
                        }
                    }
                    pNxt = pNxt->FindNext();
                }
                if ( pNxt )
                {
                    pNxt->Calc();
                    if( bValidPos && !GetIndNext() )
                    {
                        SwSectionFrm *pSct = FindSctFrm();
                        if( pSct && !pSct->GetValidSizeFlag() )
                        {
                            SwSectionFrm* pNxtSct = pNxt->FindSctFrm();
                            if( pNxtSct && pSct->IsAnFollow( pNxtSct ) )
                                bValidPos = sal_False;
                        }
                        else
                            bValidPos = sal_False;
                    }
                }
            }
        }

        //Der TxtFrm Validiert sich bei Fussnoten ggf. selbst, dass kann leicht
        //dazu fuehren, dass seine Position obwohl unrichtig valide ist.
        if ( bValidPos )
        {
            // --> OD 2006-01-23 #i59341#
            // Workaround for inadequate layout algorithm:
            // suppress invalidation and calculation of position, if paragraph
            // has formatted itself at least STOP_FLY_FORMAT times and
            // has anchored objects.
            // Thus, the anchored objects get the possibility to format itself
            // and this probably solve the layout loop.
            if ( bFtn &&
                 nFormatCount <= STOP_FLY_FORMAT &&
                 !GetDrawObjs() )
            // <--
            {
                bValidPos = sal_False;
                MakePos();
                aOldFrmPos = (Frm().*fnRect->fnGetPos)();
                aOldPrtPos = (Prt().*fnRect->fnGetPos)();
            }
        }

        // --> OD 2006-09-25 #b6448963# - loop prevention
        {
            if ( aOldFrm_StopFormat == Frm() &&
                 aOldPrt_StopFormat == Prt() )
            {
                ++nConsequetiveFormatsWithoutChange;
            }
            else
            {
                nConsequetiveFormatsWithoutChange = 0;
            }
        }
        // <--

        //Wieder ein Wert ungueltig? - dann nochmal das ganze...
        if ( !bValidPos || !bValidSize || !bValidPrtArea )
            continue;

        //Fertig?
        // Achtung, wg. Hoehe==0, ist es besser statt Bottom() Top()+Height() zu nehmen
        // (kommt bei Undersized TxtFrms an der Unterkante eines spaltigen Bereichs vor)
        const long nPrtBottom = (GetUpper()->*fnRect->fnGetPrtBottom)();
        const long nBottomDist =  (Frm().*fnRect->fnBottomDist)( nPrtBottom );
        if( nBottomDist >= 0 )
        {
            if ( bKeep && bMoveable )
            {
                //Wir sorgen dafuer, dass der Nachfolger gleich mit formatiert
                //wird. Dadurch halten wir das Heft in der Hand, bis wirklich
                //(fast) alles stabil ist. So vermeiden wir Endlosschleifen,
                //die durch staendig wiederholte Versuche entstehen.
                //Das bMoveFwdInvalid ist fuer #38407# notwendig. War urspruenglich
                //in flowfrm.cxx rev 1.38 behoben, das unterbrach aber obiges
                //Schema und spielte lieber Tuerme von Hanoi (#43669#).
                SwFrm *pNxt = HasFollow() ? NULL : FindNext();
                // Bei Bereichen nehmen wir lieber den Inhalt, denn nur
                // dieser kann ggf. die Seite wechseln
                while( pNxt && pNxt->IsSctFrm() )
                {
                    if( ((SwSectionFrm*)pNxt)->GetSection() )
                    {
                        pNxt = ((SwSectionFrm*)pNxt)->ContainsAny();
                        break;
                    }
                    pNxt = pNxt->FindNext();
                }
                if ( pNxt )
                {
                    const sal_Bool bMoveFwdInvalid = 0 != GetIndNext();
                    const sal_Bool bNxtNew =
                        ( 0 == (pNxt->Prt().*fnRect->fnGetHeight)() ) &&
                        (!pNxt->IsTxtFrm() ||!((SwTxtFrm*)pNxt)->IsHiddenNow());

                    pNxt->Calc();

                    if ( !bMovedBwd &&
                         ((bMoveFwdInvalid && !GetIndNext()) ||
                          bNxtNew) )
                    {
                        if( bMovedFwd )
                            pNotify->SetInvaKeep();
                        bMovedFwd = sal_False;
                    }
                }
            }
            continue;
        }

        //Ich passe nicht mehr in meinen Uebergeordneten, also ist es jetzt
        //an der Zeit moeglichst konstruktive Veranderungen vorzunehmen

        //Wenn ich den uebergeordneten Frm nicht verlassen darf, habe
        //ich ein Problem; Frei nach Artur Dent tun wir das einzige das man
        //mit einen nicht loesbaren Problem tun kann: wir ignorieren es - und
        //zwar mit aller Kraft.
        if ( !bMoveable || IsUndersized() )
        {
            if( !bMoveable && IsInTab() )
            {
                long nDiff = -(Frm().*fnRect->fnBottomDist)(
                                        (GetUpper()->*fnRect->fnGetPrtBottom)() );
                long nReal = GetUpper()->Grow( nDiff );
                if( nReal )
                    continue;
            }
            break;
        }

        //Wenn ich nun ueberhaupt ganz und garnicht in meinen Upper passe
        //so kann die Situation vielleicht doch noch durch Aufbrechen
        //aufgeklart werden. Diese Situation tritt bei einem frisch
        //erzeugten Follow auf, der zwar auf die Folgeseite geschoben wurde
        //aber selbst noch zu gross fuer diese ist; also wiederum
        //aufgespalten werden muss.
        //Wenn ich nicht passe und nicht Spaltbar (WouldFit()) bin, so schicke
        //ich meinem TxtFrmanteil die Nachricht, dass eben falls moeglich
        //trotz des Attributes 'nicht aufspalten' aufgespalten werden muss.
        sal_Bool bMoveOrFit = sal_False;
        sal_Bool bDontMoveMe = !GetIndPrev();
        if( bDontMoveMe && IsInSct() )
        {
            SwFtnBossFrm* pBoss = FindFtnBossFrm();
            bDontMoveMe = !pBoss->IsInSct() ||
                          ( !pBoss->Lower()->GetNext() && !pBoss->GetPrev() );
        }

        // Finally, we are able to split table rows. Therefore, bDontMoveMe
        // can be set to sal_False:
        if( bDontMoveMe && IsInTab() &&
            0 != const_cast<SwCntntFrm*>(this)->GetNextCellLeaf( MAKEPAGE_NONE ) )
            bDontMoveMe = sal_False;

        if ( bDontMoveMe && (Frm().*fnRect->fnGetHeight)() >
                            (GetUpper()->Prt().*fnRect->fnGetHeight)() )
        {
            if ( !bFitPromise )
            {
                SwTwips nTmp = (GetUpper()->Prt().*fnRect->fnGetHeight)() -
                               (Prt().*fnRect->fnGetTop)();
                sal_Bool bSplit = !IsFwdMoveAllowed();
                if ( nTmp > 0 && WouldFit( nTmp, bSplit, sal_False ) )
                {
                    Prepare( PREP_WIDOWS_ORPHANS, 0, sal_False );
                    bValidSize = sal_False;
                    bFitPromise = sal_True;
                    continue;
                }
                /* --------------------------------------------------
                 * Frueher wurde in Rahmen und Bereichen niemals versucht,
                 * durch bMoveOrFit den TxtFrm unter Verzicht auf seine
                 * Attribute (Widows,Keep) doch noch passend zu bekommen.
                 * Dies haette zumindest bei spaltigen Rahmen versucht
                 * werden muessen, spaetestens bei verketteten Rahmen und
                 * in Bereichen muss es versucht werden.
                 * Ausnahme: Wenn wir im FormatWidthCols stehen, duerfen die
                 * Attribute nicht ausser Acht gelassen werden.
                 * --------------------------------------------------*/
                else if ( !bFtn && bMoveable &&
                      ( !bFly || !FindFlyFrm()->IsColLocked() ) &&
                      ( !bSct || !FindSctFrm()->IsColLocked() ) )
                    bMoveOrFit = sal_True;
            }
#if OSL_DEBUG_LEVEL > 1
            else
            {
                OSL_FAIL( "+TxtFrm hat WouldFit-Versprechen nicht eingehalten." );
            }
#endif
        }

        //Mal sehen ob ich irgenwo Platz finde...
        //Benachbarte Fussnoten werden in _MoveFtnCntFwd 'vorgeschoben'
        SwFrm *pPre = GetIndPrev();
        SwFrm *pOldUp = GetUpper();

/* MA 13. Oct. 98: Was soll das denn sein!?
 * AMA 14. Dec 98: Wenn ein spaltiger Bereich keinen Platz mehr fuer seinen ersten ContentFrm
 *      bietet, so soll dieser nicht nur in die naechste Spalte, sondern ggf. bis zur naechsten
 *      Seite wandern und dort einen Section-Follow erzeugen.
 */
        if( IsInSct() && bMovedFwd && bMakePage && pOldUp->IsColBodyFrm() &&
            pOldUp->GetUpper()->GetUpper()->IsSctFrm() &&
            ( pPre || pOldUp->GetUpper()->GetPrev() ) &&
            ((SwSectionFrm*)pOldUp->GetUpper()->GetUpper())->MoveAllowed(this) )
            bMovedFwd = sal_False;

        const sal_Bool bCheckForGrownBody = pOldUp->IsBodyFrm();
        const long nOldBodyHeight = (pOldUp->Frm().*fnRect->fnGetHeight)();

        if ( !bMovedFwd && !MoveFwd( bMakePage, sal_False ) )
            bMakePage = sal_False;
        SWREFRESHFN( this )

        // If MoveFwd moves the paragraph to the next page, a following
        // paragraph, which contains footnotes can can cause the old upper
        // frame to grow. In this case we explicitely allow a new check
        // for MoveBwd. Robust: We also check the bMovedBwd flag again.
        // If pOldUp was a footnote frame, it has been deleted inside MoveFwd.
        // Therefore we only check for growing body frames.
        if ( bCheckForGrownBody && ! bMovedBwd && pOldUp != GetUpper() &&
             (pOldUp->Frm().*fnRect->fnGetHeight)() > nOldBodyHeight )
            bMovedFwd = sal_False;
        else
            bMovedFwd = sal_True;

        bFormatted = sal_False;
        if ( bMoveOrFit && GetUpper() == pOldUp )
        {
            // FME 2007-08-30 #i81146# new loop control
            if ( nConsequetiveFormatsWithoutChange <= cnStopFormat )
            {
                Prepare( PREP_MUST_FIT, 0, sal_False );
                bValidSize = sal_False;
                bMustFit = sal_True;
                continue;
            }

#if OSL_DEBUG_LEVEL > 1
            OSL_FAIL( "LoopControl in SwCntntFrm::MakeAll" );
#endif
        }
        if ( bMovedBwd && GetUpper() )
        {   //Unuetz gewordene Invalidierungen zuruecknehmen.
            GetUpper()->ResetCompletePaint();
            if( pPre && !pPre->IsSctFrm() )
                ::ValidateSz( pPre );
        }

    } //while ( !bValidPos || !bValidSize || !bValidPrtArea )


    // NEW: Looping Louie (Light). Should not be applied in balanced sections.
    // Should only be applied if there is no better solution!
    LOOPING_LOUIE_LIGHT( bMovedFwd && bMovedBwd && !IsInBalancedSection() &&
                            (

                                // --> FME 2005-01-26 #118572#
                                ( bFtn && !FindFtnFrm()->GetRef()->IsInSct() ) ||
                                // <--

                                // --> FME 2005-01-27 #i33887#
                                ( IsInSct() && bKeep )
                                // <--

                                // ... add your conditions here ...

                            ),
                         static_cast<SwTxtFrm&>(*this) );


    if ( pSaveFtn )
        delete pSaveFtn;

    UnlockJoin();
    if ( bMovedFwd || bMovedBwd )
        pNotify->SetInvaKeep();
    // OD 2004-02-26 #i25029#
    if ( bMovedFwd )
    {
        pNotify->SetInvalidatePrevPrtArea();
    }
    delete pNotify;
    SetFlyLock( sal_False );
}

/*************************************************************************
|*
|*  SwCntntFrm::_WouldFit()
|*
|*************************************************************************/




void MakeNxt( SwFrm *pFrm, SwFrm *pNxt )
{
    //fix(25455): Validieren, sonst kommt es zu einer Rekursion.
    //Der erste Versuch, der Abbruch mit pFrm = 0 wenn !Valid,
    //fuehrt leider zu dem Problem, dass das Keep dann u.U. nicht mehr
    //korrekt beachtet wird (27417)
    const sal_Bool bOldPos = pFrm->GetValidPosFlag();
    const sal_Bool bOldSz  = pFrm->GetValidSizeFlag();
    const sal_Bool bOldPrt = pFrm->GetValidPrtAreaFlag();
    pFrm->bValidPos = pFrm->bValidPrtArea = pFrm->bValidSize = sal_True;

    //fix(29272): Nicht MakeAll rufen, dort wird evtl. pFrm wieder invalidert
    //und kommt rekursiv wieder herein.
    if ( pNxt->IsCntntFrm() )
    {
        SwCntntNotify aNotify( (SwCntntFrm*)pNxt );
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pNxt );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( !pNxt->GetValidSizeFlag() )
        {
            if( pNxt->IsVertical() )
                pNxt->Frm().Height( pNxt->GetUpper()->Prt().Height() );
            else
                pNxt->Frm().Width( pNxt->GetUpper()->Prt().Width() );
        }
        ((SwCntntFrm*)pNxt)->MakePrtArea( rAttrs );
        pNxt->Format( &rAttrs );
    }
    else
    {
        SwLayNotify aNotify( (SwLayoutFrm*)pNxt );
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pNxt );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( !pNxt->GetValidSizeFlag() )
        {
            if( pNxt->IsVertical() )
                pNxt->Frm().Height( pNxt->GetUpper()->Prt().Height() );
            else
                pNxt->Frm().Width( pNxt->GetUpper()->Prt().Width() );
        }
        pNxt->Format( &rAttrs );
    }

    pFrm->bValidPos      = bOldPos;
    pFrm->bValidSize     = bOldSz;
    pFrm->bValidPrtArea  = bOldPrt;
}

// Diese Routine ueberprueft, ob zwischen dem FtnBoss von pFrm und dem
// von pNxt keine anderen FtnBosse liegen

sal_Bool lcl_IsNextFtnBoss( const SwFrm *pFrm, const SwFrm* pNxt )
{
    OSL_ENSURE( pFrm && pNxt, "lcl_IsNextFtnBoss: No Frames?" );
    pFrm = pFrm->FindFtnBossFrm();
    pNxt = pNxt->FindFtnBossFrm();
    // Falls pFrm eine letzte Spalte ist, wird stattdessen die Seite genommen
    while( pFrm && pFrm->IsColumnFrm() && !pFrm->GetNext() )
        pFrm = pFrm->GetUpper()->FindFtnBossFrm();
    // Falls pNxt eine erste Spalte ist, wird stattdessen die Seite genommen
    while( pNxt && pNxt->IsColumnFrm() && !pNxt->GetPrev() )
        pNxt = pNxt->GetUpper()->FindFtnBossFrm();
    // So, jetzt muessen pFrm und pNxt entweder zwei benachbarte Seiten oder Spalten sein.
    return ( pFrm && pNxt && pFrm->GetNext() == pNxt );
}

// --> OD 2007-11-26 #b6614158#
sal_Bool SwCntntFrm::_WouldFit( SwTwips nSpace,
                            SwLayoutFrm *pNewUpper,
                            sal_Bool bTstMove,
                            const bool bObjsInNewUpper )
// <--
{
    //Damit die Fussnote sich ihren Platz sorgsam waehlt, muss
    //sie in jedem Fall gemoved werden, wenn zwischen dem
    //neuen Upper und ihrer aktuellen Seite/Spalte mindestens eine
    //Seite/Spalte liegt.
    SwFtnFrm* pFtnFrm = 0;
    if ( IsInFtn() )
    {
        if( !lcl_IsNextFtnBoss( pNewUpper, this ) )
            return sal_True;
        pFtnFrm = FindFtnFrm();
    }

    sal_Bool bRet;
    sal_Bool bSplit = !pNewUpper->Lower();
    SwCntntFrm *pFrm = this;
    const SwFrm *pTmpPrev = pNewUpper->Lower();
    if( pTmpPrev && pTmpPrev->IsFtnFrm() )
        pTmpPrev = ((SwFtnFrm*)pTmpPrev)->Lower();
    while ( pTmpPrev && pTmpPrev->GetNext() )
        pTmpPrev = pTmpPrev->GetNext();
    do
    {
        // --> FME 2005-03-31 #b6236853# #i46181#
        SwTwips nSecondCheck = 0;
        SwTwips nOldSpace = nSpace;
        sal_Bool bOldSplit = bSplit;
        // <--

        if ( bTstMove || IsInFly() || ( IsInSct() &&
             ( pFrm->GetUpper()->IsColBodyFrm() || ( pFtnFrm &&
               pFtnFrm->GetUpper()->GetUpper()->IsColumnFrm() ) ) ) )
        {
            //Jetzt wirds ein bischen hinterlistig; empfindliche Gemueter sollten
            //lieber wegsehen. Wenn ein Flys Spalten enthaelt so sind die Cntnts
            //moveable, mit Ausnahme der in der letzten Spalte (siehe
            //SwFrm::IsMoveable()). Zurueckfliessen duerfen sie aber natuerlich.
            //Das WouldFit() liefert leider nur dann einen vernueftigen Wert, wenn
            //der Frm moveable ist. Um dem WouldFit() einen Moveable Frm
            //vorzugaukeln haenge ich ihn einfach solange um.
            // Auch bei spaltigen Bereichen muss umgehaengt werden, damit
            // SwSectionFrm::Growable() den richtigen Wert liefert.
            // Innerhalb von Fussnoten muss ggf. sogar der SwFtnFrm umgehaengt werden,
            // falls es dort keinen SwFtnFrm gibt.
            SwFrm* pTmpFrm = pFrm->IsInFtn() && !pNewUpper->FindFtnFrm() ?
                             (SwFrm*)pFrm->FindFtnFrm() : pFrm;
            SwLayoutFrm *pUp = pTmpFrm->GetUpper();
            SwFrm *pOldNext = pTmpFrm->GetNext();
            pTmpFrm->Remove();
            pTmpFrm->InsertBefore( pNewUpper, 0 );
            if ( pFrm->IsTxtFrm() &&
                 ( bTstMove ||
                   ((SwTxtFrm*)pFrm)->HasFollow() ||
                   ( !((SwTxtFrm*)pFrm)->HasPara() &&
                     !((SwTxtFrm*)pFrm)->IsEmpty()
                   )
                 )
               )
            {
                bTstMove = sal_True;
                bRet = ((SwTxtFrm*)pFrm)->TestFormat( pTmpPrev, nSpace, bSplit );
            }
            else
                bRet = pFrm->WouldFit( nSpace, bSplit, sal_False );

            pTmpFrm->Remove();
            pTmpFrm->InsertBefore( pUp, pOldNext );
        }
        else
        {
            bRet = pFrm->WouldFit( nSpace, bSplit, sal_False );
            nSecondCheck = !bSplit ? 1 : 0;
        }

        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
        const SwBorderAttrs &rAttrs = *aAccess.Get();

        //Bitter aber wahr: Der Abstand muss auch noch mit einkalkuliert werden.
        //Bei TestFormatierung ist dies bereits geschehen.
        if ( bRet && !bTstMove )
        {
            SwTwips nUpper;

            if ( pTmpPrev )
            {
                nUpper = CalcUpperSpace( NULL, pTmpPrev );

                // in balanced columned section frames we do not want the
                // common border
                sal_Bool bCommonBorder = sal_True;
                if ( pFrm->IsInSct() && pFrm->GetUpper()->IsColBodyFrm() )
                {
                    const SwSectionFrm* pSct = pFrm->FindSctFrm();
                    bCommonBorder = pSct->GetFmt()->GetBalancedColumns().GetValue();
                }

                // --> FME 2005-03-31 #b6236853# #i46181#
                nSecondCheck = ( 1 == nSecondCheck &&
                                 pFrm == this &&
                                 IsTxtFrm() &&
                                 bCommonBorder &&
                                 !static_cast<const SwTxtFrm*>(this)->IsEmpty() ) ?
                                 nUpper :
                                 0;
                // <--

                nUpper += bCommonBorder ?
                          rAttrs.GetBottomLine( *(pFrm) ) :
                          rAttrs.CalcBottomLine();

            }
            else
            {
                // --> FME 2005-03-31 #b6236853# #i46181#
                nSecondCheck = 0;
                // <--

                if( pFrm->IsVertical() )
                    nUpper = pFrm->Frm().Width() - pFrm->Prt().Width();
                else
                    nUpper = pFrm->Frm().Height() - pFrm->Prt().Height();
            }

            nSpace -= nUpper;

            if ( nSpace < 0 )
            {
                bRet = sal_False;

                // --> FME 2005-03-31 #b6236853# #i46181#
                if ( nSecondCheck > 0 )
                {
                    // The following code is indented to solve a (rare) problem
                    // causing some frames not to move backward:
                    // SwTxtFrm::WouldFit() claims that the whole paragraph
                    // fits into the given space and subtracts the height of
                    // all lines from nSpace. nSpace - nUpper is not a valid
                    // indicator if the frame should be allowed to move backward.
                    // We do a second check with the original remaining space
                    // reduced by the required upper space:
                    nOldSpace -= nSecondCheck;
                    const bool bSecondRet = nOldSpace >= 0 && pFrm->WouldFit( nOldSpace, bOldSplit, sal_False );
                    if ( bSecondRet && bOldSplit && nOldSpace >= 0 )
                    {
                        bRet = sal_True;
                        bSplit = sal_True;
                    }
                }
                // <--
            }
        }

        // OD 2004-03-01 #106629# - also consider lower spacing in table cells
        if ( bRet && IsInTab() &&
             pNewUpper->GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) )
        {
            nSpace -= rAttrs.GetULSpace().GetLower();
            if ( nSpace < 0 )
            {
                bRet = sal_False;
            }
        }

        if ( bRet && !bSplit && pFrm->IsKeep( rAttrs.GetAttrSet() ) )
        {
            if( bTstMove )
            {
                while( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->HasFollow() )
                {
                    pFrm = ((SwTxtFrm*)pFrm)->GetFollow();
                }
                // OD 11.04.2003 #108824# - If last follow frame of <this> text
                // frame isn't valid, a formatting of the next content frame
                // doesn't makes sense. Thus, return sal_True.
                if ( IsAnFollow( pFrm ) && !pFrm->IsValid() )
                {
                    OSL_FAIL( "Only a warning for task 108824:/n<SwCntntFrm::_WouldFit(..) - follow not valid!" );
                    return sal_True;
                }
            }
            SwFrm *pNxt;
            if( 0 != (pNxt = pFrm->FindNext()) && pNxt->IsCntntFrm() &&
                ( !pFtnFrm || ( pNxt->IsInFtn() &&
                  pNxt->FindFtnFrm()->GetAttr() == pFtnFrm->GetAttr() ) ) )
            {
                // ProbeFormatierung vertraegt keine absatz- oder gar zeichengebundene Objekte
                // --> OD 2007-11-26 #b6614158#
                // current solution for the test formatting doesn't work, if
                // objects are present in the remaining area of the new upper
                if ( bTstMove &&
                     ( pNxt->GetDrawObjs() || bObjsInNewUpper ) )
                {
                    return sal_True;
                }
                // <--

                if ( !pNxt->IsValid() )
                    MakeNxt( pFrm, pNxt );

                //Kleiner Trick: Wenn der naechste einen Vorgaenger hat, so hat
                //er den Absatzabstand bereits berechnet. Er braucht dann nicht
                //teuer kalkuliert werden.
                if( lcl_NotHiddenPrev( pNxt ) )
                    pTmpPrev = 0;
                else
                {
                    if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsHiddenNow() )
                        pTmpPrev = lcl_NotHiddenPrev( pFrm );
                    else
                        pTmpPrev = pFrm;
                }
                pFrm = (SwCntntFrm*)pNxt;
            }
            else
                pFrm = 0;
        }
        else
            pFrm = 0;

    } while ( bRet && pFrm );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
