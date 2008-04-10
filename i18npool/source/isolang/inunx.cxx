/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: inunx.cxx,v $
 * $Revision: 1.7 $
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


// no include "precompiled_i18npool.hxx" because this file is included in insys.cxx

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
#include "i18npool/mslangid.hxx"

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
    if (! pLang)
        pLang = getenv ( "LC_CTYPE" );
    if (! pLang)
        pLang = getenv( "LANG" );
    if (! pLang)
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
    if (! pLang)
        pLang = getenv ( "LC_ALL" );
    if (! pLang)
        pLang = getenv ( "LC_MESSAGES" );
    if (! pLang)
        pLang = getenv( "LANG" );
    if (! pLang)
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
                rtl::OUString     rLang( procLocale->Language );
                rtl::OUString     rCountry( procLocale->Country );

                nLang = MsLangId::convertIsoNamesToLanguage( rLang, rCountry );
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                rSystemLanguage = nLang;
#ifdef DEBUG
                if ( rSystemLanguage == LANGUAGE_DONTKNOW )
                    fprintf( stderr, "intnunx.cxx:  failed to convert osl_getProcessLocale() language to system language.\n" );
#endif
            }
#else   /* MACOSX */
            rtl::OString aUnxLang( (pGetLangFromEnv)() );
            nLang = MsLangId::convertUnxByteStringToLanguage( aUnxLang );
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            rSystemLanguage = nLang;
#endif  /* MACOSX */
        }
        else
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
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
