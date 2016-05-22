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

#ifndef INCLUDED_SC_INC_FILLINFO_HXX
#define INCLUDED_SC_INC_FILLINFO_HXX

#include <sal/config.h>

#include <memory>

#include <svx/framelinkarray.hxx>
#include "global.hxx"
#include "colorscale.hxx"
#include "cellvalue.hxx"

class SfxItemSet;
class SvxBrushItem;
class SvxBoxItem;
class SvxLineItem;
class SvxShadowItem;
class Color;

class ScPatternAttr;

const sal_uInt8 SC_ROTDIR_NONE       = 0;
const sal_uInt8 SC_ROTDIR_STANDARD   = 1;
const sal_uInt8 SC_ROTDIR_LEFT       = 2;
const sal_uInt8 SC_ROTDIR_RIGHT      = 3;
const sal_uInt8 SC_ROTDIR_CENTER     = 4;

const sal_uInt8 SC_CLIPMARK_NONE     = 0;
const sal_uInt8 SC_CLIPMARK_LEFT     = 1;
const sal_uInt8 SC_CLIPMARK_RIGHT    = 2;
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
    bool mbShowValue;
};

struct CellInfo
{
    CellInfo()
        : pPatternAttr(nullptr)
        , pConditionSet(nullptr)
        , pBackground(nullptr)   // TODO: omit?
        , pLinesAttr(nullptr)
        , mpTLBRLine(nullptr)
        , mpBLTRLine(nullptr)
        , pShadowAttr(nullptr)
        , pHShadowOrigin(nullptr)
        , pVShadowOrigin(nullptr)
        , eHShadowPart(SC_SHADOW_HSTART)
        , eVShadowPart(SC_SHADOW_HSTART)
        , nClipMark(SC_CLIPMARK_NONE)
        , nWidth(0)
        , nRotateDir(SC_ROTDIR_NONE)
        , bMarked(false)
        , bEmptyCellText(false)
        , bMerged(false)
        , bHOverlapped(false)
        , bVOverlapped(false)
        , bAutoFilter(false)
        , bPivotButton(false)
        , bPivotPopupButton(false)
        , bFilterActive(false)
        , bPrinted(false)       // view-internal
        , bHideGrid(false)      // view-internal
        , bEditEngine(false)    // view-internal
    {
    }

    ~CellInfo() = default;
    CellInfo(const CellInfo&) = delete;
    const CellInfo& operator=(const CellInfo&) = delete;

    ScRefCellValue              maCell;

    const ScPatternAttr*        pPatternAttr;
    const SfxItemSet*           pConditionSet;
    std::unique_ptr<const Color> pColorScale;
    std::unique_ptr<const ScDataBarInfo> pDataBar;
    std::unique_ptr<const ScIconSetInfo> pIconSet;

    const SvxBrushItem*         pBackground;

    const SvxBoxItem*           pLinesAttr;         /// original item from document.
    const SvxLineItem*          mpTLBRLine;         /// original item from document.
    const SvxLineItem*          mpBLTRLine;         /// original item from document.

    const SvxShadowItem*        pShadowAttr;            // original item (internal)

    const SvxShadowItem*        pHShadowOrigin;
    const SvxShadowItem*        pVShadowOrigin;

    ScShadowPart                eHShadowPart : 4;           // shadow effective for drawing
    ScShadowPart                eVShadowPart : 4;
    sal_uInt8                        nClipMark;
    sal_uInt16                      nWidth;
    sal_uInt8                        nRotateDir;

    bool                        bMarked : 1;
    bool                        bEmptyCellText : 1;
    bool                        bMerged : 1;
    bool                        bHOverlapped : 1;
    bool                        bVOverlapped : 1;
    bool                        bAutoFilter : 1;
    bool                        bPivotButton:1;
    bool                        bPivotPopupButton:1;
    bool                        bFilterActive:1;
    bool                        bPrinted : 1;               // when required (pagebreak mode)
    bool                        bHideGrid : 1;              // output-internal
    bool                        bEditEngine : 1;            // output-internal
};

const SCCOL SC_ROTMAX_NONE = SCCOL_MAX;

struct RowInfo
{
    RowInfo() = default;
    ~RowInfo() = default;
    RowInfo(const RowInfo&) = delete;
    const RowInfo& operator=(const RowInfo&) = delete;

    CellInfo*           pCellInfo;

    sal_uInt16          nHeight;
    SCROW               nRowNo;
    SCCOL               nRotMaxCol;         // SC_ROTMAX_NONE, if nothing

    bool                bEmptyBack:1;
    bool                bEmptyText:1;
    bool                bAutoFilter:1;
    bool                bPivotButton:1;
    bool                bChanged:1;           // TRUE, if not tested
};

struct ScTableInfo
{
    svx::frame::Array   maArray;
    RowInfo*            mpRowInfo;
    sal_uInt16              mnArrCount;
    bool                mbPageMode;

    explicit            ScTableInfo();
                        ~ScTableInfo();
    ScTableInfo(const ScTableInfo&) = delete;
    const ScTableInfo& operator=(const ScTableInfo&) = delete;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
