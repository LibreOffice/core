/*************************************************************************
 *
 *  $RCSfile: pagepar.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:15 $
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

#pragma hdrstop

#include <string.h>
#include "segmentc.hxx"

#include "pagepar.hxx"

SEG_EOFGLOBALS()

//========================================================================
// struct ScPageTableParam:
#pragma SEG_FUNCDEF(pagepar_01)

ScPageTableParam::ScPageTableParam()
{
    Reset();
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pagepar_02)

ScPageTableParam::ScPageTableParam( const ScPageTableParam& r )
{
    *this = r;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pagepar_03)

__EXPORT ScPageTableParam::~ScPageTableParam()
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pagepar_04)

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
#pragma SEG_FUNCDEF(pagepar_05)

ScPageTableParam& __EXPORT ScPageTableParam::operator=( const ScPageTableParam& r )
{
    memcpy( this, &r, sizeof(ScPageTableParam) );

    return *this;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pagepar_06)

BOOL __EXPORT ScPageTableParam::operator==( const ScPageTableParam& r ) const
{
    return ( memcmp( this, &r, sizeof(ScPageTableParam) ) == 0 );
}

//========================================================================
// struct ScPageAreaParam:
#pragma SEG_FUNCDEF(pagepar_07)

ScPageAreaParam::ScPageAreaParam()
{
    Reset();
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pagepar_08)

ScPageAreaParam::ScPageAreaParam( const ScPageAreaParam& r )
{
    *this = r;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pagepar_09)

__EXPORT ScPageAreaParam::~ScPageAreaParam()
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pagepar_0a)

void __EXPORT ScPageAreaParam::Reset()
{
    bPrintArea = bRepeatRow = bRepeatCol = FALSE;

    memset( &aPrintArea, 0, sizeof(ScRange) );
    memset( &aRepeatRow, 0, sizeof(ScRange) );
    memset( &aRepeatCol, 0, sizeof(ScRange) );
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pagepar_0b)

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
#pragma SEG_FUNCDEF(pagepar_0c)

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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.14  2000/09/17 14:08:37  willem.vandorp
    OpenOffice header added.

    Revision 1.13  2000/08/31 16:37:58  willem.vandorp
    Header and footer replaced

    Revision 1.12  1997/11/13 19:58:42  NN
    ifndef PCH raus


      Rev 1.11   13 Nov 1997 20:58:42   NN
   ifndef PCH raus

      Rev 1.10   06 Nov 1997 19:45:46   NN
   bSkipEmpty

      Rev 1.9   22 Nov 1995 16:30:54   MO
   ScAreaItem -> ScRangeItem

      Rev 1.8   10 Oct 1995 11:21:52   MO
   Formeln/Nullwerte drucken im TableParam, Store/Load entfernt

      Rev 1.7   07 Oct 1995 13:18:28   NN
   nTabCount, aTabArr raus

      Rev 1.6   21 Jul 1995 09:37:02   WKC
   memory.h -> string.h

      Rev 1.5   26 Jun 1995 13:59:38   MO
   bDrawings und nFirstPageNo

      Rev 1.4   11 Jun 1995 20:56:06   NN
   Objekte/Charts drucken per Default an

      Rev 1.3   15 May 1995 19:08:08   NN
   Load/Store

      Rev 1.2   09 May 1995 20:00:38   MO
   AreaParam: RefTripel -> ScArea, Flags, ob Areas vorhanden

      Rev 1.1   09 May 1995 12:19:34   TRI
   memory.h included

      Rev 1.0   08 May 1995 20:04:16   MO
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

