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

#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <osl/diagnose.h>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>

#include "ipwin.hxx"
#include "hatchwindow.hxx"

/************************************************************************/
/*************************************************************************
|*    SvResizeHelper::SvResizeHelper()
|*
|*    Description
*************************************************************************/
SvResizeHelper::SvResizeHelper()
    : aBorder( 5, 5 )
    , nGrab( -1 )
    , mofStartingRatio( std::nullopt )
{
}

/*************************************************************************
|*    SvResizeHelper::FillHandleRects()
|*
|*    Description: the eight handles to magnify
*************************************************************************/
std::array<tools::Rectangle,8> SvResizeHelper::FillHandleRectsPixel() const
{
    std::array<tools::Rectangle,8> aRects;

    // only because of EMPTY_RECT
    Point aBottomRight = aOuter.BottomRight();

    // upper left
    aRects[ 0 ] = tools::Rectangle( aOuter.TopLeft(), aBorder );
    // upper middle
    aRects[ 1 ] = tools::Rectangle( Point( aOuter.Center().X() - aBorder.Width() / 2,
                                    aOuter.Top() ),
                            aBorder );
    // upper right
    aRects[ 2 ] = tools::Rectangle( Point( aBottomRight.X() - aBorder.Width() +1,
                                    aOuter.Top() ),
                            aBorder );
    // middle right
    aRects[ 3 ] = tools::Rectangle( Point( aBottomRight.X() - aBorder.Width() +1,
                                    aOuter.Center().Y() - aBorder.Height() / 2 ),
                            aBorder );
    // lower right
    aRects[ 4 ] = tools::Rectangle( Point( aBottomRight.X() - aBorder.Width() +1,
                                    aBottomRight.Y() - aBorder.Height() +1 ),
                            aBorder );
    // lower middle
    aRects[ 5 ] = tools::Rectangle( Point( aOuter.Center().X() - aBorder.Width() / 2,
                                    aBottomRight.Y() - aBorder.Height() +1),
                            aBorder );
    // lower left
    aRects[ 6 ] = tools::Rectangle( Point( aOuter.Left(),
                                    aBottomRight.Y() - aBorder.Height() +1),
                            aBorder );
    // middle left
    aRects[ 7 ] = tools::Rectangle( Point( aOuter.Left(),
                                    aOuter.Center().Y() - aBorder.Height() / 2 ),
                            aBorder );
    return aRects;
}

/*************************************************************************
|*    SvResizeHelper::FillMoveRectsPixel()
|*
|*    Description: the four edges are calculated
*************************************************************************/
std::array<tools::Rectangle,4> SvResizeHelper::FillMoveRectsPixel() const
{
    std::array<tools::Rectangle,4> aRects;

    // upper
    aRects[ 0 ] = aOuter;
    aRects[ 0 ].SetBottom( aRects[ 0 ].Top() + aBorder.Height() -1 );
    // right
    aRects[ 1 ] = aOuter;
    if (!aOuter.IsWidthEmpty())
        aRects[ 1 ].SetLeft( aRects[ 1 ].Right() - aBorder.Width() -1 );
    // lower
    aRects[ 2 ] = aOuter;
    if (!aOuter.IsHeightEmpty())
        aRects[ 2 ].SetTop( aRects[ 2 ].Bottom() - aBorder.Height() -1 );
    // left
    aRects[ 3 ] = aOuter;
    aRects[ 3 ].SetRight( aRects[ 3 ].Left() + aBorder.Width() -1 );

    return aRects;
}

/*************************************************************************
|*    SvResizeHelper::Draw()
|*
|*    Description
*************************************************************************/
void SvResizeHelper::Draw(vcl::RenderContext& rRenderContext)
{
    rRenderContext.Push();
    rRenderContext.SetMapMode( MapMode() );

    rRenderContext.SetFillColor( COL_LIGHTGRAY );
    rRenderContext.SetLineColor();

    std::array<tools::Rectangle,4> aMoveRects = FillMoveRectsPixel();
    sal_uInt16 i;
    for (i = 0; i < 4; i++)
        rRenderContext.DrawRect(aMoveRects[i]);
    // draw handles
    rRenderContext.SetFillColor(Color()); // black
    std::array<tools::Rectangle,8> aRects = FillHandleRectsPixel();
    for (i = 0; i < 8; i++)
        rRenderContext.DrawRect( aRects[ i ] );
    rRenderContext.Pop();
}

