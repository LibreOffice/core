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

#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <unotools/textsearch.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <stdlib.h>
#include <unotools/transliterationwrapper.hxx>
#include <com/sun/star/i18n/KParseTokens.hpp>
#include <com/sun/star/i18n/KParseType.hpp>
#include <sal/log.hxx>

#include <refdata.hxx>
#include <table.hxx>
#include <scitems.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <global.hxx>
#include <stlpool.hxx>
#include <patattr.hxx>
#include <subtotal.hxx>
#include <docoptio.hxx>
#include <markdata.hxx>
#include <rangelst.hxx>
#include <userlist.hxx>
#include <progress.hxx>
#include <cellform.hxx>
#include <queryparam.hxx>
#include <queryentry.hxx>
#include <subtotalparam.hxx>
#include <docpool.hxx>
#include <cellvalue.hxx>
#include <tokenarray.hxx>
#include <mtvcellfunc.hxx>
#include <columnspanset.hxx>
#include <fstalgorithm.hxx>
#include <listenercontext.hxx>
#include <sharedformula.hxx>
#include <stlsheet.hxx>
#include <refhint.hxx>
#include <listenerquery.hxx>
#include <bcaslot.hxx>
#include <reordermap.hxx>
#include <drwlayer.hxx>

#include <svl/sharedstringpool.hxx>

#include <memory>
#include <set>
#include <unordered_set>
#include <vector>
#include <mdds/flat_segment_tree.hpp>

using namespace ::com::sun::star;

namespace naturalsort {

using namespace ::com::sun::star::i18n;

/** Splits a given string into three parts: the prefix, number string, and
    the suffix.

    @param sWhole
    Original string to be split into pieces

    @param sPrefix
    Prefix string that consists of the part before the first number token.
    If no number was found, sPrefix is unchanged.

    @param sSuffix
    String after the last number token.  This may still contain number strings.
    If no number was found, sSuffix is unchanged.

    @param fNum
    Number converted from the middle number string
    If no number was found, fNum is unchanged.

    @return Returns TRUE if a numeral element is found in a given string, or
    FALSE if no numeral element is found.
*/
static bool SplitString( const OUString &sWhole,
    OUString &sPrefix, OUString &sSuffix, double &fNum )
{
    // Get prefix element, search for any digit and stop.
    sal_Int32 nPos = 0;
    while (nPos < sWhole.getLength())
    {
        const sal_uInt16 nType = ScGlobal::getCharClassPtr()->getCharacterType( sWhole, nPos);
        if (nType & KCharacterType::DIGIT)
            break;
        sWhole.iterateCodePoints( &nPos );
    }

    // Return FALSE if no numeral element is found
    if ( nPos == sWhole.getLength() )
        return false;

    // Get numeral element
    const OUString& sUser = ScGlobal::getLocaleDataPtr()->getNumDecimalSep();
    ParseResult aPRNum = ScGlobal::getCharClassPtr()->parsePredefinedToken(
        KParseType::ANY_NUMBER, sWhole, nPos,
        KParseTokens::ANY_NUMBER, "", KParseTokens::ANY_NUMBER, sUser );

    if ( aPRNum.EndPos == nPos )
    {
        SAL_WARN("sc.core","naturalsort::SplitString - digit found but no number parsed, pos " <<
                nPos << " : " << sWhole);
        return false;
    }

    sPrefix = sWhole.copy( 0, nPos );
    fNum = aPRNum.Value;
    sSuffix = sWhole.copy( aPRNum.EndPos );

    return true;
}

/** Naturally compares two given strings.

    This is the main function that should be called externally.  It returns
    either 1, 0, or -1 depending on the comparison result of given two strings.

    @param sInput1
    Input string 1

    @param sInput2
    Input string 2

    @param bCaseSens
    Boolean value for case sensitivity

    @param pData
    Pointer to user defined sort list

    @param pCW
    Pointer to collator wrapper for normal string comparison

    @return Returns 1 if sInput1 is greater, 0 if sInput1 == sInput2, and -1 if
    sInput2 is greater.
*/
static short Compare( const OUString &sInput1, const OUString &sInput2,
               const bool bCaseSens, const ScUserListData* pData, const CollatorWrapper *pCW )
{
    OUString sStr1( sInput1 ), sStr2( sInput2 ), sPre1, sSuf1, sPre2, sSuf2;

    do
    {
        double nNum1, nNum2;
        bool bNumFound1 = SplitString( sStr1, sPre1, sSuf1, nNum1 );
        bool bNumFound2 = SplitString( sStr2, sPre2, sSuf2, nNum2 );

        short nPreRes; // Prefix comparison result
        if ( pData )
        {
            if ( bCaseSens )
            {
                if ( !bNumFound1 || !bNumFound2 )
                    return static_cast<short>(pData->Compare( sStr1, sStr2 ));
                else
                    nPreRes = pData->Compare( sPre1, sPre2 );
            }
            else
            {
                if ( !bNumFound1 || !bNumFound2 )
                    return static_cast<short>(pData->ICompare( sStr1, sStr2 ));
                else
                    nPreRes = pData->ICompare( sPre1, sPre2 );
            }
        }
        else
        {
            if ( !bNumFound1 || !bNumFound2 )
                return static_cast<short>(pCW->compareString( sStr1, sStr2 ));
            else
                nPreRes = static_cast<short>(pCW->compareString( sPre1, sPre2 ));
        }

        // Prefix strings differ.  Return immediately.
        if ( nPreRes != 0 ) return nPreRes;

        if ( nNum1 != nNum2 )
        {
            if ( nNum1 < nNum2 ) return -1;
            return (nNum1 > nNum2) ? 1 : 0;
        }

        // The prefix and the first numerical elements are equal, but the suffix
        // strings may still differ.  Stay in the loop.

        sStr1 = sSuf1;
        sStr2 = sSuf2;

    } while (true);

    return 0;
}

}

namespace {

struct ScSortInfo final
{
    ScRefCellValue maCell;
    SCCOLROW        nOrg;
};

}

class ScSortInfoArray
{
public:

    struct Cell
    {
        ScRefCellValue maCell;
        const sc::CellTextAttr* mpAttr;
        const ScPostIt* mpNote;
        std::vector<SdrObject*> maDrawObjects;
        const ScPatternAttr* mpPattern;

        Cell() : mpAttr(nullptr), mpNote(nullptr), maDrawObjects(), mpPattern(nullptr) {}
    };

    struct Row
    {
        std::vector<Cell> maCells;

        bool mbHidden:1;
        bool mbFiltered:1;

        explicit Row( size_t nColSize ) : maCells(nColSize, Cell()), mbHidden(false), mbFiltered(false) {}
    };

    typedef std::vector<Row> RowsType;

private:
    std::unique_ptr<RowsType> mpRows; /// row-wise data table for sort by row operation.

    std::vector<std::unique_ptr<ScSortInfo[]>> mvppInfo;
    SCCOLROW        nStart;
    SCCOLROW        mnLastIndex; /// index of last non-empty cell position.

    std::vector<SCCOLROW> maOrderIndices;
    bool mbKeepQuery;
    bool mbUpdateRefs;

public:
    ScSortInfoArray(const ScSortInfoArray&) = delete;
    const ScSortInfoArray& operator=(const ScSortInfoArray&) = delete;

    ScSortInfoArray( sal_uInt16 nSorts, SCCOLROW nInd1, SCCOLROW nInd2 ) :
        mvppInfo(nSorts),
        nStart( nInd1 ),
        mnLastIndex(nInd2),
        mbKeepQuery(false),
        mbUpdateRefs(false)
    {
        SCSIZE nCount( nInd2 - nInd1 + 1 );
        if (nSorts)
        {
            for ( sal_uInt16 nSort = 0; nSort < nSorts; nSort++ )
            {
                mvppInfo[nSort].reset(new ScSortInfo[nCount]);
            }
        }

        for (size_t i = 0; i < nCount; ++i)
            maOrderIndices.push_back(i+nStart);
    }

    void SetKeepQuery( bool b ) { mbKeepQuery = b; }

    bool IsKeepQuery() const { return mbKeepQuery; }

    void SetUpdateRefs( bool b ) { mbUpdateRefs = b; }

    bool IsUpdateRefs() const { return mbUpdateRefs; }

    /**
     * Call this only during normal sorting, not from reordering.
     */
    std::unique_ptr<ScSortInfo[]> const & GetFirstArray() const
    {
        return mvppInfo[0];
    }

    /**
     * Call this only during normal sorting, not from reordering.
     */
    ScSortInfo & Get( sal_uInt16 nSort, SCCOLROW nInd )
    {
        return mvppInfo[nSort][ nInd - nStart ];
    }

    /**
     * Call this only during normal sorting, not from reordering.
     */
    void Swap( SCCOLROW nInd1, SCCOLROW nInd2 )
    {
        if (nInd1 == nInd2) // avoid self-move-assign
            return;
        SCSIZE n1 = static_cast<SCSIZE>(nInd1 - nStart);
        SCSIZE n2 = static_cast<SCSIZE>(nInd2 - nStart);
        for ( sal_uInt16 nSort = 0; nSort < static_cast<sal_uInt16>(mvppInfo.size()); nSort++ )
        {
            auto & ppInfo = mvppInfo[nSort];
            std::swap(ppInfo[n1], ppInfo[n2]);
        }

        std::swap(maOrderIndices[n1], maOrderIndices[n2]);

        if (mpRows)
        {
            // Swap rows in data table.
            RowsType& rRows = *mpRows;
            std::swap(rRows[n1], rRows[n2]);
        }
    }

    void SetOrderIndices( const std::vector<SCCOLROW>& rIndices )
    {
        maOrderIndices = rIndices;
    }

    /**
     * @param rIndices indices are actual row positions on the sheet, not an
     *                 offset from the top row.
     */
    void ReorderByRow( const std::vector<SCCOLROW>& rIndices )
    {
        if (!mpRows)
            return;

        RowsType& rRows = *mpRows;

        std::vector<SCCOLROW> aOrderIndices2;
        aOrderIndices2.reserve(rIndices.size());

        RowsType aRows2;
        aRows2.reserve(rRows.size());

        for (const auto& rIndex : rIndices)
        {
            size_t nPos = rIndex - nStart; // switch to an offset to top row.
            aRows2.push_back(rRows[nPos]);
            aOrderIndices2.push_back(maOrderIndices[nPos]);
        }

        rRows.swap(aRows2);
        maOrderIndices.swap(aOrderIndices2);
    }

    sal_uInt16      GetUsedSorts() const { return mvppInfo.size(); }

    SCCOLROW    GetStart() const { return nStart; }
    SCCOLROW GetLast() const { return mnLastIndex; }

    const std::vector<SCCOLROW>& GetOrderIndices() const { return maOrderIndices; }

    RowsType& InitDataRows( size_t nRowSize, size_t nColSize )
    {
        mpRows.reset(new RowsType);
        mpRows->resize(nRowSize, Row(nColSize));
        return *mpRows;
    }

    RowsType* GetDataRows()
    {
        return mpRows.get();
    }
};

namespace {

void initDataRows(
    ScSortInfoArray& rArray, ScTable& rTab, ScColContainer& rCols,
    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    bool bPattern, bool bHiddenFiltered )
{
    // Fill row-wise data table.
    ScSortInfoArray::RowsType& rRows = rArray.InitDataRows(nRow2-nRow1+1, nCol2-nCol1+1);

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
    {
        ScColumn& rCol = rCols[nCol];

        // Skip reordering of cell formats if the whole span is on the same pattern entry.
        bool bUniformPattern = rCol.GetPatternCount(nRow1, nRow2) < 2u;

        sc::ColumnBlockConstPosition aBlockPos;
        rCol.InitBlockPosition(aBlockPos);
        std::map<SCROW, std::vector<SdrObject*>> aRowDrawObjects;
        ScDrawLayer* pDrawLayer = rTab.GetDoc().GetDrawLayer();
        if (pDrawLayer)
            aRowDrawObjects = pDrawLayer->GetObjectsAnchoredToRange(rTab.GetTab(), nCol, nRow1, nRow2);

        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
        {
            ScSortInfoArray::Row& rRow = rRows[nRow-nRow1];
            ScSortInfoArray::Cell& rCell = rRow.maCells[nCol-nCol1];
            rCell.maCell = rCol.GetCellValue(aBlockPos, nRow);
            rCell.mpAttr = rCol.GetCellTextAttr(aBlockPos, nRow);
            rCell.mpNote = rCol.GetCellNote(aBlockPos, nRow);
            if (pDrawLayer)
                rCell.maDrawObjects = aRowDrawObjects[nRow];

            if (!bUniformPattern && bPattern)
                rCell.mpPattern = rCol.GetPattern(nRow);
        }
    }

    if (bHiddenFiltered)
    {
        for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
        {
            ScSortInfoArray::Row& rRow = rRows[nRow-nRow1];
            rRow.mbHidden = rTab.RowHidden(nRow);
            rRow.mbFiltered = rTab.RowFiltered(nRow);
        }
    }
}

}

std::unique_ptr<ScSortInfoArray> ScTable::CreateSortInfoArray( const sc::ReorderParam& rParam )
{
    std::unique_ptr<ScSortInfoArray> pArray;

    if (rParam.mbByRow)
    {
        // Create a sort info array with just the data table.
        SCROW nRow1 = rParam.maSortRange.aStart.Row();
        SCROW nRow2 = rParam.maSortRange.aEnd.Row();
        SCCOL nCol1 = rParam.maSortRange.aStart.Col();
        SCCOL nCol2 = rParam.maSortRange.aEnd.Col();

        pArray.reset(new ScSortInfoArray(0, nRow1, nRow2));
        pArray->SetKeepQuery(rParam.mbHiddenFiltered);
        pArray->SetUpdateRefs(rParam.mbUpdateRefs);

        initDataRows(
            *pArray, *this, aCol, nCol1, nRow1, nCol2, nRow2,
            rParam.mbPattern, rParam.mbHiddenFiltered);
    }
    else
    {
        SCCOLROW nCol1 = rParam.maSortRange.aStart.Col();
        SCCOLROW nCol2 = rParam.maSortRange.aEnd.Col();

        pArray.reset(new ScSortInfoArray(0, nCol1, nCol2));
        pArray->SetKeepQuery(rParam.mbHiddenFiltered);
        pArray->SetUpdateRefs(rParam.mbUpdateRefs);
    }

    return pArray;
}

std::unique_ptr<ScSortInfoArray> ScTable::CreateSortInfoArray(
    const ScSortParam& rSortParam, SCCOLROW nInd1, SCCOLROW nInd2,
    bool bKeepQuery, bool bUpdateRefs )
{
    sal_uInt16 nUsedSorts = 1;
    while ( nUsedSorts < rSortParam.GetSortKeyCount() && rSortParam.maKeyState[nUsedSorts].bDoSort )
        nUsedSorts++;
    std::unique_ptr<ScSortInfoArray> pArray(new ScSortInfoArray( nUsedSorts, nInd1, nInd2 ));
    pArray->SetKeepQuery(bKeepQuery);
    pArray->SetUpdateRefs(bUpdateRefs);

    if ( rSortParam.bByRow )
    {
        for ( sal_uInt16 nSort = 0; nSort < nUsedSorts; nSort++ )
        {
            SCCOL nCol = static_cast<SCCOL>(rSortParam.maKeyState[nSort].nField);
            ScColumn* pCol = &aCol[nCol];
            sc::ColumnBlockConstPosition aBlockPos;
            pCol->InitBlockPosition(aBlockPos);
            for ( SCROW nRow = nInd1; nRow <= nInd2; nRow++ )
            {
                ScSortInfo & rInfo = pArray->Get( nSort, nRow );
                rInfo.maCell = pCol->GetCellValue(aBlockPos, nRow);
                rInfo.nOrg = nRow;
            }
        }

        initDataRows(
            *pArray, *this, aCol, rSortParam.nCol1, nInd1, rSortParam.nCol2, nInd2,
            rSortParam.bIncludePattern, bKeepQuery);
    }
    else
    {
        for ( sal_uInt16 nSort = 0; nSort < nUsedSorts; nSort++ )
        {
            SCROW nRow = rSortParam.maKeyState[nSort].nField;
            for ( SCCOL nCol = static_cast<SCCOL>(nInd1);
                    nCol <= static_cast<SCCOL>(nInd2); nCol++ )
            {
                ScSortInfo & rInfo = pArray->Get( nSort, nCol );
                rInfo.maCell = GetCellValue(nCol, nRow);
                rInfo.nOrg = nCol;
            }
        }
    }
    return pArray;
}

