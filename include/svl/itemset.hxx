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

#include <sal/config.h>

#include <cassert>
#include <memory>
#include <utility>
#include <unordered_map>

#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>
#include <svl/typedwhich.hxx>
#include <svl/whichranges.hxx>

class SfxItemPool;

#ifdef DBG_UTIL
SVL_DLLPUBLIC size_t getAllocatedSfxItemSetCount();
SVL_DLLPUBLIC size_t getUsedSfxItemSetCount();
SVL_DLLPUBLIC size_t getAllocatedSfxPoolItemHolderCount();
SVL_DLLPUBLIC size_t getUsedSfxPoolItemHolderCount();
SVL_DLLPUBLIC void listSfxPoolItemsWithHighestUsage(sal_uInt16);
SVL_DLLPUBLIC void listSfxItemSetUsage();
#endif

// ItemSet/ItemPool helpers
SfxPoolItem const* implCreateItemEntry(SfxItemPool& rPool, SfxPoolItem const* pSource, bool bPassingOwnership);
void implCleanupItemEntry(const SfxPoolItem* pSource);

// coverity[ missing_move_assignment : SUPPRESS] - don't report about missing move assignment
class SAL_WARN_UNUSED SVL_DLLPUBLIC SfxPoolItemHolder
{
    SfxItemPool*            m_pPool;
    const SfxPoolItem*      m_pItem;
#ifndef NDEBUG
    bool                    m_bDeleted;
#endif
public:
    SfxPoolItemHolder();
    SfxPoolItemHolder(SfxItemPool&, const SfxPoolItem*, bool bPassingOwnership = false);
    SfxPoolItemHolder(const SfxPoolItemHolder&);
    ~SfxPoolItemHolder();

#ifndef NDEBUG
    bool isDeleted() const { return m_bDeleted; }
#endif

    const SfxPoolItemHolder& operator=(const SfxPoolItemHolder&);
    bool operator==(const SfxPoolItemHolder &) const;

    SfxItemPool& getPool() const { assert(!isDeleted() && "Destructed instance used (!)"); return *m_pPool; }
    const SfxPoolItem* getItem() const { assert(!isDeleted() && "Destructed instance used (!)"); return m_pItem; }

    bool operator!() const { return nullptr == m_pItem; }
    explicit operator bool() const { return nullptr != m_pItem; }
    bool is() const { return nullptr != m_pItem; }

    sal_uInt16 Which() const { if(nullptr != m_pItem) return m_pItem->Which(); return 0; }
};

typedef std::unordered_map<sal_uInt16, const SfxPoolItem*> PoolItemMap;

class SAL_WARN_UNUSED SVL_DLLPUBLIC SfxItemSet
{
    friend class SfxItemIter;
    friend class SfxWhichIter;
    friend class SfxAllItemSet;

    // allow ItemSetTooling to access
    friend SfxPoolItem const* implCreateItemEntry(SfxItemPool&, SfxPoolItem const*, bool);
    friend void implCleanupItemEntry(const SfxPoolItem*);

    SfxItemPool*      m_pPool;         ///< pool that stores the items
    const SfxItemSet* m_pParent;       ///< derivation
    sal_uInt16        m_nRegister;     ///< number of items with NeedsSurrogateSupport

#ifdef DBG_UTIL
    sal_uInt16          m_nRegisteredSfxItemIter;
#endif

    WhichRangesContainer m_aWhichRanges;  ///< array of Which Ranges

    // the set SfxPoolItems, indexed by WhichID
    PoolItemMap         m_aPoolItemMap;

    // Notification-Callback mechanism for SwAttrSet in SW, functionPtr for callback
    std::function<void(const SfxPoolItem*, const SfxPoolItem*)> m_aCallback;

    // helpers to keep m_nRegister up-to-date
    void checkRemovePoolRegistration(const SfxPoolItem* pItem);
    void checkAddPoolRegistration(const SfxPoolItem* pItem);

protected:
    // Notification-Callback mechanism for SwAttrSet in SW
    void setCallback(const std::function<void(const SfxPoolItem*, const SfxPoolItem*)> &func) { m_aCallback = func; }
    void clearCallback() { m_aCallback = nullptr; }

    virtual const SfxPoolItem*  PutImpl( const SfxPoolItem&, bool bPassingOwnership );
    const SfxPoolItem* PutImplAsTargetWhich( const SfxPoolItem&, sal_uInt16 nTargetWhich, bool bPassingOwnership );

private:
    SVL_DLLPRIVATE void RecreateRanges_Impl(const WhichRangesContainer& pNewRanges);
    const SfxItemSet& operator=(const SfxItemSet &) = delete;

public:
    SfxItemSet( const SfxItemSet& );
    SfxItemSet( SfxItemSet&& ) noexcept;
    SfxItemSet( SfxItemPool& );
    SfxItemSet( SfxItemPool&, WhichRangesContainer ranges );
    SfxItemSet( SfxItemPool& rPool, sal_uInt16 nWhichStart, sal_uInt16 nWhichEnd )
        : SfxItemSet(rPool, WhichRangesContainer(nWhichStart, nWhichEnd)) {}

