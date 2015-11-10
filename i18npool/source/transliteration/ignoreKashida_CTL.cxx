/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreKashida_CTL_translator (const sal_Unicode c)
{
    if( c == 0x0640 ) // Check if it's Kashida
        return 0xffff; // Then skip this character

    return c;
}

ignoreKashida_CTL::ignoreKashida_CTL()
{
    func = ignoreKashida_CTL_translator;
    table = nullptr;
    map = nullptr;
    transliterationName = "ignoreKashida_CTL";
    implementationName = "com.sun.star.i18n.Transliteration.ignoreKashida_CTL";
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
