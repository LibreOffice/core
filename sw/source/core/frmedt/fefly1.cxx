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

#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/soerr.hxx>
#include <editeng/protitem.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdouno.hxx>
#include <svx/fmglob.hxx>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <fmtanchr.hxx>
#include <txtflcnt.hxx>
#include <fmtcntnt.hxx>
#include <fmtornt.hxx>
#include <fmtflcnt.hxx>
#include <fmturl.hxx>
#include <fmtclds.hxx>
#include <fmtfsize.hxx>
#include <docary.hxx>
#include <fesh.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <viewimp.hxx>
#include <viscrs.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <dview.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <ndtxt.hxx>
#include <edimp.hxx>
#include <swtable.hxx>
#include <mvsave.hxx>       // Strukturen zum Sichern beim Move/Delete
#include <ndgrf.hxx>
#include <flyfrms.hxx>
#include <flypos.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <swundo.hxx>
#include <frame.hxx>
#include <notxtfrm.hxx>
#include <HandleAnchorNodeChg.hxx>
#include <frmatr.hxx>
#include <fmtsrnd.hxx> // #i89920#
#include <ndole.hxx>
#include <editeng/opaqitem.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;

//Zum anmelden von Flys in Flys in ...
//definiert in layout/frmtool.cxx
void RegistFlys( SwPageFrm*, const SwLayoutFrm* );

/***********************************************************************
#*  Class       :  SwDoc
#*  Methode     :  UseSpzLayoutFmt
#*  Beschreibung:  Anhand des Request werden zu dem Format entsprechende
#*      Aenderungen an den Spezifischen Layouts vorgenommen.
#***********************************************************************/

sal_Bool lcl_SetNewFlyPos( const SwNode& rNode, SwFmtAnchor& rAnchor,
                        const Point& rPt )
{
    sal_Bool bRet = sal_False;
    const SwStartNode* pStNode = rNode.FindFlyStartNode();
    if( pStNode )
    {
        SwPosition aPos( *pStNode );
        rAnchor.SetAnchor( &aPos );
        bRet = sal_True;
    }
    else
    {
        const SwCntntNode *pCntNd = rNode.GetCntntNode();
        const SwCntntFrm* pCFrm = pCntNd ? pCntNd->GetFrm( &rPt, 0, sal_False ) : 0;
        const SwPageFrm *pPg = pCFrm ? pCFrm->FindPageFrm() : 0;

        rAnchor.SetPageNum( pPg ? pPg->GetPhyPageNum() : 1 );
        rAnchor.SetType( FLY_AT_PAGE );
    }
    return bRet;
}

sal_Bool lcl_FindAnchorPos( SwDoc& rDoc, const Point& rPt, const SwFrm& rFrm,
                        SfxItemSet& rSet )
{
    sal_Bool bRet = sal_True;
    SwFmtAnchor aNewAnch( (SwFmtAnchor&)rSet.Get( RES_ANCHOR ) );
    RndStdIds nNew = aNewAnch.GetAnchorId();
    const SwFrm *pNewAnch;

    //Neuen Anker ermitteln
    Point aTmpPnt( rPt );
    switch( nNew )
    {
    case FLY_AS_CHAR:  // sollte der nicht auch mit hinein?
    case FLY_AT_PARA:
    case FLY_AT_CHAR: // LAYER_IMPL
        {
            //Ausgehend von der linken oberen Ecke des Fly den
            //dichtesten CntntFrm suchen.
            const SwFrm* pFrm = rFrm.IsFlyFrm() ? ((SwFlyFrm&)rFrm).GetAnchorFrm()
                                                : &rFrm;
            pNewAnch = ::FindAnchor( pFrm, aTmpPnt );
            if( pNewAnch->IsProtected() )
            {
                bRet = sal_False;
                break;
            }

            SwPosition aPos( *((SwCntntFrm*)pNewAnch)->GetNode() );
            if ((FLY_AT_CHAR == nNew) || (FLY_AS_CHAR == nNew))
            {
                // es muss ein TextNode gefunden werden, denn nur in diesen
                // ist ein Inhaltsgebundene Frames zu verankern
                SwCrsrMoveState aState( MV_SETONLYTEXT );
                aTmpPnt.X() -= 1;                   //nicht im Fly landen!!
                if( !pNewAnch->GetCrsrOfst( &aPos, aTmpPnt, &aState ) )
                {
                    SwCntntNode* pCNd = ((SwCntntFrm*)pNewAnch)->GetNode();
                    if( pNewAnch->Frm().Bottom() < aTmpPnt.Y() )
                        pCNd->MakeStartIndex( &aPos.nContent );
                    else
                        pCNd->MakeEndIndex( &aPos.nContent );
                }
            }
            aNewAnch.SetAnchor( &aPos );
        }
        break;

    case FLY_AT_FLY: // LAYER_IMPL
        {
            //Ausgehend von der linken oberen Ecke des Fly den
            //dichtesten SwFlyFrm suchen.
            SwCrsrMoveState aState( MV_SETONLYTEXT );
            SwPosition aPos( rDoc.GetNodes() );
            aTmpPnt.X() -= 1;                   //nicht im Fly landen!!
            rDoc.GetRootFrm()->GetCrsrOfst( &aPos, aTmpPnt, &aState );
            pNewAnch = ::FindAnchor(
                aPos.nNode.GetNode().GetCntntNode()->GetFrm( 0, 0, sal_False ),
                aTmpPnt )->FindFlyFrm();

            if( pNewAnch && &rFrm != pNewAnch && !pNewAnch->IsProtected() )
            {
                aPos.nNode = *((SwFlyFrm*)pNewAnch)->GetFmt()->GetCntnt().
                                GetCntntIdx();
                aNewAnch.SetAnchor( &aPos );
                break;
            }
        }

        aNewAnch.SetType( nNew = FLY_AT_PAGE );
        // no break

    case FLY_AT_PAGE:
        pNewAnch = rFrm.FindPageFrm();
        aNewAnch.SetPageNum( pNewAnch->GetPhyPageNum() );
        break;

    default:
        OSL_ENSURE( !&rDoc, "Falsche ID fuer neuen Anker." );
    }

    rSet.Put( aNewAnch );
    return bRet;
}

//
//! also used in unoframe.cxx
//
sal_Bool lcl_ChkAndSetNewAnchor( const SwFlyFrm& rFly, SfxItemSet& rSet )
{
    const SwFrmFmt& rFmt = *rFly.GetFmt();
    const SwFmtAnchor &rOldAnch = rFmt.GetAnchor();
    const RndStdIds nOld = rOldAnch.GetAnchorId();

    RndStdIds nNew = ((SwFmtAnchor&)rSet.Get( RES_ANCHOR )).GetAnchorId();

    if( nOld == nNew )
        return sal_False;

    SwDoc* pDoc = (SwDoc*)rFmt.GetDoc();

#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE( !(nNew == FLY_AT_PAGE &&
        (FLY_AT_PARA==nOld || FLY_AT_CHAR==nOld || FLY_AS_CHAR==nOld ) &&
        pDoc->IsInHeaderFooter( rOldAnch.GetCntntAnchor()->nNode )),
            "Unerlaubter Ankerwechsel in Head/Foot." );
#endif

    return ::lcl_FindAnchorPos( *pDoc, rFly.Frm().Pos(), rFly, rSet );
}

void SwFEShell::SelectFlyFrm( SwFlyFrm& rFrm, sal_Bool bNew )
{
    SET_CURR_SHELL( this );

    //  Wenn es ein neuer Rahmen ist, so soll er selektiert sein.
    //  !!Rahmen immer selektieren, wenn sie nicht selektiert sind.
    //  - Es kann ein neuer 'alter' sein weil der Anker gewechselt wurde.
    //  - 'alte' Rahmen sind vorher immer selektiert denn sonst wird nix
    //    an ihnen veraendert.
    //  Der Rahmen darf nicht per Dokumentposition selektiert werden, weil er
    //  auf jedenfall selektiert sein muss!
    SwViewImp *pImpl = Imp();
    if( GetWin() && (bNew || !pImpl->GetDrawView()->AreObjectsMarked()) )
    {
        OSL_ENSURE( rFrm.IsFlyFrm(), "SelectFlyFrm will einen Fly" );

        //Wenn der Fly bereits selektiert ist gibt es hier ja wohl nichts
        //zu tun.
        if ( FindFlyFrm() == &rFrm )
            return;

        //Damit der Anker ueberhaupt noch gepaintet wird.
        if( rFrm.IsFlyInCntFrm() && rFrm.GetAnchorFrm() )
             rFrm.GetAnchorFrm()->SetCompletePaint();

        if( pImpl->GetDrawView()->AreObjectsMarked() )
            pImpl->GetDrawView()->UnmarkAll();

        pImpl->GetDrawView()->MarkObj( rFrm.GetVirtDrawObj(),
                                      pImpl->GetPageView(), sal_False, sal_False );
        KillPams();
        ClearMark();
        SelFlyGrabCrsr();
    }
}

/*************************************************************************
|*
|*  SwFEShell::FindFlyFrm()
|*
|*  Beschreibung        Liefert den Fly wenn einer Selektiert ist.
|*
*************************************************************************/

SwFlyFrm *SwFEShell::FindFlyFrm() const
{
    if ( Imp()->HasDrawView() )
    {
        // Ein Fly ist genau dann erreichbar, wenn er selektiert ist.
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        if( rMrkList.GetMarkCount() != 1 )
            return 0;

        SdrObject *pO = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        return pO->ISA(SwVirtFlyDrawObj) ? ((SwVirtFlyDrawObj*)pO)->GetFlyFrm() : 0;
    }
    return 0;
}

