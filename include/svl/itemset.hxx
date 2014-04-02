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
#include <tools/rtti.hxx>

class SfxItemPool;
class SfxPoolItem;
class SvStream;

typedef SfxPoolItem const** SfxItemArray;

#define USHORT_ARG int

#ifdef DBG
#undef DBG
#endif
#define DBG(s)

#define SFX_ITEMSET_GET( rSet, pItem, ItemType, nSlotId, bDeep ) \
    const ItemType *pItem = (const ItemType*) \
                            (rSet).GetItem( nSlotId, bDeep, TYPE(ItemType) )

class SVL_DLLPUBLIC SfxItemSet
{
    friend class SfxItemIter;

    SfxItemPool*                _pPool;         // der verwendete Pool
    const SfxItemSet*           _pParent;       // Ableitung
    SfxItemArray                _aItems;        // Item-Feld
    sal_uInt16*                     _pWhichRanges;  // Array von Which-Bereichen
    sal_uInt16                      _nCount;        // Anzahl Items

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

private:
    const SfxItemSet&           operator=(const SfxItemSet &);   // n.i.!!

protected:
    // Notification-Callback
    virtual void                Changed( const SfxPoolItem& rOld, const SfxPoolItem& rNew );

    // direkte Put-Methode
    int                         PutDirect(const SfxPoolItem &rItem);

public:
                                SfxItemSet( const SfxItemSet& );

                                SfxItemSet( SfxItemPool&, bool bTotalPoolRanges = false );
                                SfxItemSet( SfxItemPool&, sal_uInt16 nWhich1, sal_uInt16 nWhich2 );
                                SfxItemSet( SfxItemPool&, USHORT_ARG nWh1, USHORT_ARG nWh2, USHORT_ARG nNull, ... );
                                SfxItemSet( SfxItemPool&, const sal_uInt16* nWhichPairTable );
    virtual                     ~SfxItemSet();

    virtual SfxItemSet *        Clone(bool bItems = true, SfxItemPool *pToPool = 0) const;

    // Items erfragen
    sal_uInt16                      Count() const { return _nCount; }
    sal_uInt16                      TotalCount() const;

    virtual const SfxPoolItem&  Get( sal_uInt16 nWhich, bool bSrchInParent = true ) const;
    const SfxPoolItem*          GetItem( sal_uInt16 nWhich, bool bSrchInParent = true,
                                         TypeId aItemType = 0 ) const;

    // Which-Wert des Items an der Position nPos erfragen
    sal_uInt16                      GetWhichByPos(sal_uInt16 nPos) const;

    // Item-Status erfragen
    SfxItemState                GetItemState(   sal_uInt16 nWhich,
                                                bool bSrchInParent = true,
                                                const SfxPoolItem **ppItem = 0 ) const;

    bool                        HasItem(sal_uInt16 nWhich, const SfxPoolItem** ppItem = NULL) const;

    virtual void                DisableItem(sal_uInt16 nWhich);
    virtual void                InvalidateItem( sal_uInt16 nWhich );
    virtual sal_uInt16          ClearItem( sal_uInt16 nWhich = 0);
    virtual void                ClearInvalidItems( bool bHardDefault = false );
            void                InvalidateAllItems(); // HACK(via nWhich = 0) ???
            void                InvalidateDefaultItems();

    inline void                 SetParent( const SfxItemSet* pNew );

    // Items hinzufuegen, loeschen etc.
    virtual const SfxPoolItem*  Put( const SfxPoolItem&, sal_uInt16 nWhich );
    const SfxPoolItem*          Put( const SfxPoolItem& rItem )
                                { return Put(rItem, rItem.Which()); }
    bool                        Put( const SfxItemSet&,
                                     bool bInvalidAsDefault = true );
    void                        PutExtended( const SfxItemSet&,
                                             SfxItemState eDontCareAs = SFX_ITEM_UNKNOWN,
                                             SfxItemState eDefaultAs = SFX_ITEM_UNKNOWN );

    virtual bool                Set( const SfxItemSet&, bool bDeep = true );

    virtual void                Intersect( const SfxItemSet& rSet );
    virtual void                MergeValues( const SfxItemSet& rSet, bool bOverwriteDefaults = false );
    virtual void                Differentiate( const SfxItemSet& rSet );
    virtual void                MergeValue( const SfxPoolItem& rItem, bool bOverwriteDefaults = false  );

    SfxItemPool*                GetPool() const { return _pPool; }
    const sal_uInt16*               GetRanges() const { return _pWhichRanges; }
    void                        SetRanges( const sal_uInt16 *pRanges );
    void                        MergeRange( sal_uInt16 nFrom, sal_uInt16 nTo );
    const SfxItemSet*           GetParent() const { return _pParent; }

    virtual SvStream &          Load( SvStream &, bool bDirect = false,
                                      const SfxItemPool *pRefPool = 0 );
    virtual SvStream &          Store( SvStream &, bool bDirect = false ) const;

    bool                        operator==(const SfxItemSet &) const;
    sal_Int32                   getHash() const;
    virtual OString             stringify() const;
};

inline void SfxItemSet::SetParent( const SfxItemSet* pNew )
{
    DBG( if (_pParent) --*_pChildCount(_pParent) );
    _pParent = pNew;
    DBG( if (_pParent) ++*_pChildCount(_pParent) );
}

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

    virtual SfxItemSet *        Clone( bool bItems = true, SfxItemPool *pToPool = 0 ) const SAL_OVERRIDE;
    virtual const SfxPoolItem*  Put( const SfxPoolItem&, sal_uInt16 nWhich ) SAL_OVERRIDE;
    using SfxItemSet::Put;
};

#endif // INCLUDED_SVL_ITEMSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
