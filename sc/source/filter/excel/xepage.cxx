/*************************************************************************
 *
 *  $RCSfile: xepage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:34:33 $
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

#ifndef SC_XEPAGE_HXX
#include "xepage.hxx"
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
#ifndef SC_STLPOOL_HXX
#include "stlpool.hxx"
#endif
#ifndef SC_STLSHEET_HXX
#include "stlsheet.hxx"
#endif
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
#endif

#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif


// Page settings records ======================================================

XclExpHeaderFooter::XclExpHeaderFooter( sal_uInt16 nRecId, const String& rHdrString ) :
    XclExpRecord( nRecId ),
    maHdrString( rHdrString )
{
}

void XclExpHeaderFooter::WriteBody( XclExpStream& rStrm )
{
    if( maHdrString.Len() )
    {
        XclExpString aExString;
        if( rStrm.GetRoot().GetBiff() < xlBiff8 )
            aExString.AssignByte( maHdrString, rStrm.GetRoot().GetCharSet(), EXC_STR_8BITLENGTH );
        else
            aExString.Assign( maHdrString, EXC_STR_DEFAULT, 255 );  // 16-bit length, but max 255 chars
        rStrm << aExString;
    }
}


// ----------------------------------------------------------------------------

XclExpSetup::XclExpSetup( const XclPageData& rPageData ) :
    XclExpRecord( EXC_ID_SETUP, 34 ),
    mrData( rPageData )
{
}

void XclExpSetup::WriteBody( XclExpStream& rStrm )
{
    XclBiff eBiff = rStrm.GetRoot().GetBiff();

    sal_uInt16 nFlags = 0;
    ::set_flag( nFlags, EXC_SETUP_INROWS,       mrData.mbPrintInRows );
    ::set_flag( nFlags, EXC_SETUP_PORTRAIT,     mrData.mbPortrait );
    ::set_flag( nFlags, EXC_SETUP_INVALID,      !mrData.mbValid );
    ::set_flag( nFlags, EXC_SETUP_BLACKWHITE,   mrData.mbBlackWhite );
    if( eBiff >= xlBiff5 )
    {
        ::set_flag( nFlags, EXC_SETUP_DRAFT,        mrData.mbDraftQuality );
        /*  Set the Comments/Notes to "At end of sheet" if Print Notes is true.
            We don't currently support "as displayed on sheet". Thus this value
            will be re-interpreted to "At end of sheet". */
        sal_uInt16 nNotes = EXC_SETUP_PRINTNOTES | EXC_SETUP_NOTES_END;
        ::set_flag( nFlags, nNotes,                 mrData.mbPrintNotes );
        ::set_flag( nFlags, EXC_SETUP_STARTPAGE,    mrData.mbManualStart );
    }

    rStrm   << mrData.mnPaperSize << mrData.mnScaling << mrData.mnStartPage
            << mrData.mnFitToWidth << mrData.mnFitToHeight << nFlags;
    if( eBiff >= xlBiff5 )
    {
        rStrm   << mrData.mnHorPrintRes << mrData.mnVerPrintRes
                << mrData.mfHeaderMargin << mrData.mfFooterMargin << mrData.mnCopies;
    }
}


// Manual page breaks ---------------------------------------------------------

XclExpPageBreaks::XclExpPageBreaks( sal_uInt16 nRecId, const ScfUInt16Vec& rPageBreaks, sal_uInt16 nMaxPos ) :
    XclExpRecord( nRecId ),
    mrPageBreaks( rPageBreaks ),
    mnMaxPos( nMaxPos )
{
}

void XclExpPageBreaks::Save( XclExpStream& rStrm )
{
    if( !mrPageBreaks.empty() )
    {
        SetRecSize( 2 + ((rStrm.GetRoot().GetBiff() < xlBiff8) ? 2 : 6) * mrPageBreaks.size() );
        XclExpRecord::Save( rStrm );
    }
}

void XclExpPageBreaks::WriteBody( XclExpStream& rStrm )
{
    bool bWriteRange = (rStrm.GetRoot().GetBiff() >= xlBiff8);

    rStrm << static_cast< sal_uInt16 >( mrPageBreaks.size() );
    for( ScfUInt16Vec::const_iterator aIt = mrPageBreaks.begin(), aEnd = mrPageBreaks.end(); aIt != aEnd; ++aIt )
    {
        rStrm << *aIt;
        if( bWriteRange )
            rStrm << sal_uInt16( 0 ) << mnMaxPos;
    }
}


// Background bitmap ----------------------------------------------------------

XclExpBitmap::XclExpBitmap( const Graphic& rGraphic ) :
    mrGraphic( rGraphic )
{
}

