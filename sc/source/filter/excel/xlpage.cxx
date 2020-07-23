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

#include <xlpage.hxx>
#include <xltools.hxx>
#include <editeng/paperinf.hxx>
#include <sal/macros.h>
#include <editeng/brushitem.hxx>
#include <global.hxx>
#include <xlconst.hxx>

namespace{

struct XclPaperSize
{
    Paper               mePaper;            /// SVX paper size identifier.
    long                mnWidth;            /// Paper width in twips.
    long                mnHeight;           /// Paper height in twips.
};

constexpr long in2twips(double n_inch)
{
    return static_cast<long>( (n_inch * EXC_TWIPS_PER_INCH) + 0.5);
}
constexpr long mm2twips(double n_mm)
{
    return static_cast<long>( (n_mm * EXC_TWIPS_PER_INCH / CM_PER_INCH / 10.0) + 0.5);
}
constexpr long twips2mm(long n_twips)
{
    return static_cast<long>((static_cast<double>(n_twips) - 0.5) / EXC_TWIPS_PER_INCH * CM_PER_INCH * 10.0);
}

constexpr XclPaperSize pPaperSizeTable[] =
{
/*  0*/ { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_LETTER,     in2twips( 8.5 ),    in2twips( 11 )      },  // Letter
        { PAPER_USER,       in2twips( 8.5 ),    in2twips( 11 )      },  // Letter Small
        { PAPER_TABLOID,    in2twips( 11 ),     in2twips( 17 )      },  // Tabloid
        { PAPER_LEDGER,     in2twips( 17 ),     in2twips( 11 )      },  // Ledger
/*  5*/ { PAPER_LEGAL,      in2twips( 8.5 ),    in2twips( 14 )      },  // Legal
        { PAPER_STATEMENT,  in2twips( 5.5 ),    in2twips( 8.5 )     },  // Statement
        { PAPER_EXECUTIVE,  in2twips( 7.25 ),   in2twips( 10.5 )    },  // Executive
        { PAPER_A3,         mm2twips( 297 ),    mm2twips( 420 )     },  // A3
        { PAPER_A4,         mm2twips( 210 ),    mm2twips( 297 )     },  // A4
/* 10*/ { PAPER_USER,       mm2twips( 210 ),    mm2twips( 297 )     },  // A4 Small
        { PAPER_A5,         mm2twips( 148 ),    mm2twips( 210 )     },  // A5
        //See: http://wiki.openoffice.org/wiki/DefaultPaperSize comments
        //near DMPAPER_B4 in vcl
        //i.e.
        //http://msdn.microsoft.com/en-us/library/bb241398.aspx makes the claim:
        //xlPaperB4 12  B4 (250 mm x 354 mm)
        //xlPaperB5 13  A5 (148 mm x 210 mm)
        //but, a paper enum called B5 is surely not actually "A5", and furthermore
        //the XlPaperSize enumeration otherwise follows the DMPAPER values
        //http://msdn.microsoft.com/en-us/library/ms776398(VS.85).aspx
        //which has
        //DMPAPER_B4    12  B4 (JIS) 250 x 354
        //DMPAPER_B5    13  B5 (JIS) 182 x 257 mm
    //which claim them to be the JIS sizes. Though that document then gives
    //"B4 (JIS)" an *ISO* B4 size in the text, but
        //http://partners.adobe.com/public/developer/en/ps/5003.PPD_Spec_v4.3.pdf
        //claims that the MS DMPAPER_B4 and DMPAPER_B5 truly are the JIS sizes
        //which at least makes some sort of sense. (cmc)
        { PAPER_B4_JIS,     mm2twips( 257 ),    mm2twips( 364 )     },  // B4 (JIS)
        { PAPER_B5_JIS,     mm2twips( 182 ),    mm2twips( 257 )     },  // B5 (JIS)
        { PAPER_USER,       in2twips( 8.5 ),    in2twips( 13 )      },  // Folio
/* 15*/ { PAPER_QUARTO,     mm2twips( 215 ),    mm2twips( 275 )     },  // Quarto
        { PAPER_10x14,      in2twips( 10 ),     in2twips( 14 )      },  // 10x14
        { PAPER_USER,       in2twips( 11 ),     in2twips( 17 )      },  // 11x17
        { PAPER_USER,       in2twips( 8.5 ),    in2twips( 11 )      },  // Note
        { PAPER_ENV_9,       in2twips( 3.875 ),  in2twips( 8.875 )   },  // Envelope #9
/* 20*/ { PAPER_ENV_10,      in2twips( 4.125 ),  in2twips( 9.5 )     },  // Envelope #10
        { PAPER_ENV_11,      in2twips( 4.5 ),    in2twips( 10.375 )  },  // Envelope #11
        { PAPER_ENV_12,      in2twips( 4.75 ),   in2twips( 11 )      },  // Envelope #12
        { PAPER_ENV_14,      in2twips( 5 ),      in2twips( 11.5 )    },  // Envelope #14
        { PAPER_C,          in2twips( 17 ),     in2twips( 22 )      },  // ANSI-C
/* 25*/ { PAPER_D,          in2twips( 22 ),     in2twips( 34 )      },  // ANSI-D
        { PAPER_E,          in2twips( 34 ),     in2twips( 44 )      },  // ANSI-E
        { PAPER_ENV_DL,         mm2twips( 110 ),    mm2twips( 220 )     },  // Envelope DL
        { PAPER_ENV_C5,         mm2twips( 162 ),    mm2twips( 229 )     },  // Envelope C5
        { PAPER_ENV_C3,         mm2twips( 324 ),    mm2twips( 458 )     },  // Envelope C3
/* 30*/ { PAPER_ENV_C4,         mm2twips( 229 ),    mm2twips( 324 )     },  // Envelope C4
        { PAPER_ENV_C6,         mm2twips( 114 ),    mm2twips( 162 )     },  // Envelope C6
        { PAPER_ENV_C65,        mm2twips( 114 ),    mm2twips( 229 )     },  // Envelope C65
        { PAPER_B4_ISO,     mm2twips( 250 ),    mm2twips( 353 )     },  // B4 (ISO)
        { PAPER_B5_ISO,     mm2twips( 176 ),    mm2twips( 250 )     },  // B5 (ISO)
/* 35*/ { PAPER_B6_ISO,     mm2twips( 125 ),    mm2twips( 176 )     },  // B6 (ISO)
        { PAPER_ENV_ITALY,    mm2twips( 110 ),    mm2twips( 230 )     },  // Envelope Italy
        { PAPER_ENV_MONARCH,    in2twips( 3.875 ),  in2twips( 7.5 )     },  // Envelope Monarch
        { PAPER_ENV_PERSONAL,     in2twips( 3.625 ),  in2twips( 6.5 )     },  // 6 3/4 Envelope
        { PAPER_FANFOLD_US, in2twips( 14.875 ), in2twips( 11 )      },  // US Std Fanfold
/* 40*/ { PAPER_FANFOLD_DE, in2twips( 8.5 ),    in2twips( 12 )      },  // German Std Fanfold
        { PAPER_FANFOLD_LEGAL_DE,     in2twips( 8.5 ),    in2twips( 13 )      },  // German Legal Fanfold
        { PAPER_B4_ISO,     mm2twips( 250 ),    mm2twips( 353 )     },  // B4 (ISO)
        { PAPER_POSTCARD_JP,mm2twips( 100 ),    mm2twips( 148 )     },  // Japanese Postcard
        { PAPER_9x11,       in2twips( 9 ),      in2twips( 11 )      },  // 9x11
/* 45*/ { PAPER_10x11,      in2twips( 10 ),     in2twips( 11 )      },  // 10x11
        { PAPER_15x11,      in2twips( 15 ),     in2twips( 11 )      },  // 15x11
        { PAPER_ENV_INVITE,     mm2twips( 220 ),    mm2twips( 220 )     },  // Envelope Invite
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
/* 50*/ { PAPER_USER,       in2twips( 9.5 ),    in2twips( 12 )      },  // Letter Extra
        { PAPER_USER,       in2twips( 9.5 ),    in2twips( 15 )      },  // Legal Extra
        { PAPER_USER,       in2twips( 11.69 ),  in2twips( 18 )      },  // Tabloid Extra
        { PAPER_USER,       mm2twips( 235 ),    mm2twips( 322 )     },  // A4 Extra
        { PAPER_USER,       in2twips( 8.5 ),    in2twips( 11 )      },  // Letter Transverse
/* 55*/ { PAPER_USER,       mm2twips( 210 ),    mm2twips( 297 )     },  // A4 Transverse
        { PAPER_USER,       in2twips( 9.5 ),    in2twips( 12 )      },  // Letter Extra Transverse
        { PAPER_A_PLUS,     mm2twips( 227 ),    mm2twips( 356 )     },  // Super A/A4
        { PAPER_B_PLUS,     mm2twips( 305 ),    mm2twips( 487 )     },  // Super B/A3
        { PAPER_LETTER_PLUS,in2twips( 8.5 ),    in2twips( 12.69 )   },  // Letter Plus
/* 60*/ { PAPER_A4_PLUS,    mm2twips( 210 ),    mm2twips( 330 )     },  // A4 Plus
        { PAPER_USER,       mm2twips( 148 ),    mm2twips( 210 )     },  // A5 Transverse
        { PAPER_USER,       mm2twips( 182 ),    mm2twips( 257 )     },  // B5 (JIS) Transverse
        { PAPER_USER,       mm2twips( 322 ),    mm2twips( 445 )     },  // A3 Extra
        { PAPER_USER,       mm2twips( 174 ),    mm2twips( 235 )     },  // A5 Extra
/* 65*/ { PAPER_USER,       mm2twips( 201 ),    mm2twips( 276 )     },  // B5 (ISO) Extra
        { PAPER_A2,         mm2twips( 420 ),    mm2twips( 594 )     },  // A2
        { PAPER_USER,       mm2twips( 297 ),    mm2twips( 420 )     },  // A3 Transverse
        { PAPER_USER,       mm2twips( 322 ),    mm2twips( 445 )     },  // A3 Extra Transverse
        { PAPER_DOUBLEPOSTCARD_JP,       mm2twips( 200 ),    mm2twips( 148 )     },  // Double Japanese Postcard
/* 70*/ { PAPER_A6,         mm2twips( 105 ),    mm2twips( 148 )     },  // A6
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
/* 75*/ { PAPER_USER,       in2twips( 11 ),     in2twips( 8.5 )     },  // Letter Rotated
        { PAPER_USER,       mm2twips( 420 ),    mm2twips( 297 )     },  // A3 Rotated
        { PAPER_USER,       mm2twips( 297 ),    mm2twips( 210 )     },  // A4 Rotated
        { PAPER_USER,       mm2twips( 210 ),    mm2twips( 148 )     },  // A5 Rotated
        { PAPER_USER,       mm2twips( 364 ),    mm2twips( 257 )     },  // B4 (JIS) Rotated
/* 80*/ { PAPER_USER,       mm2twips( 257 ),    mm2twips( 182 )     },  // B5 (JIS) Rotated
        { PAPER_USER,       mm2twips( 148 ),    mm2twips( 100 )     },  // Japanese Postcard Rotated
        { PAPER_USER,       mm2twips( 148 ),    mm2twips( 200 )     },  // Double Japanese Postcard Rotated
        { PAPER_USER,       mm2twips( 148 ),    mm2twips( 105 )     },  // A6 Rotated
        { PAPER_USER,       0,                  0                   },  // undefined
/* 85*/ { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_B6_JIS,     mm2twips( 128 ),    mm2twips( 182 )     },  // B6 (JIS)
        { PAPER_USER,       mm2twips( 182 ),    mm2twips( 128 )     },  // B6 (JIS) Rotated
/* 90*/ { PAPER_12x11,      in2twips( 12 ),     in2twips( 11 )      }   // 12x11
};

} //namespace

