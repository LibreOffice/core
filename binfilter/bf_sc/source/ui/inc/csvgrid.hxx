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

// ============================================================================

#ifndef _SC_CSVGRID_HXX
#define _SC_CSVGRID_HXX

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <bf_svtools/lstner.hxx>
#endif

#include <vector>
#include <memory>

#ifndef _SC_CSVCONTROL_HXX
#include "csvcontrol.hxx"
#endif
#ifndef _SC_CSVSPLITS_HXX
#include "csvsplits.hxx"
#endif

namespace binfilter {
class ColorConfig;

// ----------------------------------------------------------------------------

class EditEngine;
class ScEditEngineDefaulter;
class ScAsciiOptions;
class ScAccessibleCsvControl;


// ============================================================================

const sal_uInt8 CSV_COLFLAG_NONE    = 0x00;         /// Nothing set.
const sal_uInt8 CSV_COLFLAG_SELECT  = 0x01;         /// Column is selected.

const sal_uInt32 CSV_COLUMN_INVALID = CSV_VEC_NOTFOUND;


// ----------------------------------------------------------------------------

/** This struct contains the state of one table column. */
struct ScCsvColState
{
    sal_Int32                   mnType;             /// Data type.
    sal_uInt8                   mnFlags;            /// Flags (i.e. selection state).

    inline explicit             ScCsvColState(
                                        sal_Int32 nType = CSV_TYPE_DEFAULT,
                                        sal_uInt8 nFlags = CSV_COLFLAG_NONE ) :
                                    mnType( nType ), mnFlags( nFlags ) {}

    inline bool                 IsSelected() const;
    inline void                 Select( bool bSel );
};

inline bool ScCsvColState::IsSelected() const
{
    return (mnFlags & CSV_COLFLAG_SELECT) != 0;
}

inline void ScCsvColState::Select( bool bSel )
{
    if( bSel ) mnFlags |= CSV_COLFLAG_SELECT; else mnFlags &= ~CSV_COLFLAG_SELECT;
}


// ----------------------------------------------------------------------------

typedef ::std::vector< ScCsvColState > ScCsvColStateVec;


// ============================================================================

/** A data grid control for the CSV import dialog. The design of this control
    simulates a Calc spreadsheet with row and column headers. */
class ScCsvGrid : public ScCsvControl, public SfxListener
{
private:
    typedef ::std::auto_ptr< ScEditEngineDefaulter > ScEditEnginePtr;

    VirtualDevice               maBackgrDev;        /// Grid background, headers, cell texts.
    VirtualDevice               maGridDev;          /// Data grid with selection and cursor.
    PopupMenu                   maPopup;            /// Popup menu for column types.

    ColorConfig&				mrColorConfig;      /// Application color configuration.
    Color                       maBackColor;        /// Cell background color.
    Color                       maGridColor;        /// Table grid color.
    Color                       maGridPBColor;      /// Grid color for "first imported line" delimiter.
    Color                       maAppBackColor;     /// Background color for unused area.
    Color                       maTextColor;        /// Text color for data area.
    Color                       maHeaderBackColor;  /// Background color for headers.
    Color                       maHeaderGridColor;  /// Grid color for headers.
    Color                       maHeaderTextColor;  /// Text color for headers.
    Color                       maSelectColor;      /// Header color of selected columns.

    ScEditEnginePtr             mpEditEngine;       /// For drawing cell texts.
    Font                        maHeaderFont;       /// Font for column and row headers.
    Font                        maMonoFont;         /// Monospace font for data cells.
    Size                        maWinSize;          /// Size of the control.
    Size                        maEdEngSize;        /// Paper size for edit engine.

    ScCsvSplits                 maSplits;           /// Vector with split positions.
    ScCsvColStateVec            maColStates;        /// State of each column.
    StringVec                   maTypeNames;        /// UI names of data types.
    StringVecVec                maTexts;            /// 2D-vector for cell texts.

    sal_Int32                   mnFirstImpLine;     /// First imported line (0-based).
    sal_uInt32                  mnRecentSelCol;     /// Index of most recently selected column.
    sal_uInt32                  mnMTCurrCol;        /// Current column of mouse tracking.
    bool                        mbMTSelecting;      /// Mouse tracking: true = select, false = deselect.

    // ------------------------------------------------------------------------
public:
    explicit                    ScCsvGrid( ScCsvControl& rParent );
    virtual                     ~ScCsvGrid();

    // common grid handling ---------------------------------------------------
};


// ============================================================================

} //namespace binfilter
#endif

