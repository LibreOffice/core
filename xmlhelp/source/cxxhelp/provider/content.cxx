/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: content.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:38:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALACCESSEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalAccessException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDDATASINKEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASTREAMER_HPP_
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPERSISTENTPROPERTYSET_HPP_
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROPERTYVALUESET_HXX
#include <ucbhelper/propertyvalueset.hxx>
#endif
#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX
#include <ucbhelper/cancelcommandexecution.hxx>
#endif
#ifndef _CONTENT_HXX
#include <provider/content.hxx>
#endif
#ifndef _PROVIDER_HXX
#include <provider/provider.hxx>
#endif
#ifndef _RESULTSET_HXX
#include <provider/resultset.hxx>
#endif
#ifndef _DATABASES_HXX_
#include <provider/databases.hxx>
#endif
#ifndef _RESULTSETFACTORY_HXX
#include <provider/resultsetfactory.hxx>
#endif
#ifndef _RESULTSETBASE_HXX
#include <provider/resultsetbase.hxx>
#endif
#ifndef _RESULTSETFORROOT_HXX
#include <provider/resultsetforroot.hxx>
#endif
#ifndef _RESULTSETFORQUERY_HXX
#include <provider/resultsetforquery.hxx>
#endif

using namespace com::sun::star::io;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;

using namespace chelp;

//=========================================================================
//=========================================================================
//
// Content Implementation.
//
//=========================================================================
//=========================================================================

