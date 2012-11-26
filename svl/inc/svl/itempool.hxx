/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SFXITEMPOOL_HXX
#define _SFXITEMPOOL_HXX

#include "svl/svldllapi.h"

#ifndef INCLUDED_LIMITS_H
#include <limits.h>
#define INCLUDED_LIMITS_H
#endif
#include <tools/solar.h>
#include <tools/string.hxx>
#include <svl/svarray.hxx>
#include <svl/poolitem.hxx>
#include <vector>

class SvStream;
class SfxBroadcaster;
struct SfxItemPool_Impl;

#define SFX_WHICH_MAX 4999

DBG_NAMEEX(SfxItemPool)

//====================================================================

#define SFX_ITEM_POOLABLE           0x0001
#define SFX_ITEM_NOT_POOLABLE       0x0002

#define SFX_ITEM_USERFLAG0          0x0100
#define SFX_ITEM_USERFLAG1          0x0200
#define SFX_ITEM_USERFLAG2          0x0400
#define SFX_ITEM_USERFLAG3          0x0800
#define SFX_ITEM_USERFLAG4          0x1000
#define SFX_ITEM_USERFLAG5          0x2000
#define SFX_ITEM_USERFLAG6          0x4000
#define SFX_ITEM_USERFLAG7          0x8000
#define SFX_ITEM_USERFLAG8          0x0010
#define SFX_ITEM_USERFLAG9          0x0020
#define SFX_ITEM_USERFLAGA          0x0040
#define SFX_ITEM_USERFLAGB          0x0080

//====================================================================

struct SfxItemInfo
{
    sal_uInt16      _nSID;
    sal_uInt16      _nFlags;
};

//====================================================================

class SfxStyleSheetIterator;
struct SfxPoolItemArray_Impl;
class SfxItemPool;

class SVL_DLLPUBLIC SfxItemPoolUser
{
public:
    virtual void ObjectInDestruction(const SfxItemPool& rSfxItemPool) = 0;
};

typedef ::std::vector< SfxItemPoolUser* > SfxItemPoolUserVector;

class SVL_DLLPUBLIC SfxItemPool

/*  [Beschreibung]

    Die von dieser Klasse abgeleiteten Klassen dienen der Bereitstellung von
    Defaults von SfxPoolItems und halten konkrete (konstante) Instanzen, die
    dann von mehreren Stellen (i.d.R. eines Dokuments) referenziert werden
    k�nnen.

    Dadurch ist jeder Wert nur einmalig gespeichert, was zu wenig Konstruktor
    und Destruktor-Aufrufen f�hrt, Vergleiche zwischen Items eines Dokuments
    beschleunigt und ein einfaches Laden und Speichern von Attributen
    bereitstellt.
*/

