/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: keycodes.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SV_KEYCODES_HXX
#define _SV_KEYCODES_HXX

#include <vcl/sv.h>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyGroup.hpp>

// -----------------
// - KeyCode-Types -
// -----------------

// By changes you must also change: rsc/vclrsc.hxx

// Key-Gruppen
#define KEYGROUP_NUM    ((USHORT)::com::sun::star::awt::KeyGroup::NUM)
#define KEYGROUP_ALPHA  ((USHORT)::com::sun::star::awt::KeyGroup::ALPHA)
#define KEYGROUP_FKEYS  ((USHORT)::com::sun::star::awt::KeyGroup::FKEYS)
#define KEYGROUP_CURSOR ((USHORT)::com::sun::star::awt::KeyGroup::CURSOR)
#define KEYGROUP_MISC   ((USHORT)::com::sun::star::awt::KeyGroup::MISC)
#define KEYGROUP_TYPE   ((USHORT)::com::sun::star::awt::KeyGroup::TYPE)

// Key-Codes
#define KEY_0           ((USHORT)::com::sun::star::awt::Key::NUM0)
#define KEY_1           ((USHORT)::com::sun::star::awt::Key::NUM1)
#define KEY_2           ((USHORT)::com::sun::star::awt::Key::NUM2)
#define KEY_3           ((USHORT)::com::sun::star::awt::Key::NUM3)
#define KEY_4           ((USHORT)::com::sun::star::awt::Key::NUM4)
#define KEY_5           ((USHORT)::com::sun::star::awt::Key::NUM5)
#define KEY_6           ((USHORT)::com::sun::star::awt::Key::NUM6)
#define KEY_7           ((USHORT)::com::sun::star::awt::Key::NUM7)
#define KEY_8           ((USHORT)::com::sun::star::awt::Key::NUM8)
#define KEY_9           ((USHORT)::com::sun::star::awt::Key::NUM9)

#define KEY_A           ((USHORT)::com::sun::star::awt::Key::A)
#define KEY_B           ((USHORT)::com::sun::star::awt::Key::B)
#define KEY_C           ((USHORT)::com::sun::star::awt::Key::C)
#define KEY_D           ((USHORT)::com::sun::star::awt::Key::D)
#define KEY_E           ((USHORT)::com::sun::star::awt::Key::E)
#define KEY_F           ((USHORT)::com::sun::star::awt::Key::F)
#define KEY_G           ((USHORT)::com::sun::star::awt::Key::G)
#define KEY_H           ((USHORT)::com::sun::star::awt::Key::H)
#define KEY_I           ((USHORT)::com::sun::star::awt::Key::I)
#define KEY_J           ((USHORT)::com::sun::star::awt::Key::J)
#define KEY_K           ((USHORT)::com::sun::star::awt::Key::K)
#define KEY_L           ((USHORT)::com::sun::star::awt::Key::L)
#define KEY_M           ((USHORT)::com::sun::star::awt::Key::M)
#define KEY_N           ((USHORT)::com::sun::star::awt::Key::N)
#define KEY_O           ((USHORT)::com::sun::star::awt::Key::O)
#define KEY_P           ((USHORT)::com::sun::star::awt::Key::P)
#define KEY_Q           ((USHORT)::com::sun::star::awt::Key::Q)
#define KEY_R           ((USHORT)::com::sun::star::awt::Key::R)
#define KEY_S           ((USHORT)::com::sun::star::awt::Key::S)
#define KEY_T           ((USHORT)::com::sun::star::awt::Key::T)
#define KEY_U           ((USHORT)::com::sun::star::awt::Key::U)
#define KEY_V           ((USHORT)::com::sun::star::awt::Key::V)
#define KEY_W           ((USHORT)::com::sun::star::awt::Key::W)
#define KEY_X           ((USHORT)::com::sun::star::awt::Key::X)
#define KEY_Y           ((USHORT)::com::sun::star::awt::Key::Y)
#define KEY_Z           ((USHORT)::com::sun::star::awt::Key::Z)

