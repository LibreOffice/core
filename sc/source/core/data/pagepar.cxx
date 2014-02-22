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

#include <string.h>

#include "pagepar.hxx"


//========================================================================
// struct ScPageTableParam:

ScPageTableParam::ScPageTableParam()
{
    Reset();
}



ScPageTableParam::~ScPageTableParam()
{
}



void ScPageTableParam::Reset()
{
    bCellContent = true;
    bNotes=bGrid=bHeaders=bDrawings=
    bLeftRight=bScaleAll=bScaleTo=bScalePageNum=
    bFormulas=bNullVals=bSkipEmpty=bForceBreaks = false;
    bTopDown=bScaleNone=bCharts=bObjects    = true;
    nScaleAll = 100;
    nScalePageNum = nScaleWidth = nScaleHeight = 0;
    nFirstPageNo = 1;
}



bool ScPageTableParam::operator==( const ScPageTableParam& r ) const
{
    return ( memcmp( this, &r, sizeof(ScPageTableParam) ) == 0 );
}

//========================================================================
// struct ScPageAreaParam:

ScPageAreaParam::ScPageAreaParam()
{
    Reset();
}



ScPageAreaParam::~ScPageAreaParam()
{
}



void ScPageAreaParam::Reset()
{
    bPrintArea = bRepeatRow = bRepeatCol = false;

    memset( &aPrintArea, 0, sizeof(ScRange) );
    memset( &aRepeatRow, 0, sizeof(ScRange) );
    memset( &aRepeatCol, 0, sizeof(ScRange) );
}



bool ScPageAreaParam::operator==( const ScPageAreaParam& r ) const
{
    bool bEqual =
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
