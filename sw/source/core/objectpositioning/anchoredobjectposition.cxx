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
#include <flyfrm.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <frmatr.hxx>
#include <frmtool.hxx>
#include <svx/svdobj.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <frmfmt.hxx>
#include <fmtornt.hxx>
#include <fmtfollowtextflow.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <ndtxt.hxx>
#include <IDocumentSettingAccess.hxx>
#include <textboxhelper.hxx>
#include <fmtsrnd.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star;

namespace objectpositioning
{

SwAnchoredObjectPosition::SwAnchoredObjectPosition( SdrObject& _rDrawObj )
    : mrDrawObj( _rDrawObj ),
      mpAnchoredObj( nullptr ),
      mpAnchorFrame( nullptr ),
      mpContact( nullptr ),
      mbIsObjFly( false ),
      // #i62875#
      mbFollowTextFlow( false ),
      mbDoNotCaptureAnchoredObj( false )
{
#if OSL_DEBUG_LEVEL > 0
    // assert, if object isn't of expected type
    const bool bObjOfExceptedType =
            dynamic_cast<const SwVirtFlyDrawObj*>( &mrDrawObj) !=  nullptr || // object representing fly frame
            dynamic_cast<const SwDrawVirtObj*>( &mrDrawObj) !=  nullptr    || // 'virtual' drawing object
            ( dynamic_cast<const SdrVirtObj*>( &mrDrawObj) ==  nullptr &&    // 'master' drawing object
              dynamic_cast<const SwFlyDrawObj*>( &mrDrawObj) ==  nullptr );  // - indirectly checked
    OSL_ENSURE( bObjOfExceptedType,
            "SwAnchoredObjectPosition(..) - object of unexpected type!" );
#endif

    GetInfoAboutObj();
}

/** determine information about object

    members <mbIsObjFly>, <mpFrameOfObj>, <mpAnchorFrame>, <mpContact>,
    <mbFollowTextFlow> and <mbDoNotCaptureAnchoredObj> are set
*/
void SwAnchoredObjectPosition::GetInfoAboutObj()
{
    // determine, if object represents a fly frame
    {
        mbIsObjFly = dynamic_cast<const SwVirtFlyDrawObj*>( &mrDrawObj) !=  nullptr;
    }

    // determine contact object
    {
        mpContact = GetUserCall( &mrDrawObj );
        assert(mpContact &&
                "SwAnchoredObjectPosition::GetInfoAboutObj() - missing SwContact-object.");
    }

    // determine anchored object, the object belongs to
    {
        // #i26791#
        mpAnchoredObj = mpContact->GetAnchoredObj( &mrDrawObj );
        assert(mpAnchoredObj &&
                "SwAnchoredObjectPosition::GetInfoAboutObj() - missing anchored object.");
    }

    // determine frame, the object is anchored at
    {
        // #i26791#
        mpAnchorFrame = mpAnchoredObj->AnchorFrame();
        OSL_ENSURE( mpAnchorFrame,
                "SwAnchoredObjectPosition::GetInfoAboutObj() - missing anchor frame." );
    }

    // determine format the object belongs to
    {
        // #i28701#
        mpFrameFormat = mpAnchoredObj->GetFrameFormat();
        assert(mpFrameFormat &&
                "<SwAnchoredObjectPosition::GetInfoAboutObj() - missing frame format.");
    }

    // #i62875# - determine attribute value of <Follow-Text-Flow>
    {
        mbFollowTextFlow = mpFrameFormat->GetFollowTextFlow().GetValue();
    }

    // determine, if anchored object has not to be captured on the page.
    // the following conditions must be hold to *not* capture it:
    // - corresponding document compatibility flag is set
    // - it's a drawing object or it's a non-textbox wrap-though fly frame
    // - it doesn't follow the text flow
    {
        bool bTextBox = SwTextBoxHelper::isTextBox(mpFrameFormat, RES_FLYFRMFMT);
        bool bWrapThrough = mpFrameFormat->GetSurround().GetSurround() == css::text::WrapTextMode_THROUGH;
        mbDoNotCaptureAnchoredObj = (!mbIsObjFly || (!bTextBox && bWrapThrough)) && !mbFollowTextFlow &&
                                    mpFrameFormat->getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE);
    }
}

SwAnchoredObjectPosition::~SwAnchoredObjectPosition()
{}

bool SwAnchoredObjectPosition::IsAnchoredToChar() const
{
    return false;
}

const SwFrame* SwAnchoredObjectPosition::ToCharOrientFrame() const
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
    return 0;
}

