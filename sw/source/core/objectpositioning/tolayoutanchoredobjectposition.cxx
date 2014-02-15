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
#include "viewsh.hxx"
#include "viewopt.hxx"
#include "rootfrm.hxx"
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>

using namespace objectpositioning;
using namespace ::com::sun::star;


SwToLayoutAnchoredObjectPosition::SwToLayoutAnchoredObjectPosition( SdrObject& _rDrawObj )
    : SwAnchoredObjectPosition( _rDrawObj ),
      maRelPos( Point() ),
      // #i26791#
      maOffsetToFrmAnchorPos( Point() )
{}

SwToLayoutAnchoredObjectPosition::~SwToLayoutAnchoredObjectPosition()
{}

/** calculate position for object position type TO_LAYOUT */
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
        // #i26791# - get vertical offset to frame anchor position.
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
        // #i26791# - determine offset to 'vertical' frame
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
        const SwViewShell *pSh = GetAnchorFrm().getRootFrm()->GetCurrShell();
        if ( !bFlyAtFly && GetAnchorFrm().IsPageFrm() &&
             pSh && pSh->GetViewOptions()->getBrowseMode() )
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
        // #i26791# - determine offset to 'horizontal' frame
        // anchor position, depending on layout-direction
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        // --> OD 2009-09-04 #mongolianlayout#
        if( bVert || bVertL2R )
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

/** calculated relative position for object position */
Point SwToLayoutAnchoredObjectPosition::GetRelPos() const
{
    return maRelPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