/*************************************************************************
|*
|*  SwFEShell::IsFlyInFly()
|*
|*  Beschreibung        Liefert sal_True, wenn der aktuelle Fly an einem anderen
|*                      verankert werden koennte (also innerhalb ist)
|*
*************************************************************************/

const SwFrmFmt* SwFEShell::IsFlyInFly()
{
    SET_CURR_SHELL( this );

    if ( !Imp()->HasDrawView() )
        return NULL;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if ( !rMrkList.GetMarkCount() )
    {
        SwCntntFrm *pCntnt = GetCurrFrm( sal_False );
        if( !pCntnt )
            return NULL;
        SwFlyFrm *pFly = pCntnt->FindFlyFrm();
        if ( !pFly )
            return NULL;
        return pFly->GetFmt();
    }
    else if ( rMrkList.GetMarkCount() != 1 ||
         !GetUserCall(rMrkList.GetMark( 0 )->GetMarkedSdrObj()) )
        return NULL;

    SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();

    SwFrmFmt *pFmt = FindFrmFmt( pObj );
    if( pFmt && FLY_AT_FLY == pFmt->GetAnchor().GetAnchorId() )
    {
        const SwFrm* pFly = pObj->ISA(SwVirtFlyDrawObj) ?
            ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->GetAnchorFrm() :
            ((SwDrawContact*)GetUserCall(pObj))->GetAnchorFrm( pObj );
        OSL_ENSURE( pFly, "IsFlyInFly: Where's my anchor?" );
        OSL_ENSURE( pFly->IsFlyFrm(), "IsFlyInFly: Funny anchor!" );
        return ((SwFlyFrm*)pFly)->GetFmt();
    }

    Point aTmpPos = pObj->GetCurrentBoundRect().TopLeft();

    SwFrm *pTxtFrm;
    {
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        SwNodeIndex aSwNodeIndex( GetDoc()->GetNodes() );
        SwPosition aPos( aSwNodeIndex );
        Point aPoint( aTmpPos );
        aPoint.X() -= 1;                    //nicht im Fly landen!!
        GetLayout()->GetCrsrOfst( &aPos, aPoint, &aState );
        // determine text frame by left-top-corner of object
        pTxtFrm = aPos.nNode.GetNode().GetCntntNode()->GetFrm( &aTmpPos, 0, sal_False );
    }
    const SwFrm *pTmp = ::FindAnchor( pTxtFrm, aTmpPos );
    const SwFlyFrm *pFly = pTmp->FindFlyFrm();
    if( pFly )
        return pFly->GetFmt();
    return NULL;
}

/*************************************************************************
|*
|*  SwFEShell::SetFlyPos
|*
*************************************************************************/

void SwFEShell::SetFlyPos( const Point& rAbsPos )
{
    SET_CURR_SHELL( this );

    //Bezugspunkt in Dokumentkoordinaten bestimmen
    SwCntntFrm *pCntnt = GetCurrFrm( sal_False );
    if( !pCntnt )
        return;
    SwFlyFrm *pFly = pCntnt->FindFlyFrm();
    if ( !pFly )
        return;

    //SwSaveHdl aSaveX( Imp() );

    //Bei Absatzgebundenen Flys muss ausgehend von der absoluten
    //Position ein neuer Anker gesetzt werden. Anker und neue RelPos werden
    //vom Fly selbst berechnet und gesetzt.
    if ( pFly->IsFlyAtCntFrm() )
        ((SwFlyAtCntFrm*)pFly)->SetAbsPos( rAbsPos );
    else
    {
            const SwFrm *pAnch = pFly->GetAnchorFrm();
            Point aOrient( pAnch->Frm().Pos() );

        if ( pFly->IsFlyInCntFrm() )
            aOrient.X() = rAbsPos.X();

        //RelPos errechnen.
        aOrient.X() = rAbsPos.X() - aOrient.X();
        aOrient.Y() = rAbsPos.Y() - aOrient.Y();
        pFly->ChgRelPos( aOrient );
    }
    CallChgLnk();       // rufe das AttrChangeNotify auf der UI-Seite.
}

/*************************************************************************
|*
|*  SwFEShell::FindAnchorPos
|*
*************************************************************************/