    template<sal_uInt16... WIDs>
    static SfxItemSet makeFixedSfxItemSet(SfxItemPool& pool)
    {
        return SfxItemSet(pool, WhichRangesContainer(svl::Items_t<WIDs...>{}));
    }

    virtual ~SfxItemSet();

    virtual std::unique_ptr<SfxItemSet> Clone(bool bItems = true, SfxItemPool *pToPool = nullptr) const;
    /** note that this only works if you know for sure that you are dealing with an SfxItemSet
        and not one of it's subclasses. */
    SfxItemSet CloneAsValue(bool bItems = true, SfxItemPool *pToPool = nullptr) const;

    // Get number of items
    sal_uInt16                  Count() const { return m_aPoolItemMap.size(); }
    sal_uInt16                  TotalCount() const { return m_aWhichRanges.TotalCount(); }

    const SfxPoolItem&          Get( sal_uInt16 nWhich, bool bSrchInParent = true ) const;
    template<class T>
    const T&                    Get( TypedWhichId<T> nWhich, bool bSrchInParent = true ) const
    {
        return static_cast<const T&>(Get(sal_uInt16(nWhich), bSrchInParent));
    }

    /** This method eases accessing single Items in the SfxItemSet.

        @param nId SlotId or the Item's WhichId
        @param bSearchInParent also search in parent ItemSets
        @returns 0 if the ItemSet does not contain an Item with the Id 'nWhich'
    */
    const SfxPoolItem*          GetItem(sal_uInt16 nWhich, bool bSearchInParent = true) const;

    /// Templatized version of GetItem() to directly return the correct type.
    template<class T> const T* GetItem(sal_uInt16 nWhich, bool bSearchInParent = true) const
    {
        const SfxPoolItem* pItem = GetItem(nWhich, bSearchInParent);
        const T* pCastedItem = dynamic_cast<const T*>(pItem);

        assert(!pItem || pCastedItem); // if it exists, must have the correct type
        return pCastedItem;
    }
    template<class T> const T* GetItem( TypedWhichId<T> nWhich, bool bSearchInParent = true ) const
    {
        return GetItem<T>(sal_uInt16(nWhich), bSearchInParent);
    }

    /// Templatized static version of GetItem() to directly return the correct type if the SfxItemSet is available.
    template<class T> static const T* GetItem(const SfxItemSet* pItemSet, sal_uInt16 nWhich, bool bSearchInParent)
    {
        if (pItemSet)
            return pItemSet->GetItem<T>(nWhich, bSearchInParent);

        return nullptr;
    }
    template <class T>
    static const T* GetItem(const SfxItemSet* pItemSet, TypedWhichId<T> nWhich,
                            bool bSearchInParent)
    {
        return GetItem<T>(pItemSet, static_cast<sal_uInt16>(nWhich), bSearchInParent);
    }

    SfxItemState GetItemState(sal_uInt16 nWhich, bool bSrchInParent = true, const SfxPoolItem **ppItem = nullptr) const
    {
        // use local helper, start value for looped-through SfxItemState value is SfxItemState::UNKNOWN
        return GetItemState_ForWhichID(SfxItemState::UNKNOWN, nWhich, bSrchInParent, ppItem);
    }

    template <class T> SfxItemState GetItemState(TypedWhichId<T> nWhich, bool bSrchInParent = true, const T **ppItem = nullptr ) const
    {
        // use local helper, start value for looped-through SfxItemState value is SfxItemState::UNKNOWN
        return GetItemState_ForWhichID(SfxItemState::UNKNOWN, sal_uInt16(nWhich), bSrchInParent, reinterpret_cast<SfxPoolItem const**>(ppItem));
    }

    /// Templatized version of GetItemState() to directly return the correct type.
    template<class T>
    const T *                   GetItemIfSet(   TypedWhichId<T> nWhich,
                                                bool bSrchInParent = true ) const
    {
        const SfxPoolItem * pItem = nullptr;
        if (SfxItemState::SET == GetItemState_ForWhichID(SfxItemState::UNKNOWN, sal_uInt16(nWhich), bSrchInParent, &pItem))
            return static_cast<const T*>(pItem);
        return nullptr;
    }

    bool                        HasItem(sal_uInt16 nWhich, const SfxPoolItem** ppItem = nullptr) const;
    template<class T>
    bool                        HasItem(TypedWhichId<T> nWhich, const T** ppItem = nullptr) const
    { return HasItem(sal_uInt16(nWhich), reinterpret_cast<const SfxPoolItem**>(ppItem)); }

