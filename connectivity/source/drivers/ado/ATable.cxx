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

#include "ado/ATable.hxx"
#include "ado/AIndexes.hxx"
#include "ado/AColumns.hxx"
#include "ado/AColumn.hxx"
#include "ado/AKeys.hxx"
#include "ado/AConnection.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include "ado/Awrapado.hxx"
#include <comphelper/sequence.hxx>
#include "TConnection.hxx"
#include <comphelper/types.hxx>

using namespace ::comphelper;

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;

// -------------------------------------------------------------------------
OAdoTable::OAdoTable(sdbcx::OCollection* _pTables,sal_Bool _bCase,OCatalog* _pCatalog,_ADOTable* _pTable)
    : OTable_TYPEDEF(_pTables,_bCase,::rtl::OUString(),::rtl::OUString())
    ,m_pCatalog(_pCatalog)
{
    construct();
    m_aTable = WpADOTable(_pTable);
    //  m_aTable.putref_ParentCatalog(_pCatalog->getCatalog());
    fillPropertyValues();

}
// -----------------------------------------------------------------------------
OAdoTable::OAdoTable(sdbcx::OCollection* _pTables,sal_Bool _bCase,OCatalog* _pCatalog)
    : OTable_TYPEDEF(_pTables,_bCase)
    ,m_pCatalog(_pCatalog)
{
    construct();
    m_aTable.Create();
    m_aTable.putref_ParentCatalog(_pCatalog->getCatalog());

}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoTable::disposing(void)
{
    OTable_TYPEDEF::disposing();
    m_aTable.clear();
}
// -------------------------------------------------------------------------
void OAdoTable::refreshColumns()
{
    TStringVector aVector;

    WpADOColumns aColumns;
    if ( m_aTable.IsValid() )
    {
        aColumns = m_aTable.get_Columns();
        aColumns.fillElementNames(aVector);
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns = new OColumns(*this,m_aMutex,aVector,aColumns,isCaseSensitive(),m_pCatalog->getConnection());
}
// -------------------------------------------------------------------------
void OAdoTable::refreshKeys()
{
    TStringVector aVector;

    WpADOKeys aKeys;
    if(m_aTable.IsValid())
    {
        aKeys = m_aTable.get_Keys();
        aKeys.fillElementNames(aVector);
    }

    if(m_pKeys)
        m_pKeys->reFill(aVector);
    else
        m_pKeys = new OKeys(*this,m_aMutex,aVector,aKeys,isCaseSensitive(),m_pCatalog->getConnection());
}
// -------------------------------------------------------------------------
void OAdoTable::refreshIndexes()
{
    TStringVector aVector;

    WpADOIndexes aIndexes;
    if(m_aTable.IsValid())
    {
        aIndexes = m_aTable.get_Indexes();
        aIndexes.fillElementNames(aVector);
    }

    if(m_pIndexes)
        m_pIndexes->reFill(aVector);
    else
        m_pIndexes = new OIndexes(*this,m_aMutex,aVector,aIndexes,isCaseSensitive(),m_pCatalog->getConnection());
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoTable::getUnoTunnelImplementationId()
{
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

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OAdoTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OTable_TYPEDEF::getSomething(rId);
}
// -------------------------------------------------------------------------
// XRename
void SAL_CALL OAdoTable::rename( const ::rtl::OUString& newName ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE_TYPEDEF::rBHelper.bDisposed);

    m_aTable.put_Name(newName);
    ADOS::ThrowException(*(m_pCatalog->getConnection()->getConnection()),*this);

    OTable_TYPEDEF::rename(newName);
}
// -----------------------------------------------------------------------------
Reference< XDatabaseMetaData> OAdoTable::getMetaData() const
{
    return m_pCatalog->getConnection()->getMetaData();
}
// -------------------------------------------------------------------------
// XAlterTable
void SAL_CALL OAdoTable::alterColumnByName( const ::rtl::OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE_TYPEDEF::rBHelper.bDisposed);

    sal_Bool bError = sal_True;
    OAdoColumn* pColumn = NULL;
    if(::comphelper::getImplementation(pColumn,descriptor) && pColumn != NULL)
    {
        WpADOColumns aColumns = m_aTable.get_Columns();
        bError = !aColumns.Delete(colName);
        bError = bError || !aColumns.Append(pColumn->getColumnImpl());
    }
    if(bError)
        ADOS::ThrowException(*(m_pCatalog->getConnection()->getConnection()),*this);

    m_pColumns->refresh();
    refreshColumns();
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoTable::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor ) throw(SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE_TYPEDEF::rBHelper.bDisposed);

    Reference< XPropertySet > xOld;
    m_pColumns->getByIndex(index) >>= xOld;
    if(xOld.is())
        alterColumnByName(getString(xOld->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),descriptor);
}
// -------------------------------------------------------------------------
void OAdoTable::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aTable.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                m_aTable.put_Name(getString(rValue));
                break;

            case PROPERTY_ID_TYPE:
                OTools::putValue(   m_aTable.get_Properties(),
                                OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),
                                getString(rValue));
                break;

            case PROPERTY_ID_DESCRIPTION:
                OTools::putValue(   m_aTable.get_Properties(),
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Description")),
                                getString(rValue));
                break;

            case PROPERTY_ID_SCHEMANAME:
                break;

            default:
                                throw Exception();
        }
    }
    OTable_TYPEDEF::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoTable::acquire() throw()
{
    OTable_TYPEDEF::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoTable::release() throw()
{
    OTable_TYPEDEF::release();
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OAdoTable::getName() throw(::com::sun::star::uno::RuntimeException)
{
      return m_aTable.get_Name();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
