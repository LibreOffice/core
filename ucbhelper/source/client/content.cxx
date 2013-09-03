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

#include "sal/config.h"

#include <cassert>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <cppuhelper/weak.hxx>

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/ucb/CheckinArgument.hpp>
#include <com/sun/star/ucb/ContentCreationError.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/CommandInfo.hpp>
#include <com/sun/star/ucb/ContentAction.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/GlobalTransferCommandArgument2.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <com/sun/star/ucb/XContentEventListener.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/SortedDynamicResultSetFactory.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/content.hxx>
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

    mutable OUString               m_aURL;
    Reference< XComponentContext >      m_xCtx;
    Reference< XContent >               m_xContent;
    Reference< XCommandProcessor >          m_xCommandProcessor;
    Reference< XCommandEnvironment >    m_xEnv;
    Reference< XContentEventListener >  m_xContentEventListener;
    mutable osl::Mutex                  m_aMutex;

private:
    void reinit( const Reference< XContent >& xContent );
    void disposing(const EventObject& Source);

public:
    Content_Impl() {};
    Content_Impl( const Reference< XComponentContext >& rCtx,
                  const Reference< XContent >& rContent,
                  const Reference< XCommandEnvironment >& rEnv );

    virtual ~Content_Impl();

    const OUString&           getURL() const;
    Reference< XContent >          getContent();
    Reference< XCommandProcessor > getCommandProcessor();
    Reference< XComponentContext > getComponentContext()
    { assert(m_xCtx.is()); return m_xCtx; }

    Any  executeCommand( const Command& rCommand );

    inline const Reference< XCommandEnvironment >& getEnvironment() const;
    inline void setEnvironment(
                        const Reference< XCommandEnvironment >& xNewEnv );

    void inserted();
};

//=========================================================================
// Helpers.
//=========================================================================

static void ensureContentProviderForURL( const Reference< XUniversalContentBroker >& rBroker,
                                         const OUString & rURL )
    throw ( ContentCreationException, RuntimeException )
{
    Reference< XContentProvider > xProv
        = rBroker->queryContentProvider( rURL );
    if ( !xProv.is() )
    {
        throw ContentCreationException(
            "No Content Provider available for URL: " + rURL,
            Reference< XInterface >(),
            ContentCreationError_NO_CONTENT_PROVIDER );
    }
}

//=========================================================================
static Reference< XContentIdentifier > getContentIdentifier(
                                    const Reference< XUniversalContentBroker > & rBroker,
                                    const OUString & rURL,
                                    bool bThrow )
    throw ( ContentCreationException, RuntimeException )
{
    Reference< XContentIdentifier > xId
        = rBroker->createContentIdentifier( rURL );

    if ( xId.is() )
        return xId;

    if ( bThrow )
    {
        ensureContentProviderForURL( rBroker, rURL );

        throw ContentCreationException(
            "Unable to create Content Identifier!",
            Reference< XInterface >(),
            ContentCreationError_IDENTIFIER_CREATION_FAILED );
    }

    return Reference< XContentIdentifier >();
}