/** helper method to determine top of a frame for the vertical
    object positioning

    #i11860#
*/
SwTwips SwAnchoredObjectPosition::GetTopForObjPos( const SwFrame& _rFrame,
                                                    const SwRectFn& _fnRect,
                                                    const bool _bVert ) const
{
    SwTwips nTopOfFrameForObjPos = (_rFrame.getFrameArea().*_fnRect->fnGetTop)();

    if ( _rFrame.IsTextFrame() )
    {
        const SwTextFrame& rTextFrame = static_cast<const SwTextFrame&>(_rFrame);
        if ( _bVert )
        {
            nTopOfFrameForObjPos -=
                rTextFrame.GetUpperSpaceAmountConsideredForPrevFrameAndPageGrid();
        }
        else
        {
            nTopOfFrameForObjPos +=
                rTextFrame.GetUpperSpaceAmountConsideredForPrevFrameAndPageGrid();

            const SwFormatSurround& rSurround = mpFrameFormat->GetSurround();
            bool bWrapThrough = rSurround.GetSurround() == css::text::WrapTextMode_THROUGH;
            // If the frame format is a TextBox of a draw shape, then use the
            // surround of the original shape.
            SwTextBoxHelper::getShapeWrapThrough(mpFrameFormat, bWrapThrough);

            // Get the offset between the top of the text frame and the top of
            // the first line inside the frame that has more than just fly
            // portions.
            nTopOfFrameForObjPos += rTextFrame.GetBaseVertOffsetForFly(!bWrapThrough);
        }
    }

    return nTopOfFrameForObjPos;
}

