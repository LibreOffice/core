/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: keyevent.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_vcl.hxx"

#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <tools/debug.hxx>
#include <vcl/event.hxx>

KeyEvent::KeyEvent (const KeyEvent& rKeyEvent) :
    maKeyCode (rKeyEvent.maKeyCode),
    mnRepeat  (rKeyEvent.mnRepeat),
    mnCharCode(rKeyEvent.mnCharCode)
{}

/** inits this vcl KeyEvent with all settings from the given awt event **/
KeyEvent::KeyEvent( const ::com::sun::star::awt::KeyEvent& rEvent )
{
    maKeyCode = KeyCode(
        rEvent.KeyCode,
        (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::SHIFT) != 0,
        (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD1) != 0,
        (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD2) != 0 );
    mnRepeat = 0;
    mnCharCode = rEvent.KeyChar;
}

/** fills out the given awt KeyEvent with all settings from this vcl event **/
void KeyEvent::InitKeyEvent( ::com::sun::star::awt::KeyEvent& rEvent ) const
{
    rEvent.Modifiers = 0;
    if( GetKeyCode().IsShift() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::SHIFT;
    if( GetKeyCode().IsMod1() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD1;
    if( GetKeyCode().IsMod2() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD2;

    rEvent.KeyCode = GetKeyCode().GetCode();
    rEvent.KeyChar = GetCharCode();
    rEvent.KeyFunc = sal::static_int_cast< sal_Int16 >(GetKeyCode().GetFunction());
}

KeyEvent KeyEvent::LogicalTextDirectionality (TextDirectionality eMode) const
{
    KeyEvent aClone(*this);

    USHORT nCode = maKeyCode.GetCode();
    USHORT nMod  = maKeyCode.GetAllModifier();

    switch (eMode)
    {
        case TextDirectionality_RightToLeft_TopToBottom:
            switch (nCode)
            {
                case KEY_LEFT:  aClone.maKeyCode = KeyCode(KEY_RIGHT, nMod); break;
                case KEY_RIGHT: aClone.maKeyCode = KeyCode(KEY_LEFT,  nMod); break;
            }
            break;

        case TextDirectionality_TopToBottom_RightToLeft:
            switch (nCode)
            {
                case KEY_DOWN:  aClone.maKeyCode = KeyCode(KEY_RIGHT, nMod); break;
                case KEY_UP:    aClone.maKeyCode = KeyCode(KEY_LEFT,  nMod); break;
                case KEY_LEFT:  aClone.maKeyCode = KeyCode(KEY_DOWN,  nMod); break;
                case KEY_RIGHT: aClone.maKeyCode = KeyCode(KEY_UP,    nMod); break;
            }
            break;

        case TextDirectionality_LeftToRight_TopToBottom:
            /* do nothing */
            break;
    }

    return aClone;
}


// -------------------------------------------------------

const Point&   HelpEvent::GetMousePosPixel() const
{
    //DBG_ASSERT( !mbKeyboardActivated, "Keyboard help has no mouse position !");
    return maPos;
}

