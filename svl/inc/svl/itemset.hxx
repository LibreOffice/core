/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFXITEMSET_HXX
#define _SFXITEMSET_HXX

#include "svl/svldllapi.h"

#include <cstdarg>  // std::va_list and friends
#include <svl/poolitem.hxx>
#include <tools/rtti.hxx>
#include <tools/solar.h>

class SfxItemPool;
class SfxPoolItem;
class SvStream;

typedef SfxPoolItem const** SfxItemArray;

#define USHORT_ARG int

#ifdef MI_HATS_REPARIERT
#ifndef DBG
#ifdef DBG_UTILx
#define DBG(s) s
#define _pChildCount(THIS)  (  *(sal_uInt16**)SfxPointerServer::GetServer()->GetPointer(THIS) )
#define _pChildCountCtor    ( (*(sal_uInt16**)SfxPointerServer::GetServer()->CreatePointer(this)) = new sal_uInt16 )
#define _pChildCountDtor    ( SfxPointerServer::GetServer()->ReleasePointer(this) )
#else
#define DBG(s)
#endif
#endif
#else
#ifdef DBG
#undef DBG
#endif
#define DBG(s)
#endif

//========================================================================

#define SFX_ITEMSET_GET( rSet, pItem, ItemType, nSlotId, bDeep ) \
    const ItemType *pItem = (const ItemType*) \
                            (rSet).GetItem( nSlotId, bDeep, TYPE(ItemType) )

//========================================================================

class SVL_DLLPUBLIC SfxItemSet
{
    friend class SfxItemIter;

    SfxItemPool*                _pPool;         // der verwendete Pool
    const SfxItemSet*           _pParent;       // Ableitung
    SfxItemArray                _aItems;        // Item-Feld
    sal_uInt16*                     _pWhichRanges;  // Array von Which-Bereichen
    sal_uInt16                      _nCount;        // Anzahl Items

    //---------------------------------------------------------------------
#ifndef _SFXITEMS_HXX

friend class SfxItemPoolCache;
friend class SfxAllItemSet;
friend const char *DbgCheckItemSet( const void* );

private:
    SVL_DLLPRIVATE void                     InitRanges_Impl(const sal_uInt16 *nWhichPairTable);
    SVL_DLLPRIVATE void                     InitRanges_Impl(va_list pWhich, sal_uInt16 n1, sal_uInt16 n2, sal_uInt16 n3);
    SVL_DLLPRIVATE void                     InitRanges_Impl(sal_uInt16 nWh1, sal_uInt16 nWh2);

public:
    SfxItemArray                GetItems_Impl() const { return _aItems; }

#endif
    //---------------------------------------------------------------------

private:
    const SfxItemSet&           operator=(const SfxItemSet &);   // n.i.!!

protected:
    // Notification-Callback
    virtual void                Changed( const SfxPoolItem& rOld, const SfxPoolItem& rNew );

    // direkte Put-Methode
    int                         PutDirect(const SfxPoolItem &rItem);

public:
                                SfxItemSet( const SfxItemSet& );

                                SfxItemSet( SfxItemPool&, sal_Bool bTotalPoolRanges = sal_False );
                                SfxItemSet( SfxItemPool&, sal_uInt16 nWhich1, sal_uInt16 nWhich2 );
                                SfxItemSet( SfxItemPool&, USHORT_ARG nWh1, USHORT_ARG nWh2, USHORT_ARG nNull, ... );
                                SfxItemSet( SfxItemPool&, const sal_uInt16* nWhichPairTable );
    virtual                     ~SfxItemSet();

    virtual SfxItemSet *        Clone(sal_Bool bItems = sal_True, SfxItemPool *pToPool = 0) const;

    // Items erfragen
    sal_uInt16                      Count() const { return _nCount; }
    sal_uInt16                      TotalCount() const;

    virtual const SfxPoolItem&  Get( sal_uInt16 nWhich, sal_Bool bSrchInParent = sal_True ) const;
    const SfxPoolItem*          GetItem( sal_uInt16 nWhich, sal_Bool bSrchInParent = sal_True,
                                         TypeId aItemType = 0 ) const;

