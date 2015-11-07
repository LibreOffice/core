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
#ifndef INCLUDED_SW_INC_INDEX_HXX
#define INCLUDED_SW_INC_INDEX_HXX

#include <sal/types.h>
#include <swdllapi.h>

#include <iostream>

class SwIndexReg;
struct SwPosition;

namespace sw {
namespace mark {
class IMark;
}
}

/// Marks a character position inside a document model node.
class SW_DLLPUBLIC SwIndex
{
private:
    friend class SwIndexReg;

    sal_Int32 m_nIndex;
    SwIndexReg * m_pIndexReg;
    // doubly linked list of Indexes registered at m_pIndexReg
    SwIndex * m_pNext;
    SwIndex * m_pPrev;

    /// Pointer to a mark that owns this position to allow fast lookup of marks of an SwIndexReg.
    const sw::mark::IMark* m_pMark;

    SwIndex& ChgValue( const SwIndex& rIdx, sal_Int32 nNewValue );
    void Init(sal_Int32 const nIdx);
    void Remove();

public:
    explicit SwIndex(SwIndexReg *const pReg, sal_Int32 const nIdx = 0);
    SwIndex( const SwIndex & );
    SwIndex( const SwIndex &, short nDiff );
    ~SwIndex() { Remove(); }

    SwIndex& operator=( sal_Int32 const );
    SwIndex& operator=( const SwIndex & );

    sal_Int32 operator++();
    sal_Int32 operator--();
    sal_Int32 operator++(int);
    sal_Int32 operator--(int);

    sal_Int32 operator+=( sal_Int32 const );
    sal_Int32 operator-=( sal_Int32 const );
    sal_Int32 operator+=( const SwIndex& );
    sal_Int32 operator-=( const SwIndex& );

    bool operator< ( const SwIndex& ) const;
    bool operator<=( const SwIndex& ) const;
    bool operator> ( const SwIndex& ) const;
    bool operator>=( const SwIndex& ) const;

    bool operator< ( sal_Int32 const nVal ) const { return m_nIndex <  nVal; }
    bool operator<=( sal_Int32 const nVal ) const { return m_nIndex <= nVal; }
    bool operator> ( sal_Int32 const nVal ) const { return m_nIndex >  nVal; }
    bool operator>=( sal_Int32 const nVal ) const { return m_nIndex >= nVal; }
    bool operator==( sal_Int32 const nVal ) const { return m_nIndex == nVal; }
    bool operator!=( sal_Int32 const nVal ) const { return m_nIndex != nVal; }

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

    sal_Int32 GetIndex() const { return m_nIndex; }

    // Assignments without creating a temporary object.
    SwIndex &Assign(SwIndexReg *, sal_Int32);

    // Returns pointer to IndexArray (for RTTI at SwIndexReg).
    const SwIndexReg* GetIdxReg() const { return m_pIndexReg; }
    const SwIndex* GetNext() const { return m_pNext; }

    const sw::mark::IMark* GetMark() const { return m_pMark; }
    void SetMark(const sw::mark::IMark* pMark);
};

SW_DLLPUBLIC std::ostream& operator <<(std::ostream& s, const SwIndex& index);

class SwIndexReg
{
    friend class SwIndex;
    friend bool sw_PosOk(const SwPosition & aPos);

    const SwIndex * m_pFirst;
    const SwIndex * m_pLast;

protected:
    virtual void Update( SwIndex const & rPos, const sal_Int32 nChangeLen,
                 const bool bNegative = false, const bool bDelete = false );

    bool HasAnyIndex() const { return 0 != m_pFirst; }

public:
    SwIndexReg();
    virtual ~SwIndexReg();

    void MoveTo( SwIndexReg& rArr );
    const SwIndex* GetFirstIndex() const { return m_pFirst; }
};

#ifndef DBG_UTIL

inline sal_Int32 SwIndex::operator++()
{
    return ChgValue( *this, m_nIndex+1 ).m_nIndex;
}

inline sal_Int32 SwIndex::operator--()
{
    return ChgValue( *this, m_nIndex-1 ).m_nIndex;
}

inline sal_Int32 SwIndex::operator++(int)
{
    sal_Int32 const nOldIndex = m_nIndex;
    ChgValue( *this, m_nIndex+1 );
    return nOldIndex;
}

inline sal_Int32 SwIndex::operator--(int)
{
    sal_Int32 const nOldIndex = m_nIndex;
    ChgValue( *this, m_nIndex-1 );
    return nOldIndex;
}

inline sal_Int32 SwIndex::operator+=( sal_Int32 const nVal )
{
    return ChgValue( *this, m_nIndex + nVal ).m_nIndex;
}

inline sal_Int32 SwIndex::operator-=( sal_Int32 const nVal )
{
    return ChgValue( *this, m_nIndex - nVal ).m_nIndex;
}

inline sal_Int32 SwIndex::operator+=( const SwIndex& rIndex )
{
    return ChgValue( *this, m_nIndex + rIndex.m_nIndex ).m_nIndex;
}

inline sal_Int32 SwIndex::operator-=( const SwIndex& rIndex )
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

inline SwIndex& SwIndex::operator= ( sal_Int32 const nVal )
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
