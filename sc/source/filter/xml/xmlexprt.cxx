/*************************************************************************
 *
 *  $RCSfile: xmlexprt.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-10 09:57:28 $
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

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlkywd.hxx>
#include <xmloff/xmlmetae.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/styleexp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/numehelp.hxx>
#ifndef _XMLOFF_XMLNUMFE_HXX
#include <xmloff/xmlnumfe.hxx>
#endif

#include <comphelper/processfactory.hxx>
#include <tools/lang.hxx>
#include <tools/solmath.hxx>
#include <tools/date.hxx>
#include <tools/intn.hxx>
#include <svtools/zforlist.hxx>
#include <sfx2/objsh.hxx>
#include <rtl/ustrbuf.hxx>
#include <vector>
#include <algorithm>

#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif

#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XUsedAreaCursor.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/sheet/XNamedRanges.hpp>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#include <com/sun/star/sheet/NamedRangeFlag.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#include <com/sun/star/sheet/DataImportMode.hpp>
#include <com/sun/star/util/SortField.hpp>
#include <com/sun/star/sheet/XSubTotalField.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/sheet/CellFlags.hpp>

#ifndef _COM_SUN_STAR_SHEET_XLABELRANGE_HPP_
#include <com/sun/star/sheet/XLabelRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XAREALINKS_HPP_
#include <com/sun/star/sheet/XAreaLinks.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XAREALINK_HPP_
#include <com/sun/star/sheet/XAreaLink.hpp>
#endif

#include "cellsuno.hxx"
#include "xmlexprt.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "scitems.hxx"
#include "attrib.hxx"
#include "docuno.hxx"
#include "rangenam.hxx"
#include "globstr.hrc"
#include "dbcolect.hxx"
#include "global.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpshttab.hxx"
#include "dpsdbtab.hxx"
#include "dociter.hxx"
#include "patattr.hxx"
#include "olinetab.hxx"
#include "rangeutl.hxx"

#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef _SC_XMLTABLEMASTERPAGEEXPORT_HXX
#include "XMLTableMasterPageExport.hxx"
#endif


const sal_Int8 SC_MAXDIGITSCOUNT_TIME = 11;

//! not found in unonames.hxx
#define SC_USERLIST "UserList"
#define SC_SORTASCENDING "SortAscending"
#define SC_ENABLEUSERSORTLIST "EnableUserSortList"
#define SC_USERSORTLISTINDEX "UserSortListIndex"
#define SC_STANDARDFORMAT "StandardFormat"
#define SC_LAYERID "LayerID"

#define SC_DEFAULT_TABLE_COUNT 3

using namespace rtl;
using namespace com::sun::star;

 // -----------------------------------------------------------------------

ScMyRowFormatRange::ScMyRowFormatRange()
    : nStartColumn(0),
    nRepeatColumns(0),
    nRepeatRows(0),
    nIndex(-1),
    bIsAutoStyle(sal_True)
{
}

ScRowFormatRanges::ScRowFormatRanges()
    : aRowFormatRanges()
{
}

ScRowFormatRanges::~ScRowFormatRanges()
{
}

void ScRowFormatRanges::Clear()
{
    aRowFormatRanges.clear();
}

void ScRowFormatRanges::AddRange(const ScMyRowFormatRange& aFormatRange)
{
    aRowFormatRanges.push_back(aFormatRange);
}

sal_Bool ScRowFormatRanges::GetNext(ScMyRowFormatRange& aFormatRange)
{
    ScMyRowFormatRangesVec::iterator aItr = aRowFormatRanges.begin();
    if (aItr != aRowFormatRanges.end())
    {
        aFormatRange = (*aItr);
        aRowFormatRanges.erase(aItr);
        return sal_True;
    }
    return sal_False;
}

sal_Int32 ScRowFormatRanges::GetMaxRows()
{
    ScMyRowFormatRangesVec::iterator aItr = aRowFormatRanges.begin();
    sal_Int32 nMaxRows = MAXROW + 1;
    if (aItr != aRowFormatRanges.end())
        while (aItr != aRowFormatRanges.end())
        {
            if ((*aItr).nRepeatRows < nMaxRows)
                nMaxRows = (*aItr).nRepeatRows;
            aItr++;
        }
    else
        nMaxRows = 0;
    return nMaxRows;
}

sal_Int32 ScRowFormatRanges::GetSize()
{
    return aRowFormatRanges.size();
}

sal_Bool LessRowFormatRange (const ScMyRowFormatRange& aRange1, const ScMyRowFormatRange& aRange2)
{
    return (aRange1.nStartColumn < aRange2.nStartColumn);
}

void ScRowFormatRanges::Sort()
{
    std::sort(aRowFormatRanges.begin(), aRowFormatRanges.end(), LessRowFormatRange);
}

// ============================================================================
ScMyFormatRange::ScMyFormatRange()
    : nStyleNameIndex(-1),
    bIsAutoStyle(sal_True)
{
}

ScFormatRangeStyles::ScFormatRangeStyles()
    : aTables(),
    aStyleNames(),
    aAutoStyleNames()
{
}

ScFormatRangeStyles::~ScFormatRangeStyles()
{
    ScMyOUStringVec::iterator i = aStyleNames.begin();
    while (i != aStyleNames.end())
    {
        delete *i;
        i++;
    }
    ScMyFormatRangeVectorVec::iterator j = aTables.begin();
    while (j != aTables.end())
    {
        delete *j;
        j++;
    }
}

void ScFormatRangeStyles::AddNewTable(const sal_Int16 nTable)
{
    sal_Int16 nSize = aTables.size() - 1;
    if (nTable > nSize)
        for (sal_Int16 i = nSize; i < nTable; i++)
        {
            ScMyFormatRangeAddresses* aRangeAddresses = new ScMyFormatRangeAddresses;
            aTables.push_back(aRangeAddresses);
        }
}

sal_Int32 ScFormatRangeStyles::AddStyleName(rtl::OUString* pString, const sal_Bool bIsAutoStyle)
{
    if (bIsAutoStyle)
    {
        aAutoStyleNames.push_back(pString);
        return aAutoStyleNames.size() - 1;
    }
    else
    {
        sal_Int32 nCount = aStyleNames.size();
        sal_Bool bFound(sal_False);
        sal_Int32 i = nCount - 1;
        while ((i >= 0) && (!bFound))
        {
            if (aStyleNames.at(i)->equals(*pString))
                bFound = sal_True;
            else
                i--;
        }
        if (bFound)
            return i;
        else
        {
            aStyleNames.push_back(pString);
            return aStyleNames.size() - 1;
        }
    }
}

sal_Int32 ScFormatRangeStyles::GetIndexOfStyleName(const rtl::OUString& rString, const rtl::OUString& rPrefix, sal_Bool& bIsAutoStyle)
{
    sal_Int16 nPrefixLength = rPrefix.getLength();
    rtl::OUString sTemp = rString.copy(nPrefixLength);
    sal_Int32 nIndex = sTemp.toInt32();
    if (aAutoStyleNames.at(nIndex - 1)->equals(rString))
    {
        bIsAutoStyle = sal_True;
        return nIndex - 1;
    }
    else
    {
        sal_Int32 i = 0;
        sal_Bool bFound(sal_False);
        while (!bFound && i < aStyleNames.size())
        {
            if (aStyleNames.at(i)->equals(rString))
                bFound = sal_True;
            else
                i++;
        }
        if (bFound)
        {
            bIsAutoStyle = sal_False;
            return i;
        }
        else
        {
            i = 0;
            while (!bFound && i < aAutoStyleNames.size())
            {
                if (aAutoStyleNames.at(i)->equals(rString))
                    bFound = sal_True;
                else
                    i++;
            }
            if (bFound)
            {
                bIsAutoStyle = sal_True;
                return i;
            }
            else
                return -1;
        }
    }
}

sal_Int32 ScFormatRangeStyles::GetStyleNameIndex(const sal_Int16 nTable, const sal_Int32 nColumn, const sal_Int32 nRow, sal_Bool& bIsAutoStyle)
{
    ScMyFormatRangeAddresses* pFormatRanges = aTables[nTable];
    ScMyFormatRangeAddresses::iterator aItr = pFormatRanges->begin();
    while (aItr != pFormatRanges->end())
    {
        if (((*aItr).aRangeAddress.StartColumn <= nColumn) &&
            ((*aItr).aRangeAddress.EndColumn >= nColumn) &&
            ((*aItr).aRangeAddress.StartRow <= nRow) &&
            ((*aItr).aRangeAddress.EndRow >= nRow))
        {
            bIsAutoStyle = (*aItr).bIsAutoStyle;
            return (*aItr).nStyleNameIndex;
        }
        else
        {
            if ((*aItr).aRangeAddress.EndRow < nRow)
                aItr = pFormatRanges->erase(aItr);
            else
                aItr++;
        }
    }
    return -1;
}
void ScFormatRangeStyles::GetFormatRanges(const sal_Int32 nStartColumn, const sal_Int32 nEndColumn, const sal_Int32 nRow,
                    const sal_Int16 nTable, ScRowFormatRanges& aFormatRanges)
{
    sal_Int32 nTotalColumns = nEndColumn - nStartColumn + 1;
    ScMyFormatRangeAddresses* pFormatRanges = aTables[nTable];
    ScMyFormatRangeAddresses::iterator aItr = pFormatRanges->begin();
    sal_Int32 nColumns = 0;
    while (aItr != pFormatRanges->end() && nColumns < nTotalColumns)
    {
        if (((*aItr).aRangeAddress.StartRow <= nRow) &&
            ((*aItr).aRangeAddress.EndRow >= nRow))
        {
            if ((((*aItr).aRangeAddress.StartColumn <= nStartColumn) &&
                ((*aItr).aRangeAddress.EndColumn >= nStartColumn)) ||
                (((*aItr).aRangeAddress.StartColumn <= nEndColumn) &&
                ((*aItr).aRangeAddress.EndColumn >= nEndColumn)) ||
                (((*aItr).aRangeAddress.StartColumn >= nStartColumn) &&
                ((*aItr).aRangeAddress.EndColumn <= nEndColumn)))
            {
                ScMyRowFormatRange aRange;
                aRange.aRangeAddress = (*aItr).aRangeAddress;
                aRange.nIndex = (*aItr).nStyleNameIndex;
                aRange.bIsAutoStyle = (*aItr).bIsAutoStyle;
                if (((*aItr).aRangeAddress.StartColumn < nStartColumn) &&
                    ((*aItr).aRangeAddress.EndColumn >= nStartColumn))
                {
                    if ((*aItr).aRangeAddress.EndColumn >= nEndColumn)
                        aRange.nRepeatColumns = nTotalColumns;
                    else
                        aRange.nRepeatColumns = (*aItr).aRangeAddress.EndColumn - nStartColumn + 1;
                    aRange.nStartColumn = nStartColumn;
                }
                else if (((*aItr).aRangeAddress.StartColumn >= nStartColumn) &&
                    ((*aItr).aRangeAddress.EndColumn <= nEndColumn))
                {
                    aRange.nRepeatColumns = (*aItr).aRangeAddress.EndColumn - (*aItr).aRangeAddress.StartColumn + 1;
                    aRange.nStartColumn = (*aItr).aRangeAddress.StartColumn;
                }
                else if (((*aItr).aRangeAddress.StartColumn >= nStartColumn) &&
                    ((*aItr).aRangeAddress.StartColumn <= nEndColumn) &&
                    ((*aItr).aRangeAddress.EndColumn > nEndColumn))
                {
                    aRange.nRepeatColumns = nEndColumn - (*aItr).aRangeAddress.StartColumn + 1;
                    aRange.nStartColumn = (*aItr).aRangeAddress.StartColumn;
                }
                aRange.nRepeatRows = (*aItr).aRangeAddress.EndRow - nRow + 1;
                aFormatRanges.AddRange(aRange);
                nColumns += aRange.nRepeatColumns;
            }
            aItr++;
        }
        else
            if((*aItr).aRangeAddress.EndRow < nRow)
                aItr = pFormatRanges->erase(aItr);
            else
                aItr++;
    }
    aFormatRanges.Sort();
}

void ScFormatRangeStyles::AddRangeStyleName(const table::CellRangeAddress aCellRangeAddress, const sal_Int32 nStringIndex, const sal_Bool bIsAutoStyle)
{
    ScMyFormatRange aFormatRange;
    aFormatRange.aRangeAddress = aCellRangeAddress;
    aFormatRange.nStyleNameIndex = nStringIndex;
    aFormatRange.bIsAutoStyle = bIsAutoStyle;
    ScMyFormatRangeAddresses* pFormatRanges = aTables[aCellRangeAddress.Sheet];
    pFormatRanges->push_back(aFormatRange);
}

rtl::OUString* ScFormatRangeStyles::GetStyleName(const sal_Int16 nTable, const sal_Int32 nColumn, const sal_Int32 nRow)
{
    sal_Bool bIsAutoStyle;
    sal_Int32 nIndex = GetStyleNameIndex(nTable, nColumn, nRow, bIsAutoStyle);
    if (bIsAutoStyle)
        return aAutoStyleNames.at(nIndex);
    else
        return aStyleNames.at(nIndex);
}

rtl::OUString* ScFormatRangeStyles::GetStyleNameByIndex(const sal_Int32 nIndex, const sal_Bool bIsAutoStyle)
{
    if (bIsAutoStyle)
        return aAutoStyleNames[nIndex];
    else
        return aStyleNames[nIndex];
}

sal_Bool LessFormatRange(const ScMyFormatRange& aRange1, const ScMyFormatRange& aRange2)
{
    if (aRange1.aRangeAddress.StartRow < aRange2.aRangeAddress.StartRow)
        return sal_True;
    else
        if (aRange1.aRangeAddress.StartRow == aRange2.aRangeAddress.StartRow)
            if (aRange1.aRangeAddress.StartColumn < aRange2.aRangeAddress.StartColumn)
                return sal_True;
            else
                return sal_False;
        else
            return sal_False;
}

void ScFormatRangeStyles::Sort()
{
    sal_Int16 nTables = aTables.size();
    for (sal_Int16 i = 0; i < nTables; i++)
        if (aTables[i]->size() > 1)
            std::sort(aTables[i]->begin(), aTables[i]->end(), LessFormatRange);
}

//===========================================================================

ScColumnRowStyles::ScColumnRowStyles()
    : aTables(),
    aStyleNames()
{
}

ScColumnRowStyles::~ScColumnRowStyles()
{
    ScMyOUStringVec::iterator i = aStyleNames.begin();
    while (i != aStyleNames.end())
    {
        delete *i;
        i++;
    }
}

void ScColumnRowStyles::AddNewTable(const sal_Int16 nTable, const sal_Int32 nFields)
{
    sal_Int16 nSize = aTables.size() - 1;
    if (nTable > nSize)
        for (sal_Int32 i = nSize; i < nTable; i++)
        {
            ScMysalInt32Vec aFieldsVec(nFields + 1, -1);
            aTables.push_back(aFieldsVec);
        }
}

sal_Int32 ScColumnRowStyles::AddStyleName(rtl::OUString* pString)
{
    aStyleNames.push_back(pString);
    return aStyleNames.size() - 1;
}

sal_Int32 ScColumnRowStyles::GetIndexOfStyleName(const rtl::OUString& rString, const rtl::OUString& rPrefix)
{
    sal_Int16 nPrefixLength = rPrefix.getLength();
    rtl::OUString sTemp = rString.copy(nPrefixLength);
    sal_Int32 nIndex = sTemp.toInt32();
    if (aStyleNames.at(nIndex - 1)->equals(rString))
        return nIndex - 1;
    else
    {
        sal_Int32 i = 0;
        sal_Bool bFound(sal_False);
        while (!bFound && i < aStyleNames.size())
        {
            if (aStyleNames.at(i)->equals(rString))
                bFound = sal_True;
            else
                i++;
        }
        if (bFound)
            return i;
        else
            return -1;
    }
}

sal_Int32 ScColumnRowStyles::GetStyleNameIndex(const sal_Int16 nTable, const sal_Int32 nField)
{
    if (nField < aTables[nTable].size())
        return aTables[nTable].at(nField);
    else
        return aTables[nTable].at(aTables[nTable].size() - 1);
}

void ScColumnRowStyles::AddFieldStyleName(const sal_Int16 nTable, const sal_Int32 nField, const sal_Int32 nStringIndex)
{
    aTables[nTable].at(nField) = nStringIndex;
}

rtl::OUString* ScColumnRowStyles::GetStyleName(const sal_Int16 nTable, const sal_Int32 nField)
{
    return aStyleNames[GetStyleNameIndex(nTable, nField)];
}

rtl::OUString* ScColumnRowStyles::GetStyleNameByIndex(const sal_Int32 nIndex)
{
    return aStyleNames[nIndex];
}

//==============================================================================

ScMyColumnRowGroup::ScMyColumnRowGroup()
{
}

ScMyOpenCloseColumnRowGroup::ScMyOpenCloseColumnRowGroup(ScXMLExport& rTempExport, const sal_Char *pName)
    : rExport(rTempExport),
    sName(rtl::OUString::createFromAscii(pName)),
    aTableStart(),
    aTableEnd(),
    bNamespaced(sal_False)
{
}

ScMyOpenCloseColumnRowGroup::~ScMyOpenCloseColumnRowGroup()
{
}

void ScMyOpenCloseColumnRowGroup::NewTable()
{
    if (!bNamespaced)
    {
        sName = rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, sName);
        bNamespaced = sal_True;
    }
    aTableStart.clear();
    aTableEnd.clear();
}

void ScMyOpenCloseColumnRowGroup::AddGroup(const ScMyColumnRowGroup& aGroup, sal_Int32 nEndField)
{
    aTableStart.push_back(aGroup);
    aTableEnd.push_back(nEndField);
}

sal_Bool ScMyOpenCloseColumnRowGroup::IsGroupStart(const sal_Int32 nField)
{
    sal_Bool bGroupStart(sal_False);
    if (aTableStart.size())
    {
        if (aTableStart[0].nField == nField)
            bGroupStart = sal_True;
    }
    return bGroupStart;
}

void ScMyOpenCloseColumnRowGroup::OpenGroup(const ScMyColumnRowGroup* pGroup)
{
    if (!pGroup->bDisplay)
        rExport.AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_display, sXML_false);
    rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
    rExport.GetDocHandler()->startElement( sName, rExport.GetXAttrList());
    rExport.ClearAttrList();
}

void ScMyOpenCloseColumnRowGroup::OpenGroups(const sal_Int32 nField)
{
    ScMyColumnRowGroupVec::iterator aItr = aTableStart.begin();
    sal_Bool bReady(sal_False);
    while(!bReady && aItr != aTableStart.end())
    {
        if (aItr->nField == nField)
        {
            OpenGroup(aItr);
            aItr = aTableStart.erase(aItr);
        }
        else
            bReady = sal_True;
    }
}

sal_Bool ScMyOpenCloseColumnRowGroup::IsGroupEnd(const sal_Int32 nField)
{
    sal_Bool bGroupEnd(sal_False);
    if (aTableEnd.size())
    {
        if (aTableEnd[0] == nField)
            bGroupEnd = sal_True;
    }
    return bGroupEnd;
}

void ScMyOpenCloseColumnRowGroup::CloseGroup()
{
    rExport.GetDocHandler()->ignorableWhitespace(rExport.sWS);
    rExport.GetDocHandler()->endElement( sName);
}

void ScMyOpenCloseColumnRowGroup::CloseGroups(const sal_Int32 nField)
{
    ScMyFieldGroupVec::iterator aItr = aTableEnd.begin();
    sal_Bool bReady(sal_False);
    while(!bReady && aItr != aTableEnd.end())
    {
        if (*aItr == nField)
        {
            CloseGroup();
            aItr = aTableEnd.erase(aItr);
        }
        else
            bReady = sal_True;
    }
}

sal_Bool LessGroup(const ScMyColumnRowGroup& aGroup1, const ScMyColumnRowGroup& aGroup2)
{
    if (aGroup1.nField < aGroup2.nField)
        return sal_True;
    else
        if (aGroup1.nField == aGroup2.nField && aGroup1.nLevel < aGroup2.nLevel)
            return sal_True;
        else
            return sal_False;
}

sal_Int32 ScMyOpenCloseColumnRowGroup::GetLast()
{
    sal_Int32 max(-1);
    for (sal_Int32 i = 0; i < aTableEnd.size(); i++)
        if (aTableEnd[i] > max)
            max = aTableEnd[i];
    return max;
}

void ScMyOpenCloseColumnRowGroup::Sort()
{
    std::sort(aTableStart.begin(), aTableStart.end(), LessGroup);
    std::sort(aTableEnd.begin(), aTableEnd.end());
}

//==============================================================================

void ScXMLExport::SetLastColumn(const sal_Int32 nTable, const sal_Int32 nCol)
{
    if(nCol > nLastColumns[nTable]) nLastColumns[nTable] = nCol;
}

sal_Int32 ScXMLExport::GetLastColumn(const sal_Int32 nTable)
{
    return nLastColumns[nTable];
}

void ScXMLExport::SetLastRow(const sal_Int32 nTable, const sal_Int32 nRow)
{
    if(nRow > nLastRows[nTable]) nLastRows[nTable] = nRow;
}

sal_Int32 ScXMLExport::GetLastRow(const sal_Int32 nTable)
{
    return nLastRows[nTable];
}


sal_Int16 ScXMLExport::GetFieldUnit()
{
    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xProperties(
                comphelper::getProcessServiceFactory()->createInstance(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.GlobalSheetSettings" )) ),
                com::sun::star::uno::UNO_QUERY);
    if (xProperties.is())
    {
        com::sun::star::uno::Any aAny = xProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Metric")));
        sal_Int16 nFieldUnit;
        if (aAny >>= nFieldUnit)
            return nFieldUnit;
    }
    return 0;
}

ScDocument* ScXMLExport::GetDocument()
{
    if (!pDoc)
    {
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            ScModelObj* pDocObj = ScModelObj::getImplementation( xSpreadDoc );
            if ( pDocObj )
                pDoc = pDocObj->GetDocument();
        }
    }
    return pDoc;
}

ScXMLExport::ScXMLExport( const uno::Reference <frame::XModel>& xTempModel, const NAMESPACE_RTL(OUString)& rFileName,
                        const uno::Reference<xml::sax::XDocumentHandler>& rHandler,
                        sal_Bool bShowProgr ) :
SvXMLExport( rFileName, rHandler, xTempModel, GetFieldUnit() ),
               xModel(xTempModel),
    pDoc(NULL),
    mbShowProgress( bShowProgr ),
    nLastColumns(SC_DEFAULT_TABLE_COUNT, 0),
    nLastRows(SC_DEFAULT_TABLE_COUNT, 0),
    aColumnStyles(),
    aRowStyles(),
    aCellStyles(),
    aShapesContainer(),
    aMergedRangesContainer(),
    aValidationsContainer(),
    xChartExportMapper(new ScExportMapper()),
    nOpenRow(-1),
    aRowFormatRanges(),
    nCurrentTable(0),
    aTableStyles(),
    pCellsItr(NULL),
    bHasRowHeader(sal_False),
    bRowHeaderOpen(sal_False),
    aGroupColumns(*this, sXML_table_column_group),
    aGroupRows(*this, sXML_table_row_group)
{
    pDoc = GetDocument();
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY );
    if ( xSpreadDoc.is() )
    {
        uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
        uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
        if ( xIndex.is() )
        {
            sal_Int32 nTableCount = xIndex->getCount();
            if (nTableCount > SC_DEFAULT_TABLE_COUNT)
            {
                nLastColumns.resize(nTableCount, 0);
                nLastRows.resize(nTableCount, 0);
            }
        }
    }

    xScPropHdlFactory = new XMLScPropHdlFactory;
    xCellStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScCellStylesProperties, xScPropHdlFactory);
    xColumnStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScColumnStylesProperties, xScPropHdlFactory);
    xRowStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScRowStylesProperties, xScPropHdlFactory);
    xTableStylesPropertySetMapper = new XMLPropertySetMapper((XMLPropertyMapEntry*)aXMLScTableStylesProperties, xScPropHdlFactory);
    xCellStylesExportPropertySetMapper = new ScXMLCellExportPropertyMapper(xCellStylesPropertySetMapper);
    xColumnStylesExportPropertySetMapper = new SvXMLExportPropertyMapper(xColumnStylesPropertySetMapper);
    xRowStylesExportPropertySetMapper = new SvXMLExportPropertyMapper(xRowStylesPropertySetMapper);
    xTableStylesExportPropertySetMapper = new SvXMLExportPropertyMapper(xTableStylesPropertySetMapper);

    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_CELL, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME)),
        xCellStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_COLUMN, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME)),
        xColumnStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_ROW, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME)),
        xRowStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX)));
    GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME)),
        xTableStylesExportPropertySetMapper, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX)));
}


ScXMLExport::~ScXMLExport()
{
}

table::CellRangeAddress ScXMLExport::GetEndAddress(uno::Reference<sheet::XSpreadsheet>& xTable,const sal_Int16 nTable)
{
    table::CellRangeAddress aCellAddress;
    uno::Reference<sheet::XSheetCellCursor> xCursor = xTable->createCursor();
    uno::Reference<sheet::XUsedAreaCursor> xUsedArea (xCursor, uno::UNO_QUERY);
    uno::Reference<sheet::XCellRangeAddressable> xCellAddress (xCursor, uno::UNO_QUERY);
    if (xUsedArea.is() && xCellAddress.is())
    {
        xUsedArea->gotoEndOfUsedArea(sal_True);
        aCellAddress = xCellAddress->getRangeAddress();
    }
    return aCellAddress;
}

ScMyEmptyDatabaseRangesContainer ScXMLExport::GetEmptyDatabaseRanges()
{
    ScMyEmptyDatabaseRangesContainer aSkipRanges;
    sal_Int32 nSkipRangesCount = 0;
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY );
    if ( xSpreadDoc.is() )
    {
        uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            uno::Any aDatabaseRanges = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_DATABASERNG)));
            uno::Reference <sheet::XDatabaseRanges> xDatabaseRanges;
            CheckAttrList();
            if (aDatabaseRanges >>= xDatabaseRanges)
            {
                uno::Sequence <rtl::OUString> aRanges = xDatabaseRanges->getElementNames();
                sal_Int32 nDatabaseRangesCount = aRanges.getLength();
                for (sal_Int32 i = 0; i < nDatabaseRangesCount; i++)
                {
                    rtl::OUString sDatabaseRangeName = aRanges[i];
                    uno::Any aDatabaseRange = xDatabaseRanges->getByName(sDatabaseRangeName);
                    uno::Reference <sheet::XDatabaseRange> xDatabaseRange;
                    if (aDatabaseRange >>= xDatabaseRange)
                    {
                        uno::Reference <beans::XPropertySet> xDatabaseRangePropertySet (xDatabaseRange, uno::UNO_QUERY);
                        if (xDatabaseRangePropertySet.is())
                        {
                            uno::Any aStripDataProperty = xDatabaseRangePropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_STRIPDAT)));
                            sal_Bool bStripData = sal_False;
                            if (aStripDataProperty >>= bStripData)
                                if (bStripData)
                                {
                                    uno::Sequence <beans::PropertyValue> aImportProperties = xDatabaseRange->getImportDescriptor();
                                    sal_Int32 nLength = aImportProperties.getLength();
                                    sheet::DataImportMode nSourceType = sheet::DataImportMode_NONE;
                                    for (sal_Int32 j = 0; j < nLength; j++)
                                        if (aImportProperties[j].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SRCTYPE)))
                                        {
                                            uno::Any aSourceType = aImportProperties[j].Value;
                                            aSourceType >>= nSourceType;
                                        }
                                    if (nSourceType != sheet::DataImportMode_NONE)
                                        aSkipRanges.AddNewEmptyDatabaseRange(xDatabaseRange->getDataArea());
                                }
                        }
                    }
                }
                if (nSkipRangesCount > 1)
                    aSkipRanges.Sort();
            }
        }
    }
    return aSkipRanges;
}

void ScXMLExport::GetAreaLinks( uno::Reference< sheet::XSpreadsheetDocument>& xSpreadDoc,
                                ScMyAreaLinksContainer& rAreaLinks )
{
    uno::Reference< beans::XPropertySet > xPropSet( xSpreadDoc, uno::UNO_QUERY );
    if( !xPropSet.is() ) return;

    uno::Reference< sheet::XAreaLinks > xAreaLinks;
    uno::Any aAny( xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_AREALINKS ) ) ) );
    if( aAny >>= xAreaLinks )
    {
        uno::Reference< container::XIndexAccess > xLinksIAccess( xAreaLinks, uno::UNO_QUERY );
        if( xLinksIAccess.is() )
        {
            const OUString sFilter( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FILTER ) );
            const OUString sFilterOpt( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_FILTOPT ) );
            const OUString sURL( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_LINKURL ) );

            sal_Int32 nCount = xLinksIAccess->getCount();
            for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                uno::Reference< sheet::XAreaLink > xAreaLink;
                uno::Any aLinkAny( xLinksIAccess->getByIndex( nIndex ) );
                if( aLinkAny >>= xAreaLink )
                {
                    ScMyAreaLink aAreaLink;
                    aAreaLink.aDestRange = xAreaLink->getDestArea();
                    aAreaLink.sSourceStr = xAreaLink->getSourceArea();
                    uno::Reference< beans::XPropertySet > xLinkProp( xAreaLink, uno::UNO_QUERY );
                    if( xLinkProp.is() )
                    {
                        aLinkAny = xLinkProp->getPropertyValue( sFilter );
                        aLinkAny >>= aAreaLink.sFilter;
                        aLinkAny = xLinkProp->getPropertyValue( sFilterOpt );
                        aLinkAny >>= aAreaLink.sFilterOptions;
                        aLinkAny = xLinkProp->getPropertyValue( sURL );
                        aLinkAny >>= aAreaLink.sURL;
                    }
                    rAreaLinks.AddNewAreaLink( aAreaLink );
                }
            }
        }
    }
    rAreaLinks.Sort();
}

// core implementation
void ScXMLExport::GetDetectiveOpList( ScMyDetectiveOpContainer& rDetOp )
{
    ScDetOpList* pOpList = pDoc->GetDetOpList();
    if( pOpList )
    {
        sal_Int32 nCount = pOpList->Count();
        for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
        {
            ScDetOpData* pDetData = pOpList->GetObject( nIndex );
            if( pDetData )
                rDetOp.AddOperation( pDetData->GetOperation(), pDetData->GetPos() );
        }
        rDetOp.Sort();
    }
}

sal_Bool ScXMLExport::GetxCurrentShapes(uno::Reference<container::XIndexAccess>& xShapes)
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xCurrentTable, uno::UNO_QUERY);
    if (xDrawPageSupplier.is())
    {
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        if (xDrawPage.is())
        {
            uno::Reference<container::XIndexAccess> xShapesIndex (xDrawPage, uno::UNO_QUERY);
            if (xShapesIndex.is())
            {
                xShapes = xShapesIndex;
                return sal_True;
            }
        }
    }
    return sal_False;
}

void ScXMLExport::WriteColumn(const sal_Int32 nRepeatColumns, const sal_Int32 nStyleIndex, const sal_Bool bIsVisible)
{
    CheckAttrList();
    AddAttribute(XML_NAMESPACE_TABLE, sXML_style_name, *aColumnStyles.GetStyleNameByIndex(nStyleIndex));
    if (!bIsVisible)
        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_visibility, sXML_collapse);
    if (nRepeatColumns > 1)
    {
        OUString sOUEndCol = OUString::valueOf(static_cast <sal_Int32> (nRepeatColumns));
        AddAttribute(XML_NAMESPACE_TABLE, sXML_number_columns_repeated, sOUEndCol);
    }
    SvXMLElementExport aElemR(*this, XML_NAMESPACE_TABLE, sXML_table_column, sal_True, sal_True);
}

void ScXMLExport::OpenHeaderColumn()
{
    rtl::OUString sName (GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_table_header_columns))));
    GetDocHandler()->ignorableWhitespace(sWS);
    GetDocHandler()->startElement( sName, GetXAttrList());
    ClearAttrList();
}

void ScXMLExport::CloseHeaderColumn()
{
    rtl::OUString sName (GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_table_header_columns))));
    GetDocHandler()->ignorableWhitespace(sWS);
    GetDocHandler()->endElement(sName);
}

void ScXMLExport::ExportColumns(const sal_Int16 nTable, const table::CellRangeAddress& aColumnHeaderRange, const sal_Bool bHasColumnHeader)
{
    sal_Int32 nColsRepeated (1);
    rtl::OUString sParent;
    sal_Int32 nIndex;
    sal_Bool bPrevIsVisible (sal_True);
    sal_Bool bWasHeader (sal_False);
    sal_Bool bIsHeader (sal_False);
    sal_Bool bIsClosed (sal_True);
    sal_Int32 nPrevIndex (-1);
    uno::Reference<table::XColumnRowRange> xColumnRowRange (xCurrentTable, uno::UNO_QUERY);
    if (xColumnRowRange.is())
    {
        uno::Reference<table::XTableColumns> xTableColumns = xColumnRowRange->getColumns();
        if (xTableColumns.is())
        {
            for (sal_Int32 nColumn = 0; nColumn <= GetLastColumn(nTable); nColumn++)
            {
                CheckAttrList();
                uno::Any aColumn = xTableColumns->getByIndex(nColumn);
                uno::Reference<table::XCellRange> xTableColumn;
                if (aColumn >>= xTableColumn)
                {
                    uno::Reference <beans::XPropertySet> xColumnProperties(xTableColumn, uno::UNO_QUERY);
                    if (xColumnProperties.is())
                    {
                        nIndex = aColumnStyles.GetStyleNameIndex(nTable, nColumn);

                        uno::Any aAny = xColumnProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLVIS)));
                        sal_Bool bIsVisible(sal_True);
                        aAny >>= bIsVisible;
                        bIsHeader = bHasColumnHeader && (aColumnHeaderRange.StartColumn <= nColumn) && (nColumn <= aColumnHeaderRange.EndColumn);
                        if (bIsHeader != bWasHeader)
                        {
                            if (bIsHeader)
                            {
                                if (nColumn > 0)
                                {
                                    WriteColumn(nColsRepeated, nPrevIndex, bPrevIsVisible);
                                    if (aGroupColumns.IsGroupEnd(nColumn - 1))
                                        aGroupColumns.CloseGroups(nColumn - 1);
                                    bPrevIsVisible = bIsVisible;
                                    nPrevIndex = nIndex;
                                    nColsRepeated = 1;
                                }
                                if(aGroupColumns.IsGroupStart(nColumn))
                                    aGroupColumns.OpenGroups(nColumn);
                                OpenHeaderColumn();
                                bWasHeader = sal_True;
                                bIsClosed = sal_False;
                            }
                            else
                            {
                                WriteColumn(nColsRepeated, nPrevIndex, bPrevIsVisible);
                                CloseHeaderColumn();
                                if (aGroupColumns.IsGroupEnd(nColumn - 1))
                                    aGroupColumns.CloseGroups(nColumn - 1);
                                bPrevIsVisible = bIsVisible;
                                nPrevIndex = nIndex;
                                nColsRepeated = 1;
                                bWasHeader = sal_False;
                                bIsClosed = sal_True;
                            }
                        }
                        else if (nColumn == 0)
                        {
                            if (aGroupColumns.IsGroupStart(nColumn))
                                aGroupColumns.OpenGroups(nColumn);
                            bPrevIsVisible = bIsVisible;
                            nPrevIndex = nIndex;
                        }
                        else if ((bIsVisible == bPrevIsVisible) && (nIndex == nPrevIndex) &&
                            !aGroupColumns.IsGroupStart(nColumn) && !aGroupColumns.IsGroupEnd(nColumn - 1))
                            nColsRepeated++;
                        else
                        {
                            WriteColumn(nColsRepeated, nPrevIndex, bPrevIsVisible);
                            if (aGroupColumns.IsGroupEnd(nColumn - 1))
                                aGroupColumns.CloseGroups(nColumn - 1);
                            if (aGroupColumns.IsGroupStart(nColumn))
                            {
                                if (bIsHeader)
                                    CloseHeaderColumn();
                                aGroupColumns.OpenGroups(nColumn);
                                if (bIsHeader)
                                    OpenHeaderColumn();
                            }
                            bPrevIsVisible = bIsVisible;
                            nPrevIndex = nIndex;
                            nColsRepeated = 1;
                        }
                    }
                }
            }
            if (nColsRepeated > 1)
                WriteColumn(nColsRepeated, nPrevIndex, bPrevIsVisible);
            if (!bIsClosed)
                CloseHeaderColumn();
            if (aGroupColumns.IsGroupEnd(nColumn - 1))
                aGroupColumns.CloseGroups(nColumn - 1);
        }
    }
}

void ScXMLExport::WriteRowContent()
{
    ScMyRowFormatRange aRange;
    sal_Int32 nIndex = -1;
    sal_Int32 nCols = 0;
    sal_Int32 nPrevValidationIndex = -1;
    sal_Bool bIsAutoStyle(sal_True);
    sal_Bool bIsFirst(sal_True);
    while (aRowFormatRanges.GetNext(aRange))
    {
        if (bIsFirst)
        {
            nIndex = aRange.nIndex;
            bIsAutoStyle = aRange.bIsAutoStyle;
            nCols = aRange.nRepeatColumns;
            bIsFirst = sal_False;
            table::CellAddress aCellAddress;
            aCellAddress.Column = aRange.aRangeAddress.EndColumn;
            aCellAddress.Row = aRange.aRangeAddress.EndRow;
            aCellAddress.Sheet = aRange.aRangeAddress.Sheet;
            nPrevValidationIndex = aValidationsContainer.GetValidationIndex(aCellAddress);
        }
        else
        {
            table::CellAddress aCellAddress;
            aCellAddress.Column = aRange.aRangeAddress.EndColumn;
            aCellAddress.Row = aRange.aRangeAddress.EndRow;
            aCellAddress.Sheet = aRange.aRangeAddress.Sheet;
            sal_Int32 nValidationIndex = aValidationsContainer.GetValidationIndex(aCellAddress);
            if (aRange.nIndex == nIndex && aRange.bIsAutoStyle == bIsAutoStyle &&
                nPrevValidationIndex == nValidationIndex)
                nCols += aRange.nRepeatColumns;
            else
            {
                AddAttribute(XML_NAMESPACE_TABLE, sXML_style_name, *aCellStyles.GetStyleNameByIndex(nIndex, bIsAutoStyle));
                if (nPrevValidationIndex > -1)
                    AddAttribute(XML_NAMESPACE_TABLE, sXML_content_validation_name, aValidationsContainer.GetValidationName(nPrevValidationIndex));
                if (nCols > 1)
                {
                    rtl::OUStringBuffer aBuf;
                    GetMM100UnitConverter().convertNumber(aBuf, nCols);
                    AddAttribute(XML_NAMESPACE_TABLE, sXML_number_columns_repeated, aBuf.makeStringAndClear());
                }
                SvXMLElementExport aElemC(*this, XML_NAMESPACE_TABLE, sXML_table_cell, sal_True, sal_True);
                nIndex = aRange.nIndex;
                bIsAutoStyle = aRange.bIsAutoStyle;
                nCols = aRange.nRepeatColumns;
                nPrevValidationIndex = nValidationIndex;
            }
        }
    }
    if (!bIsFirst)
    {
        table::CellAddress aCellAddress;
        AddAttribute(XML_NAMESPACE_TABLE, sXML_style_name, *aCellStyles.GetStyleNameByIndex(nIndex, bIsAutoStyle));
        if (nPrevValidationIndex > -1)
            AddAttribute(XML_NAMESPACE_TABLE, sXML_content_validation_name, aValidationsContainer.GetValidationName(nPrevValidationIndex));
        if (nCols > 1)
        {
            rtl::OUStringBuffer aBuf;
            GetMM100UnitConverter().convertNumber(aBuf, nCols);
            AddAttribute(XML_NAMESPACE_TABLE, sXML_number_columns_repeated, aBuf.makeStringAndClear());
        }
        SvXMLElementExport aElemC(*this, XML_NAMESPACE_TABLE, sXML_table_cell, sal_True, sal_True);
    }
}

void ScXMLExport::WriteRowStartTag(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nEqualRows)
{
    AddAttribute(XML_NAMESPACE_TABLE, sXML_style_name, *aRowStyles.GetStyleNameByIndex(nIndex));
    if (nFlag)
        if (nFlag & CR_HIDDEN)
            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_visibility, sXML_collapse);
        else
            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_visibility, sXML_filter);
    if (nEqualRows > 1)
    {
        rtl::OUStringBuffer aBuf;
        GetMM100UnitConverter().convertNumber(aBuf, nEqualRows);
        AddAttribute(XML_NAMESPACE_TABLE, sXML_number_rows_repeated, aBuf.makeStringAndClear());
    }
    rtl::OUString aName = GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_table_row)));
    GetDocHandler()->ignorableWhitespace(sWS);
    GetDocHandler()->startElement( aName, GetXAttrList());
    ClearAttrList();
}

void ScXMLExport::OpenHeaderRows()
{
    rtl::OUString aName = GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_table_header_rows)));
    GetDocHandler()->ignorableWhitespace(sWS);
    GetDocHandler()->startElement( aName, GetXAttrList());
    ClearAttrList();
    bRowHeaderOpen = sal_True;
}

void ScXMLExport::CloseHeaderRows()
{
    rtl::OUString sName = GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_table_header_rows)));
    GetDocHandler()->ignorableWhitespace(sWS);
    GetDocHandler()->endElement(sName);
}

void ScXMLExport::OpenNewRow(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nStartRow, const sal_Int32 nEqualRows)
{
    nOpenRow = nStartRow;
    if (aGroupRows.IsGroupStart(nStartRow))
    {
        if (bHasRowHeader && bRowHeaderOpen)
            CloseHeaderRows();
        aGroupRows.OpenGroups(nStartRow);
        if (bHasRowHeader && bRowHeaderOpen)
            OpenHeaderRows();
    }
    if (bHasRowHeader && !bRowHeaderOpen && nStartRow >= aRowHeaderRange.StartRow && nStartRow <= aRowHeaderRange.EndRow)
    {
        if (nStartRow == aRowHeaderRange.StartRow)
            OpenHeaderRows();
        sal_Int32 nEquals;
        if (aRowHeaderRange.EndRow < nStartRow + nEqualRows - 1)
            nEquals = aRowHeaderRange.EndRow - nStartRow + 1;
        else
            nEquals = nEqualRows;
        WriteRowStartTag(nIndex, nFlag, nEquals);
        nOpenRow = nStartRow + nEquals - 1;
        if (nEquals < nEqualRows)
        {
            CloseRow(nStartRow + nEquals - 1);
            WriteRowStartTag(nIndex, nFlag, nEqualRows - nEquals);
            nOpenRow = nStartRow + nEqualRows - 1;
        }
    }
    else
        WriteRowStartTag(nIndex, nFlag, nEqualRows);
}

void ScXMLExport::OpenAndCloseRow(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nStartRow, const sal_Int32 nEqualRows)
{
    OpenNewRow(nIndex, nFlag, nStartRow, nEqualRows);
    WriteRowContent();
    CloseRow(nStartRow + nEqualRows - 1);
    aRowFormatRanges.Clear();
}

void ScXMLExport::OpenRow(const sal_Int16 nTable, const sal_Int32 nStartRow, const sal_Int32 nRepeatRow)
{
    if (nRepeatRow > 1)
    {
        sal_Int32 nPrevIndex, nIndex;
        sal_Int8 nPrevFlag, nFlag;
        sal_Int32 nEqualRows = 1;
        for (sal_Int32 nRow = nStartRow; nRow < nStartRow + nRepeatRow; nRow++)
        {
            if (nRow == nStartRow)
            {
                nPrevIndex = aRowStyles.GetStyleNameIndex(nTable, nRow);
                nPrevFlag = (pDoc->GetRowFlags(nRow, nTable)) & (CR_HIDDEN | CR_FILTERED);
            }
            else
            {
                nIndex = aRowStyles.GetStyleNameIndex(nTable, nRow);
                nFlag = (pDoc->GetRowFlags(nRow, nTable)) & (CR_HIDDEN | CR_FILTERED);
                if (nIndex == nPrevIndex && nFlag == nPrevFlag &&
                    !(bHasRowHeader && nRow == aRowHeaderRange.StartRow) &&
                    !(aGroupRows.IsGroupStart(nRow)) &&
                    !(aGroupRows.IsGroupEnd(nRow - 1)))
                    nEqualRows++;
                else
                {
                    OpenAndCloseRow(nPrevIndex, nPrevFlag, nRow - nEqualRows, nEqualRows);
                    nEqualRows = 1;
                    nPrevIndex = nIndex;
                    nPrevFlag = nFlag;
                }
            }
        }
        OpenNewRow(nPrevIndex, nPrevFlag, nRow - nEqualRows, nEqualRows);
    }
    else
    {
        sal_Int32 nIndex = aRowStyles.GetStyleNameIndex(nTable, nStartRow);
        sal_Int8 nFlag = (pDoc->GetRowFlags(nStartRow, nTable)) & (CR_HIDDEN | CR_FILTERED);
        OpenNewRow(nIndex, nFlag, nStartRow, 1);
    }
    nOpenRow = nStartRow + nRepeatRow - 1;
}

void ScXMLExport::CloseRow(const sal_Int32 nRow)
{
    if (nOpenRow > -1)
    {
        rtl::OUString sName = GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_table_row)));
        GetDocHandler()->ignorableWhitespace(sWS);
        GetDocHandler()->endElement(sName);
        if (bHasRowHeader && nRow == aRowHeaderRange.EndRow)
        {
            CloseHeaderRows();
            bRowHeaderOpen = sal_False;
        }
        if (aGroupRows.IsGroupEnd(nRow))
        {
            if (bHasRowHeader && bRowHeaderOpen)
                CloseHeaderRows();
            aGroupRows.CloseGroups(nRow);
            if (bHasRowHeader && bRowHeaderOpen)
                OpenHeaderRows();
        }
    }
    nOpenRow = -1;
}

void ScXMLExport::ExportFormatRanges(const sal_Int32 nStartCol, const sal_Int32 nStartRow,
    const sal_Int32 nEndCol, const sal_Int32 nEndRow, const sal_Int16 nSheet)
{
    aRowFormatRanges.Clear();
    if (nStartRow == nEndRow)
    {
        aCellStyles.GetFormatRanges(nStartCol, nEndCol, nStartRow, nSheet, aRowFormatRanges);
        if (nOpenRow == - 1)
            OpenRow(nSheet, nStartRow, 1);
        WriteRowContent();
        aRowFormatRanges.Clear();
    }
    else
    {
        if (nOpenRow > -1)
        {
            aCellStyles.GetFormatRanges(nStartCol, GetLastColumn(nSheet), nStartRow, nSheet, aRowFormatRanges);
            WriteRowContent();
            CloseRow(nStartRow);
            sal_Int32 nRows = 1;
            sal_Int32 nTotalRows = nEndRow - nStartRow + 1 - 1;
            while (nRows < nTotalRows)
            {
                aRowFormatRanges.Clear();
                aCellStyles.GetFormatRanges(0, GetLastColumn(nSheet), nStartRow + nRows, nSheet, aRowFormatRanges);
                sal_Int32 nMaxRows = aRowFormatRanges.GetMaxRows();
                if (nMaxRows >= nTotalRows - nRows)
                {
                    OpenRow(nSheet, nStartRow + nRows, nTotalRows - nRows);
                    nRows += nTotalRows - nRows;
                }
                else
                {
                    OpenRow(nSheet, nStartRow + nRows, nMaxRows);
                    nRows += nMaxRows;
                }
                if (!aRowFormatRanges.GetSize())
                    aCellStyles.GetFormatRanges(0, GetLastColumn(nSheet), nStartRow + nRows, nSheet, aRowFormatRanges);
                WriteRowContent();
                CloseRow(nStartRow + nRows - 1);
            }
            if (nTotalRows == 1)
                CloseRow(nStartRow);
            OpenRow(nSheet, nEndRow, 1);
            aRowFormatRanges.Clear();
            aCellStyles.GetFormatRanges(0, nEndCol, nEndRow, nSheet, aRowFormatRanges);
            WriteRowContent();
        }
        else
        {
            sal_Int32 nRows = 0;
            sal_Int32 nTotalRows = nEndRow - nStartRow + 1 - 1;
            while (nRows < nTotalRows)
            {
                aCellStyles.GetFormatRanges(0, GetLastColumn(nSheet), nStartRow + nRows, nSheet, aRowFormatRanges);
                sal_Int32 nMaxRows = aRowFormatRanges.GetMaxRows();
                if (nMaxRows >= nTotalRows - nRows)
                {
                    OpenRow(nSheet, nStartRow + nRows, nTotalRows - nRows);
                    nRows += nTotalRows - nRows;
                }
                else
                {
                    OpenRow(nSheet, nStartRow + nRows, nMaxRows);
                    nRows += nMaxRows;
                }
                if (!aRowFormatRanges.GetSize())
                    aCellStyles.GetFormatRanges(0, GetLastColumn(nSheet), nStartRow + nRows, nSheet, aRowFormatRanges);
                WriteRowContent();
                CloseRow(nStartRow + nRows - 1);
            }
            OpenRow(nSheet, nEndRow, 1);
            aRowFormatRanges.Clear();
            aCellStyles.GetFormatRanges(0, nEndCol, nEndRow, nSheet, aRowFormatRanges);
            WriteRowContent();
        }
    }
}

sal_Bool ScXMLExport::GetColumnHeader(com::sun::star::table::CellRangeAddress& aColumnHeaderRange) const
{
    sal_Bool bResult(sal_False);
    uno::Reference <sheet::XPrintAreas> xPrintAreas (xCurrentTable, uno::UNO_QUERY);
    if (xPrintAreas.is())
    {
        bResult = xPrintAreas->getPrintTitleColumns();
        aColumnHeaderRange = xPrintAreas->getTitleColumns();
    }
    return bResult;
}

sal_Bool ScXMLExport::GetRowHeader(com::sun::star::table::CellRangeAddress& aRowHeaderRange) const
{
    sal_Bool bResult(sal_False);
    uno::Reference <sheet::XPrintAreas> xPrintAreas (xCurrentTable, uno::UNO_QUERY);
    if (xPrintAreas.is())
    {
        bResult = xPrintAreas->getPrintTitleRows();
        aRowHeaderRange = xPrintAreas->getTitleRows();
    }
    return bResult;
}

void ScXMLExport::FillFieldGroup(ScOutlineArray* pFields, ScMyOpenCloseColumnRowGroup& rGroups)
{
    sal_Int32 nDepth = pFields->GetDepth();
    for(sal_Int32 i = 0; i < nDepth; i++)
    {
        sal_Int32 nFields = pFields->GetCount(i);
        for (sal_Int32 j = 0; j < nFields; j++)
        {
            ScMyColumnRowGroup aGroup;
            ScOutlineEntry* pEntry = pFields->GetEntry(i, j);
            aGroup.nField = pEntry->GetStart();
            aGroup.nLevel = i;
            aGroup.bDisplay = !(pEntry->IsHidden());
            rGroups.AddGroup(aGroup, pEntry->GetEnd());
        }
    }
    if (nDepth)
        rGroups.Sort();
}

void ScXMLExport::FillColumnRowGroups()
{
    ScOutlineTable* pOutlineTable = GetDocument()->GetOutlineTable( nCurrentTable, sal_False );
    if(pOutlineTable)
    {
        ScOutlineArray* pCols = pOutlineTable->GetColArray();
        ScOutlineArray* pRows = pOutlineTable->GetRowArray();
        if (pCols)
            FillFieldGroup(pCols, aGroupColumns);
        if (pRows)
            FillFieldGroup(pRows, aGroupRows);
        SetLastColumn(nCurrentTable, aGroupColumns.GetLast());
        SetLastRow(nCurrentTable, aGroupRows.GetLast());
    }
}

void ScXMLExport::_ExportContent()
{
    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY );
    if ( xSpreadDoc.is() )
    {
        uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
        uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
        if ( xIndex.is() )
        {
            sal_Int32 nTableCount = xIndex->getCount();
            ScMyAreaLinksContainer aAreaLinks;
            GetAreaLinks( xSpreadDoc, aAreaLinks );
            ScMyEmptyDatabaseRangesContainer aEmptyRanges = GetEmptyDatabaseRanges();
            ScMyDetectiveOpContainer aDetectiveOpContainer;
            GetDetectiveOpList( aDetectiveOpContainer );

            aCellStyles.Sort();
            aShapesContainer.Sort();
            aMergedRangesContainer.Sort();
            aValidationsContainer.Sort();
            aDetectiveObjContainer.Sort();

            ScMyNotEmptyCellsIterator aCellsItr(*this);
            pCellsItr = &aCellsItr;
            aCellsItr.SetShapes( &aShapesContainer );
            aCellsItr.SetMergedRanges( &aMergedRangesContainer );
            aCellsItr.SetAreaLinks( &aAreaLinks );
            aCellsItr.SetEmptyDatabaseRanges( &aEmptyRanges );
            aCellsItr.SetValidations( &aValidationsContainer );
            aCellsItr.SetDetectiveObj( &aDetectiveObjContainer );
            aCellsItr.SetDetectiveOp( &aDetectiveOpContainer );

            if (nTableCount > 0)
                aValidationsContainer.WriteValidations(*this);
            WriteTheLabelRanges( xSpreadDoc );
            for (sal_Int32 nTable = 0; nTable < nTableCount; nTable++)
            {
                uno::Any aTable = xIndex->getByIndex(nTable);
                uno::Reference<sheet::XSpreadsheet> xTable;
                if (aTable>>=xTable)
                {
                    xCurrentTable = xTable;
                    uno::Reference<container::XNamed> xName (xTable, uno::UNO_QUERY );
                    if ( xName.is() )
                    {
                        nCurrentTable = nTable;
                        rtl::OUString sOUTableName = xName->getName();
                        AddAttribute(XML_NAMESPACE_TABLE, sXML_name, sOUTableName);
                        AddAttribute(XML_NAMESPACE_TABLE, sXML_style_name, aTableStyles[nTable]);
                        uno::Reference<util::XProtectable> xProtectable (xTable, uno::UNO_QUERY);
                        if (xProtectable.is())
                            if (xProtectable->isProtected())
                                AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_use_cell_protection, sXML_true);
                        rtl::OUString sPrintRanges( GetPrintRanges() );
                        if( sPrintRanges.getLength() )
                            AddAttribute( XML_NAMESPACE_TABLE, sXML_print_ranges, sPrintRanges );
                        SvXMLElementExport aElemT(*this, XML_NAMESPACE_TABLE, sXML_table, sal_True, sal_True);
                        CheckAttrList();
                        WriteScenario();
                        table::CellRangeAddress aRange = GetEndAddress(xTable, nTable);
                        SetLastColumn(nTable, aRange.EndColumn);
                        SetLastRow(nTable, aRange.EndRow);
                        aCellsItr.SetCurrentTable(nTable);
                        aGroupColumns.NewTable();
                        aGroupRows.NewTable();
                        GetxCurrentShapes(xCurrentShapes);
                        FillColumnRowGroups();
                        table::CellRangeAddress aColumnHeaderRange;
                        sal_Bool bHasColumnHeader(GetColumnHeader(aColumnHeaderRange));
                        if (bHasColumnHeader)
                            SetLastColumn(nTable, aColumnHeaderRange.EndColumn);
                        ExportColumns(nTable, aColumnHeaderRange, bHasColumnHeader);
                        bHasRowHeader = GetRowHeader(aRowHeaderRange);
                        bRowHeaderOpen = sal_False;
                        if (bHasRowHeader)
                            SetLastRow(nTable, aRowHeaderRange.EndRow);
                        sal_Bool bIsFirst(sal_True);
                        sal_Int32 nEqualCells(0);
                        ScMyCell aCell;
                        ScMyCell aPrevCell;
                        while(aCellsItr.GetNext(aCell))
                        {
                            if (bIsFirst)
                            {
                                ExportFormatRanges(0, 0, aCell.aCellAddress.Column - 1, aCell.aCellAddress.Row, nTable);
                                aPrevCell = aCell;
                                bIsFirst = sal_False;
                            }
                            else
                            {
                                if ((aPrevCell.aCellAddress.Row == aCell.aCellAddress.Row) &&
                                    (aPrevCell.aCellAddress.Column + nEqualCells + 1 == aCell.aCellAddress.Column))
                                {
                                    if(IsCellEqual(aPrevCell, aCell))
                                        nEqualCells++;
                                    else
                                    {
                                        SetRepeatAttribute(nEqualCells);
                                        WriteCell(aPrevCell);
/*                                      if (aPrevCell.bHasShape)
                                            aCellsItr.GetNextShape();*/
                                        nEqualCells = 0;
                                        aPrevCell = aCell;
                                    }
                                }
                                else
                                {
                                    SetRepeatAttribute(nEqualCells);
                                    WriteCell(aPrevCell);
/*                                  if (aPrevCell.bHasShape)
                                        aCellsItr.GetNextShape();*/
                                    ExportFormatRanges(aPrevCell.aCellAddress.Column + nEqualCells + 1, aPrevCell.aCellAddress.Row,
                                        aCell.aCellAddress.Column - 1, aCell.aCellAddress.Row, nTable);
                                    nEqualCells = 0;
                                    aPrevCell = aCell;
                                }
                            }
                        }
                        if (!bIsFirst)
                        {
                            SetRepeatAttribute(nEqualCells);
                            WriteCell(aPrevCell);
                            ExportFormatRanges(aPrevCell.aCellAddress.Column + nEqualCells + 1, aPrevCell.aCellAddress.Row,
                                GetLastColumn(nTable), GetLastRow(nTable), nTable);
                        }
                        else
                            ExportFormatRanges(0, 0, GetLastColumn(nTable), GetLastRow(nTable), nTable);
                        CloseRow(GetLastRow(nTable));
                        nEqualCells = 0;
                    }
                }
            }
        }
        WriteNamedExpressions(xSpreadDoc);
        WriteDatabaseRanges(xSpreadDoc);
        WriteDataPilots(xSpreadDoc);
        WriteConsolidation();
    }
}

