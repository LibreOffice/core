/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_I18NPOOL_INC_NUMTOTEXT_CJK_HXX
#define INCLUDED_I18NPOOL_INC_NUMTOTEXT_CJK_HXX

#include <transliteration_Numeric.hxx>

namespace i18npool {

#define TRANSLITERATION_NUMTOTEXT_CJK( name ) \
class NumToText##name : public transliteration_Numeric \
{ \
public: \
        NumToText##name (); \
};

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

#undef TRANSLITERATION_NUMTOTEXT

}

#endif // INCLUDED_I18NPOOL_INC_NUMTOTEXT_CJK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
