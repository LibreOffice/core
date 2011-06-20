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
#ifndef SC_SCEXTOPT_HXX
#define SC_SCEXTOPT_HXX

#include <memory>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include "global.hxx"
#include "rangelst.hxx"

/** Extended settings for the document, used in import/export filters. */
struct ScExtDocSettings
{
    String              maGlobCodeName;     /// Global codename (VBA module name).
    double              mfTabBarWidth;      /// Width of the tabbar, relative to frame window width (0.0 ... 1.0).
    sal_uInt32          mnLinkCnt;          /// Recursive counter for loading external documents.
    SCTAB               mnDisplTab;         /// Index of displayed sheet.

    explicit            ScExtDocSettings();
};

/** Enumerates possible positions of panes in split sheets. */
enum ScExtPanePos
{
    SCEXT_PANE_TOPLEFT,         /// Single, top, left, or top-left pane.
    SCEXT_PANE_TOPRIGHT,        /// Right, or top-right pane.
    SCEXT_PANE_BOTTOMLEFT,      /// Bottom, or bottom-left pane.
    SCEXT_PANE_BOTTOMRIGHT      /// Bottom-right pane.
};

/** Extended settings for a sheet, used in import/export filters. */
struct ScExtTabSettings
{
    ScRange             maUsedArea;         /// Used area in the sheet (columns/rows only).
    ScRangeList         maSelection;        /// Selected cell ranges (columns/rows only).
    ScAddress           maCursor;           /// The cursor position (column/row only).
    ScAddress           maFirstVis;         /// Top-left visible cell (column/row only).
    ScAddress           maSecondVis;        /// Top-left visible cell in add. panes (column/row only).
    ScAddress           maFreezePos;        /// Position of frozen panes (column/row only).
    Point               maSplitPos;         /// Position of split.
    ScExtPanePos        meActivePane;       /// Active (focused) pane.
    Color               maGridColor;        /// Grid color.
    long                mnNormalZoom;       /// Zoom in percent for normal view.
    long                mnPageZoom;         /// Zoom in percent for pagebreak preview.
    bool                mbSelected;         /// true = Sheet is selected.
    bool                mbFrozenPanes;      /// true = Frozen panes; false = Normal splits.
    bool                mbPageMode;         /// true = Pagebreak mode; false = Normal view mode.
    bool                mbShowGrid;         /// Whether or not to display gridlines.

    explicit            ScExtTabSettings();
};

struct ScExtDocOptionsImpl;

/** Extended options held by an ScDocument containing additional settings for filters.

    This object is owned by a Calc document. It contains global document settings
    (struct ScExtDocSettings), settings for all sheets in the document
    (struct ScExtTabSettings), and a list of codenames used for VBA import/export.
 */
class SC_DLLPUBLIC ScExtDocOptions
{
public:
    explicit            ScExtDocOptions();
                        ScExtDocOptions( const ScExtDocOptions& rSrc );
                        ~ScExtDocOptions();

    ScExtDocOptions&    operator=( const ScExtDocOptions& rSrc );

    /** Returns true, if the data needs to be copied to the view data after import. */
    bool                IsChanged() const;
    /** If set to true, the data will be copied to the view data after import. */
    void                SetChanged( bool bChanged );

    /** Returns read access to the global document settings. */
    const ScExtDocSettings& GetDocSettings() const;
    /** Returns read/write access to the global document settings. */
    ScExtDocSettings&   GetDocSettings();

    /** Returns read access to the settings of a sheet, if extant; otherwise 0. */
    const ScExtTabSettings* GetTabSettings( SCTAB nTab ) const;
    /** Returns read/write access to the settings of a sheet, may create a new struct. */
    ScExtTabSettings&   GetOrCreateTabSettings( SCTAB nTab );

    /** Returns the number of sheet codenames. */
    SCTAB               GetCodeNameCount() const;
    /** Returns the specified codename (empty string = no codename). */
    const String&       GetCodeName( SCTAB nTab ) const;
    /** Appends a codename for a sheet. */
    void                SetCodeName( SCTAB nTab, const String& rCodeName );

private:
    ::std::auto_ptr< ScExtDocOptionsImpl > mxImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
