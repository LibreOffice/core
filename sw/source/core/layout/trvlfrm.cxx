/*************************************************************************
 *
 *  $RCSfile: trvlfrm.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-15 10:16:52 $
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


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SV_SETTINGS_HXX //autogen
#include <vcl/settings.hxx>
#endif
#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif

#ifndef _PAGEFRM_HXX //autogen
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX //autogen
#include <rootfrm.hxx>
#endif
#ifndef _CNTFRM_HXX //autogen
#include <cntfrm.hxx>
#endif
#ifndef _FTNFRM_HXX //autogen
#include <ftnfrm.hxx>
#endif
#ifndef _FLYFRM_HXX //autogen
#include <flyfrm.hxx>
#endif
#ifndef _TABFRM_HXX //autogen
#include <tabfrm.hxx>
#endif
#ifndef _CELLFRM_HXX //autogen
#include <cellfrm.hxx>
#endif
#ifndef _TXTFRM_HXX //autogen
#include <txtfrm.hxx>
#endif
#ifndef _VIEWSH_HXX //autogen
#include <viewsh.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _VISCRS_HXX //autogen
#include <viscrs.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _SWTABLE_HXX //autogen
#include <swtable.hxx>
#endif
#ifndef _DFLYOBJ_HXX //autogen
#include <dflyobj.hxx>
#endif
#ifndef _CRSTATE_HXX //autogen
#include <crstate.hxx>
#endif
#ifndef _FRMTOOL_HXX //autogen
#include <frmtool.hxx>
#endif
#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif


//Fuer SwFlyFrm::GetCrsrOfst
class SwCrsrOszControl
{
public:
    // damit schon der Compiler die Klasse initialisieren kann, keinen
    // DTOR und member als publics:
    const SwFlyFrm *pEntry;
    const SwFlyFrm *pStk1;
    const SwFlyFrm *pStk2;

//public:
//    SwCrsrOszControl() : pStk1( 0 ), pStk2( 0 ) {}; // ; <- ????

    BOOL ChkOsz( const SwFlyFrm *pFly )
        {
            BOOL bRet = TRUE;
            if ( pFly != pStk1 && pFly != pStk2 )
            {
                pStk1 = pStk2;
                pStk2 = pFly;
                bRet  = FALSE;
            }
            return bRet;
        }
    void Entry( const SwFlyFrm *pFly )
        {
            if ( !pEntry )
                pEntry = pStk1 = pFly;
        }
    void Exit( const SwFlyFrm *pFly )
        {
            if ( pFly == pEntry )
                pEntry = pStk1 = pStk2 = 0;
        }
};

static SwCrsrOszControl aOszCtrl = { 0, 0, 0 };

/*************************************************************************
|*
|*  SwLayoutFrm::GetCrsrOfst()
|*
|*  Beschreibung:       Sucht denjenigen CntntFrm, innerhalb dessen
|*                      PrtArea der Point liegt.
|*  Ersterstellung      MA 20. Jul. 92
|*  Letzte Aenderung    MA 23. May. 95
|*
|*************************************************************************/
BOOL SwLayoutFrm::GetCrsrOfst( SwPosition *pPos, Point &rPoint,
                            const SwCrsrMoveState* pCMS ) const
{
    BOOL bRet = FALSE;
    const SwFrm *pFrm = Lower();
    while ( !bRet && pFrm )
    {   pFrm->Calc();
        SwRect aPaintRect( pFrm->PaintArea() );
        if ( aPaintRect.IsInside( rPoint ) &&
            pFrm->GetCrsrOfst( pPos, rPoint, pCMS ) )
            bRet = TRUE;
        else
            pFrm = pFrm->GetNext();
        if ( pCMS && pCMS->bStop )
            return FALSE;
    }
    return bRet;
}

/*************************************************************************
|*
|*  SwPageFrm::GetCrsrOfst()
|*
|*  Beschreibung:       Sucht die Seite, innerhalb der der gesuchte Point
|*                      liegt.
|*  Ersterstellung      MA 20. Jul. 92
|*  Letzte Aenderung    MA 18. Jul. 96
|*
|*************************************************************************/
#pragma optimize("e",off)

BOOL SwPageFrm::GetCrsrOfst( SwPosition *pPos, Point &rPoint,
                            const SwCrsrMoveState* pCMS ) const
{
    BOOL bRet     = FALSE;
    const SwPageFrm *pPage = this;
    const PtPtr pFix = pFIXPOS;
    const PtPtr pVar = pVARPOS;
    Point aStPoint( rPoint );
    Point aPoint;
    while ( !bRet && pPage )
    {
        aPoint = aStPoint;

        //Wenn der Point in der VarRichtung zwischen zwei Seiten liegt
        //erhaelt die Seite die dem Point naeher steht den Zuschlag.
        SwTwips nTmp = bVarHeight ? pPage->Frm().Top() :
                                    pPage->Frm().Left();
        if ( pPage->GetPrev() )
        {
            const SwTwips nPreTmp = bVarHeight ?
                                pPage->GetPrev()->Frm().Bottom() :
                                pPage->GetPrev()->Frm().Right();
            if ( (aPoint.*pVar > nPreTmp) &&
                 (aPoint.*pVar < nTmp)    &&
                 ((aPoint.*pVar - nPreTmp) >= (nTmp - aPoint.*pVar)) )
                aPoint.*pVar = nTmp;
        }
        else if ( aPoint.*pVar < nTmp )
            aPoint.*pVar = nTmp;

        nTmp = bVarHeight ? pPage->Frm().Bottom() : pPage->Frm().Right();
        if ( pPage->GetNext() )
        {
            const SwTwips nNxtTmp = bVarHeight ?
                                pPage->GetNext()->Frm().Top() :
                                pPage->GetNext()->Frm().Left();
            if ( (aPoint.*pVar > nTmp) &&
                 (aPoint.*pVar < nNxtTmp) &&
                 ((nNxtTmp - aPoint.*pVar) >= (aPoint.*pVar - nTmp)) )
                aPoint.*pVar = nTmp;
        }
        else if ( aPoint.*pVar > nTmp )
            aPoint.*pVar = nTmp;

        //Wenn der Punkt in der Fix-Richtung neben der Seite liegt wird er
        //hineingezogen.
        const SwTwips nVarA = pPage->Frm().Pos().*pFix;
        const SwTwips nVarB = bVarHeight ? pPage->Frm().Right() :
                                           pPage->Frm().Bottom();
        if ( nVarA > aPoint.*pFix )
            aPoint.*pFix = nVarA;
        else if ( nVarB < aPoint.*pFix )
            aPoint.*pFix = nVarB;

        //Weitere versuche mit der aktuellen Seite nur dann, wenn sich der
        //Point innerhalb der Seite befindet.
        const BOOL bInside = pPage->Frm().IsInside( aPoint );

        //Koennte ein Freifliegender gemeint sein?
        //Wenn sein Inhalt geschuetzt werden soll, so ist nix mit Crsr
        //hineinsetzen, dadurch sollten alle Aenderungen unmoeglich sein.
        if ( bInside && pPage->GetSortedObjs() )
        {
            SwOrderIter aIter( pPage );
            aIter.Top();
            while ( aIter() )
            {
                SwVirtFlyDrawObj *pObj = (SwVirtFlyDrawObj*)aIter();
                SwFlyFrm *pFly = pObj ? pObj->GetFlyFrm() : 0;
                if ( pFly &&
                     ((pCMS?pCMS->bSetInReadOnly:FALSE) ||
                      !pFly->IsProtected())
                     && pFly->GetCrsrOfst( pPos, aPoint, pCMS ) )
                {
                    bRet = TRUE;
                    break;
                }
                if ( pCMS && pCMS->bStop )
                    return FALSE;
                aIter.Prev();
            }
        }
        if ( !bRet && bInside )
        {
            //Wenn kein Cntnt unterhalb der Seite 'antwortet', so korrigieren
            //wir den StartPoint und fangen nochmal eine Seite vor der
            //aktuellen an. Mit Flys ist es dann allerdings vorbei.
            if ( pPage->SwLayoutFrm::GetCrsrOfst( pPos, aPoint, pCMS ) )
                bRet = TRUE;
            else
            {
                if ( pCMS && (pCMS->bStop || pCMS->bExactOnly) )
                {
                    ((SwCrsrMoveState*)pCMS)->bStop = TRUE;
                    return FALSE;
                }
                const SwCntntFrm *pCnt = pPage->GetCntntPos(
                                    aPoint, FALSE, FALSE, FALSE, pCMS, FALSE );
                if ( pCMS && pCMS->bStop )
                    return FALSE;
                ASSERT( pCnt, "Crsr is gone to a Black hole" );
                if( pCMS && pCMS->pFill && pCnt->IsTxtFrm() )
                    pCnt->GetCrsrOfst( pPos, rPoint, pCMS );
                else
                    pCnt->GetCrsrOfst( pPos, aPoint, pCMS );
                bRet = TRUE;
            }
        }
        pPage = (const SwPageFrm*)pPage->GetNext();
    }
    if ( bRet )
        rPoint = aPoint;
    return bRet;
}

#pragma optimize("",on)

/*************************************************************************
|*
|*  SwRootFrm::GetCrsrOfst()
|*
|*  Beschreibung:       Reicht Primaer den Aufruf an die erste Seite weiter.
|*                      Wenn der 'reingereichte Point veraendert wird,
|*                      so wird FALSE zurueckgegeben.
|*  Ersterstellung      MA 01. Jun. 92
|*  Letzte Aenderung    MA 30. Nov. 94
|*
|*************************************************************************/
BOOL SwRootFrm::GetCrsrOfst( SwPosition *pPos, Point &rPoint,
                            const SwCrsrMoveState* pCMS ) const
{
    sal_Bool bOldAction = IsCallbackActionEnabled();
    ((SwRootFrm*)this)->SetCallbackActionEnabled( FALSE );
    ASSERT( (Lower() && Lower()->IsPageFrm()), "Keinen PageFrm gefunden." );
    if( pCMS && pCMS->pFill )
        ((SwCrsrMoveState*)pCMS)->bFillRet = FALSE;
    Point aOldPoint = rPoint;
    const SwPageFrm *pPage = (SwPageFrm*)Lower();
    pPage->SwPageFrm::GetCrsrOfst( pPos, rPoint, pCMS );
    ((SwRootFrm*)this)->SetCallbackActionEnabled( bOldAction );
    if( pCMS )
    {
        if( pCMS->bStop )
            return FALSE;
        if( pCMS->pFill )
            return pCMS->bFillRet;
    }
    return aOldPoint == rPoint;
}