void ScXMLExport::_ExportStyles( sal_Bool bUsed )
{
    uno::Reference <style::XStyleFamiliesSupplier> xStyleFamiliesSupplier (xModel, uno::UNO_QUERY);
    if (xStyleFamiliesSupplier.is())
    {
        uno::Reference <container::XNameAccess> aStylesFamilies = xStyleFamiliesSupplier->getStyleFamilies();
        if (aStylesFamilies.is())
        {
            uno::Any aStyleFamily = aStylesFamilies->getByName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CellStyles")));
            uno::Reference <container::XIndexAccess> aCellStyles;
            if (aStyleFamily >>= aCellStyles)
            {
                sal_Int32 nCount = aCellStyles->getCount();
                for (sal_Int32 i = 0; i < nCount; i++)
                {
                    uno::Any aCellStyle = aCellStyles->getByIndex(i);
                    uno::Reference <beans::XPropertySet> xCellProperties;
                    if (aCellStyle >>= xCellProperties)
                    {
                        uno::Any aNumberFormat = xCellProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_NUMFMT)));
                        sal_Int32 nNumberFormat;
                        if (aNumberFormat >>= nNumberFormat)
                        {
                            addDataStyle(nNumberFormat);
                        }
                    }
                }
            }
        }
    }
    exportDataStyles();

    ScXMLStyleExport aStylesExp(*this, rtl::OUString(), GetAutoStylePool().get());
    aStylesExp.exportStyleFamily(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CellStyles")),
        XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME, xCellStylesExportPropertySetMapper, FALSE, XML_STYLE_FAMILY_TABLE_CELL);
}

