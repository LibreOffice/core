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
#include "precompiled_sw.hxx"



#include "tabcol.hxx"
#include <errhdl.hxx>       // fuer Create-Methoden


SwTabCols::SwTabCols( sal_uInt16 nSize ) :
    nLeftMin( 0 ),
    nLeft( 0 ),
    nRight( 0 ),
    nRightMax( 0 ),
    bLastRowAllowedToChange( true )
{
    if ( nSize )
        aData.reserve( nSize );
}

SwTabCols::SwTabCols( const SwTabCols& rCpy ) :
    nLeftMin( rCpy.GetLeftMin() ),
    nLeft( rCpy.GetLeft() ),
    nRight( rCpy.GetRight() ),
    nRightMax( rCpy.GetRightMax() ),
    bLastRowAllowedToChange( rCpy.IsLastRowAllowedToChange() ),
    aData( rCpy.GetData() )
{
#if OSL_DEBUG_LEVEL > 1
    for ( sal_uInt16 i = 0; i < Count(); ++i )
    {
        SwTabColsEntry aEntry1 = aData[i];
        SwTabColsEntry aEntry2 = rCpy.GetData()[i];
        (void) aEntry1;
        (void) aEntry2;
        ASSERT( aEntry1.nPos == aEntry2.nPos &&
                aEntry1.nMin == aEntry2.nMin &&
                aEntry1.nMax == aEntry2.nMax &&
                aEntry1.bHidden == aEntry2.bHidden,
                "CopyContructor of SwTabColsEntries did not succeed!" )
    }
#endif
}

SwTabCols &SwTabCols::operator=( const SwTabCols& rCpy )
{
    nLeftMin = rCpy.GetLeftMin();
    nLeft    = rCpy.GetLeft();
    nRight   = rCpy.GetRight();
    nRightMax= rCpy.GetRightMax();
    bLastRowAllowedToChange = rCpy.IsLastRowAllowedToChange();

    Remove( 0, Count() );
    aData = rCpy.GetData();

    return *this;
}

sal_Bool SwTabCols::operator==( const SwTabCols& rCmp ) const
{
    sal_uInt16 i;

    if ( !(nLeftMin == rCmp.GetLeftMin() &&
           nLeft    == rCmp.GetLeft()    &&
           nRight   == rCmp.GetRight()   &&
           nRightMax== rCmp.GetRightMax()&&
           bLastRowAllowedToChange== rCmp.IsLastRowAllowedToChange() &&
           Count()== rCmp.Count()) )
        return sal_False;

    for ( i = 0; i < Count(); ++i )
    {
        SwTabColsEntry aEntry1 = aData[i];
        SwTabColsEntry aEntry2 = rCmp.GetData()[i];
        if ( aEntry1.nPos != aEntry2.nPos || aEntry1.bHidden != aEntry2.bHidden  )
            return sal_False;
    }

    return sal_True;
}

void SwTabCols::Insert( long nValue, long nMin, long nMax, sal_Bool bValue, sal_uInt16 nPos )
{
    SwTabColsEntry aEntry;
    aEntry.nPos = nValue;
    aEntry.nMin = nMin;
    aEntry.nMax = nMax;
    aEntry.bHidden = bValue;
    aData.insert( aData.begin() + nPos, aEntry );
}

void SwTabCols::Insert( long nValue, sal_Bool bValue, sal_uInt16 nPos )
{
    SwTabColsEntry aEntry;
    aEntry.nPos = nValue;
    aEntry.nMin = 0;
    aEntry.nMax = LONG_MAX;
    aEntry.bHidden = bValue;
    aData.insert( aData.begin() + nPos, aEntry );

#if OSL_DEBUG_LEVEL > 1
    SwTabColsEntries::iterator aPos = aData.begin();
    for ( ; aPos != aData.end(); ++aPos )
    {
        aEntry =(*aPos);
    }
#endif
}

void SwTabCols::Remove( sal_uInt16 nPos, sal_uInt16 nAnz )
{
    SwTabColsEntries::iterator aStart = aData.begin() + nPos;
    aData.erase( aStart, aStart + nAnz );
}

