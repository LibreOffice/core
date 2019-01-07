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

#include <memory>

#include <cppuhelper/queryinterface.hxx>
#include <osl/diagnose.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/simpleinteractionrequest.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>

#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/NotRemoveableException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/beans/PropertySetInfoChange.hpp>
#include <com/sun/star/beans/PropertySetInfoChangeEvent.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/task/PasswordContainerInteractionHandler.hpp>
#include <com/sun/star/ucb/CommandEnvironment.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/InteractiveLockingLockedException.hpp>
#include <com/sun/star/ucb/InteractiveLockingLockExpiredException.hpp>
#include <com/sun/star/ucb/InteractiveLockingNotLockedException.hpp>
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
#include "SerfUri.hxx"
#include "UCBDeadPropertyValue.hxx"
#include "DAVException.hxx"

using namespace com::sun::star;
using namespace http_dav_ucp;

namespace
{
void lcl_sendPartialGETRequest( bool &bError,
                                DAVException &aLastException,
                                const std::vector< OUString >& rProps,
                                std::vector< OUString > &aHeaderNames,
                                const std::unique_ptr< DAVResourceAccess > &xResAccess,
                                std::unique_ptr< ContentProperties > &xProps,
                                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    DAVResource aResource;
    DAVRequestHeaders aPartialGet;
    aPartialGet.push_back(
        DAVRequestHeader(
            OUString( "Range" ),
            OUString( "bytes=0-0" )));

    bool bIsRequestSize = std::any_of(aHeaderNames.begin(), aHeaderNames.end(),
        [](const OUString& rHeaderName) { return rHeaderName == "Content-Length"; });

    if ( bIsRequestSize )
    {
        // we need to know if the server accepts range requests for a resource
        // and the range unit it uses
        aHeaderNames.push_back( OUString( "Accept-Ranges" ) );
        aHeaderNames.push_back( OUString( "Content-Range" ) );
    }
    try
    {
        uno::Reference< io::XInputStream > xIn = xResAccess->GET( aPartialGet,
                                                                  aHeaderNames,
                                                                  aResource,
                                                                  xEnv );
        bError = false;

        if ( bIsRequestSize )
        {
            // the ContentProperties maps "Content-Length" to the UCB "Size" property
            // This would have an unrealistic value of 1 byte because we did only a partial GET
            // Solution: if "Content-Range" is present, map it with UCB "Size" property
            OUString aAcceptRanges, aContentRange, aContentLength;
            std::vector< DAVPropertyValue > &aResponseProps = aResource.properties;
            for ( const auto& rResponseProp : aResponseProps )
            {
                if ( rResponseProp.Name == "Accept-Ranges" )
                    rResponseProp.Value >>= aAcceptRanges;
                else if ( rResponseProp.Name == "Content-Range" )
                    rResponseProp.Value >>= aContentRange;
                else if ( rResponseProp.Name == "Content-Length" )
                    rResponseProp.Value >>= aContentLength;
            }

            sal_Int64 nSize = 1;
            if ( aContentLength.getLength() )
            {
                nSize = aContentLength.toInt64();
            }

            // according to http://tools.ietf.org/html/rfc2616#section-3.12
            // the only range unit defined is "bytes" and implementations
            // MAY ignore ranges specified using other units.
            if ( nSize == 1 &&
                    aContentRange.getLength() &&
                    aAcceptRanges == "bytes" )
            {
                // Parse the Content-Range to get the size
                // vid. http://tools.ietf.org/html/rfc2616#section-14.16
                // Content-Range: <range unit> <bytes range>/<size>
                sal_Int32 nSlash = aContentRange.lastIndexOf( '/' );
                if ( nSlash != -1 )
                {
                    OUString aSize = aContentRange.copy( nSlash + 1 );
                    // "*" means that the instance-length is unknown at the time when the response was generated
                    if ( aSize != "*" )
                    {
                        auto it = std::find_if(aResponseProps.begin(), aResponseProps.end(),
                            [](const DAVPropertyValue& rProp) { return rProp.Name == "Content-Length"; });
                        if (it != aResponseProps.end())
                        {
                            it->Value <<= aSize;
                        }
                    }
                }
            }
        }

        if ( xProps.get() )
            xProps->addProperties(
                rProps,
                ContentProperties( aResource ) );
        else
            xProps.reset ( new ContentProperties( aResource ) );
    }
    catch ( DAVException const & ex )
    {
        aLastException = ex;
    }
}
}


