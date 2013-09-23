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

#ifndef _SFXITEMPOOL_HXX
#define _SFXITEMPOOL_HXX

#include <limits.h>
#include <rtl/string.hxx>
#include <svl/poolitem.hxx>
#include <svl/svldllapi.h>
#include <tools/solar.h>
#include <vector>

class SvStream;
class SfxBroadcaster;
struct SfxItemPool_Impl;

#define SFX_WHICH_MAX 4999

DBG_NAMEEX(SfxItemPool)

#define SFX_ITEM_POOLABLE           0x0001
#define SFX_ITEM_NOT_POOLABLE       0x0002

struct SfxItemInfo
{
    sal_uInt16      _nSID;
    sal_uInt16      _nFlags;
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

/*  [Beschreibung]

    Die von dieser Klasse abgeleiteten Klassen dienen der Bereitstellung von
    Defaults von SfxPoolItems und halten konkrete (konstante) Instanzen, die
    dann von mehreren Stellen (i.d.R. eines Dokuments) referenziert werden
    k"onnen.

    Dadurch ist jeder Wert nur einmalig gespeichert, was zu wenig Konstruktor
    und Destruktor-Aufrufen f"ahrt, Vergleiche zwischen Items eines Dokuments
    beschleunigt und ein einfaches Laden und Speichern von Attributen
    bereitstellt.
*/
class SVL_DLLPUBLIC SfxItemPool
{
    friend struct SfxItemPool_Impl;

    const SfxItemInfo*              pItemInfos;
    SfxItemPool_Impl*               pImp;

public:
    void AddSfxItemPoolUser(SfxItemPoolUser& rNewUser);
    void RemoveSfxItemPoolUser(SfxItemPoolUser& rOldUser);

#ifndef _SFXITEMS_HXX
private:
    sal_uInt16                      GetIndex_Impl(sal_uInt16 nWhich) const;
    sal_uInt16                      GetSize_Impl() const;

    SVL_DLLPRIVATE SvStream&        Load1_Impl( SvStream &rStream );
    SVL_DLLPRIVATE bool             IsItemFlag_Impl( sal_uInt16 nWhich, sal_uInt16 nFlag ) const;

public:
    // fuer dflt. SfxItemSet::CTOR, setze dflt. WhichRanges
    void                            FillItemIdRanges_Impl( sal_uInt16*& pWhichRanges ) const;
    const sal_uInt16*               GetFrozenIdRanges() const;
#endif

protected:
    static inline void              SetRefCount( SfxPoolItem& rItem, sal_uLong n );
    static inline sal_uLong         AddRef( const SfxPoolItem& rItem, sal_uLong n = 1 );
    static inline sal_uLong         ReleaseRef( const SfxPoolItem& rItem, sal_uLong n = 1);
    static inline void              SetKind( SfxPoolItem& rItem, sal_uInt16 nRef );

public:
                                    SfxItemPool( const SfxItemPool &rPool,
                                                 sal_Bool bCloneStaticDefaults = sal_False );
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
    void                            ReleaseDefaults( sal_Bool bDelete = sal_False );
    static void                     ReleaseDefaults( SfxPoolItem **pDefaults, sal_uInt16 nCount, sal_Bool bDelete = sal_False );

    virtual SfxMapUnit              GetMetric( sal_uInt16 nWhich ) const;
    void                            SetDefaultMetric( SfxMapUnit eNewMetric );

    /** Request string representation of pool items

        This virtual function produces a string representation, of
        from the respective SfxItemPool subclass' known SfxPoolItems.

        Subclasses, please overwrite this method, and handle
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
    */
    virtual SfxItemPresentation     GetPresentation( const SfxPoolItem& rItem,
                                        SfxItemPresentation ePresentation,
                                        SfxMapUnit          ePresentationMetric,
                                        OUString&           rText,
                                        const IntlWrapper * pIntlWrapper
                                         = 0 ) const;
    virtual SfxItemPool*            Clone() const;
    const OUString&            GetName() const;

