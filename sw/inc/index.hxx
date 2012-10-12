/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef SW_INDEX_HXX
#define SW_INDEX_HXX

#include <limits.h>

#include "rtl/instance.hxx"
#include <tools/solar.h>
#include <tools/rtti.hxx>               // for RTTI of SwIndexReg
#include <swdllapi.h>

// Maximum index in IndexArray (for testing on overflows).
#define INVALID_INDEX STRING_NOTFOUND

class SwIndex;
class SwIndexReg;
struct SwPosition;

#ifdef DBG_UTIL
#define INLINE
#else
#define INLINE inline
#endif

/// Marks a character position inside a document model node.
class SW_DLLPUBLIC SwIndex
{
private:
    friend class SwIndexReg;

    xub_StrLen m_nIndex;
    SwIndexReg * m_pIndexReg;
    // doubly linked list of Indexes registered at m_pIndexReg
    SwIndex * m_pNext;
    SwIndex * m_pPrev;

    SwIndex& ChgValue( const SwIndex& rIdx, xub_StrLen nNewValue );
    void Init(xub_StrLen const nIdx);
    void Remove();

public:
    explicit SwIndex(SwIndexReg *const pReg, xub_StrLen const nIdx = 0);
    SwIndex( const SwIndex & );
    SwIndex( const SwIndex &, short nDiff );
    ~SwIndex() { Remove(); }

    INLINE SwIndex& operator=( xub_StrLen const );
    SwIndex& operator=( const SwIndex & );

    INLINE xub_StrLen operator++();
    INLINE xub_StrLen operator--();
    INLINE xub_StrLen operator++(int);
    INLINE xub_StrLen operator--(int);

    INLINE xub_StrLen operator+=( xub_StrLen const );
    INLINE xub_StrLen operator-=( xub_StrLen const );
    INLINE xub_StrLen operator+=( const SwIndex& );
    INLINE xub_StrLen operator-=( const SwIndex& );

    INLINE bool operator< ( const SwIndex& ) const;
    INLINE bool operator<=( const SwIndex& ) const;
    INLINE bool operator> ( const SwIndex& ) const;
    INLINE bool operator>=( const SwIndex& ) const;

    bool operator< ( xub_StrLen const nVal ) const { return m_nIndex <  nVal; }
    bool operator<=( xub_StrLen const nVal ) const { return m_nIndex <= nVal; }
    bool operator> ( xub_StrLen const nVal ) const { return m_nIndex >  nVal; }
    bool operator>=( xub_StrLen const nVal ) const { return m_nIndex >= nVal; }
    bool operator==( xub_StrLen const nVal ) const { return m_nIndex == nVal; }
    bool operator!=( xub_StrLen const nVal ) const { return m_nIndex != nVal; }

    bool operator==( const SwIndex& rSwIndex ) const
    {
        return (m_nIndex    == rSwIndex.m_nIndex)
            && (m_pIndexReg == rSwIndex.m_pIndexReg);
    }

    bool operator!=( const SwIndex& rSwIndex ) const
    {
        return (m_nIndex    != rSwIndex.m_nIndex)
            || (m_pIndexReg != rSwIndex.m_pIndexReg);
    }

    xub_StrLen GetIndex() const { return m_nIndex; }

    // Assignments without creating a temporary object.
    SwIndex &Assign(SwIndexReg *,xub_StrLen);

    // Returns pointer to IndexArray (for RTTI at SwIndexReg).
    const SwIndexReg* GetIdxReg() const { return m_pIndexReg; }
};

#undef INLINE

class SwIndexReg
{
    friend class SwIndex;
    friend bool sw_PosOk(const SwPosition & aPos);

    const SwIndex * m_pFirst;
    const SwIndex * m_pLast;

protected:
    virtual void Update( SwIndex const & rPos, const xub_StrLen nChangeLen,
                 const bool bNegative = false, const bool bDelete = false );

    void ChkArr();

    bool HasAnyIndex() const { return 0 != m_pFirst; }

public:
    explicit SwIndexReg();
    virtual ~SwIndexReg();

    // rtti, abgeleitete moegens gleichtun oder nicht. Wenn sie es gleichtun
    // kann ueber das SwIndexReg typsicher gecastet werden.
    TYPEINFO();

    void MoveTo( SwIndexReg& rArr );
};


#ifndef DBG_UTIL

inline xub_StrLen SwIndex::operator++()
{
    return ChgValue( *this, m_nIndex+1 ).m_nIndex;
}
inline xub_StrLen SwIndex::operator--()
{
    return ChgValue( *this, m_nIndex-1 ).m_nIndex;
}
inline xub_StrLen SwIndex::operator++(int)
{
    xub_StrLen const nOldIndex = m_nIndex;
    ChgValue( *this, m_nIndex+1 );
    return nOldIndex;
}
inline xub_StrLen SwIndex::operator--(int)
{
    xub_StrLen const nOldIndex = m_nIndex;
    ChgValue( *this, m_nIndex-1 );
    return nOldIndex;
}

inline xub_StrLen SwIndex::operator+=( xub_StrLen const nVal )
{
    return ChgValue( *this, m_nIndex + nVal ).m_nIndex;
}
inline xub_StrLen SwIndex::operator-=( xub_StrLen const nVal )
{
    return ChgValue( *this, m_nIndex - nVal ).m_nIndex;
}
inline xub_StrLen SwIndex::operator+=( const SwIndex& rIndex )
{
    return ChgValue( *this, m_nIndex + rIndex.m_nIndex ).m_nIndex;
}
inline xub_StrLen SwIndex::operator-=( const SwIndex& rIndex )
{
    return ChgValue( *this, m_nIndex - rIndex.m_nIndex ).m_nIndex;
}

inline bool SwIndex::operator< ( const SwIndex& rIndex ) const
{
    return m_nIndex <  rIndex.m_nIndex;
}
inline bool SwIndex::operator<=( const SwIndex& rIndex ) const
{
    return m_nIndex <= rIndex.m_nIndex;
}
inline bool SwIndex::operator> ( const SwIndex& rIndex ) const
{
    return m_nIndex >  rIndex.m_nIndex;
}
inline bool SwIndex::operator>=( const SwIndex& rIndex ) const
{
    return m_nIndex >= rIndex.m_nIndex;
}
inline SwIndex& SwIndex::operator= ( xub_StrLen const nVal )
{
    if (m_nIndex != nVal)
    {
        ChgValue( *this, nVal );
    }
    return *this;
}

#endif // ifndef DBG_UTIL

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
