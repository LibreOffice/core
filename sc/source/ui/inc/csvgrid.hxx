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

#include <vcl/virdev.hxx>
#include <unotools/options.hxx>

#include <vector>
#include <memory>
#include <scdllapi.h>
#include <editutil.hxx>
#include "csvcontrol.hxx"
#include "csvsplits.hxx"

namespace svtools { class ColorConfig; }
class EditEngine;
class ScAsciiOptions;
class ScAccessibleCsvControl;
class ScCsvTableBox;

const sal_uInt32 CSV_COLUMN_INVALID = CSV_VEC_NOTFOUND;

/** This struct contains the state of one table column. */
struct ScCsvColState
{
    sal_Int32            mnType;             /// Data type.
    bool                 mbColumnSelected;

    explicit             ScCsvColState( sal_Int32 nType = CSV_TYPE_DEFAULT ) :
                                    mnType( nType ), mbColumnSelected( false ) {}

    bool                 IsSelected() const { return mbColumnSelected; }
    void                 Select( bool bSel ) { mbColumnSelected = bSel; }
};

typedef ::std::vector< ScCsvColState > ScCsvColStateVec;

/** A data grid control for the CSV import dialog. The design of this control
    simulates a Calc spreadsheet with row and column headers. */
class SAL_DLLPUBLIC_RTTI ScCsvGrid : public ScCsvControl, public utl::ConfigurationListener
{
private:
    ScCsvTableBox*              mpTableBox;         /// Grid Parent
    VclPtr<VirtualDevice>       mpBackgrDev;        /// Grid background, headers, cell texts.
    VclPtr<VirtualDevice>       mpGridDev;          /// Data grid with selection and cursor.
    std::unique_ptr<weld::Menu> mxPopup;            /// Popup menu for column types.

    ::svtools::ColorConfig*     mpColorConfig;      /// Application color configuration.
    Color                       maBackColor;        /// Cell background color.
    Color                       maGridColor;        /// Table grid color.
    Color                       maGridPBColor;      /// Grid color for "first imported line" delimiter.
    Color                       maAppBackColor;     /// Background color for unused area.
    Color                       maTextColor;        /// Text color for data area.
    Color                       maHeaderBackColor;  /// Background color for headers.
    Color                       maHeaderGridColor;  /// Grid color for headers.
    Color                       maHeaderTextColor;  /// Text color for headers.
    Color                       maSelectColor;      /// Header color of selected columns.

    std::unique_ptr< ScEditEngineDefaulter >
                                mpEditEngine;       /// For drawing cell texts.
    vcl::Font                   maHeaderFont;       /// Font for column and row headers.
    vcl::Font                   maMonoFont;         /// Monospace font for data cells.
    Size                        maWinSize;          /// Size of the control.
    Size                        maEdEngSize;        /// Paper size for edit engine.

    ScCsvSplits                 maSplits;           /// Vector with split positions.
    ScCsvColStateVec            maColStates;        /// State of each column.
    std::vector<OUString>       maTypeNames;        /// UI names of data types.
    std::vector< std::vector<OUString> > maTexts;   /// 2D-vector for cell texts.

    sal_Int32                   mnFirstImpLine;     /// First imported line (0-based).
    sal_uInt32                  mnRecentSelCol;     /// Index of most recently selected column.
    sal_uInt32                  mnMTCurrCol;        /// Current column of mouse tracking.
    bool                        mbTracking;         /// True if Mouse tracking
    bool                        mbMTSelecting;      /// Mouse tracking mode: true = select, false = deselect.

public:
    explicit ScCsvGrid(const ScCsvLayoutData& rData, std::unique_ptr<weld::Menu> xPopup, ScCsvTableBox* pTableBox);
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    ScCsvTableBox* GetTableBox() { return mpTableBox; }
    virtual ~ScCsvGrid() override;

    /** Finishes initialization. Must be called after constructing a new object. */
    void Init();

