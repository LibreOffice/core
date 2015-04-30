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
class SW_DLLPUBLIC SwNodeIndex SAL_FINAL : public sw::Ring<SwNodeIndex>
{
    SwNode* pNd;

    // These are not allowed!
    SwNodeIndex( SwNodes& rNds, sal_uInt16 nIdx ) SAL_DELETED_FUNCTION;
    SwNodeIndex( SwNodes& rNds, int nIdx ) SAL_DELETED_FUNCTION;
    void RegisterIndex( SwNodes& rNodes )
    {
        if(!rNodes.vIndices)
            rNodes.vIndices = this;
        MoveTo(rNodes.vIndices);
    }
    void DeRegisterIndex( SwNodes& rNodes )
    {
        if(rNodes.vIndices == this)
            rNodes.vIndices = GetNextInRing();
        MoveTo(nullptr);
        if(rNodes.vIndices == this)
            rNodes.vIndices = nullptr;
    }

public:
    SwNodeIndex( SwNodes& rNds, sal_uLong nIdx = 0 )
        : pNd( rNds[ nIdx ] )
    {
        RegisterIndex( rNds );
    };
    SwNodeIndex( const SwNodeIndex& rIdx, long nDiff = 0 )
        : sw::Ring<SwNodeIndex>()
    {
        if( nDiff )
            pNd = rIdx.GetNodes()[ rIdx.GetIndex() + nDiff ];
        else
            pNd = rIdx.pNd;
        RegisterIndex( pNd->GetNodes() );
    }

    SwNodeIndex( const SwNode& rNd, long nDiff = 0 )
    {
        if( nDiff )
            pNd = rNd.GetNodes()[ rNd.GetIndex() + nDiff ];
        else
            pNd = const_cast<SwNode*>(&rNd);
        RegisterIndex( pNd->GetNodes() );
    }

   virtual  ~SwNodeIndex()
        { DeRegisterIndex( pNd->GetNodes() ); };

    inline sal_uLong operator++();
    inline sal_uLong operator--();
    inline sal_uLong operator++(int);
    inline sal_uLong operator--(int);

    inline sal_uLong operator+=( sal_uLong );
    inline sal_uLong operator-=( sal_uLong );
    inline sal_uLong operator+=( const  SwNodeIndex& );
    inline sal_uLong operator-=( const SwNodeIndex& );

    inline bool operator< ( const SwNodeIndex& ) const;
    inline bool operator<=( const SwNodeIndex& ) const;
    inline bool operator> ( const SwNodeIndex& ) const;
    inline bool operator>=( const SwNodeIndex& ) const;
    inline bool operator==( const SwNodeIndex& ) const;
    inline bool operator!=( const SwNodeIndex& ) const;

    inline bool operator< ( sal_uLong nWert ) const;
    inline bool operator<=( sal_uLong nWert ) const;
    inline bool operator> ( sal_uLong nWert ) const;
    inline bool operator>=( sal_uLong nWert ) const;
    inline bool operator==( sal_uLong nWert ) const;
    inline bool operator!=( sal_uLong nWert ) const;

    inline SwNodeIndex& operator=( sal_uLong );
    inline SwNodeIndex& operator=( const SwNodeIndex& );
    inline SwNodeIndex& operator=( const SwNode& );

    // Return value of index as sal_uLong.
    inline sal_uLong GetIndex() const;

    // Enables assignments without creation of a temporary object.
    inline SwNodeIndex& Assign( SwNodes& rNds, sal_uLong );
    inline SwNodeIndex& Assign( const SwNode& rNd, long nOffset = 0 );

    // Gets pointer on NodesArray.
    inline const SwNodes& GetNodes() const;
    inline       SwNodes& GetNodes();

    SwNode& GetNode() const { return *pNd; }
};

inline std::ostream &operator <<(std::ostream& s, const SwNodeIndex& index)
{
    return s << "SwNodeIndex (node " << index.GetIndex() << ")";
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

    SwNodeRange( SwNodes& rNds, sal_uLong nSttIdx = 0, sal_uLong nEndIdx = 0 )
        : aStart( rNds, nSttIdx ), aEnd( rNds, nEndIdx ) {};

    SwNodeRange( const SwNodeIndex& rS, long nSttDiff, const SwNodeIndex& rE, long nEndDiff = 0 )
        : aStart( rS, nSttDiff ), aEnd( rE, nEndDiff ) {};
    SwNodeRange( const SwNode& rS, long nSttDiff, const SwNode& rE, long nEndDiff = 0 )
        : aStart( rS, nSttDiff ), aEnd( rE, nEndDiff ) {};
};

// For inlines node.hxx is needed which in turn needs this one.
// Therefore all inlines accessing pNd are implemented here.

