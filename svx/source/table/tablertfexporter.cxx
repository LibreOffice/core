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


#include <vector>

#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <tools/diagnose_ex.h>
#include <tools/stream.hxx>
#include <tools/UnitConversion.hxx>
#include <svtools/rtfkeywd.hxx>
#include <svtools/rtfout.hxx>

#include <editeng/eeitem.hxx>
#include <svx/sdtaitm.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>

#include <cell.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outlobj.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

namespace sdr::table {

class SdrTableRtfExporter
{
public:
    SdrTableRtfExporter( SvStream& rStrmP, SdrTableObj& rObj );
    void Write();
    void WriteRow( const Reference< XPropertySet >& xRowSet, sal_Int32 nRow, const std::vector< sal_Int32 >& aColumnStart );
    void WriteCell( sal_Int32 nCol, sal_Int32 nRow );

private:
    SvStream& mrStrm;
    SdrTableObj& mrObj;
    Reference< XTable > mxTable;
};

void ExportAsRTF( SvStream& rStrm, SdrTableObj& rObj )
{
    SdrTableRtfExporter aEx( rStrm, rObj );
    aEx.Write();
}

constexpr OUStringLiteral gsSize( u"Size" );

SdrTableRtfExporter::SdrTableRtfExporter( SvStream& rStrm, SdrTableObj& rObj )
: mrStrm( rStrm )
, mrObj( rObj )
, mxTable( rObj.getTable() )
{
}

void SdrTableRtfExporter::Write()
{
    mrStrm.WriteChar( '{' ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_RTF );
    mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ANSI ).WriteCharPtr( SAL_NEWLINE_STRING );

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
        xSet->getPropertyValue( gsSize ) >>= nWidth;
        nPos += o3tl::toTwips(nWidth, o3tl::Length::mm100);
        aColumnStart.push_back( nPos );
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION("svx", "");
    }

    // export rows
    Reference< XTableRows > xRows( mxTable->getRows() );
    const sal_Int32 nRowCount = xRows->getCount();

    for( sal_Int32 nRow = 0; nRow < nRowCount; nRow++ ) try
    {
        Reference< XPropertySet > xRowSet( xRows->getByIndex(nRow), UNO_QUERY_THROW );
        WriteRow( xRowSet, nRow, aColumnStart );
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION("svx", "");
    }

    mrStrm.WriteChar( '}' ).WriteCharPtr( SAL_NEWLINE_STRING );
}

void SdrTableRtfExporter::WriteRow( const Reference< XPropertySet >& xRowSet, sal_Int32 nRow, const std::vector< sal_Int32 >& aColumnStart )
{
    sal_Int32 nRowHeight = 0;
    xRowSet->getPropertyValue( gsSize ) >>= nRowHeight;

    mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TROWD ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TRGAPH ).WriteCharPtr( "30" ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TRLEFT ).WriteCharPtr( "-30" );
    mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_TRRH ).WriteOString( OString::number(nRowHeight) );

    const sal_Int32 nColCount = mxTable->getColumnCount();
    for( sal_Int32 nCol = 0; nCol < nColCount; nCol++ )
    {
        CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );

        if( !xCell.is() )
            continue;

        mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CELLX ).WriteOString( OString::number(aColumnStart[nCol]) );
        if ( (nCol & 0x0F) == 0x0F )
            mrStrm.WriteCharPtr( SAL_NEWLINE_STRING );        // prevent long lines
    }
    mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PARD ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PLAIN ).WriteCharPtr( OOO_STRING_SVTOOLS_RTF_INTBL ).WriteCharPtr( SAL_NEWLINE_STRING );

    sal_uInt64 nStrmPos = mrStrm.Tell();
    for( sal_Int32 nCol = 0; nCol < nColCount; nCol++ )
    {
        WriteCell( nCol, nRow );
        if ( mrStrm.Tell() - nStrmPos > 255 )
        {
            mrStrm.WriteCharPtr( SAL_NEWLINE_STRING );
            nStrmPos = mrStrm.Tell();
        }
    }
    mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_ROW ).WriteCharPtr( SAL_NEWLINE_STRING );
}


void SdrTableRtfExporter::WriteCell( sal_Int32 nCol, sal_Int32 nRow )
{
    CellRef xCell( dynamic_cast< Cell* >( mxTable->getCellByPosition( nCol, nRow ).get() ) );

    if( !xCell.is() || xCell->isMerged() )
    {
        mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CELL );
        return ;
    }

    OUString aContent;

    std::optional<OutlinerParaObject> pParaObj = xCell->CreateEditOutlinerParaObject();

    if( !pParaObj && xCell->GetOutlinerParaObject() )
        pParaObj = *xCell->GetOutlinerParaObject();

    if(pParaObj)
    {
        // handle outliner attributes
        SdrOutliner& rOutliner = mrObj.ImpGetDrawOutliner();
        rOutliner.SetText(*pParaObj);

        aContent = rOutliner.GetEditEngine().GetText();

        rOutliner.Clear();
    }

    bool bResetAttr = false;

    SdrTextHorzAdjust eHAdj = xCell->GetTextHorizontalAdjust();

    const SfxItemSet& rCellSet = xCell->GetItemSet();

    const SvxWeightItem&        rWeightItem     = rCellSet.Get( EE_CHAR_WEIGHT );
    const SvxPostureItem&       rPostureItem    = rCellSet.Get( EE_CHAR_ITALIC );
    const SvxUnderlineItem&     rUnderlineItem  = rCellSet.Get( EE_CHAR_UNDERLINE );

    const char* pChar;

    switch( eHAdj )
    {
        case SDRTEXTHORZADJUST_CENTER:  pChar = OOO_STRING_SVTOOLS_RTF_QC;  break;
        case SDRTEXTHORZADJUST_BLOCK:   pChar = OOO_STRING_SVTOOLS_RTF_QJ;  break;
        case SDRTEXTHORZADJUST_RIGHT:   pChar = OOO_STRING_SVTOOLS_RTF_QR;  break;
        case SDRTEXTHORZADJUST_LEFT:
        default:                        pChar = OOO_STRING_SVTOOLS_RTF_QL;  break;
    }
    mrStrm.WriteCharPtr( pChar );

    if ( rWeightItem.GetWeight() >= WEIGHT_BOLD )
    {   // bold
        bResetAttr = true;
        mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_B );
    }
    if ( rPostureItem.GetPosture() != ITALIC_NONE )
    {   // italic
        bResetAttr = true;
        mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_I );
    }
    if ( rUnderlineItem.GetLineStyle() != LINESTYLE_NONE )
    {   // underline
        bResetAttr = true;
        mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_UL );
    }

    mrStrm.WriteChar( ' ' );
    RTFOutFuncs::Out_String( mrStrm, aContent );
    mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_CELL );

    if ( bResetAttr )
        mrStrm.WriteCharPtr( OOO_STRING_SVTOOLS_RTF_PLAIN );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
