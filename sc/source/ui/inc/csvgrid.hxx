/*************************************************************************
 *
 *  $RCSfile: csvgrid.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2002-07-08 08:19:56 $
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
#include <svtools/lstner.hxx>
#endif

#include <vector>
#include <memory>

#ifndef _SC_CSVCONTROL_HXX
#include "csvcontrol.hxx"
#endif
#ifndef _SC_CSVSPLITS_HXX
#include "csvsplits.hxx"
#endif


// ----------------------------------------------------------------------------

namespace svx { class ColorConfig; }
class ScEditEngineDefaulter;
class ScAsciiOptions;


// ============================================================================

/** A data grid control for the CSV import dialog. The design of this control
    simulates a Calc spreadsheet with row and column headers. */
class ScCsvGrid : public ScCsvControl, public SfxListener
{
private:
    typedef ::std::auto_ptr< ScEditEngineDefaulter >    ScEditEnginePtr;

private:
    VirtualDevice               maBackgrDev;        /// Grid background, headers, cell texts.
    VirtualDevice               maGridDev;          /// Data grid with selection and cursor.
    PopupMenu                   maPopup;            /// Popup menu for column types.

    ::svx::ColorConfig&         mrColorConfig;      /// Application color configuration.
    Color                       maBackColor;        /// Cell background color.
    Color                       maGridColor;        /// Table grid color.
    Color                       maAppBackColor;     /// Background color for unused area.
    Color                       maTextColor;        /// Text color for headers and data.
    Color                       maHeaderBackColor;  /// Background color for headers.
    Color                       maHeaderGridColor;  /// Grid color for headers.
    Color                       maSelectColor;      /// Header color of selected columns.

    ScEditEnginePtr             mpEditEngine;       /// For drawing cell texts.
    Font                        maHeaderFont;       /// Font for column and row headers.
    Font                        maMonoFont;         /// Monospace font for data cells.
    Size                        maWinSize;          /// Size of the control.

    ScCsvSplits                 maSplits;           /// Vector with split positions.
    ScCsvColTypeVec             maColTypes;         /// Data type of each column.
    ScCsvColFlagsVec            maColFlags;         /// Flags for each column.
    ScCsvStringVec              maTypeNames;        /// UI names of data types.
    ScCsvStringVecVec           maTexts;            /// 2D-vector for cell texts.

    sal_uInt32                  mnRecentSelCol;     /// Index of most recently selected column.

    // ------------------------------------------------------------------------
public:
                                ScCsvGrid( ScCsvControl& rParent );
    virtual                     ~ScCsvGrid();

    // grid handling ----------------------------------------------------------

    /** Redraws the entire data grid. */
    void                        ImplRedraw();

    /** Updates layout data dependent from the control's state. */
    void                        UpdateLayoutData();
    /** Updates X coordinate of first visible position dependent from line numbers. */
    void                        UpdateOffsetX();
    /** Apply current layout data to the grid control. */
    void                        ApplyLayout( const ScCsvLayoutData& rOldData );

    /** Finds a column position nearest to nPos which does not cause scrolling the visible area. */
    sal_Int32                   GetNoScrollCol( sal_Int32 nPos ) const;

    /** Inserts a split. */
    void                        InsertSplit( sal_Int32 nPos );
    /** Removes a split. */
    void                        RemoveSplit( sal_Int32 nPos );
    /** Moves a split from nPos to nNewPos. */
    void                        MoveSplit( sal_Int32 nPos, sal_Int32 nNewPos );
    /** Removes all splits. */
    void                        RemoveAllSplits();
    /** Removes all splits and inserts the splits from rSplits. */
    void                        SetSplits( const ScCsvSplits& rSplits );

    /** Returns the vector with the data types of all columns. */
    inline const ScCsvColTypeVec& GetColumnTypes() const { return maColTypes; }
    /** Sets all column data types to the values in the passed vector. */
    void                        SetColumnTypes( const ScCsvColTypeVec& rColTypes );
    /** Returns the data type of the selected columns (or -1, if different types are selected). */
    sal_Int32                   GetSelColumnType() const;
    /** Changes the data type of all selected columns. */
    void                        SetSelColumnType( sal_Int32 nType );
    /** Sets new UI data type names. */
    void                        SetTypeNames( const ScCsvStringVec& rTypeNames );

    /** Fills all cells of a line with the passed text (separators mode). */
    void                        ImplSetTextLineSep(
                                    sal_Int32 nLine, const String& rTextLine,
                                    const String& rSepChars, sal_Unicode cTextSep, bool bMergeSep );
    /** Fills all cells of a line with the passed text (fixed width mode). */
    void                        ImplSetTextLineFix( sal_Int32 nLine, const String& rTextLine );

    /** Fills the options object with column data for separators mode. */
    void                        FillColumnDataSep( ScAsciiOptions& rOptions ) const;
    /** Fills the options object with column data for fixed width mode. */
    void                        FillColumnDataFix( ScAsciiOptions& rOptions ) const;

    // event handling ---------------------------------------------------------
protected:
    virtual void                Resize();
    virtual void                GetFocus();
    virtual void                LoseFocus();

    virtual void                MouseButtonDown( const MouseEvent& rMEvt );
    virtual void                KeyInput( const KeyEvent& rKEvt );
    virtual void                Command( const CommandEvent& rCEvt );

    virtual void                DataChanged( const DataChangedEvent& rDCEvt );
    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // initialization ---------------------------------------------------------
private:
    /** Reads colors from system settings. */
    void                        InitColors();
    /** Initializes all font settings. */
    void                        InitFonts();
    /** Initializes all data dependent from the control's size. */
    void                        InitSizeData();

