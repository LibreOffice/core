/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <dview.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <ndtxt.hxx>
#include <edimp.hxx>
#include <swtable.hxx>
#include <mvsave.hxx>
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
#include <fmtsrnd.hxx>
#include <ndole.hxx>
#include <editeng/opaqitem.hxx>
#include <fefly.hxx>

using namespace ::com::sun::star;

// Based on the request, changes to the specific layouts will be made, to
// fit to the format
static bool lcl_SetNewFlyPos( const SwNode& rNode, SwFormatAnchor& rAnchor,
                        const Point& rPt )
{
    bool bRet = false;
    const SwStartNode* pStNode = rNode.FindFlyStartNode();
    if( pStNode )
    {
        SwPosition aPos( *pStNode );
        rAnchor.SetAnchor( &aPos );
        bRet = true;
    }
    else
    {
        const SwContentNode *pCntNd = rNode.GetContentNode();
        const SwContentFrm* pCFrm = pCntNd ? pCntNd->getLayoutFrm( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &rPt, 0, false ) : 0;
        const SwPageFrm *pPg = pCFrm ? pCFrm->FindPageFrm() : 0;

        rAnchor.SetPageNum( pPg ? pPg->GetPhyPageNum() : 1 );
        rAnchor.SetType( FLY_AT_PAGE );
    }
    return bRet;
}

