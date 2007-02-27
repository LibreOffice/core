/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: csvruler.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:21:10 $
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

// ============================================================================

#ifndef _SC_CSVRULER_HXX
#define _SC_CSVRULER_HXX

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _SC_CSVCONTROL_HXX
#include "csvcontrol.hxx"
#endif
#ifndef _SC_CSVSPLITS_HXX
#include "csvsplits.hxx"
#endif

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

class ScAccessibleCsvControl;


// ============================================================================

/** A ruler control for the CSV import dialog. Supports setting and moving
    splits (which divide lines of data into several columns). */
class SC_DLLPUBLIC ScCsvRuler : public ScCsvControl
{
private:
    VirtualDevice               maBackgrDev;        /// Ruler background, scaling.
    VirtualDevice               maRulerDev;         /// Ruler with splits and cursor.

    Color                       maBackColor;        /// Background color.
    Color                       maActiveColor;      /// Color for active part of ruler.
    Color                       maTextColor;        /// Text and scale color.
    Color                       maSplitColor;       /// Split area color.

    ScCsvSplits                 maSplits;           /// Vector with split positions.
    ScCsvSplits                 maOldSplits;        /// Old state for cancellation.

    sal_Int32                   mnPosCursorLast;    /// Last valid position of cursor.
    sal_Int32                   mnPosMTStart;       /// Start position of mouse tracking.
    sal_Int32                   mnPosMTCurr;        /// Current position of mouse tracking.
    bool                        mbPosMTMoved;       /// Tracking: Anytime moved to another position?

    Size                        maWinSize;          /// Size of the control.
    Rectangle                   maActiveRect;       /// The active area of the ruler.
    sal_Int32                   mnSplitSize;        /// Size of a split circle.

    // ------------------------------------------------------------------------
public:
    explicit                    ScCsvRuler( ScCsvControl& rParent );

    // common ruler handling --------------------------------------------------
public:
    using Window::SetPosSizePixel;
    /** Sets position and size of the ruler. The height is calculated internally. */
    virtual void                SetPosSizePixel(
                                    sal_Int32 nX, sal_Int32 nY,
                                    sal_Int32 nWidth, sal_Int32 nHeight,
                                    sal_uInt16 nFlags = WINDOW_POSSIZE_ALL );

    /** Apply current layout data to the ruler. */
    void                        ApplyLayout( const ScCsvLayoutData& rOldData );

private:
    /** Reads colors from system settings. */
    SC_DLLPRIVATE void                        InitColors();
    /** Initializes all data dependent from the control's size. */
    SC_DLLPRIVATE void                        InitSizeData();

    /** Moves cursor to a new position.
        @param bScroll  TRUE = The method may scroll the ruler. */
    SC_DLLPRIVATE void                        MoveCursor( sal_Int32 nPos, bool bScroll = true );
    /** Moves cursor to the given direction. */
    SC_DLLPRIVATE void                        MoveCursorRel( ScMoveMode eDir );
    /** Sets cursor to an existing split, according to eDir. */
    SC_DLLPRIVATE void                        MoveCursorToSplit( ScMoveMode eDir );
    /** Scrolls data grid vertically. */
    SC_DLLPRIVATE void                        ScrollVertRel( ScMoveMode eDir );

    // split handling ---------------------------------------------------------
public:
    /** Returns the split array. */
    inline const ScCsvSplits&   GetSplits() const { return maSplits; }
    /** Returns the number of splits. */
    inline sal_uInt32           GetSplitCount() const
                                    { return maSplits.Count(); }
    /** Returns the position of the specified split. */
    inline sal_Int32            GetSplitPos( sal_uInt32 nIndex ) const
                                    { return maSplits[ nIndex ]; }
    /** Finds a position nearest to nPos which does not cause scrolling the visible area. */
    sal_Int32                   GetNoScrollPos( sal_Int32 nPos ) const;

