/*************************************************************************
 *
 *  $RCSfile: frmbase.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:11 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop


#include "formel.hxx"




_ScRangeList::~_ScRangeList()
    {
    ScRange*    p = ( ScRange* ) First();

    while( p )
        {
        delete p;
        p = ( ScRange* ) Next();
        }
    }




_ScRangeListTabs::_ScRangeListTabs( void )
    {
    ppTabLists = new _ScRangeList*[ MAXTAB + 1 ];

    for( UINT16 n = 0 ; n <= MAXTAB ; n++ )
        ppTabLists[ n ] = NULL;

    bHasRanges = FALSE;
    pAct = NULL;
    nAct = 0;
    }


_ScRangeListTabs::~_ScRangeListTabs()
    {
    if( bHasRanges )
        {
        for( UINT16 n = 0 ; n <= MAXTAB ; n++ )
            {
            if( ppTabLists[ n ] )
                delete ppTabLists[ n ];
            }
        }

    delete[] ppTabLists;
    }


void _ScRangeListTabs::Append( SingleRefData a, const BOOL b )
    {
    if( b )
        {
        if( a.nTab > MAXTAB )
            a.nTab = MAXTAB;

        if( a.nCol > MAXCOL )
            a.nCol = MAXCOL;

        if( a.nRow > MAXROW )
            a.nRow = MAXROW;
        }
    else
        {
        DBG_ASSERT( a.nTab <= MAXTAB, "-_ScRangeListTabs::Append(): Luegen haben kurze Abstuerze!" );
        }

    bHasRanges = TRUE;

    _ScRangeList*   p = ppTabLists[ a.nTab ];

    if( !p )
        p = ppTabLists[ a.nTab ] = new _ScRangeList;

    p->Append( a );
    }


void _ScRangeListTabs::Append( ComplRefData a, const BOOL b )
    {
    if( b )
        {
        INT16&  rTab = a.Ref1.nTab;
        if( rTab > MAXTAB )
            rTab = MAXTAB;
        else if( rTab < 0 )
            rTab = 0;

        INT16&  rCol1 = a.Ref1.nCol;
        if( rCol1 > MAXCOL )
            rCol1 = MAXCOL;
        else if( rCol1 < 0 )
            rCol1 = 0;

        INT16&  rRow1 = a.Ref1.nRow;
        if( rRow1 > MAXROW )
            rRow1 = MAXROW;
        else if( rRow1 < 0 )
            rRow1 = 0;

        INT16&  rCol2 = a.Ref2.nCol;
        if( rCol2 > MAXCOL )
            rCol2 = MAXCOL;
        else if( rCol2 < 0 )
            rCol2 = 0;

        INT16&  rRow2 = a.Ref2.nRow;
        if( rRow2 > MAXROW )
            rRow2 = MAXROW;
        else if( rRow2 < 0 )
            rRow2 = 0;
        }
    else
        {
        DBG_ASSERT( a.Ref1.nTab <= MAXTAB,
            "-_ScRangeListTabs::Append(): Luegen haben kurze Abstuerze!" );
        DBG_ASSERT( a.Ref1.nTab == a.Ref2.nTab,
            "+_ScRangeListTabs::Append(): 3D-Ranges werden in SC nicht unterstuetzt!" );
        }

    bHasRanges = TRUE;

    _ScRangeList*   p = ppTabLists[ a.Ref1.nTab ];

    if( !p )
        p = ppTabLists[ a.Ref1.nTab ] = new _ScRangeList;

    p->Append( a );
    }


const ScRange* _ScRangeListTabs::First( const UINT16 n )
    {
    DBG_ASSERT( n <= MAXTAB, "-_ScRangeListTabs::First(): Und tschuessssssss!" );

    if( ppTabLists[ n ] )
        {
        pAct = ppTabLists[ n ];
        nAct = n;
        return pAct->First();
        }
    else
        {
        pAct = NULL;
        nAct = 0;
        return NULL;
        }
    }


const ScRange* _ScRangeListTabs::Next( void )
    {
    if( pAct )
        return pAct->Next();
    else
        return NULL;
    }




ConverterBase::ConverterBase( SvStream &rStr, UINT16 nNewBuffer ) :
    aIn( rStr ),
    aEingPos( ( UINT16 ) 0, ( UINT16 ) 0, ( UINT16 ) 0 )
    {
    DBG_ASSERT( nNewBuffer > 0, "-ExcelToSc::ExcelToSc(): nNewBuffer == 0!" );

    pBuffer = new sal_Char[ nNewBuffer ];

    nBufferSize = nNewBuffer;
    nBytesLeft = 0;
    eStatus = ConvOK;
    }


ConverterBase::~ConverterBase()
    {
    delete[] pBuffer;
    }


void ConverterBase::Reset( INT32 nLen, ScAddress aNewEingPos )
    {
    nBytesLeft = nLen;
    eStatus = ConvOK;
    aEingPos = aNewEingPos;
    aPool.Reset();
    aStack.Reset();
    }


void ConverterBase::Reset( INT32 nLen )
    {
    nBytesLeft = nLen;
    eStatus = ConvOK;
    aEingPos.Set( 0, 0, 0 );
    aPool.Reset();
    aStack.Reset();
    }


void ConverterBase::Reset( ScAddress aNewEingPos )
    {
    nBytesLeft = 0;
    eStatus = ConvOK;
    aEingPos = aNewEingPos;
    aPool.Reset();
    aStack.Reset();
    }



