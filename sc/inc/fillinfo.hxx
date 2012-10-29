/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_FILLINFO_HXX
#define SC_FILLINFO_HXX

#include <svx/framelinkarray.hxx>
#include "global.hxx"
#include "colorscale.hxx"

class SfxItemSet;
class SvxBrushItem;
class SvxBoxItem;
class SvxLineItem;
class SvxShadowItem;
class Color;

class ScBaseCell;
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
};

struct CellInfo
{
    ScBaseCell*                 pCell;

    const ScPatternAttr*        pPatternAttr;
    const SfxItemSet*           pConditionSet;
    const Color*                pColorScale;
    const ScDataBarInfo*        pDataBar;
    const ScIconSetInfo*        pIconSet;

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

    sal_Bool                        bMarked : 1;
    sal_Bool                        bEmptyCellText : 1;

    sal_Bool                        bMerged : 1;
    sal_Bool                        bHOverlapped : 1;
    sal_Bool                        bVOverlapped : 1;
    sal_Bool                        bAutoFilter : 1;
    sal_Bool                        bPushButton : 1;
    bool                        bPopupButton: 1;
    bool                        bFilterActive:1;

    sal_Bool                        bPrinted : 1;               // when required (pagebreak mode)

    sal_Bool                        bHideGrid : 1;              // output-internal
    sal_Bool                        bEditEngine : 1;            // output-internal

    CellInfo():
        pColorScale(NULL),
        pDataBar(NULL),
        pIconSet(NULL) {}

    ~CellInfo()
    {
        delete pColorScale;
        delete pDataBar;
        delete pIconSet;
    }
};

const SCCOL SC_ROTMAX_NONE = SCCOL_MAX;

struct RowInfo
{
    CellInfo*           pCellInfo;

    sal_uInt16              nHeight;
    SCROW               nRowNo;
    SCCOL               nRotMaxCol;         // SC_ROTMAX_NONE, if nothing

    sal_Bool                bEmptyBack;
    sal_Bool                bEmptyText;
    sal_Bool                bAutoFilter;
    sal_Bool                bPushButton;
    sal_Bool                bChanged;           // TRUE, if not tested

    inline explicit     RowInfo() : pCellInfo( 0 ) {}

private:
                    RowInfo( const RowInfo& );
    RowInfo&        operator=( const RowInfo& );
};

struct ScTableInfo
{
    svx::frame::Array   maArray;
    RowInfo*            mpRowInfo;
    sal_uInt16              mnArrCount;
    bool                mbPageMode;

    explicit            ScTableInfo();
                        ~ScTableInfo();

private:
                        ScTableInfo( const ScTableInfo& );
    ScTableInfo&        operator=( const ScTableInfo& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
