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

#include <tablecontainer.hxx>
#include <stringconstants.hxx>
#include <table.hxx>
#include <sal/log.hxx>
#include <comphelper/property.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/sdb/TableDefinition.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <TableDeco.hxx>
#include <sdbcoretools.hxx>
#include <ContainerMediator.hxx>
#include <definitioncolumn.hxx>
#include <objectnameapproval.hxx>
#include <tools/diagnose_ex.h>

using namespace dbaccess;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace ::connectivity::sdbcx;

namespace
{
    bool lcl_isPropertySetDefaulted(const Sequence< OUString>& _aNames,const Reference<XPropertySet>& _xProp)
    {
        Reference<XPropertyState> xState(_xProp,UNO_QUERY);
        if ( xState.is() )
        {
            const OUString* pIter = _aNames.getConstArray();
            const OUString* pEnd   = pIter + _aNames.getLength();
            for(;pIter != pEnd;++pIter)
            {
                try
                {
                    PropertyState aState = xState->getPropertyState(*pIter);
                    if ( aState != PropertyState_DEFAULT_VALUE )
                        break;
                }
                catch(const Exception&)
                {
                    SAL_WARN("dbaccess", "lcl_isPropertySetDefaulted: Exception caught!" );
                }
            }
            return ( pIter == pEnd );
        }
        return false;
    }
}

// OTableContainer

OTableContainer::OTableContainer(::cppu::OWeakObject& _rParent,
                                 ::osl::Mutex& _rMutex,
                                 const Reference< XConnection >& _xCon,
                                 bool _bCase,
                                 const Reference< XNameContainer >& _xTableDefinitions,
                                 IRefreshListener*  _pRefreshListener,
                                 std::atomic<std::size_t>& _nInAppend)
    :OFilteredContainer(_rParent,_rMutex,_xCon,_bCase,_pRefreshListener,_nInAppend)
    ,m_xTableDefinitions(_xTableDefinitions)
{
}

OTableContainer::~OTableContainer()
{
}

