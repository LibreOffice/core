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

#ifndef INCLUDED_SC_SOURCE_CORE_INC_BCASLOT_HXX
#define INCLUDED_SC_SOURCE_CORE_INC_BCASLOT_HXX

#include <functional>
#include <memory>
#include <map>
#include <set>
#include <unordered_set>

#include <svl/broadcast.hxx>

#include "global.hxx"
#include "brdcst.hxx"
#include <columnspanset.hxx>

class ScBroadcastArea;

namespace sc {

struct AreaListener
{
    ScRange maArea;
    bool mbGroupListening;
    SvtListener* mpListener;
};

}

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

    bool mbInUpdateChain:1;
    bool mbGroupListening:1;

public:
    ScBroadcastArea(const ScBroadcastArea&) = delete;
    const ScBroadcastArea& operator=(const ScBroadcastArea&) = delete;

    ScBroadcastArea( const ScRange& rRange );

    SvtBroadcaster&       GetBroadcaster()       { return aBroadcaster; }
    const SvtBroadcaster& GetBroadcaster() const { return aBroadcaster; }
    void         UpdateRange( const ScRange& rNewRange )
                            { aRange = rNewRange; }
    const ScRange&   GetRange() const { return aRange; }
    void         IncRef() { ++nRefCount; }
    sal_uLong        DecRef() { return nRefCount ? --nRefCount : 0; }
    sal_uLong        GetRef() { return nRefCount; }
    ScBroadcastArea* GetUpdateChainNext() const { return pUpdateChainNext; }
    void         SetUpdateChainNext( ScBroadcastArea* p ) { pUpdateChainNext = p; }
    bool         IsInUpdateChain() const { return mbInUpdateChain; }
    void         SetInUpdateChain( bool b ) { mbInUpdateChain = b; }

    bool IsGroupListening() const { return mbGroupListening; }
    void SetGroupListening( bool b ) { mbGroupListening = b; }

    /** Equalness of this or range. */
    inline  bool        operator==( const ScBroadcastArea & rArea ) const;
};

inline bool ScBroadcastArea::operator==( const ScBroadcastArea & rArea ) const
{
    return aRange == rArea.aRange && mbGroupListening == rArea.mbGroupListening;
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
        return rEntry.mpArea->GetRange().hashArea() + static_cast<size_t>(rEntry.mpArea->IsGroupListening());
    }
};

struct ScBroadcastAreaEqual
{
    bool operator()( const ScBroadcastAreaEntry& rEntry1, const ScBroadcastAreaEntry& rEntry2) const
    {
        return *rEntry1.mpArea == *rEntry2.mpArea;
    }
};

typedef std::unordered_set< ScBroadcastAreaEntry, ScBroadcastAreaHash, ScBroadcastAreaEqual > ScBroadcastAreas;

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

typedef std::unordered_set< const ScBroadcastArea*, ScBroadcastAreaBulkHash,
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

    /**
     * If true, the slot has at least one area broadcaster marked for removal.
     * This flag is used only during broadcast iteration, to speed up
     * iteration.  Using this flag is cheaper than dereferencing each iterator
     * and checking its own flag inside especially when no areas are marked
     * for removal.
     */
    bool mbHasErasedArea;

    ScBroadcastAreas::iterator FindBroadcastArea( const ScRange& rRange, bool bGroupListening );

    /**
        More hypothetical (memory would probably be doomed anyway) check
        whether there would be an overflow when adding an area, setting the
        proper state if so.

        @return HARDRECALCSTATE_ETERNAL if a HardRecalcState is effective and
                area is not to be added.
      */
    ScDocument::HardRecalcState CheckHardRecalcStateCondition() const;

    /** Finally erase all areas pushed as to-be-erased. */
    void                FinallyEraseAreas();

    static bool         isMarkedErased( const ScBroadcastAreas::const_iterator& rIter )
    {
        return rIter->mbErasure;
    }

public:
                        ScBroadcastAreaSlot( ScDocument* pDoc,
                                        ScBroadcastAreaSlotMachine* pBASM );
                        ~ScBroadcastAreaSlot();

    /**
        Only here new ScBroadcastArea objects are created, prevention of dupes.

        @param rpArea
            If NULL, a new ScBroadcastArea is created and assigned ton the
            reference if a matching area wasn't found. If a matching area was
            found, that is assigned. In any case, the SvtListener is added to
            the broadcaster.

            If not NULL then no listeners are started, only the area is
            inserted and the reference count incremented. Effectively the same
            as InsertListeningArea(), so use that instead.

        @return
            true if rpArea passed was NULL and ScBroadcastArea is newly
            created.
     */
    bool StartListeningArea(
        const ScRange& rRange, bool bGroupListening, SvtListener* pListener, ScBroadcastArea*& rpArea );

    /**
        Insert a ScBroadcastArea obtained via StartListeningArea() to
        subsequent slots.
     */
    void                InsertListeningArea( ScBroadcastArea* pArea );

    void EndListeningArea(
        const ScRange& rRange, bool bGroupListening, SvtListener* pListener, ScBroadcastArea*& rpArea );

    bool AreaBroadcast( const ScRange& rRange, SfxHintId nHint );
    bool                AreaBroadcast( const ScHint& rHint );
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateRemove( UpdateRefMode eUpdateRefMode,
                                        const ScRange& rRange,
                                        SCCOL nDx, SCROW nDy, SCTAB nDz );
    void                UpdateRemoveArea( ScBroadcastArea* pArea );
    void                UpdateInsert( ScBroadcastArea* pArea );

    bool                IsInBroadcastIteration() const { return mbInBroadcastIteration; }

    /** Erase an area from set and delete it if last reference, or if
        mbInBroadcastIteration is set push it to the vector of to-be-erased
        areas instead.

        Meant to be used internally and from ScBroadcastAreaSlotMachine only.
     */
    void                EraseArea( ScBroadcastAreas::iterator& rIter );

    void GetAllListeners(
        const ScRange& rRange, std::vector<sc::AreaListener>& rListeners,
        sc::AreaOverlapType eType, sc::ListenerGroupType eGroup );

