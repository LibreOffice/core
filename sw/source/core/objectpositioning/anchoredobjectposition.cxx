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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <anchoredobjectposition.hxx>
#ifndef _ENVIRONMENTOFANCHOREDOBJECT
#include <environmentofanchoredobject.hxx>
#endif
#include <flyfrm.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <frmtool.hxx>
#ifndef _SVX_SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <frmfmt.hxx>
#include <fmtornt.hxx>
// --> OD 2006-03-15 #i62875#
#include <fmtfollowtextflow.hxx>
// <--
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <ndtxt.hxx>
#include <IDocumentSettingAccess.hxx>

using namespace ::com::sun::star;
using namespace objectpositioning;

// **************************************************************************
// constructor, destructor, initialization
// **************************************************************************
SwAnchoredObjectPosition::SwAnchoredObjectPosition( SdrObject& _rDrawObj )
    : mrDrawObj( _rDrawObj ),
      mbIsObjFly( false ),
      mpAnchoredObj( 0 ),
      mpAnchorFrm( 0 ),
      mpContact( 0 ),
      // --> OD 2006-03-15 #i62875#
      mbFollowTextFlow( false ),
      mbDoNotCaptureAnchoredObj( false )
      // <--
{
#if OSL_DEBUG_LEVEL > 1
    // assert, if object isn't of excepted type
    const bool bObjOfExceptedType =
            mrDrawObj.ISA(SwVirtFlyDrawObj) || // object representing fly frame
            mrDrawObj.ISA(SwDrawVirtObj)    || // 'virtual' drawing object
            ( !mrDrawObj.ISA(SdrVirtObj) &&    // 'master' drawing object
              !mrDrawObj.ISA(SwFlyDrawObj) );  // - indirectly checked
    (void) bObjOfExceptedType;
    ASSERT( bObjOfExceptedType,
            "SwAnchoredObjectPosition(..) - object of unexcepted type!" );
#endif

    _GetInfoAboutObj();
}

/** determine information about object

    OD 30.07.2003 #110978#
    members <mbIsObjFly>, <mpFrmOfObj>, <mpAnchorFrm>, <mpContact>,
    <mbFollowTextFlow> and <mbDoNotCaptureAnchoredObj> are set

    @author OD
*/
void SwAnchoredObjectPosition::_GetInfoAboutObj()
{
    // determine, if object represents a fly frame
    {
        mbIsObjFly = mrDrawObj.ISA(SwVirtFlyDrawObj);
    }

    // determine contact object
    {
        mpContact = static_cast<SwContact*>(GetUserCall( &mrDrawObj ));
        ASSERT( mpContact,
                "SwAnchoredObjectPosition::_GetInfoAboutObj() - missing SwContact-object." );
    }

    // determine anchored object, the object belongs to
    {
        // OD 2004-03-30 #i26791#
        mpAnchoredObj = mpContact->GetAnchoredObj( &mrDrawObj );
        ASSERT( mpAnchoredObj,
                "SwAnchoredObjectPosition::_GetInfoAboutObj() - missing anchored object." );
    }

    // determine frame, the object is anchored at
    {
        // OD 2004-03-23 #i26791#
        mpAnchorFrm = mpAnchoredObj->AnchorFrm();
        ASSERT( mpAnchorFrm,
                "SwAnchoredObjectPosition::_GetInfoAboutObj() - missing anchor frame." );
    }

    // determine format the object belongs to
    {
        // --> OD 2004-07-01 #i28701#
        mpFrmFmt = &mpAnchoredObj->GetFrmFmt();
        ASSERT( mpFrmFmt,
                "<SwAnchoredObjectPosition::_GetInfoAboutObj() - missing frame format." );
    }

    // --> OD 2006-03-15 #i62875#
    // determine attribute value of <Follow-Text-Flow>
    {
        mbFollowTextFlow = mpFrmFmt->GetFollowTextFlow().GetValue();
    }

    // determine, if anchored object has not to be captured on the page.
    // the following conditions must be hold to *not* capture it:
    // - corresponding document compatibility flag is set
    // - it's a drawing object
    // - it doesn't follow the text flow
    {
        mbDoNotCaptureAnchoredObj = !mbIsObjFly && !mbFollowTextFlow &&
                                    mpFrmFmt->getIDocumentSettingAccess()->get(IDocumentSettingAccess::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE);
    }
    // <--
}

SwAnchoredObjectPosition::~SwAnchoredObjectPosition()
{}

bool SwAnchoredObjectPosition::IsAnchoredToChar() const
{
    return false;
}

const SwFrm* SwAnchoredObjectPosition::ToCharOrientFrm() const
{
    return NULL;
}

const SwRect* SwAnchoredObjectPosition::ToCharRect() const
{
    return NULL;
}

// OD 12.11.2003 #i22341#
SwTwips SwAnchoredObjectPosition::ToCharTopOfLine() const
{
    return 0L;
}

/** helper method to determine top of a frame for the vertical
    object positioning

    OD 2004-03-11 #i11860#

    @author OD
*/
SwTwips SwAnchoredObjectPosition::_GetTopForObjPos( const SwFrm& _rFrm,
                                                    const SwRectFn& _fnRect,
                                                    const bool _bVert ) const
{
    SwTwips nTopOfFrmForObjPos = (_rFrm.Frm().*_fnRect->fnGetTop)();

    if ( _rFrm.IsTxtFrm() )
    {
        const SwTxtFrm& rTxtFrm = static_cast<const SwTxtFrm&>(_rFrm);
        if ( _bVert )
        {
            nTopOfFrmForObjPos -=
                rTxtFrm.GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
        }
        else
        {
            nTopOfFrmForObjPos +=
                rTxtFrm.GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
        }
    }

    return nTopOfFrmForObjPos;
}

