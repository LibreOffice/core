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




#include <hash_map>
#include <vector>


#include "svxids.hrc"
#include "unoshprp.hxx"
#include "unoapi.hxx"
#include "svdobj.hxx"

#include <algorithm>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

using rtl::OUString;

//----------------------------------------------------------------------

struct SfxItemPropertyMapHash
{
    size_t operator()(const SfxItemPropertyMap* pMap) const { return (size_t)pMap; }
};

class SvxInfoSetCache;

class SvxCachedItemPropertySetInfo : public SfxItemPropertySetInfo
{
private:
    SvxInfoSetCache*	mpCache;

public:
    SvxCachedItemPropertySetInfo(const SfxItemPropertyMap *pMap, SvxInfoSetCache* pCache );
    virtual void SAL_CALL release() throw ();
};

/** this class caches the created XPropertySetInfo objects for each different
    SfxItemPropertyMap pointer. This class can't be used with dynamicly created
    SfxItemPropertyMaps!
*/

class SvxInfoSetCache
{
private:
    typedef std::hash_map< const SfxItemPropertyMap*, uno::Reference< beans::XPropertySetInfo >,  SfxItemPropertyMapHash > InfoMap;
    typedef std::hash_map< const SfxItemPropertyMap*, const SfxItemPropertyMap*,  SfxItemPropertyMapHash > PropertyMap;

    InfoMap maInfoMap;
    PropertyMap maPropertyMap;

    static ::osl::Mutex maMutex;
    static SvxInfoSetCache* mpGlobalCache;

    SvxInfoSetCache() {};
    ~SvxInfoSetCache() {};
public:
    static uno::Reference< beans::XPropertySetInfo > getCachedPropertySetInfo( const SfxItemPropertyMap* pMap );
    static const SfxItemPropertyMap* getSortedPropertyMap( const SfxItemPropertyMap* pMap );

    void dispose( SvxCachedItemPropertySetInfo* pInfo );
};

SvxCachedItemPropertySetInfo::SvxCachedItemPropertySetInfo(const SfxItemPropertyMap *pMap, SvxInfoSetCache* pCache )
: SfxItemPropertySetInfo( pMap ), mpCache( pCache )
{
}

void SAL_CALL SvxCachedItemPropertySetInfo::release() throw ()
{
    SvxInfoSetCache* pCache = mpCache;
    if( pCache && m_refCount == 2 )
    {
        mpCache = NULL;
        pCache->dispose( this );
    }
    SfxItemPropertySetInfo::release();
}

::osl::Mutex SvxInfoSetCache::maMutex;
SvxInfoSetCache* SvxInfoSetCache::mpGlobalCache = NULL;

uno::Reference< beans::XPropertySetInfo > SvxInfoSetCache::getCachedPropertySetInfo( const SfxItemPropertyMap* pMap )
{
    ::osl::MutexGuard aGuard(maMutex);

    if( NULL == mpGlobalCache )
        mpGlobalCache = new SvxInfoSetCache();

    InfoMap::iterator aIt(mpGlobalCache->maInfoMap.find(pMap));
    if (aIt != mpGlobalCache->maInfoMap.end())
        return aIt->second.get();

    uno::Reference< beans::XPropertySetInfo > xInfo( new SvxCachedItemPropertySetInfo( pMap, mpGlobalCache ) );
    mpGlobalCache->maInfoMap.insert(InfoMap::value_type(pMap,xInfo));

    /* if this assertion is triggered this class is possible used with dynamicly created
       SfxItemPropertyMap pointers. This will cause a cache overflow as the current
       implementation is designed for a limited number of different SfxItemPropertyMap
       pointers */
    DBG_ASSERT( mpGlobalCache->maInfoMap.size() < 200, "WARNING: SvxInfoSetCache::get(), possible cache overflow!" );
        
    return xInfo;
}

