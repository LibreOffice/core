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

#include <sal/config.h>
#include <sal/log.hxx>

#include <o3tl/safeint.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/charclass.hxx>

#include <dbdata.hxx>
#include <compiler.hxx>
#include <tokenarray.hxx>
#include <globalnames.hxx>
#include <refupdat.hxx>
#include <document.hxx>
#include <queryparam.hxx>
#include <queryentry.hxx>
#include <mid.h>
#include <globstr.hrc>
#include <scresid.hxx>
#include <subtotalparam.hxx>
#include <sortparam.hxx>
#include <dociter.hxx>
#include <brdcst.hxx>
#include <osl/diagnose.h>

#include <comphelper/stl_types.hxx>
#include <comphelper/propertyvalue.hxx>

#include <memory>
#include <utility>

using namespace com::sun::star;

constexpr int DBSETTING_PARAMS = 8;

ScDatabaseSettingItem::ScDatabaseSettingItem():
    SfxPoolItem(SCITEM_DATABASE_SETTING),
    mbHeaderRow(false),
    mbTotalRow(false),
    mbFirstCol(false),
    mbLastCol(false),
    mbStripedRows(false),
    mbStripedCols(false),
    mbShowFilters(false)
{
}

ScDatabaseSettingItem::ScDatabaseSettingItem(bool bHeaderRow, bool bTotalRow, bool bFirstCol,
                                             bool bLastCol, bool bStripedRows, bool bStripedCols,
                                             bool bShowFilters, const OUString& aStyleID)
    :
    SfxPoolItem(SCITEM_DATABASE_SETTING),
    mbHeaderRow(bHeaderRow),
    mbTotalRow(bTotalRow),
    mbFirstCol(bFirstCol),
    mbLastCol(bLastCol),
    mbStripedRows(bStripedRows),
    mbStripedCols(bStripedCols),
    mbShowFilters(bShowFilters),
    maStyleID(aStyleID)
{
}

ScDatabaseSettingItem::ScDatabaseSettingItem(const ScDatabaseSettingItem& rItem):
    SfxPoolItem(SCITEM_DATABASE_SETTING),
    mbHeaderRow(rItem.mbHeaderRow),
    mbTotalRow(rItem.mbTotalRow),
    mbFirstCol(rItem.mbFirstCol),
    mbLastCol(rItem.mbLastCol),
    mbStripedRows(rItem.mbStripedRows),
    mbStripedCols(rItem.mbStripedCols),
    mbShowFilters(rItem.mbShowFilters),
    maStyleID(rItem.maStyleID)
{
}

ScDatabaseSettingItem::~ScDatabaseSettingItem()
{
}

bool ScDatabaseSettingItem::QueryValue(uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case 0:
        {
            css::uno::Sequence<css::beans::PropertyValue> aSeq{
                comphelper::makePropertyValue(u"ContainsHeader"_ustr, mbHeaderRow),
                comphelper::makePropertyValue(u"TotalsRow"_ustr, mbTotalRow),
                comphelper::makePropertyValue(u"UseFirstColumnFormatting"_ustr, mbFirstCol),
                comphelper::makePropertyValue(u"UseLastColumnFormatting"_ustr, mbLastCol),
                comphelper::makePropertyValue(u"UseRowStripes"_ustr, mbStripedRows),
                comphelper::makePropertyValue(u"UseColStripes"_ustr, mbStripedCols),
                comphelper::makePropertyValue(u"AutoFilter"_ustr, mbShowFilters),
                comphelper::makePropertyValue(u"TableStyleName"_ustr, maStyleID)
            };
            assert(aSeq.getLength() == DBSETTING_PARAMS);
            rVal <<= aSeq;
            break;
        }
        case MID_1:
            rVal <<= mbHeaderRow;
            break;
        case MID_2:
            rVal <<= mbTotalRow;
            break;
        case MID_3:
            rVal <<= mbFirstCol;
            break;
        case MID_4:
            rVal <<= mbLastCol;
            break;
        case MID_5:
            rVal <<= mbStripedRows;
            break;
        case MID_6:
            rVal <<= mbStripedCols;
            break;
        case MID_7:
            rVal <<= mbShowFilters;
            break;
        case MID_8:
            rVal <<= maStyleID;
            break;
        default:
            OSL_FAIL("Wrong MemberID!");
            return false;
    }

    return true;
}

bool ScDatabaseSettingItem::PutValue(const uno::Any& rVal, sal_uInt8 nMemberId)
{
    bool bVal = false;
    bool bRet = false;

    nMemberId &= ~CONVERT_TWIPS;
    switch (nMemberId)
    {
        case 0:
        {
            css::uno::Sequence<css::beans::PropertyValue> aSeq;
            if ((rVal >>= aSeq) && (aSeq.getLength() == DBSETTING_PARAMS))
            {
                OUString sTmpID;
                bool bTmpHRow(false);
                bool bTmpTRow(false);
                bool bTmpFCol(false);
                bool bTmpLCol(false);
                bool bTmpSRows(false);
                bool bTmpSCols(false);
                bool bTmpFilt(false);

                bool bAllConverted(true);
                sal_Int16 nConvertedCount(0);
                for (const auto& rProp : aSeq)
                {
                    if (rProp.Name == u"ContainsHeader")
                    {
                        bAllConverted &= (rProp.Value >>= bTmpHRow);
                        ++nConvertedCount;
                    }
                    else if (rProp.Name == u"TotalsRow")
                    {
                        bAllConverted &= (rProp.Value >>= bTmpTRow);
                        ++nConvertedCount;
                    }
                    else if (rProp.Name == u"UseFirstColumnFormatting")
                    {
                        bAllConverted &= (rProp.Value >>= bTmpFCol);
                        ++nConvertedCount;
                    }
                    else if (rProp.Name == u"UseLastColumnFormatting")
                    {
                        bAllConverted &= (rProp.Value >>= bTmpLCol);
                        ++nConvertedCount;
                    }
                    else if (rProp.Name == u"UseRowStripes")
                    {
                        bAllConverted &= (rProp.Value >>= bTmpSRows);
                        ++nConvertedCount;
                    }
                    else if (rProp.Name == u"UseColStripes")
                    {
                        bAllConverted &= (rProp.Value >>= bTmpSCols);
                        ++nConvertedCount;
                    }
                    else if (rProp.Name == u"AutoFilter")
                    {
                        bAllConverted &= (rProp.Value >>= bTmpFilt);
                        ++nConvertedCount;
                    }
                    else if (rProp.Name == u"TableStyleName")
                    {
                        bAllConverted &= (rProp.Value >>= sTmpID);
                        ++nConvertedCount;
                    }
                }

                if (bAllConverted && nConvertedCount == DBSETTING_PARAMS)
                {
                    mbHeaderRow = bTmpHRow;
                    mbTotalRow = bTmpTRow;
                    mbFirstCol = bTmpFCol;
                    mbLastCol = bTmpLCol;
                    mbStripedRows = bTmpSRows;
                    mbStripedCols = bTmpSCols;
                    mbShowFilters = bTmpFilt;
                    maStyleID = sTmpID;
                    return true;
                }
            }
            return false;
        }
        case MID_1:
            bRet = (rVal >>= bVal); if (bRet) mbHeaderRow=bVal; break;
            break;
        case MID_2:
            bRet = (rVal >>= bVal); if (bRet) mbTotalRow=bVal; break;
            break;
        case MID_3:
            bRet = (rVal >>= bVal); if (bRet) mbFirstCol=bVal; break;
            break;
        case MID_4:
            bRet = (rVal >>= bVal); if (bRet) mbLastCol=bVal; break;
            break;
        case MID_5:
            bRet = (rVal >>= bVal); if (bRet) mbStripedRows=bVal; break;
            break;
        case MID_6:
            bRet = (rVal >>= bVal); if (bRet) mbStripedCols=bVal; break;
            break;
        case MID_7:
            bRet = (rVal >>= bVal); if (bRet) mbShowFilters=bVal; break;
            break;
        case MID_8:
        {
            OUString aVal;
            bRet = (rVal >>= aVal); if (bRet) maStyleID = std::move(aVal); break;
        }
        default:
            OSL_FAIL("Wrong MemberID!");
            return false;
    }

    return true;
}

ScDatabaseSettingItem* ScDatabaseSettingItem::Clone(SfxItemPool* ) const
{
    return new ScDatabaseSettingItem(*this);
}

SfxPoolItem* ScDatabaseSettingItem::CreateDefault()
{
    return new ScDatabaseSettingItem;
}

ScDatabaseSettingItem& ScDatabaseSettingItem::operator=(const ScDatabaseSettingItem& rItem)
{
    mbHeaderRow = rItem.mbHeaderRow;
    mbTotalRow = rItem.mbTotalRow;
    mbFirstCol = rItem.mbFirstCol;
    mbLastCol = rItem.mbLastCol;
    mbStripedRows = rItem.mbStripedRows;
    mbStripedCols = rItem.mbStripedCols;
    mbShowFilters = rItem.mbShowFilters;
    maStyleID = rItem.maStyleID;

    return *this;
}

bool ScDatabaseSettingItem::operator==(const SfxPoolItem& rItem) const
{
    bool bSameBase = SfxPoolItem::operator==(rItem);
    if (!bSameBase)
        return false;

    const ScDatabaseSettingItem& rDBItem = static_cast<const ScDatabaseSettingItem&>(rItem);
    return mbHeaderRow == rDBItem.mbHeaderRow && mbTotalRow == rDBItem.mbTotalRow && mbFirstCol == rDBItem.mbFirstCol &&
        mbLastCol == rDBItem.mbLastCol && mbStripedRows == rDBItem.mbStripedRows && mbStripedCols == rDBItem.mbStripedCols &&
        mbShowFilters == rDBItem.mbShowFilters && maStyleID == rDBItem.maStyleID;
}

bool ScDatabaseSettingItem::HasHeaderRow() const
{
    return mbHeaderRow;
}

bool ScDatabaseSettingItem::HasTotalRow() const
{
    return mbTotalRow;
}

bool ScDatabaseSettingItem::HasFirstCol() const
{
    return mbFirstCol;
}

bool ScDatabaseSettingItem::HasLastCol() const
{
    return mbLastCol;
}

bool ScDatabaseSettingItem::HasStripedRows() const
{
    return mbStripedRows;
}

bool ScDatabaseSettingItem::HasStripedCols() const
{
    return mbStripedCols;
}

bool ScDatabaseSettingItem::HasShowFilters() const
{
    return mbShowFilters;
}

const OUString& ScDatabaseSettingItem::GetStyleID() const
{
    return maStyleID;
}

ScTableStyleParam::ScTableStyleParam():
    mbRowStripes(true),
    mbColumnStripes(false),
    mbFirstColumn(false),
    mbLastColumn(false)
{
}