static bool lcl_FindAnchorPos(
    SwDoc& rDoc,
    const Point& rPt,
    const SwFrm& rFrm,
    SfxItemSet& rSet )
{
    bool bRet = true;
    SwFormatAnchor aNewAnch( static_cast<const SwFormatAnchor&>(rSet.Get( RES_ANCHOR )) );
    RndStdIds nNew = aNewAnch.GetAnchorId();
    const SwFrm *pNewAnch;

    //determine new anchor
    Point aTmpPnt( rPt );
    switch( nNew )
    {
    case FLY_AS_CHAR:  // also include this?
    case FLY_AT_PARA:
    case FLY_AT_CHAR: // LAYER_IMPL
        {
            // starting from the upper-left corner of the Fly,
            // search nearest ContentFrm
            const SwFrm* pFrm = rFrm.IsFlyFrm() ? static_cast<const SwFlyFrm&>(rFrm).GetAnchorFrm()
                                                : &rFrm;
            pNewAnch = ::FindAnchor( pFrm, aTmpPnt );
            if( pNewAnch->IsProtected() )
            {
                bRet = false;
                break;
            }

            SwPosition aPos( *static_cast<const SwContentFrm*>(pNewAnch)->GetNode() );
            if ((FLY_AT_CHAR == nNew) || (FLY_AS_CHAR == nNew))
            {
                // textnode should be found, as only in those
                // a content bound frame can be anchored
                SwCrsrMoveState aState( MV_SETONLYTEXT );
                aTmpPnt.setX(aTmpPnt.getX() - 1);                   // do not land in the fly!
                if( !pNewAnch->GetCrsrOfst( &aPos, aTmpPnt, &aState ) )
                {
                    SwContentNode* pCNd = const_cast<SwContentFrm*>(static_cast<const SwContentFrm*>(pNewAnch))->GetNode();
                    if( pNewAnch->Frm().Bottom() < aTmpPnt.Y() )
                        pCNd->MakeStartIndex( &aPos.nContent );
                    else
                        pCNd->MakeEndIndex( &aPos.nContent );
                }
                else
                {
                    if ( SwCrsrShell::PosInsideInputField( aPos ) )
                    {
                        aPos.nContent = SwCrsrShell::StartOfInputFieldAtPos( aPos );
                    }
                }
            }
            aNewAnch.SetAnchor( &aPos );
        }
        break;

    case FLY_AT_FLY: // LAYER_IMPL
        {
            // starting from the upper-left corner of the Fly
            // search nearest SwFlyFrm
            SwCrsrMoveState aState( MV_SETONLYTEXT );
            SwPosition aPos( rDoc.GetNodes() );
            aTmpPnt.setX(aTmpPnt.getX() - 1);                   // do not land in the fly!
            rDoc.getIDocumentLayoutAccess().GetCurrentLayout()->GetCrsrOfst( &aPos, aTmpPnt, &aState );
            pNewAnch = ::FindAnchor(
                aPos.nNode.GetNode().GetContentNode()->getLayoutFrm( rFrm.getRootFrm(), 0, 0, false ),
                aTmpPnt )->FindFlyFrm();

            if( pNewAnch && &rFrm != pNewAnch && !pNewAnch->IsProtected() )
            {
                aPos.nNode = *static_cast<const SwFlyFrm*>(pNewAnch)->GetFormat()->GetContent().
                                GetContentIdx();
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
        OSL_ENSURE( false, "Falsche ID fuer neuen Anker." );
    }

    rSet.Put( aNewAnch );
    return bRet;
}

//! also used in unoframe.cxx

bool sw_ChkAndSetNewAnchor(
    const SwFlyFrm& rFly,
    SfxItemSet& rSet )
{
    const SwFrameFormat& rFormat = *rFly.GetFormat();
    const SwFormatAnchor &rOldAnch = rFormat.GetAnchor();
    const RndStdIds nOld = rOldAnch.GetAnchorId();

    RndStdIds nNew = static_cast<const SwFormatAnchor&>(rSet.Get( RES_ANCHOR )).GetAnchorId();

    if( nOld == nNew )
        return false;

    SwDoc* pDoc = const_cast<SwDoc*>(rFormat.GetDoc());

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( !(nNew == FLY_AT_PAGE &&
        (FLY_AT_PARA==nOld || FLY_AT_CHAR==nOld || FLY_AS_CHAR==nOld ) &&
        pDoc->IsInHeaderFooter( rOldAnch.GetContentAnchor()->nNode )),
            "forbidden anchor change in Head/Foot." );
#endif

    return ::lcl_FindAnchorPos( *pDoc, rFly.Frm().Pos(), rFly, rSet );
}

void SwFEShell::SelectFlyFrm( SwFlyFrm& rFrm, bool bNew )
{
    SET_CURR_SHELL( this );

    // The frame is new, thus select it.
    // !! Always select the frame, if it's not selected.
    // - it could be a new "old" one because the anchor was changed
    // - "old" frames have had to be selected previously otherwise they could
    //   not have been changed
    // The frames should not be selected by the document position, because
    // it should have been selected!
    SwViewShellImp *pImpl = Imp();
    if( GetWin() && (bNew || !pImpl->GetDrawView()->AreObjectsMarked()) )
    {
        OSL_ENSURE( rFrm.IsFlyFrm(), "SelectFlyFrm will einen Fly" );

       // nothing to be done if the Fly already was selected
        if (GetSelectedFlyFrm() == &rFrm)
            return;

        // assure the anchor is drawn
        if( rFrm.IsFlyInCntFrm() && rFrm.GetAnchorFrm() )
             rFrm.GetAnchorFrm()->SetCompletePaint();

        if( pImpl->GetDrawView()->AreObjectsMarked() )
            pImpl->GetDrawView()->UnmarkAll();

        pImpl->GetDrawView()->MarkObj( rFrm.GetVirtDrawObj(),
                                      pImpl->GetPageView() );
        KillPams();
        ClearMark();
        SelFlyGrabCrsr();
    }
}

// Get selected fly
SwFlyFrm* SwFEShell::GetSelectedFlyFrm() const
{
    if ( Imp()->HasDrawView() )
    {
        // A Fly is only accessible if it is selected
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        if( rMrkList.GetMarkCount() != 1 )
            return 0;

        SdrObject *pO = rMrkList.GetMark( 0 )->GetMarkedSdrObj();

        SwVirtFlyDrawObj *pFlyObj = dynamic_cast<SwVirtFlyDrawObj*>(pO);

        return pFlyObj ? pFlyObj->GetFlyFrm() : nullptr;
    }
    return 0;
}

// Get current fly in which the cursor is positioned
SwFlyFrm* SwFEShell::GetCurrFlyFrm(const bool bCalcFrm) const
{
    SwContentFrm *pContent = GetCurrFrm(bCalcFrm);
    return pContent ? pContent->FindFlyFrm() : 0;
}

// Get selected fly, but if none Get current fly in which the cursor is positioned
SwFlyFrm* SwFEShell::GetSelectedOrCurrFlyFrm(const bool bCalcFrm) const
{
    SwFlyFrm *pFly = GetSelectedFlyFrm();
    if (pFly)
        return pFly;
    return GetCurrFlyFrm(bCalcFrm);
}

// Returns non-null pointer, if the current Fly could be anchored to another one (so it is inside)
const SwFrameFormat* SwFEShell::IsFlyInFly()
{
    SET_CURR_SHELL( this );

    if ( !Imp()->HasDrawView() )
        return NULL;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if ( !rMrkList.GetMarkCount() )
    {
        SwFlyFrm *pFly = GetCurrFlyFrm(false);
        if (!pFly)
            return NULL;
        return pFly->GetFormat();
    }
    else if ( rMrkList.GetMarkCount() != 1 ||
         !GetUserCall(rMrkList.GetMark( 0 )->GetMarkedSdrObj()) )
        return NULL;

    SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();

    SwFrameFormat *pFormat = FindFrameFormat( pObj );
    if( pFormat && FLY_AT_FLY == pFormat->GetAnchor().GetAnchorId() )
    {
        const SwFrm* pFly;
        if (SwVirtFlyDrawObj* pFlyObj = dynamic_cast<SwVirtFlyDrawObj *>(pObj))
        {
            pFly = pFlyObj->GetFlyFrm()->GetAnchorFrm();
        }
        else
        {
            pFly = static_cast<SwDrawContact*>(GetUserCall(pObj))->GetAnchorFrm(pObj);
        }

        OSL_ENSURE( pFly, "IsFlyInFly: Where's my anchor?" );
        OSL_ENSURE( pFly->IsFlyFrm(), "IsFlyInFly: Funny anchor!" );
        return static_cast<const SwFlyFrm*>(pFly)->GetFormat();
    }

    Point aTmpPos = pObj->GetCurrentBoundRect().TopLeft();

    SwFrm *pTextFrm;
    {
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        SwNodeIndex aSwNodeIndex( GetDoc()->GetNodes() );
        SwPosition aPos( aSwNodeIndex );
        Point aPoint( aTmpPos );
        aPoint.setX(aPoint.getX() - 1);                    //do not land in the fly!!
        GetLayout()->GetCrsrOfst( &aPos, aPoint, &aState );
        // determine text frame by left-top-corner of object
        SwContentNode *pNd = aPos.nNode.GetNode().GetContentNode();
        pTextFrm = pNd ? pNd->getLayoutFrm(GetLayout(), &aTmpPos, 0, false) : NULL;
    }
    const SwFrm *pTmp = pTextFrm ? ::FindAnchor(pTextFrm, aTmpPos) : NULL;
    const SwFlyFrm *pFly = pTmp ? pTmp->FindFlyFrm() : NULL;
    if( pFly )
        return pFly->GetFormat();
    return NULL;
}

void SwFEShell::SetFlyPos( const Point& rAbsPos )
{
    SET_CURR_SHELL( this );

    // Determine reference point in document coordinates
    SwFlyFrm *pFly = GetCurrFlyFrm(false);
    if (!pFly)
        return;

    //SwSaveHdl aSaveX( Imp() );

    // Set an anchor starting from the absolute position for paragraph bound Flys
    // Anchor and new RelPos will be calculated and set by the Fly
    if ( pFly->IsFlyAtCntFrm() )
        static_cast<SwFlyAtCntFrm*>(pFly)->SetAbsPos( rAbsPos );
    else
    {
            const SwFrm *pAnch = pFly->GetAnchorFrm();
            Point aOrient( pAnch->Frm().Pos() );

        if ( pFly->IsFlyInCntFrm() )
            aOrient.setX(rAbsPos.getX());

        // calculate RelPos.
        aOrient.setX(rAbsPos.getX() - aOrient.getX());
        aOrient.setY(rAbsPos.getY() - aOrient.getY());
        pFly->ChgRelPos( aOrient );
    }
    CallChgLnk();       // call the AttrChangeNotify on the UI-side.
}

Point SwFEShell::FindAnchorPos( const Point& rAbsPos, bool bMoveIt )
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
    // #i28701#
    SwAnchoredObject* pAnchoredObj = ::GetUserCall( pObj )->GetAnchoredObj( pObj );
    SwFrameFormat& rFormat = pAnchoredObj->GetFrameFormat();
    const RndStdIds nAnchorId = rFormat.GetAnchor().GetAnchorId();

    if ( FLY_AS_CHAR == nAnchorId )
        return aRet;

    bool bFlyFrame = dynamic_cast<SwVirtFlyDrawObj *>(pObj) != nullptr;

    SwFlyFrm* pFly = 0L;
    const SwFrm* pFooterOrHeader = NULL;

    if( bFlyFrame )
    {
        // Calculate reference point in document coordinates
        SwContentFrm *pContent = GetCurrFrm( false );
        if( !pContent )
            return aRet;
        pFly = pContent->FindFlyFrm();
        if ( !pFly )
            return aRet;
        const SwFrm* pOldAnch = pFly->GetAnchorFrm();
        if( !pOldAnch )
            return aRet;
        if ( FLY_AT_PAGE != nAnchorId )
        {
            pFooterOrHeader = pContent->FindFooterOrHeader();
        }
    }
    // set <pFooterOrHeader> also for drawing
    // objects, but not for control objects.
    // Necessary for moving 'anchor symbol' at the user interface inside header/footer.
    else if ( !::CheckControlLayer( pObj ) )
    {
        SwContentFrm *pContent = GetCurrFrm( false );
        if( !pContent )
            return aRet;
        pFooterOrHeader = pContent->FindFooterOrHeader();
    }

    // Search nearest SwFlyFrm starting from the upper-left corner
    // of the fly
    SwContentFrm *pTextFrm = NULL;
    {
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        SwPosition aPos( GetDoc()->GetNodes().GetEndOfExtras() );
        Point aTmpPnt( rAbsPos );
        GetLayout()->GetCrsrOfst( &aPos, aTmpPnt, &aState );
        if (aPos.nNode != GetDoc()->GetNodes().GetEndOfExtras().GetIndex()
            && (nAnchorId != FLY_AT_CHAR || !PosInsideInputField(aPos)))
        {
            SwContentNode* pCNode = aPos.nNode.GetNode().GetContentNode();
            assert(pCNode);
            pTextFrm = pCNode->getLayoutFrm(GetLayout(), 0, &aPos, false);
        }
    }
    const SwFrm *pNewAnch = NULL;
    if( pTextFrm != NULL )
    {
        if ( FLY_AT_PAGE == nAnchorId )
        {
            pNewAnch = pTextFrm->FindPageFrm();
        }
        else
        {
            pNewAnch = ::FindAnchor( pTextFrm, rAbsPos );

            if( FLY_AT_FLY == nAnchorId ) // LAYER_IMPL
            {
                pNewAnch = pNewAnch->FindFlyFrm();
            }
        }
    }

    if( pNewAnch && !pNewAnch->IsProtected() )
    {
        const SwFlyFrm* pCheck = bFlyFrame ? pNewAnch->FindFlyFrm() : 0;
        // If we land inside the frame, make sure
        // that the frame does not land inside its own content
        while( pCheck )
        {
            if( pCheck == pFly )
                break;
            const SwFrm *pTmp = pCheck->GetAnchorFrm();
            pCheck = pTmp ? pTmp->FindFlyFrm() : NULL;
        }

        // Do not switch from header/footer to another area,
        // do not switch to a header/footer
        if( !pCheck &&
            pFooterOrHeader == pNewAnch->FindFooterOrHeader() )
        {
            aRet = pNewAnch->GetFrmAnchorPos( ::HasWrap( pObj ) );

            if ( bMoveIt || (nAnchorId == FLY_AT_CHAR) )
            {
                SwFormatAnchor aAnch( rFormat.GetAnchor() );
                switch ( nAnchorId )
                {
                    case FLY_AT_PARA:
                    {
                        SwPosition pos = *aAnch.GetContentAnchor();
                        pos.nNode = *pTextFrm->GetNode();
                        pos.nContent.Assign(0,0);
                        aAnch.SetAnchor( &pos );
                        break;
                    }
                    case FLY_AT_PAGE:
                    {
                        aAnch.SetPageNum( static_cast<const SwPageFrm*>(pNewAnch)->
                                          GetPhyPageNum() );
                        break;
                    }

                    case FLY_AT_FLY:
                    {
                        SwPosition aPos( *static_cast<const SwFlyFrm*>(pNewAnch)->GetFormat()->
                                                  GetContent().GetContentIdx() );
                        aAnch.SetAnchor( &aPos );
                        break;
                    }

                    case FLY_AT_CHAR:
                        {
                            SwPosition pos = *aAnch.GetContentAnchor();
                            Point aTmpPnt( rAbsPos );
                            if( pTextFrm->GetCrsrOfst( &pos, aTmpPnt, NULL ) )
                            {
                                SwRect aTmpRect;
                                pTextFrm->GetCharRect( aTmpRect, pos );
                                aRet = aTmpRect.Pos();
                            }
                            else
                            {
                                pos.nNode = *pTextFrm->GetNode();
                                pos.nContent.Assign(0,0);
                            }
                            aAnch.SetAnchor( &pos );
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
                        SwFlyFrameFormat* pFlyFrameFormat( dynamic_cast<SwFlyFrameFormat*>(&rFormat) );
                        if ( pFlyFrameFormat )
                        {
                            pHandleAnchorNodeChg =
                                new SwHandleAnchorNodeChg( *pFlyFrameFormat, aAnch );
                        }
                        rFormat.GetDoc()->SetAttr( aAnch, rFormat );
                        delete pHandleAnchorNodeChg;
                    }
                    // #i28701# - no call of method
                    // <CheckCharRectAndTopOfLine()> for to-character anchored
                    // Writer fly frame needed. This method call can cause a
                    // format of the anchor frame, which is no longer intended.
                    // Instead clear the anchor character rectangle and
                    // the top of line values for all to-character anchored objects.
                    pAnchoredObj->ClearCharRectAndTopOfLine();
                    EndAllAction();
                }
            }

            SwRect aTmpRect( aRet, rAbsPos );
            if( aTmpRect.HasArea() )
                MakeVisible( aTmpRect );
#if OSL_DEBUG_LEVEL > 0
            //TODO: That doesn't seem to be intended
            if( Color(COL_TRANSPARENT) != GetOut()->GetLineColor() )
            {
                OSL_FAIL( "Hey, Joe: Where's my Null Pen?" );
                GetOut()->SetLineColor( Color(COL_TRANSPARENT) );
            }
#endif
        }
    }

    return aRet;
}

const SwFrameFormat *SwFEShell::NewFlyFrm( const SfxItemSet& rSet, bool bAnchValid,
                           SwFrameFormat *pParent )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCrsr = GetCrsr();
    const Point aPt( GetCrsrDocPos() );

    SwSelBoxes aBoxes;
    bool bMoveContent = true;
    if( IsTableMode() )
    {
        GetTableSel( *this, aBoxes );
        if( !aBoxes.empty() )
        {
            // Crsr should be removed from the removal area.
            // Always put it after/on the table; via the
            // document position they will be set to the old
            // position
            ParkCrsr( SwNodeIndex( *aBoxes[0]->GetSttNd() ));

            // #i127787# pCurCrsr will be deleted in ParkCrsr,
            // we better get the current pCurCrsr instead of working with the
            // deleted one:
            pCrsr = GetCrsr();
        }
        else
            bMoveContent = false;
    }
    else if( !pCrsr->HasMark() && !pCrsr->IsMultiSelection() )
        bMoveContent = false;

    const SwPosition& rPos = *pCrsr->Start();

    SwFormatAnchor& rAnch = const_cast<SwFormatAnchor&>(static_cast<const SwFormatAnchor&>(rSet.Get( RES_ANCHOR )));
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
        OSL_ENSURE( false, "What is the purpose of this Fly?" );
        break;
    }

    SwFlyFrameFormat *pRet;
    if( bMoveContent )
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_INSLAYFMT, NULL );
        SwFormatAnchor* pOldAnchor = 0;
        bool bHOriChgd = false, bVOriChgd = false;
        SwFormatVertOrient aOldV;
        SwFormatHoriOrient aOldH;

        if ( FLY_AT_PAGE != eRndId )
        {
           // First as with page link. Paragraph/character link on if
            // everything was shifted. Then the position is valid!
            // JP 13.05.98: if necessary also convert the horizontal/vertical
            //              orientation, to prevent correction during re-anchoring
            pOldAnchor = new SwFormatAnchor( rAnch );
            const_cast<SfxItemSet&>(rSet).Put( SwFormatAnchor( FLY_AT_PAGE, 1 ) );

            const SfxPoolItem* pItem;
            if( SfxItemState::SET == rSet.GetItemState( RES_HORI_ORIENT, false, &pItem )
                && text::HoriOrientation::NONE == static_cast<const SwFormatHoriOrient*>(pItem)->GetHoriOrient() )
            {
                bHOriChgd = true;
                aOldH = *static_cast<const SwFormatHoriOrient*>(pItem);
                const_cast<SfxItemSet&>(rSet).Put( SwFormatHoriOrient( 0, text::HoriOrientation::LEFT ) );
            }
            if( SfxItemState::SET == rSet.GetItemState( RES_VERT_ORIENT, false, &pItem )
                && text::VertOrientation::NONE == static_cast<const SwFormatVertOrient*>(pItem)->GetVertOrient() )
            {
                bVOriChgd = true;
                aOldV = *static_cast<const SwFormatVertOrient*>(pItem);
                const_cast<SfxItemSet&>(rSet).Put( SwFormatVertOrient( 0, text::VertOrientation::TOP ) );
            }
        }

        pRet = GetDoc()->MakeFlyAndMove( *pCrsr, rSet, &aBoxes, pParent );

        KillPams();

        if( pOldAnchor )
        {
            if( pRet )
            {
                // calculate new position
                // JP 24.03.97: also go via page links
                //              chaos::anchor should not lie in the shifted area
                pRet->DelFrms();

                const SwFrm* pAnch = ::FindAnchor( GetLayout(), aPt );
                SwPosition aPos( *static_cast<const SwContentFrm*>(pAnch)->GetNode() );
                if ( FLY_AS_CHAR == eRndId )
                {
                    aPos.nContent.Assign( const_cast<SwContentFrm*>(static_cast<const SwContentFrm*>(pAnch))->GetNode(), 0 );
                }
                pOldAnchor->SetAnchor( &aPos );

                // shifting of table selection is not Undo-capable. therefore
                // changing the anchors should not be recorded
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

                const_cast<SfxItemSet&>(rSet).Put( *pOldAnchor );

                if( bHOriChgd )
                    const_cast<SfxItemSet&>(rSet).Put( aOldH );
                if( bVOriChgd )
                    const_cast<SfxItemSet&>(rSet).Put( aOldV );

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
        pRet = GetDoc()->MakeFlySection( eRndId, &rPos, &rSet, pParent, true );

    if( pRet )
    {
        SwFlyFrm* pFrm = pRet->GetFrm( &aPt );
        if( pFrm )
            SelectFlyFrm( *pFrm, true );
        else
        {
            GetLayout()->SetAssertFlyPages();
            pRet = 0;
        }
    }
    EndAllActionAndCall();

    return pRet;
}

void SwFEShell::Insert( const OUString& rGrfName, const OUString& rFltName,
                        const Graphic* pGraphic,
                        const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet,
                        SwFrameFormat* pFrameFormat )
{
    SwFlyFrameFormat* pFormat = 0;
    SET_CURR_SHELL( this );
    StartAllAction();
    SwShellCrsr *pStartCursor = dynamic_cast<SwShellCrsr*>(this->GetSwCrsr());
    SwShellCrsr *pCursor = pStartCursor;
    do
    {
        if (!pCursor)
            break;

        // Has the anchor not been set or been set incompletely?
        if( pFlyAttrSet )
        {
            const SfxPoolItem* pItem;
            if( SfxItemState::SET == pFlyAttrSet->GetItemState( RES_ANCHOR, false,
                    &pItem ) )
            {
                SwFormatAnchor* pAnchor = const_cast<SwFormatAnchor*>(static_cast<const SwFormatAnchor*>(pItem));
                switch( pAnchor->GetAnchorId())
                {
                case FLY_AT_PARA:
                case FLY_AT_CHAR: // LAYER_IMPL
                case FLY_AS_CHAR:
                    if( !pAnchor->GetContentAnchor() )
                    {
                        pAnchor->SetAnchor( pCursor->GetPoint() );
                    }
                    break;
                case FLY_AT_FLY:
                    if( !pAnchor->GetContentAnchor() )
                    {
                        lcl_SetNewFlyPos( pCursor->GetNode(),
                                *pAnchor, GetCrsrDocPos() );
                    }
                    break;
                case FLY_AT_PAGE:
                    if( !pAnchor->GetPageNum() )
                    {
                        pAnchor->SetPageNum( pCursor->GetPageNum(
                                true, &pCursor->GetPtPos() ) );
                    }
                    break;
                default :
                    break;
                }
            }
        }
        pFormat = GetDoc()->getIDocumentContentOperations().Insert(*pCursor, rGrfName,
                                rFltName, pGraphic,
                                pFlyAttrSet,
                                pGrfAttrSet, pFrameFormat );
        OSL_ENSURE( pFormat, "Doc->getIDocumentContentOperations().Insert(notxt) failed." );

        pCursor = dynamic_cast<SwShellCrsr*>(pCursor->GetNext());
    } while( pCursor != pStartCursor );

    EndAllAction();

    if( pFormat )
    {
        const Point aPt( GetCrsrDocPos() );
        SwFlyFrm* pFrm = pFormat->GetFrm( &aPt );

        if( pFrm )
        {
            // fdo#36681: Invalidate the content and layout to refresh
            // the picture anchoring properly
            SwPageFrm* pPageFrm = pFrm->FindPageFrmOfAnchor();
            pPageFrm->InvalidateFlyLayout();
            pPageFrm->InvalidateContent();

            SelectFlyFrm( *pFrm, true );
        }
        else
            GetLayout()->SetAssertFlyPages();
    }
}

SwFlyFrameFormat* SwFEShell::InsertObject( const svt::EmbeddedObjectRef&  xObj,
                        const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet,
                        SwFrameFormat* pFrameFormat )
{
    SwFlyFrameFormat* pFormat = 0;
    SET_CURR_SHELL( this );
    StartAllAction();
        for(SwPaM& rPaM : GetCrsr()->GetRingContainer())
        {
            pFormat = GetDoc()->getIDocumentContentOperations().Insert(rPaM, xObj,
                                    pFlyAttrSet, pGrfAttrSet, pFrameFormat );
            OSL_ENSURE( pFormat, "Doc->getIDocumentContentOperations().Insert(notxt) failed." );

        }
    EndAllAction();

    if( pFormat )
    {
        const Point aPt( GetCrsrDocPos() );
        SwFlyFrm* pFrm = pFormat->GetFrm( &aPt );

        if( pFrm )
            SelectFlyFrm( *pFrm, true );
        else
            GetLayout()->SetAssertFlyPages();
    }

    return pFormat;
}

void SwFEShell::InsertDrawObj( SdrObject& rDrawObj,
                               const Point& rInsertPosition )
{
    SET_CURR_SHELL( this );

    SfxItemSet rFlyAttrSet( GetDoc()->GetAttrPool(), aFrameFormatSetRange );
    rFlyAttrSet.Put( SwFormatAnchor( FLY_AT_PARA ));
    // #i89920#
    rFlyAttrSet.Put( SwFormatSurround( SURROUND_THROUGHT ) );
    rDrawObj.SetLayer( getIDocumentDrawModelAccess().GetHeavenId() );

    // find anchor position
    SwPaM aPam( mpDoc->GetNodes() );
    {
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        Point aTmpPt( rInsertPosition );
        GetLayout()->GetCrsrOfst( aPam.GetPoint(), aTmpPt, &aState );
        const SwFrm* pFrm = aPam.GetContentNode()->getLayoutFrm( GetLayout(), 0, 0, false );
        const Point aRelPos( rInsertPosition.X() - pFrm->Frm().Left(),
                             rInsertPosition.Y() - pFrm->Frm().Top() );
        rDrawObj.SetRelativePos( aRelPos );
        ::lcl_FindAnchorPos( *GetDoc(), rInsertPosition, *pFrm, rFlyAttrSet );
    }
    // insert drawing object into the document creating a new <SwDrawFrameFormat> instance
    SwDrawFrameFormat* pFormat = GetDoc()->getIDocumentContentOperations().InsertDrawObj( aPam, rDrawObj, rFlyAttrSet );

    // move object to visible layer
    SwContact* pContact = static_cast<SwContact*>(rDrawObj.GetUserCall());
    if ( pContact )
    {
        pContact->MoveObjToVisibleLayer( &rDrawObj );
    }

    if ( pFormat )
    {
        // select drawing object
        Imp()->GetDrawView()->MarkObj( &rDrawObj, Imp()->GetPageView() );
    }
    else
    {
        GetLayout()->SetAssertFlyPages();
    }
}

void SwFEShell::GetPageObjs( std::vector<SwFrameFormat*>& rFillArr )
{
    rFillArr.clear();

    for( auto pFormat : *mpDoc->GetSpzFrameFormats() )
    {
        if (FLY_AT_PAGE == pFormat->GetAnchor().GetAnchorId())
        {
            rFillArr.push_back( pFormat );
        }
    }
}

void SwFEShell::SetPageObjsNewPage( std::vector<SwFrameFormat*>& rFillArr, int nOffset )
{
    if( rFillArr.empty() || !nOffset )
        return;

    StartAllAction();
    StartUndo();

    long nNewPage;
    SwRootFrm* pTmpRootFrm = GetLayout();
    sal_uInt16 nMaxPage = pTmpRootFrm->GetPageNum();
    bool bTmpAssert = false;
    for( auto pFormat : rFillArr )
    {
        if( mpDoc->GetSpzFrameFormats()->Contains( pFormat ))
        {
            // FlyFormat is still valid, therefore process

            SwFormatAnchor aNewAnchor( pFormat->GetAnchor() );
            if ((FLY_AT_PAGE != aNewAnchor.GetAnchorId()) ||
                0 >= ( nNewPage = aNewAnchor.GetPageNum() + nOffset ) )
                // chaos::Anchor has been changed or invalid page number,
                // therefore: do not change!
                continue;

            if( sal_uInt16(nNewPage) > nMaxPage )
            {
                if ( RES_DRAWFRMFMT == pFormat->Which() )
                {
                    SwContact *pCon = pFormat->FindContactObj();
                    if( pCon )
                        static_cast<SwDrawContact*>(pCon)->DisconnectFromLayout();
                }
                else
                    pFormat->DelFrms();
                bTmpAssert = true;
            }
            aNewAnchor.SetPageNum( sal_uInt16(nNewPage) );
            mpDoc->SetAttr( aNewAnchor, *pFormat );
        }
    }

    if( bTmpAssert )
        pTmpRootFrm->SetAssertFlyPages();

    EndUndo();
    EndAllAction();
}

// All attributes in the "baskets" will be filled with the attributes of the
// current FlyFrms. Attributes which cannot be filled due to being at the
// wrong place or which are ambiguous (multiple selections) will be removed.
bool SwFEShell::GetFlyFrmAttr( SfxItemSet &rSet ) const
{
    SwFlyFrm *pFly = GetSelectedOrCurrFlyFrm();
    if (!pFly)
    {
        OSL_ENSURE( false, "GetFlyFrmAttr, no Fly selected." );
        return false;
    }

    SET_CURR_SHELL( const_cast<SwViewShell*>(static_cast<SwViewShell const *>(this)) );

    if( !rSet.Set( pFly->GetFormat()->GetAttrSet() ) )
        return false;

    // now examine all attributes. Remove forbidden attributes, then
    // get all remaining attributes and enter them
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rSet.GetItemState( RES_ANCHOR, false, &pItem ) )
    {
        const SwFormatAnchor* pAnchor = static_cast<const SwFormatAnchor*>(pItem);
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
    rSet.SetParent( pFly->GetFormat()->GetAttrSet().GetParent() );
    // attributes must be removed
    rSet.ClearItem( RES_FILL_ORDER );
    rSet.ClearItem( RES_CNTNT );
    //MA: remove first (Template by example etc.)
    rSet.ClearItem( RES_CHAIN );
    return true;
}

// Attributes of the current fly will change.
bool SwFEShell::SetFlyFrmAttr( SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );
    bool bRet = false;

    if( rSet.Count() )
    {
        SwFlyFrm *pFly = GetSelectedOrCurrFlyFrm();
        OSL_ENSURE( pFly, "SetFlyFrmAttr, no Fly selected." );
        if( pFly )
        {
            StartAllAction();
            const Point aPt( pFly->Frm().Pos() );

            if( SfxItemState::SET == rSet.GetItemState( RES_ANCHOR, false ))
                sw_ChkAndSetNewAnchor( *pFly, rSet );
            SwFlyFrameFormat* pFlyFormat = pFly->GetFormat();

            if( GetDoc()->SetFlyFrmAttr( *pFlyFormat, rSet ))
            {
                bRet = true;
                SwFlyFrm* pFrm = pFlyFormat->GetFrm( &aPt );
                if( pFrm )
                    SelectFlyFrm( *pFrm, true );
                else
                    GetLayout()->SetAssertFlyPages();
            }

            EndAllActionAndCall();
        }
    }
    return bRet;
}

