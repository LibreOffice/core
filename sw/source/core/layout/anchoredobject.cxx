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

#include <txtfrm.hxx>
#include <frmatr.hxx>
#include <fmtornt.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <fmtsrnd.hxx>
#include <dcontact.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <sortedobjs.hxx>
#include <pagefrm.hxx>
#include <layouter.hxx>
#include <pam.hxx>

using namespace ::com::sun::star;

// --> #i28701# -
// implementation of helper class <SwObjPositioningInProgress>

SwObjPositioningInProgress::SwObjPositioningInProgress( SdrObject& _rSdrObj ) :
    mpAnchoredObj( nullptr ),
    // --> #i52904#
    mbOldObjPositioningInProgress( false )
{
    mpAnchoredObj = ::GetUserCall( &_rSdrObj )->GetAnchoredObj( &_rSdrObj );
    // --> #i52904#
    mbOldObjPositioningInProgress = mpAnchoredObj->IsPositioningInProgress();
    mpAnchoredObj->SetPositioningInProgress( true );
}
SwObjPositioningInProgress::SwObjPositioningInProgress( SwAnchoredObject& _rAnchoredObj ) :
    mpAnchoredObj( &_rAnchoredObj ),
    // --> #i52904#
    mbOldObjPositioningInProgress( false )
{
    // --> #i52904#
    mbOldObjPositioningInProgress = mpAnchoredObj->IsPositioningInProgress();
    mpAnchoredObj->SetPositioningInProgress( true );
}

SwObjPositioningInProgress::~SwObjPositioningInProgress()
{
    if ( mpAnchoredObj )
    {
        // --> #i52904#
        mpAnchoredObj->SetPositioningInProgress( mbOldObjPositioningInProgress );
    }
}


SwAnchoredObject::SwAnchoredObject() :
    mpDrawObj( nullptr ),
    mpAnchorFrame( nullptr ),
    // --> #i28701#
    mpPageFrame( nullptr ),
    maRelPos(),
    maLastCharRect(),
    mnLastTopOfLine( 0 ),
    mpVertPosOrientFrame( nullptr ),
    // --> #i28701#
    mbPositioningInProgress( false ),
    mbConsiderForTextWrap( false ),
    mbPositionLocked( false ),
    // --> #i40147#
    mbKeepPositionLockedForSection( false ),
    mbRestartLayoutProcess( false ),
    // --> #i35911#
    mbClearedEnvironment( false ),
    // --> #i3317#
    mbTmpConsiderWrapInfluence( false ),
    // --> #i68520#
    maObjRectWithSpaces(),
    mbObjRectWithSpacesValid( false ),
    maLastObjRect()
{
}

void SwAnchoredObject::ClearVertPosOrientFrame()
{
    if (mpVertPosOrientFrame)
    {
        const_cast<SwLayoutFrame*>(mpVertPosOrientFrame)->ClearVertPosOrientFrameFor(this);
        mpVertPosOrientFrame = nullptr;
    }
}

SwAnchoredObject::~SwAnchoredObject()
{
    ClearVertPosOrientFrame();
}

void SwAnchoredObject::SetDrawObj( SdrObject& _rDrawObj )
{
    mpDrawObj = &_rDrawObj;
}


void SwAnchoredObject::ChgAnchorFrame( SwFrame* _pNewAnchorFrame )
{
    mpAnchorFrame = _pNewAnchorFrame;

    if ( mpAnchorFrame )
    {
        ObjectAttachedToAnchorFrame();
    }
}

/** determine anchor frame containing the anchor position

    #i26945#
    the anchor frame, which is determined, is <mpAnchorFrame>
    for an at-page, at-frame or at-paragraph anchored object
    and the anchor character frame for an at-character and as-character
    anchored object.
*/
SwFrame* SwAnchoredObject::GetAnchorFrameContainingAnchPos()
{
    SwFrame* pAnchorFrameContainingAnchPos = FindAnchorCharFrame();
    if ( !pAnchorFrameContainingAnchPos )
    {
        pAnchorFrameContainingAnchPos = AnchorFrame();
    }

    return pAnchorFrameContainingAnchPos;
}