bool ScTableStyleParam::operator==(const ScTableStyleParam& rParam) const
{
    if(maStyleID != rParam.maStyleID)
        return false;

    if (mbRowStripes != rParam.mbRowStripes)
        return false;

    if (mbColumnStripes != rParam.mbColumnStripes)
        return false;

    if (mbFirstColumn != rParam.mbFirstColumn)
        return false;

    if (mbLastColumn != rParam.mbLastColumn)
        return false;

    return true;
}

bool ScDBData::less::operator() (const std::unique_ptr<ScDBData>& left, const std::unique_ptr<ScDBData>& right) const
{
    return ScGlobal::GetTransliteration().compareString(left->GetUpperName(), right->GetUpperName()) < 0;
}

ScDBData::ScDBData( const OUString& rName,
                    SCTAB nTab,
                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    bool bByR, bool bHasH, bool bTotals, const OUString& rTableType, const OUString& rTableStyleID) :
    // Listeners are to be setup by the "parent" container.
    mpSortParam(new ScSortParam),
    mpQueryParam(new ScQueryParam),
    mpSubTotal(new ScSubTotalParam),
    mpImportParam(new ScImportParam),
    mpContainer (nullptr),
    aName       (rName),
    aUpper      (rName),
    aTableType  (rTableType),
    nTable      (nTab),
    nStartCol   (nCol1),
    nStartRow   (nRow1),
    nEndCol     (nCol2),
    nEndRow     (nRow2),
    bByRow      (bByR),
    bHasHeader  (bHasH),
    bHasTotals  (bTotals),
    bDoSize     (false),
    bKeepFmt    (false),
    bStripData  (false),
    bIsAdvanced (false),
    bDBSelection(false),
    nIndex      (0),
    bAutoFilter (false),
    bModified   (false),
    mbTableColumnNamesDirty(true),
    nFilteredRowCount(SCSIZE_MAX)
{
    aUpper = ScGlobal::getCharClass().uppercase(aUpper);
    if (!rTableStyleID.isEmpty())
    {
        mpTableStyles.reset(new ScTableStyleParam());
        mpTableStyles->maStyleID = rTableStyleID;
    }
}

ScDBData::ScDBData( const ScDBData& rData ) :
    // Listeners are to be setup by the "parent" container.
    SvtListener         (),
    ScRefreshTimer      ( rData ),
    mpSortParam(new ScSortParam(*rData.mpSortParam)),
    mpQueryParam(new ScQueryParam(*rData.mpQueryParam)),
    mpSubTotal(new ScSubTotalParam(*rData.mpSubTotal)),
    mpImportParam(new ScImportParam(*rData.mpImportParam)),
    mpContainer         (nullptr),
    aName               (rData.aName),
    aUpper              (rData.aUpper),
    aTableType          (rData.aTableType),
    nTable              (rData.nTable),
    nStartCol           (rData.nStartCol),
    nStartRow           (rData.nStartRow),
    nEndCol             (rData.nEndCol),
    nEndRow             (rData.nEndRow),
    bByRow              (rData.bByRow),
    bHasHeader          (rData.bHasHeader),
    bHasTotals          (rData.bHasTotals),
    bDoSize             (rData.bDoSize),
    bKeepFmt            (rData.bKeepFmt),
    bStripData          (rData.bStripData),
    bIsAdvanced         (rData.bIsAdvanced),
    aAdvSource          (rData.aAdvSource),
    bDBSelection        (rData.bDBSelection),
    nIndex              (rData.nIndex),
    bAutoFilter         (rData.bAutoFilter),
    bModified           (rData.bModified),
    maTableColumnNames  (rData.maTableColumnNames),
    mbTableColumnNamesDirty(rData.mbTableColumnNamesDirty),
    nFilteredRowCount   (rData.nFilteredRowCount)
{
    if (rData.mpTableStyles)
        mpTableStyles.reset(new ScTableStyleParam(*rData.mpTableStyles));
}

ScDBData::ScDBData( const OUString& rName, const ScDBData& rData ) :
    // Listeners are to be setup by the "parent" container.
    SvtListener         (),
    ScRefreshTimer      ( rData ),
    mpSortParam(new ScSortParam(*rData.mpSortParam)),
    mpQueryParam(new ScQueryParam(*rData.mpQueryParam)),
    mpSubTotal(new ScSubTotalParam(*rData.mpSubTotal)),
    mpImportParam(new ScImportParam(*rData.mpImportParam)),
    mpContainer         (nullptr),
    aName               (rName),
    aUpper              (rName),
    aTableType          (rData.aTableType),
    nTable              (rData.nTable),
    nStartCol           (rData.nStartCol),
    nStartRow           (rData.nStartRow),
    nEndCol             (rData.nEndCol),
    nEndRow             (rData.nEndRow),
    bByRow              (rData.bByRow),
    bHasHeader          (rData.bHasHeader),
    bHasTotals          (rData.bHasTotals),
    bDoSize             (rData.bDoSize),
    bKeepFmt            (rData.bKeepFmt),
    bStripData          (rData.bStripData),
    bIsAdvanced         (rData.bIsAdvanced),
    aAdvSource          (rData.aAdvSource),
    bDBSelection        (rData.bDBSelection),
    nIndex              (rData.nIndex),
    bAutoFilter         (rData.bAutoFilter),
    bModified           (rData.bModified),
    maTableColumnNames  (rData.maTableColumnNames),
    mbTableColumnNamesDirty (rData.mbTableColumnNamesDirty),
    nFilteredRowCount   (rData.nFilteredRowCount)
{
    aUpper = ScGlobal::getCharClass().uppercase(aUpper);
    if (rData.mpTableStyles)
        mpTableStyles.reset(new ScTableStyleParam(*rData.mpTableStyles));
}

ScDBData& ScDBData::operator= (const ScDBData& rData)
{
    if (this != &rData)
    {
        // Don't modify the name.  The name is not mutable as it is used as a key
        // in the container to keep the db ranges sorted by the name.

        bool bHeaderRangeDiffers = (nTable != rData.nTable || nStartCol != rData.nStartCol ||
                nEndCol != rData.nEndCol || nStartRow != rData.nStartRow);
        bool bNeedsListening = ((bHasHeader && bHeaderRangeDiffers) || (!bHasHeader && rData.bHasHeader));
        if (bHasHeader && (!rData.bHasHeader || bHeaderRangeDiffers))
        {
            EndTableColumnNamesListener();
        }
        ScRefreshTimer::operator=( rData );
        mpSortParam.reset(new ScSortParam(*rData.mpSortParam));
        mpQueryParam.reset(new ScQueryParam(*rData.mpQueryParam));
        mpSubTotal.reset(new ScSubTotalParam(*rData.mpSubTotal));
        mpImportParam.reset(new ScImportParam(*rData.mpImportParam));
        // Keep mpContainer.
        nTable              = rData.nTable;
        nStartCol           = rData.nStartCol;
        nStartRow           = rData.nStartRow;
        nEndCol             = rData.nEndCol;
        nEndRow             = rData.nEndRow;
        bByRow              = rData.bByRow;
        bHasHeader          = rData.bHasHeader;
        bHasTotals          = rData.bHasTotals;
        bDoSize             = rData.bDoSize;
        bKeepFmt            = rData.bKeepFmt;
        bStripData          = rData.bStripData;
        bIsAdvanced         = rData.bIsAdvanced;
        aAdvSource          = rData.aAdvSource;
        aTableType          = rData.aTableType;
        bDBSelection        = rData.bDBSelection;
        nIndex              = rData.nIndex;
        bAutoFilter         = rData.bAutoFilter;
        nFilteredRowCount   = rData.nFilteredRowCount;
        if (rData.mpTableStyles)
            mpTableStyles.reset(new ScTableStyleParam(*rData.mpTableStyles));
        else
            mpTableStyles.reset();


        if (bHeaderRangeDiffers)
            InvalidateTableColumnNames( true);
        else
        {
            maTableColumnNames  = rData.maTableColumnNames;
            mbTableColumnNamesDirty = rData.mbTableColumnNamesDirty;
        }

        if (bNeedsListening)
            StartTableColumnNamesListener();
    }
    return *this;
}

bool ScDBData::operator== (const ScDBData& rData) const
{
    // Data that is not in sort or query params.

    if ( nTable     != rData.nTable     ||
         bDoSize    != rData.bDoSize    ||
         bKeepFmt   != rData.bKeepFmt   ||
         bIsAdvanced!= rData.bIsAdvanced||
         bStripData != rData.bStripData ||
//       SAB: I think this should be here, but I don't want to break something
//         bAutoFilter!= rData.bAutoFilter||
         ScRefreshTimer::operator!=( rData )
        )
        return false;

    if ( bIsAdvanced && aAdvSource != rData.aAdvSource )
        return false;

    ScSortParam aSort1, aSort2;
    GetSortParam(aSort1);
    rData.GetSortParam(aSort2);
    if (!(aSort1 == aSort2))
        return false;

    ScQueryParam aQuery1, aQuery2;
    GetQueryParam(aQuery1);
    rData.GetQueryParam(aQuery2);
    if (!(aQuery1 == aQuery2))
        return false;

    ScSubTotalParam aSubTotal1, aSubTotal2;
    GetSubTotalParam(aSubTotal1);
    rData.GetSubTotalParam(aSubTotal2);
    if (!(aSubTotal1 == aSubTotal2))
        return false;

    if ((mpTableStyles && !rData.mpTableStyles ) || (!mpTableStyles && rData.mpTableStyles))
        return false;

    if (mpTableStyles && ((*mpTableStyles) != (*rData.mpTableStyles)))
        return false;

    ScImportParam aImport1, aImport2;
    GetImportParam(aImport1);
    rData.GetImportParam(aImport2);
    return aImport1 == aImport2;
}

ScDBData::~ScDBData()
{
    StopRefreshTimer();
}

OUString ScDBData::GetSourceString() const
{
    if (mpImportParam->bImport)
        return mpImportParam->aDBName + "/" + mpImportParam->aStatement;
    return OUString();
}

OUString ScDBData::GetOperations() const
{
    OUStringBuffer aBuf;
    if (mpQueryParam->GetEntryCount())
    {
        const ScQueryEntry& rEntry = mpQueryParam->GetEntry(0);
        if (rEntry.bDoQuery)
            aBuf.append(ScResId(STR_OPERATION_FILTER));
    }

    if (mpSortParam->maKeyState[0].bDoSort)
    {
        if (!aBuf.isEmpty())
            aBuf.append(", ");
        aBuf.append(ScResId(STR_OPERATION_SORT));
    }

    if (mpSubTotal->aGroups[0].bActive && !mpSubTotal->bRemoveOnly)
    {
        if (!aBuf.isEmpty())
            aBuf.append(", ");
        aBuf.append(ScResId(STR_OPERATION_SUBTOTAL));
    }

    if (aBuf.isEmpty())
        aBuf.append(ScResId(STR_OPERATION_NONE));

    return aBuf.makeStringAndClear();
}