SfxItemSet SwFEShell::makeItemSetFromFormatAnchor(SfxItemPool& rPool, const SwFormatAnchor &rAnchor)
{
    // The set also includes VERT/HORI_ORIENT, because the align
    // shall be changed in FEShell::SetFlyFrmAttr/SetFlyFrmAnchor,
    // possibly as a result of the anchor change.
    SfxItemSet aSet(rPool, RES_VERT_ORIENT, RES_ANCHOR);
    aSet.Put(rAnchor);
    return aSet;
}

bool SwFEShell::SetDrawingAttr( SfxItemSet& rSet )
{
    bool bRet = false;
    SET_CURR_SHELL( this );
    if ( !rSet.Count() ||
            !Imp()->HasDrawView() )
        return bRet;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if ( rMrkList.GetMarkCount() != 1 )
        return bRet;

    StartUndo();
    SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
    SwFrameFormat *pFormat = FindFrameFormat( pObj );
    StartAllAction();
    if( SfxItemState::SET == rSet.GetItemState( RES_ANCHOR, false ))
    {
        RndStdIds nNew = static_cast<const SwFormatAnchor&>(rSet.Get( RES_ANCHOR )).GetAnchorId();
        if ( nNew != pFormat->GetAnchor().GetAnchorId() )
        {
            ChgAnchor( nNew );
            // #i26791# - clear anchor attribute in item set,
            // because method <ChgAnchor(..)> takes care of it.
            rSet.ClearItem( RES_ANCHOR );
        }
    }

    if( GetDoc()->SetFlyFrmAttr( *pFormat, rSet ))
    {
        bRet = true;
        Point aTmp;
        SelectObj( aTmp, 0, pObj );
    }
    EndAllActionAndCall();
    EndUndo();
    return bRet;
}