void ScXMLExport::_ExportAutoStyles()
{
    rtl::OUString SC_SCOLUMNPREFIX(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX));
    rtl::OUString SC_SROWPREFIX(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX));
    rtl::OUString SC_SCELLPREFIX(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX));
    GetChartExport()->setTableAddressMapper(xChartExportMapper);
    GetAutoStylePool()->ClearEntries();

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY );
    if ( xSpreadDoc.is() )
    {
        uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
        uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
        if ( xIndex.is() )
        {
            sal_Int32 nTableCount = xIndex->getCount();
            aCellStyles.AddNewTable(nTableCount - 1);
            for (sal_Int32 nTable = 0; nTable < nTableCount; nTable++)
            {
                uno::Any aTable = xIndex->getByIndex(nTable);
                uno::Reference<sheet::XSpreadsheet> xTable;
                if (aTable>>=xTable)
                {
                    uno::Reference<beans::XPropertySet> xTableProperties(xTable, uno::UNO_QUERY);
                    if (xTableProperties.is())
                    {
                        std::vector<XMLPropertyState> xPropStates = xTableStylesExportPropertySetMapper->Filter(xTableProperties);
                        if(xPropStates.size())
                        {
                            rtl::OUString sParent;
                            rtl::OUString sName = GetAutoStylePool()->Find(XML_STYLE_FAMILY_TABLE_TABLE, sParent, xPropStates);
                            if (!sName.len())
                            {
                                sName = GetAutoStylePool()->Add(XML_STYLE_FAMILY_TABLE_TABLE, sParent, xPropStates);
                            }
                            aTableStyles.push_back(sName);
                        }
                    }
                    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xTable, uno::UNO_QUERY);
                    if (xDrawPageSupplier.is())
                    {
                        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
                        uno::Reference<container::XIndexAccess> xShapesIndex (xDrawPage, uno::UNO_QUERY);
                        if (xShapesIndex.is())
                        {
                            sal_Int32 nShapesCount = xShapesIndex->getCount();
                            for (sal_Int32 nShape = 0; nShape < nShapesCount; nShape++)
                            {
                                uno::Any aShape = xShapesIndex->getByIndex(nShape);
                                uno::Reference<drawing::XShape> xShape;
                                if (aShape >>= xShape)
                                {
                                    uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
                                    if( xShapeProp.is() )
                                    {
                                        uno::Any aPropAny = xShapeProp->getPropertyValue(
                                            OUString( RTL_CONSTASCII_USTRINGPARAM( SC_LAYERID ) ) );
                                        sal_Int16 nLayerID;
                                        if( aPropAny >>= nLayerID )
                                        {
                                            if( nLayerID == SC_LAYER_INTERN )
                                                CollectInternalShape( xShape );
                                            else
                                            {
                                                GetShapeExport()->collectShapeAutoStyles(xShape);
                                                if (pDoc)
                                                {
                                                    awt::Point aPoint = xShape->getPosition();
                                                    awt::Size aSize = xShape->getSize();
                                                    Rectangle aRectangle(aPoint.X, aPoint.Y, aPoint.X + aSize.Width, aPoint.Y + aSize.Height);
                                                    ScRange aRange = pDoc->GetRange(nTable, aRectangle);
                                                    ScMyShape aMyShape;
                                                    aMyShape.aAddress = aRange.aStart;
                                                    aMyShape.nIndex = nShape;
                                                    aShapesContainer.AddNewShape(aMyShape);
                                                    SetLastColumn(nTable, aRange.aStart.Col());
                                                    SetLastRow(nTable, aRange.aStart.Row());
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    uno::Reference<sheet::XCellFormatRangesSupplier> xCellFormatRanges ( xTable, uno::UNO_QUERY );
                    if ( xCellFormatRanges.is() )
                    {
                        uno::Reference<container::XIndexAccess> xFormatRangesIndex = xCellFormatRanges->getCellFormatRanges();
                        if (xFormatRangesIndex.is())
                        {
                            sal_Int32 nFormatRangesCount = xFormatRangesIndex->getCount();
                            for (sal_Int32 nFormatRange = 0; nFormatRange < nFormatRangesCount; nFormatRange++)
                            {
                                uno::Any aFormatRange = xFormatRangesIndex->getByIndex(nFormatRange);
                                uno::Reference<table::XCellRange> xCellRange;
                                if (aFormatRange >>= xCellRange)
                                {
                                    uno::Reference <beans::XPropertySet> xProperties (xCellRange, uno::UNO_QUERY);
                                    if (xProperties.is())
                                    {
                                        uno::Reference <sheet::XCellRangeAddressable> xCellRangeAddressable(xCellRange, uno::UNO_QUERY);
                                        if (xCellRangeAddressable.is())
                                        {
                                            table::CellRangeAddress aRangeAddress = xCellRangeAddressable->getRangeAddress();
                                            uno::Any aValidation = xProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_VALIXML)));
                                            if (aValidationsContainer.AddValidation(aValidation, aRangeAddress))
                                            {
                                                SetLastColumn(nTable, aRangeAddress.EndColumn);
                                                SetLastRow(nTable, aRangeAddress.EndRow);
                                            }
                                            uno::Any aNumberFormat = xProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_NUMFMT)));
                                            sal_Int32 nNumberFormat;
                                            if (aNumberFormat >>= nNumberFormat)
                                            {
                                                addDataStyle(nNumberFormat);
                                            }
                                            uno::Any aStyle = xProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLSTYL)));
                                            rtl::OUString sStyleName;
                                            if (aStyle >>= sStyleName)
                                            {
                                                std::vector< XMLPropertyState > xPropStates = xCellStylesExportPropertySetMapper->Filter( xProperties );
                                                if (xPropStates.size())
                                                {
                                                    sal_Int32 nIndex;
                                                    rtl::OUString sName = GetAutoStylePool()->Find(XML_STYLE_FAMILY_TABLE_CELL, sStyleName, xPropStates);
                                                    sal_Bool bIsAutoStyle = sal_True;
                                                    if (!sName.len())
                                                    {
                                                        sName = GetAutoStylePool()->Add(XML_STYLE_FAMILY_TABLE_CELL, sStyleName, xPropStates);
                                                        rtl::OUString* pTemp = new rtl::OUString(sName);
                                                        nIndex = aCellStyles.AddStyleName(pTemp);
                                                    }
                                                    else
                                                        nIndex = aCellStyles.GetIndexOfStyleName(sName, SC_SCELLPREFIX, bIsAutoStyle);
                                                    SetLastColumn(nTable, aRangeAddress.EndColumn);
                                                    SetLastRow(nTable, aRangeAddress.EndRow);
                                                    aCellStyles.AddRangeStyleName(aRangeAddress, nIndex, bIsAutoStyle);
                                                    uno::Reference<sheet::XSheetCellRange> xSheetCellRange(xCellRange, uno::UNO_QUERY);
                                                    if (xSheetCellRange.is())
                                                    {
                                                        uno::Reference<sheet::XSheetCellCursor> xCursor = xTable->createCursorByRange(xSheetCellRange);
                                                        if(xCursor.is())
                                                        {
                                                            uno::Reference<sheet::XCellRangeAddressable> xCellAddress (xCursor, uno::UNO_QUERY);
                                                            table::CellRangeAddress aCellAddress = xCellAddress->getRangeAddress();
                                                            xCursor->collapseToMergedArea();
                                                            table::CellRangeAddress aCellAddress2 = xCellAddress->getRangeAddress();
                                                            if (aCellAddress2.EndColumn > aCellAddress.EndColumn ||
                                                                aCellAddress2.EndRow > aCellAddress.EndRow)
                                                            {
                                                                SetLastColumn(nTable, aCellAddress2.EndColumn);
                                                                SetLastRow(nTable, aCellAddress2.EndRow);
                                                                aMergedRangesContainer.AddRange(aCellAddress2);
                                                            }
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    uno::Reference<sheet::XSheetCellRange> xSheetCellRange(xCellRange, uno::UNO_QUERY);
                                                    if (xSheetCellRange.is())
                                                    {
                                                        uno::Reference<sheet::XSheetCellCursor> xCursor = xTable->createCursorByRange(xSheetCellRange);
                                                        if(xCursor.is())
                                                        {
                                                            uno::Reference<sheet::XCellRangeAddressable> xCellAddress (xCursor, uno::UNO_QUERY);
                                                            table::CellRangeAddress aCellAddress = xCellAddress->getRangeAddress();
                                                            xCursor->collapseToMergedArea();
                                                            table::CellRangeAddress aCellAddress2 = xCellAddress->getRangeAddress();
                                                            if (aCellAddress2.EndColumn > aCellAddress.EndColumn ||
                                                                aCellAddress2.EndRow > aCellAddress.EndRow)
                                                            {
                                                                SetLastColumn(nTable, aCellAddress2.EndColumn);
                                                                SetLastRow(nTable, aCellAddress2.EndRow);
                                                                aMergedRangesContainer.AddRange(aCellAddress2);
                                                            }
                                                            rtl::OUString* pTemp = new rtl::OUString(sStyleName);
                                                            sal_Int32 nIndex = aCellStyles.AddStyleName(pTemp, sal_False);
                                                            aCellStyles.AddRangeStyleName(aCellAddress, nIndex, sal_False);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    uno::Reference<sheet::XCellRangesQuery> xCellRangesQuery (xTable, uno::UNO_QUERY);
                    uno::Reference<table::XColumnRowRange> xColumnRowRange (xTable, uno::UNO_QUERY);
                    if (xColumnRowRange.is())
                    {
                        if (pDoc)
                        {
                            uno::Reference<table::XTableColumns> xTableColumns = xColumnRowRange->getColumns();
                            if (xTableColumns.is())
                            {
                                sal_Int32 nColumns = pDoc->GetLastFlaggedCol(nTable);
                                SetLastColumn(nTable, nColumns);
                                table::CellRangeAddress aCellAddress = GetEndAddress(xTable, nTable);
                                if (aCellAddress.EndColumn > nColumns)
                                {
                                    nColumns++;
                                    aColumnStyles.AddNewTable(nTable, aCellAddress.EndColumn);
                                }
                                else if (nColumns < MAXCOL)
                                    aColumnStyles.AddNewTable(nTable, ++nColumns);
                                else
                                    aColumnStyles.AddNewTable(nTable, nColumns);
                                for (sal_Int32 nColumn = 0; nColumn <= nColumns; nColumn++)
                                {
                                    uno::Any aColumn = xTableColumns->getByIndex(nColumn);
                                    uno::Reference<table::XCellRange> xTableColumn;
                                    if (aColumn >>= xTableColumn)
                                    {
                                        uno::Reference <beans::XPropertySet> xColumnProperties(xTableColumn, uno::UNO_QUERY);
                                        if (xColumnProperties.is())
                                        {
                                            std::vector<XMLPropertyState> xPropStates = xColumnStylesExportPropertySetMapper->Filter(xColumnProperties);
                                            if(xPropStates.size())
                                            {
                                                sal_Int32 nIndex;
                                                rtl::OUString sParent;
                                                rtl::OUString sName = GetAutoStylePool()->Find(XML_STYLE_FAMILY_TABLE_COLUMN, sParent, xPropStates);
                                                if (!sName.len())
                                                {
                                                    sName = GetAutoStylePool()->Add(XML_STYLE_FAMILY_TABLE_COLUMN, sParent, xPropStates);
                                                    rtl::OUString* pTemp = new rtl::OUString(sName);
                                                    nIndex = aColumnStyles.AddStyleName(pTemp);
                                                }
                                                else
                                                    nIndex = aColumnStyles.GetIndexOfStyleName(sName, SC_SCOLUMNPREFIX);
                                                aColumnStyles.AddFieldStyleName(nTable, nColumn, nIndex);
                                            }
                                        }
                                    }
                                }
                                if (aCellAddress.EndColumn > nColumns)
                                {
                                    sal_Int32 nIndex = aColumnStyles.GetStyleNameIndex(nTable, nColumns);
                                    for (sal_Int32 i = nColumns + 1; i <= aCellAddress.EndColumn; i++)
                                        aColumnStyles.AddFieldStyleName(nTable, i, nIndex);
                                }
                            }
                            uno::Reference<table::XTableRows> xTableRows = xColumnRowRange->getRows();
                            if (xTableRows.is())
                            {
                                sal_Int32 nRows = pDoc->GetLastFlaggedRow(nTable);
                                SetLastRow(nTable, nRows);
                                table::CellRangeAddress aCellAddress = GetEndAddress(xTable, nTable);
                                if (aCellAddress.EndRow > nRows)
                                {
                                    nRows++;
                                    aRowStyles.AddNewTable(nTable, aCellAddress.EndRow);
                                }
                                else if (nRows < MAXROW)
                                    aRowStyles.AddNewTable(nTable, ++nRows);
                                else
                                    aRowStyles.AddNewTable(nTable, nRows);
                                for (sal_Int32 nRow = 0; nRow <= nRows; nRow++)
                                {
                                    uno::Any aRow = xTableRows->getByIndex(nRow);
                                    uno::Reference<table::XCellRange> xTableRow;
                                    if (aRow >>= xTableRow)
                                    {
                                        uno::Reference <beans::XPropertySet> xRowProperties(xTableRow, uno::UNO_QUERY);
                                        if(xRowProperties.is())
                                        {
                                            std::vector<XMLPropertyState> xPropStates = xRowStylesExportPropertySetMapper->Filter(xRowProperties);
                                            if(xPropStates.size())
                                            {
                                                sal_Int32 nIndex;
                                                rtl::OUString sParent;
                                                rtl::OUString sName = GetAutoStylePool()->Find(XML_STYLE_FAMILY_TABLE_ROW, sParent, xPropStates);
                                                if (!sName.len())
                                                {
                                                    sName = GetAutoStylePool()->Add(XML_STYLE_FAMILY_TABLE_ROW, sParent, xPropStates);
                                                    rtl::OUString* pTemp = new rtl::OUString(sName);
                                                    nIndex = aRowStyles.AddStyleName(pTemp);
                                                }
                                                else
                                                    nIndex = aRowStyles.GetIndexOfStyleName(sName, SC_SROWPREFIX);
                                                aRowStyles.AddFieldStyleName(nTable, nRow, nIndex);
                                            }
                                        }
                                    }
                                }
                                if (aCellAddress.EndRow > nRows)
                                {
                                    sal_Int32 nIndex = aRowStyles.GetStyleNameIndex(nTable, nRows);
                                    for (sal_Int32 i = nRows + 1; i <= aCellAddress.EndRow; i++)
                                        aRowStyles.AddFieldStyleName(nTable, i, nIndex);
                                }
                            }
                        }
                    }
                    if (xCellRangesQuery.is())
                    {
                        uno::Reference<sheet::XSheetCellRanges> xSheetCellRanges = xCellRangesQuery->queryContentCells(sheet::CellFlags::STRING);
                        if (xSheetCellRanges.is())
                        {
                            uno::Reference<container::XEnumerationAccess> xCellsAccess = xSheetCellRanges->getCells();
                            if (xCellsAccess.is())
                            {
                                uno::Reference<container::XEnumeration> xCells = xCellsAccess->createEnumeration();
                                if (xCells.is())
                                {
                                    while (xCells->hasMoreElements())
                                    {
                                        uno::Any aCell = xCells->nextElement();
                                        uno::Reference<table::XCell> xCell;
                                        if (aCell >>= xCell)
                                        {
                                            if (IsEditCell(xCell))
                                            {
                                                uno::Reference<text::XText> xText(xCell, uno::UNO_QUERY);
                                                if (xText.is())
                                                {
                                                    GetTextParagraphExport()->collectTextAutoStyles(xText);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            GetPageExport()->collectAutoStyles(sal_True);

            GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_COLUMN,
                GetDocHandler(), GetMM100UnitConverter(), GetNamespaceMap());
            GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_ROW,
                GetDocHandler(), GetMM100UnitConverter(), GetNamespaceMap());
            GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_TABLE,
                GetDocHandler(), GetMM100UnitConverter(), GetNamespaceMap());
            GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_CELL,
                GetDocHandler(), GetMM100UnitConverter(), GetNamespaceMap());
            GetTextParagraphExport()->exportTextAutoStyles();
            GetShapeExport()->exportAutoStyles();
            GetChartExport()->exportAutoStyles();

            GetPageExport()->exportAutoStyles();
        }
    }
}

void ScXMLExport::_ExportMasterStyles()
{
    GetPageExport()->exportMasterStyles( sal_True );
}

void ScXMLExport::CollectInternalShape( uno::Reference< drawing::XShape > xShape )
{
    // detective objects
    SvxShape* pShapeImp = SvxShape::getImplementation( xShape );
    if( pShapeImp )
    {
        SdrObject *pObject = pShapeImp->GetSdrObject();
        if( pObject )
        {
            ScDetectiveFunc aDetFunc( pDoc, nCurrentTable );
            ScAddress       aPosition;
            ScRange         aSourceRange;
            sal_Bool        bRedLine;
            ScDetectiveObjType eObjType = aDetFunc.GetDetectiveObjectType(
                pObject, aPosition, aSourceRange, bRedLine );
            aDetectiveObjContainer.AddObject( eObjType, aPosition, aSourceRange, bRedLine );
        }
    }
}

sal_Bool ScXMLExport::IsMerged (const uno::Reference <table::XCellRange>& xCellRange, const sal_Int32 nCol, const sal_Int32 nRow,
                            table::CellRangeAddress& aCellAddress) const
{
    uno::Reference <table::XCellRange> xMergeCellRange = xCellRange->getCellRangeByPosition(nCol,nRow,nCol,nRow);
    uno::Reference <util::XMergeable> xMergeable (xMergeCellRange, uno::UNO_QUERY);
    if (xMergeable.is())
        if (xMergeable->getIsMerged())
        {
            uno::Reference<sheet::XSheetCellRange> xMergeSheetCellRange (xMergeCellRange, uno::UNO_QUERY);
            uno::Reference<sheet::XSpreadsheet> xTable = xMergeSheetCellRange->getSpreadsheet();
            uno::Reference<sheet::XSheetCellCursor> xMergeSheetCursor = xTable->createCursorByRange(xMergeSheetCellRange);
            if (xMergeSheetCursor.is())
            {
                xMergeSheetCursor->collapseToMergedArea();
                uno::Reference<sheet::XCellRangeAddressable> xMergeCellAddress (xMergeSheetCursor, uno::UNO_QUERY);
                if (xMergeCellAddress.is())
                {
                    aCellAddress = xMergeCellAddress->getRangeAddress();
                    return sal_True;
                }
            }
        }
    return sal_False;
}

sal_Bool ScXMLExport::IsMatrix (const uno::Reference <table::XCellRange>& xCellRange,
                            const uno::Reference <sheet::XSpreadsheet>& xTable,
                            const sal_Int32 nCol, const sal_Int32 nRow,
                            table::CellRangeAddress& aCellAddress, sal_Bool& bIsFirst) const
{
    bIsFirst = sal_False;
    uno::Reference <table::XCellRange> xMatrixCellRange = xCellRange->getCellRangeByPosition(nCol,nRow,nCol,nRow);
    uno::Reference <sheet::XArrayFormulaRange> xArrayFormulaRange (xMatrixCellRange, uno::UNO_QUERY);
    if (xMatrixCellRange.is() && xArrayFormulaRange.is())
    {
        rtl::OUString sArrayFormula = xArrayFormulaRange->getArrayFormula();
        if (sArrayFormula.getLength())
        {
            uno::Reference<sheet::XSheetCellRange> xMatrixSheetCellRange (xMatrixCellRange, uno::UNO_QUERY);
            if (xMatrixSheetCellRange.is())
            {
                uno::Reference<sheet::XSheetCellCursor> xMatrixSheetCursor = xTable->createCursorByRange(xMatrixSheetCellRange);
                if (xMatrixSheetCursor.is())
                {
                    xMatrixSheetCursor->collapseToCurrentArray();
                    uno::Reference<sheet::XCellRangeAddressable> xMatrixCellAddress (xMatrixSheetCursor, uno::UNO_QUERY);
                    if (xMatrixCellAddress.is())
                    {
                        aCellAddress = xMatrixCellAddress->getRangeAddress();
                        if ((aCellAddress.StartColumn == nCol && aCellAddress.StartRow == nRow) &&
                            (aCellAddress.EndColumn > nCol || aCellAddress.EndRow > nRow))
                        {
                            bIsFirst = sal_True;
                            return sal_True;
                        }
                        else if (aCellAddress.StartColumn != nCol || aCellAddress.StartRow != nRow ||
                            aCellAddress.EndColumn != nCol || aCellAddress.EndRow != nRow)
                            return sal_True;
                        else
                        {
                            bIsFirst = sal_True;
                            return sal_True;
                        }
                    }
                }
            }
        }
    }
    return sal_False;
}

/*sal_Bool ScXMLExport::GetCell (const uno::Reference <table::XCellRange>& xCellRange,
                                   const sal_Int32 nCol, const sal_Int32 nRow, uno::Reference <table::XCell>& xTempCell) const
{
    xTempCell = xCellRange->getCellByPosition(nCol, nRow);
    return xTempCell.is();
}*/

sal_Bool ScXMLExport::GetCellText (const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell,
        rtl::OUString& sOUTemp) const
{
    uno::Reference <text::XText> xText (xCell, uno::UNO_QUERY);
    if (xText.is())
    {
        sOUTemp = xText->getString();
        return sal_True;
    }
    return sal_False;
}

sal_Int16 ScXMLExport::GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard)
{
    uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier = GetNumberFormatsSupplier();
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xNumberFormats = xNumberFormatsSupplier->getNumberFormats();
        if (xNumberFormats.is())
        {
            try
            {
                uno::Reference <beans::XPropertySet> xNumberPropertySet = xNumberFormats->getByKey(nNumberFormat);
                uno::Any aIsStandardFormat = xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_STANDARDFORMAT)));
                aIsStandardFormat >>= bIsStandard;
                uno::Any aNumberFormat = xNumberPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_TYPE)));
                sal_Int16 nNumberFormat;
                if ( aNumberFormat >>= nNumberFormat )
                {
                    return nNumberFormat;
                }
            }
            catch ( uno::Exception& )
            {
                DBG_ERROR("Numberformat not found");
            }
        }
    }
    return 0;
}

sal_Int32 ScXMLExport::GetCellNumberFormat(const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell) const
{
/*  uno::Reference <util::XNumberFormatsSupplier> xNumberFormatsSupplier = GetNumberFormatsSupplier();
    if (xNumberFormatsSupplier.is())
    {
        uno::Reference <util::XNumberFormats> xNumberFormats = xNumberFormatsSupplier->getNumberFormats();
        if (xNumberFormats.is())
        {*/
            uno::Reference <beans::XPropertySet> xPropertySet (xCell, uno::UNO_QUERY);
            if (xPropertySet.is())
            {
                uno::Any aNumberFormatPropertyKey = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_NUMFMT)));
                sal_Int32 nNumberFormatPropertyKey;
                if ( aNumberFormatPropertyKey>>=nNumberFormatPropertyKey )
                {
                    return nNumberFormatPropertyKey;
                }
            }
/*      }
    }*/
    return 0;
}

sal_Bool ScXMLExport::GetCellStyleNameIndex(const ScMyCell& aCell, sal_Int32& nStyleNameIndex, sal_Bool& bIsAutoStyle)
{
//  uno::Reference <beans::XPropertySet> xProperties (xCell, uno::UNO_QUERY);
//  if (xProperties.is())
//  {
//      uno::Any aStyle = xProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CELLSTYL)));
//      rtl::OUString sStyleName;
//      if (aStyle >>= sStyleName)
//      {
            //std::vector< XMLPropertyState > xPropStates = pCellStylesPropertySetMapper->Filter( xProperties );
            //rtl::OUString sName = GetAutoStylePool()->Find(XML_STYLE_FAMILY_TABLE_CELL, sStyleName, xPropStates);
            sal_Int32 nIndex = aCellStyles.GetStyleNameIndex(aCell.aCellAddress.Sheet, aCell.aCellAddress.Column, aCell.aCellAddress.Row, bIsAutoStyle);
            if (nIndex > -1)
            {
                nStyleNameIndex = nIndex;
                return sal_True;
            }
            /*else
            {
                rtl::OUString* pTemp = new rtl::OUString(sStyleName);
                nStyleNameIndex = aCellStyles.AddStyleName(pTemp, sal_False);
                return sal_True;
            }*/
//      }
//  }
    return sal_False;
}

OUString ScXMLExport::GetPrintRanges()
{
    rtl::OUString sPrintRanges;
    uno::Reference< sheet::XPrintAreas > xPrintAreas( xCurrentTable, uno::UNO_QUERY );
    if( xPrintAreas.is() )
    {
        uno::Sequence< table::CellRangeAddress > aRangeList( xPrintAreas->getPrintAreas() );
        ScXMLConverter::GetStringFromRangeList( sPrintRanges, aRangeList, pDoc );
    }
    return sPrintRanges;
}

void ScXMLExport::WriteCell (const ScMyCell& aCell)
{
    sal_Int32 nIndex;
    sal_Bool bIsAutoStyle;
    if (GetCellStyleNameIndex(aCell, nIndex, bIsAutoStyle))
        AddAttribute(XML_NAMESPACE_TABLE, sXML_style_name, *aCellStyles.GetStyleNameByIndex(nIndex, bIsAutoStyle));
    if (aCell.nValidationIndex > -1)
        AddAttribute(XML_NAMESPACE_TABLE, sXML_content_validation_name, aValidationsContainer.GetValidationName(aCell.nValidationIndex));
    sal_Bool bIsMatrix(aCell.bIsMatrixBase || aCell.bIsMatrixCovered);
    sal_Bool bIsFirstMatrixCell(aCell.bIsMatrixBase);
    if (bIsFirstMatrixCell)
    {
        sal_Int32 nColumns = aCell.aMatrixRange.EndColumn - aCell.aMatrixRange.StartColumn + 1;
        sal_Int32 nRows = aCell.aMatrixRange.EndRow - aCell.aMatrixRange.StartRow + 1;
        rtl::OUStringBuffer sColumns;
        rtl::OUStringBuffer sRows;
        SvXMLUnitConverter::convertNumber(sColumns, nColumns);
        SvXMLUnitConverter::convertNumber(sRows, nRows);
        AddAttribute(XML_NAMESPACE_TABLE, sXML_number_matrix_columns_spanned, sColumns.makeStringAndClear());
        AddAttribute(XML_NAMESPACE_TABLE, sXML_number_matrix_rows_spanned, sRows.makeStringAndClear());
    }
    table::CellContentType xCellType = aCell.xCell->getType();
    sal_Bool bIsEmpty = sal_False;
    switch (xCellType)
    {
    case table::CellContentType_EMPTY :
        {
            bIsEmpty = sal_True;
        }
        break;
    case table::CellContentType_VALUE :
        {
            XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(
                *this, GetCellNumberFormat(aCell.xCell), aCell.xCell->getValue(), XML_NAMESPACE_TABLE);
        }
        break;
    case table::CellContentType_TEXT :
        {
            rtl::OUString sValue;
            if (GetCellText(aCell.xCell, sValue))
                XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(
                    *this, aCell.xCell->getFormula(), sValue, XML_NAMESPACE_TABLE);
        }
        break;
    case table::CellContentType_FORMULA :
        {
            String sFormula;
            ScCellObj* pCellObj = (ScCellObj*) ScCellRangesBase::getImplementation( aCell.xCell );
            if ( pCellObj )
            {
                ScBaseCell* pBaseCell = pCellObj->GetDocument()->GetCell(pCellObj->GetPosition());
                ScFormulaCell* pFormulaCell = (ScFormulaCell*) pBaseCell;
                if (pBaseCell && pBaseCell->GetCellType() == CELLTYPE_FORMULA)
                {
                    if (!bIsMatrix || (bIsMatrix && bIsFirstMatrixCell))
                    {
                        pFormulaCell->GetEnglishFormula(sFormula, sal_True);
                        rtl::OUString sOUFormula(sFormula);
                        if (!bIsMatrix)
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_formula, sOUFormula);
                        else
                        {
                            rtl::OUString sMatrixFormula = sOUFormula.copy(1, sOUFormula.getLength() - 2);
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_formula, sMatrixFormula);
                        }
                    }
                    if (pFormulaCell->IsValue())
                    {
                        sal_Bool bIsStandard = sal_True;
                        GetCellType(GetCellNumberFormat(aCell.xCell), bIsStandard);
                        if (bIsStandard)
                        {
                            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( xModel, uno::UNO_QUERY );
                            if ( xSpreadDoc.is() )
                            {
                                if (pDoc)
                                {
                                    pFormulaCell->GetStandardFormat(*pDoc->GetFormatTable(), 0);
                                    XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(
                                        *this, pFormulaCell->GetStandardFormat(*pDoc->GetFormatTable(), 0),
                                        aCell.xCell->getValue(), XML_NAMESPACE_TABLE);
                                }
                            }
                        }
                        else
                            XMLNumberFormatAttributesExportHelper::SetNumberFormatAttributes(*this,
                                GetCellNumberFormat(aCell.xCell), aCell.xCell->getValue(), XML_NAMESPACE_TABLE);
                    }
                    else
                    {
                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_value_type, sXML_string);
                        rtl::OUString sValue;
                        if (GetCellText(aCell.xCell, sValue))
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_string_value, sValue);
                    }
                }
            }
        }
        break;
    }
    if (aCell.bIsCovered)
    {
        SvXMLElementExport aElemC(*this, XML_NAMESPACE_TABLE, sXML_covered_table_cell, sal_True, sal_True);
        CheckAttrList();
        WriteAreaLink(aCell);
        WriteAnnotation(aCell);
        WriteDetective(aCell);
        if (!bIsEmpty)
        {
            if (IsEditCell(aCell.xCell))
            {
                uno::Reference<text::XText> xText(aCell.xCell, uno::UNO_QUERY);
                if ( xText.is())
                    GetTextParagraphExport()->exportText(xText);
            }
            else
            {
                SvXMLElementExport aElemC(*this, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
                OUString sOUText;
                if (GetCellText(aCell.xCell, sOUText))
                    GetDocHandler()->characters(sOUText);
            }
        }
        WriteShapes(aCell);
    }
    else
    {
        if (aCell.bIsMergedBase)
        {
            sal_Int32 nColumns = aCell.aMergeRange.EndColumn - aCell.aMergeRange.StartColumn + 1;
            sal_Int32 nRows = aCell.aMergeRange.EndRow - aCell.aMergeRange.StartRow + 1;
            rtl::OUStringBuffer sColumns;
            rtl::OUStringBuffer sRows;
            SvXMLUnitConverter::convertNumber(sColumns, nColumns);
            SvXMLUnitConverter::convertNumber(sRows, nRows);
            AddAttribute(XML_NAMESPACE_TABLE, sXML_number_columns_spanned, sColumns.makeStringAndClear());
            AddAttribute(XML_NAMESPACE_TABLE, sXML_number_rows_spanned, sRows.makeStringAndClear());
        }
        SvXMLElementExport aElemC(*this, XML_NAMESPACE_TABLE, sXML_table_cell, sal_True, sal_True);
        CheckAttrList();
        WriteAreaLink(aCell);
        WriteAnnotation(aCell);
        WriteDetective(aCell);
        if (!bIsEmpty)
        {
            if (IsEditCell(aCell.xCell))
            {
                uno::Reference<text::XText> xText(aCell.xCell, uno::UNO_QUERY);
                OUString sOUText = xText->getString();
                if ( xText.is())
                    GetTextParagraphExport()->exportText(xText);
            }
            else
            {
                SvXMLElementExport aElemC(*this, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
                rtl::OUString sOUText;
                  if (GetCellText(aCell.xCell, sOUText))
                    GetDocHandler()->characters(sOUText);
            }
        }
        WriteShapes(aCell);
    }
}

void ScXMLExport::WriteShapes(const ScMyCell& rMyCell)
{
    if( rMyCell.bHasShape && xCurrentShapes.is() && rMyCell.aShapeVec.size() && pDoc )
    {
        awt::Point aPoint;
        Rectangle aRec = pDoc->GetMMRect(rMyCell.aCellAddress.Column, rMyCell.aCellAddress.Row,
            rMyCell.aCellAddress.Column, rMyCell.aCellAddress.Row, rMyCell.aCellAddress.Sheet);
        aPoint.X = aRec.Left();
        aPoint.Y = aRec.Top();
        awt::Point* pPoint = &aPoint;
        ScMyShapeVec::const_iterator aItr = rMyCell.aShapeVec.begin();
        while (aItr != rMyCell.aShapeVec.end())
        {
            uno::Any aAny = xCurrentShapes->getByIndex(aItr->nIndex);
            uno::Reference<drawing::XShape> xShape;
            if (aAny >>= xShape)
                GetShapeExport()->exportShape(xShape/*, pPoint*/);
            aItr++;
        }
    }
}

void ScXMLExport::WriteAreaLink( const ScMyCell& rMyCell )
{
    if( rMyCell.bHasAreaLink )
    {
        const ScMyAreaLink& rAreaLink = rMyCell.aAreaLink;
        AddAttribute( XML_NAMESPACE_TABLE, sXML_name, rAreaLink.sSourceStr );
        AddAttribute( XML_NAMESPACE_XLINK, sXML_href, rAreaLink.sURL );
        AddAttribute( XML_NAMESPACE_TABLE, sXML_filter_name, rAreaLink.sFilter );
        if( rAreaLink.sFilterOptions.getLength() )
            AddAttribute( XML_NAMESPACE_TABLE, sXML_filter_options, rAreaLink.sFilterOptions );
        OUStringBuffer sValue;
        SvXMLUnitConverter::convertNumber( sValue, rAreaLink.GetColCount() );
        AddAttribute( XML_NAMESPACE_TABLE, sXML_last_column_spanned, sValue.makeStringAndClear() );
        SvXMLUnitConverter::convertNumber( sValue, rAreaLink.GetRowCount() );
        AddAttribute( XML_NAMESPACE_TABLE, sXML_last_row_spanned, sValue.makeStringAndClear() );
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, sXML_cell_range_source, sal_True, sal_True );
    }
}

void ScXMLExport::WriteAnnotation(const ScMyCell& rMyCell)
{
    if( rMyCell.bHasAnnotation )
    {
        uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor(rMyCell.xCell, uno::UNO_QUERY);
        if (xSheetAnnotationAnchor.is())
        {
            uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation = xSheetAnnotationAnchor->getAnnotation();
            uno::Reference<text::XSimpleText> xSimpleText(xSheetAnnotation, uno::UNO_QUERY);
            if (xSheetAnnotation.is() && xSimpleText.is())
            {
                rtl::OUString sText = xSimpleText->getString();
                if (sText.getLength())
                {
                    AddAttribute(XML_NAMESPACE_OFFICE, sXML_author, xSheetAnnotation->getAuthor());
                    String aDate(xSheetAnnotation->getDate());
                    if (pDoc)
                    {
                        SvNumberFormatter* pNumForm = pDoc->GetFormatTable();
                        double fDate;
                        sal_uInt32 nfIndex = pNumForm->GetFormatIndex(NF_DATE_SYS_DDMMYYYY, LANGUAGE_SYSTEM);
                        if (pNumForm->IsNumberFormat(aDate, nfIndex, fDate))
                        {
                            rtl::OUStringBuffer sBuf;
                            GetMM100UnitConverter().convertDateTime(sBuf, fDate);
                            AddAttribute(XML_NAMESPACE_OFFICE, sXML_create_date, sBuf.makeStringAndClear());
                        }
                        else
                            AddAttribute(XML_NAMESPACE_OFFICE, sXML_create_date_string, rtl::OUString(aDate));
                    }
                    else
                        AddAttribute(XML_NAMESPACE_OFFICE, sXML_create_date_string, rtl::OUString(aDate));
                    if (!xSheetAnnotation->getIsVisible())
                        AddAttributeASCII(XML_NAMESPACE_OFFICE, sXML_display, sXML_false);
                    SvXMLElementExport aElemA(*this, XML_NAMESPACE_OFFICE, sXML_annotation, sal_True, sal_False);
                    GetDocHandler()->characters(sText);
                }
            }
            CheckAttrList();
        }
    }
}

void ScXMLExport::WriteDetective( const ScMyCell& rMyCell )
{
    if( rMyCell.bHasDetectiveObj || rMyCell.bHasDetectiveOp )
    {
        const ScMyDetectiveObjVec& rObjVec = rMyCell.aDetectiveObjVec;
        const ScMyDetectiveOpVec& rOpVec = rMyCell.aDetectiveOpVec;
        sal_Int32 nObjCount = rObjVec.size();
        sal_Int32 nOpCount = rOpVec.size();
        if( nObjCount || nOpCount )
        {
            SvXMLElementExport aDetElem( *this, XML_NAMESPACE_TABLE, sXML_detective, sal_True, sal_True );
            OUString sString;
            for( ScMyDetectiveObjVec::const_iterator aObjItr = rObjVec.begin(); aObjItr != rObjVec.end(); aObjItr++ )
            {
                if( aObjItr->eObjType == SC_DETOBJ_ARROW )
                {
                    ScXMLConverter::GetStringFromRange( sString, aObjItr->aSourceRange, pDoc );
                    AddAttribute( XML_NAMESPACE_TABLE, sXML_cell_range_address, sString );
                }
                ScXMLConverter::GetStringFromDetObjType( sString, aObjItr->eObjType );
                AddAttribute( XML_NAMESPACE_TABLE, sXML_direction, sString );
                if( aObjItr->bHasError )
                    AddAttributeASCII( XML_NAMESPACE_TABLE, sXML_contains_error, sXML_true );
                SvXMLElementExport aRangeElem( *this, XML_NAMESPACE_TABLE, sXML_highlighted_range, sal_True, sal_True );
            }
            OUStringBuffer aBuffer;
            for( ScMyDetectiveOpVec::const_iterator aOpItr = rOpVec.begin(); aOpItr != rOpVec.end(); aOpItr++ )
            {
                OUString sString;
                ScXMLConverter::GetStringFromDetOpType( sString, aOpItr->eOpType );
                AddAttribute( XML_NAMESPACE_TABLE, sXML_name, sString );
                SvXMLUnitConverter::convertNumber( aBuffer, aOpItr->nIndex );
                AddAttribute( XML_NAMESPACE_TABLE, sXML_index, aBuffer.makeStringAndClear() );
                SvXMLElementExport aRangeElem( *this, XML_NAMESPACE_TABLE, sXML_operation, sal_True, sal_True );
            }
        }
    }
}

void ScXMLExport::SetRepeatAttribute (const sal_Int32 nEqualCellCount)
{
    if (nEqualCellCount > 0)
    {
        sal_Int32 nTemp = nEqualCellCount + 1;
        OUString sOUEqualCellCount = OUString::valueOf(nTemp);
        AddAttribute(XML_NAMESPACE_TABLE, sXML_number_columns_repeated, sOUEqualCellCount);
    }
}

sal_Bool ScXMLExport::IsCellTypeEqual (const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell1,
        const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell2) const
{
    return (xCell1->getType() == xCell2->getType());
}

sal_Bool ScXMLExport::IsEditCell(const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell) const
{
    ScCellObj* pCellObj = (ScCellObj*) ScCellRangesBase::getImplementation( xCell );
    if ( pCellObj )
    {
        ScBaseCell* pBaseCell = pCellObj->GetDocument()->GetCell(pCellObj->GetPosition());
        if (pBaseCell)
        {
            if (pBaseCell->GetCellType() == CELLTYPE_EDIT)
                return sal_True;
            else
                return sal_False;
        }
    }
    return sal_True;
}

sal_Bool ScXMLExport::IsAnnotationEqual(const uno::Reference<table::XCell>& xCell1,
                                        const uno::Reference<table::XCell>& xCell2)
{
    uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor1(xCell1, uno::UNO_QUERY);
    uno::Reference<sheet::XSheetAnnotationAnchor> xSheetAnnotationAnchor2(xCell2, uno::UNO_QUERY);
    if (xSheetAnnotationAnchor1.is() && xSheetAnnotationAnchor2.is())
    {
        uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation1 = xSheetAnnotationAnchor1->getAnnotation();
        uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation2 = xSheetAnnotationAnchor2->getAnnotation();
        uno::Reference<text::XSimpleText> xSimpleText1(xSheetAnnotation1, uno::UNO_QUERY);
        uno::Reference<text::XSimpleText> xSimpleText2(xSheetAnnotation2, uno::UNO_QUERY);
        if (xSheetAnnotation1.is() && xSimpleText1.is() &&
            xSheetAnnotation2.is() && xSimpleText2.is())
        {
            rtl::OUString sText1 = xSimpleText1->getString();
            rtl::OUString sText2 = xSimpleText2->getString();
            sal_Int32 nLength1 = sText1.getLength();
            sal_Int32 nLength2 = sText2.getLength();
            if (nLength1 && nLength2)
                if (sText1 == sText2 &&
                    xSheetAnnotation1->getAuthor() == xSheetAnnotation2->getAuthor() &&
                    xSheetAnnotation1->getDate() == xSheetAnnotation2->getDate() &&
                    xSheetAnnotation1->getIsVisible() == xSheetAnnotation2->getIsVisible())
                    return sal_True;
                else
                    return sal_False;
            else
                if (nLength1 || nLength2)
                    return sal_False;
                else
                    return sal_True;
        }
    }
    return sal_False;
}

sal_Bool ScXMLExport::IsCellEqual (const ScMyCell& aCell1, const ScMyCell& aCell2)
{
    sal_Bool bIsEqual = sal_False;
    if( !aCell1.bIsMergedBase && !aCell2.bIsMergedBase &&
        aCell1.bIsCovered == aCell2.bIsCovered &&
        !aCell1.bIsMatrixBase && !aCell2.bIsMatrixBase &&
        aCell1.bIsMatrixCovered == aCell2.bIsMatrixCovered &&
        aCell1.bHasAnnotation == aCell2.bHasAnnotation &&
        !aCell1.bHasShape && !aCell2.bHasShape &&
        aCell1.nValidationIndex == aCell2.nValidationIndex &&
        aCell1.bHasAreaLink == aCell2.bHasAreaLink &&
        !aCell1.bHasDetectiveObj && !aCell2.bHasDetectiveObj)
    {
        if( aCell1.bHasAreaLink &&
            (aCell1.aAreaLink.GetColCount() == 1) &&
            (aCell2.aAreaLink.GetColCount() == 1) &&
            aCell1.aAreaLink.Compare( aCell2.aAreaLink ) )
        {
            if (!aCell1.bHasAnnotation || (aCell1.bHasAnnotation && IsAnnotationEqual(aCell1.xCell, aCell2.xCell)))
            {
                sal_Int32 nIndex1, nIndex2;
                sal_Bool bIsAutoStyle1, bIsAutoStyle2;
                if (GetCellStyleNameIndex(aCell1, nIndex1, bIsAutoStyle1) &&
                    GetCellStyleNameIndex(aCell2, nIndex2, bIsAutoStyle2))
                {
                    if ((nIndex1 == nIndex2) && (bIsAutoStyle1 == bIsAutoStyle2) &&
                        IsCellTypeEqual(aCell1.xCell, aCell2.xCell))
                    {
                        table::CellContentType eCellType = aCell1.xCell->getType();
                        switch ( eCellType )
                        {
                        case table::CellContentType_EMPTY :
                            {
                                bIsEqual = sal_True;
                            }
                            break;
                        case table::CellContentType_VALUE :
                            {
                                double fCell1 = aCell1.xCell->getValue();
                                double fCell2 = aCell2.xCell->getValue();
                                bIsEqual = (fCell1 == fCell2);
                            }
                            break;
                        case table::CellContentType_TEXT :
                            {
                                if (IsEditCell(aCell1.xCell) || IsEditCell(aCell2.xCell))
                                    bIsEqual = sal_False;
                                else
                                {
                                    OUString sOUCell1, sOUCell2;
                                    if (GetCellText(aCell1.xCell, sOUCell1) && GetCellText(aCell2.xCell, sOUCell2))
                                    {
                                        bIsEqual = (sOUCell1 == sOUCell2);
                                    }
                                    else
                                        bIsEqual = sal_False;
                                }
                            }
                            break;
                        case table::CellContentType_FORMULA :
                            {
                                bIsEqual = sal_False;
                            }
                            break;
                        default :
                            {
                                bIsEqual = sal_False;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    return bIsEqual;
}

// core implementation
void ScXMLExport::WriteScenario()
{
    if (pDoc->IsScenario(nCurrentTable))
    {
        String      sComment;
        Color       aColor;
        sal_uInt16  nFlags;
        pDoc->GetScenarioData(nCurrentTable, sComment, aColor, nFlags);
        if (!(nFlags & SC_SCENARIO_SHOWFRAME))
            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_display_border, sXML_false);
        rtl::OUStringBuffer aBuffer;
        SvXMLUnitConverter::convertColor(aBuffer, aColor);
        AddAttribute(XML_NAMESPACE_TABLE, sXML_border_color, aBuffer.makeStringAndClear());
        if (!(nFlags & SC_SCENARIO_TWOWAY))
            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_copy_back, sXML_false);
        if (!(nFlags & SC_SCENARIO_ATTRIB))
            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_copy_styles, sXML_false);
        if (nFlags & SC_SCENARIO_VALUE)
            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_copy_formulas, sXML_false);
        SvXMLUnitConverter::convertBool(aBuffer, pDoc->IsActiveScenario(nCurrentTable));
        AddAttribute(XML_NAMESPACE_TABLE, sXML_is_active, aBuffer.makeStringAndClear());
        const ScRangeList* pRangeList = pDoc->GetScenarioRanges(nCurrentTable);
        rtl::OUString sRangeListStr;
        ScXMLConverter::GetStringFromRangeList( sRangeListStr, pRangeList, pDoc );
        AddAttribute(XML_NAMESPACE_TABLE, sXML_scenario_ranges, sRangeListStr);
        if (sComment.Len())
            AddAttribute(XML_NAMESPACE_TABLE, sXML_comment, rtl::OUString(sComment));
        SvXMLElementExport aElem(*this, XML_NAMESPACE_TABLE, sXML_scenario, sal_True, sal_True);
    }
}

void ScXMLExport::WriteTheLabelRanges( const uno::Reference< sheet::XSpreadsheetDocument >& xSpreadDoc )
{
    uno::Reference< beans::XPropertySet > xDocProp( xSpreadDoc, uno::UNO_QUERY );
    if( !xDocProp.is() ) return;

    sal_Int32 nCount = 0;
    uno::Any aAny = xDocProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_COLLABELRNG ) ) );
    uno::Reference< sheet::XLabelRanges > xLabelRanges;
    uno::Reference< container::XIndexAccess > xColRangesIAccess;
    if( aAny >>= xLabelRanges )
        xColRangesIAccess = uno::Reference< container::XIndexAccess >( xLabelRanges, uno::UNO_QUERY );
    if( xColRangesIAccess.is() )
        nCount += xColRangesIAccess->getCount();

    aAny = xDocProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNO_ROWLABELRNG ) ) );
    uno::Reference< container::XIndexAccess > xRowRangesIAccess;
    if( aAny >>= xLabelRanges )
        xRowRangesIAccess = uno::Reference< container::XIndexAccess >( xLabelRanges, uno::UNO_QUERY );
    if( xRowRangesIAccess.is() )
        nCount += xRowRangesIAccess->getCount();

    if( nCount )
    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, sXML_label_ranges, sal_True, sal_True );
        WriteLabelRanges( xColRangesIAccess, sal_True );
        WriteLabelRanges( xRowRangesIAccess, sal_False );
    }
}

