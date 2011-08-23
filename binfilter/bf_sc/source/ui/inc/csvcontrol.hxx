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

#ifndef _SC_CSVCONTROL_HXX
#define _SC_CSVCONTROL_HXX

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef _SC_CSVSPLITS_HXX
#include "csvsplits.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
namespace binfilter {


class ScAccessibleCsvControl;
} //namespace binfilter
namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessible;
} } } }
namespace binfilter {//STRIP009

// ============================================================================

/** Minimum character count for a column in separators mode. */
const sal_Int32 CSV_MINCOLWIDTH         = 8;
/** Maximum length of a cell string. */
const xub_StrLen CSV_MAXSTRLEN          = 0x7FFF;
/** Transparency for header color of selected columns. */
const sal_uInt16 CSV_HDR_TRANSPARENCY   = 85;
/** Minimum distance to border for auto scroll. */
const sal_Int32 CSV_SCROLL_DIST         = 3;

//! TODO make string array dynamic
const sal_Int32 CSV_PREVIEW_LINES       = 32; // maximum count of preview lines
/** Maximum count of columns. */
const sal_Int32 CSV_MAXCOLCOUNT         = MAXCOL + 1;

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


// ============================================================================

/** Exported data of a column (data used in the dialog). */
struct ScCsvExpData
{
    xub_StrLen                  mnIndex;        /// Index of a column.
    sal_uInt8                   mnType;         /// External type of the column.

    inline                      ScCsvExpData() : mnIndex( 0 ), mnType( SC_COL_STANDARD ) {}
    inline                      ScCsvExpData( xub_StrLen nIndex, sal_uInt8 nType ) :
                                    mnIndex( nIndex ), mnType( nType ) {}
};

typedef ::std::vector< ScCsvExpData > ScCsvExpDataVec;


// ============================================================================

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


// ============================================================================

/** Flags for comparison of old and new control layout data. */
typedef sal_uInt32 ScCsvDiff;

const ScCsvDiff CSV_DIFF_EQUAL          = 0x00000000;
const ScCsvDiff CSV_DIFF_POSCOUNT       = 0x00000001;
const ScCsvDiff CSV_DIFF_POSOFFSET      = 0x00000002;
const ScCsvDiff CSV_DIFF_HDRWIDTH       = 0x00000004;
const ScCsvDiff CSV_DIFF_CHARWIDTH      = 0x00000008;
const ScCsvDiff CSV_DIFF_LINECOUNT      = 0x00000010;
const ScCsvDiff CSV_DIFF_LINEOFFSET     = 0x00000020;
const ScCsvDiff CSV_DIFF_HDRHEIGHT      = 0x00000040;
const ScCsvDiff CSV_DIFF_LINEHEIGHT     = 0x00000080;
const ScCsvDiff CSV_DIFF_RULERCURSOR    = 0x00000100;
const ScCsvDiff CSV_DIFF_GRIDCURSOR     = 0x00000200;

const ScCsvDiff CSV_DIFF_HORIZONTAL     = CSV_DIFF_POSCOUNT | CSV_DIFF_POSOFFSET | CSV_DIFF_HDRWIDTH | CSV_DIFF_CHARWIDTH;
const ScCsvDiff CSV_DIFF_VERTICAL       = CSV_DIFF_LINECOUNT | CSV_DIFF_LINEOFFSET | CSV_DIFF_HDRHEIGHT | CSV_DIFF_LINEHEIGHT;
const ScCsvDiff CSV_DIFF_CURSOR         = CSV_DIFF_RULERCURSOR | CSV_DIFF_GRIDCURSOR;


// ----------------------------------------------------------------------------

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

};




// ============================================================================

/** Enumeration of possible commands to change any settings of the CSV controls.
    @descr  Controls have to send commands instead of changing their settings directly.
    This helps to keep the different controls consistent to each other.
    A command can contain 0 to 2 sal_Int32 parameters. In the description of each
    command the required parameters are swown in brackets. [-] means no parameter. */
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


// ----------------------------------------------------------------------------

/** Data for a CSV control command. The stored position data is aways character based,
    it's never a column index (required for internal consistency). */
class ScCsvCmd
{
private:
    ScCsvCmdType                meType;         /// The command.
    sal_Int32                   mnParam1;       /// First parameter.
    sal_Int32                   mnParam2;       /// Second parameter.

public:
    inline explicit             ScCsvCmd() : meType( CSVCMD_NONE ),
                                    mnParam1( CSV_POS_INVALID ), mnParam2( CSV_POS_INVALID ) {}

    inline void                 Set( ScCsvCmdType eType, sal_Int32 nParam1, sal_Int32 nParam2 );

    inline ScCsvCmdType         GetType() const     { return meType; }
    inline sal_Int32            GetParam1() const   { return mnParam1; }
    inline sal_Int32            GetParam2() const   { return mnParam2; }
};

inline void ScCsvCmd::Set( ScCsvCmdType eType, sal_Int32 nParam1, sal_Int32 nParam2 )
{
    meType = eType; mnParam1 = nParam1; mnParam2 = nParam2;
}


// ============================================================================

/** Base class for the CSV ruler and the data grid control. Implements command handling. */
class ScCsvControl : public Control
{
protected:
    typedef ::std::vector< String >     StringVec;
    typedef ::std::vector< StringVec >  StringVecVec;

    typedef ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > XAccessibleRef;

private:
    Link                        maCmdHdl;           /// External command handler.
    ScCsvCmd                    maCmd;              /// Data of last command.
    const ScCsvLayoutData&      mrData;             /// Shared layout data.

    XAccessibleRef              mxAccessible;       /// The accessible object of the control.
    ScAccessibleCsvControl*     mpAccessible;       /// Pointer to the accessible implementation object.
    bool                        mbValidGfx;         /// Content of virtual devices valid?

    // ------------------------------------------------------------------------
public:
    explicit                    ScCsvControl( ScCsvControl& rParent );
    explicit                    ScCsvControl( Window* pParent, const ScCsvLayoutData& rData, WinBits nStyle = 0 );
    explicit                    ScCsvControl( Window* pParent, const ScCsvLayoutData& rData, const ResId& rResId );
    virtual                     ~ScCsvControl();

    // event handling ---------------------------------------------------------

    virtual void                GetFocus();
    virtual void                LoseFocus();

    /** Sends a GetFocus or LoseFocus event to the accessibility object. */
    void                        AccSendFocusEvent( bool bFocused );

    // repaint helpers --------------------------------------------------------


    // command handling -------------------------------------------------------

    /** Sets a new command handler. */
    inline void                 SetCmdHdl( const Link& rHdl ) { maCmdHdl = rHdl; }
    /** Returns the current command handler. */
    inline const Link&          GetCmdHdl() const { return maCmdHdl; }
    /** Returns data of the last command. */
    inline const ScCsvCmd&      GetCmd() const { return maCmd; }

    // layout helpers ---------------------------------------------------------

    /** Returns a reference to the current layout data. */
    inline const ScCsvLayoutData& GetLayoutData() const { return mrData; }
    /** Returns true, if the Right-to-Left layout mode is active. */
    inline bool                 IsRTL() const { return mrData.mbAppRTL; }


    // static helpers ---------------------------------------------------------



    // accessibility ----------------------------------------------------------
};


// ============================================================================

} //namespace binfilter
#endif

