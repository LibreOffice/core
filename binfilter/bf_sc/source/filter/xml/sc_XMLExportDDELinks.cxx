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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "XMLExportDDELinks.hxx"

#include <bf_xmloff/xmlnmspe.hxx>
#include <bf_xmloff/xmluconv.hxx>

#include "xmlexprt.hxx"
#include "unonames.hxx"
#include "document.hxx"

#include <com/sun/star/sheet/XDDELink.hpp>
namespace binfilter {

class ScMatrix;

using namespace ::com::sun::star;
using namespace xmloff::token;

ScXMLExportDDELinks::ScXMLExportDDELinks(ScXMLExport& rTempExport)
    : rExport(rTempExport)
{
}

ScXMLExportDDELinks::~ScXMLExportDDELinks()
{
}

sal_Bool ScXMLExportDDELinks::CellsEqual(const sal_Bool bPrevEmpty, const sal_Bool bPrevString, const String& sPrevValue, const double& fPrevValue,
                     const sal_Bool bEmpty, const sal_Bool bString, const String& sValue, const double& fValue)
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
            return sal_False;
    else
        return sal_False;
}

void ScXMLExportDDELinks::WriteCell(const sal_Bool bEmpty, const sal_Bool bString, const String& sValue, const double& fValue, const sal_Int32 nRepeat)
{
    ::rtl::OUStringBuffer sBuffer;
    if (!bEmpty)
        if (bString)
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_STRING);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STRING_VALUE, ::rtl::OUString(sValue));
        }
        else
        {
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE_TYPE, XML_FLOAT);
            rExport.GetMM100UnitConverter().convertDouble(sBuffer, fValue);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_VALUE, sBuffer.makeStringAndClear());
        }
    if (nRepeat > 1)
    {
        rExport.GetMM100UnitConverter().convertNumber(sBuffer, nRepeat);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, sBuffer.makeStringAndClear());
    }
    SvXMLElementExport(rExport, XML_NAMESPACE_TABLE, XML_TABLE_CELL, sal_True, sal_True);
}

void ScXMLExportDDELinks::WriteTable(const sal_Int32 nPos)
{
    sal_Int32 nRowCount;
    sal_Int32 nColCount;
    ScMatrix* pMatrix = NULL;
    sal_uInt16 nuRow, nuCol;
    if (rExport.GetDocument() &&
        rExport.GetDocument()->GetDdeLinkResultDimension(static_cast<USHORT>(nPos), nuCol, nuRow, pMatrix))
    {
        nRowCount = nuRow;
        nColCount = nuCol;
        SvXMLElementExport aTableElem(rExport, XML_NAMESPACE_TABLE, XML_TABLE, sal_True, sal_True);
        ::rtl::OUStringBuffer sBuffer;
        if (nColCount > 1)
        {
            rExport.GetMM100UnitConverter().convertNumber(sBuffer, nColCount);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_REPEATED, sBuffer.makeStringAndClear());
        }
        {
            SvXMLElementExport aElemCol(rExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True);
        }
        sal_Bool bPrevString(sal_True);
        sal_Bool bPrevEmpty(sal_True);
        double fPrevValue;
        String sPrevValue;
        sal_Int32 nRepeatColsCount(1);
        for(sal_Int32 nRow = 0; nRow < nRowCount; nRow++)
        {
            SvXMLElementExport aElemRow(rExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True);
            for(sal_Int32 nColumn = 0; nColumn < nColCount; nColumn++)
            {
                if (nColumn == 0)
                    bPrevEmpty = rExport.GetDocument()->GetDdeLinkResult(pMatrix, static_cast<USHORT>(nColumn), static_cast<USHORT>(nRow),
                                                                        sPrevValue, fPrevValue, bPrevString);
                else
                {
                    double fValue;
                    String sValue;
                    sal_Bool bString(sal_True);
                    sal_Bool bEmpty = rExport.GetDocument()->GetDdeLinkResult(pMatrix, static_cast<USHORT>(nColumn), static_cast<USHORT>(nRow),
                                                                        sValue, fValue, bString);
                    if (CellsEqual(bPrevEmpty, bPrevString, sPrevValue, fPrevValue,
                                bEmpty, bString, sValue, fValue))
                        nRepeatColsCount++;
                    else
                    {
                        WriteCell(bPrevEmpty, bPrevString, sPrevValue, fPrevValue, nRepeatColsCount);
                        nRepeatColsCount = 1;
                        bPrevEmpty = bEmpty;
                        fPrevValue = fValue;
                        sPrevValue = sValue;
                    }
                }
            }
            WriteCell(bPrevEmpty, bPrevString, sPrevValue, fPrevValue, nRepeatColsCount);
            nRepeatColsCount = 1;
        }
    }
}

void ScXMLExportDDELinks::WriteDDELinks(uno::Reference<sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        uno::Any aDDELinks = xPropertySet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_DDELINKS)));
        uno::Reference<container::XIndexAccess> xIndex;
        if (aDDELinks >>= xIndex)
        {
            sal_Int32 nCount = xIndex->getCount();
            if (nCount)
            {
                SvXMLElementExport aElemDDEs(rExport, XML_NAMESPACE_TABLE, XML_DDE_LINKS, sal_True, sal_True);
                for (sal_uInt16 nDDELink = 0; nDDELink < nCount; nDDELink++)
                {
                    uno::Any aDDELink = xIndex->getByIndex(nDDELink);
                    uno::Reference<sheet::XDDELink> xDDELink;
                    if (aDDELink >>= xDDELink)
                    {
                        SvXMLElementExport aElemDDE(rExport, XML_NAMESPACE_TABLE, XML_DDE_LINK, sal_True, sal_True);
                        {
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_DDE_APPLICATION, xDDELink->getApplication());
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_DDE_TOPIC, xDDELink->getTopic());
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_DDE_ITEM, xDDELink->getItem());
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_AUTOMATIC_UPDATE, XML_TRUE);
                            sal_uInt16 nMode;
                            if (rExport.GetDocument() &&
                                rExport.GetDocument()->GetDdeLinkMode(nDDELink, nMode))
                            {
                                switch (nMode)
                                {
                                    case SC_DDE_ENGLISH :
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONVERSION_MODE, XML_INTO_ENGLISH_NUMBER);
                                    case SC_DDE_TEXT :
                                        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_CONVERSION_MODE, XML_LET_TEXT);
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
