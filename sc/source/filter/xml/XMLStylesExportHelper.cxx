/*************************************************************************
 *
 *  $RCSfile: XMLStylesExportHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-15 12:26:19 $
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

#ifndef _SC_XMLSTYLESEXPORTHELPER_HXX
#include "XMLStylesExportHelper.hxx"
#endif

#include "global.hxx"

#include <algorithm>

using namespace com::sun::star;

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

