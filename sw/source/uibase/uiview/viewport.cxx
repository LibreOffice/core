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

#include <config_features.h>

#include <hintids.hxx>

#include <vcl/help.hxx>
#include <vcl/settings.hxx>

#include <svx/ruler.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/lrspitem.hxx>
#include <sfx2/bindings.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <viewopt.hxx>
#include <frmatr.hxx>
#include <docsh.hxx>
#include <cmdid.h>
#include <edtwin.hxx>
#include <scroll.hxx>
#include <wview.hxx>
#include <usrpref.hxx>
#include <pagedesc.hxx>
#include <workctrl.hxx>

#include <PostItMgr.hxx>

#include <IDocumentSettingAccess.hxx>

#include <basegfx/utils/zoomtools.hxx>
#include <comphelper/lok.hxx>
#include <vcl/weld.hxx>

#include "viewfunc.hxx"

// The SetVisArea of the DocShell must not be called from InnerResizePixel.
// But our adjustments must take place.
static bool bProtectDocShellVisArea = false;

static sal_uInt16 nPgNum = 0;

bool SwView::IsDocumentBorder()
{
    if (GetDocShell()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED)
        return true;

    if (!m_pWrtShell)
        return false;

    return m_pWrtShell->GetViewOptions()->getBrowseMode() ||
           SvxZoomType::PAGEWIDTH_NOBORDER == m_pWrtShell->GetViewOptions()->GetZoomType();
}

static long GetLeftMargin( SwView const &rView )
{
    SvxZoomType eType = rView.GetWrtShell().GetViewOptions()->GetZoomType();
    long lRet = rView.GetWrtShell().GetAnyCurRect(CurRectType::PagePrt).Left();
    return eType == SvxZoomType::PERCENT   ? lRet + DOCUMENTBORDER :
           eType == SvxZoomType::PAGEWIDTH || eType == SvxZoomType::PAGEWIDTH_NOBORDER ? 0 :
                                         lRet + DOCUMENTBORDER + nLeftOfst;
}

static void lcl_GetPos(SwView const * pView,
                Point& rPos,
                SwScrollbar const * pScrollbar,
                bool bBorder)
{
    SwWrtShell &rSh = pView->GetWrtShell();
    const Size aDocSz( rSh.GetDocSize() );

    const long lBorder = bBorder ? DOCUMENTBORDER : DOCUMENTBORDER * 2;
    const bool bHori = pScrollbar->IsHoriScroll();

    const long lPos = pScrollbar->GetThumbPos() + (bBorder ? DOCUMENTBORDER : 0);

    long lDelta = lPos - (bHori ? rSh.VisArea().Pos().X() : rSh.VisArea().Pos().Y());

    const long lSize = (bHori ? aDocSz.Width() : aDocSz.Height()) + lBorder;
    // Should right or below are too much space,
    // then they must be subtracted out of the VisArea!
    long nTmp = pView->GetVisArea().Right()+lDelta;
    if ( bHori && nTmp > lSize )
        lDelta -= nTmp - lSize;
    nTmp = pView->GetVisArea().Bottom()+lDelta;
    if ( !bHori && nTmp > lSize )
        lDelta -= nTmp - lSize;

    bHori ? rPos.AdjustX(lDelta) : rPos.AdjustY(lDelta);
    if ( bBorder && (bHori ? rPos.X() : rPos.Y()) < DOCUMENTBORDER )
        bHori ? rPos.setX(DOCUMENTBORDER) : rPos.setY(DOCUMENTBORDER);
}

// Set zero ruler

void SwView::InvalidateRulerPos()
{
    static sal_uInt16 aInval[] =
    {
        SID_ATTR_PARA_LRSPACE, SID_RULER_BORDERS, SID_RULER_PAGE_POS,
        SID_RULER_LR_MIN_MAX, SID_ATTR_LONG_ULSPACE, SID_ATTR_LONG_LRSPACE,
        SID_RULER_BORDER_DISTANCE,
        SID_ATTR_PARA_LRSPACE_VERTICAL, SID_RULER_BORDERS_VERTICAL,
        SID_RULER_TEXT_RIGHT_TO_LEFT,
        SID_RULER_ROWS, SID_RULER_ROWS_VERTICAL, FN_STAT_PAGE,
        0
    };

    GetViewFrame()->GetBindings().Invalidate(aInval);

    assert(m_pHRuler && "Why is the ruler not there?");
    m_pHRuler->ForceUpdate();
    m_pVRuler->ForceUpdate();
}

// Limits the scrolling so far that only a quarter of the
// screen can be scrolled up before the end of the document.

long SwView::SetHScrollMax( long lMax )
{
    const long lBorder = IsDocumentBorder() ? DOCUMENTBORDER : DOCUMENTBORDER * 2;
    const long lSize = GetDocSz().Width() + lBorder - m_aVisArea.GetWidth();

    // At negative values the document is completely visible.
    // In this case, no scrolling.
    return std::max( std::min( lMax, lSize ), 0L );
}

long SwView::SetVScrollMax( long lMax )
{
    const long lBorder = IsDocumentBorder() ? DOCUMENTBORDER : DOCUMENTBORDER * 2;
    long lSize = GetDocSz().Height() + lBorder - m_aVisArea.GetHeight();
    return std::max( std::min( lMax, lSize), 0L );        // see horizontal
}

Point SwView::AlignToPixel(const Point &rPt) const
{
    return GetEditWin().PixelToLogic( GetEditWin().LogicToPixel( rPt ) );
}

// Document size has changed.

