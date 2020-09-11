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

#include <dcontact.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <tocntntanchoredobjectposition.hxx>
#include <tolayoutanchoredobjectposition.hxx>
#include <frmtool.hxx>
#include <fmtornt.hxx>
#include <txtfrm.hxx>
#include <vector>
#include <svx/svdogrp.hxx>
#include <tools/fract.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentState.hxx>
#include <txtfly.hxx>
#include <viewimp.hxx>
#include <textboxhelper.hxx>
#include <unomid.h>
#include <svx/svdoashp.hxx>

using namespace ::com::sun::star;

namespace {

/// helper class for correct notification due to the positioning of
/// the anchored drawing object
class SwPosNotify
{
    private:
        SwAnchoredDrawObject* mpAnchoredDrawObj;
        SwRect maOldObjRect;
        SwPageFrame* mpOldPageFrame;

    public:
        explicit SwPosNotify( SwAnchoredDrawObject* _pAnchoredDrawObj );
        ~SwPosNotify() COVERITY_NOEXCEPT_FALSE;
        // #i32795#
        Point const & LastObjPos() const;
};

}

SwPosNotify::SwPosNotify( SwAnchoredDrawObject* _pAnchoredDrawObj ) :
    mpAnchoredDrawObj( _pAnchoredDrawObj )
{
    maOldObjRect = mpAnchoredDrawObj->GetObjRect();
    // --> #i35640# - determine correct page frame
    mpOldPageFrame = mpAnchoredDrawObj->GetPageFrame();
}

SwPosNotify::~SwPosNotify() COVERITY_NOEXCEPT_FALSE
{
    if ( maOldObjRect != mpAnchoredDrawObj->GetObjRect() )
    {
        if( maOldObjRect.HasArea() && mpOldPageFrame )
        {
            mpAnchoredDrawObj->NotifyBackground( mpOldPageFrame, maOldObjRect,
                                                 PrepareHint::FlyFrameLeave );
        }
        SwRect aNewObjRect( mpAnchoredDrawObj->GetObjRect() );
        if( aNewObjRect.HasArea() )
        {
            // --> #i35640# - determine correct page frame
            SwPageFrame* pNewPageFrame = mpAnchoredDrawObj->GetPageFrame();
            if( pNewPageFrame )
                mpAnchoredDrawObj->NotifyBackground( pNewPageFrame, aNewObjRect,
                                                     PrepareHint::FlyFrameArrive );
        }

        ::ClrContourCache( mpAnchoredDrawObj->GetDrawObj() );

        // --> #i35640# - additional notify anchor text frame
        // Needed for negative positioned drawing objects
        // --> #i43255# - refine condition to avoid unneeded
        // invalidations: anchored object had to be on the page of its anchor
        // text frame.
        if ( mpAnchoredDrawObj->GetAnchorFrame()->IsTextFrame() &&
             mpOldPageFrame == mpAnchoredDrawObj->GetAnchorFrame()->FindPageFrame() )
        {
            mpAnchoredDrawObj->AnchorFrame()->Prepare( PrepareHint::FlyFrameLeave );
        }

        // indicate a restart of the layout process
        mpAnchoredDrawObj->SetRestartLayoutProcess( true );
    }
    else
    {
        // lock position
        mpAnchoredDrawObj->LockPosition();

        if ( !mpAnchoredDrawObj->ConsiderForTextWrap() )
        {
            // indicate that object has to be considered for text wrap
            mpAnchoredDrawObj->SetConsiderForTextWrap( true );
            // invalidate 'background' in order to allow its 'background'
            // to wrap around it.
            mpAnchoredDrawObj->NotifyBackground( mpAnchoredDrawObj->GetPageFrame(),
                                    mpAnchoredDrawObj->GetObjRectWithSpaces(),
                                    PrepareHint::FlyFrameArrive );
            // invalidate position of anchor frame in order to force
            // a re-format of the anchor frame, which also causes a
            // re-format of the invalid previous frames of the anchor frame.
            mpAnchoredDrawObj->AnchorFrame()->InvalidatePos();
        }
    }
    // tdf#101464 notify SwAccessibleMap about new drawing object position
    if (mpOldPageFrame && mpOldPageFrame->getRootFrame()->IsAnyShellAccessible())
    {
        mpOldPageFrame->getRootFrame()->GetCurrShell()->Imp()->MoveAccessible(
                nullptr, mpAnchoredDrawObj->GetDrawObj(), maOldObjRect);
    }
}

