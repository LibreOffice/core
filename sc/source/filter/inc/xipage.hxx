/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

