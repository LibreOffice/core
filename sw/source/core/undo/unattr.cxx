/*************************************************************************
 *
 *  $RCSfile: unattr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:27 $
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

#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif


#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer die UndoIds
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _SWATRSET_HXX
#include <swatrset.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif

inline SwDoc& SwUndoIter::GetDoc() const
{ return *pAktPam->GetDoc(); }

// -----------------------------------------------------

_UndoFmtAttr::_UndoFmtAttr( SwFmt& rFmt, BOOL bSvDrwPt )
    : SwClient( &rFmt ), pUndo( 0 ), bSaveDrawPt( bSvDrwPt )
{
}

void _UndoFmtAttr::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    if( pOld && pNew )
    {
        if( POOLATTR_END >= pOld->Which() )
        {
            if( pUndo )
                pUndo->PutAttr( *pOld );
            else
                pUndo = new SwUndoFmtAttr( *pOld, *(SwFmt*)pRegisteredIn,
                                            bSaveDrawPt );
        }
        else if( RES_ATTRSET_CHG == pOld->Which() )
        {
            if( pUndo )
            {
                SfxItemIter aIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
                const SfxPoolItem* pItem = aIter.GetCurItem();
                while( pItem )
                {
                    pUndo->PutAttr( *pItem );
                    if( aIter.IsAtEnd() )
                        break;
                    pItem = aIter.NextItem();
                }
            }
            else
                pUndo = new SwUndoFmtAttr( *((SwAttrSetChg*)pOld)->GetChgSet(),
                                        *(SwFmt*)pRegisteredIn, bSaveDrawPt );
        }
        else
            SwClient::Modify( pOld, pNew );
    }
    else
        SwClient::Modify( pOld, pNew );
}

SwUndoFmtAttr::SwUndoFmtAttr( const SfxItemSet& rSet, SwFmt& rChgFmt,
                                BOOL bSvDrwPt )
    : SwUndo( UNDO_INSFMTATTR ), pFmt( &rChgFmt ),
    nFmtWhich( rChgFmt.Which() ), nNode( 0 ), bSaveDrawPt( bSvDrwPt )
{
    pOldSet = new SfxItemSet( rSet );
    Init();
}

SwUndoFmtAttr::SwUndoFmtAttr( const SfxPoolItem& rItem, SwFmt& rChgFmt,
                                BOOL bSvDrwPt )
    : SwUndo( UNDO_INSFMTATTR ), pFmt( &rChgFmt ),
    nFmtWhich( rChgFmt.Which() ), nNode( 0 ), bSaveDrawPt( bSvDrwPt )
{
    pOldSet = pFmt->GetAttrSet().Clone( FALSE );
    pOldSet->Put( rItem );
    Init();
}

void SwUndoFmtAttr::Init()
{
    // Ankerwechsel gesondert behandeln
    if( SFX_ITEM_SET == pOldSet->GetItemState( RES_ANCHOR, FALSE ))
        SaveFlyAnchor( bSaveDrawPt );
    else if( RES_FRMFMT == nFmtWhich )
    {
        SwDoc* pDoc = pFmt->GetDoc();
        if( USHRT_MAX !=
            pDoc->GetTblFrmFmts()->GetPos( (const SwFrmFmtPtr)pFmt ))
        {
            // TabellenFormat -> Tabellen Index Position merken, TabellenFormate
            //                      sind fluechtig!
            SwClient* pTbl = SwClientIter( *pFmt ).First( TYPE( SwTable ));
            if( pTbl )
                nNode = ((SwTable*)pTbl)->GetTabSortBoxes()[ 0 ]->
                            GetSttNd()->FindTableNode()->GetIndex();
        }
        else if( USHRT_MAX !=
            pDoc->GetSections().GetPos( (const SwSectionFmtPtr)pFmt ))
            nNode = pFmt->GetCntnt().GetCntntIdx()->GetIndex();
    }
}

SwUndoFmtAttr::~SwUndoFmtAttr()
{
    delete pOldSet;
}

void SwUndoFmtAttr::Undo( SwUndoIter& rUndoIter)
{
    if( !pOldSet || !pFmt || !IsFmtInDoc( &rUndoIter.GetDoc() ))
        return;

    if( SFX_ITEM_SET == pOldSet->GetItemState( RES_ANCHOR, FALSE ))
    {
        RestoreFlyAnchor( rUndoIter );
        SaveFlyAnchor();
    }
    else
    {
        _UndoFmtAttr aTmp( *pFmt, bSaveDrawPt );
        pFmt->SetAttr( *pOldSet );
        if( aTmp.pUndo )
        {
            delete pOldSet;
            pOldSet = aTmp.pUndo->pOldSet;
            aTmp.pUndo->pOldSet = 0;    // den Pointer auf 0 setzen (nicht
                                        // doppelt loeschen) !!
            delete aTmp.pUndo;          // Undo-Object wieder loeschen
        }
        else
            pOldSet->ClearItem();

        if( RES_FLYFRMFMT == nFmtWhich || RES_DRAWFRMFMT == nFmtWhich )
            rUndoIter.pSelFmt = (SwFrmFmt*)pFmt;
    }
}

int SwUndoFmtAttr::IsFmtInDoc( SwDoc* pDoc )
{
    // suche im Dokument nach dem Format. Ist es nicht mehr vorhanden
    // so wird das Attribut nicht mehr gesetzt !
    USHORT nPos = USHRT_MAX;
    switch( nFmtWhich )
    {
    case RES_TXTFMTCOLL:
        nPos = pDoc->GetTxtFmtColls()->GetPos(
                                        (const SwTxtFmtCollPtr)pFmt );
        break;

    case RES_GRFFMTCOLL:
        nPos = pDoc->GetGrfFmtColls()->GetPos(
                                        (const SwGrfFmtCollPtr)pFmt );
        break;
    case RES_CHRFMT:
        nPos = pDoc->GetCharFmts()->GetPos(
                                    (const SwCharFmtPtr)pFmt );
        break;

    case RES_FRMFMT:
        if( nNode && nNode < pDoc->GetNodes().Count() )
        {
            SwNode* pNd = pDoc->GetNodes()[ nNode ];
            if( pNd->IsTableNode() )
            {
                pFmt = ((SwTableNode*)pNd)->GetTable().GetFrmFmt();
                nPos = 0;
                break;
            }
            else if( pNd->IsSectionNode() )
            {
                pFmt = ((SwSectionNode*)pNd)->GetSection().GetFmt();
                nPos = 0;
                break;
            }
        }
        // kein break!
    case RES_DRAWFRMFMT:
    case RES_FLYFRMFMT:
        if( USHRT_MAX == ( nPos = pDoc->GetSpzFrmFmts()->GetPos(
                            (const SwFrmFmtPtr)pFmt )) )
            nPos = pDoc->GetFrmFmts()->GetPos(
                            (const SwFrmFmtPtr)pFmt );
        break;
    }

    // Format nicht mehr vorhanden, zurueck
    if( USHRT_MAX == nPos )
        pFmt = 0;

    return 0 != pFmt;
}

// prueft, ob es noch im Doc ist!
SwFmt* SwUndoFmtAttr::GetFmt( SwDoc& rDoc )
{
    return pFmt && IsFmtInDoc( &rDoc ) ? pFmt : 0;
}

void SwUndoFmtAttr::Redo( SwUndoIter& rUndoIter)
{
    if( !pOldSet || !pFmt || !IsFmtInDoc( &rUndoIter.GetDoc() ))
        return;

    if( SFX_ITEM_SET == pOldSet->GetItemState( RES_ANCHOR, FALSE ))
    {
        RestoreFlyAnchor( rUndoIter );
        SaveFlyAnchor();
        return;     // der Rest passierte schon im RestoreFlyAnchor !!
    }
    else
    {
        _UndoFmtAttr aTmp( *pFmt, bSaveDrawPt );
        pFmt->SetAttr( *pOldSet );
        if( aTmp.pUndo )
        {
            delete pOldSet;
            pOldSet = aTmp.pUndo->pOldSet;
            aTmp.pUndo->pOldSet = 0;    // den Pointer auf 0 setzen (nicht
                                        // doppelt loeschen) !!
            delete aTmp.pUndo;          // Undo-Object wieder loeschen
        }
        else
            pOldSet->ClearItem();

        if( RES_FLYFRMFMT == nFmtWhich || RES_DRAWFRMFMT == nFmtWhich )
            rUndoIter.pSelFmt = (SwFrmFmt*)pFmt;
    }
}

void SwUndoFmtAttr::Repeat( SwUndoIter& rUndoIter)
{
    if( !pOldSet )
        return;

    SwUndoFmtAttr* pLast;
    if( UNDO_INSFMTATTR == rUndoIter.GetLastUndoId() &&
        ( pLast = ((SwUndoFmtAttr*)rUndoIter.pLastUndoObj))->pOldSet &&
        pLast->pFmt )
        return;

    switch( nFmtWhich )
    {
    case RES_GRFFMTCOLL:
        {
            SwNoTxtNode * pNd = rUndoIter.pAktPam->GetNode()->GetNoTxtNode();
            if( pNd )
                rUndoIter.GetDoc().SetAttr( pFmt->GetAttrSet(),
                                            *pNd->GetFmtColl() );
        }
        break;

    case RES_TXTFMTCOLL:
        {
            SwTxtNode * pNd = rUndoIter.pAktPam->GetNode()->GetTxtNode();
            if( pNd )
                rUndoIter.GetDoc().SetAttr( pFmt->GetAttrSet(),
                                            *pNd->GetFmtColl() );
        }
        break;

//  case RES_CHRFMT:
//  case RES_FRMFMT:

    case RES_FLYFRMFMT:
        {
            // erstal pruefen, ob der Cursor ueberhaupt in einem fliegenden
            // Rahmen steht. Der Weg ist: suche in allen FlyFrmFormaten
            // nach dem FlyCntnt-Attribut und teste ob der Cursor in der
            // entsprechenden Section liegt.
            SwFrmFmt* pFly = rUndoIter.pAktPam->GetNode()->GetFlyFmt();
            if( pFly )
            {
                // Bug 43672: es duerfen nicht alle Attribute gesetzt werden!
                if( SFX_ITEM_SET == pFmt->GetAttrSet().GetItemState( RES_CNTNT ))
                {
                    SfxItemSet aTmpSet( pFmt->GetAttrSet() );
                    aTmpSet.ClearItem( RES_CNTNT );
                    if( aTmpSet.Count() )
                        rUndoIter.GetDoc().SetAttr( aTmpSet, *pFly );
                }
                else
                    rUndoIter.GetDoc().SetAttr( pFmt->GetAttrSet(), *pFly );
            }
            break;
        }
    }

    rUndoIter.pLastUndoObj = this;
}

void SwUndoFmtAttr::PutAttr( const SfxPoolItem& rItem )
{
    pOldSet->Put( rItem );
    if( RES_ANCHOR == rItem.Which() )
        SaveFlyAnchor( bSaveDrawPt );
}

void SwUndoFmtAttr::SaveFlyAnchor( BOOL bSvDrwPt )
{
    // das Format ist gueltig, sonst wuerde man gar bis hier kommen
    if( bSvDrwPt )
    {
        if( RES_DRAWFRMFMT == pFmt->Which() )
        {
            Point aPt( ((SwFrmFmt*)pFmt)->FindSdrObject()->GetRelativePos() );
            // den alten Wert als zwischenspeichern. Attribut dafuer benutzen,
            // dadurch bleibt der SwUndoFmtAttr klein.
            pOldSet->Put( SwFmtFrmSize( ATT_VAR_SIZE, aPt.X(), aPt.Y() ) );
        }
/*      else
        {
            pOldSet->Put( pFmt->GetVertOrient() );
            pOldSet->Put( pFmt->GetHoriOrient() );
        }
*/  }

    const SwFmtAnchor& rAnchor = (SwFmtAnchor&)pOldSet->Get(
                                                RES_ANCHOR, FALSE );
    if( !rAnchor.GetCntntAnchor() )
        return;

    xub_StrLen nCntnt = 0;
    switch( rAnchor.GetAnchorId() )
    {
    case FLY_IN_CNTNT:
    case FLY_AUTO_CNTNT:
        nCntnt = rAnchor.GetCntntAnchor()->nContent.GetIndex();
    case FLY_AT_CNTNT:
    case FLY_AT_FLY:
        nNode = rAnchor.GetCntntAnchor()->nNode.GetIndex();
        break;
    default:
        return;
    }

    SwFmtAnchor aAnchor( rAnchor.GetAnchorId(), nCntnt );
    pOldSet->Put( aAnchor );
}