void SwAnchoredObjectPosition::_GetVertAlignmentValues(
                                        const SwFrm& _rVertOrientFrm,
                                        const SwFrm& _rPageAlignLayFrm,
                                        const sal_Int16 _eRelOrient,
                                        SwTwips&      _orAlignAreaHeight,
                                        SwTwips&      _orAlignAreaOffset ) const
{
    SwTwips nHeight = 0;
    SwTwips nOffset = 0;
    SWRECTFN( (&_rVertOrientFrm) )
    // OD 2004-03-11 #i11860# - top of <_rVertOrientFrm> for object positioning
    const SwTwips nVertOrientTop = _GetTopForObjPos( _rVertOrientFrm, fnRect, bVert );
    // OD 2004-03-11 #i11860# - upper space amount of <_rVertOrientFrm> considered
    // for previous frame
    const SwTwips nVertOrientUpperSpaceForPrevFrmAndPageGrid =
            _rVertOrientFrm.IsTxtFrm()
            ? static_cast<const SwTxtFrm&>(_rVertOrientFrm).
                        GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid()
            : 0;
    switch ( _eRelOrient )
    {
        case text::RelOrientation::FRAME:
        {
            // OD 2004-03-11 #i11860# - consider upper space of previous frame
            nHeight = (_rVertOrientFrm.Frm().*fnRect->fnGetHeight)() -
                      nVertOrientUpperSpaceForPrevFrmAndPageGrid;
            nOffset = 0;
        }
        break;
        case text::RelOrientation::PRINT_AREA:
        {
            nHeight = (_rVertOrientFrm.Prt().*fnRect->fnGetHeight)();
            // OD 2004-03-11 #i11860# - consider upper space of previous frame
            nOffset = (_rVertOrientFrm.*fnRect->fnGetTopMargin)() -
                      nVertOrientUpperSpaceForPrevFrmAndPageGrid;
            // if aligned to page in horizontal layout, consider header and
            // footer frame height appropriately.
            if( _rVertOrientFrm.IsPageFrm() && !bVert )
            {
                const SwFrm* pPrtFrm =
                        static_cast<const SwPageFrm&>(_rVertOrientFrm).Lower();
                while( pPrtFrm )
                {
                    if( pPrtFrm->IsHeaderFrm() )
                    {
                        nHeight -= pPrtFrm->Frm().Height();
                        nOffset += pPrtFrm->Frm().Height();
                    }
                    else if( pPrtFrm->IsFooterFrm() )
                    {
                        nHeight -= pPrtFrm->Frm().Height();
                    }
                    pPrtFrm = pPrtFrm->GetNext();
                }
            }
        }
        break;
        case text::RelOrientation::PAGE_FRAME:
        {
            nHeight = (_rPageAlignLayFrm.Frm().*fnRect->fnGetHeight)();
            nOffset = (*fnRect->fnYDiff)(
                        (_rPageAlignLayFrm.Frm().*fnRect->fnGetTop)(),
                        nVertOrientTop );
        }
        break;
        case text::RelOrientation::PAGE_PRINT_AREA:
        {
            nHeight = (_rPageAlignLayFrm.Prt().*fnRect->fnGetHeight)();
            nOffset = (_rPageAlignLayFrm.*fnRect->fnGetTopMargin)() +
                      (*fnRect->fnYDiff)(
                        (_rPageAlignLayFrm.Frm().*fnRect->fnGetTop)(),
                        nVertOrientTop );
            // if aligned to page in horizontal layout, consider header and
            // footer frame height appropriately.
            if( _rPageAlignLayFrm.IsPageFrm() && !bVert )
            {
                const SwFrm* pPrtFrm =
                        static_cast<const SwPageFrm&>(_rPageAlignLayFrm).Lower();
                while( pPrtFrm )
                {
                    if( pPrtFrm->IsHeaderFrm() )
                    {
                        nHeight -= pPrtFrm->Frm().Height();
                        nOffset += pPrtFrm->Frm().Height();
                    }
                    else if( pPrtFrm->IsFooterFrm() )
                    {
                        nHeight -= pPrtFrm->Frm().Height();
                    }
                    pPrtFrm = pPrtFrm->GetNext();
                }
            }
        }
        break;
        // OD 12.11.2003 #i22341# - vertical alignment at top of line
        case text::RelOrientation::TEXT_LINE:
        {
            if ( IsAnchoredToChar() )
            {
                nHeight = 0;
                nOffset = (*fnRect->fnYDiff)( ToCharTopOfLine(), nVertOrientTop );
            }
            else
            {
                ASSERT( false,
                        "<SwAnchoredObjectPosition::_GetVertAlignmentValues(..)> - invalid relative alignment" );
            }
        }
        break;
        case text::RelOrientation::CHAR:
        {
            if ( IsAnchoredToChar() )
            {
                nHeight = (ToCharRect()->*fnRect->fnGetHeight)();
                nOffset = (*fnRect->fnYDiff)( (ToCharRect()->*fnRect->fnGetTop)(),
                                              nVertOrientTop );
            }
            else
            {
                ASSERT( false,
                        "<SwAnchoredObjectPosition::_GetVertAlignmentValues(..)> - invalid relative alignment" );
            }
        }
        break;
        // no break here, because text::RelOrientation::CHAR is invalid, if !mbAnchorToChar
        default:
        //case text::RelOrientation::PAGE_LEFT:     not valid for vertical alignment
        //case text::RelOrientation::PAGE_RIGHT:    not valid for vertical alignment
        //case text::RelOrientation::FRAME_LEFT:    not valid for vertical alignment
        //case text::RelOrientation::FRAME_RIGHT:   not valid for vertical alignment
        {
            ASSERT( false,
                    "<SwAnchoredObjectPosition::_GetVertAlignmentValues(..)> - invalid relative alignment" );
        }
    }

    _orAlignAreaHeight = nHeight;
    _orAlignAreaOffset = nOffset;
}

