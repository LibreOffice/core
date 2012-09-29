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

#include <string.h>

#include "TableDeco.hxx"
#include <definitioncolumn.hxx>
#include "dbastrings.hrc"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <tools/debug.hxx>
#include <osl/diagnose.h>

#include <cppuhelper/typeprovider.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/container.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/util/XRefreshListener.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <comphelper/extract.hxx>
#include "ContainerMediator.hxx"
#include <rtl/logfile.hxx>

using namespace dbaccess;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::dbtools;
using namespace ::cppu;

//==========================================================================
//= ODBTableDecorator
//==========================================================================
DBG_NAME(ODBTableDecorator)

ODBTableDecorator::ODBTableDecorator( const Reference< XConnection >& _rxConnection, const Reference< XColumnsSupplier >& _rxNewTable,
        const Reference< XNumberFormatsSupplier >& _rxNumberFormats, const Reference< XNameAccess >& _xColumnDefinitions ) throw(SQLException)
    :OTableDescriptor_BASE(m_aMutex)
    ,ODataSettings(OTableDescriptor_BASE::rBHelper)
    ,m_xTable(_rxNewTable)
    ,m_xColumnDefinitions(_xColumnDefinitions)
    ,m_xConnection( _rxConnection )
    ,m_xMetaData( _rxConnection.is() ? _rxConnection->getMetaData() : Reference< XDatabaseMetaData >() )
    ,m_xNumberFormats( _rxNumberFormats )
    ,m_nPrivileges(-1)
    ,m_pColumns(NULL)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::ODBTableDecorator" );
    DBG_CTOR(ODBTableDecorator, NULL);
    ODataSettings::registerPropertiesFor(this);
}

ODBTableDecorator::~ODBTableDecorator()
{
    DBG_DTOR(ODBTableDecorator, NULL);
    if ( m_pColumns )
        delete m_pColumns;
}

Sequence< sal_Int8 > ODBTableDecorator::getImplementationId() throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::getImplementationId" );
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// OComponentHelper
void SAL_CALL ODBTableDecorator::disposing()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::disposing" );
    OPropertySetHelper::disposing();
    OTableDescriptor_BASE::disposing();

    MutexGuard aGuard(m_aMutex);
    m_xTable        = NULL;
    m_xMetaData     = NULL;
    m_pTables       = NULL;
    m_xColumnDefinitions = NULL;
    m_xNumberFormats = NULL;
    if ( m_pColumns )
        m_pColumns->disposing();
    m_xColumnMediator = NULL;
}

sal_Bool SAL_CALL ODBTableDecorator::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bRet = sal_True;
    switch(nHandle)
    {
        case PROPERTY_ID_PRIVILEGES:
        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_ORDER:
        case PROPERTY_ID_APPLYFILTER:
        case PROPERTY_ID_FONT:
        case PROPERTY_ID_ROW_HEIGHT:
        case PROPERTY_ID_TEXTCOLOR:
        case PROPERTY_ID_TEXTLINECOLOR:
        case PROPERTY_ID_TEXTEMPHASIS:
        case PROPERTY_ID_TEXTRELIEF:
        case PROPERTY_ID_FONTCHARWIDTH:
        case PROPERTY_ID_FONTCHARSET:
        case PROPERTY_ID_FONTFAMILY:
        case PROPERTY_ID_FONTHEIGHT:
        case PROPERTY_ID_FONTKERNING:
        case PROPERTY_ID_FONTNAME:
        case PROPERTY_ID_FONTORIENTATION:
        case PROPERTY_ID_FONTPITCH:
        case PROPERTY_ID_FONTSLANT:
        case PROPERTY_ID_FONTSTRIKEOUT:
        case PROPERTY_ID_FONTSTYLENAME:
        case PROPERTY_ID_FONTUNDERLINE:
        case PROPERTY_ID_FONTWEIGHT:
        case PROPERTY_ID_FONTWIDTH:
        case PROPERTY_ID_FONTWORDLINEMODE:
        case PROPERTY_ID_FONTTYPE:
            bRet = ODataSettings::convertFastPropertyValue(rConvertedValue, rOldValue,nHandle,rValue);
            break;

        default:
            {
                Any aValue;
                getFastPropertyValue(aValue,nHandle);
                bRet = ::comphelper::tryPropertyValue(rConvertedValue,rOldValue,rValue,aValue,::getCppuType(static_cast< ::rtl::OUString*>(0)));
            }
            break; // we assume that it works
    }
    return bRet;
}