void OTableContainer::removeMasterContainerListener()
{
    try
    {
        Reference<XContainer> xCont( m_xMasterContainer, UNO_QUERY_THROW );
        xCont->removeContainerListener( this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

OUString OTableContainer::getTableTypeRestriction() const
{
    // no restriction at all (other than the ones provided externally)
    return OUString();
}

// XServiceInfo
IMPLEMENT_SERVICE_INFO2(OTableContainer, "com.sun.star.sdb.dbaccess.OTableContainer", SERVICE_SDBCX_CONTAINER, SERVICE_SDBCX_TABLES)

namespace
{
void lcl_createDefintionObject(const OUString& _rName
                           ,const Reference< XNameContainer >& _xTableDefinitions
                           ,Reference<XPropertySet>& _xTableDefinition
                           ,Reference<XNameAccess>& _xColumnDefinitions)
{
    if ( _xTableDefinitions.is() )
    {
        if ( _xTableDefinitions->hasByName(_rName) )
            _xTableDefinition.set(_xTableDefinitions->getByName(_rName),UNO_QUERY);
        else
        {
            // set as folder
            _xTableDefinition.set( TableDefinition::createWithName( ::comphelper::getProcessComponentContext(), _rName ), UNO_QUERY);
            _xTableDefinitions->insertByName(_rName,makeAny(_xTableDefinition));
        }
        Reference<XColumnsSupplier> xColumnsSupplier(_xTableDefinition,UNO_QUERY);
        if ( xColumnsSupplier.is() )
            _xColumnDefinitions = xColumnsSupplier->getColumns();
    }
}

}

connectivity::sdbcx::ObjectType OTableContainer::createObject(const OUString& _rName)
{
    Reference<XColumnsSupplier > xSup;
    if(m_xMasterContainer.is() && m_xMasterContainer->hasByName(_rName))
        xSup.set(m_xMasterContainer->getByName(_rName),UNO_QUERY);

    connectivity::sdbcx::ObjectType xRet;
    if ( m_xMetaData.is() )
    {
        Reference<XPropertySet> xTableDefinition;
        Reference<XNameAccess> xColumnDefinitions;
        lcl_createDefintionObject(_rName,m_xTableDefinitions,xTableDefinition,xColumnDefinitions);

        if ( xSup.is() )
        {
            ODBTableDecorator* pTable = new ODBTableDecorator( m_xConnection, xSup, ::dbtools::getNumberFormats( m_xConnection ) ,xColumnDefinitions);
            xRet = pTable;
            pTable->construct();
        }
        else
        {
            OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(m_xMetaData,
                                                _rName,
                                                sCatalog,
                                                sSchema,
                                                sTable,
                                                ::dbtools::EComposeRule::InDataManipulation);
            Any aCatalog;
            if(!sCatalog.isEmpty())
                aCatalog <<= sCatalog;
            OUString sType,sDescription;
            Sequence< OUString> aTypeFilter;
            getAllTableTypeFilter( aTypeFilter );

            Reference< XResultSet > xRes;
            if ( m_xMetaData.is() )
                xRes = m_xMetaData->getTables(aCatalog,sSchema,sTable,aTypeFilter);
            if(xRes.is() && xRes->next())
            {
                Reference< XRow > xRow(xRes,UNO_QUERY);
                if(xRow.is())
                {
                    sType           = xRow->getString(4);
                    sDescription    = xRow->getString(5);
                }
            }
            ::comphelper::disposeComponent(xRes);
            ODBTable* pTable = new ODBTable(this
                                ,m_xConnection
                                ,sCatalog
                                ,sSchema
                                ,sTable
                                ,sType
                                ,sDescription
                                ,xColumnDefinitions);
            xRet = pTable;
            pTable->construct();
        }
        Reference<XPropertySet> xDest(xRet,UNO_QUERY);
        if ( xTableDefinition.is() )
            ::comphelper::copyProperties(xTableDefinition,xDest);

        if ( !m_pTableMediator.is() )
            m_pTableMediator = new OContainerMediator(
                    this, m_xTableDefinitions.get() );
        if ( m_pTableMediator.is() )
            m_pTableMediator->notifyElementCreated(_rName,xDest);
    }

    return xRet;
}

Reference< XPropertySet > OTableContainer::createDescriptor()
{
    Reference< XPropertySet > xRet;

    // first we have to look if the master tables support this
    // and if so then create a table object as well with the master tables
    Reference<XColumnsSupplier > xMasterColumnsSup;
    Reference<XDataDescriptorFactory> xDataFactory(m_xMasterContainer,UNO_QUERY);
    if ( xDataFactory.is() && m_xMetaData.is() )
    {
        xMasterColumnsSup.set( xDataFactory->createDataDescriptor(), UNO_QUERY );
        ODBTableDecorator* pTable = new ODBTableDecorator( m_xConnection, xMasterColumnsSup, ::dbtools::getNumberFormats( m_xConnection ) ,nullptr);
        xRet = pTable;
        pTable->construct();
    }
    else
    {
        ODBTable* pTable = new ODBTable(this, m_xConnection);
        xRet = pTable;
        pTable->construct();
    }
    return xRet;
}

// XAppend
ObjectType OTableContainer::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    // append the new table with a create stmt
    OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));
    if(m_xMasterContainer.is() && m_xMasterContainer->hasByName(aName))
    {
        OUString sMessage(DBA_RES(RID_STR_TABLE_IS_FILTERED));
        throw SQLException(sMessage.replaceAll("$name$", aName),static_cast<XTypeProvider*>(static_cast<OFilteredContainer*>(this)),SQLSTATE_GENERAL,1000,Any());
    }

    Reference< XConnection > xConnection( m_xConnection.get(), UNO_QUERY );
    PContainerApprove pApprove( new ObjectNameApproval( xConnection, ObjectNameApproval::TypeTable ) );
    pApprove->approveElement( aName, descriptor );

    {
        EnsureReset aReset(m_nInAppend);
        Reference<XAppend> xAppend(m_xMasterContainer,UNO_QUERY);
        if(xAppend.is())
        {
            xAppend->appendByDescriptor(descriptor);
        }
        else
        {
            OUString aSql = ::dbtools::createSqlCreateTableStatement(descriptor,m_xConnection);

            Reference<XConnection> xCon = m_xConnection;
            OSL_ENSURE(xCon.is(),"Connection is null!");
            if ( xCon.is() )
            {
                Reference< XStatement > xStmt = xCon->createStatement(  );
                if ( xStmt.is() )
                    xStmt->execute(aSql);
                ::comphelper::disposeComponent(xStmt);
            }
        }
    }

    Reference<XPropertySet> xTableDefinition;
    Reference<XNameAccess> xColumnDefinitions;
    lcl_createDefintionObject(getNameForObject(descriptor),m_xTableDefinitions,xTableDefinition,xColumnDefinitions);
    Reference<XColumnsSupplier> xSup(descriptor,UNO_QUERY);
    Reference<XDataDescriptorFactory> xFac(xColumnDefinitions,UNO_QUERY);
    Reference<XAppend> xAppend(xColumnDefinitions,UNO_QUERY);
    bool bModified = false;
    if ( xSup.is() && xColumnDefinitions.is() && xFac.is() && xAppend.is() )
    {
        Reference<XNameAccess> xNames = xSup->getColumns();
        if ( xNames.is() )
        {
            Reference<XPropertySet> xProp = xFac->createDataDescriptor();
            Sequence< OUString> aSeq = xNames->getElementNames();
            const OUString* pIter = aSeq.getConstArray();
            const OUString* pEnd   = pIter + aSeq.getLength();
            for(;pIter != pEnd;++pIter)
            {
                if ( !xColumnDefinitions->hasByName(*pIter) )
                {
                    Reference<XPropertySet> xColumn(xNames->getByName(*pIter),UNO_QUERY);
                    if ( !OColumnSettings::hasDefaultSettings( xColumn ) )
                    {
                        ::comphelper::copyProperties( xColumn, xProp );
                        xAppend->appendByDescriptor( xProp );
                        bModified = true;
                    }
                }
            }
        }
    }
    Sequence< OUString> aNames{
        PROPERTY_FILTER, PROPERTY_ORDER, PROPERTY_APPLYFILTER, PROPERTY_FONT,
        PROPERTY_ROW_HEIGHT, PROPERTY_TEXTCOLOR, PROPERTY_TEXTLINECOLOR,
        PROPERTY_TEXTEMPHASIS, PROPERTY_TEXTRELIEF};
    if ( bModified || !lcl_isPropertySetDefaulted(aNames,xTableDefinition) )
        ::dbaccess::notifyDataSourceModified(m_xTableDefinitions);

    return createObject( _rForName );
}