    void DisableItem(sal_uInt16 nWhich)
        { DisableOrInvalidateItem_ForWhichID(true, nWhich); }
    void InvalidateItem(sal_uInt16 nWhich)
        { DisableOrInvalidateItem_ForWhichID(false, nWhich); }
    sal_uInt16                  ClearItem( sal_uInt16 nWhich = 0);
    void                        ClearInvalidItems();
    inline void                 SetParent( const SfxItemSet* pNew )
    {
        m_pParent = pNew;
    }

    // add, delete items, work on items
    const SfxPoolItem*          Put( const SfxPoolItem& rItem )
        { return PutImpl(rItem, /*bPassingOwnership*/false); }
    const SfxPoolItem*          Put( std::unique_ptr<SfxPoolItem> xItem )
        { return PutImpl(*xItem.release(), /*bPassingOwnership*/true); }
    const SfxPoolItem* PutAsTargetWhich(const SfxPoolItem& rItem, sal_uInt16 nTargetWhich )
        { return PutImplAsTargetWhich(rItem, nTargetWhich, false); }
    const SfxPoolItem* PutAsTargetWhich(std::unique_ptr<SfxPoolItem> xItem, sal_uInt16 nTargetWhich )
        { return PutImplAsTargetWhich(*xItem.release(), nTargetWhich, true); }
    bool                        Put( const SfxItemSet&,
                                     bool bInvalidAsDefault = true );
    void                        PutExtended( const SfxItemSet&,
                                             SfxItemState eDontCareAs,
                                             SfxItemState eDefaultAs );

    bool                        Set( const SfxItemSet&, bool bDeep = true );

    void                        Intersect( const SfxItemSet& rSet );
    void                        MergeValues( const SfxItemSet& rSet );
    void                        Differentiate( const SfxItemSet& rSet );
    void                        MergeValue( const SfxPoolItem& rItem);

    SfxItemPool*                GetPool() const { return m_pPool; }
    const WhichRangesContainer & GetRanges() const { return m_aWhichRanges; }
    void                        SetRanges( const WhichRangesContainer& );
    void                        SetRanges( WhichRangesContainer&& );
    void                        MergeRange( sal_uInt16 nFrom, sal_uInt16 nTo );
    const SfxItemSet*           GetParent() const { return m_pParent; }

    bool                        operator==(const SfxItemSet &) const;

    /** Compare possibly ignoring SfxItemPool pointer.

        This can be used to compare the content of two SfxItemSet even if they
        don't share the same pool. EditTextObject::Equals(...,false) uses this
        which is needed in ScGlobal::EETextObjEqual() for
        ScPageHFItem::operator==()

        @param  bComparePool
                if <FALSE/> ignore SfxItemPool pointer,
                if <TRUE/> compare also SfxItemPool pointer (identical to operator==())
     */
    bool                        Equals(const SfxItemSet &, bool bComparePool) const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

private:
    // split version(s) of ClearSingleItemImpl for input type WhichID
    sal_uInt16 ClearSingleItem_ForWhichID( sal_uInt16 nWhich );
    void ClearSingleItem_PrepareRemove(const SfxPoolItem* pItem);

    // cleanup all Items, but do not reset/change m_ppItems array. That is
    // responsibility of the caller & allows specific resets
    sal_uInt16 ClearAllItemsImpl();

    // Merge two given Item(entries)
    void MergeItem_Impl(sal_uInt16 nWhich, const SfxPoolItem *pFnd2, bool bIgnoreDefaults);

    // InvalidateItem/DisableItem for input types WhichID and Offset
    void DisableOrInvalidateItem_ForWhichID(bool bDsiable, sal_uInt16 nWhich);

    // GetItemStateImpl for input type WhichID
    SfxItemState GetItemState_ForWhichID( SfxItemState eState, sal_uInt16 nWhich, bool bSrchInParent, const SfxPoolItem **ppItem) const;
};

//  Handles all Ranges. Ranges are automatically modified by putting items.
class SVL_DLLPUBLIC SfxAllItemSet final : public SfxItemSet
{
public:
                                SfxAllItemSet( SfxItemPool &rPool );
                                SfxAllItemSet( const SfxItemSet & );
                                SfxAllItemSet( const SfxAllItemSet & );

    virtual std::unique_ptr<SfxItemSet> Clone( bool bItems = true, SfxItemPool *pToPool = nullptr ) const override;
private:
    virtual const SfxPoolItem*  PutImpl( const SfxPoolItem&, bool bPassingOwnership ) override;
};


// Was: Allocate the items array inside the object, to reduce allocation cost.
// NOTE: No longer needed with unordered_set, but there are 560+ places to
// replace/adapt, so keep it for now.
// To adapt, there is the static makeFixedSfxItemSet member in SfxItemSet. Did
// that in one place to check functionality (easy hack...?)
template<sal_uInt16... WIDs>
class SfxItemSetFixed : public SfxItemSet
{
public:
    SfxItemSetFixed( SfxItemPool& rPool)
        : SfxItemSet(rPool, WhichRangesContainer(svl::Items_t<WIDs...>{})){}//,
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
