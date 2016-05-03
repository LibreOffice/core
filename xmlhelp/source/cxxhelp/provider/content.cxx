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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <osl/diagnose.h>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include "content.hxx"
#include "provider.hxx"
#include "resultset.hxx"
#include "databases.hxx"
#include "resultsetfactory.hxx"
#include "resultsetbase.hxx"
#include "resultsetforroot.hxx"
#include "resultsetforquery.hxx"

using namespace com::sun::star;
using namespace chelp;

// Content Implementation.

Content::Content( const uno::Reference< uno::XComponentContext >& rxContext,
                  ::ucbhelper::ContentProviderImplHelper* pProvider,
                  const uno::Reference< ucb::XContentIdentifier >&
                      Identifier,
                  Databases* pDatabases )
    : ContentImplHelper( rxContext, pProvider, Identifier ),
      m_aURLParameter( Identifier->getContentIdentifier(),pDatabases ),
      m_pDatabases( pDatabases ) // not owner
{
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
    uno::Any aRet;
     return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface( rType );
}

// XTypeProvider methods.

XTYPEPROVIDER_COMMON_IMPL( Content );

// virtual
uno::Sequence< uno::Type > SAL_CALL Content::getTypes()
    throw( uno::RuntimeException, std::exception )
{
    static cppu::OTypeCollection* pCollection = nullptr;

    if ( !pCollection )
    {
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );
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

// XServiceInfo methods.

// virtual
OUString SAL_CALL Content::getImplementationName()
    throw( uno::RuntimeException, std::exception )
{
    return OUString( "CHelpContent" );
}

// virtual
uno::Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException, std::exception )
{
    uno::Sequence<OUString> aSNS { "com.sun.star.ucb.CHelpContent" };

    return aSNS;
}

// XContent methods.

// virtual
OUString SAL_CALL Content::getContentType()
    throw( uno::RuntimeException, std::exception )
{
    return OUString( MYUCP_CONTENT_TYPE );
}

// XCommandProcessor methods.

//virtual
void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
    throw( uno::RuntimeException, std::exception )
{
}

class ResultSetForRootFactory
    : public ResultSetFactory
{
private:

    uno::Reference< uno::XComponentContext >     m_xContext;
    uno::Reference< ucb::XContentProvider >      m_xProvider;
    uno::Sequence< beans::Property >             m_seq;
    uno::Sequence< ucb::NumberedSortingInfo >    m_seqSort;
    URLParameter                                 m_aURLParameter;
    Databases*                                   m_pDatabases;

public:

    ResultSetForRootFactory(
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< ucb::XContentProvider >&  xProvider,
        const uno::Sequence< beans::Property >& seq,
        const uno::Sequence< ucb::NumberedSortingInfo >& seqSort,
        const URLParameter& rURLParameter,
        Databases* pDatabases )
        : m_xContext( xContext ),
          m_xProvider( xProvider ),
          m_seq( seq ),
          m_seqSort( seqSort ),
          m_aURLParameter( rURLParameter ),
          m_pDatabases( pDatabases )
    {
    }

    ResultSetBase* createResultSet() override
    {
        return new ResultSetForRoot( m_xContext,
                                     m_xProvider,
                                     m_seq,
                                     m_seqSort,
                                     m_aURLParameter,
                                     m_pDatabases );
    }
};

