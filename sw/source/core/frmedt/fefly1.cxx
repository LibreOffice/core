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
        const SwContentFrame* pCFrame = pCntNd ? pCntNd->getLayoutFrame( pCntNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &rPt, nullptr, false ) : nullptr;
        const SwPageFrame *pPg = pCFrame ? pCFrame->FindPageFrame() : nullptr;

        rAnchor.SetPageNum( pPg ? pPg->GetPhyPageNum() : 1 );
        rAnchor.SetType( FLY_AT_PAGE );
    }
    return bRet;
}

static bool lcl_FindAnchorPos(
    SwDoc& rDoc,
    const Point& rPt,
    const SwFrame& rFrame,
    SfxItemSet& rSet )
{
    bool bRet = true;
    SwFormatAnchor aNewAnch( static_cast<const SwFormatAnchor&>(rSet.Get( RES_ANCHOR )) );
    RndStdIds nNew = aNewAnch.GetAnchorId();
    const SwFrame *pNewAnch;

    //determine new anchor
    Point aTmpPnt( rPt );
    switch( nNew )
    {
    case FLY_AS_CHAR:  // also include this?
    case FLY_AT_PARA:
    case FLY_AT_CHAR: // LAYER_IMPL
        {
            // starting from the upper-left corner of the Fly,
            // search nearest ContentFrame
            const SwFrame* pFrame = rFrame.IsFlyFrame() ? static_cast<const SwFlyFrame&>(rFrame).GetAnchorFrame()
                                                : &rFrame;
            pNewAnch = ::FindAnchor( pFrame, aTmpPnt );
            if( pNewAnch->IsProtected() )
            {
                bRet = false;
                break;
            }

            SwPosition aPos( *static_cast<const SwContentFrame*>(pNewAnch)->GetNode() );
            if ((FLY_AT_CHAR == nNew) || (FLY_AS_CHAR == nNew))
            {
                // textnode should be found, as only in those
                // a content bound frame can be anchored
                SwCursorMoveState aState( MV_SETONLYTEXT );
                aTmpPnt.setX(aTmpPnt.getX() - 1);                   // do not land in the fly!
                if( !pNewAnch->GetCursorOfst( &aPos, aTmpPnt, &aState ) )
                {
                    SwContentNode* pCNd = const_cast<SwContentFrame*>(static_cast<const SwContentFrame*>(pNewAnch))->GetNode();
                    if( pNewAnch->Frame().Bottom() < aTmpPnt.Y() )
                        pCNd->MakeStartIndex( &aPos.nContent );
                    else
                        pCNd->MakeEndIndex( &aPos.nContent );
                }
                else
                {
                    if ( SwCursorShell::PosInsideInputField( aPos ) )
                    {
                        aPos.nContent = SwCursorShell::StartOfInputFieldAtPos( aPos );
                    }
                }
            }
            aNewAnch.SetAnchor( &aPos );
        }
        break;

    case FLY_AT_FLY: // LAYER_IMPL
        {
            // starting from the upper-left corner of the Fly
            // search nearest SwFlyFrame
            SwCursorMoveState aState( MV_SETONLYTEXT );
            SwPosition aPos( rDoc.GetNodes() );
            aTmpPnt.setX(aTmpPnt.getX() - 1);                   // do not land in the fly!
            rDoc.getIDocumentLayoutAccess().GetCurrentLayout()->GetCursorOfst( &aPos, aTmpPnt, &aState );
            pNewAnch = ::FindAnchor(
                aPos.nNode.GetNode().GetContentNode()->getLayoutFrame( rFrame.getRootFrame(), nullptr, nullptr, false ),
                aTmpPnt )->FindFlyFrame();

            if( pNewAnch && &rFrame != pNewAnch && !pNewAnch->IsProtected() )
            {
                aPos.nNode = *static_cast<const SwFlyFrame*>(pNewAnch)->GetFormat()->GetContent().
                                GetContentIdx();
                aNewAnch.SetAnchor( &aPos );
                break;
            }
        }

        aNewAnch.SetType( nNew = FLY_AT_PAGE );
        SAL_FALLTHROUGH;

    case FLY_AT_PAGE:
        pNewAnch = rFrame.FindPageFrame();
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
    const SwFlyFrame& rFly,
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

    return ::lcl_FindAnchorPos( *pDoc, rFly.Frame().Pos(), rFly, rSet );
}

void SwFEShell::SelectFlyFrame( SwFlyFrame& rFrame, bool bNew )
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
        OSL_ENSURE( rFrame.IsFlyFrame(), "SelectFlyFrame will einen Fly" );

       // nothing to be done if the Fly already was selected
        if (GetSelectedFlyFrame() == &rFrame)
            return;

        // assure the anchor is drawn
        if( rFrame.IsFlyInContentFrame() && rFrame.GetAnchorFrame() )
             rFrame.GetAnchorFrame()->SetCompletePaint();

        if( pImpl->GetDrawView()->AreObjectsMarked() )
            pImpl->GetDrawView()->UnmarkAll();

        pImpl->GetDrawView()->MarkObj( rFrame.GetVirtDrawObj(),
                                      pImpl->GetPageView() );
        KillPams();
        ClearMark();
        SelFlyGrabCursor();
    }
}