/*************************************************************************
|*    SvResizeHelper::InvalidateBorder()
|*
|*    Description
*************************************************************************/
void SvResizeHelper::InvalidateBorder( vcl::Window * pWin )
{
    std::array<tools::Rectangle,4> aMoveRects = FillMoveRectsPixel();
    for(const auto & rMoveRect : aMoveRects)
        pWin->Invalidate( rMoveRect );
}

/*************************************************************************
|*    SvResizeHelper::SelectBegin()
|*
|*    Description
*************************************************************************/
bool SvResizeHelper::SelectBegin( vcl::Window * pWin, const Point & rPos, const bool bShiftPressed )
{
    if( -1 == nGrab && pWin )
    {
        nGrab = SelectMove( pWin, rPos, bShiftPressed );
        if( -1 != nGrab )
        {
            aSelPos = rPos; // store start position
            mofStartingRatio = std::nullopt;
            const auto aWinSize = pWin->GetSizePixel();
            if( aWinSize.Height() )
                mofStartingRatio  = aWinSize.Width() / static_cast<double>( aWinSize.Height() );
            pWin->CaptureMouse();
            return true;
        }
    }
    return false;
}

/*************************************************************************
|*    SvResizeHelper::SelectMove()
|*
|*    Description
*************************************************************************/
short SvResizeHelper::SelectMove( vcl::Window * pWin, const Point & rPos, const bool bShiftPressed )
{
    if( -1 == nGrab )
    {
        std::array<tools::Rectangle,8> aRects = FillHandleRectsPixel();
        for( sal_uInt16 i = 0; i < 8; i++ )
            if( aRects[ i ].Contains( rPos ) )
                return i;
        // Move-Rect overlaps Handles
        std::array<tools::Rectangle,4> aMoveRects = FillMoveRectsPixel();
        for(const auto & rMoveRect : aMoveRects)
            if( rMoveRect.Contains( rPos ) )
                return 8;
    }
    else
    {
        tools::Rectangle aRect = pWin->PixelToLogic(GetTrackRectPixel( rPos, bShiftPressed ));
        pWin->ShowTracking( aRect );
    }
    return nGrab;
}

Point SvResizeHelper::GetTrackPosPixel( const tools::Rectangle & rRect ) const
{
    // not important how the rectangle is returned, it is important
    // which handle has been touched
    Point aPos;
    tools::Rectangle aRect( rRect );
    aRect.Normalize();
    // only because of EMPTY_RECT
    Point aBR = aOuter.BottomRight();
    Point aTR = aOuter.TopRight();
    Point aBL = aOuter.BottomLeft();
    bool bRTL = AllSettings::GetLayoutRTL();
    switch( nGrab )
    {
        case 0:
            // FIXME: disable it for RTL because it's wrong calculations
            if( bRTL )
                break;
            aPos = aRect.TopLeft() - aOuter.TopLeft();
            break;
        case 1:
            aPos.setY(  aRect.Top() - aOuter.Top() );
            break;
        case 2:
            // FIXME: disable it for RTL because it's wrong calculations
            if( bRTL )
                break;
            aPos =  aRect.TopRight() - aTR;
            break;
        case 3:
            if( bRTL )
                aPos.setX( aRect.Left() - aTR.X() );
            else
                aPos.setX( aRect.Right() - aTR.X() );
            break;
        case 4:
            // FIXME: disable it for RTL because it's wrong calculations
            if( bRTL )
                break;
            aPos =  aRect.BottomRight() - aBR;
            break;
        case 5:
            aPos.setY( aRect.Bottom() - aBR.Y() );
            break;
        case 6:
            // FIXME: disable it for RTL because it's wrong calculations
            if( bRTL )
                break;
            aPos =  aRect.BottomLeft() - aBL;
            break;
        case 7:
            if( bRTL )
                aPos.setX( aRect.Right() + aOuter.Right() - aOuter.Right() );
            else
                aPos.setX( aRect.Left() - aOuter.Left() );
            break;
        case 8:
            aPos = aRect.TopLeft() - aOuter.TopLeft();
            break;
    }
    return aPos + aSelPos;
}