#define KEY_F1          ((USHORT)::com::sun::star::awt::Key::F1)
#define KEY_F2          ((USHORT)::com::sun::star::awt::Key::F2)
#define KEY_F3          ((USHORT)::com::sun::star::awt::Key::F3)
#define KEY_F4          ((USHORT)::com::sun::star::awt::Key::F4)
#define KEY_F5          ((USHORT)::com::sun::star::awt::Key::F5)
#define KEY_F6          ((USHORT)::com::sun::star::awt::Key::F6)
#define KEY_F7          ((USHORT)::com::sun::star::awt::Key::F7)
#define KEY_F8          ((USHORT)::com::sun::star::awt::Key::F8)
#define KEY_F9          ((USHORT)::com::sun::star::awt::Key::F9)
#define KEY_F10         ((USHORT)::com::sun::star::awt::Key::F10)
#define KEY_F11         ((USHORT)::com::sun::star::awt::Key::F11)
#define KEY_F12         ((USHORT)::com::sun::star::awt::Key::F12)
#define KEY_F13         ((USHORT)::com::sun::star::awt::Key::F13)
#define KEY_F14         ((USHORT)::com::sun::star::awt::Key::F14)
#define KEY_F15         ((USHORT)::com::sun::star::awt::Key::F15)
#define KEY_F16         ((USHORT)::com::sun::star::awt::Key::F16)
#define KEY_F17         ((USHORT)::com::sun::star::awt::Key::F17)
#define KEY_F18         ((USHORT)::com::sun::star::awt::Key::F18)
#define KEY_F19         ((USHORT)::com::sun::star::awt::Key::F19)
#define KEY_F20         ((USHORT)::com::sun::star::awt::Key::F20)
#define KEY_F21         ((USHORT)::com::sun::star::awt::Key::F21)
#define KEY_F22         ((USHORT)::com::sun::star::awt::Key::F22)
#define KEY_F23         ((USHORT)::com::sun::star::awt::Key::F23)
#define KEY_F24         ((USHORT)::com::sun::star::awt::Key::F24)
#define KEY_F25         ((USHORT)::com::sun::star::awt::Key::F25)
#define KEY_F26         ((USHORT)::com::sun::star::awt::Key::F26)

#define KEY_DOWN        ((USHORT)::com::sun::star::awt::Key::DOWN)
#define KEY_UP          ((USHORT)::com::sun::star::awt::Key::UP)
#define KEY_LEFT        ((USHORT)::com::sun::star::awt::Key::LEFT)
#define KEY_RIGHT       ((USHORT)::com::sun::star::awt::Key::RIGHT)
#define KEY_HOME        ((USHORT)::com::sun::star::awt::Key::HOME)
#define KEY_END         ((USHORT)::com::sun::star::awt::Key::END)
#define KEY_PAGEUP      ((USHORT)::com::sun::star::awt::Key::PAGEUP)
#define KEY_PAGEDOWN    ((USHORT)::com::sun::star::awt::Key::PAGEDOWN)

#define KEY_RETURN      ((USHORT)::com::sun::star::awt::Key::RETURN)
#define KEY_ESCAPE      ((USHORT)::com::sun::star::awt::Key::ESCAPE)
#define KEY_TAB         ((USHORT)::com::sun::star::awt::Key::TAB)
#define KEY_BACKSPACE   ((USHORT)::com::sun::star::awt::Key::BACKSPACE)
#define KEY_SPACE       ((USHORT)::com::sun::star::awt::Key::SPACE)
#define KEY_INSERT      ((USHORT)::com::sun::star::awt::Key::INSERT)
#define KEY_DELETE      ((USHORT)::com::sun::star::awt::Key::DELETE)

