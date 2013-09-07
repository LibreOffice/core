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

#include "xlpage.hxx"
#include <sfx2/printer.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/paperinf.hxx>
#include <vcl/svapp.hxx>
#include <sal/macros.h>
#include "scitems.hxx"
#include <editeng/brushitem.hxx>
#include "global.hxx"
#include "xlconst.hxx"
#include <oox/core/xmlfilterbase.hxx>

// Paper size =================================================================

struct XclPaperSize
{
    Paper               mePaper;            /// SVX paper size identifier.
    long                mnWidth;            /// Paper width in twips.
    long                mnHeight;           /// Paper height in twips.
};

#define IN2TWIPS( v )      ((long)((v) * EXC_TWIPS_PER_INCH + 0.5))
#define MM2TWIPS( v )      ((long)((v) * EXC_TWIPS_PER_INCH / CM_PER_INCH / 10.0 + 0.5))
#define TWIPS2MM( v )      ((long)((v - 0.5) / EXC_TWIPS_PER_INCH * CM_PER_INCH * 10.0))


static const XclPaperSize pPaperSizeTable[] =
{
/*  0*/ { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_LETTER,     IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  // Letter
        { PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  // Letter Small
        { PAPER_TABLOID,    IN2TWIPS( 11 ),     IN2TWIPS( 17 )      },  // Tabloid
        { PAPER_LEDGER,     IN2TWIPS( 17 ),     IN2TWIPS( 11 )      },  // Ledger
/*  5*/ { PAPER_LEGAL,      IN2TWIPS( 8.5 ),    IN2TWIPS( 14 )      },  // Legal
        { PAPER_STATEMENT,  IN2TWIPS( 5.5 ),    IN2TWIPS( 8.5 )     },  // Statement
        { PAPER_EXECUTIVE,  IN2TWIPS( 7.25 ),   IN2TWIPS( 10.5 )    },  // Executive
        { PAPER_A3,         MM2TWIPS( 297 ),    MM2TWIPS( 420 )     },  // A3
        { PAPER_A4,         MM2TWIPS( 210 ),    MM2TWIPS( 297 )     },  // A4
/* 10*/ { PAPER_USER,       MM2TWIPS( 210 ),    MM2TWIPS( 297 )     },  // A4 Small
        { PAPER_A5,         MM2TWIPS( 148 ),    MM2TWIPS( 210 )     },  // A5
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
        { PAPER_B4_JIS,     MM2TWIPS( 257 ),    MM2TWIPS( 364 )     },  // B4 (JIS)
        { PAPER_B5_JIS,     MM2TWIPS( 182 ),    MM2TWIPS( 257 )     },  // B5 (JIS)
        { PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 13 )      },  // Folio
/* 15*/ { PAPER_QUARTO,     MM2TWIPS( 215 ),    MM2TWIPS( 275 )     },  // Quarto
        { PAPER_10x14,      IN2TWIPS( 10 ),     IN2TWIPS( 14 )      },  // 10x14
        { PAPER_USER,       IN2TWIPS( 11 ),     IN2TWIPS( 17 )      },  // 11x17
        { PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  // Note
        { PAPER_ENV_9,       IN2TWIPS( 3.875 ),  IN2TWIPS( 8.875 )   },  // Envelope #9
/* 20*/ { PAPER_ENV_10,      IN2TWIPS( 4.125 ),  IN2TWIPS( 9.5 )     },  // Envelope #10
        { PAPER_ENV_11,      IN2TWIPS( 4.5 ),    IN2TWIPS( 10.375 )  },  // Envelope #11
        { PAPER_ENV_12,      IN2TWIPS( 4.75 ),   IN2TWIPS( 11 )      },  // Envelope #12
        { PAPER_ENV_14,      IN2TWIPS( 5 ),      IN2TWIPS( 11.5 )    },  // Envelope #14
        { PAPER_C,          IN2TWIPS( 17 ),     IN2TWIPS( 22 )      },  // ANSI-C
/* 25*/ { PAPER_D,          IN2TWIPS( 22 ),     IN2TWIPS( 34 )      },  // ANSI-D
        { PAPER_E,          IN2TWIPS( 34 ),     IN2TWIPS( 44 )      },  // ANSI-E
        { PAPER_ENV_DL,         MM2TWIPS( 110 ),    MM2TWIPS( 220 )     },  // Envelope DL
        { PAPER_ENV_C5,         MM2TWIPS( 162 ),    MM2TWIPS( 229 )     },  // Envelope C5
        { PAPER_ENV_C3,         MM2TWIPS( 324 ),    MM2TWIPS( 458 )     },  // Envelope C3
/* 30*/ { PAPER_ENV_C4,         MM2TWIPS( 229 ),    MM2TWIPS( 324 )     },  // Envelope C4
        { PAPER_ENV_C6,         MM2TWIPS( 114 ),    MM2TWIPS( 162 )     },  // Envelope C6
        { PAPER_ENV_C65,        MM2TWIPS( 114 ),    MM2TWIPS( 229 )     },  // Envelope C65
        { PAPER_B4_ISO,     MM2TWIPS( 250 ),    MM2TWIPS( 353 )     },  // B4 (ISO)
        { PAPER_B5_ISO,     MM2TWIPS( 176 ),    MM2TWIPS( 250 )     },  // B5 (ISO)
/* 35*/ { PAPER_B6_ISO,     MM2TWIPS( 125 ),    MM2TWIPS( 176 )     },  // B6 (ISO)
        { PAPER_ENV_ITALY,    MM2TWIPS( 110 ),    MM2TWIPS( 230 )     },  // Envelope Italy
        { PAPER_ENV_MONARCH,    IN2TWIPS( 3.875 ),  IN2TWIPS( 7.5 )     },  // Envelope Monarch
        { PAPER_ENV_PERSONAL,     IN2TWIPS( 3.625 ),  IN2TWIPS( 6.5 )     },  // 6 3/4 Envelope
        { PAPER_FANFOLD_US, IN2TWIPS( 14.875 ), IN2TWIPS( 11 )      },  // US Std Fanfold
/* 40*/ { PAPER_FANFOLD_DE, IN2TWIPS( 8.5 ),    IN2TWIPS( 12 )      },  // German Std Fanfold
        { PAPER_FANFOLD_LEGAL_DE,     IN2TWIPS( 8.5 ),    IN2TWIPS( 13 )      },  // German Legal Fanfold
        { PAPER_B4_ISO,     MM2TWIPS( 250 ),    MM2TWIPS( 353 )     },  // B4 (ISO)
        { PAPER_POSTCARD_JP,MM2TWIPS( 100 ),    MM2TWIPS( 148 )     },  // Japanese Postcard
        { PAPER_9x11,       IN2TWIPS( 9 ),      IN2TWIPS( 11 )      },  // 9x11
/* 45*/ { PAPER_10x11,      IN2TWIPS( 10 ),     IN2TWIPS( 11 )      },  // 10x11
        { PAPER_15x11,      IN2TWIPS( 15 ),     IN2TWIPS( 11 )      },  // 15x11
        { PAPER_ENV_INVITE,     MM2TWIPS( 220 ),    MM2TWIPS( 220 )     },  // Envelope Invite
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
/* 50*/ { PAPER_USER,       IN2TWIPS( 9.5 ),    IN2TWIPS( 12 )      },  // Letter Extra
        { PAPER_USER,       IN2TWIPS( 9.5 ),    IN2TWIPS( 15 )      },  // Legal Extra
        { PAPER_USER,       IN2TWIPS( 11.69 ),  IN2TWIPS( 18 )      },  // Tabloid Extra
        { PAPER_USER,       MM2TWIPS( 235 ),    MM2TWIPS( 322 )     },  // A4 Extra
        { PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  // Letter Transverse
/* 55*/ { PAPER_USER,       MM2TWIPS( 210 ),    MM2TWIPS( 297 )     },  // A4 Transverse
        { PAPER_USER,       IN2TWIPS( 9.5 ),    IN2TWIPS( 12 )      },  // Letter Extra Transverse
        { PAPER_A_PLUS,     MM2TWIPS( 227 ),    MM2TWIPS( 356 )     },  // Super A/A4
        { PAPER_B_PLUS,     MM2TWIPS( 305 ),    MM2TWIPS( 487 )     },  // Super B/A3
        { PAPER_LETTER_PLUS,IN2TWIPS( 8.5 ),    IN2TWIPS( 12.69 )   },  // Letter Plus
/* 60*/ { PAPER_A4_PLUS,    MM2TWIPS( 210 ),    MM2TWIPS( 330 )     },  // A4 Plus
        { PAPER_USER,       MM2TWIPS( 148 ),    MM2TWIPS( 210 )     },  // A5 Transverse
        { PAPER_USER,       MM2TWIPS( 182 ),    MM2TWIPS( 257 )     },  // B5 (JIS) Transverse
        { PAPER_USER,       MM2TWIPS( 322 ),    MM2TWIPS( 445 )     },  // A3 Extra
        { PAPER_USER,       MM2TWIPS( 174 ),    MM2TWIPS( 235 )     },  // A5 Extra
/* 65*/ { PAPER_USER,       MM2TWIPS( 201 ),    MM2TWIPS( 276 )     },  // B5 (ISO) Extra
        { PAPER_A2,         MM2TWIPS( 420 ),    MM2TWIPS( 594 )     },  // A2
        { PAPER_USER,       MM2TWIPS( 297 ),    MM2TWIPS( 420 )     },  // A3 Transverse
        { PAPER_USER,       MM2TWIPS( 322 ),    MM2TWIPS( 445 )     },  // A3 Extra Transverse
        { PAPER_DOUBLEPOSTCARD_JP,       MM2TWIPS( 200 ),    MM2TWIPS( 148 )     },  // Double Japanese Postcard
/* 70*/ { PAPER_A6,         MM2TWIPS( 105 ),    MM2TWIPS( 148 )     },  // A6
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
/* 75*/ { PAPER_USER,       IN2TWIPS( 11 ),     IN2TWIPS( 8.5 )     },  // Letter Rotated
        { PAPER_USER,       MM2TWIPS( 420 ),    MM2TWIPS( 297 )     },  // A3 Rotated
        { PAPER_USER,       MM2TWIPS( 297 ),    MM2TWIPS( 210 )     },  // A4 Rotated
        { PAPER_USER,       MM2TWIPS( 210 ),    MM2TWIPS( 148 )     },  // A5 Rotated
        { PAPER_USER,       MM2TWIPS( 364 ),    MM2TWIPS( 257 )     },  // B4 (JIS) Rotated
/* 80*/ { PAPER_USER,       MM2TWIPS( 257 ),    MM2TWIPS( 182 )     },  // B5 (JIS) Rotated
        { PAPER_USER,       MM2TWIPS( 148 ),    MM2TWIPS( 100 )     },  // Japanese Postcard Rotated
        { PAPER_USER,       MM2TWIPS( 148 ),    MM2TWIPS( 200 )     },  // Double Japanese Postcard Rotated
        { PAPER_USER,       MM2TWIPS( 148 ),    MM2TWIPS( 105 )     },  // A6 Rotated
        { PAPER_USER,       0,                  0                   },  // undefined
/* 85*/ { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_USER,       0,                  0                   },  // undefined
        { PAPER_B6_JIS,     MM2TWIPS( 128 ),    MM2TWIPS( 182 )     },  // B6 (JIS)
        { PAPER_USER,       MM2TWIPS( 182 ),    MM2TWIPS( 128 )     },  // B6 (JIS) Rotated
/* 90*/ { PAPER_12x11,      IN2TWIPS( 12 ),     IN2TWIPS( 11 )      }   // 12x11
};

#undef IN2TWIPS
#undef MM2TWIPS

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
    maHeader.Erase();
    maFooter.Erase();
    mfLeftMargin    = mfRightMargin    = XclTools::GetInchFromHmm( EXC_MARGIN_DEFAULT_LR );
    mfTopMargin     = mfBottomMargin   = XclTools::GetInchFromHmm( EXC_MARGIN_DEFAULT_TB );
    mfHeaderMargin  = mfFooterMargin   = XclTools::GetInchFromHmm( EXC_MARGIN_DEFAULT_HF );
    mfHdrLeftMargin = mfHdrRightMargin = XclTools::GetInchFromHmm( EXC_MARGIN_DEFAULT_HLR );
    mfFtrLeftMargin = mfFtrRightMargin = XclTools::GetInchFromHmm( EXC_MARGIN_DEFAULT_FLR );
    mnPaperSize = EXC_PAPERSIZE_DEFAULT;
    mnPaperWidth = 0;
    mnPaperHeight = 0;
    mnCopies = 1;
    mnStartPage = 0;
    mnScaling = 100;
    mnFitToWidth = mnFitToHeight = 1;
    mnHorPrintRes = mnVerPrintRes = 300;
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
        ::std::swap( aSize.Width(), aSize.Height() );

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

    mnPaperWidth = TWIPS2MM( nWidth );
    mnPaperHeight = TWIPS2MM( nHeight );
    if( bStrictSize )
    {
        nMaxWDiff = 5;
        nMaxHDiff = 5;
        mnStrictPaperSize = EXC_PAPERSIZE_USER;
    }
    else
    {
        mnPaperSize = 0;
    }

    for( const XclPaperSize* pEntry = pPaperSizeTable; pEntry != STATIC_ARRAY_END( pPaperSizeTable ); ++pEntry )
    {
        long nWDiff = std::abs( pEntry->mnWidth - nWidth );
        long nHDiff = std::abs( pEntry->mnHeight - nHeight );
        if( ((nWDiff <= nMaxWDiff) && (nHDiff < nMaxHDiff)) ||
            ((nWDiff < nMaxWDiff) && (nHDiff <= nMaxHDiff)) )
        {
            sal_uInt16 nIndex = static_cast< sal_uInt16 >( pEntry - pPaperSizeTable );
            if( !bStrictSize )
                mnPaperSize = nIndex;
            else
                mnStrictPaperSize = mnPaperSize = nIndex;

            nMaxWDiff = nWDiff;
            nMaxHDiff = nHDiff;
        }
    }
    if( !bStrictSize )
        SetScPaperSize( rSize, bPortrait, sal_True );
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
