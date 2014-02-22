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


#include <sal/config.h>

#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#endif
#include <windef.h>
#include <winbase.h>
#include <winnls.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <rtl/instance.hxx>
#include "i18nlangtag/mslangid.hxx"

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
        {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        }
    }
}



LanguageType MsLangId::getPlatformSystemLanguage()
{
    getPlatformSystemLanguageImpl( nImplSystemLanguage,
            &GetUserDefaultLangID, &GetSystemDefaultLangID);
    return nImplSystemLanguage;
}



LanguageType MsLangId::getPlatformSystemUILanguage()
{
    // TODO: this could be distinguished, #if(WINVER >= 0x0500)
    // needs _run_ time differentiation though, not at compile time.
    getPlatformSystemLanguageImpl( nImplSystemUILanguage,
            &GetUserDefaultUILanguage, &GetSystemDefaultUILanguage);
    return nImplSystemUILanguage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
