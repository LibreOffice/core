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

#include <objectformattertxtfrm.hxx>
#include <anchoredobject.hxx>
#include <sortedobjs.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <rowfrm.hxx>
#include <layouter.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <fmtfollowtextflow.hxx>
#include <layact.hxx>

using namespace ::com::sun::star;

// =============================================================================

// little helper class to forbid follow formatting for the given text frame
class SwForbidFollowFormat
{
private:
    SwTxtFrm& mrTxtFrm;
    const bool bOldFollowFormatAllowed;

public:
    SwForbidFollowFormat( SwTxtFrm& _rTxtFrm )
        : mrTxtFrm( _rTxtFrm ),
          bOldFollowFormatAllowed( _rTxtFrm.FollowFormatAllowed() )
    {
        mrTxtFrm.ForbidFollowFormat();
    }

    ~SwForbidFollowFormat()
    {
        if ( bOldFollowFormatAllowed )
        {
            mrTxtFrm.AllowFollowFormat();
        }
    }
};

// =============================================================================
// implementation of class <SwObjectFormatterTxtFrm>
// =============================================================================
SwObjectFormatterTxtFrm::SwObjectFormatterTxtFrm( SwTxtFrm& _rAnchorTxtFrm,
                                                  const SwPageFrm& _rPageFrm,
                                                  SwTxtFrm* _pMasterAnchorTxtFrm,
                                                  SwLayAction* _pLayAction )
    : SwObjectFormatter( _rPageFrm, _pLayAction, true ),
      mrAnchorTxtFrm( _rAnchorTxtFrm ),
      mpMasterAnchorTxtFrm( _pMasterAnchorTxtFrm )
{
}

SwObjectFormatterTxtFrm::~SwObjectFormatterTxtFrm()
{
}

SwObjectFormatterTxtFrm* SwObjectFormatterTxtFrm::CreateObjFormatter(
                                                SwTxtFrm& _rAnchorTxtFrm,
                                                const SwPageFrm& _rPageFrm,
                                                SwLayAction* _pLayAction )
{
    SwObjectFormatterTxtFrm* pObjFormatter = 0L;

    // determine 'master' of <_rAnchorTxtFrm>, if anchor frame is a follow text frame.
    SwTxtFrm* pMasterOfAnchorFrm = 0L;
    if ( _rAnchorTxtFrm.IsFollow() )
    {
        pMasterOfAnchorFrm = _rAnchorTxtFrm.FindMaster();
        while ( pMasterOfAnchorFrm && pMasterOfAnchorFrm->IsFollow() )
        {
            pMasterOfAnchorFrm = pMasterOfAnchorFrm->FindMaster();
        }
    }

    // create object formatter, if floating screen objects are registered
    // at anchor frame (or at 'master' anchor frame)
    if ( _rAnchorTxtFrm.GetDrawObjs() ||
         ( pMasterOfAnchorFrm && pMasterOfAnchorFrm->GetDrawObjs() ) )
    {
        pObjFormatter =
            new SwObjectFormatterTxtFrm( _rAnchorTxtFrm, _rPageFrm,
                                         pMasterOfAnchorFrm, _pLayAction );
    }

    return pObjFormatter;
}

SwFrm& SwObjectFormatterTxtFrm::GetAnchorFrm()
{
    return mrAnchorTxtFrm;
}

