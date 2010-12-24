/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
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
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------
#include "scdpoutputimpl.hxx"
#include "scitems.hxx"
#include <editeng/boxitem.hxx>
// -----------------------------------------------------------------------

namespace
{
    bool lcl_compareColfuc ( SCCOL i,  SCCOL j) { return (i<j); }
    bool lcl_compareRowfuc ( SCROW i,  SCROW j) { return (i<j); }
}


void OutputImpl::OutputDataArea()
{
    AddRow( mnDataStartRow );
    AddCol( mnDataStartCol );

    mnCols.push_back( mnTabEndCol+1); //set last row bottom
    mnRows.push_back( mnTabEndRow+1); //set last col bottom

    sal_Bool bAllRows = ( ( mnTabEndRow - mnDataStartRow + 2 ) == (SCROW) mnRows.size() );

    std::sort( mnCols.begin(), mnCols.end(), lcl_compareColfuc );
    std::sort( mnRows.begin(), mnRows.end(), lcl_compareRowfuc );

    for( SCCOL nCol = 0; nCol < (SCCOL)mnCols.size()-1; nCol ++ )
    {
        if ( !bAllRows )
        {
            if ( nCol < (SCCOL)mnCols.size()-2)
            {
                for ( SCROW i = nCol%2; i < (SCROW)mnRows.size()-2; i +=2 )
                    OutputBlockFrame( mnCols[nCol], mnRows[i], mnCols[nCol+1]-1, mnRows[i+1]-1 );
                if ( mnRows.size()>=2 )
                    OutputBlockFrame(  mnCols[nCol], mnRows[mnRows.size()-2], mnCols[nCol+1]-1, mnRows[mnRows.size()-1]-1 );
            }
            else
            {
                for ( SCROW i = 0 ; i < (SCROW)mnRows.size()-1; i++ )
                    OutputBlockFrame(  mnCols[nCol], mnRows[i], mnCols[nCol+1]-1,  mnRows[i+1]-1 );
            }
        }
        else
            OutputBlockFrame( mnCols[nCol], mnRows.front(), mnCols[nCol+1]-1, mnRows.back()-1, bAllRows );
    }
    //out put rows area outer framer
    if ( mnTabStartCol != mnDataStartCol )
    {
        if ( mnTabStartRow != mnDataStartRow )
            OutputBlockFrame( mnTabStartCol, mnTabStartRow, mnDataStartCol-1, mnDataStartRow-1 );
        OutputBlockFrame( mnTabStartCol, mnDataStartRow, mnDataStartCol-1, mnTabEndRow );
    }
    //out put cols area outer framer
    OutputBlockFrame( mnDataStartCol, mnTabStartRow, mnTabEndCol, mnDataStartRow-1 );
}

OutputImpl::OutputImpl( ScDocument* pDoc, sal_uInt16 nTab,
        SCCOL   nTabStartCol,
        SCROW   nTabStartRow,
        SCCOL   nMemberStartCol,
        SCROW   nMemberStartRow,
        SCCOL nDataStartCol,
        SCROW nDataStartRow,
        SCCOL nTabEndCol,
        SCROW nTabEndRow ):
    mpDoc( pDoc ),
    mnTab( nTab ),
    mnTabStartCol( nTabStartCol ),
    mnTabStartRow( nTabStartRow ),
    mnMemberStartCol( nMemberStartCol),
    mnMemberStartRow( nMemberStartRow),
    mnDataStartCol ( nDataStartCol ),
    mnDataStartRow ( nDataStartRow ),
    mnTabEndCol(  nTabEndCol ),
    mnTabEndRow(  nTabEndRow )
{
    mbNeedLineCols.resize( nTabEndCol-nDataStartCol+1, false );
    mbNeedLineRows.resize( nTabEndRow-nDataStartRow+1, false );

}

sal_Bool OutputImpl::AddRow( SCROW nRow )
{
    if ( !mbNeedLineRows[ nRow - mnDataStartRow ] )
    {
        mbNeedLineRows[ nRow - mnDataStartRow ] = true;
        mnRows.push_back( nRow );
        return sal_True;
    }
    else
        return false;
}

sal_Bool OutputImpl::AddCol( SCCOL nCol )
{

    if ( !mbNeedLineCols[ nCol - mnDataStartCol ] )
    {
        mbNeedLineCols[ nCol - mnDataStartCol ] = true;
        mnCols.push_back( nCol );
        return sal_True;
    }
    else
        return false;
}

void OutputImpl::OutputBlockFrame ( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, sal_Bool bHori )
{
    Color color = SC_DP_FRAME_COLOR;
    SvxBorderLine aLine( &color, SC_DP_FRAME_INNER_BOLD );
    SvxBorderLine aOutLine( &color, SC_DP_FRAME_OUTER_BOLD );

    SvxBoxItem aBox( ATTR_BORDER );

    if ( nStartCol == mnTabStartCol )
        aBox.SetLine(&aOutLine, BOX_LINE_LEFT);
    else
        aBox.SetLine(&aLine, BOX_LINE_LEFT);

    if ( nStartRow == mnTabStartRow )
        aBox.SetLine(&aOutLine, BOX_LINE_TOP);
    else
        aBox.SetLine(&aLine, BOX_LINE_TOP);

    if ( nEndCol == mnTabEndCol ) //bottom row
        aBox.SetLine(&aOutLine, BOX_LINE_RIGHT);
    else
        aBox.SetLine(&aLine,  BOX_LINE_RIGHT);

     if ( nEndRow == mnTabEndRow ) //bottom
        aBox.SetLine(&aOutLine,  BOX_LINE_BOTTOM);
    else
        aBox.SetLine(&aLine,  BOX_LINE_BOTTOM);


    SvxBoxInfoItem aBoxInfo( ATTR_BORDER_INNER );
    aBoxInfo.SetValid(VALID_VERT,false );
    if ( bHori )
    {
        aBoxInfo.SetValid(VALID_HORI,sal_True);
        aBoxInfo.SetLine( &aLine, BOXINFO_LINE_HORI );
    }
    else
        aBoxInfo.SetValid(VALID_HORI,false );

    aBoxInfo.SetValid(VALID_DISTANCE,false);

    mpDoc->ApplyFrameAreaTab( ScRange(  nStartCol, nStartRow, mnTab, nEndCol, nEndRow , mnTab ), &aBox, &aBoxInfo );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
