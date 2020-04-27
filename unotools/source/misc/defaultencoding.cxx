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

OUString utl_getLocaleForGlobalDefaultEncoding()
{
    if (utl::ConfigManager::IsFuzzing())
        return "en-US";
    OUString result(officecfg::Office::Linguistic::General::DefaultLocale::get());
    if (result.isEmpty())
        result = officecfg::Setup::L10N::ooSetupSystemLocale::get();
    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
