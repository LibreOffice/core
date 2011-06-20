/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    bFormulas=bNullVals=bSkipEmpty          = false;
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
    bPrintArea = bRepeatRow = bRepeatCol = false;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
