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
#include <fmtornt.hxx>
// --> #i28701#
#include <doc.hxx>
#include <fmtsrnd.hxx>
#include <dcontact.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <sortedobjs.hxx>
#include <pagefrm.hxx>
// --> #i35911#
#include <layouter.hxx>

using namespace ::com::sun::star;

// --> #i28701# -
// implementation of helper class <SwObjPositioningInProgress>

SwObjPositioningInProgress::SwObjPositioningInProgress( SdrObject& _rSdrObj ) :
    mpAnchoredObj( 0L ),
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

// SwAnchoredObject

TYPEINIT0(SwAnchoredObject);

SwAnchoredObject::SwAnchoredObject() :
    mpDrawObj( 0L ),
    mpAnchorFrm( 0L ),
    // --> #i28701#
    mpPageFrm( 0L ),
    maRelPos(),
    maLastCharRect(),
    mnLastTopOfLine( 0L ),
    mpVertPosOrientFrm( 0L ),
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

void SwAnchoredObject::ClearVertPosOrientFrm()
{
    if (mpVertPosOrientFrm)
    {
        const_cast<SwLayoutFrm*>(mpVertPosOrientFrm)->ClearVertPosOrientFrmFor(this);
        mpVertPosOrientFrm = NULL;
    }
}

SwAnchoredObject::~SwAnchoredObject()
{
    ClearVertPosOrientFrm();
}

void SwAnchoredObject::SetDrawObj( SdrObject& _rDrawObj )
{
    mpDrawObj = &_rDrawObj;
}

const SdrObject* SwAnchoredObject::GetDrawObj() const
{
    return mpDrawObj;
}

SdrObject* SwAnchoredObject::DrawObj()
{
    return mpDrawObj;
}

const SwFrm* SwAnchoredObject::GetAnchorFrm() const
{
    return mpAnchorFrm;
}

SwFrm* SwAnchoredObject::AnchorFrm()
{
    return mpAnchorFrm;
}

void SwAnchoredObject::ChgAnchorFrm( SwFrm* _pNewAnchorFrm )
{
    mpAnchorFrm = _pNewAnchorFrm;

    if ( mpAnchorFrm )
    {
        ObjectAttachedToAnchorFrame();
    }
}

/** determine anchor frame containing the anchor position

    #i26945#
    the anchor frame, which is determined, is <mpAnchorFrm>
    for an at-page, at-frame or at-paragraph anchored object
    and the anchor character frame for an at-character and as-character
    anchored object.
*/
SwFrm* SwAnchoredObject::GetAnchorFrmContainingAnchPos()
{
    SwFrm* pAnchorFrmContainingAnchPos = FindAnchorCharFrm();
    if ( !pAnchorFrmContainingAnchPos )
    {
        pAnchorFrmContainingAnchPos = AnchorFrm();
    }

    return pAnchorFrmContainingAnchPos;
}

SwPageFrm* SwAnchoredObject::GetPageFrm()
{
    return mpPageFrm;
}

const SwPageFrm* SwAnchoredObject::GetPageFrm() const
{
    return mpPageFrm;
}

void SwAnchoredObject::SetPageFrm( SwPageFrm* _pNewPageFrm )
{
    if ( mpPageFrm != _pNewPageFrm )
    {
        // clear member, which denotes the layout frame at which the vertical
        // position is oriented at, if it doesn't fit to the new page frame.
        if ( GetVertPosOrientFrm() &&
             ( !_pNewPageFrm ||
               _pNewPageFrm != GetVertPosOrientFrm()->FindPageFrm() ) )
        {
            ClearVertPosOrientFrm();
        }

        // assign new page frame
        mpPageFrm = _pNewPageFrm;
    }
}

const SwRect& SwAnchoredObject::GetLastCharRect() const
{
    return maLastCharRect;
}

SwTwips SwAnchoredObject::GetRelCharX( const SwFrm* pFrm ) const
{
    return maLastCharRect.Left() - pFrm->Frm().Left();
}

SwTwips SwAnchoredObject::GetRelCharY( const SwFrm* pFrm ) const
{
    return maLastCharRect.Bottom() - pFrm->Frm().Top();
}

void SwAnchoredObject::AddLastCharY( long nDiff )
{
    maLastCharRect.Pos().Y() += nDiff;
}

void SwAnchoredObject::ResetLastCharRectHeight()
{
    maLastCharRect.Height( 0 );
}

