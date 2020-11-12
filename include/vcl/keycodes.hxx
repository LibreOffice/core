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

#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyGroup.hpp>
#include <o3tl/typed_flags_set.hxx>

constexpr sal_uInt16 KEY_CODE_MASK = 0x0FFF;

// Modifier keys
constexpr sal_uInt16 KEY_SHIFT = 0x1000;
constexpr sal_uInt16 KEY_MOD1 = 0x2000;
constexpr sal_uInt16 KEY_MOD2 = 0x4000;
constexpr sal_uInt16 KEY_MOD3 = 0x8000;
constexpr sal_uInt16 KEY_MODIFIERS_MASK = 0xF000;

// Key groups
constexpr sal_uInt16 KEYGROUP_NUM = css::awt::KeyGroup::NUM;
constexpr sal_uInt16 KEYGROUP_ALPHA = css::awt::KeyGroup::ALPHA;
constexpr sal_uInt16 KEYGROUP_FKEYS = css::awt::KeyGroup::FKEYS;
constexpr sal_uInt16 KEYGROUP_CURSOR = css::awt::KeyGroup::CURSOR;
constexpr sal_uInt16 KEYGROUP_MISC = css::awt::KeyGroup::MISC;
constexpr sal_uInt16 KEYGROUP_TYPE = css::awt::KeyGroup::TYPE;

// Key codes
constexpr sal_uInt16 KEY_0 = css::awt::Key::NUM0;
constexpr sal_uInt16 KEY_1 = css::awt::Key::NUM1;
constexpr sal_uInt16 KEY_2 = css::awt::Key::NUM2;
constexpr sal_uInt16 KEY_3 = css::awt::Key::NUM3;
constexpr sal_uInt16 KEY_4 = css::awt::Key::NUM4;
constexpr sal_uInt16 KEY_5 = css::awt::Key::NUM5;
constexpr sal_uInt16 KEY_6 = css::awt::Key::NUM6;
constexpr sal_uInt16 KEY_7 = css::awt::Key::NUM7;
constexpr sal_uInt16 KEY_8 = css::awt::Key::NUM8;
constexpr sal_uInt16 KEY_9 = css::awt::Key::NUM9;

constexpr sal_uInt16 KEY_A = css::awt::Key::A;
constexpr sal_uInt16 KEY_B = css::awt::Key::B;
constexpr sal_uInt16 KEY_C = css::awt::Key::C;
constexpr sal_uInt16 KEY_D = css::awt::Key::D;
constexpr sal_uInt16 KEY_E = css::awt::Key::E;
constexpr sal_uInt16 KEY_F = css::awt::Key::F;
constexpr sal_uInt16 KEY_G = css::awt::Key::G;
constexpr sal_uInt16 KEY_H = css::awt::Key::H;
constexpr sal_uInt16 KEY_I = css::awt::Key::I;
constexpr sal_uInt16 KEY_J = css::awt::Key::J;
constexpr sal_uInt16 KEY_K = css::awt::Key::K;
constexpr sal_uInt16 KEY_L = css::awt::Key::L;
constexpr sal_uInt16 KEY_M = css::awt::Key::M;
constexpr sal_uInt16 KEY_N = css::awt::Key::N;
constexpr sal_uInt16 KEY_O = css::awt::Key::O;
constexpr sal_uInt16 KEY_P = css::awt::Key::P;
constexpr sal_uInt16 KEY_Q = css::awt::Key::Q;
constexpr sal_uInt16 KEY_R = css::awt::Key::R;
constexpr sal_uInt16 KEY_S = css::awt::Key::S;
constexpr sal_uInt16 KEY_T = css::awt::Key::T;
constexpr sal_uInt16 KEY_U = css::awt::Key::U;
constexpr sal_uInt16 KEY_V = css::awt::Key::V;
constexpr sal_uInt16 KEY_W = css::awt::Key::W;
constexpr sal_uInt16 KEY_X = css::awt::Key::X;
constexpr sal_uInt16 KEY_Y = css::awt::Key::Y;
constexpr sal_uInt16 KEY_Z = css::awt::Key::Z;

