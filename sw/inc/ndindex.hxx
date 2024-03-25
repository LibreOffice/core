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

#include "node.hxx"
#include "ring.hxx"
#include "ndarr.hxx"
#include "nodeoffset.hxx"

/// Marks a node in the document model.
class SAL_WARN_UNUSED SW_DLLPUBLIC SwNodeIndex final : public sw::Ring<SwNodeIndex>
{
    SwNode * m_pNode;

    void RegisterIndex()
    {
        SwNodes& rNodes = GetNodes();
        if(!rNodes.m_vIndices)
        {
#if defined(__GNUC__) && (__GNUC__ == 12 || __GNUC__ == 13)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"
#endif
            rNodes.m_vIndices = this;
#if defined(__GNUC__) && (__GNUC__ == 12 || __GNUC__ == 13)
#pragma GCC diagnostic pop
#endif
        }
        MoveTo(rNodes.m_vIndices);
    }
    void DeRegisterIndex()
    {
        SwNodes& rNodes = GetNodes();
        if(rNodes.m_vIndices == this)
            rNodes.m_vIndices = GetNextInRing();
        MoveTo(nullptr);
        if(rNodes.m_vIndices == this)
            rNodes.m_vIndices = nullptr;
    }

    SwNodeIndex(SwNode* pNode) : m_pNode(pNode) { RegisterIndex(); }

public:
    SwNodeIndex( SwNodes& rNds, sal_Int32 nIdx ) : SwNodeIndex(rNds, SwNodeOffset(nIdx)) {}
    explicit SwNodeIndex( SwNodes& rNds, SwNodeOffset nIdx = SwNodeOffset(0) )
        : SwNodeIndex( rNds[ nIdx ] ) {}

    SwNodeIndex( const SwNodeIndex& rIdx, sal_Int32 nDiff ) : SwNodeIndex(rIdx, SwNodeOffset(nDiff)) {}
    SwNodeIndex( const SwNodeIndex& rIdx, SwNodeOffset nDiff = SwNodeOffset(0) )
        : SwNodeIndex( nDiff ? rIdx.GetNodes()[ rIdx.GetIndex() + nDiff ] : rIdx.m_pNode ) {}

    SwNodeIndex( const SwNode& rNd, sal_Int32 nDiff ) : SwNodeIndex(rNd, SwNodeOffset(nDiff)) {}
    explicit SwNodeIndex( const SwNode& rNd )
        : SwNodeIndex( const_cast<SwNode*>(&rNd) ) {}
    explicit SwNodeIndex( const SwNode& rNd, SwNodeOffset nDiff )
        : SwNodeIndex( nDiff ? *rNd.GetNodes()[ rNd.GetIndex() + nDiff ] : rNd ) {}

    virtual ~SwNodeIndex() override { DeRegisterIndex(); }

    SwNodeIndex& operator++() { return operator+=(SwNodeOffset(1)); }
    SwNodeIndex& operator--() { return operator-=(SwNodeOffset(1)); }

    SwNodeIndex& operator+=( SwNodeOffset nOffset ) { return operator=(GetIndex() + nOffset); }
    SwNodeIndex& operator-=( SwNodeOffset nOffset ) { return operator=(GetIndex() - nOffset); }

    bool operator<( const SwNodeIndex& rIndex ) const { return operator<(rIndex.GetNode()); }
    bool operator<=( const SwNodeIndex& rIndex ) const { return operator<=(rIndex.GetNode()); }
    bool operator>( const SwNodeIndex& rIndex ) const { return operator>(rIndex.GetNode()); }
    bool operator>=( const SwNodeIndex& rIndex ) const { return operator>=(rIndex.GetNode()); }
    bool operator==( const SwNodeIndex& rIndex ) const { return operator==(rIndex.GetNode()); }
    bool operator!=( const SwNodeIndex& rIndex ) const { return operator!=(rIndex.GetNode()); }

    bool operator<( SwNodeOffset nOther ) const { return GetIndex() < nOther; }
    bool operator<=( SwNodeOffset nOther ) const { return GetIndex() <= nOther; }
    bool operator>( SwNodeOffset nOther ) const { return GetIndex() > nOther; }
    bool operator>=( SwNodeOffset nOther ) const { return GetIndex() >= nOther; }
    bool operator==( SwNodeOffset nOther ) const { return GetIndex() == nOther; }
    bool operator!=( SwNodeOffset nOther ) const { return GetIndex() != nOther; }

