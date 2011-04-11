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

#ifndef SC_SELECTIONSTATE_HXX
#define SC_SELECTIONSTATE_HXX

#include <editeng/editdata.hxx>
#include "rangelst.hxx"

// ============================================================================

/** Enumerates all possible types of selections in a Calc document. */
enum ScSelectionType
{
    SC_SELECTTYPE_NONE,             /// No selection, simple cell cursor.
    SC_SELECTTYPE_SHEET,            /// Single cell, cell range, or multi range selection.
    SC_SELECTTYPE_EDITCELL,         /// Cell in edit mode (with or without selection).
    SC_SELECTTYPE_DRAWING,          /// One or more drawing objects.
    SC_SELECTTYPE_EDITDRAW          /// Edit mode in drawing object (with or without selection).
};

// ----------------------------------------------------------------------------

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
inline bool operator!=( const ScSelectionState& rL, const ScSelectionState& rR ) { return !(rL == rR); }

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
