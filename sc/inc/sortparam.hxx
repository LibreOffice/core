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

#pragma once

#define DEFSORT 3

#include <vector>

#include "address.hxx"
#include <editeng/colritem.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include "scdllapi.h"
#include "celltextattr.hxx"
#include "cellvalue.hxx"
#include "patattr.hxx"

struct ScSubTotalParam;
struct ScQueryParam;
class SdrObject;
class ScPostIt;

enum class ScColorSortMode {
    None,
    TextColor,
    BackgroundColor
};

struct ScSortKeyState
{
    SCCOLROW nField;
    bool     bDoSort;
    bool     bAscending;
    ScColorSortMode aColorSortMode;
    Color    aColorSortColor;
};

/** Struct to hold non-data extended area, used with
    ScDocument::ShrinkToUsedDataArea().
*/
struct ScDataAreaExtras
{
    /// If TRUE, consider the presence of cell notes besides data.
    bool    mbCellNotes = false;
    /// If TRUE, consider the presence of draw objects anchored to the cell.
    bool    mbCellDrawObjects = false;
    /// If TRUE, consider the presence of cell formats.
    bool    mbCellFormats = false;
    SCCOL   mnStartCol = SCCOL_MAX;
    SCROW   mnStartRow = SCROW_MAX;
    SCCOL   mnEndCol = -1;
    SCROW   mnEndRow = -1;

    bool anyExtrasWanted() const { return mbCellNotes || mbCellDrawObjects || mbCellFormats; }
    void resetArea() { mnStartCol = SCCOL_MAX; mnStartRow = SCROW_MAX; mnEndCol = -1; mnEndRow = -1; }

    bool operator==( const ScDataAreaExtras& rOther ) const
    {
        // Ignore area range, this is used in ScSortParam::operator==().
        return mbCellNotes       == rOther.mbCellNotes
            && mbCellDrawObjects == rOther.mbCellDrawObjects
            && mbCellFormats     == rOther.mbCellFormats;
    }

    enum class Clip
    {
        None,
        Col,
        Row
    };

    /// Obtain the overall range if area extras are larger.
    void GetOverallRange( SCCOL& nCol1, SCROW& nRow1, SCCOL& nCol2, SCROW& nRow2, Clip eClip = Clip::None ) const
    {
        if (eClip != Clip::Col)
        {
            if (nCol1 > mnStartCol)
                nCol1 = mnStartCol;
            if (nCol2 < mnEndCol)
                nCol2 = mnEndCol;
        }
        if (eClip != Clip::Row)
        {
            if (nRow1 > mnStartRow)
                nRow1 = mnStartRow;
            if (nRow2 < mnEndRow)
                nRow2 = mnEndRow;
        }
    }

    /// Set the overall range.
    void SetOverallRange( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
    {
        mnStartCol = nCol1;
        mnStartRow = nRow1;
        mnEndCol   = nCol2;
        mnEndRow   = nRow2;
    }
};

struct SC_DLLPUBLIC ScSortParam
{
    SCCOL       nCol1;
    SCROW       nRow1;
    SCCOL       nCol2;
    SCROW       nRow2;
    SCTAB       nSourceTab;
    ScDataAreaExtras aDataAreaExtras;
    sal_uInt16  nUserIndex;
    bool        bHasHeader;
    bool        bByRow;
    bool        bCaseSens;
    bool        bNaturalSort;
    bool        bUserDef;
    bool        bInplace;
    SCTAB       nDestTab;
    SCCOL       nDestCol;
    SCROW       nDestRow;
    ::std::vector<ScSortKeyState>
                maKeyState;
    css::lang::Locale aCollatorLocale;
    OUString    aCollatorAlgorithm;
    sal_uInt16  nCompatHeader;

    ScSortParam();
    ScSortParam( const ScSortParam& r );
    /// SubTotals sort
    ScSortParam( const ScSubTotalParam& rSub, const ScSortParam& rOld );
    /// TopTen sort
    ScSortParam( const ScQueryParam&, SCCOL nCol );
    ~ScSortParam();

    ScSortParam&    operator=  ( const ScSortParam& r );
    bool            operator== ( const ScSortParam& rOther ) const;
    void            Clear       ();
    void            MoveToDest();

    sal_uInt16 GetSortKeyCount() const { return maKeyState.size(); }
};

struct ScSortInfo final
{
    ScRefCellValue maCell;
    SCCOLROW       nOrg;
};

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

        Cell() : mpAttr(nullptr), mpNote(nullptr),  mpPattern(nullptr) {}
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

    void SetOrderIndices( std::vector<SCCOLROW>&& rIndices )
    {
        maOrderIndices = std::move(rIndices);
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

namespace sc {

struct ReorderParam
{
    /**
     * This sort range already takes into account the presence or absence of
     * header row / column i.e. if a header row / column is present, it
     * excludes that row / column.
     */
    ScRange maSortRange;
    ScDataAreaExtras maDataAreaExtras;

    /**
     * List of original column / row positions after reordering.
     */
    std::vector<SCCOLROW> maOrderIndices;
    bool mbByRow;
    bool mbHiddenFiltered;
    bool mbUpdateRefs;
    bool mbHasHeaders;

    /**
     * Reorder the position indices such that it can be used to undo the
     * original reordering.
     */
    void reverse();

    ReorderParam()
        : mbByRow(false)
        , mbHiddenFiltered(false)
        , mbUpdateRefs(false)
        , mbHasHeaders(false)
    {
    }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