inline sal_uLong SwNodeIndex::GetIndex() const
{
    return pNd->GetIndex();
}
inline const SwNodes& SwNodeIndex::GetNodes() const
{
    return pNd->GetNodes();
}
inline SwNodes& SwNodeIndex::GetNodes()
{
    return pNd->GetNodes();
}
inline bool SwNodeIndex::operator< ( sal_uLong nWert ) const
{
    return pNd->GetIndex() < nWert;
}
inline bool SwNodeIndex::operator<=( sal_uLong nWert ) const
{
    return pNd->GetIndex() <= nWert;
}
inline bool SwNodeIndex::operator> ( sal_uLong nWert ) const
{
    return pNd->GetIndex() > nWert;
}
inline bool SwNodeIndex::operator>=( sal_uLong nWert ) const
{
    return pNd->GetIndex() >= nWert;
}
inline bool SwNodeIndex::operator==( sal_uLong nWert ) const
{
    return pNd->GetIndex() == nWert;
}
inline bool SwNodeIndex::operator!=( sal_uLong nWert ) const
{
    return pNd->GetIndex() != nWert;
}
inline bool SwNodeIndex::operator<( const SwNodeIndex& rIndex ) const
{
    return pNd->GetIndex() < rIndex.GetIndex();
}
inline bool SwNodeIndex::operator<=( const SwNodeIndex& rIndex ) const
{
    return pNd->GetIndex() <= rIndex.GetIndex();
}
inline bool SwNodeIndex::operator>( const SwNodeIndex& rIndex ) const
{
    return pNd->GetIndex() > rIndex.GetIndex();
}
inline bool SwNodeIndex::operator>=( const SwNodeIndex& rIndex ) const
{
    return pNd->GetIndex() >= rIndex.GetIndex();
}
inline bool SwNodeIndex::operator==( const SwNodeIndex& rIdx ) const
{
    return pNd == rIdx.pNd;
}
inline bool SwNodeIndex::operator!=( const SwNodeIndex& rIdx ) const
{
    return pNd != rIdx.pNd;
}

inline sal_uLong SwNodeIndex::operator++()
{
    return ( pNd = GetNodes()[ pNd->GetIndex()+1 ] )->GetIndex();
}
inline sal_uLong SwNodeIndex::operator--()
{
    return ( pNd = GetNodes()[ pNd->GetIndex()-1 ] )->GetIndex();
}
inline sal_uLong SwNodeIndex::operator++(int)
{
    sal_uLong nOldIndex = pNd->GetIndex();
    pNd = GetNodes()[ nOldIndex + 1 ];
    return nOldIndex;
}
inline sal_uLong SwNodeIndex::operator--(int)
{
    sal_uLong nOldIndex = pNd->GetIndex();
    pNd = GetNodes()[ nOldIndex - 1 ];
    return nOldIndex;
}

inline sal_uLong SwNodeIndex::operator+=( sal_uLong nWert )
{
    return ( pNd = GetNodes()[ pNd->GetIndex() + nWert ] )->GetIndex();
}
inline sal_uLong SwNodeIndex::operator-=( sal_uLong nWert )
{
    return ( pNd = GetNodes()[ pNd->GetIndex() - nWert ] )->GetIndex();
}
inline sal_uLong SwNodeIndex::operator+=( const  SwNodeIndex& rIndex )
{
    return ( pNd = GetNodes()[ pNd->GetIndex() + rIndex.GetIndex() ] )->GetIndex();
}
inline sal_uLong SwNodeIndex::operator-=( const SwNodeIndex& rIndex )
{
    return ( pNd = GetNodes()[ pNd->GetIndex() - rIndex.GetIndex() ] )->GetIndex();
}

inline SwNodeIndex& SwNodeIndex::operator=( sal_uLong nWert )
{
    pNd = GetNodes()[ nWert ];
    return *this;
}

SwNodeIndex& SwNodeIndex::operator=( const SwNodeIndex& rIdx )
{
    *this = *(rIdx.pNd);
    return *this;
}

SwNodeIndex& SwNodeIndex::operator=( const SwNode& rNd )
{
    if( &pNd->GetNodes() != &rNd.GetNodes() )
    {
        DeRegisterIndex( pNd->GetNodes() );
        pNd = const_cast<SwNode*>(&rNd);
        RegisterIndex( pNd->GetNodes() );
    }
    else
        pNd = const_cast<SwNode*>(&rNd);
    return *this;
}

SwNodeIndex& SwNodeIndex::Assign( SwNodes& rNds, sal_uLong nIdx )
{
    *this = *rNds[ nIdx ];
    return *this;
}

SwNodeIndex& SwNodeIndex::Assign( const SwNode& rNd, long nOffset )
{
    *this = rNd;

    if( nOffset )
        pNd = pNd->GetNodes()[ pNd->GetIndex() + nOffset ];

    return *this;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
