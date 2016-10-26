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

#include "externalrefmgr.hxx"
#include "document.hxx"
#include "token.hxx"
#include "tokenarray.hxx"
#include "address.hxx"
#include "tablink.hxx"
#include "docsh.hxx"
#include "scextopt.hxx"
#include "rangenam.hxx"
#include "formulacell.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "cellvalue.hxx"
#include "defaultsoptions.hxx"

#include <osl/file.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/objsh.hxx>
#include <svl/broadcast.hxx>
#include <svl/smplhint.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/urihelper.hxx>
#include <svl/zformat.hxx>
#include <svl/sharedstringpool.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbhelper.hxx>
#include <vcl/msgbox.hxx>
#include "stringutil.hxx"
#include "scmatrix.hxx"
#include <columnspanset.hxx>
#include <column.hxx>
#include <com/sun/star/document/MacroExecMode.hpp>

#include <memory>
#include <algorithm>

using ::std::unique_ptr;
using ::com::sun::star::uno::Any;
using ::std::vector;
using ::std::find;
using ::std::find_if;
using ::std::distance;
using ::std::pair;
using ::std::list;
using ::std::unary_function;
using namespace formula;

#define SRCDOC_LIFE_SPAN     30000      // 5 minutes (in 100th of a sec)
#define SRCDOC_SCAN_INTERVAL 1000*30    // every 30 seconds (in msec)

namespace {

class TabNameSearchPredicate : public unary_function<ScExternalRefCache::TableName, bool>
{
public:
    explicit TabNameSearchPredicate(const OUString& rSearchName) :
        maSearchName(ScGlobal::pCharClass->uppercase(rSearchName))
    {
    }

    bool operator()(const ScExternalRefCache::TableName& rTabNameSet) const
    {
        // Ok, I'm doing case insensitive search here.
        return rTabNameSet.maUpperName.equals(maSearchName);
    }

private:
    OUString maSearchName;
};

class FindSrcFileByName : public unary_function<ScExternalRefManager::SrcFileData, bool>
{
public:
    explicit FindSrcFileByName(const OUString& rMatchName) :
        mrMatchName(rMatchName)
    {
    }

    bool operator()(const ScExternalRefManager::SrcFileData& rSrcData) const
    {
        return rSrcData.maFileName.equals(mrMatchName);
    }

private:
    const OUString& mrMatchName;
};

class NotifyLinkListener : public unary_function<ScExternalRefManager::LinkListener*,  void>
{
public:
    NotifyLinkListener(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType) :
        mnFileId(nFileId), meType(eType) {}

    NotifyLinkListener(const NotifyLinkListener& r) :
        mnFileId(r.mnFileId), meType(r.meType) {}

    void operator() (ScExternalRefManager::LinkListener* p) const
    {
        p->notify(mnFileId, meType);
    }
private:
    sal_uInt16 mnFileId;
    ScExternalRefManager::LinkUpdateType meType;
};

struct UpdateFormulaCell : public unary_function<ScFormulaCell*, void>
{
    void operator() (ScFormulaCell* pCell) const
    {
        // Check to make sure the cell really contains ocExternalRef.
        // External names, external cell and range references all have a
        // ocExternalRef token.
        ScTokenArray* pCode = pCell->GetCode();
        if (!pCode->HasExternalRef())
            return;

        if (pCode->GetCodeError())
        {
            // Clear the error code, or a cell with error won't get re-compiled.
            pCode->SetCodeError(0);
            pCell->SetCompile(true);
            pCell->CompileTokenArray();
        }

        pCell->SetDirty();
    }
};

class RemoveFormulaCell : public unary_function<pair<const sal_uInt16, ScExternalRefManager::RefCellSet>, void>
{
public:
    explicit RemoveFormulaCell(ScFormulaCell* p) : mpCell(p) {}
    void operator() (pair<const sal_uInt16, ScExternalRefManager::RefCellSet>& r) const
    {
        r.second.erase(mpCell);
    }
private:
    ScFormulaCell* mpCell;
};

class ConvertFormulaToStatic : public unary_function<ScFormulaCell*, void>
{
public:
    explicit ConvertFormulaToStatic(ScDocument* pDoc) : mpDoc(pDoc) {}
    void operator() (ScFormulaCell* pCell) const
    {
        ScAddress aPos = pCell->aPos;

        // We don't check for empty cells because empty external cells are
        // treated as having a value of 0.

        if (pCell->IsValue())
        {
            // Turn this into value cell.
            mpDoc->SetValue(aPos, pCell->GetValue());
        }
        else
        {
            // string cell otherwise.
            ScSetStringParam aParam;
            aParam.setTextInput();
            mpDoc->SetString(aPos, pCell->GetString().getString(), &aParam);
        }
    }
private:
    ScDocument* mpDoc;
};

/**
 * Check whether a named range contains an external reference to a
 * particular document.
 */
bool hasRefsToSrcDoc(ScRangeData& rData, sal_uInt16 nFileId)
{
    ScTokenArray* pArray = rData.GetCode();
    if (!pArray)
        return false;

    pArray->Reset();
    formula::FormulaToken* p = pArray->GetNextReference();
    for (; p; p = pArray->GetNextReference())
    {
        if (!p->IsExternalRef())
            continue;

        if (p->GetIndex() == nFileId)
            return true;
    }
    return false;
}

class EraseRangeByIterator : public unary_function<ScRangeName::iterator, void>
{
    ScRangeName& mrRanges;
public:
    explicit EraseRangeByIterator(ScRangeName& rRanges) : mrRanges(rRanges) {}
    void operator() (const ScRangeName::iterator& itr)
    {
        mrRanges.erase(itr);
    }
};

/**
 * Remove all named ranges that contain references to specified source
 * document.
 */
void removeRangeNamesBySrcDoc(ScRangeName& rRanges, sal_uInt16 nFileId)
{
    ScRangeName::iterator itr = rRanges.begin(), itrEnd = rRanges.end();
    vector<ScRangeName::iterator> v;
    for (; itr != itrEnd; ++itr)
    {
        if (hasRefsToSrcDoc(*itr->second, nFileId))
            v.push_back(itr);
    }
    for_each(v.begin(), v.end(), EraseRangeByIterator(rRanges));
}

}

ScExternalRefCache::Table::Table()
    : meReferenced( REFERENCED_MARKED )
      // Prevent accidental data loss due to lack of knowledge.
{
}

ScExternalRefCache::Table::~Table()
{
}

void ScExternalRefCache::Table::clear()
{
    maRows.clear();
    maCachedRanges.RemoveAll();
    meReferenced = REFERENCED_MARKED;
}

void ScExternalRefCache::Table::setReferencedFlag( ScExternalRefCache::Table::ReferencedFlag eFlag )
{
    meReferenced = eFlag;
}

void ScExternalRefCache::Table::setReferenced( bool bReferenced )
{
    if (meReferenced != REFERENCED_PERMANENT)
        meReferenced = (bReferenced ? REFERENCED_MARKED : UNREFERENCED);
}

bool ScExternalRefCache::Table::isReferenced() const
{
    return meReferenced != UNREFERENCED;
}

void ScExternalRefCache::Table::setCell(SCCOL nCol, SCROW nRow, TokenRef pToken, sal_uLong nFmtIndex, bool bSetCacheRange)
{
    using ::std::pair;
    RowsDataType::iterator itrRow = maRows.find(nRow);
    if (itrRow == maRows.end())
    {
        // This row does not exist yet.
        pair<RowsDataType::iterator, bool> res = maRows.insert(
            RowsDataType::value_type(nRow, RowDataType()));

        if (!res.second)
            return;

        itrRow = res.first;
    }

    // Insert this token into the specified column location.  I don't need to
    // check for existing data.  Just overwrite it.
    RowDataType& rRow = itrRow->second;
    ScExternalRefCache::Cell aCell;
    aCell.mxToken = pToken;
    aCell.mnFmtIndex = nFmtIndex;
    rRow.insert(RowDataType::value_type(nCol, aCell));
    if (bSetCacheRange)
        setCachedCell(nCol, nRow);
}

ScExternalRefCache::TokenRef ScExternalRefCache::Table::getCell(SCCOL nCol, SCROW nRow, sal_uInt32* pnFmtIndex) const
{
    RowsDataType::const_iterator itrTable = maRows.find(nRow);
    if (itrTable == maRows.end())
    {
        // this table doesn't have the specified row.
        return getEmptyOrNullToken(nCol, nRow);
    }

    const RowDataType& rRowData = itrTable->second;
    RowDataType::const_iterator itrRow = rRowData.find(nCol);
    if (itrRow == rRowData.end())
    {
        // this row doesn't have the specified column.
        return getEmptyOrNullToken(nCol, nRow);
    }

    const Cell& rCell = itrRow->second;
    if (pnFmtIndex)
        *pnFmtIndex = rCell.mnFmtIndex;

    return rCell.mxToken;
}

bool ScExternalRefCache::Table::hasRow( SCROW nRow ) const
{
    RowsDataType::const_iterator itrRow = maRows.find(nRow);
    return itrRow != maRows.end();
}

void ScExternalRefCache::Table::getAllRows(vector<SCROW>& rRows, SCROW nLow, SCROW nHigh) const
{
    vector<SCROW> aRows;
    aRows.reserve(maRows.size());
    RowsDataType::const_iterator itr = maRows.begin(), itrEnd = maRows.end();
    for (; itr != itrEnd; ++itr)
        if (nLow <= itr->first && itr->first <= nHigh)
            aRows.push_back(itr->first);

    // hash map is not ordered, so we need to explicitly sort it.
    ::std::sort(aRows.begin(), aRows.end());
    rRows.swap(aRows);
}

::std::pair< SCROW, SCROW > ScExternalRefCache::Table::getRowRange() const
{
    ::std::pair< SCROW, SCROW > aRange( 0, 0 );
    if( !maRows.empty() )
    {
        // iterate over entire container (hash map is not sorted by key)
        RowsDataType::const_iterator itr = maRows.begin(), itrEnd = maRows.end();
        aRange.first = itr->first;
        aRange.second = itr->first + 1;
        while( ++itr != itrEnd )
        {
            if( itr->first < aRange.first )
                aRange.first = itr->first;
            else if( itr->first >= aRange.second )
                aRange.second = itr->first + 1;
        }
    }
    return aRange;
}

void ScExternalRefCache::Table::getAllCols(SCROW nRow, vector<SCCOL>& rCols, SCCOL nLow, SCCOL nHigh) const
{
    RowsDataType::const_iterator itrRow = maRows.find(nRow);
    if (itrRow == maRows.end())
        // this table doesn't have the specified row.
        return;

    const RowDataType& rRowData = itrRow->second;
    vector<SCCOL> aCols;
    aCols.reserve(rRowData.size());
    RowDataType::const_iterator itrCol = rRowData.begin(), itrColEnd = rRowData.end();
    for (; itrCol != itrColEnd; ++itrCol)
        if (nLow <= itrCol->first && itrCol->first <= nHigh)
            aCols.push_back(itrCol->first);

    // hash map is not ordered, so we need to explicitly sort it.
    ::std::sort(aCols.begin(), aCols.end());
    rCols.swap(aCols);
}

::std::pair< SCCOL, SCCOL > ScExternalRefCache::Table::getColRange( SCROW nRow ) const
{
    ::std::pair< SCCOL, SCCOL > aRange( 0, 0 );

    RowsDataType::const_iterator itrRow = maRows.find( nRow );
    if (itrRow == maRows.end())
        // this table doesn't have the specified row.
        return aRange;

    const RowDataType& rRowData = itrRow->second;
    if( !rRowData.empty() )
    {
        // iterate over entire container (hash map is not sorted by key)
        RowDataType::const_iterator itr = rRowData.begin(), itrEnd = rRowData.end();
        aRange.first = itr->first;
        aRange.second = itr->first + 1;
        while( ++itr != itrEnd )
        {
            if( itr->first < aRange.first )
                aRange.first = itr->first;
            else if( itr->first >= aRange.second )
                aRange.second = itr->first + 1;
        }
    }
    return aRange;
}

void ScExternalRefCache::Table::getAllNumberFormats(vector<sal_uInt32>& rNumFmts) const
{
    RowsDataType::const_iterator itrRow = maRows.begin(), itrRowEnd = maRows.end();
    for (; itrRow != itrRowEnd; ++itrRow)
    {
        const RowDataType& rRowData = itrRow->second;
        RowDataType::const_iterator itrCol = rRowData.begin(), itrColEnd = rRowData.end();
        for (; itrCol != itrColEnd; ++itrCol)
        {
            const Cell& rCell = itrCol->second;
            rNumFmts.push_back(rCell.mnFmtIndex);
        }
    }
}

bool ScExternalRefCache::Table::isRangeCached(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    return maCachedRanges.In(ScRange(nCol1, nRow1, 0, nCol2, nRow2, 0));
}

void ScExternalRefCache::Table::setCachedCell(SCCOL nCol, SCROW nRow)
{
    setCachedCellRange(nCol, nRow, nCol, nRow);
}

void ScExternalRefCache::Table::setCachedCellRange(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    ScRange aRange(nCol1, nRow1, 0, nCol2, nRow2, 0);
    if ( maCachedRanges.empty() )
        maCachedRanges.Append(aRange);
    else
        maCachedRanges.Join(aRange);
}

void ScExternalRefCache::Table::setWholeTableCached()
{
    setCachedCellRange(0, 0, MAXCOL, MAXROW);
}

bool ScExternalRefCache::Table::isInCachedRanges(SCCOL nCol, SCROW nRow) const
{
    return maCachedRanges.In(ScRange(nCol, nRow, 0, nCol, nRow, 0));
}

ScExternalRefCache::TokenRef ScExternalRefCache::Table::getEmptyOrNullToken(
    SCCOL nCol, SCROW nRow) const
{
    if (isInCachedRanges(nCol, nRow))
    {
        TokenRef p(new ScEmptyCellToken(false, false));
        return p;
    }
    return TokenRef();
}

ScExternalRefCache::TableName::TableName(const OUString& rUpper, const OUString& rReal) :
    maUpperName(rUpper), maRealName(rReal)
{
}

ScExternalRefCache::CellFormat::CellFormat() :
    mbIsSet(false), mnType(css::util::NumberFormat::ALL), mnIndex(0)
{
}

ScExternalRefCache::ScExternalRefCache() {}

ScExternalRefCache::~ScExternalRefCache() {}

const OUString* ScExternalRefCache::getRealTableName(sal_uInt16 nFileId, const OUString& rTabName) const
{
    osl::MutexGuard aGuard(&maMtxDocs);

    DocDataType::const_iterator itrDoc = maDocs.find(nFileId);
    if (itrDoc == maDocs.end())
    {
        // specified document is not cached.
        return nullptr;
    }

    const DocItem& rDoc = itrDoc->second;
    TableNameIndexMap::const_iterator itrTabId = rDoc.findTableNameIndex( rTabName);
    if (itrTabId == rDoc.maTableNameIndex.end())
    {
        // the specified table is not in cache.
        return nullptr;
    }

    return &rDoc.maTableNames[itrTabId->second].maRealName;
}

