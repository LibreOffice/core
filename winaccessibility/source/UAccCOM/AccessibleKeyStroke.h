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

#pragma once

struct ACCESSIBLE_KEYSTROKE
{
    short modifiers;
    short keyCode;
    char keyChar;
    short keyFunc;
};

const short MODIFIER_SHIFT = 1;
const short MODIFIER_CTRL = 2;
const short MODIFIER_ALT = 4;

const short KEYCODE_NUM0 = 256;
const short KEYCODE_NUM1 = 257;
const short KEYCODE_NUM2 = 258;
const short KEYCODE_NUM3 = 259;
const short KEYCODE_NUM4 = 260;
const short KEYCODE_NUM5 = 261;
const short KEYCODE_NUM6 = 262;
const short KEYCODE_NUM7 = 263;
const short KEYCODE_NUM8 = 264;
const short KEYCODE_NUM9 = 265;
const short KEYCODE_A = 512;
const short KEYCODE_B = 513;
const short KEYCODE_C = 514;
const short KEYCODE_D = 515;
const short KEYCODE_E = 516;
const short KEYCODE_F = 517;
const short KEYCODE_G = 518;
const short KEYCODE_H = 519;
const short KEYCODE_I = 520;
const short KEYCODE_J = 521;
const short KEYCODE_K = 522;
const short KEYCODE_L = 523;
const short KEYCODE_M = 524;
const short KEYCODE_N = 525;
const short KEYCODE_O = 526;
const short KEYCODE_P = 527;
const short KEYCODE_Q = 528;
const short KEYCODE_R = 529;
const short KEYCODE_S = 530;
const short KEYCODE_T = 531;
const short KEYCODE_U = 532;
const short KEYCODE_V = 533;
const short KEYCODE_W = 534;
const short KEYCODE_X = 535;
const short KEYCODE_Y = 536;
const short KEYCODE_Z = 537;
const short KEYCODE_F1 = 768;
const short KEYCODE_F2 = 769;
const short KEYCODE_F3 = 770;
const short KEYCODE_F4 = 771;
const short KEYCODE_F5 = 772;
const short KEYCODE_F6 = 773;
const short KEYCODE_F7 = 774;
const short KEYCODE_F8 = 775;
const short KEYCODE_F9 = 776;
const short KEYCODE_F10 = 777;
const short KEYCODE_F11 = 778;
const short KEYCODE_F12 = 779;
const short KEYCODE_F13 = 780;
const short KEYCODE_F14 = 781;
const short KEYCODE_F15 = 782;
const short KEYCODE_F16 = 783;
const short KEYCODE_F17 = 784;
const short KEYCODE_F18 = 785;
const short KEYCODE_F19 = 786;
const short KEYCODE_F20 = 787;
const short KEYCODE_F21 = 788;
const short KEYCODE_F22 = 789;
const short KEYCODE_F23 = 790;
const short KEYCODE_F24 = 791;
const short KEYCODE_F25 = 792;
const short KEYCODE_F26 = 793;
const short KEYCODE_DOWN = 1024;
const short KEYCODE_UP = 1025;
const short KEYCODE_LEFT = 1026;
const short KEYCODE_RIGHT = 1027;
const short KEYCODE_HOME = 1028;
const short KEYCODE_END = 1029;
const short KEYCODE_PAGEUP = 1030;
const short KEYCODE_PAGEDOWN = 1031;
const short KEYCODE_RETURN = 1280;
const short KEYCODE_ESCAPE = 1281;
const short KEYCODE_TAB = 1282;
const short KEYCODE_BACKSPACE = 1283;
const short KEYCODE_SPACE = 1284;
const short KEYCODE_INSERT = 1285;
const short KEYCODE_DELETE = 1286;
const short KEYCODE_ADD = 1287;
const short KEYCODE_SUBTRACT = 1288;
const short KEYCODE_MULTIPLY = 1289;
const short KEYCODE_DIVIDE = 1290;
const short KEYCODE_POINT = 1291;
const short KEYCODE_COMMA = 1292;
const short KEYCODE_LESS = 1293;
const short KEYCODE_GREATER = 1294;
const short KEYCODE_EQUAL = 1295;
const short KEYCODE_OPEN = 1296;
const short KEYCODE_CUT = 1297;
const short KEYCODE_COPY = 1298;
const short KEYCODE_PASTE = 1299;
const short KEYCODE_UNDO = 1300;
const short KEYCODE_REPEAT = 1301;
const short KEYCODE_FIND = 1302;
const short KEYCODE_PROPERTIES = 1303;
const short KEYCODE_FRONT = 1304;
const short KEYCODE_CONTEXTMENU = 1305;
const short KEYCODE_HELP = 1306;

const short SHORTCUT_DONTKNOW = 0;
const short NEW = 1;
const short OPEN = 2;
const short SAVE = 3;
const short SAVEAS = 4;
const short PRINT = 5;
const short CLOSE = 6;
const short QUIT = 7;
const short CUT = 8;
const short COPY = 9;
const short PASTE = 10;
const short UNDO = 11;
const short REDO = 12;
const short UNODELETE = 13;
const short REPEAT = 14;
const short FIND = 15;
const short FINDBACKWARD = 16;
const short PROPERTIES = 17;
const short FRONT = 18;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
