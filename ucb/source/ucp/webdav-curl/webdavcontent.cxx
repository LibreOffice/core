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
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Inet.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/simpleinteractionrequest.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <svl/lockfilecommon.hxx>

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
#include "CurlUri.hxx"
#include "UCBDeadPropertyValue.hxx"
#include "DAVException.hxx"
#include "DAVProperties.hxx"

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
    aPartialGet.emplace_back(
            u"Range"_ustr, // see <https://tools.ietf.org/html/rfc7233#section-3.1>
            u"bytes=0-0"_ustr);

    bool bIsRequestSize = std::any_of(aHeaderNames.begin(), aHeaderNames.end(),
        [](const OUString& rHeaderName) { return rHeaderName == "Content-Length"; });

    if ( bIsRequestSize )
    {
        // we need to know if the server accepts range requests for a resource
        // and the range unit it uses
        aHeaderNames.emplace_back("Accept-Ranges"); // see <https://tools.ietf.org/html/rfc7233#section-2.3>
        aHeaderNames.emplace_back("Content-Range"); // see <https://tools.ietf.org/html/rfc7233#section-4.2>
    }
    try
    {
        xResAccess->GET0( aPartialGet, aHeaderNames, aResource, xEnv );
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

        if (xProps)
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

// Static value, to manage a simple OPTIONS cache
// Key is the URL, element is the DAVOptions resulting from an OPTIONS call.
// Cached DAVOptions have a lifetime that depends on the errors received or not received
// and on the value of received options.
static DAVOptionsCache aStaticDAVOptionsCache;


// Content Implementation.


// ctr for content on an existing webdav resource
Content::Content(
          const uno::Reference< uno::XComponentContext >& rxContext,
          ContentProvider* pProvider,
          const uno::Reference< ucb::XContentIdentifier >& Identifier,
          rtl::Reference< DAVSessionFactory > const & rSessionFactory )
: ContentImplHelper( rxContext, pProvider, Identifier ),
  m_eResourceType( UNKNOWN ),
  m_eResourceTypeForLocks( UNKNOWN ),
  m_pProvider( pProvider ),
  m_bTransient( false ),
  m_bCollection( false ),
  m_bDidGetOrHead( false )
{
    try
    {
        initOptsCacheLifeTime();
        m_xResAccess.reset( new DAVResourceAccess(
                rxContext,
                rSessionFactory,
                Identifier->getContentIdentifier() ) );

        CurlUri const aURI( Identifier->getContentIdentifier() );
        m_aEscapedTitle = aURI.GetPathBaseName();
    }
    catch ( DAVException const & )
    {
        throw ucb::ContentCreationException();
    }
}


// ctr for content on a non-existing webdav resource
Content::Content(
            const uno::Reference< uno::XComponentContext >& rxContext,
            ContentProvider* pProvider,
            const uno::Reference< ucb::XContentIdentifier >& Identifier,
            rtl::Reference< DAVSessionFactory > const & rSessionFactory,
            bool isCollection )
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
        initOptsCacheLifeTime();
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
void SAL_CALL Content::acquire() noexcept
{
    ContentImplHelper::acquire();
}


// virtual
void SAL_CALL Content::release() noexcept
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
            uno::Reference< task::XInteractionHandler > xIH(
                task::PasswordContainerInteractionHandler::create(m_xContext) );

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
    return u"com.sun.star.comp.ucb.WebDAVContent"_ustr;
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
        return WEBDAV_COLLECTION_TYPE;

    return WEBDAV_CONTENT_TYPE;
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
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
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
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
                                    -1 ) ),
                Environment );
            // Unreachable
        }

        if ( !aProperties.getLength() )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                                    u"No properties!"_ustr,
                                    getXWeak(),
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
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
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
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
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
                xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
            }
            aStaticDAVOptionsCache.removeDAVOptions( xResAccess->getURL() );
            // clean cached value of PROPFIND property names
            removeCachedPropertyNames( xResAccess->getURL() );
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
                uno::Any( lang::IllegalArgumentException(
                                  u"Wrong argument type!"_ustr,
                                  getXWeak(),
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
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
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
        // do not check for a DAV resource
        // the lock store will be checked before sending
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
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
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
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
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
            ucbhelper::cancelCommandExecution( uno::Any( e ), Environment );
        }
        catch ( const beans::IllegalTypeException&e )
        {
            ucbhelper::cancelCommandExecution( uno::Any( e ), Environment );
        }
        catch ( const lang::IllegalArgumentException&e )
        {
            ucbhelper::cancelCommandExecution( uno::Any( e ), Environment );
        }
    }
    else if ( aCommand.Name == "removeProperty" )
    {
        OUString sPropName;
        if ( !( aCommand.Argument >>= sPropName ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::Any( lang::IllegalArgumentException(
                                    u"Wrong argument type!"_ustr,
                                    getXWeak(),
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
            ucbhelper::cancelCommandExecution( uno::Any( e ), Environment );
        }
        catch( const beans::NotRemoveableException &e )
        {
            ucbhelper::cancelCommandExecution( uno::Any( e ), Environment );
        }
    }
    else
    {

        // Unsupported command


        ucbhelper::cancelCommandExecution(
            uno::Any( ucb::UnsupportedCommandException(
                              aCommand.Name,
                              getXWeak() ) ),
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
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
        }
        xResAccess->abort();
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
            u"\"addProperty\" with empty Property.Name"_ustr,
            getXWeak(),
            -1 );

    // Check property type.
    if ( !UCBDeadPropertyValue::supportsType( aProperty.Type ) )
        throw beans::IllegalTypeException(
            u"\"addProperty\" unsupported Property.Type"_ustr,
            getXWeak() );

    // check default value
    if ( aDefaultValue.hasValue() && aDefaultValue.getValueType() != aProperty.Type )
        throw beans::IllegalTypeException(
            u"\"addProperty\" DefaultValue does not match Property.Type"_ustr,
            getXWeak() );


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
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
        }
        aStaticDAVOptionsCache.removeDAVOptions( xResAccess->getURL() );
        // clean cached value of PROPFIND property names
        // PROPPATCH can change them
        removeCachedPropertyNames( xResAccess->getURL() );
        xResAccess->PROPPATCH( aProppatchValues, xEnv );
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
        }

        // Notify propertyset info change listeners.
        beans::PropertySetInfoChangeEvent evt(
            getXWeak(),
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
                    const ResourceType eType = getResourceType( xEnv );
                    switch ( eType )
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
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
        }
        aStaticDAVOptionsCache.removeDAVOptions( xResAccess->getURL() );
        // clean cached value of PROPFIND property names
        // PROPPATCH can change them
        removeCachedPropertyNames( xResAccess->getURL() );
        xResAccess->PROPPATCH( aProppatchValues, xEnv );
        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
        }

        // Notify propertyset info change listeners.
        beans::PropertySetInfoChangeEvent evt(
            getXWeak(),
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
                    const ResourceType eType = getResourceType( xEnv );
                    switch ( eType )
                    {
                    case UNKNOWN:
                    case DAV:
                        throw beans::UnknownPropertyException(Name);

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
    m_pProvider->getProperty( u"Title"_ustr, aProp );

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
                    xAdditionalPropSet =
                            rProvider->getAdditionalPropertySet( rContentId,
                                                                 false );
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
        uno::Reference< beans::XPropertySet > xSet =
            rProvider->getAdditionalPropertySet( rContentId, false );
        xRow->appendPropertySet( xSet );
    }

    return uno::Reference<sdbc::XRow>(xRow);
}

namespace {
void GetPropsUsingHeadRequest(DAVResource& resource,
                              const std::unique_ptr< DAVResourceAccess >& xResAccess,
                              const std::vector< OUString >& aHTTPNames,
                              const uno::Reference< ucb::XCommandEnvironment >& xEnv)
{
    if (!aHTTPNames.empty())
    {
        DAVOptions aDAVOptions;
        OUString   aTargetURL = xResAccess->getURL();
        // retrieve the cached options if any
        aStaticDAVOptionsCache.getDAVOptions(aTargetURL, aDAVOptions);

        // clean cached value of PROPFIND property names
        // PROPPATCH can change them
        Content::removeCachedPropertyNames(aTargetURL);
        // test if HEAD allowed, if not, throw, should be caught immediately
        // SC_GONE used internally by us, see comment in Content::getPropertyValues
        // in the catch scope
        if (aDAVOptions.getHttpResponseStatusCode() != SC_GONE &&
            !aDAVOptions.isHeadAllowed())
        {
            throw DAVException(DAVException::DAV_HTTP_ERROR, u"405 Not Implemented"_ustr, SC_METHOD_NOT_ALLOWED);
        }
        // if HEAD is enabled on this site
        // check if there is a relevant HTTP response status code cached
        if (aDAVOptions.getHttpResponseStatusCode() != SC_NONE)
        {
            // throws exception as if there was a server error, a DAV exception
            throw DAVException(DAVException::DAV_HTTP_ERROR,
                aDAVOptions.getHttpResponseStatusText(),
                aDAVOptions.getHttpResponseStatusCode());
            // Unreachable
        }

        xResAccess->HEAD(aHTTPNames, resource, xEnv);
    }
}
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