const OUString* ScExternalRefCache::getRealRangeName(sal_uInt16 nFileId, const OUString& rRangeName) const
{
    osl::MutexGuard aGuard(&maMtxDocs);

    DocDataType::const_iterator itrDoc = maDocs.find(nFileId);
    if (itrDoc == maDocs.end())
    {
        // specified document is not cached.
        return nullptr;
    }

    const DocItem& rDoc = itrDoc->second;
    NamePairMap::const_iterator itr = rDoc.maRealRangeNameMap.find(
        ScGlobal::pCharClass->uppercase(rRangeName));
    if (itr == rDoc.maRealRangeNameMap.end())
        // range name not found.
        return nullptr;

    return &itr->second;
}

ScExternalRefCache::TokenRef ScExternalRefCache::getCellData(
    sal_uInt16 nFileId, const OUString& rTabName, SCCOL nCol, SCROW nRow, sal_uInt32* pnFmtIndex)
{
    osl::MutexGuard aGuard(&maMtxDocs);

    DocDataType::const_iterator itrDoc = maDocs.find(nFileId);
    if (itrDoc == maDocs.end())
    {
        // specified document is not cached.
        return TokenRef();
    }

    const DocItem& rDoc = itrDoc->second;
    TableNameIndexMap::const_iterator itrTabId = rDoc.findTableNameIndex( rTabName);
    if (itrTabId == rDoc.maTableNameIndex.end())
    {
        // the specified table is not in cache.
        return TokenRef();
    }

    const TableTypeRef& pTableData = rDoc.maTables[itrTabId->second];
    if (!pTableData.get())
    {
        // the table data is not instantiated yet.
        return TokenRef();
    }

    return pTableData->getCell(nCol, nRow, pnFmtIndex);
}

ScExternalRefCache::TokenArrayRef ScExternalRefCache::getCellRangeData(
    sal_uInt16 nFileId, const OUString& rTabName, const ScRange& rRange)
{
    osl::MutexGuard aGuard(&maMtxDocs);

    DocDataType::iterator itrDoc = maDocs.find(nFileId);
    if (itrDoc == maDocs.end())
        // specified document is not cached.
        return TokenArrayRef();

    DocItem& rDoc = itrDoc->second;

    TableNameIndexMap::const_iterator itrTabId = rDoc.findTableNameIndex( rTabName);
    if (itrTabId == rDoc.maTableNameIndex.end())
        // the specified table is not in cache.
        return TokenArrayRef();

    const ScAddress& s = rRange.aStart;
    const ScAddress& e = rRange.aEnd;

    const SCTAB nTab1 = s.Tab(), nTab2 = e.Tab();
    const SCCOL nCol1 = s.Col(), nCol2 = e.Col();
    const SCROW nRow1 = s.Row(), nRow2 = e.Row();

    // Make sure I have all the tables cached.
    size_t nTabFirstId = itrTabId->second;
    size_t nTabLastId  = nTabFirstId + nTab2 - nTab1;
    if (nTabLastId >= rDoc.maTables.size())
        // not all tables are cached.
        return TokenArrayRef();

    ScRange aCacheRange( nCol1, nRow1, static_cast<SCTAB>(nTabFirstId), nCol2, nRow2, static_cast<SCTAB>(nTabLastId));

    RangeArrayMap::const_iterator itrRange = rDoc.maRangeArrays.find( aCacheRange);
    if (itrRange != rDoc.maRangeArrays.end())
        // Cache hit!
        return itrRange->second;

    std::unique_ptr<ScRange> pNewRange;
    TokenArrayRef pArray;
    bool bFirstTab = true;
    for (size_t nTab = nTabFirstId; nTab <= nTabLastId; ++nTab)
    {
        TableTypeRef pTab = rDoc.maTables[nTab];
        if (!pTab.get())
            return TokenArrayRef();

        SCCOL nDataCol1 = nCol1, nDataCol2 = nCol2;
        SCROW nDataRow1 = nRow1, nDataRow2 = nRow2;

        if (!pTab->isRangeCached(nDataCol1, nDataRow1, nDataCol2, nDataRow2))
        {
            // specified range is not entirely within cached ranges.
            return TokenArrayRef();
        }

        SCSIZE nMatrixColumns = static_cast<SCSIZE>(nDataCol2-nDataCol1+1);
        SCSIZE nMatrixRows = static_cast<SCSIZE>(nDataRow2-nDataRow1+1);
        ScMatrixRef xMat = new ScFullMatrix( nMatrixColumns, nMatrixRows);

        // Needed in shrink and fill.
        vector<SCROW> aRows;
        pTab->getAllRows(aRows, nDataRow1, nDataRow2);
        bool bFill = true;

        // Check if size could be allocated and if not skip the fill, there's
        // one error element instead. But retry first with the actual data area
        // if that is smaller than the original range, which works for most
        // functions just not some that operate/compare with the original size
        // and expect empty values in non-data areas.
        // Restrict this though to ranges of entire columns or rows, other
        // ranges might be on purpose. (Other special cases to handle?)
        /* TODO: sparse matrix could help */
        SCSIZE nMatCols, nMatRows;
        xMat->GetDimensions( nMatCols, nMatRows);
        if (nMatCols != nMatrixColumns || nMatRows != nMatrixRows)
        {
            bFill = false;
            if (aRows.empty())
            {
                // There's no data at all. Set the one matrix element to empty
                // for column-repeated and row-repeated access.
                xMat->PutEmpty(0,0);
            }
            else if ((nCol1 == 0 && nCol2 == MAXCOL) || (nRow1 == 0 && nRow2 == MAXROW))
            {
                nDataRow1 = aRows.front();
                nDataRow2 = aRows.back();
                SCCOL nMinCol = std::numeric_limits<SCCOL>::max();
                SCCOL nMaxCol = std::numeric_limits<SCCOL>::min();
                for (vector<SCROW>::const_iterator itr = aRows.begin(), itrEnd = aRows.end(); itr != itrEnd; ++itr)
                {
                    vector<SCCOL> aCols;
                    pTab->getAllCols(*itr, aCols, nDataCol1, nDataCol2);
                    if (!aCols.empty())
                    {
                        nMinCol = std::min( nMinCol, aCols.front());
                        nMaxCol = std::max( nMaxCol, aCols.back());
                    }
                }

                if (nMinCol <= nMaxCol && ((static_cast<SCSIZE>(nMaxCol-nMinCol+1) < nMatrixColumns) ||
                            (static_cast<SCSIZE>(nDataRow2-nDataRow1+1) < nMatrixRows)))
                {
                    nMatrixColumns = static_cast<SCSIZE>(nMaxCol-nMinCol+1);
                    nMatrixRows = static_cast<SCSIZE>(nDataRow2-nDataRow1+1);
                    xMat = new ScFullMatrix( nMatrixColumns, nMatrixRows);
                    xMat->GetDimensions( nMatCols, nMatRows);
                    if (nMatCols == nMatrixColumns && nMatRows == nMatrixRows)
                    {
                        nDataCol1 = nMinCol;
                        nDataCol2 = nMaxCol;
                        bFill = true;
                    }
                }
            }
        }

        if (bFill)
        {
            // Only fill non-empty cells, for better performance.
            for (vector<SCROW>::const_iterator itr = aRows.begin(), itrEnd = aRows.end(); itr != itrEnd; ++itr)
            {
                SCROW nRow = *itr;
                vector<SCCOL> aCols;
                pTab->getAllCols(nRow, aCols, nDataCol1, nDataCol2);
                for (vector<SCCOL>::const_iterator itrCol = aCols.begin(), itrColEnd = aCols.end(); itrCol != itrColEnd; ++itrCol)
                {
                    SCCOL nCol = *itrCol;
                    TokenRef pToken = pTab->getCell(nCol, nRow);
                    if (!pToken)
                        // This should never happen!
                        return TokenArrayRef();

                    SCSIZE nC = nCol - nDataCol1, nR = nRow - nDataRow1;
                    switch (pToken->GetType())
                    {
                        case svDouble:
                            xMat->PutDouble(pToken->GetDouble(), nC, nR);
                            break;
                        case svString:
                            xMat->PutString(pToken->GetString(), nC, nR);
                            break;
                        default:
                            ;
                    }
                }
            }

            if (!bFirstTab)
                pArray->AddOpCode(ocSep);

            ScMatrixToken aToken(xMat);
            if (!pArray)
                pArray.reset(new ScTokenArray);
            pArray->AddToken(aToken);

            bFirstTab = false;

            if (!pNewRange)
                pNewRange.reset(new ScRange(nDataCol1, nDataRow1, nTab, nDataCol2, nDataRow2, nTab));
            else
                pNewRange->ExtendTo(ScRange(nDataCol1, nDataRow1, nTab, nDataCol2, nDataRow2, nTab));
        }
    }

    rDoc.maRangeArrays.insert( RangeArrayMap::value_type(aCacheRange, pArray));
    if (pNewRange && *pNewRange != aCacheRange)
        rDoc.maRangeArrays.insert( RangeArrayMap::value_type(*pNewRange, pArray));

    return pArray;
}

ScExternalRefCache::TokenArrayRef ScExternalRefCache::getRangeNameTokens(sal_uInt16 nFileId, const OUString& rName)
{
    osl::MutexGuard aGuard(&maMtxDocs);

    DocItem* pDoc = getDocItem(nFileId);
    if (!pDoc)
        return TokenArrayRef();

    RangeNameMap& rMap = pDoc->maRangeNames;
    RangeNameMap::const_iterator itr = rMap.find(
        ScGlobal::pCharClass->uppercase(rName));
    if (itr == rMap.end())
        return TokenArrayRef();

    return itr->second;
}

void ScExternalRefCache::setRangeNameTokens(sal_uInt16 nFileId, const OUString& rName, TokenArrayRef pArray)
{
    osl::MutexGuard aGuard(&maMtxDocs);

    DocItem* pDoc = getDocItem(nFileId);
    if (!pDoc)
        return;

    OUString aUpperName = ScGlobal::pCharClass->uppercase(rName);
    RangeNameMap& rMap = pDoc->maRangeNames;
    rMap.insert(RangeNameMap::value_type(aUpperName, pArray));
    pDoc->maRealRangeNameMap.insert(NamePairMap::value_type(aUpperName, rName));
}

bool ScExternalRefCache::isValidRangeName(sal_uInt16 nFileId, const OUString& rName) const
{
    osl::MutexGuard aGuard(&maMtxDocs);

    DocItem* pDoc = getDocItem(nFileId);
    if (!pDoc)
        return false;

    const RangeNameMap& rMap = pDoc->maRangeNames;
    return rMap.count(rName) > 0;
}

void ScExternalRefCache::setRangeName(sal_uInt16 nFileId, const OUString& rName)
{
    osl::MutexGuard aGuard(&maMtxDocs);

    DocItem* pDoc = getDocItem(nFileId);
    if (!pDoc)
        return;

    OUString aUpperName = ScGlobal::pCharClass->uppercase(rName);
    pDoc->maRealRangeNameMap.insert(NamePairMap::value_type(aUpperName, rName));
}

void ScExternalRefCache::setCellData(sal_uInt16 nFileId, const OUString& rTabName, SCCOL nCol, SCROW nRow,
                                     TokenRef pToken, sal_uLong nFmtIndex)
{
    if (!isDocInitialized(nFileId))
        return;

    using ::std::pair;
    DocItem* pDocItem = getDocItem(nFileId);
    if (!pDocItem)
        return;

    DocItem& rDoc = *pDocItem;

    // See if the table by this name already exists.
    TableNameIndexMap::const_iterator itrTabName = rDoc.findTableNameIndex( rTabName);
    if (itrTabName == rDoc.maTableNameIndex.end())
        // Table not found.  Maybe the table name or the file id is wrong ???
        return;

    TableTypeRef& pTableData = rDoc.maTables[itrTabName->second];
    if (!pTableData.get())
        pTableData.reset(new Table);

    pTableData->setCell(nCol, nRow, pToken, nFmtIndex);
    pTableData->setCachedCell(nCol, nRow);
}

void ScExternalRefCache::setCellRangeData(sal_uInt16 nFileId, const ScRange& rRange, const vector<SingleRangeData>& rData,
                                          const TokenArrayRef& pArray)
{
    using ::std::pair;
    if (rData.empty() || !isDocInitialized(nFileId))
        // nothing to cache
        return;

    // First, get the document item for the given file ID.
    DocItem* pDocItem = getDocItem(nFileId);
    if (!pDocItem)
        return;

    DocItem& rDoc = *pDocItem;

    // Now, find the table position of the first table to cache.
    const OUString& rFirstTabName = rData.front().maTableName;
    TableNameIndexMap::const_iterator itrTabName = rDoc.findTableNameIndex( rFirstTabName);
    if (itrTabName == rDoc.maTableNameIndex.end())
    {
        // table index not found.
        return;
    }

    size_t nTabFirstId = itrTabName->second;
    SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();
    SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
    vector<SingleRangeData>::const_iterator itrDataBeg = rData.begin(), itrDataEnd = rData.end();
    for (vector<SingleRangeData>::const_iterator itrData = itrDataBeg; itrData != itrDataEnd; ++itrData)
    {
        size_t i = nTabFirstId + ::std::distance(itrDataBeg, itrData);
        TableTypeRef& pTabData = rDoc.maTables[i];
        if (!pTabData.get())
            pTabData.reset(new Table);

        const ScMatrixRef& pMat = itrData->mpRangeData;
        SCSIZE nMatCols, nMatRows;
        pMat->GetDimensions( nMatCols, nMatRows);
        if (nMatCols > static_cast<SCSIZE>(nCol2 - nCol1) && nMatRows > static_cast<SCSIZE>(nRow2 - nRow1))
        {
            for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
            {
                const SCSIZE nR = nRow - nRow1;
                for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
                {
                    const SCSIZE nC = nCol - nCol1;

                    ScMatrixValue value = pMat->Get(nC, nR);

                    TokenRef pToken;

                    switch (value.nType) {
                        case SC_MATVAL_VALUE:
                        case SC_MATVAL_BOOLEAN:
                            pToken.reset(new formula::FormulaDoubleToken(value.fVal));
                            break;
                        case SC_MATVAL_STRING:
                            pToken.reset(new formula::FormulaStringToken(value.aStr));
                            break;
                        default:
                            // Don't cache empty cells.
                            break;
                    }

                    if (pToken)
                        // Don't mark this cell 'cached' here, for better performance.
                        pTabData->setCell(nCol, nRow, pToken, 0, false);
                }
            }
            // Mark the whole range 'cached'.
            pTabData->setCachedCellRange(nCol1, nRow1, nCol2, nRow2);
        }
        else
        {
            // This may happen due to a matrix not been allocated earlier, in
            // which case it should have exactly one error element.
            SAL_WARN("sc.ui","ScExternalRefCache::setCellRangeData - matrix size mismatch");
            if (nMatCols != 1 || nMatRows != 1)
                SAL_WARN("sc.ui","ScExternalRefCache::setCellRangeData - not a one element matrix");
            else
            {
                sal_uInt16 nErr = GetDoubleErrorValue( pMat->GetDouble(0,0));
                SAL_WARN("sc.ui","ScExternalRefCache::setCellRangeData - matrix error value is " << nErr <<
                        (nErr ? ", ok" : ", not ok"));
            }
        }
    }

    size_t nTabLastId = nTabFirstId + rRange.aEnd.Tab() - rRange.aStart.Tab();
    ScRange aCacheRange( nCol1, nRow1, static_cast<SCTAB>(nTabFirstId), nCol2, nRow2, static_cast<SCTAB>(nTabLastId));

    rDoc.maRangeArrays.insert( RangeArrayMap::value_type( aCacheRange, pArray));
}