namespace {

struct SortedColumn
{
    typedef mdds::flat_segment_tree<SCROW, const ScPatternAttr*> PatRangeType;

    sc::CellStoreType maCells;
    sc::CellTextAttrStoreType maCellTextAttrs;
    sc::BroadcasterStoreType maBroadcasters;
    sc::CellNoteStoreType maCellNotes;
    std::vector<std::vector<SdrObject*>> maCellDrawObjects;

    PatRangeType maPatterns;
    PatRangeType::const_iterator miPatternPos;

    SortedColumn(const SortedColumn&) = delete;
    const SortedColumn operator=(const SortedColumn&) = delete;

    explicit SortedColumn( size_t nTopEmptyRows, const ScSheetLimits& rSheetLimits ) :
        maCells(nTopEmptyRows),
        maCellTextAttrs(nTopEmptyRows),
        maBroadcasters(nTopEmptyRows),
        maCellNotes(nTopEmptyRows),
        maCellDrawObjects(),
        maPatterns(0, rSheetLimits.GetMaxRowCount(), nullptr),
        miPatternPos(maPatterns.begin()) {}

    void setPattern( SCROW nRow, const ScPatternAttr* pPat )
    {
        miPatternPos = maPatterns.insert(miPatternPos, nRow, nRow+1, pPat).first;
    }
};

struct SortedRowFlags
{
    typedef mdds::flat_segment_tree<SCROW,bool> FlagsType;

    FlagsType maRowsHidden;
    FlagsType maRowsFiltered;
    FlagsType::const_iterator miPosHidden;
    FlagsType::const_iterator miPosFiltered;

    SortedRowFlags(const ScSheetLimits& rSheetLimits) :
        maRowsHidden(0, rSheetLimits.GetMaxRowCount(), false),
        maRowsFiltered(0, rSheetLimits.GetMaxRowCount(), false),
        miPosHidden(maRowsHidden.begin()),
        miPosFiltered(maRowsFiltered.begin()) {}

    void setRowHidden( SCROW nRow, bool b )
    {
        miPosHidden = maRowsHidden.insert(miPosHidden, nRow, nRow+1, b).first;
    }

    void setRowFiltered( SCROW nRow, bool b )
    {
        miPosFiltered = maRowsFiltered.insert(miPosFiltered, nRow, nRow+1, b).first;
    }

    void swap( SortedRowFlags& r )
    {
        maRowsHidden.swap(r.maRowsHidden);
        maRowsFiltered.swap(r.maRowsFiltered);

        // Just reset the position hints.
        miPosHidden = maRowsHidden.begin();
        miPosFiltered = maRowsFiltered.begin();
    }
};

struct PatternSpan
{
    SCROW mnRow1;
    SCROW mnRow2;
    const ScPatternAttr* mpPattern;

    PatternSpan( SCROW nRow1, SCROW nRow2, const ScPatternAttr* pPat ) :
        mnRow1(nRow1), mnRow2(nRow2), mpPattern(pPat) {}
};

}

bool ScTable::IsSortCollatorGlobal() const
{
    return  pSortCollator == ScGlobal::GetCollator() ||
            pSortCollator == ScGlobal::GetCaseCollator();
}

void ScTable::InitSortCollator( const ScSortParam& rPar )
{
    if ( !rPar.aCollatorLocale.Language.isEmpty() )
    {
        if ( !pSortCollator || IsSortCollatorGlobal() )
            pSortCollator = new CollatorWrapper( comphelper::getProcessComponentContext() );
        pSortCollator->loadCollatorAlgorithm( rPar.aCollatorAlgorithm,
            rPar.aCollatorLocale, (rPar.bCaseSens ? 0 : SC_COLLATOR_IGNORES) );
    }
    else
    {   // SYSTEM
        DestroySortCollator();
        pSortCollator = (rPar.bCaseSens ? ScGlobal::GetCaseCollator() :
            ScGlobal::GetCollator());
    }
}

void ScTable::DestroySortCollator()
{
    if ( pSortCollator )
    {
        if ( !IsSortCollatorGlobal() )
            delete pSortCollator;
        pSortCollator = nullptr;
    }
}

namespace {

template<typename Hint, typename ReorderMap, typename Index>
class ReorderNotifier
{
    Hint maHint;
public:
    ReorderNotifier( const ReorderMap& rMap, SCTAB nTab, Index nPos1, Index nPos2 ) :
        maHint(rMap, nTab, nPos1, nPos2) {}

    void operator() ( SvtListener* p )
    {
        p->Notify(maHint);
    }
};

class FormulaGroupPosCollector
{
    sc::RefQueryFormulaGroup& mrQuery;

public:
    explicit FormulaGroupPosCollector( sc::RefQueryFormulaGroup& rQuery ) : mrQuery(rQuery) {}

    void operator() ( const SvtListener* p )
    {
        p->Query(mrQuery);
    }
};

void fillSortedColumnArray(
    std::vector<std::unique_ptr<SortedColumn>>& rSortedCols,
    SortedRowFlags& rRowFlags,
    std::vector<SvtListener*>& rCellListeners,
    ScSortInfoArray* pArray, SCTAB nTab, SCCOL nCol1, SCCOL nCol2, ScProgress* pProgress, const ScTable* pTable )
{
    SCROW nRow1 = pArray->GetStart();
    ScSortInfoArray::RowsType* pRows = pArray->GetDataRows();
    std::vector<SCCOLROW> aOrderIndices = pArray->GetOrderIndices();

    size_t nColCount = nCol2 - nCol1 + 1;
    std::vector<std::unique_ptr<SortedColumn>> aSortedCols; // storage for copied cells.
    SortedRowFlags aRowFlags(pTable->GetDoc().GetSheetLimits());
    aSortedCols.reserve(nColCount);
    for (size_t i = 0; i < nColCount; ++i)
    {
        // In the sorted column container, element positions and row
        // positions must match, else formula cells may mis-behave during
        // grouping.
        aSortedCols.push_back(std::make_unique<SortedColumn>(nRow1, pTable->GetDoc().GetSheetLimits()));
    }

    for (size_t i = 0; i < pRows->size(); ++i)
    {
        ScSortInfoArray::Row& rRow = (*pRows)[i];
        for (size_t j = 0; j < rRow.maCells.size(); ++j)
        {
            ScAddress aCellPos(nCol1 + j, nRow1 + i, nTab);

            ScSortInfoArray::Cell& rCell = rRow.maCells[j];

            sc::CellStoreType& rCellStore = aSortedCols.at(j)->maCells;
            switch (rCell.maCell.meType)
            {
                case CELLTYPE_STRING:
                    assert(rCell.mpAttr);
                    rCellStore.push_back(*rCell.maCell.mpString);
                break;
                case CELLTYPE_VALUE:
                    assert(rCell.mpAttr);
                    rCellStore.push_back(rCell.maCell.mfValue);
                break;
                case CELLTYPE_EDIT:
                    assert(rCell.mpAttr);
                    rCellStore.push_back(rCell.maCell.mpEditText->Clone().release());
                break;
                case CELLTYPE_FORMULA:
                {
                    assert(rCell.mpAttr);
                    ScAddress aOldPos = rCell.maCell.mpFormula->aPos;

                    ScFormulaCell* pNew = rCell.maCell.mpFormula->Clone( aCellPos );
                    if (pArray->IsUpdateRefs())
                    {
                        pNew->CopyAllBroadcasters(*rCell.maCell.mpFormula);
                        pNew->GetCode()->AdjustReferenceOnMovedOrigin(aOldPos, aCellPos);
                    }
                    else
                    {
                        pNew->GetCode()->AdjustReferenceOnMovedOriginIfOtherSheet(aOldPos, aCellPos);
                    }

                    if (!rCellListeners.empty())
                    {
                        // Original source cells will be deleted during
                        // sc::CellStoreType::transfer(), SvtListener is a base
                        // class, so we need to replace it.
                        auto it( ::std::find( rCellListeners.begin(), rCellListeners.end(), rCell.maCell.mpFormula));
                        if (it != rCellListeners.end())
                            *it = pNew;
                    }

                    rCellStore.push_back(pNew);
                }
                break;
                default:
                    //assert(!rCell.mpAttr);
                    // This assert doesn't hold, for example
                    // CopyCellsFromClipHandler may omit copying cells during
                    // PasteSpecial for which CopyTextAttrsFromClipHandler
                    // still copies a CellTextAttr. So if that really is not
                    // expected then fix it there.
                    rCellStore.push_back_empty();
            }

            sc::CellTextAttrStoreType& rAttrStore = aSortedCols.at(j)->maCellTextAttrs;
            if (rCell.mpAttr)
                rAttrStore.push_back(*rCell.mpAttr);
            else
                rAttrStore.push_back_empty();

            if (pArray->IsUpdateRefs())
            {
                // At this point each broadcaster instance is managed by 2
                // containers. We will release those in the original storage
                // below before transferring them to the document.
                const SvtBroadcaster* pBroadcaster = pTable->GetBroadcaster( nCol1 + j, aOrderIndices[i]);
                sc::BroadcasterStoreType& rBCStore = aSortedCols.at(j)->maBroadcasters;
                if (pBroadcaster)
                    // A const pointer would be implicitly converted to a bool type.
                    rBCStore.push_back(const_cast<SvtBroadcaster*>(pBroadcaster));
                else
                    rBCStore.push_back_empty();
            }

            // The same with cell note instances ...
            sc::CellNoteStoreType& rNoteStore = aSortedCols.at(j)->maCellNotes;
            if (rCell.mpNote)
                rNoteStore.push_back(const_cast<ScPostIt*>(rCell.mpNote));
            else
                rNoteStore.push_back_empty();

            // Add cell anchored images
            aSortedCols.at(j)->maCellDrawObjects.push_back(rCell.maDrawObjects);

            if (rCell.mpPattern)
                aSortedCols.at(j)->setPattern(aCellPos.Row(), rCell.mpPattern);
        }

        if (pArray->IsKeepQuery())
        {
            // Hidden and filtered flags are first converted to segments.
            SCROW nRow = nRow1 + i;
            aRowFlags.setRowHidden(nRow, rRow.mbHidden);
            aRowFlags.setRowFiltered(nRow, rRow.mbFiltered);
        }

        if (pProgress)
            pProgress->SetStateOnPercent(i);
    }

    rSortedCols.swap(aSortedCols);
    rRowFlags.swap(aRowFlags);
}

void expandRowRange( ScRange& rRange, SCROW nTop, SCROW nBottom )
{
    if (nTop < rRange.aStart.Row())
        rRange.aStart.SetRow(nTop);

    if (rRange.aEnd.Row() < nBottom)
        rRange.aEnd.SetRow(nBottom);
}

class FormulaCellCollectAction : public sc::ColumnSpanSet::ColumnAction
{
    std::vector<ScFormulaCell*>& mrCells;
    ScColumn* mpCol;

public:
    explicit FormulaCellCollectAction( std::vector<ScFormulaCell*>& rCells ) :
        mrCells(rCells), mpCol(nullptr) {}

    virtual void startColumn( ScColumn* pCol ) override
    {
        mpCol = pCol;
    }

    virtual void execute( SCROW nRow1, SCROW nRow2, bool bVal ) override
    {
        assert(mpCol);

        if (!bVal)
            return;

        mpCol->CollectFormulaCells(mrCells, nRow1, nRow2);
    }
};

class ListenerStartAction : public sc::ColumnSpanSet::ColumnAction
{
    ScColumn* mpCol;

    std::shared_ptr<sc::ColumnBlockPositionSet> mpPosSet;
    sc::StartListeningContext maStartCxt;
    sc::EndListeningContext maEndCxt;

public:
    explicit ListenerStartAction( ScDocument& rDoc ) :
        mpCol(nullptr),
        mpPosSet(std::make_shared<sc::ColumnBlockPositionSet>(rDoc)),
        maStartCxt(rDoc, mpPosSet),
        maEndCxt(rDoc, mpPosSet) {}

    virtual void startColumn( ScColumn* pCol ) override
    {
        mpCol = pCol;
    }

    virtual void execute( SCROW nRow1, SCROW nRow2, bool bVal ) override
    {
        assert(mpCol);

        if (!bVal)
            return;

        mpCol->StartListeningFormulaCells(maStartCxt, maEndCxt, nRow1, nRow2);
    }
};

}

void ScTable::SortReorderByColumn(
    const ScSortInfoArray* pArray, SCROW nRow1, SCROW nRow2, bool bPattern, ScProgress* pProgress )
{
    SCCOLROW nStart = pArray->GetStart();
    SCCOLROW nLast = pArray->GetLast();

    std::vector<SCCOLROW> aIndices = pArray->GetOrderIndices();
    size_t nCount = aIndices.size();

    // Cut formula grouping at row and reference boundaries before the reordering.
    ScRange aSortRange(nStart, nRow1, nTab, nLast, nRow2, nTab);
    for (SCCOL nCol = nStart; nCol <= static_cast<SCCOL>(nLast); ++nCol)
        aCol[nCol].SplitFormulaGroupByRelativeRef(aSortRange);

    // Collect all listeners of cell broadcasters of sorted range.
    std::vector<SvtListener*> aCellListeners;

    if (!pArray->IsUpdateRefs())
    {
        // Collect listeners of cell broadcasters.
        for (SCCOL nCol = nStart; nCol <= static_cast<SCCOL>(nLast); ++nCol)
            aCol[nCol].CollectListeners(aCellListeners, nRow1, nRow2);

        // Remove any duplicate listener entries.  We must ensure that we
        // notify each unique listener only once.
        std::sort(aCellListeners.begin(), aCellListeners.end());
        aCellListeners.erase(std::unique(aCellListeners.begin(), aCellListeners.end()), aCellListeners.end());

        // Notify the cells' listeners to stop listening.
        /* TODO: for performance this could be enhanced to stop and later
         * restart only listening to within the reordered range and keep
         * listening to everything outside untouched. */
        sc::RefStopListeningHint aHint;
        for (auto const & l : aCellListeners)
            l->Notify(aHint);
    }

    // table to keep track of column index to position in the index table.
    std::vector<SCCOLROW> aPosTable(nCount);
    for (size_t i = 0; i < nCount; ++i)
        aPosTable[aIndices[i]-nStart] = i;

    SCCOLROW nDest = nStart;
    for (size_t i = 0; i < nCount; ++i, ++nDest)
    {
        SCCOLROW nSrc = aIndices[i];
        if (nDest != nSrc)
        {
            aCol[nDest].Swap(aCol[nSrc], nRow1, nRow2, bPattern);

            // Update the position of the index that was originally equal to nDest.
            size_t nPos = aPosTable[nDest-nStart];
            aIndices[nPos] = nSrc;
            aPosTable[nSrc-nStart] = nPos;
        }

        if (pProgress)
            pProgress->SetStateOnPercent(i);
    }

    // Reset formula cell positions which became out-of-sync after column reordering.
    bool bUpdateRefs = pArray->IsUpdateRefs();
    for (SCCOL nCol = nStart; nCol <= static_cast<SCCOL>(nLast); ++nCol)
        aCol[nCol].ResetFormulaCellPositions(nRow1, nRow2, bUpdateRefs);

    if (pArray->IsUpdateRefs())
    {
        // Set up column reorder map (for later broadcasting of reference updates).
        sc::ColRowReorderMapType aColMap;
        const std::vector<SCCOLROW>& rOldIndices = pArray->GetOrderIndices();
        for (size_t i = 0, n = rOldIndices.size(); i < n; ++i)
        {
            SCCOL nNew = i + nStart;
            SCCOL nOld = rOldIndices[i];
            aColMap.emplace(nOld, nNew);
        }

        // Collect all listeners within sorted range ahead of time.
        std::vector<SvtListener*> aListeners;

        for (SCCOL nCol = nStart; nCol <= static_cast<SCCOL>(nLast); ++nCol)
            aCol[nCol].CollectListeners(aListeners, nRow1, nRow2);

        // Get all area listeners that listen on one column within the range
        // and end their listening.
        ScRange aMoveRange( nStart, nRow1, nTab, nLast, nRow2, nTab);
        std::vector<sc::AreaListener> aAreaListeners = rDocument.GetBASM()->GetAllListeners(
                aMoveRange, sc::AreaOverlapType::OneColumnInside);
        {
            for (auto& rAreaListener : aAreaListeners)
            {
                rDocument.EndListeningArea(rAreaListener.maArea, rAreaListener.mbGroupListening, rAreaListener.mpListener);
                aListeners.push_back( rAreaListener.mpListener);
            }
        }

        // Remove any duplicate listener entries and notify all listeners
        // afterward.  We must ensure that we notify each unique listener only
        // once.
        std::sort(aListeners.begin(), aListeners.end());
        aListeners.erase(std::unique(aListeners.begin(), aListeners.end()), aListeners.end());
        ReorderNotifier<sc::RefColReorderHint, sc::ColRowReorderMapType, SCCOL> aFunc(aColMap, nTab, nRow1, nRow2);
        std::for_each(aListeners.begin(), aListeners.end(), aFunc);

        // Re-start area listeners on the reordered columns.
        {
            for (auto& rAreaListener : aAreaListeners)
            {
                ScRange aNewRange = rAreaListener.maArea;
                sc::ColRowReorderMapType::const_iterator itCol = aColMap.find( aNewRange.aStart.Col());
                if (itCol != aColMap.end())
                {
                    aNewRange.aStart.SetCol( itCol->second);
                    aNewRange.aEnd.SetCol( itCol->second);
                }
                rDocument.StartListeningArea(aNewRange, rAreaListener.mbGroupListening, rAreaListener.mpListener);
            }
        }
    }
    else    // !(pArray->IsUpdateRefs())
    {
        // Notify the cells' listeners to (re-)start listening.
        sc::RefStartListeningHint aHint;
        for (auto const & l : aCellListeners)
            l->Notify(aHint);
    }

    // Re-join formulas at row boundaries now that all the references have
    // been adjusted for column reordering.
    for (SCCOL nCol = nStart; nCol <= static_cast<SCCOL>(nLast); ++nCol)
    {
        sc::CellStoreType& rCells = aCol[nCol].maCells;
        sc::CellStoreType::position_type aPos = rCells.position(nRow1);
        sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
        if (nRow2 < rDocument.MaxRow())
        {
            aPos = rCells.position(aPos.first, nRow2+1);
            sc::SharedFormulaUtil::joinFormulaCellAbove(aPos);
        }
    }
}

