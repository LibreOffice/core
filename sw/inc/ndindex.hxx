/*************************************************************************
 *
 *  $RCSfile: ndindex.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _NDINDEX_HXX
#define _NDINDEX_HXX

#include <limits.h>
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#include "node.hxx"
#include "ndarr.hxx"

class SwNode;
class SwNodes;

class SwNodeIndex
{
    friend void SwNodes::RegisterIndex( SwNodeIndex& );
    friend void SwNodes::DeRegisterIndex( SwNodeIndex& );
    friend void SwNodes::RemoveNode( ULONG, ULONG, FASTBOOL );

#ifndef PRODUCT
    static int nSerial;
    int MySerial;
#endif

    SwNode* pNd;
    SwNodeIndex *pNext, *pPrev;

    void Remove();                  // Ausketten

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
    SwNodeIndex& Assign( SwNodes& rNds, ULONG );
    SwNodeIndex& Assign( const SwNode& rNd, long nOffset = 0 );

        // Herausgabe des Pointers auf das NodesArray,
    inline const SwNodes& GetNodes() const;
    inline       SwNodes& GetNodes();

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

    SwNodeRange( SwNodes& rArr, ULONG nSttIdx = 0, ULONG nEndIdx = 0 );
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
//              neue Schnittstelle angepasst werden
inline SwNode* SwNodes::operator[]( const SwNodeIndex& rIdx ) const
{
    return &rIdx.GetNode();
}


/******************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/inc/ndindex.hxx,v 1.1.1.1 2000-09-18 17:14:27 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.3  2000/09/18 16:03:23  willem.vandorp
    OpenOffice header added.

    Revision 1.2  2000/09/08 13:24:18  willem.vandorp
    Header and footer replaced

    Revision 1.1  1997/10/09 13:12:34  JP
    Umstellung NodeIndex/-Array/BigPtrArray


      Rev 1.0   09 Oct 1997 15:12:34   JP
   Umstellung NodeIndex/-Array/BigPtrArray

*******************************************************************/

#endif