void ScXMLExport::WriteLabelRanges( const uno::Reference< container::XIndexAccess >& xRangesIAccess, sal_Bool bColumn )
{
    if( !xRangesIAccess.is() ) return;

    sal_Int32 nCount = xRangesIAccess->getCount();
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        uno::Any aRangeAny = xRangesIAccess->getByIndex( nIndex );
        uno::Reference< sheet::XLabelRange > xRange;
        if( aRangeAny >>= xRange )
        {
            OUString sRangeStr;
            table::CellRangeAddress aCellRange( xRange->getLabelArea() );
            ScXMLConverter::GetStringFromRange( sRangeStr, aCellRange, pDoc );
            AddAttribute( XML_NAMESPACE_TABLE, sXML_label_cell_range_address, sRangeStr );
            aCellRange = xRange->getDataArea();
            ScXMLConverter::GetStringFromRange( sRangeStr, aCellRange, pDoc );
            AddAttribute( XML_NAMESPACE_TABLE, sXML_data_cell_range_address, sRangeStr );
            AddAttribute( XML_NAMESPACE_TABLE, sXML_orientation, OUString::createFromAscii( bColumn ? sXML_column : sXML_row ) );
            SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, sXML_label_range, sal_True, sal_True );
        }
    }
}

void ScXMLExport::WriteNamedExpressions(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        uno::Any aNamedRanges = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_NAMEDRANGES)));
        uno::Reference <sheet::XNamedRanges> xNamedRanges;
        CheckAttrList();
        if (aNamedRanges >>= xNamedRanges)
        {
            uno::Sequence <rtl::OUString> aRangesNames = xNamedRanges->getElementNames();
            sal_Int32 nNamedRangesCount = aRangesNames.getLength();
            if (nNamedRangesCount > 0)
            {
                if (pDoc)
                {
                    ScRangeName* pNamedRanges = pDoc->GetRangeName();
                    SvXMLElementExport aElemNEs(*this, XML_NAMESPACE_TABLE, sXML_named_expressions, sal_True, sal_True);
                    for (sal_Int32 i = 0; i < nNamedRangesCount; i++)
                    {
                        CheckAttrList();
                        rtl::OUString sNamedRange = aRangesNames[i];
                        uno::Any aNamedRange = xNamedRanges->getByName(sNamedRange);
                        uno::Reference <sheet::XNamedRange> xNamedRange;
                        if (aNamedRange >>= xNamedRange)
                        {
                            uno::Reference <container::XNamed> xNamed (xNamedRange, uno::UNO_QUERY);
                            uno::Reference <sheet::XCellRangeReferrer> xCellRangeReferrer (xNamedRange, uno::UNO_QUERY);
                            if (xNamed.is() && xCellRangeReferrer.is())
                            {
                                rtl::OUString sOUName = xNamed->getName();
                                AddAttribute(XML_NAMESPACE_TABLE, sXML_name, sOUName);

                                OUString sOUBaseCellAddress;
                                ScXMLConverter::GetStringFromAddress( sOUBaseCellAddress,
                                    xNamedRange->getReferencePosition(), pDoc, sal_False, SCA_ABS_3D );
                                AddAttribute(XML_NAMESPACE_TABLE, sXML_base_cell_address, sOUBaseCellAddress);

                                sal_uInt16 nRangeIndex;
                                String sName(sOUName);
                                pNamedRanges->SearchName(sName, nRangeIndex);
                                ScRangeData* pNamedRange = (*pNamedRanges)[nRangeIndex]; //should get directly and not with ScDocument
                                String sContent(xNamedRange->getContent());
                                pNamedRange->GetEnglishSymbol(sContent, sal_True);
                                rtl::OUString sOUTempContent(sContent);
                                uno::Reference <table::XCellRange> xCellRange = xCellRangeReferrer->getReferredCells();
                                if(xCellRange.is())
                                {
                                    rtl::OUString sOUContent = sOUTempContent.copy(1, sOUTempContent.getLength() - 2);
                                    AddAttribute(XML_NAMESPACE_TABLE, sXML_cell_range_address, sOUContent);
                                    sal_Int32 nRangeType = xNamedRange->getType();
                                    rtl::OUStringBuffer sBufferRangeType;
                                    if (nRangeType & sheet::NamedRangeFlag::COLUMN_HEADER == sheet::NamedRangeFlag::COLUMN_HEADER)
                                        sBufferRangeType.appendAscii(sXML_repeat_column);
                                    if (nRangeType & sheet::NamedRangeFlag::ROW_HEADER == sheet::NamedRangeFlag::ROW_HEADER)
                                    {
                                        if (sBufferRangeType.getLength() > 0)
                                            sBufferRangeType.appendAscii(" ");
                                        sBufferRangeType.appendAscii(sXML_repeat_row);
                                    }
                                    if (nRangeType & sheet::NamedRangeFlag::FILTER_CRITERIA == sheet::NamedRangeFlag::FILTER_CRITERIA)
                                    {
                                        if (sBufferRangeType.getLength() > 0)
                                            sBufferRangeType.appendAscii(" ");
                                        sBufferRangeType.appendAscii(sXML_filter);
                                    }
                                    if (nRangeType & sheet::NamedRangeFlag::PRINT_AREA == sheet::NamedRangeFlag::PRINT_AREA)
                                    {
                                        if (sBufferRangeType.getLength() > 0)
                                            sBufferRangeType.appendAscii(" ");
                                        sBufferRangeType.appendAscii(sXML_print_range);
                                    }
                                    rtl::OUString sRangeType = sBufferRangeType.makeStringAndClear();
                                    AddAttribute(XML_NAMESPACE_TABLE, sXML_range_usable_as, sRangeType);
                                    SvXMLElementExport aElemNR(*this, XML_NAMESPACE_TABLE, sXML_named_range, sal_True, sal_True);
                                }
                                else
                                {
                                    AddAttribute(XML_NAMESPACE_TABLE, sXML_expression, sOUTempContent);
                                    SvXMLElementExport aElemNE(*this, XML_NAMESPACE_TABLE, sXML_named_expression, sal_True, sal_True);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ScXMLExport::WriteImportDescriptor(const uno::Sequence <beans::PropertyValue> aImportDescriptor)
{
    sal_Int32 nProperties = aImportDescriptor.getLength();
    rtl::OUString sDatabaseName;
    rtl::OUString sSourceObject;
    sheet::DataImportMode nSourceType;
    sal_Bool bNative;
    for (sal_Int16 i = 0; i < nProperties; i++)
    {
        if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_DBNAME)))
        {
            uno::Any aDatabaseName = aImportDescriptor[i].Value;
            aDatabaseName >>= sDatabaseName;
        }
        else if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SRCOBJ)))
        {
            uno::Any aSourceObject = aImportDescriptor[i].Value;
            aSourceObject >>= sSourceObject;
        }
        else if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SRCTYPE)))
        {
            uno::Any aSourceType = aImportDescriptor[i].Value;
            aSourceType >>= nSourceType;
        }
        else if (aImportDescriptor[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_NATIVE)))
        {
            uno::Any aNative = aImportDescriptor[i].Value;
            aNative >>= bNative;
        }
    }
    switch (nSourceType)
    {
        case sheet::DataImportMode_NONE : break;
        case sheet::DataImportMode_QUERY :
        {
            AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, sDatabaseName);
            AddAttribute(XML_NAMESPACE_TABLE, sXML_query_name, sSourceObject);
            SvXMLElementExport aElemID(*this, XML_NAMESPACE_TABLE, sXML_database_source_query, sal_True, sal_True);
            CheckAttrList();
        }
        break;
        case sheet::DataImportMode_TABLE :
        {
            AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, sDatabaseName);
            AddAttribute(XML_NAMESPACE_TABLE, sXML_table_name, sSourceObject);
            SvXMLElementExport aElemID(*this, XML_NAMESPACE_TABLE, sXML_database_source_table, sal_True, sal_True);
            CheckAttrList();
        }
        break;
        case sheet::DataImportMode_SQL :
        {
            AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, sDatabaseName);
            AddAttribute(XML_NAMESPACE_TABLE, sXML_sql_statement, sSourceObject);
            if (!bNative)
                AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_parse_sql_statement, sXML_true);
            SvXMLElementExport aElemID(*this, XML_NAMESPACE_TABLE, sXML_database_source_sql, sal_True, sal_True);
            CheckAttrList();
        }
        break;
    }
}

