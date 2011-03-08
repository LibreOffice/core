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

#ifndef SC_XIPAGE_HXX
#define SC_XIPAGE_HXX

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
    inline const XclPageData& GetPageData() const { return maData; }

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
    inline void         SetFitToPages( bool bFitToPages ) { maData.mbFitToPages = bFitToPages; }

    /** Creates a page stylesheet from current settings and sets it at current sheet. */
    void                Finalize();

private:
    XclPageData         maData;         /// Page settings data.
    bool                mbValidPaper;   /// true = Paper size and orientation valid.
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