// #i40147# - add parameter <_bCheckForMovedFwd>.
bool SwObjectFormatterTxtFrm::DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                           const bool _bCheckForMovedFwd )
{
    // check, if only as-character anchored object have to be formatted, and
    // check the anchor type
    if ( FormatOnlyAsCharAnchored() &&
         !(_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
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
                   _rAnchoredObj.GetFrmFmt().GetFollowTextFlow().GetValue() );
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
        // have to be considered in method <SwObjectFormatterTxtFrm::DoFormatObjs()>
        if ( bSuccess &&
             _rAnchoredObj.ConsiderObjWrapInfluenceOnObjPos() &&
             ( _bCheckForMovedFwd ||
               _rAnchoredObj.GetFrmFmt().GetWrapInfluenceOnObjPos().
                    // #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                    GetWrapInfluenceOnObjPos( true ) ==
                        // #i35017# - constant name has changed
                        text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ) )
        {
            // #i26945# - check conditions for move forward of
            // anchor text frame
            // determine, if anchor text frame has previous frame
            const bool bDoesAnchorHadPrev = ( mrAnchorTxtFrm.GetIndPrev() != 0 );

            // #i40141# - use new method - it also formats the
            // section the anchor frame is in.
            _FormatAnchorFrmForCheckMoveFwd();

            // #i35911#
            if ( _rAnchoredObj.HasClearedEnvironment() )
            {
                _rAnchoredObj.SetClearedEnvironment( true );
                // #i44049# - consider, that anchor frame
                // could already been marked to move forward.
                SwPageFrm* pAnchorPageFrm( mrAnchorTxtFrm.FindPageFrm() );
                if ( pAnchorPageFrm != _rAnchoredObj.GetPageFrm() )
                {
                    bool bInsert( true );
                    sal_uInt32 nToPageNum( 0L );
                    const SwDoc& rDoc = *(GetPageFrm().GetFmt()->GetDoc());
                    if ( SwLayouter::FrmMovedFwdByObjPos(
                                            rDoc, mrAnchorTxtFrm, nToPageNum ) )
                    {
                        if ( nToPageNum < pAnchorPageFrm->GetPhyPageNum() )
                            SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTxtFrm );
                        else
                            bInsert = false;
                    }
                    if ( bInsert )
                    {
                        SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTxtFrm,
                                                       pAnchorPageFrm->GetPhyPageNum() );
                        mrAnchorTxtFrm.InvalidatePos();
                        bSuccess = false;
                        _InvalidatePrevObjs( _rAnchoredObj );
                        _InvalidateFollowObjs( _rAnchoredObj, true );
                    }
                    else
                    {
                        OSL_FAIL( "<SwObjectFormatterTxtFrm::DoFormatObj(..)> - anchor frame not marked to move forward" );
                    }
                }
            }
            else if ( !mrAnchorTxtFrm.IsFollow() && bDoesAnchorHadPrev )
            {
                // index of anchored object in collection of page numbers and
                // anchor types
                sal_uInt32 nIdx( CountOfCollected() );
                OSL_ENSURE( nIdx > 0,
                        "<SwObjectFormatterTxtFrm::DoFormatObj(..)> - anchored object not collected!?" );
                --nIdx;

                sal_uInt32 nToPageNum( 0L );
                // #i43913#
                bool bDummy( false );
                // #i58182# - consider new method signature
                if ( SwObjectFormatterTxtFrm::CheckMovedFwdCondition( *GetCollectedObj( nIdx ),
                                              GetPgNumOfCollected( nIdx ),
                                              IsCollectedAnchoredAtMaster( nIdx ),
                                              nToPageNum, bDummy ) )
                {
                    // #i49987# - consider, that anchor frame
                    // could already been marked to move forward.
                    bool bInsert( true );
                    sal_uInt32 nMovedFwdToPageNum( 0L );
                    const SwDoc& rDoc = *(GetPageFrm().GetFmt()->GetDoc());
                    if ( SwLayouter::FrmMovedFwdByObjPos(
                                            rDoc, mrAnchorTxtFrm, nMovedFwdToPageNum ) )
                    {
                        if ( nMovedFwdToPageNum < nToPageNum )
                            SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTxtFrm );
                        else
                            bInsert = false;
                    }
                    if ( bInsert )
                    {
                        // Indicate that anchor text frame has to move forward and
                        // invalidate its position to force a re-format.
                        SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTxtFrm,
                                                       nToPageNum );
                        mrAnchorTxtFrm.InvalidatePos();

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
                        OSL_FAIL( "<SwObjectFormatterTxtFrm::DoFormatObj(..)> - anchor frame not marked to move forward" );
                    }
                }
            }
            // i40155# - mark anchor frame not to wrap around
            // objects under the condition, that its follow contains all its text.
            else if ( !mrAnchorTxtFrm.IsFollow() &&
                      mrAnchorTxtFrm.GetFollow() &&
                      mrAnchorTxtFrm.GetFollow()->GetOfst() == 0 )
            {
                SwLayouter::RemoveMovedFwdFrm(
                                *(mrAnchorTxtFrm.FindPageFrm()->GetFmt()->GetDoc()),
                                mrAnchorTxtFrm );
            }
        }
    }

    return bSuccess;
}