rtl::OUString ScXMLExport::getOperatorXML(const sheet::FilterOperator aFilterOperator, const sal_Bool bUseRegularExpressions) const
{
    if (bUseRegularExpressions)
    {
        switch (aFilterOperator)
        {
            case sheet::FilterOperator_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_match));
                break;
            case sheet::FilterOperator_NOT_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_nomatch));
                break;
        }
    }
    else
    {
        switch (aFilterOperator)
        {
            case sheet::FilterOperator_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
                break;
            case sheet::FilterOperator_NOT_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
                break;
            case sheet::FilterOperator_BOTTOM_PERCENT :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bottom_percent));
                break;
            case sheet::FilterOperator_BOTTOM_VALUES :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bottom_values));
                break;
            case sheet::FilterOperator_EMPTY :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_empty));
                break;
            case sheet::FilterOperator_GREATER :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
                break;
            case sheet::FilterOperator_GREATER_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
                break;
            case sheet::FilterOperator_LESS :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
                break;
            case sheet::FilterOperator_LESS_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
                break;
            case sheet::FilterOperator_NOT_EMPTY :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_noempty));
                break;
            case sheet::FilterOperator_TOP_PERCENT :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_top_percent));
                break;
            case sheet::FilterOperator_TOP_VALUES :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_top_values));
                break;
        }
    }
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
}