// --> #i32795#
Point const & SwPosNotify::LastObjPos() const
{
    return maOldObjRect.Pos();
}

namespace {

// #i32795#
/// helper class for oscillation control on object positioning
class SwObjPosOscillationControl
{
    private:
        const SwAnchoredDrawObject* mpAnchoredDrawObj;

        std::vector<Point> maObjPositions;

    public:
        explicit SwObjPosOscillationControl( const SwAnchoredDrawObject& _rAnchoredDrawObj );

        bool OscillationDetected();
};

}

SwObjPosOscillationControl::SwObjPosOscillationControl(
                                const SwAnchoredDrawObject& _rAnchoredDrawObj )
    : mpAnchoredDrawObj( &_rAnchoredDrawObj )
{
}

bool SwObjPosOscillationControl::OscillationDetected()
{
    bool bOscillationDetected = false;

    if ( maObjPositions.size() == 20 )
    {
        // position stack is full -> oscillation
        bOscillationDetected = true;
    }
    else
    {
        Point aNewObjPos = mpAnchoredDrawObj->GetObjRect().Pos();
        for ( auto const & pt : maObjPositions )
        {
            if ( aNewObjPos == pt )
            {
                // position already occurred -> oscillation
                bOscillationDetected = true;
                break;
            }
        }
        if ( !bOscillationDetected )
        {
            maObjPositions.push_back( aNewObjPos );
        }
    }

    return bOscillationDetected;
}


SwAnchoredDrawObject::SwAnchoredDrawObject() :
    SwAnchoredObject(),
    mbValidPos( false ),
    mbNotYetAttachedToAnchorFrame( true ),
    // --> #i28749#
    mbNotYetPositioned( true ),
    // --> #i62875#
    mbCaptureAfterLayoutDirChange( false )
{
}

SwAnchoredDrawObject::~SwAnchoredDrawObject()
{
}

// --> #i62875#
void SwAnchoredDrawObject::UpdateLayoutDir()
{
    SwFrameFormat::tLayoutDir nOldLayoutDir( GetFrameFormat().GetLayoutDir() );

    SwAnchoredObject::UpdateLayoutDir();

    if ( !NotYetPositioned() &&
         GetFrameFormat().GetLayoutDir() != nOldLayoutDir &&
         GetFrameFormat().GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE) &&
         !IsOutsidePage() )
    {
        mbCaptureAfterLayoutDirChange = true;
    }
}

// --> #i62875#
bool SwAnchoredDrawObject::IsOutsidePage() const
{
    bool bOutsidePage( false );

    if ( !NotYetPositioned() && GetPageFrame() )
    {
        SwRect aTmpRect( GetObjRect() );
        bOutsidePage =
            ( aTmpRect.Intersection( GetPageFrame()->getFrameArea() ) != GetObjRect() );
    }

    return bOutsidePage;
}

