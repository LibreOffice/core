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
#include <com/sun/star/ucb/OpenCommandArgument3.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#include <com/sun/star/ucb/PropertyCommandArgument.hpp>
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
#include "NeonUri.hxx"
#include "UCBDeadPropertyValue.hxx"

using namespace com::sun::star;
using namespace webdav_ucp;


// Content Implementation.


// ctr for content on an existing webdav resource
Content::Content(
          const uno::Reference< uno::XComponentContext >& rxContext,
          ContentProvider* pProvider,
          const uno::Reference< ucb::XContentIdentifier >& Identifier,
          rtl::Reference< DAVSessionFactory > const & rSessionFactory )
  throw ( ucb::ContentCreationException )
: ContentImplHelper( rxContext, pProvider, Identifier ),
  m_eResourceType( UNKNOWN ),
  m_eResourceTypeForLocks( UNKNOWN ),
  m_pProvider( pProvider ),
  m_rSessionFactory( rSessionFactory ),
  m_bTransient( false ),
  m_bCollection( false ),
  m_bDidGetOrHead( false )
{
    try
    {
        m_xResAccess.reset( new DAVResourceAccess(
                rxContext,
                rSessionFactory,
                Identifier->getContentIdentifier() ) );

        NeonUri aURI( Identifier->getContentIdentifier() );
        m_aEscapedTitle = aURI.GetPathBaseName();
    }
    catch ( DAVException const & )
    {
        throw ucb::ContentCreationException();
    }
}


// ctr for content on an non-existing webdav resource
Content::Content(
            const uno::Reference< uno::XComponentContext >& rxContext,
            ContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            rtl::Reference< DAVSessionFactory > const & rSessionFactory,
            bool isCollection )
  throw ( ucb::ContentCreationException )
: ContentImplHelper( rxContext, pProvider, Identifier ),
  m_eResourceType( UNKNOWN ),
  m_eResourceTypeForLocks( UNKNOWN ),
  m_pProvider( pProvider ),
  m_bTransient( true ),
  m_bCollection( isCollection ),
  m_bDidGetOrHead( false )
{
    try
    {
        m_xResAccess.reset( new DAVResourceAccess(
            rxContext, rSessionFactory, Identifier->getContentIdentifier() ) );
    }
    catch ( DAVException const & )
    {
        throw ucb::ContentCreationException();
    }

    // Do not set m_aEscapedTitle here! Content::insert relays on this!!!
}


// virtual
Content::~Content()
{
}


// XInterface methods.


// virtual
void SAL_CALL Content::acquire()
    throw( )
{
    ContentImplHelper::acquire();
}


// virtual
void SAL_CALL Content::release()
    throw( )
{
    ContentImplHelper::release();
}


