/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xepage.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:32:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

// Header/footer --------------------------------------------------------------

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
        if( rStrm.GetRoot().GetBiff() <= EXC_BIFF5 )
            aExString.AssignByte( maHdrString, rStrm.GetRoot().GetCharSet(), EXC_STR_8BITLENGTH );
        else
            aExString.Assign( maHdrString, EXC_STR_DEFAULT, 255 );  // 16-bit length, but max 255 chars
        rStrm << aExString;
    }
}

// General page settings ------------------------------------------------------

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
    if( eBiff >= EXC_BIFF5 )
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
    if( eBiff >= EXC_BIFF5 )
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
        SetRecSize( 2 + ((rStrm.GetRoot().GetBiff() <= EXC_BIFF5) ? 2 : 6) * mrPageBreaks.size() );
        XclExpRecord::Save( rStrm );
    }
}

void XclExpPageBreaks::WriteBody( XclExpStream& rStrm )
{
    bool bWriteRange = (rStrm.GetRoot().GetBiff() == EXC_BIFF8);

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
        if( (nWidth > 0) && (nHeight > 0) )
        {
            sal_uInt8 nPadding = static_cast< sal_uInt8 >( nWidth & 0x03 );
            sal_uInt32 nTmpSize = static_cast< sal_uInt32 >( (nWidth * 3 + nPadding) * nHeight + 12 );

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
    SCTAB nScTab = GetCurrScTab();

    if( SfxStyleSheetBase* pStyleSheet = GetStyleSheetPool().Find( rDoc.GetPageStyle( nScTab ), SFX_STYLE_FAMILY_PAGE ) )
    {
        const SfxItemSet& rItemSet = pStyleSheet->GetItemSet();
        maData.mbValid = true;

        // *** page settings ***

        maData.mbPrintInRows   = !GETITEMBOOL( rItemSet, ATTR_PAGE_TOPDOWN   );
        maData.mbHorCenter     =  GETITEMBOOL( rItemSet, ATTR_PAGE_HORCENTER );
        maData.mbVerCenter     =  GETITEMBOOL( rItemSet, ATTR_PAGE_VERCENTER );
        maData.mbPrintHeadings =  GETITEMBOOL( rItemSet, ATTR_PAGE_HEADERS   );
        maData.mbPrintGrid     =  GETITEMBOOL( rItemSet, ATTR_PAGE_GRID      );
        maData.mbPrintNotes    =  GETITEMBOOL( rItemSet, ATTR_PAGE_NOTES     );

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

        const ScPageScaleToItem& rScaleToItem = GETITEM( rItemSet, ScPageScaleToItem, ATTR_PAGE_SCALETO );
        sal_uInt16 nPages = GETITEMVALUE( rItemSet, SfxUInt16Item, ATTR_PAGE_SCALETOPAGES, sal_uInt16 );
        sal_uInt16 nScale = GETITEMVALUE( rItemSet, SfxUInt16Item, ATTR_PAGE_SCALE, sal_uInt16 );

        if( ScfTools::CheckItem( rItemSet, ATTR_PAGE_SCALETO, false ) && rScaleToItem.IsValid() )
        {
            maData.mnFitToWidth = rScaleToItem.GetWidth();
            maData.mnFitToHeight = rScaleToItem.GetHeight();
            maData.mbFitToPages = true;

        }
        else if( ScfTools::CheckItem( rItemSet, ATTR_PAGE_SCALETOPAGES, false ) && nPages )
        {
            maData.mnFitToWidth = 1;
            maData.mnFitToHeight = nPages;
            maData.mbFitToPages = true;
        }
        else if( nScale )
        {
            maData.mnScaling = nScale;
            maData.mbFitToPages = false;
        }

        maData.mxBrushItem.reset( new SvxBrushItem( GETITEM( rItemSet, SvxBrushItem, ATTR_BACKGROUND ) ) );

        // *** header and footer ***

        XclExpHFConverter aHFConv( GetRoot() );

        // header
        const SfxItemSet& rHdrItemSet = GETITEM( rItemSet, SvxSetItem, ATTR_PAGE_HEADERSET ).GetItemSet();
        if( GETITEMBOOL( rHdrItemSet, ATTR_PAGE_ON ) )
        {
            const ScPageHFItem& rHFItem = GETITEM( rItemSet, ScPageHFItem, ATTR_PAGE_HEADERRIGHT );
            aHFConv.GenerateString( rHFItem.GetLeftArea(), rHFItem.GetCenterArea(), rHFItem.GetRightArea() );
            maData.maHeader = aHFConv.GetHFString();
            // header height (Excel excludes header from top margin)
            sal_Int32 nHdrHeight = GETITEMBOOL( rHdrItemSet, ATTR_PAGE_DYNAMIC ) ?
                // dynamic height: calculate header height, add header <-> sheet area distance
                (aHFConv.GetTotalHeight() + GETITEM( rHdrItemSet, SvxULSpaceItem, ATTR_ULSPACE ).GetLower()) :
                // static height: ATTR_PAGE_SIZE already includes header <-> sheet area distance
                static_cast< sal_Int32 >( GETITEM( rHdrItemSet, SvxSizeItem, ATTR_PAGE_SIZE ).GetSize().Height() );
            maData.mfHeaderMargin = maData.mfTopMargin;
            maData.mfTopMargin += XclTools::GetInchFromTwips( nHdrHeight );
        }

        // footer
        const SfxItemSet& rFtrItemSet = GETITEM( rItemSet, SvxSetItem, ATTR_PAGE_FOOTERSET ).GetItemSet();
        if( GETITEMBOOL( rFtrItemSet, ATTR_PAGE_ON ) )
        {
            const ScPageHFItem& rHFItem = GETITEM( rItemSet, ScPageHFItem, ATTR_PAGE_FOOTERRIGHT );
            aHFConv.GenerateString( rHFItem.GetLeftArea(), rHFItem.GetCenterArea(), rHFItem.GetRightArea() );
            maData.maFooter = aHFConv.GetHFString();
            // footer height (Excel excludes footer from bottom margin)
            sal_Int32 nFtrHeight = GETITEMBOOL( rFtrItemSet, ATTR_PAGE_DYNAMIC ) ?
                // dynamic height: calculate footer height, add sheet area <-> footer distance
                (aHFConv.GetTotalHeight() + GETITEM( rFtrItemSet, SvxULSpaceItem, ATTR_ULSPACE ).GetUpper()) :
                // static height: ATTR_PAGE_SIZE already includes sheet area <-> footer distance
                static_cast< sal_Int32 >( GETITEM( rFtrItemSet, SvxSizeItem, ATTR_PAGE_SIZE ).GetSize().Height() );
            maData.mfFooterMargin = maData.mfBottomMargin;
            maData.mfBottomMargin += XclTools::GetInchFromTwips( nFtrHeight );
        }
    }

    // *** page breaks ***

    ScCompressedArrayIterator< SCROW, BYTE> aIter( rDoc.GetRowFlagsArray( nScTab), 1, GetMaxPos().Row());
    do
    {
        if (*aIter & CR_MANUALBREAK)
            for (SCROW j=aIter.GetRangeStart(); j<=aIter.GetRangeEnd(); ++j)
                maData.maHorPageBreaks.push_back( static_cast< sal_uInt16 >( j ) );
    } while (aIter.NextRange());

    for( SCCOL nScCol = 1, nScMaxCol = GetMaxPos().Col(); nScCol <= nScMaxCol; ++nScCol )
        if( rDoc.GetColFlags( nScCol, nScTab ) & CR_MANUALBREAK )
            maData.maVerPageBreaks.push_back( static_cast< sal_uInt16 >( nScCol ) );
}

void XclExpPageSettings::Save( XclExpStream& rStrm )
{
    XclExpBoolRecord( EXC_ID_PRINTHEADERS, maData.mbPrintHeadings ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_PRINTGRIDLINES, maData.mbPrintGrid ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_GRIDSET, true ).Save( rStrm );
    XclExpPageBreaks( EXC_ID_HORPAGEBREAKS, maData.maHorPageBreaks, static_cast< sal_uInt16 >( GetXclMaxPos().Col() ) ).Save( rStrm );
    XclExpPageBreaks( EXC_ID_VERPAGEBREAKS, maData.maVerPageBreaks, static_cast< sal_uInt16 >( GetXclMaxPos().Row() ) ).Save( rStrm );
    XclExpHeaderFooter( EXC_ID_HEADER, maData.maHeader ).Save( rStrm );
    XclExpHeaderFooter( EXC_ID_FOOTER, maData.maFooter ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_HCENTER, maData.mbHorCenter ).Save( rStrm );
    XclExpBoolRecord( EXC_ID_VCENTER, maData.mbVerCenter ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_LEFTMARGIN, maData.mfLeftMargin ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_RIGHTMARGIN, maData.mfRightMargin ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_TOPMARGIN, maData.mfTopMargin ).Save( rStrm );
    XclExpDoubleRecord( EXC_ID_BOTTOMMARGIN, maData.mfBottomMargin ).Save( rStrm );
    XclExpSetup( maData ).Save( rStrm );

    if( (GetBiff() == EXC_BIFF8) && maData.mxBrushItem.get() )
        if( const Graphic* pGraphic = maData.mxBrushItem->GetGraphic() )
            XclExpBitmap( *pGraphic ).Save( rStrm );
}

// ============================================================================