void SwUndoFmtAttr::RestoreFlyAnchor( SwUndoIter& rIter )
{
    SwDoc* pDoc = &rIter.GetDoc();
    SwFlyFrmFmt* pFrmFmt = (SwFlyFrmFmt*)pFmt;
    const SwFmtAnchor& rAnchor = (SwFmtAnchor&)pOldSet->Get(
                                                RES_ANCHOR, FALSE );

    SwFmtAnchor aNewAnchor( rAnchor.GetAnchorId() );
    if( FLY_PAGE != rAnchor.GetAnchorId() )
    {
        SwNode* pNd = pDoc->GetNodes()[ nNode  ];

        if( FLY_AT_FLY == rAnchor.GetAnchorId() ? ( !pNd->IsStartNode() ||
            SwFlyStartNode != ((SwStartNode*)pNd)->GetStartNodeType() ) :
            !pNd->IsTxtNode() )
            return;     // ungueltige Position

        SwPosition aPos( *pNd );
        if( FLY_IN_CNTNT == rAnchor.GetAnchorId() ||
            FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
        {
            aPos.nContent.Assign( (SwTxtNode*)pNd, rAnchor.GetPageNum() );
            if( aPos.nContent.GetIndex() > ((SwTxtNode*)pNd)->GetTxt().Len() )
                return;     // ungueltige Position
        }
        aNewAnchor.SetAnchor( &aPos );
    }
    else
        aNewAnchor.SetPageNum( rAnchor.GetPageNum() );

    Point aDrawSavePt, aDrawOldPt;
    if( pDoc->GetRootFrm() )
    {
        if( RES_DRAWFRMFMT == pFrmFmt->Which() )
        {
            // den alten zwischengespeicherten Wert herausholen.
            const SwFmtFrmSize& rOldSize = (const SwFmtFrmSize&)
                                            pOldSet->Get( RES_FRM_SIZE );
            aDrawSavePt.X() = rOldSize.GetWidth();
            aDrawSavePt.Y() = rOldSize.GetHeight();
            pOldSet->ClearItem( RES_FRM_SIZE );

            // den akt. wieder zwischenspeichern
            aDrawOldPt = pFrmFmt->FindSdrObject()->GetRelativePos();
//JP 08.10.97: ist laut AMA/MA nicht mehr noetig
//          pCont->DisconnectFromLayout();
        }
        else
            pFrmFmt->DelFrms();         // Frms vernichten.
    }

    const SwFmtAnchor &rOldAnch = pFrmFmt->GetAnchor();
    if( FLY_IN_CNTNT == rOldAnch.GetAnchorId() &&
        FLY_IN_CNTNT != aNewAnchor.GetAnchorId() )
    {
        //Bei InCntnt's wird es spannend: Das TxtAttribut muss vernichtet
        //werden. Leider reisst dies neben den Frms auch noch das Format mit
        //in sein Grab. Um dass zu unterbinden loesen wir vorher die
        //Verbindung zwischen Attribut und Format.
        const SwPosition *pPos = rOldAnch.GetCntntAnchor();
        SwTxtNode *pTxtNode = (SwTxtNode*)&pPos->nNode.GetNode();
        ASSERT( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
        const xub_StrLen nIdx = pPos->nContent.GetIndex();
        SwTxtAttr * pHnt = pTxtNode->GetTxtAttr( nIdx, RES_TXTATR_FLYCNT );
#ifndef PRODUCT
        ASSERT( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                    "Missing FlyInCnt-Hint." );
        ASSERT( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == pFrmFmt,
                    "Wrong TxtFlyCnt-Hint." );
#endif
        ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).SetFlyFmt();

        //Die Verbindung ist geloest, jetzt muss noch das Attribut vernichtet
        //werden.
        pTxtNode->Delete( RES_TXTATR_FLYCNT, nIdx, nIdx );
    }

    {
        pOldSet->Put( aNewAnchor );
        _UndoFmtAttr aTmp( *pFmt, bSaveDrawPt );
        pFmt->SetAttr( *pOldSet );
        if( aTmp.pUndo )
        {
            delete pOldSet;
            nNode = aTmp.pUndo->nNode;
            pOldSet = aTmp.pUndo->pOldSet;
            aTmp.pUndo->pOldSet = 0;    // den Pointer auf 0 setzen (nicht
                                        // doppelt loeschen) !!
            delete aTmp.pUndo;          // Undo-Object wieder loeschen
        }
        else
            pOldSet->ClearItem();
    }

    if( RES_DRAWFRMFMT == pFrmFmt->Which() )
    {
        SwDrawContact *pCont = (SwDrawContact*)pFrmFmt->FindContactObj();
        // das Draw-Model hat auch noch ein Undo-Object fuer die
        // richtige Position vorbereitet; dieses ist aber relativ.
        // Darum verhinder hier, das durch setzen des Ankers das
        // Contact-Object seine Position aendert.
//JP 08.10.97: ist laut AMA/MA nicht mehr noetig
//          pCont->ConnectToLayout();
        SdrObject* pObj = pCont->GetMaster();

        if( pCont->GetAnchor() && !pObj->IsInserted() )
        {
            ASSERT( pDoc->GetDrawModel(), "RestoreFlyAnchor without DrawModel" );
            pDoc->GetDrawModel()->GetPage( 0 )->InsertObject( pObj );
        }
        pObj->SetRelativePos( aDrawSavePt );

        // den alten Wert wieder zwischenspeichern.
        pOldSet->Put( SwFmtFrmSize( ATT_VAR_SIZE, aDrawOldPt.X(), aDrawOldPt.Y() ) );
    }

    if( FLY_IN_CNTNT == aNewAnchor.GetAnchorId() )
    {
        const SwPosition* pPos = aNewAnchor.GetCntntAnchor();
        SwTxtNode* pTxtNd = pPos->nNode.GetNode().GetTxtNode();
        ASSERT( pTxtNd, "Kein Textnode an dieser Position" );
        pTxtNd->Insert( SwFmtFlyCnt( pFrmFmt ), pPos->nContent.GetIndex(), 0 );
    }


    if( RES_DRAWFRMFMT != pFrmFmt->Which() )
        pFrmFmt->MakeFrms();

    rIter.pSelFmt = pFrmFmt;
}

/*  */

SwUndoRstAttr::SwUndoRstAttr( const SwPaM& rRange, USHORT nFmt )
    : SwUndo( UNDO_RESETATTR ), SwUndRng( rRange ), nFmtId( nFmt ),
    pHistory( new SwHistory )
{
#ifdef COMPACT
    ((SwDoc*)rRange.GetDoc())->DelUndoGroups();
#endif
}

SwUndoRstAttr::SwUndoRstAttr( const SwDoc& rDoc, const SwPosition& rPos,
                                USHORT nWhich )
    : SwUndo( UNDO_RESETATTR ), nFmtId( nWhich ), pHistory( new SwHistory )
{
    nSttNode = nEndNode = rPos.nNode.GetIndex();
    nSttCntnt = nEndCntnt = rPos.nContent.GetIndex();
}

SwUndoRstAttr::~SwUndoRstAttr()
{
    delete pHistory;
}

void SwUndoRstAttr::Undo( SwUndoIter& rUndoIter )
{
    // die alten Werte wieder zurueck
    SwDoc& rDoc = rUndoIter.GetDoc();
    pHistory->TmpRollback( &rDoc, 0 );
    pHistory->SetTmpEnd( pHistory->Count() );

    if( RES_CONDTXTFMTCOLL == nFmtId && nSttNode == nEndNode &&
        nSttCntnt == nEndCntnt )
    {
        SwTxtNode* pTNd = rDoc.GetNodes()[ nSttNode ]->GetTxtNode();
        if( pTNd )
        {
            SwIndex aIdx( pTNd, nSttCntnt );
            pTNd->DontExpandFmt( aIdx, FALSE );
        }
    }

    // setze noch den Cursor auf den Undo-Bereich
    SetPaM( rUndoIter );
}

void SwUndoRstAttr::Redo( SwUndoIter& rUndoIter )
{
    // setze Attribut in dem Bereich:
    SetPaM( rUndoIter );
    SwDoc& rDoc = rUndoIter.GetDoc();
    rUndoIter.pLastUndoObj = 0;
    SvUShortsSort* pIdArr = aIds.Count() ? &aIds : 0;

    switch( nFmtId )
    {
    case RES_CHRFMT:
        rUndoIter.GetDoc().RstTxtAttr( *rUndoIter.pAktPam );
        break;
    case RES_TXTFMTCOLL:
        rUndoIter.GetDoc().ResetAttr( *rUndoIter.pAktPam, FALSE, pIdArr );
        break;
    case RES_CONDTXTFMTCOLL:
        rUndoIter.GetDoc().ResetAttr( *rUndoIter.pAktPam, TRUE, pIdArr );

        break;
    case RES_TXTATR_TOXMARK:
        // Sonderbehandlung fuer TOXMarks
        {
            SwTOXMarks aArr;
            SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode );
            SwPosition aPos( aIdx, SwIndex( aIdx.GetNode().GetCntntNode(),
                                                                nSttCntnt ));

            USHORT nCnt = rDoc.GetCurTOXMark( aPos, aArr );
            if( nCnt )
            {
                if( 1 < nCnt )
                {
                    // dann den richtigen suchen
                    SwHstryHint* pHHint = (*GetHistory())[ 0 ];
                    if( pHHint && HSTRY_SETTOXMARKHNT == pHHint->Which() )
                    {
                        while( nCnt )
                            if( ((SwSetTOXMarkHint*)pHHint)->IsEqual(
                                    *aArr[ --nCnt ] ) )
                            {
                                ++nCnt;
                                break;
                            }
                    }
                    else
                        nCnt = 0;
                }
                // gefunden, also loeschen
                if( nCnt-- )
                    rDoc.Delete( aArr[ nCnt ] );
            }
        }
        break;
    }
    rUndoIter.pLastUndoObj = 0;
}