void ScTable::SortReorderByRow(
    ScSortInfoArray* pArray, SCCOL nCol1, SCCOL nCol2, ScProgress* pProgress )
{
    assert(!pArray->IsUpdateRefs());

    if (nCol2 < nCol1)
        return;

    SCROW nRow1 = pArray->GetStart();
    SCROW nRow2 = pArray->GetLast();

    // Collect all listeners of cell broadcasters of sorted range.
    std::vector<SvtListener*> aCellListeners;

    // When the update ref mode is disabled, we need to detach all formula
    // cells in the sorted range before reordering, and re-start them
    // afterward.
    {
        sc::EndListeningContext aCxt(rDocument);
        DetachFormulaCells(aCxt, nCol1, nRow1, nCol2, nRow2);
    }

    // Collect listeners of cell broadcasters.
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].CollectListeners(aCellListeners, nRow1, nRow2);

    // Remove any duplicate listener entries.  We must ensure that we notify
    // each unique listener only once.
    std::sort(aCellListeners.begin(), aCellListeners.end());
    aCellListeners.erase(std::unique(aCellListeners.begin(), aCellListeners.end()), aCellListeners.end());

    // Notify the cells' listeners to stop listening.
    /* TODO: for performance this could be enhanced to stop and later
     * restart only listening to within the reordered range and keep
     * listening to everything outside untouched. */
    {
        sc::RefStopListeningHint aHint;
        for (auto const & l : aCellListeners)
            l->Notify(aHint);
    }

    // Split formula groups at the sort range boundaries (if applicable).
    std::vector<SCROW> aRowBounds;
    aRowBounds.reserve(2);
    aRowBounds.push_back(nRow1);
    aRowBounds.push_back(nRow2+1);
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        SplitFormulaGroups(nCol, aRowBounds);

    // Cells in the data rows only reference values in the document. Make
    // a copy before updating the document.
    std::vector<std::unique_ptr<SortedColumn>> aSortedCols; // storage for copied cells.
    SortedRowFlags aRowFlags(GetDoc().GetSheetLimits());
    fillSortedColumnArray(aSortedCols, aRowFlags, aCellListeners, pArray, nTab, nCol1, nCol2, pProgress, this);

    for (size_t i = 0, n = aSortedCols.size(); i < n; ++i)
    {
        SCCOL nThisCol = i + nCol1;

        {
            sc::CellStoreType& rDest = aCol[nThisCol].maCells;
            sc::CellStoreType& rSrc = aSortedCols[i]->maCells;
            rSrc.transfer(nRow1, nRow2, rDest, nRow1);
        }

        {
            sc::CellTextAttrStoreType& rDest = aCol[nThisCol].maCellTextAttrs;
            sc::CellTextAttrStoreType& rSrc = aSortedCols[i]->maCellTextAttrs;
            rSrc.transfer(nRow1, nRow2, rDest, nRow1);
        }

        {
            sc::CellNoteStoreType& rSrc = aSortedCols[i]->maCellNotes;
            sc::CellNoteStoreType& rDest = aCol[nThisCol].maCellNotes;

            // Do the same as broadcaster storage transfer (to prevent double deletion).
            rDest.release_range(nRow1, nRow2);
            rSrc.transfer(nRow1, nRow2, rDest, nRow1);
            aCol[nThisCol].UpdateNoteCaptions(nRow1, nRow2);
        }

        // Update draw object positions
        aCol[nThisCol].UpdateDrawObjects(aSortedCols[i]->maCellDrawObjects, nRow1, nRow2);

        {
            // Get all row spans where the pattern is not NULL.
            std::vector<PatternSpan> aSpans =
                sc::toSpanArrayWithValue<SCROW,const ScPatternAttr*,PatternSpan>(
                    aSortedCols[i]->maPatterns);

            for (const auto& rSpan : aSpans)
            {
                assert(rSpan.mpPattern); // should never be NULL.
                rDocument.GetPool()->Put(*rSpan.mpPattern);
            }

            for (const auto& rSpan : aSpans)
            {
                aCol[nThisCol].SetPatternArea(rSpan.mnRow1, rSpan.mnRow2, *rSpan.mpPattern);
                rDocument.GetPool()->Remove(*rSpan.mpPattern);
            }
        }

        aCol[nThisCol].CellStorageModified();
    }

    if (pArray->IsKeepQuery())
    {
        aRowFlags.maRowsHidden.build_tree();
        aRowFlags.maRowsFiltered.build_tree();

        // Remove all flags in the range first.
        SetRowHidden(nRow1, nRow2, false);
        SetRowFiltered(nRow1, nRow2, false);

        std::vector<sc::RowSpan> aSpans =
            sc::toSpanArray<SCROW,sc::RowSpan>(aRowFlags.maRowsHidden, nRow1);

        for (const auto& rSpan : aSpans)
            SetRowHidden(rSpan.mnRow1, rSpan.mnRow2, true);

        aSpans = sc::toSpanArray<SCROW,sc::RowSpan>(aRowFlags.maRowsFiltered, nRow1);

        for (const auto& rSpan : aSpans)
            SetRowFiltered(rSpan.mnRow1, rSpan.mnRow2, true);
    }

    // Notify the cells' listeners to (re-)start listening.
    {
        sc::RefStartListeningHint aHint;
        for (auto const & l : aCellListeners)
            l->Notify(aHint);
    }

    // Re-group columns in the sorted range too.
    for (SCCOL i = nCol1; i <= nCol2; ++i)
        aCol[i].RegroupFormulaCells();

    {
        sc::StartListeningContext aCxt(rDocument);
        AttachFormulaCells(aCxt, nCol1, nRow1, nCol2, nRow2);
    }
}

void ScTable::SortReorderByRowRefUpdate(
    ScSortInfoArray* pArray, SCCOL nCol1, SCCOL nCol2, ScProgress* pProgress )
{
    assert(pArray->IsUpdateRefs());

    if (nCol2 < nCol1)
        return;

    SCROW nRow1 = pArray->GetStart();
    SCROW nRow2 = pArray->GetLast();

    ScRange aMoveRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab);
    sc::ColumnSpanSet aGrpListenerRanges;

    {
        // Get the range of formula group listeners within sorted range (if any).
        sc::QueryRange aQuery;

        ScBroadcastAreaSlotMachine* pBASM = rDocument.GetBASM();
        std::vector<sc::AreaListener> aGrpListeners =
            pBASM->GetAllListeners(
                aMoveRange, sc::AreaOverlapType::InsideOrOverlap, sc::ListenerGroupType::Group);

        {
            for (const auto& rGrpListener : aGrpListeners)
            {
                assert(rGrpListener.mbGroupListening);
                SvtListener* pGrpLis = rGrpListener.mpListener;
                pGrpLis->Query(aQuery);
                rDocument.EndListeningArea(rGrpListener.maArea, rGrpListener.mbGroupListening, pGrpLis);
            }
        }

        ScRangeList aTmp;
        aQuery.swapRanges(aTmp);

        // If the range is within the sorted range, we need to expand its rows
        // to the top and bottom of the sorted range, since the formula cells
        // could be anywhere in the sorted range after reordering.
        for (size_t i = 0, n = aTmp.size(); i < n; ++i)
        {
            ScRange aRange = aTmp[i];
            if (!aMoveRange.Intersects(aRange))
            {
                // Doesn't overlap with the sorted range at all.
                aGrpListenerRanges.set(GetDoc(), aRange, true);
                continue;
            }

            if (aMoveRange.aStart.Col() <= aRange.aStart.Col() && aRange.aEnd.Col() <= aMoveRange.aEnd.Col())
            {
                // Its column range is within the column range of the sorted range.
                expandRowRange(aRange, aMoveRange.aStart.Row(), aMoveRange.aEnd.Row());
                aGrpListenerRanges.set(GetDoc(), aRange, true);
                continue;
            }

            // It intersects with the sorted range, but its column range is
            // not within the column range of the sorted range.  Split it into
            // 2 ranges.
            ScRange aR1 = aRange;
            ScRange aR2 = aRange;
            if (aRange.aStart.Col() < aMoveRange.aStart.Col())
            {
                // Left half is outside the sorted range while the right half is inside.
                aR1.aEnd.SetCol(aMoveRange.aStart.Col()-1);
                aR2.aStart.SetCol(aMoveRange.aStart.Col());
                expandRowRange(aR2, aMoveRange.aStart.Row(), aMoveRange.aEnd.Row());
            }
            else
            {
                // Left half is inside the sorted range while the right half is outside.
                aR1.aEnd.SetCol(aMoveRange.aEnd.Col()-1);
                aR2.aStart.SetCol(aMoveRange.aEnd.Col());
                expandRowRange(aR1, aMoveRange.aStart.Row(), aMoveRange.aEnd.Row());
            }

            aGrpListenerRanges.set(GetDoc(), aR1, true);
            aGrpListenerRanges.set(GetDoc(), aR2, true);
        }
    }

    // Split formula groups at the sort range boundaries (if applicable).
    std::vector<SCROW> aRowBounds;
    aRowBounds.reserve(2);
    aRowBounds.push_back(nRow1);
    aRowBounds.push_back(nRow2+1);
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        SplitFormulaGroups(nCol, aRowBounds);

    // Cells in the data rows only reference values in the document. Make
    // a copy before updating the document.
    std::vector<std::unique_ptr<SortedColumn>> aSortedCols; // storage for copied cells.
    SortedRowFlags aRowFlags(GetDoc().GetSheetLimits());
    std::vector<SvtListener*> aListenersDummy;
    fillSortedColumnArray(aSortedCols, aRowFlags, aListenersDummy, pArray, nTab, nCol1, nCol2, pProgress, this);

    for (size_t i = 0, n = aSortedCols.size(); i < n; ++i)
    {
        SCCOL nThisCol = i + nCol1;

        {
            sc::CellStoreType& rDest = aCol[nThisCol].maCells;
            sc::CellStoreType& rSrc = aSortedCols[i]->maCells;
            rSrc.transfer(nRow1, nRow2, rDest, nRow1);
        }

        {
            sc::CellTextAttrStoreType& rDest = aCol[nThisCol].maCellTextAttrs;
            sc::CellTextAttrStoreType& rSrc = aSortedCols[i]->maCellTextAttrs;
            rSrc.transfer(nRow1, nRow2, rDest, nRow1);
        }

        {
            sc::BroadcasterStoreType& rSrc = aSortedCols[i]->maBroadcasters;
            sc::BroadcasterStoreType& rDest = aCol[nThisCol].maBroadcasters;

            // Release current broadcasters first, to prevent them from getting deleted.
            rDest.release_range(nRow1, nRow2);

            // Transfer sorted broadcaster segment to the document.
            rSrc.transfer(nRow1, nRow2, rDest, nRow1);
        }

        {
            sc::CellNoteStoreType& rSrc = aSortedCols[i]->maCellNotes;
            sc::CellNoteStoreType& rDest = aCol[nThisCol].maCellNotes;

            // Do the same as broadcaster storage transfer (to prevent double deletion).
            rDest.release_range(nRow1, nRow2);
            rSrc.transfer(nRow1, nRow2, rDest, nRow1);
            aCol[nThisCol].UpdateNoteCaptions(nRow1, nRow2);
        }

        // Update draw object positions
        aCol[nThisCol].UpdateDrawObjects(aSortedCols[i]->maCellDrawObjects, nRow1, nRow2);

        {
            // Get all row spans where the pattern is not NULL.
            std::vector<PatternSpan> aSpans =
                sc::toSpanArrayWithValue<SCROW,const ScPatternAttr*,PatternSpan>(
                    aSortedCols[i]->maPatterns);

            for (const auto& rSpan : aSpans)
            {
                assert(rSpan.mpPattern); // should never be NULL.
                rDocument.GetPool()->Put(*rSpan.mpPattern);
            }

            for (const auto& rSpan : aSpans)
            {
                aCol[nThisCol].SetPatternArea(rSpan.mnRow1, rSpan.mnRow2, *rSpan.mpPattern);
                rDocument.GetPool()->Remove(*rSpan.mpPattern);
            }
        }

        aCol[nThisCol].CellStorageModified();
    }

    if (pArray->IsKeepQuery())
    {
        aRowFlags.maRowsHidden.build_tree();
        aRowFlags.maRowsFiltered.build_tree();

        // Remove all flags in the range first.
        SetRowHidden(nRow1, nRow2, false);
        SetRowFiltered(nRow1, nRow2, false);

        std::vector<sc::RowSpan> aSpans =
            sc::toSpanArray<SCROW,sc::RowSpan>(aRowFlags.maRowsHidden, nRow1);

        for (const auto& rSpan : aSpans)
            SetRowHidden(rSpan.mnRow1, rSpan.mnRow2, true);

        aSpans = sc::toSpanArray<SCROW,sc::RowSpan>(aRowFlags.maRowsFiltered, nRow1);

        for (const auto& rSpan : aSpans)
            SetRowFiltered(rSpan.mnRow1, rSpan.mnRow2, true);
    }

    // Set up row reorder map (for later broadcasting of reference updates).
    sc::ColRowReorderMapType aRowMap;
    const std::vector<SCCOLROW>& rOldIndices = pArray->GetOrderIndices();
    for (size_t i = 0, n = rOldIndices.size(); i < n; ++i)
    {
        SCROW nNew = i + nRow1;
        SCROW nOld = rOldIndices[i];
        aRowMap.emplace(nOld, nNew);
    }

    // Collect all listeners within sorted range ahead of time.
    std::vector<SvtListener*> aListeners;

    // Collect listeners of cell broadcasters.
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].CollectListeners(aListeners, nRow1, nRow2);

    // Get all area listeners that listen on one row within the range and end
    // their listening.
    std::vector<sc::AreaListener> aAreaListeners = rDocument.GetBASM()->GetAllListeners(
            aMoveRange, sc::AreaOverlapType::OneRowInside);
    {
        for (auto& rAreaListener : aAreaListeners)
        {
            rDocument.EndListeningArea(rAreaListener.maArea, rAreaListener.mbGroupListening, rAreaListener.mpListener);
            aListeners.push_back( rAreaListener.mpListener);
        }
    }

    {
        // Get all formula cells from the former group area listener ranges.

        std::vector<ScFormulaCell*> aFCells;
        FormulaCellCollectAction aAction(aFCells);
        aGrpListenerRanges.executeColumnAction(rDocument, aAction);

        aListeners.insert( aListeners.end(), aFCells.begin(), aFCells.end() );
    }

    // Remove any duplicate listener entries.  We must ensure that we notify
    // each unique listener only once.
    std::sort(aListeners.begin(), aListeners.end());
    aListeners.erase(std::unique(aListeners.begin(), aListeners.end()), aListeners.end());

    // Collect positions of all shared formula cells outside the sorted range,
    // and make them unshared before notifying them.
    sc::RefQueryFormulaGroup aFormulaGroupPos;
    aFormulaGroupPos.setSkipRange(ScRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab));

    std::for_each(aListeners.begin(), aListeners.end(), FormulaGroupPosCollector(aFormulaGroupPos));
    const sc::RefQueryFormulaGroup::TabsType& rGroupTabs = aFormulaGroupPos.getAllPositions();
    for (const auto& [rTab, rCols] : rGroupTabs)
    {
        for (const auto& [nCol, rCol] : rCols)
        {
            std::vector<SCROW> aBounds(rCol);
            rDocument.UnshareFormulaCells(rTab, nCol, aBounds);
        }
    }

    // Notify the listeners to update their references.
    ReorderNotifier<sc::RefRowReorderHint, sc::ColRowReorderMapType, SCROW> aFunc(aRowMap, nTab, nCol1, nCol2);
    std::for_each(aListeners.begin(), aListeners.end(), aFunc);

    // Re-group formulas in affected columns.
    for (const auto& [rTab, rCols] : rGroupTabs)
    {
        for (const auto& rEntry : rCols)
            rDocument.RegroupFormulaCells(rTab, rEntry.first);
    }

    // Re-start area listeners on the reordered rows.
    for (const auto& rAreaListener : aAreaListeners)
    {
        ScRange aNewRange = rAreaListener.maArea;
        sc::ColRowReorderMapType::const_iterator itRow = aRowMap.find( aNewRange.aStart.Row());
        if (itRow != aRowMap.end())
        {
            aNewRange.aStart.SetRow( itRow->second);
            aNewRange.aEnd.SetRow( itRow->second);
        }
        rDocument.StartListeningArea(aNewRange, rAreaListener.mbGroupListening, rAreaListener.mpListener);
    }

    // Re-group columns in the sorted range too.
    for (SCCOL i = nCol1; i <= nCol2; ++i)
        aCol[i].RegroupFormulaCells();

    {
        // Re-start area listeners on the old group listener ranges.
        ListenerStartAction aAction(rDocument);
        aGrpListenerRanges.executeColumnAction(rDocument, aAction);
    }
}