// Reset requested attributes or the ones contained in the set.
bool SwFEShell::ResetFlyFrmAttr( sal_uInt16 nWhich, const SfxItemSet* pSet )
{
    bool bRet = false;

    if( RES_ANCHOR != nWhich && RES_CHAIN != nWhich && RES_CNTNT != nWhich )
    {
        SET_CURR_SHELL( this );

        SwFlyFrm *pFly = GetSelectedOrCurrFlyFrm();
        OSL_ENSURE( pFly, "SetFlyFrmAttr, no Fly selected." );
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
                        pFly->GetFormat()->ResetFormatAttr( nWhich );
                    pItem = aIter.NextItem();
                }
            }
            else
                pFly->GetFormat()->ResetFormatAttr( nWhich );

            bRet = true;
            EndAllActionAndCall();
            GetDoc()->getIDocumentState().SetModified();
        }
    }
    return bRet;
}

// Returns frame-format if frame, otherwise 0
SwFrameFormat* SwFEShell::GetSelectedFrameFormat() const
{
    SwFrameFormat* pRet = 0;
    SwLayoutFrm *pFly = GetSelectedFlyFrm();
    if( pFly && ( pRet = static_cast<SwFrameFormat*>(pFly->GetFormat()->DerivedFrom()) ) ==
                                            GetDoc()->GetDfltFrameFormat() )
        pRet = 0;
    return pRet;
}