void SwAnchoredObjectPosition::GetVertAlignmentValues(
                                        const SwFrame& _rVertOrientFrame,
                                        const SwFrame& _rPageAlignLayFrame,
                                        const sal_Int16 _eRelOrient,
                                        SwTwips&      _orAlignAreaHeight,
                                        SwTwips&      _orAlignAreaOffset ) const
{
    SwTwips nHeight = 0;
    SwTwips nOffset = 0;
    SwRectFnSet aRectFnSet(&_rVertOrientFrame);
    // #i11860# - top of <_rVertOrientFrame> for object positioning
    const SwTwips nVertOrientTop = GetTopForObjPos( _rVertOrientFrame, aRectFnSet.FnRect(), aRectFnSet.IsVert() );
    // #i11860# - upper space amount of <_rVertOrientFrame> considered
    // for previous frame
    const SwTwips nVertOrientUpperSpaceForPrevFrameAndPageGrid =
            _rVertOrientFrame.IsTextFrame()
            ? static_cast<const SwTextFrame&>(_rVertOrientFrame).
                        GetUpperSpaceAmountConsideredForPrevFrameAndPageGrid()
            : 0;
    switch ( _eRelOrient )
    {
        case text::RelOrientation::FRAME:
        {
            // #i11860# - consider upper space of previous frame
            nHeight = aRectFnSet.GetHeight(_rVertOrientFrame.getFrameArea()) -
                      nVertOrientUpperSpaceForPrevFrameAndPageGrid;
            nOffset = 0;
        }
        break;
        case text::RelOrientation::PRINT_AREA:
        {
            nHeight = aRectFnSet.GetHeight(_rVertOrientFrame.getFramePrintArea());
            // #i11860# - consider upper space of previous frame
            nOffset = aRectFnSet.GetTopMargin(_rVertOrientFrame) -
                      nVertOrientUpperSpaceForPrevFrameAndPageGrid;
            // if aligned to page in horizontal layout, consider header and
            // footer frame height appropriately.
            if( _rVertOrientFrame.IsPageFrame() && !aRectFnSet.IsVert() )
            {
                const SwFrame* pPrtFrame =
                        static_cast<const SwPageFrame&>(_rVertOrientFrame).Lower();
                while( pPrtFrame )
                {
                    if( pPrtFrame->IsHeaderFrame() )
                    {
                        nHeight -= pPrtFrame->getFrameArea().Height();
                        nOffset += pPrtFrame->getFrameArea().Height();
                    }
                    else if( pPrtFrame->IsFooterFrame() )
                    {
                        nHeight -= pPrtFrame->getFrameArea().Height();
                    }
                    pPrtFrame = pPrtFrame->GetNext();
                }
            }
        }
        break;
        case text::RelOrientation::PAGE_FRAME:
        case text::RelOrientation::PAGE_PRINT_AREA_TOP:
        {
            nHeight = aRectFnSet.GetHeight(_rPageAlignLayFrame.getFrameArea());
            nOffset = aRectFnSet.YDiff(
                        aRectFnSet.GetTop(_rPageAlignLayFrame.getFrameArea()),
                        nVertOrientTop );
        }
        break;
        case text::RelOrientation::PAGE_PRINT_AREA:
        {
            nHeight = aRectFnSet.GetHeight(_rPageAlignLayFrame.getFramePrintArea());
            nOffset = aRectFnSet.GetTopMargin(_rPageAlignLayFrame) +
                      aRectFnSet.YDiff(
                        aRectFnSet.GetTop(_rPageAlignLayFrame.getFrameArea()),
                        nVertOrientTop );
            // if aligned to page in horizontal layout, consider header and
            // footer frame height appropriately.
            if( _rPageAlignLayFrame.IsPageFrame() && !aRectFnSet.IsVert() )
            {
                const SwFrame* pPrtFrame =
                        static_cast<const SwPageFrame&>(_rPageAlignLayFrame).Lower();
                while( pPrtFrame )
                {
                    if( pPrtFrame->IsHeaderFrame() )
                    {
                        nHeight -= pPrtFrame->getFrameArea().Height();
                        nOffset += pPrtFrame->getFrameArea().Height();
                    }
                    else if( pPrtFrame->IsFooterFrame() )
                    {
                        nHeight -= pPrtFrame->getFrameArea().Height();
                    }
                    pPrtFrame = pPrtFrame->GetNext();
                }
            }
        }
        break;
        case text::RelOrientation::PAGE_PRINT_AREA_BOTTOM:
        {
            nHeight = aRectFnSet.GetBottomMargin(_rPageAlignLayFrame);
            nOffset = aRectFnSet.YDiff(
                      aRectFnSet.GetPrtBottom(_rPageAlignLayFrame),
                      nVertOrientTop);

            if (_rPageAlignLayFrame.IsPageFrame() && !aRectFnSet.IsVert())
            {
                const SwFrame* pPrtFrame =
                    static_cast<const SwPageFrame&>(_rPageAlignLayFrame).Lower();

                while (pPrtFrame)
                {
                    if (pPrtFrame->IsFooterFrame())
                    {
                        nHeight += pPrtFrame->getFrameArea().Height();
                        nOffset -= pPrtFrame->getFrameArea().Height();
                    }
                    pPrtFrame = pPrtFrame->GetNext();
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
                nOffset = aRectFnSet.YDiff( ToCharTopOfLine(), nVertOrientTop );
            }
            else
            {
                OSL_FAIL( "<SwAnchoredObjectPosition::GetVertAlignmentValues(..)> - invalid relative alignment" );
            }
        }
        break;
        case text::RelOrientation::CHAR:
        {
            if ( IsAnchoredToChar() )
            {
                nHeight = aRectFnSet.GetHeight(*ToCharRect());
                nOffset = aRectFnSet.YDiff( aRectFnSet.GetTop(*ToCharRect()),
                                              nVertOrientTop );
            }
            else
            {
                OSL_FAIL( "<SwAnchoredObjectPosition::GetVertAlignmentValues(..)> - invalid relative alignment" );
            }
        }
        break;
        // no break here, because text::RelOrientation::CHAR is invalid, if !mbAnchorToChar
        default:
        {
            OSL_FAIL( "<SwAnchoredObjectPosition::GetVertAlignmentValues(..)> - invalid relative alignment" );
        }
    }

    _orAlignAreaHeight = nHeight;
    _orAlignAreaOffset = nOffset;
}

// #i26791# - add output parameter <_roVertOffsetToFrameAnchorPos>
SwTwips SwAnchoredObjectPosition::GetVertRelPos(
                                    const SwFrame& _rVertOrientFrame,
                                    const SwFrame& _rPageAlignLayFrame,
                                    const sal_Int16 _eVertOrient,
                                    const sal_Int16 _eRelOrient,
                                    const SwTwips          _nVertPos,
                                    const SvxLRSpaceItem& _rLRSpacing,
                                    const SvxULSpaceItem& _rULSpacing,
                                    SwTwips& _roVertOffsetToFrameAnchorPos ) const
{
    SwTwips nRelPosY = 0;
    SwRectFnSet aRectFnSet(&_rVertOrientFrame);

    SwTwips nAlignAreaHeight;
    SwTwips nAlignAreaOffset;
    GetVertAlignmentValues( _rVertOrientFrame, _rPageAlignLayFrame,
                             _eRelOrient, nAlignAreaHeight, nAlignAreaOffset );

    nRelPosY = nAlignAreaOffset;
    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );
    const SwTwips nObjHeight = aRectFnSet.GetHeight(aObjBoundRect);

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
              nRelPosY +=   aRectFnSet.IsVert()
                            ? ( aRectFnSet.IsVertL2R()
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
                        ( nObjHeight + ( aRectFnSet.IsVert()
                                         ? ( aRectFnSet.IsVertL2R()
                                             ? _rLRSpacing.GetRight()
                                             : _rLRSpacing.GetLeft() )
                                         : _rULSpacing.GetLower() ) );
        }
        break;
        default:
        {
            OSL_FAIL( "<SwAnchoredObjectPosition::GetVertRelPos(..) - invalid vertical positioning" );
        }
    }

    // #i26791#
    _roVertOffsetToFrameAnchorPos = nAlignAreaOffset;

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
SwTwips SwAnchoredObjectPosition::ImplAdjustVertRelPos( const SwTwips nTopOfAnch,
                                                         const bool bVert,
                                                         const bool bVertL2R,
                                                         const SwFrame& rPageAlignLayFrame,
                                                         const SwTwips nProposedRelPosY,
                                                         const bool bFollowTextFlow,
                                                         const bool bCheckBottom ) const
{
    SwTwips nAdjustedRelPosY = nProposedRelPosY;
    // TODO: Replace the following condition with the correction
    // of the implementation of option FollowTextFlow.
    if ( SwAnchoredObject::IsDraggingOffPageAllowed(FindFrameFormat(&mrDrawObj)) &&
        !(GetAnchorFrame().IsInTab() && DoesObjFollowsTextFlow()) )
    {
        return nAdjustedRelPosY;
    }

    const Size aObjSize(GetAnchoredObj().GetObjRect().SSize());
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
               !GetAnchoredObj().GetAnchorFrame()->IsInTab() ) )
        {
            aPgAlignArea = rPageAlignLayFrame.FindPageFrame()->getFrameArea();
        }
        else
        {
            aPgAlignArea = rPageAlignLayFrame.getFrameArea();
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
            // tdf#112443 if position is completely off-page
            // return the proposed position and do not adjust it...
            // tdf#120839 .. unless anchored to char (anchor can jump on other page)
            const bool bDisablePositioning =  mpFrameFormat->getIDocumentSettingAccess().get(DocumentSettingId::DISABLE_OFF_PAGE_POSITIONING);
            if ( bDisablePositioning && !IsAnchoredToChar() && nTopOfAnch + nAdjustedRelPosY > aPgAlignArea.Right() )
            {
                return nProposedRelPosY;
            }

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
        // tdf#112443 if position is completely off-page
        // return the proposed position and do not adjust it...
        const bool bDisablePositioning =  mpFrameFormat->getIDocumentSettingAccess().get(DocumentSettingId::DISABLE_OFF_PAGE_POSITIONING);

        // tdf#123002 disable the positioning in header and footer only
        // we should limit this since anchors of body frames may appear on other pages
        const bool bIsFooterOrHeader = GetAnchorFrame().GetUpper()
                                     && (GetAnchorFrame().GetUpper()->IsFooterFrame() || GetAnchorFrame().GetUpper()->IsHeaderFrame() );

        if ( bDisablePositioning && bIsFooterOrHeader && nTopOfAnch + nAdjustedRelPosY > aPgAlignArea.Bottom() )
        {
            return nProposedRelPosY;
        }

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
    }
    return nAdjustedRelPosY;
}

