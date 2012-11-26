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



#ifndef _SVX_UNOIPSET_HXX_
#define _SVX_UNOIPSET_HXX_

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "editeng/editengdllapi.h"
#include <svl/itemprop.hxx>

class SvxIDPropertyCombineList;
class SfxItemSet;
class SvxShape;

#define SFX_METRIC_ITEM                         (0x40)

class EDITENG_DLLPUBLIC SvxItemPropertySet
{
    SfxItemPropertyMap          m_aPropertyMap;
    mutable com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo> m_xInfo;
    const SfxItemPropertyMapEntry*  _pMap;
    SvxIDPropertyCombineList*   pCombiList;
    sal_Bool                    mbConvertTwips;
    SfxItemPool&                mrItemPool;

public:
    SvxItemPropertySet( const SfxItemPropertyMapEntry *pMap, SfxItemPool& rPool, sal_Bool bConvertTwips = sal_False );
    ~SvxItemPropertySet();

    // Methoden, die direkt mit dem ItemSet arbeiten
    ::com::sun::star::uno::Any getPropertyValue( const SfxItemPropertySimpleEntry* pMap, const SfxItemSet& rSet, bool bSearchInParent, bool bDontConvertNegativeValues ) const;
    void setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const ::com::sun::star::uno::Any& rVal, SfxItemSet& rSet, bool bDontConvertNegativeValues ) const;

    // Methoden, die stattdessen Any benutzen
    ::com::sun::star::uno::Any getPropertyValue( const SfxItemPropertySimpleEntry* pMap ) const;
    void setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const ::com::sun::star::uno::Any& rVal ) const;

    sal_Bool AreThereOwnUsrAnys() const { return (pCombiList ? sal_True : sal_False); }
    ::com::sun::star::uno::Any* GetUsrAnyForID(sal_uInt16 nWID) const;
    void AddUsrAnyForID(const ::com::sun::star::uno::Any& rAny, sal_uInt16 nWID);
    void ClearAllUsrAny();

    com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > getPropertySetInfo() const;
    const SfxItemPropertyMapEntry* getPropertyMapEntries() const {return _pMap;}
    const SfxItemPropertyMap* getPropertyMap()const { return &m_aPropertyMap;}
    const SfxItemPropertySimpleEntry* getPropertyMapEntry(const ::rtl::OUString &rName) const;

    static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > getPropertySetInfo( const SfxItemPropertyMapEntry* pMap );
};

/** converts the given any with a metric to 100th/mm if needed */
EDITENG_DLLPUBLIC void SvxUnoConvertToMM( const SfxMapUnit eSourceMapUnit, com::sun::star::uno::Any & rMetric ) throw();

/** converts the given any with a metric from 100th/mm to the given metric if needed */
EDITENG_DLLPUBLIC void SvxUnoConvertFromMM( const SfxMapUnit eDestinationMapUnit, com::sun::star::uno::Any & rMetric ) throw();

#endif // _SVX_UNOIPSET_HXX_

