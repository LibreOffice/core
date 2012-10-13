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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                  TODO
 **************************************************************************

 *************************************************************************/

#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include "osl/doublecheckedlocking.h"
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/simpleinteractionrequest.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <com/sun/star/beans/PropertySetInfoChangeEvent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/task/PasswordContainerInteractionHandler.hpp>
#include <com/sun/star/ucb/CommandEnvironment.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include "com/sun/star/ucb/InteractiveLockingLockedException.hpp"
#include "com/sun/star/ucb/InteractiveLockingLockExpiredException.hpp"
#include "com/sun/star/ucb/InteractiveLockingNotLockedException.hpp"
#include <com/sun/star/ucb/InteractiveNetworkConnectException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkGeneralException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkReadException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkWriteException.hpp>
#include <com/sun/star/ucb/MissingInputStreamException.hpp>
#include <com/sun/star/ucb/MissingPropertiesException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "webdavcontent.hxx"
#include "webdavprovider.hxx"
#include "webdavresultset.hxx"
#include "ContentProperties.hxx"
#include "SerfUri.hxx"
#include "UCBDeadPropertyValue.hxx"

using namespace com::sun::star;
using namespace http_dav_ucp;

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
// ctr for content on an existing webdav resource
Content::Content(
          const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
          ContentProvider* pProvider,
          const uno::Reference< ucb::XContentIdentifier >& Identifier,
          rtl::Reference< DAVSessionFactory > const & rSessionFactory )
  throw ( ucb::ContentCreationException )
: ContentImplHelper( rxSMgr, pProvider, Identifier ),
  m_eResourceType( UNKNOWN ),
  m_pProvider( pProvider ),
  m_bTransient( false ),
  m_bCollection( false ),
  m_bDidGetOrHead( false )
{
    try
    {
        m_xResAccess.reset( new DAVResourceAccess(
                rxSMgr,
                rSessionFactory,
                Identifier->getContentIdentifier() ) );

        SerfUri aURI( Identifier->getContentIdentifier() );
        m_aEscapedTitle = aURI.GetPathBaseName();
    }
    catch ( DAVException const & )
    {
        throw ucb::ContentCreationException();
    }
}

//=========================================================================
// ctr for content on an non-existing webdav resource
Content::Content(
            const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
            ContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            rtl::Reference< DAVSessionFactory > const & rSessionFactory,
            sal_Bool isCollection )
  throw ( ucb::ContentCreationException )
: ContentImplHelper( rxSMgr, pProvider, Identifier ),
  m_eResourceType( UNKNOWN ),
  m_pProvider( pProvider ),
  m_bTransient( true ),
  m_bCollection( isCollection ),
  m_bDidGetOrHead( false )
{
    try
    {
        m_xResAccess.reset( new DAVResourceAccess(
            rxSMgr, rSessionFactory, Identifier->getContentIdentifier() ) );
    }
    catch ( DAVException const & )
    {
        throw ucb::ContentCreationException();
    }

    // Do not set m_aEscapedTitle here! Content::insert relays on this!!!
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
    throw( )
{
    ContentImplHelper::acquire();
}

//=========================================================================
// virtual
void SAL_CALL Content::release()
    throw( )
{
    ContentImplHelper::release();
}

//=========================================================================
// virtual
uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    // Note: isFolder may require network activities! So call it only
    //       if it is really necessary!!!
    uno::Any aRet = cppu::queryInterface(
        rType,
        static_cast< ucb::XContentCreator * >( this ) );
    if ( aRet.hasValue() )
    {
        try
        {
            uno::Reference< beans::XPropertySet > const xProps(
                m_xSMgr, uno::UNO_QUERY_THROW );
            uno::Reference< uno::XComponentContext > xCtx;
            xCtx.set( xProps->getPropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ),
                uno::UNO_QUERY_THROW );

            uno::Reference< task::XInteractionHandler > xIH(
                task::PasswordContainerInteractionHandler::create( xCtx ) );

            // Supply a command env to isFolder() that contains an interaction
            // handler that uses the password container service to obtain
            // credentials without displaying a password gui.

            uno::Reference< ucb::XCommandEnvironment > xCmdEnv(
                ucb::CommandEnvironment::create(
                   xCtx,
                   xIH,
                   uno::Reference< ucb::XProgressHandler >() ) );

            return isFolder( xCmdEnv ) ? aRet : uno::Any();
        }
        catch ( uno::RuntimeException const & )
        {
            throw;
        }
        catch ( uno::Exception const & )
        {
            return uno::Any();
        }
    }
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
    sal_Bool bFolder = sal_False;
    try
    {
        bFolder
            = isFolder( uno::Reference< ucb::XCommandEnvironment >() );
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
    }

    cppu::OTypeCollection * pCollection = 0;

    if ( bFolder )
    {
        static cppu::OTypeCollection* pFolderTypes = 0;

        pCollection = pFolderTypes;
        if ( !pCollection )
        {
            osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );

            pCollection = pFolderTypes;
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
                        CPPU_TYPE_REF( container::XChild ),
                        CPPU_TYPE_REF( ucb::XContentCreator ) ); // !!
                pCollection = &aCollection;
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                pFolderTypes = pCollection;
            }
        }
        else {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        }
    }
    else
    {
        static cppu::OTypeCollection* pDocumentTypes = 0;

        pCollection = pDocumentTypes;
        if ( !pCollection )
        {
            osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );

            pCollection = pDocumentTypes;
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
                OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
                pDocumentTypes = pCollection;
            }
        }
        else {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
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
    return rtl::OUString::createFromAscii(
                            "com.sun.star.comp.ucb.WebDAVContent" );
}

//=========================================================================
// virtual
uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
        = rtl::OUString::createFromAscii( WEBDAV_CONTENT_SERVICE_NAME );
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
    sal_Bool bFolder = sal_False;
    try
    {
        bFolder
            = isFolder( uno::Reference< ucb::XCommandEnvironment >() );
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
    }

    if ( bFolder )
        return rtl::OUString::createFromAscii( WEBDAV_COLLECTION_TYPE );

    return rtl::OUString::createFromAscii( WEBDAV_CONTENT_TYPE );
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

