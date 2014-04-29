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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_misc.h"
#include "dp_resource.h"
#include "osl/module.hxx"
#include "osl/mutex.hxx"
#include "rtl/ustring.h"
#include "cppuhelper/implbase1.hxx"
#include "unotools/configmgr.hxx"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_misc {
namespace {

struct OfficeLocale :
        public rtl::StaticWithInit<const OUString, OfficeLocale> {
    const OUString operator () () {
        OUString slang;
        if (! (::utl::ConfigManager::GetDirectConfigProperty(
                   ::utl::ConfigManager::LOCALE ) >>= slang))
            throw RuntimeException( OUSTR("Cannot determine language!"), 0 );
        //fallback, the locale is currently only set when the user starts the
        //office for the first time.
        if (slang.getLength() == 0)
            slang =  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en-US"));
        return slang;
    }
};

struct DeploymentResMgr : public rtl::StaticWithInit<
    ResMgr *, DeploymentResMgr> {
    ResMgr * operator () () {
        return ResMgr::CreateResMgr( "deployment", getOfficeLocale() );
    }
};

osl::Mutex s_mutex;

} // anon namespace

//==============================================================================
ResId getResId( sal_uInt16 id )
{
    const osl::MutexGuard guard( s_mutex );
    return ResId( id, *DeploymentResMgr::get() );
}

//==============================================================================
String getResourceString( sal_uInt16 id )
{
    const osl::MutexGuard guard( s_mutex );
    String ret( ResId( id, *DeploymentResMgr::get() ) );
    if (ret.SearchAscii( "%PRODUCTNAME" ) != STRING_NOTFOUND) {
        static String s_brandName;
        if (s_brandName.Len() == 0) {
            OUString brandName(
                ::utl::ConfigManager::GetDirectConfigProperty(
                    ::utl::ConfigManager::PRODUCTNAME ).get<OUString>() );
            s_brandName = brandName;
        }
        ret.SearchAndReplaceAllAscii( "%PRODUCTNAME", s_brandName );
    }
    return ret;
}

//throws an Exception on failure
//primary subtag 2 or three letters(A-Z, a-z), i or x
void checkPrimarySubtag(::rtl::OUString const & tag)
{
    sal_Int32 len = tag.getLength();
    sal_Unicode const * arLang = tag.getStr();
    if (len < 1 || len > 3)
        throw Exception(OUSTR("Invalid language string."), 0);

    if (len == 1
        && (arLang[0] != 'i' && arLang[0] != 'x'))
        throw Exception(OUSTR("Invalid language string."), 0);

    if (len == 2 || len == 3)
    {
        for (sal_Int32 i = 0; i < len; i++)
        {
            if ( !((arLang[i] >= 'A' && arLang[i] <= 'Z')
                || (arLang[i] >= 'a' && arLang[i] <= 'z')))
            {
                throw Exception(OUSTR("Invalid language string."), 0);
            }
        }
    }
}

//throws an Exception on failure
//second subtag 2 letter country code or 3-8 letter other code(A-Z, a-z, 0-9)
void checkSecondSubtag(::rtl::OUString const & tag, bool & bIsCountry)
{
    sal_Int32 len = tag.getLength();
    sal_Unicode const * arLang = tag.getStr();
    if (len < 2 || len > 8)
        throw Exception(OUSTR("Invalid language string."), 0);
    //country code
    bIsCountry = false;
    if (len == 2)
    {
        for (sal_Int32 i = 0; i < 2; i++)
        {
            if (!( (arLang[i] >= 'A' && arLang[i] <= 'Z')
                || (arLang[i] >= 'a' && arLang[i] <= 'z')))
            {
                throw Exception(OUSTR("Invalid language string."), 0);
            }
        }
        bIsCountry = true;
    }

    if (len > 2)
    {
        for (sal_Int32 i = 0; i < len; i++)
        {
            if (!( (arLang[i] >= 'A' && arLang[i] <= 'Z')
                || (arLang[i] >= 'a' && arLang[i] <= 'z')
                || (arLang[i] >= '0' && arLang[i] <= '9') ))
            {
                throw Exception(OUSTR("Invalid language string."), 0);
            }
        }
    }
}

void checkThirdSubtag(::rtl::OUString const & tag)
{
    sal_Int32 len = tag.getLength();
    sal_Unicode const * arLang = tag.getStr();
    if (len < 1 || len > 8)
        throw Exception(OUSTR("Invalid language string."), 0);

    for (sal_Int32 i = 0; i < len; i++)
    {
        if (!( (arLang[i] >= 'A' && arLang[i] <= 'Z')
            || (arLang[i] >= 'a' && arLang[i] <= 'z')
            || (arLang[i] >= '0' && arLang[i] <= '9') ))
        {
            throw Exception(OUSTR("Invalid language string."), 0);
        }
    }
}

//=============================================================================

//We parse the string according to RFC 3066
//We only use the primary sub-tag and two subtags. That is lang-country-variant
//We do some simple tests if the string is correct. Actually this should do a
//validating parser
//We may have the case that there is no country tag, for example en-welsh
::com::sun::star::lang::Locale toLocale( ::rtl::OUString const & slang )
{
    OUString _sLang = slang.trim();
    ::com::sun::star::lang::Locale locale;
    sal_Int32 nIndex = 0;
    OUString lang = _sLang.getToken( 0, '-', nIndex );
    checkPrimarySubtag(lang);
    locale.Language = lang;
    OUString country = _sLang.getToken( 0, '-', nIndex );
    if (country.getLength() > 0)
    {
        bool bIsCountry = false;
        checkSecondSubtag(country, bIsCountry);
        if (bIsCountry)
        {
            locale.Country = country;
        }
        else
        {
             locale.Variant = country;
        }
    }
    if (locale.Variant.getLength() == 0)
    {
        OUString variant = _sLang.getToken( 0, '-', nIndex );
        if (variant.getLength() > 0)
        {
            checkThirdSubtag(variant);
            locale.Variant = variant;
        }
    }

    return locale;
}

//==============================================================================
lang::Locale getOfficeLocale()
{
    return toLocale(OfficeLocale::get());
}

::rtl::OUString getOfficeLocaleString()
{
    return OfficeLocale::get();
}

}

