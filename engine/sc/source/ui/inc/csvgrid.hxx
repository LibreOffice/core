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

#include <vector>
#include <scdllapi.h>
#include "csvcontrol.hxx"
#include "csvsplits.hxx"

class ScAsciiOptions;
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

/** Data model for the CSV import dialog grid.

    Holds the column splits, column states, cell texts and selection state
    for the CSV preview. Rendering is performed on the client side: this
    class serialises its state via DumpWidgetData() and receives user
    interactions via HandleCustomEvent(). No VCL drawing is done here. */
class UNLESS_MERGELIBS_MORE(SAL_DLLPUBLIC_RTTI) ScCsvGrid : public ScCsvControl
{
private:
    ScCsvTableBox*              mpTableBox;         /// Grid Parent

    ScCsvSplits                 maSplits;           /// Vector with split positions.
    ScCsvColStateVec            maColStates;        /// State of each column.
    std::vector<OUString>       maTypeNames;        /// UI names of data types.
    std::vector< std::vector<OUString> > maTexts;   /// 2D-vector for cell texts.

    sal_Int32                   mnFirstImpLine;     /// First imported line (0-based).
    sal_uInt32                  mnRecentSelCol;     /// Index of most recently selected column.

public:
    explicit ScCsvGrid(const ScCsvLayoutData& rData, ScCsvTableBox* pTableBox);
    ScCsvTableBox* GetTableBox() { return mpTableBox; }
    virtual ~ScCsvGrid() override;

    static ScCsvSplits          LoadFixedWidthList();
    void                        SaveFixedWidthList();

    // common grid handling ---------------------------------------------------
public:
    void                        ApplyPosCountChange( sal_Int32 nOldPosCount );
    /** Sets the number of the first imported line (for visual feedback). nLine is 0-based! */
    void                        SetFirstImportedLine( sal_Int32 nLine );

    // split handling ---------------------------------------------------------
public:
    const ScCsvSplits&          GetSplits() const { return maSplits; }
    bool                        HasSplit( sal_Int32 nPos ) const { return maSplits.HasSplit( nPos ); }
    /** Inserts a split. */
    void                        InsertSplit( sal_Int32 nPos );
    /** Removes a split. */
    void                        RemoveSplit( sal_Int32 nPos );
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

    /** Returns true, if nColIndex points to an existing column. */
    bool                        IsValidColumn( sal_uInt32 nColIndex ) const;

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

    virtual void                DumpWidgetData( tools::JsonWriter& rWriter ) override;
    virtual OUString            GetCustomWidgetType() const override { return u"csvgrid"_ustr; }
    virtual bool                HandleCustomEvent( const OUString& rCmd,
                                                   const OUString& rData ) override;

private:
    /** Returns the data type of the specified column. */
    sal_Int32                   GetColumnType( sal_uInt32 nColIndex ) const;
    /** Sets the data type of the specified column. */
    void                        SetColumnType( sal_uInt32 nColIndex, sal_Int32 nColType );

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

private:
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
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