    /** Returns true if at position nPos is a split. */
    inline bool                 HasSplit( sal_Int32 nPos ) const { return maSplits.HasSplit( nPos ); }
    /** Inserts a split. */
    void                        InsertSplit( sal_Int32 nPos );
    /** Removes a split. */
    void                        RemoveSplit( sal_Int32 nPos );
    /** Moves a split from nPos to nNewPos. */
    void                        MoveSplit( sal_Int32 nPos, sal_Int32 nNewPos );
    /** Removes all splits of the ruler. */
    void                        RemoveAllSplits();

private:
    /** Finds next position without a split. */
    SC_DLLPRIVATE sal_Int32                   FindEmptyPos( sal_Int32 nPos, ScMoveMode eDir ) const;

    /** Moves split and cursor to nNewPos and commits event. */
    SC_DLLPRIVATE void                        MoveCurrSplit( sal_Int32 nNewPos );
    /** Moves split and cursor to the given direction and commits event. */
    SC_DLLPRIVATE void                        MoveCurrSplitRel( ScMoveMode eDir );

    // event handling ---------------------------------------------------------
protected:
    virtual void                Resize();
    virtual void                GetFocus();
    virtual void                LoseFocus();
    virtual void                DataChanged( const DataChangedEvent& rDCEvt );

    virtual void                MouseButtonDown( const MouseEvent& rMEvt );
    virtual void                MouseMove( const MouseEvent& rMEvt );
    virtual void                Tracking( const TrackingEvent& rTEvt );

    virtual void                KeyInput( const KeyEvent& rKEvt );

private:
    /** Starts tracking at the specified position. */
    SC_DLLPRIVATE void                        StartMouseTracking( sal_Int32 nPos );
    /** Moves tracking to a new position. */
    SC_DLLPRIVATE void                        MoveMouseTracking( sal_Int32 nPos );
    /** Applies tracking action for the current tracking position.
        @param bApply  TRUE = apply action, FALSE = cancel action. */
    SC_DLLPRIVATE void                        EndMouseTracking( bool bApply );

    // painting ---------------------------------------------------------------
protected:
    virtual void                Paint( const Rectangle& );

public:
    /** Redraws the entire ruler. */
    void                        ImplRedraw();

private:
    /** Returns the width of the control. */
    inline sal_Int32            GetWidth() const { return maWinSize.Width(); }
    /** Returns the height of the control. */
    inline sal_Int32            GetHeight() const { return maWinSize.Height(); }

    /** Draws the background and active area to maBackgrDev (only the given X range). */
    SC_DLLPRIVATE void                        ImplDrawArea( sal_Int32 nPosX, sal_Int32 nWidth );
    /** Draws the entire ruler background with scaling to maBackgrDev. */
    SC_DLLPRIVATE void                        ImplDrawBackgrDev();

    /** Draws a split to maRulerDev. */
    SC_DLLPRIVATE void                        ImplDrawSplit( sal_Int32 nPos );
    /** Erases a split from maRulerDev. */
    SC_DLLPRIVATE void                        ImplEraseSplit( sal_Int32 nPos );
    /** Draws the ruler background, all splits and the cursor to maRulerDev. */
    SC_DLLPRIVATE void                        ImplDrawRulerDev();

    /** Inverts the cursor bar at the specified position in maRulerDev. */
    SC_DLLPRIVATE void                        ImplInvertCursor( sal_Int32 nPos );
    /** Draws directly tracking rectangle to the column with the specified index. */
    SC_DLLPRIVATE void                        ImplDrawTrackingRect();

    /** Sets arrow or horizontal split pointer. */
    SC_DLLPRIVATE void                        ImplSetMousePointer( sal_Int32 nPos );

    // accessibility ----------------------------------------------------------
protected:
    /** Creates a new accessible object. */
    virtual ScAccessibleCsvControl* ImplCreateAccessible();
};


// ============================================================================

#endif

