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

#ifdef MACOSX
#include <osl/process.h>
#include <rtl/locale.h>
#include <rtl/ustring.hxx>
#include <i18nlangtag/languagetag.hxx>

#else   // MACOSX
#include <rtl/string.hxx>
#endif  // MACOSX

#include <osl/mutex.hxx>
#include <osl/doublecheckedlocking.h>
#include <i18nlangtag/mslangid.hxx>


static LanguageType nImplSystemLanguage = LANGUAGE_DONTKNOW;
static LanguageType nImplSystemUILanguage = LANGUAGE_DONTKNOW;


// Get locale of category LC_CTYPE of environment variables
static const char* getLangFromEnvironment( bool& rbColonList )
{
    static const char* const pFallback = "C";
    const char *pLang = nullptr;

    rbColonList = false;
    pLang = getenv ( "LC_ALL" );
    if (! pLang || pLang[0] == 0)
        pLang = getenv ( "LC_CTYPE" );
    if (! pLang || pLang[0] == 0)
        pLang = getenv( "LANG" );
    if (! pLang || pLang[0] == 0)
        pLang = pFallback;

    return pLang;
}


// Get locale of category LC_MESSAGES of environment variables
static const char* getUILangFromEnvironment( bool& rbColonList )
{
    static const char* const pFallback = "C";
    const char *pLang = nullptr;

    rbColonList = true;
    pLang = getenv ( "LANGUAGE" );      // respect the GNU extension
    if (! pLang || pLang[0] == 0)
    {
        rbColonList = false;
        pLang = getenv ( "LC_ALL" );
    }
    if (! pLang || pLang[0] == 0)
        pLang = getenv ( "LC_MESSAGES" );
    if (! pLang || pLang[0] == 0)
        pLang = getenv( "LANG" );
    if (! pLang || pLang[0] == 0)
        pLang = pFallback;

    return pLang;
}


typedef const char * (*getLangFromEnv)( bool& rbColonList );

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
                nLang = LanguageTag( *procLocale ).makeFallback().getLanguageType();
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                rSystemLanguage = nLang;
#ifdef DEBUG
                if ( rSystemLanguage == LANGUAGE_DONTKNOW )
                    fprintf( stderr, "intnunx.cxx:  failed to convert osl_getProcessLocale() language to system language.\n" );
#endif
            }
#else   /* MACOSX */
            bool bColonList = false;
            OString aUnxLang( pGetLangFromEnv( bColonList));
            if (bColonList)
            {
                // Only a very simple "take first". If empty try second or keep empty.
                sal_Int32 n = aUnxLang.indexOf(':');
                if (n >= 0)
                {
                    sal_Int32 s = 0;
                    if (n == 0 && aUnxLang.getLength() > 1)
                    {
                        n = aUnxLang.indexOf(':', 1);
                        if (n < 0)
                            n = aUnxLang.getLength();
                        if (n < 2)
                            s = n = 0;
                        else
                        {
                            s = 1;
                            --n;
                        }
                    }
                    aUnxLang = aUnxLang.copy(s,n);
                }
            }
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


LanguageType MsLangId::getPlatformSystemLanguage()
{
    getPlatformSystemLanguageImpl( nImplSystemLanguage, &getLangFromEnvironment);
    return nImplSystemLanguage;
}


LanguageType MsLangId::getPlatformSystemUILanguage()
{
    getPlatformSystemLanguageImpl( nImplSystemUILanguage, &getUILangFromEnvironment);
    return nImplSystemUILanguage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