void XclExpBitmap::Save( XclExpStream& rStrm )
{
    Bitmap aBmp( mrGraphic.GetBitmap() );
    if( aBmp.GetBitCount() != 24 )
        aBmp.Convert( BMP_CONVERSION_24BIT );

    if( BitmapReadAccess* pAccess = aBmp.AcquireReadAccess() )
    {
        sal_Int32 nWidth = ::std::min< sal_Int32 >( pAccess->Width(), 0xFFFF );
        sal_Int32 nHeight = ::std::min< sal_Int32 >( pAccess->Height(), 0xFFFF );
        if( (0 < nWidth) && (nWidth <= 0xFFFF) && (0 < nHeight) && (nHeight <= 0xFFFF) )
        {
            sal_uInt8 nPadding = static_cast< sal_uInt8 >( nWidth & 0x03 );
            sal_uInt32 nTmpSize = (nWidth * 3UL + nPadding) * nHeight + 12;

            rStrm.StartRecord( EXC_ID_BITMAP, nTmpSize + 4 );

            rStrm   << EXC_BITMAP_UNKNOWNID
                    << nTmpSize                             // size after _this_ field
                    << sal_uInt32( 12 )                     // unknown
                    << static_cast< sal_uInt16 >( nWidth )  // width
                    << static_cast< sal_uInt16 >( nHeight ) // height
                    << sal_uInt16( 1 )                      // planes
                    << sal_uInt16( 24 );                    // bits per pixel

            for( sal_Int32 nY = nHeight - 1; nY >= 0; --nY )
            {
                for( sal_Int32 nX = 0; nX < nWidth; ++nX )
                {
                    const BitmapColor& rBmpColor = pAccess->GetPixel( nY, nX );
                    rStrm << rBmpColor.GetBlue() << rBmpColor.GetGreen() << rBmpColor.GetRed();
                }
                rStrm.WriteZeroBytes( nPadding );
            }

            rStrm.EndRecord();
        }
        aBmp.ReleaseAccess( pAccess );
    }
}


// Page settings ==============================================================

