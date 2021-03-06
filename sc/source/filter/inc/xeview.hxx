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

#pragma once

#include "xerecord.hxx"
#include "xlview.hxx"
#include "xeroot.hxx"

// Workbook view settings records =============================================

/** Represents the WINDOW1 record containing global workbook view settings. */
class XclExpWindow1 : public XclExpRecord
{
public:
    explicit                XclExpWindow1( const XclExpRoot& rRoot );

    virtual void            SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Writes the contents of the WINDOW1 record. */
    virtual void            WriteBody( XclExpStream& rStrm ) override;

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
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    Color               maGridColor;        /// Grid color (<=BIFF5).
    sal_uInt32          mnGridColorId;      /// Color ID of grid color (>=BIFF8).
    sal_uInt16          mnFlags;            /// Option flags.
    XclAddress          maFirstXclPos;      /// First visible cell.
    sal_uInt16          mnNormalZoom;       /// Zoom factor for normal view.
    sal_uInt16          mnPageZoom;         /// Zoom factor for pagebreak preview.
};

/** Represents an SCL record for the zoom factor of the current view of a sheet. */
class XclExpScl : public XclExpRecord
{
public:
    explicit            XclExpScl( sal_uInt16 nZoom );

private:
    /** Tries to shorten numerator and denominator by the passed value. */
    void                Shorten( sal_uInt16 nFactor );
    /** Writes the contents of the SCL record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    sal_uInt16          mnNum;              /// Numerator of the zoom factor.
    sal_uInt16          mnDenom;            /// Denominator of the zoom factor.
};

/** Represents a PANE record containing settings for split/frozen windows. */
class XclExpPane : public XclExpRecord
{
public:
    explicit            XclExpPane( const XclTabViewData& rData );

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Writes the contents of the PANE record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    sal_uInt16          mnSplitX;           /// Split X position, or frozen column.
    sal_uInt32          mnSplitY;           /// Split Y position, or frozen row.
    XclAddress          maSecondXclPos;     /// First visible cell in additional panes.
    sal_uInt8           mnActivePane;       /// Active pane (with cell cursor).
    bool                mbFrozenPanes;      /// true = "frozen" panes; false = "split" window.
};

/** Represents a SELECTION record with selection data for a pane. */
class XclExpSelection : public XclExpRecord
{
public:
    explicit            XclExpSelection( const XclTabViewData& rData, sal_uInt8 nPane );

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
private:
    /** Writes the contents of the SELECTION record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

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
    virtual void        WriteBody( XclExpStream& rStrm ) override;

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
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

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
    bool                mbHasTabSettings;   /// It's false for embedded OLE spreadsheets.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