/*************************************************************************
|*    SvResizeHelper::GetTrackRectPixel()
|*
|*    Description
*************************************************************************/
tools::Rectangle SvResizeHelper::GetTrackRectPixel( const Point & rTrackPos, const bool bShiftPressed ) const
{
    tools::Rectangle aTrackRect;
    if( -1 == nGrab )
        return aTrackRect;
    Point aDiff = rTrackPos - aSelPos;
    aTrackRect = aOuter;
    Point aBR = aOuter.BottomRight();
    bool bRTL = AllSettings::GetLayoutRTL();
    switch( nGrab )
    {
        case 0:
            aTrackRect.AdjustTop(aDiff.Y() );
            // ugly solution for resizing OLE objects in RTL
            if( bRTL )
                aTrackRect.SetRight( aBR.X() - aDiff.X() );
            else
                aTrackRect.AdjustLeft(aDiff.X() );
            break;
        case 1:
            aTrackRect.AdjustTop(aDiff.Y() );
            break;
        case 2:
            aTrackRect.AdjustTop(aDiff.Y() );
            // ugly solution for resizing OLE objects in RTL
            if( bRTL )
                aTrackRect.AdjustLeft( -(aDiff.X()) );
            else
                aTrackRect.SetRight( aBR.X() + aDiff.X() );
            break;
        case 3:
            // ugly solution for resizing OLE objects in RTL
            if( bRTL )
                aTrackRect.AdjustLeft( -(aDiff.X()) );
            else
                aTrackRect.SetRight( aBR.X() + aDiff.X() );
            break;
        case 4:
            aTrackRect.SetBottom( aBR.Y() + aDiff.Y() );
            // ugly solution for resizing OLE objects in RTL
            if( bRTL )
                aTrackRect.AdjustLeft( -(aDiff.X()) );
            else
                aTrackRect.SetRight( aBR.X() + aDiff.X() );
            break;
        case 5:
            aTrackRect.SetBottom( aBR.Y() + aDiff.Y() );
            break;
        case 6:
            aTrackRect.SetBottom( aBR.Y() + aDiff.Y() );
            // ugly solution for resizing OLE objects in RTL
            if( bRTL )
                aTrackRect.SetRight( aBR.X() - aDiff.X() );
            else
                aTrackRect.AdjustLeft(aDiff.X() );
            break;
        case 7:
            // ugly solution for resizing OLE objects in RTL
            if( bRTL )
                aTrackRect.SetRight( aBR.X() - aDiff.X() );
            else
                aTrackRect.AdjustLeft(aDiff.X() );
            break;
        case 8:
            if( bRTL )
                aDiff.setX( -aDiff.X() ); // workaround for move in RTL mode
            if( bShiftPressed )
            {
                const Point aDiffAbs( std::abs( aDiff.X() ), std::abs( aDiff.Y() ) );
                if( 2 * aDiffAbs.X() < aDiffAbs.Y() ) // vertical move
                    aDiff.setX( 0L );
                else if( aDiffAbs.X() > 2 * aDiffAbs.Y() ) // horizontal move
                    aDiff.setY( 0L );
                else if( aDiffAbs.X() != aDiffAbs.Y() ) // 45° move
                {
                    if( aDiffAbs.X() > aDiffAbs.Y() )
                        aDiff.setY( aDiff.Y() / aDiffAbs.Y() * aDiffAbs.X() );
                    else
                        aDiff.setX( aDiff.X() / aDiffAbs.X() * aDiffAbs.Y() );
                }
            }
            aTrackRect.SetPos( aTrackRect.TopLeft() + aDiff );
            break;
    }
    // TODO: should use same code as other objects. See SdrTextObj::ImpDragCalcRect() and SdrObject::ImpDragCalcRect()
    // uneven values are for edges
    const bool bIsEdge = ( nGrab != (( nGrab / 2 ) * 2) );
    // tdf#163816: Resizing OLE object in edited mode should behave like in not-edited mode
    // Corner handles are proportional by default, whereas edge handles are not proportional
    // pressed Shift toggles behavior
    const bool bProportional = ( bIsEdge == bShiftPressed );
    if( bProportional && nGrab != 8 && mofStartingRatio )
    {
        bool bChangeWidth = false;
        if( nGrab == 1 ||  nGrab == 5 ) // top and bottom handles
        {
            bChangeWidth = true;
        }
        else if ( nGrab != 3 &&  nGrab != 7 )
        {   // handles in corners: keep the largest size
            bChangeWidth = aTrackRect.GetWidth() < aTrackRect.GetHeight() * mofStartingRatio.value();
        }
        if( bChangeWidth )
        {
            const tools::Long nNewWidth =  aTrackRect.GetHeight() * mofStartingRatio.value();
            if ( nGrab == 6 ||  nGrab == 0 ) // corners on left side
                // move left with right as reference
                aTrackRect.SetLeft( aTrackRect.Right() - nNewWidth );
            else if ( nGrab == 1 ||  nGrab == 5 ) // top and bottom handles
                // move left half of the change of width to keep the handle in the middle
                aTrackRect.SetLeft( aTrackRect.Left() + ( aTrackRect.GetWidth() - nNewWidth ) / 2 );
                // other handles use left as reference, then no change needed
            aTrackRect.SetWidth( nNewWidth );
        }
        else
        {
            const tools::Long nNewHeight =  aTrackRect.GetWidth() / mofStartingRatio.value();
            if ( nGrab == 2 ||  nGrab == 0 )  // corners on top
                // move top with bottom as reference
                aTrackRect.SetTop( aTrackRect.Bottom() - nNewHeight );
            else if ( nGrab == 3 ||  nGrab == 7 )  // right and left handles
                // move top half of the change of height to keep the handle in the middle
                aTrackRect.SetTop( aTrackRect.Top() + ( aTrackRect.GetHeight() - nNewHeight ) / 2 );
                // other handles use top as reference, then no change needed
            aTrackRect.SetHeight( nNewHeight );
        }
    }
    return aTrackRect;
}