Point SwFEShell::FindAnchorPos( const Point& rAbsPos, sal_Bool bMoveIt )
{
    Point aRet;

    SET_CURR_SHELL( this );

    if ( !Imp()->HasDrawView() )
        return aRet;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if ( rMrkList.GetMarkCount() != 1 ||
         !GetUserCall(rMrkList.GetMark( 0 )->GetMarkedSdrObj()) )
        return aRet;

    SdrObject* pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
    // --> OD 2004-07-16 #i28701#
    SwAnchoredObject* pAnchoredObj = ::GetUserCall( pObj )->GetAnchoredObj( pObj );
    SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
    RndStdIds nAnchorId = rFmt.GetAnchor().GetAnchorId();

    if ( FLY_AS_CHAR == nAnchorId )
        return aRet;

    sal_Bool bFlyFrame = pObj->ISA(SwVirtFlyDrawObj);

    SwFlyFrm* pFly = 0L;
    const SwFrm* pOldAnch;
    const SwFrm* pFooterOrHeader = NULL;

    if( bFlyFrame )
    {
        //Bezugspunkt in Dokumentkoordinaten bestimmen
        SwCntntFrm *pCntnt = GetCurrFrm( sal_False );
        if( !pCntnt )
            return aRet;
        pFly = pCntnt->FindFlyFrm();
        if ( !pFly )
            return aRet;
        pOldAnch = pFly->GetAnchorFrm();
        if( !pOldAnch )
            return aRet;
        if ( FLY_AT_PAGE != nAnchorId )
        {
            pFooterOrHeader = pCntnt->FindFooterOrHeader();
        }
    }
    // set <pFooterOrHeader> also for drawing
    // objects, but not for control objects.
    // Necessary for moving 'anchor symbol' at the user interface inside header/footer.
    else if ( !::CheckControlLayer( pObj ) )
    {
        SwCntntFrm *pCntnt = GetCurrFrm( sal_False );
        if( !pCntnt )
            return aRet;
        pFooterOrHeader = pCntnt->FindFooterOrHeader();
    }

    //Ausgehend von der linken oberen Ecke des Fly den
    //dichtesten SwFlyFrm suchen.
    SwCntntFrm *pTxtFrm;
    {
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        SwPosition aPos( GetDoc()->GetNodes().GetEndOfExtras() );
        Point aTmpPnt( rAbsPos );
        GetLayout()->GetCrsrOfst( &aPos, aTmpPnt, &aState );
        pTxtFrm = aPos.nNode.GetNode().GetCntntNode()->GetFrm(0,&aPos,sal_False );
    }
    const SwFrm *pNewAnch;
    if( pTxtFrm )
    {
        if ( FLY_AT_PAGE == nAnchorId )
        {
            pNewAnch = pTxtFrm->FindPageFrm();
        }
        else
        {
            pNewAnch = ::FindAnchor( pTxtFrm, rAbsPos );

            if( FLY_AT_FLY == nAnchorId ) // LAYER_IMPL
            {
                pNewAnch = pNewAnch->FindFlyFrm();
            }
        }
    }
    else
        pNewAnch = 0;

    if( pNewAnch && !pNewAnch->IsProtected() )
    {
        const SwFlyFrm* pCheck = bFlyFrame ? pNewAnch->FindFlyFrm() : 0;
        // Falls wir innerhalb eines Rahmens landen, muss sichergestellt werden,
        // dass der Rahmen nicht in seinem eigenen Inhalt landet!
        while( pCheck )
        {
            if( pCheck == pFly )
                break;
            const SwFrm *pTmp = pCheck->GetAnchorFrm();
            pCheck = pTmp ? pTmp->FindFlyFrm() : NULL;
        }
        // Es darf nicht aus einer Kopf-/Fusszeile in einen anderen Bereich
        // gewechselt werden, es darf nicht in eine Kopf-/Fusszeile hinein-
        // gewechselt werden.
        if( !pCheck &&
            pFooterOrHeader == pNewAnch->FindFooterOrHeader() )
        {
            aRet = pNewAnch->GetFrmAnchorPos( ::HasWrap( pObj ) );

            if ( bMoveIt || (nAnchorId == FLY_AT_CHAR) )
            {
                SwFmtAnchor aAnch( rFmt.GetAnchor() );
                switch ( nAnchorId )
                {
                    case FLY_AT_PARA:
                    {
                        SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
                        pPos->nNode = *pTxtFrm->GetNode();
                        pPos->nContent.Assign(0,0);
                        break;
                    }
                    case FLY_AT_PAGE:
                    {
                        aAnch.SetPageNum( ((const SwPageFrm*)pNewAnch)->
                                          GetPhyPageNum() );
                        break;
                    }
                    case FLY_AT_FLY:
                    {
                        SwPosition aPos( *((SwFlyFrm*)pNewAnch)->GetFmt()->
                                                  GetCntnt().GetCntntIdx() );
                        aAnch.SetAnchor( &aPos );
                        break;
                    }
                    case FLY_AT_CHAR:
                    {
                        SwPosition *pPos = (SwPosition*)aAnch.GetCntntAnchor();
                        Point aTmpPnt( rAbsPos );
                        if( pTxtFrm->GetCrsrOfst( pPos, aTmpPnt, NULL ) )
                        {
                            SwRect aTmpRect;
                            pTxtFrm->GetCharRect( aTmpRect, *pPos );
                            aRet = aTmpRect.Pos();
                        }
                        else
                        {
                            pPos->nNode = *pTxtFrm->GetNode();
                            pPos->nContent.Assign(0,0);
                        }
                        break;
                    }
                    default:
                        break;
                }
                if( bMoveIt )
                {
                    StartAllAction();
                    // --> handle change of anchor node:
                    // if count of the anchor frame also change, the fly frames have to be
                    // re-created. Thus, delete all fly frames except the <this> before the
                    // anchor attribute is change and re-create them afterwards.
                    {
                        SwHandleAnchorNodeChg* pHandleAnchorNodeChg( 0L );
                        SwFlyFrmFmt* pFlyFrmFmt( dynamic_cast<SwFlyFrmFmt*>(&rFmt) );
                        if ( pFlyFrmFmt )
                        {
                            pHandleAnchorNodeChg =
                                new SwHandleAnchorNodeChg( *pFlyFrmFmt, aAnch );
                        }
                        rFmt.GetDoc()->SetAttr( aAnch, rFmt );
                        delete pHandleAnchorNodeChg;
                    }
                    // <--
                    // --> OD 2004-06-24 #i28701# - no call of method
                    // <CheckCharRectAndTopOfLine()> for to-character anchored
                    // Writer fly frame needed. This method call can cause a
                    // format of the anchor frame, which is no longer intended.
                    // Instead clear the anchor character rectangle and
                    // the top of line values for all to-character anchored objects.
                    pAnchoredObj->ClearCharRectAndTopOfLine();
                    // <--
                    EndAllAction();
                }
            }

            SwRect aTmpRect( aRet, rAbsPos );
            if( aTmpRect.HasArea() )
                MakeVisible( aTmpRect );
#if OSL_DEBUG_LEVEL > 1
            //TODO: That doesn't seem to be intended
            if( Color(COL_TRANSPARENT) != GetOut()->GetLineColor() )
            {
                OSL_ENSURE( sal_False, "Hey, Joe: Where's my Null Pen?" );
                GetOut()->SetLineColor( Color(COL_TRANSPARENT) );
            }
#endif
        }
    }

    return aRet;
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  NewFlyFrm
#*  Beschreibung:
#***********************************************************************/

const SwFrmFmt *SwFEShell::NewFlyFrm( const SfxItemSet& rSet, sal_Bool bAnchValid,
                           SwFrmFmt *pParent )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    const Point aPt( GetCrsrDocPos() );

    SwSelBoxes aBoxes;
    sal_Bool bMoveCntnt = sal_True;
    if( IsTableMode() )
    {
        GetTblSel( *this, aBoxes );
        if( aBoxes.Count() )
        {
            // die Crsr muessen noch aus dem Loeschbereich entfernt
            // werden. Setze sie immer hinter/auf die Tabelle; ueber die
            // Dokument-Position werden sie dann immer an die alte
            // Position gesetzt.
            ParkCrsr( SwNodeIndex( *aBoxes[0]->GetSttNd() ));

            // --> FME 2005-12-01 #i127787# pCurCrsr will be deleted in ParkCrsr,
            // we better get the current pCurCrsr instead of working with the
            // deleted one:
            pCrsr = GetCrsr();
        }
        else
            bMoveCntnt = sal_False;
    }
    else if( !pCrsr->HasMark() && pCrsr->GetNext() == pCrsr )
        bMoveCntnt = sal_False;

    const SwPosition& rPos = *pCrsr->Start();

    SwFmtAnchor& rAnch = (SwFmtAnchor&)rSet.Get( RES_ANCHOR );
    RndStdIds eRndId = rAnch.GetAnchorId();
    switch( eRndId )
    {
    case FLY_AT_PAGE:
        if( !rAnch.GetPageNum() )       //HotFix: Bug in UpdateByExample
            rAnch.SetPageNum( 1 );
        break;

    case FLY_AT_FLY:
    case FLY_AT_PARA:
    case FLY_AT_CHAR:
    case FLY_AS_CHAR:
        if( !bAnchValid )
        {
            if( FLY_AT_FLY != eRndId )
            {
                rAnch.SetAnchor( &rPos );
            }
            else if( lcl_SetNewFlyPos( rPos.nNode.GetNode(), rAnch, aPt ) )
            {
                eRndId = FLY_AT_PAGE;
            }
        }
        break;

    default:
        OSL_ENSURE( !this, "Was sollte das fuer ein Fly werden?" );
        break;
    }

    SwFlyFrmFmt *pRet;
    if( bMoveCntnt )
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_INSLAYFMT, NULL );
        SwFmtAnchor* pOldAnchor = 0;
        sal_Bool bHOriChgd = sal_False, bVOriChgd = sal_False;
        SwFmtVertOrient aOldV;
        SwFmtHoriOrient aOldH;

        if ( FLY_AT_PAGE != eRndId )
        {
            // erstmal als mit Seitenbindung, Absatz/Zeichenbindung erst wenn
            // alles verschoben ist. Dann ist die Position gueltig!
            // JP 13.05.98: ggfs. auch noch die Hori/Vert-Orientierung
            //              umsetzen, damit diese beim Umanker NICHT
            //              korrigiert wird
            pOldAnchor = new SwFmtAnchor( rAnch );
            const_cast<SfxItemSet&>(rSet).Put( SwFmtAnchor( FLY_AT_PAGE, 1 ) );

            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == rSet.GetItemState( RES_HORI_ORIENT, sal_False, &pItem )
                && text::HoriOrientation::NONE == ((SwFmtHoriOrient*)pItem)->GetHoriOrient() )
            {
                bHOriChgd = sal_True;
                aOldH = *((SwFmtHoriOrient*)pItem);
                ((SfxItemSet&)rSet).Put( SwFmtHoriOrient( 0, text::HoriOrientation::LEFT ) );
            }
            if( SFX_ITEM_SET == rSet.GetItemState( RES_VERT_ORIENT, sal_False, &pItem )
                && text::VertOrientation::NONE == ((SwFmtVertOrient*)pItem)->GetVertOrient() )
            {
                bVOriChgd = sal_True;
                aOldV = *((SwFmtVertOrient*)pItem);
                ((SfxItemSet&)rSet).Put( SwFmtVertOrient( 0, text::VertOrientation::TOP ) );
            }
        }

        pRet = GetDoc()->MakeFlyAndMove( *pCrsr, rSet, &aBoxes, pParent );

        KillPams();

        if( pOldAnchor )
        {
            if( pRet )
            {
                // neue Position bestimmen
                //JP 24.03.97: immer ueber die Seitenbindung gehen - der
                //           chaos::Anchor darf nie im verschobenen Bereich
                //              liegen
                pRet->DelFrms();

                const SwFrm* pAnch = ::FindAnchor( GetLayout(), aPt, sal_False );
                SwPosition aPos( *((SwCntntFrm*)pAnch)->GetNode() );
                if ( FLY_AS_CHAR == eRndId )
                {
                    aPos.nContent.Assign( ((SwCntntFrm*)pAnch)->GetNode(), 0 );
                }
                pOldAnchor->SetAnchor( &aPos );

                // das verschieben von TabelleSelektion ist noch nicht
                // Undofaehig - also darf das UmAnkern auch nicht
                // aufgezeichnet werden.
                bool const bDoesUndo =
                    GetDoc()->GetIDocumentUndoRedo().DoesUndo();
                SwUndoId nLastUndoId(UNDO_EMPTY);
                if (bDoesUndo &&
                    GetDoc()->GetIDocumentUndoRedo().GetLastUndoInfo(0,
                        & nLastUndoId))
                {
                    if (UNDO_INSLAYFMT == nLastUndoId)
                    {
                        GetDoc()->GetIDocumentUndoRedo().DoUndo(false);
                    }
                }

                ((SfxItemSet&)rSet).Put( *pOldAnchor );

                if( bHOriChgd )
                    ((SfxItemSet&)rSet).Put( aOldH );
                if( bVOriChgd )
                    ((SfxItemSet&)rSet).Put( aOldV );

                GetDoc()->SetFlyFrmAttr( *pRet, (SfxItemSet&)rSet );
                GetDoc()->GetIDocumentUndoRedo().DoUndo(bDoesUndo);
            }
            delete pOldAnchor;
        }
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_INSLAYFMT, NULL );
    }
    else
        /* If called from a shell try to propagate an
            existing adjust item from rPos to the content node of the
            new frame. */
        pRet = GetDoc()->MakeFlySection( eRndId, &rPos, &rSet, pParent, sal_True );

    if( pRet )
    {
        SwFlyFrm* pFrm = pRet->GetFrm( &aPt );
        if( pFrm )
            SelectFlyFrm( *pFrm, sal_True );
        else
        {
            GetLayout()->SetAssertFlyPages();
            pRet = 0;
        }
    }
    EndAllActionAndCall();

    return pRet;
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  Insert
#***********************************************************************/

