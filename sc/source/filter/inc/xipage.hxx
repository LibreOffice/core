/*************************************************************************
 *
 *  $RCSfile: xipage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:42:04 $
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
    explicit                    XclImpPageSettings( const XclImpRoot& rRoot );

    /** Returns read-only access to the page data. */
    inline const XclPageData&   GetPageData() const { return maData; }

    /** Reads a SETUP record and inserts contained data. */
    void                        ReadSetup( XclImpStream& rStrm );
    /** Reads a ***MARGIN record (reads all 4 margin records). */
    void                        ReadMargin( XclImpStream& rStrm );
    /** Reads a HCENTER or VCENTER record. */
    void                        ReadCenter( XclImpStream& rStrm );
    /** Reads a HEADER or FOOTER record. */
    void                        ReadHeaderFooter( XclImpStream& rStrm );
    /** Reads a HORIZONTALPAGEBREAKS or VERTICALPAGEBREAKS record. */
    void                        ReadPageBreaks( XclImpStream& rStrm );
    /** Reads a PRINTHEADERS record. */
    void                        ReadPrintheaders( XclImpStream& rStrm );
    /** Reads a PRINTGRIDLINES record. */
    void                        ReadPrintgridlines( XclImpStream& rStrm );
    /** Reads a BITMAP record and creates the SvxBrushItem. */
    void                        ReadBitmap( XclImpStream& rStrm );

    /** Overrides paper size and orientation (used in sheet-charts). */
    void                        SetPaperSize( sal_uInt16 nXclPaperSize, bool bPortrait );
    /** Sets or clears the fit-to-pages setting (contained in WSBOOL record). */
    inline void                 SetFitToPages( bool bFitToPages ) { maData.mbFitToPages = bFitToPages; }

    /** Creates a page stylesheet from current settings and sets it at current sheet. */
    void                        CreatePageStyle();

private:
    XclPageData                 maData;         /// Page settings data.
    bool                        mbValidPaper;   /// true = Paper size and orientation valid.
};


// ============================================================================

#endif