void SvResizeHelper::ValidateRect( tools::Rectangle & rValidate ) const
{
    switch( nGrab )
    {
        case 0:
            if( rValidate.Top() > rValidate.Bottom() )
                rValidate.SetTop( rValidate.Bottom() );
            if( rValidate.Left() > rValidate.Right() )
                rValidate.SetLeft( rValidate.Right() );
            break;
        case 1:
            if( rValidate.Top() > rValidate.Bottom() )
                rValidate.SetTop( rValidate.Bottom() );
            break;
        case 2:
            if( rValidate.Top() > rValidate.Bottom() )
                rValidate.SetTop( rValidate.Bottom() );
            if( rValidate.Left() > rValidate.Right() )
                rValidate.SetRight( rValidate.Left() );
            break;
        case 3:
            if( rValidate.Left() > rValidate.Right() )
                rValidate.SetRight( rValidate.Left() );
            break;
        case 4:
            if( rValidate.Top() > rValidate.Bottom() )
                rValidate.SetBottom( rValidate.Top() );
            if( rValidate.Left() > rValidate.Right() )
                rValidate.SetRight( rValidate.Left() );
            break;
        case 5:
            if( rValidate.Top() > rValidate.Bottom() )
                rValidate.SetBottom( rValidate.Top() );
            break;
        case 6:
            if( rValidate.Top() > rValidate.Bottom() )
                rValidate.SetBottom( rValidate.Top() );
            if( rValidate.Left() > rValidate.Right() )
                rValidate.SetLeft( rValidate.Right() );
            break;
        case 7:
            if( rValidate.Left() > rValidate.Right() )
                rValidate.SetLeft( rValidate.Right() );
            break;
    }

    // Minimum size 5 x 5
    if( rValidate.Left() + 5 > rValidate.Right() )
        rValidate.SetRight( rValidate.Left() + 5 );
    if( rValidate.Top() + 5 > rValidate.Bottom() )
        rValidate.SetBottom( rValidate.Top() + 5 );
}

