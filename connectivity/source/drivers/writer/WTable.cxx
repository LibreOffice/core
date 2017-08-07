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

#include "writer/WTable.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <svl/converter.hxx>
#include "writer/WConnection.hxx"
#include "component/CColumns.hxx"
#include <connectivity/sdbcx/VColumn.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/thread.h>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/sequence.hxx>
#include <svl/zforlist.hxx>
#include <rtl/math.hxx>
#include <comphelper/extract.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbconversion.hxx>
#include <comphelper/types.hxx>
#include <connectivity/StdTypeDefs.hxx>
#include <connectivity/sdbcx/VCollection.hxx>

using namespace ::com::sun::star;

static void lcl_GetDataArea(const uno::Reference<text::XTextTable>& xTable, sal_Int32& rColumnCount, sal_Int32& rRowCount)
{
    uno::Reference<container::XIndexAccess> xColumns(xTable->getColumns(), uno::UNO_QUERY);
    if (xColumns.is())
        rColumnCount = xColumns->getCount();

    uno::Reference<container::XIndexAccess> xRows(xTable->getRows(), uno::UNO_QUERY);
    if (xRows.is())
        rRowCount = xRows->getCount() - 1; // first row (headers) is not counted

}

static void lcl_GetColumnInfo(const uno::Reference<text::XTextTable>& xTable,
                              sal_Int32 nDocColumn, sal_Int32 nStartRow, bool bHasHeaders,
                              OUString& rName, sal_Int32& rDataType, bool& rCurrency)
{
    uno::Reference<table::XCellRange> xCellRange(xTable, uno::UNO_QUERY);
    // get column name from first row, if range contains headers
    if (bHasHeaders)
    {
        uno::Reference<text::XText> xHeaderText(xCellRange->getCellByPosition(nDocColumn, nStartRow), uno::UNO_QUERY);
        if (xHeaderText.is())
            rName = xHeaderText->getString();
    }

    rCurrency = false;
    rDataType = sdbc::DataType::VARCHAR;
}


static void lcl_SetValue(connectivity::ORowSetValue& rValue, const uno::Reference<text::XTextTable>& xTable,
                         sal_Int32 nStartCol, sal_Int32 nStartRow, bool bHasHeaders,
                         sal_Int32 nDBRow, sal_Int32 nDBColumn)
{
    sal_Int32 nDocColumn = nStartCol + nDBColumn - 1;   // database counts from 1
    sal_Int32 nDocRow = nStartRow + nDBRow - 1;
    if (bHasHeaders)
        ++nDocRow;

    uno::Reference<table::XCellRange> xCellRange(xTable, uno::UNO_QUERY);
    const uno::Reference<table::XCell> xCell = xCellRange->getCellByPosition(nDocColumn, nDocRow);
    if (xCell.is())
    {
        const uno::Reference<text::XText> xText(xCell, uno::UNO_QUERY);
        if (xText.is())
            rValue = xText->getString();
    }
}

