/*************************************************************************
 *
 *  $RCSfile: tabcol.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-16 09:33:01 $
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


#pragma hdrstop

#include "tabcol.hxx"

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>       // fuer Create-Methoden
#endif


SwTabCols::SwTabCols( USHORT nSize ) :
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
    for ( USHORT i = 0; i < Count(); ++i )
    {
        SwTabColsEntry aEntry1 = aData[i];
        SwTabColsEntry aEntry2 = rCpy.GetData()[i];
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

BOOL SwTabCols::operator==( const SwTabCols& rCmp ) const
{
    USHORT i;

    if ( !(nLeftMin == rCmp.GetLeftMin() &&
           nLeft    == rCmp.GetLeft()    &&
           nRight   == rCmp.GetRight()   &&
           nRightMax== rCmp.GetRightMax()&&
           bLastRowAllowedToChange== rCmp.IsLastRowAllowedToChange() &&
           Count()== rCmp.Count()) )
        return FALSE;

    for ( i = 0; i < Count(); ++i )
    {
        SwTabColsEntry aEntry1 = aData[i];
        SwTabColsEntry aEntry2 = rCmp.GetData()[i];
        if ( aEntry1.nPos != aEntry2.nPos || aEntry1.bHidden != aEntry2.bHidden  )
            return FALSE;
    }

    return TRUE;
}

void SwTabCols::Insert( long nValue, long nMin, long nMax, BOOL bValue, USHORT nPos )
{
    SwTabColsEntry aEntry;
    aEntry.nPos = nValue;
    aEntry.nMin = nMin;
    aEntry.nMax = nMax;
    aEntry.bHidden = bValue;
    aData.insert( aData.begin() + nPos, aEntry );
}

void SwTabCols::Insert( long nValue, BOOL bValue, USHORT nPos )
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

void SwTabCols::Remove( USHORT nPos, USHORT nAnz )
{
    SwTabColsEntries::iterator aStart = aData.begin() + nPos;
    aData.erase( aStart, aStart + nAnz );
}

