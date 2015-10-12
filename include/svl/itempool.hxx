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

#include <rtl/string.hxx>
#include <svl/poolitem.hxx>
#include <svl/svldllapi.h>
#include <tools/solar.h>
#include <o3tl/typed_flags_set.hxx>

class SvStream;
class SfxBroadcaster;
struct SfxItemPool_Impl;

#define SFX_WHICH_MAX 4999

enum class SfxItemPoolFlags
{
    NONE               = 0x00,
    POOLABLE           = 0x01,
    NOT_POOLABLE       = 0x02,
};
namespace o3tl
{
    template<> struct typed_flags<SfxItemPoolFlags> : is_typed_flags<SfxItemPoolFlags, 0x03> {};
}

struct SfxItemInfo
{
    sal_uInt16       _nSID;
    SfxItemPoolFlags _nFlags;
};

class SfxStyleSheetIterator;
struct SfxPoolItemArray_Impl;
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

    const SfxItemInfo*              pItemInfos;
    SfxItemPool_Impl*               pImp;

public:
    void AddSfxItemPoolUser(SfxItemPoolUser& rNewUser);
    void RemoveSfxItemPoolUser(SfxItemPoolUser& rOldUser);

private:
    sal_uInt16                      GetIndex_Impl(sal_uInt16 nWhich) const;
    sal_uInt16                      GetSize_Impl() const;

    SVL_DLLPRIVATE bool             IsItemFlag_Impl( sal_uInt16 nWhich, SfxItemPoolFlags nFlag ) const;

public:
    // for default SfxItemSet::CTOR, set default WhichRanges
    void                            FillItemIdRanges_Impl( sal_uInt16*& pWhichRanges ) const;
    const sal_uInt16*               GetFrozenIdRanges() const;

protected:
    static inline void              SetRefCount( SfxPoolItem& rItem, sal_uLong n );
    static inline void              AddRef( const SfxPoolItem& rItem, sal_uLong n = 1 );
    static inline sal_uLong         ReleaseRef( const SfxPoolItem& rItem, sal_uLong n = 1);
    static inline void              SetKind( SfxPoolItem& rItem, SfxItemKind nRef );

public:
                                    SfxItemPool( const SfxItemPool &rPool,
                                                 bool bCloneStaticDefaults = false );
                                    SfxItemPool( const OUString &rName,
                                                 sal_uInt16 nStart, sal_uInt16 nEnd,
                                                 const SfxItemInfo *pItemInfos,
                                                 SfxPoolItem **pDefaults = 0,
                                                 bool bLoadRefCounts = true );

protected:
    virtual                         ~SfxItemPool();

