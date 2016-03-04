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
#ifndef INCLUDED_SVL_ITEMSET_HXX
#define INCLUDED_SVL_ITEMSET_HXX

#include <svl/svldllapi.h>

#include <cstdarg>
#include <svl/poolitem.hxx>

class SfxItemPool;
class SfxPoolItem;
class SvStream;

typedef SfxPoolItem const** SfxItemArray;

class SVL_DLLPUBLIC SfxItemSet
{
    friend class SfxItemIter;

    SfxItemPool*      m_pPool;         ///< pool that stores the items
    const SfxItemSet* m_pParent;       ///< derivation
    SfxItemArray      m_pItems;        ///< array of items
    sal_uInt16*       m_pWhichRanges;  ///< array of Which Ranges
    sal_uInt16        m_nCount;        ///< number of items

friend class SfxItemPoolCache;
friend class SfxAllItemSet;
friend const char *DbgCheckItemSet( const void* );

private:
    SVL_DLLPRIVATE void                     InitRanges_Impl(const sal_uInt16 *nWhichPairTable);
    SVL_DLLPRIVATE void                     InitRanges_Impl(va_list pWhich, sal_uInt16 n1, sal_uInt16 n2, sal_uInt16 n3);
    SVL_DLLPRIVATE void                     InitRanges_Impl(sal_uInt16 nWh1, sal_uInt16 nWh2);

public:
    SfxItemArray                GetItems_Impl() const { return m_pItems; }

private:
    const SfxItemSet&           operator=(const SfxItemSet &) = delete;

protected:
    // Notification-Callback
    virtual void                Changed( const SfxPoolItem& rOld, const SfxPoolItem& rNew );

    void                        PutDirect(const SfxPoolItem &rItem);

public:
                                SfxItemSet( const SfxItemSet& );

                                SfxItemSet( SfxItemPool&, bool bTotalPoolRanges = false );
                                SfxItemSet( SfxItemPool&, sal_uInt16 nWhich1, sal_uInt16 nWhich2 );
                                SfxItemSet( SfxItemPool&, int nWh1, int nWh2, int nNull, ... );
                                SfxItemSet( SfxItemPool&, const sal_uInt16* nWhichPairTable );
    virtual                     ~SfxItemSet();

    virtual SfxItemSet *        Clone(bool bItems = true, SfxItemPool *pToPool = nullptr) const;

    // Get number of items
    sal_uInt16                  Count() const { return m_nCount; }
    sal_uInt16                  TotalCount() const;

    const SfxPoolItem&          Get( sal_uInt16 nWhich, bool bSrchInParent = true ) const;

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

    /// Templatized static version of GetItem() to directly return the correct type if the SfxItemSet is available.
    template<class T> static const T* GetItem(const SfxItemSet* pItemSet, sal_uInt16 nWhich, bool bSearchInParent = true)
    {
        if (pItemSet)
            return pItemSet->GetItem<T>(nWhich, bSearchInParent);

        return nullptr;
    }

    // Get Which-value of the item at position nPos
    sal_uInt16                  GetWhichByPos(sal_uInt16 nPos) const;

    // Get item-status
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
    virtual const SfxPoolItem*  Put( const SfxPoolItem&, sal_uInt16 nWhich );
    const SfxPoolItem*          Put( const SfxPoolItem& rItem )
                                { return Put(rItem, rItem.Which()); }
    bool                        Put( const SfxItemSet&,
                                     bool bInvalidAsDefault = true );
    void                        PutExtended( const SfxItemSet&,
                                             SfxItemState eDontCareAs = SfxItemState::UNKNOWN,
                                             SfxItemState eDefaultAs = SfxItemState::UNKNOWN );

    bool                        Set( const SfxItemSet&, bool bDeep = true );

    void                        Intersect( const SfxItemSet& rSet );
    void                        MergeValues( const SfxItemSet& rSet );
    void                        Differentiate( const SfxItemSet& rSet );
    void                        MergeValue( const SfxPoolItem& rItem, bool bOverwriteDefaults = false  );

    SfxItemPool*                GetPool() const { return m_pPool; }
    const sal_uInt16*           GetRanges() const { return m_pWhichRanges; }
    void                        SetRanges( const sal_uInt16 *pRanges );
    void                        MergeRange( sal_uInt16 nFrom, sal_uInt16 nTo );
    const SfxItemSet*           GetParent() const { return m_pParent; }

    void                        Load( SvStream & );
    void                        Store( SvStream &, bool bDirect = false ) const;

    bool                        operator==(const SfxItemSet &) const;
    sal_Int32                   getHash() const;
    OString                     stringify() const;
    void dumpAsXml(struct _xmlTextWriter* pWriter) const;
};

inline void SfxItemSet::SetParent( const SfxItemSet* pNew )
{
    m_pParent = pNew;
}

class SVL_DLLPUBLIC SfxAllItemSet: public SfxItemSet

//  Handles all Ranges. Ranges are automatically modified by putting items.

{
    sal_uInt16                      nFree;

public:
                                SfxAllItemSet( SfxItemPool &rPool );
                                SfxAllItemSet( const SfxItemSet & );
                                SfxAllItemSet( const SfxAllItemSet & );

    virtual SfxItemSet *        Clone( bool bItems = true, SfxItemPool *pToPool = nullptr ) const override;
    virtual const SfxPoolItem*  Put( const SfxPoolItem&, sal_uInt16 nWhich ) override;
    using SfxItemSet::Put;
};

#endif // INCLUDED_SVL_ITEMSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
