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
#ifndef _NDINDEX_HXX
#define _NDINDEX_HXX

#include <limits.h>
#include <tools/solar.h>

#include "node.hxx"
#include "ndarr.hxx"
namespace binfilter {

class SwNode;
class SwNodes;

class SwNodeIndex
{
    friend void SwNodes::RegisterIndex( SwNodeIndex& );
    friend void SwNodes::DeRegisterIndex( SwNodeIndex& );
    friend void SwNodes::RemoveNode( ULONG, ULONG, FASTBOOL );

#ifdef DBG_UTIL
    static int nSerial;
    int MySerial;
#endif

    SwNode* pNd;
    SwNodeIndex *pNext, *pPrev;

    void Remove();					// Ausketten

    // diese sind nicht erlaubt!
    SwNodeIndex( SwNodes& rNds, USHORT nIdx );
    SwNodeIndex( SwNodes& rNds, int nIdx );

public:
    SwNodeIndex( SwNodes& rNds, ULONG nIdx = 0 );
    SwNodeIndex( const SwNodeIndex &, long nDiff = 0 );
    SwNodeIndex( const SwNode&, long nDiff = 0 );
    ~SwNodeIndex() { Remove(); }

    inline ULONG operator++();
    inline ULONG operator--();
#ifndef CFRONT
    inline ULONG operator++(int);
    inline ULONG operator--(int);
#endif

    inline ULONG operator+=( ULONG );
    inline ULONG operator-=( ULONG );
    inline ULONG operator+=( const  SwNodeIndex& );
    inline ULONG operator-=( const SwNodeIndex& );

    inline BOOL operator< ( const SwNodeIndex& ) const;
    inline BOOL operator<=( const SwNodeIndex& ) const;
    inline BOOL operator> ( const SwNodeIndex& ) const;
    inline BOOL operator>=( const SwNodeIndex& ) const;
    inline BOOL operator==( const SwNodeIndex& ) const;
    inline BOOL operator!=( const SwNodeIndex& ) const;

    inline BOOL operator< ( ULONG nWert ) const;
    inline BOOL operator<=( ULONG nWert ) const;
    inline BOOL operator> ( ULONG nWert ) const;
    inline BOOL operator>=( ULONG nWert ) const;
    inline BOOL operator==( ULONG nWert ) const;
    inline BOOL operator!=( ULONG nWert ) const;

    inline SwNodeIndex& operator=( ULONG );
           SwNodeIndex& operator=( const SwNodeIndex& );
           SwNodeIndex& operator=( const SwNode& );

    // gebe den Wert vom Index als ULONG zurueck
    inline ULONG GetIndex() const;

    // ermoeglicht Zuweisungen ohne Erzeugen eines temporaeren Objektes
    SwNodeIndex& Assign( const SwNode& rNd, long nOffset = 0 );

        // Herausgabe des Pointers auf das NodesArray,
    inline const SwNodes& GetNodes() const;
    inline	  	 SwNodes& GetNodes();

    SwNode& GetNode() const { return *pNd; }
};

/*
 * SwRange
 */
class SwNodeRange
{
public:
    SwNodeIndex aStart;
    SwNodeIndex aEnd;

    SwNodeRange( const SwNodeIndex &rS, const SwNodeIndex &rE );
    SwNodeRange( const SwNodeRange &rRange );

    SwNodeRange( const SwNodeIndex& rS, long nSttDiff,
                 const SwNodeIndex& rE, long nEndDiff = 0 );
    SwNodeRange( const SwNode& rS, long nSttDiff,
                 const SwNode& rE, long nEndDiff = 0 );
};




// fuer die inlines wird aber der node.hxx benoetigt. Dieses braucht aber
// auch wieder dieses. Also alle Inlines, die auf pNd zugreifen werden
// hier implementiert.

inline ULONG SwNodeIndex::GetIndex() const
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
inline BOOL SwNodeIndex::operator< ( ULONG nWert ) const
{
    return pNd->GetIndex() < nWert;
}
inline BOOL SwNodeIndex::operator<=( ULONG nWert ) const
{
    return pNd->GetIndex() <= nWert;
}
inline BOOL SwNodeIndex::operator> ( ULONG nWert ) const
{
    return pNd->GetIndex() > nWert;
}
inline BOOL SwNodeIndex::operator>=( ULONG nWert ) const
{
    return pNd->GetIndex() >= nWert;
}
inline BOOL SwNodeIndex::operator==( ULONG nWert ) const
{
    return pNd->GetIndex() == nWert;
}
inline BOOL SwNodeIndex::operator!=( ULONG nWert ) const
{
    return pNd->GetIndex() != nWert;
}
inline BOOL SwNodeIndex::operator<( const SwNodeIndex& rIndex ) const
{
    return pNd->GetIndex() < rIndex.GetIndex();
}
inline BOOL SwNodeIndex::operator<=( const SwNodeIndex& rIndex ) const
{
    return pNd->GetIndex() <= rIndex.GetIndex();
}
inline BOOL SwNodeIndex::operator>( const SwNodeIndex& rIndex ) const
{
    return pNd->GetIndex() > rIndex.GetIndex();
}
inline BOOL SwNodeIndex::operator>=( const SwNodeIndex& rIndex ) const
{
    return pNd->GetIndex() >= rIndex.GetIndex();
}
inline BOOL SwNodeIndex::operator==( const SwNodeIndex& rIdx ) const
{
    return pNd == rIdx.pNd;
}
inline BOOL SwNodeIndex::operator!=( const SwNodeIndex& rIdx ) const
{
    return pNd != rIdx.pNd;
}

inline ULONG SwNodeIndex::operator++()
{
    return ( pNd = GetNodes()[ pNd->GetIndex()+1 ] )->GetIndex();
}
inline ULONG SwNodeIndex::operator--()
{
    return ( pNd = GetNodes()[ pNd->GetIndex()-1 ] )->GetIndex();
}
#ifndef CFRONT
inline ULONG SwNodeIndex::operator++(int)
{
    ULONG nOldIndex = pNd->GetIndex();
    pNd = GetNodes()[ nOldIndex + 1 ];
    return nOldIndex;
}
inline ULONG SwNodeIndex::operator--(int)
{
    ULONG nOldIndex = pNd->GetIndex();
    pNd = GetNodes()[ nOldIndex - 1 ];
    return nOldIndex;
}
#endif

inline ULONG SwNodeIndex::operator+=( ULONG nWert )
{
    return ( pNd = GetNodes()[ pNd->GetIndex() + nWert ] )->GetIndex();
}
inline ULONG SwNodeIndex::operator-=( ULONG nWert )
{
    return ( pNd = GetNodes()[ pNd->GetIndex() - nWert ] )->GetIndex();
}
inline ULONG SwNodeIndex::operator+=( const  SwNodeIndex& rIndex )
{
    return ( pNd = GetNodes()[ pNd->GetIndex() + rIndex.GetIndex() ] )->GetIndex();
}
inline ULONG SwNodeIndex::operator-=( const SwNodeIndex& rIndex )
{
    return ( pNd = GetNodes()[ pNd->GetIndex() - rIndex.GetIndex() ] )->GetIndex();
}

inline SwNodeIndex& SwNodeIndex::operator=( ULONG nWert )
{
    pNd = GetNodes()[ nWert ];
    return *this;
}



//JP 29.09.97: impl. steht im ndindex.hxx - sollte moeglichst bald auf die
//				neue Schnittstelle angepasst werden
inline SwNode* SwNodes::operator[]( const SwNodeIndex& rIdx ) const
{
    return &rIdx.GetNode();
}


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
