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

#ifndef INCLUDED_SC_SOURCE_UI_INC_CSVCONTROL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CSVCONTROL_HXX

#include <vcl/ctrl.hxx>
#include <scdllapi.h>
#include <global.hxx>
#include <address.hxx>
#include "csvsplits.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <o3tl/typed_flags_set.hxx>

class ScAccessibleCsvControl;
namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessible;
} } } }

/** Minimum character count for a column in separators mode. */
const sal_Int32 CSV_MINCOLWIDTH         = 8;
/** Maximum length of a cell string. */
const sal_Int32 CSV_MAXSTRLEN          = 0x7FFF;
/** Transparency for header color of selected columns. */
const sal_uInt16 CSV_HDR_TRANSPARENCY   = 85;
/** Minimum distance to border for auto scroll. */
const sal_Int32 CSV_SCROLL_DIST         = 3;

//! TODO make string array dynamic
const sal_Int32 CSV_PREVIEW_LINES       = 32; // maximum count of preview lines
/** Maximum count of columns. */
const sal_Int32 CSV_MAXCOLCOUNT         = MAXCOLCOUNT;

/** Default column data type. */
const sal_Int32 CSV_TYPE_DEFAULT        = 0;
/** Multi selection with different types. */
const sal_Int32 CSV_TYPE_MULTI          = -1;
/** No column selected. */
const sal_Int32 CSV_TYPE_NOSELECTION    = -2;

// External used column types.
const sal_uInt8 SC_COL_STANDARD         = 1;
const sal_uInt8 SC_COL_TEXT             = 2;
const sal_uInt8 SC_COL_MDY              = 3;
const sal_uInt8 SC_COL_DMY              = 4;
const sal_uInt8 SC_COL_YMD              = 5;
const sal_uInt8 SC_COL_SKIP             = 9;
const sal_uInt8 SC_COL_ENGLISH          = 10;

/** Exported data of a column (data used in the dialog). */
struct ScCsvExpData
{
    sal_Int32                   mnIndex;        /// Index of a column.
    sal_uInt8                   mnType;         /// External type of the column.

    ScCsvExpData() : mnIndex( 0 ), mnType( SC_COL_STANDARD ) {}
    ScCsvExpData( sal_Int32 nIndex, sal_uInt8 nType ) :
                                    mnIndex( nIndex ), mnType( nType ) {}
};

typedef ::std::vector< ScCsvExpData > ScCsvExpDataVec;

/** Specifies which element should be used to perform an action. */
enum ScMoveMode
{
    MOVE_NONE,                  /// No action.
    MOVE_FIRST,                 /// First element in current context.
    MOVE_LAST,                  /// Last element in current context.
    MOVE_PREV,                  /// Predecessor of current element in current context.
    MOVE_NEXT,                  /// Successor of current element in current context.
    MOVE_PREVPAGE,              /// Previous page relative to current context.
    MOVE_NEXTPAGE               /// Next page relative to current context.
};

/** Flags for comparison of old and new control layout data. */
enum class ScCsvDiff : sal_uInt32 {
    Equal          = 0x0000,
    PosCount       = 0x0001,
    PosOffset      = 0x0002,
    HeaderWidth    = 0x0004,
    CharWidth      = 0x0008,
    LineCount      = 0x0010,
    LineOffset     = 0x0020,
    HeaderHeight   = 0x0040,
    LineHeight     = 0x0080,
    RulerCursor    = 0x0100,
    GridCursor     = 0x0200,

    HorizontalMask = PosCount | PosOffset | HeaderWidth | CharWidth,
    VerticalMask   = LineCount | LineOffset | HeaderHeight | LineHeight
};
namespace o3tl {
    template<> struct typed_flags<ScCsvDiff> : is_typed_flags<ScCsvDiff, 0x03ff> {};
}


/** A structure containing all layout data valid for both ruler and data grid
    (i.e. scroll position or column width). */
struct ScCsvLayoutData
{
    // horizontal settings
    sal_Int32                   mnPosCount;         /// Number of positions.
    sal_Int32                   mnPosOffset;        /// Horizontal scroll offset.

    sal_Int32                   mnWinWidth;         /// Width of ruler and data grid.
    sal_Int32                   mnHdrWidth;         /// Width of the header column.
    sal_Int32                   mnCharWidth;        /// Pixel width of one character.