short ScTable::CompareCell(
    sal_uInt16 nSort,
    ScRefCellValue& rCell1, SCCOL nCell1Col, SCROW nCell1Row,
    ScRefCellValue& rCell2, SCCOL nCell2Col, SCROW nCell2Row ) const
{
    short nRes = 0;

    CellType eType1 = rCell1.meType, eType2 = rCell2.meType;

    if (!rCell1.isEmpty())
    {
        if (!rCell2.isEmpty())
        {
            bool bErr1 = false;
            bool bStr1 = ( eType1 != CELLTYPE_VALUE );
            if (eType1 == CELLTYPE_FORMULA)
            {
                if (rCell1.mpFormula->GetErrCode() != FormulaError::NONE)
                {
                    bErr1 = true;
                    bStr1 = false;
                }
                else if (rCell1.mpFormula->IsValue())
                {
                    bStr1 = false;
                }
            }

            bool bErr2 = false;
            bool bStr2 = ( eType2 != CELLTYPE_VALUE );
            if (eType2 == CELLTYPE_FORMULA)
            {
                if (rCell2.mpFormula->GetErrCode() != FormulaError::NONE)
                {
                    bErr2 = true;
                    bStr2 = false;
                }
                else if (rCell2.mpFormula->IsValue())
                {
                    bStr2 = false;
                }
            }

            if ( bStr1 && bStr2 )           // only compare strings as strings!
            {
                OUString aStr1;
                OUString aStr2;
                if (eType1 == CELLTYPE_STRING)
                    aStr1 = rCell1.mpString->getString();
                else
                    GetString(nCell1Col, nCell1Row, aStr1);
                if (eType2 == CELLTYPE_STRING)
                    aStr2 = rCell2.mpString->getString();
                else
                    GetString(nCell2Col, nCell2Row, aStr2);

                bool bUserDef     = aSortParam.bUserDef;        // custom sort order
                bool bNaturalSort = aSortParam.bNaturalSort;    // natural sort
                bool bCaseSens    = aSortParam.bCaseSens;       // case sensitivity

                ScUserList* pList = ScGlobal::GetUserList();
                if (bUserDef && pList && pList->size() > aSortParam.nUserIndex )
                {
                    const ScUserListData& rData = (*pList)[aSortParam.nUserIndex];

                    if ( bNaturalSort )
                        nRes = naturalsort::Compare( aStr1, aStr2, bCaseSens, &rData, pSortCollator );
                    else
                    {
                        if ( bCaseSens )
                            nRes = sal::static_int_cast<short>( rData.Compare(aStr1, aStr2) );
                        else
                            nRes = sal::static_int_cast<short>( rData.ICompare(aStr1, aStr2) );
                    }

                }
                if (!bUserDef)
                {
                    if ( bNaturalSort )
                        nRes = naturalsort::Compare( aStr1, aStr2, bCaseSens, nullptr, pSortCollator );
                    else
                        nRes = static_cast<short>( pSortCollator->compareString( aStr1, aStr2 ) );
                }
            }
            else if ( bStr1 )               // String <-> Number or Error
            {
                if (bErr2)
                    nRes = -1;              // String in front of Error
                else
                    nRes = 1;               // Number in front of String
            }
            else if ( bStr2 )               // Number or Error <-> String
            {
                if (bErr1)
                    nRes = 1;               // String in front of Error
                else
                    nRes = -1;              // Number in front of String
            }
            else if (bErr1 && bErr2)
            {
                // nothing, two Errors are equal
            }
            else if (bErr1)                 // Error <-> Number
            {
                nRes = 1;                   // Number in front of Error
            }
            else if (bErr2)                 // Number <-> Error
            {
                nRes = -1;                  // Number in front of Error
            }
            else                            // Mixed numbers
            {
                double nVal1 = rCell1.getValue();
                double nVal2 = rCell2.getValue();
                if (nVal1 < nVal2)
                    nRes = -1;
                else if (nVal1 > nVal2)
                    nRes = 1;
            }
            if ( !aSortParam.maKeyState[nSort].bAscending )
                nRes = -nRes;
        }
        else
            nRes = -1;
    }
    else
    {
        if (!rCell2.isEmpty())
            nRes = 1;
        else
            nRes = 0;                   // both empty
    }
    return nRes;
}

short ScTable::Compare( ScSortInfoArray* pArray, SCCOLROW nIndex1, SCCOLROW nIndex2 ) const
{
    short nRes;
    sal_uInt16 nSort = 0;
    do
    {
        ScSortInfo& rInfo1 = pArray->Get( nSort, nIndex1 );
        ScSortInfo& rInfo2 = pArray->Get( nSort, nIndex2 );
        if ( aSortParam.bByRow )
            nRes = CompareCell( nSort,
                rInfo1.maCell, static_cast<SCCOL>(aSortParam.maKeyState[nSort].nField), rInfo1.nOrg,
                rInfo2.maCell, static_cast<SCCOL>(aSortParam.maKeyState[nSort].nField), rInfo2.nOrg );
        else
            nRes = CompareCell( nSort,
                rInfo1.maCell, static_cast<SCCOL>(rInfo1.nOrg), aSortParam.maKeyState[nSort].nField,
                rInfo2.maCell, static_cast<SCCOL>(rInfo2.nOrg), aSortParam.maKeyState[nSort].nField );
    } while ( nRes == 0 && ++nSort < pArray->GetUsedSorts() );
    if( nRes == 0 )
    {
        ScSortInfo& rInfo1 = pArray->Get( 0, nIndex1 );
        ScSortInfo& rInfo2 = pArray->Get( 0, nIndex2 );
        if( rInfo1.nOrg < rInfo2.nOrg )
            nRes = -1;
        else if( rInfo1.nOrg > rInfo2.nOrg )
            nRes = 1;
    }
    return nRes;
}

void ScTable::QuickSort( ScSortInfoArray* pArray, SCCOLROW nLo, SCCOLROW nHi )
{
    if ((nHi - nLo) == 1)
    {
        if (Compare(pArray, nLo, nHi) > 0)
            pArray->Swap( nLo, nHi );
    }
    else
    {
        SCCOLROW ni = nLo;
        SCCOLROW nj = nHi;
        do
        {
            while ((ni <= nHi) && (Compare(pArray, ni, nLo)) < 0)
                ni++;
            while ((nj >= nLo) && (Compare(pArray, nLo, nj)) < 0)
                nj--;
            if (ni <= nj)
            {
                if (ni != nj)
                    pArray->Swap( ni, nj );
                ni++;
                nj--;
            }
        } while (ni < nj);
        if ((nj - nLo) < (nHi - ni))
        {
            if (nLo < nj)
                QuickSort(pArray, nLo, nj);
            if (ni < nHi)
                QuickSort(pArray, ni, nHi);
        }
        else
        {
            if (ni < nHi)
                QuickSort(pArray, ni, nHi);
            if (nLo < nj)
                QuickSort(pArray, nLo, nj);
        }
    }
}

short ScTable::Compare(SCCOLROW nIndex1, SCCOLROW nIndex2) const
{
    short nRes;
    sal_uInt16 nSort = 0;
    const sal_uInt32 nMaxSorts = aSortParam.GetSortKeyCount();
    if (aSortParam.bByRow)
    {
        do
        {
            SCCOL nCol = static_cast<SCCOL>(aSortParam.maKeyState[nSort].nField);
            ScRefCellValue aCell1 = aCol[nCol].GetCellValue(nIndex1);
            ScRefCellValue aCell2 = aCol[nCol].GetCellValue(nIndex2);
            nRes = CompareCell(nSort, aCell1, nCol, nIndex1, aCell2, nCol, nIndex2);
        } while ( nRes == 0 && ++nSort < nMaxSorts && aSortParam.maKeyState[nSort].bDoSort );
    }
    else
    {
        do
        {
            SCROW nRow = aSortParam.maKeyState[nSort].nField;
            ScRefCellValue aCell1 = aCol[nIndex1].GetCellValue(nRow);
            ScRefCellValue aCell2 = aCol[nIndex2].GetCellValue(nRow);
            nRes = CompareCell( nSort, aCell1, static_cast<SCCOL>(nIndex1),
                    nRow, aCell2, static_cast<SCCOL>(nIndex2), nRow );
        } while ( nRes == 0 && ++nSort < nMaxSorts && aSortParam.maKeyState[nSort].bDoSort );
    }
    return nRes;
}

bool ScTable::IsSorted( SCCOLROW nStart, SCCOLROW nEnd ) const   // over aSortParam
{
    for (SCCOLROW i=nStart; i<nEnd; i++)
    {
        if (Compare( i, i+1 ) > 0)
            return false;
    }
    return true;
}

void ScTable::DecoladeRow( ScSortInfoArray* pArray, SCROW nRow1, SCROW nRow2 )
{
    SCROW nRow;
    int nMax = nRow2 - nRow1;
    for (SCROW i = nRow1; (i + 4) <= nRow2; i += 4)
    {
        nRow = comphelper::rng::uniform_int_distribution(0, nMax-1);
        pArray->Swap(i, nRow1 + nRow);
    }
}

void ScTable::Sort(
    const ScSortParam& rSortParam, bool bKeepQuery, bool bUpdateRefs,
    ScProgress* pProgress, sc::ReorderParam* pUndo )
{
    InitSortCollator( rSortParam );
    bGlobalKeepQuery = bKeepQuery;

    if (pUndo)
    {
        // Copy over the basic sort parameters.
        pUndo->mbByRow = rSortParam.bByRow;
        pUndo->mbPattern = rSortParam.bIncludePattern;
        pUndo->mbHiddenFiltered = bKeepQuery;
        pUndo->mbUpdateRefs = bUpdateRefs;
        pUndo->mbHasHeaders = rSortParam.bHasHeader;
    }

    // It is assumed that the data area has already been trimmed as necessary.

    aSortParam = rSortParam;    // must be assigned before calling IsSorted()
    if (rSortParam.bByRow)
    {
        SCROW nLastRow = rSortParam.nRow2;
        SCROW nRow1 = (rSortParam.bHasHeader ? rSortParam.nRow1 + 1 : rSortParam.nRow1);
        if (nRow1 < nLastRow && !IsSorted(nRow1, nLastRow))
        {
            if(pProgress)
                pProgress->SetState( 0, nLastRow-nRow1 );

            std::unique_ptr<ScSortInfoArray> pArray(CreateSortInfoArray(aSortParam, nRow1, nLastRow, bKeepQuery, bUpdateRefs));

            if ( nLastRow - nRow1 > 255 )
                DecoladeRow(pArray.get(), nRow1, nLastRow);

            QuickSort(pArray.get(), nRow1, nLastRow);
            if (pArray->IsUpdateRefs())
                SortReorderByRowRefUpdate(pArray.get(), aSortParam.nCol1, aSortParam.nCol2, pProgress);
            else
                SortReorderByRow(pArray.get(), aSortParam.nCol1, aSortParam.nCol2, pProgress);

            if (pUndo)
            {
                pUndo->maSortRange = ScRange(rSortParam.nCol1, nRow1, nTab, rSortParam.nCol2, nLastRow, nTab);
                pUndo->maOrderIndices = pArray->GetOrderIndices();
            }
        }
    }
    else
    {
        SCCOL nLastCol = rSortParam.nCol2;
        SCCOL nCol1 = (rSortParam.bHasHeader ? rSortParam.nCol1 + 1 : rSortParam.nCol1);
        if (nCol1 < nLastCol && !IsSorted(nCol1, nLastCol))
        {
            if(pProgress)
                pProgress->SetState( 0, nLastCol-nCol1 );

            std::unique_ptr<ScSortInfoArray> pArray(CreateSortInfoArray(aSortParam, nCol1, nLastCol, bKeepQuery, bUpdateRefs));

            QuickSort(pArray.get(), nCol1, nLastCol);
            SortReorderByColumn(pArray.get(), aSortParam.nRow1, aSortParam.nRow2, aSortParam.bIncludePattern, pProgress);

            if (pUndo)
            {
                pUndo->maSortRange = ScRange(nCol1, aSortParam.nRow1, nTab, nLastCol, aSortParam.nRow2, nTab);
                pUndo->maOrderIndices = pArray->GetOrderIndices();
            }
        }
    }
    DestroySortCollator();
}

void ScTable::Reorder( const sc::ReorderParam& rParam )
{
    if (rParam.maOrderIndices.empty())
        return;

    std::unique_ptr<ScSortInfoArray> pArray(CreateSortInfoArray(rParam));
    if (!pArray)
        return;

    if (rParam.mbByRow)
    {
        // Re-play sorting from the known sort indices.
        pArray->ReorderByRow(rParam.maOrderIndices);
        if (pArray->IsUpdateRefs())
            SortReorderByRowRefUpdate(
                pArray.get(), rParam.maSortRange.aStart.Col(), rParam.maSortRange.aEnd.Col(), nullptr);
        else
            SortReorderByRow(
                pArray.get(), rParam.maSortRange.aStart.Col(), rParam.maSortRange.aEnd.Col(), nullptr);
    }
    else
    {
        // Ordering by column is much simpler.  Just set the order indices and we are done.
        pArray->SetOrderIndices(rParam.maOrderIndices);
        SortReorderByColumn(
            pArray.get(), rParam.maSortRange.aStart.Row(), rParam.maSortRange.aEnd.Row(),
            rParam.mbPattern, nullptr);
    }
}

