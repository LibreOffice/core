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
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <cppuhelper/weak.hxx>

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/ucb/ContentCreationError.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/ucb/ContentAction.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/GlobalTransferCommandArgument.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <com/sun/star/ucb/XContentEventListener.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/activedatasink.hxx>
#include <ucbhelper/activedatastreamer.hxx>
#include <ucbhelper/interactionrequest.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>

using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

namespace ucbhelper
{

class EmptyInputStream : public ::cppu::WeakImplHelper1< XInputStream >
{
public:
    virtual sal_Int32 SAL_CALL readBytes(
        Sequence< sal_Int8 > & data, sal_Int32 nBytesToRead )
        throw (IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes(
        Sequence< sal_Int8 > & data, sal_Int32 nMaxBytesToRead )
        throw (IOException, RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw (IOException, RuntimeException);
    virtual sal_Int32 SAL_CALL available()
        throw (IOException, RuntimeException);
    virtual void SAL_CALL closeInput()
        throw (IOException, RuntimeException);
};

sal_Int32 EmptyInputStream::readBytes(
    Sequence< sal_Int8 > & data, sal_Int32 )
    throw (IOException, RuntimeException)
{
    data.realloc( 0 );
    return 0;
}

sal_Int32 EmptyInputStream::readSomeBytes(
    Sequence< sal_Int8 > & data, sal_Int32 )
    throw (IOException, RuntimeException)
{
    data.realloc( 0 );
    return 0;
}

void EmptyInputStream::skipBytes( sal_Int32 )
    throw (IOException, RuntimeException)
{
}

sal_Int32 EmptyInputStream::available()
    throw (IOException, RuntimeException)
{
    return 0;
}

void EmptyInputStream::closeInput()
    throw (IOException, RuntimeException)
{
}


//=========================================================================
//=========================================================================
//
// class ContentEventListener_Impl.
//
//=========================================================================
//=========================================================================

class ContentEventListener_Impl : public cppu::OWeakObject,
                                      public XContentEventListener
{
    Content_Impl& m_rContent;

public:
    ContentEventListener_Impl( Content_Impl& rContent )
    : m_rContent( rContent ) {}

    // XInterface
    XINTERFACE_DECL()

    // XContentEventListener
    virtual void SAL_CALL contentEvent( const ContentEvent& evt )
        throw( RuntimeException );

    // XEventListener ( base of XContentEventListener )
    virtual void SAL_CALL disposing( const EventObject& Source )
        throw( RuntimeException );
};

//=========================================================================
//=========================================================================
//
// class Content_Impl.
//
//=========================================================================
//=========================================================================

class Content_Impl : public salhelper::SimpleReferenceObject
{
friend class ContentEventListener_Impl;

    mutable rtl::OUString               m_aURL;
    Reference< XMultiServiceFactory >   m_xSMgr;
    Reference< XContent >               m_xContent;
    Reference< XCommandProcessor >          m_xCommandProcessor;
    Reference< XCommandEnvironment >    m_xEnv;
    Reference< XContentEventListener >  m_xContentEventListener;
    mutable osl::Mutex                  m_aMutex;
    sal_Int32                           m_nCommandId;

private:
    void reinit( const Reference< XContent >& xContent );
    void disposing(const EventObject& Source);

public:
    Content_Impl() : m_nCommandId( 0 ) {};
    Content_Impl( const Reference< XMultiServiceFactory >& rSMgr,
                  const Reference< XContent >& rContent,
                  const Reference< XCommandEnvironment >& rEnv );

    virtual ~Content_Impl();

    const rtl::OUString&           getURL() const;
    Reference< XContent >          getContent();
    Reference< XCommandProcessor > getCommandProcessor();
    sal_Int32 getCommandId();
    Reference< XMultiServiceFactory > getServiceManager() { return m_xSMgr; }

    Any  executeCommand( const Command& rCommand );
    void abortCommand();

    inline const Reference< XCommandEnvironment >& getEnvironment() const;
    inline void setEnvironment(
                        const Reference< XCommandEnvironment >& xNewEnv );

    void inserted();
};

//=========================================================================
// Helpers.
//=========================================================================

static void ensureContentProviderForURL( const ContentBroker & rBroker,
                                         const rtl::OUString & rURL )
    throw ( ContentCreationException, RuntimeException )
{
    Reference< XContentProviderManager > xMgr
        = rBroker.getContentProviderManagerInterface();
    if ( !xMgr.is() )
    {
        throw RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "UCB does not implement mandatory interface "
                "XContentProviderManager!" )),
            Reference< XInterface >() );
    }
    else
    {
        Reference< XContentProvider > xProv
            = xMgr->queryContentProvider( rURL );
        if ( !xProv.is() )
        {
            throw ContentCreationException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "No Content Provider available for given URL!" )),
                Reference< XInterface >(),
                ContentCreationError_NO_CONTENT_PROVIDER );
        }
    }
}

//=========================================================================
static ContentBroker* getContentBroker( bool bThrow )
    throw ( ContentCreationException, RuntimeException )
{
    ContentBroker* pBroker = ContentBroker::get();

    if ( !pBroker )
    {
        if ( bThrow )
            throw RuntimeException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("No Content Broker!")),
                    Reference< XInterface >() );
    }
    else
    {
#if OSL_DEBUG_LEVEL > 1
        Reference< XContentProviderManager > xMgr
            = pBroker->getContentProviderManagerInterface();
        if ( !xMgr.is() )
        {
            if ( bThrow )
                throw RuntimeException(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "UCB does not implement mandatory interface "
                            "XContentProviderManager!" )),
                        Reference< XInterface >() );
        }
        else
        {
            OSL_ENSURE( xMgr->queryContentProviders().getLength(),
                        "Content Broker not configured (no providers)!" );
        }
#endif
    }

    return pBroker;
}