void SwUndoRstAttr::Repeat( SwUndoIter& rUndoIter )
{
    if( RES_FMT_BEGIN > nFmtId ||
        ( UNDO_RESETATTR == rUndoIter.GetLastUndoId() &&
         nFmtId == ((SwUndoRstAttr*)rUndoIter.pLastUndoObj)->nFmtId ))
        return;

    SvUShortsSort* pIdArr = aIds.Count() ? &aIds : 0;
    switch( nFmtId )
    {
    case RES_CHRFMT:
        rUndoIter.GetDoc().RstTxtAttr( *rUndoIter.pAktPam );
        break;
    case RES_TXTFMTCOLL:
        rUndoIter.GetDoc().ResetAttr( *rUndoIter.pAktPam, FALSE, pIdArr );
        break;
    case RES_CONDTXTFMTCOLL:
        rUndoIter.GetDoc().ResetAttr( *rUndoIter.pAktPam, TRUE, pIdArr );
        break;
    }
    rUndoIter.pLastUndoObj = this;
}


void SwUndoRstAttr::SetAttrs( const SvUShortsSort& rArr )
{
    if( aIds.Count() )
        aIds.Remove( 0, aIds.Count() );
    aIds.Insert( &rArr );
}

// -----------------------------------------------------



SwUndoAttr::SwUndoAttr( const SwPaM& rRange, const SfxPoolItem& rAttr,
                        USHORT nFlags )
    : SwUndo( UNDO_INSATTR ), SwUndRng( rRange ),
    aSet( rRange.GetDoc()->GetAttrPool(), rAttr.Which(), rAttr.Which() ),
    nInsFlags( nFlags ), pHistory( new SwHistory ),
    pRedlData( 0 ), pRedlSaveData( 0 ),
    nNdIdx( ULONG_MAX )
{
    aSet.Put( rAttr );
#ifdef COMPACT
    (SwDoc*)rRange.GetDoc()->DelUndoGroups();
#endif
}