void SwView::DocSzChgd(const Size &rSz)
{
    m_aDocSz = rSz;

    if( !m_pWrtShell || m_aVisArea.IsEmpty() )      // no shell -> no change
    {
        bDocSzUpdated = false;
        return;
    }

    //If text has been deleted, it may be that the VisArea points behind the visible range.
    tools::Rectangle aNewVisArea( m_aVisArea );
    bool bModified = false;
    SwTwips lGreenOffset = IsDocumentBorder() ? DOCUMENTBORDER : DOCUMENTBORDER * 2;
    SwTwips lTmp = m_aDocSz.Width() + lGreenOffset;

    if ( aNewVisArea.Right() >= lTmp  )
    {
        lTmp = aNewVisArea.Right() - lTmp;
        aNewVisArea.AdjustRight( -lTmp );
        aNewVisArea.AdjustLeft( -lTmp );
        bModified = true;
    }

    lTmp = m_aDocSz.Height() + lGreenOffset;
    if ( aNewVisArea.Bottom() >= lTmp )
    {
        lTmp = aNewVisArea.Bottom() - lTmp;
        aNewVisArea.AdjustBottom( -lTmp );
        aNewVisArea.AdjustTop( -lTmp );
        bModified = true;
    }

    if ( bModified )
        SetVisArea( aNewVisArea, false );

    if ( UpdateScrollbars() && !m_bInOuterResizePixel && !m_bInInnerResizePixel &&
            !GetViewFrame()->GetFrame().IsInPlace())
        OuterResizePixel( Point(),
                          GetViewFrame()->GetWindow().GetOutputSizePixel() );
}

// Set VisArea newly

void SwView::SetVisArea( const tools::Rectangle &rRect, bool bUpdateScrollbar )
{
    Size aOldSz( m_aVisArea.GetSize() );
    if (comphelper::LibreOfficeKit::isActive() && m_pWrtShell)
        // If m_pWrtShell's visible area is the whole document, do the same here.
        aOldSz = m_pWrtShell->VisArea().SSize();

    if( rRect == m_aVisArea )
        return;

    const SwTwips lMin = IsDocumentBorder() ? DOCUMENTBORDER : 0;

    // No negative position, no negative size
    tools::Rectangle aLR = rRect;
    if( aLR.Top() < lMin )
    {
        aLR.AdjustBottom(lMin - aLR.Top() );
        aLR.SetTop( lMin );
    }
    if( aLR.Left() < lMin )
    {
        aLR.AdjustRight(lMin - aLR.Left() );
        aLR.SetLeft( lMin );
    }
    if( aLR.Right() < 0 )
        aLR.SetRight( 0 );
    if( aLR.Bottom() < 0 )
        aLR.SetBottom( 0 );

    if( aLR == m_aVisArea )
        return;

    const Size aSize( aLR.GetSize() );
    if( aSize.Width() < 0 || aSize.Height() < 0 )
        return;

    // Before the data can be changed, call an update if necessary. This
    // ensures that adjacent Paints in document coordinates are converted
    // correctly.
    // As a precaution, we do this only when an action is running in the
    // shell, because then it is not really drawn but the rectangles will
    // be only marked (in document coordinates).
    if ( m_pWrtShell && m_pWrtShell->ActionPend() )
        m_pWrtShell->GetWin()->Update();

    m_aVisArea = aLR;

    const bool bOuterResize = bUpdateScrollbar && UpdateScrollbars();

    if ( m_pWrtShell )
    {
        m_pWrtShell->VisPortChgd( m_aVisArea );
        if ( aOldSz != m_pWrtShell->VisArea().SSize() &&
             ( std::abs(aOldSz.Width() - m_pWrtShell->VisArea().Width()) > 2 ||
                std::abs(aOldSz.Height() - m_pWrtShell->VisArea().Height()) > 2 ) )
            m_pWrtShell->InvalidateLayout( false );
    }

    if ( !bProtectDocShellVisArea )
    {
        // If the size of VisArea is unchanged, we extend the size of the VisArea
        // InternalObject on. By that the transport of errors shall be avoided.
        tools::Rectangle aVis( m_aVisArea );
        if ( aVis.GetSize() == aOldSz )
            aVis.SetSize( GetDocShell()->SfxObjectShell::GetVisArea(ASPECT_CONTENT).GetSize() );
                    // TODO/LATER: why casting?!
                    //GetDocShell()->SfxInPlaceObject::GetVisArea().GetSize() );

        // With embedded always with modify...
        // TODO/LATER: why casting?!
        GetDocShell()->SfxObjectShell::SetVisArea( aVis );
        /*
        if ( GetDocShell()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
            GetDocShell()->SfxInPlaceObject::SetVisArea( aVis );
        else
            GetDocShell()->SvEmbeddedObject::SetVisArea( aVis );*/
    }

    SfxViewShell::VisAreaChanged();

    InvalidateRulerPos();

    if ( bOuterResize && !m_bInOuterResizePixel && !m_bInInnerResizePixel)
            OuterResizePixel( Point(),
                GetViewFrame()->GetWindow().GetOutputSizePixel() );
}

// Set Pos VisArea

void SwView::SetVisArea( const Point &rPt, bool bUpdateScrollbar )
{
    // Align once, so brushes will be inserted correctly.
    // This goes wrong in the BrowseView, because the entire document may
    // not be visible. Since the content in frames is fitting exactly,
    // align is not possible (better idea?!?!)
    // (fix: Bild.de, 200%) It does not work completely without alignment
    // Let's see how far we get with half BrushSize.
    Point aPt( rPt );
    aPt = GetEditWin().LogicToPixel( aPt );
#if HAVE_FEATURE_DESKTOP
    const long nTmp = GetWrtShell().IsFrameView() ? 4 : 8;
    aPt.AdjustX( -(aPt.X() % nTmp) );
    aPt.AdjustY( -(aPt.Y() % nTmp) );
#endif
    aPt = GetEditWin().PixelToLogic( aPt );

    if ( aPt == m_aVisArea.TopLeft() )
        return;

    const long lXDiff = m_aVisArea.Left() - aPt.X();
    const long lYDiff = m_aVisArea.Top()  - aPt.Y();
    SetVisArea( tools::Rectangle( aPt,
            Point( m_aVisArea.Right() - lXDiff, m_aVisArea.Bottom() - lYDiff ) ),
            bUpdateScrollbar);
}

void SwView::CheckVisArea()
{
    m_pHScrollbar->SetAuto( m_pWrtShell->GetViewOptions()->getBrowseMode() &&
                            !GetViewFrame()->GetFrame().IsInPlace() );
    if ( IsDocumentBorder() )
    {
        if ( m_aVisArea.Left() != DOCUMENTBORDER ||
             m_aVisArea.Top()  != DOCUMENTBORDER )
        {
            tools::Rectangle aNewVisArea( m_aVisArea );
            aNewVisArea.Move( DOCUMENTBORDER - m_aVisArea.Left(),
                              DOCUMENTBORDER - m_aVisArea.Top() );
            SetVisArea( aNewVisArea );
        }
    }
}

