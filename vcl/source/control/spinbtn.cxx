/*************************************************************************
 *
 *  $RCSfile: spinbtn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:36 $
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

#define _SV_SPIN_CXX

#ifndef _SV_RCID_H
#include <rcid.h>
#endif
#ifndef _SV_SPIN_H
#include <spin.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SPIN_HXX
#include <spin.hxx>
#endif

// =======================================================================

void SpinButton::ImplInit( Window* pParent, WinBits nStyle )
{
    mbUpperIn     = FALSE;
    mbLowerIn     = FALSE;
    mbInitialUp   = FALSE;
    mbInitialDown = FALSE;

    if ( nStyle & WB_REPEAT )
    {
        mbRepeat = TRUE;

        maRepeatTimer.SetTimeout( SPIN_DELAY );
        maRepeatTimer.SetTimeoutHdl( LINK( this, SpinButton, ImplTimeout ) );
    }
    else
        mbRepeat = FALSE;

    if ( nStyle & WB_HSCROLL )
        mbHorz = TRUE;
    else
        mbHorz = FALSE;

    Control::ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

SpinButton::SpinButton( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_SPINBUTTON )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

SpinButton::SpinButton( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_SPINBUTTON )
{
    rResId.SetRT( RSC_SPINBUTTON );
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
    Resize();
}

// -----------------------------------------------------------------------

SpinButton::~SpinButton()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( SpinButton, ImplTimeout, Timer*, pTimer )
{
    if ( pTimer->GetTimeout() == SPIN_DELAY )
    {
        pTimer->SetTimeout( SPIN_SPEED );
        pTimer->Start();
    }
    else
    {
        if ( mbInitialUp )
            Up();
        else
            Down();
    }

    return 0;
}

// -----------------------------------------------------------------------

void SpinButton::Up()
{
    maUpHdlLink.Call( this );
}

// -----------------------------------------------------------------------

void SpinButton::Down()
{
    maDownHdlLink.Call( this );
}

// -----------------------------------------------------------------------

void SpinButton::Resize()
{
    Size aSize( GetOutputSizePixel() );
    Point aTmpPoint;
    Rectangle aRect( aTmpPoint, aSize );
    if ( mbHorz )
    {
        maUpperRect = Rectangle( 0, 0, aSize.Width()/2, aSize.Height()-1 );
        maLowerRect = Rectangle( maUpperRect.TopRight(), aRect.BottomRight() );
    }
    else
    {
        maUpperRect = Rectangle( 0, 0, aSize.Width()-1, aSize.Height()/2 );
        maLowerRect = Rectangle( maUpperRect.BottomLeft(), aRect.BottomRight() );
    }

    Invalidate();
}

// -----------------------------------------------------------------------

void SpinButton::Paint( const Rectangle& )
{
    BOOL bEnable = IsEnabled();
    ImplDrawSpinButton( this, maUpperRect, maLowerRect, mbUpperIn, mbLowerIn,
                        bEnable, bEnable, mbHorz );
}

// -----------------------------------------------------------------------

void SpinButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( maUpperRect.IsInside( rMEvt.GetPosPixel() ) )
    {
        mbUpperIn   = TRUE;
        mbInitialUp = TRUE;
        Invalidate( maUpperRect );
    }
    else if ( maLowerRect.IsInside( rMEvt.GetPosPixel() ) )
    {
        mbLowerIn     = TRUE;
        mbInitialDown = TRUE;
        Invalidate( maLowerRect );
    }

    if ( mbUpperIn || mbLowerIn )
    {
        Update();
        CaptureMouse();
        if ( mbRepeat )
            maRepeatTimer.Start();
    }
}

// -----------------------------------------------------------------------

void SpinButton::MouseButtonUp( const MouseEvent& )
{
    ReleaseMouse();

    if ( mbUpperIn )
    {
        mbUpperIn   = FALSE;
        Invalidate( maUpperRect );
        Update();
        Up();
    }
    else if ( mbLowerIn )
    {
        mbLowerIn = FALSE;
        Invalidate( maLowerRect );
        Update();
        Down();
    }

    mbInitialUp = mbInitialDown = FALSE;

    if ( mbRepeat )
    {
        maRepeatTimer.Stop();
        maRepeatTimer.SetTimeout( SPIN_DELAY );
    }
}

// -----------------------------------------------------------------------

void SpinButton::MouseMove( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() || (!mbInitialUp && !mbInitialDown) )
        return;

    if ( !maUpperRect.IsInside( rMEvt.GetPosPixel() ) &&
         mbUpperIn && mbInitialUp )
    {
        mbUpperIn = FALSE;
        maRepeatTimer.Stop();
        Invalidate( maUpperRect );
        Update();
    }
    else if ( !maLowerRect.IsInside( rMEvt.GetPosPixel() ) &&
              mbLowerIn & mbInitialDown )
    {
        mbLowerIn = FALSE;
        maRepeatTimer.Stop();
        Invalidate( maLowerRect );
        Update();
    }
    else if ( maUpperRect.IsInside( rMEvt.GetPosPixel() ) &&
              !mbUpperIn && mbInitialUp )
    {
        mbUpperIn = TRUE;
        if ( mbRepeat )
            maRepeatTimer.Start();
        Invalidate( maUpperRect );
        Update();
    }
    else if ( maLowerRect.IsInside( rMEvt.GetPosPixel() ) &&
              !mbLowerIn && mbInitialDown )
    {
        mbLowerIn = TRUE;
        if ( mbRepeat )
            maRepeatTimer.Start();
        Invalidate( maLowerRect );
        Update();
    }
}

// -----------------------------------------------------------------------

void SpinButton::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode = rKEvt.GetKeyCode();

    if ( aCode.GetCode() == KEY_UP )
        Up();
    else if( aCode.GetCode() == KEY_DOWN )
        Down();
    else
        Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void SpinButton::StateChanged( StateChangedType nType )
{
    if ( nType == STATE_CHANGE_ENABLE )
        Invalidate();
    Control::StateChanged( nType );
}
