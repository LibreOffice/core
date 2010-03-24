/*************************************************************************
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

#include "precompiled_ucb.hxx"

#include "ucpext_content.hxx"

#include "ucpext_content.hxx"
#include "ucpext_provider.hxx"
#include "ucpext_resultset.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
/** === end UNO includes === **/

#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/string.hxx>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::ucb::XContentIdentifier;
    using ::com::sun::star::ucb::IllegalIdentifierException;
    using ::com::sun::star::ucb::XContent;
    using ::com::sun::star::ucb::XCommandEnvironment;
    using ::com::sun::star::ucb::Command;
    using ::com::sun::star::ucb::CommandAbortedException;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::ucb::OpenCommandArgument2;
    using ::com::sun::star::ucb::XDynamicResultSet;
    using ::com::sun::star::ucb::UnsupportedOpenModeException;
    using ::com::sun::star::io::XOutputStream;
    using ::com::sun::star::io::XActiveDataSink;
    using ::com::sun::star::io::XInputStream;
    using ::com::sun::star::ucb::UnsupportedDataSinkException;
    using ::com::sun::star::ucb::UnsupportedCommandException;
    using ::com::sun::star::sdbc::XRow;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::lang::IllegalAccessException;
    using ::com::sun::star::ucb::CommandInfo;
    /** === end UNO using === **/
    namespace OpenMode = ::com::sun::star::ucb::OpenMode;
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    //==================================================================================================================
    //= Content
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    Content::Content( const Reference< XMultiServiceFactory >& i_rORB, ::ucbhelper::ContentProviderImplHelper* i_pProvider,
                      const Reference< XContentIdentifier >& i_rIdentifier )
        :Content_Base( i_rORB, i_pProvider, i_rIdentifier )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Content::~Content()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL Content::getImplementationName() throw( RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.ucp.ext.Content" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL Content::getSupportedServiceNames() throw( RuntimeException )
    {
        Sequence< ::rtl::OUString > aServiceNames(2);
        aServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.Content" ) );
        aServiceNames[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.ExtensionContent" ) );
        return aServiceNames;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL Content::getContentType() throw( RuntimeException )
    {
        return m_aProps.aContentType;
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL Content::execute( const Command& aCommand, sal_Int32 /* CommandId */, const Reference< XCommandEnvironment >& i_rEvironment )
        throw( Exception, CommandAbortedException, RuntimeException )
    {
        Any aRet;

        if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "getPropertyValues" ) ) )
        {
            Sequence< Property > Properties;
            if ( !( aCommand.Argument >>= Properties ) )
            {
                ::ucbhelper::cancelCommandExecution( makeAny( IllegalArgumentException(
                    ::rtl::OUString(), *this, -1 ) ),
                    i_rEvironment );
                // unreachable
            }

            aRet <<= getPropertyValues( Properties, i_rEvironment );
        }
        else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "setPropertyValues" ) ) )
        {
            Sequence< PropertyValue > aProperties;
            if ( !( aCommand.Argument >>= aProperties ) )
            {
                ::ucbhelper::cancelCommandExecution( makeAny( IllegalArgumentException(
                    ::rtl::OUString(), *this, -1 ) ),
                    i_rEvironment );
                // unreachable
            }

            if ( !aProperties.getLength() )
            {
                ::ucbhelper::cancelCommandExecution( makeAny( IllegalArgumentException(
                    ::rtl::OUString(), *this, -1 ) ),
                    i_rEvironment );
                // unreachable
            }

            aRet <<= setPropertyValues( aProperties, i_rEvironment );
        }
        else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "getPropertySetInfo" ) ) )
        {
            // implemented by base class.
            aRet <<= getPropertySetInfo( i_rEvironment );
        }
        else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "getCommandInfo" ) ) )
        {
            // implemented by base class.
            aRet <<= getCommandInfo( i_rEvironment );
        }
        else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "open" ) ) )
        {
            OpenCommandArgument2 aOpenCommand;
              if ( !( aCommand.Argument >>= aOpenCommand ) )
            {
                ::ucbhelper::cancelCommandExecution( makeAny( IllegalArgumentException(
                    ::rtl::OUString(), *this, -1 ) ),
                    i_rEvironment );
                // unreachable
            }

            sal_Bool bOpenFolder =
                ( ( aOpenCommand.Mode == OpenMode::ALL ) ||
                  ( aOpenCommand.Mode == OpenMode::FOLDERS ) ||
                  ( aOpenCommand.Mode == OpenMode::DOCUMENTS ) );


            if ( bOpenFolder && m_aProps.bIsFolder )
            {
                Reference< XDynamicResultSet > xSet = new ResultSet(
                    m_xSMgr, this, aOpenCommand, i_rEvironment );
                aRet <<= xSet;
              }

            if ( aOpenCommand.Sink.is() )
            {
                // Open document - supply document data stream.

                // Check open mode
                if  (   ( aOpenCommand.Mode == OpenMode::DOCUMENT_SHARE_DENY_NONE )
                    ||  ( aOpenCommand.Mode == OpenMode::DOCUMENT_SHARE_DENY_WRITE )
                    )
                {
                    // unsupported.
                    ::ucbhelper::cancelCommandExecution( makeAny( UnsupportedOpenModeException(
                        ::rtl::OUString(), *this, sal_Int16( aOpenCommand.Mode ) ) ),
                        i_rEvironment );
                    // unreachable
                }

                ::rtl::OUString aURL = m_xIdentifier->getContentIdentifier();
                Reference< XOutputStream > xOut( aOpenCommand.Sink, UNO_QUERY );
                if ( xOut.is() )
                  {
                    // TODO: write data into xOut
                  }
                else
                  {
                    Reference< XActiveDataSink > xDataSink( aOpenCommand.Sink, UNO_QUERY );
                      if ( xDataSink.is() )
                    {
                        Reference< XInputStream > xIn
                            /* @@@ your XInputStream + XSeekable impl. object */;
                        // TODO
                        xDataSink->setInputStream( xIn );
                    }
                      else
                    {
                        // Note: aOpenCommand.Sink may contain an XStream
                        //       implementation. Support for this type of
                        //       sink is optional...
                        ::ucbhelper::cancelCommandExecution( makeAny( UnsupportedDataSinkException(
                            ::rtl::OUString(), *this, aOpenCommand.Sink ) ),
                            i_rEvironment );
                        // unreachable
                    }
                  }
            }
        }

        else
        {
            ::ucbhelper::cancelCommandExecution( makeAny( UnsupportedCommandException(
                ::rtl::OUString(), *this ) ),
                i_rEvironment );
            // unreachable
        }

        return aRet;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL Content::abort( sal_Int32 ) throw( RuntimeException )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString Content::escapeIdentifier( const ::rtl::OUString& i_rIdentifier )
    {
        const ::rtl::OUString sQuoteQuotes = ::comphelper::string::searchAndReplaceAllAsciiWithAscii(
            i_rIdentifier, "%", "%%" );
        const ::rtl::OUString sQuoteSlashes = ::comphelper::string::searchAndReplaceAllAsciiWithAscii(
            i_rIdentifier, "/", "%47" );
        return sQuoteSlashes;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool Content::denotesRootContent( const Reference< XContentIdentifier >& i_rIdentifier )
    {
        const sal_Char* pScheme = "vnd.oracle.ooo.extension";
        const sal_Int32 nSchemeLength = sizeof( "vnd.oracle.ooo.extension" ) - 1;
        const ::rtl::OUString sIdentifier( i_rIdentifier->getContentIdentifier() );
        ENSURE_OR_RETURN_FALSE( sIdentifier.matchAsciiL( pScheme, nSchemeLength ), "illegal content URL" );
        return sIdentifier.copy( nSchemeLength ).equalsAsciiL( ":/", 2 );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString Content::getParentURL()
    {
        const ::rtl::OUString sURL = m_xIdentifier->getContentIdentifier();
        const ::rtl::OUString sParentURL( sURL.copy( 0, sURL.lastIndexOf( '/' ) + 1 ) );
        return sParentURL;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XRow > Content::getPropertyValues( const Reference< XMultiServiceFactory >& i_rORB,
                const Sequence< Property >& i_rProperties, const ContentProperties& i_rData )
    {
        // note: empty sequence means "get values of all supported properties".
        ::rtl::Reference< ::ucbhelper::PropertyValueSet > xRow = new ::ucbhelper::PropertyValueSet( i_rORB );

        const sal_Int32 nCount = i_rProperties.getLength();
        if ( nCount )
        {
            Reference< XPropertySet > xAdditionalPropSet;

            const Property* pProps = i_rProperties.getConstArray();
            for ( sal_Int32 n = 0; n < nCount; ++n )
            {
                const Property& rProp = pProps[ n ];

                // Process Core properties.
                if ( rProp.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
                {
                    xRow->appendString ( rProp, i_rData.aContentType );
                }
                else if ( rProp.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
                {
                    xRow->appendString ( rProp, i_rData.aTitle );
                }
                else if ( rProp.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
                {
                    xRow->appendBoolean( rProp, i_rData.bIsDocument );
                }
                else if ( rProp.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
                {
                    xRow->appendBoolean( rProp, i_rData.bIsFolder );
                }
                else
                {
                    // append empty entry.
                    xRow->appendVoid( rProp );
                }
            }
        }
        else
        {
            // Append all Core Properties.
            xRow->appendString ( Property( ::rtl::OUString::createFromAscii( "ContentType" ),
                          -1,
                          getCppuType( static_cast< const ::rtl::OUString * >( 0 ) ),
                          PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
                i_rData.aContentType );
            xRow->appendString ( Property( ::rtl::OUString::createFromAscii( "Title" ),
                          -1,
                          getCppuType( static_cast< const ::rtl::OUString * >( 0 ) ),
                          PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
                i_rData.aTitle );
            xRow->appendBoolean( Property( ::rtl::OUString::createFromAscii( "IsDocument" ),
                          -1,
                          getCppuBooleanType(),
                          PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
                i_rData.bIsDocument );
            xRow->appendBoolean( Property( ::rtl::OUString::createFromAscii( "IsFolder" ),
                          -1,
                          getCppuBooleanType(),
                          PropertyAttribute::BOUND | PropertyAttribute::READONLY ),
                i_rData.bIsFolder );
        }

        return Reference< XRow >( xRow.get() );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XRow > Content::getPropertyValues( const Sequence< Property >& i_rProperties, const Reference< XCommandEnvironment >& /* xEnv */)
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        return getPropertyValues( m_xSMgr,
                                  i_rProperties,
                                  m_aProps );
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< Any > Content::setPropertyValues( const Sequence< PropertyValue >& i_rValues, const Reference< XCommandEnvironment >& /* xEnv */)
    {
        ::osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

        Sequence< Any > aRet( i_rValues.getLength() );
        Sequence< PropertyChangeEvent > aChanges( i_rValues.getLength() );

        PropertyChangeEvent aEvent;
        aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
        aEvent.Further        = sal_False;
        aEvent.PropertyHandle = -1;

        const PropertyValue* pValues = i_rValues.getConstArray();
        const sal_Int32 nCount = i_rValues.getLength();

        for ( sal_Int32 n = 0; n < nCount; ++n, ++pValues )
        {
            // all our properties are read-only ...
            aRet[ n ] <<= IllegalAccessException( ::rtl::OUString::createFromAscii( "property is read-only." ), *this );
        }

        return aRet;
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< CommandInfo > Content::getCommands( const Reference< XCommandEnvironment > & /*xEnv*/ )
    {
        sal_uInt32 nCommandCount = 5;
        static const CommandInfo aCommandInfoTable[] =
        {
            ///////////////////////////////////////////////////////////////
            // Mandatory commands
            ///////////////////////////////////////////////////////////////
            CommandInfo(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            CommandInfo(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
                -1,
                getCppuVoidType()
            ),
            CommandInfo(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
                -1,
                getCppuType(
                    static_cast< Sequence< Property > * >( 0 ) )
            ),
            CommandInfo(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
                -1,
                getCppuType(
                    static_cast< Sequence< PropertyValue > * >( 0 ) )
            )
            ///////////////////////////////////////////////////////////////
            // Optional standard commands
            ///////////////////////////////////////////////////////////////

    #ifdef IMPLEMENT_COMMAND_OPEN
            , CommandInfo(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
                -1,
                getCppuType( static_cast< OpenCommandArgument2 * >( 0 ) )
            )
    #endif
        };

        return Sequence< CommandInfo >( aCommandInfoTable, nCommandCount );
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< Property > Content::getProperties( const Reference< XCommandEnvironment > & /*xEnv*/ )
    {
        static Property aProperties[] =
        {
            Property(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
                -1,
                getCppuType( static_cast< const ::rtl::OUString * >( 0 ) ),
                PropertyAttribute::BOUND | PropertyAttribute::READONLY
            ),
            Property(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
                -1,
                getCppuBooleanType(),
                PropertyAttribute::BOUND | PropertyAttribute::READONLY
            ),
            Property(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
                -1,
                getCppuBooleanType(),
                PropertyAttribute::BOUND | PropertyAttribute::READONLY
            ),
            Property(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                -1,
                getCppuType( static_cast< const ::rtl::OUString * >( 0 ) ),
                PropertyAttribute::BOUND | PropertyAttribute::READONLY
            )
        };
        return Sequence< Property >( aProperties, sizeof( aProperties ) / sizeof( aProperties[0] ) );
    }

//......................................................................................................................
} } }   // namespace ucp::ext
//......................................................................................................................