bool ScExternalRefCache::isDocInitialized(sal_uInt16 nFileId)
{
    DocItem* pDoc = getDocItem(nFileId);
    if (!pDoc)
        return false;

    return pDoc->mbInitFromSource;
}

static bool lcl_getStrictTableDataIndex(const ScExternalRefCache::TableNameIndexMap& rMap, const OUString& rName, size_t& rIndex)
{
    ScExternalRefCache::TableNameIndexMap::const_iterator itr = rMap.find(rName);
    if (itr == rMap.end())
        return false;

    rIndex = itr->second;
    return true;
}

bool ScExternalRefCache::DocItem::getTableDataIndex( const OUString& rTabName, size_t& rIndex ) const
{
    ScExternalRefCache::TableNameIndexMap::const_iterator itr = findTableNameIndex(rTabName);
    if (itr == maTableNameIndex.end())
        return false;

    rIndex = itr->second;
    return true;
}

namespace {
OUString getFirstSheetName()
{
    // Get Custom prefix.
    const ScDefaultsOptions& rOpt = SC_MOD()->GetDefaultsOptions();
    // Form sheet name identical to the first generated sheet name when
    // creating an internal document, e.g. 'Sheet1'.
    return rOpt.GetInitTabPrefix() + "1";
}
}

void ScExternalRefCache::initializeDoc(sal_uInt16 nFileId, const vector<OUString>& rTabNames,
        const OUString& rBaseName)
{
    DocItem* pDoc = getDocItem(nFileId);
    if (!pDoc)
        return;

    size_t n = rTabNames.size();

    // table name list - the list must include all table names in the source
    // document and only to be populated when loading the source document, not
    // when loading cached data from, say, Excel XCT/CRN records.
    vector<TableName> aNewTabNames;
    aNewTabNames.reserve(n);
    for (vector<OUString>::const_iterator itr = rTabNames.begin(), itrEnd = rTabNames.end();
          itr != itrEnd; ++itr)
    {
        TableName aNameItem(ScGlobal::pCharClass->uppercase(*itr), *itr);
        aNewTabNames.push_back(aNameItem);
    }
    pDoc->maTableNames.swap(aNewTabNames);

    // data tables - preserve any existing data that may have been set during
    // file import.
    vector<TableTypeRef> aNewTables(n);
    for (size_t i = 0; i < n; ++i)
    {
        size_t nIndex;
        if (lcl_getStrictTableDataIndex(pDoc->maTableNameIndex, pDoc->maTableNames[i].maUpperName, nIndex))
        {
            aNewTables[i] = pDoc->maTables[nIndex];
        }
    }
    pDoc->maTables.swap(aNewTables);

    // name index map
    TableNameIndexMap aNewNameIndex;
    for (size_t i = 0; i < n; ++i)
        aNewNameIndex.insert(TableNameIndexMap::value_type(pDoc->maTableNames[i].maUpperName, i));
    pDoc->maTableNameIndex.swap(aNewNameIndex);

    // Setup name for Sheet1 vs base name to be able to load documents
    // that store the base name as table name, or vice versa.
    pDoc->maSingleTableNameAlias.clear();
    if (!rBaseName.isEmpty() && pDoc->maTableNames.size() == 1)
    {
        OUString aSheetName = getFirstSheetName();
        // If the one and only table name matches exactly, carry on the base
        // file name for further alias use. If instead the table name matches
        // the base name, carry on the sheet name as alias.
        if (ScGlobal::GetpTransliteration()->isEqual( pDoc->maTableNames[0].maRealName, aSheetName))
            pDoc->maSingleTableNameAlias = rBaseName;
        else if (ScGlobal::GetpTransliteration()->isEqual( pDoc->maTableNames[0].maRealName, rBaseName))
            pDoc->maSingleTableNameAlias = aSheetName;
    }

    pDoc->mbInitFromSource = true;
}

ScExternalRefCache::TableNameIndexMap::const_iterator ScExternalRefCache::DocItem::findTableNameIndex(
        const OUString& rTabName ) const
{
    const OUString aTabNameUpper = ScGlobal::pCharClass->uppercase( rTabName);
    TableNameIndexMap::const_iterator itrTabName = maTableNameIndex.find( aTabNameUpper);
    if (itrTabName != maTableNameIndex.end())
        return itrTabName;

    // Since some time for external references to CSV files the base name is
    // used as sheet name instead of Sheet1, check if we can resolve that.
    // Also helps users that got accustomed to one or the other way.
    if (maSingleTableNameAlias.isEmpty() || maTableNameIndex.size() != 1)
        return itrTabName;

    // maSingleTableNameAlias has been set up only if the original file loaded
    // had exactly one sheet and internal sheet name was Sheet1 or localized or
    // customized equivalent, or base name.
    if (aTabNameUpper == ScGlobal::pCharClass->uppercase( maSingleTableNameAlias))
        return maTableNameIndex.begin();

    return itrTabName;
}

bool ScExternalRefCache::DocItem::getSingleTableNameAlternative( OUString& rTabName ) const
{
    if (maSingleTableNameAlias.isEmpty() || maTableNames.size() != 1)
        return false;
    if (ScGlobal::GetpTransliteration()->isEqual( rTabName, maTableNames[0].maRealName))
    {
        rTabName = maSingleTableNameAlias;
        return true;
    }
    if (ScGlobal::GetpTransliteration()->isEqual( rTabName, maSingleTableNameAlias))
    {
        rTabName = maTableNames[0].maRealName;
        return true;
    }
    return false;
}

bool ScExternalRefCache::getSrcDocTable( const ScDocument& rSrcDoc, const OUString& rTabName, SCTAB& rTab,
        sal_uInt16 nFileId ) const
{
    bool bFound = rSrcDoc.GetTable( rTabName, rTab);
    if (!bFound)
    {
        // Check the one table alias alternative.
        const DocItem* pDoc = getDocItem( nFileId );
        if (pDoc)
        {
            OUString aTabName( rTabName);
            if (pDoc->getSingleTableNameAlternative( aTabName))
                bFound = rSrcDoc.GetTable( aTabName, rTab);
        }
    }
    return bFound;
}

OUString ScExternalRefCache::getTableName(sal_uInt16 nFileId, size_t nCacheId) const
{
    if( DocItem* pDoc = getDocItem( nFileId ) )
        if( nCacheId < pDoc->maTableNames.size() )
            return pDoc->maTableNames[ nCacheId ].maRealName;
    return EMPTY_OUSTRING;
}

void ScExternalRefCache::getAllTableNames(sal_uInt16 nFileId, vector<OUString>& rTabNames) const
{
    rTabNames.clear();
    DocItem* pDoc = getDocItem(nFileId);
    if (!pDoc)
        return;

    size_t n = pDoc->maTableNames.size();
    rTabNames.reserve(n);
    for (vector<TableName>::const_iterator itr = pDoc->maTableNames.begin(), itrEnd = pDoc->maTableNames.end();
          itr != itrEnd; ++itr)
        rTabNames.push_back(itr->maRealName);
}

SCsTAB ScExternalRefCache::getTabSpan( sal_uInt16 nFileId, const OUString& rStartTabName, const OUString& rEndTabName ) const
{
    DocItem* pDoc = getDocItem(nFileId);
    if (!pDoc)
        return -1;

    vector<TableName>::const_iterator itrBeg = pDoc->maTableNames.begin();
    vector<TableName>::const_iterator itrEnd = pDoc->maTableNames.end();

    vector<TableName>::const_iterator itrStartTab = ::std::find_if( itrBeg, itrEnd,
            TabNameSearchPredicate( rStartTabName));
    if (itrStartTab == itrEnd)
        return -1;

    vector<TableName>::const_iterator itrEndTab = ::std::find_if( itrBeg, itrEnd,
            TabNameSearchPredicate( rEndTabName));
    if (itrEndTab == itrEnd)
        return 0;

    size_t nStartDist = ::std::distance( itrBeg, itrStartTab);
    size_t nEndDist = ::std::distance( itrBeg, itrEndTab);
    return nStartDist <= nEndDist ? static_cast<SCsTAB>(nEndDist - nStartDist + 1) : -static_cast<SCsTAB>(nStartDist - nEndDist + 1);
}

void ScExternalRefCache::getAllNumberFormats(vector<sal_uInt32>& rNumFmts) const
{
    osl::MutexGuard aGuard(&maMtxDocs);

    using ::std::sort;
    using ::std::unique;

    vector<sal_uInt32> aNumFmts;
    for (DocDataType::const_iterator itrDoc = maDocs.begin(), itrDocEnd = maDocs.end();
          itrDoc != itrDocEnd; ++itrDoc)
    {
        const vector<TableTypeRef>& rTables = itrDoc->second.maTables;
        for (vector<TableTypeRef>::const_iterator itrTab = rTables.begin(), itrTabEnd = rTables.end();
              itrTab != itrTabEnd; ++itrTab)
        {
            TableTypeRef pTab = *itrTab;
            if (!pTab)
                continue;

            pTab->getAllNumberFormats(aNumFmts);
        }
    }

    // remove duplicates.
    sort(aNumFmts.begin(), aNumFmts.end());
    aNumFmts.erase(unique(aNumFmts.begin(), aNumFmts.end()), aNumFmts.end());
    rNumFmts.swap(aNumFmts);
}

bool ScExternalRefCache::setCacheDocReferenced( sal_uInt16 nFileId )
{
    DocItem* pDocItem = getDocItem(nFileId);
    if (!pDocItem)
        return areAllCacheTablesReferenced();

    for (::std::vector<TableTypeRef>::iterator itrTab = pDocItem->maTables.begin();
            itrTab != pDocItem->maTables.end(); ++itrTab)
    {
        if ((*itrTab).get())
            (*itrTab)->setReferenced( true);
    }
    addCacheDocToReferenced( nFileId);
    return areAllCacheTablesReferenced();
}

bool ScExternalRefCache::setCacheTableReferenced( sal_uInt16 nFileId, const OUString& rTabName, size_t nSheets, bool bPermanent )
{
    DocItem* pDoc = getDocItem(nFileId);
    if (pDoc)
    {
        size_t nIndex = 0;
        if (pDoc->getTableDataIndex( rTabName, nIndex))
        {
            size_t nStop = ::std::min( nIndex + nSheets, pDoc->maTables.size());
            for (size_t i = nIndex; i < nStop; ++i)
            {
                TableTypeRef pTab = pDoc->maTables[i];
                if (pTab.get())
                {
                    Table::ReferencedFlag eNewFlag = (bPermanent ?
                            Table::REFERENCED_PERMANENT :
                            Table::REFERENCED_MARKED);
                    Table::ReferencedFlag eOldFlag = pTab->getReferencedFlag();
                    if (eOldFlag != Table::REFERENCED_PERMANENT && eNewFlag != eOldFlag)
                    {
                        pTab->setReferencedFlag( eNewFlag);
                        addCacheTableToReferenced( nFileId, i);
                    }
                }
            }
        }
    }
    return areAllCacheTablesReferenced();
}

void ScExternalRefCache::setAllCacheTableReferencedStati( bool bReferenced )
{
    osl::MutexGuard aGuard(&maMtxDocs);

    if (bReferenced)
    {
        maReferenced.reset(0);
        for (DocDataType::iterator itrDoc = maDocs.begin(); itrDoc != maDocs.end(); ++itrDoc)
        {
            ScExternalRefCache::DocItem& rDocItem = (*itrDoc).second;
            for (::std::vector<TableTypeRef>::iterator itrTab = rDocItem.maTables.begin();
                    itrTab != rDocItem.maTables.end(); ++itrTab)
            {
                if ((*itrTab).get())
                    (*itrTab)->setReferenced( true);
            }
        }
    }
    else
    {
        size_t nDocs = 0;
        for (DocDataType::const_iterator itrDoc = maDocs.begin(); itrDoc != maDocs.end(); ++itrDoc)
        {
            if (nDocs <= (*itrDoc).first)
                nDocs  = (*itrDoc).first + 1;
        }
        maReferenced.reset( nDocs);

        for (DocDataType::iterator itrDoc = maDocs.begin(); itrDoc != maDocs.end(); ++itrDoc)
        {
            ScExternalRefCache::DocItem& rDocItem = (*itrDoc).second;
            sal_uInt16 nFileId = (*itrDoc).first;
            size_t nTables = rDocItem.maTables.size();
            ReferencedStatus::DocReferenced & rDocReferenced = maReferenced.maDocs[nFileId];
            // All referenced => non-existing tables evaluate as completed.
            rDocReferenced.maTables.resize( nTables, true);
            for (size_t i=0; i < nTables; ++i)
            {
                TableTypeRef & xTab = rDocItem.maTables[i];
                if (xTab.get())
                {
                    if (xTab->getReferencedFlag() == Table::REFERENCED_PERMANENT)
                        addCacheTableToReferenced( nFileId, i);
                    else
                    {
                        xTab->setReferencedFlag( Table::UNREFERENCED);
                        rDocReferenced.maTables[i] = false;
                        rDocReferenced.mbAllTablesReferenced = false;
                        // An addCacheTableToReferenced() actually may have
                        // resulted in mbAllReferenced been set. Clear it.
                        maReferenced.mbAllReferenced = false;
                    }
                }
            }
        }
    }
}

void ScExternalRefCache::addCacheTableToReferenced( sal_uInt16 nFileId, size_t nIndex )
{
    if (nFileId >= maReferenced.maDocs.size())
        return;

    ::std::vector<bool> & rTables = maReferenced.maDocs[nFileId].maTables;
    size_t nTables = rTables.size();
    if (nIndex >= nTables)
        return;

    if (!rTables[nIndex])
    {
        rTables[nIndex] = true;
        size_t i = 0;
        while (i < nTables && rTables[i])
            ++i;
        if (i == nTables)
        {
            maReferenced.maDocs[nFileId].mbAllTablesReferenced = true;
            maReferenced.checkAllDocs();
        }
    }
}

void ScExternalRefCache::addCacheDocToReferenced( sal_uInt16 nFileId )
{
    if (nFileId >= maReferenced.maDocs.size())
        return;

    if (!maReferenced.maDocs[nFileId].mbAllTablesReferenced)
    {
        ::std::vector<bool> & rTables = maReferenced.maDocs[nFileId].maTables;
        size_t nSize = rTables.size();
        for (size_t i=0; i < nSize; ++i)
            rTables[i] = true;
        maReferenced.maDocs[nFileId].mbAllTablesReferenced = true;
        maReferenced.checkAllDocs();
    }
}