void SwAnchoredObject::SetPageFrame( SwPageFrame* _pNewPageFrame )
{
    if ( mpPageFrame != _pNewPageFrame )
    {
        // clear member, which denotes the layout frame at which the vertical
        // position is oriented at, if it doesn't fit to the new page frame.
        if ( GetVertPosOrientFrame() &&
             ( !_pNewPageFrame ||
               _pNewPageFrame != GetVertPosOrientFrame()->FindPageFrame() ) )
        {
            ClearVertPosOrientFrame();
        }

        // assign new page frame
        mpPageFrame = _pNewPageFrame;
    }
}


SwTwips SwAnchoredObject::GetRelCharX( const SwFrame* pFrame ) const
{
    return maLastCharRect.Left() - pFrame->getFrameArea().Left();
}

SwTwips SwAnchoredObject::GetRelCharY( const SwFrame* pFrame ) const
{
    return maLastCharRect.Bottom() - pFrame->getFrameArea().Top();
}

void SwAnchoredObject::AddLastCharY( long nDiff )
{
    maLastCharRect.Pos().AdjustY(nDiff );
}

void SwAnchoredObject::ResetLastCharRectHeight()
{
    maLastCharRect.Height( 0 );
}

void SwAnchoredObject::SetVertPosOrientFrame( const SwLayoutFrame& _rVertPosOrientFrame )
{
    ClearVertPosOrientFrame();

    mpVertPosOrientFrame = &_rVertPosOrientFrame;
    const_cast<SwLayoutFrame*>(mpVertPosOrientFrame)->SetVertPosOrientFrameFor(this);

    // #i28701# - take over functionality of deleted method
    // <SwFlyAtContentFrame::AssertPage()>: assure for at-paragraph and at-character
    // an anchored object, that it is registered at the correct page frame
    RegisterAtCorrectPage();
}


// #i28701# - follow-up of #i22341#
void SwAnchoredObject::AddLastTopOfLineY( SwTwips _nDiff )
{
    mnLastTopOfLine += _nDiff;
}

/** check anchor character rectangle and top of line

    #i26791
    For to-character anchored Writer fly frames the members <maLastCharRect>
    and <maLastTopOfLine> are updated. These are checked for change and
    depending on the applied positioning, it's decided, if the Writer fly
    frame has to be invalidated.

    add parameter <_bCheckForParaPorInf>, default value <true>
*/
void SwAnchoredObject::CheckCharRectAndTopOfLine(
                                        const bool _bCheckForParaPorInf )
{
    if ( GetAnchorFrame() &&
         GetAnchorFrame()->IsTextFrame() )
    {
        const SwFormatAnchor& rAnch = GetFrameFormat().GetAnchor();
        if ( (rAnch.GetAnchorId() == RndStdIds::FLY_AT_CHAR) &&
             rAnch.GetContentAnchor() )
        {
            // --> if requested, assure that anchor frame,
            // which contains the anchor character, has a paragraph portion information.
            // The paragraph portion information is needed to determine the
            // anchor character rectangle respectively the top of the line.
            // Thus, a format of this frame is avoided to determine the
            // paragraph portion information.
            // --> #i26945# - use new method <FindAnchorCharFrame()>
            const SwTextFrame& aAnchorCharFrame = *(FindAnchorCharFrame());
            if ( !_bCheckForParaPorInf || aAnchorCharFrame.HasPara() )
            {
                CheckCharRect( rAnch, aAnchorCharFrame );
                CheckTopOfLine( rAnch, aAnchorCharFrame );
            }
        }
    }
}