// Content Implementation.


// ctr for content on an existing webdav resource
Content::Content(
          const uno::Reference< uno::XComponentContext >& rxContext,
          ContentProvider* pProvider,
          const uno::Reference< ucb::XContentIdentifier >& Identifier,
          rtl::Reference< DAVSessionFactory > const & rSessionFactory )
: ContentImplHelper( rxContext, pProvider, Identifier ),
  m_eResourceType( UNKNOWN ),
  m_pProvider( pProvider ),
  m_bTransient( false ),
  m_bLocked( false ),
  m_bCollection( false ),
  m_bDidGetOrHead( false )
{
    try
    {
        m_xResAccess.reset( new DAVResourceAccess(
                rxContext,
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


// ctr for content on an non-existing webdav resource
Content::Content(
            const uno::Reference< uno::XComponentContext >& rxContext,
            ContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            rtl::Reference< DAVSessionFactory > const & rSessionFactory,
            bool isCollection )
: ContentImplHelper( rxContext, pProvider, Identifier ),
  m_eResourceType( UNKNOWN ),
  m_pProvider( pProvider ),
  m_bTransient( true ),
  m_bLocked( false ),
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
    if (m_bLocked)
        unlock(uno::Reference< ucb::XCommandEnvironment >());
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
                m_xContext, uno::UNO_QUERY_THROW );
            uno::Reference< uno::XComponentContext > xCtx;
            xCtx.set( xProps->getPropertyValue( "DefaultContext" ),
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


// XTypeProvider methods.


XTYPEPROVIDER_COMMON_IMPL( Content );


// virtual
uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
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
    {
        static cppu::OTypeCollection s_aFolderTypes(
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
                        CPPU_TYPE_REF( ucb::XContentCreator ) );

        return s_aFolderTypes.getTypes();
    }
    else
    {
        static cppu::OTypeCollection s_aDocumentTypes(
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

        return  s_aDocumentTypes.getTypes();
    }
}


// XServiceInfo methods.


// virtual
OUString SAL_CALL Content::getImplementationName()
{
    return OUString( "com.sun.star.comp.ucb.WebDAVContent" );
}


// virtual
uno::Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
{
    uno::Sequence<OUString> aSNS { WEBDAV_CONTENT_SERVICE_NAME };
    return aSNS;
}


// XContent methods.


// virtual
OUString SAL_CALL Content::getContentType()
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
{
    SAL_INFO("ucb.ucp.webdav",  ">>>>> Content::execute: start: command: " << aCommand.Name
            << ", env: " << (Environment.is() ? "present" : "missing") );

    uno::Any aRet;

    if ( aCommand.Name == "getPropertyValues" )
    {

        // getPropertyValues


        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    "Wrong argument type!",
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
                                    "Wrong argument type!",
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.getLength() )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    "No properties!",
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


        ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    "Wrong argument type!",
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet = open( aOpenCommand, Environment );

        if ( (aOpenCommand.Mode == ucb::OpenMode::DOCUMENT ||
              aOpenCommand.Mode == ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE) &&
                supportsExclusiveWriteLock( Environment ) )
            lock( Environment );
    }
    else if ( aCommand.Name == "insert" )
    {

        // insert


        ucb::InsertCommandArgument arg;
        if ( !( aCommand.Argument >>= arg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    "Wrong argument type!",
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
                m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
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
        removeAdditionalPropertySet();
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
                                  "Wrong argument type!",
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
                                    "Wrong argument type!",
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        post( aArg, Environment );
    }
    else if ( aCommand.Name == "lock" &&
              supportsExclusiveWriteLock( Environment ) )
    {

        // lock


        lock( Environment );
    }
    else if ( aCommand.Name == "unlock" &&
              supportsExclusiveWriteLock( Environment ) )
    {

        // unlock


        unlock( Environment );
    }
    else if ( aCommand.Name == "createNewContent" &&
              isFolder( Environment ) )
    {

        // createNewContent


        ucb::ContentInfo aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    "Wrong argument type!",
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        aRet <<= createNewContent( aArg );
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

    SAL_INFO("ucb.ucp.webdav",  "<<<<< Content::execute: end: command: " << aCommand.Name);
    return aRet;
}


// virtual
void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
{
    try
    {
        std::unique_ptr< DAVResourceAccess > xResAccess;
        {
            osl::MutexGuard aGuard( m_aMutex );
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        }
        DAVResourceAccess::abort();
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
        }
    }
    catch ( DAVException const & )
    {
        // abort failed!
    }
}


// XPropertyContainer methods.


void Content::addProperty( const css::ucb::PropertyCommandArgument &aCmdArg,
                           const uno::Reference< ucb::XCommandEnvironment >& xEnv  )
{
//    if ( m_bTransient )
//   @@@ ???
    const beans::Property aProperty = aCmdArg.Property;
    const uno::Any aDefaultValue = aCmdArg.DefaultValue;

    // check property Name
    if ( !aProperty.Name.getLength() )
        throw lang::IllegalArgumentException(
            "\"addProperty\" with empty Property.Name",
            static_cast< ::cppu::OWeakObject * >( this ),
            -1 );

    // Check property type.
    if ( !UCBDeadPropertyValue::supportsType( aProperty.Type ) )
        throw beans::IllegalTypeException(
            "\"addProperty\" unsupported Property.Type",
            static_cast< ::cppu::OWeakObject * >( this ) );

    // check default value
    if ( aDefaultValue.hasValue() && aDefaultValue.getValueType() != aProperty.Type )
        throw beans::IllegalTypeException(
            "\"addProperty\" DefaultValue does not match Property.Type",
            static_cast< ::cppu::OWeakObject * >( this ) );


    // Make sure a property with the requested name does not already
    // exist in dynamic and static(!) properties.


    // Take into account special properties with custom namespace
    // using <prop:the_propname xmlns:prop="the_namespace">
    OUString aSpecialName;
    bool bIsSpecial = DAVProperties::isUCBSpecialProperty( aProperty.Name, aSpecialName );

    // Note: This requires network access!
    if ( getPropertySetInfo( xEnv, false /* don't cache data */ )
            ->hasPropertyByName( bIsSpecial ? aSpecialName : aProperty.Name ) )
    {
        // Property does already exist.
        throw beans::PropertyExistException();
    }


    // Add a new dynamic property.


    ProppatchValue aValue( PROPSET, aProperty.Name, aDefaultValue );

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
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
        }

        // Notify propertyset info change listeners.
        beans::PropertySetInfoChangeEvent evt(
            static_cast< cppu::OWeakObject * >( this ),
            bIsSpecial ? aSpecialName : aProperty.Name,
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
            ContentImplHelper::addProperty( bIsSpecial ? aSpecialName : aProperty.Name,
                                            aProperty.Attributes,
                                            aDefaultValue );
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
                        ContentImplHelper::addProperty( bIsSpecial ? aSpecialName : aProperty.Name,
                                                        aProperty.Attributes,
                                                        aDefaultValue );
                        break;

                    default:
                        SAL_WARN( "ucb.ucp.webdav",
                                    "Content::addProperty - "
                                    "Unsupported resource type!" );
                        break;
                    }
                }
                catch ( uno::Exception const & )
                {
                    SAL_WARN( "ucb.ucp.webdav",
                                "Content::addProperty - "
                                "Unable to determine resource type!" );
                }
            }
            else
            {
                SAL_WARN( "ucb.ucp.webdav",
                            "Content::addProperty - "
                            "Unable to determine resource type!" );
            }
        }
    }
}

