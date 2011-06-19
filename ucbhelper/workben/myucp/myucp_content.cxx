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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "osl/diagnose.h"

#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/XPropertyAccess.hpp"
#include "com/sun/star/lang/IllegalAccessException.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/ucb/XCommandInfo.hpp"
#include "com/sun/star/ucb/XPersistentPropertySet.hpp"
#include "ucbhelper/contentidentifier.hxx"
#include "ucbhelper/propertyvalueset.hxx"
#include "ucbhelper/cancelcommandexecution.hxx"

#include "myucp_content.hxx"
#include "myucp_provider.hxx"

#ifdef IMPLEMENT_COMMAND_INSERT
#include "com/sun/star/ucb/InsertCommandArgument.hpp"
#include "com/sun/star/ucb/MissingInputStreamException.hpp"
#include "com/sun/star/ucb/MissingPropertiesException.hpp"
#endif
#ifdef IMPLEMENT_COMMAND_OPEN
#include "com/sun/star/io/XOutputStream.hpp"
#include "com/sun/star/io/XActiveDataSink.hpp"
#include "com/sun/star/ucb/OpenCommandArgument2.hpp"
#include "com/sun/star/ucb/OpenMode.hpp"
#include "com/sun/star/ucb/UnsupportedDataSinkException.hpp"
#include "com/sun/star/ucb/UnsupportedOpenModeException.hpp"
#include "myucp_resultset.hxx"
#endif

using namespace com::sun::star;

// @@@ Adjust namespace name.
using namespace myucp;

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

Content::Content( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
                  ::ucbhelper::ContentProviderImplHelper* pProvider,
                  const uno::Reference< ucb::XContentIdentifier >& Identifier )
: ContentImplHelper( rxSMgr, pProvider, Identifier )
{
    // @@@ Fill m_aProps here or implement lazy evaluation logic for this.
    // m_aProps.aTitle       =
    // m_aprops.aContentType =
    // m_aProps.bIsDocument  =
    // m_aProps.bIsFolder    =
}

//=========================================================================
// virtual
Content::~Content()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// virtual
void SAL_CALL Content::acquire()
    throw()
{
    ContentImplHelper::acquire();
}

//=========================================================================
// virtual
void SAL_CALL Content::release()
    throw()
{
    ContentImplHelper::release();
}

//=========================================================================
// virtual
uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet;

     return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface( rType );
}

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_COMMON_IMPL( Content );

//=========================================================================
// virtual
uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
    throw( uno::RuntimeException )
{
    // @@@ Add own interfaces.

    static cppu::OTypeCollection* pCollection = 0;

    if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pCollection )
          {
            static cppu::OTypeCollection aCollection(
                CPPU_TYPE_REF( lang::XTypeProvider ),
                CPPU_TYPE_REF( lang::XServiceInfo ),
                CPPU_TYPE_REF( lang::XComponent ),
                CPPU_TYPE_REF( ucb::XContent ),
                CPPU_TYPE_REF( ucb::XCommandProcessor ),
                CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
                CPPU_TYPE_REF( beans::XPropertyContainer ),
                CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                CPPU_TYPE_REF( container::XChild ) );
              pCollection = &aCollection;
        }
    }

    return (*pCollection).getTypes();
}

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL Content::getImplementationName()
    throw( uno::RuntimeException )
{
    // @@@ Adjust implementation name.
    // Prefix with reversed company domain name.
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.myucp.Content"));
}

