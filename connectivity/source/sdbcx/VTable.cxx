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

#include "connectivity/sdbcx/VTable.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "connectivity/sdbcx/VIndex.hxx"
#include <comphelper/sequence.hxx>
#include "connectivity/sdbcx/VCollection.hxx"
#include "TConnection.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include "connectivity/sdbcx/VKey.hxx"
#include "connectivity/dbtools.hxx"
#include <connectivity/dbexception.hxx>

// -------------------------------------------------------------------------
using namespace ::connectivity;
using namespace ::connectivity::sdbcx;
using namespace ::dbtools;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OTable::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if(isNew())
        return ::rtl::OUString("com.sun.star.sdbcx.VTableDescriptor");
    return ::rtl::OUString("com.sun.star.sdbcx.Table");
}

// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OTable::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);
    if(isNew())
        aSupported[0] = ::rtl::OUString("com.sun.star.sdbcx.TableDescriptor");
    else
        aSupported[0] = ::rtl::OUString("com.sun.star.sdbcx.Table");

    return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OTable::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
OTable::OTable(OCollection* _pTables,
               sal_Bool _bCase)
               : OTableDescriptor_BASE(m_aMutex)
                ,ODescriptor(OTableDescriptor_BASE::rBHelper,_bCase,sal_True)
                ,m_pKeys(NULL)
                ,m_pColumns(NULL)
                ,m_pIndexes(NULL)
                ,m_pTables(_pTables)
{
}
// -----------------------------------------------------------------------------
OTable::OTable( OCollection*    _pTables,
                sal_Bool _bCase,
                const ::rtl::OUString& _Name,       const ::rtl::OUString& _Type,
                const ::rtl::OUString& _Description,const ::rtl::OUString& _SchemaName,
                const ::rtl::OUString& _CatalogName) :  OTableDescriptor_BASE(m_aMutex)
                ,ODescriptor(OTableDescriptor_BASE::rBHelper,_bCase)
                ,m_CatalogName(_CatalogName)
                ,m_SchemaName(_SchemaName)
                ,m_Description(_Description)
                ,m_Type(_Type)
                ,m_pKeys(NULL)
                ,m_pColumns(NULL)
                ,m_pIndexes(NULL)
                ,m_pTables(_pTables)
{
    m_Name = _Name;
}
// -------------------------------------------------------------------------
OTable::~OTable()
{
    delete m_pKeys;
    delete m_pColumns;
    delete m_pIndexes;
}
// -------------------------------------------------------------------------
void OTable::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOGNAME),     PROPERTY_ID_CATALOGNAME,nAttrib,&m_CatalogName, ::getCppuType(static_cast< ::rtl::OUString*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME),      PROPERTY_ID_SCHEMANAME, nAttrib,&m_SchemaName,  ::getCppuType(static_cast< ::rtl::OUString*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION),     PROPERTY_ID_DESCRIPTION,nAttrib,&m_Description, ::getCppuType(static_cast< ::rtl::OUString*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),            PROPERTY_ID_TYPE,       nAttrib,&m_Type,        ::getCppuType(static_cast< ::rtl::OUString*>(0)));
}
// -----------------------------------------------------------------------------
void SAL_CALL OTable::acquire() throw()
{
    OTableDescriptor_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OTable::release() throw()
{
    OTableDescriptor_BASE::release();
}

// -------------------------------------------------------------------------
Any SAL_CALL OTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    if(!aRet.hasValue())
    {
        if(!isNew())
            aRet = OTable_BASE::queryInterface( rType);
        if(isNew() && (rType == getCppuType( (Reference<XIndexesSupplier>*)0)))
            return Any();
        if(!aRet.hasValue())
            aRet = OTableDescriptor_BASE::queryInterface( rType);
    }
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OTable::getTypes(  ) throw(RuntimeException)
{
    if(isNew())
        return ::comphelper::concatSequences(ODescriptor::getTypes(),OTableDescriptor_BASE::getTypes());
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OTableDescriptor_BASE::getTypes(),OTable_BASE::getTypes());
}
// -------------------------------------------------------------------------
void SAL_CALL OTable::disposing(void)
{
    ODescriptor::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pKeys)
        m_pKeys->disposing();
    if(m_pColumns)
        m_pColumns->disposing();
    if(m_pIndexes)
        m_pIndexes->disposing();

    m_pTables = NULL;
}
// -----------------------------------------------------------------------------
// XColumnsSupplier
Reference< XNameAccess > SAL_CALL OTable::getColumns(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_pColumns )
            refreshColumns();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return m_pColumns;
}