// --> OD 2004-06-17 #i26791# - add output parameter <_roVertOffsetToFrmAnchorPos>
SwTwips SwAnchoredObjectPosition::_GetVertRelPos(
                                    const SwFrm& _rVertOrientFrm,
                                    const SwFrm& _rPageAlignLayFrm,
                                    const sal_Int16 _eVertOrient,
                                    const sal_Int16 _eRelOrient,
                                    const SwTwips          _nVertPos,
                                    const SvxLRSpaceItem& _rLRSpacing,
                                    const SvxULSpaceItem& _rULSpacing,
                                    SwTwips& _roVertOffsetToFrmAnchorPos ) const
{
    SwTwips nRelPosY = 0;
    SWRECTFN( (&_rVertOrientFrm) );

    SwTwips nAlignAreaHeight;
    SwTwips nAlignAreaOffset;
    _GetVertAlignmentValues( _rVertOrientFrm, _rPageAlignLayFrm,
                             _eRelOrient, nAlignAreaHeight, nAlignAreaOffset );

    nRelPosY = nAlignAreaOffset;
    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );
    const SwTwips nObjHeight = (aObjBoundRect.*fnRect->fnGetHeight)();

    switch ( _eVertOrient )
    {
        case text::VertOrientation::NONE:
        {
            // 'manual' vertical position
            nRelPosY += _nVertPos;
        }
        break;
        case text::VertOrientation::TOP:
        {
            nRelPosY += bVert ? _rLRSpacing.GetRight() : _rULSpacing.GetUpper();
        }
        break;
        case text::VertOrientation::CENTER:
        {
            nRelPosY += (nAlignAreaHeight / 2) - (nObjHeight / 2);
        }
        break;
        case text::VertOrientation::BOTTOM:
        {
            nRelPosY += nAlignAreaHeight -
                       ( nObjHeight + ( bVert ? _rLRSpacing.GetLeft() : _rULSpacing.GetLower() ) );
        }
        break;
        default:
        {
            ASSERT( false,
                    "<SwAnchoredObjectPosition::_GetVertRelPos(..) - invalid vertical positioning" );
        }
    }

    // --> OD 2004-06-17 #i26791#
    _roVertOffsetToFrmAnchorPos = nAlignAreaOffset;

    return nRelPosY;
}

/** adjust calculated vertical in order to keep object inside
    'page' alignment layout frame.

    OD 2004-07-01 #i28701# - parameter <_nTopOfAnch> and <_bVert> added
    OD 2004-07-22 #i31805# - add parameter <_bCheckBottom>
    OD 2004-10-08 #i26945# - add parameter <_bFollowTextFlow>
    OD 2006-03-15 #i62875# - method now private and renamed.

    @author OD
*/
SwTwips SwAnchoredObjectPosition::_ImplAdjustVertRelPos( const SwTwips _nTopOfAnch,
                                                         const bool _bVert,
                                                         const SwFrm&  _rPageAlignLayFrm,
                                                         const SwTwips _nProposedRelPosY,
                                                         const bool _bFollowTextFlow,
                                                         const bool _bCheckBottom ) const
{
    SwTwips nAdjustedRelPosY = _nProposedRelPosY;

    const Size aObjSize( GetAnchoredObj().GetObjRect().SSize() );

    // determine the area of 'page' alignment frame, to which the vertical
    // position is restricted.
    // --> OD 2004-07-06 #i28701# - Extend restricted area for the vertical
    // position to area of the page frame, if wrapping style influence is
    // considered on object positioning. Needed to avoid layout loops in the
    // object positioning algorithm considering the wrapping style influence
    // caused by objects, which follow the text flow and thus are restricted
    // to its environment (e.g. page header/footer).
    SwRect aPgAlignArea;
    {
        // --> OD 2004-10-08 #i26945# - no extension of restricted area, if
        // object's attribute follow text flow is set and its inside a table
        if ( GetFrmFmt().getIDocumentSettingAccess()->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) &&
             ( !_bFollowTextFlow ||
               !GetAnchoredObj().GetAnchorFrm()->IsInTab() ) )
        {
            aPgAlignArea = _rPageAlignLayFrm.FindPageFrm()->Frm();
        }
        else
        {
            aPgAlignArea = _rPageAlignLayFrm.Frm();
        }
    }

    if ( _bVert )
    {
        // OD 2004-07-22 #i31805# - consider value of <_bCheckBottom>
        if ( _bCheckBottom &&
             _nTopOfAnch - nAdjustedRelPosY - aObjSize.Width() <
                aPgAlignArea.Left() )
        {
            nAdjustedRelPosY = aPgAlignArea.Left() +
                               _nTopOfAnch -
                               aObjSize.Width();
        }
        // --> OD 2004-08-13 #i32964# - correction
        if ( _nTopOfAnch - nAdjustedRelPosY > aPgAlignArea.Right() )
        {
            nAdjustedRelPosY = _nTopOfAnch - aPgAlignArea.Right();
        }
        // <--
    }
    else
    {
        // OD 2004-07-22 #i31805# - consider value of <_bCheckBottom>
        if ( _bCheckBottom &&
             _nTopOfAnch + nAdjustedRelPosY + aObjSize.Height() >
                // --> OD 2006-01-13 #129959#
                // Do not mix usage of <top + height> and <bottom>
//                aPgAlignArea.Bottom() )
                aPgAlignArea.Top() + aPgAlignArea.Height() )
                // <--
        {
            // --> OD 2006-01-13 #129959#
            // Do not mix usage of <top + height> and <bottom>
//            nAdjustedRelPosY = aPgAlignArea.Bottom() -
            nAdjustedRelPosY = aPgAlignArea.Top() + aPgAlignArea.Height() -
            // <--
                               _nTopOfAnch -
                               aObjSize.Height();
        }
        if ( _nTopOfAnch + nAdjustedRelPosY <  aPgAlignArea.Top() )
        {
            nAdjustedRelPosY = aPgAlignArea.Top() - _nTopOfAnch;
        }
    }

    return nAdjustedRelPosY;
}

