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
#include "precompiled_svtools.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <vcl/svapp.hxx>

#include <ipwin.hxx>
#include <hatchwindow.hxx>

/************************************************************************/
/*************************************************************************
|*    SvResizeHelper::SvResizeHelper()
|*
|*    Beschreibung
*************************************************************************/
SvResizeHelper::SvResizeHelper()
    : aBorder( 5, 5 )
    , nGrab( -1 )
    , bResizeable( sal_True )
{
}

/*************************************************************************
|*    SvResizeHelper::FillHandleRects()
|*
|*    Beschreibung: Die acht Handles zum vergroessern
*************************************************************************/
void SvResizeHelper::FillHandleRectsPixel( Rectangle aRects[ 8 ] ) const
{
    // nur wegen EMPTY_RECT
    Point aBottomRight = aOuter.BottomRight();

    // Links Oben
    aRects[ 0 ] = Rectangle( aOuter.TopLeft(), aBorder );
    // Oben Mitte
    aRects[ 1 ] = Rectangle( Point( aOuter.Center().X() - aBorder.Width() / 2,
                                    aOuter.Top() ),
                            aBorder );
    // Oben Rechts
    aRects[ 2 ] = Rectangle( Point( aBottomRight.X() - aBorder.Width() +1,
                                    aOuter.Top() ),
                            aBorder );
    // Mitte Rechts
    aRects[ 3 ] = Rectangle( Point( aBottomRight.X() - aBorder.Width() +1,
                                    aOuter.Center().Y() - aBorder.Height() / 2 ),
                            aBorder );
    // Unten Rechts
    aRects[ 4 ] = Rectangle( Point( aBottomRight.X() - aBorder.Width() +1,
                                    aBottomRight.Y() - aBorder.Height() +1 ),
                            aBorder );
    // Mitte Unten
    aRects[ 5 ] = Rectangle( Point( aOuter.Center().X() - aBorder.Width() / 2,
                                    aBottomRight.Y() - aBorder.Height() +1),
                            aBorder );
    // Links Unten
    aRects[ 6 ] = Rectangle( Point( aOuter.Left(),
                                    aBottomRight.Y() - aBorder.Height() +1),
                            aBorder );
    // Mitte Links
    aRects[ 7 ] = Rectangle( Point( aOuter.Left(),
                                    aOuter.Center().Y() - aBorder.Height() / 2 ),
                            aBorder );
}

/*************************************************************************
|*    SvResizeHelper::FillMoveRectsPixel()
|*
|*    Beschreibung: Die vier Kanten werden berechnet
*************************************************************************/
void SvResizeHelper::FillMoveRectsPixel( Rectangle aRects[ 4 ] ) const
{
    // Oben
    aRects[ 0 ] = aOuter;
    aRects[ 0 ].Bottom() = aRects[ 0 ].Top() + aBorder.Height() -1;
    // Rechts
    aRects[ 1 ] = aOuter;
    aRects[ 1 ].Left() = aRects[ 1 ].Right() - aBorder.Width() -1;
    //Unten
    aRects[ 2 ] = aOuter;
    aRects[ 2 ].Top() = aRects[ 2 ].Bottom() - aBorder.Height() -1;
    //Links
    aRects[ 3 ] = aOuter;
    aRects[ 3 ].Right() = aRects[ 3 ].Left() + aBorder.Width() -1;
}

