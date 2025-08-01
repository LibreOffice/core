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
#include <rootfrm.hxx>
#include <viewopt.hxx>
#include <viewsh.hxx>
#include <frmfmt.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <IDocumentSettingAccess.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svx/svdobj.hxx>
#include <osl/diagnose.h>
#include <environmentofanchoredobject.hxx>
#include <frmatr.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <rowfrm.hxx>
#include <sortedobjs.hxx>
#include <textboxhelper.hxx>
#include <flyfrms.hxx>

using namespace ::com::sun::star;

namespace objectpositioning
{
SwToContentAnchoredObjectPosition::SwToContentAnchoredObjectPosition( SdrObject& _rDrawObj )
    : SwAnchoredObjectPosition ( _rDrawObj ),
      mpVertPosOrientFrame( nullptr ),
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
    assert( dynamic_cast<const SwTextFrame*>( &GetAnchorFrame()) &&
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
        // expected amount of twips.
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
    const bool bNoSurround = rSurround.GetSurround() == css::text::WrapTextMode_NONE;
    const bool bWrapThrough = rSurround.GetSurround() == css::text::WrapTextMode_THROUGH;

    // new class <SwEnvironmentOfAnchoredObject>
    SwEnvironmentOfAnchoredObject aEnvOfObj( DoesObjFollowsTextFlow() );

    // #i18732# - grow only, if object has to follow the text flow
    const bool bGrow = DoesObjFollowsTextFlow() &&
                       ( !GetAnchorFrame().IsInTab() ||
                         !rFrameFormat.GetFrameSize().GetHeightPercent() );

    // get text frame the object is anchored at
    const SwTextFrame& rAnchorTextFrame = GetAnchorTextFrame();
    SwRectFnSet aRectFnSet(&rAnchorTextFrame);

    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );

    // local variable keeping the calculated relative position; initialized with
    // current relative position.
    // #i26791# - use new object instance of <SwAnchoredObject>
    Point aRelPos( GetAnchoredObj().GetCurrRelPos() );

    SwTwips nRelDiff = 0;

    bool bMoveable = rAnchorTextFrame.IsMoveable();

