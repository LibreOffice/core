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

#include <texttonum.hxx>

using namespace com::sun::star::uno;

namespace i18npool {

#define TRANSLITERATION_TEXTTONUM( name ) \
TextToNum##name::TextToNum##name() \
{ \
        nNativeNumberMode = 0; \
        tableSize = 0; \
        transliterationName = "TextToNum"#name; \
        implementationName = "com.sun.star.i18n.Transliteration.TextToNum"#name; \
}

TRANSLITERATION_TEXTTONUM( Lower_zh_CN)
TRANSLITERATION_TEXTTONUM( Upper_zh_CN)
TRANSLITERATION_TEXTTONUM( Lower_zh_TW)
TRANSLITERATION_TEXTTONUM( Upper_zh_TW)
TRANSLITERATION_TEXTTONUM( FormalLower_ko)
TRANSLITERATION_TEXTTONUM( FormalUpper_ko)
TRANSLITERATION_TEXTTONUM( FormalHangul_ko)
TRANSLITERATION_TEXTTONUM( InformalLower_ko)
TRANSLITERATION_TEXTTONUM( InformalUpper_ko)
TRANSLITERATION_TEXTTONUM( InformalHangul_ko)
TRANSLITERATION_TEXTTONUM( KanjiLongTraditional_ja_JP)
TRANSLITERATION_TEXTTONUM( KanjiLongModern_ja_JP)

#undef TRANSLITERATION_TEXTTONUM

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
