/*************************************************************************
 *
 *  $RCSfile: csvruler.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2002-07-05 15:42:16 $
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


// ============================================================================

/** A ruler control for the CSV import dialog. Supports setting and moving
    splits (which divide lines of data into several columns). */
class ScCsvRuler : public ScCsvControl
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
                                ScCsvRuler( ScCsvControl& rParent );

    // ruler handling ---------------------------------------------------------

    /** Redraws the entire ruler. */
    void                        ImplRedraw();

    /** Sets position and size of the ruler. The height is calculated internally. */
    virtual void                SetPosSizePixel(
                                    sal_Int32 nX, sal_Int32 nY,
                                    sal_Int32 nWidth, sal_Int32 nHeight,
                                    sal_uInt16 nFlags = WINDOW_POSSIZE_ALL );

    /** Apply current layout data to the ruler. */
    void                        ApplyLayout( const ScCsvLayoutData& rOldData );

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
    /** Inserts a split and commits event. */
    void                        InsertSplit( sal_Int32 nPos );
    /** Removes a split and commits event. */
    void                        RemoveSplit( sal_Int32 nPos );
    /** Inserts a new or removes an existing split and commits event. */
    void                        ToggleSplit( sal_Int32 nPos );
    /** Moves a split from nPos to nNewPos and commits event. */
    void                        MoveSplit( sal_Int32 nPos, sal_Int32 nNewPos );
    /** Moves the split from nPos to the given direction and commits event. */
    void                        MoveSplitRel( sal_Int32 nPos, ScMoveMode eDir );
    /** Removes all splits of the ruler and commits event. */
    void                        RemoveAllSplits();

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

    // initialization ---------------------------------------------------------
private:
    /** Reads colors from system settings. */
    void                        InitColors();
    /** Initializes all data dependent from the control's size. */
    void                        InitSizeData();

    // ruler handling ---------------------------------------------------------

    /** Returns the height the ruler needs to draw itself. */
    sal_Int32                   GetRequiredHeight() const;

    /** Finds next position without a split. */
    sal_Int32                   FindEmptyPos( sal_Int32 nPos, ScMoveMode eDir ) const;

    /** Moves cursor to a new position.
        @param bScroll  TRUE = The method may scroll the ruler. */
    void                        MoveCursor( sal_Int32 nPos, bool bScroll = true );
    /** Moves cursor to the given direction. */
    void                        MoveCursorRel( ScMoveMode eDir );
    /** Sets cursor to an existing split, according to eDir. */
    void                        MoveCursorToSplit( ScMoveMode eDir );
    /** Scrolls data grid vertically. */
    void                        ScrollVertRel( ScMoveMode eDir );

    /** Moves split and cursor to nNewPos and commits event. */
    void                        MoveCurrSplit( sal_Int32 nNewPos );
    /** Moves split and cursor to the given direction and commits event. */
    void                        MoveCurrSplitRel( ScMoveMode eDir );

    /** Starts tracking at the specified position. */
    void                        StartMouseTracking( sal_Int32 nPos );
    /** Moves tracking to a new position. */
    void                        MoveMouseTracking( sal_Int32 nPos );
    /** Applies tracking action for the current tracking position.
        @param bApply  TRUE = apply action, FALSE = cancel action. */
    void                        EndMouseTracking( bool bApply );

    // painting ---------------------------------------------------------------
protected:
    virtual void                Paint( const Rectangle& );

private:
    /** Returns the width of the control. */
    inline sal_Int32            GetWidth() const { return maWinSize.Width(); }
    /** Returns the height of the control. */
    inline sal_Int32            GetHeight() const { return maWinSize.Height(); }

    /** Draws the background and active area to maBackgrDev (only the given X range). */
    void                        ImplDrawArea( sal_Int32 nPosX, sal_Int32 nWidth );
    /** Draws the entire ruler background with scaling to maBackgrDev. */
    void                        ImplDrawBackgrDev();

    /** Draws a split to maRulerDev. */
    void                        ImplDrawSplit( sal_Int32 nPos );
    /** Erases a split from maRulerDev. */
    void                        ImplEraseSplit( sal_Int32 nPos );
    /** Draws the ruler background, all splits and the cursor to maRulerDev. */
    void                        ImplDrawRulerDev();

    /** Draws the cursor bar to the specified position to maRulerDev. */
    void                        ImplDrawCursor( sal_Int32 nPos );
    /** Erases the cursor bar from the specified position from maRulerDev. */
    inline void                 ImplEraseCursor( sal_Int32 nPos ) { ImplDrawCursor( nPos ); }
    /** Draws directly tracking rectangle to the column with the specified index. */
    void                        ImplDrawTrackingRect();

    /** Sets arrow or horizontal split pointer. */
    void                        ImplSetMousePointer( sal_Int32 nPos );
};


// ============================================================================

#endif

