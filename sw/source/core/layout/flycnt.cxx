/*************************************************************************
 *
 *  $RCSfile: flycnt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-02 10:43:36 $
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


#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "flyfrm.hxx"
#include "txtfrm.hxx"
#include "doc.hxx"
#include "viewsh.hxx"
#include "viewimp.hxx"
#include "pam.hxx"
#include "frmfmt.hxx"
#include "frmtool.hxx"
#include "dflyobj.hxx"
#include "hints.hxx"
#include "swundo.hxx"
#include "errhdl.hxx"

#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#include "tabfrm.hxx"
#include "flyfrms.hxx"
#include "crstate.hxx"
#include "sectfrm.hxx"

/*************************************************************************
|*
|*  SwFlyAtCntFrm::SwFlyAtCntFrm()
|*
|*  Ersterstellung      MA 11. Nov. 92
|*  Letzte Aenderung    MA 09. Apr. 99
|*
|*************************************************************************/

SwFlyAtCntFrm::SwFlyAtCntFrm( SwFlyFrmFmt *pFmt, SwFrm *pAnch ) :
    SwFlyFreeFrm( pFmt, pAnch )
{
    bAtCnt = TRUE;
    bAutoPosition = FLY_AUTO_CNTNT == pFmt->GetAnchor().GetAnchorId();
}

/*************************************************************************
|*
|*  SwFlyAtCntFrm::CheckCharRect()
|*
|*************************************************************************/

void SwFlyAtCntFrm::CheckCharRect()
{
    SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
    if( FLY_AUTO_CNTNT == pFmt->GetAnchor().GetAnchorId() && GetAnchor() )
    {
        SwRect aAutoPos;
        const SwFmtAnchor& rAnch = pFmt->GetAnchor();
        if( !rAnch.GetCntntAnchor() )
            return;
        if( ((SwTxtFrm*)GetAnchor())->GetAutoPos( aAutoPos,
              *rAnch.GetCntntAnchor() ) && aAutoPos != aLastCharRect )
        {
            SwFmtVertOrient aVert( pFmt->GetVertOrient() );
            SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
            if( ( REL_CHAR == aHori.GetRelationOrient() &&
                  aAutoPos.Left() != aLastCharRect.Left() ) ||
                  ( REL_CHAR == aVert.GetRelationOrient() &&
                    ( aAutoPos.Top() != aLastCharRect.Top() ||
                      aAutoPos.Height() != aLastCharRect.Height() ) ) )
                InvalidatePos();
            aLastCharRect = aAutoPos;
        }
    }
}

/*************************************************************************
|*
|*  SwFlyAtCntFrm::Modify()
|*
|*  Ersterstellung      MA 08. Feb. 93
|*  Letzte Aenderung    MA 23. Nov. 94
|*
|*************************************************************************/

void SwFlyAtCntFrm::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
{
    USHORT nWhich = pNew ? pNew->Which() : 0;
    const SwFmtAnchor *pAnch = 0;
    if( RES_ATTRSET_CHG == nWhich && SFX_ITEM_SET ==
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_ANCHOR, FALSE,
            (const SfxPoolItem**)&pAnch ))
        ;       // Beim GetItemState wird der AnkerPointer gesetzt !

    else if( RES_ANCHOR == nWhich )
    {
        //Ankerwechsel, ich haenge mich selbst um.
        //Es darf sich nicht um einen Wechsel des Ankertyps handeln,
        //dies ist nur ueber die SwFEShell moeglich.
        pAnch = (const SwFmtAnchor*)pNew;
    }

    if( pAnch )
    {
        ASSERT( pAnch->GetAnchorId() == GetFmt()->GetAnchor().GetAnchorId(),
                "Unzulaessiger Wechsel des Ankertyps." );

        //Abmelden, neuen Anker besorgen und 'dranhaengen.
        SwRect aOld( AddSpacesToFrm() );
        SwPageFrm *pOldPage = FindPageFrm();
        const SwFrm *pOldAnchor = GetAnchor();
        SwCntntFrm *pCntnt = (SwCntntFrm*)GetAnchor();
        GetAnchor()->RemoveFly( this );

        const BOOL bBodyFtn = (pCntnt->IsInDocBody() || pCntnt->IsInFtn());

        //Den neuen Anker anhand des NodeIdx suchen, am alten und
        //neuen NodeIdx kann auch erkannt werden, in welche Richtung
        //gesucht werden muss.
        const SwNodeIndex aNewIdx( pAnch->GetCntntAnchor()->nNode );
        SwNodeIndex aOldIdx( *pCntnt->GetNode() );

        //fix: Umstellung, ehemals wurde in der do-while-Schleife nach vorn bzw.
        //nach hinten gesucht; je nachdem wie welcher Index kleiner war.
        //Das kann aber u.U. zu einer Endlosschleife fuehren. Damit
        //wenigstens die Schleife unterbunden wird suchen wir nur in eine
        //Richtung. Wenn der neue Anker nicht gefunden wird koennen wir uns
        //immer noch vom Node einen Frame besorgen. Die Change, dass dies dann
        //der richtige ist, ist gut.
        const FASTBOOL bNext = aOldIdx < aNewIdx;
        while ( pCntnt && aOldIdx != aNewIdx )
        {
            do
            {   if ( bNext )
                    pCntnt = pCntnt->GetNextCntntFrm();
                else
                    pCntnt = pCntnt->GetPrevCntntFrm();
            } while ( pCntnt &&
                      !(bBodyFtn == (pCntnt->IsInDocBody() ||
                                     pCntnt->IsInFtn())) );
            if (pCntnt)
                aOldIdx = *pCntnt->GetNode();
        }
        if ( !pCntnt )
        {
            SwCntntNode *pNode = aNewIdx.GetNode().GetCntntNode();
            pCntnt = pNode->GetFrm( &pOldAnchor->Frm().Pos(), 0, FALSE );
            ASSERT( pCntnt, "Neuen Anker nicht gefunden" );
        }
        //Flys haengen niemals an einem Follow sondern immer am
        //Master, den suchen wir uns jetzt.
        const SwFlowFrm *pFlow = pCntnt;
        while ( pFlow->IsFollow() )
            pFlow = pFlow->FindMaster();
        pCntnt = (SwCntntFrm*)pFlow->GetFrm();

        //und schwupp angehaengt das teil...
        pCntnt->AppendFly( this );
        if ( pOldPage && pOldPage != FindPageFrm() )
            NotifyBackground( pOldPage, aOld, PREP_FLY_LEAVE );

        //Fix(3495)
        _InvalidatePos();
        InvalidatePage();
        SetNotifyBack();
    }
    else
        SwFlyFrm::Modify( pOld, pNew );
}

/*************************************************************************
|*
|*  SwFlyAtCntFrm::MakeAll()
|*
|*  Beschreibung        Bei einem Absatzgebunden Fly kann es durchaus sein,
|*      das der Anker auf die Veraenderung des Flys reagiert. Auf diese
|*      Reaktion hat der Fly natuerlich auch wieder zu reagieren.
|*      Leider kann dies zu Oszillationen fuehren z.b. Der Fly will nach
|*      unten, dadurch kann der Inhalt nach oben, der TxtFrm wird kleiner,
|*      der Fly muss wieder hoeher woduch der Text wieder nach unten
|*      verdraengt wird...
|*      Um derartige Oszillationen zu vermeiden, wird ein kleiner Positions-
|*      stack aufgebaut. Wenn der Fly ein Position erreicht, die er bereits
|*      einmal einnahm, so brechen wir den Vorgang ab. Um keine Risiken
|*      einzugehen, wird der Positionsstack so aufgebaut, dass er fuenf
|*      Positionen zurueckblickt.
|*      Wenn der Stack ueberlaeuft, wird ebenfalls abgebrochen.
|*      Der Abbruch fuer dazu, dass der Fly am Ende eine unguenste Position
|*      einnimmt. Damit es nicht durch einen wiederholten Aufruf von
|*      Aussen zu einer 'grossen Oszillation' kommen kann wird im Abbruch-
|*      fall das Attribut des Rahmens auf automatische Ausrichtung oben
|*      eingestellt.
|*
|*  Ersterstellung      MA 12. Nov. 92
|*  Letzte Aenderung    MA 20. Sep. 96
|*
|*************************************************************************/
//Wir brauchen ein Paar Hilfsklassen zur Kontrolle der Ozillation und ein paar
//Funktionen um die Uebersicht zu gewaehrleisten.

class SwOszControl
{
    static const SwFlyFrm *pStk1;
    static const SwFlyFrm *pStk2;
    static const SwFlyFrm *pStk3;
    static const SwFlyFrm *pStk4;
    static const SwFlyFrm *pStk5;

    const SwFlyFrm *pFly;
    Point aStk1, aStk2, aStk3, aStk4, aStk5;

public:
    SwOszControl( const SwFlyFrm *pFrm );
    ~SwOszControl();
    FASTBOOL ChkOsz();
    static FASTBOOL IsInProgress( const SwFlyFrm *pFly );
};
const SwFlyFrm *SwOszControl::pStk1 = 0;
const SwFlyFrm *SwOszControl::pStk2 = 0;
const SwFlyFrm *SwOszControl::pStk3 = 0;
const SwFlyFrm *SwOszControl::pStk4 = 0;
const SwFlyFrm *SwOszControl::pStk5 = 0;

SwOszControl::SwOszControl( const SwFlyFrm *pFrm ) :
    pFly( pFrm )
{
    if ( !SwOszControl::pStk1 )
        SwOszControl::pStk1 = pFly;
    else if ( !SwOszControl::pStk2 )
        SwOszControl::pStk2 = pFly;
    else if ( !SwOszControl::pStk3 )
        SwOszControl::pStk3 = pFly;
    else if ( !SwOszControl::pStk4 )
        SwOszControl::pStk4 = pFly;
    else if ( !SwOszControl::pStk5 )
        SwOszControl::pStk5 = pFly;
}