// virtual
uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception,
           ucb::CommandAbortedException,
           uno::RuntimeException )
{
    OSL_TRACE( ">>>>> Content::execute: start: command: %s, env: %s",
               rtl::OUStringToOString( aCommand.Name,
                                       RTL_TEXTENCODING_UTF8 ).getStr(),
               Environment.is() ? "present" : "missing" );

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
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
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
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.getLength() )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "No properties!" ),
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
        aRet <<= getPropertySetInfo( Environment,
                                     sal_False /* don't cache data */ );
    }
    else if ( aCommand.Name.equalsAsciiL(
                  RTL_CONSTASCII_STRINGPARAM( "getCommandInfo" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // getCommandInfo
        //////////////////////////////////////////////////////////////////

        // Note: Implemented by base class.
        aRet <<= getCommandInfo( Environment, sal_False );
    }
    else if ( aCommand.Name.equalsAsciiL(
                  RTL_CONSTASCII_STRINGPARAM( "open" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // open
        //////////////////////////////////////////////////////////////////

        ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet = open( aOpenCommand, Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                  RTL_CONSTASCII_STRINGPARAM( "insert" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // insert
        //////////////////////////////////////////////////////////////////

        ucb::InsertCommandArgument arg;
        if ( !( aCommand.Argument >>= arg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        insert( arg.Data, arg.ReplaceExisting, Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                  RTL_CONSTASCII_STRINGPARAM( "delete" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // delete
        //////////////////////////////////////////////////////////////////

        sal_Bool bDeletePhysical = sal_False;
        aCommand.Argument >>= bDeletePhysical;

//  KSO: Ignore parameter and destroy the content, if you don't support
//       putting objects into trashcan. ( Since we do not have a trash can
//       service yet (src603), you actually have no other choice. )
//      if ( bDeletePhysical )
//  {
        try
        {
            std::auto_ptr< DAVResourceAccess > xResAccess;
            {
                osl::Guard< osl::Mutex > aGuard( m_aMutex );
                xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
            }
            xResAccess->DESTROY( Environment );
            {
                osl::Guard< osl::Mutex > aGuard( m_aMutex );
                m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
            }
        }
        catch ( DAVException const & e )
        {
            cancelCommandExecution( e, Environment, sal_True );
            // Unreachable
        }
//      }

        // Propagate destruction.
        destroy( bDeletePhysical );

        // Remove own and all children's Additional Core Properties.
        removeAdditionalPropertySet( sal_True );
    }
    else if ( aCommand.Name.equalsAsciiL(
                  RTL_CONSTASCII_STRINGPARAM( "transfer" ) )
              && isFolder( Environment ) )
    {
        //////////////////////////////////////////////////////////////////
        // transfer
        //  ( Not available at documents )
        //////////////////////////////////////////////////////////////////

        ucb::TransferInfo transferArgs;
        if ( !( aCommand.Argument >>= transferArgs ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                  rtl::OUString::createFromAscii(
                                      "Wrong argument type!" ),
                                  static_cast< cppu::OWeakObject * >( this ),
                                  -1 ) ),
                Environment );
            // Unreachable
        }

        transfer( transferArgs, Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                  RTL_CONSTASCII_STRINGPARAM( "post" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // post
        //////////////////////////////////////////////////////////////////

        ucb::PostCommandArgument2 aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        post( aArg, Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                  RTL_CONSTASCII_STRINGPARAM( "lock" ) ) &&
              supportsExclusiveWriteLock( Environment ) )
    {
        //////////////////////////////////////////////////////////////////
        // lock
        //////////////////////////////////////////////////////////////////

        lock( Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                  RTL_CONSTASCII_STRINGPARAM( "unlock" ) ) &&
              supportsExclusiveWriteLock( Environment ) )
    {
        //////////////////////////////////////////////////////////////////
        // unlock
        //////////////////////////////////////////////////////////////////

        unlock( Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(
                  RTL_CONSTASCII_STRINGPARAM( "createNewContent" ) ) &&
              isFolder( Environment ) )
    {
        //////////////////////////////////////////////////////////////////
        // createNewContent
        //////////////////////////////////////////////////////////////////

        ucb::ContentInfo aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    rtl::OUString::createFromAscii(
                                        "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet = uno::makeAny( createNewContent( aArg ) );
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unsupported command
        //////////////////////////////////////////////////////////////////

        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                              aCommand.Name,
                              static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    OSL_TRACE( "<<<<< Content::execute: end: command: %s",
               rtl::OUStringToOString( aCommand.Name,
                                       RTL_TEXTENCODING_UTF8 ).getStr() );

    return aRet;
}

//=========================================================================
// virtual
void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
    throw( uno::RuntimeException )
{
    try
    {
        std::auto_ptr< DAVResourceAccess > xResAccess;
        {
            osl::MutexGuard aGuard( m_aMutex );
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        }
        xResAccess->abort();
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
        }
    }
    catch ( DAVException const & )
    {
        // abort failed!
    }
}

//=========================================================================
//
// XPropertyContainer methods.
//
//=========================================================================

// virtual
void SAL_CALL Content::addProperty( const rtl::OUString& Name,
                                    sal_Int16 Attributes,
                                    const uno::Any& DefaultValue )
    throw( beans::PropertyExistException,
           beans::IllegalTypeException,
           lang::IllegalArgumentException,
           uno::RuntimeException )
{
//    if ( m_bTransient )
//   @@@ ???

    if ( !Name.getLength() )
        throw lang::IllegalArgumentException();

    // Check property type.
    if ( !UCBDeadPropertyValue::supportsType( DefaultValue.getValueType() ) )
    {
        OSL_ENSURE( sal_False,
                    "Content::addProperty - Unsupported property type!" );
        throw beans::IllegalTypeException();
    }

    //////////////////////////////////////////////////////////////////////
    // Make sure a property with the requested name does not already
    // exist in dynamic and static(!) properties.
    //////////////////////////////////////////////////////////////////////

    // @@@ Need real command environment here, but where to get it from?
    //     XPropertyContainer interface should be replaced by
    //     XCommandProcessor commands!
    uno::Reference< ucb::XCommandEnvironment > xEnv;

    // Note: This requires network access!
    if ( getPropertySetInfo( xEnv, sal_False /* don't cache data */ )
             ->hasPropertyByName( Name ) )
    {
        // Property does already exist.
        throw beans::PropertyExistException();
    }

    //////////////////////////////////////////////////////////////////////
    // Add a new dynamic property.
    //////////////////////////////////////////////////////////////////////

    ProppatchValue aValue( PROPSET, Name, DefaultValue );

    std::vector< ProppatchValue > aProppatchValues;
    aProppatchValues.push_back( aValue );

    try
    {
        // Set property value at server.
        std::auto_ptr< DAVResourceAccess > xResAccess;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        }
        xResAccess->PROPPATCH( aProppatchValues, xEnv );
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
        }

        // Notify propertyset info change listeners.
        beans::PropertySetInfoChangeEvent evt(
            static_cast< cppu::OWeakObject * >( this ),
            Name,
            -1, // No handle available
            beans::PropertySetInfoChange::PROPERTY_INSERTED );
        notifyPropertySetInfoChange( evt );
    }
    catch ( DAVException const & e )
    {
        if ( e.getStatus() == SC_FORBIDDEN )
        {
            // Support for setting arbitrary dead properties is optional!

            // Store property locally.
            ContentImplHelper::addProperty(
                Name, Attributes, DefaultValue );
        }
        else
        {
            if ( shouldAccessNetworkAfterException( e ) )
            {
                try
                {
                    const ResourceType & rType = getResourceType( xEnv );
                    switch ( rType )
                    {
                    case UNKNOWN:
                    case DAV:
                        throw lang::IllegalArgumentException();

                    case NON_DAV:
                        // Store property locally.
                        ContentImplHelper::addProperty( Name,
                                                        Attributes,
                                                        DefaultValue );
                        break;

                    default:
                        OSL_ENSURE( sal_False,
                                    "Content::addProperty - "
                                    "Unsupported resource type!" );
                        break;
                    }
                }
                catch ( uno::Exception const & )
                {
                    OSL_ENSURE( sal_False,
                                "Content::addProperty - "
                                "Unable to determine resource type!" );
                }
            }
            else
            {
                OSL_ENSURE( sal_False,
                            "Content::addProperty - "
                            "Unable to determine resource type!" );
            }
        }
    }
}

//=========================================================================
// virtual
void SAL_CALL Content::removeProperty( const rtl::OUString& Name )
    throw( beans::UnknownPropertyException,
           beans::NotRemoveableException,
           uno::RuntimeException )
{
    // @@@ Need real command environment here, but where to get it from?
    //     XPropertyContainer interface should be replaced by
    //     XCommandProcessor commands!
    uno::Reference< ucb::XCommandEnvironment > xEnv;

#if 0
    // @@@ REMOVEABLE z.Z. nicht richtig an der PropSetInfo gesetzt!!!
    try
    {
        beans::Property aProp
            = getPropertySetInfo( xEnv, sal_False /* don't cache data */ )
                ->getPropertyByName( Name );

        if ( !( aProp.Attributes & beans::PropertyAttribute::REMOVEABLE ) )
        {
            // Not removeable!
            throw beans::NotRemoveableException();
        }
    }
    catch ( beans::UnknownPropertyException const & )
    {
        //OSL_ENSURE( sal_False, "removeProperty - Unknown property!" );
        throw;
    }
#endif

    //////////////////////////////////////////////////////////////////////
    // Try to remove property from server.
    //////////////////////////////////////////////////////////////////////

    try
    {
        std::vector< ProppatchValue > aProppatchValues;
        ProppatchValue aValue( PROPREMOVE, Name, uno::Any() );
        aProppatchValues.push_back( aValue );

        // Remove property value from server.
        std::auto_ptr< DAVResourceAccess > xResAccess;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        }
        xResAccess->PROPPATCH( aProppatchValues, xEnv );
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
        }

        // Notify propertyset info change listeners.
        beans::PropertySetInfoChangeEvent evt(
            static_cast< cppu::OWeakObject * >( this ),
            Name,
            -1, // No handle available
            beans::PropertySetInfoChange::PROPERTY_REMOVED );
        notifyPropertySetInfoChange( evt );
    }
    catch ( DAVException const & e )
    {
        if ( e.getStatus() == SC_FORBIDDEN )
        {
            // Support for setting arbitrary dead properties is optional!

            // Try to remove property from local store.
            ContentImplHelper::removeProperty( Name );
        }
        else
        {
            if ( shouldAccessNetworkAfterException( e ) )
            {
                try
                {
                    const ResourceType & rType = getResourceType( xEnv );
                    switch ( rType )
                    {
                        case UNKNOWN:
                        case DAV:
                            throw beans::UnknownPropertyException();

                        case NON_DAV:
                            // Try to remove property from local store.
                            ContentImplHelper::removeProperty( Name );
                            break;

                        default:
                            OSL_ENSURE( sal_False,
                                        "Content::removeProperty - "
                                        "Unsupported resource type!" );
                            break;
                    }
                }
                catch ( uno::Exception const & )
                {
                    OSL_ENSURE( sal_False,
                                "Content::removeProperty - "
                                "Unable to determine resource type!" );
                }
            }
            else
            {
                OSL_ENSURE( sal_False,
                            "Content::removeProperty - "
                            "Unable to determine resource type!" );
//                throw beans::UnknownPropertyException();
            }
        }
    }
}

//=========================================================================
//
// XContentCreator methods.
//
//=========================================================================

// virtual
uno::Sequence< ucb::ContentInfo > SAL_CALL
Content::queryCreatableContentsInfo()
    throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< ucb::ContentInfo > aSeq( 2 );

    // document.
    aSeq.getArray()[ 0 ].Type
        = rtl::OUString::createFromAscii( WEBDAV_CONTENT_TYPE );
    aSeq.getArray()[ 0 ].Attributes
        = ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
          | ucb::ContentInfoAttribute::KIND_DOCUMENT;

    beans::Property aProp;
    m_pProvider->getProperty(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ), aProp );

    uno::Sequence< beans::Property > aDocProps( 1 );
    aDocProps.getArray()[ 0 ] = aProp;
    aSeq.getArray()[ 0 ].Properties = aDocProps;

    // folder.
    aSeq.getArray()[ 1 ].Type
        = rtl::OUString::createFromAscii( WEBDAV_COLLECTION_TYPE );
    aSeq.getArray()[ 1 ].Attributes
        = ucb::ContentInfoAttribute::KIND_FOLDER;

    uno::Sequence< beans::Property > aFolderProps( 1 );
    aFolderProps.getArray()[ 0 ] = aProp;
    aSeq.getArray()[ 1 ].Properties = aFolderProps;
    return aSeq;
}

//=========================================================================
// virtual
uno::Reference< ucb::XContent > SAL_CALL
Content::createNewContent( const ucb::ContentInfo& Info )
    throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !Info.Type.getLength() )
        return uno::Reference< ucb::XContent >();

    if ( ( !Info.Type.equalsAsciiL(
               RTL_CONSTASCII_STRINGPARAM( WEBDAV_COLLECTION_TYPE ) ) )
         &&
         ( !Info.Type.equalsAsciiL(
             RTL_CONSTASCII_STRINGPARAM( WEBDAV_CONTENT_TYPE ) ) ) )
        return uno::Reference< ucb::XContent >();

    rtl::OUString aURL = m_xIdentifier->getContentIdentifier();

    OSL_ENSURE( aURL.getLength() > 0,
                "WebdavContent::createNewContent - empty identifier!" );

    if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
        aURL += rtl::OUString::createFromAscii( "/" );

    sal_Bool isCollection;
    if ( Info.Type.equalsAsciiL(
             RTL_CONSTASCII_STRINGPARAM( WEBDAV_COLLECTION_TYPE ) ) )
    {
        aURL += rtl::OUString::createFromAscii( "New_Collection" );
        isCollection = sal_True;
    }
    else
    {
        aURL += rtl::OUString::createFromAscii( "New_Content" );
        isCollection = sal_False;
    }

    uno::Reference< ucb::XContentIdentifier > xId(
                    new ::ucbhelper::ContentIdentifier( m_xSMgr, aURL ) );

    // create the local content
    try
    {
        return new ::http_dav_ucp::Content( m_xSMgr,
                                          m_pProvider,
                                          xId,
                                          m_xResAccess->getSessionFactory(),
                                          isCollection );
    }
    catch ( ucb::ContentCreationException & )
    {
        return uno::Reference< ucb::XContent >();
    }
}

//=========================================================================
// virtual
rtl::OUString Content::getParentURL()
{
    // <scheme>://              -> ""
    // <scheme>://foo           -> ""
    // <scheme>://foo/          -> ""
    // <scheme>://foo/bar       -> <scheme>://foo/
    // <scheme>://foo/bar/      -> <scheme>://foo/
    // <scheme>://foo/bar/abc   -> <scheme>://foo/bar/

    rtl::OUString aURL = m_xIdentifier->getContentIdentifier();

    sal_Int32 nPos = aURL.lastIndexOf( '/' );
    if ( nPos == ( aURL.getLength() - 1 ) )
    {
        // Trailing slash found. Skip.
        nPos = aURL.lastIndexOf( '/', nPos );
    }

    sal_Int32 nPos1 = aURL.lastIndexOf( '/', nPos );
    if ( nPos1 != -1 )
        nPos1 = aURL.lastIndexOf( '/', nPos1 );

    if ( nPos1 == -1 )
        return rtl::OUString();

    return rtl::OUString( aURL.copy( 0, nPos + 1 ) );
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

// static
uno::Reference< sdbc::XRow > Content::getPropertyValues(
    const uno::Reference< lang::XMultiServiceFactory >& rSMgr,
    const uno::Sequence< beans::Property >& rProperties,
    const ContentProperties& rData,
    const rtl::Reference< ::ucbhelper::ContentProviderImplHelper >& rProvider,
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

            // Process standard UCB, DAV and HTTP properties.
            const uno::Any & rValue = rData.getValue( rProp.Name );
            if ( rValue.hasValue() )
            {
                xRow->appendObject( rProp, rValue );
            }
            else
            {
                // Process local Additional Properties.
                if ( !bTriedToGetAdditonalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet
                        = uno::Reference< beans::XPropertySet >(
                            rProvider->getAdditionalPropertySet( rContentId,
                                                                 sal_False ),
                            uno::UNO_QUERY );
                    bTriedToGetAdditonalPropSet = sal_True;
                }

                if ( !xAdditionalPropSet.is() ||
                     !xRow->appendPropertySetValue(
                                            xAdditionalPropSet, rProp ) )
                {
                    // Append empty entry.
                    xRow->appendVoid( rProp );
                }
            }
        }
    }
    else
    {
        // Append all standard UCB, DAV and HTTP properties.

        const std::auto_ptr< PropertyValueMap > & xProps = rData.getProperties();

        PropertyValueMap::const_iterator it  = xProps->begin();
        PropertyValueMap::const_iterator end = xProps->end();

        ContentProvider * pProvider
            = static_cast< ContentProvider * >( rProvider.get() );
        beans::Property aProp;

        while ( it != end )
        {
            if ( pProvider->getProperty( (*it).first, aProp ) )
                xRow->appendObject( aProp, (*it).second.value() );

            ++it;
        }

        // Append all local Additional Properties.
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
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw ( uno::Exception )
{
    std::auto_ptr< ContentProperties > xProps;
    std::auto_ptr< ContentProperties > xCachedProps;
    std::auto_ptr< DAVResourceAccess > xResAccess;
    rtl::OUString aUnescapedTitle;
    bool bHasAll = false;
    uno::Reference< lang::XMultiServiceFactory > xSMgr;
    uno::Reference< ucb::XContentIdentifier > xIdentifier;
    rtl::Reference< ::ucbhelper::ContentProviderImplHelper > xProvider;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        aUnescapedTitle = SerfUri::unescape( m_aEscapedTitle );
        xSMgr.set( m_xSMgr );
        xIdentifier.set( m_xIdentifier );
        xProvider.set( m_xProvider.get() );
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );

        // First, ask cache...
        if ( m_xCachedProps.get() )
        {
            xCachedProps.reset( new ContentProperties( *m_xCachedProps.get() ) );

            std::vector< rtl::OUString > aMissingProps;
            if ( xCachedProps->containsAllNames( rProperties, aMissingProps ) )
            {
                // All properties are already in cache! No server access needed.
                bHasAll = true;
            }

            // use the cached ContentProperties instance
            xProps.reset( new ContentProperties( *xCachedProps.get() ) );
        }
    }

    if ( !m_bTransient && !bHasAll )
    {
        /////////////////////////////////////////////////////////////////////
        // Obtain values from server...
        /////////////////////////////////////////////////////////////////////

        // First, identify whether resource is DAV or not
        const ResourceType & rType = getResourceType( xEnv, xResAccess );

        bool bNetworkAccessAllowed = true;

        if ( DAV == rType )
        {
            // cache lookup... getResourceType may fill the props cache via
            // PROPFIND!
            if ( m_xCachedProps.get() )
            {
                xCachedProps.reset(
                    new ContentProperties( *m_xCachedProps.get() ) );

                std::vector< rtl::OUString > aMissingProps;
                if ( xCachedProps->containsAllNames(
                         rProperties, aMissingProps ) )
                {
                    // All properties are already in cache! No server access
                    // needed.
                    bHasAll = true;
                }

                // use the cached ContentProperties instance
                xProps.reset( new ContentProperties( *xCachedProps.get() ) );
            }

            if ( !bHasAll )
            {
                // Only DAV resources support PROPFIND
                std::vector< rtl::OUString > aPropNames;

                uno::Sequence< beans::Property > aProperties(
                    rProperties.getLength() );

                if ( m_aFailedPropNames.size() > 0 )
                {
                    sal_Int32 nProps = 0;
                    sal_Int32 nCount = rProperties.getLength();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        const rtl::OUString & rName = rProperties[ n ].Name;

                        std::vector< rtl::OUString >::const_iterator it
                            = m_aFailedPropNames.begin();
                        std::vector< rtl::OUString >::const_iterator end
                            = m_aFailedPropNames.end();

                        while ( it != end )
                        {
                            if ( *it == rName )
                                break;

                            ++it;
                        }

                        if ( it == end )
                        {
                            aProperties[ nProps ] = rProperties[ n ];
                            nProps++;
                        }
                    }

                    aProperties.realloc( nProps );
                }
                else
                {
                    aProperties = rProperties;
                }

                if ( aProperties.getLength() > 0 )
                    ContentProperties::UCBNamesToDAVNames(
                        aProperties, aPropNames );

                if ( aPropNames.size() > 0 )
                {
                    std::vector< DAVResource > resources;
                    try
                    {
                        xResAccess->PROPFIND(
                            DAVZERO, aPropNames, resources, xEnv );

                        if ( 1 == resources.size() )
                        {
                            if ( xProps.get())
                                xProps->addProperties(
                                    aPropNames,
                                    ContentProperties( resources[ 0 ] ));
                            else
                                xProps.reset(
                                    new ContentProperties( resources[ 0 ] ) );
                        }
                    }
                    catch ( DAVException const & e )
                    {
                        bNetworkAccessAllowed
                            = shouldAccessNetworkAfterException( e );

                        if ( !bNetworkAccessAllowed )
                        {
                            cancelCommandExecution( e, xEnv );
                            // unreachable
                        }
                    }
                }
            }
        }

        if ( bNetworkAccessAllowed )
        {
            // All properties obtained already?
            std::vector< rtl::OUString > aMissingProps;
            if ( !( xProps.get()
                    && xProps->containsAllNames(
                        rProperties, aMissingProps ) )
                 && !m_bDidGetOrHead )
            {
                // Possibly the missing props can be obtained using a HEAD
                // request.

                std::vector< rtl::OUString > aHeaderNames;
                ContentProperties::UCBNamesToHTTPNames(
                    rProperties,
                    aHeaderNames,
                    true /* bIncludeUnmatched */ );

                if ( aHeaderNames.size() > 0 )
                {
                    try
                    {
                        DAVResource resource;
                        xResAccess->HEAD( aHeaderNames, resource, xEnv );
                        m_bDidGetOrHead = true;

                        if ( xProps.get() )
                            xProps->addProperties(
                                aMissingProps,
                                ContentProperties( resource ) );
                        else
                            xProps.reset ( new ContentProperties( resource ) );

                        if ( m_eResourceType == NON_DAV )
                            xProps->addProperties( aMissingProps,
                                                   ContentProperties(
                                                       aUnescapedTitle,
                                                       false ) );
                    }
                    catch ( DAVException const & e )
                    {
                        bNetworkAccessAllowed
                            = shouldAccessNetworkAfterException( e );

                        if ( !bNetworkAccessAllowed )
                        {
                            cancelCommandExecution( e, xEnv );
                            // unreachable
                        }
                    }
                }
            }
        }

        // might trigger HTTP redirect.
        // Therefore, title must be updated here.
        SerfUri aUri( xResAccess->getURL() );
        aUnescapedTitle = aUri.GetPathBaseNameUnescaped();

        if ( rType == UNKNOWN )
        {
            xProps.reset( new ContentProperties( aUnescapedTitle ) );
        }

        // For DAV resources we only know the Title, for non-DAV
        // resources we additionally know that it is a document.

        if ( rType == DAV )
        {
            //xProps.reset(
            //    new ContentProperties( aUnescapedTitle ) );
            xProps->addProperty(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                uno::makeAny( aUnescapedTitle ),
                true );
        }
        else
        {
            if ( !xProps.get() )
                xProps.reset( new ContentProperties( aUnescapedTitle, false ) );
            else
                xProps->addProperty(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                    uno::makeAny( aUnescapedTitle ),
                    true );

            xProps->addProperty(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
                uno::makeAny( false ),
                true );
            xProps->addProperty(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
                uno::makeAny( true ),
                true );
        }
    }
    else
    {
        // No server access for just created (not yet committed) objects.
        // Only a minimal set of properties supported at this stage.
        if (m_bTransient)
            xProps.reset( new ContentProperties( aUnescapedTitle,
                                                 m_bCollection ) );
    }

    sal_Int32 nCount = rProperties.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const rtl::OUString rName = rProperties[ n ].Name;
        if ( rName.equalsAsciiL(
                      RTL_CONSTASCII_STRINGPARAM( "BaseURI" ) ) )
        {
            // Add BaseURI property, if requested.
            xProps->addProperty(
                 rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BaseURI" ) ),
                 uno::makeAny( getBaseURI( xResAccess ) ),
                 true );
        }
        else if ( rName.equalsAsciiL(
                      RTL_CONSTASCII_STRINGPARAM( "CreatableContentsInfo" ) ) )
        {
            // Add CreatableContentsInfo property, if requested.
            sal_Bool bFolder = sal_False;
            xProps->getValue(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ) )
                    >>= bFolder;
            xProps->addProperty(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "CreatableContentsInfo" ) ),
                uno::makeAny( bFolder
                                  ? queryCreatableContentsInfo()
                                  : uno::Sequence< ucb::ContentInfo >() ),
                true );
        }
    }

    uno::Reference< sdbc::XRow > xResultRow
        = getPropertyValues( xSMgr,
                             rProperties,
                             *xProps,
                             xProvider,
                             xIdentifier->getContentIdentifier() );

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !m_xCachedProps.get() )
            m_xCachedProps.reset( new CachableContentProperties( *xProps.get() ) );
        else
            m_xCachedProps->addProperties( *xProps.get() );

        m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
        m_aEscapedTitle = SerfUri::escapeSegment( aUnescapedTitle );
    }

    return xResultRow;
}

