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
#pragma once

#include <sal/types.h>
#include "swdllapi.h"

#include <iostream>

class SwContentNode;
class SwContentIndexReg;
struct SwPosition;

namespace sw::mark { class IMark; }

/// Marks a character position inside a document model content node (SwContentNode)
class SAL_WARN_UNUSED SW_DLLPUBLIC SwContentIndex
{
private:
    friend class SwContentIndexReg;

    sal_Int32 m_nIndex;
    SwContentNode * m_pContentNode;
    // doubly linked list of Indexes registered at m_pIndexReg
    SwContentIndex * m_pNext;
    SwContentIndex * m_pPrev;

    /// Pointer to a mark that owns this position to allow fast lookup of marks of an SwContentIndexReg.
    const sw::mark::IMark* m_pMark;

    SwContentIndex& ChgValue( const SwContentIndex& rIdx, sal_Int32 nNewValue );
    void Init(sal_Int32 const nIdx);
    void Remove();

public:
    explicit SwContentIndex(SwContentNode *const pContentNode, sal_Int32 const nIdx = 0);
    SwContentIndex( const SwContentIndex & );
    SwContentIndex( const SwContentIndex &, short nDiff );
    ~SwContentIndex() { Remove(); }

    SwContentIndex& operator=( sal_Int32 const );
    SwContentIndex& operator=( const SwContentIndex & );

    sal_Int32 operator++();
    sal_Int32 operator--();
    sal_Int32 operator--(int);

    sal_Int32 operator+=( sal_Int32 const );
    sal_Int32 operator-=( sal_Int32 const );

    bool operator< ( const SwContentIndex& ) const;
    bool operator<=( const SwContentIndex& ) const;
    bool operator> ( const SwContentIndex& ) const;
    bool operator>=( const SwContentIndex& ) const;

    bool operator< ( sal_Int32 const nVal ) const { return m_nIndex <  nVal; }
    bool operator<=( sal_Int32 const nVal ) const { return m_nIndex <= nVal; }
    bool operator> ( sal_Int32 const nVal ) const { return m_nIndex >  nVal; }
    bool operator>=( sal_Int32 const nVal ) const { return m_nIndex >= nVal; }
    bool operator==( sal_Int32 const nVal ) const { return m_nIndex == nVal; }
    bool operator!=( sal_Int32 const nVal ) const { return m_nIndex != nVal; }

    bool operator==( const SwContentIndex& rSwContentIndex ) const
    {
        return (m_nIndex    == rSwContentIndex.m_nIndex)
            && (m_pContentNode == rSwContentIndex.m_pContentNode);
    }

    bool operator!=( const SwContentIndex& rSwContentIndex ) const
    {
        return (m_nIndex    != rSwContentIndex.m_nIndex)
            || (m_pContentNode != rSwContentIndex.m_pContentNode);
    }

    sal_Int32 GetIndex() const { return m_nIndex; }

    // Assignments without creating a temporary object.
    SwContentIndex &Assign(SwContentNode *, sal_Int32);

    // Returns pointer to SwContentNode (for RTTI at SwContentIndexReg).
    const SwContentNode* GetContentNode() const { return m_pContentNode; }
    const SwContentIndex* GetNext() const { return m_pNext; }

    const sw::mark::IMark* GetMark() const { return m_pMark; }
    void SetMark(const sw::mark::IMark* pMark);
};

SW_DLLPUBLIC std::ostream& operator <<(std::ostream& s, const SwContentIndex& index);

/// Helper base class for SwContentNode to manage the list of attached SwContentIndex
class SAL_WARN_UNUSED SwContentIndexReg
{
    friend class SwContentIndex;
    friend bool sw_PosOk(const SwPosition & aPos);

    const SwContentIndex * m_pFirst;
    const SwContentIndex * m_pLast;

protected:
    virtual void Update( SwContentIndex const & rPos, const sal_Int32 nChangeLen,
                 const bool bNegative = false, const bool bDelete = false );

    bool HasAnyIndex() const { return nullptr != m_pFirst; }

    SwContentIndexReg();
public:
    virtual ~SwContentIndexReg();

    void MoveTo( SwContentNode& rArr );
    const SwContentIndex* GetFirstIndex() const { return m_pFirst; }
};

#ifndef DBG_UTIL

inline sal_Int32 SwContentIndex::operator++()
{
    return ChgValue( *this, m_nIndex+1 ).m_nIndex;
}

inline sal_Int32 SwContentIndex::operator--()
{
    return ChgValue( *this, m_nIndex-1 ).m_nIndex;
}

inline sal_Int32 SwContentIndex::operator--(int)
{
    sal_Int32 const nOldIndex = m_nIndex;
    ChgValue( *this, m_nIndex-1 );
    return nOldIndex;
}

inline sal_Int32 SwContentIndex::operator+=( sal_Int32 const nVal )
{
    return ChgValue( *this, m_nIndex + nVal ).m_nIndex;
}

inline sal_Int32 SwContentIndex::operator-=( sal_Int32 const nVal )
{
    return ChgValue( *this, m_nIndex - nVal ).m_nIndex;
}

inline bool SwContentIndex::operator< ( const SwContentIndex& rIndex ) const
{
    return m_nIndex <  rIndex.m_nIndex;
}

inline bool SwContentIndex::operator<=( const SwContentIndex& rIndex ) const
{
    return m_nIndex <= rIndex.m_nIndex;
}

inline bool SwContentIndex::operator> ( const SwContentIndex& rIndex ) const
{
    return m_nIndex >  rIndex.m_nIndex;
}

inline bool SwContentIndex::operator>=( const SwContentIndex& rIndex ) const
{
    return m_nIndex >= rIndex.m_nIndex;
}

inline SwContentIndex& SwContentIndex::operator= ( sal_Int32 const nVal )
{
    if (m_nIndex != nVal)
    {
        ChgValue( *this, nVal );
    }
    return *this;
}

#endif // ifndef DBG_UTIL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
