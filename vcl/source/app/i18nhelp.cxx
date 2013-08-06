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


#include "comphelper/processfactory.hxx"
#include "unotools/localedatawrapper.hxx"
#include "unotools/transliterationwrapper.hxx"

#include "i18nlangtag/languagetag.hxx"

#include "rtl/ustrbuf.hxx"

#include "vcl/i18nhelp.hxx"

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/i18n/TransliterationModules.hpp"

using namespace ::com::sun::star;

vcl::I18nHelper::I18nHelper(  const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext, const LanguageTag& rLanguageTag )
    :
        maLanguageTag( rLanguageTag)
{
    m_xContext = rxContext;
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

        ((vcl::I18nHelper*)this)->mpTransliterationWrapper = new utl::TransliterationWrapper( m_xContext, (i18n::TransliterationModules)nModules );
        ((vcl::I18nHelper*)this)->mpTransliterationWrapper->loadModuleIfNeeded( maLanguageTag.getLanguageType() );
    }
    return *mpTransliterationWrapper;
}

LocaleDataWrapper& vcl::I18nHelper::ImplGetLocaleDataWrapper() const
{
    if ( !mpLocaleDataWrapper )
    {
        ((vcl::I18nHelper*)this)->mpLocaleDataWrapper = new LocaleDataWrapper( m_xContext, maLanguageTag );
    }
    return *mpLocaleDataWrapper;
}

inline bool is_formatting_mark( sal_Unicode c )
{
    if( (c >= 0x200B) && (c <= 0x200F) )    // BiDi and zero-width-markers
        return true;
    if( (c >= 0x2028) && (c <= 0x202E) )    // BiDi and paragraph-markers
        return true;
    return false;
}

/* #i100057# filter formatting marks out of strings before passing them to
   the transliteration. The real solution would have been an additional TransliterationModule
   to ignore these marks during transliteration; however changin the code in i18npool that actually
   implements this could produce unwanted side effects.

   Of course this copying around is not really good, but looking at i18npool, one more time
   will not hurt.
*/
OUString vcl::I18nHelper::filterFormattingChars( const OUString& rStr )
{
    sal_Int32 nUnicodes = rStr.getLength();
    OUStringBuffer aBuf( nUnicodes );
    const sal_Unicode* pStr = rStr.getStr();
    while( nUnicodes-- )
    {
        if( ! is_formatting_mark( *pStr ) )
            aBuf.append( *pStr );
        pStr++;
    }
    return aBuf.makeStringAndClear();
}

sal_Int32 vcl::I18nHelper::CompareString( const OUString& rStr1, const OUString& rStr2 ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

    if ( mbTransliterateIgnoreCase )
    {
        // Change mbTransliterateIgnoreCase and destroy the warpper, next call to
        // ImplGetTransliterationWrapper() will create a wrapper with the correct bIgnoreCase
        ((vcl::I18nHelper*)this)->mbTransliterateIgnoreCase = sal_False;
        delete ((vcl::I18nHelper*)this)->mpTransliterationWrapper;
        ((vcl::I18nHelper*)this)->mpTransliterationWrapper = NULL;
    }


    String aStr1( filterFormattingChars(rStr1) );
    String aStr2( filterFormattingChars(rStr2) );
    return ImplGetTransliterationWrapper().compareString( aStr1, aStr2 );
}

sal_Bool vcl::I18nHelper::MatchString( const OUString& rStr1, const OUString& rStr2 ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

    if ( !mbTransliterateIgnoreCase )
    {
        // Change mbTransliterateIgnoreCase and destroy the warpper, next call to
        // ImplGetTransliterationWrapper() will create a wrapper with the correct bIgnoreCase
        ((vcl::I18nHelper*)this)->mbTransliterateIgnoreCase = sal_True;
        delete ((vcl::I18nHelper*)this)->mpTransliterationWrapper;
        ((vcl::I18nHelper*)this)->mpTransliterationWrapper = NULL;
    }

    String aStr1( filterFormattingChars(rStr1) );
    String aStr2( filterFormattingChars(rStr2) );
    return ImplGetTransliterationWrapper().isMatch( aStr1, aStr2 );
}

sal_Bool vcl::I18nHelper::MatchMnemonic( const OUString& rString, sal_Unicode cMnemonicChar ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

    sal_Bool bEqual = sal_False;
    sal_Int32 n = rString.indexOf( '~' );
    if ( n != -1 )
    {
        OUString aMatchStr = rString.copy( n+1 );   // not only one char, because of transliteration...
        bEqual = MatchString( OUString(cMnemonicChar), aMatchStr );
    }
    return bEqual;
}


OUString vcl::I18nHelper::GetNum( long nNumber, sal_uInt16 nDecimals, sal_Bool bUseThousandSep, sal_Bool bTrailingZeros ) const
{
    return ImplGetLocaleDataWrapper().getNum( nNumber, nDecimals, bUseThousandSep, bTrailingZeros );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
