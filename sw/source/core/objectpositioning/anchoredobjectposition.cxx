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

#include <anchoredobjectposition.hxx>
#include <environmentofanchoredobject.hxx>
#include <doc.hxx>
#include <flyfrm.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <frmtool.hxx>
#include <svx/svdobj.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <frmfmt.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtfollowtextflow.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <ndtxt.hxx>
#include <IDocumentSettingAccess.hxx>
#include <textboxhelper.hxx>

using namespace ::com::sun::star;
using namespace objectpositioning;

SwAnchoredObjectPosition::SwAnchoredObjectPosition( SdrObject& _rDrawObj )
    : mrDrawObj( _rDrawObj ),
      mbIsObjFly( false ),
      mpAnchoredObj( nullptr ),
      mpAnchorFrm( nullptr ),
      mpContact( nullptr ),
      // #i62875#
      mbFollowTextFlow( false ),
      mbDoNotCaptureAnchoredObj( false )
{
#if OSL_DEBUG_LEVEL > 0
    // assert, if object isn't of excepted type
    const bool bObjOfExceptedType =
            dynamic_cast<const SwVirtFlyDrawObj*>( &mrDrawObj) !=  nullptr || // object representing fly frame
            dynamic_cast<const SwDrawVirtObj*>( &mrDrawObj) !=  nullptr    || // 'virtual' drawing object
            ( dynamic_cast<const SdrVirtObj*>( &mrDrawObj) ==  nullptr &&    // 'master' drawing object
              dynamic_cast<const SwFlyDrawObj*>( &mrDrawObj) ==  nullptr );  // - indirectly checked
    (void) bObjOfExceptedType;
    OSL_ENSURE( bObjOfExceptedType,
            "SwAnchoredObjectPosition(..) - object of unexpected type!" );
#endif

    _GetInfoAboutObj();
}

/** determine information about object

    members <mbIsObjFly>, <mpFrmOfObj>, <mpAnchorFrm>, <mpContact>,
    <mbFollowTextFlow> and <mbDoNotCaptureAnchoredObj> are set
*/
void SwAnchoredObjectPosition::_GetInfoAboutObj()
{
    // determine, if object represents a fly frame
    {
        mbIsObjFly = dynamic_cast<const SwVirtFlyDrawObj*>( &mrDrawObj) !=  nullptr;
    }

    // determine contact object
    {
        mpContact = static_cast<SwContact*>(GetUserCall( &mrDrawObj ));
        assert(mpContact &&
                "SwAnchoredObjectPosition::_GetInfoAboutObj() - missing SwContact-object.");
    }

    // determine anchored object, the object belongs to
    {
        // #i26791#
        mpAnchoredObj = mpContact->GetAnchoredObj( &mrDrawObj );
        assert(mpAnchoredObj &&
                "SwAnchoredObjectPosition::_GetInfoAboutObj() - missing anchored object.");
    }

    // determine frame, the object is anchored at
    {
        // #i26791#
        mpAnchorFrm = mpAnchoredObj->AnchorFrm();
        OSL_ENSURE( mpAnchorFrm,
                "SwAnchoredObjectPosition::_GetInfoAboutObj() - missing anchor frame." );
    }

    // determine format the object belongs to
    {
        // #i28701#
        mpFrameFormat = &mpAnchoredObj->GetFrameFormat();
        assert(mpFrameFormat &&
                "<SwAnchoredObjectPosition::_GetInfoAboutObj() - missing frame format.");
    }

    // #i62875# - determine attribute value of <Follow-Text-Flow>
    {
        mbFollowTextFlow = mpFrameFormat->GetFollowTextFlow().GetValue();
    }

    // determine, if anchored object has not to be captured on the page.
    // the following conditions must be hold to *not* capture it:
    // - corresponding document compatibility flag is set
    // - it's a drawing object
    // - it doesn't follow the text flow
    {
        mbDoNotCaptureAnchoredObj = !mbIsObjFly && !mbFollowTextFlow &&
                                    mpFrameFormat->getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE);
    }
}

SwAnchoredObjectPosition::~SwAnchoredObjectPosition()
{}

bool SwAnchoredObjectPosition::IsAnchoredToChar() const
{
    return false;
}

const SwFrm* SwAnchoredObjectPosition::ToCharOrientFrm() const
{
    return nullptr;
}

const SwRect* SwAnchoredObjectPosition::ToCharRect() const
{
    return nullptr;
}