/*************************************************************************
|*    SvResizeHelper::Draw()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeHelper::Draw( OutputDevice * pDev )
{
    pDev->Push();
    pDev->SetMapMode( MapMode() );
    Color aColBlack;
    Color aFillColor( COL_LIGHTGRAY );

    pDev->SetFillColor( aFillColor );
    pDev->SetLineColor();

    Rectangle   aMoveRects[ 4 ];
    FillMoveRectsPixel( aMoveRects );
    sal_uInt16 i;
    for( i = 0; i < 4; i++ )
        pDev->DrawRect( aMoveRects[ i ] );
    if( bResizeable )
    {
        // Handles malen
        pDev->SetFillColor( aColBlack );
        Rectangle   aRects[ 8 ];
        FillHandleRectsPixel( aRects );
        for( i = 0; i < 8; i++ )
            pDev->DrawRect( aRects[ i ] );
    }
    pDev->Pop();
}

/*************************************************************************
|*    SvResizeHelper::InvalidateBorder()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeHelper::InvalidateBorder( Window * pWin )
{
    Rectangle   aMoveRects[ 4 ];
    FillMoveRectsPixel( aMoveRects );
    for( sal_uInt16 i = 0; i < 4; i++ )
        pWin->Invalidate( aMoveRects[ i ] );
}

/*************************************************************************
|*    SvResizeHelper::SelectBegin()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SvResizeHelper::SelectBegin( Window * pWin, const Point & rPos )
{
    if( -1 == nGrab )
    {
        nGrab = SelectMove( pWin, rPos );
        if( -1 != nGrab )
        {
            aSelPos = rPos; // Start-Position merken
            pWin->CaptureMouse();
            return sal_True;
        }
    }
    return sal_False;
}

/*************************************************************************
|*    SvResizeHelper::SelectMove()
|*
|*    Beschreibung
*************************************************************************/
short SvResizeHelper::SelectMove( Window * pWin, const Point & rPos )
{
    if( -1 == nGrab )
    {
        if( bResizeable )
        {
            Rectangle aRects[ 8 ];
            FillHandleRectsPixel( aRects );
            for( sal_uInt16 i = 0; i < 8; i++ )
                if( aRects[ i ].IsInside( rPos ) )
                    return i;
        }
        // Move-Rect ueberlappen Handles
        Rectangle aMoveRects[ 4 ];
        FillMoveRectsPixel( aMoveRects );
        for( sal_uInt16 i = 0; i < 4; i++ )
            if( aMoveRects[ i ].IsInside( rPos ) )
                return 8;
    }
    else
    {
        Rectangle aRect( GetTrackRectPixel( rPos ) );
        aRect.SetSize( pWin->PixelToLogic( aRect.GetSize() ) );
        aRect.SetPos( pWin->PixelToLogic( aRect.TopLeft() ) );
        pWin->ShowTracking( aRect );
    }
    return nGrab;
}

Point SvResizeHelper::GetTrackPosPixel( const Rectangle & rRect ) const
{
    // wie das Rechteck zurueckkommt ist egal, es zaehlt welches Handle
    // initial angefasst wurde
    Point aPos;
    Rectangle aRect( rRect );
    aRect.Justify();
    // nur wegen EMPTY_RECT
    Point aBR = aOuter.BottomRight();
    Point aTR = aOuter.TopRight();
    Point aBL = aOuter.BottomLeft();
    switch( nGrab )
    {
        case 0:
            aPos = aRect.TopLeft() - aOuter.TopLeft();
            break;
        case 1:
            aPos.Y() =  aRect.Top() - aOuter.Top();
            break;
        case 2:
            aPos =  aRect.TopRight() - aTR;
            break;
        case 3:
            aPos.X() = aRect.Right() - aTR.X();
            break;
        case 4:
            aPos =  aRect.BottomRight() - aBR;
            break;
        case 5:
            aPos.Y() = aRect.Bottom() - aBR.Y();
            break;
        case 6:
            aPos =  aRect.BottomLeft() - aBL;
            break;
        case 7:
            aPos.X() = aRect.Left() - aOuter.Left();
            break;
        case 8:
            aPos = aRect.TopLeft() - aOuter.TopLeft();
            break;
    }
    return aPos += aSelPos;
}

/*************************************************************************
|*    SvResizeHelper::GetTrackRectPixel()
|*
|*    Beschreibung
*************************************************************************/
Rectangle SvResizeHelper::GetTrackRectPixel( const Point & rTrackPos ) const
{
    Rectangle aTrackRect;
    if( -1 != nGrab )
    {
        Point aDiff = rTrackPos - aSelPos;
        aTrackRect = aOuter;
        Point aBR = aOuter.BottomRight();
        switch( nGrab )
        {
            case 0:
                aTrackRect.Top() += aDiff.Y();
                aTrackRect.Left() += aDiff.X();
                break;
            case 1:
                aTrackRect.Top() += aDiff.Y();
                break;
            case 2:
                aTrackRect.Top() += aDiff.Y();
                aTrackRect.Right() = aBR.X() + aDiff.X();
                break;
            case 3:
                aTrackRect.Right() = aBR.X() + aDiff.X();
                break;
            case 4:
                aTrackRect.Bottom() = aBR.Y() + aDiff.Y();
                aTrackRect.Right() = aBR.X() + aDiff.X();
                break;
            case 5:
                aTrackRect.Bottom() = aBR.Y() + aDiff.Y();
                break;
            case 6:
                aTrackRect.Bottom() = aBR.Y() + aDiff.Y();
                aTrackRect.Left() += aDiff.X();
                break;
            case 7:
                aTrackRect.Left() += aDiff.X();
                break;
            case 8:
                if( Application::GetSettings().GetLayoutRTL() )
                    aDiff.X() = -aDiff.X(); // workaround for move in RTL mode
                aTrackRect.SetPos( aTrackRect.TopLeft() + aDiff );
                break;
/*
            case 0:
                aTrackRect = Rectangle( rTrackPos, aOuter.BottomRight() );
                break;
            case 1:
                aTrackRect = Rectangle( Point( aOuter.Left(), rTrackPos.Y() ),
                                         aOuter.BottomRight() );
                break;
            case 2:
                aTrackRect = Rectangle( rTrackPos, aOuter.BottomLeft() );
                break;
            case 3:
                aTrackRect = Rectangle( Point( rTrackPos.X(), aOuter.Top() ),
                                         aOuter.BottomLeft() );
                break;
            case 4:
                aTrackRect = Rectangle( rTrackPos, aOuter.TopLeft() );
                break;
            case 5:
                aTrackRect = Rectangle( aOuter.TopLeft(),
                                     Point( aOuter.Right(), rTrackPos.Y() ) );
                break;
            case 6:
                aTrackRect = Rectangle( aOuter.TopRight(), rTrackPos );
                break;
            case 7:
                aTrackRect = Rectangle( Point( rTrackPos.X(), aOuter.Top() ),
                                         aOuter.BottomRight() );
                break;
            case 8:
                aTrackRect = Rectangle( aOuter.TopLeft() + rTrackPos - aSelPos,
                                        aOuter.GetSize() );
                break;
*/
        }
    }
    return aTrackRect;
}