//=========================================================================
static Reference< XContent > getContent(
                                    const Reference< XUniversalContentBroker > & rBroker,
                                    const Reference< XContentIdentifier > & xId,
                                    bool bThrow )
    throw ( ContentCreationException, RuntimeException )
{
    Reference< XContent > xContent;
    OUString msg;
    try
    {
        xContent = rBroker->queryContent( xId );
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
            "Unable to create Content! " + msg,
            Reference< XInterface >(),
            ContentCreationError_CONTENT_CREATION_FAILED );
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
Content::Content( const OUString& rURL,
                  const Reference< XCommandEnvironment >& rEnv,
                  const Reference< XComponentContext >& rCtx )
    throw ( ContentCreationException, RuntimeException )
{
    Reference< XUniversalContentBroker > pBroker(
        UniversalContentBroker::create( rCtx ) );

    Reference< XContentIdentifier > xId
        = getContentIdentifier( pBroker, rURL, true );

    Reference< XContent > xContent = getContent( pBroker, xId, true );

    m_xImpl = new Content_Impl( rCtx, xContent, rEnv );
}

//=========================================================================
Content::Content( const Reference< XContent >& rContent,
                  const Reference< XCommandEnvironment >& rEnv,
                  const Reference< XComponentContext >& rCtx )
    throw ( ContentCreationException, RuntimeException )
{
    m_xImpl = new Content_Impl( rCtx, rContent, rEnv );
}

//=========================================================================
Content::Content( const Content& rOther )
{
    m_xImpl = rOther.m_xImpl;
}

//=========================================================================
// static
sal_Bool Content::create( const OUString& rURL,
                          const Reference< XCommandEnvironment >& rEnv,
                          const Reference< XComponentContext >& rCtx,
                          Content& rContent )
{
    Reference< XUniversalContentBroker > pBroker(
        UniversalContentBroker::create( rCtx ) );

    Reference< XContentIdentifier > xId
        = getContentIdentifier( pBroker, rURL, false );
    if ( !xId.is() )
        return sal_False;

    Reference< XContent > xContent = getContent( pBroker, xId, false );
    if ( !xContent.is() )
        return sal_False;

    rContent.m_xImpl
        = new Content_Impl( rCtx, xContent, rEnv );

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
const OUString& Content::getURL() const
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
    aCommand.Name     = OUString("getCommandInfo");
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
    aCommand.Name     = OUString("getPropertySetInfo");
    aCommand.Handle   = -1; // n/a
    aCommand.Argument = Any();

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XPropertySetInfo > xInfo;
    aResult >>= xInfo;
    return xInfo;
}

//=========================================================================
Any Content::getPropertyValue( const OUString& rPropertyName )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< OUString > aNames( 1 );
    aNames.getArray()[ 0 ] = rPropertyName;

    Sequence< Any > aRet = getPropertyValues( aNames );
    return aRet.getConstArray()[ 0 ];
}

//=========================================================================
Any Content::setPropertyValue( const OUString& rName,
                                const Any& rValue )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Sequence< OUString > aNames( 1 );
    aNames.getArray()[ 0 ] = rName;

    Sequence< Any > aValues( 1 );
    aValues.getArray()[ 0 ] = rValue;

    Sequence< Any > aErrors = setPropertyValues( aNames, aValues );
    return aErrors.getConstArray()[ 0 ];
}

//=========================================================================
Sequence< Any > Content::getPropertyValues(
                            const Sequence< OUString >& rPropertyNames )
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
Reference< XRow > Content::getPropertyValuesInterface(
                            const Sequence< OUString >& rPropertyNames )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();

    const OUString* pNames  = rPropertyNames.getConstArray();

    for ( sal_Int32 n = 0; n< nCount; ++n )
    {
        Property& rProp = pProps[ n ];

        rProp.Name       = pNames[ n ];
        rProp.Handle     = -1; // n/a
//        rProp.Type       =
//        rProp.Attributes = ;
    }

    Command aCommand;
    aCommand.Name     = OUString("getPropertyValues");
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Reference< XRow > xRow;
    aResult >>= xRow;
    return xRow;
}

//=========================================================================
Sequence< Any > Content::setPropertyValues(
                            const Sequence< OUString >& rPropertyNames,
                                const Sequence< Any >& rValues )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rPropertyNames.getLength() != rValues.getLength() )
    {
        ucbhelper::cancelCommandExecution(
            makeAny( IllegalArgumentException(
                        OUString(
                            "Length of property names sequence and value "
                            "sequence are unequal!" ),
                        get(),
                        -1 ) ),
            m_xImpl->getEnvironment() );
        // Unreachable
    }

    sal_Int32 nCount = rValues.getLength();
    Sequence< PropertyValue > aProps( nCount );
    PropertyValue* pProps = aProps.getArray();

    const OUString* pNames  = rPropertyNames.getConstArray();
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
    aCommand.Name     = OUString("setPropertyValues");
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aProps;

    Any aResult = m_xImpl->executeCommand( aCommand );

    Sequence< Any > aErrors;
    aResult >>= aErrors;
    return aErrors;
}

