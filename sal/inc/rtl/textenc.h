/*************************************************************************
 *
 *  $RCSfile: textenc.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: th $ $Date: 2001-03-16 15:18:35 $
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

#ifdef _SOLAR_RSC_INVOKED
/* Enable resources to use these values, rsc can't handle casts */
#define RTL_TEXTENC_CAST( val ) (val)

#else /* !_SOLAR_RSC_INVOKED */

#ifndef _SAL_TYPES_H
#include <sal/types.h>
#endif

typedef sal_uInt16 rtl_TextEncoding;
#define RTL_TEXTENC_CAST( val ) ((rtl_TextEncoding) val)

#endif /* _SOLAR_RSC_INVOKED */

#define RTL_TEXTENCODING_DONTKNOW               (RTL_TEXTENC_CAST( 0 ))
#define RTL_TEXTENCODING_MS_1252                (RTL_TEXTENC_CAST( 1 ))
#define RTL_TEXTENCODING_APPLE_ROMAN            (RTL_TEXTENC_CAST( 2 ))
#define RTL_TEXTENCODING_IBM_437                (RTL_TEXTENC_CAST( 3 ))
#define RTL_TEXTENCODING_IBM_850                (RTL_TEXTENC_CAST( 4 ))
#define RTL_TEXTENCODING_IBM_860                (RTL_TEXTENC_CAST( 5 ))
#define RTL_TEXTENCODING_IBM_861                (RTL_TEXTENC_CAST( 6 ))
#define RTL_TEXTENCODING_IBM_863                (RTL_TEXTENC_CAST( 7 ))
#define RTL_TEXTENCODING_IBM_865                (RTL_TEXTENC_CAST( 8 ))
/* Reserved: RTL_TEXTENCODING_SYSTEM            (RTL_TEXTENC_CAST( 9 )) */
#define RTL_TEXTENCODING_SYMBOL                 (RTL_TEXTENC_CAST( 10 ))
#define RTL_TEXTENCODING_ASCII_US               (RTL_TEXTENC_CAST( 11 ))
#define RTL_TEXTENCODING_ISO_8859_1             (RTL_TEXTENC_CAST( 12 ))
#define RTL_TEXTENCODING_ISO_8859_2             (RTL_TEXTENC_CAST( 13 ))
#define RTL_TEXTENCODING_ISO_8859_3             (RTL_TEXTENC_CAST( 14 ))
#define RTL_TEXTENCODING_ISO_8859_4             (RTL_TEXTENC_CAST( 15 ))
#define RTL_TEXTENCODING_ISO_8859_5             (RTL_TEXTENC_CAST( 16 ))
#define RTL_TEXTENCODING_ISO_8859_6             (RTL_TEXTENC_CAST( 17 ))
#define RTL_TEXTENCODING_ISO_8859_7             (RTL_TEXTENC_CAST( 18 ))
#define RTL_TEXTENCODING_ISO_8859_8             (RTL_TEXTENC_CAST( 19 ))
#define RTL_TEXTENCODING_ISO_8859_9             (RTL_TEXTENC_CAST( 20 ))
#define RTL_TEXTENCODING_ISO_8859_14            (RTL_TEXTENC_CAST( 21 ))
#define RTL_TEXTENCODING_ISO_8859_15            (RTL_TEXTENC_CAST( 22 ))
#define RTL_TEXTENCODING_IBM_737                (RTL_TEXTENC_CAST( 23 ))
#define RTL_TEXTENCODING_IBM_775                (RTL_TEXTENC_CAST( 24 ))
#define RTL_TEXTENCODING_IBM_852                (RTL_TEXTENC_CAST( 25 ))
#define RTL_TEXTENCODING_IBM_855                (RTL_TEXTENC_CAST( 26 ))
#define RTL_TEXTENCODING_IBM_857                (RTL_TEXTENC_CAST( 27 ))
#define RTL_TEXTENCODING_IBM_862                (RTL_TEXTENC_CAST( 28 ))
#define RTL_TEXTENCODING_IBM_864                (RTL_TEXTENC_CAST( 29 ))
#define RTL_TEXTENCODING_IBM_866                (RTL_TEXTENC_CAST( 30 ))
#define RTL_TEXTENCODING_IBM_869                (RTL_TEXTENC_CAST( 31 ))
#define RTL_TEXTENCODING_MS_874                 (RTL_TEXTENC_CAST( 32 ))
#define RTL_TEXTENCODING_MS_1250                (RTL_TEXTENC_CAST( 33 ))
#define RTL_TEXTENCODING_MS_1251                (RTL_TEXTENC_CAST( 34 ))
#define RTL_TEXTENCODING_MS_1253                (RTL_TEXTENC_CAST( 35 ))
#define RTL_TEXTENCODING_MS_1254                (RTL_TEXTENC_CAST( 36 ))
#define RTL_TEXTENCODING_MS_1255                (RTL_TEXTENC_CAST( 37 ))
#define RTL_TEXTENCODING_MS_1256                (RTL_TEXTENC_CAST( 38 ))
#define RTL_TEXTENCODING_MS_1257                (RTL_TEXTENC_CAST( 39 ))
#define RTL_TEXTENCODING_MS_1258                (RTL_TEXTENC_CAST( 40 ))
#define RTL_TEXTENCODING_APPLE_ARABIC           (RTL_TEXTENC_CAST( 41 ))
#define RTL_TEXTENCODING_APPLE_CENTEURO         (RTL_TEXTENC_CAST( 42 ))
#define RTL_TEXTENCODING_APPLE_CROATIAN         (RTL_TEXTENC_CAST( 43 ))
#define RTL_TEXTENCODING_APPLE_CYRILLIC         (RTL_TEXTENC_CAST( 44 ))
#define RTL_TEXTENCODING_APPLE_DEVANAGARI       (RTL_TEXTENC_CAST( 45 ))
#define RTL_TEXTENCODING_APPLE_FARSI            (RTL_TEXTENC_CAST( 46 ))
#define RTL_TEXTENCODING_APPLE_GREEK            (RTL_TEXTENC_CAST( 47 ))
#define RTL_TEXTENCODING_APPLE_GUJARATI         (RTL_TEXTENC_CAST( 48 ))
#define RTL_TEXTENCODING_APPLE_GURMUKHI         (RTL_TEXTENC_CAST( 49 ))
#define RTL_TEXTENCODING_APPLE_HEBREW           (RTL_TEXTENC_CAST( 50 ))
#define RTL_TEXTENCODING_APPLE_ICELAND          (RTL_TEXTENC_CAST( 51 ))
#define RTL_TEXTENCODING_APPLE_ROMANIAN         (RTL_TEXTENC_CAST( 52 ))
#define RTL_TEXTENCODING_APPLE_THAI             (RTL_TEXTENC_CAST( 53 ))
#define RTL_TEXTENCODING_APPLE_TURKISH          (RTL_TEXTENC_CAST( 54 ))
#define RTL_TEXTENCODING_APPLE_UKRAINIAN        (RTL_TEXTENC_CAST( 55 ))
#define RTL_TEXTENCODING_APPLE_CHINSIMP         (RTL_TEXTENC_CAST( 56 ))
#define RTL_TEXTENCODING_APPLE_CHINTRAD         (RTL_TEXTENC_CAST( 57 ))
#define RTL_TEXTENCODING_APPLE_JAPANESE         (RTL_TEXTENC_CAST( 58 ))
#define RTL_TEXTENCODING_APPLE_KOREAN           (RTL_TEXTENC_CAST( 59 ))
#define RTL_TEXTENCODING_MS_932                 (RTL_TEXTENC_CAST( 60 ))
#define RTL_TEXTENCODING_MS_936                 (RTL_TEXTENC_CAST( 61 ))
#define RTL_TEXTENCODING_MS_949                 (RTL_TEXTENC_CAST( 62 ))
#define RTL_TEXTENCODING_MS_950                 (RTL_TEXTENC_CAST( 63 ))
#define RTL_TEXTENCODING_SHIFT_JIS              (RTL_TEXTENC_CAST( 64 ))
#define RTL_TEXTENCODING_GB_2312                (RTL_TEXTENC_CAST( 65 ))
#define RTL_TEXTENCODING_GBT_12345              (RTL_TEXTENC_CAST( 66 ))
#define RTL_TEXTENCODING_GBK                    (RTL_TEXTENC_CAST( 67 ))
#define RTL_TEXTENCODING_BIG5                   (RTL_TEXTENC_CAST( 68 ))
#define RTL_TEXTENCODING_EUC_JP                 (RTL_TEXTENC_CAST( 69 ))
#define RTL_TEXTENCODING_EUC_CN                 (RTL_TEXTENC_CAST( 70 ))
#define RTL_TEXTENCODING_EUC_TW                 (RTL_TEXTENC_CAST( 71 ))
#define RTL_TEXTENCODING_ISO_2022_JP            (RTL_TEXTENC_CAST( 72 ))
#define RTL_TEXTENCODING_ISO_2022_CN            (RTL_TEXTENC_CAST( 73 ))
#define RTL_TEXTENCODING_KOI8_R                 (RTL_TEXTENC_CAST( 74 ))
#define RTL_TEXTENCODING_UTF7                   (RTL_TEXTENC_CAST( 75 ))
#define RTL_TEXTENCODING_UTF8                   (RTL_TEXTENC_CAST( 76 ))
#define RTL_TEXTENCODING_ISO_8859_10            (RTL_TEXTENC_CAST( 77 ))
#define RTL_TEXTENCODING_ISO_8859_13            (RTL_TEXTENC_CAST( 78 ))
#define RTL_TEXTENCODING_EUC_KR                 (RTL_TEXTENC_CAST( 79 ))
#define RTL_TEXTENCODING_ISO_2022_KR            (RTL_TEXTENC_CAST( 80 ))
#define RTL_TEXTENCODING_JIS_X_0201             (RTL_TEXTENC_CAST( 81 ))
#define RTL_TEXTENCODING_JIS_X_0208             (RTL_TEXTENC_CAST( 82 ))
#define RTL_TEXTENCODING_JIS_X_0212             (RTL_TEXTENC_CAST( 83 ))
#define RTL_TEXTENCODING_MS_1361                (RTL_TEXTENC_CAST( 84 ))

#define RTL_TEXTENCODING_STD_COUNT              (RTL_TEXTENC_CAST( 85 ))

#define RTL_TEXTENCODING_USER_START             (RTL_TEXTENC_CAST( 0x8000 ))
#define RTL_TEXTENCODING_USER_END               (RTL_TEXTENC_CAST( 0xEFFF ))

#define RTL_TEXTENCODING_UCS4                   (RTL_TEXTENC_CAST( 0xFFFE ))
#define RTL_TEXTENCODING_UCS2                   (RTL_TEXTENC_CAST( 0xFFFF ))
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
Korean (Windows-Johab-1361)                     RTL_TEXTENCODING_MS_1361

Latin 3 (ISO-8859-3)                            RTL_TEXTENCODING_ISO_8859_3

Japanese (Apple Macintosh)                      RTL_TEXTENCODING_APPLE_JAPANESE
Japanese (EUC-JP)                               RTL_TEXTENCODING_EUC_JP
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

