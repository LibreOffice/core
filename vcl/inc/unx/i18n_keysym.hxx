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

#ifndef INCLUDED_VCL_INC_UNX_I18N_KEYSYM_HXX
#define INCLUDED_VCL_INC_UNX_I18N_KEYSYM_HXX

/*
 convert a keysym as defined in /usr/{X11R6|openwin}/include/X11/keysymdef.h
 to unicode

 supported charsets: (byte1 and byte2 are always 0x0)

     Latin-1    Byte 3 = 0x00
     Latin-2        Byte 3 = 0x01
     Latin-3        Byte 3 = 0x02
     Latin-4        Byte 3 = 0x03
     Kana       Byte 3 = 0x04
     Arabic     Byte 3 = 0x05
    Cyrillic    Byte 3 = 0x06
     Greek      Byte 3 = 0x07
     Technical  Byte 3 = 0x08
     Special        Byte 3 = 0x09
     Publishing Byte 3 = 0x0a = 10
     APL            Byte 3 = 0x0b = 11
     Hebrew     Byte 3 = 0x0c = 12
     Thai       Byte 3 = 0x0d = 13
     Korean     Byte 3 = 0x0e = 14
     Latin-9    Byte 3 = 0x13 = 19
     Currency   Byte 3 = 0x20 = 32
     Keyboard   Byte 3 = 0xff = 255

 missing charsets:

     Latin-8    Byte 3 = 0x12 = 18
     Armenian   Byte 3 = 0x14 = 20
     Georgian   Byte 3 = 0x15 = 21
     Azeri      Byte 3 = 0x16 = 22
     Vietnamese     Byte 3 = 0x1e = 30

 of course not all keysyms can be mapped to a unicode code point
*/

sal_Unicode KeysymToUnicode (KeySym nKeySym);

#endif // INCLUDED_VCL_INC_UNX_I18N_KEYSYM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
