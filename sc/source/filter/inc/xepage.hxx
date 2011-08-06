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

#ifndef SC_XEPAGE_HXX
#define SC_XEPAGE_HXX

#include "xerecord.hxx"
#include "xlpage.hxx"
#include "xeroot.hxx"

// Page settings records ======================================================

// Header/footer --------------------------------------------------------------

/** Represents a HEADER or FOOTER record. */
class XclExpHeaderFooter : public XclExpRecord
{
public:
    explicit            XclExpHeaderFooter( sal_uInt16 nRecId, const String& rHdrString );

    virtual void        SaveXml( XclExpXmlStream& rStrm );
private:
    /** Writes the header or footer string. Writes an empty record, if no header/footer present. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    String              maHdrString;        /// Header or footer contents.
};

// General page settings ------------------------------------------------------

/** Represents a SETUP record that contains common page settings. */
class XclExpSetup : public XclExpRecord
{
public:
    explicit            XclExpSetup( const XclPageData& rPageData );

    virtual void        SaveXml( XclExpXmlStream& rStrm );
private:
    /** Writes the contents of the SETUP record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    const XclPageData&  mrData;             /// Page settings data of current sheet.
};

// Manual page breaks ---------------------------------------------------------

/** Stores an array of manual page breaks for columns or rows. */
class XclExpPageBreaks : public XclExpRecord
{
public:
    explicit            XclExpPageBreaks(
                            sal_uInt16 nRecId,
                            const ScfUInt16Vec& rPageBreaks,
                            sal_uInt16 nMaxPos );

    /** Writes the record, if the list is not empty. */
    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    /** Writes the page break list. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    const ScfUInt16Vec& mrPageBreaks;       /// Page settings data of current sheet.
    sal_uInt16          mnMaxPos;           /// Maximum row/column for BIFF8 page breaks.
};

// Page settings ==============================================================

/** Contains all page (print) settings records for a single sheet. */
class XclExpPageSettings : public XclExpRecordBase, protected XclExpRoot
{
public:
    /** Creates all records containing the current page settings. */
    explicit            XclExpPageSettings( const XclExpRoot& rRoot );

    /** Returns read-only access to the page data. */
    inline const XclPageData& GetPageData() const { return maData; }

    /** Writes all page settings records to the stream. */
    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    XclPageData         maData;         /// Page settings data.
};

// ----------------------------------------------------------------------------

/** Contains all page (print) settings records for a chart object. */
class XclExpChartPageSettings : public XclExpRecordBase, protected XclExpRoot
{
public:
    /** Creates all records containing the current page settings. */
    explicit            XclExpChartPageSettings( const XclExpRoot& rRoot );

    /** Returns read-only access to the page data. */
    inline const XclPageData& GetPageData() const { return maData; }

    /** Writes all page settings records to the stream. */
    virtual void        Save( XclExpStream& rStrm );

private:
    XclPageData         maData;         /// Page settings data.
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