// Get selected fly
SwFlyFrame* SwFEShell::GetSelectedFlyFrame() const
{
    if ( Imp()->HasDrawView() )
    {
        // A Fly is only accessible if it is selected
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        if( rMrkList.GetMarkCount() != 1 )
            return nullptr;

        SdrObject *pO = rMrkList.GetMark( 0 )->GetMarkedSdrObj();

        SwVirtFlyDrawObj *pFlyObj = dynamic_cast<SwVirtFlyDrawObj*>(pO);

        return pFlyObj ? pFlyObj->GetFlyFrame() : nullptr;
    }
    return nullptr;
}

// Get current fly in which the cursor is positioned
SwFlyFrame* SwFEShell::GetCurrFlyFrame(const bool bCalcFrame) const
{
    SwContentFrame *pContent = GetCurrFrame(bCalcFrame);
    return pContent ? pContent->FindFlyFrame() : nullptr;
}

// Get selected fly, but if none Get current fly in which the cursor is positioned
SwFlyFrame* SwFEShell::GetSelectedOrCurrFlyFrame(const bool bCalcFrame) const
{
    SwFlyFrame *pFly = GetSelectedFlyFrame();
    if (pFly)
        return pFly;
    return GetCurrFlyFrame(bCalcFrame);
}

// Returns non-null pointer, if the current Fly could be anchored to another one (so it is inside)
const SwFrameFormat* SwFEShell::IsFlyInFly()
{
    SET_CURR_SHELL( this );

    if ( !Imp()->HasDrawView() )
        return nullptr;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if ( !rMrkList.GetMarkCount() )
    {
        SwFlyFrame *pFly = GetCurrFlyFrame(false);
        if (!pFly)
            return nullptr;
        return pFly->GetFormat();
    }
    else if ( rMrkList.GetMarkCount() != 1 ||
         !GetUserCall(rMrkList.GetMark( 0 )->GetMarkedSdrObj()) )
        return nullptr;

    SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();

    SwFrameFormat *pFormat = FindFrameFormat( pObj );
    if( pFormat && FLY_AT_FLY == pFormat->GetAnchor().GetAnchorId() )
    {
        const SwFrame* pFly;
        if (SwVirtFlyDrawObj* pFlyObj = dynamic_cast<SwVirtFlyDrawObj *>(pObj))
        {
            pFly = pFlyObj->GetFlyFrame()->GetAnchorFrame();
        }
        else
        {
            pFly = static_cast<SwDrawContact*>(GetUserCall(pObj))->GetAnchorFrame(pObj);
        }

        OSL_ENSURE( pFly, "IsFlyInFly: Where's my anchor?" );
        OSL_ENSURE( pFly->IsFlyFrame(), "IsFlyInFly: Funny anchor!" );
        return static_cast<const SwFlyFrame*>(pFly)->GetFormat();
    }

    Point aTmpPos = pObj->GetCurrentBoundRect().TopLeft();

    SwFrame *pTextFrame;
    {
        SwCursorMoveState aState( MV_SETONLYTEXT );
        SwNodeIndex aSwNodeIndex( GetDoc()->GetNodes() );
        SwPosition aPos( aSwNodeIndex );
        Point aPoint( aTmpPos );
        aPoint.setX(aPoint.getX() - 1);                    //do not land in the fly!!
        GetLayout()->GetCursorOfst( &aPos, aPoint, &aState );
        // determine text frame by left-top-corner of object
        SwContentNode *pNd = aPos.nNode.GetNode().GetContentNode();
        pTextFrame = pNd ? pNd->getLayoutFrame(GetLayout(), &aTmpPos, nullptr, false) : nullptr;
    }
    const SwFrame *pTmp = pTextFrame ? ::FindAnchor(pTextFrame, aTmpPos) : nullptr;
    const SwFlyFrame *pFly = pTmp ? pTmp->FindFlyFrame() : nullptr;
    if( pFly )
        return pFly->GetFormat();
    return nullptr;
}