//=========================================================================
static Reference< XContentIdentifier > getContentIdentifier(
                                    const ContentBroker & rBroker,
                                    const rtl::OUString & rURL,
                                    bool bThrow )
    throw ( ContentCreationException, RuntimeException )
{
    Reference< XContentIdentifierFactory > xIdFac
                        = rBroker.getContentIdentifierFactoryInterface();
    if ( xIdFac.is() )
    {
        Reference< XContentIdentifier > xId
            = xIdFac->createContentIdentifier( rURL );

        if ( xId.is() )
            return xId;

        if ( bThrow )
        {
            ensureContentProviderForURL( rBroker, rURL );

            throw ContentCreationException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "Unable to create Content Identifier!" )),
                Reference< XInterface >(),
                ContentCreationError_IDENTIFIER_CREATION_FAILED );
        }
    }
    else
    {
        if ( bThrow )
            throw RuntimeException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "UCB does not implement mandatory interface "
                        "XContentIdentifierFactory!" )),
                    Reference< XInterface >() );
    }

    return Reference< XContentIdentifier >();
}

//=========================================================================
static Reference< XContent > getContent(
                                    const ContentBroker & rBroker,
                                    const Reference< XContentIdentifier > & xId,
                                    bool bThrow )
    throw ( ContentCreationException, RuntimeException )
{
    Reference< XContentProvider > xProvider
        = rBroker.getContentProviderInterface();
    if ( xProvider.is() )
    {
        Reference< XContent > xContent;
        rtl::OUString msg;
        try
        {
            xContent = xProvider->queryContent( xId );
        }
        catch ( IllegalIdentifierException const & e )
        {
            msg = e.Message;
            // handled below.
        }

        if ( xContent.is() )
            return xContent;

        if ( bThrow )
        {
            ensureContentProviderForURL( rBroker, xId->getContentIdentifier() );

            throw ContentCreationException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Unable to create Content! " )) + msg,
                    Reference< XInterface >(),
                    ContentCreationError_CONTENT_CREATION_FAILED );
        }
    }
    else
    {
        if ( bThrow )
            throw RuntimeException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "UCB does not implement mandatory interface "
                        "XContentProvider!" )),
                    Reference< XInterface >() );
    }

    return Reference< XContent >();
}

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

Content::Content()
: m_xImpl( new Content_Impl )
{
}

//=========================================================================
Content::Content( const rtl::OUString& rURL,
                  const Reference< XCommandEnvironment >& rEnv )
    throw ( ContentCreationException, RuntimeException )
{
    ContentBroker* pBroker = getContentBroker( true );

    Reference< XContentIdentifier > xId
        = getContentIdentifier( *pBroker, rURL, true );

    Reference< XContent > xContent = getContent( *pBroker, xId, true );

    m_xImpl = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );
}

//=========================================================================
Content::Content( const Reference< XContentIdentifier >& rId,
                  const Reference< XCommandEnvironment >& rEnv )
    throw ( ContentCreationException, RuntimeException )
{
    ContentBroker* pBroker = getContentBroker( true );

    Reference< XContent > xContent = getContent( *pBroker, rId, true );

    m_xImpl = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );
}

//=========================================================================
Content::Content( const Reference< XContent >& rContent,
                  const Reference< XCommandEnvironment >& rEnv )
    throw ( ContentCreationException, RuntimeException )
{
    ContentBroker* pBroker = getContentBroker( true );

    m_xImpl = new Content_Impl( pBroker->getServiceManager(), rContent, rEnv );
}

//=========================================================================
Content::Content( const Content& rOther )
{
    m_xImpl = rOther.m_xImpl;
}

//=========================================================================
// static
sal_Bool Content::create( const rtl::OUString& rURL,
                          const Reference< XCommandEnvironment >& rEnv,
                          Content& rContent )
{
    ContentBroker* pBroker = getContentBroker( false );
    if ( !pBroker )
        return sal_False;

    Reference< XContentIdentifier > xId
        = getContentIdentifier( *pBroker, rURL, false );
    if ( !xId.is() )
        return sal_False;

    Reference< XContent > xContent = getContent( *pBroker, xId, false );
    if ( !xContent.is() )
        return sal_False;

    rContent.m_xImpl
        = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );

    return sal_True;
}

//=========================================================================
// static
sal_Bool Content::create( const Reference< XContentIdentifier >& rId,
                          const Reference< XCommandEnvironment >& rEnv,
                          Content& rContent )
{
    ContentBroker* pBroker = getContentBroker( false );
    if ( !pBroker )
        return sal_False;

    Reference< XContent > xContent = getContent( *pBroker, rId, false );
    if ( !xContent.is() )
        return sal_False;

    rContent.m_xImpl
        = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );

    return sal_True;
}

//=========================================================================
// static
sal_Bool Content::create( const Reference< XContent >& xContent,
                          const Reference< XCommandEnvironment >& rEnv,
                          Content& rContent )
{
    ContentBroker* pBroker = getContentBroker( false );
    if ( !pBroker )
        return sal_False;

    rContent.m_xImpl
        = new Content_Impl( pBroker->getServiceManager(), xContent, rEnv );

    return sal_True;
}

//=========================================================================
Content::~Content()
{
}

//=========================================================================
Content& Content::operator=( const Content& rOther )
{
    m_xImpl = rOther.m_xImpl;
    return *this;
}

//=========================================================================
Reference< XContent > Content::get() const
{
    return m_xImpl->getContent();
}

//=========================================================================
const rtl::OUString& Content::getURL() const
{
    return m_xImpl->getURL();
}

//=========================================================================
const Reference< XCommandEnvironment >& Content::getCommandEnvironment() const
{
    return m_xImpl->getEnvironment();
}

//=========================================================================
void Content::setCommandEnvironment(
                        const Reference< XCommandEnvironment >& xNewEnv )
{
    m_xImpl->setEnvironment( xNewEnv );
}