void SwFEShell::Insert( const String& rGrfName, const String& rFltName,
                        const Graphic* pGraphic,
                        const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet,
                        SwFrmFmt* pFrmFmt )
{
    SwFlyFrmFmt* pFmt = 0;
    SET_CURR_SHELL( this );
    StartAllAction();
    SwShellCrsr *pStartCursor = dynamic_cast<SwShellCrsr*>(this->GetSwCrsr());
    SwShellCrsr *pCursor = pStartCursor;
    do {

        // Anker noch nicht oder unvollstaendig gesetzt ?
        if( pFlyAttrSet )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pFlyAttrSet->GetItemState( RES_ANCHOR, sal_False,
                    &pItem ) )
            {
                SwFmtAnchor* pAnchor = (SwFmtAnchor*)pItem;
                switch( pAnchor->GetAnchorId())
                {
                case FLY_AT_PARA:
                case FLY_AT_CHAR: // LAYER_IMPL
                case FLY_AS_CHAR:
                    if( !pAnchor->GetCntntAnchor() )
                    {
                        pAnchor->SetAnchor( pCursor->GetPoint() );
                    }
                    break;
                case FLY_AT_FLY:
                    if( !pAnchor->GetCntntAnchor() )
                    {
                        lcl_SetNewFlyPos( *pCursor->GetNode(),
                                *pAnchor, GetCrsrDocPos() );
                    }
                    break;
                case FLY_AT_PAGE:
                    if( !pAnchor->GetPageNum() )
                    {
                        pAnchor->SetPageNum( pCursor->GetPageNum(
                                sal_True, &pCursor->GetPtPos() ) );
                    }
                    break;
                default :
                    break;
                }
            }
        }
        pFmt = GetDoc()->Insert(*pCursor, rGrfName,
                                rFltName, pGraphic,
                                pFlyAttrSet,
                                pGrfAttrSet, pFrmFmt );
        OSL_ENSURE( pFmt, "Doc->Insert(notxt) failed." );

    } while( (pCursor = dynamic_cast<SwShellCrsr*>(pCursor->GetNext()))
             != pStartCursor );

    EndAllAction();

    if( pFmt )
    {
        const Point aPt( GetCrsrDocPos() );
        SwFlyFrm* pFrm = pFmt->GetFrm( &aPt );

        if( pFrm )
            SelectFlyFrm( *pFrm, sal_True );
        else
            GetLayout()->SetAssertFlyPages();
    }
}

SwFlyFrmFmt* SwFEShell::InsertObject( const svt::EmbeddedObjectRef&  xObj,
                        const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet,
                        SwFrmFmt* pFrmFmt )
{
    SwFlyFrmFmt* pFmt = 0;
    SET_CURR_SHELL( this );
    StartAllAction();
        FOREACHPAM_START( this )
            pFmt = GetDoc()->Insert(*PCURCRSR, xObj,
                                    pFlyAttrSet, pGrfAttrSet, pFrmFmt );
            OSL_ENSURE( pFmt, "Doc->Insert(notxt) failed." );

        FOREACHPAM_END()
    EndAllAction();

    if( pFmt )
    {
        const Point aPt( GetCrsrDocPos() );
        SwFlyFrm* pFrm = pFmt->GetFrm( &aPt );

        if( pFrm )
            SelectFlyFrm( *pFrm, sal_True );
        else
            GetLayout()->SetAssertFlyPages();
    }

    return pFmt;
}


