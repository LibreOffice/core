/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xipage.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:33:27 $
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
#ifndef SC_XIPAGE_HXX
#define SC_XIPAGE_HXX

#ifndef SC_XLPAGE_HXX
#include "xlpage.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

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
    /** Reads a BITMAP record and creates the SvxBrushItem. */
    void                ReadBitmap( XclImpStream& rStrm );

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