/** adjust calculated horizontal in order to keep object inside
    'page' alignment layout frame.

    OD 2006-03-15 #i62875# - method now private and renamed.

    @author OD
*/
SwTwips SwAnchoredObjectPosition::_ImplAdjustHoriRelPos(
                                        const SwFrm&  _rPageAlignLayFrm,
                                        const SwTwips _nProposedRelPosX ) const
{
    SwTwips nAdjustedRelPosX = _nProposedRelPosX;

    const SwFrm& rAnchorFrm = GetAnchorFrm();
    const bool bVert = rAnchorFrm.IsVertical();

    const Size aObjSize( GetAnchoredObj().GetObjRect().SSize() );

    if( bVert )
    {
        if ( rAnchorFrm.Frm().Top() + nAdjustedRelPosX + aObjSize.Height() >
                _rPageAlignLayFrm.Frm().Bottom() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrm.Frm().Bottom() -
                               rAnchorFrm.Frm().Top() -
                               aObjSize.Height();
        }
        if ( rAnchorFrm.Frm().Top() + nAdjustedRelPosX <
                _rPageAlignLayFrm.Frm().Top() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrm.Frm().Top() -
                               rAnchorFrm.Frm().Top();
        }
    }
    else
    {
        if ( rAnchorFrm.Frm().Left() + nAdjustedRelPosX + aObjSize.Width() >
                _rPageAlignLayFrm.Frm().Right() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrm.Frm().Right() -
                               rAnchorFrm.Frm().Left() -
                               aObjSize.Width();
        }
        if ( rAnchorFrm.Frm().Left() + nAdjustedRelPosX <
                _rPageAlignLayFrm.Frm().Left() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrm.Frm().Left() -
                               rAnchorFrm.Frm().Left();
        }
    }

    return nAdjustedRelPosX;
}