void ScExternalRefCache::getAllCachedDataSpans( sal_uInt16 nFileId, sc::ColumnSpanSet& rSet ) const
{
    const DocItem* pDocItem = getDocItem(nFileId);
    if (!pDocItem)
        // This document is not cached.
        return;

    const std::vector<TableTypeRef>& rTables = pDocItem->maTables;
    for (size_t nTab = 0, nTabCount = rTables.size(); nTab < nTabCount; ++nTab)
    {
        TableTypeRef pTab = rTables[nTab];
        if (!pTab)
            continue;

        std::vector<SCROW> aRows;
        pTab->getAllRows(aRows);
        std::vector<SCROW>::const_iterator itRow = aRows.begin(), itRowEnd = aRows.end();
        for (; itRow != itRowEnd; ++itRow)
        {
            SCROW nRow = *itRow;
            std::vector<SCCOL> aCols;
            pTab->getAllCols(nRow, aCols);
            std::vector<SCCOL>::const_iterator itCol = aCols.begin(), itColEnd = aCols.end();
            for (; itCol != itColEnd; ++itCol)
            {
                SCCOL nCol = *itCol;
                rSet.set(nTab, nCol, nRow, true);
            }
        }
    }
}

ScExternalRefCache::ReferencedStatus::ReferencedStatus() :
    mbAllReferenced(false)
{
    reset(0);
}

void ScExternalRefCache::ReferencedStatus::reset( size_t nDocs )
{
    if (nDocs)
    {
        mbAllReferenced = false;
        DocReferencedVec aRefs( nDocs);
        maDocs.swap( aRefs);
    }
    else
    {
        mbAllReferenced = true;
        DocReferencedVec aRefs;
        maDocs.swap( aRefs);
    }
}

void ScExternalRefCache::ReferencedStatus::checkAllDocs()
{
    for (DocReferencedVec::const_iterator itr = maDocs.begin(); itr != maDocs.end(); ++itr)
    {
        if (!(*itr).mbAllTablesReferenced)
            return;
    }
    mbAllReferenced = true;
}

ScExternalRefCache::TableTypeRef ScExternalRefCache::getCacheTable(sal_uInt16 nFileId, size_t nTabIndex) const
{
    DocItem* pDoc = getDocItem(nFileId);
    if (!pDoc || nTabIndex >= pDoc->maTables.size())
        return TableTypeRef();

    return pDoc->maTables[nTabIndex];
}

ScExternalRefCache::TableTypeRef ScExternalRefCache::getCacheTable(sal_uInt16 nFileId, const OUString& rTabName,
        bool bCreateNew, size_t* pnIndex, const OUString* pExtUrl)
{
    // In API, the index is transported as cached sheet ID of type sal_Int32 in
    // sheet::SingleReference.Sheet or sheet::ComplexReference.Reference1.Sheet
    // in a sheet::FormulaToken, choose a sensible value for N/A. Effectively
    // being 0xffffffff
    const size_t nNotAvailable = static_cast<size_t>( static_cast<sal_Int32>( -1));

    DocItem* pDoc = getDocItem(nFileId);
    if (!pDoc)
    {
        if (pnIndex) *pnIndex = nNotAvailable;
        return TableTypeRef();
    }

    DocItem& rDoc = *pDoc;

    size_t nIndex;
    if (rDoc.getTableDataIndex(rTabName, nIndex))
    {
        // specified table found.
        if( pnIndex ) *pnIndex = nIndex;
        if (bCreateNew && !rDoc.maTables[nIndex])
            rDoc.maTables[nIndex].reset(new Table);

        return rDoc.maTables[nIndex];
    }

    if (!bCreateNew)
    {
        if (pnIndex) *pnIndex = nNotAvailable;
        return TableTypeRef();
    }

    // If this is the first table to be created propagate the base name or
    // Sheet1 as an alias. For subsequent tables remove it again.
    if (rDoc.maTableNames.empty())
    {
        if (pExtUrl)
        {
            const OUString aBaseName( INetURLObject( *pExtUrl).GetBase());
            const OUString aSheetName( getFirstSheetName());
            if (ScGlobal::GetpTransliteration()->isEqual( rTabName, aSheetName))
                pDoc->maSingleTableNameAlias = aBaseName;
            else if (ScGlobal::GetpTransliteration()->isEqual( rTabName, aBaseName))
                pDoc->maSingleTableNameAlias = aSheetName;
        }
    }
    else
    {
        rDoc.maSingleTableNameAlias.clear();
    }

    // Specified table doesn't exist yet.  Create one.
    OUString aTabNameUpper = ScGlobal::pCharClass->uppercase(rTabName);
    nIndex = rDoc.maTables.size();
    if( pnIndex ) *pnIndex = nIndex;
    TableTypeRef pTab(new Table);
    rDoc.maTables.push_back(pTab);
    rDoc.maTableNames.push_back(TableName(aTabNameUpper, rTabName));
    rDoc.maTableNameIndex.insert(
        TableNameIndexMap::value_type(aTabNameUpper, nIndex));
    return pTab;
}

void ScExternalRefCache::clearCache(sal_uInt16 nFileId)
{
    osl::MutexGuard aGuard(&maMtxDocs);
    maDocs.erase(nFileId);
}

void ScExternalRefCache::clearCacheTables(sal_uInt16 nFileId)
{
    osl::MutexGuard aGuard(&maMtxDocs);
    DocItem* pDocItem = getDocItem(nFileId);
    if (!pDocItem)
        // This document is not cached at all.
        return;

    // Clear all cache table content, but keep the tables.
    std::vector<TableTypeRef>& rTabs = pDocItem->maTables;
    for (size_t i = 0, n = rTabs.size(); i < n; ++i)
    {
        TableTypeRef pTab = rTabs[i];
        if (!pTab)
            continue;

        pTab->clear();
    }

    // Clear the external range name caches.
    pDocItem->maRangeNames.clear();
    pDocItem->maRangeArrays.clear();
    pDocItem->maRealRangeNameMap.clear();
}

ScExternalRefCache::DocItem* ScExternalRefCache::getDocItem(sal_uInt16 nFileId) const
{
    osl::MutexGuard aGuard(&maMtxDocs);

    using ::std::pair;
    DocDataType::iterator itrDoc = maDocs.find(nFileId);
    if (itrDoc == maDocs.end())
    {
        // specified document is not cached.
        pair<DocDataType::iterator, bool> res = maDocs.insert(
                DocDataType::value_type(nFileId, DocItem()));

        if (!res.second)
            // insertion failed.
            return nullptr;

        itrDoc = res.first;
    }

    return &itrDoc->second;
}

ScExternalRefLink::ScExternalRefLink(ScDocument* pDoc, sal_uInt16 nFileId, const OUString& rFilter) :
    ::sfx2::SvBaseLink(::SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SIMPLE_FILE),
    mnFileId(nFileId),
    maFilterName(rFilter),
    mpDoc(pDoc),
    mbDoRefresh(true)
{
}

ScExternalRefLink::~ScExternalRefLink()
{
}

void ScExternalRefLink::Closed()
{
    ScExternalRefManager* pMgr = mpDoc->GetExternalRefManager();
    pMgr->breakLink(mnFileId);
}

::sfx2::SvBaseLink::UpdateResult ScExternalRefLink::DataChanged(const OUString& /*rMimeType*/, const Any& /*rValue*/)
{
    if (!mbDoRefresh)
        return SUCCESS;

    OUString aFile, aFilter;
    sfx2::LinkManager::GetDisplayNames(this, nullptr, &aFile, nullptr, &aFilter);
    ScExternalRefManager* pMgr = mpDoc->GetExternalRefManager();

    if (!pMgr->isFileLoadable(aFile))
        return ERROR_GENERAL;

    const OUString* pCurFile = pMgr->getExternalFileName(mnFileId);
    if (!pCurFile)
        return ERROR_GENERAL;

    if (pCurFile->equals(aFile))
    {
        // Refresh the current source document.
        if (!pMgr->refreshSrcDocument(mnFileId))
            return ERROR_GENERAL;
    }
    else
    {
        // The source document has changed.
        ScDocShell* pDocShell = ScDocShell::GetViewData()->GetDocShell();
        ScDocShellModificator aMod(*pDocShell);
        pMgr->switchSrcFile(mnFileId, aFile, aFilter);
        maFilterName = aFilter;
        aMod.SetDocumentModified();
    }

    return SUCCESS;
}

void ScExternalRefLink::Edit(vcl::Window* pParent, const Link<SvBaseLink&,void>& /*rEndEditHdl*/)
{
    SvBaseLink::Edit(pParent, Link<SvBaseLink&,void>());
}

void ScExternalRefLink::SetDoReferesh(bool b)
{
    mbDoRefresh = b;
}

static FormulaToken* convertToToken( ScDocument* pHostDoc, ScDocument* pSrcDoc, ScRefCellValue& rCell )
{
    if (rCell.hasEmptyValue())
    {
        bool bInherited = (rCell.meType == CELLTYPE_FORMULA);
        return new ScEmptyCellToken(bInherited, false);
    }

    switch (rCell.meType)
    {
        case CELLTYPE_EDIT:
        case CELLTYPE_STRING:
        {
            OUString aStr = rCell.getString(pSrcDoc);
            svl::SharedString aSS = pHostDoc->GetSharedStringPool().intern(aStr);
            return new formula::FormulaStringToken(aSS);
        }
        case CELLTYPE_VALUE:
            return new formula::FormulaDoubleToken(rCell.mfValue);
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFCell = rCell.mpFormula;
            sal_uInt16 nError = pFCell->GetErrCode();
            if (nError)
                return new FormulaErrorToken( nError);
            else if (pFCell->IsValue())
            {
                double fVal = pFCell->GetValue();
                return new formula::FormulaDoubleToken(fVal);
            }
            else
            {
                svl::SharedString aSS = pHostDoc->GetSharedStringPool().intern( pFCell->GetString().getString());
                return new formula::FormulaStringToken(aSS);
            }
        }
        default:
            OSL_FAIL("attempted to convert an unknown cell type.");
    }

    return nullptr;
}

template<class T>
struct ColumnBatch
{
    ScDocument* mpHostDoc;
    ScDocument* mpSrcDoc;

    std::vector<T> maStorage;
    CellType meType1;
    CellType meType2;
    SCROW mnRowStart;

    ColumnBatch( ScDocument* pHostDoc, ScDocument* pSrcDoc, CellType eType1, CellType eType2 ) :
        mpHostDoc(pHostDoc),
        mpSrcDoc(pSrcDoc),
        meType1(eType1),
        meType2(eType2),
        mnRowStart(-1) {}

    void update(ScRefCellValue& raCell, const SCCOL nCol, const SCROW nRow, ScMatrixRef& xMat)
    {
        if (raCell.meType == meType1 || raCell.meType == meType2)
        {
            if (mnRowStart < 0)
                mnRowStart = nRow;
            maStorage.push_back(getValue(raCell));
        }
        else
        {
            flush(nCol, xMat);
        }
    }

    void flush(const SCCOL nCol, ScMatrixRef& xMat)
    {
        if (maStorage.empty())
            return;
        putValues(xMat, nCol);
        mnRowStart = -1;
        maStorage.clear();
    }

    T getValue(ScRefCellValue& raCell) const;
    void putValues(ScMatrixRef& xMat, const SCCOL nCol) const;
};

template<>
inline svl::SharedString ColumnBatch<svl::SharedString>::getValue(ScRefCellValue& rCell) const
{
    OUString aStr = rCell.getString(mpSrcDoc);
    return mpHostDoc->GetSharedStringPool().intern(aStr);
}

template<class T>
inline T ColumnBatch<T>::getValue(ScRefCellValue& raCell) const
{
    return raCell.mfValue;
}

template<>
inline void ColumnBatch<svl::SharedString>::putValues(ScMatrixRef& xMat, const SCCOL nCol) const
{
    xMat->PutString(&maStorage.front(), maStorage.size(), nCol, mnRowStart);
}

template<class T>
inline void ColumnBatch<T>::putValues(ScMatrixRef& xMat, const SCCOL nCol) const
{
    xMat->PutDouble(&maStorage.front(), maStorage.size(), nCol, mnRowStart);
}