//=========================================================================
uno::Sequence< uno::Any > Content::setPropertyValues(
                const uno::Sequence< beans::PropertyValue >& rValues,
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw ( uno::Exception )
{
    uno::Reference< lang::XMultiServiceFactory > xSMgr;
    uno::Reference< ucb::XContentIdentifier >    xIdentifier;
    rtl::Reference< ContentProvider >            xProvider;
    sal_Bool bTransient;
    std::auto_ptr< DAVResourceAccess > xResAccess;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        xProvider.set( m_pProvider );
        xIdentifier.set( m_xIdentifier );
        bTransient = m_bTransient;
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        xSMgr.set( m_xSMgr );
    }

    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    beans::PropertyChangeEvent aEvent;
    aEvent.Source         = static_cast< cppu::OWeakObject * >( this );
    aEvent.Further        = sal_False;
    // aEvent.PropertyName =
    aEvent.PropertyHandle = -1;
    // aEvent.OldValue   =
    // aEvent.NewValue   =

    std::vector< ProppatchValue > aProppatchValues;
    std::vector< sal_Int32 > aProppatchPropsPositions;

    uno::Reference< ucb::XPersistentPropertySet > xAdditionalPropSet;
    sal_Bool bTriedToGetAdditonalPropSet = sal_False;

    sal_Bool bExchange = sal_False;
    rtl::OUString aNewTitle;
    rtl::OUString aOldTitle;
    sal_Int32 nTitlePos = -1;

    uno::Reference< beans::XPropertySetInfo > xInfo;

    const beans::PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = pValues[ n ];
        const rtl::OUString & rName = rValue.Name;

        beans::Property aTmpProp;
        xProvider->getProperty( rName, aTmpProp );

        if ( aTmpProp.Attributes & beans::PropertyAttribute::READONLY )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
            continue;
        }

        //////////////////////////////////////////////////////////////////
        // Mandatory props.
        //////////////////////////////////////////////////////////////////

        if ( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ContentType" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                rtl::OUString::createFromAscii(
                    "Property is read-only!" ),
                static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName.equalsAsciiL(
                      RTL_CONSTASCII_STRINGPARAM( "IsDocument" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                rtl::OUString::createFromAscii(
                    "Property is read-only!" ),
                static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName.equalsAsciiL(
                      RTL_CONSTASCII_STRINGPARAM( "IsFolder" ) ) )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            rtl::OUString::createFromAscii(
                                "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Title" ) ) )
        {
            rtl::OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                // No empty titles!
                if ( aNewValue.getLength() > 0 )
                {
                    try
                    {
                        SerfUri aURI( xIdentifier->getContentIdentifier() );
                        aOldTitle = aURI.GetPathBaseNameUnescaped();

                        if ( aNewValue != aOldTitle )
                        {
                            // modified title -> modified URL -> exchange !
                            if ( !bTransient )
                                bExchange = sal_True;

                            // new value will be set later...
                            aNewTitle = aNewValue;

                            // remember position within sequence of values (for
                            // error handling).
                            nTitlePos = n;
                        }
                    }
                    catch ( DAVException const & )
                    {
                        aRet[ n ] <<= lang::IllegalArgumentException(
                            rtl::OUString::createFromAscii(
                                "Invalid content identifier!" ),
                            static_cast< cppu::OWeakObject * >( this ),
                            -1 );
                    }
                }
                else
                {
                    aRet[ n ] <<= lang::IllegalArgumentException(
                        rtl::OUString::createFromAscii(
                            "Empty title not allowed!" ),
                        static_cast< cppu::OWeakObject * >( this ),
                        -1 );
                }
            }
            else
            {
                aRet[ n ] <<= beans::IllegalTypeException(
                    rtl::OUString::createFromAscii(
                        "Property value has wrong type!" ),
                    static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        else
        {
            //////////////////////////////////////////////////////////////
            // Optional props.
            //////////////////////////////////////////////////////////////

            if ( !xInfo.is() )
                xInfo = getPropertySetInfo( xEnv,
                                            sal_False /* don't cache data */ );

            if ( !xInfo->hasPropertyByName( rName ) )
            {
                // Check, whether property exists. Skip otherwise.
                // PROPPATCH::set would add the property automatically, which
                // is not allowed for "setPropertyValues" command!
                aRet[ n ] <<= beans::UnknownPropertyException(
                                rtl::OUString::createFromAscii(
                                    "Property is unknown!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                continue;
            }

            if ( rName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Size" ) ) )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                rtl::OUString::createFromAscii(
                                    "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "DateCreated" ) ) )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                rtl::OUString::createFromAscii(
                                    "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "DateModified" ) ) )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                rtl::OUString::createFromAscii(
                                    "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
            {
                // Read-only property!
                // (but could be writable, if 'getcontenttype' would be)
                aRet[ n ] <<= lang::IllegalAccessException(
                                rtl::OUString::createFromAscii(
                                    "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            if ( rName.equalsAsciiL(
                     RTL_CONSTASCII_STRINGPARAM( "CreatableContentsInfo" ) ) )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                rtl::OUString::createFromAscii(
                                    "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else
            {
                if ( getResourceType( xEnv, xResAccess ) == DAV )
                {
                    // Property value will be set on server.
                    ProppatchValue aValue( PROPSET, rName, rValue.Value );
                    aProppatchValues.push_back( aValue );

                    // remember position within sequence of values (for
                    // error handling).
                    aProppatchPropsPositions.push_back( n );
                }
                else
                {
                    // Property value will be stored in local property store.
                    if ( !bTriedToGetAdditonalPropSet &&
                         !xAdditionalPropSet.is() )
                    {
                        xAdditionalPropSet
                            = getAdditionalPropertySet( sal_False );
                        bTriedToGetAdditonalPropSet = sal_True;
                    }

                    if ( xAdditionalPropSet.is() )
                    {
                        try
                        {
                            uno::Any aOldValue
                                = xAdditionalPropSet->getPropertyValue( rName );
                            if ( aOldValue != rValue.Value )
                            {
                                xAdditionalPropSet->setPropertyValue(
                                                        rName, rValue.Value );

                                aEvent.PropertyName = rName;
                                aEvent.OldValue     = aOldValue;
                                aEvent.NewValue     = rValue.Value;

                                aChanges.getArray()[ nChanged ] = aEvent;
                                nChanged++;
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
                                rtl::OUString::createFromAscii(
                                    "No property set for storing the value!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                    }
                }
            }
        }
    } // for

    if ( !bTransient && aProppatchValues.size() )
    {
        try
        {
            // Set property values at server.
            xResAccess->PROPPATCH( aProppatchValues, xEnv );

            std::vector< ProppatchValue >::const_iterator it
                = aProppatchValues.begin();
            std::vector< ProppatchValue >::const_iterator end
                = aProppatchValues.end();

            while ( it != end )
            {
                aEvent.PropertyName = (*it).name;
                aEvent.OldValue     = uno::Any(); // @@@ to expensive to obtain!
                aEvent.NewValue     = (*it).value;

                aChanges.getArray()[ nChanged ] = aEvent;
                nChanged++;

                ++it;
            }
        }
        catch ( DAVException const & e )
        {
//            OSL_ENSURE( sal_False,
//                        "Content::setPropertyValues - PROPPATCH failed!" );

#if 1
            cancelCommandExecution( e, xEnv );
            // unreachable
#else
            // Note: PROPPATCH either sets ALL property values OR NOTHING.

            std::vector< sal_Int32 >::const_iterator it
                = aProppatchPropsPositions.begin();
            std::vector< sal_Int32 >::const_iterator end
                = aProppatchPropsPositions.end();

            while ( it != end )
            {
                // Set error.
                aRet[ (*it) ] <<= MapDAVException( e, sal_True );
                ++it;
            }
#endif
        }
    }

    if ( bExchange )
    {
        // Assemble new content identifier...

        rtl::OUString aNewURL = getParentURL();
        if ( aNewURL.lastIndexOf( '/' ) != ( aNewURL.getLength() - 1 ) )
            aNewURL += rtl::OUString::createFromAscii( "/" );

        aNewURL += SerfUri::escapeSegment( aNewTitle );

        uno::Reference< ucb::XContentIdentifier > xNewId
            = new ::ucbhelper::ContentIdentifier( xSMgr, aNewURL );
        uno::Reference< ucb::XContentIdentifier > xOldId = xIdentifier;

        try
        {
            SerfUri sourceURI( xOldId->getContentIdentifier() );
            SerfUri targetURI( xNewId->getContentIdentifier() );
            targetURI.SetScheme( sourceURI.GetScheme() );

            xResAccess->MOVE(
                sourceURI.GetPath(), targetURI.GetURI(), sal_False, xEnv );
            // @@@ Should check for resources that could not be moved
            //     (due to source access or target overwrite) and send
            //     this information through the interaction handler.

            // @@@ Existing content should be checked to see if it needs
            //     to be deleted at the source

            // @@@ Existing content should be checked to see if it has
            //     been overwritten at the target

            if ( exchangeIdentity( xNewId ) )
            {
                xResAccess->setURL( aNewURL );

// DAV resources store all additional props on server!
//              // Adapt Additional Core Properties.
//              renameAdditionalPropertySet( xOldId->getContentIdentifier(),
//                                           xNewId->getContentIdentifier(),
//                                           sal_True );
            }
            else
            {
                // Do not set new title!
                aNewTitle = rtl::OUString();

                // Set error .
                aRet[ nTitlePos ] <<= uno::Exception(
                    rtl::OUString::createFromAscii( "Exchange failed!" ),
                    static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        catch ( DAVException const & e )
        {
            // Do not set new title!
            aNewTitle = rtl::OUString();

            // Set error .
            aRet[ nTitlePos ] <<= MapDAVException( e, sal_True );
        }
    }

    if ( aNewTitle.getLength() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        aEvent.PropertyName = rtl::OUString::createFromAscii( "Title" );
        aEvent.OldValue     = uno::makeAny( aOldTitle );
        aEvent.NewValue     = uno::makeAny( aNewTitle );

        m_aEscapedTitle     = SerfUri::escapeSegment( aNewTitle );

        aChanges.getArray()[ nChanged ] = aEvent;
        nChanged++;
    }

    if ( nChanged > 0 )
    {
        aChanges.realloc( nChanged );
        notifyPropertiesChange( aChanges );
    }

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
    }

    return aRet;
}

//=========================================================================
uno::Any Content::open(
                const ucb::OpenCommandArgument2 & rArg,
                const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    uno::Any aRet;

    sal_Bool bOpenFolder = ( ( rArg.Mode == ucb::OpenMode::ALL ) ||
                             ( rArg.Mode == ucb::OpenMode::FOLDERS ) ||
                             ( rArg.Mode == ucb::OpenMode::DOCUMENTS ) );
    if ( bOpenFolder )
    {
        if ( isFolder( xEnv ) )
        {
            // Open collection.

            uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet( m_xSMgr, this, rArg, xEnv );
            aRet <<= xSet;
        }
        else
        {
            // Error: Not a folder!

            rtl::OUStringBuffer aMsg;
            aMsg.appendAscii( "Non-folder resource cannot be "
                              "opened as folder! Wrong Open Mode!" );

            ucbhelper::cancelCommandExecution(
                uno::makeAny(
                    lang::IllegalArgumentException(
                        aMsg.makeStringAndClear(),
                        static_cast< cppu::OWeakObject * >( this ),
                        -1 ) ),
                xEnv );
            // Unreachable
        }
    }

    if ( rArg.Sink.is() )
    {
        // Open document.

        if ( ( rArg.Mode == ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
             ( rArg.Mode == ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
        {
            // Currently(?) unsupported.
            ucbhelper::cancelCommandExecution(
                uno::makeAny(
                    ucb::UnsupportedOpenModeException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ),
                            sal_Int16( rArg.Mode ) ) ),
                xEnv );
            // Unreachable
        }

        rtl::OUString aURL = m_xIdentifier->getContentIdentifier();
        uno::Reference< io::XOutputStream > xOut
            = uno::Reference< io::XOutputStream >( rArg.Sink, uno::UNO_QUERY );
        if ( xOut.is() )
        {
            // PUSH: write data
            try
            {
                std::auto_ptr< DAVResourceAccess > xResAccess;

                {
                    osl::MutexGuard aGuard( m_aMutex );

                    xResAccess.reset(
                        new DAVResourceAccess( *m_xResAccess.get() ) );
                }

                DAVResource aResource;
                std::vector< rtl::OUString > aHeaders;

                xResAccess->GET( xOut, aHeaders, aResource, xEnv );
                m_bDidGetOrHead = true;

                {
                    osl::MutexGuard aGuard( m_aMutex );

                    // cache headers.
                    if ( !m_xCachedProps.get())
                        m_xCachedProps.reset(
                            new CachableContentProperties( aResource ) );
                    else
                        m_xCachedProps->addProperties( aResource );

                    m_xResAccess.reset(
                        new DAVResourceAccess( *xResAccess.get() ) );
                }
            }
            catch ( DAVException const & e )
            {
                cancelCommandExecution( e, xEnv );
                // Unreachable
            }
        }
        else
        {
            uno::Reference< io::XActiveDataSink > xDataSink
                = uno::Reference< io::XActiveDataSink >( rArg.Sink,
                                                         uno::UNO_QUERY );
            if ( xDataSink.is() )
            {
                // PULL: wait for client read
                try
                {
                    std::auto_ptr< DAVResourceAccess > xResAccess;
                    {
                        osl::MutexGuard aGuard( m_aMutex );

                        xResAccess.reset(
                            new DAVResourceAccess( *m_xResAccess.get() ) );
                    }

                    // fill inputsream sync; return if all data present
                    DAVResource aResource;
                    std::vector< rtl::OUString > aHeaders;

                    uno::Reference< io::XInputStream > xIn
                        = xResAccess->GET( aHeaders, aResource, xEnv );
                    m_bDidGetOrHead = true;

                    {
                        osl::MutexGuard aGuard( m_aMutex );

                        // cache headers.
                        if ( !m_xCachedProps.get())
                            m_xCachedProps.reset(
                                new CachableContentProperties( aResource ) );
                        else
                            m_xCachedProps->addProperties(
                                aResource.properties );

                        m_xResAccess.reset(
                            new DAVResourceAccess( *xResAccess.get() ) );
                    }

                    xDataSink->setInputStream( xIn );
                }
                catch ( DAVException const & e )
                {
                    cancelCommandExecution( e, xEnv );
                    // Unreachable
                }
            }
            else
            {
                // Note: aOpenCommand.Sink may contain an XStream
                //       implementation. Support for this type of
                //       sink is optional...
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        ucb::UnsupportedDataSinkException(
                            rtl::OUString(),
                            static_cast< cppu::OWeakObject * >( this ),
                            rArg.Sink ) ),
                    xEnv );
                // Unreachable
            }
        }
    }

    return aRet;
}

//=========================================================================
void Content::post(
                const ucb::PostCommandArgument2 & rArg,
                const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    uno::Reference< io::XActiveDataSink > xSink( rArg.Sink, uno::UNO_QUERY );
    if ( xSink.is() )
    {
        try
        {
            std::auto_ptr< DAVResourceAccess > xResAccess;
            {
                osl::MutexGuard aGuard( m_aMutex );
                xResAccess.reset(
                    new DAVResourceAccess( *m_xResAccess.get() ) );
            }

            uno::Reference< io::XInputStream > xResult
                = xResAccess->POST( rArg.MediaType,
                                    rArg.Referer,
                                    rArg.Source,
                                    xEnv );

            {
                 osl::MutexGuard aGuard( m_aMutex );
                 m_xResAccess.reset(
                     new DAVResourceAccess( *xResAccess.get() ) );
            }

            xSink->setInputStream( xResult );
        }
        catch ( DAVException const & e )
        {
            cancelCommandExecution( e, xEnv, sal_True );
            // Unreachable
        }
    }
    else
    {
        uno::Reference< io::XOutputStream > xResult( rArg.Sink, uno::UNO_QUERY );
        if ( xResult.is() )
        {
            try
            {
                std::auto_ptr< DAVResourceAccess > xResAccess;
                {
                    osl::MutexGuard aGuard( m_aMutex );
                    xResAccess.reset(
                        new DAVResourceAccess( *m_xResAccess.get() ) );
                }

                xResAccess->POST( rArg.MediaType,
                                  rArg.Referer,
                                  rArg.Source,
                                  xResult,
                                  xEnv );

                {
                    osl::MutexGuard aGuard( m_aMutex );
                    m_xResAccess.reset(
                        new DAVResourceAccess( *xResAccess.get() ) );
                }
            }
            catch ( DAVException const & e )
            {
                cancelCommandExecution( e, xEnv, sal_True );
                // Unreachable
            }
        }
        else
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny(
                    ucb::UnsupportedDataSinkException(
                        rtl::OUString(),
                        static_cast< cppu::OWeakObject * >( this ),
                        rArg.Sink ) ),
                xEnv );
            // Unreachable
        }
    }
}

//=========================================================================
void Content::queryChildren( ContentRefList& rChildren )
{
    // Obtain a list with a snapshot of all currently instanciated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ::ucbhelper::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    rtl::OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nURLPos = aURL.lastIndexOf( '/' );

    if ( nURLPos != ( aURL.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aURL += rtl::OUString::createFromAscii( "/" );
    }

    sal_Int32 nLen = aURL.getLength();

    ::ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

    while ( it != end )
    {
        ::ucbhelper::ContentImplHelperRef xChild = (*it);
        rtl::OUString aChildURL
            = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.compareTo( aURL, nLen ) == 0 ) )
        {
            sal_Int32 nPos = nLen;
            nPos = aChildURL.indexOf( '/', nPos );

            if ( ( nPos == -1 ) ||
                 ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                // No further slashes / only a final slash. It's a child!
                rChildren.push_back(
                    ::http_dav_ucp::Content::ContentRef(
                        static_cast< ::http_dav_ucp::Content * >(
                            xChild.get() ) ) );
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
    sal_Bool bTransient, bCollection;
    rtl::OUString aEscapedTitle;
    std::auto_ptr< DAVResourceAccess > xResAccess;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        bTransient    = m_bTransient;
        bCollection   = m_bCollection;
        aEscapedTitle = m_aEscapedTitle;
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
    }

    // Check, if all required properties are present.

    if ( aEscapedTitle.getLength() == 0 )
    {
        OSL_ENSURE( sal_False, "Content::insert - Title missing!" );

        uno::Sequence< rtl::OUString > aProps( 1 );
        aProps[ 0 ] = rtl::OUString::createFromAscii( "Title" );
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::MissingPropertiesException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                aProps ) ),
            Environment );
        // Unreachable
    }

    if ( !bReplaceExisting )
    {
        /* [RFC 2616] - HTTP

           The PUT method requests that the enclosed entity be stored under the
           supplied Request-URI. If the Request-URI refers to an already
           existing resource, the enclosed entity SHOULD be considered as a
           modified version of the one residing on the origin server.
        */

        /* [RFC 2518] - WebDAV

           MKCOL creates a new collection resource at the location specified by
           the Request-URI.  If the resource identified by the Request-URI is
           non-null then the MKCOL MUST fail.
        */

        // ==> Complain on PUT, continue on MKCOL.
        if ( !bTransient || ( bTransient && !bCollection  ) )
        {
#undef ERROR
            ucb::UnsupportedNameClashException aEx(
                rtl::OUString::createFromAscii( "Unable to write without overwrite!" ),
                static_cast< cppu::OWeakObject * >( this ),
                ucb::NameClash::ERROR );

            uno::Reference< task::XInteractionHandler > xIH;

            if ( Environment.is() )
                xIH = Environment->getInteractionHandler();

            if ( xIH.is() )
            {
                uno::Any aExAsAny( uno::makeAny( aEx ) );

                rtl::Reference< ucbhelper::SimpleInteractionRequest > xRequest
                    = new ucbhelper::SimpleInteractionRequest(
                        aExAsAny,
                        ucbhelper::CONTINUATION_APPROVE
                            | ucbhelper::CONTINUATION_DISAPPROVE );
                xIH->handle( xRequest.get() );

                const sal_Int32 nResp = xRequest->getResponse();

                switch ( nResp )
                {
                    case ucbhelper::CONTINUATION_UNKNOWN:
                        // Not handled; throw.
                        throw aEx;
//                            break;

                    case ucbhelper::CONTINUATION_APPROVE:
                        // Continue -> Overwrite.
                        bReplaceExisting = sal_True;
                        break;

                    case ucbhelper::CONTINUATION_DISAPPROVE:
                        // Abort.
                        throw ucb::CommandFailedException(
                                    rtl::OUString(),
                                    uno::Reference< uno::XInterface >(),
                                    aExAsAny );
//                            break;

                    default:
                        OSL_ENSURE( sal_False,
                                    "Content::insert - "
                                    "Unknown interaction selection!" );
                        throw ucb::CommandFailedException(
                                    rtl::OUString::createFromAscii(
                                        "Unknown interaction selection!" ),
                                    uno::Reference< uno::XInterface >(),
                                    aExAsAny );
//                            break;
                }
            }
            else
            {
                // No IH; throw.
                throw aEx;
            }
        }
    }

    if ( bTransient )
    {
        // Assemble new content identifier...
        rtl::OUString aURL = getParentURL();
        if ( aURL.lastIndexOf( '/' ) != ( aURL.getLength() - 1 ) )
            aURL += rtl::OUString::createFromAscii( "/" );

        aURL += aEscapedTitle;

        try
        {
            xResAccess->setURL( aURL );

            if ( bCollection )
                xResAccess->MKCOL( Environment );
            else
                xResAccess->PUT( xInputStream, Environment );
        }
        catch ( DAVException const & except )
        {
            if ( bCollection )
            {
                if ( except.getStatus() == SC_METHOD_NOT_ALLOWED )
                {
                    // [RFC 2518] - WebDAV
                    // 405 (Method Not Allowed) - MKCOL can only be
                    // executed on a deleted/non-existent resource.

                    if ( bReplaceExisting )
                    {
                        // Destroy old resource.
                        try
                        {
                            xResAccess->DESTROY( Environment );
                        }
                        catch ( DAVException const & e )
                        {
                            cancelCommandExecution( e, Environment, sal_True );
                            // Unreachable
                        }

                        // Insert (recursion!).
                        insert( xInputStream, bReplaceExisting, Environment );

                        {
                            osl::Guard< osl::Mutex > aGuard( m_aMutex );
                            m_xResAccess.reset(
                                new DAVResourceAccess( *xResAccess.get() ) );
                        }

                        // Success!
                        return;
                    }
                    else
                    {
                        rtl::OUString aTitle;
                        try
                        {
                            SerfUri aURI( aURL );
                            aTitle = aURI.GetPathBaseNameUnescaped();
                        }
                        catch ( DAVException const & )
                        {
                        }

                        ucbhelper::cancelCommandExecution(
                            uno::makeAny(
                                ucb::NameClashException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    task::InteractionClassification_ERROR,
                                    aTitle ) ),
                            Environment );
                        // Unreachable
                    }
                }
            }

            cancelCommandExecution( except, Environment, sal_True );
            // Unreachable
        }

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xIdentifier
                = new ::ucbhelper::ContentIdentifier( m_xSMgr, aURL );
        }

        inserted();

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_bTransient = sal_False;
        }
    }
    else
    {
        if ( !xInputStream.is() )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny(
                    ucb::MissingInputStreamException(
                        rtl::OUString(),
                        static_cast< cppu::OWeakObject * >( this ) ) ),
                Environment );
            // Unreachable
        }

        try
        {
            xResAccess->PUT( xInputStream, Environment );
        }
        catch ( DAVException const & e )
        {
            cancelCommandExecution( e, Environment, sal_True );
            // Unreachable
        }
    }

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
    }
}