void SwFEShell::InsertDrawObj( SdrObject& rDrawObj,
                               const Point& rInsertPosition )
{
    SET_CURR_SHELL( this );

    SfxItemSet rFlyAttrSet( GetDoc()->GetAttrPool(), aFrmFmtSetRange );
    rFlyAttrSet.Put( SwFmtAnchor( FLY_AT_PARA ));
    // --> OD 2009-12-29 #i89920#
    rFlyAttrSet.Put( SwFmtSurround( SURROUND_THROUGHT ) );
    rDrawObj.SetLayer( getIDocumentDrawModelAccess()->GetHeavenId() );
    // <--

    // find anchor position
    SwPaM aPam( pDoc->GetNodes() );
    {
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        Point aTmpPt( rInsertPosition );
        getIDocumentLayoutAccess()->GetRootFrm()->GetCrsrOfst( aPam.GetPoint(), aTmpPt, &aState );
        const SwFrm* pFrm = aPam.GetCntntNode()->GetFrm( 0, 0, sal_False );
        const Point aRelPos( rInsertPosition.X() - pFrm->Frm().Left(),
                             rInsertPosition.Y() - pFrm->Frm().Top() );
        rDrawObj.SetRelativePos( aRelPos );
        ::lcl_FindAnchorPos( *GetDoc(), rInsertPosition, *pFrm, rFlyAttrSet );
    }
    // insert drawing object into the document creating a new <SwDrawFrmFmt> instance
    SwDrawFrmFmt* pFmt = GetDoc()->Insert( aPam, rDrawObj, &rFlyAttrSet, 0 );

    // move object to visible layer
    SwContact* pContact = static_cast<SwContact*>(rDrawObj.GetUserCall());
    if ( pContact )
    {
        pContact->MoveObjToVisibleLayer( &rDrawObj );
    }

    if ( pFmt )
    {
        // select drawing object
        Imp()->GetDrawView()->MarkObj( &rDrawObj, Imp()->GetPageView(),
                                       sal_False, sal_False );
    }
    else
    {
        GetLayout()->SetAssertFlyPages();
    }
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  GetPageObjs
#***********************************************************************/

void SwFEShell::GetPageObjs( SvPtrarr& rFillArr )
{
    if( rFillArr.Count() )
        rFillArr.Remove( 0, rFillArr.Count() );

    const SwFrmFmt* pFmt;
    for( sal_uInt16 n = 0; n < pDoc->GetSpzFrmFmts()->Count(); ++n )
    {
        pFmt = (const SwFrmFmt*)(*pDoc->GetSpzFrmFmts())[n];
        if (FLY_AT_PAGE == pFmt->GetAnchor().GetAnchorId())
        {
            rFillArr.Insert( (VoidPtr)pFmt, rFillArr.Count() );
        }
    }
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  SetPageFlysNewPage
#***********************************************************************/

void SwFEShell::SetPageObjsNewPage( SvPtrarr& rFillArr, int nOffset )
{
    if( !rFillArr.Count() || !nOffset )
        return;

    StartAllAction();
    StartUndo();

    SwFrmFmt* pFmt;
    long nNewPage;
    SwRootFrm* pTmpRootFrm = getIDocumentLayoutAccess()->GetRootFrm();
    sal_uInt16 nMaxPage = pTmpRootFrm->GetPageNum();
    sal_Bool bTmpAssert = sal_False;
    for( sal_uInt16 n = 0; n < rFillArr.Count(); ++n )
    {
        pFmt = (SwFrmFmt*)rFillArr[n];
        if( USHRT_MAX != pDoc->GetSpzFrmFmts()->GetPos( pFmt ))
        {
            // FlyFmt ist noch gueltig, also behandeln
            SwFmtAnchor aNewAnchor( pFmt->GetAnchor() );
            if ((FLY_AT_PAGE != aNewAnchor.GetAnchorId()) ||
                0 >= ( nNewPage = aNewAnchor.GetPageNum() + nOffset ) )
                // chaos::Anchor wurde veraendert oder ungueltige SeitenNummer,
                // also nicht veraendern !!
                continue;

            if( sal_uInt16(nNewPage) > nMaxPage )
            {
                if ( RES_DRAWFRMFMT == pFmt->Which() )
                {
                    SwContact *pCon = pFmt->FindContactObj();
                    if( pCon )
                        ((SwDrawContact*)pCon)->DisconnectFromLayout();
                }
                else
                    pFmt->DelFrms();
                bTmpAssert = sal_True;
            }
            aNewAnchor.SetPageNum( sal_uInt16(nNewPage) );
            pDoc->SetAttr( aNewAnchor, *pFmt );
        }
    }

    if( bTmpAssert )
        pTmpRootFrm->SetAssertFlyPages();

    EndUndo();
    EndAllAction();
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  GetFlyFrmAttr
#*  Beschreibung:  Alle Attribute in dem 'Koerbchen' werden mit den
#*                 Attributen des aktuellen FlyFrms gefuellt.
#*                 Sind Attribute nicht zu fuellen weil fehl am Platz oder
#*                 uneindeutig (Mehrfachtselektionen) so werden sie entfernt.
#***********************************************************************/

sal_Bool SwFEShell::GetFlyFrmAttr( SfxItemSet &rSet ) const
{
    SwFlyFrm *pFly = FindFlyFrm();
    if ( !pFly )
    {
        SwFrm* pCurrFrm( GetCurrFrm() );
        if ( !pCurrFrm )
        {
            OSL_ENSURE( false,
                    "<SwFEShell::GetFlyFrmAttr(..)> - missing current frame. This is a serious defect, please inform OD." );
            return sal_False;
        }
        pFly = GetCurrFrm()->FindFlyFrm();
        if ( !pFly )
        {
            OSL_ENSURE( !this, "GetFlyFrmAttr, no Fly selected." );
            return sal_False;
        }
    }

    SET_CURR_SHELL( (ViewShell*)this );

    if( !rSet.Set( pFly->GetFmt()->GetAttrSet(), sal_True ) )
        return sal_False;

    //Und die Attribute durchschaufeln. Unerlaubte Attribute entfernen, dann
    //alle restlichen Attribute besorgen und eintragen.
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( RES_ANCHOR, sal_False, &pItem ) )
    {
        SwFmtAnchor* pAnchor = (SwFmtAnchor*)pItem;
        RndStdIds eType = pAnchor->GetAnchorId();

        if ( FLY_AT_PAGE != eType )
        {
            // OD 12.11.2003 #i22341# - content anchor of anchor item is needed.
            // Thus, don't overwrite anchor item by default contructed anchor item.
            if ( FLY_AS_CHAR == eType )
            {
                rSet.ClearItem( RES_OPAQUE );
                rSet.ClearItem( RES_SURROUND );
            }
        }
    }
    rSet.SetParent( pFly->GetFmt()->GetAttrSet().GetParent() );
    // die Attribute MUESSEN entfern werden!
    rSet.ClearItem( RES_FILL_ORDER );
    rSet.ClearItem( RES_CNTNT );
    //MA: Ersteinmal entfernen (Template by example usw.)
    rSet.ClearItem( RES_CHAIN );
    return sal_True;
}
/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  SetFlyFrmAttr
#*  Beschreibung:  Die Attribute des aktuellen Flys aendern sich.
#***********************************************************************/

sal_Bool SwFEShell::SetFlyFrmAttr( SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );
    sal_Bool bRet = sal_False;

    if( rSet.Count() )
    {
        SwFlyFrm *pFly = FindFlyFrm();
        if( !pFly )
        {
            OSL_ENSURE( GetCurrFrm(), "Crsr in parking zone" );
            pFly = GetCurrFrm()->FindFlyFrm();
            OSL_ENSURE( pFly, "SetFlyFrmAttr, no Fly selected." );
        }
        if( pFly )
        {
            StartAllAction();
            const Point aPt( pFly->Frm().Pos() );

            if( SFX_ITEM_SET == rSet.GetItemState( RES_ANCHOR, sal_False ))
                ::lcl_ChkAndSetNewAnchor( *pFly, rSet );
            SwFlyFrmFmt* pFlyFmt = (SwFlyFrmFmt*)pFly->GetFmt();

            if( GetDoc()->SetFlyFrmAttr( *pFlyFmt, rSet ))
            {
                bRet = sal_True;
                SwFlyFrm* pFrm = pFlyFmt->GetFrm( &aPt );
                if( pFrm )
                    SelectFlyFrm( *pFrm, sal_True );
                else
                    GetLayout()->SetAssertFlyPages();
            }

            EndAllActionAndCall();
        }
    }
    return bRet;
}

sal_Bool SwFEShell::SetDrawingAttr( SfxItemSet& rSet )
{
    sal_Bool bRet = sal_False;
    SET_CURR_SHELL( this );
    if ( !rSet.Count() ||
            !Imp()->HasDrawView() )
        return bRet;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if ( rMrkList.GetMarkCount() != 1 )
        return bRet;

    StartUndo();
    SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
    SwFrmFmt *pFmt = FindFrmFmt( pObj );
    StartAllAction();
    if( SFX_ITEM_SET == rSet.GetItemState( RES_ANCHOR, sal_False ))
    {
        RndStdIds nNew = ((SwFmtAnchor&)rSet.Get( RES_ANCHOR )).GetAnchorId();
        if ( nNew != pFmt->GetAnchor().GetAnchorId() )
        {
            ChgAnchor( nNew );
            // --> OD 2004-06-17 #i26791# - clear anchor attribute in item set,
            // because method <ChgAnchor(..)> takes care of it.
            rSet.ClearItem( RES_ANCHOR );
        }
    }

    if( GetDoc()->SetFlyFrmAttr( *pFmt, rSet ))
    {
        bRet = sal_True;
        Point aTmp;
        SelectObj( aTmp, 0, pObj );
    }
    EndAllActionAndCall();
    EndUndo();
    return bRet;
}


/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  ResetFlyFrmAttr
#*  Beschreibung:  Das gewuenschte Attribut oder die im Set befindlichen
#*                  werden zurueckgesetzt.
#***********************************************************************/

sal_Bool SwFEShell::ResetFlyFrmAttr( sal_uInt16 nWhich, const SfxItemSet* pSet )
{
    sal_Bool bRet = sal_False;

    if( RES_ANCHOR != nWhich && RES_CHAIN != nWhich && RES_CNTNT != nWhich )
    {
        SET_CURR_SHELL( this );

        SwFlyFrm *pFly = FindFlyFrm();
        if( !pFly )
        {
            OSL_ENSURE( GetCurrFrm(), "Crsr in parking zone" );
            pFly = GetCurrFrm()->FindFlyFrm();
            OSL_ENSURE( pFly, "SetFlyFrmAttr, no Fly selected." );
        }

        if( pFly )
        {
            StartAllAction();

            if( pSet )
            {
                SfxItemIter aIter( *pSet );
                const SfxPoolItem* pItem = aIter.FirstItem();
                while( pItem )
                {
                    if( !IsInvalidItem( pItem ) &&
                        RES_ANCHOR != ( nWhich = pItem->Which() ) &&
                        RES_CHAIN != nWhich && RES_CNTNT != nWhich )
                        pFly->GetFmt()->ResetFmtAttr( nWhich );
                    pItem = aIter.NextItem();
                }
            }
            else
                pFly->GetFmt()->ResetFmtAttr( nWhich );

            bRet = sal_True;
            EndAllActionAndCall();
            GetDoc()->SetModified();
        }
    }
    return bRet;
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  GetCurFrmFmt
#*  Beschreibung:  liefert wenn Rahmen, dann Rahmenvorlage, sonst 0
#***********************************************************************/

SwFrmFmt* SwFEShell::GetCurFrmFmt() const
{
    SwFrmFmt* pRet = 0;
    SwLayoutFrm *pFly = FindFlyFrm();
    if( pFly && ( pRet = (SwFrmFmt*)pFly->GetFmt()->DerivedFrom() ) ==
                                            GetDoc()->GetDfltFrmFmt() )
        pRet = 0;
    return pRet;
}

/******************************************************************************
 *  Methode     :   void SwFEShell::SetFrmFmt(SwFrmFmt *pNewFmt)
 *  Beschreibung:
 ******************************************************************************/

void SwFEShell::SetFrmFmt( SwFrmFmt *pNewFmt, sal_Bool bKeepOrient, Point* pDocPos )
{
    SwFlyFrm *pFly = 0;
    if(pDocPos)
    {
        const SwFrmFmt* pFmt = GetFmtFromObj( *pDocPos );

        if(PTR_CAST(SwFlyFrmFmt, pFmt))
            pFly = ((SwFlyFrmFmt*)pFmt)->GetFrm();
    }
    else
        pFly = FindFlyFrm();
    OSL_ENSURE( pFly, "SetFrmFmt: kein Frame" );
    if( pFly )
    {
        StartAllAction();
        SET_CURR_SHELL( this );

        SwFlyFrmFmt* pFlyFmt = (SwFlyFrmFmt*)pFly->GetFmt();
        const Point aPt( pFly->Frm().Pos() );

        SfxItemSet* pSet = 0;
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pNewFmt->GetItemState( RES_ANCHOR, sal_False, &pItem ))
        {
            pSet = new SfxItemSet( GetDoc()->GetAttrPool(), aFrmFmtSetRange );
            pSet->Put( *pItem );
            if( !::lcl_ChkAndSetNewAnchor( *pFly, *pSet ))
                delete pSet, pSet = 0;
        }

        if( GetDoc()->SetFrmFmtToFly( *pFlyFmt, *pNewFmt, pSet, bKeepOrient ))
        {
            SwFlyFrm* pFrm = pFlyFmt->GetFrm( &aPt );
            if( pFrm )
                SelectFlyFrm( *pFrm, sal_True );
            else
                GetLayout()->SetAssertFlyPages();
        }
        if( pSet )
            delete pSet;

        EndAllActionAndCall();
    }
}

/*************************************************************************
|*
|*  SwFEShell::GetFlyFrmFmt()
|*
*************************************************************************/

const SwFrmFmt* SwFEShell::GetFlyFrmFmt() const
{
    const SwFlyFrm* pFly = FindFlyFrm();
    if ( !pFly )
    {
        SwFrm* pCurrFrm = GetCurrFrm();
        pFly = pCurrFrm ? pCurrFrm->FindFlyFrm() : 0;
    }
    if( pFly )
        return pFly->GetFmt();
    return 0;
}

SwFrmFmt* SwFEShell::GetFlyFrmFmt()
{
    SwFlyFrm* pFly = FindFlyFrm();
    if ( !pFly )
    {
        SwFrm* pCurrFrm = GetCurrFrm();
        pFly = pCurrFrm ? pCurrFrm->FindFlyFrm() : 0;
    }
    if( pFly )
        return pFly->GetFmt();
    return 0;
}

/*************************************************************************
|*
|*  SwFEShell::GetFlyRect()
|*
*************************************************************************/

SwRect SwFEShell::GetFlyRect() const
{
    SwCntntFrm *pCntnt = GetCurrFrm( sal_False );
    SwFlyFrm *pFly = pCntnt ? pCntnt->FindFlyFrm() : 0;
    if ( !pFly )
    {
        SwRect aRect;
        return aRect;
    }
    else
        return pFly->Frm();
}

/*************************************************************************
|*
|*  SwFEShell::GetObjRect()
|*
*************************************************************************/

SwRect SwFEShell::GetObjRect() const
{
    if( Imp()->HasDrawView() )
        return Imp()->GetDrawView()->GetAllMarkedRect();
    else
    {
        SwRect aRect;
        return aRect;
    }
}

void SwFEShell::SetObjRect( const SwRect& rRect )
{
    if ( Imp()->HasDrawView() )
    {
        Imp()->GetDrawView()->SetAllMarkedRect( rRect.SVRect() );
        CallChgLnk();   // rufe das AttrChangeNotify auf der UI-Seite.
    }
}

/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  RequestObjectResize()
#***********************************************************************/

Size SwFEShell::RequestObjectResize( const SwRect &rRect, const uno::Reference < embed::XEmbeddedObject >& xObj )
{
    Size aResult;

    SwFlyFrm *pFly = FindFlyFrm( xObj );
    if ( !pFly )
    {
        aResult = rRect.SSize();
        return aResult;
    }

    aResult = pFly->Prt().SSize();

    sal_Bool bPosProt = pFly->GetFmt()->GetProtect().IsPosProtected();
    sal_Bool bSizeProt = pFly->GetFmt()->GetProtect().IsSizeProtected();

    StartAllAction();

    //MA wir lassen den Fly nicht Clippen, damit die Ole-Server mit
    //beliebigen Wuenschen kommen koennen. Die Formatierung uebernimmt das
    //Clippen. Die richtige Darstellung wird per Scalierung erledigt.
    //Die Scalierung wird von SwNoTxtFrm::Format durch einen Aufruf von
    //SwWrtShell::CalcAndSetScale() erledigt.
    if ( rRect.SSize() != pFly->Prt().SSize() && !bSizeProt )
    {
         Size aSz( rRect.SSize() );

        //JP 28.02.2001: Task 74707 - ask for fly in fly with automatic size
        //
        const SwFrm* pAnchor;
        const SwTxtNode* pTNd;
        const SwpHints* pHts;
        const SwFmtFrmSize& rFrmSz = pFly->GetFmt()->GetFrmSize();
        if( bCheckForOLEInCaption &&
            0 != rFrmSz.GetWidthPercent() &&
            0 != (pAnchor = pFly->GetAnchorFrm()) &&
            pAnchor->IsTxtFrm() &&
            !pAnchor->GetNext() && !pAnchor->GetPrev() &&
            pAnchor->GetUpper()->IsFlyFrm() &&
            0 != ( pTNd = ((SwTxtFrm*)pAnchor)->GetNode()->GetTxtNode()) &&
            0 != ( pHts = pTNd->GetpSwpHints() ))
        {
            // search for a sequence field:
            const SfxPoolItem* pItem;
            for( sal_uInt16 n = 0, nEnd = pHts->Count(); n < nEnd; ++n )
                if( RES_TXTATR_FIELD == ( pItem =
                            &(*pHts)[ n ]->GetAttr())->Which() &&
                    TYP_SEQFLD == ((SwFmtFld*)pItem)->GetFld()->GetTypeId() )
                {
                    // sequence field found
                    SwFlyFrm* pChgFly = (SwFlyFrm*)pAnchor->GetUpper();
                    // calculate the changed size:
                    // width must change, height can change
                    Size aNewSz( aSz.Width() + pChgFly->Frm().Width() -
                                   pFly->Prt().Width(), aSz.Height() );

                    SwFrmFmt *pFmt = pChgFly->GetFmt();
                    SwFmtFrmSize aFrmSz( pFmt->GetFrmSize() );
                    aFrmSz.SetWidth( aNewSz.Width() );
                    if( ATT_MIN_SIZE != aFrmSz.GetHeightSizeType() )
                    {
                        aNewSz.Height() += pChgFly->Frm().Height() -
                                               pFly->Prt().Height();
                        if( Abs( aNewSz.Height() - pChgFly->Frm().Height()) > 1 )
                            aFrmSz.SetHeight( aNewSz.Height() );
                    }
                    // uebers Doc fuers Undo!
                    pFmt->GetDoc()->SetAttr( aFrmSz, *pFmt );
                    break;
                }
        }

        // set the new Size at the fly themself
        if ( pFly->Prt().Height() > 0 && pFly->Prt().Width() > 0 )
        {
            aSz.Width() += pFly->Frm().Width() - pFly->Prt().Width();
            aSz.Height()+= pFly->Frm().Height()- pFly->Prt().Height();
        }
        aResult = pFly->ChgSize( aSz );

        //Wenn sich das Objekt aendert ist die Kontur hoechstwahrscheinlich daneben.
        OSL_ENSURE( pFly->Lower()->IsNoTxtFrm(), "Request ohne NoTxt" );
        SwNoTxtNode *pNd = ((SwCntntFrm*)pFly->Lower())->GetNode()->GetNoTxtNode();
        OSL_ENSURE( pNd, "Request ohne Node" );
        pNd->SetContour( 0 );
        ClrContourCache();
    }

    //Wenn nur die Size angepasst werden soll, so wird eine Pos mit
    //ausgezeichneten Werten transportiert.
    Point aPt( pFly->Prt().Pos() );
    aPt += pFly->Frm().Pos();
    if ( rRect.Top() != LONG_MIN && rRect.Pos() != aPt && !bPosProt )
    {
        aPt = rRect.Pos();
        aPt.X() -= pFly->Prt().Left();
        aPt.Y() -= pFly->Prt().Top();
        //Bei Absatzgebundenen Flys muss ausgehend von der neuen Position ein
        //neuer Anker gesetzt werden. Anker und neue RelPos werden vom Fly
        //selbst berechnet und gesetzt.
        if( pFly->IsFlyAtCntFrm() )
            ((SwFlyAtCntFrm*)pFly)->SetAbsPos( aPt );
        else
        {
            const SwFrmFmt *pFmt = pFly->GetFmt();
            const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
            const SwFmtHoriOrient &rHori = pFmt->GetHoriOrient();
            const long lXDiff = aPt.X() - pFly->Frm().Left();
            const long lYDiff = aPt.Y() - pFly->Frm().Top();
            const Point aTmp( rHori.GetPos() + lXDiff,
                              rVert.GetPos() + lYDiff );
            pFly->ChgRelPos( aTmp );
        }
    }

    SwFlyFrmFmt *pFlyFrmFmt = pFly->GetFmt();
    OSL_ENSURE( pFlyFrmFmt, "fly frame format missing!" );
    if ( pFlyFrmFmt )
        pFlyFrmFmt->SetLastFlyFrmPrtRectPos( pFly->Prt().Pos() ); //stores the value of last Prt rect

    EndAllAction();

    return aResult;
}


/***********************************************************************
#*  Class       :  SwFEShell
#*  Methode     :  WizzardFindCurFrmFmt
#***********************************************************************/

SwFrmFmt* SwFEShell::WizzardGetFly()
{
    // mal nicht uebers Layout den Fly suchen. Dann kann auch ohne gueltiges
    // Layout ein Rahmen geloescht werden. ( z.B.: fuer die Wizard's )
    SwSpzFrmFmts& rSpzArr = *pDoc->GetSpzFrmFmts();
    sal_uInt16 nCnt = rSpzArr.Count();
    if( nCnt )
    {
        SwNodeIndex& rCrsrNd = GetCrsr()->GetPoint()->nNode;
        if( rCrsrNd.GetIndex() > pDoc->GetNodes().GetEndOfExtras().GetIndex() )
            // Cusor steht im Body-Bereich!
            return 0;

        for( sal_uInt16 n = 0; n < nCnt; ++n )
        {
            SwFrmFmt* pFmt = rSpzArr[ n ];
            const SwNodeIndex* pIdx = pFmt->GetCntnt( sal_False ).GetCntntIdx();
            SwStartNode* pSttNd;
            if( pIdx &&
                0 != ( pSttNd = pIdx->GetNode().GetStartNode() ) &&
                pSttNd->GetIndex() < rCrsrNd.GetIndex() &&
                rCrsrNd.GetIndex() < pSttNd->EndOfSectionIndex() )
            {
                // gefunden: also raus damit
                return pFmt;
            }
        }
    }
    return 0;
}

void SwFEShell::SetFlyName( const String& rName )
{
    SwLayoutFrm *pFly = FindFlyFrm();
    if( pFly )
        GetDoc()->SetFlyName( *(SwFlyFrmFmt*)pFly->GetFmt(), rName );
    else {
        OSL_ENSURE( !this, "kein FlyFrame selektiert" );
    }
}

const String& SwFEShell::GetFlyName() const
{
    SwLayoutFrm *pFly = FindFlyFrm();
    if( pFly )
        return pFly->GetFmt()->GetName();

    OSL_ENSURE( !this, "kein FlyFrame selektiert" );
    return aEmptyStr;
}


const uno::Reference < embed::XEmbeddedObject > SwFEShell::GetOleRef() const
{
    uno::Reference < embed::XEmbeddedObject > xObj;
    SwFlyFrm * pFly = FindFlyFrm();
    if (pFly && pFly->Lower() && pFly->Lower()->IsNoTxtFrm())
    {
        SwOLENode *pNd = ((SwNoTxtFrm*)pFly->Lower())->GetNode()->GetOLENode();
        if (pNd)
            xObj = pNd->GetOLEObj().GetOleRef();
    }
    return xObj;
}


String SwFEShell::GetUniqueGrfName() const
{
    return GetDoc()->GetUniqueGrfName();
}

const SwFrmFmt* SwFEShell::IsURLGrfAtPos( const Point& rPt, String* pURL,
                                        String *pTargetFrameName,
                                        String *pDescription ) const
{
    if( !Imp()->HasDrawView() )
        return 0;

    SdrObject* pObj;
    SdrPageView* pPV;
    const SwFrmFmt* pRet = 0;
    SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();

    sal_uInt16 nOld = pDView->GetHitTolerancePixel();
    pDView->SetHitTolerancePixel( 2 );

    if( pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPV,SDRSEARCH_PICKMACRO ) &&
        pObj->ISA(SwVirtFlyDrawObj) )
    {
        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
        const SwFmtURL &rURL = pFly->GetFmt()->GetURL();
        if( rURL.GetURL().Len() || rURL.GetMap() )
        {
            sal_Bool bSetTargetFrameName = pTargetFrameName != 0;
            sal_Bool bSetDescription = pDescription != 0;
            if ( rURL.GetMap() )
            {
                IMapObject *pObject = pFly->GetFmt()->GetIMapObject( rPt, pFly );
                if ( pObject && pObject->GetURL().Len() )
                {
                    if( pURL )
                        *pURL = pObject->GetURL();
                    if ( bSetTargetFrameName && pObject->GetTarget().Len() )
                    {
                        bSetTargetFrameName = sal_False;
                        *pTargetFrameName = pObject->GetTarget();
                    }
                    if ( bSetDescription )
                    {
                        bSetDescription = sal_False;
                        *pDescription = pObject->GetAltText();
                    }
                    pRet = pFly->GetFmt();
                }
            }
            else
            {
                if( pURL )
                {
                    *pURL = rURL.GetURL();
                    if( rURL.IsServerMap() )
                    {
                        // dann die rel. Pixel Position anhaengen !!
                        Point aPt( rPt );
                        aPt -= pFly->Frm().Pos();
                        // ohne MapMode-Offset, ohne Offset, o ... !!!!!
                        aPt = GetOut()->LogicToPixel(
                                aPt, MapMode( MAP_TWIP ) );
                        ((( *pURL += '?' ) += String::CreateFromInt32( aPt.X() ))
                                  += ',' ) += String::CreateFromInt32(aPt.Y() );
                    }
                }
                pRet = pFly->GetFmt();
            }
            if ( bSetTargetFrameName )
                *pTargetFrameName = rURL.GetTargetFrameName();
            if ( bSetDescription )
                *pDescription = pFly->GetFmt()->GetName();
        }
    }
    pDView->SetHitTolerancePixel( nOld );
    return pRet;
}

