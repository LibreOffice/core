/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xeview.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 12:36:55 $
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

#ifndef SC_XEVIEW_HXX
#define SC_XEVIEW_HXX

#ifndef SC_XLVIEW_HXX
#include "xlview.hxx"
#endif
#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif
#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif

// Workbook view settings records =============================================

/** Represents the WINDOW1 record containing global workbook view settings. */
class XclExpWindow1 : public XclExpRecord
{
public:
    explicit                XclExpWindow1( const XclExpRoot& rRoot );

private:
    /** Writes the contents of the WINDOW1 record. */
    virtual void            WriteBody( XclExpStream& rStrm );

private:
    sal_uInt16              mnFlags;            /// Option flags.
    sal_uInt16              mnTabBarSize;       /// Size of tabbar relative to window width (per mill).
};

// Sheet view settings records ================================================

/** Represents a WINDOW2 record with general view settings for a sheet. */
class XclExpWindow2 : public XclExpRecord
{
public:
    explicit            XclExpWindow2( const XclExpRoot& rRoot,
                            const XclTabViewData& rData, sal_uInt32 nGridColorId );

private:
    /** Writes the contents of the WINDOW2 record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    Color               maGridColor;        /// Grid color (<=BIFF5).
    sal_uInt32          mnGridColorId;      /// Color ID of grid color (>=BIFF8).
    sal_uInt16          mnFlags;            /// Option flags.
    XclAddress          maFirstXclPos;      /// First visible cell.
    sal_uInt16          mnNormalZoom;       /// Zoom factor for normal view.
    sal_uInt16          mnPageZoom;         /// Zoom factor for pagebreak preview.
};

// ----------------------------------------------------------------------------

/** Represents an SCL record for the zoom factor of the current view of a sheet. */
class XclExpScl : public XclExpRecord
{
public:
    explicit            XclExpScl( sal_uInt16 nZoom );

private:
    /** Tries to shorten numerator and denominator by the passed value. */
    void                Shorten( sal_uInt16 nFactor );
    /** Writes the contents of the SCL record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    sal_uInt16          mnNum;              /// Numerator of the zoom factor.
    sal_uInt16          mnDenom;            /// Denominator of the zoom factor.
};

// ----------------------------------------------------------------------------

/** Represents a PANE record containing settings for split/frozen windows. */
class XclExpPane : public XclExpRecord
{
public:
    explicit            XclExpPane( const XclTabViewData& rData );

private:
    /** Writes the contents of the PANE record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    sal_uInt16          mnSplitX;           /// Split X position, or frozen column.
    sal_uInt16          mnSplitY;           /// Split Y position, or frozen row.
    XclAddress          maSecondXclPos;     /// First visible cell in additional panes.
    sal_uInt8           mnActivePane;       /// Active pane (with cell cursor).
};

// ----------------------------------------------------------------------------

/** Represents a SELECTION record with selection data for a pane. */
class XclExpSelection : public XclExpRecord
{
public:
    explicit            XclExpSelection( const XclTabViewData& rData, sal_uInt8 nPane );

private:
    /** Writes the contents of the SELECTION record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    XclSelectionData    maSelData;          /// Selection data.
    sal_uInt8           mnPane;             /// Pane identifier of this selection.
};

// View settings ==============================================================

/** Contains all view settings records for a single sheet. */
class XclExpTabViewSettings : public XclExpRecordBase, protected XclExpRoot
{
public:
    /** Creates all records containing the view settings of the specified sheet. */
    explicit            XclExpTabViewSettings( const XclExpRoot& rRoot, SCTAB nScTab );

    /** Writes all view settings records to the stream. */
    virtual void        Save( XclExpStream& rStrm );

private:
    /** Creates selection data for the specified pane. */
    void                CreateSelectionData( sal_uInt8 nPane,
                            const ScAddress& rCursor, const ScRangeList& rSelection );

    void                WriteWindow2( XclExpStream& rStrm ) const;
    void                WriteScl( XclExpStream& rStrm ) const;
    void                WritePane( XclExpStream& rStrm ) const;
    void                WriteSelection( XclExpStream& rStrm, sal_uInt8 nPane ) const;

private:
    XclTabViewData      maData;             /// All view settings for a sheet.
    sal_uInt32          mnGridColorId;      /// Color identifier for grid color.
};

// ============================================================================

#endif

