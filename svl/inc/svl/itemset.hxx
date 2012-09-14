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


#ifndef _SFXITEMSET_HXX
#define _SFXITEMSET_HXX

#include "svl/svldllapi.h"

#if STLPORT_VERSION<321
#include <stdarg.h>
#else
#include <cstdarg>  // std::va_list and friends
#endif
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

    //optimize a comparing operation from 'memcmp' to 'hash compare' to improve xls loading performance, i120575
    sal_Int32 _aHashKey; //hash result of array of points: _aItems.

    void UpdateHashKey();
    //Need invlidate the hashkey at every possible place where the array _aItems may be changed.
    //thread safe : there is always solarmutex outter, so no need to add mutex here.
    void InvalidateHashKey() { _aHashKey = 0;} //treat '0' as an invalidate key.
    sal_Bool IsValidateHashKey() const { return (0!=_aHashKey);}
    sal_Int32 GetHashKey() const { return _aHashKey; }
    //end:i120575
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
    //optimize a comparing operation from 'memcmp' to 'hash compare' to improve xls loading performance, i120575
    //Make this method public is dangerous, may disrupt the item array. so invalidate the hash key here.
    //currently this method is never called, pls also do not do it in future.
    SfxItemArray    GetItems_Impl() const
    {
        const_cast<SfxItemSet &>(*this).InvalidateHashKey();
        return _aItems;
    }//end:i120575

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

    virtual SvStream &          Load( SvStream &, FASTBOOL bDirect = sal_False,
                                      const SfxItemPool *pRefPool = 0 );
    virtual SvStream &          Store( SvStream &, FASTBOOL bDirect = sal_False ) const;

    virtual int                 operator==(const SfxItemSet &) const;

    //optimize a comparing operation from 'memcmp' to 'hash compare' to improve xls loading performance, i120575
    //in some situation (e.g.. ScPatternAttr::operator== ),
    //two sfxitemset can be compared 'quickly' by the Hashkey only.
    //may also update the hashkey in this method.
    sal_Bool QuickCompare( SfxItemSet & rCmp);
    //end:i120575
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