    // common grid handling ---------------------------------------------------
public:
    /** Updates layout data dependent from the control's state. */
    void                        UpdateLayoutData();
    /** Updates X coordinate of first visible position dependent from line numbers. */
    void                        UpdateOffsetX();
    /** Apply current layout data to the grid control. */
    void                        ApplyLayout( const ScCsvLayoutData& rOldData );
    /** Sets the number of the first imported line (for visual feedback). nLine is 0-based! */
    void                        SetFirstImportedLine( sal_Int32 nLine );

    /** Finds a column position nearest to nPos which does not cause scrolling the visible area. */
    sal_Int32                   GetNoScrollCol( sal_Int32 nPos ) const;

private:
    /** Reads colors from system settings. */
    void                        InitColors();
    /** Initializes all font settings. */
    void                        InitFonts();
    /** Initializes all data dependent from the control's size. */
    void                        InitSizeData();

    // split handling ---------------------------------------------------------
public:
    /** Inserts a split. */
    void                        InsertSplit( sal_Int32 nPos );
    /** Removes a split. */
    void                        RemoveSplit( sal_Int32 nPos );
    /** Inserts a new or removes an existing split. */
    void                        MoveSplit( sal_Int32 nPos, sal_Int32 nNewPos );
    /** Removes all splits. */
    void                        RemoveAllSplits();
    /** Removes all splits and inserts the splits from rSplits. */
    void                        SetSplits( const ScCsvSplits& rSplits );

private:
    /** Inserts a split and adjusts column data. */
    bool                        ImplInsertSplit( sal_Int32 nPos );
    /** Removes a split and adjusts column data. */
    bool                        ImplRemoveSplit( sal_Int32 nPos );
    /** Clears the split array and re-inserts boundary splits. */
    void                        ImplClearSplits();

    // columns/column types ---------------------------------------------------
public:
    /** Returns the number of columns. */
    sal_uInt32           GetColumnCount() const { return maColStates.size(); }
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

    /** Returns start position of the column with the specified index. */
    sal_Int32            GetColumnPos( sal_uInt32 nColIndex ) const { return maSplits[ nColIndex ]; }
    /** Returns column index from position. A split counts to its following column. */
    sal_uInt32                  GetColumnFromPos( sal_Int32 nPos ) const;
    /** Returns the character width of the column with the specified index. */
    sal_Int32                   GetColumnWidth( sal_uInt32 nColIndex ) const;

    /** Returns the vector with the states of all columns. */
    const ScCsvColStateVec& GetColumnStates() const { return maColStates; }
    /** Sets all column states to the values in the passed vector. */
    void                        SetColumnStates( ScCsvColStateVec&& rColStates );
    /** Returns the data type of the selected columns. */
    SC_DLLPUBLIC sal_Int32      GetSelColumnType() const;
    /** Changes the data type of all selected columns. */
    void                        SetSelColumnType( sal_Int32 nType );
    /** Sets new UI data type names. */
    void                        SetTypeNames( std::vector<OUString>&& rTypeNames );
    /** Returns the UI type name of the specified column. */
    OUString             GetColumnTypeName( sal_uInt32 nColIndex ) const;

    /** Fills the options object with column data for separators mode. */
    void                        FillColumnDataSep( ScAsciiOptions& rOptions ) const;
    /** Fills the options object with column data for fixed width mode. */
    void                        FillColumnDataFix( ScAsciiOptions& rOptions ) const;

private:
    /** Returns the data type of the specified column. */
    sal_Int32                   GetColumnType( sal_uInt32 nColIndex ) const;
    /** Sets the data type of the specified column. */
    void                        SetColumnType( sal_uInt32 nColIndex, sal_Int32 nColType );

    /** Scrolls data grid vertically. */
    void                        ScrollVertRel( ScMoveMode eDir );
    /** Executes the data type popup menu. */
    void                        ExecutePopup( const Point& rPos );

