/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ignoreZiZu_ja_JP.cxx,v $
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

#define TRANSLITERATION_ZiZu_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreZiZu_ja_JP_translator (const sal_Unicode c)
{

    switch (c) {
        case 0x30C2:     // KATAKANA LETTER DI
            return 0x30B8; // KATAKANA LETTER ZI

        case 0x3062:     // HIRAGANA LETTER DI
            return 0x3058; // HIRAGANA LETTER ZI

        case 0x30C5:     // KATAKANA LETTER DU
            return 0x30BA; // KATAKANA LETTER ZU

        case 0x3065:     // HIRAGANA LETTER DU
            return 0x305A; // HIRAGANA LETTER ZU
    }
    return c;
}

ignoreZiZu_ja_JP::ignoreZiZu_ja_JP()
{
        func = ignoreZiZu_ja_JP_translator;
        table = 0;
        map = 0;
        transliterationName = "ignoreZiZu_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreZiZu_ja_JP";
}

} } } }
