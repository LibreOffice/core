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

#include <chartonum.hxx>

namespace i18npool {

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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
