/*************************************************************************
 *
 *  $RCSfile: tolayoutanchoredobjectposition.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-28 13:43:27 $
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
#ifndef _TOLAYOUTANCHOREDOBJECTPOSITION_HXX
#include <tolayoutanchoredobjectposition.hxx>
#endif

#ifndef _FRAME_HXX
#include <frame.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _SVX_SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif

using namespace objectpositioning;

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
    bool bVertChgd = false;
    {
        // to-frame anchored objects are *only* vertical positioned centered or
        // bottom, if its wrap mode is 'throught' and its anchor frame has fixed
        // size. Otherwise, it's positioned top.
        SwVertOrient eVertOrient = aVert.GetVertOrient();
        if ( ( bFlyAtFly &&
               ( eVertOrient == VERT_CENTER ||
                 eVertOrient == VERT_BOTTOM ) &&
             SURROUND_THROUGHT != rFrmFmt.GetSurround().GetSurround() &&
             !GetAnchorFrm().HasFixSize() ) )
        {
            eVertOrient = VERT_TOP;
        }
        // --> OD 2004-06-17 #i26791# - get vertical offset to frame anchor position.
        SwTwips nVertOffsetToFrmAnchorPos( 0L );
        SwTwips nRelPosY =
                _GetVertRelPos( GetAnchorFrm(), GetAnchorFrm(), eVertOrient,
                                aVert.GetRelationOrient(), aVert.GetPos(),
                                rLR, rUL, nVertOffsetToFrmAnchorPos );


        // ??? Why saving calculated relative position
        // keep the calculated relative vertical position
        if ( aVert.GetVertOrient() != VERT_NONE &&
             aVert.GetPos() != nRelPosY )
        {
            aVert.SetPos( nRelPosY );
            bVertChgd = true;
        }

        // determine absolute 'vertical' position, depending on layout-direction
        // --> OD 2004-06-17 #i26791# - determine offset to 'vertical' frame
        // anchor position, depending on layout-direction
        if( bVert )
        {
            ASSERT( !bRev, "<SwToLayoutAnchoredObjectPosition::CalcPosition()> - reverse layout set." );
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
             rFrmFmt.GetDoc()->IsBrowseMode() )
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
    bool bHoriChgd = false;
    {
        // consider toggle of horizontal position for even pages.
        const bool bToggle = aHori.IsPosToggle() &&
                             !GetAnchorFrm().FindPageFrm()->OnRightPage();
        SwHoriOrient eHoriOrient = aHori.GetHoriOrient();
        SwRelationOrient eRelOrient = aHori.GetRelationOrient();
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
        if ( HORI_NONE == eHoriOrient )
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
        else if ( HORI_CENTER == eHoriOrient )
            nRelPosX = (nWidth / 2) - (nObjWidth / 2);
        else if ( HORI_RIGHT == eHoriOrient )
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
        if ( bVert )
        {
            aRelPos.Y() = nRelPosX;
            maOffsetToFrmAnchorPos.Y() = nOffset;
        }
        else
        {
            aRelPos.X() = nRelPosX;
            maOffsetToFrmAnchorPos.X() = nOffset;
        }

        // ??? Why saving calculated relative position
        // keep the calculated relative horizontal position
        if ( HORI_NONE != aHori.GetHoriOrient() &&
             aHori.GetPos() != nRelPosX )
        {
            aHori.SetPos( nRelPosX );
            bHoriChgd = true;
        }
    } // end of determination of horizontal position

    // keep calculate relative position
    maRelPos = aRelPos;

    // ??? Why saving calculated relative position
    // update attributes, if changed
    {
        const_cast<SwFrmFmt&>(rFrmFmt).LockModify();
        if ( bVertChgd )
            const_cast<SwFrmFmt&>(rFrmFmt).SetAttr( aVert );
        if ( bHoriChgd )
            const_cast<SwFrmFmt&>(rFrmFmt).SetAttr( aHori );
        const_cast<SwFrmFmt&>(rFrmFmt).UnlockModify();
    }
}

/** calculated relative position for object position

    @author OD
*/
Point SwToLayoutAnchoredObjectPosition::GetRelPos() const
{
    return maRelPos;
}

/** determined offset to frame anchor position

    --> OD 2004-06-17 #i26791#

    @author OD
*/
Point SwToLayoutAnchoredObjectPosition::GetOffsetToFrmAnchorPos() const
{
    return maOffsetToFrmAnchorPos;
}
