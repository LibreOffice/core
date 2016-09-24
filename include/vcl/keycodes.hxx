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

#ifndef INCLUDED_VCL_KEYCODES_HXX
#define INCLUDED_VCL_KEYCODES_HXX

#include <rsc/rsc-vcl-shared-types.hxx>
#undef DELETE
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyGroup.hpp>
#include <o3tl/typed_flags_set.hxx>

// Key-Gruppen
#define KEYGROUP_NUM    ((sal_uInt16)css::awt::KeyGroup::NUM)
#define KEYGROUP_ALPHA  ((sal_uInt16)css::awt::KeyGroup::ALPHA)
#define KEYGROUP_FKEYS  ((sal_uInt16)css::awt::KeyGroup::FKEYS)
#define KEYGROUP_CURSOR ((sal_uInt16)css::awt::KeyGroup::CURSOR)
#define KEYGROUP_MISC   ((sal_uInt16)css::awt::KeyGroup::MISC)
#define KEYGROUP_TYPE   ((sal_uInt16)css::awt::KeyGroup::TYPE)

// Key-Codes
#define KEY_0           ((sal_uInt16)css::awt::Key::NUM0)
#define KEY_1           ((sal_uInt16)css::awt::Key::NUM1)
#define KEY_2           ((sal_uInt16)css::awt::Key::NUM2)
#define KEY_3           ((sal_uInt16)css::awt::Key::NUM3)
#define KEY_4           ((sal_uInt16)css::awt::Key::NUM4)
#define KEY_5           ((sal_uInt16)css::awt::Key::NUM5)
#define KEY_6           ((sal_uInt16)css::awt::Key::NUM6)
#define KEY_7           ((sal_uInt16)css::awt::Key::NUM7)
#define KEY_8           ((sal_uInt16)css::awt::Key::NUM8)
#define KEY_9           ((sal_uInt16)css::awt::Key::NUM9)

#define KEY_A           ((sal_uInt16)css::awt::Key::A)
#define KEY_B           ((sal_uInt16)css::awt::Key::B)
#define KEY_C           ((sal_uInt16)css::awt::Key::C)
#define KEY_D           ((sal_uInt16)css::awt::Key::D)
#define KEY_E           ((sal_uInt16)css::awt::Key::E)
#define KEY_F           ((sal_uInt16)css::awt::Key::F)
#define KEY_G           ((sal_uInt16)css::awt::Key::G)
#define KEY_H           ((sal_uInt16)css::awt::Key::H)
#define KEY_I           ((sal_uInt16)css::awt::Key::I)
#define KEY_J           ((sal_uInt16)css::awt::Key::J)
#define KEY_K           ((sal_uInt16)css::awt::Key::K)
#define KEY_L           ((sal_uInt16)css::awt::Key::L)
#define KEY_M           ((sal_uInt16)css::awt::Key::M)
#define KEY_N           ((sal_uInt16)css::awt::Key::N)
#define KEY_O           ((sal_uInt16)css::awt::Key::O)
#define KEY_P           ((sal_uInt16)css::awt::Key::P)
#define KEY_Q           ((sal_uInt16)css::awt::Key::Q)
#define KEY_R           ((sal_uInt16)css::awt::Key::R)
#define KEY_S           ((sal_uInt16)css::awt::Key::S)
#define KEY_T           ((sal_uInt16)css::awt::Key::T)
#define KEY_U           ((sal_uInt16)css::awt::Key::U)
#define KEY_V           ((sal_uInt16)css::awt::Key::V)
#define KEY_W           ((sal_uInt16)css::awt::Key::W)
#define KEY_X           ((sal_uInt16)css::awt::Key::X)
#define KEY_Y           ((sal_uInt16)css::awt::Key::Y)
#define KEY_Z           ((sal_uInt16)css::awt::Key::Z)

#define KEY_F1          ((sal_uInt16)css::awt::Key::F1)
#define KEY_F2          ((sal_uInt16)css::awt::Key::F2)
#define KEY_F3          ((sal_uInt16)css::awt::Key::F3)
#define KEY_F4          ((sal_uInt16)css::awt::Key::F4)
#define KEY_F5          ((sal_uInt16)css::awt::Key::F5)
#define KEY_F6          ((sal_uInt16)css::awt::Key::F6)
#define KEY_F7          ((sal_uInt16)css::awt::Key::F7)
#define KEY_F8          ((sal_uInt16)css::awt::Key::F8)
#define KEY_F9          ((sal_uInt16)css::awt::Key::F9)
#define KEY_F10         ((sal_uInt16)css::awt::Key::F10)
#define KEY_F11         ((sal_uInt16)css::awt::Key::F11)
#define KEY_F12         ((sal_uInt16)css::awt::Key::F12)
#define KEY_F13         ((sal_uInt16)css::awt::Key::F13)
#define KEY_F14         ((sal_uInt16)css::awt::Key::F14)
#define KEY_F15         ((sal_uInt16)css::awt::Key::F15)
#define KEY_F16         ((sal_uInt16)css::awt::Key::F16)
#define KEY_F17         ((sal_uInt16)css::awt::Key::F17)
#define KEY_F18         ((sal_uInt16)css::awt::Key::F18)
#define KEY_F19         ((sal_uInt16)css::awt::Key::F19)
#define KEY_F20         ((sal_uInt16)css::awt::Key::F20)
#define KEY_F21         ((sal_uInt16)css::awt::Key::F21)
#define KEY_F22         ((sal_uInt16)css::awt::Key::F22)
#define KEY_F23         ((sal_uInt16)css::awt::Key::F23)
#define KEY_F24         ((sal_uInt16)css::awt::Key::F24)
#define KEY_F25         ((sal_uInt16)css::awt::Key::F25)
#define KEY_F26         ((sal_uInt16)css::awt::Key::F26)

