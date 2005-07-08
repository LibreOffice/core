/*************************************************************************
 *
 *  $RCSfile: objectformattertxtfrm.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 11:04:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _OBJECTFORMATTERTXTFRM_HXX
#include <objectformattertxtfrm.hxx>
#endif
#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif

#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _ROWFRM_HXX
#include <rowfrm.hxx>
#endif

#ifndef _LAYOUTER_HXX
#include <layouter.hxx>
#endif

#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTWRAPINFLUENCEONOBJPOS_HXX
#include <fmtwrapinfluenceonobjpos.hxx>
#endif
// --> OD 2004-11-03 #114798#
#ifndef _FMTFOLLOWTEXTFLOW_HXX
#include <fmtfollowtextflow.hxx>
#endif
// <--

#ifndef _LAYACT_HXX
#include <layact.hxx>
#endif

// --> OD 2004-12-03 #115759# - little helper class to forbid follow formatting
// for the given text frame
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
// <--

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
        while ( pMasterOfAnchorFrm->IsFollow() )
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

// --> OD 2005-01-10 #i40147# - add parameter <_bCheckForMovedFwd>.
bool SwObjectFormatterTxtFrm::DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                           const bool _bCheckForMovedFwd )
{
    // check, if only as-character anchored object have to be formatted, and
    // check the anchor type
    if ( FormatOnlyAsCharAnchored() &&
         !_rAnchoredObj.GetFrmFmt().GetAnchor().GetAnchorId() == FLY_IN_CNTNT )
    {
        return true;
    }

    bool bSuccess( true );

    if ( _rAnchoredObj.IsFormatPossible() )
    {
        _rAnchoredObj.SetRestartLayoutProcess( false );

        _FormatObj( _rAnchoredObj );

        // check, if layout process has to be restarted.
        // if yes, perform needed invalidations.
        // --> OD 2004-11-03 #114798# - no restart of layout process,
        // if anchored object is anchored inside a Writer fly frame,
        // its position is already locked, and it follows the text flow.
        const bool bRestart =
                _rAnchoredObj.RestartLayoutProcess() &&
                !( _rAnchoredObj.PositionLocked() &&
                   _rAnchoredObj.GetAnchorFrm()->IsInFly() &&
                   _rAnchoredObj.GetFrmFmt().GetFollowTextFlow().GetValue() );
        if ( bRestart )
        // <--
        {
            bSuccess = false;
            _InvalidatePrevObjs( _rAnchoredObj );
            _InvalidateFollowObjs( _rAnchoredObj, true );
        }

        // format anchor text frame, if wrapping style influence of the object
        // has to be considered and it's <NONE_SUCCESSIVE_POSITIONED>
        // --> OD 2004-08-25 #i3317# - consider also anchored objects, whose
        // wrapping style influence is temporarly considered.
        // --> OD 2005-01-10 #i40147# - consider also anchored objects, for
        // whose the check of a moved forward anchor frame is requested.
        if ( bSuccess &&
             ( ( _rAnchoredObj.ConsiderObjWrapInfluenceOnObjPos() &&
                 ( _bCheckForMovedFwd ||
                   _rAnchoredObj.GetFrmFmt().GetWrapInfluenceOnObjPos().
                        // --> OD 2004-10-18 #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                        GetWrapInfluenceOnObjPos( true ) ==
                            // --> OD 2004-10-18 #i35017# - constant name has changed
                            text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ) ) ||
               _rAnchoredObj.IsTmpConsiderWrapInfluence() ) )
        // <--
        {
            // --> OD 2004-10-11 #i26945# - check conditions for move forward of
            // anchor text frame
            // determine, if anchor text frame has previous frame
            const bool bDoesAnchorHadPrev = ( mrAnchorTxtFrm.GetIndPrev() != 0 );

            // --> OD 2005-01-11 #i40141# - use new method - it also formats the
            // section the anchor frame is in.
            _FormatAnchorFrmForCheckMoveFwd();
            // <--

            // --> OD 2004-10-22 #i35911#
            if ( _rAnchoredObj.HasClearedEnvironment() )
            {
                _rAnchoredObj.SetClearedEnvironment( true );
                // --> OD 2005-03-08 #i44049# - consider, that anchor frame
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
                        ASSERT( false,
                                "<SwObjectFormatterTxtFrm::DoFormatObj(..)> - anchor frame not marked to move forward" );
                    }
                }
                // <--
            }
            else if ( !mrAnchorTxtFrm.IsFollow() && bDoesAnchorHadPrev )
            // <--
            {
                // index of anchored object in collection of page numbers and
                // anchor types
                sal_uInt32 nIdx( CountOfCollected() );
                ASSERT( nIdx > 0,
                        "<SwObjectFormatterTxtFrm::DoFormatObj(..)> - anchored object not collected!?" );
                --nIdx;

                sal_uInt32 nToPageNum( 0L );
                // --> OD 2005-03-30 #i43913#
                bool bDummy( false );
                if ( _CheckMovedFwdCondition( nIdx, nToPageNum, bDummy ) )
                // <--
                {
                    // --> OD 2005-06-01 #i49987# - consider, that anchor frame
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
                        ASSERT( false,
                                "<SwObjectFormatterTxtFrm::DoFormatObj(..)> - anchor frame not marked to move forward" );
                    }
                    // <--
                }
            }
            // <--
            // --> OD 2005-01-12 #i40155# - mark anchor frame not to wrap around
            // objects under the condition, that its follow contains all its text.
            else if ( !mrAnchorTxtFrm.IsFollow() &&
                      mrAnchorTxtFrm.GetFollow() &&
                      mrAnchorTxtFrm.GetFollow()->GetOfst() == 0 )
            {
                SwLayouter::InsertFrmNotToWrap(
                                *(mrAnchorTxtFrm.FindPageFrm()->GetFmt()->GetDoc()),
                                mrAnchorTxtFrm );
                SwLayouter::RemoveMovedFwdFrm(
                                *(mrAnchorTxtFrm.FindPageFrm()->GetFmt()->GetDoc()),
                                mrAnchorTxtFrm );
            }
            // <--
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
            ASSERT( false,
                    "<SwObjectFormatterTxtFrm::DoFormatObjs()> called for invalidate anchor text frame." );
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
        ASSERT( mpMasterAnchorTxtFrm,
                "SwObjectFormatterTxtFrm::DoFormatObjs() - missing 'master' anchor text frame" );
        bSuccess = _FormatObjsAtFrm( mpMasterAnchorTxtFrm );

        if ( bSuccess )
        {
            // format of as-character anchored floating screen objects - no failure
            // excepted on the format of these objects.
            _FormatObjsAtFrm();
        }
    }
    else
    {
        bSuccess = _FormatObjsAtFrm();
    }

    if ( bSuccess && ConsiderWrapOnObjPos() )
    {
        const bool bDoesAnchorHadPrev = ( mrAnchorTxtFrm.GetIndPrev() != 0 );

        // Format anchor text frame after its objects are formatted.
        // Note: The format of the anchor frame also formats the invalid
        //       previous frames of the anchor frame. The format of the previous
        //       frames is needed to get a correct result of format of the
        //       anchor frame for the following check for moved forward anchors
        // --> OD 2005-01-11 #i40141# - use new method - it also formats the
        // section the anchor frame is in.
        _FormatAnchorFrmForCheckMoveFwd();
        // <--

        sal_uInt32 nToPageNum( 0L );
        // --> OD 2005-03-30 #i43913#
        bool bInFollow( false );
        // <--
        SwAnchoredObject* pObj = 0L;
        if ( !mrAnchorTxtFrm.IsFollow() )
        {
            pObj = _GetFirstObjWithMovedFwdAnchor(
                    // --> OD 2004-10-18 #i35017# - constant name has changed
                    text::WrapInfluenceOnPosition::ONCE_CONCURRENT,
                    // <--
                    nToPageNum, bInFollow );
        }
        // --> OD 2004-10-25 #i35911#
        if ( pObj && pObj->HasClearedEnvironment() )
        {
            pObj->SetClearedEnvironment( true );
            // --> OD 2005-03-08 #i44049# - consider, that anchor frame
            // could already been marked to move forward.
            SwPageFrm* pAnchorPageFrm( mrAnchorTxtFrm.FindPageFrm() );
            // --> OD 2005-03-30 #i43913# - consider, that anchor frame
            // is a follow or is in a follow row, which will move forward.
            if ( pAnchorPageFrm != pObj->GetPageFrm() ||
                 bInFollow )
            // <--
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
                    _InvalidatePrevObjs( *pObj );
                    _InvalidateFollowObjs( *pObj, true );
                }
                else
                {
                    ASSERT( false,
                            "<SwObjectFormatterTxtFrm::DoFormatObjs(..)> - anchor frame not marked to move forward" );
                }
            }
        }
        else if ( pObj && bDoesAnchorHadPrev )
        // <--
        {
            // Object found, whose anchor is moved forward

            // --> OD 2005-06-01 #i49987# - consider, that anchor frame
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
                ASSERT( false,
                        "<SwObjectFormatterTxtFrm::DoFormatObjs(..)> - anchor frame not marked to move forward" );
            }
            // <--
        }
        // <--
        // --> OD 2005-01-12 #i40155# - mark anchor frame not to wrap around
        // objects under the condition, that its follow contains all its text.
        else if ( !mrAnchorTxtFrm.IsFollow() &&
                  mrAnchorTxtFrm.GetFollow() &&
                  mrAnchorTxtFrm.GetFollow()->GetOfst() == 0 )
        {
            SwLayouter::InsertFrmNotToWrap(
                            *(mrAnchorTxtFrm.FindPageFrm()->GetFmt()->GetDoc()),
                            mrAnchorTxtFrm );
            SwLayouter::RemoveMovedFwdFrm(
                            *(mrAnchorTxtFrm.FindPageFrm()->GetFmt()->GetDoc()),
                            mrAnchorTxtFrm );
        }
        // <--
    }

    return bSuccess;
}

void SwObjectFormatterTxtFrm::_InvalidatePrevObjs( SwAnchoredObject& _rAnchoredObj )
{
    // invalidate all previous objects, whose wrapping influence on the object
    // positioning is <NONE_CONCURRENT_POSIITIONED>.
    // Note: list of objects at anchor frame is sorted by this property.
    if ( _rAnchoredObj.GetFrmFmt().GetWrapInfluenceOnObjPos().
                // --> OD 2004-10-18 #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                GetWrapInfluenceOnObjPos( true ) ==
                // <--
                            // --> OD 2004-10-18 #i35017# - constant name has changed
                            text::WrapInfluenceOnPosition::ONCE_CONCURRENT )
                            // <--
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
                        // --> OD 2004-10-18 #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                        GetWrapInfluenceOnObjPos( true ) ==
                        // <--
                            // --> OD 2004-10-18 #i35017# - constant name has changed
                            text::WrapInfluenceOnPosition::ONCE_CONCURRENT )
                            // <--
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
    // --> OD 2004-10-18 #i35017# - constant names have changed
    ASSERT( _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_SUCCESSIVE ||
            _nWrapInfluenceOnPosition == text::WrapInfluenceOnPosition::ONCE_CONCURRENT,
            "<SwObjectFormatterTxtFrm::_GetFirstObjWithMovedFwdAnchor(..)> - invalid value for parameter <_nWrapInfluenceOnPosition>" );
    // <--

    SwAnchoredObject* pRetAnchoredObj = 0L;

    sal_uInt32 i = 0L;
    for ( ; i < CountOfCollected(); ++i )
    {
        SwAnchoredObject* pAnchoredObj = GetCollectedObj(i);
        if ( pAnchoredObj->ConsiderObjWrapInfluenceOnObjPos() &&
             pAnchoredObj->GetFrmFmt().GetWrapInfluenceOnObjPos().
                    // --> OD 2004-10-18 #i35017# - handle ITERATIVE as ONCE_SUCCESSIVE
                    GetWrapInfluenceOnObjPos( true ) == _nWrapInfluenceOnPosition )
                    // <--
        {
            // --> OD 2004-10-11 #i26945# - use new method <_CheckMovedFwdCondition(..)>
            // --> OD 2005-03-30 #i43913#
            if ( _CheckMovedFwdCondition( i, _noToPageNum, _boInFollow ) )
            {
                pRetAnchoredObj = pAnchoredObj;
                break;
            }
            // <--
        }
    }

    return pRetAnchoredObj;
}

bool SwObjectFormatterTxtFrm::_CheckMovedFwdCondition(
                                            const sal_uInt32 _nIdxOfCollected,
                                            sal_uInt32& _noToPageNum,
                                            bool& _boInFollow )
{
    bool bAnchorIsMovedForward( false );

    const sal_uInt32 nFromPageNum = GetPgNumOfCollected( _nIdxOfCollected );
    SwAnchoredObject* pAnchoredObj = GetCollectedObj( _nIdxOfCollected );
    SwPageFrm* pPageFrmOfAnchor = pAnchoredObj->FindPageFrmOfAnchor();
    if ( pPageFrmOfAnchor )
    {
        const sal_uInt32 nPageNum = pPageFrmOfAnchor->GetPhyPageNum();
        if ( nPageNum > nFromPageNum )
        {
            _noToPageNum = nPageNum;
            bAnchorIsMovedForward = true;
        }
    }
    // <--
    // --> OD 2004-11-05 #i26945# - check, if an at-paragraph|at-character
    // anchored object is now anchored at a follow text frame, which will be
    // on the next page. Also check, if an at-character anchored object
    // is now anchored at a text frame,  which is in a follow flow row,
    // which will be on the next page.
    if ( !bAnchorIsMovedForward &&
         ( pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AUTO_CNTNT ||
           pAnchoredObj->GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AT_CNTNT ) &&
         IsCollectedAnchoredAtMaster( _nIdxOfCollected ) )
    {
        SwFrm* pAnchorFrm = pAnchoredObj->GetAnchorFrmContainingAnchPos();
        ASSERT( pAnchorFrm->IsTxtFrm(),
                "<SwObjectFormatterTxtFrm::_CheckMovedFwdCondition(..) - wrong type of anchor frame>" );
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
                _noToPageNum = nFromPageNum + 1;
                bAnchorIsMovedForward = true;
                // --> OD 2005-03-30 #i43913#
                _boInFollow = true;
                // <--
            }
        }
    }
    // <--

    return bAnchorIsMovedForward;
}

// --> OD 2005-01-12 #i40140# - helper method to format layout frames used by
// method <SwObjectFormatterTxtFrm::_FormatAnchorFrmForCheckMoveFwd()>
// --> OD 2005-03-04 #i44049# - format till a certain lower frame, if provided.
void lcl_FormatCntntOfLayoutFrm( SwLayoutFrm* pLayFrm,
                                 SwFrm* pLastLowerFrm = 0L )
{
    SwFrm* pLowerFrm = pLayFrm->GetLower();
    while ( pLowerFrm )
    {
        // --> OD 2005-03-04 #i44049#
        if ( pLastLowerFrm && pLowerFrm == pLastLowerFrm )
        {
            break;
        }
        // <--
        if ( pLowerFrm->IsLayoutFrm() )
            lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pLowerFrm),
                                        pLastLowerFrm );
        else
            pLowerFrm->Calc();

        pLowerFrm = pLowerFrm->GetNext();
    }
}
// <--
/** method to format the anchor frame for checking of the move forward condition

    OD 2005-01-11 #i40141#

    @author OD
*/
void SwObjectFormatterTxtFrm::_FormatAnchorFrmForCheckMoveFwd()
{
    // --> OD 2005-04-13 #i47014# - no format of section and previous columns
    // for follow text frames.
    if ( !mrAnchorTxtFrm.IsFollow() )
    {
        // if anchor frame is directly inside a section, format this section and
        // its previous frames.
        // Note: It's a very simple format without formatting objects.
        if ( mrAnchorTxtFrm.IsInSct() )
        {
            SwFrm* pSectFrm = mrAnchorTxtFrm.GetUpper();
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
                // --> OD 2005-03-04 #i44049#
                mrAnchorTxtFrm.LockJoin();
                // <--
                SwFrm* pFrm = pSectFrm->GetUpper()->GetLower();
                // --> OD 2005-05-23 #i49605# - section frame could move forward
                // by the format of its previous frame.
                // Thus, check for valid <pFrm>.
                while ( pFrm && pFrm != pSectFrm )
                // <--
                {
                    if ( pFrm->IsLayoutFrm() )
                        lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pFrm) );
                    else
                        pFrm->Calc();

                    pFrm = pFrm->GetNext();
                }
                lcl_FormatCntntOfLayoutFrm( static_cast<SwLayoutFrm*>(pSectFrm),
                                            &mrAnchorTxtFrm );
                // --> OD 2005-03-04 #i44049#
                mrAnchorTxtFrm.UnlockJoin();
                // <--
            }
        }

        // --> OD 2005-01-12 #i40140# - if anchor frame is inside a column,
        // format the content of the previous columns.
        // Note: It's a very simple format without formatting objects.
        SwFrm* pColFrmOfAnchor = mrAnchorTxtFrm.FindColFrm();
        if ( pColFrmOfAnchor )
        {
            // --> OD 2005-03-04 #i44049#
            mrAnchorTxtFrm.LockJoin();
            // <--
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
            // --> OD 2005-03-04 #i44049#
            mrAnchorTxtFrm.UnlockJoin();
            // <--
        }
        // <--
    }
    // <--

    // format anchor frame - format of its follow not needed
    // --> OD 2005-04-08 #i43255# - forbid follow format, only if anchor text
    // frame is in table
    if ( mrAnchorTxtFrm.IsInTab() )
    {
        SwForbidFollowFormat aForbidFollowFormat( mrAnchorTxtFrm );
        mrAnchorTxtFrm.Calc();
    }
    else
    {
        mrAnchorTxtFrm.Calc();
    }
}