//=========================================================================
Reference< XCommandInfo > Content::getCommands()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getCommandInfo"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument = Any();

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XCommandInfo > xInfo;
    aResult >>= xInfo;
    return xInfo;
}

//=========================================================================
Reference< XPropertySetInfo > Content::getProperties()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getPropertySetInfo"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument = Any();

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XPropertySetInfo > xInfo;
    aResult >>= xInfo;
    return xInfo;
}

//=========================================================================
Any Content::getPropertyValue( const rtl::OUString& rPropertyName )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< rtl::OUString > aNames( 1 );
    aNames.getArray()[ 0 ] = rPropertyName;

    Sequence< Any > aRet = getPropertyValues( aNames );
    return aRet.getConstArray()[ 0 ];
}

//=========================================================================
Any Content::getPropertyValue( sal_Int32 nPropertyHandle )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< sal_Int32 > aHandles( 1 );
    aHandles.getArray()[ 0 ] = nPropertyHandle;

    Sequence< Any > aRet = getPropertyValues( aHandles );
    return aRet.getConstArray()[ 0 ];
}

//=========================================================================
Any Content::setPropertyValue( const rtl::OUString& rName,
                                const Any& rValue )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< rtl::OUString > aNames( 1 );
    aNames.getArray()[ 0 ] = rName;

    Sequence< Any > aValues( 1 );
    aValues.getArray()[ 0 ] = rValue;

    Sequence< Any > aErrors = setPropertyValues( aNames, aValues );
    return aErrors.getConstArray()[ 0 ];
}

//=========================================================================
Any Content::setPropertyValue( const sal_Int32 nPropertyHandle,
                                const Any& rValue )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< sal_Int32 > aHandles( 1 );
    aHandles.getArray()[ 0 ] = nPropertyHandle;

    Sequence< Any > aValues( 1 );
    aValues.getArray()[ 0 ] = rValue;

    Sequence< Any > aErrors = setPropertyValues( aHandles, aValues );
    return aErrors.getConstArray()[ 0 ];
}

//=========================================================================
Sequence< Any > Content::getPropertyValues(
                            const Sequence< rtl::OUString >& rPropertyNames )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XRow > xRow = getPropertyValuesInterface( rPropertyNames );

    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Any > aValues( nCount );

    if ( xRow.is() )
    {
        Any* pValues = aValues.getArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
            pValues[ n ] = xRow->getObject( n + 1, Reference< XNameAccess >() );
    }

    return aValues;
}

//=========================================================================
Sequence< Any > Content::getPropertyValues(
                            const Sequence< sal_Int32 >& nPropertyHandles )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XRow > xRow = getPropertyValuesInterface( nPropertyHandles );

    sal_Int32 nCount = nPropertyHandles.getLength();
    Sequence< Any > aValues( nCount );

    if ( xRow.is() )
    {
        Any* pValues = aValues.getArray();

        for ( sal_Int32 n = 0; n < nCount; ++n )
            pValues[ n ] = xRow->getObject( n + 1, Reference< XNameAccess >() );
    }

    return aValues;
}

//=========================================================================
Reference< XRow > Content::getPropertyValuesInterface(
                            const Sequence< rtl::OUString >& rPropertyNames )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();

    const rtl::OUString* pNames  = rPropertyNames.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        Property& rProp = pProps[ n ];

        rProp.Name       = pNames[ n ];
        rProp.Handle     = -1; // n/a
//        rProp.Type       =
//        rProp.Attributes = ;
    }

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getPropertyValues"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XRow > xRow;
    aResult >>= xRow;
    return xRow;
}

//=========================================================================
Reference< XRow > Content::getPropertyValuesInterface(
                            const Sequence< sal_Int32 >& nPropertyHandles )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = nPropertyHandles.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();

    const sal_Int32* pHandles  = nPropertyHandles.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        Property& rProp = pProps[ n ];

        rProp.Name       = rtl::OUString(); // n/a
        rProp.Handle     = pHandles[ n ];
//        rProp.Type       =
//        rProp.Attributes = ;
    }

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getPropertyValues"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XRow > xRow;
    aResult >>= xRow;
    return xRow;
}

//=========================================================================
Sequence< Any > Content::setPropertyValues(
                            const Sequence< rtl::OUString >& rPropertyNames,
                                const Sequence< Any >& rValues )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rPropertyNames.getLength() != rValues.getLength() )
    {
        ucbhelper::cancelCommandExecution(
            makeAny( IllegalArgumentException(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "Length of property names sequence and value "
                            "sequence are unequal!" )),
                        get(),
                        -1 ) ),
            m_xImpl->getEnvironment() );
        // Unreachable
    }

    sal_Int32 nCount = rValues.getLength();
    Sequence< PropertyValue > aProps( nCount );
    PropertyValue* pProps = aProps.getArray();

    const rtl::OUString* pNames  = rPropertyNames.getConstArray();
    const Any* pValues = rValues.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        PropertyValue& rProp = pProps[ n ];

        rProp.Name   = pNames[ n ];
        rProp.Handle = -1; // n/a
        rProp.Value  = pValues[ n ];
//        rProp.State  = ;
    }

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("setPropertyValues"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Sequence< Any > aErrors;
    aResult >>= aErrors;
    return aErrors;
}

//=========================================================================
Sequence< Any > Content::setPropertyValues(
                            const Sequence< sal_Int32 >& nPropertyHandles,
                                const Sequence< Any >& rValues )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( nPropertyHandles.getLength() != rValues.getLength() )
    {
        ucbhelper::cancelCommandExecution(
            makeAny( IllegalArgumentException(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "Length of property handles sequence and value "
                            "sequence are unequal!" )),
                        get(),
                        -1 ) ),
            m_xImpl->getEnvironment() );
        // Unreachable
    }

    sal_Int32 nCount = rValues.getLength();
    Sequence< PropertyValue > aProps( nCount );
    PropertyValue* pProps = aProps.getArray();

    const sal_Int32* pHandles = nPropertyHandles.getConstArray();
    const Any*       pValues  = rValues.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        PropertyValue& rProp = pProps[ n ];

        rProp.Name   = rtl::OUString(); // n/a
        rProp.Handle = pHandles[ n ];
        rProp.Value  = pValues[ n ];