/** adjust calculated horizontal in order to keep object inside
    'page' alignment layout frame.

    #i62875# - method now private and renamed.
*/
SwTwips SwAnchoredObjectPosition::ImplAdjustHoriRelPos(
                                        const SwFrame&  _rPageAlignLayFrame,
                                        const SwTwips _nProposedRelPosX ) const
{
    SwTwips nAdjustedRelPosX = _nProposedRelPosX;

    if (SwAnchoredObject::IsDraggingOffPageAllowed(FindFrameFormat(&mrDrawObj)))
        return nAdjustedRelPosX;

    const SwFrame& rAnchorFrame = GetAnchorFrame();
    const bool bVert = rAnchorFrame.IsVertical();

    const Size aObjSize( GetAnchoredObj().GetObjRect().SSize() );

    if( bVert )
    {
        if ( rAnchorFrame.getFrameArea().Top() + nAdjustedRelPosX + aObjSize.Height() >
                _rPageAlignLayFrame.getFrameArea().Bottom() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrame.getFrameArea().Bottom() -
                               rAnchorFrame.getFrameArea().Top() -
                               aObjSize.Height();
        }
        if ( rAnchorFrame.getFrameArea().Top() + nAdjustedRelPosX <
                _rPageAlignLayFrame.getFrameArea().Top() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrame.getFrameArea().Top() -
                               rAnchorFrame.getFrameArea().Top();
        }
    }
    else
    {
        if ( rAnchorFrame.getFrameArea().Left() + nAdjustedRelPosX + aObjSize.Width() >
                _rPageAlignLayFrame.getFrameArea().Right() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrame.getFrameArea().Right() -
                               rAnchorFrame.getFrameArea().Left() -
                               aObjSize.Width();
        }
        if ( rAnchorFrame.getFrameArea().Left() + nAdjustedRelPosX <
                _rPageAlignLayFrame.getFrameArea().Left() )
        {
            nAdjustedRelPosX = _rPageAlignLayFrame.getFrameArea().Left() -
                               rAnchorFrame.getFrameArea().Left();
        }
    }

    return nAdjustedRelPosX;
}

