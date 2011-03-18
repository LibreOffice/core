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
#include "precompiled_xmlhelp.hxx"

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

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

Content::Content( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
                  ::ucbhelper::ContentProviderImplHelper* pProvider,
                  const uno::Reference< ucb::XContentIdentifier >&
                      Identifier,
                  Databases* pDatabases )
    : ContentImplHelper( rxSMgr, pProvider, Identifier ),
      m_aURLParameter( Identifier->getContentIdentifier(),pDatabases ),
      m_pDatabases( pDatabases ) // not owner
{
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
    static cppu::OTypeCollection* pCollection = NULL;

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

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// virtual
rtl::OUString SAL_CALL Content::getImplementationName()
    throw( uno::RuntimeException )
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "CHelpContent" ));
}

//=========================================================================
// virtual
uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
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
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( MYUCP_CONTENT_TYPE ));
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

//virtual
void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ )
    throw( uno::RuntimeException )
{
}



class ResultSetForRootFactory
    : public ResultSetFactory
{
private:

    uno::Reference< lang::XMultiServiceFactory > m_xSMgr;
    uno::Reference< ucb::XContentProvider >      m_xProvider;
    sal_Int32                                    m_nOpenMode;
    uno::Sequence< beans::Property >             m_seq;
    uno::Sequence< ucb::NumberedSortingInfo >    m_seqSort;
    URLParameter                                 m_aURLParameter;
    Databases*                                   m_pDatabases;


public:

    ResultSetForRootFactory(
        const uno::Reference< lang::XMultiServiceFactory >& xSMgr,
        const uno::Reference< ucb::XContentProvider >&  xProvider,
        sal_Int32 nOpenMode,
        const uno::Sequence< beans::Property >& seq,
        const uno::Sequence< ucb::NumberedSortingInfo >& seqSort,
        URLParameter aURLParameter,
        Databases* pDatabases )
        : m_xSMgr( xSMgr ),
          m_xProvider( xProvider ),
          m_nOpenMode( nOpenMode ),
          m_seq( seq ),
          m_seqSort( seqSort ),
          m_aURLParameter( aURLParameter ),
          m_pDatabases( pDatabases )
    {
    }

    ResultSetBase* createResultSet()
    {
        return new ResultSetForRoot( m_xSMgr,
                                     m_xProvider,
                                     m_nOpenMode,
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

    uno::Reference< lang::XMultiServiceFactory > m_xSMgr;
    uno::Reference< ucb::XContentProvider >      m_xProvider;
    sal_Int32                                    m_nOpenMode;
    uno::Sequence< beans::Property >             m_seq;
    uno::Sequence< ucb::NumberedSortingInfo >    m_seqSort;
    URLParameter                                 m_aURLParameter;
    Databases*                                   m_pDatabases;


public:

    ResultSetForQueryFactory(
        const uno::Reference< lang::XMultiServiceFactory >& xSMgr,
        const uno::Reference< ucb::XContentProvider >&  xProvider,
        sal_Int32 nOpenMode,
        const uno::Sequence< beans::Property >& seq,
        const uno::Sequence< ucb::NumberedSortingInfo >& seqSort,
        URLParameter aURLParameter,
        Databases* pDatabases )
        : m_xSMgr( xSMgr ),
          m_xProvider( xProvider ),
          m_nOpenMode( nOpenMode ),
          m_seq( seq ),
          m_seqSort( seqSort ),
          m_aURLParameter( aURLParameter ),
          m_pDatabases( pDatabases )
    {
    }

    ResultSetBase* createResultSet()
    {
        return new ResultSetForQuery( m_xSMgr,
                                      m_xProvider,
                                      m_nOpenMode,
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
           uno::RuntimeException )
{
    uno::Any aRet;

    if ( aCommand.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getPropertyValues")) )
    {
        uno::Sequence< beans::Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            aRet <<= lang::IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        aRet <<= getPropertyValues( Properties );
    }
    else if ( aCommand.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("setPropertyValues")) )
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
    else if ( aCommand.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getPropertySetInfo")) )
    {
        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("getCommandInfo")) )
    {
        // Note: Implemented by base class.
        aRet <<= getCommandInfo( Environment );
    }
    else if ( aCommand.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("open")) )
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
            m_aURLParameter.open(m_xSMgr,
                                 aCommand,
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
            m_aURLParameter.open(m_xSMgr,
                                 aCommand,
                                 CommandId,
                                 Environment,
                                 xOutputStream);

        if( m_aURLParameter.isRoot() )
        {
            uno::Reference< ucb::XDynamicResultSet > xSet
                = new DynamicResultSet(
                    m_xSMgr,
                    this,
                    aOpenCommand,
                    Environment,
                    new ResultSetForRootFactory(
                        m_xSMgr,
                        m_xProvider.get(),
                        aOpenCommand.Mode,
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
                    m_xSMgr,
                    this,
                    aOpenCommand,
                    Environment,
                    new ResultSetForQueryFactory(
                        m_xSMgr,
                        m_xProvider.get(),
                        aOpenCommand.Mode,
                        aOpenCommand.Properties,
                        aOpenCommand.SortingInfo,
                        m_aURLParameter,
                        m_pDatabases ) );
            aRet <<= xSet;
        }
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // Unsupported command
        //////////////////////////////////////////////////////////////////
        aRet <<= ucb::UnsupportedCommandException();
        ucbhelper::cancelCommandExecution(aRet,Environment);
    }

    return aRet;
}




//=========================================================================
uno::Reference< sdbc::XRow > Content::getPropertyValues(
    const uno::Sequence< beans::Property >& rProperties )
{
    osl::MutexGuard aGuard( m_aMutex );

    rtl::Reference< ::ucbhelper::PropertyValueSet > xRow =
        new ::ucbhelper::PropertyValueSet( m_xSMgr );

    for ( sal_Int32 n = 0; n < rProperties.getLength(); ++n )
    {
        const beans::Property& rProp = rProperties[n];

        if ( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ContentType")) )
            xRow->appendString(
                rProp,
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "application/vnd.sun.star.help" )) );
        else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Title")) )
            xRow->appendString ( rProp,m_aURLParameter.get_title() );
        else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("IsReadOnly")) )
            xRow->appendBoolean( rProp,true );
        else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("IsDocument")) )
            xRow->appendBoolean(
                rProp,
                m_aURLParameter.isFile() || m_aURLParameter.isRoot() );
        else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("IsFolder")) )
            xRow->appendBoolean(
                rProp,
                ! m_aURLParameter.isFile() || m_aURLParameter.isRoot() );
        else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("IsErrorDocument")) )
            xRow->appendBoolean( rProp, m_aURLParameter.isErrorDocument() );
        else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MediaType")) )
            if( m_aURLParameter.isPicture() )
                xRow->appendString(
                    rProp,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "image/gif" )) );
            else if( m_aURLParameter.isActive() )
                xRow->appendString(
                    rProp,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "text/plain" )) );
            else if( m_aURLParameter.isFile() )
                xRow->appendString(
                    rProp,rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "text/html" )) );
            else if( m_aURLParameter.isRoot() )
                xRow->appendString(
                    rProp,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "text/css" )) );
            else
                xRow->appendVoid( rProp );
        else if( m_aURLParameter.isModule() )
            if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("KeywordList")) )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                uno::Any aAny;
                if( inf )
                    aAny <<= inf->getKeywordList();
                xRow->appendObject( rProp,aAny );
            }
            else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("KeywordRef")) )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                uno::Any aAny;
                if( inf )
                    aAny <<= inf->getIdList();
                xRow->appendObject( rProp,aAny );
            }
            else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("KeywordAnchorForRef")) )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                uno::Any aAny;
                if( inf )
                    aAny <<= inf->getAnchorList();
                xRow->appendObject( rProp,aAny );
            }
            else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("KeywordTitleForRef")) )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                uno::Any aAny;
                if( inf )
                    aAny <<= inf->getTitleList();
                xRow->appendObject( rProp,aAny );
            }
            else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("SearchScopes")) )
            {
                uno::Sequence< rtl::OUString > seq( 2 );
                seq[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Heading" ));
                seq[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FullText" ));
                uno::Any aAny;
                aAny <<= seq;
                xRow->appendObject( rProp,aAny );
            }
            else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Order")) )
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
        else if( rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("AnchorName")) &&
                 m_aURLParameter.isFile() )
            xRow->appendString( rProp,m_aURLParameter.get_tag() );
        else
            xRow->appendVoid( rProp );
    }

    return uno::Reference< sdbc::XRow >( xRow.get() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
