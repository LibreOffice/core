/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <cassert>
#include <vector>



#ifdef DBG_UTIL

#define DBG_CHECK_RANGES(sal_uInt16, pArr)                                 \
    for ( const sal_uInt16 *pRange = pArr; *pRange; pRange += 2 )          \
    {                                                                   \
        DBG_ASSERT( pRange[0] <= pRange[1], "ranges must be sorted" );  \
        DBG_ASSERT( !pRange[2] || ( pRange[2] - pRange[1] ) > 1,        \
                    "ranges must be sorted and discrete" );             \
    }

#else

#define DBG_CHECK_RANGES(sal_uInt16,pArr)

#endif

inline void Swap_Impl(const sal_uInt16 *& rp1, const sal_uInt16 *& rp2)
{
    const sal_uInt16 * pTemp = rp1;
    rp1 = rp2;
    rp2 = pTemp;
}


sal_uInt16 InitializeRanges_Impl( sal_uInt16 *&rpRanges, va_list pArgs,
                               sal_uInt16 nWh1, sal_uInt16 nWh2, sal_uInt16 nNull )

/** <H3>Description</H3>

    Creates an sal_uInt16-ranges-array in 'rpRanges' using 'nWh1' and 'nWh2' as
    first range, 'nNull' as terminator or start of 2nd range and 'pArgs' as
    remaider.

    It returns the number of sal_uInt16s which are contained in the described
    set of sal_uInt16s.
*/

{
    sal_uInt16 nSize = 0, nIns = 0;
    std::vector<sal_uInt16> aNumArr;
    aNumArr.push_back( nWh1 );
    aNumArr.push_back( nWh2 );
    DBG_ASSERT( nWh1 <= nWh2, "Ungueltiger Bereich" );
    nSize += nWh2 - nWh1 + 1;
    aNumArr.push_back( nNull );
    bool bEndOfRange = false;
    while ( 0 !=
            ( nIns =
              sal::static_int_cast< sal_uInt16 >(
                  va_arg( pArgs, int ) ) ) )
    {
        bEndOfRange = !bEndOfRange;
        if ( bEndOfRange )
        {
            const sal_uInt16 nPrev(*aNumArr.rbegin());
            DBG_ASSERT( nPrev <= nIns, "Ungueltiger Bereich" );
            nSize += nIns - nPrev + 1;
        }
        aNumArr.push_back( nIns );
    }

    assert( bEndOfRange ); 

    
    rpRanges = new sal_uInt16[ aNumArr.size() + 1 ];
    std::copy( aNumArr.begin(), aNumArr.end(), rpRanges);
    *(rpRanges + aNumArr.size()) = 0;

    return nSize;
}


sal_uInt16 Count_Impl( const sal_uInt16 *pRanges )

/** <H3>Description</H3>

    Determines the number of sal_uInt16s in an 0-terminated array of pairs of
    sal_uInt16s. The terminating 0 is not included in the count.
*/

{
    sal_uInt16 nCount = 0;
    while ( *pRanges )
    {
        nCount += 2;
        pRanges += 2;
    }
    return nCount;
}


sal_uInt16 Capacity_Impl( const sal_uInt16 *pRanges )

/** <H3>Description</H3>

    Determines the total number of sal_uInt16s described in an 0-terminated
    array of pairs of sal_uInt16s, each representing an range of sal_uInt16s.
*/