void Content::removeProperty( const OUString& Name,
                              const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
#if 0
    // @@@ REMOVABLE at the moment not properly set in the PropSetInfo
    try
    {
        beans::Property aProp
        = getPropertySetInfo( xEnv, false /* don't cache data */ )
          ->getPropertyByName( Name );

        if ( !( aProp.Attributes & beans::PropertyAttribute::REMOVABLE ) )
        {
            // Not removable!
            throw beans::NotRemoveableException();
        }
    }
    catch ( beans::UnknownPropertyException const & )
    {
        //SAL_WARN( "ucb.ucp.webdav", "removeProperty - Unknown property!" );
        throw;
    }
#endif

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
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
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
                        SAL_WARN( "ucb.ucp.webdav",
                                    "Content::removeProperty - "
                                    "Unsupported resource type!" );
                        break;
                    }
                }
                catch ( uno::Exception const & )
                {
                    SAL_WARN( "ucb.ucp.webdav",
                                "Content::removeProperty - "
                                "Unable to determine resource type!" );
                }
            }
            else
            {
                SAL_WARN( "ucb.ucp.webdav",
                            "Content::removeProperty - "
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
{
    removeProperty( Name,
                    uno::Reference< ucb::XCommandEnvironment >() );
}


// XContentCreator methods.


// virtual
uno::Sequence< ucb::ContentInfo > SAL_CALL
Content::queryCreatableContentsInfo()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< ucb::ContentInfo > aSeq( 2 );

    // document.
    aSeq.getArray()[ 0 ].Type = WEBDAV_CONTENT_TYPE;
    aSeq.getArray()[ 0 ].Attributes
        = ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
          | ucb::ContentInfoAttribute::KIND_DOCUMENT;

    beans::Property aProp;
    m_pProvider->getProperty( "Title", aProp );

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
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !Info.Type.getLength() )
        return uno::Reference< ucb::XContent >();

    if ( ( Info.Type != WEBDAV_COLLECTION_TYPE )
         &&
         ( Info.Type != WEBDAV_CONTENT_TYPE ) )
        return uno::Reference< ucb::XContent >();

    OUString aURL = m_xIdentifier->getContentIdentifier();

    SAL_WARN_IF( aURL.isEmpty(), "ucb.ucp.webdav",
                "WebdavContent::createNewContent - empty identifier!" );

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
        return new ::http_dav_ucp::Content( m_xContext,
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

    return aURL.copy( 0, nPos + 1 );
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

        ContentProvider * pProvider
            = static_cast< ContentProvider * >( rProvider.get() );
        beans::Property aProp;

        for ( const auto& rProp : *xProps )
        {
            if ( pProvider->getProperty( rProp.first, aProp ) )
                xRow->appendObject( aProp, rProp.second.value() );
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
{
    std::unique_ptr< ContentProperties > xProps;
    std::unique_ptr< ContentProperties > xCachedProps;
    std::unique_ptr< DAVResourceAccess > xResAccess;
    OUString aUnescapedTitle;
    bool bHasAll = false;
    uno::Reference< uno::XComponentContext > xContext;
    uno::Reference< ucb::XContentIdentifier > xIdentifier;
    rtl::Reference< ::ucbhelper::ContentProviderImplHelper > xProvider;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        aUnescapedTitle = SerfUri::unescape( m_aEscapedTitle );
        xContext.set( m_xContext );
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
            xProps.reset( new ContentProperties( *xCachedProps ) );
        }
    }

    if ( !m_bTransient && !bHasAll )
    {
        // Obtain values from server...


        // First, identify whether resource is DAV or not
        bool bNetworkAccessAllowed = true;
        const ResourceType & rType = getResourceType( xEnv, xResAccess, &bNetworkAccessAllowed );

        if ( DAV == rType )
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
                xProps.reset( new ContentProperties( *xCachedProps ) );
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

                        if ( std::none_of(m_aFailedPropNames.begin(), m_aFailedPropNames.end(),
                                [&rName](const OUString& rPropName) { return rPropName == rName; }) )
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
                        bNetworkAccessAllowed = bNetworkAccessAllowed &&
                            shouldAccessNetworkAfterException( e );

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
            std::vector< OUString > aMissingProps;
            if ( !( xProps.get()
                    && xProps->containsAllNames(
                        rProperties, aMissingProps ) )
                 || !m_bDidGetOrHead )
            {
                // Possibly the missing props can be obtained using a HEAD
                // request.

                std::vector< OUString > aHeaderNames;
                ContentProperties::UCBNamesToHTTPNames(
                    rProperties,
                    aHeaderNames,
                    true /* bIncludeUnmatched */ );

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
                        // non "general-purpose servers" may not support HEAD requests
                        // see http://www.w3.org/Protocols/rfc2616/rfc2616-sec5.html#sec5.1.1
                        // In this case, perform a partial GET only to get the header info
                        // vid. http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.35
                        // WARNING if the server does not support partial GETs,
                        // the GET will transfer the whole content
                        bool bError = true;
                        DAVException aLastException = e;

                        // According to the spec. the origin server SHOULD return
                        // * 405 (Method Not Allowed):
                        //      the method is known but not allowed for the requested resource
                        // * 501 (Not Implemented):
                        //      the method is unrecognized or not implemented
                        // TODO SC_NOT_FOUND is only for google-code server
                        if ( aLastException.getStatus() == SC_NOT_IMPLEMENTED ||
                                aLastException.getStatus() == SC_METHOD_NOT_ALLOWED ||
                                aLastException.getStatus() == SC_NOT_FOUND )
                        {
                            lcl_sendPartialGETRequest( bError,
                                                       aLastException,
                                                       aMissingProps,
                                                       aHeaderNames,
                                                       xResAccess,
                                                       xProps,
                                                       xEnv );
                            m_bDidGetOrHead = !bError;
                        }

                        if ( bError )
                        {
                            if ( !shouldAccessNetworkAfterException( aLastException ) )
                            {
                                cancelCommandExecution( aLastException, xEnv );
                                // unreachable
                            }
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
            xProps->addProperty(
                "ContentType",
                uno::makeAny( OUString(WEBDAV_CONTENT_TYPE) ),
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
            xProps->getValue( "IsFolder" )
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
        = getPropertyValues( xContext,
                             rProperties,
                             *xProps,
                             xProvider,
                             xIdentifier->getContentIdentifier() );

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !m_xCachedProps.get() )
            m_xCachedProps.reset( new CachableContentProperties( *xProps ) );
        else
            m_xCachedProps->addProperties( *xProps );

        m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
        m_aEscapedTitle = SerfUri::escapeSegment( aUnescapedTitle );
    }

    return xResultRow;
}


uno::Sequence< uno::Any > Content::setPropertyValues(
                const uno::Sequence< beans::PropertyValue >& rValues,
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
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
    aEvent.Further        = false;
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
                            "Property is read-only!",
                            static_cast< cppu::OWeakObject * >( this ) );
            continue;
        }


        // Mandatory props.


        if ( rName == "ContentType" )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                "Property is read-only!",
                static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName == "IsDocument" )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                "Property is read-only!",
                static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName == "IsFolder" )
        {
            // Read-only property!
            aRet[ n ] <<= lang::IllegalAccessException(
                            "Property is read-only!",
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName == "Title" )
        {
            OUString aNewValue;
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
                            "Invalid content identifier!",
                            static_cast< cppu::OWeakObject * >( this ),
                            -1 );
                    }
                }
                else
                {
                    aRet[ n ] <<= lang::IllegalArgumentException(
                        "Empty title not allowed!",
                        static_cast< cppu::OWeakObject * >( this ),
                        -1 );
                }
            }
            else
            {
                aRet[ n ] <<= beans::IllegalTypeException(
                    "Property value has wrong type!",
                    static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        else
        {

            // Optional props.


            OUString aSpecialName;
            bool bIsSpecial = DAVProperties::isUCBSpecialProperty( rName, aSpecialName );

            if ( !xInfo.is() )
                xInfo = getPropertySetInfo( xEnv,
                                            false /* don't cache data */ );

            if ( !xInfo->hasPropertyByName( bIsSpecial ? aSpecialName : rName ) )
            {
                // Check, whether property exists. Skip otherwise.
                // PROPPATCH::set would add the property automatically, which
                // is not allowed for "setPropertyValues" command!
                aRet[ n ] <<= beans::UnknownPropertyException(
                                "Property is unknown!",
                                static_cast< cppu::OWeakObject * >( this ) );
                continue;
            }

            if ( rName == "Size" )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                "Property is read-only!",
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName == "DateCreated" )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                "Property is read-only!",
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName == "DateModified" )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                "Property is read-only!",
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName == "MediaType" )
            {
                // Read-only property!
                // (but could be writable, if 'getcontenttype' would be)
                aRet[ n ] <<= lang::IllegalAccessException(
                                "Property is read-only!",
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            if ( rName == "CreatableContentsInfo" )
            {
                // Read-only property!
                aRet[ n ] <<= lang::IllegalAccessException(
                                "Property is read-only!",
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
                                "No property set for storing the value!",
                                static_cast< cppu::OWeakObject * >( this ) );
                    }
                }
            }
        }
    } // for

    if ( !bTransient && (!aProppatchValues.empty()) )
    {
        try
        {
            // Set property values at server.
            xResAccess->PROPPATCH( aProppatchValues, xEnv );

            for ( const auto& rProppatchValue : aProppatchValues )
            {
                aEvent.PropertyName = rProppatchValue.name;
                aEvent.OldValue     = uno::Any(); // @@@ to expensive to obtain!
                aEvent.NewValue     = rProppatchValue.value;

                aChanges.getArray()[ nChanged ] = aEvent;
                nChanged++;
            }
        }
        catch ( DAVException const & e )
        {
//            SAL_WARN( "ucb.ucp.webdav",
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
                aRet[ (*it) ] <<= MapDAVException( e, true );
                ++it;
            }
#endif
        }
    }

    if ( bExchange )
    {
        // Assemble new content identifier...

        OUString aNewURL = getParentURL();
        if ( aNewURL.lastIndexOf( '/' ) != ( aNewURL.getLength() - 1 ) )
            aNewURL += "/";

        aNewURL += SerfUri::escapeSegment( aNewTitle );

        uno::Reference< ucb::XContentIdentifier > xNewId
            = new ::ucbhelper::ContentIdentifier( aNewURL );
        uno::Reference< ucb::XContentIdentifier > xOldId = xIdentifier;

        try
        {
            SerfUri sourceURI( xOldId->getContentIdentifier() );
            SerfUri targetURI( xNewId->getContentIdentifier() );
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
//                                           true );
            }
            else
            {
                // Do not set new title!
                aNewTitle.clear();

                // Set error .
                aRet[ nTitlePos ] <<= uno::Exception(
                    "Exchange failed!",
                    static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        catch ( DAVException const & e )
        {
            // Do not set new title!
            aNewTitle.clear();

            // Set error .
            aRet[ nTitlePos ] = MapDAVException( e, true );
        }
    }

    if ( aNewTitle.getLength() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        aEvent.PropertyName = "Title";
        aEvent.OldValue     <<= aOldTitle;
        aEvent.NewValue     <<= aNewTitle;

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
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
    }

    return aRet;
}


uno::Any Content::open(
                const ucb::OpenCommandArgument2 & rArg,
                const uno::Reference< ucb::XCommandEnvironment > & xEnv )
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
            aMsg.append( "Non-folder resource cannot be opened as folder! Wrong Open Mode!" );

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
                        new DAVResourceAccess( *xResAccess ) );
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

                    // fill inputstream sync; return if all data present
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
                            new DAVResourceAccess( *xResAccess ) );
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
                     new DAVResourceAccess( *xResAccess ) );
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
                        new DAVResourceAccess( *xResAccess ) );
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

    for ( const auto& rChild : aAllContents )
    {
        ::ucbhelper::ContentImplHelperRef xChild = rChild;
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
                    ::http_dav_ucp::Content::ContentRef(
                        static_cast< ::http_dav_ucp::Content * >(
                            xChild.get() ) ) );
            }
        }
    }
}


