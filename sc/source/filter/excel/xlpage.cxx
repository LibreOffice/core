/*************************************************************************
 *
 *  $RCSfile: xlpage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:36:28 $
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
#include "xlpage.hxx"
#endif

#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <svx/svxenum.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX
#include <svx/paperinf.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif


// Paper size =================================================================

struct XclPaperSize
{
    SvxPaper                    mePaper;            /// SVX paper size identifier.
    sal_Int32                   mnWidth;            /// Paper width in twips.
    sal_Int32                   mnHeight;           /// Paper height in twips.
};

#define IN2TWIPS( v )      ((sal_Int32)((v) * EXC_TWIPS_PER_INCH + 0.5))
#define MM2TWIPS( v )      ((sal_Int32)((v) * EXC_TWIPS_PER_INCH / CM_PER_INCH / 10.0 + 0.5))

static const XclPaperSize pPaperSizeTable[] =
{
    { SVX_PAPER_USER,       0,                  0                   },  // undefined
    { SVX_PAPER_LETTER,     IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  // Letter
    { SVX_PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  // Letter Small
    { SVX_PAPER_TABLOID,    IN2TWIPS( 11 ),     IN2TWIPS( 17 )      },  // Tabloid
    { SVX_PAPER_USER,       IN2TWIPS( 17 ),     IN2TWIPS( 11 )      },  // Ledger
    { SVX_PAPER_LEGAL,      IN2TWIPS( 8.5 ),    IN2TWIPS( 14 )      },  // Legal
    { SVX_PAPER_USER,       IN2TWIPS( 5.5 ),    IN2TWIPS( 8.5 )     },  // Statement
    { SVX_PAPER_USER,       IN2TWIPS( 7.25 ),   IN2TWIPS( 10.5 )    },  // Executive
    { SVX_PAPER_A3,         MM2TWIPS( 297 ),    MM2TWIPS( 420 )     },  // A3
    { SVX_PAPER_A4,         MM2TWIPS( 210 ),    MM2TWIPS( 297 )     },  // A4
    { SVX_PAPER_USER,       MM2TWIPS( 210 ),    MM2TWIPS( 297 )     },  // A4 Small
    { SVX_PAPER_A5,         MM2TWIPS( 148 ),    MM2TWIPS( 210 )     },  // A5
    { SVX_PAPER_B4,         MM2TWIPS( 250 ),    MM2TWIPS( 354 )     },  // B4
    { SVX_PAPER_B5,         MM2TWIPS( 182 ),    MM2TWIPS( 257 )     },  // B5
    { SVX_PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 13 )      },  // Folio
    { SVX_PAPER_USER,       MM2TWIPS( 215 ),    MM2TWIPS( 275 )     },  // Quarto
    { SVX_PAPER_USER,       IN2TWIPS( 10 ),     IN2TWIPS( 14 )      },  // 10x14
    { SVX_PAPER_USER,       IN2TWIPS( 11 ),     IN2TWIPS( 17 )      },  // 11x17
    { SVX_PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 11 )      },  // Note
    { SVX_PAPER_USER,       IN2TWIPS( 3.875 ),  IN2TWIPS( 8.875 )   },  // Envelope #9
    { SVX_PAPER_USER,       IN2TWIPS( 4.125 ),  IN2TWIPS( 9.5 )     },  // Envelope #10
    { SVX_PAPER_USER,       IN2TWIPS( 4.5 ),    IN2TWIPS( 10.375 )  },  // Envelope #11
    { SVX_PAPER_USER,       IN2TWIPS( 4.75 ),   IN2TWIPS( 11 )      },  // Envelope #12
    { SVX_PAPER_USER,       IN2TWIPS( 14.5 ),   IN2TWIPS( 11.5 )    },  // Envelope #14
    { SVX_PAPER_USER,       IN2TWIPS( 17 ),     IN2TWIPS( 22 )      },  // C size sheet
    { SVX_PAPER_USER,       IN2TWIPS( 22 ),     IN2TWIPS( 34 )      },  // D size sheet
    { SVX_PAPER_USER,       IN2TWIPS( 34 ),     IN2TWIPS( 44 )      },  // E size sheet
    { SVX_PAPER_DL,         MM2TWIPS( 110 ),    MM2TWIPS( 220 )     },  // Envelope DL
    { SVX_PAPER_C5,         MM2TWIPS( 162 ),    MM2TWIPS( 229 )     },  // Envelope C5
    { SVX_PAPER_USER,       MM2TWIPS( 324 ),    MM2TWIPS( 458 )     },  // Envelope C3
    { SVX_PAPER_C4,         MM2TWIPS( 229 ),    MM2TWIPS( 324 )     },  // Envelope C4
    { SVX_PAPER_C6,         MM2TWIPS( 114 ),    MM2TWIPS( 162 )     },  // Envelope C6
    { SVX_PAPER_C65,        MM2TWIPS( 114 ),    MM2TWIPS( 229 )     },  // Envelope C65
    { SVX_PAPER_B4,         MM2TWIPS( 250 ),    MM2TWIPS( 353 )     },  // Envelope B4
    { SVX_PAPER_B5,         MM2TWIPS( 176 ),    MM2TWIPS( 250 )     },  // Envelope B5
    { SVX_PAPER_B6,         MM2TWIPS( 176 ),    MM2TWIPS( 125 )     },  // Envelope B6
    { SVX_PAPER_USER,       MM2TWIPS( 110 ),    MM2TWIPS( 230 )     },  // Envelope Italy
    { SVX_PAPER_USER,       IN2TWIPS( 3.875 ),  IN2TWIPS( 7.5 )     },  // Envelope Monarch
    { SVX_PAPER_USER,       IN2TWIPS( 3.625 ),  IN2TWIPS( 6.5 )     },  // 6 3/4 Envelope
    { SVX_PAPER_USER,       IN2TWIPS( 14.875 ), IN2TWIPS( 11 )      },  // US Std Fanfold
    { SVX_PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 12 )      },  // German Std Fanfold
    { SVX_PAPER_USER,       IN2TWIPS( 8.5 ),    IN2TWIPS( 13 )      }   // German Legal Fanfold
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
    mpBrushItem.reset();
    maHeader.Erase();
    maFooter.Erase();
    mfLeftMargin   = mfRightMargin  = XclTools::GetInchFromHmm( 1900 );
    mfTopMargin    = mfBottomMargin = XclTools::GetInchFromHmm( 2500 );
    mfHeaderMargin = mfFooterMargin = XclTools::GetInchFromHmm( 1300 );
    mnPaperSize = EXC_PAPERSIZE_DEFAULT;
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

Size XclPageData::GetScPaperSize( SfxPrinter* pPrinter ) const
{
    const XclPaperSize* pEntry = pPaperSizeTable;
    if( mnPaperSize < STATIC_TABLE_SIZE( pPaperSizeTable ) )
        pEntry += mnPaperSize;

    Size aSize;
    if( pEntry->mePaper == SVX_PAPER_USER )
        aSize = Size( pEntry->mnWidth, pEntry->mnHeight );
    else
        aSize = SvxPaperInfo::GetPaperSize( pEntry->mePaper );

    // invalid size -> back to default
    if( !aSize.Width() || !aSize.Height() )
        aSize = SvxPaperInfo::GetPaperSize( pPrinter );

    if( !mbPortrait )
        ::std::swap( aSize.Width(), aSize.Height() );

    return aSize;
}

void XclPageData::SetScPaperSize( const Size& rSize, bool bPortrait )
{
    mbPortrait = bPortrait;
    mnPaperSize = 0;
    sal_Int32 nWidth = bPortrait ? rSize.Width() : rSize.Height();
    sal_Int32 nHeight = bPortrait ? rSize.Height() : rSize.Width();
    for( const XclPaperSize* pEntry = pPaperSizeTable; pEntry != STATIC_TABLE_END( pPaperSizeTable ); ++pEntry )
    {
        if( (Abs( pEntry->mnWidth - nWidth ) < 80) && (Abs( pEntry->mnHeight - nHeight ) < 50) )
        {
            mnPaperSize = static_cast< sal_uInt16 >( pEntry - pPaperSizeTable );
            return;
        }
    }
}


// ============================================================================