void SwFEShell::SetFrameFormat( SwFrameFormat *pNewFormat, bool bKeepOrient, Point* pDocPos )
{
    SwFlyFrm *pFly = 0;
    if(pDocPos)
    {
        const SwFrameFormat* pFormat = GetFormatFromObj( *pDocPos );

        if (const SwFlyFrameFormat* pFlyFormat = dynamic_cast<const SwFlyFrameFormat*>(pFormat))
            pFly = pFlyFormat->GetFrm();
    }
    else
        pFly = GetSelectedFlyFrm();
    OSL_ENSURE( pFly, "SetFrameFormat: no frame" );
    if( pFly )
    {
        StartAllAction();
        SET_CURR_SHELL( this );

        SwFlyFrameFormat* pFlyFormat = pFly->GetFormat();
        const Point aPt( pFly->Frm().Pos() );

        SfxItemSet* pSet = 0;
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pNewFormat->GetItemState( RES_ANCHOR, false, &pItem ))
        {
            pSet = new SfxItemSet( GetDoc()->GetAttrPool(), aFrameFormatSetRange );
            pSet->Put( *pItem );
            if( !sw_ChkAndSetNewAnchor( *pFly, *pSet ))
                delete pSet, pSet = 0;
        }

        if( GetDoc()->SetFrameFormatToFly( *pFlyFormat, *pNewFormat, pSet, bKeepOrient ))
        {
            SwFlyFrm* pFrm = pFlyFormat->GetFrm( &aPt );
            if( pFrm )
                SelectFlyFrm( *pFrm, true );
            else
                GetLayout()->SetAssertFlyPages();
        }
        delete pSet;

        EndAllActionAndCall();
    }
}

const SwFrameFormat* SwFEShell::GetFlyFrameFormat() const
{
    const SwFlyFrm* pFly = GetSelectedOrCurrFlyFrm();
    if (pFly)
        return pFly->GetFormat();
    return 0;
}

SwFrameFormat* SwFEShell::GetFlyFrameFormat()
{
    SwFlyFrm* pFly = GetSelectedOrCurrFlyFrm();
    if (pFly)
        return pFly->GetFormat();
    return 0;
}