//=========================================================================
void Content::transfer(
        const ucb::TransferInfo & rArgs,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception )
{
    uno::Reference< lang::XMultiServiceFactory > xSMgr;
    uno::Reference< ucb::XContentIdentifier >    xIdentifier;
    uno::Reference< ucb::XContentProvider >      xProvider;
    std::auto_ptr< DAVResourceAccess > xResAccess;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        xSMgr.set( m_xSMgr );
        xIdentifier.set( m_xIdentifier );
        xProvider.set( m_xProvider.get() );
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
    }

    rtl::OUString aTargetURI;
    try
    {
        SerfUri sourceURI( rArgs.SourceURL );
        SerfUri targetURI( xIdentifier->getContentIdentifier() );
        aTargetURI = targetURI.GetPathBaseNameUnescaped();

        // Check source's and target's URL scheme
        //
        const rtl::OUString aScheme = sourceURI.GetScheme().toAsciiLowerCase();
        if ( aScheme.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( WEBDAV_URL_SCHEME ) ) )
        {
            sourceURI.SetScheme(
                rtl::OUString::createFromAscii( HTTP_URL_SCHEME ) );
        }
        else if ( aScheme.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( DAV_URL_SCHEME ) ) )
        {
            sourceURI.SetScheme(
                rtl::OUString::createFromAscii( HTTP_URL_SCHEME ) );
        }
        else if ( aScheme.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( DAVS_URL_SCHEME ) ) )
        {
            sourceURI.SetScheme(
                rtl::OUString::createFromAscii( HTTPS_URL_SCHEME ) );
        }
        else
        {
            if ( !aScheme.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( HTTP_URL_SCHEME ) ) &&
                 !aScheme.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM( HTTPS_URL_SCHEME ) ) )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        ucb::InteractiveBadTransferURLException(
                            rtl::OUString::createFromAscii(
                                "Unsupported URL scheme!" ),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
                    Environment );
                // Unreachable
            }
        }

        if ( targetURI.GetScheme().toAsciiLowerCase().equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM( WEBDAV_URL_SCHEME ) ) )
            targetURI.SetScheme(
                rtl::OUString::createFromAscii( HTTP_URL_SCHEME ) );
        else if ( targetURI.GetScheme().toAsciiLowerCase().equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM( DAV_URL_SCHEME ) ) )
            targetURI.SetScheme(
                rtl::OUString::createFromAscii( HTTP_URL_SCHEME ) );

        // @@@ This implementation of 'transfer' only works
        //     if the source and target are located at same host.
        //     (Neon does not support cross-server copy/move)

        // Check for same host
        //
        if ( sourceURI.GetHost().getLength() &&
             ( sourceURI.GetHost() != targetURI.GetHost() ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( ucb::InteractiveBadTransferURLException(
                                rtl::OUString::createFromAscii(
                                    "Different hosts!" ),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
                Environment );
            // Unreachable
        }

        rtl::OUString aTitle = rArgs.NewTitle;

        if ( !aTitle.getLength() )
            aTitle = sourceURI.GetPathBaseNameUnescaped();

        if ( aTitle.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "/" ) ) )
        {
            // kso: ???
            aTitle = rtl::OUString();
        }

        targetURI.AppendPath( aTitle );

        rtl::OUString aTargetURL = xIdentifier->getContentIdentifier();
        if ( ( aTargetURL.lastIndexOf( '/' ) + 1 )
                != aTargetURL.getLength() )
            aTargetURL += rtl::OUString::createFromAscii( "/" );

        aTargetURL += aTitle;

        uno::Reference< ucb::XContentIdentifier > xTargetId
            = new ::ucbhelper::ContentIdentifier( xSMgr, aTargetURL );

        DAVResourceAccess aSourceAccess( xSMgr,
                                         xResAccess->getSessionFactory(),
                                         sourceURI.GetURI() );

        if ( rArgs.MoveData == sal_True )
        {
            uno::Reference< ucb::XContentIdentifier > xId
                = new ::ucbhelper::ContentIdentifier( xSMgr, rArgs.SourceURL );

            // Note: The static cast is okay here, because its sure that
            //       xProvider is always the WebDAVContentProvider.
            rtl::Reference< Content > xSource
                = static_cast< Content * >(
                    xProvider->queryContent( xId ).get() );

            // [RFC 2518] - WebDAV
            // If a resource exists at the destination and the Overwrite
            // header is "T" then prior to performing the move the server
            // MUST perform a DELETE with "Depth: infinity" on the
            // destination resource.  If the Overwrite header is set to
            // "F" then the operation will fail.

            aSourceAccess.MOVE( sourceURI.GetPath(),
                                targetURI.GetURI(),
                                rArgs.NameClash
                                    == ucb::NameClash::OVERWRITE,
                                Environment );

            if ( xSource.is() )
            {
                // Propagate destruction to listeners.
                xSource->destroy( sal_True );
            }

// DAV resources store all additional props on server!
//              // Rename own and all children's Additional Core Properties.
//              renameAdditionalPropertySet( xId->getContentIdentifier(),
//                                           xTargetId->getContentIdentifier(),
//                                           sal_True );
        }
        else
        {
            // [RFC 2518] - WebDAV
            // If a resource exists at the destination and the Overwrite
            // header is "T" then prior to performing the copy the server
            // MUST perform a DELETE with "Depth: infinity" on the
            // destination resource.  If the Overwrite header is set to
            // "F" then the operation will fail.

            aSourceAccess.COPY( sourceURI.GetPath(),
                                targetURI.GetURI(),
                                rArgs.NameClash
                                    == ucb::NameClash::OVERWRITE,
                                Environment );

// DAV resources store all additional props on server!
//              // Copy own and all children's Additional Core Properties.
//              copyAdditionalPropertySet( xId->getContentIdentifier(),
//                                         xTargetId->getContentIdentifier(),
//                                         sal_True );
        }

        // Note: The static cast is okay here, because its sure that
        //       xProvider is always the WebDAVContentProvider.
        rtl::Reference< Content > xTarget
            = static_cast< Content * >(
                    xProvider->queryContent( xTargetId ).get() );

        // Announce transfered content in its new folder.
        xTarget->inserted();
    }
    catch ( ucb::IllegalIdentifierException const & )
    {
        // queryContent
    }
    catch ( DAVException const & e )
    {
        // [RFC 2518] - WebDAV
        // 412 (Precondition Failed) - The server was unable to maintain
        // the liveness of the properties listed in the propertybehavior
        // XML element or the Overwrite header is "F" and the state of
        // the destination resource is non-null.

        if ( e.getStatus() == SC_PRECONDITION_FAILED )
        {
            switch ( rArgs.NameClash )
            {
                case 0/*ucb::NameClash::ERROR*/:
                {
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny(
                            ucb::NameClashException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                task::InteractionClassification_ERROR,
                                aTargetURI ) ),
                        Environment );
                    // Unreachable
                }

                case ucb::NameClash::OVERWRITE:
                    break;

                case ucb::NameClash::KEEP: // deprecated
                case ucb::NameClash::RENAME:
                case ucb::NameClash::ASK:
                default:
                {
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny(
                            ucb::UnsupportedNameClashException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                rArgs.NameClash ) ),
                        Environment );
                    // Unreachable
                }
            }
        }

        cancelCommandExecution( e, Environment, sal_True );
        // Unreachable
    }

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
    }
}