    // vertical settings
    sal_Int32                   mnLineCount;        /// Number of data lines.
    sal_Int32                   mnLineOffset;       /// Index of first visible line (0-based).

    sal_Int32                   mnWinHeight;        /// Height of entire data grid (incl. header).
    sal_Int32                   mnHdrHeight;        /// Height of the header line.
    sal_Int32                   mnLineHeight;       /// Height of a data line.

    // cursor settings
    sal_Int32                   mnPosCursor;        /// Position of ruler cursor.
    sal_Int32                   mnColCursor;        /// Position of grid column cursor.

    mutable sal_Int32           mnNoRepaint;        /// >0 = no repaint.
    bool                        mbAppRTL;           /// true = application in RTL mode.

    explicit                    ScCsvLayoutData();

    /** Returns differences to rData.
        @descr  For each difference the appropriate bit is set in the returned value. */
    ScCsvDiff                   GetDiff( const ScCsvLayoutData& rData ) const;
};

inline bool operator==( const ScCsvLayoutData& rData1, const ScCsvLayoutData& rData2 )
{
    return rData1.GetDiff( rData2 ) == ScCsvDiff::Equal;
}

inline bool operator!=( const ScCsvLayoutData& rData1, const ScCsvLayoutData& rData2 )
{
    return !(rData1 == rData2);
}

/** Enumeration of possible commands to change any settings of the CSV controls.
    @descr  Controls have to send commands instead of changing their settings directly.
    This helps to keep the different controls consistent to each other.
    A command can contain 0 to 2 sal_Int32 parameters. In the description of each
    command the required parameters are shown in brackets. [-] means no parameter. */
enum ScCsvCmdType
{
    // misc
    CSVCMD_NONE,                /// No command. [-]
    CSVCMD_REPAINT,             /// Repaint all controls. [-]

    // modify horizontal dimensions
    CSVCMD_SETPOSCOUNT,         /// Change position/column count. [character count]
    CSVCMD_SETPOSOFFSET,        /// Change position offset (scroll pos). [position]
    CSVCMD_SETHDRWIDTH,         /// Change width of the header column. [width in pixel]
    CSVCMD_SETCHARWIDTH,        /// Change character pixel width. [width in pixel]

    // modify vertical dimensions
    CSVCMD_SETLINECOUNT,        /// Change number of data lines. [line count]
    CSVCMD_SETLINEOFFSET,       /// Change first visible line. [line index]
    CSVCMD_SETHDRHEIGHT,        /// Change height of top header line. [height in pixel]
    CSVCMD_SETLINEHEIGHT,       /// Change data line pixel height. [height in pixel}

    // cursors/positions
    CSVCMD_MOVERULERCURSOR,     /// Move ruler cursor to new position. [position]
    CSVCMD_MOVEGRIDCURSOR,      /// Move data grid cursor to new column. [position]
    CSVCMD_MAKEPOSVISIBLE,      /// Move to make passed position visible (for mouse tracking). [position]

    // table contents
    CSVCMD_NEWCELLTEXTS,        /// Recalculate splits and cell texts. [-]
    CSVCMD_UPDATECELLTEXTS,     /// Update cell texts with current split settings. [-]
    CSVCMD_SETCOLUMNTYPE,       /// Change data type of selected columns. [column type]
    CSVCMD_EXPORTCOLUMNTYPE,    /// Send selected column type to external controls. [-]
    CSVCMD_SETFIRSTIMPORTLINE,  /// Set number of first imported line. [line index]

    // splits
    CSVCMD_INSERTSPLIT,         /// Insert a split. [position]
    CSVCMD_REMOVESPLIT,         /// Remove a split. [position]
    CSVCMD_TOGGLESPLIT,         /// Inserts or removes a split. [position]
    CSVCMD_MOVESPLIT,           /// Move a split. [old position, new position]
    CSVCMD_REMOVEALLSPLITS      /// Remove all splits. [-]
};

/** Data for a CSV control command. The stored position data is always character based,
    it's never a column index (required for internal consistency). */
class ScCsvCmd
{
private:
    ScCsvCmdType                meType;         /// The command.
    sal_Int32                   mnParam1;       /// First parameter.
    sal_Int32                   mnParam2;       /// Second parameter.

public:
    explicit             ScCsvCmd() : meType( CSVCMD_NONE ),
                                    mnParam1( CSV_POS_INVALID ), mnParam2( CSV_POS_INVALID ) {}

