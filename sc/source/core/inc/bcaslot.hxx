/*************************************************************************
 *
 *  $RCSfile: bcaslot.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:30:46 $
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

#ifndef _SC_BCASLOT_HXX
#define _SC_BCASLOT_HXX

#include <set>
#include <functional>

#ifndef _SVT_BROADCAST_HXX
#include <svtools/broadcast.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#include "global.hxx"
#include "brdcst.hxx"

/// Used in a Unique Sorted Associative Container
class ScBroadcastArea
{
private:
    ScBroadcastArea*    pUpdateChainNext;
    SvtBroadcaster      aBroadcaster;
    ScRange             aRange;
    ULONG               nRefCount;
    BOOL                bInUpdateChain;

public:
            ScBroadcastArea( const ScRange& rRange )
                : pUpdateChainNext( NULL ), aRange( rRange ),
                nRefCount( 0 ), bInUpdateChain( FALSE ) {}
    inline SvtBroadcaster&       GetBroadcaster()       { return aBroadcaster; }
    inline const SvtBroadcaster& GetBroadcaster() const { return aBroadcaster; }
    inline void         UpdateRange( const ScRange& rNewRange )
                            { aRange = rNewRange; }
    inline const ScRange&   GetRange() const { return aRange; }
    inline const ScAddress& GetStart() const { return aRange.aStart; }
    inline const ScAddress& GetEnd() const { return aRange.aEnd; }
    inline void         IncRef() { ++nRefCount; }
    inline ULONG        DecRef() { return --nRefCount; }
    inline ScBroadcastArea* GetUpdateChainNext() const { return pUpdateChainNext; }
    inline void         SetUpdateChainNext( ScBroadcastArea* p ) { pUpdateChainNext = p; }
    inline BOOL         IsInUpdateChain() const { return bInUpdateChain; }
    inline void         SetInUpdateChain( BOOL b ) { bInUpdateChain = b; }

    /** Strict weak sorting order, upper left corner and then lower right */
    inline  bool        operator<( const ScBroadcastArea& rArea ) const;
};

inline bool ScBroadcastArea::operator<( const ScBroadcastArea& rArea ) const
{
    return aRange < rArea.aRange;
}


struct ScBroadcastAreaSort
{
    bool operator()( const ScBroadcastArea* p1, const ScBroadcastArea* p2) const
    {
        return *p1 < *p2;
    }
};

typedef ::std::set< ScBroadcastArea*, ScBroadcastAreaSort > ScBroadcastAreas;


class ScBroadcastAreaSlotMachine;

/// Collection of BroadcastAreas
class ScBroadcastAreaSlot
{
private:
    ScBroadcastAreas    aBroadcastAreaTbl;
    mutable ScBroadcastArea aTmpSeekBroadcastArea;      // for FindBroadcastArea()
    ScDocument*         pDoc;
    ScBroadcastAreaSlotMachine* pBASM;

    ScBroadcastAreas::iterator  FindBroadcastArea( const ScRange& rRange ) const;

public:
                        ScBroadcastAreaSlot( ScDocument* pDoc,
                                        ScBroadcastAreaSlotMachine* pBASM );
                        ~ScBroadcastAreaSlot();
    const ScBroadcastAreas& GetBroadcastAreas() const
                                            { return aBroadcastAreaTbl; }
    void                StartListeningArea( const ScRange& rRange,
                                            SvtListener* pListener,
                                            ScBroadcastArea*& rpArea );
    void                EndListeningArea( const ScRange& rRange,
                                            SvtListener* pListener,
                                            ScBroadcastArea*& rpArea );
    BOOL                AreaBroadcast( const ScHint& rHint ) const;
        // return: mindestens ein Broadcast gewesen
    BOOL                AreaBroadcastInRange( const ScRange& rRange,
                                              const ScHint& rHint ) const;
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateRemove( UpdateRefMode eUpdateRefMode,
                                        const ScRange& rRange,
                                        SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void                UpdateInsert( ScBroadcastArea* pArea );
};


/*
    BroadcastAreaSlots und deren Verwaltung, einmal je Dokument

    +---+---+
    | 0 | 2 |   Anordnung Cols/Rows
    +---+---+
    | 1 | 3 |
    +---+---+
 */

class  ScBroadcastAreaSlotMachine
{
private:
    ScBroadcastAreaSlot**   ppSlots;
    SvtBroadcaster*     pBCAlways;      // for the RC_ALWAYS special range
    ScDocument*         pDoc;
    ScBroadcastArea*    pUpdateChain;
    ScBroadcastArea*    pEOUpdateChain;

    inline SCSIZE       ComputeSlotOffset( const ScAddress& rAddress ) const;
    void                ComputeAreaPoints( const ScRange& rRange,
                                            SCSIZE& nStart, SCSIZE& nEnd,
                                            SCSIZE& nRowBreak ) const;

public:
                        ScBroadcastAreaSlotMachine( ScDocument* pDoc );
                        ~ScBroadcastAreaSlotMachine();
    void                StartListeningArea( const ScRange& rRange,
                                            SvtListener* pListener );
    void                EndListeningArea( const ScRange& rRange,
                                            SvtListener* pListener );
    BOOL                AreaBroadcast( const ScHint& rHint ) const;
        // return: mindestens ein Broadcast gewesen
    BOOL                AreaBroadcastInRange( const ScRange& rRange, const ScHint& rHint ) const;
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateBroadcastAreas( UpdateRefMode eUpdateRefMode,
                                            const ScRange& rRange,
                                            SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    inline ScBroadcastArea* GetUpdateChain() const { return pUpdateChain; }
    inline void SetUpdateChain( ScBroadcastArea* p ) { pUpdateChain = p; }
    inline ScBroadcastArea* GetEOUpdateChain() const { return pEOUpdateChain; }
    inline void SetEOUpdateChain( ScBroadcastArea* p ) { pEOUpdateChain = p; }
};



#endif
