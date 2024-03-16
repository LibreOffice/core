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

#include <config_options.h>
#include <rtl/ref.hxx>
#include <svl/poolitem.hxx>
#include <svl/svldllapi.h>
#include <svl/typedwhich.hxx>
#include <svl/whichranges.hxx>
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <o3tl/sorted_vector.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <svl/SfxBroadcaster.hxx>

// flag definitions to be used for _nItemInfoFlags
// in SfxItemInfo
#define SFX_ITEMINFOFLAG_NONE               0x0000

// Defines if this Item needs to be registered at the pool
// to make it accessible for the GetItemSurrogates call. It
// will not be included when this flag is not set, but also
// needs no registration. There are SAL_INFO calls in the
// GetItemSurrogates impl that will mention that
#define SFX_ITEMINFOFLAG_SUPPORT_SURROGATE  0x0001

class SVL_DLLPUBLIC ItemInfo
{
    sal_uInt16          m_nWhich;
    sal_uInt16          m_nSlotID;
    sal_uInt16          m_nItemInfoFlags;

public:
    ItemInfo(sal_uInt16 nWhich, sal_uInt16 nSlotID, sal_uInt16 nItemInfoFlags)
    : m_nWhich(nWhich), m_nSlotID(nSlotID), m_nItemInfoFlags(nItemInfoFlags) {}
    ItemInfo(const ItemInfo& rIemInfo) = default;
    virtual ~ItemInfo() = default;

    sal_uInt16 getWhich() const { return m_nWhich; }
    virtual const SfxPoolItem* getItem() const = 0;
    sal_uInt16 getSlotID() const { return m_nSlotID; }
    sal_uInt16 getItemInfoFlags() const { return m_nItemInfoFlags; }
};

class SVL_DLLPUBLIC ItemInfoStatic : public ItemInfo
{
    friend class ItemInfoPackage;
    void setItem(SfxPoolItem* pItem)
    {
        if (nullptr != pItem)
            pItem->setStaticDefault();
        m_pItem.reset(pItem);
    }

    std::unique_ptr<const SfxPoolItem> m_pItem;

public:
    ItemInfoStatic(sal_uInt16 nWhich, SfxPoolItem* pItem, sal_uInt16 nSlotID, sal_uInt16 nItemInfoFlags)
    : ItemInfo(nWhich, nSlotID, nItemInfoFlags)
    , m_pItem(pItem) { if(nullptr != pItem) pItem->setStaticDefault(); }

    virtual const SfxPoolItem* getItem() const override { return m_pItem.get(); }
};

class SVL_DLLPUBLIC ItemInfoDynamic : public ItemInfo
{
    std::unique_ptr<const SfxPoolItem> m_pItem;

public:
    ItemInfoDynamic(const ItemInfo& rItemInfo, SfxPoolItem* pItem)
    : ItemInfo(rItemInfo)
    , m_pItem(pItem) { if(nullptr != pItem) pItem->setDynamicDefault(); }

    virtual const SfxPoolItem* getItem() const override { return m_pItem.get(); }
};

class UNLESS_MERGELIBS(SVL_DLLPUBLIC) ItemInfoUser : public ItemInfo
{
    const SfxPoolItem* m_pItem;

public:
    ItemInfoUser(const ItemInfo& rItemInfo, SfxItemPool& rItemPool, const SfxPoolItem& rItem, bool bPassingOwnership = false);
    virtual ~ItemInfoUser();

    virtual const SfxPoolItem* getItem() const override { return m_pItem; }
};

typedef std::unordered_map<sal_uInt16, sal_uInt16> SlotIDToWhichIDMap;

