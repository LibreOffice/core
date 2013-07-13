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

#include <stdlib.h>  // for getenv()
#include <stdio.h>

#ifdef MACOSX
#include <osl/process.h>
#include <rtl/locale.h>
#include <rtl/ustring.hxx>

#else   // MACOSX
#include <rtl/string.hxx>

#endif  // MACOSX
#include <rtl/instance.hxx>
#include "i18nlangtag/languagetag.hxx"
#include "i18nlangtag/mslangid.hxx"

// =======================================================================

static LanguageType nImplSystemLanguage = LANGUAGE_DONTKNOW;
static LanguageType nImplSystemUILanguage = LANGUAGE_DONTKNOW;

// -----------------------------------------------------------------------

// Get locale of category LC_CTYPE of environment variables
static const sal_Char* getLangFromEnvironment()
{
    static const sal_Char* pFallback = "C";
    const sal_Char *pLang = NULL;

    pLang = getenv ( "LC_ALL" );
    if (! pLang || pLang[0] == 0)
        pLang = getenv ( "LC_CTYPE" );
    if (! pLang || pLang[0] == 0)
        pLang = getenv( "LANG" );
    if (! pLang || pLang[0] == 0)
        pLang = pFallback;

    return pLang;
}

// -----------------------------------------------------------------------

// Get locale of category LC_MESSAGES of environment variables
static const sal_Char* getUILangFromEnvironment()
{
    static const sal_Char* pFallback = "C";
    const sal_Char *pLang = NULL;

    pLang = getenv ( "LANGUAGE" );      // respect the GNU extension
    if (! pLang || pLang[0] == 0)
        pLang = getenv ( "LC_ALL" );
    if (! pLang || pLang[0] == 0)
        pLang = getenv ( "LC_MESSAGES" );
    if (! pLang || pLang[0] == 0)
        pLang = getenv( "LANG" );
    if (! pLang || pLang[0] == 0)
        pLang = pFallback;

    return pLang;
}

// -----------------------------------------------------------------------

typedef const sal_Char * (*getLangFromEnv)();

static void getPlatformSystemLanguageImpl( LanguageType& rSystemLanguage,
        getLangFromEnv pGetLangFromEnv )
{
    /* get the language from the user environment */
    LanguageType nLang = rSystemLanguage;
    if ( nLang == LANGUAGE_DONTKNOW )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex());
        nLang = rSystemLanguage;
        if ( nLang == LANGUAGE_DONTKNOW )
        {
#ifdef MACOSX
            rtl_Locale    *procLocale;
            (void) pGetLangFromEnv; /* unused */

            if ( osl_getProcessLocale(&procLocale) == osl_Process_E_None )
            {
                nLang = LanguageTag::convertToLanguageType( *procLocale );
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                rSystemLanguage = nLang;
#ifdef DEBUG
                if ( rSystemLanguage == LANGUAGE_DONTKNOW )
                    fprintf( stderr, "intnunx.cxx:  failed to convert osl_getProcessLocale() language to system language.\n" );
#endif
            }
#else   /* MACOSX */
            OString aUnxLang( (pGetLangFromEnv)() );
            nLang = MsLangId::convertUnxByteStringToLanguage( aUnxLang );
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            rSystemLanguage = nLang;
#endif  /* MACOSX */
        }
        else {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        }
    }
}

// -----------------------------------------------------------------------

LanguageType MsLangId::getPlatformSystemLanguage()
{
    getPlatformSystemLanguageImpl( nImplSystemLanguage, &getLangFromEnvironment);
    return nImplSystemLanguage;
}

// -----------------------------------------------------------------------

LanguageType MsLangId::getPlatformSystemUILanguage()
{
    getPlatformSystemLanguageImpl( nImplSystemUILanguage, &getUILangFromEnvironment);
    return nImplSystemUILanguage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