void ScXMLExport::WriteCondition(const sheet::TableFilterField& aFilterField, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions)
{
    AddAttribute(XML_NAMESPACE_TABLE, sXML_field_number, rtl::OUString::valueOf(aFilterField.Field));
    if (bIsCaseSensitive)
        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_case_sensitive, sXML_true);
    if (aFilterField.IsNumeric)
    {
        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_data_type, sXML_number);
        rtl::OUStringBuffer sBuffer;
        GetMM100UnitConverter().convertNumber(sBuffer, aFilterField.NumericValue);
        AddAttribute(XML_NAMESPACE_TABLE, sXML_value, sBuffer.makeStringAndClear());
    }
    else
        AddAttribute(XML_NAMESPACE_TABLE, sXML_value, aFilterField.StringValue);
    AddAttribute(XML_NAMESPACE_TABLE, sXML_operator, getOperatorXML(aFilterField.Operator, bUseRegularExpressions));
    SvXMLElementExport aElemC(*this, XML_NAMESPACE_TABLE, sXML_filter_condition, sal_True, sal_True);
}

void ScXMLExport::WriteFilterDescriptor(const uno::Reference <sheet::XSheetFilterDescriptor>& xSheetFilterDescriptor, const rtl::OUString sDatabaseRangeName)
{
    uno::Sequence <sheet::TableFilterField> aTableFilterFields = xSheetFilterDescriptor->getFilterFields();
    sal_Int32 nTableFilterFields = aTableFilterFields.getLength();
    if (nTableFilterFields > 0)
    {
        uno::Reference <beans::XPropertySet> xPropertySet (xSheetFilterDescriptor, uno::UNO_QUERY);
        if (xPropertySet.is())
        {
            sal_Bool bCopyOutputData;
            uno::Any aCopyOutputData = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_COPYOUT)));
            if (aCopyOutputData >>= bCopyOutputData)
                if (bCopyOutputData)
                {
                    table::CellAddress aOutputPosition;
                    uno::Any aTempOutputPosition = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_OUTPOS)));
                    if (aTempOutputPosition >>= aOutputPosition)
                    {
                        OUString sOUCellAddress;
                        ScXMLConverter::GetStringFromAddress( sOUCellAddress, aOutputPosition, pDoc );
                        AddAttribute(XML_NAMESPACE_TABLE, sXML_target_range_address, sOUCellAddress);
                    }
                }
            ScDBCollection* pDBCollection = pDoc->GetDBCollection();
            sal_uInt16 nIndex;
            pDBCollection->SearchName(sDatabaseRangeName, nIndex);
            ScDBData* pDBData = (*pDBCollection)[nIndex];
            ScRange aAdvSource;
            if (pDBData->GetAdvancedQuerySource(aAdvSource))
            {
                rtl::OUString sOUCellAddress;
                ScXMLConverter::GetStringFromRange( sOUCellAddress, aAdvSource, pDoc );
                AddAttribute(XML_NAMESPACE_TABLE, sXML_target_range_address, sOUCellAddress);
            }

            sal_Bool bSkipDuplicates;
            uno::Any aSkipDuplicates = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SKIPDUP)));
            if (aSkipDuplicates >>= bSkipDuplicates)
                if (bSkipDuplicates)
                    AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_display_duplicates, sXML_false);
            SvXMLElementExport aElemF(*this, XML_NAMESPACE_TABLE, sXML_filter, sal_True, sal_True);
            CheckAttrList();
            sal_Bool bIsCaseSensitive = sal_False;
            uno::Any aIsCaseSensitive = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ISCASE)));
            aIsCaseSensitive >>= bIsCaseSensitive;
            sal_Bool bUseRegularExpressions = sal_False;
            uno::Any aUseRegularExpressions = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_USEREGEX)));
            aUseRegularExpressions >>= bUseRegularExpressions;
            sal_Bool bAnd = sal_False;
            sal_Bool bOr = sal_False;
            for (sal_Int32 i = 1; i < nTableFilterFields; i++)
            {
                if (aTableFilterFields[i].Connection == sheet::FilterConnection_AND)
                    bAnd = sal_True;
                else
                    bOr = sal_True;
            }
            if (bOr && !bAnd)
            {
                SvXMLElementExport aElemOr(*this, XML_NAMESPACE_TABLE, sXML_filter_or, sal_True, sal_True);
                for (i = 0; i < nTableFilterFields; i++)
                {
                    WriteCondition(aTableFilterFields[i], bIsCaseSensitive, bUseRegularExpressions);
                }
            }
            else if (bAnd && !bOr)
            {
                SvXMLElementExport aElemAnd(*this, XML_NAMESPACE_TABLE, sXML_filter_and, sal_True, sal_True);
                for (i = 0; i < nTableFilterFields; i++)
                {
                    WriteCondition(aTableFilterFields[i], bIsCaseSensitive, bUseRegularExpressions);
                }
            }
            else if (nTableFilterFields  == 1)
            {
                WriteCondition(aTableFilterFields[0], bIsCaseSensitive, bUseRegularExpressions);
            }
            else
            {
                SvXMLElementExport aElemC(*this, XML_NAMESPACE_TABLE, sXML_filter_or, sal_True, sal_True);
                sheet::TableFilterField aPrevFilterField = aTableFilterFields[0];
                sheet::FilterConnection aConnection = aTableFilterFields[1].Connection;
                sal_Bool bOpenAndElement;
                rtl::OUString aName = GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_filter_and)));
                if (aConnection == sheet::FilterConnection_AND)
                {
                    GetDocHandler()->ignorableWhitespace(sWS);
                    GetDocHandler()->startElement( aName, GetXAttrList());
                    ClearAttrList();
                    bOpenAndElement = sal_True;
                }
                else
                    bOpenAndElement = sal_False;
                for (i = 1; i < nTableFilterFields; i++)
                {
                    if (aConnection != aTableFilterFields[i].Connection)
                    {
                        aConnection = aTableFilterFields[i].Connection;
                        if (aTableFilterFields[i].Connection == sheet::FilterConnection_AND)
                        {
                            GetDocHandler()->ignorableWhitespace(sWS);
                            GetDocHandler()->startElement( aName, GetXAttrList());
                            ClearAttrList();
                            bOpenAndElement = sal_True;
                            WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                            aPrevFilterField = aTableFilterFields[i];
                            if (i == nTableFilterFields - 1)
                            {
                                WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                                GetDocHandler()->ignorableWhitespace(sWS);
                                GetDocHandler()->endElement(aName);
                                bOpenAndElement = sal_False;
                            }
                        }
                        else
                        {
                            WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                            aPrevFilterField = aTableFilterFields[i];
                            if (bOpenAndElement)
                            {
                                GetDocHandler()->ignorableWhitespace(sWS);
                                GetDocHandler()->endElement(aName);
                                bOpenAndElement = sal_False;
                            }
                            if (i == nTableFilterFields - 1)
                            {
                                WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                            }
                        }
                    }
                    else
                    {
                        WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                        aPrevFilterField = aTableFilterFields[i];
                        if (i == nTableFilterFields - 1)
                            WriteCondition(aPrevFilterField, bIsCaseSensitive, bUseRegularExpressions);
                    }
                }
            }
        }
    }
}

