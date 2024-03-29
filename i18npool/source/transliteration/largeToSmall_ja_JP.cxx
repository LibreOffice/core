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

#include <i18nutil/oneToOneMapping.hxx>

#include <transliteration_OneToOne.hxx>

namespace i18npool
{
// ftp://ftp.unicode.org/Public/3.0-Update/UnicodeData-3.0.0.txt
// ftp://ftp.unicode.org/Public/3.0-Update/UnicodeData-3.0.0.html
// http://charts.unicode.org/Web/U3040.html Hiragana (U+3040..U+309F)
// http://charts.unicode.org/Web/U30A0.html Katakana (U+30A0..U+30FF)
// http://charts.unicode.org/Web/UFF00.html

i18nutil::OneToOneMappingTable_t const large2small[] = {
    { 0x3041, 0x3042 }, // HIRAGANA LETTER SMALL A --> HIRAGANA LETTER A
    { 0x3043, 0x3044 }, // HIRAGANA LETTER SMALL I --> HIRAGANA LETTER I
    { 0x3045, 0x3046 }, // HIRAGANA LETTER SMALL U --> HIRAGANA LETTER U
    { 0x3047, 0x3048 }, // HIRAGANA LETTER SMALL E --> HIRAGANA LETTER E
    { 0x3049, 0x304A }, // HIRAGANA LETTER SMALL O --> HIRAGANA LETTER O
    { 0x3063, 0x3064 }, // HIRAGANA LETTER SMALL TU --> HIRAGANA LETTER TU
    { 0x3083, 0x3084 }, // HIRAGANA LETTER SMALL YA --> HIRAGANA LETTER YA
    { 0x3085, 0x3086 }, // HIRAGANA LETTER SMALL YU --> HIRAGANA LETTER YU
    { 0x3087, 0x3088 }, // HIRAGANA LETTER SMALL YO --> HIRAGANA LETTER YO
    { 0x308E, 0x308F }, // HIRAGANA LETTER SMALL WA --> HIRAGANA LETTER WA
    { 0x30A1, 0x30A2 }, // KATAKANA LETTER SMALL A --> KATAKANA LETTER A
    { 0x30A3, 0x30A4 }, // KATAKANA LETTER SMALL I --> KATAKANA LETTER I
    { 0x30A5, 0x30A6 }, // KATAKANA LETTER SMALL U --> KATAKANA LETTER U
    { 0x30A7, 0x30A8 }, // KATAKANA LETTER SMALL E --> KATAKANA LETTER E
    { 0x30A9, 0x30AA }, // KATAKANA LETTER SMALL O --> KATAKANA LETTER O
    { 0x30C3, 0x30C4 }, // KATAKANA LETTER SMALL TU --> KATAKANA LETTER TU
    { 0x30E3, 0x30E4 }, // KATAKANA LETTER SMALL YA --> KATAKANA LETTER YA
    { 0x30E5, 0x30E6 }, // KATAKANA LETTER SMALL YU --> KATAKANA LETTER YU
    { 0x30E7, 0x30E8 }, // KATAKANA LETTER SMALL YO --> KATAKANA LETTER YO
    { 0x30EE, 0x30EF }, // KATAKANA LETTER SMALL WA --> KATAKANA LETTER WA
    { 0x30F5, 0x30AB }, // KATAKANA LETTER SMALL KA --> KATAKANA LETTER KA
    { 0x30F6, 0x30B1 }, // KATAKANA LETTER SMALL KE --> KATAKANA LETTER KE
    { 0xFF67, 0xFF71 }, // HALFWIDTH KATAKANA LETTER SMALL A --> HALFWIDTH KATAKANA LETTER A
    { 0xFF68, 0xFF72 }, // HALFWIDTH KATAKANA LETTER SMALL I --> HALFWIDTH KATAKANA LETTER I
    { 0xFF69, 0xFF73 }, // HALFWIDTH KATAKANA LETTER SMALL U --> HALFWIDTH KATAKANA LETTER U
    { 0xFF6A, 0xFF74 }, // HALFWIDTH KATAKANA LETTER SMALL E --> HALFWIDTH KATAKANA LETTER E
    { 0xFF6B, 0xFF75 }, // HALFWIDTH KATAKANA LETTER SMALL O --> HALFWIDTH KATAKANA LETTER O
    { 0xFF6C, 0xFF94 }, // HALFWIDTH KATAKANA LETTER SMALL YA --> HALFWIDTH KATAKANA LETTER YA
    { 0xFF6D, 0xFF95 }, // HALFWIDTH KATAKANA LETTER SMALL YU --> HALFWIDTH KATAKANA LETTER YU
    { 0xFF6E, 0xFF96 }, // HALFWIDTH KATAKANA LETTER SMALL YO --> HALFWIDTH KATAKANA LETTER YO
    { 0xFF6F, 0xFF82 } // HALFWIDTH KATAKANA LETTER SMALL TU --> HALFWIDTH KATAKANA LETTER TU
};

largeToSmall_ja_JP::largeToSmall_ja_JP()
{
    static i18nutil::oneToOneMapping _table(large2small, sizeof(large2small));
    func = nullptr;
    table = &_table;
    transliterationName = "largeToSmall_ja_JP";
    implementationName = "com.sun.star.i18n.Transliteration.largeToSmall_ja_JP";
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