//=========================================================================
Any Content::executeCommand( const OUString& rCommandName,
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
Any Content::createCursorAny( const Sequence< OUString >& rPropertyNames,
                              ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Int32 nCount = rPropertyNames.getLength();
    Sequence< Property > aProps( nCount );
    Property* pProps = aProps.getArray();
    const OUString* pNames = rPropertyNames.getConstArray();
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
    aArg.Sink.clear(); // unused
    aArg.Properties = aProps;

    Command aCommand;
    aCommand.Name     = OUString("open");
    aCommand.Handle   = -1; // n/a
    aCommand.Argument <<= aArg;

    return m_xImpl->executeCommand( aCommand );
}

//=========================================================================
Reference< XResultSet > Content::createCursor(
                            const Sequence< OUString >& rPropertyNames,
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
Reference< XDynamicResultSet > Content::createDynamicCursor(
                            const Sequence< OUString >& rPropertyNames,
                            ResultSetInclude eMode )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XDynamicResultSet > aResult;
    createCursorAny( rPropertyNames, eMode ) >>= aResult;

    OSL_ENSURE( aResult.is(), "Content::createDynamicCursor - no cursor!" );

    return aResult;
}

//=========================================================================
Reference< XResultSet > Content::createSortedCursor(
                            const Sequence< OUString >& rPropertyNames,
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

        if( m_xImpl->getComponentContext().is() )
        {
            Reference< XSortedDynamicResultSetFactory > aSortFactory =
                                SortedDynamicResultSetFactory::create( m_xImpl->getComponentContext());

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
    aCommand.Name     = OUString("open");
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
    aCommand.Name     = OUString("open");
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
    aCommand.Name     = OUString("open");
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
    aCommand.Name     = OUString("open");
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
    aCommand.Name     = OUString("open");
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
    aCommand.Name     = OUString("open");
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
    aCommand.Name     = OUString("insert");
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
             OUString("CreatableContentsInfo") )
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
sal_Bool Content::insertNewContent( const OUString& rContentType,
                                    const Sequence< OUString >&
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
sal_Bool Content::insertNewContent( const OUString& rContentType,
                                    const Sequence< OUString >&
                                        rPropertyNames,
                                    const Sequence< Any >& rPropertyValues,
                                    const Reference< XInputStream >& rData,
                                    Content& rNewContent )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    if ( rContentType.isEmpty() )
        return sal_False;

    // First, try it using "createNewContent" command -> the "new" way.
    ContentInfo aInfo;
    aInfo.Type = rContentType;
    aInfo.Attributes = 0;

    Command aCommand;
    aCommand.Name     = OUString("createNewContent");
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

    Content aNewContent(
        xNew, m_xImpl->getEnvironment(), m_xImpl->getComponentContext() );
    aNewContent.setPropertyValues( rPropertyNames, rPropertyValues );
    aNewContent.executeCommand( OUString("insert"),
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
                                   const OUString & rTitle,
                                   const sal_Int32 nNameClashAction,
                                   const OUString & rMimeType,
                                   bool bMajorVersion,
                                   const OUString & rVersionComment,
                                   OUString* pResultURL,
                                   const OUString & rDocumentId )
    throw( CommandAbortedException, RuntimeException, Exception )
{
    Reference< XUniversalContentBroker > pBroker(
        UniversalContentBroker::create( m_xImpl->getComponentContext() ) );

    // Execute command "globalTransfer" at UCB.

    TransferCommandOperation eTransOp = TransferCommandOperation();
    OUString sCommand( "globalTransfer" );
    bool bCheckIn = false;
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

        case InsertOperation_CHECKIN:
            eTransOp = TransferCommandOperation_COPY;
            sCommand = OUString( "checkin" );
            bCheckIn = true;
            break;

        default:
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                            OUString(
                                "Unknown transfer operation!" ),
                            get(),
                            -1 ) ),
                         m_xImpl->getEnvironment() );
            // Unreachable
    }
    Command aCommand;
    aCommand.Name     = sCommand;
    aCommand.Handle   = -1; // n/a

    if ( !bCheckIn )
    {
        GlobalTransferCommandArgument2 aTransferArg(
                                            eTransOp,
                                            rSourceContent.getURL(), // SourceURL
                                            getURL(),   // TargetFolderURL,
                                            rTitle,
                                            nNameClashAction,
                                            rMimeType,
                                            rDocumentId );
        aCommand.Argument <<= aTransferArg;
    }
    else
    {
        CheckinArgument aCheckinArg( bMajorVersion, rVersionComment,
                rSourceContent.getURL(), getURL(), rTitle, rMimeType );
        aCommand.Argument <<= aCheckinArg;
    }

    Any aRet = pBroker->execute( aCommand, 0, m_xImpl->getEnvironment() );
    if ( pResultURL != NULL )
        aRet >>= *pResultURL;
    return sal_True;
}

