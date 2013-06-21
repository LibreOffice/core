/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define TRANSLITERATION_Diacritics_CTL
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreDiacritics_CTL_translator (const sal_Unicode c)
{
    switch (c) {
        //Arabic Diacritics
        case 0x064B:
        case 0x064C:
        case 0x064D:
        case 0x064E:
        case 0x064F:
        case 0x0650:
        case 0x0651:
        case 0x0652:
        // no break;
        return 0xffff; // Skip this character
    }
    return c;
}

ignoreDiacritics_CTL::ignoreDiacritics_CTL()
{
    func = ignoreDiacritics_CTL_translator;
    table = 0;
    map = 0;
    transliterationName = "ignoreDiacritics_CTL";
    implementationName = "com.sun.star.i18n.Transliteration.ignoreDiacritics_CTL";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
