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
    bool                bInUpdateChain;

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
    inline bool         IsInUpdateChain() const { return bInUpdateChain; }
    inline void         SetInUpdateChain( bool b ) { bInUpdateChain = b; }

    /** Equalness of this or range. */
    inline  bool        operator==( const ScBroadcastArea & rArea ) const;
};

inline bool ScBroadcastArea::operator==( const ScBroadcastArea & rArea ) const
{
    return aRange == rArea.aRange;
}



struct ScBroadcastAreaEntry
{
    ScBroadcastArea* mpArea;
    mutable bool     mbErasure;     ///< TRUE if marked for erasure in this set

    ScBroadcastAreaEntry( ScBroadcastArea* p ) : mpArea( p), mbErasure( false) {}
};

struct ScBroadcastAreaHash
{
    size_t operator()( const ScBroadcastAreaEntry& rEntry ) const
    {
        return rEntry.mpArea->GetRange().hashArea();
    }
};

struct ScBroadcastAreaEqual
{
    bool operator()( const ScBroadcastAreaEntry& rEntry1, const ScBroadcastAreaEntry& rEntry2) const
    {
        return *rEntry1.mpArea == *rEntry2.mpArea;
    }
};

typedef ::boost::unordered_set< ScBroadcastAreaEntry, ScBroadcastAreaHash, ScBroadcastAreaEqual > ScBroadcastAreas;



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



class ScBroadcastAreaSlotMachine;

/// Collection of BroadcastAreas
class ScBroadcastAreaSlot
{
private:
    ScBroadcastAreas    aBroadcastAreaTbl;
    mutable ScBroadcastArea aTmpSeekBroadcastArea;      // for FindBroadcastArea()
    ScDocument*         pDoc;
    ScBroadcastAreaSlotMachine* pBASM;
    bool                mbInBroadcastIteration;

    ScBroadcastAreas::const_iterator  FindBroadcastArea( const ScRange& rRange ) const;

    /**
        More hypothetical (memory would probably be doomed anyway) check
        whether there would be an overflow when adding an area, setting the
        proper state if so.

        @return true if a HardRecalcState is effective and area is not to be
        added.
      */
    bool                CheckHardRecalcStateCondition() const;

    /** Finally erase all areas pushed as to-be-erased. */
    void                FinallyEraseAreas();

    bool                isMarkedErased( const ScBroadcastAreas::iterator& rIter )
                            {
                                return (*rIter).mbErasure;
                            }

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
            true if rpArea passed was NULL and ScBroadcastArea is newly
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
    bool                AreaBroadcast( const ScHint& rHint );
    /// @return true if at least one broadcast occurred.
    bool                AreaBroadcastInRange( const ScRange& rRange,
                                              const ScHint& rHint );
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateRemove( UpdateRefMode eUpdateRefMode,
                                        const ScRange& rRange,
                                        SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void                UpdateRemoveArea( ScBroadcastArea* pArea );
    void                UpdateInsert( ScBroadcastArea* pArea );


    bool                IsInBroadcastIteration() const { return mbInBroadcastIteration; }

    /** Erase an area from set and delete it if last reference, or if
        mbInBroadcastIteration is set push it to the vector of to-be-erased
        areas instead.

        Meant to be used internally and from ScBroadcastAreaSlotMachine only.
     */
    void                EraseArea( ScBroadcastAreas::iterator& rIter );
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

    typedef ::std::vector< ::std::pair< ScBroadcastAreaSlot*, ScBroadcastAreas::iterator > > AreasToBeErased;

private:
    ScBroadcastAreasBulk  aBulkBroadcastAreas;
    TableSlotsMap         aTableSlotsMap;
    AreasToBeErased       maAreasToBeErased;
    SvtBroadcaster       *pBCAlways;             // for the RC_ALWAYS special range
    ScDocument           *pDoc;
    ScBroadcastArea      *pUpdateChain;
    ScBroadcastArea      *pEOUpdateChain;
    sal_uLong             nInBulkBroadcast;

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
    bool                AreaBroadcast( const ScHint& rHint ) const;
        // return: at least one broadcast occurred
    bool                AreaBroadcastInRange( const ScRange& rRange, const ScHint& rHint ) const;
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

    // only for ScBroadcastAreaSlot
    void                PushAreaToBeErased( ScBroadcastAreaSlot* pSlot,
                                            ScBroadcastAreas::iterator& rIter );
    // only for ScBroadcastAreaSlot
    void                FinallyEraseAreas( ScBroadcastAreaSlot* pSlot );
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