//=========================================================================
// virtual
uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    // @@@ Adjust macro name.
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
            = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( MYUCP_CONTENT_SERVICE_NAME ));
    return aSNS;
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL Content::getContentType()
    throw( uno::RuntimeException )
{
    // @@@ Adjust macro name ( def in myucp_provider.hxx ).
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( MYUCP_CONTENT_TYPE ));
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /* CommandId */,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception,
           ucb::CommandAbortedException,
           uno::RuntimeException )
{
    uno::Any aRet;

    if ( aCommand.Name.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( "getPropertyValues" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertyValues
        //////////////////////////////////////////////////////////////////

        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ::ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= getPropertyValues( Properties, Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "setPropertyValues" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // setPropertyValues
        //////////////////////////////////////////////////////////////////

        uno::Sequence< beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ::ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.getLength() )
        {
            OSL_FAIL( "No properties!" );
            ::ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= setPropertyValues( aProperties, Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "getPropertySetInfo" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getPropertySetInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "getCommandInfo" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getCommandInfo( Environment );
    }
#ifdef IMPLEMENT_COMMAND_OPEN
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "open" ) ) )
    {
        ucb::OpenCommandArgument2 aOpenCommand;
          if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ::ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        sal_Bool bOpenFolder =
            ( ( aOpenCommand.Mode == ucb::OpenMode::ALL ) ||
              ( aOpenCommand.Mode == ucb::OpenMode::FOLDERS ) ||
              ( aOpenCommand.Mode == ucb::OpenMode::DOCUMENTS ) );

        if ( bOpenFolder /*&& isFolder( Environment )*/ )
        {
            // open as folder - return result set

            uno::Reference< ucb::XDynamicResultSet > xSet
                            = new DynamicResultSet( m_xSMgr,
                                                    this,
                                                    aOpenCommand,
                                                    Environment );
            aRet <<= xSet;
          }

        if ( aOpenCommand.Sink.is() )
        {
            // Open document - supply document data stream.

            // Check open mode
            if ( ( aOpenCommand.Mode
                    == ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
                 ( aOpenCommand.Mode
                    == ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
            {
                // Unsupported.
                ::ucbhelper::cancelCommandExecution(
                    uno::makeAny( ucb::UnsupportedOpenModeException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    sal_Int16( aOpenCommand.Mode ) ) ),
                    Environment );
                // Unreachable
            }


            rtl::OUString aURL = m_xIdentifier->getContentIdentifier();
            uno::Reference< io::XOutputStream > xOut
                = uno::Reference< io::XOutputStream >(
                    aOpenCommand.Sink, uno::UNO_QUERY );
            if ( xOut.is() )
              {
                // @@@ write data into xOut
              }
            else
              {
                uno::Reference< io::XActiveDataSink > xDataSink(
                        aOpenCommand.Sink, uno::UNO_QUERY );
                  if ( xDataSink.is() )
                {
                    uno::Reference< io::XInputStream > xIn
                        /* @@@ your XInputStream + XSeekable impl. object */;
                    xDataSink->setInputStream( xIn );
                }
                  else
                {
                    // Note: aOpenCommand.Sink may contain an XStream
                    //       implementation. Support for this type of
                    //       sink is optional...
                    ::ucbhelper::cancelCommandExecution(
                        uno::makeAny( ucb::UnsupportedDataSinkException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                aOpenCommand.Sink ) ),
                        Environment );
                    // Unreachable
                }
              }
        }
    }
#endif // IMPLEMENT_COMMAND_OPEN

#ifdef IMPLEMENT_COMMAND_INSERT
    else if ( aCommand.Name.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "insert" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // insert
        //////////////////////////////////////////////////////////////////

        ucb::InsertCommandArgument arg;
          if ( !( aCommand.Argument >>= arg ) )
        {
              OSL_FAIL( "Wrong argument type!" );
            ::ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

          insert( arg.Data, arg.ReplaceExisting, Environment );
    }
#endif // IMPLEMENT_COMMAND_INSERT

#ifdef IMPLEMENT_COMMAND_DELETE
    else if ( aCommand.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "delete" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // delete
        //////////////////////////////////////////////////////////////////

        sal_Bool bDeletePhysical = sal_False;
        aCommand.Argument >>= bDeletePhysical;
        destroy( bDeletePhysical );

        // Remove own and all children's Additional Core Properties.
        removeAdditionalPropertySet( sal_True );

        // Remove own and all childrens(!) persistent data.
//      removeData();
    }
#endif // IMPLEMENT_COMMAND_DELETE
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unsupported command
        //////////////////////////////////////////////////////////////////

        OSL_FAIL( "Content::execute - unsupported command!" );

        ::ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL Content::abort( sal_Int32 )
    throw( uno::RuntimeException )
{
    // @@@ Implement logic to abort running commands, if this makes
    //     sense for your content.
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

// virtual
rtl::OUString Content::getParentURL()
{
    rtl::OUString aURL = m_xIdentifier->getContentIdentifier();

    // @@@ Extract URL of parent from aURL and return it...

    return rtl::OUString();
}

//=========================================================================
// static
uno::Reference< sdbc::XRow > Content::getPropertyValues(
            const uno::Reference< lang::XMultiServiceFactory >& rSMgr,
            const uno::Sequence< beans::Property >& rProperties,
            const ContentProperties& rData,
            const rtl::Reference<
                ::ucbhelper::ContentProviderImplHelper >& rProvider,
            const rtl::OUString& rContentId )
{
    // Note: Empty sequence means "get values of all supported properties".

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow
        = new ::ucbhelper::PropertyValueSet( rSMgr );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        uno::Reference< beans::XPropertySet > xAdditionalPropSet;
        sal_Bool bTriedToGetAdditonalPropSet = sal_False;

        const beans::Property* pProps = rProperties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::Property& rProp = pProps[ n ];

            // Process Core properties.

            if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
            {
                xRow->appendString ( rProp, rData.aContentType );
            }
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
            {
                xRow->appendString ( rProp, rData.aTitle );
            }
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
            {
                xRow->appendBoolean( rProp, rData.bIsDocument );
            }
            else if ( rProp.Name.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
            {
                xRow->appendBoolean( rProp, rData.bIsFolder );
            }
            else
            {
                // @@@ Note: If your data source supports adding/removing
                //     properties, you should implement the interface
                //     XPropertyContainer by yourself and supply your own
                //     logic here. The base class uses the service
                //     "com.sun.star.ucb.Store" to maintain Additional Core
                //     properties. But using server functionality is preferred!

                // Not a Core Property! Maybe it's an Additional Core Property?!

                if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet
                        = uno::Reference< beans::XPropertySet >(
                            rProvider->getAdditionalPropertySet( rContentId,
                                                                 sal_False ),
                            uno::UNO_QUERY );
                    bTriedToGetAdditonalPropSet = sal_True;
                }

                if ( xAdditionalPropSet.is() )
                {
                    if ( !xRow->appendPropertySetValue(
                                                xAdditionalPropSet,
                                                rProp ) )
                    {
                        // Append empty entry.
                        xRow->appendVoid( rProp );
                    }
                }
                else
                {
                    // Append empty entry.
                    xRow->appendVoid( rProp );
                }
            }
        }
    }
    else
    {
        // Append all Core Properties.
        xRow->appendString (
            beans::Property( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ContentType")),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.aContentType );
        xRow->appendString (
            beans::Property( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title")),
                      -1,
                      getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                      beans::PropertyAttribute::BOUND ),
            rData.aTitle );
        xRow->appendBoolean(
            beans::Property( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsDocument")),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.bIsDocument );
        xRow->appendBoolean(
            beans::Property( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsFolder")),
                      -1,
                      getCppuBooleanType(),
                      beans::PropertyAttribute::BOUND
                        | beans::PropertyAttribute::READONLY ),
            rData.bIsFolder );

        // @@@ Append other properties supported directly.

        // @@@ Note: If your data source supports adding/removing
        //     properties, you should implement the interface
        //     XPropertyContainer by yourself and supply your own
        //     logic here. The base class uses the service
        //     "com.sun.star.ucb.Store" to maintain Additional Core
        //     properties. But using server functionality is preferred!

        // Append all Additional Core Properties.

        uno::Reference< beans::XPropertySet > xSet(
            rProvider->getAdditionalPropertySet( rContentId, sal_False ),
            uno::UNO_QUERY );
        xRow->appendPropertySet( xSet );
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
}

//=========================================================================
uno::Reference< sdbc::XRow > Content::getPropertyValues(
            const uno::Sequence< beans::Property >& rProperties,
            const uno::Reference< ucb::XCommandEnvironment >& /* xEnv */)
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    return getPropertyValues( m_xSMgr,
                              rProperties,
                              m_aProps,
                              rtl::Reference<
                                ::ucbhelper::ContentProviderImplHelper >(
                                    m_xProvider.get() ),
                              m_xIdentifier->getContentIdentifier() );
}

//=========================================================================
uno::Sequence< uno::Any > Content::setPropertyValues(
            const uno::Sequence< beans::PropertyValue >& rValues,
            const uno::Reference< ucb::XCommandEnvironment >& /* xEnv */)
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    beans::PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further        = sal_False;
//  aEvent.PropertyName   =
    aEvent.PropertyHandle = -1;
//  aEvent.OldValue       =
//  aEvent.NewValue       =

    const beans::PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();

    uno::Reference< ucb::XPersistentPropertySet > xAdditionalPropSet;
    sal_Bool bTriedToGetAdditonalPropSet = sal_False;

    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = pValues[ n ];

        if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "Property is read-only!" )),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "Property is read-only!" )),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "Property is read-only!" )),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rValue.Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
        {
            rtl::OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                if ( aNewValue != m_aProps.aTitle )
                {
                    aEvent.PropertyName = rValue.Name;
                    aEvent.OldValue     = uno::makeAny( m_aProps.aTitle );
                    aEvent.NewValue     = uno::makeAny( aNewValue );

                    aChanges.getArray()[ nChanged ] = aEvent;

                    m_aProps.aTitle = aNewValue;
                    nChanged++;
                }
                else
                {
                    // Old value equals new value. No error!
                }
            }
            else
            {
                aRet[ n ] <<= beans::IllegalTypeException(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                    "Property value has wrong type!" )),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }

        else
        {
            // @@@ Note: If your data source supports adding/removing
            //     properties, you should implement the interface
            //     XPropertyContainer by yourself and supply your own
            //     logic here. The base class uses the service
            //     "com.sun.star.ucb.Store" to maintain Additional Core
            //     properties. But using server functionality is preferred!

            // Not a Core Property! Maybe it's an Additional Core Property?!

            if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
            {
                xAdditionalPropSet = getAdditionalPropertySet( sal_False );
                bTriedToGetAdditonalPropSet = sal_True;
            }

            if ( xAdditionalPropSet.is() )
            {
                try
                {
                    uno::Any aOldValue
                        = xAdditionalPropSet->getPropertyValue( rValue.Name );
                    if ( aOldValue != rValue.Value )
                    {
                        xAdditionalPropSet->setPropertyValue(
                                                rValue.Name, rValue.Value );

                        aEvent.PropertyName = rValue.Name;
                        aEvent.OldValue     = aOldValue;
                        aEvent.NewValue     = rValue.Value;

                        aChanges.getArray()[ nChanged ] = aEvent;
                        nChanged++;
                    }
                    else
                    {
                        // Old value equals new value. No error!
                    }
                }
                catch ( beans::UnknownPropertyException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( lang::WrappedTargetException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( beans::PropertyVetoException const & e )
                {
                    aRet[ n ] <<= e;
                }
                catch ( lang::IllegalArgumentException const & e )
                {
                    aRet[ n ] <<= e;
                }
            }
            else
            {
                aRet[ n ] <<= uno::Exception(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                    "No property set for storing the value!" )),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
        }
    }

    if ( nChanged > 0 )
    {
        // @@@ Save changes.
//      storeData();

        aGuard.clear();
        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }

    return aRet;
}

