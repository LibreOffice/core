/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillinfo.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-05 16:16:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_FILLINFO_HXX
#define SC_FILLINFO_HXX

#ifndef SVX_FRAMELINKARRAY_HXX
#include <svx/framelinkarray.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class SfxItemSet;
class SvxBrushItem;
class SvxBoxItem;
class SvxLineItem;
class SvxShadowItem;

class ScBaseCell;
class ScPatternAttr;

// ============================================================================

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

    BOOL                        bPrinted : 1;               // bei Bedarf (Pagebreak-Modus)

    BOOL                        bHideGrid : 1;              // output-intern
    BOOL                        bEditEngine : 1;            // output-intern
};

const SCCOL SC_ROTMAX_NONE = SCCOL_MAX;

// ============================================================================

struct RowInfo
{
    CellInfo*           pCellInfo;

    USHORT              nHeight;
    SCROW               nRowNo;
    SCCOL               nRotMaxCol;         // SC_ROTMAX_NONE, wenn nichts

    BOOL                bEmptyBack;
    BOOL                bEmptyText;
    BOOL                bAutoFilter;
    BOOL                bPushButton;
    BOOL                bChanged;           // TRUE, wenn nicht getestet

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
    USHORT              mnArrCount;
    bool                mbPageMode;

    explicit            ScTableInfo();
                        ~ScTableInfo();

private:
                        ScTableInfo( const ScTableInfo& );
    ScTableInfo&        operator=( const ScTableInfo& );
};

// ============================================================================

#endif