/** determine alignment value for horizontal position of object */
void SwAnchoredObjectPosition::GetHoriAlignmentValues( const SwFrame&  _rHoriOrientFrame,
                                                        const SwFrame&  _rPageAlignLayFrame,
                                                        const sal_Int16 _eRelOrient,
                                                        const bool    _bObjWrapThrough,
                                                        SwTwips&      _orAlignAreaWidth,
                                                        SwTwips&      _orAlignAreaOffset,
                                                        bool&         _obAlignedRelToPage ) const
{
    SwTwips nWidth = 0;
    SwTwips nOffset = 0;
    SwRectFnSet aRectFnSet(&_rHoriOrientFrame);
    switch ( _eRelOrient )
    {
        case text::RelOrientation::PRINT_AREA:
        {
            nWidth = aRectFnSet.GetWidth(_rHoriOrientFrame.getFramePrintArea());
            nOffset = aRectFnSet.GetLeftMargin(_rHoriOrientFrame);
            if ( _rHoriOrientFrame.IsTextFrame() )
            {
                // consider movement of text frame left
                nOffset += static_cast<const SwTextFrame&>(_rHoriOrientFrame).GetBaseOffsetForFly( !_bObjWrapThrough );
            }
            else if ( _rHoriOrientFrame.IsPageFrame() && aRectFnSet.IsVert() )
            {
                // for to-page anchored objects, consider header/footer frame
                // in vertical layout
                const SwFrame* pPrtFrame =
                        static_cast<const SwPageFrame&>(_rHoriOrientFrame).Lower();
                while( pPrtFrame )
                {
                    if( pPrtFrame->IsHeaderFrame() )
                    {
                        nWidth -= pPrtFrame->getFrameArea().Height();
                        nOffset += pPrtFrame->getFrameArea().Height();
                    }
                    else if( pPrtFrame->IsFooterFrame() )
                    {
                        nWidth -= pPrtFrame->getFrameArea().Height();
                    }
                    pPrtFrame = pPrtFrame->GetNext();
                }
            }
            break;
        }
        case text::RelOrientation::PAGE_LEFT:
        {
            // align at left border of page frame/fly frame/cell frame
            nWidth = aRectFnSet.GetLeftMargin(_rPageAlignLayFrame);
            nOffset = aRectFnSet.XDiff(
                      aRectFnSet.GetLeft(_rPageAlignLayFrame.getFrameArea()),
                      aRectFnSet.GetLeft(_rHoriOrientFrame.getFrameArea()) );
            _obAlignedRelToPage = true;
        }
        break;
        case text::RelOrientation::PAGE_RIGHT:
        {
            // align at right border of page frame/fly frame/cell frame
            nWidth = aRectFnSet.GetRightMargin(_rPageAlignLayFrame);
            nOffset = aRectFnSet.XDiff(
                      aRectFnSet.GetPrtRight(_rPageAlignLayFrame),
                      aRectFnSet.GetLeft(_rHoriOrientFrame.getFrameArea()) );
            _obAlignedRelToPage = true;
        }
        break;
        case text::RelOrientation::FRAME_LEFT:
        {
            // align at left border of anchor frame
            nWidth = aRectFnSet.GetLeftMargin(_rHoriOrientFrame);
            nOffset = 0;
        }
        break;
        case text::RelOrientation::FRAME_RIGHT:
        {
            // align at right border of anchor frame
            // Unify and simplify
            nWidth = aRectFnSet.GetRightMargin(_rHoriOrientFrame);
            nOffset = aRectFnSet.GetRight(_rHoriOrientFrame.getFramePrintArea());
        }
        break;
        case text::RelOrientation::CHAR:
        {
            // alignment relative to character - assure, that corresponding
            // character rectangle is set.
            if ( IsAnchoredToChar() )
            {
                nWidth = 0;
                nOffset = aRectFnSet.XDiff(
                            aRectFnSet.GetLeft(*ToCharRect()),
                            aRectFnSet.GetLeft(ToCharOrientFrame()->getFrameArea()) );
                break;
            }
            [[fallthrough]];
        }
        case text::RelOrientation::PAGE_PRINT_AREA:
        {
            nWidth = aRectFnSet.GetWidth(_rPageAlignLayFrame.getFramePrintArea());
            nOffset = aRectFnSet.XDiff(
                        aRectFnSet.GetPrtLeft(_rPageAlignLayFrame),
                        aRectFnSet.GetLeft(_rHoriOrientFrame.getFrameArea()) );
            if ( _rHoriOrientFrame.IsPageFrame() && aRectFnSet.IsVert() )
            {
                // for to-page anchored objects, consider header/footer frame
                // in vertical layout
                const SwFrame* pPrtFrame =
                        static_cast<const SwPageFrame&>(_rHoriOrientFrame).Lower();
                while( pPrtFrame )
                {
                    if( pPrtFrame->IsHeaderFrame() )
                    {
                        nWidth -= pPrtFrame->getFrameArea().Height();
                        nOffset += pPrtFrame->getFrameArea().Height();
                    }
                    else if( pPrtFrame->IsFooterFrame() )
                    {
                        nWidth -= pPrtFrame->getFrameArea().Height();
                    }
                    pPrtFrame = pPrtFrame->GetNext();
                }
            }
            _obAlignedRelToPage = true;
            break;
        }
        case text::RelOrientation::PAGE_FRAME:
        {
            nWidth = aRectFnSet.GetWidth(_rPageAlignLayFrame.getFrameArea());
            nOffset = aRectFnSet.XDiff(
                        aRectFnSet.GetLeft(_rPageAlignLayFrame.getFrameArea()),
                        aRectFnSet.GetLeft(_rHoriOrientFrame.getFrameArea()) );
            _obAlignedRelToPage = true;
            break;
        }
        default:
        {
            nWidth = aRectFnSet.GetWidth(_rHoriOrientFrame.getFrameArea());

            bool bWrapThrough = _bObjWrapThrough;
            // If the frame format is a TextBox of a draw shape, then use the
            // surround of the original shape.
            SwTextBoxHelper::getShapeWrapThrough(mpFrameFormat, bWrapThrough);

            bool bIgnoreFlysAnchoredAtFrame = !bWrapThrough;
            nOffset = _rHoriOrientFrame.IsTextFrame() ?
                   static_cast<const SwTextFrame&>(_rHoriOrientFrame).GetBaseOffsetForFly( bIgnoreFlysAnchoredAtFrame ) :
                   0;
            break;
        }
    }

    _orAlignAreaWidth = nWidth;
    _orAlignAreaOffset = nOffset;
}