static std::unique_ptr<ScTokenArray> convertToTokenArray(
    ScDocument* pHostDoc, ScDocument* pSrcDoc, ScRange& rRange, vector<ScExternalRefCache::SingleRangeData>& rCacheData )
{
    ScAddress& s = rRange.aStart;
    ScAddress& e = rRange.aEnd;

    const SCTAB nTab1 = s.Tab(), nTab2 = e.Tab();
    const SCCOL nCol1 = s.Col(), nCol2 = e.Col();
    const SCROW nRow1 = s.Row(), nRow2 = e.Row();

    if (nTab2 != nTab1)
        // For now, we don't support multi-sheet ranges intentionally because
        // we don't have a way to express them in a single token.  In the
        // future we can introduce a new stack variable type svMatrixList with
        // a new token type that can store a 3D matrix value and convert a 3D
        // range to it.
        return nullptr;

    std::unique_ptr<ScRange> pUsedRange;

    unique_ptr<ScTokenArray> pArray(new ScTokenArray);
    bool bFirstTab = true;
    vector<ScExternalRefCache::SingleRangeData>::iterator
        itrCache = rCacheData.begin(), itrCacheEnd = rCacheData.end();

    for (SCTAB nTab = nTab1; nTab <= nTab2 && itrCache != itrCacheEnd; ++nTab, ++itrCache)
    {
        // Only loop within the data area.
        SCCOL nDataCol1 = nCol1, nDataCol2 = nCol2;
        SCROW nDataRow1 = nRow1, nDataRow2 = nRow2;
        bool bShrunk;
        if (!pSrcDoc->ShrinkToUsedDataArea( bShrunk, nTab, nDataCol1, nDataRow1, nDataCol2, nDataRow2, false))
            // no data within specified range.
            continue;

        if (pUsedRange.get())
            // Make sure the used area only grows, not shrinks.
            pUsedRange->ExtendTo(ScRange(nDataCol1, nDataRow1, 0, nDataCol2, nDataRow2, 0));
        else
            pUsedRange.reset(new ScRange(nDataCol1, nDataRow1, 0, nDataCol2, nDataRow2, 0));

        SCSIZE nMatrixColumns = static_cast<SCSIZE>(nCol2-nCol1+1);
        SCSIZE nMatrixRows = static_cast<SCSIZE>(nRow2-nRow1+1);
        ScMatrixRef xMat = new ScFullMatrix( nMatrixColumns, nMatrixRows);

        bool bFill = true;
        SCCOL nEffectiveCol1 = nCol1;
        SCROW nEffectiveRow1 = nRow1;

        // Check if size could be allocated and if not skip the fill, there's
        // one error element instead. But retry first with the actual data area
        // if that is smaller than the original range, which works for most
        // functions just not some that operate/compare with the original size
        // and expect empty values in non-data areas.
        // Restrict this though to ranges of entire columns or rows, other
        // ranges might be on purpose. (Other special cases to handle?)
        /* TODO: sparse matrix could help */
        SCSIZE nMatCols, nMatRows;
        xMat->GetDimensions( nMatCols, nMatRows);
        if (nMatCols != nMatrixColumns || nMatRows != nMatrixRows)
        {
            bFill = false;
            if ((nCol1 == 0 && nCol2 == MAXCOL) || (nRow1 == 0 && nRow2 == MAXROW))
            {
                if ((static_cast<SCSIZE>(nDataCol2-nDataCol1+1) < nMatrixColumns) ||
                    (static_cast<SCSIZE>(nDataRow2-nDataRow1+1) < nMatrixRows))
                {
                    nMatrixColumns = static_cast<SCSIZE>(nDataCol2-nDataCol1+1);
                    nMatrixRows = static_cast<SCSIZE>(nDataRow2-nDataRow1+1);
                    xMat = new ScFullMatrix( nMatrixColumns, nMatrixRows);
                    xMat->GetDimensions( nMatCols, nMatRows);
                    if (nMatCols == nMatrixColumns && nMatRows == nMatrixRows)
                    {
                        nEffectiveCol1 = nDataCol1;
                        nEffectiveRow1 = nDataRow1;
                        bFill = true;
                    }
                }
            }
        }

        if (bFill)
        {
            ColumnBatch<svl::SharedString> aStringBatch(pHostDoc, pSrcDoc, CELLTYPE_STRING, CELLTYPE_EDIT);
            ColumnBatch<double> aDoubleBatch(pHostDoc, pSrcDoc, CELLTYPE_VALUE, CELLTYPE_VALUE);

            for (SCCOL nCol = nDataCol1; nCol <= nDataCol2; ++nCol)
            {
                const SCSIZE nC = nCol - nEffectiveCol1;
                for (SCROW nRow = nDataRow1; nRow <= nDataRow2; ++nRow)
                {
                    const SCSIZE nR = nRow - nEffectiveRow1;

                    ScRefCellValue aCell(*pSrcDoc, ScAddress(nCol, nRow, nTab));

                    aStringBatch.update(aCell, nC, nR, xMat);
                    aDoubleBatch.update(aCell, nC, nR, xMat);

                    if (aCell.hasEmptyValue())
                        // Skip empty cells.  Matrix's default values are empty elements.
                        continue;

                    switch (aCell.meType)
                    {
                        case CELLTYPE_FORMULA:
                            {
                                ScFormulaCell* pFCell = aCell.mpFormula;
                                sal_uInt16 nError = pFCell->GetErrCode();
                                if (nError)
                                    xMat->PutDouble( CreateDoubleError( nError), nC, nR);
                                else if (pFCell->IsValue())
                                {
                                    double fVal = pFCell->GetValue();
                                    xMat->PutDouble(fVal, nC, nR);
                                }
                                else
                                {
                                    svl::SharedString aStr = pFCell->GetString();
                                    aStr = pHostDoc->GetSharedStringPool().intern(aStr.getString());
                                    xMat->PutString(aStr, nC, nR);
                                }
                            }
                            break;
                            // These are handled in batch:
                        case CELLTYPE_VALUE:
                        case CELLTYPE_STRING:
                        case CELLTYPE_EDIT:
                            break;
                        default:
                            OSL_FAIL("attempted to convert an unknown cell type.");
                    }
                }

                aStringBatch.flush(nC, xMat);
                aDoubleBatch.flush(nC, xMat);
            }
        }

        if (!bFirstTab)
            pArray->AddOpCode(ocSep);

        ScMatrixToken aToken(xMat);
        pArray->AddToken(aToken);

        itrCache->mpRangeData = xMat;

        bFirstTab = false;
    }

    if (!pUsedRange.get())
        return nullptr;

    s.SetCol(pUsedRange->aStart.Col());
    s.SetRow(pUsedRange->aStart.Row());
    e.SetCol(pUsedRange->aEnd.Col());
    e.SetRow(pUsedRange->aEnd.Row());

    return pArray;
}

static std::unique_ptr<ScTokenArray> lcl_fillEmptyMatrix(const ScRange& rRange)
{
    SCSIZE nC = static_cast<SCSIZE>(rRange.aEnd.Col()-rRange.aStart.Col()+1);
    SCSIZE nR = static_cast<SCSIZE>(rRange.aEnd.Row()-rRange.aStart.Row()+1);
    ScMatrixRef xMat = new ScFullMatrix(nC, nR);

    ScMatrixToken aToken(xMat);
    unique_ptr<ScTokenArray> pArray(new ScTokenArray);
    pArray->AddToken(aToken);
    return pArray;
}

ScExternalRefManager::ScExternalRefManager(ScDocument* pDoc) :
    mpDoc(pDoc),
    mbInReferenceMarking(false),
    mbUserInteractionEnabled(true),
    mbDocTimerEnabled(true)
{
    maSrcDocTimer.SetTimeoutHdl( LINK(this, ScExternalRefManager, TimeOutHdl) );
    maSrcDocTimer.SetTimeout(SRCDOC_SCAN_INTERVAL);
}

ScExternalRefManager::~ScExternalRefManager()
{
    clear();
}

OUString ScExternalRefManager::getCacheTableName(sal_uInt16 nFileId, size_t nTabIndex) const
{
    return maRefCache.getTableName(nFileId, nTabIndex);
}

ScExternalRefCache::TableTypeRef ScExternalRefManager::getCacheTable(sal_uInt16 nFileId, size_t nTabIndex) const
{
    return maRefCache.getCacheTable(nFileId, nTabIndex);
}

ScExternalRefCache::TableTypeRef ScExternalRefManager::getCacheTable(
    sal_uInt16 nFileId, const OUString& rTabName, bool bCreateNew, size_t* pnIndex, const OUString* pExtUrl)
{
    return maRefCache.getCacheTable(nFileId, rTabName, bCreateNew, pnIndex, pExtUrl);
}

ScExternalRefManager::LinkListener::LinkListener()
{
}

ScExternalRefManager::LinkListener::~LinkListener()
{
}

ScExternalRefManager::ApiGuard::ApiGuard(ScDocument* pDoc) :
    mpMgr(pDoc->GetExternalRefManager()),
    mbOldInteractionEnabled(mpMgr->mbUserInteractionEnabled)
{
    // We don't want user interaction handled in the API.
    mpMgr->mbUserInteractionEnabled = false;
}

ScExternalRefManager::ApiGuard::~ApiGuard()
{
    // Restore old value.
    mpMgr->mbUserInteractionEnabled = mbOldInteractionEnabled;
}

void ScExternalRefManager::getAllCachedTableNames(sal_uInt16 nFileId, vector<OUString>& rTabNames) const
{
    maRefCache.getAllTableNames(nFileId, rTabNames);
}

SCsTAB ScExternalRefManager::getCachedTabSpan( sal_uInt16 nFileId, const OUString& rStartTabName, const OUString& rEndTabName ) const
{
    return maRefCache.getTabSpan( nFileId, rStartTabName, rEndTabName);
}

void ScExternalRefManager::getAllCachedNumberFormats(vector<sal_uInt32>& rNumFmts) const
{
    maRefCache.getAllNumberFormats(rNumFmts);
}

sal_uInt16 ScExternalRefManager::getExternalFileCount() const
{
    return static_cast< sal_uInt16 >( maSrcFiles.size() );
}

bool ScExternalRefManager::markUsedByLinkListeners()
{
    bool bAllMarked = false;
    for (LinkListenerMap::const_iterator itr = maLinkListeners.begin();
            itr != maLinkListeners.end() && !bAllMarked; ++itr)
    {
        if (!(*itr).second.empty())
            bAllMarked = maRefCache.setCacheDocReferenced( (*itr).first);
        /* TODO: LinkListeners should remember the table they're listening to.
         * As is, listening to one table will mark all tables of the document
         * being referenced. */
    }
    return bAllMarked;
}

bool ScExternalRefManager::markUsedExternalRefCells()
{
    RefCellMap::iterator itr = maRefCells.begin(), itrEnd = maRefCells.end();
    for (; itr != itrEnd; ++itr)
    {
        RefCellSet::iterator itrCell = itr->second.begin(), itrCellEnd = itr->second.end();
        for (; itrCell != itrCellEnd; ++itrCell)
        {
            ScFormulaCell* pCell = *itrCell;
            bool bUsed = pCell->MarkUsedExternalReferences();
            if (bUsed)
                // Return true when at least one cell references external docs.
                return true;
        }
    }
    return false;
}

bool ScExternalRefManager::setCacheTableReferenced( sal_uInt16 nFileId, const OUString& rTabName, size_t nSheets )
{
    return maRefCache.setCacheTableReferenced( nFileId, rTabName, nSheets, false);
}

void ScExternalRefManager::setAllCacheTableReferencedStati( bool bReferenced )
{
    mbInReferenceMarking = !bReferenced;
    maRefCache.setAllCacheTableReferencedStati( bReferenced );
}

void ScExternalRefManager::storeRangeNameTokens(sal_uInt16 nFileId, const OUString& rName, const ScTokenArray& rArray)
{
    ScExternalRefCache::TokenArrayRef pArray(rArray.Clone());
    maRefCache.setRangeNameTokens(nFileId, rName, pArray);
}

namespace {

/**
 * Put a single cell data into internal cache table.
 *
 * @param pFmt optional cell format index that may need to be stored with
 *             the cell value.
 */
void putCellDataIntoCache(
    ScExternalRefCache& rRefCache, const ScExternalRefCache::TokenRef& pToken,
    sal_uInt16 nFileId, const OUString& rTabName, const ScAddress& rCell,
    const ScExternalRefCache::CellFormat* pFmt)
{
    // Now, insert the token into cache table but don't cache empty cells.
    if (pToken->GetType() != formula::svEmptyCell)
    {
        sal_uLong nFmtIndex = (pFmt && pFmt->mbIsSet) ? pFmt->mnIndex : 0;
        rRefCache.setCellData(nFileId, rTabName, rCell.Col(), rCell.Row(), pToken, nFmtIndex);
    }
}

/**
 * Put the data into our internal cache table.
 *
 * @param rRefCache cache table set.
 * @param pArray single range data to be returned.
 * @param nFileId external file ID
 * @param rTabName name of the table where the data should be cached.
 * @param rCacheData range data to be cached.
 * @param rCacheRange original cache range, including the empty region if
 *                    any.
 * @param rDataRange reduced cache range that includes only the non-empty
 *                   data area.
 */
void putRangeDataIntoCache(
    ScExternalRefCache& rRefCache, ScExternalRefCache::TokenArrayRef& pArray,
    sal_uInt16 nFileId, const OUString& rTabName,
    const vector<ScExternalRefCache::SingleRangeData>& rCacheData,
    const ScRange& rCacheRange, const ScRange& rDataRange)
{
    if (pArray)
        // Cache these values.
        rRefCache.setCellRangeData(nFileId, rDataRange, rCacheData, pArray);
    else
    {
        // Array is empty.  Fill it with an empty matrix of the required size.
        pArray = lcl_fillEmptyMatrix(rCacheRange);

        // Make sure to set this range 'cached', to prevent unnecessarily
        // accessing the src document time and time again.
        ScExternalRefCache::TableTypeRef pCacheTab =
            rRefCache.getCacheTable(nFileId, rTabName, true, nullptr, nullptr);
        if (pCacheTab)
            pCacheTab->setCachedCellRange(
                rCacheRange.aStart.Col(), rCacheRange.aStart.Row(), rCacheRange.aEnd.Col(), rCacheRange.aEnd.Row());
    }
}

/**
 * When accessing an external document for the first time, we need to
 * populate the cache with all its sheet names (whether they are referenced
 * or not) in the correct order.  Many client codes that use external
 * references make this assumption.
 *
 * @param rRefCache cache table set.
 * @param pSrcDoc source document instance.
 * @param nFileId external file ID associated with the source document.
 */
void initDocInCache(ScExternalRefCache& rRefCache, const ScDocument* pSrcDoc, sal_uInt16 nFileId)
{
    if (!pSrcDoc)
        return;

    if (rRefCache.isDocInitialized(nFileId))
        // Already initialized.  No need to do this twice.
        return;

    SCTAB nTabCount = pSrcDoc->GetTableCount();
    if (nTabCount)
    {
        // Populate the cache with all table names in the source document.
        vector<OUString> aTabNames;
        aTabNames.reserve(nTabCount);
        for (SCTAB i = 0; i < nTabCount; ++i)
        {
            OUString aName;
            pSrcDoc->GetName(i, aName);
            aTabNames.push_back(aName);
        }

        // Obtain the base name, don't bother if there are more than one sheets.
        OUString aBaseName;
        if (nTabCount == 1)
        {
            const SfxObjectShell* pShell = pSrcDoc->GetDocumentShell();
            if (pShell && pShell->GetMedium())
            {
                OUString aName = pShell->GetMedium()->GetName();
                aBaseName = INetURLObject( aName).GetBase();
            }
        }

        rRefCache.initializeDoc(nFileId, aTabNames, aBaseName);
    }
}

}

bool ScExternalRefManager::getSrcDocTable( const ScDocument& rSrcDoc, const OUString& rTabName, SCTAB& rTab,
        sal_uInt16 nFileId ) const
{
    return maRefCache.getSrcDocTable( rSrcDoc, rTabName, rTab, nFileId);
}

