/*************************************************************************
 *
 *  $RCSfile: rtl_String_Const.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 16:37:17 $
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

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _RTL_STRING_CONST_H_
#define _RTL_STRING_CONST_H_

#ifndef _RTL_STRING_UTILS_HXX_
    #include <rtl_String_Utils.hxx>
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#include <limits.h>

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
    #include <sal/types.h>
#endif

#ifndef _RTL_TEXTENC_H
    #include <rtl/textenc.h>
#endif

#ifndef _RTL_USTRING_H_
    #include <rtl/ustring.h>
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const rtl_TextEncoding kEncodingRTLTextUSASCII = RTL_TEXTENCODING_ASCII_US;

//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const sal_uInt32 kConvertFlagsOUStringToOString = OUSTRING_TO_OSTRING_CVTFLAGS;
static const sal_uInt32 kConvertFlagsOStringToOUString = OSTRING_TO_OUSTRING_CVTFLAGS;

//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const sal_Char *kTestStr1  = "Sun Microsystems";
static const sal_Char *kTestStr2  = "Sun Microsystems Java Technology";
static const sal_Char *kTestStr3  = "Sun microsystems";
static const sal_Char *kTestStr4  = "SUN MICROSYSTEMS";
static const sal_Char *kTestStr5  = "sun microsystems";
static const sal_Char *kTestStr6  = "Java Technology";
static const sal_Char *kTestStr7  = "Sun ";
static const sal_Char *kTestStr8  = "Microsystems";
static const sal_Char *kTestStr9  = "sun microsystems java technology";
static const sal_Char *kTestStr10 = " Sun Microsystems";
static const sal_Char *kTestStr11 = "Sun Microsystems ";
static const sal_Char *kTestStr12 = " Sun Microsystems ";
static const sal_Char *kTestStr13 = "Sun Microsystems   ";
static const sal_Char *kTestStr14 = "   Sun Microsystems";
static const sal_Char *kTestStr15 = "   Sun Microsystems ";
static const sal_Char *kTestStr16 = " Sun Microsystems   ";
static const sal_Char *kTestStr17 = "   Sun Microsystems   ";
static const sal_Char *kTestStr18 = "sUN MICROsYsTEMs";
static const sal_Char *kTestStr19 = "---Sun-Microsystems---";
static const sal_Char *kTestStr20 = "sun";
static const sal_Char *kTestStr21 = "SUN";
static const sal_Char *kTestStr22 = "SUN MICROSYSTEMS JAVA TECHNOLOGY";
static const sal_Char *kTestStr23  = " Java Technology";
static const sal_Char *kTestStr24 = "Sun Microsystems Java Technolog";
static const sal_Char *kTestStr25 = "";
static const sal_Char *kTestStr26 = " Mic";
static const sal_Char *kTestStr27 = "s";
static const sal_Char *kTestStr28 = "\50\3\5\7\11\13\15\17sun";
static const sal_Char *kTestStr29 = "\50\3\5\7\11\13\15\17sun\21\23\25\27\31\33\50";
static const sal_Char *kTestStr30 = "sun\21\23\25\27\31\33\50";
static const sal_Char *kTestStr31 = "sun Microsystems";
static const sal_Char *kTestStr32 = "Sun Microsystem ";
static const sal_Char *kTestStr33 = " ";
static const sal_Char *kTestStr34 = "\50\5\5\7\11\13\15\17sun";
static const sal_Char *kTestStr35 = "\50\373\5\7\11\13\15\17sun";
static const sal_Char *kTestStr36 = "Microsystems Java Technology";
static const sal_Char *kTestStr37 = "Sun  Java Technology";
static const sal_Char *kTestStr38 = "\21\23\25\27\31\33\50";
static const sal_Char *kTestStr39 = "\50\3\5\7\11\13\15\17sun   Sun Microsystems   ";
static const sal_Char *kTestStr40 = "\50\3\5\7\11\13\15\17sunsun Microsystems";
static const sal_Char *kTestStr41 = "Sun";
static const sal_Char *kTestStr42 = "\50\3\5\7\11\13\15\17su";
static const sal_Char *kTestStr43 = "\50\3\5\7\11\13\15\17sun\256\345";
static const sal_Char *kTestStr44 = "\256\345";
static const sal_Char *kTestStr45  = "Sun true";
static const sal_Char *kTestStr46  = "Sun false";
static const sal_Char *kTestStr47  = "true";
static const sal_Char *kTestStr48  = "false";
static const sal_Char *kTestStr49 = "\50\3\5\7\11\13\15\17suntrue";
static const sal_Char *kTestStr50 = "\50\3\5\7\11\13\15\17sunfalse";
static const sal_Char *kTestStr51  = "Sun M";
//static const sal_Char *kTestStr52  = "Sun \077777";
//static const sal_Char *kTestStr53  = "Sun \100000";
//static const sal_Char *kTestStr54  = "\77777";
//static const sal_Char *kTestStr55  = "\100000";
static const sal_Char *kTestStr56 = "\50\3\5\7\11\13\15\17suns";
//static const sal_Char *kTestStr57 = "\50\3\5\7\11\13\15\17sun\77777";
//static const sal_Char *kTestStr58 = "\50\3\5\7\11\13\15\17sun\10000";
static const sal_Char *kTestStr59  = "Sun 11";
static const sal_Char *kTestStr60  = "11";
static const sal_Char *kTestStr61  = "\50\3\5\7\11\13\15\17sun11";
static const sal_Char *kTestStr62  = "Sun 0";
static const sal_Char *kTestStr63  = "Sun -11";
static const sal_Char *kTestStr64  = "Sun 2147483647";
static const sal_Char *kTestStr65  = "Sun -2147483648";
static const sal_Char *kTestStr66  = "0";
static const sal_Char *kTestStr67  = "-11";
static const sal_Char *kTestStr68  = "2147483647";
static const sal_Char *kTestStr69  = "-2147483648";
static const sal_Char *kTestStr70  = "\50\3\5\7\11\13\15\17sun0";
static const sal_Char *kTestStr71  = "\50\3\5\7\11\13\15\17sun-11";
static const sal_Char *kTestStr72  = "\50\3\5\7\11\13\15\17sun2147483647";
static const sal_Char *kTestStr73  = "\50\3\5\7\11\13\15\17sun-2147483648";
static const sal_Char *kTestStr74  = "Sun 3.0";
static const sal_Char *kTestStr75  = "Sun 3.5";
static const sal_Char *kTestStr76  = "Sun 3.0625";
static const sal_Char *kTestStr77  = "Sun 3.502525";
static const sal_Char *kTestStr78  = "Sun 3.141592";
static const sal_Char *kTestStr79  = "Sun 3.5025255";
static const sal_Char *kTestStr80  = "Sun 3.0039062";
static const sal_Char *kTestStr81  = "3.0";
static const sal_Char *kTestStr82  = "3.5";
static const sal_Char *kTestStr83  = "3.0625";
static const sal_Char *kTestStr84  = "3.502525";
static const sal_Char *kTestStr85  = "3.141592";
static const sal_Char *kTestStr86  = "3.5025255";
static const sal_Char *kTestStr87  = "3.0039062";
static const sal_Char *kTestStr88  = "\50\3\5\7\11\13\15\17sun3.0";
static const sal_Char *kTestStr89  = "\50\3\5\7\11\13\15\17sun3.5";
static const sal_Char *kTestStr90  = "\50\3\5\7\11\13\15\17sun3.0625";
static const sal_Char *kTestStr91  = "\50\3\5\7\11\13\15\17sun3.502525";
static const sal_Char *kTestStr92  = "\50\3\5\7\11\13\15\17sun3.141592";
static const sal_Char *kTestStr93  = "\50\3\5\7\11\13\15\17sun3.5025255";
static const sal_Char *kTestStr94  = "\50\3\5\7\11\13\15\17sun3.0039062";
static const sal_Char *kTestStr95  = "Sun -3.0";
static const sal_Char *kTestStr96  = "Sun -3.5";
static const sal_Char *kTestStr97  = "Sun -3.0625";
static const sal_Char *kTestStr98  = "Sun -3.502525";
static const sal_Char *kTestStr99  = "Sun -3.141592";
static const sal_Char *kTestStr100  = "Sun -3.5025255";
static const sal_Char *kTestStr101  = "Sun -3.0039062";
static const sal_Char *kTestStr102  = "-3.0";
static const sal_Char *kTestStr103  = "-3.5";
static const sal_Char *kTestStr104  = "-3.0625";
static const sal_Char *kTestStr105  = "-3.502525";
static const sal_Char *kTestStr106  = "-3.141592";
static const sal_Char *kTestStr107  = "-3.5025255";
static const sal_Char *kTestStr108  = "-3.0039062";
static const sal_Char *kTestStr109  = "\50\3\5\7\11\13\15\17sun-3.0";
static const sal_Char *kTestStr110  = "\50\3\5\7\11\13\15\17sun-3.5";
static const sal_Char *kTestStr111  = "\50\3\5\7\11\13\15\17sun-3.0625";
static const sal_Char *kTestStr112  = "\50\3\5\7\11\13\15\17sun-3.502525";
static const sal_Char *kTestStr113  = "\50\3\5\7\11\13\15\17sun-3.141592";
static const sal_Char *kTestStr114  = "\50\3\5\7\11\13\15\17sun-3.5025255";
static const sal_Char *kTestStr115  = "\50\3\5\7\11\13\15\17sun-3.0039062";
static const sal_Char *kTestStr116  = "Sun 9223372036854775807";
static const sal_Char *kTestStr117  = "Sun -9223372036854775808";
static const sal_Char *kTestStr118  = "9223372036854775807";
static const sal_Char *kTestStr119  = "-9223372036854775808";
static const sal_Char *kTestStr120  = "\50\3\5\7\11\13\15\17sun9223372036854775807";
static const sal_Char *kTestStr121  = "\50\3\5\7\11\13\15\17sun-9223372036854775808";
static const sal_Char *kTestStr122  = "Sun 3.1415926535";
static const sal_Char *kTestStr123  = "Sun 3.141592653589793";
static const sal_Char *kTestStr124  = "Sun 3.14159265358979323";
static const sal_Char *kTestStr125  = "Sun 3.141592653589793238462643";
static const sal_Char *kTestStr126  = "3.1415926535";
static const sal_Char *kTestStr127  = "3.141592653589793";
static const sal_Char *kTestStr128  = "3.14159265358979323";
static const sal_Char *kTestStr129  = "3.141592653589793238462643";
static const sal_Char *kTestStr130  = "\50\3\5\7\11\13\15\17sun3.1415926535";
static const sal_Char *kTestStr131  = "\50\3\5\7\11\13\15\17sun3.141592653589793";
static const sal_Char *kTestStr132  = "\50\3\5\7\11\13\15\17sun3.14159265358979323";
static const sal_Char *kTestStr133  = "\50\3\5\7\11\13\15\17sun3.141592653589793238462643";
static const sal_Char *kTestStr134  = "Sun -3.141592653589793";
static const sal_Char *kTestStr135  = "Sun -3.14159265358979323";
static const sal_Char *kTestStr136  = "Sun -3.141592653589793238462643";
static const sal_Char *kTestStr137  = "-3.141592653589793";
static const sal_Char *kTestStr138  = "-3.14159265358979323";
static const sal_Char *kTestStr139  = "-3.141592653589793238462643";
static const sal_Char *kTestStr140  = "\50\3\5\7\11\13\15\17sun-3.141592653589793";
static const sal_Char *kTestStr141  = "\50\3\5\7\11\13\15\17sun-3.14159265358979323";
static const sal_Char *kTestStr142  = "\50\3\5\7\11\13\15\17sun-3.141592653589793238462643";
static const sal_Char *kTestStr143  = "Sun \377";
static const sal_Char *kTestStr144  = "\377";
static const sal_Char *kTestStr145 = "\50\3\5\7\11\13\15\17sun\377";
static const sal_Char *kTestStr146 = "Sun Java Technology";
static const sal_Char *kTestStr147 = "Java Sun Technology";
static const sal_Char *kTestStr148 = "Java TechnologySun ";
static const sal_Char *kTestStr149 = "Java Sun Microsystems Technology";
static const sal_Char *kTestStr150 = "Java TechnologySun Microsystems ";
static const sal_Char *kTestStr151 = "Sun MicrosystemsJava Technology";
static const sal_Char *kTestStr152 = "Java Sun MicrosystemsTechnology";
static const sal_Char *kTestStr153 = "Java TechnologySun Microsystems";
static const sal_Char *kTestStr154 = "Sun Sun Microsystems Java Technology";
static const sal_Char *kTestStr155 = "Sun Sun Microsystems Java Technology";
static const sal_Char *kTestStr156 = "Sun Microsystems Java TechnologySun ";
static const sal_Char *kTestStr157 = "Sun Microsystems Sun Microsystems Java Technology";
static const sal_Char *kTestStr158 = "Sun Sun Microsystems Microsystems Java Technology";
static const sal_Char *kTestStr159 = "Sun Microsystems Java TechnologySun Microsystems ";
static const sal_Char *kTestStr160 = "Sun MicrosystemsSun Microsystems Java Technology";
static const sal_Char *kTestStr161 = "Sun Sun MicrosystemsMicrosystems Java Technology";
static const sal_Char *kTestStr162 = "Sun Microsystems Java TechnologySun Microsystems";
static const sal_Char *kTestStr163 = "Sun \50\3\5\7\11\13\15\17sun";
static const sal_Char *kTestStr164 = "\50\3\5\7Sun \11\13\15\17sun";
static const sal_Char *kTestStr165 = "\50\3\5\7\11\13\15\17sunSun ";
static const sal_Char *kTestStr166 = "Sun Microsystems \50\3\5\7\11\13\15\17sun";
static const sal_Char *kTestStr167 = "\50\3\5\7Sun Microsystems \11\13\15\17sun";
static const sal_Char *kTestStr168 = "\50\3\5\7\11\13\15\17sunSun Microsystems ";
static const sal_Char *kTestStr169 = "Sun Microsystems\50\3\5\7\11\13\15\17sun";
static const sal_Char *kTestStr170 = "\50\3\5\7Sun Microsystems\11\13\15\17sun";
static const sal_Char *kTestStr171 = "\50\3\5\7\11\13\15\17sunSun Microsystems";
static const sal_Char *kTestStr1PlusStr6  = "Sun Microsystems" "Java Technology";


//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const sal_Int32 kTestStr1Len  = 16;
static const sal_Int32 kTestStr2Len  = 32;
static const sal_Int32 kTestStr3Len  = 16;
static const sal_Int32 kTestStr4Len  = 16;
static const sal_Int32 kTestStr5Len  = 16;
static const sal_Int32 kTestStr6Len  = 15;
static const sal_Int32 kTestStr7Len  = 4;
static const sal_Int32 kTestStr8Len  = 12;
static const sal_Int32 kTestStr9Len  = 32;
static const sal_Int32 kTestStr10Len = 17;
static const sal_Int32 kTestStr11Len = 17;
static const sal_Int32 kTestStr12Len = 18;
static const sal_Int32 kTestStr13Len = 19;
static const sal_Int32 kTestStr14Len = 19;
static const sal_Int32 kTestStr15Len = 20;
static const sal_Int32 kTestStr16Len = 20;
static const sal_Int32 kTestStr17Len = 22;
static const sal_Int32 kTestStr18Len = 16;
static const sal_Int32 kTestStr19Len = 22;
static const sal_Int32 kTestStr20Len = 3;
static const sal_Int32 kTestStr21Len = 3;
static const sal_Int32 kTestStr22Len = 32;
static const sal_Int32 kTestStr23Len = 16;
static const sal_Int32 kTestStr24Len = 31;
static const sal_Int32 kTestStr25Len = 0;
static const sal_Int32 kTestStr26Len = 4;
static const sal_Int32 kTestStr27Len = 1;
static const sal_Int32 kTestStr28Len = 11;
static const sal_Int32 kTestStr29Len = 18;
static const sal_Int32 kTestStr30Len = 10;
static const sal_Int32 kTestStr31Len = 16;
static const sal_Int32 kTestStr32Len = 16;
static const sal_Int32 kTestStr33Len = 1;
static const sal_Int32 kTestStr34Len = 11;
static const sal_Int32 kTestStr35Len = 11;
static const sal_Int32 kTestStr36Len = 28;
static const sal_Int32 kTestStr37Len = 20;
static const sal_Int32 kTestStr38Len = 7;
static const sal_Int32 kTestStr39Len = 33;
static const sal_Int32 kTestStr40Len = 27;
static const sal_Int32 kTestStr41Len = 3;
static const sal_Int32 kTestStr42Len = 10;
static const sal_Int32 kTestStr43Len = 13;
static const sal_Int32 kTestStr44Len = 2;
static const sal_Int32 kTestStr45Len = 8;
static const sal_Int32 kTestStr46Len = 9;
static const sal_Int32 kTestStr47Len = 4;
static const sal_Int32 kTestStr48Len = 5;
static const sal_Int32 kTestStr49Len = 15;
static const sal_Int32 kTestStr50Len = 16;
static const sal_Int32 kTestStr51Len = 5;
static const sal_Int32 kTestStr52Len = 5;
static const sal_Int32 kTestStr53Len = 5;
static const sal_Int32 kTestStr54Len = 1;
static const sal_Int32 kTestStr55Len = 1;
static const sal_Int32 kTestStr56Len = 12;
static const sal_Int32 kTestStr57Len = 12;
static const sal_Int32 kTestStr58Len = 12;
static const sal_Int32 kTestStr59Len = 6;
static const sal_Int32 kTestStr60Len = 2;
static const sal_Int32 kTestStr61Len = 13;
static const sal_Int32 kTestStr62Len = 5;
static const sal_Int32 kTestStr63Len = 7;
static const sal_Int32 kTestStr64Len = 14;
static const sal_Int32 kTestStr65Len = 15;
static const sal_Int32 kTestStr66Len = 1;
static const sal_Int32 kTestStr67Len = 3;
static const sal_Int32 kTestStr68Len = 10;
static const sal_Int32 kTestStr69Len = 11;
static const sal_Int32 kTestStr70Len = 12;
static const sal_Int32 kTestStr71Len = 14;
static const sal_Int32 kTestStr72Len = 21;
static const sal_Int32 kTestStr73Len = 22;
static const sal_Int32 kTestStr74Len = 7;
static const sal_Int32 kTestStr75Len = 7;
static const sal_Int32 kTestStr76Len = 10;
static const sal_Int32 kTestStr77Len = 12;
static const sal_Int32 kTestStr78Len = 12;
static const sal_Int32 kTestStr79Len = 13;
static const sal_Int32 kTestStr80Len = 13;
static const sal_Int32 kTestStr81Len = 3;
static const sal_Int32 kTestStr82Len = 3;
static const sal_Int32 kTestStr83Len = 6;
static const sal_Int32 kTestStr84Len = 8;
static const sal_Int32 kTestStr85Len = 8;
static const sal_Int32 kTestStr86Len = 9;
static const sal_Int32 kTestStr87Len = 9;
static const sal_Int32 kTestStr88Len = 14;
static const sal_Int32 kTestStr89Len = 14;
static const sal_Int32 kTestStr90Len = 17;
static const sal_Int32 kTestStr91Len = 19;
static const sal_Int32 kTestStr92Len = 19;
static const sal_Int32 kTestStr93Len = 20;
static const sal_Int32 kTestStr94Len = 20;
static const sal_Int32 kTestStr95Len = 8;
static const sal_Int32 kTestStr96Len = 8;
static const sal_Int32 kTestStr97Len = 11;
static const sal_Int32 kTestStr98Len = 13;
static const sal_Int32 kTestStr99Len = 13;
static const sal_Int32 kTestStr100Len = 14;
static const sal_Int32 kTestStr101Len = 14;
static const sal_Int32 kTestStr102Len = 4;
static const sal_Int32 kTestStr103Len = 4;
static const sal_Int32 kTestStr104Len = 7;
static const sal_Int32 kTestStr105Len = 9;
static const sal_Int32 kTestStr106Len = 9;
static const sal_Int32 kTestStr107Len = 10;
static const sal_Int32 kTestStr108Len = 10;
static const sal_Int32 kTestStr109Len = 15;
static const sal_Int32 kTestStr110Len = 15;
static const sal_Int32 kTestStr111Len = 18;
static const sal_Int32 kTestStr112Len = 20;
static const sal_Int32 kTestStr113Len = 20;
static const sal_Int32 kTestStr114Len = 21;
static const sal_Int32 kTestStr115Len = 21;
static const sal_Int32 kTestStr116Len = 23;
static const sal_Int32 kTestStr117Len = 24;
static const sal_Int32 kTestStr118Len = 19;
static const sal_Int32 kTestStr119Len = 20;
static const sal_Int32 kTestStr120Len = 30;
static const sal_Int32 kTestStr121Len = 31;
static const sal_Int32 kTestStr122Len = 16;
static const sal_Int32 kTestStr123Len = 21;
static const sal_Int32 kTestStr124Len = 23;
static const sal_Int32 kTestStr125Len = 30;
static const sal_Int32 kTestStr126Len = 12;
static const sal_Int32 kTestStr127Len = 17;
static const sal_Int32 kTestStr128Len = 19;
static const sal_Int32 kTestStr129Len = 26;
static const sal_Int32 kTestStr130Len = 23;
static const sal_Int32 kTestStr131Len = 28;
static const sal_Int32 kTestStr132Len = 30;
static const sal_Int32 kTestStr133Len = 37;
static const sal_Int32 kTestStr134Len = 22;
static const sal_Int32 kTestStr135Len = 24;
static const sal_Int32 kTestStr136Len = 31;
static const sal_Int32 kTestStr137Len = 18;
static const sal_Int32 kTestStr138Len = 20;
static const sal_Int32 kTestStr139Len = 27;
static const sal_Int32 kTestStr140Len = 29;
static const sal_Int32 kTestStr141Len = 31;
static const sal_Int32 kTestStr142Len = 38;
static const sal_Int32 kTestStr143Len = 5;
static const sal_Int32 kTestStr144Len = 1;
static const sal_Int32 kTestStr145Len = 12;
static const sal_Int32 kTestStr146Len = 19;
static const sal_Int32 kTestStr147Len = 19;
static const sal_Int32 kTestStr148Len = 19;
static const sal_Int32 kTestStr149Len = 32;
static const sal_Int32 kTestStr150Len = 32;
static const sal_Int32 kTestStr151Len = 31;
static const sal_Int32 kTestStr152Len = 31;
static const sal_Int32 kTestStr153Len = 31;
static const sal_Int32 kTestStr154Len = 36;
static const sal_Int32 kTestStr155Len = 36;
static const sal_Int32 kTestStr156Len = 36;
static const sal_Int32 kTestStr157Len = 49;
static const sal_Int32 kTestStr158Len = 49;
static const sal_Int32 kTestStr159Len = 49;
static const sal_Int32 kTestStr160Len = 48;
static const sal_Int32 kTestStr161Len = 48;
static const sal_Int32 kTestStr162Len = 48;
static const sal_Int32 kTestStr163Len = 15;
static const sal_Int32 kTestStr164Len = 15;
static const sal_Int32 kTestStr165Len = 15;
static const sal_Int32 kTestStr166Len = 28;
static const sal_Int32 kTestStr167Len = 28;
static const sal_Int32 kTestStr168Len = 28;
static const sal_Int32 kTestStr169Len = 27;
static const sal_Int32 kTestStr170Len = 27;
static const sal_Int32 kTestStr171Len = 27;
static const sal_Int32 kTestStr1PlusStr6Len = kTestStr1Len + kTestStr6Len;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
 static sal_Unicode aUStr1[kTestStr1Len+1];
 static sal_Unicode aUStr2[kTestStr2Len+1];
 static sal_Unicode aUStr3[kTestStr3Len+1];
 static sal_Unicode aUStr4[kTestStr4Len+1];
 static sal_Unicode aUStr5[kTestStr5Len+1];
 static sal_Unicode aUStr6[kTestStr6Len+1];
 static sal_Unicode aUStr7[kTestStr7Len+1];
 static sal_Unicode aUStr8[kTestStr8Len+1];
 static sal_Unicode aUStr9[kTestStr9Len+1];
 static sal_Unicode aUStr10[kTestStr10Len+1];
 static sal_Unicode aUStr11[kTestStr11Len+1];
 static sal_Unicode aUStr12[kTestStr12Len+1];
 static sal_Unicode aUStr13[kTestStr13Len+1];
 static sal_Unicode aUStr14[kTestStr14Len+1];
 static sal_Unicode aUStr15[kTestStr15Len+1];
 static sal_Unicode aUStr16[kTestStr16Len+1];
 static sal_Unicode aUStr17[kTestStr17Len+1];
 static sal_Unicode aUStr18[kTestStr18Len+1];
 static sal_Unicode aUStr19[kTestStr19Len+1];
 static sal_Unicode aUStr20[kTestStr20Len+1];
 static sal_Unicode aUStr21[kTestStr21Len+1];
 static sal_Unicode aUStr22[kTestStr22Len+1];
 static sal_Unicode aUStr23[kTestStr23Len+1];
 static sal_Unicode aUStr24[kTestStr24Len+1];
 static sal_Unicode aUStr25[kTestStr25Len+1];
 static sal_Unicode aUStr26[kTestStr26Len+1];
 static sal_Unicode aUStr27[kTestStr27Len+1];
 static sal_Unicode aUStr28[kTestStr28Len+1];
 static sal_Unicode aUStr29[kTestStr29Len+1];
 static sal_Unicode aUStr30[kTestStr30Len+1];
 static sal_Unicode aUStr31[kTestStr31Len+1];
 static sal_Unicode aUStr32[kTestStr32Len+1];
 static sal_Unicode aUStr33[kTestStr33Len+1];
 static sal_Unicode aUStr34[kTestStr34Len+1];
 static sal_Unicode aUStr35[kTestStr35Len+1];
 static sal_Unicode aUStr36[kTestStr36Len+1];
 static sal_Unicode aUStr37[kTestStr37Len+1];
 static sal_Unicode aUStr38[kTestStr38Len+1];
 static sal_Unicode aUStr39[kTestStr39Len+1];
 static sal_Unicode aUStr40[kTestStr40Len+1];
 static sal_Unicode aUStr41[kTestStr41Len+1];
 static sal_Unicode aUStr42[kTestStr42Len+1];
 static sal_Unicode aUStr43[kTestStr43Len+1];
 static sal_Unicode aUStr44[kTestStr44Len+1];
 static sal_Unicode aUStr45[kTestStr45Len+1];
 static sal_Unicode aUStr46[kTestStr46Len+1];
 static sal_Unicode aUStr47[kTestStr47Len+1];
 static sal_Unicode aUStr48[kTestStr48Len+1];
 static sal_Unicode aUStr49[kTestStr49Len+1];
 static sal_Unicode aUStr50[kTestStr50Len+1];
 static sal_Unicode aUStr51[kTestStr51Len+1];
 static sal_Unicode aUStr52[kTestStr52Len+1]={83,117,110,32,32767};
 static sal_Unicode aUStr53[kTestStr53Len+1]={83,117,110,32,-32768};
 static sal_Unicode aUStr54[kTestStr54Len+1]={32767};
 static sal_Unicode aUStr55[kTestStr55Len+1]={-32768};
 static sal_Unicode aUStr56[kTestStr56Len+1];
 static sal_Unicode aUStr57[kTestStr57Len+1]={40,3,5,7,9,11,13,15,115,117,110,32767};
 static sal_Unicode aUStr58[kTestStr58Len+1]={40,3,5,7,9,11,13,15,115,117,110,-32768};
 static sal_Unicode aUStr59[kTestStr59Len+1];
 static sal_Unicode aUStr60[kTestStr60Len+1];
 static sal_Unicode aUStr61[kTestStr61Len+1];
 static sal_Unicode aUStr62[kTestStr62Len+1];
 static sal_Unicode aUStr63[kTestStr63Len+1];
 static sal_Unicode aUStr64[kTestStr64Len+1];
 static sal_Unicode aUStr65[kTestStr65Len+1];
 static sal_Unicode aUStr66[kTestStr66Len+1];
 static sal_Unicode aUStr67[kTestStr67Len+1];
 static sal_Unicode aUStr68[kTestStr68Len+1];
 static sal_Unicode aUStr69[kTestStr69Len+1];
 static sal_Unicode aUStr70[kTestStr70Len+1];
 static sal_Unicode aUStr71[kTestStr71Len+1];
 static sal_Unicode aUStr72[kTestStr72Len+1];
 static sal_Unicode aUStr73[kTestStr73Len+1];
 static sal_Unicode aUStr74[kTestStr74Len+1];
 static sal_Unicode aUStr75[kTestStr75Len+1];
 static sal_Unicode aUStr76[kTestStr76Len+1];
 static sal_Unicode aUStr77[kTestStr77Len+1];
 static sal_Unicode aUStr78[kTestStr78Len+1];
 static sal_Unicode aUStr79[kTestStr79Len+1];
 static sal_Unicode aUStr80[kTestStr80Len+1];
 static sal_Unicode aUStr81[kTestStr81Len+1];
 static sal_Unicode aUStr82[kTestStr82Len+1];
 static sal_Unicode aUStr83[kTestStr83Len+1];
 static sal_Unicode aUStr84[kTestStr84Len+1];
 static sal_Unicode aUStr85[kTestStr85Len+1];
 static sal_Unicode aUStr86[kTestStr86Len+1];
 static sal_Unicode aUStr87[kTestStr87Len+1];
 static sal_Unicode aUStr88[kTestStr88Len+1];
 static sal_Unicode aUStr89[kTestStr89Len+1];
 static sal_Unicode aUStr90[kTestStr90Len+1];
 static sal_Unicode aUStr91[kTestStr91Len+1];
 static sal_Unicode aUStr92[kTestStr92Len+1];
 static sal_Unicode aUStr93[kTestStr93Len+1];
 static sal_Unicode aUStr94[kTestStr94Len+1];
 static sal_Unicode aUStr95[kTestStr95Len+1];
 static sal_Unicode aUStr96[kTestStr96Len+1];
 static sal_Unicode aUStr97[kTestStr97Len+1];
 static sal_Unicode aUStr98[kTestStr98Len+1];
 static sal_Unicode aUStr99[kTestStr99Len+1];
 static sal_Unicode aUStr100[kTestStr100Len+1];
 static sal_Unicode aUStr101[kTestStr101Len+1];
 static sal_Unicode aUStr102[kTestStr102Len+1];
 static sal_Unicode aUStr103[kTestStr103Len+1];
 static sal_Unicode aUStr104[kTestStr104Len+1];
 static sal_Unicode aUStr105[kTestStr105Len+1];
 static sal_Unicode aUStr106[kTestStr106Len+1];
 static sal_Unicode aUStr107[kTestStr107Len+1];
 static sal_Unicode aUStr108[kTestStr108Len+1];
 static sal_Unicode aUStr109[kTestStr109Len+1];
 static sal_Unicode aUStr110[kTestStr110Len+1];
 static sal_Unicode aUStr111[kTestStr111Len+1];
 static sal_Unicode aUStr112[kTestStr112Len+1];
 static sal_Unicode aUStr113[kTestStr113Len+1];
 static sal_Unicode aUStr114[kTestStr114Len+1];
 static sal_Unicode aUStr115[kTestStr115Len+1];
 static sal_Unicode aUStr116[kTestStr116Len+1];
 static sal_Unicode aUStr117[kTestStr117Len+1];
 static sal_Unicode aUStr118[kTestStr118Len+1];
 static sal_Unicode aUStr119[kTestStr119Len+1];
 static sal_Unicode aUStr120[kTestStr120Len+1];
 static sal_Unicode aUStr121[kTestStr121Len+1];
 static sal_Unicode aUStr122[kTestStr122Len+1];
 static sal_Unicode aUStr123[kTestStr123Len+1];
 static sal_Unicode aUStr124[kTestStr124Len+1];
 static sal_Unicode aUStr125[kTestStr125Len+1];
 static sal_Unicode aUStr126[kTestStr126Len+1];
 static sal_Unicode aUStr127[kTestStr127Len+1];
 static sal_Unicode aUStr128[kTestStr128Len+1];
 static sal_Unicode aUStr129[kTestStr129Len+1];
 static sal_Unicode aUStr130[kTestStr130Len+1];
 static sal_Unicode aUStr131[kTestStr131Len+1];
 static sal_Unicode aUStr132[kTestStr132Len+1];
 static sal_Unicode aUStr133[kTestStr133Len+1];
 static sal_Unicode aUStr134[kTestStr134Len+1];
 static sal_Unicode aUStr135[kTestStr135Len+1];
 static sal_Unicode aUStr136[kTestStr136Len+1];
 static sal_Unicode aUStr137[kTestStr137Len+1];
 static sal_Unicode aUStr138[kTestStr138Len+1];
 static sal_Unicode aUStr139[kTestStr139Len+1];
 static sal_Unicode aUStr140[kTestStr140Len+1];
 static sal_Unicode aUStr141[kTestStr141Len+1];
 static sal_Unicode aUStr142[kTestStr142Len+1];
 static sal_Unicode aUStr143[kTestStr143Len+1];
 static sal_Unicode aUStr144[kTestStr144Len+1];
 static sal_Unicode aUStr145[kTestStr145Len+1];
 static sal_Unicode aUStr146[kTestStr146Len+1];
 static sal_Unicode aUStr147[kTestStr147Len+1];
 static sal_Unicode aUStr148[kTestStr148Len+1];
 static sal_Unicode aUStr149[kTestStr149Len+1];
 static sal_Unicode aUStr150[kTestStr150Len+1];
 static sal_Unicode aUStr151[kTestStr151Len+1];
 static sal_Unicode aUStr152[kTestStr152Len+1];
 static sal_Unicode aUStr153[kTestStr153Len+1];
 static sal_Unicode aUStr154[kTestStr154Len+1];
 static sal_Unicode aUStr155[kTestStr155Len+1];
 static sal_Unicode aUStr156[kTestStr156Len+1];
 static sal_Unicode aUStr157[kTestStr157Len+1];
 static sal_Unicode aUStr158[kTestStr158Len+1];
 static sal_Unicode aUStr159[kTestStr159Len+1];
 static sal_Unicode aUStr160[kTestStr160Len+1];
 static sal_Unicode aUStr161[kTestStr161Len+1];
 static sal_Unicode aUStr162[kTestStr162Len+1];
 static sal_Unicode aUStr163[kTestStr163Len+1];
 static sal_Unicode aUStr164[kTestStr164Len+1];
 static sal_Unicode aUStr165[kTestStr165Len+1];
 static sal_Unicode aUStr166[kTestStr166Len+1];
 static sal_Unicode aUStr167[kTestStr167Len+1];
 static sal_Unicode aUStr168[kTestStr168Len+1];
 static sal_Unicode aUStr169[kTestStr169Len+1];
 static sal_Unicode aUStr170[kTestStr170Len+1];
 static sal_Unicode aUStr171[kTestStr171Len+1];
 static sal_Unicode aUStr1PlusUStr6[kTestStr1Len + kTestStr6Len + 1];

// we are already in "C"
static sal_Bool SAL_CALL test_ini_uString()
{

    sal_Bool iniResult= sal_True;

    iniResult &=AStringToUStringNCopy( aUStr1, kTestStr1, kTestStr1Len );

    iniResult &=AStringToUStringNCopy( aUStr2, kTestStr2, kTestStr2Len );

    iniResult &=AStringToUStringNCopy( aUStr3, kTestStr3, kTestStr3Len );

    iniResult &=AStringToUStringNCopy( aUStr4, kTestStr4, kTestStr4Len );

    iniResult &=AStringToUStringNCopy( aUStr5, kTestStr5, kTestStr5Len );

    iniResult &=AStringToUStringNCopy( aUStr6, kTestStr6, kTestStr6Len );

        iniResult &=AStringToUStringNCopy( aUStr7, kTestStr7, kTestStr7Len );

        iniResult &=AStringToUStringNCopy( aUStr8, kTestStr8, kTestStr8Len );

    iniResult &=AStringToUStringNCopy( aUStr9, kTestStr9, kTestStr9Len );

    iniResult &=AStringToUStringNCopy( aUStr10, kTestStr10, kTestStr10Len );

    iniResult &=AStringToUStringNCopy( aUStr11, kTestStr11, kTestStr11Len );

        iniResult &=AStringToUStringNCopy( aUStr12, kTestStr12, kTestStr12Len );

        iniResult &=AStringToUStringNCopy( aUStr13, kTestStr13, kTestStr13Len );

        iniResult &=AStringToUStringNCopy( aUStr14, kTestStr14, kTestStr14Len );

        iniResult &=AStringToUStringNCopy( aUStr15, kTestStr15, kTestStr15Len );

        iniResult &=AStringToUStringNCopy( aUStr16, kTestStr16, kTestStr16Len );

        iniResult &=AStringToUStringNCopy( aUStr17, kTestStr17, kTestStr17Len );

        iniResult &=AStringToUStringNCopy( aUStr18, kTestStr18, kTestStr18Len );

        iniResult &=AStringToUStringNCopy( aUStr19, kTestStr19, kTestStr19Len );

        iniResult &=AStringToUStringNCopy( aUStr20, kTestStr20, kTestStr20Len );

        iniResult &=AStringToUStringNCopy( aUStr21, kTestStr21, kTestStr21Len );

    iniResult &=AStringToUStringNCopy( aUStr22, kTestStr22, kTestStr22Len );

    iniResult &=AStringToUStringNCopy( aUStr23, kTestStr23, kTestStr23Len );

    iniResult &=AStringToUStringNCopy( aUStr1PlusUStr6, kTestStr1PlusStr6, kTestStr1PlusStr6Len );

        iniResult &=AStringToUStringNCopy( aUStr24, kTestStr24, kTestStr24Len );

        iniResult &=AStringToUStringNCopy( aUStr25, kTestStr25, kTestStr25Len );

        iniResult &=AStringToUStringNCopy( aUStr26, kTestStr26, kTestStr26Len );

        iniResult &=AStringToUStringNCopy( aUStr27, kTestStr27, kTestStr27Len );

        iniResult &=AStringToUStringNCopy( aUStr28, kTestStr28, kTestStr28Len );

        iniResult &=AStringToUStringNCopy( aUStr29, kTestStr29, kTestStr29Len );

        iniResult &=AStringToUStringNCopy( aUStr30, kTestStr30, kTestStr30Len );

        iniResult &=AStringToUStringNCopy( aUStr31, kTestStr31, kTestStr31Len );

        iniResult &=AStringToUStringNCopy( aUStr32, kTestStr32, kTestStr32Len );

        iniResult &=AStringToUStringNCopy( aUStr33, kTestStr33, kTestStr33Len );

        iniResult &=AStringToUStringNCopy( aUStr34, kTestStr34, kTestStr34Len );

        iniResult &=AStringToUStringNCopy( aUStr35, kTestStr35, kTestStr35Len );

        iniResult &=AStringToUStringNCopy( aUStr36, kTestStr36, kTestStr36Len );

        iniResult &=AStringToUStringNCopy( aUStr37, kTestStr37, kTestStr37Len );

        iniResult &=AStringToUStringNCopy( aUStr38, kTestStr38, kTestStr38Len );

        iniResult &=AStringToUStringNCopy( aUStr39, kTestStr39, kTestStr39Len );

        iniResult &=AStringToUStringNCopy( aUStr40, kTestStr40, kTestStr40Len );

        iniResult &=AStringToUStringNCopy( aUStr41, kTestStr41, kTestStr41Len );

        iniResult &=AStringToUStringNCopy( aUStr42, kTestStr42, kTestStr42Len );

        iniResult &=AStringToUStringNCopy( aUStr43, kTestStr43, kTestStr43Len );

        iniResult &=AStringToUStringNCopy( aUStr44, kTestStr44, kTestStr44Len );

        iniResult &=AStringToUStringNCopy( aUStr45, kTestStr45, kTestStr45Len );

        iniResult &=AStringToUStringNCopy( aUStr46, kTestStr46, kTestStr46Len );

        iniResult &=AStringToUStringNCopy( aUStr47, kTestStr47, kTestStr47Len );

        iniResult &=AStringToUStringNCopy( aUStr48, kTestStr48, kTestStr48Len );

        iniResult &=AStringToUStringNCopy( aUStr49, kTestStr49, kTestStr49Len );

        iniResult &=AStringToUStringNCopy( aUStr50, kTestStr50, kTestStr50Len );

        iniResult &=AStringToUStringNCopy( aUStr51, kTestStr51, kTestStr51Len );

        //iniResult &=AStringToUStringNCopy( aUStr52, kTestStr52, kTestStr52Len );

       // iniResult &=AStringToUStringNCopy( aUStr53, kTestStr53, kTestStr53Len );

        //iniResult &=AStringToUStringNCopy( aUStr54, kTestStr54, kTestStr54Len );

        //iniResult &=AStringToUStringNCopy( aUStr55, kTestStr55, kTestStr55Len );

        iniResult &=AStringToUStringNCopy( aUStr56, kTestStr56, kTestStr56Len );

       // iniResult &=AStringToUStringNCopy( aUStr57, kTestStr57, kTestStr57Len );

        //iniResult &=AStringToUStringNCopy( aUStr58, kTestStr58, kTestStr58Len );

        iniResult &=AStringToUStringNCopy( aUStr59, kTestStr59, kTestStr59Len );

        iniResult &=AStringToUStringNCopy( aUStr60, kTestStr60, kTestStr60Len );

        iniResult &=AStringToUStringNCopy( aUStr61, kTestStr61, kTestStr61Len );

        iniResult &=AStringToUStringNCopy( aUStr62, kTestStr62, kTestStr62Len );

        iniResult &=AStringToUStringNCopy( aUStr63, kTestStr63, kTestStr63Len );

        iniResult &=AStringToUStringNCopy( aUStr64, kTestStr64, kTestStr64Len );

        iniResult &=AStringToUStringNCopy( aUStr65, kTestStr65, kTestStr65Len );

        iniResult &=AStringToUStringNCopy( aUStr66, kTestStr66, kTestStr66Len );

        iniResult &=AStringToUStringNCopy( aUStr67, kTestStr67, kTestStr67Len );

        iniResult &=AStringToUStringNCopy( aUStr68, kTestStr68, kTestStr68Len );

        iniResult &=AStringToUStringNCopy( aUStr69, kTestStr69, kTestStr69Len );

        iniResult &=AStringToUStringNCopy( aUStr70, kTestStr70, kTestStr70Len );

        iniResult &=AStringToUStringNCopy( aUStr71, kTestStr71, kTestStr71Len );

        iniResult &=AStringToUStringNCopy( aUStr72, kTestStr72, kTestStr72Len );

        iniResult &=AStringToUStringNCopy( aUStr73, kTestStr73, kTestStr73Len );

        iniResult &=AStringToUStringNCopy( aUStr74, kTestStr74, kTestStr74Len );

        iniResult &=AStringToUStringNCopy( aUStr75, kTestStr75, kTestStr75Len );

        iniResult &=AStringToUStringNCopy( aUStr76, kTestStr76, kTestStr76Len );

        iniResult &=AStringToUStringNCopy( aUStr77, kTestStr77, kTestStr77Len );

        iniResult &=AStringToUStringNCopy( aUStr78, kTestStr78, kTestStr78Len );

        iniResult &=AStringToUStringNCopy( aUStr79, kTestStr79, kTestStr79Len );

        iniResult &=AStringToUStringNCopy( aUStr80, kTestStr80, kTestStr80Len );

        iniResult &=AStringToUStringNCopy( aUStr81, kTestStr81, kTestStr81Len );

        iniResult &=AStringToUStringNCopy( aUStr82, kTestStr82, kTestStr82Len );

        iniResult &=AStringToUStringNCopy( aUStr83, kTestStr83, kTestStr83Len );

        iniResult &=AStringToUStringNCopy( aUStr84, kTestStr84, kTestStr84Len );

        iniResult &=AStringToUStringNCopy( aUStr85, kTestStr85, kTestStr85Len );

        iniResult &=AStringToUStringNCopy( aUStr86, kTestStr86, kTestStr86Len );

        iniResult &=AStringToUStringNCopy( aUStr87, kTestStr87, kTestStr87Len );

        iniResult &=AStringToUStringNCopy( aUStr88, kTestStr88, kTestStr88Len );

        iniResult &=AStringToUStringNCopy( aUStr89, kTestStr89, kTestStr89Len );

        iniResult &=AStringToUStringNCopy( aUStr90, kTestStr90, kTestStr90Len );

        iniResult &=AStringToUStringNCopy( aUStr91, kTestStr91, kTestStr91Len );

        iniResult &=AStringToUStringNCopy( aUStr92, kTestStr92, kTestStr92Len );

        iniResult &=AStringToUStringNCopy( aUStr93, kTestStr93, kTestStr93Len );

        iniResult &=AStringToUStringNCopy( aUStr94, kTestStr94, kTestStr94Len );

        iniResult &=AStringToUStringNCopy( aUStr95, kTestStr95, kTestStr95Len );

        iniResult &=AStringToUStringNCopy( aUStr96, kTestStr96, kTestStr96Len );

        iniResult &=AStringToUStringNCopy( aUStr97, kTestStr97, kTestStr97Len );

        iniResult &=AStringToUStringNCopy( aUStr98, kTestStr98, kTestStr98Len );

        iniResult &=AStringToUStringNCopy( aUStr99, kTestStr99, kTestStr99Len );

        iniResult &=AStringToUStringNCopy( aUStr100, kTestStr100, kTestStr100Len );

        iniResult &=AStringToUStringNCopy( aUStr101, kTestStr101, kTestStr101Len );

        iniResult &=AStringToUStringNCopy( aUStr102, kTestStr102, kTestStr102Len );

        iniResult &=AStringToUStringNCopy( aUStr103, kTestStr103, kTestStr103Len );

        iniResult &=AStringToUStringNCopy( aUStr104, kTestStr104, kTestStr104Len );

        iniResult &=AStringToUStringNCopy( aUStr105, kTestStr105, kTestStr105Len );

        iniResult &=AStringToUStringNCopy( aUStr106, kTestStr106, kTestStr106Len );

        iniResult &=AStringToUStringNCopy( aUStr107, kTestStr107, kTestStr107Len );

        iniResult &=AStringToUStringNCopy( aUStr108, kTestStr108, kTestStr108Len );

        iniResult &=AStringToUStringNCopy( aUStr109, kTestStr109, kTestStr109Len );

        iniResult &=AStringToUStringNCopy( aUStr110, kTestStr110, kTestStr110Len );

        iniResult &=AStringToUStringNCopy( aUStr111, kTestStr111, kTestStr111Len );

        iniResult &=AStringToUStringNCopy( aUStr112, kTestStr112, kTestStr112Len );

        iniResult &=AStringToUStringNCopy( aUStr113, kTestStr113, kTestStr113Len );

        iniResult &=AStringToUStringNCopy( aUStr114, kTestStr114, kTestStr114Len );

        iniResult &=AStringToUStringNCopy( aUStr115, kTestStr115, kTestStr115Len );

        iniResult &=AStringToUStringNCopy( aUStr116, kTestStr116, kTestStr116Len );

        iniResult &=AStringToUStringNCopy( aUStr117, kTestStr117, kTestStr117Len );

        iniResult &=AStringToUStringNCopy( aUStr118, kTestStr118, kTestStr118Len );

        iniResult &=AStringToUStringNCopy( aUStr119, kTestStr119, kTestStr119Len );

        iniResult &=AStringToUStringNCopy( aUStr120, kTestStr120, kTestStr120Len );

        iniResult &=AStringToUStringNCopy( aUStr121, kTestStr121, kTestStr121Len );

        iniResult &=AStringToUStringNCopy( aUStr122, kTestStr122, kTestStr122Len );

        iniResult &=AStringToUStringNCopy( aUStr123, kTestStr123, kTestStr123Len );

        iniResult &=AStringToUStringNCopy( aUStr124, kTestStr124, kTestStr124Len );

        iniResult &=AStringToUStringNCopy( aUStr125, kTestStr125, kTestStr125Len );

        iniResult &=AStringToUStringNCopy( aUStr126, kTestStr126, kTestStr126Len );

        iniResult &=AStringToUStringNCopy( aUStr127, kTestStr127, kTestStr127Len );

        iniResult &=AStringToUStringNCopy( aUStr128, kTestStr128, kTestStr128Len );

        iniResult &=AStringToUStringNCopy( aUStr129, kTestStr129, kTestStr129Len );

        iniResult &=AStringToUStringNCopy( aUStr130, kTestStr130, kTestStr130Len );

        iniResult &=AStringToUStringNCopy( aUStr131, kTestStr131, kTestStr131Len );

        iniResult &=AStringToUStringNCopy( aUStr132, kTestStr132, kTestStr132Len );

        iniResult &=AStringToUStringNCopy( aUStr133, kTestStr133, kTestStr133Len );

        iniResult &=AStringToUStringNCopy( aUStr134, kTestStr134, kTestStr134Len );

        iniResult &=AStringToUStringNCopy( aUStr135, kTestStr135, kTestStr135Len );

        iniResult &=AStringToUStringNCopy( aUStr136, kTestStr136, kTestStr136Len );

        iniResult &=AStringToUStringNCopy( aUStr137, kTestStr137, kTestStr137Len );

        iniResult &=AStringToUStringNCopy( aUStr138, kTestStr138, kTestStr138Len );

        iniResult &=AStringToUStringNCopy( aUStr139, kTestStr139, kTestStr139Len );

        iniResult &=AStringToUStringNCopy( aUStr140, kTestStr140, kTestStr140Len );

        iniResult &=AStringToUStringNCopy( aUStr141, kTestStr141, kTestStr141Len );

        iniResult &=AStringToUStringNCopy( aUStr142, kTestStr142, kTestStr142Len );

        iniResult &=AStringToUStringNCopy( aUStr143, kTestStr143, kTestStr143Len );

        iniResult &=AStringToUStringNCopy( aUStr144, kTestStr144, kTestStr144Len );

        iniResult &=AStringToUStringNCopy( aUStr145, kTestStr145, kTestStr145Len );

        iniResult &=AStringToUStringNCopy( aUStr146, kTestStr146, kTestStr146Len );

        iniResult &=AStringToUStringNCopy( aUStr147, kTestStr147, kTestStr147Len );

        iniResult &=AStringToUStringNCopy( aUStr148, kTestStr148, kTestStr148Len );

        iniResult &=AStringToUStringNCopy( aUStr149, kTestStr149, kTestStr149Len );

        iniResult &=AStringToUStringNCopy( aUStr150, kTestStr150, kTestStr150Len );

        iniResult &=AStringToUStringNCopy( aUStr151, kTestStr151, kTestStr151Len );

        iniResult &=AStringToUStringNCopy( aUStr152, kTestStr152, kTestStr152Len );

        iniResult &=AStringToUStringNCopy( aUStr153, kTestStr153, kTestStr153Len );

        iniResult &=AStringToUStringNCopy( aUStr154, kTestStr154, kTestStr154Len );

        iniResult &=AStringToUStringNCopy( aUStr155, kTestStr155, kTestStr155Len );

        iniResult &=AStringToUStringNCopy( aUStr156, kTestStr156, kTestStr156Len );

        iniResult &=AStringToUStringNCopy( aUStr157, kTestStr157, kTestStr157Len );

        iniResult &=AStringToUStringNCopy( aUStr158, kTestStr158, kTestStr158Len );

        iniResult &=AStringToUStringNCopy( aUStr159, kTestStr159, kTestStr159Len );

        iniResult &=AStringToUStringNCopy( aUStr160, kTestStr160, kTestStr160Len );

        iniResult &=AStringToUStringNCopy( aUStr161, kTestStr161, kTestStr161Len );

        iniResult &=AStringToUStringNCopy( aUStr162, kTestStr162, kTestStr162Len );

        iniResult &=AStringToUStringNCopy( aUStr163, kTestStr163, kTestStr163Len );

        iniResult &=AStringToUStringNCopy( aUStr164, kTestStr164, kTestStr164Len );

        iniResult &=AStringToUStringNCopy( aUStr165, kTestStr165, kTestStr165Len );

        iniResult &=AStringToUStringNCopy( aUStr166, kTestStr166, kTestStr166Len );

        iniResult &=AStringToUStringNCopy( aUStr167, kTestStr167, kTestStr167Len );

        iniResult &=AStringToUStringNCopy( aUStr168, kTestStr168, kTestStr168Len );

        iniResult &=AStringToUStringNCopy( aUStr169, kTestStr169, kTestStr169Len );

        iniResult &=AStringToUStringNCopy( aUStr170, kTestStr170, kTestStr170Len );

        iniResult &=AStringToUStringNCopy( aUStr171, kTestStr171, kTestStr171Len );

    return iniResult;

}




static const sal_Int32 uTestStr1Len  = 16;
static const sal_Int32 uTestStr2Len  = 32;
static const sal_Int32 uTestStr3Len  = 16;
static const sal_Int32 uTestStr4Len  = 16;
static const sal_Int32 uTestStr5Len  = 16;
static const sal_Int32 uTestStr9Len  = 32;
static const sal_Int32 uTestStr22Len = 32;



//------------------------------------------------------------------------
//------------------------------------------------------------------------
const sal_Unicode uTestStr31[]= {0x400,0x410,0x4DF};
const sal_Unicode uTestStr32[]= {0x9F9F,0xA000,0x8F80,0x9AD9};



//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const sal_Int32 uTestStr31Len  = 3;
static const sal_Int32 uTestStr32Len  = 4;

//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const sal_Int16 kRadixBinary     = 2;
static const sal_Int16 kRadixOctol      = 8;
static const sal_Int16 kRadixDecimal    = 10;
static const sal_Int16 kRadixHexdecimal = 16;
static const sal_Int16 kRadixBase36     = 36;

//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const sal_Int8  kSInt8Max  = SCHAR_MAX;
static const sal_Int16 kUInt8Max  = UCHAR_MAX;
static const sal_Int16 kSInt16Max = SHRT_MAX;
static const sal_Int32 kUInt16Max = USHRT_MAX;
static const sal_Int32 kSInt32Max = INT_MAX;
static const sal_Int64 kUInt32Max = UINT_MAX;
static const sal_Int64 kSInt64Max = 9223372036854775807;

//------------------------------------------------------------------------

static const sal_Int32 kInt32MaxNumsCount = 5;

static const sal_Int32 kInt32MaxNums[kInt32MaxNumsCount] =
                        {
                            kSInt8Max,  kUInt8Max,
                            kSInt16Max, kUInt16Max,
                            kSInt32Max
                        };

static const sal_Int32 kInt64MaxNumsCount = 7;

static const sal_Int64 kInt64MaxNums[kInt64MaxNumsCount] =
                        {
                            kSInt8Max,  kUInt8Max,
                            kSInt16Max, kUInt16Max,
                            kSInt32Max, kUInt32Max,
                            kSInt64Max
                        };

//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const sal_Char *kSInt8MaxBinaryStr  = "1111111";
static const sal_Char *kUInt8MaxBinaryStr  = "11111111";
static const sal_Char *kSInt16MaxBinaryStr = "111111111111111";
static const sal_Char *kUInt16MaxBinaryStr = "1111111111111111";
static const sal_Char *kSInt32MaxBinaryStr =
                                        "1111111111111111111111111111111";
static const sal_Char *kUInt32MaxBinaryStr =
                                        "11111111111111111111111111111111";
static const sal_Char *kSInt64MaxBinaryStr =
            "111111111111111111111111111111111111111111111111111111111111111";

//------------------------------------------------------------------------

static const sal_Char *kSInt8MaxOctolStr  = "177";
static const sal_Char *kUInt8MaxOctolStr  = "377";
static const sal_Char *kSInt16MaxOctolStr = "77777";
static const sal_Char *kUInt16MaxOctolStr = "177777";
static const sal_Char *kSInt32MaxOctolStr = "17777777777";
static const sal_Char *kUInt32MaxOctolStr = "37777777777";
static const sal_Char *kSInt64MaxOctolStr = "777777777777777777777";

//------------------------------------------------------------------------

static const sal_Char *kSInt8MaxDecimalStr  = "127";
static const sal_Char *kUInt8MaxDecimalStr  = "255";
static const sal_Char *kSInt16MaxDecimalStr = "32767";
static const sal_Char *kUInt16MaxDecimalStr = "65535";
static const sal_Char *kSInt32MaxDecimalStr = "2147483647";
static const sal_Char *kUInt32MaxDecimalStr = "4294967295";
static const sal_Char *kSInt64MaxDecimalStr = "9223372036854775807";

//------------------------------------------------------------------------

static const sal_Char *kSInt8MaxHexDecimalStr  = "7f";
static const sal_Char *kUInt8MaxHexDecimalStr  = "ff";
static const sal_Char *kSInt16MaxHexDecimalStr = "7fff";
static const sal_Char *kUInt16MaxHexDecimalStr = "ffff";
static const sal_Char *kSInt32MaxHexDecimalStr = "7fffffff";
static const sal_Char *kUInt32MaxHexDecimalStr = "ffffffff";
static const sal_Char *kSInt64MaxHexDecimalStr = "7fffffffffffffff";

//------------------------------------------------------------------------

static const sal_Char *kSInt8MaxBase36Str  = "3j";
static const sal_Char *kUInt8MaxBase36Str  = "73";
static const sal_Char *kSInt16MaxBase36Str = "pa7";
static const sal_Char *kUInt16MaxBase36Str = "1ekf";
static const sal_Char *kSInt32MaxBase36Str = "zik0zj";
static const sal_Char *kUInt32MaxBase36Str = "1z141z3";
static const sal_Char *kSInt64MaxBase36Str = "1y2p0ij32e8e7";

//------------------------------------------------------------------------
//------------------------------------------------------------------------

static const sal_Int32 kBinaryNumsCount = 16;

static const sal_Char *kBinaryNumsStr[kBinaryNumsCount] =
                        {
                            "0",   "1",    "10",   "11",
                            "100",  "101",  "110",  "111",
                            "1000", "1001", "1010", "1011",
                            "1100", "1101", "1110", "1111"
                        };

static const sal_Int32 kBinaryMaxNumsCount = 7;

static const sal_Char *kBinaryMaxNumsStr[kBinaryMaxNumsCount] =
                        {
                            kSInt8MaxBinaryStr,
                            kUInt8MaxBinaryStr,
                            kSInt16MaxBinaryStr,
                            kUInt16MaxBinaryStr,
                            kSInt32MaxBinaryStr,
                            kUInt32MaxBinaryStr,
                            kSInt64MaxBinaryStr
                        };

//------------------------------------------------------------------------

static const sal_Int32 kOctolNumsCount = 16;

static const sal_Char *kOctolNumsStr[kOctolNumsCount] =
                        {
                            "0",  "1",  "2",  "3",
                            "4",  "5",  "6",  "7",
                            "10", "11", "12", "13",
                            "14", "15", "16", "17"
                        };

static const sal_Int32 kOctolMaxNumsCount = 7;

static const sal_Char *kOctolMaxNumsStr[kOctolMaxNumsCount] =
                        {
                            kSInt8MaxOctolStr,
                            kUInt8MaxOctolStr,
                            kSInt16MaxOctolStr,
                            kUInt16MaxOctolStr,
                            kSInt32MaxOctolStr,
                            kUInt32MaxOctolStr,
                            kSInt64MaxOctolStr
                        };

//------------------------------------------------------------------------

static const sal_Int32 kDecimalNumsCount = 16;

static const sal_Char *kDecimalNumsStr[kDecimalNumsCount] =
                        {
                            "0",  "1",  "2",  "3",
                            "4",  "5",  "6",  "7",
                            "8",  "9", "10", "11",
                            "12", "13", "14", "15"
                        };

static const sal_Int32 kDecimalMaxNumsCount = 7;

static const sal_Char *kDecimalMaxNumsStr[kDecimalMaxNumsCount] =
                        {
                            kSInt8MaxDecimalStr,
                            kUInt8MaxDecimalStr,
                            kSInt16MaxDecimalStr,
                            kUInt16MaxDecimalStr,
                            kSInt32MaxDecimalStr,
                            kUInt32MaxDecimalStr,
                            kSInt64MaxDecimalStr
                        };

//------------------------------------------------------------------------

static const sal_Int32 kHexDecimalNumsCount = 16;

static const sal_Char *kHexDecimalNumsStr[kHexDecimalNumsCount] =
                        {
                            "0", "1", "2", "3",
                            "4", "5", "6", "7",
                            "8", "9", "a", "b",
                            "c", "d", "e", "f"
                        };

static const sal_Int32 kHexDecimalMaxNumsCount = 7;

static const sal_Char *kHexDecimalMaxNumsStr[kHexDecimalMaxNumsCount] =
                        {
                            kSInt8MaxHexDecimalStr,
                            kUInt8MaxHexDecimalStr,
                            kSInt16MaxHexDecimalStr,
                            kUInt16MaxHexDecimalStr,
                            kSInt32MaxHexDecimalStr,
                            kUInt32MaxHexDecimalStr,
                            kSInt64MaxHexDecimalStr
                        };

//------------------------------------------------------------------------

static const sal_Int32 kBase36NumsCount = 36;

static const sal_Char *kBase36NumsStr[kBase36NumsCount] =
                        {
                            "0", "1", "2", "3",
                            "4", "5", "6", "7",
                            "8", "9", "a", "b",
                            "c", "d", "e", "f",
                            "g", "h", "i", "j",
                            "k", "l", "m", "n",
                            "o", "p", "q", "r",
                            "s", "t", "u", "v",
                            "w", "x", "y", "z"
                        };

static const sal_Int32 kBase36MaxNumsCount = 7;

static const sal_Char *kBase36MaxNumsStr[kBase36MaxNumsCount] =
                        {
                            kSInt8MaxBase36Str,
                            kUInt8MaxBase36Str,
                            kSInt16MaxBase36Str,
                            kUInt16MaxBase36Str,
                            kSInt32MaxBase36Str,
                            kUInt32MaxBase36Str,
                            kSInt64MaxBase36Str
                        };

//------------------------------------------------------------------------
//------------------------------------------------------------------------
static const sal_Int32 nDoubleCount=24;
static const sal_Char *inputDouble[nDoubleCount] =
    {
            "3","3.1","3.1415","3.1415926535","3.141592653589793",
            "3.1415926535897932","3.14159265358979323","+3.1",
            "3.141592653589793238462643","9.1096e-31","2.997925e8","6.241e18","5.381e+18",
            "1.7e-309","6.5822e-16","1.7e+307","2.2e30","03.1"," 3.1","-3.1",
            "-0.0","0.0","","1.00e308"
    };
static const double   expValDouble[nDoubleCount]=
    {
            3.0,3.1,3.1415,3.1415926535,3.141592653589793,
            3.1415926535897932,3.14159265358979323,3.1,
            3.141592653589793238462643,9.1096e-31,2.997925e8,6.241e18,5.381e18,
            1.7e-309,6.5822e-16,1.7e+307,2.2e30,3.1,3.1,-3.1,
            0.0,0.0,0.0,1.00e+308
    };
//------------------------------------------------------------------------
static const sal_Int32 nFloatCount=22;
static const sal_Char *inputFloat[nFloatCount] =
        {
            "3","3.1","3.1415","3.14159","3.141592",
            "3.1415926","3.14159265","3.141592653589793238462643",
            "6.5822e-16","9.1096e-31","2.997925e8","6.241e18",
            "1.00e38","6.241e-37","6.241e37","03.1"," 3.1","-3.1",
            "+3.1","-0.0","0.0",""
        };
static const float  expValFloat[nFloatCount] =
        {
            3.0f,3.1f,3.1415f,3.14159f,3.141592f,
            3.1415926f,3.14159265f,3.141592653589793238462643f,
            6.5822e-16f,9.1096e-31f,2.997925e8f,6.241e18f,
            1.00e38f,6.241e-37f,6.241e37f,3.1f,3.1f,-3.1f,
            3.1f,0.0f,0.0f,0.0f
        };
//------------------------------------------------------------------------
static const sal_Int32 nCharCount=15;
static const sal_Char *inputChar[nCharCount] =
        {
            "A","a","0","-","_",
            "\25","\33","\35",
            "@","\n","\'","\"",
            "\0","","Sun Microsystems"
        };
static const sal_Unicode  expValChar[nCharCount] =
        {
            65,97,48,45,95,
            21,27,29,
            64,10,39,34,
            0,0,83
        };
//------------------------------------------------------------------------
static const sal_Int32 nDefaultCount=6;
static const sal_Unicode input1Default[nDefaultCount] =
        {
            77,115,85,119,32,0
        };
static const sal_Int32 input2Default[nDefaultCount] =
        {
            0,0,0,0,0,0
        };
static const sal_Int32  expValDefault[nDefaultCount] =
        {
            4,9,-1,-1,3,-1
        };
//------------------------------------------------------------------------
static const sal_Int32 nNormalCount=10;
static const sal_Unicode input1Normal[nNormalCount] =
        {
            77,77,77,115,115,115,119,119,0,0
        };
static const sal_Int32 input2Normal[nNormalCount] =
        {
            0,32,80,0,13,20,0,80,0,32
        };
static const sal_Int32  expValNormal[nNormalCount] =
        {
            4,-1,-1,9,15,-1,-1,-1,-1,-1
        };
//------------------------------------------------------------------------
static const sal_Int32 nlastDefaultCount=5;
static const sal_Unicode input1lastDefault[nlastDefaultCount] =
        {
            77,115,119,32,0
        };
static const sal_Int32 input2lastDefault[nlastDefaultCount] =
        {
            31,31,31,31,31
        };
static const sal_Int32  expVallastDefault[nlastDefaultCount] =
        {
            4,15,-1,21,-1
        };
//------------------------------------------------------------------------
static const sal_Int32 nlastNormalCount=8;
static const sal_Unicode input1lastNormal[nlastNormalCount] =
        {
            77,77,77,115,115,119,119,0
        };
static const sal_Int32 input2lastNormal[nlastNormalCount] =
        {
            29,0,80,31,3,31,80,31
        };
static const sal_Int32  expVallastNormal[nlastNormalCount] =
        {
            4,-1,4,15,-1,-1,-1,-1
        };
//------------------------------------------------------------------------
static const sal_Int32 nStrDefaultCount=6;
static const sal_Unicode *input1StrDefault[nStrDefaultCount] =
        {
            aUStr7,aUStr8,aUStr21,
            aUStr30,aUStr25,aUStr26
        };
static const sal_Int32 input2StrDefault[nStrDefaultCount] =
        {
            0,0,0,0,0,0
        };
static const sal_Int32 expValStrDefault[nStrDefaultCount] =
        {
            0,4,-1,-1,-1,3
        };
//------------------------------------------------------------------------
static const sal_Int32 nStrNormalCount=9;
static const sal_Unicode *input1StrNormal[nStrNormalCount] =
        {
            aUStr7,aUStr7,aUStr8,aUStr8,aUStr21,aUStr30,aUStr25,aUStr25,aUStr26
        };
static const sal_Int32 input2StrNormal[nStrNormalCount] =
        {
            0,32,0,30,0,0,0,32,0
        };
static const sal_Int32 expValStrNormal[nStrNormalCount] =
        {
            0,-1,4,-1,-1,-1,-1,-1,3
        };
//------------------------------------------------------------------------
static const sal_Int32 nStrLastDefaultCount=6;
static const sal_Unicode *input1StrLastDefault[nStrLastDefaultCount] =
        {
            aUStr7,aUStr8,aUStr21,aUStr30,aUStr25,aUStr26
        };
static const sal_Int32 input2StrLastDefault[nStrLastDefaultCount] =
        {
            31,31,31,31,31,31
        };
static const sal_Int32  expValStrLastDefault[nStrLastDefaultCount] =
        {
            0,4,-1,-1,-1,3
        };
//------------------------------------------------------------------------
static const sal_Int32 nStrLastNormalCount=12;
static const sal_Unicode *input1StrLastNormal[nStrLastNormalCount] =
        {
            aUStr7,aUStr7,aUStr7,aUStr8,aUStr8,aUStr21,aUStr30,
            aUStr25,aUStr25,aUStr26,aUStr27,aUStr27
        };
static const sal_Int32 input2StrLastNormal[nStrLastNormalCount] =
        {
            31,0,80,31,2,31,31,31,0,31,31,14
        };
static const sal_Int32  expValStrLastNormal[nStrLastNormalCount] =
        {
            0,-1,0,4,-1,-1,-1,-1,-1,3,15,11
        };
//------------------------------------------------------------------------
static const sal_Int32 kNonSInt64Max = LONG_MIN;
static const sal_Int32 kNonSInt32Max = INT_MIN;
static const sal_Int32 kNonSInt16Max = SHRT_MIN;
//------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif /* _RTL_STRING_CONST_H_ */

