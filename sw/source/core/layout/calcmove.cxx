/*************************************************************************
 *
 *  $RCSfile: calcmove.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: hjs $ $Date: 2003-09-25 10:49:15 $
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


#pragma hdrstop

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

#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif

#include "tabfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "frmsh.hxx"
#include "pagedesc.hxx"
#include "ftninfo.hxx"
#include "sectfrm.hxx"
#include "dbg_lay.hxx"


//------------------------------------------------------------------------
//              Move-Methoden
//------------------------------------------------------------------------

/*************************************************************************
|*
|*  SwCntntFrm::ShouldBwdMoved()
|*
|*  Beschreibung        Returnwert sagt, ob der Frm verschoben werden sollte.
|*  Ersterstellung      MA 05. Dec. 96
|*  Letzte Aenderung    MA 05. Dec. 96
|*
|*************************************************************************/


BOOL SwCntntFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, BOOL, BOOL & )
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
        BYTE nMoveAnyway = 0;
        SwPageFrm * const pNewPage = pNewUpper->FindPageFrm();
        SwPageFrm *pOldPage = FindPageFrm();

        if ( SwFlowFrm::IsMoveBwdJump() )
            return TRUE;

        if( IsInFtn() && IsInSct() )
        {
            SwFtnFrm* pFtn = FindFtnFrm();
            SwSectionFrm* pMySect = pFtn->FindSctFrm();
            if( pMySect && pMySect->IsFtnLock() )
            {
                SwSectionFrm *pSect = pNewUpper->FindSctFrm();
                while( pSect && pSect->IsInFtn() )
                    pSect = pSect->GetUpper()->FindSctFrm();
                ASSERT( pSect, "Escaping footnote" );
                if( pSect != pMySect )
                    return FALSE;
            }
        }
        SWRECTFN( this )
        SWRECTFNX( pNewUpper )
        if( Abs( (pNewUpper->Prt().*fnRectX->fnGetWidth)() -
                 (GetUpper()->Prt().*fnRect->fnGetWidth)() ) > 1 )
            nMoveAnyway = 2; // Damit kommt nur noch ein _WouldFit mit Umhaengen in Frage
        if ( (nMoveAnyway |= BwdMoveNecessary( pOldPage, Frm() )) < 3 )
        {
            SwTwips nSpace = 0;
            SwRect aRect( pNewUpper->Prt() );
            aRect.Pos() += pNewUpper->Frm().Pos();
            const SwFrm *pPrevFrm = pNewUpper->Lower();
            while ( pPrevFrm )
            {
                (aRect.*fnRectX->fnSetTop)(
                    (pPrevFrm->Frm().*fnRectX->fnGetBottom)() );
                pPrevFrm = pPrevFrm->GetNext();
            }

            nMoveAnyway |= BwdMoveNecessary( pNewPage, aRect);
            if ( nMoveAnyway < 3 )
            {
                //Zur Verfuegung stehenden Raum berechnen.
                nSpace = (aRect.*fnRectX->fnGetHeight)();
                if ( IsInFtn() || GetAttrSet()->GetDoc()->IsBrowseMode() ||
                     ( pNewUpper->IsInSct() && ( pNewUpper->IsSctFrm() ||
                       ( pNewUpper->IsColBodyFrm() &&
                         !pNewUpper->GetUpper()->GetPrev() &&
                         !pNewUpper->GetUpper()->GetNext() ) ) ) )
                    nSpace += pNewUpper->Grow( LONG_MAX PHEIGHT, TRUE );
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
                    return _WouldFit( nSpace, pNewUpper, nMoveAnyway == 2 );
                }
                //Bei einem spaltigen Bereichsfrischling kann _WouldFit kein
                //brauchbares Ergebnis liefern, also muessen wir wirklich
                //zurueckfliessen
                else if( pNewUpper->IsInSct() && pNewUpper->IsColBodyFrm() &&
                    !(pNewUpper->Prt().*fnRectX->fnGetWidth)() &&
                    ( pNewUpper->GetUpper()->GetPrev() ||
                      pNewUpper->GetUpper()->GetNext() ) )
                    return TRUE;
                else
                    return FALSE; // Kein Platz, dann ist es sinnlos, zurueckzufliessen
            }
        }
        return TRUE;
    }
    return  FALSE;
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
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 13. Dec. 93
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



