/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: keyevent.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:19:27 $
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

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include <event.hxx>

KeyEvent::KeyEvent (const KeyEvent& rKeyEvent) :
    maKeyCode (rKeyEvent.maKeyCode),
    mnRepeat  (rKeyEvent.mnRepeat),
    mnCharCode(rKeyEvent.mnCharCode)
{}

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

