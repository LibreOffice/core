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

#ifndef INCLUDED_SVL_ITEMPOOL_HXX
#define INCLUDED_SVL_ITEMPOOL_HXX

#include <svl/poolitem.hxx>
#include <svl/svldllapi.h>
#include <svl/typedwhich.hxx>
#include <memory>
#include <vector>
#include <o3tl/sorted_vector.hxx>

class SfxBroadcaster;
struct SfxItemPool_Impl;

struct SfxItemInfo
{
    sal_uInt16       _nSID;
    bool            _bPoolable;
};

class SfxItemPool;

class SVL_DLLPUBLIC SfxItemPoolUser
{
public:
    virtual void ObjectInDestruction(const SfxItemPool& rSfxItemPool) = 0;

protected:
    ~SfxItemPoolUser() {}
};

/** Base class for providers of defaults of SfxPoolItems.
 *
 * The derived classes hold the concrete (const) instances which are referenced in several places
 * (usually within a single document).
 * This helps to lower the amount of calls to lifecycle methods, speeds up comparisons within a document
 * and facilitates loading and saving of attributes.
 */
class SVL_DLLPUBLIC SfxItemPool
{
    friend struct SfxItemPool_Impl;
    friend class SfxItemSet;
    friend class SfxAllItemSet;

    const SfxItemInfo*              pItemInfos;
    std::unique_ptr<SfxItemPool_Impl>               pImpl;

public:
    void AddSfxItemPoolUser(SfxItemPoolUser& rNewUser);
    void RemoveSfxItemPoolUser(SfxItemPoolUser& rOldUser);

private:
    sal_uInt16                      GetIndex_Impl(sal_uInt16 nWhich) const;
    sal_uInt16                      GetSize_Impl() const;

    SVL_DLLPRIVATE bool             IsItemPoolable_Impl( sal_uInt16 nWhich ) const;

public:
    // for default SfxItemSet::CTOR, set default WhichRanges
    void                            FillItemIdRanges_Impl( std::unique_ptr<sal_uInt16[]>& pWhichRanges ) const;
    const sal_uInt16*               GetFrozenIdRanges() const;

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

protected:
    virtual                         ~SfxItemPool();

public:
    static void Free(SfxItemPool* pPool);

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
    virtual SfxItemPool*            Clone() const;
    const OUString&                 GetName() const;

    template<class T> const T&      Put( std::unique_ptr<T> xItem, sal_uInt16 nWhich = 0 )
    { return static_cast<const T&>(PutImpl( *xItem.release(), nWhich, /*bPassingOwnership*/true)); }
    template<class T> const T&      Put( const T& rItem, sal_uInt16 nWhich = 0 )
    { return static_cast<const T&>(PutImpl( rItem, nWhich, /*bPassingOwnership*/false)); }
    void                            Remove( const SfxPoolItem& );

    const SfxPoolItem&              GetDefaultItem( sal_uInt16 nWhich ) const;
    template<class T> const T&      GetDefaultItem( TypedWhichId<T> nWhich ) const
    { return static_cast<const T&>(GetDefaultItem(sal_uInt16(nWhich))); }

    bool                            CheckItemInPool(const SfxPoolItem *) const;

    struct Item2Range
    {
        o3tl::sorted_vector<SfxPoolItem*>::const_iterator m_begin;
        o3tl::sorted_vector<SfxPoolItem*>::const_iterator m_end;
        o3tl::sorted_vector<SfxPoolItem*>::const_iterator const & begin() const { return m_begin; }
        o3tl::sorted_vector<SfxPoolItem*>::const_iterator const & end() const { return m_end; }
    };
    const SfxPoolItem *             GetItem2Default(sal_uInt16 nWhich) const;
    template<class T> const T*      GetItem2Default( TypedWhichId<T> nWhich ) const
    { return static_cast<const T*>(GetItem2Default(sal_uInt16(nWhich))); }

    sal_uInt32                      GetItemCount2(sal_uInt16 nWhich) const;
    Item2Range                      GetItemSurrogates(sal_uInt16 nWhich) const;
    /*
        This is only valid for SfxPoolItem that override IsSortable and operator<.
        Returns a range of items defined by using operator<.
        @param rNeedle must be the same type or a supertype of the pool items for nWhich.
    */
    std::vector<const SfxPoolItem*> FindItemSurrogate(sal_uInt16 nWhich, SfxPoolItem const & rNeedle) const;

    sal_uInt16                      GetFirstWhich() const;
    sal_uInt16                      GetLastWhich() const;
    bool                            IsInRange( sal_uInt16 nWhich ) const;
    void                            SetSecondaryPool( SfxItemPool *pPool );
    SfxItemPool*                    GetSecondaryPool() const;
    SfxItemPool*                    GetMasterPool() const;
    void                            FreezeIdRanges();

    void                            Delete();

    bool                            IsItemPoolable( sal_uInt16 nWhich ) const;
    bool                            IsItemPoolable( const SfxPoolItem &rItem ) const
                                    { return IsItemPoolable( rItem.Which() ); }
    void                            SetItemInfos( const SfxItemInfo *pInfos );
    sal_uInt16                      GetWhich( sal_uInt16 nSlot, bool bDeep = true ) const;
    sal_uInt16                      GetSlotId( sal_uInt16 nWhich ) const;
    sal_uInt16                      GetTrueWhich( sal_uInt16 nSlot, bool bDeep = true ) const;
    sal_uInt16                      GetTrueSlotId( sal_uInt16 nWhich ) const;

    static bool                     IsWhich(sal_uInt16 nId) {
                                        return nId && nId <= SFX_WHICH_MAX; }
    static bool                     IsSlot(sal_uInt16 nId) {
                                        return nId && nId > SFX_WHICH_MAX; }

    void                            dumpAsXml(xmlTextWriterPtr pWriter) const;

protected:
    virtual const SfxPoolItem&      PutImpl( const SfxPoolItem&, sal_uInt16 nWhich = 0, bool bPassingOwnership = false );
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
