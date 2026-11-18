// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <cstring>

#import <i18nlangtag/languagetag.hxx>
#import <unotools/resmgr.hxx>

#import "AppDelegate.h"
#import "L10n.h"

char *app_translate(
                    TranslateId id,
                    const char *catalog)
{
    LanguageTag tag(OUString::fromUtf8([app_locale UTF8String]));
    std::locale locale = Translate::Create(catalog, tag);
    OUString result = Translate::get(id, locale);

    return strdup(result.toUtf8().getStr());
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