void ODBTableDecorator::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw (Exception)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::setFastPropertyValue_NoBroadcast" );
    switch(_nHandle)
    {
        case PROPERTY_ID_PRIVILEGES:
            OSL_FAIL("Property is readonly!");
        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_ORDER:
        case PROPERTY_ID_APPLYFILTER:
        case PROPERTY_ID_FONT:
        case PROPERTY_ID_ROW_HEIGHT:
        case PROPERTY_ID_TEXTCOLOR:
        case PROPERTY_ID_TEXTLINECOLOR:
        case PROPERTY_ID_TEXTEMPHASIS:
        case PROPERTY_ID_TEXTRELIEF:
        case PROPERTY_ID_FONTCHARWIDTH:
        case PROPERTY_ID_FONTCHARSET:
        case PROPERTY_ID_FONTFAMILY:
        case PROPERTY_ID_FONTHEIGHT:
        case PROPERTY_ID_FONTKERNING:
        case PROPERTY_ID_FONTNAME:
        case PROPERTY_ID_FONTORIENTATION:
        case PROPERTY_ID_FONTPITCH:
        case PROPERTY_ID_FONTSLANT:
        case PROPERTY_ID_FONTSTRIKEOUT:
        case PROPERTY_ID_FONTSTYLENAME:
        case PROPERTY_ID_FONTUNDERLINE:
        case PROPERTY_ID_FONTWEIGHT:
        case PROPERTY_ID_FONTWIDTH:
        case PROPERTY_ID_FONTWORDLINEMODE:
        case PROPERTY_ID_FONTTYPE:

            ODataSettings::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
            break;
        case PROPERTY_ID_CATALOGNAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                xProp->setPropertyValue(PROPERTY_CATALOGNAME,_rValue);
            }
            break;
        case PROPERTY_ID_SCHEMANAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                xProp->setPropertyValue(PROPERTY_SCHEMANAME,_rValue);
            }
            break;
        case PROPERTY_ID_NAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                xProp->setPropertyValue(PROPERTY_NAME,_rValue);
            }
            break;
        case PROPERTY_ID_DESCRIPTION:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                xProp->setPropertyValue(PROPERTY_DESCRIPTION,_rValue);
            }
            break;
        case PROPERTY_ID_TYPE:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                xProp->setPropertyValue(PROPERTY_TYPE,_rValue);
            }
            break;
    }
}

void ODBTableDecorator::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{

    switch(_nHandle)
    {
        case PROPERTY_ID_PRIVILEGES:
            {
                if ( -1 == m_nPrivileges )
                    fillPrivileges();
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                Reference<XPropertySetInfo> xInfo = xProp->getPropertySetInfo();
                if ( xInfo->hasPropertyByName(PROPERTY_PRIVILEGES) )
                {
                    _rValue <<= m_nPrivileges;
                    break;
                }
            }
            // run through

        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_ORDER:
        case PROPERTY_ID_APPLYFILTER:
        case PROPERTY_ID_FONT:
        case PROPERTY_ID_ROW_HEIGHT:
        case PROPERTY_ID_TEXTCOLOR:
        case PROPERTY_ID_TEXTLINECOLOR:
        case PROPERTY_ID_TEXTEMPHASIS:
        case PROPERTY_ID_TEXTRELIEF:
        case PROPERTY_ID_FONTCHARWIDTH:
        case PROPERTY_ID_FONTCHARSET:
        case PROPERTY_ID_FONTFAMILY:
        case PROPERTY_ID_FONTHEIGHT:
        case PROPERTY_ID_FONTKERNING:
        case PROPERTY_ID_FONTNAME:
        case PROPERTY_ID_FONTORIENTATION:
        case PROPERTY_ID_FONTPITCH:
        case PROPERTY_ID_FONTSLANT:
        case PROPERTY_ID_FONTSTRIKEOUT:
        case PROPERTY_ID_FONTSTYLENAME:
        case PROPERTY_ID_FONTUNDERLINE:
        case PROPERTY_ID_FONTWEIGHT:
        case PROPERTY_ID_FONTWIDTH:
        case PROPERTY_ID_FONTWORDLINEMODE:
        case PROPERTY_ID_FONTTYPE:
            ODataSettings::getFastPropertyValue(_rValue, _nHandle);
            break;
        case PROPERTY_ID_CATALOGNAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                _rValue = xProp->getPropertyValue(PROPERTY_CATALOGNAME);
            }
            break;
        case PROPERTY_ID_SCHEMANAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                _rValue = xProp->getPropertyValue(PROPERTY_SCHEMANAME);
            }
            break;
        case PROPERTY_ID_NAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                _rValue = xProp->getPropertyValue(PROPERTY_NAME);
            }
            break;
        case PROPERTY_ID_DESCRIPTION:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                _rValue = xProp->getPropertyValue(PROPERTY_DESCRIPTION);
            }
            break;
        case PROPERTY_ID_TYPE:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                _rValue = xProp->getPropertyValue(PROPERTY_TYPE);
            }
            break;
        default:
            OSL_FAIL("Invalid Handle for table");
    }
}