//=========================================================================
sal_Bool Content::isFolder()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Bool bFolder = sal_False;
    if ( getPropertyValue("IsFolder")
        >>= bFolder )
        return bFolder;

     ucbhelper::cancelCommandExecution(
         makeAny( UnknownPropertyException(
                    OUString(
                        "Unable to retrieve value of property 'IsFolder'!" ),
                    get() ) ),
         m_xImpl->getEnvironment() );

#if !(defined(_MSC_VER) && defined(ENABLE_LTO))
    // Unreachable - cancelCommandExecution always throws an exception.
    // But some compilers complain...
    return sal_False;
#endif
}

//=========================================================================
sal_Bool Content::isDocument()
    throw( CommandAbortedException, RuntimeException, Exception )
{
    sal_Bool bDoc = sal_False;
    if ( getPropertyValue("IsDocument")
        >>= bDoc )
        return bDoc;

     ucbhelper::cancelCommandExecution(
         makeAny( UnknownPropertyException(
                    OUString(
                        "Unable to retrieve value of property 'IsDocument'!" ),
                    get() ) ),
         m_xImpl->getEnvironment() );

#if !(defined(_MSC_VER) && defined(ENABLE_LTO))
    // Unreachable - cancelCommandExecution always throws an exception,
    // But some compilers complain...
    return sal_False;
#endif
}

//=========================================================================
//=========================================================================
//
// Content_Impl Implementation.
//
//=========================================================================
//=========================================================================

Content_Impl::Content_Impl( const Reference< XComponentContext >& rCtx,
                            const Reference< XContent >& rContent,
                            const Reference< XCommandEnvironment >& rEnv )
: m_xCtx( rCtx ),
  m_xContent( rContent ),
  m_xEnv( rEnv )
{
    assert(rCtx.is());
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

        m_aURL = OUString();
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
const OUString& Content_Impl::getURL() const
{
    if ( m_aURL.isEmpty() && m_xContent.is() )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( m_aURL.isEmpty() && m_xContent.is() )
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
    if ( !m_xContent.is() && !m_aURL.isEmpty() )
    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( !m_xContent.is() && !m_aURL.isEmpty() )
        {
            Reference< XUniversalContentBroker > pBroker(
                UniversalContentBroker::create( getComponentContext() ) );

            OSL_ENSURE( pBroker->queryContentProviders().getLength(),
                        "Content Broker not configured (no providers)!" );

            Reference< XContentIdentifier > xId
                = pBroker->createContentIdentifier( m_aURL );

            OSL_ENSURE( xId.is(), "No Content Identifier!" );

            if ( xId.is() )
            {
                try
                {
                    m_xContent = pBroker->queryContent( xId );
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
Any Content_Impl::executeCommand( const Command& rCommand )
{
    Reference< XCommandProcessor > xProc = getCommandProcessor();
    if ( !xProc.is() )
        return Any();

    // Execute command
    return xProc->execute( rCommand, 0, m_xEnv );
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
    m_aURL = OUString();
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
