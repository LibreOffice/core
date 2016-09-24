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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XEPAGE_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XEPAGE_HXX

#include "xerecord.hxx"
#include "xlpage.hxx"
#include "xeroot.hxx"

class XclExpImgData;

// Page settings records ======================================================

// Header/footer --------------------------------------------------------------

/** Represents a HEADER or FOOTER record. */
class XclExpHeaderFooter : public XclExpRecord
{
public:
    explicit            XclExpHeaderFooter( sal_uInt16 nRecId, const OUString& rHdrString );

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
private:
    /** Writes the header or footer string. Writes an empty record, if no header/footer present. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    OUString            maHdrString;        /// Header or footer contents.
};

// General page settings ------------------------------------------------------

/** Represents a SETUP record that contains common page settings. */
class XclExpSetup : public XclExpRecord
{
public:
    explicit            XclExpSetup( const XclPageData& rPageData );

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
private:
    /** Writes the contents of the SETUP record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

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
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Writes the page break list. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

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
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

    XclExpImgData* getGraphicExport();

private:
    XclPageData         maData;         /// Page settings data.
};

/** Contains all page (print) settings records for a chart object. */
class XclExpChartPageSettings : public XclExpRecordBase, protected XclExpRoot
{
public:
    /** Creates all records containing the current page settings. */
    explicit            XclExpChartPageSettings( const XclExpRoot& rRoot );

    /** Writes all page settings records to the stream. */
    virtual void        Save( XclExpStream& rStrm ) override;

private:
    XclPageData         maData;         /// Page settings data.
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
