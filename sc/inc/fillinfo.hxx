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

#include <sal/config.h>

#include <memory>

#include <svx/framelinkarray.hxx>
#include "colorscale.hxx"
#include "cellvalue.hxx"
#include <o3tl/typed_flags_set.hxx>
#include <optional>

class SfxItemSet;
class SvxBrushItem;
class SvxBoxItem;
class SvxLineItem;
class SvxShadowItem;

class ScPatternAttr;

enum class ScRotateDir : sal_uInt8 {
    NONE, Standard, Left, Right, Center
};

enum class ScClipMark : sal_uInt8 {
    NONE = 0x00, Left = 0x01, Right = 0x02, Bottom = 0x03, Top = 0x04
};
namespace o3tl {
    template<> struct typed_flags<ScClipMark> : is_typed_flags<ScClipMark, 0x07> {};
}

const sal_uInt8 SC_CLIPMARK_SIZE     = 64;

enum ScShadowPart
{
    SC_SHADOW_HSTART,
    SC_SHADOW_VSTART,
    SC_SHADOW_HORIZ,
    SC_SHADOW_VERT,
    SC_SHADOW_CORNER
};

struct ScDataBarInfo
{
    double mnZero; // 0 to 100
    Color maColor;
    double mnLength; // -100 to 100
    bool mbGradient;
    bool mbShowValue;
    Color maAxisColor;

    bool operator==(const ScDataBarInfo& r) const
    {
        if( mnZero != r.mnZero )
            return false;
        if( maColor != r.maColor )
            return false;
        if(mnLength != r.mnLength)
            return false;
        if (mbGradient != r.mbGradient)
            return false;

        return true;
    }

    bool operator!=(const ScDataBarInfo& r) const
    {
        return !(*this == r);
    }
};

struct ScIconSetInfo
{
    sal_Int32 nIconIndex;
    ScIconSetType eIconSetType;
    ScConditionMode eConditionMode;
    tools::Long mnHeight = 0;
    bool mbShowValue;
};

// FillInfo() computes some info for all cells starting from column 0,
// but most of the info is needed only for cells in the given columns.
// Keeping all the info in ScCellInfo could lead to allocation and initialization
// of MiB's of memory, so split the info needed for all cells to a smaller structure.
struct ScBasicCellInfo
{
    ScBasicCellInfo()
        : nWidth(0)
        , bEmptyCellText(true)
        , bEditEngine(false)    // view-internal
        {}
    sal_uInt16                  nWidth;
    bool                        bEmptyCellText : 1;
    bool                        bEditEngine : 1;            // output-internal
};

struct ScCellInfo
{
    ScCellInfo()
        : pPatternAttr(nullptr)
        , pConditionSet(nullptr)
        , pDataBar(nullptr)
        , pIconSet(nullptr)
        , maBackground()
        , pLinesAttr(nullptr)
        , mpTLBRLine(nullptr)
        , mpBLTRLine(nullptr)
        , pShadowAttr(nullptr)
        , pHShadowOrigin(nullptr)
        , pVShadowOrigin(nullptr)
        , eHShadowPart(SC_SHADOW_HSTART)
        , eVShadowPart(SC_SHADOW_HSTART)
        , nClipMark(ScClipMark::NONE)
        , nRotateDir(ScRotateDir::NONE)
        , bMerged(false)
        , bHOverlapped(false)
        , bVOverlapped(false)
        , bAutoFilter(false)
        , bPivotButton(false)
        , bPivotPopupButton(false)
        , bFilterActive(false)
        , bPrinted(false)       // view-internal
        , bHideGrid(false)      // view-internal
        , bPivotCollapseButton(false)
        , bPivotExpandButton(false)
        , bPivotPopupButtonMulti(false)
    {
    }

    ScCellInfo(const ScCellInfo&) = delete;
    const ScCellInfo& operator=(const ScCellInfo&) = delete;

    ScRefCellValue              maCell;

    const ScPatternAttr*        pPatternAttr;
    const SfxItemSet*           pConditionSet;
    std::optional<Color>        mxColorScale;
    const ScDataBarInfo*        pDataBar;
    const ScIconSetInfo*        pIconSet;

    SfxPoolItemHolder           maBackground;

    const SvxBoxItem*           pLinesAttr;         /// original item from document.
    const SvxLineItem*          mpTLBRLine;         /// original item from document.
    const SvxLineItem*          mpBLTRLine;         /// original item from document.

    const SvxShadowItem*        pShadowAttr;            // original item (internal)

    const SvxShadowItem*        pHShadowOrigin;
    const SvxShadowItem*        pVShadowOrigin;

