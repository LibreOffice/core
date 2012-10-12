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

#include <tocntntanchoredobjectposition.hxx>
#include <anchoredobject.hxx>
#include <frame.hxx>
#include <txtfrm.hxx>
#include <pagefrm.hxx>
#include <sectfrm.hxx>
// #i26945#
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


SwToCntntAnchoredObjectPosition::SwToCntntAnchoredObjectPosition( SdrObject& _rDrawObj )
    : SwAnchoredObjectPosition ( _rDrawObj ),
      mpVertPosOrientFrm( 0 ),
      // #i26791#
      maOffsetToFrmAnchorPos( Point() ),
      mbAnchorToChar ( false ),
      mpToCharOrientFrm( 0 ),
      mpToCharRect( 0 ),
      // #i22341#
      mnToCharTopOfLine( 0 )
{}

SwToCntntAnchoredObjectPosition::~SwToCntntAnchoredObjectPosition()
{}

bool SwToCntntAnchoredObjectPosition::IsAnchoredToChar() const
{
    return mbAnchorToChar;
}

const SwFrm* SwToCntntAnchoredObjectPosition::ToCharOrientFrm() const
{
    return mpToCharOrientFrm;
}

const SwRect* SwToCntntAnchoredObjectPosition::ToCharRect() const
{
    return mpToCharRect;
}

// #i22341#
SwTwips SwToCntntAnchoredObjectPosition::ToCharTopOfLine() const
{
    return mnToCharTopOfLine;
}

SwTxtFrm& SwToCntntAnchoredObjectPosition::GetAnchorTxtFrm() const
{
    OSL_ENSURE( GetAnchorFrm().ISA(SwTxtFrm),
            "SwToCntntAnchoredObjectPosition::GetAnchorTxtFrm() - wrong anchor frame type" );

    return static_cast<SwTxtFrm&>(GetAnchorFrm());
}

// #i23512#
static bool lcl_DoesVertPosFits( const SwTwips _nRelPosY,
                          const SwTwips _nAvail,
                          const SwLayoutFrm* _pUpperOfOrientFrm,
                          const bool _bBrowse,
                          const bool _bGrowInTable,
                          SwLayoutFrm*& _orpLayoutFrmToGrow )
{
    bool bVertPosFits = false;

    if ( _nRelPosY <= _nAvail )
    {
        bVertPosFits = true;
    }
    else if ( _bBrowse )
    {
        if ( _pUpperOfOrientFrm->IsInSct() )
        {
            SwSectionFrm* pSctFrm =
                    const_cast<SwSectionFrm*>(_pUpperOfOrientFrm->FindSctFrm());
            bVertPosFits = pSctFrm->GetUpper()->Grow( _nRelPosY - _nAvail, sal_True ) > 0;
            // Note: do not provide a layout frame for a grow.
        }
        else
        {
            bVertPosFits = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm)->
                                        Grow( _nRelPosY - _nAvail, sal_True ) > 0;
            if ( bVertPosFits )
                _orpLayoutFrmToGrow = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm);
        }
    }
    else if ( _pUpperOfOrientFrm->IsInTab() && _bGrowInTable )
    {
        // #i45085# - check, if upper frame would grow the
        // excepted amount of twips.
        const SwTwips nTwipsGrown = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm)->
                                        Grow( _nRelPosY - _nAvail, sal_True ) > 0;
        bVertPosFits = ( nTwipsGrown == ( _nRelPosY - _nAvail ) );
        if ( bVertPosFits )
            _orpLayoutFrmToGrow = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm);
    }

    return bVertPosFits;
}