void Content::insert(
        const uno::Reference< io::XInputStream > & xInputStream,
        bool bReplaceExisting,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
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
#undef ERROR
            ucb::UnsupportedNameClashException aEx(
                "Unable to write without overwrite!",
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
                        ContinuationFlags::Approve
                            | ContinuationFlags::Disapprove );
                xIH->handle( xRequest.get() );

                const ContinuationFlags nResp = xRequest->getResponse();

                switch ( nResp )
                {
                    case ContinuationFlags::NONE:
                        // Not handled; throw.
                        throw aEx;
//                            break;

                    case ContinuationFlags::Approve:
                        // Continue -> Overwrite.
                        bReplaceExisting = true;
                        break;

                    case ContinuationFlags::Disapprove:
                        // Abort.
                        throw ucb::CommandFailedException(
                                    OUString(),
                                    uno::Reference< uno::XInterface >(),
                                    aExAsAny );
//                            break;

                    default:
                        SAL_WARN( "ucb.ucp.webdav",
                                    "Content::insert - "
                                    "Unknown interaction selection!" );
                        throw ucb::CommandFailedException(
                                    "Unknown interaction selection!",
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
                            cancelCommandExecution( e, Environment, true );
                            // Unreachable
                        }

                        // Insert (recursion!).
                        insert( xInputStream, bReplaceExisting, Environment );

                        {
                            osl::Guard< osl::Mutex > aGuard( m_aMutex );
                            m_xResAccess.reset(
                                new DAVResourceAccess( *xResAccess ) );
                        }

                        // Success!
                        return;
                    }
                    else
                    {
                        OUString aTitle;
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
            m_xIdentifier
                = new ::ucbhelper::ContentIdentifier( aURL );
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
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
    }
}


