/*************************************************************************
 *
 *  $RCSfile: csvcontrol.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dr $ $Date: 2002-07-23 15:21:58 $
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

#ifndef _SC_CSVCONTROL_HXX
#define _SC_CSVCONTROL_HXX

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif

#ifndef _SC_CSVSPLITS_HXX
#include "csvsplits.hxx"
#endif


// ============================================================================

typedef ::std::vector< String >             ScCsvStringVec;
typedef ::std::vector< ScCsvStringVec >     ScCsvStringVecVec;
typedef ::std::vector< sal_Int32 >          ScCsvColTypeVec;
typedef ::std::vector< sal_uInt8 >          ScCsvColFlagsVec;

typedef ::std::vector< xub_StrLen >         ScCsvExtColPosVec;
typedef ::std::vector< sal_uInt8 >          ScCsvExtColTypeVec;


// ----------------------------------------------------------------------------

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

// flags for comparison.
const sal_uInt32 CSV_DIFF_POSCOUNT      = 0x00000001;
const sal_uInt32 CSV_DIFF_POSOFFSET     = 0x00000002;
const sal_uInt32 CSV_DIFF_OFFSETX       = 0x00000004;
const sal_uInt32 CSV_DIFF_CHARWIDTH     = 0x00000008;
const sal_uInt32 CSV_DIFF_LINECOUNT     = 0x00000010;
const sal_uInt32 CSV_DIFF_LINEOFFSET    = 0x00000020;
const sal_uInt32 CSV_DIFF_OFFSETY       = 0x00000040;
const sal_uInt32 CSV_DIFF_LINEHEIGHT    = 0x00000080;
const sal_uInt32 CSV_DIFF_RULERCURSOR   = 0x00000100;
const sal_uInt32 CSV_DIFF_GRIDCURSOR    = 0x00000200;

const sal_uInt32 CSV_DIFF_HORIZONTAL    = CSV_DIFF_POSCOUNT | CSV_DIFF_POSOFFSET | CSV_DIFF_OFFSETX | CSV_DIFF_CHARWIDTH;
const sal_uInt32 CSV_DIFF_VERTICAL      = CSV_DIFF_LINECOUNT | CSV_DIFF_LINEOFFSET | CSV_DIFF_OFFSETY | CSV_DIFF_LINEHEIGHT;
const sal_uInt32 CSV_DIFF_CURSOR        = CSV_DIFF_RULERCURSOR | CSV_DIFF_GRIDCURSOR;


// ----------------------------------------------------------------------------

/** A structure containing all layout data valid for both ruler and data grid
    (i.e. scroll position or column width). */
struct ScCsvLayoutData
{
    // horizontal settings
    sal_Int32                   mnPosCount;         /// Number of positions.
    sal_Int32                   mnPosOffset;        /// Horizontal scroll offset.

    sal_Int32                   mnWinWidth;         /// Width of ruler and data grid.
    sal_Int32                   mnOffsetX;          /// X coordinate of first visible position.
    sal_Int32                   mnCharWidth;        /// Pixel width of one character.

    // vertical settings
    sal_Int32                   mnLineCount;        /// Number of data lines.
    sal_Int32                   mnLineOffset;       /// Index of first visible line (0-based).

    sal_Int32                   mnWinHeight;        /// Height of entire data grid (incl. header).
    sal_Int32                   mnOffsetY;          /// Y coordinate of first visible line.
    sal_Int32                   mnLineHeight;       /// Height of one line.

    // cursor settings
    sal_Int32                   mnPosCursor;        /// Position of ruler cursor.
    sal_Int32                   mnColCursor;        /// Position of grid column cursor.

    mutable sal_Int32           mnNoRepaint;        /// >0 = no repaint.

                                ScCsvLayoutData();

    /** Returns differences to rData.
        @descr  For each difference the appropriate bit is set in the returned value. */
    sal_uInt32                  GetDiff( const ScCsvLayoutData& rData ) const;
};

inline bool operator==( const ScCsvLayoutData& rData1, const ScCsvLayoutData& rData2 )
{
    return rData1.GetDiff( rData2 ) == 0;
}

inline bool operator!=( const ScCsvLayoutData& rData1, const ScCsvLayoutData& rData2 )
{
    return !(rData1 == rData2);
}


// ============================================================================

/** Enumeration of possible requests of the controls.
    @descr  Controls have to send requests to be able to change the tablebox global settings. */
enum ScCsvRequestType
{
    CSVREQ_NONE,                /// No request.

    // misc
    CSVREQ_REPAINT,             /// Repaint all controls.
    CSVREQ_NEWCELLTEXTS,        /// Recalculate splits and cell texts.
    CSVREQ_UPDATECELLTEXTS,     /// Update cell texts with current split settings.
    CSVREQ_COLUMNTYPE,          /// Change data type of selected columns.

    // modify horizontal dimensions
    CSVREQ_POSCOUNT,            /// Change position/column count.
    CSVREQ_POSOFFSET,           /// Change position offset (scroll pos).
    CSVREQ_OFFSETX,             /// Change X coordinate of first visible position.
    CSVREQ_CHARWIDTH,           /// Change character pixel width.