void SwAnchoredDrawObject::MakeObjPos()
{
    if ( IsPositioningInProgress() )
    {
        // nothing to do - positioning already in progress
        return;
    }

    if ( mbValidPos )
    {
        // nothing to do - position is valid
        return;
    }

    // --> #i28749# - anchored drawing object has to be attached
    // to anchor frame
    if ( mbNotYetAttachedToAnchorFrame )
    {
        OSL_FAIL( "<SwAnchoredDrawObject::MakeObjPos() - drawing object not yet attached to anchor frame -> no positioning" );
        return;
    }

    SwDrawContact* pDrawContact =
                        static_cast<SwDrawContact*>(::GetUserCall( GetDrawObj() ));

    // --> #i28749# - if anchored drawing object hasn't been yet
    // positioned, convert its positioning attributes, if its positioning
    // attributes are given in horizontal left-to-right layout.
    // --> #i36010# - Note: horizontal left-to-right layout is made
    // the default layout direction for <SwDrawFrameFormat> instances. Thus, it has
    // to be adjusted manually, if no adjustment of the positioning attributes
    // have to be performed here.
    // --> #i35635# - additionally move drawing object to the visible layer.
    if ( mbNotYetPositioned )
    {
        // --> #i35635#
        pDrawContact->MoveObjToVisibleLayer( DrawObj() );
        // --> perform conversion of positioning
        // attributes only for 'master' drawing objects
        // #i44334#, #i44681# - check, if positioning
        // attributes already have been set.
        if ( dynamic_cast< const SwDrawVirtObj* >(GetDrawObj()) ==  nullptr &&
             !static_cast<SwDrawFrameFormat&>(GetFrameFormat()).IsPosAttrSet() )
        {
            SetPositioningAttr();
        }
        // -->
        // - reset internal flag after all needed actions are performed to
        //   avoid callbacks from drawing layer
        mbNotYetPositioned = false;
    }

    // indicate that positioning is in progress
    {
        SwObjPositioningInProgress aObjPosInProgress( *this );

        // determine relative position of drawing object and set it
        switch ( pDrawContact->GetAnchorId() )
        {
            case RndStdIds::FLY_AS_CHAR:
            {
                // indicate that position will be valid after positioning is performed
                mbValidPos = true;
                // nothing to do, because as-character anchored objects are positioned
                // during the format of its anchor frame - see <SwFlyCntPortion::SetBase(..)>
            }
            break;
            case RndStdIds::FLY_AT_PARA:
            case RndStdIds::FLY_AT_CHAR:
            {
                // --> #i32795# - move intrinsic positioning to
                // helper method <MakeObjPosAnchoredAtPara()>
                MakeObjPosAnchoredAtPara();
            }
            break;
            case RndStdIds::FLY_AT_PAGE:
            case RndStdIds::FLY_AT_FLY:
            {
                // --> #i32795# - move intrinsic positioning to
                // helper method <MakeObjPosAnchoredAtLayout()>
                MakeObjPosAnchoredAtLayout();
            }
            break;
            default:
            {
                assert(!"<SwAnchoredDrawObject::MakeObjPos()> - unknown anchor type.");
            }
        }

        // keep, current object rectangle
        // --> #i34748# - use new method <SetLastObjRect(..)>
        SetLastObjRect( GetObjRect().SVRect() );

        // Assure for 'master' drawing object, that it's registered at the correct page.
        // Perform check not for as-character anchored drawing objects and only if
        // the anchor frame is valid.
        if ( dynamic_cast< const SwDrawVirtObj* >(GetDrawObj()) ==  nullptr &&
             !pDrawContact->ObjAnchoredAsChar() &&
             GetAnchorFrame()->isFrameAreaDefinitionValid() )
        {
            pDrawContact->ChkPage();
        }
    }

    // --> #i62875#
    if ( !(mbCaptureAfterLayoutDirChange &&
         GetPageFrame()) )
        return;

    SwRect aPageRect( GetPageFrame()->getFrameArea() );
    SwRect aObjRect( GetObjRect() );
    if ( aObjRect.Right() >= aPageRect.Right() + 10 )
    {
        Size aSize( aPageRect.Right() - aObjRect.Right(), 0 );
        DrawObj()->Move( aSize );
        aObjRect = GetObjRect();
    }

    if ( aObjRect.Left() + 10 <= aPageRect.Left() )
    {
        Size aSize( aPageRect.Left() - aObjRect.Left(), 0 );
        DrawObj()->Move( aSize );
    }

    mbCaptureAfterLayoutDirChange = false;
}

