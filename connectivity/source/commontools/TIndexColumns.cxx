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

#include "connectivity/TIndexColumns.hxx"
#include "connectivity/sdbcx/VIndexColumn.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/property.hxx>
#include "connectivity/TIndex.hxx"
#include "connectivity/TTableHelper.hxx"
#include "TConnection.hxx"

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OIndexColumns::OIndexColumns(   OIndexHelper* _pIndex,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector)
            : connectivity::sdbcx::OCollection(*_pIndex,sal_True,_rMutex,_rVector)
            ,m_pIndex(_pIndex)
{
}
// -------------------------------------------------------------------------
sdbcx::ObjectType OIndexColumns::createObject(const ::rtl::OUString& _rName)
{
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    ::rtl::OUString aSchema,aTable;
    m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
    m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

    Reference< XResultSet > xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getIndexInfo(
        m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
        aSchema,aTable,sal_False,sal_False);

    sal_Bool bAsc = sal_True;
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aD(RTL_CONSTASCII_USTRINGPARAM("D"));
        while( xResult->next() )
        {
            if(xRow->getString(9) == _rName)
                bAsc = xRow->getString(10) != aD;
        }
    }

    xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getColumns(
        m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
        aSchema,aTable,_rName);

    sdbcx::ObjectType xRet;
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while( xResult->next() )
        {
            if ( xRow->getString(4) == _rName )
            {
                sal_Int32 nDataType = xRow->getInt(5);
                ::rtl::OUString aTypeName(xRow->getString(6));
                sal_Int32 nSize = xRow->getInt(7);
                sal_Int32 nDec  = xRow->getInt(9);
                sal_Int32 nNull = xRow->getInt(11);
                ::rtl::OUString aColumnDef(xRow->getString(13));

                OIndexColumn* pRet = new OIndexColumn(bAsc,
                                                      _rName,
                                                      aTypeName,
                                                      aColumnDef,
                                                      nNull,
                                                      nSize,
                                                      nDec,
                                                      nDataType,
                                                      sal_False,sal_False,sal_False,sal_True);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexColumns::createDescriptor()
{
    return new OIndexColumn(sal_True);
}
// -------------------------------------------------------------------------
void OIndexColumns::impl_refresh() throw(RuntimeException)
{
    m_pIndex->refreshColumns();
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