// virtual
uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException, std::exception )
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
            uno::Reference< task::XInteractionHandler > xIH(
                task::PasswordContainerInteractionHandler::create( m_xContext ) );

            // Supply a command env to isFolder() that contains an interaction
            // handler that uses the password container service to obtain
            // credentials without displaying a password gui.

            uno::Reference< ucb::XCommandEnvironment > xCmdEnv(
                ucb::CommandEnvironment::create(
                   m_xContext,
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


// XTypeProvider methods.


XTYPEPROVIDER_COMMON_IMPL( Content );


// virtual
uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
    throw( uno::RuntimeException, std::exception )
{
    bool bFolder = false;
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

    cppu::OTypeCollection * pCollection = nullptr;

    if ( bFolder )
    {
        static cppu::OTypeCollection* pFolderTypes = nullptr;

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
        static cppu::OTypeCollection* pDocumentTypes = nullptr;

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


// XServiceInfo methods.


// virtual
OUString SAL_CALL Content::getImplementationName()
    throw( uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.ucb.WebDAVContent" );
}


// virtual
uno::Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException, std::exception )
{
    uno::Sequence<OUString> aSNS { WEBDAV_CONTENT_SERVICE_NAME };
    return aSNS;
}


// XContent methods.


// virtual
OUString SAL_CALL Content::getContentType()
    throw( uno::RuntimeException, std::exception )
{
    bool bFolder = false;
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
        return OUString( WEBDAV_COLLECTION_TYPE );

    return OUString( WEBDAV_CONTENT_TYPE );
}


// XCommandProcessor methods.


// virtual
uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception,
           ucb::CommandAbortedException,
           uno::RuntimeException, std::exception )
{
    SAL_INFO( "ucb.ucp.webdav", "Content::execute: start: command: " <<
              aCommand.Name << ", env: " <<
              (Environment.is() ? "present" : "missing") );

    uno::Any aRet;

    if ( aCommand.Name == "getPropertyValues" )
    {

        // getPropertyValues


        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= getPropertyValues( Properties, Environment );
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {

        // setPropertyValues


        uno::Sequence< beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.getLength() )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "No properties!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= setPropertyValues( aProperties, Environment );
    }
    else if ( aCommand.Name == "getPropertySetInfo" )
    {

        // getPropertySetInfo


        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo( Environment,
                                     false /* don't cache data */ );
    }
    else if ( aCommand.Name == "getCommandInfo" )
    {

        // getCommandInfo


        // Note: Implemented by base class.
        aRet <<= getCommandInfo( Environment, false );
    }
    else if ( aCommand.Name == "open" )
    {

        // open


        ucb::OpenCommandArgument3 aOpenCommand;
        ucb::OpenCommandArgument2 aTmp;
        if ( !( aCommand.Argument >>= aTmp ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            // compat mode, extract Arg2 info into newer structure
            aOpenCommand.Mode = aTmp.Mode;
            aOpenCommand.Priority = aTmp.Priority;
            aOpenCommand.Sink = aTmp.Sink;
            aOpenCommand.Properties = aTmp.Properties;
            aOpenCommand.SortingInfo = aTmp.SortingInfo;
        }

        aRet = open( aOpenCommand, Environment );

    }
    else if ( aCommand.Name == "insert" )
    {

        // insert


        ucb::InsertCommandArgument arg;
        if ( !( aCommand.Argument >>= arg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        insert( arg.Data, arg.ReplaceExisting, Environment );
    }
    else if ( aCommand.Name == "delete" )
    {

        // delete


        bool bDeletePhysical = false;
        aCommand.Argument >>= bDeletePhysical;

//  KSO: Ignore parameter and destroy the content, if you don't support
//       putting objects into trashcan. ( Since we do not have a trash can
//       service yet (src603), you actually have no other choice. )
//      if ( bDeletePhysical )
//  {
        try
        {
            std::unique_ptr< DAVResourceAccess > xResAccess;
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
            cancelCommandExecution( e, Environment, true );
            // Unreachable
        }
//      }

        // Propagate destruction.
        destroy( bDeletePhysical );

        // Remove own and all children's Additional Core Properties.
        removeAdditionalPropertySet( true );
    }
    else if ( aCommand.Name == "transfer" && isFolder( Environment ) )
    {

        // transfer
        //  ( Not available at documents )


        ucb::TransferInfo transferArgs;
        if ( !( aCommand.Argument >>= transferArgs ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                  OUString( "Wrong argument type!" ),
                                  static_cast< cppu::OWeakObject * >( this ),
                                  -1 ) ),
                Environment );
            // Unreachable
        }

        transfer( transferArgs, Environment );
    }
    else if ( aCommand.Name == "post" )
    {

        // post


        ucb::PostCommandArgument2 aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        post( aArg, Environment );
    }
    else if ( aCommand.Name == "lock" )
    {

        // lock

        ResourceType eType = resourceTypeForLocks( Environment );
        // when the resource is not yet present the lock is used to create it
        // see: http://tools.ietf.org/html/rfc4918#section-7.3
        // If the resource doesn't exists and the lock is not enabled (DAV with
        // no lock or a simple web) the error will be dealt with inside lock() method
        if ( eType == NOT_FOUND ||
            eType == DAV )
        {
            lock( Environment );
            if ( eType == NOT_FOUND )
            {
                m_eResourceType = UNKNOWN;  // lock may have created it, need to check again
                m_eResourceTypeForLocks = UNKNOWN;
            }
        }
    }
    else if ( aCommand.Name == "unlock" )
    {

        // unlock
        if ( resourceTypeForLocks( Environment ) == DAV )
            unlock( Environment );
    }
    else if ( aCommand.Name == "createNewContent" && isFolder( Environment ) )
    {

        // createNewContent


        ucb::ContentInfo aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet = uno::makeAny( createNewContent( aArg ) );
    }
    else if ( aCommand.Name == "addProperty" )
    {
        ucb::PropertyCommandArgument aPropArg;
        if ( !( aCommand.Argument >>= aPropArg ))
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    "Wrong argument type!",
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
        }

        // TODO when/if XPropertyContainer is removed,
        // the command execution can be canceled in addProperty
        try
        {
            addProperty( aPropArg, Environment );
        }
        catch ( const beans::PropertyExistException &e )
        {
            ucbhelper::cancelCommandExecution( uno::makeAny( e ), Environment );
        }
        catch ( const beans::IllegalTypeException&e )
        {
            ucbhelper::cancelCommandExecution( uno::makeAny( e ), Environment );
        }
        catch ( const lang::IllegalArgumentException&e )
        {
            ucbhelper::cancelCommandExecution( uno::makeAny( e ), Environment );
        }
    }
    else if ( aCommand.Name == "removeProperty" )
    {
        OUString sPropName;
        if ( !( aCommand.Argument >>= sPropName ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    "Wrong argument type!",
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
        }

        // TODO when/if XPropertyContainer is removed,
        // the command execution can be canceled in removeProperty
        try
        {
            removeProperty( sPropName, Environment );
        }
        catch( const beans::UnknownPropertyException &e )
        {
            ucbhelper::cancelCommandExecution( uno::makeAny( e ), Environment );
        }
        catch( const beans::NotRemoveableException &e )
        {
            ucbhelper::cancelCommandExecution( uno::makeAny( e ), Environment );
        }
    }
    else
    {

        // Unsupported command


        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                              aCommand.Name,
                              static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        // Unreachable
    }

    SAL_INFO( "ucb.ucp.webdav", "Content::execute: end: command: " << aCommand.Name );

    return aRet;
}


// virtual
void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
    throw( uno::RuntimeException, std::exception )
{
    try
    {
        std::unique_ptr< DAVResourceAccess > xResAccess;
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


// XPropertyContainer methods.


void Content::addProperty( const ucb::PropertyCommandArgument& aCmdArg,
                           const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw( beans::PropertyExistException,
           beans::IllegalTypeException,
           lang::IllegalArgumentException,
           uno::RuntimeException,
           std::exception )
{
//    if ( m_bTransient )
//   @@@ ???

    if ( aCmdArg.Property.Name.isEmpty() )
        throw lang::IllegalArgumentException(
            "\"addProperty\" with empty Property.Name",
            static_cast< cppu::OWeakObject * >( this ),
            -1 );

    // Check property type.
    if ( !UCBDeadPropertyValue::supportsType( aCmdArg.Property.Type ) )
    {
        throw beans::IllegalTypeException(
            "\"addProperty\" unsupported Property.Type",
            static_cast< cppu::OWeakObject * >( this ) );
    }

    if ( aCmdArg.DefaultValue.hasValue()
         && aCmdArg.DefaultValue.getValueType() != aCmdArg.Property.Type )
    {
        throw beans::IllegalTypeException(
            "\"addProperty\" DefaultValue does not match Property.Type",
            static_cast< ::cppu::OWeakObject * >( this ) );
    }


    // Make sure a property with the requested name does not already
    // exist in dynamic and static(!) properties.


    // Take into account special properties with custom namespace
    // using <prop:the_propname xmlns:prop="the_namespace">
    OUString aSpecialName;
    bool bIsSpecial = DAVProperties::isUCBSpecialProperty(
        aCmdArg.Property.Name, aSpecialName );

    // Note: This requires network access!
    if ( getPropertySetInfo( xEnv, false /* don't cache data */ )
             ->hasPropertyByName(
                 bIsSpecial ? aSpecialName : aCmdArg.Property.Name ) )
    {
        // Property does already exist.
        throw beans::PropertyExistException();
    }


    // Add a new dynamic property.


    ProppatchValue aValue(
        PROPSET, aCmdArg.Property.Name, aCmdArg.DefaultValue );

    std::vector< ProppatchValue > aProppatchValues;
    aProppatchValues.push_back( aValue );

    try
    {
        // Set property value at server.
        std::unique_ptr< DAVResourceAccess > xResAccess;
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
            bIsSpecial ? aSpecialName : aCmdArg.Property.Name,
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
                bIsSpecial ? aSpecialName : aCmdArg.Property.Name,
                aCmdArg.Property.Attributes, aCmdArg.DefaultValue );
        }
        else
        {
            if ( shouldAccessNetworkAfterException( e ) )
            {
                try
                {
                    ResourceType eType = getResourceType( xEnv );
                    switch ( eType )
                    {
                    case UNKNOWN:
                    case DAV:
                        throw lang::IllegalArgumentException();

                    case FTP:
                    case NON_DAV:
                        // Store property locally.
                        ContentImplHelper::addProperty(
                            bIsSpecial ? aSpecialName : aCmdArg.Property.Name,
                            aCmdArg.Property.Attributes, aCmdArg.DefaultValue );
                        break;

                    default:
                        SAL_WARN( "ucb.ucp.webdav", "Content::addProperty - "
                                    "Unsupported resource type!" );
                        break;
                    }
                }
                catch ( uno::Exception const & )
                {
                    SAL_WARN( "ucb.ucp.webdav", "Content::addProperty - "
                                "Unable to determine resource type!" );
                }
            }
            else
            {
                SAL_WARN( "ucb.ucp.webdav", "Content::addProperty - "
                            "Unable to determine resource type!" );
            }
        }
    }
}

void Content::removeProperty( const OUString& Name,
                              const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw( beans::UnknownPropertyException,
           beans::NotRemoveableException,
           uno::RuntimeException,
           std::exception )
{

    // Try to remove property from server.


    try
    {
        std::vector< ProppatchValue > aProppatchValues;
        ProppatchValue aValue( PROPREMOVE, Name, uno::Any() );
        aProppatchValues.push_back( aValue );

        // Remove property value from server.
        std::unique_ptr< DAVResourceAccess > xResAccess;
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
                    ResourceType eType = getResourceType( xEnv );
                    switch ( eType )
                    {
                        case UNKNOWN:
                        case DAV:
                            throw beans::UnknownPropertyException();

                        case FTP:
                        case NON_DAV:
                            // Try to remove property from local store.
                            ContentImplHelper::removeProperty( Name );
                            break;

                        default:
                            SAL_WARN( "ucb.ucp.webdav", "Content::removeProperty - "
                                        "Unsupported resource type!" );
                            break;
                    }
                }
                catch ( uno::Exception const & )
                {
                    SAL_WARN( "ucb.ucp.webdav", "Content::removeProperty - "
                                "Unable to determine resource type!" );
                }
            }
            else
            {
                SAL_WARN( "ucb.ucp.webdav", "Content::removeProperty - "
                            "Unable to determine resource type!" );
//                throw beans::UnknownPropertyException();
            }
        }
    }
}

// virtual
void SAL_CALL Content::addProperty( const OUString& Name,
                                    sal_Int16 Attributes,
                                    const uno::Any& DefaultValue )
    throw( beans::PropertyExistException,
           beans::IllegalTypeException,
           lang::IllegalArgumentException,
           uno::RuntimeException, std::exception )
{
    beans::Property aProperty;
    aProperty.Name = Name;
    aProperty.Type = DefaultValue.getValueType();
    aProperty.Attributes = Attributes;
    aProperty.Handle = -1;

    addProperty( ucb::PropertyCommandArgument( aProperty, DefaultValue ),
                 uno::Reference< ucb::XCommandEnvironment >());
}

// virtual
void SAL_CALL Content::removeProperty( const OUString& Name )
    throw( beans::UnknownPropertyException,
           beans::NotRemoveableException,
           uno::RuntimeException, std::exception )
{
    removeProperty( Name,
                    uno::Reference< ucb::XCommandEnvironment >() );
}


// XContentCreator methods.


// virtual
uno::Sequence< ucb::ContentInfo > SAL_CALL
Content::queryCreatableContentsInfo()
    throw( uno::RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< ucb::ContentInfo > aSeq( 2 );

    // document.
    aSeq.getArray()[ 0 ].Type = WEBDAV_CONTENT_TYPE;
    aSeq.getArray()[ 0 ].Attributes
        = ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
          | ucb::ContentInfoAttribute::KIND_DOCUMENT;

    beans::Property aProp;
    m_pProvider->getProperty(
        "Title", aProp );

    uno::Sequence< beans::Property > aDocProps( 1 );
    aDocProps.getArray()[ 0 ] = aProp;
    aSeq.getArray()[ 0 ].Properties = aDocProps;

    // folder.
    aSeq.getArray()[ 1 ].Type = WEBDAV_COLLECTION_TYPE;
    aSeq.getArray()[ 1 ].Attributes
        = ucb::ContentInfoAttribute::KIND_FOLDER;

    uno::Sequence< beans::Property > aFolderProps( 1 );
    aFolderProps.getArray()[ 0 ] = aProp;
    aSeq.getArray()[ 1 ].Properties = aFolderProps;
    return aSeq;
}


// virtual
uno::Reference< ucb::XContent > SAL_CALL
Content::createNewContent( const ucb::ContentInfo& Info )
    throw( uno::RuntimeException, std::exception )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( Info.Type.isEmpty() )
        return uno::Reference< ucb::XContent >();

    if ( ( Info.Type != WEBDAV_COLLECTION_TYPE ) && ( Info.Type != WEBDAV_CONTENT_TYPE ) )
        return uno::Reference< ucb::XContent >();

    OUString aURL = m_xIdentifier->getContentIdentifier();

    assert( !aURL.isEmpty() && "WebdavContent::createNewContent - empty identifier!" );

    if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
        aURL += "/";

    bool isCollection;
    if ( Info.Type == WEBDAV_COLLECTION_TYPE )
    {
        aURL += "New_Collection";
        isCollection = true;
    }
    else
    {
        aURL += "New_Content";
        isCollection = false;
    }

    uno::Reference< ucb::XContentIdentifier > xId(
                    new ::ucbhelper::ContentIdentifier( aURL ) );

    // create the local content
    try
    {
        return new ::webdav_ucp::Content( m_xContext,
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


// virtual
OUString Content::getParentURL()
{
    // <scheme>://              -> ""
    // <scheme>://foo           -> ""
    // <scheme>://foo/          -> ""
    // <scheme>://foo/bar       -> <scheme>://foo/
    // <scheme>://foo/bar/      -> <scheme>://foo/
    // <scheme>://foo/bar/abc   -> <scheme>://foo/bar/

    OUString aURL = m_xIdentifier->getContentIdentifier();

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
        return OUString();

    return OUString( aURL.copy( 0, nPos + 1 ) );
}


// Non-interface methods.


// static
uno::Reference< sdbc::XRow > Content::getPropertyValues(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Sequence< beans::Property >& rProperties,
    const ContentProperties& rData,
    const rtl::Reference< ::ucbhelper::ContentProviderImplHelper >& rProvider,
    const OUString& rContentId )
{
    // Note: Empty sequence means "get values of all supported properties".

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow
        = new ::ucbhelper::PropertyValueSet( rxContext );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        uno::Reference< beans::XPropertySet > xAdditionalPropSet;
        bool bTriedToGetAdditionalPropSet = false;

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
                if ( !bTriedToGetAdditionalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet.set(
                            rProvider->getAdditionalPropertySet( rContentId,
                                                                 false ),
                            uno::UNO_QUERY );
                    bTriedToGetAdditionalPropSet = true;
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
        const std::unique_ptr< PropertyValueMap > & xProps = rData.getProperties();

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
            rProvider->getAdditionalPropertySet( rContentId, false ),
            uno::UNO_QUERY );
        xRow->appendPropertySet( xSet );
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
}


uno::Reference< sdbc::XRow > Content::getPropertyValues(
                const uno::Sequence< beans::Property >& rProperties,
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw ( uno::Exception, std::exception )
{
    std::unique_ptr< ContentProperties > xProps;
    std::unique_ptr< ContentProperties > xCachedProps;
    std::unique_ptr< DAVResourceAccess > xResAccess;
    OUString aUnescapedTitle;
    bool bHasAll = false;
    uno::Reference< ucb::XContentIdentifier > xIdentifier;
    rtl::Reference< ::ucbhelper::ContentProviderImplHelper > xProvider;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        aUnescapedTitle = NeonUri::unescape( m_aEscapedTitle );
        xIdentifier.set( m_xIdentifier );
        xProvider.set( m_xProvider.get() );
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );

        // First, ask cache...
        if ( m_xCachedProps.get() )
        {
            xCachedProps.reset( new ContentProperties( *m_xCachedProps.get() ) );

            std::vector< OUString > aMissingProps;
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

        // Obtain values from server...


        // First, identify whether resource is DAV or not
        bool bNetworkAccessAllowed = true;
        ResourceType eType = getResourceType(
            xEnv, xResAccess, &bNetworkAccessAllowed );

        if ( eType == DAV )
        {
            // cache lookup... getResourceType may fill the props cache via
            // PROPFIND!
            if ( m_xCachedProps.get() )
            {
                xCachedProps.reset(
                    new ContentProperties( *m_xCachedProps.get() ) );

                std::vector< OUString > aMissingProps;
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
                std::vector< OUString > aPropNames;

                uno::Sequence< beans::Property > aProperties(
                    rProperties.getLength() );

                if ( !m_aFailedPropNames.empty() )
                {
                    sal_Int32 nProps = 0;
                    sal_Int32 nCount = rProperties.getLength();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        const OUString & rName = rProperties[ n ].Name;

                        std::vector< OUString >::const_iterator it
                            = m_aFailedPropNames.begin();
                        std::vector< OUString >::const_iterator end
                            = m_aFailedPropNames.end();

                        while ( it != end )
                        {
                            if ( *it == rName )
                            {
                                // the failed property in cache is the same as the requested one,
                                // so add it to the requested properties list
                                aProperties[ nProps ] = rProperties[ n ];
                                nProps++;
                                break;
                            }

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

                if ( !aPropNames.empty() )
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
                        bNetworkAccessAllowed = bNetworkAccessAllowed
                            && shouldAccessNetworkAfterException( e );

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
            if( eType != DAV )
                m_bDidGetOrHead = false;

            // All properties obtained already?
            std::vector< OUString > aMissingProps;
            if ( !( xProps.get()
                    && xProps->containsAllNames(
                        rProperties, aMissingProps ) )
                 && !m_bDidGetOrHead )
            {
                // Possibly the missing props can be obtained using a HEAD
                // request.

                std::vector< OUString > aHeaderNames;
                ContentProperties::UCBNamesToHTTPNames(
                    rProperties,
                    aHeaderNames );

                if ( !aHeaderNames.empty() )
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
        NeonUri aUri( xResAccess->getURL() );
        aUnescapedTitle = aUri.GetPathBaseNameUnescaped();

        if ( eType == UNKNOWN )
        {
            xProps.reset( new ContentProperties( aUnescapedTitle ) );
        }

        // For DAV resources we only know the Title, for non-DAV
        // resources we additionally know that it is a document.

        if ( eType == DAV )
        {
            //xProps.reset(
            //    new ContentProperties( aUnescapedTitle ) );
            xProps->addProperty(
                "Title",
                uno::makeAny( aUnescapedTitle ),
                true );
        }
        else
        {
            if ( !xProps.get() )
                xProps.reset( new ContentProperties( aUnescapedTitle, false ) );
            else
                xProps->addProperty(
                    "Title",
                    uno::makeAny( aUnescapedTitle ),
                    true );

            xProps->addProperty(
                "IsFolder",
                uno::makeAny( false ),
                true );
            xProps->addProperty(
                "IsDocument",
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
        const OUString rName = rProperties[ n ].Name;
        if ( rName == "BaseURI" )
        {
            // Add BaseURI property, if requested.
            xProps->addProperty(
                 "BaseURI",
                 uno::makeAny( getBaseURI( xResAccess ) ),
                 true );
        }
        else if ( rName == "CreatableContentsInfo" )
        {
            // Add CreatableContentsInfo property, if requested.
            bool bFolder = false;
            xProps->getValue(
                "IsFolder" )
                    >>= bFolder;
            xProps->addProperty(
                "CreatableContentsInfo",
                uno::makeAny( bFolder
                                  ? queryCreatableContentsInfo()
                                  : uno::Sequence< ucb::ContentInfo >() ),
                true );
        }
    }

    uno::Reference< sdbc::XRow > xResultRow
        = getPropertyValues( m_xContext,
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
        m_aEscapedTitle = NeonUri::escapeSegment( aUnescapedTitle );
    }

    return xResultRow;
}


uno::Sequence< uno::Any > Content::setPropertyValues(
                const uno::Sequence< beans::PropertyValue >& rValues,
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw ( uno::Exception, std::exception )
{
    uno::Reference< ucb::XContentIdentifier >    xIdentifier;
    rtl::Reference< ContentProvider >            xProvider;
    bool bTransient;
    std::unique_ptr< DAVResourceAccess > xResAccess;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        xProvider.set( m_pProvider );
        xIdentifier.set( m_xIdentifier );
        bTransient = m_bTransient;
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
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
    bool bTriedToGetAdditionalPropSet = false;

    bool bExchange = false;
    OUString aNewTitle;
    OUString aOldTitle;
    sal_Int32 nTitlePos = -1;

    uno::Reference< beans::XPropertySetInfo > xInfo;

    const beans::PropertyValue* pValues = rValues.getConstArray();
    sal_Int32 nCount = rValues.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::PropertyValue& rValue = pValues[ n ];
        const OUString & rName = rValue.Name;

        beans::Property aTmpProp;
        xProvider->getProperty( rName, aTmpProp );

        if ( aTmpProp.Attributes & beans::PropertyAttribute::READONLY )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            OUString( "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
            continue;
        }


        // Mandatory props.


        if ( rName == "ContentType" )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                OUString( "Property is read-only!" ),
                static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName == "IsDocument" )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                OUString( "Property is read-only!" ),
                static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName == "IsFolder" )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            OUString( "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName == "Title" )
        {
            OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                // No empty titles!
                if ( !aNewValue.isEmpty() )
                {
                    try
                    {
                        NeonUri aURI( xIdentifier->getContentIdentifier() );
                        aOldTitle = aURI.GetPathBaseNameUnescaped();

                        if ( aNewValue != aOldTitle )
                        {
                            // modified title -> modified URL -> exchange !
                            if ( !bTransient )
                                bExchange = true;

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
                            OUString( "Invalid content identifier!" ),
                            static_cast< cppu::OWeakObject * >( this ),
                            -1 );
                    }
                }
                else
                {
                    aRet[ n ] <<= lang::IllegalArgumentException(
                        OUString( "Empty title not allowed!" ),
                        static_cast< cppu::OWeakObject * >( this ),
                        -1 );
                }
            }
            else
            {
                aRet[ n ] <<= beans::IllegalTypeException(
                    OUString( "Property value has wrong type!" ),
                    static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        else
        {

            // Optional props.


            OUString aSpecialName;
            bool bIsSpecial = DAVProperties::isUCBSpecialProperty(
                rName, aSpecialName );

            if ( !xInfo.is() )
                xInfo = getPropertySetInfo( xEnv,
                                            false /* don't cache data */ );

            if ( !xInfo->hasPropertyByName(
                     bIsSpecial ? aSpecialName : rName ) )
            {
                // Check, whether property exists. Skip otherwise.
                // PROPPATCH::set would add the property automatically, which
                // is not allowed for "setPropertyValues" command!
                aRet[ n ] <<= beans::UnknownPropertyException(
                                OUString( "Property is unknown!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                continue;
            }

            if ( rName == "Size" )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName == "DateCreated" )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName == "DateModified" )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName == "MediaType" )
            {
                // Read-only property!
                // (but could be writable, if 'getcontenttype' would be)
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            if ( rName == "CreatableContentsInfo" )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
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
                    if ( !bTriedToGetAdditionalPropSet &&
                         !xAdditionalPropSet.is() )
                    {
                        xAdditionalPropSet
                            = getAdditionalPropertySet( false );
                        bTriedToGetAdditionalPropSet = true;
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
                                OUString( "No property set for storing the value!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                    }
                }
            }
        }
    } // for

    if ( !bTransient && !aProppatchValues.empty() )
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
            SAL_WARN( "ucb.ucp.webdav", "Content::setPropertyValues - PROPPATCH failed!" );
            cancelCommandExecution( e, xEnv );
            // unreachable
        }
    }

    if ( bExchange )
    {
        // Assemble new content identifier...

        OUString aNewURL = getParentURL();
        if ( aNewURL.lastIndexOf( '/' ) != ( aNewURL.getLength() - 1 ) )
            aNewURL += "/";

        aNewURL += NeonUri::escapeSegment( aNewTitle );

        uno::Reference< ucb::XContentIdentifier > xNewId
            = new ::ucbhelper::ContentIdentifier( aNewURL );
        uno::Reference< ucb::XContentIdentifier > xOldId = xIdentifier;

        try
        {
            NeonUri sourceURI( xOldId->getContentIdentifier() );
            NeonUri targetURI( xNewId->getContentIdentifier() );
            targetURI.SetScheme( sourceURI.GetScheme() );

            xResAccess->MOVE(
                sourceURI.GetPath(), targetURI.GetURI(), false, xEnv );
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
                aNewTitle.clear();

                // Set error .
                aRet[ nTitlePos ] <<= uno::Exception(
                    OUString("Exchange failed!"),
                    static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        catch ( DAVException const & e )
        {
            // Do not set new title!
            aNewTitle.clear();

            // Set error .
            aRet[ nTitlePos ] <<= MapDAVException( e, true );
        }
    }

    if ( !aNewTitle.isEmpty() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        aEvent.PropertyName = "Title";
        aEvent.OldValue     = uno::makeAny( aOldTitle );
        aEvent.NewValue     = uno::makeAny( aNewTitle );

        m_aEscapedTitle     = NeonUri::escapeSegment( aNewTitle );

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


uno::Any Content::open(
                const ucb::OpenCommandArgument3 & rArg,
                const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw (uno::Exception, std::exception)
{
    uno::Any aRet;

    bool bOpenFolder = ( ( rArg.Mode == ucb::OpenMode::ALL ) ||
                             ( rArg.Mode == ucb::OpenMode::FOLDERS ) ||
                             ( rArg.Mode == ucb::OpenMode::DOCUMENTS ) );
    if ( bOpenFolder )
    {
        if ( isFolder( xEnv ) )
        {
            // Open collection.

            uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet( m_xContext, this, rArg, xEnv );
            aRet <<= xSet;
        }
        else
        {
            // Error: Not a folder!

            OUStringBuffer aMsg;
            if ( getResourceType( xEnv ) == FTP )
            {
                aMsg.append( "FTP over HTTP proxy: resource cannot "
                                  "be opened as folder! Wrong Open Mode!" );
            }
            else
            {
                aMsg.append( "Non-folder resource cannot be "
                                  "opened as folder! Wrong Open Mode!" );
            }

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
                            OUString(),
                            static_cast< cppu::OWeakObject * >( this ),
                            sal_Int16( rArg.Mode ) ) ),
                xEnv );
            // Unreachable
        }

        uno::Reference< io::XOutputStream > xOut( rArg.Sink, uno::UNO_QUERY );
        if ( xOut.is() )
        {
            // PUSH: write data
            try
            {
                std::unique_ptr< DAVResourceAccess > xResAccess;

                {
                    osl::MutexGuard aGuard( m_aMutex );

                    xResAccess.reset(
                        new DAVResourceAccess( *m_xResAccess.get() ) );
                }

                xResAccess->setFlags( rArg.OpeningFlags );
                DAVResource aResource;
                std::vector< OUString > aHeaders;

                xResAccess->GET( xOut, aHeaders, aResource, xEnv );
                m_bDidGetOrHead = true;

                {
                    osl::MutexGuard aGuard( m_aMutex );

                    // cache headers.
                    if ( !m_xCachedProps.get())
                        m_xCachedProps.reset(
                            new CachableContentProperties( ContentProperties( aResource ) ) );
                    else
                        m_xCachedProps->addProperties( ContentProperties( aResource ) );

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
            uno::Reference< io::XActiveDataSink > xDataSink( rArg.Sink, uno::UNO_QUERY );
            if ( xDataSink.is() )
            {
                // PULL: wait for client read
                try
                {
                    std::unique_ptr< DAVResourceAccess > xResAccess;
                    {
                        osl::MutexGuard aGuard( m_aMutex );

                        xResAccess.reset(
                            new DAVResourceAccess( *m_xResAccess.get() ) );
                    }

                    xResAccess->setFlags( rArg.OpeningFlags );

                    // fill inputsream sync; return if all data present
                    DAVResource aResource;
                    std::vector< OUString > aHeaders;

                    uno::Reference< io::XInputStream > xIn
                        = xResAccess->GET( aHeaders, aResource, xEnv );
                    m_bDidGetOrHead = true;

                    {
                        osl::MutexGuard aGuard( m_aMutex );

                        // cache headers.
                        if ( !m_xCachedProps.get())
                            m_xCachedProps.reset(
                                new CachableContentProperties( ContentProperties( aResource ) ) );
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
                            OUString(),
                            static_cast< cppu::OWeakObject * >( this ),
                            rArg.Sink ) ),
                    xEnv );
                // Unreachable
            }
        }
    }

    return aRet;
}

void Content::post(
                const ucb::PostCommandArgument2 & rArg,
                const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception, std::exception )
{
    uno::Reference< io::XActiveDataSink > xSink( rArg.Sink, uno::UNO_QUERY );
    if ( xSink.is() )
    {
        try
        {
            std::unique_ptr< DAVResourceAccess > xResAccess;
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
            cancelCommandExecution( e, xEnv, true );
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
                std::unique_ptr< DAVResourceAccess > xResAccess;
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
                cancelCommandExecution( e, xEnv, true );
                // Unreachable
            }
        }
        else
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny(
                    ucb::UnsupportedDataSinkException(
                        OUString(),
                        static_cast< cppu::OWeakObject * >( this ),
                        rArg.Sink ) ),
                xEnv );
            // Unreachable
        }
    }
}


void Content::queryChildren( ContentRefList& rChildren )
{
    // Obtain a list with a snapshot of all currently instantiated contents
    // from provider and extract the contents which are direct children
    // of this content.

    ::ucbhelper::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nURLPos = aURL.lastIndexOf( '/' );

    if ( nURLPos != ( aURL.getLength() - 1 ) )
    {
        // No trailing slash found. Append.
        aURL += "/";
    }

    sal_Int32 nLen = aURL.getLength();

    ::ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

    while ( it != end )
    {
        ::ucbhelper::ContentImplHelperRef xChild = (*it);
        OUString aChildURL
            = xChild->getIdentifier()->getContentIdentifier();

        // Is aURL a prefix of aChildURL?
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.startsWith( aURL ) ) )
        {
            sal_Int32 nPos = nLen;
            nPos = aChildURL.indexOf( '/', nPos );

            if ( ( nPos == -1 ) ||
                 ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                // No further slashes / only a final slash. It's a child!
                rChildren.push_back(
                    ::webdav_ucp::Content::ContentRef(
                        static_cast< ::webdav_ucp::Content * >(
                            xChild.get() ) ) );
            }
        }
        ++it;
    }
}


void Content::insert(
        const uno::Reference< io::XInputStream > & xInputStream,
        bool bReplaceExisting,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception, std::exception )
{
    bool bTransient, bCollection;
    OUString aEscapedTitle;
    std::unique_ptr< DAVResourceAccess > xResAccess;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        bTransient    = m_bTransient;
        bCollection   = m_bCollection;
        aEscapedTitle = m_aEscapedTitle;
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
    }

    // Check, if all required properties are present.

    if ( aEscapedTitle.isEmpty() )
    {
        SAL_WARN( "ucb.ucp.webdav", "Content::insert - Title missing!" );

        uno::Sequence<OUString> aProps { "Title" };
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::MissingPropertiesException(
                                OUString(),
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
        if ( !bTransient || !bCollection )
        {
            ucb::UnsupportedNameClashException aEx(
                OUString( "Unable to write without overwrite!" ),
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
                        bReplaceExisting = true;
                        break;

                    case ucbhelper::CONTINUATION_DISAPPROVE:
                        // Abort.
                        throw ucb::CommandFailedException(
                                    OUString(),
                                    uno::Reference< uno::XInterface >(),
                                    aExAsAny );
//                            break;

                    default:
                        SAL_WARN( "ucb.ucp.webdav", "Content::insert - "
                                    "Unknown interaction selection!" );
                        throw ucb::CommandFailedException(
                                    OUString( "Unknown interaction selection!" ),
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
        OUString aURL = getParentURL();
        if ( aURL.lastIndexOf( '/' ) != ( aURL.getLength() - 1 ) )
            aURL += "/";

        aURL += aEscapedTitle;

        try
        {
            xResAccess->setURL( aURL );

            if ( bCollection )
                xResAccess->MKCOL( Environment );
            else
            {
                xResAccess->PUT( xInputStream, Environment );
            }
            // no error , set the resourcetype to unknown type
            // the resource may have transitioned from NOT FOUND or UNKNOWN to something else
            // depending on the server behaviour
            // this will force a recheck of the resource type
            m_eResourceType = UNKNOWN;
            m_eResourceTypeForLocks = UNKNOWN;
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
                            cancelCommandExecution( e, Environment, true );
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
                        OUString aTitle;
                        try
                        {
                            NeonUri aURI( aURL );
                            aTitle = aURI.GetPathBaseNameUnescaped();
                        }
                        catch ( DAVException const & )
                        {
                        }

                        ucbhelper::cancelCommandExecution(
                            uno::makeAny(
                                ucb::NameClashException(
                                    OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    task::InteractionClassification_ERROR,
                                    aTitle ) ),
                            Environment );
                        // Unreachable
                    }
                }
            }

            cancelCommandExecution( except, Environment, true );
            // Unreachable
        }

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xIdentifier = new ::ucbhelper::ContentIdentifier( aURL );
        }

        inserted();

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_bTransient = false;
        }
    }
    else
    {
        if ( !xInputStream.is() )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny(
                    ucb::MissingInputStreamException(
                        OUString(),
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
            cancelCommandExecution( e, Environment, true );
            // Unreachable
        }
    }

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
    }
}


void Content::transfer(
        const ucb::TransferInfo & rArgs,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception, std::exception )
{
    uno::Reference< ucb::XContentIdentifier >    xIdentifier;
    uno::Reference< ucb::XContentProvider >      xProvider;
    std::unique_ptr< DAVResourceAccess > xResAccess;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        xIdentifier.set( m_xIdentifier );
        xProvider.set( m_xProvider.get() );
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
    }

    OUString aTargetURI;
    try
    {
        NeonUri sourceURI( rArgs.SourceURL );
        NeonUri targetURI( xIdentifier->getContentIdentifier() );
        aTargetURI = targetURI.GetPathBaseNameUnescaped();

        // Check source's and target's URL scheme

        OUString aScheme = sourceURI.GetScheme().toAsciiLowerCase();
        if ( aScheme == WEBDAV_URL_SCHEME )
        {
            sourceURI.SetScheme(
                HTTP_URL_SCHEME );
        }
        else if ( aScheme == WEBDAVS_URL_SCHEME )
        {
            sourceURI.SetScheme(
                HTTPS_URL_SCHEME );
        }
        else if ( aScheme == DAV_URL_SCHEME )
        {
            sourceURI.SetScheme(
                HTTP_URL_SCHEME );
        }
        else if ( aScheme == DAVS_URL_SCHEME )
        {
            sourceURI.SetScheme(
                HTTPS_URL_SCHEME );
        }
        else
        {
            if ( aScheme != HTTP_URL_SCHEME && aScheme != HTTPS_URL_SCHEME )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        ucb::InteractiveBadTransferURLException(
                            OUString( "Unsupported URL scheme!" ),
                            static_cast< cppu::OWeakObject * >( this ) ) ),
                    Environment );
                // Unreachable
            }
        }

        aScheme = targetURI.GetScheme().toAsciiLowerCase();
        if ( aScheme == WEBDAV_URL_SCHEME )
            targetURI.SetScheme(
                HTTP_URL_SCHEME );
        else if ( aScheme == WEBDAVS_URL_SCHEME )
            targetURI.SetScheme(
                HTTPS_URL_SCHEME );
        else if ( aScheme == DAV_URL_SCHEME )
            targetURI.SetScheme(
                HTTP_URL_SCHEME );
        else if ( aScheme == DAVS_URL_SCHEME )
            targetURI.SetScheme(
                HTTPS_URL_SCHEME );

        // @@@ This implementation of 'transfer' only works
        //     if the source and target are located at same host.
        //     (Neon does not support cross-server copy/move)

        // Check for same host

        if ( !sourceURI.GetHost().isEmpty() &&
             ( sourceURI.GetHost() != targetURI.GetHost() ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( ucb::InteractiveBadTransferURLException(
                                OUString( "Different hosts!" ),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
                Environment );
            // Unreachable
        }

        OUString aTitle = rArgs.NewTitle;

        if ( aTitle.isEmpty() )
            aTitle = sourceURI.GetPathBaseNameUnescaped();

        if ( aTitle == "/" )
        {
            // kso: ???
            aTitle.clear();
        }

        targetURI.AppendPath( aTitle );

        OUString aTargetURL = xIdentifier->getContentIdentifier();
        if ( ( aTargetURL.lastIndexOf( '/' ) + 1 )
                != aTargetURL.getLength() )
            aTargetURL += "/";

        aTargetURL += aTitle;

        uno::Reference< ucb::XContentIdentifier > xTargetId
            = new ::ucbhelper::ContentIdentifier( aTargetURL );

        DAVResourceAccess aSourceAccess( m_xContext,
                                         xResAccess->getSessionFactory(),
                                         sourceURI.GetURI() );

        if ( rArgs.MoveData )
        {
            uno::Reference< ucb::XContentIdentifier > xId
                = new ::ucbhelper::ContentIdentifier( rArgs.SourceURL );

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
                xSource->destroy( true );
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

        // Announce transferred content in its new folder.
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
                case ucb::NameClash::ERROR:
                {
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny(
                            ucb::NameClashException(
                                OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                task::InteractionClassification_ERROR,
                                aTargetURI ) ),
                        Environment );
                    SAL_FALLTHROUGH; // Unreachable
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
                                OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                rArgs.NameClash ) ),
                        Environment );
                    // Unreachable
                }
            }
        }

        cancelCommandExecution( e, Environment, true );
        // Unreachable
    }

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
    }
}


