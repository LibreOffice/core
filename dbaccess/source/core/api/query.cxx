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
#include "precompiled_dbaccess.hxx"

#ifndef _DBA_COREAPI_QUERY_HXX_
#include "query.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef DBTOOLS_WARNINGSCONTAINER_HXX
#include <connectivity/warningscontainer.hxx>
#endif
#ifndef DBA_HELPERCOLLECTIONS_HXX
#include "HelperCollections.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _COMPHELPER_PROPERTY_AGGREGATION_HXX_
#include <comphelper/propagg.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
/** === end UNO includes === **/

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef UNOTOOLS_INC_SHAREDUNOCOMPONENT_HXX
#include <unotools/sharedunocomponent.hxx>
#endif
#ifndef _DBACORE_DEFINITIONCOLUMN_HXX_
#include "definitioncolumn.hxx"
#endif

#include <functional>

#ifndef DBACORE_SDBCORETOOLS_HXX
#include "sdbcoretools.hxx"
#endif
#ifndef DBACCESS_CORE_API_QUERYCOMPOSER_HXX
#include "querycomposer.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef DBA_CONTAINERMEDIATOR_HXX
#include "ContainerMediator.hxx"
#endif

using namespace dbaccess;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::comphelper;
using namespace ::osl;
using namespace ::cppu;
using namespace ::utl;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OQuery
//==========================================================================
DBG_NAME(OQuery)
//--------------------------------------------------------------------------
OQuery::OQuery( const Reference< XPropertySet >& _rxCommandDefinition
               ,const Reference< XConnection >& _rxConn
               ,const Reference< XMultiServiceFactory >& _xORB)
    :OContentHelper(_xORB,NULL,TContentPtr(new OContentHelper_Impl))
    ,OQueryDescriptor_Base(m_aMutex,*this)
    ,ODataSettings(OContentHelper::rBHelper,sal_True)
    ,m_xCommandDefinition(_rxCommandDefinition)
    ,m_xConnection(_rxConn)
    ,m_pColumnMediator( NULL )
    ,m_pWarnings( NULL )
    ,m_bCaseSensitiv(sal_True)
    ,m_eDoingCurrently(NONE)
{
    DBG_CTOR(OQuery, NULL);
    registerProperties();
    ODataSettings::registerPropertiesFor(this);

    osl_incrementInterlockedCount(&m_refCount);
    DBG_ASSERT(m_xCommandDefinition.is(), "OQuery::OQuery : invalid CommandDefinition object !");
    if ( m_xCommandDefinition.is() )
    {
        try
        {
            ::comphelper::copyProperties(_rxCommandDefinition,this);
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OQueryDescriptor_Base::OQueryDescriptor_Base: caught an exception!");
        }

        m_xCommandDefinition->addPropertyChangeListener(::rtl::OUString(), this);
        //  m_xCommandDefinition->addPropertyChangeListener(PROPERTY_NAME, this);
        m_xCommandPropInfo = m_xCommandDefinition->getPropertySetInfo();
    }
    DBG_ASSERT(m_xConnection.is(), "OQuery::OQuery : invalid connection !");
    osl_decrementInterlockedCount(&m_refCount);
}