ScExternalRefCache::TokenRef ScExternalRefManager::getSingleRefToken(
    sal_uInt16 nFileId, const OUString& rTabName, const ScAddress& rCell,
    const ScAddress* pCurPos, SCTAB* pTab, ScExternalRefCache::CellFormat* pFmt)
{
    if (pCurPos)
        insertRefCell(nFileId, *pCurPos);

    maybeLinkExternalFile(nFileId);

    if (pTab)
        *pTab = -1;

    if (pFmt)
        pFmt->mbIsSet = false;

    ScDocument* pSrcDoc = getInMemorySrcDocument(nFileId);
    if (pSrcDoc)
    {
        // source document already loaded in memory.  Re-use this instance.
        SCTAB nTab;
        if (!getSrcDocTable( *pSrcDoc, rTabName, nTab, nFileId))
        {
            // specified table name doesn't exist in the source document.
            ScExternalRefCache::TokenRef pToken(new FormulaErrorToken(errNoRef));
            return pToken;
        }

        if (pTab)
            *pTab = nTab;

        ScExternalRefCache::TokenRef pToken =
            getSingleRefTokenFromSrcDoc(
                nFileId, pSrcDoc, ScAddress(rCell.Col(),rCell.Row(),nTab), pFmt);

        putCellDataIntoCache(maRefCache, pToken, nFileId, rTabName, rCell, pFmt);
        return pToken;
    }

    // Check if the given table name and the cell position is cached.
    sal_uInt32 nFmtIndex = 0;
    ScExternalRefCache::TokenRef pToken = maRefCache.getCellData(
        nFileId, rTabName, rCell.Col(), rCell.Row(), &nFmtIndex);
    if (pToken)
    {
        // Cache hit !
        fillCellFormat(nFmtIndex, pFmt);
        return pToken;
    }

    // reference not cached.  read from the source document.
    pSrcDoc = getSrcDocument(nFileId);
    if (!pSrcDoc)
    {
        // Source document not reachable.  Throw a reference error.
        pToken.reset(new FormulaErrorToken(errNoRef));
        return pToken;
    }

    SCTAB nTab;
    if (!getSrcDocTable( *pSrcDoc, rTabName, nTab, nFileId))
    {
        // specified table name doesn't exist in the source document.
        pToken.reset(new FormulaErrorToken(errNoRef));
        return pToken;
    }

    if (pTab)
        *pTab = nTab;

    SCCOL nDataCol1 = 0, nDataCol2 = MAXCOL;
    SCROW nDataRow1 = 0, nDataRow2 = MAXROW;
    bool bData = pSrcDoc->ShrinkToDataArea(nTab, nDataCol1, nDataRow1, nDataCol2, nDataRow2);
    if (!bData || rCell.Col() < nDataCol1 || nDataCol2 < rCell.Col() || rCell.Row() < nDataRow1 || nDataRow2 < rCell.Row())
    {
        // requested cell is outside the data area.  Don't even bother caching
        // this data, but add it to the cached range to prevent accessing the
        // source document time and time again.
        ScExternalRefCache::TableTypeRef pCacheTab =
            maRefCache.getCacheTable(nFileId, rTabName, true, nullptr, nullptr);
        if (pCacheTab)
            pCacheTab->setCachedCell(rCell.Col(), rCell.Row());

        pToken.reset(new ScEmptyCellToken(false, false));
        return pToken;
    }

    pToken = getSingleRefTokenFromSrcDoc(
        nFileId, pSrcDoc, ScAddress(rCell.Col(),rCell.Row(),nTab), pFmt);

    putCellDataIntoCache(maRefCache, pToken, nFileId, rTabName, rCell, pFmt);
    return pToken;
}

ScExternalRefCache::TokenArrayRef ScExternalRefManager::getDoubleRefTokens(
    sal_uInt16 nFileId, const OUString& rTabName, const ScRange& rRange, const ScAddress* pCurPos)
{
    if (pCurPos)
        insertRefCell(nFileId, *pCurPos);

    maybeLinkExternalFile(nFileId);

    ScRange aDataRange(rRange);
    ScDocument* pSrcDoc = getInMemorySrcDocument(nFileId);
    if (pSrcDoc)
    {
        // Document already loaded in memory.
        vector<ScExternalRefCache::SingleRangeData> aCacheData;
        ScExternalRefCache::TokenArrayRef pArray =
            getDoubleRefTokensFromSrcDoc(pSrcDoc, rTabName, aDataRange, aCacheData);

        // Put the data into cache.
        putRangeDataIntoCache(maRefCache, pArray, nFileId, rTabName, aCacheData, rRange, aDataRange);
        return pArray;
    }

    // Check if the given table name and the cell position is cached.
    ScExternalRefCache::TokenArrayRef pArray =
        maRefCache.getCellRangeData(nFileId, rTabName, rRange);
    if (pArray)
        // Cache hit !
        return pArray;

    pSrcDoc = getSrcDocument(nFileId);
    if (!pSrcDoc)
    {
        // Source document is not reachable.  Throw a reference error.
        pArray.reset(new ScTokenArray);
        pArray->AddToken(FormulaErrorToken(errNoRef));
        return pArray;
    }

    vector<ScExternalRefCache::SingleRangeData> aCacheData;
    pArray = getDoubleRefTokensFromSrcDoc(pSrcDoc, rTabName, aDataRange, aCacheData);

    // Put the data into cache.
    putRangeDataIntoCache(maRefCache, pArray, nFileId, rTabName, aCacheData, rRange, aDataRange);
    return pArray;
}

ScExternalRefCache::TokenArrayRef ScExternalRefManager::getRangeNameTokens(
    sal_uInt16 nFileId, const OUString& rName, const ScAddress* pCurPos)
{
    if (pCurPos)
        insertRefCell(nFileId, *pCurPos);

    maybeLinkExternalFile(nFileId);

    OUString aName = rName; // make a copy to have the casing corrected.
    ScDocument* pSrcDoc = getInMemorySrcDocument(nFileId);
    if (pSrcDoc)
    {
        // Document already loaded in memory.
        ScExternalRefCache::TokenArrayRef pArray =
            getRangeNameTokensFromSrcDoc(nFileId, pSrcDoc, aName);

        if (pArray)
            // Cache this range name array.
            maRefCache.setRangeNameTokens(nFileId, aName, pArray);

        return pArray;
    }

    ScExternalRefCache::TokenArrayRef pArray = maRefCache.getRangeNameTokens(nFileId, rName);
    if (pArray.get())
        // This range name is cached.
        return pArray;

    pSrcDoc = getSrcDocument(nFileId);
    if (!pSrcDoc)
        // failed to load document from disk.
        return ScExternalRefCache::TokenArrayRef();

    pArray = getRangeNameTokensFromSrcDoc(nFileId, pSrcDoc, aName);

    if (pArray)
        // Cache this range name array.
        maRefCache.setRangeNameTokens(nFileId, aName, pArray);

    return pArray;
}

namespace {

bool hasRangeName(ScDocument& rDoc, const OUString& rName)
{
    ScRangeName* pExtNames = rDoc.GetRangeName();
    OUString aUpperName = ScGlobal::pCharClass->uppercase(rName);
    const ScRangeData* pRangeData = pExtNames->findByUpperName(aUpperName);
    return pRangeData != nullptr;
}

}

bool ScExternalRefManager::isValidRangeName(sal_uInt16 nFileId, const OUString& rName)
{
    maybeLinkExternalFile(nFileId);
    ScDocument* pSrcDoc = getInMemorySrcDocument(nFileId);
    if (pSrcDoc)
    {
        // Only check the presence of the name.
        if (hasRangeName(*pSrcDoc, rName))
        {
            maRefCache.setRangeName(nFileId, rName);
            return true;
        }
        return false;
    }

    if (maRefCache.isValidRangeName(nFileId, rName))
        // Range name is cached.
        return true;

    pSrcDoc = getSrcDocument(nFileId);
    if (!pSrcDoc)
        // failed to load document from disk.
        return false;

    if (hasRangeName(*pSrcDoc, rName))
    {
        maRefCache.setRangeName(nFileId, rName);
        return true;
    }

    return false;
}

void ScExternalRefManager::refreshAllRefCells(sal_uInt16 nFileId)
{
    RefCellMap::iterator itrFile = maRefCells.find(nFileId);
    if (itrFile == maRefCells.end())
        return;

    RefCellSet& rRefCells = itrFile->second;
    for_each(rRefCells.begin(), rRefCells.end(), UpdateFormulaCell());

    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return;

    ScTabViewShell* pVShell = pViewData->GetViewShell();
    if (!pVShell)
        return;

    // Repainting the grid also repaints the texts, but is there a better way
    // to refresh texts?
    pVShell->Invalidate(FID_REPAINT);
    pVShell->PaintGrid();
}

namespace {

void insertRefCellByIterator(
    ScExternalRefManager::RefCellMap::iterator& itr, ScFormulaCell* pCell)
{
    if (pCell)
    {
        itr->second.insert(pCell);
        pCell->SetIsExtRef();
    }
}

}

void ScExternalRefManager::insertRefCell(sal_uInt16 nFileId, const ScAddress& rCell)
{
    RefCellMap::iterator itr = maRefCells.find(nFileId);
    if (itr == maRefCells.end())
    {
        RefCellSet aRefCells;
        pair<RefCellMap::iterator, bool> r = maRefCells.insert(
            RefCellMap::value_type(nFileId, aRefCells));
        if (!r.second)
            // insertion failed.
            return;

        itr = r.first;
    }

    insertRefCellByIterator(itr, mpDoc->GetFormulaCell(rCell));
}

void ScExternalRefManager::insertRefCellFromTemplate( ScFormulaCell* pTemplateCell, ScFormulaCell* pCell )
{
    if (!pTemplateCell || !pCell)
        return;

    for (RefCellMap::iterator itr = maRefCells.begin(); itr != maRefCells.end(); ++itr)
    {
        if (itr->second.find(pTemplateCell) != itr->second.end())
            insertRefCellByIterator(itr, pCell);
    }
}

bool ScExternalRefManager::hasCellExternalReference(const ScAddress& rCell)
{
    ScFormulaCell* pCell = mpDoc->GetFormulaCell(rCell);

    if (pCell)
      for (RefCellMap::iterator itr = maRefCells.begin(); itr != maRefCells.end(); ++itr)
      {
          if (itr->second.find(pCell) != itr->second.end())
              return true;
      }

    return false;
}

void ScExternalRefManager::enableDocTimer( bool bEnable )
{
    if (mbDocTimerEnabled == bEnable)
        return;

    mbDocTimerEnabled = bEnable;
    if (mbDocTimerEnabled)
    {
        if (!maDocShells.empty())
        {
            DocShellMap::iterator it = maDocShells.begin(), itEnd = maDocShells.end();
            for (; it != itEnd; ++it)
                it->second.maLastAccess = tools::Time(tools::Time::SYSTEM);

            maSrcDocTimer.Start();
        }
    }
    else
        maSrcDocTimer.Stop();
}

void ScExternalRefManager::fillCellFormat(sal_uLong nFmtIndex, ScExternalRefCache::CellFormat* pFmt) const
{
    if (!pFmt)
        return;

    short nFmtType = mpDoc->GetFormatTable()->GetType(nFmtIndex);
    if (nFmtType != css::util::NumberFormat::UNDEFINED)
    {
        pFmt->mbIsSet = true;
        pFmt->mnIndex = nFmtIndex;
        pFmt->mnType = nFmtType;
    }
}

ScExternalRefCache::TokenRef ScExternalRefManager::getSingleRefTokenFromSrcDoc(
    sal_uInt16 nFileId, ScDocument* pSrcDoc, const ScAddress& rPos,
    ScExternalRefCache::CellFormat* pFmt)
{
    // Get the cell from src doc, and convert it into a token.
    ScRefCellValue aCell(*pSrcDoc, rPos);
    ScExternalRefCache::TokenRef pToken(convertToToken(mpDoc, pSrcDoc, aCell));

    if (!pToken.get())
    {
        // Generate an error for unresolvable cells.
        pToken.reset( new FormulaErrorToken( errNoValue));
    }

    // Get number format information.
    sal_uInt32 nFmtIndex = 0;
    pSrcDoc->GetNumberFormat(rPos.Col(), rPos.Row(), rPos.Tab(), nFmtIndex);
    nFmtIndex = getMappedNumberFormat(nFileId, nFmtIndex, pSrcDoc);
    fillCellFormat(nFmtIndex, pFmt);
    return pToken;
}

ScExternalRefCache::TokenArrayRef ScExternalRefManager::getDoubleRefTokensFromSrcDoc(
    ScDocument* pSrcDoc, const OUString& rTabName, ScRange& rRange,
    vector<ScExternalRefCache::SingleRangeData>& rCacheData)
{
    ScExternalRefCache::TokenArrayRef pArray;
    SCTAB nTab1;

    if (!pSrcDoc->GetTable(rTabName, nTab1))
    {
        // specified table name doesn't exist in the source document.
        pArray.reset(new ScTokenArray);
        pArray->AddToken(FormulaErrorToken(errNoRef));
        return pArray;
    }

    ScRange aRange(rRange);
    aRange.PutInOrder();
    SCTAB nTabSpan = aRange.aEnd.Tab() - aRange.aStart.Tab();

    vector<ScExternalRefCache::SingleRangeData> aCacheData;
    aCacheData.reserve(nTabSpan+1);
    aCacheData.push_back(ScExternalRefCache::SingleRangeData());
    aCacheData.back().maTableName = ScGlobal::pCharClass->uppercase(rTabName);

    for (SCTAB i = 1; i < nTabSpan + 1; ++i)
    {
        OUString aTabName;
        if (!pSrcDoc->GetName(nTab1 + 1, aTabName))
            // source document doesn't have any table by the specified name.
            break;

        aCacheData.push_back(ScExternalRefCache::SingleRangeData());
        aCacheData.back().maTableName = ScGlobal::pCharClass->uppercase(aTabName);
    }

    aRange.aStart.SetTab(nTab1);
    aRange.aEnd.SetTab(nTab1 + nTabSpan);

    pArray = convertToTokenArray(mpDoc, pSrcDoc, aRange, aCacheData);
    rRange = aRange;
    rCacheData.swap(aCacheData);
    return pArray;
}

ScExternalRefCache::TokenArrayRef ScExternalRefManager::getRangeNameTokensFromSrcDoc(
    sal_uInt16 nFileId, ScDocument* pSrcDoc, OUString& rName)
{
    ScRangeName* pExtNames = pSrcDoc->GetRangeName();
    OUString aUpperName = ScGlobal::pCharClass->uppercase(rName);
    const ScRangeData* pRangeData = pExtNames->findByUpperName(aUpperName);
    if (!pRangeData)
        return ScExternalRefCache::TokenArrayRef();

    // Parse all tokens in this external range data, and replace each absolute
    // reference token with an external reference token, and cache them.  Also
    // register the source document with the link manager if it's a new
    // source.

    ScExternalRefCache::TokenArrayRef pNew(new ScTokenArray);

    ScTokenArray aCode(*pRangeData->GetCode());
    for (const FormulaToken* pToken = aCode.First(); pToken; pToken = aCode.Next())
    {
        bool bTokenAdded = false;
        switch (pToken->GetType())
        {
            case svSingleRef:
            {
                const ScSingleRefData& rRef = *pToken->GetSingleRef();
                OUString aTabName;
                pSrcDoc->GetName(rRef.Tab(), aTabName);
                ScExternalSingleRefToken aNewToken(nFileId, aTabName, *pToken->GetSingleRef());
                pNew->AddToken(aNewToken);
                bTokenAdded = true;
            }
            break;
            case svDoubleRef:
            {
                const ScSingleRefData& rRef = *pToken->GetSingleRef();
                OUString aTabName;
                pSrcDoc->GetName(rRef.Tab(), aTabName);
                ScExternalDoubleRefToken aNewToken(nFileId, aTabName, *pToken->GetDoubleRef());
                pNew->AddToken(aNewToken);
                bTokenAdded = true;
            }
            break;
            default:
                ;   // nothing
        }

        if (!bTokenAdded)
            pNew->AddToken(*pToken);
    }

    rName = pRangeData->GetName(); // Get the correctly-cased name.
    return pNew;
}

