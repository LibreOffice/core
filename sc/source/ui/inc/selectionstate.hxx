/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

