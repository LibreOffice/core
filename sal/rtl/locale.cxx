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

#include <stdlib.h>
#include <rtl/locale.h>

#include <osl/diagnose.h>

#include <rtllifecycle.h>
#include <unordered_map>


static std::unordered_map<sal_Int32, rtl_Locale*> g_pLocaleTable;

static rtl_Locale* g_pDefaultLocale = nullptr;

void rtl_locale_init()
{
}

void rtl_locale_fini()
{
    g_pLocaleTable.clear();
}

rtl_Locale * SAL_CALL rtl_locale_register(const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant)
{
    sal_Unicode c = 0;
    rtl_uString* sLanguage = nullptr;
    rtl_uString* sCountry = nullptr;
    rtl_uString* sVariant = nullptr;
    rtl_Locale *newLocale = nullptr;
    sal_Int32 hashCode = -1;

    if (!country)
        country = &c;

    if (!variant)
        variant = &c;

    ensureLocaleSingleton();

    hashCode = rtl_ustr_hashCode(language) ^ rtl_ustr_hashCode(country) ^ rtl_ustr_hashCode(variant);

    auto it = g_pLocaleTable.find(hashCode);
    if (it != g_pLocaleTable.end())
        return it->second;

    rtl_uString_newFromStr(&sLanguage, language);
    rtl_uString_newFromStr(&sCountry, country);
    rtl_uString_newFromStr(&sVariant, variant);

    newLocale = static_cast<rtl_Locale*>(malloc( sizeof(rtl_Locale) ));

    newLocale->Language = sLanguage;
    newLocale->Country = sCountry;
    newLocale->Variant = sVariant;
    newLocale->HashCode = hashCode;

    g_pLocaleTable.insert(it, { hashCode, newLocale });

    return newLocale;
}

rtl_Locale * SAL_CALL rtl_locale_getDefault()
{
    return g_pDefaultLocale;
}

void SAL_CALL rtl_locale_setDefault(const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant)
{
    g_pDefaultLocale = rtl_locale_register(language, country, variant);
}

rtl_uString * SAL_CALL rtl_locale_getLanguage(rtl_Locale * This)
{
    rtl_uString_acquire(This->Language);
    return This->Language;
}

rtl_uString * SAL_CALL rtl_locale_getCountry(rtl_Locale * This)
{
    rtl_uString_acquire(This->Country);
    return This->Country;
}

rtl_uString * SAL_CALL rtl_locale_getVariant(rtl_Locale * This)
{
    rtl_uString_acquire(This->Variant);
    return This->Variant;
}

sal_Int32 SAL_CALL rtl_locale_hashCode(rtl_Locale * This)
{
    return This->HashCode;
}

sal_Int32 SAL_CALL rtl_locale_equals(rtl_Locale * This, rtl_Locale * obj)
{
    return This == obj;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
