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


#include "MacabTable.hxx"
#include "MacabTables.hxx"
#include "MacabColumns.hxx"
#include "MacabCatalog.hxx"
#include <com/sun/star/sdbc/XRow.hpp>

using namespace connectivity::macab;
using namespace connectivity;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


MacabTable::MacabTable( sdbcx::OCollection* _pTables, MacabConnection* _pConnection)
    : MacabTable_TYPEDEF(_pTables, true),
    m_pConnection(_pConnection)
{
    construct();
}

MacabTable::MacabTable( sdbcx::OCollection* _pTables,
                MacabConnection* _pConnection,
                const OUString& Name,
                const OUString& Type,
                const OUString& Description ,
                const OUString& SchemaName,
                const OUString& CatalogName
                ) : MacabTable_TYPEDEF(_pTables,true,
                                  Name,
                                  Type,
                                  Description,
                                  SchemaName,
                                  CatalogName),
                    m_pConnection(_pConnection)
{
    construct();
}

void MacabTable::refreshColumns()
{
    ::std::vector< OUString> aVector;

    if (!isNew())
    {
        Reference< XResultSet > xResult = m_pConnection->getMetaData()->getColumns(
                Any(), m_SchemaName, m_Name, "%");

        if (xResult.is())
        {
        Reference< XRow > xRow(xResult, UNO_QUERY);
        while (xResult->next())
                aVector.push_back(xRow->getString(4));
        }
    }

    if (m_xColumns)
        m_xColumns->reFill(aVector);
    else
        m_xColumns  = new MacabColumns(this,m_aMutex,aVector);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
