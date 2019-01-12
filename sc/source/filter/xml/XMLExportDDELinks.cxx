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

#include "XMLExportDDELinks.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <sax/tools/converter.hxx>
#include "xmlexprt.hxx"
#include <unonames.hxx>
#include <document.hxx>
#include <scmatrix.hxx>
#include <com/sun/star/sheet/XDDELink.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLExportDDELinks::ScXMLExportDDELinks(ScXMLExport& rTempExport)
    : rExport(rTempExport)
{
}

ScXMLExportDDELinks::~ScXMLExportDDELinks()
{
}

void ScXMLExportDDELinks::WriteCell(const ScMatrixValue& aVal, sal_Int32 nRepeat)
{
    bool bString = ScMatrix::IsNonValueType(aVal.nType);
    bool bEmpty = ScMatrix::IsEmptyType(aVal.nType);

    if (!bEmpty)
    {
        if (bString)
        {
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING);
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_STRING_VALUE, aVal.GetString().getString());
        }
        else
        {
            OUStringBuffer aBuf;
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT);
            ::sax::Converter::convertDouble(aBuf, aVal.fVal);
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE, aBuf.makeStringAndClear());
        }
    }

    if (nRepeat > 1)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, OUString::number(nRepeat));
    }
    SvXMLElementExport(rExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, true, true);
}

void ScXMLExportDDELinks::WriteTable(const sal_Int32 nPos)
{
    ScDocument* pDoc = rExport.GetDocument();
    if (!pDoc)
        return;

    const ScMatrix* pMatrix = pDoc->GetDdeLinkResultMatrix(static_cast<sal_uInt16>(nPos));
    if (!pMatrix)
        return;

    SCSIZE nCols, nRows;
    pMatrix->GetDimensions(nCols, nRows);

    SvXMLElementExport aTableElem(rExport, XML_NAMESPACE_TABLE, XML_TABLE, true, true);
    if (nCols > 1)
    {
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, OUString::number(nCols));
    }
    {
        SvXMLElementExport aElemCol(rExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, true, true);
    }

    for (SCSIZE nRow = 0; nRow < nRows; ++nRow)
    {
        sal_Int32 nRepeat = 0;
        ScMatrixValue aPrevVal;
        SvXMLElementExport aElemRow(rExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true);
        for (SCSIZE nCol = 0; nCol < nCols; ++nCol, ++nRepeat)
        {
            ScMatrixValue aVal = pMatrix->Get(nCol, nRow);
            if (nCol > 0 && aVal != aPrevVal)
            {
                // Cell value differs.  Flush the cell content.
                WriteCell(aPrevVal, nRepeat);
                nRepeat = 0;
            }
            aPrevVal = aVal;
        }

        WriteCell(aPrevVal, nRepeat);
    }
}

void ScXMLExportDDELinks::WriteDDELinks(const uno::Reference<sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        uno::Reference<container::XIndexAccess> xIndex(xPropertySet->getPropertyValue(SC_UNO_DDELINKS), uno::UNO_QUERY);
        if (xIndex.is())
        {
            sal_Int32 nCount = xIndex->getCount();
            if (nCount)
            {
                SvXMLElementExport aElemDDEs(rExport, XML_NAMESPACE_TABLE, XML_DDE_LINKS, true, true);
                for (sal_Int32 nDDELink = 0; nDDELink < nCount; ++nDDELink)
                {
                    uno::Reference<sheet::XDDELink> xDDELink(xIndex->getByIndex(nDDELink), uno::UNO_QUERY);
                    if (xDDELink.is())
                    {
                        SvXMLElementExport aElemDDE(rExport, XML_NAMESPACE_TABLE, XML_DDE_LINK, true, true);
                        {
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_DDE_APPLICATION, xDDELink->getApplication());
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_DDE_TOPIC, xDDELink->getTopic());
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_DDE_ITEM, xDDELink->getItem());
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_AUTOMATIC_UPDATE, XML_TRUE);
                            sal_uInt8 nMode;
                            if (rExport.GetDocument() &&
                                rExport.GetDocument()->GetDdeLinkMode(nDDELink, nMode))
                            {
                                switch (nMode)
                                {
                                    case SC_DDE_ENGLISH :
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONVERSION_MODE, XML_INTO_ENGLISH_NUMBER);
                                    break;
                                    case SC_DDE_TEXT :
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONVERSION_MODE, XML_KEEP_TEXT);
                                    break;
                                }
                            }
                            SvXMLElementExport(rExport, XML_NAMESPACE_OFFICE, XML_DDE_SOURCE, true, true);
                        }
                        WriteTable(nDDELink);
                    }
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
