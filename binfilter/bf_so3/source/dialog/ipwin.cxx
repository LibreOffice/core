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

#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <bf_so3/ipenv.hxx>
#include <bf_so3/ipobj.hxx>
#include "bf_so3/ipwin.hxx"

namespace binfilter {

/************************************************************************/
/*************************************************************************
|*    SvResizeHelper::SvResizeHelper()
|*
|*    Beschreibung
*************************************************************************/
SvResizeHelper::SvResizeHelper()
    : aBorder( 5, 5 )
    , nGrab( -1 )
    , bResizeable( TRUE )
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
    USHORT i;
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
    for( USHORT i = 0; i < 4; i++ )
        pWin->Invalidate( aMoveRects[ i ] );
}

/*************************************************************************
|*    SvResizeHelper::SelectBegin()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvResizeHelper::SelectBegin( Window * pWin, const Point & rPos )
{
    if( -1 == nGrab )
    {
        nGrab = SelectMove( pWin, rPos );
        if( -1 != nGrab )
        {
            aSelPos = rPos; // Start-Position merken
            pWin->CaptureMouse();
            return TRUE;
        }
    }
    return FALSE;
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
            for( USHORT i = 0; i < 8; i++ )
                if( aRects[ i ].IsInside( rPos ) )
                    return i;
        }
        // Move-Rect ueberlappen Handles
        Rectangle aMoveRects[ 4 ];
        FillMoveRectsPixel( aMoveRects );
        for( USHORT i = 0; i < 4; i++ )
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
BOOL SvResizeHelper::SelectRelease( Window * pWin, const Point & rPos,
                                    Rectangle & rOutPosSize )
{
    if( -1 != nGrab )
    {
        rOutPosSize = GetTrackRectPixel( rPos );
        rOutPosSize.Justify();
        nGrab = -1;
        pWin->ReleaseMouse();
        pWin->HideTracking();
        return TRUE;
    }
    return FALSE;
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
/************************************************************************/
/*************************************************************************
|*    SvResizeWindow::SvResizeWindow()
|*
|*    Beschreibung
*************************************************************************/
SvResizeWindow::SvResizeWindow
(
    Window * pParent
)
    : Window( pParent, WB_CLIPCHILDREN )
    , nMoveGrab( -1 )
    , pObjWin( NULL )
{
    SetBackground();
    aResizer.SetOuterRectPixel( Rectangle( Point(), GetOutputSizePixel() ) );
}

/*************************************************************************
|*    SvResizeWindow::AdjustObjWin()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::AdjustObjWin()
{
    if( pObjWin )
    {
        Rectangle aRect = GetInnerRectPixel();
        pObjWin->SetPosSizePixel( aRect.TopLeft(),
                                  aRect.GetSize() );
    }
}

/*************************************************************************
|*    SvResizeWindow::SetObjWin()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::SetObjWin( Window * pNewWin )
{
    pObjWin = pNewWin;
    AdjustObjWin();
}

/*************************************************************************
|*    SvResizeWindow::CalcInnerRectPixel()
|*
|*    Beschreibung
*************************************************************************/
Rectangle SvResizeWindow::CalcInnerRectPixel( const Point & rPos,
                                              const Size & rSize ) const
{
    Rectangle aInner( rPos, rSize );
    return aInner -= GetAllBorderPixel();
}

/*************************************************************************
|*    SvResizeWindow::GetInnerRectPixel()
|*
|*    Beschreibung
*************************************************************************/
Rectangle SvResizeWindow::GetInnerRectPixel() const
{
    return CalcInnerRectPixel( Point(), GetOutputSizePixel() );
}

