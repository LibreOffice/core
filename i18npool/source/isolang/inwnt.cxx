/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// no include "precompiled_i18nisolang.hxx" because this file is included in insys.cxx

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
    getPlatformSystemLanguageImpl( nImplSystemUILanguage,
            &GetUserDefaultUILanguage, &GetSystemDefaultUILanguage);
    return nImplSystemUILanguage;
}
