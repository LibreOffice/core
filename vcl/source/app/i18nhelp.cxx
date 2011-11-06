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
#include "precompiled_vcl.hxx"

#include "unotools/localedatawrapper.hxx"
#include "unotools/transliterationwrapper.hxx"

#include "i18npool/mslangid.hxx"

#include "rtl/ustrbuf.hxx"

#include "vcl/i18nhelp.hxx"

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/i18n/TransliterationModules.hpp"

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
String vcl::I18nHelper::filterFormattingChars( const String& rStr )
{
    sal_Int32 nUnicodes = rStr.Len();
    rtl::OUStringBuffer aBuf( nUnicodes );
    const sal_Unicode* pStr = rStr.GetBuffer();
    while( nUnicodes-- )
    {
        if( ! is_formatting_mark( *pStr ) )
            aBuf.append( *pStr );
        pStr++;
    }
    return aBuf.makeStringAndClear();
}

sal_Int32 vcl::I18nHelper::CompareString( const String& rStr1, const String& rStr2 ) const
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

sal_Bool vcl::I18nHelper::MatchString( const String& rStr1, const String& rStr2 ) const
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

sal_Bool vcl::I18nHelper::MatchMnemonic( const String& rString, sal_Unicode cMnemonicChar ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((vcl::I18nHelper*)this)->maMutex );

    sal_Bool bEqual = sal_False;
    sal_uInt16 n = rString.Search( '~' );
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

String vcl::I18nHelper::GetNum( long nNumber, sal_uInt16 nDecimals, sal_Bool bUseThousandSep, sal_Bool bTrailingZeros ) const
{
    return ImplGetLocaleDataWrapper().getNum( nNumber, nDecimals, bUseThousandSep, bTrailingZeros );
}
