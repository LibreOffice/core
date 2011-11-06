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

// System - Includes -----------------------------------------------------



#include <string.h>

#include "pagepar.hxx"


//========================================================================
// struct ScPageTableParam:

ScPageTableParam::ScPageTableParam()
{
    Reset();
}

//------------------------------------------------------------------------

ScPageTableParam::~ScPageTableParam()
{
}

//------------------------------------------------------------------------

void ScPageTableParam::Reset()
{
    bCellContent = sal_True;
    bNotes=bGrid=bHeaders=bDrawings=
    bLeftRight=bScaleAll=bScaleTo=bScalePageNum=
    bFormulas=bNullVals=bSkipEmpty          = sal_False;
    bTopDown=bScaleNone=bCharts=bObjects    = sal_True;
    nScaleAll = 100;
    nScalePageNum = nScaleWidth = nScaleHeight = 0;
    nFirstPageNo = 1;
}

//------------------------------------------------------------------------

sal_Bool ScPageTableParam::operator==( const ScPageTableParam& r ) const
{
    return ( memcmp( this, &r, sizeof(ScPageTableParam) ) == 0 );
}

//========================================================================
// struct ScPageAreaParam:

ScPageAreaParam::ScPageAreaParam()
{
    Reset();
}

//------------------------------------------------------------------------

ScPageAreaParam::~ScPageAreaParam()
{
}

//------------------------------------------------------------------------

void ScPageAreaParam::Reset()
{
    bPrintArea = bRepeatRow = bRepeatCol = sal_False;

    memset( &aPrintArea, 0, sizeof(ScRange) );
    memset( &aRepeatRow, 0, sizeof(ScRange) );
    memset( &aRepeatCol, 0, sizeof(ScRange) );
}

//------------------------------------------------------------------------

sal_Bool ScPageAreaParam::operator==( const ScPageAreaParam& r ) const
{
    sal_Bool bEqual =
            bPrintArea  == r.bPrintArea
        &&  bRepeatRow  == r.bRepeatRow
        &&  bRepeatCol  == r.bRepeatCol;

    if ( bEqual )
        if ( bPrintArea )
            bEqual = bEqual && ( aPrintArea == r.aPrintArea );
    if ( bEqual )
        if ( bRepeatRow )
            bEqual = bEqual && ( aRepeatRow == r.aRepeatRow );
    if ( bEqual )
        if ( bRepeatCol )
            bEqual = bEqual && ( aRepeatCol == r.aRepeatCol );

    return bEqual;
}
