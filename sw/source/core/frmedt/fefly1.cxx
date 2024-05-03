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
#include <o3tl/any.hxx>
#include <svl/itemiter.hxx>
#include <vcl/imapobj.hxx>
#include <editeng/protitem.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdouno.hxx>
#include <tools/globname.hxx>
#include <sot/exchange.hxx>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <comphelper/types.hxx>
#include <osl/diagnose.h>
#include <comphelper/scopeguard.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <fmtornt.hxx>
#include <fmturl.hxx>
#include <fmtfsize.hxx>
#include <fesh.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <viewimp.hxx>
#include <viscrs.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <redline.hxx>
#include <dview.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <ndgrf.hxx>
#include <flyfrms.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <swundo.hxx>
#include <txatbase.hxx>
#include <frame.hxx>
#include <notxtfrm.hxx>
#include <HandleAnchorNodeChg.hxx>
#include <frmatr.hxx>
#include <fmtsrnd.hxx>
#include <ndole.hxx>
#include <fefly.hxx>
#include <fmtcnct.hxx>
#include <frameformats.hxx>
#include <textboxhelper.hxx>


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
        std::pair<Point, bool> const tmp(rPt, false);
        const SwContentFrame* pCFrame = pCntNd ? pCntNd->getLayoutFrame(
            pCntNd->GetDoc().getIDocumentLayoutAccess().GetCurrentLayout(),
            nullptr, &tmp) : nullptr;
        const SwPageFrame *pPg = pCFrame ? pCFrame->FindPageFrame() : nullptr;

        rAnchor.SetPageNum( pPg ? pPg->GetPhyPageNum() : 1 );
        rAnchor.SetType( RndStdIds::FLY_AT_PAGE );
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
    SwFormatAnchor aNewAnch( rSet.Get( RES_ANCHOR ) );
    RndStdIds nNew = aNewAnch.GetAnchorId();
    const SwFrame *pNewAnch;

    //determine new anchor
    Point aTmpPnt( rPt );
    switch( nNew )
    {
    case RndStdIds::FLY_AS_CHAR:  // also include this?
    case RndStdIds::FLY_AT_PARA:
    case RndStdIds::FLY_AT_CHAR: // LAYER_IMPL
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
            SwPosition aPos( pNewAnch->IsTextFrame()
                ? *static_cast<SwTextFrame const*>(pNewAnch)->GetTextNodeForParaProps()
                : *static_cast<const SwNoTextFrame*>(pNewAnch)->GetNode() );
            if ((RndStdIds::FLY_AT_CHAR == nNew) || (RndStdIds::FLY_AS_CHAR == nNew))
            {
                // textnode should be found, as only in those
                // a content bound frame can be anchored
                SwCursorMoveState aState( CursorMoveState::SetOnlyText );
                aTmpPnt.setX(aTmpPnt.getX() - 1);                   // do not land in the fly!
                if( !pNewAnch->GetModelPositionForViewPoint( &aPos, aTmpPnt, &aState ) )
                {
                    assert(pNewAnch->IsTextFrame()); // because AT_CHAR/AS_CHAR
                    SwTextFrame const*const pTextFrame(
                            static_cast<SwTextFrame const*>(pNewAnch));
                    if( pNewAnch->getFrameArea().Bottom() < aTmpPnt.Y() )
                    {
                        aPos = pTextFrame->MapViewToModelPos(TextFrameIndex(0));
                    }
                    else
                    {
                        aPos = pTextFrame->MapViewToModelPos(
                            TextFrameIndex(pTextFrame->GetText().getLength()));
                    }
                }
                else
                {
                    if ( SwCursorShell::PosInsideInputField( aPos ) )
                    {
                        aPos.SetContent( SwCursorShell::StartOfInputFieldAtPos( aPos ) );
                    }
                }
            }
            aNewAnch.SetAnchor( &aPos );
        }
        break;

    case RndStdIds::FLY_AT_FLY: // LAYER_IMPL
        {
            // starting from the upper-left corner of the Fly
            // search nearest SwFlyFrame
            SwCursorMoveState aState( CursorMoveState::SetOnlyText );
            SwPosition aPos( rDoc.GetNodes() );
            aTmpPnt.setX(aTmpPnt.getX() - 1);                   // do not land in the fly!
            rDoc.getIDocumentLayoutAccess().GetCurrentLayout()->GetModelPositionForViewPoint( &aPos, aTmpPnt, &aState );
            pNewAnch = ::FindAnchor(
                aPos.GetNode().GetContentNode()->getLayoutFrame(rFrame.getRootFrame(), nullptr, nullptr),
                aTmpPnt )->FindFlyFrame();

            if( pNewAnch && &rFrame != pNewAnch && !pNewAnch->IsProtected() )
            {
                aPos.Assign( *static_cast<const SwFlyFrame*>(pNewAnch)->GetFormat()->GetContent().
                                GetContentIdx() );
                aNewAnch.SetAnchor( &aPos );
                break;
            }
        }

        nNew = RndStdIds::FLY_AT_PAGE;
        aNewAnch.SetType( nNew );
        [[fallthrough]];

    case RndStdIds::FLY_AT_PAGE:
        pNewAnch = rFrame.FindPageFrame();
        aNewAnch.SetPageNum( pNewAnch->GetPhyPageNum() );
        break;

    default:
        OSL_ENSURE( false, "Wrong Id for new anchor." );
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

    RndStdIds nNew = rSet.Get( RES_ANCHOR ).GetAnchorId();

    if( nOld == nNew )
        return false;

    SwDoc* pDoc = const_cast<SwDoc*>(rFormat.GetDoc());

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( !(nNew == RndStdIds::FLY_AT_PAGE &&
        (RndStdIds::FLY_AT_PARA==nOld || RndStdIds::FLY_AT_CHAR==nOld || RndStdIds::FLY_AS_CHAR==nOld ) &&
        pDoc->IsInHeaderFooter( *rOldAnch.GetAnchorNode() )),
            "forbidden anchor change in Head/Foot." );
#endif

    return ::lcl_FindAnchorPos( *pDoc, rFly.getFrameArea().Pos(), rFly, rSet );
}

void SwFEShell::SelectFlyFrame( SwFlyFrame& rFrame )
{
    CurrShell aCurr( this );

    // The frame is new, thus select it.
    // !! Always select the frame, if it's not selected.
    // - it could be a new "old" one because the anchor was changed
    // - "old" frames have had to be selected previously otherwise they could
    //   not have been changed
    // The frames should not be selected by the document position, because
    // it should have been selected!
    SwViewShellImp *pImpl = Imp();
    if( !GetWin() )
        return;

    OSL_ENSURE( rFrame.IsFlyFrame(), "SelectFlyFrame wants a Fly" );

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

    rFrame.SelectionHasChanged(this);

    KillPams();
    ClearMark();
    SelFlyGrabCursor();
}