/// Calculate the visible range.

//  OUT Point *pPt:             new position of the visible area

//  IN  Rectangle &rRect:       Rectangle, which should be located
//                              within the new visible area.
//  sal_uInt16 nRange           optional accurate indication of the
//                              range by which to scroll if necessary.

void SwView::CalcPt( Point *pPt, const tools::Rectangle &rRect,
                     sal_uInt16 nRangeX, sal_uInt16 nRangeY)
{

    const SwTwips lMin = IsDocumentBorder() ? DOCUMENTBORDER : 0;

    long nYScroll = GetYScroll();
    long nDesHeight = rRect.GetHeight();
    long nCurHeight = m_aVisArea.GetHeight();
    nYScroll = std::min(nYScroll, nCurHeight - nDesHeight); // If it is scarce, then scroll not too much.
    if(nDesHeight > nCurHeight) // the height is not sufficient, then nYScroll is no longer of interest
    {
        pPt->setY( rRect.Top() );
        pPt->setY( std::max( lMin, pPt->Y() ) );
    }
    else if ( rRect.Top() < m_aVisArea.Top() )                // Upward shift
    {
        pPt->setY( rRect.Top() - (nRangeY != USHRT_MAX ? nRangeY : nYScroll) );
        pPt->setY( std::max( lMin, pPt->Y() ) );
    }
    else if( rRect.Bottom() > m_aVisArea.Bottom() )   // Downward shift
    {
        pPt->setY( rRect.Bottom() -
                    (m_aVisArea.GetHeight()) + ( nRangeY != USHRT_MAX ?
            nRangeY : nYScroll ) );
        pPt->setY( SetVScrollMax( pPt->Y() ) );
    }
    long nXScroll = GetXScroll();
    if ( rRect.Right() > m_aVisArea.Right() )         // Shift right
    {
        pPt->setX( rRect.Right()  -
                    (m_aVisArea.GetWidth()) +
                    (nRangeX != USHRT_MAX ? nRangeX : nXScroll) );
        pPt->setX( SetHScrollMax( pPt->X() ) );
    }
    else if ( rRect.Left() < m_aVisArea.Left() )      // Shift left
    {
        pPt->setX( rRect.Left() - (nRangeX != USHRT_MAX ? nRangeX : nXScroll) );
        pPt->setX( std::max( ::GetLeftMargin( *this ) + nLeftOfst, pPt->X() ) );
        pPt->setX( std::min( rRect.Left() - nScrollX, pPt->X() ) );
        pPt->setX( std::max( 0L, pPt->X() ) );
    }
}

// Scrolling

bool SwView::IsScroll( const tools::Rectangle &rRect ) const
{
    return m_bCenterCursor || m_bTopCursor || !m_aVisArea.IsInside(rRect);
}

void SwView::Scroll( const tools::Rectangle &rRect, sal_uInt16 nRangeX, sal_uInt16 nRangeY )
{
    if ( m_aVisArea.IsEmpty() )
        return;

    tools::Rectangle aOldVisArea( m_aVisArea );
    long nDiffY = 0;

    vcl::Window* pCareWn = SwViewShell::GetCareWin();
    weld::Window* pCareDialog = SwViewShell::GetCareDialog(GetWrtShell());
    if (pCareWn || pCareDialog)
    {
        int x, y, width, height;
        tools::Rectangle aDlgRect;
        if (pCareWn)
        {
            aDlgRect = GetEditWin().PixelToLogic(pCareWn->GetWindowExtentsRelative(&GetEditWin()));
        }
        else if (pCareDialog && pCareDialog->get_extents_relative_to(*GetEditWin().GetFrameWeld(), x, y, width, height))
        {
            Point aTopLeft(GetEditWin().GetSystemWindow()->OutputToAbsoluteScreenPixel(Point(x, y)));
            aTopLeft = GetEditWin().AbsoluteScreenToOutputPixel(aTopLeft);
            aDlgRect = GetEditWin().PixelToLogic(tools::Rectangle(aTopLeft, Size(width, height)));
        }

        // Only if the dialogue is not the VisArea right or left:
        if ( aDlgRect.Left() < m_aVisArea.Right() &&
             aDlgRect.Right() > m_aVisArea.Left() )
        {
            // If we are not supposed to be centered, lying in the VisArea
            // and are not covered by the dialogue ...
            if ( !m_bCenterCursor && aOldVisArea.IsInside( rRect )
                 && ( rRect.Left() > aDlgRect.Right()
                      || rRect.Right() < aDlgRect.Left()
                      || rRect.Top() > aDlgRect.Bottom()
                      || rRect.Bottom() < aDlgRect.Top() ) )
                return;

            // Is above or below the dialogue more space?
            long nTopDiff = aDlgRect.Top() - m_aVisArea.Top();
            long nBottomDiff = m_aVisArea.Bottom() - aDlgRect.Bottom();
            if ( nTopDiff < nBottomDiff )
            {
                if ( nBottomDiff > 0 ) // Is there room below at all?
                {   // then we move the upper edge and we remember this
                    nDiffY = aDlgRect.Bottom() - m_aVisArea.Top();
                    m_aVisArea.AdjustTop(nDiffY );
                }
            }
            else
            {
                if ( nTopDiff > 0 ) // Is there room below at all?
                    m_aVisArea.SetBottom( aDlgRect.Top() ); // Modify the lower edge
            }
        }
    }

    //s.o. !IsScroll()
    if( !(m_bCenterCursor || m_bTopCursor) && m_aVisArea.IsInside( rRect ) )
    {
        m_aVisArea = aOldVisArea;
        return;
    }
    // If the rectangle is larger than the visible area -->
    // upper left corner
    Size aSize( rRect.GetSize() );
    const Size aVisSize( m_aVisArea.GetSize() );
    if( !m_aVisArea.IsEmpty() && (
        aSize.Width() + GetXScroll() > aVisSize.Width() ||
        aSize.Height()+ GetYScroll() > aVisSize.Height() ))
    {
        Point aPt( m_aVisArea.TopLeft() );
        aSize.setWidth( std::min( aSize.Width(), aVisSize.Width() ) );
        aSize.setHeight( std::min( aSize.Height(),aVisSize.Height()) );

        CalcPt( &aPt, tools::Rectangle( rRect.TopLeft(), aSize ),
                static_cast< sal_uInt16 >((aVisSize.Width() - aSize.Width()) / 2),
                static_cast< sal_uInt16 >((aVisSize.Height()- aSize.Height())/ 2) );

        if( m_bTopCursor )
        {
            const long nBorder = IsDocumentBorder() ? DOCUMENTBORDER : 0;
            aPt.setY( std::min( std::max( nBorder, rRect.Top() ),
                                m_aDocSz.Height() + nBorder -
                                    m_aVisArea.GetHeight() ) );
        }
        aPt.AdjustY( -nDiffY );
        m_aVisArea = aOldVisArea;
        SetVisArea( aPt );
        return;
    }
    if( !m_bCenterCursor )
    {
        Point aPt( m_aVisArea.TopLeft() );
        CalcPt( &aPt, rRect, nRangeX, nRangeY );

        if( m_bTopCursor )
        {
            const long nBorder = IsDocumentBorder() ? DOCUMENTBORDER : 0;
            aPt.setY( std::min( std::max( nBorder, rRect.Top() ),
                                m_aDocSz.Height() + nBorder -
                                    m_aVisArea.GetHeight() ) );
        }

        aPt.AdjustY( -nDiffY );
        m_aVisArea = aOldVisArea;
        SetVisArea( aPt );
        return;
    }

    //Center cursor
    Point aPnt( m_aVisArea.TopLeft() );
    // ... in Y-direction in any case
    aPnt.AdjustY(( rRect.Top() + rRect.Bottom()
                  - m_aVisArea.Top() - m_aVisArea.Bottom() ) / 2 - nDiffY );
    // ... in X-direction, only if the rectangle protrudes over the right or left of the VisArea.
    if ( rRect.Right() > m_aVisArea.Right() || rRect.Left() < m_aVisArea.Left() )
    {
        aPnt.AdjustX(( rRect.Left() + rRect.Right()
                  - m_aVisArea.Left() - m_aVisArea.Right() ) / 2 );
        aPnt.setX( SetHScrollMax( aPnt.X() ) );
        const SwTwips lMin = IsDocumentBorder() ? DOCUMENTBORDER : 0;
        aPnt.setX( std::max( (GetLeftMargin( *this ) - lMin) + nLeftOfst, aPnt.X() ) );
    }
    m_aVisArea = aOldVisArea;
    if (pCareWn || pCareDialog)
    {   // If we want to avoid only a dialogue, we do
        // not want to go beyond the end of the document.
        aPnt.setY( SetVScrollMax( aPnt.Y() ) );
    }
    SetVisArea( aPnt );
}