void ODBTableDecorator::construct()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::construct" );
    sal_Bool bNotFound = sal_True;
    Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
    if ( xProp.is() )
    {
        Reference<XPropertySetInfo> xInfo = xProp->getPropertySetInfo();
        bNotFound = !xInfo->hasPropertyByName(PROPERTY_PRIVILEGES);
    }
    if ( bNotFound )
        registerProperty(PROPERTY_PRIVILEGES, PROPERTY_ID_PRIVILEGES, PropertyAttribute::BOUND  | PropertyAttribute::READONLY,
                        &m_nPrivileges, ::getCppuType(static_cast<sal_Int32*>(NULL)));
}

::cppu::IPropertyArrayHelper* ODBTableDecorator::createArrayHelper(sal_Int32 /*_nId*/) const
{
    Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
    Reference<XPropertySetInfo> xInfo = xProp->getPropertySetInfo();

    Sequence< Property > aTableProps = xInfo->getProperties();
    Property* pIter = aTableProps.getArray();
    Property* pEnd = pIter + aTableProps.getLength();
    for (;pIter != pEnd ; ++pIter)
    {
        if (pIter->Name.equalsAsciiL(PROPERTY_CATALOGNAME.ascii, PROPERTY_CATALOGNAME.length))
            pIter->Handle = PROPERTY_ID_CATALOGNAME;
        else if (pIter->Name.equalsAsciiL(PROPERTY_SCHEMANAME.ascii, PROPERTY_SCHEMANAME.length))
            pIter->Handle = PROPERTY_ID_SCHEMANAME;
        else if (pIter->Name.equalsAsciiL(PROPERTY_NAME.ascii, PROPERTY_NAME.length))
            pIter->Handle = PROPERTY_ID_NAME;
        else if (pIter->Name.equalsAsciiL(PROPERTY_DESCRIPTION.ascii, PROPERTY_DESCRIPTION.length))
            pIter->Handle = PROPERTY_ID_DESCRIPTION;
        else if (pIter->Name.equalsAsciiL(PROPERTY_TYPE.ascii, PROPERTY_TYPE.length))
            pIter->Handle = PROPERTY_ID_TYPE;
        else if (pIter->Name.equalsAsciiL(PROPERTY_PRIVILEGES.ascii, PROPERTY_PRIVILEGES.length))
            pIter->Handle = PROPERTY_ID_PRIVILEGES;
    }

    describeProperties(aTableProps);

    return new ::cppu::OPropertyArrayHelper(aTableProps);
}

::cppu::IPropertyArrayHelper & SAL_CALL ODBTableDecorator::getInfoHelper()
{
    Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);

    Reference<XPropertySetInfo> xInfo = xProp->getPropertySetInfo();
    bool bIsDescriptor = (xInfo->getPropertyByName(PROPERTY_NAME).Attributes & PropertyAttribute::READONLY) == 0;

    return *ODBTableDecorator_PROP::getArrayHelper( bIsDescriptor ? 0 : 1 );

    // TODO: this is a HACK, and prone to errors
    // The OIdPropertyArrayUsageHelper is intended for classes where there exists a known, limited
    // number of different property set infos (distinguished by the ID), all implemented by this very
    // same class.
    // However, in this case here we have an unknown, potentially unlimited number of different
    // property set infos: Depending on the table for which we act as decorator, different property
    // sets might exist.
}

// XServiceInfo
IMPLEMENT_SERVICE_INFO1(ODBTableDecorator, "com.sun.star.sdb.dbaccess.ODBTableDecorator", SERVICE_SDBCX_TABLE.ascii)

Any SAL_CALL ODBTableDecorator::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet;
    if(m_xTable.is())
    {
        aRet = m_xTable->queryInterface(rType);
        if(aRet.hasValue())
        {   // now we know that our table supports this type so we return ourself
            aRet = OTableDescriptor_BASE::queryInterface(rType);
            if(!aRet.hasValue())
                aRet = ODataSettings::queryInterface(rType);
        }
    }

    return aRet;
}

