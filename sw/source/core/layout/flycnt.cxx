/*************************************************************************
 *
 *  $RCSfile: flycnt.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:09:05 $
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
#include "ndtxt.hxx"
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

// OD 29.10.2003 #113049#
#ifndef _TOCNTNTANCHOREDOBJECTPOSITION_HXX
#include <tocntntanchoredobjectposition.hxx>
#endif
// OD 2004-05-24 #i28701#
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

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

// --> OD 2004-06-29 #i28701#
TYPEINIT1(SwFlyAtCntFrm,SwFlyFreeFrm);
// <--
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
        SwRect aOld( GetObjRectWithSpaces() );
        SwPageFrm *pOldPage = FindPageFrm();
        const SwFrm *pOldAnchor = GetAnchorFrm();
        SwCntntFrm *pCntnt = (SwCntntFrm*)GetAnchorFrm();
        AnchorFrm()->RemoveFly( this );

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
        SwCntntFrm* pFlow = pCntnt;
        while ( pFlow->IsFollow() )
            pFlow = pFlow->FindMaster();
        pCntnt = pFlow;

        //und schwupp angehaengt das teil...
        pCntnt->AppendFly( this );
        if ( pOldPage && pOldPage != FindPageFrm() )
            NotifyBackground( pOldPage, aOld, PREP_FLY_LEAVE );

        //Fix(3495)
        _InvalidatePos();
        InvalidatePage();
        SetNotifyBack();
        // --> OD 2004-06-24 #i28701# - reset member <maLastCharRect> and
        // <mnLastTopOfLine> for to-character anchored objects.
        ClearCharRectAndTopOfLine();
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

//
// SwFlyAtCntFrm::MakeAll wants to calculate the section, if it contains
// the anchor frame. We do not want to calculate the section, if there
// is a table between the anchor frame and the section frame
//
void lcl_CalcUpperSection( const SwFrm& rFrm )
{
    if( rFrm.IsInSct() )
    {
        // Do not calculate the section, if there is a table between
        // the anchor and the section.
        ASSERT( rFrm.IsInSct(), "lcl_SectionBeforeTable without section" )
        const SwFrm* pUpper = rFrm.GetUpper();
        while ( pUpper )
        {
            if ( pUpper->IsSctFrm() || pUpper->IsCellFrm() )
                break;
            pUpper = pUpper->GetUpper();
        }

        ASSERT( pUpper, "lcl_SectionBeforeTable without section" )
        if ( pUpper->IsSctFrm() )
            pUpper->Calc();
    }
}


void SwFlyAtCntFrm::MakeAll()
{
    // OD 2004-01-19 #110582#
    if ( !GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !SwOszControl::IsInProgress( this ) && !IsLocked() && !IsColLocked() )
    {
        // --> OD 2004-06-30 #i28701# - use new method <GetPageFrm()>
        if( !GetPageFrm() && GetAnchorFrm() && GetAnchorFrm()->IsInFly() )
        {
            SwFlyFrm* pFly = AnchorFrm()->FindFlyFrm();
            SwPageFrm *pPage = pFly ? pFly->FindPageFrm() : NULL;
            if( pPage )
                pPage->AppendFlyToPage( this );
        }
        // --> OD 2004-06-30 #i28701# - use new method <GetPageFrm()>
        if( GetPageFrm() )
        {
            bSetCompletePaintOnInvalidate = TRUE;
            {
                SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
                const SwFmtFrmSize &rFrmSz = GetFmt()->GetFrmSize();
                if( rFrmSz.GetHeightPercent() != 0xFF &&
                    rFrmSz.GetHeightPercent() >= 100 )
                {
                    pFmt->LockModify();
                    SwFmtSurround aMain( pFmt->GetSurround() );
                    if ( aMain.GetSurround() == SURROUND_NONE )
                    {
                        aMain.SetSurround( SURROUND_THROUGHT );
                        pFmt->SetAttr( aMain );
                    }
                    pFmt->UnlockModify();
                }
            }

            SwOszControl aOszCntrl( this );

            const sal_Bool bLockedAnchor =
                static_cast<const SwTxtFrm*>( GetAnchorFrm() )->IsAnyJoinLocked();

            // OD 2004-05-12 #i28701# - no format of anchor frame, if
            // wrapping style influence is considered on object positioning
            if ( !ConsiderObjWrapInfluenceOnObjPos() && !bLockedAnchor )
            {
                // If the anchor is located inside a section, we better calculate
                // the section first:
                lcl_CalcUpperSection( *AnchorFrm() );
                GetAnchorFrm()->Calc();
            }

            const SwFrm* pFooter = GetAnchorFrm()->FindFooterOrHeader();
            if( pFooter && !pFooter->IsFooterFrm() )
                pFooter = NULL;
            FASTBOOL bOsz = FALSE;
            FASTBOOL bExtra = Lower() && Lower()->IsColumnFrm();

            do {
                SWRECTFN( this )
                Point aOldPos( (Frm().*fnRect->fnGetPos)() );
                SwFlyFreeFrm::MakeAll();
                const bool bPosChgDueToOwnFormat =
                                        aOldPos != (Frm().*fnRect->fnGetPos)();

                // OD 2004-05-12 #i28701# - no format of anchor frame, if
                // wrapping style influence is considered on object positioning
                if ( !ConsiderObjWrapInfluenceOnObjPos() &&
                     !bLockedAnchor )
                {
                    // If the anchor is located inside a section, we better calculate
                    // the section first:
                    lcl_CalcUpperSection( *AnchorFrm() );
                    GetAnchorFrm()->Calc();
                }

                if ( aOldPos != (Frm().*fnRect->fnGetPos)() ||
                     ( !GetValidPosFlag() &&
                       ( pFooter || bPosChgDueToOwnFormat ) ) )
                {
                    bOsz = aOszCntrl.ChkOsz();
                }

                if ( bExtra && Lower() && !Lower()->GetValidPosFlag() )
                {
                    // Wenn ein mehrspaltiger Rahmen wg. Positionswechsel ungueltige
                    // Spalten hinterlaesst, so drehen wir lieber hier eine weitere
                    // Runde und formatieren unseren Inhalt via FormatWidthCols nochmal.
                    _InvalidateSize();
                    bExtra = FALSE; // Sicherhaltshalber gibt es nur eine Ehrenrunde.
                }
            } while ( !IsValid() && !bOsz &&
                      GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) );

            if ( bOsz )
            {
#ifdef DEBUG
                ASSERT( false,
                        "<SwFlyAtCntFrm::MakeAll()> - loop detected, perform attribute changes to avoid the loop" );
#endif
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
                if( !bLockedAnchor )
                    GetAnchorFrm()->Calc();
                if ( !GetValidPosFlag() )
                {
                    SwFlyFreeFrm::MakeAll();
                    if( !bLockedAnchor )
                        GetAnchorFrm()->Calc();
                }
                // validate fly frame
                bValidPos = bValidSize = bValidPrtArea = TRUE;
            }
            bSetCompletePaintOnInvalidate = FALSE;
        }
    }
}

/** method to determine, if a <MakeAll()> on the Writer fly frame is possible

    OD 2004-05-11 #i28701#

    @author OD
*/
bool SwFlyAtCntFrm::IsFormatPossible() const
{
    return SwFlyFreeFrm::IsFormatPossible() &&
           !SwOszControl::IsInProgress( this );
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
        SWRECTFN( pUp )
        //Dem Textflus folgen.
        if ( pUp->Frm().IsInside( rPt ) )
        {
            // OD 26.09.2003 - <rPt> point is inside environment of given content frame
            if( bVert )
                rRet.nMain =  pCnt->Frm().Left() + pCnt->Frm().Width() -rPt.X();
            else
                rRet.nMain =  rPt.Y() - pCnt->Frm().Top();
            return pCnt;
        }
        else if ( rPt.Y() <= pUp->Frm().Top() )
        {
            // OD 26.09.2003 - <rPt> point is above environment of given content frame
            // OD: correct for vertical layout?
            rRet.nMain = LONG_MAX;
        }
        else if( rPt.X() < pUp->Frm().Left() &&
                 rPt.Y() <= ( bVert ? pUp->Frm().Top() : pUp->Frm().Bottom() ) )
        {
            // OD 26.09.2003 - <rPt> point is left of environment of given content frame
            // OD: seems not to be correct for vertical layout!?
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, FALSE, pCnt );
            if( !pLay ||
                (bVert && (pLay->Frm().Top() + pLay->Prt().Bottom()) <rPt.Y())||
                (!bVert && (pLay->Frm().Left() + pLay->Prt().Right())<rPt.X()) )
            {
                // OD 26.09.2003 - <rPt> point is in left border of environment
                if( bVert )
                    rRet.nMain =  pCnt->Frm().Left() + pCnt->Frm().Width()
                                  - rPt.X();
                else
                    rRet.nMain = rPt.Y() - pCnt->Frm().Top();
                return pCnt;
            }
            else
                rRet.nMain = LONG_MAX;
        }
        else
        {
            rRet.nMain = bVert ? pCnt->Frm().Left() + pCnt->Frm().Width() -
                                 (pUp->Frm().Left() + pUp->Prt().Left())
                : (pUp->Frm().Top() + pUp->Prt().Bottom()) - pCnt->Frm().Top();

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
                    if( pLay->IsVertical() )
                    {
                        nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
                        nPrtHeight = pLay->Prt().Width();
                    }
                    else
                    {
                        nFrmTop = pLay->Frm().Top();
                        nPrtHeight = pLay->Prt().Height();
                    }
                    pSect = pNxtSect;
                }
                else
                {
                    pLay = pSect->GetUpper();
                    if( pLay->IsVertical() )
                    {
                        nFrmTop = pSect->Frm().Left();
                        nPrtHeight = pSect->Frm().Left() - pLay->Frm().Left()
                                     - pLay->Prt().Left();
                    }
                    else
                    {
                        nFrmTop = pSect->Frm().Bottom();
                        nPrtHeight = pLay->Frm().Top() + pLay->Prt().Top()
                                     + pLay->Prt().Height() - pSect->Frm().Top()
                                     - pSect->Frm().Height();
                    }
                    pSect = 0;
                }
            }
            else if( pLay )
            {
                if( pLay->IsVertical() )
                {
                    nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
                    nPrtHeight = pLay->Prt().Width();
                }
                else
                {
                    nFrmTop = pLay->Frm().Top();
                    nPrtHeight = pLay->Prt().Height();
                }
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
                            pSect = pNxtSect;
                            if( pLay->IsVertical() )
                            {
                                nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
                                nPrtHeight = pLay->Prt().Width();
                            }
                            else
                            {
                                nFrmTop = pLay->Frm().Top();
                                nPrtHeight = pLay->Prt().Height();
                            }
                        }
                        else
                        {
                            pLay = pSect->GetUpper();
                            if( pLay->IsVertical() )
                            {
                                nFrmTop = pSect->Frm().Left();
                                nPrtHeight = pSect->Frm().Left() -
                                        pLay->Frm().Left() - pLay->Prt().Left();
                            }
                            else
                            {
                                nFrmTop = pSect->Frm().Bottom();
                                nPrtHeight = pLay->Frm().Top()+pLay->Prt().Top()
                                     + pLay->Prt().Height() - pSect->Frm().Top()
                                     - pSect->Frm().Height();
                            }
                            pSect = 0;
                        }
                    }
                    else if( pLay )
                    {
                        if( pLay->IsVertical() )
                        {
                             nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
                             nPrtHeight = pLay->Prt().Width();
                        }
                        else
                        {
                            nFrmTop = pLay->Frm().Top();
                            nPrtHeight = pLay->Prt().Height();
                        }
                        bSct = 0 != pSect;
                    }
                }
            }
            if ( pLay )
            {
                if ( pLay->Frm().IsInside( rPt ) )
                {
                    SwTwips nDiff = pLay->IsVertical() ? ( nFrmTop - rPt.X() )
                                                       : ( rPt.Y() - nFrmTop );
                    if( bSct || pSect )
                        rRet.nSub += nDiff;
                    else
                        rRet.nMain += nDiff;
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

#if OSL_DEBUG_LEVEL > 1
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
    const SwCntntFrm* pCnt;
    if ( pOldAnch->IsCntntFrm() )
    {
        pCnt = (const SwCntntFrm*)pOldAnch;
    }
    else
    {
        Point aTmp( rNew );
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
        if ( nUp.nMain >= 0 && LONG_MAX != nUp.nMain )
        {
            bNegAllowed = FALSE;
            if ( nUpLst.nMain < 0 ) //nicht den falschen erwischen, wenn der Wert
                                    //gerade von negativ auf positiv gekippt ist.
            {   pUpLst = pUpFrm;
                nUpLst = nUp;
            }
        }
    } while ( pUpFrm && ( ( bNegAllowed && nUp.nMain < 0 ) || ( nUp <= nUpLst ) ) );

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
    {
        // #102861# If an OLE objects, which is contained in a fly frame
        // is resized in inplace mode and the new Position is outside the
        // fly frame, we do not want to leave our fly frame.
        if ( pCnt->IsInFly() )
            return pCnt;

        return ::lcl_FindCnt( aNew, pCnt, bBody, bFtn );
    }
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
    const SwRect aOld( GetObjRectWithSpaces() );
    Point aNew( rNew );

    if( GetAnchorFrm()->IsVertical() || GetAnchorFrm()->IsRightToLeft() )

        aNew.X() += Frm().Width();
    SwCntntFrm *pCnt = (SwCntntFrm*)::FindAnchor( GetAnchorFrm(), aNew );
    if( pCnt->IsProtected() )
        pCnt = (SwCntntFrm*)GetAnchorFrm();

    SwPageFrm *pPage = 0;
    SWRECTFN( pCnt )
    const sal_Bool bRTL = pCnt->IsRightToLeft();

    if( ( bVert != GetAnchorFrm()->IsVertical() ) ||
        ( bRTL != GetAnchorFrm()->IsRightToLeft() ) )
    {
        if( bVert || bRTL )
            aNew.X() += Frm().Width();
        else
            aNew.X() -= Frm().Width();
    }

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
    {
        if( bVert )
            nY = pCnt->Frm().Left()+pCnt->Frm().Width()-rNew.X()-Frm().Width();
        else
            nY = rNew.Y() - pCnt->Frm().Top();
    }
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
            if( pUp->IsVertical() )
                nDiff += pFollow->Frm().Left() + pFollow->Frm().Width()
                         - pUp->Frm().Left() - pUp->Prt().Left();
            else
                nDiff += pUp->Prt().Height() - pFollow->GetRelPos().Y();
            pFollow = pFollow->GetFollow();
        } while ( pFollow != pOriginal );
        nY += nDiff;
        if( bVert )
            nX = pCnt->Frm().Top() - pOriginal->Frm().Top();
        else
            nX = pCnt->Frm().Left() - pOriginal->Frm().Left();
    }

    if ( nY == LONG_MAX )
    {
        if( bVert )
            nY = pCnt->Frm().Left() + pCnt->Frm().Width() - rNew.X();
        else
            nY = rNew.Y() - pCnt->Frm().Top();
    }

    SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
    const SwFmtSurround& rSurround = pFmt->GetSurround();
    const FASTBOOL bWrapThrough =
        rSurround.GetSurround() == SURROUND_THROUGHT;
    SwTwips nBaseOfstForFly = 0;
    const SwFrm* pTmpFrm = pFrm ? pFrm : pCnt;
    if ( pTmpFrm->IsTxtFrm() )
        nBaseOfstForFly =
            ((SwTxtFrm*)pTmpFrm)->GetBaseOfstForFly( !bWrapThrough );

    if( bVert )
    {
        if( !pFrm )
            nX += rNew.Y() - pCnt->Frm().Top() - nBaseOfstForFly;
        else
            nX = rNew.Y() - pFrm->Frm().Top() - nBaseOfstForFly;
    }
    else
    {
        if( !pFrm )
        {
            if ( pCnt->IsRightToLeft() )
                nX += pCnt->Frm().Right() - rNew.X() - Frm().Width() +
                      nBaseOfstForFly;
            else
                nX += rNew.X() - pCnt->Frm().Left() - nBaseOfstForFly;
        }
        else
        {
            if ( pFrm->IsRightToLeft() )
                nX += pFrm->Frm().Right() - rNew.X() - Frm().Width() +
                      nBaseOfstForFly;
            else
                nX = rNew.X() - pFrm->Frm().Left() - nBaseOfstForFly;
        }
    }
    GetFmt()->GetDoc()->StartUndo( UNDO_START );

    if( pCnt != GetAnchorFrm() || ( IsAutoPos() && pCnt->IsTxtFrm() &&
                                  GetFmt()->GetDoc()->IsHTMLMode() ) )
    {
        //Das Ankerattribut auf den neuen Cnt setzen.
        SwFmtAnchor aAnch( pFmt->GetAnchor() );
        SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
        if( IsAutoPos() && pCnt->IsTxtFrm() )
        {
            SwCrsrMoveState eTmpState( MV_SETONLYTEXT );
            Point aPt( rNew );
            if( pCnt->GetCrsrOfst( pPos, aPt, &eTmpState )
                && pPos->nNode == *pCnt->GetNode() )
            {
                ResetLastCharRectHeight();
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
    // --> OD 2004-06-30 #i28701# - use new method <GetPageFrm()>
    else if ( pPage && pPage != GetPageFrm() )
        GetPageFrm()->MoveFly( this, pPage );

    const Point aRelPos = bVert ? Point( -nY, nX ) : Point( nX, nY );
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

void DeepCalc( const SwFrm *pFrm )
{
    if( pFrm->IsSctFrm() ||
        ( pFrm->IsFlyFrm() && ((SwFlyFrm*)pFrm)->IsFlyInCntFrm() ) )
        return;
    const SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( pFrm );
    if( pFlow && pFlow->IsAnyJoinLocked() )
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
                SWRECTFN( pUp )
                const Point aPt( (pUp->Frm().*fnRect->fnGetPos)() );
                ::DeepCalc( pUp );
                bContinue = aPt != (pUp->Frm().*fnRect->fnGetPos)();
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

/** method to assure that anchored object is registered at the correct
    page frame

    OD 2004-07-02 #i28701#
    takes over functionality of deleted method <SwFlyAtCntFrm::AssertPage()>

    @author OD
*/
void SwFlyAtCntFrm::RegisterAtCorrectPage()
{
    SwPageFrm* pPageFrm( 0L );
    if ( GetVertPosOrientFrm() )
    {
        pPageFrm = const_cast<SwPageFrm*>(GetVertPosOrientFrm()->FindPageFrm());
    }
    if ( pPageFrm && GetPageFrm() != pPageFrm )
    {
        if ( GetPageFrm() )
            GetPageFrm()->MoveFly( this, pPageFrm );
        else
            pPageFrm->AppendFlyToPage( this );
    }
}

// OD 2004-03-23 #i26791#
//void SwFlyAtCntFrm::MakeFlyPos()
void SwFlyAtCntFrm::MakeObjPos()
{
    // OD 02.10.2002 #102646#
    // if fly frame position is valid, nothing is to do, Thus, return
    if ( bValidPos )
    {
        return;
    }

    // OD 2004-03-24 #i26791# - validate position flag here.
    bValidPos = TRUE;

    // OD 29.10.2003 #113049# - use new class to position object
    objectpositioning::SwToCntntAnchoredObjectPosition
            aObjPositioning( *GetVirtDrawObj() );
    aObjPositioning.CalcPosition();

    SetVertPosOrientFrm ( aObjPositioning.GetVertPosOrientFrm() );
}

// OD 2004-05-12 #i28701#
bool SwFlyAtCntFrm::_InvalidationAllowed( const InvalidationType _nInvalid ) const
{
    bool bAllowed( SwFlyFreeFrm::_InvalidationAllowed( _nInvalid ) );

    // forbiddance of base instance can't be over ruled.
    if ( bAllowed )
    {
        if ( _nInvalid == INVALID_POS ||
             _nInvalid == INVALID_ALL )
        {
            bAllowed = InvalidationOfPosAllowed();
        }
    }

    return bAllowed;
}
