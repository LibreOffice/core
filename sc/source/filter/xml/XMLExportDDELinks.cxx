/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLExportDDELinks.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:43:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLEXPORTDDELINKS_HXX
#include "XMLExportDDELinks.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef SC_XMLEXPRT_HXX
#include "xmlexprt.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_MATRIX_HXX
#include "scmatrix.hxx"
#endif

#ifndef _COM_SUN_STAR_SHEET_XDDELINK_HPP_
#include <com/sun/star/sheet/XDDELink.hpp>
#endif

class ScMatrix;

using namespace com::sun::star;
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
    rtl::OUStringBuffer sBuffer;
    if (!bEmpty)
        if (bString)
        {
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_STRING);
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_STRING_VALUE, rtl::OUString(sValue));
        }
        else
        {
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE_TYPE, XML_FLOAT);
            rExport.GetMM100UnitConverter().convertDouble(sBuffer, fValue);
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_VALUE, sBuffer.makeStringAndClear());
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
    const ScMatrix* pMatrix(NULL);
    if (rExport.GetDocument())
        pMatrix = rExport.GetDocument()->GetDdeLinkResultMatrix( static_cast<USHORT>(nPos) );
    if (pMatrix)
    {
        SCSIZE nuCol;
        SCSIZE nuRow;
        pMatrix->GetDimensions( nuCol, nuRow );
        sal_Int32 nRowCount = static_cast<sal_Int32>(nuRow);
        sal_Int32 nColCount = static_cast<sal_Int32>(nuCol);
        SvXMLElementExport aTableElem(rExport, XML_NAMESPACE_TABLE, XML_TABLE, sal_True, sal_True);
        rtl::OUStringBuffer sBuffer;
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
        for(sal_Int32 nRow = 0; nRow < nRowCount; ++nRow)
        {
            SvXMLElementExport aElemRow(rExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True);
            for(sal_Int32 nColumn = 0; nColumn < nColCount; ++nColumn)
            {
                ScMatValType nType = SC_MATVAL_VALUE;
                const ScMatrixValue* pMatVal = pMatrix->Get( static_cast<SCSIZE>(nColumn), static_cast<SCSIZE>(nRow), nType );
                BOOL bIsString = (nType != SC_MATVAL_VALUE);

                if (nColumn == 0)
                {
                    bPrevEmpty = !pMatVal;
                    bPrevString = bIsString;
                    if( bIsString )
                        sPrevValue = pMatVal->GetString();
                    else
                        fPrevValue = pMatVal->fVal;
                }
                else
                {
                    double fValue;
                    String sValue;
                    sal_Bool bEmpty(!pMatVal);
                    sal_Bool bString(bIsString);
                    if( bIsString )
                        sValue = pMatVal->GetString();
                    else
                        fValue = pMatVal->fVal;

                    if (CellsEqual(bPrevEmpty, bPrevString, sPrevValue, fPrevValue,
                                bEmpty, bString, sValue, fValue))
                        ++nRepeatColsCount;
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
                            BYTE nMode;
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