#define KEY_DOWN        ((sal_uInt16)css::awt::Key::DOWN)
#define KEY_UP          ((sal_uInt16)css::awt::Key::UP)
#define KEY_LEFT        ((sal_uInt16)css::awt::Key::LEFT)
#define KEY_RIGHT       ((sal_uInt16)css::awt::Key::RIGHT)
#define KEY_HOME        ((sal_uInt16)css::awt::Key::HOME)
#define KEY_END         ((sal_uInt16)css::awt::Key::END)
#define KEY_PAGEUP      ((sal_uInt16)css::awt::Key::PAGEUP)
#define KEY_PAGEDOWN    ((sal_uInt16)css::awt::Key::PAGEDOWN)

#define KEY_RETURN      ((sal_uInt16)css::awt::Key::RETURN)
#define KEY_ESCAPE      ((sal_uInt16)css::awt::Key::ESCAPE)
#define KEY_TAB         ((sal_uInt16)css::awt::Key::TAB)
#define KEY_BACKSPACE   ((sal_uInt16)css::awt::Key::BACKSPACE)
#define KEY_SPACE       ((sal_uInt16)css::awt::Key::SPACE)
#define KEY_INSERT      ((sal_uInt16)css::awt::Key::INSERT)
#define KEY_DELETE      ((sal_uInt16)css::awt::Key::DELETE)

#define KEY_ADD         ((sal_uInt16)css::awt::Key::ADD)
#define KEY_SUBTRACT    ((sal_uInt16)css::awt::Key::SUBTRACT)
#define KEY_MULTIPLY    ((sal_uInt16)css::awt::Key::MULTIPLY)
#define KEY_DIVIDE      ((sal_uInt16)css::awt::Key::DIVIDE)
#define KEY_POINT       ((sal_uInt16)css::awt::Key::POINT)
#define KEY_COMMA       ((sal_uInt16)css::awt::Key::COMMA)
#define KEY_LESS        ((sal_uInt16)css::awt::Key::LESS)
#define KEY_GREATER     ((sal_uInt16)css::awt::Key::GREATER)
#define KEY_EQUAL       ((sal_uInt16)css::awt::Key::EQUAL)

#define KEY_OPEN        ((sal_uInt16)css::awt::Key::OPEN)
#define KEY_CUT         ((sal_uInt16)css::awt::Key::CUT)
#define KEY_COPY        ((sal_uInt16)css::awt::Key::COPY)
#define KEY_PASTE       ((sal_uInt16)css::awt::Key::PASTE)
#define KEY_UNDO        ((sal_uInt16)css::awt::Key::UNDO)
#define KEY_REPEAT      ((sal_uInt16)css::awt::Key::REPEAT)
#define KEY_FIND        ((sal_uInt16)css::awt::Key::FIND)
#define KEY_PROPERTIES  ((sal_uInt16)css::awt::Key::PROPERTIES)
#define KEY_FRONT       ((sal_uInt16)css::awt::Key::FRONT)
#define KEY_CONTEXTMENU ((sal_uInt16)css::awt::Key::CONTEXTMENU)
#define KEY_MENU        ((sal_uInt16)css::awt::Key::MENU)
#define KEY_HELP        ((sal_uInt16)css::awt::Key::HELP)
#define KEY_HANGUL_HANJA ((sal_uInt16)css::awt::Key::HANGUL_HANJA)
#define KEY_DECIMAL     ((sal_uInt16)css::awt::Key::DECIMAL)
#define KEY_TILDE       ((sal_uInt16)css::awt::Key::TILDE)
#define KEY_QUOTELEFT   ((sal_uInt16)css::awt::Key::QUOTELEFT)
#define KEY_BRACKETLEFT ((sal_uInt16)css::awt::Key::BRACKETLEFT)
#define KEY_BRACKETRIGHT ((sal_uInt16)css::awt::Key::BRACKETRIGHT)
#define KEY_SEMICOLON   ((sal_uInt16)css::awt::Key::SEMICOLON)
#define KEY_QUOTERIGHT  ((sal_uInt16)css::awt::Key::QUOTERIGHT)

#define KEY_CAPSLOCK    ((sal_uInt16)css::awt::Key::CAPSLOCK)
#define KEY_NUMLOCK     ((sal_uInt16)css::awt::Key::NUMLOCK)
#define KEY_SCROLLLOCK  ((sal_uInt16)css::awt::Key::SCROLLLOCK)

// extended Modifier-Keys (only used for modkey events)
#define MODKEY_LSHIFT    0x0001
#define MODKEY_RSHIFT    0x0002
#define MODKEY_LMOD1     0x0004
#define MODKEY_RMOD1     0x0008
#define MODKEY_LMOD2     0x0010
#define MODKEY_RMOD2     0x0020
#define MODKEY_LMOD3     0x0040
#define MODKEY_RMOD3     0x0080
#define MODKEY_SHIFT     (MODKEY_LSHIFT|MODKEY_RSHIFT)
#define MODKEY_MOD1      (MODKEY_LMOD1|MODKEY_RMOD1)
#define MODKEY_MOD2      (MODKEY_LMOD2|MODKEY_RMOD2)
#define MODKEY_MOD3      (MODKEY_LMOD3|MODKEY_RMOD3)

enum class KeyIndicatorState {
    NONE          = 0x0000,
    CAPSLOCK      = 0x0001,
    NUMLOCK       = 0x0002,
    SCROLLLOCK    = 0x0004
};
namespace o3tl
{
    template<> struct typed_flags<KeyIndicatorState> : is_typed_flags<KeyIndicatorState, 0x0007> {};
}

#endif // INCLUDED_VCL_KEYCODES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
