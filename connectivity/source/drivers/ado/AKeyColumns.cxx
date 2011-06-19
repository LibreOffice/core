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
#include "adabas/BKeyColumns.hxx"
#include "adabas/BKeyColumn.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include "adabas/BTable.hxx"

using namespace connectivity::adabas;
// -------------------------------------------------------------------------
sdbcx::ObjectType OKeyColumns::createObject(const ::rtl::OUString& _rName)
{

    Reference< starsdbc::XResultSet >
        xResult = m_pTable->getConnection()->getMetaData()->getImportedKeys(Any(),
                    m_pTable->getSchema(),m_pTable->getName());

    ::rtl::OUString aRefColumnName;
    if(xResult.is())
    {
        Reference< starsdbc::XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(8) == _rName)
            {
                aRefColumnName = xRow->getString(4);
                break;
            }
        }
    }

     xResult = m_pTable->getConnection()->getMetaData()->getColumns(Any(),
        m_pTable->getSchema(),m_pTable->getName(),_rName);

    sdbcx::ObjectType xRet = NULL;
    if(xResult.is())
    {
        Reference< starsdbc::XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                OKeyColumn* pRet = new OKeyColumn(aRefColumnName,
                                                    _rName,
                                                    xRow->getString(6),
                                                    xRow->getString(13),
                                                    xRow->getInt(11),
                                                    xRow->getInt(7),
                                                    xRow->getInt(9),
                                                    xRow->getInt(5),
                                                    sal_False);
                xRet = pRet;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeyColumns::createDescriptor()
{
    OKeyColumn* pNew = new OKeyColumn();
    return pNew;
}
// -------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
