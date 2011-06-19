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

#include "MacabTable.hxx"
#include "MacabTables.hxx"
#include "MacabColumns.hxx"
#include "MacabCatalog.hxx"

using namespace connectivity::macab;
using namespace connectivity;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
MacabTable::MacabTable( sdbcx::OCollection* _pTables, MacabConnection* _pConnection)
    : MacabTable_TYPEDEF(_pTables, sal_True),
    m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
MacabTable::MacabTable( sdbcx::OCollection* _pTables,
                MacabConnection* _pConnection,
                const ::rtl::OUString& _Name,
                const ::rtl::OUString& _Type,
                const ::rtl::OUString& _Description ,
                const ::rtl::OUString& _SchemaName,
                const ::rtl::OUString& _CatalogName
                ) : MacabTable_TYPEDEF(_pTables,sal_True,
                                  _Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName),
                    m_pConnection(_pConnection)
{
    construct();
}
// -------------------------------------------------------------------------
void MacabTable::refreshColumns()
{
    TStringVector aVector;

    if (!isNew())
    {
        Reference< XResultSet > xResult = m_pConnection->getMetaData()->getColumns(
                Any(),
                m_SchemaName,
                m_Name,
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")));

        if (xResult.is())
        {
        Reference< XRow > xRow(xResult, UNO_QUERY);
        while (xResult->next())
                aVector.push_back(xRow->getString(4));
        }
    }

    if (m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new MacabColumns(this,m_aMutex,aVector);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
