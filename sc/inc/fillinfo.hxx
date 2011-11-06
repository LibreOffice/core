/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

// ============================================================================

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

// ============================================================================

struct CellInfo
{
    ScBaseCell*                 pCell;

    const ScPatternAttr*        pPatternAttr;
    const SfxItemSet*           pConditionSet;

    const SvxBrushItem*         pBackground;

    const SvxBoxItem*           pLinesAttr;         /// Original item from document.
    const SvxLineItem*          mpTLBRLine;         /// Original item from document.
    const SvxLineItem*          mpBLTRLine;         /// Original item from document.

    const SvxShadowItem*        pShadowAttr;            // Original-Item (intern)

    const SvxShadowItem*        pHShadowOrigin;
    const SvxShadowItem*        pVShadowOrigin;

    ScShadowPart                eHShadowPart : 4;           // Schatten effektiv zum Zeichnen
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

    sal_Bool                        bPrinted : 1;               // bei Bedarf (Pagebreak-Modus)

    sal_Bool                        bHideGrid : 1;              // output-intern
    sal_Bool                        bEditEngine : 1;            // output-intern
};

const SCCOL SC_ROTMAX_NONE = SCCOL_MAX;

// ============================================================================

struct RowInfo
{
    CellInfo*           pCellInfo;

    sal_uInt16              nHeight;
    SCROW               nRowNo;
    SCCOL               nRotMaxCol;         // SC_ROTMAX_NONE, wenn nichts

    sal_Bool                bEmptyBack;
    sal_Bool                bEmptyText;
    sal_Bool                bAutoFilter;
    sal_Bool                bPushButton;
    sal_Bool                bChanged;           // sal_True, wenn nicht getestet

    inline explicit     RowInfo() : pCellInfo( 0 ) {}

private:
                    RowInfo( const RowInfo& );
    RowInfo&        operator=( const RowInfo& );
};

// ============================================================================

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

// ============================================================================

#endif