/*************************************************************************
|*    SvResizeWindow::SelectMouse()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::SelectMouse( const Point & rPos )
{
    short nGrab = aResizer.SelectMove( this, rPos );
    if( nGrab >= 4 )
        nGrab -= 4;
    if( nMoveGrab != nGrab )
    { // Pointer hat sich geaendert
        if( -1 == nGrab )
            SetPointer( aOldPointer );
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
            if( nMoveGrab == -1 ) // das erste mal
            {
                aOldPointer = GetPointer();
                SetPointer( Pointer( aStyle ) );
            }
            else
                SetPointer( Pointer( aStyle ) );
        }
        nMoveGrab = nGrab;
    }
}

/*************************************************************************
|*    SvResizeWindow::MouseButtonDown()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::MouseButtonDown( const MouseEvent & rEvt )
{
    if( aResizer.SelectBegin( this, rEvt.GetPosPixel() ) )
        SelectMouse( rEvt.GetPosPixel() );
}

/*************************************************************************
|*    SvResizeWindow::MouseMove()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::MouseMove( const MouseEvent & rEvt )
{
    if( aResizer.GetGrab() == -1 )
        SelectMouse( rEvt.GetPosPixel() );
    else
    {
        Rectangle aRect( aResizer.GetTrackRectPixel( rEvt.GetPosPixel() ) );
        Point aDiff = GetPosPixel() + aPosCorrection;
        aRect.SetPos( aRect.TopLeft() + aDiff );
        aRect -= GetAllBorderPixel();
        aResizer.ValidateRect( aRect );

        QueryObjAreaPixel( aRect );
        aRect += GetAllBorderPixel();
        aRect.SetPos( aRect.TopLeft() - aDiff );
        Point aPos = aResizer.GetTrackPosPixel( aRect );

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
    if( aResizer.GetGrab() != -1 )
    {
        Rectangle aRect( aResizer.GetTrackRectPixel( rEvt.GetPosPixel() ) );
        Point aDiff = GetPosPixel() + aPosCorrection;
        aRect.SetPos( aRect.TopLeft() + aDiff );
        aRect -= GetAllBorderPixel();
        aResizer.ValidateRect( aRect );
        QueryObjAreaPixel( aRect );

        Rectangle aOutRect;
        if( aResizer.SelectRelease( this, rEvt.GetPosPixel(), aOutRect ) )
        {
            nMoveGrab = -1;
            SetPointer( aOldPointer );
            RequestObjAreaPixel( aRect );
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
        aResizer.Release( this );
}

/*************************************************************************
|*    SvResizeWindow::Resize()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::Resize()
{
    aResizer.InvalidateBorder( this ); // alten Bereich
    aResizer.SetOuterRectPixel( Rectangle( Point(), GetOutputSizePixel() ) );
    aResizer.InvalidateBorder( this ); // neuen Bereich
    AdjustObjWin();
}

/*************************************************************************
|*    SvResizeWindow::SetInnerPosSizePixel()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::SetInnerPosSizePixel( const Point & rPos,
                                           const Size & rSize )
{
    Rectangle aRect( rPos, rSize );
    aRect += GetAllBorderPixel();
    SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );
}

//=========================================================================
void SvResizeWindow::QueryObjAreaPixel
(
    Rectangle & rRect/* Das innere Rechteck von dem alle Borders
                       abgezogen sind. */
)
/*	[Beschreibung]

    Es wird gefragt, ob die angegebene Groesse uebernommen werden soll.
    Ist das der Fall, dann wird rRect nicht veraendert. Ansonsten wird
    rRect auf den gew"unschten Wert gesetzt.

*/
{
    (void)rRect;
}

//=========================================================================
void SvResizeWindow::RequestObjAreaPixel
(
    const Rectangle &	rRect	/* Das innere Rechteck von dem alle Borders
                                   abgezogen sind. */
)
/*	[Beschreibung]

    Die Methode setzt die Position und Gr"osse des Fensters mit
    <SvResizeWindow::SetPosSizePixel()>.
*/
{
    Rectangle aRect = rRect;
    aRect += GetAllBorderPixel();
    SetPosSizePixel( aRect.TopLeft() - aPosCorrection,
                     aRect.GetSize() );
}