void SwAnchoredObject::SetVertPosOrientFrm( const SwLayoutFrm& _rVertPosOrientFrm )
{
    ClearVertPosOrientFrm();

    mpVertPosOrientFrm = &_rVertPosOrientFrm;
    const_cast<SwLayoutFrm*>(mpVertPosOrientFrm)->SetVertPosOrientFrmFor(this);

    // #i28701# - take over functionality of deleted method
    // <SwFlyAtCntFrm::AssertPage()>: assure for at-paragraph and at-character
    // an anchored object, that it is registered at the correct page frame
    RegisterAtCorrectPage();
}

SwTwips SwAnchoredObject::GetLastTopOfLine() const
{
    return mnLastTopOfLine;
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
    if ( GetAnchorFrm() &&
         GetAnchorFrm()->IsTxtFrm() )
    {
        const SwFmtAnchor& rAnch = GetFrmFmt().GetAnchor();
        if ( (rAnch.GetAnchorId() == FLY_AT_CHAR) &&
             rAnch.GetCntntAnchor() )
        {
            // --> if requested, assure that anchor frame,
            // which contains the anchor character, has a paragraph portion information.
            // The paragraph portion information is needed to determine the
            // anchor character rectangle respectively the top of the line.
            // Thus, a format of this frame is avoided to determine the
            // paragraph portion information.
            // --> #i26945# - use new method <FindAnchorCharFrm()>
            const SwTxtFrm& aAnchorCharFrm = *(FindAnchorCharFrm());
            if ( !_bCheckForParaPorInf || aAnchorCharFrm.HasPara() )
            {
                _CheckCharRect( rAnch, aAnchorCharFrm );
                _CheckTopOfLine( rAnch, aAnchorCharFrm );
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

    improvement - add second parameter <_rAnchorCharFrm>
*/
void SwAnchoredObject::_CheckCharRect( const SwFmtAnchor& _rAnch,
                                       const SwTxtFrm& _rAnchorCharFrm )
{
    // determine rectangle of anchor character. If not exist, abort operation
    SwRect aCharRect;
    if ( !_rAnchorCharFrm.GetAutoPos( aCharRect, *_rAnch.GetCntntAnchor() ) )
    {
        return;
    }
    // check, if anchor character rectangle has changed
    if ( aCharRect != maLastCharRect )
    {
        // check positioning and alignment for invalidation of position
        {
            SWRECTFN( (&_rAnchorCharFrm) );
            // determine positioning and alignment
            SwFmtVertOrient aVert( GetFrmFmt().GetVertOrient() );
            SwFmtHoriOrient aHori( GetFrmFmt().GetHoriOrient() );
            // check for anchor character rectangle changes for certain
            // positionings and alignments
            // add condition to invalidate position,
            // if vertical aligned at frame/page area and vertical position
            // of anchor character has changed.
            const sal_Int16 eVertRelOrient = aVert.GetRelationOrient();
            if ( ( aHori.GetRelationOrient() == text::RelOrientation::CHAR &&
                   (aCharRect.*fnRect->fnGetLeft)() !=
                        (maLastCharRect.*fnRect->fnGetLeft)() ) ||
                 ( eVertRelOrient == text::RelOrientation::CHAR &&
                   ( (aCharRect.*fnRect->fnGetTop)() !=
                        (maLastCharRect.*fnRect->fnGetTop)() ||
                     (aCharRect.*fnRect->fnGetHeight)() !=
                        (maLastCharRect.*fnRect->fnGetHeight)() ) ) ||
                 ( ( ( eVertRelOrient == text::RelOrientation::FRAME ) ||
                     ( eVertRelOrient == text::RelOrientation::PRINT_AREA ) ||
                     ( eVertRelOrient == text::RelOrientation::PAGE_FRAME ) ||
                     ( eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA ) ) &&
                   ( (aCharRect.*fnRect->fnGetTop)() !=
                        (maLastCharRect.*fnRect->fnGetTop)() ) ) )
            {
                // #i26945#, #i35911# - unlock position of
                // anchored object, if it isn't registered at the page,
                // where its anchor character frame is on.
                if ( GetPageFrm() != _rAnchorCharFrm.FindPageFrm() )
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

    improvement - add second parameter <_rAnchorCharFrm>
*/
void SwAnchoredObject::_CheckTopOfLine( const SwFmtAnchor& _rAnch,
                                        const SwTxtFrm& _rAnchorCharFrm )
{
    SwTwips nTopOfLine = 0L;
    if ( _rAnchorCharFrm.GetTopOfLine( nTopOfLine, *_rAnch.GetCntntAnchor() ) )
    {
        if ( nTopOfLine != mnLastTopOfLine )
        {
            // check alignment for invalidation of position
            if ( GetFrmFmt().GetVertOrient().GetRelationOrient() == text::RelOrientation::TEXT_LINE )
            {
                // #i26945#, #i35911# - unlock position of
                // anchored object, if it isn't registered at the page,
                // where its anchor character frame is on.
                if ( GetPageFrm() != _rAnchorCharFrm.FindPageFrm() )
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

const Point SwAnchoredObject::GetCurrRelPos() const
{
    return maRelPos;
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
    SwFrmFmt::tLayoutDir nLayoutDir = SwFrmFmt::HORI_L2R;
    const SwFrm* pAnchorFrm = GetAnchorFrm();
    if ( pAnchorFrm )
    {
        const bool bVert = pAnchorFrm->IsVertical();
        const bool bR2L = pAnchorFrm->IsRightToLeft();
        if ( bVert )
        {
            nLayoutDir = SwFrmFmt::VERT_R2L;
        }
        else if ( bR2L )
        {
            nLayoutDir = SwFrmFmt::HORI_R2L;
        }
    }
    GetFrmFmt().SetLayoutDir( nLayoutDir );
}

/** method to perform necessary invalidations for the positioning of
    objects, for whose the wrapping style influence has to be considered
    on the object positioning.

    #i28701#
*/
void SwAnchoredObject::InvalidateObjPosForConsiderWrapInfluence(
                                                    const bool _bNotifyBackgrd )
{
    if ( ConsiderObjWrapInfluenceOnObjPos() )
    {
        // indicate that object has not to be considered for text wrap
        SetConsiderForTextWrap( false );
        // unlock position
        UnlockPosition();
        // invalidate position
        InvalidateObjPos();
        // invalidate 'background', if requested
        if ( _bNotifyBackgrd )
        {
            NotifyBackground( GetPageFrm(), GetObjRectWithSpaces(), PREP_FLY_LEAVE );
        }
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

    const SwFrmFmt& rObjFmt = GetFrmFmt();

    // --> #i3317# - add condition <IsTmpConsiderWrapInfluence()>
    // --> #i55204#
    // - correction: wrapping style influence has been considered, if condition
    //   <IsTmpConsiderWrapInfluence()> is hold, regardless of its anchor type
    //   or its wrapping style.
    if ( IsTmpConsiderWrapInfluence() )
    {
        bRet = true;
    }
    else if ( rObjFmt.getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) )
    {
        const SwFmtAnchor& rAnchor = rObjFmt.GetAnchor();
        if ( ((rAnchor.GetAnchorId() == FLY_AT_CHAR) ||
              (rAnchor.GetAnchorId() == FLY_AT_PARA)) &&
             rObjFmt.GetSurround().GetSurround() != SURROUND_THROUGHT )
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

    const SwSortedObjs* pObjs = GetAnchorFrm()->GetDrawObjs();
    if ( pObjs->Count() > 1 )
    {
        sal_uInt32 i = 0;
        for ( ; i < pObjs->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
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
    OSL_ENSURE( GetVertPosOrientFrm(),
            "<SwAnchoredObject::HasClearedEnvironment()> - layout frame missing, at which the vertical position is oriented at." );
    if ( GetVertPosOrientFrm() &&
         GetAnchorFrm()->IsTxtFrm() &&
         !static_cast<const SwTxtFrm*>(GetAnchorFrm())->IsFollow() &&
         static_cast<const SwTxtFrm*>(GetAnchorFrm())->FindPageFrm()->GetPhyPageNum() >=
                GetPageFrm()->GetPhyPageNum() )
    {
        const SwFrm* pTmpFrm = GetVertPosOrientFrm()->Lower();
        while ( pTmpFrm && pTmpFrm->IsLayoutFrm() && !pTmpFrm->IsTabFrm() )
        {
            pTmpFrm = static_cast<const SwLayoutFrm*>(pTmpFrm)->Lower();
        }
        if ( !pTmpFrm )
        {
            bHasClearedEnvironment = true;
        }
        else if ( pTmpFrm->IsTxtFrm() && !pTmpFrm->GetNext() )
        {
            const SwTxtFrm* pTmpTxtFrm = static_cast<const SwTxtFrm*>(pTmpFrm);
            if ( pTmpTxtFrm->IsUndersized() ||
                 ( pTmpTxtFrm->GetFollow() &&
                   pTmpTxtFrm->GetFollow()->GetOfst() == 0 ) )
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
        OSL_FAIL( "<SwAnchoredObject::GetObjRectWithSpaces> - cache for object rectangle inclusive spaces marked as valid, but it couldn't be. Missing invalidation of cache. Please inform OD." );
        InvalidateObjRectWithSpaces();
    }
    if ( !mbObjRectWithSpacesValid )
    {
        maObjRectWithSpaces = GetObjBoundRect();
        const SwFrmFmt& rFmt = GetFrmFmt();
        const SvxULSpaceItem& rUL = rFmt.GetULSpace();
        const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
        {
            maObjRectWithSpaces.Top ( std::max( maObjRectWithSpaces.Top() - long(rUL.GetUpper()), 0L ));
            maObjRectWithSpaces.Left( std::max( maObjRectWithSpaces.Left()- long(rLR.GetLeft()),  0L ));
            maObjRectWithSpaces.SSize().Height() += rUL.GetLower();
            maObjRectWithSpaces.SSize().Width()  += rLR.GetRight();
        }

        mbObjRectWithSpacesValid = true;
        maLastObjRect = GetObjRect();
    }

    return maObjRectWithSpaces;
}

// --> #i68520#
void SwAnchoredObject::SetObjTop( const SwTwips _nTop)
{
    const bool bTopChanged( _SetObjTop( _nTop ) );
    if ( bTopChanged )
    {
        mbObjRectWithSpacesValid = false;
    }
}

void SwAnchoredObject::SetObjLeft( const SwTwips _nLeft)
{
    const bool bLeftChanged( _SetObjLeft( _nLeft ) );
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
    if ( GetAnchorFrm() )
    {
        if ( GetFrmFmt().getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) )
        {
            // invalidate position of all anchored objects at anchor frame
            if ( GetAnchorFrm()->GetDrawObjs() )
            {
                const SwSortedObjs* pObjs = GetAnchorFrm()->GetDrawObjs();
                // determine start index
                sal_uInt32 i = 0;
                for ( ; i < pObjs->Count(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                    if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
                        pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
                    else
                        pAnchoredObj->InvalidateObjPos();
                }
            }
            // invalidate all following anchored objects on the page frame
            if ( GetPageFrm() && GetPageFrm()->GetSortedObjs() )
            {
                const SwSortedObjs* pObjs = GetPageFrm()->GetSortedObjs();
                // determine start index
                sal_uInt32 i = pObjs->ListPosOf( *this ) + 1;
                for ( ; i < pObjs->Count(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                    if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
                        pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
                    else
                        pAnchoredObj->InvalidateObjPos();
                }
            }
        }
        // update its position in the sorted object list of its anchor frame
        AnchorFrm()->GetDrawObjs()->Update( *this );
        // update its position in the sorted object list of its page frame
        // note: as-character anchored object aren't registered at a page frame
        if ( GetFrmFmt().GetAnchor().GetAnchorId() != FLY_AS_CHAR )
        {
            GetPageFrm()->GetSortedObjs()->Update( *this );
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
SwPageFrm* SwAnchoredObject::FindPageFrmOfAnchor()
{
    SwPageFrm* pRetPageFrm = 0L;

    // --> #i44339# - check, if anchor frame exists.
    if ( mpAnchorFrm )
    {
        // --> #i26945# - use new method <GetAnchorFrmContainingAnchPos()>
        pRetPageFrm = GetAnchorFrmContainingAnchPos()->FindPageFrm();
    }

    return pRetPageFrm;
}

/** get frame, which contains the anchor character, if the object
    is anchored at-character or as-character.

    #i26945#

    @return SwTxtFrm*
    text frame containing the anchor character. It's NULL, if the object
    isn't anchored at-character resp. as-character.
*/
SwTxtFrm* SwAnchoredObject::FindAnchorCharFrm()
{
    SwTxtFrm* pAnchorCharFrm( 0L );

    // --> #i44339# - check, if anchor frame exists.
    if ( mpAnchorFrm )
    {
        const SwFmtAnchor& rAnch = GetFrmFmt().GetAnchor();
        if ((rAnch.GetAnchorId() == FLY_AT_CHAR) ||
            (rAnch.GetAnchorId() == FLY_AS_CHAR))
        {
            pAnchorCharFrm = &(static_cast<SwTxtFrm*>(AnchorFrm())->
                        GetFrmAtOfst( rAnch.GetCntntAnchor()->nContent.GetIndex() ));
        }
    }

    return pAnchorCharFrm;
}

/** method to determine, if a format on the anchored object is possible

    #i28701#
    A format is possible, if anchored object is in an invisible layer.
    Note: method is virtual to refine the conditions for the sub-classes.
*/
bool SwAnchoredObject::IsFormatPossible() const
{
    return GetFrmFmt().GetDoc()->IsVisibleLayerId( GetDrawObj()->GetLayer() );
}

// --> #i3317#
void SwAnchoredObject::SetTmpConsiderWrapInfluence( const bool _bTmpConsiderWrapInfluence )
{
    mbTmpConsiderWrapInfluence = _bTmpConsiderWrapInfluence;
    // --> #i35911#
    if ( mbTmpConsiderWrapInfluence )
    {
        SwLayouter::InsertObjForTmpConsiderWrapInfluence( *(GetFrmFmt().GetDoc()),
                                                          *this );
    }
}

bool SwAnchoredObject::IsTmpConsiderWrapInfluence() const
{
    return mbTmpConsiderWrapInfluence;
}

void SwAnchoredObject::SetTmpConsiderWrapInfluenceOfOtherObjs( const bool bTmpConsiderWrapInfluence )
{
    const SwSortedObjs* pObjs = GetAnchorFrm()->GetDrawObjs();
    if ( pObjs->Count() > 1 )
    {
        sal_uInt32 i = 0;
        for ( ; i < pObjs->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            if ( pAnchoredObj != this )
            {
                pAnchoredObj->SetTmpConsiderWrapInfluence( bTmpConsiderWrapInfluence );
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

    if ( mpAnchorFrm && mpAnchorFrm->IsTxtFrm() )
    {
        const SwFrm* pColFrm = mpAnchorFrm->FindColFrm();
        if ( pColFrm && pColFrm->GetPrev() )
        {
            const SwFrm* pTmpColFrm = pColFrm->GetPrev();
            SwRect aChkRect;
            while ( pTmpColFrm )
            {
                aChkRect.Union( pTmpColFrm->Frm() );
                pTmpColFrm = pTmpColFrm->GetPrev();
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
Point SwAnchoredObject::GetRelPosToAnchorFrm() const
{
    Point aRelPos;

    OSL_ENSURE( GetAnchorFrm(),
            "<SwAnchoredObject::GetRelPosToAnchorFrm()> - missing anchor frame." );
    aRelPos = GetObjRect().Pos();
    aRelPos -= GetAnchorFrm()->Frm().Pos();

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
Point SwAnchoredObject::GetRelPosToPageFrm( const bool _bFollowTextFlow,
                                            bool& _obRelToTableCell ) const
{
    Point aRelPos;
    _obRelToTableCell = false;

    OSL_ENSURE( GetAnchorFrm(),
            "<SwAnchoredObject::GetRelPosToPageFrm()> - missing anchor frame." );
    OSL_ENSURE( GetAnchorFrm()->FindPageFrm(),
            "<SwAnchoredObject::GetRelPosToPageFrm()> - missing page frame." );

    aRelPos = GetObjRect().Pos();
    // --> #i33818# - search for cell frame, if object has to
    // follow the text flow.
    const SwFrm* pFrm( 0L );
    if ( _bFollowTextFlow && !GetAnchorFrm()->IsPageFrm() )
    {
        pFrm = GetAnchorFrm()->GetUpper();
        while ( !pFrm->IsCellFrm() && !pFrm->IsPageFrm() )
        {
            pFrm = pFrm->GetUpper();
        }
    }
    else
    {
        pFrm = GetAnchorFrm()->FindPageFrm();
    }
    if ( pFrm->IsCellFrm() )
    {
        aRelPos -= ( pFrm->Frm().Pos() + pFrm->Prt().Pos() );
        _obRelToTableCell = true;
    }
    else
    {
        aRelPos -= pFrm->Frm().Pos();
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
    aRelPos.Y() -= GetLastTopOfLine();

    return aRelPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