// #i22341#
SwTwips SwAnchoredObjectPosition::ToCharTopOfLine() const
{
    return 0L;
}

/** helper method to determine top of a frame for the vertical
    object positioning

    #i11860#
*/
SwTwips SwAnchoredObjectPosition::_GetTopForObjPos( const SwFrm& _rFrm,
                                                    const SwRectFn& _fnRect,
                                                    const bool _bVert )
{
    SwTwips nTopOfFrmForObjPos = (_rFrm.Frm().*_fnRect->fnGetTop)();

    if ( _rFrm.IsTextFrm() )
    {
        const SwTextFrm& rTextFrm = static_cast<const SwTextFrm&>(_rFrm);
        if ( _bVert )
        {
            nTopOfFrmForObjPos -=
                rTextFrm.GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
        }
        else
        {
            nTopOfFrmForObjPos +=
                rTextFrm.GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
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
    // #i11860# - top of <_rVertOrientFrm> for object positioning
    const SwTwips nVertOrientTop = _GetTopForObjPos( _rVertOrientFrm, fnRect, bVert );
    // #i11860# - upper space amount of <_rVertOrientFrm> considered
    // for previous frame
    const SwTwips nVertOrientUpperSpaceForPrevFrmAndPageGrid =
            _rVertOrientFrm.IsTextFrm()
            ? static_cast<const SwTextFrm&>(_rVertOrientFrm).
                        GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid()
            : 0;
    switch ( _eRelOrient )
    {
        case text::RelOrientation::FRAME:
        {
            // #i11860# - consider upper space of previous frame
            nHeight = (_rVertOrientFrm.Frm().*fnRect->fnGetHeight)() -
                      nVertOrientUpperSpaceForPrevFrmAndPageGrid;
            nOffset = 0;
        }
        break;
        case text::RelOrientation::PRINT_AREA:
        {
            nHeight = (_rVertOrientFrm.Prt().*fnRect->fnGetHeight)();
            // #i11860# - consider upper space of previous frame
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
        // #i22341# - vertical alignment at top of line
        case text::RelOrientation::TEXT_LINE:
        {
            if ( IsAnchoredToChar() )
            {
                nHeight = 0;
                nOffset = (*fnRect->fnYDiff)( ToCharTopOfLine(), nVertOrientTop );
            }
            else
            {
                OSL_FAIL( "<SwAnchoredObjectPosition::_GetVertAlignmentValues(..)> - invalid relative alignment" );
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
                OSL_FAIL( "<SwAnchoredObjectPosition::_GetVertAlignmentValues(..)> - invalid relative alignment" );
            }
        }
        break;
        // no break here, because text::RelOrientation::CHAR is invalid, if !mbAnchorToChar
        default:
        {
            OSL_FAIL( "<SwAnchoredObjectPosition::_GetVertAlignmentValues(..)> - invalid relative alignment" );
        }
    }

    _orAlignAreaHeight = nHeight;
    _orAlignAreaOffset = nOffset;
}

// #i26791# - add output parameter <_roVertOffsetToFrmAnchorPos>
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
              nRelPosY +=   bVert
                            ? ( bVertL2R
                                ? _rLRSpacing.GetLeft()
                                : _rLRSpacing.GetRight() )
                            : _rULSpacing.GetUpper();
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
                        ( nObjHeight + ( bVert
                                         ? ( bVertL2R
                                             ? _rLRSpacing.GetRight()
                                             : _rLRSpacing.GetLeft() )
                                         : _rULSpacing.GetLower() ) );
        }
        break;
        default:
        {
            OSL_FAIL( "<SwAnchoredObjectPosition::_GetVertRelPos(..) - invalid vertical positioning" );
        }
    }

    // #i26791#
    _roVertOffsetToFrmAnchorPos = nAlignAreaOffset;

    return nRelPosY;
}