SwOszControl::~SwOszControl()
{
    if ( SwOszControl::pStk1 == pFly )
        SwOszControl::pStk1 = 0;
    else if ( SwOszControl::pStk2 == pFly )
        SwOszControl::pStk2 = 0;
    else if ( SwOszControl::pStk3 == pFly )
        SwOszControl::pStk3 = 0;
    else if ( SwOszControl::pStk4 == pFly )
        SwOszControl::pStk4 = 0;
    else if ( SwOszControl::pStk5 == pFly )
        SwOszControl::pStk5 = 0;
}

FASTBOOL IsInProgress( const SwFlyFrm *pFly )
{
    return SwOszControl::IsInProgress( pFly );
}

FASTBOOL SwOszControl::IsInProgress( const SwFlyFrm *pFly )
{
    if ( SwOszControl::pStk1 && !pFly->IsLowerOf( SwOszControl::pStk1 ) )
        return TRUE;
    if ( SwOszControl::pStk2 && !pFly->IsLowerOf( SwOszControl::pStk2 ) )
        return TRUE;
    if ( SwOszControl::pStk3 && !pFly->IsLowerOf( SwOszControl::pStk3 ) )
        return TRUE;
    if ( SwOszControl::pStk4 && !pFly->IsLowerOf( SwOszControl::pStk4 ) )
        return TRUE;
    if ( SwOszControl::pStk5 && !pFly->IsLowerOf( SwOszControl::pStk5 ) )
        return TRUE;
    return FALSE;
}

FASTBOOL SwOszControl::ChkOsz()
{
    FASTBOOL bRet = TRUE;
    Point aTmp = pFly->Frm().Pos();
    if( aTmp == Point() )
        aTmp.X() = 1;
    //Ist der Stack am Ende?
    if ( aStk1 != Point() )
        return TRUE;
    if ( aTmp != aStk1 && aTmp != aStk2 && aTmp != aStk3 &&
         aTmp != aStk4 && aTmp != aStk5 )
    {
        aStk1 = aStk2;
        aStk2 = aStk3;
        aStk3 = aStk4;
        aStk4 = aStk5;
        aStk5 = aTmp;
        bRet = FALSE;
    }
    return bRet;
}

void SwFlyAtCntFrm::MakeAll()
{
    if ( !SwOszControl::IsInProgress( this ) && !IsLocked() && !IsColLocked() &&
         GetPage() )
    {
        //Den Anker muessen wir zwischendurch natuerlich Formatieren, damit
        //Repaints usw. stimmen sollte er natuerlich trotzdem Invalid bleiben.
        //Jetzt Stufe 2: Damit Repaints stimmen muessen alle Frms wieder Invalidiert
        //werden, die unterwegs formatiert werden.
        //Dazu werden sie ein ein PrtArr eingetragen; die Frms mit CompletePaint
        //zu flaggen scheint mir hier das Mittel der Wahl.
        //(Vielleicht sollte es einmal die Moeglichkeit geben sie einfach mit
        //Paint zu flaggen; kein Formatieren, aber ein Paint-Aufruf, vor allem
        //wohl fuer TxtFrms geeignet.
        //Jetzt Stufe 3: einfach ein globales Flag und schon flaggen sie sich
        //selbst.
        bSetCompletePaintOnInvalidate = TRUE;
        SwOszControl aOszCntrl( this );

        if( GetAnchor()->IsInSct() )
        {
            SwSectionFrm *pSct = GetAnchor()->FindSctFrm();
            pSct->Calc();
        }

        GetAnchor()->Calc();
        SwFrm* pFooter = GetAnchor()->FindFooterOrHeader();
        if( pFooter && !pFooter->IsFooterFrm() )
            pFooter = NULL;
        FASTBOOL bOsz = FALSE;
        FASTBOOL bExtra = Lower() && Lower()->IsColumnFrm();

        do {
            Point aOldPos( Frm().Pos() );
            SwFlyFreeFrm::MakeAll();
            BOOL bPosChg = aOldPos != Frm().Pos();
            if( GetAnchor()->IsInSct() )
            {
                SwSectionFrm *pSct = GetAnchor()->FindSctFrm();
                pSct->Calc();
            }

            GetAnchor()->Calc();
            if ( aOldPos != Frm().Pos() || ( !GetValidPosFlag() &&
                 ( pFooter || bPosChg ) ) )
                bOsz = aOszCntrl.ChkOsz();
            if( bExtra && Lower() && !Lower()->GetValidPosFlag() )
            {  // Wenn ein mehrspaltiger Rahmen wg. Positionswechsel ungueltige
                // Spalten hinterlaesst, so drehen wir lieber hier eine weitere
                // Runde und formatieren unseren Inhalt via FormatWidthCols nochmal.
                _InvalidateSize();
                bExtra = FALSE; // Sicherhaltshalber gibt es nur eine Ehrenrunde.
            }
        } while ( !IsValid() && !bOsz );

        if ( bOsz )
        {
            SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
            pFmt->LockModify();
            SwFmtSurround aMain( pFmt->GetSurround() );
            // Im Notfall setzen wir automatisch positionierte Rahmen mit
            // Rekursion auf Durchlauf, das duerfte beruhigend wirken.
            if( IsAutoPos() && aMain.GetSurround() != SURROUND_THROUGHT )
            {
                aMain.SetSurround( SURROUND_THROUGHT );
                pFmt->SetAttr( aMain );
            }
            else
            {
                SwFmtVertOrient aOrient( pFmt->GetVertOrient() );
                aOrient.SetVertOrient( VERT_TOP );
                pFmt->SetAttr( aOrient );
                //Wenn der Rahmen auf "Kein Umlauf" steht, versuchen wir es mal
                //mit Seitenumlauf.
                if ( aMain.GetSurround() == SURROUND_NONE )
                {
                    aMain.SetSurround( SURROUND_PARALLEL );
                    pFmt->SetAttr( aMain );
                }
            }
            pFmt->UnlockModify();

            _InvalidatePos();
            SwFlyFreeFrm::MakeAll();
            GetAnchor()->Calc();
            if ( !GetValidPosFlag() )
            {
                SwFlyFreeFrm::MakeAll();
                GetAnchor()->Calc();
            }
            //Osz auf jeden fall zum Stehen bringen.
            bValidPos = bValidSize = bValidPrtArea = TRUE;
        }
        bSetCompletePaintOnInvalidate = FALSE;
    }

/* MA 18. Apr. 94: Bei Spalten kann der Fly durchaus Y-Maessig ueber dem
 * Anker stehen!
    ASSERT( Frm().Top() >= GetAnchor()->Frm().Top(),
            "Achtung: Rahmen auf Hoehenflug." );
*/

}

/*************************************************************************
|*
|*  FindAnchor() und Hilfsfunktionen.
|*
|*  Beschreibung:       Sucht ausgehend von pOldAnch einen Anker fuer
|*      Absatzgebundene Objekte.
|*      Wird beim Draggen von Absatzgebundenen Objekten zur Ankeranzeige sowie
|*      fuer Ankerwechsel benoetigt.
|*  Ersterstellung      MA 22. Jun. 93
|*  Letzte Aenderung    MA 30. Jan. 95
|*
|*************************************************************************/

class SwDistance
{
public:
    SwTwips nMain, nSub;
    SwDistance() { nMain = nSub = 0; }
    SwDistance& operator=( const SwDistance &rTwo )
        { nMain = rTwo.nMain; nSub = rTwo.nSub; return *this; }
    BOOL operator<( const SwDistance& rTwo )
        { return nMain < rTwo.nMain || ( nMain == rTwo.nMain && nSub &&
          rTwo.nSub && nSub < rTwo.nSub ); }
    BOOL operator<=( const SwDistance& rTwo )
        { return nMain < rTwo.nMain || ( nMain == rTwo.nMain && ( !nSub ||
          !rTwo.nSub || nSub <= rTwo.nSub ) ); }
};

