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

#include <memory>
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include "rangelst.hxx"

/** Extended settings for the document, used in import/export filters. */
struct ScExtDocSettings
{
    OUString            maGlobCodeName;     ///< Global codename (VBA module name).
    double              mfTabBarWidth;      ///< Width of the tabbar, relative to frame window width (0.0 ... 1.0).
    sal_uInt32          mnLinkCnt;          ///< Recursive counter for loading external documents.
    SCTAB               mnDisplTab;         ///< Index of displayed sheet.

    explicit            ScExtDocSettings();
};

/** Enumerates possible positions of panes in split sheets. */
enum ScExtPanePos
{
    SCEXT_PANE_TOPLEFT,         ///< Single, top, left, or top-left pane.
    SCEXT_PANE_TOPRIGHT,        ///< Right, or top-right pane.
    SCEXT_PANE_BOTTOMLEFT,      ///< Bottom, or bottom-left pane.
    SCEXT_PANE_BOTTOMRIGHT      ///< Bottom-right pane.
};

/** Extended settings for a sheet, used in import/export filters. */
struct ScExtTabSettings
{
    ScRange             maUsedArea;         ///< Used area in the sheet (columns/rows only).
    ScRangeList         maSelection;        ///< Selected cell ranges (columns/rows only).
    ScAddress           maCursor;           ///< The cursor position (column/row only).
    ScAddress           maFirstVis;         ///< Top-left visible cell (column/row only).
    ScAddress           maSecondVis;        ///< Top-left visible cell in add. panes (column/row only).
    ScAddress           maFreezePos;        ///< Position of frozen panes (column/row only).
    Point               maSplitPos;         ///< Position of split.
    ScExtPanePos        meActivePane;       ///< Active (focused) pane.
    Color               maGridColor;        ///< Grid color.
    tools::Long                mnNormalZoom;       ///< Zoom in percent for normal view.
    tools::Long                mnPageZoom;         ///< Zoom in percent for pagebreak preview.
    bool                mbSelected;         ///< true = Sheet is selected.
    bool                mbFrozenPanes;      ///< true = Frozen panes; false = Normal splits.
    bool                mbPageMode;         ///< true = Pagebreak mode; false = Normal view mode.
    bool                mbShowGrid;         ///< Whether or not to display gridlines.

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

    /** @return true, if the data needs to be copied to the view data after import. */
    bool                IsChanged() const;
    /** If set to true, the data will be copied to the view data after import. */
    void                SetChanged( bool bChanged );

    /** @return read access to the global document settings. */
    const ScExtDocSettings& GetDocSettings() const;
    /** @return read/write access to the global document settings. */
    ScExtDocSettings&   GetDocSettings();

    /** @return read access to the settings of a sheet, if extant; otherwise 0. */
    const ScExtTabSettings* GetTabSettings( SCTAB nTab ) const;

    /**
     * @return index of the last sheet that has settings, or -1 if no tab
     *         settings are present.
     */
    SCTAB GetLastTab() const;

    /** @return read/write access to the settings of a sheet, may create a new struct. */
    ScExtTabSettings&   GetOrCreateTabSettings( SCTAB nTab );

    /** @return the number of sheet codenames. */
    SCTAB               GetCodeNameCount() const;
    /** @return the specified codename (empty string = no codename). */
    const OUString&     GetCodeName( SCTAB nTab ) const;
    /** Appends a codename for a sheet. */
    void                SetCodeName( SCTAB nTab, const OUString& rCodeName );

private:
    ::std::unique_ptr< ScExtDocOptionsImpl > mxImpl;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