/*************************************************************************
|*    SvResizeWindow::Paint()
|*
|*    Beschreibung
*************************************************************************/
void SvResizeWindow::Paint( const Rectangle & /*rRect*/ )
{
    aResizer.Draw( this );
}

/************************************************************************/
/*************************************************************************
|*    SvInPlaceWindow::SvInPlaceWindow()
|*
|*    Beschreibung
*************************************************************************/
SvInPlaceWindow::SvInPlaceWindow( Window * pParent, SvInPlaceEnvironment * pIPEnvP )
    : SvResizeWindow( pParent )
    , pIPEnv( pIPEnvP )
{
    aResizer.SetResizeable( (pIPEnv->GetIPObj()->GetMiscStatus() & SVOBJ_MISCSTATUS_NOTRESIZEABLE) == 0 );
}

/*************************************************************************
|*    SvInPlaceWindow::KeyEvent()
|*
|*    Beschreibung
*************************************************************************/
void SvInPlaceWindow::KeyInput( const KeyEvent & rEvt )
{
    SvResizeWindow::KeyInput( rEvt );
    if( rEvt.GetKeyCode().GetCode() == KEY_ESCAPE )
    {
        pIPEnv->GetIPObj()->DoInPlaceActivate( FALSE );
    }
}

//=========================================================================
void SvInPlaceWindow::QueryObjAreaPixel
(
    Rectangle & rRect	/* Das innere Rechteck von dem alle Borders
                           abgezogen sind. */
)
/*	[Beschreibung]

    Es wird gefragt, ob die angegebene Groesse uebernommen werden soll.
    Ist das der Fall, dann wird rRect nicht veraendert. Ansonsten wird
    rRect auf den gew"unschten Wert gesetzt.

    [Querverweise]

    <SvResizeWindow::QueryObjAreaPixel>
*/
{
    pIPEnv->QueryObjAreaPixel( rRect );
}

//=========================================================================
void SvInPlaceWindow::RequestObjAreaPixel
(
    const Rectangle &	rRect	/* Das innere Rechteck von dem alle Borders
                                   abgezogen sind. */
)
/*	[Beschreibung]

    Die Methode setzt die Position und Gr"osse des Fensters mit
    <SvResizeWindow::SetPosSizePixel()>.
*/
{
    pIPEnv->GetContainerEnv()->RequestObjAreaPixel( rRect );
    pIPEnv->GetIPObj()->SendViewChanged();
}

/************************************************************************/
/*************************************************************************
|*    SvInPlaceClipWindow::SvInPlaceClipWindow()
|*
|*    Beschreibung
*************************************************************************/
SvInPlaceClipWindow::SvInPlaceClipWindow( Window * pParent )
    : Window( pParent, WB_CLIPCHILDREN )
    , pResizeWin( NULL )
{
    SetBackground();
    SetAccessibleRole( ::com::sun::star::accessibility::AccessibleRole::EMBEDDED_OBJECT );
}

/*************************************************************************
|*    SvInPlaceClipWindow::SetObjRects()
|*
|*    Beschreibung
*************************************************************************/
void SvInPlaceClipWindow::SetRectsPixel( const Rectangle & rObjRect,
                                         const Rectangle & rInPlaceWinMaxRect )
{
    // Merken fuer Border Aenderungen
    aMaxClip = rInPlaceWinMaxRect;

    // Umschliessendes Rechteck einschliesslich Tools
    Rectangle aOuter( rObjRect );
    aOuter += pResizeWin->GetAllBorderPixel();

    // Clip-Window-Groesse berechnen
    Rectangle aClip( rInPlaceWinMaxRect );
    aClip = aClip.GetIntersection( aOuter );
    SetPosSizePixel( aClip.TopLeft(), aClip.GetSize() );

    // Correction fuer Request setzen
    pResizeWin->SetPosCorrectionPixel( aClip.TopLeft() );

    // Resize-Window relativ zu Clip-Window berechnen
    pResizeWin->SetInnerPosSizePixel( rObjRect.TopLeft() - aClip.TopLeft(),
                                      rObjRect.GetSize() );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