/** check anchor character rectangle

    #i22341#
    helper method for method <CheckCharRectAndTopOfLine()>
    For to-character anchored Writer fly frames the member <maLastCharRect>
    is updated. This is checked for change and depending on the applied
    positioning, it's decided, if the Writer fly frame has to be invalidated.

    improvement - add second parameter <_rAnchorCharFrame>
*/
void SwAnchoredObject::CheckCharRect( const SwFormatAnchor& _rAnch,
                                       const SwTextFrame& _rAnchorCharFrame )
{
    // determine rectangle of anchor character. If not exist, abort operation
    SwRect aCharRect;
    if ( !_rAnchorCharFrame.GetAutoPos( aCharRect, *_rAnch.GetContentAnchor() ) )
    {
        return;
    }
    // check, if anchor character rectangle has changed
    if ( aCharRect != maLastCharRect )
    {
        // check positioning and alignment for invalidation of position
        {
            SwRectFnSet aRectFnSet(&_rAnchorCharFrame);
            // determine positioning and alignment
            SwFormatVertOrient aVert( GetFrameFormat().GetVertOrient() );
            SwFormatHoriOrient aHori( GetFrameFormat().GetHoriOrient() );
            // check for anchor character rectangle changes for certain
            // positionings and alignments
            // add condition to invalidate position,
            // if vertical aligned at frame/page area and vertical position
            // of anchor character has changed.
            const sal_Int16 eVertRelOrient = aVert.GetRelationOrient();
            if ( ( aHori.GetRelationOrient() == text::RelOrientation::CHAR &&
                   aRectFnSet.GetLeft(aCharRect) != aRectFnSet.GetLeft(maLastCharRect) ) ||
                 ( eVertRelOrient == text::RelOrientation::CHAR &&
                   ( aRectFnSet.GetTop(aCharRect) != aRectFnSet.GetTop(maLastCharRect) ||
                     aRectFnSet.GetHeight(aCharRect) != aRectFnSet.GetHeight(maLastCharRect) ) ) ||
                 ( ( ( eVertRelOrient == text::RelOrientation::FRAME ) ||
                     ( eVertRelOrient == text::RelOrientation::PRINT_AREA ) ||
                     ( eVertRelOrient == text::RelOrientation::PAGE_FRAME ) ||
                     ( eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA ) ) &&
                   ( aRectFnSet.GetTop(aCharRect) != aRectFnSet.GetTop(maLastCharRect) ) ) )
            {
                // #i26945#, #i35911# - unlock position of
                // anchored object, if it isn't registered at the page,
                // where its anchor character frame is on.
                if ( GetPageFrame() != _rAnchorCharFrame.FindPageFrame() )
                {
                    UnlockPosition();
                }
                InvalidateObjPos();
            }
        }
        // keep new anchor character rectangle
        maLastCharRect = aCharRect;
    }
}

/** check top of line

    #i22341#
    helper method for method <CheckCharRectAndTopOfLine()>
    For to-character anchored Writer fly frames the member <mnLastTopOfLine>
    is updated. This is checked for change and depending on the applied
    positioning, it's decided, if the Writer fly frame has to be invalidated.

    improvement - add second parameter <_rAnchorCharFrame>
*/
void SwAnchoredObject::CheckTopOfLine( const SwFormatAnchor& _rAnch,
                                        const SwTextFrame& _rAnchorCharFrame )
{
    SwTwips nTopOfLine = 0;
    if ( _rAnchorCharFrame.GetTopOfLine( nTopOfLine, *_rAnch.GetContentAnchor() ) )
    {
        if ( nTopOfLine != mnLastTopOfLine )
        {
            // check alignment for invalidation of position
            if ( GetFrameFormat().GetVertOrient().GetRelationOrient() == text::RelOrientation::TEXT_LINE )
            {
                // #i26945#, #i35911# - unlock position of
                // anchored object, if it isn't registered at the page,
                // where its anchor character frame is on.
                if ( GetPageFrame() != _rAnchorCharFrame.FindPageFrame() )
                {
                    UnlockPosition();
                }
                InvalidateObjPos();
            }
            // keep new top of line value
            mnLastTopOfLine = nTopOfLine;
        }
    }
}

void SwAnchoredObject::ClearCharRectAndTopOfLine()
{
    maLastCharRect.Clear();
    mnLastTopOfLine = 0;
}

void SwAnchoredObject::SetCurrRelPos( Point _aRelPos )
{
    maRelPos = _aRelPos;
}

void SwAnchoredObject::ObjectAttachedToAnchorFrame()
{
    // default behaviour:
    // update layout direction, the anchored object is assigned to
    UpdateLayoutDir();
}

/** method update layout direction the layout direction, the anchored
    object is in

    #i31698#
    method has typically to be called, if the anchored object gets its
    anchor frame assigned.
*/
void SwAnchoredObject::UpdateLayoutDir()
{
    SwFrameFormat::tLayoutDir nLayoutDir = SwFrameFormat::HORI_L2R;
    const SwFrame* pAnchorFrame = GetAnchorFrame();
    if ( pAnchorFrame )
    {
        const bool bVert = pAnchorFrame->IsVertical();
        const bool bR2L = pAnchorFrame->IsRightToLeft();
        if ( bVert )
        {
            nLayoutDir = SwFrameFormat::VERT_R2L;
        }
        else if ( bR2L )
        {
            nLayoutDir = SwFrameFormat::HORI_R2L;
        }
    }
    GetFrameFormat().SetLayoutDir( nLayoutDir );
}

