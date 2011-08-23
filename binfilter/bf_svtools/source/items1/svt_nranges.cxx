/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove


// compiled via include from itemset.cxx only!

//========================================================================

#ifdef DBG_UTIL

#define DBG_CHECK_RANGES(NUMTYPE, pArr)									\
    for ( const NUMTYPE *pRange = pArr; *pRange; pRange += 2 )          \
    {                                                                   \
        DBG_ASSERT( pRange[0] <= pRange[1], "ranges must be sorted" );  \
        DBG_ASSERT( !pRange[2] || ( pRange[2] - pRange[1] ) > 1,        \
                    "ranges must be sorted and discrete" );             \
    }

#else

#define DBG_CHECK_RANGES(NUMTYPE,pArr)

#endif

//============================================================================
inline void Swap_Impl(const NUMTYPE *& rp1, const NUMTYPE *& rp2)
{
    const NUMTYPE * pTemp = rp1;
    rp1 = rp2;
    rp2 = pTemp;
}

//========================================================================

NUMTYPE Count_Impl( const NUMTYPE *pRanges )

/**	<H3>Description</H3>

    Determines the number of NUMTYPEs in an 0-terminated array of pairs of
    NUMTYPEs. The terminating 0 is not included in the count.
*/

{
    NUMTYPE nCount = 0;
    while ( *pRanges )
    {
        nCount += 2;
        pRanges += 2;
    }
    return nCount;
}

//------------------------------------------------------------------------

NUMTYPE Capacity_Impl( const NUMTYPE *pRanges )

/**	<H3>Description</H3>

    Determines the total number of NUMTYPEs described in an 0-terminated
    array of pairs of NUMTYPEs, each representing an range of NUMTYPEs.
*/

{
    NUMTYPE nCount = 0;

    if ( pRanges )
    {
        while ( *pRanges )
        {
            nCount += pRanges[1] - pRanges[0] + 1;
            pRanges += 2;
        }
    }
    return nCount;
}

//------------------------------------------------------------------------

SfxNumRanges::SfxNumRanges( const SfxNumRanges &rOrig )

/**	<H3>Description</H3>

    Copy-Ctor.
*/

{
    if ( rOrig._pRanges )
    {
        NUMTYPE nCount = Count_Impl( rOrig._pRanges ) + 1;
        _pRanges = new NUMTYPE[nCount];
        memcpy( _pRanges, rOrig._pRanges, sizeof(NUMTYPE) * nCount );
    }
    else
        _pRanges = 0;
}

//------------------------------------------------------------------------

BOOL SfxNumRanges::operator==( const SfxNumRanges &rOther ) const
{
    // Object pointers equal?
    if ( this == &rOther )
        return TRUE;

    // Ranges pointers equal?
    if ( _pRanges == rOther._pRanges )
        return TRUE;

    // Counts equal?
    NUMTYPE nCount = Count();
    if ( nCount != rOther.Count() )
        return FALSE;

    // Check arrays.
    NUMTYPE n = 0;
    while( _pRanges[ n ] != 0 )
    {
        // Elements at current position equal?
        if ( _pRanges[ n ] != rOther._pRanges[ n ] )
            return FALSE;

        ++n;
    }

    return TRUE;
}

//------------------------------------------------------------------------

SfxNumRanges& SfxNumRanges::operator =
(
    const SfxNumRanges &rRanges
)

/**	<H3>Description</H3>

    Assigns ranges from 'rRanges' to '*this'.
*/

{
    // special case: assign itself
    if ( &rRanges == this )
        return *this;

    delete[] _pRanges;

    // special case: 'rRanges' is empty
    if ( rRanges.IsEmpty() )
        _pRanges = 0;
    else
    {
        // copy ranges
        NUMTYPE nCount = Count_Impl( rRanges._pRanges ) + 1;
        _pRanges = new NUMTYPE[ nCount ];
        memcpy( _pRanges, rRanges._pRanges, sizeof(NUMTYPE) * nCount );
    }
    return *this;
}

