/*************************************************************************
 *
 *  $RCSfile: scrwnd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
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

#include <math.h>
#include <limits.h>
#include <tools/debug.hxx>
#include "svids.hrc"
#include "svdata.hxx"
#include "timer.hxx"
#include "scrwnd.hxx"

// -----------
// - Defines -
// -----------

#define WHEEL_WIDTH     25
#define WHEEL_RADIUS    ((WHEEL_WIDTH) >> 1 )
#define MAX_TIME        300
#define MIN_TIME        20
#define DEF_TIMEOUT     50

// -------------------
// - ImplWheelWindow -
// -------------------

ImplWheelWindow::ImplWheelWindow( Window* pParent ) :
            FloatingWindow  ( pParent, 0 ),
            mnRepaintTime   ( 1UL ),
            mnActDist       ( 0UL ),
            mnWheelMode     ( WHEELMODE_NONE ),
            mnActDeltaX     ( 0L ),
            mnActDeltaY     ( 0L ),
            mnTimeout       ( DEF_TIMEOUT )
{
    // we need a parent
    DBG_ASSERT( pParent, "ImplWheelWindow::ImplWheelWindow(): Parent not set!" );

    const Size      aSize( pParent->GetOutputSizePixel() );
    const USHORT    nFlags = ImplGetSVData()->maWinData.mnAutoScrollFlags;
    const BOOL      bHorz = ( nFlags & AUTOSCROLL_HORZ ) != 0;
    const BOOL      bVert = ( nFlags & AUTOSCROLL_VERT ) != 0;

    // calculate maximum speed distance
    mnMaxWidth = (ULONG) ( 0.4 * hypot( (double) aSize.Width(), aSize.Height() ) );

    // create wheel window
    SetTitleType( FLOATWIN_TITLE_NONE );
    ImplCreateImageList();
    ImplSetRegion( Bitmap( ResId( SV_RESID_BITMAP_SCROLLMSK, ImplGetResMgr() ) ) );

    // set wheel mode
    if( bHorz && bVert )
        ImplSetWheelMode( WHEELMODE_VH );
    else if( bHorz )
        ImplSetWheelMode( WHEELMODE_H );
    else
        ImplSetWheelMode( WHEELMODE_V );

    // init timer
    mpTimer = new Timer;
    mpTimer->SetTimeoutHdl( LINK( this, ImplWheelWindow, ImplScrollHdl ) );
    mpTimer->SetTimeout( mnTimeout );
    mpTimer->Start();

    CaptureMouse();
}

// ------------------------------------------------------------------------

ImplWheelWindow::~ImplWheelWindow()
{
    ReleaseMouse();
    mpTimer->Stop();
    delete mpTimer;
}

// ------------------------------------------------------------------------

void ImplWheelWindow::ImplSetRegion( const Bitmap& rRegionBmp )
{
    Point           aPos( GetPointerPosPixel() );
    const Size      aSize( rRegionBmp.GetSizePixel() );
    Point           aPoint;
    const Rectangle aRect( aPoint, aSize );

    maCenter = maLastMousePos = aPos;
    aPos.X() -= aSize.Width() >> 1;
    aPos.Y() -= aSize.Height() >> 1;

    SetPosSizePixel( aPos, aSize );
    SetWindowRegionPixel( rRegionBmp.CreateRegion( COL_BLACK, aRect ) );
}

// ------------------------------------------------------------------------

void ImplWheelWindow::ImplCreateImageList()
{
    Bitmap aImgBmp( ResId( SV_RESID_BITMAP_SCROLLBMP, ImplGetResMgr() ) );
    maImgList = ImageList( aImgBmp, 6 );
}

// ------------------------------------------------------------------------

void ImplWheelWindow::ImplSetWheelMode( ULONG nWheelMode )
{
    if( nWheelMode != mnWheelMode )
    {
        mnWheelMode = nWheelMode;

        if( WHEELMODE_NONE == mnWheelMode )
        {
            if( IsVisible() )
                Hide();
        }
        else
        {
            if( !IsVisible() )
                Show();

            ImplDrawWheel();
        }
    }
}

// ------------------------------------------------------------------------

void ImplWheelWindow::ImplDrawWheel()
{
    USHORT nId;

    switch( mnWheelMode )
    {
        case( WHEELMODE_VH ):       nId = 1; break;
        case( WHEELMODE_V    ):     nId = 2; break;
        case( WHEELMODE_H    ):     nId = 3; break;
        case( WHEELMODE_SCROLL_VH ):nId = 4; break;
        case( WHEELMODE_SCROLL_V ): nId = 5; break;
        case( WHEELMODE_SCROLL_H ): nId = 6; break;
        default:                    nId = 0; break;
    }

    if( nId )
        DrawImage( Point(), maImgList.GetImage( nId ) );
}

// ------------------------------------------------------------------------

void ImplWheelWindow::ImplRecalcScrollValues()
{
    if( mnActDist < WHEEL_RADIUS )
    {
        mnActDeltaX = mnActDeltaY = 0L;
        mnTimeout = DEF_TIMEOUT;
    }
    else
    {
        ULONG nCurTime;

        // calc current time
        if( mnMaxWidth )
        {
            const double fExp = ( (double) mnActDist / mnMaxWidth ) * log10( (double) MAX_TIME / MIN_TIME );
            nCurTime = (ULONG) ( MAX_TIME / pow( 10., fExp ) );
        }
        else
            nCurTime = MAX_TIME;

        if( !nCurTime )
            nCurTime = 1UL;

        if( mnRepaintTime <= nCurTime )
            mnTimeout = nCurTime - mnRepaintTime;
        else
        {
            long nMult = mnRepaintTime / nCurTime;

            if( !( mnRepaintTime % nCurTime ) )
                mnTimeout = 0UL;
            else
                mnTimeout = ++nMult * nCurTime - mnRepaintTime;

            double fValX = (double) mnActDeltaX * nMult;
            double fValY = (double) mnActDeltaY * nMult;

            if( fValX > LONG_MAX )
                mnActDeltaX = LONG_MAX;
            else if( fValX < LONG_MIN )
                mnActDeltaX = LONG_MIN;
            else
                mnActDeltaX = (long) fValX;

            if( fValY > LONG_MAX )
                mnActDeltaY = LONG_MAX;
            else if( fValY < LONG_MIN )
                mnActDeltaY = LONG_MIN;
            else
                mnActDeltaY = (long) fValY;
        }
    }
}

// ------------------------------------------------------------------------

PointerStyle ImplWheelWindow::ImplGetMousePointer( long nDistX, long nDistY )
{
    PointerStyle    eStyle;
    const USHORT    nFlags = ImplGetSVData()->maWinData.mnAutoScrollFlags;
    const BOOL      bHorz = ( nFlags & AUTOSCROLL_HORZ ) != 0;
    const BOOL      bVert = ( nFlags & AUTOSCROLL_VERT ) != 0;

    if( bHorz || bVert )
    {
        if( mnActDist < WHEEL_RADIUS )
        {
            if( bHorz && bVert )
                eStyle = POINTER_AUTOSCROLL_NSWE;
            else if( bHorz )
                eStyle = POINTER_AUTOSCROLL_WE;
            else
                eStyle = POINTER_AUTOSCROLL_NS;
        }
        else
        {
            double fAngle = atan2( (double) -nDistY, nDistX ) / F_PI180;

            if( fAngle < 0.0 )
                fAngle += 360.;

            if( bHorz && bVert )
            {
                if( fAngle >= 22.5 && fAngle <= 67.5 )
                    eStyle = POINTER_AUTOSCROLL_NE;
                else if( fAngle >= 67.5 && fAngle <= 112.5 )
                    eStyle = POINTER_AUTOSCROLL_N;
                else if( fAngle >= 112.5 && fAngle <= 157.5 )
                    eStyle = POINTER_AUTOSCROLL_NW;
                else if( fAngle >= 157.5 && fAngle <= 202.5 )
                    eStyle = POINTER_AUTOSCROLL_W;
                else if( fAngle >= 202.5 && fAngle <= 247.5 )
                    eStyle = POINTER_AUTOSCROLL_SW;
                else if( fAngle >= 247.5 && fAngle <= 292.5 )
                    eStyle = POINTER_AUTOSCROLL_S;
                else if( fAngle >= 292.5 && fAngle <= 337.5 )
                    eStyle = POINTER_AUTOSCROLL_SE;
                else
                    eStyle = POINTER_AUTOSCROLL_E;
            }
            else if( bHorz )
            {
                if( fAngle >= 270. || fAngle <= 90. )
                    eStyle = POINTER_AUTOSCROLL_E;
                else
                    eStyle = POINTER_AUTOSCROLL_W;
            }
            else
            {
                if( fAngle >= 0. && fAngle <= 180. )
                    eStyle = POINTER_AUTOSCROLL_N;
                else
                    eStyle = POINTER_AUTOSCROLL_S;
            }
        }
    }
    else
        eStyle = POINTER_ARROW;

    return eStyle;
}

// ------------------------------------------------------------------------

void ImplWheelWindow::Paint( const Rectangle& rRect )
{
    ImplDrawWheel();
}

// ------------------------------------------------------------------------

void ImplWheelWindow::MouseMove( const MouseEvent& rMEvt )
{
    FloatingWindow::MouseMove( rMEvt );

    const Point aMousePos( OutputToScreenPixel( rMEvt.GetPosPixel() ) );
    const long  nDistX = aMousePos.X() - maCenter.X();
    const long  nDistY = aMousePos.Y() - maCenter.Y();

    mnActDist = (ULONG) hypot( (double) nDistX, nDistY );

    const PointerStyle  eActStyle = ImplGetMousePointer( nDistX, nDistY );
    const USHORT        nFlags = ImplGetSVData()->maWinData.mnAutoScrollFlags;
    const BOOL          bHorz = ( nFlags & AUTOSCROLL_HORZ ) != 0;
    const BOOL          bVert = ( nFlags & AUTOSCROLL_VERT ) != 0;
    const BOOL          bOuter = mnActDist > WHEEL_RADIUS;

    if( bOuter && ( maLastMousePos != aMousePos ) )
    {
        switch( eActStyle )
        {
            case( POINTER_AUTOSCROLL_N ):   mnActDeltaX = +0L, mnActDeltaY = +1L; break;
            case( POINTER_AUTOSCROLL_S ):   mnActDeltaX = +0L, mnActDeltaY = -1L; break;
            case( POINTER_AUTOSCROLL_W ):   mnActDeltaX = +1L, mnActDeltaY = +0L; break;
            case( POINTER_AUTOSCROLL_E ):   mnActDeltaX = -1L, mnActDeltaY = +0L; break;
            case( POINTER_AUTOSCROLL_NW ):  mnActDeltaX = +1L, mnActDeltaY = +1L; break;
            case( POINTER_AUTOSCROLL_NE ):  mnActDeltaX = -1L, mnActDeltaY = +1L; break;
            case( POINTER_AUTOSCROLL_SW ):  mnActDeltaX = +1L, mnActDeltaY = -1L; break;
            case( POINTER_AUTOSCROLL_SE ):  mnActDeltaX = -1L, mnActDeltaY = -1L; break;

            default:
            break;
        }
    }

    ImplRecalcScrollValues();
    maLastMousePos = aMousePos;
    SetPointer( eActStyle );

    if( bHorz && bVert )
        ImplSetWheelMode( bOuter ? WHEELMODE_SCROLL_VH : WHEELMODE_VH );
    else if( bHorz )
        ImplSetWheelMode( bOuter ? WHEELMODE_SCROLL_H : WHEELMODE_H );
    else
        ImplSetWheelMode( bOuter ? WHEELMODE_SCROLL_V : WHEELMODE_V );
}

// ------------------------------------------------------------------------

void ImplWheelWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( mnActDist > WHEEL_RADIUS )
        GetParent()->EndAutoScroll();
    else
        FloatingWindow::MouseButtonUp( rMEvt );
}

// ------------------------------------------------------------------------

IMPL_LINK( ImplWheelWindow, ImplScrollHdl, Timer*, pTimer )
{
    if ( mnActDeltaX || mnActDeltaY )
    {
        Window*             pWindow = GetParent();
        const Point         aMousePos( pWindow->OutputToScreenPixel( pWindow->GetPointerPosPixel() ) );
        Point               aCmdMousePos( pWindow->ImplFrameToOutput( aMousePos ) );
        CommandScrollData   aScrollData( mnActDeltaX, mnActDeltaY );
        CommandEvent        aCEvt( aCmdMousePos, COMMAND_AUTOSCROLL, TRUE, &aScrollData );
        NotifyEvent         aNCmdEvt( EVENT_COMMAND, pWindow, &aCEvt );

        if ( !ImplCallPreNotify( aNCmdEvt ) )
        {
            const ULONG nTime = Time::GetSystemTicks();
            pWindow->Command( aCEvt );
            mnRepaintTime = Max( Time::GetSystemTicks() - nTime, 1UL );
            ImplRecalcScrollValues();
        }
    }

    if ( mnTimeout != mpTimer->GetTimeout() )
        mpTimer->SetTimeout( mnTimeout );
    mpTimer->Start();

    return 0L;
}
