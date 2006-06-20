/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inwnt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:46:25 $
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

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#pragma warning(push,1) // disable warnings within system headers
#include <windef.h>     // needed by winnls.h
#include <winbase.h>    // needed by winnls.h
#include <winnls.h>
#pragma warning(pop)

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include "i18npool/mslangid.hxx"
#endif

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