/** removes a cached property set info from the cache. This is called by the property set
    info when its refcount goes to 1, meaning the cache holds the last reference to the
    info
*/
void SvxInfoSetCache::dispose( SvxCachedItemPropertySetInfo* pInfo )
{
    if( pInfo )
    {
        ::osl::MutexGuard aGuard(maMutex);
    
        InfoMap::iterator aIt(mpGlobalCache->maInfoMap.find(pInfo->getMap()));
        if (aIt != mpGlobalCache->maInfoMap.end())
        {
            mpGlobalCache->maInfoMap.erase( aIt );
        }
    }
}

inline bool greater_size_pmap( const SfxItemPropertyMap* pFirst,
                              const SfxItemPropertyMap* pSecond )
{
    return strcmp( pFirst->pName, pSecond->pName ) < 0;
}


const SfxItemPropertyMap* SvxInfoSetCache::getSortedPropertyMap( const SfxItemPropertyMap* pMap )
{
    ::osl::MutexGuard aGuard(maMutex);

    if( NULL == mpGlobalCache )
        mpGlobalCache = new SvxInfoSetCache();

    const SfxItemPropertyMap* pSortedMap = NULL;
    PropertyMap::iterator aIt( mpGlobalCache->maPropertyMap.find(pMap) );
    if (aIt != mpGlobalCache->maPropertyMap.end())
        pSortedMap = aIt->second;

    if( NULL == pSortedMap )
    {
        // count the entries in the map
        std::vector< const SfxItemPropertyMap * >::size_type nCount = 0;
        const SfxItemPropertyMap* pTempMap = pMap;
        while( pTempMap->pName )
        {
            pTempMap++;
            nCount++;
        }

        // fill a stl vector with the entry pointers
        std::vector< const SfxItemPropertyMap * > aMap( nCount );
        std::vector< const SfxItemPropertyMap * >::iterator aIter( aMap.begin() );

        pTempMap = pMap;
        while( pTempMap->pName )
            *aIter++ = pTempMap++;

        // sort the vector
        std::sort( aMap.begin(), aMap.end(), greater_size_pmap );

        // create a new map
        pSortedMap = new SfxItemPropertyMap[nCount+1];
        pTempMap = pSortedMap;

        // copy the sorted entries to a new map
        aIter = aMap.begin();
        while( aIter != aMap.end() )
        {
            memcpy( (void*)pTempMap, *aIter++, sizeof( SfxItemPropertyMap ) );
            pTempMap++;
        }

        ((SfxItemPropertyMap*)pTempMap)->pName = NULL;

        mpGlobalCache->maPropertyMap[pMap] = pSortedMap;

        /* if this assertion is triggered this class is possible used with dynamicly created
           SfxItemPropertyMap pointers. This will cause a cache overflow as the current
           implementation is designed for a limited number of different SfxItemPropertyMap
           pointers */
        DBG_ASSERT( mpGlobalCache->maPropertyMap.size() < 200, "WARNING: SvxInfoSetCache::get(), possible cache overflow!" );
    }
    
    return pSortedMap;
}

//----------------------------------------------------------------------

struct SvxIDPropertyCombine
{
    sal_uInt16	nWID;
    uno::Any	aAny;
};

DECLARE_LIST( SvxIDPropertyCombineList, SvxIDPropertyCombine * )//STRIP008 ;

SvxItemPropertySet::SvxItemPropertySet( const SfxItemPropertyMap* pMap, sal_Bool bConvertTwips )
:	_pMap(SvxInfoSetCache::getSortedPropertyMap(pMap)), mbConvertTwips(bConvertTwips)
{
    mpLastMap = NULL;
    pCombiList = NULL;
}

//----------------------------------------------------------------------
SvxItemPropertySet::~SvxItemPropertySet()
{
/*
    if(pItemPool)
        delete pItemPool;
    pItemPool = NULL;
*/

    if(pCombiList)
        delete pCombiList;
    pCombiList = NULL;
}