SwRect SwFEShell::GetFlyRect() const
{
    SwFlyFrm *pFly = GetCurrFlyFrm(false);
    if (!pFly)
    {
        SwRect aRect;
        return aRect;
    }
    else
        return pFly->Frm();
}

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
        CallChgLnk();   // call AttrChangeNotify on the UI-side.
    }
}

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

    bool bPosProt = pFly->GetFormat()->GetProtect().IsPosProtected();
    bool bSizeProt = pFly->GetFormat()->GetProtect().IsSizeProtected();

    StartAllAction();

    // MA we do not allow to clip the Fly, as the OLE server can
    // request various wishes. Clipping is done via the formatting.
    // Correct display is done by scaling.
    // Scaling is done by SwNoTextFrm::Format by calling
    // SwWrtShell::CalcAndSetScale()
    if ( rRect.SSize() != pFly->Prt().SSize() && !bSizeProt )
    {
         Size aSz( rRect.SSize() );

        //JP 28.02.2001: Task 74707 - ask for fly in fly with automatic size

        const SwFrm* pAnchor;
        const SwTextNode* pTNd;
        const SwpHints* pHts;
        const SwFormatFrmSize& rFrmSz = pFly->GetFormat()->GetFrmSize();
        if (m_bCheckForOLEInCaption &&
            0 != rFrmSz.GetWidthPercent() &&
            0 != (pAnchor = pFly->GetAnchorFrm()) &&
            pAnchor->IsTextFrm() &&
            !pAnchor->GetNext() && !pAnchor->GetPrev() &&
            pAnchor->GetUpper()->IsFlyFrm() &&
            0 != ( pTNd = static_cast<const SwTextFrm*>(pAnchor)->GetNode()->GetTextNode()) &&
            0 != ( pHts = pTNd->GetpSwpHints() ))
        {
            // search for a sequence field:
            const size_t nEnd = pHts->Count();
            for( size_t n = 0; n < nEnd; ++n )
            {
                const SfxPoolItem* pItem = &pHts->Get(n)->GetAttr();
                if( RES_TXTATR_FIELD == pItem->Which()
                    && TYP_SEQFLD == static_cast<const SwFormatField*>(pItem)->GetField()->GetTypeId() )
                {
                    // sequence field found
                    SwFlyFrm* pChgFly = const_cast<SwFlyFrm*>(static_cast<const SwFlyFrm*>(pAnchor->GetUpper()));
                    // calculate the changed size:
                    // width must change, height can change
                    Size aNewSz( aSz.Width() + pChgFly->Frm().Width() -
                                   pFly->Prt().Width(), aSz.Height() );

                    SwFrameFormat *pFormat = pChgFly->GetFormat();
                    SwFormatFrmSize aFrmSz( pFormat->GetFrmSize() );
                    aFrmSz.SetWidth( aNewSz.Width() );
                    if( ATT_MIN_SIZE != aFrmSz.GetHeightSizeType() )
                    {
                        aNewSz.Height() += pChgFly->Frm().Height() -
                                               pFly->Prt().Height();
                        if( std::abs( aNewSz.Height() - pChgFly->Frm().Height()) > 1 )
                            aFrmSz.SetHeight( aNewSz.Height() );
                    }
                    // via Doc for the Undo!
                    pFormat->GetDoc()->SetAttr( aFrmSz, *pFormat );
                    break;
                }
            }
        }

        // set the new Size at the fly themself
        if ( pFly->Prt().Height() > 0 && pFly->Prt().Width() > 0 )
        {
            aSz.Width() += pFly->Frm().Width() - pFly->Prt().Width();
            aSz.Height()+= pFly->Frm().Height()- pFly->Prt().Height();
        }
        aResult = pFly->ChgSize( aSz );

        // if the object changes, the contour is outside the object
        OSL_ENSURE( pFly->Lower()->IsNoTextFrm(), "Request without NoText" );
        SwNoTextNode *pNd = static_cast<SwContentFrm*>(pFly->Lower())->GetNode()->GetNoTextNode();
        OSL_ENSURE( pNd, "Request without Node" );
        pNd->SetContour( 0 );
        ClrContourCache();
    }

    // if only the size is to be adjusted, a position is transported with
    // allocated values
    Point aPt( pFly->Prt().Pos() );
    aPt += pFly->Frm().Pos();
    if ( rRect.Top() != LONG_MIN && rRect.Pos() != aPt && !bPosProt )
    {
        aPt = rRect.Pos();
        aPt.setX(aPt.getX() - pFly->Prt().Left());
        aPt.setY(aPt.getY() - pFly->Prt().Top());

        // in case of paragraph-bound Flys, starting from the new position,
        // a new anchor is to be set. The anchor and the new RelPos are
        // calculated by the Fly and set
        if( pFly->IsFlyAtCntFrm() )
            static_cast<SwFlyAtCntFrm*>(pFly)->SetAbsPos( aPt );
        else
        {
            const SwFrameFormat *pFormat = pFly->GetFormat();
            const SwFormatVertOrient &rVert = pFormat->GetVertOrient();
            const SwFormatHoriOrient &rHori = pFormat->GetHoriOrient();
            const long lXDiff = aPt.getX() - pFly->Frm().Left();
            const long lYDiff = aPt.getY() - pFly->Frm().Top();
            const Point aTmp( rHori.GetPos() + lXDiff,
                              rVert.GetPos() + lYDiff );
            pFly->ChgRelPos( aTmp );
        }
    }

    SwFlyFrameFormat *pFlyFrameFormat = pFly->GetFormat();
    OSL_ENSURE( pFlyFrameFormat, "fly frame format missing!" );
    if ( pFlyFrameFormat )
        pFlyFrameFormat->SetLastFlyFrmPrtRectPos( pFly->Prt().Pos() ); //stores the value of last Prt rect

    EndAllAction();

    return aResult;
}

SwFrameFormat* SwFEShell::WizardGetFly()
{
    // do not search the Fly via the layout. Now we can delete a frame
    // without a valid layout. ( e.g. for the wizards )
    SwFrameFormats& rSpzArr = *mpDoc->GetSpzFrameFormats();
    if( !rSpzArr.empty() )
    {
        SwNodeIndex& rCrsrNd = GetCrsr()->GetPoint()->nNode;
        if( rCrsrNd.GetIndex() > mpDoc->GetNodes().GetEndOfExtras().GetIndex() )
            // Cursor is in the body area!
            return 0;

        for( auto pFormat : rSpzArr )
        {
            const SwNodeIndex* pIdx = pFormat->GetContent( false ).GetContentIdx();
            SwStartNode* pSttNd;
            if( pIdx &&
                0 != ( pSttNd = pIdx->GetNode().GetStartNode() ) &&
                pSttNd->GetIndex() < rCrsrNd.GetIndex() &&
                rCrsrNd.GetIndex() < pSttNd->EndOfSectionIndex() )
            {
                // found: return immediately
                return pFormat;
            }
        }
    }
    return 0;
}

void SwFEShell::SetFlyName( const OUString& rName )
{
    SwLayoutFrm *pFly = GetSelectedFlyFrm();
    if( pFly )
        GetDoc()->SetFlyName( *static_cast<SwFlyFrameFormat*>(pFly->GetFormat()), rName );
    else {
        OSL_ENSURE( false, "no FlyFrame selected" );
    }
}

OUString SwFEShell::GetFlyName() const
{
    SwLayoutFrm *pFly = GetSelectedFlyFrm();
    if( pFly )
        return pFly->GetFormat()->GetName();

    OSL_ENSURE( false, "no FlyFrame selected" );
    return OUString();
}

const uno::Reference < embed::XEmbeddedObject > SwFEShell::GetOleRef() const
{
    uno::Reference < embed::XEmbeddedObject > xObj;
    SwFlyFrm * pFly = GetSelectedFlyFrm();
    if (pFly && pFly->Lower() && pFly->Lower()->IsNoTextFrm())
    {
        SwOLENode *pNd = static_cast<SwNoTextFrm*>(pFly->Lower())->GetNode()->GetOLENode();
        if (pNd)
            xObj = pNd->GetOLEObj().GetOleRef();
    }
    return xObj;
}

OUString SwFEShell::GetUniqueGrfName() const
{
    return GetDoc()->GetUniqueGrfName();
}

const SwFrameFormat* SwFEShell::IsURLGrfAtPos( const Point& rPt, OUString* pURL,
                                        OUString *pTargetFrameName,
                                        OUString *pDescription ) const
{
    if( !Imp()->HasDrawView() )
        return 0;

    SdrObject* pObj;
    SdrPageView* pPV;
    const SwFrameFormat* pRet = 0;
    SwDrawView *pDView = const_cast<SwDrawView*>(Imp()->GetDrawView());

    const auto nOld = pDView->GetHitTolerancePixel();
    pDView->SetHitTolerancePixel( 2 );

    SwVirtFlyDrawObj* pFlyObj(nullptr);
    if (pDView->PickObj(rPt, pDView->getHitTolLog(), pObj, pPV, SdrSearchOptions::PICKMACRO))
    {
        pFlyObj = dynamic_cast<SwVirtFlyDrawObj*>(pObj);
    }
    if (pFlyObj)
    {
        SwFlyFrm *pFly = pFlyObj->GetFlyFrm();
        const SwFormatURL &rURL = pFly->GetFormat()->GetURL();
        if( !rURL.GetURL().isEmpty() || rURL.GetMap() )
        {
            bool bSetTargetFrameName = pTargetFrameName != 0;
            bool bSetDescription = pDescription != 0;
            if ( rURL.GetMap() )
            {
                IMapObject *pObject = pFly->GetFormat()->GetIMapObject( rPt, pFly );
                if ( pObject && !pObject->GetURL().isEmpty() )
                {
                    if( pURL )
                        *pURL = pObject->GetURL();
                    if ( bSetTargetFrameName && !pObject->GetTarget().isEmpty() )
                    {
                        bSetTargetFrameName = false;
                        *pTargetFrameName = pObject->GetTarget();
                    }
                    if ( bSetDescription )
                    {
                        bSetDescription = false;
                        *pDescription = pObject->GetAltText();
                    }
                    pRet = pFly->GetFormat();
                }
            }
            else
            {
                if( pURL )
                {
                    *pURL = rURL.GetURL();
                    if( rURL.IsServerMap() )
                    {
                       // append the relative pixel position !!
                        Point aPt( rPt );
                        aPt -= pFly->Frm().Pos();
                        // without MapMode-Offset, without Offset, o ... !!!!!
                        aPt = GetOut()->LogicToPixel(
                                aPt, MapMode( MAP_TWIP ) );
                        ((( *pURL += "?" ) += OUString::number( aPt.getX() ))
                                  += "," ) += OUString::number(aPt.getY() );
                    }
                }
                pRet = pFly->GetFormat();
            }
            if ( bSetTargetFrameName )
                *pTargetFrameName = rURL.GetTargetFrameName();
            if ( bSetDescription )
                *pDescription = pFly->GetFormat()->GetName();
        }
    }
    pDView->SetHitTolerancePixel( nOld );
    return pRet;
}