const SwFrm * MA_FASTCALL lcl_CalcDownDist( SwDistance &rRet,
                                         const Point &rPt,
                                         const SwCntntFrm *pCnt )
{
    rRet.nSub = 0;
    //Wenn der Point direkt innerhalb des Cnt steht ist die Sache klar und
    //der Cntnt hat automatisch eine Entfernung von 0
    if ( pCnt->Frm().IsInside( rPt ) )
    {
        rRet.nMain = 0;
        return pCnt;
    }
    else
    {
        const SwLayoutFrm *pUp = pCnt->IsInTab() ? pCnt->FindTabFrm()->GetUpper() : pCnt->GetUpper();
        // einspaltige Bereiche muessen zu ihrem Upper durchschalten
        while( pUp->IsSctFrm() )
            pUp = pUp->GetUpper();
        //Dem Textflus folgen.
        if ( pCnt->IsInTab() && pUp->Frm().IsInside( rPt ) )
        {
            rRet.nMain = rPt.Y() - pCnt->Frm().Top();
            return pCnt;
        }
        else if ( pUp->Frm().IsInside( rPt ) )
        {
            rRet.nMain =  rPt.Y() - pCnt->Frm().Top();
            return pCnt;
        }
        else if ( rPt.X() <  pUp->Frm().Left() &&
                  rPt.Y() <= pUp->Frm().Bottom() )
        {
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, FALSE, pCnt );
            if ( !pLay || (pLay->Frm().Left() + pLay->Prt().Right()) < rPt.X() )
            {
                rRet.nMain = rPt.Y() - pCnt->Frm().Top();
                return pCnt;
            }
            else
                rRet.nMain = LONG_MAX;
        }
        else
        {
            rRet.nMain = (pUp->Frm().Top() + pUp->Prt().Bottom()) - pCnt->Frm().Top();

            const SwFrm *pPre = pCnt;
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, TRUE, pCnt );
            SwTwips nFrmTop, nPrtHeight;
            BOOL bSct;
            const SwSectionFrm *pSect = pUp->FindSctFrm();
            if( pSect )
            {
                rRet.nSub = rRet.nMain;
                rRet.nMain = 0;
            }
            if( pSect && !pSect->IsAnLower( pLay ) )
            {
                bSct = FALSE;
                const SwSectionFrm* pNxtSect = pLay ? pLay->FindSctFrm() : 0;
                if( pSect->IsAnFollow( pNxtSect ) )
                {
                    nFrmTop = pLay->Frm().Top();
                    nPrtHeight = pLay->Prt().Height();
                    pSect = pNxtSect;
                }
                else
                {
                    pLay = pSect->GetUpper();
                    nFrmTop = pSect->Frm().Bottom();
                    nPrtHeight = pLay->Frm().Top() + pLay->Prt().Top()
                                 + pLay->Prt().Height() - pSect->Frm().Top()
                                 - pSect->Frm().Height();
                    pSect = 0;
                }
            }
            else if( pLay )
            {
                nFrmTop = pLay->Frm().Top();
                nPrtHeight = pLay->Prt().Height();
                bSct = 0 != pSect;
            }
            while ( pLay && !pLay->Frm().IsInside( rPt ) &&
                    ( pLay->Frm().Top() <= rPt.Y() || pLay->IsInFly() ||
                      ( pLay->IsInSct() &&
                      pLay->FindSctFrm()->GetUpper()->Frm().Top() <= rPt.Y())) )
            {
                if ( pLay->IsFtnContFrm() )
                {
                    if ( !((SwLayoutFrm*)pLay)->Lower() )
                    {
                        SwFrm *pDel = (SwFrm*)pLay;
                        pDel->Cut();
                        delete pDel;
                        return pPre;
                    }
                    return 0;
                }
                else
                {
                    if( bSct || pSect )
                        rRet.nSub += nPrtHeight;
                    else
                        rRet.nMain += nPrtHeight;
                    pPre = pLay;
                    pLay = pLay->GetLeaf( MAKEPAGE_NONE, TRUE, pCnt );
                    if( pSect && !pSect->IsAnLower( pLay ) )
                    {   // If we're leaving a SwSectionFrm, the next Leaf-Frm
                        // is the part of the upper below the SectionFrm.
                        const SwSectionFrm* pNxtSect = pLay ?
                            pLay->FindSctFrm() : NULL;
                        bSct = FALSE;
                        if( pSect->IsAnFollow( pNxtSect ) )
                        {
                            nFrmTop = pLay->Frm().Top();
                            nPrtHeight = pLay->Prt().Height();
                            pSect = pNxtSect;
                        }
                        else
                        {
                            pLay = pSect->GetUpper();
                            nFrmTop = pSect->Frm().Bottom();
                            nPrtHeight = pLay->Frm().Top() + pLay->Prt().Top()
                                         + pLay->Prt().Height() - pSect->Frm().Top()
                                         - pSect->Frm().Height();
                            pSect = 0;
                        }
                    }
                    else if( pLay )
                    {
                        nFrmTop = pLay->Frm().Top();
                        nPrtHeight = pLay->Prt().Height();
                        bSct = 0 != pSect;
                    }
                }
            }
            if ( pLay )
            {
                if ( pLay->Frm().IsInside( rPt ) )
                {
                    if( bSct || pSect )
                        rRet.nSub += rPt.Y() - nFrmTop;
                    else
                        rRet.nMain += rPt.Y() - nFrmTop;
                }
                if ( pLay->IsFtnContFrm() && !((SwLayoutFrm*)pLay)->Lower() )
                {
                    SwFrm *pDel = (SwFrm*)pLay;
                    pDel->Cut();
                    delete pDel;
                    return 0;
                }
                return pLay;
            }
            else
                rRet.nMain = LONG_MAX;
        }
    }
    return 0;
}

//Bug 3985, optimierungsproblem, vergleiche auch trvlfrm.cxx lcl_FindCntnt()
#pragma optimize("e",off)

ULONG MA_FASTCALL lcl_FindCntDiff( const Point &rPt, const SwLayoutFrm *pLay,
                          const SwCntntFrm *& rpCnt,
                          const BOOL bBody, const BOOL bFtn )
{
    //Sucht unterhalb von pLay den dichtesten Cnt zum Point. Der Bezugspunkt
    //der Cntnts ist immer die linke obere Ecke.
    //Der Cnt soll moeglichst ueber dem Point liegen.

#ifdef DEBUG
    Point arPoint( rPt );
#endif

    rpCnt = 0;
    ULONG nDistance = ULONG_MAX;
    ULONG nNearest  = ULONG_MAX;
    const SwCntntFrm *pCnt = pLay->ContainsCntnt();

    while ( pCnt && (bBody != pCnt->IsInDocBody() || bFtn != pCnt->IsInFtn()))
    {
        pCnt = pCnt->GetNextCntntFrm();
        if ( !pLay->IsAnLower( pCnt ) )
            pCnt = 0;
    }
    const SwCntntFrm *pNearest = pCnt;
    if ( pCnt )
    {
        do
        {
            //Jetzt die Entfernung zwischen den beiden Punkten berechnen.
            //'Delta' X^2 + 'Delta'Y^2 = 'Entfernung'^2
            ULONG dX = Max( pCnt->Frm().Left(), rPt.X() ) -
                       Min( pCnt->Frm().Left(), rPt.X() ),
                  dY = Max( pCnt->Frm().Top(), rPt.Y() ) -
                       Min( pCnt->Frm().Top(), rPt.Y() );
            BigInt dX1( dX ), dY1( dY );
            dX1 *= dX1; dY1 *= dY1;
            const ULONG nDiff = ::SqRt( dX1 + dY1 );
            if ( pCnt->Frm().Top() <= rPt.Y() )
            {
                if ( nDiff < nDistance )
                {   //Der ist dichter dran
                    nDistance = nNearest = nDiff;
                    rpCnt = pNearest = pCnt;
                }
            }
            else if ( nDiff < nNearest )
            {
                nNearest = nDiff;
                pNearest = pCnt;
            }
            pCnt = pCnt->GetNextCntntFrm();
            while ( pCnt &&
                    (bBody != pCnt->IsInDocBody() || bFtn != pCnt->IsInFtn()))
                pCnt = pCnt->GetNextCntntFrm();

        }  while ( pCnt && pLay->IsAnLower( pCnt ) );
    }
    if ( nDistance == ULONG_MAX )
    {   rpCnt = pNearest;
        return nNearest;
    }
    return nDistance;
}

#pragma optimize("e",on)

const SwCntntFrm * MA_FASTCALL lcl_FindCnt( const Point &rPt, const SwCntntFrm *pCnt,
                                  const BOOL bBody, const BOOL bFtn )
{
    //Sucht ausgehen von pCnt denjenigen CntntFrm, dessen linke obere
    //Ecke am dichtesten am Point liegt.
    //Liefert _immer_ einen CntntFrm zurueck.

    //Zunaechst wird versucht den dichtesten Cntnt innerhalt derjenigen
    //Seite zu suchen innerhalb derer der Cntnt steht.
    //Ausgehend von der Seite muessen die Seiten in beide
    //Richtungen beruecksichtigt werden.
    //Falls moeglich wird ein Cntnt geliefert, dessen Y-Position ueber der
    //des Point sitzt.
    const SwCntntFrm  *pRet, *pNew;
    const SwLayoutFrm *pLay = pCnt->FindPageFrm();
    ULONG nDist;

    nDist = ::lcl_FindCntDiff( rPt, pLay, pNew, bBody, bFtn );
    if ( pNew )
        pRet = pNew;
    else
    {   pRet  = pCnt;
        nDist = ULONG_MAX;
    }
    const SwCntntFrm *pNearest = pRet;
    ULONG nNearest = nDist;

    if ( pLay )
    {
        const SwLayoutFrm *pPge = pLay;
        ULONG nOldNew = ULONG_MAX;
        for ( USHORT i = 0; pPge->GetPrev() && (i < 3); ++i )
        {
            pPge = (SwLayoutFrm*)pPge->GetPrev();
            const ULONG nNew = ::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFtn );
            if ( nNew < nDist )
            {
                if ( pNew->Frm().Top() <= rPt.Y() )
                {
                    pRet = pNearest = pNew;
                    nDist = nNearest = nNew;
                }
                else if ( nNew < nNearest )
                {
                    pNearest = pNew;
                    nNearest = nNew;
                }
            }
            else if ( nOldNew != ULONG_MAX && nNew > nOldNew )
                break;
            else
                nOldNew = nNew;

        }
        pPge = pLay;
        nOldNew = ULONG_MAX;
        for ( USHORT j = 0; pPge->GetNext() && (j < 3); ++j )
        {
            pPge = (SwLayoutFrm*)pPge->GetNext();
            const ULONG nNew = ::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFtn );
            if ( nNew < nDist )
            {
                if ( pNew->Frm().Top() <= rPt.Y() )
                {
                    pRet = pNearest = pNew;
                    nDist = nNearest = nNew;
                }
                else if ( nNew < nNearest )
                {
                    pNearest = pNew;
                    nNearest = nNew;
                }
            }
            else if ( nOldNew != ULONG_MAX && nNew > nOldNew )
                break;
            else
                nOldNew = nNew;
        }
    }
    if ( (pRet->Frm().Top() > rPt.Y()) )
        return pNearest;
    else
        return pRet;
}

void lcl_PointToPrt( Point &rPoint, const SwFrm *pFrm )
{
    SwRect aTmp( pFrm->Prt() );
    aTmp += pFrm->Frm().Pos();
    if ( rPoint.X() < aTmp.Left() )
        rPoint.X() = aTmp.Left();
    else if ( rPoint.X() > aTmp.Right() )
        rPoint.X() = aTmp.Right();
    if ( rPoint.Y() < aTmp.Top() )
        rPoint.Y() = aTmp.Top();
    else if ( rPoint.Y() > aTmp.Bottom() )
        rPoint.Y() = aTmp.Bottom();

}