SwUndoAttr::SwUndoAttr( const SwPaM& rRange, const SfxItemSet& rSet,
                        USHORT nFlags )
    : SwUndo( UNDO_INSATTR ), SwUndRng( rRange ), aSet( rSet ),
    nInsFlags( nFlags ), pHistory( new SwHistory ),
    pRedlData( 0 ), pRedlSaveData( 0 ),
    nNdIdx( ULONG_MAX )
{
#ifdef COMPACT
    (SwDoc*)rRange.GetDoc()->DelUndoGroups();
#endif
}

SwUndoAttr::~SwUndoAttr()
{
    delete pHistory;
    delete pRedlData;
    delete pRedlSaveData;
}

void SwUndoAttr::SaveRedlineData( const SwPaM& rPam, BOOL bIsCntnt )
{
    SwDoc* pDoc = rPam.GetDoc();
    if( pDoc->IsRedlineOn() )
        pRedlData = new SwRedlineData( bIsCntnt ? REDLINE_INSERT
                                                : REDLINE_FORMAT,
                                        pDoc->GetRedlineAuthor() );

    pRedlSaveData = new SwRedlineSaveDatas;
    if( !FillSaveDataForFmt( rPam, *pRedlSaveData ))
        delete pRedlSaveData, pRedlSaveData = 0;

    SetRedlineMode( pDoc->GetRedlineMode() );
    if( bIsCntnt )
        nNdIdx = rPam.GetPoint()->nNode.GetIndex();
}

