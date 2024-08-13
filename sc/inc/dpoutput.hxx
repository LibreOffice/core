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

#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <tools/long.hxx>

#include "address.hxx"

#include "dptypes.hxx"

#include <memory>
#include <vector>

namespace com::sun::star::sheet {
    struct DataPilotFieldFilter;
    struct DataPilotTablePositionData;
    struct DataResult;
    struct MemberResult;
    class XDimensionsSupplier;
}
namespace tools { class Rectangle; }
class ScDocument;
struct ScDPOutLevelData;

class ScDPOutput
{
private:
    ScDocument*             pDoc;
    css::uno::Reference< css::sheet::XDimensionsSupplier> xSource;
    ScAddress               aStartPos;
    std::vector<ScDPOutLevelData>       pColFields;
    std::vector<ScDPOutLevelData>       pRowFields;
    std::vector<ScDPOutLevelData>       pPageFields;
    css::uno::Sequence< css::uno::Sequence< css::sheet::DataResult> > aData;
    OUString                aDataDescription;

    // Number format related parameters
    std::unique_ptr<sal_uInt32[]>
                            pColNumFmt;
    std::unique_ptr<sal_uInt32[]>
                            pRowNumFmt;
    std::vector<bool>       aRowCompactFlags;
    sal_Int32               nColFmtCount;
    sal_Int32               nRowFmtCount;
    sal_uInt32              nSingleNumFmt;
    size_t                  nRowDims; // Including empty ones.

    // Output geometry related parameters
    sal_Int32               nColCount;
    sal_Int32               nRowCount;
    sal_Int32               nHeaderSize;
    SCCOL                   nTabStartCol;
    SCROW                   nTabStartRow;
    SCCOL                   nMemberStartCol;
    SCROW                   nMemberStartRow;
    SCCOL                   nDataStartCol;
    SCROW                   nDataStartRow;
    SCCOL                   nTabEndCol;
    SCROW                   nTabEndRow;
    bool                    bDoFilter:1;
    bool                    bResultsError:1;
    bool                    bSizesValid:1;
    bool                    bSizeOverflow:1;
    bool                    mbHeaderLayout:1;  // true : grid, false : standard
    bool                    mbHasCompactRowField:1; // true: at least one of the row fields has compact layout.
    bool                    mbExpandCollapse:1; // true: show expand/collapse buttons
    bool mbHideHeader : 1;

    void            DataCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const css::sheet::DataResult& rData );
    void            HeaderCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const css::sheet::MemberResult& rData,
                                bool bColHeader, tools::Long nLevel );

    void FieldCell(SCCOL nCol, SCROW nRow, SCTAB nTab, const ScDPOutLevelData& rData, bool bInTable);
    void MultiFieldCell(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bRowField);

    /// Computes number of columns needed to write row fields.
    SCCOL           GetColumnsForRowFields() const;
    void            CalcSizes();

    /** Query which sub-area of the table the cell is in. See
        css.sheet.DataPilotTablePositionType for the interpretation of the
        return value. */
    sal_Int32       GetPositionType(const ScAddress& rPos);
    /// Returns the range of row fields that are contained by table's row fields column nCol.
    void            GetRowFieldRange(SCCOL nCol, sal_Int32& nRowFieldStart, sal_Int32& nRowFieldEnd) const;
    /// Find row field index from row position in case of compact layout.
    sal_Int32       GetRowFieldCompact(SCCOL nColQuery, SCROW nRowQuery) const;

public:
    ScDPOutput(ScDocument* pD, css::uno::Reference<css::sheet::XDimensionsSupplier> xSrc,
               const ScAddress& rPos, bool bFilter, bool bExpandCollapse, bool bHideHeader);
    ~ScDPOutput();

    void            SetPosition( const ScAddress& rPos );

    void            Output();           //! Refresh?
    ScRange GetOutputRange( sal_Int32 nRegionType = css::sheet::DataPilotOutputRangeType::WHOLE );
    sal_Int32       GetHeaderRows() const;
    bool            HasError();         // range overflow or exception from source

    void            GetPositionData(const ScAddress& rPos, css::sheet::DataPilotTablePositionData& rPosData);

    /** Get filtering criteria based on the position of the cell within data
        field region. */
    bool            GetDataResultPositionData(::std::vector< css::sheet::DataPilotFieldFilter >& rFilters, const ScAddress& rPos);

    tools::Long            GetHeaderDim( const ScAddress& rPos, css::sheet::DataPilotFieldOrientation& rOrient );
    bool GetHeaderDrag(
        const ScAddress& rPos, bool bMouseLeft, bool bMouseTop, tools::Long nDragDim,
        tools::Rectangle& rPosRect, css::sheet::DataPilotFieldOrientation& rOrient, tools::Long& rDimPos );
    bool IsFilterButton( const ScAddress& rPos );

    void GetMemberResultNames(ScDPUniqueStringSet& rNames, tools::Long nDimension);

    void            SetHeaderLayout(bool bUseGrid);
    bool            GetHeaderLayout() const { return mbHeaderLayout;}

    static void GetDataDimensionNames(
        OUString& rSourceName, OUString& rGivenName,
        const css::uno::Reference< css::uno::XInterface>& xDim );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