const SwCntntFrm *FindAnchor( const SwFrm *pOldAnch, const Point &rNew,
                              const BOOL bBodyOnly )
{
    //Zu der angegebenen DokumentPosition wird der dichteste Cnt im
    //Textfluss gesucht. AusgangsFrm ist der uebergebene Anker.
    const SwCntntFrm *pCnt;
    if ( pOldAnch->IsCntntFrm() )
        pCnt = (const SwCntntFrm*)pOldAnch;
    else
    {   Point aTmp( rNew );
        SwLayoutFrm *pTmpLay = (SwLayoutFrm*)pOldAnch;
        if( pTmpLay->IsRootFrm() )
        {
            SwRect aTmpRect( aTmp, Size(0,0) );
            pTmpLay = (SwLayoutFrm*)::FindPage( aTmpRect, pTmpLay->Lower() );
        }
        pCnt = pTmpLay->GetCntntPos( aTmp, FALSE, bBodyOnly );
    }

    //Beim Suchen darauf achten, dass die Bereiche sinnvoll erhalten
    //bleiben. D.h. in diesem Fall nicht in Header/Footer hinein und
    //nicht aus Header/Footer hinaus.
    const BOOL bBody = pCnt->IsInDocBody() || bBodyOnly;
    const BOOL bFtn  = !bBodyOnly && pCnt->IsInFtn();

    Point aNew( rNew );
    if ( bBody )
    {
        //#38848 Vom Seitenrand in den Body ziehen.
        const SwFrm *pPage = pCnt->FindPageFrm();
        ::lcl_PointToPrt( aNew, pPage->GetUpper() );
        SwRect aTmp( aNew, Size( 0, 0 ) );
        pPage = ::FindPage( aTmp, pPage );
        ::lcl_PointToPrt( aNew, pPage );
    }

    if ( pCnt->IsInDocBody() == bBody && pCnt->Frm().IsInside( aNew ) )
        return pCnt;
    else if ( pOldAnch->IsInDocBody() || pOldAnch->IsPageFrm() )
    {
        //Vielleicht befindet sich der gewuenschte Anker ja auf derselben
        //Seite wie der aktuelle Anker.
        //So gibt es kein Problem mit Spalten.
        Point aTmp( aNew );
        const SwCntntFrm *pTmp = pCnt->FindPageFrm()->
                                        GetCntntPos( aTmp, FALSE, TRUE, FALSE );
        if ( pTmp && pTmp->Frm().IsInside( aNew ) )
            return pTmp;
    }

    //Ausgehend vom Anker suche ich jetzt in beide Richtungen bis ich
    //den jeweils dichtesten gefunden habe.
    //Nicht die direkte Entfernung ist relevant sondern die Strecke die
    //im Textfluss zurueckgelegt werden muss.
    const SwCntntFrm *pUpLst;
    const SwCntntFrm *pUpFrm = pCnt;
    SwDistance nUp, nUpLst;
    ::lcl_CalcDownDist( nUp, aNew, pUpFrm );
    SwDistance nDown = nUp;
    BOOL bNegAllowed = TRUE;//Einmal aus dem negativen Bereich heraus lassen.
    do
    {
        pUpLst = pUpFrm; nUpLst = nUp;
        pUpFrm = pUpLst->GetPrevCntntFrm();
        while ( pUpFrm &&
                (bBody != pUpFrm->IsInDocBody() || bFtn != pUpFrm->IsInFtn()))
            pUpFrm = pUpFrm->GetPrevCntntFrm();
        if ( pUpFrm )
        {
            ::lcl_CalcDownDist( nUp, aNew, pUpFrm );
            //Wenn die Distanz innnerhalb einer Tabelle waechst, so lohnt es
            //sich weiter zu suchen.
            if ( pUpLst->IsInTab() && pUpFrm->IsInTab() )
            {
                while ( pUpFrm && ((nUpLst < nUp && pUpFrm->IsInTab()) ||
                        bBody != pUpFrm->IsInDocBody()) )
                {
                    pUpFrm = pUpFrm->GetPrevCntntFrm();
                    if ( pUpFrm )
                        ::lcl_CalcDownDist( nUp, aNew, pUpFrm );
                }
            }
        }
        if ( !pUpFrm )
            nUp.nMain = LONG_MAX;
        if ( nUp.nMain >= 0 )
        {
            bNegAllowed = FALSE;
            if ( nUpLst.nMain < 0 ) //nicht den falschen erwischen, wenn der Wert
                                    //gerade von negativ auf positiv gekippt ist.
            {   pUpLst = pUpFrm;
                nUpLst = nUp;
            }
        }
    } while ( pUpFrm && ( ( bNegAllowed && nUp.nMain < 0 ) || ( nUp <= nUpLst &&
              ( nUp.nMain != LONG_MAX && nUpLst.nMain != LONG_MAX ) ) ) );

    const SwCntntFrm *pDownLst;
    const SwCntntFrm *pDownFrm = pCnt;
    SwDistance nDownLst;
    if ( nDown.nMain < 0 )
        nDown.nMain = LONG_MAX;
    do
    {
        pDownLst = pDownFrm; nDownLst = nDown;
        pDownFrm = pDownLst->GetNextCntntFrm();
        while ( pDownFrm &&
                (bBody != pDownFrm->IsInDocBody() || bFtn != pDownFrm->IsInFtn()))
            pDownFrm = pDownFrm->GetNextCntntFrm();
        if ( pDownFrm )
        {
            ::lcl_CalcDownDist( nDown, aNew, pDownFrm );
            if ( nDown.nMain < 0 )
                nDown.nMain = LONG_MAX;
            //Wenn die Distanz innnerhalb einer Tabelle waechst, so lohnt es
            //sich weiter zu suchen.
            if ( pDownLst->IsInTab() && pDownFrm->IsInTab() )
            {
                while ( pDownFrm && ( ( nDown.nMain != LONG_MAX && nDownLst < nDownLst
                        && pDownFrm->IsInTab()) || bBody != pDownFrm->IsInDocBody() ) )
                {
                    pDownFrm = pDownFrm->GetNextCntntFrm();
                    if ( pDownFrm )
                        ::lcl_CalcDownDist( nDown, aNew, pDownFrm );
                    if ( nDown.nMain < 0 )
                        nDown.nMain = LONG_MAX;
                }
            }
        }
        if ( !pDownFrm )
            nDown.nMain = LONG_MAX;

    } while ( pDownFrm && nDown <= nDownLst &&
              nDown.nMain != LONG_MAX && nDownLst.nMain != LONG_MAX );

    //Wenn ich in beide Richtungen keinen gefunden habe, so suche ich mir
    //denjenigen Cntnt dessen linke obere Ecke dem Point am naechsten liegt.
    //Eine derartige Situation tritt z.b. auf, wenn der Point nicht im Text-
    //fluss sondern in irgendwelchen Raendern steht.
    if ( nDownLst.nMain == LONG_MAX && nUpLst.nMain == LONG_MAX )
        return ::lcl_FindCnt( aNew, pCnt, bBody, bFtn );
    else
        return nDownLst < nUpLst ? pDownLst : pUpLst;
}

/*************************************************************************
|*
|*  SwFlyAtCntFrm::SetAbsPos()
|*
|*  Ersterstellung      MA 22. Jun. 93
|*  Letzte Aenderung    MA 11. Sep. 98
|*
|*************************************************************************/

void SwFlyAtCntFrm::SetAbsPos( const Point &rNew )
{
    SwPageFrm *pOldPage = FindPageFrm();
    const SwRect aOld( AddSpacesToFrm() );
    SwCntntFrm *pCnt = (SwCntntFrm*)::FindAnchor( GetAnchor(), rNew );
    if( pCnt->IsProtected() )
        pCnt = (SwCntntFrm*)GetAnchor();

    Point aNew( rNew );
    SwPageFrm *pPage = 0;
    if ( pCnt->IsInDocBody() )
    {
        //#38848 Vom Seitenrand in den Body ziehen.
        pPage = pCnt->FindPageFrm();
        ::lcl_PointToPrt( aNew, pPage->GetUpper() );
        SwRect aTmp( aNew, Size( 0, 0 ) );
        pPage = (SwPageFrm*)::FindPage( aTmp, pPage );
        ::lcl_PointToPrt( aNew, pPage );
    }

    //RelPos einstellen, nur auf Wunsch invalidieren.
    //rNew ist eine Absolute Position. Um die RelPos korrekt einzustellen
    //muessen wir uns die Entfernung von rNew zum Anker im Textfluss besorgen.
//!!!!!Hier kann Optimiert werden: FindAnchor koennte die RelPos mitliefern!
    const SwFrm *pFrm = 0;
    SwTwips nY;
    if ( pCnt->Frm().IsInside( aNew ) )
        nY = rNew.Y() - pCnt->Frm().Top();
    else
    {
        SwDistance aDist;
        pFrm = ::lcl_CalcDownDist( aDist, aNew, pCnt );
        nY = aDist.nMain + aDist.nSub;
    }

    SwTwips nX = 0;

    if ( pCnt->IsFollow() )
    {
        //Flys haengen niemals an einem Follow sondern immer am
        //Master, den suchen wir uns jetzt.
        const SwCntntFrm *pOriginal = pCnt;
        const SwCntntFrm *pFollow = pCnt;
        while ( pCnt->IsFollow() )
        {
            do
            {   pCnt = pCnt->GetPrevCntntFrm();
            } while ( pCnt->GetFollow() != pFollow );
            pFollow = pCnt;
        }
        SwTwips nDiff = 0;
        do
        {   const SwFrm *pUp = pFollow->GetUpper();
            nDiff += pUp->Prt().Height() - pFollow->GetRelPos().Y();
            pFollow = pFollow->GetFollow();
        } while ( pFollow != pOriginal );
        nY += nDiff;
        nX = pCnt->Frm().Left() - pOriginal->Frm().Left();
    }

    if ( nY == LONG_MAX )
        nY = rNew.Y() - pCnt->Frm().Top();

    if ( !pFrm )
        nX += rNew.X() - pCnt->Frm().Left();
    else
        nX = rNew.X() - pFrm->Frm().Left();

    GetFmt()->GetDoc()->StartUndo( UNDO_START );

    if( pCnt != GetAnchor() || ( IsAutoPos() && pCnt->IsTxtFrm() &&
                                  GetFmt()->GetDoc()->IsHTMLMode() ) )
    {
        //Das Ankerattribut auf den neuen Cnt setzen.
        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
        SwFmtAnchor aAnch( pFmt->GetAnchor() );
        SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
        if( IsAutoPos() && pCnt->IsTxtFrm() )
        {
            SwCrsrMoveState eTmpState( MV_SETONLYTEXT );
            Point aPt( rNew );
            if( pCnt->GetCrsrOfst( pPos, aPt, &eTmpState )
                && pPos->nNode == *pCnt->GetNode() )
            {
                aLastCharRect.Height( 0 );
                if( REL_CHAR == pFmt->GetVertOrient().GetRelationOrient() )
                    nY = LONG_MAX;
                if( REL_CHAR == pFmt->GetHoriOrient().GetRelationOrient() )
                    nX = LONG_MAX;
            }
            else
            {
                pPos->nNode = *pCnt->GetNode();
                pPos->nContent.Assign( pCnt->GetNode(), 0 );
            }
        }
        else
        {
            pPos->nNode = *pCnt->GetNode();
            pPos->nContent.Assign( pCnt->GetNode(), 0 );
        }
        pFmt->GetDoc()->SetAttr( aAnch, *pFmt );
    }
    else if ( pPage && pPage != GetPage() )
        GetPage()->MoveFly( this, pPage );

    const Point aRelPos( nX, nY );
    ChgRelPos( aRelPos );

    GetFmt()->GetDoc()->EndUndo( UNDO_END );

    if ( pOldPage != FindPageFrm() )
        ::Notify_Background( GetVirtDrawObj(), pOldPage, aOld, PREP_FLY_LEAVE,
                             FALSE );
}