void SwUndoAttr::Undo( SwUndoIter& rUndoIter )
{
    SwDoc* pDoc = &rUndoIter.GetDoc();

    RemoveIdx( *pDoc );

    if( IsRedlineOn( GetRedlineMode() ) )
    {
        SwPaM& rPam = *rUndoIter.pAktPam;
        if( ULONG_MAX != nNdIdx )
        {
            rPam.DeleteMark();
            rPam.GetPoint()->nNode = nNdIdx;
            rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), nSttCntnt );
            rPam.SetMark();
            rPam.GetPoint()->nContent++;
            pDoc->DeleteRedline( rPam, FALSE );
        }
        else
        {
            // alle Format-Redlines entfernen, werden ggfs. neu gesetzt
            SetPaM( rUndoIter );
            pDoc->DeleteRedline( rPam, FALSE, REDLINE_FORMAT );
            if( pRedlSaveData )
                SetSaveData( *pDoc, *pRedlSaveData );
        }
    }

    BOOL bToLast = 1 == aSet.Count() &&
                   RES_TXTATR_FIELD <= *aSet.GetRanges() &&
                   *aSet.GetRanges() <= RES_TXTATR_HARDBLANK;

    // die alten Werte wieder zurueck
    pHistory->TmpRollback( pDoc, 0, !bToLast );
    pHistory->SetTmpEnd( pHistory->Count() );

    // setze noch den Cursor auf den Undo-Bereich
    SetPaM( rUndoIter );
}

void SwUndoAttr::Repeat( SwUndoIter& rUndoIter )
{
    // RefMarks sind nicht repeatfaehig
    if( SFX_ITEM_SET != aSet.GetItemState( RES_TXTATR_REFMARK, FALSE ) )
        rUndoIter.GetDoc().Insert( *rUndoIter.pAktPam, aSet, nInsFlags );
    else if( 1 < aSet.Count() )
    {
        SfxItemSet aTmpSet( aSet );
        aTmpSet.ClearItem( RES_TXTATR_REFMARK );
        rUndoIter.GetDoc().Insert( *rUndoIter.pAktPam, aTmpSet, nInsFlags );
    }
    rUndoIter.pLastUndoObj = this;
}

