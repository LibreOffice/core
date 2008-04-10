/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: numtotext_cjk.hxx,v $
 * $Revision: 1.11 $
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
#ifndef _L10N_TRANSLITERATION_NUMTOTEXT_CJK_HXX_
#define _L10N_TRANSLITERATION_NUMTOTEXT_CJK_HXX_

#include <transliteration_Numeric.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_NUMTOTEXT_CJK( name ) \
class NumToText##name : public transliteration_Numeric \
{ \
public: \
        NumToText##name (); \
};

#ifdef TRANSLITERATION_ALL
TRANSLITERATION_NUMTOTEXT_CJK ( Lower_zh_CN )
TRANSLITERATION_NUMTOTEXT_CJK ( Upper_zh_CN )
TRANSLITERATION_NUMTOTEXT_CJK ( Lower_zh_TW )
TRANSLITERATION_NUMTOTEXT_CJK ( Upper_zh_TW )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_zh_CN )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_zh_TW )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( Date_zh )
TRANSLITERATION_NUMTOTEXT_CJK ( InformalLower_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( InformalUpper_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( InformalHangul_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( FormalLower_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( FormalUpper_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( FormalHangul_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiLongModern_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiLongTraditional_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiShortModern_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiShortTraditional_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( AIUFullWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( AIUHalfWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( IROHAFullWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( IROHAHalfWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( CircledNumber )
TRANSLITERATION_NUMTOTEXT_CJK ( TianGan_zh )
TRANSLITERATION_NUMTOTEXT_CJK ( DiZi_zh )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulJamo_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulSyllable_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulCircledJamo_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulCircledSyllable_ko )
#endif
#undef TRANSLITERATION_NUMTOTEXT

} } } }

#endif // _L10N_TRANSLITERATION_NUMTOTEXT_HXX_
