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

#include <connectivity/TKeyColumns.hxx>
#include <connectivity/sdbcx/VKeyColumn.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/property.hxx>
#include "TConnection.hxx"
#include <connectivity/TTableHelper.hxx>

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


OKeyColumnsHelper::OKeyColumnsHelper(   OTableKeyHelper* _pKey,
                ::osl::Mutex& _rMutex,
                const ::std::vector< OUString> &_rVector)
            : connectivity::sdbcx::OCollection(*_pKey,true,_rMutex,_rVector)
            ,m_pKey(_pKey)
{
}

sdbcx::ObjectType OKeyColumnsHelper::createObject(const OUString& _rName)
{
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    OUString aCatalog, aSchema, aTable;
    css::uno::Any Catalog(m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)));
    Catalog >>= aCatalog;
    m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))   >>= aSchema;
    m_pKey->getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))         >>= aTable;

    // frist get the related column to _rName
    Reference< XResultSet > xResult = m_pKey->getTable()->getMetaData()->getImportedKeys(
            Catalog, aSchema, aTable);

    OUString aRefColumnName;
    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        OUString aTemp;
        while(xResult->next())
        {
            aTemp = xRow->getString(4);
            if(xRow->getString(8) == _rName && m_pKey->getName() == xRow->getString(12))
            {
                aRefColumnName = aTemp;
                break;
            }
        }
    }

    sdbcx::ObjectType xRet;

    // now describe the column _rName and set his related column
    xResult = m_pKey->getTable()->getMetaData()->getColumns(Catalog, aSchema, aTable, _rName);

    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if ( xResult->next() )
        {
            if ( xRow->getString(4) == _rName )
            {
                sal_Int32 nDataType = xRow->getInt(5);
                OUString aTypeName(xRow->getString(6));
                sal_Int32 nSize = xRow->getInt(7);
                sal_Int32 nDec  = xRow->getInt(9);
                sal_Int32 nNull = xRow->getInt(11);
                OUString sColumnDef;
                try
                {
                    sColumnDef = xRow->getString(13);
                }
                catch(const SQLException&)
                {
                    // sometimes we get an error when asking for this param
                }

                OKeyColumn* pRet = new OKeyColumn(aRefColumnName,
                                                  _rName,
                                                  aTypeName,
                                                  sColumnDef,
                                                  nNull,
                                                  nSize,
                                                  nDec,
                                                  nDataType,
                                                  isCaseSensitive(),
                                                  aCatalog,
                                                  aSchema,
                                                  aTable);
                xRet = pRet;
            }
        }
    }

    return xRet;
}

Reference< XPropertySet > OKeyColumnsHelper::createDescriptor()
{
    return new OKeyColumn(isCaseSensitive());
}

void OKeyColumnsHelper::impl_refresh()
{
    m_pKey->refreshColumns();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
