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

#include <tools/bigint.hxx>
#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "flyfrm.hxx"
#include "txtfrm.hxx"
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
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
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>

#include "tabfrm.hxx"
#include "flyfrms.hxx"
#include "crstate.hxx"
#include "sectfrm.hxx"

// OD 29.10.2003 #113049#
#include <tocntntanchoredobjectposition.hxx>
// OD 2004-05-24 #i28701#
#include <dcontact.hxx>
#include <sortedobjs.hxx>
// --> OD 2005-09-29 #125370#,#125957#
#include <layouter.hxx>
// <--
// --> OD 2005-11-17 #i56300#
#include <objectformattertxtfrm.hxx>
// <--
// --> OD 2006-03-06 #125892#
#include <HandleAnchorNodeChg.hxx>
// <--

using namespace ::com::sun::star;


/*************************************************************************
|*
|*  SwFlyAtCntFrm::SwFlyAtCntFrm()
|*
|*  Ersterstellung      MA 11. Nov. 92
|*  Letzte Aenderung    MA 09. Apr. 99
|*
|*************************************************************************/

SwFlyAtCntFrm::SwFlyAtCntFrm( SwFlyFrmFmt *pFmt, SwFrm* pSib, SwFrm *pAnch ) :
    SwFlyFreeFrm( pFmt, pSib, pAnch )
{
    bAtCnt = sal_True;
    bAutoPosition = (FLY_AT_CHAR == pFmt->GetAnchor().GetAnchorId());
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

void SwFlyAtCntFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew )
{
    sal_uInt16 nWhich = pNew ? pNew->Which() : 0;
    const SwFmtAnchor *pAnch = 0;

    if( RES_ATTRSET_CHG == nWhich && SFX_ITEM_SET ==
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_ANCHOR, sal_False,
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

        const sal_Bool bBodyFtn = (pCntnt->IsInDocBody() || pCntnt->IsInFtn());

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
        const bool bNext = aOldIdx < aNewIdx;
        // --> OD 2006-02-28 #125892#
        // consider the case that at found anchor frame candidate already a
        // fly frame of the given fly format is registered.
        // --> OD 2006-03-15 #133407# - consider, that <pCntnt> is the already
        // the new anchor frame.
        bool bFound( aOldIdx == aNewIdx );
        // <--
        while ( pCntnt && !bFound )
        {
            do
            {
                if ( bNext )
                    pCntnt = pCntnt->GetNextCntntFrm();
                else
                    pCntnt = pCntnt->GetPrevCntntFrm();
            } while ( pCntnt &&
                      !( bBodyFtn == ( pCntnt->IsInDocBody() ||
                                       pCntnt->IsInFtn() ) ) );
            if ( pCntnt )
                aOldIdx = *pCntnt->GetNode();

            // --> OD 2006-02-28 #125892#
            // check, if at found anchor frame candidate already a fly frame
            // of the given fly frame format is registered.
            bFound = aOldIdx == aNewIdx;
            if ( bFound && pCntnt->GetDrawObjs() )
            {
                SwFrmFmt* pMyFlyFrmFmt( &GetFrmFmt() );
                SwSortedObjs &rObjs = *pCntnt->GetDrawObjs();
                for( sal_uInt16 i = 0; i < rObjs.Count(); ++i)
                {
                    SwFlyFrm* pFlyFrm = dynamic_cast<SwFlyFrm*>(rObjs[i]);
                    if ( pFlyFrm &&
                         &(pFlyFrm->GetFrmFmt()) == pMyFlyFrmFmt )
                    {
                        bFound = false;
                        break;
                    }
                }
            }
            // <--
        }
        // <--
        if ( !pCntnt )
        {
            SwCntntNode *pNode = aNewIdx.GetNode().GetCntntNode();
<<<<<<< local
            pCntnt = pNode->getLayoutFrm( getRootFrm(), &pOldAnchor->Frm().Pos(), 0, FALSE );
=======
            pCntnt = pNode->GetFrm( &pOldAnchor->Frm().Pos(), 0, sal_False );
>>>>>>> other
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
// OD 2004-08-25 #i3317# - re-factoring of the position stack
class SwOszControl
{
    static const SwFlyFrm *pStk1;
    static const SwFlyFrm *pStk2;
    static const SwFlyFrm *pStk3;
    static const SwFlyFrm *pStk4;
    static const SwFlyFrm *pStk5;

    const SwFlyFrm *pFly;
    // --> OD 2004-08-25 #i3317#
    sal_uInt8 mnPosStackSize;
    std::vector<Point*> maObjPositions;
    // <--

public:
    SwOszControl( const SwFlyFrm *pFrm );
    ~SwOszControl();
    bool ChkOsz();
    static sal_Bool IsInProgress( const SwFlyFrm *pFly );
};
const SwFlyFrm *SwOszControl::pStk1 = 0;
const SwFlyFrm *SwOszControl::pStk2 = 0;
const SwFlyFrm *SwOszControl::pStk3 = 0;
const SwFlyFrm *SwOszControl::pStk4 = 0;
const SwFlyFrm *SwOszControl::pStk5 = 0;

SwOszControl::SwOszControl( const SwFlyFrm *pFrm )
    : pFly( pFrm ),
      // --> OD 2004-08-25 #i3317#
      mnPosStackSize( 20 )
      // <--
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
    // --> OD 2004-08-25 #i3317#
    while ( !maObjPositions.empty() )
    {
        Point* pPos = maObjPositions.back();
        delete pPos;

        maObjPositions.pop_back();
    }
    // <--
}

sal_Bool SwOszControl::IsInProgress( const SwFlyFrm *pFly )
{
    if ( SwOszControl::pStk1 && !pFly->IsLowerOf( SwOszControl::pStk1 ) )
        return sal_True;
    if ( SwOszControl::pStk2 && !pFly->IsLowerOf( SwOszControl::pStk2 ) )
        return sal_True;
    if ( SwOszControl::pStk3 && !pFly->IsLowerOf( SwOszControl::pStk3 ) )
        return sal_True;
    if ( SwOszControl::pStk4 && !pFly->IsLowerOf( SwOszControl::pStk4 ) )
        return sal_True;
    if ( SwOszControl::pStk5 && !pFly->IsLowerOf( SwOszControl::pStk5 ) )
        return sal_True;
    return sal_False;
}

bool SwOszControl::ChkOsz()
{
    bool bOscillationDetected = false;

    if ( maObjPositions.size() == mnPosStackSize )
    {
        // position stack is full -> oscillation
        bOscillationDetected = true;
    }
    else
    {
        Point* pNewObjPos = new Point( pFly->GetObjRect().Pos() );
        for ( std::vector<Point*>::iterator aObjPosIter = maObjPositions.begin();
              aObjPosIter != maObjPositions.end();
              ++aObjPosIter )
        {
            if ( *(pNewObjPos) == *(*aObjPosIter) )
            {
                // position already occured -> oscillation
                bOscillationDetected = true;
                delete pNewObjPos;
                break;
            }
        }
        if ( !bOscillationDetected )
        {
            maObjPositions.push_back( pNewObjPos );
        }
    }

    return bOscillationDetected;
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
            SwPageFrm *pTmpPage = pFly ? pFly->FindPageFrm() : NULL;
            if( pTmpPage )
                pTmpPage->AppendFlyToPage( this );
        }
        // --> OD 2004-06-30 #i28701# - use new method <GetPageFrm()>
        if( GetPageFrm() )
        {
            bSetCompletePaintOnInvalidate = sal_True;
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
                        pFmt->SetFmtAttr( aMain );
                    }
                    pFmt->UnlockModify();
                }
            }

            SwOszControl aOszCntrl( this );

            // --> OD 2005-02-22 #i43255#
            // --> OD 2005-06-07 #i50356# - format the anchor frame, which
            // contains the anchor position. E.g., for at-character anchored
            // object this can be the follow frame of the anchor frame.
            const bool bFormatAnchor =
                    !static_cast<const SwTxtFrm*>( GetAnchorFrmContainingAnchPos() )->IsAnyJoinLocked() &&
                    !ConsiderObjWrapInfluenceOnObjPos() &&
                    !ConsiderObjWrapInfluenceOfOtherObjs();
            // <--

            const SwFrm* pFooter = GetAnchorFrm()->FindFooterOrHeader();
            if( pFooter && !pFooter->IsFooterFrm() )
                pFooter = NULL;
            bool bOsz = false;
            sal_Bool bExtra = Lower() && Lower()->IsColumnFrm();
            // --> OD 2004-08-25 #i3317# - boolean, to apply temporarly the
            // 'straightforward positioning process' for the frame due to its
            // overlapping with a previous column.
            bool bConsiderWrapInfluenceDueToOverlapPrevCol( false );
            // <--
            // --> OD 2004-10-22 #i35911# - boolean, to apply temporarly the
            // 'straightforward positioning process' for the frame due to fact
            // that it causes the complete content of its layout environment
            // to move forward.
            // --> OD 2005-01-14 #i40444# - extend usage of this boolean:
            // apply temporarly the 'straightforward positioning process' for
            // the frame due to the fact that the frame clears the area for
            // the anchor frame, thus it has to move forward.
            bool bConsiderWrapInfluenceDueToMovedFwdAnchor( false );
            // <--
            do {
                SWRECTFN( this )
                Point aOldPos( (Frm().*fnRect->fnGetPos)() );
                SwFlyFreeFrm::MakeAll();
                const bool bPosChgDueToOwnFormat =
                                        aOldPos != (Frm().*fnRect->fnGetPos)();
                // --> OD 2004-08-25 #i3317#
                if ( !ConsiderObjWrapInfluenceOnObjPos() &&
                     OverlapsPrevColumn() )
                {
                    bConsiderWrapInfluenceDueToOverlapPrevCol = true;
                }
                // <--
                // OD 2004-05-12 #i28701# - no format of anchor frame, if
                // wrapping style influence is considered on object positioning
                if ( bFormatAnchor )
                {
                    SwTxtFrm* pAnchPosAnchorFrm =
                            dynamic_cast<SwTxtFrm*>(GetAnchorFrmContainingAnchPos());
                    ASSERT( pAnchPosAnchorFrm,
                            "<SwFlyAtCntFrm::MakeAll()> - anchor frame of wrong type -> crash" );
                    // --> OD 2006-01-27 #i58182# - For the usage of new method
                    // <SwObjectFormatterTxtFrm::CheckMovedFwdCondition(..)>
                    // to check move forward of anchor frame due to the object
                    // positioning it's needed to know, if the object is anchored
                    // at the master frame before the anchor frame is formatted.
                    const bool bAnchoredAtMaster( !pAnchPosAnchorFrm->IsFollow() );
                    // <--

                    // --> OD 2005-11-17 #i56300#
                    // perform complete format of anchor text frame and its
                    // previous frames, which have become invalid due to the
                    // fly frame format.
                    SwObjectFormatterTxtFrm::FormatAnchorFrmAndItsPrevs( *pAnchPosAnchorFrm );
                    // <--
                    // --> OD 2004-10-22 #i35911#
                    // --> OD 2005-01-14 #i40444#
                    // --> OD 2006-01-27 #i58182# - usage of new method
                    // <SwObjectFormatterTxtFrm::CheckMovedFwdCondition(..)>
                    sal_uInt32 nToPageNum( 0L );
                    bool bDummy( false );
                    if ( SwObjectFormatterTxtFrm::CheckMovedFwdCondition(
                                        *this, GetPageFrm()->GetPhyPageNum(),
                                        bAnchoredAtMaster, nToPageNum, bDummy ) )
                    {
                        bConsiderWrapInfluenceDueToMovedFwdAnchor = true;
                        // --> OD 2005-09-29 #125370#,#125957# - mark anchor text frame
                        // directly, that it is moved forward by object positioning.
                        SwTxtFrm* pAnchorTxtFrm( static_cast<SwTxtFrm*>(AnchorFrm()) );
                        bool bInsert( true );
                        sal_uInt32 nAnchorFrmToPageNum( 0L );
                        const SwDoc& rDoc = *(GetFrmFmt().GetDoc());
                        if ( SwLayouter::FrmMovedFwdByObjPos(
                                                rDoc, *pAnchorTxtFrm, nAnchorFrmToPageNum ) )
                        {
                            if ( nAnchorFrmToPageNum < nToPageNum )
                                SwLayouter::RemoveMovedFwdFrm( rDoc, *pAnchorTxtFrm );
                            else
                                bInsert = false;
                        }
                        if ( bInsert )
                        {
                            SwLayouter::InsertMovedFwdFrm( rDoc, *pAnchorTxtFrm,
                                                           nToPageNum );
                        }
                        // <--
                    }
                    // <--
                }

                if ( aOldPos != (Frm().*fnRect->fnGetPos)() ||
                     ( !GetValidPosFlag() &&
                       ( pFooter || bPosChgDueToOwnFormat ) ) )
                {
                    bOsz = aOszCntrl.ChkOsz();

                    // --> OD 2006-04-13 #b6403541#
                    // special loop prevention for dedicated document:
                    if ( bOsz &&
                         HasFixSize() && IsClipped() &&
                         GetAnchorFrm()->GetUpper()->IsCellFrm() )
                    {
                        SwFrmFmt* pFmt = GetFmt();
                        const SwFmtFrmSize& rFrmSz = pFmt->GetFrmSize();
                        if ( rFrmSz.GetWidthPercent() &&
                             rFrmSz.GetHeightPercent() == 0xFF )
                        {
                            SwFmtSurround aSurround( pFmt->GetSurround() );
                            if ( aSurround.GetSurround() == SURROUND_NONE )
                            {
                                pFmt->LockModify();
                                aSurround.SetSurround( SURROUND_THROUGHT );
                                pFmt->SetFmtAttr( aSurround );
                                pFmt->UnlockModify();
                                bOsz = false;
#if OSL_DEBUG_LEVEL > 1
                                ASSERT( false,
                                        "<SwFlyAtCntFrm::MakeAll()> - special loop prevention for dedicated document of b6403541 applied" );
#endif
                            }
                        }
                    }
                    // <--
                }

                if ( bExtra && Lower() && !Lower()->GetValidPosFlag() )
                {
                    // Wenn ein mehrspaltiger Rahmen wg. Positionswechsel ungueltige
                    // Spalten hinterlaesst, so drehen wir lieber hier eine weitere
                    // Runde und formatieren unseren Inhalt via FormatWidthCols nochmal.
                        _InvalidateSize();
                    bExtra = sal_False; // Sicherhaltshalber gibt es nur eine Ehrenrunde.
                }
            } while ( !IsValid() && !bOsz &&
                      // --> OD 2004-08-25 #i3317#
                      !bConsiderWrapInfluenceDueToOverlapPrevCol &&
                      // <--
                      // --> OD 2005-01-14 #i40444#
                      !bConsiderWrapInfluenceDueToMovedFwdAnchor &&
                      // <--
                      GetFmt()->GetDoc()->IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) );

            // --> OD 2004-08-25 #i3317# - instead of attribute change apply
            // temporarly the 'straightforward positioning process'.
            // --> OD 2007-11-29 #i80924#
            // handle special case during splitting of table rows
            if ( bConsiderWrapInfluenceDueToMovedFwdAnchor &&
                 GetAnchorFrm()->IsInTab() &&
                 GetAnchorFrm()->IsInFollowFlowRow() )
            {
                const SwFrm* pCellFrm = GetAnchorFrm();
                while ( pCellFrm && !pCellFrm->IsCellFrm() )
                {
                    pCellFrm = pCellFrm->GetUpper();
                }
                if ( pCellFrm )
                {
                    SWRECTFN( pCellFrm )
                    if ( (pCellFrm->Frm().*fnRect->fnGetTop)() == 0 &&
                         (pCellFrm->Frm().*fnRect->fnGetHeight)() == 0 )
                    {
                        bConsiderWrapInfluenceDueToMovedFwdAnchor = false;
                    }
                }
            }
            // <--
            if ( bOsz || bConsiderWrapInfluenceDueToOverlapPrevCol ||
                 // --> OD 2005-01-14 #i40444#
                 bConsiderWrapInfluenceDueToMovedFwdAnchor )
                 // <--
            {
                SetTmpConsiderWrapInfluence( true );
                SetRestartLayoutProcess( true );
                // --> OD 2006-07-24 #b6449874#
                SetTmpConsiderWrapInfluenceOfOtherObjs( true );
                // <--
            }
            // <--
            bSetCompletePaintOnInvalidate = sal_False;
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
    sal_Bool operator<( const SwDistance& rTwo )
        { return nMain < rTwo.nMain || ( nMain == rTwo.nMain && nSub &&
          rTwo.nSub && nSub < rTwo.nSub ); }
    sal_Bool operator<=( const SwDistance& rTwo )
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
        const bool bVert = pUp->IsVertical();
        //Dem Textflus folgen.
        // --> OD 2009-01-12 #i70582#
        const SwTwips nTopForObjPos =
            bVert
            ? ( pCnt->Frm().Left() +
                pCnt->Frm().Width() -
                pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() )
            : ( pCnt->Frm().Top() +
                pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() );
        // <--
        if ( pUp->Frm().IsInside( rPt ) )
        {
            // OD 26.09.2003 - <rPt> point is inside environment of given content frame
            // --> OD 2009-01-12 #i70582#
            if( bVert )
                rRet.nMain =  nTopForObjPos - rPt.X();
            else
                rRet.nMain =  rPt.Y() - nTopForObjPos;
            // <--
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
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, sal_False, pCnt );
            if( !pLay ||
                (bVert && (pLay->Frm().Top() + pLay->Prt().Bottom()) <rPt.Y())||
                (!bVert && (pLay->Frm().Left() + pLay->Prt().Right())<rPt.X()) )
            {
                // OD 26.09.2003 - <rPt> point is in left border of environment
                // --> OD 2009-01-12 #i70582#
                if( bVert )
                    rRet.nMain =  nTopForObjPos - rPt.X();
                else
                    rRet.nMain = rPt.Y() - nTopForObjPos;
                // <--
                return pCnt;
            }
            else
                rRet.nMain = LONG_MAX;
        }
        else
        {
            // --> OD 2009-01-12 #i70582#
            rRet.nMain = bVert
                ? nTopForObjPos - (pUp->Frm().Left() + pUp->Prt().Left())
                : (pUp->Frm().Top() + pUp->Prt().Bottom()) - nTopForObjPos;
            // <--

            const SwFrm *pPre = pCnt;
            const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, sal_True, pCnt );
            SwTwips nFrmTop = 0;
            SwTwips nPrtHeight = 0;
            sal_Bool bSct = sal_False;
            const SwSectionFrm *pSect = pUp->FindSctFrm();
            if( pSect )
            {
                rRet.nSub = rRet.nMain;
                rRet.nMain = 0;
            }
            if( pSect && !pSect->IsAnLower( pLay ) )
            {
                bSct = sal_False;
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
                    pLay = pLay->GetLeaf( MAKEPAGE_NONE, sal_True, pCnt );
                    if( pSect && !pSect->IsAnLower( pLay ) )
                    {   // If we're leaving a SwSectionFrm, the next Leaf-Frm
                        // is the part of the upper below the SectionFrm.
                        const SwSectionFrm* pNxtSect = pLay ?
                            pLay->FindSctFrm() : NULL;
                        bSct = sal_False;
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

sal_uLong MA_FASTCALL lcl_FindCntDiff( const Point &rPt, const SwLayoutFrm *pLay,
                          const SwCntntFrm *& rpCnt,
                          const sal_Bool bBody, const sal_Bool bFtn )
{
    //Sucht unterhalb von pLay den dichtesten Cnt zum Point. Der Bezugspunkt
    //der Cntnts ist immer die linke obere Ecke.
    //Der Cnt soll moeglichst ueber dem Point liegen.

#if OSL_DEBUG_LEVEL > 1
    Point arPoint( rPt );
#endif

    rpCnt = 0;
    sal_uLong nDistance = ULONG_MAX;
    sal_uLong nNearest  = ULONG_MAX;
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
            sal_uInt32 dX = Max( pCnt->Frm().Left(), rPt.X() ) -
                       Min( pCnt->Frm().Left(), rPt.X() ),
                  dY = Max( pCnt->Frm().Top(), rPt.Y() ) -
                       Min( pCnt->Frm().Top(), rPt.Y() );
            BigInt dX1( dX ), dY1( dY );
            dX1 *= dX1; dY1 *= dY1;
            const sal_uLong nDiff = ::SqRt( dX1 + dY1 );
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

const SwCntntFrm * MA_FASTCALL lcl_FindCnt( const Point &rPt, const SwCntntFrm *pCnt,
                                  const sal_Bool bBody, const sal_Bool bFtn )
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
    sal_uLong nDist;

    nDist = ::lcl_FindCntDiff( rPt, pLay, pNew, bBody, bFtn );
    if ( pNew )
        pRet = pNew;
    else
    {   pRet  = pCnt;
        nDist = ULONG_MAX;
    }
    const SwCntntFrm *pNearest = pRet;
    sal_uLong nNearest = nDist;

    if ( pLay )
    {
        const SwLayoutFrm *pPge = pLay;
        sal_uLong nOldNew = ULONG_MAX;
        for ( sal_uInt16 i = 0; pPge->GetPrev() && (i < 3); ++i )
        {
            pPge = (SwLayoutFrm*)pPge->GetPrev();
            const sal_uLong nNew = ::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFtn );
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
        for ( sal_uInt16 j = 0; pPge->GetNext() && (j < 3); ++j )
        {
            pPge = (SwLayoutFrm*)pPge->GetNext();
            const sal_uLong nNew = ::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFtn );
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
                              const sal_Bool bBodyOnly )
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
        pCnt = pTmpLay->GetCntntPos( aTmp, sal_False, bBodyOnly );
    }

    //Beim Suchen darauf achten, dass die Bereiche sinnvoll erhalten
    //bleiben. D.h. in diesem Fall nicht in Header/Footer hinein und
    //nicht aus Header/Footer hinaus.
    const sal_Bool bBody = pCnt->IsInDocBody() || bBodyOnly;
    const sal_Bool bFtn  = !bBodyOnly && pCnt->IsInFtn();

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
                                        GetCntntPos( aTmp, sal_False, sal_True, sal_False );
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
    sal_Bool bNegAllowed = sal_True;//Einmal aus dem negativen Bereich heraus lassen.
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
            bNegAllowed = sal_False;
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

    SwPageFrm *pTmpPage = 0;
    const bool bVert = pCnt->IsVertical();
    const sal_Bool bRTL = pCnt->IsRightToLeft();

    if( ( !bVert != !GetAnchorFrm()->IsVertical() ) ||
        ( !bRTL !=  !GetAnchorFrm()->IsRightToLeft() ) )
    {
        if( bVert || bRTL )
            aNew.X() += Frm().Width();
        else
            aNew.X() -= Frm().Width();
    }

    if ( pCnt->IsInDocBody() )
    {
        //#38848 Vom Seitenrand in den Body ziehen.
        pTmpPage = pCnt->FindPageFrm();
        ::lcl_PointToPrt( aNew, pTmpPage->GetUpper() );
        SwRect aTmp( aNew, Size( 0, 0 ) );
        pTmpPage = (SwPageFrm*)::FindPage( aTmp, pTmpPage );
        ::lcl_PointToPrt( aNew, pTmpPage );
    }

    //RelPos einstellen, nur auf Wunsch invalidieren.
    //rNew ist eine Absolute Position. Um die RelPos korrekt einzustellen
    //muessen wir uns die Entfernung von rNew zum Anker im Textfluss besorgen.
//!!!!!Hier kann Optimiert werden: FindAnchor koennte die RelPos mitliefern!
    const SwFrm *pFrm = 0;
    SwTwips nY;
    if ( pCnt->Frm().IsInside( aNew ) )
    {
        // --> OD 2009-01-12 #i70582#
        const SwTwips nTopForObjPos =
                bVert
                ? ( pCnt->Frm().Left() +
                    pCnt->Frm().Width() -
                    pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() )
                : ( pCnt->Frm().Top() +
                    pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() );
        if ( bVert )
        {
            nY = nTopForObjPos - rNew.X() - Frm().Width();
        }
        else
        {
            nY = rNew.Y() - nTopForObjPos;
        }
        // <--
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
        // --> OD 2009-01-12 #i70582#
        const SwTwips nTopForObjPos =
                bVert
                ? ( pCnt->Frm().Left() +
                    pCnt->Frm().Width() -
                    pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() )
                : ( pCnt->Frm().Top() +
                    pCnt->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid() );
        if ( bVert )
        {
            nY = nTopForObjPos - rNew.X();
        }
        else
        {
            nY = rNew.Y() - nTopForObjPos;
        }
        // <--
    }

    SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
    const SwFmtSurround& rSurround = pFmt->GetSurround();
    const sal_Bool bWrapThrough =
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
    GetFmt()->GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );

    if( pCnt != GetAnchorFrm() || ( IsAutoPos() && pCnt->IsTxtFrm() &&
                                  GetFmt()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::HTML_MODE)) )
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
                if( text::RelOrientation::CHAR == pFmt->GetVertOrient().GetRelationOrient() )
                    nY = LONG_MAX;
                if( text::RelOrientation::CHAR == pFmt->GetHoriOrient().GetRelationOrient() )
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

        // --> OD 2006-02-27 #125892#
        // handle change of anchor node:
        // if count of the anchor frame also change, the fly frames have to be
        // re-created. Thus, delete all fly frames except the <this> before the
        // anchor attribute is change and re-create them afterwards.
        {
            SwHandleAnchorNodeChg aHandleAnchorNodeChg( *pFmt, aAnch, this );
            pFmt->GetDoc()->SetAttr( aAnch, *pFmt );
        }
        // <--
    }
    // --> OD 2004-06-30 #i28701# - use new method <GetPageFrm()>
    else if ( pTmpPage && pTmpPage != GetPageFrm() )
        GetPageFrm()->MoveFly( this, pTmpPage );

    const Point aRelPos = bVert ? Point( -nY, nX ) : Point( nX, nY );
    ChgRelPos( aRelPos );

    GetFmt()->GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );

    if ( pOldPage != FindPageFrm() )
        ::Notify_Background( GetVirtDrawObj(), pOldPage, aOld, PREP_FLY_LEAVE,
                             sal_False );
}

