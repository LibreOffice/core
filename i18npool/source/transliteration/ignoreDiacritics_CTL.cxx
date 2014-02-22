/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#define TRANSLITERATION_Diacritics_CTL
#include <transliteration_Ignore.hxx>
#include <unicode/uchar.h>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreDiacritics_CTL_translator (const sal_Unicode c)
{
    if(u_getIntPropertyValue(c, UCHAR_GENERAL_CATEGORY) == U_NON_SPACING_MARK)
        return 0xffff; 

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