/** method for the intrinsic positioning of an at-paragraph|at-character
    anchored drawing object

    #i32795# - helper method for method <MakeObjPos>
*/
void SwAnchoredDrawObject::MakeObjPosAnchoredAtPara()
{
    // --> #i32795# - adopt positioning algorithm from Writer
    // fly frames, which are anchored at paragraph|at character

    // Determine, if anchor frame can/has to be formatted.
    // If yes, after each object positioning the anchor frame is formatted.
    // If after the anchor frame format the object position isn't valid, the
    // object is positioned again.
    // --> #i43255# - refine condition: anchor frame format not
    // allowed, if another anchored object, has to be consider its wrap influence
    // --> #i50356# - format anchor frame containing the anchor
    // position. E.g., for at-character anchored object this can be the follow
    // frame of the anchor frame, which contains the anchor character.
    bool bJoinLocked
        = static_cast<const SwTextFrame*>(GetAnchorFrameContainingAnchPos())->IsAnyJoinLocked();
    const bool bFormatAnchor = !bJoinLocked && !ConsiderObjWrapInfluenceOnObjPos()
                               && !ConsiderObjWrapInfluenceOfOtherObjs();

    // Format of anchor is needed for (vertical) fly offsets, otherwise the
    // lack of fly portions will result in an incorrect 0 offset.
    bool bAddVerticalFlyOffsets = GetFrameFormat().getIDocumentSettingAccess().get(
        DocumentSettingId::ADD_VERTICAL_FLY_OFFSETS);
    bool bFormatAnchorOnce = !bJoinLocked && bAddVerticalFlyOffsets;

    if (bFormatAnchor || bFormatAnchorOnce)
    {
        // --> #i50356#
        GetAnchorFrameContainingAnchPos()->Calc(GetAnchorFrameContainingAnchPos()->getRootFrame()->GetCurrShell()->GetOut());
    }

    bool bOscillationDetected = false;
    SwObjPosOscillationControl aObjPosOscCtrl( *this );
    // --> #i3317# - boolean, to apply temporarily the
    // 'straightforward positioning process' for the frame due to its
    // overlapping with a previous column.
    bool bConsiderWrapInfluenceDueToOverlapPrevCol( false );
    do {
        // indicate that position will be valid after positioning is performed
        mbValidPos = true;

        // --> #i35640# - correct scope for <SwPosNotify> instance
        {
            // create instance of <SwPosNotify> for correct notification
            SwPosNotify aPosNotify( this );

            // determine and set position
            objectpositioning::SwToContentAnchoredObjectPosition
                    aObjPositioning( *DrawObj() );
            aObjPositioning.CalcPosition();

            // get further needed results of the positioning algorithm
            SetVertPosOrientFrame ( aObjPositioning.GetVertPosOrientFrame() );
            SetDrawObjAnchor();

            // check for object position oscillation, if position has changed.
            if ( GetObjRect().Pos() != aPosNotify.LastObjPos() )
            {
                bOscillationDetected = aObjPosOscCtrl.OscillationDetected();
            }
        }
        // format anchor frame, if requested.
        // Note: the format of the anchor frame can cause the object position
        // to be invalid.
        if ( bFormatAnchor )
        {
            // --> #i50356#
            GetAnchorFrameContainingAnchPos()->Calc(GetAnchorFrameContainingAnchPos()->getRootFrame()->GetCurrShell()->GetOut());
        }

        // --> #i3317#
        if ( !ConsiderObjWrapInfluenceOnObjPos() &&
             OverlapsPrevColumn() )
        {
            bConsiderWrapInfluenceDueToOverlapPrevCol = true;
        }
    } while ( !mbValidPos && !bOscillationDetected &&
              !bConsiderWrapInfluenceDueToOverlapPrevCol );

    // --> #i3317# - consider a detected oscillation and overlapping
    // with previous column.
    // temporarily consider the anchored objects wrapping style influence
    if ( bOscillationDetected || bConsiderWrapInfluenceDueToOverlapPrevCol )
    {
        SetTmpConsiderWrapInfluence( true );
        SetRestartLayoutProcess( true );
    }
}

/** method for the intrinsic positioning of an at-page|at-frame anchored
    drawing object

    #i32795# - helper method for method <MakeObjPos>
*/
void SwAnchoredDrawObject::MakeObjPosAnchoredAtLayout()
{
    // indicate that position will be valid after positioning is performed
    mbValidPos = true;

    // create instance of <SwPosNotify> for correct notification
    SwPosNotify aPosNotify( this );

    // determine position
    objectpositioning::SwToLayoutAnchoredObjectPosition
            aObjPositioning( *DrawObj() );
    aObjPositioning.CalcPosition();

    // set position

    // --> #i31698#
    // --> #i34995# - setting anchor position needed for filters,
    // especially for the xml-filter to the OpenOffice.org file format
    {
        const Point aNewAnchorPos =
                    GetAnchorFrame()->GetFrameAnchorPos( ::HasWrap( GetDrawObj() ) );
        DrawObj()->SetAnchorPos( aNewAnchorPos );
        // --> #i70122# - missing invalidation
        InvalidateObjRectWithSpaces();
    }
    SetCurrRelPos( aObjPositioning.GetRelPos() );
    const SwFrame* pAnchorFrame = GetAnchorFrame();
    SwRectFnSet aRectFnSet(pAnchorFrame);
    const Point aAnchPos( aRectFnSet.GetPos(pAnchorFrame->getFrameArea()) );
    SetObjLeft( aAnchPos.X() + GetCurrRelPos().X() );
    SetObjTop( aAnchPos.Y() + GetCurrRelPos().Y() );
}