// OD 2004-08-12 #i32795# - Note: method no longer used in <flyincnt.cxx>
//void DeepCalc( const SwFrm *pFrm )
//{
//    if( pFrm->IsSctFrm() ||
//        ( pFrm->IsFlyFrm() && ((SwFlyFrm*)pFrm)->IsFlyInCntFrm() ) )
//      return;
//    const SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( pFrm );
//    if( pFlow && pFlow->IsAnyJoinLocked() )
//        return;

//    sal_uInt16 nCnt = 0;

//  sal_Bool bContinue = sal_False;
//  do
//    {
//        if ( ++nCnt == 10 )
//      {
//          ASSERT( !nCnt, "DeepCalc: Loop detected1?" );
//          break;
//      }

//      const sal_Bool bSetComplete = !pFrm->IsValid();
//      const SwRect aOldFrm( pFrm->Frm() );
//      const SwRect aOldPrt( pFrm->Prt() );

//      const SwFrm *pUp = pFrm->GetUpper();
//      if ( pUp )
//      {
//          //Nicht weiter wenn der Up ein Fly mit Spalten ist.
//          if( ( !pUp->IsFlyFrm() || !((SwLayoutFrm*)pUp)->Lower() ||
//               !((SwLayoutFrm*)pUp)->Lower()->IsColumnFrm() ) &&
//               !pUp->IsSctFrm() )
//          {
//                SWRECTFN( pUp )
//                const Point aPt( (pUp->Frm().*fnRect->fnGetPos)() );
//              ::DeepCalc( pUp );
//                bContinue = aPt != (pUp->Frm().*fnRect->fnGetPos)();
//          }
//      }
//      else
//          pUp = pFrm;

//      pFrm->Calc();
//      if ( bSetComplete && (aOldFrm != pFrm->Frm() || aOldPrt != pFrm->Prt()))
//          pFrm->SetCompletePaint();

//      if ( pUp->IsFlyFrm() )
//      {
//          if ( ((SwFlyFrm*)pUp)->IsLocked() ||
//               (((SwFlyFrm*)pUp)->IsFlyAtCntFrm() &&
//                SwOszControl::IsInProgress( (const SwFlyFrm*)pUp )) )
//          {
//              bContinue = sal_False;
//          }
//      }
//  } while ( bContinue );
//}

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
    // if fly frame position is valid, nothing is to do. Thus, return
    if ( bValidPos )
    {
        return;
    }

    // OD 2004-03-24 #i26791# - validate position flag here.
    bValidPos = sal_True;

    // --> OD 2004-10-22 #i35911# - no calculation of new position, if
    // anchored object is marked that it clears its environment and its
    // environment is already cleared.
    // --> OD 2006-01-02 #125977# - before checking for cleared environment
    // check, if member <mpVertPosOrientFrm> is set.
    if ( GetVertPosOrientFrm() &&
         ClearedEnvironment() && HasClearedEnvironment() )
    {
        return;
    }
    // <--

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