constexpr sal_uInt16 KEY_F1 = css::awt::Key::F1;
constexpr sal_uInt16 KEY_F2 = css::awt::Key::F2;
constexpr sal_uInt16 KEY_F3 = css::awt::Key::F3;
constexpr sal_uInt16 KEY_F4 = css::awt::Key::F4;
constexpr sal_uInt16 KEY_F5 = css::awt::Key::F5;
constexpr sal_uInt16 KEY_F6 = css::awt::Key::F6;
constexpr sal_uInt16 KEY_F7 = css::awt::Key::F7;
constexpr sal_uInt16 KEY_F8 = css::awt::Key::F8;
constexpr sal_uInt16 KEY_F9 = css::awt::Key::F9;
constexpr sal_uInt16 KEY_F10 = css::awt::Key::F10;
constexpr sal_uInt16 KEY_F11 = css::awt::Key::F11;
constexpr sal_uInt16 KEY_F12 = css::awt::Key::F12;
constexpr sal_uInt16 KEY_F13 = css::awt::Key::F13;
constexpr sal_uInt16 KEY_F14 = css::awt::Key::F14;
constexpr sal_uInt16 KEY_F15 = css::awt::Key::F15;
constexpr sal_uInt16 KEY_F16 = css::awt::Key::F16;
constexpr sal_uInt16 KEY_F17 = css::awt::Key::F17;
constexpr sal_uInt16 KEY_F18 = css::awt::Key::F18;
constexpr sal_uInt16 KEY_F19 = css::awt::Key::F19;
constexpr sal_uInt16 KEY_F20 = css::awt::Key::F20;
constexpr sal_uInt16 KEY_F21 = css::awt::Key::F21;
constexpr sal_uInt16 KEY_F22 = css::awt::Key::F22;
constexpr sal_uInt16 KEY_F23 = css::awt::Key::F23;
constexpr sal_uInt16 KEY_F24 = css::awt::Key::F24;
constexpr sal_uInt16 KEY_F25 = css::awt::Key::F25;
constexpr sal_uInt16 KEY_F26 = css::awt::Key::F26;

constexpr sal_uInt16 KEY_DOWN = css::awt::Key::DOWN;
constexpr sal_uInt16 KEY_UP = css::awt::Key::UP;
constexpr sal_uInt16 KEY_LEFT = css::awt::Key::LEFT;
constexpr sal_uInt16 KEY_RIGHT = css::awt::Key::RIGHT;
constexpr sal_uInt16 KEY_HOME = css::awt::Key::HOME;
constexpr sal_uInt16 KEY_END = css::awt::Key::END;
constexpr sal_uInt16 KEY_PAGEUP = css::awt::Key::PAGEUP;
constexpr sal_uInt16 KEY_PAGEDOWN = css::awt::Key::PAGEDOWN;

constexpr sal_uInt16 KEY_RETURN = css::awt::Key::RETURN;
constexpr sal_uInt16 KEY_ESCAPE = css::awt::Key::ESCAPE;
constexpr sal_uInt16 KEY_TAB = css::awt::Key::TAB;
constexpr sal_uInt16 KEY_BACKSPACE = css::awt::Key::BACKSPACE;
constexpr sal_uInt16 KEY_SPACE = css::awt::Key::SPACE;
constexpr sal_uInt16 KEY_INSERT = css::awt::Key::INSERT;
constexpr sal_uInt16 KEY_DELETE = css::awt::Key::DELETE;

