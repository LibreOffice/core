/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: inwnt.cxx,v $
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

#include <sal/config.h>

#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#endif
#include <windef.h>     // needed by winnls.h
#include <winbase.h>    // needed by winnls.h
#include <winnls.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <rtl/instance.hxx>
#include "i18npool/mslangid.hxx"

static LanguageType nImplSystemLanguage = LANGUAGE_DONTKNOW;
static LanguageType nImplSystemUILanguage = LANGUAGE_DONTKNOW;

// =======================================================================

static LanguageType GetSVLang( LANGID nWinLangId )
{
    // No Translation, we work with the original MS code without the SORT_ID.
    // So we can get never LANG-ID's from MS, which are currently not defined
    // by us.
    return LanguageType( static_cast<sal_uInt16>(nWinLangId & 0xffff));
}

// -----------------------------------------------------------------------

typedef LANGID (WINAPI *getLangFromEnv)();

static void getPlatformSystemLanguageImpl( LanguageType& rSystemLanguage,
        getLangFromEnv pGetUserDefault, getLangFromEnv pGetSystemDefault )
{
    LanguageType nLang = rSystemLanguage;
    if ( nLang == LANGUAGE_DONTKNOW )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex());
        nLang = rSystemLanguage;
        if ( nLang == LANGUAGE_DONTKNOW )
        {
            LANGID nLangId;

            nLangId = (pGetUserDefault)();
            nLang = GetSVLang( nLangId );

            if ( nLang == LANGUAGE_DONTKNOW )
            {
                nLangId = (pGetSystemDefault)();
                nLang = GetSVLang( nLangId );
            }
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            rSystemLanguage = nLang;
        }
        else
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
}

// -----------------------------------------------------------------------

LanguageType MsLangId::getPlatformSystemLanguage()
{
    getPlatformSystemLanguageImpl( nImplSystemLanguage,
            &GetUserDefaultLangID, &GetSystemDefaultLangID);
    return nImplSystemLanguage;
}

// -----------------------------------------------------------------------

LanguageType MsLangId::getPlatformSystemUILanguage()
{
    // TODO: this could be distinguished, #if(WINVER >= 0x0500)
    // needs _run_ time differentiation though, not at compile time.
#if 0
    getPlatformSystemLanguageImpl( nImplSystemUILanguage,
            &GetUserDefaultUILanguage, &GetSystemDefaultUILanguage);
#endif
    getPlatformSystemLanguageImpl( nImplSystemUILanguage,
            &GetUserDefaultLangID, &GetSystemDefaultLangID);
    return nImplSystemUILanguage;
}
