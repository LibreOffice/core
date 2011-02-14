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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <vector>

#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <tools/stream.hxx>
#include <svtools/rtfkeywd.hxx>
#include <svtools/rtfout.hxx>

#include <editeng/eeitem.hxx>
#include <svx/sdtaitm.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>

#include "cell.hxx"
#include "celltypes.hxx"
#include "svx/svdotable.hxx"
#include "svx/svdoutl.hxx"
#include "editeng/editeng.hxx"
#include "editeng/outlobj.hxx"

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
    sal_uLong Write();
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

sal_uLong SdrTableRtfExporter::Write()
{
    mrStrm << '{' << OOO_STRING_SVTOOLS_RTF_RTF;
    mrStrm << OOO_STRING_SVTOOLS_RTF_ANSI << RTFOutFuncs::sNewLine;

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

    mrStrm << OOO_STRING_SVTOOLS_RTF_TROWD << OOO_STRING_SVTOOLS_RTF_TRGAPH << "30" << OOO_STRING_SVTOOLS_RTF_TRLEFT << "-30";
    mrStrm << OOO_STRING_SVTOOLS_RTF_TRRH << ByteString::CreateFromInt32( nRowHeight ).GetBuffer();

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
            mrStrm << OOO_STRING_SVTOOLS_RTF_CLMGF; // The first cell in a range of table cells to be merged.

        SdrTextVertAdjust eVAdj = xCell->GetTextVerticalAdjust();
        switch( eVAdj )
        {
            case SVX_VER_JUSTIFY_TOP:       pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALT;   break;
            case SVX_VER_JUSTIFY_CENTER:    pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALC;   break;
            case SVX_VER_JUSTIFY_BOTTOM:    pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALB;   break;
            case SVX_VER_JUSTIFY_STANDARD:  pChar = OOO_STRING_SVTOOLS_RTF_CLVERTALB;   break;  //! Bottom
            default:                        pChar = NULL;           break;
        }
        if ( pChar )
            mrStrm << pChar;
*/
        mrStrm << OOO_STRING_SVTOOLS_RTF_CELLX << ByteString::CreateFromInt32( aColumnStart[nCol] ).GetBuffer();
        if ( (nCol & 0x0F) == 0x0F )
            mrStrm << RTFOutFuncs::sNewLine;        // Zeilen nicht zu lang werden lassen
    }
    mrStrm << OOO_STRING_SVTOOLS_RTF_PARD << OOO_STRING_SVTOOLS_RTF_PLAIN << OOO_STRING_SVTOOLS_RTF_INTBL << RTFOutFuncs::sNewLine;

    sal_uLong nStrmPos = mrStrm.Tell();
    for( sal_Int32 nCol = 0; nCol < nColCount; nCol++ )
    {
        WriteCell( nCol, nRow );
        if ( mrStrm.Tell() - nStrmPos > 255 )
        {
            mrStrm << RTFOutFuncs::sNewLine;
            nStrmPos = mrStrm.Tell();
        }
    }
    mrStrm << OOO_STRING_SVTOOLS_RTF_ROW << RTFOutFuncs::sNewLine;
}


void SdrTableRtfExporter::WriteCell( sal_Int32 nCol, sal_Int32 nRow )
{
    CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );

    if( !xCell.is() || xCell->isMerged() )
    {
        mrStrm << OOO_STRING_SVTOOLS_RTF_CELL;
        return ;
    }

    String aContent;

    OutlinerParaObject* pParaObj = xCell->GetEditOutlinerParaObject();
    bool bOwnParaObj = pParaObj != 0;

    if( pParaObj == 0 )
        pParaObj = xCell->GetOutlinerParaObject();

    if(pParaObj)
    {
        // handle outliner attributes
        SdrOutliner& rOutliner = mrObj.ImpGetDrawOutliner();
        rOutliner.SetText(*pParaObj);

        aContent = rOutliner.GetEditEngine().GetText( LINEEND_LF );

        rOutliner.Clear();

        if( bOwnParaObj )
            delete pParaObj;
    }

    bool bResetPar, bResetAttr;
    bResetPar = bResetAttr = sal_False;

    SdrTextHorzAdjust eHAdj = xCell->GetTextHorizontalAdjust();

    const SfxItemSet& rCellSet = xCell->GetItemSet();

    const SvxWeightItem&        rWeightItem     = (const SvxWeightItem&)    rCellSet.Get( EE_CHAR_WEIGHT );
    const SvxPostureItem&       rPostureItem    = (const SvxPostureItem&)   rCellSet.Get( EE_CHAR_ITALIC );
    const SvxUnderlineItem&     rUnderlineItem  = (const SvxUnderlineItem&) rCellSet.Get( EE_CHAR_UNDERLINE );

    const sal_Char* pChar;

    switch( eHAdj )
    {
        case SDRTEXTHORZADJUST_CENTER:  pChar = OOO_STRING_SVTOOLS_RTF_QC;  break;
        case SDRTEXTHORZADJUST_BLOCK:   pChar = OOO_STRING_SVTOOLS_RTF_QJ;  break;
        case SDRTEXTHORZADJUST_RIGHT:   pChar = OOO_STRING_SVTOOLS_RTF_QR;  break;
        case SDRTEXTHORZADJUST_LEFT:
        default:                        pChar = OOO_STRING_SVTOOLS_RTF_QL;  break;
    }
    mrStrm << pChar;

    if ( rWeightItem.GetWeight() >= WEIGHT_BOLD )
    {   // bold
        bResetAttr = true;
        mrStrm << OOO_STRING_SVTOOLS_RTF_B;
    }
    if ( rPostureItem.GetPosture() != ITALIC_NONE )
    {   // italic
        bResetAttr = true;
        mrStrm << OOO_STRING_SVTOOLS_RTF_I;
    }
    if ( rUnderlineItem.GetLineStyle() != UNDERLINE_NONE )
    {   // underline
        bResetAttr = true;
        mrStrm << OOO_STRING_SVTOOLS_RTF_UL;
    }

    mrStrm << ' ';
    RTFOutFuncs::Out_String( mrStrm, aContent );
    mrStrm << OOO_STRING_SVTOOLS_RTF_CELL;

    if ( bResetPar )
        mrStrm << OOO_STRING_SVTOOLS_RTF_PARD << OOO_STRING_SVTOOLS_RTF_INTBL;
    if ( bResetAttr )
        mrStrm << OOO_STRING_SVTOOLS_RTF_PLAIN;
}

} }

