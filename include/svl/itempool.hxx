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

#include <rtl/ref.hxx>
#include <svl/poolitem.hxx>
#include <svl/svldllapi.h>
#include <svl/typedwhich.hxx>
#include <svl/whichranges.hxx>
#include <memory>
#include <vector>
#include <unordered_set>
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

struct SfxItemInfo
{
    // Defines a mapping between WhichID <-> SlotID
    sal_uInt16  _nItemInfoSlotID;

    // Pool-dependent Item-Attributes, please use
    // SFX_ITEMINFOFLAG_* to create/set. Now using a
    // sal_uInt16 and not separate bools so changes
    // will be easier
    sal_uInt16  _nItemInfoFlags;
};

typedef std::unordered_set<SfxItemSet*> registeredSfxItemSets;
class SfxPoolItemHolder;
typedef std::unordered_set<SfxPoolItemHolder*> registeredSfxPoolItemHolders;
typedef std::vector<const SfxPoolItem*> ItemSurrogates;

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

    // unit testing
    friend class PoolItemTest;

    const SfxItemInfo*              pItemInfos;
    SfxBroadcaster                  aBC;
    OUString                        aName;
    std::vector<SfxPoolItem*>       maPoolDefaults;
    std::vector<SfxPoolItem*>*      mpStaticDefaults;
    SfxItemPool*                    mpMaster;
    rtl::Reference<SfxItemPool>     mpSecondary;
    WhichRangesContainer            mpPoolRanges;
    sal_uInt16                      mnStart;
    sal_uInt16                      mnEnd;
    MapUnit                         eDefMetric;

    registeredSfxItemSets maRegisteredSfxItemSets;
    registeredSfxPoolItemHolders maRegisteredSfxPoolItemHolders;
    bool mbPreDeleteDone;

private:
    sal_uInt16                      GetIndex_Impl(sal_uInt16 nWhich) const;
    sal_uInt16                      GetSize_Impl() const;

    // moved to private: use the access methods, e.g. NeedsSurrogateSupport
    SVL_DLLPRIVATE bool CheckItemInfoFlag(sal_uInt16 nWhich, sal_uInt16 nMask) const;
    SVL_DLLPRIVATE bool CheckItemInfoFlag_Impl(sal_uInt16 nPos, sal_uInt16 nMask) const
        { return pItemInfos[nPos]._nItemInfoFlags & nMask; }

    void registerItemSet(SfxItemSet& rSet);
    void unregisterItemSet(SfxItemSet& rSet);

    void registerPoolItemHolder(SfxPoolItemHolder& rHolder);
    void unregisterPoolItemHolder(SfxPoolItemHolder& rHolder);

public:
    // for default SfxItemSet::CTOR, set default WhichRanges
    void                            FillItemIdRanges_Impl( WhichRangesContainer& pWhichRanges ) const;
    const WhichRangesContainer &    GetFrozenIdRanges() const;

protected:
    static inline void              ClearRefCount(SfxPoolItem& rItem);
    static inline void              AddRef(const SfxPoolItem& rItem);
    static inline sal_uInt32        ReleaseRef(const SfxPoolItem& rItem, sal_uInt32 n = 1);

public:
                                    SfxItemPool( const SfxItemPool &rPool,
                                                 bool bCloneStaticDefaults = false );
                                    SfxItemPool( const OUString &rName,
                                                 sal_uInt16 nStart, sal_uInt16 nEnd,
                                                 const SfxItemInfo *pItemInfos,
                                                 std::vector<SfxPoolItem*> *pDefaults = nullptr );

    virtual                         ~SfxItemPool();

    SfxBroadcaster&                 BC();

    void                            SetPoolDefaultItem( const SfxPoolItem& );

    const SfxPoolItem*              GetPoolDefaultItem( sal_uInt16 nWhich ) const;
    template<class T> const T*      GetPoolDefaultItem( TypedWhichId<T> nWhich ) const
    { return static_cast<const T*>(GetPoolDefaultItem(sal_uInt16(nWhich))); }

    void                            ResetPoolDefaultItem( sal_uInt16 nWhich );

    void                            SetDefaults(std::vector<SfxPoolItem*>* pDefaults);
    void                            ClearDefaults();
    void                            ReleaseDefaults( bool bDelete = false );
    static void                     ReleaseDefaults( std::vector<SfxPoolItem*> *pDefaults, bool bDelete = false );

    virtual MapUnit                 GetMetric( sal_uInt16 nWhich ) const;
    void                            SetDefaultMetric( MapUnit eNewMetric );
    MapUnit                         GetDefaultMetric() const;

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
    const OUString&                 GetName() const;

    const SfxPoolItem&              GetDefaultItem( sal_uInt16 nWhich ) const;
    template<class T> const T&      GetDefaultItem( TypedWhichId<T> nWhich ) const
    { return static_cast<const T&>(GetDefaultItem(sal_uInt16(nWhich))); }
    const SfxPoolItem *             GetItem2Default(sal_uInt16 nWhich) const;
    template<class T> const T*      GetItem2Default( TypedWhichId<T> nWhich ) const
    { return static_cast<const T*>(GetItem2Default(sal_uInt16(nWhich))); }

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

    sal_uInt16                      GetFirstWhich() const;
    sal_uInt16                      GetLastWhich() const;
    bool                            IsInRange( sal_uInt16 nWhich ) const;
    void                            SetSecondaryPool( SfxItemPool *pPool );
    SfxItemPool*                    GetSecondaryPool() const;
    /* get the last pool by following the GetSecondaryPool chain */
    SfxItemPool*                    GetLastPoolInChain();
    SfxItemPool*                    GetMasterPool() const;
    void                            FreezeIdRanges();

    void                            Delete();

    // syntactical sugar: direct call to not have to use the flag define
    // and make the intention clearer
    bool NeedsSurrogateSupport(sal_uInt16 nWhich) const
        { return CheckItemInfoFlag(nWhich, SFX_ITEMINFOFLAG_SUPPORT_SURROGATE); }

    void                            SetItemInfos( const SfxItemInfo *pInfos );
    sal_uInt16                      GetWhich( sal_uInt16 nSlot, bool bDeep = true ) const;
    template<class T>
    TypedWhichId<T>                 GetWhich( TypedWhichId<T> nSlot, bool bDeep = true ) const
    { return TypedWhichId<T>(GetWhich(sal_uInt16(nSlot), bDeep)); }
    sal_uInt16                      GetSlotId( sal_uInt16 nWhich ) const;
    sal_uInt16                      GetTrueWhich( sal_uInt16 nSlot, bool bDeep = true ) const;
    sal_uInt16                      GetTrueSlotId( sal_uInt16 nWhich ) const;

    static bool                     IsWhich(sal_uInt16 nId) {
                                        return nId && nId <= SFX_WHICH_MAX; }
    static bool                     IsSlot(sal_uInt16 nId) {
                                        return nId && nId > SFX_WHICH_MAX; }

private:
    const SfxItemPool&              operator=(const SfxItemPool &) = delete;

     //IDs below or equal are Which IDs, IDs above slot IDs
    static const sal_uInt16         SFX_WHICH_MAX = 4999;
};

// only the pool may manipulate the reference counts
inline void SfxItemPool::ClearRefCount(SfxPoolItem& rItem)
{
    rItem.SetRefCount(0);
}

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
