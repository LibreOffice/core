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

#include <iostream>

#include <tools/solar.h>

#include "node.hxx"
#include "ring.hxx"
#include "ndarr.hxx"
#include "nodeoffset.hxx"

/// Marks a node in the document model.
class SW_DLLPUBLIC SwNodeIndex final : public sw::Ring<SwNodeIndex>
{
    SwNode * m_pNode;

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
    // These are not allowed!
    SwNodeIndex( SwNodes& rNds, sal_Int32 nIdx ) : SwNodeIndex(rNds, SwNodeOffset(nIdx)) {}
    SwNodeIndex( SwNodes& rNds, SwNodeOffset nIdx = SwNodeOffset(0) )
        : m_pNode( rNds[ nIdx ] )
    {
        RegisterIndex( rNds );
    };
    SwNodeIndex( const SwNodeIndex& rIdx, sal_Int32 nDiff ) : SwNodeIndex(rIdx, SwNodeOffset(nDiff)) {}
    SwNodeIndex( const SwNodeIndex& rIdx, SwNodeOffset nDiff = SwNodeOffset(0) )
        : sw::Ring<SwNodeIndex>()
    {
        if( nDiff )
            m_pNode = rIdx.GetNodes()[ rIdx.GetIndex() + nDiff ];
        else
            m_pNode = rIdx.m_pNode;
        RegisterIndex( m_pNode->GetNodes() );
    }

    SwNodeIndex( const SwNode& rNd, sal_Int32 nDiff ) : SwNodeIndex(rNd, SwNodeOffset(nDiff)) {}
    SwNodeIndex( const SwNode& rNd, SwNodeOffset nDiff = SwNodeOffset(0) )
    {
        if( nDiff )
            m_pNode = rNd.GetNodes()[ rNd.GetIndex() + nDiff ];
        else
            m_pNode = const_cast<SwNode*>(&rNd);
        RegisterIndex( m_pNode->GetNodes() );
    }

    virtual  ~SwNodeIndex() override
        { DeRegisterIndex( m_pNode->GetNodes() ); }

    inline SwNodeOffset operator++();
    inline SwNodeOffset operator--();
    inline SwNodeOffset operator++(int);
    inline SwNodeOffset operator--(int);

    inline SwNodeOffset operator+=( SwNodeOffset );
    inline SwNodeOffset operator-=( SwNodeOffset );

    inline bool operator< ( const SwNodeIndex& ) const;
    inline bool operator<=( const SwNodeIndex& ) const;
    inline bool operator> ( const SwNodeIndex& ) const;
    inline bool operator>=( const SwNodeIndex& ) const;
    inline bool operator==( const SwNodeIndex& ) const;
    inline bool operator!=( const SwNodeIndex& ) const;

    inline bool operator< ( SwNodeOffset ) const;
    inline bool operator<=( SwNodeOffset ) const;
    inline bool operator> ( SwNodeOffset ) const;
    inline bool operator>=( SwNodeOffset ) const;
    inline bool operator==( SwNodeOffset ) const;
    inline bool operator!=( SwNodeOffset ) const;

    inline SwNodeIndex& operator=( SwNodeOffset );
    inline SwNodeIndex& operator=( const SwNodeIndex& );
    inline SwNodeIndex& operator=( const SwNode& );

    // Return value of index as SwNodeOffset.
    inline SwNodeOffset GetIndex() const;

    // Enables assignments without creation of a temporary object.
    inline SwNodeIndex& Assign( SwNodes const & rNds, SwNodeOffset );
    SwNodeIndex& Assign( const SwNode& rNd, sal_uInt16 ) = delete;
    SwNodeIndex& Assign( const SwNode& rNd, sal_uLong ) = delete;
    SwNodeIndex& Assign( const SwNode& rNd,  tools::Long  ) = delete;
    SwNodeIndex& Assign( const SwNode& rNd, sal_Int32 nOffset ) { return Assign(rNd, SwNodeOffset(nOffset)); }
    inline SwNodeIndex& Assign( const SwNode& rNd, SwNodeOffset nOffset = SwNodeOffset(0) );

    // Gets pointer on NodesArray.
    inline const SwNodes& GetNodes() const;
    inline       SwNodes& GetNodes();

    SwNodeIndex* GetNext() { return GetNextInRing(); }
    SwNode& GetNode() const { return *m_pNode; }
};