    // modify vertical dimensions
    CSVREQ_LINECOUNT,           /// Change number of data lines.
    CSVREQ_LINEOFFSET,          /// Change first visible line.
    CSVREQ_OFFSETY,             /// Change Y coordinate of first visible line.
    CSVREQ_LINEHEIGHT,          /// Change data line pixel height.

    // cursors/positions
    CSVREQ_MOVERULERCURSOR,     /// Move ruler cursor to new position.
    CSVREQ_MOVEGRIDCURSOR,      /// Move data grid cursor to new column.
    CSVREQ_MAKEPOSVISIBLE       /// Move to make passed position visible (for mouse tracking).
};


// ----------------------------------------------------------------------------

/** Data for a CSV control request. */
class ScCsvRequest
{
private:
    ScCsvRequestType            meType;         /// Type of the request.
    sal_Int32                   mnData;         /// Signed position data.

public:
    inline                      ScCsvRequest() : meType( CSVREQ_NONE ), mnData( POS_INVALID ) {}

    inline void                 Set( ScCsvRequestType eType, sal_Int32 nData );

    inline ScCsvRequestType     GetType() const     { return meType; }
    inline sal_Int32            GetData() const     { return mnData; }
};

inline void ScCsvRequest::Set( ScCsvRequestType eType, sal_Int32 nData )
{
    meType = eType;
    mnData = nData;
}


// ============================================================================

/** Enumeration of possible events of the controls.
    @descr  Controls send events after they have changed their own data. */
enum ScCsvEventType
{
    CSVEVENT_NONE,              /// No action.

    // ruler events
    RULEREVENT_INSERT,          /// Split inserted.
    RULEREVENT_REMOVE,          /// Split removed.
    RULEREVENT_MOVE,            /// Split moved.
    RULEREVENT_REMOVEALL,       /// All splits removed.

    // grid events
    GRIDEVENT_SELECTION,        /// Column selection changed.
    GRIDEVENT_COLUMNTYPE        /// Column type changed.
};


// ----------------------------------------------------------------------------

/** Data for a CSV control event. */
class ScCsvEvent
{
private:
    ScCsvEventType              meType;         /// Type of the event.
    sal_Int32                   mnPos;          /// Reference position of the event.
    sal_Int32                   mnOldPos;       /// Old position (i.e. for split move).

public:
    inline                      ScCsvEvent() : meType( CSVEVENT_NONE ),
                                    mnPos( POS_INVALID ), mnOldPos( POS_INVALID ) {}

    inline void                 Set( ScCsvEventType eType, sal_Int32 nPos, sal_Int32 nOldPos );

    inline ScCsvEventType       GetType() const     { return meType; }
    inline sal_Int32            GetPos() const      { return mnPos; }
    inline sal_Int32            GetOldPos() const   { return mnOldPos; }
};

inline void ScCsvEvent::Set( ScCsvEventType eType, sal_Int32 nPos, sal_Int32 nOldPos )
{
    meType = eType;
    mnPos = nPos;
    mnOldPos = nOldPos;
}


// ============================================================================


/** Base class for the CSV ruler and the data grid control. Implements event handling. */
class ScCsvControl : public Control
{
private:
    Link                        maRequestHdl;       /// External request handler.
    Link                        maEventHdl;         /// External event handler.

    ScCsvRequest                maRequest;          /// Data of last request.
    ScCsvEvent                  maEvent;            /// Data of last event.

    const ScCsvLayoutData&      mrData;             /// Shared layout data.

    bool                        mbValidGfx;         /// Content of virtual devices valid?

    // ------------------------------------------------------------------------
public:
                                ScCsvControl( ScCsvControl& rParent );
                                ScCsvControl( Window* pParent, const ScCsvLayoutData& rData, WinBits nStyle = 0 );
                                ScCsvControl( Window* pParent, const ScCsvLayoutData& rData, const ResId& rResId );

    // drawing ----------------------------------------------------------------

    /** Sets the graphic invalid (next Redraw() will not use cached graphic). */
    inline void                 InvalidateGfx() { mbValidGfx = false; }
    /** Sets the graphic valid (next Redraw() will use cached graphic). */
    inline void                 ValidateGfx() { mbValidGfx = true; }
    /** Returns true, if cached graphic is valid. */
    inline bool                 IsValidGfx() const { return mbValidGfx; }

    /** Commits repaint request.
        @param bInvalidate  true = invalidates graphics of this control. */
    void                        Repaint( bool bInvalidate = false );
    /** Increases no-repaint counter (control does not repaint until the last EnableRepaint()). */
    void                        DisableRepaint();
    /** Decreases no-repaint counter and repaints if counter reaches 0.
        @param bInvalidate  true = invalidates graphics of this control. */
    void                        EnableRepaint( bool bInvalidate = false );
    /** Returns true, if control will not repaint. */
    inline bool                 IsNoRepaint() const { return mrData.mnNoRepaint > 0; }

