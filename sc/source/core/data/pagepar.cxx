/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagepar.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:06:42 $
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
    bNotes=bGrid=bHeaders=bDrawings=
    bLeftRight=bScaleAll=bScaleTo=bScalePageNum=
    bFormulas=bNullVals=bSkipEmpty          = FALSE;
    bTopDown=bScaleNone=bCharts=bObjects    = TRUE;
    nScaleAll = 100;
    nScalePageNum = nScaleWidth = nScaleHeight = 0;
    nFirstPageNo = 1;
}

//------------------------------------------------------------------------

BOOL ScPageTableParam::operator==( const ScPageTableParam& r ) const
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
    bPrintArea = bRepeatRow = bRepeatCol = FALSE;

    memset( &aPrintArea, 0, sizeof(ScRange) );
    memset( &aRepeatRow, 0, sizeof(ScRange) );
    memset( &aRepeatCol, 0, sizeof(ScRange) );
}

//------------------------------------------------------------------------

BOOL ScPageAreaParam::operator==( const ScPageAreaParam& r ) const
{
    BOOL bEqual =
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