ScDocument* ScExternalRefManager::getInMemorySrcDocument(sal_uInt16 nFileId)
{
    const OUString* pFileName = getExternalFileName(nFileId);
    if (!pFileName)
        return nullptr;

    ScDocument* pSrcDoc = nullptr;
    ScDocShell* pShell = static_cast<ScDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<ScDocShell>, false));
    while (pShell)
    {
        SfxMedium* pMedium = pShell->GetMedium();
        if (pMedium && !pMedium->GetName().isEmpty())
        {
            // TODO: We should make the case sensitivity platform dependent.
            if (pFileName->equalsIgnoreAsciiCase(pMedium->GetName()))
            {
                // Found !
                pSrcDoc = &pShell->GetDocument();
                break;
            }
        }
        else
        {
            // handle unsaved documents here
            OUString aName = pShell->GetName();
            if (pFileName->equalsIgnoreAsciiCase(aName))
            {
                // Found !
                SrcShell aSrcDoc;
                aSrcDoc.maShell = pShell;
                maUnsavedDocShells.insert(DocShellMap::value_type(nFileId, aSrcDoc));
                StartListening(*pShell);
                pSrcDoc = &pShell->GetDocument();
                break;
            }
        }
        pShell = static_cast<ScDocShell*>(SfxObjectShell::GetNext(*pShell, checkSfxObjectShell<ScDocShell>, false));
    }

    initDocInCache(maRefCache, pSrcDoc, nFileId);
    return pSrcDoc;
}

ScDocument* ScExternalRefManager::getSrcDocument(sal_uInt16 nFileId)
{
    if (!mpDoc->IsExecuteLinkEnabled())
        return nullptr;

    DocShellMap::iterator itrEnd = maDocShells.end();
    DocShellMap::iterator itr = maDocShells.find(nFileId);

    if (itr != itrEnd)
    {
        // document already loaded.

        SfxObjectShell* p = itr->second.maShell;
        itr->second.maLastAccess = tools::Time( tools::Time::SYSTEM );
        return &static_cast<ScDocShell*>(p)->GetDocument();
    }

    itrEnd = maUnsavedDocShells.end();
    itr = maUnsavedDocShells.find(nFileId);
    if (itr != itrEnd)
    {
        //document is unsaved document

        SfxObjectShell* p = itr->second.maShell;
        itr->second.maLastAccess = tools::Time( tools::Time::SYSTEM );
        return &static_cast<ScDocShell*>(p)->GetDocument();
    }

    const OUString* pFile = getExternalFileName(nFileId);
    if (!pFile)
        // no file name associated with this ID.
        return nullptr;

    OUString aFilter;
    SrcShell aSrcDoc;
    try
    {
        aSrcDoc.maShell = loadSrcDocument(nFileId, aFilter);
    }
    catch (const css::uno::Exception&)
    {
    }
    if (!aSrcDoc.maShell.Is())
    {
        // source document could not be loaded.
        return nullptr;
    }

    return &cacheNewDocShell(nFileId, aSrcDoc);
}

SfxObjectShellRef ScExternalRefManager::loadSrcDocument(sal_uInt16 nFileId, OUString& rFilter)
{
    const SrcFileData* pFileData = getExternalFileData(nFileId);
    if (!pFileData)
        return nullptr;

    // Always load the document by using the path created from the relative
    // path.  If the referenced document is not there, simply exit.  The
    // original file name should be used only when the relative path is not
    // given.
    OUString aFile = pFileData->maFileName;
    maybeCreateRealFileName(nFileId);
    if (!pFileData->maRealFileName.isEmpty())
        aFile = pFileData->maRealFileName;

    if (!isFileLoadable(aFile))
        return nullptr;

    OUString aOptions = pFileData->maFilterOptions;
    if ( !pFileData->maFilterName.isEmpty() )
        rFilter = pFileData->maFilterName;      // don't overwrite stored filter with guessed filter
    else
        ScDocumentLoader::GetFilterName(aFile, rFilter, aOptions, true, false);
    ScDocumentLoader::GetFilterName(aFile, rFilter, aOptions, true, false);
    const SfxFilter* pFilter = ScDocShell::Factory().GetFilterContainer()->GetFilter4FilterName(rFilter);

    if (pFileData->maRelativeName.isEmpty())
    {
        // Generate a relative file path.
        INetURLObject aBaseURL(getOwnDocumentName());
        aBaseURL.insertName("content.xml");

        OUString aStr = URIHelper::simpleNormalizedMakeRelative(
            aBaseURL.GetMainURL(INetURLObject::NO_DECODE), aFile);

        setRelativeFileName(nFileId, aStr);
    }

    SfxItemSet* pSet = new SfxAllItemSet(SfxGetpApp()->GetPool());
    if (!aOptions.isEmpty())
        pSet->Put(SfxStringItem(SID_FILE_FILTEROPTIONS, aOptions));

    // make medium hidden to prevent assertion from progress bar
    pSet->Put( SfxBoolItem(SID_HIDDEN, true) );

    // If the current document is allowed to execute macros then the referenced
    // document may execute macros according to the security configuration.
    SfxObjectShell* pShell = mpDoc->GetDocumentShell();
    if (pShell)
    {
        SfxMedium* pMedium = pShell->GetMedium();
        if (pMedium)
        {
            const SfxPoolItem* pItem;
            if (pMedium->GetItemSet()->GetItemState( SID_MACROEXECMODE, false, &pItem ) == SfxItemState::SET &&
                    static_cast<const SfxUInt16Item*>(pItem)->GetValue() != css::document::MacroExecMode::NEVER_EXECUTE)
                pSet->Put( SfxUInt16Item( SID_MACROEXECMODE, css::document::MacroExecMode::USE_CONFIG));
        }
    }

    unique_ptr<SfxMedium> pMedium(new SfxMedium(aFile, STREAM_STD_READ, pFilter, pSet));
    if (pMedium->GetError() != ERRCODE_NONE)
        return nullptr;

    // To load encrypted documents with password, user interaction needs to be enabled.
    pMedium->UseInteractionHandler(mbUserInteractionEnabled);

    ScDocShell* pNewShell = new ScDocShell(SfxModelFlags::EXTERNAL_LINK);
    SfxObjectShellRef aRef = pNewShell;

    // increment the recursive link count of the source document.
    ScExtDocOptions* pExtOpt = mpDoc->GetExtDocOptions();
    sal_uInt32 nLinkCount = pExtOpt ? pExtOpt->GetDocSettings().mnLinkCnt : 0;
    ScDocument& rSrcDoc = pNewShell->GetDocument();
    rSrcDoc.EnableExecuteLink(false); // to prevent circular access of external references.
    rSrcDoc.EnableUndo(false);
    rSrcDoc.EnableAdjustHeight(false);
    rSrcDoc.EnableUserInteraction(false);

    ScExtDocOptions* pExtOptNew = rSrcDoc.GetExtDocOptions();
    if (!pExtOptNew)
    {
        pExtOptNew = new ScExtDocOptions;
        rSrcDoc.SetExtDocOptions(pExtOptNew);
    }
    pExtOptNew->GetDocSettings().mnLinkCnt = nLinkCount + 1;

    if (!pNewShell->DoLoad(pMedium.release()))
    {
        aRef->DoClose();
        aRef.Clear();
        return aRef;
    }

    // with UseInteractionHandler, options may be set by dialog during DoLoad
    OUString aNew = ScDocumentLoader::GetOptions(*pNewShell->GetMedium());
    if (!aNew.isEmpty() && aNew != aOptions)
        aOptions = aNew;
    setFilterData(nFileId, rFilter, aOptions);    // update the filter data, including the new options

    return aRef;
}

ScDocument& ScExternalRefManager::cacheNewDocShell( sal_uInt16 nFileId, SrcShell& rSrcShell )
{
    if (mbDocTimerEnabled && maDocShells.empty())
        // If this is the first source document insertion, start up the timer.
        maSrcDocTimer.Start();

    maDocShells.insert(DocShellMap::value_type(nFileId, rSrcShell));
    SfxObjectShell& rShell = *rSrcShell.maShell;
    ScDocument& rSrcDoc = static_cast<ScDocShell&>(rShell).GetDocument();
    initDocInCache(maRefCache, &rSrcDoc, nFileId);
    return rSrcDoc;
}

bool ScExternalRefManager::isFileLoadable(const OUString& rFile) const
{
    if (rFile.isEmpty())
        return false;

    if (isOwnDocument(rFile))
        return false;
    OUString aPhysical;
    if (osl::FileBase::getSystemPathFromFileURL(rFile, aPhysical)
        == osl::FileBase::E_None)
    {
        // #i114504# try IsFolder/Exists only for file URLs

        if (utl::UCBContentHelper::IsFolder(rFile))
            return false;

        return utl::UCBContentHelper::Exists(rFile);
    }
    else
        return true;    // for http and others, Exists doesn't work, but the URL can still be opened
}

void ScExternalRefManager::maybeLinkExternalFile(sal_uInt16 nFileId)
{
    if (maLinkedDocs.count(nFileId))
        // file already linked, or the link has been broken.
        return;

    // Source document not linked yet.  Link it now.
    const OUString* pFileName = getExternalFileName(nFileId);
    if (!pFileName)
        return;

    OUString aFilter, aOptions;
    const SrcFileData* pFileData = getExternalFileData(nFileId);
    if (pFileData)
    {
        aFilter = pFileData->maFilterName;
        aOptions = pFileData->maFilterOptions;
    }
    // If a filter was already set (for example, loading the cached table),
    // don't call GetFilterName which has to access the source file.
    if (aFilter.isEmpty())
        ScDocumentLoader::GetFilterName(*pFileName, aFilter, aOptions, true, false);
    sfx2::LinkManager* pLinkMgr = mpDoc->GetLinkManager();
    if (!pLinkMgr)
    {
        SAL_WARN( "sc.ui", "ScExternalRefManager::maybeLinkExternalFile: pLinkMgr==NULL");
        return;
    }
    ScExternalRefLink* pLink = new ScExternalRefLink(mpDoc, nFileId, aFilter);
    OSL_ENSURE(pFileName, "ScExternalRefManager::maybeLinkExternalFile: file name pointer is NULL");
    OUString aTmp = aFilter;
    pLinkMgr->InsertFileLink(*pLink, OBJECT_CLIENT_FILE, *pFileName, &aTmp);

    pLink->SetDoReferesh(false);
    pLink->Update();
    pLink->SetDoReferesh(true);

    maLinkedDocs.insert(LinkedDocMap::value_type(nFileId, true));
}

void ScExternalRefManager::addFilesToLinkManager()
{
    if (maSrcFiles.empty())
        return;

    SAL_WARN_IF( maSrcFiles.size() >= SAL_MAX_UINT16,
            "sc.ui", "ScExternalRefManager::addFilesToLinkManager: files overflow");
    const sal_uInt16 nSize = static_cast<sal_uInt16>( std::min<size_t>( maSrcFiles.size(), SAL_MAX_UINT16));
    for (sal_uInt16 nFileId = 0; nFileId < nSize; ++nFileId)
        maybeLinkExternalFile( nFileId);
}

void ScExternalRefManager::SrcFileData::maybeCreateRealFileName(const OUString& rOwnDocName)
{
    if (maRelativeName.isEmpty())
        // No relative path given.  Nothing to do.
        return;

    if (!maRealFileName.isEmpty())
        // Real file name already created.  Nothing to do.
        return;

    // Formulate the absolute file path from the relative path.
    const OUString& rRelPath = maRelativeName;
    INetURLObject aBaseURL(rOwnDocName);
    aBaseURL.insertName("content.xml");
    bool bWasAbs = false;
    maRealFileName = aBaseURL.smartRel2Abs(rRelPath, bWasAbs).GetMainURL(INetURLObject::NO_DECODE);
}

void ScExternalRefManager::maybeCreateRealFileName(sal_uInt16 nFileId)
{
    if (nFileId >= maSrcFiles.size())
        return;

    maSrcFiles[nFileId].maybeCreateRealFileName(getOwnDocumentName());
}

OUString ScExternalRefManager::getOwnDocumentName() const
{
    SfxObjectShell* pShell = mpDoc->GetDocumentShell();
    if (!pShell)
        // This should not happen!
        return OUString();

    SfxMedium* pMed = pShell->GetMedium();
    if (!pMed)
        return OUString();

    return pMed->GetName();
}

bool ScExternalRefManager::isOwnDocument(const OUString& rFile) const
{
    return getOwnDocumentName().equals(rFile);
}

void ScExternalRefManager::convertToAbsName(OUString& rFile) const
{
    // unsaved documents have no AbsName
    ScDocShell* pShell = static_cast<ScDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<ScDocShell>, false));
    while (pShell)
    {
        if (rFile == pShell->GetName())
            return;

        pShell = static_cast<ScDocShell*>(SfxObjectShell::GetNext(*pShell, checkSfxObjectShell<ScDocShell>, false));
    }

    SfxObjectShell* pDocShell = mpDoc->GetDocumentShell();
    rFile = ScGlobal::GetAbsDocName(rFile, pDocShell);
}

sal_uInt16 ScExternalRefManager::getExternalFileId(const OUString& rFile)
{
    vector<SrcFileData>::const_iterator itrBeg = maSrcFiles.begin(), itrEnd = maSrcFiles.end();
    vector<SrcFileData>::const_iterator itr = find_if(itrBeg, itrEnd, FindSrcFileByName(rFile));
    if (itr != itrEnd)
    {
        size_t nId = distance(itrBeg, itr);
        return static_cast<sal_uInt16>(nId);
    }

    SrcFileData aData;
    aData.maFileName = rFile;
    maSrcFiles.push_back(aData);
    return static_cast<sal_uInt16>(maSrcFiles.size() - 1);
}

const OUString* ScExternalRefManager::getExternalFileName(sal_uInt16 nFileId, bool bForceOriginal)
{
    if (nFileId >= maSrcFiles.size())
        return nullptr;

    if (bForceOriginal)
        return &maSrcFiles[nFileId].maFileName;

    maybeCreateRealFileName(nFileId);

    if (!maSrcFiles[nFileId].maRealFileName.isEmpty())
        return &maSrcFiles[nFileId].maRealFileName;

    return &maSrcFiles[nFileId].maFileName;
}

std::vector<OUString> ScExternalRefManager::getAllCachedExternalFileNames() const
{
    std::vector<OUString> aNames;
    aNames.reserve(maSrcFiles.size());
    std::vector<SrcFileData>::const_iterator it = maSrcFiles.begin(), itEnd = maSrcFiles.end();
    for (; it != itEnd; ++it)
    {
        const SrcFileData& rData = *it;
        aNames.push_back(rData.maFileName);
    }

    return aNames;
}

bool ScExternalRefManager::hasExternalFile(sal_uInt16 nFileId) const
{
    return nFileId < maSrcFiles.size();
}

bool ScExternalRefManager::hasExternalFile(const OUString& rFile) const
{
    return ::std::any_of(maSrcFiles.begin(), maSrcFiles.end(), FindSrcFileByName(rFile));
}

const ScExternalRefManager::SrcFileData* ScExternalRefManager::getExternalFileData(sal_uInt16 nFileId) const
{
    if (nFileId >= maSrcFiles.size())
        return nullptr;

    return &maSrcFiles[nFileId];
}

const OUString* ScExternalRefManager::getRealTableName(sal_uInt16 nFileId, const OUString& rTabName) const
{
    return maRefCache.getRealTableName(nFileId, rTabName);
}

const OUString* ScExternalRefManager::getRealRangeName(sal_uInt16 nFileId, const OUString& rRangeName) const
{
    return maRefCache.getRealRangeName(nFileId, rRangeName);
}