class SVL_DLLPUBLIC ItemInfoPackage
{
protected:
    // this is needed for on-demand creation of static entries in constructors
    // derived from ItemInfoPackage or implementations of ::getItemInfo(). This
    // takes ownership of the item
    static void setItemAtItemInfoStatic(SfxPoolItem* pItem, ItemInfoStatic& rItemInfo) { rItemInfo.setItem(pItem); }

private:
    // mechanism for buffered SlotIDToWhichIDMap
    virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const = 0;
    mutable SlotIDToWhichIDMap maSlotIDToWhichIDMap;

public:
    ItemInfoPackage() = default;
    virtual ~ItemInfoPackage() = default;

    virtual size_t size() const = 0;
    virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& rPool) = 0;
    virtual const ItemInfo& getExistingItemInfo(size_t /*nIndex*/);
    const SlotIDToWhichIDMap& getSlotIDToWhichIDMap() const;
};

typedef std::unordered_set<SfxItemSet*> registeredSfxItemSets;
class SfxPoolItemHolder;
typedef std::unordered_set<SfxPoolItemHolder*> registeredSfxPoolItemHolders;
typedef std::vector<const SfxPoolItem*> ItemSurrogates;
typedef std::unordered_map<sal_uInt16, const ItemInfo*> userItemInfos;
typedef std::vector<const ItemInfo*> itemInfoVector;

/** Base class for providers of defaults of SfxPoolItems.
 *
 * The derived classes hold the concrete (const) instances which are referenced in several places
 * (usually within a single document).
 * This helps to lower the amount of calls to lifecycle methods, speeds up comparisons within a document
 * and facilitates loading and saving of attributes.
 */
class SVL_DLLPUBLIC SfxItemPool : public salhelper::SimpleReferenceObject
{
    friend class SfxItemSet;
    friend class SfxPoolItemHolder;
    friend class SfxAllItemSet;

    // allow ItemSetTooling to access
    friend SfxPoolItem const* implCreateItemEntry(SfxItemPool&, SfxPoolItem const*, bool);
    friend void implCleanupItemEntry(SfxPoolItem const*);

    SfxBroadcaster                  aBC;
    OUString                        aName;
    SfxItemPool*                    mpMaster;
    rtl::Reference<SfxItemPool>     mpSecondary;
    mutable WhichRangesContainer    maPoolRanges;
    sal_uInt16                      mnStart;
    sal_uInt16                      mnEnd;
    MapUnit                         eDefMetric;

    registeredSfxItemSets maRegisteredSfxItemSets;
    registeredSfxPoolItemHolders maRegisteredSfxPoolItemHolders;
    bool mbShutdownHintSent;

    itemInfoVector maItemInfos;
    userItemInfos maUserItemInfos;
    const SlotIDToWhichIDMap* mpSlotIDToWhichIDMap;

public:
    void registerItemInfoPackage(
        ItemInfoPackage& rPackage,
        const std::function<SfxPoolItem*(sal_uInt16)>& rCallback = std::function<SfxPoolItem*(sal_uInt16)>());
protected:
    const ItemInfo* impCheckItemInfoForClone(const ItemInfo* pInfo);
    void impClearUserDefault(userItemInfos::iterator& rHit);
    void impCreateUserDefault(const SfxPoolItem& rItem);
private:
    void cleanupItemInfos();

private:
    sal_uInt16 GetIndex_Impl(sal_uInt16 nWhich) const
    {
        if (IsInRange(nWhich))
            return nWhich - mnStart;
        assert(false && "missing bounds check before use");
        return 0;
    }
    sal_uInt16 GetSize_Impl() const { return mnEnd - mnStart + 1; }
    SfxItemPool* getTargetPool(sal_uInt16 nWhich) const;

    // moved to private: use the access methods, e.g. NeedsSurrogateSupport
    SVL_DLLPRIVATE bool CheckItemInfoFlag(sal_uInt16 nWhich, sal_uInt16 nMask) const;
    SVL_DLLPRIVATE bool CheckItemInfoFlag_Impl(sal_uInt16 nPos, sal_uInt16 nMask) const
        { return maItemInfos[nPos]->getItemInfoFlags() & nMask; }