//        rProp.State  = ;
    }

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("setPropertyValues"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Sequence< Any > aErrors;
    aResult >>= aErrors;
    return aErrors;
}

//=========================================================================
Any Content::executeCommand( const rtl::OUString& rCommandName,
                             const Any& rCommandArgument )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Command aCommand;
    aCommand.Name     = rCommandName;
    aCommand.Handle   = -1; // n/a
    aCommand.Argument = rCommandArgument;

    return m_xImpl->executeCommand( aCommand );
}

//=========================================================================
Any Content::executeCommand( sal_Int32 nCommandHandle,
                             const Any& rCommandArgument )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Command aCommand;
    aCommand.Name     = rtl::OUString(); // n/a
    aCommand.Handle   = nCommandHandle;
    aCommand.Argument = rCommandArgument;

    return m_xImpl->executeCommand( aCommand );
}

//=========================================================================
void Content::abortCommand()
{
    m_xImpl->abortCommand();
}

//=========================================================================
Any Content::createCursorAny( const Sequence< rtl::OUString >& rPropertyNames,
                              ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();
    const rtl::OUString* pNames = rPropertyNames.getConstArray();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        Property& rProp = pProps[ n ];
        rProp.Name   = pNames[ n ];
        rProp.Handle = -1; // n/a
    }

    OpenCommandArgument2 aArg;
    aArg.Mode       = ( eMode == INCLUDE_FOLDERS_ONLY )
                        ? OpenMode::FOLDERS
                        : ( eMode == INCLUDE_DOCUMENTS_ONLY )
                            ? OpenMode::DOCUMENTS : OpenMode::ALL;
    aArg.Priority   = 0; // unused
    aArg.Sink       = Reference< XInterface >(); // unused
    aArg.Properties = aProps;

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    return m_xImpl->executeCommand( aCommand );
}

//=========================================================================
Any Content::createCursorAny( const Sequence< sal_Int32 >& rPropertyHandles,
                              ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = rPropertyHandles.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();
    const sal_Int32* pHandles = rPropertyHandles.getConstArray();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        Property& rProp = pProps[ n ];
        rProp.Name   = rtl::OUString(); // n/a
        rProp.Handle = pHandles[ n ];
    }

    OpenCommandArgument2 aArg;
    aArg.Mode       = ( eMode == INCLUDE_FOLDERS_ONLY )
                        ? OpenMode::FOLDERS
                        : ( eMode == INCLUDE_DOCUMENTS_ONLY )
                            ? OpenMode::DOCUMENTS : OpenMode::ALL;
    aArg.Priority   = 0; // unused
    aArg.Sink       = Reference< XInterface >(); // unused
    aArg.Properties = aProps;

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    return m_xImpl->executeCommand( aCommand );
}

//=========================================================================
Reference< XResultSet > Content::createCursor(
                            const Sequence< rtl::OUString >& rPropertyNames,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Any aCursorAny = createCursorAny( rPropertyNames, eMode );

    Reference< XDynamicResultSet > xDynSet;
    Reference< XResultSet > aResult;

    aCursorAny >>= xDynSet;
    if ( xDynSet.is() )
        aResult = xDynSet->getStaticResultSet();

    OSL_ENSURE( aResult.is(), "Content::createCursor - no cursor!" );

    if ( !aResult.is() )
    {
        // Former, the open command directly returned a XResultSet.
        aCursorAny >>= aResult;

        OSL_ENSURE( !aResult.is(),
                    "Content::createCursor - open-Command must "
                    "return a Reference< XDynnamicResultSet >!" );
    }

    return aResult;
}

//=========================================================================
Reference< XResultSet > Content::createCursor(
                            const Sequence< sal_Int32 >& rPropertyHandles,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Any aCursorAny = createCursorAny( rPropertyHandles, eMode );

    Reference< XDynamicResultSet > xDynSet;
    Reference< XResultSet > aResult;

    aCursorAny >>= xDynSet;
    if ( xDynSet.is() )
        aResult = xDynSet->getStaticResultSet();

    OSL_ENSURE( aResult.is(), "Content::createCursor - no cursor!" );

    if ( !aResult.is() )
    {
        // Former, the open command directly returned a XResultSet.
        aCursorAny >>= aResult;

        OSL_ENSURE( !aResult.is(),
                    "Content::createCursor - open-Command must "
                    "return a Reference< XDynnamicResultSet >!" );
    }

    return aResult;
}

//=========================================================================
Reference< XDynamicResultSet > Content::createDynamicCursor(
                            const Sequence< rtl::OUString >& rPropertyNames,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XDynamicResultSet > aResult;
    createCursorAny( rPropertyNames, eMode ) >>= aResult;

    OSL_ENSURE( aResult.is(), "Content::createDynamicCursor - no cursor!" );

    return aResult;
}

//=========================================================================
Reference< XDynamicResultSet > Content::createDynamicCursor(
                            const Sequence< sal_Int32 >& rPropertyHandles,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XDynamicResultSet > aResult;
    createCursorAny( rPropertyHandles, eMode ) >>= aResult;

    OSL_ENSURE( aResult.is(), "Content::createDynamicCursor - no cursor!" );

    return aResult;
}