{
    sal_uInt16 nCount = 0;

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


SfxUShortRanges::SfxUShortRanges( const SfxUShortRanges &rOrig )

/** <H3>Description</H3>

    Copy-Ctor.
*/

{
    if ( rOrig._pRanges )
    {
        sal_uInt16 nCount = Count_Impl( rOrig._pRanges ) + 1;
        _pRanges = new sal_uInt16[nCount];
        memcpy( _pRanges, rOrig._pRanges, sizeof(sal_uInt16) * nCount );
    }
    else
        _pRanges = 0;
}


SfxUShortRanges::SfxUShortRanges( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )

/** <H3>Description</H3>

    Constructs an SfxUShortRanges-instance from one range of sal_uInt16s.

    precondition:
        nWhich1 <= nWhich2
*/

:   _pRanges( new sal_uInt16[3] )
{
    _pRanges[0] = nWhich1;
    _pRanges[1] = nWhich2;
    _pRanges[2] = 0;
}


SfxUShortRanges::SfxUShortRanges( const sal_uInt16* pArr )

/** <H3>Description</H3>

    Constcurts an SfxUShortRanges-instance from an sorted ranges of sal_uInt16s,
    terminates with on 0.

    precondition: for each n >= 0 && n < (sizeof(pArr)-1)
        pArr[2n] <= pArr[2n+1] && ( pArr[2n+2]-pArr[2n+1] ) > 1
*/

{
    DBG_CHECK_RANGES(sal_uInt16, pArr);
    sal_uInt16 nCount = Count_Impl(pArr) + 1;
    _pRanges = new sal_uInt16[ nCount ];
    memcpy( _pRanges, pArr, sizeof(sal_uInt16) * nCount );
}


bool SfxUShortRanges::operator==( const SfxUShortRanges &rOther ) const
{
    
    if ( this == &rOther )
        return true;

    
    if ( _pRanges == rOther._pRanges )
        return true;

    
    sal_uInt16 nCount = Count();
    if ( nCount != rOther.Count() )
        return false;

    
    sal_uInt16 n = 0;
    while( _pRanges[ n ] != 0 )
    {
        
        if ( _pRanges[ n ] != rOther._pRanges[ n ] )
            return false;

        ++n;
    }

    return true;
}


SfxUShortRanges& SfxUShortRanges::operator =
(
    const SfxUShortRanges &rRanges
)

/** <H3>Description</H3>

    Assigns ranges from 'rRanges' to '*this'.
*/

{
    
    if ( &rRanges == this )
        return *this;

    delete[] _pRanges;

    
    if ( rRanges.IsEmpty() )
        _pRanges = 0;
    else
    {
        
        sal_uInt16 nCount = Count_Impl( rRanges._pRanges ) + 1;
        _pRanges = new sal_uInt16[ nCount ];
        memcpy( _pRanges, rRanges._pRanges, sizeof(sal_uInt16) * nCount );
    }
    return *this;
}


SfxUShortRanges& SfxUShortRanges::operator +=
(
    const SfxUShortRanges &rRanges
)

/** <H3>Description</H3>

    Merges *this with 'rRanges'.

    for each sal_uInt16 n:
        this->Contains( n ) || rRanges.Contains( n ) => this'->Contains( n )
        !this->Contains( n ) && !rRanges.Contains( n ) => !this'->Contains( n )
*/

{
    
    if ( rRanges.IsEmpty() )
        return *this;
    if ( IsEmpty() )
        return *this = rRanges;

    
    
    sal_uInt16 nCount = 0;
    const sal_uInt16 * pRA = _pRanges;
    const sal_uInt16 * pRB = rRanges._pRanges;

    for (;;)
    {
        
        
        if (pRA[0] > pRB[0])
            Swap_Impl(pRA, pRB);

        
        if (!pRA[0])
            break;

        for (;;)
        {
            
            
            while (pRB[1] <= pRA[1])
            {
                pRB += 2;

                
                if (!pRB[0])
                {
                    Swap_Impl(pRA, pRB);
                    goto count_rest;
                }
            }

            
            
            if (pRB[0] > pRA[1] + 1)
                break;

            
            
            
            
            
            
            pRA += 2;
            if (!pRA[0])
                goto count_rest;
            Swap_Impl(pRA, pRB);
        }

        
        pRA += 2;
        nCount += 2;
    }

    
count_rest:
    for (; pRB[0]; pRB += 2)
        nCount += 2;

    
    
    
    sal_uInt16 * pNew = new sal_uInt16[nCount + 1];
    pRA = _pRanges;
    pRB = rRanges._pRanges;
    sal_uInt16 * pRN = pNew;

    for (;;)
    {
        
        
        if (pRA[0] > pRB[0])
            Swap_Impl(pRA, pRB);

        
        if (!pRA[0])
            break;

        
        *pRN++ = pRA[0];

        for (;;)
        {
            
            
            while (pRB[1] <= pRA[1])
            {
                pRB += 2;

                
                if (!pRB[0])
                {
                    Swap_Impl(pRA, pRB);
                    ++pRB;
                    goto copy_rest;
                }
            }

            
            
            if (pRB[0] > pRA[1] + 1)
                break;

            
            
            
            
            
            
            pRA += 2;
            if (!pRA[0])
            {
                ++pRB;
                goto copy_rest;
            }
            Swap_Impl(pRA, pRB);
        }

        
        *pRN++ = pRA[1];
        pRA += 2;
    }

    
    
copy_rest:
    for (; *pRB;)
        *pRN++ = *pRB++;
    *pRN = 0;

    delete[] _pRanges;
    _pRanges = pNew;

    return *this;
}


SfxUShortRanges& SfxUShortRanges::operator -=
(
    const SfxUShortRanges &rRanges
)

/** <H3>Description</H3>

    Removes 'rRanges' from '*this'.

    for each sal_uInt16 n:
        this->Contains( n ) && rRanges.Contains( n ) => !this'->Contains( n )
        this->Contains( n ) && !rRanges.Contains( n ) => this'->Contains( n )
        !this->Contains( n ) => !this'->Contains( n )
*/

{
    
    if ( rRanges.IsEmpty() || IsEmpty() )
        return *this;

    
    
    sal_uInt16 nThisSize = Count_Impl(_pRanges);
    sal_uInt16 nTargetSize = 1 + (  nThisSize + Count_Impl(rRanges._pRanges) );
    sal_uInt16 *pTarget = new sal_uInt16[ nTargetSize ];
    memset( pTarget, 0, sizeof(sal_uInt16)*nTargetSize );
    memcpy( pTarget, _pRanges, sizeof(sal_uInt16)*nThisSize );

    sal_uInt16 nPos1 = 0, nPos2 = 0, nTargetPos = 0;
    while( _pRanges[ nPos1 ] )
    {
        sal_uInt16 l1 = _pRanges[ nPos1 ];      
        sal_uInt16 u1 = _pRanges[ nPos1+1 ];    
        sal_uInt16 l2 = rRanges._pRanges[ nPos2 ];      
        sal_uInt16 u2 = rRanges._pRanges[ nPos2+1 ];    

        
        
        if( !l2 )
        {
            pTarget[ nTargetPos ] = l1;
            pTarget[ nTargetPos+1 ] = u1;
            nTargetPos += 2;
            nPos1 += 2;
            continue;
        }
        
        if( u1 < l2 )
        {
            pTarget[ nTargetPos ] = l1;
            pTarget[ nTargetPos+1 ] = u1;
            nTargetPos += 2;
            nPos1 += 2;
            continue;
        }

        
        if( u2 < l1 )
        {
            nPos2 += 2;
            continue;
        }

        
        
        if( l2 <= l1 && u2 <= u1 )
        {
            
            _pRanges[ nPos1 ] = u2 + 1;
            nPos2 += 2; 
            continue;
        }

        
        if( l1 <= l2 && u1 <= u2 )
        {
            
            if( l1 < l2 ) 
            {
                pTarget[ nTargetPos ] = l1;
                pTarget[ nTargetPos+1 ] = l2 - 1;
                nTargetPos += 2;
                
            }
            nPos1 += 2; 
            continue;
        }

        
        if( l1 >= l2 && u1 <= u2 )
        {
            nPos1 += 2; 
            
            continue;
        }

        
        if( l1 <= l2 && u1 >= u2 ) 
        {
            
            if( l1 < l2 ) 
            {
                pTarget[ nTargetPos ] = l1;
                pTarget[ nTargetPos+1 ] = l2 - 1;
                nTargetPos += 2;
            }

            
            if( u1 > u2 ) 
            {
                
                _pRanges[ nPos1 ] = u2 + 1;
            }

            
            nPos2 += 2;
            continue;
        }

        
        OSL_FAIL( "SfxUShortRanges::operator-=: internal error" );
    } 

    pTarget[ nTargetPos ] = 0;

    
    delete[] _pRanges;

    sal_uInt16 nUShorts = Count_Impl(pTarget) + 1;
    if ( 1 != nUShorts )
    {
        _pRanges = new sal_uInt16[ nUShorts ];
        memcpy( _pRanges, pTarget, nUShorts * sizeof(sal_uInt16) );
    }
    else
        _pRanges = 0;

    delete [] pTarget;
    return *this;
}


SfxUShortRanges& SfxUShortRanges::operator /=
(
    const SfxUShortRanges &rRanges
)

/** <H3>Description</H3>

    Determines intersection of '*this' with 'rRanges'.

    for each sal_uInt16 n:
        this->Contains( n ) && rRanges.Contains( n ) => this'->Contains( n )
        !this->Contains( n ) => !this'->Contains( n )
        !rRanges.Contains( n ) => !this'->Contains( n )
*/

{
    
    
    
    if( rRanges.IsEmpty() )
    {
        delete[] _pRanges;

        _pRanges = new sal_uInt16[1];
        _pRanges[0] = 0;

        return *this;
    }

    
    
    sal_uInt16 nThisSize = Count_Impl(_pRanges);
    sal_uInt16 nTargetSize = 1 + (  nThisSize + Count_Impl(rRanges._pRanges) );
    sal_uInt16 *pTarget = new sal_uInt16[ nTargetSize ];
    memset( pTarget, 0, sizeof(sal_uInt16)*nTargetSize );
    memcpy( pTarget, _pRanges, sizeof(sal_uInt16)*nThisSize );

    sal_uInt16 nPos1 = 0, nPos2 = 0, nTargetPos = 0;
    while( _pRanges[ nPos1 ] != 0 && rRanges._pRanges[ nPos2 ] != 0 )
    {
        sal_uInt16 l1 = _pRanges[ nPos1 ];      
        sal_uInt16 u1 = _pRanges[ nPos1+1 ];    
        sal_uInt16 l2 = rRanges._pRanges[ nPos2 ];      
        sal_uInt16 u2 = rRanges._pRanges[ nPos2+1 ];    

        if( u1 < l2 )
        {
            
            nPos1 += 2;
            continue;
        }
        if( u2 < l1 )
        {
            
            nPos2 += 2;
            continue;
        }

        

        if( l1 <= l2 )
        {
            

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
    }; 
    pTarget[ nTargetPos ] = 0;

    
    delete[] _pRanges;

    sal_uInt16 nUShorts = Count_Impl(pTarget) + 1;
    if ( 1 != nUShorts )
    {
        _pRanges = new sal_uInt16[ nUShorts ];
        memcpy( _pRanges, pTarget, nUShorts * sizeof(sal_uInt16) );
    }
    else
        _pRanges = 0;

    delete [] pTarget;
    return *this;
}


sal_uInt16 SfxUShortRanges::Count() const

/** <H3>Description</H3>

    Determines the number of USHORTs in the set described by the ranges
    of USHORTs in '*this'.
*/

{
    return Capacity_Impl( _pRanges );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