/*************************************************************************
|*
|*  SwCellFrm::GetCrsrOfst()
|*
|*  Beschreibung        Wenn es sich um eine Cntnt-tragende Cell handelt wird
|*                      der Crsr notfalls mit Gewalt in einen der CntntFrms
|*                      gesetzt.
|*                      In geschuetzte Zellen gibt es hier keinen Eingang.
|*  Ersterstellung      MA 04. Jun. 93
|*  Letzte Aenderung    MA 23. May. 95
|*
|*************************************************************************/
BOOL SwCellFrm::GetCrsrOfst( SwPosition *pPos, Point &rPoint,
                            const SwCrsrMoveState* pCMS ) const
{
    ASSERT( Lower(), "Zelle ohne Inhalt." );

    if ( !(pCMS?pCMS->bSetInReadOnly:FALSE) &&
         GetFmt()->GetProtect().IsCntntProtected() )
        return FALSE;

    if ( pCMS && pCMS->eState == MV_TBLSEL )
    {
        const SwTabFrm *pTab = FindTabFrm();
        if ( pTab->IsFollow() && pTab->GetTable()->IsHeadlineRepeat() &&
             ((SwLayoutFrm*)pTab->Lower())->IsAnLower( this ) )
        {
            ((SwCrsrMoveState*)pCMS)->bStop = TRUE;
            return FALSE;
        }
    }

    if ( Lower()->IsLayoutFrm() )
        return SwLayoutFrm::GetCrsrOfst( pPos, rPoint, pCMS );
    else
    {
        Calc();
        BOOL bRet = FALSE;

        const SwFrm *pFrm = Lower();
        while ( pFrm && !bRet )
        {
            pFrm->Calc();
            if ( pFrm->Frm().IsInside( rPoint ) )
            {
                bRet = pFrm->GetCrsrOfst( pPos, rPoint, pCMS );
                if ( pCMS && pCMS->bStop )
                    return FALSE;
            }
            pFrm = pFrm->GetNext();
        }
        if ( !bRet )
        {
            Point *pPoint = pCMS && pCMS->pFill ? new Point( rPoint ) : NULL;
            const SwCntntFrm *pCnt = GetCntntPos( rPoint, TRUE );
            if( pPoint && pCnt->IsTxtFrm() )
            {
                pCnt->GetCrsrOfst( pPos, *pPoint, pCMS );
                rPoint = *pPoint;
            }
            else
                pCnt->GetCrsrOfst( pPos, rPoint, pCMS );
            delete pPoint;
        }
        return TRUE;
    }
}

/*************************************************************************
|*
|*  SwFlyFrm::GetCrsrOfst()
|*
|*  Ersterstellung      MA 15. Dec. 92
|*  Letzte Aenderung    MA 23. May. 95
|*
|*************************************************************************/
//Problem: Wenn zwei Flys genau gleich gross sind und auf derselben
//Position stehen, so liegt jeder innerhalb des anderen.
//Da jeweils geprueft wird, ob der Point nicht zufaellig innerhalb eines
//anderen Flys liegt, der sich vollstaendig innerhalb des aktuellen befindet
//und ggf. ein rekursiver Aufruf erfolgt wuerde o.g. Situation zu einer
//endlosen Rekursion fuehren.
//Mit der Hilfsklasse SwCrsrOszControl unterbinden wir die Rekursion. Das
//GetCrsrOfst entscheidet sich bei einer Rekursion fuer denjenigen der
//am weitesten oben liegt.

BOOL SwFlyFrm::GetCrsrOfst( SwPosition *pPos, Point &rPoint,
                            const SwCrsrMoveState* pCMS ) const
{
    aOszCtrl.Entry( this );

    //Wenn der Point innerhalb des Fly sitzt wollen wir energisch
    //versuchen den Crsr hineinzusetzen.
    //Wenn der Point allerdings in einem Flys sitzt, der sich vollstaendig
    //innerhalb des aktuellen befindet, so wird fuer diesen das
    //GetCrsrOfst gerufen.
    Calc();
    BOOL bInside = Frm().IsInside( rPoint ) && Lower(),
         bRet = FALSE;

    //Wenn der Frm eine Grafik enthaelt, aber nur Text gewuenscht ist, so
    //nimmt er den Crsr grundsaetzlich nicht an.
    if ( bInside && pCMS && pCMS->eState == MV_SETONLYTEXT &&
         (!Lower() || Lower()->IsNoTxtFrm()) )
        bInside = FALSE;

    const SwPageFrm *pPage = FindPageFrm();
    if ( bInside && pPage && pPage->GetSortedObjs() )
    {
        SwOrderIter aIter( pPage );
        aIter.Top();
        while ( aIter() && !bRet )
        {
            SwVirtFlyDrawObj *pObj = (SwVirtFlyDrawObj*)aIter();
            SwFlyFrm *pFly = pObj ? pObj->GetFlyFrm() : 0;
            if ( pFly && pFly->Frm().IsInside( rPoint ) &&
                 Frm().IsInside( pFly->Frm() ) )
            {
                if ( aOszCtrl.ChkOsz( pFly ) ||
                     TRUE == (bRet = pFly->GetCrsrOfst( pPos, rPoint, pCMS )))
                    break;
                if ( pCMS && pCMS->bStop )
                    return FALSE;
            }
            aIter.Next();
        }
    }

    while ( bInside && !bRet )
    {
        const SwFrm *pFrm = Lower();
        while ( pFrm && !bRet )
        {
            pFrm->Calc();
            if ( pFrm->Frm().IsInside( rPoint ) )
            {
                bRet = pFrm->GetCrsrOfst( pPos, rPoint, pCMS );
                if ( pCMS && pCMS->bStop )
                    return FALSE;
            }
            pFrm = pFrm->GetNext();
        }
        if ( !bRet )
        {
            Point *pPoint = pCMS && pCMS->pFill ? new Point( rPoint ) : NULL;
            const SwCntntFrm *pCnt = GetCntntPos(
                                            rPoint, TRUE, FALSE, FALSE, pCMS );
            if ( pCMS && pCMS->bStop )
                return FALSE;
            if( pPoint && pCnt->IsTxtFrm() )
            {
                pCnt->GetCrsrOfst( pPos, *pPoint, pCMS );
                rPoint = *pPoint;
            }
            else
                pCnt->GetCrsrOfst( pPos, rPoint, pCMS );
            delete pPoint;
            bRet = TRUE;
        }
    }
    aOszCtrl.Exit( this );
    return bRet;
}

/*************************************************************************
|*
|*    Beschreibung      Layoutabhaengiges Cursortravelling
|*    Ersterstellung    MA 23. Jul. 92
|*    Letzte Aenderung  MA 06. Sep. 93
|*
|*************************************************************************/
BOOL SwCntntFrm::LeftMargin(SwPaM *pPam) const
{
    if( pPam->GetNode() != (SwCntntNode*)GetNode() )
        return FALSE;
    ((SwCntntNode*)GetNode())->
        MakeStartIndex((SwIndex *) &pPam->GetPoint()->nContent);
    return TRUE;
}

BOOL SwCntntFrm::RightMargin(SwPaM *pPam, BOOL) const
{
    if( pPam->GetNode() != (SwCntntNode*)GetNode() )
        return FALSE;
    ((SwCntntNode*)GetNode())->
        MakeEndIndex((SwIndex *) &pPam->GetPoint()->nContent);
    return TRUE;
}

const SwCntntFrm *lcl_GetNxtCnt( const SwCntntFrm* pCnt )
{
    return pCnt->GetNextCntntFrm();
}

const SwCntntFrm *lcl_GetPrvCnt( const SwCntntFrm* pCnt )
{
    return pCnt->GetPrevCntntFrm();
}

typedef const SwCntntFrm *(*GetNxtPrvCnt)( const SwCntntFrm* );

//Frame in wiederholter Headline?
FASTBOOL lcl_IsInRepeatedHeadline( const SwFrm *pFrm )
{
    const SwTabFrm *pTab;
    return 0 != (pTab = pFrm->FindTabFrm()) &&
           pTab->IsFollow() &&
           pTab->GetTable()->IsHeadlineRepeat() &&
           ((SwLayoutFrm*)pTab->Lower())->IsAnLower( pFrm );
}


//Ueberspringen geschuetzter Tabellenzellen. Optional auch
//Ueberspringen von wiederholten Headlines.
//MA 26. Jan. 98: Chg auch andere Geschuetzte Bereiche ueberspringen.
const SwCntntFrm * MA_FASTCALL lcl_MissProtectedFrames( const SwCntntFrm *pCnt,
                                                       GetNxtPrvCnt fnNxtPrv,
                                                       FASTBOOL bMissHeadline,
                                                       FASTBOOL bInReadOnly )
{
    if ( pCnt && pCnt->IsInTab() )
    {
        BOOL bProtect = TRUE;
        while ( pCnt && bProtect )
        {
            const SwLayoutFrm *pCell = pCnt->GetUpper();
            while ( pCell && !pCell->IsCellFrm() )
                pCell = pCell->GetUpper();
            if ( !pCell ||
                    ((bInReadOnly || !pCell->GetFmt()->GetProtect().IsCntntProtected()) &&
                     (!bMissHeadline || !lcl_IsInRepeatedHeadline( pCell ) )))
                bProtect = FALSE;
            else
                pCnt = (*fnNxtPrv)( pCnt );
        }
    }
    else if ( !bInReadOnly )
        while ( pCnt && pCnt->IsProtected() )
            pCnt = (*fnNxtPrv)( pCnt );

    return pCnt;
}

