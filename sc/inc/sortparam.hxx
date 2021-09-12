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
#include <com/sun/star/lang/Locale.hpp>
#include "scdllapi.h"

struct ScSubTotalParam;
struct ScQueryParam;

struct ScSortKeyState
{
    SCCOLROW nField;
    bool     bDoSort;
    bool     bAscending;
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
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