    bool operator<( const SwNode& rNd ) const { assert(&GetNodes() == &rNd.GetNodes()); return operator<(rNd.GetIndex()); }
    bool operator<=( const SwNode& rNd ) const { return operator==(rNd) || operator<(rNd); }
    bool operator>( const SwNode& rNd ) const { assert(&GetNodes() == &rNd.GetNodes()); return operator>(rNd.GetIndex()); }
    bool operator>=( const SwNode& rNd ) const { return operator==(rNd) || operator>(rNd); }
    bool operator==( const SwNode& rNd ) const { return m_pNode == &rNd; }
    bool operator!=( const SwNode& rNd ) const { return m_pNode != &rNd; }

    inline SwNodeIndex& operator=( SwNodeOffset );
    SwNodeIndex& operator=( const SwNodeIndex& rIdx ) { return operator=(*rIdx.m_pNode); }
    inline SwNodeIndex& operator=( const SwNode& );

    // Return value of index as SwNodeOffset.
    SwNodeOffset GetIndex() const { return m_pNode->GetIndex(); }

    // Enables assignments without creation of a temporary object.
    SwNodeIndex& Assign( SwNodes const & rNds, SwNodeOffset nIdx ) { return operator=(*rNds[nIdx]); }
    SwNodeIndex& Assign( const SwNode& rNd, sal_Int32 nOffset ) { return Assign(rNd, SwNodeOffset(nOffset)); }
    inline SwNodeIndex& Assign( const SwNode& rNd, SwNodeOffset nOffset = SwNodeOffset(0) );

    // Gets pointer on NodesArray.
    const SwNodes& GetNodes() const { return m_pNode->GetNodes(); }
          SwNodes& GetNodes() { return m_pNode->GetNodes(); }

    SwNode& GetNode() const { return *m_pNode; }
};

inline std::ostream &operator <<(std::ostream& s, const SwNodeIndex& index)
{
    return s << "SwNodeIndex (node " << sal_Int32(index.GetIndex()) << ")";
}

// SwRange

class SW_DLLPUBLIC SwNodeRange
{
public:
    SwNodeIndex aStart;
    SwNodeIndex aEnd;

    SwNodeRange( const SwNodeIndex &rS, const SwNodeIndex &rE )
        : aStart( rS ), aEnd( rE ) {}
    SwNodeRange( const SwNode &rS, const SwNode &rE )
        : aStart( rS ), aEnd( rE ) {}
    SwNodeRange( const SwNodeRange &rRange ) = default;

    SwNodeRange( SwNodes& rNds, SwNodeOffset nSttIdx, SwNodeOffset nEndIdx = SwNodeOffset(0) )
        : aStart( rNds, nSttIdx ), aEnd( rNds, nEndIdx ) {}

    SwNodeRange( const SwNodeIndex& rS, SwNodeOffset nSttDiff, const SwNodeIndex& rE, SwNodeOffset nEndDiff = SwNodeOffset(0) )
        : aStart( rS, nSttDiff ), aEnd( rE, nEndDiff ) {}
    SwNodeRange( const SwNode& rS, SwNodeOffset nSttDiff, const SwNode& rE, SwNodeOffset nEndDiff = SwNodeOffset(0) )
        : aStart( rS, nSttDiff ), aEnd( rE, nEndDiff ) {}
};

// For inlines node.hxx is needed which in turn needs this one.
// Therefore all inlines accessing m_pNode are implemented here.

inline SwNodeIndex& SwNodeIndex::operator=( SwNodeOffset const nNew )
{
    m_pNode = GetNodes()[ nNew ];
    return *this;
}

SwNodeIndex& SwNodeIndex::operator=( const SwNode& rNd )
{
    if (&GetNodes() != &rNd.GetNodes())
    {
        DeRegisterIndex();
        m_pNode = const_cast<SwNode*>(&rNd);
        RegisterIndex();
    }
    else
        m_pNode = const_cast<SwNode*>(&rNd);
    return *this;
}

SwNodeIndex& SwNodeIndex::Assign( const SwNode& rNd, SwNodeOffset nOffset )
{
    *this = rNd;

    if( nOffset )
        m_pNode = GetNodes()[ GetIndex() + nOffset ];

    return *this;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