const Graphic *SwFEShell::GetGrfAtPos( const Point &rPt,
                                       String &rName, sal_Bool &rbLink ) const
{
    if( !Imp()->HasDrawView() )
        return 0;

    SdrObject* pObj;
    SdrPageView* pPV;
    SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();

    if( pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPV ) && pObj->ISA(SwVirtFlyDrawObj) )
    {
        SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
        if ( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
        {
            SwGrfNode *pNd = ((SwCntntFrm*)pFly->Lower())->GetNode()->GetGrfNode();
            if ( pNd )
            {
                if ( pNd->IsGrfLink() )
                {
                    //Halbfertige Grafik?
                    ::sfx2::SvLinkSource* pLnkObj = pNd->GetLink()->GetObj();
                    if( pLnkObj && pLnkObj->IsPending() )
                        return 0;
                    rbLink = sal_True;
                }

                pNd->GetFileFilterNms( &rName, 0 );
                if ( !rName.Len() )
                    rName = pFly->GetFmt()->GetName();
                pNd->SwapIn( sal_True );
                return &pNd->GetGrf();
            }
        }
    }
    return 0;
}


const SwFrmFmt* SwFEShell::GetFmtFromObj( const Point& rPt, SwRect** pRectToFill ) const
{
    SwFrmFmt* pRet = 0;

    if( Imp()->HasDrawView() )
    {
        SdrObject* pObj;
        SdrPageView* pPView;

        SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();

        sal_uInt16 nOld = pDView->GetHitTolerancePixel();
        // Tattergrenze fuer Drawing-SS
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        if( pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPView, SDRSEARCH_PICKMARKABLE ) )
        {
            // dann teste mal was es ist:
            if ( pObj->ISA(SwVirtFlyDrawObj) )
                pRet = ((SwVirtFlyDrawObj*)pObj)->GetFmt();
            else if ( pObj->GetUserCall() ) //nicht fuer Gruppenobjekte
                pRet = ((SwDrawContact*)pObj->GetUserCall())->GetFmt();
            if(pRet && pRectToFill)
                **pRectToFill = pObj->GetCurrentBoundRect();
        }
        pDView->SetHitTolerancePixel( nOld );
    }
    return pRet;
}