void SwUndoAttr::Redo( SwUndoIter& rUndoIter )
{
    // setze Attribut in dem Bereich:
    SetPaM( rUndoIter );
    SwPaM& rPam = *rUndoIter.pAktPam;
    SwDoc& rDoc = rUndoIter.GetDoc();

    if( pRedlData && IsRedlineOn( GetRedlineMode() ) )
    {
        SwRedlineMode eOld = rDoc.GetRedlineMode();
        rDoc.SetRedlineMode_intern( eOld & ~REDLINE_IGNORE );
        rDoc.Insert( rPam, aSet, nInsFlags );

        if( ULONG_MAX != nNdIdx )
        {
            rPam.SetMark();
            if( rPam.Move( fnMoveBackward ) )
                rDoc.AppendRedline( new SwRedline( *pRedlData, rPam ));
            rPam.DeleteMark();
        }
        else
            rDoc.AppendRedline( new SwRedline( *pRedlData, rPam ));

        rDoc.SetRedlineMode_intern( eOld );
    }
    else
        rDoc.Insert( rPam, aSet, nInsFlags );

    rUndoIter.pLastUndoObj = 0;
}


void SwUndoAttr::RemoveIdx( SwDoc& rDoc )
{
    if( SFX_ITEM_SET != aSet.GetItemState( RES_TXTATR_FTN, FALSE ))
        return ;

    SwHstryHint* pHstHnt;
    SwNodes& rNds = rDoc.GetNodes();
    for( USHORT n = 0; n < pHistory->Count(); ++n )
    {
        xub_StrLen nCntnt;
        ULONG nNode = 0;
        switch( ( pHstHnt = (*pHistory)[ n ] )->Which() )
        {
        case HSTRY_RESETTXTHNT:
            if( RES_TXTATR_FTN == ((SwResetTxtHint*)pHstHnt)->GetWhich() )
            {
                nNode = ((SwResetTxtHint*)pHstHnt)->GetNode();
                nCntnt = ((SwResetTxtHint*)pHstHnt)->GetCntnt();
            }
            break;
        case HSTRY_RESETATTRSET:
            if( STRING_MAXLEN != ( nCntnt =
                                ((SwHstryResetAttrSet*)pHstHnt)->GetCntnt() ))
            {
                const SvUShorts& rArr = ((SwHstryResetAttrSet*)pHstHnt)->GetArr();
                for( USHORT i = rArr.Count(); i; )
                    if( RES_TXTATR_FTN == rArr[ --i ] )
                    {
                        nNode = ((SwHstryResetAttrSet*)pHstHnt)->GetNode();
                        break;
                    }
            }
            break;
        }

        if( nNode )
        {
            SwTxtNode* pTxtNd = rNds[ nNode ]->GetTxtNode();
            if( pTxtNd )
            {
                SwIndex aIdx( pTxtNd, nCntnt );
                SwTxtAttr * pTxtHt = pTxtNd->GetTxtAttr( aIdx, RES_TXTATR_FTN );
                if( pTxtHt )
                {
                    // ok, dann hole mal die Werte
                    SwTxtFtn* pFtn = (SwTxtFtn*)pTxtHt;
                    RemoveIdxFromSection( rDoc, pFtn->GetStartNode()->GetIndex() );
                    return ;
                }
            }
        }
    }
}

/*  */

SwUndoDefaultAttr::SwUndoDefaultAttr( const SfxItemSet& rSet )
    : SwUndo( UNDO_SETDEFTATTR ), pOldSet( 0 ), pTabStop( 0 )
{
    const SfxPoolItem* pItem;
    if( rSet.GetItemState( RES_PARATR_TABSTOP, FALSE, &pItem ) )
    {
        pTabStop = (SvxTabStopItem*)pItem->Clone();     // gesondert merken, aendert sich !!!
        if( 1 != rSet.Count() )         // gibts noch mehr Attribute ?
            pOldSet = new SfxItemSet( rSet );
    }
    else
        pOldSet = new SfxItemSet( rSet );
}

SwUndoDefaultAttr::~SwUndoDefaultAttr()
{
    if( pOldSet )
        delete pOldSet;
    if( pTabStop )
        delete pTabStop;
}

void SwUndoDefaultAttr::Undo( SwUndoIter& rUndoIter)
{
    SwDoc& rDoc = rUndoIter.GetDoc();
    if( pOldSet )
    {
        _UndoFmtAttr aTmp( *(SwTxtFmtColl*)rDoc.GetDfltTxtFmtColl() );
        rDoc.SetDefault( *pOldSet );
        delete pOldSet;
        if( aTmp.pUndo )
        {
            pOldSet = aTmp.pUndo->pOldSet;
            aTmp.pUndo->pOldSet = 0;    // den Pointer auf 0 setzen (nicht
                                        // doppelt loeschen) !!
            delete aTmp.pUndo;          // Undo-Object wieder loeschen
        }
        else
            pOldSet = 0;
    }
    if( pTabStop )
    {
        SvxTabStopItem* pOld = (SvxTabStopItem*)rDoc.GetDefault(
                                                RES_PARATR_TABSTOP ).Clone();
        rDoc.SetDefault( *pTabStop );
        delete pTabStop;
        pTabStop = pOld;
    }
}

void SwUndoDefaultAttr::Redo( SwUndoIter& rUndoIter)
{
    Undo( rUndoIter );
}

/*  */

SwUndoMoveLeftMargin::SwUndoMoveLeftMargin( const SwPaM& rPam, BOOL bFlag,
                                            BOOL bMod )
    : SwUndo( bFlag ? UNDO_INC_LEFTMARGIN : UNDO_DEC_LEFTMARGIN ),
    SwUndRng( rPam ), bModulus( bMod )
{
    pHistory = new SwHistory;
}

SwUndoMoveLeftMargin::~SwUndoMoveLeftMargin()
{
    delete pHistory;
}

void SwUndoMoveLeftMargin::Undo( SwUndoIter& rIter )
{
    SwDoc* pDoc = &rIter.GetDoc();
    BOOL bUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    // die alten Werte wieder zurueck
    pHistory->TmpRollback( pDoc, 0 );
    pHistory->SetTmpEnd( pHistory->Count() );

    pDoc->DoUndo( bUndo );
    SetPaM( rIter );
}

