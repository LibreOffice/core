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

#include <sal/config.h>

#include <string_view>

#include <ado/ATable.hxx>
#include <ado/AIndexes.hxx>
#include <ado/AColumns.hxx>
#include <ado/AColumn.hxx>
#include <ado/AKeys.hxx>
#include <ado/AConnection.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <ado/Awrapado.hxx>
#include <TConnection.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/types.hxx>

using namespace ::comphelper;

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;


OAdoTable::OAdoTable(sdbcx::OCollection* _pTables,bool _bCase,OCatalog* _pCatalog,_ADOTable* _pTable)
    : OTable_TYPEDEF(_pTables,_bCase)
    ,m_pCatalog(_pCatalog)
{
    construct();
    m_aTable = WpADOTable(_pTable);
    //  m_aTable.putref_ParentCatalog(_pCatalog->getCatalog());
    fillPropertyValues();

}

OAdoTable::OAdoTable(sdbcx::OCollection* _pTables,bool _bCase,OCatalog* _pCatalog)
    : OTable_TYPEDEF(_pTables,_bCase)
    ,m_pCatalog(_pCatalog)
{
    construct();
    m_aTable.Create();
    m_aTable.putref_ParentCatalog(_pCatalog->getCatalog());

}

void SAL_CALL OAdoTable::disposing()
{
    OTable_TYPEDEF::disposing();
    m_aTable.clear();
}

void OAdoTable::refreshColumns()
{
    ::std::vector< OUString> aVector;

    WpADOColumns aColumns;
    if ( m_aTable.IsValid() )
    {
        aColumns = m_aTable.get_Columns();
        aColumns.fillElementNames(aVector);
    }

    if(m_xColumns)
        m_xColumns->reFill(aVector);
    else
        m_xColumns = new OColumns(*this,m_aMutex,aVector,aColumns,isCaseSensitive(),m_pCatalog->getConnection());
}

void OAdoTable::refreshKeys()
{
    ::std::vector< OUString> aVector;

    WpADOKeys aKeys;
    if(m_aTable.IsValid())
    {
        aKeys = m_aTable.get_Keys();
        aKeys.fillElementNames(aVector);
    }

    if(m_xKeys)
        m_xKeys->reFill(aVector);
    else
        m_xKeys = new OKeys(*this,m_aMutex,aVector,aKeys,isCaseSensitive(),m_pCatalog->getConnection());
}

void OAdoTable::refreshIndexes()
{
    ::std::vector< OUString> aVector;

    WpADOIndexes aIndexes;
    if(m_aTable.IsValid())
    {
        aIndexes = m_aTable.get_Indexes();
        aIndexes.fillElementNames(aVector);
    }

    if(m_xIndexes)
        m_xIndexes->reFill(aVector);
    else
        m_xIndexes = new OIndexes(*this,m_aMutex,aVector,aIndexes,isCaseSensitive(),m_pCatalog->getConnection());
}

Sequence< sal_Int8 > OAdoTable::getUnoTunnelId()
{
    static ::cppu::OImplementationId implId;

    return implId.getImplementationId();
}

// css::lang::XUnoTunnel

sal_Int64 OAdoTable::getSomething( const Sequence< sal_Int8 > & rId )
{
    return isUnoTunnelId<OAdoTable>(rId)
                ? reinterpret_cast< sal_Int64 >( this )
                : OTable_TYPEDEF::getSomething(rId);
}

// XRename
void SAL_CALL OAdoTable::rename( const OUString& newName )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE_TYPEDEF::rBHelper.bDisposed);

    m_aTable.put_Name(newName);
    ADOS::ThrowException(*(m_pCatalog->getConnection()->getConnection()),*this);

    OTable_TYPEDEF::rename(newName);
}

Reference< XDatabaseMetaData> OAdoTable::getMetaData() const
{
    return m_pCatalog->getConnection()->getMetaData();
}

// XAlterTable
void SAL_CALL OAdoTable::alterColumnByName( const OUString& colName, const Reference< XPropertySet >& descriptor )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE_TYPEDEF::rBHelper.bDisposed);

    bool bError = true;
    OAdoColumn* pColumn = comphelper::getUnoTunnelImplementation<OAdoColumn>(descriptor);
    if(pColumn != nullptr)
    {
        WpADOColumns aColumns = m_aTable.get_Columns();
        bError = !aColumns.Delete(colName);
        bError = bError || !aColumns.Append(pColumn->getColumnImpl());
    }
    if(bError)
        ADOS::ThrowException(*(m_pCatalog->getConnection()->getConnection()),*this);

    m_xColumns->refresh();
    refreshColumns();
}

void SAL_CALL OAdoTable::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE_TYPEDEF::rBHelper.bDisposed);

    Reference< XPropertySet > xOld;
    m_xColumns->getByIndex(index) >>= xOld;
    if(xOld.is())
        alterColumnByName(getString(xOld->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))),descriptor);
}

void OAdoTable::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
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
                                std::u16string_view(u"Description"),
                                getString(rValue));
                break;

            case PROPERTY_ID_SCHEMANAME:
                break;

            default:
                throw Exception("unknown prop " + OUString::number(nHandle), nullptr);
        }
    }
    OTable_TYPEDEF::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}

OUString SAL_CALL OAdoTable::getName()
{
      return m_aTable.get_Name();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