void Content::transfer(
        const ucb::TransferInfo & rArgs,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
{
    uno::Reference< uno::XComponentContext > xContext;
    uno::Reference< ucb::XContentIdentifier >    xIdentifier;
    uno::Reference< ucb::XContentProvider >      xProvider;
    std::unique_ptr< DAVResourceAccess > xResAccess;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        xContext.set( m_xContext );
        xIdentifier.set( m_xIdentifier );
        xProvider.set( m_xProvider.get() );
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
    }

    OUString aTargetURI;
    try
    {
        SerfUri sourceURI( rArgs.SourceURL );
        SerfUri targetURI( xIdentifier->getContentIdentifier() );
        aTargetURI = targetURI.GetPathBaseNameUnescaped();

        // Check source's and target's URL scheme

        OUString aScheme = sourceURI.GetScheme().toAsciiLowerCase();
        if ( aScheme == VNDSUNSTARWEBDAV_URL_SCHEME)
        {
            sourceURI.SetScheme( HTTP_URL_SCHEME );
        }
        else if ( aScheme == VNDSUNSTARWEBDAVS_URL_SCHEME)
        {
            sourceURI.SetScheme( HTTPS_URL_SCHEME );
        }
        else if ( aScheme == DAV_URL_SCHEME )
        {
            sourceURI.SetScheme( HTTP_URL_SCHEME );
        }
        else if ( aScheme == DAVS_URL_SCHEME )
        {
            sourceURI.SetScheme( HTTPS_URL_SCHEME );
        }
        else if (aScheme == WEBDAV_URL_SCHEME)
        {
            sourceURI.SetScheme(HTTP_URL_SCHEME);
        }
        else if (aScheme == WEBDAVS_URL_SCHEME)
        {
            sourceURI.SetScheme(HTTPS_URL_SCHEME);
        }
        else
        {
            if ( aScheme != HTTP_URL_SCHEME && aScheme != HTTPS_URL_SCHEME )
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        ucb::InteractiveBadTransferURLException(
                            "Unsupported URL scheme!",
                            static_cast< cppu::OWeakObject * >( this ) ) ),
                    Environment );
                // Unreachable
            }
        }

        aScheme = targetURI.GetScheme().toAsciiLowerCase();
        if ( aScheme == VNDSUNSTARWEBDAV_URL_SCHEME)
            targetURI.SetScheme( HTTP_URL_SCHEME );
        else if ( aScheme == VNDSUNSTARWEBDAVS_URL_SCHEME)
            targetURI.SetScheme( HTTPS_URL_SCHEME );
        else if ( aScheme == DAV_URL_SCHEME )
            targetURI.SetScheme( HTTP_URL_SCHEME );
        else if ( aScheme == DAVS_URL_SCHEME )
            targetURI.SetScheme( HTTPS_URL_SCHEME );
        else if (aScheme == WEBDAV_URL_SCHEME)
            targetURI.SetScheme(HTTP_URL_SCHEME);
        else if (aScheme == WEBDAVS_URL_SCHEME)
            targetURI.SetScheme(HTTPS_URL_SCHEME);

        // @@@ This implementation of 'transfer' only works
        //     if the source and target are located at same host.
        //     (Neon does not support cross-server copy/move)

        // Check for same host

        if ( sourceURI.GetHost().getLength() &&
             ( sourceURI.GetHost() != targetURI.GetHost() ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( ucb::InteractiveBadTransferURLException(
                                "Different hosts!",
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

        DAVResourceAccess aSourceAccess( xContext,
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
//                                           true );
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
//                                         true );
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
                case 0/*ucb::NameClash::ERROR*/:
                {
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny(
                            ucb::NameClashException(
                                OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                task::InteractionClassification_ERROR,
                                aTargetURI ) ),
                        Environment );
                    // Unreachable
                }
                [[fallthrough]];

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
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
    }
}


void Content::destroy( bool bDeletePhysical )
{
    // @@@ take care about bDeletePhysical -> trashcan support

    uno::Reference< ucb::XContent > xThis = this;

    deleted();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // Process instantiated children...

    ::http_dav_ucp::Content::ContentRefList aChildren;
    queryChildren( aChildren );

    for ( auto& rChild : aChildren )
    {
        rChild->destroy( bDeletePhysical );
    }
}


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


void Content::lock(
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
{
    try
    {
        std::unique_ptr< DAVResourceAccess > xResAccess;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        }

        uno::Any aOwnerAny;
        aOwnerAny <<= OUString( "http://ucb.openoffice.org" );

        ucb::Lock aLock(
            ucb::LockScope_EXCLUSIVE,
            ucb::LockType_WRITE,
            ucb::LockDepth_ZERO,
            aOwnerAny,
            180, // lock timeout in secs
            //-1, // infinite lock
            uno::Sequence< OUString >() );

        xResAccess->LOCK( aLock, Environment );
        m_bLocked = true;

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
        }
    }
    catch ( DAVException const & e )
    {
        cancelCommandExecution( e, Environment, false );
        // Unreachable
    }
}


void Content::unlock(
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
{
    try
    {
        std::unique_ptr< DAVResourceAccess > xResAccess;
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
        }

        xResAccess->UNLOCK( Environment );
        m_bLocked = false;

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
        }
    }
    catch ( DAVException const & e )
    {
        cancelCommandExecution( e, Environment, false );
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

            for ( const auto& rChild : aChildren )
            {
                ContentRef xChild = rChild;

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
            }
            return true;
        }
    }

    SAL_WARN( "ucb.ucp.webdav",
                "Content::exchangeIdentity - "
                "Panic! Cannot exchange identity!" );
    return false;
}