/** determine alignment value for horizontal position of object

    @author OD
*/
void SwAnchoredObjectPosition::_GetHoriAlignmentValues( const SwFrm&  _rHoriOrientFrm,
                                                        const SwFrm&  _rPageAlignLayFrm,
                                                        const sal_Int16 _eRelOrient,
                                                        const bool    _bObjWrapThrough,
                                                        SwTwips&      _orAlignAreaWidth,
                                                        SwTwips&      _orAlignAreaOffset,
                                                        bool&         _obAlignedRelToPage ) const
{
    SwTwips nWidth = 0;
    SwTwips nOffset = 0;
    SWRECTFN( (&_rHoriOrientFrm) )
    switch ( _eRelOrient )
    {
        case text::RelOrientation::PRINT_AREA:
        {
            nWidth = (_rHoriOrientFrm.Prt().*fnRect->fnGetWidth)();
            nOffset = (_rHoriOrientFrm.*fnRect->fnGetLeftMargin)();
            if ( _rHoriOrientFrm.IsTxtFrm() )
            {
                // consider movement of text frame left
                nOffset += static_cast<const SwTxtFrm&>(_rHoriOrientFrm).GetBaseOfstForFly( !_bObjWrapThrough );
            }
            else if ( _rHoriOrientFrm.IsPageFrm() && bVert )
            {
                // for to-page anchored objects, consider header/footer frame
                // in vertical layout
                const SwFrm* pPrtFrm =
                        static_cast<const SwPageFrm&>(_rHoriOrientFrm).Lower();
                while( pPrtFrm )
                {
                    if( pPrtFrm->IsHeaderFrm() )
                    {
                        nWidth -= pPrtFrm->Frm().Height();
                        nOffset += pPrtFrm->Frm().Height();
                    }
                    else if( pPrtFrm->IsFooterFrm() )
                    {
                        nWidth -= pPrtFrm->Frm().Height();
                    }
                    pPrtFrm = pPrtFrm->GetNext();
                }
            }
            break;
        }
        case text::RelOrientation::PAGE_LEFT:
        {
            // align at left border of page frame/fly frame/cell frame
            nWidth = (_rPageAlignLayFrm.*fnRect->fnGetLeftMargin)();
            nOffset = (*fnRect->fnXDiff)(
                      (_rPageAlignLayFrm.Frm().*fnRect->fnGetLeft)(),
                      (_rHoriOrientFrm.Frm().*fnRect->fnGetLeft)() );
            _obAlignedRelToPage = true;
        }
        break;
        case text::RelOrientation::PAGE_RIGHT:
        {
            // align at right border of page frame/fly frame/cell frame
            nWidth = (_rPageAlignLayFrm.*fnRect->fnGetRightMargin)();
            nOffset = (*fnRect->fnXDiff)(
                      (_rPageAlignLayFrm.*fnRect->fnGetPrtRight)(),
                      (_rHoriOrientFrm.Frm().*fnRect->fnGetLeft)() );
            _obAlignedRelToPage = true;
        }
        break;
        case text::RelOrientation::FRAME_LEFT:
        {
            // align at left border of anchor frame
            nWidth = (_rHoriOrientFrm.*fnRect->fnGetLeftMargin)();
            nOffset = 0;
        }
        break;
        case text::RelOrientation::FRAME_RIGHT:
        {
            // align at right border of anchor frame
            // OD 19.08.2003 #110978# - unify and simplify
            nWidth = (_rHoriOrientFrm.*fnRect->fnGetRightMargin)();
            //nOffset = (_rHoriOrientFrm.Frm().*fnRect->fnGetWidth)() -
            //          nWidth;
            nOffset = (_rHoriOrientFrm.Prt().*fnRect->fnGetRight)();
        }
        break;
        case text::RelOrientation::CHAR:
        {
            // alignment relative to character - assure, that corresponding
            // character rectangle is set.
            if ( IsAnchoredToChar() )
            {
                nWidth = 0;
                nOffset = (*fnRect->fnXDiff)(
                            (ToCharRect()->*fnRect->fnGetLeft)(),
                            (ToCharOrientFrm()->Frm().*fnRect->fnGetLeft)() );
                break;
            }
            // no break!
        }
        case text::RelOrientation::PAGE_PRINT_AREA:
        {
            nWidth = (_rPageAlignLayFrm.Prt().*fnRect->fnGetWidth)();
            nOffset = (*fnRect->fnXDiff)(
                        (_rPageAlignLayFrm.*fnRect->fnGetPrtLeft)(),
                        (_rHoriOrientFrm.Frm().*fnRect->fnGetLeft)() );
            if ( _rHoriOrientFrm.IsPageFrm() && bVert )
            {
                // for to-page anchored objects, consider header/footer frame
                // in vertical layout
                const SwFrm* pPrtFrm =
                        static_cast<const SwPageFrm&>(_rHoriOrientFrm).Lower();
                while( pPrtFrm )
                {
                    if( pPrtFrm->IsHeaderFrm() )
                    {
                        nWidth -= pPrtFrm->Frm().Height();
                        nOffset += pPrtFrm->Frm().Height();
                    }
                    else if( pPrtFrm->IsFooterFrm() )
                    {
                        nWidth -= pPrtFrm->Frm().Height();
                    }
                    pPrtFrm = pPrtFrm->GetNext();
                }
            }
            _obAlignedRelToPage = true;
            break;
        }
        case text::RelOrientation::PAGE_FRAME:
        {
            nWidth = (_rPageAlignLayFrm.Frm().*fnRect->fnGetWidth)();
            nOffset = (*fnRect->fnXDiff)(
                        (_rPageAlignLayFrm.Frm().*fnRect->fnGetLeft)(),
                        (_rHoriOrientFrm.Frm().*fnRect->fnGetLeft)() );
            _obAlignedRelToPage = true;
            break;
        }
        default:
        // case text::RelOrientation::FRAME:
        {
            nWidth = (_rHoriOrientFrm.Frm().*fnRect->fnGetWidth)();
            nOffset = _rHoriOrientFrm.IsTxtFrm() ?
                   static_cast<const SwTxtFrm&>(_rHoriOrientFrm).GetBaseOfstForFly( !_bObjWrapThrough ) :
                   0;
            break;
        }
    }

    _orAlignAreaWidth = nWidth;
    _orAlignAreaOffset = nOffset;
}

/** toggle given horizontal orientation and relative alignment

    @author OD
*/
void SwAnchoredObjectPosition::_ToggleHoriOrientAndAlign(
                                        const bool _bToggleLeftRight,
                                        sal_Int16& _ioeHoriOrient,
                                        sal_Int16& _iopeRelOrient
                                      ) const
{
    if( _bToggleLeftRight )
    {
        // toggle orientation
        switch ( _ioeHoriOrient )
        {
            case text::HoriOrientation::RIGHT :
                {
                    _ioeHoriOrient = text::HoriOrientation::LEFT;
                }
                break;
            case text::HoriOrientation::LEFT :
                {
                    _ioeHoriOrient = text::HoriOrientation::RIGHT;
                }
                break;
            default:
                break;
        }

        // toggle relative alignment
        switch ( _iopeRelOrient )
        {
            case text::RelOrientation::PAGE_RIGHT :
                {
                    _iopeRelOrient = text::RelOrientation::PAGE_LEFT;
                }
                break;
            case text::RelOrientation::PAGE_LEFT :
                {
                    _iopeRelOrient = text::RelOrientation::PAGE_RIGHT;
                }
                break;
            case text::RelOrientation::FRAME_RIGHT :
                {
                    _iopeRelOrient = text::RelOrientation::FRAME_LEFT;
                }
                break;
            case text::RelOrientation::FRAME_LEFT :
                {
                    _iopeRelOrient = text::RelOrientation::FRAME_RIGHT;
                }
                break;
            default:
                break;
        }
    }
}

