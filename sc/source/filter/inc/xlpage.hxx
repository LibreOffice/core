/*************************************************************************
 *
 *  $RCSfile: xlpage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:43:08 $
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

#ifndef SC_XLPAGE_HXX
#define SC_XLPAGE_HXX

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif


// Constants and Enumerations =================================================

// (0x0014, 0x0015) HEADER, FOOTER --------------------------------------------

const sal_uInt16 EXC_ID_HEADER              = 0x0014;
const sal_uInt16 EXC_ID_FOOTER              = 0x0015;


// (0x001A, 0x001B) VERTICAL-, HORIZONTALPAGEBREAKS ---------------------------

const sal_uInt16 EXC_ID_VERPAGEBREAKS       = 0x001A;
const sal_uInt16 EXC_ID_HORPAGEBREAKS       = 0x001B;

/** Orientation for page breaks. */
enum XclPBOrientation
{
    xlPBHorizontal,
    xlPBVertical
};


// (0x0026, 0x0027, 0x0028, 0x0029) LEFT-, RIGHT-, TOP-, BOTTOMMARGIN ---------

const sal_uInt16 EXC_ID_LEFTMARGIN          = 0x0026;
const sal_uInt16 EXC_ID_RIGHTMARGIN         = 0x0027;
const sal_uInt16 EXC_ID_TOPMARGIN           = 0x0028;
const sal_uInt16 EXC_ID_BOTTOMMARGIN        = 0x0029;


// (0x002A, 0x002B) PRINTHEADERS, PRINTGRIDLINES ------------------------------

const sal_uInt16 EXC_ID_PRINTHEADERS        = 0x002A;
const sal_uInt16 EXC_ID_PRINTGRIDLINES      = 0x002B;


// (0x0082, 0x0083, 0x0084) GRIDSET, HCENTER, VCENTER -------------------------

const sal_uInt16 EXC_ID_GRIDSET             = 0x0082;
const sal_uInt16 EXC_ID_HCENTER             = 0x0083;
const sal_uInt16 EXC_ID_VCENTER             = 0x0084;


// (0x00A1) SETUP -------------------------------------------------------------

const sal_uInt16 EXC_ID_SETUP               = 0x00A1;

const sal_uInt16 EXC_SETUP_INROWS           = 0x0001;
const sal_uInt16 EXC_SETUP_PORTRAIT         = 0x0002;
const sal_uInt16 EXC_SETUP_INVALID          = 0x0004;
const sal_uInt16 EXC_SETUP_BLACKWHITE       = 0x0008;
const sal_uInt16 EXC_SETUP_DRAFT            = 0x0010;
const sal_uInt16 EXC_SETUP_PRINTNOTES       = 0x0020;
const sal_uInt16 EXC_SETUP_STARTPAGE        = 0x0080;
const sal_uInt16 EXC_SETUP_NOTES_END        = 0x0200;

const sal_uInt16 EXC_PAPERSIZE_DEFAULT      = 0;


// (0x00E9) BITMAP ------------------------------------------------------------

const sal_uInt16 EXC_ID_BITMAP              = 0x00E9;
const sal_uInt32 EXC_BITMAP_UNKNOWNID       = 0x00010009;
const sal_uInt32 EXC_BITMAP_MAXREC          = 0x201C;
const sal_uInt32 EXC_BITMAP_MAXCONT         = 0x2014;


// ============================================================================

// Page settings ==============================================================

class SvxBrushItem;
class SfxPrinter;

/** Contains all page (print) settings for a single sheet. */
struct XclPageData : ScfNoCopy
{
    typedef ::std::auto_ptr< SvxBrushItem > SvxBrushItemPtr;

    ScfUInt16Vec                maHorPageBreaks;    /// Horizontal page breaks.
    ScfUInt16Vec                maVerPageBreaks;    /// Vertical page breaks.
    SvxBrushItemPtr             mpBrushItem;        /// Background bitmap.
    String                      maHeader;           /// Excel header string (empty = off).
    String                      maFooter;           /// Excel footer string (empty = off).
    double                      mfLeftMargin;       /// Left margin in inches.
    double                      mfRightMargin;      /// Right margin in inches.
    double                      mfTopMargin;        /// Top margin in inches.
    double                      mfBottomMargin;     /// Bottom margin in inches.
    double                      mfHeaderMargin;     /// Margin main page to header.
    double                      mfFooterMargin;     /// Margin main page to footer.
    sal_uInt16                  mnPaperSize;        /// Index into paper size table.
    sal_uInt16                  mnCopies;           /// Number of copies.
    sal_uInt16                  mnStartPage;        /// Start page number.
    sal_uInt16                  mnScaling;          /// Scaling in percent.
    sal_uInt16                  mnFitToWidth;       /// Fit to number of pages in width.
    sal_uInt16                  mnFitToHeight;      /// Fit to number of pages in height.
    sal_uInt16                  mnHorPrintRes;      /// Horizontal printing resolution.
    sal_uInt16                  mnVerPrintRes;      /// Vertical printing resolution.
    bool                        mbValid;            /// false = some of the values are not valid.
    bool                        mbPortrait;         /// true = portrait; false = landscape.
    bool                        mbPrintInRows;      /// true = in rows; false = in columns.
    bool                        mbBlackWhite;       /// true = black/white; false = colors.
    bool                        mbDraftQuality;     /// true = draft; false = default quality.
    bool                        mbPrintNotes;       /// true = print notes.
    bool                        mbManualStart;      /// true = mnStartPage valid; false = automatic.
    bool                        mbFitToPages;       /// true = fit to pages; false = scale in percent.
    bool                        mbHorCenter;        /// true = centered horizontally; false = left aligned.
    bool                        mbVerCenter;        /// true = centered vertically; false = top aligned.
    bool                        mbPrintHeadings;    /// true = print column and row headings.
    bool                        mbPrintGrid;        /// true = print grid lines.

    explicit                    XclPageData();
                                ~XclPageData();

    /** Sets Excel default page settings. */
    void                        SetDefaults();

    /** Returns the real paper size (twips) from the paper size index and paper orientation. */
    Size                        GetScPaperSize( SfxPrinter* pPrinter ) const;
    /** Sets the Excel paper size index and paper orientation from Calc paper size (twips). */
    void                        SetScPaperSize( const Size& rSize, bool bPortrait );
};


// ============================================================================

#endif