bool SwObjectFormatterTxtFrm::DoFormatObjs()
{
    if ( !mrAnchorTxtFrm.IsValid() )
    {
        if ( GetLayAction() &&
             mrAnchorTxtFrm.FindPageFrm() != &GetPageFrm() )
        {
            // notify layout action, thus is can restart the layout process on
            // a previous page.
            GetLayAction()->SetAgain();
        }
        else
        {
            // the anchor text frame has to be valid, thus assert.
            OSL_FAIL( "<SwObjectFormatterTxtFrm::DoFormatObjs()> called for invalidate anchor text frame." );
        }

        return false;
    }

    bool bSuccess( true );

    if ( mrAnchorTxtFrm.IsFollow() )
    {
        // Only floating screen objects anchored as-character are directly
        // registered at a follow text frame. The other floating screen objects
        // are registered at the 'master' anchor text frame.
        // Thus, format the other floating screen objects through the 'master'
        // anchor text frame
        OSL_ENSURE( mpMasterAnchorTxtFrm,
                "SwObjectFormatterTxtFrm::DoFormatObjs() - missing 'master' anchor text frame" );
        bSuccess = _FormatObjsAtFrm( mpMasterAnchorTxtFrm );

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
           ( !mrAnchorTxtFrm.IsFollow() &&
             _AtLeastOneObjIsTmpConsiderWrapInfluence() ) ) )
    {
        const bool bDoesAnchorHadPrev = ( mrAnchorTxtFrm.GetIndPrev() != 0 );

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
        SwAnchoredObject* pObj = 0L;
        if ( !mrAnchorTxtFrm.IsFollow() )
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
            SwPageFrm* pAnchorPageFrm( mrAnchorTxtFrm.FindPageFrm() );
            // #i43913# - consider, that anchor frame
            // is a follow or is in a follow row, which will move forward.
            if ( pAnchorPageFrm != pObj->GetPageFrm() ||
                 bInFollow )
            {
                bool bInsert( true );
                sal_uInt32 nTmpToPageNum( 0L );
                const SwDoc& rDoc = *(GetPageFrm().GetFmt()->GetDoc());
                if ( SwLayouter::FrmMovedFwdByObjPos(
                                        rDoc, mrAnchorTxtFrm, nTmpToPageNum ) )
                {
                    if ( nTmpToPageNum < pAnchorPageFrm->GetPhyPageNum() )
                        SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTxtFrm );
                    else
                        bInsert = false;
                }
                if ( bInsert )
                {
                    SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTxtFrm,
                                                   pAnchorPageFrm->GetPhyPageNum() );
                    mrAnchorTxtFrm.InvalidatePos();
                    bSuccess = false;
                    _InvalidatePrevObjs( *pObj );
                    _InvalidateFollowObjs( *pObj, true );
                }
                else
                {
                    OSL_FAIL( "<SwObjectFormatterTxtFrm::DoFormatObjs(..)> - anchor frame not marked to move forward" );
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
            const SwDoc& rDoc = *(GetPageFrm().GetFmt()->GetDoc());
            if ( SwLayouter::FrmMovedFwdByObjPos(
                                    rDoc, mrAnchorTxtFrm, nMovedFwdToPageNum ) )
            {
                if ( nMovedFwdToPageNum < nToPageNum )
                    SwLayouter::RemoveMovedFwdFrm( rDoc, mrAnchorTxtFrm );
                else
                    bInsert = false;
            }
            if ( bInsert )
            {
                // Indicate that anchor text frame has to move forward and
                // invalidate its position to force a re-format.
                SwLayouter::InsertMovedFwdFrm( rDoc, mrAnchorTxtFrm, nToPageNum );
                mrAnchorTxtFrm.InvalidatePos();

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
                OSL_FAIL( "<SwObjectFormatterTxtFrm::DoFormatObjs(..)> - anchor frame not marked to move forward" );
            }
        }
        // #i40155# - mark anchor frame not to wrap around
        // objects under the condition, that its follow contains all its text.
        else if ( !mrAnchorTxtFrm.IsFollow() &&
                  mrAnchorTxtFrm.GetFollow() &&
                  mrAnchorTxtFrm.GetFollow()->GetOfst() == 0 )
        {
            SwLayouter::RemoveMovedFwdFrm(
                            *(mrAnchorTxtFrm.FindPageFrm()->GetFmt()->GetDoc()),
                            mrAnchorTxtFrm );
        }
    }

    return bSuccess;
}