//=========================================================================
void Content::destroy( sal_Bool bDeletePhysical )
    throw( uno::Exception )
{
    // @@@ take care about bDeletePhysical -> trashcan support
    rtl::OUString aURL = m_xIdentifier->getContentIdentifier();

    uno::Reference< ucb::XContent > xThis = this;

    deleted();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Process instanciated children...

    ::http_dav_ucp::Content::ContentRefList aChildren;
    queryChildren( aChildren );

    ContentRefList::const_iterator it  = aChildren.begin();
    ContentRefList::const_iterator end = aChildren.end();

    while ( it != end )
    {
        (*it)->destroy( bDeletePhysical );
        ++it;
    }
}

//=========================================================================
bool Content::supportsExclusiveWriteLock(
  const uno::Reference< ucb::XCommandEnvironment >& Environment )
{
    if ( getResourceType( Environment ) == DAV )
    {
        if ( m_xCachedProps.get() )
        {
            uno::Sequence< ucb::LockEntry > aSupportedLocks;
            if ( m_xCachedProps->getValue( DAVProperties::SUPPORTEDLOCK )
                >>= aSupportedLocks )
            {
                for ( sal_Int32 n = 0; n < aSupportedLocks.getLength(); ++n )
                {
                    if ( aSupportedLocks[ n ].Scope
                            == ucb::LockScope_EXCLUSIVE &&
                         aSupportedLocks[ n ].Type
                            == ucb::LockType_WRITE )
                        return true;
                }
            }
        }
    }
    return false;
}

