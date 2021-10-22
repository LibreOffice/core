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
#include <cstddef>
#include <memory>
#include <utility>

#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>
#include <svl/typedwhich.hxx>
#include <svl/whichranges.hxx>

class SfxItemPool;

class SAL_WARN_UNUSED SVL_DLLPUBLIC SfxItemSet
{
    friend class SfxItemIter;

    SfxItemPool*      m_pPool;         ///< pool that stores the items
    const SfxItemSet* m_pParent;       ///< derivation
    SfxPoolItem const** m_ppItems;     ///< pointer to array of items, we allocate and free this unless m_bItemsFixed==true
    WhichRangesContainer m_pWhichRanges;  ///< array of Which Ranges
    sal_uInt16        m_nCount;        ///< number of items
    bool              m_bItemsFixed; ///< true if this is a SfxItemSetFixed object

friend class SfxItemPoolCache;
friend class SfxAllItemSet;

private:
    SVL_DLLPRIVATE void       RecreateRanges_Impl(const WhichRangesContainer& pNewRanges);

    SfxItemSet( SfxItemPool & pool, const WhichRangesContainer& wids, std::size_t items );

public:
    SfxPoolItem const**         GetItems_Impl() const { return m_ppItems; }

private:
    const SfxItemSet&           operator=(const SfxItemSet &) = delete;

protected:
    // Notification-Callback
    virtual void                Changed( const SfxPoolItem& rOld, const SfxPoolItem& rNew );

    void                        PutDirect(const SfxPoolItem &rItem);

    virtual const SfxPoolItem*  PutImpl( const SfxPoolItem&, sal_uInt16 nWhich, bool bPassingOwnership );

    /** special constructor for SfxAllItemSet */
    enum class SfxAllItemSetFlag { Flag };
    SfxItemSet( SfxItemPool&, SfxAllItemSetFlag );
    /** special constructor for SfxItemSetFixed */
    SfxItemSet( SfxItemPool&, WhichRangesContainer&& ranges, SfxPoolItem const ** ppItems );

public:
    SfxItemSet( const SfxItemSet& );
    SfxItemSet( SfxItemSet&& ) noexcept;
    SfxItemSet( SfxItemPool& );
    SfxItemSet( SfxItemPool&, const WhichRangesContainer& ranges );
    SfxItemSet( SfxItemPool&, WhichRangesContainer&& ranges );

    SfxItemSet( SfxItemPool& rPool, sal_uInt16 nWhichStart, sal_uInt16 nWhichEnd )
        : SfxItemSet(rPool, WhichRangesContainer(nWhichStart, nWhichEnd)) {}

    template<sal_uInt16... WIDs>
    SfxItemSet(SfxItemPool& pool, svl::Items_t<WIDs...> wids)
        : SfxItemSet(pool, WhichRangesContainer(wids)) {}

    virtual ~SfxItemSet();

    virtual std::unique_ptr<SfxItemSet> Clone(bool bItems = true, SfxItemPool *pToPool = nullptr) const;
    virtual SfxItemSet CloneAsValue(bool bItems = true, SfxItemPool *pToPool = nullptr) const;

    // Get number of items
    sal_uInt16                  Count() const { return m_nCount; }
    sal_uInt16                  TotalCount() const;

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

    sal_uInt16                  GetWhichByPos(sal_uInt16 nPos) const;

    SfxItemState                GetItemState(   sal_uInt16 nWhich,
                                                bool bSrchInParent = true,
                                                const SfxPoolItem **ppItem = nullptr ) const;

    bool                        HasItem(sal_uInt16 nWhich, const SfxPoolItem** ppItem = nullptr) const;

    void                        DisableItem(sal_uInt16 nWhich);
    void                        InvalidateItem( sal_uInt16 nWhich );
    sal_uInt16                  ClearItem( sal_uInt16 nWhich = 0);
    void                        ClearInvalidItems();
    void                        InvalidateAllItems(); // HACK(via nWhich = 0) ???

    inline void                 SetParent( const SfxItemSet* pNew );

    // add, delete items, work on items
public:
    const SfxPoolItem*          Put( const SfxPoolItem& rItem, sal_uInt16 nWhich )
    { return PutImpl(rItem, nWhich, /*bPassingOwnership*/false); }
    const SfxPoolItem*          Put( std::unique_ptr<SfxPoolItem> xItem, sal_uInt16 nWhich )
    { return PutImpl(*xItem.release(), nWhich, /*bPassingOwnership*/true); }
    const SfxPoolItem*          Put( const SfxPoolItem& rItem )
                                { return Put(rItem, rItem.Which()); }
    const SfxPoolItem*          Put( std::unique_ptr<SfxPoolItem> xItem )
                                { sal_uInt16 nWhich = xItem->Which(); return Put(std::move(xItem), nWhich); }
    bool                        Put( const SfxItemSet&,
                                     bool bInvalidAsDefault = true );
    void                        PutExtended( const SfxItemSet&,
                                             SfxItemState eDontCareAs,
                                             SfxItemState eDefaultAs );

    bool                        Set( const SfxItemSet&, bool bDeep = true );

    void                        Intersect( const SfxItemSet& rSet );
    void                        MergeValues( const SfxItemSet& rSet );
    void                        Differentiate( const SfxItemSet& rSet );
    void                        MergeValue( const SfxPoolItem& rItem, bool bOverwriteDefaults = false  );

    SfxItemPool*                GetPool() const { return m_pPool; }
    const WhichRangesContainer & GetRanges() const { return m_pWhichRanges; }
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
};

inline void SfxItemSet::SetParent( const SfxItemSet* pNew )
{
    m_pParent = pNew;
}

class SVL_DLLPUBLIC SfxAllItemSet final : public SfxItemSet

//  Handles all Ranges. Ranges are automatically modified by putting items.

{
public:
                                SfxAllItemSet( SfxItemPool &rPool );
                                SfxAllItemSet( const SfxItemSet & );
                                SfxAllItemSet( const SfxAllItemSet & );

    virtual std::unique_ptr<SfxItemSet> Clone( bool bItems = true, SfxItemPool *pToPool = nullptr ) const override;
    virtual SfxItemSet CloneAsValue( bool bItems = true, SfxItemPool *pToPool = nullptr ) const override;
private:
    virtual const SfxPoolItem*  PutImpl( const SfxPoolItem&, sal_uInt16 nWhich, bool bPassingOwnership ) override;
};


namespace svl::detail
{
/**
 * Determines the number of sal_uInt16s in a container of pairs of
 * sal_uInt16s, each representing a range of sal_uInt16s, and total capacity of the ranges.
 */
template <sal_uInt16 WID1, sal_uInt16 WID2, sal_uInt16... Rest>
static constexpr sal_uInt16 CountRanges1()
{
    sal_uInt16 nCapacity = rangeSize(WID1, WID2);
    if constexpr (sizeof...(Rest) > 0)
        nCapacity += CountRanges1<Rest...>();
    return nCapacity;
}}

// Allocate the items array inside the object, to reduce allocation cost.
//
template<sal_uInt16... WIDs>
class SfxItemSetFixed : public SfxItemSet
{
public:
    SfxItemSetFixed( SfxItemPool& rPool)
        : SfxItemSet(rPool, WhichRangesContainer(svl::Items_t<WIDs...>{}), m_aItems) {}
private:
    static constexpr sal_uInt16 NITEMS = svl::detail::CountRanges1<WIDs...>();
    const SfxPoolItem* m_aItems[NITEMS] = {};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
