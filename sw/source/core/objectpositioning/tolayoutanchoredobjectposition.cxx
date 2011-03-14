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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <tolayoutanchoredobjectposition.hxx>
#include <anchoredobject.hxx>
#include <frame.hxx>
#include <pagefrm.hxx>
#include <svx/svdobj.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <IDocumentSettingAccess.hxx>
#include <frmatr.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>

using namespace objectpositioning;
using namespace ::com::sun::star;


SwToLayoutAnchoredObjectPosition::SwToLayoutAnchoredObjectPosition( SdrObject& _rDrawObj )
    : SwAnchoredObjectPosition( _rDrawObj ),
      maRelPos( Point() ),
      // --> OD 2004-06-17 #i26791#
      maOffsetToFrmAnchorPos( Point() )
{}

SwToLayoutAnchoredObjectPosition::~SwToLayoutAnchoredObjectPosition()
{}

/** calculate position for object position type TO_LAYOUT

    @author OD
*/
void SwToLayoutAnchoredObjectPosition::CalcPosition()
{
    const SwRect aObjBoundRect( GetAnchoredObj().GetObjRect() );

    SWRECTFN( (&GetAnchorFrm()) );

    const SwFrmFmt& rFrmFmt = GetFrmFmt();
    const SvxLRSpaceItem &rLR = rFrmFmt.GetLRSpace();
    const SvxULSpaceItem &rUL = rFrmFmt.GetULSpace();

    const bool bFlyAtFly = FLY_AT_FLY == rFrmFmt.GetAnchor().GetAnchorId();

    // determine position.
    // 'vertical' and 'horizontal' position are calculated separately
    Point aRelPos;

    // calculate 'vertical' position
    SwFmtVertOrient aVert( rFrmFmt.GetVertOrient() );
    {
        // to-frame anchored objects are *only* vertical positioned centered or
        // bottom, if its wrap mode is 'throught' and its anchor frame has fixed
        // size. Otherwise, it's positioned top.
        sal_Int16 eVertOrient = aVert.GetVertOrient();
        if ( ( bFlyAtFly &&
               ( eVertOrient == text::VertOrientation::CENTER ||
                 eVertOrient == text::VertOrientation::BOTTOM ) &&
             SURROUND_THROUGHT != rFrmFmt.GetSurround().GetSurround() &&
             !GetAnchorFrm().HasFixSize() ) )
        {
            eVertOrient = text::VertOrientation::TOP;
        }
        // --> OD 2004-06-17 #i26791# - get vertical offset to frame anchor position.
        SwTwips nVertOffsetToFrmAnchorPos( 0L );
        SwTwips nRelPosY =
                _GetVertRelPos( GetAnchorFrm(), GetAnchorFrm(), eVertOrient,
                                aVert.GetRelationOrient(), aVert.GetPos(),
                                rLR, rUL, nVertOffsetToFrmAnchorPos );


        // keep the calculated relative vertical position - needed for filters
        // (including the xml-filter)
        {
            SwTwips nAttrRelPosY = nRelPosY - nVertOffsetToFrmAnchorPos;
            if ( aVert.GetVertOrient() != text::VertOrientation::NONE &&
                 aVert.GetPos() != nAttrRelPosY )
            {
                aVert.SetPos( nAttrRelPosY );
                const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
                const_cast<SwFrmFmt&>(rFrmFmt).SetFmtAttr( aVert );
                const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();
            }
        }

        // determine absolute 'vertical' position, depending on layout-direction
        // --> OD 2004-06-17 #i26791# - determine offset to 'vertical' frame
        // anchor position, depending on layout-direction
        if( bVert )
        {
            OSL_ENSURE( !bRev, "<SwToLayoutAnchoredObjectPosition::CalcPosition()> - reverse layout set." );
            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
            if ( bVertL2R )
                   aRelPos.X() = nRelPosY;
            else
                   aRelPos.X() = -nRelPosY - aObjBoundRect.Width();
            maOffsetToFrmAnchorPos.X() = nVertOffsetToFrmAnchorPos;
        }
        else
        {
            aRelPos.Y() = nRelPosY;
            maOffsetToFrmAnchorPos.Y() = nVertOffsetToFrmAnchorPos;
        }

        // if in online-layout the bottom of to-page anchored object is beyond
        // the page bottom, the page frame has to grow by growing its body frame.
        if ( !bFlyAtFly && GetAnchorFrm().IsPageFrm() &&
             rFrmFmt.getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
        {
            const long nAnchorBottom = GetAnchorFrm().Frm().Bottom();
            const long nBottom = GetAnchorFrm().Frm().Top() +
                                 aRelPos.Y() + aObjBoundRect.Height();
            if ( nAnchorBottom < nBottom )
            {
                static_cast<SwPageFrm&>(GetAnchorFrm()).
                        FindBodyCont()->Grow( nBottom - nAnchorBottom );
            }
        }
    } // end of determination of vertical position

    // calculate 'horizontal' position
    SwFmtHoriOrient aHori( rFrmFmt.GetHoriOrient() );
    {
        // consider toggle of horizontal position for even pages.
        const bool bToggle = aHori.IsPosToggle() &&
                             !GetAnchorFrm().FindPageFrm()->OnRightPage();
        sal_Int16 eHoriOrient = aHori.GetHoriOrient();
        sal_Int16 eRelOrient = aHori.GetRelationOrient();
        // toggle orientation
        _ToggleHoriOrientAndAlign( bToggle, eHoriOrient, eRelOrient );

        // determine alignment values:
        // <nWidth>: 'width' of the alignment area
        // <nOffset>: offset of alignment area, relative to 'left' of
        //            frame anchor position
        SwTwips nWidth, nOffset;
        {
            bool bDummy; // in this context irrelevant output parameter
            _GetHoriAlignmentValues( GetAnchorFrm(), GetAnchorFrm(),
                                     eRelOrient, false,
                                     nWidth, nOffset, bDummy );
        }

        SwTwips nObjWidth = (aObjBoundRect.*fnRect->fnGetWidth)();

        // determine relative horizontal position
        SwTwips nRelPosX;
        if ( text::HoriOrientation::NONE == eHoriOrient )
        {
            if( bToggle ||
                ( !aHori.IsPosToggle() && GetAnchorFrm().IsRightToLeft() ) )
            {
                nRelPosX = nWidth - nObjWidth - aHori.GetPos();
            }
            else
            {
                nRelPosX = aHori.GetPos();
            }
        }
        else if ( text::HoriOrientation::CENTER == eHoriOrient )
            nRelPosX = (nWidth / 2) - (nObjWidth / 2);
        else if ( text::HoriOrientation::RIGHT == eHoriOrient )
            nRelPosX = nWidth - ( nObjWidth +
                             ( bVert ? rUL.GetLower() : rLR.GetRight() ) );
        else
            nRelPosX = bVert ? rUL.GetUpper() : rLR.GetLeft();
        nRelPosX += nOffset;

        // no 'negative' relative horizontal position
        // OD 06.11.2003 #FollowTextFlowAtFrame# - negative positions allow for
        // to frame anchored objects.
        if ( !bFlyAtFly && nRelPosX < 0 )
        {
            nRelPosX = 0;
        }

        // determine absolute 'horizontal' position, depending on layout-direction
        // --> OD 2004-06-17 #i26791# - determine offset to 'horizontal' frame
        // anchor position, depending on layout-direction
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        // --> OD 2009-09-04 #mongolianlayout#
        if( bVert || bVertL2R )
        // <--
        {

            aRelPos.Y() = nRelPosX;
            maOffsetToFrmAnchorPos.Y() = nOffset;
        }
        else
        {
            aRelPos.X() = nRelPosX;
            maOffsetToFrmAnchorPos.X() = nOffset;
        }

        // keep the calculated relative horizontal position - needed for filters
        // (including the xml-filter)
        {
            SwTwips nAttrRelPosX = nRelPosX - nOffset;
            if ( text::HoriOrientation::NONE != aHori.GetHoriOrient() &&
                 aHori.GetPos() != nAttrRelPosX )
            {
                aHori.SetPos( nAttrRelPosX );
                const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
                const_cast<SwFrmFmt&>(rFrmFmt).SetFmtAttr( aHori );
                const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();
            }
        }
    } // end of determination of horizontal position

    // keep calculate relative position
    maRelPos = aRelPos;
}

/** calculated relative position for object position

    @author OD
*/
Point SwToLayoutAnchoredObjectPosition::GetRelPos() const
{
    return maRelPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
