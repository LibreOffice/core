/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ignoreTraditionalKana_ja_JP.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

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
