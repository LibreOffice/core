/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"


#include "connectivity/TKey.hxx"
#include "connectivity/TKeyColumns.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "TConnection.hxx"
#include "connectivity/TTableHelper.hxx"

using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OTableKeyHelper::OTableKeyHelper(OTableHelper* _pTable) : connectivity::sdbcx::OKey(sal_True)
    ,m_pTable(_pTable)
{
    construct();
}
// -------------------------------------------------------------------------
OTableKeyHelper::OTableKeyHelper(   OTableHelper* _pTable
            ,const ::rtl::OUString& _Name
            ,const sdbcx::TKeyProperties& _rProps
            ) : connectivity::sdbcx::OKey(_Name,_rProps,sal_True)
                ,m_pTable(_pTable)
{
    construct();
    refreshColumns();
}
// -------------------------------------------------------------------------
void OTableKeyHelper::refreshColumns()
{
    if ( !m_pTable )
        return;

    ::std::vector< ::rtl::OUString> aVector;
    if ( !isNew() )
    {
        aVector = m_aProps->m_aKeyColumnNames;
        if ( aVector.empty() )
        {
            ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
            ::rtl::OUString aSchema,aTable;
            m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
            m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

            if ( m_Name.getLength() ) // foreign key
            {

                Reference< XResultSet > xResult = m_pTable->getMetaData()->getImportedKeys(m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
                    aSchema,aTable);

                if ( xResult.is() )
                {
                    Reference< XRow > xRow(xResult,UNO_QUERY);
                    while( xResult->next() )
                    {
                        ::rtl::OUString aForeignKeyColumn = xRow->getString(8);
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
        m_pColumns ->reFill(aVector);
    else
        m_pColumns  = new OKeyColumnsHelper(this,m_aMutex,aVector);
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
