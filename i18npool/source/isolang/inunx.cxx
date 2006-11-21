/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inunx.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 17:28:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#include <stdlib.h>  // for getenv()
#include <stdio.h>

#ifdef MACOSX

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _RTL_LOCALE_H_
#include <rtl/locale.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#else   // MACOSX

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#endif  // MACOSX

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include "i18npool/mslangid.hxx"
#endif

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
