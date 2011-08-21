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

#ifndef _SVX_UNOIPSET_HXX_
#define _SVX_UNOIPSET_HXX_

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "editeng/editengdllapi.h"
#include <svl/itemprop.hxx>
#include <vector>

class SdrItemPool;
class SfxItemSet;
class SvxShape;
struct SvxIDPropertyCombine;

#define SFX_METRIC_ITEM                         (0x40)

class EDITENG_DLLPUBLIC SvxItemPropertySet
{
    SfxItemPropertyMap          m_aPropertyMap;
    mutable com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo> m_xInfo;
    const SfxItemPropertyMapEntry*  _pMap;
    ::std::vector< SvxIDPropertyCombine* > aCombineList;
    sal_Bool                        mbConvertTwips;
    SfxItemPool&                    mrItemPool;

public:
    SvxItemPropertySet( const SfxItemPropertyMapEntry *pMap, SfxItemPool& rPool, sal_Bool bConvertTwips = sal_False );
    ~SvxItemPropertySet();

    // Methods, which work directly with the ItemSet
    ::com::sun::star::uno::Any getPropertyValue( const SfxItemPropertySimpleEntry* pMap, const SfxItemSet& rSet, bool bSearchInParent, bool bDontConvertNegativeValues ) const;
    void setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const ::com::sun::star::uno::Any& rVal, SfxItemSet& rSet, bool bDontConvertNegativeValues ) const;

    // Methods that use Any instead
    ::com::sun::star::uno::Any getPropertyValue( const SfxItemPropertySimpleEntry* pMap ) const;
    void setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const ::com::sun::star::uno::Any& rVal ) const;

    sal_Bool AreThereOwnUsrAnys() const { return ( aCombineList.empty() ? sal_False : sal_True ); }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
