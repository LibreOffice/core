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

#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <fldmgr.hxx>
#include <dbmgr.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;

// This file contains all routines of the fldui directory, which must compile
// with exceptions. So we can reduce the code of the other files, which don't
// need any exception handling.

// Is the database field numeric?
// remark: in case of error true is returned
bool SwFieldMgr::IsDBNumeric( const OUString& rDBName, const OUString& rTableQryName,
                            bool bIsTable, const OUString& rFieldName)
{
    bool bNumeric = true;

    SwDBManager* pDBManager;
    if (m_pWrtShell)
        pDBManager = m_pWrtShell->GetDBManager();
    else
    {
        if (SwView* pView = GetActiveView())
            pDBManager = pView->GetWrtShell().GetDBManager();
        else
            return bNumeric;
    }

    Reference< XConnection> xConnection =
                    pDBManager->RegisterConnection(rDBName);

    if( !xConnection.is() )
        return bNumeric;

    Reference<XColumnsSupplier> xColsSupplier;
    if(bIsTable)
    {
        Reference<XTablesSupplier> xTSupplier(xConnection, UNO_QUERY);
        if(xTSupplier.is())
        {
            Reference<XNameAccess> xTables = xTSupplier->getTables();
            OSL_ENSURE(xTables->hasByName(rTableQryName), "table not available anymore?");
            try
            {
                Any aTable = xTables->getByName(rTableQryName);
                Reference<XPropertySet> xPropSet;
                aTable >>= xPropSet;
                xColsSupplier.set(xPropSet, UNO_QUERY);
            }
            catch (const Exception&)
            {
            }
        }
    }
    else
    {
        Reference<XQueriesSupplier> xQSupplier(xConnection, UNO_QUERY);
        if(xQSupplier.is())
        {
            Reference<XNameAccess> xQueries = xQSupplier->getQueries();
            OSL_ENSURE(xQueries->hasByName(rTableQryName), "table not available anymore?");
            try
            {
                Any aQuery = xQueries->getByName(rTableQryName);
                Reference<XPropertySet> xPropSet;
                aQuery >>= xPropSet;
                xColsSupplier.set(xPropSet, UNO_QUERY);
            }
            catch (const Exception&)
            {
            }
        }
    }

    if(xColsSupplier.is())
    {
        Reference <XNameAccess> xCols;
        try
        {
            xCols = xColsSupplier->getColumns();
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION( "sw", "getColumns()");
        }
        if(xCols.is() && xCols->hasByName(rFieldName))
        {
            Any aCol = xCols->getByName(rFieldName);
            Reference <XPropertySet> xCol;
            aCol >>= xCol;
            Any aType = xCol->getPropertyValue("Type");
            sal_Int32 eDataType = 0;
            aType >>= eDataType;
            switch(eDataType)
            {
                case DataType::BIT:
                case DataType::BOOLEAN:
                case DataType::TINYINT:
                case DataType::SMALLINT:
                case DataType::INTEGER:
                case DataType::BIGINT:
                case DataType::FLOAT:
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::NUMERIC:
                case DataType::DECIMAL:
                case DataType::DATE:
                case DataType::TIME:
                case DataType::TIMESTAMP:
                    break;

                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                case DataType::SQLNULL:
                case DataType::OTHER:
                case DataType::OBJECT:
                case DataType::DISTINCT:
                case DataType::STRUCT:
                case DataType::ARRAY:
                case DataType::BLOB:
                case DataType::CLOB:
                case DataType::REF:
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                default:
                    bNumeric = false;
            }
        }
    }
    return bNumeric;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
