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


// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_TraditionalKana_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreTraditionalKana_ja_JP_translator (const sal_Unicode c)
{

    switch (c) {
        case 0x3090:     // HIRAGANA LETTER WI
            return 0x3044; // HIRAGANA LETTER I

        case 0x3091:     // HIRAGANA LETTER WE
            return 0x3048; // HIRAGANA LETTER E

        case 0x30F0:     // KATAKANA LETTER WI
            return 0x30A4; // KATAKANA LETTER I

        case 0x30F1:     // KATAKANA LETTER WE
            return 0x30A8; // KATAKANA LETTER E
    }
    return c;
}

ignoreTraditionalKana_ja_JP::ignoreTraditionalKana_ja_JP()
{
    func = ignoreTraditionalKana_ja_JP_translator;
    table = 0;
    map = 0;
    transliterationName = "ignoreTraditionalKana_ja_JP";
    implementationName = "com.sun.star.i18n.Transliteration.ignoreTraditionalKana_ja_JP";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
