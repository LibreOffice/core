/*************************************************************************
 *
 *  $RCSfile: XMLStylesImportHelper.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: sab $ $Date: 2001-07-23 15:24:06 $
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

#ifndef _SC_XMLSTYLESIMPORTHELPER_HXX
#include "XMLStylesImportHelper.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif

using namespace com::sun::star;

void ScMyStyleNumberFormats::AddStyleNumberFormat(const rtl::OUString& rStyleName, const sal_Int32 nNumberFormat)
{
    ScMyStyleNumberFormat aFormat(rStyleName, nNumberFormat);
    aSet.insert(aFormat);
}

sal_Int32 ScMyStyleNumberFormats::GetStyleNumberFormat(const rtl::OUString& rStyleName)
{
    ScMyStyleNumberFormat aStyleNumberFormat(rStyleName);
    ScMyStyleNumberFormatSet::iterator aItr = aSet.find(aStyleNumberFormat);
    if (aItr == aSet.end())
        return -1;
    else
        return aItr->nNumberFormat;
}

ScMyStyleRanges::ScMyStyleRanges()
    :
    pTextList(NULL),
    pNumberList(NULL),
    pTimeList(NULL),
    pDateTimeList(NULL),
    pPercentList(NULL),
    pLogicalList(NULL),
    pCurrencyList(NULL)
{
}

ScMyStyleRanges::~ScMyStyleRanges()
{
    if (pTextList)
        delete pTextList;
    if (pNumberList)
        delete pNumberList;
    if (pTimeList)
        delete pTimeList;
    if (pDateTimeList)
        delete pDateTimeList;
    if (pPercentList)
        delete pPercentList;
    if (pLogicalList)
        delete pLogicalList;
    if (pCurrencyList)
        delete pCurrencyList;
}

void ScMyStyleRanges::AddRange(const ScRange& rRange, ScRangeList* pList,
    const rtl::OUString& rStyleName, const sal_Int16 nType,
    ScXMLImport& rImport, const sal_uInt32 nMaxRanges)
{
    pList->Join(rRange);
    DBG_ASSERT(nMaxRanges > 0, "MaxRanges to less");
    if (pList->Count() > nMaxRanges)
    {
        sal_Int32 nCount(pList->Count());
        ScRange* pRange = NULL;
        rtl::OUString sEmpty;
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            pRange = pList->GetObject(i);
            if (pRange && (pRange->aEnd.Row() + 1 < rRange.aStart.Row()))
            {
                rImport.SetStyleToRange(*pRange, rStyleName, nType, sEmpty);
                delete pRange;
                pRange = NULL;
                pList->Remove(i);
            }
        }
    }
}

void ScMyStyleRanges::AddCurrencyRange(const ScRange& rRange, ScRangeListRef xList,
    const rtl::OUString& rStyleName, const rtl::OUString& rCurrency,
    ScXMLImport& rImport, const sal_uInt32 nMaxRanges)
{
    xList->Join(rRange);
    DBG_ASSERT(nMaxRanges > 0, "MaxRanges to less");
    if (xList->Count() > nMaxRanges)
    {
        sal_Int32 nCount(xList->Count());
        ScRange* pRange = NULL;
        rtl::OUString sEmpty;
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            pRange = xList->GetObject(i);
            if (pRange && (pRange->aEnd.Row() + 1 < rRange.aStart.Row()))
            {
                rImport.SetStyleToRange(*pRange, rStyleName, util::NumberFormat::CURRENCY, rCurrency);
                delete pRange;
                pRange = NULL;
                xList->Remove(i);
            }
        }
    }
}

void ScMyStyleRanges::AddRange(const ScRange& rRange,
    const rtl::OUString& rStyleName, const sal_Int16 nType,
    ScXMLImport& rImport, const sal_uInt32 nMaxRanges)
{
    switch (nType)
    {
        case util::NumberFormat::NUMBER:
        {
            if (!pNumberList)
                pNumberList = new ScRangeList();
            AddRange(rRange, pNumberList, rStyleName, nType, rImport, nMaxRanges);
        }
        break;
        case util::NumberFormat::TEXT:
        {
            if (!pTextList)
                pTextList = new ScRangeList();
            AddRange(rRange, pTextList, rStyleName, nType, rImport, nMaxRanges);
        }
        break;
        case util::NumberFormat::TIME:
        {
            if (!pTimeList)
                pTimeList = new ScRangeList();
            AddRange(rRange, pTimeList, rStyleName, nType, rImport, nMaxRanges);
        }
        break;
        case util::NumberFormat::DATETIME:
        {
            if (!pDateTimeList)
                pDateTimeList = new ScRangeList();
            AddRange(rRange, pDateTimeList, rStyleName, nType, rImport, nMaxRanges);
        }
        break;
        case util::NumberFormat::PERCENT:
        {
            if (!pPercentList)
                pPercentList = new ScRangeList();
            AddRange(rRange, pPercentList, rStyleName, nType, rImport, nMaxRanges);
        }
        break;
        case util::NumberFormat::LOGICAL:
        {
            if (!pLogicalList)
                pLogicalList = new ScRangeList();
            AddRange(rRange, pLogicalList, rStyleName, nType, rImport, nMaxRanges);
        }
        break;
        default:
        {
            DBG_ERROR("wrong type");
        }
        break;
    }
}

void ScMyStyleRanges::AddCurrencyRange(const ScRange& rRange,
    const rtl::OUString& rStyleName, const rtl::OUString& rCurrency,
    ScXMLImport& rImport, const sal_uInt32 nMaxRanges)
{
    if (!pCurrencyList)
        pCurrencyList = new ScMyCurrencyStylesSet();
    ScMyCurrencyStyle aStyle;
    aStyle.sCurrency = rCurrency;
    ScMyCurrencyStylesSet::iterator aItr = pCurrencyList->find(aStyle);
    if (aItr == pCurrencyList->end())
    {
        std::pair<ScMyCurrencyStylesSet::iterator, bool> aPair = pCurrencyList->insert(aStyle);
        if (aPair.second)
        {
            aItr = aPair.first;
            AddCurrencyRange(rRange, aItr->xRanges, rStyleName, rCurrency, rImport, nMaxRanges);
        }
    }
    else
        aItr->xRanges->Join(rRange);
}

void ScMyStyleRanges::InsertColRow(const ScRange& rRange, const sal_Int16 nDx, const sal_Int16 nDy,
        const sal_Int16 nDz, ScDocument* pDoc)
{
    UpdateRefMode aRefMode = URM_INSDEL;
    if (pNumberList)
        pNumberList->UpdateReference(aRefMode, pDoc, rRange, nDx, nDy, nDz);
    if (pTextList)
        pTextList->UpdateReference(aRefMode, pDoc, rRange, nDx, nDy, nDz);
    if (pTimeList)
        pTimeList->UpdateReference(aRefMode, pDoc, rRange, nDx, nDy, nDz);
    if (pDateTimeList)
        pDateTimeList->UpdateReference(aRefMode, pDoc, rRange, nDx, nDy, nDz);
    if (pPercentList)
        pPercentList->UpdateReference(aRefMode, pDoc, rRange, nDx, nDy, nDz);
    if (pLogicalList)
        pLogicalList->UpdateReference(aRefMode, pDoc, rRange, nDx, nDy, nDz);
    if (pCurrencyList)
    {
        ScMyCurrencyStylesSet::iterator aItr = pCurrencyList->begin();
        while (aItr != pCurrencyList->end())
        {
            aItr->xRanges->UpdateReference(aRefMode, pDoc, rRange, nDx, nDy, nDz);
            aItr++;
        }
    }
}

void ScMyStyleRanges::InsertRow(const sal_Int32 nRow, const sal_Int32 nTab, ScDocument* pDoc)
{
    InsertColRow(ScRange(0, static_cast<sal_uInt16>(nRow), static_cast<sal_uInt16>(nTab),
        MAXCOL, MAXROW, static_cast<sal_uInt16>(nTab)), 0, 1, 0, pDoc);
}

void ScMyStyleRanges::InsertCol(const sal_Int32 nCol, const sal_Int32 nTab, ScDocument* pDoc)
{
    InsertColRow(ScRange(static_cast<sal_uInt16>(nCol), 0, static_cast<sal_uInt16>(nTab),
        MAXCOL, MAXROW, static_cast<sal_uInt16>(nTab)), 1, 0, 0, pDoc);
}

void ScMyStyleRanges::SetStylesToRanges(ScRangeList* pList,
    const rtl::OUString& rStyleName, const sal_Int16 nCellType,
    const rtl::OUString& rCurrency, ScXMLImport& rImport)
{
    sal_Int32 nCount(pList->Count());
    for (sal_Int32 i = 0; i < nCount; i++)
        rImport.SetStyleToRange(*pList->GetObject(i), rStyleName, nCellType, rCurrency);
}

void ScMyStyleRanges::SetStylesToRanges(ScRangeListRef xList,
    const rtl::OUString& rStyleName, const sal_Int16 nCellType,
    const rtl::OUString& rCurrency, ScXMLImport& rImport)
{
    sal_Int32 nCount(xList->Count());
    for (sal_Int32 i = 0; i < nCount; i++)
        rImport.SetStyleToRange(*xList->GetObject(i), rStyleName, nCellType, rCurrency);
}

void ScMyStyleRanges::SetStylesToRanges(const rtl::OUString& rStyleName, ScXMLImport& rImport)
{
    rtl::OUString sEmpty;
    if (pNumberList)
        SetStylesToRanges(pNumberList, rStyleName, util::NumberFormat::NUMBER, sEmpty, rImport);
    if (pTextList)
        SetStylesToRanges(pTextList, rStyleName, util::NumberFormat::TEXT, sEmpty, rImport);
    if (pTimeList)
        SetStylesToRanges(pTimeList, rStyleName, util::NumberFormat::TIME, sEmpty, rImport);
    if (pDateTimeList)
        SetStylesToRanges(pDateTimeList, rStyleName, util::NumberFormat::DATETIME, sEmpty, rImport);
    if (pPercentList)
        SetStylesToRanges(pPercentList, rStyleName, util::NumberFormat::PERCENT, sEmpty, rImport);
    if (pLogicalList)
        SetStylesToRanges(pLogicalList, rStyleName, util::NumberFormat::LOGICAL, sEmpty, rImport);
    if (pCurrencyList)
    {
        ScMyCurrencyStylesSet::iterator aItr = pCurrencyList->begin();
        while (aItr != pCurrencyList->end())
        {
            SetStylesToRanges(aItr->xRanges, rStyleName, util::NumberFormat::CURRENCY, aItr->sCurrency, rImport);
            aItr++;
        }
    }
}

//----------------------------------------------------------------------------

ScMyStylesImportHelper::ScMyStylesImportHelper(ScXMLImport& rTempImport)
    :
    aCellStyles(),
    aColDefaultStyles(),
    bPrevRangeAdded(sal_True),
    rImport(rTempImport),
    nMaxRanges(0)
{
    aRowDefaultStyle = aCellStyles.end();
}

ScMyStylesImportHelper::~ScMyStylesImportHelper()
{
}

void ScMyStylesImportHelper::ResetAttributes()
{
    sPrevStyleName = sStyleName;
    sPrevCurrency = sCurrency;
    nPrevCellType = nCellType;
    sStyleName = sEmpty;
    sCurrency = sEmpty;
    nCellType = 0;
}

ScMyStylesSet::iterator ScMyStylesImportHelper::GetIterator(const rtl::OUString& sStyleName)
{
    ScMyStyle aStyle;
    aStyle.sStyleName = sStyleName;
    ScMyStylesSet::iterator aItr = aCellStyles.find(aStyle);
    if (aItr == aCellStyles.end())
    {
        std::pair<ScMyStylesSet::iterator, bool> aPair = aCellStyles.insert(aStyle);
        if (aPair.second)
            aItr = aPair.first;
        else
        {
            DBG_ERROR("not possible to insert style");
            return aCellStyles.end();
        }
    }
    return aItr;
}

void ScMyStylesImportHelper::AddDefaultRange(const ScRange& rRange)
{
    DBG_ASSERT(aRowDefaultStyle != aCellStyles.end(), "no row default style")
    if (!aRowDefaultStyle->sStyleName.getLength())
    {
        sal_uInt32 nStartCol(rRange.aStart.Col());
        sal_uInt32 nEndCol(rRange.aEnd.Col());
        ScMyStylesSet::iterator aPrevItr = aColDefaultStyles[nStartCol];
        DBG_ASSERT(aColDefaultStyles.size() > nEndCol, "to much columns");
        for (sal_uInt32 i = nStartCol + 1; (i <= nEndCol) && (i < aColDefaultStyles.size()); i++)
        {
            if (aPrevItr != aColDefaultStyles[i])
            {
                DBG_ASSERT(aPrevItr != aCellStyles.end(), "no column default style")
                ScRange aRange(rRange);
                aRange.aStart.SetCol(static_cast<sal_uInt16>(nStartCol));
                aRange.aEnd.SetCol(static_cast<sal_uInt16>(i - 1));
                sPrevStyleName = aPrevItr->sStyleName;
                AddSingleRange(aRange);
                nStartCol = i;
                aPrevItr = aColDefaultStyles[i];
            }
        }
        if (aPrevItr != aCellStyles.end())
        {
            ScRange aRange(rRange);
            aRange.aStart.SetCol(static_cast<sal_uInt16>(nStartCol));
            sPrevStyleName = aPrevItr->sStyleName;
            AddSingleRange(aRange);
        }
        else
            DBG_ERROR("no column default style");
    }
    else
    {
        sPrevStyleName = aRowDefaultStyle->sStyleName;
        AddSingleRange(rRange);
    }
}

void ScMyStylesImportHelper::AddSingleRange(const ScRange& rRange)
{
    if (nMaxRanges == 0)
        nMaxRanges = aColDefaultStyles.size();
    ScMyStylesSet::iterator aItr = GetIterator(sPrevStyleName);
    if (aItr != aCellStyles.end())
    {
        if (nPrevCellType != util::NumberFormat::CURRENCY)
            aItr->xRanges->AddRange(rRange, sPrevStyleName, nPrevCellType,
                rImport, nMaxRanges);
        else
            aItr->xRanges->AddCurrencyRange(rRange, sPrevStyleName, sPrevCurrency,
                rImport, nMaxRanges);
    }
}

void ScMyStylesImportHelper::AddRange()
{
    if (sPrevStyleName.getLength())
        AddSingleRange(aPrevRange);
    else
        AddDefaultRange(aPrevRange);
    ResetAttributes();
}

void ScMyStylesImportHelper::AddColumnStyle(const rtl::OUString& sStyleName, const sal_Int32 nColumn, const sal_Int32 nRepeat)
{
    DBG_ASSERT(static_cast<sal_uInt32>(nColumn) == aColDefaultStyles.size(), "some columns are absent");
    ScMyStylesSet::iterator aItr = GetIterator(sStyleName);
    DBG_ASSERT(aItr != aCellStyles.end(), "no column default style")
    for (sal_Int32 i = 0; i < nRepeat; i++)
        aColDefaultStyles.push_back(aItr);
}

void ScMyStylesImportHelper::SetRowStyle(const rtl::OUString& sStyleName)
{
    aRowDefaultStyle = GetIterator(sStyleName);
}

void ScMyStylesImportHelper::SetAttributes(const rtl::OUString& sStyleName,
    const rtl::OUString& sCurrency, const sal_Int16 nCellType)
{
    this->sStyleName = sStyleName;
    this->sCurrency = sCurrency;
    this->nCellType = nCellType;
}

void ScMyStylesImportHelper::AddRange(const ScRange& rRange)
{
    if (!bPrevRangeAdded)
    {
        sal_Bool bAddRange(sal_False);
        if (nCellType == nPrevCellType &&
            sStyleName.equals(sPrevStyleName) &&
            sCurrency.equals(sPrevCurrency))
        {
            if (rRange.aStart.Row() == aPrevRange.aStart.Row())
            {
                if (rRange.aEnd.Row() == aPrevRange.aEnd.Row())
                {
                    DBG_ASSERT(aPrevRange.aEnd.Col() + 1 == rRange.aStart.Col(), "something wents wrong");
                    aPrevRange.aEnd.SetCol(rRange.aEnd.Col());
                }
                else
                    bAddRange = sal_True;
            }
            else
            {
                if (rRange.aStart.Col() == aPrevRange.aStart.Col() &&
                    rRange.aEnd.Col() == aPrevRange.aEnd.Col())
                {
                    DBG_ASSERT(aPrevRange.aEnd.Row() + 1 == rRange.aStart.Row(), "something wents wrong");
                    aPrevRange.aEnd.SetRow(rRange.aEnd.Row());
                }
                else
                    bAddRange = sal_True;
            }
        }
        else
            bAddRange = sal_True;
        if (bAddRange)
        {
            AddRange();
            aPrevRange = rRange;
        }
    }
    else
    {
        aPrevRange = rRange;
        sPrevStyleName = sStyleName;
        sPrevCurrency = sCurrency;
        nPrevCellType = nCellType;
        bPrevRangeAdded = sal_False;
    }
}

void ScMyStylesImportHelper::AddRange(const com::sun::star::table::CellRangeAddress& rRange)
{
    ScRange aScRange( static_cast<USHORT>(rRange.StartColumn), static_cast<USHORT>(rRange.StartRow), rRange.Sheet,
        static_cast<USHORT>(rRange.EndColumn), static_cast<USHORT>(rRange.EndRow), rRange.Sheet );
    AddRange(aScRange);
}

void ScMyStylesImportHelper::AddCell(const ScAddress& rAddress)
{
    ScRange aScRange( rAddress, rAddress );
    AddRange(aScRange);
}

void ScMyStylesImportHelper::AddCell(const com::sun::star::table::CellAddress& rAddress)
{
    ScAddress aScAddress( static_cast<USHORT>(rAddress.Column), static_cast<USHORT>(rAddress.Row), rAddress.Sheet );
    ScRange aScRange( aScAddress, aScAddress );
    AddRange(aScRange);
}

void ScMyStylesImportHelper::InsertRow(const sal_Int32 nRow, const sal_Int32 nTab, ScDocument* pDoc)
{
    ScMyStylesSet::iterator aItr = aCellStyles.begin();
    while (aItr != aCellStyles.end())
    {
        aItr->xRanges->InsertRow(nRow, nTab, pDoc);
        aItr++;
    }
}

void ScMyStylesImportHelper::InsertCol(const sal_Int32 nCol, const sal_Int32 nTab, ScDocument* pDoc)
{
    ScMyStylesSet::iterator aItr = aCellStyles.begin();
    while (aItr != aCellStyles.end())
    {
        aItr->xRanges->InsertCol(nCol, nTab, pDoc);
        aItr++;
    }
}

void ScMyStylesImportHelper::EndTable()
{
    if (!bPrevRangeAdded)
    {
        AddRange();
        bPrevRangeAdded = sal_True;
    }
    nMaxRanges = 0;
}

void ScMyStylesImportHelper::SetStylesToRanges()
{
    ScMyStylesSet::iterator aItr = aCellStyles.begin();
    while (aItr != aCellStyles.end())
    {
        aItr->xRanges->SetStylesToRanges(aItr->sStyleName, rImport);
        aItr++;
    }
    aCellStyles.clear();
    aColDefaultStyles.clear();
    nMaxRanges = 0;
}

