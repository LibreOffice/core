/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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



struct XclPaperSize
{
    Paper               mePaper;            
    long                mnWidth;            
    long                mnHeight;           
};

#define IN2TWIPS( v )      ((long)((v) * EXC_TWIPS_PER_INCH + 0.5))
#define MM2TWIPS( v )      ((long)((v) * EXC_TWIPS_PER_INCH / CM_PER_INCH / 10.0 + 0.5))
#define TWIPS2MM( v )      ((long)((v - 0.5) / EXC_TWIPS_PER_INCH * CM_PER_INCH * 10.0))


static const XclPaperSize pPaperSizeTable[] =
{
/*  0*/ { PAPER_USER,       0,                  0                   },  
        { PAPER_LETTER,     IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  
        { PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  
        { PAPER_TABLOID,    IN2TWIPS( 11 ),     IN2TWIPS( 17 )      },  
        { PAPER_LEDGER,     IN2TWIPS( 17 ),     IN2TWIPS( 11 )      },  
/*  5*/ { PAPER_LEGAL,      IN2TWIPS( 8.5 ),    IN2TWIPS( 14 )      },  
        { PAPER_STATEMENT,  IN2TWIPS( 5.5 ),    IN2TWIPS( 8.5 )     },  
        { PAPER_EXECUTIVE,  IN2TWIPS( 7.25 ),   IN2TWIPS( 10.5 )    },  
        { PAPER_A3,         MM2TWIPS( 297 ),    MM2TWIPS( 420 )     },  
        { PAPER_A4,         MM2TWIPS( 210 ),    MM2TWIPS( 297 )     },  
/* 10*/ { PAPER_USER,       MM2TWIPS( 210 ),    MM2TWIPS( 297 )     },  
        { PAPER_A5,         MM2TWIPS( 148 ),    MM2TWIPS( 210 )     },  
        
        
        
        
        
        
        
        
        
        
        
        
    
    
        
        
        
        { PAPER_B4_JIS,     MM2TWIPS( 257 ),    MM2TWIPS( 364 )     },  
        { PAPER_B5_JIS,     MM2TWIPS( 182 ),    MM2TWIPS( 257 )     },  
        { PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 13 )      },  
/* 15*/ { PAPER_QUARTO,     MM2TWIPS( 215 ),    MM2TWIPS( 275 )     },  
        { PAPER_10x14,      IN2TWIPS( 10 ),     IN2TWIPS( 14 )      },  
        { PAPER_USER,       IN2TWIPS( 11 ),     IN2TWIPS( 17 )      },  
        { PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  
        { PAPER_ENV_9,       IN2TWIPS( 3.875 ),  IN2TWIPS( 8.875 )   },  
/* 20*/ { PAPER_ENV_10,      IN2TWIPS( 4.125 ),  IN2TWIPS( 9.5 )     },  
        { PAPER_ENV_11,      IN2TWIPS( 4.5 ),    IN2TWIPS( 10.375 )  },  
        { PAPER_ENV_12,      IN2TWIPS( 4.75 ),   IN2TWIPS( 11 )      },  
        { PAPER_ENV_14,      IN2TWIPS( 5 ),      IN2TWIPS( 11.5 )    },  
        { PAPER_C,          IN2TWIPS( 17 ),     IN2TWIPS( 22 )      },  
/* 25*/ { PAPER_D,          IN2TWIPS( 22 ),     IN2TWIPS( 34 )      },  
        { PAPER_E,          IN2TWIPS( 34 ),     IN2TWIPS( 44 )      },  
        { PAPER_ENV_DL,         MM2TWIPS( 110 ),    MM2TWIPS( 220 )     },  
        { PAPER_ENV_C5,         MM2TWIPS( 162 ),    MM2TWIPS( 229 )     },  
        { PAPER_ENV_C3,         MM2TWIPS( 324 ),    MM2TWIPS( 458 )     },  
/* 30*/ { PAPER_ENV_C4,         MM2TWIPS( 229 ),    MM2TWIPS( 324 )     },  
        { PAPER_ENV_C6,         MM2TWIPS( 114 ),    MM2TWIPS( 162 )     },  
        { PAPER_ENV_C65,        MM2TWIPS( 114 ),    MM2TWIPS( 229 )     },  
        { PAPER_B4_ISO,     MM2TWIPS( 250 ),    MM2TWIPS( 353 )     },  
        { PAPER_B5_ISO,     MM2TWIPS( 176 ),    MM2TWIPS( 250 )     },  
/* 35*/ { PAPER_B6_ISO,     MM2TWIPS( 125 ),    MM2TWIPS( 176 )     },  
        { PAPER_ENV_ITALY,    MM2TWIPS( 110 ),    MM2TWIPS( 230 )     },  
        { PAPER_ENV_MONARCH,    IN2TWIPS( 3.875 ),  IN2TWIPS( 7.5 )     },  
        { PAPER_ENV_PERSONAL,     IN2TWIPS( 3.625 ),  IN2TWIPS( 6.5 )     },  
        { PAPER_FANFOLD_US, IN2TWIPS( 14.875 ), IN2TWIPS( 11 )      },  
/* 40*/ { PAPER_FANFOLD_DE, IN2TWIPS( 8.5 ),    IN2TWIPS( 12 )      },  
        { PAPER_FANFOLD_LEGAL_DE,     IN2TWIPS( 8.5 ),    IN2TWIPS( 13 )      },  
        { PAPER_B4_ISO,     MM2TWIPS( 250 ),    MM2TWIPS( 353 )     },  
        { PAPER_POSTCARD_JP,MM2TWIPS( 100 ),    MM2TWIPS( 148 )     },  
        { PAPER_9x11,       IN2TWIPS( 9 ),      IN2TWIPS( 11 )      },  
/* 45*/ { PAPER_10x11,      IN2TWIPS( 10 ),     IN2TWIPS( 11 )      },  
        { PAPER_15x11,      IN2TWIPS( 15 ),     IN2TWIPS( 11 )      },  
        { PAPER_ENV_INVITE,     MM2TWIPS( 220 ),    MM2TWIPS( 220 )     },  
        { PAPER_USER,       0,                  0                   },  
        { PAPER_USER,       0,                  0                   },  
/* 50*/ { PAPER_USER,       IN2TWIPS( 9.5 ),    IN2TWIPS( 12 )      },  
        { PAPER_USER,       IN2TWIPS( 9.5 ),    IN2TWIPS( 15 )      },  
        { PAPER_USER,       IN2TWIPS( 11.69 ),  IN2TWIPS( 18 )      },  
        { PAPER_USER,       MM2TWIPS( 235 ),    MM2TWIPS( 322 )     },  
        { PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  
/* 55*/ { PAPER_USER,       MM2TWIPS( 210 ),    MM2TWIPS( 297 )     },  
        { PAPER_USER,       IN2TWIPS( 9.5 ),    IN2TWIPS( 12 )      },  
        { PAPER_A_PLUS,     MM2TWIPS( 227 ),    MM2TWIPS( 356 )     },  
        { PAPER_B_PLUS,     MM2TWIPS( 305 ),    MM2TWIPS( 487 )     },  
        { PAPER_LETTER_PLUS,IN2TWIPS( 8.5 ),    IN2TWIPS( 12.69 )   },  
/* 60*/ { PAPER_A4_PLUS,    MM2TWIPS( 210 ),    MM2TWIPS( 330 )     },  
        { PAPER_USER,       MM2TWIPS( 148 ),    MM2TWIPS( 210 )     },  
        { PAPER_USER,       MM2TWIPS( 182 ),    MM2TWIPS( 257 )     },  
        { PAPER_USER,       MM2TWIPS( 322 ),    MM2TWIPS( 445 )     },  
        { PAPER_USER,       MM2TWIPS( 174 ),    MM2TWIPS( 235 )     },  
/* 65*/ { PAPER_USER,       MM2TWIPS( 201 ),    MM2TWIPS( 276 )     },  
        { PAPER_A2,         MM2TWIPS( 420 ),    MM2TWIPS( 594 )     },  
        { PAPER_USER,       MM2TWIPS( 297 ),    MM2TWIPS( 420 )     },  
        { PAPER_USER,       MM2TWIPS( 322 ),    MM2TWIPS( 445 )     },  
        { PAPER_DOUBLEPOSTCARD_JP,       MM2TWIPS( 200 ),    MM2TWIPS( 148 )     },  
/* 70*/ { PAPER_A6,         MM2TWIPS( 105 ),    MM2TWIPS( 148 )     },  
        { PAPER_USER,       0,                  0                   },  
        { PAPER_USER,       0,                  0                   },  
        { PAPER_USER,       0,                  0                   },  
        { PAPER_USER,       0,                  0                   },  
/* 75*/ { PAPER_USER,       IN2TWIPS( 11 ),     IN2TWIPS( 8.5 )     },  
        { PAPER_USER,       MM2TWIPS( 420 ),    MM2TWIPS( 297 )     },  
        { PAPER_USER,       MM2TWIPS( 297 ),    MM2TWIPS( 210 )     },  
        { PAPER_USER,       MM2TWIPS( 210 ),    MM2TWIPS( 148 )     },  
        { PAPER_USER,       MM2TWIPS( 364 ),    MM2TWIPS( 257 )     },  
/* 80*/ { PAPER_USER,       MM2TWIPS( 257 ),    MM2TWIPS( 182 )     },  
        { PAPER_USER,       MM2TWIPS( 148 ),    MM2TWIPS( 100 )     },  
        { PAPER_USER,       MM2TWIPS( 148 ),    MM2TWIPS( 200 )     },  
        { PAPER_USER,       MM2TWIPS( 148 ),    MM2TWIPS( 105 )     },  
        { PAPER_USER,       0,                  0                   },  
/* 85*/ { PAPER_USER,       0,                  0                   },  
        { PAPER_USER,       0,                  0                   },  
        { PAPER_USER,       0,                  0                   },  
        { PAPER_B6_JIS,     MM2TWIPS( 128 ),    MM2TWIPS( 182 )     },  
        { PAPER_USER,       MM2TWIPS( 182 ),    MM2TWIPS( 128 )     },  
/* 90*/ { PAPER_12x11,      IN2TWIPS( 12 ),     IN2TWIPS( 11 )      }   
};

#undef IN2TWIPS
#undef MM2TWIPS



XclPageData::XclPageData()
{
    SetDefaults();
}

XclPageData::~XclPageData()
{
    
}

void XclPageData::SetDefaults()
{
    maHorPageBreaks.clear();
    maVerPageBreaks.clear();
    mxBrushItem.reset();
    maHeader = "";
    maFooter = "";
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
        SetScPaperSize( rSize, bPortrait, true );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
