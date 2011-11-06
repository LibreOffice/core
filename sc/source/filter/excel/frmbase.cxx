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

    for( sal_uInt16 n = 0 ; n <= MAXTAB ; n++ )
        ppTabLists[ n ] = NULL;

    bHasRanges = sal_False;
    pAct = NULL;
    nAct = 0;
}


_ScRangeListTabs::~_ScRangeListTabs()
{
    if( bHasRanges )
    {
        for( sal_uInt16 n = 0 ; n <= MAXTAB ; n++ )
        {
            if( ppTabLists[ n ] )
                delete ppTabLists[ n ];
        }
    }

    delete[] ppTabLists;
}


void _ScRangeListTabs::Append( ScSingleRefData a, const sal_Bool b )
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

    bHasRanges = sal_True;

    if( a.nTab >= 0 )
    {
        _ScRangeList*   p = ppTabLists[ a.nTab ];

        if( !p )
            p = ppTabLists[ a.nTab ] = new _ScRangeList;

        p->Append( a );
    }
}


void _ScRangeListTabs::Append( ScComplexRefData a, const sal_Bool b )
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

    bHasRanges = sal_True;

    if( a.Ref1.nTab >= 0 )
    {
        _ScRangeList*   p = ppTabLists[ a.Ref1.nTab ];

        if( !p )
            p = ppTabLists[ a.Ref1.nTab ] = new _ScRangeList;

        p->Append( a );
    }
}


const ScRange* _ScRangeListTabs::First( const sal_uInt16 n )
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




ConverterBase::ConverterBase( sal_uInt16 nNewBuffer ) :
    aEingPos( 0, 0, 0 ),
    eStatus( ConvOK ),
    nBufferSize( nNewBuffer )
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




ExcelConverterBase::ExcelConverterBase( sal_uInt16 nNewBuffer ) :
    ConverterBase( nNewBuffer )
{
}

ExcelConverterBase::~ExcelConverterBase()
{
}

void ExcelConverterBase::Reset( const ScAddress& rEingPos )
{
    ConverterBase::Reset();
    aEingPos = rEingPos;
}

void ExcelConverterBase::Reset()
{
    ConverterBase::Reset();
    aEingPos.Set( 0, 0, 0 );
}




LotusConverterBase::LotusConverterBase( SvStream &rStr, sal_uInt16 nNewBuffer ) :
    ConverterBase( nNewBuffer ),
    aIn( rStr ),
    nBytesLeft( 0 )
{
}

LotusConverterBase::~LotusConverterBase()
{
}

//UNUSED2008-05  void LotusConverterBase::Reset( sal_Int32 nLen, const ScAddress& rEingPos )
//UNUSED2008-05  {
//UNUSED2008-05      ConverterBase::Reset();
//UNUSED2008-05      nBytesLeft = nLen;
//UNUSED2008-05      aEingPos = rEingPos;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  void LotusConverterBase::Reset( sal_Int32 nLen )
//UNUSED2008-05  {
//UNUSED2008-05      ConverterBase::Reset();
//UNUSED2008-05      nBytesLeft = nLen;
//UNUSED2008-05      aEingPos.Set( 0, 0, 0 );
//UNUSED2008-05  }

void LotusConverterBase::Reset( const ScAddress& rEingPos )
{
    ConverterBase::Reset();
    nBytesLeft = 0;
    aEingPos = rEingPos;
}

