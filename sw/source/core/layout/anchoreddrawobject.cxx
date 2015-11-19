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

using namespace ::com::sun::star;

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
        ~SwPosNotify();
        // #i32795#
        Point LastObjPos() const;
};

SwPosNotify::SwPosNotify( SwAnchoredDrawObject* _pAnchoredDrawObj ) :
    mpAnchoredDrawObj( _pAnchoredDrawObj )
{
    maOldObjRect = mpAnchoredDrawObj->GetObjRect();
    // --> #i35640# - determine correct page frame
    mpOldPageFrame = mpAnchoredDrawObj->GetPageFrame();
}

SwPosNotify::~SwPosNotify()
{
    if ( maOldObjRect != mpAnchoredDrawObj->GetObjRect() )
    {
        if( maOldObjRect.HasArea() && mpOldPageFrame )
        {
            mpAnchoredDrawObj->NotifyBackground( mpOldPageFrame, maOldObjRect,
                                                 PREP_FLY_LEAVE );
        }
        SwRect aNewObjRect( mpAnchoredDrawObj->GetObjRect() );
        if( aNewObjRect.HasArea() )
        {
            // --> #i35640# - determine correct page frame
            SwPageFrame* pNewPageFrame = mpAnchoredDrawObj->GetPageFrame();
            if( pNewPageFrame )
                mpAnchoredDrawObj->NotifyBackground( pNewPageFrame, aNewObjRect,
                                                     PREP_FLY_ARRIVE );
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
            mpAnchoredDrawObj->AnchorFrame()->Prepare( PREP_FLY_LEAVE );
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
                                    PREP_FLY_ARRIVE );
            // invalidate position of anchor frame in order to force
            // a re-format of the anchor frame, which also causes a
            // re-format of the invalid previous frames of the anchor frame.
            mpAnchoredDrawObj->AnchorFrame()->InvalidatePos();
        }
    }
}

// --> #i32795#
Point SwPosNotify::LastObjPos() const
{
    return maOldObjRect.Pos();
}

// #i32795#
/// helper class for oscillation control on object positioning
class SwObjPosOscillationControl
{
    private:
        sal_uInt8 mnPosStackSize;

        const SwAnchoredDrawObject* mpAnchoredDrawObj;

        std::vector<Point*> maObjPositions;

    public:
        explicit SwObjPosOscillationControl( const SwAnchoredDrawObject& _rAnchoredDrawObj );
        ~SwObjPosOscillationControl();

        bool OscillationDetected();
};

SwObjPosOscillationControl::SwObjPosOscillationControl(
                                const SwAnchoredDrawObject& _rAnchoredDrawObj )
    : mnPosStackSize( 20 ),
      mpAnchoredDrawObj( &_rAnchoredDrawObj )
{
}

SwObjPosOscillationControl::~SwObjPosOscillationControl()
{
    while ( !maObjPositions.empty() )
    {
        Point* pPos = maObjPositions.back();
        delete pPos;

        maObjPositions.pop_back();
    }
}

