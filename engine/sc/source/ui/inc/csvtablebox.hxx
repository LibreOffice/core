/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <vcl/weld.hxx>
#include <scdllapi.h>
#include "csvcontrol.hxx"
#include "csvgrid.hxx"

class ScAsciiOptions;
namespace weld {
    class ComboBox;
}

/* ============================================================================
Position: Positions between the characters (the dots in the ruler).
Character: The characters (the range from one position to the next).
Split: Positions which contain a split to divide characters into groups (columns).
Column: The range between two splits.
============================================================================ */

/** The control in the CSV import dialog that contains a ruler and a data grid
    to visualize and modify the current import settings. */
class SC_DLLPUBLIC ScCsvTableBox
{
private:
    ScCsvLayoutData             maData;             /// Current layout data of the controls.

    std::unique_ptr<ScCsvGrid> mxGrid;              /// Calc-like data table for fixed width mode.
    std::unique_ptr<weld::CustomClientWeld> mxGridWeld;  /// Connect the grid to its box

    Link<ScCsvTableBox&,void>   maUpdateTextHdl;    /// Updates all cell texts.
    Link<ScCsvTableBox&,void>   maColTypeHdl;       /// Handler for exporting the column type.

    ScCsvColStateVec            maFixColStates;     /// Column states in fixed width mode.
    ScCsvColStateVec            maSepColStates;     /// Column states in separators mode.
    ScCsvSplits                 maFixedSplits;      /// Cached splits for fixed width mode.

    sal_Int32                   mnFixedWidth;       /// Cached total width for fixed width mode.

    bool                        mbFixedMode;        /// false = Separators, true = Fixed width.

public:
    explicit ScCsvTableBox(weld::Builder& rBuilder);
    ~ScCsvTableBox();

    // common table box handling ----------------------------------------------
public:
    void                        Refresh();
    /** Sets the control to separators mode. */
    void                        SetSeparatorsMode();
    /** Sets the control to fixed width mode. */
    void                        SetFixedWidthMode();
    bool                        IsFixedWidthMode(){ return mbFixedMode; }

    ScCsvGrid& GetGrid() { return *mxGrid; }

    // cell contents ----------------------------------------------------------
public:
    /** Fills all cells of all lines with the passed texts (Unicode strings). */
    void                        SetUniStrings(
                                    const std::vector<OUString>& rTextLines, const OUString& rSepChars,
                                    sal_Unicode cTextSep, bool bMergeSep, bool bRemoveSpace );

    // column settings --------------------------------------------------------
public:
    /** Reads UI strings for data types from the list box. */
    void                 InitTypes(const weld::ComboBox& rListBox);
    /** Returns the data type of the selected columns. */
    sal_Int32            GetSelColumnType() const { return mxGrid->GetSelColumnType(); }

    /** Fills the options object with current column data. */
    void                        FillColumnData( ScAsciiOptions& rOptions ) const;

    // event handling ---------------------------------------------------------
public:
    /** Sets a new handler for "update cell texts" requests. */
    void                 SetUpdateTextHdl( const Link<ScCsvTableBox&,void>& rHdl ) { maUpdateTextHdl = rHdl; }
    /** Sets a new handler for "column selection changed" events. */
    void                 SetColTypeHdl( const Link<ScCsvTableBox&,void>& rHdl ) { maColTypeHdl = rHdl; }

private:
    DECL_DLLPRIVATE_LINK( CsvCmdHdl, ScCsvControl&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
