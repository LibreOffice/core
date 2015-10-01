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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SELECTIONSTATE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SELECTIONSTATE_HXX

#include <editeng/editdata.hxx>
#include "rangelst.hxx"

/** Enumerates all possible types of selections in a Calc document. */
enum ScSelectionType
{
    SC_SELECTTYPE_NONE,             /// No selection, simple cell cursor.
    SC_SELECTTYPE_SHEET,            /// Single cell, cell range, or multi range selection.
    SC_SELECTTYPE_EDITCELL,         /// Cell in edit mode (with or without selection).
    SC_SELECTTYPE_DRAWING,          /// One or more drawing objects.
    SC_SELECTTYPE_EDITDRAW          /// Edit mode in drawing object (with or without selection).
};

class ScViewData;

/** Contains all available data about any possible selection in a Calc document. */
class ScSelectionState
{
public:
    explicit            ScSelectionState( ScViewData& rViewData );

    /** Returns the type of the selection this object contains. */
    inline ScSelectionType GetSelectionType() const { return meType; }

    /** Returns the position of the cell cursor. */
    inline const ScAddress& GetCellCursor() const { return maCursor; }
    /** Returns a range list containing all selected cell ranges. */
    inline const ScRangeList& GetSheetSelection() const { return maSheetSel; }
    /** Returns the edit engine selection. */
    inline const ESelection& GetEditSelection() const { return maEditSel; }

private:
    ScSelectionType     meType;             /// Type of the selection.
    ScAddress           maCursor;           /// Cell cursor position.
    ScRangeList         maSheetSel;         /// Sheet selection.
    ESelection          maEditSel;          /// Selection in edit mode.
};

bool operator==( const ScSelectionState& rL, const ScSelectionState& rR );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
