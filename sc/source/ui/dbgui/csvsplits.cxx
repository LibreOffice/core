/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: csvsplits.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:32:30 $
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
#include "ui_pch.hxx"
#endif
#pragma hdrstop

// ============================================================================

#ifndef _SC_CSVSPLITS_HXX
#include "csvsplits.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <algorithm>


// ============================================================================

bool ScCsvSplits::Insert( sal_Int32 nPos )
{
    bool bValid = (nPos >= 0);
    if( bValid )
    {
        iterator aIter = ::std::lower_bound( maVec.begin(), maVec.end(), nPos );
        bValid = (aIter == maVec.end()) || (*aIter != nPos);
        if( bValid )
            aIter = maVec.insert( aIter, nPos );
    }
    return bValid;
}

bool ScCsvSplits::Remove( sal_Int32 nPos )
{
    sal_uInt32 nIndex = GetIndex( nPos );
    bool bValid = (nIndex != CSV_VEC_NOTFOUND);
    if( bValid )
        maVec.erase( maVec.begin() + nIndex );
    return bValid;
}

void ScCsvSplits::RemoveRange( sal_Int32 nPosStart, sal_Int32 nPosEnd )
{
    sal_uInt32 nStartIx = LowerBound( nPosStart );
    sal_uInt32 nEndIx = UpperBound( nPosEnd );
    if( (nStartIx != CSV_VEC_NOTFOUND) && (nEndIx != CSV_VEC_NOTFOUND) && (nStartIx <= nEndIx) )
        maVec.erase( maVec.begin() + nStartIx, maVec.begin() + nEndIx + 1 );
}

void ScCsvSplits::Clear()
{
    maVec.clear();
}

bool ScCsvSplits::HasSplit( sal_Int32 nPos ) const
{
    return GetIndex( nPos ) != CSV_VEC_NOTFOUND;
}


// ----------------------------------------------------------------------------

sal_uInt32 ScCsvSplits::GetIndex( sal_Int32 nPos ) const
{
    const_iterator aIter = ::std::lower_bound( maVec.begin(), maVec.end(), nPos );
    return GetIterIndex( ((aIter != maVec.end()) && (*aIter == nPos)) ? aIter : maVec.end() );
}

sal_uInt32 ScCsvSplits::LowerBound( sal_Int32 nPos ) const
{
    return GetIterIndex( ::std::lower_bound( maVec.begin(), maVec.end(), nPos ) );
}

sal_uInt32 ScCsvSplits::UpperBound( sal_Int32 nPos ) const
{
    sal_uInt32 nIndex = LowerBound( nPos );
    if( nIndex == CSV_VEC_NOTFOUND )
        return Count() ? (Count() - 1) : CSV_VEC_NOTFOUND;
    if( GetPos( nIndex ) == nPos )
        return nIndex;
    return nIndex ? (nIndex - 1) : CSV_VEC_NOTFOUND;
}

sal_Int32 ScCsvSplits::GetPos( sal_uInt32 nIndex ) const
{
    return (nIndex < Count()) ? maVec[ nIndex ] : CSV_POS_INVALID;
}


// ----------------------------------------------------------------------------

sal_uInt32 ScCsvSplits::GetIterIndex( const_iterator aIter ) const
{
    return (aIter == maVec.end()) ? CSV_VEC_NOTFOUND : (aIter - maVec.begin());
}


// ============================================================================