    // grid handling ----------------------------------------------------------

    /** Inserts a split and adjusts column data. */
    bool                        ImplInsertSplit( sal_Int32 nPos );
    /** Removes a split and adjusts column data. */
    bool                        ImplRemoveSplit( sal_Int32 nPos );
    /** Clears the split array and re-inserts boundary splits. */
    void                        ImplClearSplits();

    /** Returns the number of columns. */
    inline sal_uInt32           GetColumnCount() const { return maColTypes.size(); }
    /** Returns start position of the column with the specified index. */
    inline sal_Int32            GetColumnPos( sal_uInt32 nColIndex ) const { return maSplits[ nColIndex ]; }
    /** Returns the character width of the column with the specified index. */
    sal_Int32                   GetColumnWidth( sal_uInt32 nColIndex ) const;
    /** Returns column index from position. A split counts to its following column. */
    sal_uInt32                  GetColumnFromPos( sal_Int32 nPos ) const;
    /** Returns the index of the first visible column. */
    sal_uInt32                  GetFirstVisColumn() const;
    /** Returns the index of the last visible column. */
    sal_uInt32                  GetLastVisColumn() const;

    /** Returns true, if nColIndex points to an existing column. */
    bool                        IsValidColumn( sal_uInt32 nColIndex ) const;
    /** Returns true, if column with index nColIndex is (at least partly) visible. */
    bool                        IsVisibleColumn( sal_uInt32 nColIndex ) const;

    /** Returns X coordinate of the specified column. */
    sal_Int32                   GetColumnX( sal_uInt32 nColIndex ) const;
    /** Returns column index from output coordinate. */
    sal_uInt32                  GetColumnFromX( sal_Int32 nX ) const;

    /** Returns the data type of the specified column. */
    sal_Int32                   GetColumnType( sal_uInt32 nColIndex ) const;
    /** Returns the data type of the specified column. */
    void                        SetColumnType( sal_uInt32 nColIndex, sal_Int32 nColType );
    /** Returns the UI type name of the specified column. */
    const String&               GetColumnTypeName( sal_uInt32 nColIndex ) const;

    /** Scrolls data grid vertically. */
    void                        ScrollVertRel( ScMoveMode eDir );
    /** Executes the data type popup menu. */
    void                        ExecutePopup( const Point& rPos );

    // selection handling -----------------------------------------------------

    /** Returns index of the focused column. */
    inline sal_uInt32           GetFocusColumn() const { return GetColumnFromPos( GetGridCursorPos() ); }
    /** Moves column cursor to a new position. */
    void                        MoveCursor( sal_uInt32 nColIndex );
    /** Moves column cursor to the given direction. */
    void                        MoveCursorRel( ScMoveMode eDir );

    /** Clears the entire selection without notify. */
    void                        ImplClearSelection();

    /** Returns true, if the specified column is selected. */
    bool                        IsSelected( sal_uInt32 nColIndex ) const;
    /** Returns index of the first selected column. */
    sal_uInt32                  GetFirstSelected() const;
    /** Returns index of the first selected column really after nFromIndex. */
    sal_uInt32                  GetNextSelected( sal_uInt32 nFromIndex ) const;
    /** Returns true, if at least one column is selected. */
    inline bool                 HasSelection() const { return GetFirstSelected() != VEC_NOTFOUND; }

    /** Selects or deselects the specified column. */
    void                        Select( sal_uInt32 nColIndex, bool bSelect = true );
    /** Toggles selection of the specified column. */
    void                        ToggleSelect( sal_uInt32 nColIndex );
    /** Selects or deselects the specified column range. */
    void                        SelectRange( sal_uInt32 nColIndex1, sal_uInt32 nColIndex2, bool bSelect = true );
    /** Selects all columns. */
    void                        SelectAll();

    // painting ---------------------------------------------------------------
protected:
    virtual void                Paint( const Rectangle& );

private:
    /** Returns the width of the control. */
    inline sal_Int32            GetWidth() const { return maWinSize.Width(); }
    /** Returns the height of the control. */
    inline sal_Int32            GetHeight() const { return maWinSize.Height(); }

    /** Sets a clip region in the specified output device for the specified column. */
    void                        ImplSetColumnClipRegion( OutputDevice& rOutDev, sal_uInt32 nColIndex );
    /** Draws the header of the specified column to the specified output device. */
    void                        ImplDrawColumnHeader( OutputDevice& rOutDev, sal_uInt32 nColIndex, Color aFillColor );

    /** Draws the column with index nColIndex to maBackgrDev. */
    void                        ImplDrawColumnBackgr( sal_uInt32 nColIndex );
    /** Draws the row headers column to maBackgrDev. */
    void                        ImplDrawRowHeaders();
    /** Draws all columns and the row headers column to maBackgrDev. */
    void                        ImplDrawBackgrDev();

    /** Draws the column with index nColIndex with its selection state to maGridDev. */
    void                        ImplDrawColumnSelection( sal_uInt32 nColIndex );
    /** Draws all columns with selection and cursor to maGridDev. */
    void                        ImplDrawGridDev();

    /** Redraws the entire column (background and selection). */
    void                        ImplDrawColumn( sal_uInt32 nColIndex );

    /** Draws the cursor bar to the specified position to maGridDev. */
    void                        ImplDrawCursor( sal_Int32 nPos );
    /** Erases the cursor bar from the specified position from maGridDev. */
    inline void                 ImplEraseCursor( sal_Int32 nPos ) { ImplDrawCursor( nPos ); }

    /** Draws directly tracking rectangle to the column with the specified index. */
    void                        ImplDrawTrackingRect( sal_uInt32 nColIndex );
};


// ============================================================================

#endif

