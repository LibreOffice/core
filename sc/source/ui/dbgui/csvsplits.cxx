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


// ============================================================================
#include "csvsplits.hxx"
#include <tools/debug.hxx>

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

