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

#include <objectformattertxtfrm.hxx>
#include <sortedobjs.hxx>
#include <rootfrm.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <rowfrm.hxx>
#include <layouter.hxx>
#include <fmtanchr.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <fmtfollowtextflow.hxx>
#include <layact.hxx>

using namespace ::com::sun::star;

// little helper class to forbid follow formatting for the given text frame
class SwForbidFollowFormat
{
private:
    SwTextFrm& mrTextFrm;
    const bool bOldFollowFormatAllowed;

public:
    explicit SwForbidFollowFormat( SwTextFrm& _rTextFrm )
        : mrTextFrm( _rTextFrm ),
          bOldFollowFormatAllowed( _rTextFrm.FollowFormatAllowed() )
    {
        mrTextFrm.ForbidFollowFormat();
    }

    ~SwForbidFollowFormat()
    {
        if ( bOldFollowFormatAllowed )
        {
            mrTextFrm.AllowFollowFormat();
        }
    }
};

SwObjectFormatterTextFrm::SwObjectFormatterTextFrm( SwTextFrm& _rAnchorTextFrm,
                                                  const SwPageFrm& _rPageFrm,
                                                  SwTextFrm* _pMasterAnchorTextFrm,
                                                  SwLayAction* _pLayAction )
    : SwObjectFormatter( _rPageFrm, _pLayAction, true ),
      mrAnchorTextFrm( _rAnchorTextFrm ),
      mpMasterAnchorTextFrm( _pMasterAnchorTextFrm )
{
}

SwObjectFormatterTextFrm::~SwObjectFormatterTextFrm()
{
}

SwObjectFormatterTextFrm* SwObjectFormatterTextFrm::CreateObjFormatter(
                                                SwTextFrm& _rAnchorTextFrm,
                                                const SwPageFrm& _rPageFrm,
                                                SwLayAction* _pLayAction )
{
    SwObjectFormatterTextFrm* pObjFormatter = nullptr;

    // determine 'master' of <_rAnchorTextFrm>, if anchor frame is a follow text frame.
    SwTextFrm* pMasterOfAnchorFrm = nullptr;
    if ( _rAnchorTextFrm.IsFollow() )
    {
        pMasterOfAnchorFrm = _rAnchorTextFrm.FindMaster();
        while ( pMasterOfAnchorFrm && pMasterOfAnchorFrm->IsFollow() )
        {
            pMasterOfAnchorFrm = pMasterOfAnchorFrm->FindMaster();
        }
    }

    // create object formatter, if floating screen objects are registered
    // at anchor frame (or at 'master' anchor frame)
    if ( _rAnchorTextFrm.GetDrawObjs() ||
         ( pMasterOfAnchorFrm && pMasterOfAnchorFrm->GetDrawObjs() ) )
    {
        pObjFormatter =
            new SwObjectFormatterTextFrm( _rAnchorTextFrm, _rPageFrm,
                                         pMasterOfAnchorFrm, _pLayAction );
    }

    return pObjFormatter;
}

SwFrm& SwObjectFormatterTextFrm::GetAnchorFrm()
{
    return mrAnchorTextFrm;
}

