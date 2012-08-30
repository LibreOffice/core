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


#include "tablecontainer.hxx"
#include "dbastrings.hrc"
#include "table.hxx"
#include <comphelper/property.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <comphelper/enumhelper.hxx>
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/extract.hxx>
#include <connectivity/dbexception.hxx>
#include "TableDeco.hxx"
#include "sdbcoretools.hxx"
#include "ContainerMediator.hxx"
#include "definitioncolumn.hxx"
#include "objectnameapproval.hxx"
#include <tools/string.hxx>
#include <rtl/logfile.hxx>
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
    sal_Bool lcl_isPropertySetDefaulted(const Sequence< ::rtl::OUString>& _aNames,const Reference<XPropertySet>& _xProp)
    {
        Reference<XPropertyState> xState(_xProp,UNO_QUERY);
        if ( xState.is() )
        {
            const ::rtl::OUString* pIter = _aNames.getConstArray();
            const ::rtl::OUString* pEnd   = pIter + _aNames.getLength();
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
                    OSL_FAIL( "lcl_isPropertySetDefaulted: Exception caught!" );
                }
            }
            return ( pIter == pEnd );
        }
        return sal_False;
    }
}
//==========================================================================
//= OTableContainer
//==========================================================================
DBG_NAME(OTableContainer)

OTableContainer::OTableContainer(::cppu::OWeakObject& _rParent,
                                 ::osl::Mutex& _rMutex,
                                 const Reference< XConnection >& _xCon,
                                 sal_Bool _bCase,
                                 const Reference< XNameContainer >& _xTableDefinitions,
                                 IRefreshListener*  _pRefreshListener,
                                 ::dbtools::IWarningsContainer* _pWarningsContainer
                                 ,oslInterlockedCount& _nInAppend)
    :OFilteredContainer(_rParent,_rMutex,_xCon,_bCase,_pRefreshListener,_pWarningsContainer,_nInAppend)
    ,m_xTableDefinitions(_xTableDefinitions)
    ,m_pTableMediator( NULL )
    ,m_bInDrop(sal_False)
{
    DBG_CTOR(OTableContainer, NULL);
}

OTableContainer::~OTableContainer()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::OTableContainer" );
    DBG_DTOR(OTableContainer, NULL);
}

