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
#ifndef INCLUDED_SW_INC_NDINDEX_HXX
#define INCLUDED_SW_INC_NDINDEX_HXX

#include <limits.h>
#include <iostream>

#include <tools/solar.h>

#include <node.hxx>
#include <ring.hxx>
#include <ndarr.hxx>

class SwNode;
class SwNodes;

/// Marks a node in the document model.
class SW_DLLPUBLIC SwNodeIndex final : public sw::Ring<SwNodeIndex>
{
    SwNode * m_pNode;

    // These are not allowed!
    SwNodeIndex( SwNodes& rNds, sal_uInt16 nIdx ) = delete;
    SwNodeIndex( SwNodes& rNds, int nIdx ) = delete;
    void RegisterIndex( SwNodes& rNodes )
    {
        if(!rNodes.m_vIndices)
            rNodes.m_vIndices = this;
        MoveTo(rNodes.m_vIndices);
    }
    void DeRegisterIndex( SwNodes& rNodes )
    {
        if(rNodes.m_vIndices == this)
            rNodes.m_vIndices = GetNextInRing();
        MoveTo(nullptr);
        if(rNodes.m_vIndices == this)
            rNodes.m_vIndices = nullptr;
    }

public:
    SwNodeIndex( SwNodes& rNds, sal_uLong nIdx = 0 )
        : m_pNode( rNds[ nIdx ] )
    {
        RegisterIndex( rNds );
    };
    SwNodeIndex( const SwNodeIndex& rIdx, long nDiff = 0 )
        : sw::Ring<SwNodeIndex>()
    {
        if( nDiff )
            m_pNode = rIdx.GetNodes()[ rIdx.GetIndex() + nDiff ];
        else
            m_pNode = rIdx.m_pNode;
        RegisterIndex( m_pNode->GetNodes() );
    }

    SwNodeIndex( const SwNode& rNd, long nDiff = 0 )
    {
        if( nDiff )
            m_pNode = rNd.GetNodes()[ rNd.GetIndex() + nDiff ];
        else
            m_pNode = const_cast<SwNode*>(&rNd);
        RegisterIndex( m_pNode->GetNodes() );
    }

    virtual  ~SwNodeIndex() override
        { DeRegisterIndex( m_pNode->GetNodes() ); }

    inline sal_uLong operator++();
    inline sal_uLong operator--();
    inline sal_uLong operator++(int);
    inline sal_uLong operator--(int);

    inline sal_uLong operator+=( sal_uLong );
    inline sal_uLong operator-=( sal_uLong );

    inline bool operator< ( const SwNodeIndex& ) const;
    inline bool operator<=( const SwNodeIndex& ) const;
    inline bool operator> ( const SwNodeIndex& ) const;
    inline bool operator>=( const SwNodeIndex& ) const;
    inline bool operator==( const SwNodeIndex& ) const;
    inline bool operator!=( const SwNodeIndex& ) const;

    inline bool operator< ( sal_uLong ) const;
    inline bool operator<=( sal_uLong ) const;
    inline bool operator> ( sal_uLong ) const;
    inline bool operator>=( sal_uLong ) const;
    inline bool operator==( sal_uLong ) const;
    inline bool operator!=( sal_uLong ) const;

    inline SwNodeIndex& operator=( sal_uLong );
    inline SwNodeIndex& operator=( const SwNodeIndex& );
    inline SwNodeIndex& operator=( const SwNode& );

    // Return value of index as sal_uLong.
    inline sal_uLong GetIndex() const;

    // Enables assignments without creation of a temporary object.
    inline SwNodeIndex& Assign( SwNodes const & rNds, sal_uLong );
    inline SwNodeIndex& Assign( const SwNode& rNd, long nOffset = 0 );

    // Gets pointer on NodesArray.
    inline const SwNodes& GetNodes() const;
    inline       SwNodes& GetNodes();

    SwNode& GetNode() const { return *m_pNode; }
};

inline std::ostream &operator <<(std::ostream& s, const SwNodeIndex& index)
{
    return s << "SwNodeIndex (node " << index.GetIndex() << ")";
};

// SwRange

class SwNodeRange
{
public:
    SwNodeIndex aStart;
    SwNodeIndex aEnd;

    SwNodeRange( const SwNodeIndex &rS, const SwNodeIndex &rE )
        : aStart( rS ), aEnd( rE ) {};
    SwNodeRange( const SwNodeRange &rRange )
        : aStart( rRange.aStart ), aEnd( rRange.aEnd ) {};

    SwNodeRange( SwNodes& rNds, sal_uLong nSttIdx, sal_uLong nEndIdx = 0 )
        : aStart( rNds, nSttIdx ), aEnd( rNds, nEndIdx ) {};

    SwNodeRange( const SwNodeIndex& rS, long nSttDiff, const SwNodeIndex& rE, long nEndDiff = 0 )
        : aStart( rS, nSttDiff ), aEnd( rE, nEndDiff ) {};
    SwNodeRange( const SwNode& rS, long nSttDiff, const SwNode& rE, long nEndDiff = 0 )
        : aStart( rS, nSttDiff ), aEnd( rE, nEndDiff ) {};
};