void ScXMLExport::WriteSortDescriptor(const uno::Sequence <beans::PropertyValue> aSortProperties)
{
    uno::Sequence <util::SortField> aSortFields;
    sal_Bool bBindFormatsToContent = sal_True;
    sal_Bool bCopyOutputData = sal_False;
    sal_Bool bIsCaseSensitive = sal_False;
    sal_Bool bIsUserListEnabled = sal_False;
    table::CellAddress aOutputPosition;
    sal_Int32 nUserListIndex;
    sal_Int32 nProperties = aSortProperties.getLength();
    for (sal_Int32 i = 0; i < nProperties; i++)
    {
        if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_BINDFMT)))
        {
            uno::Any aBindFormatsToContent = aSortProperties[i].Value;
            aBindFormatsToContent >>= bBindFormatsToContent;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_COPYOUT)))
        {
            uno::Any aCopyOutputData = aSortProperties[i].Value;
            aCopyOutputData >>= bCopyOutputData;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ISCASE)))
        {
            uno::Any aIsCaseSensitive = aSortProperties[i].Value;
            aIsCaseSensitive >>= bIsCaseSensitive;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ISULIST)))
        {
            uno::Any aIsUserListEnabled = aSortProperties[i].Value;
            aIsUserListEnabled >>= bIsUserListEnabled;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_OUTPOS)))
        {
            uno::Any aTempOutputPosition = aSortProperties[i].Value;
            aTempOutputPosition >>= aOutputPosition;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_UINDEX)))
        {
            uno::Any aUserListIndex = aSortProperties[i].Value;
            aUserListIndex >>= nUserListIndex;
        }
        else if (aSortProperties[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SORTFLD)))
        {
            uno::Any aTempSortFields = aSortProperties[i].Value;
            aTempSortFields >>= aSortFields;
        }
    }
    sal_Int32 nSortFields = aSortFields.getLength();
    if (nSortFields > 0)
    {
        if (!bBindFormatsToContent)
            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_bind_styles_to_content, sXML_false);
        if (bCopyOutputData)
        {
            OUString sOUCellAddress;
            ScXMLConverter::GetStringFromAddress( sOUCellAddress, aOutputPosition, pDoc );
            AddAttribute(XML_NAMESPACE_TABLE, sXML_target_range_address, sOUCellAddress);
        }
        if (bIsCaseSensitive)
            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_case_sensitive, sXML_true);
        SvXMLElementExport aElemS(*this, XML_NAMESPACE_TABLE, sXML_sort, sal_True, sal_True);
        CheckAttrList();
        for (i = 0; i < nSortFields; i++)
        {
            AddAttribute(XML_NAMESPACE_TABLE, sXML_field_number, rtl::OUString::valueOf(aSortFields[i].Field));
            if (!aSortFields[i].SortAscending)
                AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_order, sXML_descending);
            if (!bIsUserListEnabled)
            {
                switch (aSortFields[i].FieldType)
                {
                    case util::SortFieldType_ALPHANUMERIC :
                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_data_type, sXML_text);
                    break;
                    case util::SortFieldType_AUTOMATIC :
                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_data_type, sXML_automatic);
                    break;
                    case util::SortFieldType_NUMERIC :
                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_data_type, sXML_number);
                    break;
                }
            }
            else
                AddAttribute(XML_NAMESPACE_TABLE, sXML_data_type, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_USERLIST)) + rtl::OUString::valueOf(nUserListIndex));
            SvXMLElementExport aElemSb(*this, XML_NAMESPACE_TABLE, sXML_sort_by, sal_True, sal_True);
            CheckAttrList();
        }
    }
}

void ScXMLExport::WriteSubTotalDescriptor(const com::sun::star::uno::Reference <com::sun::star::sheet::XSubTotalDescriptor> xSubTotalDescriptor, const rtl::OUString sDatabaseRangeName)
{
    uno::Reference <container::XIndexAccess> xIndexAccess (xSubTotalDescriptor, uno::UNO_QUERY);
    if (xIndexAccess.is())
    {
        sal_Int32 nSubTotalFields = xIndexAccess->getCount();
        if (nSubTotalFields > 0)
        {
            uno::Reference <beans::XPropertySet> xPropertySet (xSubTotalDescriptor, uno::UNO_QUERY);
            sal_Bool bEnableUserSortList = sal_False;
            sal_Bool bSortAscending = sal_True;
            sal_Int32 nUserSortListIndex = 0;
            if (xPropertySet.is())
            {
                sal_Bool bBindFormatsToContent;
                uno::Any aBindFormatsToContent = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_BINDFMT)));
                if (aBindFormatsToContent >>= bBindFormatsToContent)
                    if (!bBindFormatsToContent)
                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_bind_styles_to_content, sXML_false);
                sal_Bool bInsertPageBreaks;
                uno::Any aInsertPageBreaks = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_INSBRK)));
                if (aInsertPageBreaks >>= bInsertPageBreaks)
                    if (bInsertPageBreaks)
                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_page_breaks_on_group_change, sXML_true);
                sal_Bool bIsCaseSensitive;
                uno::Any aIsCaseSensitive = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ISCASE)));
                if (aIsCaseSensitive >>= bIsCaseSensitive)
                    if (bIsCaseSensitive)
                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_case_sensitive, sXML_true);
                uno::Any aSortAscending = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_SORTASCENDING)));
                aSortAscending >>= bSortAscending;
                uno::Any aEnabledUserSortList = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_ENABLEUSERSORTLIST)));
                if (aEnabledUserSortList >>= bEnableUserSortList)
                    if (bEnableUserSortList)
                    {
                        uno::Any aUserSortListIndex = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_USERSORTLISTINDEX)));
                        aUserSortListIndex >>= nUserSortListIndex;
                    }
            }
            SvXMLElementExport aElemSTRs(*this, XML_NAMESPACE_TABLE, sXML_subtotal_rules, sal_True, sal_True);
            CheckAttrList();
            {
                ScDBCollection* pDBCollection = pDoc->GetDBCollection();
                sal_uInt16 nIndex;
                pDBCollection->SearchName(sDatabaseRangeName, nIndex);
                ScDBData* pDBData = (*pDBCollection)[nIndex];
                ScSubTotalParam aSubTotalParam;
                pDBData->GetSubTotalParam(aSubTotalParam);
                if (aSubTotalParam.bDoSort)
                {
                    if (!aSubTotalParam.bAscending)
                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_order, sXML_descending);
                    if (aSubTotalParam.bUserDef)
                    {
                        rtl::OUString sUserList = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_USERLIST));
                        sUserList += rtl::OUString::valueOf(aSubTotalParam.nUserIndex);
                        AddAttribute(XML_NAMESPACE_TABLE, sXML_data_type, sUserList);
                    }
                    SvXMLElementExport aElemSGs(*this, XML_NAMESPACE_TABLE, sXML_sort_groups, sal_True, sal_True);
                    CheckAttrList();
                }
            }
            for (sal_Int32 i = 0; i < nSubTotalFields; i++)
            {
                uno::Reference <sheet::XSubTotalField> xSubTotalField;
                uno::Any aSubTotalField = xIndexAccess->getByIndex(i);
                if (aSubTotalField >>= xSubTotalField)
                {
                    sal_Int32 nGroupColumn = xSubTotalField->getGroupColumn();
                    AddAttribute(XML_NAMESPACE_TABLE, sXML_group_by_field_number, rtl::OUString::valueOf(nGroupColumn));
                    SvXMLElementExport aElemSTR(*this, XML_NAMESPACE_TABLE, sXML_subtotal_rule, sal_True, sal_True);
                    CheckAttrList();
                    uno::Sequence <sheet::SubTotalColumn> aSubTotalColumns = xSubTotalField->getSubTotalColumns();
                    sal_Int32 nSubTotalColumns = aSubTotalColumns.getLength();
                    for (sal_Int32 j = 0; j < nSubTotalColumns; j++)
                    {
                        AddAttribute(XML_NAMESPACE_TABLE, sXML_field_number, rtl::OUString::valueOf(aSubTotalColumns[j].Column));
                        rtl::OUString sFunction;
                        ScXMLConverter::GetStringFromFunction( sFunction, aSubTotalColumns[j].Function );
                        AddAttribute(XML_NAMESPACE_TABLE, sXML_function, sFunction);
                        SvXMLElementExport aElemSTF(*this, XML_NAMESPACE_TABLE, sXML_subtotal_field, sal_True, sal_True);
                        CheckAttrList();
                    }
                }
            }
        }
    }
}

void ScXMLExport::WriteDatabaseRanges(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    uno::Reference <beans::XPropertySet> xPropertySet (xSpreadDoc, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        uno::Any aDatabaseRanges = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNO_DATABASERNG)));
        uno::Reference <sheet::XDatabaseRanges> xDatabaseRanges;
        CheckAttrList();
        if (aDatabaseRanges >>= xDatabaseRanges)
        {
            uno::Sequence <rtl::OUString> aRanges = xDatabaseRanges->getElementNames();
            sal_Int32 nDatabaseRangesCount = aRanges.getLength();
            if (nDatabaseRangesCount > 0)
            {
                SvXMLElementExport aElemDRs(*this, XML_NAMESPACE_TABLE, sXML_database_ranges, sal_True, sal_True);
                for (sal_Int32 i = 0; i < nDatabaseRangesCount; i++)
                {
                    rtl::OUString sDatabaseRangeName = aRanges[i];
                    uno::Any aDatabaseRange = xDatabaseRanges->getByName(sDatabaseRangeName);
                    uno::Reference <sheet::XDatabaseRange> xDatabaseRange;
                    if (aDatabaseRange >>= xDatabaseRange)
                    {
                        String sUnbenannt = ScGlobal::GetRscString(STR_DB_NONAME);
                        rtl::OUString sOUUnbenannt (sUnbenannt);
                        if (sOUUnbenannt != sDatabaseRangeName)
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_name, sDatabaseRangeName);
                        table::CellRangeAddress aRangeAddress = xDatabaseRange->getDataArea();
                        rtl::OUString sOUAddress;
                        ScXMLConverter::GetStringFromRange( sOUAddress, aRangeAddress, pDoc );
                        AddAttribute (XML_NAMESPACE_TABLE, sXML_target_range_address, sOUAddress);
                        ScDBCollection* pDBCollection = pDoc->GetDBCollection();
                        sal_uInt16 nIndex;
                        pDBCollection->SearchName(sDatabaseRangeName, nIndex);
                        ScDBData* pDBData = (*pDBCollection)[nIndex];
                        if (pDBData->HasImportSelection())
                            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_is_selection, sXML_true);
                        if (pDBData->HasAutoFilter())
                            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_display_filter_buttons, sXML_true);
                        uno::Reference <beans::XPropertySet> xPropertySetDatabaseRange (xDatabaseRange, uno::UNO_QUERY);
                        if (xPropertySetDatabaseRange.is())
                        {
                            uno::Any aKeepFormatsProperty = xPropertySetDatabaseRange->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_KEEPFORM)));
                            sal_Bool bKeepFormats = sal_False;
                            if (aKeepFormatsProperty >>= bKeepFormats)
                                if (bKeepFormats)
                                    AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_on_update_keep_styles, sXML_true);
                            uno::Any aMoveCellsProperty = xPropertySetDatabaseRange->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_MOVCELLS)));
                            sal_Bool bMoveCells = sal_False;
                            if (aMoveCellsProperty >>= bMoveCells)
                                if (bMoveCells)
                                    AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_on_update_keep_size, sXML_false);
                            uno::Any aStripDataProperty = xPropertySetDatabaseRange->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_STRIPDAT)));
                            sal_Bool bStripData = sal_False;
                            if (aStripDataProperty >>= bStripData)
                                if (bStripData)
                                    AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_has_persistent_data, sXML_false);
                        }
                        uno::Reference <sheet::XSheetFilterDescriptor> xSheetFilterDescriptor = xDatabaseRange->getFilterDescriptor();
                        if (xSheetFilterDescriptor.is())
                        {
                            uno::Reference <beans::XPropertySet> xFilterProperties (xSheetFilterDescriptor, uno::UNO_QUERY);
                            if (xFilterProperties.is())
                            {
                                uno::Any aContainsHeaderProperty = xFilterProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_CONTHDR)));
                                sal_Bool bContainsHeader = sal_True;
                                if (aContainsHeaderProperty >>= bContainsHeader)
                                    if (!bContainsHeader)
                                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_contains_header, sXML_false);
                                uno::Any aOrientationProperty = xFilterProperties->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ORIENT)));
                                sal_Bool bOrientation = sal_False;
                                if (aOrientationProperty >>= bOrientation)
                                    if (bOrientation)
                                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_orientation, sXML_column);
                            }
                        }
                        SvXMLElementExport aElemDR(*this, XML_NAMESPACE_TABLE, sXML_database_range, sal_True, sal_True);
                        CheckAttrList();
                        WriteImportDescriptor(xDatabaseRange->getImportDescriptor());
                        if (xSheetFilterDescriptor.is())
                            WriteFilterDescriptor(xSheetFilterDescriptor, sDatabaseRangeName);
                        WriteSortDescriptor(xDatabaseRange->getSortDescriptor());
                        WriteSubTotalDescriptor(xDatabaseRange->getSubTotalDescriptor(), sDatabaseRangeName);
                    }
                }
            }
        }
    }

}

rtl::OUString ScXMLExport::getDPOperatorXML(const ScQueryOp aFilterOperator, const sal_Bool bUseRegularExpressions,
    const sal_Bool bIsString, const double dVal) const
{
    if (bUseRegularExpressions)
    {
        switch (aFilterOperator)
        {
            case SC_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_match));
                break;
            case SC_NOT_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_nomatch));
                break;
        }
    }
    else
    {
        switch (aFilterOperator)
        {
            case SC_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
                break;
            case SC_NOT_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("!="));
                break;
            case SC_BOTPERC :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bottom_percent));
                break;
            case SC_BOTVAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_bottom_values));
                break;
            case SC_GREATER :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
                break;
            case SC_GREATER_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
                break;
            case SC_LESS :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
                break;
            case SC_LESS_EQUAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
                break;
            case SC_TOPPERC :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_top_percent));
                break;
            case SC_TOPVAL :
                return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_top_values));
                break;
            default:
            {
                if (bIsString)
                {
                    if (dVal == SC_EMPTYFIELDS)
                        return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_empty));
                    else if (dVal == SC_NONEMPTYFIELDS)
                        return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_noempty));
                }
            }
        }
    }
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("="));
}

