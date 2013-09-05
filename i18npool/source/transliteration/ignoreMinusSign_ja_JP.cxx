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

#define TRANSLITERATION_MinusSign_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreMinusSign_ja_JP_translator (const sal_Unicode c)
{
    switch (c) {
        case 0x2212: // MINUS SIGN
        case 0x002d: // HYPHEN-MINUS
        case 0x2010: // HYPHEN
        case 0x2011: // NON-BREAKING HYPHEN
        case 0x2012: // FIGURE DASH
        case 0x2013: // EN DASH
        case 0x2014: // EM DASH
        case 0x2015: // HORIZONTAL BAR
        case 0xff0d: // FULLWIDTH HYPHEN-MINUS
        case 0xff70: // HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK
        return 0x30fc; // KATAKANA-HIRAGANA PROLONGED SOUND MARK
    }
    return c;
}

ignoreMinusSign_ja_JP::ignoreMinusSign_ja_JP()
{
    func = ignoreMinusSign_ja_JP_translator;
    table = 0;
    map = 0;
    transliterationName = "ignoreMinusSign_ja_JP";
    implementationName = "com.sun.star.i18n.Transliteration.ignoreMinusSign_ja_JP";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
