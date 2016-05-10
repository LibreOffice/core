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

#include <tocntntanchoredobjectposition.hxx>
#include <anchoredobject.hxx>
#include <frame.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <sectfrm.hxx>
#include <tabfrm.hxx>
#include "rootfrm.hxx"
#include "viewopt.hxx"
#include "viewsh.hxx"
#include <frmfmt.hxx>
#include <IDocumentSettingAccess.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svx/svdobj.hxx>
#include <pam.hxx>
#include <environmentofanchoredobject.hxx>
#include <frmtool.hxx>
#include <ndtxt.hxx>
#include <dflyobj.hxx>

using namespace objectpositioning;
using namespace ::com::sun::star;

SwToContentAnchoredObjectPosition::SwToContentAnchoredObjectPosition( SdrObject& _rDrawObj )
    : SwAnchoredObjectPosition ( _rDrawObj ),
      mpVertPosOrientFrame( nullptr ),
      // #i26791#
      maOffsetToFrameAnchorPos( Point() ),
      mbAnchorToChar ( false ),
      mpToCharOrientFrame( nullptr ),
      mpToCharRect( nullptr ),
      // #i22341#
      mnToCharTopOfLine( 0 )
{}

SwToContentAnchoredObjectPosition::~SwToContentAnchoredObjectPosition()
{}

bool SwToContentAnchoredObjectPosition::IsAnchoredToChar() const
{
    return mbAnchorToChar;
}

const SwFrame* SwToContentAnchoredObjectPosition::ToCharOrientFrame() const
{
    return mpToCharOrientFrame;
}

const SwRect* SwToContentAnchoredObjectPosition::ToCharRect() const
{
    return mpToCharRect;
}

// #i22341#
SwTwips SwToContentAnchoredObjectPosition::ToCharTopOfLine() const
{
    return mnToCharTopOfLine;
}

SwTextFrame& SwToContentAnchoredObjectPosition::GetAnchorTextFrame() const
{
    OSL_ENSURE( dynamic_cast<const SwTextFrame*>( &GetAnchorFrame()) != nullptr ,
            "SwToContentAnchoredObjectPosition::GetAnchorTextFrame() - wrong anchor frame type" );

    return static_cast<SwTextFrame&>(GetAnchorFrame());
}

// #i23512#
static bool lcl_DoesVertPosFits( const SwTwips _nRelPosY,
                          const SwTwips _nAvail,
                          const SwLayoutFrame* _pUpperOfOrientFrame,
                          const bool _bBrowse,
                          const bool _bGrowInTable,
                          SwLayoutFrame*& _orpLayoutFrameToGrow )
{
    bool bVertPosFits = false;

    if ( _nRelPosY <= _nAvail )
    {
        bVertPosFits = true;
    }
    else if ( _bBrowse )
    {
        if ( _pUpperOfOrientFrame->IsInSct() )
        {
            SwSectionFrame* pSctFrame =
                    const_cast<SwSectionFrame*>(_pUpperOfOrientFrame->FindSctFrame());
            bVertPosFits = pSctFrame->GetUpper()->Grow( _nRelPosY - _nAvail, true ) > 0;
            // Note: do not provide a layout frame for a grow.
        }
        else
        {
            bVertPosFits = const_cast<SwLayoutFrame*>(_pUpperOfOrientFrame)->
                                        Grow( _nRelPosY - _nAvail, true ) > 0;
            if ( bVertPosFits )
                _orpLayoutFrameToGrow = const_cast<SwLayoutFrame*>(_pUpperOfOrientFrame);
        }
    }
    else if ( _pUpperOfOrientFrame->IsInTab() && _bGrowInTable )
    {
        // #i45085# - check, if upper frame would grow the
        // excepted amount of twips.
        const SwTwips nTwipsGrown = const_cast<SwLayoutFrame*>(_pUpperOfOrientFrame)->
                                        Grow( _nRelPosY - _nAvail, true );
        bVertPosFits = ( nTwipsGrown == ( _nRelPosY - _nAvail ) );
        if ( bVertPosFits )
            _orpLayoutFrameToGrow = const_cast<SwLayoutFrame*>(_pUpperOfOrientFrame);
    }

    return bVertPosFits;
}

