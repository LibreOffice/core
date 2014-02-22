/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <osl/diagnose.h>
#include <osl/doublecheckedlocking.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
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

using namespace com::sun::star;
using namespace http_dav_ucp;

namespace
{
static void lcl_sendPartialGETRequest( bool &bError,
                                       DAVException &aLastException,
                                       const std::vector< rtl::OUString > aProps,
                                       std::vector< rtl::OUString > &aHeaderNames,
                                       const std::auto_ptr< DAVResourceAccess > &xResAccess,
                                       std::auto_ptr< ContentProperties > &xProps,
                                       const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    bool bIsRequestSize = false;
    DAVResource aResource;
    DAVRequestHeaders aPartialGet;
    aPartialGet.push_back(
        DAVRequestHeader(
            OUString( "Range" ),
            OUString( "bytes=0-0" )));

    for ( std::vector< rtl::OUString >::const_iterator it = aHeaderNames.begin();
            it != aHeaderNames.end(); ++it )
    {
        if ( *it == "Content-Length" )
        {
            bIsRequestSize = true;
            break;
        }
    }

    if ( bIsRequestSize )
    {
        
        
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
            
            
            
            rtl::OUString aAcceptRanges, aContentRange, aContentLength;
            std::vector< DAVPropertyValue > &aResponseProps = aResource.properties;
            for ( std::vector< DAVPropertyValue >::const_iterator it = aResponseProps.begin();
                    it != aResponseProps.end(); ++it )
            {
                if ( it->Name == "Accept-Ranges" )
                    it->Value >>= aAcceptRanges;
                else if ( it->Name == "Content-Range" )
                    it->Value >>= aContentRange;
                else if ( it->Name == "Content-Length" )
                    it->Value >>= aContentLength;
            }

            sal_Int64 nSize = 1;
            if ( aContentLength.getLength() )
            {
                nSize = aContentLength.toInt64();
            }

            
            
            
            if ( nSize == 1 &&
                    aContentRange.getLength() &&
                    aAcceptRanges == "bytes" )
            {
                
                
                
                sal_Int32 nSlash = aContentRange.lastIndexOf( '/' );
                if ( nSlash != -1 )
                {
                    rtl::OUString aSize = aContentRange.copy( nSlash + 1 );
                    
                    if ( aSize != "*" )
                    {
                        for ( std::vector< DAVPropertyValue >::iterator it = aResponseProps.begin();
                                it != aResponseProps.end(); ++it )
                        {
                            if (it->Name == "Content-Length")
                            {
                                it->Value <<= aSize;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if ( xProps.get() )
            xProps->addProperties(
                aProps,
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



//

//





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

    
}



Content::~Content()
{
}


//

//



void SAL_CALL Content::acquire()
    throw( )
{
    ContentImplHelper::acquire();
}



void SAL_CALL Content::release()
    throw( )
{
    ContentImplHelper::release();
}



uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType )
    throw ( uno::RuntimeException )
{
    
    
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
                OUString( "DefaultContext" ) ),
                uno::UNO_QUERY_THROW );

            uno::Reference< task::XInteractionHandler > xIH(
                task::PasswordContainerInteractionHandler::create( xCtx ) );

            
            
            

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


//

//


XTYPEPROVIDER_COMMON_IMPL( Content );



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
                        CPPU_TYPE_REF( ucb::XContentCreator ) ); 
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


//

//



OUString SAL_CALL Content::getImplementationName()
    throw( uno::RuntimeException )
{
    return OUString( "com.sun.star.comp.ucb.WebDAVContent" );
}



uno::Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = WEBDAV_CONTENT_SERVICE_NAME;
    return aSNS;
}


//

//



OUString SAL_CALL Content::getContentType()
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
        return OUString( WEBDAV_COLLECTION_TYPE );

    return OUString( WEBDAV_CONTENT_TYPE );
}


//

//



uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception,
           ucb::CommandAbortedException,
           uno::RuntimeException )
{
    OSL_TRACE( ">>>>> Content::execute: start: command: %s, env: %s",
               OUStringToOString( aCommand.Name,
                                       RTL_TEXTENCODING_UTF8 ).getStr(),
               Environment.is() ? "present" : "missing" );

    uno::Any aRet;

    if ( aCommand.Name == "getPropertyValues" )
    {
        
        
        

        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        aRet <<= getPropertyValues( Properties, Environment );
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {
        
        
        

        uno::Sequence< beans::PropertyValue > aProperties;
        if ( !( aCommand.Argument >>= aProperties ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        if ( !aProperties.getLength() )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "No properties!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        aRet <<= setPropertyValues( aProperties, Environment );
    }
    else if ( aCommand.Name == "getPropertySetInfo" )
    {
        
        
        

        
        aRet <<= getPropertySetInfo( Environment,
                                     sal_False /* don't cache data */ );
    }
    else if ( aCommand.Name == "getCommandInfo" )
    {
        
        
        

        
        aRet <<= getCommandInfo( Environment, sal_False );
    }
    else if ( aCommand.Name "open" )
    {
        
        
        

        ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        aRet = open( aOpenCommand, Environment );
    }
    else if ( aCommand.Name == "insert" )
    {
        
        
        

        ucb::InsertCommandArgument arg;
        if ( !( aCommand.Argument >>= arg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        insert( arg.Data, arg.ReplaceExisting, Environment );
    }
    else if ( aCommand.Name == "delete" )
    {
        
        
        

        sal_Bool bDeletePhysical = sal_False;
        aCommand.Argument >>= bDeletePhysical;






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
            
        }


        
        destroy( bDeletePhysical );

        
        removeAdditionalPropertySet( sal_True );
    }
    else if ( aCommand.Name == "transfer" && isFolder( Environment ) )
    {
        
        
        
        

        ucb::TransferInfo transferArgs;
        if ( !( aCommand.Argument >>= transferArgs ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                  OUString( "Wrong argument type!" ),
                                  static_cast< cppu::OWeakObject * >( this ),
                                  -1 ) ),
                Environment );
            
        }

        transfer( transferArgs, Environment );
    }
    else if ( aCommand.Name == "post" )
    {
        
        
        

        ucb::PostCommandArgument2 aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
        }

        post( aArg, Environment );
    }
    else if ( aCommand.Name == "lock" &&
              supportsExclusiveWriteLock( Environment ) )
    {
        
        
        

        lock( Environment );
    }
    else if ( aCommand.Name == "unlock" &&
              supportsExclusiveWriteLock( Environment ) )
    {
        
        
        

        unlock( Environment );
    }
    else if ( aCommand.Name == "createNewContent" &&
              isFolder( Environment ) )
    {
        
        
        

        ucb::ContentInfo aArg;
        if ( !( aCommand.Argument >>= aArg ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            
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
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
        }

        
        
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
        rtl::OUString sPropName;
        if ( !( aCommand.Argument >>= sPropName ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                    OUString( "Wrong argument type!" ),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
        }

        
        
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
        
        
        

        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::UnsupportedCommandException(
                              aCommand.Name,
                              static_cast< cppu::OWeakObject * >( this ) ) ),
            Environment );
        
    }

    OSL_TRACE( "<<<<< Content::execute: end: command: %s",
               OUStringToOString( aCommand.Name,
                                       RTL_TEXTENCODING_UTF8 ).getStr() );

    return aRet;
}



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
        
    }
}


//

//


void Content::addProperty( const com::sun::star::ucb::PropertyCommandArgument &aCmdArg,
                           const uno::Reference< ucb::XCommandEnvironment >& xEnv  )
throw( beans::PropertyExistException,
       beans::IllegalTypeException,
       lang::IllegalArgumentException,
       uno::RuntimeException )
{


    const beans::Property aProperty = aCmdArg.Property;
    const uno::Any aDefaultValue = aCmdArg.DefaultValue;

    
    if ( !aProperty.Name.getLength() )
        throw lang::IllegalArgumentException(
            OUString( "\"addProperty\" with empty Property.Name"),
            static_cast< ::cppu::OWeakObject * >( this ),
            -1 );

    
    if ( !UCBDeadPropertyValue::supportsType( aProperty.Type ) )
        throw beans::IllegalTypeException(
            OUString( "\"addProperty\" unsupported Property.Type"),
            static_cast< ::cppu::OWeakObject * >( this ) );

    
    if ( aDefaultValue.hasValue() && aDefaultValue.getValueType() != aProperty.Type )
        throw beans::IllegalTypeException(
            OUString( "\"addProperty\" DefaultValue does not match Property.Type"),
            static_cast< ::cppu::OWeakObject * >( this ) );

    
    
    
    

    
    
    rtl::OUString aSpecialName;
    bool bIsSpecial = DAVProperties::isUCBSpecialProperty( aProperty.Name, aSpecialName );

    
    if ( getPropertySetInfo( xEnv, sal_False /* don't cache data */ )
            ->hasPropertyByName( bIsSpecial ? aSpecialName : aProperty.Name ) )
    {
        
        throw beans::PropertyExistException();
    }

    
    
    

    ProppatchValue aValue( PROPSET, aProperty.Name, aDefaultValue );

    std::vector< ProppatchValue > aProppatchValues;
    aProppatchValues.push_back( aValue );

    try
    {
        
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

        
        beans::PropertySetInfoChangeEvent evt(
            static_cast< cppu::OWeakObject * >( this ),
            bIsSpecial ? aSpecialName : aProperty.Name,
            -1, 
            beans::PropertySetInfoChange::PROPERTY_INSERTED );
        notifyPropertySetInfoChange( evt );
    }
    catch ( DAVException const & e )
    {
        if ( e.getStatus() == SC_FORBIDDEN )
        {
            

            
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
                        
                        ContentImplHelper::addProperty( bIsSpecial ? aSpecialName : aProperty.Name,
                                                        aProperty.Attributes,
                                                        aDefaultValue );
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

void Content::removeProperty( const rtl::OUString& Name,
                              const uno::Reference< ucb::XCommandEnvironment >& xEnv )
throw( beans::UnknownPropertyException,
       beans::NotRemoveableException,
       uno::RuntimeException )
{
#if 0
    
    try
    {
        beans::Property aProp
        = getPropertySetInfo( xEnv, sal_False /* don't cache data */ )
          ->getPropertyByName( Name );

        if ( !( aProp.Attributes & beans::PropertyAttribute::REMOVABLE ) )
        {
            
            throw beans::NotRemoveableException();
        }
    }
    catch ( beans::UnknownPropertyException const & )
    {
        
        throw;
    }
#endif

    
    
    

    try
    {
        std::vector< ProppatchValue > aProppatchValues;
        ProppatchValue aValue( PROPREMOVE, Name, uno::Any() );
        aProppatchValues.push_back( aValue );

        
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

        
        beans::PropertySetInfoChangeEvent evt(
            static_cast< cppu::OWeakObject * >( this ),
            Name,
            -1, 
            beans::PropertySetInfoChange::PROPERTY_REMOVED );
        notifyPropertySetInfoChange( evt );
    }
    catch ( DAVException const & e )
    {
        if ( e.getStatus() == SC_FORBIDDEN )
        {
            

            
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

            }
        }
    }
}


void SAL_CALL Content::addProperty( const rtl::OUString& Name,
                                    sal_Int16 Attributes,
                                    const uno::Any& DefaultValue )
    throw( beans::PropertyExistException,
           beans::IllegalTypeException,
           lang::IllegalArgumentException,
           uno::RuntimeException )
{
    beans::Property aProperty;
    aProperty.Name = Name;
    aProperty.Type = DefaultValue.getValueType();
    aProperty.Attributes = Attributes;
    aProperty.Handle = -1;

    addProperty( ucb::PropertyCommandArgument( aProperty, DefaultValue ),
                 uno::Reference< ucb::XCommandEnvironment >());
}


void SAL_CALL Content::removeProperty( const rtl::OUString& Name )
    throw( beans::UnknownPropertyException,
           beans::NotRemoveableException,
           uno::RuntimeException )
{
    removeProperty( Name,
                    uno::Reference< ucb::XCommandEnvironment >() );
}


//

//



uno::Sequence< ucb::ContentInfo > SAL_CALL
Content::queryCreatableContentsInfo()
    throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    uno::Sequence< ucb::ContentInfo > aSeq( 2 );

    
    aSeq.getArray()[ 0 ].Type = OUString( WEBDAV_CONTENT_TYPE );
    aSeq.getArray()[ 0 ].Attributes
        = ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM
          | ucb::ContentInfoAttribute::KIND_DOCUMENT;

    beans::Property aProp;
    m_pProvider->getProperty(
        OUString( "Title" ), aProp );

    uno::Sequence< beans::Property > aDocProps( 1 );
    aDocProps.getArray()[ 0 ] = aProp;
    aSeq.getArray()[ 0 ].Properties = aDocProps;

    
    aSeq.getArray()[ 1 ].Type = OUString( WEBDAV_COLLECTION_TYPE );
    aSeq.getArray()[ 1 ].Attributes
        = ucb::ContentInfoAttribute::KIND_FOLDER;

    uno::Sequence< beans::Property > aFolderProps( 1 );
    aFolderProps.getArray()[ 0 ] = aProp;
    aSeq.getArray()[ 1 ].Properties = aFolderProps;
    return aSeq;
}



uno::Reference< ucb::XContent > SAL_CALL
Content::createNewContent( const ucb::ContentInfo& Info )
    throw( uno::RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !Info.Type.getLength() )
        return uno::Reference< ucb::XContent >();

    if ( ( Info.Type != WEBDAV_COLLECTION_TYPE )
         &&
         ( Info.Type != WEBDAV_CONTENT_TYPE ) )
        return uno::Reference< ucb::XContent >();

    OUString aURL = m_xIdentifier->getContentIdentifier();

    OSL_ENSURE( !aURL.isEmpty(),
                "WebdavContent::createNewContent - empty identifier!" );

    if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
        aURL += "/";

    sal_Bool isCollection;
    if ( Info.Type == WEBDAV_COLLECTION_TYPE )
    {
        aURL += "New_Collection";
        isCollection = sal_True;
    }
    else
    {
        aURL += "New_Content";
        isCollection = sal_False;
    }

    uno::Reference< ucb::XContentIdentifier > xId(
                    new ::ucbhelper::ContentIdentifier( m_xSMgr, aURL ) );

    
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



OUString Content::getParentURL()
{
    
    
    
    
    
    

    OUString aURL = m_xIdentifier->getContentIdentifier();

    sal_Int32 nPos = aURL.lastIndexOf( '/' );
    if ( nPos == ( aURL.getLength() - 1 ) )
    {
        
        nPos = aURL.lastIndexOf( '/', nPos );
    }

    sal_Int32 nPos1 = aURL.lastIndexOf( '/', nPos );
    if ( nPos1 != -1 )
        nPos1 = aURL.lastIndexOf( '/', nPos1 );

    if ( nPos1 == -1 )
        return OUString();

    return OUString( aURL.copy( 0, nPos + 1 ) );
}


//

//



uno::Reference< sdbc::XRow > Content::getPropertyValues(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Sequence< beans::Property >& rProperties,
    const ContentProperties& rData,
    const rtl::Reference< ::ucbhelper::ContentProviderImplHelper >& rProvider,
    const OUString& rContentId )
{
    

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow
        = new ::ucbhelper::PropertyValueSet( rxContext );

    sal_Int32 nCount = rProperties.getLength();
    if ( nCount )
    {
        uno::Reference< beans::XPropertySet > xAdditionalPropSet;
        sal_Bool bTriedToGetAdditionalPropSet = sal_False;

        const beans::Property* pProps = rProperties.getConstArray();
        for ( sal_Int32 n = 0; n < nCount; ++n )
        {
            const beans::Property& rProp = pProps[ n ];

            
            const uno::Any & rValue = rData.getValue( rProp.Name );
            if ( rValue.hasValue() )
            {
                xRow->appendObject( rProp, rValue );
            }
            else
            {
                
                if ( !bTriedToGetAdditionalPropSet && !xAdditionalPropSet.is() )
                {
                    xAdditionalPropSet
                        = uno::Reference< beans::XPropertySet >(
                            rProvider->getAdditionalPropertySet( rContentId,
                                                                 sal_False ),
                            uno::UNO_QUERY );
                    bTriedToGetAdditionalPropSet = sal_True;
                }

                if ( !xAdditionalPropSet.is() ||
                     !xRow->appendPropertySetValue(
                                            xAdditionalPropSet, rProp ) )
                {
                    
                    xRow->appendVoid( rProp );
                }
            }
        }
    }
    else
    {
        

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

        
        uno::Reference< beans::XPropertySet > xSet(
            rProvider->getAdditionalPropertySet( rContentId, sal_False ),
            uno::UNO_QUERY );
        xRow->appendPropertySet( xSet );
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
}


uno::Reference< sdbc::XRow > Content::getPropertyValues(
                const uno::Sequence< beans::Property >& rProperties,
                const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw ( uno::Exception )
{
    std::auto_ptr< ContentProperties > xProps;
    std::auto_ptr< ContentProperties > xCachedProps;
    std::auto_ptr< DAVResourceAccess > xResAccess;
    OUString aUnescapedTitle;
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

        
        if ( m_xCachedProps.get() )
        {
            xCachedProps.reset( new ContentProperties( *m_xCachedProps.get() ) );

            std::vector< OUString > aMissingProps;
            if ( xCachedProps->containsAllNames( rProperties, aMissingProps ) )
            {
                
                bHasAll = true;
            }

            
            xProps.reset( new ContentProperties( *xCachedProps.get() ) );
        }
    }

    if ( !m_bTransient && !bHasAll )
    {
        
        
        

        
        const ResourceType & rType = getResourceType( xEnv, xResAccess );

        bool bNetworkAccessAllowed = true;

        if ( DAV == rType )
        {
            
            
            if ( m_xCachedProps.get() )
            {
                xCachedProps.reset(
                    new ContentProperties( *m_xCachedProps.get() ) );

                std::vector< OUString > aMissingProps;
                if ( xCachedProps->containsAllNames(
                         rProperties, aMissingProps ) )
                {
                    
                    
                    bHasAll = true;
                }

                
                xProps.reset( new ContentProperties( *xCachedProps.get() ) );
            }

            if ( !bHasAll )
            {
                
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
                        bNetworkAccessAllowed
                            = shouldAccessNetworkAfterException( e );

                        if ( !bNetworkAccessAllowed )
                        {
                            cancelCommandExecution( e, xEnv );
                            
                        }
                    }
                }
            }
        }

        if ( bNetworkAccessAllowed )
        {
            
            std::vector< OUString > aMissingProps;
            if ( !( xProps.get()
                    && xProps->containsAllNames(
                        rProperties, aMissingProps ) )
                 || !m_bDidGetOrHead )
            {
                
                

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
                        
                        
                        
                        
                        
                        
                        bool bError = true;
                        DAVException aLastException = e;

                        
                        
                        
                        
                        
                        
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
                            if ( !(bNetworkAccessAllowed
                                    = shouldAccessNetworkAfterException( aLastException )) )
                            {
                                cancelCommandExecution( aLastException, xEnv );
                                
                            }
                        }
                    }
                }
            }
        }

        
        
        SerfUri aUri( xResAccess->getURL() );
        aUnescapedTitle = aUri.GetPathBaseNameUnescaped();

        if ( rType == UNKNOWN )
        {
            xProps.reset( new ContentProperties( aUnescapedTitle ) );
        }

        
        

        if ( rType == DAV )
        {
            
            
            xProps->addProperty(
                OUString( "Title" ),
                uno::makeAny( aUnescapedTitle ),
                true );
        }
        else
        {
            if ( !xProps.get() )
                xProps.reset( new ContentProperties( aUnescapedTitle, false ) );
            else
                xProps->addProperty(
                    OUString( "Title" ),
                    uno::makeAny( aUnescapedTitle ),
                    true );

            xProps->addProperty(
                OUString( "IsFolder" ),
                uno::makeAny( false ),
                true );
            xProps->addProperty(
                OUString( "IsDocument" ),
                uno::makeAny( true ),
                true );
            xProps->addProperty(
                OUString( "ContentType" ),
                uno::makeAny( OUString(WEBDAV_CONTENT_TYPE) ),
                true );
        }
    }
    else
    {
        
        
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
            
            xProps->addProperty(
                 OUString( "BaseURI" ),
                 uno::makeAny( getBaseURI( xResAccess ) ),
                 true );
        }
        else if ( rName == "CreatableContentsInfo" )
        {
            
            sal_Bool bFolder = sal_False;
            xProps->getValue(
                OUString( "IsFolder" ) )
                    >>= bFolder;
            xProps->addProperty(
                OUString( "CreatableContentsInfo" ),
                uno::makeAny( bFolder
                                  ? queryCreatableContentsInfo()
                                  : uno::Sequence< ucb::ContentInfo >() ),
                true );
        }
    }

    uno::Reference< sdbc::XRow > xResultRow
        = getPropertyValues( comphelper::getComponentContext(xSMgr),
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
    
    aEvent.PropertyHandle = -1;
    
    

    std::vector< ProppatchValue > aProppatchValues;
    std::vector< sal_Int32 > aProppatchPropsPositions;

    uno::Reference< ucb::XPersistentPropertySet > xAdditionalPropSet;
    sal_Bool bTriedToGetAdditionalPropSet = sal_False;

    sal_Bool bExchange = sal_False;
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
            
            aRet[ n ] <<= lang::IllegalAccessException(
                            OUString( "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
            continue;
        }

        
        
        

        if ( rName == "ContentType" )
        {
            
            aRet[ n ] <<= lang::IllegalAccessException(
                OUString( "Property is read-only!" ),
                static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName == "IsDocument" )
        {
            
            aRet[ n ] <<= lang::IllegalAccessException(
                OUString( "Property is read-only!" ),
                static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName == "IsFolder" )
        {
            
            aRet[ n ] <<= lang::IllegalAccessException(
                            OUString( "Property is read-only!" ),
                            static_cast< cppu::OWeakObject * >( this ) );
        }
        else if ( rName == "Title" )
        {
            OUString aNewValue;
            if ( rValue.Value >>= aNewValue )
            {
                
                if ( aNewValue.getLength() > 0 )
                {
                    try
                    {
                        SerfUri aURI( xIdentifier->getContentIdentifier() );
                        aOldTitle = aURI.GetPathBaseNameUnescaped();

                        if ( aNewValue != aOldTitle )
                        {
                            
                            if ( !bTransient )
                                bExchange = sal_True;

                            
                            aNewTitle = aNewValue;

                            
                            
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
            
            
            

            rtl::OUString aSpecialName;
            bool bIsSpecial = DAVProperties::isUCBSpecialProperty( rName, aSpecialName );

            if ( !xInfo.is() )
                xInfo = getPropertySetInfo( xEnv,
                                            sal_False /* don't cache data */ );

            if ( !xInfo->hasPropertyByName( bIsSpecial ? aSpecialName : rName ) )
            {
                
                
                
                aRet[ n ] <<= beans::UnknownPropertyException(
                                OUString( "Property is unknown!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
                continue;
            }

            if ( rName == "Size" )
            {
                
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName == "DateCreated" )
            {
                
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName == "DateModified" )
            {
                
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else if ( rName == "MediaType" )
            {
                
                
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            if ( rName == "CreatableContentsInfo" )
            {
                
                aRet[ n ] <<= lang::IllegalAccessException(
                                OUString( "Property is read-only!" ),
                                static_cast< cppu::OWeakObject * >( this ) );
            }
            else
            {
                if ( getResourceType( xEnv, xResAccess ) == DAV )
                {
                    
                    ProppatchValue aValue( PROPSET, rName, rValue.Value );
                    aProppatchValues.push_back( aValue );

                    
                    
                    aProppatchPropsPositions.push_back( n );
                }
                else
                {
                    
                    if ( !bTriedToGetAdditionalPropSet &&
                         !xAdditionalPropSet.is() )
                    {
                        xAdditionalPropSet
                            = getAdditionalPropertySet( sal_False );
                        bTriedToGetAdditionalPropSet = sal_True;
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
    } 

    if ( !bTransient && (!aProppatchValues.empty()) )
    {
        try
        {
            
            xResAccess->PROPPATCH( aProppatchValues, xEnv );

            std::vector< ProppatchValue >::const_iterator it
                = aProppatchValues.begin();
            std::vector< ProppatchValue >::const_iterator end
                = aProppatchValues.end();

            while ( it != end )
            {
                aEvent.PropertyName = (*it).name;
                aEvent.OldValue     = uno::Any(); 
                aEvent.NewValue     = (*it).value;

                aChanges.getArray()[ nChanged ] = aEvent;
                nChanged++;

                ++it;
            }
        }
        catch ( DAVException const & e )
        {



#if 1
            cancelCommandExecution( e, xEnv );
            
#else
            

            std::vector< sal_Int32 >::const_iterator it
                = aProppatchPropsPositions.begin();
            std::vector< sal_Int32 >::const_iterator end
                = aProppatchPropsPositions.end();

            while ( it != end )
            {
                
                aRet[ (*it) ] <<= MapDAVException( e, sal_True );
                ++it;
            }
#endif
        }
    }

    if ( bExchange )
    {
        

        OUString aNewURL = getParentURL();
        if ( aNewURL.lastIndexOf( '/' ) != ( aNewURL.getLength() - 1 ) )
            aNewURL += "/";

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
            
            
            

            
            

            
            

            if ( exchangeIdentity( xNewId ) )
            {
                xResAccess->setURL( aNewURL );






            }
            else
            {
                
                aNewTitle = OUString();

                
                aRet[ nTitlePos ] <<= uno::Exception(
                    OUString( "Exchange failed!" ),
                    static_cast< cppu::OWeakObject * >( this ) );
            }
        }
        catch ( DAVException const & e )
        {
            
            aNewTitle = OUString();

            
            aRet[ nTitlePos ] <<= MapDAVException( e, sal_True );
        }
    }

    if ( aNewTitle.getLength() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        aEvent.PropertyName = "Title";
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
            

            uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet( comphelper::getComponentContext(m_xSMgr), this, rArg, xEnv );
            aRet <<= xSet;
        }
        else
        {
            

            OUStringBuffer aMsg;
            aMsg.appendAscii( "Non-folder resource cannot be "
                              "opened as folder! Wrong Open Mode!" );

            ucbhelper::cancelCommandExecution(
                uno::makeAny(
                    lang::IllegalArgumentException(
                        aMsg.makeStringAndClear(),
                        static_cast< cppu::OWeakObject * >( this ),
                        -1 ) ),
                xEnv );
            
        }
    }

    if ( rArg.Sink.is() )
    {
        

        if ( ( rArg.Mode == ucb::OpenMode::DOCUMENT_SHARE_DENY_NONE ) ||
             ( rArg.Mode == ucb::OpenMode::DOCUMENT_SHARE_DENY_WRITE ) )
        {
            
            ucbhelper::cancelCommandExecution(
                uno::makeAny(
                    ucb::UnsupportedOpenModeException(
                            OUString(),
                            static_cast< cppu::OWeakObject * >( this ),
                            sal_Int16( rArg.Mode ) ) ),
                xEnv );
            
        }

        OUString aURL = m_xIdentifier->getContentIdentifier();
        uno::Reference< io::XOutputStream > xOut
            = uno::Reference< io::XOutputStream >( rArg.Sink, uno::UNO_QUERY );
        if ( xOut.is() )
        {
            
            try
            {
                std::auto_ptr< DAVResourceAccess > xResAccess;

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
                
            }
        }
        else
        {
            uno::Reference< io::XActiveDataSink > xDataSink
                = uno::Reference< io::XActiveDataSink >( rArg.Sink,
                                                         uno::UNO_QUERY );
            if ( xDataSink.is() )
            {
                
                try
                {
                    std::auto_ptr< DAVResourceAccess > xResAccess;
                    {
                        osl::MutexGuard aGuard( m_aMutex );

                        xResAccess.reset(
                            new DAVResourceAccess( *m_xResAccess.get() ) );
                    }

                    
                    DAVResource aResource;
                    std::vector< OUString > aHeaders;

                    uno::Reference< io::XInputStream > xIn
                        = xResAccess->GET( aHeaders, aResource, xEnv );
                    m_bDidGetOrHead = true;

                    {
                        osl::MutexGuard aGuard( m_aMutex );

                        
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
                
            }
        }
    }

    return aRet;
}


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
            
        }
    }
}


void Content::queryChildren( ContentRefList& rChildren )
{
    
    
    

    ::ucbhelper::ContentRefList aAllContents;
    m_xProvider->queryExistingContents( aAllContents );

    OUString aURL = m_xIdentifier->getContentIdentifier();
    sal_Int32 nURLPos = aURL.lastIndexOf( '/' );

    if ( nURLPos != ( aURL.getLength() - 1 ) )
    {
        
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

        
        if ( ( aChildURL.getLength() > nLen ) &&
             ( aChildURL.compareTo( aURL, nLen ) == 0 ) )
        {
            sal_Int32 nPos = nLen;
            nPos = aChildURL.indexOf( '/', nPos );

            if ( ( nPos == -1 ) ||
                 ( nPos == ( aChildURL.getLength() - 1 ) ) )
            {
                
                rChildren.push_back(
                    ::http_dav_ucp::Content::ContentRef(
                        static_cast< ::http_dav_ucp::Content * >(
                            xChild.get() ) ) );
            }
        }
        ++it;
    }
}


void Content::insert(
        const uno::Reference< io::XInputStream > & xInputStream,
        sal_Bool bReplaceExisting,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception )
{
    sal_Bool bTransient, bCollection;
    OUString aEscapedTitle;
    std::auto_ptr< DAVResourceAccess > xResAccess;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        bTransient    = m_bTransient;
        bCollection   = m_bCollection;
        aEscapedTitle = m_aEscapedTitle;
        xResAccess.reset( new DAVResourceAccess( *m_xResAccess.get() ) );
    }

    

    if ( aEscapedTitle.isEmpty() )
    {
        OSL_ENSURE( sal_False, "Content::insert - Title missing!" );

        uno::Sequence< OUString > aProps( 1 );
        aProps[ 0 ] = "Title";
        ucbhelper::cancelCommandExecution(
            uno::makeAny( ucb::MissingPropertiesException(
                                OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                aProps ) ),
            Environment );
        
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

        
        if ( !bTransient || ( bTransient && !bCollection  ) )
        {
#undef ERROR
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
                        
                        throw aEx;


                    case ucbhelper::CONTINUATION_APPROVE:
                        
                        bReplaceExisting = sal_True;
                        break;

                    case ucbhelper::CONTINUATION_DISAPPROVE:
                        
                        throw ucb::CommandFailedException(
                                    OUString(),
                                    uno::Reference< uno::XInterface >(),
                                    aExAsAny );


                    default:
                        OSL_ENSURE( sal_False,
                                    "Content::insert - "
                                    "Unknown interaction selection!" );
                        throw ucb::CommandFailedException(
                                    OUString( "Unknown interaction selection!" ),
                                    uno::Reference< uno::XInterface >(),
                                    aExAsAny );

                }
            }
            else
            {
                
                throw aEx;
            }
        }
    }

    if ( bTransient )
    {
        
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
                    
                    
                    

                    if ( bReplaceExisting )
                    {
                        
                        try
                        {
                            xResAccess->DESTROY( Environment );
                        }
                        catch ( DAVException const & e )
                        {
                            cancelCommandExecution( e, Environment, sal_True );
                            
                        }

                        
                        insert( xInputStream, bReplaceExisting, Environment );

                        {
                            osl::Guard< osl::Mutex > aGuard( m_aMutex );
                            m_xResAccess.reset(
                                new DAVResourceAccess( *xResAccess.get() ) );
                        }

                        
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
                        
                    }
                }
            }

            cancelCommandExecution( except, Environment, sal_True );
            
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
                        OUString(),
                        static_cast< cppu::OWeakObject * >( this ) ) ),
                Environment );
            
        }

        try
        {
            xResAccess->PUT( xInputStream, Environment );
        }
        catch ( DAVException const & e )
        {
            cancelCommandExecution( e, Environment, sal_True );
            
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

    OUString aTargetURI;
    try
    {
        SerfUri sourceURI( rArgs.SourceURL );
        SerfUri targetURI( xIdentifier->getContentIdentifier() );
        aTargetURI = targetURI.GetPathBaseNameUnescaped();

        
        //
        const OUString aScheme = sourceURI.GetScheme().toAsciiLowerCase();
        if ( aScheme == WEBDAV_URL_SCHEME )
        {
            sourceURI.SetScheme(
                OUString( HTTP_URL_SCHEME ) );
        }
        else if ( aScheme == DAV_URL_SCHEME )
        {
            sourceURI.SetScheme(
                OUString( HTTP_URL_SCHEME ) );
        }
        else if ( aScheme == DAVS_URL_SCHEME )
        {
            sourceURI.SetScheme(
                OUString( HTTPS_URL_SCHEME ) );
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
                
            }
        }

        if ( targetURI.GetScheme().toAsciiLowerCase() == WEBDAV_URL_SCHEME )
            targetURI.SetScheme(
                OUString( HTTP_URL_SCHEME ) );
        else if ( targetURI.GetScheme().toAsciiLowerCase() == DAV_URL_SCHEME )
            targetURI.SetScheme(
                OUString( HTTP_URL_SCHEME ) );

        
        
        

        
        //
        if ( sourceURI.GetHost().getLength() &&
             ( sourceURI.GetHost() != targetURI.GetHost() ) )
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( ucb::InteractiveBadTransferURLException(
                                OUString( "Different hosts!" ),
                                static_cast< cppu::OWeakObject * >( this ) ) ),
                Environment );
            
        }

        OUString aTitle = rArgs.NewTitle;

        if ( aTitle.isEmpty() )
            aTitle = sourceURI.GetPathBaseNameUnescaped();

        if ( aTitle == "/" )
        {
            
            aTitle = OUString();
        }

        targetURI.AppendPath( aTitle );

        OUString aTargetURL = xIdentifier->getContentIdentifier();
        if ( ( aTargetURL.lastIndexOf( '/' ) + 1 )
                != aTargetURL.getLength() )
            aTargetURL += "/";

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

            
            
            rtl::Reference< Content > xSource
                = static_cast< Content * >(
                    xProvider->queryContent( xId ).get() );

            
            
            
            
            
            

            aSourceAccess.MOVE( sourceURI.GetPath(),
                                targetURI.GetURI(),
                                rArgs.NameClash
                                    == ucb::NameClash::OVERWRITE,
                                Environment );

            if ( xSource.is() )
            {
                
                xSource->destroy( sal_True );
            }






        }
        else
        {
            
            
            
            
            
            

            aSourceAccess.COPY( sourceURI.GetPath(),
                                targetURI.GetURI(),
                                rArgs.NameClash
                                    == ucb::NameClash::OVERWRITE,
                                Environment );






        }

        
        
        rtl::Reference< Content > xTarget
            = static_cast< Content * >(
                    xProvider->queryContent( xTargetId ).get() );

        
        xTarget->inserted();
    }
    catch ( ucb::IllegalIdentifierException const & )
    {
        
    }
    catch ( DAVException const & e )
    {
        
        
        
        
        

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
                    
                }

                case ucb::NameClash::OVERWRITE:
                    break;

                case ucb::NameClash::KEEP: 
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
                    
                }
            }
        }

        cancelCommandExecution( e, Environment, sal_True );
        
    }

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
    }
}