// #i40147# - add parameter <_bCheckForMovedFwd>.
bool SwObjectFormatterTextFrm::DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                           const bool _bCheckForMovedFwd )
{
    // check, if only as-character anchored object have to be formatted, and
    // check the anchor type
    if ( FormatOnlyAsCharAnchored() &&
         !(_rAnchoredObj.GetFrameFormat().GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
    {
        return true;
    }

    // consider, if the layout action has to be
    // restarted due to a delete of a page frame.
    if ( GetLayAction() && GetLayAction()->IsAgain() )
    {
        return false;
    }

    bool bSuccess( true );

    if ( _rAnchoredObj.IsFormatPossible() )
    {
        _rAnchoredObj.SetRestartLayoutProcess( false );

        _FormatObj( _rAnchoredObj );
        // consider, if the layout action has to be
        // restarted due to a delete of a page frame.
        if ( GetLayAction() && GetLayAction()->IsAgain() )
        {
            return false;
        }

        // check, if layout process has to be restarted.
        // if yes, perform needed invalidations.

        // no restart of layout process,
        // if anchored object is anchored inside a Writer fly frame,
        // its position is already locked, and it follows the text flow.
        const bool bRestart =
                _rAnchoredObj.RestartLayoutProcess() &&
                !( _rAnchoredObj.PositionLocked() &&
                   _rAnchoredObj.GetAnchorFrm()->IsInFly() &&
                   _rAnchoredObj.GetFrameFormat().GetFollowTextFlow().GetValue() );
        if ( bRestart )
        {
            bSuccess = false;
            _InvalidatePrevObjs( _rAnchoredObj );
            _InvalidateFollowObjs( _rAnchoredObj, true );
        }

        // format anchor text frame, if wrapping style influence of the object
        // has to be considered and it's <NONE_SUCCESSIVE_POSITIONED>
        // #i3317# - consider also anchored objects, whose
        // wrapping style influence is temporarly considered.
        // #i40147# - consider also anchored objects, for
        // whose the check of a moved forward anchor frame is requested.
        // revise decision made for i3317:
        // anchored objects, whose wrapping style influence is temporarly considered,
        // have to be considered in method <SwObjectFormatterTextFrm::DoFormatObjs()>
        if ( bSuccess &&
             _rAnchoredObj.ConsiderObjWrapInfluenceOnObjPos() &&
             ( _bCheckForMovedFwd ||
               _rAnchoredObj.GetFrameFormat().GetWrapInfluenceOnObjPos().
                    // #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                    GetWrapInfluenceOnObjPos( true ) ==
                        // #i35017# - constant name has changed
                        text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ) )
        {
            // #i26945# - check conditions for move forward of
            // anchor text frame
            // determine, if anchor text frame has previous frame
            const bool bDoesAnchorHadPrev = ( mrAnchorTextFrm.GetIndPrev() != nullptr );

            // #i40141# - use new method - it also formats the
            // section the anchor frame is in.
            _FormatAnchorFrmForCheckMoveFwd();

            // #i35911#
            if ( _rAnchoredObj.HasClearedEnvironment() )
            {
                _rAnchoredObj.SetClearedEnvironment( true );
                // #i44049# - consider, that anchor frame
                // could already been marked to move forward.
                SwPageFrm* pAnchorPageFrm( mrAnchorTextFrm.FindPageFrm() );
                if ( pAnchorPageFrm != _rAnchoredObj.GetPageFrm() )
                {
                    bool bInsert( true );
                    sal_uInt32 nToPageNum( 0L );
                    const SwDoc& rDoc = *(GetPageFrm().GetFormat()->GetDoc());
                    if ( SwLayouter::FrmMovedFwdByObjPos(
                                            rDoc, mrAnchorTextFrm, nToPageNum ) )
                    {
                        if ( nToPageNum < pAnchorPageFrm->GetPhyPageNum() )
                            SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTextFrm );
                        else
                            bInsert = false;
                    }
                    if ( bInsert )
                    {
                        SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTextFrm,
                                                       pAnchorPageFrm->GetPhyPageNum() );
                        mrAnchorTextFrm.InvalidatePos();
                        bSuccess = false;
                        _InvalidatePrevObjs( _rAnchoredObj );
                        _InvalidateFollowObjs( _rAnchoredObj, true );
                    }
                    else
                    {
                        OSL_FAIL( "<SwObjectFormatterTextFrm::DoFormatObj(..)> - anchor frame not marked to move forward" );
                    }
                }
            }
            else if ( !mrAnchorTextFrm.IsFollow() && bDoesAnchorHadPrev )
            {
                // index of anchored object in collection of page numbers and
                // anchor types
                sal_uInt32 nIdx( CountOfCollected() );
                OSL_ENSURE( nIdx > 0,
                        "<SwObjectFormatterTextFrm::DoFormatObj(..)> - anchored object not collected!?" );
                --nIdx;

                sal_uInt32 nToPageNum( 0L );
                // #i43913#
                bool bDummy( false );
                // #i58182# - consider new method signature
                if ( SwObjectFormatterTextFrm::CheckMovedFwdCondition( *GetCollectedObj( nIdx ),
                                              GetPgNumOfCollected( nIdx ),
                                              IsCollectedAnchoredAtMaster( nIdx ),
                                              nToPageNum, bDummy ) )
                {
                    // #i49987# - consider, that anchor frame
                    // could already been marked to move forward.
                    bool bInsert( true );
                    sal_uInt32 nMovedFwdToPageNum( 0L );
                    const SwDoc& rDoc = *(GetPageFrm().GetFormat()->GetDoc());
                    if ( SwLayouter::FrmMovedFwdByObjPos(
                                            rDoc, mrAnchorTextFrm, nMovedFwdToPageNum ) )
                    {
                        if ( nMovedFwdToPageNum < nToPageNum )
                            SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTextFrm );
                        else
                            bInsert = false;
                    }
                    if ( bInsert )
                    {
                        // Indicate that anchor text frame has to move forward and
                        // invalidate its position to force a re-format.
                        SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTextFrm,
                                                       nToPageNum );
                        mrAnchorTextFrm.InvalidatePos();

                        // Indicate restart of the layout process
                        bSuccess = false;

                        // If needed, invalidate previous objects anchored at same anchor
                        // text frame.
                        _InvalidatePrevObjs( _rAnchoredObj );

                        // Invalidate object and following objects for the restart of the
                        // layout process
                        _InvalidateFollowObjs( _rAnchoredObj, true );
                    }
                    else
                    {
                        OSL_FAIL( "<SwObjectFormatterTextFrm::DoFormatObj(..)> - anchor frame not marked to move forward" );
                    }
                }
            }
            // i40155# - mark anchor frame not to wrap around
            // objects under the condition, that its follow contains all its text.
            else if ( !mrAnchorTextFrm.IsFollow() &&
                      mrAnchorTextFrm.GetFollow() &&
                      mrAnchorTextFrm.GetFollow()->GetOfst() == 0 )
            {
                SwLayouter::RemoveMovedFwdFrm(
                                *(mrAnchorTextFrm.FindPageFrm()->GetFormat()->GetDoc()),
                                mrAnchorTextFrm );
            }
        }
    }

    return bSuccess;
}