    inline void                 Set( ScCsvCmdType eType, sal_Int32 nParam1, sal_Int32 nParam2 );

    ScCsvCmdType         GetType() const     { return meType; }
    sal_Int32            GetParam1() const   { return mnParam1; }
    sal_Int32            GetParam2() const   { return mnParam2; }
};

inline void ScCsvCmd::Set( ScCsvCmdType eType, sal_Int32 nParam1, sal_Int32 nParam2 )
{
    meType = eType; mnParam1 = nParam1; mnParam2 = nParam2;
}

/** Base class for the CSV ruler and the data grid control. Implements command handling. */
class SC_DLLPUBLIC ScCsvControl : public Control
{
private:
    Link<ScCsvControl&,void>    maCmdHdl;           /// External command handler.
    ScCsvCmd                    maCmd;              /// Data of last command.
    const ScCsvLayoutData&      mrData;             /// Shared layout data.

    rtl::Reference<ScAccessibleCsvControl> mxAccessible; /// Reference to the accessible implementation object.
    bool                        mbValidGfx;         /// Content of virtual devices valid?

public:
    explicit                    ScCsvControl( ScCsvControl& rParent );
    explicit                    ScCsvControl( vcl::Window* pParent, const ScCsvLayoutData& rData, WinBits nBits );
    virtual                     ~ScCsvControl() override;
    virtual void                dispose() override;

    // event handling ---------------------------------------------------------

    virtual void                GetFocus() override;
    virtual void                LoseFocus() override;