public:
    static void Free(SfxItemPool* pPool);

    SfxBroadcaster&                 BC();

    void                            SetPoolDefaultItem( const SfxPoolItem& );
    const SfxPoolItem*              GetPoolDefaultItem( sal_uInt16 nWhich ) const;
    void                            ResetPoolDefaultItem( sal_uInt16 nWhich );

    void                            SetDefaults( SfxPoolItem **pDefaults );
    void                            ReleaseDefaults( bool bDelete = false );
    static void                     ReleaseDefaults( SfxPoolItem **pDefaults, sal_uInt16 nCount, bool bDelete = false );

    virtual SfxMapUnit              GetMetric( sal_uInt16 nWhich ) const;
    void                            SetDefaultMetric( SfxMapUnit eNewMetric );

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
                                        SfxMapUnit          ePresentationMetric,
                                        OUString&           rText,
                                        const IntlWrapper * pIntlWrapper = 0 ) const;
    virtual SfxItemPool*            Clone() const;
    const OUString&                 GetName() const;

    virtual const SfxPoolItem&      Put( const SfxPoolItem&, sal_uInt16 nWhich = 0 );
    virtual void                    Remove( const SfxPoolItem& );
    const SfxPoolItem&              GetDefaultItem( sal_uInt16 nWhich ) const;

    const SfxPoolItem*              LoadItem( SvStream &rStream,
                                              bool bDirect = false,
                                              const SfxItemPool *pRefPool = 0 );
    bool                            StoreItem( SvStream &rStream,
                                               const SfxPoolItem &rItem,
                                               bool bDirect = false ) const;

    sal_uInt32                      GetSurrogate(const SfxPoolItem *) const;
    const SfxPoolItem *             GetItem2(sal_uInt16 nWhich, sal_uInt32 nSurrogate) const;
    const SfxPoolItem *             GetItem2Default(sal_uInt16 nWhich) const;
    sal_uInt32                      GetItemCount2(sal_uInt16 nWhich) const;
    const SfxPoolItem*              LoadSurrogate(SvStream& rStream,
                                            sal_uInt16 &rWhich, sal_uInt16 nSlotId,
                                            const SfxItemPool* pRefPool = 0 );
    bool                            StoreSurrogate(SvStream& rStream,
                                            const SfxPoolItem *pItem ) const;

    SvStream &                      Load(SvStream &);
    virtual SvStream &              Store(SvStream &) const;
    bool                            HasPersistentRefCounts() const;
    void                            LoadCompleted();

    sal_uInt16                      GetFirstWhich() const;
    sal_uInt16                      GetLastWhich() const;
    bool                            IsInRange( sal_uInt16 nWhich ) const;
    bool                            IsInVersionsRange( sal_uInt16 nWhich ) const;
    bool                            IsInStoringRange( sal_uInt16 nWhich ) const;
    void                            SetStoringRange( sal_uInt16 nFrom, sal_uInt16 nTo );
    void                            SetSecondaryPool( SfxItemPool *pPool );
    SfxItemPool*                    GetSecondaryPool() const;
    SfxItemPool*                    GetMasterPool() const;
    void                            FreezeIdRanges();

    void                            Delete();

    bool                            IsItemFlag( sal_uInt16 nWhich, SfxItemPoolFlags nFlag ) const;
    bool                            IsItemFlag( const SfxPoolItem &rItem, SfxItemPoolFlags nFlag ) const
                                    { return IsItemFlag( rItem.Which(), nFlag ); }
    void                            SetItemInfos( const SfxItemInfo *pInfos );
    sal_uInt16                      GetWhich( sal_uInt16 nSlot, bool bDeep = true ) const;
    sal_uInt16                      GetSlotId( sal_uInt16 nWhich, bool bDeep = true ) const;
    sal_uInt16                      GetTrueWhich( sal_uInt16 nSlot, bool bDeep = true ) const;
    sal_uInt16                      GetTrueSlotId( sal_uInt16 nWhich, bool bDeep = true ) const;

    void                            SetVersionMap( sal_uInt16 nVer,
                                                   sal_uInt16 nOldStart, sal_uInt16 nOldEnd,
                                                   const sal_uInt16 *pWhichIdTab );
    sal_uInt16                      GetNewWhich( sal_uInt16 nOldWhich ) const;
    void                            SetFileFormatVersion( sal_uInt16 nFileFormatVersion );
    bool                            IsCurrentVersionLoading() const;

    static bool                     IsWhich(sal_uInt16 nId) {
                                        return nId && nId <= SFX_WHICH_MAX; }
    static bool                     IsSlot(sal_uInt16 nId) {
                                        return nId && nId > SFX_WHICH_MAX; }

    static const SfxItemPool*       GetStoringPool();

private:
    const SfxItemPool&              operator=(const SfxItemPool &) = delete;

    static const SfxItemPool*       pStoringPool_;
};

// only the pool may manipulate the reference counts
inline void SfxItemPool::SetRefCount( SfxPoolItem& rItem, sal_uLong n )
{
    rItem.SetRefCount(n);
}

// only the pool may manipulate the reference counts
inline void SfxItemPool::AddRef( const SfxPoolItem& rItem, sal_uLong n )
{
    rItem.AddRef(n);
}

// only the pool may manipulate the reference counts
inline sal_uLong SfxItemPool::ReleaseRef( const SfxPoolItem& rItem, sal_uLong n )
{
    return rItem.ReleaseRef(n);
}

inline void SfxItemPool::SetKind( SfxPoolItem& rItem, SfxItemKind nRef )
{
    rItem.SetKind( nRef );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