bool SwObjPosOscillationControl::OscillationDetected()
{
    bool bOscillationDetected = false;

    if ( maObjPositions.size() == mnPosStackSize )
    {
        // position stack is full -> oscillation
        bOscillationDetected = true;
    }
    else
    {
        Point* pNewObjPos = new Point( mpAnchoredDrawObj->GetObjRect().Pos() );
        for ( std::vector<Point*>::iterator aObjPosIter = maObjPositions.begin();
              aObjPosIter != maObjPositions.end();
              ++aObjPosIter )
        {
            if ( *(pNewObjPos) == *(*aObjPosIter) )
            {
                // position already occurred -> oscillation
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


SwAnchoredDrawObject::SwAnchoredDrawObject() :
    SwAnchoredObject(),
    mbValidPos( false ),
    // --> #i34748#
    mpLastObjRect( nullptr ),
    mbNotYetAttachedToAnchorFrame( true ),
    // --> #i28749#
    mbNotYetPositioned( true ),
    // --> #i62875#
    mbCaptureAfterLayoutDirChange( false )
{
}

SwAnchoredDrawObject::~SwAnchoredDrawObject()
{
    // #i34748#
    delete mpLastObjRect;
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
            ( aTmpRect.Intersection( GetPageFrame()->Frame() ) != GetObjRect() );
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
            _SetPositioningAttr();
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
            case FLY_AS_CHAR:
            {
                // indicate that position will be valid after positioning is performed
                mbValidPos = true;
                // nothing to do, because as-character anchored objects are positioned
                // during the format of its anchor frame - see <SwFlyCntPortion::SetBase(..)>
            }
            break;
            case FLY_AT_PARA:
            case FLY_AT_CHAR:
            {
                // --> #i32795# - move intrinsic positioning to
                // helper method <_MakeObjPosAnchoredAtPara()>
                _MakeObjPosAnchoredAtPara();
            }
            break;
            case FLY_AT_PAGE:
            case FLY_AT_FLY:
            {
                // --> #i32795# - move intrinsic positioning to
                // helper method <_MakeObjPosAnchoredAtLayout()>
                _MakeObjPosAnchoredAtLayout();
            }
            break;
            default:
            {
                OSL_FAIL( "<SwAnchoredDrawObject::MakeObjPos()> - unknown anchor type." );
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
             GetAnchorFrame()->IsValid() )
        {
            pDrawContact->ChkPage();
        }
    }

    // --> #i62875#
    if ( mbCaptureAfterLayoutDirChange &&
         GetPageFrame() )
    {
        SwRect aPageRect( GetPageFrame()->Frame() );
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
}

/** method for the intrinsic positioning of a at-paragraph|at-character
    anchored drawing object

    #i32795# - helper method for method <MakeObjPos>
*/
void SwAnchoredDrawObject::_MakeObjPosAnchoredAtPara()
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
    const bool bFormatAnchor =
            !static_cast<const SwTextFrame*>( GetAnchorFrameContainingAnchPos() )->IsAnyJoinLocked() &&
            !ConsiderObjWrapInfluenceOnObjPos() &&
            !ConsiderObjWrapInfluenceOfOtherObjs();

    if ( bFormatAnchor )
    {
        // --> #i50356#
        GetAnchorFrameContainingAnchPos()->Calc(GetAnchorFrameContainingAnchPos()->getRootFrame()->GetCurrShell()->GetOut());
    }

    bool bOscillationDetected = false;
    SwObjPosOscillationControl aObjPosOscCtrl( *this );
    // --> #i3317# - boolean, to apply temporarly the
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
            _SetDrawObjAnchor();

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
    // temporarly consider the anchored objects wrapping style influence
    if ( bOscillationDetected || bConsiderWrapInfluenceDueToOverlapPrevCol )
    {
        SetTmpConsiderWrapInfluence( true );
        SetRestartLayoutProcess( true );
    }
}

/** method for the intrinsic positioning of a at-page|at-frame anchored
    drawing object

    #i32795# - helper method for method <MakeObjPos>
*/
void SwAnchoredDrawObject::_MakeObjPosAnchoredAtLayout()
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
    SWRECTFN( pAnchorFrame );
    const Point aAnchPos( (pAnchorFrame->Frame().*fnRect->fnGetPos)() );
    SetObjLeft( aAnchPos.X() + GetCurrRelPos().X() );
    SetObjTop( aAnchPos.Y() + GetCurrRelPos().Y() );
}

void SwAnchoredDrawObject::_SetDrawObjAnchor()
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
void SwAnchoredDrawObject::_InvalidatePage( SwPageFrame* _pPageFrame )
{
    if ( _pPageFrame && !_pPageFrame->GetFormat()->GetDoc()->IsInDtor() )
    {
        if ( _pPageFrame->GetUpper() )
        {
            // --> #i35007# - correct invalidation for as-character
            // anchored objects.
            if ( GetFrameFormat().GetAnchor().GetAnchorId() == FLY_AS_CHAR )
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
    }
}

void SwAnchoredDrawObject::InvalidateObjPos()
{
    // --> #i28701# - check, if invalidation is allowed
    if ( mbValidPos &&
         InvalidationOfPosAllowed() )
    {
        mbValidPos = false;
        // --> #i68520#
        InvalidateObjRectWithSpaces();

        // --> #i44339# - check, if anchor frame exists.
        if ( GetAnchorFrame() )
        {
            // --> #118547# - notify anchor frame of as-character
            // anchored object, because its positioned by the format of its anchor frame.
            // --> #i44559# - assure, that text hint is already
            // existing in the text frame
            if ( dynamic_cast< const SwTextFrame* >(GetAnchorFrame()) !=  nullptr &&
                 (GetFrameFormat().GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
            {
                SwTextFrame* pAnchorTextFrame( static_cast<SwTextFrame*>(AnchorFrame()) );
                if ( pAnchorTextFrame->GetTextNode()->GetpSwpHints() &&
                     pAnchorTextFrame->CalcFlyPos( &GetFrameFormat() ) != COMPLETE_STRING )
                {
                    AnchorFrame()->Prepare( PREP_FLY_ATTR_CHG, &GetFrameFormat() );
                }
            }

            SwPageFrame* pPageFrame = AnchorFrame()->FindPageFrame();
            _InvalidatePage( pPageFrame );

            // --> #i32270# - also invalidate page frame, at which the
            // drawing object is registered at.
            SwPageFrame* pPageFrameRegisteredAt = GetPageFrame();
            if ( pPageFrameRegisteredAt &&
                 pPageFrameRegisteredAt != pPageFrame )
            {
                _InvalidatePage( pPageFrameRegisteredAt );
            }
            // #i33751#, #i34060# - method <GetPageFrameOfAnchor()>
            // is replaced by method <FindPageFrameOfAnchor()>. It's return value
            // have to be checked.
            SwPageFrame* pPageFrameOfAnchor = FindPageFrameOfAnchor();
            if ( pPageFrameOfAnchor &&
                 pPageFrameOfAnchor != pPageFrame &&
                 pPageFrameOfAnchor != pPageFrameRegisteredAt )
            {
                _InvalidatePage( pPageFrameOfAnchor );
            }
        }
    }
}

SwFrameFormat& SwAnchoredDrawObject::GetFrameFormat()
{
    OSL_ENSURE( static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFormat(),
            "<SwAnchoredDrawObject::GetFrameFormat()> - missing frame format -> crash." );
    return *(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFormat());
}
const SwFrameFormat& SwAnchoredDrawObject::GetFrameFormat() const
{
    OSL_ENSURE( static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFormat(),
            "<SwAnchoredDrawObject::GetFrameFormat()> - missing frame format -> crash." );
    return *(static_cast<SwDrawContact*>(GetUserCall(GetDrawObj()))->GetFormat());
}

const SwRect SwAnchoredDrawObject::GetObjRect() const
{
    // use geometry of drawing object
    //return GetDrawObj()->GetCurrentBoundRect();
    return GetDrawObj()->GetSnapRect();
}

// --> #i70122#
const SwRect SwAnchoredDrawObject::GetObjBoundRect() const
{
    bool bGroupShape = dynamic_cast<const SdrObjGroup*>( GetDrawObj() );
    // Resize objects with relative width or height
    if ( !bGroupShape && GetPageFrame( ) && ( GetDrawObj( )->GetRelativeWidth( ) || GetDrawObj()->GetRelativeHeight( ) ) )
    {
        Rectangle aCurrObjRect = GetDrawObj()->GetCurrentBoundRect();

        long nTargetWidth = aCurrObjRect.GetWidth( );
        if ( GetDrawObj( )->GetRelativeWidth( ) )
        {
            Rectangle aPageRect;
            if (GetDrawObj()->GetRelativeWidthRelation() == text::RelOrientation::FRAME)
                // Exclude margins.
                aPageRect = GetPageFrame()->Prt().SVRect();
            else
                aPageRect = GetPageFrame( )->GetBoundRect( GetPageFrame()->getRootFrame()->GetCurrShell()->GetOut() ).SVRect();
            nTargetWidth = aPageRect.GetWidth( ) * (*GetDrawObj( )->GetRelativeWidth());
        }

        long nTargetHeight = aCurrObjRect.GetHeight( );
        if ( GetDrawObj( )->GetRelativeHeight( ) )
        {
            Rectangle aPageRect;
            if (GetDrawObj()->GetRelativeHeightRelation() == text::RelOrientation::FRAME)
                // Exclude margins.
                aPageRect = GetPageFrame()->Prt().SVRect();
            else
                aPageRect = GetPageFrame( )->GetBoundRect( GetPageFrame()->getRootFrame()->GetCurrShell()->GetOut() ).SVRect();
            nTargetHeight = aPageRect.GetHeight( ) * (*GetDrawObj( )->GetRelativeHeight());
        }

        if ( nTargetWidth != aCurrObjRect.GetWidth( ) || nTargetHeight != aCurrObjRect.GetHeight( ) )
        {
            SwDoc* pDoc = const_cast<SwDoc*>(GetPageFrame()->GetFormat()->GetDoc());
            bool bModified = pDoc->getIDocumentState().IsModified();
            const_cast< SdrObject* >( GetDrawObj() )->Resize( aCurrObjRect.TopLeft(),
                    Fraction( nTargetWidth, aCurrObjRect.GetWidth() ),
                    Fraction( nTargetHeight, aCurrObjRect.GetHeight() ), false );
            if (!bModified)
                pDoc->getIDocumentState().ResetModified();
        }
    }
    return GetDrawObj()->GetCurrentBoundRect();
}

// --> #i68520#
bool SwAnchoredDrawObject::_SetObjTop( const SwTwips _nTop )
{
    SwTwips nDiff = _nTop - GetObjRect().Top();
    DrawObj()->Move( Size( 0, nDiff ) );

    return nDiff != 0;
}
bool SwAnchoredDrawObject::_SetObjLeft( const SwTwips _nLeft )
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

    GetFrameFormat().SetFormatAttr( SwFormatHoriOrient( nHoriRelPos, text::HoriOrientation::NONE, text::RelOrientation::FRAME ) );
    GetFrameFormat().SetFormatAttr( SwFormatVertOrient( nVertRelPos, text::VertOrientation::NONE, text::RelOrientation::FRAME ) );
}

// --> #i34748# - change return type.
// If member <mpLastObjRect> is NULL, create one.
void SwAnchoredDrawObject::SetLastObjRect( const Rectangle& _rNewLastRect )
{
    if ( !mpLastObjRect )
    {
        mpLastObjRect = new Rectangle;
    }
    *(mpLastObjRect) = _rNewLastRect;
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
void SwAnchoredDrawObject::_SetPositioningAttr()
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
                    OSL_FAIL( "<SwAnchoredDrawObject::_SetPositioningAttr()> - unsupported layout direction" );
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
