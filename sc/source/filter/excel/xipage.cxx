/*************************************************************************
 *
 *  $RCSfile: xipage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:35:58 $
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
#include "xipage.hxx"
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SVX_PAGEITEM_HXX
#include <svx/pageitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_STLSHEET_HXX
#include "stlsheet.hxx"
#endif

#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif
#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif


#include "namebuff.hxx"


// Page settings ==============================================================

XclImpPageSettings::XclImpPageSettings( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mbValidPaper( false )
{
}

void XclImpPageSettings::ReadSetup( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() >= xlBiff4 );
    if( GetBiff() < xlBiff4 )
        return;

    // BIFF4 - BIFF8
    sal_uInt16 nFlags;
    rStrm   >> maData.mnPaperSize >> maData.mnScaling >> maData.mnStartPage
            >> maData.mnFitToWidth >> maData.mnFitToHeight >> nFlags;

    mbValidPaper = maData.mbValid = !::get_flag( nFlags, EXC_SETUP_INVALID );
    maData.mbPrintInRows = ::get_flag( nFlags, EXC_SETUP_INROWS );
    maData.mbPortrait = ::get_flag( nFlags, EXC_SETUP_PORTRAIT );
    maData.mbBlackWhite = ::get_flag( nFlags, EXC_SETUP_BLACKWHITE );
    maData.mbManualStart = true;

    // new in BIFF5 - BIFF8
    if( GetBiff() >= xlBiff5 )
    {
        rStrm   >> maData.mnHorPrintRes >> maData.mnVerPrintRes
                >> maData.mfHeaderMargin >> maData.mfFooterMargin >> maData.mnCopies;

        maData.mbDraftQuality = ::get_flag( nFlags, EXC_SETUP_DRAFT );
        maData.mbPrintNotes = ::get_flag( nFlags, EXC_SETUP_PRINTNOTES );
        maData.mbManualStart = ::get_flag( nFlags, EXC_SETUP_STARTPAGE );
    }
    GetTracer().TracePrintFitToPages(maData.mnFitToWidth);
}

void XclImpPageSettings::ReadMargin( XclImpStream& rStrm )
{
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_LEFTMARGIN:     rStrm >> maData.mfLeftMargin;   break;
        case EXC_ID_RIGHTMARGIN:    rStrm >> maData.mfRightMargin;  break;
        case EXC_ID_TOPMARGIN:      rStrm >> maData.mfTopMargin;    break;
        case EXC_ID_BOTTOMMARGIN:   rStrm >> maData.mfBottomMargin; break;
        default:    DBG_ERRORFILE( "XclImpPageSettings::ReadMargin - unknown record" );
    }
}

void XclImpPageSettings::ReadCenter( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() >= xlBiff3 );    // read it anyway
    bool bCenter = (rStrm.ReaduInt16() != 0);
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_HCENTER:    maData.mbHorCenter = bCenter;   break;
        case EXC_ID_VCENTER:    maData.mbVerCenter = bCenter;   break;
        default:    DBG_ERRORFILE( "XclImpPageSettings::ReadCenter - unknown record" );
    }
}

void XclImpPageSettings::ReadHeaderFooter( XclImpStream& rStrm )
{
    String aString;
    if( rStrm.GetRecLeft() )
    {
        if( GetBiff() < xlBiff8 )
            rStrm.AppendByteString( aString, false );
        else
            rStrm.AppendUniString( aString );
    }

    switch( rStrm.GetRecId() )
    {
        case EXC_ID_HEADER:     maData.maHeader = aString;  break;
        case EXC_ID_FOOTER:     maData.maFooter = aString;  break;
        default:    DBG_ERRORFILE( "XclImpPageSettings::ReadHeaderFooter - unknown record" );
    }
}

void XclImpPageSettings::ReadPageBreaks( XclImpStream& rStrm )
{
    ScfUInt16Vec* pVec = NULL;
    switch( rStrm.GetRecId() )
    {
        case EXC_ID_HORPAGEBREAKS:  pVec = &maData.maHorPageBreaks;     break;
        case EXC_ID_VERPAGEBREAKS:  pVec = &maData.maVerPageBreaks;     break;
        default:    DBG_ERRORFILE( "XclImpPageSettings::ReadPageBreaks - unknown record" );
    }

    if( pVec )
    {
        bool bIgnore = (GetBiff() >= xlBiff8);  // ignore start/end columns or rows in BIFF8

        sal_uInt16 nCount, nBreak;
        rStrm >> nCount;
        pVec->clear();
        pVec->reserve( nCount );

        while( nCount-- )
        {
            rStrm >> nBreak;
            if( nBreak )
                pVec->push_back( nBreak );
            if( bIgnore )
                rStrm.Ignore( 4 );
        }
    }
}

void XclImpPageSettings::ReadPrintheaders( XclImpStream& rStrm )
{
    maData.mbPrintHeadings = (rStrm.ReaduInt16() != 0);
}

void XclImpPageSettings::ReadPrintgridlines( XclImpStream& rStrm )
{
    maData.mbPrintGrid = (rStrm.ReaduInt16() != 0);
}

void XclImpPageSettings::ReadBitmap( XclImpStream& rStrm )
{
    sal_uInt32 nID;
    sal_uInt16 nWidth, nHeight, nPlanes, nDepth;

    rStrm >> nID;
    rStrm.Ignore( 8 );
    rStrm >> nWidth >> nHeight >> nPlanes >> nDepth;

    DBG_ASSERT( nID == EXC_BITMAP_UNKNOWNID, "XclImpPageSettings::ReadBitmap - wrong ID" );
    DBG_ASSERT( nDepth == 24, "XclImpPageSettings::ReadBitmap - wrong depth" );
    DBG_ASSERT( nPlanes == 1, "XclImpPageSettings::ReadBitmap - wrong plane count" );
    if( rStrm.IsValid() && (nID == EXC_BITMAP_UNKNOWNID) && (nDepth == 24) && (nPlanes == 1) )
    {
        sal_uInt32 nPadding = nWidth % 4;
        if( rStrm.GetRecLeft() == (nWidth * 3UL + nPadding) * nHeight )
        {
            sal_Int32 nVclWidth = nWidth;
            sal_Int32 nVclHeight = nHeight;
            Bitmap aBmp( Size( nVclWidth, nVclHeight ), nDepth );
            BitmapWriteAccess* pAccess = aBmp.AcquireWriteAccess();
            if( pAccess )
            {
                sal_uInt8 nBlue, nGreen, nRed;
                for( sal_Int32 nY = nVclHeight - 1; nY >= 0; --nY )
                {
                    for( sal_Int32 nX = 0; nX < nVclWidth; ++nX )
                    {
                        rStrm >> nBlue >> nGreen >> nRed;
                        pAccess->SetPixel( nY, nX, BitmapColor( nRed, nGreen, nBlue ) );
                    }
                    rStrm.Ignore( nPadding );
                }

                aBmp.ReleaseAccess( pAccess );
                maData.mpBrushItem.reset( new SvxBrushItem( Graphic( aBmp ), GPOS_TILED ) );
            }
        }
        else
            DBG_ERRORFILE( "XclImpPageSettings::ReadBitmap - record size invalid" );
    }
}

void XclImpPageSettings::SetPaperSize( sal_uInt16 nXclPaperSize, bool bPortrait )
{
    maData.mnPaperSize = nXclPaperSize;
    maData.mbPortrait = bPortrait;
    mbValidPaper = true;
}


// ----------------------------------------------------------------------------

namespace {

void lclPutMarginItem( SfxItemSet& rItemSet, sal_uInt16 nRecId, double fMarginInch )
{
    sal_uInt16 nMarginTwips = XclTools::GetTwipsFromInch( fMarginInch );
    if( (nRecId == EXC_ID_TOPMARGIN) || (nRecId == EXC_ID_BOTTOMMARGIN) )
    {
        SvxULSpaceItem aItem( GETITEM( rItemSet, SvxULSpaceItem, ATTR_ULSPACE ) );
        if( nRecId == EXC_ID_TOPMARGIN )
            aItem.SetUpperValue( nMarginTwips );
        else
            aItem.SetLowerValue( nMarginTwips );
        rItemSet.Put( aItem );
    }
    else if( (nRecId == EXC_ID_LEFTMARGIN) || (nRecId == EXC_ID_RIGHTMARGIN) )
    {
        SvxLRSpaceItem aItem( GETITEM( rItemSet, SvxLRSpaceItem, ATTR_LRSPACE ) );
        if( nRecId == EXC_ID_LEFTMARGIN )
            aItem.SetLeftValue( nMarginTwips );
        else
            aItem.SetRightValue( nMarginTwips );
        rItemSet.Put( aItem );
    }
    else
        DBG_ERRORFILE( "XclImpPageSettings::SetMarginItem - unknown record id" );
}

} // namespace

void XclImpPageSettings::CreatePageStyle()
{
    ScDocument& rDoc = GetDoc();
    sal_uInt16 nScTab = GetScTab();

    // *** create page style sheet ***

    String aStyleName( RTL_CONSTASCII_USTRINGPARAM( "PageStyle_" ) );
    if( const String* pTableName = mpRD->pTabNameBuff->Get( nScTab ) )
        aStyleName.Append( *pTableName );
    else
        aStyleName.Append( String::CreateFromInt32( nScTab + 1 ) );

    ScStyleSheet& rStyleSheet = ScfTools::MakePageStyleSheet( GetStyleSheetPool(), aStyleName, false );
    SfxItemSet& rItemSet = rStyleSheet.GetItemSet();

    // *** page settings ***

    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_TOPDOWN,    !maData.mbPrintInRows ),    true );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_HORCENTER,  maData.mbHorCenter ),       true );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_VERCENTER,  maData.mbVerCenter ),       true );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_HEADERS,    maData.mbPrintHeadings ),   true );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_GRID,       maData.mbPrintGrid ),       true );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_PAGE_NOTES,      maData.mbPrintNotes ),      true );

    sal_uInt16 nStartPage = maData.mbManualStart ? maData.mnStartPage : 0;
    ScfTools::PutItem( rItemSet, SfxUInt16Item( ATTR_PAGE_FIRSTPAGENO, nStartPage ), true );

    lclPutMarginItem( rItemSet, EXC_ID_LEFTMARGIN,   maData.mfLeftMargin );
    lclPutMarginItem( rItemSet, EXC_ID_RIGHTMARGIN,  maData.mfRightMargin );
    lclPutMarginItem( rItemSet, EXC_ID_TOPMARGIN,    maData.mfTopMargin );
    lclPutMarginItem( rItemSet, EXC_ID_BOTTOMMARGIN, maData.mfBottomMargin );

    if( maData.mpBrushItem.get() )
        rItemSet.Put( *maData.mpBrushItem );

    if( mbValidPaper )
    {
        SvxPageItem aPageItem( GETITEM( rItemSet, SvxPageItem, ATTR_PAGE ) );
        aPageItem.SetLandscape( !maData.mbPortrait );
        rItemSet.Put( aPageItem );
        ScfTools::PutItem( rItemSet, SvxSizeItem( ATTR_PAGE_SIZE, maData.GetScPaperSize( GetPrinter() ) ), true );
    }

    if( maData.mbValid )
        if( !maData.mbFitToPages || !maData.mnFitToWidth || !maData.mnFitToHeight )
            rItemSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, maData.mnScaling ) );

    if( maData.mbFitToPages )
        if( sal_uInt16 nPages = maData.mnFitToWidth * maData.mnFitToHeight )
            rItemSet.Put( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, nPages ) );

    // *** header and footer ***

    XclImpHFConverter aHFConv( GetRoot() );

    // header
    bool bHasHeader = (maData.maHeader.Len() != 0);
    SvxSetItem aHdrSetItem( GETITEM( rItemSet, SvxSetItem, ATTR_PAGE_HEADERSET ) );
    SfxItemSet& rHdrItemSet = aHdrSetItem.GetItemSet();
    rHdrItemSet.Put( SfxBoolItem( ATTR_PAGE_ON, bHasHeader ) );
    lclPutMarginItem( rHdrItemSet, EXC_ID_BOTTOMMARGIN, maData.mfHeaderMargin );
    rItemSet.Put( aHdrSetItem );

    if( bHasHeader )
    {
        aHFConv.ParseString( maData.maHeader );
        aHFConv.FillToItemSet( rItemSet, ATTR_PAGE_HEADERLEFT );
        aHFConv.FillToItemSet( rItemSet, ATTR_PAGE_HEADERRIGHT );
    }

    // footer
    bool bHasFooter = (maData.maFooter.Len() != 0);
    SvxSetItem aFtrSetItem( GETITEM( rItemSet, SvxSetItem, ATTR_PAGE_FOOTERSET ) );
    SfxItemSet& rFtrItemSet = aFtrSetItem.GetItemSet();
    rFtrItemSet.Put( SfxBoolItem( ATTR_PAGE_ON, bHasFooter ) );
    lclPutMarginItem( rFtrItemSet, EXC_ID_TOPMARGIN, maData.mfFooterMargin );
    rItemSet.Put( aFtrSetItem );

    if( bHasFooter )
    {
        aHFConv.ParseString( maData.maFooter );
        aHFConv.FillToItemSet( rItemSet, ATTR_PAGE_FOOTERLEFT );
        aHFConv.FillToItemSet( rItemSet, ATTR_PAGE_FOOTERRIGHT );
    }

    // *** put style sheet into document ***

    rDoc.SetPageStyle( nScTab, rStyleSheet.GetName() );

    // *** page breaks ***

    ScfUInt16Vec::const_iterator aIt, aEnd;

    for( aIt = maData.maHorPageBreaks.begin(), aEnd = maData.maHorPageBreaks.end(); (aIt != aEnd) && (*aIt <= MAXROW); ++aIt )
        rDoc.SetRowFlags( *aIt, nScTab, rDoc.GetRowFlags( *aIt, nScTab ) | CR_MANUALBREAK );
    for( aIt = maData.maVerPageBreaks.begin(), aEnd = maData.maVerPageBreaks.end(); (aIt != aEnd) && (*aIt <= MAXCOL); ++aIt )
        rDoc.SetColFlags( *aIt, nScTab, rDoc.GetColFlags( *aIt, nScTab ) | CR_MANUALBREAK );

    // set to defaults for next sheet
    maData.SetDefaults();
}


// ============================================================================