void ScDBData::GetArea(SCTAB& rTab, SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2) const
{
    rTab  = nTable;
    rCol1 = nStartCol;
    rRow1 = nStartRow;
    rCol2 = nEndCol;
    rRow2 = nEndRow;
}

void ScDBData::GetArea(ScRange& rRange) const
{
    SCROW nNewEndRow = nEndRow;
    rRange = ScRange( nStartCol, nStartRow, nTable, nEndCol, nNewEndRow, nTable );
}

ScRange ScDBData::GetHeaderArea() const
{
    if (HasHeader())
        return ScRange( nStartCol, nStartRow, nTable, nEndCol, nStartRow, nTable);
    return ScRange( ScAddress::INITIALIZE_INVALID);
}

void ScDBData::SetArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    bool bHeaderRangeChange = (nTab != nTable || nCol1 != nStartCol || nCol2 != nEndCol || nRow1 != nStartRow);
    if (bHeaderRangeChange)
        EndTableColumnNamesListener();

    nTable  = nTab;
    nStartCol = nCol1;
    nStartRow = nRow1;
    nEndCol   = nCol2;
    nEndRow   = nRow2;

    if (bHeaderRangeChange)
    {
        SAL_WARN_IF( !maTableColumnNames.empty(), "sc.core", "ScDBData::SetArea - invalidating column names/offsets");
        // Invalidate *after* new area has been set above to add the proper
        // header range to dirty list.
        InvalidateTableColumnNames( true);
        StartTableColumnNamesListener();
    }
}

void ScDBData::MoveTo(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                      SCCOL nUpdateCol)
{
    tools::Long nDifX = static_cast<tools::Long>(nCol1) - static_cast<tools::Long>(nStartCol);
    tools::Long nDifY = static_cast<tools::Long>(nRow1) - static_cast<tools::Long>(nStartRow);

    tools::Long nSortDif = bByRow ? nDifX : nDifY;
    tools::Long nSortEnd = bByRow ? static_cast<tools::Long>(nCol2) : static_cast<tools::Long>(nRow2);

    for (sal_uInt16 i=0; i<mpSortParam->GetSortKeyCount(); i++)
    {
        mpSortParam->maKeyState[i].nField += nSortDif;
        if (mpSortParam->maKeyState[i].nField > nSortEnd)
        {
            mpSortParam->maKeyState[i].nField = 0;
            mpSortParam->maKeyState[i].bDoSort = false;
        }
    }

    SCSIZE nCount = mpQueryParam->GetEntryCount();
    for (SCSIZE i = 0; i < nCount; ++i)
    {
        ScQueryEntry& rEntry = mpQueryParam->GetEntry(i);
        rEntry.nField += nDifX;

        // tdf#48025, tdf#141946: update the column index of the filter criteria,
        // when the deleted/inserted columns are inside the data range
        if (nUpdateCol != -1)
        {
            nUpdateCol += nDifX;
            tools::Long nDifX2
                = static_cast<tools::Long>(nCol2) - static_cast<tools::Long>(nEndCol);
            if (rEntry.nField >= nUpdateCol)
                rEntry.nField += nDifX2;
            else if (rEntry.nField >= nUpdateCol + nDifX2)
                rEntry.Clear();
        }

        if (rEntry.nField > nCol2)
        {
            rEntry.nField = 0;
            rEntry.bDoQuery = false;
        }
    }
    for (auto& group : mpSubTotal->aGroups)
    {
        group.nField += nDifX;
        if (group.nField > nCol2)
        {
            group.nField = 0;
            group.bActive = false;
        }
    }

    SetArea( nTab, nCol1, nRow1, nCol2, nRow2 );
}

void ScDBData::GetSortParam( ScSortParam& rSortParam ) const
{
    rSortParam = *mpSortParam;
    rSortParam.nCol1 = nStartCol;
    rSortParam.nRow1 = nStartRow;
    rSortParam.nCol2 = nEndCol;
    rSortParam.nRow2 = nEndRow;
    rSortParam.bByRow = bByRow;
    rSortParam.bHasHeader = bHasHeader;
    /* TODO: add Totals to ScSortParam? */
}

void ScDBData::SetSortParam( const ScSortParam& rSortParam )
{
    mpSortParam.reset(new ScSortParam(rSortParam));
    bByRow = rSortParam.bByRow;
}

void ScDBData::UpdateFromSortParam( const ScSortParam& rSortParam )
{
    bHasHeader = rSortParam.bHasHeader;
}

void ScDBData::GetQueryParam( ScQueryParam& rQueryParam ) const
{
    rQueryParam = *mpQueryParam;
    rQueryParam.nCol1 = nStartCol;
    rQueryParam.nRow1 = nStartRow;
    rQueryParam.nCol2 = nEndCol;
    rQueryParam.nRow2 = nEndRow;
    rQueryParam.nTab  = nTable;
    rQueryParam.bByRow = bByRow;
    rQueryParam.bHasHeader = bHasHeader;
    rQueryParam.bHasTotals = bHasTotals;
}

void ScDBData::SetQueryParam(const ScQueryParam& rQueryParam)
{
    mpQueryParam.reset(new ScQueryParam(rQueryParam));

    //  set bIsAdvanced to false for everything that is not from the
    //  advanced filter dialog
    bIsAdvanced = false;
}

void ScDBData::SetAdvancedQuerySource(const ScRange* pSource)
{
    if (pSource)
    {
        aAdvSource = *pSource;
        bIsAdvanced = true;
    }
    else
        bIsAdvanced = false;
}

bool ScDBData::GetAdvancedQuerySource(ScRange& rSource) const
{
    rSource = aAdvSource;
    return bIsAdvanced;
}

void ScDBData::GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const
{
    rSubTotalParam = *mpSubTotal;

    // Share the data range with the parent db data.  The range in the subtotal
    // param struct is not used.
    rSubTotalParam.nCol1 = nStartCol;
    rSubTotalParam.nRow1 = nStartRow;
    rSubTotalParam.nCol2 = nEndCol;
    rSubTotalParam.nRow2 = nEndRow;
}

void ScDBData::SetSubTotalParam(const ScSubTotalParam& rSubTotalParam)
{
    mpSubTotal.reset(new ScSubTotalParam(rSubTotalParam));
}

void ScDBData::ImportTotalRowParam(ScSubTotalParam& rSubTotalParam,
                                   const std::vector<TableColumnAttributes>& rAttributesVector,
                                   formula::FormulaGrammar::Grammar eGrammar) const
{
    rSubTotalParam.bDoSort = false;
    rSubTotalParam.bGroupedBy = false;
    rSubTotalParam.aGroups[0].nField = rSubTotalParam.nCol1; // which column we add 'Summary'

    const size_t nEntryCount = rSubTotalParam.nCol2 - rSubTotalParam.nCol1 + 1; // col count
    if (nEntryCount > 0)
    {
        SCCOL nCol = rSubTotalParam.nCol1;
        std::vector<std::pair<SCCOL, OUString>> vColLabels;
        std::vector<std::pair<SCCOL, std::unique_ptr<ScTokenArray>>> vColFuncs;
        for (const auto& rxTableColumn : rAttributesVector)
        {
            if (rxTableColumn.maTotalsRowLabel.has_value())
            {
                OUString aStr = rxTableColumn.maTotalsRowLabel.value();
                if (!aStr.isEmpty())
                    vColLabels.push_back(std::make_pair(nCol, std::move(aStr)));
            }
            else if (mpContainer && rxTableColumn.maTotalsFunction.has_value())
            {
                ScDocument& rDoc = mpContainer->GetDocument();
                const OUString& sFuncName = rxTableColumn.maTotalsFunction.value();
                if (sFuncName == u"custom"_ustr)
                {
                    if (rxTableColumn.maCustomFunction.has_value())
                    {
                        SCROW nLastRow = rSubTotalParam.nRow2;
                        if (!HasTotals())
                            nLastRow++;

                        ScAddress aPos(nCol, nLastRow, nTable);
                        ScCompiler aComp(rDoc, aPos, eGrammar, true,
                                         false);
                        std::unique_ptr<ScTokenArray> pArr
                            = aComp.CompileString(rxTableColumn.maCustomFunction.value());
                        if (pArr)
                        {
                            vColFuncs.push_back(std::make_pair(nCol, std::move(pArr)));
                        }
                    }
                }
                else
                {
                    ScSubTotalFunc eSubType = GetSubTotalFuncFromString(sFuncName);
                    if (eSubType != SUBTOTAL_FUNC_NONE)
                    {
                        std::unique_ptr<ScTokenArray> pArr(new ScTokenArray(rDoc));
                        pArr->AddOpCode(ocSubTotal);
                        pArr->AddOpCode(ocOpen);
                        pArr->AddDouble(static_cast<double>(eSubType));
                        pArr->AddOpCode(ocSep);
                        // Table refs structure
                        pArr->AddTableRef(GetIndex());
                        pArr->AddOpCode(ocTableRefOpen);
                        ScSingleRefData aSingleRef;
                        aSingleRef.InitAddress(nCol, rSubTotalParam.nRow1, nTable);
                        pArr->AddSingleReference(aSingleRef);
                        pArr->AddOpCode(ocTableRefClose);
                        // Table refs structure end
                        pArr->AddOpCode(ocClose);
                        pArr->AddOpCode(ocStop);
                        // Store
                        vColFuncs.push_back(std::make_pair(nCol, std::move(pArr)));
                    }
                }
            }
            nCol++;
        }
        rSubTotalParam.SetSubLabels(static_cast<sal_uInt16>(0), vColLabels, vColLabels.size());
        rSubTotalParam.SetCustFuncs(static_cast<sal_uInt16>(0), vColFuncs, vColFuncs.size());
    }
}