void SwFrm::PrepareMake()
{
    StackHack aHack;
    if ( GetUpper() )
    {
        if( !GetUpper()->IsSctFrm() && !GetUpper()->IsFooterFrm() )
            GetUpper()->Calc();
        ASSERT( GetUpper(), ":-( Layoutgeruest wackelig (Upper wech)." );
        if ( !GetUpper() )
            return;

        const BOOL bCnt = IsCntntFrm();
        const BOOL bTab = IsTabFrm();
        BOOL bNoSect = IsInSct();
        BOOL bOldTabLock = FALSE, bFoll = FALSE;
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
            bNoSect = FALSE;
        }
        else if ( bCnt && TRUE == (bFoll = pThis->IsFollow()) &&
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

        SwFrm *pFrm = GetUpper()->Lower();
        while ( pFrm != this )
        {
            ASSERT( pFrm, ":-( Layoutgeruest wackelig (this not found)." );
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
        ASSERT( GetUpper(), "Layoutgeruest wackelig (Upper wech II)." );
        if ( !GetUpper() )
            return;

        if( !GetUpper()->IsSctFrm() && !GetUpper()->IsFooterFrm() )
            GetUpper()->Calc();

        ASSERT( GetUpper(), "Layoutgeruest wackelig (Upper wech III)." );

        if ( bTab && !bOldTabLock )
            ::PrepareUnlock( (SwTabFrm*)this );
    }
    MakeAll();
}

void SwFrm::OptPrepareMake()
{
    if ( GetUpper() && !GetUpper()->IsFooterFrm() )
    {
        GetUpper()->Calc();
        ASSERT( GetUpper(), ":-( Layoutgeruest wackelig (Upper wech)." );
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

        ASSERT( GetUpper(), ":-( Layoutgeruest wackelig (Upper wech)." );
        if ( !GetUpper() )
            return;

        const BOOL bCnt = IsCntntFrm();
        const BOOL bTab = IsTabFrm();
        BOOL bNoSect = IsInSct();

        BOOL bOldTabLock = FALSE, bFoll;
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
            bNoSect = FALSE;
        }
        bFoll = pThis && pThis->IsFollow();

        SwFrm *pFrm = GetUpper()->Lower();
        while ( pFrm != this )
        {
            ASSERT( pFrm, ":-( Layoutgeruest wackelig (this not found)." );
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
        ASSERT( GetUpper(), "Layoutgeruest wackelig (Upper wech II)." );
        if ( !GetUpper() )
            return;

        GetUpper()->Calc();

        ASSERT( GetUpper(), "Layoutgeruest wackelig (Upper wech III)." );

        if ( bTab && !bOldTabLock )
            ::PrepareUnlock( (SwTabFrm*)this );
    }
    Calc();
}

/*************************************************************************
|*
|*  SwFrm::MakePos()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 24. May. 93
|*
|*************************************************************************/

// Hier wird GetPrev() zurueckgegeben, allerdings werden
// dabei leere SectionFrms ueberlesen
SwFrm* lcl_Prev( SwFrm* pFrm, BOOL bSectPrv = TRUE )
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
        bValidPos = TRUE;
        FASTBOOL bUseUpper = FALSE;
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
                bUseUpper = TRUE;
            }
        }

        pPrv = lcl_Prev( this, FALSE );
        USHORT nMyType = GetType();
        SWRECTFN( this )
        if ( !bUseUpper && pPrv )
        {
            aFrm.Pos( pPrv->Frm().Pos() );
            if( FRM_NEIGHBOUR & nMyType )
            {
                BOOL bR2L = IsRightToLeft();
                if( bR2L )
                    (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() -
                                               (aFrm.*fnRect->fnGetWidth)() );
                else
                    (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() +
                                          (pPrv->Frm().*fnRect->fnGetWidth)() );
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
        else if ( GetUpper() )
        {
            /// OD 15.10.2002 #103517# - add safeguard for <SwFooterFrm::Calc()>
            /// If parent frame is a footer frame and its <ColLocked()>, then
            /// do *not* calculate it.
            /// NOTE: Footer frame is <ColLocked()> during its
            ///     <FormatSize(..)>, which is called from <Format(..)>, which
            ///     is called from <MakeAll()>, which is called from <Calc()>.
            if ( !GetUpper()->IsSctFrm() &&
                 !( GetUpper()->IsFooterFrm() &&
                    GetUpper()->IsColLocked() )
               )
            {
                SwFlyFrm* pTmpFly = FindFlyFrm();
                if( !pTmpFly || !pTmpFly->IsFlyInCntFrm() )
                    GetUpper()->Calc();
            }
            pPrv = lcl_Prev( this, FALSE );
            if ( !bUseUpper && pPrv )
            {
                aFrm.Pos( pPrv->Frm().Pos() );
                if( FRM_NEIGHBOUR & nMyType )
                {
                    BOOL bR2L = IsRightToLeft();
                    if( bR2L )
                        (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() -
                                                 (aFrm.*fnRect->fnGetWidth)() );
                    else
                        (aFrm.*fnRect->fnSetPosX)( (aFrm.*fnRect->fnGetLeft)() +
                                          (pPrv->Frm().*fnRect->fnGetWidth)() );
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
                else if( bVert && FRM_NOTE_VERT & nMyType && !bReverse )
                    aFrm.Pos().X() -= aFrm.Width() - GetUpper()->Prt().Width();
            }
        }
        else
            aFrm.Pos().X() = aFrm.Pos().Y() = 0;
        if( IsBodyFrm() && bVert && !bReverse && GetUpper() )
            aFrm.Pos().X() += GetUpper()->Prt().Width() - aFrm.Width();
        bValidPos = TRUE;
    }
}

/*************************************************************************
|*
|*  SwPageFrm::MakeAll()
|*
|*  Ersterstellung      MA 23. Feb. 93
|*  Letzte Aenderung    MA 20. Jul. 98
|*
|*************************************************************************/

void lcl_CheckObjects( SwSortDrawObjs* pSortedObjs, SwFrm* pFrm, long& rBot )
{
    //Und dann kann es natuerlich noch Absatzgebundene
    //Rahmen geben, die unterhalb ihres Absatzes stehen.
    long nMax = 0;
    for ( USHORT i = 0; i < pSortedObjs->Count(); ++i )
    {
        SdrObject *pObj = (*pSortedObjs)[i];
        long nTmp = 0;
        if ( pObj->IsWriterFlyFrame() )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            if( pFly->Frm().Top() != WEIT_WECH &&
                ( pFrm->IsPageFrm() ? pFly->IsFlyLayFrm() :
                  ( pFly->IsFlyAtCntFrm() &&
                    ( pFrm->IsBodyFrm() ? pFly->GetAnchor()->IsInDocBody() :
                                          pFly->GetAnchor()->IsInFtn() ) ) ) )
            {
                nTmp = pFly->Frm().Bottom();
            }
        }
        else
            nTmp = pObj->GetBoundRect().Bottom();
        nMax = Max( nTmp, nMax );
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
            MakePos();
            if ( GetPrev() &&  !((SwPageFrm*)GetPrev())->IsEmptyPage() )
                aFrm.Pos().Y() += DOCUMENTBORDER/2;
        }

        if ( !bValidSize || !bValidPrtArea )
        {
            if ( IsEmptyPage() )
            {
                Frm().Width( 0 );  Prt().Width( 0 );
                Frm().Height( 0 ); Prt().Height( 0 );
                Prt().Left( 0 );   Prt().Top( 0 );
                bValidSize = bValidPrtArea = TRUE;
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
                if ( pSh && GetFmt()->GetDoc()->IsBrowseMode() )
                {
                    const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
                    const long nTop    = pAttrs->CalcTopLine()   + aBorder.Height();
                    const long nBottom = pAttrs->CalcBottomLine()+ aBorder.Height();

                    long nWidth = GetUpper() ? ((SwRootFrm*)GetUpper())->
                        GetBrowseWidth() + 2 * aBorder.Width() : 0;
//                  if ( !pSh->VisArea().Width() )
//                      nWidth = Max( nWidth, 5000L );
                    if ( nWidth < pSh->VisArea().Width() )
                        nWidth = pSh->VisArea().Width();
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
                                ASSERT( !(pFrm->Frm().Height() < pFrm->Prt().Height()),
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
                        if ( !GetPrev() )
                            nBot = Max( nBot, pSh->VisArea().Height() );
                        Frm().Height( nBot );
                    }
                    Prt().Left ( pAttrs->CalcLeftLine() + aBorder.Width() );
                    Prt().Top  ( nTop );
                    Prt().Width( Frm().Width() - ( Prt().Left()
                        + pAttrs->CalcRightLine() + aBorder.Width() ) );
                    Prt().Height( Frm().Height() - (nTop + nBottom) );
                    bValidSize = bValidPrtArea = TRUE;
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
    if ( Frm() != aOldRect )
        AdjustRootSize( CHG_CHGPAGE, &aOldRect );

#ifndef PRODUCT
    //Der Upper (Root) muss mindestens so breit
    //sein, dass er die breiteste Seite aufnehmen kann.
    if ( GetUpper() )
    {
        ASSERT( GetUpper()->Prt().Width() >= aFrm.Width(), "Rootsize" );
    }
#endif
}

/*************************************************************************
|*
|*  SwLayoutFrm::MakeAll()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 28. Nov. 95
|*
|*************************************************************************/


void SwLayoutFrm::MakeAll()
{
    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

        //uebernimmt im DTor die Benachrichtigung
    const SwLayNotify aNotify( this );
    BOOL bVert = IsVertical();
    SwRectFn fnRect = ( IsNeighbourFrm() == bVert )? fnRectHori : fnRectVert;

    SwBorderAttrAccess *pAccess = 0;
    const SwBorderAttrs*pAttrs = 0;

    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        if ( !bValidPos )
            MakePos();

        if ( GetUpper() )
        {
            if ( !bValidSize )
            {
                //FixSize einstellen, die VarSize wird von Format() nach
                //Berechnung der PrtArea eingestellt.
                bValidPrtArea = FALSE;
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
            {   // Don't leave your upper
                const SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                if( (Frm().*fnRect->fnOverStep)( nDeadLine ) )
                    bValidSize = FALSE;
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
|*  Ersterstellung      MA 17. Nov. 92
|*  Letzte Aenderung    MA 03. Mar. 96
|*
|*************************************************************************/

BOOL SwCntntFrm::MakePrtArea( const SwBorderAttrs &rAttrs )
{
    BOOL bSizeChgd = FALSE;

    if ( !bValidPrtArea )
    {
        bValidPrtArea = TRUE;

        SWRECTFN( this )
        const FASTBOOL bTxtFrm = IsTxtFrm();
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
#ifdef BIDI
            const long nRight = ((SwBorderAttrs&)rAttrs).CalcRight( this );
            (this->*fnRect->fnSetXMargins)( nLeft, nRight );
#else
            (this->*fnRect->fnSetXMargins)( nLeft, rAttrs.CalcRight() );
#endif

            ViewShell *pSh = GetShell();
            SwTwips nWidthArea;
            if( pSh && 0!=(nWidthArea=(pSh->VisArea().*fnRect->fnGetWidth)()) &&
                GetUpper()->IsPageBodyFrm() &&  // nicht dagegen bei BodyFrms in Columns
                pSh->GetDoc()->IsBrowseMode() )
            {
                //Nicht ueber die Kante des sichbaren Bereiches hinausragen.
                //Die Seite kann breiter sein, weil es Objekte mit "ueberbreite"
                //geben kann (RootFrm::ImplCalcBrowseWidth())
                long nMinWidth = 0;

                for (USHORT i = 0; GetDrawObjs() && i < GetDrawObjs()->Count();++i)
                {
                    SdrObject *pObj = (*GetDrawObjs())[i];
                    SwFrmFmt *pFmt = ::FindFrmFmt( pObj );
                    const FASTBOOL bFly = pObj->IsWriterFlyFrame();
                    if ( bFly &&
                         WEIT_WECH == ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Frm().Width()||
                         pFmt->GetFrmSize().GetWidthPercent() )
                        continue;

                    if ( FLY_IN_CNTNT == pFmt->GetAnchor().GetAnchorId() )
                        nMinWidth = Max( nMinWidth,
                                      bFly ? pFmt->GetFrmSize().GetWidth()
                                           : pObj->GetBoundRect().GetWidth() );
                }

                const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
                long nWidth = nWidthArea - 2 * ( IsVertical() ?
                                           aBorder.Width() : aBorder.Height() );
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
            // in balanced columned section frames we do not want the
            // common border
            sal_Bool bCommonBorder = sal_True;
            if ( IsInSct() && GetUpper()->IsColBodyFrm() )
            {
                const SwSectionFrm* pSct = FindSctFrm();
                bCommonBorder = pSct->GetFmt()->GetBalancedColumns().GetValue();
            }
            SwTwips nLower = bCommonBorder ?
                             rAttrs.GetBottomLine( this ) :
                             rAttrs.CalcBottomLine();

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
            bSizeChgd = TRUE;
        }
    }
    return bSizeChgd;
}

/*************************************************************************
|*
|*  SwCntntFrm::MakeAll()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 16. Dec. 96
|*
|*************************************************************************/

#define STOP_FLY_FORMAT 10

inline void ValidateSz( SwFrm *pFrm )
{
    if ( pFrm )
    {
        pFrm->bValidSize = TRUE;
        pFrm->bValidPrtArea = TRUE;
    }
}


void SwCntntFrm::MakeAll()
{
    ASSERT( GetUpper(), "keinen Upper?" );
    ASSERT( IsTxtFrm(), "MakeAll(), NoTxt" );

    if ( !IsFollow() && StackHack::IsLocked() )
        return;

    if ( IsJoinLocked() )
        return;

    ASSERT( !((SwTxtFrm*)this)->IsSwapped(), "Calculation of a swapped frame" );

    StackHack aHack;

    if ( ((SwTxtFrm*)this)->IsLocked() )
    {
        ASSERT( FALSE, "Format fuer gelockten TxtFrm." );
        return;
    }

    LockJoin();
    long nFormatCount = 0;
    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

#ifndef PRODUCT
    SwDoc *pDoc = GetAttrSet()->GetDoc();
    if( pDoc )
    {
        static sal_Bool bWarn = sal_False;
        if( pDoc->InXMLExport() )
        {
            ASSERT( bWarn, "Formatting during XML-export!" );
            bWarn = sal_True;
        }
        else
            bWarn = sal_False;
    }
#endif

    //uebernimmt im DTor die Benachrichtigung
    SwCntntNotify *pNotify = new SwCntntNotify( this );

    BOOL    bMakePage   = TRUE;     //solange TRUE kann eine neue Seite
                                    //angelegt werden (genau einmal)
    BOOL    bMovedBwd   = FALSE;    //Wird TRUE wenn der Frame zurueckfliesst
    BOOL    bMovedFwd   = FALSE;    //solange FALSE kann der Frm zurueck-
                                    //fliessen (solange, bis er einmal
                                    //vorwaerts ge'moved wurde).
    BOOL    bFormatted  = FALSE;    //Fuer die Witwen und Waisen Regelung
                                    //wird der letzte CntntFrm einer Kette
                                    //u.U. zum Formatieren angeregt, dies
                                    //braucht nur einmal zu passieren.
                                    //Immer wenn der Frm gemoved wird muss
                                    //das Flag zurueckgesetzt werden.
    BOOL    bMustFit    = FALSE;    //Wenn einmal die Notbremse gezogen wurde,
                                    //werden keine anderen Prepares mehr
                                    //abgesetzt.
    BOOL    bFitPromise = FALSE;    //Wenn ein Absatz nicht passte, mit WouldFit
                                    //aber verspricht, dass er sich passend
                                    //einstellt wird dieses Flag gesetzt.
                                    //Wenn er dann sein Versprechen nicht haelt,
                                    //kann kontrolliert verfahren werden.
    BOOL bMoveable;
    const BOOL bFly = IsInFly();
    const BOOL bTab = IsInTab();
    const BOOL bFtn = IsInFtn();
    const BOOL bSct = IsInSct();
    Point aOldFrmPos;               //Damit bei Turnarounds jew. mit der
    Point aOldPrtPos;               //letzten Pos verglichen und geprueft
                                    //werden kann, ob ein Prepare sinnvoll ist.

    SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    const BOOL bKeep = IsKeep( rAttrs );

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
                    ((SwTxtFrm*)pFtn->GetRef())->GetFtnLine( pFtn->GetAttr(),
                                                   pFtn->IsBackMoveLocked() ) );
        }
    }

    //Wenn ein Follow neben seinem Master steht und nicht passt, kann er
    //gleich verschoben werden.
    if( lcl_Prev( this ) && ((SwTxtFrm*)this)->IsFollow() && IsMoveable() )
    {
        bMovedFwd = TRUE;
        MoveFwd( bMakePage, FALSE );
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
                bMovedFwd = TRUE;
                MoveFwd( bMakePage, FALSE );
            }
        }
    }

    SWRECTFN( this )

    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        if ( TRUE == (bMoveable = IsMoveable()) )
        {
            SwFrm *pPre = GetIndPrev();
            if ( CheckMoveFwd( bMakePage, bKeep, bMovedBwd ) )
            {
                SWREFRESHFN( this )
                bMovedFwd = TRUE;
                if ( bMovedBwd )
                {
                    //Beim zurueckfliessen wurde der Upper angeregt sich
                    //vollstaendig zu Painten, dass koennen wir uns jetzt
                    //nach dem hin und her fliessen sparen.
                    GetUpper()->ResetCompletePaint();
                    //Der Vorgaenger wurde Invalidiert, das ist jetzt auch obsolete.
                    ASSERT( pPre, "missing old Prev" );
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
            (Frm().*fnRect->fnSetWidth)( (GetUpper()->
                                         Prt().*fnRect->fnGetWidth)() );
        if ( !bValidPrtArea )
        {
            const long nOldW = (Prt().*fnRect->fnGetWidth)();
            MakePrtArea( rAttrs );
            if ( nOldW != (Prt().*fnRect->fnGetWidth)() )
                Prepare( PREP_FIXSIZE_CHG );
        }

        if ( aOldFrmPos != (Frm().*fnRect->fnGetPos)() )
            CalcFlys( TRUE );
        //Damit die Witwen- und Waisen-Regelung eine Change bekommt muss der
        //CntntFrm benachrichtigt werden.
        //Kriterium:
        //- Er muss Moveable sein (sonst mach das Spalten keinen Sinn.)
        //- Er muss mit der Unterkante der PrtArea des Upper ueberlappen.
        if ( !bMustFit )
        {
            BOOL bWidow = TRUE;
            const SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
            if ( bMoveable && !bFormatted && ( GetFollow() ||
                 ( (Frm().*fnRect->fnOverStep)( nDeadLine ) ) ) )
            {
                Prepare( PREP_WIDOWS_ORPHANS, 0, FALSE );
                bValidSize = bWidow = FALSE;
            }
            if( (Frm().*fnRect->fnGetPos)() != aOldFrmPos ||
                (Prt().*fnRect->fnGetPos)() != aOldPrtPos )
            {
                // In diesem Prepare erfolgt ggf. ein _InvalidateSize().
                // bValidSize wird FALSE und das Format() wird gerufen.
                Prepare( PREP_POS_CHGD, (const void*)&bFormatted, FALSE );
                if ( bWidow && GetFollow() )
                {   Prepare( PREP_WIDOWS_ORPHANS, 0, FALSE );
                    bValidSize = FALSE;
                }
            }
        }
        if ( !bValidSize )
        {
            bValidSize = bFormatted = TRUE;
            ++nFormatCount;
            if( nFormatCount > STOP_FLY_FORMAT )
                SetFlyLock( TRUE );
            Format();
        }

        // FME 16.07.2003 #i16930# - removed this code because it did not work

        // OD 04.04.2003 #108446# - react on the situation detected in the text
        // formatting - see <SwTxtFrm::FormatAdjust(..)>:
        // text frame has to move forward, because its text formatting stopped,
        // created a follow and detected, that it contains no content.
/*        if ( IsTxtFrm() && bValidPos && bValidSize && bValidPrtArea &&
             (Frm().*fnRect->fnGetHeight)() == 0 &&
             HasFollow()
           )
        {
            SwFrm* pOldUpper = GetUpper();
            MoveFwd( TRUE, FALSE );
            if ( GetUpper() != pOldUpper )
            {
                bMovedFwd = TRUE;
                SWREFRESHFN( this )
                continue;
            }
        } */

        //Wenn ich der erste einer Kette bin koennte ich mal sehen ob
        //ich zurueckfliessen kann (wenn ich mich ueberhaupt bewegen soll).
        //Damit es keine Oszillation gibt, darf ich nicht gerade vorwaerts
        //geflossen sein.
        BOOL bDummy;
        if ( !lcl_Prev( this ) && !bMovedFwd && (bMoveable || (bFly && !bTab)) &&
             (!bFtn || !GetUpper()->FindFtnFrm()->GetPrev()) && MoveBwd( bDummy ))
        {
            SWREFRESHFN( this )
            bMovedBwd = TRUE;
            bFormatted = FALSE;
            if ( bKeep && bMoveable )
            {
                if( CheckMoveFwd( bMakePage, FALSE, bMovedBwd ) )
                {
                    bMovedFwd = TRUE;
                    bMoveable = IsMoveable();
                    SWREFRESHFN( this )
                }
                Point aOldPos = (Frm().*fnRect->fnGetPos)();
                MakePos();
                if( aOldPos != (Frm().*fnRect->fnGetPos)() )
                {
                    CalcFlys( TRUE );
                    Prepare( PREP_POS_CHGD, (const void*)&bFormatted, FALSE );
                    if ( !bValidSize )
                    {
                        (Frm().*fnRect->fnSetWidth)( (GetUpper()->
                                                Prt().*fnRect->fnGetWidth)() );
                        if ( !bValidPrtArea )
                        {
                            const long nOldW = (Prt().*fnRect->fnGetWidth)();
                            MakePrtArea( rAttrs );
                            if( nOldW != (Prt().*fnRect->fnGetWidth)() )
                                Prepare( PREP_FIXSIZE_CHG, 0, FALSE );
                        }
                        if( GetFollow() )
                            Prepare( PREP_WIDOWS_ORPHANS, 0, FALSE );
                        bValidSize = bFormatted = TRUE;
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
                                bValidPos = FALSE;
                        }
                        else
                            bValidPos = FALSE;
                    }
                }
            }
        }

        //Der TxtFrm Validiert sich bei Fussnoten ggf. selbst, dass kann leicht
        //dazu fuehren, dass seine Position obwohl unrichtig valide ist.
        if ( bValidPos )
        {
            if ( bFtn )
            {
                bValidPos = FALSE;
                MakePos();
                aOldFrmPos = (Frm().*fnRect->fnGetPos)();
                aOldPrtPos = (Prt().*fnRect->fnGetPos)();
            }
        }

        //Wieder ein Wert ungueltig? - dann nochmal das ganze...
        if ( !bValidPos || !bValidSize || !bValidPrtArea )
            continue;

        //Fertig?
        // Achtung, wg. Hoehe==0, ist es besser statt Bottom() Top()+Height() zu nehmen
        // (kommt bei Undersized TxtFrms an der Unterkante eines spaltigen Bereichs vor)
        if( (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)() )
            >= 0 )
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
                    const FASTBOOL bMoveFwdInvalid = 0 != GetIndNext();
                    const FASTBOOL bNxtNew =
                        ( 0 == (pNxt->Prt().*fnRect->fnGetHeight)() ) &&
                        (!pNxt->IsTxtFrm() ||!((SwTxtFrm*)pNxt)->IsHiddenNow());

                    pNxt->Calc();

                    if ( !bMovedBwd &&
                         ((bMoveFwdInvalid && !GetIndNext()) ||
                          bNxtNew) )
                    {
                        if( bMovedFwd )
                            pNotify->SetInvaKeep();
                        bMovedFwd = FALSE;
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
                long nReal = GetUpper()->Grow( nDiff PHEIGHT );
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
        BOOL bMoveOrFit = FALSE;
        BOOL bDontMoveMe = !GetIndPrev();
        if( bDontMoveMe && IsInSct() )
        {
            SwFtnBossFrm* pBoss = FindFtnBossFrm();
            bDontMoveMe = !pBoss->IsInSct() ||
                          ( !pBoss->Lower()->GetNext() && !pBoss->GetPrev() );
        }

        if ( bDontMoveMe && (Frm().*fnRect->fnGetHeight)() >
                            (GetUpper()->Prt().*fnRect->fnGetHeight)() )
        {
            if ( !bFitPromise )
            {
                SwTwips nTmp = (GetUpper()->Prt().*fnRect->fnGetHeight)() -
                               (Prt().*fnRect->fnGetTop)();
                BOOL bSplit = !GetIndPrev();
                if ( nTmp > 0 && WouldFit( nTmp, bSplit, sal_False ) )
                {
                    Prepare( PREP_WIDOWS_ORPHANS, 0, FALSE );
                    bValidSize = FALSE;
                    bFitPromise = TRUE;
                    continue;
                }
                /* -----------------19.02.99 12:58-------------------
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
                    bMoveOrFit = TRUE;
            }
#ifndef PRODUCT
            else
            {
                ASSERT( FALSE, "+TxtFrm hat WouldFit-Versprechen nicht eingehalten." );
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
            bMovedFwd = FALSE;

        const sal_Bool bCheckForGrownBody = pOldUp->IsBodyFrm();
        const long nOldBodyHeight = (pOldUp->Frm().*fnRect->fnGetHeight)();

        if ( !bMovedFwd && !MoveFwd( bMakePage, FALSE ) )
            bMakePage = FALSE;
        SWREFRESHFN( this )

        // If MoveFwd moves the paragraph to the next page, a following
        // paragraph, which contains footnotes can can cause the old upper
        // frame to grow. In this case we explicitely allow a new check
        // for MoveBwd. Robust: We also check the bMovedBwd flag again.
        // If pOldUp was a footnote frame, it has been deleted inside MoveFwd.
        // Therefore we only check for growing body frames.
        if ( bCheckForGrownBody && ! bMovedBwd && pOldUp != GetUpper() &&
             (pOldUp->Frm().*fnRect->fnGetHeight)() > nOldBodyHeight )
            bMovedFwd = FALSE;
        else
            bMovedFwd = TRUE;

        bFormatted = FALSE;
        if ( bMoveOrFit && GetUpper() == pOldUp )
        {
            Prepare( PREP_MUST_FIT, 0, FALSE );
            bValidSize = FALSE;
            bMustFit = TRUE;
            continue;
        }
        if ( bMovedBwd && GetUpper() )
        {   //Unuetz gewordene Invalidierungen zuruecknehmen.
            GetUpper()->ResetCompletePaint();
            if( pPre && !pPre->IsSctFrm() )
                ::ValidateSz( pPre );
        }

        if ( bValidPos && bValidSize && bValidPrtArea && GetDrawObjs() &&
             Prt().SSize() != pNotify->Prt().SSize() )
        {
            //Wenn sich meine PrtArea in der Groesse verandert hat, so ist die
            //automatische Ausrichtung der Flys zum Teufel. Diese muss
            //Waehrend der Fahrt korrigiert werden, weil sie mich ggf. wiederum
            //invalidiert.
            SwDrawObjs &rObjs = *GetDrawObjs();
            for ( USHORT i = 0; i < rObjs.Count(); ++i )
            {
                SdrObject *pO = rObjs[i];
                if ( pO->IsWriterFlyFrame() )
                    ((SwVirtFlyDrawObj*)pO)->GetFlyFrm()->InvalidatePos();
            }
        }

    } //while ( !bValidPos || !bValidSize || !bValidPrtArea )

    if ( pSaveFtn )
        delete pSaveFtn;

    UnlockJoin();
    if ( bMovedFwd || bMovedBwd )
        pNotify->SetInvaKeep();
    delete pNotify;
    SetFlyLock( FALSE );
}

/*************************************************************************
|*
|*  SwCntntFrm::_WouldFit()
|*
|*  Ersterstellung      MA 28. Feb. 95
|*  Letzte Aenderung    AMA 15. Feb. 99
|*
|*************************************************************************/




void MakeNxt( SwFrm *pFrm, SwFrm *pNxt )
{
    //fix(25455): Validieren, sonst kommt es zu einer Rekursion.
    //Der erste Versuch, der Abbruch mit pFrm = 0 wenn !Valid,
    //fuehrt leider zu dem Problem, dass das Keep dann u.U. nicht mehr
    //korrekt beachtet wird (27417)
    const BOOL bOldPos = pFrm->GetValidPosFlag();
    const BOOL bOldSz  = pFrm->GetValidSizeFlag();
    const BOOL bOldPrt = pFrm->GetValidPrtAreaFlag();
    pFrm->bValidPos = pFrm->bValidPrtArea = pFrm->bValidSize = TRUE;

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

BOOL lcl_IsNextFtnBoss( const SwFrm *pFrm, const SwFrm* pNxt )
{
    ASSERT( pFrm && pNxt, "lcl_IsNextFtnBoss: No Frames?" );
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

BOOL SwCntntFrm::_WouldFit( SwTwips nSpace, SwLayoutFrm *pNewUpper, BOOL bTstMove )
{
    //Damit die Fussnote sich ihren Platz sorgsam waehlt, muss
    //sie in jedem Fall gemoved werden, wenn zwischen dem
    //neuen Upper und ihrer aktuellen Seite/Spalte mindestens eine
    //Seite/Spalte liegt.
    SwFtnFrm* pFtnFrm = 0;
    if ( IsInFtn() )
    {
        if( !lcl_IsNextFtnBoss( pNewUpper, this ) )
            return TRUE;
        pFtnFrm = FindFtnFrm();
    }

    BOOL bRet;
    BOOL bSplit = !pNewUpper->Lower();
    SwCntntFrm *pFrm = this;
    const SwFrm *pPrev = pNewUpper->Lower();
    if( pPrev && pPrev->IsFtnFrm() )
        pPrev = ((SwFtnFrm*)pPrev)->Lower();
    while ( pPrev && pPrev->GetNext() )
        pPrev = pPrev->GetNext();
    do
    {
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
                bTstMove = TRUE;
                bRet = ((SwTxtFrm*)pFrm)->TestFormat( pPrev, nSpace, bSplit );
            }
            else
                bRet = pFrm->WouldFit( nSpace, bSplit, sal_False );
            pTmpFrm->Remove();
            pTmpFrm->InsertBefore( pUp, pOldNext );
        }
        else
            bRet = pFrm->WouldFit( nSpace, bSplit, sal_False );

        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
        const SwBorderAttrs &rAttrs = *aAccess.Get();

        //Bitter aber wahr: Der Abstand muss auch noch mit einkalkuliert werden.
        //Bei TestFormatierung ist dies bereits geschehen.
        if ( bRet && !bTstMove )
        {
            SwTwips nUpper;
            if ( pPrev )
            {
                nUpper = CalcUpperSpace( NULL, pPrev );

                // in balanced columned section frames we do not want the
                // common border
                sal_Bool bCommonBorder = sal_True;
                if ( pFrm->IsInSct() && pFrm->GetUpper()->IsColBodyFrm() )
                {
                    const SwSectionFrm* pSct = pFrm->FindSctFrm();
                    bCommonBorder = pSct->GetFmt()->GetBalancedColumns().GetValue();
                }
                nUpper += bCommonBorder ?
                          rAttrs.GetBottomLine( pFrm ) :
                          rAttrs.CalcBottomLine();
            }
            else
            {
                if( pFrm->IsVertical() )
                    nUpper = pFrm->Frm().Width() - pFrm->Prt().Width();
                else
                    nUpper = pFrm->Frm().Height() - pFrm->Prt().Height();
            }
            nSpace -= nUpper;
            if ( nSpace < 0 )
                bRet = FALSE;
        }

        if ( bRet && !bSplit && pFrm->IsKeep( rAttrs ) )
        {
            if( bTstMove )
            {
                while( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->HasFollow() )
                {
                    pFrm = ((SwTxtFrm*)pFrm)->GetFollow();
                }
                // OD 11.04.2003 #108824# - If last follow frame of <this> text
                // frame isn't valid, a formatting of the next content frame
                // doesn't makes sense. Thus, return TRUE.
                if ( IsAnFollow( pFrm ) && !pFrm->IsValid() )
                {
                    ASSERT( false, "Only a warning for task 108824:/n<SwCntntFrm::_WouldFit(..) - follow not valid!" );
                    return TRUE;
                }
            }
            SwFrm *pNxt;
            if( 0 != (pNxt = pFrm->FindNext()) && pNxt->IsCntntFrm() &&
                ( !pFtnFrm || ( pNxt->IsInFtn() &&
                  pNxt->FindFtnFrm()->GetAttr() == pFtnFrm->GetAttr() ) ) )
            {
                // ProbeFormatierung vertraegt keine absatz- oder gar zeichengebundene Objekte
                if( bTstMove && pNxt->GetDrawObjs() )
                    return TRUE;

                if ( !pNxt->IsValid() )
                    MakeNxt( pFrm, pNxt );

                //Kleiner Trick: Wenn der naechste einen Vorgaenger hat, so hat
                //er den Absatzabstand bereits berechnet. Er braucht dann nicht
                //teuer kalkuliert werden.
                if( lcl_NotHiddenPrev( pNxt ) )
                    pPrev = 0;
                else
                {
                    if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsHiddenNow() )
                        pPrev = lcl_NotHiddenPrev( pFrm );
                    else
                        pPrev = pFrm;
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
