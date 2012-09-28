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

#include "dlgedview.hxx"
#include "dlged.hxx"
#include <dlgedpage.hxx>

#include <svx/svxids.hrc>
#include <sfx2/viewfrm.hxx>

#include <basidesh.hxx>
#include <iderdll.hxx>
#include "dlgedobj.hxx"

namespace basctl
{

TYPEINIT1( DlgEdView, SdrView );

//----------------------------------------------------------------------------

DlgEdView::DlgEdView (SdrModel& rModel, OutputDevice& rOut, DlgEditor& rEditor) :
    SdrView(&rModel, &rOut),
    rDlgEditor(rEditor)
{
    // #114898#
    SetBufferedOutputAllowed(true);
    SetBufferedOverlayAllowed(true);
}

//----------------------------------------------------------------------------

DlgEdView::~DlgEdView()
{
}

//----------------------------------------------------------------------------

void DlgEdView::MarkListHasChanged()
{
    SdrView::MarkListHasChanged();

    DlgEdHint aHint( DlgEdHint::SELECTIONCHANGED );
    rDlgEditor.Broadcast( aHint );
    rDlgEditor.UpdatePropertyBrowserDelayed();
}

//----------------------------------------------------------------------------

void DlgEdView::MakeVisible( const Rectangle& rRect, Window& rWin )
{
    // visible area
    MapMode aMap( rWin.GetMapMode() );
    Point aOrg( aMap.GetOrigin() );
    Size aVisSize( rWin.GetOutputSize() );
    Rectangle RectTmp( Point(-aOrg.X(),-aOrg.Y()), aVisSize );
    Rectangle aVisRect( RectTmp );

    // check, if rectangle is inside visible area
    if ( !aVisRect.IsInside( rRect ) )
    {
        // calculate scroll distance; the rectangle must be inside the visible area
        sal_Int32 nScrollX = 0, nScrollY = 0;

        sal_Int32 nVisLeft   = aVisRect.Left();
        sal_Int32 nVisRight  = aVisRect.Right();
        sal_Int32 nVisTop    = aVisRect.Top();
        sal_Int32 nVisBottom = aVisRect.Bottom();

        sal_Int32 nDeltaX = rDlgEditor.GetHScroll()->GetLineSize();
        sal_Int32 nDeltaY = rDlgEditor.GetVScroll()->GetLineSize();

        while ( rRect.Right() > nVisRight + nScrollX )
            nScrollX += nDeltaX;

        while ( rRect.Left() < nVisLeft + nScrollX )
            nScrollX -= nDeltaX;

        while ( rRect.Bottom() > nVisBottom + nScrollY )
            nScrollY += nDeltaY;

        while ( rRect.Top() < nVisTop + nScrollY )
            nScrollY -= nDeltaY;

        // don't scroll beyond the page size
        Size aPageSize = rDlgEditor.GetPage().GetSize();
        sal_Int32 nPageWidth  = aPageSize.Width();
        sal_Int32 nPageHeight = aPageSize.Height();

        if ( nVisRight + nScrollX > nPageWidth )
            nScrollX = nPageWidth - nVisRight;

        if ( nVisLeft + nScrollX < 0 )
            nScrollX = -nVisLeft;

        if ( nVisBottom + nScrollY > nPageHeight )
            nScrollY = nPageHeight - nVisBottom;

        if ( nVisTop + nScrollY < 0 )
            nScrollY = -nVisTop;

        // scroll window
        rWin.Update();
        rWin.Scroll( -nScrollX, -nScrollY );
        aMap.SetOrigin( Point( aOrg.X() - nScrollX, aOrg.Y() - nScrollY ) );
        rWin.SetMapMode( aMap );
        rWin.Update();
        rWin.Invalidate();

        // update scroll bars
        rDlgEditor.UpdateScrollBars();

        DlgEdHint aHint( DlgEdHint::WINDOWSCROLLED );
        rDlgEditor.Broadcast( aHint );
    }
}

//----------------------------------------------------------------------------

SdrObject* impLocalHitCorrection(SdrObject* pRetval, const Point& rPnt, sal_uInt16 nTol)
{
    DlgEdObj* pDlgEdObj = dynamic_cast< DlgEdObj* >(pRetval);

    if(pDlgEdObj)
    {
        bool bExcludeInner(false);

        if(0 != dynamic_cast< DlgEdForm* >(pRetval))
        {
            // from DlgEdForm::CheckHit; exclude inner for DlgEdForm
            bExcludeInner = true;
        }
        else if(pDlgEdObj->supportsService("com.sun.star.awt.UnoControlGroupBoxModel"))
        {
            // from DlgEdObj::CheckHit; exclude inner for group shapes
            bExcludeInner = true;
        }

        if(bExcludeInner)
        {
            // use direct model data; it's a DlgEdObj, so GetLastBoundRect()
            // will access aOutRect directly
            const Rectangle aOuterRectangle(pDlgEdObj->GetLastBoundRect());

            if(!aOuterRectangle.IsEmpty()
                && RECT_EMPTY != aOuterRectangle.Right()
                && RECT_EMPTY != aOuterRectangle.Bottom())
            {
                basegfx::B2DRange aOuterRange(
                    aOuterRectangle.Left(), aOuterRectangle.Top(),
                    aOuterRectangle.Right(), aOuterRectangle.Bottom());

                if(nTol)
                {
                    aOuterRange.grow(-1.0 * nTol);
                }

                if(aOuterRange.isInside(basegfx::B2DPoint(rPnt.X(), rPnt.Y())))
                {
                    pRetval = 0;
                }
            }
        }
    }

    return pRetval;
}

SdrObject* DlgEdView::CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, sal_uLong nOptions, const SetOfByte* pMVisLay) const
{
    // call parent
    SdrObject* pRetval = SdrView::CheckSingleSdrObjectHit(rPnt, nTol, pObj, pPV, nOptions, pMVisLay);

    if(pRetval)
    {
        // check hitted object locally
        pRetval = impLocalHitCorrection(pRetval, rPnt, nTol);
    }

    return pRetval;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