void ScDBData::CreateTotalRowParam(ScSubTotalParam& rSubTotalParam) const
{
    rSubTotalParam.bDoSort = false;
    rSubTotalParam.bGroupedBy = false;
    rSubTotalParam.aGroups[0].nField = rSubTotalParam.nCol1; // which column we add 'Summary'

    const size_t nEntryCount = rSubTotalParam.nCol2 - rSubTotalParam.nCol1 + 1; // col count
    if (nEntryCount > 0 && mpContainer)
    {
        ScDocument& rDoc = mpContainer->GetDocument();
        ScHorizontalCellIterator aIter(rDoc, nTable, rSubTotalParam.nCol1, rSubTotalParam.nRow2,
                                       rSubTotalParam.nCol2,
                                       rSubTotalParam.nRow2); // Total row only
        ScRefCellValue* pCell;
        SCCOL nCol = rSubTotalParam.nCol1 - 1;
        SCROW nRow;
        std::vector<std::pair<SCCOL, OUString>> vColLabels;
        std::vector<std::pair<SCCOL, std::unique_ptr<ScTokenArray>>> vColFuncs;
        while ((pCell = aIter.GetNext(nCol, nRow)) != nullptr)
        {
            if (pCell->getType() != CELLTYPE_FORMULA)
            {
                OUString aStr = pCell->getString(rDoc);
                if (!aStr.isEmpty())
                    vColLabels.push_back(std::make_pair(nCol, std::move(aStr)));
            }
            else
            {
                if (ScFormulaCell* pFC = pCell->getFormula())
                {
                    std::unique_ptr<ScTokenArray> pTokens = pFC->GetCode()->Clone();
                    if (pTokens)
                    {
                        vColFuncs.push_back(std::make_pair(nCol, std::move(pTokens)));
                    }
                }
            }
        }
        rSubTotalParam.SetSubLabels(static_cast<sal_uInt16>(0), vColLabels, vColLabels.size());
        rSubTotalParam.SetCustFuncs(static_cast<sal_uInt16>(0), vColFuncs, vColFuncs.size());
    }
}

std::vector<TableColumnAttributes> ScDBData::GetTotalRowAttributes(formula::FormulaGrammar::Grammar eGrammar) const
{
    ScSubTotalParam rParam;
    GetSubTotalParam(rParam);

    const SCCOL nEntryCount = rParam.nCol2 - rParam.nCol1 + 1; // col count
    std::vector<TableColumnAttributes> aAttributesVector(nEntryCount);
    if (nEntryCount > 0)
    {
        if (HasTotals())
        {
            if (!mpContainer)
                assert(!"ScDBData::GetTotalRowAttributes - how did we end up here without container?");
            else
            {
                ScDocument& rDoc = mpContainer->GetDocument();
                ScHorizontalCellIterator aIter(rDoc, nTable, rParam.nCol1, rParam.nRow2,
                                               rParam.nCol2,
                                               rParam.nRow2); // Total row only
                ScRefCellValue* pCell;
                SCCOL nCol = rParam.nCol1 - 1;
                SCROW nRow;
                while ((pCell = aIter.GetNext(nCol, nRow)) != nullptr)
                {
                    TableColumnAttributes aNameAttr;
                    if (pCell->getType() != CELLTYPE_FORMULA)
                    {
                        OUString aStr = pCell->getString(rDoc);
                        if (!aStr.isEmpty())
                            aNameAttr.maTotalsRowLabel = aStr;
                    }
                    else
                    {
                        if (ScFormulaCell* pFC = pCell->getFormula())
                        {
                            bool bSubTotal = pFC->IsSubTotal();
                            ScTokenArray* pTokens = pFC->GetCode();
                            if (bSubTotal && pTokens)
                            {
                                OUString aFunctype = GetSimpleSubTotalFunction(pTokens, nCol, rParam.nRow1);
                                if (aFunctype != u"custom"_ustr)
                                    aNameAttr.maTotalsFunction = aFunctype;
                                else
                                    bSubTotal = false; // fallback to custom
                            }

                            if (!bSubTotal && pTokens)
                            {
                                ScAddress aPos(nCol, rParam.nRow2, nTable);
                                ScCompiler aComp(rDoc, aPos, *pTokens, eGrammar);
                                OUStringBuffer aBuf;
                                aComp.CreateStringFromTokenArray(aBuf);
                                OUString aFormula = aBuf.makeStringAndClear();
                                aNameAttr.maTotalsFunction = u"custom"_ustr;
                                aNameAttr.maCustomFunction = aFormula;
                            }
                        }
                    }
                    SCCOL nPos = nCol - rParam.nCol1;
                    if (nPos < nEntryCount)
                        aAttributesVector[nPos] = std::move(aNameAttr);
                    else
                        SAL_WARN("sc.core", "ScDBData::GetTotalRowAttributes - invalid attributes/columns");
                }
            }
        }
        else
        {
            const auto& group = rParam.aGroups[0];
            if (group.nSubLabels > 0)
            {
                for (SCCOL nResult = 0; nResult < group.nSubLabels; ++nResult)
                {
                    SCCOL nPos = group.collabels(nResult) - rParam.nCol1;
                    if (nPos < nEntryCount)
                        aAttributesVector[nPos].maTotalsRowLabel = group.label(nResult);
                    else
                        SAL_WARN("sc.core", "ScDBData::GetTotalRowAttributes - invalid attributes/columns");
                }
            }

            // insert the formulas
            if (group.nCustFuncs > 0)
            {
                for (SCCOL nResult = 0; nResult < group.nCustFuncs; ++nResult)
                {
                    if (ScTokenArray* pTokens = group.custToken(nResult))
                    {
                        SCCOL nCol = group.colcust(nResult);
                        bool bSubTotal = pTokens->HasOpCode(ocSubTotal);
                        if (bSubTotal)
                        {
                            OUString aFunctype = GetSimpleSubTotalFunction(pTokens, nCol, rParam.nRow1);
                            if (aFunctype != u"custom"_ustr)
                            {
                                SCCOL nPos = nCol - rParam.nCol1;
                                if (nPos < nEntryCount)
                                    aAttributesVector[nPos].maTotalsFunction = aFunctype;
                                else
                                    SAL_WARN("sc.core", "ScDBData::GetTotalRowAttributes - invalid "
                                                        "attributes/columns");
                            }
                            else
                                bSubTotal = false; // fallback to custom
                        }

                        if (!bSubTotal)
                        {
                            if (!mpContainer)
                                assert(!"ScDBData::GetTotalRowAttributes - how did we end up here without container?");
                            else
                            {
                                ScDocument& rDoc = mpContainer->GetDocument();
                                ScAddress aPos(nCol, rParam.nRow2 + 1, nTable);
                                ScCompiler aComp(rDoc, aPos, *pTokens, eGrammar);
                                OUStringBuffer aBuf;
                                aComp.CreateStringFromTokenArray(aBuf);
                                OUString aFormula = aBuf.makeStringAndClear();

                                SCCOL nPos = nCol - rParam.nCol1;
                                if (nPos < nEntryCount)
                                {
                                    aAttributesVector[nPos].maTotalsFunction = u"custom"_ustr;
                                    aAttributesVector[nPos].maCustomFunction = aFormula;
                                }
                                else
                                    SAL_WARN("sc.core", "ScDBData::GetTotalRowAttributes - invalid "
                                                        "attributes/columns");
                            }
                        }
                    }
                }
            }
        }
    }
    return aAttributesVector;
}

OUString ScDBData::GetSimpleSubTotalFunction(const ScTokenArray* pTokens, SCCOL nCol, SCROW nHeaderRow) const
{
    std::vector<std::pair<OpCode, formula::StackVar>> expected
        = { { ocSubTotal, formula::svByte },  { ocOpen, formula::svSep },
            { ocPush, formula::svDouble },    { ocSep, formula::svSep },
            { ocTableRef, formula::svIndex }, { ocTableRefOpen, formula::svSep },
            { ocPush, formula::svSingleRef }, { ocTableRefClose, formula::svSep },
            { ocClose, formula::svSep } };

    size_t nIdx = 0;
    ScSubTotalFunc eSubType = SUBTOTAL_FUNC_NONE;
    formula::FormulaTokenArrayPlainIterator aIterResult(*pTokens);
    for (formula::FormulaToken* t = aIterResult.NextNoSpaces(); t; t = aIterResult.NextNoSpaces())
    {
        // check for subtotal opcode
        OpCode eOpCode = t->GetOpCode();
        formula::StackVar eType = t->GetType();

        if (nIdx < expected.size() && eOpCode == expected[nIdx].first
            && eType == expected[nIdx].second)
        {
            if (nIdx == 2) // { ocPush, formula::svDouble }
            {
                sal_Int32 nFunc = static_cast<sal_Int32>(t->GetDouble());
                if (nFunc > 100.)
                    nFunc -= 100;

                if (nFunc < 1 || nFunc > 11)
                {
                    return u"custom"_ustr;
                }
                else
                    eSubType = static_cast<ScSubTotalFunc>(nFunc);
            }
            else if (nIdx == 4) // { ocTableRef, formula::svIndex }
            {
                sal_uInt16 nDbIndex = t->GetIndex();
                if (GetIndex() != nDbIndex)
                {
                    return u"custom"_ustr;
                }
            }
            else if (nIdx == 6) // { ocPush, formula::svSingleRef }
            {
                const ScSingleRefData* pRef = t->GetSingleRef();
                if (!(pRef && pRef->Col() == nCol && pRef->Row() == nHeaderRow))
                {
                    return u"custom"_ustr;
                }
            }
            else
            { /*Nothing to do*/
            }

            ++nIdx;
        }
        else
        {
            return u"custom"_ustr;
        }
    }

    return GetStringFromSubTotalFunc(eSubType);
}

void ScDBData::GetImportParam(ScImportParam& rImportParam) const
{
    rImportParam = *mpImportParam;
    // set the range.
    rImportParam.nCol1 = nStartCol;
    rImportParam.nRow1 = nStartRow;
    rImportParam.nCol2 = nEndCol;
    rImportParam.nRow2 = nEndRow;
}

void ScDBData::SetImportParam(const ScImportParam& rImportParam)
{
    // the range is ignored.
    mpImportParam.reset(new ScImportParam(rImportParam));
}

bool ScDBData::IsDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) const
{
    if (nTab == nTable)
    {
        switch (ePortion)
        {
            case ScDBDataPortion::TOP_LEFT:
                return nCol == nStartCol && nRow == nStartRow;
            case ScDBDataPortion::AREA:
                return nCol >= nStartCol && nCol <= nEndCol && nRow >= nStartRow && nRow <= nEndRow;
        }
    }

    return false;
}

bool ScDBData::IsDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    return (nTab == nTable)
           && (nCol1 == nStartCol) && (nRow1 == nStartRow)
           && (nCol2 == nEndCol) && (nRow2 == nEndRow);
}

bool ScDBData::HasImportParam() const
{
    return mpImportParam && mpImportParam->bImport;
}

bool ScDBData::HasQueryParam() const
{
    if (!mpQueryParam)
        return false;

    if (!mpQueryParam->GetEntryCount())
        return false;

    return mpQueryParam->GetEntry(0).bDoQuery;
}

bool ScDBData::HasSortParam() const
{
    return mpSortParam &&
        !mpSortParam->maKeyState.empty() &&
        mpSortParam->maKeyState[0].bDoSort;
}

