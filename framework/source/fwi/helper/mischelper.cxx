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

uno::Reference< linguistic2::XLanguageGuessing > LanguageGuessingHelper::GetGuesser() const
{
    if (!m_xLanguageGuesser.is())
    {
        try
        {
            m_xLanguageGuesser = linguistic2::LanguageGuessing::create( m_xContext );
        }
        catch (const uno::Exception &)
        {
            DBG_ASSERT( false, "failed to get language guessing component" );
        }
    }
    return m_xLanguageGuesser;
}

OUString RetrieveLabelFromCommand(
    const OUString& aCmdURL,
    const uno::Reference< uno::XComponentContext >& _xContext,
    uno::Reference< container::XNameAccess >& _xUICommandLabels,
    const uno::Reference< frame::XFrame >& _xFrame,
    OUString& _rModuleIdentifier,
    bool& _rIni,
    const sal_Char* _pName)
{
    OUString aLabel;

    // Retrieve popup menu labels
    if ( !_xUICommandLabels.is() )
    {
      try
        {
            if ( !_rIni )
            {
                _rIni = true;
                Reference< XModuleManager2 > xModuleManager = ModuleManager::create( _xContext );

                try
                {
                    _rModuleIdentifier = xModuleManager->identify( _xFrame );
                }
                catch( const Exception& )
                {
                }
            }

            Reference< XNameAccess > xNameAccess = frame::theUICommandDescription::get( _xContext );
            xNameAccess->getByName( _rModuleIdentifier ) >>= _xUICommandLabels;
        }
        catch ( const Exception& )
        {
        }
    }

    if ( _xUICommandLabels.is() )
    {
        try
        {
            if ( !aCmdURL.isEmpty() )
            {
                OUString aStr;
                Sequence< PropertyValue > aPropSeq;
                if( _xUICommandLabels->hasByName( aCmdURL ) )
                {
                    if ( _xUICommandLabels->getByName( aCmdURL ) >>= aPropSeq )
                    {
                        for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                        {
                            if ( aPropSeq[i].Name.equalsAscii( _pName/*"Label"*/ ))
                            {
                                aPropSeq[i].Value >>= aStr;
                                break;
                            }
                        }
                    }
                }
                aLabel = aStr;
            }
        }
        catch ( const com::sun::star::uno::Exception& )
        {
        }
    }

    return aLabel;
}

void FillLangItems( std::set< OUString > &rLangItems,
        const SvtLanguageTable &    rLanguageTable,
        const uno::Reference< frame::XFrame > & rxFrame,
        const LanguageGuessingHelper & rLangGuessHelper,
        sal_Int16        nScriptType,
        const OUString & rCurLang,
        const OUString & rKeyboardLang,
        const OUString & rGuessedTextLang )
{
    rLangItems.clear();

    //1--add current language
    if( !rCurLang.isEmpty() &&
        LANGUAGE_DONTKNOW != rLanguageTable.GetType( rCurLang ))
        rLangItems.insert( rCurLang );

    //2--System
    const AllSettings& rAllSettings = Application::GetSettings();
    LanguageType rSystemLanguage = rAllSettings.GetLanguageTag().getLanguageType();
    if( rSystemLanguage != LANGUAGE_DONTKNOW )
    {
        if ( IsScriptTypeMatchingToLanguage( nScriptType, rSystemLanguage ))
            rLangItems.insert( OUString( rLanguageTable.GetString( rSystemLanguage )) );
    }

    //3--UI
    LanguageType rUILanguage = rAllSettings.GetUILanguageTag().getLanguageType();
    if( rUILanguage != LANGUAGE_DONTKNOW )
    {
        if ( IsScriptTypeMatchingToLanguage( nScriptType, rUILanguage ))
            rLangItems.insert( OUString( rLanguageTable.GetString( rUILanguage )) );
    }

    //4--guessed language
    uno::Reference< linguistic2::XLanguageGuessing > xLangGuesser( rLangGuessHelper.GetGuesser() );
    if ( xLangGuesser.is() && !rGuessedTextLang.isEmpty())
    {
        ::com::sun::star::lang::Locale aLocale(xLangGuesser->guessPrimaryLanguage( rGuessedTextLang, 0, rGuessedTextLang.getLength()) );
        LanguageType nLang = LanguageTag( aLocale ).makeFallback().getLanguageType();
        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_NONE && nLang != LANGUAGE_SYSTEM
            && IsScriptTypeMatchingToLanguage( nScriptType, nLang ))
            rLangItems.insert( rLanguageTable.GetString( nLang ));
    }

    //5--keyboard language
    if( !rKeyboardLang.isEmpty() )
    {
        if ( IsScriptTypeMatchingToLanguage( nScriptType, rLanguageTable.GetType( rKeyboardLang )))
            rLangItems.insert( rKeyboardLang );
    }

    //6--all languages used in current document
    Reference< com::sun::star::frame::XModel > xModel;
    if ( rxFrame.is() )
    {
       Reference< com::sun::star::frame::XController > xController( rxFrame->getController(), UNO_QUERY );
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
        Sequence< Locale > rLocales( xDocumentLanguages->getDocumentLanguages( nScriptType, nMaxCount ));
        if ( rLocales.getLength() > 0 )
        {
            for ( sal_uInt16 i = 0; i < rLocales.getLength(); ++i )
            {
                if ( rLangItems.size() == static_cast< size_t >(nMaxCount) )
                    break;
                const Locale& rLocale=rLocales[i];
                if( IsScriptTypeMatchingToLanguage( nScriptType, rLanguageTable.GetType( rLocale.Language )))
                    rLangItems.insert( OUString( rLocale.Language ) );
            }
        }
    }
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