void SwAnchoredDrawObject::SetDrawObjAnchor()
{
    // new anchor position
    // --> #i31698# -
    Point aNewAnchorPos =
                GetAnchorFrame()->GetFrameAnchorPos( ::HasWrap( GetDrawObj() ) );
    Point aCurrAnchorPos = GetDrawObj()->GetAnchorPos();
    if ( aNewAnchorPos != aCurrAnchorPos )
    {
        // determine movement to be applied after setting the new anchor position
        Size aMove( aCurrAnchorPos.getX() - aNewAnchorPos.getX(),
                    aCurrAnchorPos.getY() - aNewAnchorPos.getY() );
        // set new anchor position
        DrawObj()->SetAnchorPos( aNewAnchorPos );
        // correct object position, caused by setting new anchor position
        DrawObj()->Move( aMove );
        // --> #i70122# - missing invalidation
        InvalidateObjRectWithSpaces();
    }
}

/** method to invalidate the given page frame

    #i28701#
*/
void SwAnchoredDrawObject::InvalidatePage_( SwPageFrame* _pPageFrame )
{
    if ( !_pPageFrame || _pPageFrame->GetFormat()->GetDoc()->IsInDtor() )
        return;

    if ( !_pPageFrame->GetUpper() )
        return;

    // --> #i35007# - correct invalidation for as-character
    // anchored objects.
    if ( GetFrameFormat().GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR )
    {
        _pPageFrame->InvalidateFlyInCnt();
    }
    else
    {
        _pPageFrame->InvalidateFlyLayout();
    }

    SwRootFrame* pRootFrame = static_cast<SwRootFrame*>(_pPageFrame->GetUpper());
    pRootFrame->DisallowTurbo();
    if ( pRootFrame->GetTurbo() )
    {
        const SwContentFrame* pTmpFrame = pRootFrame->GetTurbo();
        pRootFrame->ResetTurbo();
        pTmpFrame->InvalidatePage();
    }
    pRootFrame->SetIdleFlags();
}

void SwAnchoredDrawObject::InvalidateObjPos()
{
    // --> #i28701# - check, if invalidation is allowed
    if ( !(mbValidPos &&
         InvalidationOfPosAllowed()) )
        return;

    mbValidPos = false;
    // --> #i68520#
    InvalidateObjRectWithSpaces();

    // --> #i44339# - check, if anchor frame exists.
    if ( !GetAnchorFrame() )
        return;

    // --> #118547# - notify anchor frame of as-character
    // anchored object, because its positioned by the format of its anchor frame.
    // --> #i44559# - assure, that text hint is already
    // existing in the text frame
    if ( dynamic_cast< const SwTextFrame* >(GetAnchorFrame()) !=  nullptr &&
         (GetFrameFormat().GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR) )
    {
        SwTextFrame* pAnchorTextFrame( static_cast<SwTextFrame*>(AnchorFrame()) );
        if (pAnchorTextFrame->CalcFlyPos(&GetFrameFormat()) != TextFrameIndex(COMPLETE_STRING))
        {
            AnchorFrame()->Prepare( PrepareHint::FlyFrameAttributesChanged, &GetFrameFormat() );
        }
    }

    SwPageFrame* pPageFrame = AnchorFrame()->FindPageFrame();
    InvalidatePage_( pPageFrame );

    // --> #i32270# - also invalidate page frame, at which the
    // drawing object is registered at.
    SwPageFrame* pPageFrameRegisteredAt = GetPageFrame();
    if ( pPageFrameRegisteredAt &&
         pPageFrameRegisteredAt != pPageFrame )
    {
        InvalidatePage_( pPageFrameRegisteredAt );
    }
    // #i33751#, #i34060# - method <GetPageFrameOfAnchor()>
    // is replaced by method <FindPageFrameOfAnchor()>. It's return value
    // have to be checked.
    SwPageFrame* pPageFrameOfAnchor = FindPageFrameOfAnchor();
    if ( pPageFrameOfAnchor &&
         pPageFrameOfAnchor != pPageFrame &&
         pPageFrameOfAnchor != pPageFrameRegisteredAt )
    {
        InvalidatePage_( pPageFrameOfAnchor );
    }
}

SwFrameFormat& SwAnchoredDrawObject::GetFrameFormat()
{
    assert(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFormat());
    return *(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFormat());
}
const SwFrameFormat& SwAnchoredDrawObject::GetFrameFormat() const
{
    assert(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFormat());
    return *(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFormat());
}

SwRect SwAnchoredDrawObject::GetObjRect() const
{
    // use geometry of drawing object
    //return GetDrawObj()->GetCurrentBoundRect();
    return GetDrawObj()->GetSnapRect();
}

