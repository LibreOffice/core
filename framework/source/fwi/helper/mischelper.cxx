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
#include "precompiled_framework.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
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
                        rtl::OUString::createFromAscii( "com.sun.star.linguistic2.LanguageGuessing" ) ),
                        uno::UNO_QUERY );
        }
        catch (uno::Exception &r)
        {
            (void) r;
            DBG_ASSERT( 0, "failed to get language guessing component" );
        }
    }
    return m_xLanguageGuesser;
}

////////////////////////////////////////////////////////////

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
                Reference< XModuleManager > xModuleManager( _xServiceFactory->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );

                try
                {
                    _rModuleIdentifier = xModuleManager->identify( _xFrame );
                }
                catch( Exception& )
                {
                }
            }

            Reference< XNameAccess > xNameAccess( _xServiceFactory->createInstance( SERVICENAME_UICOMMANDDESCRIPTION ), UNO_QUERY );
            if ( xNameAccess.is() )
            {
                xNameAccess->getByName( _rModuleIdentifier ) >>= _xUICommandLabels;
            }
        }
        catch ( Exception& )
        {
        }
    }

    if ( _xUICommandLabels.is() )
    {
        try
        {
            if ( aCmdURL.getLength() > 0 )
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
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }

    return aLabel;
}

////////////////////////////////////////////////////////////

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
    if( rCurLang != OUString() &&
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
    if ( xLangGuesser.is() && rGuessedTextLang.getLength() > 0)
    {
        ::com::sun::star::lang::Locale aLocale(xLangGuesser->guessPrimaryLanguage( rGuessedTextLang, 0, rGuessedTextLang.getLength()) );
        LanguageType nLang = MsLangId::convertLocaleToLanguageWithFallback( aLocale );
        if (nLang != LANGUAGE_DONTKNOW && nLang != LANGUAGE_NONE && nLang != LANGUAGE_SYSTEM
            && IsScriptTypeMatchingToLanguage( nScriptType, nLang ))
            rLangItems.insert( rLanguageTable.GetString( nLang ));
    }

    //5--keyboard language
    if( rKeyboardLang != OUString())
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