bool ScDBData::HasSubTotalParam() const
{
    return mpSubTotal && mpSubTotal->aGroups[0].bActive;
}

void ScDBData::UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos)
{
    ScRange aRange;
    GetArea(aRange);
    SCTAB nTab = aRange.aStart.Tab(); // a database range is only on one sheet

    //  customize as the current table as ScTablesHint (tabvwsh5.cxx)

    if (nTab == nOldPos) // moved sheet
        nTab = nNewPos;
    else if (nOldPos < nNewPos) // moved to the back
    {
        if (nTab > nOldPos && nTab <= nNewPos) // move this sheet
            --nTab;
    }
    else // moved to the front
    {
        if (nTab >= nNewPos && nTab < nOldPos) // move this sheet
            ++nTab;
    }

    bool bChanged = (nTab != aRange.aStart.Tab());
    if (bChanged)
    {
        // SetArea() invalidates column names, but it is the same column range
        // just on a different sheet; remember and set new.
        ::std::vector<OUString> aNames(maTableColumnNames);
        bool bTableColumnNamesDirty = mbTableColumnNamesDirty;
        // Same column range.
        SetArea(nTab, aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                aRange.aEnd.Row());
        // Do not use SetTableColumnNames() because that resets mbTableColumnNamesDirty.
        maTableColumnNames = std::move(aNames);
        mbTableColumnNamesDirty = bTableColumnNamesDirty;
    }

    //  MoveTo() is not necessary if only the sheet changed.

    SetModified(bChanged);
}