void SwUndoMoveLeftMargin::Redo( SwUndoIter& rIter )
{
    SwDoc* pDoc = &rIter.GetDoc();
    SetPaM( rIter );
    pDoc->MoveLeftMargin( *rIter.pAktPam, GetId() == UNDO_INC_LEFTMARGIN, bModulus );
}

void SwUndoMoveLeftMargin::Repeat( SwUndoIter& rIter )
{
    SwDoc* pDoc = &rIter.GetDoc();
    pDoc->MoveLeftMargin( *rIter.pAktPam, GetId() == UNDO_INC_LEFTMARGIN, bModulus );
    rIter.pLastUndoObj = this;
}

/*  */

SwUndoChgFtn::SwUndoChgFtn( const SwPaM& rRange, const String& rTxt,
                            USHORT nNum, BOOL bIsEndNote )
    : SwUndo( UNDO_CHGFTN ), SwUndRng( rRange ),
    sTxt( rTxt ), nNo( nNum ), bEndNote( bIsEndNote ),
    pHistory( new SwHistory() )
{
}

SwUndoChgFtn::~SwUndoChgFtn()
{
    delete pHistory;
}

void SwUndoChgFtn::Undo( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();
    SetPaM( rIter );

    BOOL bUndo = rDoc.DoesUndo();
    rDoc.DoUndo( FALSE );

    pHistory->TmpRollback( &rDoc, 0 );
    pHistory->SetTmpEnd( pHistory->Count() );

    rDoc.GetFtnIdxs().UpdateAllFtn();

    SetPaM( rIter );
    rDoc.DoUndo( bUndo );
}

void SwUndoChgFtn::Redo( SwUndoIter& rIter )
{
    SetPaM( rIter );
    rIter.GetDoc().SetCurFtn( *rIter.pAktPam, sTxt, nNo, bEndNote );
    SetPaM( rIter );
}

void SwUndoChgFtn::Repeat( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();
    rDoc.SetCurFtn( *rIter.pAktPam, sTxt, nNo, bEndNote );
    rIter.pLastUndoObj = this;
}


/*  */


SwUndoFtnInfo::SwUndoFtnInfo( const SwFtnInfo &rInfo ) :
    SwUndo( UNDO_FTNINFO ),
    pFtnInfo( new SwFtnInfo( rInfo ) )
{
}

SwUndoFtnInfo::~SwUndoFtnInfo()
{
    delete pFtnInfo;
}

void SwUndoFtnInfo::Undo( SwUndoIter &rIter )
{
    SwDoc &rDoc = rIter.GetDoc();
    SwFtnInfo *pInf = new SwFtnInfo( rDoc.GetFtnInfo() );
    rDoc.SetFtnInfo( *pFtnInfo );
    delete pFtnInfo;
    pFtnInfo = pInf;
}

void SwUndoFtnInfo::Redo( SwUndoIter &rIter )
{
    SwDoc &rDoc = rIter.GetDoc();
    SwFtnInfo *pInf = new SwFtnInfo( rDoc.GetFtnInfo() );
    rDoc.SetFtnInfo( *pFtnInfo );
    delete pFtnInfo;
    pFtnInfo = pInf;
}

/*  */

SwUndoEndNoteInfo::SwUndoEndNoteInfo( const SwEndNoteInfo &rInfo ) :
    SwUndo( UNDO_FTNINFO ),
    pEndNoteInfo( new SwEndNoteInfo( rInfo ) )
{
}

SwUndoEndNoteInfo::~SwUndoEndNoteInfo()
{
    delete pEndNoteInfo;
}

void SwUndoEndNoteInfo::Undo( SwUndoIter &rIter )
{
    SwDoc &rDoc = rIter.GetDoc();
    SwEndNoteInfo *pInf = new SwEndNoteInfo( rDoc.GetEndNoteInfo() );
    rDoc.SetEndNoteInfo( *pEndNoteInfo );
    delete pEndNoteInfo;
    pEndNoteInfo = pInf;
}

void SwUndoEndNoteInfo::Redo( SwUndoIter &rIter )
{
    SwDoc &rDoc = rIter.GetDoc();
    SwEndNoteInfo *pInf = new SwEndNoteInfo( rDoc.GetEndNoteInfo() );
    rDoc.SetEndNoteInfo( *pEndNoteInfo );
    delete pEndNoteInfo;
    pEndNoteInfo = pInf;
}

/*  */

SwUndoDontExpandFmt::SwUndoDontExpandFmt( const SwPosition& rPos )
    : SwUndo( UNDO_DONTEXPAND ),
    nNode( rPos.nNode.GetIndex() ), nCntnt( rPos.nContent.GetIndex() )
{
}

void SwUndoDontExpandFmt::Undo( SwUndoIter& rIter )
{
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();

    SwPosition& rPos = *pPam->GetPoint();
    rPos.nNode = nNode;
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), nCntnt );
    pDoc->DontExpandFmt( rPos, FALSE );
}


void SwUndoDontExpandFmt::Redo( SwUndoIter& rIter )
{
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();

    SwPosition& rPos = *pPam->GetPoint();
    rPos.nNode = nNode;
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), nCntnt );
    pDoc->DontExpandFmt( rPos );
}