void SwToContentAnchoredObjectPosition::CalcPosition()
{
    // get format of object
    const SwFrameFormat& rFrameFormat = GetFrameFormat();

    // declare and set <pFooter> to footer frame, if object is anchored
    // at a frame belonging to the footer.
    const SwFrame* pFooter = GetAnchorFrame().FindFooterOrHeader();
    if ( pFooter && !pFooter->IsFooterFrame() )
        pFooter = nullptr;

    // declare and set <bBrowse> to true, if document is in browser mode and
    // object is anchored at the body, but not at frame belonging to a table.
    bool bBrowse = GetAnchorFrame().IsInDocBody() && !GetAnchorFrame().IsInTab();
    if( bBrowse )
    {
        const SwViewShell *pSh = GetAnchorFrame().getRootFrame()->GetCurrShell();
        if( !pSh || !pSh->GetViewOptions()->getBrowseMode() )
            bBrowse = false;
    }

    // determine left/right and its upper/lower spacing.
    const SvxLRSpaceItem &rLR = rFrameFormat.GetLRSpace();
    const SvxULSpaceItem &rUL = rFrameFormat.GetULSpace();

    // determine, if object has no surrounding.
    const SwFormatSurround& rSurround = rFrameFormat.GetSurround();
    const bool bNoSurround = rSurround.GetSurround() == SURROUND_NONE;
    const bool bWrapThrough = rSurround.GetSurround() == SURROUND_THROUGHT;

    // new class <SwEnvironmentOfAnchoredObject>
    SwEnvironmentOfAnchoredObject aEnvOfObj( DoesObjFollowsTextFlow() );

    // #i18732# - grow only, if object has to follow the text flow
    const bool bGrow = DoesObjFollowsTextFlow() &&
                       ( !GetAnchorFrame().IsInTab() ||
                         !rFrameFormat.GetFrameSize().GetHeightPercent() );

    // get text frame the object is anchored at
    const SwTextFrame& rAnchorTextFrame = GetAnchorTextFrame();
    SWRECTFN( (&rAnchorTextFrame) )

    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );

    // local variable keeping the calculated relative position; initialized with
    // current relative position.
    // #i26791# - use new object instance of <SwAnchoredObject>
    Point aRelPos( GetAnchoredObj().GetCurrRelPos() );

    SwTwips nRelDiff = 0;

    bool bMoveable = rAnchorTextFrame.IsMoveable();

    // determine frame the object position has to be oriented at.
    const SwTextFrame* pOrientFrame = &rAnchorTextFrame;
    const SwTextFrame* pAnchorFrameForVertPos = &rAnchorTextFrame;
    {
        // if object is at-character anchored, determine character-rectangle
        // and frame, position has to be oriented at.
        mbAnchorToChar = (FLY_AT_CHAR == rFrameFormat.GetAnchor().GetAnchorId());
        if ( mbAnchorToChar )
        {
            const SwFormatAnchor& rAnch = rFrameFormat.GetAnchor();
            // #i26791# - use new object instance of <SwAnchoredObject>
            // Due to table break algorithm the character
            // rectangle can have no height. Thus, check also the width
            if ( ( !GetAnchoredObj().GetLastCharRect().Height() &&
                   !GetAnchoredObj().GetLastCharRect().Width() ) ||
                 !GetAnchoredObj().GetLastTopOfLine() )
            {
                GetAnchoredObj().CheckCharRectAndTopOfLine( false );
                // Due to table break algorithm the character
                // rectangle can have no height. Thus, check also the width
                if ( ( !GetAnchoredObj().GetLastCharRect().Height() &&
                       !GetAnchoredObj().GetLastCharRect().Width() ) ||
                     !GetAnchoredObj().GetLastTopOfLine() )
                {
                    // Get default for <mpVertPosOrientFrame>, if it's not set.
                    if ( !mpVertPosOrientFrame )
                    {
                        mpVertPosOrientFrame = rAnchorTextFrame.GetUpper();
                    }
                    return;
                }
            }
            mpToCharRect = &(GetAnchoredObj().GetLastCharRect());
            // #i22341# - get top of line, in which the anchor character is.
            mnToCharTopOfLine = GetAnchoredObj().GetLastTopOfLine();
            pOrientFrame = &(const_cast<SwTextFrame&>(rAnchorTextFrame).GetFrameAtOfst(
                                rAnch.GetContentAnchor()->nContent.GetIndex() ) );
            mpToCharOrientFrame = pOrientFrame;
        }
    }
    SWREFRESHFN( pOrientFrame )

    // determine vertical position
    {

        // determine vertical positioning and alignment attributes
        SwFormatVertOrient aVert( rFrameFormat.GetVertOrient() );

        // #i18732# - determine layout frame for vertical
        // positions aligned to 'page areas'.
        const SwLayoutFrame& rPageAlignLayFrame =
                aEnvOfObj.GetVertEnvironmentLayoutFrame( *pOrientFrame );

        if ( aVert.GetVertOrient() != text::VertOrientation::NONE )
        {
            // #i18732# - adjustments for follow text flow or not
            // AND vertical alignment at 'page areas'.
            SwTwips nAlignAreaHeight;
            SwTwips nAlignAreaOffset;
            _GetVertAlignmentValues( *pOrientFrame, rPageAlignLayFrame,
                                     aVert.GetRelationOrient(),
                                     nAlignAreaHeight, nAlignAreaOffset );

            // determine relative vertical position
            SwTwips nRelPosY = nAlignAreaOffset;
            const SwTwips nObjHeight = (aObjBoundRect.*fnRect->fnGetHeight)();
            const SwTwips nUpperSpace = bVert
                                        ? ( bVertL2R
                                            ? rLR.GetLeft()
                                            : rLR.GetRight() )
                                        : rUL.GetUpper();
            // --> OD 2009-08-31 #monglianlayout#
            const SwTwips nLowerSpace = bVert
                                        ? ( bVertL2R
                                            ? rLR.GetLeft()
                                            : rLR.GetRight() )
                                        : rUL.GetLower();
            switch ( aVert.GetVertOrient() )
            {
                case text::VertOrientation::CHAR_BOTTOM:
                {
                    if ( mbAnchorToChar )
                    {
                        // bottom (to character anchored)
                        nRelPosY += nAlignAreaHeight + nUpperSpace;
                        if ( bVert && !bVertL2R )
                        {
                            nRelPosY += nObjHeight;
                        }
                        break;
                    }
                    SAL_FALLTHROUGH;
                }
                case text::VertOrientation::TOP:
                {
                    // #i22341# - special case for vertical
                    // alignment at top of line
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                    {
                        nRelPosY -= (nObjHeight + nLowerSpace);
                    }
                    else
                    {
                        nRelPosY += nUpperSpace;
                    }
                }
                break;
                // #i22341#
                case text::VertOrientation::LINE_TOP:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                    {
                        nRelPosY -= (nObjHeight + nLowerSpace);
                    }
                    else
                    {
                        OSL_FAIL( "<SwToContentAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
                case text::VertOrientation::CENTER:
                {
                    nRelPosY += (nAlignAreaHeight / 2) - (nObjHeight / 2);
                }
                break;
                // #i22341#
                case text::VertOrientation::LINE_CENTER:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                    {
                        nRelPosY += (nAlignAreaHeight / 2) - (nObjHeight / 2);
                    }
                    else
                    {
                        OSL_FAIL( "<SwToContentAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
                case text::VertOrientation::BOTTOM:
                {
                    if ( ( aVert.GetRelationOrient() == text::RelOrientation::FRAME ||
                           aVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA ) &&
                         bNoSurround )
                    {
                        // bottom (aligned to 'paragraph areas')
                        nRelPosY += nAlignAreaHeight + nUpperSpace;
                    }
                    else
                    {
                        // #i22341# - special case for vertical
                        // alignment at top of line
                        if ( mbAnchorToChar &&
                             aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                        {
                            nRelPosY += nUpperSpace;
                        }
                        else
                        {
                            nRelPosY += nAlignAreaHeight -
                                        ( nObjHeight + nLowerSpace );
                        }
                    }
                }
                break;
                // #i22341#
                case text::VertOrientation::LINE_BOTTOM:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE )
                    {
                        nRelPosY += nUpperSpace;
                    }
                    else
                    {
                        OSL_FAIL( "<SwToContentAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
                default:
                break;
            }

            // adjust relative position by distance between anchor frame and
            // the frame, the object is oriented at.
            // #i28701# - correction: adjust relative position,
            // only if the floating screen object has to follow the text flow.
            if ( DoesObjFollowsTextFlow() && pOrientFrame != &rAnchorTextFrame )
            {
                // #i11860# - use new method <_GetTopForObjPos>
                // to get top of frame for object positioning.
                const SwTwips nTopOfOrient = _GetTopForObjPos( *pOrientFrame, fnRect, bVert );
                nRelPosY += (*fnRect->fnYDiff)( nTopOfOrient,
                                      _GetTopForObjPos( rAnchorTextFrame, fnRect, bVert ) );
            }

            // #i42124# - capture object inside vertical
            // layout environment.
            {
                const SwTwips nTopOfAnch =
                                _GetTopForObjPos( *pOrientFrame, fnRect, bVert );
                const SwLayoutFrame& rVertEnvironLayFrame =
                    aEnvOfObj.GetVertEnvironmentLayoutFrame(
                                            *(pOrientFrame->GetUpper()) );
                const bool bCheckBottom = !DoesObjFollowsTextFlow();
                nRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                              rVertEnvironLayFrame, nRelPosY,
                                              DoesObjFollowsTextFlow(),
                                              bCheckBottom );
            }

            // keep calculated relative vertical position - needed for filters
            // (including the xml-filter)
            {
                // determine position
                SwTwips nAttrRelPosY = nRelPosY - nAlignAreaOffset;
                // set
                if ( nAttrRelPosY != aVert.GetPos() )
                {
                    aVert.SetPos( nAttrRelPosY );
                    const_cast<SwFrameFormat&>(rFrameFormat).LockModify();
                    const_cast<SwFrameFormat&>(rFrameFormat).SetFormatAttr( aVert );
                    const_cast<SwFrameFormat&>(rFrameFormat).UnlockModify();
                }
            }

            // determine absolute 'vertical' position, depending on layout-direction
            // #i26791# - determine offset to 'vertical' frame
            // anchor position, depending on layout-direction
            if ( bVert )
            {
                aRelPos.X() = nRelPosY;
                maOffsetToFrameAnchorPos.X() = nAlignAreaOffset;
            }
            else
            {
                aRelPos.Y() = nRelPosY;
                maOffsetToFrameAnchorPos.Y() = nAlignAreaOffset;
            }
        }

        // Determine upper of frame vertical position is oriented at.
        // #i28701# - determine 'virtual' anchor frame.
        // This frame is used in the following instead of the 'real' anchor
        // frame <rAnchorTextFrame> for the 'vertical' position in all cases.
        const SwLayoutFrame* pUpperOfOrientFrame = nullptr;
        {
            // #i28701# - As long as the anchor frame is on the
            // same page as <pOrientFrame> and the vertical position isn't aligned
            // automatic at the anchor character or the top of the line of the
            // anchor character, the anchor frame determines the vertical position.
            if ( &rAnchorTextFrame == pOrientFrame ||
                 ( rAnchorTextFrame.FindPageFrame() == pOrientFrame->FindPageFrame() &&
                   aVert.GetVertOrient() == text::VertOrientation::NONE &&
                   aVert.GetRelationOrient() != text::RelOrientation::CHAR &&
                   aVert.GetRelationOrient() != text::RelOrientation::TEXT_LINE ) )
            {
                pUpperOfOrientFrame = rAnchorTextFrame.GetUpper();
                pAnchorFrameForVertPos = &rAnchorTextFrame;
            }
            else
            {
                pUpperOfOrientFrame = pOrientFrame->GetUpper();
                pAnchorFrameForVertPos = pOrientFrame;
            }
        }

        // ignore one-column sections.
        // #i23512# - correction: also ignore one-columned
        // sections with footnotes/endnotes
        if ( pUpperOfOrientFrame->IsInSct() )
        {
            const SwSectionFrame* pSctFrame = pUpperOfOrientFrame->FindSctFrame();
            const bool bIgnoreSection = pUpperOfOrientFrame->IsSctFrame() ||
                                        ( pSctFrame->Lower()->IsColumnFrame() &&
                                          !pSctFrame->Lower()->GetNext() );
            if ( bIgnoreSection )
                pUpperOfOrientFrame = pSctFrame->GetUpper();
        }

        if ( aVert.GetVertOrient() == text::VertOrientation::NONE )
        {
            // local variable <nRelPosY> for calculation of relative vertical
            // distance to anchor.
            SwTwips nRelPosY = 0;
            // #i26791# - local variable <nVertOffsetToFrameAnchorPos>
            // for determination of the 'vertical' offset to the frame anchor
            // position
            SwTwips nVertOffsetToFrameAnchorPos( 0L );
            // #i22341# - add special case for vertical alignment
            // at top of line.
            if ( mbAnchorToChar &&
                 ( aVert.GetRelationOrient() == text::RelOrientation::CHAR ||
                   aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE ) )
            {
                // #i11860# - use new method <_GetTopForObjPos>
                // to get top of frame for object positioning.
                SwTwips nTopOfOrient = _GetTopForObjPos( *pOrientFrame, fnRect, bVert );
                if ( aVert.GetRelationOrient() == text::RelOrientation::CHAR )
                {
                    nVertOffsetToFrameAnchorPos = (*fnRect->fnYDiff)(
                                        (ToCharRect()->*fnRect->fnGetBottom)(),
                                        nTopOfOrient );
                }
                else
                {
                    nVertOffsetToFrameAnchorPos = (*fnRect->fnYDiff)( ToCharTopOfLine(),
                                                                    nTopOfOrient );
                }
                nRelPosY = nVertOffsetToFrameAnchorPos - aVert.GetPos();
            }
            else
            {
                // #i28701# - correction: use <pAnchorFrameForVertPos>
                // instead of <pOrientFrame> and do not adjust relative position
                // to get correct vertical position.
                nVertOffsetToFrameAnchorPos = 0L;
                // #i11860# - use new method <_GetTopForObjPos>
                // to get top of frame for object positioning.
                const SwTwips nTopOfOrient =
                        _GetTopForObjPos( *pAnchorFrameForVertPos, fnRect, bVert );
                // Increase <nRelPosY> by margin height,
                // if position is vertical aligned to "paragraph text area"
                if ( aVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA )
                {
                    // #i11860# - consider upper space amount of previous frame
                    SwTwips nTopMargin = (pAnchorFrameForVertPos->*fnRect->fnGetTopMargin)();
                    if ( pAnchorFrameForVertPos->IsTextFrame() )
                    {
                        nTopMargin -= static_cast<const SwTextFrame*>(pAnchorFrameForVertPos)->
                            GetUpperSpaceAmountConsideredForPrevFrameAndPageGrid();
                    }
                    nVertOffsetToFrameAnchorPos += nTopMargin;
                }
                // #i18732# - adjust <nRelPosY> by difference
                // between 'page area' and 'anchor' frame, if position is
                // vertical aligned to 'page areas'
                else if ( aVert.GetRelationOrient() == text::RelOrientation::PAGE_FRAME )
                {
                    nVertOffsetToFrameAnchorPos += (*fnRect->fnYDiff)(
                                    (rPageAlignLayFrame.Frame().*fnRect->fnGetTop)(),
                                    nTopOfOrient );
                }
                else if ( aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    SwRect aPgPrtRect( rPageAlignLayFrame.Frame() );
                    if ( rPageAlignLayFrame.IsPageFrame() )
                    {
                        aPgPrtRect =
                            static_cast<const SwPageFrame&>(rPageAlignLayFrame).PrtWithoutHeaderAndFooter();
                    }
                    nVertOffsetToFrameAnchorPos += (*fnRect->fnYDiff)(
                                                (aPgPrtRect.*fnRect->fnGetTop)(),
                                                nTopOfOrient );
                }
                nRelPosY = nVertOffsetToFrameAnchorPos + aVert.GetPos();
            }

            // <pUpperOfOrientFrame>: layout frame, at which the position has to
            //                      is oriented at
            // <nRelPosY>:          rest of the relative distance in the current
            //                      layout frame
            // <nAvail>:            space, which is available in the current
            //                      layout frame

            // #i26791# - determine offset to 'vertical'
            // frame anchor position, depending on layout-direction
            if ( bVert )
                maOffsetToFrameAnchorPos.X() = nVertOffsetToFrameAnchorPos;
            else
                maOffsetToFrameAnchorPos.Y() = nVertOffsetToFrameAnchorPos;
            // #i11860# - use new method <_GetTopForObjPos>
            // to get top of frame for object positioning.
            const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrameForVertPos, fnRect, bVert );
            if( nRelPosY <= 0 )
            {
                // Allow negative position, but keep it
                // inside environment layout frame.
                const SwLayoutFrame& rVertEnvironLayFrame =
                    aEnvOfObj.GetVertEnvironmentLayoutFrame( *pUpperOfOrientFrame );
                // #i31805# - do not check, if bottom of
                // anchored object would fit into environment layout frame, if
                // anchored object has to follow the text flow.
                const bool bCheckBottom = !DoesObjFollowsTextFlow();
                nRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                              rVertEnvironLayFrame, nRelPosY,
                                              DoesObjFollowsTextFlow(),
                                              bCheckBottom );
                if ( bVert )
                    aRelPos.X() = nRelPosY;
                else
                    aRelPos.Y() = nRelPosY;
            }
            else
            {
                SWREFRESHFN( pAnchorFrameForVertPos )
                SwTwips nAvail =
                    (*fnRect->fnYDiff)( (pUpperOfOrientFrame->*fnRect->fnGetPrtBottom)(),
                                        nTopOfAnch );
                const bool bInFootnote = pAnchorFrameForVertPos->IsInFootnote();
                while ( nRelPosY )
                {
                    // #i23512# - correction:
                    // consider section frame for grow in online layout.
                    // use new local method <lcl_DoesVertPosFits(..)>
                    SwLayoutFrame* pLayoutFrameToGrow = nullptr;
                    const bool bDoesVertPosFits = lcl_DoesVertPosFits(
                            nRelPosY, nAvail, pUpperOfOrientFrame, bBrowse,
                            bGrow, pLayoutFrameToGrow );

                    if ( bDoesVertPosFits )
                    {
                        SwTwips nTmpRelPosY =
                            (*fnRect->fnYDiff)( (pUpperOfOrientFrame->*fnRect->fnGetPrtBottom)(),
                                                nTopOfAnch ) -
                            nAvail + nRelPosY;
                        // #i28701# - adjust calculated
                        // relative vertical position to object's environment.
                        const SwFrame& rVertEnvironLayFrame =
                            aEnvOfObj.GetVertEnvironmentLayoutFrame( *pUpperOfOrientFrame );
                        // Do not check, if bottom of
                        // anchored object would fit into environment layout
                        // frame, if anchored object has to follow the text flow.
                        const bool bCheckBottom = !DoesObjFollowsTextFlow();
                        nTmpRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                                         rVertEnvironLayFrame,
                                                         nTmpRelPosY,
                                                         DoesObjFollowsTextFlow(),
                                                         bCheckBottom );
                        if ( bVert )
                            aRelPos.X() = nTmpRelPosY;
                        else
                            aRelPos.Y() = nTmpRelPosY;

                        // #i23512# - use local variable
                        // <pLayoutFrameToGrow> provided by new method
                        // <lcl_DoesVertPosFits(..)>.
                        if ( pLayoutFrameToGrow )
                        {
                            pLayoutFrameToGrow->Grow( nRelPosY - nAvail );
                        }
                        nRelPosY = 0;
                    }
                    else
                    {
                        // #i26495# - floating screen objects,
                        // which are anchored inside a table, doesn't follow
                        // the text flow.
                        if ( DoesObjFollowsTextFlow() &&
                             !( aVert.GetRelationOrient() == text::RelOrientation::PAGE_FRAME ||
                                aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA ) &&
                             !GetAnchorFrame().IsInTab() )
                        {
                            if ( bMoveable )
                            {
                                // follow the text flow
                                nRelPosY -= nAvail;
                                MakePageType eMakePage = bInFootnote ? MAKEPAGE_NONE
                                                                : MAKEPAGE_APPEND;
                                const bool bInSct = pUpperOfOrientFrame->IsInSct();
                                if( bInSct )
                                    eMakePage = MAKEPAGE_NOSECTION;

                                const SwLayoutFrame* pTmp =
                                    pUpperOfOrientFrame->GetLeaf( eMakePage, true, &rAnchorTextFrame );
                                if ( pTmp &&
                                     ( !bInSct ||
                                       pUpperOfOrientFrame->FindSctFrame()->IsAnFollow( pTmp->FindSctFrame() ) ) )
                                {
                                    pUpperOfOrientFrame = pTmp;
                                    bMoveable = rAnchorTextFrame.IsMoveable( pUpperOfOrientFrame );
                                    SWREFRESHFN( pUpperOfOrientFrame )
                                    nAvail = (pUpperOfOrientFrame->Prt().*fnRect->fnGetHeight)();
                                }
                                else
                                {
                                    // if there isn't enough space in the (columned)
                                    // section, leave it and set available space <nAvail>
                                    // to the space below the section.
                                    // if the new available space isn't also enough,
                                    // new pages can be created.
                                    if( bInSct )
                                    {
                                        const SwFrame* pSct = pUpperOfOrientFrame->FindSctFrame();
                                        pUpperOfOrientFrame = pSct->GetUpper();
                                        nAvail = (*fnRect->fnYDiff)(
                                                   (pUpperOfOrientFrame->*fnRect->fnGetPrtBottom)(),
                                                   (pSct->*fnRect->fnGetPrtBottom)() );
                                    }
                                    else
                                    {
#if OSL_DEBUG_LEVEL > 1
                                        OSL_FAIL( "<SwToContentAnchoredObjectPosition::CalcPosition()> - !bInSct" );
#endif
                                        nRelDiff = nRelPosY;
                                        nRelPosY = 0;
                                    }
                                }
                            }
                            else
                            {
                                nRelPosY = 0;
                            }
                        }
                        else
                        {
                            // #i18732# - do not follow text flow respectively
                            // align at 'page areas', but stay inside given environment
                            const SwFrame& rVertEnvironLayFrame =
                                aEnvOfObj.GetVertEnvironmentLayoutFrame( *pUpperOfOrientFrame );
                            nRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                                          rVertEnvironLayFrame,
                                                          nRelPosY,
                                                          DoesObjFollowsTextFlow() );
                            if( bVert )
                                aRelPos.X() = nRelPosY;
                            else
                                aRelPos.Y() = nRelPosY;
                            nRelPosY = 0;
                        }
                    }
                } // end of <while ( nRelPosY )>
            } // end of else <nRelPosY <= 0>
        } // end of <aVert.GetVertOrient() == text::VertOrientation::NONE>

        // We need to calculate the part's absolute position, in order for
        // it to be put onto the right page and to be pulled into the
        // LayLeaf's PrtArea
        const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrameForVertPos, fnRect, bVert );
        if( bVert )
        {
            // --> OD 2009-08-31 #monglianlayout#
            if ( !bVertL2R )
            {
                GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                             ( aRelPos.X() - nRelDiff ) -
                                             aObjBoundRect.Width() );
            }
            else
            {
                GetAnchoredObj().SetObjLeft( nTopOfAnch +
                                             ( aRelPos.X() - nRelDiff ) );
            }
        }
        else
        {
            GetAnchoredObj().SetObjTop( nTopOfAnch +
                                        ( aRelPos.Y() - nRelDiff ) );
        }

        // grow environment under certain conditions
        // ignore one-column sections.
        // #i23512# - correction: also ignore one-columned
        // sections with footnotes/endnotes
        if ( pUpperOfOrientFrame->IsInSct() )
        {
            const SwSectionFrame* pSctFrame = pUpperOfOrientFrame->FindSctFrame();
            const bool bIgnoreSection = pUpperOfOrientFrame->IsSctFrame() ||
                                        ( pSctFrame->Lower()->IsColumnFrame() &&
                                          !pSctFrame->Lower()->GetNext() );
            if ( bIgnoreSection )
                pUpperOfOrientFrame = pSctFrame->GetUpper();
        }
        SwTwips nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                          (pUpperOfOrientFrame->*fnRect->fnGetPrtBottom)() );
        if( nDist < 0 )
        {
            // #i23512# - correction:
            // consider section frame for grow in online layout and
            // consider page alignment for grow in table.
            SwLayoutFrame* pLayoutFrameToGrow = nullptr;
            if ( bBrowse && rAnchorTextFrame.IsMoveable() )
            {
                if ( pUpperOfOrientFrame->IsInSct() )
                {
                    pLayoutFrameToGrow = const_cast<SwLayoutFrame*>(
                                    pUpperOfOrientFrame->FindSctFrame()->GetUpper());
                    nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                              (pLayoutFrameToGrow->*fnRect->fnGetPrtBottom)() );
                    if ( nDist >= 0 )
                    {
                        pLayoutFrameToGrow = nullptr;
                    }
                }
                else
                {
                    pLayoutFrameToGrow =
                                    const_cast<SwLayoutFrame*>(pUpperOfOrientFrame);
                }
            }
            else if ( rAnchorTextFrame.IsInTab() && bGrow )
            {
                pLayoutFrameToGrow = const_cast<SwLayoutFrame*>(pUpperOfOrientFrame);
            }
            if ( pLayoutFrameToGrow )
            {
                pLayoutFrameToGrow->Grow( -nDist );
            }
        }

        if ( DoesObjFollowsTextFlow() &&
             !( aVert.GetRelationOrient() == text::RelOrientation::PAGE_FRAME ||
                aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA ) )
        {

            nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                      (pUpperOfOrientFrame->*fnRect->fnGetPrtBottom)() );
            // #i26945# - floating screen objects, which are
            // anchored inside a table, doesn't follow the text flow. But, they
            // have to stay inside its layout environment.
            if ( nDist < 0 && pOrientFrame->IsInTab() )
            {
                // If the anchor frame is the first content of the table cell
                // and has no follow, the table frame is notified,
                // that the object doesn't fit into the table cell.
                // Adjustment of position isn't needed in this case.
                if ( pOrientFrame == &rAnchorTextFrame &&
                     !pOrientFrame->GetFollow() &&
                     !pOrientFrame->GetIndPrev() )
                {
                    const_cast<SwTabFrame*>(pOrientFrame->FindTabFrame())
                                                    ->SetDoesObjsFit( false );
                }
                else
                {
                    SwTwips nTmpRelPosY( 0L );
                    if ( bVert )
                        nTmpRelPosY = aRelPos.X() - nDist;
                    else
                        nTmpRelPosY = aRelPos.Y() + nDist;
                    const SwLayoutFrame& rVertEnvironLayFrame =
                        aEnvOfObj.GetVertEnvironmentLayoutFrame( *pUpperOfOrientFrame );
                    nTmpRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                                     rVertEnvironLayFrame,
                                                     nTmpRelPosY,
                                                     DoesObjFollowsTextFlow(),
                                                     false );
                    if ( bVert )
                    {
                        aRelPos.X() = nTmpRelPosY;
                        // --> OD 2009-08-31 #mongolianlayout#
                        if ( !bVertL2R )
                        {
                            GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                                         aRelPos.X() -
                                                         aObjBoundRect.Width() );
                        }
                        else
                        {
                            GetAnchoredObj().SetObjLeft( nTopOfAnch + aRelPos.X() );
                        }
                    }
                    else
                    {
                        aRelPos.Y() = nTmpRelPosY;
                        GetAnchoredObj().SetObjTop( nTopOfAnch + aRelPos.Y() );
                    }
                    // If the anchor frame is the first content of the table cell
                    // and the object still doesn't fit, the table frame is notified,
                    // that the object doesn't fit into the table cell.
                    nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                              (pUpperOfOrientFrame->*fnRect->fnGetPrtBottom)() );
                    if ( nDist < 0 &&
                         pOrientFrame == &rAnchorTextFrame && !pOrientFrame->GetIndPrev() )
                    {
                        const_cast<SwTabFrame*>(pOrientFrame->FindTabFrame())
                                                        ->SetDoesObjsFit( false );
                    }
                }
            }
            else
            {
                // follow text flow
                const bool bInFootnote = rAnchorTextFrame.IsInFootnote();
                while( bMoveable && nDist < 0 )
                {
                    bool bInSct = pUpperOfOrientFrame->IsInSct();
                    if ( bInSct )
                    {
                        const SwLayoutFrame* pTmp = pUpperOfOrientFrame->FindSctFrame()->GetUpper();
                        nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                  (pTmp->*fnRect->fnGetPrtBottom)() );
                        // #i23129# - Try to flow into next
                        // section|section column. Thus, do *not* leave section
                        // area, if anchored object doesn't fit into upper of section.
                        // But the anchored object is allowed to overlap bottom
                        // section|section column.
                        if ( nDist >= 0 )
                        {
                            break;
                        }
                    }
                    if ( !bInSct &&
                         (GetAnchoredObj().GetObjRect().*fnRect->fnGetTop)() ==
                                 (pUpperOfOrientFrame->*fnRect->fnGetPrtTop)() )
                        // It doesn't fit, moving it would not help either anymore
                        break;

                    const SwLayoutFrame* pNextLay = pUpperOfOrientFrame->GetLeaf(
                                    ( bInSct
                                      ? MAKEPAGE_NOSECTION
                                      : ( bInFootnote ? MAKEPAGE_NONE : MAKEPAGE_APPEND ) ),
                                    true, &rAnchorTextFrame );
                    // correction:
                    // If anchor is in footnote and proposed next layout environment
                    // isn't a footnote frame, object can't follow the text flow
                    if ( bInFootnote && pNextLay && !pNextLay->IsFootnoteFrame() )
                    {
                        pNextLay = nullptr;
                    }
                    if ( pNextLay )
                    {
                        SWRECTFNX( pNextLay )
                        if ( !bInSct ||
                             ( pUpperOfOrientFrame->FindSctFrame()->IsAnFollow( pNextLay->FindSctFrame() ) &&
                               (pNextLay->Prt().*fnRectX->fnGetHeight)() ) )
                        {
                            SwTwips nTmpRelPosY =
                                (*fnRect->fnYDiff)( (pNextLay->*fnRect->fnGetPrtTop)(),
                                                    nTopOfAnch );
                            if ( bVert )
                                aRelPos.X() = nTmpRelPosY;
                            else
                                aRelPos.Y() = nTmpRelPosY;
                            pUpperOfOrientFrame = pNextLay;
                            SWREFRESHFN( pUpperOfOrientFrame )
                            bMoveable = rAnchorTextFrame.IsMoveable( pUpperOfOrientFrame );
                            if( bVertX )
                            {
                                // --> OD 2009-08-31 #mongolianlayout#
                                if ( !bVertL2R )
                                {
                                    GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                                                 aRelPos.X() -
                                                                 aObjBoundRect.Width() );
                                }
                                else
                                {
                                    GetAnchoredObj().SetObjLeft( nTopOfAnch +
                                                                 aRelPos.X() );
                                }
                            }
                            else
                                GetAnchoredObj().SetObjTop( nTopOfAnch +
                                                            aRelPos.Y() );
                            nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                      (pUpperOfOrientFrame->*fnRect->fnGetPrtBottom)() );
                        }
                        // #i23129# - leave section area
                        else if ( bInSct )
                        {
                            const SwLayoutFrame* pTmp = pUpperOfOrientFrame->FindSctFrame()->GetUpper();
                            nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                      (pTmp->*fnRect->fnGetPrtBottom)() );
                            if( nDist < 0 )
                                pUpperOfOrientFrame = pTmp;
                            else
                                break;
                        }
                    }
                    else if ( bInSct )
                    {
                        // If we don't have enough room within the Area, we take a look at
                        // the Page
                        const SwLayoutFrame* pTmp = pUpperOfOrientFrame->FindSctFrame()->GetUpper();
                        nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                  (pTmp->*fnRect->fnGetPrtBottom)() );
                        if( nDist < 0 )
                            pUpperOfOrientFrame = pTmp;
                        else
                            break;
                    }
                    else
                        bMoveable = false;
                }
            }
        }

        // keep layout frame vertical position is oriented at.
        mpVertPosOrientFrame = pUpperOfOrientFrame;

    }

    // determine 'horizontal' position
    {
        // determine horizontal positioning and alignment attributes
        SwFormatHoriOrient aHori( rFrameFormat.GetHoriOrient() );

        // set calculated vertical position in order to determine correct
        // frame, the horizontal position is oriented at.
        const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrameForVertPos, fnRect, bVert );
        if( bVert )
        {
            // --> OD 2009-08-31 #mongolianlayout#
            if ( !bVertL2R )
            {
                GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                             aRelPos.X() - aObjBoundRect.Width() );
            }
            else
            {
                GetAnchoredObj().SetObjLeft( nTopOfAnch + aRelPos.X() );
            }
        }
        else
            GetAnchoredObj().SetObjTop( nTopOfAnch + aRelPos.Y() );

        // determine frame, horizontal position is oriented at.
        // #i28701# - If floating screen object doesn't follow
        // the text flow, its horizontal position is oriented at <pOrientFrame>.
        const SwFrame* pHoriOrientFrame = DoesObjFollowsTextFlow()
                                      ? &_GetHoriVirtualAnchor( *mpVertPosOrientFrame )
                                      : pOrientFrame;

        // #i26791# - get 'horizontal' offset to frame anchor position.
        SwTwips nHoriOffsetToFrameAnchorPos( 0L );
        SwTwips nRelPosX = _CalcRelPosX( *pHoriOrientFrame, aEnvOfObj,
                                         aHori, rLR, rUL, bWrapThrough,
                                         ( bVert ? aRelPos.X() : aRelPos.Y() ),
                                         nHoriOffsetToFrameAnchorPos );

        // #i26791# - determine offset to 'horizontal' frame
        // anchor position, depending on layout-direction
        if ( bVert )
        {
            aRelPos.Y() = nRelPosX;
            maOffsetToFrameAnchorPos.Y() = nHoriOffsetToFrameAnchorPos;
        }
        else
        {
            aRelPos.X() = nRelPosX;
            maOffsetToFrameAnchorPos.X() = nHoriOffsetToFrameAnchorPos;
        }

        // save calculated horizontal position - needed for filters
        // (including the xml-filter)
        {
            SwTwips nAttrRelPosX = nRelPosX - nHoriOffsetToFrameAnchorPos;
            if ( aHori.GetHoriOrient() != text::HoriOrientation::NONE &&
                 aHori.GetPos() != nAttrRelPosX )
            {
                aHori.SetPos( nAttrRelPosX );
                const_cast<SwFrameFormat&>(rFrameFormat).LockModify();
                const_cast<SwFrameFormat&>(rFrameFormat).SetFormatAttr( aHori );
                const_cast<SwFrameFormat&>(rFrameFormat).UnlockModify();
            }
        }
    }

    // set absolute position at object
    const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrameForVertPos, fnRect, bVert );
    if( bVert )
    {
        // --> OD 2009-08-31 #mongolianlayout#
        if ( !bVertL2R )
        {
            GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                         aRelPos.X() - aObjBoundRect.Width() );
        }
        else
        {
            GetAnchoredObj().SetObjLeft( nTopOfAnch + aRelPos.X() );
        }
        GetAnchoredObj().SetObjTop( rAnchorTextFrame.Frame().Top() +
                                    aRelPos.Y() );
    }
    else
    {
        GetAnchoredObj().SetObjLeft( rAnchorTextFrame.Frame().Left() +
                                     aRelPos.X() );
        GetAnchoredObj().SetObjTop( nTopOfAnch + aRelPos.Y() );
    }

    // set relative position at object
    GetAnchoredObj().SetCurrRelPos( aRelPos );
}

/**
 * Determine frame for horizontal position
 */
const SwFrame& SwToContentAnchoredObjectPosition::_GetHoriVirtualAnchor(
                                        const SwLayoutFrame& _rProposedFrame ) const
{
    const SwFrame* pHoriVirtAnchFrame = &_rProposedFrame;

    // Search for first lower content frame, which is the anchor or a follow
    // of the anchor (Note: <Anchor.IsAnFollow( Anchor )> is true)
    // If none found, <_rProposedFrame> is returned.
    const SwFrame* pFrame = _rProposedFrame.Lower();
    while ( pFrame )
    {
        if ( pFrame->IsContentFrame() &&
             GetAnchorTextFrame().IsAnFollow( static_cast<const SwContentFrame*>(pFrame) ) )
        {
            pHoriVirtAnchFrame = pFrame;
            break;
        }
        pFrame = pFrame->GetNext();
    }

    return *pHoriVirtAnchFrame;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
