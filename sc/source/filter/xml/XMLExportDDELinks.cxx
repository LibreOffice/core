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
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------
#include "XMLExportDDELinks.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmlexprt.hxx"
#include "unonames.hxx"
#include "document.hxx"
#include "scmatrix.hxx"
#include <com/sun/star/sheet/XDDELink.hpp>

class ScMatrix;

using namespace com::sun::star;
using namespace xmloff::token;
using ::rtl::OUStringBuffer;

ScXMLExportDDELinks::ScXMLExportDDELinks(ScXMLExport& rTempExport)
    : rExport(rTempExport)
{
}

ScXMLExportDDELinks::~ScXMLExportDDELinks()
{
}

sal_Bool ScXMLExportDDELinks::CellsEqual(const sal_Bool bPrevEmpty, const sal_Bool bPrevString, const String& sPrevValue, const double& fPrevValue,
                     const sal_Bool bEmpty, const sal_Bool bString, const String& sValue, const double& fValue) const
{
    if (bEmpty == bPrevEmpty)
        if (bEmpty)
            return sal_True;
        else if (bString == bPrevString)
            if (bString)
                return (sPrevValue == sValue);
            else
                return (fPrevValue == fValue);
        else
            return false;
    else
        return false;
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
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_STRING_VALUE, aVal.GetString());
        }
        else
        {
            OUStringBuffer aBuf;
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT);
            rExport.GetMM100UnitConverter().convertDouble(aBuf, aVal.fVal);
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE, aBuf.makeStringAndClear());
        }
    }

    if (nRepeat > 1)
    {
        OUStringBuffer aBuf;
        rExport.GetMM100UnitConverter().convertNumber(aBuf, nRepeat);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, aBuf.makeStringAndClear());
    }
    SvXMLElementExport(rExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
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

    SvXMLElementExport aTableElem(rExport, XML_NAMESPACE_TABLE, XML_TABLE, sal_True, sal_True);
    if (nCols > 1)
    {
        OUStringBuffer aBuf;
        rExport.GetMM100UnitConverter().convertNumber(aBuf, static_cast<sal_Int32>(nCols));
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, aBuf.makeStringAndClear());
    }
    {
        SvXMLElementExport aElemCol(rExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True);
    }

    for (SCSIZE nRow = 0; nRow < nRows; ++nRow)
    {
        sal_Int32 nRepeat = 0;
        ScMatrixValue aPrevVal;
        SvXMLElementExport aElemRow(rExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True);
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

void ScXMLExportDDELinks::WriteDDELinks(uno::Reference<sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        uno::Reference<container::XIndexAccess> xIndex(xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_DDELINKS))), uno::UNO_QUERY);
        if (xIndex.is())
        {
            sal_Int32 nCount = xIndex->getCount();
            if (nCount)
            {
                SvXMLElementExport aElemDDEs(rExport, XML_NAMESPACE_TABLE, XML_DDE_LINKS, sal_True, sal_True);
                for (sal_uInt16 nDDELink = 0; nDDELink < nCount; ++nDDELink)
                {
                    uno::Reference<sheet::XDDELink> xDDELink(xIndex->getByIndex(nDDELink), uno::UNO_QUERY);
                    if (xDDELink.is())
                    {
                        SvXMLElementExport aElemDDE(rExport, XML_NAMESPACE_TABLE, XML_DDE_LINK, sal_True, sal_True);
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
                            SvXMLElementExport(rExport, XML_NAMESPACE_OFFICE, XML_DDE_SOURCE, sal_True, sal_True);
                        }
                        WriteTable(nDDELink);
                    }
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
