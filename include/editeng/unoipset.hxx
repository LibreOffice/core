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

#include <editeng/editengdllapi.h>
#include <rtl/ref.hxx>
#include <svl/itemprop.hxx>
#include <vector>

class SfxItemSet;
class SvxItemPropertySetUsrAnys;

class EDITENG_DLLPUBLIC SvxItemPropertySet
{
    SfxItemPropertyMap          m_aPropertyMap;
    mutable rtl::Reference<SfxItemPropertySetInfo> m_xInfo;
    SfxItemPool&                    mrItemPool;

public:
    SvxItemPropertySet( std::span<const SfxItemPropertyMapEntry> pMap, SfxItemPool& rPool );
    ~SvxItemPropertySet();

    SvxItemPropertySet& operator=( SvxItemPropertySet const & ) = delete; // MSVC2015 workaround
    SvxItemPropertySet( SvxItemPropertySet const & ) = delete; // MSVC2015 workaround

    // Methods, which work directly with the ItemSet
    static css::uno::Any getPropertyValue( const SfxItemPropertyMapEntry* pMap, const SfxItemSet& rSet, bool bSearchInParent, bool bDontConvertNegativeValues );
    static void setPropertyValue( const SfxItemPropertyMapEntry* pMap, const css::uno::Any& rVal, SfxItemSet& rSet, bool bDontConvertNegativeValues );

    // Methods that use Any instead
    css::uno::Any getPropertyValue( const SfxItemPropertyMapEntry* pMap, SvxItemPropertySetUsrAnys& rAnys ) const;
    static void setPropertyValue( const SfxItemPropertyMapEntry* pMap, const css::uno::Any& rVal, SvxItemPropertySetUsrAnys& rAnys );

    rtl::Reference< SfxItemPropertySetInfo > const & getPropertySetInfo() const;
    const SfxItemPropertyMap& getPropertyMap() const { return m_aPropertyMap;}
    const SfxItemPropertyMapEntry* getPropertyMapEntry(std::u16string_view rName) const;
};

struct SvxIDPropertyCombine
{
    sal_uInt16  nWID;
    sal_uInt8   memberId;
    css::uno::Any    aAny;
};


class EDITENG_DLLPUBLIC SvxItemPropertySetUsrAnys
{
    ::std::vector< SvxIDPropertyCombine > aCombineList;

public:
    SvxItemPropertySetUsrAnys();
    ~SvxItemPropertySetUsrAnys();
    bool AreThereOwnUsrAnys() const { return ! aCombineList.empty(); }
    css::uno::Any* GetUsrAnyForID(SfxItemPropertyMapEntry const & entry) const;
    void AddUsrAnyForID(const css::uno::Any& rAny, SfxItemPropertyMapEntry const & entry);
    void ClearAllUsrAny();
};

/** converts the given any with a metric to 100th/mm if needed */
EDITENG_DLLPUBLIC void SvxUnoConvertToMM( const MapUnit eSourceMapUnit, css::uno::Any & rMetric ) noexcept;

/** converts the given any with a metric from 100th/mm to the given metric if needed */
EDITENG_DLLPUBLIC void SvxUnoConvertFromMM( const MapUnit eDestinationMapUnit, css::uno::Any & rMetric ) noexcept;

#endif // INCLUDED_EDITENG_UNOIPSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
