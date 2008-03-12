/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablertfexporter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:06:33 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <vector>

#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <tools/stream.hxx>
#include <svtools/rtfkeywd.hxx>
#include <svtools/rtfout.hxx>

#include <svx/eeitem.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/wghtitem.hxx>
#include <svx/postitem.hxx>
#include <svx/udlnitem.hxx>

#include "cell.hxx"
#include "celltypes.hxx"
#include "svx/svdotable.hxx"
#include "svx/svdoutl.hxx"
#include "svx/editeng.hxx"

//#include <tablertfexporter.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

namespace sdr { namespace table {

class SdrTableRtfExporter
{
public:
    SdrTableRtfExporter( SvStream& rStrmP, SdrTableObj& rObj );
    ULONG Write();
    void WriteRow( const Reference< XPropertySet >& xRowSet, sal_Int32 nRow, const std::vector< sal_Int32 >& aColumnStart );
    void WriteCell( sal_Int32 nCol, sal_Int32 nRow );

private:
    SvStream& mrStrm;
    SdrTableObj& mrObj;
    Reference< XTable > mxTable;
    const OUString msSize;
};

void SdrTableObj::ExportAsRTF( SvStream& rStrm, SdrTableObj& rObj )
{
    SdrTableRtfExporter aEx( rStrm, rObj );
    aEx.Write();
}

SdrTableRtfExporter::SdrTableRtfExporter( SvStream& rStrm, SdrTableObj& rObj )
: mrStrm( rStrm )
, mrObj( rObj )
, mxTable( rObj.getTable() )
, msSize( RTL_CONSTASCII_USTRINGPARAM("Size") )
{
}

long HundMMToTwips( long nIn )
{
    long nRet = OutputDevice::LogicToLogic( nIn, MAP_100TH_MM, MAP_TWIP );
    return nRet;
}

ULONG SdrTableRtfExporter::Write()
{
    mrStrm << '{' << sRTF_RTF;
    mrStrm << sRTF_ANSI << RTFOutFuncs::sNewLine;

    Reference< XTableColumns > xColumns( mxTable->getColumns() );
    const sal_Int32 nColCount = xColumns->getCount();

    std::vector< sal_Int32 > aColumnStart;
    aColumnStart.reserve( nColCount );

    // determine right offset of cells
    sal_Int32 nPos = 0;
    for( sal_Int32 nCol = 0; nCol < nColCount; nCol++ ) try
    {
        Reference< XPropertySet > xSet( xColumns->getByIndex(nCol), UNO_QUERY_THROW );
        sal_Int32 nWidth = 0;
        xSet->getPropertyValue( msSize ) >>= nWidth;
        nPos += HundMMToTwips( nWidth );
        aColumnStart.push_back( nPos );
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR("SdrTableRtfExporter::Write(), exception caught!");
    }

    // export rows
    Reference< XTableRows > xRows( mxTable->getRows() );
    const sal_Int32 nRowCount = xRows->getCount();

    for( sal_Int32 nRow = 0; nRow < nRowCount; nRow++ ) try
    {
        Reference< XPropertySet > xRowSet( xRows->getByIndex(nRow), UNO_QUERY_THROW );
        WriteRow( xRowSet, nRow, aColumnStart );
    }
    catch( Exception& e )
    {
        (void)e;
        DBG_ERROR("SdrTableRtfExporter::Write(), exception caught!");
    }

    mrStrm << '}' << RTFOutFuncs::sNewLine;
    return mrStrm.GetError();
}

void SdrTableRtfExporter::WriteRow( const Reference< XPropertySet >& xRowSet, sal_Int32 nRow, const std::vector< sal_Int32 >& aColumnStart )
{
    sal_Int32 nRowHeight = 0;
    xRowSet->getPropertyValue( msSize ) >>= nRowHeight;

    mrStrm << sRTF_TROWD << sRTF_TRGAPH << "30" << sRTF_TRLEFT << "-30";
    mrStrm << sRTF_TRRH << ByteString::CreateFromInt32( nRowHeight ).GetBuffer();

    const sal_Int32 nColCount = mxTable->getColumnCount();
    for( sal_Int32 nCol = 0; nCol < nColCount; nCol++ )
    {
        CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );

        if( !xCell.is() )
            continue;

/*
        const sal_Bool bIsMerged = xCell->isMerged();
        const sal_Int32 nRowSpan = xCell->getRowSpan();
        const sal_Int32 nColSpan = xCell->getColumnSpan();

        const sal_Char* pChar;

        if( !bIsMerged && ((nRowSpan > 1) || (nColSpan > 1)) )
            mrStrm << sRTF_CLMGF; // The first cell in a range of table cells to be merged.

        SdrTextVertAdjust eVAdj = xCell->GetTextVerticalAdjust();
        switch( eVAdj )
        {
            case SVX_VER_JUSTIFY_TOP:       pChar = sRTF_CLVERTALT; break;
            case SVX_VER_JUSTIFY_CENTER:    pChar = sRTF_CLVERTALC; break;
            case SVX_VER_JUSTIFY_BOTTOM:    pChar = sRTF_CLVERTALB; break;
            case SVX_VER_JUSTIFY_STANDARD:  pChar = sRTF_CLVERTALB; break;  //! Bottom
            default:                        pChar = NULL;           break;
        }
        if ( pChar )
            mrStrm << pChar;
*/
        mrStrm << sRTF_CELLX << ByteString::CreateFromInt32( aColumnStart[nCol] ).GetBuffer();
        if ( (nCol & 0x0F) == 0x0F )
            mrStrm << RTFOutFuncs::sNewLine;        // Zeilen nicht zu lang werden lassen
    }
    mrStrm << sRTF_PARD << sRTF_PLAIN << sRTF_INTBL << RTFOutFuncs::sNewLine;