    void registerItemSet(SfxItemSet& rSet);
    void unregisterItemSet(SfxItemSet& rSet);

    void registerPoolItemHolder(SfxPoolItemHolder& rHolder);
    void unregisterPoolItemHolder(SfxPoolItemHolder& rHolder);

public:
    // for default SfxItemSet::CTOR, set default WhichRanges
    const WhichRangesContainer& GetMergedIdRanges() const;

protected:
    static inline void              AddRef(const SfxPoolItem& rItem);
    static inline sal_uInt32        ReleaseRef(const SfxPoolItem& rItem, sal_uInt32 n = 1);

public:
    SfxItemPool(const SfxItemPool &rPool);
    SfxItemPool(const OUString &rName);
    virtual ~SfxItemPool();

    SfxBroadcaster&                 BC();

    // UserDefaults: Every PoolDefault can be 'overloaded' with a user-defined
    // default. This is then owned by the pool. The read access is limited
    // to check the UserDefaults, so it *will* return nullptr if none is set
    void                            SetUserDefaultItem( const SfxPoolItem& );
    const SfxPoolItem*              GetUserDefaultItem( sal_uInt16 nWhich ) const;
    template<class T> const T*      GetUserDefaultItem( TypedWhichId<T> nWhich ) const
    { return static_cast<const T*>(GetUserDefaultItem(sal_uInt16(nWhich))); }
    void                            ResetUserDefaultItem( sal_uInt16 nWhich );

    // PoolDefaults: Owned by the pool. The read access will only return
    // nullptr if the WhichID asked for is not in the range of the pool,
    // making the request invalid.
    const SfxPoolItem *             GetPoolDefaultItem(sal_uInt16 nWhich) const;
    template<class T> const T*      GetPoolDefaultItem( TypedWhichId<T> nWhich ) const
    { return static_cast<const T*>(GetPoolDefaultItem(sal_uInt16(nWhich))); }

    // UserOrPoolDefaults: Combination of UserDefaults and PoolDefaults.
    // UserDefaults will be preferred. If none is set for that WhichID,
    // the PoolDefault will be returned.
    // Note that read access will return a reference, but this will lead
    // to an asserted error when the given WhichID is not in the range of
    // the pool.
    const SfxPoolItem&              GetUserOrPoolDefaultItem( sal_uInt16 nWhich ) const;
    template<class T> const T&      GetUserOrPoolDefaultItem( TypedWhichId<T> nWhich ) const
    { return static_cast<const T&>(GetUserOrPoolDefaultItem(sal_uInt16(nWhich))); }

    virtual MapUnit                 GetMetric( sal_uInt16 nWhich ) const;
    void                            SetDefaultMetric( MapUnit eNewMetric );
    MapUnit GetDefaultMetric() const { return eDefMetric; }

    /** Request string representation of pool items.

        This virtual function produces a string representation
        from the respective SfxItemPool subclass' known SfxPoolItems.

        Subclasses, please override this method, and handle
        SfxPoolItems that don't return useful/complete information on
        SfxPoolItem::GetPresentation()

        This baseclass yields the unmodified string representation of
        rItem.

        @param[in] rItem
        SfxPoolItem to query the string representation of

        @param[in] ePresent
        requested kind of representation - see SfxItemPresentation

        @param[in] eMetric
        requested unit of measure of the representation

        @param[out] rText
        string representation of 'rItem'

        @return true if it has a valid string representation
    */
    virtual bool                    GetPresentation( const SfxPoolItem& rItem,
                                                     MapUnit ePresentationMetric,
                                                     OUString& rText,
                                                     const IntlWrapper& rIntlWrapper ) const;
    virtual rtl::Reference<SfxItemPool> Clone() const;
    const OUString& GetName() const { return aName; }

public:
    // SurrogateData callback helper for iterateItemSurrogates
    class SurrogateData
    {
    public:
        virtual ~SurrogateData() = default;
        SurrogateData(const SurrogateData&) = default;
        SurrogateData() = default;

