/*************************************************************************
 *
 *  $RCSfile: csvtablebox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2002-07-11 15:38:28 $
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

#ifndef _SC_CSVTABLEBOX_HXX
#define _SC_CSVTABLEBOX_HXX

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

#ifndef _SC_CSVCONTROL_HXX
#include "csvcontrol.hxx"
#endif
#ifndef _SC_CSVRULER_HXX
#include "csvruler.hxx"
#endif
#ifndef _SC_CSVGRID_HXX
#include "csvgrid.hxx"
#endif


// ----------------------------------------------------------------------------

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
class ScCsvTableBox : public ScCsvControl
{
private:
    ScCsvLayoutData             maData;             /// Current layout data of the controls.

    ScCsvRuler                  maRuler;            /// The ruler for fixed width mode.
    ScCsvGrid                   maGrid;             /// Calc-like data table for fixed width mode.
    ScrollBar                   maHScroll;          /// Horizontal scroll bar.
    ScrollBar                   maVScroll;          /// Vertical scroll bar.
    ScrollBarBox                maScrollBox;        /// For the bottom right edge.

    Link                        maUpdateTextHdl;    /// Updates all cell texts.
    Link                        maColSelectHdl;     /// Handler for changed column selection.

    ScCsvColTypeVec             maFixColTypes;      /// Column data types in fixed width mode.
    ScCsvColTypeVec             maSepColTypes;      /// Column data types in separators mode.

    sal_Int32                   mnSelColType;       /// Cached data type of selected columns.
    sal_Int32                   mnFixedWidth;       /// Cached total width for fixed width mode.

    bool                        mbFixedMode;        /// false = Separators, true = Fixed width.

    // ------------------------------------------------------------------------
public:
                                ScCsvTableBox( Window* pParent );
                                ScCsvTableBox( Window* pParent, const ResId& rResId );

    // initialization ---------------------------------------------------------

    /** Reads UI strings for data types from the list box. */
    void                        InitTypes( const ListBox& rListBox );

    /** Sets a new handler for "update cell texts" requests. */
    inline void                 SetUpdateTextHdl( const Link& rHdl ) { maUpdateTextHdl = rHdl; }
    /** Returns the handler for "update cell texts" requests. */
    inline const Link&          GetUpdateTextHdl() const { return maUpdateTextHdl; }
    /** Sets a new handler for "column selection changed" events. */
    inline void                 SetColSelectHdl( const Link& rHdl ) { maColSelectHdl = rHdl; }
    /** Returns the handler for "column selection changed" events. */
    inline const Link&          GetColSelectHdl() const { return maColSelectHdl; }

    // control handling -------------------------------------------------------

    /** Sets the control to separators mode. */
    void                        SetSeparatorsMode();
    /** Sets the control to fixed width mode. */
    void                        SetFixedWidthMode();

    /** Returns the data type of the selected columns (or -1, if different types are selected). */
    inline sal_Int32            GetSelColumnType() const { return mnSelColType; }

    /** Fills all cells of all lines with the passed texts (Unicode strings). */
    void                        SetUniStrings(
                                    const String* pTextLines, const String& rSepChars,
                                    sal_Unicode cTextSep, bool bMergeSep );
    /** Fills all cells of all lines with the passed texts (ByteStrings). */
    void                        SetByteStrings(
                                    const ByteString* pLineTexts, CharSet eCharSet,
                                    const String& rSepChars, sal_Unicode cTextSep, bool bMergeSep );

    /** Fills the options object with current column data. */
    void                        FillColumnData( ScAsciiOptions& rOptions ) const;

    // event handling ---------------------------------------------------------
protected:
    virtual void                Resize();
    virtual void                DataChanged( const DataChangedEvent& rDCEvt );

    // initialization ---------------------------------------------------------
private:
    /** Initialisation on construction. */
    void                        Init();
    /** Initializes the children controls (pos/size, scroll bars, ...). */
    void                        InitControls();
    /** Initializes size and position data of horizontal scrollbar. */
    void                        InitHScrollBar();
    /** Initializes size and position data of vertical scrollbar. */
    void                        InitVScrollBar();

    // control handling -------------------------------------------------------

    /** Calculates and sets valid position offset nearest to nPos. */
    inline void                 ImplSetPosOffset( sal_Int32 nPos )
                                    { maData.mnPosOffset = Max( Min( nPos, GetMaxPosOffset() ), 0L ); }
    /** Calculates and sets valid line offset nearest to nLine. */
    inline void                 ImplSetLineOffset( sal_Int32 nLine )
                                    { maData.mnLineOffset = Max( Min( nLine, GetMaxLineOffset() ), 0L ); }
    /** Moves controls (not cursors!) so that nPos becomes visible. */
    void                        MakePosVisible( sal_Int32 nPos );

    // event handling ---------------------------------------------------------

                                DECL_LINK( CsvEventHdl, ScCsvControl* );
                                DECL_LINK( CsvRequestHdl, ScCsvControl* );

                                DECL_LINK( ScrollHdl, ScrollBar* );
                                DECL_LINK( ScrollEndHdl, ScrollBar* );
};


// ============================================================================

#endif

