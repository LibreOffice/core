/*************************************************************************
 *
 *  $RCSfile: unoipset.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-20 12:19:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#include <svtools/itemprop.hxx>

#include "unoipset.hxx"
#include "svdpool.hxx"
#include "svxids.hrc"
#include "deflt3d.hxx"
#include "unoshprp.hxx"
#include "editeng.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

struct SvxIDPropertyCombine
{
    sal_uInt16  nWID;
    uno::Any    aAny;
};

DECLARE_LIST( SvxIDPropertyCombineList, SvxIDPropertyCombine * );

SvxItemPropertySet::SvxItemPropertySet( const SfxItemPropertyMap* pMap )
:   _pMap(pMap)
{
    pItemPool = NULL;
    pCombiList = NULL;
}

//----------------------------------------------------------------------
SvxItemPropertySet::~SvxItemPropertySet()
{
    if(pItemPool)
        delete pItemPool;
    pItemPool = NULL;

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
//-/void SvxItemPropertySet::Generate3DAttrDefaultItem(sal_uInt16 nWID, SfxItemSet& rSet)
//-/{
//-/    SfxItemSet aSet( *rSet.GetPool(), SID_ATTR_3D_START, SID_ATTR_3D_END);
//-/    E3dDefaultAttributes a3DDefaultAttr;
//-/    a3DDefaultAttr.TakeDefaultValues(aSet);
//-/    const SfxPoolItem* pItem = 0;
//-/    SfxPoolItem *pNewItem = 0;
//-/    SfxItemState eState = aSet.GetItemState( nWID, sal_True, &pItem );
//-/    if(eState >= SFX_ITEM_DEFAULT && pItem)
//-/    {
//-/        pNewItem = pItem->Clone();
//-/        rSet.Put( *pNewItem, nWID );
//-/        delete pNewItem;
//-/    }
//-/}

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
                            // Eintrag sicherstellen in ItemSet
//-/                            if(pDst->nWID >= SID_ATTR_3D_START && pDst->nWID <= SID_ATTR_3D_END)
//-/                            {
//-/                                // 3D-Attribut, eigenen Default
//-/                                Generate3DAttrDefaultItem(pDst->nWID, rSet);
//-/                            }
//-/                            else
//-/                            {
                                // Default aus ItemPool holen
                                if(rSet.GetPool()->IsWhich(pDst->nWID))
                                    rSet.Put(rSet.GetPool()->GetDefaultItem(pDst->nWID));
//-/                            }

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

//----------------------------------------------------------------------
uno::Any SvxItemPropertySet::getPropertyValue( const SfxItemPropertyMap* pMap, const SfxItemSet& rSet ) const
{
    uno::Any aVal;
    if(!pMap || !pMap->nWID)
        return aVal;

    // item holen
    const SfxPoolItem* pItem = 0;
    SfxItemState eState = rSet.GetItemState( pMap->nWID, sal_True, &pItem );

    if( NULL == pItem )
    {
        SfxItemPool* pPool = rSet.GetPool();
        if( pPool )
        {
            pItem = &(pPool->GetDefaultItem( pMap->nWID ));
        }
    }

    // item-Wert als UnoAny zurueckgeben
    if(pItem)
    {
        pItem->QueryValue( aVal, pMap->nMemberId );

        // allgemeine SfxEnumItem Values in konkrete wandeln
        if ( pMap->pType->getTypeClass() == uno::TypeClass_ENUM &&
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

    // UnoAny in item-Wert stecken
    if(eState < SFX_ITEM_DEFAULT || pItem == NULL)
    {
        SfxItemPool* pPool = rSet.GetPool();
        if( pPool == NULL )
        {
            DBG_ERROR( "No default item and no pool?" );
            return;
        }

        pItem = pPool->GetPoolDefaultItem( pMap->nWID );
    }

    if( pItem )
    {
        pNewItem = pItem->Clone();
        if( pNewItem->PutValue( rVal, pMap->nMemberId ) )
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
    if(!pItemPool)
    {
        // ItemPool generieren
        ((SvxItemPropertySet*)this)->pItemPool = new SdrItemPool;
        // pItemPool->SetDefaultMetric(...eScaleUnit...);
        // Der Outliner hat keinen eigenen Pool, deshalb den der EditEngine
        SfxItemPool* pOutlPool=EditEngine::CreatePool();
        // OutlinerPool als SecondaryPool des SdrPool
        pItemPool->SetSecondaryPool(pOutlPool);
    }

    uno::Any aVal;
    SfxItemSet aSet( *pItemPool, pMap->nWID, pMap->nWID);
//-/    if(pMap->nWID >= SID_ATTR_3D_START && pMap->nWID <= SID_ATTR_3D_END)
//-/    {
//-/        // 3D-Attribut, eigenen Default generieren
//-/        ((SvxItemPropertySet*)this)->Generate3DAttrDefaultItem(pMap->nWID, aSet);
//-/    }
//-/    else
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
            pItem->QueryValue( aVal, pMap->nMemberId );
            ((SvxItemPropertySet*)this)->AddUsrAnyForID(aVal, pMap->nWID);
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
    return SfxItemPropertyMap::GetByName( _pMap, rName );
}

//----------------------------------------------------------------------

Reference< ::com::sun::star::beans::XPropertySetInfo >  SvxItemPropertySet::getPropertySetInfo() const
{
    Reference< ::com::sun::star::beans::XPropertySetInfo >  aRef(new SfxItemPropertySetInfo( _pMap ));
    return aRef;
}