void SwFEShell::SetFlyPos( const Point& rAbsPos )
{
    SET_CURR_SHELL( this );

    // Determine reference point in document coordinates
    SwFlyFrame *pFly = GetCurrFlyFrame(false);
    if (!pFly)
        return;

    //SwSaveHdl aSaveX( Imp() );

    // Set an anchor starting from the absolute position for paragraph bound Flys
    // Anchor and new RelPos will be calculated and set by the Fly
    if ( pFly->IsFlyAtContentFrame() )
        static_cast<SwFlyAtContentFrame*>(pFly)->SetAbsPos( rAbsPos );
    else
    {
            const SwFrame *pAnch = pFly->GetAnchorFrame();
            Point aOrient( pAnch->Frame().Pos() );

        if ( pFly->IsFlyInContentFrame() )
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

    SwFlyFrame* pFly = nullptr;
    const SwFrame* pFooterOrHeader = nullptr;

    if( bFlyFrame )
    {
        // Calculate reference point in document coordinates
        SwContentFrame *pContent = GetCurrFrame( false );
        if( !pContent )
            return aRet;
        pFly = pContent->FindFlyFrame();
        if ( !pFly )
            return aRet;
        const SwFrame* pOldAnch = pFly->GetAnchorFrame();
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
        SwContentFrame *pContent = GetCurrFrame( false );
        if( !pContent )
            return aRet;
        pFooterOrHeader = pContent->FindFooterOrHeader();
    }

    // Search nearest SwFlyFrame starting from the upper-left corner
    // of the fly
    SwContentFrame *pTextFrame = nullptr;
    {
        SwCursorMoveState aState( MV_SETONLYTEXT );
        SwPosition aPos( GetDoc()->GetNodes().GetEndOfExtras() );
        Point aTmpPnt( rAbsPos );
        GetLayout()->GetCursorOfst( &aPos, aTmpPnt, &aState );
        if (aPos.nNode != GetDoc()->GetNodes().GetEndOfExtras().GetIndex()
            && (nAnchorId != FLY_AT_CHAR || !PosInsideInputField(aPos)))
        {
            SwContentNode* pCNode = aPos.nNode.GetNode().GetContentNode();
            assert(pCNode);
            pTextFrame = pCNode->getLayoutFrame(GetLayout(), nullptr, &aPos, false);
        }
    }
    const SwFrame *pNewAnch = nullptr;
    if( pTextFrame != nullptr )
    {
        if ( FLY_AT_PAGE == nAnchorId )
        {
            pNewAnch = pTextFrame->FindPageFrame();
        }
        else
        {
            pNewAnch = ::FindAnchor( pTextFrame, rAbsPos );

            if( FLY_AT_FLY == nAnchorId ) // LAYER_IMPL
            {
                pNewAnch = pNewAnch->FindFlyFrame();
            }
        }
    }

    if( pNewAnch && !pNewAnch->IsProtected() )
    {
        const SwFlyFrame* pCheck = bFlyFrame ? pNewAnch->FindFlyFrame() : nullptr;
        // If we land inside the frame, make sure
        // that the frame does not land inside its own content
        while( pCheck )
        {
            if( pCheck == pFly )
                break;
            const SwFrame *pTmp = pCheck->GetAnchorFrame();
            pCheck = pTmp ? pTmp->FindFlyFrame() : nullptr;
        }

        // Do not switch from header/footer to another area,
        // do not switch to a header/footer
        if( !pCheck &&
            pFooterOrHeader == pNewAnch->FindFooterOrHeader() )
        {
            aRet = pNewAnch->GetFrameAnchorPos( ::HasWrap( pObj ) );

            if ( bMoveIt || (nAnchorId == FLY_AT_CHAR) )
            {
                SwFormatAnchor aAnch( rFormat.GetAnchor() );
                switch ( nAnchorId )
                {
                    case FLY_AT_PARA:
                    {
                        SwPosition pos = *aAnch.GetContentAnchor();
                        pos.nNode = *pTextFrame->GetNode();
                        pos.nContent.Assign(nullptr,0);
                        aAnch.SetAnchor( &pos );
                        break;
                    }
                    case FLY_AT_PAGE:
                    {
                        aAnch.SetPageNum( static_cast<const SwPageFrame*>(pNewAnch)->
                                          GetPhyPageNum() );
                        break;
                    }

                    case FLY_AT_FLY:
                    {
                        SwPosition aPos( *static_cast<const SwFlyFrame*>(pNewAnch)->GetFormat()->
                                                  GetContent().GetContentIdx() );
                        aAnch.SetAnchor( &aPos );
                        break;
                    }

                    case FLY_AT_CHAR:
                        {
                            SwPosition pos = *aAnch.GetContentAnchor();
                            Point aTmpPnt( rAbsPos );
                            if( pTextFrame->GetCursorOfst( &pos, aTmpPnt ) )
                            {
                                SwRect aTmpRect;
                                pTextFrame->GetCharRect( aTmpRect, pos );
                                aRet = aTmpRect.Pos();
                            }
                            else
                            {
                                pos.nNode = *pTextFrame->GetNode();
                                pos.nContent.Assign(nullptr,0);
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
                        SwHandleAnchorNodeChg* pHandleAnchorNodeChg( nullptr );
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

const SwFrameFormat *SwFEShell::NewFlyFrame( const SfxItemSet& rSet, bool bAnchValid,
                           SwFrameFormat *pParent )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    SwPaM* pCursor = GetCursor();
    const Point aPt( GetCursorDocPos() );

    SwSelBoxes aBoxes;
    bool bMoveContent = true;
    if( IsTableMode() )
    {
        GetTableSel( *this, aBoxes );
        if( !aBoxes.empty() )
        {
            // Cursor should be removed from the removal area.
            // Always put it after/on the table; via the
            // document position they will be set to the old
            // position
            ParkCursor( SwNodeIndex( *aBoxes[0]->GetSttNd() ));

            // #i127787# pCurrentCursor will be deleted in ParkCursor,
            // we better get the current pCurrentCursor instead of working with the
            // deleted one:
            pCursor = GetCursor();
        }
        else
            bMoveContent = false;
    }
    else if( !pCursor->HasMark() && !pCursor->IsMultiSelection() )
        bMoveContent = false;

    const SwPosition& rPos = *pCursor->Start();

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
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_INSLAYFMT, nullptr );
        SwFormatAnchor* pOldAnchor = nullptr;
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

        pRet = GetDoc()->MakeFlyAndMove( *pCursor, rSet, &aBoxes, pParent );

        KillPams();

        if( pOldAnchor )
        {
            if( pRet )
            {
                // calculate new position
                // JP 24.03.97: also go via page links
                //              chaos::anchor should not lie in the shifted area
                pRet->DelFrames();

                const SwFrame* pAnch = ::FindAnchor( GetLayout(), aPt );
                SwPosition aPos( *static_cast<const SwContentFrame*>(pAnch)->GetNode() );
                if ( FLY_AS_CHAR == eRndId )
                {
                    aPos.nContent.Assign( const_cast<SwContentFrame*>(static_cast<const SwContentFrame*>(pAnch))->GetNode(), 0 );
                }
                pOldAnchor->SetAnchor( &aPos );

                // shifting of table selection is not Undo-capable. therefore
                // changing the anchors should not be recorded
                bool const bDoesUndo =
                    GetDoc()->GetIDocumentUndoRedo().DoesUndo();
                SwUndoId nLastUndoId(UNDO_EMPTY);
                if (bDoesUndo &&
                    GetDoc()->GetIDocumentUndoRedo().GetLastUndoInfo(nullptr,
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

                GetDoc()->SetFlyFrameAttr( *pRet, (SfxItemSet&)rSet );
                GetDoc()->GetIDocumentUndoRedo().DoUndo(bDoesUndo);
            }
            delete pOldAnchor;
        }
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_INSLAYFMT, nullptr );
    }
    else
        /* If called from a shell try to propagate an
            existing adjust item from rPos to the content node of the
            new frame. */
        pRet = GetDoc()->MakeFlySection( eRndId, &rPos, &rSet, pParent, true );

    if( pRet )
    {
        SwFlyFrame* pFrame = pRet->GetFrame( &aPt );
        if( pFrame )
            SelectFlyFrame( *pFrame, true );
        else
        {
            GetLayout()->SetAssertFlyPages();
            pRet = nullptr;
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
    SwFlyFrameFormat* pFormat = nullptr;
    SET_CURR_SHELL( this );
    StartAllAction();
    SwShellCursor *pStartCursor = dynamic_cast<SwShellCursor*>(this->GetSwCursor());
    SwShellCursor *pCursor = pStartCursor;
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
                                *pAnchor, GetCursorDocPos() );
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

        pCursor = dynamic_cast<SwShellCursor*>(pCursor->GetNext());
    } while( pCursor != pStartCursor );

    EndAllAction();

    if( pFormat )
    {
        const Point aPt( GetCursorDocPos() );
        SwFlyFrame* pFrame = pFormat->GetFrame( &aPt );

        if( pFrame )
        {
            // fdo#36681: Invalidate the content and layout to refresh
            // the picture anchoring properly
            SwPageFrame* pPageFrame = pFrame->FindPageFrameOfAnchor();
            pPageFrame->InvalidateFlyLayout();
            pPageFrame->InvalidateContent();

            SelectFlyFrame( *pFrame, true );
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
    SwFlyFrameFormat* pFormat = nullptr;
    SET_CURR_SHELL( this );
    StartAllAction();
        for(SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            pFormat = GetDoc()->getIDocumentContentOperations().Insert(rPaM, xObj,
                                    pFlyAttrSet, pGrfAttrSet, pFrameFormat );
            OSL_ENSURE( pFormat, "Doc->getIDocumentContentOperations().Insert(notxt) failed." );

        }
    EndAllAction();

    if( pFormat )
    {
        const Point aPt( GetCursorDocPos() );
        SwFlyFrame* pFrame = pFormat->GetFrame( &aPt );

        if( pFrame )
            SelectFlyFrame( *pFrame, true );
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
        SwCursorMoveState aState( MV_SETONLYTEXT );
        Point aTmpPt( rInsertPosition );
        GetLayout()->GetCursorOfst( aPam.GetPoint(), aTmpPt, &aState );
        const SwFrame* pFrame = aPam.GetContentNode()->getLayoutFrame( GetLayout(), nullptr, nullptr, false );
        const Point aRelPos( rInsertPosition.X() - pFrame->Frame().Left(),
                             rInsertPosition.Y() - pFrame->Frame().Top() );
        rDrawObj.SetRelativePos( aRelPos );
        ::lcl_FindAnchorPos( *GetDoc(), rInsertPosition, *pFrame, rFlyAttrSet );
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
    SwRootFrame* pTmpRootFrame = GetLayout();
    sal_uInt16 nMaxPage = pTmpRootFrame->GetPageNum();
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
                    pFormat->DelFrames();
                bTmpAssert = true;
            }
            aNewAnchor.SetPageNum( sal_uInt16(nNewPage) );
            mpDoc->SetAttr( aNewAnchor, *pFormat );
        }
    }

    if( bTmpAssert )
        pTmpRootFrame->SetAssertFlyPages();

    EndUndo();
    EndAllAction();
}

// All attributes in the "baskets" will be filled with the attributes of the
// current FlyFrames. Attributes which cannot be filled due to being at the
// wrong place or which are ambiguous (multiple selections) will be removed.
bool SwFEShell::GetFlyFrameAttr( SfxItemSet &rSet ) const
{
    SwFlyFrame *pFly = GetSelectedOrCurrFlyFrame();
    if (!pFly)
    {
        OSL_ENSURE( false, "GetFlyFrameAttr, no Fly selected." );
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
bool SwFEShell::SetFlyFrameAttr( SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );
    bool bRet = false;

    if( rSet.Count() )
    {
        SwFlyFrame *pFly = GetSelectedOrCurrFlyFrame();
        OSL_ENSURE( pFly, "SetFlyFrameAttr, no Fly selected." );
        if( pFly )
        {
            StartAllAction();
            const Point aPt( pFly->Frame().Pos() );

            if( SfxItemState::SET == rSet.GetItemState( RES_ANCHOR, false ))
                sw_ChkAndSetNewAnchor( *pFly, rSet );
            SwFlyFrameFormat* pFlyFormat = pFly->GetFormat();

            if( GetDoc()->SetFlyFrameAttr( *pFlyFormat, rSet ))
            {
                bRet = true;
                SwFlyFrame* pFrame = pFlyFormat->GetFrame( &aPt );
                if( pFrame )
                    SelectFlyFrame( *pFrame, true );
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
    // shall be changed in FEShell::SetFlyFrameAttr/SetFlyFrameAnchor,
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

    if( GetDoc()->SetFlyFrameAttr( *pFormat, rSet ))
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
bool SwFEShell::ResetFlyFrameAttr( sal_uInt16 nWhich, const SfxItemSet* pSet )
{
    bool bRet = false;

    if( RES_ANCHOR != nWhich && RES_CHAIN != nWhich && RES_CNTNT != nWhich )
    {
        SET_CURR_SHELL( this );

        SwFlyFrame *pFly = GetSelectedOrCurrFlyFrame();
        OSL_ENSURE( pFly, "SetFlyFrameAttr, no Fly selected." );
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
    SwFrameFormat* pRet = nullptr;
    SwLayoutFrame *pFly = GetSelectedFlyFrame();
    if( pFly && ( pRet = static_cast<SwFrameFormat*>(pFly->GetFormat()->DerivedFrom()) ) ==
                                            GetDoc()->GetDfltFrameFormat() )
        pRet = nullptr;
    return pRet;
}

void SwFEShell::SetFrameFormat( SwFrameFormat *pNewFormat, bool bKeepOrient, Point* pDocPos )
{
    SwFlyFrame *pFly = nullptr;
    if(pDocPos)
    {
        const SwFrameFormat* pFormat = GetFormatFromObj( *pDocPos );

        if (const SwFlyFrameFormat* pFlyFormat = dynamic_cast<const SwFlyFrameFormat*>(pFormat))
            pFly = pFlyFormat->GetFrame();
    }
    else
        pFly = GetSelectedFlyFrame();
    OSL_ENSURE( pFly, "SetFrameFormat: no frame" );
    if( pFly )
    {
        StartAllAction();
        SET_CURR_SHELL( this );

        SwFlyFrameFormat* pFlyFormat = pFly->GetFormat();
        const Point aPt( pFly->Frame().Pos() );

        SfxItemSet* pSet = nullptr;
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pNewFormat->GetItemState( RES_ANCHOR, false, &pItem ))
        {
            pSet = new SfxItemSet( GetDoc()->GetAttrPool(), aFrameFormatSetRange );
            pSet->Put( *pItem );
            if( !sw_ChkAndSetNewAnchor( *pFly, *pSet ))
            {
                delete pSet;
                pSet = nullptr;
            }
        }

        if( GetDoc()->SetFrameFormatToFly( *pFlyFormat, *pNewFormat, pSet, bKeepOrient ))
        {
            SwFlyFrame* pFrame = pFlyFormat->GetFrame( &aPt );
            if( pFrame )
                SelectFlyFrame( *pFrame, true );
            else
                GetLayout()->SetAssertFlyPages();
        }
        delete pSet;

        EndAllActionAndCall();
    }
}

const SwFrameFormat* SwFEShell::GetFlyFrameFormat() const
{
    const SwFlyFrame* pFly = GetSelectedOrCurrFlyFrame();
    if (pFly)
        return pFly->GetFormat();
    return nullptr;
}

SwFrameFormat* SwFEShell::GetFlyFrameFormat()
{
    SwFlyFrame* pFly = GetSelectedOrCurrFlyFrame();
    if (pFly)
        return pFly->GetFormat();
    return nullptr;
}

SwRect SwFEShell::GetFlyRect() const
{
    SwFlyFrame *pFly = GetCurrFlyFrame(false);
    if (!pFly)
    {
        SwRect aRect;
        return aRect;
    }
    else
        return pFly->Frame();
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

    SwFlyFrame *pFly = FindFlyFrame( xObj );
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
    // Scaling is done by SwNoTextFrame::Format by calling
    // SwWrtShell::CalcAndSetScale()
    if ( rRect.SSize() != pFly->Prt().SSize() && !bSizeProt )
    {
         Size aSz( rRect.SSize() );

        //JP 28.02.2001: Task 74707 - ask for fly in fly with automatic size

        const SwFrame* pAnchor;
        const SwTextNode* pTNd;
        const SwpHints* pHts;
        const SwFormatFrameSize& rFrameSz = pFly->GetFormat()->GetFrameSize();
        if (m_bCheckForOLEInCaption &&
            0 != rFrameSz.GetWidthPercent() &&
            nullptr != (pAnchor = pFly->GetAnchorFrame()) &&
            pAnchor->IsTextFrame() &&
            !pAnchor->GetNext() && !pAnchor->GetPrev() &&
            pAnchor->GetUpper()->IsFlyFrame() &&
            nullptr != ( pTNd = static_cast<const SwTextFrame*>(pAnchor)->GetNode()->GetTextNode()) &&
            nullptr != ( pHts = pTNd->GetpSwpHints() ))
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
                    SwFlyFrame* pChgFly = const_cast<SwFlyFrame*>(static_cast<const SwFlyFrame*>(pAnchor->GetUpper()));
                    // calculate the changed size:
                    // width must change, height can change
                    Size aNewSz( aSz.Width() + pChgFly->Frame().Width() -
                                   pFly->Prt().Width(), aSz.Height() );

                    SwFrameFormat *pFormat = pChgFly->GetFormat();
                    SwFormatFrameSize aFrameSz( pFormat->GetFrameSize() );
                    aFrameSz.SetWidth( aNewSz.Width() );
                    if( ATT_MIN_SIZE != aFrameSz.GetHeightSizeType() )
                    {
                        aNewSz.Height() += pChgFly->Frame().Height() -
                                               pFly->Prt().Height();
                        if( std::abs( aNewSz.Height() - pChgFly->Frame().Height()) > 1 )
                            aFrameSz.SetHeight( aNewSz.Height() );
                    }
                    // via Doc for the Undo!
                    pFormat->GetDoc()->SetAttr( aFrameSz, *pFormat );
                    break;
                }
            }
        }

        // set the new Size at the fly themself
        if ( pFly->Prt().Height() > 0 && pFly->Prt().Width() > 0 )
        {
            aSz.Width() += pFly->Frame().Width() - pFly->Prt().Width();
            aSz.Height()+= pFly->Frame().Height()- pFly->Prt().Height();
        }
        aResult = pFly->ChgSize( aSz );

        // if the object changes, the contour is outside the object
        OSL_ENSURE( pFly->Lower()->IsNoTextFrame(), "Request without NoText" );
        SwNoTextNode *pNd = static_cast<SwContentFrame*>(pFly->Lower())->GetNode()->GetNoTextNode();
        OSL_ENSURE( pNd, "Request without Node" );
        pNd->SetContour( nullptr );
        ClrContourCache();
    }

    // if only the size is to be adjusted, a position is transported with
    // allocated values
    Point aPt( pFly->Prt().Pos() );
    aPt += pFly->Frame().Pos();
    if ( rRect.Top() != LONG_MIN && rRect.Pos() != aPt && !bPosProt )
    {
        aPt = rRect.Pos();
        aPt.setX(aPt.getX() - pFly->Prt().Left());
        aPt.setY(aPt.getY() - pFly->Prt().Top());

        // in case of paragraph-bound Flys, starting from the new position,
        // a new anchor is to be set. The anchor and the new RelPos are
        // calculated by the Fly and set
        if( pFly->IsFlyAtContentFrame() )
            static_cast<SwFlyAtContentFrame*>(pFly)->SetAbsPos( aPt );
        else
        {
            const SwFrameFormat *pFormat = pFly->GetFormat();
            const SwFormatVertOrient &rVert = pFormat->GetVertOrient();
            const SwFormatHoriOrient &rHori = pFormat->GetHoriOrient();
            const long lXDiff = aPt.getX() - pFly->Frame().Left();
            const long lYDiff = aPt.getY() - pFly->Frame().Top();
            const Point aTmp( rHori.GetPos() + lXDiff,
                              rVert.GetPos() + lYDiff );
            pFly->ChgRelPos( aTmp );
        }
    }

    SwFlyFrameFormat *pFlyFrameFormat = pFly->GetFormat();
    OSL_ENSURE( pFlyFrameFormat, "fly frame format missing!" );
    if ( pFlyFrameFormat )
        pFlyFrameFormat->SetLastFlyFramePrtRectPos( pFly->Prt().Pos() ); //stores the value of last Prt rect

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
        SwNodeIndex& rCursorNd = GetCursor()->GetPoint()->nNode;
        if( rCursorNd.GetIndex() > mpDoc->GetNodes().GetEndOfExtras().GetIndex() )
            // Cursor is in the body area!
            return nullptr;

        for( auto pFormat : rSpzArr )
        {
            const SwNodeIndex* pIdx = pFormat->GetContent( false ).GetContentIdx();
            SwStartNode* pSttNd;
            if( pIdx &&
                nullptr != ( pSttNd = pIdx->GetNode().GetStartNode() ) &&
                pSttNd->GetIndex() < rCursorNd.GetIndex() &&
                rCursorNd.GetIndex() < pSttNd->EndOfSectionIndex() )
            {
                // found: return immediately
                return pFormat;
            }
        }
    }
    return nullptr;
}

void SwFEShell::SetFlyName( const OUString& rName )
{
    SwLayoutFrame *pFly = GetSelectedFlyFrame();
    if( pFly )
        GetDoc()->SetFlyName( *static_cast<SwFlyFrameFormat*>(pFly->GetFormat()), rName );
    else {
        OSL_ENSURE( false, "no FlyFrame selected" );
    }
}

OUString SwFEShell::GetFlyName() const
{
    SwLayoutFrame *pFly = GetSelectedFlyFrame();
    if( pFly )
        return pFly->GetFormat()->GetName();

    OSL_ENSURE( false, "no FlyFrame selected" );
    return OUString();
}

const uno::Reference < embed::XEmbeddedObject > SwFEShell::GetOleRef() const
{
    uno::Reference < embed::XEmbeddedObject > xObj;
    SwFlyFrame * pFly = GetSelectedFlyFrame();
    if (pFly && pFly->Lower() && pFly->Lower()->IsNoTextFrame())
    {
        SwOLENode *pNd = static_cast<SwNoTextFrame*>(pFly->Lower())->GetNode()->GetOLENode();
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
        return nullptr;

    SdrObject* pObj;
    SdrPageView* pPV;
    const SwFrameFormat* pRet = nullptr;
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
        SwFlyFrame *pFly = pFlyObj->GetFlyFrame();
        const SwFormatURL &rURL = pFly->GetFormat()->GetURL();
        if( !rURL.GetURL().isEmpty() || rURL.GetMap() )
        {
            bool bSetTargetFrameName = pTargetFrameName != nullptr;
            bool bSetDescription = pDescription != nullptr;
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
                        aPt -= pFly->Frame().Pos();
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
        return nullptr;

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
        SwFlyFrame *pFly = pFlyObj->GetFlyFrame();
        if ( pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
        {
            SwGrfNode *pNd = static_cast<SwContentFrame*>(pFly->Lower())->GetNode()->GetGrfNode();
            if ( pNd )
            {
                if ( pNd->IsGrfLink() )
                {
                    // halfway ready graphic?
                    ::sfx2::SvLinkSource* pLnkObj = pNd->GetLink()->GetObj();
                    if( pLnkObj && pLnkObj->IsPending() )
                        return nullptr;
                    rbLink = true;
                }

                pNd->GetFileFilterNms( &rName, nullptr );
                if ( rName.isEmpty() )
                    rName = pFly->GetFormat()->GetName();
                return &pNd->GetGrf(true);
            }
        }
    }
    return nullptr;
}

const SwFrameFormat* SwFEShell::GetFormatFromObj( const Point& rPt, SwRect** pRectToFill ) const
{
    SwFrameFormat* pRet = nullptr;

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
        SwPosition aPos( *GetCursor()->GetPoint() );
        Point aPt( rPt );
        GetLayout()->GetCursorOfst( &aPos, aPt );
        SwContentNode *pNd = aPos.nNode.GetNode().GetContentNode();
        SwFrame* pFrame = pNd->getLayoutFrame( GetLayout(), &rPt, nullptr, false )->FindFlyFrame();
        pRet = pFrame ? static_cast<SwLayoutFrame*>(pFrame)->GetFormat() : nullptr;
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
        const SwFlyFrame *pFly = pFlyObj->GetFlyFrame();
        if ( pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
        {
            if ( static_cast<const SwContentFrame*>(pFly->Lower())->GetNode()->GetGrfNode() )
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
        SfxItemSet aFrameSet( mpDoc->GetAttrPool(),
                            pFormat->GetAttrSet().GetRanges() );
        aFrameSet.Set( pFormat->GetAttrSet() );

        // set size and position?
        if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) == nullptr )
        {
            // then let's do it:
            const Rectangle &rBound = pObj->GetSnapRect();
            Point aRelPos( pObj->GetRelativePos() );

            const long nWidth = rBound.Right()  - rBound.Left();
            const long nHeight= rBound.Bottom() - rBound.Top();
            aFrameSet.Put( SwFormatFrameSize( ATT_MIN_SIZE,
                                std::max( nWidth,  long(MINFLY) ),
                                std::max( nHeight, long(MINFLY) )));

            if( SfxItemState::SET != aFrameSet.GetItemState( RES_HORI_ORIENT ))
                aFrameSet.Put( SwFormatHoriOrient( aRelPos.getX(), text::HoriOrientation::NONE, text::RelOrientation::FRAME ));

            if( SfxItemState::SET != aFrameSet.GetItemState( RES_VERT_ORIENT ))
                aFrameSet.Put( SwFormatVertOrient( aRelPos.getY(), text::VertOrientation::NONE, text::RelOrientation::FRAME ));

        }

        pObj->GetOrdNum();

        StartAllAction();
        StartUndo();

        // delete "Sdr-Object", insert the graphic instead
        DelSelectedObj();

        GetDoc()->getIDocumentContentOperations().Insert( *GetCursor(), rGrfName, rFltName, pGrf, &aFrameSet, nullptr, nullptr );

        EndUndo();
        EndAllAction();
        bRet = true;
    }
    return bRet;
}

static sal_uInt16 SwFormatGetPageNum(const SwFlyFrameFormat * pFormat)
{
    OSL_ENSURE(pFormat != nullptr, "invalid argument");

    SwFlyFrame * pFrame = pFormat->GetFrame();

    sal_uInt16 aResult;

    if (pFrame != nullptr)
        aResult = pFrame->GetPhyPageNum();
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
                GetDoc()->SetFlyFrameTitle( dynamic_cast<SwFlyFrameFormat&>(*pFormat),
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
                GetDoc()->SetFlyFrameDescription(dynamic_cast<SwFlyFrameFormat&>(*pFormat),
                                               rDescription);
            }
            else
            {
                pObj->SetDescription( rDescription );
            }
        }
    }
}

void SwFEShell::AlignFormulaToBaseline( const uno::Reference < embed::XEmbeddedObject >& xObj, SwFlyFrame * pFly )
{
#if OSL_DEBUG_LEVEL > 0
    SvGlobalName aCLSID( xObj->getClassID() );
    const bool bStarMath = ( SotExchange::IsMath( aCLSID ) != 0 );
    OSL_ENSURE( bStarMath, "AlignFormulaToBaseline should only be called for Math objects" );

    if ( !bStarMath )
        return;
#endif

    if (!pFly)
        pFly = FindFlyFrame( xObj );
    OSL_ENSURE( pFly , "No fly frame!" );
    SwFrameFormat * pFrameFormat = pFly ? pFly->GetFormat() : nullptr;

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
            nBaseline += pFlyFrameFormat->GetLastFlyFramePrtRectPos().Y();

        const SwFormatVertOrient &rVert = pFrameFormat->GetVertOrient();
        SwFormatVertOrient aVert( rVert );
        aVert.SetPos( -nBaseline );
        aVert.SetVertOrient( css::text::VertOrientation::NONE );

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
    while ( nullptr != (pStNd = aIdx.GetNode().GetStartNode()) )
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