void SwFEShell::UnfloatFlyFrame()
{
    GetIDocumentUndoRedo().StartUndo(SwUndoId::DELLAYFMT, nullptr);
    comphelper::ScopeGuard g([this]
                             { GetIDocumentUndoRedo().EndUndo(SwUndoId::DELLAYFMT, nullptr); });

    SwFlyFrame* pFly = GetSelectedFlyFrame();
    if (!pFly)
    {
        return;
    }

    SwFrameFormat* pFlyFormat = pFly->GetFrameFormat();
    const SwFormatContent& rContent = pFlyFormat->GetContent();
    const SwNodeIndex* pFlyStart = rContent.GetContentIdx();
    if (!pFlyStart)
    {
        return;
    }

    const SwEndNode* pFlyEnd = pFlyStart->GetNode().EndOfSectionNode();
    if (!pFlyEnd)
    {
        return;
    }

    // Create an empty paragraph after the table, so the frame's SwNodes section is non-empty after
    // MoveNodeRange(). Undo would ensure it's non-empty and then node offsets won't match.
    IDocumentContentOperations& rIDCO = GetDoc()->getIDocumentContentOperations();
    {
        SwNodeIndex aInsertIndex(*pFlyEnd);
        --aInsertIndex;
        SwPosition aInsertPos(aInsertIndex);
        StartAllAction();
        rIDCO.AppendTextNode(aInsertPos);
        // Make sure that a layout frame is created for the node, so the fly frame is not deleted,
        // during MoveNodeRange(), either.
        EndAllAction();
    }

    SwNodeRange aRange(pFlyStart->GetNode(), SwNodeOffset(1), *pFlyEnd, SwNodeOffset(-1));
    const SwFormatAnchor& rAnchor = pFlyFormat->GetAnchor();
    SwNode* pAnchor = rAnchor.GetAnchorNode();
    if (!pAnchor)
    {
        return;
    }

    // Move the content outside of the text frame.
    SwNodeIndex aInsertPos(*pAnchor);
    rIDCO.MoveNodeRange(aRange, aInsertPos.GetNode(), SwMoveFlags::CREATEUNDOOBJ);

    // Remove the fly frame frame.
    IDocumentLayoutAccess& rIDLA = pFlyFormat->getIDocumentLayoutAccess();
    rIDLA.DelLayoutFormat(pFlyFormat);
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
SwFlyFrame* SwFEShell::GetSelectedOrCurrFlyFrame() const
{
    SwFlyFrame *pFly = GetSelectedFlyFrame();
    if (pFly)
        return pFly;
    return GetCurrFlyFrame();
}

// Returns non-null pointer, if the current Fly could be anchored to another one (so it is inside)
const SwFrameFormat* SwFEShell::IsFlyInFly()
{
    CurrShell aCurr( this );

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
    else if ( rMrkList.GetMarkCount() != 1 )
        return nullptr;

    SdrObject *pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
    SwContact* pContact = GetUserCall( pObj );
    if (!pContact)
        return nullptr;

    SwFrameFormat *pFormat = FindFrameFormat( pObj );
    if( pFormat && RndStdIds::FLY_AT_FLY == pFormat->GetAnchor().GetAnchorId() )
    {
        const SwFrame* pFly;
        if (SwVirtFlyDrawObj* pFlyObj = dynamic_cast<SwVirtFlyDrawObj *>(pObj))
        {
            pFly = pFlyObj->GetFlyFrame()->GetAnchorFrame();
        }
        else
        {
            pFly = static_cast<SwDrawContact*>(pContact)->GetAnchorFrame(pObj);
        }

        assert(pFly && "IsFlyInFly: Where's my anchor?");
        OSL_ENSURE( pFly->IsFlyFrame(), "IsFlyInFly: Funny anchor!" );
        return static_cast<const SwFlyFrame*>(pFly)->GetFormat();
    }

    Point aTmpPos = pObj->GetCurrentBoundRect().TopLeft();

    SwFrame *pTextFrame;
    {
        SwCursorMoveState aState( CursorMoveState::SetOnlyText );
        SwNodeIndex aSwNodeIndex( GetDoc()->GetNodes() );
        SwPosition aPos( aSwNodeIndex );
        Point aPoint( aTmpPos );
        aPoint.setX(aPoint.getX() - 1);                    //do not land in the fly!!
        GetLayout()->GetModelPositionForViewPoint( &aPos, aPoint, &aState );
        // determine text frame by left-top-corner of object
        SwContentNode *pNd = aPos.GetNode().GetContentNode();
        std::pair<Point, bool> const tmp(aTmpPos, false);
        pTextFrame = pNd ? pNd->getLayoutFrame(GetLayout(), nullptr, &tmp) : nullptr;
    }
    const SwFrame *pTmp = pTextFrame ? ::FindAnchor(pTextFrame, aTmpPos) : nullptr;
    const SwFlyFrame *pFly = pTmp ? pTmp->FindFlyFrame() : nullptr;
    if( pFly )
        return pFly->GetFormat();
    return nullptr;
}

void SwFEShell::SetFlyPos( const Point& rAbsPos )
{
    CurrShell aCurr( this );

    // Determine reference point in document coordinates
    SwFlyFrame *pFly = GetCurrFlyFrame(false);
    if (!pFly)
        return;

    //SwSaveHdl aSaveX( Imp() );

    // Set an anchor starting from the absolute position for paragraph bound Flys
    // Anchor and new RelPos will be calculated and set by the Fly
    if ( pFly->IsFlyAtContentFrame() )
    {
        if(pFly->IsFlyFreeFrame() && static_cast< SwFlyFreeFrame* >(pFly)->isTransformableSwFrame())
        {
            // RotateFlyFrame3: When we have a change and are in transformed state (e.g. rotation used),
            // we need to correct the absolute position (rAbsPos) which was created in
            // transformed coordinates to untransformed state
            TransformableSwFrame* pTransformableSwFrame(static_cast<SwFlyFreeFrame*>(pFly)->getTransformableSwFrame());
            const SwRect aUntransformedFrameArea(pTransformableSwFrame->getUntransformedFrameArea());
            const Point aNewAbsPos(
                rAbsPos.X() + aUntransformedFrameArea.Left() - pFly->getFrameArea().Left(),
                rAbsPos.Y() + aUntransformedFrameArea.Top() - pFly->getFrameArea().Top());
            static_cast<SwFlyAtContentFrame*>(pFly)->SetAbsPos(aNewAbsPos);
        }
        else
        {
            static_cast<SwFlyAtContentFrame*>(pFly)->SetAbsPos( rAbsPos );
        }
    }
    else
    {
        const SwFrame *pAnch = pFly->GetAnchorFrame();
        Point aOrient( pAnch->getFrameArea().Pos() );

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

    CurrShell aCurr( this );

    if ( !Imp()->HasDrawView() )
        return aRet;

    const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
    if (rMrkList.GetMarkCount() != 1)
        return aRet;

    SdrObject* pObj = rMrkList.GetMark(0)->GetMarkedSdrObj();

    SwContact* pContact = ::GetUserCall( pObj );
    if (!pContact)
        return aRet;

    assert(pObj);

    // #i28701#
    SwAnchoredObject* pAnchoredObj = pContact->GetAnchoredObj( pObj );
    SwFrameFormat* pFormat = pAnchoredObj->GetFrameFormat();
    const RndStdIds nAnchorId = pFormat->GetAnchor().GetAnchorId();

    if ( RndStdIds::FLY_AS_CHAR == nAnchorId )
        return aRet;

    bool bFlyFrame = dynamic_cast<SwVirtFlyDrawObj *>(pObj) != nullptr;

    bool bTextBox = false;
    if (pFormat->Which() == RES_DRAWFRMFMT)
    {
        bTextBox = SwTextBoxHelper::isTextBox(pFormat, RES_DRAWFRMFMT, pObj);
    }

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
        if ( RndStdIds::FLY_AT_PAGE != nAnchorId )
        {
            pFooterOrHeader = pContent->FindFooterOrHeader();
        }
    }
    else if (bTextBox)
    {
        auto pFlyFormat
            = dynamic_cast<const SwFlyFrameFormat*>(SwTextBoxHelper::getOtherTextBoxFormat(
                pFormat, RES_DRAWFRMFMT, pObj));
        if (pFlyFormat)
        {
            pFly = pFlyFormat->GetFrame();
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
        SwCursorMoveState aState( CursorMoveState::SetOnlyText );
        SwPosition aPos( GetDoc()->GetNodes().GetEndOfExtras() );
        Point aTmpPnt( rAbsPos );
        GetLayout()->GetModelPositionForViewPoint( &aPos, aTmpPnt, &aState );
        if (aPos.GetNode() != GetDoc()->GetNodes().GetEndOfExtras()
            && (nAnchorId != RndStdIds::FLY_AT_CHAR || !PosInsideInputField(aPos)))
        {
            SwContentNode* pCNode = aPos.GetNode().GetContentNode();
            assert(pCNode);
            pTextFrame = pCNode->getLayoutFrame(GetLayout(), &aPos, nullptr);
        }
    }
    const SwFrame *pNewAnch = nullptr;
    if( pTextFrame != nullptr )
    {
        if ( RndStdIds::FLY_AT_PAGE == nAnchorId )
        {
            pNewAnch = pTextFrame->FindPageFrame();
        }
        else
        {
            pNewAnch = ::FindAnchor( pTextFrame, rAbsPos );

            if( RndStdIds::FLY_AT_FLY == nAnchorId ) // LAYER_IMPL
            {
                pNewAnch = pNewAnch->FindFlyFrame();
            }
        }
    }

    if( pNewAnch && !pNewAnch->IsProtected() )
    {
        const SwFlyFrame* pCheck = (bFlyFrame || bTextBox) ? pNewAnch->FindFlyFrame() : nullptr;
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

            if ( bMoveIt || (nAnchorId == RndStdIds::FLY_AT_CHAR) )
            {
                SwFormatAnchor aAnch( pFormat->GetAnchor() );
                switch ( nAnchorId )
                {
                    case RndStdIds::FLY_AT_PARA:
                    {
                        SwPosition pos(pTextFrame->IsTextFrame()
                            ? *static_cast<SwTextFrame const*>(pTextFrame)->GetTextNodeForParaProps()
                            : *static_cast<const SwNoTextFrame*>(pTextFrame)->GetNode());
                        aAnch.SetAnchor( &pos );
                        break;
                    }
                    case RndStdIds::FLY_AT_PAGE:
                    {
                        aAnch.SetPageNum( static_cast<const SwPageFrame*>(pNewAnch)->
                                          GetPhyPageNum() );
                        break;
                    }

                    case RndStdIds::FLY_AT_FLY:
                    {
                        SwPosition aPos( *static_cast<const SwFlyFrame*>(pNewAnch)->GetFormat()->
                                                  GetContent().GetContentIdx() );
                        aAnch.SetAnchor( &aPos );
                        break;
                    }

                    case RndStdIds::FLY_AT_CHAR:
                        {
                            SwPosition pos = *aAnch.GetContentAnchor();
                            Point aTmpPnt( rAbsPos );
                            if( pTextFrame->GetModelPositionForViewPoint( &pos, aTmpPnt ) )
                            {
                                SwRect aTmpRect;
                                pTextFrame->GetCharRect( aTmpRect, pos );
                                aRet = aTmpRect.Pos();
                            }
                            else
                            {
                                pos = static_cast<SwTextFrame const*>(pTextFrame)->MapViewToModelPos(TextFrameIndex(0));
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
                        std::unique_ptr<SwHandleAnchorNodeChg> pHandleAnchorNodeChg;
                        SwFlyFrameFormat* pFlyFrameFormat( dynamic_cast<SwFlyFrameFormat*>(pFormat) );
                        if ( pFlyFrameFormat )
                        {
                            pHandleAnchorNodeChg.reset(
                                new SwHandleAnchorNodeChg( *pFlyFrameFormat, aAnch ));
                        }
                        pFormat->GetDoc()->SetAttr( aAnch, *pFormat );
                        if (SwTextBoxHelper::getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT,
                            pObj))
                        {
                            if (SdrObjList* pObjList = pObj->getChildrenOfSdrObject())
                            {
                                for (const rtl::Reference<SdrObject>& pChild : *pObjList)
                                    SwTextBoxHelper::changeAnchor(pFormat, pChild.get());
                            }
                            else
                                SwTextBoxHelper::syncFlyFrameAttr(
                                    *pFormat, pFormat->GetAttrSet(), pObj);
                        }
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
            if( COL_TRANSPARENT != GetOut()->GetLineColor() )
            {
                OSL_FAIL( "Hey, Joe: Where's my Null Pen?" );
                GetOut()->SetLineColor( COL_TRANSPARENT );
            }
#endif
        }
    }

    return aRet;
}

const SwFrameFormat *SwFEShell::NewFlyFrame( const SfxItemSet& rSet, bool bAnchValid,
                           SwFrameFormat *pParent )
{
    CurrShell aCurr( this );
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
            ParkCursor( *aBoxes[0]->GetSttNd() );

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

    SwFormatAnchor& rAnch = const_cast<SwFormatAnchor&>(rSet.Get( RES_ANCHOR ));
    RndStdIds eRndId = rAnch.GetAnchorId();
    switch( eRndId )
    {
    case RndStdIds::FLY_AT_PAGE:
        if( !rAnch.GetPageNum() )       //HotFix: Bug in UpdateByExample
            rAnch.SetPageNum( 1 );
        break;

    case RndStdIds::FLY_AT_FLY:
    case RndStdIds::FLY_AT_PARA:
    case RndStdIds::FLY_AT_CHAR:
    case RndStdIds::FLY_AS_CHAR:
        if( !bAnchValid )
        {
            if( RndStdIds::FLY_AT_FLY != eRndId )
            {
                rAnch.SetAnchor( &rPos );
            }
            else if( lcl_SetNewFlyPos( rPos.GetNode(), rAnch, aPt ) )
            {
                eRndId = RndStdIds::FLY_AT_PAGE;
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
        GetDoc()->GetIDocumentUndoRedo().StartUndo( SwUndoId::INSLAYFMT, nullptr );
        std::unique_ptr<SwFormatAnchor> pOldAnchor;
        bool bHOriChgd = false, bVOriChgd = false;
        std::shared_ptr<SwFormatVertOrient> aOldV;
        std::shared_ptr<SwFormatHoriOrient> aOldH;

        if ( RndStdIds::FLY_AT_PAGE != eRndId )
        {
           // First as with page link. Paragraph/character link on if
            // everything was shifted. Then the position is valid!
            // JP 13.05.98: if necessary also convert the horizontal/vertical
            //              orientation, to prevent correction during re-anchoring
            pOldAnchor.reset(new SwFormatAnchor( rAnch ));
            const_cast<SfxItemSet&>(rSet).Put( SwFormatAnchor( RndStdIds::FLY_AT_PAGE, 1 ) );

            const SwFormatHoriOrient* pHoriOrientItem;
            if( (pHoriOrientItem = rSet.GetItemIfSet( RES_HORI_ORIENT, false ))
                && text::HoriOrientation::NONE == pHoriOrientItem->GetHoriOrient() )
            {
                bHOriChgd = true;
                aOldH.reset(pHoriOrientItem->Clone());
                const_cast<SfxItemSet&>(rSet).Put( SwFormatHoriOrient( 0, text::HoriOrientation::LEFT ) );
            }
            const SwFormatVertOrient* pVertOrientItem;
            if( (pVertOrientItem = rSet.GetItemIfSet( RES_VERT_ORIENT, false ))
                && text::VertOrientation::NONE == pVertOrientItem->GetVertOrient() )
            {
                bVOriChgd = true;
                aOldV.reset(pVertOrientItem->Clone());
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
                //              anchor should not lie in the shifted area
                pRet->DelFrames();

                const SwFrame* pAnch = ::FindAnchor( GetLayout(), aPt );
                SwPosition aPos( pAnch->IsTextFrame()
                    ? *static_cast<SwTextFrame const*>(pAnch)->GetTextNodeForParaProps()
                    : *static_cast<const SwNoTextFrame*>(pAnch)->GetNode() );

                if ( RndStdIds::FLY_AS_CHAR == eRndId )
                {
                    assert(pAnch->IsTextFrame());
                    aPos = static_cast<SwTextFrame const*>(pAnch)->MapViewToModelPos(TextFrameIndex(0));
                }
                pOldAnchor->SetAnchor( &aPos );

                // shifting of table selection is not Undo-capable. therefore
                // changing the anchors should not be recorded
                bool const bDoesUndo =
                    GetDoc()->GetIDocumentUndoRedo().DoesUndo();
                SwUndoId nLastUndoId(SwUndoId::EMPTY);
                if (bDoesUndo &&
                    GetDoc()->GetIDocumentUndoRedo().GetLastUndoInfo(nullptr,
                        & nLastUndoId))
                {
                    if (SwUndoId::INSLAYFMT == nLastUndoId)
                    {
                        GetDoc()->GetIDocumentUndoRedo().DoUndo(false);
                    }
                }

                const_cast<SfxItemSet&>(rSet).Put( std::move(pOldAnchor) );

                if( bHOriChgd )
                    const_cast<SfxItemSet&>(rSet).Put( *aOldH );
                if( bVOriChgd )
                    const_cast<SfxItemSet&>(rSet).Put( *aOldV );

                GetDoc()->SetFlyFrameAttr( *pRet, const_cast<SfxItemSet&>(rSet) );
                GetDoc()->GetIDocumentUndoRedo().DoUndo(bDoesUndo);
            }
        }
        GetDoc()->GetIDocumentUndoRedo().EndUndo( SwUndoId::INSLAYFMT, nullptr );
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
            SelectFlyFrame( *pFrame );
        else
        {
            GetLayout()->SetAssertFlyPages();
            pRet = nullptr;
        }
    }
    EndAllActionAndCall();

    return pRet;
}

namespace
{
/// If pCursor points to an as-char anchored graphic node, then set the node's anchor position on
/// pAnchor and rPam.
bool SetAnchorOnGrfNodeForAsChar(SwShellCursor *pCursor, SwFormatAnchor* pAnchor, std::optional<SwPaM>& rPam)
{
    const SwPosition* pPoint = pCursor->GetPoint();
    if (pAnchor->GetAnchorId() != RndStdIds::FLY_AS_CHAR)
    {
        return false;
    }

    if (!pPoint->GetNode().IsGrfNode())
    {
        return false;
    }

    SwFrameFormat* pFrameFormat = pPoint->GetNode().GetFlyFormat();
    if (!pFrameFormat)
    {
        return false;
    }

    const SwPosition* pContentAnchor = pFrameFormat->GetAnchor().GetContentAnchor();
    if (!pContentAnchor)
    {
        return false;
    }

    SwPosition aPosition(*pContentAnchor);
    ++aPosition.nContent;
    pAnchor->SetAnchor(&aPosition);
    rPam.emplace(aPosition);
    return true;
}
}

void SwFEShell::Insert( const OUString& rGrfName, const OUString& rFltName,
                        const Graphic* pGraphic,
                        const SfxItemSet* pFlyAttrSet )
{
    SwFlyFrameFormat* pFormat = nullptr;
    CurrShell aCurr( this );
    StartAllAction();
    SwShellCursor *pStartCursor = dynamic_cast<SwShellCursor*>(GetCursor());
    SwShellCursor *pCursor = pStartCursor;
    do
    {
        if (!pCursor)
            break;

        // Has the anchor not been set or been set incompletely?
        std::optional<SwPaM> oPam;
        if( pFlyAttrSet )
        {
            if( const SwFormatAnchor* pItem = pFlyAttrSet->GetItemIfSet( RES_ANCHOR, false ) )
            {
                SwFormatAnchor* pAnchor = const_cast<SwFormatAnchor*>(pItem);
                switch( pAnchor->GetAnchorId())
                {
                case RndStdIds::FLY_AT_PARA:
                case RndStdIds::FLY_AT_CHAR: // LAYER_IMPL
                case RndStdIds::FLY_AS_CHAR:
                    if( !pAnchor->GetAnchorNode() )
                    {
                        if (SetAnchorOnGrfNodeForAsChar(pCursor, pAnchor, oPam))
                        {
                            // Don't anchor the image on the previous image, rather insert them next
                            // to each other.
                            break;
                        }

                        pAnchor->SetAnchor( pCursor->GetPoint() );
                    }
                    break;
                case RndStdIds::FLY_AT_FLY:
                    if( !pAnchor->GetAnchorNode() )
                    {
                        lcl_SetNewFlyPos( pCursor->GetPointNode(),
                                *pAnchor, GetCursorDocPos() );
                    }
                    break;
                case RndStdIds::FLY_AT_PAGE:
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
        pFormat = GetDoc()->getIDocumentContentOperations().InsertGraphic(
                                oPam.has_value() ? *oPam : *pCursor, rGrfName,
                                rFltName, pGraphic,
                                pFlyAttrSet,
                                nullptr, nullptr );
        OSL_ENSURE(pFormat, "IDocumentContentOperations::InsertGraphic failed.");

        pCursor = pCursor->GetNext();
    } while( pCursor != pStartCursor );

    EndAllAction();

    if( !pFormat )
        return;

    const Point aPt( GetCursorDocPos() );
    SwFlyFrame* pFrame = pFormat->GetFrame( &aPt );

    if( pFrame )
    {
        // add a redline to the anchor point at tracked insertion of picture
        if ( IsRedlineOn() )
        {
            const SwPosition & rPos(*pFormat->GetAnchor().GetContentAnchor());
            SwPaM aPaM(rPos.GetNode(), rPos.GetContentIndex(),
                    rPos.GetNode(), rPos.GetContentIndex() + 1);
            GetDoc()->getIDocumentRedlineAccess().AppendRedline(
                    new SwRangeRedline( RedlineType::Insert, aPaM ), true);
        }

        // fdo#36681: Invalidate the content and layout to refresh
        // the picture anchoring properly
        SwPageFrame* pPageFrame = pFrame->FindPageFrameOfAnchor();
        pPageFrame->InvalidateFlyLayout();
        pPageFrame->InvalidateContent();

        SelectFlyFrame( *pFrame );
    }
    else
        GetLayout()->SetAssertFlyPages();
}

SwFlyFrameFormat* SwFEShell::InsertObject( const svt::EmbeddedObjectRef&  xObj,
                        SfxItemSet* pFlyAttrSet )
{
    SwFlyFrameFormat* pFormat = nullptr;
    CurrShell aCurr( this );
    StartAllAction();
    {
        for(const SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            pFormat = GetDoc()->getIDocumentContentOperations().InsertEmbObject(
                            rPaM, xObj, pFlyAttrSet );
            OSL_ENSURE(pFormat, "IDocumentContentOperations::InsertEmbObject failed.");
        }
    }
    EndAllAction();

    if( pFormat )
    {
        const Point aPt( GetCursorDocPos() );
        SwFlyFrame* pFrame = pFormat->GetFrame( &aPt );

        if( pFrame )
            SelectFlyFrame( *pFrame );
        else
            GetLayout()->SetAssertFlyPages();
    }

    return pFormat;
}

void SwFEShell::InsertDrawObj( SdrObject& rDrawObj,
                               const Point& rInsertPosition )
{
    CurrShell aCurr( this );

    SfxItemSet rFlyAttrSet( GetDoc()->GetAttrPool(), aFrameFormatSetRange );
    rFlyAttrSet.Put( SwFormatAnchor( RndStdIds::FLY_AT_PARA ));
    // #i89920#
    rFlyAttrSet.Put( SwFormatSurround( css::text::WrapTextMode_THROUGH ) );
    rDrawObj.SetLayer( getIDocumentDrawModelAccess().GetHeavenId() );

    // find anchor position
    SwPaM aPam( mxDoc->GetNodes() );
    {
        SwCursorMoveState aState( CursorMoveState::SetOnlyText );
        Point aTmpPt( rInsertPosition );
        GetLayout()->GetModelPositionForViewPoint( aPam.GetPoint(), aTmpPt, &aState );
        const SwFrame* pFrame = aPam.GetPointContentNode()->getLayoutFrame(GetLayout(), nullptr, nullptr);
        const Point aRelPos( rInsertPosition.X() - pFrame->getFrameArea().Left(),
                             rInsertPosition.Y() - pFrame->getFrameArea().Top() );
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

    if (pFormat)
    {
        pFormat->SetFormatName(rDrawObj.GetName());
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

    for(sw::SpzFrameFormat* pFormat : *mxDoc->GetSpzFrameFormats() )
    {
        if (RndStdIds::FLY_AT_PAGE == pFormat->GetAnchor().GetAnchorId())
        {
            rFillArr.push_back( pFormat );
        }
    }
}

void SwFEShell::SetPageObjsNewPage( std::vector<SwFrameFormat*>& rFillArr )
{
    if( rFillArr.empty() )
        return;

    StartAllAction();
    StartUndo();

    SwRootFrame* pTmpRootFrame = GetLayout();
    sal_uInt16 nMaxPage = pTmpRootFrame->GetPageNum();
    bool bTmpAssert = false;
    for( auto pFormat : rFillArr )
    {
        if (mxDoc->GetSpzFrameFormats()->IsAlive(static_cast<sw::SpzFrameFormat*>(pFormat)))
        {
            // FlyFormat is still valid, therefore process

            SwFormatAnchor aNewAnchor( pFormat->GetAnchor() );
            if (RndStdIds::FLY_AT_PAGE != aNewAnchor.GetAnchorId())
                // Anchor has been changed, therefore: do not change!
                continue;
            sal_uInt16 nNewPage = aNewAnchor.GetPageNum() + 1;
            if (nNewPage > nMaxPage)
            {
                if ( RES_DRAWFRMFMT == pFormat->Which() )
                    pFormat->CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::PAGE_OUT_OF_BOUNDS));
                else
                    pFormat->DelFrames();
                bTmpAssert = true;
            }
            aNewAnchor.SetPageNum(nNewPage);
            mxDoc->SetAttr( aNewAnchor, *pFormat );
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

    CurrShell aCurr( const_cast<SwFEShell*>(this) );

    if( !rSet.Set( pFly->GetFormat()->GetAttrSet() ) )
        return false;

    // now examine all attributes. Remove forbidden attributes, then
    // get all remaining attributes and enter them

    if( const SwFormatAnchor* pAnchor = rSet.GetItemIfSet( RES_ANCHOR, false ) )
    {
        RndStdIds eType = pAnchor->GetAnchorId();

        if ( RndStdIds::FLY_AT_PAGE != eType )
        {
            // OD 12.11.2003 #i22341# - content anchor of anchor item is needed.
            // Thus, don't overwrite anchor item by default constructed anchor item.
            if ( RndStdIds::FLY_AS_CHAR == eType )
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
    CurrShell aCurr( this );
    bool bRet = false;

    if( rSet.Count() )
    {
        SwFlyFrame *pFly = GetSelectedOrCurrFlyFrame();
        OSL_ENSURE(pFly, "SetFlyFrameAttr, no Fly selected.");
        if (pFly)
        {
            StartAllAction();
            const Point aPt( pFly->getFrameArea().Pos() );

            if( SfxItemState::SET == rSet.GetItemState( RES_ANCHOR, false ))
                sw_ChkAndSetNewAnchor( *pFly, rSet );
            SwFlyFrameFormat* pFlyFormat = pFly->GetFormat();

            if( GetDoc()->SetFlyFrameAttr( *pFlyFormat, rSet ))
            {
                bRet = true;
                SwFlyFrame* pFrame = pFlyFormat->GetFrame( &aPt );
                if( pFrame )
                    SelectFlyFrame( *pFrame );
                else
                    GetLayout()->SetAssertFlyPages();
            }

            EndAllActionAndCall();
        }
    }
    return bRet;
}

SfxItemSetFixed<RES_VERT_ORIENT, RES_ANCHOR> SwFEShell::makeItemSetFromFormatAnchor(SfxItemPool& rPool, const SwFormatAnchor &rAnchor)
{
    // The set also includes VERT/HORI_ORIENT, because the align
    // shall be changed in FEShell::SetFlyFrameAttr/SetFlyFrameAnchor,
    // possibly as a result of the anchor change.
    SfxItemSetFixed<RES_VERT_ORIENT, RES_ANCHOR> aSet(rPool);
    aSet.Put(rAnchor);
    return aSet;
}

bool SwFEShell::SetDrawingAttr( SfxItemSet& rSet )
{
    bool bRet = false;
    CurrShell aCurr( this );
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
        RndStdIds nNew = rSet.Get( RES_ANCHOR ).GetAnchorId();
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
        SelectObj( Point(), 0, pObj );
    }
    EndAllActionAndCall();
    EndUndo();
    return bRet;
}

// Reset attributes contained in the set.
void SwFEShell::ResetFlyFrameAttr( const SfxItemSet* pSet )
{
    CurrShell aCurr( this );

    SwFlyFrame *pFly = GetSelectedOrCurrFlyFrame();
    OSL_ENSURE( pFly, "SetFlyFrameAttr, no Fly selected." );
    if( !pFly )
        return;

    StartAllAction();

    SfxItemIter aIter( *pSet );
    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
        if( !IsInvalidItem( pItem ) )
        {
            sal_uInt16 nWhich = pItem->Which();
            if( RES_ANCHOR != nWhich && RES_CHAIN != nWhich && RES_CNTNT != nWhich )
                pFly->GetFormat()->ResetFormatAttr( nWhich );
        }
    }

    EndAllActionAndCall();
    GetDoc()->getIDocumentState().SetModified();
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

void SwFEShell::SetFrameFormat( SwFrameFormat *pNewFormat, bool bKeepOrient, Point const * pDocPos )
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
    if( !pFly )
        return;

    StartAllAction();
    CurrShell aCurr( this );

    SwFlyFrameFormat* pFlyFormat = pFly->GetFormat();
    const Point aPt( pFly->getFrameArea().Pos() );

    std::optional<SfxItemSet> oSet;
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == pNewFormat->GetItemState( RES_ANCHOR, false, &pItem ))
    {
        oSet.emplace( GetDoc()->GetAttrPool(), aFrameFormatSetRange );
        oSet->Put( *pItem );
        if( !sw_ChkAndSetNewAnchor( *pFly, *oSet ))
        {
            oSet.reset();
        }
    }

    if( GetDoc()->SetFrameFormatToFly( *pFlyFormat, *pNewFormat, oSet ? &*oSet : nullptr, bKeepOrient ))
    {
        SwFlyFrame* pFrame = pFlyFormat->GetFrame( &aPt );
        if( pFrame )
            SelectFlyFrame( *pFrame );
        else
            GetLayout()->SetAssertFlyPages();
    }
    oSet.reset();

    EndAllActionAndCall();
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
        return pFly->getFrameArea();
}

SwRect SwFEShell::GetObjRect() const
{
    if( Imp()->HasDrawView() )
        return SwRect(Imp()->GetDrawView()->GetAllMarkedRect());
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

    aResult = pFly->getFramePrintArea().SSize();

    bool bPosProt = pFly->GetFormat()->GetProtect().IsPosProtected();
    bool bSizeProt = pFly->GetFormat()->GetProtect().IsSizeProtected();

    StartAllAction();

    // MA we do not allow to clip the Fly, as the OLE server can
    // request various wishes. Clipping is done via the formatting.
    // Correct display is done by scaling.
    // Scaling is done by SwNoTextFrame::Format by calling
    // SwWrtShell::CalcAndSetScale()
    if ( rRect.SSize() != pFly->getFramePrintArea().SSize() && !bSizeProt )
    {
        Size aSz( rRect.SSize() );

        //JP 28.02.2001: Task 74707 - ask for fly in fly with automatic size

        const SwFrame* pAnchor;
        const SwFormatFrameSize& rFrameSz = pFly->GetFormat()->GetFrameSize();
        if (m_bCheckForOLEInCaption &&
            0 != rFrameSz.GetWidthPercent() &&
            nullptr != (pAnchor = pFly->GetAnchorFrame()) &&
            pAnchor->IsTextFrame() &&
            !pAnchor->GetNext() && !pAnchor->GetPrev() &&
            pAnchor->GetUpper()->IsFlyFrame())
        {
            // search for a sequence field:
            sw::MergedAttrIter iter(*static_cast<SwTextFrame const*>(pAnchor));
            for (SwTextAttr const* pHint = iter.NextAttr(); pHint; pHint = iter.NextAttr())
            {
                const SfxPoolItem* pItem = &pHint->GetAttr();
                if( RES_TXTATR_FIELD == pItem->Which()
                    && SwFieldTypesEnum::Sequence == static_cast<const SwFormatField*>(pItem)->GetField()->GetTypeId() )
                {
                    // sequence field found
                    SwFlyFrame* pChgFly = const_cast<SwFlyFrame*>(static_cast<const SwFlyFrame*>(pAnchor->GetUpper()));
                    // calculate the changed size:
                    // width must change, height can change
                    Size aNewSz( aSz.Width() + pChgFly->getFrameArea().Width() -
                                   pFly->getFramePrintArea().Width(), aSz.Height() );

                    SwFrameFormat *pFormat = pChgFly->GetFormat();
                    SwFormatFrameSize aFrameSz( pFormat->GetFrameSize() );
                    aFrameSz.SetWidth( aNewSz.Width() );
                    if( SwFrameSize::Minimum != aFrameSz.GetHeightSizeType() )
                    {
                        aNewSz.AdjustHeight(pChgFly->getFrameArea().Height() -
                                               pFly->getFramePrintArea().Height() );
                        if( std::abs( aNewSz.Height() - pChgFly->getFrameArea().Height()) > 1 )
                            aFrameSz.SetHeight( aNewSz.Height() );
                    }
                    // via Doc for the Undo!
                    pFormat->GetDoc()->SetAttr( aFrameSz, *pFormat );
                    break;
                }
            }
        }

        // set the new Size at the fly themself
        if ( !pFly->getFramePrintArea().IsEmpty() )
        {
            aSz.AdjustWidth(pFly->getFrameArea().Width() - pFly->getFramePrintArea().Width() );
            aSz.AdjustHeight(pFly->getFrameArea().Height()- pFly->getFramePrintArea().Height() );
        }
        aResult = pFly->ChgSize( aSz );

        // if the object changes, the contour is outside the object
        assert(pFly->Lower()->IsNoTextFrame());
        SwNoTextNode *pNd = static_cast<SwNoTextFrame*>(pFly->Lower())->GetNode()->GetNoTextNode();
        assert(pNd);
        pNd->SetContour( nullptr );
        ClrContourCache();
    }

    // if only the size is to be adjusted, a position is transported with
    // allocated values
    Point aPt( pFly->getFramePrintArea().Pos() );
    aPt += pFly->getFrameArea().Pos();
    if ( rRect.Top() != LONG_MIN && rRect.Pos() != aPt && !bPosProt )
    {
        aPt = rRect.Pos();
        aPt.setX(aPt.getX() - pFly->getFramePrintArea().Left());
        aPt.setY(aPt.getY() - pFly->getFramePrintArea().Top());

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
            const tools::Long lXDiff = aPt.getX() - pFly->getFrameArea().Left();
            const tools::Long lYDiff = aPt.getY() - pFly->getFrameArea().Top();
            const Point aTmp( rHori.GetPos() + lXDiff,
                              rVert.GetPos() + lYDiff );
            pFly->ChgRelPos( aTmp );
        }
    }

    SwFlyFrameFormat *pFlyFrameFormat = pFly->GetFormat();
    OSL_ENSURE( pFlyFrameFormat, "fly frame format missing!" );
    if ( pFlyFrameFormat )
        pFlyFrameFormat->SetLastFlyFramePrtRectPos( pFly->getFramePrintArea().Pos() ); //stores the value of last Prt rect

    EndAllAction();

    return aResult;
}

SwFrameFormat* SwFEShell::WizardGetFly()
{
    // do not search the Fly via the layout. Now we can delete a frame
    // without a valid layout. ( e.g. for the wizards )
    sw::SpzFrameFormats& rSpzArr = *mxDoc->GetSpzFrameFormats();
    if( !rSpzArr.empty() )
    {
        SwNode& rCursorNd = GetCursor()->GetPoint()->GetNode();
        if( rCursorNd > mxDoc->GetNodes().GetEndOfExtras() )
            // Cursor is in the body area!
            return nullptr;

        for( auto pFormat : rSpzArr )
        {
            const SwNodeIndex* pIdx = pFormat->GetContent( false ).GetContentIdx();
            SwStartNode* pSttNd;
            if( pIdx &&
                nullptr != ( pSttNd = pIdx->GetNode().GetStartNode() ) &&
                *pSttNd < rCursorNd &&
                rCursorNd < *pSttNd->EndOfSectionNode() )
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

uno::Reference < embed::XEmbeddedObject > SwFEShell::GetOleRef() const
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

    SdrPageView* pPV;
    const SwFrameFormat* pRet = nullptr;
    SwDrawView *pDView = const_cast<SwDrawView*>(Imp()->GetDrawView());

    const auto nOld = pDView->GetHitTolerancePixel();
    pDView->SetHitTolerancePixel( 2 );

    SdrObject* pObj = pDView->PickObj(rPt, pDView->getHitTolLog(), pPV, SdrSearchOptions::PICKMACRO);
    SwVirtFlyDrawObj* pFlyObj = dynamic_cast<SwVirtFlyDrawObj*>(pObj);
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
                        aPt -= pFly->getFrameArea().Pos();
                        // without MapMode-Offset, without Offset, o ... !!!!!
                        aPt = GetOut()->LogicToPixel(
                                aPt, MapMode( MapUnit::MapTwip ) );
                        *pURL = *pURL + "?" + OUString::number( aPt.getX() )
                                + "," + OUString::number(aPt.getY() );
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

    SdrPageView* pPV;
    SwDrawView *pDView = const_cast<SwDrawView*>(Imp()->GetDrawView());

    SdrObject* pObj = pDView->PickObj(rPt, pDView->getHitTolLog(), pPV);
    SwVirtFlyDrawObj* pFlyObj = dynamic_cast<SwVirtFlyDrawObj*>(pObj);
    if (pFlyObj)
    {
        SwFlyFrame *pFly = pFlyObj->GetFlyFrame();
        if ( pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
        {
            SwGrfNode *const pNd = static_cast<SwNoTextFrame*>(pFly->Lower())->GetNode()->GetGrfNode();
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
        SdrPageView* pPView;

        SwDrawView *pDView = const_cast<SwDrawView*>(Imp()->GetDrawView());

        const auto nOld = pDView->GetHitTolerancePixel();
        // tolerance for Drawing-SS
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        SdrObject* pObj = pDView->PickObj(rPt, pDView->getHitTolLog(), pPView, SdrSearchOptions::PICKMARKABLE);
        if (pObj)
        {
           // first check it:
            if (SwVirtFlyDrawObj* pFlyObj = dynamic_cast<SwVirtFlyDrawObj*>(pObj))
                pRet = pFlyObj->GetFormat();
            else if ( pObj->GetUserCall() ) //not for group objects
                pRet = static_cast<SwDrawContact*>(pObj->GetUserCall())->GetFormat();
            if(pRet && pRectToFill)
                **pRectToFill = SwRect(pObj->GetCurrentBoundRect());
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
        GetLayout()->GetModelPositionForViewPoint( &aPos, aPt );
        SwContentNode *pNd = aPos.GetNode().GetContentNode();
        std::pair<Point, bool> const tmp(rPt, false);
        SwFrame* pFrame = pNd->getLayoutFrame(GetLayout(), nullptr, &tmp)->FindFlyFrame();
        pRet = pFrame ? static_cast<SwLayoutFrame*>(pFrame)->GetFormat() : nullptr;
    }
    return pRet;
}

ObjCntType SwFEShell::GetObjCntType( const SdrObject& rObj )
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

    if( SdrInventor::FmForm == pInvestigatedObj->GetObjInventor() )
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
                aVal = xSet->getPropertyValue( sName );
                if( aVal.hasValue() && form::FormButtonType_URL == *o3tl::doAccess<form::FormButtonType>(aVal) )
                    eType = OBJCNT_URLBUTTON;
            }
        }
    }
    else if (const SwVirtFlyDrawObj *pFlyObj = dynamic_cast<const SwVirtFlyDrawObj*>(pInvestigatedObj))
    {
        const SwFlyFrame *pFly = pFlyObj->GetFlyFrame();
        if ( pFly->Lower() && pFly->Lower()->IsNoTextFrame() )
        {
            if (static_cast<const SwNoTextFrame*>(pFly->Lower())->GetNode()->GetGrfNode())
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
            else if ( RndStdIds::FLY_AS_CHAR != pFrameFormat->GetAnchor().GetAnchorId() )
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
        SdrPageView* pPView;

        SwDrawView *pDView = const_cast<SwDrawView*>(Imp()->GetDrawView());

        const auto nOld = pDView->GetHitTolerancePixel();
        // tolerance for Drawing-SS
        pDView->SetHitTolerancePixel( pDView->GetMarkHdlSizePixel()/2 );

        SdrObject* pObj = pDView->PickObj(rPt, pDView->getHitTolLog(), pPView, SdrSearchOptions::PICKMARKABLE);
        if (pObj)
        {
            rpObj = pObj;
            eType = GetObjCntType( *rpObj );
        }

        pDView->SetHitTolerancePixel( nOld );
    }
    return eType;
}

ObjCntType SwFEShell::GetObjCntTypeOfSelection() const
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

void SwFEShell::ReplaceSdrObj( const OUString& rGrfName, const Graphic* pGrf )
{
    CurrShell aCurr( this );

    const SdrMarkList *pMrkList;
    if( !(Imp()->HasDrawView() &&  1 ==
        ( pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList())->GetMarkCount()) )
        return;

    SdrObject* pObj = pMrkList->GetMark( 0 )->GetMarkedSdrObj();
    assert(pObj);
    SwFrameFormat *pFormat = FindFrameFormat( pObj );

    // store attributes, then set the graphic
    SfxItemSet aFrameSet( mxDoc->GetAttrPool(),
                        pFormat->GetAttrSet().GetRanges() );
    aFrameSet.Set( pFormat->GetAttrSet() );

    // set size and position?
    if( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) == nullptr )
    {
        // then let's do it:
        const tools::Rectangle &rBound = pObj->GetSnapRect();
        Point aRelPos( pObj->GetRelativePos() );

        const tools::Long nWidth = rBound.Right()  - rBound.Left();
        const tools::Long nHeight= rBound.Bottom() - rBound.Top();
        aFrameSet.Put( SwFormatFrameSize( SwFrameSize::Minimum,
                            std::max( nWidth,  tools::Long(MINFLY) ),
                            std::max( nHeight, tools::Long(MINFLY) )));

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

    GetDoc()->getIDocumentContentOperations().InsertGraphic(
        *GetCursor(), rGrfName, "", pGrf, &aFrameSet, nullptr, nullptr);

    EndUndo();
    EndAllAction();
}

static sal_uInt16 SwFormatGetPageNum(const SwFlyFrameFormat * pFormat)
{
    assert(pFormat != nullptr && "invalid argument");

    SwFlyFrame * pFrame = pFormat->GetFrame();

    sal_uInt16 aResult;

    if (pFrame != nullptr)
        aResult = pFrame->GetPhyPageNum();
    else
        aResult = pFormat->GetAnchor().GetPageNum();

    return aResult;
}

void SwFEShell::GetConnectableFrameFormats(SwFrameFormat & rFormat,
                                      std::u16string_view rReference,
                                      bool bSuccessors,
                                      std::vector< OUString > & aPrevPageVec,
                                      std::vector< OUString > & aThisPageVec,
                                      std::vector< OUString > & aNextPageVec,
                                      std::vector< OUString > & aRestVec)
{
    StartAction();

    SwFormatChain rChain = rFormat.GetChain();
    SwFrameFormat * pOldChainNext = rChain.GetNext();
    SwFrameFormat * pOldChainPrev = rChain.GetPrev();

    if (pOldChainNext)
        mxDoc->Unchain(rFormat);

    if (pOldChainPrev)
        mxDoc->Unchain(*pOldChainPrev);

    /* potential successors resp. predecessors */
    std::vector<const SwFrameFormat *> aTmpSpzArray = mxDoc->GetFlyFrameFormats(FLYCNTTYPE_FRM, false);

    for (auto it = aTmpSpzArray.begin(); it != aTmpSpzArray.end(); ++it)
    {
        /*
           pFormat is a potential successor of rFormat if it is chainable after
           rFormat.

           pFormat is a potential predecessor of rFormat if rFormat is chainable
           after pFormat.
        */

        SwChainRet nChainState;

        if (bSuccessors)
            nChainState = mxDoc->Chainable(rFormat, **it);
        else
            nChainState = mxDoc->Chainable(**it, rFormat);

        if (nChainState != SwChainRet::OK)
            *it = nullptr;
    }
    std::erase(aTmpSpzArray, nullptr);

    if  (!aTmpSpzArray.empty())
    {
        aPrevPageVec.clear();
        aThisPageVec.clear();
        aNextPageVec.clear();
        aRestVec.clear();

        /* number of page rFormat resides on */
        sal_uInt16 nPageNum = SwFormatGetPageNum(static_cast<SwFlyFrameFormat *>(&rFormat));

        for (const auto& rpFormat : aTmpSpzArray)
        {
            const OUString aString = rpFormat->GetName();

            /* rFormat is not a valid successor or predecessor of
               itself */
            if (aString != rReference && aString != rFormat.GetName())
            {
                sal_uInt16 nNum1 =
                    SwFormatGetPageNum(static_cast<const SwFlyFrameFormat *>(rpFormat));

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
        mxDoc->Chain(rFormat, *pOldChainNext);

    if (pOldChainPrev)
        mxDoc->Chain(*pOldChainPrev, rFormat);

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
    if ( !Imp()->HasDrawView() )
        return;

    const SdrMarkList *pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
    if ( pMrkList->GetMarkCount() != 1 )
        return;

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
    if ( !Imp()->HasDrawView() )
        return;

    const SdrMarkList *pMrkList = &Imp()->GetDrawView()->GetMarkedObjectList();
    if ( pMrkList->GetMarkCount() != 1 )
        return;

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

bool SwFEShell::IsObjDecorative() const
{
    if (!Imp()->HasDrawView())
    {
        return false;
    }

    SdrMarkList const& rMarkList(Imp()->GetDrawView()->GetMarkedObjectList());
    if (rMarkList.GetMarkCount() != 1)
    {
        return false;
    }

    SdrObject const*const pObj(rMarkList.GetMark(0)->GetMarkedSdrObj());
    SwFrameFormat const*const pFormat(FindFrameFormat(pObj));
    if (pFormat->Which() == RES_FLYFRMFMT)
    {
        return dynamic_cast<const SwFlyFrameFormat&>(*pFormat).GetAttrSet().Get(RES_DECORATIVE).GetValue();
    }
    return pObj->IsDecorative();
}

void SwFEShell::SetObjDecorative(bool const isDecorative)
{
    if (!Imp()->HasDrawView())
    {
        return;
    }

    SdrMarkList const& rMarkList(Imp()->GetDrawView()->GetMarkedObjectList());
    if (rMarkList.GetMarkCount() != 1)
    {
        return;
    }

    SdrObject *const pObj(rMarkList.GetMark(0)->GetMarkedSdrObj());
    SwFrameFormat *const pFormat(FindFrameFormat(pObj));
    if (pFormat->Which() == RES_FLYFRMFMT)
    {
        GetDoc()->SetFlyFrameDecorative(dynamic_cast<SwFlyFrameFormat&>(*pFormat),
                                       isDecorative);
    }
    else
    {
        pObj->SetDecorative(isDecorative);
    }
}


void SwFEShell::AlignFormulaToBaseline( const uno::Reference < embed::XEmbeddedObject >& xObj )
{
#if OSL_DEBUG_LEVEL > 0
    SvGlobalName aCLSID( xObj->getClassID() );
    const bool bStarMath = ( SotExchange::IsMath( aCLSID ) != 0 );
    OSL_ENSURE( bStarMath, "AlignFormulaToBaseline should only be called for Math objects" );

    if ( !bStarMath )
        return;
#endif

    SwFlyFrame * pFly = FindFlyFrame( xObj );
    OSL_ENSURE( pFly , "No fly frame!" );
    SwFrameFormat * pFrameFormat = pFly ? pFly->GetFormat() : nullptr;

    // baseline to baseline alignment should only be applied to formulas anchored as char
    if ( !pFly || !pFrameFormat || RndStdIds::FLY_AS_CHAR != pFrameFormat->GetAnchor().GetAnchorId() )
        return;

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
    nBaseline = o3tl::toTwips( nBaseline, o3tl::Length::mm100 );

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

void SwFEShell::AlignAllFormulasToBaseline()
{
    StartAllAction();

    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while ( nullptr != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        SwOLENode *pOleNode = aIdx.GetNode().GetOLENode();
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