/** method to perform necessary invalidations for the positioning of
    objects, for whose the wrapping style influence has to be considered
    on the object positioning.

    #i28701#
*/
void SwAnchoredObject::InvalidateObjPosForConsiderWrapInfluence()
{
    if ( ConsiderObjWrapInfluenceOnObjPos() )
    {
        // indicate that object has not to be considered for text wrap
        SetConsiderForTextWrap( false );
        // unlock position
        UnlockPosition();
        // invalidate position
        InvalidateObjPos();
        // invalidate 'background'
        NotifyBackground( GetPageFrame(), GetObjRectWithSpaces(), PREP_FLY_LEAVE );
    }
}

/** method to determine, if wrapping style influence of the anchored
    object has to be considered on the object positioning

    #i28701#
    Note: result of this method also decides, if the booleans for the
    layout process are of relevance.
*/
bool SwAnchoredObject::ConsiderObjWrapInfluenceOnObjPos() const
{
    bool bRet( false );

    const SwFrameFormat& rObjFormat = GetFrameFormat();

    // --> #i3317# - add condition <IsTmpConsiderWrapInfluence()>
    // --> #i55204#
    // - correction: wrapping style influence has been considered, if condition
    //   <IsTmpConsiderWrapInfluence()> is hold, regardless of its anchor type
    //   or its wrapping style.
    if ( IsTmpConsiderWrapInfluence() )
    {
        bRet = true;
    }
    else if ( rObjFormat.getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) )
    {
        const SwFormatAnchor& rAnchor = rObjFormat.GetAnchor();
        if ( ((rAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR) ||
              (rAnchor.GetAnchorId() == RndStdIds::FLY_AT_PARA)) &&
             rObjFormat.GetSurround().GetSurround() != css::text::WrapTextMode_THROUGH )
        {
            // --> #i34520# - text also wraps around anchored
            // objects in the layer Hell - see the text formatting.
            // Thus, it hasn't to be checked here.
            bRet = true;
        }
    }

    return bRet;
}