//=========================================================================
Reference< XDynamicResultSet > Content::createSortedDynamicCursor(
                            const Sequence< rtl::OUString >& rPropertyNames,
                            const Sequence< NumberedSortingInfo >& rSortInfo,
                            Reference< XAnyCompareFactory > rAnyCompareFactory,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XDynamicResultSet > aResult;
    Reference< XDynamicResultSet > aOrigCursor = createDynamicCursor( rPropertyNames, eMode );

    if( aOrigCursor.is() )
    {
        Reference< XMultiServiceFactory > aServiceManager = m_xImpl->getServiceManager();

        if( aServiceManager.is() )
        {
            Reference< XSortedDynamicResultSetFactory > aSortFactory( aServiceManager->createInstance(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SortedDynamicResultSetFactory"))),
                                UNO_QUERY );

            aResult = aSortFactory->createSortedDynamicResultSet( aOrigCursor,
                                                              rSortInfo,
                                                              rAnyCompareFactory );
        }

        OSL_ENSURE( aResult.is(), "Content::createSortedDynamicCursor - no sorted cursor!\n" );

        if( !aResult.is() )
            aResult = aOrigCursor;
    }

    return aResult;
}

//=========================================================================
Reference< XDynamicResultSet > Content::createSortedDynamicCursor(
                            const Sequence< sal_Int32 >& rPropertyHandles,
                            const Sequence< NumberedSortingInfo >& rSortInfo,
                            Reference< XAnyCompareFactory > rAnyCompareFactory,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XDynamicResultSet > aResult;
    Reference< XDynamicResultSet > aOrigCursor = createDynamicCursor( rPropertyHandles, eMode );

    if( aOrigCursor.is() )
    {
        Reference< XMultiServiceFactory > aServiceManager = m_xImpl->getServiceManager();

        if( aServiceManager.is() )
        {
            Reference< XSortedDynamicResultSetFactory > aSortFactory( aServiceManager->createInstance(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SortedDynamicResultSetFactory"))),
                                UNO_QUERY );

            aResult = aSortFactory->createSortedDynamicResultSet( aOrigCursor,
                                                              rSortInfo,
                                                              rAnyCompareFactory );
        }

        OSL_ENSURE( aResult.is(), "Content::createSortedDynamicCursor - no sorted cursor!\n" );

        if( !aResult.is() )
            aResult = aOrigCursor;
    }

    return aResult;
}

//=========================================================================
Reference< XResultSet > Content::createSortedCursor(
                            const Sequence< rtl::OUString >& rPropertyNames,
                            const Sequence< NumberedSortingInfo >& rSortInfo,
                            Reference< XAnyCompareFactory > rAnyCompareFactory,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XResultSet > aResult;
    Reference< XDynamicResultSet > aDynSet;

    Any aCursorAny = createCursorAny( rPropertyNames, eMode );

    aCursorAny >>= aDynSet;

    if( aDynSet.is() )
    {
        Reference< XDynamicResultSet > aDynResult;
        Reference< XMultiServiceFactory > aServiceManager = m_xImpl->getServiceManager();

        if( aServiceManager.is() )
        {
            Reference< XSortedDynamicResultSetFactory > aSortFactory( aServiceManager->createInstance(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SortedDynamicResultSetFactory"))),
                                UNO_QUERY );

            aDynResult = aSortFactory->createSortedDynamicResultSet( aDynSet,
                                                              rSortInfo,
                                                              rAnyCompareFactory );
        }

        OSL_ENSURE( aDynResult.is(), "Content::createSortedCursor - no sorted cursor!\n" );

        if( aDynResult.is() )
            aResult = aDynResult->getStaticResultSet();
        else
            aResult = aDynSet->getStaticResultSet();
    }

    OSL_ENSURE( aResult.is(), "Content::createSortedCursor - no cursor!" );

    if ( !aResult.is() )
    {
        // Former, the open command directly returned a XResultSet.
        aCursorAny >>= aResult;

        OSL_ENSURE( !aResult.is(),
                    "Content::createCursor - open-Command must "
                    "return a Reference< XDynnamicResultSet >!" );
    }

    return aResult;
}

//=========================================================================
Reference< XResultSet > Content::createSortedCursor(
                            const Sequence< sal_Int32 >& rPropertyHandles,
                            const Sequence< NumberedSortingInfo >& rSortInfo,
                            Reference< XAnyCompareFactory > rAnyCompareFactory,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XResultSet > aResult;
    Reference< XDynamicResultSet > aDynSet;

    Any aCursorAny = createCursorAny( rPropertyHandles, eMode );

    aCursorAny >>= aDynSet;

    if( aDynSet.is() )
    {
        Reference< XDynamicResultSet > aDynResult;
        Reference< XMultiServiceFactory > aServiceManager = m_xImpl->getServiceManager();

        if( aServiceManager.is() )
        {
            Reference< XSortedDynamicResultSetFactory > aSortFactory( aServiceManager->createInstance(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SortedDynamicResultSetFactory"))),
                                UNO_QUERY );

            aDynResult = aSortFactory->createSortedDynamicResultSet( aDynSet,
                                                              rSortInfo,
                                                              rAnyCompareFactory );
        }

        OSL_ENSURE( aDynResult.is(), "Content::createSortedCursor - no sorted cursor!\n" );

        if( aDynResult.is() )
            aResult = aDynResult->getStaticResultSet();
        else
            aResult = aDynSet->getStaticResultSet();
    }

    OSL_ENSURE( aResult.is(), "Content::createSortedCursor - no cursor!" );

    if ( !aResult.is() )
    {
        // Former, the open command directly returned a XResultSet.
        aCursorAny >>= aResult;

        OSL_ENSURE( !aResult.is(),
                    "Content::createCursor - open-Command must "
                    "return a Reference< XDynnamicResultSet >!" );
    }

    return aResult;
}

//=========================================================================
Reference< XInputStream > Content::openStream()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isDocument() )
        return Reference< XInputStream >();

    Reference< XActiveDataSink > xSink = new ActiveDataSink;

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT;
    aArg.Priority   = 0; // unused
    aArg.Sink       = xSink;
    aArg.Properties = Sequence< Property >( 0 ); // unused

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );

    return xSink->getInputStream();
}

//=========================================================================
Reference< XInputStream > Content::openStreamNoLock()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isDocument() )
        return Reference< XInputStream >();

    Reference< XActiveDataSink > xSink = new ActiveDataSink;

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT_SHARE_DENY_NONE;
    aArg.Priority   = 0; // unused
    aArg.Sink       = xSink;
    aArg.Properties = Sequence< Property >( 0 ); // unused

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );

    return xSink->getInputStream();
}