// XDrop
void OTableContainer::dropObject(sal_Int32 _nPos, const OUString& _sElementName)
{
    Reference< XDrop > xDrop(m_xMasterContainer,UNO_QUERY);
    if(xDrop.is())
        xDrop->dropByName(_sElementName);
    else
    {
        OUString sCatalog,sSchema,sTable,sComposedName;

        bool bIsView = false;
        Reference<XPropertySet> xTable(getObject(_nPos),UNO_QUERY);
        if ( xTable.is() && m_xMetaData.is() )
        {
            if (m_xMetaData->supportsCatalogsInTableDefinitions())
                xTable->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
            if (m_xMetaData->supportsSchemasInTableDefinitions())
                xTable->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;
            xTable->getPropertyValue(PROPERTY_NAME)         >>= sTable;

            sComposedName = ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sTable, true, ::dbtools::EComposeRule::InTableDefinitions );

            OUString sType;
            xTable->getPropertyValue(PROPERTY_TYPE)         >>= sType;
            bIsView = sType.equalsIgnoreAsciiCase("VIEW");
        }

        if(sComposedName.isEmpty())
            ::dbtools::throwFunctionSequenceException(static_cast<XTypeProvider*>(static_cast<OFilteredContainer*>(this)));

        OUString aSql("DROP ");

        if ( bIsView ) // here we have a view
            aSql += "VIEW ";
        else
            aSql += "TABLE ";
        aSql += sComposedName;
        Reference<XConnection> xCon = m_xConnection;
        OSL_ENSURE(xCon.is(),"Connection is null!");
        if ( xCon.is() )
        {
            Reference< XStatement > xStmt = xCon->createStatement(  );
            if(xStmt.is())
                xStmt->execute(aSql);
            ::comphelper::disposeComponent(xStmt);
        }
    }

    if ( m_xTableDefinitions.is() && m_xTableDefinitions->hasByName(_sElementName) )
    {
        m_xTableDefinitions->removeByName(_sElementName);
    }
}

void SAL_CALL OTableContainer::elementInserted( const ContainerEvent& Event )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    OUString sName;
    Event.Accessor >>= sName;
    if ( !m_nInAppend && !hasByName(sName) )
    {
        if(!m_xMasterContainer.is() || m_xMasterContainer->hasByName(sName))
        {
            ObjectType xName = createObject(sName);
            insertElement(sName,xName);
            // and notify our listeners
            ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(sName), makeAny(xName), Any());
            m_aContainerListeners.notifyEach( &XContainerListener::elementInserted, aEvent );
        }
    }
}

void SAL_CALL OTableContainer::elementRemoved( const ContainerEvent& /*Event*/ )
{
}

void SAL_CALL OTableContainer::elementReplaced( const ContainerEvent& Event )
{
    // create a new config entry
    OUString sOldComposedName,sNewComposedName;
    Event.ReplacedElement   >>= sOldComposedName;
    Event.Accessor          >>= sNewComposedName;

    renameObject(sOldComposedName,sNewComposedName);
}

void OTableContainer::disposing()
{
    OFilteredContainer::disposing();
    // say goodbye to our listeners
    m_xTableDefinitions = nullptr;
    m_pTableMediator = nullptr;
}

void SAL_CALL OTableContainer::disposing( const css::lang::EventObject& /*Source*/ )
{
}

void OTableContainer::addMasterContainerListener()
{
    try
    {
        Reference< XContainer > xCont( m_xMasterContainer, UNO_QUERY_THROW );
        xCont->addContainerListener( this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