namespace {

class SubTotalRowFinder
{
    const ScTable& mrTab;
    const ScSubTotalParam& mrParam;

public:
    SubTotalRowFinder(const ScTable& rTab, const ScSubTotalParam& rParam) :
        mrTab(rTab), mrParam(rParam) {}

    bool operator() (size_t nRow, const ScFormulaCell* pCell)
    {
        if (!pCell->IsSubTotal())
            return false;

        SCCOL nStartCol = mrParam.nCol1;
        SCCOL nEndCol = mrParam.nCol2;

        for (SCCOL nCol : mrTab.GetColumnsRange(0, nStartCol - 1))
        {
            if (mrTab.HasData(nCol, nRow))
                return true;
        }
        for (SCCOL nCol : mrTab.GetColumnsRange(nEndCol + 1, mrTab.GetDoc().MaxCol()))
        {
            if (mrTab.HasData(nCol, nRow))
                return true;
        }
        return false;
    }
};

}

bool ScTable::TestRemoveSubTotals( const ScSubTotalParam& rParam )
{
    SCCOL nStartCol = rParam.nCol1;
    SCROW nStartRow = rParam.nRow1 + 1;     // Header
    SCCOL nEndCol   = ClampToAllocatedColumns(rParam.nCol2);
    SCROW nEndRow    = rParam.nRow2;

    for (SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol)
    {
        const sc::CellStoreType& rCells = aCol[nCol].maCells;
        SubTotalRowFinder aFunc(*this, rParam);
        std::pair<sc::CellStoreType::const_iterator,size_t> aPos =
            sc::FindFormula(rCells, nStartRow, nEndRow, aFunc);
        if (aPos.first != rCells.end())
            return true;
    }
    return false;
}

namespace {

struct RemoveSubTotalsHandler
{
    std::set<SCROW> aRemoved;

    void operator() (size_t nRow, const ScFormulaCell* p)
    {
        if (p->IsSubTotal())
            aRemoved.insert(nRow);
    }
};

}

void ScTable::RemoveSubTotals( ScSubTotalParam& rParam )
{
    SCCOL nStartCol = rParam.nCol1;
    SCROW nStartRow = rParam.nRow1 + 1;     // Header
    SCCOL nEndCol   = ClampToAllocatedColumns(rParam.nCol2);
    SCROW nEndRow    = rParam.nRow2;        // will change

    RemoveSubTotalsHandler aFunc;
    for (SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol)
    {
        const sc::CellStoreType& rCells = aCol[nCol].maCells;
        sc::ParseFormula(rCells.begin(), rCells, nStartRow, nEndRow, aFunc);
    }

    auto& aRows = aFunc.aRemoved;

    std::for_each(aRows.rbegin(), aRows.rend(), [this](const SCROW nRow) {
            RemoveRowBreak(nRow+1, false, true);
            rDocument.DeleteRow(0, nTab, rDocument.MaxCol(), nTab, nRow, 1);
        });

    rParam.nRow2 -= aRows.size();
}

//  Delete hard number formats (for result formulas)

static void lcl_RemoveNumberFormat( ScTable* pTab, SCCOL nCol, SCROW nRow )
{
    const ScPatternAttr* pPattern = pTab->GetPattern( nCol, nRow );
    if ( pPattern->GetItemSet().GetItemState( ATTR_VALUE_FORMAT, false )
            == SfxItemState::SET )
    {
        auto pNewPattern = std::make_unique<ScPatternAttr>( *pPattern );
        SfxItemSet& rSet = pNewPattern->GetItemSet();
        rSet.ClearItem( ATTR_VALUE_FORMAT );
        rSet.ClearItem( ATTR_LANGUAGE_FORMAT );
        pTab->SetPattern( nCol, nRow, std::move(pNewPattern) );
    }
}

namespace {

struct RowEntry
{
    sal_uInt16  nGroupNo;
    SCROW   nSubStartRow;
    SCROW   nDestRow;
    SCROW   nFuncStart;
    SCROW   nFuncEnd;
};

}

static const char* lcl_GetSubTotalStrId(int id)
{
    switch ( id )
    {
        case SUBTOTAL_FUNC_AVE:     return STR_FUN_TEXT_AVG;
        case SUBTOTAL_FUNC_CNT:
        case SUBTOTAL_FUNC_CNT2:    return STR_FUN_TEXT_COUNT;
        case SUBTOTAL_FUNC_MAX:     return STR_FUN_TEXT_MAX;
        case SUBTOTAL_FUNC_MIN:     return STR_FUN_TEXT_MIN;
        case SUBTOTAL_FUNC_PROD:    return STR_FUN_TEXT_PRODUCT;
        case SUBTOTAL_FUNC_STD:
        case SUBTOTAL_FUNC_STDP:    return STR_FUN_TEXT_STDDEV;
        case SUBTOTAL_FUNC_SUM:     return STR_FUN_TEXT_SUM;
        case SUBTOTAL_FUNC_VAR:
        case SUBTOTAL_FUNC_VARP:    return STR_FUN_TEXT_VAR;
        default:
        {
             return STR_EMPTYDATA;
            // added to avoid warnings
        }
    }
}

//      new intermediate results
//      rParam.nRow2 is changed!

bool ScTable::DoSubTotals( ScSubTotalParam& rParam )
{
    SCCOL nStartCol = rParam.nCol1;
    SCROW nStartRow = rParam.nRow1 + 1;     // Header
    SCCOL nEndCol   = rParam.nCol2;
    SCROW nEndRow    = rParam.nRow2;        // will change
    sal_uInt16 i;

    //  Remove empty rows at the end
    //  so that all exceeding (rDocument.MaxRow()) can be found by InsertRow (#35180#)
    //  If sorted, all empty rows are at the end.
    SCSIZE nEmpty = GetEmptyLinesInBlock( nStartCol, nStartRow, nEndCol, nEndRow, DIR_BOTTOM );
    nEndRow -= nEmpty;

    sal_uInt16 nLevelCount = 0;             // Number of levels
    bool bDoThis = true;
    for (i=0; i<MAXSUBTOTAL && bDoThis; i++)
        if (rParam.bGroupActive[i])
            nLevelCount = i+1;
        else
            bDoThis = false;

    if (nLevelCount==0)                 // do nothing
        return true;

    SCCOL*          nGroupCol = rParam.nField;  // columns which will be used when grouping

    //  With (blank) as a separate category, subtotal rows from
    //  the other columns must always be tested
    //  (previously only when a column occurred more than once)
    bool bTestPrevSub = ( nLevelCount > 1 );

    OUString  aSubString;

    bool bIgnoreCase = !rParam.bCaseSens;

    OUString aCompString[MAXSUBTOTAL];

                                //TODO: sort?

    ScStyleSheet* pStyle = static_cast<ScStyleSheet*>(rDocument.GetStyleSheetPool()->Find(
                                ScResId(STR_STYLENAME_RESULT), SfxStyleFamily::Para ));

    bool bSpaceLeft = true;                                         // Success when inserting?

    // For performance reasons collect formula entries so their
    // references don't have to be tested for updates each time a new row is
    // inserted
    RowEntry aRowEntry;
    ::std::vector< RowEntry > aRowVector;

    for (sal_uInt16 nLevel=0; nLevel<nLevelCount && bSpaceLeft; nLevel++)
    {
        aRowEntry.nGroupNo = nLevelCount - nLevel - 1;

        // how many results per level
        SCCOL nResCount         = rParam.nSubTotals[aRowEntry.nGroupNo];
        // result functions
        ScSubTotalFunc* pResFunc = rParam.pFunctions[aRowEntry.nGroupNo];

        if (nResCount > 0)                                      // otherwise only sort
        {
            for (i=0; i<=aRowEntry.nGroupNo; i++)
            {
                GetString( nGroupCol[i], nStartRow, aSubString );
                if ( bIgnoreCase )
                    aCompString[i] = ScGlobal::getCharClassPtr()->uppercase( aSubString );
                else
                    aCompString[i] = aSubString;
            }                                                   // aSubString stays on the last

            bool bBlockVis = false;             // group visible?
            aRowEntry.nSubStartRow = nStartRow;
            for (SCROW nRow=nStartRow; nRow<=nEndRow+1 && bSpaceLeft; nRow++)
            {
                bool bChanged;
                if (nRow>nEndRow)
                    bChanged = true;
                else
                {
                    bChanged = false;
                    OUString aString;
                    for (i=0; i<=aRowEntry.nGroupNo && !bChanged; i++)
                    {
                        GetString( nGroupCol[i], nRow, aString );
                        if (bIgnoreCase)
                            aString = ScGlobal::getCharClassPtr()->uppercase(aString);
                        //  when sorting, blanks are separate group
                        //  otherwise blank cells are allowed below
                        bChanged = ( ( !aString.isEmpty() || rParam.bDoSort ) &&
                                        aString != aCompString[i] );
                    }
                    if ( bChanged && bTestPrevSub )
                    {
                        // No group change on rows that will contain subtotal formulas
                        bChanged = std::none_of(aRowVector.begin(), aRowVector.end(),
                            [&nRow](const RowEntry& rEntry) { return rEntry.nDestRow == nRow; });
                    }
                }
                if ( bChanged )
                {
                    aRowEntry.nDestRow   = nRow;
                    aRowEntry.nFuncStart = aRowEntry.nSubStartRow;
                    aRowEntry.nFuncEnd   = nRow-1;

                    bSpaceLeft = rDocument.InsertRow( 0, nTab, rDocument.MaxCol(), nTab,
                            aRowEntry.nDestRow, 1 );
                    DBShowRow( aRowEntry.nDestRow, bBlockVis );
                    if ( rParam.bPagebreak && nRow < rDocument.MaxRow() &&
                            aRowEntry.nSubStartRow != nStartRow && nLevel == 0)
                        SetRowBreak(aRowEntry.nSubStartRow, false, true);

                    if (bSpaceLeft)
                    {
                        for ( auto& rRowEntry : aRowVector)
                        {
                            if ( aRowEntry.nDestRow <= rRowEntry.nSubStartRow )
                                ++rRowEntry.nSubStartRow;
                            if ( aRowEntry.nDestRow <= rRowEntry.nDestRow )
                                ++rRowEntry.nDestRow;
                            if ( aRowEntry.nDestRow <= rRowEntry.nFuncStart )
                                ++rRowEntry.nFuncStart;
                            if ( aRowEntry.nDestRow <= rRowEntry.nFuncEnd )
                                ++rRowEntry.nFuncEnd;
                        }
                        // collect formula positions
                        aRowVector.push_back( aRowEntry );

                        OUString aOutString = aSubString;
                        if (aOutString.isEmpty())
                            aOutString = ScResId( STR_EMPTYDATA );
                        aOutString += " ";
                        const char* pStrId = STR_TABLE_ERGEBNIS;
                        if ( nResCount == 1 )
                            pStrId = lcl_GetSubTotalStrId(pResFunc[0]);
                        aOutString += ScResId(pStrId);
                        SetString( nGroupCol[aRowEntry.nGroupNo], aRowEntry.nDestRow, nTab, aOutString );
                        ApplyStyle( nGroupCol[aRowEntry.nGroupNo], aRowEntry.nDestRow, pStyle );

                        ++nRow;
                        ++nEndRow;
                        aRowEntry.nSubStartRow = nRow;
                        for (i=0; i<=aRowEntry.nGroupNo; i++)
                        {
                            GetString( nGroupCol[i], nRow, aSubString );
                            if ( bIgnoreCase )
                                aCompString[i] = ScGlobal::getCharClassPtr()->uppercase( aSubString );
                            else
                                aCompString[i] = aSubString;
                        }
                    }
                }
                bBlockVis = !RowFiltered(nRow);
            }
        }
    }

    if (!aRowVector.empty())
    {
        // generate global total
        SCROW nGlobalStartRow = aRowVector[0].nSubStartRow;
        SCROW nGlobalStartFunc = aRowVector[0].nFuncStart;
        SCROW nGlobalEndRow = 0;
        SCROW nGlobalEndFunc = 0;
        for (const auto& rRowEntry : aRowVector)
        {
            nGlobalEndRow = (nGlobalEndRow < rRowEntry.nDestRow) ? rRowEntry.nDestRow : nGlobalEndRow;
            nGlobalEndFunc = (nGlobalEndFunc < rRowEntry.nFuncEnd) ? rRowEntry.nFuncEnd : nGlobalEndRow;
        }

        for (sal_uInt16 nLevel = 0; nLevel<nLevelCount; nLevel++)
        {
            const sal_uInt16 nGroupNo = nLevelCount - nLevel - 1;
            const ScSubTotalFunc* pResFunc = rParam.pFunctions[nGroupNo];
            if (!pResFunc)
            {
                // No subtotal function given for this group => no formula or
                // label and do not insert a row.
                continue;
            }

            // increment end row
            nGlobalEndRow++;

            // add row entry for formula
            aRowEntry.nGroupNo = nGroupNo;
            aRowEntry.nSubStartRow = nGlobalStartRow;
            aRowEntry.nFuncStart = nGlobalStartFunc;
            aRowEntry.nDestRow = nGlobalEndRow;
            aRowEntry.nFuncEnd = nGlobalEndFunc;

            // increment row
            nGlobalEndFunc++;

            bSpaceLeft = rDocument.InsertRow(0, nTab, rDocument.MaxCol(), nTab, aRowEntry.nDestRow, 1);

            if (bSpaceLeft)
            {
                aRowVector.push_back(aRowEntry);
                nEndRow++;
                DBShowRow(aRowEntry.nDestRow, true);

                // insert label
                OUString label = ScResId(STR_TABLE_GRAND) + " " + ScResId(lcl_GetSubTotalStrId(pResFunc[0]));
                SetString(nGroupCol[aRowEntry.nGroupNo], aRowEntry.nDestRow, nTab, label);
                ApplyStyle(nGroupCol[aRowEntry.nGroupNo], aRowEntry.nDestRow, pStyle);
            }
        }
    }

    // now insert the formulas
    ScComplexRefData aRef;
    aRef.InitFlags();
    aRef.Ref1.SetAbsTab(nTab);
    aRef.Ref2.SetAbsTab(nTab);
    for (const auto& rRowEntry : aRowVector)
    {
        SCCOL nResCount         = rParam.nSubTotals[rRowEntry.nGroupNo];
        SCCOL* nResCols         = rParam.pSubTotals[rRowEntry.nGroupNo];
        ScSubTotalFunc* pResFunc = rParam.pFunctions[rRowEntry.nGroupNo];
        for ( SCCOL nResult=0; nResult < nResCount; ++nResult )
        {
            aRef.Ref1.SetAbsCol(nResCols[nResult]);
            aRef.Ref1.SetAbsRow(rRowEntry.nFuncStart);
            aRef.Ref2.SetAbsCol(nResCols[nResult]);
            aRef.Ref2.SetAbsRow(rRowEntry.nFuncEnd);

            ScTokenArray aArr(rDocument);
            aArr.AddOpCode( ocSubTotal );
            aArr.AddOpCode( ocOpen );
            aArr.AddDouble( static_cast<double>(pResFunc[nResult]) );
            aArr.AddOpCode( ocSep );
            aArr.AddDoubleReference( aRef );
            aArr.AddOpCode( ocClose );
            aArr.AddOpCode( ocStop );
            ScFormulaCell* pCell = new ScFormulaCell(
                rDocument, ScAddress(nResCols[nResult], rRowEntry.nDestRow, nTab), aArr);
            if ( rParam.bIncludePattern )
                pCell->SetNeedNumberFormat(true);

            SetFormulaCell(nResCols[nResult], rRowEntry.nDestRow, pCell);
            if ( nResCols[nResult] != nGroupCol[rRowEntry.nGroupNo] )
            {
                ApplyStyle( nResCols[nResult], rRowEntry.nDestRow, pStyle );

                lcl_RemoveNumberFormat( this, nResCols[nResult], rRowEntry.nDestRow );
            }
        }

    }

    //TODO: according to setting, shift intermediate-sum rows up?

    //TODO: create Outlines directly?

    if (bSpaceLeft)
        DoAutoOutline( nStartCol, nStartRow, nEndCol, nEndRow );

    rParam.nRow2 = nEndRow;                 // new end
    return bSpaceLeft;
}

