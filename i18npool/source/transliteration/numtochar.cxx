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

#include <numtochar.hxx>

namespace i18npool {

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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
