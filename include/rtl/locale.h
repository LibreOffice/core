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

#ifndef INCLUDED_RTL_LOCALE_H
#define INCLUDED_RTL_LOCALE_H

#include "sal/config.h"

#include "rtl/ustring.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#   pragma pack(push, 8)
#endif

/**
    The implementation structure of a locale. Do not create this structure
    direct. Only use the functions rtl_locale_register and
    rtl_locale_setDefault. The strings Language, Country and Variant
    are constants, so it is not necessary to acquire and release them.
 */
typedef struct _rtl_Locale
{
    /**
         Lowercase two-letter ISO 639-1 or three-letter ISO 639-3 code.
     */
    rtl_uString *   Language;
    /**
         uppercase two-letter ISO-3166 code.
     */
    rtl_uString *   Country;
    /**
         Lowercase vendor and browser specific code.
     */
    rtl_uString *   Variant;
    /**
         The merged hash value of the Language, Country and Variant strings.
     */
    sal_Int32       HashCode;
} rtl_Locale;

#if defined( _WIN32)
#pragma pack(pop)
#endif

/**
    Register a locale from language, country and variant.
    @param language lowercase two-letter ISO 639-1 or three-letter ISO 639-3 code.
    @param country uppercase two-letter ISO-3166 code. May be null.
    @param variant vendor and browser specific code. May be null.
 */
SAL_DLLPUBLIC rtl_Locale * SAL_CALL rtl_locale_register(
        const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant );

/**
    Common method of getting the current default Locale.

    @deprecated  LibreOffice itself does not use this anymore, and client code
    should not have good use for it either.  It may eventually be removed.
 */
SAL_DLLPUBLIC rtl_Locale * SAL_CALL rtl_locale_getDefault(void);

/**
    Sets the default.

    <code>setDefault</code> does not reset the host locale.

    @param language lowercase two-letter ISO 639-1 or three-letter ISO 639-3 code.
    @param country uppercase two-letter ISO-3166 code.
    @param variant vendor and browser specific code. See class description.

    @deprecated  LibreOffice itself does not use this anymore, and client code
    should not have good use for it either.  It may eventually be removed.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_locale_setDefault(
        const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant );

/**
    Getter for programmatic name of field,
    a lowercase two-letter ISO 639-1 or three-letter ISO 639-3 code.
    @see getDisplayLanguage
 */
SAL_DLLPUBLIC rtl_uString * SAL_CALL rtl_locale_getLanguage( rtl_Locale * This );

/**
    Getter for programmatic name of field,
    an uppercased two-letter ISO-3166 code.
    @see getDisplayCountry
 */
SAL_DLLPUBLIC rtl_uString * SAL_CALL rtl_locale_getCountry( rtl_Locale * This );

/**
    Getter for programmatic name of field.
    @see getDisplayVariant
 */
SAL_DLLPUBLIC rtl_uString * SAL_CALL rtl_locale_getVariant( rtl_Locale * This );

/**
    Returns the hash code of the locale This.
 */
SAL_DLLPUBLIC sal_Int32 SAL_CALL rtl_locale_hashCode( rtl_Locale * This );

/**
    Returns true if the locals are equal, otherwise false.
 */
SAL_DLLPUBLIC sal_Int32 SAL_CALL rtl_locale_equals( rtl_Locale * This, rtl_Locale * obj  );

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_RTL_LOCALE_H


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
