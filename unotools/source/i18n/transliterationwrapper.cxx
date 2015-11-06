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

#include <sal/log.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <com/sun/star/i18n/TransliterationModulesExtra.hpp>
#include <com/sun/star/i18n/Transliteration.hpp>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::utl;

TransliterationWrapper::TransliterationWrapper(
                    const Reference< XComponentContext > & rxContext,
                    sal_uInt32 nTyp )
    : xTrans( Transliteration::create(rxContext) ),
      aLanguageTag( LANGUAGE_SYSTEM ), nType( nTyp ), bFirstCall( true )
{
}

TransliterationWrapper::~TransliterationWrapper()
{
}

OUString TransliterationWrapper::transliterate(const OUString& rStr, sal_uInt16 nLang,
                                               sal_Int32 nStart, sal_Int32 nLen,
                                               Sequence <sal_Int32>* pOffset )
{
    OUString sRet;
    if( xTrans.is() )
    {
        try
        {
            loadModuleIfNeeded( nLang );

            if ( pOffset )
                sRet = xTrans->transliterate( rStr, nStart, nLen, *pOffset );
            else
                sRet = xTrans->transliterateString2String( rStr, nStart, nLen);
        }
        catch( Exception&  )
        {
            SAL_WARN( "unotools.i18n", "transliterate: Exception caught!" );
        }
    }
    return sRet;
}

OUString TransliterationWrapper::transliterate( const OUString& rStr,
                                                sal_Int32 nStart, sal_Int32 nLen,
                                                Sequence <sal_Int32>* pOffset ) const
{
    OUString sRet( rStr );
    if( xTrans.is() )
    {
        try
        {
            if ( pOffset )
                sRet = xTrans->transliterate( rStr, nStart, nLen, *pOffset );
            else
                sRet = xTrans->transliterateString2String( rStr, nStart, nLen);
        }
        catch( Exception&  )
        {
            SAL_WARN( "unotools.i18n", "transliterate: Exception caught!" );
        }
    }
    return sRet;
}

bool TransliterationWrapper::needLanguageForTheMode() const
{
    return TransliterationModules_UPPERCASE_LOWERCASE == nType ||
           TransliterationModules_LOWERCASE_UPPERCASE == nType ||
           TransliterationModules_IGNORE_CASE == nType ||
           (sal_uInt32) TransliterationModulesExtra::SENTENCE_CASE == (sal_uInt32) nType ||
           (sal_uInt32) TransliterationModulesExtra::TITLE_CASE    == (sal_uInt32) nType ||
           (sal_uInt32) TransliterationModulesExtra::TOGGLE_CASE   == (sal_uInt32) nType;
}

void TransliterationWrapper::setLanguageLocaleImpl( sal_uInt16 nLang )
{
    if( LANGUAGE_NONE == nLang )
        nLang = LANGUAGE_SYSTEM;
    aLanguageTag.reset( nLang);
}

void TransliterationWrapper::loadModuleIfNeeded( sal_uInt16 nLang )
{
    bool bLoad = bFirstCall;
    bFirstCall = false;

    if( static_cast< sal_Int32 >(nType) == TransliterationModulesExtra::SENTENCE_CASE )
    {
        if( bLoad )
            loadModuleByImplName("SENTENCE_CASE", nLang);
    }
    else if( static_cast< sal_Int32 >(nType) == TransliterationModulesExtra::TITLE_CASE )
    {
        if( bLoad )
            loadModuleByImplName("TITLE_CASE", nLang);
    }
    else if( static_cast< sal_Int32 >(nType) == TransliterationModulesExtra::TOGGLE_CASE )
    {
        if( bLoad )
            loadModuleByImplName("TOGGLE_CASE", nLang);
    }
    else
    {
        if( aLanguageTag.getLanguageType() != nLang )
        {
            setLanguageLocaleImpl( nLang );
            if( !bLoad )
                bLoad = needLanguageForTheMode();
        }
        if( bLoad )
            loadModuleImpl();
    }
}

void TransliterationWrapper::loadModuleImpl() const
{
    if ( bFirstCall )
        const_cast<TransliterationWrapper*>(this)->setLanguageLocaleImpl( LANGUAGE_SYSTEM );

    try
    {
        if ( xTrans.is() )
            xTrans->loadModule( (TransliterationModules)nType, aLanguageTag.getLocale() );
    }
    catch ( const Exception& e )
    {
        SAL_WARN( "unotools.i18n", "loadModuleImpl: Exception caught " << e.Message );
    }

    bFirstCall = false;
}

void TransliterationWrapper::loadModuleByImplName(const OUString& rModuleName,
                                                  sal_uInt16 nLang )
{
    try
    {
        setLanguageLocaleImpl( nLang );
        com::sun::star::lang::Locale aLocale( aLanguageTag.getLocale());
        // Reset LanguageTag, so the next call to loadModuleIfNeeded() forces
        // new settings.
        aLanguageTag.reset( LANGUAGE_DONTKNOW);
        if ( xTrans.is() )
            xTrans->loadModuleByImplName( rModuleName, aLocale );
    }
    catch ( const Exception& e )
    {
        SAL_WARN( "unotools.i18n", "loadModuleByImplName: Exception caught " << e.Message );
    }

    bFirstCall = false;
}

bool TransliterationWrapper::equals(
    const OUString& rStr1, sal_Int32 nPos1, sal_Int32 nCount1, sal_Int32& nMatch1,
    const OUString& rStr2, sal_Int32 nPos2, sal_Int32 nCount2, sal_Int32& nMatch2 ) const
{
    try
    {
        if( bFirstCall )
            loadModuleImpl();
        if ( xTrans.is() )
            return xTrans->equals( rStr1, nPos1, nCount1, nMatch1, rStr2, nPos2, nCount2, nMatch2 );
    }
    catch ( const Exception& e )
    {
        SAL_WARN( "unotools.i18n", "equals: Exception caught " << e.Message );
    }
    return false;
}

sal_Int32 TransliterationWrapper::compareString( const OUString& rStr1, const OUString& rStr2 ) const
{
    try
    {
        if( bFirstCall )
            loadModuleImpl();
        if ( xTrans.is() )
            return xTrans->compareString( rStr1, rStr2 );
    }
    catch (const Exception& e)
    {
        SAL_WARN( "unotools.i18n", "compareString: Exception caught " << e.Message );
    }
    return 0;
}

// --- helpers --------------------------------------------------------

bool TransliterationWrapper::isEqual( const OUString& rStr1, const OUString& rStr2 ) const
{
    sal_Int32 nMatch1(0), nMatch2(0);
    bool bMatch = equals(
        rStr1, 0, rStr1.getLength(), nMatch1,
        rStr2, 0, rStr2.getLength(), nMatch2 );
    return bMatch;
}

bool TransliterationWrapper::isMatch( const OUString& rStr1, const OUString& rStr2 ) const
{
    sal_Int32 nMatch1(0), nMatch2(0);
    equals(
        rStr1, 0, rStr1.getLength(), nMatch1,
        rStr2, 0, rStr2.getLength(), nMatch2 );
    return (nMatch1 <= nMatch2) && (nMatch1 == rStr1.getLength());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