//=========================================================================
Reference< XStream > Content::openWriteableStream()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isDocument() )
        return Reference< XStream >();

    Reference< XActiveDataStreamer > xStreamer = new ActiveDataStreamer;

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT;
    aArg.Priority   = 0; // unused
    aArg.Sink       = xStreamer;
    aArg.Properties = Sequence< Property >( 0 ); // unused

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );

    return xStreamer->getStream();
}

//=========================================================================
Reference< XStream > Content::openWriteableStreamNoLock()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isDocument() )
        return Reference< XStream >();

    Reference< XActiveDataStreamer > xStreamer = new ActiveDataStreamer;

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT_SHARE_DENY_NONE;
    aArg.Priority   = 0; // unused
    aArg.Sink       = xStreamer;
    aArg.Properties = Sequence< Property >( 0 ); // unused

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );

    return xStreamer->getStream();
}

//=========================================================================
sal_Bool Content::openStream( const Reference< XActiveDataSink >& rSink )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isDocument() )
        return sal_False;

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT;
    aArg.Priority   = 0; // unused
    aArg.Sink       = rSink;
    aArg.Properties = Sequence< Property >( 0 ); // unused

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );

    return sal_True;
}

//=========================================================================
sal_Bool Content::openStream( const Reference< XOutputStream >& rStream )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( !isDocument() )
        return sal_False;

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::DOCUMENT;
    aArg.Priority   = 0; // unused
    aArg.Sink       = rStream;
    aArg.Properties = Sequence< Property >( 0 ); // unused

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );

    return sal_True;
}

//=========================================================================
void Content::writeStream( const Reference< XInputStream >& rStream,
                           sal_Bool bReplaceExisting )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    InsertCommandArgument aArg;
    aArg.Data            = rStream.is() ? rStream : new EmptyInputStream;
    aArg.ReplaceExisting = bReplaceExisting;

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("insert"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    m_xImpl->executeCommand( aCommand );

    m_xImpl->inserted();
}

//=========================================================================
Sequence< ContentInfo > Content::queryCreatableContentsInfo()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    // First, try it using "CreatableContentsInfo" property -> the "new" way.
    Sequence< ContentInfo > aInfo;
    if ( getPropertyValue(
             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CreatableContentsInfo")) )
         >>= aInfo )
        return aInfo;

    // Second, try it using XContentCreator interface -> the "old" way (not
    // providing the chance to supply an XCommandEnvironment.
    Reference< XContentCreator > xCreator( m_xImpl->getContent(), UNO_QUERY );
    if ( xCreator.is() )
        aInfo = xCreator->queryCreatableContentsInfo();

    return aInfo;
}

//=========================================================================
sal_Bool Content::insertNewContent( const rtl::OUString& rContentType,
                                    const Sequence< rtl::OUString >&
                                        rPropertyNames,
                                    const Sequence< Any >& rPropertyValues,
                                    Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    return insertNewContent( rContentType,
                             rPropertyNames,
                             rPropertyValues,
                             new EmptyInputStream,
                             rNewContent );
}

//=========================================================================
sal_Bool Content::insertNewContent( const rtl::OUString& rContentType,
                                    const Sequence< sal_Int32 >&
                                        nPropertyHandles,
                                    const Sequence< Any >& rPropertyValues,
                                    Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    return insertNewContent( rContentType,
                             nPropertyHandles,
                             rPropertyValues,
                             new EmptyInputStream,
                             rNewContent );
}

//=========================================================================
sal_Bool Content::insertNewContent( const rtl::OUString& rContentType,
                                    const Sequence< rtl::OUString >&
                                        rPropertyNames,
                                    const Sequence< Any >& rPropertyValues,
                                    const Reference< XInputStream >& rData,
                                    Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rContentType.getLength() == 0 )
        return sal_False;

    // First, try it using "createNewContent" command -> the "new" way.
    ContentInfo aInfo;
    aInfo.Type = rContentType;
    aInfo.Attributes = 0;

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("createNewContent"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aInfo;

    Reference< XContent > xNew;
    try
    {
        m_xImpl->executeCommand( aCommand ) >>= xNew;
    }
    catch ( RuntimeException const & )
    {
        throw;
    }
    catch ( Exception const & )
    {
    }

    if ( !xNew.is() )
    {
        // Second, try it using XContentCreator interface -> the "old"
        // way (not providing the chance to supply an XCommandEnvironment.
        Reference< XContentCreator > xCreator( m_xImpl->getContent(), UNO_QUERY );

        if ( !xCreator.is() )
            return sal_False;

        xNew = xCreator->createNewContent( aInfo );

        if ( !xNew.is() )
            return sal_False;
    }

    Content aNewContent( xNew, m_xImpl->getEnvironment() );
    aNewContent.setPropertyValues( rPropertyNames, rPropertyValues );
    aNewContent.executeCommand( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("insert")),
                                makeAny(
                                    InsertCommandArgument(
                                        rData.is() ? rData : new EmptyInputStream,
                                        sal_False /* ReplaceExisting */ ) ) );
    aNewContent.m_xImpl->inserted();

    rNewContent = aNewContent;
    return sal_True;
}

