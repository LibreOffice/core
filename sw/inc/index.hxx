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

    /// rtti, derived classes might do the same. If so, one can cast typesavely
    /// via SwIndexReg.
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