/// Scroll page by page
//  Returns the value by which to be scrolled with PageUp / Down

bool SwView::GetPageScrollUpOffset( SwTwips &rOff ) const
{
    // in the LOK case, force the value set by the API
    if (comphelper::LibreOfficeKit::isActive() && m_nLOKPageUpDownOffset > 0)
    {
        rOff = -m_nLOKPageUpDownOffset;
        return true;
    }

    if ( !m_aVisArea.Top() || !m_aVisArea.GetHeight() )
        return false;
    long nYScrl = GetYScroll() / 2;
    rOff = -(m_aVisArea.GetHeight() - nYScrl);
    // Do not scroll before the beginning of the document.
    if( m_aVisArea.Top() - rOff < 0 )
        rOff = rOff - m_aVisArea.Top();
    else if( GetWrtShell().GetCharRect().Top() < (m_aVisArea.Top() + nYScrl))
        rOff += nYScrl;

    return true;
}

bool SwView::GetPageScrollDownOffset( SwTwips &rOff ) const
{
    // in the LOK case, force the value set by the API
    if (comphelper::LibreOfficeKit::isActive() && m_nLOKPageUpDownOffset > 0)
    {
        rOff = m_nLOKPageUpDownOffset;
        return true;
    }

    if ( !m_aVisArea.GetHeight() ||
         (m_aVisArea.GetHeight() > m_aDocSz.Height()) )
        return false;
    long nYScrl = GetYScroll() / 2;
    rOff = m_aVisArea.GetHeight() - nYScrl;
    // Do not scroll past the end of the document.
    if ( m_aVisArea.Top() + rOff > m_aDocSz.Height() )
        rOff = m_aDocSz.Height() - m_aVisArea.Bottom();
    else if( GetWrtShell().GetCharRect().Bottom() >
                                            ( m_aVisArea.Bottom() - nYScrl ))
        rOff -= nYScrl;

    return rOff > 0;
}

// Scroll page by page
bool SwView::PageUp()
{
    if (!m_aVisArea.GetHeight())
        return false;

    Point aPos(m_aVisArea.TopLeft());
    aPos.AdjustY( -(m_aVisArea.GetHeight() - (GetYScroll() / 2)) );
    aPos.setY( std::max(0L, aPos.Y()) );
    SetVisArea( aPos );
    return true;
}

bool SwView::PageDown()
{
    if ( !m_aVisArea.GetHeight() )
        return false;
    Point aPos( m_aVisArea.TopLeft() );
    aPos.AdjustY(m_aVisArea.GetHeight() - (GetYScroll() / 2) );
    aPos.setY( SetVScrollMax( aPos.Y() ) );
    SetVisArea( aPos );
    return true;
}

void SwView::PhyPageUp()
{
    // Check for the currently visible page, do not format
    sal_uInt16 nActPage = m_pWrtShell->GetNextPrevPageNum( false );

    if( USHRT_MAX != nActPage )
    {
        const Point aPt( m_aVisArea.Left(),
                         m_pWrtShell->GetPagePos( nActPage ).Y() );
        Point aAlPt( AlignToPixel( aPt ) );
        // If there is a difference, has been truncated --> then add one pixel,
        // so that no residue of the previous page is visible.
        if( aPt.Y() != aAlPt.Y() )
            aAlPt.AdjustY(3 * GetEditWin().PixelToLogic( Size( 0, 1 ) ).Height() );
        SetVisArea( aAlPt );
    }
}

