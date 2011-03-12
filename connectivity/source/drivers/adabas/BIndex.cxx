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
#include "adabas/BIndex.hxx"
#include "adabas/BIndexColumns.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include "adabas/BTable.hxx"
#include <comphelper/types.hxx>

using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
OAdabasIndex::OAdabasIndex( OAdabasTable* _pTable,
                const ::rtl::OUString& _Name,
                const ::rtl::OUString& _Catalog,
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
// -------------------------------------------------------------------------
OAdabasIndex::OAdabasIndex(OAdabasTable* _pTable)
    : connectivity::sdbcx::OIndex(sal_True)
    ,m_pTable(_pTable)
{
    construct();
}
// -----------------------------------------------------------------------------

void OAdabasIndex::refreshColumns()
{
    if(!m_pTable)
        return;

    TStringVector aVector;
    if(!isNew())
    {
        Reference< XResultSet > xResult = m_pTable->getMetaData()->getIndexInfo(Any(),
        m_pTable->getSchema(),m_pTable->getTableName(),sal_False,sal_False);

        if(xResult.is())
        {
                    Reference< XRow > xRow(xResult,UNO_QUERY);
            ::rtl::OUString aColName;
            while(xResult->next())
            {
                if(xRow->getString(6) == m_Name)
                {
                    aColName = xRow->getString(9);
                    if(!xRow->wasNull())
                        aVector.push_back(aColName);
                }
            }
            ::comphelper::disposeComponent(xResult);
        }
    }
    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new OIndexColumns(this,m_aMutex,aVector);
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
