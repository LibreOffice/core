/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chartonum.hxx,v $
 * $Revision: 1.5 $
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