#define KEY_ADD         ((USHORT)::com::sun::star::awt::Key::ADD)
#define KEY_SUBTRACT    ((USHORT)::com::sun::star::awt::Key::SUBTRACT)
#define KEY_MULTIPLY    ((USHORT)::com::sun::star::awt::Key::MULTIPLY)
#define KEY_DIVIDE      ((USHORT)::com::sun::star::awt::Key::DIVIDE)
#define KEY_POINT       ((USHORT)::com::sun::star::awt::Key::POINT)
#define KEY_COMMA       ((USHORT)::com::sun::star::awt::Key::COMMA)
#define KEY_LESS        ((USHORT)::com::sun::star::awt::Key::LESS)
#define KEY_GREATER     ((USHORT)::com::sun::star::awt::Key::GREATER)
#define KEY_EQUAL       ((USHORT)::com::sun::star::awt::Key::EQUAL)

#define KEY_OPEN        ((USHORT)::com::sun::star::awt::Key::OPEN)
#define KEY_CUT         ((USHORT)::com::sun::star::awt::Key::CUT)
#define KEY_COPY        ((USHORT)::com::sun::star::awt::Key::COPY)
#define KEY_PASTE       ((USHORT)::com::sun::star::awt::Key::PASTE)
#define KEY_UNDO        ((USHORT)::com::sun::star::awt::Key::UNDO)
#define KEY_REPEAT      ((USHORT)::com::sun::star::awt::Key::REPEAT)
#define KEY_FIND        ((USHORT)::com::sun::star::awt::Key::FIND)
#define KEY_PROPERTIES  ((USHORT)::com::sun::star::awt::Key::PROPERTIES)
#define KEY_FRONT       ((USHORT)::com::sun::star::awt::Key::FRONT)
#define KEY_CONTEXTMENU ((USHORT)::com::sun::star::awt::Key::CONTEXTMENU)
#define KEY_MENU        ((USHORT)::com::sun::star::awt::Key::MENU)
#define KEY_HELP        ((USHORT)::com::sun::star::awt::Key::HELP)
#define KEY_HANGUL_HANJA ((USHORT)::com::sun::star::awt::Key::HANGUL_HANJA)
#define KEY_DECIMAL     ((USHORT)::com::sun::star::awt::Key::DECIMAL)
#define KEY_TILDE       ((USHORT)::com::sun::star::awt::Key::TILDE)
#define KEY_QUOTELEFT   ((USHORT)::com::sun::star::awt::Key::QUOTELEFT)

#define KEY_CODE        ((USHORT)0x0FFF)

// Modifier-Tasten
#define KEY_SHIFT       ((USHORT)0x1000)
#define KEY_MOD1        ((USHORT)0x2000)
#define KEY_MOD2        ((USHORT)0x4000)
#define KEY_MOD3    ((USHORT)0x8000)
#define KEY_MODTYPE ((USHORT)0xF000)
#define KEY_ALLMODTYPE  ((USHORT)0xF000)

// extended Modifier-Keys (only used for modkey events)
#define MODKEY_LSHIFT    0x0001
#define MODKEY_RSHIFT    0x0002
#define MODKEY_LMOD1     0x0004
#define MODKEY_RMOD1     0x0008
#define MODKEY_LMOD2     0x0010
#define MODKEY_RMOD2     0x0020
#define MODKEY_SHIFT     (MODKEY_LSHIFT|MODKEY_RSHIFT)
#define MODKEY_MOD1      (MODKEY_LMOD1|MODKEY_RMOD1)
#define MODKEY_MOD2      (MODKEY_LMOD2|MODKEY_RMOD2)

// ---------------
// - Mouse-Types -
// ---------------

#define MOUSE_LEFT      ((USHORT)0x0001)
#define MOUSE_MIDDLE    ((USHORT)0x0002)
#define MOUSE_RIGHT     ((USHORT)0x0004)

#endif // _SV_KEYCODES_HXX