    // Which-Wert des Items an der Position nPos erfragen
    sal_uInt16                      GetWhichByPos(sal_uInt16 nPos) const;

    // Item-Status erfragen
    SfxItemState                GetItemState(   sal_uInt16 nWhich,
                                                sal_Bool bSrchInParent = sal_True,
                                                const SfxPoolItem **ppItem = 0 ) const;

    virtual void                DisableItem(sal_uInt16 nWhich);
    virtual void                InvalidateItem( sal_uInt16 nWhich );
    virtual sal_uInt16              ClearItem( sal_uInt16 nWhich = 0);
    virtual void                ClearInvalidItems( sal_Bool bHardDefault = sal_False );
            void                InvalidateAllItems(); HACK(via nWhich = 0)

    inline void                 SetParent( const SfxItemSet* pNew );

    // Items hinzufuegen, loeschen etc.
    virtual const SfxPoolItem*  Put( const SfxPoolItem&, sal_uInt16 nWhich );
    const SfxPoolItem*          Put( const SfxPoolItem& rItem )
                                { return Put(rItem, rItem.Which()); }
    virtual int                 Put( const SfxItemSet&,
                                     sal_Bool bInvalidAsDefault = sal_True );
    void                        PutExtended( const SfxItemSet&,
                                             SfxItemState eDontCareAs = SFX_ITEM_UNKNOWN,
                                             SfxItemState eDefaultAs = SFX_ITEM_UNKNOWN );

    virtual int                 Set( const SfxItemSet&, sal_Bool bDeep = sal_True );

    virtual void                Intersect( const SfxItemSet& rSet );
    virtual void                MergeValues( const SfxItemSet& rSet, sal_Bool bOverwriteDefaults = sal_False );
    virtual void                Differentiate( const SfxItemSet& rSet );
    virtual void                MergeValue( const SfxPoolItem& rItem, sal_Bool bOverwriteDefaults = sal_False  );

    SfxItemPool*                GetPool() const { return _pPool; }
    const sal_uInt16*               GetRanges() const { return _pWhichRanges; }
    void                        SetRanges( const sal_uInt16 *pRanges );
    void                        MergeRange( sal_uInt16 nFrom, sal_uInt16 nTo );
    const SfxItemSet*           GetParent() const { return _pParent; }

    virtual SvStream &          Load( SvStream &, bool bDirect = false,
                                      const SfxItemPool *pRefPool = 0 );
    virtual SvStream &          Store( SvStream &, bool bDirect = false ) const;

    virtual int                 operator==(const SfxItemSet &) const;
};

// --------------- Inline Implementierungen ------------------------

inline void SfxItemSet::SetParent( const SfxItemSet* pNew )
{
    DBG( if (_pParent) --*_pChildCount(_pParent) );
    _pParent = pNew;
    DBG( if (_pParent) ++*_pChildCount(_pParent) );
}

//========================================================================

class SVL_DLLPUBLIC SfxAllItemSet: public SfxItemSet

/*  versteht alle Ranges; werden durch das Putten der Items
    automatisch angepasst
*/

{
    SfxVoidItem                 aDefault;
    sal_uInt16                      nFree;

public:
                                SfxAllItemSet( SfxItemPool &rPool );
                                SfxAllItemSet( const SfxItemSet & );
                                SfxAllItemSet( const SfxAllItemSet & );

    virtual SfxItemSet *        Clone( sal_Bool bItems = sal_True, SfxItemPool *pToPool = 0 ) const;
    virtual const SfxPoolItem*  Put( const SfxPoolItem&, sal_uInt16 nWhich );
    const SfxPoolItem*          Put( const SfxPoolItem& rItem )
    { return Put(rItem, rItem.Which()); }
    virtual int                 Put( const SfxItemSet&,
                                     sal_Bool bInvalidAsDefault = sal_True );
};

#endif // #ifndef _SFXITEMSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
