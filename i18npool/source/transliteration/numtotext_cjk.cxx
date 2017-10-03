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

#include <com/sun/star/i18n/NativeNumberMode.hpp>

#include <numtotext_cjk.hxx>
#include <bullet.h>

namespace i18npool {

#define TRANSLITERATION_NUMTOTEXT( name, number ) \
NumToText##name::NumToText##name() \
{ \
        nNativeNumberMode = number; \
        tableSize = 0; \
        transliterationName = "NumToText"#name; \
        implementationName = "com.sun.star.i18n.Transliteration.NumToText"#name; \
}
using namespace com::sun::star::i18n::NativeNumberMode;

TRANSLITERATION_NUMTOTEXT( Lower_zh_CN, NATNUM4)
TRANSLITERATION_NUMTOTEXT( Upper_zh_CN, NATNUM5)
TRANSLITERATION_NUMTOTEXT( Lower_zh_TW, NATNUM4)
TRANSLITERATION_NUMTOTEXT( Upper_zh_TW, NATNUM5)
TRANSLITERATION_NUMTOTEXT( Fullwidth_zh_CN, NATNUM6)
TRANSLITERATION_NUMTOTEXT( Fullwidth_zh_TW, NATNUM6)
TRANSLITERATION_NUMTOTEXT( Fullwidth_ja_JP, NATNUM6)
TRANSLITERATION_NUMTOTEXT( Fullwidth_ko, NATNUM6)
TRANSLITERATION_NUMTOTEXT( FormalLower_ko, NATNUM4)
TRANSLITERATION_NUMTOTEXT( FormalUpper_ko, NATNUM5)
TRANSLITERATION_NUMTOTEXT( FormalHangul_ko, NATNUM10)
TRANSLITERATION_NUMTOTEXT( InformalLower_ko, NATNUM7)
TRANSLITERATION_NUMTOTEXT( InformalUpper_ko, NATNUM8)
TRANSLITERATION_NUMTOTEXT( InformalHangul_ko, NATNUM11)
TRANSLITERATION_NUMTOTEXT( KanjiLongTraditional_ja_JP, NATNUM5)
TRANSLITERATION_NUMTOTEXT( KanjiLongModern_ja_JP, NATNUM4)
TRANSLITERATION_NUMTOTEXT( Date_zh, NATNUM7)
TRANSLITERATION_NUMTOTEXT( KanjiShortTraditional_ja_JP, NATNUM8)
TRANSLITERATION_NUMTOTEXT( KanjiShortModern_ja_JP, NATNUM7)

#undef TRANSLITERATION_NUMTOTEXT

#define TRANSLITERATION_NUMTOTEXT( name, _table, recycle ) \
NumToText##name::NumToText##name() \
{ \
        table = _table;\
        tableSize = SAL_N_ELEMENTS(_table); \
        recycleSymbol = recycle; \
        transliterationName = "NumToText"#name; \
        implementationName = "com.sun.star.i18n.Transliteration.NumToText"#name; \
}

TRANSLITERATION_NUMTOTEXT ( AIUFullWidth_ja_JP, table_AIUFullWidth_ja_JP, true)
TRANSLITERATION_NUMTOTEXT ( AIUHalfWidth_ja_JP, table_AIUHalfWidth_ja_JP, true)
TRANSLITERATION_NUMTOTEXT ( IROHAFullWidth_ja_JP, table_IROHAFullWidth_ja_JP, true)
TRANSLITERATION_NUMTOTEXT ( IROHAHalfWidth_ja_JP, table_IROHAHalfWidth_ja_JP, true)
TRANSLITERATION_NUMTOTEXT ( CircledNumber, table_CircledNumber, false)
TRANSLITERATION_NUMTOTEXT ( TianGan_zh, table_TianGan_zh, false)
TRANSLITERATION_NUMTOTEXT ( DiZi_zh, table_DiZi_zh, false)
TRANSLITERATION_NUMTOTEXT ( HangulJamo_ko, table_HangulJamo_ko, true)
TRANSLITERATION_NUMTOTEXT ( HangulSyllable_ko, table_HangulSyllable_ko, true)
TRANSLITERATION_NUMTOTEXT ( HangulCircledJamo_ko, table_HangulCircledJamo_ko, true)
TRANSLITERATION_NUMTOTEXT ( HangulCircledSyllable_ko, table_HangulCircledSyllable_ko, true)

#undef TRANSLITERATION_NUMTOTEXT

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
