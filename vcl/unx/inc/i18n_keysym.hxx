/*************************************************************************
 *
 *  $RCSfile: i18n_keysym.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cp $ $Date: 2001-05-17 11:12:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