bool ScDBData::UpdateReference(const ScDocument& rDoc, UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCCOL nDx, SCROW nDy, SCTAB nDz)
{
    SCCOL theCol1;
    SCROW theRow1;
    SCTAB theTab1;
    SCCOL theCol2;
    SCROW theRow2;
    SCTAB theTab2;
    GetArea( theTab1, theCol1, theRow1, theCol2, theRow2 );
    theTab2 = theTab1;
    SCCOL nOldCol1 = theCol1, nOldCol2 = theCol2;

    ScRefUpdateRes eRet
        = ScRefUpdate::Update(rDoc, eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx,
                              nDy, nDz, theCol1, theRow1, theTab1, theCol2, theRow2, theTab2);

    bool bDoUpdate = eRet != UR_NOTHING;

    if (bDoUpdate && eRet != UR_INVALID)
    {
        // MoveTo() invalidates column names via SetArea(); adjust, remember and set new.
        AdjustTableColumnNames( eUpdateRefMode, nDx, nCol1, nOldCol1, nOldCol2, theCol1, theCol2);
        ::std::vector<OUString> aNames( maTableColumnNames);
        bool bTableColumnNamesDirty = mbTableColumnNamesDirty;
        // tdf#48025, tdf#141946: update the column index of the filter criteria,
        // when the deleted/inserted columns are inside the data range
        if (HasAutoFilter() && theCol1 - nOldCol1 != theCol2 - nOldCol2)
            MoveTo(theTab1, theCol1, theRow1, theCol2, theRow2, nCol1);
        else
            MoveTo( theTab1, theCol1, theRow1, theCol2, theRow2 );
        // Do not use SetTableColumnNames() because that resets mbTableColumnNamesDirty.
        maTableColumnNames = std::move(aNames);
        mbTableColumnNamesDirty = bTableColumnNamesDirty;
    }

    ScRange aRangeAdvSource;
    if ( GetAdvancedQuerySource(aRangeAdvSource) )
    {
        aRangeAdvSource.GetVars( theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
        if ( ScRefUpdate::Update( rDoc, eUpdateRefMode,
                                    nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                                    theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) )
        {
            aRangeAdvSource.aStart.Set( theCol1,theRow1,theTab1 );
            aRangeAdvSource.aEnd.Set( theCol2,theRow2,theTab2 );
            SetAdvancedQuerySource( &aRangeAdvSource );

            bDoUpdate = true;       // DBData is modified
        }
    }

    SetModified(bDoUpdate);

    return eRet == UR_INVALID;

    //TODO: check if something was deleted/inserted with-in the range !!!
}

void ScDBData::ExtendDataArea(const ScDocument& rDoc)
{
    // Extend the DB area to include data rows immediately below.
    SCCOL nOldCol1 = nStartCol, nOldCol2 = nEndCol;
    SCROW nOldEndRow = nEndRow;
    rDoc.GetDataArea(nTable, nStartCol, nStartRow, nEndCol, nEndRow, false, true);
    // nOldEndRow==rDoc.MaxRow() may easily happen when selecting whole columns and
    // setting an AutoFilter (i.e. creating an anonymous database-range). We
    // certainly don't want to iterate over nearly a million empty cells, but
    // keep only an intentionally user selected range.
    if (nOldEndRow < rDoc.MaxRow() && nEndRow < nOldEndRow)
        nEndRow = nOldEndRow;
    if (nStartCol != nOldCol1 || nEndCol != nOldCol2)
    {
        SAL_WARN_IF( !maTableColumnNames.empty(), "sc.core", "ScDBData::ExtendDataArea - invalidating column names/offsets");
        InvalidateTableColumnNames( true);
    }
}

void ScDBData::ExtendBackColorArea(const ScDocument& rDoc)
{
    // Extend the DB area to include data rows immediately below.
    SCCOL nOldCol1 = nStartCol, nOldCol2 = nEndCol;
    SCROW nOldEndRow = nEndRow;
    rDoc.GetBackColorArea(nTable, nStartCol, nStartRow, nEndCol, nEndRow);

    if (nOldEndRow < rDoc.MaxRow() && nEndRow < nOldEndRow)
        nEndRow = nOldEndRow;

    if (nStartCol != nOldCol1 || nEndCol != nOldCol2)
    {
        SAL_WARN_IF( !maTableColumnNames.empty(), "sc.core", "ScDBData::ExtendBackColorArea - invalidating column names/offsets");
        InvalidateTableColumnNames( true);
    }
}

void ScDBData::StartTableColumnNamesListener()
{
    if (mpContainer && bHasHeader)
    {
        ScDocument& rDoc = mpContainer->GetDocument();
        if (!rDoc.IsClipOrUndo())
            rDoc.StartListeningArea( GetHeaderArea(), false, this);
    }
}

void ScDBData::EndTableColumnNamesListener()
{
    EndListeningAll();
}

void ScDBData::SetTableColumnNames( ::std::vector< OUString >&& rNames )
{
    maTableColumnNames = std::move(rNames);
    mbTableColumnNamesDirty = false;
}

void ScDBData::AdjustTableColumnNames( UpdateRefMode eUpdateRefMode, SCCOL nDx, SCCOL nCol1,
        SCCOL nOldCol1, SCCOL nOldCol2, SCCOL nNewCol1, SCCOL nNewCol2 )
{
    if (maTableColumnNames.empty())
        return;

    SCCOL nDiff1 = nNewCol1 - nOldCol1;
    SCCOL nDiff2 = nNewCol2 - nOldCol2;
    if (nDiff1 == nDiff2)
        return;     // not moved or entirely moved, nothing to do

    ::std::vector<OUString> aNewNames;
    if (eUpdateRefMode == URM_INSDEL)
    {
        if (nDx > 0)
            mbTableColumnNamesDirty = true;     // inserted columns will have empty names

        // nCol1 is the first column of the block that gets shifted, determine
        // the head and tail elements that are to be copied for deletion or
        // insertion.
        size_t nHead = static_cast<size_t>(::std::max( nCol1 + std::min<SCCOL>(nDx, 0) - nOldCol1, 0));
        size_t nTail = static_cast<size_t>(::std::max( nOldCol2 - nCol1 + 1, 0));
        size_t n = nHead + nTail;
        if (0 < n && n <= maTableColumnNames.size())
        {
            if (nDx > 0)
                n += nDx;
            aNewNames.resize(n);
            // Copy head.
            for (size_t i = 0; i < nHead; ++i)
            {
                aNewNames[i] = maTableColumnNames[i];
            }
            // Copy tail, inserted middle range, if any, stays empty.
            for (size_t i = n - nTail, j = maTableColumnNames.size() - nTail; i < n; ++i, ++j)
            {
                aNewNames[i] = maTableColumnNames[j];
            }
        }
    } // else   empty aNewNames invalidates names/offsets

    SAL_WARN_IF( !maTableColumnNames.empty() && aNewNames.empty(),
            "sc.core", "ScDBData::AdjustTableColumnNames - invalidating column names/offsets");
    aNewNames.swap( maTableColumnNames);
    if (maTableColumnNames.empty())
        mbTableColumnNamesDirty = true;
    if (mbTableColumnNamesDirty)
        InvalidateTableColumnNames( false);     // preserve new column names array
}

void ScDBData::InvalidateTableColumnNames( bool bSwapToEmptyNames )
{
    mbTableColumnNamesDirty = true;
    if (bSwapToEmptyNames && !maTableColumnNames.empty())
        ::std::vector<OUString>().swap( maTableColumnNames);
    if (mpContainer)
    {
        // Add header range to dirty list.
        if (HasHeader())
            mpContainer->GetDirtyTableColumnNames().Join( GetHeaderArea());
        else
        {
            // We need *some* range in the dirty list even without header area,
            // otherwise the container would not attempt to call a refresh.
            mpContainer->GetDirtyTableColumnNames().Join( ScRange( nStartCol, nStartRow, nTable));
        }
    }
}

namespace {
class TableColumnNameSearch
{
public:
    explicit TableColumnNameSearch( OUString aSearchName ) :
        maSearchName(std::move( aSearchName ))
    {
    }

    bool operator()( const OUString& rName ) const
    {
        return ScGlobal::GetTransliteration().isEqual( maSearchName, rName);
    }

private:
    OUString maSearchName;
};

/** Set a numbered table column name at given nIndex, preventing duplicates,
    numbering starting at nCount. If nCount==0 then the first attempt is made
    with an unnumbered name and if already present the next attempt with
    nCount=2, so "Original" and "Original2". No check whether nIndex is valid. */
void SetTableColumnName( ::std::vector<OUString>& rVec, size_t nIndex, const OUString& rName, size_t nCount )
{
    OUString aStr;
    do
    {
        if (nCount)
            aStr = rName + OUString::number( nCount);
        else
        {
            aStr = rName;
            ++nCount;
        }

        if (std::none_of( rVec.begin(), rVec.end(), TableColumnNameSearch( aStr)))
        {
            rVec[nIndex] = aStr;
            break;  // do while
        }
        ++nCount;
    } while(true);
}
}

void ScDBData::RefreshTableColumnNames( ScDocument* pDoc )
{
    ::std::vector<OUString> aNewNames;
    aNewNames.resize( nEndCol - nStartCol + 1);
    bool bHaveEmpty = false;
    if (!HasHeader() || !pDoc)
        bHaveEmpty = true;  // Assume we have empty ones and fill below.
    else
    {
        ScHorizontalCellIterator aIter(*pDoc, nTable, nStartCol, nStartRow, nEndCol, nStartRow);  // header row only
        ScRefCellValue* pCell;
        SCCOL nCol, nLastColFilled = nStartCol - 1;
        SCROW nRow;
        while ((pCell = aIter.GetNext( nCol, nRow)) != nullptr)
        {
            if (pCell->hasString())
            {
                const OUString aStr = pCell->getString(*pDoc);
                if (aStr.isEmpty())
                    bHaveEmpty = true;
                else
                {
                    SetTableColumnName( aNewNames, nCol-nStartCol, aStr, 0);
                    if (nLastColFilled < nCol-1)
                        bHaveEmpty = true;
                }
                nLastColFilled = nCol;
            }
            else
                bHaveEmpty = true;
        }
    }

    // Never leave us with empty names, try to remember previous name that
    // might had been used to compile formulas, but only if same number of
    // columns and no duplicates.
    if (bHaveEmpty && aNewNames.size() == maTableColumnNames.size())
    {
        bHaveEmpty = false;
        for (size_t i=0, n=aNewNames.size(); i < n; ++i)
        {
            if (aNewNames[i].isEmpty())
            {
                const OUString& rStr = maTableColumnNames[i];
                if (rStr.isEmpty())
                    bHaveEmpty = true;
                else
                    SetTableColumnName( aNewNames, i, rStr, 0);
            }
        }
    }

    // If we still have empty ones then fill those with "Column#" with #
    // starting at the column offset number. Still no duplicates of course.
    if (bHaveEmpty)
    {
        OUString aColumn( ScResId(STR_COLUMN));
        for (size_t i=0, n=aNewNames.size(); i < n; ++i)
        {
            if (aNewNames[i].isEmpty())
                SetTableColumnName( aNewNames, i, aColumn, i+1);
        }
    }

    aNewNames.swap( maTableColumnNames);
    mbTableColumnNamesDirty = false;
}

void ScDBData::RefreshTableColumnNames( ScDocument& rDoc, const ScRange& rRange )
{
    // Header-less tables get names generated, completely empty a full refresh.
    if (mbTableColumnNamesDirty && (!HasHeader() || maTableColumnNames.empty()))
    {
        RefreshTableColumnNames( &rDoc);
        return;
    }

    // Check if this is affected for the range requested.
    ScRange aIntersection( GetHeaderArea().Intersection( rRange));
    if (!aIntersection.IsValid())
        return;

    // Always fully refresh, only one cell of a range was broadcasted per area
    // listener if multiple cells were affected. We don't know if there were
    // more. Also, we need the full check anyway in case a duplicated name was
    // entered.
    RefreshTableColumnNames( &rDoc);
}

sal_Int32 ScDBData::GetColumnNameOffset( const OUString& rName ) const
{
    if (maTableColumnNames.empty())
        return -1;

    ::std::vector<OUString>::const_iterator it(
            ::std::find_if( maTableColumnNames.begin(), maTableColumnNames.end(), TableColumnNameSearch( rName)));
    if (it != maTableColumnNames.end())
        return it - maTableColumnNames.begin();

    return -1;
}

const OUString & ScDBData::GetTableColumnName( SCCOL nCol ) const
{
    if (maTableColumnNames.empty())
        return EMPTY_OUSTRING;

    SCCOL nOffset = nCol - nStartCol;
    if (nOffset <  0 || maTableColumnNames.size() <= o3tl::make_unsigned(nOffset))
        return EMPTY_OUSTRING;

    return maTableColumnNames[nOffset];
}

void ScDBData::Notify( const SfxHint& rHint )
{
    if (rHint.GetId() != SfxHintId::ScDataChanged)
        return;
    const ScHint* pScHint = static_cast<const ScHint*>(&rHint);

    mbTableColumnNamesDirty = true;
    if (!mpContainer)
        assert(!"ScDBData::Notify - how did we end up here without container?");
    else
    {
        // Only one cell of a range is broadcasted per area listener if
        // multiple cells are affected. Expand the range to what this is
        // listening to. Broadcasted address outside should not happen,
        // but... let it trigger a refresh if.
        const ScRange aHeaderRange( GetHeaderArea());
        ScAddress aHintAddress( pScHint->GetStartAddress());
        if (aHeaderRange.IsValid())
        {
            mpContainer->GetDirtyTableColumnNames().Join( aHeaderRange);
            // Header range is one row.
            // The ScHint's "range" is an address with row count.
            // Though broadcasted is usually only one cell, check for the
            // possible case of row block and for one cell in the same row.
            if (aHintAddress.Row() <= aHeaderRange.aStart.Row()
                    && aHeaderRange.aStart.Row() < aHintAddress.Row() + pScHint->GetRowCount())
            {
                aHintAddress.SetRow( aHeaderRange.aStart.Row());
                if (!aHeaderRange.Contains( aHintAddress))
                    mpContainer->GetDirtyTableColumnNames().Join( ScRange(aHintAddress) );
            }
        }
        else
        {
            // We need *some* range in the dirty list even without header area,
            // otherwise the container would not attempt to call a refresh.
            aHintAddress.SetRow( nStartRow);
            mpContainer->GetDirtyTableColumnNames().Join( ScRange(aHintAddress) );
        }
    }

    // Do not refresh column names here, which might trigger unwanted
    // recalculation.
}

void ScDBData::CalcSaveFilteredCount( SCSIZE nNonFilteredRowCount )
{
    SCSIZE nTotal = nEndRow - nStartRow + 1;
    if ( bHasHeader )
        nTotal -= 1;
    nFilteredRowCount = nTotal - nNonFilteredRowCount;
}

void ScDBData::GetFilterSelCount( SCSIZE& nSelected, SCSIZE& nTotal )
{
    nTotal = nEndRow - nStartRow + 1;
    if ( bHasHeader )
        nTotal -= 1;
    if( nFilteredRowCount != SCSIZE_MAX )
        nSelected = nTotal - nFilteredRowCount;
    else
        nSelected = nFilteredRowCount;
}

void ScDBData::SetTableStyleInfo(const ScTableStyleParam& rParam)
{
    mpTableStyles.reset(new ScTableStyleParam(rParam));
}

const ScTableStyleParam* ScDBData::GetTableStyleInfo() const
{
    return mpTableStyles.get();
}

void ScDBData::RemoveTableStyleInfo()
{
    mpTableStyles.reset();
}

ScSubTotalFunc ScDBData::GetSubTotalFuncFromString(std::u16string_view sFunction)
{
    if (sFunction == u"sum")
        return SUBTOTAL_FUNC_SUM;
    else if (sFunction == u"countNums")
        return SUBTOTAL_FUNC_CNT;
    else if (sFunction == u"count")
        return SUBTOTAL_FUNC_CNT2;
    /*else if (sFunction)
        return SUBTOTAL_FUNC_PROD;*/
    else if (sFunction == u"average")
        return SUBTOTAL_FUNC_AVE;
    /*else if (sFunction)
        return SUBTOTAL_FUNC_MED;*/
    else if (sFunction == u"max")
        return SUBTOTAL_FUNC_MAX;
    else if (sFunction == u"min")
        return SUBTOTAL_FUNC_MIN;
    else if (sFunction == u"stdDev")
        return SUBTOTAL_FUNC_STD;
    /*else if (sFunction)
        return SUBTOTAL_FUNC_STDP;*/
    else if (sFunction == u"var")
        return SUBTOTAL_FUNC_VAR;
    /*else if (sFunction)
        return SUBTOTAL_FUNC_VARP;*/
    else
        return SUBTOTAL_FUNC_NONE;
}

OUString ScDBData::GetStringFromSubTotalFunc(ScSubTotalFunc eFunc)
{
    if (eFunc == SUBTOTAL_FUNC_SUM)
        return u"sum"_ustr;
    else if (eFunc == SUBTOTAL_FUNC_CNT)
        return u"countNums"_ustr;
    else if (eFunc == SUBTOTAL_FUNC_CNT2)
        return u"count"_ustr;
    else if (eFunc == SUBTOTAL_FUNC_PROD)
        return u"custom"_ustr; // ooxml not support in Total row
    else if (eFunc == SUBTOTAL_FUNC_AVE)
        return u"average"_ustr;
    else if (eFunc == SUBTOTAL_FUNC_MED)
        return u"custom"_ustr; // ooxml not support in Total row
    else if (eFunc == SUBTOTAL_FUNC_MAX)
        return u"max"_ustr;
    else if (eFunc == SUBTOTAL_FUNC_MIN)
        return u"min"_ustr;
    else if (eFunc == SUBTOTAL_FUNC_STD)
        return u"stdDev"_ustr;
    else if (eFunc == SUBTOTAL_FUNC_STDP)
        return u"custom"_ustr; // ooxml not support in Total row
    else if (eFunc == SUBTOTAL_FUNC_VAR)
        return u"var"_ustr;
    else
        return u"none"_ustr;
}

namespace {

class FindByTable
{
    SCTAB mnTab;
public:
    explicit FindByTable(SCTAB nTab) : mnTab(nTab) {}

    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        ScRange aRange;
        p->GetArea(aRange);
        return aRange.aStart.Tab() == mnTab;
    }
};

class UpdateMoveTabFunc
{
    SCTAB mnOldTab;
    SCTAB mnNewTab;
public:
    UpdateMoveTabFunc(SCTAB nOld, SCTAB nNew) : mnOldTab(nOld), mnNewTab(nNew) {}
    void operator() (std::unique_ptr<ScDBData> const& p)
    {
        p->UpdateMoveTab(mnOldTab, mnNewTab);
    }
};

OUString lcl_IncrementNumberInNamedRange(ScDBCollection::NamedDBs& namedDBs,
                                         std::u16string_view rOldName)
{
    // Append or increment a numeric suffix and do not generate names that
    // could result in a cell reference by ensuring at least one underscore is
    // present.
    // "aa"     => "aa_2"
    // "aaaa1"  => "aaaa1_2"
    // "aa_a"   => "aa_a_2"
    // "aa_a_"  => "aa_a__2"
    // "aa_a1"  => "aa_a1_2"
    // "aa_1a"  => "aa_1a_2"
    // "aa_1"   => "aa_2"
    // "aa_2"   => "aa_3"

    size_t nLastIndex = rOldName.rfind('_');
    sal_Int32 nOldNumber = 1;
    OUString aPrefix;
    if (nLastIndex != std::u16string_view::npos)
    {
        ++nLastIndex;
        std::u16string_view sLastPart(rOldName.substr(nLastIndex));
        nOldNumber = o3tl::toInt32(sLastPart);

        // If that number is exactly at the end then increment the number; else
        // append "_" and number.
        // toInt32() returns 0 on failure and also stops at trailing non-digit
        // characters (toInt32("1a")==1).
        if (OUString::number(nOldNumber) == sLastPart)
            aPrefix = rOldName.substr(0, nLastIndex);
        else
        {
            aPrefix = OUString::Concat(rOldName) + "_";
            nOldNumber = 1;
        }
    }
    else // No "_" found, append "_" and number.
        aPrefix = OUString::Concat(rOldName) + "_";
    OUString sNewName;
    do
    {
        sNewName = aPrefix + OUString::number(++nOldNumber);
    } while (namedDBs.findByName(sNewName) != nullptr);
    return sNewName;
}

class FindTableByCursor
{
    SCCOL mnCol;
    SCROW mnRow;
    SCTAB mnTab;
    ScDBDataPortion mePortion;

public:
    FindTableByCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion)
        : mnCol(nCol)
        , mnRow(nRow)
        , mnTab(nTab)
        , mePortion(ePortion)
    {
    }

    bool operator()(std::unique_ptr<ScDBData> const& p)
    {
        return p->IsDBAtCursor(mnCol, mnRow, mnTab, mePortion) && p->GetTableStyleInfo();
    }
};

