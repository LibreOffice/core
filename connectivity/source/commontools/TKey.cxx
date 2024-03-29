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

#include <TKey.hxx>
#include <TKeyColumns.hxx>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <TConnection.hxx>
#include <connectivity/TTableHelper.hxx>

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;

OTableKeyHelper::OTableKeyHelper(OTableHelper* _pTable) : connectivity::sdbcx::OKey(true)
    ,m_pTable(_pTable)
{
    construct();
}

OTableKeyHelper::OTableKeyHelper(   OTableHelper* _pTable
            ,const OUString& Name
            ,const std::shared_ptr<sdbcx::KeyProperties>& _rProps
            ) : connectivity::sdbcx::OKey(Name,_rProps,true)
                ,m_pTable(_pTable)
{
    construct();
    refreshColumns();
}

void OTableKeyHelper::refreshColumns()
{
    if ( !m_pTable )
        return;

    std::vector< OUString> aVector;
    if ( !isNew() )
    {
        aVector = m_aProps->m_aKeyColumnNames;
        if ( aVector.empty() )
        {
            ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
            OUString aSchema,aTable;
            m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
            m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

            if ( !m_Name.isEmpty() ) // foreign key
            {

                Reference< XResultSet > xResult = m_pTable->getMetaData()->getImportedKeys(m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
                    aSchema,aTable);

                if ( xResult.is() )
                {
                    Reference< XRow > xRow(xResult,UNO_QUERY);
                    while( xResult->next() )
                    {
                        OUString aForeignKeyColumn = xRow->getString(8);
                        if(xRow->getString(12) == m_Name)
                            aVector.push_back(aForeignKeyColumn);
                    }
                }
            }

            if ( aVector.empty() )
            {
                const Reference< XResultSet > xResult = m_pTable->getMetaData()->getPrimaryKeys(m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
                    aSchema,aTable);

                if ( xResult.is() )
                {
                    const Reference< XRow > xRow(xResult,UNO_QUERY);
                    while( xResult->next() )
                        aVector.push_back(xRow->getString(4));
                } // if ( xResult.is() )
            }
        }
    }


    if ( m_pColumns )
        m_pColumns->reFill(aVector);
    else
        m_pColumns.reset(new OKeyColumnsHelper(this,m_aMutex,aVector));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