/** calculate relative horizontal position

    @author OD
*/
SwTwips SwAnchoredObjectPosition::_CalcRelPosX(
                                const SwFrm& _rHoriOrientFrm,
                                const SwEnvironmentOfAnchoredObject& _rEnvOfObj,
                                const SwFmtHoriOrient& _rHoriOrient,
                                const SvxLRSpaceItem& _rLRSpacing,
                                const SvxULSpaceItem& _rULSpacing,
                                const bool _bObjWrapThrough,
                                const SwTwips _nRelPosY,
                                SwTwips& _roHoriOffsetToFrmAnchorPos
                              ) const
{
    // determine 'page' alignment layout frame
    const SwFrm& rPageAlignLayFrm =
            _rEnvOfObj.GetHoriEnvironmentLayoutFrm( _rHoriOrientFrm );

    const bool bEvenPage = !rPageAlignLayFrm.OnRightPage();
    const bool bToggle = _rHoriOrient.IsPosToggle() && bEvenPage;

    // determine orientation and relative alignment
    sal_Int16 eHoriOrient = _rHoriOrient.GetHoriOrient();
    sal_Int16 eRelOrient = _rHoriOrient.GetRelationOrient();
    // toggle orientation and relative alignment
    _ToggleHoriOrientAndAlign( bToggle, eHoriOrient, eRelOrient );

    // determine alignment parameter
    // <nWidth>:  'width' of alignment area
    // <nOffset>: offset of alignment area, relative to 'left' of anchor frame
    SwTwips nWidth = 0;
    SwTwips nOffset = 0;
    bool bAlignedRelToPage = false;
    _GetHoriAlignmentValues( _rHoriOrientFrm, rPageAlignLayFrm,
                             eRelOrient, _bObjWrapThrough,
                             nWidth, nOffset, bAlignedRelToPage );

    const SwFrm& rAnchorFrm = GetAnchorFrm();
    SWRECTFN( (&_rHoriOrientFrm) )
    SwTwips nObjWidth = (GetAnchoredObj().GetObjRect().*fnRect->fnGetWidth)();
    SwTwips nRelPosX = nOffset;
    if ( _rHoriOrient.GetHoriOrient() == text::HoriOrientation::NONE )
    {
        // 'manual' horizonal position
        const bool bR2L = rAnchorFrm.IsRightToLeft();
        if( IsAnchoredToChar() && text::RelOrientation::CHAR == eRelOrient )
        {
            if( bR2L )
                nRelPosX -= _rHoriOrient.GetPos();
            else
                nRelPosX += _rHoriOrient.GetPos();
        }
        else if ( bToggle || ( !_rHoriOrient.IsPosToggle() && bR2L ) )
        {
            // OD 04.08.2003 #110978# - correction: consider <nOffset> also for
            // toggling from left to right.
            nRelPosX += nWidth - nObjWidth - _rHoriOrient.GetPos();
        }
        else
        {
            nRelPosX += _rHoriOrient.GetPos();
        }
    }
    else if ( text::HoriOrientation::CENTER == eHoriOrient )
        nRelPosX += (nWidth / 2) - (nObjWidth / 2);
    else if ( text::HoriOrientation::RIGHT == eHoriOrient )
        nRelPosX += nWidth -
                    ( nObjWidth +
                      ( bVert ? _rULSpacing.GetLower() : _rLRSpacing.GetRight() ) );
    else
        nRelPosX += bVert ? _rULSpacing.GetUpper() : _rLRSpacing.GetLeft();

    // adjust relative position by distance between anchor frame and
    // the frame, the object is oriented at.
    if ( &rAnchorFrm != &_rHoriOrientFrm )
    {
        SwTwips nLeftOrient = (_rHoriOrientFrm.Frm().*fnRect->fnGetLeft)();
        SwTwips nLeftAnchor = (rAnchorFrm.Frm().*fnRect->fnGetLeft)();
        nRelPosX += (*fnRect->fnXDiff)( nLeftOrient, nLeftAnchor );
    }

    // OD 2004-05-21 #i28701# - deactivate follow code
//    // adjust relative horizontal position, if object is manual horizontal
//    // positioned (not 'page' aligned) and orients not at the anchor frame,
//    // but it overlaps anchor frame.
//    if ( _rHoriOrient.GetHoriOrient() == text::HoriOrientation::NONE && !bAlignedRelToPage &&
//         &rAnchorFrm != &_rHoriOrientFrm )
//    {
//        // E.g.: consider a columned page/section with an horizontal
//        //       negative positioned object.
//        // OD 2004-03-23 #i26791#
//        const SwRect& rObjRect = GetAnchoredObj().GetObjRect();
//        if( bVert )
//        {
//            if( _rHoriOrientFrm.Frm().Top() > rAnchorFrm.Frm().Bottom() &&
//                rObjRect.Right() > rAnchorFrm.Frm().Left() )
//            {
//                const SwTwips nProposedPosX = nRelPosX + rAnchorFrm.Frm().Top();
//                if ( nProposedPosX < rAnchorFrm.Frm().Bottom() )
//                    nRelPosX = rAnchorFrm.Frm().Height() + 1;
//            }
//        }
//        else
//        {
//            if( _rHoriOrientFrm.Frm().Left() > rAnchorFrm.Frm().Right() &&
//                rObjRect.Top() < rAnchorFrm.Frm().Bottom() )
//            {
//                // OD 04.08.2003 #110978# - correction: use <nRelPosX>
//                // instead of <aRelPos.X()>
//                const SwTwips nProposedPosX = nRelPosX + rAnchorFrm.Frm().Left();
//                if ( nProposedPosX < rAnchorFrm.Frm().Right() )
//                    nRelPosX = rAnchorFrm.Frm().Width() + 1;
//            }
//        }
//    }
    // adjust calculated relative horizontal position, in order to
    // keep object inside 'page' alignment layout frame
    const SwFrm& rEnvironmentLayFrm =
            _rEnvOfObj.GetHoriEnvironmentLayoutFrm( _rHoriOrientFrm );
    nRelPosX = _AdjustHoriRelPos( rEnvironmentLayFrm, nRelPosX );

    // if object is a Writer fly frame and it's anchored to a content and
    // it is horizontal positioned left or right, but not relative to character,
    // it has to be drawn aside another object, which have the same horizontal
    // position and lay below it.
    if ( GetAnchoredObj().ISA(SwFlyFrm) &&
         ( GetContact().ObjAnchoredAtPara() || GetContact().ObjAnchoredAtChar() ) &&
         ( eHoriOrient == text::HoriOrientation::LEFT || eHoriOrient == text::HoriOrientation::RIGHT ) &&
         eRelOrient != text::RelOrientation::CHAR )
    {
        nRelPosX = _AdjustHoriRelPosForDrawAside( _rHoriOrientFrm,
                                                  nRelPosX, _nRelPosY,
                                                  eHoriOrient, eRelOrient,
                                                  _rLRSpacing, _rULSpacing,
                                                  bEvenPage );
    }

    // --> OD 2004-06-17 #i26791#
    _roHoriOffsetToFrmAnchorPos = nOffset;

    return nRelPosX;
}