BOOL MA_FASTCALL lcl_UpDown( SwPaM *pPam, const SwCntntFrm *pStart,
                    GetNxtPrvCnt fnNxtPrv, BOOL bInReadOnly )
{
    ASSERT( pPam->GetNode() == (SwCntntNode*)pStart->GetNode(),
            "lcl_UpDown arbeitet nicht fuer andere." );

    const SwCntntFrm *pCnt = 0;

    //Wenn gerade eine Tabellenselection laeuft muss ein bischen getricktst
    //werden: Beim hochlaufen an den Anfang der Zelle gehen, beim runterlaufen
    //an das Ende der Zelle gehen.
    if ( pStart->IsInTab() &&
        pPam->GetNode( TRUE )->StartOfSectionNode() !=
        pPam->GetNode( FALSE )->StartOfSectionNode() )
    {
        const SwLayoutFrm  *pCell = pStart->GetUpper();
        while ( !pCell->IsCellFrm() )
            pCell = pCell->GetUpper();
        const SwCntntFrm *pNxt = pCnt = pStart;
        while ( pCell->IsAnLower( pNxt ) )
        {   pCnt = pNxt;
            pNxt = (*fnNxtPrv)( pNxt );
        }
    }

    pCnt = (*fnNxtPrv)( pCnt ? pCnt : pStart );
    pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, TRUE, bInReadOnly );


    const SwTabFrm *pStTab = pStart->FindTabFrm();
    const SwTabFrm *pTable;
    const BOOL bTab = pStTab || (pCnt && pCnt->IsInTab()) ? TRUE : FALSE;
    BOOL bEnd = bTab ? FALSE : TRUE;

    SwTwips nX;
    if ( bTab )
    {
        SwRect aRect( pStart->Frm() );
        pStart->GetCharRect( aRect, *pPam->GetPoint() );
        nX = aRect.Center().X();

        const SwTabFrm *pTab = pCnt ? pCnt->FindTabFrm() : 0;
        if ( !pTab )
            pTab = pStTab;
        pTable = pTab;
        if ( pStTab )
        {
            //Der Fluss fuehrt von einer Tabelle in die nachste. Der X-Wert
            //muss ausgehend von der Mitte der Startzelle um die Verschiebung
            //der Tabellen korrigiert werden.
            const SwFrm *pCell = pStart->GetUpper();
            while ( pCell && !pCell->IsCellFrm() )
                pCell = pCell->GetUpper();
            ASSERT( pCell, "Zelle nicht gefunden." );
            nX = pCell->Frm().Left() + pCell->Frm().Width() / 2;
            nX += pTab->Frm().Left() - pStTab->Frm().Left();
        }

        const SwCntntFrm *pTmp = pTab->ContainsCntnt();
        if ( nX < (pTmp->Frm().Left() + pTmp->Prt().Left()) )
            nX = pTmp->Frm().Left() + pTmp->Prt().Left();
        else
        {
            pTmp = pTab->FindLastCntnt();
            if ( nX > (pTmp->Frm().Left() + pTmp->Prt().Right()) )
                nX = pTmp->Frm().Left() + pTmp->Prt().Right();
        }
    }
    do
    {
        //Wenn ich im DokumentBody bin, so will ich da auch bleiben
        if ( pStart->IsInDocBody() )
            while ( pCnt && (!pCnt->IsInDocBody() ||
                             (pCnt->IsTxtFrm() && ((SwTxtFrm*)pCnt)->IsHiddenNow())))
            {
                pCnt = (*fnNxtPrv)( pCnt );
                pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, TRUE, bInReadOnly );
            }

        //Wenn ich im Fussnotenbereich bin, so versuche ich notfalls den naechsten
        //Fussnotenbereich zu erreichen.
        else if ( pStart->IsInFtn() )
            while ( pCnt && !pCnt->IsInFtn() )
            {
                pCnt = (*fnNxtPrv)( pCnt );
                pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, TRUE, bInReadOnly );
            }

        //In Flys kann es Blind weitergehen solange ein Cntnt
        //gefunden wird.
        else if ( pStart->IsInFly() )
        {
            if ( pCnt && pCnt->IsTxtFrm() && ((SwTxtFrm*)pCnt)->IsHiddenNow() )
            {
                pCnt = (*fnNxtPrv)( pCnt );
                pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, TRUE, bInReadOnly );
            }
        }

        //Andernfalls weigere ich mich einfach den derzeitigen Bereich zu
        //verlassen.
        else if ( pCnt )
        {
            const SwFrm *pUp = pStart->GetUpper();               //Head/Foot
            while ( pUp && pUp->GetUpper() && !(pUp->GetType() & 0x0018 ) )
                pUp = pUp->GetUpper();
            BOOL bSame = FALSE;
            const SwFrm *pCntUp = pCnt->GetUpper();
            while ( pCntUp && !bSame )
            {   if ( pUp == pCntUp )
                    bSame = TRUE;
                else
                    pCntUp = pCntUp->GetUpper();
            }
            if ( !bSame )
                pCnt = 0;
        }

        if ( bTab )
        {
            if ( !pCnt )
                bEnd = TRUE;
            else
            {   const SwTabFrm *pTab = pCnt->FindTabFrm();
                if( !pTab )
                    bEnd = TRUE;
                else
                {
                    if ( pTab != pTable )
                    {   //Der Fluss fuehrt von einer Tabelle in die nachste. Der
                        //X-Wert muss um die Verschiebung der Tabellen korrigiert
                        //werden.
                        if ( pTable )
                            nX += pTab->Frm().Left() - pTable->Frm().Left();
                        pTable = pTab;
                    }
                    const SwLayoutFrm *pCell = pTab ? pCnt->GetUpper() : 0;
                    while ( pCell && !pCell->IsCellFrm() )
                        pCell = pCell->GetUpper();
                    if ( pCell &&
                         pCell->Frm().IsInside( Point( nX, pCell->Frm().Top())) )
                    {
                        bEnd = TRUE;
                        //Jetzt noch schnell den richtigen Cntnt in der Zelle
                        //greifen.
                        if ( !pCnt->Frm().IsInside( Point( nX, pCnt->Frm().Top())))
                        {
                            pCnt = pCell->ContainsCntnt();
                            if ( fnNxtPrv == lcl_GetPrvCnt )
                                while ( pCell->IsAnLower(pCnt->GetNextCntntFrm()) )
                                    pCnt = pCnt->GetNextCntntFrm();
                        }
                    }
                    else if ( pCnt->Frm().IsInside( Point( nX, pCnt->Frm().Top())))
                        bEnd = TRUE;
                }
            }
            if ( !bEnd )
            {
                pCnt = (*fnNxtPrv)( pCnt );
                pCnt = ::lcl_MissProtectedFrames( pCnt, fnNxtPrv, TRUE, bInReadOnly );
            }
        }

    } while ( !bEnd ||
              (pCnt && pCnt->IsTxtFrm() && ((SwTxtFrm*)pCnt)->IsHiddenNow()));

    if( pCnt )
    {   // setze den Point auf den Content-Node
        SwCntntNode *pCNd = (SwCntntNode*)pCnt->GetNode();
        pPam->GetPoint()->nNode = *pCNd;
        if ( fnNxtPrv == lcl_GetPrvCnt )
            pCNd->MakeEndIndex( (SwIndex*)&pPam->GetPoint()->nContent );
        else
            pCNd->MakeStartIndex( (SwIndex*)&pPam->GetPoint()->nContent );
        return TRUE;
    }
    return FALSE;
}

BOOL SwCntntFrm::UnitUp( SwPaM* pPam, const SwTwips, BOOL bInReadOnly ) const
{
    return ::lcl_UpDown( pPam, this, lcl_GetPrvCnt, bInReadOnly );
}

BOOL SwCntntFrm::UnitDown( SwPaM* pPam, const SwTwips, BOOL bInReadOnly ) const
{
    return ::lcl_UpDown( pPam, this, lcl_GetNxtCnt, bInReadOnly );
}

/*************************************************************************
|*
|*  SwRootFrm::GetCurrPage()
|*
|*  Beschreibung:       Liefert die Nummer der aktuellen Seite.
|*          Wenn die Methode einen PaM bekommt, so ist die aktuelle Seite
|*          diejenige in der der PaM sitzt. Anderfalls ist die aktuelle
|*          Seite die erste Seite innerhalb der VisibleArea.
|*          Es wird nur auf den vorhandenen Seiten gearbeitet!
|*  Ersterstellung      MA 20. May. 92
|*  Letzte Aenderung    MA 09. Oct. 97
|*
|*************************************************************************/
USHORT SwRootFrm::GetCurrPage( const SwPaM *pActualCrsr ) const
{
    ASSERT( pActualCrsr, "Welche Seite soll's denn sein?" );
    const SwFrm *pActFrm = GetFmt()->GetDoc()->GetNodes()[pActualCrsr->GetPoint()->nNode]->
                                    GetCntntNode()->GetFrm( 0,
                                                    pActualCrsr->GetPoint(),
                                                    FALSE );
    return pActFrm->FindPageFrm()->GetPhyPageNum();
}

/*************************************************************************
|*
|*  SwRootFrm::SetCurrPage()
|*
|*  Beschreibung:       Liefert einen PaM der am Anfang der gewuenschten
|*          Seite sitzt.
|*          Formatiert wird soweit notwendig
|*          Liefert Null, wenn die Operation nicht moeglich ist.
|*          Der PaM sitzt in der letzten Seite, wenn die Seitenzahl zu gross
|*          gewaehlt wurde.
|*  Ersterstellung      MA 20. May. 92
|*  Letzte Aenderung    MA 09. Oct. 97
|*
|*************************************************************************/
USHORT SwRootFrm::SetCurrPage( SwCursor* pToSet, USHORT nPageNum )
{
    ASSERT( Lower() && Lower()->IsPageFrm(), "Keine Seite vorhanden." );

    SwPageFrm *pPage = (SwPageFrm*)Lower();
    BOOL bEnd =FALSE;
    while ( !bEnd && pPage->GetPhyPageNum() != nPageNum )
    {   if ( pPage->GetNext() )
            pPage = (SwPageFrm*)pPage->GetNext();
        else
        {   //Ersten CntntFrm Suchen, und solange Formatieren bis
            //eine neue Seite angefangen wird oder bis die CntntFrm's alle
            //sind.
            const SwCntntFrm *pCntnt = pPage->ContainsCntnt();
            while ( pCntnt && pPage->IsAnLower( pCntnt ) )
            {   pCntnt->Calc();
                pCntnt = pCntnt->GetNextCntntFrm();
            }
            //Jetzt ist entweder eine neue Seite da, oder die letzte Seite
            //ist gefunden.
            if ( pPage->GetNext() )
                pPage = (SwPageFrm*)pPage->GetNext();
            else
                bEnd = TRUE;
        }
    }
    //pPage zeigt jetzt auf die 'gewuenschte' Seite. Jetzt muss noch der
    //PaM auf den Anfang des ersten CntntFrm im Body-Text erzeugt werden.
    //Wenn es sich um eine Fussnotenseite handelt, wird der PaM in die erste
    //Fussnote gesetzt.
    const SwCntntFrm *pCntnt = pPage->ContainsCntnt();
    if ( pPage->IsFtnPage() )
        while ( pCntnt && !pCntnt->IsInFtn() )
            pCntnt = pCntnt->GetNextCntntFrm();
    else
        while ( pCntnt && !pCntnt->IsInDocBody() )
            pCntnt = pCntnt->GetNextCntntFrm();
    if ( pCntnt )
    {
        SwCntntNode* pCNd = (SwCntntNode*)pCntnt->GetNode();
        pToSet->GetPoint()->nNode = *pCNd;
        pCNd->MakeStartIndex( (SwIndex*)&pToSet->GetPoint()->nContent );
        pToSet->GetPoint()->nContent = ((SwTxtFrm*)pCntnt)->GetOfst();

        SwShellCrsr* pSCrsr = (SwShellCrsr*)*pToSet;
        if( pSCrsr )
        {
            Point &rPt = pSCrsr->GetPtPos();
            rPt = pCntnt->Frm().Pos();
            rPt += pCntnt->Prt().Pos();
        }
        return pPage->GetPhyPageNum();
    }
    return 0;
}

/*************************************************************************
|*
|*    SwCntntFrm::StartxxPage(), EndxxPage()
|*
|*    Beschreibung      Cursor an Anfang/Ende der aktuellen/vorherigen/
|*      naechsten Seite. Alle sechs Methoden rufen GetFrmInPage() mit der
|*      entsprechenden Parametrisierung.
|*      Zwei Parameter steuern die Richtung: einer bestimmt die Seite, der
|*      andere Anfang/Ende.
|*      Fuer die Bestimmung der Seite und des Cntnt (Anfang/Ende) werden
|*      die im folgenden definierten Funktionen benutzt.
|*    Ersterstellung    MA 15. Oct. 92
|*    Letzte Aenderung  MA 28. Feb. 93
|*
|*************************************************************************/
SwCntntFrm *GetFirstSub( const SwLayoutFrm *pLayout )
{
    return ((SwPageFrm*)pLayout)->FindFirstBodyCntnt();
}

SwCntntFrm *GetLastSub( const SwLayoutFrm *pLayout )
{
    return ((SwPageFrm*)pLayout)->FindLastBodyCntnt();
}

SwLayoutFrm *GetNextFrm( const SwLayoutFrm *pFrm )
{
    return (pFrm->GetNext() && pFrm->GetNext()->IsLayoutFrm()) ?
                                            (SwLayoutFrm*)pFrm->GetNext() : 0;
}

SwLayoutFrm *GetThisFrm( const SwLayoutFrm *pFrm )
{
    return (SwLayoutFrm*)pFrm;
}

SwLayoutFrm *GetPrevFrm( const SwLayoutFrm *pFrm )
{
    return (pFrm->GetPrev() && pFrm->GetPrev()->IsLayoutFrm()) ?
                                            (SwLayoutFrm*)pFrm->GetPrev() : 0;
}

