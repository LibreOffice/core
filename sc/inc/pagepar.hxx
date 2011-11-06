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



#ifndef SC_PAGEPAR_HXX
#define SC_PAGEPAR_HXX

#include "global.hxx"
#include "address.hxx"

//-----------------------------------------------------------------------

struct ScPageTableParam
{
    sal_Bool    bCellContent;
    sal_Bool    bNotes;
    sal_Bool    bGrid;
    sal_Bool    bHeaders;
    sal_Bool    bCharts;
    sal_Bool    bObjects;
    sal_Bool    bDrawings;
    sal_Bool    bFormulas;
    sal_Bool    bNullVals;
    sal_Bool    bTopDown;
    sal_Bool    bLeftRight;
    sal_Bool    bSkipEmpty;
    sal_Bool    bScaleNone;
    sal_Bool    bScaleAll;
    sal_Bool    bScaleTo;
    sal_Bool    bScalePageNum;
    sal_uInt16  nScaleAll;
    sal_uInt16  nScaleWidth;
    sal_uInt16  nScaleHeight;
    sal_uInt16  nScalePageNum;
    sal_uInt16  nFirstPageNo;

    ScPageTableParam();
    ~ScPageTableParam();

    sal_Bool                operator==  ( const ScPageTableParam& r ) const;
    void                Reset       ();
};

struct ScPageAreaParam
{
    sal_Bool    bPrintArea;
    sal_Bool    bRepeatRow;
    sal_Bool    bRepeatCol;
    ScRange aPrintArea;
    ScRange aRepeatRow;
    ScRange aRepeatCol;

    ScPageAreaParam();
    ~ScPageAreaParam();

    sal_Bool                operator==  ( const ScPageAreaParam& r ) const;
    void                Reset       ();
};


#endif