//=========================================================================
void Content::lock(
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception )
{
    try
    {
        std::auto_ptr< DAVResourceAccess > xResAccess;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        }

        uno::Any aOwnerAny;
        aOwnerAny
            <<= rtl::OUString::createFromAscii( "http://ucb.openoffice.org" );

        ucb::Lock aLock(
            ucb::LockScope_EXCLUSIVE,
            ucb::LockType_WRITE,
            ucb::LockDepth_ZERO,
            aOwnerAny,
            180, // lock timeout in secs
            //-1, // infinite lock
            uno::Sequence< ::rtl::OUString >() );

        xResAccess->LOCK( aLock, Environment );

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
        }
    }
    catch ( DAVException const & e )
    {
        cancelCommandExecution( e, Environment, sal_False );
        // Unreachable
    }
}

//=========================================================================
void Content::unlock(
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception )
{
    try
    {
        std::auto_ptr< DAVResourceAccess > xResAccess;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        }

        xResAccess->UNLOCK( Environment );

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
        }
    }
    catch ( DAVException const & e )
    {
        cancelCommandExecution( e, Environment, sal_False );
        // Unreachable
    }
}

//=========================================================================
sal_Bool Content::exchangeIdentity(
    const uno::Reference< ucb::XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return sal_False;

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< ucb::XContent > xThis = this;

    // Already persistent?
    if ( m_bTransient )
    {
        OSL_ENSURE( sal_False, "Content::exchangeIdentity - Not persistent!" );
        return sal_False;
    }

    // Exchange own identitity.

    // Fail, if a content with given id already exists.
//  if ( !hasData( xNewId ) )
    {
        rtl::OUString aOldURL = m_xIdentifier->getContentIdentifier();

        aGuard.clear();
        if ( exchange( xNewId ) )
        {
            // Process instanciated children...

            ContentRefList aChildren;
            queryChildren( aChildren );

            ContentRefList::const_iterator it  = aChildren.begin();
            ContentRefList::const_iterator end = aChildren.end();

            while ( it != end )
            {
                ContentRef xChild = (*it);

                // Create new content identifier for the child...
                uno::Reference< ucb::XContentIdentifier >
                    xOldChildId = xChild->getIdentifier();
                rtl::OUString aOldChildURL
                    = xOldChildId->getContentIdentifier();
                rtl::OUString aNewChildURL
                    = aOldChildURL.replaceAt(
                        0,
                        aOldURL.getLength(),
                        xNewId->getContentIdentifier() );
                uno::Reference< ucb::XContentIdentifier > xNewChildId
                    = new ::ucbhelper::ContentIdentifier(
                        m_xSMgr, aNewChildURL );

                if ( !xChild->exchangeIdentity( xNewChildId ) )
                    return sal_False;

                ++it;
            }
            return sal_True;
        }
    }

    OSL_ENSURE( sal_False,
                "Content::exchangeIdentity - "
                "Panic! Cannot exchange identity!" );
    return sal_False;
}

