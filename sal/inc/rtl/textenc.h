/*************************************************************************
 *
 *  $RCSfile: textenc.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:15 $
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

#ifndef _RTL_TEXTENC_H
#define _RTL_TEXTENC_H

#ifndef _SAL_TYPES_H
#include <sal/types.h>
#endif

typedef sal_uInt16 rtl_TextEncoding;
#define RTL_TEXTENCODING_DONTKNOW               ((rtl_TextEncoding)0)
#define RTL_TEXTENCODING_MS_1252                ((rtl_TextEncoding)1)
#define RTL_TEXTENCODING_APPLE_ROMAN            ((rtl_TextEncoding)2)
#define RTL_TEXTENCODING_IBM_437                ((rtl_TextEncoding)3)
#define RTL_TEXTENCODING_IBM_850                ((rtl_TextEncoding)4)
#define RTL_TEXTENCODING_IBM_860                ((rtl_TextEncoding)5)
#define RTL_TEXTENCODING_IBM_861                ((rtl_TextEncoding)6)
#define RTL_TEXTENCODING_IBM_863                ((rtl_TextEncoding)7)
#define RTL_TEXTENCODING_IBM_865                ((rtl_TextEncoding)8)
/* Reserved: RTL_TEXTENCODING_SYSTEM            ((rtl_TextEncoding)9) */
#define RTL_TEXTENCODING_SYMBOL                 ((rtl_TextEncoding)10)
#define RTL_TEXTENCODING_ASCII_US               ((rtl_TextEncoding)11)
#define RTL_TEXTENCODING_ISO_8859_1             ((rtl_TextEncoding)12)
#define RTL_TEXTENCODING_ISO_8859_2             ((rtl_TextEncoding)13)
#define RTL_TEXTENCODING_ISO_8859_3             ((rtl_TextEncoding)14)
#define RTL_TEXTENCODING_ISO_8859_4             ((rtl_TextEncoding)15)
#define RTL_TEXTENCODING_ISO_8859_5             ((rtl_TextEncoding)16)
#define RTL_TEXTENCODING_ISO_8859_6             ((rtl_TextEncoding)17)
#define RTL_TEXTENCODING_ISO_8859_7             ((rtl_TextEncoding)18)
#define RTL_TEXTENCODING_ISO_8859_8             ((rtl_TextEncoding)19)
#define RTL_TEXTENCODING_ISO_8859_9             ((rtl_TextEncoding)20)
#define RTL_TEXTENCODING_ISO_8859_14            ((rtl_TextEncoding)21)
#define RTL_TEXTENCODING_ISO_8859_15            ((rtl_TextEncoding)22)
#define RTL_TEXTENCODING_IBM_737                ((rtl_TextEncoding)23)
#define RTL_TEXTENCODING_IBM_775                ((rtl_TextEncoding)24)
#define RTL_TEXTENCODING_IBM_852                ((rtl_TextEncoding)25)
#define RTL_TEXTENCODING_IBM_855                ((rtl_TextEncoding)26)
#define RTL_TEXTENCODING_IBM_857                ((rtl_TextEncoding)27)
#define RTL_TEXTENCODING_IBM_862                ((rtl_TextEncoding)28)
#define RTL_TEXTENCODING_IBM_864                ((rtl_TextEncoding)29)
#define RTL_TEXTENCODING_IBM_866                ((rtl_TextEncoding)30)
#define RTL_TEXTENCODING_IBM_869                ((rtl_TextEncoding)31)
#define RTL_TEXTENCODING_MS_874                 ((rtl_TextEncoding)32)
#define RTL_TEXTENCODING_MS_1250                ((rtl_TextEncoding)33)
#define RTL_TEXTENCODING_MS_1251                ((rtl_TextEncoding)34)
#define RTL_TEXTENCODING_MS_1253                ((rtl_TextEncoding)35)
#define RTL_TEXTENCODING_MS_1254                ((rtl_TextEncoding)36)
#define RTL_TEXTENCODING_MS_1255                ((rtl_TextEncoding)37)
#define RTL_TEXTENCODING_MS_1256                ((rtl_TextEncoding)38)
#define RTL_TEXTENCODING_MS_1257                ((rtl_TextEncoding)39)
#define RTL_TEXTENCODING_MS_1258                ((rtl_TextEncoding)40)
#define RTL_TEXTENCODING_APPLE_ARABIC           ((rtl_TextEncoding)41)
#define RTL_TEXTENCODING_APPLE_CENTEURO         ((rtl_TextEncoding)42)
#define RTL_TEXTENCODING_APPLE_CROATIAN         ((rtl_TextEncoding)43)
#define RTL_TEXTENCODING_APPLE_CYRILLIC         ((rtl_TextEncoding)44)
#define RTL_TEXTENCODING_APPLE_DEVANAGARI       ((rtl_TextEncoding)45)
#define RTL_TEXTENCODING_APPLE_FARSI            ((rtl_TextEncoding)46)
#define RTL_TEXTENCODING_APPLE_GREEK            ((rtl_TextEncoding)47)
#define RTL_TEXTENCODING_APPLE_GUJARATI         ((rtl_TextEncoding)48)
#define RTL_TEXTENCODING_APPLE_GURMUKHI         ((rtl_TextEncoding)49)
#define RTL_TEXTENCODING_APPLE_HEBREW           ((rtl_TextEncoding)50)
#define RTL_TEXTENCODING_APPLE_ICELAND          ((rtl_TextEncoding)51)
#define RTL_TEXTENCODING_APPLE_ROMANIAN         ((rtl_TextEncoding)52)
#define RTL_TEXTENCODING_APPLE_THAI             ((rtl_TextEncoding)53)
#define RTL_TEXTENCODING_APPLE_TURKISH          ((rtl_TextEncoding)54)
#define RTL_TEXTENCODING_APPLE_UKRAINIAN        ((rtl_TextEncoding)55)
#define RTL_TEXTENCODING_APPLE_CHINSIMP         ((rtl_TextEncoding)56)
#define RTL_TEXTENCODING_APPLE_CHINTRAD         ((rtl_TextEncoding)57)
#define RTL_TEXTENCODING_APPLE_JAPANESE         ((rtl_TextEncoding)58)
#define RTL_TEXTENCODING_APPLE_KOREAN           ((rtl_TextEncoding)59)
#define RTL_TEXTENCODING_MS_932                 ((rtl_TextEncoding)60)
#define RTL_TEXTENCODING_MS_936                 ((rtl_TextEncoding)61)
#define RTL_TEXTENCODING_MS_949                 ((rtl_TextEncoding)62)
#define RTL_TEXTENCODING_MS_950                 ((rtl_TextEncoding)63)
#define RTL_TEXTENCODING_SHIFT_JIS              ((rtl_TextEncoding)64)
#define RTL_TEXTENCODING_GB_2312                ((rtl_TextEncoding)65)
#define RTL_TEXTENCODING_GBT_12345              ((rtl_TextEncoding)66)
#define RTL_TEXTENCODING_GBK                    ((rtl_TextEncoding)67)
#define RTL_TEXTENCODING_BIG5                   ((rtl_TextEncoding)68)
#define RTL_TEXTENCODING_EUC_JP                 ((rtl_TextEncoding)69)
#define RTL_TEXTENCODING_EUC_CN                 ((rtl_TextEncoding)70)
#define RTL_TEXTENCODING_EUC_TW                 ((rtl_TextEncoding)71)
#define RTL_TEXTENCODING_ISO_2022_JP            ((rtl_TextEncoding)72)
#define RTL_TEXTENCODING_ISO_2022_CN            ((rtl_TextEncoding)73)
#define RTL_TEXTENCODING_KOI8_R                 ((rtl_TextEncoding)74)
#define RTL_TEXTENCODING_UTF7                   ((rtl_TextEncoding)75)
#define RTL_TEXTENCODING_UTF8                   ((rtl_TextEncoding)76)
#define RTL_TEXTENCODING_ISO_8859_10            ((rtl_TextEncoding)77)
#define RTL_TEXTENCODING_ISO_8859_13            ((rtl_TextEncoding)78)
#define RTL_TEXTENCODING_EUC_KR                 ((rtl_TextEncoding)79)
#define RTL_TEXTENCODING_ISO_2022_KR            ((rtl_TextEncoding)80)
#define RTL_TEXTENCODING_JIS_X_0201             ((rtl_TextEncoding)81)
#define RTL_TEXTENCODING_JIS_X_0208             ((rtl_TextEncoding)82)
#define RTL_TEXTENCODING_JIS_X_0212             ((rtl_TextEncoding)83)

