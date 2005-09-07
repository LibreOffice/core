/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numtochar.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:33:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_ALL
#include <numtochar.hxx>
#include <com/sun/star/i18n/NativeNumberMode.hpp>

namespace com { namespace sun { namespace star { namespace i18n {


#define TRANSLITERATION_NUMTOCHAR( name, number ) \
NumToChar##name::NumToChar##name() \
{ \
        nNativeNumberMode = number; \
        tableSize = 0; \
        transliterationName = "NumToChar"#name; \
        implementationName = "com.sun.star.i18n.Transliteration.NumToChar"#name; \
}

using namespace com::sun::star::i18n::NativeNumberMode;

TRANSLITERATION_NUMTOCHAR( Halfwidth, NATNUM0 )
TRANSLITERATION_NUMTOCHAR( Fullwidth, NATNUM3 )
TRANSLITERATION_NUMTOCHAR( Lower_zh_CN, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Lower_zh_TW, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Upper_zh_CN, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( Upper_zh_TW, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( KanjiShort_ja_JP, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( KanjiTraditional_ja_JP, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( Lower_ko, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Upper_ko, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( Hangul_ko, NATNUM9 )
TRANSLITERATION_NUMTOCHAR( Indic_ar, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( EastIndic_ar, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Indic_hi, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( _th, NATNUM1 )
#undef TRANSLITERATION_NUMTOCHAR

} } } }