//=========================================================================
sal_Bool Content::insertNewContent( const rtl::OUString& rContentType,
                                    const Sequence< sal_Int32 >&
                                        nPropertyHandles,
                                    const Sequence< Any >& rPropertyValues,
                                    const Reference< XInputStream >& rData,
                                    Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rContentType.getLength() == 0 )
        return sal_False;

    // First, try it using "createNewContent" command -> the "new" way.
    ContentInfo aInfo;
    aInfo.Type = rContentType;
    aInfo.Attributes = 0;

    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("createNewContent"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aInfo;

    Reference< XContent > xNew;
    try
    {
        m_xImpl->executeCommand( aCommand ) >>= xNew;
    }
    catch ( RuntimeException const & )
    {
        throw;
    }
    catch ( Exception const & )
    {
    }

    if ( !xNew.is() )
    {
        // Second, try it using XContentCreator interface -> the "old"
        // way (not providing the chance to supply an XCommandEnvironment.
        Reference< XContentCreator > xCreator( m_xImpl->getContent(), UNO_QUERY );

        if ( !xCreator.is() )
            return sal_False;

        xNew = xCreator->createNewContent( aInfo );

        if ( !xNew.is() )
            return sal_False;
    }

    Content aNewContent( xNew, m_xImpl->getEnvironment() );
    aNewContent.setPropertyValues( nPropertyHandles, rPropertyValues );
    aNewContent.executeCommand( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("insert")),
                                makeAny(
                                    InsertCommandArgument(
                                        rData.is() ? rData : new EmptyInputStream,
                                        sal_False /* ReplaceExisting */ ) ) );
    aNewContent.m_xImpl->inserted();

    rNewContent = aNewContent;
    return sal_True;
}

//=========================================================================
sal_Bool Content::transferContent( const Content& rSourceContent,
                                   InsertOperation eOperation,
                                   const rtl::OUString & rTitle,
                                   const sal_Int32 nNameClashAction )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    ContentBroker* pBroker = ContentBroker::get();
    if ( !pBroker )
    {
        OSL_FAIL( "Content::transferContent - No Content Broker!" );
        return sal_False;
    }

    Reference< XCommandProcessor > xCmdProc(
                                    pBroker->getCommandProcessorInterface() );
    if ( !xCmdProc.is() )
    {
        OSL_FAIL( "Content::transferContent - No XCommandProcessor!" );
        return sal_False;
    }

    // Execute command "globalTransfer" at UCB.

    TransferCommandOperation eTransOp = TransferCommandOperation();
    switch ( eOperation )
    {
        case InsertOperation_COPY:
            eTransOp = TransferCommandOperation_COPY;
            break;

        case InsertOperation_MOVE:
            eTransOp = TransferCommandOperation_MOVE;
            break;

        case InsertOperation_LINK:
            eTransOp = TransferCommandOperation_LINK;
            break;

        default:
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                "Unknown transfer operation!" )),
                            get(),
                            -1 ) ),
                         m_xImpl->getEnvironment() );
            // Unreachable
    }

    GlobalTransferCommandArgument aTransferArg(
                                        eTransOp,
                                        rSourceContent.getURL(), // SourceURL
                                        getURL(),   // TargetFolderURL,
                                        rTitle,
                                        nNameClashAction );
    Command aCommand;
    aCommand.Name     = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("globalTransfer"));
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aTransferArg;

    xCmdProc->execute( aCommand, 0, m_xImpl->getEnvironment() );
    return sal_True;
}

//=========================================================================
sal_Bool Content::isFolder()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Bool bFolder = sal_False;
    if ( getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsFolder")) )
        >>= bFolder )
        return bFolder;

     ucbhelper::cancelCommandExecution(
         makeAny( UnknownPropertyException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Unable to retreive value of property 'IsFolder'!" )),
                    get() ) ),
         m_xImpl->getEnvironment() );

    // Unreachable - cancelCommandExecution always throws an exception.
    // But some compilers complain...
    return sal_False;
}

//=========================================================================
sal_Bool Content::isDocument()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Bool bDoc = sal_False;
    if ( getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsDocument")) )
        >>= bDoc )
        return bDoc;

     ucbhelper::cancelCommandExecution(
         makeAny( UnknownPropertyException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Unable to retreive value of property 'IsDocument'!" )),
                    get() ) ),
         m_xImpl->getEnvironment() );

    // Unreachable - cancelCommandExecution always throws an exception,
    // But some compilers complain...
    return sal_False;
}

//=========================================================================
//=========================================================================
//
// Content_Impl Implementation.
//
//=========================================================================
//=========================================================================

Content_Impl::Content_Impl( const Reference< XMultiServiceFactory >& rSMgr,
                            const Reference< XContent >& rContent,
                            const Reference< XCommandEnvironment >& rEnv )
: m_xSMgr( rSMgr ),
  m_xContent( rContent ),
  m_xEnv( rEnv ),
  m_nCommandId( 0 )
{
    if ( m_xContent.is() )
    {
        m_xContentEventListener = new ContentEventListener_Impl( *this );
        m_xContent->addContentEventListener( m_xContentEventListener );

#if OSL_DEBUG_LEVEL > 1
        // Only done on demand in product version for performance reasons,
        // but a nice debug helper.
        getURL();
#endif
    }
}

//=========================================================================
void Content_Impl::reinit( const Reference< XContent >& xContent )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_xCommandProcessor = 0;
    m_nCommandId = 0;

    // #92581# - Don't reset m_aURL!!!

    if ( m_xContent.is() )
    {
        try
        {
            m_xContent->removeContentEventListener( m_xContentEventListener );
        }
        catch ( RuntimeException const & )
        {
        }
    }

    if ( xContent.is() )
    {
        m_xContent = xContent;
        m_xContent->addContentEventListener( m_xContentEventListener );

#if OSL_DEBUG_LEVEL > 1
        // Only done on demand in product version for performance reasons,
        // but a nice debug helper.
        getURL();
#endif
    }
    else
    {
        // We need m_xContent's URL in order to be able to create the
        // content object again if demanded ( --> Content_Impl::getContent() )
        getURL();

        m_xContent = 0;
    }
}