#if DEBUG_AREA_BROADCASTER
    void Dump() const;
#endif
};

/**
    BroadcastAreaSlots and their management, once per document.
 */

class  ScBroadcastAreaSlotMachine
{
private:
    typedef std::map<ScBroadcastArea*, std::unique_ptr<sc::ColumnSpanSet>> BulkGroupAreasType;

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
        ScBroadcastAreaSlot**    getSlots() { return ppSlots.get(); }

        /**
            Obtain slot pointer, no check on validity! It is assumed that
            all calls are made with the results of ComputeSlotOffset(),
            ComputeAreaPoints() and ComputeNextSlot()
          */
        ScBroadcastAreaSlot*     getAreaSlot( SCSIZE nOff ) { return ppSlots[nOff]; }

    private:
        std::unique_ptr<ScBroadcastAreaSlot*[]>   ppSlots;

        TableSlots( const TableSlots& ) = delete;
        TableSlots& operator=( const TableSlots& ) = delete;
    };

    typedef ::std::map< SCTAB, TableSlots* > TableSlotsMap;

    typedef ::std::vector< ::std::pair< ScBroadcastAreaSlot*, ScBroadcastAreas::iterator > > AreasToBeErased;

private:
    ScBroadcastAreasBulk  aBulkBroadcastAreas;
    BulkGroupAreasType m_BulkGroupAreas;
    TableSlotsMap         aTableSlotsMap;
    AreasToBeErased       maAreasToBeErased;
    SvtBroadcaster       *pBCAlways;             // for the RC_ALWAYS special range
    ScDocument           *pDoc;
    ScBroadcastArea      *pUpdateChain;
    ScBroadcastArea      *pEOUpdateChain;
    sal_uInt32            nInBulkBroadcast;

    static inline SCSIZE ComputeSlotOffset( const ScAddress& rAddress );
    static void          ComputeAreaPoints( const ScRange& rRange,
                                            SCSIZE& nStart, SCSIZE& nEnd,
                                            SCSIZE& nRowBreak );

public:
                        ScBroadcastAreaSlotMachine( ScDocument* pDoc );
                        ~ScBroadcastAreaSlotMachine();
    void StartListeningArea(
        const ScRange& rRange, bool bGroupListening, SvtListener* pListener );

    void EndListeningArea(
        const ScRange& rRange, bool bGroupListening, SvtListener* pListener );

    bool AreaBroadcast( const ScRange& rRange, SfxHintId nHint );
    bool                AreaBroadcast( const ScHint& rHint ) const;
        // return: at least one broadcast occurred
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateBroadcastAreas( UpdateRefMode eUpdateRefMode,
                                            const ScRange& rRange,
                                            SCCOL nDx, SCROW nDy, SCTAB nDz );
    void                EnterBulkBroadcast();
    void                LeaveBulkBroadcast( SfxHintId nHintId );
    bool                InsertBulkArea( const ScBroadcastArea* p );

    void InsertBulkGroupArea( ScBroadcastArea* pArea, const ScRange& rRange );
    void RemoveBulkGroupArea( ScBroadcastArea* pArea );
    bool BulkBroadcastGroupAreas( SfxHintId nHintId );

    /// @return: how many removed
    size_t              RemoveBulkArea( const ScBroadcastArea* p );
    void SetUpdateChain( ScBroadcastArea* p ) { pUpdateChain = p; }
    ScBroadcastArea* GetEOUpdateChain() const { return pEOUpdateChain; }
    void SetEOUpdateChain( ScBroadcastArea* p ) { pEOUpdateChain = p; }
    bool IsInBulkBroadcast() const { return nInBulkBroadcast > 0; }

    // only for ScBroadcastAreaSlot
    void                PushAreaToBeErased( ScBroadcastAreaSlot* pSlot,
                                            ScBroadcastAreas::iterator& rIter );
    // only for ScBroadcastAreaSlot
    void                FinallyEraseAreas( ScBroadcastAreaSlot* pSlot );

    std::vector<sc::AreaListener> GetAllListeners(
        const ScRange& rRange, sc::AreaOverlapType eType,
        sc::ListenerGroupType eGroup = sc::ListenerGroupType::Both );

#if DEBUG_AREA_BROADCASTER
    void Dump() const;
#endif
};

class ScBulkBroadcast
{
    ScBroadcastAreaSlotMachine* pBASM;
    SfxHintId                  mnHintId;
public:
    explicit ScBulkBroadcast( ScBroadcastAreaSlotMachine* p, SfxHintId nHintId ) :
        pBASM(p),
        mnHintId(nHintId)
    {
        if (pBASM)
            pBASM->EnterBulkBroadcast();
    }
    ~ScBulkBroadcast() COVERITY_NOEXCEPT_FALSE
    {
        if (pBASM)
            pBASM->LeaveBulkBroadcast( mnHintId );
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