const Graphic *SwFEShell::GetGrfAtPos( const Point &rPt,
                                       OUString &rName, bool &rbLink ) const
{
    if( !Imp()->HasDrawView() )
        return 0;

    SdrObject* pObj;
    SdrPageView* pPV;
    SwDrawView *pDView = const_cast<SwDrawView*>(Imp()->GetDrawView());

    SwVirtFlyDrawObj* pFlyObj(nullptr);
    if (pDView->PickObj(rPt, pDView->getHitTolLog(), pObj, pPV))
    {
        pFlyObj = dynamic_cast<SwVirtFlyDrawObj*>(pObj);
    }
    if (pFlyObj)
    {
        SwFlyFrm *pFly = pFlyObj->GetFlyFrm();
        if ( pFly->Lower() && pFly->Lower()->IsNoTextFrm() )
        {
            SwGrfNode *pNd = static_cast<SwContentFrm*>(pFly->Lower())->GetNode()->GetGrfNode();
            if ( pNd )
            {
                if ( pNd->IsGrfLink() )
                {
                    // halfway ready graphic?
                    ::sfx2::SvLinkSource* pLnkObj = pNd->GetLink()->GetObj();
                    if( pLnkObj && pLnkObj->IsPending() )
                        return 0;
                    rbLink = true;
                }

                pNd->GetFileFilterNms( &rName, 0 );
                if ( rName.isEmpty() )
                    rName = pFly->GetFormat()->GetName();
                return &pNd->GetGrf(true);
            }
        }
    }
    return 0;
}

const SwFrameFormat* SwFEShell::GetFormatFromObj( const Point& rPt, SwRect** pRectToFill ) const
{
    SwFrameFormat* pRet = 0;

    if( Imp()->HasDrawView() )
    {
        SdrObject* pObj;
        SdrPageView* pPView;

        SwDrawView *pDView = const_cast<SwDrawView*>(Imp()->GetDrawView());

        const auto nOld = pDView->GetHitTolerancePixel();
        // tolerance for Drawing-SS
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        if( pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPView, SdrSearchOptions::PICKMARKABLE ) )
        {
           // first check it:
            if (SwVirtFlyDrawObj* pFlyObj = dynamic_cast<SwVirtFlyDrawObj*>(pObj))
                pRet = pFlyObj->GetFormat();
            else if ( pObj->GetUserCall() ) //not for group objects
                pRet = static_cast<SwDrawContact*>(pObj->GetUserCall())->GetFormat();
            if(pRet && pRectToFill)
                **pRectToFill = pObj->GetCurrentBoundRect();
        }
        pDView->SetHitTolerancePixel( nOld );
    }
    return pRet;
}

// returns a format too, if the point is over the text of any fly
const SwFrameFormat* SwFEShell::GetFormatFromAnyObj( const Point& rPt ) const
{
    const SwFrameFormat* pRet = GetFormatFromObj( rPt );
    if( !pRet || RES_FLYFRMFMT == pRet->Which() )
    {
        SwPosition aPos( *GetCrsr()->GetPoint() );
        Point aPt( rPt );
        GetLayout()->GetCrsrOfst( &aPos, aPt );
        SwContentNode *pNd = aPos.nNode.GetNode().GetContentNode();
        SwFrm* pFrm = pNd->getLayoutFrm( GetLayout(), &rPt, 0, false )->FindFlyFrm();
        pRet = pFrm ? static_cast<SwLayoutFrm*>(pFrm)->GetFormat() : 0;
    }
    return pRet;
}

ObjCntType SwFEShell::GetObjCntType( const SdrObject& rObj ) const
{
    ObjCntType eType = OBJCNT_NONE;

    // investigate 'master' drawing object, if method
    // is called for a 'virtual' drawing object.
    const SdrObject* pInvestigatedObj;
    if (const SwDrawVirtObj* pDrawVirtObj = dynamic_cast<const SwDrawVirtObj*>( &rObj))
    {
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
                static_cast<const SdrUnoObj&>(*pInvestigatedObj).GetUnoControlModel();
        if( xModel.is() )
        {
            uno::Any aVal;
            OUString sName("ButtonType");
            uno::Reference< beans::XPropertySet >  xSet(xModel, uno::UNO_QUERY);

            uno::Reference< beans::XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
            if(xInfo->hasPropertyByName( sName ))
            {
                beans::Property xProperty = xInfo->getPropertyByName( sName );
                aVal = xSet->getPropertyValue( sName );
                if( aVal.getValue() && form::FormButtonType_URL == *static_cast<form::FormButtonType const *>(aVal.getValue()) )
                    eType = OBJCNT_URLBUTTON;
            }
        }
    }
    else if (const SwVirtFlyDrawObj *pFlyObj = dynamic_cast<const SwVirtFlyDrawObj*>(pInvestigatedObj))
    {
        const SwFlyFrm *pFly = pFlyObj->GetFlyFrm();
        if ( pFly->Lower() && pFly->Lower()->IsNoTextFrm() )
        {
            if ( static_cast<const SwContentFrm*>(pFly->Lower())->GetNode()->GetGrfNode() )
                eType = OBJCNT_GRF;
            else
                eType = OBJCNT_OLE;
        }
        else
            eType = OBJCNT_FLY;
    }
    else if ( dynamic_cast<const SdrObjGroup*>( pInvestigatedObj) != nullptr )
    {
        SwDrawContact* pDrawContact( dynamic_cast<SwDrawContact*>(GetUserCall( pInvestigatedObj ) ) );
        if ( !pDrawContact )
        {
            OSL_FAIL( "<SwFEShell::GetObjCntType(..)> - missing draw contact object" );
            eType = OBJCNT_NONE;
        }
        else
        {
            SwFrameFormat* pFrameFormat( pDrawContact->GetFormat() );
            if ( !pFrameFormat )
            {
                OSL_FAIL( "<SwFEShell::GetObjCntType(..)> - missing frame format" );
                eType = OBJCNT_NONE;
            }
            else if ( FLY_AS_CHAR != pFrameFormat->GetAnchor().GetAnchorId() )
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

        SwDrawView *pDView = const_cast<SwDrawView*>(Imp()->GetDrawView());

        const auto nOld = pDView->GetHitTolerancePixel();
        // tolerance for Drawing-SS
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        if( pDView->PickObj( rPt, pDView->getHitTolLog(), pObj, pPView, SdrSearchOptions::PICKMARKABLE ) )
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
        for( size_t i = 0, nE = rMrkList.GetMarkCount(); i < nE; ++i )
        {
            SdrObject* pObj = rMrkList.GetMark( i )->GetMarkedSdrObj();
            if( !pObj )
                continue;
            ObjCntType eTmp = GetObjCntType( *pObj );
            if( !i )
            {
                eType = eTmp;
                if( ppObj ) *ppObj = pObj;
            }
            else if( eTmp != eType )
            {
                eType = OBJCNT_DONTCARE;
                // once DontCare, always DontCare!
                break;
            }
        }
    }
    return eType;
}

bool SwFEShell::ReplaceSdrObj( const OUString& rGrfName, const OUString& rFltName,
                                const Graphic* pGrf )
{
    SET_CURR_SHELL( this );

    bool bRet = false;
    const SdrMarkList *pMrkList;
    if( Imp()->HasDrawView() &&  1 ==
        ( pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList())->GetMarkCount() )
    {
        SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
        SwFrameFormat *pFormat = FindFrameFormat( pObj );

        // store attributes, then set the graphic
        SfxItemSet aFrmSet( mpDoc->GetAttrPool(),
                            pFormat->GetAttrSet().GetRanges() );
        aFrmSet.Set( pFormat->GetAttrSet() );

        // set size and position?
        if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) == nullptr )
        {
            // then let's do it:
            const Rectangle &rBound = pObj->GetSnapRect();
            Point aRelPos( pObj->GetRelativePos() );

            const long nWidth = rBound.Right()  - rBound.Left();
            const long nHeight= rBound.Bottom() - rBound.Top();
            aFrmSet.Put( SwFormatFrmSize( ATT_MIN_SIZE,
                                std::max( nWidth,  long(MINFLY) ),
                                std::max( nHeight, long(MINFLY) )));

            if( SfxItemState::SET != aFrmSet.GetItemState( RES_HORI_ORIENT ))
                aFrmSet.Put( SwFormatHoriOrient( aRelPos.getX(), text::HoriOrientation::NONE, text::RelOrientation::FRAME ));

            if( SfxItemState::SET != aFrmSet.GetItemState( RES_VERT_ORIENT ))
                aFrmSet.Put( SwFormatVertOrient( aRelPos.getY(), text::VertOrientation::NONE, text::RelOrientation::FRAME ));

        }

        pObj->GetOrdNum();

        StartAllAction();
        StartUndo();

        // delete "Sdr-Object", insert the graphic instead
        DelSelectedObj();

        GetDoc()->getIDocumentContentOperations().Insert( *GetCrsr(), rGrfName, rFltName, pGrf, &aFrmSet, NULL, NULL );

        EndUndo();
        EndAllAction();
        bRet = true;
    }
    return bRet;
}

