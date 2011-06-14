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
#include "precompiled_basctl.hxx"


#include "dlgedview.hxx"
#include "dlged.hxx"
#include <dlgedpage.hxx>

#include <svx/svxids.hrc>
#include <sfx2/viewfrm.hxx>

#include <basidesh.hxx>
#include <iderdll.hxx>
#include "dlgedobj.hxx"

TYPEINIT1( DlgEdView, SdrView );

//----------------------------------------------------------------------------

DlgEdView::DlgEdView( SdrModel* pModel, OutputDevice* pOut, DlgEditor* pEditor )
    :SdrView( pModel, pOut )
    ,pDlgEditor( pEditor )
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

    DlgEdHint aHint( DLGED_HINT_SELECTIONCHANGED );
    if ( pDlgEditor )
    {
        pDlgEditor->Broadcast( aHint );
        pDlgEditor->UpdatePropertyBrowserDelayed();
    }
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

        sal_Int32 nDeltaX = pDlgEditor->GetHScroll()->GetLineSize();
        sal_Int32 nDeltaY = pDlgEditor->GetVScroll()->GetLineSize();

        while ( rRect.Right() > nVisRight + nScrollX )
            nScrollX += nDeltaX;

        while ( rRect.Left() < nVisLeft + nScrollX )
            nScrollX -= nDeltaX;

        while ( rRect.Bottom() > nVisBottom + nScrollY )
            nScrollY += nDeltaY;

        while ( rRect.Top() < nVisTop + nScrollY )
            nScrollY -= nDeltaY;

        // don't scroll beyond the page size
        Size aPageSize = pDlgEditor->GetPage()->GetSize();
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
        if ( pDlgEditor )
            pDlgEditor->UpdateScrollBars();

        DlgEdHint aHint( DLGED_HINT_WINDOWSCROLLED );
        if ( pDlgEditor )
            pDlgEditor->Broadcast( aHint );
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

//----------------------------------------------------------------------------
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
