/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: texttonum.hxx,v $
 * $Revision: 1.4 $
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
#ifndef _L10N_TRANSLITERATION_TEXTTONUM_HXX_
#define _L10N_TRANSLITERATION_TEXTTONUM_HXX_

#include <transliteration_Numeric.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_TEXTTONUM( name ) \
class TextToNum##name : public transliteration_Numeric \
{ \
public: \
        TextToNum##name (); \
};

#ifdef TRANSLITERATION_ALL
TRANSLITERATION_TEXTTONUM(Lower_zh_CN)
TRANSLITERATION_TEXTTONUM(Upper_zh_CN)
TRANSLITERATION_TEXTTONUM(Lower_zh_TW)
TRANSLITERATION_TEXTTONUM(Upper_zh_TW)
TRANSLITERATION_TEXTTONUM(InformalLower_ko)
TRANSLITERATION_TEXTTONUM(InformalUpper_ko)
TRANSLITERATION_TEXTTONUM(InformalHangul_ko)
TRANSLITERATION_TEXTTONUM(FormalLower_ko)
TRANSLITERATION_TEXTTONUM(FormalUpper_ko)
TRANSLITERATION_TEXTTONUM(FormalHangul_ko)
TRANSLITERATION_TEXTTONUM(KanjiLongModern_ja_JP)
TRANSLITERATION_TEXTTONUM(KanjiLongTraditional_ja_JP)
#endif
#undef TRANSLITERATION_TEXTTONUM

} } } }

#endif // _L10N_TRANSLITERATION_TEXTTONUM_HXX_