/*************************************************************************
|*
|*  SwFlyAtCntFrm::MakeFlyPos()
|*
|*  Beschreibung:
|*
|*      virtueller Anker: Der virtuelle Anker eines Flys ist der Anker selbst
|*                        oder einer seiner Follows. Es ist genau derjenige
|*                        Cntnt, der dem Fly aktuell am naechsten liegt,
|*                        genauer der aktuellen relativen Position des Fly
|*                        (siehe auch VertPos, Fix). Es wird nur die
|*                        vertikale Entfernung gemessen.
|*                        Der virtuelle Anker fuer die Horizontale Ausrichtung
|*                        muss nicht ein CntntFrm sein, denn wenn der Fly
|*                        z.B. ueber einer leeren Spalte steht, so muss eben
|*                        der LayoutFrm als virtueller Anker dienen, der im
|*                        Textfluss des Ankers liegt.
|*
|*      HoriPos:
|*          - Automatisch: Die automatische Ausrichtung orientiert sich
|*            an einem SwFrm der folgendermassen ermittelt wird: Abhaengig
|*            vom Attriut und ausgehend vom virtuellen Anker wird der
|*            Bezugsframe gesucht (CntntFrm, LayoutFrm).
|*          - Fix: Der Wert der relativen Entfernung aus dem Attribut ist
|*                 die relative Entfernung vom virtuellen Anker.
|*      VertPos:
|*          - Automatisch: Die automatische Ausrichtung orientiert sich immer
|*            am virtuellen Anker.
|*          - Fix: Der Fly muss nicht in der Umgebung untergebracht sein, in
|*                 der sein Anker steht; er folgt aber stets dem Textfluss dem
|*                 der Anker folgt. Geclippt (Drawing) wird der Fly am Rootfrm.
|*                 Damit die erstgenannte Bedingung erreicht wird, wird der
|*                 Fly ggf. entsprechend verschoben. Dabei bleibt die relative
|*                 Position des Attributes erhalten, die tatsaechliche relative
|*                 Position verhaelt sich zu der des Attributes etwa wie ein
|*                 Teleskoparm. Der Betrag der relativen Position ist die
|*                 Entfernung zur AbsPos des Ankers im Textfluss.
|*
|*      Es wird immer zuerst die vertikale Position bestimmt, denn erst dann
|*      steht der virtuelle Anker fest.
|*      Die tatsaechliche relative Position (Member aRelPos) ist immer die
|*      die Entfernung zum Anker - sie muss also nicht mit den im Attribut
|*      angegebenen Werten uebereinstimmen, denn diese geben die Entfernung
|*      'im Textfluss' an.
|*
|*  Ersterstellung      MA 19. Nov. 92
|*  Letzte Aenderung    MA 14. Nov. 96
|*
|*************************************************************************/

inline void ValidateSz( SwFrm *pFrm )
{
    if ( pFrm )
        pFrm->bValidSize = TRUE;
}

void DeepCalc( const SwFrm *pFrm )
{
    if( pFrm->IsSctFrm() )
        return;
    USHORT nCnt = 0;

    FASTBOOL bContinue = FALSE;
    do
    {   if ( ++nCnt == 10 )
        {
            ASSERT( !nCnt, "DeepCalc: Loop detected1?" );
            break;
        }

        const FASTBOOL bSetComplete = !pFrm->IsValid();
        const SwRect aOldFrm( pFrm->Frm() );
        const SwRect aOldPrt( pFrm->Prt() );

        const SwFrm *pUp = pFrm->GetUpper();
        if ( pUp )
        {
            //Nicht weiter wenn der Up ein Fly mit Spalten ist.
            if( ( !pUp->IsFlyFrm() || !((SwLayoutFrm*)pUp)->Lower() ||
                 !((SwLayoutFrm*)pUp)->Lower()->IsColumnFrm() ) &&
                 !pUp->IsSctFrm() )
            {
                const Point aPt( pUp->Frm().Pos() );
                ::DeepCalc( pUp );
                bContinue = aPt != pUp->Frm().Pos();
            }
        }
        else
            pUp = pFrm;

        pFrm->Calc();
        if ( bSetComplete && (aOldFrm != pFrm->Frm() || aOldPrt != pFrm->Prt()))
            pFrm->SetCompletePaint();

//      bContinue = !pUp->IsValid();
        if ( pUp->IsFlyFrm() )
        {
            if ( ((SwFlyFrm*)pUp)->IsLocked() ||
                 (((SwFlyFrm*)pUp)->IsFlyAtCntFrm() &&
                  SwOszControl::IsInProgress( (const SwFlyFrm*)pUp )) )
            {
                bContinue = FALSE;
            }
        }
    } while ( bContinue );
}

//Ermittlung des virtuellen Ankers fuer die Positionierung.
//Dieser ist entweder der Anker selbst oder einer seiner Follows.

const SwCntntFrm *GetVirtualAnchor( const SwFlyAtCntFrm *pFly, xub_StrLen nOfs )
{
    const SwTxtFrm *pAct = (const SwTxtFrm*)pFly->GetAnchor();
    const SwTxtFrm* pTmp;
    do
    {
        pTmp = pAct;
        pAct = pTmp->GetFollow();
    }
    while( pAct && nOfs >= pAct->GetOfst() );
    return pTmp;
}

//Ermittlung des virtuellen Ankers, an dem sich die horizontale Ausrichtung
//orientieren muss.
//pAssumed enthaelt entweder bereits den Anker (Es ist dann der Anker des
//Flys oder einer seiner Follows) oder die Umgebung die der Orientierung,
//mangels einer besseren Moeglichkeit, dienen muss.

const SwFrm *GetVirtualHoriAnchor( const SwFrm *pAssumed, const SwFlyFrm *pFly )
{
    const SwFrm *pRet = pAssumed;

    if ( !pRet->IsCntntFrm() )
    {   //Wenn es Lower gibt, die selbst der Anker des Fly oder ein Follow
        //desselben sind, so wird derjenige ausgewaehlt, der der aktuellen
        //absoluten vertikalen Position des Fly am naechsten steht.
        //Gibt es keinen, so bleib es bei pAssumed
        const SwFrm *pFlow = ((SwLayoutFrm*)pRet)->Lower();
        SwTwips nCntDiff = LONG_MAX;
        while ( pFlow )
        {
            if ( pFlow->IsCntntFrm() &&
                 ((SwCntntFrm*)pFly->GetAnchor())->IsAnFollow( (SwCntntFrm*)pFlow ) )
            {
                SwTwips nDiff = pFly->Frm().Top() - pFlow->Frm().Top();
                if ( (nDiff = Abs(nDiff)) < nCntDiff )
                {
                    pRet = pFlow;           //Der ist dichter dran
                    nCntDiff = nDiff;
                }
            }
            pFlow = pFlow->GetNext();
        }
    }
    return pRet;

}

