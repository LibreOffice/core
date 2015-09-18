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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XLPAGE_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XLPAGE_HXX

#include <tools/gen.hxx>
#include <boost/noncopyable.hpp>
#include "xltools.hxx"
#include <memory>

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

// Page settings ==============================================================

class SvxBrushItem;

/** Contains all page (print) settings for a single sheet. */
struct XclPageData : private boost::noncopyable
{
    typedef std::unique_ptr< SvxBrushItem > SvxBrushItemPtr;

    ScfUInt16Vec        maHorPageBreaks;    /// Horizontal page breaks.
    ScfUInt16Vec        maVerPageBreaks;    /// Vertical page breaks.
    SvxBrushItemPtr     mxBrushItem;        /// Background bitmap.
    OUString            maHeader;           /// Excel header string (empty = off).
    OUString            maFooter;           /// Excel footer string (empty = off).
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