// For inlines node.hxx is needed which in turn needs this one.
// Therefore all inlines accessing m_pNode are implemented here.

inline sal_uLong SwNodeIndex::GetIndex() const
{
    return m_pNode->GetIndex();
}
inline const SwNodes& SwNodeIndex::GetNodes() const
{
    return m_pNode->GetNodes();
}
inline SwNodes& SwNodeIndex::GetNodes()
{
    return m_pNode->GetNodes();
}
inline bool SwNodeIndex::operator< ( sal_uLong const nOther ) const
{
    return m_pNode->GetIndex() < nOther;
}
inline bool SwNodeIndex::operator<=( sal_uLong const nOther ) const
{
    return m_pNode->GetIndex() <= nOther;
}
inline bool SwNodeIndex::operator> ( sal_uLong const nOther ) const
{
    return m_pNode->GetIndex() > nOther;
}
inline bool SwNodeIndex::operator>=( sal_uLong const nOther ) const
{
    return m_pNode->GetIndex() >= nOther;
}
inline bool SwNodeIndex::operator==( sal_uLong const nOther ) const
{
    return m_pNode->GetIndex() == nOther;
}
inline bool SwNodeIndex::operator!=( sal_uLong const nOther ) const
{
    return m_pNode->GetIndex() != nOther;
}
inline bool SwNodeIndex::operator<( const SwNodeIndex& rIndex ) const
{
    return m_pNode->GetIndex() < rIndex.GetIndex();
}
inline bool SwNodeIndex::operator<=( const SwNodeIndex& rIndex ) const
{
    return m_pNode->GetIndex() <= rIndex.GetIndex();
}
inline bool SwNodeIndex::operator>( const SwNodeIndex& rIndex ) const
{
    return m_pNode->GetIndex() > rIndex.GetIndex();
}
inline bool SwNodeIndex::operator>=( const SwNodeIndex& rIndex ) const
{
    return m_pNode->GetIndex() >= rIndex.GetIndex();
}
inline bool SwNodeIndex::operator==( const SwNodeIndex& rIdx ) const
{
    return m_pNode == rIdx.m_pNode;
}
inline bool SwNodeIndex::operator!=( const SwNodeIndex& rIdx ) const
{
    return m_pNode != rIdx.m_pNode;
}

inline sal_uLong SwNodeIndex::operator++()
{
    m_pNode = GetNodes()[ m_pNode->GetIndex()+1 ];
    return m_pNode->GetIndex();
}
inline sal_uLong SwNodeIndex::operator--()
{
    m_pNode = GetNodes()[ m_pNode->GetIndex()-1 ];
    return m_pNode->GetIndex();
}
inline sal_uLong SwNodeIndex::operator++(int)
{
    sal_uLong nOldIndex = m_pNode->GetIndex();
    m_pNode = GetNodes()[ nOldIndex + 1 ];
    return nOldIndex;
}
inline sal_uLong SwNodeIndex::operator--(int)
{
    sal_uLong nOldIndex = m_pNode->GetIndex();
    m_pNode = GetNodes()[ nOldIndex - 1 ];
    return nOldIndex;
}

inline sal_uLong SwNodeIndex::operator+=( sal_uLong const nOffset )
{
    m_pNode = GetNodes()[ m_pNode->GetIndex() + nOffset ];
    return m_pNode->GetIndex();
}
inline sal_uLong SwNodeIndex::operator-=( sal_uLong const nOffset )
{
    m_pNode = GetNodes()[ m_pNode->GetIndex() - nOffset ];
    return m_pNode->GetIndex();
}

inline SwNodeIndex& SwNodeIndex::operator=( sal_uLong const nNew )
{
    m_pNode = GetNodes()[ nNew ];
    return *this;
}

SwNodeIndex& SwNodeIndex::operator=( const SwNodeIndex& rIdx )
{
    *this = *(rIdx.m_pNode);
    return *this;
}

SwNodeIndex& SwNodeIndex::operator=( const SwNode& rNd )
{
    if (&m_pNode->GetNodes() != &rNd.GetNodes())
    {
        DeRegisterIndex( m_pNode->GetNodes() );
        m_pNode = const_cast<SwNode*>(&rNd);
        RegisterIndex( m_pNode->GetNodes() );
    }
    else
        m_pNode = const_cast<SwNode*>(&rNd);
    return *this;
}

SwNodeIndex& SwNodeIndex::Assign( SwNodes const & rNds, sal_uLong nIdx )
{
    *this = *rNds[ nIdx ];
    return *this;
}

SwNodeIndex& SwNodeIndex::Assign( const SwNode& rNd, long nOffset )
{
    *this = rNd;

    if( nOffset )
        m_pNode = m_pNode->GetNodes()[ m_pNode->GetIndex() + nOffset ];

    return *this;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
