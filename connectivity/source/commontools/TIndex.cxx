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

#include "connectivity/TIndex.hxx"
#include "connectivity/TIndexColumns.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "connectivity/TTableHelper.hxx"
#include "TConnection.hxx"

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OIndexHelper::OIndexHelper( OTableHelper* _pTable) : connectivity::sdbcx::OIndex(sal_True)
                 , m_pTable(_pTable)
{
    construct();
    ::std::vector< OUString> aVector;
    m_pColumns  = new OIndexColumns(this,m_aMutex,aVector);
}

OIndexHelper::OIndexHelper( OTableHelper* _pTable,
                const OUString& _Name,
                const OUString& _Catalog,
                sal_Bool _isUnique,
                sal_Bool _isPrimaryKeyIndex,
                sal_Bool _isClustered
                ) : connectivity::sdbcx::OIndex(_Name,
                                  _Catalog,
                                  _isUnique,
                                  _isPrimaryKeyIndex,
                                  _isClustered,sal_True)
                ,m_pTable(_pTable)
{
    construct();
    refreshColumns();
}


void OIndexHelper::refreshColumns()
{
    if ( !m_pTable )
        return;

    ::std::vector< OUString> aVector;
    if ( !isNew() )
    {
        ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
        OUString aSchema,aTable;
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

        Reference< XResultSet > xResult = m_pTable->getMetaData()->getIndexInfo(
            m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
            aSchema,aTable,sal_False,sal_False);

        if ( xResult.is() )
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            OUString aColName;
            while( xResult->next() )
            {
                if ( xRow->getString(6) == m_Name )
                {
                    aColName = xRow->getString(9);
                    if ( !xRow->wasNull() )
                        aVector.push_back(aColName);
                }
            }
        }
    }
    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new OIndexColumns(this,m_aMutex,aVector);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