/** toggle given horizontal orientation and relative alignment */
void SwAnchoredObjectPosition::ToggleHoriOrientAndAlign(
                                        const bool _bToggleLeftRight,
                                        sal_Int16& _ioeHoriOrient,
                                        sal_Int16& _iopeRelOrient
                                      )
{
    if( !_bToggleLeftRight )
        return;

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

/** calculate relative horizontal position */
SwTwips SwAnchoredObjectPosition::CalcRelPosX(
                                const SwFrame& _rHoriOrientFrame,
                                const SwEnvironmentOfAnchoredObject& _rEnvOfObj,
                                const SwFormatHoriOrient& _rHoriOrient,
                                const SvxLRSpaceItem& _rLRSpacing,
                                const SvxULSpaceItem& _rULSpacing,
                                const bool _bObjWrapThrough,
                                const SwTwips _nRelPosY,
                                SwTwips& _roHoriOffsetToFrameAnchorPos
                              ) const
{
    // determine 'page' alignment layout frame
    const SwFrame& rPageAlignLayFrame =
            _rEnvOfObj.GetHoriEnvironmentLayoutFrame( _rHoriOrientFrame );

    const bool bEvenPage = !rPageAlignLayFrame.OnRightPage();
    const bool bToggle = _rHoriOrient.IsPosToggle() && bEvenPage;

    // determine orientation and relative alignment
    sal_Int16 eHoriOrient = _rHoriOrient.GetHoriOrient();
    sal_Int16 eRelOrient = _rHoriOrient.GetRelationOrient();
    // toggle orientation and relative alignment
    ToggleHoriOrientAndAlign( bToggle, eHoriOrient, eRelOrient );

    // determine alignment parameter
    // <nWidth>:  'width' of alignment area
    // <nOffset>: offset of alignment area, relative to 'left' of anchor frame
    SwTwips nWidth = 0;
    SwTwips nOffset = 0;
    bool bAlignedRelToPage = false;
    GetHoriAlignmentValues( _rHoriOrientFrame, rPageAlignLayFrame,
                             eRelOrient, _bObjWrapThrough,
                             nWidth, nOffset, bAlignedRelToPage );

    const SwFrame& rAnchorFrame = GetAnchorFrame();
    SwRectFnSet aRectFnSet(&_rHoriOrientFrame);
    SwTwips nObjWidth = aRectFnSet.GetWidth(GetAnchoredObj().GetObjRect());
    SwTwips nRelPosX = nOffset;
    if ( _rHoriOrient.GetHoriOrient() == text::HoriOrientation::NONE )
    {
        // 'manual' horizontal position
        const bool bR2L = rAnchorFrame.IsRightToLeft();
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
                      ( aRectFnSet.IsVert() ? _rULSpacing.GetLower() : _rLRSpacing.GetRight() ) );
    else
        nRelPosX += aRectFnSet.IsVert() ? _rULSpacing.GetUpper() : _rLRSpacing.GetLeft();

    // adjust relative position by distance between anchor frame and
    // the frame, the object is oriented at.
    if ( &rAnchorFrame != &_rHoriOrientFrame )
    {
        SwTwips nLeftOrient = aRectFnSet.GetLeft(_rHoriOrientFrame.getFrameArea());
        SwTwips nLeftAnchor = aRectFnSet.GetLeft(rAnchorFrame.getFrameArea());
        nRelPosX += aRectFnSet.XDiff( nLeftOrient, nLeftAnchor );
    }

    // adjust calculated relative horizontal position, in order to
    // keep object inside 'page' alignment layout frame
    const SwFrame& rEnvironmentLayFrame =
            _rEnvOfObj.GetHoriEnvironmentLayoutFrame( _rHoriOrientFrame );
    bool bFollowTextFlow = GetFrameFormat().GetFollowTextFlow().GetValue();
    bool bWrapThrough = GetFrameFormat().GetSurround().GetSurround() != text::WrapTextMode_THROUGH;
    // Don't try to keep wrap-though objects inside the cell, even if they are following text flow.
    if (!rEnvironmentLayFrame.IsInTab() || !bFollowTextFlow || bWrapThrough)
    {
        nRelPosX = AdjustHoriRelPos( rEnvironmentLayFrame, nRelPosX );
    }

    // if object is a Writer fly frame and it's anchored to a content and
    // it is horizontal positioned left or right, but not relative to character,
    // it has to be drawn aside another object, which have the same horizontal
    // position and lay below it.
    if ( GetAnchoredObj().DynCastFlyFrame() &&
         ( mpContact->ObjAnchoredAtPara() || mpContact->ObjAnchoredAtChar() ) &&
         ( eHoriOrient == text::HoriOrientation::LEFT || eHoriOrient == text::HoriOrientation::RIGHT ) &&
         eRelOrient != text::RelOrientation::CHAR )
    {
        nRelPosX = AdjustHoriRelPosForDrawAside( _rHoriOrientFrame,
                                                  nRelPosX, _nRelPosY,
                                                  eHoriOrient, eRelOrient,
                                                  _rLRSpacing, _rULSpacing,
                                                  bEvenPage );
    }

    // #i26791#
    _roHoriOffsetToFrameAnchorPos = nOffset;

    return nRelPosX;
}

// method incl. helper methods for adjusting proposed horizontal position,
// if object has to draw aside another object.
/** adjust calculated horizontal position in order to draw object
    aside other objects with same positioning
*/
SwTwips SwAnchoredObjectPosition::AdjustHoriRelPosForDrawAside(
                                            const SwFrame&  _rHoriOrientFrame,
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
    if ( GetAnchorFrame().DynCastTextFrame() ==  nullptr ||
         dynamic_cast<const SwFlyAtContentFrame*>( &GetAnchoredObj() ) ==  nullptr )
    {
        OSL_FAIL( "<SwAnchoredObjectPosition::AdjustHoriRelPosForDrawAside(..) - usage for wrong anchor type" );
        return _nProposedRelPosX;
    }

    const SwTextFrame& rAnchorTextFrame = static_cast<const SwTextFrame&>(GetAnchorFrame());
    // #i26791#
    const SwFlyAtContentFrame& rFlyAtContentFrame =
                        static_cast<const SwFlyAtContentFrame&>(GetAnchoredObj());
    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );
    SwRectFnSet aRectFnSet(&_rHoriOrientFrame);

    SwTwips nAdjustedRelPosX = _nProposedRelPosX;

    // determine proposed object bound rectangle
    Point aTmpPos = aRectFnSet.GetPos(rAnchorTextFrame.getFrameArea());
    if( aRectFnSet.IsVert() )
    {
        aTmpPos.AdjustX( -(_nRelPosY + aObjBoundRect.Width()) );
        aTmpPos.AdjustY(nAdjustedRelPosX );
    }
    else
    {
        aTmpPos.AdjustX(nAdjustedRelPosX );
        aTmpPos.AdjustY(_nRelPosY );
    }
    SwRect aTmpObjRect( aTmpPos, aObjBoundRect.SSize() );

    const sal_uInt32 nObjOrdNum = GetObject().GetOrdNum();
    const SwPageFrame* pObjPage = rFlyAtContentFrame.FindPageFrame();
    const SwFrame* pObjContext = ::FindContext( &rAnchorTextFrame, SwFrameType::Column );
    SwNodeOffset nObjIndex = rAnchorTextFrame.GetTextNodeFirst()->GetIndex();
    SwOrderIter aIter( pObjPage );
    const SwFlyFrame* pFly = static_cast<const SwVirtFlyDrawObj*>(aIter.Bottom())->GetFlyFrame();
    while ( pFly && nObjOrdNum > pFly->GetVirtDrawObj()->GetOrdNumDirect() )
    {
        if ( DrawAsideFly( pFly, aTmpObjRect, pObjContext, nObjIndex,
                           _bEvenPage, _eHoriOrient, _eRelOrient ) )
        {
            if( aRectFnSet.IsVert() )
            {
                const SvxULSpaceItem& rOtherUL = pFly->GetFormat()->GetULSpace();
                const SwTwips nOtherTop = pFly->getFrameArea().Top() - rOtherUL.GetUpper();
                const SwTwips nOtherBot = pFly->getFrameArea().Bottom() + rOtherUL.GetLower();
                if ( nOtherTop <= aTmpObjRect.Bottom() + _rULSpacing.GetLower() &&
                     nOtherBot >= aTmpObjRect.Top() - _rULSpacing.GetUpper() )
                {
                    if ( _eHoriOrient == text::HoriOrientation::LEFT )
                    {
                        SwTwips nTmp = nOtherBot + 1 + _rULSpacing.GetUpper() -
                                       rAnchorTextFrame.getFrameArea().Top();
                        if ( nTmp > nAdjustedRelPosX &&
                             rAnchorTextFrame.getFrameArea().Top() + nTmp +
                             aObjBoundRect.Height() + _rULSpacing.GetLower()
                             <= pObjPage->getFrameArea().Height() + pObjPage->getFrameArea().Top() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    else if ( _eHoriOrient == text::HoriOrientation::RIGHT )
                    {
                        SwTwips nTmp = nOtherTop - 1 - _rULSpacing.GetLower() -
                                       aObjBoundRect.Height() -
                                       rAnchorTextFrame.getFrameArea().Top();
                        if ( nTmp < nAdjustedRelPosX &&
                             rAnchorTextFrame.getFrameArea().Top() + nTmp - _rULSpacing.GetUpper()
                              >= pObjPage->getFrameArea().Top() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    aTmpObjRect.Pos().setY( rAnchorTextFrame.getFrameArea().Top() +
                                            nAdjustedRelPosX );
                }
            }
            else
            {
                const SvxLRSpaceItem& rOtherLR = pFly->GetFormat()->GetLRSpace();
                const SwTwips nOtherLeft = pFly->getFrameArea().Left() - rOtherLR.GetLeft();
                const SwTwips nOtherRight = pFly->getFrameArea().Right() + rOtherLR.GetRight();
                if( nOtherLeft <= aTmpObjRect.Right() + _rLRSpacing.GetRight() &&
                    nOtherRight >= aTmpObjRect.Left() - _rLRSpacing.GetLeft() )
                {
                    if ( _eHoriOrient == text::HoriOrientation::LEFT )
                    {
                        SwTwips nTmp = nOtherRight + 1 + _rLRSpacing.GetLeft() -
                                       rAnchorTextFrame.getFrameArea().Left();
                        if ( nTmp > nAdjustedRelPosX &&
                             rAnchorTextFrame.getFrameArea().Left() + nTmp +
                             aObjBoundRect.Width() + _rLRSpacing.GetRight()
                             <= pObjPage->getFrameArea().Width() + pObjPage->getFrameArea().Left() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    else if ( _eHoriOrient == text::HoriOrientation::RIGHT )
                    {
                        SwTwips nTmp = nOtherLeft - 1 - _rLRSpacing.GetRight() -
                                       aObjBoundRect.Width() -
                                       rAnchorTextFrame.getFrameArea().Left();
                        if ( nTmp < nAdjustedRelPosX &&
                             rAnchorTextFrame.getFrameArea().Left() + nTmp - _rLRSpacing.GetLeft()
                             >= pObjPage->getFrameArea().Left() )
                        {
                            nAdjustedRelPosX = nTmp;
                        }
                    }
                    aTmpObjRect.Pos().setX( rAnchorTextFrame.getFrameArea().Left() +
                                            nAdjustedRelPosX );
                }
            } // end of <if (bVert)>
        } // end of <if DrawAsideFly(..)>

        pFly = static_cast<const SwVirtFlyDrawObj*>(aIter.Next())->GetFlyFrame();
    } // end of <loop on fly frames

    return nAdjustedRelPosX;
}

/** determine, if object has to draw aside given fly frame

    method used by <AdjustHoriRelPosForDrawAside(..)>
*/
bool SwAnchoredObjectPosition::DrawAsideFly( const SwFlyFrame* _pFly,
                                              const SwRect&   _rObjRect,
                                              const SwFrame*    _pObjContext,
                                              const SwNodeOffset _nObjIndex,
                                              const bool      _bEvenPage,
                                              const sal_Int16 _eHoriOrient,
                                              const sal_Int16 _eRelOrient
                                            ) const
{
    bool bRetVal = false;

    SwRectFnSet aRectFnSet(&GetAnchorFrame());

    if ( _pFly->IsFlyAtContentFrame() &&
         aRectFnSet.BottomDist( _pFly->getFrameArea(), aRectFnSet.GetTop(_rObjRect) ) < 0 &&
         aRectFnSet.BottomDist( _rObjRect, aRectFnSet.GetTop(_pFly->getFrameArea()) ) < 0 &&
         ::FindContext( _pFly->GetAnchorFrame(), SwFrameType::Column ) == _pObjContext )
    {
        SwNodeOffset nOtherIndex =
            static_cast<const SwTextFrame*>(_pFly->GetAnchorFrame())->GetTextNodeFirst()->GetIndex();
        if (sw::FrameContainsNode(static_cast<SwTextFrame const&>(*_pFly->GetAnchorFrame()), _nObjIndex)
            || nOtherIndex < _nObjIndex)
        {
            const SwFormatHoriOrient& rHori = _pFly->GetFormat()->GetHoriOrient();
            sal_Int16 eOtherRelOrient = rHori.GetRelationOrient();
            if( text::RelOrientation::CHAR != eOtherRelOrient )
            {
                sal_Int16 eOtherHoriOrient = rHori.GetHoriOrient();
                ToggleHoriOrientAndAlign( _bEvenPage && rHori.IsPosToggle(),
                                           eOtherHoriOrient,
                                           eOtherRelOrient );
                if ( eOtherHoriOrient == _eHoriOrient &&
                    Minor_( _eRelOrient, eOtherRelOrient, text::HoriOrientation::LEFT == _eHoriOrient ) )
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
    method used by <DrawAsideFly(..)>
*/
bool SwAnchoredObjectPosition::Minor_( sal_Int16 _eRelOrient1,
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