    virtual const SfxPoolItem&      Put( const SfxPoolItem&, sal_uInt16 nWhich = 0 );
    virtual void                    Remove( const SfxPoolItem& );
    virtual const SfxPoolItem&      GetDefaultItem( sal_uInt16 nWhich ) const;

    const SfxPoolItem*              LoadItem( SvStream &rStream,
                                              bool bDirect = false,
                                              const SfxItemPool *pRefPool = 0 );
    bool                            StoreItem( SvStream &rStream,
                                               const SfxPoolItem &rItem,
                                               bool bDirect = false ) const;

    sal_uInt32                      GetSurrogate(const SfxPoolItem *) const;
    const SfxPoolItem *             GetItem2(sal_uInt16 nWhich, sal_uInt32 nSurrogate) const;
    sal_uInt32                      GetItemCount2(sal_uInt16 nWhich) const;
    const SfxPoolItem*              LoadSurrogate(SvStream& rStream,
                                            sal_uInt16 &rWhich, sal_uInt16 nSlotId,
                                            const SfxItemPool* pRefPool = 0 );
    bool                            StoreSurrogate(SvStream& rStream,
                                            const SfxPoolItem *pItem ) const;

    virtual SvStream &              Load(SvStream &);
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

    bool                            IsItemFlag( sal_uInt16 nWhich, sal_uInt16 nFlag ) const;
    bool                            IsItemFlag( const SfxPoolItem &rItem, sal_uInt16 nFlag ) const
                                    { return IsItemFlag( rItem.Which(), nFlag ); }
    void                            SetItemInfos( const SfxItemInfo *pInfos );
    sal_uInt16                      GetWhich( sal_uInt16 nSlot, sal_Bool bDeep = sal_True ) const;
    sal_uInt16                      GetSlotId( sal_uInt16 nWhich, sal_Bool bDeep = sal_True ) const;
    sal_uInt16                      GetTrueWhich( sal_uInt16 nSlot, sal_Bool bDeep = sal_True ) const;
    sal_uInt16                      GetTrueSlotId( sal_uInt16 nWhich, sal_Bool bDeep = sal_True ) const;

    void                            SetVersionMap( sal_uInt16 nVer,
                                                   sal_uInt16 nOldStart, sal_uInt16 nOldEnd,
                                                   const sal_uInt16 *pWhichIdTab );
    sal_uInt16                      GetNewWhich( sal_uInt16 nOldWhich ) const;
    sal_uInt16                      GetVersion() const;
    void                            SetFileFormatVersion( sal_uInt16 nFileFormatVersion );
    bool                            IsCurrentVersionLoading() const;

    static int                      IsWhich(sal_uInt16 nId) {
                                        return nId && nId <= SFX_WHICH_MAX; }
    static int                      IsSlot(sal_uInt16 nId) {
                                        return nId && nId > SFX_WHICH_MAX; }

    static const SfxItemPool*       GetStoringPool();

private:
    const SfxItemPool&              operator=(const SfxItemPool &);   // n.i.!!

    static const SfxItemPool*       pStoringPool_;
};

// nur der Pool darf den Referenz-Zaehler manipulieren !!!
inline void SfxItemPool::SetRefCount( SfxPoolItem& rItem, sal_uLong n )
{
    rItem.SetRefCount(n);
}

// nur der Pool darf den Referenz-Zaehler manipulieren !!!
inline sal_uLong SfxItemPool::AddRef( const SfxPoolItem& rItem, sal_uLong n )
{
    return rItem.AddRef(n);
}

// nur der Pool darf den Referenz-Zaehler manipulieren !!!
inline sal_uLong SfxItemPool::ReleaseRef( const SfxPoolItem& rItem, sal_uLong n )
{
    return rItem.ReleaseRef(n);
}

inline void SfxItemPool::SetKind( SfxPoolItem& rItem, sal_uInt16 nRef )
{
    rItem.SetKind( nRef );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