inline std::ostream &operator <<(std::ostream& s, const SwNodeIndex& index)
{
    return s << "SwNodeIndex (node " << sal_Int32(index.GetIndex()) << ")";
};

// SwRange

class SW_DLLPUBLIC SwNodeRange
{
public:
    SwNodeIndex aStart;
    SwNodeIndex aEnd;

    SwNodeRange( const SwNodeIndex &rS, const SwNodeIndex &rE )
        : aStart( rS ), aEnd( rE ) {};
    SwNodeRange( const SwNodeRange &rRange )
        : aStart( rRange.aStart ), aEnd( rRange.aEnd ) {};

    SwNodeRange( SwNodes& rNds, SwNodeOffset nSttIdx, SwNodeOffset nEndIdx = SwNodeOffset(0) )
        : aStart( rNds, nSttIdx ), aEnd( rNds, nEndIdx ) {};

    SwNodeRange( const SwNodeIndex& rS, SwNodeOffset nSttDiff, const SwNodeIndex& rE, SwNodeOffset nEndDiff = SwNodeOffset(0) )
        : aStart( rS, nSttDiff ), aEnd( rE, nEndDiff ) {};
    SwNodeRange( const SwNode& rS, SwNodeOffset nSttDiff, const SwNode& rE, SwNodeOffset nEndDiff = SwNodeOffset(0) )
        : aStart( rS, nSttDiff ), aEnd( rE, nEndDiff ) {};
};

// For inlines node.hxx is needed which in turn needs this one.
// Therefore all inlines accessing m_pNode are implemented here.

inline SwNodeOffset SwNodeIndex::GetIndex() const
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
inline bool SwNodeIndex::operator< ( SwNodeOffset const nOther ) const
{
    return m_pNode->GetIndex() < nOther;
}
inline bool SwNodeIndex::operator<=( SwNodeOffset const nOther ) const
{
    return m_pNode->GetIndex() <= nOther;
}
inline bool SwNodeIndex::operator> ( SwNodeOffset const nOther ) const
{
    return m_pNode->GetIndex() > nOther;
}
inline bool SwNodeIndex::operator>=( SwNodeOffset const nOther ) const
{
    return m_pNode->GetIndex() >= nOther;
}
inline bool SwNodeIndex::operator==( SwNodeOffset const nOther ) const
{
    return m_pNode->GetIndex() == nOther;
}
inline bool SwNodeIndex::operator!=( SwNodeOffset const nOther ) const
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

inline SwNodeOffset SwNodeIndex::operator++()
{
    m_pNode = GetNodes()[ m_pNode->GetIndex() + 1 ];
    return m_pNode->GetIndex();
}
inline SwNodeOffset SwNodeIndex::operator--()
{
    m_pNode = GetNodes()[ m_pNode->GetIndex() - 1 ];
    return m_pNode->GetIndex();
}
inline SwNodeOffset SwNodeIndex::operator++(int)
{
    SwNodeOffset nOldIndex = m_pNode->GetIndex();
    m_pNode = GetNodes()[ nOldIndex + 1 ];
    return nOldIndex;
}
inline SwNodeOffset SwNodeIndex::operator--(int)
{
    SwNodeOffset nOldIndex = m_pNode->GetIndex();
    m_pNode = GetNodes()[ nOldIndex - 1 ];
    return nOldIndex;
}

inline SwNodeOffset SwNodeIndex::operator+=( SwNodeOffset const nOffset )
{
    m_pNode = GetNodes()[ m_pNode->GetIndex() + nOffset ];
    return m_pNode->GetIndex();
}
inline SwNodeOffset SwNodeIndex::operator-=( SwNodeOffset const nOffset )
{
    m_pNode = GetNodes()[ m_pNode->GetIndex() - nOffset ];
    return m_pNode->GetIndex();
}

inline SwNodeIndex& SwNodeIndex::operator=( SwNodeOffset const nNew )
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

SwNodeIndex& SwNodeIndex::Assign( SwNodes const & rNds, SwNodeOffset nIdx )
{
    *this = *rNds[ nIdx ];
    return *this;
}

SwNodeIndex& SwNodeIndex::Assign( const SwNode& rNd, SwNodeOffset nOffset )
{
    *this = rNd;

    if( nOffset )
        m_pNode = m_pNode->GetNodes()[ m_pNode->GetIndex() + nOffset ];

    return *this;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