void SwUndoDontExpandFmt::Repeat( SwUndoIter& rIter )
{
    SwPaM* pPam = rIter.pAktPam;
    SwDoc* pDoc = pPam->GetDoc();
    pDoc->DontExpandFmt( *pPam->GetPoint() );
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unattr.cxx,v 1.1.1.1 2000-09-19 00:08:27 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.80  2000/09/18 16:04:28  willem.vandorp
      OpenOffice header added.

      Revision 1.79  2000/05/09 10:03:54  jp
      Changes for Unicode

      Revision 1.78  1999/11/15 14:52:35  jp
      SwUndoAttr:Undo: if undo of textattr without end, then TmpRolbck from first to last

      Revision 1.77  1999/11/08 16:18:43  jp
      UpdateAllFtn calls the Update at RootFrm

      Revision 1.76  1999/03/30 07:15:24  JP
      Task #63092#: ResetAttr - auch einzelne Attribute zuruecksetzen


      Rev 1.75   30 Mar 1999 09:15:24   JP
   Task #63092#: ResetAttr - auch einzelne Attribute zuruecksetzen

      Rev 1.74   29 Mar 1999 18:45:56   JP
   Task #63092#: ResetAttr - auch einzelne Attribute zuruecksetzen

      Rev 1.73   25 Feb 1999 18:11:38   JP
   Bug #62421#: ResetAttr - ohne Selektion zus. die Zeichenattribute stoppen

      Rev 1.72   27 Jan 1999 18:52:30   JP
   Task #61014#: FindSdrObject/FindContactObject als Methoden vom SwFrmFmt

      Rev 1.71   15 Jan 1999 15:48:54   JP
   Bug #60203#: MoveLeftMargin - optional um Betrag verschieben

      Rev 1.70   02 Dec 1998 10:36:38   JP
   Task #59951#: UnAttr fuer Sectionformate erweitert

      Rev 1.69   12 May 1998 15:48:16   JP
   rund um Flys/DrawObjs im Doc umgestellt/optimiert

      Rev 1.68   02 Apr 1998 15:12:48   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.67   24 Mar 1998 20:54:14   JP
   neu: Redlining fuer harte Attributierung

      Rev 1.66   16 Jan 1998 11:11:22   JP
   pAttrHistory am Doc entfernt, UndoAttr mit Redline

      Rev 1.65   15 Dec 1997 19:36:18   JP
   ResetAttr: die richtige Reset-Methode am Doc rufen

      Rev 1.64   12 Dec 1997 14:47:36   MA
   undo setftn korrigiert

      Rev 1.63   10 Dec 1997 16:34:42   JP
   neu: Undo fuers aendern der Fussnoten

      Rev 1.62   08 Dec 1997 12:35:50   MA
   vorb. Endnoten

      Rev 1.61   20 Nov 1997 18:35:30   MA
   includes

      Rev 1.60   06 Nov 1997 15:31:20   JP
   benutze neue Methode GetFlyFmt() vom SwNode

      Rev 1.59   03 Nov 1997 13:06:16   MA
   precomp entfernt

      Rev 1.58   09 Oct 1997 15:45:56   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.57   11 Sep 1997 12:35:02   JP
   Bug #41975#: InsertLabel wurde undofaehig

      Rev 1.56   10 Sep 1997 14:31:58   JP
   Bug #43672#: FmtAttr::Repeat - ContentAttribut NIE setzen

      Rev 1.55   03 Sep 1997 10:28:44   JP
   zusaetzliches include von docary

      Rev 1.54   18 Aug 1997 10:34:52   OS
   includes

      Rev 1.53   15 Aug 1997 12:37:38   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.52   11 Aug 1997 16:27:06   OS
   Header-Umstellung

      Rev 1.51   11 Jun 1997 10:42:44   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.50   17 Apr 1997 16:23:08   AMA
   New: Rahmengebundene Rahmen

      Rev 1.49   15 Apr 1997 14:51:38   AMA
   New: Rahmengebundene Rahmen und auto.positionierte Rahmen

      Rev 1.48   15 Jan 1997 19:54:04   JP
   SwUndoRstAttr: loeschen von SwTOXMarks impl.

      Rev 1.47   14 Jan 1997 14:46:56   JP
   neu: SwUndoMoveLeftMargin - linken Rand stufen

      Rev 1.46   07 Jan 1997 12:44:32   JP
   UndoFmtAttr: TabellenFormate beachten

      Rev 1.45   31 Oct 1996 16:19:06   MA
   opt: Which nicht mehr virtuell, kann ISA ersetzen

      Rev 1.44   29 Oct 1996 14:52:58   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.43   25 Oct 1996 13:43:50   JP
   InsertAttr/SetAttr: Mode uebergeben

      Rev 1.42   23 Sep 1996 20:06:30   JP
   SetTmpEnd: DocPtr entfernt

      Rev 1.41   29 Aug 1996 10:18:50   OS
   includes

      Rev 1.40   07 Aug 1996 10:04:26   NF
   includes

      Rev 1.39   17 May 1996 15:04:22   AMA
   Fix: Undo von zeichengebundenen Zeichenobjekten

      Rev 1.38   24 Nov 1995 17:14:06   OM
   PCH->PRECOMPILED

      Rev 1.37   13 Nov 1995 12:09:00   MA
   chg: static -> lcl_

      Rev 1.36   03 Nov 1995 19:31:32   AMA
   Opt.StartUp: DrawView/Model erst bei Bedarf.

      Rev 1.35   08 Sep 1995 19:11:40   ER
   _SFXUNDO_HXX --> _UNDO_HXX

      Rev 1.34   22 Aug 1995 17:44:48   JP
   Ankerwechsel: beachte die DrawObjecte

      Rev 1.33   22 Jun 1995 19:33:18   JP
   virt. Methode GetUndoRange vernichtet, Objecte rufen jetzt die Basis-Klasse

      Rev 1.32   15 May 1995 17:00:36   JP
   UndoAttrSet: beim Repeat Refmarks ausklammern

      Rev 1.31   03 May 1995 12:22:04   JP
   fehlendes Include zugefuegt

*************************************************************************/