class ResultSetForQueryFactory
    : public ResultSetFactory
{
private:

    uno::Reference< uno::XComponentContext >     m_xContext;
    uno::Reference< ucb::XContentProvider >      m_xProvider;
    uno::Sequence< beans::Property >             m_seq;
    uno::Sequence< ucb::NumberedSortingInfo >    m_seqSort;
    URLParameter                                 m_aURLParameter;
    Databases*                                   m_pDatabases;

public:

    ResultSetForQueryFactory(
        const uno::Reference< uno::XComponentContext >& rxContext,
        const uno::Reference< ucb::XContentProvider >&  xProvider,
        const uno::Sequence< beans::Property >& seq,
        const uno::Sequence< ucb::NumberedSortingInfo >& seqSort,
        const URLParameter& rURLParameter,
        Databases* pDatabases )
        : m_xContext( rxContext ),
          m_xProvider( xProvider ),
          m_seq( seq ),
          m_seqSort( seqSort ),
          m_aURLParameter( rURLParameter ),
          m_pDatabases( pDatabases )
    {
    }

    ResultSetBase* createResultSet() override
    {
        return new ResultSetForQuery( m_xContext,
                                      m_xProvider,
                                      m_seq,
                                      m_seqSort,
                                      m_aURLParameter,
                                      m_pDatabases );
    }
};

