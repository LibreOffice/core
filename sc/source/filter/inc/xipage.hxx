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

#include "xlpage.hxx"
#include "xiroot.hxx"

// Page settings ==============================================================

/** Contains all page (print) settings for a single sheet.
    @descr  Supports reading all related records and creating a page style sheet. */
class XclImpPageSettings : protected XclImpRoot
{
public:
    explicit            XclImpPageSettings( const XclImpRoot& rRoot );

    /** Returns read-only access to the page data. */
    const XclPageData& GetPageData() const { return maData; }

    /** Initializes the object to be used for a new sheet. */
    void                Initialize();

    /** Reads a SETUP record and inserts contained data. */
    void                ReadSetup( XclImpStream& rStrm );
    /** Reads a ***MARGIN record (reads all 4 margin records). */
    void                ReadMargin( XclImpStream& rStrm );
    /** Reads a HCENTER or VCENTER record. */
    void                ReadCenter( XclImpStream& rStrm );
    /** Reads a HEADER or FOOTER record. */
    void                ReadHeaderFooter( XclImpStream& rStrm );
    /** Reads a HORIZONTALPAGEBREAKS or VERTICALPAGEBREAKS record. */
    void                ReadPageBreaks( XclImpStream& rStrm );
    /** Reads a PRINTHEADERS record. */
    void                ReadPrintHeaders( XclImpStream& rStrm );
    /** Reads a PRINTGRIDLINES record. */
    void                ReadPrintGridLines( XclImpStream& rStrm );
    /** Reads an IMGDATA record and creates the SvxBrushItem. */
    void                ReadImgData( XclImpStream& rStrm );

    /** Overrides paper size and orientation (used in sheet-charts). */
    void                SetPaperSize( sal_uInt16 nXclPaperSize, bool bPortrait );
    /** Sets or clears the fit-to-pages setting (contained in WSBOOL record). */
    void         SetFitToPages( bool bFitToPages ) { maData.mbFitToPages = bFitToPages; }

    /** Creates a page stylesheet from current settings and sets it at current sheet. */
    void                Finalize();

private:
    XclPageData         maData;         /// Page settings data.
    bool                mbValidPaper;   /// true = Paper size and orientation valid.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
