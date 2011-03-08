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

#include "adabas/BIndexColumns.hxx"
#include "connectivity/sdbcx/VIndexColumn.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include "adabas/BTable.hxx"
#include "adabas/BCatalog.hxx"
#include <comphelper/types.hxx>
#include <comphelper/property.hxx>

using namespace connectivity::adabas;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
ObjectType OIndexColumns::createObject(const ::rtl::OUString& _rName)
{

    Reference< XResultSet > xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getIndexInfo(Any(),
                    m_pIndex->getTable()->getSchema(),m_pIndex->getTable()->getTableName(),sal_False,sal_False);

    sal_Bool bAsc = sal_True;
    if(xResult.is())
    {
                Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aD(RTL_CONSTASCII_USTRINGPARAM("D"));
        while(xResult->next())
        {
            if(xRow->getString(9) == _rName)
                bAsc = xRow->getString(10) != aD;
        }
        ::comphelper::disposeComponent(xResult);
    }

    xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getColumns(Any(),
            m_pIndex->getTable()->getSchema(),m_pIndex->getTable()->getTableName(),_rName);

    ObjectType xRet = NULL;
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
                OAdabasCatalog::correctColumnProperties(nPrec,nType,sTypeName);

                OIndexColumn* pRet = new OIndexColumn(bAsc,
                                                    _rName,
                                                    sTypeName,
                                                    xRow->getString(13),
                                                    xRow->getInt(11),
                                                    nPrec,
                                                    xRow->getInt(9),
                                                    nType,
                                                    sal_False,sal_False,sal_False,sal_True);
                xRet = pRet;
                break;
            }
        }
        ::comphelper::disposeComponent(xResult);
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexColumns::createDescriptor()
{
    return new OIndexColumn(sal_True);
}
// -----------------------------------------------------------------------------
void OIndexColumns::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
    m_pIndex->refreshColumns();
}
// -----------------------------------------------------------------------------
ObjectType OIndexColumns::appendObject( const ::rtl::OUString& /*_rForName*/, const Reference< XPropertySet >& descriptor )
{
    return cloneDescriptor( descriptor );
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
