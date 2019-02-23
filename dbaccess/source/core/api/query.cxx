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

#include "query.hxx"
#include <stringconstants.hxx>
#include <connectivity/warningscontainer.hxx>
#include "HelperCollections.hxx"
#include <core_resource.hxx>
#include <strings.hrc>

#include <cppuhelper/queryinterface.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>

#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>

#include <comphelper/types.hxx>
#include <comphelper/property.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <definitioncolumn.hxx>

#include <functional>

#include <sdbcoretools.hxx>
#include <querycomposer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <ContainerMediator.hxx>

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

namespace dbaccess
{

// OQuery

OQuery::OQuery( const Reference< XPropertySet >& _rxCommandDefinition
               ,const Reference< XConnection >& _rxConn
               ,const Reference< XComponentContext >& _xORB)
    :OContentHelper(_xORB,nullptr,std::make_shared<OContentHelper_Impl>())
    ,OQueryDescriptor_Base(m_aMutex,*this)
    ,ODataSettings(OContentHelper::rBHelper,true)
    ,m_xCommandDefinition(_rxCommandDefinition)
    ,m_xConnection(_rxConn)
    ,m_pWarnings( nullptr )
    ,m_eDoingCurrently(AggregateAction::NONE)
{
    registerProperties();
    ODataSettings::registerPropertiesFor(this);

    osl_atomic_increment(&m_refCount);
    OSL_ENSURE(m_xCommandDefinition.is(), "OQuery::OQuery : invalid CommandDefinition object !");
    if ( m_xCommandDefinition.is() )
    {
        try
        {
            ::comphelper::copyProperties(_rxCommandDefinition,this);
        }
        catch(Exception&)
        {
            OSL_FAIL("OQueryDescriptor_Base::OQueryDescriptor_Base: caught an exception!");
        }

        m_xCommandDefinition->addPropertyChangeListener(OUString(), this);
        //  m_xCommandDefinition->addPropertyChangeListener(PROPERTY_NAME, this);
        m_xCommandPropInfo = m_xCommandDefinition->getPropertySetInfo();
    }
    OSL_ENSURE(m_xConnection.is(), "OQuery::OQuery : invalid connection !");
    osl_atomic_decrement(&m_refCount);
}

OQuery::~OQuery()
{
}

css::uno::Sequence<sal_Int8> OQuery::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

IMPLEMENT_GETTYPES3(OQuery,OQueryDescriptor_Base,ODataSettings,OContentHelper);
IMPLEMENT_FORWARD_XINTERFACE3( OQuery,OContentHelper,OQueryDescriptor_Base,ODataSettings)

void OQuery::rebuildColumns()
{
    OSL_PRECOND( getColumnCount() == 0, "OQuery::rebuildColumns: column container should be empty!" );
        // the base class' definition of rebuildColumns promised that clearColumns is called before rebuildColumns

    try
    {
        m_pColumnMediator = nullptr;

        Reference<XColumnsSupplier> xColSup(m_xCommandDefinition,UNO_QUERY);
        Reference< XNameAccess > xColumnDefinitions;
        if ( xColSup.is() )
        {
            xColumnDefinitions = xColSup->getColumns();
            if ( xColumnDefinitions.is() )
                m_pColumnMediator = new OContainerMediator( m_pColumns.get(), xColumnDefinitions );
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
                OUString sError( DBA_RES( RID_STR_STATEMENT_WITHOUT_RESULT_SET ) );
                ::dbtools::throwSQLException( sError, StandardSQLState::GENERAL_ERROR, *this );
            }

            Reference< XDatabaseMetaData > xDBMeta( m_xConnection->getMetaData(), UNO_QUERY_THROW );
            ::rtl::Reference< OSQLColumns > aParseColumns(
                ::connectivity::parse::OParseColumn::createColumnsForResultSet( xResultSetMeta, xDBMeta,xColumnDefinitions ) );
            xColumns = OPrivateColumns::createWithIntrinsicNames(
                aParseColumns, xDBMeta->supportsMixedCaseQuotedIdentifiers(), *this, m_aMutex ).release();
            if ( !xColumns.is() )
                throw RuntimeException();
        }

        for ( const OUString& rName : xColumns->getElementNames() )
        {
            Reference<XPropertySet> xSource(xColumns->getByName( rName ),UNO_QUERY);
            OUString sLabel = rName;
            if ( xColumnDefinitions.is() && xColumnDefinitions->hasByName(rName) )
            {
                Reference<XPropertySet> xCommandColumn(xColumnDefinitions->getByName( rName ),UNO_QUERY);
                xCommandColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
            }
            OQueryColumn* pColumn = new OQueryColumn( xSource, m_xConnection, sLabel);
            Reference< XChild > xChild( *pColumn, UNO_QUERY_THROW );
            xChild->setParent( *this );

            implAppendColumn( rName, pColumn );
            Reference< XPropertySet > xDest( *pColumn, UNO_QUERY_THROW );
            if ( m_pColumnMediator.is() )
                m_pColumnMediator->notifyElementCreated( rName, xDest );
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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

// XServiceInfo
IMPLEMENT_SERVICE_INFO3(OQuery, "com.sun.star.sdb.dbaccess.OQuery", SERVICE_SDB_DATASETTINGS, SERVICE_SDB_QUERY, "com.sun.star.sdb.QueryDefinition")

// css::beans::XPropertyChangeListener
void SAL_CALL OQuery::propertyChange( const PropertyChangeEvent& _rSource )
{
    sal_Int32 nOwnHandle = -1;
    {
        MutexGuard aGuard(m_aMutex);

        OSL_ENSURE(_rSource.Source.get() == Reference< XInterface >(m_xCommandDefinition, UNO_QUERY).get(),
            "OQuery::propertyChange : where did this call come from ?");

        if (m_eDoingCurrently == AggregateAction::SettingProperties)
            // we're setting the property ourself, so we will do the necessary notifications later
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
            OSL_FAIL("OQuery::propertyChange : my CommandDefinition has more properties than I do !");
        }
    }

    fire(&nOwnHandle, &_rSource.NewValue, &_rSource.OldValue, 1, false);
}

void SAL_CALL OQuery::disposing( const EventObject& _rSource )
{
    MutexGuard aGuard(m_aMutex);

    OSL_ENSURE(_rSource.Source.get() == Reference< XInterface >(m_xCommandDefinition, UNO_QUERY).get(),
        "OQuery::disposing : where did this call come from ?");

    m_xCommandDefinition->removePropertyChangeListener(OUString(), this);
    m_xCommandDefinition = nullptr;
}

// XDataDescriptorFactory
Reference< XPropertySet > SAL_CALL OQuery::createDataDescriptor(  )
{
    return new OQueryDescriptor(*this);
}

// pseudo-XComponent
void SAL_CALL OQuery::disposing()
{
    MutexGuard aGuard(m_aMutex);
    if (m_xCommandDefinition.is())
    {
        m_xCommandDefinition->removePropertyChangeListener(OUString(), this);
        m_xCommandDefinition = nullptr;
    }
    disposeColumns();

    m_pWarnings = nullptr;
}

void OQuery::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
{
    ODataSettings::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    OUString sAggPropName;
    sal_Int16 nAttr = 0;
    if (getInfoHelper().fillPropertyMembersByHandle(&sAggPropName,&nAttr,_nHandle) &&
        m_xCommandPropInfo.is() &&
        m_xCommandPropInfo->hasPropertyByName(sAggPropName))
    {   // the base class holds the property values itself, but we have to forward this to our CommandDefinition

        m_eDoingCurrently = AggregateAction::SettingProperties;
        OAutoActionReset aAutoReset(*this);
        m_xCommandDefinition->setPropertyValue(sAggPropName, _rValue);

        if ( PROPERTY_ID_COMMAND == _nHandle )
            // the columns are out of date if we are based on a new statement ....
            setColumnsOutOfDate();
    }
}

Reference< XPropertySetInfo > SAL_CALL OQuery::getPropertySetInfo(  )
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

::cppu::IPropertyArrayHelper& OQuery::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* OQuery::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    // our own props
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

OColumn* OQuery::createColumn(const OUString& /*_rName*/) const
{
    return nullptr;
}

void SAL_CALL OQuery::rename( const OUString& newName )
{
    MutexGuard aGuard(m_aMutex);
    Reference<XRename> xRename(m_xCommandDefinition,UNO_QUERY);
    OSL_ENSURE(xRename.is(),"No XRename interface!");
    if(xRename.is())
        xRename->rename(newName);
}

void OQuery::registerProperties()
{
    // the properties which OCommandBase supplies (it has no own registration, as it's not derived from
    // a OPropertyStateContainer)
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND|PropertyAttribute::CONSTRAINED,
                    &m_sElementName, cppu::UnoType<decltype(m_sElementName)>::get());

    registerProperty(PROPERTY_COMMAND, PROPERTY_ID_COMMAND, PropertyAttribute::BOUND,
                    &m_sCommand, cppu::UnoType<decltype(m_sCommand)>::get());

    registerProperty(PROPERTY_ESCAPE_PROCESSING, PROPERTY_ID_ESCAPE_PROCESSING, PropertyAttribute::BOUND,
                    &m_bEscapeProcessing, cppu::UnoType<bool>::get());

    registerProperty(PROPERTY_UPDATE_TABLENAME, PROPERTY_ID_UPDATE_TABLENAME, PropertyAttribute::BOUND,
                    &m_sUpdateTableName, cppu::UnoType<decltype(m_sUpdateTableName)>::get());

    registerProperty(PROPERTY_UPDATE_SCHEMANAME, PROPERTY_ID_UPDATE_SCHEMANAME, PropertyAttribute::BOUND,
                    &m_sUpdateSchemaName, cppu::UnoType<decltype(m_sUpdateSchemaName)>::get());

    registerProperty(PROPERTY_UPDATE_CATALOGNAME, PROPERTY_ID_UPDATE_CATALOGNAME, PropertyAttribute::BOUND,
                    &m_sUpdateCatalogName, cppu::UnoType<decltype(m_sUpdateCatalogName)>::get());

    registerProperty(PROPERTY_LAYOUTINFORMATION, PROPERTY_ID_LAYOUTINFORMATION, PropertyAttribute::BOUND,
                    &m_aLayoutInformation, cppu::UnoType<decltype(m_aLayoutInformation)>::get());
}

OUString OQuery::determineContentType() const
{
    return OUString( "application/vnd.org.openoffice.DatabaseQuery" );
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