void SvResizeHelper::ValidateRect( Rectangle & rValidate ) const
{
    switch( nGrab )
    {
        case 0:
            if( rValidate.Top() > rValidate.Bottom() )
            {
                rValidate.Top() = rValidate.Bottom();
                rValidate.Bottom() = RECT_EMPTY;
            }
            if( rValidate.Left() > rValidate.Right() )
            {
                rValidate.Left() = rValidate.Right();
                rValidate.Right() = RECT_EMPTY;
            }
            break;
        case 1:
            if( rValidate.Top() > rValidate.Bottom() )
            {
                rValidate.Top() = rValidate.Bottom();
                rValidate.Bottom() = RECT_EMPTY;
            }
            break;
        case 2:
            if( rValidate.Top() > rValidate.Bottom() )
            {
                rValidate.Top() = rValidate.Bottom();
                rValidate.Bottom() = RECT_EMPTY;
            }
            if( rValidate.Left() > rValidate.Right() )
                rValidate.Right() = RECT_EMPTY;
            break;
        case 3:
            if( rValidate.Left() > rValidate.Right() )
                rValidate.Right() = RECT_EMPTY;
            break;
        case 4:
            if( rValidate.Top() > rValidate.Bottom() )
                rValidate.Bottom() = RECT_EMPTY;
            if( rValidate.Left() > rValidate.Right() )
                rValidate.Right() = RECT_EMPTY;
            break;
        case 5:
            if( rValidate.Top() > rValidate.Bottom() )
                rValidate.Bottom() = RECT_EMPTY;
            break;
        case 6:
            if( rValidate.Top() > rValidate.Bottom() )
                rValidate.Bottom() = RECT_EMPTY;
            if( rValidate.Left() > rValidate.Right() )
            {
                rValidate.Left() = rValidate.Right();
                rValidate.Right() = RECT_EMPTY;
            }
            break;
        case 7:
            if( rValidate.Left() > rValidate.Right() )
            {
                rValidate.Left() = rValidate.Right();
                rValidate.Right() = RECT_EMPTY;
            }
            break;
    }
    if( rValidate.Right() == RECT_EMPTY )
        rValidate.Right() = rValidate.Left();
    if( rValidate.Bottom() == RECT_EMPTY )
        rValidate.Bottom() = rValidate.Top();

    // Mindestgr"osse 5 x 5
    if( rValidate.Left() + 5 > rValidate.Right() )
        rValidate.Right() = rValidate.Left() +5;
    if( rValidate.Top() + 5 > rValidate.Bottom() )
        rValidate.Bottom() = rValidate.Top() +5;
}

/*************************************************************************
|*    SvResizeHelper::SelectRelease()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SvResizeHelper::SelectRelease( Window * pWin, const Point & rPos,
                                    Rectangle & rOutPosSize )
{
    if( -1 != nGrab )
    {
        rOutPosSize = GetTrackRectPixel( rPos );
        rOutPosSize.Justify();
        nGrab = -1;
        pWin->ReleaseMouse();
        pWin->HideTracking();
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
|*    SvResizeHelper::Release()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeHelper::Release( Window * pWin )
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
|*    Beschreibung
*************************************************************************/
SvResizeWindow::SvResizeWindow
(
    Window * pParent,
    VCLXHatchWindow* pWrapper
)
    : Window( pParent, WB_CLIPCHILDREN )
    , m_nMoveGrab( -1 )
    , m_bActive( sal_False )
    , m_pWrapper( pWrapper )
{
    OSL_ENSURE( pParent != NULL && pWrapper != NULL, "Wrong initialization of hatch window!\n" );
    SetBackground();
    SetAccessibleRole( ::com::sun::star::accessibility::AccessibleRole::EMBEDDED_OBJECT );
    m_aResizer.SetOuterRectPixel( Rectangle( Point(), GetOutputSizePixel() ) );
}

