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
#include "MTables.hxx"
#include "MTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include "MCatalog.hxx"
#include "MConnection.hxx"
#include <comphelper/extract.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/dbexception.hxx"
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/types.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace ::cppu;
using namespace connectivity::mozab;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OTables::createObject(const ::rtl::OUString& _rName)
{
    ::rtl::OUString aName,aSchema;
    // sal_Int32 nLen = _rName.indexOf('.');
    // aSchema = _rName.copy(0,nLen);
    // aName    = _rName.copy(nLen+1);
    aSchema = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%"));
    aName = _rName;

    Sequence< ::rtl::OUString > aTypes(1);
    aTypes[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("%"));
    //  aTypes[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TABLE"));
    //  aTypes[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SYSTEMTABLE"));
    ::rtl::OUString sEmpty;

    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),aSchema,aName,aTypes);

    sdbcx::ObjectType xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next()) // there can be only one table with this name
        {
            OTable* pRet = new OTable(  this, static_cast<OCatalog&>(m_rParent).getConnection(),
                                        aName,xRow->getString(4),xRow->getString(5));
            xRet = pRet;
        }
    }
    ::comphelper::disposeComponent(xResult);

    return xRet;
}
// -------------------------------------------------------------------------
void OTables::impl_refresh(  ) throw(RuntimeException)
{
    static_cast<OCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void OTables::disposing(void)
{
m_xMetaData.clear();
    OCollection::disposing();
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
