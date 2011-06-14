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

#ifndef SC_XLPAGE_HXX
#define SC_XLPAGE_HXX

#include <tools/gen.hxx>
#include <boost/noncopyable.hpp>
#include "xltools.hxx"

// Constants and Enumerations =================================================

// (0x0014, 0x0015) HEADER, FOOTER --------------------------------------------

const sal_uInt16 EXC_ID_HEADER              = 0x0014;
const sal_uInt16 EXC_ID_FOOTER              = 0x0015;

// (0x001A, 0x001B) VERTICAL-, HORIZONTALPAGEBREAKS ---------------------------

const sal_uInt16 EXC_ID_VERPAGEBREAKS       = 0x001A;
const sal_uInt16 EXC_ID_HORPAGEBREAKS       = 0x001B;

// (0x0026, 0x0027, 0x0028, 0x0029) LEFT-, RIGHT-, TOP-, BOTTOMMARGIN ---------

const sal_uInt16 EXC_ID_LEFTMARGIN          = 0x0026;
const sal_uInt16 EXC_ID_RIGHTMARGIN         = 0x0027;
const sal_uInt16 EXC_ID_TOPMARGIN           = 0x0028;
const sal_uInt16 EXC_ID_BOTTOMMARGIN        = 0x0029;

const sal_Int32 EXC_MARGIN_DEFAULT_LR       = 1900;     /// Left/right default margin in 1/100mm.
const sal_Int32 EXC_MARGIN_DEFAULT_TB       = 2500;     /// Top/bottom default margin in 1/100mm.
const sal_Int32 EXC_MARGIN_DEFAULT_HF       = 1300;     /// Header/footer default margin in 1/100mm.
const sal_Int32 EXC_MARGIN_DEFAULT_HLR      = 1900;     /// Left/right header default margin in 1/100mm.
const sal_Int32 EXC_MARGIN_DEFAULT_FLR      = 1900;     /// Left/right footer default margin in 1/100mm.

// (0x002A, 0x002B) PRINTHEADERS, PRINTGRIDLINES ------------------------------

const sal_uInt16 EXC_ID_PRINTHEADERS        = 0x002A;
const sal_uInt16 EXC_ID_PRINTGRIDLINES      = 0x002B;

// (0x0033) PRINTSIZE ---------------------------------------------------------

const sal_uInt16 EXC_ID_PRINTSIZE           = 0x0033;

const sal_uInt16 EXC_PRINTSIZE_SCREEN       = 1;
const sal_uInt16 EXC_PRINTSIZE_PAGE         = 2;
const sal_uInt16 EXC_PRINTSIZE_FULL         = 3;

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
const sal_uInt16 EXC_PAPERSIZE_USER         = 0xFFFF;

// ============================================================================

// Page settings ==============================================================

class SvxBrushItem;
class SfxPrinter;

/** Contains all page (print) settings for a single sheet. */
struct XclPageData : private boost::noncopyable
{
    typedef ::std::auto_ptr< SvxBrushItem > SvxBrushItemPtr;

    ScfUInt16Vec        maHorPageBreaks;    /// Horizontal page breaks.
    ScfUInt16Vec        maVerPageBreaks;    /// Vertical page breaks.
    SvxBrushItemPtr     mxBrushItem;        /// Background bitmap.
    String              maHeader;           /// Excel header string (empty = off).
    String              maFooter;           /// Excel footer string (empty = off).
    double              mfLeftMargin;       /// Left margin in inches.
    double              mfRightMargin;      /// Right margin in inches.
    double              mfTopMargin;        /// Top margin in inches.
    double              mfBottomMargin;     /// Bottom margin in inches.
    double              mfHeaderMargin;     /// Margin main page to header.
    double              mfFooterMargin;     /// Margin main page to footer.
    double              mfHdrLeftMargin;    /// Left margin to header.
    double              mfHdrRightMargin;   /// Right margin to header.
    double              mfFtrLeftMargin;    /// Left margin to footer.
    double              mfFtrRightMargin;   /// Right margin to footer.
    sal_uInt16          mnPaperSize;        /// Index into paper size table.
    sal_uInt16          mnStrictPaperSize;  /// Same as papersize - but for ooxml (considering stricter dimensions)
    sal_uInt16          mnPaperWidth;       /// Paper Width in mm
    sal_uInt16          mnPaperHeight;      /// Paper Height in mm
    sal_uInt16          mnCopies;           /// Number of copies.
    sal_uInt16          mnStartPage;        /// Start page number.
    sal_uInt16          mnScaling;          /// Scaling in percent.
    sal_uInt16          mnFitToWidth;       /// Fit to number of pages in width.
    sal_uInt16          mnFitToHeight;      /// Fit to number of pages in height.
    sal_uInt16          mnHorPrintRes;      /// Horizontal printing resolution.
    sal_uInt16          mnVerPrintRes;      /// Vertical printing resolution.
    bool                mbValid;            /// false = some of the values are not valid.
    bool                mbPortrait;         /// true = portrait; false = landscape.
    bool                mbPrintInRows;      /// true = in rows; false = in columns.
    bool                mbBlackWhite;       /// true = black/white; false = colors.
    bool                mbDraftQuality;     /// true = draft; false = default quality.
    bool                mbPrintNotes;       /// true = print notes.
    bool                mbManualStart;      /// true = mnStartPage valid; false = automatic.
    bool                mbFitToPages;       /// true = fit to pages; false = scale in percent.
    bool                mbHorCenter;        /// true = centered horizontally; false = left aligned.
    bool                mbVerCenter;        /// true = centered vertically; false = top aligned.
    bool                mbPrintHeadings;    /// true = print column and row headings.
    bool                mbPrintGrid;        /// true = print grid lines.

    explicit            XclPageData();
                        ~XclPageData();

    /** Sets Excel default page settings. */
    void                SetDefaults();

    /** Returns the real paper size (twips) from the paper size index and paper orientation. */
    Size                GetScPaperSize() const;
    /** Sets the Excel paper size index and paper orientation from Calc paper size (twips). */
    void                SetScPaperSize( const Size& rSize, bool bPortrait, bool bStrict = false );
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