bool SwObjectFormatterTextFrm::DoFormatObjs()
{
    if ( !mrAnchorTextFrm.IsValid() )
    {
        if ( GetLayAction() &&
             mrAnchorTextFrm.FindPageFrm() != &GetPageFrm() )
        {
            // notify layout action, thus is can restart the layout process on
            // a previous page.
            GetLayAction()->SetAgain();
        }
        else
        {
            // the anchor text frame has to be valid, thus assert.
            OSL_FAIL( "<SwObjectFormatterTextFrm::DoFormatObjs()> called for invalidate anchor text frame." );
        }

        return false;
    }

    bool bSuccess( true );

    if ( mrAnchorTextFrm.IsFollow() )
    {
        // Only floating screen objects anchored as-character are directly
        // registered at a follow text frame. The other floating screen objects
        // are registered at the 'master' anchor text frame.
        // Thus, format the other floating screen objects through the 'master'
        // anchor text frame
        OSL_ENSURE( mpMasterAnchorTextFrm,
                "SwObjectFormatterTextFrm::DoFormatObjs() - missing 'master' anchor text frame" );
        bSuccess = _FormatObjsAtFrm( mpMasterAnchorTextFrm );

        if ( bSuccess )
        {
            // format of as-character anchored floating screen objects - no failure
            // excepted on the format of these objects.
            bSuccess = _FormatObjsAtFrm();
        }
    }
    else
    {
        bSuccess = _FormatObjsAtFrm();
    }

    // consider anchored objects, whose wrapping style influence are temporarly
    // considered.
    if ( bSuccess &&
         ( ConsiderWrapOnObjPos() ||
           ( !mrAnchorTextFrm.IsFollow() &&
             _AtLeastOneObjIsTmpConsiderWrapInfluence() ) ) )
    {
        const bool bDoesAnchorHadPrev = ( mrAnchorTextFrm.GetIndPrev() != nullptr );

        // Format anchor text frame after its objects are formatted.
        // Note: The format of the anchor frame also formats the invalid
        //       previous frames of the anchor frame. The format of the previous
        //       frames is needed to get a correct result of format of the
        //       anchor frame for the following check for moved forward anchors
        // #i40141# - use new method - it also formats the
        // section the anchor frame is in.
        _FormatAnchorFrmForCheckMoveFwd();

        sal_uInt32 nToPageNum( 0L );
        // #i43913#
        bool bInFollow( false );
        SwAnchoredObject* pObj = nullptr;
        if ( !mrAnchorTextFrm.IsFollow() )
        {
            pObj = _GetFirstObjWithMovedFwdAnchor(
                    // #i35017# - constant name has changed
                    text::WrapInfluenceOnPosition::ONCE_CONCURRENT,
                    nToPageNum, bInFollow );
        }
        // #i35911#
        if ( pObj && pObj->HasClearedEnvironment() )
        {
            pObj->SetClearedEnvironment( true );
            // #i44049# - consider, that anchor frame
            // could already been marked to move forward.
            SwPageFrm* pAnchorPageFrm( mrAnchorTextFrm.FindPageFrm() );
            // #i43913# - consider, that anchor frame
            // is a follow or is in a follow row, which will move forward.
            if ( pAnchorPageFrm != pObj->GetPageFrm() ||
                 bInFollow )
            {
                bool bInsert( true );
                sal_uInt32 nTmpToPageNum( 0L );
                const SwDoc& rDoc = *(GetPageFrm().GetFormat()->GetDoc());
                if ( SwLayouter::FrmMovedFwdByObjPos(
                                        rDoc, mrAnchorTextFrm, nTmpToPageNum ) )
                {
                    if ( nTmpToPageNum < pAnchorPageFrm->GetPhyPageNum() )
                        SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTextFrm );
                    else
                        bInsert = false;
                }
                if ( bInsert )
                {
                    SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTextFrm,
                                                   pAnchorPageFrm->GetPhyPageNum() );
                    mrAnchorTextFrm.InvalidatePos();
                    bSuccess = false;
                    _InvalidatePrevObjs( *pObj );
                    _InvalidateFollowObjs( *pObj, true );
                }
                else
                {
                    OSL_FAIL( "<SwObjectFormatterTextFrm::DoFormatObjs(..)> - anchor frame not marked to move forward" );
                }
            }
        }
        else if ( pObj && bDoesAnchorHadPrev )
        {
            // Object found, whose anchor is moved forward

            // #i49987# - consider, that anchor frame
            // could already been marked to move forward.
            bool bInsert( true );
            sal_uInt32 nMovedFwdToPageNum( 0L );
            const SwDoc& rDoc = *(GetPageFrm().GetFormat()->GetDoc());
            if ( SwLayouter::FrmMovedFwdByObjPos(
                                    rDoc, mrAnchorTextFrm, nMovedFwdToPageNum ) )
            {
                if ( nMovedFwdToPageNum < nToPageNum )
                    SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTextFrm );
                else
                    bInsert = false;
            }
            if ( bInsert )
            {
                // Indicate that anchor text frame has to move forward and
                // invalidate its position to force a re-format.
                SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTextFrm, nToPageNum );
                mrAnchorTextFrm.InvalidatePos();

                // Indicate restart of the layout process
                bSuccess = false;

                // If needed, invalidate previous objects anchored at same anchor
                // text frame.
                _InvalidatePrevObjs( *pObj );

                // Invalidate object and following objects for the restart of the
                // layout process
                _InvalidateFollowObjs( *pObj, true );
            }
            else
            {
                OSL_FAIL( "<SwObjectFormatterTextFrm::DoFormatObjs(..)> - anchor frame not marked to move forward" );
            }
        }
        // #i40155# - mark anchor frame not to wrap around
        // objects under the condition, that its follow contains all its text.
        else if ( !mrAnchorTextFrm.IsFollow() &&
                  mrAnchorTextFrm.GetFollow() &&
                  mrAnchorTextFrm.GetFollow()->GetOfst() == 0 )
        {
            SwLayouter::RemoveMovedFwdFrm(
                            *(mrAnchorTextFrm.FindPageFrm()->GetFormat()->GetDoc()),
                            mrAnchorTextFrm );
        }
    }

    return bSuccess;
}

