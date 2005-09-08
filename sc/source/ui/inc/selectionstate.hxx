/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: selectionstate.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:49:17 $
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

#ifndef SC_SELECTIONSTATE_HXX
#define SC_SELECTIONSTATE_HXX

#include <svx/editdata.hxx>

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

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

