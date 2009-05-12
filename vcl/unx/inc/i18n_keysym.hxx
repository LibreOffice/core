/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: i18n_keysym.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SAL_I18N_KEYSYM_HXX
#define _SAL_I18N_KEYSYM_HXX

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

#endif /* _SAL_I18N_KEYSYM_HXX */