    ULONG nStrmPos = mrStrm.Tell();
    for( sal_Int32 nCol = 0; nCol < nColCount; nCol++ )
    {
        WriteCell( nCol, nRow );
        if ( mrStrm.Tell() - nStrmPos > 255 )
        {
            mrStrm << RTFOutFuncs::sNewLine;
            nStrmPos = mrStrm.Tell();
        }
    }
    mrStrm << sRTF_ROW << RTFOutFuncs::sNewLine;
}


void SdrTableRtfExporter::WriteCell( sal_Int32 nCol, sal_Int32 nRow )
{
    CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );

    if( !xCell.is() || xCell->isMerged() )
    {
        mrStrm << sRTF_CELL;
        return ;
    }

    String aContent;

    OutlinerParaObject* pParaObj = xCell->GetEditOutlinerParaObject();

    if( pParaObj == 0 )
        pParaObj = xCell->GetOutlinerParaObject();

    if(pParaObj)
    {
        // handle outliner attributes
        SdrOutliner& rOutliner = mrObj.ImpGetDrawOutliner();
        rOutliner.SetText(*pParaObj);

        aContent = rOutliner.GetEditEngine().GetText( LINEEND_LF );

        rOutliner.Clear();
    }

    bool bResetPar, bResetAttr;
    bResetPar = bResetAttr = FALSE;

    SdrTextHorzAdjust eHAdj = xCell->GetTextHorizontalAdjust();

    const SfxItemSet& rCellSet = xCell->GetItemSet();

    const SvxWeightItem&        rWeightItem     = (const SvxWeightItem&)    rCellSet.Get( EE_CHAR_WEIGHT );
    const SvxPostureItem&       rPostureItem    = (const SvxPostureItem&)   rCellSet.Get( EE_CHAR_ITALIC );
    const SvxUnderlineItem&     rUnderlineItem  = (const SvxUnderlineItem&) rCellSet.Get( EE_CHAR_UNDERLINE );

    const sal_Char* pChar;

    switch( eHAdj )
    {
        case SDRTEXTHORZADJUST_CENTER:  pChar = sRTF_QC;    break;
        case SDRTEXTHORZADJUST_BLOCK:   pChar = sRTF_QJ;    break;
        case SDRTEXTHORZADJUST_RIGHT:   pChar = sRTF_QR;    break;
        case SDRTEXTHORZADJUST_LEFT:
        default:                        pChar = sRTF_QL;    break;
    }
    mrStrm << pChar;

    if ( rWeightItem.GetWeight() >= WEIGHT_BOLD )
    {   // bold
        bResetAttr = true;
        mrStrm << sRTF_B;
    }
    if ( rPostureItem.GetPosture() != ITALIC_NONE )
    {   // italic
        bResetAttr = true;
        mrStrm << sRTF_I;
    }
    if ( rUnderlineItem.GetUnderline() != UNDERLINE_NONE )
    {   // underline
        bResetAttr = true;
        mrStrm << sRTF_UL;
    }

    mrStrm << ' ';
    RTFOutFuncs::Out_String( mrStrm, aContent );
    mrStrm << sRTF_CELL;

    if ( bResetPar )
        mrStrm << sRTF_PARD << sRTF_INTBL;
    if ( bResetAttr )
        mrStrm << sRTF_PLAIN;
}

} }