Sequence< Type > SAL_CALL ODBTableDecorator::getTypes(  ) throw(RuntimeException)
{
    Reference<XTypeProvider> xTypes(m_xTable,UNO_QUERY);
    OSL_ENSURE(xTypes.is(),"Table must be a TypePropvider!");
    return xTypes->getTypes();
}

// XRename,
void SAL_CALL ODBTableDecorator::rename( const ::rtl::OUString& _rNewName ) throw(SQLException, ElementExistException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::rename" );
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    Reference<XRename> xRename(m_xTable,UNO_QUERY);
    if(xRename.is())
    {
        xRename->rename(_rNewName);
    }
    else // not supported
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_TABLE_RENAME),*this,SQLSTATE_GENERAL,1000,Any() );
}

// XAlterTable,
void SAL_CALL ODBTableDecorator::alterColumnByName( const ::rtl::OUString& _rName, const Reference< XPropertySet >& _rxDescriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::alterColumnByName" );
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    Reference<XAlterTable> xAlter(m_xTable,UNO_QUERY);
    if(xAlter.is())
    {
        xAlter->alterColumnByName(_rName,_rxDescriptor);
    }
    else
        throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_ALTER_BY_NAME),*this,SQLSTATE_GENERAL,1000,Any() );
    if(m_pColumns)
        m_pColumns->refresh();
}

void SAL_CALL ODBTableDecorator::alterColumnByIndex( sal_Int32 _nIndex, const Reference< XPropertySet >& _rxDescriptor ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::alterColumnByIndex" );
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    Reference<XAlterTable> xAlter(m_xTable,UNO_QUERY);
    if(xAlter.is())
    {
        xAlter->alterColumnByIndex(_nIndex,_rxDescriptor);
        if(m_pColumns)
            m_pColumns->refresh();
    }
    else // not supported
        throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_ALTER_BY_INDEX),*this,SQLSTATE_GENERAL,1000,Any() );
}

Reference< XNameAccess> ODBTableDecorator::getIndexes() throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::getIndexes" );
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    return Reference< XIndexesSupplier>(m_xTable,UNO_QUERY)->getIndexes();
}

Reference< XIndexAccess> ODBTableDecorator::getKeys() throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::getKeys" );
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    return Reference< XKeysSupplier>(m_xTable,UNO_QUERY)->getKeys();
}

Reference< XNameAccess> ODBTableDecorator::getColumns() throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::getColumns" );
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    if(!m_pColumns)
        refreshColumns();

    return m_pColumns;
}

::rtl::OUString SAL_CALL ODBTableDecorator::getName() throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::getName" );
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    Reference<XNamed> xName(m_xTable,UNO_QUERY);
    OSL_ENSURE(xName.is(),"Table should support the XNamed interface");
    return xName->getName();
}

sal_Int64 SAL_CALL ODBTableDecorator::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::getSomething" );
    if (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    sal_Int64 nRet = 0;
    Reference<XUnoTunnel> xTunnel(m_xTable,UNO_QUERY);
    if(xTunnel.is())
        nRet = xTunnel->getSomething(rId);
    return nRet;
}

Sequence< sal_Int8 > ODBTableDecorator::getUnoTunnelImplementationId()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::getUnoTunnelImplementationId" );
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

void ODBTableDecorator::fillPrivileges() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::fillPrivileges" );
    // somebody is asking for the privileges and we do not know them, yet
    m_nPrivileges = 0;
    try
    {
        Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
        if ( xProp.is() )
        {
            if ( xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_PRIVILEGES) )
            {
                xProp->getPropertyValue(PROPERTY_PRIVILEGES) >>= m_nPrivileges;
            }
            if ( m_nPrivileges == 0 ) // second chance
            {
                ::rtl::OUString sCatalog,sSchema,sName;
                xProp->getPropertyValue(PROPERTY_CATALOGNAME)   >>= sCatalog;
                xProp->getPropertyValue(PROPERTY_SCHEMANAME)    >>= sSchema;
                xProp->getPropertyValue(PROPERTY_NAME)          >>= sName;
                m_nPrivileges = ::dbtools::getTablePrivileges(getMetaData(),sCatalog,sSchema, sName);
            }
        }
    }
    catch(const SQLException& e)
    {
        (void)e;
        OSL_FAIL("ODBTableDecorator::ODBTableDecorator : could not collect the privileges !");
    }
}