namespace {

class QueryEvaluator
{
    ScDocument& mrDoc;
    svl::SharedStringPool& mrStrPool;
    const ScTable& mrTab;
    const ScQueryParam& mrParam;
    bool mpTestEqualCondition;
    utl::TransliterationWrapper* mpTransliteration;
    CollatorWrapper* mpCollator;
    const bool mbMatchWholeCell;
    const bool mbCaseSensitive;

    static bool isPartialTextMatchOp(const ScQueryEntry& rEntry)
    {
        switch (rEntry.eOp)
        {
            // these operators can only be used with textural comparisons.
            case SC_CONTAINS:
            case SC_DOES_NOT_CONTAIN:
            case SC_BEGINS_WITH:
            case SC_ENDS_WITH:
            case SC_DOES_NOT_BEGIN_WITH:
            case SC_DOES_NOT_END_WITH:
                return true;
            default:
                ;
        }
        return false;
    }

    static bool isTextMatchOp(const ScQueryEntry& rEntry)
    {
        if (isPartialTextMatchOp(rEntry))
            return true;

        switch (rEntry.eOp)
        {
            // these operators can be used for either textural or value comparison.
            case SC_EQUAL:
            case SC_NOT_EQUAL:
                return true;
            default:
                ;
        }
        return false;
    }

    bool isRealWildOrRegExp(const ScQueryEntry& rEntry) const
    {
        if (mrParam.eSearchType == utl::SearchParam::SearchType::Normal)
            return false;

        return isTextMatchOp(rEntry);
    }

    bool isTestWildOrRegExp(const ScQueryEntry& rEntry) const
    {
        if (!mpTestEqualCondition)
            return false;

        if (mrParam.eSearchType == utl::SearchParam::SearchType::Normal)
            return false;

        return (rEntry.eOp == SC_LESS_EQUAL || rEntry.eOp == SC_GREATER_EQUAL);
    }

    void setupTransliteratorIfNeeded()
    {
        if (!mpTransliteration)
            mpTransliteration = mrParam.bCaseSens ? ScGlobal::GetCaseTransliteration() : ScGlobal::GetpTransliteration();
    }

    void setupCollatorIfNeeded()
    {
        if (!mpCollator)
            mpCollator = mrParam.bCaseSens ? ScGlobal::GetCaseCollator() : ScGlobal::GetCollator();
    }

public:
    QueryEvaluator(ScDocument& rDoc, const ScTable& rTab, const ScQueryParam& rParam,
                   bool pTestEqualCondition) :
        mrDoc(rDoc),
        mrStrPool(rDoc.GetSharedStringPool()),
        mrTab(rTab),
        mrParam(rParam),
        mpTestEqualCondition(pTestEqualCondition),
        mpTransliteration(nullptr),
        mpCollator(nullptr),
        mbMatchWholeCell(rDoc.GetDocOptions().IsMatchWholeCell()),
        mbCaseSensitive( rParam.bCaseSens )
    {
    }

    bool isQueryByValue(const ScQueryEntry& rEntry,
        const ScQueryEntry::Item& rItem, SCCOL nCol, SCROW nRow, ScRefCellValue& rCell)
    {
        if (rItem.meType == ScQueryEntry::ByString || isPartialTextMatchOp(rEntry))
            return false;

        if (!rCell.isEmpty())
        {
            if (rCell.meType == CELLTYPE_FORMULA && rCell.mpFormula->GetErrCode() != FormulaError::NONE)
                // Error values are compared as string.
                return false;

            return rCell.hasNumeric();
        }

        return mrTab.HasValueData(nCol, nRow);
    }

    bool isQueryByString(
        const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem,
        SCCOL nCol, SCROW nRow, const ScRefCellValue& rCell)
    {
        if (isTextMatchOp(rEntry))
            return true;

        if (rItem.meType != ScQueryEntry::ByString)
            return false;

        if (!rCell.isEmpty())
            return rCell.hasString();

        return mrTab.HasStringData(nCol, nRow);
    }

    std::pair<bool,bool> compareByValue(
        const ScRefCellValue& rCell, SCCOL nCol, SCROW nRow,
        const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem,
        const ScInterpreterContext* pContext)
    {
        bool bOk = false;
        bool bTestEqual = false;
        double nCellVal;
        if (!rCell.isEmpty())
        {
            switch (rCell.meType)
            {
                case CELLTYPE_VALUE :
                    nCellVal = rCell.mfValue;
                break;
                case CELLTYPE_FORMULA :
                    nCellVal = rCell.mpFormula->GetValue();
                break;
                default:
                    nCellVal = 0.0;
            }

        }
        else
            nCellVal = mrTab.GetValue(nCol, nRow);

        /* NOTE: lcl_PrepareQuery() prepares a filter query such that if a
         * date+time format was queried rEntry.bQueryByDate is not set. In
         * case other queries wanted to use this mechanism they should do
         * the same, in other words only if rEntry.nVal is an integer value
         * rEntry.bQueryByDate should be true and the time fraction be
         * stripped here. */
        if (rItem.meType == ScQueryEntry::ByDate)
        {
            sal_uInt32 nNumFmt = pContext ? mrTab.GetNumberFormat(*pContext, ScAddress(nCol, nRow, mrTab.GetTab())) :
                mrTab.GetNumberFormat(nCol, nRow);
            SvNumberFormatter* pFormatter = pContext ? pContext->GetFormatTable() : mrDoc.GetFormatTable();
            const SvNumberformat* pEntry = pFormatter->GetEntry(nNumFmt);
            if (pEntry)
            {
                SvNumFormatType nNumFmtType = pEntry->GetType();
                /* NOTE: Omitting the check for absence of
                 * css::util::NumberFormat::TIME would include also date+time formatted
                 * values of the same day. That may be desired in some
                 * cases, querying all time values of a day, but confusing
                 * in other cases. A user can always setup a standard
                 * filter query for x >= date AND x < date+1 */
                if ((nNumFmtType & SvNumFormatType::DATE) && !(nNumFmtType & SvNumFormatType::TIME))
                {
                    // The format is of date type.  Strip off the time
                    // element.
                    nCellVal = ::rtl::math::approxFloor(nCellVal);
                }
            }
        }

        switch (rEntry.eOp)
        {
            case SC_EQUAL :
                bOk = ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            case SC_LESS :
                bOk = (nCellVal < rItem.mfVal) && !::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            case SC_GREATER :
                bOk = (nCellVal > rItem.mfVal) && !::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            case SC_LESS_EQUAL :
                bOk = (nCellVal < rItem.mfVal) || ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                if ( bOk && mpTestEqualCondition )
                    bTestEqual = ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            case SC_GREATER_EQUAL :
                bOk = (nCellVal > rItem.mfVal) || ::rtl::math::approxEqual( nCellVal, rItem.mfVal);
                if ( bOk && mpTestEqualCondition )
                    bTestEqual = ::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            case SC_NOT_EQUAL :
                bOk = !::rtl::math::approxEqual(nCellVal, rItem.mfVal);
                break;
            default:
            {
                // added to avoid warnings
            }
        }

        return std::pair<bool,bool>(bOk, bTestEqual);
    }

   std::pair<bool,bool> compareByString(
        ScRefCellValue& rCell, SCROW nRow, const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem,
        const ScInterpreterContext* pContext)
    {
        if (!rCell.isEmpty())
        {
            if (rCell.meType == CELLTYPE_FORMULA && rCell.mpFormula->GetErrCode() != FormulaError::NONE)
            {
                // Error cell is evaluated as string (for now).
                const svl::SharedString aCellStr = mrStrPool.intern(ScGlobal::GetErrorString(rCell.mpFormula->GetErrCode()));
                return compareByStringComparator(rEntry, rItem, &aCellStr, nullptr);
            }
            else if (rCell.meType == CELLTYPE_STRING)
            {
                return compareByStringComparator(rEntry, rItem, rCell.mpString, nullptr);
            }
            else
            {
                sal_uInt32 nFormat = pContext ? mrTab.GetNumberFormat( *pContext, ScAddress(static_cast<SCCOL>(rEntry.nField), nRow, mrTab.GetTab()) ) :
                    mrTab.GetNumberFormat( static_cast<SCCOL>(rEntry.nField), nRow );
                OUString aStr;
                SvNumberFormatter* pFormatter = pContext ? pContext->GetFormatTable() : mrDoc.GetFormatTable();
                ScCellFormat::GetInputString(rCell, nFormat, aStr, *pFormatter, mrDoc);
                return compareByStringComparator(rEntry, rItem, nullptr, &aStr);
            }
        }
        else
        {
            OUString aStr;
            mrTab.GetInputString(static_cast<SCCOL>(rEntry.nField), nRow, aStr);
            return compareByStringComparator(rEntry, rItem, nullptr, &aStr);
        }
    }