// returns a format too, if the point is over the text of any fly
const SwFrmFmt* SwFEShell::GetFmtFromAnyObj( const Point& rPt ) const
{
    const SwFrmFmt* pRet = GetFmtFromObj( rPt );
    if( !pRet || RES_FLYFRMFMT == pRet->Which() )
    {
        SwPosition aPos( *GetCrsr()->GetPoint() );
        Point aPt( rPt );
        GetLayout()->GetCrsrOfst( &aPos, aPt );
        SwCntntNode *pNd = aPos.nNode.GetNode().GetCntntNode();
        SwFrm* pFrm = pNd->GetFrm( &rPt )->FindFlyFrm();
        pRet = pFrm ? ((SwLayoutFrm*)pFrm)->GetFmt() : 0;
    }
    return pRet;
}

ObjCntType SwFEShell::GetObjCntType( const SdrObject& rObj ) const
{
    ObjCntType eType = OBJCNT_NONE;

    // investigate 'master' drawing object, if method
    // is called for a 'virtual' drawing object.
    const SdrObject* pInvestigatedObj;
    if ( rObj.ISA(SwDrawVirtObj) )
    {
        const SwDrawVirtObj* pDrawVirtObj = static_cast<const SwDrawVirtObj*>(&rObj);
        pInvestigatedObj = &(pDrawVirtObj->GetReferencedObj());
    }
    else
    {
        pInvestigatedObj = &rObj;
    }

    if( FmFormInventor == pInvestigatedObj->GetObjInventor() )
    {
        eType = OBJCNT_CONTROL;
        uno::Reference< awt::XControlModel >  xModel =
                ((SdrUnoObj&)(*pInvestigatedObj)).GetUnoControlModel();
        if( xModel.is() )
        {
            uno::Any aVal;
            OUString sName(RTL_CONSTASCII_USTRINGPARAM("ButtonType"));
            uno::Reference< beans::XPropertySet >  xSet(xModel, uno::UNO_QUERY);

            uno::Reference< beans::XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
            if(xInfo->hasPropertyByName( sName ))
            {
                beans::Property xProperty = xInfo->getPropertyByName( sName );
                aVal = xSet->getPropertyValue( sName );
                if( aVal.getValue() && form::FormButtonType_URL == *((form::FormButtonType*)aVal.getValue()) )
                    eType = OBJCNT_URLBUTTON;
            }
        }
    }
    else if( pInvestigatedObj->ISA(SwVirtFlyDrawObj) )
    {
        SwFlyFrm *pFly = ((SwVirtFlyDrawObj&)(*pInvestigatedObj)).GetFlyFrm();
        if ( pFly->Lower() && pFly->Lower()->IsNoTxtFrm() )
        {
            if ( ((SwCntntFrm*)pFly->Lower())->GetNode()->GetGrfNode() )
                eType = OBJCNT_GRF;
            else
                eType = OBJCNT_OLE;
        }
        else
            eType = OBJCNT_FLY;
    }
    else if ( pInvestigatedObj->ISA( SdrObjGroup ) )
    {
        SwDrawContact* pDrawContact( dynamic_cast<SwDrawContact*>(GetUserCall( pInvestigatedObj ) ) );
        if ( !pDrawContact )
        {
            OSL_ENSURE( false,
                    "<SwFEShell::GetObjCntType(..)> - missing draw contact object" );
            eType = OBJCNT_NONE;
        }
        else
        {
            SwFrmFmt* pFrmFmt( pDrawContact->GetFmt() );
            if ( !pFrmFmt )
            {
                OSL_ENSURE( false,
                        "<SwFEShell::GetObjCntType(..)> - missing frame format" );
                eType = OBJCNT_NONE;
            }
            else if ( FLY_AS_CHAR != pFrmFmt->GetAnchor().GetAnchorId() )
            {
                eType = OBJCNT_GROUPOBJ;
            }
        }
    }
    else
        eType = OBJCNT_SIMPLE;
    return eType;
}

ObjCntType SwFEShell::GetObjCntType( const Point &rPt, SdrObject *&rpObj ) const
{
    ObjCntType eType = OBJCNT_NONE;

    if( Imp()->HasDrawView() )
    {
        SdrObject* pObj;
        SdrPageView* pPView;

        SwDrawView *pDView = (SwDrawView*)Imp()->GetDrawView();

        sal_uInt16 nOld = pDView->GetHitTolerancePixel();
        // Tattergrenze fuer Drawing-SS
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        if( pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPView, SDRSEARCH_PICKMARKABLE ) )
            eType = GetObjCntType( *(rpObj = pObj) );

        pDView->SetHitTolerancePixel( nOld );
    }
    return eType;
}

ObjCntType SwFEShell::GetObjCntTypeOfSelection( SdrObject** ppObj ) const
{
    ObjCntType eType = OBJCNT_NONE;

    if( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        for( sal_uInt32 i = 0, nE = rMrkList.GetMarkCount(); i < nE; ++i )
        {
            SdrObject* pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            ObjCntType eTmp = GetObjCntType( *pObj );
            if( !i )
            {
                eType = eTmp;
                if( ppObj ) *ppObj = pObj;
            }
            else if( eTmp != eType )
            {
                eType = OBJCNT_DONTCARE;
                // einmal DontCare, immer DontCare!
                break;
            }
        }
    }
    return eType;
}


sal_Bool SwFEShell::ReplaceSdrObj( const String& rGrfName, const String& rFltName,
                                const Graphic* pGrf )
{
    SET_CURR_SHELL( this );

    sal_Bool bRet = sal_False;
    const SdrMarkList *pMrkList;
    if( Imp()->HasDrawView() &&  1 ==
        ( pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList())->GetMarkCount() )
    {
        SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
        SwFrmFmt *pFmt = FindFrmFmt( pObj );

        // Attribute sichern und dann an der Grafik setzen
        SfxItemSet aFrmSet( pDoc->GetAttrPool(),
                            pFmt->GetAttrSet().GetRanges() );
        aFrmSet.Set( pFmt->GetAttrSet() );

        // Groesse und Position setzen ??
        if( !pObj->ISA(SwVirtFlyDrawObj) )
        {
            // dann mal los:
            const Rectangle &rBound = pObj->GetSnapRect();
            Point aRelPos( pObj->GetRelativePos() );

            const long nWidth = rBound.Right()  - rBound.Left();
            const long nHeight= rBound.Bottom() - rBound.Top();
            aFrmSet.Put( SwFmtFrmSize( ATT_MIN_SIZE,
                                Max( nWidth,  long(MINFLY) ),
                                Max( nHeight, long(MINFLY) )));

            if( SFX_ITEM_SET != aFrmSet.GetItemState( RES_HORI_ORIENT ))
                aFrmSet.Put( SwFmtHoriOrient( aRelPos.X(), text::HoriOrientation::NONE, text::RelOrientation::FRAME ));

            if( SFX_ITEM_SET != aFrmSet.GetItemState( RES_VERT_ORIENT ))
                aFrmSet.Put( SwFmtVertOrient( aRelPos.Y(), text::VertOrientation::NONE, text::RelOrientation::FRAME ));

        }

        pObj->GetOrdNum();

        StartAllAction();
        StartUndo();

        // das "Sdr-Object" loeschen und dafuer die Grafik einfuegen
        DelSelectedObj();

        pFmt = GetDoc()->Insert( *GetCrsr(), rGrfName, rFltName, pGrf, &aFrmSet, NULL, NULL );

        EndUndo();
        EndAllAction();
        bRet = sal_True;
    }
    return bRet;
}

