/*************************************************************************
 *
 *  $RCSfile: fillinfo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:51:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

    ScShadowPart                eHShadowPart;           // Schatten effektiv zum Zeichnen
    ScShadowPart                eVShadowPart;
    const SvxShadowItem*        pHShadowOrigin;
    const SvxShadowItem*        pVShadowOrigin;

    USHORT                      nWidth;

    BOOL                        bMarked;
    BYTE                        nClipMark;
    BOOL                        bEmptyCellText;

    BOOL                        bMerged;
    BOOL                        bHOverlapped;
    BOOL                        bVOverlapped;
    BOOL                        bAutoFilter;
    BOOL                        bPushButton;
    BYTE                        nRotateDir;

    BOOL                        bPrinted;               // bei Bedarf (Pagebreak-Modus)

    BOOL                        bHideGrid;              // output-intern
    BOOL                        bEditEngine;            // output-intern
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