//=========================================================================
sal_Bool Content::isFolder(
            const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw( uno::Exception )
{
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( m_bTransient )
            return m_bCollection;
    }

    uno::Sequence< beans::Property > aProperties( 1 );
    aProperties[ 0 ].Name   = rtl::OUString::createFromAscii( "IsFolder" );
    aProperties[ 0 ].Handle = -1;
    uno::Reference< sdbc::XRow > xRow( getPropertyValues( aProperties, xEnv ) );
    if ( xRow.is() )
    {
        try
        {
            return xRow->getBoolean( 1 );
        }
        catch ( sdbc::SQLException const & )
        {
        }
    }

    return sal_False;
}

//=========================================================================
uno::Any Content::MapDAVException( const DAVException & e, sal_Bool bWrite )
{
    // Map DAVException...
    uno::Any aException;

    rtl::OUString aURL;
    if ( m_bTransient )
    {
        aURL = getParentURL();
        if ( aURL.lastIndexOf( '/' ) != ( aURL.getLength() - 1 ) )
            aURL += rtl::OUString::createFromAscii( "/" );

        aURL += m_aEscapedTitle;
    }
    else
    {
        aURL = m_xIdentifier->getContentIdentifier();
    }

    switch ( e.getStatus() )
    {
        case SC_NOT_FOUND:
        {
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] <<= beans::PropertyValue(
                rtl::OUString::createFromAscii("Uri"), -1,
                uno::makeAny(aURL),
                beans::PropertyState_DIRECT_VALUE);

            aException <<=
                ucb::InteractiveAugmentedIOException(
                    rtl::OUString::createFromAscii( "Not found!" ),
                    static_cast< cppu::OWeakObject * >( this ),
                    task::InteractionClassification_ERROR,
                    ucb::IOErrorCode_NOT_EXISTING,
                    aArgs );
            return aException;
        }
        default:
            break;
    }

    switch ( e.getError() )
    {
    case DAVException::DAV_HTTP_ERROR:
        {
            if ( bWrite )
                aException <<=
                    ucb::InteractiveNetworkWriteException(
                        e.getData(),
                        static_cast< cppu::OWeakObject * >( this ),
                        task::InteractionClassification_ERROR,
                        e.getData() );
            else
                aException <<=
                    ucb::InteractiveNetworkReadException(
                        e.getData(),
                        static_cast< cppu::OWeakObject * >( this ),
                        task::InteractionClassification_ERROR,
                        e.getData() );
            break;
        }

    case DAVException::DAV_HTTP_LOOKUP:
        aException <<=
            ucb::InteractiveNetworkResolveNameException(
                rtl::OUString(),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                e.getData() );
        break;

// @@@ No matching InteractiveNetwork*Exception
//    case DAVException::DAV_HTTP_AUTH:
//        break;

// @@@ No matching InteractiveNetwork*Exception
//    case DAVException::DAV_HTTP_AUTHPROXY:
//        break;

    case DAVException::DAV_HTTP_CONNECT:
        aException <<=
            ucb::InteractiveNetworkConnectException(
                rtl::OUString(),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                e.getData() );
        break;

// @@@ No matching InteractiveNetwork*Exception
//    case DAVException::DAV_HTTP_TIMEOUT:
//        break;

// @@@ No matching InteractiveNetwork*Exception
//     case DAVException::DAV_HTTP_REDIRECT:
//         break;

// @@@ No matching InteractiveNetwork*Exception
//     case DAVException::DAV_SESSION_CREATE:
//         break;

    case DAVException::DAV_INVALID_ARG:
        aException <<=
            lang::IllegalArgumentException(
                rtl::OUString(),
                static_cast< cppu::OWeakObject * >( this ),
                -1 );
        break;

    case DAVException::DAV_LOCKED:
#if 1
        aException <<=
            ucb::InteractiveLockingLockedException(
                rtl::OUString::createFromAscii( "Locked!" ),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL,
                sal_False ); // not SelfOwned
#else
        {
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] <<= beans::PropertyValue(
                rtl::OUString::createFromAscii("Uri"), -1,
                uno::makeAny(aURL),
                beans::PropertyState_DIRECT_VALUE);

            aException <<=
                ucb::InteractiveAugmentedIOException(
                    rtl::OUString::createFromAscii( "Locked!" ),
                    static_cast< cppu::OWeakObject * >( this ),
                    task::InteractionClassification_ERROR,
                    ucb::IOErrorCode_LOCKING_VIOLATION,
                    aArgs );
        }