//----------------------------------------------------------------------
uno::Any* SvxItemPropertySet::GetUsrAnyForID(sal_uInt16 nWID) const
{
    if(pCombiList && pCombiList->Count())
    {
        SvxIDPropertyCombine* pActual = pCombiList->First();
        while(pActual)
        {
            if(pActual->nWID == nWID)
                return &pActual->aAny;
            pActual = pCombiList->Next();

        }
    }
    return NULL;
}

//----------------------------------------------------------------------
void SvxItemPropertySet::AddUsrAnyForID(const uno::Any& rAny, sal_uInt16 nWID)
{
    if(!pCombiList)
        pCombiList = new SvxIDPropertyCombineList();

    SvxIDPropertyCombine* pNew = new SvxIDPropertyCombine;
    pNew->nWID = nWID;
    pNew->aAny = rAny;
    pCombiList->Insert(pNew);
}

//----------------------------------------------------------------------
void SvxItemPropertySet::ObtainSettingsFromPropertySet(SvxItemPropertySet& rPropSet,
  SfxItemSet& rSet, Reference< beans::XPropertySet > xSet )
{
    if(rPropSet.AreThereOwnUsrAnys())
    {
        const SfxItemPropertyMap* pSrc = rPropSet.getPropertyMap();
        const SfxItemPropertyMap* pDst = _pMap;
        while(pSrc->pName)
        {
            if(pSrc->nWID)
            {
                uno::Any* pUsrAny = rPropSet.GetUsrAnyForID(pSrc->nWID);
                if(pUsrAny)
                {
                    // Aequivalenten Eintrag in pDst suchen
                    const SfxItemPropertyMap* pTmp = pDst;
                    int nDiff = strcmp( pSrc->pName, pTmp->pName );
                    while(nDiff > 0)
                    {
                        pTmp++;
                        nDiff = strcmp( pSrc->pName, pTmp->pName );
                    }

                    if(nDiff == 0)
                    {
                        // Eintrag gefunden
                        pDst = pTmp;

                        if(pDst->nWID >= OWN_ATTR_VALUE_START && pDst->nWID <= OWN_ATTR_VALUE_END)
                        {
                            // Special ID im PropertySet, kann nur direkt am
                            // Objekt gesetzt werden+
                            OUString aName( OUString::createFromAscii( pDst->pName ) );
                            xSet->setPropertyValue( aName, *pUsrAny);
                        }
                        else
                        {
                            if(rSet.GetPool()->IsWhich(pDst->nWID))
                                rSet.Put(rSet.GetPool()->GetDefaultItem(pDst->nWID));

                            // setzen
                            setPropertyValue(pDst, *pUsrAny, rSet);
                        }
                    }
                }
            }

            // Naechster Eintrag
            pSrc++;
        }
    }
}

/** this function checks if a SFX_METRIC_ITEM realy needs to be converted.
    This check is for items that store either metric values if theire positiv
    or percentage if theire negativ.
*/
sal_Bool SvxUnoCheckForConversion( const SfxItemSet& rSet, sal_Int32 nWID, const uno::Any& rVal )
{
    sal_Bool bConvert = sal_True; // the default is that all metric items must be converted

    switch( nWID )
    {
    case XATTR_FILLBMP_SIZEX:
    case XATTR_FILLBMP_SIZEY:
        {
            sal_Int32 nValue;
            if( rVal >>= nValue )
                bConvert = nValue > 0;
            break;
        }
    }

    // the default is to always
    return bConvert;
}