namespace
{
    // Imagine an open book, inside margin is the one that is at the inner side of the pages, at the center of the book,
    // outside margin is at the two opposite edges of the book.
    // outside --text-- inside | inside --text-- outside
    // With mirrored margins, when relating the size of an object from the inside margin for example, on the
    // first page we calculate the new size of the object using the size of the right margin,
    // on second page the left margin, third page right margin, etc.
    long getInsideOutsideRelativeWidth(bool isOutside, const SwPageFrame* const pPageFrame)
    {
        // Alternating between the only two possible cases: inside and outside.
        // Inside = false, Outside = true.
        auto nPageNum = pPageFrame->GetPhyPageNum();
        if (nPageNum % 2 == (isOutside ? 0 : 1))
            return pPageFrame->GetRightMargin();
        else
            return pPageFrame->GetLeftMargin();
    }
}

// --> #i70122#
SwRect SwAnchoredDrawObject::GetObjBoundRect() const
{
    bool bGroupShape = dynamic_cast<const SdrObjGroup*>( GetDrawObj() );
    // Resize objects with relative width or height
    if ( !bGroupShape && GetPageFrame( ) && ( GetDrawObj( )->GetRelativeWidth( ) || GetDrawObj()->GetRelativeHeight( ) ) )
    {
        tools::Rectangle aCurrObjRect = GetDrawObj()->GetCurrentBoundRect();

        long nTargetWidth = aCurrObjRect.GetWidth( );
        if ( GetDrawObj( )->GetRelativeWidth( ) )
        {
            long nWidth = 0;
            if (GetDrawObj()->GetRelativeWidthRelation() == text::RelOrientation::FRAME)
                // Exclude margins.
                nWidth = GetPageFrame()->getFramePrintArea().SVRect().GetWidth();
            // Here we handle the relative size of the width of some shape.
            // The size of the shape's width is going to be relative to the size of the left margin.
            // E.g.: (left margin = 8 && relative size = 150%) -> width of some shape = 12.
            else if (GetDrawObj()->GetRelativeWidthRelation() == text::RelOrientation::PAGE_LEFT)
            {
                if (GetPageFrame()->GetPageDesc()->GetUseOn() == UseOnPage::Mirror)
                    // We want to get the width of whatever is going through here using the size of the
                    // outside margin.
                    nWidth = getInsideOutsideRelativeWidth(true, GetPageFrame());
                else
                    nWidth = GetPageFrame()->GetLeftMargin();
            }
            // Same as the left margin above.
            else if (GetDrawObj()->GetRelativeWidthRelation() == text::RelOrientation::PAGE_RIGHT)
                if (GetPageFrame()->GetPageDesc()->GetUseOn() == UseOnPage::Mirror)
                    // We want to get the width of whatever is going through here using the size of the
                    // inside margin.
                    nWidth = getInsideOutsideRelativeWidth(false, GetPageFrame());
                else
                    nWidth = GetPageFrame()->GetRightMargin();
            else
                nWidth = GetPageFrame( )->GetBoundRect( GetPageFrame()->getRootFrame()->GetCurrShell()->GetOut() ).SVRect().GetWidth();
            nTargetWidth = nWidth * (*GetDrawObj( )->GetRelativeWidth());
        }

        bool bCheck = GetDrawObj()->GetRelativeHeight();
        if (bCheck)
        {
            auto pObjCustomShape = dynamic_cast<const SdrObjCustomShape*>(GetDrawObj());
            bCheck = !pObjCustomShape || !pObjCustomShape->IsAutoGrowHeight();
        }

        long nTargetHeight = aCurrObjRect.GetHeight();
        if (bCheck)
        {
            long nHeight = 0;
            if (GetDrawObj()->GetRelativeHeightRelation() == text::RelOrientation::FRAME)
                // Exclude margins.
                nHeight = GetPageFrame()->getFramePrintArea().SVRect().GetHeight();
            else if (GetDrawObj()->GetRelativeHeightRelation() == text::RelOrientation::PAGE_PRINT_AREA)
            {
                // count required height: print area top = top margin + header
                SwRect aHeaderRect;
                const SwHeaderFrame* pHeaderFrame = GetPageFrame()->GetHeaderFrame();
                if (pHeaderFrame)
                    aHeaderRect = pHeaderFrame->GetPaintArea();
                nHeight = GetPageFrame()->GetTopMargin() + aHeaderRect.Height();
            }
            else if (GetDrawObj()->GetRelativeHeightRelation() == text::RelOrientation::PAGE_PRINT_AREA_BOTTOM)
            {
                // count required height: print area bottom = bottom margin + footer
                SwRect aFooterRect;
                auto pFooterFrame = GetPageFrame()->GetFooterFrame();
                if (pFooterFrame)
                    aFooterRect = pFooterFrame->GetPaintArea();
                nHeight = GetPageFrame()->GetBottomMargin() + aFooterRect.Height();
            }
            else
                nHeight = GetPageFrame( )->GetBoundRect( GetPageFrame()->getRootFrame()->GetCurrShell()->GetOut() ).SVRect().GetHeight();
            nTargetHeight = nHeight * (*GetDrawObj()->GetRelativeHeight());
        }

        if ( nTargetWidth != aCurrObjRect.GetWidth( ) || nTargetHeight != aCurrObjRect.GetHeight( ) )
        {
            SwDoc* pDoc = const_cast<SwDoc*>(GetPageFrame()->GetFormat()->GetDoc());

            bool bEnableSetModified = pDoc->getIDocumentState().IsEnableSetModified();
            pDoc->getIDocumentState().SetEnableSetModified(false);
            auto pObject = const_cast<SdrObject*>(GetDrawObj());
            pObject->Resize( aCurrObjRect.TopLeft(),
                    Fraction( nTargetWidth, aCurrObjRect.GetWidth() ),
                    Fraction( nTargetHeight, aCurrObjRect.GetHeight() ), false );

            if (SwFrameFormat* pFrameFormat = FindFrameFormat(pObject))
            {
                if (SwTextBoxHelper::isTextBox(pFrameFormat, RES_DRAWFRMFMT))
                {
                    // Shape has relative size and also a textbox, update its text area as well.
                    uno::Reference<drawing::XShape> xShape(pObject->getUnoShape(), uno::UNO_QUERY);
                    SwTextBoxHelper::syncProperty(pFrameFormat, RES_FRM_SIZE, MID_FRMSIZE_SIZE,
                                                  uno::makeAny(xShape->getSize()));
                }
            }

            pDoc->getIDocumentState().SetEnableSetModified(bEnableSetModified);
        }
    }
    return GetDrawObj()->GetCurrentBoundRect();
}

