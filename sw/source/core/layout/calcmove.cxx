/*************************************************************************
 *
 *  $RCSfile: calcmove.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
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
        if( Abs(pNewUpper->Prt().Width() - GetUpper()->Prt().Width()) > 1 )
            nMoveAnyway = 2; // Damit kommt nur noch ein _WouldFit mit Umhaengen in Frage
        if ( (nMoveAnyway |= BwdMoveNecessary( pOldPage, Frm() )) < 3 )
        {
            SwTwips nSpace = 0;
            SwRect aRect( pNewUpper->Prt() );
            aRect.Pos() += pNewUpper->Frm().Pos();
            const SwFrm *pPrevFrm = pNewUpper->Lower();
            while ( pPrevFrm )
            {
                if( !pPrevFrm->GetNext() && !pPrevFrm->IsValid() )
                    return TRUE;
                aRect.Top( pPrevFrm->Frm().Bottom() );
                pPrevFrm = pPrevFrm->GetNext();
            }

            nMoveAnyway |= BwdMoveNecessary( pNewPage, aRect);
            if ( nMoveAnyway < 3 )
            {
                //Zur Verfuegung stehenden Raum berechenen.
                nSpace = aRect.Height();
                if ( IsInFtn() || GetAttrSet()->GetDoc()->IsBrowseMode() ||
                     ( pNewUpper->IsInSct() && ( pNewUpper->IsSctFrm() ||
                       ( pNewUpper->IsColBodyFrm() &&
                         !pNewUpper->GetUpper()->GetPrev() &&
                         !pNewUpper->GetUpper()->GetNext() ) ) ) )
                    nSpace += pNewUpper->Grow( LONG_MAX, pHeight, TRUE );
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
                    !pNewUpper->Prt().Width() &&
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
        if( !GetUpper()->IsSctFrm() )
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

        if( !GetUpper()->IsSctFrm() )
            GetUpper()->Calc();

        ASSERT( GetUpper(), "Layoutgeruest wackelig (Upper wech III)." );

        if ( bTab && !bOldTabLock )
            ::PrepareUnlock( (SwTabFrm*)this );
    }
    MakeAll();
}

void SwFrm::OptPrepareMake()
{
    if ( GetUpper() )
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
        if ( pPrv && (!pPrv->IsCntntFrm() ||
              (((SwCntntFrm*)pPrv)->GetFollow() != this)) )
        {
            if( !StackHack::IsLocked() && ( !IsInSct() || IsSctFrm() ) &&
                !pPrv->IsSctFrm() && !pPrv->GetAttrSet()->GetKeep().GetValue() )
                pPrv->Calc();   //hierbei kann der Prev verschwinden!
            else if ( pPrv->Frm().Top() == 0 )
                bUseUpper = TRUE;
        }

        pPrv = lcl_Prev( this, FALSE );
        if ( !bUseUpper && pPrv )
        {   aFrm.Pos( pPrv->Frm().Pos() );
            aFrm.Pos().*pVARPOS += pPrv->Frm().SSize().*pVARSIZE;
        }
        else if ( GetUpper() )
        {
            // In einem spaltigen Rahmen rufen wir lieber kein Calc "von unten"
            if( !GetUpper()->IsSctFrm() )
                GetUpper()->Calc(); // Jetzt koennte ein Prev dazugekommen sein...
            pPrv = lcl_Prev( this, FALSE );
            if ( !bUseUpper && pPrv )
            {   aFrm.Pos( pPrv->Frm().Pos() );
                aFrm.Pos().*pVARPOS += pPrv->Frm().SSize().*pVARSIZE;
            }
            else
            {
                aFrm.Pos( GetUpper()->Frm().Pos() );
                aFrm.Pos() += GetUpper()->Prt().Pos();
            }
        }
        else
            aFrm.Pos().X() = aFrm.Pos().Y() = 0;
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
                            nTmp += pFrm->Frm().Height() - pFrm->Prt().Height();
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
    {   if ( bVarHeight )
        {   ASSERT( GetUpper()->Prt().Width() >= aFrm.Width(),
                    "Rootsize" );
        }
        else
        {   ASSERT( GetUpper()->Prt().Height() >= aFrm.Height(),
                    "Rootsize" );
        }
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
    const SzPtr pFix = pFIXSIZE;

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
                aFrm.SSize().*pFix = GetUpper()->Prt().SSize().*pFix;
                bValidPrtArea = FALSE;
            }
            else
            {   //nicht ueber die auessere Kante des Upper hinausragen.
                const SwTwips nDeadLine = GetUpper()->Frm().Pos().*pVARPOS +
                    (bVarHeight ?
                        GetUpper()->Prt().Top() + GetUpper()->Prt().Height() :
                        GetUpper()->Prt().Left() + GetUpper()->Prt().Width());
                const SwTwips nBot = bVarHeight ?
                    Frm().Top() + Frm().Height() : Frm().Left() + Frm().Width();
                if ( nBot > nDeadLine )
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

        const FASTBOOL bTxtFrm = IsTxtFrm();
        SwTwips nUpper = 0;
        if ( bTxtFrm && ((SwTxtFrm*)this)->IsHiddenNow() )
        {
            if ( Prt().Height() )
                ((SwTxtFrm*)this)->HideHidden();
            Prt().Pos().X() = Prt().Pos().Y() = 0;
            Prt().Width( Frm().Width() );
            Prt().Height( 0 );
            nUpper = -Frm().Height();
        }
        else
        {
            //Vereinfachung: CntntFrms sind immer in der Hoehe Variabel!

            //An der FixSize gibt der umgebende Frame die Groesse vor, die
            //Raender werden einfach abgezogen.
            const long nLeft = rAttrs.CalcLeft( this );
            Prt().Width( Frm().Width() - (nLeft + rAttrs.CalcRight()) );
            Prt().Pos().X() = nLeft;

            ViewShell *pSh = GetShell();
            if ( pSh && pSh->VisArea().Width() &&
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
                long nWidth = pSh->VisArea().Width() - 2 * aBorder.Width();
                nWidth -= Prt().Left();
                nWidth -= rAttrs.CalcRightLine();
                nWidth = Max( nMinWidth, nWidth );
                Prt().Width( Min( nWidth, Prt().Width() ) );
            }

            if ( Prt().Width() <= MINLAY )
            {
                //Die PrtArea sollte schon wenigstens MINLAY breit sein, passend
                //zu den Minimalwerten des UI
                Prt().Width( Min( long(MINLAY), Frm().Width() ) );
                if ( (Prt().Pos().X() + Prt().Width()) > Frm().Width() )
                    Prt().Pos().X() = Frm().Width() - Prt().Width();
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
            Prt().Pos().Y() = nUpper;

            nUpper += rAttrs.GetBottomLine( this );
            nUpper -= Frm().Height() - Prt().Height();
        }
        //Wenn Unterschiede zwischen Alter und neuer Groesse,
        //Grow() oder Shrink() rufen
        if ( nUpper )
        {
            if ( nUpper > 0 )
                GrowFrm( nUpper, pHeight );
            else
                ShrinkFrm( -nUpper, pHeight );
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

    StackHack aHack;

    if ( ((SwTxtFrm*)this)->IsLocked() )
    {
        ASSERT( FALSE, "Format fuer gelockten TxtFrm." );
        return;
    }

    LockJoin();
    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

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
    long  nKeepBottom = Frm().Bottom(); //Um beim Keep den naechsten sinnvoll
                                        //anstossen zu koennen.

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

    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        if ( TRUE == (bMoveable = IsMoveable()) )
        {
            SwFrm *pPre = GetIndPrev();
            if ( CheckMoveFwd( bMakePage, bKeep, bMovedBwd ) )
            {
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


        aOldFrmPos = Frm().Pos();
        aOldPrtPos = Prt().Pos();

        if ( !bValidPos )
            MakePos();

        //FixSize einstellen, die VarSize wird von Format() justiert.
        if ( !bValidSize )
            Frm().Width( GetUpper()->Prt().Width() );

        if ( !bValidPrtArea )
        {
            const long nWidth = Prt().Width();
            MakePrtArea( rAttrs );
            if ( nWidth != Prt().Width() )
                Prepare( PREP_FIXSIZE_CHG );
        }

        if ( aOldFrmPos != Frm().Pos() )//Erst nach Berechnung von Breite und PrtArea,
            CalcFlys( TRUE );           //sonst droht mehrfache Berechnung!

        //Damit die Witwen- und Waisen-Regelung eine Change bekommt muss der
        //CntntFrm benachrichtigt werden.
        //Kriterium:
        //- Er muss Moveable sein (sonst mach das Spalten keinen Sinn.)
        //- Er muss mit der Unterkante der PrtArea des Upper ueberlappen.
        if ( !bMustFit )
        {
            BOOL bWidow = TRUE;
            const SwTwips nDeadLine = GetUpper()->Prt().Bottom() +
                                      GetUpper()->Frm().Top();
            if ( bMoveable && !bFormatted &&
                 ((Frm().Top() < nDeadLine && Frm().Bottom() > nDeadLine) ||
                  GetFollow()) )
            {
                Prepare( PREP_WIDOWS_ORPHANS, 0, FALSE );
                bValidSize = bWidow = FALSE;
            }
            if ( (Frm().Pos() != aOldFrmPos) || (Prt().Pos() != aOldPrtPos) )
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
        {   bValidSize = bFormatted = TRUE;
            Format();
        }
        //Wenn ich der erste einer Kette bin koennte ich mal sehen ob
        //ich zurueckfliessen kann (wenn ich mich ueberhaupt bewegen soll).
        //Damit es keine Oszillation gibt, darf ich nicht gerade vorwaerts
        //geflossen sein.
        BOOL bDummy;
        if ( !lcl_Prev( this ) && !bMovedFwd && (bMoveable || (bFly && !bTab)) &&
             (!bFtn || !GetUpper()->FindFtnFrm()->GetPrev()) && MoveBwd( bDummy ))
        {
            bMovedBwd = TRUE;
            bFormatted = FALSE;
            if ( bKeep )
            {
                if( CheckMoveFwd( bMakePage, FALSE, bMovedBwd ) )
                {
                    bMovedFwd = TRUE;
                    bMoveable = IsMoveable();
                }
                const Point aOldFrmPos( Frm().Pos() );
                MakePos();
                if ( aOldFrmPos != Frm().Pos() )
                {
                    CalcFlys( TRUE );
                    Prepare( PREP_POS_CHGD, (const void*)&bFormatted, FALSE );
                    if ( !bValidSize )
                    {
                        Frm().Width( GetUpper()->Prt().Width() );
                        if ( !bValidPrtArea )
                        {
                            const long nWidth = Prt().Width();
                            MakePrtArea( rAttrs );
                            if ( nWidth != Prt().Width() )
                                Prepare( PREP_FIXSIZE_CHG, 0, FALSE );
                        }
                        if( GetFollow() )
                            Prepare( PREP_WIDOWS_ORPHANS, 0, FALSE );
                        bValidSize = bFormatted = TRUE;
                        Format();
                    }
                }

                SwFrm *pNxt = FindNext();
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
            {   bValidPos = FALSE;
                MakePos();
                aOldFrmPos = Frm().Pos();
                aOldPrtPos = Prt().Pos();
            }
        }

        //Wieder ein Wert ungueltig? - dann nochmal das ganze...
        if ( !bValidPos || !bValidSize || !bValidPrtArea )
            continue;

        //Fertig?
        // Achtung, wg. Hoehe==0, ist es besser statt Bottom() Top()+Height() zu nehmen
        // (kommt bei Undersized TxFrms an der Unterkante eines spaltigen Bereichs vor)
        if ( GetUpper()->Prt().Top()+GetUpper()->Prt().Height()+GetUpper()->Frm().Top() >=
             Frm().Top()+Frm().Height() )
        {
            if ( bKeep )
            {
                //Wir sorgen dafuer, dass der Nachfolger gleich mit formatiert
                //wird. Dadurch halten wir das Heft in der Hand, bis wirklich
                //(fast) alles stabil ist. So vermeiden wir Endlosschleifen,
                //die durch staendig wiederholte Versuche entstehen.
                //Das bMoveFwdInvalid ist fuer #38407# notwendig. War urspruenglich
                //in flowfrm.cxx rev 1.38 behoben, das unterbrach aber obiges
                //Schema und spielte lieber Tuerme von Hanoi (#43669#).
                SwFrm *pNxt = FindNext();
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
                    const FASTBOOL bNxtNew = !pNxt->Prt().Height() &&
                       (!pNxt->IsTxtFrm() || !((SwTxtFrm*)pNxt)->IsHiddenNow());
                    nKeepBottom = Frm().Bottom();
                    pNxt->Calc();
                    if ( !bMovedBwd &&
                         ((bMoveFwdInvalid && !GetIndNext()) ||
                          bNxtNew) )
                        bMovedFwd = FALSE;
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
                long nDiff = Frm().Top()+Frm().Height() -GetUpper()->Prt().Top()
                        -GetUpper()->Prt().Height()-GetUpper()->Frm().Top();
                long nReal = Grow( nDiff, pHeight );
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

        if ( bDontMoveMe && Frm().Height() > GetUpper()->Prt().Height() )
        {
#ifdef USED
            // Wozu mag dies gut gewesen sein?
            // Wurde am 24. Jan 94 mit dem Kommentar "Diverse Ftn Bugs." eingecheckt.

            //Koennte es sein, dass der Upper nicht so recht angepasst wurde?
            if ( bFtn )
            {
                GetUpper()->_InvalidateSize();
                GetUpper()->Calc();
                if ( !bValidPos )
                {
                    MakePos();
                    aOldFrmPos = Frm().Pos();
                    aOldPrtPos = Prt().Pos();
                }
            }
#endif
            if ( !bFitPromise ) //Wer einmal luegt...
            {
                // In WouldFit kann wird der obere Absatzabstand _nicht_
                // beruecksichtigt werden, da er nicht bei jedem Aufruf
                // von WouldFit auf die richtige Umgebung bezieht. An dieser
                // Stelle allerdings stimmt er, deshalb ziehen wir ihn schon
                // mal von der zur Verfuegung stehenden Hoehe ab.
                SwTwips nTmp = GetUpper()->Prt().Height() - Prt().Top();
                BOOL bSplit = !GetIndPrev();
                if ( nTmp > 0 && WouldFit( nTmp, bSplit ) )
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

        if ( !bMovedFwd && !MoveFwd( bMakePage, FALSE ) )
            bMakePage = FALSE;
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
            pNxt->Frm().Width( pNxt->GetUpper()->Prt().Width() );
        ((SwCntntFrm*)pNxt)->MakePrtArea( rAttrs );
        pNxt->Format( &rAttrs );
    }
    else
    {
        SwLayNotify aNotify( (SwLayoutFrm*)pNxt );
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pNxt );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( !pNxt->GetValidSizeFlag() )
            pNxt->Frm().Width( pNxt->GetUpper()->Prt().Width() );
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
            if( pFrm->IsTxtFrm() && ( bTstMove ||
                ((SwTxtFrm*)pFrm)->HasFollow() ||
                ( !((SwTxtFrm*)pFrm)->HasPara() &&
                    !((SwTxtFrm*)pFrm)->IsEmpty() ) ) )
            {
                bTstMove = TRUE;
                bRet = ((SwTxtFrm*)pFrm)->TestFormat( pPrev, nSpace, bSplit );
            }
            else
                bRet = pFrm->WouldFit( nSpace, bSplit );
            pTmpFrm->Remove();
            pTmpFrm->InsertBefore( pUp, pOldNext );
        }
        else
            bRet = pFrm->WouldFit( nSpace, bSplit );

        //Bitter aber wahr: Der Abstand muss auch noch mit einkalkuliert werden.
        //Bei TestFormatierung ist dies bereits geschehen.
        if ( bRet && !bTstMove )
        {
            SwTwips nUpper;
            if ( pPrev )
            {
                nUpper = CalcUpperSpace( NULL, pPrev );
                SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                nUpper += rAttrs.GetBottomLine( pFrm );
            }
            else
                nUpper = pFrm->Frm().Height() - pFrm->Prt().Height();
            nSpace -= nUpper;
            if ( nSpace < 0 )
                bRet = FALSE;
        }

        if ( bRet && !bSplit && pFrm->GetAttrSet()->GetKeep().GetValue() )
        {
            if( bTstMove )
                while( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->HasFollow() )
                    pFrm = ((SwTxtFrm*)pFrm)->GetFollow();
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




