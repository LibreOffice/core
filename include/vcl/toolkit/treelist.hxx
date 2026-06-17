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

#pragma once

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <config_options.h>
#include <vcl/dllapi.h>
#include <vcl/toolkit/treelistentries.hxx>

#include <tools/link.hxx>
#include <tools/contnr.hxx>

#include <memory>
#include <unordered_map>

enum class SvListAction
{
    INSERTED,
    REMOVING,
    REMOVED,
    MOVING,
    MOVED,
    INSERTED_TREE,
    INVALIDATE_ENTRY,
    RESORTING,
    RESORTED,
};

class SvTreeListBox;
class SvTreeListEntry;
class SvViewDataEntry;

enum class SvSortMode { Ascending, Descending, None };

// For the return values of Sortlink:
// See International::Compare( pLeft, pRight )
// ( Compare(a,b) ==> b.Compare(a) ==> strcmp(a,b) )
struct SvSortData
{
    const SvTreeListEntry* pLeft;
    const SvTreeListEntry* pRight;
};

class UNLESS_MERGELIBS_MORE(VCL_DLLPUBLIC) SvTreeList final
{
    friend class SvTreeListBox;

    SvTreeListBox& mrOwnerListView;
    sal_uInt32 m_nEntryCount;

    Link<SvTreeListEntry&, SvTreeListEntry*> m_aCloneLink;
    Link<const SvSortData&, sal_Int32> m_aCompareLink;
    SvSortMode m_eSortMode;

    bool m_bAbsPositionsValid;

    bool mbEnableInvalidate;

    std::unique_ptr<SvTreeListEntry> m_pRootItem;

    SvTreeListEntry*        FirstVisible() const { return First(); }
    SvTreeListEntry* NextVisible(const SvTreeListBox& rView, SvTreeListEntry* pEntry,
                                 sal_uInt16* pDepth = nullptr) const;
    SvTreeListEntry* PrevVisible(const SvTreeListBox& rView, SvTreeListEntry* pEntry) const;
    SvTreeListEntry* LastVisible(const SvTreeListBox& rView) const;
    SvTreeListEntry* NextVisible(const SvTreeListBox& rView, SvTreeListEntry* pEntry,
                                 sal_uInt16& rDelta) const;
    SvTreeListEntry* PrevVisible(const SvTreeListBox& rView, SvTreeListEntry* pEntry,
                                 sal_uInt16& rDelta) const;

    bool IsEntryVisible(const SvTreeListBox& rView, SvTreeListEntry* pEntry) const;
    SvTreeListEntry* GetEntryAtVisPos(const SvTreeListBox& rView, sal_uInt32 nVisPos) const;
    sal_uInt32 GetVisiblePos(const SvTreeListBox& rView, SvTreeListEntry const* pEntry) const;
    sal_uInt32 GetVisibleCount(SvTreeListBox& rView) const;
    sal_uInt32 GetVisibleChildCount(const SvTreeListBox& rView, SvTreeListEntry* pParent) const;

    SvTreeListEntry* FirstSelected(const SvTreeListBox& rView) const;
    SvTreeListEntry* NextSelected(const SvTreeListBox& rView, SvTreeListEntry* pEntry) const;

    sal_uInt32 GetChildSelectionCount(const SvTreeListBox& rView, SvTreeListEntry* pParent) const;

    SAL_DLLPRIVATE void SetAbsolutePositions();

    SAL_DLLPRIVATE void CloneChildren(
        SvTreeListEntries& rDst, sal_uInt32& rCloneCount, SvTreeListEntries& rSrc, SvTreeListEntry& rNewParent) const;

    /**
     * Invalidate the cached position data to have them re-generated before
     * the next access.
     */
    SAL_DLLPRIVATE static void SetListPositions( SvTreeListEntries& rEntries );

    // rPos is not changed for SortModeNone
    SAL_DLLPRIVATE void GetInsertionPos(
                            const SvTreeListEntry& rEntry,
                            SvTreeListEntry* pParent,
                            sal_uInt32& rPos
                        );

