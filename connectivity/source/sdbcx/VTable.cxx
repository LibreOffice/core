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

#include <connectivity/sdbcx/VTable.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <connectivity/sdbcx/VIndex.hxx>
#include <comphelper/sequence.hxx>
#include <connectivity/sdbcx/VCollection.hxx>
#include <TConnection.hxx>
#include <connectivity/sdbcx/VColumn.hxx>
#include <connectivity/sdbcx/VKey.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::connectivity;
using namespace ::connectivity::sdbcx;
using namespace ::dbtools;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


OUString SAL_CALL OTable::getImplementationName(  )
{
    if(isNew())
        return OUString("com.sun.star.sdbcx.VTableDescriptor");
    return OUString("com.sun.star.sdbcx.Table");
}


css::uno::Sequence< OUString > SAL_CALL OTable::getSupportedServiceNames(  )
{
    css::uno::Sequence< OUString > aSupported(1);
    if(isNew())
        aSupported[0] = "com.sun.star.sdbcx.TableDescriptor";
    else
        aSupported[0] = "com.sun.star.sdbcx.Table";

    return aSupported;
}

sal_Bool SAL_CALL OTable::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

OTable::OTable(OCollection* _pTables,
               bool _bCase)
               : OTableDescriptor_BASE(m_aMutex)
                ,ODescriptor(OTableDescriptor_BASE::rBHelper,_bCase,true)
                ,m_pTables(_pTables)
{
}

OTable::OTable( OCollection*    _pTables,
                bool            _bCase,
                const OUString& Name,       const OUString& Type,
                const OUString& Description,const OUString& SchemaName,
                const OUString& CatalogName) :  OTableDescriptor_BASE(m_aMutex)
                ,ODescriptor(OTableDescriptor_BASE::rBHelper,_bCase)
                ,m_CatalogName(CatalogName)
                ,m_SchemaName(SchemaName)
                ,m_Description(Description)
                ,m_Type(Type)
                ,m_pTables(_pTables)
{
    m_Name = Name;
}

OTable::~OTable()
{
}

void OTable::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOGNAME),     PROPERTY_ID_CATALOGNAME,nAttrib,&m_CatalogName, ::cppu::UnoType<OUString>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME),      PROPERTY_ID_SCHEMANAME, nAttrib,&m_SchemaName,  ::cppu::UnoType<OUString>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION),     PROPERTY_ID_DESCRIPTION,nAttrib,&m_Description, ::cppu::UnoType<OUString>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),            PROPERTY_ID_TYPE,       nAttrib,&m_Type,        ::cppu::UnoType<OUString>::get());
}

void SAL_CALL OTable::acquire() throw()
{
    OTableDescriptor_BASE::acquire();
}

void SAL_CALL OTable::release() throw()
{
    OTableDescriptor_BASE::release();
}


Any SAL_CALL OTable::queryInterface( const Type & rType )
{
    Any aRet = ODescriptor::queryInterface( rType);
    if(!aRet.hasValue())
    {
        if(!isNew())
            aRet = OTable_BASE::queryInterface( rType);
        if(isNew() && (rType == cppu::UnoType<XIndexesSupplier>::get()))
            return Any();
        if(!aRet.hasValue())
            aRet = OTableDescriptor_BASE::queryInterface( rType);
    }
    return aRet;
}

Sequence< Type > SAL_CALL OTable::getTypes(  )
{
    if(isNew())
        return ::comphelper::concatSequences(ODescriptor::getTypes(),OTableDescriptor_BASE::getTypes());
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OTableDescriptor_BASE::getTypes(),OTable_BASE::getTypes());
}

void SAL_CALL OTable::disposing()
{
    ODescriptor::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_xKeys)
        m_xKeys->disposing();
    if(m_xColumns)
        m_xColumns->disposing();
    if(m_xIndexes)
        m_xIndexes->disposing();

    m_pTables = nullptr;
}

// XColumnsSupplier
Reference< XNameAccess > SAL_CALL OTable::getColumns(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_xColumns )
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

    return m_xColumns.get();
}


// XKeysSupplier
Reference< XIndexAccess > SAL_CALL OTable::getKeys(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    Reference< XIndexAccess > xKeys;

    try
    {
        if ( !m_xKeys )
            refreshKeys();
        xKeys = m_xKeys.get();
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

cppu::IPropertyArrayHelper* OTable::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}

cppu::IPropertyArrayHelper & OTable::getInfoHelper()
{
    return *getArrayHelper(isNew() ? 1 : 0);
}

Reference< XPropertySet > SAL_CALL OTable::createDataDescriptor(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    OTable* pTable = new OTable(m_pTables,isCaseSensitive(),m_Name,m_Type,m_Description,m_SchemaName,m_CatalogName);
    pTable->setNew(true);
    return pTable;
}

// XIndexesSupplier
Reference< XNameAccess > SAL_CALL OTable::getIndexes(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_xIndexes )
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

    return m_xIndexes.get();
}

// XRename
void SAL_CALL OTable::rename( const OUString& newName )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    const OUString sOldComposedName = getName();
    const Reference< XDatabaseMetaData> xMetaData = getMetaData();
    if ( xMetaData.is() )
        ::dbtools::qualifiedNameComponents(xMetaData,newName,m_CatalogName,m_SchemaName,m_Name,::dbtools::EComposeRule::InDataManipulation);
    else
        m_Name = newName;

    m_pTables->renameObject(sOldComposedName,newName);
}

Reference< XDatabaseMetaData> OTable::getMetaData() const
{
    return nullptr;
}

// XAlterTable
void SAL_CALL OTable::alterColumnByName( const OUString& /*colName*/, const Reference< XPropertySet >& /*descriptor*/ )
{
    throwFeatureNotImplementedSQLException( "XAlterTable::alterColumnByName", *this );
}

void SAL_CALL OTable::alterColumnByIndex( sal_Int32 /*index*/, const Reference< XPropertySet >& /*descriptor*/ )
{
    throwFeatureNotImplementedSQLException( "XAlterTable::alterColumnByIndex", *this );
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OTable::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

OUString SAL_CALL OTable::getName()
{
    // this is only correct for tables who haven't a schema or catalog name
    OSL_ENSURE(m_CatalogName.isEmpty(),"getName(): forgot to override getName()!");
    OSL_ENSURE(m_SchemaName.isEmpty(),"getName(): forgot to override getName()!");
    return m_Name;
}

void SAL_CALL OTable::setName( const OUString& /*aName*/ )
{
}

void OTable::refreshColumns()
{
}

void OTable::refreshKeys()
{
}

void OTable::refreshIndexes()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