bool Content::isFolder(
            const uno::Reference< ucb::XCommandEnvironment >& xEnv )
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
        if ( aURL.lastIndexOf( '/' ) != ( aURL.getLength() - 1 ) )
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
                "Uri", -1,
                uno::makeAny(aURL),
                beans::PropertyState_DIRECT_VALUE);

            aException <<=
                ucb::InteractiveAugmentedIOException(
                    "Not found!",
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

    case DAVException::DAV_HTTP_CONNECT:
        aException <<=
            ucb::InteractiveNetworkConnectException(
                OUString(),
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
                OUString(),
                static_cast< cppu::OWeakObject * >( this ),
                -1 );
        break;

    case DAVException::DAV_LOCKED:
#if 1
        aException <<=
            ucb::InteractiveLockingLockedException(
                "Locked!",
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL,
                false ); // not SelfOwned
#else
        {
            uno::Sequence< uno::Any > aArgs( 1 );
            aArgs[ 0 ] <<= beans::PropertyValue(
                OUString("Uri"), -1,
                uno::makeAny(aURL),
                beans::PropertyState_DIRECT_VALUE);

            aException <<=
                ucb::InteractiveAugmentedIOException(
                    OUString( "Locked!" ),
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
                "Locked (self)!",
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL,
                true ); // SelfOwned
        break;

    case DAVException::DAV_NOT_LOCKED:
        aException <<=
            ucb::InteractiveLockingNotLockedException(
                "Not locked!",
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL );
        break;

    case DAVException::DAV_LOCK_EXPIRED:
        aException <<=
            ucb::InteractiveLockingLockExpiredException(
                "Lock expired!",
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
                bool bWrite /* = false */ )
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

    return rResAccess->getURL();
}


Content::ResourceType Content::getResourceType(
                    const uno::Reference< ucb::XCommandEnvironment >& xEnv,
                    const std::unique_ptr< DAVResourceAccess > & rResAccess,
                    bool * networkAccessAllowed )
{
    {
        osl::MutexGuard g(m_aMutex);
        if (m_eResourceType != UNKNOWN) {
            return m_eResourceType;
        }
    }

    ResourceType eResourceType = UNKNOWN;

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

        ContentProperties::UCBNamesToDAVNames(
            aProperties, aPropNames );

        rResAccess->PROPFIND(
            DAVZERO, aPropNames, resources, xEnv );

        // TODO - is this really only one?
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

        // cancel command execution is case that no user authentication data has been provided.
        if ( e.getError() == DAVException::DAV_HTTP_NOAUTH )
        {
            cancelCommandExecution( e, uno::Reference< ucb::XCommandEnvironment >() );
        }
    }

    osl::MutexGuard g(m_aMutex);
    if (m_eResourceType == UNKNOWN) {
        m_eResourceType = eResourceType;
    } else {
        SAL_WARN_IF(
            eResourceType != m_eResourceType, "ucb.ucp.webdav",
            "different resource types for <" << rResAccess->getURL() << ">: "
            << +eResourceType << " vs. " << +m_eResourceType);
    }
    return m_eResourceType;
}


Content::ResourceType Content::getResourceType(
                    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    std::unique_ptr< DAVResourceAccess > xResAccess;
    {
        osl::MutexGuard aGuard( m_aMutex );
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
    }
    const Content::ResourceType & ret = getResourceType( xEnv, xResAccess );
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
    }
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
