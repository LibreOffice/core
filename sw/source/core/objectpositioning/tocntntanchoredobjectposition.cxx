/*************************************************************************
 *
 *  $RCSfile: tocntntanchoredobjectposition.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 13:08:47 $
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
#ifndef _TOCNTNTANCHOREDOBJECTPOSITION_HXX
#include <tocntntanchoredobjectposition.hxx>
#endif

#ifndef _FRAME_HXX
#include <frame.hxx>
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
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFOLLOWTEXTFLOW_HXX
#include <fmtfollowtextflow.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _ENVIRONMENTOFANCHOREDOBJECT
#include <environmentofanchoredobject.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif

using namespace objectpositioning;

SwToCntntAnchoredObjectPosition::SwToCntntAnchoredObjectPosition( SdrObject& _rDrawObj )
    : SwAnchoredObjectPosition ( _rDrawObj ),
      mpVertPosOrientFrm( 0 ),
      // --> OD 2004-06-17 #i26791#
      maOffsetToFrmAnchorPos( Point() ),
      mbAnchorToChar ( false ),
      mpToCharOrientFrm( 0 ),
      mpToCharRect( 0 ),
      // OD 12.11.2003 #i22341#
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

// OD 12.11.2003 #i22341#
SwTwips SwToCntntAnchoredObjectPosition::ToCharTopOfLine() const
{
    return mnToCharTopOfLine;
}

SwTxtFrm& SwToCntntAnchoredObjectPosition::GetAnchorTxtFrm() const
{
    ASSERT( GetAnchorFrm().ISA(SwTxtFrm),
            "SwToCntntAnchoredObjectPosition::GetAnchorTxtFrm() - wrong anchor frame type" );

    return static_cast<SwTxtFrm&>(GetAnchorFrm());
}

// --> OD 2004-07-20 #i23512#
bool lcl_DoesVertPosFits( const SwTwips _nRelPosY,
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
            bVertPosFits = pSctFrm->GetUpper()->Grow( _nRelPosY - _nAvail, TRUE ) > 0;
            // Note: do not provide a layout frame for a grow.
        }
        else
        {
            bVertPosFits = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm)->
                                        Grow( _nRelPosY - _nAvail, TRUE ) > 0;
            if ( bVertPosFits )
                _orpLayoutFrmToGrow = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm);
        }
    }
    else if ( _pUpperOfOrientFrm->IsInTab() && _bGrowInTable )
    {
        bVertPosFits = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm)->
                                        Grow( _nRelPosY - _nAvail, TRUE ) > 0;
        if ( bVertPosFits )
            _orpLayoutFrmToGrow = const_cast<SwLayoutFrm*>(_pUpperOfOrientFrm);
    }

    return bVertPosFits;
}
// <--

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
    const bool bBrowse = GetAnchorFrm().IsInDocBody() &&
                         !GetAnchorFrm().IsInTab()
                            ? rFrmFmt.GetDoc()->IsBrowseMode()
                            : false;

    // determine left/right and its upper/lower spacing.
    const SvxLRSpaceItem &rLR = rFrmFmt.GetLRSpace();
    const SvxULSpaceItem &rUL = rFrmFmt.GetULSpace();

    // determine, if object has no surrounding.
    const SwFmtSurround& rSurround = rFrmFmt.GetSurround();
    const bool bNoSurround = rSurround.GetSurround() == SURROUND_NONE;
    const bool bWrapThrough = rSurround.GetSurround() == SURROUND_THROUGHT;

    // OD 30.09.2003 #i18732# - determine, if object has to follow the text flow
    const bool bFollowTextFlow = rFrmFmt.GetFollowTextFlow().GetValue();

    // OD 29.10.2003 #110978# - new class <SwEnvironmentOfAnchoredObject>
    SwEnvironmentOfAnchoredObject aEnvOfObj( bFollowTextFlow );

    // OD 30.09.2003 #i18732# - grow only, if object has to follow the text flow
    const bool bGrow = bFollowTextFlow &&
                       ( !GetAnchorFrm().IsInTab() ||
                         !rFrmFmt.GetFrmSize().GetHeightPercent() );

    // get text frame the object is anchored at
    const SwTxtFrm& rAnchorTxtFrm = GetAnchorTxtFrm();
    SWRECTFN( (&rAnchorTxtFrm) )

    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );

    // local variable keeping the calculated relative position; initialized with
    // current relative position.
    // OD 2004-03-24 #i26791# - use new object instance of <SwAnchoredObject>
    Point aRelPos( GetAnchoredObj().GetCurrRelPos() );

    // member variable keeping the frame the vertical position is oriented at;
    // initialized with the current one.
    // OD 2004-03-24 #i26791# - use new object instance of <SwAnchoredObject>
    mpVertPosOrientFrm = GetAnchoredObj().GetVertPosOrientFrm();

    SwTwips nRelDiff = 0;

    bool bMoveable = rAnchorTxtFrm.IsMoveable();

    // determine frame the object position has to be oriented at.
    const SwFrm* pOrientFrm = &rAnchorTxtFrm;
    {
        // if object is at-character anchored, determine character-rectangle
        // and frame, position has to be oriented at.
        mbAnchorToChar = FLY_AUTO_CNTNT == rFrmFmt.GetAnchor().GetAnchorId();
        if ( mbAnchorToChar )
        {
            const SwFmtAnchor& rAnch = rFrmFmt.GetAnchor();
            // OD 2004-03-24 #i26791# - use new object instance of <SwAnchoredObject>
            if ( !GetAnchoredObj().GetLastCharRect().Height() ||
                 !GetAnchoredObj().GetLastTopOfLine() )
            {
                // --> OD 2004-07-15 #117380# - suppress check for paragraph
                // portion information by passing <false> as first parameter
                GetAnchoredObj().CheckCharRectAndTopOfLine( false );
                if ( !GetAnchoredObj().GetLastCharRect().Height() ||
                     !GetAnchoredObj().GetLastTopOfLine() )
                {
                    return;
                }
            }
            mpToCharRect = &(GetAnchoredObj().GetLastCharRect());
            // OD 12.11.2003 #i22341# - get top of line, in which the anchor
            // character is.
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
        bool bVertChgd = false;

        // OD 22.09.2003 #i18732# - determine layout frame for vertical
        // positions aligned to 'page areas'.
        const SwLayoutFrm& rPageAlignLayFrm =
                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pOrientFrm, true );

        if ( aVert.GetVertOrient() != VERT_NONE )
        {
            // OD 22.09.2003 #i18732# - adjustments for follow text flow or not
            // AND vertical alignment at 'page areas'.
            SwTwips nAlignAreaHeight;
            SwTwips nAlignAreaOffset;
            _GetVertAlignmentValues( *pOrientFrm, rPageAlignLayFrm,
                                     aVert.GetRelationOrient(),
                                     nAlignAreaHeight, nAlignAreaOffset );

            // determine relative vertical position
            SwTwips nRelPosY = nAlignAreaOffset;
            SwTwips nObjHeight = (aObjBoundRect.*fnRect->fnGetHeight)();
            SwTwips nUpperSpace = bVert ? rLR.GetRight() : rUL.GetUpper();
            SwTwips nLowerSpace = bVert ? rLR.GetLeft() : rUL.GetLower();
            switch ( aVert.GetVertOrient() )
            {
                case VERT_CHAR_BOTTOM:
                {
                    if ( mbAnchorToChar )
                    {
                        // bottom (to character anchored)
                        nRelPosY += nAlignAreaHeight + nUpperSpace;
                        if ( bVert )
                            nRelPosY += nObjHeight;
                        break;
                    }
                }
                // no break here
                case VERT_TOP:
                {
                    // OD 12.11.2003 #i22341# - special case for vertical
                    // alignment at top of line
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == REL_VERT_LINE )
                    {
                        nRelPosY -= (nObjHeight + nLowerSpace);
                    }
                    else
                    {
                        nRelPosY += nUpperSpace;
                    }
                }
                break;
                // OD 14.11.2003 #i22341#
                case VERT_LINE_TOP:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == REL_VERT_LINE )
                    {
                        nRelPosY -= (nObjHeight + nLowerSpace);
                    }
                    else
                    {
                        ASSERT( false,
                                "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
                case VERT_CENTER:
                {
                    nRelPosY += (nAlignAreaHeight / 2) - (nObjHeight / 2);
                }
                break;
                // OD 14.11.2003 #i22341#
                case VERT_LINE_CENTER:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == REL_VERT_LINE )
                    {
                        nRelPosY += (nAlignAreaHeight / 2) - (nObjHeight / 2);
                    }
                    else
                    {
                        ASSERT( false,
                                "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
                case VERT_BOTTOM:
                {
                    if ( ( aVert.GetRelationOrient() == FRAME ||
                           aVert.GetRelationOrient() == PRTAREA ) &&
                         bNoSurround )
                    {
                        // bottom (aligned to 'paragraph areas')
                        nRelPosY += nAlignAreaHeight + nUpperSpace;
                    }
                    else
                    {
                        // OD 12.11.2003 #i22341# - special case for vertical
                        // alignment at top of line
                        if ( mbAnchorToChar &&
                             aVert.GetRelationOrient() == REL_VERT_LINE )
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
                // OD 14.11.2003 #i22341#
                case VERT_LINE_BOTTOM:
                {
                    if ( mbAnchorToChar &&
                         aVert.GetRelationOrient() == REL_VERT_LINE )
                    {
                        nRelPosY += nUpperSpace;
                    }
                    else
                    {
                        ASSERT( false,
                                "<SwToCntntAnchoredObjectPosition::CalcPosition()> - unknown combination of vertical position and vertical alignment." );
                    }
                }
                break;
            }

            // adjust relative position by distance between anchor frame and
            // the frame, the object is oriented at.
            if ( pOrientFrm != &rAnchorTxtFrm )
            {
                // OD 2004-03-11 #i11860# - use new method <_GetTopForObjPos>
                // to get top of frame for object positioning.
                const SwTwips nTopOfOrient = _GetTopForObjPos( *pOrientFrm, fnRect, bVert );
                nRelPosY += (*fnRect->fnYDiff)( nTopOfOrient,
                                      _GetTopForObjPos( rAnchorTxtFrm, fnRect, bVert ) );
            }

            // ??? Why saving calculated relative position
            // keep calculated relative vertical position
            if ( nRelPosY != aVert.GetPos() )
            {
                aVert.SetPos( nRelPosY );
                bVertChgd = true;
            }

            // determine absolute 'vertical' position, depending on layout-direction
            // --> OD 2004-06-17 #i26791# - determine offset to 'vertical' frame
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

        // OD 29.10.2003 #110978# - determine upper of frame vertical position
        // is oriented at.
        const SwLayoutFrm* pUpperOfOrientFrm = 0L;
        if ( bFollowTextFlow )
        {
            pUpperOfOrientFrm = aVert.GetVertOrient() == VERT_NONE
                                ? rAnchorTxtFrm.GetUpper()
                                : pOrientFrm->GetUpper();
        }
        else
        {
            pUpperOfOrientFrm = pOrientFrm->GetUpper();
        }

        // ignore one-column sections.
        // --> OD 2004-07-20 #i23512# - correction: also ignore one-columned
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

        if ( aVert.GetVertOrient() == VERT_NONE )
        {
            // local variable <nRelPosY> for calculation of relative vertical
            // distance to anchor.
            SwTwips nRelPosY = 0;
            // --> OD 2004-06-17 #i26791# - local variable <nVertOffsetToFrmAnchorPos>
            // for determination of the 'vertical' offset to the frame anchor
            // position
            SwTwips nVertOffsetToFrmAnchorPos( 0L );
            // OD 12.11.2003 #i22341# - add special case for vertical alignment
            // at top of line.
            if ( mbAnchorToChar &&
                 ( aVert.GetRelationOrient() == REL_CHAR ||
                   aVert.GetRelationOrient() == REL_VERT_LINE ) )
            {
                // OD 2004-03-11 #i11860# - use new method <_GetTopForObjPos>
                // to get top of frame for object positioning.
                SwTwips nTopOfOrient = _GetTopForObjPos( *pOrientFrm, fnRect, bVert );
                if ( aVert.GetRelationOrient() == REL_CHAR )
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
                if( pOrientFrm != &rAnchorTxtFrm )
                {
                    const SwTxtFrm* pTmp = &rAnchorTxtFrm;
                    SWREFRESHFN( pTmp )
                    // OD 2004-03-11 #i11860# - use new method <_GetTopForObjPos>
                    // to get top of frame for object positioning.
                    SwTwips nTop = _GetTopForObjPos( rAnchorTxtFrm, fnRect, bVert );
                    nVertOffsetToFrmAnchorPos -= (*fnRect->fnYDiff)( nTop,
                                    (pTmp->GetUpper()->*fnRect->fnGetPrtTop)());
                    while( pTmp != pOrientFrm )
                    {
                        SWREFRESHFN( pTmp )
                        nVertOffsetToFrmAnchorPos +=
                                (pTmp->GetUpper()->Prt().*fnRect->fnGetHeight)();
                        pTmp = pTmp->GetFollow();
                    }
                    SWREFRESHFN( pTmp )
                }
                nRelPosY = nVertOffsetToFrmAnchorPos - aVert.GetPos();
            }
            else
            {
                nVertOffsetToFrmAnchorPos = 0L;
                // OD 2004-03-11 #i11860# - use new method <_GetTopForObjPos>
                // to get top of frame for object positioning.
                const SwTwips nTopOfOrient = _GetTopForObjPos( *pOrientFrm, fnRect, bVert );
                // OD 02.10.2002 #102646# - increase <nRelPosY> by margin height,
                // if position is vertical aligned to "paragraph text area"
                if ( aVert.GetRelationOrient() == PRTAREA )
                {
                    // OD 2004-03-11 #i11860# - consider upper space amount
                    // of previous frame
                    SwTwips nTopMargin = (pOrientFrm->*fnRect->fnGetTopMargin)();
                    if ( pOrientFrm->IsTxtFrm() )
                    {
                        nTopMargin -= static_cast<const SwTxtFrm*>(pOrientFrm)->
                            GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
                    }
                    nVertOffsetToFrmAnchorPos += nTopMargin;
                }
                // OD 22.09.2003 #i18732# - adjust <nRelPosY> by difference
                // between 'page area' and 'anchor' frame, if position is
                // vertical aligned to 'page areas'
                else if ( aVert.GetRelationOrient() == REL_PG_FRAME )
                {
                    nVertOffsetToFrmAnchorPos += (*fnRect->fnYDiff)(
                                    (rPageAlignLayFrm.Frm().*fnRect->fnGetTop)(),
                                    nTopOfOrient );
                }
                else if ( aVert.GetRelationOrient() == REL_PG_PRTAREA )
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

                if ( pOrientFrm != &rAnchorTxtFrm )
                {
                    const SwTwips nTopOfAnch =
                                _GetTopForObjPos( rAnchorTxtFrm, fnRect, bVert );
                    nVertOffsetToFrmAnchorPos +=
                                (*fnRect->fnYDiff)( nTopOfOrient, nTopOfAnch );
                }

                nRelPosY = nVertOffsetToFrmAnchorPos + aVert.GetPos();
            }

            // <pUpperOfOrientFrm>: layout frame, at which the position has to
            //                      is oriented at
            // <nRelPosY>:          rest of the relative distance in the current
            //                      layout frame
            // <nAvail>:            space, which is available in the current
            //                      layout frame

            // --> OD 2004-06-17 #i26791# - determine offset to 'vertical'
            // frame anchor position, depending on layout-direction
            if ( bVert )
                maOffsetToFrmAnchorPos.X() = nVertOffsetToFrmAnchorPos;
            else
                maOffsetToFrmAnchorPos.Y() = nVertOffsetToFrmAnchorPos;
            // <--
            if( nRelPosY <= 0 )
            {
                // OD 08.09.2003 #110354# - allow negative position, but keep it
                // inside environment layout frame.
                const SwLayoutFrm& rVertEnvironLayFrm =
                    aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm, false );
                // --> OD 2004-07-22 #i31805# - do not check, if bottom of
                // anchored object would fit into environment layout frame, if
                // anchored object has to follow the text flow.
                const bool bCheckBottom = !bFollowTextFlow;
                nRelPosY = _AdjustVertRelPos( rVertEnvironLayFrm, nRelPosY,
                                              bCheckBottom );
                // <--
                if ( bVert )
                    aRelPos.X() = nRelPosY;
                else
                    aRelPos.Y() = nRelPosY;
            }
            else
            {
                SWREFRESHFN( (&rAnchorTxtFrm) )
                // OD 2004-03-11 #i11860# - use new method <_GetTopForObjPos>
                // to get top of frame for object positioning.
                const SwTwips nTopOfAnch = _GetTopForObjPos( rAnchorTxtFrm, fnRect, bVert );
                SwTwips nAvail =
                    (*fnRect->fnYDiff)( (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)(),
                                        nTopOfAnch );
                const bool bInFtn = rAnchorTxtFrm.IsInFtn();
                while ( nRelPosY )
                {
                    // --> OD 2004-07-20 #i23512# - correction:
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
                        if ( bVert )
                            aRelPos.X() = nTmpRelPosY;
                        else
                            aRelPos.Y() = nTmpRelPosY;
//                        if( bVert )
//                            aRelPos.X() = rAnchorTxtFrm.Frm().Left() +
//                                          rAnchorTxtFrm.Frm().Width() -
//                                          ( pUpperOfOrientFrm->Frm().Left() +
//                                            pUpperOfOrientFrm->Prt().Left() ) -
//                                          nAvail + nRelPosY;
//                        else
//                            aRelPos.Y() = pUpperOfOrientFrm->Frm().Top() +
//                                          pUpperOfOrientFrm->Prt().Top() +
//                                          pUpperOfOrientFrm->Prt().Height() -
//                                          nAvail + nRelPosY -
//                                          rAnchorTxtFrm.Frm().Top();

                        // --> OD 2004-07-20 #i23512# - use local variable
                        // <pLayoutFrmToGrow> provided by new method
                        // <lcl_DoesVertPosFits(..)>.
                        if ( pLayoutFrmToGrow )
                        {
                            pLayoutFrmToGrow->Grow( nRelPosY - nAvail );
                        }
                        // <--
                        nRelPosY = 0;
                    }
                    else
                    {
                        if ( bFollowTextFlow &&
                             !( aVert.GetRelationOrient() == REL_PG_FRAME ||
                                aVert.GetRelationOrient() == REL_PG_PRTAREA ) )
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
                                    pUpperOfOrientFrm->GetLeaf( eMakePage, TRUE, &rAnchorTxtFrm );
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
                                        ASSERT( false, "<SwToCntntAnchoredObjectPosition::CalcPosition()> - code under investigation by OD, please inform OD about this assertion!" );
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
                            // OD 06.10.2003 #i18732# - do not follow text flow respectively
                            // align at 'page areas', but stay inside given environment
                            const SwFrm& rVertEnvironLayFrm =
                                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pUpperOfOrientFrm, false );
                            nRelPosY = _AdjustVertRelPos( rVertEnvironLayFrm, nRelPosY );
                            if ( bVert )
                                aRelPos.X() = nRelPosY;
                            else
                                aRelPos.Y() = nRelPosY;
                            nRelPosY = 0;
                        }
                    }
                } // end of <while ( nRelPosY )>
            } // end of else <nRelPosY <= 0>
        } // end of <aVert.GetVertOrient() == VERT_NONE>

        //Damit das Teil ggf. auf die richtige Seite gestellt und in die
        //PrtArea des LayLeaf gezogen werden kann, muss hier seine
        //absolute Position berechnet werden.
        const SwTwips nTopOfAnch = _GetTopForObjPos( rAnchorTxtFrm, fnRect, bVert );
        if( bVert )
        {
            GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                         ( aRelPos.X() - nRelDiff ) -
                                         aObjBoundRect.Width() );
        }
        else
        {
            GetAnchoredObj().SetObjTop( nTopOfAnch +
                                        ( aRelPos.Y() - nRelDiff ) );
        }

        // grow environment under certain conditions
        // ignore one-column sections.
        // --> OD 2004-07-20 #i23512# - correction: also ignore one-columned
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
            // --> OD 2004-07-20 #i23512# - correction:
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
            // <--
        }

        if ( bFollowTextFlow &&
             !( aVert.GetRelationOrient() == REL_PG_FRAME ||
                aVert.GetRelationOrient() == REL_PG_PRTAREA ) )
        {
            // follow text flow
            const bool bInFtn = rAnchorTxtFrm.IsInFtn();
            nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                      (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
            while( bMoveable && nDist < 0 )
            {
                bool bInSct = pUpperOfOrientFrm->IsInSct();
                if ( bInSct )
                {
                    const SwLayoutFrm* pTmp = pUpperOfOrientFrm->FindSctFrm()->GetUpper();
                    nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                              (pTmp->*fnRect->fnGetPrtBottom)() );
                    if( nDist < 0 )
                        pUpperOfOrientFrm = pTmp;
                    else
                        break;
                    bInSct = pUpperOfOrientFrm->IsInSct();
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
                                TRUE, &rAnchorTxtFrm );
                // OD 06.10.2003 #110978# - correction:
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
//                        if( bVertX )
//                            aRelPos.X() = rAnchorTxtFrm.Frm().Left() +
//                                          rAnchorTxtFrm.Frm().Width() -
//                                          ( pNextLay->Frm().Left() +
//                                            pNextLay->Prt().Left() +
//                                            pNextLay->Prt().Width() );
//                        else
//                            aRelPos.Y() = pNextLay->Frm().Top() +
//                                          pNextLay->Prt().Top() -
//                                          rAnchorTxtFrm.Frm().Top();
                        pUpperOfOrientFrm = pNextLay;
                        SWREFRESHFN( pUpperOfOrientFrm )
                        bMoveable = rAnchorTxtFrm.IsMoveable( (SwLayoutFrm*)pUpperOfOrientFrm );
                        if( bVertX )
                            GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                                         aRelPos.X() -
                                                         aObjBoundRect.Width() );
                        else
                            GetAnchoredObj().SetObjTop( nTopOfAnch +
                                                        aRelPos.Y() );
                        nDist = (GetAnchoredObj().GetObjRect().*fnRect->fnBottomDist)(
                                  (pUpperOfOrientFrm->*fnRect->fnGetPrtBottom)() );
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

        // save calculated vertical position
        const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
        if ( bVertChgd )
            const_cast<SwFrmFmt&>(rFrmFmt).SetAttr( aVert );
        const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();

        // keep layout frame vertical position is oriented at.
        mpVertPosOrientFrm = pUpperOfOrientFrm;

    }

    // determine 'horizontal' position
    {
        // determine horizontal positioning and alignment attributes
        SwFmtHoriOrient aHori( rFrmFmt.GetHoriOrient() );
        bool bHoriChgd = false;

        // set calculated vertical position in order to determine correct
        // frame, the horizontal position is oriented at.
        const SwTwips nTopOfAnch = _GetTopForObjPos( rAnchorTxtFrm, fnRect, bVert );
        if( bVert )
            GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                         aRelPos.X() - aObjBoundRect.Width() );
        else
            GetAnchoredObj().SetObjTop( nTopOfAnch + aRelPos.Y() );

        // determine frame, horizontal position is oriented at.
        const SwFrm* pHoriOrientFrm = &_GetHoriVirtualAnchor( *mpVertPosOrientFrm );

        // --> OD 2004-06-17 #i26791# - get 'horizontal' offset to frame anchor position.
        SwTwips nHoriOffsetToFrmAnchorPos( 0L );
        SwTwips nRelPosX = _CalcRelPosX( *pHoriOrientFrm, aEnvOfObj,
                                         aHori, rLR, rUL, bWrapThrough,
                                         ( bVert ? aRelPos.X() : aRelPos.Y() ),
                                         nHoriOffsetToFrmAnchorPos );

        // --> OD 2004-06-17 #i26791# - determine offset to 'horizontal' frame
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

        if ( HORI_NONE != aHori.GetHoriOrient() &&
             aHori.GetPos() != nRelPosX )
        {
            aHori.SetPos( nRelPosX );
            bHoriChgd = true;
        }

        // WHY ???
        // save calculated horizontal position
        if ( bHoriChgd )
        {
            const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
            const_cast<SwFrmFmt&>(rFrmFmt).SetAttr( aHori );
            const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();
        }
    }

    // set absolute position at object
    const SwTwips nTopOfAnch = _GetTopForObjPos( rAnchorTxtFrm, fnRect, bVert );
    if( bVert )
    {
        GetAnchoredObj().SetObjLeft( nTopOfAnch -
                                     aRelPos.X() - aObjBoundRect.Width() );
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

/** determined offset to frame anchor position

    --> OD 2004-06-17 #i26791#

    @author OD
*/
Point SwToCntntAnchoredObjectPosition::GetOffsetToFrmAnchorPos() const
{
    return maOffsetToFrmAnchorPos;
}

