/*************************************************************************
 *
 *  $RCSfile: pagepar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mh $ $Date: 2001-10-23 10:55:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// INCLUDE ---------------------------------------------------------------

// System - Includes -----------------------------------------------------

#ifdef PCH
#include "core_pch.hxx"
#endif


#include <string.h>

#include "pagepar.hxx"


//========================================================================
// struct ScPageTableParam:

ScPageTableParam::ScPageTableParam()
{
    Reset();
}

//------------------------------------------------------------------------

ScPageTableParam::ScPageTableParam( const ScPageTableParam& r )
{
    *this = r;
}

//------------------------------------------------------------------------

__EXPORT ScPageTableParam::~ScPageTableParam()
{
}

//------------------------------------------------------------------------

void __EXPORT ScPageTableParam::Reset()
{
    bNotes=bGrid=bHeaders=bDrawings=
    bLeftRight=bScaleAll=bScalePageNum=
    bFormulas=bNullVals=bSkipEmpty          = FALSE;
    bTopDown=bScaleNone=bCharts=bObjects    = TRUE;
    nScaleAll       = 100;
    nScalePageNum   = 0;
    nFirstPageNo    = 1;
}

//------------------------------------------------------------------------

ScPageTableParam& __EXPORT ScPageTableParam::operator=( const ScPageTableParam& r )
{
    memcpy( this, &r, sizeof(ScPageTableParam) );

    return *this;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScPageTableParam::operator==( const ScPageTableParam& r ) const
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

ScPageAreaParam::ScPageAreaParam( const ScPageAreaParam& r )
{
    *this = r;
}

//------------------------------------------------------------------------

__EXPORT ScPageAreaParam::~ScPageAreaParam()
{
}

//------------------------------------------------------------------------

void __EXPORT ScPageAreaParam::Reset()
{
    bPrintArea = bRepeatRow = bRepeatCol = FALSE;

    memset( &aPrintArea, 0, sizeof(ScRange) );
    memset( &aRepeatRow, 0, sizeof(ScRange) );
    memset( &aRepeatCol, 0, sizeof(ScRange) );
}

//------------------------------------------------------------------------

ScPageAreaParam& __EXPORT ScPageAreaParam::operator=( const ScPageAreaParam& r )
{
    bPrintArea = r.bPrintArea;
    bRepeatRow = r.bRepeatRow;
    bRepeatCol = r.bRepeatCol;

    memcpy( &aPrintArea, &r.aPrintArea, sizeof(ScRange) );
    memcpy( &aRepeatRow, &r.aRepeatRow, sizeof(ScRange) );
    memcpy( &aRepeatCol, &r.aRepeatCol, sizeof(ScRange) );

    return *this;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScPageAreaParam::operator==( const ScPageAreaParam& r ) const
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