//------------------------------------------------------------------------

SfxNumRanges& SfxNumRanges::operator +=
(
    const SfxNumRanges &rRanges
)

/**	<H3>Description</H3>

    Merges *this with 'rRanges'.

    for each NUMTYPE n:
        this->Contains( n ) || rRanges.Contains( n ) => this'->Contains( n )
        !this->Contains( n ) && !rRanges.Contains( n ) => !this'->Contains( n )
*/

{
    // special cases: one is empty
    if ( rRanges.IsEmpty() )
        return *this;
    if ( IsEmpty() )
        return *this = rRanges;

    // First, run thru _pRanges and rRanges._pRanges and determine the size of
    // the new, merged ranges:
    NUMTYPE nCount = 0;
    const NUMTYPE * pRA = _pRanges;
    const NUMTYPE * pRB = rRanges._pRanges;

    for (;;)
    {
        // The first pair of pRA has a lower lower bound than the first pair
        // of pRB:
        if (pRA[0] > pRB[0])
            Swap_Impl(pRA, pRB);

        // We are done with the merging if at least pRA is exhausted:
        if (!pRA[0])
            break;

        for (;;)
        {
            // Skip those pairs in pRB that completely lie in the first pair
            // of pRA:
            while (pRB[1] <= pRA[1])
            {
                pRB += 2;

                // Watch out for exhaustion of pRB:
                if (!pRB[0])
                {
                    Swap_Impl(pRA, pRB);
                    goto count_rest;
                }
            }

            // If the next pair of pRA does not at least touch the current new
            // pair, we are done with the current new pair:
            if (pRB[0] > pRA[1] + 1)
                break;

            // The next pair of pRB extends the current new pair; first,
            // extend the current new pair (we are done if pRB is then
            // exhausted); second, switch the roles of pRA and pRB in order to
            // merge in those following pairs of the original pRA that will
            // lie in the (now larger) current new pair or will even extend it
            // further:
            pRA += 2;
            if (!pRA[0])
                goto count_rest;
            Swap_Impl(pRA, pRB);
        }

        // Done with the current new pair:
        pRA += 2;
        nCount += 2;
    }

    // Only pRB has more pairs available, pRA is already exhausted:
count_rest:
    for (; pRB[0]; pRB += 2)
        nCount += 2;

    // Now, create new ranges of the correct size and, on a second run thru
    // _pRanges and rRanges._pRanges, copy the merged pairs into the new
    // ranges:
    NUMTYPE * pNew = new NUMTYPE[nCount + 1];
    pRA = _pRanges;
    pRB = rRanges._pRanges;
    NUMTYPE * pRN = pNew;

    for (;;)
    {
        // The first pair of pRA has a lower lower bound than the first pair
        // of pRB:
        if (pRA[0] > pRB[0])
            Swap_Impl(pRA, pRB);

        // We are done with the merging if at least pRA is exhausted:
        if (!pRA[0])
            break;

        // Lower bound of current new pair is already known:
        *pRN++ = pRA[0];

        for (;;)
        {
            // Skip those pairs in pRB that completely lie in the first pair
            // of pRA:
            while (pRB[1] <= pRA[1])
            {
                pRB += 2;

                // Watch out for exhaustion of pRB:
                if (!pRB[0])
                {
                    Swap_Impl(pRA, pRB);
                    ++pRB;
                    goto copy_rest;
                }
            }

            // If the next pair of pRA does not at least touch the current new
            // pair, we are done with the current new pair:
            if (pRB[0] > pRA[1] + 1)
                break;

            // The next pair of pRB extends the current new pair; first,
            // extend the current new pair (we are done if pRB is then
            // exhausted); second, switch the roles of pRA and pRB in order to
            // merge in those following pairs of the original pRA that will
            // lie in the (now larger) current new pair or will even extend it
            // further:
            pRA += 2;
            if (!pRA[0])
            {
                ++pRB;
                goto copy_rest;
            }
            Swap_Impl(pRA, pRB);
        }

        // Done with the current new pair, now upper bound is also known:
        *pRN++ = pRA[1];
        pRA += 2;
    }

    // Only pRB has more pairs available (which are copied to the new ranges
    // unchanged), pRA is already exhausted:
copy_rest:
    for (; *pRB;)
        *pRN++ = *pRB++;
    *pRN = 0;

    delete[] _pRanges;
    _pRanges = pNew;

    return *this;
}