    // Called from compareByString() method, where different sources of strings are checked.
    // The value is placed inside one parameter: [pValueSource1] or [pValueSource2] but never in both.
    std::pair<bool,bool> compareByStringComparator(const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem,
        const svl::SharedString* pValueSource1, const OUString * pValueSource2)
    {
        bool bOk = false;
        bool bTestEqual = false;
        bool bMatchWholeCell = mbMatchWholeCell;
        if (isPartialTextMatchOp(rEntry))
            // may have to do partial textural comparison.
            bMatchWholeCell = false;

        const bool bRealWildOrRegExp = isRealWildOrRegExp(rEntry);
        const bool bTestWildOrRegExp = isTestWildOrRegExp(rEntry);

        // [pValueSource1] or [pValueSource2] but never both of them or none of them
        assert((pValueSource1 != nullptr) != (pValueSource2 != nullptr));

        if ( bRealWildOrRegExp || bTestWildOrRegExp )
        {
            const OUString & rValue = pValueSource1 ? pValueSource1->getString() : *pValueSource2;

            sal_Int32 nStart = 0;
            sal_Int32 nEnd   = rValue.getLength();

            // from 614 on, nEnd is behind the found text
            bool bMatch = false;
            if ( rEntry.eOp == SC_ENDS_WITH || rEntry.eOp == SC_DOES_NOT_END_WITH )
            {
                nEnd = 0;
                nStart = rValue.getLength();
                bMatch = rEntry.GetSearchTextPtr( mrParam.eSearchType, mrParam.bCaseSens, bMatchWholeCell )
                    ->SearchBackward(rValue, &nStart, &nEnd);
            }
            else
            {
                bMatch = rEntry.GetSearchTextPtr( mrParam.eSearchType, mrParam.bCaseSens, bMatchWholeCell )
                    ->SearchForward(rValue, &nStart, &nEnd);
            }
            if ( bMatch && bMatchWholeCell
                    && (nStart != 0 || nEnd != rValue.getLength()) )
                bMatch = false;    // RegExp must match entire cell string
            if ( bRealWildOrRegExp )
            {
                switch (rEntry.eOp)
                {
                    case SC_EQUAL:
                    case SC_CONTAINS:
                        bOk = bMatch;
                        break;
                    case SC_NOT_EQUAL:
                    case SC_DOES_NOT_CONTAIN:
                        bOk = !bMatch;
                        break;
                    case SC_BEGINS_WITH:
                        bOk = ( bMatch && (nStart == 0) );
                        break;
                    case SC_DOES_NOT_BEGIN_WITH:
                        bOk = !( bMatch && (nStart == 0) );
                        break;
                    case SC_ENDS_WITH:
                        bOk = ( bMatch && (nEnd == rValue.getLength()) );
                        break;
                    case SC_DOES_NOT_END_WITH:
                        bOk = !( bMatch && (nEnd == rValue.getLength()) );
                        break;
                    default:
                        {
                            // added to avoid warnings
                        }
                }
            }
            else
                bTestEqual = bMatch;
        }
        if ( !bRealWildOrRegExp )
        {
            // Simple string matching i.e. no regexp match.
            if (isTextMatchOp(rEntry))
            {
                if (rItem.meType != ScQueryEntry::ByString && rItem.maString.isEmpty())
                {
                    // #i18374# When used from functions (match, countif, sumif, vlookup, hlookup, lookup),
                    // the query value is assigned directly, and the string is empty. In that case,
                    // don't find any string (isEqual would find empty string results in formula cells).
                    bOk = false;
                    if ( rEntry.eOp == SC_NOT_EQUAL )
                        bOk = !bOk;
                }
                else if ( bMatchWholeCell )
                {
                    if (pValueSource1)
                    {
                        // Fast string equality check by comparing string identifiers.
                        if (mrParam.bCaseSens)
                        {
                            bOk = pValueSource1->getData() == rItem.maString.getData();
                        }
                        else
                        {
                            bOk = pValueSource1->getDataIgnoreCase() == rItem.maString.getDataIgnoreCase();
                        }
                    }
                    else // if (pValueSource2)
                    {
                        if (mrParam.bCaseSens)
                        {
                            bOk = (*pValueSource2 == rItem.maString.getString());
                        }
                        else
                        {
                            // fallback
                            const svl::SharedString rSource2(mrStrPool.intern(*pValueSource2));
                            // Fast string equality check by comparing string identifiers.
                            bOk = rSource2.getDataIgnoreCase() == rItem.maString.getDataIgnoreCase();
                        }
                    }

                    if ( rEntry.eOp == SC_NOT_EQUAL )
                        bOk = !bOk;
                }
                else
                {
                    // Where do we find a match (if at all)
                    sal_Int32 nStrPos;

                    if (!mbCaseSensitive)
                    { // Common case for vlookup etc.
                        const svl::SharedString rSource(pValueSource1? *pValueSource1 : mrStrPool.intern(*pValueSource2));

                        const rtl_uString *pQuer = rItem.maString.getDataIgnoreCase();
                        const rtl_uString *pCellStr = rSource.getDataIgnoreCase();

                        assert(pQuer != nullptr);
                        assert(pCellStr != nullptr);

                        const sal_Int32 nIndex = (rEntry.eOp == SC_ENDS_WITH ||
                                            rEntry.eOp == SC_DOES_NOT_END_WITH) ?
                            (pCellStr->length - pQuer->length) : 0;

                        if (nIndex < 0)
                            nStrPos = -1;
                        else
                        { // OUString::indexOf
                            nStrPos = rtl_ustr_indexOfStr_WithLength(
                                pCellStr->buffer + nIndex, pCellStr->length - nIndex,
                                pQuer->buffer, pQuer->length );

                            if (nStrPos >= 0)
                                nStrPos += nIndex;
                        }
                    }
                    else
                    {
                        const OUString & rValue = pValueSource1 ? pValueSource1->getString() : *pValueSource2;
                        const OUString aQueryStr = rItem.maString.getString();
                        const LanguageType nLang = ScGlobal::xSysLocale->GetLanguageTag().getLanguageType();
                        setupTransliteratorIfNeeded();
                        const OUString aCell( mpTransliteration->transliterate(
                                            rValue, nLang, 0, rValue.getLength(),
                                            nullptr ) );

                        const OUString aQuer( mpTransliteration->transliterate(
                                            aQueryStr, nLang, 0, aQueryStr.getLength(),
                                            nullptr ) );

                        const sal_Int32 nIndex = (rEntry.eOp == SC_ENDS_WITH || rEntry.eOp == SC_DOES_NOT_END_WITH) ?
                            (aCell.getLength() - aQuer.getLength()) : 0;
                        nStrPos = ((nIndex < 0) ? -1 : aCell.indexOf( aQuer, nIndex ));
                    }
                    switch (rEntry.eOp)
                    {
                    case SC_EQUAL:
                    case SC_CONTAINS:
                        bOk = ( nStrPos != -1 );
                        break;
                    case SC_NOT_EQUAL:
                    case SC_DOES_NOT_CONTAIN:
                        bOk = ( nStrPos == -1 );
                        break;
                    case SC_BEGINS_WITH:
                        bOk = ( nStrPos == 0 );
                        break;
                    case SC_DOES_NOT_BEGIN_WITH:
                        bOk = ( nStrPos != 0 );
                        break;
                    case SC_ENDS_WITH:
                        bOk = ( nStrPos >= 0 );
                        break;
                    case SC_DOES_NOT_END_WITH:
                        bOk = ( nStrPos < 0 );
                        break;
                    default:
                        {
                            // added to avoid warnings
                        }
                    }
                }
            }
            else
            {   // use collator here because data was probably sorted
                const OUString & rValue = pValueSource1 ? pValueSource1->getString() : *pValueSource2;
                setupCollatorIfNeeded();
                sal_Int32 nCompare = mpCollator->compareString(
                    rValue, rItem.maString.getString());
                switch (rEntry.eOp)
                {
                    case SC_LESS :
                        bOk = (nCompare < 0);
                        break;
                    case SC_GREATER :
                        bOk = (nCompare > 0);
                        break;
                    case SC_LESS_EQUAL :
                        bOk = (nCompare <= 0);
                        if ( bOk && mpTestEqualCondition && !bTestEqual )
                            bTestEqual = (nCompare == 0);
                        break;
                    case SC_GREATER_EQUAL :
                        bOk = (nCompare >= 0);
                        if ( bOk && mpTestEqualCondition && !bTestEqual )
                            bTestEqual = (nCompare == 0);
                        break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
        }

        return std::pair<bool,bool>(bOk, bTestEqual);
    }

    // To be called only if both isQueryByValue() and isQueryByString()
    // returned false and range lookup is wanted! In range lookup comparison
    // numbers are less than strings. Nothing else is compared.
    std::pair<bool,bool> compareByRangeLookup(
        const ScRefCellValue& rCell, SCCOL nCol, SCROW nRow,
        const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem)
    {
        bool bTestEqual = false;

        if (rItem.meType == ScQueryEntry::ByString && rEntry.eOp != SC_LESS && rEntry.eOp != SC_LESS_EQUAL)
            return std::pair<bool,bool>(false, bTestEqual);

        if (rItem.meType != ScQueryEntry::ByString && rEntry.eOp != SC_GREATER && rEntry.eOp != SC_GREATER_EQUAL)
            return std::pair<bool,bool>(false, bTestEqual);

        if (!rCell.isEmpty())
        {
            if (rItem.meType == ScQueryEntry::ByString)
            {
                if (rCell.meType == CELLTYPE_FORMULA && rCell.mpFormula->GetErrCode() != FormulaError::NONE)
                    // Error values are compared as string.
                    return std::pair<bool,bool>(false, bTestEqual);

                return std::pair<bool,bool>(rCell.hasNumeric(), bTestEqual);
            }

            return std::pair<bool,bool>(!rCell.hasNumeric(), bTestEqual);
        }

        if (rItem.meType == ScQueryEntry::ByString)
            return std::pair<bool,bool>(mrTab.HasValueData(nCol, nRow), bTestEqual);

        return std::pair<bool,bool>(!mrTab.HasValueData(nCol, nRow), bTestEqual);
    }
};

}

bool ScTable::ValidQuery(
    SCROW nRow, const ScQueryParam& rParam, const ScRefCellValue* pCell, bool* pbTestEqualCondition,
    const ScInterpreterContext* pContext, sc::TableColumnBlockPositionSet* pBlockPos)
{
    if (!rParam.GetEntry(0).bDoQuery)
        return true;

    //---------------------------------------------------------------

    const SCSIZE nFixedBools = 32;
    bool aBool[nFixedBools];
    bool aTest[nFixedBools];
    SCSIZE nEntryCount = rParam.GetEntryCount();
    bool* pPasst = ( nEntryCount <= nFixedBools ? &aBool[0] : new bool[nEntryCount] );
    bool* pTest = ( nEntryCount <= nFixedBools ? &aTest[0] : new bool[nEntryCount] );

    tools::Long    nPos = -1;
    QueryEvaluator aEval(rDocument, *this, rParam, pbTestEqualCondition != nullptr);
    ScQueryParam::const_iterator it, itBeg = rParam.begin(), itEnd = rParam.end();
    for (it = itBeg; it != itEnd && (*it)->bDoQuery; ++it)
    {
        const ScQueryEntry& rEntry = **it;
        SCCOL nCol = static_cast<SCCOL>(rEntry.nField);

        // We can only handle one single direct query passed as a known pCell,
        // subsequent queries have to obtain the cell.
        ScRefCellValue aCell;
        if(pCell && it == itBeg)
            aCell = *pCell;
        else if( pBlockPos )
        {   // hinted mdds access
            ScColumn* column = FetchColumn(nCol);
            aCell = column->GetCellValue(*pBlockPos->getBlockPosition( nCol ), nRow);
        }
        else
            aCell = GetCellValue(nCol, nRow);

        std::pair<bool,bool> aRes(false, false);

        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (rItems.size() == 1 && rItems.front().meType == ScQueryEntry::ByEmpty)
        {
            bool hasData;
            if( pBlockPos )
            {
                ScColumn* column = FetchColumn(rEntry.nField);
                hasData = column->HasDataAt(*pBlockPos->getBlockPosition(rEntry.nField), nRow);
            }
            else
                hasData = aCol[rEntry.nField].HasDataAt(nRow);
            if (rEntry.IsQueryByEmpty())
                aRes.first = !hasData;
            else
            {
                assert(rEntry.IsQueryByNonEmpty());
                aRes.first = hasData;
            }
        }
        else
        {
            for (const auto& rItem : rItems)
            {
                if (aEval.isQueryByValue(rEntry, rItem, nCol, nRow, aCell))
                {
                    std::pair<bool,bool> aThisRes =
                        aEval.compareByValue(aCell, nCol, nRow, rEntry, rItem, pContext);
                    aRes.first |= aThisRes.first;
                    aRes.second |= aThisRes.second;
                }
                else if (aEval.isQueryByString(rEntry, rItem, nCol, nRow, aCell))
                {
                    std::pair<bool,bool> aThisRes =
                        aEval.compareByString(aCell, nRow, rEntry, rItem, pContext);
                    aRes.first |= aThisRes.first;
                    aRes.second |= aThisRes.second;
                }
                else if (rParam.mbRangeLookup)
                {
                    std::pair<bool,bool> aThisRes =
                        aEval.compareByRangeLookup(aCell, nCol, nRow, rEntry, rItem);
                    aRes.first |= aThisRes.first;
                    aRes.second |= aThisRes.second;
                }

                if (aRes.first && aRes.second)
                    break;
            }
        }

        if (nPos == -1)
        {
            nPos++;
            pPasst[nPos] = aRes.first;
            pTest[nPos] = aRes.second;
        }
        else
        {
            if (rEntry.eConnect == SC_AND)
            {
                pPasst[nPos] = pPasst[nPos] && aRes.first;
                pTest[nPos] = pTest[nPos] && aRes.second;
            }
            else
            {
                nPos++;
                pPasst[nPos] = aRes.first;
                pTest[nPos] = aRes.second;
            }
        }
    }

    for ( tools::Long j=1; j <= nPos; j++ )
    {
        pPasst[0] = pPasst[0] || pPasst[j];
        pTest[0] = pTest[0] || pTest[j];
    }

    bool bRet = pPasst[0];
    if ( pPasst != &aBool[0] )
        delete [] pPasst;
    if ( pbTestEqualCondition )
        *pbTestEqualCondition = pTest[0];
    if ( pTest != &aTest[0] )
        delete [] pTest;

    return bRet;
}

void ScTable::TopTenQuery( ScQueryParam& rParam )
{
    bool bSortCollatorInitialized = false;
    SCSIZE nEntryCount = rParam.GetEntryCount();
    SCROW nRow1 = (rParam.bHasHeader ? rParam.nRow1 + 1 : rParam.nRow1);
    SCSIZE nCount = static_cast<SCSIZE>(rParam.nRow2 - nRow1 + 1);
    for ( SCSIZE i=0; (i<nEntryCount) && (rParam.GetEntry(i).bDoQuery); i++ )
    {
        ScQueryEntry& rEntry = rParam.GetEntry(i);
        ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

        switch ( rEntry.eOp )
        {
            case SC_TOPVAL:
            case SC_BOTVAL:
            case SC_TOPPERC:
            case SC_BOTPERC:
            {
                ScSortParam aLocalSortParam( rParam, static_cast<SCCOL>(rEntry.nField) );
                aSortParam = aLocalSortParam;       // used in CreateSortInfoArray, Compare
                if ( !bSortCollatorInitialized )
                {
                    bSortCollatorInitialized = true;
                    InitSortCollator( aLocalSortParam );
                }
                std::unique_ptr<ScSortInfoArray> pArray(CreateSortInfoArray(aSortParam, nRow1, rParam.nRow2, bGlobalKeepQuery, false));
                DecoladeRow( pArray.get(), nRow1, rParam.nRow2 );
                QuickSort( pArray.get(), nRow1, rParam.nRow2 );
                std::unique_ptr<ScSortInfo[]> const & ppInfo = pArray->GetFirstArray();
                SCSIZE nValidCount = nCount;
                // Don't count note or blank cells, they are sorted to the end
                while (nValidCount > 0 && ppInfo[nValidCount-1].maCell.isEmpty())
                    nValidCount--;
                // Don't count Strings, they are between Value and blank
                while (nValidCount > 0 && ppInfo[nValidCount-1].maCell.hasString())
                    nValidCount--;
                if ( nValidCount > 0 )
                {
                    if ( rItem.meType == ScQueryEntry::ByString )
                    {   // by string ain't going to work
                        rItem.meType = ScQueryEntry::ByValue;
                        rItem.mfVal = 10;   // 10 and 10% respectively
                    }
                    SCSIZE nVal = (rItem.mfVal >= 1 ? static_cast<SCSIZE>(rItem.mfVal) : 1);
                    SCSIZE nOffset = 0;
                    switch ( rEntry.eOp )
                    {
                        case SC_TOPVAL:
                        {
                            rEntry.eOp = SC_GREATER_EQUAL;
                            if ( nVal > nValidCount )
                                nVal = nValidCount;
                            nOffset = nValidCount - nVal;   // 1 <= nVal <= nValidCount
                        }
                        break;
                        case SC_BOTVAL:
                        {
                            rEntry.eOp = SC_LESS_EQUAL;
                            if ( nVal > nValidCount )
                                nVal = nValidCount;
                            nOffset = nVal - 1;     // 1 <= nVal <= nValidCount
                        }
                        break;
                        case SC_TOPPERC:
                        {
                            rEntry.eOp = SC_GREATER_EQUAL;
                            if ( nVal > 100 )
                                nVal = 100;
                            nOffset = nValidCount - (nValidCount * nVal / 100);
                            if ( nOffset >= nValidCount )
                                nOffset = nValidCount - 1;
                        }
                        break;
                        case SC_BOTPERC:
                        {
                            rEntry.eOp = SC_LESS_EQUAL;
                            if ( nVal > 100 )
                                nVal = 100;
                            nOffset = (nValidCount * nVal / 100);
                            if ( nOffset >= nValidCount )
                                nOffset = nValidCount - 1;
                        }
                        break;
                        default:
                        {
                            // added to avoid warnings
                        }
                    }
                    ScRefCellValue aCell = ppInfo[nOffset].maCell;
                    if (aCell.hasNumeric())
                        rItem.mfVal = aCell.getValue();
                    else
                    {
                        OSL_FAIL( "TopTenQuery: pCell no ValueData" );
                        rEntry.eOp = SC_GREATER_EQUAL;
                        rItem.mfVal = 0;
                    }
                }
                else
                {
                    rEntry.eOp = SC_GREATER_EQUAL;
                    rItem.meType = ScQueryEntry::ByValue;
                    rItem.mfVal = 0;
                }
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
    if ( bSortCollatorInitialized )
        DestroySortCollator();
}

namespace {

bool CanOptimizeQueryStringToNumber( SvNumberFormatter* pFormatter, sal_uInt32 nFormatIndex )
{
    // tdf#105629: ScQueryEntry::ByValue queries are faster than ScQueryEntry::ByString.
    // The problem with this optimization is that the autofilter dialog apparently converts
    // the value to text and then converts that back to a number for filtering.
    // If that leads to any change of value (such as when time is rounded to seconds),
    // even matching values will be filtered out. Therefore query by value only for formats
    // where no such change should occur.
    if(const SvNumberformat* pEntry = pFormatter->GetEntry(nFormatIndex))
    {
        switch(pEntry->GetType())
        {
        case SvNumFormatType::NUMBER:
        case SvNumFormatType::FRACTION:
        case SvNumFormatType::SCIENTIFIC:
            return true;
        default:
            break;
        }
    }
    return false;
}

class PrepareQueryItem
{
    const ScDocument& mrDoc;
public:
    explicit PrepareQueryItem(const ScDocument& rDoc) : mrDoc(rDoc) {}

    void operator() (ScQueryEntry::Item& rItem)
    {
        if (rItem.meType != ScQueryEntry::ByString && rItem.meType != ScQueryEntry::ByDate)
            return;

        sal_uInt32 nIndex = 0;
        bool bNumber = mrDoc.GetFormatTable()->
            IsNumberFormat(rItem.maString.getString(), nIndex, rItem.mfVal);

        // Advanced Filter creates only ByString queries that need to be
        // converted to ByValue if appropriate. rItem.mfVal now holds the value
        // if bNumber==true.

        if (rItem.meType == ScQueryEntry::ByString)
        {
            if (bNumber && CanOptimizeQueryStringToNumber( mrDoc.GetFormatTable(), nIndex ))
                rItem.meType = ScQueryEntry::ByValue;
            return;
        }

        // Double-check if the query by date is really appropriate.

        if (bNumber && ((nIndex % SV_COUNTRY_LANGUAGE_OFFSET) != 0))
        {
            const SvNumberformat* pEntry = mrDoc.GetFormatTable()->GetEntry(nIndex);
            if (pEntry)
            {
                SvNumFormatType nNumFmtType = pEntry->GetType();
                if (!(nNumFmtType & SvNumFormatType::DATE) || (nNumFmtType & SvNumFormatType::TIME))
                    rItem.meType = ScQueryEntry::ByValue;    // not a date only
            }
            else
                rItem.meType = ScQueryEntry::ByValue;    // what the ... not a date
        }
        else
            rItem.meType = ScQueryEntry::ByValue;    // not a date
    }
};

void lcl_PrepareQuery( const ScDocument* pDoc, ScTable* pTab, ScQueryParam& rParam )
{
    bool bTopTen = false;
    SCSIZE nEntryCount = rParam.GetEntryCount();

    for ( SCSIZE i = 0; i < nEntryCount; ++i )
    {
        ScQueryEntry& rEntry = rParam.GetEntry(i);
        if (!rEntry.bDoQuery)
            continue;

        ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        std::for_each(rItems.begin(), rItems.end(), PrepareQueryItem(*pDoc));

        if ( !bTopTen )
        {
            switch ( rEntry.eOp )
            {
                case SC_TOPVAL:
                case SC_BOTVAL:
                case SC_TOPPERC:
                case SC_BOTPERC:
                {
                    bTopTen = true;
                }
                break;
                default:
                {
                }
            }
        }
    }

    if ( bTopTen )
    {
        pTab->TopTenQuery( rParam );
    }
}

}

SCSIZE ScTable::Query(const ScQueryParam& rParamOrg, bool bKeepSub)
{
    ScQueryParam    aParam( rParamOrg );
    typedef std::unordered_set<OUString> StrSetType;
    StrSetType aStrSet;

    bool    bStarted = false;
    bool    bOldResult = true;
    SCROW   nOldStart = 0;
    SCROW   nOldEnd = 0;

    SCSIZE nCount   = 0;
    SCROW nOutRow   = 0;
    SCROW nHeader   = aParam.bHasHeader ? 1 : 0;

    lcl_PrepareQuery(&rDocument, this, aParam);

    if (!aParam.bInplace)
    {
        nOutRow = aParam.nDestRow + nHeader;
        if (nHeader > 0)
            CopyData( aParam.nCol1, aParam.nRow1, aParam.nCol2, aParam.nRow1,
                            aParam.nDestCol, aParam.nDestRow, aParam.nDestTab );
    }

    sc::TableColumnBlockPositionSet blockPos( GetDoc(), nTab ); // cache mdds access

    SCROW nRealRow2 = aParam.nRow2;
    for (SCROW j = aParam.nRow1 + nHeader; j <= nRealRow2; ++j)
    {
        bool bResult;                                   // Filter result
        bool bValid = ValidQuery(j, aParam, nullptr, nullptr, nullptr, &blockPos);
        if (!bValid && bKeepSub)                        // Keep subtotals
        {
            for (SCCOL nCol=aParam.nCol1; nCol<=aParam.nCol2 && !bValid; nCol++)
            {
                ScRefCellValue aCell = GetCellValue(nCol, j);
                if (aCell.meType != CELLTYPE_FORMULA)
                    continue;

                if (!aCell.mpFormula->IsSubTotal())
                    continue;

                if (RefVisible(aCell.mpFormula))
                    bValid = true;
            }
        }
        if (bValid)
        {
            if (aParam.bDuplicate)
                bResult = true;
            else
            {
                OUStringBuffer aStr;
                for (SCCOL k=aParam.nCol1; k <= aParam.nCol2; k++)
                {
                    OUString aCellStr;
                    GetString(k, j, aCellStr);
                    aStr.append(aCellStr + u"\x0001");
                }

                bResult = aStrSet.insert(aStr.makeStringAndClear()).second; // unique if inserted.
            }
        }
        else
            bResult = false;

        if (aParam.bInplace)
        {
            if (bResult == bOldResult && bStarted)
                nOldEnd = j;
            else
            {
                if (bStarted)
                    DBShowRows(nOldStart,nOldEnd, bOldResult);
                nOldStart = nOldEnd = j;
                bOldResult = bResult;
            }
            bStarted = true;
        }
        else
        {
            if (bResult)
            {
                CopyData( aParam.nCol1,j, aParam.nCol2,j, aParam.nDestCol,nOutRow,aParam.nDestTab );
                if( nTab == aParam.nDestTab ) // copy to self, changes may invalidate caching position hints
                    blockPos.invalidate();
                ++nOutRow;
            }
        }
        if (bResult)
            ++nCount;
    }

    if (aParam.bInplace && bStarted)
        DBShowRows(nOldStart,nOldEnd, bOldResult);

    if (aParam.bInplace)
        SetDrawPageSize();

    return nCount;
}

bool ScTable::CreateExcelQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam)
{
    bool    bValid = true;
    std::unique_ptr<SCCOL[]> pFields(new SCCOL[nCol2-nCol1+1]);
    OUString  aCellStr;
    SCCOL   nCol = nCol1;
    OSL_ENSURE( rQueryParam.nTab != SCTAB_MAX, "rQueryParam.nTab no value, not bad but no good" );
    SCTAB   nDBTab = (rQueryParam.nTab == SCTAB_MAX ? nTab : rQueryParam.nTab);
    SCROW   nDBRow1 = rQueryParam.nRow1;
    SCCOL   nDBCol2 = rQueryParam.nCol2;
    // First row must be column headers
    while (bValid && (nCol <= nCol2))
    {
        OUString aQueryStr;
        GetUpperCellString(nCol, nRow1, aQueryStr);
        bool bFound = false;
        SCCOL i = rQueryParam.nCol1;
        while (!bFound && (i <= nDBCol2))
        {
            if ( nTab == nDBTab )
                GetUpperCellString(i, nDBRow1, aCellStr);
            else
                rDocument.GetUpperCellString(i, nDBRow1, nDBTab, aCellStr);
            bFound = (aCellStr == aQueryStr);
            if (!bFound) i++;
        }
        if (bFound)
            pFields[nCol - nCol1] = i;
        else
            bValid = false;
        nCol++;
    }
    if (bValid)
    {
        sal_uLong nVisible = 0;
        for ( nCol=nCol1; nCol<=nCol2; nCol++ )
            nVisible += aCol[nCol].VisibleCount( nRow1+1, nRow2 );

        if ( nVisible > SCSIZE_MAX / sizeof(void*) )
        {
            OSL_FAIL("too many filter criteria");
            nVisible = 0;
        }

        SCSIZE nNewEntries = nVisible;
        rQueryParam.Resize( nNewEntries );

        SCSIZE nIndex = 0;
        SCROW nRow = nRow1 + 1;
        svl::SharedStringPool& rPool = rDocument.GetSharedStringPool();
        while (nRow <= nRow2)
        {
            nCol = nCol1;
            while (nCol <= nCol2)
            {
                GetInputString( nCol, nRow, aCellStr );
                if (!aCellStr.isEmpty())
                {
                    if (nIndex < nNewEntries)
                    {
                        rQueryParam.GetEntry(nIndex).nField = pFields[nCol - nCol1];
                        rQueryParam.FillInExcelSyntax(rPool, aCellStr, nIndex, nullptr);
                        nIndex++;
                        if (nIndex < nNewEntries)
                            rQueryParam.GetEntry(nIndex).eConnect = SC_AND;
                    }
                    else
                        bValid = false;
                }
                nCol++;
            }
            nRow++;
            if (nIndex < nNewEntries)
                rQueryParam.GetEntry(nIndex).eConnect = SC_OR;
        }
    }
    return bValid;
}

bool ScTable::CreateStarQuery(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam)
{
    // A valid StarQuery must be at least 4 columns wide. To be precise it
    // should be exactly 4 columns ...
    // Additionally, if this wasn't checked, a formula pointing to a valid 1-3
    // column Excel style query range immediately left to itself would result
    // in a circular reference when the field name or operator or value (first
    // to third query range column) is obtained (#i58354#). Furthermore, if the
    // range wasn't sufficiently specified data changes wouldn't flag formula
    // cells for recalculation.
    if (nCol2 - nCol1 < 3)
        return false;

    bool bValid;
    OUString aCellStr;
    SCSIZE nIndex = 0;
    SCROW nRow = nRow1;
    OSL_ENSURE( rQueryParam.nTab != SCTAB_MAX, "rQueryParam.nTab no value, not bad but no good" );
    SCTAB   nDBTab = (rQueryParam.nTab == SCTAB_MAX ? nTab : rQueryParam.nTab);
    SCROW   nDBRow1 = rQueryParam.nRow1;
    SCCOL   nDBCol2 = rQueryParam.nCol2;

    SCSIZE nNewEntries = static_cast<SCSIZE>(nRow2-nRow1+1);
    rQueryParam.Resize( nNewEntries );
    svl::SharedStringPool& rPool = rDocument.GetSharedStringPool();

    do
    {
        ScQueryEntry& rEntry = rQueryParam.GetEntry(nIndex);

        bValid = false;
        // First column AND/OR
        if (nIndex > 0)
        {
            GetUpperCellString(nCol1, nRow, aCellStr);
            if ( aCellStr == ScResId(STR_TABLE_AND) )
            {
                rEntry.eConnect = SC_AND;
                bValid = true;
            }
            else if ( aCellStr == ScResId(STR_TABLE_OR) )
            {
                rEntry.eConnect = SC_OR;
                bValid = true;
            }
        }
        // Second column field name
        if ((nIndex < 1) || bValid)
        {
            bool bFound = false;
            GetUpperCellString(nCol1 + 1, nRow, aCellStr);
            for (SCCOL i=rQueryParam.nCol1; (i <= nDBCol2) && (!bFound); i++)
            {
                OUString aFieldStr;
                if ( nTab == nDBTab )
                    GetUpperCellString(i, nDBRow1, aFieldStr);
                else
                    rDocument.GetUpperCellString(i, nDBRow1, nDBTab, aFieldStr);
                bFound = (aCellStr == aFieldStr);
                if (bFound)
                {
                    rEntry.nField = i;
                    bValid = true;
                }
                else
                    bValid = false;
            }
        }
        // Third column operator =<>...
        if (bValid)
        {
            GetUpperCellString(nCol1 + 2, nRow, aCellStr);
            if (aCellStr.startsWith("<"))
            {
                if (aCellStr[1] == '>')
                    rEntry.eOp = SC_NOT_EQUAL;
                else if (aCellStr[1] == '=')
                    rEntry.eOp = SC_LESS_EQUAL;
                else
                    rEntry.eOp = SC_LESS;
            }
            else if (aCellStr.startsWith(">"))
            {
                if (aCellStr[1] == '=')
                    rEntry.eOp = SC_GREATER_EQUAL;
                else
                    rEntry.eOp = SC_GREATER;
            }
            else if (aCellStr.startsWith("="))
                rEntry.eOp = SC_EQUAL;

        }
        // Fourth column values
        if (bValid)
        {
            OUString aStr;
            GetString(nCol1 + 3, nRow, aStr);
            rEntry.GetQueryItem().maString = rPool.intern(aStr);
            rEntry.bDoQuery = true;
        }
        nIndex++;
        nRow++;
    }
    while (bValid && (nRow <= nRow2) /* && (nIndex < MAXQUERY) */ );
    return bValid;
}

bool ScTable::CreateQueryParam(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScQueryParam& rQueryParam)
{
    SCSIZE i, nCount;
    PutInOrder(nCol1, nCol2);
    PutInOrder(nRow1, nRow2);

    nCount = rQueryParam.GetEntryCount();
    for (i=0; i < nCount; i++)
        rQueryParam.GetEntry(i).Clear();

    // Standard query table
    bool bValid = CreateStarQuery(nCol1, nRow1, nCol2, nRow2, rQueryParam);
    // Excel Query table
    if (!bValid)
        bValid = CreateExcelQuery(nCol1, nRow1, nCol2, nRow2, rQueryParam);

    SvNumberFormatter* pFormatter = rDocument.GetFormatTable();
    nCount = rQueryParam.GetEntryCount();
    if (bValid)
    {
        //  query type must be set
        for (i=0; i < nCount; i++)
        {
            ScQueryEntry::Item& rItem = rQueryParam.GetEntry(i).GetQueryItem();
            sal_uInt32 nIndex = 0;
            bool bNumber = pFormatter->IsNumberFormat(
                rItem.maString.getString(), nIndex, rItem.mfVal);
            rItem.meType = bNumber && CanOptimizeQueryStringToNumber( pFormatter, nIndex )
                ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
        }
    }
    else
    {
        for (i=0; i < nCount; i++)
            rQueryParam.GetEntry(i).Clear();
    }
    return bValid;
}

bool ScTable::HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow) const
{
    if (nStartRow == nEndRow)
        // Assume only data.
        /* XXX NOTE: previous behavior still checked this one row and could
         * evaluate it has header row, but that doesn't make much sense. */
        return false;

    if (nStartCol == nEndCol)
    {
        CellType eFirstCellType = GetCellType(nStartCol, nStartRow);
        CellType eSecondCellType = GetCellType(nStartCol, nStartRow+1);
        return ((eFirstCellType == CELLTYPE_STRING || eFirstCellType == CELLTYPE_EDIT) &&
                (eSecondCellType != CELLTYPE_STRING && eSecondCellType != CELLTYPE_EDIT));
    }

    for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        CellType eType = GetCellType( nCol, nStartRow );
        // Any non-text cell in first row => not headers.
        if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
            return false;
    }

    // First row all text cells, any non-text cell in second row => headers.
    SCROW nTestRow = nStartRow + 1;
    for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        CellType eType = GetCellType( nCol, nTestRow );
        if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
            return true;
    }