    SAL_DLLPRIVATE void ResortChildren( SvTreeListEntry* pParent );

    SvTreeList(const SvTreeList&) = delete;
    SvTreeList& operator= (const SvTreeList&) = delete;

public:

                        SvTreeList() = delete;
                        SvTreeList(SvTreeListBox&);
                        ~SvTreeList();

    void Broadcast(SvListAction eAction, SvTreeListEntry* pEntry = nullptr);

    void                EnableInvalidate( bool bEnable );
    bool                IsEnableInvalidate() const { return mbEnableInvalidate; }

    // Notify all Listeners
    void InvalidateEntry(SvTreeListEntry& rEntry);

    sal_uInt32 GetEntryCount() const { return m_nEntryCount; }
    SvTreeListEntry*    First() const;
    SvTreeListEntry*    Next( SvTreeListEntry* pEntry, sal_uInt16* pDepth=nullptr ) const;
    SvTreeListEntry*    Last() const;

    SvTreeListEntry* FirstChild(const SvTreeListEntry* pParent) const;

    void Insert(SvTreeListEntry* pEntry, sal_uInt32 nPos, SvTreeListEntry* pPar);

    void                InsertTree( SvTreeListEntry* pTree, SvTreeListEntry* pTargetParent, sal_uInt32 nListPos );

    // Entries need to be in the same Model!
    void                Move( SvTreeListEntry* pSource, SvTreeListEntry* pTarget );

    // Creates ChildList if needed
    sal_uInt32          Move( SvTreeListEntry* pSource, SvTreeListEntry* pTargetParent, sal_uInt32 nListPos);
    sal_uInt32 Copy(SvTreeListEntry& rSource, SvTreeListEntry* pTargetParent, sal_uInt32 nListPos);

    bool Remove( const SvTreeListEntry* pEntry );
    void                Clear();

    bool                IsChild(const SvTreeListEntry* pParent, const SvTreeListEntry* pChild) const;
    SvTreeListEntry*        GetEntry( SvTreeListEntry* pParent, sal_uInt32 nPos ) const;
    SvTreeListEntry*        GetEntry( sal_uInt32 nRootPos ) const;
    SvTreeListEntry*        GetEntryAtAbsPos( sal_uInt32 nAbsPos ) const;

    SvTreeListEntry* GetParent(const SvTreeListEntry* pEntry) const;

    const SvTreeListEntries& GetChildList( SvTreeListEntry* pParent ) const;

    sal_uInt32 GetAbsPos( const SvTreeListEntry* pEntry ) const;
    static sal_uInt32 GetRelPos( const SvTreeListEntry* pChild );

    sal_uInt32 GetChildCount( const SvTreeListEntry* pParent ) const;
    sal_uInt16 GetDepth( const SvTreeListEntry* pEntry ) const;
    bool IsAtRootDepth( const SvTreeListEntry* pEntry ) const;

    // The Model calls the Clone Link to clone Entries.
    // Thus we do not need to derive from the Model if we derive from SvTreeListEntry.
    // The Handler needs to return a SvTreeListEntry*
    SvTreeListEntry* Clone(SvTreeListEntry& rEntry, sal_uInt32& nCloneCount) const;
    void SetCloneLink(const Link<SvTreeListEntry&, SvTreeListEntry*>& rLink)
    {
        m_aCloneLink = rLink;
    }

    SvTreeListEntry* CloneEntry(SvTreeListEntry& rSource) const; // Calls the Clone Link

    void SetSortMode(SvSortMode eMode) { m_eSortMode = eMode; }
    SvSortMode GetSortMode() const { return m_eSortMode; }
    sal_Int32           Compare(const SvTreeListEntry* pLeft, const SvTreeListEntry* pRight) const;
    void SetCompareHdl(const Link<const SvSortData&, sal_Int32>& rLink) { m_aCompareLink = rLink; }
    void                Resort();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