void SwView::PhyPageDown()
{
    // Check for the currently visible page, do not format
    sal_uInt16 nActPage = m_pWrtShell->GetNextPrevPageNum();
    // If the last page of the document is visible, do nothing.
    if( USHRT_MAX != nActPage )
    {
        const Point aPt( m_aVisArea.Left(),
                         m_pWrtShell->GetPagePos( nActPage ).Y() );
        Point aAlPt( AlignToPixel( aPt ) );
        // If there is a difference, has been truncated --> then add one pixel,
        // so that no residue of the previous page is visible.
        if( aPt.Y() != aAlPt.Y() )
            aAlPt.AdjustY(3 * GetEditWin().PixelToLogic( Size( 0, 1 ) ).Height() );
        SetVisArea( aAlPt );
    }
}

bool SwView::PageUpCursor( bool bSelect )
{
    if ( !bSelect )
    {
        const FrameTypeFlags eType = m_pWrtShell->GetFrameType(nullptr,true);
        if ( eType & FrameTypeFlags::FOOTNOTE )
        {
            m_pWrtShell->MoveCursor();
            m_pWrtShell->GotoFootnoteAnchor();
            m_pWrtShell->Right(CRSR_SKIP_CHARS, false, 1, false );
            return true;
        }
    }

    SwTwips lOff = 0;
    if ( GetPageScrollUpOffset( lOff ) &&
         (m_pWrtShell->IsCursorReadonly() ||
          !m_pWrtShell->PageCursor( lOff, bSelect )) &&
         PageUp() )
    {
        m_pWrtShell->ResetCursorStack();
        return true;
    }
    return false;
}

bool SwView::PageDownCursor(bool bSelect)
{
    SwTwips lOff = 0;
    if ( GetPageScrollDownOffset( lOff ) &&
         (m_pWrtShell->IsCursorReadonly() ||
          !m_pWrtShell->PageCursor( lOff, bSelect )) &&
         PageDown() )
    {
        m_pWrtShell->ResetCursorStack();
        return true;
    }
    return false;
}

// Handler of the scrollbars

IMPL_LINK( SwView, ScrollHdl, ScrollBar *, p, void )
{
    SwScrollbar* pScrollbar = static_cast<SwScrollbar*>(p);
    if ( GetWrtShell().ActionPend() )
        return;

    if ( pScrollbar->GetType() == ScrollType::Drag )
        m_pWrtShell->EnableSmooth( false );

    if(!m_pWrtShell->GetViewOptions()->getBrowseMode() &&
        pScrollbar->GetType() == ScrollType::Drag)
    {
        // Here comment out again if it is not desired to scroll together:
        // The end scrollhandler invalidate the FN_STAT_PAGE,
        // so we don't must do it again.
        EndScrollHdl(pScrollbar);

        if ( !m_bWheelScrollInProgress && Help::IsQuickHelpEnabled() &&
             m_pWrtShell->GetViewOptions()->IsShowScrollBarTips())
        {

            Point aPos( m_aVisArea.TopLeft() );
            lcl_GetPos(this, aPos, pScrollbar, IsDocumentBorder());

            sal_uInt16 nPhNum = 1;
            sal_uInt16 nVirtNum = 1;

            OUString sDisplay;
            if(m_pWrtShell->GetPageNumber( aPos.Y(), false, nPhNum, nVirtNum, sDisplay ))
            {
                // The end scrollhandler invalidate the FN_STAT_PAGE,
                // so we don't must do it again.
        //      if(!GetViewFrame()->GetFrame().IsInPlace())
        //            S F X_BINDINGS().Update(FN_STAT_PAGE);

                //QuickHelp:
                if( m_pWrtShell->GetPageCnt() > 1 )
                {
                    tools::Rectangle aRect;
                    aRect.SetLeft( pScrollbar->GetParent()->OutputToScreenPixel(
                                        pScrollbar->GetPosPixel() ).X() -8 );
                    aRect.SetTop( pScrollbar->OutputToScreenPixel(
                                    pScrollbar->GetPointerPosPixel() ).Y() );
                    aRect.SetRight( aRect.Left() );
                    aRect.SetBottom( aRect.Top() );

                    OUString sPageStr( GetPageStr( nPhNum, nVirtNum, sDisplay ));
                    SwContentAtPos aCnt( IsAttrAtPos::Outline );
                    bool bSuccess = m_pWrtShell->GetContentAtPos(aPos, aCnt);
                    if (bSuccess && !aCnt.sStr.isEmpty())
                    {
                        sPageStr += "  - ";
                        sal_Int32 nChunkLen = std::min<sal_Int32>(aCnt.sStr.getLength(), 80);
                        OUString sChunk = aCnt.sStr.copy(0, nChunkLen);
                        sPageStr = sChunk + sPageStr;
                        sPageStr = sPageStr.replace('\t', ' ');
                        sPageStr = sPageStr.replace(0x0a, ' ');
                    }
                    nPgNum = nPhNum;
                }
            }
        }
    }
    else
        EndScrollHdl(pScrollbar);

    if ( pScrollbar->GetType() == ScrollType::Drag )
        m_pWrtShell->EnableSmooth( true );
}

// Handler of the scrollbars

IMPL_LINK( SwView, EndScrollHdl, ScrollBar *, p, void )
{
    SwScrollbar* pScrollbar = static_cast<SwScrollbar*>(p);
    if ( !GetWrtShell().ActionPend() )
    {
        if(nPgNum)
        {
            nPgNum = 0;
            Help::ShowQuickHelp(pScrollbar, tools::Rectangle(), OUString());
        }
        Point aPos( m_aVisArea.TopLeft() );
        bool bBorder = IsDocumentBorder();
        lcl_GetPos(this, aPos, pScrollbar, bBorder);
        if ( bBorder && aPos == m_aVisArea.TopLeft() )
            UpdateScrollbars();
        else
            SetVisArea( aPos, false );

        GetViewFrame()->GetBindings().Update(FN_STAT_PAGE);
    }
}

// Calculates the size of the m_aVisArea in dependency of the size of
// EditWin on the screen.

