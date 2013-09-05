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

#define TRANSLITERATION_MiddleDot_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreMiddleDot_ja_JP_translator (const sal_Unicode c)
{
    switch (c) {
        case 0x30FB: // KATAKANA MIDDLE DOT
        case 0xFF65: // HALFWIDTH KATAKANA MIDDLE DOT
        // no break;
        return 0xffff; // Skip this character
    }
    return c;
}

ignoreMiddleDot_ja_JP::ignoreMiddleDot_ja_JP()
{
    func = ignoreMiddleDot_ja_JP_translator;
    table = 0;
    map = 0;
    transliterationName = "ignoreMiddleDot_ja_JP";
    implementationName = "com.sun.star.i18n.Transliteration.ignoreMiddleDot_ja_JP";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