template<typename MapContainer>
static void lcl_removeByFileId(sal_uInt16 nFileId, MapContainer& rMap)
{
    typename MapContainer::iterator itr = rMap.find(nFileId);
    if (itr != rMap.end())
    {
        // Close this document shell.
        itr->second.maShell->DoClose();
        rMap.erase(itr);
    }
}

void ScExternalRefManager::clearCache(sal_uInt16 nFileId)
{
    maRefCache.clearCache(nFileId);
}

namespace {

class RefCacheFiller : public sc::ColumnSpanSet::ColumnAction
{
    svl::SharedStringPool& mrStrPool;

    ScExternalRefCache& mrRefCache;
    ScExternalRefCache::TableTypeRef mpRefTab;
    sal_uInt16 mnFileId;
    ScColumn* mpCurCol;
    sc::ColumnBlockConstPosition maBlockPos;

public:
    RefCacheFiller( svl::SharedStringPool& rStrPool, ScExternalRefCache& rRefCache, sal_uInt16 nFileId ) :
        mrStrPool(rStrPool), mrRefCache(rRefCache), mnFileId(nFileId), mpCurCol(nullptr) {}

    virtual void startColumn( ScColumn* pCol ) override
    {
        mpCurCol = pCol;
        if (!mpCurCol)
            return;

        mpCurCol->InitBlockPosition(maBlockPos);
        mpRefTab = mrRefCache.getCacheTable(mnFileId, mpCurCol->GetTab());
    }

    virtual void execute( SCROW nRow1, SCROW nRow2, bool bVal ) override
    {
        if (!mpCurCol || !bVal)
            return;

        if (!mpRefTab)
            return;

        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
        {
            ScExternalRefCache::TokenRef pTok;
            ScRefCellValue aCell = mpCurCol->GetCellValue(maBlockPos, nRow);
            switch (aCell.meType)
            {
                case CELLTYPE_STRING:
                case CELLTYPE_EDIT:
                {
                    OUString aStr = aCell.getString(&mpCurCol->GetDoc());
                    svl::SharedString aSS = mrStrPool.intern(aStr);
                    pTok.reset(new formula::FormulaStringToken(aSS));
                }
                break;
                case CELLTYPE_VALUE:
                    pTok.reset(new formula::FormulaDoubleToken(aCell.mfValue));
                break;
                case CELLTYPE_FORMULA:
                {
                    sc::FormulaResultValue aRes = aCell.mpFormula->GetResult();
                    switch (aRes.meType)
                    {
                        case sc::FormulaResultValue::Value:
                            pTok.reset(new formula::FormulaDoubleToken(aRes.mfValue));
                        break;
                        case sc::FormulaResultValue::String:
                        {
                            // Re-intern the string to the host document pool.
                            svl::SharedString aInterned = mrStrPool.intern(aRes.maString.getString());
                            pTok.reset(new formula::FormulaStringToken(aInterned));
                        }
                        break;
                        case sc::FormulaResultValue::Error:
                        case sc::FormulaResultValue::Invalid:
                        default:
                            pTok.reset(new FormulaErrorToken(errNoValue));
                    }
                }
                break;
                default:
                    pTok.reset(new FormulaErrorToken(errNoValue));
            }

            if (pTok)
            {
                // Cache this cell.
                mpRefTab->setCell(mpCurCol->GetCol(), nRow, pTok, mpCurCol->GetNumberFormat(nRow));
                mpRefTab->setCachedCell(mpCurCol->GetCol(), nRow);
            }
        }
    };
};

}

bool ScExternalRefManager::refreshSrcDocument(sal_uInt16 nFileId)
{
    sc::ColumnSpanSet aCachedArea(false);
    maRefCache.getAllCachedDataSpans(nFileId, aCachedArea);

    OUString aFilter;
    SfxObjectShellRef xDocShell;
    try
    {
        xDocShell = loadSrcDocument(nFileId, aFilter);
    }
    catch ( const css::uno::Exception& ) {}

    if (!xDocShell.Is())
        // Failed to load the document.  Bail out.
        return false;

    ScDocShell& rDocSh = static_cast<ScDocShell&>(*xDocShell);
    ScDocument& rSrcDoc = rDocSh.GetDocument();

    // Clear the existing cache, and refill it.  Make sure we keep the
    // existing cache table instances here.
    maRefCache.clearCacheTables(nFileId);
    RefCacheFiller aAction(mpDoc->GetSharedStringPool(), maRefCache, nFileId);
    aCachedArea.executeColumnAction(rSrcDoc, aAction);

    DocShellMap::iterator it = maDocShells.find(nFileId);
    if (it != maDocShells.end())
    {
        it->second.maShell->DoClose();
        it->second.maShell = xDocShell;
        it->second.maLastAccess = tools::Time(tools::Time::SYSTEM);
    }
    else
    {
        SrcShell aSrcDoc;
        aSrcDoc.maShell = xDocShell;
        aSrcDoc.maLastAccess = tools::Time(tools::Time::SYSTEM);
        cacheNewDocShell(nFileId, aSrcDoc);
    }

    // Update all cells containing names from this source document.
    refreshAllRefCells(nFileId);

    notifyAllLinkListeners(nFileId, LINK_MODIFIED);

    return true;
}

void ScExternalRefManager::breakLink(sal_uInt16 nFileId)
{
    // Turn all formula cells referencing this external document into static
    // cells.
    RefCellMap::iterator itrRefs = maRefCells.find(nFileId);
    if (itrRefs != maRefCells.end())
    {
        // Make a copy because removing the formula cells below will modify
        // the original container.
        RefCellSet aSet = itrRefs->second;
        for_each(aSet.begin(), aSet.end(), ConvertFormulaToStatic(mpDoc));
        maRefCells.erase(nFileId);
    }

    // Remove all named ranges that reference this document.

    // Global named ranges.
    ScRangeName* pRanges = mpDoc->GetRangeName();
    if (pRanges)
        removeRangeNamesBySrcDoc(*pRanges, nFileId);

    // Sheet-local named ranges.
    for (SCTAB i = 0, n = mpDoc->GetTableCount(); i < n; ++i)
    {
        pRanges = mpDoc->GetRangeName(i);
        if (pRanges)
            removeRangeNamesBySrcDoc(*pRanges, nFileId);
    }

    clearCache(nFileId);
    lcl_removeByFileId(nFileId, maDocShells);

    if (maDocShells.empty())
        maSrcDocTimer.Stop();

    LinkedDocMap::iterator itr = maLinkedDocs.find(nFileId);
    if (itr != maLinkedDocs.end())
        itr->second = false;

    notifyAllLinkListeners(nFileId, LINK_BROKEN);
}

void ScExternalRefManager::switchSrcFile(sal_uInt16 nFileId, const OUString& rNewFile, const OUString& rNewFilter)
{
    maSrcFiles[nFileId].maFileName = rNewFile;
    maSrcFiles[nFileId].maRelativeName.clear();
    maSrcFiles[nFileId].maRealFileName.clear();
    if (!maSrcFiles[nFileId].maFilterName.equals(rNewFilter))
    {
        // Filter type has changed.
        maSrcFiles[nFileId].maFilterName = rNewFilter;
        maSrcFiles[nFileId].maFilterOptions.clear();
    }
    refreshSrcDocument(nFileId);
}

void ScExternalRefManager::setRelativeFileName(sal_uInt16 nFileId, const OUString& rRelUrl)
{
    if (nFileId >= maSrcFiles.size())
        return;
    maSrcFiles[nFileId].maRelativeName = rRelUrl;
}

void ScExternalRefManager::setFilterData(sal_uInt16 nFileId, const OUString& rFilterName, const OUString& rOptions)
{
    if (nFileId >= maSrcFiles.size())
        return;
    maSrcFiles[nFileId].maFilterName = rFilterName;
    maSrcFiles[nFileId].maFilterOptions = rOptions;
}

void ScExternalRefManager::clear()
{
    DocShellMap::iterator itrEnd = maDocShells.end();
    for (DocShellMap::iterator itr = maDocShells.begin(); itr != itrEnd; ++itr)
        itr->second.maShell->DoClose();

    maDocShells.clear();
    maSrcDocTimer.Stop();
}

bool ScExternalRefManager::hasExternalData() const
{
    return !maSrcFiles.empty();
}

void ScExternalRefManager::resetSrcFileData(const OUString& rBaseFileUrl)
{
    for (vector<SrcFileData>::iterator itr = maSrcFiles.begin(), itrEnd = maSrcFiles.end();
          itr != itrEnd; ++itr)
    {
        // Re-generate relative file name from the absolute file name.
        OUString aAbsName = itr->maRealFileName;
        if (aAbsName.isEmpty())
            aAbsName = itr->maFileName;

        itr->maRelativeName = URIHelper::simpleNormalizedMakeRelative(
            rBaseFileUrl, aAbsName);
    }
}

void ScExternalRefManager::updateAbsAfterLoad()
{
    OUString aOwn( getOwnDocumentName() );
    for (vector<SrcFileData>::iterator itr = maSrcFiles.begin(), itrEnd = maSrcFiles.end();
          itr != itrEnd; ++itr)
    {
        // update maFileName to the real file name,
        // to be called when the original name is no longer needed (after CompileXML)

        itr->maybeCreateRealFileName( aOwn );
        OUString aReal = itr->maRealFileName;
        if (!aReal.isEmpty())
            itr->maFileName = aReal;
    }
}

void ScExternalRefManager::removeRefCell(ScFormulaCell* pCell)
{
    for_each(maRefCells.begin(), maRefCells.end(), RemoveFormulaCell(pCell));
}

void ScExternalRefManager::addLinkListener(sal_uInt16 nFileId, LinkListener* pListener)
{
    LinkListenerMap::iterator itr = maLinkListeners.find(nFileId);
    if (itr == maLinkListeners.end())
    {
        pair<LinkListenerMap::iterator, bool> r = maLinkListeners.insert(
            LinkListenerMap::value_type(nFileId, LinkListeners()));
        if (!r.second)
        {
            OSL_FAIL("insertion of new link listener list failed");
            return;
        }

        itr = r.first;
    }

    LinkListeners& rList = itr->second;
    rList.insert(pListener);
}

void ScExternalRefManager::removeLinkListener(sal_uInt16 nFileId, LinkListener* pListener)
{
    LinkListenerMap::iterator itr = maLinkListeners.find(nFileId);
    if (itr == maLinkListeners.end())
        // no listeners for a specified file.
        return;

    LinkListeners& rList = itr->second;
    rList.erase(pListener);

    if (rList.empty())
        // No more listeners for this file.  Remove its entry.
        maLinkListeners.erase(itr);
}

void ScExternalRefManager::removeLinkListener(LinkListener* pListener)
{
    LinkListenerMap::iterator itr = maLinkListeners.begin(), itrEnd = maLinkListeners.end();
    for (; itr != itrEnd; ++itr)
        itr->second.erase(pListener);
}

void ScExternalRefManager::notifyAllLinkListeners(sal_uInt16 nFileId, LinkUpdateType eType)
{
    LinkListenerMap::iterator itr = maLinkListeners.find(nFileId);
    if (itr == maLinkListeners.end())
        // no listeners for a specified file.
        return;

    LinkListeners& rList = itr->second;
    for_each(rList.begin(), rList.end(), NotifyLinkListener(nFileId, eType));
}

void ScExternalRefManager::purgeStaleSrcDocument(sal_Int32 nTimeOut)
{
    // To avoid potentially freezing Calc, we close one stale document at a time.
    DocShellMap::iterator itr = maDocShells.begin(), itrEnd = maDocShells.end();
    for (; itr != itrEnd; ++itr)
    {
        // in 100th of a second.
        sal_Int32 nSinceLastAccess = (tools::Time( tools::Time::SYSTEM ) - itr->second.maLastAccess).GetTime();
        if (nSinceLastAccess >= nTimeOut)
        {
            // Timed out.  Let's close this, and exit the loop.
            itr->second.maShell->DoClose();
            maDocShells.erase(itr);
            break;
        }
    }

    if (maDocShells.empty())
        maSrcDocTimer.Stop();
}

sal_uInt32 ScExternalRefManager::getMappedNumberFormat(sal_uInt16 nFileId, sal_uInt32 nNumFmt, const ScDocument* pSrcDoc)
{
    NumFmtMap::iterator itr = maNumFormatMap.find(nFileId);
    if (itr == maNumFormatMap.end())
    {
        // Number formatter map is not initialized for this external document.
        pair<NumFmtMap::iterator, bool> r = maNumFormatMap.insert(
            NumFmtMap::value_type(nFileId, SvNumberFormatterMergeMap()));

        if (!r.second)
            // insertion failed.
            return nNumFmt;

        itr = r.first;
        mpDoc->GetFormatTable()->MergeFormatter( *pSrcDoc->GetFormatTable());
        SvNumberFormatterMergeMap aMap = mpDoc->GetFormatTable()->ConvertMergeTableToMap();
        itr->second.swap(aMap);
    }
    const SvNumberFormatterMergeMap& rMap = itr->second;
    SvNumberFormatterMergeMap::const_iterator itrNumFmt = rMap.find(nNumFmt);
    if (itrNumFmt != rMap.end())
        // mapped value found.
        return itrNumFmt->second;

    return nNumFmt;
}

void ScExternalRefManager::transformUnsavedRefToSavedRef( SfxObjectShell* pShell )
{
    DocShellMap::iterator itr = maUnsavedDocShells.begin();
    while( itr != maUnsavedDocShells.end() )
    {
        if (&(itr->second.maShell) == pShell)
        {
            // found that the shell is marked as unsaved
            OUString aFileURL = pShell->GetMedium()->GetURLObject().GetMainURL(INetURLObject::DECODE_TO_IURI);
            switchSrcFile(itr->first, aFileURL, OUString());
            EndListening(*pShell);
            maUnsavedDocShells.erase(itr++);
        }
    }
}

void ScExternalRefManager::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxEventHint* pEventHint = dynamic_cast<const SfxEventHint*>(&rHint);
    if ( pEventHint )
    {
        sal_uLong nEventId = pEventHint->GetEventId();
        switch ( nEventId )
        {
            case SFX_EVENT_PREPARECLOSEDOC:
                {
                    ScopedVclPtrInstance<WarningBox> aBox( ScDocShell::GetActiveDialogParent(), WinBits( WB_OK ),
                                        ScGlobal::GetRscString( STR_CLOSE_WITH_UNSAVED_REFS ) );
                    aBox->Execute();
                }
                break;
            case SFX_EVENT_SAVEDOCDONE:
            case SFX_EVENT_SAVEASDOCDONE:
                {
                    SfxObjectShell* pObjShell = static_cast<const SfxEventHint&>( rHint ).GetObjShell();
                    transformUnsavedRefToSavedRef(pObjShell);
                }
                break;
            default:
                break;
        }
    }
}

IMPL_LINK_TYPED(ScExternalRefManager, TimeOutHdl, Timer*, pTimer, void)
{
    if (pTimer == &maSrcDocTimer)
        purgeStaleSrcDocument(SRCDOC_LIFE_SPAN);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