//----------------------------------------------------------------------
uno::Any SvxItemPropertySet::getPropertyValue( const SfxItemPropertyMap* pMap, const SfxItemSet& rSet ) const
{
    uno::Any aVal;
    if(!pMap || !pMap->nWID)
        return aVal;

    // item holen
    const SfxPoolItem* pItem = 0;
    SfxItemPool* pPool = rSet.GetPool();

    SfxItemState eState = rSet.GetItemState( pMap->nWID, pMap->nWID != SDRATTR_XMLATTRIBUTES, &pItem );

    if( NULL == pItem && pPool )
    {
        pItem = &(pPool->GetDefaultItem( pMap->nWID ));
    }

    const SfxMapUnit eMapUnit = pPool ? pPool->GetMetric((USHORT)pMap->nWID) : SFX_MAPUNIT_100TH_MM;

    BYTE nMemberId = pMap->nMemberId & (~SFX_METRIC_ITEM);
    if( eMapUnit == SFX_MAPUNIT_100TH_MM )
        nMemberId &= (~CONVERT_TWIPS);

    // item-Wert als UnoAny zurueckgeben
    if(pItem)
    {
        pItem->QueryValue( aVal, nMemberId );

        if( pMap->nMemberId & SFX_METRIC_ITEM )
        {
            // check for needed metric translation
            if(pMap->nMemberId & SFX_METRIC_ITEM && eMapUnit != SFX_MAPUNIT_100TH_MM)
            {
                if( SvxUnoCheckForConversion( rSet, pMap->nWID, aVal ) )
                    SvxUnoConvertToMM( eMapUnit, aVal );
            }			
        }
        // convert typeless SfxEnumItem to enum type
        else if ( pMap->pType->getTypeClass() == uno::TypeClass_ENUM &&
              aVal.getValueType() == ::getCppuType((const sal_Int32*)0) )
        {
            sal_Int32 nEnum;
            aVal >>= nEnum;

            aVal.setValue( &nEnum, *pMap->pType );
        }
    }
    else
    {
        DBG_ERROR( "No SfxPoolItem found for property!" );
    }

    return aVal;
}

//----------------------------------------------------------------------
void SvxItemPropertySet::setPropertyValue( const SfxItemPropertyMap* pMap, const uno::Any& rVal, SfxItemSet& rSet ) const
{
    if(!pMap || !pMap->nWID)
        return;

    // item holen
    const SfxPoolItem* pItem = 0;
    SfxPoolItem *pNewItem = 0;
    SfxItemState eState = rSet.GetItemState( pMap->nWID, sal_True, &pItem );
    SfxItemPool* pPool = rSet.GetPool();

    // UnoAny in item-Wert stecken
    if(eState < SFX_ITEM_DEFAULT || pItem == NULL)
    {
        if( pPool == NULL )
        {
            DBG_ERROR( "No default item and no pool?" );
            return;
        }

        pItem = &pPool->GetDefaultItem( pMap->nWID );
    }

    DBG_ASSERT( pItem, "Got no default for item!" );
    if( pItem )
    {
        uno::Any aValue( rVal );

        const SfxMapUnit eMapUnit = pPool ? pPool->GetMetric((USHORT)pMap->nWID) : SFX_MAPUNIT_100TH_MM;

        if( pMap->nMemberId & SFX_METRIC_ITEM )
        {
            // check for needed metric translation
            if(pMap->nMemberId & SFX_METRIC_ITEM && eMapUnit != SFX_MAPUNIT_100TH_MM)
            {
                if( SvxUnoCheckForConversion( rSet, pMap->nWID, aValue ) )
                    SvxUnoConvertFromMM( eMapUnit, aValue );
            }			
        }

        pNewItem = pItem->Clone();

        BYTE nMemberId = pMap->nMemberId & (~SFX_METRIC_ITEM);
        if( eMapUnit == SFX_MAPUNIT_100TH_MM )
            nMemberId &= (~CONVERT_TWIPS);

        if( pNewItem->PutValue( aValue, nMemberId ) )
        {
            // neues item in itemset setzen
            rSet.Put( *pNewItem, pMap->nWID );
        }
        delete pNewItem;
    }
}