/** method to determine, if other anchored objects, also attached at
    to the anchor frame, have to consider its wrap influence.

    // --> #i43255#
*/
bool SwAnchoredObject::ConsiderObjWrapInfluenceOfOtherObjs() const
{
    bool bRet( false );

    const SwSortedObjs* pObjs = GetAnchorFrame()->GetDrawObjs();
    if ( pObjs->size() > 1 )
    {
        for (SwAnchoredObject* pAnchoredObj : *pObjs)
        {
            if ( pAnchoredObj != this &&
                 pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
            {
                bRet = true;
                break;
            }
        }
    }

    return bRet;
}

bool SwAnchoredObject::ConsiderForTextWrap() const
{
    if ( ConsiderObjWrapInfluenceOnObjPos() )
        return mbConsiderForTextWrap;
    else
        return true;
}

void SwAnchoredObject::SetConsiderForTextWrap( const bool _bConsiderForTextWrap )
{
    mbConsiderForTextWrap = _bConsiderForTextWrap;
}

bool SwAnchoredObject::PositionLocked() const
{
    if ( ConsiderObjWrapInfluenceOnObjPos() )
        return mbPositionLocked;
    else
        return false;
}

bool SwAnchoredObject::RestartLayoutProcess() const
{
    if ( ConsiderObjWrapInfluenceOnObjPos() )
        return mbRestartLayoutProcess;
    else
        return false;
}

void SwAnchoredObject::SetRestartLayoutProcess( const bool _bRestartLayoutProcess )
{
    mbRestartLayoutProcess = _bRestartLayoutProcess;
}

// --> #i35911#
bool SwAnchoredObject::ClearedEnvironment() const
{
    if ( ConsiderObjWrapInfluenceOnObjPos() )
        return mbClearedEnvironment;
    else
        return false;
}
void SwAnchoredObject::SetClearedEnvironment( const bool _bClearedEnvironment )
{
    mbClearedEnvironment = _bClearedEnvironment;
}

/** method to determine, if due to anchored object size and wrapping
    style, its layout environment is cleared.

    #i35911#
*/
bool SwAnchoredObject::HasClearedEnvironment() const
{
    bool bHasClearedEnvironment( false );

    // --> #i43913# - layout frame, vertical position is orient at, has to be set.
    OSL_ENSURE( GetVertPosOrientFrame(),
            "<SwAnchoredObject::HasClearedEnvironment()> - layout frame missing, at which the vertical position is oriented at." );
    if ( GetVertPosOrientFrame() &&
         GetAnchorFrame()->IsTextFrame() &&
         !static_cast<const SwTextFrame*>(GetAnchorFrame())->IsFollow() &&
         static_cast<const SwTextFrame*>(GetAnchorFrame())->FindPageFrame()->GetPhyPageNum() >=
                GetPageFrame()->GetPhyPageNum() )
    {
        const SwFrame* pTmpFrame = GetVertPosOrientFrame()->Lower();
        while ( pTmpFrame && pTmpFrame->IsLayoutFrame() && !pTmpFrame->IsTabFrame() )
        {
            pTmpFrame = static_cast<const SwLayoutFrame*>(pTmpFrame)->Lower();
        }
        if ( !pTmpFrame )
        {
            bHasClearedEnvironment = true;
        }
        else if ( pTmpFrame->IsTextFrame() && !pTmpFrame->GetNext() )
        {
            const SwTextFrame* pTmpTextFrame = static_cast<const SwTextFrame*>(pTmpFrame);
            if ( pTmpTextFrame->IsUndersized() ||
                 ( pTmpTextFrame->GetFollow() &&
                   pTmpTextFrame->GetFollow()->GetOfst() == TextFrameIndex(0)))
            {
                bHasClearedEnvironment = true;
            }
        }
    }

    return bHasClearedEnvironment;
}

/** method to add spacing to object area

    #i28701#
    #i68520# - return constant reference and use cache
*/
const SwRect& SwAnchoredObject::GetObjRectWithSpaces() const
{
    if ( mbObjRectWithSpacesValid &&
         maLastObjRect != GetObjRect() )
    {
        OSL_FAIL( "<SwAnchoredObject::GetObjRectWithSpaces> - cache for object rectangle inclusive spaces marked as valid, but it couldn't be. Missing invalidation of cache." );
        InvalidateObjRectWithSpaces();
    }
    if ( !mbObjRectWithSpacesValid )
    {
        maObjRectWithSpaces = GetObjBoundRect();
        const SwFrameFormat& rFormat = GetFrameFormat();
        const SvxULSpaceItem& rUL = rFormat.GetULSpace();
        const SvxLRSpaceItem& rLR = rFormat.GetLRSpace();
        {
            maObjRectWithSpaces.Top ( std::max( maObjRectWithSpaces.Top() - long(rUL.GetUpper()), 0L ));
            maObjRectWithSpaces.Left( std::max( maObjRectWithSpaces.Left()- rLR.GetLeft(),  0L ));
            maObjRectWithSpaces.SSize().AdjustHeight(rUL.GetLower() );
            maObjRectWithSpaces.SSize().AdjustWidth(rLR.GetRight() );
        }

        mbObjRectWithSpacesValid = true;
        maLastObjRect = GetObjRect();
    }

    return maObjRectWithSpaces;
}

// --> #i68520#
void SwAnchoredObject::SetObjTop( const SwTwips _nTop)
{
    const bool bTopChanged( SetObjTop_( _nTop ) );
    if ( bTopChanged )
    {
        mbObjRectWithSpacesValid = false;
    }
}

void SwAnchoredObject::SetObjLeft( const SwTwips _nLeft)
{
    const bool bLeftChanged( SetObjLeft_( _nLeft ) );
    if ( bLeftChanged )
    {
        mbObjRectWithSpacesValid = false;
    }
}

/** method to update anchored object in the <SwSortedObjs> lists

    #i28701#
    If document compatibility option 'Consider wrapping style influence
    on object positioning' is ON, additionally all anchored objects
    at the anchor frame and all following anchored objects on the page
    frame are invalidated.
*/
void SwAnchoredObject::UpdateObjInSortedList()
{
    if ( GetAnchorFrame() )
    {
        if ( GetFrameFormat().getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) )
        {
            // invalidate position of all anchored objects at anchor frame
            if ( GetAnchorFrame()->GetDrawObjs() )
            {
                const SwSortedObjs* pObjs = GetAnchorFrame()->GetDrawObjs();
                // determine start index
                for (auto it = pObjs->begin(); it != pObjs->end(); ++it)
                {
                    SwAnchoredObject* pAnchoredObj = *it;
                    if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
                        pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence();
                    else
                        pAnchoredObj->InvalidateObjPos();
                }
            }
            // invalidate all following anchored objects on the page frame
            if ( GetPageFrame() && GetPageFrame()->GetSortedObjs() )
            {
                const SwSortedObjs* pObjs = GetPageFrame()->GetSortedObjs();
                // determine start index
                for ( size_t i = pObjs->ListPosOf( *this ) + 1; i < pObjs->size(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                    if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
                        pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence();
                    else
                        pAnchoredObj->InvalidateObjPos();
                }
            }
        }
        // update its position in the sorted object list of its anchor frame
        AnchorFrame()->GetDrawObjs()->Update( *this );
        // update its position in the sorted object list of its page frame
        // note: as-character anchored object aren't registered at a page frame
        if ( GetFrameFormat().GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR )
        {
            GetPageFrame()->GetSortedObjs()->Update( *this );
        }
    }
}

/** method to determine, if invalidation of position is allowed

    #i28701#
*/
bool SwAnchoredObject::InvalidationOfPosAllowed() const
{
    // --> Check, if page frame layout is in progress,
    // isn't needed, because of anchored object, whose are moved forward.
    return !PositionLocked();
}

/** method to determine the page frame, on which the 'anchor' of
    the given anchored object is.

    #i28701#
    #i33751#, #i34060#
    Adjust meaning of method and thus its name: If the anchored object
    or its anchor isn't correctly inserted in the layout, no page frame
    can be found. Thus, the return type changed to be a pointer and can
    be NULL.
*/
SwPageFrame* SwAnchoredObject::FindPageFrameOfAnchor()
{
    SwPageFrame* pRetPageFrame = nullptr;

    // --> #i44339# - check, if anchor frame exists.
    if ( mpAnchorFrame )
    {
        // --> #i26945# - use new method <GetAnchorFrameContainingAnchPos()>
        pRetPageFrame = GetAnchorFrameContainingAnchPos()->FindPageFrame();
    }

    return pRetPageFrame;
}

/** get frame, which contains the anchor character, if the object
    is anchored at-character or as-character.

    #i26945#

    @return SwTextFrame*
    text frame containing the anchor character. It's NULL, if the object
    isn't anchored at-character resp. as-character.
*/
SwTextFrame* SwAnchoredObject::FindAnchorCharFrame()
{
    SwTextFrame* pAnchorCharFrame( nullptr );

    // --> #i44339# - check, if anchor frame exists.
    if ( mpAnchorFrame )
    {
        const SwFormatAnchor& rAnch = GetFrameFormat().GetAnchor();
        if ((rAnch.GetAnchorId() == RndStdIds::FLY_AT_CHAR) ||
            (rAnch.GetAnchorId() == RndStdIds::FLY_AS_CHAR))
        {
            SwTextFrame *const pFrame(static_cast<SwTextFrame*>(AnchorFrame()));
            TextFrameIndex const nOffset(pFrame->MapModelToViewPos(*rAnch.GetContentAnchor()));
            pAnchorCharFrame = &pFrame->GetFrameAtOfst(nOffset);
        }
    }

    return pAnchorCharFrame;
}

/** method to determine, if a format on the anchored object is possible

    #i28701#
    A format is possible, if anchored object is in an invisible layer.
    Note: method is virtual to refine the conditions for the sub-classes.
*/
bool SwAnchoredObject::IsFormatPossible() const
{
    return GetFrameFormat().GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId( GetDrawObj()->GetLayer() );
}

// --> #i3317#
void SwAnchoredObject::SetTmpConsiderWrapInfluence( const bool _bTmpConsiderWrapInfluence )
{
    mbTmpConsiderWrapInfluence = _bTmpConsiderWrapInfluence;
    // --> #i35911#
    if ( mbTmpConsiderWrapInfluence )
    {
        SwLayouter::InsertObjForTmpConsiderWrapInfluence( *(GetFrameFormat().GetDoc()),
                                                          *this );
    }
}

void SwAnchoredObject::ClearTmpConsiderWrapInfluence()
{
    mbTmpConsiderWrapInfluence = false;
    mbClearedEnvironment = false;
    SetClearedEnvironment( false );
    SwLayouter::RemoveObjForTmpConsiderWrapInfluence( *(GetFrameFormat().GetDoc()),
                                                      *this );
}
void SwAnchoredObject::SetTmpConsiderWrapInfluenceOfOtherObjs()
{
    const SwSortedObjs* pObjs = GetAnchorFrame()->GetDrawObjs();
    if ( pObjs->size() > 1 )
    {
        for (SwAnchoredObject* pAnchoredObj : *pObjs)
        {
            if ( pAnchoredObj != this )
            {
                pAnchoredObj->SetTmpConsiderWrapInfluence( true/*bTmpConsiderWrapInfluence*/ );
            }
        }
    }
}

/** method to determine, if the anchored object is overlapping with a
    previous column

    #i3317#
    overlapping with a previous column means, that the object overlaps
    with a column, which is a previous one of the column its anchor
    frame is in.
    Only applied for at-paragraph and at-character anchored objects.
*/
bool SwAnchoredObject::OverlapsPrevColumn() const
{
    bool bOverlapsPrevColumn( false );

    if ( mpAnchorFrame && mpAnchorFrame->IsTextFrame() )
    {
        const SwFrame* pColFrame = mpAnchorFrame->FindColFrame();
        if ( pColFrame && pColFrame->GetPrev() )
        {
            const SwFrame* pTmpColFrame = pColFrame->GetPrev();
            SwRect aChkRect;
            while ( pTmpColFrame )
            {
                aChkRect.Union( pTmpColFrame->getFrameArea() );
                pTmpColFrame = pTmpColFrame->GetPrev();
            }
            bOverlapsPrevColumn = GetObjRect().IsOver( aChkRect );
        }
    }

    return bOverlapsPrevColumn;
}

/** method to determine position of anchored object relative to
    anchor frame

    #i30669#
    Usage: Needed layout information for WW8 export
*/
Point SwAnchoredObject::GetRelPosToAnchorFrame() const
{
    Point aRelPos;

    assert(GetAnchorFrame());
    aRelPos = GetObjRect().Pos();
    aRelPos -= GetAnchorFrame()->getFrameArea().Pos();

    return aRelPos;
}

/** method to determine position of anchored object relative to
    page frame

    #i30669#
    Usage: Needed layout information for WW8 export
    #i33818# - add parameters <_bFollowTextFlow> and
    <_obRelToTableCell>
    If <_bFollowTextFlow> is set and object is anchored inside table,
    the position relative to the table cell is determined. Output
    parameter <_obRelToTableCell> reflects this situation
*/
Point SwAnchoredObject::GetRelPosToPageFrame( const bool _bFollowTextFlow,
                                            bool& _obRelToTableCell ) const
{
    Point aRelPos;
    _obRelToTableCell = false;

    assert(GetAnchorFrame());
    assert(GetAnchorFrame()->FindPageFrame());

    aRelPos = GetObjRect().Pos();
    // --> #i33818# - search for cell frame, if object has to
    // follow the text flow.
    const SwFrame* pFrame( nullptr );
    if ( _bFollowTextFlow && !GetAnchorFrame()->IsPageFrame() )
    {
        pFrame = GetAnchorFrame()->GetUpper();
        while ( !pFrame->IsCellFrame() && !pFrame->IsPageFrame() )
        {
            pFrame = pFrame->GetUpper();
        }
    }
    else
    {
        pFrame = GetAnchorFrame()->FindPageFrame();
    }
    if ( pFrame->IsCellFrame() )
    {
        aRelPos -= pFrame->getFrameArea().Pos() + pFrame->getFramePrintArea().Pos();
        _obRelToTableCell = true;
    }
    else
    {
        aRelPos -= pFrame->getFrameArea().Pos();
    }

    return aRelPos;
}

/** method to determine position of anchored object relative to
    anchor character

    #i30669#
    Usage: Needed layout information for WW8 export
*/
Point SwAnchoredObject::GetRelPosToChar() const
{
    Point aRelPos;

    aRelPos = GetObjRect().Pos();
    aRelPos -= GetLastCharRect().Pos();

    return aRelPos;
}

/** method to determine position of anchored object relative to
    top of line

    #i30669#
    Usage: Needed layout information for WW8 export
*/
Point SwAnchoredObject::GetRelPosToLine() const
{
    Point aRelPos;

    aRelPos = GetObjRect().Pos();
    aRelPos.AdjustY( -(GetLastTopOfLine()) );

    return aRelPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
