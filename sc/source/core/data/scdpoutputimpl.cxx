/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
        return sal_False;
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
        return sal_False;
}

void OutputImpl::OutputBlockFrame ( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, sal_Bool bHori )
{

    SvxBorderLine aLine, aOutLine;
    aLine.SetColor( SC_DP_FRAME_COLOR );
    aLine.SetOutWidth( SC_DP_FRAME_INNER_BOLD );
    aOutLine.SetColor( SC_DP_FRAME_COLOR );
    aOutLine.SetOutWidth( SC_DP_FRAME_OUTER_BOLD );

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
    aBoxInfo.SetValid(VALID_VERT,sal_False );
    if ( bHori )
    {
        aBoxInfo.SetValid(VALID_HORI,sal_True);
        aBoxInfo.SetLine( &aLine, BOXINFO_LINE_HORI );
    }
    else
        aBoxInfo.SetValid(VALID_HORI,sal_False );

    aBoxInfo.SetValid(VALID_DISTANCE,sal_False);

    mpDoc->ApplyFrameAreaTab( ScRange(  nStartCol, nStartRow, mnTab, nEndCol, nEndRow , mnTab ), &aBox, &aBoxInfo );

}