//----------------------------------------------------------------------
uno::Any SvxItemPropertySet::getPropertyValue( const SfxItemPropertyMap* pMap ) const
{
    // Schon ein Wert eingetragen? Dann schnell fertig
    uno::Any* pUsrAny = GetUsrAnyForID(pMap->nWID);
    if(pUsrAny)
        return *pUsrAny;

    // Noch kein UsrAny gemerkt, generiere Default-Eintrag und gib
    // diesen zurueck

    SdrItemPool* pItemPool = SdrObject::GetGlobalDrawObjectItemPool();
/*
    if(!pItemPool)
    {
        // ItemPool generieren
        ((SvxItemPropertySet*)this)->pItemPool = new SdrItemPool;
        // Der Outliner hat keinen eigenen Pool, deshalb den der EditEngine
        SfxItemPool* pOutlPool=EditEngine::CreatePool();
        // OutlinerPool als SecondaryPool des SdrPool
        pItemPool->SetSecondaryPool(pOutlPool);
    }
*/
    const SfxMapUnit eMapUnit = pItemPool ? pItemPool->GetMetric((USHORT)pMap->nWID) : SFX_MAPUNIT_100TH_MM;
    BYTE nMemberId = pMap->nMemberId & (~SFX_METRIC_ITEM);
    if( eMapUnit == SFX_MAPUNIT_100TH_MM )
        nMemberId &= (~CONVERT_TWIPS);

    uno::Any aVal;
    SfxItemSet aSet( *pItemPool, pMap->nWID, pMap->nWID);

    if( (pMap->nWID < OWN_ATTR_VALUE_START) && (pMap->nWID > OWN_ATTR_VALUE_END ) )
    {
        // Default aus ItemPool holen
        if(pItemPool->IsWhich(pMap->nWID))
            aSet.Put(pItemPool->GetDefaultItem(pMap->nWID));
    }

    if(aSet.Count())
    {
        const SfxPoolItem* pItem = NULL;
        SfxItemState eState = aSet.GetItemState( pMap->nWID, sal_True, &pItem );
        if(eState >= SFX_ITEM_DEFAULT && pItem)
        {
            pItem->QueryValue( aVal, nMemberId );
            ((SvxItemPropertySet*)this)->AddUsrAnyForID(aVal, pMap->nWID);
        }
    }

    if( pMap->nMemberId & SFX_METRIC_ITEM )
    {
        // check for needed metric translation
        if(pMap->nMemberId & SFX_METRIC_ITEM && eMapUnit != SFX_MAPUNIT_100TH_MM)
        {
            SvxUnoConvertToMM( eMapUnit, aVal );
        }			
    }

    if ( pMap->pType->getTypeClass() == uno::TypeClass_ENUM &&
          aVal.getValueType() == ::getCppuType((const sal_Int32*)0) )
    {
        sal_Int32 nEnum;
        aVal >>= nEnum;

        aVal.setValue( &nEnum, *pMap->pType );
    }

    return aVal;
}

//----------------------------------------------------------------------

void SvxItemPropertySet::setPropertyValue( const SfxItemPropertyMap* pMap, const uno::Any& rVal ) const
{
    uno::Any* pUsrAny = GetUsrAnyForID(pMap->nWID);
    if(!pUsrAny)
        ((SvxItemPropertySet*)this)->AddUsrAnyForID(rVal, pMap->nWID);
    else
        *pUsrAny = rVal;
}

//----------------------------------------------------------------------

const SfxItemPropertyMap* SvxItemPropertySet::getPropertyMapEntry(const OUString &rName) const
{
    const SfxItemPropertyMap* pMap = mpLastMap ? mpLastMap : _pMap;
    while ( pMap->pName ) 
    {
        if( rName.equalsAsciiL( pMap->pName, pMap->nNameLen ) )
        {
            const_cast<SvxItemPropertySet*>(this)->mpLastMap = pMap + 1;
            if( NULL == mpLastMap->pName )
                const_cast<SvxItemPropertySet*>(this)->mpLastMap = NULL;

            return pMap;
        }
        ++pMap;
    }

    if( mpLastMap == NULL )
        return 0;

    pMap = _pMap;
    while ( pMap->pName && (_pMap != mpLastMap) )
    {
        if( rName.equalsAsciiL( pMap->pName, pMap->nNameLen ) )
        {
            const_cast<SvxItemPropertySet*>(this)->mpLastMap = pMap + 1;
            if( NULL == mpLastMap->pName )
                const_cast<SvxItemPropertySet*>(this)->mpLastMap = NULL;
            return pMap;
        }
        ++pMap;
    }

    return 0;
}

