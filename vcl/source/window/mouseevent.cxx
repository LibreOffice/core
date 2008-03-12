/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mouseevent.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:05:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include <vcl/event.hxx>

/** inits this vcl KeyEvent with all settings from the given awt event **/
MouseEvent::MouseEvent( const ::com::sun::star::awt::MouseEvent& rEvent )
: maPos( rEvent.X, rEvent.Y )
, mnMode( 0 )
, mnClicks( static_cast< USHORT >( rEvent.ClickCount ) )
, mnCode( 0 )
{
    if( rEvent.Modifiers )
    {
        if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::SHIFT) != 0 )
            mnCode |= KEY_SHIFT;
        if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD1) != 0 )
            mnCode |= KEY_MOD1;
        if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD2) != 0 )
            mnCode |= KEY_MOD2;
    }

    if( rEvent.Buttons )
    {
        if( (rEvent.Buttons & ::com::sun::star::awt::MouseButton::LEFT) != 0 )
            mnCode |= MOUSE_LEFT;
        if( (rEvent.Buttons & ::com::sun::star::awt::MouseButton::RIGHT) != 0 )
            mnCode |= MOUSE_RIGHT;
        if( (rEvent.Buttons & ::com::sun::star::awt::MouseButton::MIDDLE) != 0 )
            mnCode |= MOUSE_MIDDLE;
    }
}

/** fills out the given awt KeyEvent with all settings from this vcl event **/
void MouseEvent::InitMouseEvent( ::com::sun::star::awt::MouseEvent& rEvent ) const
{
    rEvent.Modifiers = 0;
    if ( IsShift() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::SHIFT;
    if ( IsMod1() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD1;
    if ( IsMod2() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD2;

    rEvent.Buttons = 0;
    if ( IsLeft() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::LEFT;
    if ( IsRight() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::RIGHT;
    if ( IsMiddle() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::MIDDLE;

    rEvent.X = GetPosPixel().X();
    rEvent.Y = GetPosPixel().Y();
    rEvent.ClickCount = GetClicks();
    rEvent.PopupTrigger = sal_False;
}
