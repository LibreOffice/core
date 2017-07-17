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

#include "writer/WDatabaseMetaData.hxx"
#include "writer/WConnection.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include "FDatabaseMetaDataResultSet.hxx"
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/types.hxx>

using namespace connectivity::file;
using namespace ::com::sun::star;

namespace connectivity
{
namespace writer
{

OWriterDatabaseMetaData::OWriterDatabaseMetaData(OConnection* pConnection) :ODatabaseMetaData(pConnection)
{
}

OWriterDatabaseMetaData::~OWriterDatabaseMetaData()
{
}

uno::Reference<sdbc::XResultSet> OWriterDatabaseMetaData::impl_getTypeInfo_throw()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);
    uno::Reference<sdbc::XResultSet> xRef = pResult;

    static ODatabaseMetaDataResultSet::ORows aRows;
    if (aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow;

        aRows.reserve(6);
        aRow.reserve(18);

        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(OUString("VARCHAR")));
        aRow.push_back(new ORowSetValueDecorator(sdbc::DataType::VARCHAR));
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)65535));
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value()); // ORowSetValue((sal_Int32)ColumnValue::NULLABLE)
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)sdbc::ColumnSearch::CHAR));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)10));
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("DECIMAL"));
        aRow[2] = new ORowSetValueDecorator(sdbc::DataType::DECIMAL);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("BOOL"));
        aRow[2] = new ORowSetValueDecorator(sdbc::DataType::BIT);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)20);
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = new ORowSetValueDecorator((sal_Int32)15);
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("DATE"));
        aRow[2] = new ORowSetValueDecorator(sdbc::DataType::DATE);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("TIME"));
        aRow[2] = new ORowSetValueDecorator(sdbc::DataType::TIME);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("TIMESTAMP"));
        aRow[2] = new ORowSetValueDecorator(sdbc::DataType::TIMESTAMP);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);
    }

    pResult->setRows(aRows);
    return xRef;
}


uno::Reference<sdbc::XResultSet> SAL_CALL OWriterDatabaseMetaData::getColumns(
    const uno::Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& tableNamePattern,
    const OUString& columnNamePattern)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference<sdbcx::XTablesSupplier> xTables = m_pConnection->createCatalog();
    if (!xTables.is())
        throw sdbc::SQLException();

    uno::Reference<container::XNameAccess> xNames = xTables->getTables();
    if (!xNames.is())
        throw sdbc::SQLException();

    ODatabaseMetaDataResultSet::ORows aRows;
    ODatabaseMetaDataResultSet::ORow  aRow(19);

    aRow[10] = new ORowSetValueDecorator((sal_Int32)10);

    uno::Sequence<OUString> aTabNames(xNames->getElementNames());
    const OUString* pTabIter = aTabNames.getConstArray();
    const OUString* pTabEnd      = pTabIter + aTabNames.getLength();
    for (; pTabIter != pTabEnd; ++pTabIter)
    {
        if (match(tableNamePattern,*pTabIter,'\0'))
        {
            uno::Reference<sdbcx::XColumnsSupplier> xTable(xNames->getByName(*pTabIter), uno::UNO_QUERY_THROW);
            aRow[3] = new ORowSetValueDecorator(*pTabIter);

            uno::Reference<container::XNameAccess> xColumns = xTable->getColumns();
            if (!xColumns.is())
                throw sdbc::SQLException();

            uno::Sequence<OUString> aColNames(xColumns->getElementNames());

            const OUString* pColumnIter = aColNames.getConstArray();
            const OUString* pEnd = pColumnIter + aColNames.getLength();
            uno::Reference<beans::XPropertySet> xColumn;
            for (sal_Int32 i=1; pColumnIter != pEnd; ++pColumnIter,++i)
            {
                if (match(columnNamePattern,*pColumnIter,'\0'))
                {
                    aRow[4]  = new ORowSetValueDecorator(*pColumnIter);

                    xColumns->getByName(*pColumnIter) >>= xColumn;
                    OSL_ENSURE(xColumn.is(),"Columns contains a column who isn't a fastpropertyset!");
                    aRow[5] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))));
                    aRow[6] = new ORowSetValueDecorator(::comphelper::getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME))));
                    aRow[7] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))));
                    //  aRow[8] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    aRow[9] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))));
                    aRow[11] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))));
                    //  aRow[12] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    aRow[13] = new ORowSetValueDecorator(::comphelper::getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE))));
                    //  aRow[14] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    //  aRow[15] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    switch (sal_Int32(aRow[5]->getValue()))
                    {
                    case sdbc::DataType::CHAR:
                    case sdbc::DataType::VARCHAR:
                        aRow[16] = new ORowSetValueDecorator((sal_Int32)254);
                        break;
                    case sdbc::DataType::LONGVARCHAR:
                        aRow[16] = new ORowSetValueDecorator((sal_Int32)65535);
                        break;
                    default:
                        aRow[16] = new ORowSetValueDecorator((sal_Int32)0);
                    }
                    aRow[17] = new ORowSetValueDecorator(i);
                    switch (sal_Int32(aRow[11]->getValue()))
                    {
                    case sdbc::ColumnValue::NO_NULLS:
                        aRow[18]  = new ORowSetValueDecorator(OUString("NO"));
                        break;
                    case sdbc::ColumnValue::NULLABLE:
                        aRow[18]  = new ORowSetValueDecorator(OUString("YES"));
                        break;
                    default:
                        aRow[18]  = new ORowSetValueDecorator(OUString());
                    }
                    aRows.push_back(aRow);
                }
            }
        }
    }

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    uno::Reference<sdbc::XResultSet> xRef = pResult;
    pResult->setRows(aRows);

    return xRef;
}


OUString SAL_CALL OWriterDatabaseMetaData::getURL()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return "sdbc:writer:" + m_pConnection->getURL();
}


sal_Int32 SAL_CALL OWriterDatabaseMetaData::getMaxBinaryLiteralLength()
{
    return SAL_MAX_INT32;
}


sal_Int32 SAL_CALL OWriterDatabaseMetaData::getMaxCharLiteralLength()
{
    return SAL_MAX_INT32;
}

sal_Int32 SAL_CALL OWriterDatabaseMetaData::getMaxColumnNameLength()
{
    return SAL_MAX_INT32;
}

sal_Int32 SAL_CALL OWriterDatabaseMetaData::getMaxColumnsInIndex()
{
    return 1;
}

sal_Int32 SAL_CALL OWriterDatabaseMetaData::getMaxColumnsInTable()
{
    return 256;
}

uno::Reference<sdbc::XResultSet> SAL_CALL OWriterDatabaseMetaData::getTables(
    const uno::Any& /*catalog*/, const OUString& /*schemaPattern*/,
    const OUString& tableNamePattern, const uno::Sequence< OUString >& types)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTables);
    uno::Reference<sdbc::XResultSet> xRef = pResult;

    // check if ORowSetValue type is given
    // when no types are given then we have to return all tables e.g. TABLE

    OUString aTable("TABLE");

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
        return xRef;

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
    for (sal_Int32 nTable=0; nTable<nTableCount; nTable++)
    {
        OUString aName = aTableNames[nTable];
        if (match(tableNamePattern,aName,'\0'))
        {
            ODatabaseMetaDataResultSet::ORow aRow { nullptr, nullptr, nullptr };
            aRow.reserve(6);
            aRow.push_back(new ORowSetValueDecorator(aName));
            aRow.push_back(new ORowSetValueDecorator(aTable));
            aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
            aRows.push_back(aRow);
        }
    }

    pResult->setRows(aRows);

    return xRef;
}

} // namespace writer
} // namespace connectivity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
