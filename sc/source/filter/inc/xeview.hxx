/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SC_XEVIEW_HXX
#define SC_XEVIEW_HXX

#include "xerecord.hxx"
#include "xlview.hxx"
#include "xeroot.hxx"

// Workbook view settings records =============================================

/** Represents the WINDOW1 record containing global workbook view settings. */
class XclExpWindow1 : public XclExpRecord
{
public:
    explicit                XclExpWindow1( const XclExpRoot& rRoot );

    virtual void            SaveXml( XclExpXmlStream& rStrm );

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

    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    /** Writes the contents of the PANE record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    sal_uInt16          mnSplitX;           /// Split X position, or frozen column.
    sal_uInt32          mnSplitY;           /// Split Y position, or frozen row.
    XclAddress          maSecondXclPos;     /// First visible cell in additional panes.
    sal_uInt8           mnActivePane;       /// Active pane (with cell cursor).
};

// ----------------------------------------------------------------------------

/** Represents a SELECTION record with selection data for a pane. */
class XclExpSelection : public XclExpRecord
{
public:
    explicit            XclExpSelection( const XclTabViewData& rData, sal_uInt8 nPane );

    virtual void        SaveXml( XclExpXmlStream& rStrm );
private:
    /** Writes the contents of the SELECTION record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    XclSelectionData    maSelData;          /// Selection data.
    sal_uInt8           mnPane;             /// Pane identifier of this selection.
};

class XclExpTabBgColor : public XclExpRecord
{
public:
    explicit            XclExpTabBgColor( const XclTabViewData& rTabViewData );

    /* virtual void        SaveXml( XclExpXmlStream& rStrm ); TODO Fix XML Saving Stream */
private:
    /** Writes the contents of the SHEETEXT record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    const XclTabViewData&  mrTabViewData;             /// view settings data of current sheet.
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
    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    /** Creates selection data for the specified pane. */
    void                CreateSelectionData( sal_uInt8 nPane,
                            const ScAddress& rCursor, const ScRangeList& rSelection );

    void                WriteWindow2( XclExpStream& rStrm ) const;
    void                WriteScl( XclExpStream& rStrm ) const;
    void                WritePane( XclExpStream& rStrm ) const;
    void                WriteSelection( XclExpStream& rStrm, sal_uInt8 nPane ) const;
    void                WriteTabBgColor( XclExpStream& rStrm ) const;

private:
    XclTabViewData      maData;             /// All view settings for a sheet.
    sal_uInt32          mnGridColorId;      /// Color identifier for grid color.
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