static sal_uInt16 SwFmtGetPageNum(const SwFlyFrmFmt * pFmt)
{
    OSL_ENSURE(pFmt != NULL, "invalid argument");

    SwFlyFrm * pFrm = pFmt->GetFrm();

    sal_uInt16 aResult;

    if (pFrm != NULL)
        aResult = pFrm->GetPhyPageNum();
    else
        aResult = pFmt->GetAnchor().GetPageNum();

    return aResult;
}

#include <fmtcnct.hxx>

void SwFEShell::GetConnectableFrmFmts(SwFrmFmt & rFmt,
                                      const String & rReference,
                                      sal_Bool bSuccessors,
                                      ::std::vector< String > & aPrevPageVec,
                                      ::std::vector< String > & aThisPageVec,
                                      ::std::vector< String > & aNextPageVec,
                                      ::std::vector< String > & aRestVec)
{
    StartAction();

    SwFmtChain rChain = rFmt.GetChain();
    SwFrmFmt * pOldChainNext = (SwFrmFmt *) rChain.GetNext();
    SwFrmFmt * pOldChainPrev = (SwFrmFmt *) rChain.GetPrev();

    if (pOldChainNext)
        pDoc->Unchain(rFmt);

    if (pOldChainPrev)
        pDoc->Unchain(*pOldChainPrev);

    sal_uInt16 nCnt = pDoc->GetFlyCount(FLYCNTTYPE_FRM);

    /* potential successors resp. predecessors */
    ::std::vector< const SwFrmFmt * > aTmpSpzArray;

    (SwFrmFmt *) pDoc->FindFlyByName(rReference);

    for (sal_uInt16 n = 0; n < nCnt; n++)
    {
        const SwFrmFmt & rFmt1 = *(pDoc->GetFlyNum(n, FLYCNTTYPE_FRM));

        /*
           pFmt is a potential successor of rFmt if it is chainable after
           rFmt.

           pFmt is a potential predecessor of rFmt if rFmt is chainable
           after pFmt.
        */

        int nChainState;

        if (bSuccessors)
            nChainState = pDoc->Chainable(rFmt, rFmt1);
        else
            nChainState = pDoc->Chainable(rFmt1, rFmt);

        if (nChainState == SW_CHAIN_OK)
        {
            aTmpSpzArray.push_back(&rFmt1);

        }

    }

    if  (aTmpSpzArray.size() > 0)
    {
        aPrevPageVec.clear();
        aThisPageVec.clear();
        aNextPageVec.clear();
        aRestVec.clear();

        /* number of page rFmt resides on */
        sal_uInt16 nPageNum = SwFmtGetPageNum((SwFlyFrmFmt *) &rFmt);

        ::std::vector< const SwFrmFmt * >::const_iterator aIt;

        for (aIt = aTmpSpzArray.begin(); aIt != aTmpSpzArray.end(); aIt++)
        {
            String  aString = (*aIt)->GetName();

            /* rFmt is not a vaild successor or predecessor of
               itself */
            if (aString != rReference && aString != rFmt.GetName())
            {
                sal_uInt16 nNum1 =
                    SwFmtGetPageNum((SwFlyFrmFmt *) *aIt);

                if (nNum1 == nPageNum -1)
                    aPrevPageVec.push_back(aString);
                else if (nNum1 == nPageNum)
                    aThisPageVec.push_back(aString);
                else if (nNum1 == nPageNum + 1)
                    aNextPageVec.push_back(aString);
                else
                    aRestVec.push_back(aString);
            }
        }

    }

    if (pOldChainNext)
        pDoc->Chain(rFmt, *pOldChainNext);

    if (pOldChainPrev)
        pDoc->Chain(*pOldChainPrev, rFmt);

    EndAction();
}

// --> OD 2009-07-13 #i73249#
const String SwFEShell::GetObjTitle() const
{
    String aTitle;

    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList *pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        if ( pMrkList->GetMarkCount() == 1 )
        {
            const SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            const SwFrmFmt* pFmt = FindFrmFmt( pObj );
            if ( pFmt->Which() == RES_FLYFRMFMT )
            {
                aTitle = dynamic_cast<const SwFlyFrmFmt*>(pFmt)->GetObjTitle();
            }
            else
            {
                aTitle = pObj->GetTitle();
            }
        }
    }

    return aTitle;
}

void SwFEShell::SetObjTitle( const String& rTitle )
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList *pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        if ( pMrkList->GetMarkCount() == 1 )
        {
            SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            SwFrmFmt* pFmt = FindFrmFmt( pObj );
            if ( pFmt->Which() == RES_FLYFRMFMT )
            {
                GetDoc()->SetFlyFrmTitle( *(dynamic_cast<SwFlyFrmFmt*>(pFmt)),
                                          rTitle );
            }
            else
            {
                pObj->SetTitle( rTitle );
            }
        }
    }
}

const String SwFEShell::GetObjDescription() const
{
    String aDescription;

    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList *pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        if ( pMrkList->GetMarkCount() == 1 )
        {
            const SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            const SwFrmFmt* pFmt = FindFrmFmt( pObj );
            if ( pFmt->Which() == RES_FLYFRMFMT )
            {
                aDescription = dynamic_cast<const SwFlyFrmFmt*>(pFmt)->GetObjDescription();
            }
            else
            {
                aDescription = pObj->GetDescription();
            }
        }
    }

    return aDescription;
}

void SwFEShell::SetObjDescription( const String& rDescription )
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList *pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        if ( pMrkList->GetMarkCount() == 1 )
        {
            SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            SwFrmFmt* pFmt = FindFrmFmt( pObj );
            if ( pFmt->Which() == RES_FLYFRMFMT )
            {
                GetDoc()->SetFlyFrmDescription( *(dynamic_cast<SwFlyFrmFmt*>(pFmt)),
                                                rDescription );
            }
            else
            {
                pObj->SetDescription( rDescription );
            }
        }
    }
}


void SwFEShell::AlignFormulaToBaseline( const uno::Reference < embed::XEmbeddedObject >& xObj, SwFlyFrm * pFly )
{
#if OSL_DEBUG_LEVEL > 1
    SvGlobalName aCLSID( xObj->getClassID() );
    const bool bStarMath = ( SotExchange::IsMath( aCLSID ) != 0 );
    OSL_ENSURE( bStarMath, "AlignFormulaToBaseline should only be called for Math objects" );

    if ( !bStarMath )
        return;
#endif

    if (!pFly)
        pFly = FindFlyFrm( xObj );
    OSL_ENSURE( pFly , "No fly frame!" );
    SwFrmFmt * pFrmFmt = pFly ? pFly->GetFmt() : 0;

    // baseline to baseline alignment should only be applied to formulas anchored as char
    if ( pFly && pFrmFmt && FLY_AS_CHAR == pFrmFmt->GetAnchor().GetAnchorId() )
    {
        // get baseline from Math object
        uno::Any aBaseline;
        if( svt::EmbeddedObjectRef::TryRunningState( xObj ) )
        {
            uno::Reference < beans::XPropertySet > xSet( xObj->getComponent(), uno::UNO_QUERY );
            if ( xSet.is() )
            {
                try
                {
                    aBaseline = xSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BaseLine") ) );
                }
                catch ( uno::Exception& )
                {
                    OSL_ENSURE( sal_False , "Baseline could not be retrieved from Starmath!" );
                }
            }
        }

        sal_Int32 nBaseline = ::comphelper::getINT32(aBaseline);
        const MapMode aSourceMapMode( MAP_100TH_MM );
        const MapMode aTargetMapMode( MAP_TWIP );
        nBaseline = OutputDevice::LogicToLogic( nBaseline, aSourceMapMode.GetMapUnit(), aTargetMapMode.GetMapUnit() );

        OSL_ENSURE( nBaseline > 0, "Wrong value of Baseline while retrieving from Starmath!" );
        //nBaseline must be moved by aPrt position
        const SwFlyFrmFmt *pFlyFrmFmt = pFly->GetFmt();
        OSL_ENSURE( pFlyFrmFmt, "fly frame format missing!" );
        if ( pFlyFrmFmt )
            nBaseline += pFlyFrmFmt->GetLastFlyFrmPrtRectPos().Y();

        const SwFmtVertOrient &rVert = pFrmFmt->GetVertOrient();
        SwFmtVertOrient aVert( rVert );
        aVert.SetPos( -nBaseline );
        aVert.SetVertOrient( com::sun::star::text::VertOrientation::NONE );

        pFrmFmt->LockModify();
        pFrmFmt->SetFmtAttr( aVert );
        pFrmFmt->UnlockModify();
        pFly->InvalidatePos();
    }
}


void SwFEShell::AlignAllFormulasToBaseline()
{
    StartAllAction();

    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        SwOLENode *pOleNode = dynamic_cast< SwOLENode * >( &aIdx.GetNode() );
        if ( pOleNode )
        {
            const uno::Reference < embed::XEmbeddedObject > & xObj( pOleNode->GetOLEObj().GetOleRef() );
            if (xObj.is())
            {
                SvGlobalName aCLSID( xObj->getClassID() );
                if ( SotExchange::IsMath( aCLSID ) )
                    AlignFormulaToBaseline( xObj );
            }
        }

        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }

    EndAllAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