// Page settings ==============================================================

XclPageData::XclPageData()
{
    SetDefaults();
}

XclPageData::~XclPageData()
{
    // SvxBrushItem incomplete in header
}

void XclPageData::SetDefaults()
{
    maHorPageBreaks.clear();
    maVerPageBreaks.clear();
    mxBrushItem.reset();
    maHeader.clear();
    maFooter.clear();
    maHeaderEven.clear();
    maFooterEven.clear();
    mfLeftMargin    = mfRightMargin    = XclTools::GetInchFromHmm( EXC_MARGIN_DEFAULT_LR );
    mfTopMargin     = mfBottomMargin   = XclTools::GetInchFromHmm( EXC_MARGIN_DEFAULT_TB );
    mfHeaderMargin  = mfFooterMargin   = XclTools::GetInchFromHmm( EXC_MARGIN_DEFAULT_HF );
    mfHdrLeftMargin = mfHdrRightMargin = XclTools::GetInchFromHmm( EXC_MARGIN_DEFAULT_HLR );
    mfFtrLeftMargin = mfFtrRightMargin = XclTools::GetInchFromHmm( EXC_MARGIN_DEFAULT_FLR );
    mnStrictPaperSize = mnPaperSize = EXC_PAPERSIZE_DEFAULT;
    mnPaperWidth = 0;
    mnPaperHeight = 0;
    mnCopies = 1;
    mnStartPage = 0;
    mnScaling = 100;
    mnFitToWidth = mnFitToHeight = 1;
    mnHorPrintRes = mnVerPrintRes = 300;
    mbUseEvenHF = /*mbUseFirstHF =*/ false;
    mbValid = false;
    mbPortrait = true;
    mbPrintInRows = mbBlackWhite = mbDraftQuality = mbPrintNotes = mbManualStart = mbFitToPages = false;
    mbHorCenter = mbVerCenter = mbPrintHeadings = mbPrintGrid = false;
}