Reference< XPropertySet > SAL_CALL ODBTableDecorator::createDataDescriptor(  ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::createDataDescriptor" );
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    Reference< XDataDescriptorFactory > xFactory( m_xTable, UNO_QUERY );
    OSL_ENSURE( xFactory.is(), "ODBTableDecorator::createDataDescriptor: invalid table!" );
    Reference< XColumnsSupplier > xColsSupp;
    if ( xFactory.is() )
        xColsSupp = xColsSupp.query( xFactory->createDataDescriptor() );

    return new ODBTableDecorator(
        m_xConnection,
        xColsSupp,
        m_xNumberFormats,
        NULL
    );
}

Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL ODBTableDecorator::getPropertySetInfo(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::getPropertySetInfo" );
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

void ODBTableDecorator::refreshColumns()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::refreshColumns" );
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    ::std::vector< ::rtl::OUString> aVector;

    Reference<XNameAccess> xNames;
    if(m_xTable.is())
    {
        xNames = m_xTable->getColumns();
        if(xNames.is())
        {
            Sequence< ::rtl::OUString> aNames = xNames->getElementNames();
            const ::rtl::OUString* pIter    = aNames.getConstArray();
            const ::rtl::OUString* pEnd     = pIter + aNames.getLength();
            for(;pIter != pEnd;++pIter)
                aVector.push_back(*pIter);
        }
    }
    if(!m_pColumns)
    {
        OColumns* pCol = new OColumns(*this,m_aMutex,xNames,m_xMetaData.is() && m_xMetaData->supportsMixedCaseQuotedIdentifiers(),aVector,
                                    this,this,
                                    m_xMetaData.is() && m_xMetaData->supportsAlterTableWithAddColumn(),
                                    m_xMetaData.is() && m_xMetaData->supportsAlterTableWithDropColumn());

        pCol->setParent(*this);
        OContainerMediator* pMediator = new OContainerMediator( pCol, m_xColumnDefinitions, m_xConnection );
        m_xColumnMediator = pMediator;
        pCol->setMediator( pMediator );
        m_pColumns  = pCol;
    }
    else
        m_pColumns->reFill(aVector);
}

OColumn* ODBTableDecorator::createColumn(const ::rtl::OUString& _rName) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::createColumn" );
    OColumn* pReturn = NULL;

    Reference<XNameAccess> xNames;
    if ( m_xTable.is() )
    {
        xNames = m_xTable->getColumns();

        if ( xNames.is() && xNames->hasByName(_rName) )
        {
            Reference<XPropertySet> xProp(xNames->getByName(_rName),UNO_QUERY);

            Reference<XPropertySet> xColumnDefintion;
            if ( m_xColumnDefinitions.is() && m_xColumnDefinitions->hasByName(_rName))
                xColumnDefintion.set(m_xColumnDefinitions->getByName(_rName),UNO_QUERY);

            pReturn = new OTableColumnWrapper( xProp, xColumnDefintion, false );
        }
    }
    return pReturn;
}

void ODBTableDecorator::columnAppended( const Reference< XPropertySet >& /*_rxSourceDescriptor*/ )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::columnAppended" );
    // not interested in
}

void ODBTableDecorator::columnDropped(const ::rtl::OUString& _sName)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::columnDropped" );
    Reference<XDrop> xDrop(m_xColumnDefinitions,UNO_QUERY);
    if ( xDrop.is() && m_xColumnDefinitions->hasByName(_sName) )
        xDrop->dropByName(_sName);
}

Reference< XPropertySet > ODBTableDecorator::createColumnDescriptor()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::createColumnDescriptor" );
    Reference<XDataDescriptorFactory> xNames;
    if(m_xTable.is())
        xNames.set(m_xTable->getColumns(),UNO_QUERY);
    Reference< XPropertySet > xRet;
    if ( xNames.is() )
        xRet = new OTableColumnDescriptorWrapper( xNames->createDataDescriptor(), false, true );
    return xRet;
}

void SAL_CALL ODBTableDecorator::acquire() throw()
{
    OTableDescriptor_BASE::acquire();
}

void SAL_CALL ODBTableDecorator::release() throw()
{
    OTableDescriptor_BASE::release();
}

void SAL_CALL ODBTableDecorator::setName( const ::rtl::OUString& /*aName*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "ODBTableDecorator::setName" );
    throwFunctionNotSupportedException( "XNamed::setName", *this );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
