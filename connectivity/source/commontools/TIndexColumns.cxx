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

#include <TIndexColumns.hxx>
#include <sdbcx/VIndexColumn.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <TIndex.hxx>
#include <connectivity/TTableHelper.hxx>
#include <TConnection.hxx>

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;

OIndexColumns::OIndexColumns(   OIndexHelper* _pIndex,
                        ::osl::Mutex& _rMutex,
                        const std::vector< OUString> &_rVector)
            : connectivity::sdbcx::OCollection(*_pIndex,true,_rMutex,_rVector)
            ,m_pIndex(_pIndex)
{
}

sdbcx::ObjectType OIndexColumns::createObject(const OUString& _rName)
{
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    OUString aCatalog, aSchema, aTable;
    css::uno::Any Catalog(m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)));
    Catalog >>= aCatalog;
    m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
    m_pIndex->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

    Reference< XResultSet > xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getIndexInfo(
        Catalog, aSchema, aTable, false, false);

    bool bAsc = true;
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while( xResult->next() )
        {
            if(xRow->getString(9) == _rName)
                bAsc = xRow->getString(10) != "D";
        }
    }

    xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getColumns(
        Catalog, aSchema, aTable, _rName);

    sdbcx::ObjectType xRet;
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while( xResult->next() )
        {
            if ( xRow->getString(4) == _rName )
            {
                sal_Int32 nDataType = xRow->getInt(5);
                OUString aTypeName(xRow->getString(6));
                sal_Int32 nSize = xRow->getInt(7);
                sal_Int32 nDec  = xRow->getInt(9);
                sal_Int32 nNull = xRow->getInt(11);
                OUString aColumnDef(xRow->getString(13));

                xRet = new OIndexColumn(bAsc,
                                        _rName,
                                        aTypeName,
                                        aColumnDef,
                                        nNull,
                                        nSize,
                                        nDec,
                                        nDataType,
                                        true,
                                        aCatalog, aSchema, aTable);
                break;
            }
        }
    }

    return xRet;
}

Reference< XPropertySet > OIndexColumns::createDescriptor()
{
    return new OIndexColumn(true);
}

void OIndexColumns::impl_refresh()
{
    m_pIndex->refreshColumns();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