//------------------------------------------------------------------------

SfxNumRanges& SfxNumRanges::operator -=
(
    const SfxNumRanges &rRanges
)

/**	<H3>Description</H3>

    Removes 'rRanges' from '*this'.

    for each NUMTYPE n:
        this->Contains( n ) && rRanges.Contains( n ) => !this'->Contains( n )
        this->Contains( n ) && !rRanges.Contains( n ) => this'->Contains( n )
        !this->Contains( n ) => !this'->Contains( n )
*/

{
    // special cases: one is empty
    if ( rRanges.IsEmpty() || IsEmpty() )
        return *this;

    // differentiate 'rRanges' in a temporary copy of '*this'
    // (size is computed for maximal possibly split-count plus terminating 0)
    NUMTYPE nThisSize = Count_Impl(_pRanges);
    NUMTYPE nTargetSize = 1 + (  nThisSize + Count_Impl(rRanges._pRanges) );
    NUMTYPE *pTarget = new NUMTYPE[ nTargetSize ];
    memset( pTarget, 0, sizeof(NUMTYPE)*nTargetSize );
    memcpy( pTarget, _pRanges, sizeof(NUMTYPE)*nThisSize );

    NUMTYPE nPos1 = 0, nPos2 = 0, nTargetPos = 0;
    while( _pRanges[ nPos1 ] )
    {
        NUMTYPE l1 = _pRanges[ nPos1 ]; 	 // lower bound of interval 1
        NUMTYPE u1 = _pRanges[ nPos1+1 ];	 // upper bound of interval 1
        NUMTYPE l2 = rRanges._pRanges[ nPos2 ]; 	 // lower bound of interval 2
        NUMTYPE u2 = rRanges._pRanges[ nPos2+1 ];	 // upper bound of interval 2

        // boundary cases
        // * subtrahend is empty -> copy the minuend
        if( !l2 )
        {
            pTarget[ nTargetPos ] = l1;
            pTarget[ nTargetPos+1 ] = u1;
            nTargetPos += 2;
            nPos1 += 2;
            continue;
        }
        // * next subtrahend interval is completely higher -> copy the minuend
        if( u1 < l2 )
        {
            pTarget[ nTargetPos ] = l1;
            pTarget[ nTargetPos+1 ] = u1;
            nTargetPos += 2;
            nPos1 += 2;
            continue;
        }

        // * next subtrahend interval is completely lower -> try next
        if( u2 < l1 )
        {
            nPos2 += 2;
            continue;
        }

        // intersecting cases
        // * subtrahend cuts out from the beginning of the minuend
        if( l2 <= l1 && u2 <= u1 )
        {
            // reduce minuend interval, try again (minuend might be affected by other subtrahend intervals)
            _pRanges[ nPos1 ] = u2 + 1;
            nPos2 += 2; // this cannot hurt any longer
            continue;
        }

        // * subtrahend cuts out from the end of the minuend
        if( l1 <= l2 && u1 <= u2 )
        {
            // copy remaining part of minuend (cannot be affected by other intervals)
            if( l1 < l2 ) // anything left at all?
            {
                pTarget[ nTargetPos ] = l1;
                pTarget[ nTargetPos+1 ] = l2 - 1;
                nTargetPos += 2;
                // do not increment nPos2, might affect next minuend interval, too
            }
            nPos1 += 2; // nothing left at all
            continue;
        }

        // * subtrahend completely deletes minuend (larger or same at both ends)
        if( l1 >= l2 && u1 <= u2 )
        {
            nPos1 += 2; // minuend deleted
            // do not increment nPos2, might affect next minuend interval, too
            continue;
        }

        // * subtrahend divides minuend into two pieces
        if( l1 <= l2 && u1 >= u2 ) // >= and <= since they may be something left only at one side
        {
            // left side
            if( l1 < l2 ) // anything left at all
            {
                pTarget[ nTargetPos ] = l1;
                pTarget[ nTargetPos+1 ] = l2 - 1;
                nTargetPos += 2;
            }

            // right side
            if( u1 > u2 ) // anything left at all
            {
                // reduce minuend interval, try again (minuend might be affected by other subtrahend itnervals )
                _pRanges[ nPos1 ] = u2 + 1;
            }

            // subtrahend is completely used
            nPos2 += 2;
            continue;
        }

        // we should never be here
        DBG_ERROR( "SfxNumRanges::operator-=: internal error" );
    } // while

    pTarget[ nTargetPos ] = 0;

    // assign the differentiated ranges
    delete[] _pRanges;

    NUMTYPE nUShorts = Count_Impl(pTarget) + 1;
    if ( 1 != nUShorts )
    {
        _pRanges = new NUMTYPE[ nUShorts ];
        memcpy( _pRanges, pTarget, nUShorts * sizeof(NUMTYPE) );
    }
    else
        _pRanges = 0;

    delete [] pTarget;
    return *this;

    /* untested code from MI commented out (MDA, 28.01.97)
    do
    {
        // 1st range is smaller than 2nd range?
        if ( pRange1[1] < pRange2[0] )
            // => keep 1st range
            pRange1 += 2;

        // 2nd range is smaller than 1st range?
        else if ( pRange2[1] < pRange1[0] )
            // => skip 2nd range
            pRange2 += 2;

        // 2nd range totally overlaps the 1st range?
        else if ( pRange2[0] <= pRange1[0] && pRange2[1] >= pRange1[1] )
            // => remove 1st range
            memmove( pRange1, pRange1+2, sizeof(NUMTYPE) * (pEndOfTarget-pRange1+2) );

        // 2nd range overlaps only the beginning of 1st range?
        else if ( pRange2[0] <= pRange1[0] && pRange2[1] < pRange1[1] )
        {
            // => cut the beginning of 1st range and goto next 2nd range
            pRange1[0] = pRange2[1] + 1;
            pRange2 += 2;
        }

        // 2nd range overlaps only the end of 1st range?
        else if ( pRange2[0] > pRange1[0] && pRange2[1] >= pRange1[0] )
            // => cut the beginning of 1st range
            pRange1[0] = pRange2[1]+1;

        // 2nd range is a real subset of 1st range
        else
        {
            // => split 1st range and goto next 2nd range
            memmove( pRange1+3, pRange1+1, sizeof(NUMTYPE) * (pEndOfTarget-pRange1-1) );
            pRange1[1] = pRange2[0] - 1;
            pRange1[2] = pRange2[1] + 1;
            pRange1 += 2;
            pRange2 += 2;
        }
    }
    while ( *pRange1 && *pRange2 );

    // assign the differentiated ranges
    delete[] _pRanges;
    NUMTYPE nUShorts = Count_Impl(pTarget) + 1;
    if ( 1 != nUShorts )
    {
        _pRanges = new NUMTYPE[ nUShorts ];
        memcpy( _pRanges, pTarget, nUShorts * sizeof(NUMTYPE) );
        _pRanges[ nUShorts-1 ] = 0;
    }
    else
        _pRanges = 0;
    return *this;
    */
}