void OTableContainer::removeMasterContainerListener()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::removeMasterContainerListener" );
    try
    {
        Reference<XContainer> xCont( m_xMasterContainer, UNO_QUERY_THROW );
        xCont->removeContainerListener( this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

::rtl::OUString OTableContainer::getTableTypeRestriction() const
{
    // no restriction at all (other than the ones provided externally)
    return ::rtl::OUString();
}

// XServiceInfo
IMPLEMENT_SERVICE_INFO2(OTableContainer, "com.sun.star.sdb.dbaccess.OTableContainer", SERVICE_SDBCX_CONTAINER.ascii, SERVICE_SDBCX_TABLES.ascii)

namespace
{
void lcl_createDefintionObject(const ::rtl::OUString& _rName
                           ,const Reference< XNameContainer >& _xTableDefinitions
                           ,Reference<XPropertySet>& _xTableDefinition
                           ,Reference<XNameAccess>& _xColumnDefinitions
                           ,sal_Bool _bModified)
{
    if ( _xTableDefinitions.is() )
    {
        if ( _xTableDefinitions->hasByName(_rName) )
            _xTableDefinition.set(_xTableDefinitions->getByName(_rName),UNO_QUERY);
        else
        {
            Sequence< Any > aArguments(1);
            PropertyValue aValue;
            // set as folder
            aValue.Name = PROPERTY_NAME;
            aValue.Value <<= _rName;
            aArguments[0] <<= aValue;
            _xTableDefinition.set(::comphelper::getProcessServiceFactory()->createInstanceWithArguments(SERVICE_SDB_TABLEDEFINITION,aArguments),UNO_QUERY);
            _xTableDefinitions->insertByName(_rName,makeAny(_xTableDefinition));
            ::dbaccess::notifyDataSourceModified(_xTableDefinitions,_bModified);
        }
        Reference<XColumnsSupplier> xColumnsSupplier(_xTableDefinition,UNO_QUERY);
        if ( xColumnsSupplier.is() )
            _xColumnDefinitions = xColumnsSupplier->getColumns();
    }
}

}

connectivity::sdbcx::ObjectType OTableContainer::createObject(const ::rtl::OUString& _rName)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::createObject" );
    Reference<XColumnsSupplier > xSup;
    if(m_xMasterContainer.is() && m_xMasterContainer->hasByName(_rName))
        xSup.set(m_xMasterContainer->getByName(_rName),UNO_QUERY);

    connectivity::sdbcx::ObjectType xRet;
    if ( m_xMetaData.is() )
    {
        Reference<XPropertySet> xTableDefinition;
        Reference<XNameAccess> xColumnDefinitions;
        lcl_createDefintionObject(_rName,m_xTableDefinitions,xTableDefinition,xColumnDefinitions,sal_False);

        if ( xSup.is() )
        {
            ODBTableDecorator* pTable = new ODBTableDecorator( m_xConnection, xSup, ::dbtools::getNumberFormats( m_xConnection ) ,xColumnDefinitions);
            xRet = pTable;
            pTable->construct();
        }
        else
        {
            ::rtl::OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(m_xMetaData,
                                                _rName,
                                                sCatalog,
                                                sSchema,
                                                sTable,
                                                ::dbtools::eInDataManipulation);
            Any aCatalog;
            if(!sCatalog.isEmpty())
                aCatalog <<= sCatalog;
            ::rtl::OUString sType,sDescription;
            Sequence< ::rtl::OUString> aTypeFilter;
            getAllTableTypeFilter( aTypeFilter );

            Reference< XResultSet > xRes =  m_xMetaData.is() ? m_xMetaData->getTables(aCatalog,sSchema,sTable,aTypeFilter) : Reference< XResultSet >();
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
                    this, m_xTableDefinitions.get(), m_xConnection );
        if ( m_pTableMediator.is() )
            m_pTableMediator->notifyElementCreated(_rName,xDest);
    }

    return xRet;
}

Reference< XPropertySet > OTableContainer::createDescriptor()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::createDescriptor" );
    Reference< XPropertySet > xRet;

    // first we have to look if the master tables support this
    // and if so then create a table object as well with the master tables
    Reference<XColumnsSupplier > xMasterColumnsSup;
    Reference<XDataDescriptorFactory> xDataFactory(m_xMasterContainer,UNO_QUERY);
    if ( xDataFactory.is() && m_xMetaData.is() )
    {
        xMasterColumnsSup = Reference< XColumnsSupplier >( xDataFactory->createDataDescriptor(), UNO_QUERY );
        ODBTableDecorator* pTable = new ODBTableDecorator( m_xConnection, xMasterColumnsSup, ::dbtools::getNumberFormats( m_xConnection ) ,NULL);
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
ObjectType OTableContainer::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::appendObject" );
    // append the new table with a create stmt
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));
    if(m_xMasterContainer.is() && m_xMasterContainer->hasByName(aName))
    {
        String sMessage(DBACORE_RESSTRING(RID_STR_TABLE_IS_FILTERED));
        sMessage.SearchAndReplaceAscii("$name$", aName);
        throw SQLException(sMessage,static_cast<XTypeProvider*>(static_cast<OFilteredContainer*>(this)),SQLSTATE_GENERAL,1000,Any());
    }

    Reference< XConnection > xConnection( m_xConnection.get(), UNO_QUERY );
    PContainerApprove pApprove( new ObjectNameApproval( xConnection, ObjectNameApproval::TypeTable ) );
    pApprove->approveElement( aName, descriptor );

    try
    {
        EnsureReset aReset(m_nInAppend);
        Reference<XAppend> xAppend(m_xMasterContainer,UNO_QUERY);
        if(xAppend.is())
        {
            xAppend->appendByDescriptor(descriptor);
        }
        else
        {
            ::rtl::OUString aSql = ::dbtools::createSqlCreateTableStatement(descriptor,m_xConnection);

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
    catch(const Exception&)
    {
        throw;
    }

    Reference<XPropertySet> xTableDefinition;
    Reference<XNameAccess> xColumnDefinitions;
    lcl_createDefintionObject(getNameForObject(descriptor),m_xTableDefinitions,xTableDefinition,xColumnDefinitions,sal_False);
    Reference<XColumnsSupplier> xSup(descriptor,UNO_QUERY);
    Reference<XDataDescriptorFactory> xFac(xColumnDefinitions,UNO_QUERY);
    Reference<XAppend> xAppend(xColumnDefinitions,UNO_QUERY);
    sal_Bool bModified = sal_False;
    if ( xSup.is() && xColumnDefinitions.is() && xFac.is() && xAppend.is() )
    {
        Reference<XNameAccess> xNames = xSup->getColumns();
        if ( xNames.is() )
        {
            Reference<XPropertySet> xProp = xFac->createDataDescriptor();
            Sequence< ::rtl::OUString> aSeq = xNames->getElementNames();
            const ::rtl::OUString* pIter = aSeq.getConstArray();
            const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
            for(;pIter != pEnd;++pIter)
            {
                if ( !xColumnDefinitions->hasByName(*pIter) )
                {
                    Reference<XPropertySet> xColumn(xNames->getByName(*pIter),UNO_QUERY);
                    if ( !OColumnSettings::hasDefaultSettings( xColumn ) )
                    {
                        ::comphelper::copyProperties( xColumn, xProp );
                        xAppend->appendByDescriptor( xProp );
                        bModified = sal_True;
                    }
                }
            }
        }
    }
    const static ::rtl::OUString s_pTableProps[] = {    ::rtl::OUString(PROPERTY_FILTER), ::rtl::OUString(PROPERTY_ORDER)
                                                    , ::rtl::OUString(PROPERTY_APPLYFILTER), ::rtl::OUString(PROPERTY_FONT)
                                                    , ::rtl::OUString(PROPERTY_ROW_HEIGHT), ::rtl::OUString(PROPERTY_TEXTCOLOR)
                                                    , ::rtl::OUString(PROPERTY_TEXTLINECOLOR), ::rtl::OUString(PROPERTY_TEXTEMPHASIS)
                                                    , ::rtl::OUString(PROPERTY_TEXTRELIEF) };
    Sequence< ::rtl::OUString> aNames(s_pTableProps,sizeof(s_pTableProps)/sizeof(s_pTableProps[0]));
    if ( bModified || !lcl_isPropertySetDefaulted(aNames,xTableDefinition) )
        ::dbaccess::notifyDataSourceModified(m_xTableDefinitions,sal_True);

    return createObject( _rForName );
}

// XDrop
void OTableContainer::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::dropObject" );
    m_bInDrop = sal_True;
    try
    {
        Reference< XDrop > xDrop(m_xMasterContainer,UNO_QUERY);
        if(xDrop.is())
            xDrop->dropByName(_sElementName);
        else
        {
            ::rtl::OUString sCatalog,sSchema,sTable,sComposedName;

            sal_Bool bIsView = sal_False;
            Reference<XPropertySet> xTable(getObject(_nPos),UNO_QUERY);
            if ( xTable.is() && m_xMetaData.is() )
            {
                if( m_xMetaData.is() && m_xMetaData->supportsCatalogsInTableDefinitions() )
                    xTable->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
                if( m_xMetaData.is() && m_xMetaData->supportsSchemasInTableDefinitions() )
                    xTable->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;
                xTable->getPropertyValue(PROPERTY_NAME)         >>= sTable;

                sComposedName = ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sTable, sal_True, ::dbtools::eInTableDefinitions );

                ::rtl::OUString sType;
                xTable->getPropertyValue(PROPERTY_TYPE)         >>= sType;
                bIsView = sType.equalsIgnoreAsciiCase(::rtl::OUString("VIEW"));
            }

            if(sComposedName.isEmpty())
                ::dbtools::throwFunctionSequenceException(static_cast<XTypeProvider*>(static_cast<OFilteredContainer*>(this)));

            ::rtl::OUString aSql("DROP ");

            if ( bIsView ) // here we have a view
                aSql += ::rtl::OUString("VIEW ");
            else
                aSql += ::rtl::OUString("TABLE ");
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
    catch(const Exception&)
    {
        m_bInDrop = sal_False;
        throw;
    }
    m_bInDrop = sal_False;
}

void SAL_CALL OTableContainer::elementInserted( const ContainerEvent& Event ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::elementInserted" );
    ::osl::MutexGuard aGuard(m_rMutex);
    ::rtl::OUString sName;
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

void SAL_CALL OTableContainer::elementRemoved( const ContainerEvent& /*Event*/ ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::elementRemoved" );
}

void SAL_CALL OTableContainer::elementReplaced( const ContainerEvent& Event ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::elementReplaced" );
    // create a new config entry
    {
        ::rtl::OUString sOldComposedName,sNewComposedName;
        Event.ReplacedElement   >>= sOldComposedName;
        Event.Accessor          >>= sNewComposedName;

        renameObject(sOldComposedName,sNewComposedName);
    }
}

void SAL_CALL OTableContainer::disposing()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::disposing" );
    OFilteredContainer::disposing();
    // say goodbye to our listeners
    m_xTableDefinitions = NULL;
    m_pTableMediator = NULL;
}

void SAL_CALL OTableContainer::disposing( const ::com::sun::star::lang::EventObject& /*Source*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "api", "Ocke.Janssen@sun.com", "OTableContainer::disposing" );
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
        DBG_UNHANDLED_EXCEPTION();
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
