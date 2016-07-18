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

#ifndef INCLUDED_EDITENG_UNOIPSET_HXX
#define INCLUDED_EDITENG_UNOIPSET_HXX

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <editeng/editengdllapi.h>
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
    mutable css::uno::Reference<css::beans::XPropertySetInfo> m_xInfo;
    ::std::vector< SvxIDPropertyCombine* > aCombineList;
    SfxItemPool&                    mrItemPool;

public:
    SvxItemPropertySet( const SfxItemPropertyMapEntry *pMap, SfxItemPool& rPool );
    ~SvxItemPropertySet();

    // Methods, which work directly with the ItemSet
    static css::uno::Any getPropertyValue( const SfxItemPropertySimpleEntry* pMap, const SfxItemSet& rSet, bool bSearchInParent, bool bDontConvertNegativeValues );
    static void setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const css::uno::Any& rVal, SfxItemSet& rSet, bool bDontConvertNegativeValues );

    // Methods that use Any instead
    css::uno::Any getPropertyValue( const SfxItemPropertySimpleEntry* pMap ) const;
    void setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const css::uno::Any& rVal ) const;

    bool AreThereOwnUsrAnys() const { return ! aCombineList.empty(); }
    css::uno::Any* GetUsrAnyForID(sal_uInt16 nWID) const;
    void AddUsrAnyForID(const css::uno::Any& rAny, sal_uInt16 nWID);
    void ClearAllUsrAny();

    css::uno::Reference< css::beans::XPropertySetInfo > const & getPropertySetInfo() const;
    const SfxItemPropertyMap& getPropertyMap() const { return m_aPropertyMap;}
    const SfxItemPropertySimpleEntry* getPropertyMapEntry(const OUString &rName) const;
};

/** converts the given any with a metric to 100th/mm if needed */
EDITENG_DLLPUBLIC void SvxUnoConvertToMM( const SfxMapUnit eSourceMapUnit, css::uno::Any & rMetric ) throw();

/** converts the given any with a metric from 100th/mm to the given metric if needed */
EDITENG_DLLPUBLIC void SvxUnoConvertFromMM( const SfxMapUnit eDestinationMapUnit, css::uno::Any & rMetric ) throw();

#endif // INCLUDED_EDITENG_UNOIPSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
