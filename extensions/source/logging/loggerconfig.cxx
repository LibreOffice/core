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


#include "loggerconfig.hxx"
#include <stdio.h>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/logging/XLogHandler.hpp>
#include <com/sun/star/logging/XLogFormatter.hpp>

#include <tools/diagnose_ex.h>
#include <osl/process.h>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/component_context.hxx>

#include <vector>
#include <sal/macros.h>

//........................................................................
namespace logging
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::logging::XLogger;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::container::XNameContainer;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::lang::XSingleServiceFactory;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::util::XChangesBatch;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::lang::ServiceNotRegisteredException;
    using ::com::sun::star::beans::NamedValue;
    using ::com::sun::star::logging::XLogHandler;
    using ::com::sun::star::logging::XLogFormatter;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::uno::XComponentContext;

    namespace LogLevel = ::com::sun::star::logging::LogLevel;

    namespace
    {
        //----------------------------------------------------------------
        typedef void (*SettingTranslation)( const Reference< XLogger >&, const OUString&, Any& );

        //----------------------------------------------------------------
        void    lcl_substituteFileHandlerURLVariables_nothrow( const Reference< XLogger >& _rxLogger, OUString& _inout_rFileURL )
        {
            struct Variable
            {
                const sal_Char*         pVariablePattern;
                const sal_Int32         nPatternLength;
                rtl_TextEncoding        eEncoding;
                const OUString   sVariableValue;

                Variable( const sal_Char* _pVariablePattern,  const sal_Int32 _nPatternLength, rtl_TextEncoding _eEncoding,
                        const OUString& _rVariableValue )
                    :pVariablePattern( _pVariablePattern )
                    ,nPatternLength( _nPatternLength )
                    ,eEncoding( _eEncoding )
                    ,sVariableValue( _rVariableValue )
                {
                }
            };

            OUString sLoggerName;
            try { sLoggerName = _rxLogger->getName(); }
            catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }

            TimeValue aTimeValue;
            oslDateTime aDateTime;
            OSL_VERIFY( osl_getSystemTime( &aTimeValue ) );
            OSL_VERIFY( osl_getDateTimeFromTimeValue( &aTimeValue, &aDateTime ) );

            char buffer[ 30 ];
            const size_t buffer_size = sizeof( buffer );

            snprintf( buffer, buffer_size, "%04i-%02i-%02i",
                      (int)aDateTime.Year,
                      (int)aDateTime.Month,
                      (int)aDateTime.Day );
            rtl::OUString sDate = rtl::OUString::createFromAscii( buffer );

            snprintf( buffer, buffer_size, "%02i-%02i-%02i.%03i",
                (int)aDateTime.Hours,
                (int)aDateTime.Minutes,
                (int)aDateTime.Seconds,
                ::sal::static_int_cast< sal_Int16 >( aDateTime.NanoSeconds / 10000000 ) );
            rtl::OUString sTime = rtl::OUString::createFromAscii( buffer );

            rtl::OUStringBuffer aBuff;
            aBuff.append( sDate );
            aBuff.append( sal_Unicode( '.' ) );
            aBuff.append( sTime );
            rtl::OUString sDateTime = aBuff.makeStringAndClear();

            oslProcessIdentifier aProcessId = 0;
            oslProcessInfo info;
            info.Size = sizeof (oslProcessInfo);
            if ( osl_getProcessInfo ( 0, osl_Process_IDENTIFIER, &info ) == osl_Process_E_None)
                aProcessId = info.Ident;
            rtl::OUString aPID = OUString::number( aProcessId );

            Variable aVariables[] =
            {
                Variable( RTL_CONSTASCII_USTRINGPARAM( "$(loggername)" ), sLoggerName ),
                Variable( RTL_CONSTASCII_USTRINGPARAM( "$(date)" ), sDate ),
                Variable( RTL_CONSTASCII_USTRINGPARAM( "$(time)" ), sTime ),
                Variable( RTL_CONSTASCII_USTRINGPARAM( "$(datetime)" ), sDateTime ),
                Variable( RTL_CONSTASCII_USTRINGPARAM( "$(pid)" ), aPID )
            };

            for ( size_t i = 0; i < SAL_N_ELEMENTS( aVariables ); ++i )
            {
                OUString sPattern( aVariables[i].pVariablePattern, aVariables[i].nPatternLength, aVariables[i].eEncoding );
                sal_Int32 nVariableIndex = _inout_rFileURL.indexOf( sPattern );
                if  (   ( nVariableIndex == 0 )
                    ||  (   ( nVariableIndex > 0 )
                        &&  ( sPattern[ nVariableIndex - 1 ] != '$' )
                        )
                    )
                {
                    // found an (unescaped) variable
                    _inout_rFileURL = _inout_rFileURL.replaceAt( nVariableIndex, sPattern.getLength(), aVariables[i].sVariableValue );
                }
            }
        }

        //----------------------------------------------------------------
        void    lcl_transformFileHandlerSettings_nothrow( const Reference< XLogger >& _rxLogger, const OUString& _rSettingName, Any& _inout_rSettingValue )
        {
            if ( _rSettingName != "FileURL" )
                // not interested in this setting
                return;

            OUString sURL;
            OSL_VERIFY( _inout_rSettingValue >>= sURL );
            lcl_substituteFileHandlerURLVariables_nothrow( _rxLogger, sURL );
            _inout_rSettingValue <<= sURL;
        }

        //----------------------------------------------------------------
        Reference< XInterface > lcl_createInstanceFromSetting_throw(
                const Reference<XComponentContext>& _rContext,
                const Reference< XLogger >& _rxLogger,
                const Reference< XNameAccess >& _rxLoggerSettings,
                const sal_Char* _pServiceNameAsciiNodeName,
                const sal_Char* _pServiceSettingsAsciiNodeName,
                SettingTranslation _pSettingTranslation = NULL
            )
        {
            Reference< XInterface > xInstance;

            // read the settings for the to-be-created service
            Reference< XNameAccess > xServiceSettingsNode( _rxLoggerSettings->getByName(
                OUString::createFromAscii( _pServiceSettingsAsciiNodeName ) ), UNO_QUERY_THROW );

            Sequence< OUString > aSettingNames( xServiceSettingsNode->getElementNames() );
            size_t nServiceSettingCount( aSettingNames.getLength() );
            Sequence< NamedValue > aSettings( nServiceSettingCount );
            if ( nServiceSettingCount )
            {
                const OUString* pSettingNames = aSettingNames.getConstArray();
                const OUString* pSettingNamesEnd = aSettingNames.getConstArray() + aSettingNames.getLength();
                NamedValue* pSetting = aSettings.getArray();

                for (   ;
                        pSettingNames != pSettingNamesEnd;
                        ++pSettingNames, ++pSetting
                    )
                {
                    pSetting->Name = *pSettingNames;
                    pSetting->Value = xServiceSettingsNode->getByName( *pSettingNames );

                    if ( _pSettingTranslation )
                        (_pSettingTranslation)( _rxLogger, pSetting->Name, pSetting->Value );
                }
            }

            OUString sServiceName;
            _rxLoggerSettings->getByName( OUString::createFromAscii( _pServiceNameAsciiNodeName ) ) >>= sServiceName;
            if ( !sServiceName.isEmpty() )
            {
                bool bSuccess = false;
                if ( aSettings.getLength() )
                {
                    Sequence< Any > aConstructionArgs(1);
                    aConstructionArgs[0] <<= aSettings;
                    xInstance = _rContext->getServiceManager()->createInstanceWithArgumentsAndContext(sServiceName, aConstructionArgs, _rContext);
                    bSuccess = xInstance.is();
                }
                else
                {
                    xInstance = _rContext->getServiceManager()->createInstanceWithContext(sServiceName, _rContext);
                    bSuccess = xInstance.is();
                }

                if ( !bSuccess )
                    throw ServiceNotRegisteredException( sServiceName, NULL );
            }

            return xInstance;
        }
    }

    //--------------------------------------------------------------------
    void initializeLoggerFromConfiguration( const Reference<XComponentContext>& _rContext, const Reference< XLogger >& _rxLogger )
    {
        try
        {
            if ( !_rxLogger.is() )
                throw NullPointerException();

            Reference< XMultiServiceFactory > xConfigProvider(
                com::sun::star::configuration::theDefaultProvider::get(_rContext));

            // write access to the "Settings" node (which includes settings for all loggers)
            Sequence< Any > aArguments(1);
            aArguments[0] <<= NamedValue(
                OUString( "nodepath" ),
                makeAny( OUString( "/org.openoffice.Office.Logging/Settings" ) )
            );
            Reference< XNameContainer > xAllSettings( xConfigProvider->createInstanceWithArguments(
                OUString( "com.sun.star.configuration.ConfigurationUpdateAccess" ),
                aArguments
            ), UNO_QUERY_THROW );

            OUString sLoggerName( _rxLogger->getName() );
            if ( !xAllSettings->hasByName( sLoggerName ) )
            {
                // no node yet for this logger. Create default settings.
                Reference< XSingleServiceFactory > xNodeFactory( xAllSettings, UNO_QUERY_THROW );
                Reference< XInterface > xLoggerSettings( xNodeFactory->createInstance(), UNO_QUERY_THROW );
                xAllSettings->insertByName( sLoggerName, makeAny( xLoggerSettings ) );
                Reference< XChangesBatch > xChanges( xAllSettings, UNO_QUERY_THROW );
                xChanges->commitChanges();
            }

            // actually read and forward the settings
            Reference< XNameAccess > xLoggerSettings( xAllSettings->getByName( sLoggerName ), UNO_QUERY_THROW );

            // the log level
            sal_Int32 nLogLevel( LogLevel::OFF );
            OSL_VERIFY( xLoggerSettings->getByName("LogLevel") >>= nLogLevel );
            _rxLogger->setLevel( nLogLevel );

            // the default handler, if any
            Reference< XInterface > xUntyped( lcl_createInstanceFromSetting_throw( _rContext, _rxLogger, xLoggerSettings, "DefaultHandler", "HandlerSettings", &lcl_transformFileHandlerSettings_nothrow ) );
            if ( !xUntyped.is() )
                // no handler -> we're done
                return;
            Reference< XLogHandler > xHandler( xUntyped, UNO_QUERY_THROW );
            _rxLogger->addLogHandler( xHandler );

            // The newly created handler might have an own (default) level. Ensure that it uses
            // the same level as the logger.
            xHandler->setLevel( nLogLevel );

            // the default formatter for the handler
            xUntyped = lcl_createInstanceFromSetting_throw( _rContext, _rxLogger, xLoggerSettings, "DefaultFormatter", "FormatterSettings" );
            if ( !xUntyped.is() )
                // no formatter -> we're done
                return;
            Reference< XLogFormatter > xFormatter( xUntyped, UNO_QUERY_THROW );
            xHandler->setFormatter( xFormatter );

            // TODO: we could first create the formatter, then the handler. This would allow
            // passing the formatter as value in the component context, so the handler would
            // not create an own default formatter
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

//........................................................................
} // namespace logging
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