static sal_uInt16 SwFormatGetPageNum(const SwFlyFrameFormat * pFormat)
{
    OSL_ENSURE(pFormat != NULL, "invalid argument");

    SwFlyFrm * pFrm = pFormat->GetFrm();

    sal_uInt16 aResult;

    if (pFrm != NULL)
        aResult = pFrm->GetPhyPageNum();
    else
        aResult = pFormat->GetAnchor().GetPageNum();

    return aResult;
}

#include <fmtcnct.hxx>

void SwFEShell::GetConnectableFrameFormats(SwFrameFormat & rFormat,
                                      const OUString & rReference,
                                      bool bSuccessors,
                                      ::std::vector< OUString > & aPrevPageVec,
                                      ::std::vector< OUString > & aThisPageVec,
                                      ::std::vector< OUString > & aNextPageVec,
                                      ::std::vector< OUString > & aRestVec)
{
    StartAction();

    SwFormatChain rChain = rFormat.GetChain();
    SwFrameFormat * pOldChainNext = static_cast<SwFrameFormat *>(rChain.GetNext());
    SwFrameFormat * pOldChainPrev = static_cast<SwFrameFormat *>(rChain.GetPrev());

    if (pOldChainNext)
        mpDoc->Unchain(rFormat);

    if (pOldChainPrev)
        mpDoc->Unchain(*pOldChainPrev);

    const size_t nCnt = mpDoc->GetFlyCount(FLYCNTTYPE_FRM);

    /* potential successors resp. predecessors */
    ::std::vector< const SwFrameFormat * > aTmpSpzArray;

    mpDoc->FindFlyByName(rReference);

    for (size_t n = 0; n < nCnt; ++n)
    {
        const SwFrameFormat & rFormat1 = *(mpDoc->GetFlyNum(n, FLYCNTTYPE_FRM));

        /*
           pFormat is a potential successor of rFormat if it is chainable after
           rFormat.

           pFormat is a potential predecessor of rFormat if rFormat is chainable
           after pFormat.
        */

        SwChainRet nChainState;

        if (bSuccessors)
            nChainState = mpDoc->Chainable(rFormat, rFormat1);
        else
            nChainState = mpDoc->Chainable(rFormat1, rFormat);

        if (nChainState == SwChainRet::OK)
        {
            aTmpSpzArray.push_back(&rFormat1);

        }

    }

    if  (aTmpSpzArray.size() > 0)
    {
        aPrevPageVec.clear();
        aThisPageVec.clear();
        aNextPageVec.clear();
        aRestVec.clear();

        /* number of page rFormat resides on */
        sal_uInt16 nPageNum = SwFormatGetPageNum(static_cast<SwFlyFrameFormat *>(&rFormat));

        ::std::vector< const SwFrameFormat * >::const_iterator aIt;

        for (aIt = aTmpSpzArray.begin(); aIt != aTmpSpzArray.end(); ++aIt)
        {
            const OUString aString = (*aIt)->GetName();

            /* rFormat is not a valid successor or predecessor of
               itself */
            if (aString != rReference && aString != rFormat.GetName())
            {
                sal_uInt16 nNum1 =
                    SwFormatGetPageNum(static_cast<const SwFlyFrameFormat *>(*aIt));

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
        mpDoc->Chain(rFormat, *pOldChainNext);

    if (pOldChainPrev)
        mpDoc->Chain(*pOldChainPrev, rFormat);

    EndAction();
}

// #i73249#
OUString SwFEShell::GetObjTitle() const
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList *pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        if ( pMrkList->GetMarkCount() == 1 )
        {
            const SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            const SwFrameFormat* pFormat = FindFrameFormat( pObj );
            if ( pFormat->Which() == RES_FLYFRMFMT )
            {
                return static_cast<const SwFlyFrameFormat*>(pFormat)->GetObjTitle();
            }
            return pObj->GetTitle();
        }
    }

    return OUString();
}

void SwFEShell::SetObjTitle( const OUString& rTitle )
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList *pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        if ( pMrkList->GetMarkCount() == 1 )
        {
            SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            SwFrameFormat* pFormat = FindFrameFormat( pObj );
            if ( pFormat->Which() == RES_FLYFRMFMT )
            {
                GetDoc()->SetFlyFrmTitle( dynamic_cast<SwFlyFrameFormat&>(*pFormat),
                                          rTitle );
            }
            else
            {
                pObj->SetTitle( rTitle );
            }
        }
    }
}

OUString SwFEShell::GetObjDescription() const
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList *pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        if ( pMrkList->GetMarkCount() == 1 )
        {
            const SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            const SwFrameFormat* pFormat = FindFrameFormat( pObj );
            if ( pFormat->Which() == RES_FLYFRMFMT )
            {
                return dynamic_cast<const SwFlyFrameFormat&>(*pFormat).GetObjDescription();
            }
            return pObj->GetDescription();
        }
    }

    return OUString();
}

void SwFEShell::SetObjDescription( const OUString& rDescription )
{
    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList *pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
        if ( pMrkList->GetMarkCount() == 1 )
        {
            SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
            SwFrameFormat* pFormat = FindFrameFormat( pObj );
            if ( pFormat->Which() == RES_FLYFRMFMT )
            {
                GetDoc()->SetFlyFrmDescription(dynamic_cast<SwFlyFrameFormat&>(*pFormat),
                                               rDescription);
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
#if OSL_DEBUG_LEVEL > 0
    SvGlobalName aCLSID( xObj->getClassID() );
    const bool bStarMath = ( SotExchange::IsMath( aCLSID ) != 0 );
    OSL_ENSURE( bStarMath, "AlignFormulaToBaseline should only be called for Math objects" );

    if ( !bStarMath )
        return;
#endif

    if (!pFly)
        pFly = FindFlyFrm( xObj );
    OSL_ENSURE( pFly , "No fly frame!" );
    SwFrameFormat * pFrameFormat = pFly ? pFly->GetFormat() : 0;

    // baseline to baseline alignment should only be applied to formulas anchored as char
    if ( pFly && pFrameFormat && FLY_AS_CHAR == pFrameFormat->GetAnchor().GetAnchorId() )
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
                    aBaseline = xSet->getPropertyValue("BaseLine");
                }
                catch ( uno::Exception& )
                {
                    OSL_FAIL( "Baseline could not be retrieved from Starmath!" );
                }
            }
        }

        sal_Int32 nBaseline = ::comphelper::getINT32(aBaseline);
        const MapMode aSourceMapMode( MAP_100TH_MM );
        const MapMode aTargetMapMode( MAP_TWIP );
        nBaseline = OutputDevice::LogicToLogic( nBaseline, aSourceMapMode.GetMapUnit(), aTargetMapMode.GetMapUnit() );

        OSL_ENSURE( nBaseline > 0, "Wrong value of Baseline while retrieving from Starmath!" );
        //nBaseline must be moved by aPrt position
        const SwFlyFrameFormat *pFlyFrameFormat = pFly->GetFormat();
        OSL_ENSURE( pFlyFrameFormat, "fly frame format missing!" );
        if ( pFlyFrameFormat )
            nBaseline += pFlyFrameFormat->GetLastFlyFrmPrtRectPos().Y();

        const SwFormatVertOrient &rVert = pFrameFormat->GetVertOrient();
        SwFormatVertOrient aVert( rVert );
        aVert.SetPos( -nBaseline );
        aVert.SetVertOrient( com::sun::star::text::VertOrientation::NONE );

        pFrameFormat->LockModify();
        pFrameFormat->SetFormatAttr( aVert );
        pFrameFormat->UnlockModify();
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
