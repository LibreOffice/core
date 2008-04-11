/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: i18nhelp.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"



#include <vcl/i18nhelp.hxx>

/*
#include <com/sun/star/lang/XSingleServiceFactory.hpp>


#include <comphelper/processfactory.hxx>
*/

// #include <cppuhelper/servicefactory.hxx>


#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <unotools/localedatawrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <i18npool/mslangid.hxx>

using namespace ::com::sun::star;

vcl::I18nHelper::I18nHelper(  ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF, const ::com::sun::star::lang::Locale& rLocale )
{
    mxMSF = rxMSF;
    maLocale = rLocale;
    mpLocaleDataWrapper = NULL;
    mpTransliterationWrapper= NULL;
    mbTransliterateIgnoreCase = sal_False;
}

vcl::I18nHelper::~I18nHelper()
{
    ImplDestroyWrappers();
}

void vcl::I18nHelper::ImplDestroyWrappers()
{
    delete mpLocaleDataWrapper;
    mpLocaleDataWrapper = NULL;

    delete mpTransliterationWrapper;
    mpTransliterationWrapper= NULL;
}

utl::TransliterationWrapper& vcl::I18nHelper::ImplGetTransliterationWrapper() const
{
    if ( !mpTransliterationWrapper )
    {
        sal_Int32 nModules = i18n::TransliterationModules_IGNORE_WIDTH;
        if ( mbTransliterateIgnoreCase )
            nModules |= i18n::TransliterationModules_IGNORE_CASE;

        ((vcl::I18nHelper*)this)->mpTransliterationWrapper = new utl::TransliterationWrapper( mxMSF, (i18n::TransliterationModules)nModules );
        ((vcl::I18nHelper*)this)->mpTransliterationWrapper->loadModuleIfNeeded( MsLangId::convertLocaleToLanguage( maLocale ) );
    }
    return *mpTransliterationWrapper;
}

LocaleDataWrapper& vcl::I18nHelper::ImplGetLocaleDataWrapper() const
{
    if ( !mpLocaleDataWrapper )
    {
        ((vcl::I18nHelper*)this)->mpLocaleDataWrapper = new LocaleDataWrapper( mxMSF, maLocale );
    }
    return *mpLocaleDataWrapper;
}

const ::com::sun::star::lang::Locale& vcl::I18nHelper::getLocale() const
{
    return maLocale;
}

sal_Int32 vcl::I18nHelper::CompareString( const String& rStr1, const String& rStr2 ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

    if ( mbTransliterateIgnoreCase )
    {
        // Change mbTransliterateIgnoreCase and destroy the warpper, next call to
        // ImplGetTransliterationWrapper() will create a wrapper with the correct bIgnoreCase
        ((vcl::I18nHelper*)this)->mbTransliterateIgnoreCase = FALSE;
        delete ((vcl::I18nHelper*)this)->mpTransliterationWrapper;
        ((vcl::I18nHelper*)this)->mpTransliterationWrapper = NULL;
    }

    return ImplGetTransliterationWrapper().compareString( rStr1, rStr2 );
}

sal_Bool vcl::I18nHelper::MatchString( const String& rStr1, const String& rStr2 ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

    if ( !mbTransliterateIgnoreCase )
    {
        // Change mbTransliterateIgnoreCase and destroy the warpper, next call to
        // ImplGetTransliterationWrapper() will create a wrapper with the correct bIgnoreCase
        ((vcl::I18nHelper*)this)->mbTransliterateIgnoreCase = TRUE;
        delete ((vcl::I18nHelper*)this)->mpTransliterationWrapper;
        ((vcl::I18nHelper*)this)->mpTransliterationWrapper = NULL;
    }

    return ImplGetTransliterationWrapper().isMatch( rStr1, rStr2 );
}

sal_Bool vcl::I18nHelper::MatchMnemonic( const String& rString, sal_Unicode cMnemonicChar ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

    BOOL bEqual = FALSE;
    USHORT n = rString.Search( '~' );
    if ( n != STRING_NOTFOUND )
    {
        String aMatchStr( rString, n+1, STRING_LEN );   // not only one char, because of transliteration...
        bEqual = MatchString( cMnemonicChar, aMatchStr );
    }
    return bEqual;
}


String vcl::I18nHelper::GetDate( const Date& rDate ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

    return ImplGetLocaleDataWrapper().getDate( rDate );
}

String vcl::I18nHelper::GetNum( long nNumber, USHORT nDecimals, BOOL bUseThousandSep, BOOL bTrailingZeros ) const
{
    return ImplGetLocaleDataWrapper().getNum( nNumber, nDecimals, bUseThousandSep, bTrailingZeros );
}
