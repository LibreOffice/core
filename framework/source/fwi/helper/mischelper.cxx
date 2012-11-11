/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <tools/debug.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <i18npool/mslangid.hxx>
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

using ::rtl::OUString;


namespace framework
{

uno::Reference< linguistic2::XLanguageGuessing > LanguageGuessingHelper::GetGuesser() const
{
    if (!m_xLanguageGuesser.is())
    {
        try
        {
            m_xLanguageGuesser = uno::Reference< linguistic2::XLanguageGuessing >(
                    m_xServiceManager->createInstance(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.linguistic2.LanguageGuessing")) ),
                        uno::UNO_QUERY );
        }
        catch (const uno::Exception &)
        {
            DBG_ASSERT( 0, "failed to get language guessing component" );
        }
    }
    return m_xLanguageGuesser;
}


::rtl::OUString RetrieveLabelFromCommand(
    const ::rtl::OUString& aCmdURL,
    const uno::Reference< lang::XMultiServiceFactory >& _xServiceFactory,
    uno::Reference< container::XNameAccess >& _xUICommandLabels,
    const uno::Reference< frame::XFrame >& _xFrame,
    ::rtl::OUString& _rModuleIdentifier,
    sal_Bool& _rIni,
    const sal_Char* _pName)
{
    ::rtl::OUString aLabel;

    // Retrieve popup menu labels
    if ( !_xUICommandLabels.is() )
    {
      try
        {
            if ( !_rIni )
            {
                _rIni = sal_True;
                Reference< XModuleManager2 > xModuleManager = ModuleManager::create( comphelper::getComponentContext(_xServiceFactory) );

                try
                {
                    _rModuleIdentifier = xModuleManager->identify( _xFrame );
                }
                catch( const Exception& )
                {
                }
            }

            Reference< XNameAccess > xNameAccess( _xServiceFactory->createInstance( SERVICENAME_UICOMMANDDESCRIPTION ), UNO_QUERY );
            if ( xNameAccess.is() )
            {
                xNameAccess->getByName( _rModuleIdentifier ) >>= _xUICommandLabels;
            }
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
                rtl::OUString aStr;
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
    LanguageType rSystemLanguage = rAllSettings.GetLanguage();
    if( rSystemLanguage != LANGUAGE_DONTKNOW )
    {
        if ( IsScriptTypeMatchingToLanguage( nScriptType, rSystemLanguage ))
            rLangItems.insert( OUString( rLanguageTable.GetString( rSystemLanguage )) );
    }

    //3--UI
    LanguageType rUILanguage = rAllSettings.GetUILanguage();
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
        LanguageType nLang = MsLangId::convertLocaleToLanguageWithFallback( aLocale );
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