/** adjust calculated vertical in order to keep object inside
    'page' alignment layout frame.

    #i28701# - parameter <_nTopOfAnch> and <_bVert> added
    #i31805# - add parameter <_bCheckBottom>
    #i26945# - add parameter <_bFollowTextFlow>
    #i62875# - method now private and renamed.
    OD 2009-09-01 #mongolianlayout# - add parameter <bVertL2R>
*/
SwTwips SwAnchoredObjectPosition::_ImplAdjustVertRelPos( const SwTwips nTopOfAnch,
                                                         const bool bVert,
                                                         const bool bVertL2R,
                                                         const SwFrm& rPageAlignLayFrm,
                                                         const SwTwips nProposedRelPosY,
                                                         const bool bFollowTextFlow,
                                                         const bool bCheckBottom ) const
{
    SwTwips nAdjustedRelPosY = nProposedRelPosY;

    const Size aObjSize( GetAnchoredObj().GetObjRect().SSize() );

    // determine the area of 'page' alignment frame, to which the vertical
    // position is restricted.
    // #i28701# - Extend restricted area for the vertical
    // position to area of the page frame, if wrapping style influence is
    // considered on object positioning. Needed to avoid layout loops in the
    // object positioning algorithm considering the wrapping style influence
    // caused by objects, which follow the text flow and thus are restricted
    // to its environment (e.g. page header/footer).
    SwRect aPgAlignArea;
    {
        // #i26945# - no extension of restricted area, if
        // object's attribute follow text flow is set and its inside a table
        if ( GetFrameFormat().getIDocumentSettingAccess().get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) &&
             ( !bFollowTextFlow ||
               !GetAnchoredObj().GetAnchorFrm()->IsInTab() ) )
        {
            aPgAlignArea = rPageAlignLayFrm.FindPageFrm()->Frm();
        }
        else
        {
            aPgAlignArea = rPageAlignLayFrm.Frm();
        }
    }

    if ( bVert )
    {
        // #i31805# - consider value of <_bCheckBottom>
        if ( !bVertL2R )
        {
            if ( bCheckBottom &&
                 nTopOfAnch - nAdjustedRelPosY - aObjSize.Width() <
                    aPgAlignArea.Left() )
            {
                nAdjustedRelPosY = aPgAlignArea.Left() +
                                   nTopOfAnch -
                                   aObjSize.Width();
            }
        // #i32964# - correction
            if ( nTopOfAnch - nAdjustedRelPosY > aPgAlignArea.Right() )
            {
                nAdjustedRelPosY = nTopOfAnch - aPgAlignArea.Right();
            }
        }
        else
        {
            if ( bCheckBottom &&
                 nTopOfAnch + nAdjustedRelPosY + aObjSize.Width() >
                    aPgAlignArea.Right() )
            {
                nAdjustedRelPosY = aPgAlignArea.Right() -
                                   nTopOfAnch -
                                   aObjSize.Width();
            }
            if ( nTopOfAnch + nAdjustedRelPosY < aPgAlignArea.Left() )
            {
                nAdjustedRelPosY = aPgAlignArea.Left() - nTopOfAnch;
            }
        }
    }
    else
    {
        // #i31805# - consider value of <bCheckBottom>
        if ( bCheckBottom &&
             nTopOfAnch + nAdjustedRelPosY + aObjSize.Height() >
                aPgAlignArea.Top() + aPgAlignArea.Height() )
        {
            nAdjustedRelPosY = aPgAlignArea.Top() + aPgAlignArea.Height() -
                               nTopOfAnch -
                               aObjSize.Height();
        }
        if ( nTopOfAnch + nAdjustedRelPosY < aPgAlignArea.Top() )
        {
            nAdjustedRelPosY = aPgAlignArea.Top() - nTopOfAnch;
        }

        // tdf#91260 - allow textboxes extending beyond the page bottom
        if ( nAdjustedRelPosY < nProposedRelPosY )
        {
            const SwFrameFormat* pFormat = &(GetFrameFormat());
            if ( SwTextBoxHelper::isTextBox(&GetObject()) )
            {
                // shrink textboxes to extend beyond the page bottom
                SwFrameFormat* pFrameFormat = ::FindFrameFormat(&GetObject());
                SwFormatFrmSize aSize(pFormat->GetFrmSize());
                SwTwips nShrinked = aSize.GetHeight() - (nProposedRelPosY - nAdjustedRelPosY);
                if (nShrinked >= 0) {
                    aSize.SetHeight( nShrinked );
                    pFrameFormat->SetFormatAttr(aSize);
                }
                nAdjustedRelPosY = nProposedRelPosY;
            } else if ( SwTextBoxHelper::findTextBox(pFormat) )
                // when the shape has a textbox, use only the proposed vertical position
                nAdjustedRelPosY = nProposedRelPosY;
        }
    }
    return nAdjustedRelPosY;
}

