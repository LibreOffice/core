/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <unotools/configmgr.hxx>
#include <unotools/defaultencoding.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <officecfg/Setup.hxx>
#include <officecfg/System.hxx>

OUString utl_getLocaleForGlobalDefaultEncoding()
{
    if (comphelper::IsFuzzing())
        return u"en-US"_ustr;
    // First try document default language
    OUString result(officecfg::Office::Linguistic::General::DefaultLocale::get());
    // Fallback to LO locale
    if (result.isEmpty())
        result = officecfg::Setup::L10N::ooSetupSystemLocale::get();
    // Fallback to system locale
    if (result.isEmpty())
        result = officecfg::System::L10N::Locale::get();
    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
