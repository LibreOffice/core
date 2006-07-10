/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmbase.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:30:03 $
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
        DBG_ASSERT( ValidTab(a.nTab), "-_ScRangeListTabs::Append(): Luegen haben kurze Abstuerze!" );
    }

    bHasRanges = TRUE;

    if( a.nTab >= 0 )
    {
        _ScRangeList*   p = ppTabLists[ a.nTab ];

        if( !p )
            p = ppTabLists[ a.nTab ] = new _ScRangeList;

        p->Append( a );
    }
}


void _ScRangeListTabs::Append( ComplRefData a, const BOOL b )
{
    if( b )
    {
        // #96263# ignore 3D ranges
        if( a.Ref1.nTab != a.Ref2.nTab )
            return;

        SCsTAB& rTab = a.Ref1.nTab;
        if( rTab > MAXTAB )
            rTab = MAXTAB;
        else if( rTab < 0 )
            rTab = 0;

        SCsCOL& rCol1 = a.Ref1.nCol;
        if( rCol1 > MAXCOL )
            rCol1 = MAXCOL;
        else if( rCol1 < 0 )
            rCol1 = 0;

        SCsROW& rRow1 = a.Ref1.nRow;
        if( rRow1 > MAXROW )
            rRow1 = MAXROW;
        else if( rRow1 < 0 )
            rRow1 = 0;

        SCsCOL& rCol2 = a.Ref2.nCol;
        if( rCol2 > MAXCOL )
            rCol2 = MAXCOL;
        else if( rCol2 < 0 )
            rCol2 = 0;

        SCsROW& rRow2 = a.Ref2.nRow;
        if( rRow2 > MAXROW )
            rRow2 = MAXROW;
        else if( rRow2 < 0 )
            rRow2 = 0;
    }
    else
    {
        DBG_ASSERT( ValidTab(a.Ref1.nTab),
            "-_ScRangeListTabs::Append(): Luegen haben kurze Abstuerze!" );
        DBG_ASSERT( a.Ref1.nTab == a.Ref2.nTab,
            "+_ScRangeListTabs::Append(): 3D-Ranges werden in SC nicht unterstuetzt!" );
    }

    bHasRanges = TRUE;

    if( a.Ref1.nTab >= 0 )
    {
        _ScRangeList*   p = ppTabLists[ a.Ref1.nTab ];

        if( !p )
            p = ppTabLists[ a.Ref1.nTab ] = new _ScRangeList;

        p->Append( a );
    }
}


const ScRange* _ScRangeListTabs::First( const UINT16 n )
{
    DBG_ASSERT( ValidTab(n), "-_ScRangeListTabs::First(): Und tschuessssssss!" );

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




ConverterBase::ConverterBase( UINT16 nNewBuffer ) :
    aPool(),
    aStack(),
    aEingPos( 0, 0, 0 ),
    nBufferSize( nNewBuffer ),
    eStatus( ConvOK )
{
    DBG_ASSERT( nNewBuffer > 0, "ConverterBase::ConverterBase - nNewBuffer == 0!" );
    pBuffer = new sal_Char[ nNewBuffer ];
}

ConverterBase::~ConverterBase()
{
    delete[] pBuffer;
}

void ConverterBase::Reset()
{
    eStatus = ConvOK;
    aPool.Reset();
    aStack.Reset();
}




ExcelConverterBase::ExcelConverterBase( UINT16 nNewBuffer ) :
    ConverterBase( nNewBuffer )
{
}

ExcelConverterBase::~ExcelConverterBase()
{
}

void ExcelConverterBase::Reset( ScAddress aNewEingPos )
{
    ConverterBase::Reset();
    aEingPos = aNewEingPos;
}

void ExcelConverterBase::Reset()
{
    ConverterBase::Reset();
    aEingPos.Set( 0, 0, 0 );
}




LotusConverterBase::LotusConverterBase( SvStream &rStr, UINT16 nNewBuffer ) :
    ConverterBase( nNewBuffer ),
    aIn( rStr ),
    nBytesLeft( 0 )
{
}

LotusConverterBase::~LotusConverterBase()
{
}

void LotusConverterBase::Reset( INT32 nLen, ScAddress aNewEingPos )
{
    ConverterBase::Reset();
    nBytesLeft = nLen;
    aEingPos = aNewEingPos;
}

void LotusConverterBase::Reset( INT32 nLen )
{
    ConverterBase::Reset();
    nBytesLeft = nLen;
    aEingPos.Set( 0, 0, 0 );
}

void LotusConverterBase::Reset( ScAddress aNewEingPos )
{
    ConverterBase::Reset();
    nBytesLeft = 0;
    aEingPos = aNewEingPos;
}