// --> #i68520#
bool SwAnchoredDrawObject::SetObjTop_( const SwTwips _nTop )
{
    SwTwips nDiff = _nTop - GetObjRect().Top();
    DrawObj()->Move( Size( 0, nDiff ) );

    return nDiff != 0;
}
bool SwAnchoredDrawObject::SetObjLeft_( const SwTwips _nLeft )
{
    SwTwips nDiff = _nLeft - GetObjRect().Left();
    DrawObj()->Move( Size( nDiff, 0 ) );

    return nDiff != 0;
}

/** adjust positioning and alignment attributes for new anchor frame

    #i33313# - add second optional parameter <_pNewObjRect>
*/
void SwAnchoredDrawObject::AdjustPositioningAttr( const SwFrame* _pNewAnchorFrame,
                                                  const SwRect* _pNewObjRect )
{
    SwTwips nHoriRelPos = 0;
    SwTwips nVertRelPos = 0;
    const Point aAnchorPos = _pNewAnchorFrame->GetFrameAnchorPos( ::HasWrap( GetDrawObj() ) );
    // --> #i33313#
    const SwRect aObjRect( _pNewObjRect ? *_pNewObjRect : GetObjRect() );
    const bool bVert = _pNewAnchorFrame->IsVertical();
    const bool bR2L = _pNewAnchorFrame->IsRightToLeft();
    if ( bVert )
    {
        nHoriRelPos = aObjRect.Top() - aAnchorPos.Y();
        nVertRelPos = aAnchorPos.X() - aObjRect.Right();
    }
    else if ( bR2L )
    {
        nHoriRelPos = aAnchorPos.X() - aObjRect.Right();
        nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
    }
    else
    {
        nHoriRelPos = aObjRect.Left() - aAnchorPos.X();
        nVertRelPos = aObjRect.Top() - aAnchorPos.Y();
    }

    GetFrameFormat().SetFormatAttr( SwFormatHoriOrient( nHoriRelPos, text::HoriOrientation::NONE,
        GetFrameFormat().GetAnchor().GetAnchorId() == RndStdIds::FLY_AT_PAGE
            ? text::RelOrientation::PAGE_FRAME
            : text::RelOrientation::FRAME ) );
    GetFrameFormat().SetFormatAttr( SwFormatVertOrient( nVertRelPos, text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
}

// --> #i34748# - change return type.
// If member <mpLastObjRect> is NULL, create one.
void SwAnchoredDrawObject::SetLastObjRect( const tools::Rectangle& _rNewLastRect )
{
    maLastObjRect = _rNewLastRect;
}

void SwAnchoredDrawObject::ObjectAttachedToAnchorFrame()
{
    // --> #i31698#
    SwAnchoredObject::ObjectAttachedToAnchorFrame();

    if ( mbNotYetAttachedToAnchorFrame )
    {
        mbNotYetAttachedToAnchorFrame = false;
    }
}

/** method to set positioning attributes

    #i35798#
    During load the positioning attributes aren't set.
    Thus, the positioning attributes are set by the current object geometry.
    This method is also used for the conversion for drawing objects
    (not anchored as-character) imported from OpenOffice.org file format
    once and directly before the first positioning.
*/
void SwAnchoredDrawObject::SetPositioningAttr()
{
    SwDrawContact* pDrawContact =
                        static_cast<SwDrawContact*>(GetUserCall( GetDrawObj() ));

    if ( !pDrawContact->ObjAnchoredAsChar() )
    {
        SwRect aObjRect( GetObjRect() );

        SwTwips nHoriPos = aObjRect.Left();
        SwTwips nVertPos = aObjRect.Top();
        // #i44334#, #i44681#
        // perform conversion only if position is in horizontal-left-to-right-layout.
        if ( GetFrameFormat().GetPositionLayoutDir() ==
                text::PositionLayoutDir::PositionInHoriL2R )
        {
            SwFrameFormat::tLayoutDir eLayoutDir = GetFrameFormat().GetLayoutDir();
            switch ( eLayoutDir )
            {
                case SwFrameFormat::HORI_L2R:
                {
                    // nothing to do
                }
                break;
                case SwFrameFormat::HORI_R2L:
                {
                    nHoriPos = -aObjRect.Left() - aObjRect.Width();
                }
                break;
                case SwFrameFormat::VERT_R2L:
                {
                    nHoriPos = aObjRect.Top();
                    nVertPos = -aObjRect.Left() - aObjRect.Width();
                }
                break;
                default:
                {
                    assert(!"<SwAnchoredDrawObject::SetPositioningAttr()> - unsupported layout direction");
                }
            }
        }

        // --> #i71182#
        // only change position - do not lose other attributes

        SwFormatHoriOrient aHori( GetFrameFormat().GetHoriOrient() );
        if (nHoriPos != aHori.GetPos()) {
            aHori.SetPos( nHoriPos );
            InvalidateObjRectWithSpaces();
            GetFrameFormat().SetFormatAttr( aHori );
        }

        SwFormatVertOrient aVert( GetFrameFormat().GetVertOrient() );
        if (nVertPos != aVert.GetPos()) {
            aVert.SetPos( nVertPos );
            InvalidateObjRectWithSpaces();
            GetFrameFormat().SetFormatAttr( aVert );
        }

        // --> #i36010# - set layout direction of the position
        GetFrameFormat().SetPositionLayoutDir(
            text::PositionLayoutDir::PositionInLayoutDirOfAnchor );
    }
    // --> #i65798# - also for as-character anchored objects
    // --> #i45952# - indicate that position
    // attributes are set now.
    static_cast<SwDrawFrameFormat&>(GetFrameFormat()).PosAttrSet();
}

void SwAnchoredDrawObject::NotifyBackground( SwPageFrame* _pPageFrame,
                                             const SwRect& _rRect,
                                             PrepareHint _eHint )
{
    ::Notify_Background( GetDrawObj(), _pPageFrame, _rRect, _eHint, true );
}

/** method to assure that anchored object is registered at the correct
    page frame

    #i28701#
*/
void SwAnchoredDrawObject::RegisterAtCorrectPage()
{
    SwPageFrame* pPageFrame( nullptr );
    if ( GetVertPosOrientFrame() )
    {
        pPageFrame = const_cast<SwPageFrame*>(GetVertPosOrientFrame()->FindPageFrame());
    }
    if ( pPageFrame && GetPageFrame() != pPageFrame )
    {
        if ( GetPageFrame() )
            GetPageFrame()->RemoveDrawObjFromPage( *this );
        pPageFrame->AppendDrawObjToPage( *this );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
