/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartonum.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:25:55 $
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

#define TRANSLITERATION_ALL
#include <chartonum.hxx>
#include <data/numberchar.h>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_CHARTONUM( name ) \
CharToNum##name::CharToNum##name() \
{ \
        nNativeNumberMode = 0; \
        tableSize = 0; \
        implementationName = "com.sun.star.i18n.Transliteration.CharToNum"#name; \
}
TRANSLITERATION_CHARTONUM( Fullwidth)
TRANSLITERATION_CHARTONUM( Lower_zh_CN)
TRANSLITERATION_CHARTONUM( Lower_zh_TW)
TRANSLITERATION_CHARTONUM( Upper_zh_CN)
TRANSLITERATION_CHARTONUM( Upper_zh_TW)
TRANSLITERATION_CHARTONUM( KanjiShort_ja_JP)
TRANSLITERATION_CHARTONUM( KanjiTraditional_ja_JP)
TRANSLITERATION_CHARTONUM( Lower_ko)
TRANSLITERATION_CHARTONUM( Upper_ko)
TRANSLITERATION_CHARTONUM( Hangul_ko)
TRANSLITERATION_CHARTONUM( Indic_ar)
TRANSLITERATION_CHARTONUM( EastIndic_ar)
TRANSLITERATION_CHARTONUM( Indic_hi)
TRANSLITERATION_CHARTONUM( _th)
#undef TRANSLITERATION_CHARTONUM

} } } }