    /** Sends a GetFocus or LoseFocus event to the accessibility object. */
    void                        AccSendFocusEvent( bool bFocused );
    /** Sends a caret changed event to the accessibility object. */
    void                        AccSendCaretEvent();
    /** Sends a visible area changed event to the accessibility object. */
    void                        AccSendVisibleEvent();
    /** Sends a selection changed event to the accessibility object. */
    void                        AccSendSelectionEvent();
    /** Sends a table model changed event for changed cell contents to the accessibility object. */
    void                        AccSendTableUpdateEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn, bool bAllRows = true );
    /** Sends a table model changed event for an inserted column to the accessibility object. */
    void                        AccSendInsertColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn );
    /** Sends a table model changed event for a removed column to the accessibility object. */
    void                        AccSendRemoveColumnEvent( sal_uInt32 nFirstColumn, sal_uInt32 nLastColumn );

    // repaint helpers --------------------------------------------------------

    /** Sets the graphic invalid (next Redraw() will not use cached graphic). */
    void                 InvalidateGfx() { mbValidGfx = false; }
    /** Sets the graphic valid (next Redraw() will use cached graphic). */
    void                 ValidateGfx() { mbValidGfx = true; }
    /** Returns true, if cached graphic is valid. */
    bool                 IsValidGfx() const { return mbValidGfx; }

    /** Repaints all controls.
        @param bInvalidate  true = invalidates graphics of this control (not all). */
    void                        Repaint( bool bInvalidate = false );
    /** Increases no-repaint counter (controls do not repaint until the last EnableRepaint()). */
    void                        DisableRepaint();
    /** Decreases no-repaint counter and repaints if counter reaches 0. */
    void                        EnableRepaint();
    /** Returns true, if controls will not repaint. */
    bool                 IsNoRepaint() const { return mrData.mnNoRepaint > 0; }

    // command handling -------------------------------------------------------

    /** Sets a new command handler. */
    void                 SetCmdHdl( const Link<ScCsvControl&,void>& rHdl ) { maCmdHdl = rHdl; }
    /** Returns data of the last command. */
    const ScCsvCmd&      GetCmd() const { return maCmd; }

    /** Executes a command by calling command handler. */
    void                        Execute(
                                    ScCsvCmdType eType,
                                    sal_Int32 nParam1 = CSV_POS_INVALID,
                                    sal_Int32 nParam2 = CSV_POS_INVALID );

    // layout helpers ---------------------------------------------------------

    /** Returns a reference to the current layout data. */
    const ScCsvLayoutData& GetLayoutData() const { return mrData; }
    /** Returns true, if the Right-to-Left layout mode is active. */
    bool                 IsRTL() const { return mrData.mbAppRTL; }

    /** Returns the number of available positions. */
    sal_Int32            GetPosCount() const { return mrData.mnPosCount; }
    /** Returns the number of visible positions. */
    sal_Int32                   GetVisPosCount() const;
    /** Returns the first visible position. */
    sal_Int32            GetFirstVisPos() const { return mrData.mnPosOffset; }
    /** Returns the last visible position. */
    sal_Int32            GetLastVisPos() const { return GetFirstVisPos() + GetVisPosCount(); }
    /** Returns highest possible position for first visible character. */
    sal_Int32                   GetMaxPosOffset() const;

    /** Returns true, if it is allowed to set a split at nPos. */
    bool                        IsValidSplitPos( sal_Int32 nPos ) const;
    /** Returns true, if nPos is an allowed AND visible split position. */
    bool                        IsVisibleSplitPos( sal_Int32 nPos ) const;

    /** Returns the width of the header column. */
    sal_Int32            GetHdrWidth() const { return mrData.mnHdrWidth; }
    /** Returns the width of one character column. */
    sal_Int32            GetCharWidth() const { return mrData.mnCharWidth; }
    /** Returns the start position of the header column. */
    sal_Int32                   GetHdrX() const;
    /** Returns the X position of the first pixel of the data area. */
    sal_Int32                   GetFirstX() const;
    /** Returns the X position of the last pixel of the data area. */
    sal_Int32                   GetLastX() const;
    /** Returns output X coordinate of the specified position. */
    sal_Int32                   GetX( sal_Int32 nPos ) const;
    /** Returns position from output coordinate. */
    sal_Int32                   GetPosFromX( sal_Int32 nX ) const;

    /** Returns the number of data lines. */
    sal_Int32            GetLineCount() const { return mrData.mnLineCount; }
    /** Returns the number of visible lines (including partly visible bottom line). */
    sal_Int32                   GetVisLineCount() const;
    /** Returns index of first visible line. */
    sal_Int32            GetFirstVisLine() const { return mrData.mnLineOffset; }
    /** Returns index of last visible line. */
    sal_Int32                   GetLastVisLine() const;
    /** Returns highest possible index for first line. */
    sal_Int32                   GetMaxLineOffset() const;

    /** Returns true, if nLine is a valid line index. */
    bool                        IsValidLine( sal_Int32 nLine ) const;
    /** Returns true, if nLine is a valid and visible line index. */
    bool                        IsVisibleLine( sal_Int32 nLine ) const;

    /** Returns the height of the header line. */
    sal_Int32            GetHdrHeight() const { return mrData.mnHdrHeight; }
    /** Returns the height of one line. */
    sal_Int32            GetLineHeight() const { return mrData.mnLineHeight; }
    /** Returns output Y coordinate of the specified line. */
    sal_Int32                   GetY( sal_Int32 nLine ) const;
    /** Returns line index from output coordinate. */
    sal_Int32                   GetLineFromY( sal_Int32 nY ) const;

    /** Returns the ruler cursor position. */
    sal_Int32            GetRulerCursorPos() const { return mrData.mnPosCursor; }
    /** Returns the data grid cursor position (not column index!). */
    sal_Int32            GetGridCursorPos() const { return mrData.mnColCursor; }

    // static helpers ---------------------------------------------------------

    /** Inverts a rectangle in the specified output device. */
    static void                 ImplInvertRect( OutputDevice& rOutDev, const tools::Rectangle& rRect );

    /** Returns direction code for the keys LEFT, RIGHT, HOME, END.
        @param bHomeEnd  false = ignore HOME and END key. */
    static ScMoveMode           GetHorzDirection( sal_uInt16 nCode, bool bHomeEnd );
    /** Returns direction code for the keys UP, DOWN, HOME, END, PAGE UP, PAGE DOWN.
        @param bHomeEnd  false = ignore HOME and END key. */
    static ScMoveMode           GetVertDirection( sal_uInt16 nCode, bool bHomeEnd );

    // accessibility ----------------------------------------------------------
public:
    /** Creates and returns the accessible object of this control. Do not overwrite in
        derived classes, use ImplCreateAccessible() instead. */
    virtual css::uno::Reference< css::accessibility::XAccessible >
                                CreateAccessible() override;

protected:
    /** Derived classes create a new accessible object here. */
    virtual rtl::Reference<ScAccessibleCsvControl> ImplCreateAccessible() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