{
    SVL_DLLPRIVATE void readTheItems(SvStream & rStream, sal_uInt32 nCount, sal_uInt16 nVersion,
                                     SfxPoolItem * pDefItem, SfxPoolItemArray_Impl ** pArr);

    UniString                       aName;
    sal_uInt16                          nStart, nEnd;
    sal_uInt16                          _nFileFormatVersion;
#ifdef TF_POOLABLE
    const SfxItemInfo*              pItemInfos;
#else
    sal_uInt16*                         pSlotIds;
#endif
    SfxItemPool_Impl*               pImp;
    SfxPoolItem**                   ppStaticDefaults;
    SfxPoolItem**                   ppPoolDefaults;
    SfxItemPool*                    pSecondary;
    SfxItemPool*                    pMaster;
    sal_uInt16*                         _pPoolRanges;
    bool                        bPersistentRefCounts;

private:
    // ObjectUser section
    SfxItemPoolUserVector           maSfxItemPoolUsers;

public:
    void AddSfxItemPoolUser(SfxItemPoolUser& rNewUser);
    void RemoveSfxItemPoolUser(SfxItemPoolUser& rOldUser);

    //---------------------------------------------------------------------
#ifndef _SFXITEMS_HXX

friend class SfxPoolWhichMap;

private:
    inline  sal_uInt16                  GetIndex_Impl(sal_uInt16 nWhich) const;
    inline  sal_uInt16                  GetSize_Impl() const { return nEnd - nStart + 1; }

    SVL_DLLPRIVATE SvStream&                Load1_Impl( SvStream &rStream );
    SVL_DLLPRIVATE bool                     IsItemFlag_Impl( sal_uInt16 nWhich, sal_uInt16 nFlag ) const;

public:
    // fuer dflt. SfxItemSet::CTOR, setze dflt. WhichRanges
    void                            FillItemIdRanges_Impl( sal_uInt16*& pWhichRanges ) const;
    const sal_uInt16*                   GetFrozenIdRanges() const
                                    { return _pPoolRanges; }
    bool                        IsVer2_Impl() const;

#endif
    //---------------------------------------------------------------------

protected:
    static inline void              SetRefCount( SfxPoolItem& rItem, sal_uLong n );
    static inline sal_uLong             AddRef( const SfxPoolItem& rItem, sal_uLong n = 1 );
    static inline sal_uLong             ReleaseRef( const SfxPoolItem& rItem, sal_uLong n = 1);

public:
                                    SfxItemPool( const SfxItemPool &rPool,
                                                 sal_Bool bCloneStaticDefaults = sal_False );
                                    SfxItemPool( const UniString &rName,
                                                 sal_uInt16 nStart, sal_uInt16 nEnd,
#ifdef TF_POOLABLE
                                                 const SfxItemInfo *pItemInfos,
#endif
                                                 SfxPoolItem **pDefaults = 0,
#ifndef TF_POOLABLE
                                                 sal_uInt16 *pSlotIds = 0,
#endif
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
    virtual SfxItemPresentation     GetPresentation( const SfxPoolItem& rItem,
                                        SfxItemPresentation ePresentation,
                                        SfxMapUnit          ePresentationMetric,
                                        XubString&              rText,
                                        const IntlWrapper * pIntlWrapper
                                         = 0 ) const;
    virtual SfxItemPool*            Clone() const;
    UniString const &               GetName() const { return aName; }

    virtual const SfxPoolItem&      Put( const SfxPoolItem&, sal_uInt16 nWhich = 0 );
    virtual void                    Remove( const SfxPoolItem& );
    virtual const SfxPoolItem&      GetDefaultItem( sal_uInt16 nWhich ) const;

    const SfxPoolItem*              LoadItem( SvStream &rStream,
                                              bool bDirect = false,
                                              const SfxItemPool *pRefPool = 0 );
    bool                        StoreItem( SvStream &rStream,
                                               const SfxPoolItem &rItem,
                                               bool bDirect = false) const;

    sal_uInt32                      GetSurrogate(const SfxPoolItem *) const;
    const SfxPoolItem *             GetItem2(sal_uInt16 nWhich, sal_uInt32 nSurrogate) const;
    sal_uInt32                      GetItemCount2(sal_uInt16 nWhich) const;
    const SfxPoolItem*              LoadSurrogate(SvStream& rStream,
                                            sal_uInt16 &rWhich, sal_uInt16 nSlotId,
                                            const SfxItemPool* pRefPool = 0 );
    bool                        StoreSurrogate(SvStream& rStream,
                                            const SfxPoolItem *pItem ) const;

    virtual SvStream &              Load(SvStream &);
    virtual SvStream &              Store(SvStream &) const;
    int                             HasPersistentRefCounts() const {
                                        return bPersistentRefCounts; }
    void                            LoadCompleted();

    sal_uInt16                          GetFirstWhich() const { return nStart; }
    sal_uInt16                          GetLastWhich() const { return nEnd; }
    bool                        IsInRange( sal_uInt16 nWhich ) const {
                                        return nWhich >= nStart &&
                                               nWhich <= nEnd; }
    bool                        IsInVersionsRange( sal_uInt16 nWhich ) const;
    bool                        IsInStoringRange( sal_uInt16 nWhich ) const;
    void                            SetStoringRange( sal_uInt16 nFrom, sal_uInt16 nTo );
    void                            SetSecondaryPool( SfxItemPool *pPool );
    SfxItemPool*                    GetSecondaryPool() const {
                                        return pSecondary; }
    SfxItemPool*                    GetMasterPool() const {
                                        return pMaster; }
    void                            FreezeIdRanges();

    void                            Cleanup();
    void                            Delete();

#ifdef TF_POOLABLE
    bool                        IsItemFlag( sal_uInt16 nWhich, sal_uInt16 nFlag ) const;
    bool                        IsItemFlag( const SfxPoolItem &rItem, sal_uInt16 nFlag ) const
                                    { return IsItemFlag( rItem.Which(), nFlag ); }
    void                            SetItemInfos( const SfxItemInfo *pInfos )
                                    { pItemInfos = pInfos; }
#else
    int                             HasMap() const { return 0 != pSlotIds; }
    void                            SetMap( sal_uInt16 *pNewSlotIds )
                                    { pSlotIds = pNewSlotIds; }
#endif
    sal_uInt16                          GetWhich( sal_uInt16 nSlot, sal_Bool bDeep = sal_True ) const;
    sal_uInt16                          GetSlotId( sal_uInt16 nWhich, sal_Bool bDeep = sal_True ) const;
    sal_uInt16                          GetTrueWhich( sal_uInt16 nSlot, sal_Bool bDeep = sal_True ) const;
    sal_uInt16                          GetTrueSlotId( sal_uInt16 nWhich, sal_Bool bDeep = sal_True ) const;

    void                            SetVersionMap( sal_uInt16 nVer,
                                                   sal_uInt16 nOldStart, sal_uInt16 nOldEnd,
                                                   sal_uInt16 *pWhichIdTab );
    sal_uInt16                          GetNewWhich( sal_uInt16 nOldWhich ) const;
    sal_uInt16                          GetVersion() const;
    sal_uInt16                          GetFileFormatVersion() const
                                    { return _nFileFormatVersion; }
    void                            SetFileFormatVersion( sal_uInt16 nFileFormatVersion );
    sal_uInt16                          GetLoadingVersion() const;
    bool                        IsCurrentVersionLoading() const;

    static int                      IsWhich(sal_uInt16 nId) {
                                        return nId && nId <= SFX_WHICH_MAX; }
    static int                      IsSlot(sal_uInt16 nId) {
                                        return nId && nId > SFX_WHICH_MAX; }

    static const SfxItemPool*       GetStoringPool();
    static void                     SetStoringPool( const SfxItemPool * );

private:
    const SfxItemPool&              operator=(const SfxItemPool &);   // n.i.!!
};

// --------------- Inline Implementierungen ------------------------------

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

#endif