    ScShadowPart                eHShadowPart : 4;           // shadow effective for drawing
    ScShadowPart                eVShadowPart : 4;
    ScClipMark                  nClipMark;
    ScRotateDir                 nRotateDir;

    bool                        bMerged : 1;
    bool                        bHOverlapped : 1;
    bool                        bVOverlapped : 1;
    bool                        bAutoFilter : 1;
    bool                        bPivotButton:1;
    bool                        bPivotPopupButton:1;
    bool                        bFilterActive:1;
    bool                        bPrinted : 1;               // when required (pagebreak mode)
    bool                        bHideGrid : 1;              // output-internal
    bool                        bPivotCollapseButton : 1;   // dp compact layout collapse button
    bool                        bPivotExpandButton : 1;     // dp compact layout expand button
    bool                        bPivotPopupButtonMulti : 1; // dp button with popup arrow for multiple fields
};

const SCCOL SC_ROTMAX_NONE = SCCOL_MAX;

struct RowInfo
{
    RowInfo() = default;
    RowInfo(const RowInfo&) = delete;
    const RowInfo& operator=(const RowInfo&) = delete;

    ScCellInfo&         cellInfo(SCCOL nCol)
    {
        assert( nCol >= nStartCol - 1 );
#ifdef DBG_UTIL
        assert( nCol <= nEndCol + 1 );
#endif
        return pCellInfo[ nCol - nStartCol + 1 ];
    }
    const ScCellInfo&   cellInfo(SCCOL nCol) const
    {
        return const_cast<RowInfo*>(this)->cellInfo(nCol);
    }

    ScBasicCellInfo&    basicCellInfo(SCCOL nCol)
    {
        assert( nCol >= -1 );
#ifdef DBG_UTIL
        assert( nCol <= nEndCol + 1 );
#endif
        return pBasicCellInfo[ nCol + 1 ];
    }
    const ScBasicCellInfo& basicCellInfo(SCCOL nCol) const
    {
        return const_cast<RowInfo*>(this)->basicCellInfo(nCol);
    }

    void                allocCellInfo(SCCOL startCol, SCCOL endCol)
    {
        nStartCol = startCol;
#ifdef DBG_UTIL
        nEndCol = endCol;
#endif
        pCellInfo = new ScCellInfo[ endCol - nStartCol + 1 + 2 ];
        pBasicCellInfo = new ScBasicCellInfo[ endCol + 1 + 2 ];
    }
    void                freeCellInfo()
    {
        delete[] pCellInfo;
        delete[] pBasicCellInfo;
    }

    sal_uInt16          nHeight;
    SCROW               nRowNo;
    SCCOL               nRotMaxCol;         // SC_ROTMAX_NONE, if nothing

    bool                bEmptyBack:1;
    bool                bAutoFilter:1;
    bool                bPivotButton:1;
    bool                bChanged:1;           // TRUE, if not tested
    bool                bPivotToggle:1;

private:
    // This class allocates ScCellInfo with also one item extra before and after.
    // To make handling easier, this is private and access functions take care of adjusting
    // the array indexes and error-checking. ScCellInfo is allocated only for a given
    // range of columns plus one on each side, ScBasicCellInfo is allocated for columns
    // starting from column 0 until the last column given, again plus one on each side.
    ScCellInfo*         pCellInfo;
    ScBasicCellInfo*    pBasicCellInfo;
    SCCOL               nStartCol;
#ifdef DBG_UTIL
    SCCOL               nEndCol;
#endif
};

struct ScTableInfo
{
    svx::frame::Array   maArray;
    std::unique_ptr<RowInfo[]>
                        mpRowInfo;
    SCSIZE              mnArrCount;
    SCSIZE              mnArrCapacity;
    bool                mbPageMode;

    explicit            ScTableInfo(SCROW nStartRow, SCROW nEndRow, bool bHintOnly);
                        ~ScTableInfo();
    ScTableInfo(const ScTableInfo&) = delete;
    const ScTableInfo& operator=(const ScTableInfo&) = delete;

    void addDataBarInfo(std::unique_ptr<const ScDataBarInfo> info)
    {
        mDataBarInfos.push_back(std::move(info));
    }
    void addIconSetInfo(std::unique_ptr<const ScIconSetInfo> info)
    {
        mIconSetInfos.push_back(std::move(info));
    }
private:
    // These are owned here and not in ScCellInfo to avoid freeing
    // memory for every pointer in ScCellInfo, most of which are nullptr.
    std::vector<std::unique_ptr<const ScDataBarInfo>> mDataBarInfos;
    std::vector<std::unique_ptr<const ScIconSetInfo>> mIconSetInfos;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