void SwFlyAtCntFrm::AssertPage()
{
    //Prueft ob der Fly an der Seite haengt, auf der er steht, falls nicht
    //wird er umgehaengt. Zur Pruefung wird nur die vertikale Ausrichtung
    //herangezogen.

    SwPageFrm *pNewPage = FindPageFrm();
    SwPageFrm *pMyPage  = pNewPage;
    BOOL bSuperfluous = FALSE;

    //#45516# Ausnahmebehandlung. Eine Tabelle ist zu gross und haengt aus der
    //Seite heraus. Der Rahmen kann dann zwar richtig bei seinem Anker stehen,
    //Positionsmaessig aber ueber der naechsten Seite haengen. Damit das dann
    //noch halbwegs brauchbar gedruckt werden kann (HTML) und der Rahmen nicht
    //wirr in der Gegend gepaintet wird, wird der Rahmen bei der Seite verankert,
    //auf der auch sein Anker sitzt.
    if ( GetAnchor()->GetValidSizeFlag() &&
         Frm().Top() >= GetAnchor()->Frm().Top() &&
         Frm().Top() < GetAnchor()->Frm().Bottom() )
    {
        pNewPage = GetAnchor()->FindPageFrm();
    }
    else
    {
        BOOL bFound = FALSE;
        const BOOL bFtn = GetAnchor()->IsInFtn();
        int nDir = INT_MAX; // 1 == Forward, 2 == Backward.
        while ( !bFound )
        {
            pNewPage->Calc();
            if ( Frm().Top() < pNewPage->Frm().Top() && pNewPage->GetPrev() )
            {
                pNewPage = (SwPageFrm*)pNewPage->GetPrev();
                if ( nDir == 2 )
                {
                    bFound = TRUE;
                    pNewPage = GetAnchor()->FindPageFrm();
                }
                else
                    nDir = 1;
            }
            else if ( Frm().Top() > pNewPage->Frm().Bottom() )
            {
                if ( nDir == 1 )
                {
                    bFound = TRUE;
                    pNewPage = GetAnchor()->FindPageFrm();
                }
                else
                {
                    nDir = 2;
                    if ( !pNewPage->GetNext() )
                    {
                        pNewPage->GetLeaf( bFtn ? MAKEPAGE_NONE : MAKEPAGE_APPEND,
                                            TRUE, GetAnchor());
                        bSuperfluous = TRUE;
                    }
                    if ( pNewPage->GetNext() )
                    {
                        pNewPage = (SwPageFrm*)pNewPage->GetNext();
                        if( pNewPage->IsEmptyPage() )
                        {
                            if( pNewPage->GetNext() )
                                pNewPage = (SwPageFrm*)pNewPage->GetNext();
                            else
                            {
                                bFound = TRUE;
                                pNewPage = (SwPageFrm*)pNewPage->GetPrev();
                            }
                        }
                    }
                    else
                        bFound = TRUE;
                }
            }
            else
                bFound = TRUE;
        }
    }

    if ( pMyPage != pNewPage )
    {
        ASSERT( IsLocked(), "AssertPage: Unlocked Frame??" );
        pMyPage->MoveFly( this, pNewPage );
        if ( bSuperfluous && pMyPage->GetPhyPageNum() > pNewPage->GetPhyPageNum() )
            ((SwRootFrm*)pNewPage->GetUpper())->SetSuperfluous();
    }

}

BOOL MA_FASTCALL lcl_IsMoveable( SwFlyFrm *pFly, SwLayoutFrm *pLay )
{
    //Waere der Anker auch in der neuen Umgebung noch moveable?
    BOOL bRet;
    SwLayoutFrm *pUp = pFly->GetAnchor()->GetUpper();
    SwFrm *pNext = pFly->GetAnchor()->GetNext();
    pFly->GetAnchor()->Remove();
    pFly->GetAnchor()->InsertBefore( pLay, pLay->Lower() );
    bRet = pFly->GetAnchor()->IsMoveable();
    pFly->GetAnchor()->Remove();
    pFly->GetAnchor()->InsertBefore( pUp, pNext );
    return bRet;

}

// Wer weicht wem aus bzw. welcher Bereich ist "linker"/"rechter" als welcher?
BOOL MA_FASTCALL lcl_Minor( SwRelationOrient eRelO, SwRelationOrient eRelO2,
    BOOL bLeft )
{
    // Die Ausweichreihenfolge der SwRelationOrient-Enums bei linker Ausrichtung
    static USHORT __READONLY_DATA aLeft[ LAST_ENUM_DUMMY ] =
        { 5, 6, 0, 1, 8, 4, 7, 2, 3 };
    // Die Ausweichreihenfolge der SwRelationOrient-Enums Ausrichtung rechts
    static USHORT __READONLY_DATA aRight[ LAST_ENUM_DUMMY ] =
        { 5, 6, 0, 8, 1, 7, 4, 2, 3 };
    // Hier wird z.B. entschieden, dass ein Rahmen im Absatzbereich
    // einem Rahmen im Seitentextbereich ausweicht usw.
    if( bLeft )
        return aLeft[ eRelO ] >= aLeft[ eRelO2 ];
    return aRight[ eRelO ] >= aRight[ eRelO2 ];
}