void SwView::CalcVisArea( const Size &rOutPixel )
{
    Point aTopLeft;
    tools::Rectangle aRect( aTopLeft, rOutPixel );
    aTopLeft = GetEditWin().PixelToLogic( aTopLeft );
    Point aBottomRight( GetEditWin().PixelToLogic( aRect.BottomRight() ) );

    aRect.SetLeft( aTopLeft.X() );
    aRect.SetTop( aTopLeft.Y() );
    aRect.SetRight( aBottomRight.X() );
    aRect.SetBottom( aBottomRight.Y() );

    // The shifts to the right and/or below can now be incorrect
    // (e.g. change zoom level, change view size).
    const long lBorder = IsDocumentBorder() ? DOCUMENTBORDER : DOCUMENTBORDER*2;
    if ( aRect.Left() )
    {
        const long lWidth = GetWrtShell().GetDocSize().Width() + lBorder;
        if ( aRect.Right() > lWidth )
        {
            long lDelta    = aRect.Right() - lWidth;
            aRect.AdjustLeft( -lDelta );
            aRect.AdjustRight( -lDelta );
        }
    }
    if ( aRect.Top() )
    {
        const long lHeight = GetWrtShell().GetDocSize().Height() + lBorder;
        if ( aRect.Bottom() > lHeight )
        {
            long lDelta     = aRect.Bottom() - lHeight;
            aRect.AdjustTop( -lDelta );
            aRect.AdjustBottom( -lDelta );
        }
    }
    SetVisArea( aRect );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOM );
    GetViewFrame()->GetBindings().Invalidate( SID_ATTR_ZOOMSLIDER ); // for snapping points
}

// Rearrange control elements

void SwView::CalcAndSetBorderPixel( SvBorder &rToFill )
{
    bool bRightVRuler = m_pWrtShell->GetViewOptions()->IsVRulerRight();
    if ( m_pVRuler->IsVisible() )
    {
        long nWidth = m_pVRuler->GetSizePixel().Width();
        if(bRightVRuler)
            rToFill.Right() = nWidth;
        else
            rToFill.Left() = nWidth;
    }

    OSL_ENSURE(m_pHRuler, "Why is the ruler not present?");
    if ( m_pHRuler->IsVisible() )
        rToFill.Top() = m_pHRuler->GetSizePixel().Height();

    const StyleSettings &rSet = GetEditWin().GetSettings().GetStyleSettings();
    const long nTmp = rSet.GetScrollBarSize();
    if( m_pVScrollbar->IsVisible(true) )
    {
        if(bRightVRuler)
            rToFill.Left() = nTmp;
        else
            rToFill.Right()  = nTmp;
    }
    if ( m_pHScrollbar->IsVisible(true) )
        rToFill.Bottom() = nTmp;

    SetBorderPixel( rToFill );
}

void ViewResizePixel( const vcl::RenderContext &rRef,
                    const Point &rOfst,
                    const Size &rSize,
                    const Size &rEditSz,
                    SwScrollbar& rVScrollbar,
                    SwScrollbar& rHScrollbar,
                    vcl::Window& rScrollBarBox,
                    SvxRuler* pVRuler,
                    SvxRuler* pHRuler,
                    bool bVRulerRight )
{
// ViewResizePixel is also used by Preview!!!

    const bool bHRuler = pHRuler && pHRuler->IsVisible();
    const long nHLinSzHeight = bHRuler ?
                        pHRuler->GetSizePixel().Height() : 0;
    const bool bVRuler = pVRuler && pVRuler->IsVisible();
    const long nVLinSzWidth = bVRuler ?
                        pVRuler->GetSizePixel().Width() : 0;

    const long nScrollBarSize = rRef.GetSettings().GetStyleSettings().GetScrollBarSize();
    const long nHBSzHeight = rHScrollbar.IsVisible(true) ? nScrollBarSize : 0;
    const long nVBSzWidth = rVScrollbar.IsVisible(true) ? nScrollBarSize : 0;

    if(pVRuler)
    {
        WinBits nStyle = pVRuler->GetStyle()&~WB_RIGHT_ALIGNED;
        Point aPos( rOfst.X(), rOfst.Y()+nHLinSzHeight );
        if(bVRulerRight)
        {
            aPos.AdjustX(rSize.Width() - nVLinSzWidth );
            nStyle |= WB_RIGHT_ALIGNED;
        }
        Size  aSize( nVLinSzWidth, rEditSz.Height() );
        if(!aSize.Width())
            aSize.setWidth( pVRuler->GetSizePixel().Width() );
        pVRuler->SetStyle(nStyle);
        pVRuler->SetPosSizePixel( aPos, aSize );
        if(!pVRuler->IsVisible())
            pVRuler->Resize();
    }
    // Ruler needs a resize, otherwise it will not work in the invisible condition
    if(pHRuler)
    {
        Size aSize( rSize.Width(), nHLinSzHeight );
        if ( nVBSzWidth && !bVRulerRight)
            aSize.AdjustWidth( -nVBSzWidth );
        if(!aSize.Height())
            aSize.setHeight( pHRuler->GetSizePixel().Height() );
        pHRuler->SetPosSizePixel( rOfst, aSize );
        // VCL calls no resize on invisible windows
        // but that is not a good idea for the ruler
        if(!pHRuler->IsVisible())
            pHRuler->Resize();
    }

    // Arrange scrollbars and SizeBox
    Point aScrollFillPos;
    {
        Point aPos( rOfst.X(),
                    rOfst.Y()+rSize.Height()-nHBSzHeight );
        if(bVRulerRight)
        {
            aPos.AdjustX(nVBSzWidth );
        }

        Size  aSize( rSize.Width(), nHBSzHeight );
        if ( nVBSzWidth )
            aSize.AdjustWidth( -nVBSzWidth );
        rHScrollbar.SetPosSizePixel( aPos, aSize );
        aScrollFillPos.setY( aPos.Y() );
    }
    {
        Point aPos( rOfst.X()+rSize.Width()-nVBSzWidth,
                    rOfst.Y() );
        Size  aSize( nVBSzWidth, rSize.Height() );
        if(bVRulerRight)
        {
            aPos.setX( rOfst.X() );
            if(bHRuler)
            {
                aPos.AdjustY(nHLinSzHeight );
                aSize.AdjustHeight( -nHLinSzHeight );
            }
        }

        if ( nHBSzHeight )
            aSize.AdjustHeight( -nHBSzHeight );
        rVScrollbar.SetPosSizePixel( aPos, aSize );

        aPos.AdjustY(aSize.Height() );

        aScrollFillPos.setX( aPos.X() );
    }

    rScrollBarBox.SetPosSizePixel(aScrollFillPos, Size(nVBSzWidth, nHBSzHeight));
}

