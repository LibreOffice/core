/*************************************************************************
 *
 *  $RCSfile: numtochar.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 10:54:50 $
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

// prevent internal compiler error with MSVC6SP3
#include <stl/utility>

#define TRANSLITERATION_ALL
#include <numtochar.hxx>
#include <data/numberchar.h>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL NumToChar::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
    const sal_Unicode *src = inStr.getStr() + startPos;
    rtl_uString *newStr = x_rtl_uString_new_WithLength(nCount);
    offset.realloc(nCount);

    for (sal_Int32 i = 0; i < nCount; i++) {
        sal_Unicode ch = src[i];
        newStr->buffer[i] = (isNumber(ch) ? NumberChar[number][ ch - NUMBER_ZERO ] :
            (isDecimal(ch) ? DecimalChar[number] : (isMinus(ch) ? MinusChar[number] : ch)));
        offset[i] = startPos + i;
    }
    return OUString(newStr->buffer, nCount);
}

#define TRANSLITERATION_NUMTOCHAR( name, _number ) \
NumToChar##name::NumToChar##name() \
{ \
    number = NumberChar_##_number; \
    transliterationName = "NumToChar"#name; \
    implementationName = "com.sun.star.i18n.Transliteration.NumToChar"#name; \
}

TRANSLITERATION_NUMTOCHAR( Halfwidth, HalfWidth )
TRANSLITERATION_NUMTOCHAR( Fullwidth, FullWidth )
TRANSLITERATION_NUMTOCHAR( Lower_zh_CN, Lower_zh )
TRANSLITERATION_NUMTOCHAR( Lower_zh_TW, Lower_zh )
TRANSLITERATION_NUMTOCHAR( Upper_zh_CN, Upper_zh )
TRANSLITERATION_NUMTOCHAR( Upper_zh_TW, Upper_zh_TW )
TRANSLITERATION_NUMTOCHAR( KanjiShort_ja_JP, Modern_ja )
TRANSLITERATION_NUMTOCHAR( KanjiTraditional_ja_JP, Traditional_ja )
TRANSLITERATION_NUMTOCHAR( Lower_ko, Lower_ko )
TRANSLITERATION_NUMTOCHAR( Upper_ko, Upper_ko )
TRANSLITERATION_NUMTOCHAR( Hangul_ko, Hangul_ko )
TRANSLITERATION_NUMTOCHAR( Indic_ar, Indic_ar )
TRANSLITERATION_NUMTOCHAR( EastIndic_ar, EastIndic_ar )
TRANSLITERATION_NUMTOCHAR( Indic_hi, Indic_hi )
TRANSLITERATION_NUMTOCHAR( _th, th )
#undef TRANSLITERATION_NUMTOCHAR

} } } }
