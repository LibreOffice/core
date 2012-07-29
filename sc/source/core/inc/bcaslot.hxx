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

#ifndef SC_BCASLOT_HXX
#define SC_BCASLOT_HXX

#include <set>
#include <boost/unordered_set.hpp>
#include <functional>
#include <svl/broadcast.hxx>

#include "global.hxx"
#include "brdcst.hxx"

/**
    Used in a Unique Associative Container.
 */

class ScBroadcastArea
{
private:
    ScBroadcastArea*    pUpdateChainNext;
    SvtBroadcaster      aBroadcaster;
    ScRange             aRange;
    sal_uLong               nRefCount;
    sal_Bool                bInUpdateChain;

public:
            ScBroadcastArea( const ScRange& rRange )
                : pUpdateChainNext( NULL ), aRange( rRange ),
                nRefCount( 0 ), bInUpdateChain( false ) {}
    inline SvtBroadcaster&       GetBroadcaster()       { return aBroadcaster; }
    inline const SvtBroadcaster& GetBroadcaster() const { return aBroadcaster; }
    inline void         UpdateRange( const ScRange& rNewRange )
                            { aRange = rNewRange; }
    inline const ScRange&   GetRange() const { return aRange; }
    inline const ScAddress& GetStart() const { return aRange.aStart; }
    inline const ScAddress& GetEnd() const { return aRange.aEnd; }
    inline void         IncRef() { ++nRefCount; }
    inline sal_uLong        DecRef() { return nRefCount ? --nRefCount : 0; }
    inline sal_uLong        GetRef() { return nRefCount; }
    inline ScBroadcastArea* GetUpdateChainNext() const { return pUpdateChainNext; }
    inline void         SetUpdateChainNext( ScBroadcastArea* p ) { pUpdateChainNext = p; }
    inline sal_Bool         IsInUpdateChain() const { return bInUpdateChain; }
    inline void         SetInUpdateChain( sal_Bool b ) { bInUpdateChain = b; }

    /** Equalness of this or range. */
    inline  bool        operator==( const ScBroadcastArea & rArea ) const;
};

inline bool ScBroadcastArea::operator==( const ScBroadcastArea & rArea ) const
{
    return aRange == rArea.aRange;
}

//=============================================================================

struct ScBroadcastAreaHash
{
    size_t operator()( const ScBroadcastArea* p ) const
    {
        return p->GetRange().hashArea();
    }
};

struct ScBroadcastAreaEqual
{
    bool operator()( const ScBroadcastArea* p1, const ScBroadcastArea* p2) const
    {
        return *p1 == *p2;
    }
};

typedef ::boost::unordered_set< ScBroadcastArea*, ScBroadcastAreaHash, ScBroadcastAreaEqual > ScBroadcastAreas;

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

typedef ::boost::unordered_set< const ScBroadcastArea*, ScBroadcastAreaBulkHash,
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

    /**
        More hypothetical (memory would probably be doomed anyway) check
        whether there would be an overflow when adding an area, setting the
        proper state if so.

        @return sal_True if a HardRecalcState is effective and area is not to be
        added.
      */
    bool                CheckHardRecalcStateCondition() const;

public:
                        ScBroadcastAreaSlot( ScDocument* pDoc,
                                        ScBroadcastAreaSlotMachine* pBASM );
                        ~ScBroadcastAreaSlot();
    const ScBroadcastAreas& GetBroadcastAreas() const
                                            { return aBroadcastAreaTbl; }

    /**
        Only here new ScBroadcastArea objects are created, prevention of dupes.

        @param rpArea
            If NULL, a new ScBroadcastArea is created and assigned ton the
            reference if a matching area wasn't found. If a matching area was
            found, that is assigned. In any case, the SvtListener is added to
            the broadcaster.

            If not NULL then no listeners are startet, only the area is
            inserted and the reference count incremented. Effectively the same
            as InsertListeningArea(), so use that instead.

        @return
            sal_True if rpArea passed was NULL and ScBroadcastArea is newly
            created.
     */
    bool                StartListeningArea( const ScRange& rRange,
                                            SvtListener* pListener,
                                            ScBroadcastArea*& rpArea );

    /**
        Insert a ScBroadcastArea obtained via StartListeningArea() to
        subsequent slots.
     */
    void                InsertListeningArea( ScBroadcastArea* pArea );

    void                EndListeningArea( const ScRange& rRange,
                                            SvtListener* pListener,
                                            ScBroadcastArea*& rpArea );
    sal_Bool                AreaBroadcast( const ScHint& rHint ) const;
    /// @return sal_True if at least one broadcast occurred.
    sal_Bool                AreaBroadcastInRange( const ScRange& rRange,
                                              const ScHint& rHint ) const;
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateRemove( UpdateRefMode eUpdateRefMode,
                                        const ScRange& rRange,
                                        SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void                UpdateRemoveArea( ScBroadcastArea* pArea );
    void                UpdateInsert( ScBroadcastArea* pArea );
};


/**
    BroadcastAreaSlots and their management, once per document.
 */

class  ScBroadcastAreaSlotMachine
{
private:

    /**
        Slot offset arrangement of columns and rows, once per sheet.

        +---+---+
        | 0 | 3 |
        +---+---+
        | 1 | 4 |
        +---+---+
        | 2 | 5 |
        +---+---+
     */

    class TableSlots
    {
        public:
                                            TableSlots();
                                            ~TableSlots();
            inline ScBroadcastAreaSlot**    getSlots() { return ppSlots; }

            /**
                Obtain slot pointer, no check on validity! It is assumed that
                all calls are made with the results of ComputeSlotOffset(),
                ComputeAreaPoints() and ComputeNextSlot()
              */
            inline ScBroadcastAreaSlot*     getAreaSlot( SCSIZE nOff ) { return *(ppSlots + nOff); }

        private:
            ScBroadcastAreaSlot**   ppSlots;

            // prevent usage
            TableSlots( const TableSlots& );
            TableSlots& operator=( const TableSlots& );
    };

    typedef ::std::map< SCTAB, TableSlots* > TableSlotsMap;

private:
    ScBroadcastAreasBulk  aBulkBroadcastAreas;
    TableSlotsMap         aTableSlotsMap;
    SvtBroadcaster       *pBCAlways;             // for the RC_ALWAYS special range
    ScDocument           *pDoc;
    ScBroadcastArea      *pUpdateChain;
    ScBroadcastArea      *pEOUpdateChain;
    sal_uLong                 nInBulkBroadcast;

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
    sal_Bool                AreaBroadcast( const ScHint& rHint ) const;
        // return: at least one broadcast occurred
    sal_Bool                AreaBroadcastInRange( const ScRange& rRange, const ScHint& rHint ) const;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
