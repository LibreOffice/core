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

class SfxItemSet;
class SvxBrushItem;
class SvxBoxItem;
class SvxLineItem;
class SvxShadowItem;

class ScBaseCell;
class ScPatternAttr;

const BYTE SC_ROTDIR_NONE       = 0;
const BYTE SC_ROTDIR_STANDARD   = 1;
const BYTE SC_ROTDIR_LEFT       = 2;
const BYTE SC_ROTDIR_RIGHT      = 3;
const BYTE SC_ROTDIR_CENTER     = 4;

const BYTE SC_CLIPMARK_NONE     = 0;
const BYTE SC_CLIPMARK_LEFT     = 1;
const BYTE SC_CLIPMARK_RIGHT    = 2;
const BYTE SC_CLIPMARK_SIZE     = 64;

enum ScShadowPart
{
    SC_SHADOW_HSTART,
    SC_SHADOW_VSTART,
    SC_SHADOW_HORIZ,
    SC_SHADOW_VERT,
    SC_SHADOW_CORNER
};

struct CellInfo
{
    ScBaseCell*                 pCell;

    const ScPatternAttr*        pPatternAttr;
    const SfxItemSet*           pConditionSet;

    const SvxBrushItem*         pBackground;

    const SvxBoxItem*           pLinesAttr;         /// original item from document.
    const SvxLineItem*          mpTLBRLine;         /// original item from document.
    const SvxLineItem*          mpBLTRLine;         /// original item from document.

    const SvxShadowItem*        pShadowAttr;            // original item (internal)

    const SvxShadowItem*        pHShadowOrigin;
    const SvxShadowItem*        pVShadowOrigin;

    ScShadowPart                eHShadowPart : 4;           // shadow effective for drawing
    ScShadowPart                eVShadowPart : 4;
    BYTE                        nClipMark;
    USHORT                      nWidth;
    BYTE                        nRotateDir;

    BOOL                        bMarked : 1;
    BOOL                        bEmptyCellText : 1;

    BOOL                        bMerged : 1;
    BOOL                        bHOverlapped : 1;
    BOOL                        bVOverlapped : 1;
    BOOL                        bAutoFilter : 1;
    BOOL                        bPushButton : 1;
    bool                        bPopupButton: 1;
    bool                        bFilterActive:1;

    BOOL                        bPrinted : 1;               // when required (pagebreak mode)

    BOOL                        bHideGrid : 1;              // output-internal
    BOOL                        bEditEngine : 1;            // output-internal
};

const SCCOL SC_ROTMAX_NONE = SCCOL_MAX;

struct RowInfo
{
    CellInfo*           pCellInfo;

    USHORT              nHeight;
    SCROW               nRowNo;
    SCCOL               nRotMaxCol;         // SC_ROTMAX_NONE, if nothing

    BOOL                bEmptyBack;
    BOOL                bEmptyText;
    BOOL                bAutoFilter;
    BOOL                bPushButton;
    BOOL                bChanged;           // TRUE, if not tested

    inline explicit     RowInfo() : pCellInfo( 0 ) {}

private:
                    RowInfo( const RowInfo& );
    RowInfo&        operator=( const RowInfo& );
};

struct ScTableInfo
{
    svx::frame::Array   maArray;
    RowInfo*            mpRowInfo;
    USHORT              mnArrCount;
    bool                mbPageMode;

    explicit            ScTableInfo();
                        ~ScTableInfo();

private:
                        ScTableInfo( const ScTableInfo& );
    ScTableInfo&        operator=( const ScTableInfo& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