//------------------------------------------------------------------------

SfxNumRanges& SfxNumRanges::operator /=
(
    const SfxNumRanges &rRanges
)

/**	<H3>Description</H3>

    Determines intersection of '*this' with 'rRanges'.

    for each NUMTYPE n:
        this->Contains( n ) && rRanges.Contains( n ) => this'->Contains( n )
        !this->Contains( n ) => !this'->Contains( n )
        !rRanges.Contains( n ) => !this'->Contains( n )
*/

{
    // boundary cases
    // * first set is empty -> nothing to be done
    // * second set is empty -> delete first set
    if( rRanges.IsEmpty() )
    {
        delete[] _pRanges;

        _pRanges = new NUMTYPE[1];
        _pRanges[0] = 0;

        return *this;
    }

    // intersect 'rRanges' in a temporary copy of '*this'
    // (size is computed for maximal possibly split-count plus terminating 0)
    NUMTYPE nThisSize = Count_Impl(_pRanges);
    NUMTYPE nTargetSize = 1 + (  nThisSize + Count_Impl(rRanges._pRanges) );
    NUMTYPE *pTarget = new NUMTYPE[ nTargetSize ];
    memset( pTarget, 0, sizeof(NUMTYPE)*nTargetSize );
    memcpy( pTarget, _pRanges, sizeof(NUMTYPE)*nThisSize );

    NUMTYPE nPos1 = 0, nPos2 = 0, nTargetPos = 0;
    while( _pRanges[ nPos1 ] != 0 && rRanges._pRanges[ nPos2 ] != 0 )
    {
        NUMTYPE l1 = _pRanges[ nPos1 ]; 	 // lower bound of interval 1
        NUMTYPE u1 = _pRanges[ nPos1+1 ];	 // upper bound of interval 1
        NUMTYPE l2 = rRanges._pRanges[ nPos2 ]; 	 // lower bound of interval 2
        NUMTYPE u2 = rRanges._pRanges[ nPos2+1 ];	 // upper bound of interval 2

        if( u1 < l2 )
        {
            // current interval in s1 is completely before ci in s2
            nPos1 += 2;
            continue;
        }
        if( u2 < l1 )
        {
            // ci in s2 is completely before ci in s1
            nPos2 += 2;
            continue;
        }

        // assert: there exists an intersection between ci1 and ci2

        if( l1 <= l2 )
        {
            // c1 "is more to the left" than c2

            if( u1 <= u2 )
            {
                pTarget[ nTargetPos ] = l2;
                pTarget[ nTargetPos+1 ] = u1;
                nTargetPos += 2;
                nPos1 += 2;
                continue;
            }
            else
            {
                pTarget[ nTargetPos ] = l2;
                pTarget[ nTargetPos+1 ] = u2;
                nTargetPos += 2;
                nPos2 += 2;
            }
        }
        else
        {
            // c2 "is more to the left" than c1"

            if( u1 > u2 )
            {
                pTarget[ nTargetPos ] = l1;
                pTarget[ nTargetPos+1 ] = u2;
                nTargetPos += 2;
                nPos2 += 2;
            }
            else
            {
                pTarget[ nTargetPos ] = l1;
                pTarget[ nTargetPos+1 ] = u1;
                nTargetPos += 2;
                nPos1 += 2;
            }
        }
    }; // while
    pTarget[ nTargetPos ] = 0;

    // assign the intersected ranges
    delete[] _pRanges;

    NUMTYPE nUShorts = Count_Impl(pTarget) + 1;
    if ( 1 != nUShorts )
    {
        _pRanges = new NUMTYPE[ nUShorts ];
        memcpy( _pRanges, pTarget, nUShorts * sizeof(NUMTYPE) );
    }
    else
        _pRanges = 0;

    delete [] pTarget;
    return *this;
}

//------------------------------------------------------------------------

NUMTYPE SfxNumRanges::Count() const

/**	<H3>Description</H3>

    Determines the number of USHORTs in the set described by the ranges
    of USHORTs in '*this'.
*/

{
    return Capacity_Impl( _pRanges );
}