void SwObjectFormatterTextFrm::_InvalidatePrevObjs( SwAnchoredObject& _rAnchoredObj )
{
    // invalidate all previous objects, whose wrapping influence on the object
    // positioning is <NONE_CONCURRENT_POSITIONED>.
    // Note: list of objects at anchor frame is sorted by this property.
    if ( _rAnchoredObj.GetFrameFormat().GetWrapInfluenceOnObjPos().
                // #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                GetWrapInfluenceOnObjPos( true ) ==
                            // #i35017# - constant name has changed
                            text::WrapInfluenceOnPosition::ONCE_CONCURRENT )
    {
        const SwSortedObjs* pObjs = GetAnchorFrm().GetDrawObjs();
        if ( pObjs )
        {
            // determine start index
            size_t i = pObjs->ListPosOf( _rAnchoredObj );
            while (i > 0)
            {
                --i;
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                if ( pAnchoredObj->GetFrameFormat().GetWrapInfluenceOnObjPos().
                        // #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                        GetWrapInfluenceOnObjPos( true ) ==
                            // #i35017# - constant name has changed
                            text::WrapInfluenceOnPosition::ONCE_CONCURRENT )
                {
                    pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
                }
            }
        }
    }
}

void SwObjectFormatterTextFrm::_InvalidateFollowObjs( SwAnchoredObject& _rAnchoredObj,
                                                     const bool _bInclObj )
{
    if ( _bInclObj )
    {
        _rAnchoredObj.InvalidateObjPosForConsiderWrapInfluence( true );
    }

    const SwSortedObjs* pObjs = GetPageFrm().GetSortedObjs();
    if ( pObjs )
    {
        // determine start index
        for ( size_t i = pObjs->ListPosOf( _rAnchoredObj ) + 1; i < pObjs->size(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
        }
    }
}

SwAnchoredObject* SwObjectFormatterTextFrm::_GetFirstObjWithMovedFwdAnchor(
                                    const sal_Int16 _nWrapInfluenceOnPosition,
                                    sal_uInt32& _noToPageNum,
                                    bool& _boInFollow )
{
    // #i35017# - constant names have changed
    OSL_ENSURE( _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ||
            _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_CONCURRENT,
            "<SwObjectFormatterTextFrm::_GetFirstObjWithMovedFwdAnchor(..)> - invalid value for parameter <_nWrapInfluenceOnPosition>" );

    SwAnchoredObject* pRetAnchoredObj = nullptr;

    sal_uInt32 i = 0L;
    for ( ; i < CountOfCollected(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = GetCollectedObj(i);
        if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() &&
             pAnchoredObj->GetFrameFormat().GetWrapInfluenceOnObjPos().
                    // #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                    GetWrapInfluenceOnObjPos( true ) == _nWrapInfluenceOnPosition )
        {
            // #i26945# - use new method <_CheckMovedFwdCondition(..)>
            // #i43913#
            // #i58182# - consider new method signature
            if ( SwObjectFormatterTextFrm::CheckMovedFwdCondition( *GetCollectedObj( i ),
                                          GetPgNumOfCollected( i ),
                                          IsCollectedAnchoredAtMaster( i ),
                                          _noToPageNum, _boInFollow ) )
            {
                pRetAnchoredObj = pAnchoredObj;
                break;
            }
        }
    }

    return pRetAnchoredObj;
}

// #i58182#
// - replace private method by corresponding static public method
bool SwObjectFormatterTextFrm::CheckMovedFwdCondition(
                                            SwAnchoredObject& _rAnchoredObj,
                                            const sal_uInt32 _nFromPageNum,
                                            const bool _bAnchoredAtMasterBeforeFormatAnchor,
                                            sal_uInt32& _noToPageNum,
                                            bool& _boInFollow )
{
    bool bAnchorIsMovedForward( false );

    SwPageFrm* pPageFrmOfAnchor = _rAnchoredObj.FindPageFrmOfAnchor();
    if ( pPageFrmOfAnchor )
    {
        const sal_uInt32 nPageNum = pPageFrmOfAnchor->GetPhyPageNum();
        if ( nPageNum > _nFromPageNum )
        {
            _noToPageNum = nPageNum;
            // Handling of special case:
            // If anchor frame is move forward into a follow flow row,
            // <_noToPageNum> is set to <_nFromPageNum + 1>, because it is
            // possible that the anchor page frame isn't valid, because the
            // page distance between master row and follow flow row is greater
            // than 1.
            if ( _noToPageNum > (_nFromPageNum + 1) )
            {
                SwFrm* pAnchorFrm = _rAnchoredObj.GetAnchorFrmContainingAnchPos();
                if ( pAnchorFrm->IsInTab() &&
                     pAnchorFrm->IsInFollowFlowRow() )
                {
                    _noToPageNum = _nFromPageNum + 1;
                }
            }
            bAnchorIsMovedForward = true;
        }
    }
    // #i26945# - check, if an at-paragraph|at-character
    // anchored object is now anchored at a follow text frame, which will be
    // on the next page. Also check, if an at-character anchored object
    // is now anchored at a text frame,  which is in a follow flow row,
    // which will be on the next page.
    if ( !bAnchorIsMovedForward &&
         _bAnchoredAtMasterBeforeFormatAnchor &&
        ((_rAnchoredObj.GetFrameFormat().GetAnchor().GetAnchorId() == FLY_AT_CHAR) ||
         (_rAnchoredObj.GetFrameFormat().GetAnchor().GetAnchorId() == FLY_AT_PARA)))
    {
        SwFrm* pAnchorFrm = _rAnchoredObj.GetAnchorFrmContainingAnchPos();
        OSL_ENSURE( pAnchorFrm->IsTextFrm(),
                "<SwObjectFormatterTextFrm::CheckMovedFwdCondition(..) - wrong type of anchor frame>" );
        SwTextFrm* pAnchorTextFrm = static_cast<SwTextFrm*>(pAnchorFrm);
        bool bCheck( false );
        if ( pAnchorTextFrm->IsFollow() )
        {
            bCheck = true;
        }
        else if( pAnchorTextFrm->IsInTab() )
        {
            const SwRowFrm* pMasterRow = pAnchorTextFrm->IsInFollowFlowRow();
            if ( pMasterRow &&
                 pMasterRow->FindPageFrm() == pPageFrmOfAnchor )
            {
                bCheck = true;
            }
        }
        if ( bCheck )
        {
            // check, if found text frame will be on the next page
            // by checking, if it's in a column, which has no next.
            SwFrm* pColFrm = pAnchorTextFrm->FindColFrm();
            while ( pColFrm && !pColFrm->GetNext() )
            {
                pColFrm = pColFrm->FindColFrm();
            }
            if ( !pColFrm || !pColFrm->GetNext() )
            {
                _noToPageNum = _nFromPageNum + 1;
                bAnchorIsMovedForward = true;
                // #i43913#
                _boInFollow = true;
            }
        }
    }

    return bAnchorIsMovedForward;
}

// #i40140# - helper method to format layout frames used by
// method <SwObjectFormatterTextFrm::_FormatAnchorFrmForCheckMoveFwd()>
// #i44049# - format till a certain lower frame, if provided.
static void lcl_FormatContentOfLayoutFrm( SwLayoutFrm* pLayFrm,
                                 SwFrm* pLastLowerFrm = nullptr )
{
    SwFrm* pLowerFrm = pLayFrm->GetLower();
    while ( pLowerFrm )
    {
        // #i44049#
        if ( pLastLowerFrm && pLowerFrm == pLastLowerFrm )
        {
            break;
        }
        if ( pLowerFrm->IsLayoutFrm() )
            lcl_FormatContentOfLayoutFrm( static_cast<SwLayoutFrm*>(pLowerFrm),
                                        pLastLowerFrm );
        else
            pLowerFrm->Calc(pLowerFrm->getRootFrm()->GetCurrShell()->GetOut());

        pLowerFrm = pLowerFrm->GetNext();
    }
}

/** method to format given anchor text frame and its previous frames

    #i56300#
    Usage: Needed to check, if the anchor text frame is moved forward
    due to the positioning and wrapping of its anchored objects, and
    to format the frames, which have become invalid due to the anchored
    object formatting in the iterative object positioning algorithm
*/
void SwObjectFormatterTextFrm::FormatAnchorFrmAndItsPrevs( SwTextFrm& _rAnchorTextFrm )
{
    // #i47014# - no format of section and previous columns
    // for follow text frames.
    if ( !_rAnchorTextFrm.IsFollow() )
    {
        // if anchor frame is directly inside a section, format this section and
        // its previous frames.
        // Note: It's a very simple format without formatting objects.
        if ( _rAnchorTextFrm.IsInSct() )
        {
            SwFrm* pSectFrm = _rAnchorTextFrm.GetUpper();
            while ( pSectFrm )
            {
                if ( pSectFrm->IsSctFrm() || pSectFrm->IsCellFrm() )
                {
                    break;
                }
                pSectFrm = pSectFrm->GetUpper();
            }
            if ( pSectFrm && pSectFrm->IsSctFrm() )
            {
                // #i44049#
                _rAnchorTextFrm.LockJoin();
                SwFrm* pFrm = pSectFrm->GetUpper()->GetLower();
                // #i49605# - section frame could move forward
                // by the format of its previous frame.
                // Thus, check for valid <pFrm>.
                while ( pFrm && pFrm != pSectFrm )
                {
                    if ( pFrm->IsLayoutFrm() )
                        lcl_FormatContentOfLayoutFrm( static_cast<SwLayoutFrm*>(pFrm) );
                    else
                        pFrm->Calc(pFrm->getRootFrm()->GetCurrShell()->GetOut());

                    pFrm = pFrm->GetNext();
                }
                lcl_FormatContentOfLayoutFrm( static_cast<SwLayoutFrm*>(pSectFrm),
                                            &_rAnchorTextFrm );
                // #i44049#
                _rAnchorTextFrm.UnlockJoin();
            }
        }

        // #i40140# - if anchor frame is inside a column,
        // format the content of the previous columns.
        // Note: It's a very simple format without formatting objects.
        SwFrm* pColFrmOfAnchor = _rAnchorTextFrm.FindColFrm();
        if ( pColFrmOfAnchor )
        {
            // #i44049#
            _rAnchorTextFrm.LockJoin();
            SwFrm* pColFrm = pColFrmOfAnchor->GetUpper()->GetLower();
            while ( pColFrm != pColFrmOfAnchor )
            {
                SwFrm* pFrm = pColFrm->GetLower();
                while ( pFrm )
                {
                    if ( pFrm->IsLayoutFrm() )
                        lcl_FormatContentOfLayoutFrm( static_cast<SwLayoutFrm*>(pFrm) );
                    else
                        pFrm->Calc(pFrm->getRootFrm()->GetCurrShell()->GetOut());

                    pFrm = pFrm->GetNext();
                }

                pColFrm = pColFrm->GetNext();
            }
            // #i44049#
            _rAnchorTextFrm.UnlockJoin();
        }
    }

    // format anchor frame - format of its follow not needed
    // #i43255# - forbid follow format, only if anchor text
    // frame is in table
    if ( _rAnchorTextFrm.IsInTab() )
    {
        SwForbidFollowFormat aForbidFollowFormat( _rAnchorTextFrm );
        _rAnchorTextFrm.Calc(_rAnchorTextFrm.getRootFrm()->GetCurrShell()->GetOut());
    }
    else
    {
        _rAnchorTextFrm.Calc(_rAnchorTextFrm.getRootFrm()->GetCurrShell()->GetOut());
    }
}

/** method to format the anchor frame for checking of the move forward condition

    #i40141#
*/
void SwObjectFormatterTextFrm::_FormatAnchorFrmForCheckMoveFwd()
{
    SwObjectFormatterTextFrm::FormatAnchorFrmAndItsPrevs( mrAnchorTextFrm );
}

/** method to determine if at least one anchored object has state
    <temporarly consider wrapping style influence> set.
*/
bool SwObjectFormatterTextFrm::_AtLeastOneObjIsTmpConsiderWrapInfluence()
{
    bool bRet( false );

    const SwSortedObjs* pObjs = GetAnchorFrm().GetDrawObjs();
    if ( pObjs && pObjs->size() > 1 )
    {
        for ( size_t i = 0; i < pObjs->size(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() )
            {
                bRet = true;
                break;
            }
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