void SwObjectFormatterTxtFrm::_InvalidatePrevObjs( SwAnchoredObject& _rAnchoredObj )
{
    // invalidate all previous objects, whose wrapping influence on the object
    // positioning is <NONE_CONCURRENT_POSIITIONED>.
    // Note: list of objects at anchor frame is sorted by this property.
    if ( _rAnchoredObj.GetFrmFmt().GetWrapInfluenceOnObjPos().
                // #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                GetWrapInfluenceOnObjPos( true ) ==
                            // #i35017# - constant name has changed
                            text::WrapInfluenceOnPosition::ONCE_CONCURRENT )
    {
        const SwSortedObjs* pObjs = GetAnchorFrm().GetDrawObjs();
        if ( pObjs )
        {
            // determine start index
            sal_Int32 i = pObjs->ListPosOf( _rAnchoredObj ) - 1;
            for ( ; i >= 0; --i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                if ( pAnchoredObj->GetFrmFmt().GetWrapInfluenceOnObjPos().
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

void SwObjectFormatterTxtFrm::_InvalidateFollowObjs( SwAnchoredObject& _rAnchoredObj,
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
        sal_uInt32 i = pObjs->ListPosOf( _rAnchoredObj ) + 1;
        for ( ; i < pObjs->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
            pAnchoredObj->InvalidateObjPosForConsiderWrapInfluence( true );
        }
    }
}

SwAnchoredObject* SwObjectFormatterTxtFrm::_GetFirstObjWithMovedFwdAnchor(
                                    const sal_Int16 _nWrapInfluenceOnPosition,
                                    sal_uInt32& _noToPageNum,
                                    bool& _boInFollow )
{
    // #i35017# - constant names have changed
    OSL_ENSURE( _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ||
            _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_CONCURRENT,
            "<SwObjectFormatterTxtFrm::_GetFirstObjWithMovedFwdAnchor(..)> - invalid value for parameter <_nWrapInfluenceOnPosition>" );

    SwAnchoredObject* pRetAnchoredObj = 0L;

    sal_uInt32 i = 0L;
    for ( ; i < CountOfCollected(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = GetCollectedObj(i);
        if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() &&
             pAnchoredObj->GetFrmFmt().GetWrapInfluenceOnObjPos().
                    // #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                    GetWrapInfluenceOnObjPos( true ) == _nWrapInfluenceOnPosition )
        {
            // #i26945# - use new method <_CheckMovedFwdCondition(..)>
            // #i43913#
            // #i58182# - consider new method signature
            if ( SwObjectFormatterTxtFrm::CheckMovedFwdCondition( *GetCollectedObj( i ),
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
bool SwObjectFormatterTxtFrm::CheckMovedFwdCondition(
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
        ((_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AT_CHAR) ||
         (_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AT_PARA)))
    {
        SwFrm* pAnchorFrm = _rAnchoredObj.GetAnchorFrmContainingAnchPos();
        OSL_ENSURE( pAnchorFrm->IsTxtFrm(),
                "<SwObjectFormatterTxtFrm::CheckMovedFwdCondition(..) - wrong type of anchor frame>" );
        SwTxtFrm* pAnchorTxtFrm = static_cast<SwTxtFrm*>(pAnchorFrm);
        bool bCheck( false );
        if ( pAnchorTxtFrm->IsFollow() )
        {
            bCheck = true;
        }
        else if( pAnchorTxtFrm->IsInTab() )
        {
            const SwRowFrm* pMasterRow = pAnchorTxtFrm->IsInFollowFlowRow();
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
            SwFrm* pColFrm = pAnchorTxtFrm->FindColFrm();
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
// method <SwObjectFormatterTxtFrm::_FormatAnchorFrmForCheckMoveFwd()>
// #i44049# - format till a certain lower frame, if provided.
static void lcl_FormatCntntOfLayoutFrm( SwLayoutFrm* pLayFrm,
                                 SwFrm* pLastLowerFrm = 0L )
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
            lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pLowerFrm),
                                        pLastLowerFrm );
        else
            pLowerFrm->Calc();

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
void SwObjectFormatterTxtFrm::FormatAnchorFrmAndItsPrevs( SwTxtFrm& _rAnchorTxtFrm )
{
    // #i47014# - no format of section and previous columns
    // for follow text frames.
    if ( !_rAnchorTxtFrm.IsFollow() )
    {
        // if anchor frame is directly inside a section, format this section and
        // its previous frames.
        // Note: It's a very simple format without formatting objects.
        if ( _rAnchorTxtFrm.IsInSct() )
        {
            SwFrm* pSectFrm = _rAnchorTxtFrm.GetUpper();
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
                _rAnchorTxtFrm.LockJoin();
                SwFrm* pFrm = pSectFrm->GetUpper()->GetLower();
                // #i49605# - section frame could move forward
                // by the format of its previous frame.
                // Thus, check for valid <pFrm>.
                while ( pFrm && pFrm != pSectFrm )
                {
                    if ( pFrm->IsLayoutFrm() )
                        lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pFrm) );
                    else
                        pFrm->Calc();

                    pFrm = pFrm->GetNext();
                }
                lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pSectFrm),
                                            &_rAnchorTxtFrm );
                // #i44049#
                _rAnchorTxtFrm.UnlockJoin();
            }
        }

        // #i40140# - if anchor frame is inside a column,
        // format the content of the previous columns.
        // Note: It's a very simple format without formatting objects.
        SwFrm* pColFrmOfAnchor = _rAnchorTxtFrm.FindColFrm();
        if ( pColFrmOfAnchor )
        {
            // #i44049#
            _rAnchorTxtFrm.LockJoin();
            SwFrm* pColFrm = pColFrmOfAnchor->GetUpper()->GetLower();
            while ( pColFrm != pColFrmOfAnchor )
            {
                SwFrm* pFrm = pColFrm->GetLower();
                while ( pFrm )
                {
                    if ( pFrm->IsLayoutFrm() )
                        lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pFrm) );
                    else
                        pFrm->Calc();

                    pFrm = pFrm->GetNext();
                }

                pColFrm = pColFrm->GetNext();
            }
            // #i44049#
            _rAnchorTxtFrm.UnlockJoin();
        }
    }

    // format anchor frame - format of its follow not needed
    // #i43255# - forbid follow format, only if anchor text
    // frame is in table
    if ( _rAnchorTxtFrm.IsInTab() )
    {
        SwForbidFollowFormat aForbidFollowFormat( _rAnchorTxtFrm );
        _rAnchorTxtFrm.Calc();
    }
    else
    {
        _rAnchorTxtFrm.Calc();
    }
}

/** method to format the anchor frame for checking of the move forward condition

    #i40141#
*/
void SwObjectFormatterTxtFrm::_FormatAnchorFrmForCheckMoveFwd()
{
    SwObjectFormatterTxtFrm::FormatAnchorFrmAndItsPrevs( mrAnchorTxtFrm );
}

/** method to determine if at least one anchored object has state
    <temporarly consider wrapping style influence> set.
*/
bool SwObjectFormatterTxtFrm::_AtLeastOneObjIsTmpConsiderWrapInfluence()
{
    bool bRet( false );

    const SwSortedObjs* pObjs = GetAnchorFrm().GetDrawObjs();
    if ( pObjs && pObjs->Count() > 1 )
    {
        sal_uInt32 i = 0;
        for ( ; i < pObjs->Count(); ++i )
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