        // read-access to Item
        virtual const SfxPoolItem& getItem() const = 0;

        // write-access when Item needs to be modified
        virtual const SfxPoolItem* setItem(std::unique_ptr<SfxPoolItem>) = 0;
    };

    // Iterate using a lambda/callback with read/write access to registered SfxPoolItems.
    // If you use this (look for current usages) inside the callback you may
    //   return true; // to continue callback (like 'continue')
    //   return false; // to end callbacks (like 'break')
    void iterateItemSurrogates(
        sal_uInt16 nWhich,
        const std::function<bool(SfxItemPool::SurrogateData& rData)>& rItemCallback) const;

    // Read-only access to registered SfxPoolItems
    // NOTE: In *no* case use const_cast and change those Items (!)
    // Read commit text for more information
    void GetItemSurrogates(ItemSurrogates& rTarget, sal_uInt16 nWhich) const;

    sal_uInt16 GetFirstWhich() const { return mnStart; }
    sal_uInt16 GetLastWhich() const { return mnEnd; }
    bool IsInRange( sal_uInt16 nWhich ) const { return nWhich >= mnStart && nWhich <= mnEnd; }

    void                            SetSecondaryPool( SfxItemPool *pPool );
    SfxItemPool* GetSecondaryPool() const { return mpSecondary.get(); }
    /* get the last pool by following the GetSecondaryPool chain */
    SfxItemPool*                    GetLastPoolInChain();
    SfxItemPool* GetMasterPool() const { return mpMaster; }
    void sendShutdownHint();

    // syntactical sugar: direct call to not have to use the flag define
    // and make the intention clearer
    bool NeedsSurrogateSupport(sal_uInt16 nWhich) const
        { return CheckItemInfoFlag(nWhich, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE); }

    // tries to translate back from SlotID to WhichID.
    // If none is defined, return nSlot.
    // If nSlot is not a SlotID, return nSlot.
    sal_uInt16 GetWhichIDFromSlotID(sal_uInt16 nSlot, bool bDeep = true) const;
    template<class T> TypedWhichId<T> GetWhichIDFromSlotID(TypedWhichId<T> nSlot, bool bDeep = true) const
     { return TypedWhichId<T>(GetWhichIDFromSlotID(sal_uInt16(nSlot), bDeep)); }

    // get SlotID that may be registered in the SfxItemInfo for
    // the given WhichID.
    // If none is defined, return nWhich.
    // If nWhich is not a WhichID, return nWhich.
    sal_uInt16 GetSlotId( sal_uInt16 nWhich ) const;

    // same as GetWhichIDFromSlotID, but returns 0 in error cases, so:
    // If none is defined, return 0.
    // If nSlot is not a SlotID, return 0.
    sal_uInt16 GetTrueWhichIDFromSlotID( sal_uInt16 nSlot, bool bDeep = true ) const;

    // same as GetSlotId, but returns 0 in error cases, so:
    // If none is defined, return 0.
    // If nWhich is not a WhichID, return 0.
    sal_uInt16 GetTrueSlotId( sal_uInt16 nWhich ) const;

    static bool IsWhich(sal_uInt16 nId) { return nId && nId <= SFX_WHICH_MAX; }
    static bool IsSlot(sal_uInt16 nId) { return nId && nId > SFX_WHICH_MAX; }

private:
    const SfxItemPool& operator=(const SfxItemPool &) = delete;

     //IDs below or equal are Which IDs, IDs above slot IDs
    static const sal_uInt16 SFX_WHICH_MAX = 4999;
};

// only the pool may manipulate the reference counts
inline void SfxItemPool::AddRef(const SfxPoolItem& rItem)
{
    rItem.AddRef();
}

// only the pool may manipulate the reference counts
inline sal_uInt32 SfxItemPool::ReleaseRef(const SfxPoolItem& rItem, sal_uInt32 n)
{
    return rItem.ReleaseRef(n);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