void SwToCntntAnchoredObjectPosition::CalcPosition()
{
    // get format of object
    const SwFrmFmt& rFrmFmt = GetFrmFmt();

    // declare and set <pFooter> to footer frame, if object is anchored
    // at a frame belonging to the footer.
    const SwFrm* pFooter = GetAnchorFrm().FindFooterOrHeader();
    if ( pFooter && !pFooter->IsFooterFrm() )
        pFooter = NULL;

    // declare and set <bBrowse> to true, if document is in browser mode and
    // object is anchored at the body, but not at frame belonging to a table.
    bool bBrowse = GetAnchorFrm().IsInDocBody() && !GetAnchorFrm().IsInTab();
    if( bBrowse )
    {
        const ViewShell *pSh = GetAnchorFrm().getRootFrm()->GetCurrShell();
        if( !pSh || !pSh->GetViewOptions()->getBrowseMode() )
            bBrowse = false;
    }

    // determine left/right and its upper/lower spacing.
    const SvxLRSpaceItem &rLR = rFrmFmt.GetLRSpace();
    const SvxULSpaceItem &rUL = rFrmFmt.GetULSpace();

    // determine, if object has no surrounding.
    const SwFmtSurround& rSurround = rFrmFmt.GetSurround();
    const bool bNoSurround = rSurround.GetSurround() == SURROUND_NONE;
    const bool bWrapThrough = rSurround.GetSurround() == SURROUND_THROUGHT;

    // new class <SwEnvironmentOfAnchoredObject>
    SwEnvironmentOfAnchoredObject aEnvOfObj( DoesObjFollowsTextFlow() );

    // #i18732# - grow only, if object has to follow the text flow
    const bool bGrow = DoesObjFollowsTextFlow() &&
                       ( !GetAnchorFrm().IsInTab() ||
                         !rFrmFmt.GetFrmSize().GetHeightPercent() );

    // get text frame the object is anchored at
    const SwTxtFrm& rAnchorTxtFrm = GetAnchorTxtFrm();
    SWRECTFN( (&rAnchorTxtFrm) )

    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );

    // local variable keeping the calculated relative position; initialized with
    // current relative position.
    // #i26791# - use new object instance of <SwAnchoredObject>
    Point aRelPos( GetAnchoredObj().GetCurrRelPos() );

    SwTwips nRelDiff = 0;

    bool bMoveable = rAnchorTxtFrm.IsMoveable();

    // determine frame the object position has to be oriented at.
    const SwTxtFrm* pOrientFrm = &rAnchorTxtFrm;
    const SwTxtFrm* pAnchorFrmForVertPos = &rAnchorTxtFrm;
    {
        // if object is at-character anchored, determine character-rectangle
        // and frame, position has to be oriented at.
        mbAnchorToChar = (FLY_AT_CHAR == rFrmFmt.GetAnchor().GetAnchorId());
        if ( mbAnchorToChar )
        {
            const SwFmtAnchor& rAnch = rFrmFmt.GetAnchor();
            // #i26791# - use new object instance of <SwAnchoredObject>
            // Due to table break algorithm the character
            // rectangle can have no height. Thus, check also the width
            if ( ( !GetAnchoredObj().GetLastCharRect().Height() &&
                   !GetAnchoredObj().GetLastCharRect().Width() ) ||
                 !GetAnchoredObj().GetLastTopOfLine() )
            {
                // #i111886#
                // Check existence of paragraph portion information in order
                // to avoid formatting which could cause deletion of follow frames.
                GetAnchoredObj().CheckCharRectAndTopOfLine();

                // Due to table break algorithm the character
                // rectangle can have no height. Thus, check also the width
                if ( ( !GetAnchoredObj().GetLastCharRect().Height() &&
                       !GetAnchoredObj().GetLastCharRect().Width() ) ||
                     !GetAnchoredObj().GetLastTopOfLine() )
                {
                    // Get default for <mpVertPosOrientFrm>, if it's not set.
                    if ( !mpVertPosOrientFrm )
                    {
                        mpVertPosOrientFrm = rAnchorTxtFrm.GetUpper();
                    }
                    return;
                }
            }
            mpToCharRect = &(GetAnchoredObj().GetLastCharRect());
            // #i22341# - get top of line, in which the anchor character is.
            mnToCharTopOfLine = GetAnchoredObj().GetLastTopOfLine();
            pOrientFrm = &(const_cast<SwTxtFrm&>(rAnchorTxtFrm).GetFrmAtOfst(
                                rAnch.GetCntntAnchor()->nContent.GetIndex() ) );
            mpToCharOrientFrm = pOrientFrm;
        }
    }
    SWREFRESHFN( pOrientFrm )

    // determine vertical position
    {

        // determine vertical positioning and alignment attributes
        SwFmtVertOrient aVert( rFrmFmt.GetVertOrient() );

        // #i18732# - determine layout frame for vertical
        // positions aligned to 'page areas'.
        const SwLayoutFrm& rPageAlignLayFrm =
                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pOrientFrm );

        if ( aVert.GetVertOrient() != text::VertOrientation::NONE )
        {
            // #i18732# - adjustments for follow text flow or not
            // AND vertical alignment at 'page areas'.
            SwTwips nAlignAreaHeight;
            SwTwips nAlignAreaOffset;
            _GetVertAlignmentValues( *pOrientFrm, rPageAlignLayFrm,
                                     aVert.GetRelationOrient(),
                                     nAlignAreaHeight, nAlignAreaOffset );

            // determine relative vertical position
            SwTwips nRelPosY = nAlignAreaOffset;
            const SwTwips nObjHeight = (aObjBoundRect.*fnRect->fnGetHeight)();
            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
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
                        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
                        if ( bVert && !bVertL2R )
                        {
                            nRelPosY += nObjHeight;
                        }
                        break;
                    }
                }
                // no break here
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
                        OSL_FAIL( "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
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
                        OSL_FAIL( "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
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
                        OSL_FAIL( "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
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
            if ( DoesObjFollowsTextFlow() && pOrientFrm != &rAnchorTxtFrm )
            {
                // #i11860# - use new method <_GetTopForObjPos>
                // to get top of frame for object positioning.
                const SwTwips nTopOfOrient = _GetTopForObjPos( *pOrientFrm, fnRect, bVert );
                nRelPosY += (*fnRect->fnYDiff)( nTopOfOrient,
                                      _GetTopForObjPos( rAnchorTxtFrm, fnRect, bVert ) );
            }

            // #i42124# - capture object inside vertical
            // layout environment.
            {
                const SwTwips nTopOfAnch =
                                _GetTopForObjPos( *pOrientFrm, fnRect, bVert );
                const SwLayoutFrm& rVertEnvironLayFrm =
                    aEnvOfObj.GetVertEnvironmentLayoutFrm(
                                            *(pOrientFrm->GetUpper()) );
                const bool bCheckBottom = !DoesObjFollowsTextFlow();
                nRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                              rVertEnvironLayFrm, nRelPosY,
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
                    const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
                    const_cast<SwFrmFmt&>(rFrmFmt).SetFmtAttr( aVert );
                    const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();
                }
            }

            // determine absolute 'vertical' position, depending on layout-direction
            // #i26791# - determine offset to 'vertical' frame
            // anchor position, depending on layout-direction
            if ( bVert )
            {
                aRelPos.X() = nRelPosY;
                maOffsetToFrmAnchorPos.X() = nAlignAreaOffset;
            }
            else
            {
                aRelPos.Y() = nRelPosY;
                maOffsetToFrmAnchorPos.Y() = nAlignAreaOffset;
            }
        }

        // Determine upper of frame vertical position is oriented at.
        // #i28701# - determine 'virtual' anchor frame.
        // This frame is used in the following instead of the 'real' anchor
        // frame <rAnchorTxtFrm> for the 'vertical' position in all cases.
        const SwLayoutFrm* pUpperOfOrientFrm = 0L;
        {
            // #i28701# - As long as the anchor frame is on the
            // same page as <pOrientFrm> and the vertical position isn't aligned
            // automatic at the anchor character or the top of the line of the
            // anchor character, the anchor frame determines the vertical position.
            if ( &rAnchorTxtFrm == pOrientFrm ||
                 ( rAnchorTxtFrm.FindPageFrm() == pOrientFrm->FindPageFrm() &&
                   aVert.GetVertOrient() == text::VertOrientation::NONE &&
                   aVert.GetRelationOrient() != text::RelOrientation::CHAR &&
                   aVert.GetRelationOrient() != text::RelOrientation::TEXT_LINE ) )
            {
                pUpperOfOrientFrm = rAnchorTxtFrm.GetUpper();
                pAnchorFrmForVertPos = &rAnchorTxtFrm;
            }
            else
            {
                pUpperOfOrientFrm = pOrientFrm->GetUpper();
                pAnchorFrmForVertPos = pOrientFrm;
            }
        }

        // ignore one-column sections.
        // #i23512# - correction: also ignore one-columned
        // sections with footnotes/endnotes
        if ( pUpperOfOrientFrm->IsInSct() )
        {
            const SwSectionFrm* pSctFrm = pUpperOfOrientFrm->FindSctFrm();
            const bool bIgnoreSection = pUpperOfOrientFrm->IsSctFrm() ||
                                        ( pSctFrm->Lower()->IsColumnFrm() &&
                                          !pSctFrm->Lower()->GetNext() );
            if ( bIgnoreSection )
                pUpperOfOrientFrm = pSctFrm->GetUpper();
        }

        if ( aVert.GetVertOrient() == text::VertOrientation::NONE )
        {
            // local variable <nRelPosY> for calculation of relative vertical
            // distance to anchor.
            SwTwips nRelPosY = 0;
            // #i26791# - local variable <nVertOffsetToFrmAnchorPos>
            // for determination of the 'vertical' offset to the frame anchor
            // position
            SwTwips nVertOffsetToFrmAnchorPos( 0L );
            // #i22341# - add special case for vertical alignment
            // at top of line.
            if ( mbAnchorToChar &&
                 ( aVert.GetRelationOrient() == text::RelOrientation::CHAR ||
                   aVert.GetRelationOrient() == text::RelOrientation::TEXT_LINE ) )
            {
                // #i11860# - use new method <_GetTopForObjPos>
                // to get top of frame for object positioning.
                SwTwips nTopOfOrient = _GetTopForObjPos( *pOrientFrm, fnRect, bVert );
                if ( aVert.GetRelationOrient() == text::RelOrientation::CHAR )
                {
                    nVertOffsetToFrmAnchorPos = (*fnRect->fnYDiff)(
                                        (ToCharRect()->*fnRect->fnGetBottom)(),
                                        nTopOfOrient );
                }
                else
                {
                    nVertOffsetToFrmAnchorPos = (*fnRect->fnYDiff)( ToCharTopOfLine(),
                                                                    nTopOfOrient );
                }
                nRelPosY = nVertOffsetToFrmAnchorPos - aVert.GetPos();
            }
            else
            {
                // #i28701# - correction: use <pAnchorFrmForVertPos>
                // instead of <pOrientFrm> and do not adjust relative position
                // to get correct vertical position.
                nVertOffsetToFrmAnchorPos = 0L;
                // #i11860# - use new method <_GetTopForObjPos>
                // to get top of frame for object positioning.
                const SwTwips nTopOfOrient =
                        _GetTopForObjPos( *pAnchorFrmForVertPos, fnRect, bVert );
                // Increase <nRelPosY> by margin height,
                // if position is vertical aligned to "paragraph text area"
                if ( aVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA )
                {
                    // #i11860# - consider upper space amount of previous frame
                    SwTwips nTopMargin = (pAnchorFrmForVertPos->*fnRect->fnGetTopMargin)();
                    if ( pAnchorFrmForVertPos->IsTxtFrm() )
                    {
                        nTopMargin -= static_cast<const SwTxtFrm*>(pAnchorFrmForVertPos)->
                            GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
                    }
                    nVertOffsetToFrmAnchorPos += nTopMargin;
                }
                // #i18732# - adjust <nRelPosY> by difference
                // between 'page area' and 'anchor' frame, if position is
                // vertical aligned to 'page areas'
                else if ( aVert.GetRelationOrient() == text::RelOrientation::PAGE_FRAME )
                {
                    nVertOffsetToFrmAnchorPos += (*fnRect->fnYDiff)(
                                    (rPageAlignLayFrm.Frm().*fnRect->fnGetTop)(),
                                    nTopOfOrient );
                }
                else if ( aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    SwRect aPgPrtRect( rPageAlignLayFrm.Frm() );
                    if ( rPageAlignLayFrm.IsPageFrm() )
                    {
                        aPgPrtRect =
                            static_cast<const SwPageFrm&>(rPageAlignLayFrm).PrtWithoutHeaderAndFooter();
                    }
                    nVertOffsetToFrmAnchorPos += (*fnRect->fnYDiff)(
                                                (aPgPrtRect.*fnRect->fnGetTop)(),
                                                nTopOfOrient );
                }
                nRelPosY = nVertOffsetToFrmAnchorPos + aVert.GetPos();
            }

            // <pUpperOfOrientFrm>: layout frame, at which the position has to
            //                      is oriented at
            // <nRelPosY>:          rest of the relative distance in the current
            //                      layout frame
            // <nAvail>:            space, which is available in the current
            //                      layout frame

            // #i26791# - determine offset to 'vertical'
            // frame anchor position, depending on layout-direction
            if ( bVert )
                maOffsetToFrmAnchorPos.X() = nVertOffsetToFrmAnchorPos;
            else
                maOffsetToFrmAnchorPos.Y() = nVertOffsetToFrmAnchorPos;
            // #i11860# - use new method <_GetTopForObjPos>
            // to get top of frame for object positioning.
            const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrmForVertPos, fnRect, bVert );
            if( nRelPosY <= 0 )
            {
                // Allow negative position, but keep it
                // inside environment layout frame.
                const SwLayoutFrm& rVertEnvironLayFrm =
                    aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm );
                // #i31805# - do not check, if bottom of
                // anchored object would fit into environment layout frame, if
                // anchored object has to follow the text flow.
                const bool bCheckBottom = !DoesObjFollowsTextFlow();
                nRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                              rVertEnvironLayFrm, nRelPosY,
                                              DoesObjFollowsTextFlow(),
                                              bCheckBottom );
                if ( bVert )
                    aRelPos.X() = nRelPosY;
                else
                    aRelPos.Y() = nRelPosY;
            }
            else
            {
                SWREFRESHFN( pAnchorFrmForVertPos )
                SwTwips nAvail =
                    (*fnRect->fnYDiff)( (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)(),
                                        nTopOfAnch );
                const bool bInFtn = pAnchorFrmForVertPos->IsInFtn();
                while ( nRelPosY )
                {
                    // #i23512# - correction:
                    // consider section frame for grow in online layout.
                    // use new local method <lcl_DoesVertPosFits(..)>
                    SwLayoutFrm* pLayoutFrmToGrow = 0L;
                    const bool bDoesVertPosFits = lcl_DoesVertPosFits(
                            nRelPosY, nAvail, pUpperOfOrientFrm, bBrowse,
                            bGrow, pLayoutFrmToGrow );

                    if ( bDoesVertPosFits )
                    {
                        SwTwips nTmpRelPosY =
                            (*fnRect->fnYDiff)( (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)(),
                                                nTopOfAnch ) -
                            nAvail + nRelPosY;
                        // #i28701# - adjust calculated
                        // relative vertical position to object's environment.
                        const SwFrm& rVertEnvironLayFrm =
                            aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm );
                        // Do not check, if bottom of
                        // anchored object would fit into environment layout
                        // frame, if anchored object has to follow the text flow.
                        const bool bCheckBottom = !DoesObjFollowsTextFlow();
                        nTmpRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                                         rVertEnvironLayFrm,
                                                         nTmpRelPosY,
                                                         DoesObjFollowsTextFlow(),
                                                         bCheckBottom );
                        if ( bVert )
                            aRelPos.X() = nTmpRelPosY;
                        else
                            aRelPos.Y() = nTmpRelPosY;

                        // #i23512# - use local variable
                        // <pLayoutFrmToGrow> provided by new method
                        // <lcl_DoesVertPosFits(..)>.
                        if ( pLayoutFrmToGrow )
                        {
                            pLayoutFrmToGrow->Grow( nRelPosY - nAvail );
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
                             !GetAnchorFrm().IsInTab() )
                        {
                            if ( bMoveable )
                            {
                                // follow the text flow
                                nRelPosY -= nAvail;
                                MakePageType eMakePage = bInFtn ? MAKEPAGE_NONE
                                                                : MAKEPAGE_APPEND;
                                const bool bInSct = pUpperOfOrientFrm->IsInSct();
                                if( bInSct )
                                    eMakePage = MAKEPAGE_NOSECTION;

                                const SwLayoutFrm* pTmp =
                                    pUpperOfOrientFrm->GetLeaf( eMakePage, sal_True, &rAnchorTxtFrm );
                                if ( pTmp &&
                                     ( !bInSct ||
                                       pUpperOfOrientFrm->FindSctFrm()->IsAnFollow( pTmp->FindSctFrm() ) ) )
                                {
                                    pUpperOfOrientFrm = pTmp;
                                    bMoveable = rAnchorTxtFrm.IsMoveable( (SwLayoutFrm*)pUpperOfOrientFrm );
                                    SWREFRESHFN( pUpperOfOrientFrm )
                                    nAvail = (pUpperOfOrientFrm->Prt().*fnRect->fnGetHeight)();
                                }
                                else
                                {
                                    // if there isn't enough space in the (colmuned)
                                    // section, leave it and set available space <nAvail>
                                    // to the space below the section.
                                    // if the new available space isn't also enough,
                                    // new pages can be created.
                                    if( bInSct )
                                    {
                                        const SwFrm* pSct = pUpperOfOrientFrm->FindSctFrm();
                                        pUpperOfOrientFrm = pSct->GetUpper();
                                        nAvail = (*fnRect->fnYDiff)(
                                                   (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)(),
                                                   (pSct->*fnRect->fnGetPrtBottom)() );
                                    }
                                    else
                                    {
#if OSL_DEBUG_LEVEL > 1
                                        OSL_FAIL( "<SwToCntntAnchoredObjectPosition::CalcPosition()> - code under investigation by OD, please inform OD about this assertion!" );
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
                            const SwFrm& rVertEnvironLayFrm =
                                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm );
                            nRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                                          rVertEnvironLayFrm,
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

        //Damit das Teil ggf. auf die richtige Seite gestellt und in die
        //PrtArea des LayLeaf gezogen werden kann, muss hier seine
        //absolute Position berechnet werden.
        const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrmForVertPos, fnRect, bVert );
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
        if ( pUpperOfOrientFrm->IsInSct() )
        {
            const SwSectionFrm* pSctFrm = pUpperOfOrientFrm->FindSctFrm();
            const bool bIgnoreSection = pUpperOfOrientFrm->IsSctFrm() ||
                                        ( pSctFrm->Lower()->IsColumnFrm() &&
                                          !pSctFrm->Lower()->GetNext() );
            if ( bIgnoreSection )
                pUpperOfOrientFrm = pSctFrm->GetUpper();
        }
        SwTwips nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                          (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
        if( nDist < 0 )
        {
            // #i23512# - correction:
            // consider section frame for grow in online layout and
            // consider page alignment for grow in table.
            SwLayoutFrm* pLayoutFrmToGrow = 0L;
            if ( bBrowse && rAnchorTxtFrm.IsMoveable() )
            {
                if ( pUpperOfOrientFrm->IsInSct() )
                {
                    pLayoutFrmToGrow = const_cast<SwLayoutFrm*>(
                                    pUpperOfOrientFrm->FindSctFrm()->GetUpper());
                    nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                              (pLayoutFrmToGrow->*fnRect->fnGetPrtBottom)() );
                    if ( nDist >= 0 )
                    {
                        pLayoutFrmToGrow = 0L;
                    }
                }
                else
                {
                    pLayoutFrmToGrow =
                                    const_cast<SwLayoutFrm*>(pUpperOfOrientFrm);
                }
            }
            else if ( rAnchorTxtFrm.IsInTab() && bGrow )
            {
                pLayoutFrmToGrow = const_cast<SwLayoutFrm*>(pUpperOfOrientFrm);
            }
            if ( pLayoutFrmToGrow )
            {
                pLayoutFrmToGrow->Grow( -nDist );
            }
        }

        if ( DoesObjFollowsTextFlow() &&
             !( aVert.GetRelationOrient() == text::RelOrientation::PAGE_FRAME ||
                aVert.GetRelationOrient() == text::RelOrientation::PAGE_PRINT_AREA ) )
        {

            nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                      (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
            // #i26945# - floating screen objects, which are
            // anchored inside a table, doesn't follow the text flow. But, they
            // have to stay inside its layout environment.
            if ( nDist < 0 && pOrientFrm->IsInTab() )
            {
                // If the anchor frame is the first content of the table cell
                // and has no follow, the table frame is notified,
                // that the object doesn't fit into the table cell.
                // Adjustment of position isn't needed in this case.
                if ( pOrientFrm == &rAnchorTxtFrm &&
                     !pOrientFrm->GetFollow() &&
                     !pOrientFrm->GetIndPrev() )
                {
                    const_cast<SwTabFrm*>(pOrientFrm->FindTabFrm())
                                                    ->SetDoesObjsFit( sal_False );
                }
                else
                {
                    SwTwips nTmpRelPosY( 0L );
                    if ( bVert )
                        nTmpRelPosY = aRelPos.X() - nDist;
                    else
                        nTmpRelPosY = aRelPos.Y() + nDist;
                    const SwLayoutFrm& rVertEnvironLayFrm =
                        aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm );
                    nTmpRelPosY = _AdjustVertRelPos( nTopOfAnch, bVert, bVertL2R,
                                                     rVertEnvironLayFrm,
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
                              (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
                    if ( nDist < 0 &&
                         pOrientFrm == &rAnchorTxtFrm && !pOrientFrm->GetIndPrev() )
                    {
                        const_cast<SwTabFrm*>(pOrientFrm->FindTabFrm())
                                                        ->SetDoesObjsFit( sal_False );
                    }
                }
            }
            else
            {
                // follow text flow
                const bool bInFtn = rAnchorTxtFrm.IsInFtn();
                while( bMoveable && nDist < 0 )
                {
                    bool bInSct = pUpperOfOrientFrm->IsInSct();
                    if ( bInSct )
                    {
                        const SwLayoutFrm* pTmp = pUpperOfOrientFrm->FindSctFrm()->GetUpper();
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
                                 (pUpperOfOrientFrm->*fnRect->fnGetPrtTop)() )
                        //Das teil passt nimmer, da hilft auch kein moven.
                        break;

                    const SwLayoutFrm* pNextLay = pUpperOfOrientFrm->GetLeaf(
                                    ( bInSct
                                      ? MAKEPAGE_NOSECTION
                                      : ( bInFtn ? MAKEPAGE_NONE : MAKEPAGE_APPEND ) ),
                                    sal_True, &rAnchorTxtFrm );
                    // correction:
                    // If anchor is in footnote and proposed next layout environment
                    // isn't a footnote frame, object can't follow the text flow
                    if ( bInFtn && pNextLay && !pNextLay->IsFtnFrm() )
                    {
                        pNextLay = 0L;
                    }
                    if ( pNextLay )
                    {
                        SWRECTFNX( pNextLay )
                        if ( !bInSct ||
                             ( pUpperOfOrientFrm->FindSctFrm()->IsAnFollow( pNextLay->FindSctFrm() ) &&
                               (pNextLay->Prt().*fnRectX->fnGetHeight)() ) )
                        {
                            SwTwips nTmpRelPosY =
                                (*fnRect->fnYDiff)( (pNextLay->*fnRect->fnGetPrtTop)(),
                                                    nTopOfAnch );
                            if ( bVert )
                                aRelPos.X() = nTmpRelPosY;
                            else
                                aRelPos.Y() = nTmpRelPosY;
                            pUpperOfOrientFrm = pNextLay;
                            SWREFRESHFN( pUpperOfOrientFrm )
                            bMoveable = rAnchorTxtFrm.IsMoveable( (SwLayoutFrm*)pUpperOfOrientFrm );
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
                                      (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
                        }
                        // #i23129# - leave section area
                        else if ( bInSct )
                        {
                            const SwLayoutFrm* pTmp = pUpperOfOrientFrm->FindSctFrm()->GetUpper();
                            nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                      (pTmp->*fnRect->fnGetPrtBottom)() );
                            if( nDist < 0 )
                                pUpperOfOrientFrm = pTmp;
                            else
                                break;
                        }
                    }
                    else if ( bInSct )
                    {
                        // Wenn wir innerhalb des Bereich nicht genug Platz haben, gucken
                        // wir uns mal die Seite an.
                        const SwLayoutFrm* pTmp = pUpperOfOrientFrm->FindSctFrm()->GetUpper();
                        nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                  (pTmp->*fnRect->fnGetPrtBottom)() );
                        if( nDist < 0 )
                            pUpperOfOrientFrm = pTmp;
                        else
                            break;
                    }
                    else
                        bMoveable = false;
                }
            }
        }

        // keep layout frame vertical position is oriented at.
        mpVertPosOrientFrm = pUpperOfOrientFrm;

    }

    // determine 'horizontal' position
    {
        // determine horizontal positioning and alignment attributes
        SwFmtHoriOrient aHori( rFrmFmt.GetHoriOrient() );

        // set calculated vertical position in order to determine correct
        // frame, the horizontal position is oriented at.
        const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrmForVertPos, fnRect, bVert );
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
        // the text flow, its horizontal position is oriented at <pOrientFrm>.
        const SwFrm* pHoriOrientFrm = DoesObjFollowsTextFlow()
                                      ? &_GetHoriVirtualAnchor( *mpVertPosOrientFrm )
                                      : pOrientFrm;

        // #i26791# - get 'horizontal' offset to frame anchor position.
        SwTwips nHoriOffsetToFrmAnchorPos( 0L );
        SwTwips nRelPosX = _CalcRelPosX( *pHoriOrientFrm, aEnvOfObj,
                                         aHori, rLR, rUL, bWrapThrough,
                                         ( bVert ? aRelPos.X() : aRelPos.Y() ),
                                         nHoriOffsetToFrmAnchorPos );

        // #i26791# - determine offset to 'horizontal' frame
        // anchor position, depending on layout-direction
        if ( bVert )
        {
            aRelPos.Y() = nRelPosX;
            maOffsetToFrmAnchorPos.Y() = nHoriOffsetToFrmAnchorPos;
        }
        else
        {
            aRelPos.X() = nRelPosX;
            maOffsetToFrmAnchorPos.X() = nHoriOffsetToFrmAnchorPos;
        }

        // save calculated horizontal position - needed for filters
        // (including the xml-filter)
        {
            SwTwips nAttrRelPosX = nRelPosX - nHoriOffsetToFrmAnchorPos;
            if ( aHori.GetHoriOrient() != text::HoriOrientation::NONE &&
                 aHori.GetPos() != nAttrRelPosX )
            {
                aHori.SetPos( nAttrRelPosX );
                const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
                const_cast<SwFrmFmt&>(rFrmFmt).SetFmtAttr( aHori );
                const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();
            }
        }
    }

    // set absolute position at object
    const SwTwips nTopOfAnch = _GetTopForObjPos( *pAnchorFrmForVertPos, fnRect, bVert );
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
        GetAnchoredObj().SetObjTop( rAnchorTxtFrm.Frm().Top() +
                                    aRelPos.Y() );
    }
    else
    {
        GetAnchoredObj().SetObjLeft( rAnchorTxtFrm.Frm().Left() +
                                     aRelPos.X() );
        GetAnchoredObj().SetObjTop( nTopOfAnch + aRelPos.Y() );
    }

    // set relative position at object
    GetAnchoredObj().SetCurrRelPos( aRelPos );
}

/** determine frame for horizontal position

    @author OD
*/
const SwFrm& SwToCntntAnchoredObjectPosition::_GetHoriVirtualAnchor(
                                        const SwLayoutFrm& _rProposedFrm ) const
{
    const SwFrm* pHoriVirtAnchFrm = &_rProposedFrm;

    // Search for first lower content frame, which is the anchor or a follow
    // of the anchor (Note: <Anchor.IsAnFollow( Anchor )> is true)
    // If none found, <_rProposedFrm> is returned.
    const SwFrm* pFrm = _rProposedFrm.Lower();
    while ( pFrm )
    {
        if ( pFrm->IsCntntFrm() &&
             GetAnchorTxtFrm().IsAnFollow( static_cast<const SwCntntFrm*>(pFrm) ) )
        {
            pHoriVirtAnchFrm = pFrm;
            break;
        }
        pFrm = pFrm->GetNext();
    }

    return *pHoriVirtAnchFrm;
}

const SwLayoutFrm& SwToCntntAnchoredObjectPosition::GetVertPosOrientFrm() const
{
    return *mpVertPosOrientFrm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