/*************************************************************************
|*    SvResizeHelper::SelectRelease()
|*
|*    Description
*************************************************************************/
bool SvResizeHelper::SelectRelease( vcl::Window * pWin, const Point & rPos,
                                    tools::Rectangle & rOutPosSize, const bool bShiftPressed )
{
    if( -1 != nGrab )
    {
        rOutPosSize = GetTrackRectPixel( rPos, bShiftPressed );
        rOutPosSize.Normalize();
        nGrab = -1;
        pWin->ReleaseMouse();
        pWin->HideTracking();
        return true;
    }
    return false;
}

/*************************************************************************
|*    SvResizeHelper::Release()
|*
|*    Description
*************************************************************************/
void SvResizeHelper::Release( vcl::Window * pWin )
{
    if( nGrab != -1 )
    {
        pWin->ReleaseMouse();
        pWin->HideTracking();
        nGrab = -1;
    }
}

/*************************************************************************
|*    SvResizeWindow::SvResizeWindow()
|*
|*    Description
*************************************************************************/
SvResizeWindow::SvResizeWindow
(
    vcl::Window * pParent,
    VCLXHatchWindow* pWrapper
)
    : Window( pParent, WB_CLIPCHILDREN )
    , m_aOldPointer(PointerStyle::Arrow)
    , m_nMoveGrab( -1 )
    , m_bActive( false )
    , m_pWrapper( pWrapper )
{
    OSL_ENSURE( pParent != nullptr && pWrapper != nullptr, "Wrong initialization of hatch window!" );
    SetBackground();
    SetAccessibleRole( css::accessibility::AccessibleRole::EMBEDDED_OBJECT );
    m_aResizer.SetOuterRectPixel( tools::Rectangle( Point(), GetOutputSizePixel() ) );
}

/*************************************************************************
|*    SvResizeWindow::SetHatchBorderPixel()
|*
|*    Description
*************************************************************************/
void SvResizeWindow::SetHatchBorderPixel( const Size & rSize )
{
     m_aResizer.SetBorderPixel( rSize );
}

/*************************************************************************
|*    SvResizeWindow::SelectMouse()
|*
|*    Description
*************************************************************************/
void SvResizeWindow::SelectMouse( const Point & rPos, const bool bShiftPressed )
{
    short nGrab = m_aResizer.SelectMove( this, rPos, bShiftPressed );
    if( nGrab >= 4 )
        nGrab -= 4;
    if( m_nMoveGrab == nGrab )
        return;

    // Pointer did change
    if( -1 == nGrab )
        SetPointer( m_aOldPointer );
    else
    {
        PointerStyle aStyle = PointerStyle::Move;
        if( nGrab == 3 )
            aStyle = PointerStyle::ESize;
        else if( nGrab == 2 )
            aStyle = PointerStyle::NESize;
        else if( nGrab == 1 )
            aStyle = PointerStyle::SSize;
        else if( nGrab == 0 )
            aStyle = PointerStyle::SESize;
        if( m_nMoveGrab == -1 ) // the first time
        {
            m_aOldPointer = GetPointer();
            SetPointer( aStyle );
        }
        else
            SetPointer( aStyle );
    }
    m_nMoveGrab = nGrab;
}

