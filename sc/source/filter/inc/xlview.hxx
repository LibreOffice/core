/*************************************************************************
 *
 *  $RCSfile: xlview.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-21 13:48:02 $
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
#ifndef SC_XLVIEW_HXX
#define SC_XLVIEW_HXX

#include <map>

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif
#ifndef SC_XLADDRESS_HXX
#include "xladdress.hxx"
#endif

// Constants and enumerations =================================================

const sal_uInt16 EXC_ZOOM_MIN               = 10;
const sal_uInt16 EXC_ZOOM_MAX               = 400;

// (0x001D) SELECTION ---------------------------------------------------------

const sal_uInt16 EXC_ID_SELECTION           = 0x001D;

// (0x003D) WINDOW1 -----------------------------------------------------------

const sal_uInt16 EXC_ID_WINDOW1             = 0x003D;

const sal_uInt16 EXC_WIN1_HIDDEN            = 0x0001;
const sal_uInt16 EXC_WIN1_MINIMIZED         = 0x0002;
const sal_uInt16 EXC_WIN1_HOR_SCROLLBAR     = 0x0008;
const sal_uInt16 EXC_WIN1_VER_SCROLLBAR     = 0x0010;
const sal_uInt16 EXC_WIN1_TABBAR            = 0x0020;

// (0x003E, 0x023E) WINDOW2 ---------------------------------------------------

const sal_uInt16 EXC_ID2_WINDOW2            = 0x003E;
const sal_uInt16 EXC_ID_WINDOW2             = 0x023E;

const sal_uInt16 EXC_WIN2_SHOWFORMULAS      = 0x0001;
const sal_uInt16 EXC_WIN2_SHOWGRID          = 0x0002;
const sal_uInt16 EXC_WIN2_SHOWHEADINGS      = 0x0004;
const sal_uInt16 EXC_WIN2_FROZEN            = 0x0008;
const sal_uInt16 EXC_WIN2_SHOWZEROS         = 0x0010;
const sal_uInt16 EXC_WIN2_DEFGRIDCOLOR      = 0x0020;
const sal_uInt16 EXC_WIN2_MIRRORED          = 0x0040;
const sal_uInt16 EXC_WIN2_SHOWOUTLINE       = 0x0080;
const sal_uInt16 EXC_WIN2_FROZENNOSPLIT     = 0x0100;
const sal_uInt16 EXC_WIN2_SELECTED          = 0x0200;
const sal_uInt16 EXC_WIN2_DISPLAYED         = 0x0400;
const sal_uInt16 EXC_WIN2_PAGEBREAKMODE     = 0x0800;

const sal_uInt16 EXC_WIN2_NORMALZOOM_DEF    = 100;      /// Default zoom for normal view.
const sal_uInt16 EXC_WIN2_PAGEZOOM_DEF      = 60;       /// Default zoom for pagebreak preview.

// (0x0041) PANE --------------------------------------------------------------

const sal_uInt16 EXC_ID_PANE                = 0x0041;

const sal_uInt8 EXC_PANE_BOTTOMRIGHT        = 0;        /// Bottom-right pane.
const sal_uInt8 EXC_PANE_TOPRIGHT           = 1;        /// Right, or top-right pane.
const sal_uInt8 EXC_PANE_BOTTOMLEFT         = 2;        /// Bottom, or bottom-left pane.
const sal_uInt8 EXC_PANE_TOPLEFT            = 3;        /// Single, top, left, or top-left pane.

// (0x00A0) SCL ---------------------------------------------------------------

const sal_uInt16 EXC_ID_SCL                 = 0x00A0;

// ============================================================================

// View settings ==============================================================

/** Contains all settings for a selection in a single pane of a sheet. */
struct XclSelectionData
{
    XclAddress          maXclCursor;        /// Cell cursor position.
    XclRangeList        maXclSelection;     /// Selected cell ranges.
    sal_uInt16          mnCursorIdx;        /// Index of cursor in selection list.

    inline explicit     XclSelectionData() : mnCursorIdx( 0 ) {}
};

typedef ScfRef< XclSelectionData > XclSelectionDataRef;

// ----------------------------------------------------------------------------

/** Contains all view settings for a single sheet. */
struct XclTabViewData
{
    typedef ::std::map< sal_uInt8, XclSelectionDataRef > XclSelectionMap;

    XclSelectionMap     maSelMap;           /// Selections of all panes.
    Color               maGridColor;        /// Grid color.
    XclAddress          maFirstXclPos;      /// First visible cell.
    XclAddress          maSecondXclPos;     /// First visible cell in additional panes.
    sal_uInt16          mnSplitX;           /// Split X position, or number of frozen columns.
    sal_uInt16          mnSplitY;           /// Split Y position, or number of frozen rows.
    sal_uInt16          mnNormalZoom;       /// Zoom factor for normal view.
    sal_uInt16          mnPageZoom;         /// Zoom factor for pagebreak preview.
    sal_uInt16          mnCurrentZoom;      /// Zoom factor for current view.
    sal_uInt8           mnActivePane;       /// Active pane (with cell cursor).
    bool                mbSelected;         /// true = Sheet is selected.
    bool                mbDisplayed;        /// true = Sheet is displayed (active).
    bool                mbMirrored;         /// true = Mirrored (right-to-left) sheet.
    bool                mbFrozenPanes;      /// true = Frozen panes; false = split window.
    bool                mbPageMode;         /// true = Pagebreak preview; false = Normal view.
    bool                mbDefGridColor;     /// true = Default grid color.
    bool                mbShowFormulas;     /// true = Show formulas instead of results.
    bool                mbShowGrid;         /// true = Show cell grid.
    bool                mbShowHeadings;     /// true = Show column/row headings.
    bool                mbShowZeros;        /// true = Show zero value zells.
    bool                mbShowOutline;      /// true = Show outlines.

    explicit            XclTabViewData();
                        ~XclTabViewData();

    /** Sets Excel default view settings. */
    void                SetDefaults();

    /** Returns true, if the window is split in any direction. */
    bool                IsSplit() const;
    /** Returns true, if the specified pane (EXC_PANE_*) is available. */
    bool                HasPane( sal_uInt8 nPaneId ) const;

    /** Returns the selection data, if available, otherwise 0. */
    const XclSelectionData* GetSelectionData( sal_uInt8 nPane ) const;
    /** Returns read/write access to the selection data of the specified pane. */
    XclSelectionData&   CreateSelectionData( sal_uInt8 nPane );
};

// ============================================================================

#endif

