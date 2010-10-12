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

#include "KColumns.hxx"
#include "KTable.hxx"
#include "KTables.hxx"
#include "KCatalog.hxx"
#include "connectivity/sdbcx/VColumn.hxx"

using namespace connectivity::kab;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
sdbcx::ObjectType KabColumns::createObject(const ::rtl::OUString& _rName)
{
    Reference< XResultSet > xResult = m_pTable->getConnection()->getMetaData()->getColumns(
        Any(),
        m_pTable->getSchema(),
        m_pTable->getTableName(),
        _rName);

    sdbcx::ObjectType xRet = NULL;
    if (xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);

        while (xResult->next())
        {
            if (xRow->getString(4) == _rName)
            {
                OColumn* pRet = new OColumn(
                        _rName,
                        xRow->getString(6),
                        xRow->getString(13),
                        xRow->getString(12),
                        xRow->getInt(11),
                        xRow->getInt(7),
                        xRow->getInt(9),
                        xRow->getInt(5),
                        sal_False,
                        sal_False,
                        sal_False,
                        sal_True);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
void KabColumns::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshColumns();
}
// -------------------------------------------------------------------------
KabColumns::KabColumns( KabTable* _pTable,
                        ::osl::Mutex& _rMutex,
                        const TStringVector &_rVector)
    : sdbcx::OCollection(*_pTable, sal_True, _rMutex, _rVector),
      m_pTable(_pTable)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