void Content::destroy( sal_Bool bDeletePhysical )
    throw( uno::Exception )
{
    
    OUString aURL = m_xIdentifier->getContentIdentifier();

    uno::Reference< ucb::XContent > xThis = this;

    deleted();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    

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
        aOwnerAny <<= OUString( "http:

        ucb::Lock aLock(
            ucb::LockScope_EXCLUSIVE,
            ucb::LockType_WRITE,
            ucb::LockDepth_ZERO,
            aOwnerAny,
            180, 
            
            uno::Sequence< OUString >() );

        xResAccess->LOCK( aLock, Environment );

        {
            osl::Guard< osl::Mutex > aGuard( m_aMutex );
            m_xResAccess.reset( new DAVResourceAccess( *xResAccess.get() ) );
        }
    }
    catch ( DAVException const & e )
    {
        cancelCommandExecution( e, Environment, sal_False );
        
    }
}


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
        
    }
}


sal_Bool Content::exchangeIdentity(
    const uno::Reference< ucb::XContentIdentifier >& xNewId )
{
    if ( !xNewId.is() )
        return sal_False;

    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    uno::Reference< ucb::XContent > xThis = this;

    
    if ( m_bTransient )
    {
        OSL_ENSURE( sal_False, "Content::exchangeIdentity - Not persistent!" );
        return sal_False;
    }

    

    

    {
        OUString aOldURL = m_xIdentifier->getContentIdentifier();

        aGuard.clear();
        if ( exchange( xNewId ) )
        {
            

            ContentRefList aChildren;
            queryChildren( aChildren );

            ContentRefList::const_iterator it  = aChildren.begin();
            ContentRefList::const_iterator end = aChildren.end();

            while ( it != end )
            {
                ContentRef xChild = (*it);

                
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

    return sal_False;
}


uno::Any Content::MapDAVException( const DAVException & e, sal_Bool bWrite )
{
    
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
                OUString("Uri"), -1,
                uno::makeAny(aURL),
                beans::PropertyState_DIRECT_VALUE);

            aException <<=
                ucb::InteractiveAugmentedIOException(
                    OUString( "Not found!" ),
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









    case DAVException::DAV_HTTP_CONNECT:
        aException <<=
            ucb::InteractiveNetworkConnectException(
                OUString(),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                e.getData() );
        break;













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
                OUString( "Locked!" ),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL,
                sal_False ); 
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
                OUString( "Locked (self)!" ),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL,
                sal_True ); 
        break;

    case DAVException::DAV_NOT_LOCKED:
        aException <<=
            ucb::InteractiveLockingNotLockedException(
                OUString( "Not locked!" ),
                static_cast< cppu::OWeakObject * >( this ),
                task::InteractionClassification_ERROR,
                aURL );
        break;

    case DAVException::DAV_LOCK_EXPIRED:
        aException <<=
            ucb::InteractiveLockingLockExpiredException(
                OUString( "Lock expired!" ),
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
                sal_Bool bWrite /* = sal_False */ )
    throw ( uno::Exception )
{
    ucbhelper::cancelCommandExecution( MapDAVException( e, bWrite ), xEnv );
    
}


const OUString
Content::getBaseURI( const std::auto_ptr< DAVResourceAccess > & rResAccess )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    
    if ( m_xCachedProps.get() )
    {
        OUString aLocation;
        m_xCachedProps->getValue( OUString(
                                        "Content-Location" ) ) >>= aLocation;
        if ( aLocation.getLength() )
        {
            try
            {
                
                
                
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

        const OUString & rURL = rResAccess->getURL();
        const OUString aScheme(
            rURL.copy( 0, rURL.indexOf( ':' ) ).toAsciiLowerCase() );

        try
        {
            
            
            
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
                
                
                eResourceType = NON_DAV;
            }
            
            if ( e.getError() == DAVException::DAV_HTTP_NOAUTH )
            {
                cancelCommandExecution( e, uno::Reference< ucb::XCommandEnvironment >() );
            }
        }
        m_eResourceType = eResourceType;
    }
    return m_eResourceType;
}


const Content::ResourceType & Content::getResourceType(
                    const uno::Reference< ucb::XCommandEnvironment >& xEnv )
    throw ( uno::Exception )
{
    return getResourceType( xEnv, m_xResAccess );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
