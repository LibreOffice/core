/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _RTL_LOCALE_H_
#define _RTL_LOCALE_H_

#include "sal/config.h"

#include "rtl/ustring.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SAL_W32
#   pragma pack(push, 8)
#endif

/**
    The implementation structur of a locale. Do not create this structure
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

#if defined( SAL_W32)
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
    Used for the presentation: menus, dialogs, etc.
    Generally set once when your applet or application is initialized,
    then never reset. (If you do reset the default locale, you
    probably want to reload your GUI, so that the change is reflected
    in your interface.)
    <p>More advanced programs will allow users to use different locales
    for different fields, e.g. in a spreadsheet.
    <BR>Note that the initial setting will match the host system.
 */
SAL_DLLPUBLIC rtl_Locale * SAL_CALL rtl_locale_getDefault();

/**
    Sets the default.
    Normally set once at the beginning of applet or application,
    then never reset. <code>setDefault</code> does not reset the host locale.
    @param language lowercase two-letter ISO 639-1 or three-letter ISO 639-3 code.
    @param country uppercase two-letter ISO-3166 code.
    @param variant vendor and browser specific code. See class description.
 */
SAL_DLLPUBLIC void SAL_CALL rtl_locale_setDefault(
        const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant );

/**
    Getter for programmatic name of field,
    a lowercased two-letter ISO 639-1 or three-letter ISO 639-3 code.
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
    Returns true if the locals are equal, otherwis false.
 */
SAL_DLLPUBLIC sal_Int32 SAL_CALL rtl_locale_equals( rtl_Locale * This, rtl_Locale * obj  );

#ifdef __cplusplus
}
#endif

#endif /* _RTL_LOCALE_H_ */


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
