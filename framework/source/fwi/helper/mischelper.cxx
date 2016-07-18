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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/linguistic2/LanguageGuessing.hpp>

#include <tools/debug.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svtools/langtab.hxx>
#include <comphelper/processfactory.hxx>
#include <helper/mischelper.hxx>
#include <services.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace framework
{

uno::Reference< linguistic2::XLanguageGuessing > const & LanguageGuessingHelper::GetGuesser() const
{
    if (!m_xLanguageGuesser.is())
    {
        try
        {
            m_xLanguageGuesser = linguistic2::LanguageGuessing::create( m_xContext );
        }
        catch (const uno::Exception &)
        {
            SAL_WARN( "fwk", "failed to get language guessing component" );
        }
    }
    return m_xLanguageGuesser;
}

void FillLangItems( std::set< OUString > &rLangItems,
        const uno::Reference< frame::XFrame > & rxFrame,
        const LanguageGuessingHelper & rLangGuessHelper,
        SvtScriptType    nScriptType,
        const OUString & rCurLang,
        const OUString & rKeyboardLang,
        const OUString & rGuessedTextLang )
{
    rLangItems.clear();

    //1--add current language
    if( !rCurLang.isEmpty() &&
        LANGUAGE_DONTKNOW != SvtLanguageTable::GetLanguageType( rCurLang ))
        rLangItems.insert( rCurLang );

    //2--System
    const AllSettings& rAllSettings = Application::GetSettings();
    LanguageType rSystemLanguage = rAllSettings.GetLanguageTag().getLanguageType();
    if( rSystemLanguage != LANGUAGE_DONTKNOW )
    {
        if ( IsScriptTypeMatchingToLanguage( nScriptType, rSystemLanguage ))
            rLangItems.insert( OUString( SvtLanguageTable::GetLanguageString( rSystemLanguage )) );
    }

    //3--UI
    LanguageType rUILanguage = rAllSettings.GetUILanguageTag().getLanguageType();
    if( rUILanguage != LANGUAGE_DONTKNOW )
    {
        if ( IsScriptTypeMatchingToLanguage( nScriptType, rUILanguage ))
            rLangItems.insert( OUString( SvtLanguageTable::GetLanguageString( rUILanguage )) );
    }

    //4--guessed language
    uno::Reference< linguistic2::XLanguageGuessing > xLangGuesser( rLangGuessHelper.GetGuesser() );
    if ( xLangGuesser.is() && !rGuessedTextLang.isEmpty())
    {
        css::lang::Locale aLocale(xLangGuesser->guessPrimaryLanguage( rGuessedTextLang, 0, rGuessedTextLang.getLength()) );
        LanguageType nLang = LanguageTag( aLocale ).makeFallback().getLanguageType();
        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_NONE && nLang != LANGUAGE_SYSTEM
            && IsScriptTypeMatchingToLanguage( nScriptType, nLang ))
            rLangItems.insert( SvtLanguageTable::GetLanguageString( nLang ));
    }

    //5--keyboard language
    if( !rKeyboardLang.isEmpty() )
    {
        if ( IsScriptTypeMatchingToLanguage( nScriptType, SvtLanguageTable::GetLanguageType( rKeyboardLang )))
            rLangItems.insert( rKeyboardLang );
    }

    //6--all languages used in current document
    Reference< css::frame::XModel > xModel;
    if ( rxFrame.is() )
    {
       Reference< css::frame::XController > xController( rxFrame->getController(), UNO_QUERY );
       if ( xController.is() )
           xModel = xController->getModel();
    }
    Reference< document::XDocumentLanguages > xDocumentLanguages( xModel, UNO_QUERY );
    /*the description of nScriptType
      LATIN :   0x001
      ASIAN :   0x002
      COMPLEX:  0x004
    */
    const sal_Int16 nMaxCount = 7;
    if ( xDocumentLanguages.is() )
    {
        Sequence< Locale > rLocales( xDocumentLanguages->getDocumentLanguages( static_cast<sal_Int16>(nScriptType), nMaxCount ));
        if ( rLocales.getLength() > 0 )
        {
            for ( sal_Int32 i = 0; i < rLocales.getLength(); ++i )
            {
                if ( rLangItems.size() == static_cast< size_t >(nMaxCount) )
                    break;
                const Locale& rLocale=rLocales[i];
                if( IsScriptTypeMatchingToLanguage( nScriptType, SvtLanguageTable::GetLanguageType( rLocale.Language )))
                    rLangItems.insert( OUString( rLocale.Language ) );
            }
        }
    }
}

auto (*g_pGetMultiplexerListener)(
    uno::Reference<uno::XInterface> const&,
    std::function<bool (uno::Reference<ui::XContextChangeEventListener> const&)> const&)
    -> uno::Reference<ui::XContextChangeEventListener> = nullptr;

uno::Reference<ui::XContextChangeEventListener>
GetFirstListenerWith_Impl(
    uno::Reference<uno::XInterface> const& xEventFocus,
    std::function<bool (uno::Reference<ui::XContextChangeEventListener> const&)> const& rPredicate)
{
    assert(g_pGetMultiplexerListener != nullptr); // should not be called too early, nor too late
    return g_pGetMultiplexerListener(xEventFocus, rPredicate);
}


} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