//--------------------------------------------------------------------------
OQuery::~OQuery()
{
    DBG_DTOR(OQuery, NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_IMPLEMENTATION_ID(OQuery);
IMPLEMENT_GETTYPES3(OQuery,OQueryDescriptor_Base,ODataSettings,OContentHelper);
IMPLEMENT_FORWARD_XINTERFACE3( OQuery,OContentHelper,OQueryDescriptor_Base,ODataSettings)
//--------------------------------------------------------------------------
void OQuery::rebuildColumns()
{
    OSL_PRECOND( getColumnCount() == 0, "OQuery::rebuildColumns: column container should be empty!" );
        // the base class' definition of rebuildColumns promised that clearColumns is called before rebuildColumns

    try
    {
        m_pColumnMediator = NULL;

        Reference<XColumnsSupplier> xColSup(m_xCommandDefinition,UNO_QUERY);
        Reference< XNameAccess > xColumnDefinitions;
        if ( xColSup.is() )
        {
            xColumnDefinitions = xColSup->getColumns();
            if ( xColumnDefinitions.is() )
                m_pColumnMediator = new OContainerMediator( m_pColumns, xColumnDefinitions, m_xConnection, OContainerMediator::eColumns );
        }

        // fill the columns with columns from the statement
        Reference< XMultiServiceFactory > xFactory( m_xConnection, UNO_QUERY_THROW );
        SharedUNOComponent< XSingleSelectQueryComposer, DisposableComponent > xComposer(
            Reference< XSingleSelectQueryComposer >( xFactory->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW ) );

        Reference< XNameAccess > xColumns;
        Reference< XIndexAccess > xColumnsIndexed;
        try
        {
            xComposer->setQuery( m_sCommand );
            Reference< XColumnsSupplier > xCols( xComposer, UNO_QUERY_THROW );
            xColumns.set( xCols->getColumns(), UNO_QUERY_THROW );
            xColumnsIndexed.set( xColumns, UNO_QUERY_THROW );
        }
        catch( const SQLException& ) { }

        SharedUNOComponent< XPreparedStatement, DisposableComponent > xPreparedStatement;
        if ( !xColumns.is() || ( xColumnsIndexed->getCount() == 0 ) )
        {   // the QueryComposer could not parse it. Try a lean version.
            xPreparedStatement.set( m_xConnection->prepareStatement( m_sCommand ), UNO_QUERY_THROW );
            Reference< XResultSetMetaDataSupplier > xResMetaDataSup( xPreparedStatement, UNO_QUERY_THROW );
            Reference< XResultSetMetaData > xResultSetMeta( xResMetaDataSup->getMetaData() );
            if ( !xResultSetMeta.is() )
            {
                ::rtl::OUString sError( DBA_RES( RID_STR_STATEMENT_WITHOUT_RESULT_SET ) );
                ::dbtools::throwSQLException( sError, SQL_GENERAL_ERROR, *this );
            }

            Reference< XDatabaseMetaData > xDBMeta( m_xConnection->getMetaData(), UNO_QUERY_THROW );
            ::vos::ORef< OSQLColumns > aParseColumns(
                ::connectivity::parse::OParseColumn::createColumnsForResultSet( xResultSetMeta, xDBMeta,xColumnDefinitions ) );
            xColumns = OPrivateColumns::createWithIntrinsicNames(
                aParseColumns, xDBMeta->supportsMixedCaseQuotedIdentifiers(), *this, m_aMutex );
            if ( !xColumns.is() )
                throw RuntimeException();
        }

        Sequence< ::rtl::OUString> aNames = xColumns->getElementNames();
        const ::rtl::OUString* pIter = aNames.getConstArray();
        const ::rtl::OUString* pEnd  = pIter + aNames.getLength();
        for ( sal_Int32 i = 0;pIter != pEnd; ++pIter,++i)
        {
            Reference<XPropertySet> xSource(xColumns->getByName( *pIter ),UNO_QUERY);
            ::rtl::OUString sLabel = *pIter;
            if ( xColumnDefinitions.is() && xColumnDefinitions->hasByName(*pIter) )
            {
                Reference<XPropertySet> xCommandColumn(xColumnDefinitions->getByName( *pIter ),UNO_QUERY);
                xCommandColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
            }
            OQueryColumn* pColumn = new OQueryColumn( xSource, m_xConnection, sLabel);
            Reference< XChild > xChild( *pColumn, UNO_QUERY_THROW );
            xChild->setParent( *this );

            implAppendColumn( *pIter, pColumn );
            Reference< XPropertySet > xDest( *pColumn, UNO_QUERY_THROW );
            if ( m_pColumnMediator.is() )
                m_pColumnMediator->notifyElementCreated( *pIter, xDest );
        }
    }
    catch( const SQLContext& e )
    {
        if ( m_pWarnings )
            m_pWarnings->appendWarning( e );
    }
    catch( const SQLWarning& e )
    {
        if ( m_pWarnings )
            m_pWarnings->appendWarning( e );
    }
    catch( const SQLException& e )
    {
        if ( m_pWarnings )
            m_pWarnings->appendWarning( e );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// XServiceInfo
//--------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO3(OQuery, "com.sun.star.sdb.dbaccess.OQuery", SERVICE_SDB_DATASETTINGS, SERVICE_SDB_QUERY, SERVICE_SDB_QUERYDEFINITION)

// ::com::sun::star::beans::XPropertyChangeListener
//--------------------------------------------------------------------------
void SAL_CALL OQuery::propertyChange( const PropertyChangeEvent& _rSource ) throw(RuntimeException)
{
    sal_Int32 nOwnHandle = -1;
    {
        MutexGuard aGuard(m_aMutex);

        DBG_ASSERT(_rSource.Source.get() == Reference< XInterface >(m_xCommandDefinition, UNO_QUERY).get(),
            "OQuery::propertyChange : where did this call come from ?");

        if (m_eDoingCurrently == SETTING_PROPERTIES)
            // we're setting the property ourself, so we will do the neccessary notifications later
            return;

        // forward this to our own member holding a copy of the property value
        if (getArrayHelper()->hasPropertyByName(_rSource.PropertyName))
        {
            Property aOwnProp = getArrayHelper()->getPropertyByName(_rSource.PropertyName);
            nOwnHandle = aOwnProp.Handle;
            ODataSettings::setFastPropertyValue_NoBroadcast(nOwnHandle, _rSource.NewValue);
                // don't use our own setFastPropertyValue_NoBroadcast, this would forward it to the CommandSettings,
                // again
                // and don't use the "real" setPropertyValue, this is to expensive and not sure to succeed
        }
        else
        {
            DBG_ERROR("OQuery::propertyChange : my CommandDefinition has more properties than I do !");
        }
    }

    fire(&nOwnHandle, &_rSource.NewValue, &_rSource.OldValue, 1, sal_False);
}

//--------------------------------------------------------------------------
void SAL_CALL OQuery::disposing( const EventObject& _rSource ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    (void)_rSource;
    DBG_ASSERT(_rSource.Source.get() == Reference< XInterface >(m_xCommandDefinition, UNO_QUERY).get(),
        "OQuery::disposing : where did this call come from ?");

    m_xCommandDefinition->removePropertyChangeListener(::rtl::OUString(), this);
    m_xCommandDefinition = NULL;
}

// XDataDescriptorFactory
//--------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OQuery::createDataDescriptor(  ) throw(RuntimeException)
{
    return new OQueryDescriptor(*this);
}

// pseudo-XComponent
//--------------------------------------------------------------------------
void SAL_CALL OQuery::disposing()
{
    MutexGuard aGuard(m_aMutex);
    if (m_xCommandDefinition.is())
    {
        m_xCommandDefinition->removePropertyChangeListener(::rtl::OUString(), this);
        m_xCommandDefinition = NULL;
    }
    disposeColumns();

    m_pWarnings = NULL;
}

//--------------------------------------------------------------------------
void OQuery::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw (Exception)
{
    ODataSettings::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    ::rtl::OUString sAggPropName;
    sal_Int16 nAttr = 0;
    if (getInfoHelper().fillPropertyMembersByHandle(&sAggPropName,&nAttr,_nHandle) &&
        m_xCommandPropInfo.is() &&
        m_xCommandPropInfo->hasPropertyByName(sAggPropName))
    {   // the base class holds the property values itself, but we have to forward this to our CommandDefinition

        m_eDoingCurrently = SETTING_PROPERTIES;
        OAutoActionReset aActionResetter(this);
        m_xCommandDefinition->setPropertyValue(sAggPropName, _rValue);

        if ( PROPERTY_ID_COMMAND == _nHandle )
            // the columns are out of date if we are based on a new statement ....
            // 90573 - 16.08.2001 - frank.schoenheit@sun.com
            setColumnsOutOfDate();
    }
}

//--------------------------------------------------------------------------
Reference< XPropertySetInfo > SAL_CALL OQuery::getPropertySetInfo(  ) throw(RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OQuery::getInfoHelper()
{
    return *getArrayHelper();
}

//--------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OQuery::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    // our own props
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -----------------------------------------------------------------------------
OColumn* OQuery::createColumn(const ::rtl::OUString& /*_rName*/) const
{
    return NULL;
}
// -----------------------------------------------------------------------------
void SAL_CALL OQuery::rename( const ::rtl::OUString& newName ) throw (SQLException, ElementExistException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    Reference<XRename> xRename(m_xCommandDefinition,UNO_QUERY);
    OSL_ENSURE(xRename.is(),"No XRename interface!");
    if(xRename.is())
        xRename->rename(newName);
}
// -----------------------------------------------------------------------------
void OQuery::registerProperties()
{
    // the properties which OCommandBase supplies (it has no own registration, as it's not derived from
    // a OPropertyStateContainer)
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND|PropertyAttribute::CONSTRAINED,
                    &m_sElementName, ::getCppuType(&m_sElementName));

    registerProperty(PROPERTY_COMMAND, PROPERTY_ID_COMMAND, PropertyAttribute::BOUND,
                    &m_sCommand, ::getCppuType(&m_sCommand));

    registerProperty(PROPERTY_ESCAPE_PROCESSING, PROPERTY_ID_ESCAPE_PROCESSING, PropertyAttribute::BOUND,
                    &m_bEscapeProcessing, ::getBooleanCppuType());

    registerProperty(PROPERTY_UPDATE_TABLENAME, PROPERTY_ID_UPDATE_TABLENAME, PropertyAttribute::BOUND,
                    &m_sUpdateTableName, ::getCppuType(&m_sUpdateTableName));

    registerProperty(PROPERTY_UPDATE_SCHEMANAME, PROPERTY_ID_UPDATE_SCHEMANAME, PropertyAttribute::BOUND,
                    &m_sUpdateSchemaName, ::getCppuType(&m_sUpdateSchemaName));

    registerProperty(PROPERTY_UPDATE_CATALOGNAME, PROPERTY_ID_UPDATE_CATALOGNAME, PropertyAttribute::BOUND,
                    &m_sUpdateCatalogName, ::getCppuType(&m_sUpdateCatalogName));

    registerProperty(PROPERTY_LAYOUTINFORMATION, PROPERTY_ID_LAYOUTINFORMATION, PropertyAttribute::BOUND,
                    &m_aLayoutInformation, ::getCppuType(&m_aLayoutInformation));
}

// -----------------------------------------------------------------------------
::rtl::OUString OQuery::determineContentType() const
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.org.openoffice.DatabaseQuery" ) );
}

// -----------------------------------------------------------------------------
//........................................................................
}   // namespace dbaccess
//........................................................................

