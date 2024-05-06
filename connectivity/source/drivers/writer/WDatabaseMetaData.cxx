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

#include <writer/WDatabaseMetaData.hxx>
#include <writer/WConnection.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

using namespace ::com::sun::star;

namespace connectivity::writer
{
OWriterDatabaseMetaData::OWriterDatabaseMetaData(file::OConnection* pConnection)
    : OComponentDatabaseMetaData(pConnection)
{
}

OWriterDatabaseMetaData::~OWriterDatabaseMetaData() = default;

OUString SAL_CALL OWriterDatabaseMetaData::getURL()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return "sdbc:writer:" + m_pConnection->getURL();
}

uno::Reference<sdbc::XResultSet> SAL_CALL OWriterDatabaseMetaData::getTables(
    const uno::Any& /*catalog*/, const OUString& /*schemaPattern*/,
    const OUString& tableNamePattern, const uno::Sequence<OUString>& types)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    rtl::Reference<ODatabaseMetaDataResultSet> pResult
        = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTables);

    // check if ORowSetValue type is given
    // when no types are given then we have to return all tables e.g. TABLE

    OUString aTable(u"TABLE"_ustr);

    bool bTableFound = true;
    sal_Int32 nLength = types.getLength();
    if (nLength)
    {
        bTableFound = false;

        const OUString* pIter = types.getConstArray();
        const OUString* pEnd = pIter + nLength;
        for (; pIter != pEnd; ++pIter)
        {
            if (*pIter == aTable)
            {
                bTableFound = true;
                break;
            }
        }
    }
    if (!bTableFound)
        return pResult;

    // get the table names from the document

    OWriterConnection::ODocHolder aDocHolder(static_cast<OWriterConnection*>(m_pConnection));
    uno::Reference<text::XTextTablesSupplier> xDoc(aDocHolder.getDoc(), uno::UNO_QUERY);
    if (!xDoc.is())
        throw sdbc::SQLException();
    uno::Reference<container::XNameAccess> xTables = xDoc->getTextTables();
    if (!xTables.is())
        throw sdbc::SQLException();
    uno::Sequence<OUString> aTableNames = xTables->getElementNames();

    ODatabaseMetaDataResultSet::ORows aRows;
    sal_Int32 nTableCount = aTableNames.getLength();
    for (sal_Int32 nTable = 0; nTable < nTableCount; nTable++)
    {
        const OUString& rName = aTableNames[nTable];
        if (match(tableNamePattern, rName, '\0'))
        {
            ODatabaseMetaDataResultSet::ORow aRow{ nullptr,
                                                   nullptr,
                                                   nullptr,
                                                   new ORowSetValueDecorator(rName),
                                                   new ORowSetValueDecorator(aTable),
                                                   ODatabaseMetaDataResultSet::getEmptyValue() };
            aRows.push_back(aRow);
        }
    }

    pResult->setRows(std::move(aRows));

    return pResult;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