// **************************************************************************
// method incl. helper methods for adjusting proposed horizontal position,
// if object has to draw aside another object.
// **************************************************************************
/** adjust calculated horizontal position in order to draw object
    aside other objects with same positioning

    @author OD
*/
SwTwips SwAnchoredObjectPosition::_AdjustHoriRelPosForDrawAside(
                                            const SwFrm&  _rHoriOrientFrm,
                                            const SwTwips _nProposedRelPosX,
                                            const SwTwips _nRelPosY,
                                            const sal_Int16 _eHoriOrient,
                                            const sal_Int16 _eRelOrient,
                                            const SvxLRSpaceItem& _rLRSpacing,
                                            const SvxULSpaceItem& _rULSpacing,
                                            const bool _bEvenPage
                                          ) const
{
    // OD 2004-03-23 #i26791#
    if ( !GetAnchorFrm().ISA(SwTxtFrm) ||
         !GetAnchoredObj().ISA(SwFlyAtCntFrm) )
    {
        ASSERT( false,
                "<SwAnchoredObjectPosition::_AdjustHoriRelPosForDrawAside(..) - usage for wrong anchor type" );
        return _nProposedRelPosX;
    }

    const SwTxtFrm& rAnchorTxtFrm = static_cast<const SwTxtFrm&>(GetAnchorFrm());
    // OD 2004-03-23 #i26791#
    const SwFlyAtCntFrm& rFlyAtCntFrm =
                        static_cast<const SwFlyAtCntFrm&>(GetAnchoredObj());
    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );
    SWRECTFN( (&_rHoriOrientFrm) )

    SwTwips nAdjustedRelPosX = _nProposedRelPosX;

    // determine proposed object bound rectangle
    Point aTmpPos = (rAnchorTxtFrm.Frm().*fnRect->fnGetPos)();
    if( bVert )
    {
        aTmpPos.X() -= _nRelPosY + aObjBoundRect.Width();
        aTmpPos.Y() += nAdjustedRelPosX;
    }
    else
    {
        aTmpPos.X() += nAdjustedRelPosX;
        aTmpPos.Y() += _nRelPosY;
    }
    SwRect aTmpObjRect( aTmpPos, aObjBoundRect.SSize() );

    const sal_uInt32 nObjOrdNum = GetObject().GetOrdNum();
    const SwPageFrm* pObjPage = rFlyAtCntFrm.FindPageFrm();
    const SwFrm* pObjContext = ::FindKontext( &rAnchorTxtFrm, FRM_COLUMN );
    sal_uLong nObjIndex = rAnchorTxtFrm.GetTxtNode()->GetIndex();
    SwOrderIter aIter( pObjPage, sal_True );
    const SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)aIter.Bottom())->GetFlyFrm();
    while ( pFly && nObjOrdNum > pFly->GetVirtDrawObj()->GetOrdNumDirect() )
    {
        if ( _DrawAsideFly( pFly, aTmpObjRect, pObjContext, nObjIndex,
                           _bEvenPage, _eHoriOrient, _eRelOrient ) )
        {
            if( bVert )
            {
                const SvxULSpaceItem& rOtherUL = pFly->GetFmt()->GetULSpace();
                const SwTwips nOtherTop = pFly->Frm().Top() - rOtherUL.GetUpper();
                const SwTwips nOtherBot = pFly->Frm().Bottom() + rOtherUL.GetLower();
                if ( nOtherTop <= aTmpObjRect.Bottom() + _rULSpacing.GetLower() &&
                     nOtherBot >= aTmpObjRect.Top() - _rULSpacing.GetUpper() )
                {
                    if ( _eHoriOrient == text::HoriOrientation::LEFT )
                    {
                        SwTwips nTmp = nOtherBot + 1 + _rULSpacing.GetUpper() -
                                       rAnchorTxtFrm.Frm().Top();
                        if ( nTmp > nAdjustedRelPosX &&
                             rAnchorTxtFrm.Frm().Top() + nTmp +
                             aObjBoundRect.Height() + _rULSpacing.GetLower()
                             <= pObjPage->Frm().Height() + pObjPage->Frm().Top() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    else if ( _eHoriOrient == text::HoriOrientation::RIGHT )
                    {
                        SwTwips nTmp = nOtherTop - 1 - _rULSpacing.GetLower() -
                                       aObjBoundRect.Height() -
                                       rAnchorTxtFrm.Frm().Top();
                        if ( nTmp < nAdjustedRelPosX &&
                             rAnchorTxtFrm.Frm().Top() + nTmp - _rULSpacing.GetUpper()
                              >= pObjPage->Frm().Top() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    aTmpObjRect.Pos().Y() = rAnchorTxtFrm.Frm().Top() +
                                            nAdjustedRelPosX;
                }
            }
            else
            {
                const SvxLRSpaceItem& rOtherLR = pFly->GetFmt()->GetLRSpace();
                const SwTwips nOtherLeft = pFly->Frm().Left() - rOtherLR.GetLeft();
                const SwTwips nOtherRight = pFly->Frm().Right() + rOtherLR.GetRight();
                if( nOtherLeft <= aTmpObjRect.Right() + _rLRSpacing.GetRight() &&
                    nOtherRight >= aTmpObjRect.Left() - _rLRSpacing.GetLeft() )
                {
                    if ( _eHoriOrient == text::HoriOrientation::LEFT )
                    {
                        SwTwips nTmp = nOtherRight + 1 + _rLRSpacing.GetLeft() -
                                       rAnchorTxtFrm.Frm().Left();
                        if ( nTmp > nAdjustedRelPosX &&
                             rAnchorTxtFrm.Frm().Left() + nTmp +
                             aObjBoundRect.Width() + _rLRSpacing.GetRight()
                             <= pObjPage->Frm().Width() + pObjPage->Frm().Left() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    else if ( _eHoriOrient == text::HoriOrientation::RIGHT )
                    {
                        SwTwips nTmp = nOtherLeft - 1 - _rLRSpacing.GetRight() -
                                       aObjBoundRect.Width() -
                                       rAnchorTxtFrm.Frm().Left();
                        if ( nTmp < nAdjustedRelPosX &&
                             rAnchorTxtFrm.Frm().Left() + nTmp - _rLRSpacing.GetLeft()
                             >= pObjPage->Frm().Left() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    aTmpObjRect.Pos().X() = rAnchorTxtFrm.Frm().Left() +
                                            nAdjustedRelPosX;
                }
            } // end of <if (bVert)>
        } // end of <if _DrawAsideFly(..)>

        pFly = ((SwVirtFlyDrawObj*)aIter.Next())->GetFlyFrm();
    } // end of <loop on fly frames

    return nAdjustedRelPosX;
}

/** detemine, if object has to draw aside given fly frame

    method used by <_AdjustHoriRelPosForDrawAside(..)>

    @author OD
*/
bool SwAnchoredObjectPosition::_DrawAsideFly( const SwFlyFrm* _pFly,
                                              const SwRect&   _rObjRect,
                                              const SwFrm*    _pObjContext,
                                              const sal_uLong     _nObjIndex,
                                              const bool      _bEvenPage,
                                              const sal_Int16 _eHoriOrient,
                                              const sal_Int16 _eRelOrient
                                            ) const
{
    bool bRetVal = false;

    SWRECTFN( (&GetAnchorFrm()) )

    if ( _pFly->IsFlyAtCntFrm() &&
         (_pFly->Frm().*fnRect->fnBottomDist)( (_rObjRect.*fnRect->fnGetTop)() ) < 0 &&
         (_rObjRect.*fnRect->fnBottomDist)( (_pFly->Frm().*fnRect->fnGetTop)() ) < 0 &&
         ::FindKontext( _pFly->GetAnchorFrm(), FRM_COLUMN ) == _pObjContext )
    {
        sal_uLong nOtherIndex =
            static_cast<const SwTxtFrm*>(_pFly->GetAnchorFrm())->GetTxtNode()->GetIndex();
        if( _nObjIndex >= nOtherIndex )
        {
            const SwFmtHoriOrient& rHori = _pFly->GetFmt()->GetHoriOrient();
            sal_Int16 eOtherRelOrient = rHori.GetRelationOrient();
            if( text::RelOrientation::CHAR != eOtherRelOrient )
            {
                sal_Int16 eOtherHoriOrient = rHori.GetHoriOrient();
                _ToggleHoriOrientAndAlign( _bEvenPage && rHori.IsPosToggle(),
                                           eOtherHoriOrient,
                                           eOtherRelOrient );
                if ( eOtherHoriOrient == _eHoriOrient &&
                    _Minor( _eRelOrient, eOtherRelOrient, text::HoriOrientation::LEFT == _eHoriOrient ) )
                {
                    bRetVal = true;
                }
            }
        }
    }

    return bRetVal;
}

/** determine, if object has to draw aside another object

    the different alignments of the objects determines, if one has
    to draw aside another one. Thus, the given alignment are checked
    against each other, which one has to be drawn aside the other one.
    depending on parameter _bLeft check is done for left or right
    positioning.
    method used by <_DrawAsideFly(..)>

    @author OD
*/
bool SwAnchoredObjectPosition::_Minor( sal_Int16 _eRelOrient1,
                                       sal_Int16 _eRelOrient2,
                                       bool             _bLeft ) const
{
    bool bRetVal;

    // draw aside order for left horizontal position
    //! one array entry for each value in text::RelOrientation
    static sal_uInt16 __READONLY_DATA aLeft[ 10 ] =
        { 5, 6, 0, 1, 8, 4, 7, 2, 3, 9 };
    // draw aside order for right horizontal position
    //! one array entry for each value in text::RelOrientation
    static sal_uInt16 __READONLY_DATA aRight[ 10 ] =
        { 5, 6, 0, 8, 1, 7, 4, 2, 3, 9 };

    // decide depending on given order, which frame has to draw aside another frame
    if( _bLeft )
        bRetVal = aLeft[ _eRelOrient1 ] >= aLeft[ _eRelOrient2 ];
    else
        bRetVal = aRight[ _eRelOrient1 ] >= aRight[ _eRelOrient2 ];

    return bRetVal;
}