//=========================================================================
// virtual
Content_Impl::~Content_Impl()
{
    if ( m_xContent.is() )
    {
        try
        {
            m_xContent->removeContentEventListener( m_xContentEventListener );
        }
        catch ( RuntimeException const & )
        {
        }
    }
}

//=========================================================================
void Content_Impl::disposing( const EventObject& Source )
{
    Reference<XContent> xContent;

    {
        osl::MutexGuard aGuard( m_aMutex );
        if(Source.Source != m_xContent)
            return;

        xContent = m_xContent;

        m_nCommandId = 0;
        m_aURL = rtl::OUString();
        m_xCommandProcessor = 0;
        m_xContent = 0;
    }

    if ( xContent.is() )
    {
        try
        {
            xContent->removeContentEventListener( m_xContentEventListener );
        }
        catch ( RuntimeException const & )
        {
        }
    }
}

//=========================================================================
const rtl::OUString& Content_Impl::getURL() const
{
    if ( !m_aURL.getLength() && m_xContent.is() )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_aURL.getLength() && m_xContent.is() )
        {
            Reference< XContentIdentifier > xId = m_xContent->getIdentifier();
            if ( xId.is() )
                m_aURL = xId->getContentIdentifier();
        }
    }

    return m_aURL;
}

//=========================================================================
Reference< XContent > Content_Impl::getContent()
{
    if ( !m_xContent.is() && m_aURL.getLength() )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_xContent.is() && m_aURL.getLength() )
        {
            ContentBroker* pBroker = ContentBroker::get();

            OSL_ENSURE( pBroker, "No Content Broker!" );

            if ( pBroker )
            {
                OSL_ENSURE( pBroker->getContentProviderManagerInterface()
                                        ->queryContentProviders().getLength(),
                            "Content Broker not configured (no providers)!" );

                Reference< XContentIdentifierFactory > xIdFac
                            = pBroker->getContentIdentifierFactoryInterface();

                OSL_ENSURE( xIdFac.is(), "No Content Identifier factory!" );

                if ( xIdFac.is() )
                {
                    Reference< XContentIdentifier > xId
                                = xIdFac->createContentIdentifier( m_aURL );

                    OSL_ENSURE( xId.is(), "No Content Identifier!" );

                    if ( xId.is() )
                    {
                        Reference< XContentProvider > xProvider
                            = pBroker->getContentProviderInterface();

                        OSL_ENSURE( xProvider.is(), "No Content Provider!" );

                        if ( xProvider.is() )
                        {
                            try
                            {
                                m_xContent = xProvider->queryContent( xId );
                            }
                            catch ( IllegalIdentifierException const & )
                            {
                            }

                            if ( m_xContent.is() )
                                m_xContent->addContentEventListener(
                                                m_xContentEventListener );
                        }
                    }
                }
            }
        }
    }

    return m_xContent;
}

//=========================================================================
Reference< XCommandProcessor > Content_Impl::getCommandProcessor()
{
    if ( !m_xCommandProcessor.is() )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_xCommandProcessor.is() )
            m_xCommandProcessor
                = Reference< XCommandProcessor >( getContent(), UNO_QUERY );
    }

    return m_xCommandProcessor;
}

//=========================================================================
sal_Int32 Content_Impl::getCommandId()
{
    if ( m_nCommandId == 0 )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( m_nCommandId == 0 )
        {
            Reference< XCommandProcessor > xProc = getCommandProcessor();
            if ( xProc.is() )
                m_nCommandId = xProc->createCommandIdentifier();
        }
    }

    return m_nCommandId;
}

//=========================================================================
Any Content_Impl::executeCommand( const Command& rCommand )
{
    Reference< XCommandProcessor > xProc = getCommandProcessor();
    if ( !xProc.is() )
        return Any();

    // Execute command
    return xProc->execute( rCommand, getCommandId(), m_xEnv );
}

//=========================================================================
void Content_Impl::abortCommand()
{
    sal_Int32 nCommandId;
    Reference< XCommandProcessor > xCommandProcessor;
    {
        osl::MutexGuard aGuard( m_aMutex );
        nCommandId = m_nCommandId;
        xCommandProcessor = m_xCommandProcessor;
    }

    if ( ( nCommandId != 0 ) && xCommandProcessor.is() )
        xCommandProcessor->abort( nCommandId );
}

//=========================================================================
inline const Reference< XCommandEnvironment >&
                                        Content_Impl::getEnvironment() const
{
    return m_xEnv;
}

//=========================================================================
inline void Content_Impl::setEnvironment(
                        const Reference< XCommandEnvironment >& xNewEnv )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_xEnv = xNewEnv;
}

//=========================================================================
void Content_Impl::inserted()
{
    // URL might have changed during 'insert' => recalculate in next getURL()
    osl::MutexGuard aGuard( m_aMutex );
    m_aURL = ::rtl::OUString();
}

//=========================================================================
//=========================================================================
//
// ContentEventListener_Impl Implementation.
//
//=========================================================================
//=========================================================================

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_2( ContentEventListener_Impl,
                   XContentEventListener,
                   XEventListener ); /* base of XContentEventListener */

//=========================================================================
//
// XContentEventListener methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentEventListener_Impl::contentEvent( const ContentEvent& evt )
    throw( RuntimeException )
{
    if ( evt.Source == m_rContent.m_xContent )
    {
        switch ( evt.Action )
        {
            case ContentAction::DELETED:
                m_rContent.reinit( Reference< XContent >() );
                break;

            case ContentAction::EXCHANGED:
                m_rContent.reinit( evt.Content );
                break;

            default:
                break;
        }
    }
}

//=========================================================================
//
// XEventListenr methods.
//
//=========================================================================

// virtual
void SAL_CALL ContentEventListener_Impl::disposing( const EventObject& Source )
    throw( RuntimeException )
{
    m_rContent.disposing(Source);
}

} /* namespace ucbhelper */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