// -------------------------------------------------------------------------
// XKeysSupplier
Reference< XIndexAccess > SAL_CALL OTable::getKeys(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    Reference< XIndexAccess > xKeys;

    try
    {
        if ( !m_pKeys )
            refreshKeys();
        xKeys = m_pKeys;
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return xKeys;
}
// -----------------------------------------------------------------------------
cppu::IPropertyArrayHelper* OTable::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -------------------------------------------------------------------------
cppu::IPropertyArrayHelper & OTable::getInfoHelper()
{
    return *const_cast<OTable*>(this)->getArrayHelper(isNew() ? 1 : 0);
}
// -------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OTable::createDataDescriptor(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    OTable* pTable = new OTable(m_pTables,isCaseSensitive(),m_Name,m_Type,m_Description,m_SchemaName,m_CatalogName);
    pTable->setNew(sal_True);
    return pTable;
}
// -------------------------------------------------------------------------
// XIndexesSupplier
Reference< XNameAccess > SAL_CALL OTable::getIndexes(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_pIndexes )
            refreshIndexes();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return m_pIndexes;
}
// -------------------------------------------------------------------------
// XRename
void SAL_CALL OTable::rename( const ::rtl::OUString& newName ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    const ::rtl::OUString sOldComposedName = getName();
    const Reference< XDatabaseMetaData> xMetaData = getMetaData();
    if ( xMetaData.is() )
        ::dbtools::qualifiedNameComponents(xMetaData,newName,m_CatalogName,m_SchemaName,m_Name,::dbtools::eInDataManipulation);
    else
        m_Name = newName;

    m_pTables->renameObject(sOldComposedName,newName);
}
// -----------------------------------------------------------------------------
Reference< XDatabaseMetaData> OTable::getMetaData() const
{
    return NULL;
}
// -------------------------------------------------------------------------
// XAlterTable
void SAL_CALL OTable::alterColumnByName( const ::rtl::OUString& /*colName*/, const Reference< XPropertySet >& /*descriptor*/ ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    throwFeatureNotImplementedException( "XAlterTable::alterColumnByName", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL OTable::alterColumnByIndex( sal_Int32 /*index*/, const Reference< XPropertySet >& /*descriptor*/ ) throw(SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException)
{
    throwFeatureNotImplementedException( "XAlterTable::alterColumnByIndex", *this );
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OTable::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OTable::getName() throw(::com::sun::star::uno::RuntimeException)
{
    // this is only correct for tables who haven't a schema or catalog name
    OSL_ENSURE(m_CatalogName.isEmpty(),"getName(): forgot to overload getName()!");
    OSL_ENSURE(m_SchemaName.isEmpty(),"getName(): forgot to overload getName()!");
    return m_Name;
}
// -----------------------------------------------------------------------------
void SAL_CALL OTable::setName( const ::rtl::OUString& /*aName*/ ) throw(::com::sun::star::uno::RuntimeException)
{
}
// -----------------------------------------------------------------------------
void OTable::refreshColumns()
{
}
// -----------------------------------------------------------------------------
void OTable::refreshKeys()
{
}
// -----------------------------------------------------------------------------
void OTable::refreshIndexes()
{
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
