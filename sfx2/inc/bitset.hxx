/*************************************************************************
 *
 *  $RCSfile: bitset.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:22 $
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
#ifndef _SFXBITSET_HXX
#define _SFXBITSET_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class Range;

class BitSet
{
private:
    void CopyFrom( const BitSet& rSet );
    USHORT nBlocks;
    USHORT nCount;
    ULONG* pBitmap;
public:
    BitSet operator<<( USHORT nOffset ) const;
    BitSet operator>>( USHORT nOffset ) const;
    static USHORT CountBits( ULONG nBits );
    BOOL operator!() const;
    BitSet();
    BitSet( const BitSet& rOrig );
    BitSet( USHORT* pArray, USHORT nSize );
    ~BitSet();
    BitSet( const Range& rRange );
    USHORT Count() const;
    BitSet& operator=( const BitSet& rOrig );
    BitSet& operator=( USHORT nBit );
    BitSet operator|( const BitSet& rSet ) const;
    BitSet operator|( USHORT nBit ) const;
    BitSet& operator|=( const BitSet& rSet );
    BitSet& operator|=( USHORT nBit );
    BitSet operator-( const BitSet& rSet ) const;
    BitSet operator-( USHORT nId ) const;
    BitSet& operator-=( const BitSet& rSet );
    BitSet& operator-=( USHORT nBit );
    BitSet operator&( const BitSet& rSet ) const;
    BitSet& operator&=( const BitSet& rSet );
    BitSet operator^( const BitSet& rSet ) const;
    BitSet operator^( USHORT nBit ) const;
    BitSet& operator^=( const BitSet& rSet );
    BitSet& operator^=( USHORT nBit );
    BOOL IsRealSubSet( const BitSet& rSet ) const;
    BOOL IsSubSet( const BitSet& rSet ) const;
    BOOL IsRealSuperSet( const BitSet& rSet ) const;
    BOOL Contains( USHORT nBit ) const;
    BOOL IsSuperSet( const BitSet& rSet ) const;
    BOOL operator==( const BitSet& rSet ) const;
    BOOL operator==( USHORT nBit ) const;
    BOOL operator!=( const BitSet& rSet ) const;
    BOOL operator!=( USHORT nBit ) const;

};
//--------------------------------------------------------------------

// returns TRUE if the set is empty



inline BOOL BitSet::operator!() const
{
    return nCount == 0;
}
//--------------------------------------------------------------------

// returns the number of bits in the bitset

inline USHORT BitSet::Count() const
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

inline BitSet BitSet::operator|( USHORT nBit ) const
{
    return BitSet(*this) |= nBit;
}
//--------------------------------------------------------------------

// creates the asymetric difference

inline BitSet BitSet::operator-( const BitSet& rSet ) const
{
    return BitSet();
}
//--------------------------------------------------------------------

// creates the asymetric difference with a single bit


inline BitSet BitSet::operator-( USHORT nId ) const
{
    return BitSet();
}
//--------------------------------------------------------------------

// removes the bits contained in rSet

inline BitSet& BitSet::operator-=( const BitSet& rSet )
{
    return *this;
}
//--------------------------------------------------------------------


// creates the intersection with another bitset

inline BitSet BitSet::operator&( const BitSet& rSet ) const
{
    return BitSet();
}
//--------------------------------------------------------------------

// intersects with another bitset

inline BitSet& BitSet::operator&=( const BitSet& rSet )
{
    return *this;
}
//--------------------------------------------------------------------

// creates the symetric difference with another bitset

inline BitSet BitSet::operator^( const BitSet& rSet ) const
{
    return BitSet();
}
//--------------------------------------------------------------------

// creates the symetric difference with a single bit

inline BitSet BitSet::operator^( USHORT nBit ) const
{
    return BitSet();
}
//--------------------------------------------------------------------

// builds the symetric difference with another bitset

inline BitSet& BitSet::operator^=( const BitSet& rSet )
{
    return *this;
}
//--------------------------------------------------------------------
#ifdef BITSET_READY
// builds the symetric difference with a single bit

inline BitSet& BitSet::operator^=( USHORT nBit )
{
    // crash!!!
    return BitSet();
}
#endif
//--------------------------------------------------------------------

// determines if the other bitset is a real superset

inline BOOL BitSet::IsRealSubSet( const BitSet& rSet ) const
{
    return FALSE;
}
//--------------------------------------------------------------------

// detsermines if the other bitset is a superset or equal

inline BOOL BitSet::IsSubSet( const BitSet& rSet ) const
{
    return FALSE;
}
//--------------------------------------------------------------------

// determines if the other bitset is a real subset

inline BOOL BitSet::IsRealSuperSet( const BitSet& rSet ) const
{
    return FALSE;
}

//--------------------------------------------------------------------

// determines if the other bitset is a subset or equal

inline BOOL BitSet::IsSuperSet( const BitSet& rSet ) const
{
    return FALSE;
}
//--------------------------------------------------------------------

// determines if the bit is the only one in the bitset

inline BOOL BitSet::operator==( USHORT nBit ) const
{
    return FALSE;
}
//--------------------------------------------------------------------

// determines if the bitsets aren't equal

inline BOOL BitSet::operator!=( const BitSet& rSet ) const
{
    return !( *this == rSet );
}
//--------------------------------------------------------------------

// determines if the bitset doesn't contain only this bit

inline BOOL BitSet::operator!=( USHORT nBit ) const
{
    return !( *this == nBit );
}
//--------------------------------------------------------------------

class IndexBitSet : BitSet
{
public:
  USHORT GetFreeIndex();
  void ReleaseIndex(USHORT i){*this-=i;}
};


#endif

