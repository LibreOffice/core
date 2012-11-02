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

#include "MColumns.hxx"
#include "connectivity/sdbcx/VColumn.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include "MTable.hxx"
#include "MTables.hxx"
#include "MCatalog.hxx"
#include <comphelper/types.hxx>
#include "connectivity/dbtools.hxx"

using namespace ::comphelper;

using namespace connectivity::mork;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

sdbcx::ObjectType OColumns::createObject(const ::rtl::OUString& _rName)
{
    const Any aCatalog;
    const ::rtl::OUString sCatalogName;
    const ::rtl::OUString sSchemaName(m_pTable->getSchema());
    const ::rtl::OUString sTableName(m_pTable->getTableName());
    Reference< XResultSet > xResult = m_pTable->getConnection()->getMetaData()->getColumns(
        aCatalog, sSchemaName, sTableName, _rName);

    sdbcx::ObjectType xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                sal_Int32 nType             = xRow->getInt(5);
                ::rtl::OUString sTypeName   = xRow->getString(6);
                sal_Int32 nPrec             = xRow->getInt(7);

                OColumn* pRet = new OColumn(_rName,
                                            sTypeName,
                                            xRow->getString(13),
                                            xRow->getString(12),
                                            xRow->getInt(11),
                                            nPrec,
                                            xRow->getInt(9),
                                            nType,
                                            sal_False,sal_False,sal_False,sal_True,
                                            sCatalogName,
                                            sSchemaName,
                                            sTableName);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshColumns();
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