class FindByCursor
{
    SCCOL mnCol;
    SCROW mnRow;
    SCTAB mnTab;
    ScDBDataPortion mePortion;
public:
    FindByCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) :
        mnCol(nCol), mnRow(nRow), mnTab(nTab), mePortion(ePortion) {}

    bool operator() (std::unique_ptr<ScDBData> const& p)
    {
        return p->IsDBAtCursor(mnCol, mnRow, mnTab, mePortion);
    }
};

class FindByRange
{
    const ScRange& mrRange;
public:
    explicit FindByRange(const ScRange& rRange) : mrRange(rRange) {}

    bool operator() (std::unique_ptr<ScDBData> const& p)
    {
        return p->IsDBAtArea(
            mrRange.aStart.Tab(), mrRange.aStart.Col(), mrRange.aStart.Row(), mrRange.aEnd.Col(), mrRange.aEnd.Row());
    }
};

class FindByIndex
{
    sal_uInt16 mnIndex;
public:
    explicit FindByIndex(sal_uInt16 nIndex) : mnIndex(nIndex) {}
    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        return p->GetIndex() == mnIndex;
    }
};

class FindByUpperName
{
    const OUString& mrName;
public:
    explicit FindByUpperName(const OUString& rName) : mrName(rName) {}
    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        return p->GetUpperName() == mrName;
    }
};

class FindByName
{
    const OUString& mrName;
public:
    explicit FindByName(const OUString& rName) : mrName(rName) {}
    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        return p->GetName() == mrName;
    }
};

class FindByPointer
{
    const ScDBData* mpDBData;
public:
    explicit FindByPointer(const ScDBData* pDBData) : mpDBData(pDBData) {}
    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        return p.get() == mpDBData;
    }
};

}

ScDocument& ScDBDataContainerBase::GetDocument() const
{
    return mrDoc;
}

ScRangeList& ScDBDataContainerBase::GetDirtyTableColumnNames()
{
    return maDirtyTableColumnNames;
}

ScDBCollection::NamedDBs::NamedDBs(ScDBCollection& rParent, ScDocument& rDoc) :
    ScDBDataContainerBase(rDoc), mrParent(rParent) {}

ScDBCollection::NamedDBs::NamedDBs(const NamedDBs& r, ScDBCollection& rParent)
    : ScDBDataContainerBase(r.mrDoc)
    , mrParent(rParent)
{
    for (auto const& it : r.m_DBs)
    {
        ScDBData* p = new ScDBData(*it);
        std::unique_ptr<ScDBData> pData(p);
        if (m_DBs.insert( std::move(pData)).second)
            initInserted(p);
    }
}

ScDBCollection::NamedDBs::~NamedDBs()
{
}

void ScDBCollection::NamedDBs::initInserted( ScDBData* p )
{
    p->SetContainer( this);
    if (mrDoc.IsClipOrUndo())
        return;

    p->StartTableColumnNamesListener(); // needs the container be set already
    if (!p->AreTableColumnNamesDirty())
        return;

    if (p->HasHeader())
    {
        // Refresh table column names in next round.
        maDirtyTableColumnNames.Join( p->GetHeaderArea());
    }
    else
    {
        // Header-less table can generate its column names
        // already without accessing the document.
        p->RefreshTableColumnNames( nullptr);
    }
}

ScDBCollection::NamedDBs::iterator ScDBCollection::NamedDBs::begin()
{
    return m_DBs.begin();
}

ScDBCollection::NamedDBs::iterator ScDBCollection::NamedDBs::end()
{
    return m_DBs.end();
}

ScDBCollection::NamedDBs::const_iterator ScDBCollection::NamedDBs::begin() const
{
    return m_DBs.begin();
}

ScDBCollection::NamedDBs::const_iterator ScDBCollection::NamedDBs::end() const
{
    return m_DBs.end();
}

ScDBData* ScDBCollection::NamedDBs::findByIndex(sal_uInt16 nIndex)
{
    DBsType::iterator itr = find_if(
        m_DBs.begin(), m_DBs.end(), FindByIndex(nIndex));
    return itr == m_DBs.end() ? nullptr : itr->get();
}

ScDBData* ScDBCollection::NamedDBs::findByUpperName(const OUString& rName)
{
    DBsType::iterator itr = find_if(
        m_DBs.begin(), m_DBs.end(), FindByUpperName(rName));
    return itr == m_DBs.end() ? nullptr : itr->get();
}

auto ScDBCollection::NamedDBs::findByUpperName2(const OUString& rName) -> iterator
{
    return find_if(
        m_DBs.begin(), m_DBs.end(), FindByUpperName(rName));
}

auto ScDBCollection::NamedDBs::findByPointer( const ScDBData* p ) -> iterator
{
    return find_if(m_DBs.begin(), m_DBs.end(), FindByPointer(p));
}

ScDBData* ScDBCollection::NamedDBs::findByName(const OUString& rName)
{
    DBsType::iterator itr = find_if(m_DBs.begin(), m_DBs.end(), FindByName(rName));
    return itr == m_DBs.end() ? nullptr : itr->get();
}

bool ScDBCollection::NamedDBs::insert(std::unique_ptr<ScDBData> pData)
{
    auto p = pData.get();
    if (!pData->GetIndex())
        pData->SetIndex(mrParent.nEntryIndex++);

    std::pair<DBsType::iterator, bool> r = m_DBs.insert(std::move(pData));

    if (r.second)
    {
        initInserted(p);

        /* TODO: shouldn't the import refresh not be setup for
         * clipboard/undo documents? It was already like this before... */
        if (p->HasImportParam() && !p->HasImportSelection())
        {
            p->SetRefreshHandler(mrParent.GetRefreshHandler());
            p->SetRefreshControl(&mrDoc.GetRefreshTimerControlAddress());
        }
    }
    return r.second;
}

ScDBCollection::NamedDBs::iterator ScDBCollection::NamedDBs::erase(const iterator& itr)
{
    return m_DBs.erase(itr);
}

bool ScDBCollection::NamedDBs::empty() const
{
    return m_DBs.empty();
}

size_t ScDBCollection::NamedDBs::size() const
{
    return m_DBs.size();
}

bool ScDBCollection::NamedDBs::operator== (const NamedDBs& r) const
{
    return ::comphelper::ContainerUniquePtrEquals(m_DBs, r.m_DBs);
}

ScDBCollection::AnonDBs::iterator ScDBCollection::AnonDBs::begin()
{
    return m_DBs.begin();
}

ScDBCollection::AnonDBs::iterator ScDBCollection::AnonDBs::end()
{
    return m_DBs.end();
}

ScDBCollection::AnonDBs::const_iterator ScDBCollection::AnonDBs::begin() const
{
    return m_DBs.begin();
}

ScDBCollection::AnonDBs::const_iterator ScDBCollection::AnonDBs::end() const
{
    return m_DBs.end();
}

const ScDBData* ScDBCollection::AnonDBs::findAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab,
        ScDBDataPortion ePortion) const
{
    DBsType::const_iterator itr = find_if(
        m_DBs.begin(), m_DBs.end(), FindByCursor(nCol, nRow, nTab, ePortion));
    return itr == m_DBs.end() ? nullptr : itr->get();
}

const ScDBData* ScDBCollection::AnonDBs::findByRange(const ScRange& rRange) const
{
    DBsType::const_iterator itr = find_if(
        m_DBs.begin(), m_DBs.end(), FindByRange(rRange));
    return itr == m_DBs.end() ? nullptr : itr->get();
}

void ScDBCollection::AnonDBs::deleteOnTab(SCTAB nTab)
{
    FindByTable func(nTab);
    std::erase_if(m_DBs, func);
}

ScDBData* ScDBCollection::AnonDBs::getByRange(const ScRange& rRange)
{
    const ScDBData* pData = findByRange(rRange);
    if (!pData)
    {
        // Insert a new db data.  They all have identical names.
        ::std::unique_ptr<ScDBData> pNew(new ScDBData(
            STR_DB_GLOBAL_NONAME, rRange.aStart.Tab(), rRange.aStart.Col(), rRange.aStart.Row(),
            rRange.aEnd.Col(), rRange.aEnd.Row(), true, false, false));
        pData = pNew.get();
        m_DBs.push_back(std::move(pNew));
    }
    return const_cast<ScDBData*>(pData);
}

void ScDBCollection::AnonDBs::insert(ScDBData* p)
{
    m_DBs.push_back(std::unique_ptr<ScDBData>(p));
}

ScDBCollection::AnonDBs::iterator ScDBCollection::AnonDBs::erase(const iterator& itr)
{
    return m_DBs.erase(itr);
}

bool ScDBCollection::AnonDBs::empty() const
{
    return m_DBs.empty();
}

bool ScDBCollection::AnonDBs::has( const ScDBData* p ) const
{
    return any_of(m_DBs.begin(), m_DBs.end(), FindByPointer(p));
}

bool ScDBCollection::AnonDBs::operator== (const AnonDBs& r) const
{
    return ::comphelper::ContainerUniquePtrEquals(m_DBs, r.m_DBs);
}

ScDBCollection::AnonDBs::AnonDBs()
{
}

ScDBCollection::AnonDBs::AnonDBs(AnonDBs const& r)
{
    m_DBs.reserve(r.m_DBs.size());
    for (auto const& it : r.m_DBs)
    {
        m_DBs.push_back(std::make_unique<ScDBData>(*it));
    }
}