// virtual
uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 CommandId,
        const uno::Reference< ucb::XCommandEnvironment >& Environment )
    throw( uno::Exception,
           ucb::CommandAbortedException,
           uno::RuntimeException, std::exception )
{
    uno::Any aRet;

    if ( aCommand.Name == "getPropertyValues" )
    {
        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            aRet <<= lang::IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        aRet <<= getPropertyValues( Properties );
    }
    else if ( aCommand.Name == "setPropertyValues" )
    {
        uno::Sequence<beans::PropertyValue> propertyValues;

        if( ! ( aCommand.Argument >>= propertyValues ) ) {
            aRet <<= lang::IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        uno::Sequence< uno::Any > ret(propertyValues.getLength());
        uno::Sequence< beans::Property > props(getProperties(Environment));
        // No properties can be set
        for(sal_Int32 i = 0; i < ret.getLength(); ++i) {
            ret[i] <<= beans::UnknownPropertyException();
            for(sal_Int32 j = 0; j < props.getLength(); ++j)
                if(props[j].Name == propertyValues[i].Name) {
                    ret[i] <<= lang::IllegalAccessException();
                    break;
                }
        }

        aRet <<= ret;
    }
    else if ( aCommand.Name == "getPropertySetInfo" )
    {
        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name == "getCommandInfo" )
    {
        // Note: Implemented by base class.
        aRet <<= getCommandInfo( Environment );
    }
    else if ( aCommand.Name == "open" )
    {
        ucb::OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            aRet <<= lang::IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        uno::Reference< io::XActiveDataSink > xActiveDataSink(
            aOpenCommand.Sink, uno::UNO_QUERY);

        if(xActiveDataSink.is())
            m_aURLParameter.open(aCommand,
                                 CommandId,
                                 Environment,
                                 xActiveDataSink);

        uno::Reference< io::XActiveDataStreamer > xActiveDataStreamer(
            aOpenCommand.Sink, uno::UNO_QUERY);

        if(xActiveDataStreamer.is()) {
            aRet <<= ucb::UnsupportedDataSinkException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        uno::Reference< io::XOutputStream > xOutputStream(
            aOpenCommand.Sink, uno::UNO_QUERY);

        if(xOutputStream.is() )
            m_aURLParameter.open(aCommand,
                                 CommandId,
                                 Environment,
                                 xOutputStream);

        if( m_aURLParameter.isRoot() )
        {
            uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet(
                    m_xContext,
                    this,
                    aOpenCommand,
                    Environment,
                    new ResultSetForRootFactory(
                        m_xContext,
                        m_xProvider.get(),
                        aOpenCommand.Properties,
                        aOpenCommand.SortingInfo,
                        m_aURLParameter,
                        m_pDatabases));
            aRet <<= xSet;
        }
        else if( m_aURLParameter.isQuery() )
        {
            uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet(
                    m_xContext,
                    this,
                    aOpenCommand,
                    Environment,
                    new ResultSetForQueryFactory(
                        m_xContext,
                        m_xProvider.get(),
                        aOpenCommand.Properties,
                        aOpenCommand.SortingInfo,
                        m_aURLParameter,
                        m_pDatabases ) );
            aRet <<= xSet;
        }
    }
    else
    {
        // Unsupported command
        aRet <<= ucb::UnsupportedCommandException();
        ucbhelper::cancelCommandExecution(aRet,Environment);
    }

    return aRet;
}

uno::Reference< sdbc::XRow > Content::getPropertyValues(
    const uno::Sequence< beans::Property >& rProperties )
{
    osl::MutexGuard aGuard( m_aMutex );

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow =
        new ::ucbhelper::PropertyValueSet( m_xContext );

    for ( sal_Int32 n = 0; n < rProperties.getLength(); ++n )
    {
        const beans::Property& rProp = rProperties[n];

        if ( rProp.Name == "ContentType" )
            xRow->appendString(
                rProp,
                OUString(
                    "application/vnd.sun.star.help" ) );
        else if ( rProp.Name == "Title" )
            xRow->appendString ( rProp,m_aURLParameter.get_title() );
        else if ( rProp.Name == "IsReadOnly" )
            xRow->appendBoolean( rProp,true );
        else if ( rProp.Name == "IsDocument" )
            xRow->appendBoolean(
                rProp,
                m_aURLParameter.isFile() || m_aURLParameter.isRoot() );
        else if ( rProp.Name == "IsFolder" )
            xRow->appendBoolean(
                rProp,
                ! m_aURLParameter.isFile() || m_aURLParameter.isRoot() );
        else if ( rProp.Name == "IsErrorDocument" )
            xRow->appendBoolean( rProp, m_aURLParameter.isErrorDocument() );
        else if ( rProp.Name == "MediaType" )
            if( m_aURLParameter.isActive() )
                xRow->appendString(
                    rProp,
                    OUString( "text/plain" ) );
            else if( m_aURLParameter.isFile() )
                xRow->appendString(
                    rProp,OUString( "text/html" ) );
            else if( m_aURLParameter.isRoot() )
                xRow->appendString(
                    rProp,
                    OUString( "text/css" ) );
            else
                xRow->appendVoid( rProp );
        else if( m_aURLParameter.isModule() )
            if ( rProp.Name == "KeywordList" )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                uno::Any aAny;
                if( inf )
                    aAny <<= inf->getKeywordList();
                xRow->appendObject( rProp,aAny );
            }
            else if ( rProp.Name == "KeywordRef" )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                uno::Any aAny;
                if( inf )
                    aAny <<= inf->getIdList();
                xRow->appendObject( rProp,aAny );
            }
            else if ( rProp.Name == "KeywordAnchorForRef" )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                uno::Any aAny;
                if( inf )
                    aAny <<= inf->getAnchorList();
                xRow->appendObject( rProp,aAny );
            }
            else if ( rProp.Name == "KeywordTitleForRef" )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                uno::Any aAny;
                if( inf )
                    aAny <<= inf->getTitleList();
                xRow->appendObject( rProp,aAny );
            }
            else if ( rProp.Name == "SearchScopes" )
            {
                uno::Sequence< OUString > seq( 2 );
                seq[0] = "Heading";
                seq[1] = "FullText";
                xRow->appendObject( rProp, uno::Any(seq) );
            }
            else if ( rProp.Name == "Order" )
            {
                StaticModuleInformation *inf =
                    m_pDatabases->getStaticInformationForModule(
                        m_aURLParameter.get_module(),
                        m_aURLParameter.get_language() );

                uno::Any aAny;
                if( inf )
                    aAny <<= sal_Int32( inf->get_order() );
                xRow->appendObject( rProp,aAny );
            }
            else
                xRow->appendVoid( rProp );
        else if( "AnchorName" == rProp.Name  &&
                 m_aURLParameter.isFile() )
            xRow->appendString( rProp,m_aURLParameter.get_tag() );
        else
            xRow->appendVoid( rProp );
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
