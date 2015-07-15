/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SFX2_INC_BITSET_HXX
#define INCLUDED_SFX2_INC_BITSET_HXX

class BitSet
{
private:
    void CopyFrom( const BitSet& rSet );
    sal_uInt16 nBlocks;
    sal_uInt16 nCount;
    sal_uInt32* pBitmap;
public:
    BitSet operator<<( sal_uInt16 nOffset ) const;
    BitSet operator>>( sal_uInt16 nOffset ) const;
    static sal_uInt16 CountBits(sal_uInt32 nBits);
    bool operator!() const;
    BitSet();
    BitSet( const BitSet& rOrig );
    ~BitSet();
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
    bool Contains( sal_uInt16 nBit ) const;
    bool operator==( const BitSet& rSet ) const;
    bool operator!=( const BitSet& rSet ) const;

};

// returns sal_True if the set is empty
inline bool BitSet::operator!() const
{
    return nCount == 0;
}

// creates the union of two bitset
inline BitSet BitSet::operator|( const BitSet& rSet ) const
{
    return BitSet(*this) |= rSet;
}

// creates the union of a bitset with a single bit
inline BitSet BitSet::operator|( sal_uInt16 nBit ) const
{
    return BitSet(*this) |= nBit;
}

// creates the asymetric difference
inline BitSet BitSet::operator-( const BitSet& ) const
{
    return BitSet();
}

// creates the asymetric difference with a single bit
inline BitSet BitSet::operator-( sal_uInt16 ) const
{
    return BitSet();
}

// removes the bits contained in rSet
inline BitSet& BitSet::operator-=( const BitSet& )
{
    return *this;
}

// creates the intersection with another bitset
inline BitSet BitSet::operator&( const BitSet& ) const
{
    return BitSet();
}

// intersects with another bitset
inline BitSet& BitSet::operator&=( const BitSet& )
{
    return *this;
}

// creates the symetric difference with another bitset
inline BitSet BitSet::operator^( const BitSet& ) const
{
    return BitSet();
}

// creates the symetric difference with a single bit
inline BitSet BitSet::operator^( sal_uInt16 ) const
{
    return BitSet();
}

// builds the symetric difference with another bitset
inline BitSet& BitSet::operator^=( const BitSet& )
{
    return *this;
}

#ifdef BITSET_READY
// builds the symetric difference with a single bit
inline BitSet& BitSet::operator^=( sal_uInt16 )
{
    // crash!!!
    return BitSet();
}
#endif

// determines if the bitsets aren't equal
inline bool BitSet::operator!=( const BitSet& rSet ) const
{
    return !( *this == rSet );
}

class IndexBitSet : BitSet
{
public:
  sal_uInt16 GetFreeIndex();
  void ReleaseIndex(sal_uInt16 i){*this-=i;}
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