#ifdef IMPLEMENT_COMMAND_INSERT

//=========================================================================
void Content::queryChildren( ContentRefList& rChildren )
{
    // @@@ Adapt method to your URL scheme...

    // Obtain a list with a snapshot of all currently instanciated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ::ucbhelper::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    ::rtl::OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nPos = aURL.lastIndexOf( '/' );

    if ( nPos != ( aURL.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aURL += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    }

    sal_Int32 nLen = aURL.getLength();

    ::ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

    while ( it != end )
    {
        ::ucbhelper::ContentImplHelperRef xChild = (*it);
        ::rtl::OUString aChildURL
              = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.compareTo( aURL, nLen ) == 0 ) )
        {
            nPos = aChildURL.indexOf( '/', nLen );

            if ( ( nPos == -1 ) ||
                 ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                // No further slashes / only a final slash. It's a child!
                rChildren.push_back(
                    ContentRef(
                        static_cast< Content * >( xChild.get() ) ) );
            }
        }
        ++it;
    }
}

//=========================================================================
void Content::insert(
        const uno::Reference< io::XInputStream > & xInputStream,
        sal_Bool bReplaceExisting,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    // Check, if all required properties were set.

    bool bNeedInputStream = true; // @@@ adjust to real requirements
    if ( bNeedInputStream && !xInputStream.is() )
    {
        OSL_FAIL( "Content::insert - No data stream!" );

        ::ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::MissingInputStreamException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    // Assemble new content identifier...

    uno::Reference< ucb::XContentIdentifier > xId /* @@@ create content identifier */;

    // Fail, if a resource with given id already exists.
    if ( !bReplaceExisting /*&& hasData( xId ) @@@ impl for hasData() */ )
    {
        uno::Any aProps
            = uno::makeAny( beans::PropertyValue(
                                  rtl::OUString(
                                      RTL_CONSTASCII_USTRINGPARAM( "Uri" ) ),
                                  -1,
                                  uno::makeAny( xId->getContentIdentifier() ),
                                  beans::PropertyState_DIRECT_VALUE ) );
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_ALREADY_EXISTING,
            uno::Sequence< uno::Any >(&aProps, 1),
            Environment,
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content already existing!!")),
            this );
        // Unreachable
    }

    m_xIdentifier = xId;

//  @@@
//  storeData();

    aGuard.clear();
    inserted();
}

#endif // IMPLEMENT_COMMAND_INSERT

#ifdef IMPLEMENT_COMMAND_DELETE

//=========================================================================
void Content::destroy( sal_Bool bDeletePhysical )
    throw( uno::Exception )
{
    // @@@ take care about bDeletePhysical -> trashcan support

    uno::Reference< ucb::XContent > xThis = this;

    deleted();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Process instanciated children...

    ContentRefList aChildren;
    queryChildren( aChildren );

    ContentRefList::const_iterator it  = aChildren.begin();
    ContentRefList::const_iterator end = aChildren.end();

    while ( it != end )
    {
        (*it)->destroy( bDeletePhysical );
        ++it;
    }
}

#endif // IMPLEMENT_COMMAND_DELETE


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