//----------------------------------------------------------------------

Reference< ::com::sun::star::beans::XPropertySetInfo >  SvxItemPropertySet::getPropertySetInfo() const
{
    return SvxInfoSetCache::getCachedPropertySetInfo( _pMap );
}

//----------------------------------------------------------------------

#ifndef TWIPS_TO_MM
#define	TWIPS_TO_MM(val) ((val * 127 + 36) / 72)
#endif
#ifndef MM_TO_TWIPS
#define	MM_TO_TWIPS(val) ((val * 72 + 63) / 127)
#endif

/** converts the given any with a metric to 100th/mm if needed */
void SvxUnoConvertToMM( const SfxMapUnit eSourceMapUnit, ::com::sun::star::uno::Any & rMetric ) throw()
{
    // map the metric of the itempool to 100th mm
    switch(eSourceMapUnit)
    {
        case SFX_MAPUNIT_TWIP :
        {
            switch( rMetric.getValueTypeClass() )
            {
            case uno::TypeClass_BYTE:
                rMetric <<= (sal_Int8)(TWIPS_TO_MM(*(sal_Int8*)rMetric.getValue()));
                break;
            case uno::TypeClass_SHORT:
                rMetric <<= (sal_Int16)(TWIPS_TO_MM(*(sal_Int16*)rMetric.getValue()));
                break;
            case uno::TypeClass_UNSIGNED_SHORT:
                rMetric <<= (sal_uInt16)(TWIPS_TO_MM(*(sal_uInt16*)rMetric.getValue()));
                break;
            case uno::TypeClass_LONG:
                rMetric <<= (sal_Int32)(TWIPS_TO_MM(*(sal_Int32*)rMetric.getValue()));
                break;
            case uno::TypeClass_UNSIGNED_LONG:
                rMetric <<= (sal_uInt32)(TWIPS_TO_MM(*(sal_uInt32*)rMetric.getValue()));
                break;
            default:
                DBG_ERROR("AW: Missing unit translation to 100th mm!");
            }
            break;
        }
        default:
        {
            DBG_ERROR("AW: Missing unit translation to 100th mm!");
        }
    }
}

//----------------------------------------------------------------------

/** converts the given any with a metric from 100th/mm to the given metric if needed */
void SvxUnoConvertFromMM( const SfxMapUnit eDestinationMapUnit, ::com::sun::star::uno::Any & rMetric ) throw()
{
    switch(eDestinationMapUnit)
    {
        case SFX_MAPUNIT_TWIP :
        {
            switch( rMetric.getValueTypeClass() )
            {
                case uno::TypeClass_BYTE:
                    rMetric <<= (sal_Int8)(MM_TO_TWIPS(*(sal_Int8*)rMetric.getValue()));
                    break;
                case uno::TypeClass_SHORT:
                    rMetric <<= (sal_Int16)(MM_TO_TWIPS(*(sal_Int16*)rMetric.getValue()));
                    break;
                case uno::TypeClass_UNSIGNED_SHORT:
                    rMetric <<= (sal_uInt16)(MM_TO_TWIPS(*(sal_uInt16*)rMetric.getValue()));
                    break;
                case uno::TypeClass_LONG:
                    rMetric <<= (sal_Int32)(MM_TO_TWIPS(*(sal_Int32*)rMetric.getValue()));
                    break;
                case uno::TypeClass_UNSIGNED_LONG:
                    rMetric <<= (sal_uInt32)(MM_TO_TWIPS(*(sal_uInt32*)rMetric.getValue()));
                    break;
                default:
                    DBG_ERROR("AW: Missing unit translation to 100th mm!");
            }
            break;
        }
        default:
        {
            DBG_ERROR("AW: Missing unit translation to PoolMetrics!");
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