#define RTL_TEXTENCODING_STD_COUNT              ((rtl_TextEncoding)84)

#define RTL_TEXTENCODING_USER_START             ((rtl_TextEncoding)0x8000)
#define RTL_TEXTENCODING_USER_END               ((rtl_TextEncoding)0xEFFF)

#define RTL_TEXTENCODING_UCS4                   ((rtl_TextEncoding)0xFFFE)
#define RTL_TEXTENCODING_UCS2                   ((rtl_TextEncoding)0xFFFF)
#define RTL_TEXTENCODING_UNICODE                RTL_TEXTENCODING_UCS2

/****** Overview over the TextEncodings *****
# Arabic (Apple Macintosh)                        RTL_TEXTENCODING_APPLE_ARABIC
Arabic (DOS/OS2-864)                            RTL_TEXTENCODING_IBM_864
Arabic (ISO-8859-6)                             RTL_TEXTENCODING_ISO_8859_6
Arabic (Windows-1256)                           RTL_TEXTENCODING_MS_1256

Baltic (DOS/OS2-775)                            RTL_TEXTENCODING_IBM_775
Baltic (ISO-8859-4)                             RTL_TEXTENCODING_ISO_8859_4
Baltic (Windows-1257)                           RTL_TEXTENCODING_MS_1257

Central European (Apple Macintosh)              RTL_TEXTENCODING_APPLE_CENTEURO
Central European (Apple Macintosh/Croatian)     RTL_TEXTENCODING_APPLE_CROATIAN
Central European (Apple Macintosh/Romanian)     RTL_TEXTENCODING_APPLE_ROMANIAN
Central European (DOS/OS2-852)                  RTL_TEXTENCODING_IBM_852
Central European (ISO-8859-2)                   RTL_TEXTENCODING_ISO_8859_2
Central European (ISO-8859-10)                  RTL_TEXTENCODING_ISO_8859_10
Central European (ISO-8859-13)                  RTL_TEXTENCODING_ISO_8859_13
Central European (Windows-1250/WinLatin 2)      RTL_TEXTENCODING_MS_1250

Chinese Simplified (Apple Macintosh)            RTL_TEXTENCODING_APPLE_CHINSIMP
Chinese Simplified (EUC-CN)                     RTL_TEXTENCODING_EUC_CN
Chinese Simplified (GB-2312)                    RTL_TEXTENCODING_GB_2312
Chinese Simplified (GBK/GB-2312-80)             RTL_TEXTENCODING_GBK
# Chinese Simplified (ISO-2022-CN)                RTL_TEXTENCODING_ISO_2022_CN
Chinese Simplified (Windows-936)                RTL_TEXTENCODING_MS_936

Chinese Traditional (Apple Macintosh)           RTL_TEXTENCODING_APPLE_CHINTRAD
Chinese Traditional (BIG5)                      RTL_TEXTENCODING_BIG5
# Chinese Traditional (EUC-TW)                    RTL_TEXTENCODING_EUC_TW
Chinese Traditional (GBT-12345)                 RTL_TEXTENCODING_GBT_12345
Chinese Traditional (Windows-950)               RTL_TEXTENCODING_MS_950

Cyrillic (Apple Macintosh)                      RTL_TEXTENCODING_APPLE_CYRILLIC
Cyrillic (Apple Macintosh/Ukrainian)            RTL_TEXTENCODING_APPLE_UKRAINIAN
Cyrillic (DOS/OS2-855)                          RTL_TEXTENCODING_IBM_855
Cyrillic (DOS/OS2-866/Russian)                  RTL_TEXTENCODING_IBM_866
Cyrillic (ISO-8859-5)                           RTL_TEXTENCODING_ISO_8859_5
Cyrillic (KOI8-R)                               RTL_TEXTENCODING_KOI8_R
Cyrillic (Windows-1251)                         RTL_TEXTENCODING_MS_1251

Greek (Apple Macintosh)                         RTL_TEXTENCODING_APPLE_GREEK
Greek (DOS/OS2-737)                             RTL_TEXTENCODING_IBM_737
Greek (DOS/OS2-869/Modern)                      RTL_TEXTENCODING_IBM_869
Greek (ISO-8859-7)                              RTL_TEXTENCODING_ISO_8859_7
Greek (Windows-1253)                            RTL_TEXTENCODING_MS_1253

# Hebrew (Apple Macintosh)                        RTL_TEXTENCODING_APPLE_HEBREW
Hebrew (DOS/OS2-862)                            RTL_TEXTENCODING_IBM_862
Hebrew (ISO-8859-8)                             RTL_TEXTENCODING_ISO_8859_8
Hebrew (Windows-1255)                           RTL_TEXTENCODING_MS_1255

Korean (Apple Macintosh)                        RTL_TEXTENCODING_APPLE_KOREAN
Korean (EUC-KR)                                 RTL_TEXTENCODING_EUC_KR
# Korean (ISO-2022-KR)                            RTL_TEXTENCODING_ISO_2022_KR
Korean (Windows-Wansung-949)                    RTL_TEXTENCODING_MS_949

Latin 3 (ISO-8859-3)                            RTL_TEXTENCODING_ISO_8859_3

Japanese (Apple Macintosh)                      RTL_TEXTENCODING_APPLE_JAPANESE
# Japanese (EUC-JP)                               RTL_TEXTENCODING_EUC_JP
# Japanese (ISO-2022-JP)                          RTL_TEXTENCODING_ISO_2022_JP
Japanese (Shift-JIS)                            RTL_TEXTENCODING_SHIFT_JIS
Japanese (Windows-932)                          RTL_TEXTENCODING_MS_932

Symbol                                          RTL_TEXTENCODING_SYMBOL

# Thai (Apple Macintosh)                          RTL_TEXTENCODING_APPLE_THAI
Thai (Dos/Windows-874)                          RTL_TEXTENCODING_MS_874

Turkish (Apple Macintosh)                       RTL_TEXTENCODING_APPLE_TURKISH
Turkish (DOS/OS2-857)                           RTL_TEXTENCODING_IBM_857
Turkish (ISO-8859-9)                            RTL_TEXTENCODING_ISO_8859_9
Turkish (Windows-1254)                          RTL_TEXTENCODING_MS_1254

Unicode (UTF-7)                                 RTL_TEXTENCODING_UTF7
Unicode (UTF-8)                                 RTL_TEXTENCODING_UTF8

Vietnamese (Windows-1258)                       RTL_TEXTENCODING_MS_1258

Western (Apple Macintosh)                       RTL_TEXTENCODING_APPLE_ROMAN
Western (Apple Macintosh/Icelandic)             RTL_TEXTENCODING_APPLE_ICELAND
Western (ASCII/US)                              RTL_TEXTENCODING_ASCII_US
Western (DOS/OS2-437/US)                        RTL_TEXTENCODING_IBM_437
Western (DOS/OS2-850/International)             RTL_TEXTENCODING_IBM_850
Western (DOS/OS2-860/Portugese)                 RTL_TEXTENCODING_IBM_860
Western (DOS/OS2-861/Icelandic)                 RTL_TEXTENCODING_IBM_861
Western (DOS/OS2-863/Canadian-French)           RTL_TEXTENCODING_IBM_863
Western (DOS/OS2-865/Nordic)                    RTL_TEXTENCODING_IBM_865
Western (ISO-8859-1)                            RTL_TEXTENCODING_ISO_8859_1
Western (ISO-8859-14)                           RTL_TEXTENCODING_ISO_8859_14
Western (ISO-8859-15/EURO)                      RTL_TEXTENCODING_ISO_8859_15
Western (Window-1252/WinLatin 1)                RTL_TEXTENCODING_MS_1252

Not known and currently not supported
# RTL_TEXTENCODING_APPLE_DEVANAGARI
# RTL_TEXTENCODING_APPLE_FARSI
# RTL_TEXTENCODING_APPLE_GUJARATI
# RTL_TEXTENCODING_APPLE_GURMUKHI

Only for internal implementations and not useful for user interface.
These encodings are not used for text encodings, only used for
font-/textoutput encodings.
Japanese (JIS 0201)                             RTL_TEXTENCODING_JISX_0201
Japanese (JIS 0208)                             RTL_TEXTENCODING_JISX_0208
Japanese (JIS 0212)                             RTL_TEXTENCODING_JISX_0212

# Currently not implemented
*/

#endif /* _RTL_TEXTENC_H */

