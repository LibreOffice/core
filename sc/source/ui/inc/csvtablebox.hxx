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

#ifndef INCLUDED_SC_SOURCE_UI_INC_CSVTABLEBOX_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CSVTABLEBOX_HXX

#include <vcl/ctrl.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/vclptr.hxx>
#include "scdllapi.h"
#include "csvcontrol.hxx"
#include "csvruler.hxx"
#include "csvgrid.hxx"

class ListBox;
class ScAsciiOptions;

/* ============================================================================
Position: Positions between the characters (the dots in the ruler).
Character: The characters (the range from one position to the next).
Split: Positions which contain a split to divide characters into groups (columns).
Column: The range between two splits.
============================================================================ */

/** The control in the CSV import dialog that contains a ruler and a data grid
    to visualize and modify the current import settings. */
class SC_DLLPUBLIC ScCsvTableBox : public ScCsvControl
{
private:
    ScCsvLayoutData             maData;             /// Current layout data of the controls.

    VclPtr<ScCsvRuler>          maRuler;            /// The ruler for fixed width mode.
    VclPtr<ScCsvGrid>           maGrid;             /// Calc-like data table for fixed width mode.
    VclPtr<ScrollBar>           maHScroll;          /// Horizontal scroll bar.
    VclPtr<ScrollBar>           maVScroll;          /// Vertical scroll bar.
    VclPtr<ScrollBarBox>        maScrollBox;        /// For the bottom right edge.

    Link<ScCsvTableBox&,void>   maUpdateTextHdl;    /// Updates all cell texts.
    Link<ScCsvTableBox&,void>   maColTypeHdl;       /// Handler for exporting the column type.

    ScCsvColStateVec            maFixColStates;     /// Column states in fixed width mode.
    ScCsvColStateVec            maSepColStates;     /// Column states in separators mode.

    sal_Int32                   mnFixedWidth;       /// Cached total width for fixed width mode.

    bool                        mbFixedMode;        /// false = Separators, true = Fixed width.

public:
    explicit                    ScCsvTableBox( vcl::Window* pParent, WinBits nBits );
    virtual                     ~ScCsvTableBox();
    virtual void                dispose() override;

    /** Finishes initialization. Must be called after constructing a new object. */
    void Init();

    // common table box handling ----------------------------------------------
public:
    /** Sets the control to separators mode. */
    void                        SetSeparatorsMode();
    /** Sets the control to fixed width mode. */
    void                        SetFixedWidthMode();

private:
    /** Initializes the children controls (pos/size, scroll bars, ...). */
    SAL_DLLPRIVATE void                        InitControls();
    /** Initializes size and position data of horizontal scrollbar. */
    SAL_DLLPRIVATE void                        InitHScrollBar();
    /** Initializes size and position data of vertical scrollbar. */
    SAL_DLLPRIVATE void                        InitVScrollBar();

    /** Calculates and sets valid position offset nearest to nPos. */
    SAL_DLLPRIVATE inline void                 ImplSetPosOffset( sal_Int32 nPos )
                                    { maData.mnPosOffset = std::max( std::min( nPos, GetMaxPosOffset() ), sal_Int32( 0 ) ); }
    /** Calculates and sets valid line offset nearest to nLine. */
    SAL_DLLPRIVATE inline void                 ImplSetLineOffset( sal_Int32 nLine )
                                    { maData.mnLineOffset = std::max( std::min( nLine, GetMaxLineOffset() ), sal_Int32( 0 ) ); }
    /** Moves controls (not cursors!) so that nPos becomes visible. */
    SAL_DLLPRIVATE void                        MakePosVisible( sal_Int32 nPos );

    // cell contents ----------------------------------------------------------
public:
    /** Fills all cells of all lines with the passed texts (Unicode strings). */
    void                        SetUniStrings(
                                    const OUString* pTextLines, const OUString& rSepChars,
                                    sal_Unicode cTextSep, bool bMergeSep );

    // column settings --------------------------------------------------------
public:
    /** Reads UI strings for data types from the list box. */
    void                        InitTypes( const ListBox& rListBox );
    /** Returns the data type of the selected columns. */
    inline sal_Int32            GetSelColumnType() const { return maGrid->GetSelColumnType(); }

    /** Fills the options object with current column data. */
    void                        FillColumnData( ScAsciiOptions& rOptions ) const;

    // event handling ---------------------------------------------------------
public:
    /** Sets a new handler for "update cell texts" requests. */
    inline void                 SetUpdateTextHdl( const Link<ScCsvTableBox&,void>& rHdl ) { maUpdateTextHdl = rHdl; }
    /** Sets a new handler for "column selection changed" events. */
    inline void                 SetColTypeHdl( const Link<ScCsvTableBox&,void>& rHdl ) { maColTypeHdl = rHdl; }

protected:
    virtual void                Resize() override;
    virtual void                DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual Size                GetOptimalSize() const override;

private:
    DECL_DLLPRIVATE_LINK_TYPED( CsvCmdHdl, ScCsvControl&, void );
    DECL_DLLPRIVATE_LINK_TYPED( ScrollHdl, ScrollBar*, void );
    DECL_DLLPRIVATE_LINK_TYPED( ScrollEndHdl, ScrollBar*, void );

    // accessibility ----------------------------------------------------------
public:
    /** Creates and returns the accessible object of this control. */
    virtual XAccessibleRef      CreateAccessible() override;

protected:
    /** Creates a new accessible object. */
    virtual rtl::Reference<ScAccessibleCsvControl> ImplCreateAccessible() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