ScDBCollection::ScDBCollection(ScDocument& rDocument) :
    rDoc(rDocument), nEntryIndex(1), maNamedDBs(*this, rDocument) {}

ScDBCollection::ScDBCollection(const ScDBCollection& r) :
    rDoc(r.rDoc), nEntryIndex(r.nEntryIndex), maNamedDBs(r.maNamedDBs, *this), maAnonDBs(r.maAnonDBs) {}

const ScDBData* ScDBCollection::GetTableDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab,
                                             ScDBDataPortion ePortion) const
{
    // only search the global named db ranges since Table names can only be there.
    NamedDBs::DBsType::iterator itr = find_if(maNamedDBs.begin(), maNamedDBs.end(),
                                              FindTableByCursor(nCol, nRow, nTab, ePortion));
    if (itr != maNamedDBs.end())
        return itr->get();

    return nullptr;
}

ScDBData* ScDBCollection::GetTableDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab,
                                             ScDBDataPortion ePortion)
{
    // only search the global named db ranges since Table names can only be there.
    NamedDBs::DBsType::iterator itr = find_if(maNamedDBs.begin(), maNamedDBs.end(),
                                              FindTableByCursor(nCol, nRow, nTab, ePortion));
    if (itr != maNamedDBs.end())
        return itr->get();

    return nullptr;
}

const ScDBData* ScDBCollection::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) const
{
    // First, search the global named db ranges.
    NamedDBs::DBsType::const_iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByCursor(nCol, nRow, nTab, ePortion));
    if (itr != maNamedDBs.end())
        return itr->get();

    // Check for the sheet-local anonymous db range.
    const ScDBData* pNoNameData = rDoc.GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtCursor(nCol,nRow,nTab,ePortion))
            return pNoNameData;

    // Check the global anonymous db ranges.
    const ScDBData* pData = getAnonDBs().findAtCursor(nCol, nRow, nTab, ePortion);
    if (pData)
        return pData;

    // Do NOT check for the document global temporary anonymous db range here.

    return nullptr;
}

ScDBData* ScDBCollection::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion)
{
    // First, search the global named db ranges.
    NamedDBs::DBsType::iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByCursor(nCol, nRow, nTab, ePortion));
    if (itr != maNamedDBs.end())
        return itr->get();

    // Check for the sheet-local anonymous db range.
    ScDBData* pNoNameData = rDoc.GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtCursor(nCol,nRow,nTab,ePortion))
            return pNoNameData;

    // Check the global anonymous db ranges.
    const ScDBData* pData = getAnonDBs().findAtCursor(nCol, nRow, nTab, ePortion);
    if (pData)
        return const_cast<ScDBData*>(pData);

    // Do NOT check for the document global temporary anonymous db range here.

    return nullptr;
}

const ScDBData* ScDBCollection::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    // First, search the global named db ranges.
    ScRange aRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
    NamedDBs::DBsType::const_iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByRange(aRange));
    if (itr != maNamedDBs.end())
        return itr->get();

    // Check for the sheet-local anonymous db range.
    ScDBData* pNoNameData = rDoc.GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            return pNoNameData;

    // Lastly, check the global anonymous db ranges.
    const ScDBData* pData = maAnonDBs.findByRange(aRange);
    if (pData)
        return pData;

    // As a last resort, check for the document global temporary anonymous db range.
    pNoNameData = rDoc.GetAnonymousDBData();
    if (pNoNameData)
        if (pNoNameData->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            return pNoNameData;

    return nullptr;
}

ScDBData* ScDBCollection::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    // First, search the global named db ranges.
    ScRange aRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
    NamedDBs::DBsType::iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByRange(aRange));
    if (itr != maNamedDBs.end())
        return itr->get();

    // Check for the sheet-local anonymous db range.
    ScDBData* pNoNameData = rDoc.GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            return pNoNameData;

    // Lastly, check the global anonymous db ranges.
    const ScDBData* pData = getAnonDBs().findByRange(aRange);
    if (pData)
        return const_cast<ScDBData*>(pData);

    // As a last resort, check for the document global temporary anonymous db range.
    pNoNameData = rDoc.GetAnonymousDBData();
    if (pNoNameData)
        if (pNoNameData->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            return pNoNameData;

    return nullptr;
}

namespace {

bool intersectsRange(const ScDBData* pDBData, ScRange& rRange)
{
    ScRange aRange;
    pDBData->GetArea(aRange);
    return rRange.Intersects(aRange);
}

}

std::vector<const ScDBData*> ScDBCollection::GetAllNamedDBsInArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, SCTAB nTab) const
{
    ScRange aTargetRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
    std::vector<const ScDBData*> aDBData;
    for (const auto& rxNamedDB: maNamedDBs)
    {
        if (rxNamedDB->GetTab() != nTab)
            continue;

        if (intersectsRange(rxNamedDB.get(), aTargetRange))
        {
            aDBData.emplace_back(rxNamedDB.get());
        }
    }
    return aDBData;
}

void ScDBCollection::RefreshDirtyTableColumnNames()
{
    for (size_t i=0; i < maNamedDBs.maDirtyTableColumnNames.size(); ++i)
    {
        const ScRange & rRange = maNamedDBs.maDirtyTableColumnNames[i];
        for (auto const& it : maNamedDBs)
        {
            if (it->AreTableColumnNamesDirty())
                it->RefreshTableColumnNames( maNamedDBs.mrDoc, rRange);
        }
    }
    maNamedDBs.maDirtyTableColumnNames.RemoveAll();
}

void ScDBCollection::DeleteOnTab( SCTAB nTab )
{
    FindByTable func(nTab);
    // First, collect the positions of all items that need to be deleted.
    ::std::vector<NamedDBs::DBsType::iterator> v;
    {
        NamedDBs::DBsType::iterator itr = maNamedDBs.begin(), itrEnd = maNamedDBs.end();
        for (; itr != itrEnd; ++itr)
        {
            if (func(*itr))
                v.push_back(itr);
        }
    }

    // Delete them all.
    for (const auto& rIter : v)
        maNamedDBs.erase(rIter);

    maAnonDBs.deleteOnTab(nTab);
}

void ScDBCollection::UpdateReference(UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCCOL nDx, SCROW nDy, SCTAB nDz )
{
    ScDBData* pData = rDoc.GetAnonymousDBData(nTab1);
    if (pData)
    {
        if (nTab1 == nTab2 && nDz == 0)
        {
            // Delete the database range, if some part of the reference became invalid.
            if (pData->UpdateReference(rDoc, eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2,
                                       nTab2, nDx, nDy, nDz))
                rDoc.SetAnonymousDBData(nTab1, nullptr);
        }
        else
        {
            //this will perhaps break undo
        }
    }

    for (auto it = maNamedDBs.begin(); it != maNamedDBs.end(); )
    {
        // Delete the database range, if some part of the reference became invalid.
        if (it->get()->UpdateReference(rDoc, eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2,
                                       nTab2, nDx, nDy, nDz))
            it = maNamedDBs.erase(it);
        else
            ++it;
    }
    for (auto it = maAnonDBs.begin(); it != maAnonDBs.end(); )
    {
        // Delete the database range, if some part of the reference became invalid.
        if (it->get()->UpdateReference(rDoc, eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2,
                                       nTab2, nDx, nDy, nDz))
            it = maAnonDBs.erase(it);
        else
            ++it;
    }
}

void ScDBCollection::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    UpdateMoveTabFunc func(nOldPos, nNewPos);
    for_each(maNamedDBs.begin(), maNamedDBs.end(), func);
    for_each(maAnonDBs.begin(), maAnonDBs.end(), func);
}

void ScDBCollection::CopyToTable(SCTAB nOldPos, SCTAB nNewPos)
{
    // Create temporary copy of pointers to not insert in a set we are
    // iterating over.
    std::vector<const ScDBData*> aTemp;
    aTemp.reserve( maNamedDBs.size());
    for (const auto& rxNamedDB : maNamedDBs)
    {
        if (rxNamedDB->GetTab() != nOldPos)
            continue;
        aTemp.emplace_back( rxNamedDB.get());
    }
    for (const auto& rxNamedDB : aTemp)
    {
        const OUString newName( lcl_IncrementNumberInNamedRange( maNamedDBs, rxNamedDB->GetName()));
        std::unique_ptr<ScDBData> pDataCopy = std::make_unique<ScDBData>(newName, *rxNamedDB);
        pDataCopy->UpdateMoveTab(nOldPos, nNewPos);
        pDataCopy->SetIndex(0);
        (void)maNamedDBs.insert(std::move(pDataCopy));
    }
}

ScDBData* ScDBCollection::GetDBNearCursor(SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    ScDBData* pNearData = nullptr;
    for (const auto& rxNamedDB : maNamedDBs)
    {
        SCTAB nAreaTab;
        SCCOL nStartCol, nEndCol;
        SCROW nStartRow, nEndRow;
        rxNamedDB->GetArea( nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow );
        if ( nTab == nAreaTab && nCol+1 >= nStartCol && nCol <= nEndCol+1 &&
                                 nRow+1 >= nStartRow && nRow <= nEndRow+1 )
        {
            if ( nCol < nStartCol || nCol > nEndCol || nRow < nStartRow || nRow > nEndRow )
            {
                if (!pNearData)
                    pNearData = rxNamedDB.get(); // remember first adjacent area
            }
            else
                return rxNamedDB.get();          // not "unbenannt"/"unnamed" and cursor within
        }
    }
    if (pNearData)
        return pNearData;                   // adjacent, if no direct hit
    return rDoc.GetAnonymousDBData(nTab);  // "unbenannt"/"unnamed" only if nothing else
}

std::vector<ScDBData*> ScDBCollection::GetAllDBsFromTab(SCTAB nTab)
{
    std::vector<ScDBData*> pTabData;
    for (const auto& rxNamedDB : maNamedDBs)
    {
        if (rxNamedDB->GetTab() == nTab)
            pTabData.emplace_back(rxNamedDB.get());
    }
    auto pAnonDBData = rDoc.GetAnonymousDBData(nTab);
    if (pAnonDBData)
        pTabData.emplace_back(pAnonDBData);
    return pTabData;
}

bool ScDBCollection::empty() const
{
    return maNamedDBs.empty() && maAnonDBs.empty();
}

bool ScDBCollection::operator== (const ScDBCollection& r) const
{
    return maNamedDBs == r.maNamedDBs && maAnonDBs == r.maAnonDBs &&
        nEntryIndex == r.nEntryIndex && &rDoc == &r.rDoc && aRefreshHandler == r.aRefreshHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
