/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bcaslot.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _SC_BCASLOT_HXX
#define _SC_BCASLOT_HXX

#include <set>
#include <hash_set>
#include <functional>
#include <svtools/broadcast.hxx>
#include <svtools/svarray.hxx>

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

//=============================================================================

struct ScBroadcastAreaSort
{
    bool operator()( const ScBroadcastArea* p1, const ScBroadcastArea* p2) const
    {
        return *p1 < *p2;
    }
};

typedef ::std::set< ScBroadcastArea*, ScBroadcastAreaSort > ScBroadcastAreas;

//=============================================================================

struct ScBroadcastAreaBulkHash
{
    size_t operator()( const ScBroadcastArea* p ) const
    {
        return reinterpret_cast<size_t>(p);
    }
};

struct ScBroadcastAreaBulkEqual
{
    bool operator()( const ScBroadcastArea* p1, const ScBroadcastArea* p2) const
    {
        return p1 == p2;
    }
};

typedef ::std::hash_set< const ScBroadcastArea*, ScBroadcastAreaBulkHash,
        ScBroadcastAreaBulkEqual > ScBroadcastAreasBulk;

//=============================================================================

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
    ScBroadcastAreasBulk    aBulkBroadcastAreas;
    ScBroadcastAreaSlot**   ppSlots;
    SvtBroadcaster*     pBCAlways;      // for the RC_ALWAYS special range
    ScDocument*         pDoc;
    ScBroadcastArea*    pUpdateChain;
    ScBroadcastArea*    pEOUpdateChain;
    ULONG               nInBulkBroadcast;

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
    void                EnterBulkBroadcast();
    void                LeaveBulkBroadcast();
    bool                InsertBulkArea( const ScBroadcastArea* p );
    /// @return: how many removed
    size_t              RemoveBulkArea( const ScBroadcastArea* p );
    inline ScBroadcastArea* GetUpdateChain() const { return pUpdateChain; }
    inline void SetUpdateChain( ScBroadcastArea* p ) { pUpdateChain = p; }
    inline ScBroadcastArea* GetEOUpdateChain() const { return pEOUpdateChain; }
    inline void SetEOUpdateChain( ScBroadcastArea* p ) { pEOUpdateChain = p; }
    inline bool IsInBulkBroadcast() const { return nInBulkBroadcast > 0; }
};


class ScBulkBroadcast
{
    ScBroadcastAreaSlotMachine* pBASM;
public:
    explicit ScBulkBroadcast( ScBroadcastAreaSlotMachine* p ) : pBASM(p)
    {
        if (pBASM)
            pBASM->EnterBulkBroadcast();
    }
    ~ScBulkBroadcast()
    {
        if (pBASM)
            pBASM->LeaveBulkBroadcast();
    }
    void LeaveBulkBroadcast()
    {
        if (pBASM)
        {
            pBASM->LeaveBulkBroadcast();
            pBASM = NULL;
        }
    }
};

#endif