XclExpPageSettings::XclExpPageSettings( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
    ScDocument& rDoc = GetDoc();
    sal_uInt16 nScTab = GetScTab();

    if( SfxStyleSheetBase* pStyleSheet = GetStyleSheetPool().Find( rDoc.GetPageStyle( nScTab ), SFX_STYLE_FAMILY_PAGE ) )
    {
        const SfxItemSet& rItemSet = pStyleSheet->GetItemSet();
        maData.mbValid = true;

        // *** page settings ***

        maData.mbPrintInRows   = !GETITEMVALUE( rItemSet, SfxBoolItem, ATTR_PAGE_TOPDOWN,   bool );
        maData.mbHorCenter     =  GETITEMVALUE( rItemSet, SfxBoolItem, ATTR_PAGE_HORCENTER, bool );
        maData.mbVerCenter     =  GETITEMVALUE( rItemSet, SfxBoolItem, ATTR_PAGE_VERCENTER, bool );
        maData.mbPrintHeadings =  GETITEMVALUE( rItemSet, SfxBoolItem, ATTR_PAGE_HEADERS,   bool );
        maData.mbPrintGrid     =  GETITEMVALUE( rItemSet, SfxBoolItem, ATTR_PAGE_GRID,      bool );
        maData.mbPrintNotes    =  GETITEMVALUE( rItemSet, SfxBoolItem, ATTR_PAGE_NOTES,     bool );

        maData.mnStartPage     = GETITEMVALUE( rItemSet, SfxUInt16Item, ATTR_PAGE_FIRSTPAGENO, sal_uInt16 );
        maData.mbManualStart   = maData.mnStartPage && (!nScTab || rDoc.NeedPageResetAfterTab( nScTab - 1 ));

        const SvxLRSpaceItem& rLRItem = GETITEM( rItemSet, SvxLRSpaceItem, ATTR_LRSPACE );
        maData.mfLeftMargin    = XclTools::GetInchFromTwips( rLRItem.GetLeft() );
        maData.mfRightMargin   = XclTools::GetInchFromTwips( rLRItem.GetRight() );
        const SvxULSpaceItem& rULItem = GETITEM( rItemSet, SvxULSpaceItem, ATTR_ULSPACE );
        maData.mfTopMargin     = XclTools::GetInchFromTwips( rULItem.GetUpper() );
        maData.mfBottomMargin  = XclTools::GetInchFromTwips( rULItem.GetLower() );

        const SvxPageItem& rPageItem = GETITEM( rItemSet, SvxPageItem, ATTR_PAGE );
        const SvxSizeItem& rSizeItem = GETITEM( rItemSet, SvxSizeItem, ATTR_PAGE_SIZE );
        maData.SetScPaperSize( rSizeItem.GetSize(), !rPageItem.IsLandscape() );

        maData.mbFitToPages = ScfTools::CheckItem( rItemSet, ATTR_PAGE_SCALETOPAGES, false );
        if( maData.mbFitToPages )
            maData.mnFitToHeight = GETITEMVALUE( rItemSet, SfxUInt16Item, ATTR_PAGE_SCALETOPAGES, sal_uInt16 );
        else
            maData.mnScaling = GETITEMVALUE( rItemSet, SfxUInt16Item, ATTR_PAGE_SCALE, sal_uInt16 );

        maData.mpBrushItem.reset( new SvxBrushItem( GETITEM( rItemSet, SvxBrushItem, ATTR_BACKGROUND ) ) );

        // *** header and footer ***

        XclExpHFConverter aHFConv( GetRoot() );

        // header
        const SfxItemSet& rHdrItemSet = GETITEM( rItemSet, SvxSetItem, ATTR_PAGE_HEADERSET ).GetItemSet();
        if( GETITEMVALUE( rHdrItemSet, SfxBoolItem, ATTR_PAGE_ON, bool ) )
        {
            const ScPageHFItem& rHFItem = GETITEM( rItemSet, ScPageHFItem, ATTR_PAGE_HEADERRIGHT );
            maData.maHeader = aHFConv.GenerateString( rHFItem.GetLeftArea(), rHFItem.GetCenterArea(), rHFItem.GetRightArea() );
        }
        maData.mfHeaderMargin = XclTools::GetInchFromTwips( GETITEM( rHdrItemSet, SvxULSpaceItem, ATTR_ULSPACE ).GetLower() );

        // footer
        const SfxItemSet& rFtrItemSet = GETITEM( rItemSet, SvxSetItem, ATTR_PAGE_FOOTERSET ).GetItemSet();
        if( GETITEMVALUE( rFtrItemSet, SfxBoolItem, ATTR_PAGE_ON, bool ) )
        {
            const ScPageHFItem& rHFItem = GETITEM( rItemSet, ScPageHFItem, ATTR_PAGE_FOOTERRIGHT );
            maData.maFooter = aHFConv.GenerateString( rHFItem.GetLeftArea(), rHFItem.GetCenterArea(), rHFItem.GetRightArea() );
        }
        maData.mfFooterMargin = XclTools::GetInchFromTwips( GETITEM( rFtrItemSet, SvxULSpaceItem, ATTR_ULSPACE ).GetUpper() );
    }

    // *** page breaks ***

    for( sal_uInt16 nRow = 1; nRow <= MAXROW; ++nRow )
        if( rDoc.GetRowFlags( nRow, nScTab ) & CR_MANUALBREAK )
            maData.maHorPageBreaks.push_back( nRow );
    for( sal_uInt16 nCol = 1; nCol <= MAXCOL; ++nCol )
        if( rDoc.GetColFlags( nCol, nScTab ) & CR_MANUALBREAK )
            maData.maVerPageBreaks.push_back( nCol );
}

void XclExpPageSettings::Save( XclExpStream& rStrm )
{
    XclExpBoolRecord( EXC_ID_PRINTHEADERS, maData.mbPrintHeadings ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_PRINTGRIDLINES, maData.mbPrintGrid ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_GRIDSET, true ).Save( rStrm );
    XclExpPageBreaks( EXC_ID_HORPAGEBREAKS, maData.maHorPageBreaks, GetXclMaxPos().Col() ).Save( rStrm );
    XclExpPageBreaks( EXC_ID_VERPAGEBREAKS, maData.maVerPageBreaks, GetXclMaxPos().Row() ).Save( rStrm );
    XclExpHeaderFooter( EXC_ID_HEADER, maData.maHeader ).Save( rStrm );
    XclExpHeaderFooter( EXC_ID_FOOTER, maData.maFooter ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_HCENTER, maData.mbHorCenter ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_VCENTER, maData.mbVerCenter ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_LEFTMARGIN, maData.mfLeftMargin ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_RIGHTMARGIN, maData.mfRightMargin ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_TOPMARGIN, maData.mfTopMargin ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_BOTTOMMARGIN, maData.mfBottomMargin ).Save( rStrm );
    XclExpSetup( maData ).Save( rStrm );

    if( (GetBiff() >= xlBiff8) && maData.mpBrushItem.get() )
        if( const Graphic* pGraphic = maData.mpBrushItem->GetGraphic() )
            XclExpBitmap( *pGraphic ).Save( rStrm );
}


// ============================================================================