void SwView::ShowAtResize()
{
    m_bShowAtResize = false;
    if ( m_pWrtShell->GetViewOptions()->IsViewHRuler() )
        m_pHRuler->Show();
}

void SwView::InnerResizePixel( const Point &rOfst, const Size &rSize, bool )
{
    Size aObjSize = GetObjectShell()->GetVisArea().GetSize();
    if ( aObjSize.Width() > 0 && aObjSize.Height() > 0 )
    {
        SvBorder aBorder( GetBorderPixel() );
        Size aSize( rSize );
        aSize.AdjustWidth( -(aBorder.Left() + aBorder.Right()) );
        aSize.AdjustHeight( -(aBorder.Top() + aBorder.Bottom()) );
        Size aObjSizePixel = GetWindow()->LogicToPixel(aObjSize, MapMode(MapUnit::MapTwip));
        SfxViewShell::SetZoomFactor( Fraction( aSize.Width(), aObjSizePixel.Width() ),
                        Fraction( aSize.Height(), aObjSizePixel.Height() ) );
    }

    m_bInInnerResizePixel = true;
    const bool bHScrollVisible = m_pHScrollbar->IsVisible(true);
    const bool bVScrollVisible = m_pVScrollbar->IsVisible(true);
    bool bRepeat = false;
    do
    {
        Size aSz( rSize );
        SvBorder aBorder;
        CalcAndSetBorderPixel( aBorder );
        if ( GetViewFrame()->GetFrame().IsInPlace() )
        {
            Size aViewSize( aSz );
            Point aViewPos( rOfst );
            aViewSize.AdjustHeight( -(aBorder.Top() + aBorder.Bottom()) );
            aViewSize.AdjustWidth( -(aBorder.Left() + aBorder.Right()) );
            aViewPos.AdjustX(aBorder.Left() );
            aViewPos.AdjustY(aBorder.Top() );
            GetEditWin().SetPosSizePixel( aViewPos, aViewSize );
        }
        else
        {
            aSz.AdjustHeight(aBorder.Top()  + aBorder.Bottom() );
            aSz.AdjustWidth(aBorder.Left() + aBorder.Right() );
        }

        Size aEditSz( GetEditWin().GetOutputSizePixel() );
        ViewResizePixel( GetEditWin(), rOfst, aSz, aEditSz, *m_pVScrollbar,
                            *m_pHScrollbar, *m_pScrollFill, m_pVRuler, m_pHRuler,
                            m_pWrtShell->GetViewOptions()->IsVRulerRight());
        if ( m_bShowAtResize )
            ShowAtResize();

        if( m_pHRuler->IsVisible() || m_pVRuler->IsVisible() )
        {
            const Fraction& rFrac = GetEditWin().GetMapMode().GetScaleX();
            long nZoom = 100;
            if (rFrac.IsValid())
                nZoom = long(rFrac * 100);

            const Fraction aFrac( nZoom, 100 );
            m_pVRuler->SetZoom( aFrac );
            m_pHRuler->SetZoom( aFrac );
            InvalidateRulerPos();   // Invalidate content.
        }
        // Reset the cursor stack because the cursor positions for PageUp/Down
        // no longer fit the currently visible area.
        m_pWrtShell->ResetCursorStack();

        // EditWin never set!

        // Set VisArea, but do not call the SetVisArea of the Docshell there!
        bProtectDocShellVisArea = true;
        CalcVisArea( aEditSz );
        // Visibility changes of the automatic horizontal scrollbar
        // require to repeat the ViewResizePixel() call - but only once!
        if(bRepeat)
            bRepeat = false;
        else if(bHScrollVisible != m_pHScrollbar->IsVisible(true) ||
                bVScrollVisible != m_pVScrollbar->IsVisible(true))
            bRepeat = true;
    }while( bRepeat );
    bProtectDocShellVisArea = false;
    m_bInInnerResizePixel = false;
}

