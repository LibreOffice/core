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
#include "pivot/PivotTableFormats.hxx"
#include "pivot/PivotTableFormatOutput.hxx"

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
class ScDPOutputImpl;
class ScDPObject;

class ScDPOutput
{
private:
    ScDocument* mpDocument;
    sc::FormatOutput maFormatOutput;
    css::uno::Reference<css::sheet::XDimensionsSupplier> mxSource;
    ScAddress maStartPos;
    std::vector<ScDPOutLevelData> mpColFields;
    std::vector<ScDPOutLevelData> mpRowFields;
    std::vector<ScDPOutLevelData> mpPageFields;
    css::uno::Sequence<css::uno::Sequence<css::sheet::DataResult>> maData;
    OUString maDataDescription;

    // Number format related parameters
    std::unique_ptr<sal_uInt32[]> mpColNumberFormat;
    std::unique_ptr<sal_uInt32[]> mpRowNumberFormat;
    sal_Int32 mnColFormatCount;
    sal_Int32 mnRowFormatCount;
    sal_uInt32 mnSingleNumberFormat;
    size_t mnRowDims; // Including empty ones.

    // Output geometry related parameters
    sal_Int32 mnColCount;
    sal_Int32 mnRowCount;
    sal_Int32 mnHeaderSize;
    SCCOL mnTabStartCol;
    SCROW mnTabStartRow;
    SCCOL mnMemberStartCol;
    SCROW mnMemberStartRow;
    SCCOL mnDataStartCol;
    SCROW mnDataStartRow;
    SCCOL mnTabEndCol;
    SCROW mnTabEndRow;
    bool mbDoFilter:1;
    bool mbResultsError:1;
    bool mbSizesValid:1;
    bool mbSizeOverflow:1;
    bool mbHeaderLayout:1;  // true : grid, false : standard
    bool mbHasCompactRowField:1; // true: at least one of the row fields has compact layout.
    bool mbExpandCollapse:1; // true: show expand/collapse buttons
    bool mbHideHeader : 1;

    void            DataCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const css::sheet::DataResult& rData );
    void            HeaderCell( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const css::sheet::MemberResult& rData,
                                bool bColHeader, tools::Long nLevel );

    void FieldCell(SCCOL nCol, SCROW nRow, SCTAB nTab, const ScDPOutLevelData& rData, bool bInTable);
    void MultiFieldCell(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bRowField);

    OUString maRowHeaderCaption;  // override for "Row Labels" cell, from OOXML
    OUString maColHeaderCaption;  // override for "Column Labels" cell, from OOXML
public:
    void setRowHeaderCaption(const OUString& r) { maRowHeaderCaption = r; }
    void setColHeaderCaption(const OUString& r) { maColHeaderCaption = r; }
private:

    /// Computes number of columns needed to write row fields.
    SCCOL           GetColumnsForRowFields() const;
    /// Translate a row-header column offset to the matching mpRowFields
    /// array index, accounting for compact-shared columns. Returns -1 on miss.
    tools::Long     GetRowFieldArrayIndexForColOffset(SCCOL nClickColOff) const;
    void            CalcSizes();

    /** Query which sub-area of the table the cell is in. See
        css.sheet.DataPilotTablePositionType for the interpretation of the
        return value. */
    sal_Int32       GetPositionType(const ScAddress& rPos);
    /// Returns the range of row fields that are contained by table's row fields column nCol.
    void            GetRowFieldRange(SCCOL nCol, sal_Int32& nRowFieldStart, sal_Int32& nRowFieldEnd) const;
    /// Find row field index from row position in case of compact layout.
    sal_Int32       GetRowFieldCompact(SCCOL nColQuery, SCROW nRowQuery) const;

    void outputPageFields(SCTAB nTab);
    void outputColumnHeaders(SCTAB nTab, ScDPOutputImpl& rOutputImpl);
    void outputRowHeader(SCTAB nTab, ScDPOutputImpl& rOutputImpl);
    void outputDataResults(SCTAB nTab);

    // --- Style application support (filled during Output()) -----------------
    // These vectors are sized to the absolute span [maStartPos.Row()..mnTabEndRow]
    // and [maStartPos.Col()..mnTabEndCol] respectively, indexed by absolute
    // row/col minus the corresponding start. A value < 0 means "role does not
    // apply at this row/col" (e.g. for subtotal level on a pure data row).
    std::vector<sal_Int8> maRowSubtotalLevel; // 0..2 (mod 3) or -1
    std::vector<sal_Int8> maColSubtotalLevel; // 0..2 (mod 3) or -1
    std::vector<bool>     maRowIsGrandTotal;
    std::vector<bool>     maColIsGrandTotal;
    std::vector<bool>     maRowIsBlank;
    std::vector<bool>     maRowIsPureData;  // not subtotal, not blank, not grand
    std::vector<bool>     maColIsPureData;
    void initStyleRoleVectors();

public:
    /// Snapshot of cell-role data from the most recent Output(), consumed by
    /// pivot table style application (sc/source/core/data/dppivotstyle.cxx).
    /// All ranges are absolute sheet coordinates.
    struct StyleRoles
    {
        SCROW nTabStartRow = 0; // top of the whole pivot region
        SCCOL nTabStartCol = 0;
        SCROW nMemberStartRow = 0;
        SCROW nDataStartRow = 0; // first row of data results
        SCCOL nDataStartCol = 0;
        SCROW nTabEndRow = 0;
        SCCOL nTabEndCol = 0;
        SCROW nPageFieldFirstRow = -1; // -1 if no page fields
        SCROW nPageFieldLastRow = -1;
        const std::vector<sal_Int8>* pRowSubtotalLevel = nullptr;
        const std::vector<sal_Int8>* pColSubtotalLevel = nullptr;
        const std::vector<bool>*     pRowIsGrandTotal = nullptr;
        const std::vector<bool>*     pColIsGrandTotal = nullptr;
        const std::vector<bool>*     pRowIsBlank = nullptr;
        const std::vector<bool>*     pRowIsPureData = nullptr;
        const std::vector<bool>*     pColIsPureData = nullptr;
        size_t nNumRowFields = 0;
        size_t nNumColFields = 0;
    };

    StyleRoles getStyleRoles() const;

    ScDPOutput(ScDocument* pDocument,
               css::uno::Reference<css::sheet::XDimensionsSupplier> xSource,
               const ScAddress& rPosition, bool bFilter, bool bExpandCollapse,
               ScDPObject& rObject, bool bHideHeader);
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

    void setFormats(sc::PivotTableFormats const& rPivotTableFormats)
    {
        maFormatOutput.setFormats(rPivotTableFormats);
    }

    static void GetDataDimensionNames(
        OUString& rSourceName, OUString& rGivenName,
        const css::uno::Reference< css::uno::XInterface>& xDim );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