        aUnescapedTitle = DecodeURI(m_aEscapedTitle);
        xContext.set( m_xContext );
        xIdentifier.set( m_xIdentifier );
        xProvider = m_xProvider;
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );

        // First, ask cache...
        if (m_xCachedProps)
        {
            xCachedProps.reset( new ContentProperties( *m_xCachedProps ) );

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

    bool bNetworkAccessAllowed = true;

    if ( !m_bTransient && !bHasAll )
    {
        // Obtain values from server...


        // First, identify whether resource is DAV or not
        const ResourceType eType = getResourceType(
                xEnv, xResAccess, &bNetworkAccessAllowed );

        if ( eType == DAV )
        {
            // cache lookup... getResourceType may fill the props cache via
            // PROPFIND!
            if (m_xCachedProps)
            {
                xCachedProps.reset(
                    new ContentProperties( *m_xCachedProps ) );

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

                uno::Sequence< beans::Property > aProperties(rProperties);

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
#if defined SAL_LOG_INFO
                            {//debug
                                // print received resources
                                std::vector< DAVPropertyValue >::const_iterator it = resources[0].properties.begin();
                                std::vector< DAVPropertyValue >::const_iterator end = resources[0].properties.end();
                                while ( it != end )
                                {
                                    OUString aPropValue;
                                    bool    bValue;
                                    uno::Sequence< ucb::LockEntry > aSupportedLocks;
                                    if( (*it).Value >>= aPropValue )
                                        SAL_INFO( "ucb.ucp.webdav", "PROPFIND (getPropertyValues) - returned property: " << (*it).Name << ":" << aPropValue );
                                    else if( (*it).Value >>= bValue )
                                        SAL_INFO( "ucb.ucp.webdav", "PROPFIND (getPropertyValues) - returned property: " << (*it).Name << ":" <<
                                                  ( bValue ? "true" : "false" ) );
                                    else if( (*it).Value >>= aSupportedLocks )
                                    {
                                        SAL_INFO( "ucb.ucp.webdav", "PROPFIND (getPropertyValues) - returned property: " << (*it).Name << ":" );
                                        for ( sal_Int32 n = 0; n < aSupportedLocks.getLength(); ++n )
                                        {
                                            SAL_INFO( "ucb.ucp.webdav","      scope: "
                                                      << (aSupportedLocks[n].Scope == ucb::LockScope_SHARED ? "shared" : "exclusive")
                                                      << ", type: "
                                                      << (aSupportedLocks[n].Type != ucb::LockType_WRITE ? "" : "write") );
                                        }
                                    }
                                    ++it;
                                }
                            }
#endif
                            if (xProps)
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
            // All properties obtained already?
            std::vector< OUString > aMissingProps;
            if ( !( xProps
                    && xProps->containsAllNames(rProperties, aMissingProps))
                // i#121922 for non-DAV, uncacheable properties must be fetched
                // regardless of m_bDidGetOrHead.
                // But SharePoint may do weird things on HEAD so for DAV
                // only do this if required.
                && (eType != DAV || !m_bDidGetOrHead))
            {
                // Possibly the missing props can be obtained using a HEAD
                // request.

                std::vector< OUString > aHeaderNames;
                ContentProperties::UCBNamesToHTTPNames(
                    rProperties,
                    aHeaderNames );

                if( eType != DAV )
                {
                    // in case of not DAV PROFIND (previously in program flow) failed
                    // so we need to add the only prop that's common
                    // to DAV and NON_DAV: MediaType, that maps to Content-Type
                    aHeaderNames.emplace_back("Content-Type");
                }

                if (!aHeaderNames.empty()) try
                {
                    DAVResource resource;
                    GetPropsUsingHeadRequest(resource, xResAccess, aHeaderNames, xEnv);
                    m_bDidGetOrHead = true;

                    if (xProps)
                        xProps->addProperties(
                            aMissingProps,
                            ContentProperties( resource ) );
                    else
                        xProps.reset ( new ContentProperties( resource ) );

                    if (m_eResourceType == NON_DAV)
                        xProps->addProperties(aMissingProps,
                            ContentProperties(
                                aUnescapedTitle,
                                false));
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
                    OUString aTargetURL = xResAccess->getURL();

                    if ( e.getError() == DAVException::DAV_HTTP_ERROR )
                    {
                        // According to the spec. the origin server SHOULD return
                        // * 405 (Method Not Allowed):
                        //      the method is known but not allowed for the requested resource
                        // * 501 (Not Implemented):
                        //      the method is unrecognized or not implemented
                        // * 404 (SC_NOT_FOUND)
                        //      is for google-code server and for MS IIS 10.0 Web server
                        //      when only GET is enabled
                        if ( aLastException.getStatus() == SC_NOT_IMPLEMENTED ||
                             aLastException.getStatus() == SC_METHOD_NOT_ALLOWED ||
                             aLastException.getStatus() == SC_NOT_FOUND )
                        {
                            SAL_WARN( "ucb.ucp.webdav", "HEAD probably not implemented: fall back to a partial GET" );
                            aStaticDAVOptionsCache.setHeadAllowed( aTargetURL, false );
                            lcl_sendPartialGETRequest( bError,
                                                       aLastException,
                                                       aMissingProps,
                                                       aHeaderNames,
                                                       xResAccess,
                                                       xProps,
                                                       xEnv );
                            m_bDidGetOrHead = !bError;
                        }
                    }

                    if ( bError )
                    {
                        DAVOptions aDAVOptionsException;

                        aDAVOptionsException.setURL( aTargetURL );
                        // check if the error was SC_NOT_FOUND, meaning that the
                        // GET fall back didn't succeeded and the element is really missing
                        // we will consider the resource SC_GONE (410) for some time
                        // we use SC_GONE because has the same meaning of SC_NOT_FOUND (404)
                        // see:
                        // <https://tools.ietf.org/html/rfc7231#section-6.5.9> (retrieved 2016-10-09)
                        // apparently it's not used to mark the missing HEAD method (so far...)
                        sal_uInt16 ResponseStatusCode =
                            ( aLastException.getStatus() == SC_NOT_FOUND ) ?
                            SC_GONE :
                            aLastException.getStatus();
                        aDAVOptionsException.setHttpResponseStatusCode( ResponseStatusCode );
                        aDAVOptionsException.setHttpResponseStatusText( aLastException.getData() );
                        aStaticDAVOptionsCache.addDAVOptions( aDAVOptionsException,
                                                              m_nOptsCacheLifeNotFound );

                        if ( !shouldAccessNetworkAfterException( aLastException ) )
                        {
                            cancelCommandExecution( aLastException, xEnv );
                            // unreachable
                        }
                    }
                }
            }
        }

        // might trigger HTTP redirect.
        // Therefore, title must be updated here.
        CurlUri const aUri( xResAccess->getURL() );
        aUnescapedTitle = aUri.GetPathBaseNameUnescaped();

        if ( eType == UNKNOWN )
        {
            xProps.reset( new ContentProperties( aUnescapedTitle ) );
        }

        // For DAV resources we only know the Title, for non-DAV
        // resources we additionally know that it is a document.

        else if ( eType == DAV )
        {
            if (!xProps)
                xProps.reset(new ContentProperties(aUnescapedTitle));
            else
                xProps->addProperty(u"Title"_ustr, uno::Any(aUnescapedTitle), true);
        }
        else
        {
            if (!xProps)
                xProps.reset( new ContentProperties( aUnescapedTitle, false ) );
            else
                xProps->addProperty(
                    u"Title"_ustr,
                    uno::Any( aUnescapedTitle ),
                    true );

            xProps->addProperty(
                u"IsFolder"_ustr,
                uno::Any( false ),
                true );
            xProps->addProperty(
                u"IsDocument"_ustr,
                uno::Any( true ),
                true );
            xProps->addProperty(
                u"ContentType"_ustr,
                uno::Any( WEBDAV_CONTENT_TYPE ),
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

    // Add a default for the properties requested but not found.
    // Determine still missing properties, add a default.
    // Some client function doesn't expect a void uno::Any,
    // but instead wants some sort of default.
    std::vector< OUString > aMissingProps;
    if ( !xProps->containsAllNames(
                rProperties, aMissingProps ) )
    {
        //
        for ( std::vector< rtl::OUString >::const_iterator it = aMissingProps.begin();
              it != aMissingProps.end(); ++it )
        {
            // For the time being only a couple of properties need to be added
            if ( (*it) == "DateModified"  || (*it) == "DateCreated" )
            {
                util::DateTime aDate;
                xProps->addProperty(
                    (*it),
                    uno::Any( aDate ),
                    true );
            }
            else if (bNetworkAccessAllowed) // don't set these if connection failed
            {
                // If WebDAV didn't return the resource type, assume default
                // This happens e.g. for lists exported by SharePoint
                if ((*it) == "IsFolder")
                {
                    xProps->addProperty(
                        (*it),
                        uno::Any( false ),
                        true );
                }
                else if ((*it) == "IsDocument")
                {
                    xProps->addProperty(
                        (*it),
                        uno::Any( true ),
                        true );
                }
            }
        }
    }

    sal_Int32 nCount = rProperties.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const OUString rName = rProperties[ n ].Name;
        if ( rName == "BaseURI" )
        {
            // Add BaseURI property, if requested.
            xProps->addProperty(
                 u"BaseURI"_ustr,
                 uno::Any( getBaseURI( xResAccess ) ),
                 true );
        }
        else if ( rName == "CreatableContentsInfo" )
        {
            // Add CreatableContentsInfo property, if requested.
            bool bFolder = false;
            xProps->getValue( u"IsFolder"_ustr )
                    >>= bFolder;
            xProps->addProperty(
                u"CreatableContentsInfo"_ustr,
                uno::Any( bFolder
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

        if (!m_xCachedProps)
            m_xCachedProps.reset( new CachableContentProperties( *xProps ) );
        else
            m_xCachedProps->addProperties( *xProps );

        m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
        m_aEscapedTitle = EncodeSegment(aUnescapedTitle);
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
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
    }

    uno::Sequence< uno::Any > aRet( rValues.getLength() );
    auto aRetRange = asNonConstRange(aRet);
    uno::Sequence< beans::PropertyChangeEvent > aChanges( rValues.getLength() );
    sal_Int32 nChanged = 0;

    beans::PropertyChangeEvent aEvent;
    aEvent.Source         = getXWeak();
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
            aRetRange[ n ] <<= lang::IllegalAccessException(
                            u"Property is read-only!"_ustr,
                            getXWeak() );
            continue;
        }


        // Mandatory props.


        if ( rName == "ContentType" )
        {
            // Read-only property!
            aRetRange[ n ] <<= lang::IllegalAccessException(
                u"Property is read-only!"_ustr,
                getXWeak() );
        }
        else if ( rName == "IsDocument" )
        {
            // Read-only property!
            aRetRange[ n ] <<= lang::IllegalAccessException(
                u"Property is read-only!"_ustr,
                getXWeak() );
        }
        else if ( rName == "IsFolder" )
        {
            // Read-only property!
            aRetRange[ n ] <<= lang::IllegalAccessException(
                            u"Property is read-only!"_ustr,
                            getXWeak() );
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
                        CurlUri const aURI(xIdentifier->getContentIdentifier());
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
                        aRetRange[ n ] <<= lang::IllegalArgumentException(
                            u"Invalid content identifier!"_ustr,
                            getXWeak(),
                            -1 );
                    }
                }
                else
                {
                    aRetRange[ n ] <<= lang::IllegalArgumentException(
                        u"Empty title not allowed!"_ustr,
                        getXWeak(),
                        -1 );
                }
            }
            else
            {
                aRetRange[ n ] <<= beans::IllegalTypeException(
                    u"Property value has wrong type!"_ustr,
                    getXWeak() );
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
                aRetRange[ n ] <<= beans::UnknownPropertyException(
                                u"Property is unknown!"_ustr,
                                getXWeak() );
                continue;
            }

            if ( rName == "Size" )
            {
                // Read-only property!
                aRetRange[ n ] <<= lang::IllegalAccessException(
                                u"Property is read-only!"_ustr,
                                getXWeak() );
            }
            else if ( rName == "DateCreated" )
            {
                // Read-only property!
                aRetRange[ n ] <<= lang::IllegalAccessException(
                                u"Property is read-only!"_ustr,
                                getXWeak() );
            }
            else if ( rName == "DateModified" )
            {
                // Read-only property!
                aRetRange[ n ] <<= lang::IllegalAccessException(
                                u"Property is read-only!"_ustr,
                                getXWeak() );
            }
            else if ( rName == "MediaType" )
            {
                // Read-only property!
                // (but could be writable, if 'getcontenttype' would be)
                aRetRange[ n ] <<= lang::IllegalAccessException(
                                u"Property is read-only!"_ustr,
                                getXWeak() );
            }
            if ( rName == "CreatableContentsInfo" )
            {
                // Read-only property!
                aRetRange[ n ] <<= lang::IllegalAccessException(
                                u"Property is read-only!"_ustr,
                                getXWeak() );
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
                            aRetRange[ n ] <<= e;
                        }
                        catch ( lang::WrappedTargetException const & e )
                        {
                            aRetRange[ n ] <<= e;
                        }
                        catch ( beans::PropertyVetoException const & e )
                        {
                            aRetRange[ n ] <<= e;
                        }
                        catch ( lang::IllegalArgumentException const & e )
                        {
                            aRetRange[ n ] <<= e;
                        }
                    }
                    else
                    {
                        aRetRange[ n ] <<= uno::Exception(
                                u"No property set for storing the value!"_ustr,
                                getXWeak() );
                    }
                }
            }
        }
    } // for

    if ( !bTransient && (!aProppatchValues.empty()) )
    {
        try
        {
            // clean cached value of PROPFIND property names
            // PROPPATCH can change them
            removeCachedPropertyNames( xResAccess->getURL() );
            // Set property values at server.
            aStaticDAVOptionsCache.removeDAVOptions( xResAccess->getURL() );
            xResAccess->PROPPATCH( aProppatchValues, xEnv );

            for ( const auto& rProppatchValue : aProppatchValues )
            {
                aEvent.PropertyName = rProppatchValue.name;
                aEvent.OldValue     = uno::Any(); // @@@ too expensive to obtain!
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
                aRetRange[ (*it) ] <<= MapDAVException( e, true );
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

        aNewURL += EncodeSegment(aNewTitle);

        uno::Reference< ucb::XContentIdentifier > xNewId
            = new ::ucbhelper::ContentIdentifier( aNewURL );
        uno::Reference< ucb::XContentIdentifier > xOldId = xIdentifier;

        try
        {
            CurlUri const sourceURI( xOldId->getContentIdentifier() );
            CurlUri targetURI( xNewId->getContentIdentifier() );

            targetURI.SetScheme( sourceURI.GetScheme() );

            // clean cached value of PROPFIND property names
            removeCachedPropertyNames( sourceURI.GetURI() );
            removeCachedPropertyNames( targetURI.GetURI() );
            aStaticDAVOptionsCache.removeDAVOptions( sourceURI.GetURI() );
            aStaticDAVOptionsCache.removeDAVOptions( targetURI.GetURI() );
            xResAccess->MOVE(
                sourceURI.GetRelativeReference(), targetURI.GetURI(), false, xEnv );

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
                aRetRange[ nTitlePos ] <<= uno::Exception(
                    u"Exchange failed!"_ustr,
                    getXWeak() );
            }
        }
        catch ( DAVException const & e )
        {
            // Do not set new title!
            aNewTitle.clear();

            // Set error .
            aRetRange[ nTitlePos ] = MapDAVException( e, true );
        }
    }

    if ( aNewTitle.getLength() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        aEvent.PropertyName = "Title";
        aEvent.OldValue     <<= aOldTitle;
        aEvent.NewValue     <<= aNewTitle;

        m_aEscapedTitle     = EncodeSegment(aNewTitle);

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
                const ucb::OpenCommandArgument3 & rArg,
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

            ucbhelper::cancelCommandExecution(
                uno::Any(
                    lang::IllegalArgumentException(
                        u"Non-folder resource cannot be opened as folder! Wrong Open Mode!"_ustr,
                        getXWeak(),
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
                uno::Any(
                    ucb::UnsupportedOpenModeException(
                            OUString(),
                            getXWeak(),
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
                        new DAVResourceAccess( *m_xResAccess ) );
                }

                xResAccess->setFlags( rArg.OpeningFlags );
                DAVResource aResource;
                std::vector< OUString > aHeaders;

                removeCachedPropertyNames( xResAccess->getURL() );
                xResAccess->GET( xOut, aHeaders, aResource, xEnv );
                m_bDidGetOrHead = true;

                {
                    osl::MutexGuard aGuard( m_aMutex );

                    // cache headers.
                    if (!m_xCachedProps)
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
                OUString aTargetURL =  m_xIdentifier->getContentIdentifier();
                try
                {
                    std::unique_ptr< DAVResourceAccess > xResAccess;
                    {
                        osl::MutexGuard aGuard( m_aMutex );

                        xResAccess.reset(
                            new DAVResourceAccess( *m_xResAccess ) );
                    }
                    xResAccess->setFlags( rArg.OpeningFlags );

                    // fill inputstream sync; return if all data present
                    DAVResource aResource;
                    std::vector< OUString > aHeaders;

                    aTargetURL = xResAccess->getURL();
                    removeCachedPropertyNames( aTargetURL );
                    // check if the resource was present on the server
                    // first update it, if necessary
                    // if the open is called directly, without the default open sequence,
                    // e.g. the one used when opening a file looking for properties
                    // first this call will have no effect, since OPTIONS would have already been called
                    // as a consequence of getPropertyValues()
                    DAVOptions aDAVOptions;
                    getResourceOptions( xEnv, aDAVOptions, xResAccess );

                    if (aDAVOptions.getHttpResponseStatusCode() != SC_NONE
                        // tdf#148426 fall back to GET in case of 500
                        && aDAVOptions.getHttpResponseStatusCode() != SC_INTERNAL_SERVER_ERROR)
                    {
                        // throws exception as if there was a server error, a DAV exception
                        throw DAVException( DAVException::DAV_HTTP_ERROR,
                                            aDAVOptions.getHttpResponseStatusText(),
                                            aDAVOptions.getHttpResponseStatusCode() );
                    }
                    uno::Reference< io::XInputStream > xIn
                        = xResAccess->GET( aHeaders, aResource, xEnv );
                    m_bDidGetOrHead = true;

                    {
                        osl::MutexGuard aGuard( m_aMutex );

                        // cache headers.
                        if (!m_xCachedProps)
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
                    //TODO cache the http error if not yet cached
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
                    uno::Any(
                        ucb::UnsupportedDataSinkException(
                            OUString(),
                            getXWeak(),
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
                    new DAVResourceAccess( *m_xResAccess ) );
            }

            removeCachedPropertyNames( xResAccess->getURL() );
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
                        new DAVResourceAccess( *m_xResAccess ) );
                }

                removeCachedPropertyNames( xResAccess->getURL() );
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
                uno::Any(
                    ucb::UnsupportedDataSinkException(
                        OUString(),
                        getXWeak(),
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
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
    }

    // Check, if all required properties are present.

    if ( aEscapedTitle.isEmpty() )
    {
        SAL_WARN( "ucb.ucp.webdav", "Content::insert - Title missing!" );

        uno::Sequence<OUString> aProps { u"Title"_ustr };
        ucbhelper::cancelCommandExecution(
            uno::Any( ucb::MissingPropertiesException(
                                OUString(),
                                getXWeak(),
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
                u"Unable to write without overwrite!"_ustr,
                getXWeak(),
                ucb::NameClash::ERROR );

            uno::Reference< task::XInteractionHandler > xIH;

            if ( Environment.is() )
                xIH = Environment->getInteractionHandler();

            if ( xIH.is() )
            {
                uno::Any aExAsAny( aEx );

                rtl::Reference< ucbhelper::SimpleInteractionRequest > xRequest
                    = new ucbhelper::SimpleInteractionRequest(
                        aExAsAny,
                        ContinuationFlags::Approve
                            | ContinuationFlags::Disapprove );
                xIH->handle( xRequest );

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
                                    u"Unknown interaction selection!"_ustr,
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
            {
                aStaticDAVOptionsCache.removeDAVOptions( xResAccess->getURL() );
                removeCachedPropertyNames( xResAccess->getURL() );
                xResAccess->MKCOL( Environment );
            }
            else
            {
                // remove options from cache, PUT may change it
                // it will be refreshed when needed
                aStaticDAVOptionsCache.removeDAVOptions( xResAccess->getURL() );
                removeCachedPropertyNames( xResAccess->getURL() );
                xResAccess->PUT( xInputStream, Environment );
                // clean cached value of PROPFIND properties names
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
                            removeCachedPropertyNames( xResAccess->getURL() );
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
                            CurlUri const aURI( aURL );
                            aTitle = aURI.GetPathBaseNameUnescaped();
                        }
                        catch ( DAVException const & )
                        {
                        }

                        ucbhelper::cancelCommandExecution(
                            uno::Any(
                                ucb::NameClashException(
                                    OUString(),
                                    getXWeak(),
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
                uno::Any(
                    ucb::MissingInputStreamException(
                        OUString(),
                        getXWeak() ) ),
                Environment );
            // Unreachable
        }

        // save the URL since it may change due to redirection
        OUString    aTargetUrl = xResAccess->getURL();
        try
        {
            removeCachedPropertyNames( xResAccess->getURL() );
            // remove options from cache, PUT may change it
            // it will be refreshed when needed
            aStaticDAVOptionsCache.removeDAVOptions( aTargetUrl );
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
        xProvider.set( m_xProvider );
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
    }

    OUString aTargetURI;
    try
    {
        CurlUri sourceURI( rArgs.SourceURL );
        CurlUri targetURI( xIdentifier->getContentIdentifier() );

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
                    uno::Any(
                        ucb::InteractiveBadTransferURLException(
                            u"Unsupported URL scheme!"_ustr,
                            getXWeak() ) ),
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
                uno::Any( ucb::InteractiveBadTransferURLException(
                                u"Different hosts!"_ustr,
                                getXWeak() ) ),
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

            aStaticDAVOptionsCache.removeDAVOptions( sourceURI.GetURI() );
            aStaticDAVOptionsCache.removeDAVOptions( targetURI.GetURI() );
            aSourceAccess.MOVE( sourceURI.GetRelativeReference(),
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

            aStaticDAVOptionsCache.removeDAVOptions( sourceURI.GetURI() );
            aStaticDAVOptionsCache.removeDAVOptions( targetURI.GetURI() );
            aSourceAccess.COPY( sourceURI.GetRelativeReference(),
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
                        uno::Any(
                            ucb::NameClashException(
                                OUString(),
                                getXWeak(),
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
                        uno::Any(
                            ucb::UnsupportedNameClashException(
                                OUString(),
                                getXWeak(),
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

// returns the resource type, to be checked for locks
Content::ResourceType Content::resourceTypeForLocks(
    const uno::Reference< ucb::XCommandEnvironment >& Environment,
    const std::unique_ptr< DAVResourceAccess > & rResAccess)
{
    ResourceType eResourceTypeForLocks = UNKNOWN;
    {
        osl::MutexGuard g(m_aMutex);
        //check if cache contains what we need, usually the first PROPFIND on the URI has supported lock
        if (m_xCachedProps)
        {
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

        //{
            DAVOptions aDAVOptions;
            getResourceOptions( Environment, aDAVOptions, rResAccess );
            if( aDAVOptions.isClass1() ||
                aDAVOptions.isClass2() ||
                aDAVOptions.isClass3() )
            {
                // this is at least a DAV, lock to be confirmed
                // class 2 is needed for full lock support
                // see
                // <https://tools.ietf.org/html/rfc4918#section-18.2>
                eResourceTypeForLocks = DAV_NOLOCK;
                if( aDAVOptions.isClass2() )
                {
                    // ok, possible lock, check for it
                    try
                    {
                        // we need only DAV:supportedlock
                        std::vector< DAVResource > resources;
                        std::vector< OUString > aPropNames;
                        uno::Sequence< beans::Property > aProperties( 1 );
                        aProperties.getArray()[ 0 ].Name = DAVProperties::SUPPORTEDLOCK;

                        ContentProperties::UCBNamesToDAVNames( aProperties, aPropNames );
                        rResAccess->PROPFIND( DAVZERO, aPropNames, resources, Environment );

                        bool wasSupportedlockFound = false;

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
                                    wasSupportedlockFound = true;
                                    uno::Sequence< ucb::LockEntry > aSupportedLocks;
                                    if ( (*it).Value >>= aSupportedLocks )
                                    {
                                        for ( sal_Int32 n = 0; n < aSupportedLocks.getLength(); ++n )
                                        {
                                            // TODO: if the lock type is changed from 'exclusive write' to 'shared write'
                                            // e.g. to implement 'Calc shared file feature', the ucb::LockScope_EXCLUSIVE
                                            // value should be checked as well, adaptation the code may be needed
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
                        else
                        {
                            // PROPFIND failed; check if HEAD contains Content-Disposition: attachment (RFC1806, HTTP/1.1 19.5.1),
                            // which supposedly means no lock for the resource (happens e.g. with SharePoint exported lists)
                            OUString sContentDisposition;
                            // First, check cached properties
                            if (m_xCachedProps)
                            {
                                if ((m_xCachedProps->getValue(u"Content-Disposition"_ustr) >>= sContentDisposition)
                                    && sContentDisposition.startsWithIgnoreAsciiCase("attachment"))
                                {
                                    eResourceTypeForLocks = DAV_NOLOCK;
                                    wasSupportedlockFound = true;
                                }
                            }
                            // If no data in cache, try HEAD request
                            if (sContentDisposition.isEmpty() && !m_bDidGetOrHead) try
                            {
                                DAVResource resource;
                                GetPropsUsingHeadRequest(resource, rResAccess, {u"Content-Disposition"_ustr}, Environment);
                                m_bDidGetOrHead = true;
                                for (const auto& it : resource.properties)
                                {
                                    if (it.Name.equalsIgnoreAsciiCase("Content-Disposition"))
                                    {
                                        if ((it.Value >>= sContentDisposition) && sContentDisposition.equalsIgnoreAsciiCase("attachment"))
                                        {
                                            eResourceTypeForLocks = DAV_NOLOCK;
                                            wasSupportedlockFound = true;
                                        }
                                        break;
                                    }
                                }
                            }
                            catch (...){}
                        }
                        // check if this is still only a DAV_NOLOCK
                        // a fallback for resources that do not have DAVProperties::SUPPORTEDLOCK property
                        // we check for the returned OPTION if LOCK is allowed on the resource
                        if ( !wasSupportedlockFound && eResourceTypeForLocks == DAV_NOLOCK )
                        {
                            SAL_INFO( "ucb.ucp.webdav", "This WebDAV server has no supportedlock property, check for allowed LOCK method in OPTIONS" );
                            // ATTENTION: if the lock type is changed from 'exclusive write' to 'shared write'
                            // e.g. to implement 'Calc shared file feature' on WebDAV directly, and we arrive to this fallback
                            // and the LOCK is allowed, we should assume that only exclusive write lock is available
                            // this is just a reminder...
                            if ( aDAVOptions.isLockAllowed() )
                                eResourceTypeForLocks = DAV;
                        }
                    }
                    catch ( DAVException const & e )
                    {
                        rResAccess->resetUri();
                        //grab the error code
                        switch( e.getStatus() )
                        {
                            case SC_NOT_FOUND:
                                SAL_WARN( "ucb.ucp.webdav", "resourceTypeForLocks() - URL: <"
                                          << m_xIdentifier->getContentIdentifier() << "> was not found. ");
                                eResourceTypeForLocks = NOT_FOUND;
                                break;
                                // some servers returns SC_FORBIDDEN, instead
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
                                          << m_xIdentifier->getContentIdentifier() << ">, DAV ExceptionCode: " << e.getError() << ", HTTP error: " << e.getStatus() );
                                eResourceTypeForLocks = NON_DAV;
                                break;
                            default:
                                //fallthrough
                                SAL_WARN( "ucb.ucp.webdav", "resourceTypeForLocks() DAVException - URL: <"
                                          << m_xIdentifier->getContentIdentifier() << ">, DAV ExceptionCode: " << e.getError() << ", HTTP error: " << e.getStatus() );
                                eResourceTypeForLocks = UNKNOWN;
                        }
                    }
                }
            }
            else
                eResourceTypeForLocks = NON_DAV;

        //}
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

Content::ResourceType Content::resourceTypeForLocks(
    const uno::Reference< ucb::XCommandEnvironment >& Environment )
{
    std::unique_ptr< DAVResourceAccess > xResAccess;
    {
        osl::MutexGuard aGuard( m_aMutex );
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
    }
    Content::ResourceType ret = resourceTypeForLocks( Environment, xResAccess );
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
    }
    return ret;
}

void Content::lock(
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
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
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
        }

        uno::Any aOwnerAny;
        OUString const user(officecfg::Office::Common::Save::Document::UseUserData::get()
                ? " - " + ::svt::LockFileCommon::GetOOOUserName()
                : OUString());
        aOwnerAny <<= OUString("LibreOffice" + user);

        ucb::Lock aLock(
            ucb::LockScope_EXCLUSIVE,
            ucb::LockType_WRITE,
            ucb::LockDepth_ZERO,
            aOwnerAny,
            180, // lock timeout in secs
            //-1, // infinite lock
            uno::Sequence< OUString >() );

        // OPTIONS may change as a consequence of the lock operation
        aStaticDAVOptionsCache.removeDAVOptions( xResAccess->getURL() );
        removeCachedPropertyNames( xResAccess->getURL() );
        xResAccess->LOCK( aLock, Environment );

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
        }
    }
    catch ( DAVException const & e )
    {
        // check if the exception thrown is 'already locked'
        // this exception is mapped directly to the ucb correct one, without
        // going into the cancelCommandExecution() user interaction
        // this exception should be managed by the issuer of 'lock' command
        switch( e.getError() )
        {
            case DAVException::DAV_LOCKED:
            {
                SAL_WARN( "ucb.ucp.webdav", "lock(): resource already locked - URL: <"
                          << m_xIdentifier->getContentIdentifier() << ">");
                throw
                    ucb::InteractiveLockingLockedException(
                        u"Locked!"_ustr,
                        getXWeak(),
                        task::InteractionClassification_ERROR,
                        aURL,
                        false );
            }
            break;
            case DAVException::DAV_HTTP_NOAUTH:
            case DAVException::DAV_HTTP_AUTH:
            {
                SAL_WARN( "ucb.ucp.webdav", "lock(): DAVException Authentication error - URL: <"
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
                        u"Authentication error while trying to lock! Write only WebDAV perhaps?"_ustr,
                        getXWeak(),
                        task::InteractionClassification_ERROR,
                        e.getData() );
            }
            break;
            case DAVException::DAV_HTTP_ERROR:
                //grab the error code
                switch( e.getStatus() )
                {
                    // The 'case SC_NOT_FOUND' just below tries to solve a problem in eXo Platform
                    // WebDAV connector which apparently fail on resource first creation
                    // rfc4918 section-7.3 (see link below)
                    case SC_NOT_FOUND:              // <http://tools.ietf.org/html/rfc7231#section-6.5.4>
                    // The 'case SC_PRECONDITION_FAILED' just below tries to solve a problem
                    // in SharePoint when locking the resource on first creation fails due to this:
                    // <https://msdn.microsoft.com/en-us/library/jj575265%28v=office.12%29.aspx#id15>
                    // (retrieved on 2015-08-14)
                    case SC_PRECONDITION_FAILED:    // <http://tools.ietf.org/html/rfc7232#section-4.2>
                        // Errors SC_NOT_IMPLEMENTED and SC_METHOD_NOT_ALLOWED are
                        // part of base http 1.1 RFCs
                    case SC_NOT_IMPLEMENTED:        // <http://tools.ietf.org/html/rfc7231#section-6.6.2>
                    case SC_METHOD_NOT_ALLOWED:     // <http://tools.ietf.org/html/rfc7231#section-6.5.5>
                        SAL_WARN( "ucb.ucp.webdav", "lock() DAVException (SC_NOT_FOUND, SC_PRECONDITION_FAILED, SC_NOT_IMPLEMENTED or SC_METHOD_NOT_ALLOWED) - URL: <"
                                  << m_xIdentifier->getContentIdentifier() << ">, DAV ExceptionCode: " << e.getError() << ", HTTP error: " << e.getStatus() );
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
                    default:
                        //fallthrough
                        ;
                }
                break;
            case DAVException::DAV_LOCKED_SELF:
                // we already hold the lock and it is in our internal lockstore
                // just return as if the lock was successful
                return;
            default:
                //fallthrough
                ;
        }

        SAL_WARN( "ucb.ucp.webdav","lock() DAVException - URL: <"
                  << m_xIdentifier->getContentIdentifier() << ">, DAV ExceptionCode: " << e.getError() << ", HTTP error: " << e.getStatus() );
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
            xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
        }

        // check if the target URL is a Class1 DAV
        DAVOptions aDAVOptions;
        getResourceOptions( Environment, aDAVOptions, xResAccess );

        // at least class one is needed
        if( aDAVOptions.isClass1() )
        {
            // remove options from cache, unlock may change it
            // it will be refreshed when needed
            aStaticDAVOptionsCache.removeDAVOptions( xResAccess->getURL() );
            // clean cached value of PROPFIND properties names
            removeCachedPropertyNames( xResAccess->getURL() );
            xResAccess->UNLOCK( Environment );
        }

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
        }
    }
    catch ( DAVException const & e )
    {
        switch( e.getError() )
        {
            case DAVException::DAV_NOT_LOCKED:
                SAL_WARN( "ucb.ucp.webdav", "unlock(): DAVException::DAV_NOT_LOCKED - URL: <"
                          << m_xIdentifier->getContentIdentifier() << ">");
                // means that we don't own any lock on this resource
                // intercepted here to remove a confusing indication to the user
                // unfortunately this happens in some WebDAV server configuration
                // acting as WebDAV and having lock/unlock enabled only
                // for authorized user.
                return;
            case DAVException::DAV_HTTP_ERROR:
                //grab the error code
                switch( e.getStatus() )
                {
                    // Errors SC_NOT_IMPLEMENTED and SC_METHOD_NOT_ALLOWED are
                    // part of base http 1.1 RFCs
                    case SC_NOT_IMPLEMENTED:        // <http://tools.ietf.org/html/rfc7231#section-6.6.2>
                    case SC_METHOD_NOT_ALLOWED:     // <http://tools.ietf.org/html/rfc7231#section-6.5.5>
                        SAL_WARN( "ucb.ucp.webdav", "unlock() DAVException (SC_NOT_IMPLEMENTED or SC_METHOD_NOT_ALLOWED) - URL: <"
                                  << m_xIdentifier->getContentIdentifier() << ">, DAV ExceptionCode: " << e.getError() << ", HTTP error: " << e.getStatus() );
                        return;
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
                  << m_xIdentifier->getContentIdentifier() << ">, DAV ExceptionCode: " << e.getError() << ", HTTP error: " << e.getStatus() );
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

    // Exchange own identity.

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
    auto pProperties = aProperties.getArray();
    pProperties[ 0 ].Name   = "IsFolder";
    pProperties[ 0 ].Handle = -1;
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
            uno::Sequence<uno::Any> aArgs{ uno::Any(beans::PropertyValue(
                u"Uri"_ustr, -1, uno::Any(aURL), beans::PropertyState_DIRECT_VALUE)) };

            aException <<=
                ucb::InteractiveAugmentedIOException(
                    u"Not found!"_ustr,
                    getXWeak(),
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
                        getXWeak(),
                        task::InteractionClassification_ERROR,
                        e.getData() );
            else
                aException <<=
                    ucb::InteractiveNetworkReadException(
                        e.getData(),
                        getXWeak(),
                        task::InteractionClassification_ERROR,
                        e.getData() );
            break;
        }

    case DAVException::DAV_HTTP_LOOKUP:
        aException <<=
            ucb::InteractiveNetworkResolveNameException(
                OUString(),
                getXWeak(),
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
                getXWeak(),
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
                getXWeak(),
                -1 );
        break;

    case DAVException::DAV_LOCKED:
#if 1
        aException <<=
            ucb::InteractiveLockingLockedException(
                u"Locked!"_ustr,
                getXWeak(),
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
                    getXWeak(),
                    task::InteractionClassification_ERROR,
                    ucb::IOErrorCode_LOCKING_VIOLATION,
                    aArgs );
        }
#endif
        break;

    case DAVException::DAV_LOCKED_SELF:
        aException <<=
            ucb::InteractiveLockingLockedException(
                u"Locked (self)!"_ustr,
                getXWeak(),
                task::InteractionClassification_ERROR,
                aURL,
                true ); // SelfOwned
        break;

    case DAVException::DAV_NOT_LOCKED:
        aException <<=
            ucb::InteractiveLockingNotLockedException(
                u"Not locked!"_ustr,
                getXWeak(),
                task::InteractionClassification_ERROR,
                aURL );
        break;

    case DAVException::DAV_LOCK_EXPIRED:
        aException <<=
            ucb::InteractiveLockingLockExpiredException(
                u"Lock expired!"_ustr,
                getXWeak(),
                task::InteractionClassification_ERROR,
                aURL );
        break;

    default:
        aException <<=
            ucb::InteractiveNetworkGeneralException(
                OUString(),
                getXWeak(),
                task::InteractionClassification_ERROR );
        break;
    }

    return aException;
}


// static
bool Content::shouldAccessNetworkAfterException( const DAVException & e )
{
    if ( ( e.getStatus() == SC_NOT_FOUND ) ||
         ( e.getStatus() == SC_GONE ) ||
         ( e.getError() == DAVException::DAV_HTTP_TIMEOUT ) ||
         ( e.getError() == DAVException::DAV_HTTP_LOOKUP ) ||
         ( e.getError() == DAVException::DAV_HTTP_CONNECT ) ||
         ( e.getError() == DAVException::DAV_HTTP_NOAUTH ) ||
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


OUString
Content::getBaseURI( const std::unique_ptr< DAVResourceAccess > & rResAccess )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    // First, try to obtain value of response header "Content-Location".
    if (m_xCachedProps)
    {
        OUString aLocation;
        m_xCachedProps->getValue( u"Content-Location"_ustr ) >>= aLocation;
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

// resource type is the type of the WebDAV resource
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
    DAVOptions aDAVOptions;

    {
        getResourceOptions( xEnv, aDAVOptions, rResAccess, networkAccessAllowed );

        // at least class one is needed
        if( aDAVOptions.isClass1() )
        {
            try
            {
                // Try to fetch some frequently used property value, e.g. those
                // used when loading documents... along with identifying whether
                // this is a DAV resource.
                std::vector< DAVResource > resources;
                std::vector< OUString > aPropNames;
                uno::Sequence< beans::Property > aProperties( 5 );
                auto pProperties = aProperties.getArray();
                pProperties[ 0 ].Name = "IsFolder";
                pProperties[ 1 ].Name = "IsDocument";
                pProperties[ 2 ].Name = "IsReadOnly";
                pProperties[ 3 ].Name = "MediaType";
                pProperties[ 4 ].Name = DAVProperties::SUPPORTEDLOCK;

                ContentProperties::UCBNamesToDAVNames( aProperties, aPropNames );

                rResAccess->PROPFIND( DAVZERO, aPropNames, resources, xEnv );

                if ( resources.size() == 1 )
                {
#if defined SAL_LOG_INFO
                    {//debug
                        // print received resources
                        std::vector< DAVPropertyValue >::const_iterator it = resources[0].properties.begin();
                        std::vector< DAVPropertyValue >::const_iterator end = resources[0].properties.end();
                        while ( it != end )
                        {
                            OUString aPropValue;
                            bool    bValue;
                            uno::Sequence< ucb::LockEntry > aSupportedLocks;
                            if((*it).Value >>= aPropValue )
                                SAL_INFO( "ucb.ucp.webdav", "PROPFIND (getResourceType) - ret'd prop: " << (*it).Name << ":" << aPropValue );
                            else if( (*it).Value >>= bValue )
                                SAL_INFO( "ucb.ucp.webdav", "PROPFIND (getResourceType) - ret'd prop: " << (*it).Name << ":" <<
                                          ( bValue ? "true" : "false" ) );
                            else if( (*it).Value >>= aSupportedLocks )
                            {
                                SAL_INFO( "ucb.ucp.webdav", "PROPFIND (getResourceType) - ret'd prop: " << (*it).Name << ":" );
                                for ( sal_Int32 n = 0; n < aSupportedLocks.getLength(); ++n )
                                {
                                    SAL_INFO( "ucb.ucp.webdav","PROPFIND (getResourceType) -       supportedlock[" << n <<"]: scope: "
                                              << (aSupportedLocks[n].Scope == ucb::LockScope_SHARED ? "shared" : "exclusive")
                                              << ", type: "
                                              << (aSupportedLocks[n].Type != ucb::LockType_WRITE ? "" : "write") );
                                }
                            }
                            ++it;
                        }
                    }
#endif
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

                SAL_WARN( "ucb.ucp.webdav", "Content::getResourceType returned errors, DAV ExceptionCode: " << e.getError() << ", HTTP error: "  << e.getStatus() );

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
                if ( e.getStatus() == SC_NOT_FOUND )
                {
                    // arrives here if OPTIONS is still cached for a resource previously available
                    // operate on the OPTIONS cache:
                    // if OPTIONS was not found, do nothing
                    // else OPTIONS returned on a resource not existent  (example a server that allows lock on null resource) set
                    // not found and adjust lifetime accordingly
                    DAVOptions aDAVOptionsInner;
                    if (aStaticDAVOptionsCache.getDAVOptions(rResAccess->getURL(), aDAVOptionsInner))
                    {
                        // TODO? get redirected url
                        aDAVOptionsInner.setHttpResponseStatusCode( e.getStatus() );
                        aDAVOptionsInner.setHttpResponseStatusText( e.getData() );
                        aStaticDAVOptionsCache.addDAVOptions( aDAVOptionsInner,
                                                              m_nOptsCacheLifeNotFound );
                    }
                }
                // if the two net events below happen, something
                // is going on to the connection so break the command flow
                if ( ( e.getError() == DAVException::DAV_HTTP_TIMEOUT ) ||
                     ( e.getError() == DAVException::DAV_HTTP_CONNECT ) )
                {
                    cancelCommandExecution( e, xEnv );
                    // unreachable
                }

                // cancel command execution is case that no user authentication data has been provided.
                if ( e.getError() == DAVException::DAV_HTTP_NOAUTH )
                {
                    cancelCommandExecution( e, uno::Reference< ucb::XCommandEnvironment >() );
                }
            }
        }
        else
        {
            rResAccess->resetUri();

            // first check if the cached error can be mapped to DAVException::DAV_HTTP_TIMEOUT or mapped to DAVException::DAV_HTTP_CONNECT
            if (aDAVOptions.getHttpResponseStatusCode() == USC_CONNECTION_TIMED_OUT
                // can't get any reliable info without auth => cancel request
                || aDAVOptions.getHttpResponseStatusCode() == USC_AUTH_FAILED
                || aDAVOptions.getHttpResponseStatusCode() == USC_AUTHPROXY_FAILED)
            {
                // behave same as DAVException::DAV_HTTP_TIMEOUT or DAVException::DAV_HTTP_CONNECT was thrown
                try
                {
                    // extract host name and connection port
                    CurlUri   theUri( rResAccess->getURL() );
                    OUString  aHostName  = theUri.GetHost();
                    sal_Int32 nPort      = theUri.GetPort();
                    DAVException::ExceptionCode e{};
                    switch (aDAVOptions.getHttpResponseStatusCode())
                    {
                        case USC_CONNECTION_TIMED_OUT:
                            e = DAVException::DAV_HTTP_TIMEOUT;
                            break;
                        case USC_AUTH_FAILED:
                            e = DAVException::DAV_HTTP_AUTH;
                            break;
                        case USC_AUTHPROXY_FAILED:
                            e = DAVException::DAV_HTTP_AUTHPROXY;
                            break;
                        default:
                            assert(false);
                    }
                    throw DAVException( e,
                                        ConnectionEndPointString(aHostName, nPort) );
                }
                catch ( DAVException& exp )
                {
                    cancelCommandExecution( exp, xEnv );
                }
            }

            if ( aDAVOptions.getHttpResponseStatusCode() != SC_NOT_FOUND &&
                 aDAVOptions.getHttpResponseStatusCode() != SC_GONE ) // the cached OPTIONS can have SC_GONE
            {
                eResourceType = NON_DAV;
            }
            else
            {
                //resource doesn't exist
                if ( networkAccessAllowed != nullptr )
                    *networkAccessAllowed = false;
            }
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
    SAL_INFO( "ucb.ucp.webdav", "m_eResourceType for <" << rResAccess->getURL() << ">: " << m_eResourceType );
    return m_eResourceType;
}


Content::ResourceType Content::getResourceType(
                    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    std::unique_ptr< DAVResourceAccess > xResAccess;
    {
        osl::MutexGuard aGuard( m_aMutex );
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess ) );
    }
    const Content::ResourceType & ret = getResourceType( xEnv, xResAccess );
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess ) );
    }
    return ret;
}


void Content::initOptsCacheLifeTime()
{
    // see description in
    // officecfg/registry/schema/org/openoffice/Inet.xcs
    // for use of these field values.
    sal_uInt32 nAtime;
    nAtime = officecfg::Inet::Settings::OptsCacheLifeImplWeb::get();
    m_nOptsCacheLifeImplWeb = std::max( sal_uInt32( 0 ),
                                        std::min( nAtime, sal_uInt32( 3600 ) ) );

    nAtime = officecfg::Inet::Settings::OptsCacheLifeDAV::get();
    m_nOptsCacheLifeDAV = std::max( sal_uInt32( 0 ),
                                    std::min( nAtime, sal_uInt32( 3600 ) ) );

    nAtime = officecfg::Inet::Settings::OptsCacheLifeDAVLocked::get();
    m_nOptsCacheLifeDAVLocked = std::max( sal_uInt32( 0 ),
                                    std::min( nAtime, sal_uInt32( 3600 ) ) );

    nAtime = officecfg::Inet::Settings::OptsCacheLifeNotImpl::get();
    m_nOptsCacheLifeNotImpl = std::max( sal_uInt32( 0 ),
                                              std::min( nAtime, sal_uInt32( 43200 ) ) );

    nAtime = officecfg::Inet::Settings::OptsCacheLifeNotFound::get();
    m_nOptsCacheLifeNotFound = std::max( sal_uInt32( 0 ),
                                              std::min( nAtime, sal_uInt32( 30 ) ) );
}


void Content::getResourceOptions(
                    const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv,
                    DAVOptions& rDAVOptions,
                    const std::unique_ptr< DAVResourceAccess > & rResAccess,
                    bool * networkAccessAllowed )
{
    OUString aRedirURL;
    OUString aTargetURL = rResAccess->getURL();
    DAVOptions aDAVOptions;
    // first check if in cache, if not, then send method to server
    if ( !aStaticDAVOptionsCache.getDAVOptions( aTargetURL, aDAVOptions ) )
    {
        try
        {
            rResAccess->OPTIONS( aDAVOptions, xEnv );
            // IMPORTANT:the correctly implemented server will answer without errors, even if the resource is not present
            sal_uInt32 nLifeTime = ( aDAVOptions.isClass1() ||
                                     aDAVOptions.isClass2() ||
                                     aDAVOptions.isClass3() ) ?
                m_nOptsCacheLifeDAV : // a WebDAV site
                m_nOptsCacheLifeImplWeb;  // a site implementing OPTIONS but
                                          // it's not DAV
            // if resource is locked, will use a
            // different lifetime
            if( aDAVOptions.isLocked() )
                nLifeTime = m_nOptsCacheLifeDAVLocked;

            // check if redirected
            aRedirURL = rResAccess->getURL();
            if( aRedirURL == aTargetURL)
            { // no redirection
                aRedirURL.clear();
            }
            // cache this URL's option
            aDAVOptions.setURL( aTargetURL );
            aDAVOptions.setRedirectedURL( aRedirURL );
            aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                  nLifeTime );
        }
        catch ( DAVException const & e )
        {
            // first, remove from cache, will be added if needed, depending on the error received
            aStaticDAVOptionsCache.removeDAVOptions( aTargetURL );
            rResAccess->resetUri();

            aDAVOptions.setURL( aTargetURL );
            aDAVOptions.setRedirectedURL( aRedirURL );
            switch( e.getError() )
            {
                case DAVException::DAV_HTTP_TIMEOUT:
                case DAVException::DAV_HTTP_CONNECT:
                {
                    // something bad happened to the connection
                    // not same as not found, this instead happens when the server doesn't exist or doesn't answer at all
                    // probably a new bit stating 'timed out' should be added to opts var?
                    // in any case abort the command
                    SAL_WARN( "ucb.ucp.webdav", "OPTIONS - DAVException: DAV_HTTP_TIMEOUT or DAV_HTTP_CONNECT for URL <" << m_xIdentifier->getContentIdentifier() << ">" );
                    // cache the internal unofficial status code

                    aDAVOptions.setHttpResponseStatusCode( USC_CONNECTION_TIMED_OUT );
                    // used only internally, so the text doesn't really matter..
                    aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                          m_nOptsCacheLifeNotFound );
                    if ( networkAccessAllowed != nullptr )
                    {
                        *networkAccessAllowed = *networkAccessAllowed
                            && shouldAccessNetworkAfterException(e);
                    }
                }
                break;
                case DAVException::DAV_HTTP_LOOKUP:
                {
                    SAL_WARN( "ucb.ucp.webdav", "OPTIONS - DAVException: DAV_HTTP_LOOKUP for URL <" << m_xIdentifier->getContentIdentifier() << ">" );
                    aDAVOptions.setHttpResponseStatusCode( USC_LOOKUP_FAILED );
                    // used only internally, so the text doesn't really matter..
                    aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                          m_nOptsCacheLifeNotFound );
                    if ( networkAccessAllowed != nullptr )
                    {
                        *networkAccessAllowed = *networkAccessAllowed
                            && shouldAccessNetworkAfterException(e);
                    }
                }
                break;
                case DAVException::DAV_HTTP_NOAUTH:
                case DAVException::DAV_HTTP_AUTH:
                {
                    SAL_WARN( "ucb.ucp.webdav", "OPTIONS - DAVException: DAV_HTTP_AUTH for URL <" << m_xIdentifier->getContentIdentifier() << ">" );
                    // - the remote site is a WebDAV with special configuration: read/only for read operations
                    //   and read/write for write operations, the user is not allowed to lock/write and
                    //   she cancelled the credentials request.
                    //   this is not actually an error, it means only that for current user this is a standard web,
                    //   though possibly DAV enabled
                    aDAVOptions.setHttpResponseStatusCode( USC_AUTH_FAILED );
                    // used only internally, so the text doesn't really matter..
                    if (xEnv && xEnv->getInteractionHandler())
                    {   // only cache if there actually was a chance to request auth
                        aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                              m_nOptsCacheLifeNotFound );
                    }
                    if ( networkAccessAllowed != nullptr )
                    {
                        *networkAccessAllowed = *networkAccessAllowed
                            && shouldAccessNetworkAfterException(e);
                    }
                }
                break;
                case DAVException::DAV_HTTP_AUTHPROXY:
                {
                    SAL_WARN( "ucb.ucp.webdav", "OPTIONS - DAVException: DAV_HTTP_AUTHPROXY for URL <" << m_xIdentifier->getContentIdentifier() << ">" );
                    aDAVOptions.setHttpResponseStatusCode( USC_AUTHPROXY_FAILED );
                    // used only internally, so the text doesn't really matter..
                    aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                          m_nOptsCacheLifeNotFound );
                    if ( networkAccessAllowed != nullptr )
                    {
                        *networkAccessAllowed = *networkAccessAllowed
                            && shouldAccessNetworkAfterException(e);
                    }
                }
                break;
                case DAVException::DAV_HTTP_ERROR:
                {
                    switch( e.getStatus() )
                    {
                        case SC_FORBIDDEN:
                        {
                            SAL_WARN( "ucb.ucp.webdav","OPTIONS - SC_FORBIDDEN for URL <" << m_xIdentifier->getContentIdentifier() << ">" );
                            // cache it, so OPTIONS won't be called again, this URL does not support it
                            aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                                  m_nOptsCacheLifeNotImpl );
                        }
                        break;
                        case SC_BAD_REQUEST:
                        case SC_INTERNAL_SERVER_ERROR:
                        {
                            SAL_WARN( "ucb.ucp.webdav","OPTIONS - SC_BAD_REQUEST or SC_INTERNAL_SERVER_ERROR for URL <" << m_xIdentifier->getContentIdentifier() << ">, HTTP error: "<< e.getStatus()
                                      << ", '" << e.getData() << "'" );
                            // cache it, so OPTIONS won't be called again, this URL detect some problem while answering the method
                            aDAVOptions.setHttpResponseStatusCode( e.getStatus() );
                            aDAVOptions.setHttpResponseStatusText( e.getData() );
                            aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                                  m_nOptsCacheLifeNotFound );
                        }
                        break;
                        case SC_NOT_IMPLEMENTED:
                        case SC_METHOD_NOT_ALLOWED:
                        {
                            // OPTIONS method must be implemented in DAV
                            // resource is NON_DAV, or not advertising it
                            SAL_WARN( "ucb.ucp.webdav","OPTIONS - SC_NOT_IMPLEMENTED or SC_METHOD_NOT_ALLOWED for URL <" << m_xIdentifier->getContentIdentifier() << ">, HTTP error: "<< e.getStatus()
                                      << ", '" << e.getData() << "'" );
                            // cache it, so OPTIONS won't be called again, this URL does not support it
                            aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                                  m_nOptsCacheLifeNotImpl );
                        }
                        break;
                        case SC_NOT_FOUND:
                        {
                            // Apparently on IIS 10.0, if you disabled OPTIONS method, this error is the one reported,
                            // instead of SC_NOT_IMPLEMENTED or SC_METHOD_NOT_ALLOWED.
                            // So check if this is an available resource, or a real 'Not Found' event.
                            sal_uInt32 nLifeTime = m_nOptsCacheLifeNotFound;
                            if( isResourceAvailable( xEnv, rResAccess, aDAVOptions ) )
                            {
                                SAL_WARN( "ucb.ucp.webdav", "OPTIONS - Got an SC_NOT_FOUND, but the URL <" << m_xIdentifier->getContentIdentifier() << "> resource exists" );
                                nLifeTime = m_nOptsCacheLifeNotImpl;
                            }
                            else
                            {
                                SAL_WARN( "ucb.ucp.webdav", "OPTIONS - SC_NOT_FOUND for URL <" << m_xIdentifier->getContentIdentifier() << ">" );
                                if ( networkAccessAllowed != nullptr )
                                {
                                    *networkAccessAllowed = *networkAccessAllowed
                                        && shouldAccessNetworkAfterException(e);
                                }
                            }
                            aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                                  nLifeTime );
                        }
                        break;
                        default:
                        {
                            SAL_WARN( "ucb.ucp.webdav", "OPTIONS - DAV_HTTP_ERROR, for URL <" << m_xIdentifier->getContentIdentifier() << ">, HTTP error: "<< e.getStatus()
                                      << ", '" << e.getData() << "'" );
                            aDAVOptions.setHttpResponseStatusCode( e.getStatus() );
                            aDAVOptions.setHttpResponseStatusText( e.getData() );
                            // cache it, so OPTIONS won't be called again, this URL does not support it
                            aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                                  m_nOptsCacheLifeNotImpl );
                        }
                        break;
                    }
                }
                break;
                // The 'DAVException::DAV_HTTP_REDIRECT' means we reached the maximum
                // number of redirections, consider the resource type as UNKNOWN
                // possibly a normal web site, not DAV
                case DAVException::DAV_HTTP_REDIRECT:
                default:
                {
                    SAL_WARN( "ucb.ucp.webdav","OPTIONS - General DAVException (or max DAV_HTTP_REDIRECT reached) for URL <" << m_xIdentifier->getContentIdentifier() << ">, DAV ExceptionCode: "
                              << e.getError() << ", HTTP error: "<< e.getStatus() );
                    aStaticDAVOptionsCache.addDAVOptions( aDAVOptions,
                                                          m_nOptsCacheLifeNotImpl );
                }
                break;
            }
        }
    }
    else
    {
        // check current response status code, perhaps we need to set networkAccessAllowed
        sal_uInt16 CachedResponseStatusCode = aDAVOptions.getHttpResponseStatusCode();
        if ( networkAccessAllowed != nullptr &&
             ( ( CachedResponseStatusCode == SC_NOT_FOUND ) ||
               ( CachedResponseStatusCode == SC_GONE ) ||
               ( CachedResponseStatusCode == USC_CONNECTION_TIMED_OUT ) ||
               ( CachedResponseStatusCode == USC_LOOKUP_FAILED ) ||
               ( CachedResponseStatusCode == USC_AUTH_FAILED ) ||
               ( CachedResponseStatusCode == USC_AUTHPROXY_FAILED )
                 )
            )
        {
            *networkAccessAllowed = *networkAccessAllowed && false;
        }
    }
    rDAVOptions = aDAVOptions;
}

//static
bool Content::isResourceAvailable( const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv,
                                  const std::unique_ptr< DAVResourceAccess > & rResAccess,
                                  DAVOptions& rDAVOptions )
{
    std::vector< rtl::OUString > aHeaderNames;
    DAVResource aResource;

    try
    {
        // To check for the physical URL resource availability, first
        // try using a simple HEAD command
        // if HEAD is successful, set element found.
        rResAccess->HEAD( aHeaderNames, aResource, xEnv );
        rDAVOptions.setHttpResponseStatusCode( 0 );
        rDAVOptions.setHttpResponseStatusText(u""_ustr);
        return true;
    }
    catch ( DAVException const & e )
    {
        if ( e.getError() == DAVException::DAV_HTTP_ERROR )
        {
            if ( e.getStatus() == SC_NOT_IMPLEMENTED ||
                 e.getStatus() == SC_METHOD_NOT_ALLOWED ||
                 e.getStatus() == SC_NOT_FOUND )
            {
                SAL_WARN( "ucb.ucp.webdav", "HEAD probably not implemented: fall back to a partial GET" );
                // set in cached OPTIONS "HEAD not implemented"
                // so it won't be used again on this resource
                rDAVOptions.setHeadAllowed( false );
                try
                {
                    // do a GET with a payload of 0, the server does not
                    // support HEAD (or has HEAD disabled)
                    DAVRequestHeaders aPartialGet;
                    aPartialGet.emplace_back(
                            u"Range"_ustr,
                            u"bytes=0-0"_ustr);

                    rResAccess->GET0( aPartialGet,
                                     aHeaderNames,
                                     aResource,
                                     xEnv );
                    return true;
                }
                catch ( DAVException const & ex )
                {
                    if ( ex.getError() == DAVException::DAV_HTTP_ERROR )
                    {
                        rDAVOptions.setHttpResponseStatusCode( ex.getStatus() );
                        rDAVOptions.setHttpResponseStatusText( ex.getData() );
                    }
                }
            }
            else
            {
                rDAVOptions.setHttpResponseStatusCode( e.getStatus() );
                rDAVOptions.setHttpResponseStatusText( e.getData() );
            }
        }
        return false;
    }
    catch ( ... )
    {
    }
    // set SC_NOT_IMPLEMENTED since at a minimum GET must be implemented in a basic Web server
    rDAVOptions.setHttpResponseStatusCode( SC_NOT_IMPLEMENTED );
    rDAVOptions.setHttpResponseStatusText(u""_ustr);
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
