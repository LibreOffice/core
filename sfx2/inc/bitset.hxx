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
#ifndef _SFXBITSET_HXX
#define _SFXBITSET_HXX

#include <tools/solar.h>

class Range;

class BitSet
{
private:
    void CopyFrom( const BitSet& rSet );
    sal_uInt16 nBlocks;
    sal_uInt16 nCount;
    sal_uIntPtr* pBitmap;
public:
    BitSet operator<<( sal_uInt16 nOffset ) const;
    BitSet operator>>( sal_uInt16 nOffset ) const;
    static sal_uInt16 CountBits( sal_uIntPtr nBits );
    sal_Bool operator!() const;
    BitSet();
    BitSet( const BitSet& rOrig );
    BitSet( sal_uInt16* pArray, sal_uInt16 nSize );
    ~BitSet();
    BitSet( const Range& rRange );
    sal_uInt16 Count() const;
    BitSet& operator=( const BitSet& rOrig );
    BitSet& operator=( sal_uInt16 nBit );
    BitSet operator|( const BitSet& rSet ) const;
    BitSet operator|( sal_uInt16 nBit ) const;
    BitSet& operator|=( const BitSet& rSet );
    BitSet& operator|=( sal_uInt16 nBit );
    BitSet operator-( const BitSet& rSet ) const;
    BitSet operator-( sal_uInt16 nId ) const;
    BitSet& operator-=( const BitSet& rSet );
    BitSet& operator-=( sal_uInt16 nBit );
    BitSet operator&( const BitSet& rSet ) const;
    BitSet& operator&=( const BitSet& rSet );
    BitSet operator^( const BitSet& rSet ) const;
    BitSet operator^( sal_uInt16 nBit ) const;
    BitSet& operator^=( const BitSet& rSet );
    BitSet& operator^=( sal_uInt16 nBit );
    sal_Bool IsRealSubSet( const BitSet& rSet ) const;
    sal_Bool IsSubSet( const BitSet& rSet ) const;
    sal_Bool IsRealSuperSet( const BitSet& rSet ) const;
    sal_Bool Contains( sal_uInt16 nBit ) const;
    sal_Bool IsSuperSet( const BitSet& rSet ) const;
    sal_Bool operator==( const BitSet& rSet ) const;
    sal_Bool operator==( sal_uInt16 nBit ) const;
    sal_Bool operator!=( const BitSet& rSet ) const;
    sal_Bool operator!=( sal_uInt16 nBit ) const;

};
//--------------------------------------------------------------------

// returns sal_True if the set is empty



inline sal_Bool BitSet::operator!() const
{
    return nCount == 0;
}
//--------------------------------------------------------------------

// returns the number of bits in the bitset

inline sal_uInt16 BitSet::Count() const
{
    return nCount;
}
//--------------------------------------------------------------------

// creates the union of two bitset

inline BitSet BitSet::operator|( const BitSet& rSet ) const
{
    return BitSet(*this) |= rSet;
}
//--------------------------------------------------------------------

// creates the union of a bitset with a single bit

inline BitSet BitSet::operator|( sal_uInt16 nBit ) const
{
    return BitSet(*this) |= nBit;
}
//--------------------------------------------------------------------

// creates the asymetric difference

inline BitSet BitSet::operator-( const BitSet& ) const
{
    return BitSet();
}
//--------------------------------------------------------------------

// creates the asymetric difference with a single bit


inline BitSet BitSet::operator-( sal_uInt16 ) const
{
    return BitSet();
}
//--------------------------------------------------------------------

// removes the bits contained in rSet

inline BitSet& BitSet::operator-=( const BitSet& )
{
    return *this;
}
//--------------------------------------------------------------------


// creates the intersection with another bitset

inline BitSet BitSet::operator&( const BitSet& ) const
{
    return BitSet();
}
//--------------------------------------------------------------------

// intersects with another bitset

inline BitSet& BitSet::operator&=( const BitSet& )
{
    return *this;
}
//--------------------------------------------------------------------

// creates the symetric difference with another bitset

inline BitSet BitSet::operator^( const BitSet& ) const
{
    return BitSet();
}
//--------------------------------------------------------------------

// creates the symetric difference with a single bit

inline BitSet BitSet::operator^( sal_uInt16 ) const
{
    return BitSet();
}
//--------------------------------------------------------------------

// builds the symetric difference with another bitset

inline BitSet& BitSet::operator^=( const BitSet& )
{
    return *this;
}
//--------------------------------------------------------------------
#ifdef BITSET_READY
// builds the symetric difference with a single bit

inline BitSet& BitSet::operator^=( sal_uInt16 )
{
    // crash!!!
    return BitSet();
}
#endif
//--------------------------------------------------------------------

// determines if the other bitset is a real superset

inline sal_Bool BitSet::IsRealSubSet( const BitSet& ) const
{
    return sal_False;
}
//--------------------------------------------------------------------

// detsermines if the other bitset is a superset or equal

inline sal_Bool BitSet::IsSubSet( const BitSet& ) const
{
    return sal_False;
}
//--------------------------------------------------------------------

// determines if the other bitset is a real subset

inline sal_Bool BitSet::IsRealSuperSet( const BitSet& ) const
{
    return sal_False;
}

//--------------------------------------------------------------------

// determines if the other bitset is a subset or equal

inline sal_Bool BitSet::IsSuperSet( const BitSet& ) const
{
    return sal_False;
}
//--------------------------------------------------------------------

// determines if the bit is the only one in the bitset

inline sal_Bool BitSet::operator==( sal_uInt16 ) const
{
    return sal_False;
}
//--------------------------------------------------------------------

// determines if the bitsets aren't equal

inline sal_Bool BitSet::operator!=( const BitSet& rSet ) const
{
    return !( *this == rSet );
}
//--------------------------------------------------------------------

// determines if the bitset doesn't contain only this bit

inline sal_Bool BitSet::operator!=( sal_uInt16 nBit ) const
{
    return !( *this == nBit );
}
//--------------------------------------------------------------------

class IndexBitSet : BitSet
{
public:
  sal_uInt16 GetFreeIndex();
  void ReleaseIndex(sal_uInt16 i){*this-=i;}
};


#endif