void ScXMLExport::WriteDPCondition(const ScQueryEntry& aQueryEntry, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions)
{
    AddAttribute(XML_NAMESPACE_TABLE, sXML_field_number, rtl::OUString::valueOf(aQueryEntry.nField));
    if (bIsCaseSensitive)
        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_case_sensitive, sXML_true);
    if (aQueryEntry.bQueryByString)
    {
        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_data_type, sXML_number);
        rtl::OUStringBuffer sBuffer;
        GetMM100UnitConverter().convertNumber(sBuffer, aQueryEntry.nVal);
        AddAttribute(XML_NAMESPACE_TABLE, sXML_value, sBuffer.makeStringAndClear());
    }
    else
        AddAttribute(XML_NAMESPACE_TABLE, sXML_value, rtl::OUString(*aQueryEntry.pStr));
    AddAttribute(XML_NAMESPACE_TABLE, sXML_operator, getDPOperatorXML(aQueryEntry.eOp, bUseRegularExpressions,
        aQueryEntry.bQueryByString, aQueryEntry.nVal));
    SvXMLElementExport aElemC(*this, XML_NAMESPACE_TABLE, sXML_filter_condition, sal_True, sal_True);
}

void ScXMLExport::WriteDPFilter(const ScQueryParam& aQueryParam)
{
    sal_Int16 nQueryEntryCount = aQueryParam.GetEntryCount();
    if (nQueryEntryCount > 0)
    {
        sal_Bool bAnd(sal_False);
        sal_Bool bOr(sal_False);
        sal_Bool bHasEntries(sal_True);
        sal_Int16 nEntries(0);
        for (sal_Int32 j = 1; (j < nQueryEntryCount) && bHasEntries; j++)
        {
            ScQueryEntry aEntry = aQueryParam.GetEntry(j);
            if (aEntry.bDoQuery)
            {
                nEntries++;
                if (aEntry.eConnect == SC_AND)
                    bAnd = sal_True;
                else
                    bOr = sal_True;
            }
            else
                bHasEntries = sal_False;
        }
        nQueryEntryCount = nEntries;
        if (nQueryEntryCount)
        {
            if (!aQueryParam.bInplace)
            {
                ScAddress aTargetAddress(aQueryParam.nDestCol, aQueryParam.nDestRow, aQueryParam.nDestTab);
                OUString sAddress;
                ScXMLConverter::GetStringFromAddress( sAddress, aTargetAddress, pDoc );
                AddAttribute(XML_NAMESPACE_TABLE, sXML_target_range_address, sAddress);
            }
            if(!((aQueryParam.nCol1 == aQueryParam.nCol2) && (aQueryParam.nRow1 == aQueryParam.nRow2) && (aQueryParam.nCol1 == aQueryParam.nRow1)
                && (aQueryParam.nCol1 == 0) && (aQueryParam.nTab == USHRT_MAX)))
            {
                ScRange aConditionRange(aQueryParam.nCol1, aQueryParam.nRow1, aQueryParam.nTab,
                    aQueryParam.nCol2, aQueryParam.nRow2, aQueryParam.nTab);
                rtl::OUString sConditionRange;
                ScXMLConverter::GetStringFromRange( sConditionRange, aConditionRange, pDoc );
                AddAttribute(XML_NAMESPACE_TABLE, sXML_condition_source_range_address, sConditionRange);
            }
            if (!aQueryParam.bDuplicate)
                AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_display_duplicates, sXML_false);
            SvXMLElementExport aElemDPF(*this, XML_NAMESPACE_TABLE, sXML_filter, sal_True, sal_True);
            CheckAttrList();
            if (bOr && !bAnd)
            {
                SvXMLElementExport aElemOr(*this, XML_NAMESPACE_TABLE, sXML_filter_or, sal_True, sal_True);
                for (j = 0; j < nQueryEntryCount; j++)
                {
                    WriteDPCondition(aQueryParam.GetEntry(j), aQueryParam.bCaseSens, aQueryParam.bRegExp);
                }
            }
            else if (bAnd && !bOr)
            {
                SvXMLElementExport aElemAnd(*this, XML_NAMESPACE_TABLE, sXML_filter_and, sal_True, sal_True);
                for (j = 0; j < nQueryEntryCount; j++)
                {
                    WriteDPCondition(aQueryParam.GetEntry(j), aQueryParam.bCaseSens, aQueryParam.bRegExp);
                }
            }
            else if (nQueryEntryCount  == 1)
            {
                    WriteDPCondition(aQueryParam.GetEntry(0), aQueryParam.bCaseSens, aQueryParam.bRegExp);
            }
            else
            {
                SvXMLElementExport aElemC(*this, XML_NAMESPACE_TABLE, sXML_filter_or, sal_True, sal_True);
                ScQueryEntry aPrevFilterField = aQueryParam.GetEntry(0);
                ScQueryConnect aConnection = aQueryParam.GetEntry(1).eConnect;
                sal_Bool bOpenAndElement;
                rtl::OUString aName = GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_filter_and)));
                if (aConnection == SC_AND)
                {
                    GetDocHandler()->ignorableWhitespace(sWS);
                    GetDocHandler()->startElement( aName, GetXAttrList());
                    ClearAttrList();
                    bOpenAndElement = sal_True;
                }
                else
                    bOpenAndElement = sal_False;
                for (j = 1; j < nQueryEntryCount; j++)
                {
                    if (aConnection != aQueryParam.GetEntry(j).eConnect)
                    {
                        aConnection = aQueryParam.GetEntry(j).eConnect;
                        if (aQueryParam.GetEntry(j).eConnect == SC_AND)
                        {
                            GetDocHandler()->ignorableWhitespace(sWS);
                            GetDocHandler()->startElement( aName, GetXAttrList());
                            ClearAttrList();
                            bOpenAndElement = sal_True;
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            aPrevFilterField = aQueryParam.GetEntry(j);
                            if (j == nQueryEntryCount - 1)
                            {
                                WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                                GetDocHandler()->ignorableWhitespace(sWS);
                                GetDocHandler()->endElement(aName);
                                bOpenAndElement = sal_False;
                            }
                        }
                        else
                        {
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            aPrevFilterField = aQueryParam.GetEntry(j);
                            if (bOpenAndElement)
                            {
                                GetDocHandler()->ignorableWhitespace(sWS);
                                GetDocHandler()->endElement(aName);
                                bOpenAndElement = sal_False;
                            }
                            if (j == nQueryEntryCount - 1)
                            {
                                WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                            }
                        }
                    }
                    else
                    {
                        WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                        aPrevFilterField = aQueryParam.GetEntry(j);
                        if (j == nQueryEntryCount - 1)
                            WriteDPCondition(aPrevFilterField, aQueryParam.bCaseSens, aQueryParam.bRegExp);
                    }
                }
            }
        }
    }
}

void ScXMLExport::WriteDataPilots(const uno::Reference <sheet::XSpreadsheetDocument>& xSpreadDoc)
{
    if (pDoc)
    {
        ScDPCollection* pDPs = pDoc->GetDPCollection();
        if (pDPs)
        {
            sal_Int16 nDPCount = pDPs->GetCount();
            if (nDPCount > 0)
            {
                SvXMLElementExport aElemDPs(*this, XML_NAMESPACE_TABLE, sXML_data_pilot_tables, sal_True, sal_True);
                CheckAttrList();
                for (sal_Int16 i = 0; i < nDPCount; i++)
                {
                    ScDPSaveData* pDPSave = (*pDPs)[i]->GetSaveData();
                    if (pDPSave)
                    {
                        ScRange aOutRange = (*pDPs)[i]->GetOutRange();
                        rtl::OUString sTargetRangeAddress;
                        ScXMLConverter::GetStringFromRange( sTargetRangeAddress, aOutRange, pDoc );
                        ScDocAttrIterator aAttrItr(pDoc, aOutRange.aStart.Tab(),
                            aOutRange.aStart.Col(), aOutRange.aStart.Row(),
                            aOutRange.aEnd.Col(), aOutRange.aEnd.Row());
                        sal_uInt16 nCol, nRow1, nRow2;
                        OUString sOUButtonList;
                        const ScPatternAttr* pAttr = aAttrItr.GetNext(nCol, nRow1, nRow2);
                        while (pAttr)
                        {
                            ScMergeFlagAttr& rItem = (ScMergeFlagAttr&)pAttr->GetItem(ATTR_MERGE_FLAG);
                            if (rItem.HasButton())
                            {
                                for (sal_Int32 nButtonRow = nRow1; nButtonRow <= nRow2; nButtonRow++)
                                {
                                    ScAddress aButtonAddr(nCol, nButtonRow, aOutRange.aStart.Tab());
                                    ScXMLConverter::GetStringFromAddress(
                                        sOUButtonList, aButtonAddr, pDoc, sal_True );
                                }
                            }
                            pAttr = aAttrItr.GetNext(nCol, nRow1, nRow2);
                        }
                        rtl::OUString sName((*pDPs)[i]->GetName());
                        rtl::OUString sApplicationData((*pDPs)[i]->GetTag());
                        sal_Bool bRowGrand = pDPSave->GetRowGrand();
                        sal_Bool bColumnGrand = pDPSave->GetColumnGrand();
                        AddAttribute(XML_NAMESPACE_TABLE, sXML_name, sName);
                        AddAttribute(XML_NAMESPACE_TABLE, sXML_application_data, sApplicationData);
                        AddAttribute(XML_NAMESPACE_TABLE, sXML_target_range_address, sTargetRangeAddress);
                        AddAttribute(XML_NAMESPACE_TABLE, sXML_buttons, sOUButtonList);
                        if (!(bRowGrand && bColumnGrand))
                        {
                            if (bRowGrand)
                                AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_grand_total, sXML_row);
                            else if (bColumnGrand)
                                AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_grand_total, sXML_column);
                            else
                                AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_grand_total, sXML_none);
                        }
                        if (pDPSave->GetIgnoreEmptyRows())
                            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_ignore_empty_rows, sXML_true);
                        if (pDPSave->GetRepeatIfEmpty())
                            AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_identify_categories, sXML_true);
                        SvXMLElementExport aElemDP(*this, XML_NAMESPACE_TABLE, sXML_data_pilot_table, sal_True, sal_True);
                        CheckAttrList();
                        if ((*pDPs)[i]->IsSheetData())
                        {
                            const ScSheetSourceDesc* pSheetSource = (*pDPs)[i]->GetSheetDesc();
                            rtl::OUString sCellRangeAddress;
                            ScXMLConverter::GetStringFromRange( sCellRangeAddress, pSheetSource->aSourceRange, pDoc );
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_cell_range_address, sCellRangeAddress);
                            SvXMLElementExport aElemSCR(*this, XML_NAMESPACE_TABLE, sXML_source_cell_range, sal_True, sal_True);
                            CheckAttrList();
                            WriteDPFilter(pSheetSource->aQueryParam);
                        }
                        else if ((*pDPs)[i]->IsImportData())
                        {
                            const ScImportSourceDesc* pImpSource = (*pDPs)[i]->GetImportSourceDesc();
                            switch (pImpSource->nType)
                            {
                                case sheet::DataImportMode_NONE : break;
                                case sheet::DataImportMode_QUERY :
                                {
                                    AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, rtl::OUString(pImpSource->aDBName));
                                    AddAttribute(XML_NAMESPACE_TABLE, sXML_query_name, rtl::OUString(pImpSource->aObject));
                                    SvXMLElementExport aElemID(*this, XML_NAMESPACE_TABLE, sXML_database_source_query, sal_True, sal_True);
                                    CheckAttrList();
                                }
                                break;
                                case sheet::DataImportMode_TABLE :
                                {
                                    AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, rtl::OUString(pImpSource->aDBName));
                                    AddAttribute(XML_NAMESPACE_TABLE, sXML_table_name, rtl::OUString(pImpSource->aObject));
                                    SvXMLElementExport aElemID(*this, XML_NAMESPACE_TABLE, sXML_database_source_table, sal_True, sal_True);
                                    CheckAttrList();
                                }
                                break;
                                case sheet::DataImportMode_SQL :
                                {
                                    AddAttribute(XML_NAMESPACE_TABLE, sXML_database_name, rtl::OUString(pImpSource->aDBName));
                                    AddAttribute(XML_NAMESPACE_TABLE, sXML_sql_statement, rtl::OUString(pImpSource->aObject));
                                    if (!pImpSource->bNative)
                                        AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_parse_sql_statement, sXML_true);
                                    SvXMLElementExport aElemID(*this, XML_NAMESPACE_TABLE, sXML_database_source_sql, sal_True, sal_True);
                                    CheckAttrList();
                                }
                                break;
                            }
                        }
                        else if ((*pDPs)[i]->IsServiceData())
                        {
                            const ScDPServiceDesc* pServSource = (*pDPs)[i]->GetDPServiceDesc();
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_name, rtl::OUString(pServSource->aServiceName));
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_source_name, rtl::OUString(pServSource->aParSource));
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_object_name, rtl::OUString(pServSource->aParName));
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_username, rtl::OUString(pServSource->aParUser));
                            // How to write the Passwort? We must know, whether the passwort shoulb be written encrypted and how or not
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_passwort, rtl::OUString(pServSource->aParPass));
                            SvXMLElementExport aElemSD(*this, XML_NAMESPACE_TABLE, sXML_source_service, sal_True, sal_True);
                            CheckAttrList();
                        }
                        List aDimensions = pDPSave->GetDimensions();
                        sal_Int32 nDimCount = aDimensions.Count();
                        for (sal_Int32 nDim = 0; nDim < nDimCount; nDim++)
                        {
                            ScDPSaveDimension* pDim = (ScDPSaveDimension*)aDimensions.GetObject(nDim);
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_source_field_name, rtl::OUString(pDim->GetName()));
                            if (pDim->IsDataLayout())
                                AddAttributeASCII(XML_NAMESPACE_TABLE, sXML_is_data_layout_field, sXML_true);
                            OUString sValueStr;
                            ScXMLConverter::GetStringFromOrientation( sValueStr,
                                (sheet::DataPilotFieldOrientation) pDim->GetOrientation() );
                            if( sValueStr.getLength() )
                                AddAttribute(XML_NAMESPACE_TABLE, sXML_orientation, sValueStr );
                            if (pDim->GetUsedHierarchy() != 1)
                            {
                                rtl::OUStringBuffer sBuffer;
                                SvXMLUnitConverter::convertNumber(sBuffer, pDim->GetUsedHierarchy());
                                AddAttribute(XML_NAMESPACE_TABLE, sXML_used_hierarchy, sBuffer.makeStringAndClear());
                            }
                            ScXMLConverter::GetStringFromFunction( sValueStr,
                                (sheet::GeneralFunction) pDim->GetFunction() );
                            AddAttribute(XML_NAMESPACE_TABLE, sXML_function, sValueStr);
                            SvXMLElementExport aElemDPF(*this, XML_NAMESPACE_TABLE, sXML_data_pilot_field, sal_True, sal_True);
                            CheckAttrList();
                            {
                                rtl::OUStringBuffer sBuffer;
                                SvXMLUnitConverter::convertBool(sBuffer, pDim->GetShowEmpty());
                                AddAttribute(XML_NAMESPACE_TABLE, sXML_display_empty, sBuffer.makeStringAndClear());
                                SvXMLElementExport aElemDPL(*this, XML_NAMESPACE_TABLE, sXML_data_pilot_level, sal_True, sal_True);
                                CheckAttrList();
                                sal_Int32 nSubTotalCount = pDim->GetSubTotalsCount();
                                if (nSubTotalCount > 0)
                                {
                                    SvXMLElementExport aElemSTs(*this, XML_NAMESPACE_TABLE, sXML_data_pilot_subtotals, sal_True, sal_True);
                                    CheckAttrList();
                                    for (sal_Int32 nSubTotal = 0; nSubTotal < nSubTotalCount; nSubTotal++)
                                    {
                                        rtl::OUString sFunction;
                                        ScXMLConverter::GetStringFromFunction( sFunction, (sheet::GeneralFunction)pDim->GetSubTotalFunc(nSubTotal) );
                                        AddAttribute(XML_NAMESPACE_TABLE, sXML_function, sFunction);
                                        SvXMLElementExport aElemST(*this, XML_NAMESPACE_TABLE, sXML_data_pilot_subtotal, sal_True, sal_True);
                                    }
                                }
                                List aMembers = pDim->GetMembers();
                                sal_Int32 nMemberCount = aMembers.Count();
                                if (nMemberCount > 0)
                                {
                                    SvXMLElementExport aElemDPMs(*this, XML_NAMESPACE_TABLE, sXML_data_pilot_members, sal_True, sal_True);
                                    CheckAttrList();
                                    for (sal_Int32 nMember = 0; nMember < nMemberCount; nMember++)
                                    {
                                        AddAttribute(XML_NAMESPACE_TABLE, sXML_name, rtl::OUString(((ScDPSaveMember*)aMembers.GetObject(nMember))->GetName()));
                                        rtl::OUStringBuffer sBuffer;
                                        SvXMLUnitConverter::convertBool(sBuffer, ((ScDPSaveMember*)aMembers.GetObject(nMember))->GetIsVisible());
                                        AddAttribute(XML_NAMESPACE_TABLE, sXML_display, sBuffer.makeStringAndClear());
                                        SvXMLUnitConverter::convertBool(sBuffer, ((ScDPSaveMember*)aMembers.GetObject(nMember))->GetShowDetails());
                                        AddAttribute(XML_NAMESPACE_TABLE, sXML_display_details, sBuffer.makeStringAndClear());
                                        SvXMLElementExport aElemDPM(*this, XML_NAMESPACE_TABLE, sXML_data_pilot_member, sal_True, sal_True);                                        CheckAttrList();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// core implementation
void ScXMLExport::WriteConsolidation()
{
    const ScConsolidateParam* pCons = pDoc->GetConsolidateDlgData();
    if( pCons )
    {
        OUString sStrData;

        ScXMLConverter::GetStringFromFunction( sStrData, pCons->eFunction );
        AddAttribute( XML_NAMESPACE_TABLE, sXML_function, sStrData );

        sStrData = OUString();
        for( sal_Int32 nIndex = 0; nIndex < pCons->nDataAreaCount; nIndex++ )
            ScXMLConverter::GetStringFromArea( sStrData, *pCons->ppDataAreas[ nIndex ], pDoc, sal_True );
        AddAttribute( XML_NAMESPACE_TABLE, sXML_source_cell_range_addresses, sStrData );

        ScXMLConverter::GetStringFromAddress( sStrData, ScAddress( pCons->nCol, pCons->nRow, pCons->nTab ), pDoc );
        AddAttribute( XML_NAMESPACE_TABLE, sXML_target_cell_address, sStrData );

        if( pCons->bByCol && !pCons->bByRow )
            AddAttributeASCII( XML_NAMESPACE_TABLE, sXML_use_label, sXML_column );
        else if( !pCons->bByCol && pCons->bByRow )
            AddAttributeASCII( XML_NAMESPACE_TABLE, sXML_use_label, sXML_row );
        else if( pCons->bByCol && pCons->bByRow )
            AddAttributeASCII( XML_NAMESPACE_TABLE, sXML_use_label, sXML_both );

        if( pCons->bReferenceData )
            AddAttributeASCII( XML_NAMESPACE_TABLE, sXML_link_to_source_data, sXML_true );

        SvXMLElementExport aElem( *this, XML_NAMESPACE_TABLE, sXML_consolidation, sal_True, sal_True );
    }
}

SvXMLAutoStylePoolP* ScXMLExport::CreateAutoStylePool()
{
    return new ScXMLAutoStylePoolP(*this);
}

XMLPageExport* ScXMLExport::CreatePageExport()
{
    return new XMLTableMasterPageExport( *this );
}