/** adjust calculated horizontal in order to keep object inside
    'page' alignment layout frame.

    #i62875# - method now private and renamed.
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

/** determine alignment value for horizontal position of object */
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
            if ( _rHoriOrientFrm.IsTextFrm() )
            {
                // consider movement of text frame left
                nOffset += static_cast<const SwTextFrm&>(_rHoriOrientFrm).GetBaseOfstForFly( !_bObjWrapThrough );
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
            // Unify and simplify
            nWidth = (_rHoriOrientFrm.*fnRect->fnGetRightMargin)();
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
        {
            nWidth = (_rHoriOrientFrm.Frm().*fnRect->fnGetWidth)();

            // When positioning TextBoxes, always ignore flys anchored at the
            // text frame, as we do want to have the textbox overlap with its
            // draw shape.
            bool bIgnoreFlysAnchoredAtFrame = !_bObjWrapThrough || SwTextBoxHelper::isTextBox(&GetObject());
            nOffset = _rHoriOrientFrm.IsTextFrm() ?
                   static_cast<const SwTextFrm&>(_rHoriOrientFrm).GetBaseOfstForFly( bIgnoreFlysAnchoredAtFrame ) :
                   0;
            break;
        }
    }

    _orAlignAreaWidth = nWidth;
    _orAlignAreaOffset = nOffset;
}

/** toggle given horizontal orientation and relative alignment */
void SwAnchoredObjectPosition::_ToggleHoriOrientAndAlign(
                                        const bool _bToggleLeftRight,
                                        sal_Int16& _ioeHoriOrient,
                                        sal_Int16& _iopeRelOrient
                                      )
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