/*************************************************************************
|*    SvResizeWindow::SetHatchBorderPixel()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::SetHatchBorderPixel( const Size & rSize )
{
     m_aResizer.SetBorderPixel( rSize );
}

/*************************************************************************
|*    SvResizeWindow::SelectMouse()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::SelectMouse( const Point & rPos )
{
    short nGrab = m_aResizer.SelectMove( this, rPos );
    if( nGrab >= 4 )
        nGrab -= 4;
    if( m_nMoveGrab != nGrab )
    { // Pointer hat sich geaendert
        if( -1 == nGrab )
            SetPointer( m_aOldPointer );
        else
        {
            PointerStyle aStyle = POINTER_MOVE;
            if( nGrab == 3 )
                aStyle = POINTER_ESIZE;
            else if( nGrab == 2 )
                aStyle = POINTER_NESIZE;
            else if( nGrab == 1 )
                aStyle = POINTER_SSIZE;
            else if( nGrab == 0 )
                aStyle = POINTER_SESIZE;
            if( m_nMoveGrab == -1 ) // das erste mal
            {
                m_aOldPointer = GetPointer();
                SetPointer( Pointer( aStyle ) );
            }
            else
                SetPointer( Pointer( aStyle ) );
        }
        m_nMoveGrab = nGrab;
    }
}

/*************************************************************************
|*    SvResizeWindow::MouseButtonDown()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::MouseButtonDown( const MouseEvent & rEvt )
{
    if( m_aResizer.SelectBegin( this, rEvt.GetPosPixel() ) )
        SelectMouse( rEvt.GetPosPixel() );
}

/*************************************************************************
|*    SvResizeWindow::MouseMove()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::MouseMove( const MouseEvent & rEvt )
{
    if( m_aResizer.GetGrab() == -1 )
        SelectMouse( rEvt.GetPosPixel() );
    else
    {
        Rectangle aRect( m_aResizer.GetTrackRectPixel( rEvt.GetPosPixel() ) );
        Point aDiff = GetPosPixel();
        aRect.SetPos( aRect.TopLeft() + aDiff );
        m_aResizer.ValidateRect( aRect );

        m_pWrapper->QueryObjAreaPixel( aRect );
        aRect.SetPos( aRect.TopLeft() - aDiff );
        Point aPos = m_aResizer.GetTrackPosPixel( aRect );

        SelectMouse( aPos );
    }
}

/*************************************************************************
|*    SvResizeWindow::MouseButtonUp()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::MouseButtonUp( const MouseEvent & rEvt )
{
    if( m_aResizer.GetGrab() != -1 )
    {
        Rectangle aRect( m_aResizer.GetTrackRectPixel( rEvt.GetPosPixel() ) );
        Point aDiff = GetPosPixel();
        aRect.SetPos( aRect.TopLeft() + aDiff );
        // aRect -= GetAllBorderPixel();
        m_aResizer.ValidateRect( aRect );

        m_pWrapper->QueryObjAreaPixel( aRect );

        Rectangle aOutRect;
        if( m_aResizer.SelectRelease( this, rEvt.GetPosPixel(), aOutRect ) )
        {
            m_nMoveGrab = -1;
            SetPointer( m_aOldPointer );
            m_pWrapper->RequestObjAreaPixel( aRect );
        }
    }
}

/*************************************************************************
|*    SvResizeWindow::KeyEvent()
|*
|*    Beschreibung
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
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::Resize()
{
    m_aResizer.InvalidateBorder( this ); // alten Bereich
    m_aResizer.SetOuterRectPixel( Rectangle( Point(), GetOutputSizePixel() ) );
    m_aResizer.InvalidateBorder( this ); // neuen Bereich
}

/*************************************************************************
|*    SvResizeWindow::Paint()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::Paint( const Rectangle & /*rRect*/ )
{
    m_aResizer.Draw( this );
}

long SvResizeWindow::PreNotify( NotifyEvent& rEvt )
{
    if ( rEvt.GetType() == EVENT_GETFOCUS && !m_bActive )
    {
        m_bActive = sal_True;
        m_pWrapper->Activated();
    }

    return Window::PreNotify(rEvt);
}

long SvResizeWindow::Notify( NotifyEvent& rEvt )
{
    if ( rEvt.GetType() == EVENT_LOSEFOCUS && m_bActive )
    {
        sal_Bool bHasFocus = HasChildPathFocus(sal_True);
        if ( !bHasFocus )
        {
            m_bActive = sal_False;
            m_pWrapper->Deactivated();
        }
    }

    return Window::Notify(rEvt);
}

