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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XLVIEW_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XLVIEW_HXX

#include <map>
#include <tools/color.hxx>
#include "ftools.hxx"
#include "xladdress.hxx"
#include <boost/shared_ptr.hpp>

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

// (0x0862) SHEETEXT ----------------------------------------------------------

const sal_uInt16 EXC_ID_SHEETEXT            = 0x0862;   /// header id for sheetext
const sal_uInt8 EXC_SHEETEXT_TABCOLOR       = 0x7F;     /// mask for tab color
const sal_uInt16 EXC_COLOR_NOTABBG          = 0x7F;     /// Excel ignores Tab color when set to this value...
// Structs ====================================================================

/** Contains all view settings for the entire document. */
struct XclDocViewData
{
    sal_uInt16          mnWinX;             /// X position of the document window (twips).
    sal_uInt16          mnWinY;             /// Y position of the document window (twips).
    sal_uInt16          mnWinWidth;         /// Width of the document window (twips).
    sal_uInt16          mnWinHeight;        /// Height of the document window (twips).
    sal_uInt16          mnFlags;            /// Additional flags.
    sal_uInt16          mnDisplXclTab;      /// Displayed (active) sheet.
    sal_uInt16          mnFirstVisXclTab;   /// First visible sheet.
    sal_uInt16          mnXclSelectCnt;     /// Number of selected sheets.
    sal_uInt16          mnTabBarWidth;      /// Width of sheet tabbar (1/1000 of window width).

    explicit            XclDocViewData();
};

/** Contains all settings for a selection in a single pane of a sheet. */
struct XclSelectionData
{
    XclAddress          maXclCursor;        /// Cell cursor position.
    XclRangeList        maXclSelection;     /// Selected cell ranges.
    sal_uInt16          mnCursorIdx;        /// Index of cursor in selection list.

    inline explicit     XclSelectionData() : mnCursorIdx( 0 ) {}
};

typedef boost::shared_ptr< XclSelectionData > XclSelectionDataRef;

/** Contains all view settings for a single sheet. */
struct XclTabViewData
{
    typedef ::std::map< sal_uInt8, XclSelectionDataRef > XclSelectionMap;

    XclSelectionMap     maSelMap;           /// Selections of all panes.
    Color               maGridColor;        /// Grid color.
    XclAddress          maFirstXclPos;      /// First visible cell.
    XclAddress          maSecondXclPos;     /// First visible cell in additional panes.
    sal_uInt16          mnSplitX;           /// Split X position, or number of frozen columns.
    sal_uInt32          mnSplitY;           /// Split Y position, or number of frozen rows.
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
    Color               maTabBgColor;       /// Tab Color default = (COL_AUTO )
    bool                IsDefaultTabBgColor() const { return maTabBgColor == Color(COL_AUTO); };
    sal_uInt32          mnTabBgColorId;         /// pallette color id

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