void SwView::OuterResizePixel( const Point &rOfst, const Size &rSize )
{
    // #i16909# return, if no size (caused by minimize window).
    if ( m_bInOuterResizePixel || ( !rSize.Width() && !rSize.Height() ) )
        return;
    m_bInOuterResizePixel = true;

    // Determine whether scroll bars may be displayed.
    bool bShowH = true,
         bShowV = true,
         bAuto  = true,
         bHAuto = true;

    const SwViewOption *pVOpt = m_pWrtShell->GetViewOptions();
    if ( !pVOpt->IsReadonly() || pVOpt->IsStarOneSetting() )
    {
        bShowH = pVOpt->IsViewHScrollBar();
        bShowV = pVOpt->IsViewVScrollBar();
    }

    if (!m_bHScrollbarEnabled)
    {
        bHAuto = bShowH = false;
    }
    if (!m_bVScrollbarEnabled)
    {
        bAuto = bShowV = false;
    }

    SwDocShell* pDocSh = GetDocShell();
    bool bIsPreview = pDocSh->IsPreview();
    if( bIsPreview )
    {
        bShowH = bShowV = bHAuto = bAuto = false;
    }
    if(m_pHScrollbar->IsVisible(false) != bShowH && !bHAuto)
        ShowHScrollbar(bShowH);
    m_pHScrollbar->SetAuto( bHAuto );
    if(m_pVScrollbar->IsVisible(false) != bShowV && !bAuto)
        ShowVScrollbar(bShowV);
    m_pVScrollbar->SetAuto(bAuto);

    SET_CURR_SHELL( m_pWrtShell.get() );
    bool bRepeat = false;
    long nCnt = 0;

    bool bUnLockView = !m_pWrtShell->IsViewLocked();
    m_pWrtShell->LockView( true );
    m_pWrtShell->LockPaint();

    do {
        ++nCnt;
        const bool bScroll1 = m_pVScrollbar->IsVisible(true);
        const bool bScroll2 = m_pHScrollbar->IsVisible(true);
        SvBorder aBorder;
        CalcAndSetBorderPixel( aBorder );
        const Size aEditSz( GetEditWin().GetOutputSizePixel() );
        ViewResizePixel( GetEditWin(), rOfst, rSize, aEditSz, *m_pVScrollbar,
                                *m_pHScrollbar, *m_pScrollFill, m_pVRuler, m_pHRuler,
                                m_pWrtShell->GetViewOptions()->IsVRulerRight() );
        if ( m_bShowAtResize )
            ShowAtResize();

        if( m_pHRuler->IsVisible() || m_pVRuler->IsVisible() )
            InvalidateRulerPos();   // Invalidate content.

        // Reset the cursor stack because the cursor positions for PageUp/Down
        // no longer fit the currently visible area.
        m_pWrtShell->ResetCursorStack();

        OSL_ENSURE( !GetEditWin().IsVisible() ||
                    (( aEditSz.Width() > 0 && aEditSz.Height() > 0 )
                        || !m_aVisArea.IsEmpty()), "Small world, isn't it?" );

        // Never set EditWin!

        // Of course the VisArea must also be set.
        // Now is the right time to re-calculate the zoom if it is not a simple factor.
        m_pWrtShell->StartAction();
        CalcVisArea( aEditSz );

        //Thus also in the outplace editing the page width will be adjusted immediately.
        //TODO/LATER: is that still necessary?!
        /*
        if ( pDocSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED )
            pDocSh->SetVisArea(
                            pDocSh->SfxInPlaceObject::GetVisArea() );*/
        if ( m_pWrtShell->GetViewOptions()->GetZoomType() != SvxZoomType::PERCENT &&
             !m_pWrtShell->GetViewOptions()->getBrowseMode() )
            SetZoom_( aEditSz, m_pWrtShell->GetViewOptions()->GetZoomType(), 100, true );
        m_pWrtShell->EndAction();

        bRepeat = bScroll1 != m_pVScrollbar->IsVisible(true);
        if ( !bRepeat )
            bRepeat = bScroll2 != m_pHScrollbar->IsVisible(true);

        // Do no infinite loops.
        // If possible stop when the (auto-) scroll bars are visible.
        if ( bRepeat &&
             ( nCnt > 10 || ( nCnt > 3 && bHAuto && bAuto ) )
           )
        {
            bRepeat = false;
        }

    } while ( bRepeat );

    m_pWrtShell->UnlockPaint();
    if( bUnLockView )
        m_pWrtShell->LockView( false );

    m_bInOuterResizePixel = false;

    if ( m_pPostItMgr )
    {
        m_pPostItMgr->CalcRects();
        m_pPostItMgr->LayoutPostIts();
    }
}

void SwView::SetZoomFactor( const Fraction &rX, const Fraction &rY )
{
    const Fraction &rFrac = rX < rY ? rX : rY;
    SetZoom( SvxZoomType::PERCENT, static_cast<short>(long(rFrac * Fraction( 100, 1 ))) );

    // To minimize rounding errors we also adjust the odd values
    // of the base class if necessary.
    SfxViewShell::SetZoomFactor( rX, rY );
}

bool SwView::UpdateScrollbars()
{
    bool bRet = false;
    if ( !m_aVisArea.IsEmpty() )
    {
        const bool bBorder = IsDocumentBorder();
        tools::Rectangle aTmpRect( m_aVisArea );
        if ( bBorder )
        {
            Point aPt( DOCUMENTBORDER, DOCUMENTBORDER );
            aPt = AlignToPixel( aPt );
            aTmpRect.Move( -aPt.X(), -aPt.Y() );
        }

        Size aTmpSz( m_aDocSz );
        const long lOfst = bBorder ? 0 : DOCUMENTBORDER * 2;
        aTmpSz.AdjustWidth(lOfst ); aTmpSz.AdjustHeight(lOfst );

        {
            const bool bVScrollVisible = m_pVScrollbar->IsVisible(true);
            m_pVScrollbar->DocSzChgd( aTmpSz );
            m_pVScrollbar->ViewPortChgd( aTmpRect );
            if ( bVScrollVisible != m_pVScrollbar->IsVisible(true) )
                bRet = true;
        }
        {
            const bool bHScrollVisible = m_pHScrollbar->IsVisible(true);
            m_pHScrollbar->DocSzChgd( aTmpSz );
            m_pHScrollbar->ViewPortChgd( aTmpRect );
            if ( bHScrollVisible != m_pHScrollbar->IsVisible(true) )
                bRet = true;
            m_pScrollFill->Show(m_pHScrollbar->IsVisible(true) && m_pVScrollbar->IsVisible(true) );
        }
    }
    return bRet;
}

void SwView::Move()
{
    if ( GetWrtShell().IsInSelect() )
        GetWrtShell().EndSelect();
    SfxViewShell::Move();
}

bool SwView::HandleWheelCommands( const CommandEvent& rCEvt )
{
    bool bOk = false;
    const CommandWheelData* pWData = rCEvt.GetWheelData();
    if (pWData && CommandWheelMode::ZOOM == pWData->GetMode())
    {
        long nFact = m_pWrtShell->GetViewOptions()->GetZoom();
        if( 0L > pWData->GetDelta() )
            nFact = std::max( long(20), basegfx::zoomtools::zoomOut( nFact ));
        else
            nFact = std::min( long(600), basegfx::zoomtools::zoomIn( nFact ));

        SetZoom( SvxZoomType::PERCENT, nFact );
        bOk = true;
    }
    else
    {
        if (pWData && pWData->GetMode()==CommandWheelMode::SCROLL)
        {
            // This influences whether quick help is shown
            m_bWheelScrollInProgress=true;
        }

        if (pWData && (CommandWheelMode::SCROLL==pWData->GetMode()) &&
                (COMMAND_WHEEL_PAGESCROLL == pWData->GetScrollLines()))
        {
            if (pWData->GetDelta()<0)
                PhyPageDown();
            else
                PhyPageUp();
            bOk = true;
        }
        else
            bOk = m_pEditWin->HandleScrollCommand(rCEvt, m_pHScrollbar, m_pVScrollbar);

        // Restore default state for case when scroll command comes from dragging scrollbar handle
        m_bWheelScrollInProgress=false;
    }
    return bOk;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