#endif
        break;

    case DAVException::DAV_LOCKED_SELF:
        aException <<=
            ucb::InteractiveLockingLockedException(
                rtl::OUString::createFromAscii( "Locked (self)!" ),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL,
                sal_True ); // SelfOwned
        break;

    case DAVException::DAV_NOT_LOCKED:
        aException <<=
            ucb::InteractiveLockingNotLockedException(
                rtl::OUString::createFromAscii( "Not locked!" ),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL );
        break;

    case DAVException::DAV_LOCK_EXPIRED:
        aException <<=
            ucb::InteractiveLockingLockExpiredException(
                rtl::OUString::createFromAscii( "Lock expired!" ),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL );
        break;

    default:
        aException <<=
            ucb::InteractiveNetworkGeneralException(
                rtl::OUString(),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR );
        break;
    }

    return aException;
}

//=========================================================================
// static
bool Content::shouldAccessNetworkAfterException( const DAVException & e )
{
    if ( ( e.getStatus() == SC_NOT_FOUND ) ||
         ( e.getError() == DAVException::DAV_HTTP_LOOKUP ) ||
         ( e.getError() == DAVException::DAV_HTTP_CONNECT ) ||
         ( e.getError() == DAVException::DAV_HTTP_AUTH ) ||
         ( e.getError() == DAVException::DAV_HTTP_AUTHPROXY ) )
        return false;

    return true;
}

//=========================================================================
void Content::cancelCommandExecution(
                const DAVException & e,
                const uno::Reference< ucb::XCommandEnvironment > & xEnv,
                sal_Bool bWrite /* = sal_False */ )
    throw ( uno::Exception )
{
    ucbhelper::cancelCommandExecution( MapDAVException( e, bWrite ), xEnv );
    // Unreachable
}

//=========================================================================
const rtl::OUString
Content::getBaseURI( const std::auto_ptr< DAVResourceAccess > & rResAccess )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // First, try to obtain value of response header "Content-Location".
    if ( m_xCachedProps.get() )
    {
        rtl::OUString aLocation;
        m_xCachedProps->getValue( rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "Content-Location" ) ) ) >>= aLocation;
        if ( aLocation.getLength() )
        {
            try
            {
                // Do not use m_xIdentifier->getContentIdentifier() because it
                // for example does not reflect redirects applied to requests
                // done using the original URI but m_xResAccess' URI does.
                return rtl::Uri::convertRelToAbs( rResAccess->getURL(),
                                                  aLocation );
            }
            catch ( rtl::MalformedUriException const & )
            {
            }
        }
    }

    return rtl::OUString( rResAccess->getURL() );
}

//=========================================================================
const Content::ResourceType & Content::getResourceType(
                    const uno::Reference< ucb::XCommandEnvironment >& xEnv,
                    const std::auto_ptr< DAVResourceAccess > & rResAccess )
    throw ( uno::Exception )
{
    if ( m_eResourceType == UNKNOWN )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        ResourceType eResourceType;
        eResourceType = m_eResourceType;

        const rtl::OUString & rURL = rResAccess->getURL();
        const rtl::OUString aScheme(
            rURL.copy( 0, rURL.indexOf( ':' ) ).toAsciiLowerCase() );

        try
        {
            // Try to fetch some frequently used property value, e.g. those
            // used when loading documents... along with identifying whether
            // this is a DAV resource.
            std::vector< DAVResource > resources;
            std::vector< rtl::OUString > aPropNames;
            uno::Sequence< beans::Property > aProperties( 5 );
            aProperties[ 0 ].Name
                = rtl::OUString::createFromAscii( "IsFolder" );
            aProperties[ 1 ].Name
                = rtl::OUString::createFromAscii( "IsDocument" );
            aProperties[ 2 ].Name
                = rtl::OUString::createFromAscii( "IsReadOnly" );
            aProperties[ 3 ].Name
                = rtl::OUString::createFromAscii( "MediaType" );
            aProperties[ 4 ].Name
                = DAVProperties::SUPPORTEDLOCK;

            ContentProperties::UCBNamesToDAVNames(
                aProperties, aPropNames );

            rResAccess->PROPFIND(
                DAVZERO, aPropNames, resources, xEnv );

            // TODO - is this really only one?
            if ( resources.size() == 1 )
            {
                m_xCachedProps.reset(
                    new CachableContentProperties( resources[ 0 ] ) );
                m_xCachedProps->containsAllNames(
                    aProperties, m_aFailedPropNames );
            }

            eResourceType = DAV;
        }
        catch ( DAVException const & e )
        {
            rResAccess->resetUri();

            if ( e.getStatus() == SC_METHOD_NOT_ALLOWED )
            {
                // Status SC_METHOD_NOT_ALLOWED is a safe indicator that the
                // resource is NON_DAV
                eResourceType = NON_DAV;
            }
            // cancel command execution is case that no user authentication data has been provided.
            if ( e.getError() == DAVException::DAV_HTTP_NOAUTH )
            {
                cancelCommandExecution( e, uno::Reference< ucb::XCommandEnvironment >() );
            }
        }
        m_eResourceType = eResourceType;
    }
    return m_eResourceType;
}

//=========================================================================
const Content::ResourceType & Content::getResourceType(
                    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw ( uno::Exception )
{
    return getResourceType( xEnv, m_xResAccess );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
