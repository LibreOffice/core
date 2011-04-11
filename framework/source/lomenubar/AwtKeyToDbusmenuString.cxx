/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * A LibreOffice extension to send the menubar structure through DBusMenu
 *
 * Copyright 2011 Canonical, Ltd.
 * Authors:
 *     Alberto Ruiz <alberto.ruiz@codethink.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the the GNU Lesser General Public License version 3, as published by the Free
 * Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR PURPOSE.  See the applicable
 * version of the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with this program. If not, see <http://www.gnu.org/licenses/>
 *
 */

#define XK_MISCELLANY
#define XK_LATIN1
#define XK_PUBLISHING

#include <X11/Xlib.h>
#include <X11/keysymdef.h>

#include <glib.h>

#include <com/sun/star/awt/Key.hpp>

using namespace ::com::sun::star;

KeySym MapAwtToXKey (guint16 aKeyCode);

gchar*
AwtKeyToDbusmenuString (guint16 aKeyCode)
{
    KeySym code = (KeySym)0;

    if (!aKeyCode)
        return NULL;

    //We try the consecutive ranges first, if the code is not there
    //we use MapAwtToXKey(), se below
    if (aKeyCode >= awt::Key::A && aKeyCode <= awt::Key::Z)
        code = (KeySym)aKeyCode - (KeySym)awt::Key::A + XK_A;
    else if (aKeyCode >= awt::Key::F1 && aKeyCode <= awt::Key::F26)
        code = (KeySym)aKeyCode - (KeySym)awt::Key::F1 + XK_F1;
    else if (aKeyCode >= awt::Key::NUM0 && aKeyCode <= awt::Key::NUM9)
        code = (KeySym)aKeyCode - (KeySym)awt::Key::NUM0 + XK_0;
    else
        code = MapAwtToXKey (aKeyCode);

    if (code == 0)
        return NULL;

    return XKeysymToString (code);;
}


//This is a 1-1 mapper between com::sun::star::awt:Key values and X11 KeySyms
//note that some symbols are missing
KeySym
MapAwtToXKey (guint16 aKeyCode)
{
    switch (aKeyCode)
    {
        case awt::Key::UP:
            return XK_Up;
        case awt::Key::DOWN:
            return XK_Down;
        case awt::Key::LEFT:
            return XK_Left;
        case awt::Key::RIGHT:
            return XK_Right;
        case awt::Key::HOME:
            return XK_Home;
        case awt::Key::END:
            return XK_End;
        case awt::Key::PAGEUP:
            return XK_Page_Up;
        case awt::Key::PAGEDOWN:
            return XK_Page_Down;
        case awt::Key::RETURN:
            return XK_Return;
        case awt::Key::ESCAPE:
            return XK_Escape;
        case awt::Key::TAB:
            return XK_Tab;
        case awt::Key::BACKSPACE:
            return XK_BackSpace;
        case awt::Key::SPACE:
            return XK_space;
        case awt::Key::INSERT:
            return XK_Insert;
        case awt::Key::DELETE:
            return XK_Delete;
        case awt::Key::ADD:
            return XK_plus;
        case awt::Key::SUBTRACT:
            return XK_minus;
        case awt::Key::MULTIPLY:
            return XK_asterisk;
        case awt::Key::DIVIDE:
            return XK_slash;
        case awt::Key::POINT:
            return XK_period;
        case awt::Key::COMMA:
            return XK_comma;
        case awt::Key::LESS:
            return XK_less;
        case awt::Key::GREATER:
            return XK_greater;
        case awt::Key::EQUAL:
            return XK_equal;
        case awt::Key::UNDO:
            return XK_Undo;
        case awt::Key::REPEAT:
            return XK_Redo;
        case awt::Key::FIND:
            return XK_Find;
        case awt::Key::DECIMAL:
            return XK_decimalpoint;
        case awt::Key::TILDE:
            return XK_asciitilde;
        case awt::Key::QUOTELEFT:
            return XK_leftsinglequotemark;
        //Sun keys and other unsupported symbols
        case awt::Key::OPEN:
        case awt::Key::CUT:
        case awt::Key::COPY:
        case awt::Key::PASTE:
        case awt::Key::PROPERTIES:
        case awt::Key::FRONT:
        case awt::Key::CONTEXTMENU:
        case awt::Key::HELP:
        case awt::Key::MENU:
        default:
            return 0;
    }
}