void SwFlyAtCntFrm::MakeFlyPos()
{
    if ( !bValidPos )
    {
        const SwFrm* pFooter = GetAnchor()->FindFooterOrHeader();
        if( pFooter && !pFooter->IsFooterFrm() )
            pFooter = NULL;
        const FASTBOOL bBrowse = GetAnchor()->IsInDocBody() && !GetAnchor()->IsInTab() ?
                                     GetFmt()->GetDoc()->IsBrowseMode() : FALSE;
        FASTBOOL bInvalidatePage = FALSE;
        SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
        const SvxLRSpaceItem &rLR = pFmt->GetLRSpace();
        const SvxULSpaceItem &rUL = pFmt->GetULSpace();
        const FASTBOOL bNoSurround
            = pFmt->GetSurround().GetSurround() == SURROUND_NONE;
        BOOL bGrow =
            !GetAnchor()->IsInTab() || !pFmt->GetFrmSize().GetHeightPercent();

        for (;;)
        {
        bValidPos = TRUE;
        if( !pFooter )
            ::DeepCalc( GetAnchor() );
        bValidPos = TRUE;

        //Die Werte in den Attributen muessen ggf. upgedated werden,
        //deshalb werden hier Attributinstanzen und Flags benoetigt.
        SwFmtVertOrient aVert( pFmt->GetVertOrient() );
        SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
        BOOL bVertChgd = FALSE,
             bHoriChgd = FALSE,
             bMoveable = GetAnchor()->IsMoveable();

        //Wird waehrend der Berechnung der vertikalen Position benutzt
        //und enthaelt hinterher den Frm, an dem sich die horizontale
        //Positionierung orientieren muss.
        const SwFrm *pOrient = GetAnchor();

        // Dies wird der Frame, der das Zeichen eines am Zeichen gebundenen
        // Rahmens enthaelt.
        const SwFrm *pAutoOrient = pOrient;

        SwRect *pAutoPos;
        if( FLY_AUTO_CNTNT == pFmt->GetAnchor().GetAnchorId() )
        {
            const SwFmtAnchor& rAnch = pFmt->GetAnchor();
            if( !aLastCharRect.Height() &&
                !((SwTxtFrm*)GetAnchor())->GetAutoPos( aLastCharRect,
                                                    *rAnch.GetCntntAnchor() ) )
                return;
            pAutoPos = &aLastCharRect;
            pAutoOrient = ::GetVirtualAnchor( this, rAnch.GetCntntAnchor()->
                                              nContent.GetIndex() );
        }
        else
            pAutoPos = NULL;

        //Horizontale und vertikale Positionen werden getrennt berechnet.
        //Sie koennen jeweils Fix oder Variabel sein.

        //Zuerst die vertikale Position, damit feststeht auf welcher Seite
        //bzw. in welchen Upper sich der Fly befindet.
        if ( aVert.GetVertOrient() != VERT_NONE )
        {
            pOrient = pAutoOrient;
            if( !pFooter )
                ::DeepCalc( pOrient );
            SwTwips nHeight, nAdd;
            if ( aVert.GetRelationOrient() == PRTAREA )
            {
                nHeight = pOrient->Prt().Height();
                nAdd = pOrient->Prt().Top();
            }
            else if( pAutoPos && REL_CHAR == aVert.GetRelationOrient() )
            {
                nHeight = pAutoPos->Height();
                nAdd = pAutoPos->Top() - pAutoOrient->Frm().Top();
            }
            else
            {   nHeight = pOrient->Frm().Height();
                nAdd = 0;
            }
            if ( aVert.GetVertOrient() == VERT_CENTER )
                aRelPos.Y() = (nHeight / 2) - (aFrm.Height() / 2);
            else if ( aVert.GetVertOrient() == VERT_BOTTOM )
            {
                if( bNoSurround )
                    aRelPos.Y() = nHeight + rUL.GetUpper();
                else
                    aRelPos.Y() = nHeight - (aFrm.Height() + rUL.GetLower());
            }
            else  if( pAutoPos && aVert.GetVertOrient() == VERT_CHAR_BOTTOM )
                aRelPos.Y() = nHeight + rUL.GetUpper();
            else
                aRelPos.Y() = rUL.GetUpper();
            aRelPos.Y() += nAdd;
            SwTwips nBot = pOrient->Frm().Top() + aRelPos.Y() + Frm().Height()
                           - pOrient->GetUpper()->Frm().Top()
                           - pOrient->GetUpper()->Prt().Bottom() - 1;
            if( nBot > 0 )
                aRelPos.Y() -= nBot;
            aRelPos.Y() = Max( aRelPos.Y(), 0L );
            //Da die relative Position immer zum Anker relativ ist, muss dessen
            //Entfernung zum virtuellen Anker aufaddiert werden.
            if ( GetAnchor() != pOrient )
                aRelPos.Y() += pOrient->Frm().Top() - GetAnchor()->Frm().Top();

            if ( aRelPos.Y() != aVert.GetPos() )
            {   aVert.SetPos( aRelPos.Y() );
                bVertChgd = TRUE;
            }
        }

        pOrient = aVert.GetVertOrient() == VERT_NONE ?
                  GetAnchor()->GetUpper() : pAutoOrient->GetUpper();
        if( !pFooter )
            ::DeepCalc( pOrient );

        SwTwips nRelDiff = 0;
        if ( aVert.GetVertOrient() == VERT_NONE )
        {
            SwTwips nRel;
            if( pAutoPos && REL_CHAR == aVert.GetRelationOrient() )
            {
                nRel = pAutoPos->Bottom() - pAutoOrient->Frm().Top() + 1
                       - aVert.GetPos();
                if( pAutoOrient != GetAnchor() )
                {
                    SwTxtFrm* pTmp = (SwTxtFrm*)GetAnchor();
                    nRel -= pTmp->GetRelPos().Y();
                    while( pTmp != pAutoOrient )
                    {
                        nRel += pTmp->GetUpper()->Prt().Height();
                        pTmp = pTmp->GetFollow();
                    }
                }
            }
            else
                nRel = aVert.GetPos();

            // Einen einspaltigen Bereich koennen wir getrost ignorieren,
            // er hat keine Auswirkung auf die Fly-Position
            while( pOrient->IsSctFrm() )
                pOrient = pOrient->GetUpper();
            //pOrient ist das LayoutBlatt, das gerade verfolgt wird.
            //nRel    enthaelt die noch zu verarbeitende relative Entfernung.
            //nAvail  enthaelt die Strecke die im LayoutBlatt, das gerade
            //        verfolgt wird zur Verfuegung steht.

            if ( nRel <= 0 )
                aRelPos.Y() = 0;
            else
            {
                SwTwips nAvail = pOrient->Frm().Top() + pOrient->Prt().Top()
                    + pOrient->Prt().Height() - GetAnchor()->Frm().Top();
                const BOOL bFtn = GetAnchor()->IsInFtn();
                while ( nRel )
                {   if ( nRel <= nAvail ||
                            (bBrowse &&
                            ((SwFrm*)pOrient)->Grow( nRel-nAvail, pHeight, TRUE)) ||
                            (pOrient->IsInTab() && bGrow && //MA_FLY_HEIGHT
                            ((SwFrm*)pOrient)->Grow( nRel-nAvail, pHeight, TRUE)))
                    {   aRelPos.Y() = (pOrient->Frm().Top() +
                                        pOrient->Prt().Top() +
                                        (pOrient->Prt().Height() -
                                        nAvail) + nRel) -
                                        GetAnchor()->Frm().Top();
                        if ( ( bBrowse || ( pOrient->IsInTab() && bGrow ) )
                             && nRel - nAvail > 0 )
                        {
                            nRel = ((SwFrm*)pOrient)->Grow( nRel-nAvail, pHeight );
                            SwFrm *pTmp = (SwFrm*) pOrient->FindPageFrm();
                            ::ValidateSz( pTmp );
                            bInvalidatePage = TRUE;
                            //Schon mal einstellen, weil wir wahrscheinlich
                            //wegen Invalidierung eine Ehrenrunde drehen.
                            aFrm.Pos().Y() = aFrm.Top() + nRel;
                        }
                        nRel = 0;
                    }
                    else if ( bMoveable )
                    {   //Dem Textfluss folgen.
                        nRel -= nAvail;
                        const BOOL bSct = pOrient->IsInSct();
                        const SwFrm *pTmp = pOrient->
                            GetLeaf( ( bFtn || bSct ) ? MAKEPAGE_NONE : MAKEPAGE_APPEND,
                                        TRUE, GetAnchor() );
                        if ( pTmp && ( !bSct || pOrient->FindSctFrm()->
                                IsAnFollow( pTmp->FindSctFrm() ) ) )
                        {
                            pOrient = pTmp;
                            bMoveable =
                                    ::lcl_IsMoveable( this, (SwLayoutFrm*)pOrient);
                            if( !pFooter )
                                ::DeepCalc( pOrient );
                            nAvail = pOrient->Prt().Height();
                        }
                        else
                        {
                            // Wenn wir innerhalb des (spaltigen) Bereichs nicht genug
                            // Platz ist, wird es Zeit, diesen zu verlassen. Wir gehen
                            // also in seinen Upper und nehmen als nAvail den Platz, der
                            // hinter dem Bereich ist. Sollte dieser immer noch nicht
                            // ausreichen, wandern wir weiter, es hindert uns aber nun
                            // niemand mehr, neue Seiten anzulegen.
                            if( bSct )
                            {
                                const SwFrm* pSct = pOrient->FindSctFrm();
                                pOrient = pSct->GetUpper();
                                nAvail = pOrient->Frm().Top() + pOrient->Prt().Bottom()
                                            - pSct->Frm().Top() - pSct->Prt().Bottom();
                            }
                            else
                            {
                                nRelDiff = nRel;
                                nRel = 0;
                            }
                        }
                    }
                    else
                        nRel = 0;
                }
                if ( !bValidPos )
                    continue;
            }
        }
        //Damit das Teil ggf. auf die richtige Seite gestellt und in die
        //PrtArea des LayLeaf gezogen werden kann, muss hier seine
        //absolute Position berechnet werden.
        aFrm.Pos().Y() = GetAnchor()->Frm().Top() +
                         (aRelPos.Y() - nRelDiff);

        //Bei automatischer Ausrichtung nicht ueber die Oberkante hinausschiessen.
        if ( aVert.GetVertOrient() != VERT_NONE )
        {
            SwTwips nTop = pOrient->Frm().Top();
            if ( aVert.GetRelationOrient() == PRTAREA )
                nTop += pOrient->Prt().Top();
            if ( aFrm.Top() < nTop )
            {
                aFrm.Pos().Y() = nTop;
                aRelPos.Y() = nTop - GetAnchor()->Frm().Top();
                bHeightClipped = TRUE;
            }
        }

        const BOOL bFtn = GetAnchor()->IsInFtn();
        while( pOrient->IsSctFrm() )
            pOrient = pOrient->GetUpper();
        if ( aFrm.Bottom() > (pOrient->Frm().Top() + pOrient->Prt().Bottom()) )
        {
                                                           //MA_FLY_HEIGHT
            if ( ( bBrowse && GetAnchor()->IsMoveable() ) ||
                 ( GetAnchor()->IsInTab() && bGrow ) )
            {
                ((SwFrm*)pOrient)->Grow(
                        aFrm.Bottom() - (pOrient->Frm().Top() + pOrient->Prt().Bottom()), pHeight );
                SwFrm *pTmp = (SwFrm*) pOrient->FindPageFrm();
                ::ValidateSz( pTmp );
                bInvalidatePage = TRUE;
            }

            while ( bMoveable &&
                    aFrm.Bottom() > (pOrient->Frm().Top() + pOrient->Prt().Bottom()) )
            {
                // Vorsicht, auch innerhalb von Bereichen duerfen keine neuen Seiten angelegt werden
                const BOOL bSct = pOrient->IsInSct();
                if ( !bSct && Frm().Top() == pOrient->Frm().Top() + pOrient->Prt().Top() )
                    //Das teil passt nimmer, da hilft auch kein moven.
                    break;

                const SwLayoutFrm *pNextLay = pOrient->GetLeaf(
                    ( bFtn || bSct ) ? MAKEPAGE_NONE : MAKEPAGE_APPEND, TRUE, GetAnchor() );
                if ( pNextLay && ( !bSct || ( pOrient->FindSctFrm()->
                     IsAnFollow( pNextLay->FindSctFrm() ) && pNextLay->Prt().Height() ) ) )
                {
                    if( !pFooter )
                        ::DeepCalc( pNextLay );
                    aRelPos.Y() = pNextLay->Frm().Top() + pNextLay->Prt().Top()
                                  - GetAnchor()->Frm().Top();
                    pOrient = pNextLay;
                    bMoveable = ::lcl_IsMoveable( this, (SwLayoutFrm*)pOrient );
                    if ( bMoveable && !pFooter )
                        ::DeepCalc( pOrient );
                    aFrm.Pos().Y() = GetAnchor()->Frm().Top() + aRelPos.Y();
                }
                else if( bSct )
                {
                    // Wenn wir innerhalb des Bereich nicht genug Platz haben, gucken
                    // wir uns mal die Seite an.
                    const SwFrm* pTmp = pOrient->FindSctFrm()->GetUpper();
                    if( aFrm.Bottom() > (pTmp->Frm().Top() + pTmp->Prt().Bottom()) )
                        pOrient = pTmp;
                    else
                        break;
                }
                else
                    bMoveable = FALSE;
            }
        }
        AssertPage();

        //Horizontale Ausrichtung.
        //Die absolute Pos in der vertikalen muss schon mal eingestellt
        //werden, sonst habe ich Schwierigkeiten den virtuellen Anker
        //zu ermitteln.
        aFrm.Pos().Y() = aRelPos.Y() + GetAnchor()->Frm().Top();

        //Den Frm besorgen, an dem sich die horizontale Ausrichtung orientiert.
        pOrient = ::GetVirtualHoriAnchor( pOrient, this );

        if( !pFooter )
            ::DeepCalc( pOrient );

        // Achtung: pPage ist nicht unbedingt ein PageFrm, es kann auch ein
        // SwFlyFrm oder SwCellFrm dahinterstecken
        const SwFrm *pPage = pOrient;
        while( !pPage->IsPageFrm() && !pPage->IsFlyFrm() && !pPage->IsCellFrm() )
        {
            ASSERT( pPage->GetUpper(), "MakeFlyPos: No Page/FlyFrm Found" );
            pPage = pPage->GetUpper();
        }

        const BOOL bEven = !pPage->OnRightPage();
        const BOOL bToggle = aHori.IsPosToggle() && bEven;
        BOOL bTmpToggle = bToggle;
        BOOL bPageRel = FALSE;
        SwTwips nWidth, nAdd;
        switch ( aHori.GetRelationOrient() )
        {
            case PRTAREA:
            {
                nWidth = pOrient->Prt().Width();
                nAdd = pOrient->Prt().Left();
                break;
            }
            case REL_PG_LEFT:
                bTmpToggle = !bToggle;
                // kein break;
            case REL_PG_RIGHT:
            {
                if ( bTmpToggle )    // linker Seitenrand
                {
                    nWidth = pPage->Prt().Left();
                    nAdd = pPage->Frm().Left() - pOrient->Frm().Left();
                }
                else            // rechter Seitenrand
                {
                    nWidth = pPage->Frm().Width();
                    nAdd = pPage->Prt().Right();
                    nWidth -= nAdd;
                    nAdd += pPage->Frm().Left() - pOrient->Frm().Left();
                }
                bPageRel = TRUE;
                break;
            }
            case REL_FRM_LEFT:
                bTmpToggle = !bToggle;
                // kein break;
            case REL_FRM_RIGHT:
            {
                if ( bTmpToggle )    // linker Absatzrand
                {
                    nWidth = pOrient->Prt().Left();
                    nAdd = 0;
                }
                else            // rechter Absatzrand
                {
                    nWidth = pOrient->Frm().Width();
                    nAdd = pOrient->Prt().Right();
                    nWidth -= nAdd;
                }
                break;
            }
            case REL_CHAR:
            {
                if( pAutoPos )
                {
                    nWidth = 0;
                    nAdd = pAutoPos->Left() - pAutoOrient->Frm().Left();
                    break;
                }
                // No Break!
            }
            case REL_PG_PRTAREA:
            {
                nWidth = pPage->Prt().Width();
                nAdd = pPage->Frm().Left() + pPage->Prt().Left()
                       - pOrient->Frm().Left();
                bPageRel = TRUE;
                break;
            }
            case REL_PG_FRAME:
            {
                nWidth = pPage->Frm().Width();
                nAdd = pPage->Frm().Left() - pOrient->Frm().Left();
                bPageRel = TRUE;
                break;
            }
            default:
            {
                nWidth = pOrient->Frm().Width();
                nAdd = 0;
                break;
            }
        }
        if ( aHori.GetHoriOrient() == HORI_NONE )
        {
            if( pAutoPos && REL_CHAR == aHori.GetRelationOrient() )
                aRelPos.X() = aHori.GetPos() + nAdd;
            else if( bToggle )
                aRelPos.X() = nWidth - aFrm.Width() - aHori.GetPos();
            else
                aRelPos.X() = aHori.GetPos() + nAdd;
            //Da die relative Position immer zum Anker relativ ist,
            //muss dessen Entfernung zum virtuellen Anker aufaddiert werden.
            if ( GetAnchor() != pOrient )
            {
                long nTmp = pOrient->Frm().Left();
                aRelPos.X() += nTmp - GetAnchor()->Frm().Left();
                //fix(18546): Kleine Notbremse, wenn der Rahmen jetzt so positioniert
                //wird, dass er den Anker verdraengt, muessen wir unbedingt agieren.
                //fix(22698): in Ergaenzung zu obigem Bug passen wir jetzt etwas
                //grundlicher auf.
                if( !bPageRel && nTmp > pAnchor->Frm().Right() &&
                    Frm().Top() < GetAnchor()->Frm().Bottom() )
                {
                    nTmp = aRelPos.X() + GetAnchor()->Frm().Left();
                    if ( nTmp < GetAnchor()->Frm().Right() )
                        aRelPos.X() = GetAnchor()->Frm().Width()+1;
                }
            }
            if( GetAnchor()->Frm().Left() + aRelPos.X() + aFrm.Width() >
                pPage->Frm().Right() )
                aRelPos.X() = pPage->Frm().Right() - GetAnchor()->Frm().Left()
                              - aFrm.Width();
            if( GetAnchor()->Frm().Left() + aRelPos.X() < pPage->Frm().Left() )
                aRelPos.X() = pPage->Frm().Left() - GetAnchor()->Frm().Left();
        }
        else
        {
            SwHoriOrient eHOri = aHori.GetHoriOrient();
            SwRelationOrient eRelO = aHori.GetRelationOrient();
            if( bToggle )
            {
                if( HORI_RIGHT == eHOri )
                    eHOri = HORI_LEFT;
                else if( HORI_LEFT == eHOri )
                    eHOri = HORI_RIGHT;
                if( REL_PG_RIGHT == eRelO )
                    eRelO = REL_PG_LEFT;
                else if( REL_PG_LEFT == eRelO )
                    eRelO = REL_PG_RIGHT;
                else if( REL_FRM_RIGHT == eRelO )
                    eRelO = REL_FRM_LEFT;
                else if( REL_FRM_LEFT == eRelO )
                    eRelO = REL_FRM_RIGHT;
            }
            if ( eHOri == HORI_CENTER )
                aRelPos.X() = (nWidth / 2) - (aFrm.Width() / 2);
            else if ( eHOri == HORI_RIGHT )
                aRelPos.X() = nWidth - (aFrm.Width() + rLR.GetRight());
            else
                aRelPos.X() = rLR.GetLeft();
            aRelPos.X() += nAdd;

            //Da die relative Position immer zum Anker relativ ist,
            //muss dessen Entfernung zum virtuellen Anker aufaddiert werden.
            if ( GetAnchor() != pOrient )
                aRelPos.X() += pOrient->Frm().Left() -
                               GetAnchor()->Frm().Left();

            if( GetAnchor()->Frm().Left() + aRelPos.X() + aFrm.Width() >
                pPage->Frm().Right() )
                aRelPos.X() = pPage->Frm().Right() - GetAnchor()->Frm().Left()
                              - aFrm.Width();
            if( GetAnchor()->Frm().Left() + aRelPos.X() < pPage->Frm().Left() )
                aRelPos.X() = pPage->Frm().Left() - GetAnchor()->Frm().Left();

            //Es muss allen Rahmen ausgewichen werden, die die selbe
            //automatische Ausrichtung haben und die unter dem Rahmen liegen.
            if ( HORI_CENTER != eHOri && REL_CHAR != eRelO )
            {
                SwRect aTmpFrm( GetAnchor()->Frm().Pos() + aRelPos, Frm().SSize() );
                const UINT32 nMyOrd = GetVirtDrawObj()->GetOrdNum();
                SwOrderIter aIter( FindPageFrm(), TRUE );
                const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)aIter.Bottom())->GetFlyFrm();
                const SwFrm *pKontext = ::FindKontext( GetAnchor(), FRM_COLUMN );
                while ( pFly && nMyOrd > pFly->GetVirtDrawObj()->GetOrdNumDirect() )
                {
                    if ( pFly->IsFlyAtCntFrm() && //pFly->IsValid() &&
                         pFly->Frm().Bottom() >= aTmpFrm.Top() &&
                         pFly->Frm().Top() <= aTmpFrm.Bottom() &&
                         ::FindKontext( pFly->GetAnchor(), FRM_COLUMN ) == pKontext )
                    {
                        const SwFmtHoriOrient &rHori = pFly->GetFmt()->GetHoriOrient();
                        SwRelationOrient eRelO2 = rHori.GetRelationOrient();
                        if( REL_CHAR != eRelO2 )
                        {
                            SwHoriOrient eHOri2 = rHori.GetHoriOrient();
                            if( bEven && rHori.IsPosToggle() )
                            {
                                if( HORI_RIGHT == eHOri2 )
                                    eHOri2 = HORI_LEFT;
                                else if( HORI_LEFT == eHOri2 )
                                    eHOri2 = HORI_RIGHT;
                                if( REL_PG_RIGHT == eRelO2 )
                                    eRelO2 = REL_PG_LEFT;
                                else if( REL_PG_LEFT == eRelO2 )
                                    eRelO2 = REL_PG_RIGHT;
                                else if( REL_FRM_RIGHT == eRelO2 )
                                    eRelO2 = REL_FRM_LEFT;
                                else if( REL_FRM_LEFT == eRelO2 )
                                    eRelO2 = REL_FRM_RIGHT;
                            }
                            if ( eHOri2 == eHOri &&
                                 lcl_Minor( eRelO, eRelO2, HORI_LEFT == eHOri ) )
                            {
                                //Die Berechnung wird dadurch etwas aufwendiger, das die
                                //Ausgangsbasis der Flys unterschiedlich sein koennen.
                                const SvxLRSpaceItem &rLRI = pFly->GetFmt()->GetLRSpace();
                                const SwTwips nFlyLeft = pFly->Frm().Left() - rLRI.GetLeft();
                                const SwTwips nFlyRight = pFly->Frm().Right() + rLRI.GetRight();
                                // Hier wird noch abgefangen, dass die beiden Rahmen in verschiedenen
                                // Spalten stehen, aber an ein und demselben TxtFrm (Master/Follow)
                                // verankert sind. Das FindKontext ist nur bedingt aussagekraeftig,
                                // da es sich immer auf den Master bezieht.
                                if( nFlyLeft <= aTmpFrm.Right() + rLR.GetRight() &&
                                    nFlyRight >= aTmpFrm.Left() - rLR.GetLeft() )
                                {
                                    // Im folgenden GetAnchor() und nicht pOrient!
                                    if ( eHOri == HORI_LEFT )
                                        aRelPos.X() = Max( aRelPos.X(), nFlyRight+1
                                            +rLRI.GetRight() -GetAnchor()->Frm().Left());
                                    else if ( eHOri == HORI_RIGHT )
                                        aRelPos.X() = Min( aRelPos.X(), nFlyLeft-1
                                            -rLRI.GetLeft() - Frm().Width() - GetAnchor()->Frm().Left());
                                    aTmpFrm.Pos() = GetAnchor()->Frm().Pos() + aRelPos;
                                }
                            }
                        }
                    }
                    pFly = ((SwVirtFlyDrawObj*)aIter.Next())->GetFlyFrm();
                }
            }

            if ( aHori.GetPos() != aRelPos.X() )
            {   aHori.SetPos( aRelPos.X() );
                bHoriChgd = TRUE;
            }
        }

        AssertPage();

        //Die AbsPos ergibt sich aus der Absoluten Position des Ankers
        //plus der relativen Position
        aFrm.Pos( aRelPos + GetAnchor()->Frm().Pos() );

        //Und ggf. noch die aktuellen Werte im Format updaten, dabei darf
        //zu diesem Zeitpunkt natuerlich kein Modify verschickt werden.
        pFmt->LockModify();
        if ( bVertChgd )
            pFmt->SetAttr( aVert );
        if ( bHoriChgd )
            pFmt->SetAttr( aHori );
        pFmt->UnlockModify();

        break;
        }
        if ( bInvalidatePage )
            FindPageFrm()->InvalidateSize();
        if ( !bValidPos && !GetAnchor()->IsValid() )
        {
//          ASSERT( StackHack::IsLocked(), "invalid Anchor" );
            bValidPos = TRUE;
        }
    }
}