namespace connectivity
{
namespace writer
{

void OWriterTable::fillColumns()
{
    if (!m_xTable.is())
        throw sdbc::SQLException();

    OUString aTypeName;
    ::comphelper::UStringMixEqual aCase(m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers());
    const bool bStoresMixedCaseQuotedIdentifiers = getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers();

    for (sal_Int32 i = 0; i < m_nDataCols; i++)
    {
        OUString aColumnName;
        sal_Int32 eType = sdbc::DataType::OTHER;
        bool bCurrency = false;

        lcl_GetColumnInfo(m_xTable, m_nStartCol + i, 0, m_bHasHeaders,
                          aColumnName, eType, bCurrency);

        sal_Int32 nPrecision = 0;   //! ...
        sal_Int32 nDecimals = 0;    //! ...

        switch (eType)
        {
        case sdbc::DataType::VARCHAR:
            aTypeName = "VARCHAR";
            break;
        case sdbc::DataType::DECIMAL:
            aTypeName = "DECIMAL";
            break;
        case sdbc::DataType::BIT:
            aTypeName = "BOOL";
            break;
        case sdbc::DataType::DATE:
            aTypeName = "DATE";
            break;
        case sdbc::DataType::TIME:
            aTypeName = "TIME";
            break;
        case sdbc::DataType::TIMESTAMP:
            aTypeName = "TIMESTAMP";
            break;
        default:
            SAL_WARN("connectivity.writer", "missing type name");
            aTypeName.clear();
        }

        // check if the column name already exists
        OUString aAlias = aColumnName;
        OSQLColumns::Vector::const_iterator aFind = connectivity::find(m_aColumns->get().begin(),m_aColumns->get().end(),aAlias,aCase);
        sal_Int32 nExprCnt = 0;
        while (aFind != m_aColumns->get().end())
        {
            (aAlias = aColumnName) += OUString::number(++nExprCnt);
            aFind = connectivity::find(m_aColumns->get().begin(),m_aColumns->get().end(),aAlias,aCase);
        }

        sdbcx::OColumn* pColumn = new sdbcx::OColumn(aAlias, aTypeName, OUString(),OUString(),
                sdbc::ColumnValue::NULLABLE, nPrecision, nDecimals,
                eType, false, false, bCurrency,
                bStoresMixedCaseQuotedIdentifiers,
                m_CatalogName, getSchema(), getName());
        uno::Reference< XPropertySet> xCol = pColumn;
        m_aColumns->get().push_back(xCol);
        m_aTypes.push_back(eType);
        m_aPrecisions.push_back(nPrecision);
        m_aScales.push_back(nDecimals);
    }
}


OWriterTable::OWriterTable(sdbcx::OCollection* _pTables, OWriterConnection* _pConnection,
                           const OUString& Name,
                           const OUString& Type
                          ) : OWriterTable_BASE(_pTables,_pConnection,Name,
                                      Type,
                                      OUString()/*Description*/,
                                      OUString()/*SchemaName*/,
                                      OUString()/*CatalogName*/)
    ,m_pWriterConnection(_pConnection)
    ,m_nStartCol(0)
    ,m_nDataCols(0)
    ,m_bHasHeaders(false)
{
}

void OWriterTable::construct()
{
    uno::Reference<text::XTextDocument> xDoc = m_pWriterConnection->acquireDoc();
    if (xDoc.is())
    {
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(xDoc, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xTables = xTextTablesSupplier->getTextTables();
        if (xTables.is() && xTables->hasByName(m_Name))
        {
            m_xTable.set(xTables->getByName(m_Name), uno::UNO_QUERY);
            if (m_xTable.is())
            {
                lcl_GetDataArea(m_xTable, m_nDataCols, m_nDataRows);
                m_bHasHeaders = true;
            }
        }

        uno::Reference<util::XNumberFormatsSupplier> xSupp(xDoc, uno::UNO_QUERY);
        if (xSupp.is())
            m_xFormats = xSupp->getNumberFormats();
    }

    fillColumns();

    refreshColumns();
}

void SAL_CALL OWriterTable::disposing()
{
    OFileTable::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aColumns = nullptr;
    if (m_pWriterConnection)
        m_pWriterConnection->releaseDoc();
    m_pWriterConnection = nullptr;

}

uno::Sequence< sal_Int8 > OWriterTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId implId;

    return implId.getImplementationId();
}

sal_Int64 OWriterTable::getSomething(const uno::Sequence< sal_Int8 >& rId)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16))
           ? reinterpret_cast< sal_Int64 >(this)
           : OWriterTable_BASE::getSomething(rId);
}

bool OWriterTable::fetchRow(OValueRefRow& _rRow, const OSQLColumns& _rCols,
                            bool bRetrieveData)
{
    // read the bookmark

    _rRow->setDeleted(false);
    *(_rRow->get())[0] = m_nFilePos;

    if (!bRetrieveData)
        return true;

    // fields

    OSQLColumns::Vector::const_iterator aIter = _rCols.get().begin();
    OSQLColumns::Vector::const_iterator aEnd = _rCols.get().end();
    const OValueRefVector::Vector::size_type nCount = _rRow->get().size();
    for (OValueRefVector::Vector::size_type i = 1; aIter != aEnd && i < nCount;
            ++aIter, i++)
    {
        if ((_rRow->get())[i]->isBound())
        {
            lcl_SetValue((_rRow->get())[i]->get(), m_xTable, m_nStartCol, 0, m_bHasHeaders,
                         m_nFilePos, i);
        }
    }
    return true;
}

} // namespace writer
} // namespace connectivity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