void Content::destroy( bool bDeletePhysical )
    throw( uno::Exception, std::exception )
{
    // @@@ take care about bDeletePhysical -> trashcan support
    uno::Reference< ucb::XContent > xThis = this;

    deleted();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Process instantiated children...

    ::webdav_ucp::Content::ContentRefList aChildren;
    queryChildren( aChildren );

    ContentRefList::const_iterator it  = aChildren.begin();
    ContentRefList::const_iterator end = aChildren.end();

    while ( it != end )
    {
        (*it)->destroy( bDeletePhysical );
        ++it;
    }
}

// returns the resource type, to be checked for locks
Content::ResourceType Content::resourceTypeForLocks(
  const uno::Reference< ucb::XCommandEnvironment >& Environment )
{
    ResourceType eResourceTypeForLocks = UNKNOWN;
    {
        osl::MutexGuard g(m_aMutex);
        //check if cache contains what we need, usually the first PROPFIND on the URI has supported lock
        std::unique_ptr< ContentProperties > xProps;
        if ( m_xCachedProps.get() )
        {
            std::unique_ptr< ContentProperties > xCachedProps;
            xCachedProps.reset( new ContentProperties( *m_xCachedProps.get() ) );
            uno::Sequence< ucb::LockEntry > aSupportedLocks;
            if ( m_xCachedProps->getValue( DAVProperties::SUPPORTEDLOCK )
                 >>= aSupportedLocks )            //get the cached value for supportedlock
            {
                for ( sal_Int32 n = 0; n < aSupportedLocks.getLength(); ++n )
                {
                    if ( aSupportedLocks[ n ].Scope
                         == ucb::LockScope_EXCLUSIVE &&
                         aSupportedLocks[ n ].Type
                         == ucb::LockType_WRITE )
                        eResourceTypeForLocks = DAV;
                }
            }
        }
    }

    const OUString & rURL = m_xIdentifier->getContentIdentifier();

    if ( eResourceTypeForLocks == UNKNOWN )
    {
        // resource type for lock/unlock operations still unknown, need to ask the server
        std::unique_ptr< DAVResourceAccess > xResAccess;

        xResAccess.reset( new DAVResourceAccess(
                              m_xContext,
                              m_rSessionFactory,
                              rURL ) );

        const OUString aScheme(
            rURL.copy( 0, rURL.indexOf( ':' ) ).toAsciiLowerCase() );

        if ( aScheme == FTP_URL_SCHEME )
        {
            eResourceTypeForLocks = FTP;
        }
        else
        {
            try
            {
                // we need only DAV:supportedlock
                std::vector< DAVResource > resources;
                std::vector< OUString > aPropNames;
                uno::Sequence< beans::Property > aProperties( 1 );
                aProperties[ 0 ].Name = DAVProperties::SUPPORTEDLOCK;

                ContentProperties::UCBNamesToDAVNames( aProperties, aPropNames );
                xResAccess->PROPFIND( DAVZERO, aPropNames, resources, Environment );

                // only one resource should be returned
                if ( resources.size() == 1 )
                {
                    // we may have received a bunch of other properties
                    // (some servers seems to do so)
                    // but we need only supported lock for this check
                    // all returned properties are in
                    // resources.properties[n].Name/.Value

                    std::vector< DAVPropertyValue >::iterator it;

                    for ( it = resources[0].properties.begin();
                          it != resources[0].properties.end(); ++it)
                    {
                        if ( (*it).Name ==  DAVProperties::SUPPORTEDLOCK )
                        {
                            uno::Sequence< ucb::LockEntry > aSupportedLocks;
                            if ( (*it).Value >>= aSupportedLocks )
                            {
                                // this is at least a DAV, no lock confirmed yet
                                eResourceTypeForLocks = DAV_NOLOCK;
                                for ( sal_Int32 n = 0; n < aSupportedLocks.getLength(); ++n )
                                {
                                    if ( aSupportedLocks[ n ].Scope == ucb::LockScope_EXCLUSIVE &&
                                         aSupportedLocks[ n ].Type == ucb::LockType_WRITE )
                                    {
                                        // requested locking mode is supported
                                        eResourceTypeForLocks = DAV;
                                        SAL_INFO( "ucb.ucp.webdav", "resourceTypeForLocks - URL: <"
                                                  << m_xIdentifier->getContentIdentifier() << ">, DAV lock/unlock supported");
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
            catch ( DAVException const & e )
            {
                xResAccess->resetUri();
                //grab the error code
                switch( e.getStatus() )
                {
                    case SC_NOT_FOUND:
                        SAL_WARN( "ucb.ucp.webdav", "resourceTypeForLocks() - URL: <"
                                  << m_xIdentifier->getContentIdentifier() << "> was not found. ");
                        eResourceTypeForLocks = NOT_FOUND;
                        break;
                        // some servers returns SC_FORBIDDEN, instead
                        // TODO: probably remove it, when OPTIONS implemented
                        // the meaning of SC_FORBIDDEN is, according to <http://tools.ietf.org/html/rfc7231#section-6.5.3>:
                        // The 403 (Forbidden) status code indicates that the server understood
                        // the request but refuses to authorize it
                    case SC_FORBIDDEN:
                        // Errors SC_NOT_IMPLEMENTED and SC_METHOD_NOT_ALLOWED are
                        // part of base http 1.1 RFCs
                    case SC_NOT_IMPLEMENTED:        // <http://tools.ietf.org/html/rfc7231#section-6.6.2>
                    case SC_METHOD_NOT_ALLOWED:     // <http://tools.ietf.org/html/rfc7231#section-6.5.5>
                        // they all mean the resource is NON_DAV
                        SAL_WARN( "ucb.ucp.webdav", "resourceTypeForLocks() DAVException (SC_FORBIDDEN, SC_NOT_IMPLEMENTED or SC_METHOD_NOT_ALLOWED) - URL: <"
                                  << m_xIdentifier->getContentIdentifier() << ">, DAV error: " << e.getError() << ", HTTP error: " << e.getStatus() );
                        eResourceTypeForLocks = NON_DAV;
                        break;
                    default:
                        //fallthrough
                        SAL_WARN( "ucb.ucp.webdav", "resourceTypeForLocks() DAVException - URL: <"
                                  << m_xIdentifier->getContentIdentifier() << ">, DAV error: " << e.getError() << ", HTTP error: " << e.getStatus() );
                        eResourceTypeForLocks = UNKNOWN;
                }
            }
        }
    }
    osl::MutexGuard g(m_aMutex);
    if (m_eResourceTypeForLocks == UNKNOWN)
    {
        m_eResourceTypeForLocks = eResourceTypeForLocks;
    }
    else
    {
        SAL_WARN_IF(
            eResourceTypeForLocks != m_eResourceTypeForLocks, "ucb.ucp.webdav",
            "different resource types for <" << rURL << ">: "
            << +eResourceTypeForLocks << " vs. " << +m_eResourceTypeForLocks);
    }
    SAL_INFO( "ucb.ucp.webdav", "resourceTypeForLocks() - URL: <"
              << m_xIdentifier->getContentIdentifier() << ">, m_eResourceTypeForLocks: " << m_eResourceTypeForLocks );
    return m_eResourceTypeForLocks;
}


void Content::lock(
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception, std::exception )
{
// prepare aURL to be used in exception, see below
    OUString aURL;
    if ( m_bTransient )
    {
        aURL = getParentURL();
        if ( aURL.lastIndexOf('/') != ( aURL.getLength() - 1 ) )
            aURL += "/";

        aURL += m_aEscapedTitle;
    }
    else
    {
        aURL = m_xIdentifier->getContentIdentifier();
    }

    try
    {
        std::unique_ptr< DAVResourceAccess > xResAccess;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        }

        uno::Any aOwnerAny;
        aOwnerAny
            <<= OUString("LibreOffice - http://www.libreoffice.org/");

        ucb::Lock aLock(
            ucb::LockScope_EXCLUSIVE,
            ucb::LockType_WRITE,
            ucb::LockDepth_ZERO,
            aOwnerAny,
            180, // lock timeout in secs
            //-1, // infinite lock
            uno::Sequence< OUString >() );

        xResAccess->LOCK( aLock, Environment );

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
        }
    }
    catch ( DAVException const & e )
    {
        // check if the exception thrown is 'already locked'
        // this exception is mapped directly to the ucb correct one, without
        // going into the cancelCommandExecution() user interaction
        // this exception should be managed by the issuer of 'lock' command
        switch(e.getError())
        {
            case DAVException::DAV_LOCKED:
            {
                SAL_WARN( "ucb.ucp.webdav", "lock() resource already locked - URL: <"
                          << m_xIdentifier->getContentIdentifier() << ">");
                throw
                    ucb::InteractiveLockingLockedException(
                        OUString( "Locked!" ),
                        static_cast< cppu::OWeakObject * >( this ),
                        task::InteractionClassification_ERROR,
                        aURL,
                        false );
            }
            break;
            case DAVException::DAV_HTTP_AUTH:
            {
                SAL_WARN( "ucb.ucp.webdav", "lock() DAVException Authentication error - URL: <"
                          << m_xIdentifier->getContentIdentifier() << ">" );
                // DAVException::DAV_HTTP_AUTH exception can mean:
                // - interaction handler for credential management not present (happens, depending
                //   on the LO framework processing)
                // - the remote site is a WebDAV with special configuration: read/only for read operations
                //   and read/write for write operations, the user is not allowed to lock/write and
                //   she cancelled the credentials request.
                //   this is not actually an error, but the exception is sent directly from here, avoiding the automatic
                //   management that takes part in cancelCommandExecution() below
                // Unfortunately there is no InteractiveNetwork*Exception available to signal this
                // since it mostly happens on read/only part of webdav, this appears to be the most correct exception available
                throw
                    ucb::InteractiveNetworkWriteException(
                        OUString( "Authentication error while trying to lock! Write only WebDAV perhaps?" ),
                        static_cast< cppu::OWeakObject * >( this ),
                        task::InteractionClassification_ERROR,
                        e.getData() );
            }
            break;
            case DAVException::DAV_HTTP_ERROR:
                //grab the error code
                switch( e.getStatus() )
                {
                    // The 'case SC_PRECONDITION_FAILED' just below tries to solve a problem
                    // in SharePoint when locking the resource on first creation fails due to this:
                    // <https://msdn.microsoft.com/en-us/library/jj575265%28v=office.12%29.aspx#id15>
                    // (retrieved on 2015-08-14)
                    case SC_PRECONDITION_FAILED:    // <http://tools.ietf.org/html/rfc7232#section-4.2>
                        // Errors SC_NOT_IMPLEMENTED and SC_METHOD_NOT_ALLOWED are
                        // part of base http 1.1 RFCs
                    case SC_NOT_IMPLEMENTED:        // <http://tools.ietf.org/html/rfc7231#section-6.6.2>
                    case SC_METHOD_NOT_ALLOWED:     // <http://tools.ietf.org/html/rfc7231#section-6.5.5>
                        SAL_WARN( "ucb.ucp.webdav", "lock() DAVException (SC_PRECONDITION_FAILED, SC_NOT_IMPLEMENTED or SC_METHOD_NOT_ALLOWED) - URL: <"
                                  << m_xIdentifier->getContentIdentifier() << ">, DAV error: " << e.getError() << ", HTTP error: " << e.getStatus() );
                        // act as nothing happened
                        // that's because when a resource is first created
                        // the lock is sent before the put, so the resource
                        // is actually created by LOCK, locking it before
                        // the first PUT, but if LOCK is not supported
                        // (simple web or DAV with lock disabled) we end with one of these http
                        // errors.
                        // These same errors may be reported when the LOCK on an unmapped
                        // (i.e. non existent) resource is not implemented.
                        // Detailed specification in:
                        // <http://tools.ietf.org/html/rfc4918#section-7.3>
                        return;
                        break;
                    default:
                        //fallthrough
                        ;
                }
                break;
            case DAVException::DAV_LOCKED_SELF:
                // we already hold the lock and it is in our internal lockstore
                // just return as if the lock was successful
                return;
                break;
            default:
                //fallthrough
                ;
        }

        SAL_WARN( "ucb.ucp.webdav","lock() DAVException - URL: <"
                  << m_xIdentifier->getContentIdentifier() << ">, DAV error: " << e.getError() << ", HTTP error: " << e.getStatus() );
        cancelCommandExecution( e, Environment );
        // Unreachable
    }
}


void Content::unlock(
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception, std::exception )
{
    try
    {
        std::unique_ptr< DAVResourceAccess > xResAccess;
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
        switch( e.getError() )
        {
            case DAVException::DAV_NOT_LOCKED:
                SAL_WARN( "ucb.ucp.webdav", "unlock() DAVException::DAV_NOT_LOCKED - URL: <"
                          << m_xIdentifier->getContentIdentifier() << ">");
                // means that we don't own any lock on this resource
                // intercepted here to remove a confusing indication to the user
                // unfortunately this happens in some WebDAV server configuration
                // acting as WebDAV and having lock/unlock enabled only
                // for authorized user.
                return;
                break;
            case DAVException::DAV_HTTP_ERROR:
                //grab the error code
                switch( e.getStatus() )
                {
                    // Errors SC_NOT_IMPLEMENTED and SC_METHOD_NOT_ALLOWED are
                    // part of base http 1.1 RFCs
                    case SC_NOT_IMPLEMENTED:        // <http://tools.ietf.org/html/rfc7231#section-6.6.2>
                    case SC_METHOD_NOT_ALLOWED:     // <http://tools.ietf.org/html/rfc7231#section-6.5.5>
                        SAL_WARN( "ucb.ucp.webdav", "unlock() DAVException (SC_NOT_IMPLEMENTED or SC_METHOD_NOT_ALLOWED) - URL: <"
                                  << m_xIdentifier->getContentIdentifier() << ">, DAV error: " << e.getError() << ", HTTP error: " << e.getStatus() );
                        return;
                        break;
                    default:
                        //fallthrough
                        ;
                }
                break;
            default:
                //fallthrough
                ;
        }
        SAL_WARN( "ucb.ucp.webdav","unlock() DAVException - URL: <"
                  << m_xIdentifier->getContentIdentifier() << ">, DAV error: " << e.getError() << ", HTTP error: " << e.getStatus() );
        cancelCommandExecution( e, Environment );
        // Unreachable
    }
}


bool Content::exchangeIdentity(
    const uno::Reference< ucb::XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return false;

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< ucb::XContent > xThis = this;

    // Already persistent?
    if ( m_bTransient )
    {
        SAL_WARN( "ucb.ucp.webdav", "Content::exchangeIdentity - Not persistent!" );
        return false;
    }

    // Exchange own identitity.

    // Fail, if a content with given id already exists.
//  if ( !hasData( xNewId ) )
    {
        OUString aOldURL = m_xIdentifier->getContentIdentifier();

        aGuard.clear();
        if ( exchange( xNewId ) )
        {
            // Process instantiated children...

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
                OUString aOldChildURL
                    = xOldChildId->getContentIdentifier();
                OUString aNewChildURL
                    = aOldChildURL.replaceAt(
                        0,
                        aOldURL.getLength(),
                        xNewId->getContentIdentifier() );
                uno::Reference< ucb::XContentIdentifier > xNewChildId
                    = new ::ucbhelper::ContentIdentifier( aNewChildURL );

                if ( !xChild->exchangeIdentity( xNewChildId ) )
                    return false;

                ++it;
            }
            return true;
        }
    }

    SAL_WARN( "ucb.ucp.webdav", "Content::exchangeIdentity - "
                "Panic! Cannot exchange identity!" );
    return false;
}


bool Content::isFolder(
            const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw( uno::Exception, std::exception )
{
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( m_bTransient )
            return m_bCollection;
    }

    uno::Sequence< beans::Property > aProperties( 1 );
    aProperties[ 0 ].Name   = "IsFolder";
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

    return false;
}


uno::Any Content::MapDAVException( const DAVException & e, bool bWrite )
{
    // Map DAVException...
    uno::Any aException;

    OUString aURL;
    if ( m_bTransient )
    {
        aURL = getParentURL();
        if ( aURL.lastIndexOf('/') != ( aURL.getLength() - 1 ) )
            aURL += "/";

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
                OUString("Uri"), -1,
                uno::makeAny(aURL),
                beans::PropertyState_DIRECT_VALUE);

            aException <<=
                ucb::InteractiveAugmentedIOException(
                    OUString("Not found!"),
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
                OUString(),
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

    case DAVException::DAV_HTTP_TIMEOUT:
    case DAVException::DAV_HTTP_CONNECT:
        aException <<=
            ucb::InteractiveNetworkConnectException(
                OUString(),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                e.getData() );
        break;

// @@@ No matching InteractiveNetwork*Exception
//     case DAVException::DAV_HTTP_REDIRECT:
//         break;

// @@@ No matching InteractiveNetwork*Exception
//     case DAVException::DAV_SESSION_CREATE:
//         break;

    case DAVException::DAV_INVALID_ARG:
        aException <<=
            lang::IllegalArgumentException(
                OUString(),
                static_cast< cppu::OWeakObject * >( this ),
                -1 );
        break;

    case DAVException::DAV_LOCKED:
        aException <<=
            ucb::InteractiveLockingLockedException(
                OUString("Locked!"),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL,
                false ); // not SelfOwned
        break;

    case DAVException::DAV_LOCKED_SELF:
        aException <<=
            ucb::InteractiveLockingLockedException(
                OUString("Locked (self!)"),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL,
                true ); // SelfOwned
        break;

    case DAVException::DAV_NOT_LOCKED:
        aException <<=
            ucb::InteractiveLockingNotLockedException(
                OUString("Not locked!"),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL );
        break;

    case DAVException::DAV_LOCK_EXPIRED:
        aException <<=
            ucb::InteractiveLockingLockExpiredException(
                OUString("Lock expired!"),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL );
        break;

    default:
        aException <<=
            ucb::InteractiveNetworkGeneralException(
                OUString(),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR );
        break;
    }

    return aException;
}


// static
bool Content::shouldAccessNetworkAfterException( const DAVException & e )
{
    if ( ( e.getStatus() == SC_NOT_FOUND ) ||
         ( e.getError() == DAVException::DAV_HTTP_TIMEOUT ) ||
         ( e.getError() == DAVException::DAV_HTTP_LOOKUP ) ||
         ( e.getError() == DAVException::DAV_HTTP_CONNECT ) ||
         ( e.getError() == DAVException::DAV_HTTP_AUTH ) ||
         ( e.getError() == DAVException::DAV_HTTP_AUTHPROXY ) )
        return false;

    return true;
}


void Content::cancelCommandExecution(
                const DAVException & e,
                const uno::Reference< ucb::XCommandEnvironment > & xEnv,
                bool bWrite /* = sal_False */ )
    throw ( uno::Exception, std::exception )
{
    ucbhelper::cancelCommandExecution( MapDAVException( e, bWrite ), xEnv );
    // Unreachable
}


const OUString
Content::getBaseURI( const std::unique_ptr< DAVResourceAccess > & rResAccess )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // First, try to obtain value of response header "Content-Location".
    if ( m_xCachedProps.get() )
    {
        OUString aLocation;
        m_xCachedProps->getValue( "Content-Location" ) >>= aLocation;
        if ( !aLocation.isEmpty() )
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

    return OUString( rResAccess->getURL() );
}

// resource type is the type of the WebDAV resource
Content::ResourceType Content::getResourceType(
                    const uno::Reference< ucb::XCommandEnvironment >& xEnv,
                    const std::unique_ptr< DAVResourceAccess > & rResAccess,
                    bool * networkAccessAllowed)
    throw ( uno::Exception, std::exception )
{
    {
        osl::MutexGuard g(m_aMutex);
        if (m_eResourceType != UNKNOWN) {
            return m_eResourceType;
        }
    }

    ResourceType eResourceType = UNKNOWN;

    const OUString & rURL = rResAccess->getURL();
    const OUString aScheme(
        rURL.copy( 0, rURL.indexOf( ':' ) ).toAsciiLowerCase() );

    if ( aScheme == FTP_URL_SCHEME )
    {
        eResourceType = FTP;
    }
    else
    {
        try
        {
            // Try to fetch some frequently used property value, e.g. those
            // used when loading documents... along with identifying whether
            // this is a DAV resource.
            std::vector< DAVResource > resources;
            std::vector< OUString > aPropNames;
            uno::Sequence< beans::Property > aProperties( 5 );
            aProperties[ 0 ].Name = "IsFolder";
            aProperties[ 1 ].Name = "IsDocument";
            aProperties[ 2 ].Name = "IsReadOnly";
            aProperties[ 3 ].Name = "MediaType";
            aProperties[ 4 ].Name = DAVProperties::SUPPORTEDLOCK;

            ContentProperties::UCBNamesToDAVNames( aProperties, aPropNames );

            rResAccess->PROPFIND( DAVZERO, aPropNames, resources, xEnv );

            if ( resources.size() == 1 )
            {
                osl::MutexGuard g(m_aMutex);
                m_xCachedProps.reset(
                    new CachableContentProperties( ContentProperties( resources[ 0 ] ) ) );
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
            else if (networkAccessAllowed != nullptr)
            {
                *networkAccessAllowed = *networkAccessAllowed
                    && shouldAccessNetworkAfterException(e);
            }
            // if the two net events below happen, something
            // is going on to the connection so break the command flow
            if ( ( e.getError() == DAVException::DAV_HTTP_TIMEOUT ) ||
                 ( e.getError() == DAVException::DAV_HTTP_CONNECT ) )
            {
                cancelCommandExecution( e, xEnv );
                // unreachable
            }
        }
    }

    osl::MutexGuard g(m_aMutex);
    if (m_eResourceType == UNKNOWN) {
        m_eResourceType = eResourceType;
    } else {
        SAL_WARN_IF(
            eResourceType != m_eResourceType, "ucb.ucp.webdav",
            "different resource types for <" << rURL << ">: "
            << +eResourceType << " vs. " << +m_eResourceType);
    }
    SAL_INFO( "ucb.ucp.webdav", "m_eResourceType for <"<<rURL<<">: " << m_eResourceType );
    return m_eResourceType;
}


Content::ResourceType Content::getResourceType(
                    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw ( uno::Exception, std::exception )
{
    std::unique_ptr< DAVResourceAccess > xResAccess;
    {
        osl::MutexGuard aGuard( m_aMutex );
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
    }
    Content::ResourceType const ret = getResourceType( xEnv, xResAccess );
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
    }
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
