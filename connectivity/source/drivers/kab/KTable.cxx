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


#include "KTable.hxx"
#include "KTables.hxx"
#include "KColumns.hxx"
#include "KCatalog.hxx"

using namespace connectivity::kab;
using namespace connectivity;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
KabTable::KabTable( sdbcx::OCollection* _pTables,
                KabConnection* _pConnection,
                const ::rtl::OUString& _Name,
                const ::rtl::OUString& _Type,
                const ::rtl::OUString& _Description ,
                const ::rtl::OUString& _SchemaName,
                const ::rtl::OUString& _CatalogName
                ) : KabTable_TYPEDEF(_pTables,sal_True,
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
void KabTable::refreshColumns()
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
        m_pColumns  = new KabColumns(this,m_aMutex,aVector);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