Content::Content( const Reference< XMultiServiceFactory >& rxSMgr,
                  ::ucb::ContentProviderImplHelper* pProvider,
                  const Reference< XContentIdentifier >& Identifier,
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
Any SAL_CALL Content::queryInterface( const Type & rType )
    throw ( RuntimeException )
{
    Any aRet;
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
Sequence< Type > SAL_CALL Content::getTypes()
    throw( RuntimeException )
{
    static OTypeCollection* pCollection = NULL;

    if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
          if ( !pCollection )
          {
              static OTypeCollection aCollection(
                CPPU_TYPE_REF( XTypeProvider ),
                   CPPU_TYPE_REF( XServiceInfo ),
                   CPPU_TYPE_REF( XComponent ),
                   CPPU_TYPE_REF( XContent ),
                   CPPU_TYPE_REF( XCommandProcessor ),
                   CPPU_TYPE_REF( XPropertiesChangeNotifier ),
                   CPPU_TYPE_REF( XCommandInfoChangeNotifier ),
                   CPPU_TYPE_REF( XPropertyContainer ),
                   CPPU_TYPE_REF( XPropertySetInfoChangeNotifier ),
                   CPPU_TYPE_REF( XChild ) );
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
OUString SAL_CALL Content::getImplementationName()
    throw( RuntimeException )
{
    return OUString::createFromAscii( "CHelpContent" );
}

//=========================================================================
// virtual
Sequence< OUString > SAL_CALL Content::getSupportedServiceNames()
    throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ]
            = OUString::createFromAscii( MYUCP_CONTENT_SERVICE_NAME );
    return aSNS;
}

//=========================================================================
//
// XContent methods.
//
//=========================================================================

// virtual
OUString SAL_CALL Content::getContentType()
    throw( RuntimeException )
{
    return OUString::createFromAscii( MYUCP_CONTENT_TYPE );
}

//=========================================================================
//
// XCommandProcessor methods.
//
//=========================================================================

//virtual
void SAL_CALL Content::abort( sal_Int32 CommandId )
    throw( RuntimeException )
{
    (void)CommandId;
}



class ResultSetForRootFactory
    : public ResultSetFactory
{
private:

    Reference< XMultiServiceFactory >               m_xSMgr;
    Reference< XContentProvider >                   m_xProvider;
    sal_Int32                                       m_nOpenMode;
    Sequence< Property >                            m_seq;
    Sequence< NumberedSortingInfo >                 m_seqSort;
    URLParameter                                    m_aURLParameter;
    Databases*                                      m_pDatabases;


public:

    ResultSetForRootFactory( const Reference< XMultiServiceFactory >& xSMgr,
                             const Reference< XContentProvider >&  xProvider,
                             sal_Int32 nOpenMode,
                             const Sequence< Property >& seq,
                             const Sequence< NumberedSortingInfo >& seqSort,
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

    Reference< XMultiServiceFactory >               m_xSMgr;
    Reference< XContentProvider >                   m_xProvider;
    sal_Int32                                       m_nOpenMode;
    Sequence< Property >                            m_seq;
    Sequence< NumberedSortingInfo >                 m_seqSort;
    URLParameter                                    m_aURLParameter;
    Databases*                                      m_pDatabases;


public:

    ResultSetForQueryFactory( const Reference< XMultiServiceFactory >& xSMgr,
                              const Reference< XContentProvider >&  xProvider,
                              sal_Int32 nOpenMode,
                              const Sequence< Property >& seq,
                              const Sequence< NumberedSortingInfo >& seqSort,
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
Any SAL_CALL Content::execute( const Command& aCommand,
                               sal_Int32 CommandId,
                               const Reference<
                               XCommandEnvironment >& Environment )
    throw( Exception, CommandAbortedException, RuntimeException )
{
    Any aRet;

    if ( aCommand.Name.compareToAscii( "getPropertyValues" ) == 0 )
    {
        Sequence< Property > Properties;
        if ( !( aCommand.Argument >>= Properties ) )
        {
            aRet <<= IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        aRet <<= getPropertyValues( Properties );
    }
    else if ( aCommand.Name.compareToAscii( "setPropertyValues" ) == 0 )
    {
        Sequence<PropertyValue> propertyValues;

        if( ! ( aCommand.Argument >>= propertyValues ) ) {
            aRet <<= IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        Sequence<Any> ret(propertyValues.getLength());
        Sequence< Property > props(getProperties(Environment));
        // No properties can be set
        for(sal_Int32 i = 0; i < ret.getLength(); ++i) {
            ret[i] <<= UnknownPropertyException();
            for(sal_Int32 j = 0; j < props.getLength(); ++j)
                if(props[j].Name == propertyValues[i].Name) {
                    ret[i] <<= IllegalAccessException();
                    break;
                }
        }

        aRet <<= ret;
    }
    else if ( aCommand.Name.compareToAscii( "getPropertySetInfo" ) == 0 )
    {
        // Note: Implemented by base class.
        aRet <<= getPropertySetInfo( Environment );
    }
    else if ( aCommand.Name.compareToAscii( "getCommandInfo" ) == 0 )
    {
        // Note: Implemented by base class.
        aRet <<= getCommandInfo( Environment );
    }
    else if ( aCommand.Name.compareToAscii( "open" ) == 0 )
    {
        OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            aRet <<= IllegalArgumentException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        Reference< XActiveDataSink > xActiveDataSink(
            aOpenCommand.Sink,UNO_QUERY);

        if(xActiveDataSink.is())
            m_aURLParameter.open(m_xSMgr,
                                 aCommand,
                                 CommandId,
                                 Environment,
                                 xActiveDataSink);

        Reference< XActiveDataStreamer > xActiveDataStreamer(
            aOpenCommand.Sink,UNO_QUERY);

        if(xActiveDataStreamer.is()) {
            aRet <<= UnsupportedDataSinkException();
            ucbhelper::cancelCommandExecution(aRet,Environment);
        }

        Reference< XOutputStream > xOutputStream(
            aOpenCommand.Sink,UNO_QUERY);

        if(xOutputStream.is() )
            m_aURLParameter.open(m_xSMgr,
                                 aCommand,
                                 CommandId,
                                 Environment,
                                 xOutputStream);

        if( m_aURLParameter.isRoot() )
        {
            Reference< XDynamicResultSet > xSet
                = new DynamicResultSet(
                    m_xSMgr,
                    this,
                    aOpenCommand,
                    Environment,
                    new ResultSetForRootFactory(
                        m_xSMgr,
                        m_xProvider.getBodyPtr(),
                        aOpenCommand.Mode,
                        aOpenCommand.Properties,
                        aOpenCommand.SortingInfo,
                        m_aURLParameter,
                        m_pDatabases));
            aRet <<= xSet;
        }
        else if( m_aURLParameter.isQuery() )
        {
            Reference< XDynamicResultSet > xSet
                = new DynamicResultSet(
                    m_xSMgr,
                    this,
                    aOpenCommand,
                    Environment,
                    new ResultSetForQueryFactory(
                        m_xSMgr,
                        m_xProvider.getBodyPtr(),
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
        aRet <<= UnsupportedCommandException();
        ucbhelper::cancelCommandExecution(aRet,Environment);
    }

    return aRet;
}




//=========================================================================
Reference< XRow > Content::getPropertyValues( const Sequence< Property >& rProperties )
{
    osl::MutexGuard aGuard( m_aMutex );

    vos::ORef< ::ucb::PropertyValueSet > xRow =
        new ::ucb::PropertyValueSet( m_xSMgr );

    for ( sal_Int32 n = 0; n < rProperties.getLength(); ++n )
    {
        const Property& rProp = rProperties[n];

        if ( rProp.Name.compareToAscii( "ContentType" ) == 0 )
            xRow->appendString(
                rProp,
                rtl::OUString::createFromAscii(
                    "application/vnd.sun.star.help" ) );
        else if( rProp.Name.compareToAscii( "Title" ) == 0 )
            xRow->appendString ( rProp,m_aURLParameter.get_title() );
        else if( rProp.Name.compareToAscii( "IsReadOnly" ) == 0 )
            xRow->appendBoolean( rProp,true );
        else if( rProp.Name.compareToAscii( "IsDocument" ) == 0 )
            xRow->appendBoolean(
                rProp,
                m_aURLParameter.isFile() || m_aURLParameter.isRoot() );
        else if( rProp.Name.compareToAscii( "IsFolder" ) == 0 )
            xRow->appendBoolean(
                rProp,
                ! m_aURLParameter.isFile() || m_aURLParameter.isRoot() );
        else if( rProp.Name.compareToAscii( "IsErrorDocument" ) == 0 )
            xRow->appendBoolean( rProp, m_aURLParameter.isErrorDocument() );
        else if( rProp.Name.compareToAscii( "MediaType" ) == 0  )
            if( m_aURLParameter.isPicture() )
                xRow->appendString(
                    rProp,
                    rtl::OUString::createFromAscii( "image/gif" ) );
            else if( m_aURLParameter.isActive() )
                xRow->appendString(
                    rProp,
                    rtl::OUString::createFromAscii( "text/plain" ) );
            else if( m_aURLParameter.isFile() )
                xRow->appendString(
                    rProp,rtl::OUString::createFromAscii( "text/html" ) );
            else if( m_aURLParameter.isRoot() )
                xRow->appendString(
                    rProp,
                    rtl::OUString::createFromAscii( "text/css" ) );
            else
                xRow->appendVoid( rProp );
        else if( m_aURLParameter.isModule() )
            if( rProp.Name.compareToAscii( "KeywordList" ) == 0 )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                Any aAny;
                if( inf )
                    aAny <<= inf->getKeywordList();
                xRow->appendObject( rProp,aAny );
            }
            else if( rProp.Name.compareToAscii( "KeywordRef" ) == 0 )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                Any aAny;
                if( inf )
                    aAny <<= inf->getIdList();
                xRow->appendObject( rProp,aAny );
            }
            else if( rProp.Name.compareToAscii( "KeywordAnchorForRef" ) == 0 )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                Any aAny;
                if( inf )
                    aAny <<= inf->getAnchorList();
                xRow->appendObject( rProp,aAny );
            }
            else if( rProp.Name.compareToAscii( "KeywordTitleForRef" ) == 0 )
            {
                KeywordInfo *inf =
                    m_pDatabases->getKeyword( m_aURLParameter.get_module(),
                                              m_aURLParameter.get_language() );

                Any aAny;
                if( inf )
                    aAny <<= inf->getTitleList();
                xRow->appendObject( rProp,aAny );
            }
            else if( rProp.Name.compareToAscii( "SearchScopes" ) == 0 )
            {
                Sequence< rtl::OUString > seq( 2 );
                seq[0] = rtl::OUString::createFromAscii( "Heading" );
                seq[1] = rtl::OUString::createFromAscii( "FullText" );
                Any aAny;
                aAny <<= seq;
                xRow->appendObject( rProp,aAny );
            }
            else if( rProp.Name.compareToAscii( "Order" ) == 0 )
            {
                StaticModuleInformation *inf =
                    m_pDatabases->getStaticInformationForModule(
                        m_aURLParameter.get_module(),
                        m_aURLParameter.get_language() );

                Any aAny;
                if( inf )
                    aAny <<= sal_Int32( inf->get_order() );
                xRow->appendObject( rProp,aAny );
            }
            else
                xRow->appendVoid( rProp );
        else if( rProp.Name.compareToAscii( "AnchorName" ) == 0 &&
                 m_aURLParameter.isFile() )
            xRow->appendString( rProp,m_aURLParameter.get_tag() );
        else
            xRow->appendVoid( rProp );
    }

    return Reference< XRow >( xRow.getBodyPtr() );
}