/*************************************************************************
|*    SvResizeWindow::MouseButtonDown()
|*
|*    Description
*************************************************************************/
void SvResizeWindow::MouseButtonDown( const MouseEvent & rEvt )
{
    if( m_aResizer.SelectBegin( this, rEvt.GetPosPixel(), rEvt.IsShift() ) )
        SelectMouse( rEvt.GetPosPixel(), rEvt.IsShift() );
}

/*************************************************************************
|*    SvResizeWindow::MouseMove()
|*
|*    Description
*************************************************************************/
void SvResizeWindow::MouseMove( const MouseEvent & rEvt )
{
    if( m_aResizer.GetGrab() == -1 )
        SelectMouse( rEvt.GetPosPixel(), rEvt.IsShift() );
    else
    {
        tools::Rectangle aRect( m_aResizer.GetTrackRectPixel( rEvt.GetPosPixel(), rEvt.IsShift() ) );
        Point aDiff = GetPosPixel();
        aRect.SetPos( aRect.TopLeft() + aDiff );
        m_aResizer.ValidateRect( aRect );

        m_pWrapper->QueryObjAreaPixel( aRect );
        aRect.SetPos( aRect.TopLeft() - aDiff );
        Point aPos = m_aResizer.GetTrackPosPixel( aRect );

        SelectMouse( aPos, rEvt.IsShift() );
    }
}

/*************************************************************************
|*    SvResizeWindow::MouseButtonUp()
|*
|*    Description
*************************************************************************/
void SvResizeWindow::MouseButtonUp( const MouseEvent & rEvt )
{
    if( m_aResizer.GetGrab() == -1 )
        return;

    tools::Rectangle aRect( m_aResizer.GetTrackRectPixel( rEvt.GetPosPixel(), rEvt.IsShift() ) );
    Point aDiff = GetPosPixel();
    aRect.SetPos( aRect.TopLeft() + aDiff );
    // aRect -= GetAllBorderPixel();
    m_aResizer.ValidateRect( aRect );

    m_pWrapper->QueryObjAreaPixel( aRect );

    tools::Rectangle aOutRect;
    if( m_aResizer.SelectRelease( this, rEvt.GetPosPixel(), aOutRect, rEvt.IsShift() ) )
    {
        m_nMoveGrab = -1;
        SetPointer( m_aOldPointer );
        m_pWrapper->RequestObjAreaPixel( aRect );
    }
}

/*************************************************************************
|*    SvResizeWindow::KeyEvent()
|*
|*    Description
*************************************************************************/
void SvResizeWindow::KeyInput( const KeyEvent & rEvt )
{
    if( rEvt.GetKeyCode().GetCode() == KEY_ESCAPE )
    {
        m_aResizer.Release( this );
        m_pWrapper->InplaceDeactivate();
    }
}

/*************************************************************************
|*    SvResizeWindow::Resize()
|*
|*    Description
*************************************************************************/
void SvResizeWindow::Resize()
{
    m_aResizer.InvalidateBorder( this ); // old area
    m_aResizer.SetOuterRectPixel( tools::Rectangle( Point(), GetOutputSizePixel() ) );
    m_aResizer.InvalidateBorder( this ); // new area
}

/*************************************************************************
|*    SvResizeWindow::Paint()
|*
|*    Description
*************************************************************************/
void SvResizeWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle & /*rRect*/ )
{
    m_aResizer.Draw(rRenderContext);
}

bool SvResizeWindow::PreNotify( NotifyEvent& rEvt )
{
    if ( rEvt.GetType() == NotifyEventType::GETFOCUS && !m_bActive )
    {
        m_bActive = true;
        m_pWrapper->Activated();
    }

    return Window::PreNotify(rEvt);
}

bool SvResizeWindow::EventNotify( NotifyEvent& rEvt )
{
    if ( rEvt.GetType() == NotifyEventType::LOSEFOCUS && m_bActive )
    {
        bool bHasFocus = HasChildPathFocus(true);
        if ( !bHasFocus )
        {
            m_bActive = false;
            m_pWrapper->Deactivated();
        }
    }

    return Window::EventNotify(rEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