Size XclPageData::GetScPaperSize() const
{
    const XclPaperSize* pEntry = pPaperSizeTable;
    if( mnPaperSize < SAL_N_ELEMENTS( pPaperSizeTable ) )
        pEntry += mnPaperSize;

    Size aSize;
    if( pEntry->mePaper == PAPER_USER )
        aSize = Size( pEntry->mnWidth, pEntry->mnHeight );
    else
        aSize = SvxPaperInfo::GetPaperSize( pEntry->mePaper );

    // invalid size -> back to default
    if( !aSize.Width() || !aSize.Height() )
        aSize = SvxPaperInfo::GetDefaultPaperSize();

    if( !mbPortrait )
    {
        // swap width and height
        long n = aSize.Width();
        aSize.setWidth(aSize.Height());
        aSize.setHeight(n);
    }

    return aSize;
}

void XclPageData::SetScPaperSize( const Size& rSize, bool bPortrait, bool bStrictSize )
{
    mbPortrait = bPortrait;
    mnPaperSize = 0;
    long nWidth = bPortrait ? rSize.Width() : rSize.Height();
    long nHeight = bPortrait ? rSize.Height() : rSize.Width();
    long nMaxWDiff = 80;
    long nMaxHDiff = 50;

    mnPaperWidth = twips2mm( nWidth );
    mnPaperHeight = twips2mm( nHeight );
    if( bStrictSize )
    {
        nMaxWDiff = 5;
        nMaxHDiff = 5;
        mnStrictPaperSize = EXC_PAPERSIZE_USER;
    }
    else
    {
        mnPaperSize = EXC_PAPERSIZE_DEFAULT;
    }

    for( const auto &rEntry : pPaperSizeTable)
    {
        long nWDiff = std::abs( rEntry.mnWidth - nWidth );
        long nHDiff = std::abs( rEntry.mnHeight - nHeight );
        if( ((nWDiff <= nMaxWDiff) && (nHDiff < nMaxHDiff)) ||
            ((nWDiff < nMaxWDiff) && (nHDiff <= nMaxHDiff)) )
        {
            sal_uInt16 nIndex = static_cast< sal_uInt16 >( &rEntry - pPaperSizeTable );
            mnPaperSize = nIndex;
            if( bStrictSize )
                mnStrictPaperSize = nIndex;

            nMaxWDiff = nWDiff;
            nMaxHDiff = nHDiff;
        }
    }
    if( !bStrictSize )
        SetScPaperSize( rSize, bPortrait, true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