    // selection handling -----------------------------------------------------
public:
    /** Returns true, if the specified column is selected. */
    bool                        IsSelected( sal_uInt32 nColIndex ) const;
    /** Returns index of the first selected column. */
    sal_uInt32                  GetFirstSelected() const;
    /** Returns index of the first selected column really after nFromIndex. */
    sal_uInt32                  GetNextSelected( sal_uInt32 nFromIndex ) const;
    /** Selects or deselects the specified column. */
    void                        Select( sal_uInt32 nColIndex, bool bSelect = true );
    /** Toggles selection of the specified column. */
    void                        ToggleSelect( sal_uInt32 nColIndex );
    /** Selects or deselects the specified column range. */
    void                        SelectRange( sal_uInt32 nColIndex1, sal_uInt32 nColIndex2, bool bSelect = true );
    /** Selects or deselects all columns. */
    void                        SelectAll( bool bSelect = true );

    /** Returns index of the focused column. */
    sal_uInt32           GetFocusColumn() const { return GetColumnFromPos( GetGridCursorPos() ); }

private:
    /** Moves column cursor to a new position. */
    void                        MoveCursor( sal_uInt32 nColIndex );
    /** Moves column cursor to the given direction. */
    void                        MoveCursorRel( ScMoveMode eDir );

    /** Clears the entire selection without notify. */
    void                        ImplClearSelection();

    /** Executes selection action for a specific column. */
    void                        DoSelectAction( sal_uInt32 nColIndex, sal_uInt16 nModifier );

    // cell contents ----------------------------------------------------------
public:
    /** Fills all cells of a line with the passed text (separators mode). */
    void                        ImplSetTextLineSep(
                                    sal_Int32 nLine, const OUString& rTextLine,
                                    const OUString& rSepChars, sal_Unicode cTextSep, bool bMergeSep, bool bRemoveSpace = false );
    /** Fills all cells of a line with the passed text (fixed width mode). */
    void                        ImplSetTextLineFix( sal_Int32 nLine, std::u16string_view rTextLine );

    /** Returns the text of the specified cell. */
    OUString             GetCellText( sal_uInt32 nColIndex, sal_Int32 nLine ) const;

    // event handling ---------------------------------------------------------
protected:
    virtual void                Resize() override;
    virtual void                GetFocus() override;
    virtual void                LoseFocus() override;

    virtual bool                MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool                MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool                MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual bool                KeyInput( const KeyEvent& rKEvt ) override;
    virtual bool                Command( const CommandEvent& rCEvt ) override;

    virtual tools::Rectangle    GetFocusRect() override;

    virtual void                StyleUpdated() override;

    virtual void                ConfigurationChanged( ::utl::ConfigurationBroadcaster*, ConfigurationHints ) override;

    // painting ---------------------------------------------------------------
protected:
    virtual void                Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;

public:
    /** Redraws the entire data grid. */
    void                        ImplRedraw(vcl::RenderContext& rRenderContext);
    /** Returns a pointer to the used edit engine. */
    EditEngine*                 GetEditEngine();

private:
    /** Returns the width of the control. */
    sal_Int32            GetWidth() const { return maWinSize.Width(); }
    /** Returns the height of the control. */
    sal_Int32            GetHeight() const { return maWinSize.Height(); }

    /** Sets a clip region in the specified output device for the specified column. */
    void                        ImplSetColumnClipRegion( OutputDevice& rOutDev, sal_uInt32 nColIndex );
    /** Draws the header of the specified column to the specified output device. */
    void                        ImplDrawColumnHeader( OutputDevice& rOutDev, sal_uInt32 nColIndex, Color aFillColor );

    /** Draws the text at the specified position to maBackgrDev. */
    void                        ImplDrawCellText( const Point& rPos, const OUString& rText );
    /** Draws the "first imported line" separator to maBackgrDev (or erases, if bSet is false). */
    void                        ImplDrawFirstLineSep( bool bSet );
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

    /** Optimized drawing: Scrolls horizontally and redraws only missing parts. */
    void                        ImplDrawHorzScrolled( sal_Int32 nOldPos );

    /** Inverts the cursor bar at the specified position in maGridDev. */
    void                        ImplInvertCursor( sal_Int32 nPos );

    // accessibility ----------------------------------------------------------
protected:
    /** Creates a new accessible object. */
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