    // determine frame the object position has to be oriented at.
    const SwTextFrame* pOrientFrame = &rAnchorTextFrame;
    const SwTextFrame* pAnchorFrameForVertPos;
    // If true, this means that the anchored object is a split fly frame and it's not a master but
    // one of the follows.
    bool bFollowSplitFly = false;
    // The anchored object is a fly that is allowed to split.
    bool bSplitFly = false;
    {
        // if object is at-character anchored, determine character-rectangle
        // and frame, position has to be oriented at.
        mbAnchorToChar = (RndStdIds::FLY_AT_CHAR == rFrameFormat.GetAnchor().GetAnchorId());
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
                rAnchorTextFrame.MapModelToViewPos(*rAnch.GetContentAnchor())));
            mpToCharOrientFrame = pOrientFrame;
        }
        else if (SwFlyFrame* pFlyFrame = GetAnchoredObj().DynCastFlyFrame())
        {
            // See if this fly is split. If so, then the anchor is also split. All anchors are
            // empty, except the last follow.
            if (pFlyFrame->IsFlySplitAllowed())
            {
                auto pFlyAtContentFrame = static_cast<SwFlyAtContentFrame*>(pFlyFrame);
                // Decrement pFly to point to the master; increment pAnchor to point to the correct
                // follow anchor.
                SwFlyAtContentFrame* pFly = pFlyAtContentFrame;
                SwTextFrame* pAnchor = const_cast<SwTextFrame*>(&rAnchorTextFrame);
                while (pFly->GetPrecede())
                {
                    pFly = pFly->GetPrecede();
                    if (!pAnchor)
                    {
                        SAL_WARN("sw.core", "SwToContentAnchoredObjectPosition::CalcPosition: fly "
                                            "chain length is longer then anchor chain length");
                        break;
                    }
                    pAnchor = pAnchor->GetFollow();
                }
                if (pAnchor && pAnchor->GetPrecede())
                {
                    pOrientFrame = pAnchor;
                    // Anchored object has a precede, so it's a follow.
                    bFollowSplitFly = true;
                }
                bSplitFly = true;
            }
        }
    }
    aRectFnSet.Refresh(pOrientFrame);

    // Microsoft allows WrapThrough shapes to be placed outside of the cell despite layoutInCell
    // (Re-use existing compat flag to identify MSO formats. The name also matches this purpose.)
    const bool bMSOLayout = rFrameFormat.getIDocumentSettingAccess().get(
        DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION);
    const bool bMSOLayoutInCell
        = bMSOLayout && DoesObjFollowsTextFlow() && GetAnchorTextFrame().IsInTab();
    const bool bIgnoreVertLayoutInCell = bMSOLayoutInCell && bWrapThrough;

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
            GetVertAlignmentValues( *pOrientFrame, rPageAlignLayFrame,
                                     aVert.GetRelationOrient(),
                                     nAlignAreaHeight, nAlignAreaOffset );

            SwRect aHeaderRect;
            const SwPageFrame* aPageFrame = pOrientFrame->FindPageFrame();
            const SwHeaderFrame* pHeaderFrame = aPageFrame->GetHeaderFrame();
            if (pHeaderFrame)
                aHeaderRect = pHeaderFrame->GetPaintArea();
            const SwTwips nTopMarginHeight = aPageFrame->GetTopMargin() + aHeaderRect.Height();
            const SwTwips nHeightBetweenOffsetAndMargin = nAlignAreaOffset + nTopMarginHeight;

            // determine relative vertical position
            SwTwips nRelPosY = nAlignAreaOffset;
            const SwTwips nObjHeight = aRectFnSet.GetHeight(aObjBoundRect);
            const SwTwips nUpperSpace
                = aRectFnSet.IsVert()
                      ? (aRectFnSet.IsVertL2R() ? rLR.ResolveLeft({}) : rLR.ResolveRight({}))
                      : rUL.GetUpper();
            // --> OD 2009-08-31 #monglianlayout#
            const SwTwips nLowerSpace
                = aRectFnSet.IsVert()
                      ? (aRectFnSet.IsVertL2R() ? rLR.ResolveLeft({}) : rLR.ResolveRight({}))
                      : rUL.GetLower();
            switch ( aVert.GetVertOrient() )
            {
                case text::VertOrientation::CHAR_BOTTOM:
                {
                    if ( mbAnchorToChar )
                    {
                        // bottom (to character anchored)
                        nRelPosY += nAlignAreaHeight + nUpperSpace;
                        if ( aRectFnSet.IsVert() && !aRectFnSet.IsVertL2R() )
                        {
                            nRelPosY += nObjHeight;
                        }
                        break;
                    }
                    [[fallthrough]];
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
                    if (aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA_TOP)
                        nRelPosY = (nAlignAreaOffset / 2) - (nObjHeight / 2) + (nHeightBetweenOffsetAndMargin / 2);
                    else
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
                            if (aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA_TOP)
                                nRelPosY = 0 - (nObjHeight + nLowerSpace) + nHeightBetweenOffsetAndMargin;
                            else
                                nRelPosY += nAlignAreaHeight - (nObjHeight + nLowerSpace);
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
            // Also don't do this for split flys: pOrientFrame already points to the follow anchor,
            // so pOrientFrame is not the anchor text frame anymore, and that would lead to an
            // additional, unwanted increase of nRelPosY.
            if (DoesObjFollowsTextFlow() && pOrientFrame != &rAnchorTextFrame && !bFollowSplitFly)
            {
                // #i11860# - use new method <GetTopForObjPos>
                // to get top of frame for object positioning.
                const SwTwips nTopOfOrient = GetTopForObjPos( *pOrientFrame, aRectFnSet.FnRect(), aRectFnSet.IsVert() );
                nRelPosY += aRectFnSet.YDiff( nTopOfOrient,
                                      GetTopForObjPos( rAnchorTextFrame, aRectFnSet.FnRect(), aRectFnSet.IsVert() ) );
            }

            // #i42124# - capture object inside vertical
            // layout environment.
            {
                const SwTwips nTopOfAnch =
                                GetTopForObjPos( *pOrientFrame, aRectFnSet.FnRect(), aRectFnSet.IsVert() );
                const SwLayoutFrame& rVertEnvironLayFrame =
                    aEnvOfObj.GetVertEnvironmentLayoutFrame(
                                            *(pOrientFrame->GetUpper()) );
                const bool bCheckBottom = !DoesObjFollowsTextFlow();
                nRelPosY = AdjustVertRelPos( nTopOfAnch, aRectFnSet.IsVert(), aRectFnSet.IsVertL2R(),
                                              rVertEnvironLayFrame, aVert, nRelPosY,
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
            if ( aRectFnSet.IsVert() )
            {
                aRelPos.setX( nRelPosY );
                maOffsetToFrameAnchorPos.setX( nAlignAreaOffset );
            }
            else
            {
                aRelPos.setY( nRelPosY );
                maOffsetToFrameAnchorPos.setY( nAlignAreaOffset );
            }
        }

        // Determine upper of frame vertical position is oriented at.
        // #i28701# - determine 'virtual' anchor frame.
        // This frame is used in the following instead of the 'real' anchor
        // frame <rAnchorTextFrame> for the 'vertical' position in all cases.
        const SwLayoutFrame* pUpperOfOrientFrame = nullptr;
        // #i28701# - As long as the anchor frame is on the
        // same page as <pOrientFrame> and the vertical position isn't aligned
        // automatic at the anchor character or the top of the line of the
        // anchor character, the anchor frame determines the vertical position.
        // Split fly follows: always let the anchor char frame determine the vertical position.
        // This gives us a vertical cut position between the master and the follow.
        if ( &rAnchorTextFrame == pOrientFrame ||
             ( rAnchorTextFrame.FindPageFrame() == pOrientFrame->FindPageFrame() &&
               aVert.GetVertOrient() == text::VertOrientation::NONE &&
               aVert.GetRelationOrient() != text::RelOrientation::CHAR &&
               aVert.GetRelationOrient() != text::RelOrientation::TEXT_LINE && !bFollowSplitFly ) )
        {
            pUpperOfOrientFrame = rAnchorTextFrame.GetUpper();
            pAnchorFrameForVertPos = &rAnchorTextFrame;
        }
        else
        {
            pUpperOfOrientFrame = pOrientFrame->GetUpper();
            pAnchorFrameForVertPos = pOrientFrame;
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
            SwTwips nVertOffsetToFrameAnchorPos( 0 );
            // #i22341# - add special case for vertical alignment
            // at top of line.
            if ( mbAnchorToChar &&
                 ( aVert.GetRelationOrient() == text::RelOrientation::CHAR ||
                   aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE ) )
            {
                // #i11860# - use new method <GetTopForObjPos>
                // to get top of frame for object positioning.
                SwTwips nTopOfOrient = GetTopForObjPos( *pOrientFrame, aRectFnSet.FnRect(), aRectFnSet.IsVert() );
                if ( aVert.GetRelationOrient() == text::RelOrientation::CHAR )
                {
                    nVertOffsetToFrameAnchorPos = aRectFnSet.YDiff(
                                        aRectFnSet.GetBottom(*ToCharRect()),
                                        nTopOfOrient );
                }
                else
                {
                    nVertOffsetToFrameAnchorPos = aRectFnSet.YDiff( ToCharTopOfLine(),
                                                                    nTopOfOrient );
                }
                nRelPosY = nVertOffsetToFrameAnchorPos - aVert.GetPos();
            }
            else
            {
                // #i28701# - correction: use <pAnchorFrameForVertPos>
                // instead of <pOrientFrame> and do not adjust relative position
                // to get correct vertical position.
                nVertOffsetToFrameAnchorPos = 0;
                // #i11860# - use new method <GetTopForObjPos>
                // to get top of frame for object positioning.
                const SwTwips nTopOfOrient =
                        GetTopForObjPos( *pAnchorFrameForVertPos, aRectFnSet.FnRect(), aRectFnSet.IsVert() );
                // Increase <nRelPosY> by margin height,
                // if position is vertical aligned to "paragraph text area"
                if ( aVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA )
                {
                    // #i11860# - consider upper space amount of previous frame
                    SwTwips nTopMargin = aRectFnSet.GetTopMargin(*pAnchorFrameForVertPos);
                    if ( pAnchorFrameForVertPos->IsTextFrame() )
                    {
                        nTopMargin -= pAnchorFrameForVertPos->
                            GetUpperSpaceAmountConsideredForPrevFrameAndPageGrid();
                    }
                    nVertOffsetToFrameAnchorPos += nTopMargin;
                }
                // #i18732# - adjust <nRelPosY> by difference
                // between 'page area' and 'anchor' frame, if position is
                // vertical aligned to 'page areas'
                else if (aVert.GetRelationOrient() == text::RelOrientation::PAGE_FRAME
                         || aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA_TOP)
                {
                    nVertOffsetToFrameAnchorPos += aRectFnSet.YDiff(
                                    aRectFnSet.GetTop(rPageAlignLayFrame.getFrameArea()),
                                    nTopOfOrient );
                }
                else if ( aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    SwRect aPgPrtRect( rPageAlignLayFrame.getFrameArea() );
                    if ( rPageAlignLayFrame.IsPageFrame() )
                    {
                        aPgPrtRect =
                            static_cast<const SwPageFrame&>(rPageAlignLayFrame).PrtWithoutHeaderAndFooter();
                    }
                    nVertOffsetToFrameAnchorPos += aRectFnSet.YDiff(
                                                aRectFnSet.GetTop(aPgPrtRect),
                                                nTopOfOrient );

                    if (rPageAlignLayFrame.IsCellFrame())
                    {
                        // Cell upper/lower comes from the max margin of the entire row of cells
                        const auto pRow = const_cast<SwLayoutFrame&>(rPageAlignLayFrame).FindRowFrame();
                        assert(pRow);
                        nVertOffsetToFrameAnchorPos += pRow->GetTopMarginForLowers();
                    }
                }
                else if (aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA_BOTTOM)
                {
                    // The anchored object is relative from the bottom of the page's print area.
                    SwRect aPgPrtRect(rPageAlignLayFrame.getFrameArea());
                    if (rPageAlignLayFrame.IsPageFrame())
                    {
                        auto& rPageFrame = static_cast<const SwPageFrame&>(rPageAlignLayFrame);
                        aPgPrtRect = rPageFrame.PrtWithoutHeaderAndFooter();
                    }
                    SwTwips nPageBottom = aRectFnSet.GetBottom(aPgPrtRect);
                    nVertOffsetToFrameAnchorPos += aRectFnSet.YDiff(nPageBottom, nTopOfOrient);
                }
                nRelPosY = nVertOffsetToFrameAnchorPos + aVert.GetPos();
                if (bFollowSplitFly)
                {
                    // This is a follow of a split fly: shift it up to match the anchor position,
                    // because the vertical offset is meant to be handled only on the first page.
                    nRelPosY -= aVert.GetPos();

                    if (aVert.GetRelationOrient() == text::RelOrientation::PAGE_FRAME
                        && rPageAlignLayFrame.IsPageFrame())
                    {
                        // Master is positioned relative to the edge of the page, with an offset.
                        // Follow will have no offset, but is relative to the bottom of the header.
                        auto& rPageFrame = static_cast<const SwPageFrame&>(rPageAlignLayFrame);
                        const SwLayoutFrame* pBodyFrame = rPageFrame.FindBodyCont();
                        if (pBodyFrame)
                        {
                            SwTwips nDiff = pBodyFrame->getFrameArea().Top()
                                            - rPageFrame.getFrameArea().Top();
                            nRelPosY += nDiff;
                        }
                    }
                }
            }

            // <pUpperOfOrientFrame>: layout frame, at which the position has to
            //                      is oriented at
            // <nRelPosY>:          rest of the relative distance in the current
            //                      layout frame
            // <nAvail>:            space, which is available in the current
            //                      layout frame

            // #i26791# - determine offset to 'vertical'
            // frame anchor position, depending on layout-direction
            if ( aRectFnSet.IsVert() )
                maOffsetToFrameAnchorPos.setX( nVertOffsetToFrameAnchorPos );
            else
                maOffsetToFrameAnchorPos.setY( nVertOffsetToFrameAnchorPos );
            // #i11860# - use new method <GetTopForObjPos>
            // to get top of frame for object positioning.
            const SwTwips nTopOfAnch = GetTopForObjPos( *pAnchorFrameForVertPos, aRectFnSet.FnRect(), aRectFnSet.IsVert() );
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
                nRelPosY = AdjustVertRelPos( nTopOfAnch, aRectFnSet.IsVert(), aRectFnSet.IsVertL2R(),
                                              rVertEnvironLayFrame, aVert, nRelPosY,
                                              !bIgnoreVertLayoutInCell && DoesObjFollowsTextFlow(),
                                              bCheckBottom );
                if ( aRectFnSet.IsVert() )
                    aRelPos.setX( nRelPosY );
                else
                    aRelPos.setY( nRelPosY );
            }
            else
            {
                aRectFnSet.Refresh(pAnchorFrameForVertPos);
                SwTwips nAvail =
                    aRectFnSet.YDiff( aRectFnSet.GetPrtBottom(*pUpperOfOrientFrame),
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
                            aRectFnSet.YDiff( aRectFnSet.GetPrtBottom(*pUpperOfOrientFrame),
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
                        nTmpRelPosY = AdjustVertRelPos( nTopOfAnch, aRectFnSet.IsVert(), aRectFnSet.IsVertL2R(),
                                                         rVertEnvironLayFrame, aVert,
                                                         nTmpRelPosY,
                                                         DoesObjFollowsTextFlow(),
                                                         bCheckBottom );
                        if ( aRectFnSet.IsVert() )
                            aRelPos.setX( nTmpRelPosY );
                        else
                            aRelPos.setY( nTmpRelPosY );

                        // #i23512# - use local variable
                        // <pLayoutFrameToGrow> provided by new method
                        // <lcl_DoesVertPosFits(..)>.
                        if ( pLayoutFrameToGrow )
                        {
                            // No need to grow the anchor cell in case the follow-text-flow object
                            // is wrap-though.
                            if (!GetAnchorFrame().IsInTab() || !DoesObjFollowsTextFlow() || !bWrapThrough)
                            {
                                pLayoutFrameToGrow->Grow( nRelPosY - nAvail );
                            }
                        }
                        nRelPosY = 0;
                    }
                    else
                    {
                        // #i26495# - floating screen objects,
                        // which are anchored inside a table, doesn't follow
                        // the text flow.
                        if ( DoesObjFollowsTextFlow() &&
                             ( aVert.GetRelationOrient() != text::RelOrientation::PAGE_FRAME &&
                                aVert.GetRelationOrient() != text::RelOrientation::PAGE_PRINT_AREA ) &&
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
                                    aRectFnSet.Refresh(pUpperOfOrientFrame);
                                    nAvail = aRectFnSet.GetHeight(pUpperOfOrientFrame->getFramePrintArea());
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
                                        nAvail = aRectFnSet.YDiff(
                                                   aRectFnSet.GetPrtBottom(*pUpperOfOrientFrame),
                                                   aRectFnSet.GetPrtBottom(*pSct) );
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
                            nRelPosY = AdjustVertRelPos( nTopOfAnch, aRectFnSet.IsVert(), aRectFnSet.IsVertL2R(),
                                                          rVertEnvironLayFrame, aVert,
                                                          nRelPosY,
                                                          DoesObjFollowsTextFlow() );
                            if( aRectFnSet.IsVert() )
                                aRelPos.setX( nRelPosY );
                            else
                                aRelPos.setY( nRelPosY );
                            nRelPosY = 0;
                        }
                    }
                } // end of <while ( nRelPosY )>
            } // end of else <nRelPosY <= 0>
        } // end of <aVert.GetVertOrient() == text::VertOrientation::NONE>

        // We need to calculate the part's absolute position, in order for
        // it to be put onto the right page and to be pulled into the
        // LayLeaf's PrtArea
        const SwTwips nTopOfAnch = GetTopForObjPos( *pAnchorFrameForVertPos, aRectFnSet.FnRect(), aRectFnSet.IsVert() );
        if( aRectFnSet.IsVert() )
        {
            // --> OD 2009-08-31 #monglianlayout#
            if ( !aRectFnSet.IsVertL2R() )
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
        SwTwips nDist = aRectFnSet.BottomDist( GetAnchoredObj().GetObjRect(),
            aRectFnSet.GetPrtBottom(*pUpperOfOrientFrame) );
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
                    nDist = aRectFnSet.BottomDist( GetAnchoredObj().GetObjRect(),
                        aRectFnSet.GetPrtBottom(*pLayoutFrameToGrow) );
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
                // No need to grow the anchor cell in case the follow-text-flow object
                // is wrap-though.
                if (!GetAnchorFrame().IsInTab() || !DoesObjFollowsTextFlow() || !bWrapThrough)
                {
                    pLayoutFrameToGrow->Grow( -nDist );
                }
            }
        }

        if (!bIgnoreVertLayoutInCell && DoesObjFollowsTextFlow() &&
             ( aVert.GetRelationOrient() != text::RelOrientation::PAGE_FRAME &&
                aVert.GetRelationOrient() != text::RelOrientation::PAGE_PRINT_AREA ) )
        {

            nDist = aRectFnSet.BottomDist( GetAnchoredObj().GetObjRect(),
                aRectFnSet.GetPrtBottom(*pUpperOfOrientFrame) );
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
                    SwTwips nTmpRelPosY( 0 );
                    if ( aRectFnSet.IsVert() )
                        nTmpRelPosY = aRelPos.X() - nDist;
                    else
                        nTmpRelPosY = aRelPos.Y() + nDist;
                    const SwLayoutFrame& rVertEnvironLayFrame =
                        aEnvOfObj.GetVertEnvironmentLayoutFrame( *pUpperOfOrientFrame );
                    nTmpRelPosY = AdjustVertRelPos( nTopOfAnch, aRectFnSet.IsVert(), aRectFnSet.IsVertL2R(),
                                                     rVertEnvironLayFrame, aVert,
                                                     nTmpRelPosY,
                                                     DoesObjFollowsTextFlow(),
                                                     false );
                    if ( aRectFnSet.IsVert() )
                    {
                        aRelPos.setX( nTmpRelPosY );
                        // --> OD 2009-08-31 #mongolianlayout#
                        if ( !aRectFnSet.IsVertL2R() )
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
                        aRelPos.setY( nTmpRelPosY );
                        GetAnchoredObj().SetObjTop( nTopOfAnch + aRelPos.Y() );
                    }
                    // If the anchor frame is the first content of the table cell
                    // and the object still doesn't fit, the table frame is notified,
                    // that the object doesn't fit into the table cell.
                    nDist = aRectFnSet.BottomDist( GetAnchoredObj().GetObjRect(),
                        aRectFnSet.GetPrtBottom(*pUpperOfOrientFrame) );
                    if ( nDist < 0 &&
                         pOrientFrame == &rAnchorTextFrame && !pOrientFrame->GetIndPrev() )
                    {
                        const_cast<SwTabFrame*>(pOrientFrame->FindTabFrame())
                                                        ->SetDoesObjsFit( false );
                    }
                }
            }
            // Don't move split flys around for follow text flow purposes; if they don't fit their
            // parent anymore, they will shrink and part of the content will move to the follow fly.
            else if (!bSplitFly)
            {
                // follow text flow
                const bool bInFootnote = rAnchorTextFrame.IsInFootnote();
                while( bMoveable && nDist < 0 )
                {
                    bool bInSct = pUpperOfOrientFrame->IsInSct();
                    if ( bInSct )
                    {
                        const SwLayoutFrame* pTmp = pUpperOfOrientFrame->FindSctFrame()->GetUpper();
                        nDist = aRectFnSet.BottomDist( GetAnchoredObj().GetObjRect(),
                            aRectFnSet.GetPrtBottom(*pTmp) );
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
                         aRectFnSet.GetTop(GetAnchoredObj().GetObjRect()) ==
                                 aRectFnSet.GetPrtTop(*pUpperOfOrientFrame) )
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
                        SwRectFnSet fnRectX(pNextLay);
                        if ( !bInSct ||
                             ( pUpperOfOrientFrame->FindSctFrame()->IsAnFollow( pNextLay->FindSctFrame() ) &&
                               fnRectX.GetHeight(pNextLay->getFramePrintArea()) ) )
                        {
                            SwTwips nTmpRelPosY =
                                aRectFnSet.YDiff( aRectFnSet.GetPrtTop(*pNextLay),
                                                    nTopOfAnch );
                            if ( aRectFnSet.IsVert() )
                                aRelPos.setX( nTmpRelPosY );
                            else
                                aRelPos.setY( nTmpRelPosY );
                            pUpperOfOrientFrame = pNextLay;
                            aRectFnSet.Refresh(pUpperOfOrientFrame);
                            bMoveable = rAnchorTextFrame.IsMoveable( pUpperOfOrientFrame );
                            if( fnRectX.IsVert() )
                            {
                                // --> OD 2009-08-31 #mongolianlayout#
                                if ( !aRectFnSet.IsVertL2R() )
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
                            nDist = aRectFnSet.BottomDist( GetAnchoredObj().GetObjRect(),
                                aRectFnSet.GetPrtBottom(*pUpperOfOrientFrame) );
                        }
                        // #i23129# - leave section area
                        else if ( bInSct )
                        {
                            const SwLayoutFrame* pTmp = pUpperOfOrientFrame->FindSctFrame()->GetUpper();
                            nDist = aRectFnSet.BottomDist( GetAnchoredObj().GetObjRect(),
                                aRectFnSet.GetPrtBottom(*pTmp) );
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
                        nDist = aRectFnSet.BottomDist( GetAnchoredObj().GetObjRect(),
                            aRectFnSet.GetPrtBottom(*pTmp) );
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

        // If it was requested to not overlap with already formatted objects, take care of that
        // here.
        CalcOverlap(pAnchorFrameForVertPos, aRelPos, nTopOfAnch);
    }

    // determine 'horizontal' position
    {
        // determine horizontal positioning and alignment attributes
        SwFormatHoriOrient aHori( rFrameFormat.GetHoriOrient() );

        // set calculated vertical position in order to determine correct
        // frame, the horizontal position is oriented at.
        const SwTwips nTopOfAnch = GetTopForObjPos( *pAnchorFrameForVertPos, aRectFnSet.FnRect(), aRectFnSet.IsVert() );
        if( aRectFnSet.IsVert() )
        {
            // --> OD 2009-08-31 #mongolianlayout#
            if ( !aRectFnSet.IsVertL2R() )
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
                                      ? &GetHoriVirtualAnchor( *mpVertPosOrientFrame )
                                      : pOrientFrame;

        // #i26791# - get 'horizontal' offset to frame anchor position.
        SwTwips nHoriOffsetToFrameAnchorPos( 0 );
        SwTwips nRelPosX = CalcRelPosX( *pHoriOrientFrame, aEnvOfObj,
                                         aHori, rLR, rUL, bWrapThrough,
                                         ( aRectFnSet.IsVert() ? aRelPos.X() : aRelPos.Y() ),
                                         nHoriOffsetToFrameAnchorPos );

        // #i26791# - determine offset to 'horizontal' frame
        // anchor position, depending on layout-direction
        if ( aRectFnSet.IsVert() )
        {
            aRelPos.setY( nRelPosX );
            maOffsetToFrameAnchorPos.setY( nHoriOffsetToFrameAnchorPos );
        }
        else
        {
            aRelPos.setX( nRelPosX );
            maOffsetToFrameAnchorPos.setX( nHoriOffsetToFrameAnchorPos );
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
    const SwTwips nTopOfAnch = GetTopForObjPos( *pAnchorFrameForVertPos, aRectFnSet.FnRect(), aRectFnSet.IsVert() );
    if( aRectFnSet.IsVert() )
    {
        // --> OD 2009-08-31 #mongolianlayout#
        if ( !aRectFnSet.IsVertL2R() )
        {
            GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                         aRelPos.X() - aObjBoundRect.Width() );
        }
        else
        {
            GetAnchoredObj().SetObjLeft( nTopOfAnch + aRelPos.X() );
        }
        GetAnchoredObj().SetObjTop( rAnchorTextFrame.getFrameArea().Top() +
                                    aRelPos.Y() );
    }
    else
    {
        GetAnchoredObj().SetObjLeft( rAnchorTextFrame.getFrameArea().Left() +
                                     aRelPos.X() );
        GetAnchoredObj().SetObjTop( nTopOfAnch + aRelPos.Y() );
    }

    // set relative position at object
    GetAnchoredObj().SetCurrRelPos( aRelPos );
}

void SwToContentAnchoredObjectPosition::CalcOverlap(const SwTextFrame* pAnchorFrameForVertPos,
                                                    Point& rRelPos, const SwTwips nTopOfAnch)
{
    const SwFrameFormat& rFrameFormat = GetFrameFormat();
    bool bAllowOverlap = rFrameFormat.GetWrapInfluenceOnObjPos().GetAllowOverlap();
    if (bAllowOverlap)
    {
        return;
    }

    if (rFrameFormat.GetSurround().GetSurround() == css::text::WrapTextMode_THROUGH)
    {
        // This is explicit wrap through: allowed to overlap.
        return;
    }

    if (SwTextBoxHelper::isTextBox(&rFrameFormat, RES_FLYFRMFMT))
    {
        // This is the frame part of a textbox, just take the offset from the textbox's shape part.
        SwFrameFormat* pShapeOfTextBox
            = SwTextBoxHelper::getOtherTextBoxFormat(&rFrameFormat, RES_FLYFRMFMT);
        if (pShapeOfTextBox)
        {
            SwTwips nYDiff = pShapeOfTextBox->GetWrapInfluenceOnObjPos().GetOverlapVertOffset();
            if (nYDiff > 0)
            {
                rRelPos.setY(rRelPos.getY() + nYDiff + 1);
                GetAnchoredObj().SetObjTop(nTopOfAnch + rRelPos.Y());
            }
        }
        return;
    }

    // Get the list of objects.
    auto pSortedObjs = pAnchorFrameForVertPos->GetDrawObjs();
    const SwLayoutFrame* pAnchorUpper = pAnchorFrameForVertPos->GetUpper();

    bool bSplitFly = false;
    SwFlyFrame* pFlyFrame = GetAnchoredObj().DynCastFlyFrame();
    if (pFlyFrame && pFlyFrame->IsFlySplitAllowed())
    {
        // At least for split flys we need to consider objects on the same page, but anchored in
        // different text frames.
        bSplitFly = true;

        SwFrame* pFlyFrameAnchor = pFlyFrame->GetAnchorFrameContainingAnchPos();
        if (pFlyFrameAnchor && pFlyFrameAnchor->IsInFly())
        {
            // An inner fly overlapping with its outer fly is fine.
            return;
        }

        const SwPageFrame* pPageFrame = pAnchorFrameForVertPos->FindPageFrame();
        if (pPageFrame)
        {
            pSortedObjs = pPageFrame->GetSortedObjs();
        }
    }

    if (!pSortedObjs)
    {
        return;
    }

    for (const auto& pAnchoredObj : *pSortedObjs)
    {
        if (pAnchoredObj == &GetAnchoredObj())
        {
            // We found ourselves, stop iterating.
            break;
        }

        if (SwTextBoxHelper::isTextBox(pAnchoredObj->GetFrameFormat(), RES_FLYFRMFMT))
        {
            // Overlapping with the frame of a textbox is fine.
            continue;
        }

        SwFlyFrame* pAnchoredObjFly = pAnchoredObj->DynCastFlyFrame();
        if (bSplitFly)
        {
            if (!pAnchoredObjFly)
            {
                // This is a split fly, then overlap is only checked against other split flys.
                continue;
            }

            if (pAnchoredObjFly->getRootFrame()->IsInFlyDelList(pAnchoredObjFly))
            {
                // A fly overlapping with a to-be-deleted fly is fine.
                continue;
            }

            SwFrame* pAnchoredObjFlyAnchor = pAnchoredObjFly->GetAnchorFrameContainingAnchPos();
            if (pAnchoredObjFlyAnchor && pAnchoredObjFlyAnchor->IsInFly())
            {
                // An inner fly overlapping with its outer fly is fine.
                continue;
            }

            if (pAnchoredObjFlyAnchor && pAnchoredObjFlyAnchor->GetUpper() != pAnchorUpper)
            {
                // A fly overlapping with a fly from another upper is fine.
                continue;
            }
        }

        css::text::WrapTextMode eWrap = pAnchoredObj->GetFrameFormat()->GetSurround().GetSurround();
        if (eWrap == css::text::WrapTextMode_THROUGH)
        {
            // The other object is wrap through: allowed to overlap.
            continue;
        }

        if (!GetAnchoredObj().GetObjRect().Overlaps(pAnchoredObj->GetObjRect()))
        {
            // Found an already positioned object, but it doesn't overlap, ignore.
            continue;
        }

        // Already formatted, overlaps: resolve the conflict by shifting ourselves down.
        SwTwips nYDiff = pAnchoredObj->GetObjRect().Bottom() - GetAnchoredObj().GetObjRect().Top();
        rRelPos.setY(rRelPos.getY() + nYDiff + 1);
        GetAnchoredObj().SetObjTop(nTopOfAnch + rRelPos.Y());

        // Store our offset that avoids the overlap. If this is a shape of a textbox, then the frame
        // of the textbox will use it.
        SwFormatWrapInfluenceOnObjPos aInfluence(rFrameFormat.GetWrapInfluenceOnObjPos());
        aInfluence.SetOverlapVertOffset(nYDiff);
        const_cast<SwFrameFormat&>(rFrameFormat).LockModify();
        const_cast<SwFrameFormat&>(rFrameFormat).SetFormatAttr(aInfluence);
        const_cast<SwFrameFormat&>(rFrameFormat).UnlockModify();
    }
}

/**
 * Determine frame for horizontal position
 */
const SwFrame& SwToContentAnchoredObjectPosition::GetHoriVirtualAnchor(
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
