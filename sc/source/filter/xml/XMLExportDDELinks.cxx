/*************************************************************************
 *
 *  $RCSfile: XMLExportDDELinks.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: aw $ $Date: 2001-02-27 14:20:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLEXPORTDDELINKS_HXX
#include "XMLExportDDELinks.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
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

#ifndef _COM_SUN_STAR_SHEET_XDDELINK_HPP_
#include <com/sun/star/sheet/XDDELink.hpp>
#endif

class ScMatrix;

using namespace com::sun::star;

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
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_string);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_string_value, rtl::OUString(sValue));
        }
        else
        {
            rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_float);
            rExport.GetMM100UnitConverter().convertDouble(sBuffer, fValue);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_value, sBuffer.makeStringAndClear());
        }
    if (nRepeat > 1)
    {
        rExport.GetMM100UnitConverter().convertNumber(sBuffer, nRepeat);
        rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_number_columns_repeated, sBuffer.makeStringAndClear());
    }
    SvXMLElementExport(rExport, XML_NAMESPACE_TABLE, sXML_table_cell, sal_True, sal_True);
}

void ScXMLExportDDELinks::WriteTable(const sal_Int32 nPos)
{
    sal_Int32 nRowCount;
    sal_Int32 nColCount;
    ScMatrix* pMatrix = NULL;
    sal_uInt16 nuRow, nuCol;
    if (rExport.GetDocument()->GetDdeLinkResultDimension(static_cast<USHORT>(nPos), nuCol, nuRow, pMatrix))
    {
        nRowCount = nuRow;
        nColCount = nuCol;
        SvXMLElementExport aTableElem(rExport, XML_NAMESPACE_TABLE, sXML_table, sal_True, sal_True);
        rtl::OUStringBuffer sBuffer;
        if (nColCount > 1)
        {
            rExport.GetMM100UnitConverter().convertNumber(sBuffer, nColCount);
            rExport.AddAttribute(XML_NAMESPACE_TABLE, sXML_number_columns_repeated, sBuffer.makeStringAndClear());
        }
        {
            SvXMLElementExport aElemCol(rExport, XML_NAMESPACE_TABLE, sXML_table_column, sal_True, sal_True);
        }
        sal_Bool bPrevString(sal_True);
        sal_Bool bPrevEmpty(sal_True);
        double fPrevValue;
        String sPrevValue;
        sal_Int32 nRepeatColsCount(1);
        for(sal_Int32 nRow = 0; nRow < nRowCount; nRow++)
        {
            SvXMLElementExport aElemRow(rExport, XML_NAMESPACE_TABLE, sXML_table_row, sal_True, sal_True);
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
        uno::Any aDDELinks = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_DDELINKS)));
        uno::Reference<container::XIndexAccess> xIndex;
        if (aDDELinks >>= xIndex)
        {
            sal_Int32 nCount = xIndex->getCount();
            if (nCount)
            {
                SvXMLElementExport aElemDDEs(rExport, XML_NAMESPACE_TABLE, sXML_dde_links, sal_True, sal_True);
                for (sal_uInt16 nDDELink = 0; nDDELink < nCount; nDDELink++)
                {
                    uno::Any aDDELink = xIndex->getByIndex(nDDELink);
                    uno::Reference<sheet::XDDELink> xDDELink;
                    if (aDDELink >>= xDDELink)
                    {
                        SvXMLElementExport aElemDDE(rExport, XML_NAMESPACE_TABLE, sXML_dde_link, sal_True, sal_True);
                        {
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_dde_application, xDDELink->getApplication());
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_dde_topic, xDDELink->getTopic());
                            rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_dde_item, xDDELink->getItem());
                            rExport.AddAttributeASCII(XML_NAMESPACE_OFFICE, sXML_automatic_update, sXML_true);
                            sal_uInt16 nMode;
                            if (rExport.GetDocument()->GetDdeLinkMode(nDDELink, nMode))
                            {
                                switch (nMode)
                                {
                                    case SC_DDE_ENGLISH :
                                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_conversion_mode, sXML_into_english_number);
                                    case SC_DDE_TEXT :
                                        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_conversion_mode, sXML_let_text);
                                }
                            }
                            SvXMLElementExport(rExport, XML_NAMESPACE_OFFICE, sXML_dde_source, sal_True, sal_True);
                        }
                        WriteTable(nDDELink);
                    }
                }
            }
        }
    }
}
