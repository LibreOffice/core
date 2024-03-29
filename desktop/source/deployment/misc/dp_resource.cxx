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

#include <dp_resource.h>
#include <unotools/configmgr.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace ::com::sun::star;

namespace dp_misc
{
const LanguageTag& getOfficeLanguageTag()
{
    static const LanguageTag OFFICE_LANG = []() {
        OUString slang(utl::ConfigManager::getUILocale());
        //fallback, the locale is currently only set when the user starts the
        //office for the first time.
        if (slang.isEmpty())
            slang = "en-US";
        return LanguageTag(slang);
    }();
    return OFFICE_LANG;
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
