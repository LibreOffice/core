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

#include <unotools/localedatawrapper.hxx>
#include <unotools/transliterationwrapper.hxx>

#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/transliteration.hxx>

#include <rtl/ustrbuf.hxx>

#include <utility>
#include <vcl/i18nhelp.hxx>

using namespace ::com::sun::star;

vcl::I18nHelper::I18nHelper(  const css::uno::Reference< css::uno::XComponentContext >& rxContext, LanguageTag aLanguageTag )
    :
        maLanguageTag(std::move( aLanguageTag))
{
    m_xContext = rxContext;
    mpLocaleDataWrapper = nullptr;
    mpTransliterationWrapper= nullptr;
    mbTransliterateIgnoreCase = false;
}

vcl::I18nHelper::~I18nHelper()
{
    ImplDestroyWrappers();
}

void vcl::I18nHelper::ImplDestroyWrappers()
{
    mpLocaleDataWrapper = nullptr;
    mpTransliterationWrapper.reset();
}

utl::TransliterationWrapper& vcl::I18nHelper::ImplGetTransliterationWrapper() const
{
    if ( !mpTransliterationWrapper )
    {
        TransliterationFlags nModules = TransliterationFlags::IGNORE_WIDTH;
        if ( mbTransliterateIgnoreCase )
            nModules |= TransliterationFlags::IGNORE_CASE;

        const_cast<vcl::I18nHelper*>(this)->mpTransliterationWrapper.reset(new utl::TransliterationWrapper( m_xContext, nModules ));
        const_cast<vcl::I18nHelper*>(this)->mpTransliterationWrapper->loadModuleIfNeeded( maLanguageTag.getLanguageType() );
    }
    return *mpTransliterationWrapper;
}

const LocaleDataWrapper& vcl::I18nHelper::ImplGetLocaleDataWrapper() const
{
    if ( !mpLocaleDataWrapper )
    {
        const_cast<vcl::I18nHelper*>(this)->mpLocaleDataWrapper = LocaleDataWrapper::get( maLanguageTag );
    }
    return *mpLocaleDataWrapper;
}

static bool is_formatting_mark( sal_Unicode c )
{
    if( (c >= 0x200B) && (c <= 0x200F) )    // BiDi and zero-width-markers
        return true;
    if( (c >= 0x2028) && (c <= 0x202E) )    // BiDi and paragraph-markers
        return true;
    return false;
}

/* #i100057# filter formatting marks out of strings before passing them to
   the transliteration. The real solution would have been an additional TransliterationModule
   to ignore these marks during transliteration; however changing the code in i18npool that actually
   implements this could produce unwanted side effects.

   Of course this copying around is not really good, but looking at i18npool, one more time
   will not hurt.
*/
OUString vcl::I18nHelper::filterFormattingChars( const OUString& rStr )
{
    sal_Int32 nLength = rStr.getLength();
    OUStringBuffer aBuf( nLength );
    const sal_Unicode* pStr = rStr.getStr();
    while( nLength-- )
    {
        if( ! is_formatting_mark( *pStr ) )
            aBuf.append( *pStr );
        pStr++;
    }
    return aBuf.makeStringAndClear();
}

sal_Int32 vcl::I18nHelper::CompareString( const OUString& rStr1, const OUString& rStr2 ) const
{
    std::unique_lock aGuard( maMutex );

    if ( mbTransliterateIgnoreCase )
    {
        // Change mbTransliterateIgnoreCase and destroy the wrapper, next call to
        // ImplGetTransliterationWrapper() will create a wrapper with the correct bIgnoreCase
        const_cast<vcl::I18nHelper*>(this)->mbTransliterateIgnoreCase = false;
        const_cast<vcl::I18nHelper*>(this)->mpTransliterationWrapper.reset();
    }

    OUString aStr1( filterFormattingChars(rStr1) );
    OUString aStr2( filterFormattingChars(rStr2) );
    return ImplGetTransliterationWrapper().compareString( aStr1, aStr2 );
}

bool vcl::I18nHelper::MatchString( const OUString& rStr1, const OUString& rStr2 ) const
{
    std::unique_lock aGuard( maMutex );

    if ( !mbTransliterateIgnoreCase )
    {
        // Change mbTransliterateIgnoreCase and destroy the wrapper, next call to
        // ImplGetTransliterationWrapper() will create a wrapper with the correct bIgnoreCase
        const_cast<vcl::I18nHelper*>(this)->mbTransliterateIgnoreCase = true;
        const_cast<vcl::I18nHelper*>(this)->mpTransliterationWrapper.reset();
    }

    OUString aStr1( filterFormattingChars(rStr1) );
    OUString aStr2( filterFormattingChars(rStr2) );
    return ImplGetTransliterationWrapper().isMatch( aStr1, aStr2 );
}

bool vcl::I18nHelper::MatchMnemonic( std::u16string_view rString, sal_Unicode cMnemonicChar ) const
{
    size_t n = rString.find( '~' );
    if ( n == std::u16string_view::npos )
        return false;
    OUString aMatchStr( rString.substr( n+1 ) );   // not only one char, because of transliteration...
    return MatchString( OUString(cMnemonicChar), aMatchStr );
}

OUString vcl::I18nHelper::GetNum( tools::Long nNumber, sal_uInt16 nDecimals, bool bUseThousandSep, bool bTrailingZeros ) const
{
    return ImplGetLocaleDataWrapper().getNum( nNumber, nDecimals, bUseThousandSep, bTrailingZeros );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