//Jetzt koennen auch die Funktionspointer initalisiert werden;
//sie sind in cshtyp.hxx declariert.
SwPosPage fnPageStart = GetFirstSub;
SwPosPage fnPageEnd = GetLastSub;
SwWhichPage fnPagePrev = GetPrevFrm;
SwWhichPage fnPageCurr = GetThisFrm;
SwWhichPage fnPageNext = GetNextFrm;

//Liefert den ersten/den letzten Contentframe (gesteuert ueber
//den Parameter fnPosPage) in der
//aktuellen/vorhergehenden/folgenden Seite (gesteuert durch den
//Parameter fnWhichPage).
BOOL GetFrmInPage( const SwCntntFrm *pCnt, SwWhichPage fnWhichPage,
                   SwPosPage fnPosPage, SwPaM *pPam )
{
    //Erstmal die gewuenschte Seite besorgen, anfangs die aktuelle, dann
    //die die per fnWichPage gewuenscht wurde
    const SwLayoutFrm *pLayoutFrm = pCnt->FindPageFrm();
    if ( !pLayoutFrm || (0 == (pLayoutFrm = (*fnWhichPage)(pLayoutFrm))) )
        return FALSE;

    //Jetzt den gewuenschen CntntFrm unterhalb der Seite
    if( 0 == (pCnt = (*fnPosPage)(pLayoutFrm)) )
        return FALSE;
    else
    {
        SwCntntNode *pCNd = (SwCntntNode*)pCnt->GetNode();
        pPam->GetPoint()->nNode = *pCNd;
        xub_StrLen nIdx;
        if( fnPosPage == GetFirstSub )
            nIdx = ((SwTxtFrm*)pCnt)->GetOfst();
        else
            nIdx = pCnt->GetFollow() ?
                    ((SwTxtFrm*)pCnt)->GetFollow()->GetOfst()-1 : pCNd->Len();
        pPam->GetPoint()->nContent.Assign( pCNd, nIdx );
        return TRUE;
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::GetCntntPos()
|*
|*  Beschreibung        Es wird der nachstliegende Cntnt zum uebergebenen
|*                      gesucht. Betrachtet werden die vorhergehende, die
|*                      aktuelle und die folgende Seite.
|*                      Wenn kein Inhalt gefunden wird, so wird der Bereich
 *                      erweitert bis einer gefunden wird.
|*                      Zurueckgegeben wird die 'Semantisch richtige' Position
|*                      innerhalb der PrtArea des gefundenen CntntFrm
|*  Ersterstellung      MA 15. Jul. 92
|*  Letzte Aenderung    MA 09. Jan. 97
|*
|*************************************************************************/
ULONG CalcDiff( const Point &rPt1, const Point &rPt2 )
{
    //Jetzt die Entfernung zwischen den beiden Punkten berechnen.
    //'Delta' X^2 + 'Delta'Y^2 = 'Entfernung'^2
    ULONG dX = Max( rPt1.X(), rPt2.X() ) -
               Min( rPt1.X(), rPt2.X() ),
          dY = Max( rPt1.Y(), rPt2.Y() ) -
               Min( rPt1.Y(), rPt2.Y() );
    BigInt dX1( dX ), dY1( dY );
    dX1 *= dX1; dY1 *= dY1;
    return ::SqRt( dX1 + dY1 );
}

// lcl_Inside ueberprueft, ob der Punkt innerhalb des Seitenteils liegt, in dem
// auch der CntntFrame liegt. Als Seitenteile gelten in diesem Zusammenhang
// Kopfzeile, Seitenbody, Fusszeile und FussnotenContainer.
// Dies dient dazu, dass ein CntntFrm, der im "richtigen" Seitenteil liegt,
// eher akzeptiert wird als ein anderer, der nicht dort liegt, auch wenn
// dessen Abstand zum Punkt geringer ist.

const SwLayoutFrm* lcl_Inside( const SwCntntFrm *pCnt, Point& rPt )
{
    const SwLayoutFrm* pUp = pCnt->GetUpper();
    while( pUp )
    {
        if( pUp->IsPageBodyFrm() || pUp->IsFooterFrm() || pUp->IsHeaderFrm() )
        {
            if( rPt.Y() >= pUp->Frm().Top() && rPt.Y() <= pUp->Frm().Bottom() )
                return pUp;
            return NULL;
        }
        if( pUp->IsFtnContFrm() )
            return pUp->Frm().IsInside( rPt ) ? pUp : NULL;
        pUp = pUp->GetUpper();
    }
    return NULL;
}

//Fuer MSC keine Optimierung mit e (enable register...) hier, sonst gibts
//einen Bug (ID: 2857)
#pragma optimize("e",off)

const SwCntntFrm *SwLayoutFrm::GetCntntPos( Point& rPoint,
                                            const BOOL bDontLeave,
                                            const BOOL bBodyOnly,
                                            const BOOL bCalc,
                                            const SwCrsrMoveState *pCMS,
                                            const BOOL bDefaultExpand ) const
{
    //Ersten CntntFrm ermitteln.
    const SwLayoutFrm *pStart = (!bDontLeave && bDefaultExpand && GetPrev()) ?
                                    (SwLayoutFrm*)GetPrev() : this;
    const SwCntntFrm *pCntnt = pStart->ContainsCntnt();

    if ( !pCntnt && (GetPrev() && !bDontLeave) )
        pCntnt = ContainsCntnt();

    if ( bBodyOnly && pCntnt && !pCntnt->IsInDocBody() )
        while ( pCntnt && !pCntnt->IsInDocBody() )
            pCntnt = pCntnt->GetNextCntntFrm();

    const SwCntntFrm *pActual= pCntnt;
    const SwLayoutFrm *pInside = NULL;
    USHORT nMaxPage = GetPhyPageNum() + (bDefaultExpand ? 1 : 0);
    Point aPoint = rPoint;
    ULONG nDistance = ULONG_MAX;

    while ( TRUE )  //Sicherheitsschleifchen, damit immer einer gefunden wird.
    {
        while ( pCntnt &&
                ((!bDontLeave || IsAnLower( pCntnt )) &&
                (pCntnt->GetPhyPageNum() <= nMaxPage)) )
        {
            if ( ( bCalc || pCntnt->Frm().Width() ) &&
                 ( !bBodyOnly || pCntnt->IsInDocBody() ) )
            {
                //Wenn der Cntnt in einem geschuetzen Bereich (Zelle, Ftn, Section)
                //liegt, wird der nachste Cntnt der nicht geschuetzt ist gesucht.
                const SwCntntFrm *pComp = pCntnt;
                pCntnt = ::lcl_MissProtectedFrames( pCntnt, lcl_GetNxtCnt, FALSE,
                                        pCMS ? pCMS->bSetInReadOnly : FALSE );
                if ( pComp != pCntnt )
                    continue;

                if ( !pCntnt->IsTxtFrm() || !((SwTxtFrm*)pCntnt)->IsHiddenNow() )
                {
                    if ( bCalc )
                        pCntnt->Calc();

                    SwRect aCntFrm( pCntnt->UnionFrm() );
                    if ( aCntFrm.IsInside( rPoint ) )
                    {
                        pActual = pCntnt;
                        aPoint = rPoint;
                        break;
                    }
                    //Die Strecke von rPoint zum dichtesten Punkt von pCntnt wird
                    //jetzt berechnet.
                    Point aCntntPoint( rPoint );

                    //Erst die Vertikale Position einstellen
                    if ( aCntFrm.Top() > aCntntPoint.Y() )
                        aCntntPoint.Y() = aCntFrm.Top();
                    else if ( aCntFrm.Bottom() < aCntntPoint.Y() )
                        aCntntPoint.Y() = aCntFrm.Bottom();

                    //Jetzt die Horizontale Position
                    if ( aCntFrm.Left() > aCntntPoint.X() )
                        aCntntPoint.X() = aCntFrm.Left();
                    else if ( aCntFrm.Right() < aCntntPoint.X() )
                        aCntntPoint.X() = aCntFrm.Right();

                    // pInside ist ein Seitenbereich, in dem der Punkt liegt,
                    // sobald pInside!=0 ist, werden nur noch Frames akzeptiert,
                    // die innerhalb liegen.
                    if( !pInside || ( pInside->IsAnLower( pCntnt ) &&
                        ( !pCntnt->IsInFtn() || pInside->IsFtnContFrm() ) ) )
                    {
                        const ULONG nDiff = ::CalcDiff( aCntntPoint, rPoint );
                        BOOL bBetter = nDiff < nDistance;  // Dichter dran
                        if( !pInside )
                        {
                            pInside = lcl_Inside( pCntnt, rPoint );
                            if( pInside )  // Im "richtigen" Seitenteil
                                bBetter = TRUE;
                        }
                        if( bBetter )
                        {
                            aPoint = aCntntPoint;
                            nDistance = nDiff;
                            pActual = pCntnt;
                        }
                    }
                }
            }
            pCntnt = pCntnt->GetNextCntntFrm();
            if ( bBodyOnly )
                while ( pCntnt && !pCntnt->IsInDocBody() )
                    pCntnt = pCntnt->GetNextCntntFrm();
        }
        if ( !pActual )
        {   //Wenn noch keiner gefunden wurde muss der Suchbereich erweitert
            //werden, irgenwann muessen wir einen Finden!
            //MA 09. Jan. 97: Opt fuer viele leere Seiten, wenn wir nur im
            //Body suchen, koennen wir den Suchbereich gleich in einem
            //Schritt hinreichend erweitern.
            if ( bBodyOnly )
            {
                while ( !pCntnt && pStart->GetPrev() )
                {
                    ++nMaxPage;
                    if( !pStart->GetPrev()->IsLayoutFrm() )
                        return 0;
                    pStart = (SwLayoutFrm*)pStart->GetPrev();
                    pCntnt = pStart->IsInDocBody()
                                ? pStart->ContainsCntnt()
                                : pStart->FindPageFrm()->FindFirstBodyCntnt();
                }
                if ( !pCntnt )  //irgendwann muessen wir mit irgendeinem Anfangen!
                {
                    pCntnt = pStart->FindPageFrm()->GetUpper()->ContainsCntnt();
                    while ( pCntnt && !pCntnt->IsInDocBody() )
                        pCntnt = pCntnt->GetNextCntntFrm();
                    if ( !pCntnt )
                        return 0;   //Es gibt noch keine Dokumentinhalt!
                }
            }
            else
            {
                ++nMaxPage;
                if ( pStart->GetPrev() )
                {
                    if( !pStart->GetPrev()->IsLayoutFrm() )
                        return 0;
                    pStart = (SwLayoutFrm*)pStart->GetPrev();
                    pCntnt = pStart->ContainsCntnt();
                }
                else //irgendwann muessen wir mit irgendeinem Anfangen!
                    pCntnt = pStart->FindPageFrm()->GetUpper()->ContainsCntnt();
            }
            pActual = pCntnt;
        }
        else
            break;
    }

#ifndef PRODUCT
    ASSERT( pActual, "Keinen Cntnt gefunden." );
    if ( bBodyOnly )
        ASSERT( pActual->IsInDocBody(), "Cnt nicht im Body." );
#endif

    //Spezialfall fuer das selektieren von Tabellen, nicht in wiederholte
    //TblHedlines.
    if ( pActual->IsInTab() && pCMS && pCMS->eState == MV_TBLSEL )
    {
        const SwTabFrm *pTab = pActual->FindTabFrm();
        if ( pTab->IsFollow() && pTab->GetTable()->IsHeadlineRepeat() &&
             ((SwLayoutFrm*)pTab->Lower())->IsAnLower( pActual ) )
        {
            ((SwCrsrMoveState*)pCMS)->bStop = TRUE;
            return 0;
        }
    }

    //Jetzt noch eine kleine Korrektur beim ersten/letzten
    Size aActualSize( pActual->Prt().SSize() );
    if ( aActualSize.Height() > pActual->GetUpper()->Prt().Height() )
        aActualSize.Height() = pActual->GetUpper()->Prt().Height();
    if ( !pActual->GetPrev() &&
        ((pActual->Frm().Top() + pActual->Prt().Top()) > rPoint.Y()) )
        aPoint = pActual->Frm().Pos() + pActual->Prt().Pos();
    else if ( !pActual->GetNext() &&
            ((pActual->Frm().Top() + pActual->Prt().Bottom()) < rPoint.Y()) )
    {   aPoint.Y() = pActual->Frm().Top() + pActual->Prt().Bottom();
        aPoint.X() = pActual->Frm().Left() + pActual->Prt().Right();
    }
    //Und den Point in die PrtArea bringen
    if ( bCalc )
        pActual->Calc();
    const SwRect aRect( pActual->Frm().Pos() + pActual->Prt().Pos(),
                        aActualSize );
    if ( aPoint.Y() < aRect.Top() )
        aPoint.Y() = aRect.Top();
    else if ( aPoint.Y() > aRect.Bottom() )
        aPoint.Y() = aRect.Bottom();
    if ( aPoint.X() < aRect.Left() )
        aPoint.X() = aRect.Left();
    else if ( aPoint.X() > aRect.Right() )
        aPoint.X() = aRect.Right();
    rPoint = aPoint;
    return pActual;
}

#pragma optimize("",on)

/*************************************************************************
|*
|*  SwPageFrm::GetCntntPosition()
|*
|*  Beschreibung        Analog zu SwLayoutFrm::GetCntntPos().
|*                      Spezialisiert fuer Felder in Rahmen.
|*
|*  Ersterstellung      MA 22. Mar. 95
|*  Letzte Aenderung    MA 07. Nov. 95
|*
|*************************************************************************/
void SwPageFrm::GetCntntPosition( const Point &rPt, SwPosition &rPos ) const
{
    //Ersten CntntFrm ermitteln.
    const SwCntntFrm *pCntnt = ContainsCntnt();
    if ( pCntnt )
    {
        //Einen weiter zurueck schauen (falls moeglich).
        const SwCntntFrm *pTmp = pCntnt->GetPrevCntntFrm();
        while ( pTmp && !pTmp->IsInDocBody() )
            pTmp = pTmp->GetPrevCntntFrm();
        if ( pTmp )
            pCntnt = pTmp;
    }
    else
        pCntnt = GetUpper()->ContainsCntnt();

    const SwCntntFrm *pAct = pCntnt;
    Point aAct       = rPt;
    ULONG nDist      = ULONG_MAX;

    while ( pCntnt )
    {
        SwRect aCntFrm( pCntnt->UnionFrm() );
        if ( aCntFrm.IsInside( rPt ) )
        {
            //dichter gehts nimmer.
            pAct = pCntnt;
            break;
        }

        //Die Strecke von rPt zum dichtesten Punkt von pCntnt berechnen.
        Point aPoint( rPt );

        //Erst die vertikale Position einstellen
        if ( aCntFrm.Top() > rPt.Y() )
            aPoint.Y() = aCntFrm.Top();
        else if ( aCntFrm.Bottom() < rPt.Y() )
            aPoint.Y() = aCntFrm.Bottom();

        //Jetzt die horizontale Position
        if ( aCntFrm.Left() > rPt.X() )
            aPoint.X() = aCntFrm.Left();
        else if ( aCntFrm.Right() < rPt.X() )
            aPoint.X() = aCntFrm.Right();

        const ULONG nDiff = ::CalcDiff( aPoint, rPt );
        if ( nDiff < nDist )
        {
            aAct    = aPoint;
            nDist   = nDiff;
            pAct    = pCntnt;
        }
        else if ( aCntFrm.Top() > Frm().Bottom() )
            //Dichter wirds im Sinne der Felder nicht mehr!
            break;

        pCntnt = pCntnt->GetNextCntntFrm();
        while ( pCntnt && !pCntnt->IsInDocBody() )
            pCntnt = pCntnt->GetNextCntntFrm();
    }

    //Und den Point in die PrtArea bringen
    const SwRect aRect( pAct->Frm().Pos() + pAct->Prt().Pos(), pAct->Prt().SSize() );
    if ( aAct.Y() < aRect.Top() )
        aAct.Y() = aRect.Top();
    else if ( aAct.Y() > aRect.Bottom() )
        aAct.Y() = aRect.Bottom();
    if ( aAct.X() < aRect.Left() )
        aAct.X() = aRect.Left();
    else if ( aAct.X() > aRect.Right() )
        aAct.X() = aRect.Right();

    if( !pAct->IsValid() )
    {
        // CntntFrm nicht formatiert -> immer auf Node-Anfang
        SwCntntNode* pCNd = (SwCntntNode*)pAct->GetNode();
        ASSERT( pCNd, "Wo ist mein CntntNode?" );
        rPos.nNode = *pCNd;
        rPos.nContent.Assign( pCNd, 0 );
    }
    else
    {
        SwCrsrMoveState aTmpState( MV_SETONLYTEXT );
        pAct->GetCrsrOfst( &rPos, aAct, &aTmpState );
    }
}

/*************************************************************************
|*
|*  SwRootFrm::GetNextCntntPos()
|*
|*  Beschreibung        Es wird der naechstliegende Cntnt zum uebergebenen
|*                      Point gesucht. Es wird nur im BodyText gesucht.
|*  Ersterstellung      MA 15. Jul. 92
|*  Letzte Aenderung    MA 20. Dec. 93
|*
|*************************************************************************/

//!!!!! Es wird nur der vertikal naechstliegende gesucht.
Point SwRootFrm::GetNextCntntPos( const Point& rPoint, BOOL bIgnoreTblHdln ) const
{
    //Ersten CntntFrm und seinen Nachfolger im Body-Bereich suchen
    //Damit wir uns nicht tot suchen (und vor allem nicht zuviel formatieren)
    //gehen wir schon mal von der richtigen Seite aus.
    SwLayoutFrm *pPage = (SwLayoutFrm*)Lower();
    while ( pPage && pPage->Frm().Bottom() < rPoint.Y() )
        pPage = (SwLayoutFrm*)pPage->GetNext();

    const SwCntntFrm *pCnt = pPage ? pPage->ContainsCntnt() : ContainsCntnt();
    while ( pCnt && !pCnt->IsInDocBody() )
        pCnt = pCnt->GetNextCntntFrm();

    if ( !pCnt )
        return Point( 0, 0 );

    //Liegt der Point ueber dem ersten CntntFrm?
    pCnt->Calc();
    if ( rPoint.Y() < pCnt->Frm().Top() && !lcl_IsInRepeatedHeadline( pCnt ) )
        return  pCnt->Frm().Pos();

    while ( pCnt )
    {   //Liegt der Point mitten im aktuellen CntntFrm?
        SwRect aCntFrm( pCnt->UnionFrm() );
        if ( aCntFrm.IsInside( rPoint ) && !lcl_IsInRepeatedHeadline( pCnt ) )
            return rPoint;

        const SwCntntFrm *pNxt = pCnt->GetNextCntntFrm();
        while ( pNxt && !pNxt->IsInDocBody() )
            pNxt = pNxt->GetNextCntntFrm();

        //Liegt der Point hinter dem letzten CntntFrm?
        if ( !pNxt )
            return Point( aCntFrm.Right(), aCntFrm.Bottom() );

        //Wenn der naechste CntntFrm hinter dem Point liegt ist er der
        //gesuchte.
        pNxt->Calc();
        if ( pNxt->Frm().Top() > rPoint.Y() && !lcl_IsInRepeatedHeadline( pCnt ) )
            return pNxt->Frm().Pos();
        pCnt = pNxt;
    }
    return Point( 0, 0 );
}
/*************************************************************************
|*
|*  SwRootFrm::GetPrevCntntPos()
|*
|*  Beschreibung        Es wird der vorhergehende Cntnt zum uebergebenen
|*                      Point gesucht. Es wird nur im BodyTextGesucht
|*  Ersterstellung      MA 13. Jul. 92
|*  Letzte Aenderung    MA 19. Apr. 94
|*
|*************************************************************************/
//!!!!! Es wird nur der vertikal naechstliegende gesucht.
Point SwRootFrm::GetPrevCntntPos( const Point& rPoint ) const
{
    //Ersten CntntFrm und seinen Nachfolger im Body-Bereich suchen
    //Damit wir uns nicht tot suchen (und vor allem nicht zuviel formatieren)
    //gehen wir schon mal von der richtigen Seite aus.
    SwLayoutFrm *pPage = (SwLayoutFrm*)Lower();
    while ( pPage && pPage->Frm().Bottom() < rPoint.Y() )
        pPage = (SwLayoutFrm*)pPage->GetNext();

    const SwCntntFrm *pCnt = pPage ? pPage->ContainsCntnt() : ContainsCntnt();
    while ( pCnt && !pCnt->IsInDocBody() )
        pCnt = pCnt->GetNextCntntFrm();

    if ( !pCnt )
        return Point( 0, 0 );

    //Solange der Point vor dem ersten CntntFrm liegt und es noch vorhergehende
    //Seiten gibt gehe ich jeweils eine Seite nach vorn.
    pCnt->Calc();
    while ( rPoint.Y() < pCnt->Frm().Top() && pPage->GetPrev() )
    {
        pPage = (SwLayoutFrm*)pPage->GetPrev();
        pCnt = pPage->ContainsCntnt();
        while ( !pCnt )
        {   pPage = (SwLayoutFrm*)pPage->GetPrev();
            if ( pPage )
                pCnt = pPage->ContainsCntnt();
            else
                return ContainsCntnt()->UnionFrm().Pos();
        }
        pCnt->Calc();
    }
    if ( rPoint.Y() < pCnt->Frm().Top() )
        return pCnt->UnionFrm().Pos();

    while ( pCnt )
    {   //Liegt der Point im aktuellen CntntFrm?
        SwRect aCntFrm( pCnt->UnionFrm() );
        if ( aCntFrm.IsInside( rPoint ) )
            return rPoint;

        //Ist der aktuelle der letzte CntntFrm? ||
        //Wenn der naechste CntntFrm hinter dem Point liegt, ist der
        //aktuelle der gesuchte.
        const SwCntntFrm *pNxt = pCnt->GetNextCntntFrm();
        while ( pNxt && !pNxt->IsInDocBody() )
            pNxt = pNxt->GetNextCntntFrm();

        if ( pNxt )
            pNxt->Calc();

        if ( !pNxt ||
             pNxt->Frm().Top() > rPoint.Y() )
            return Point( aCntFrm.Right(), aCntFrm.Bottom() );
        pCnt = pNxt;
    }
    return Point( 0, 0 );
}

/*************************************************************************
|*
|*  SwRootFrm::GetPagePos()
|*
|*  Beschreibung:   Liefert die absolute Dokumentpositon der gewuenschten
|*          Seite.
|*          Formatiert wird nur soweit notwendig und nur dann wenn bFormat=TRUE
|*          Liefert Null, wenn die Operation nicht moeglich ist.
|*          Die Pos ist die der letzten Seite, wenn die Seitenzahl zu gross
|*          gewaehlt wurde.
|*  Ersterstellung      MA 01. Jun. 92
|*  Letzte Aenderung    MA 09. Oct. 97
|*
|*************************************************************************/
Point SwRootFrm::GetPagePos( USHORT nPageNum ) const
{
    ASSERT( Lower() && Lower()->IsPageFrm(), "Keine Seite vorhanden." );

    const SwPageFrm *pPage = (const SwPageFrm*)Lower();
    while ( TRUE )
    {
        if ( pPage->GetPhyPageNum() >= nPageNum || !pPage->GetNext() )
            break;
        pPage = (const SwPageFrm*)pPage->GetNext();
    }
    return pPage->Frm().Pos();
}

/*************************************************************************
|*
|*    SwFrm::IsProtected()
|*
|*    Beschreibung      Ist der Frm bzw. die Section in der er steht
|*                      geschuetzt?
|*                      Auch Fly in Fly in ... und Fussnoten
|*
|*    Ersterstellung    MA 28. Jul. 93
|*    Letzte Aenderung  MA 06. Nov. 97
|*
|*************************************************************************/
BOOL SwFrm::IsProtected() const
{
    //Der Frm kann in Rahmen, Zellen oder Bereichen geschuetzt sein.
    //Geht auch FlyFrms rekursiv hoch. Geht auch von Fussnoten zum Anker.
    const SwFrm *pFrm = this;
    do
    {
        if ( pFrm->IsCntntFrm() )
        {
            if ( ((SwCntntFrm*)pFrm)->GetNode()->IsInProtectSect() )
                return TRUE;
        }
        else if ( ((SwLayoutFrm*)pFrm)->GetFmt()->GetProtect().IsCntntProtected() )
            return TRUE;

        if ( pFrm->IsFlyFrm() )
        {
            //Der Schutz des Inhaltes kann bei Verkettung vom Master der Kette
            //vorgegeben werden.
            if ( ((SwFlyFrm*)pFrm)->GetPrevLink() )
            {
                SwFlyFrm *pMaster = (SwFlyFrm*)pFrm;
                do
                {   pMaster = pMaster->GetPrevLink();
                } while ( pMaster->GetPrevLink() );
                if ( pMaster->IsProtected() )
                    return TRUE;
            }
            pFrm = ((SwFlyFrm*)pFrm)->GetAnchor();
        }
        else if ( pFrm->IsFtnFrm() )
            pFrm = ((SwFtnFrm*)pFrm)->GetRef();
        else
            pFrm = pFrm->GetUpper();

    } while ( pFrm );

    return FALSE;
}

/*************************************************************************
|*
|*    SwFrm::GetPhyPageNum()
|*    Beschreibung:     Liefert die physikalische Seitennummer
|*
|*    Ersterstellung    OK 06.07.93 08:35
|*    Letzte Aenderung  MA 30. Nov. 94
|*
|*************************************************************************/
USHORT SwFrm::GetPhyPageNum() const
{
    const SwPageFrm *pPage = FindPageFrm();
    return pPage ? pPage->GetPhyPageNum() : 0;
}

/*-----------------26.02.01 11:25-------------------
 * SwFrm::WannaRightPage()
 * decides if the page want to be a rightpage or not.
 * If the first content of the page has a page descriptor,
 * we take the follow of the page descriptor of the last not empty page.
 * If this descriptor allows only right(left) pages and the page
 * isn't an empty page then it wanna be such right(left) page.
 * If the descriptor allows right and left pages, we look for a number offset
 * in the first content. If there is one, odd number results right pages,
 * even number results left pages.
 * If there is no number offset, we take the physical page number instead,
 * but a previous empty page don't count.
 * --------------------------------------------------*/

BOOL SwFrm::WannaRightPage() const
{
    const SwPageFrm *pPage = FindPageFrm();
    if ( !pPage || !pPage->GetUpper() )
        return TRUE;

    const SwFrm *pFlow = pPage->FindFirstBodyCntnt();
    SwPageDesc *pDesc = 0;
    USHORT nPgNum = 0;
    if ( pFlow )
    {
        if ( pFlow->IsInTab() )
            pFlow = pFlow->FindTabFrm();
        const SwFlowFrm *pTmp = SwFlowFrm::CastFlowFrm( pFlow );
        if ( !pTmp->IsFollow() )
        {
            const SwFmtPageDesc& rPgDesc = pFlow->GetAttrSet()->GetPageDesc();
            pDesc = (SwPageDesc*)rPgDesc.GetPageDesc();
            nPgNum = rPgDesc.GetNumOffset();
        }
    }
    if ( !pDesc )
    {
        SwPageFrm *pPrv = (SwPageFrm*)pPage->GetPrev();
        if( pPrv && pPrv->IsEmptyPage() )
            pPrv = (SwPageFrm*)pPrv->GetPrev();
        if( pPrv )
            pDesc = pPrv->GetPageDesc()->GetFollow();
        else
        {
            const SwDoc* pDoc = pPage->GetFmt()->GetDoc();
            pDesc = (SwPageDesc*)&pDoc->GetPageDesc( 0 );
        }
    }
    ASSERT( pDesc, "No pagedescriptor" );
    BOOL bOdd;
    if( nPgNum )
        bOdd = nPgNum % 2 ? TRUE : FALSE;
    else
    {
        bOdd = pPage->OnRightPage();
        if( pPage->GetPrev() && ((SwPageFrm*)pPage->GetPrev())->IsEmptyPage() )
            bOdd = !bOdd;
    }
    if( !pPage->IsEmptyPage() )
    {
        if( !pDesc->GetRightFmt() )
            bOdd = FALSE;
        else if( !pDesc->GetLeftFmt() )
            bOdd = TRUE;
    }
    return bOdd;
}

/*************************************************************************
|*
|*    SwFrm::GetVirtPageNum()
|*    Beschreibung:     Liefert die virtuelle Seitennummer mit Offset
|*
|*    Ersterstellung    OK 06.07.93 08:35
|*    Letzte Aenderung  MA 30. Nov. 94
|*
|*************************************************************************/
USHORT SwFrm::GetVirtPageNum() const
{
    const SwPageFrm *pPage = FindPageFrm();
    if ( !pPage || !pPage->GetUpper() )
        return 0;

    USHORT nPhyPage = pPage->GetPhyPageNum();
    if ( !((SwRootFrm*)pPage->GetUpper())->IsVirtPageNum() )
        return nPhyPage;

    //Den am naechsten stehenden Absatz mit virtueller Seitennummer suchen.
    //Da das rueckwaertsuchen insgesamt sehr viel Zeit verschlingt suchen
    //wir jetzt gezielt ueber die Abhaengigkeiten.
    //von den PageDescs bekommen wir die Attribute, von den Attributen
    //wiederum bekommen wir die Absaetze.
    const SwPageFrm *pVirtPage = 0;
    const SwFrm *pFrm = 0;
    const SfxItemPool &rPool = pPage->GetFmt()->GetDoc()->GetAttrPool();
    const SfxPoolItem* pItem;
    USHORT nMaxItems = rPool.GetItemCount( RES_PAGEDESC );
    for( USHORT n = 0; n < nMaxItems; ++n )
    {
        if( 0 == (pItem = rPool.GetItem( RES_PAGEDESC, n ) ))
            continue;

        const SwFmtPageDesc *pDesc = (SwFmtPageDesc*)pItem;
        if ( pDesc->GetNumOffset() && pDesc->GetDefinedIn() )
        {
            const SwModify *pMod = pDesc->GetDefinedIn();
            SwVirtPageNumInfo aInfo( pPage );
            pMod->GetInfo( aInfo );
            if ( aInfo.GetPage() )
            {
                if( !pVirtPage || ( pVirtPage && aInfo.GetPage()->
                    GetPhyPageNum() > pVirtPage->GetPhyPageNum() ) )
                {
                    pVirtPage = aInfo.GetPage();
                    pFrm = aInfo.GetFrm();
                }
            }
        }
    }
    if ( pFrm )
        return nPhyPage - pFrm->GetPhyPageNum() +
               pFrm->GetAttrSet()->GetPageDesc().GetNumOffset();
    return nPhyPage;
}

/*************************************************************************
|*
|*  SwRootFrm::MakeTblCrsrs()
|*
|*  Ersterstellung      MA 14. May. 93
|*  Letzte Aenderung    MA 02. Feb. 94
|*
|*************************************************************************/
//Ermitteln und einstellen derjenigen Zellen die von der Selektion
//eingeschlossen sind.

void SwRootFrm::MakeTblCrsrs( SwTableCursor& rTblCrsr )
{
    //Union-Rects und Tabellen (Follows) der Selektion besorgen.
    ASSERT( rTblCrsr.GetCntntNode() && rTblCrsr.GetCntntNode( FALSE ),
            "Tabselection nicht auf Cnt." );

    Point aPtPt, aMkPt;
    {
        SwShellCrsr* pShCrsr =  rTblCrsr.operator SwShellCrsr*();
        // Aufgrund eines CompilerBugs von Linux muss
        // der Zeigeroperator explizit gerufen werden

        if( pShCrsr )
        {
            aPtPt = pShCrsr->GetPtPos();
            aMkPt = pShCrsr->GetMkPos();
        }
    }
    const SwLayoutFrm *pStart = rTblCrsr.GetCntntNode()->GetFrm(
                                            &aPtPt, 0, FALSE )->GetUpper(),
                      *pEnd   = rTblCrsr.GetCntntNode(FALSE)->GetFrm(
                                            &aMkPt, 0, FALSE )->GetUpper();
    SwSelUnions aUnions;
    ::MakeSelUnions( aUnions, pStart, pEnd );

    SwSelBoxes aNew;

    const FASTBOOL bReadOnlyAvailable = rTblCrsr.IsReadOnlyAvailable();

    for ( USHORT i = 0; i < aUnions.Count(); ++i )
    {
        SwSelUnion *pUnion = aUnions[i];
        const SwTabFrm *pTable = pUnion->GetTable();

        SwLayoutFrm *pRow = (SwLayoutFrm*)pTable->Lower();
        if ( pRow && pTable->IsFollow() &&
             pTable->GetTable()->IsHeadlineRepeat() )
            pRow = (SwLayoutFrm*)pRow->GetNext();
        while ( pRow )
        {
            if ( pRow->Frm().IsOver( pUnion->GetUnion() ) )
            {
                const SwLayoutFrm *pCell = pRow->FirstCell();

                while ( pCell && pRow->IsAnLower( pCell ) )
                {
                    ASSERT( pCell->IsCellFrm(), "Frame ohne Celle" );
                    if( IsFrmInTblSel( pUnion->GetUnion(), pCell ) &&
                        (bReadOnlyAvailable ||
                         !pCell->GetFmt()->GetProtect().IsCntntProtected()) )
                    {
                        SwTableBox* pInsBox = (SwTableBox*)((SwCellFrm*)pCell)->GetTabBox();
                        aNew.Insert( pInsBox );
                    }
                    if ( pCell->GetNext() )
                    {
                        pCell = (const SwLayoutFrm*)pCell->GetNext();
                        if ( pCell->Lower()->IsRowFrm() )
                            pCell = pCell->FirstCell();
                    }
                    else
                    {
                        const SwLayoutFrm* pLastCell = pCell;
                        do
                        {
                            pCell = pCell->GetNextLayoutLeaf();
                        } while ( pCell && pLastCell->IsAnLower( pCell ) );
                        // Fuer (spaltige) Bereiche...
                        if( pCell && pCell->IsInTab() )
                        {
                            while( !pCell->IsCellFrm() )
                            {
                                pCell = pCell->GetUpper();
                                ASSERT( pCell, "Where's my cell?" );
                            }
                        }
                    }
                }
            }
            pRow = (SwLayoutFrm*)pRow->GetNext();
        }
    }

    SwSelBoxes& rOld = (SwSelBoxes&)rTblCrsr.GetBoxes();
    USHORT nOld = 0, nNew = 0;
    while ( nOld < rOld.Count() && nNew < aNew.Count() )
    {
        const SwTableBox* pPOld = *( rOld.GetData() + nOld );
        const SwTableBox* pPNew = *( aNew.GetData() + nNew );
        if( pPOld == pPNew )
        {
            // diese Box bleibt erhalten
            ++nOld;
            aNew.Remove( nNew );
        }
        else if( pPOld->GetSttIdx() < pPNew->GetSttIdx() )
            rTblCrsr.DeleteBox( nOld );
        else
        {
            rTblCrsr.InsertBox( *pPNew );
            ++nOld;
            ++nNew;
        }
    }

    while( nOld < rOld.Count() )
        rTblCrsr.DeleteBox( nOld );

    for( ; nNew < aNew.Count(); ++nNew )
        rTblCrsr.InsertBox( **( aNew.GetData() + nNew ) );
}


/*************************************************************************
|*
|*  SwRootFrm::CalcFrmRects
|*
|*  Ersterstellung      MA 24. Aug. 92
|*  Letzte Aenderung    MA 24. Aug. 93
|*
|*************************************************************************/

/*
 * nun koennen folgende Situationen auftreten:
 *  1. Start und Ende liegen in einer Bildschirm - Zeile und im
 *     gleichen Node
 *      -> aus Start und End ein Rectangle, dann Ok
 *  2. Start und Ende liegen in einem Frame (dadurch im gleichen Node!)
 *      -> Start nach rechts, End nach links erweitern,
 *         und bei mehr als 2 Bildschirm - Zeilen, das dazwischen
 *         liegende berechnen
 *  3. Start und Ende liegen in verschiedenen Frames
 *      -> Start nach rechts erweitern, bis Frame-Ende Rect berechnen
 *         Ende nach links erweitern, bis Frame-Start Rect berechnen
 *         und bei mehr als 2 Frames von allen dazwischen liegenden
 *         Frames die PrtArea dazu.
 *  4. Wenn es sich um eine Tabellenselektion handelt wird fuer jeden
 *     PaM im Ring der CellFrm besorgt, dessen PrtArea wird zu den
 *     Rechtecken addiert.
 *
 * Grosser Umbau wg. der FlyFrm; denn diese muessen ausgespart werden.
 * Ausnahmen: - Der Fly in dem die Selektion stattfindet (wenn sie in einem Fly
 *              stattfindet).
 *            - Die Flys, die vom Text unterlaufen werden.
 * Arbeitsweise: Zuerst wird eine SwRegion mit der Root initialisiert.
 *               Aus der Region werden die zu invertierenden Bereiche
 *               ausgestantzt. Die Region wird Komprimiert und letztlich
 *               invertiert. Damit liegen dann die zu invertierenden
 *               Rechtecke vor.
 *               Am Ende werden die Flys aus der Region ausgestanzt.
 */

inline void Sub( SwRegionRects& rRegion, const SwRect& rRect )
{
    if( rRect.Width() > 1 && rRect.Height() > 1 &&
        rRect.IsOver( rRegion.GetOrigin() ))
        rRegion -= rRect;
}

void SwRootFrm::CalcFrmRects( SwShellCrsr &rCrsr, BOOL bIsTblMode )
{
    ViewShell *pSh = GetShell();
    SwRegionRects aRegion( pSh ? pSh->VisArea() : Frm() );
    const SwNodes &rNds = GetFmt()->GetDoc()->GetNodes();

    //Erstmal die CntntFrms zum Start und End besorgen, die brauch ich auf
    //jedenfall.
    SwPosition *pStartPos = rCrsr.Start(),
               *pEndPos   = rCrsr.GetPoint() == pStartPos ?
                                rCrsr.GetMark() : rCrsr.GetPoint();
    const SwCntntFrm *pStartFrm = rNds[ pStartPos->nNode ]->
        GetCntntNode()->GetFrm( &rCrsr.GetSttPos(), pStartPos );

    const SwCntntFrm *pEndFrm   = rNds[ pEndPos->nNode ]->
        GetCntntNode()->GetFrm( &rCrsr.GetEndPos(), pEndPos );

    ASSERT( (pStartFrm && pEndFrm), "Keine CntntFrms gefunden." );

    //Damit die FlyFrms, in denen selektierte Frames stecken, nicht
    //abgezogen werden
    SwSortDrawObjs aSortObjs;
    if ( pStartFrm->IsInFly() )
    {
        const SdrObjectPtr pObj = (SdrObject*)pStartFrm->FindFlyFrm()->GetVirtDrawObj();
        aSortObjs.Insert( pObj );
        const SdrObjectPtr pObj2 = (SdrObject*)pEndFrm->FindFlyFrm()->GetVirtDrawObj();
        aSortObjs.Insert( pObj2 );
    }

    //Fall 4: Tabellenselection
    if( bIsTblMode )
    {
        const SwFrm *pCell = pStartFrm->GetUpper();
        while ( !pCell->IsCellFrm() )
            pCell = pCell->GetUpper();
        SwRect aTmp( pCell->Prt() );
        aTmp.Pos() += pCell->Frm().Pos();
        aRegion.ChangeOrigin( aTmp );
        aRegion.Remove( 0, aRegion.Count() );
        aRegion.Insert( aTmp, 0 );
    }
    else
    {
        // falls eine nicht erlaubte Selection besteht, dann korrigiere das
        // nicht erlaubt ist Header/Footer/TableHeadline ueber 2 Seiten
        do {    // middle check loop
            const SwLayoutFrm* pSttLFrm = pStartFrm->GetUpper();
            const USHORT cHdFtTblHd = FRM_HEADER | FRM_FOOTER | FRM_TAB;
            while( pSttLFrm &&
                ! (cHdFtTblHd & pSttLFrm->GetType() ))
                pSttLFrm = pSttLFrm->GetUpper();
            if( !pSttLFrm )
                break;
            const SwLayoutFrm* pEndLFrm = pEndFrm->GetUpper();
            while( pEndLFrm &&
                ! (cHdFtTblHd & pEndLFrm->GetType() ))
                pEndLFrm = pEndLFrm->GetUpper();
            if( !pEndLFrm )
                break;

            ASSERT( pEndLFrm->GetType() == pSttLFrm->GetType(),
                    "Selection ueber unterschiedliche Inhalte" );
            switch( pSttLFrm->GetType() )
            {
            case FRM_HEADER:
            case FRM_FOOTER:
                // auf unterschiedlichen Seiten ??
                // dann immer auf die Start-Seite
                if( pEndLFrm->FindPageFrm() != pSttLFrm->FindPageFrm() )
                {
                    // End- auf den Start-CntntFrame setzen
                    if( pStartPos == rCrsr.GetPoint() )
                        pEndFrm = pStartFrm;
                    else
                        pStartFrm = pEndFrm;
                }
                break;
            case FRM_TAB:
                // auf unterschiedlichen Seiten ??
                // existiert
                // dann teste auf Tabelle-Headline
                {
                    const SwTabFrm* pTabFrm = (SwTabFrm*)pSttLFrm;
                    if( ( pTabFrm->GetFollow() ||
                        ((SwTabFrm*)pEndLFrm)->GetFollow() ) &&
                        pTabFrm->GetTable()->IsHeadlineRepeat() &&
                        pTabFrm->GetLower() !=
                            ((SwTabFrm*)pEndLFrm)->GetLower() )
                    {
                        // End- auf den Start-CntntFrame setzen
                        if( pStartPos == rCrsr.GetPoint() )
                            pEndFrm = pStartFrm;
                        else
                            pStartFrm = pEndFrm;
                    }
                }
                break;
            }
        } while( FALSE );

        SwCrsrMoveState aTmpState( MV_NONE );
        aTmpState.b2Lines = sal_True;
        //CntntRects zu Start- und EndFrms.
        SwRect aStRect, aEndRect;
        pStartFrm->GetCharRect( aStRect, *pStartPos, &aTmpState );
        Sw2LinesPos *pSt2Pos = aTmpState.p2Lines;
        aTmpState.p2Lines = NULL;
        pEndFrm->GetCharRect  ( aEndRect, *pEndPos, &aTmpState );
        Sw2LinesPos *pEnd2Pos = aTmpState.p2Lines;

        SwRect aStFrm ( pStartFrm->UnionFrm( sal_True ) );
        aStFrm.Intersection( pStartFrm->PaintArea() );
        SwRect aEndFrm( pStartFrm == pEndFrm ? aStFrm :
                                               pEndFrm->UnionFrm( sal_True ) );
        if( pStartFrm != pEndFrm )
            aEndFrm.Intersection( pEndFrm->PaintArea() );
        // If there's no doubleline portion involved or start and end are both
        // in the same doubleline portion, all works fine, but otherwise
        // we need the following...
        if( pSt2Pos != pEnd2Pos && ( !pSt2Pos || !pEnd2Pos ||
            pSt2Pos->aPortion != pEnd2Pos->aPortion ) )
        {
            // If we have a start(end) position inside a doubleline portion
            // the surrounded part of the doubleline portion is subtracted
            // from the region and the aStRect(aEndRect) is set to the
            // end(start) of the doubleline portion.
            if( pSt2Pos )
            {
                SwRect aTmp( aStRect.Pos(),
                    Point( pSt2Pos->aPortion.Right(), aStRect.Bottom() ) );
                if( !pSt2Pos->nMultiType ||
                    pSt2Pos->aPortion.Top() == aTmp.Top() )
                    aTmp.Top( pSt2Pos->aLine.Top() );
                aTmp.Intersection( aStFrm );
                Sub( aRegion, aTmp );
                if( pSt2Pos->nMultiType &&
                    aStRect.Bottom() < pSt2Pos->aLine.Bottom() )
                {
                    aTmp.Top( aTmp.Bottom() );
                    aTmp.Bottom( pSt2Pos->aLine.Bottom() );
                    if( aStRect.Bottom() < pSt2Pos->aPortion.Bottom() )
                        aTmp.Left( pSt2Pos->aPortion.Left() );
                    aTmp.Intersection( aStFrm );
                    Sub( aRegion, aTmp );
                }
                aStRect = SwRect( Point( pSt2Pos->aPortion.Right(),
                    pSt2Pos->aLine.Top() ), Size( 1, pSt2Pos->aLine.Height() ));
            }
            if( pEnd2Pos )
            {
                SwRect aTmp( Point( pEnd2Pos->aPortion.Left(), aEndRect.Top() ),
                    Point( aEndRect.Left() + aEndRect.Width(),
                           aEndRect.Top() + aEndRect.Height() ) );
                if( !pEnd2Pos->nMultiType ||
                    pEnd2Pos->aPortion.Bottom() == aEndRect.Bottom() )
                    aTmp.Bottom( pEnd2Pos->aLine.Bottom() );
                aTmp.Intersection( aEndFrm );
                Sub( aRegion, aTmp );
                if( pEnd2Pos->nMultiType &&
                    aEndRect.Top() > pEnd2Pos->aLine.Top() )
                {
                    aTmp.Bottom( aTmp.Top() );
                    aTmp.Top( pEnd2Pos->aLine.Top() );
                    if( aEndRect.Top() > pEnd2Pos->aPortion.Top() )
                        aTmp.Right( pEnd2Pos->aPortion.Right() );
                    aTmp.Intersection( aEndFrm );
                    Sub( aRegion, aTmp );
                }
                aEndRect = SwRect( Point( pEnd2Pos->aPortion.Left(),
                    pEnd2Pos->aLine.Top()), Size( 1, pEnd2Pos->aLine.Height()));
            }
        }

        // The charrect may be outside the paintarea (for cursortravelling)
        // but the selection has to be restricted to the paintarea
        if( aStRect.Left() < aStFrm.Left() )
            aStRect.Left( aStFrm.Left() );
        else if( aStRect.Left() > aStFrm.Right() )
            aStRect.Left( aStFrm.Right() );
        SwTwips nTmp = aStRect.Right();
        if( nTmp < aStFrm.Left() )
            aStRect.Right( aStFrm.Left() );
        else if( nTmp > aStFrm.Right() )
            aStRect.Right( aStFrm.Right() );
        if( aEndRect.Left() < aEndFrm.Left() )
            aEndRect.Left( aEndFrm.Left() );
        else if( aEndRect.Left() > aEndFrm.Right() )
            aEndRect.Left( aEndFrm.Right() );
        nTmp = aEndRect.Right();
        if( nTmp < aEndFrm.Left() )
            aEndRect.Right( aEndFrm.Left() );
        else if( nTmp > aEndFrm.Right() )
            aEndRect.Right( aEndFrm.Right() );

        if( pStartFrm == pEndFrm )
        {
            sal_Bool bSameRotatedLine =
                pSt2Pos && pEnd2Pos && 2 != pSt2Pos->nMultiType &&
                pSt2Pos->aPortion == pEnd2Pos->aPortion;
            //Fall 1: (Gleicher Frame und gleiche Zeile)
            if( aStRect.Top() == aEndRect.Top() || bSameRotatedLine )
            {
                Point aTmpSt( aStRect.Pos() );
                Point aTmpEnd( aEndRect.Right(), aEndRect.Bottom() );
                if( bSameRotatedLine && aTmpSt.Y() > aTmpEnd.Y() )
                {
                    long nTmpY = aTmpEnd.Y();
                    aTmpEnd.Y() = aTmpSt.Y();
                    aTmpSt.Y() = nTmpY;
                }

                SwRect aTmp = SwRect( aTmpSt, aTmpEnd );
                // Bug 34888: falls Inhalt selektiert ist, der keinen Platz
                //            einnimmt (z.B. PostIts,RefMarks, TOXMarks),
                //            dann mindestens die Breite des Crsr setzen.
                if( 1 == aTmp.Width() && pStartPos->nContent.GetIndex() !=
                    pEndPos->nContent.GetIndex() )
                {
                    OutputDevice* pOut = pSh->GetOut();
                    long nCrsrWidth = pOut->GetSettings().GetStyleSettings().
                                        GetCursorSize();
                    aTmp.Width( pOut->PixelToLogic( Size( nCrsrWidth, 0 ) ).Width() );
                }
                aTmp.Intersection( aStFrm );
                Sub( aRegion, aTmp );
            }
            //Fall 2: (Gleicher Frame ueber mehr als eine Zeile)
            else
            {
                SwTwips lLeft, lRight;
                if( pSt2Pos && pEnd2Pos && pSt2Pos->aPortion == pEnd2Pos->aPortion )
                {
                    lLeft = pSt2Pos->aPortion.Left();
                    lRight = pSt2Pos->aPortion.Right();
                }
                else
                {
                    lLeft = pStartFrm->Frm().Left() + pStartFrm->Prt().Left();
                    lRight = pStartFrm->Frm().Left() + pStartFrm->Prt().Right();
                }
                if( lLeft < aStFrm.Left() )
                    lLeft = aStFrm.Left();
                if( lRight > aStFrm.Right() )
                    lRight = aStFrm.Right();
                //Erste Zeile
                Sub( aRegion, SwRect( aStRect.Pos(),Point( lRight, aStRect.Bottom())));

                //Wenn mindestens ein Twips zwischen Start- und Endzeile liegt,
                //so wird halt alles dazwischenliegende mit aufgenommen.
                if( (aStRect.Pos().Y()+aStRect.SSize().Height()) != aEndRect.Top() )
                    Sub( aRegion, SwRect( Point( lLeft,
                                       (aStRect.Pos().Y()+aStRect.SSize().Height()) ),
                                       Point( lRight, aEndRect.Top()-1 )));

                //und die letzte Zeile
                Sub( aRegion, SwRect( Point( lLeft, aEndRect.Top() ),
                                   Point( aEndRect.Left(), aEndRect.Bottom() )));
            }
        }
        //Fall 3: (Unterschiedliche Frm's, moeglicherweise auch welche
        //         dazwischen)
        else
        {
                //Erst den StartFrm verknurpseln...
            Sub( aRegion, SwRect( aStRect.Pos(),
                            Point( aStFrm.Right(), aStRect.Bottom())));

            if( aStFrm.Bottom() != aStRect.Bottom() )
                Sub( aRegion, SwRect( Point( aStFrm.Left(),
                                    (aStRect.Pos().Y()+aStRect.SSize().Height()) ),
                                Point( aStFrm.Right(), aStFrm.Bottom() )));

                //Nun alles dazwischenliegende (kann auch nix sein)...
            BOOL bBody = pStartFrm->IsInDocBody();
            const SwCntntFrm *pCntnt = pStartFrm->GetNextCntntFrm();
            SwRect aPrvRect;

// JP - 24.11.93:
// wie kann man die Schleife noch optimieren, damit nur die Frames, die
// in der VisArea liegen, beachtet werden

            while ( pCntnt != pEndFrm )
            {
                if ( pCntnt->IsInFly() )
                {
                    const SdrObjectPtr pObj = (SdrObject*)pCntnt->FindFlyFrm()->GetVirtDrawObj();
                    aSortObjs.Insert( pObj );
                }

                //Wenn ich im DocumentBody war, so beachte ich nur Frm's
                //die im Body liegen und umgekehrt.
                if ( bBody == pCntnt->IsInDocBody() )
                {
                    SwRect aCRect( pCntnt->UnionFrm( sal_True ) );
                    aCRect.Intersection( pCntnt->PaintArea() );
                    if( aCRect.IsOver( aRegion.GetOrigin() ))
                    {
                        SwRect aTmp( aPrvRect );
                        aTmp.Union( aCRect );
                        if ( (aPrvRect.Height() * aPrvRect.Width() +
                            aCRect.Height()   * aCRect.Width()) ==
                            (aTmp.Height() * aTmp.Width()) )
                        {
                            aPrvRect.Union( aCRect );
                        }
                        else
                        {
                            if ( aPrvRect.HasArea() )
                                Sub( aRegion, aPrvRect );
                            aPrvRect = aCRect;
                        }
                    }
                }
                pCntnt = pCntnt->GetNextCntntFrm();
            }
            if ( aPrvRect.HasArea() )
                Sub( aRegion, aPrvRect );

                //Jetzt den EndFrm...
            if ( aEndFrm.Top() != aEndRect.Top() )
                Sub( aRegion, SwRect( aEndFrm.Pos(),
                                Point( aEndFrm.Right(), aEndRect.Top()-1 ) ));

            Sub( aRegion, SwRect( aEndFrm.Pos(),
                               Point( aEndRect.Left(), aEndRect.Bottom() )));

        }
//      aRegion.Compress( FALSE );
        aRegion.Invert();
        delete pSt2Pos;
        delete pEnd2Pos;
    }

    //Flys mit Durchlauf ausstanzen. Nicht ausgestanzt werden Flys:
    //- die Lower des StartFrm/EndFrm sind (FlyInCnt und alle Flys die wiederum
    //  darin sitzen)
    //- in der Z-Order ueber denjenigen Flys stehen in denen sich der StartFrm
    //  befindet.
    const SwPageFrm *pPage      = pStartFrm->FindPageFrm();
    const SwPageFrm *pEndPage   = pEndFrm->FindPageFrm();
    const SwFlyFrm  *pStartFly  = pStartFrm->FindFlyFrm();
    const SwFlyFrm  *pEndFly    = pEndFrm->FindFlyFrm();
    while ( pPage )
    {
        if ( pPage->GetSortedObjs() )
        {
            const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
            for ( USHORT i = 0; i < rObjs.Count(); ++i )
            {
                SdrObject *pO = rObjs[i];
                if ( !pO->IsWriterFlyFrame() )
                    continue;
                const SwVirtFlyDrawObj *pObj = (SwVirtFlyDrawObj*)pO;
                const SwLayoutFrm *pFly = pObj->GetFlyFrm();
                const SwFmtSurround &rSur = pFly->GetFmt()->GetSurround();
                if ( !pFly->IsAnLower( pStartFrm ) &&
                     (rSur.GetSurround() != SURROUND_THROUGHT &&
                      !rSur.IsContour()) )
                {
                    if ( USHRT_MAX != aSortObjs.Seek_Entry( pO ) )
                        continue;

                    FASTBOOL bSub = TRUE;
                    const UINT32 nPos = pObj->GetOrdNum();
                    for ( USHORT k = 0; bSub && k < aSortObjs.Count(); ++k )
                    {
                        SwFlyFrm *pTmp = ((SwVirtFlyDrawObj*)aSortObjs[k])->GetFlyFrm();
                        do
                        {   if ( nPos < pTmp->GetVirtDrawObj()->GetOrdNumDirect() )
                                bSub = FALSE;
                            else
                                pTmp = pTmp->GetAnchor()->FindFlyFrm();
                        } while ( bSub && pTmp );
                    }
                    if ( bSub )
                        Sub( aRegion, pFly->Frm() );
                }
            }
        }
        if ( pPage == pEndPage )
            break;
        else
            pPage = (SwPageFrm*)pPage->GetNext();
    }

    //Weil's besser aussieht noch die DropCaps ausschliessen.
    SwRect aDropRect;
    if ( pStartFrm->IsTxtFrm() )
    {
        if ( ((SwTxtFrm*)pStartFrm)->GetDropRect( aDropRect ) )
            Sub( aRegion, aDropRect );
    }
    if ( pEndFrm != pStartFrm && pEndFrm->IsTxtFrm() )
    {
        if ( ((SwTxtFrm*)pEndFrm)->GetDropRect( aDropRect ) )
            Sub( aRegion, aDropRect );
    }

    rCrsr.Remove( 0, rCrsr.Count() );
    rCrsr.Insert( &aRegion, 0 );
}


