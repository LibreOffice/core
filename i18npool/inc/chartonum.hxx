/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartonum.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:50:50 $
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
#ifndef _L10N_TRANSLITERATION_CHARTONUM_HXX_
#define _L10N_TRANSLITERATION_CHARTONUM_HXX_

#include <transliteration_Numeric.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_CHARTONUM( name ) \
class CharToNum##name : public transliteration_Numeric \
{ \
public: \
        CharToNum##name (); \
};

#ifdef TRANSLITERATION_ALL
TRANSLITERATION_CHARTONUM(Lower_zh_CN)
TRANSLITERATION_CHARTONUM(Upper_zh_CN)
TRANSLITERATION_CHARTONUM(Lower_zh_TW)
TRANSLITERATION_CHARTONUM(Upper_zh_TW)
TRANSLITERATION_CHARTONUM(Upper_ko)
TRANSLITERATION_CHARTONUM(Hangul_ko)
TRANSLITERATION_CHARTONUM(Lower_ko)
TRANSLITERATION_CHARTONUM(KanjiShort_ja_JP)
TRANSLITERATION_CHARTONUM(KanjiTraditional_ja_JP)
TRANSLITERATION_CHARTONUM(Fullwidth)
TRANSLITERATION_CHARTONUM(Indic_ar)
TRANSLITERATION_CHARTONUM(EastIndic_ar)
TRANSLITERATION_CHARTONUM(Indic_hi)
TRANSLITERATION_CHARTONUM(_th)
#endif
#undef TRANSLITERATION_CHARTONUM

} } } }

#endif // _L10N_TRANSLITERATION_CHARTONUM_HXX_