    // Also second row all text cells => first row not headers.
    return false;
}

bool ScTable::HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow ) const
{
    if (nStartCol == nEndCol)
        // Assume only data.
        /* XXX NOTE: previous behavior still checked this one column and could
         * evaluate it has header column, but that doesn't make much sense. */
        return false;

    if (nStartRow == nEndRow)
    {
        CellType eFirstCellType = GetCellType(nStartCol, nStartRow);
        CellType eSecondCellType = GetCellType(nStartCol+1, nStartRow);
        return ((eFirstCellType == CELLTYPE_STRING || eFirstCellType == CELLTYPE_EDIT) &&
                (eSecondCellType != CELLTYPE_STRING && eSecondCellType != CELLTYPE_EDIT));
    }

    for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
    {
        CellType eType = GetCellType( nStartCol, nRow );
        // Any non-text cell in first column => not headers.
        if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
            return false;
    }

    // First column all text cells, any non-text cell in second column => headers.
    SCCOL nTestCol = nStartCol + 1;
    for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
    {
        CellType eType = GetCellType( nRow, nTestCol );
        if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
            return true;
    }

    // Also second column all text cells => first column not headers.
    return false;
}

void ScTable::GetFilterEntries( SCCOL nCol, SCROW nRow1, SCROW nRow2, ScFilterEntries& rFilterEntries )
{
    sc::ColumnBlockConstPosition aBlockPos;
    aCol[nCol].InitBlockPosition(aBlockPos);
    aCol[nCol].GetFilterEntries(aBlockPos, nRow1, nRow2, rFilterEntries);
}

void ScTable::GetFilteredFilterEntries(
    SCCOL nCol, SCROW nRow1, SCROW nRow2, const ScQueryParam& rParam, ScFilterEntries& rFilterEntries )
{
    sc::ColumnBlockConstPosition aBlockPos;
    aCol[nCol].InitBlockPosition(aBlockPos);

    // remove the entry for this column from the query parameter
    ScQueryParam aParam( rParam );
    aParam.RemoveEntryByField(nCol);

    lcl_PrepareQuery(&rDocument, this, aParam);
    for ( SCROW j = nRow1; j <= nRow2; ++j )
    {
        if (ValidQuery(j, aParam))
        {
            aCol[nCol].GetFilterEntries(aBlockPos, j, j, rFilterEntries);
        }
    }
}

bool ScTable::GetDataEntries(SCCOL nCol, SCROW nRow, std::set<ScTypedStrData>& rStrings)
{
    return aCol[nCol].GetDataEntries( nRow, rStrings);
}

sal_uLong ScTable::GetCellCount() const
{
    sal_uLong nCellCount = 0;

    for ( SCCOL nCol=0; nCol < aCol.size(); nCol++ )
        nCellCount += aCol[nCol].GetCellCount();

    return nCellCount;
}

sal_uLong ScTable::GetWeightedCount() const
{
    sal_uLong nCellCount = 0;

    for ( SCCOL nCol=0; nCol < aCol.size(); nCol++ )
        nCellCount += aCol[nCol].GetWeightedCount();

    return nCellCount;
}

sal_uLong ScTable::GetWeightedCount(SCROW nStartRow, SCROW nEndRow) const
{
    sal_uLong nCellCount = 0;

    for ( SCCOL nCol=0; nCol < aCol.size(); nCol++ )
        nCellCount += aCol[nCol].GetWeightedCount(nStartRow, nEndRow);

    return nCellCount;
}

sal_uLong ScTable::GetCodeCount() const
{
    sal_uLong nCodeCount = 0;

    for ( SCCOL nCol=0; nCol < aCol.size(); nCol++ )
        if ( aCol[nCol].GetCellCount() )
            nCodeCount += aCol[nCol].GetCodeCount();

    return nCodeCount;
}

sal_Int32 ScTable::GetMaxStringLen( SCCOL nCol, SCROW nRowStart,
        SCROW nRowEnd, rtl_TextEncoding eCharSet ) const
{
    if ( IsColValid( nCol ) )
        return aCol[nCol].GetMaxStringLen( nRowStart, nRowEnd, eCharSet );
    else
        return 0;
}

sal_Int32 ScTable::GetMaxNumberStringLen(
    sal_uInt16& nPrecision, SCCOL nCol, SCROW nRowStart, SCROW nRowEnd ) const
{
    if ( IsColValid( nCol ) )
        return aCol[nCol].GetMaxNumberStringLen( nPrecision, nRowStart, nRowEnd );
    else
        return 0;
}

void ScTable::UpdateSelectionFunction( ScFunctionData& rData, const ScMarkData& rMark )
{
    ScRangeList aRanges = rMark.GetMarkedRangesForTab( nTab );
    ScRange aMarkArea( ScAddress::UNINITIALIZED );
    if (rMark.IsMultiMarked())
        rMark.GetMultiMarkArea( aMarkArea );
    else if (rMark.IsMarked())
        rMark.GetMarkArea( aMarkArea );
    else
    {
        assert(!"ScTable::UpdateSelectionFunction - called without anything marked");
        aMarkArea.aStart.SetCol(0);
        aMarkArea.aEnd.SetCol(rDocument.MaxCol());
    }
    const SCCOL nStartCol = aMarkArea.aStart.Col();
    const SCCOL nEndCol = ClampToAllocatedColumns(aMarkArea.aEnd.Col());
    for (SCCOL nCol = nStartCol; nCol <= nEndCol && !rData.getError(); ++nCol)
    {
        if (mpColFlags && ColHidden(nCol))
            continue;

        aCol[nCol].UpdateSelectionFunction(aRanges, rData, *mpHiddenRows);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