constexpr sal_uInt16 KEY_ADD = css::awt::Key::ADD;
constexpr sal_uInt16 KEY_SUBTRACT = css::awt::Key::SUBTRACT;
constexpr sal_uInt16 KEY_MULTIPLY = css::awt::Key::MULTIPLY;
constexpr sal_uInt16 KEY_DIVIDE = css::awt::Key::DIVIDE;
constexpr sal_uInt16 KEY_POINT = css::awt::Key::POINT;
constexpr sal_uInt16 KEY_COMMA = css::awt::Key::COMMA;
constexpr sal_uInt16 KEY_LESS = css::awt::Key::LESS;
constexpr sal_uInt16 KEY_GREATER = css::awt::Key::GREATER;
constexpr sal_uInt16 KEY_EQUAL = css::awt::Key::EQUAL;

constexpr sal_uInt16 KEY_OPEN = css::awt::Key::OPEN;
constexpr sal_uInt16 KEY_CUT = css::awt::Key::CUT;
constexpr sal_uInt16 KEY_COPY = css::awt::Key::COPY;
constexpr sal_uInt16 KEY_PASTE = css::awt::Key::PASTE;
constexpr sal_uInt16 KEY_UNDO = css::awt::Key::UNDO;
constexpr sal_uInt16 KEY_REPEAT = css::awt::Key::REPEAT;
constexpr sal_uInt16 KEY_FIND = css::awt::Key::FIND;
constexpr sal_uInt16 KEY_PROPERTIES = css::awt::Key::PROPERTIES;
constexpr sal_uInt16 KEY_FRONT = css::awt::Key::FRONT;
constexpr sal_uInt16 KEY_CONTEXTMENU = css::awt::Key::CONTEXTMENU;
constexpr sal_uInt16 KEY_MENU = css::awt::Key::MENU;
constexpr sal_uInt16 KEY_HELP = css::awt::Key::HELP;
constexpr sal_uInt16 KEY_HANGUL_HANJA = css::awt::Key::HANGUL_HANJA;
constexpr sal_uInt16 KEY_DECIMAL = css::awt::Key::DECIMAL;
constexpr sal_uInt16 KEY_TILDE = css::awt::Key::TILDE;
constexpr sal_uInt16 KEY_QUOTELEFT = css::awt::Key::QUOTELEFT;
constexpr sal_uInt16 KEY_BRACKETLEFT = css::awt::Key::BRACKETLEFT;
constexpr sal_uInt16 KEY_BRACKETRIGHT = css::awt::Key::BRACKETRIGHT;
constexpr sal_uInt16 KEY_SEMICOLON = css::awt::Key::SEMICOLON;
constexpr sal_uInt16 KEY_QUOTERIGHT = css::awt::Key::QUOTERIGHT;

constexpr sal_uInt16 KEY_CAPSLOCK = css::awt::Key::CAPSLOCK;
constexpr sal_uInt16 KEY_NUMLOCK = css::awt::Key::NUMLOCK;
constexpr sal_uInt16 KEY_SCROLLLOCK = css::awt::Key::SCROLLLOCK;

// extended Modifier-Keys (only used for modkey events)
enum class ModKeyFlags
{
    NONE = 0x0000,
    LeftShift = 0x0001,
    RightShift = 0x0002,
    LeftMod1 = 0x0004,
    RightMod1 = 0x0008,
    LeftMod2 = 0x0010,
    RightMod2 = 0x0020,
    LeftMod3 = 0x0040,
    RightMod3 = 0x0080,
    Mod1Msk = LeftMod1 | RightMod1, // should be Mod1Mask, but that conflicts with a X.h macro grrrr
    Mod2Msk = LeftMod2 | RightMod2,
};
namespace o3tl
{
template <> struct typed_flags<ModKeyFlags> : is_typed_flags<ModKeyFlags, 0x00ff>
{
};
}

enum class KeyIndicatorState
{
    NONE = 0x0000,
    CAPSLOCK = 0x0001,
    NUMLOCK = 0x0002,
    SCROLLLOCK = 0x0004
};
namespace o3tl
{
template <> struct typed_flags<KeyIndicatorState> : is_typed_flags<KeyIndicatorState, 0x0007>
{
};
}

#endif // INCLUDED_VCL_KEYCODES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