/** calculate relative horizontal position */
SwTwips SwAnchoredObjectPosition::_CalcRelPosX(
                                const SwFrm& _rHoriOrientFrm,
                                const SwEnvironmentOfAnchoredObject& _rEnvOfObj,
                                const SwFormatHoriOrient& _rHoriOrient,
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
            // Correction: consider <nOffset> also for
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

    // adjust calculated relative horizontal position, in order to
    // keep object inside 'page' alignment layout frame
    const SwFrm& rEnvironmentLayFrm =
            _rEnvOfObj.GetHoriEnvironmentLayoutFrm( _rHoriOrientFrm );
    nRelPosX = _AdjustHoriRelPos( rEnvironmentLayFrm, nRelPosX );

    // if object is a Writer fly frame and it's anchored to a content and
    // it is horizontal positioned left or right, but not relative to character,
    // it has to be drawn aside another object, which have the same horizontal
    // position and lay below it.
    if ( dynamic_cast<const SwFlyFrm*>( &GetAnchoredObj() ) !=  nullptr &&
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

    // #i26791#
    _roHoriOffsetToFrmAnchorPos = nOffset;

    return nRelPosX;
}

// method incl. helper methods for adjusting proposed horizontal position,
// if object has to draw aside another object.
/** adjust calculated horizontal position in order to draw object
    aside other objects with same positioning
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
    // #i26791#
    if ( dynamic_cast<const SwTextFrm*>( &GetAnchorFrm() ) ==  nullptr ||
         dynamic_cast<const SwFlyAtCntFrm*>( &GetAnchoredObj() ) ==  nullptr )
    {
        OSL_FAIL( "<SwAnchoredObjectPosition::_AdjustHoriRelPosForDrawAside(..) - usage for wrong anchor type" );
        return _nProposedRelPosX;
    }

    const SwTextFrm& rAnchorTextFrm = static_cast<const SwTextFrm&>(GetAnchorFrm());
    // #i26791#
    const SwFlyAtCntFrm& rFlyAtCntFrm =
                        static_cast<const SwFlyAtCntFrm&>(GetAnchoredObj());
    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );
    SWRECTFN( (&_rHoriOrientFrm) )

    SwTwips nAdjustedRelPosX = _nProposedRelPosX;

    // determine proposed object bound rectangle
    Point aTmpPos = (rAnchorTextFrm.Frm().*fnRect->fnGetPos)();
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
    const SwFrm* pObjContext = ::FindKontext( &rAnchorTextFrm, FRM_COLUMN );
    sal_uLong nObjIndex = rAnchorTextFrm.GetTextNode()->GetIndex();
    SwOrderIter aIter( pObjPage, true );
    const SwFlyFrm* pFly = static_cast<const SwVirtFlyDrawObj*>(aIter.Bottom())->GetFlyFrm();
    while ( pFly && nObjOrdNum > pFly->GetVirtDrawObj()->GetOrdNumDirect() )
    {
        if ( _DrawAsideFly( pFly, aTmpObjRect, pObjContext, nObjIndex,
                           _bEvenPage, _eHoriOrient, _eRelOrient ) )
        {
            if( bVert )
            {
                const SvxULSpaceItem& rOtherUL = pFly->GetFormat()->GetULSpace();
                const SwTwips nOtherTop = pFly->Frm().Top() - rOtherUL.GetUpper();
                const SwTwips nOtherBot = pFly->Frm().Bottom() + rOtherUL.GetLower();
                if ( nOtherTop <= aTmpObjRect.Bottom() + _rULSpacing.GetLower() &&
                     nOtherBot >= aTmpObjRect.Top() - _rULSpacing.GetUpper() )
                {
                    if ( _eHoriOrient == text::HoriOrientation::LEFT )
                    {
                        SwTwips nTmp = nOtherBot + 1 + _rULSpacing.GetUpper() -
                                       rAnchorTextFrm.Frm().Top();
                        if ( nTmp > nAdjustedRelPosX &&
                             rAnchorTextFrm.Frm().Top() + nTmp +
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
                                       rAnchorTextFrm.Frm().Top();
                        if ( nTmp < nAdjustedRelPosX &&
                             rAnchorTextFrm.Frm().Top() + nTmp - _rULSpacing.GetUpper()
                              >= pObjPage->Frm().Top() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    aTmpObjRect.Pos().Y() = rAnchorTextFrm.Frm().Top() +
                                            nAdjustedRelPosX;
                }
            }
            else
            {
                const SvxLRSpaceItem& rOtherLR = pFly->GetFormat()->GetLRSpace();
                const SwTwips nOtherLeft = pFly->Frm().Left() - rOtherLR.GetLeft();
                const SwTwips nOtherRight = pFly->Frm().Right() + rOtherLR.GetRight();
                if( nOtherLeft <= aTmpObjRect.Right() + _rLRSpacing.GetRight() &&
                    nOtherRight >= aTmpObjRect.Left() - _rLRSpacing.GetLeft() )
                {
                    if ( _eHoriOrient == text::HoriOrientation::LEFT )
                    {
                        SwTwips nTmp = nOtherRight + 1 + _rLRSpacing.GetLeft() -
                                       rAnchorTextFrm.Frm().Left();
                        if ( nTmp > nAdjustedRelPosX &&
                             rAnchorTextFrm.Frm().Left() + nTmp +
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
                                       rAnchorTextFrm.Frm().Left();
                        if ( nTmp < nAdjustedRelPosX &&
                             rAnchorTextFrm.Frm().Left() + nTmp - _rLRSpacing.GetLeft()
                             >= pObjPage->Frm().Left() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    aTmpObjRect.Pos().X() = rAnchorTextFrm.Frm().Left() +
                                            nAdjustedRelPosX;
                }
            } // end of <if (bVert)>
        } // end of <if _DrawAsideFly(..)>

        pFly = static_cast<const SwVirtFlyDrawObj*>(aIter.Next())->GetFlyFrm();
    } // end of <loop on fly frames

    return nAdjustedRelPosX;
}

/** detemine, if object has to draw aside given fly frame

    method used by <_AdjustHoriRelPosForDrawAside(..)>
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
            static_cast<const SwTextFrm*>(_pFly->GetAnchorFrm())->GetTextNode()->GetIndex();
        if( _nObjIndex >= nOtherIndex )
        {
            const SwFormatHoriOrient& rHori = _pFly->GetFormat()->GetHoriOrient();
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
*/
bool SwAnchoredObjectPosition::_Minor( sal_Int16 _eRelOrient1,
                                       sal_Int16 _eRelOrient2,
                                       bool             _bLeft )
{
    bool bRetVal;

    // draw aside order for left horizontal position
    //! one array entry for each value in text::RelOrientation
    static sal_uInt16 const aLeft[ 10 ] =
        { 5, 6, 0, 1, 8, 4, 7, 2, 3, 9 };
    // draw aside order for right horizontal position
    //! one array entry for each value in text::RelOrientation
    static sal_uInt16 const aRight[ 10 ] =
        { 5, 6, 0, 8, 1, 7, 4, 2, 3, 9 };

    // decide depending on given order, which frame has to draw aside another frame
    if( _bLeft )
        bRetVal = aLeft[ _eRelOrient1 ] >= aLeft[ _eRelOrient2 ];
    else
        bRetVal = aRight[ _eRelOrient1 ] >= aRight[ _eRelOrient2 ];

    return bRetVal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