    /** Inverts a rectangle in the specified output device. */
    void                        ImplInvertRect( OutputDevice& rOutDev, const Rectangle& rRect );

    // event handling ---------------------------------------------------------

    /** Sets a new request handler. */
    inline void                 SetRequestHdl( const Link& rHdl ) { maRequestHdl = rHdl; }
    /** Returns the current request handler. */
    inline const Link&          GetRequestHdl() const { return maRequestHdl; }
    /** Returns data of the last request. */
    inline const ScCsvRequest&  GetRequest() const { return maRequest; }
    /** Sets a request with position data and calls request handler. */
    void                        CommitRequest(
                                    ScCsvRequestType eType,
                                    sal_Int32 nData = POS_INVALID );

    /** Sets a new event handler. */
    inline void                 SetEventHdl( const Link& rHdl ) { maEventHdl = rHdl; }
    /** Returns the current event handler. */
    inline const Link&          GetEventHdl() const { return maEventHdl; }
    /** Returns data of the last event. */
    inline const ScCsvEvent&    GetEvent() const { return maEvent; }
    /** Sets the data of the last event and calls event handler. */
    void                        CommitEvent(
                                    ScCsvEventType eType,
                                    sal_Int32 nPos = POS_INVALID,
                                    sal_Int32 nOldPos = POS_INVALID );

    // layout helpers ---------------------------------------------------------

    /** Returns a reference to the current layout data. */
    inline const ScCsvLayoutData& GetLayoutData() const { return mrData; }

    /** Returns the number of available positions. */
    inline sal_Int32            GetPosCount() const { return mrData.mnPosCount; }
    /** Returns the number of visible positions. */
    sal_Int32                   GetVisPosCount() const;
    /** Returns the first visible position. */
    inline sal_Int32            GetFirstVisPos() const { return mrData.mnPosOffset; }
    /** Returns the last visible position. */
    inline sal_Int32            GetLastVisPos() const { return GetFirstVisPos() + GetVisPosCount(); }
    /** Returns highest possible position for first visible character. */
    sal_Int32                   GetMaxPosOffset() const;

    /** Returns true, if it is allowed to set a split at nPos. */
    bool                        IsValidSplitPos( sal_Int32 nPos ) const;
    /** Returns true, if nPos is an allowed AND visible split position. */
    bool                        IsVisibleSplitPos( sal_Int32 nPos ) const;

    /** Returns X coordinate of first visible position. */
    inline sal_Int32            GetOffsetX() const { return mrData.mnOffsetX; }
    /** Returns the width of one character column. */
    inline sal_Int32            GetCharWidth() const { return mrData.mnCharWidth; }
    /** Returns output X coordinate of the specified position. */
    sal_Int32                   GetX( sal_Int32 nPos ) const;
    /** Returns position from output coordinate. */
    sal_Int32                   GetPosFromX( sal_Int32 nX ) const;

    /** Returns the number of data lines. */
    inline sal_Int32            GetLineCount() const { return mrData.mnLineCount; }
    /** Returns the number of visible lines (including partly visible bottom line). */
    sal_Int32                   GetVisLineCount() const;
    /** Returns index of first visible line. */
    inline sal_Int32            GetFirstVisLine() const { return mrData.mnLineOffset; }
    /** Returns index of last visible line. */
    sal_Int32                   GetLastVisLine() const;
    /** Returns highest possible index for first line. */
    sal_Int32                   GetMaxLineOffset() const;

    /** Returns true, if nLine is a valid line index. */
    bool                        IsValidLine( sal_Int32 nLine ) const;
    /** Returns true, if nLine is a valid and visible line index. */
    bool                        IsVisibleLine( sal_Int32 nLine ) const;

    /** Returns Y coordinate of first visible line. */
    inline sal_Int32            GetOffsetY() const { return mrData.mnOffsetY; }
    /** Returns the height of one line. */
    inline sal_Int32            GetLineHeight() const { return mrData.mnLineHeight; }
    /** Returns output Y coordinate of the specified line. */
    sal_Int32                   GetY( sal_Int32 nLine ) const;

    /** Returns the ruler cursor position. */
    inline sal_Int32            GetRulerCursorPos() const { return mrData.mnPosCursor; }
    /** Returns the data grid cursor position (not column index!). */
    inline sal_Int32            GetGridCursorPos() const { return mrData.mnColCursor; }

    // keyboard helpers -------------------------------------------------------

    /** Returns direction code for the keys LEFT, RIGHT, HOME, END.
        @return  bHomeEnd  true = Evaluate HOME and END key. */
    ScMoveMode                  GetHorzDirection( sal_uInt16 nCode, bool bHomeEnd );
    /** Returns direction code for the keys UP, DOWN, HOME, END, PAGE UP, PAGE DOWN.
        @return  bHomeEnd  true = Evaluate HOME and END key. */
    ScMoveMode                  GetVertDirection( sal_uInt16 nCode, bool bHomeEnd );
};


// ============================================================================

#endif

