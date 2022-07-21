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
#ifndef INCLUDED_SVL_ITEMPROP_HXX
#define INCLUDED_SVL_ITEMPROP_HXX

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <comphelper/propertysetinfo.hxx>
#include <cppuhelper/implbase.hxx>
#include <svl/itemset.hxx>
#include <svl/svldllapi.h>
#include <o3tl/sorted_vector.hxx>
#include <o3tl/span.hxx>
#include <string_view>
#include <utility>

// values from com/sun/star/beans/PropertyAttribute
#define PROPERTY_NONE 0

/// map a property between beans::XPropertySet and SfxPoolItem
struct SfxItemPropertyMapEntry
{
    OUString                            aName; ///< name of property
    css::uno::Type                      aType; ///< UNO type of property
    sal_uInt16                          nWID;  ///< WhichId of SfxPoolItem
    /// flag bitmap, @see css::beans::PropertyAttribute
    sal_Int16                           nFlags;
    /// "member ID" to tell QueryValue/PutValue which property it is
    /// (when multiple properties map to the same nWID)
    sal_uInt8                           nMemberId;
    PropertyMoreFlags                   nMoreFlags;

    SfxItemPropertyMapEntry(OUString _aName, sal_uInt16 _nWID, css::uno::Type const & _rType,
                               sal_Int16 _nFlags, sal_uInt8 const _nMemberId, PropertyMoreFlags _nMoreFlags = PropertyMoreFlags::NONE)
        : aName(std::move( _aName ))
        , aType(     _rType )
        , nWID(      _nWID )
        , nFlags(    _nFlags )
        , nMemberId( _nMemberId )
        , nMoreFlags( _nMoreFlags )
        {
            assert(_nFlags <= 0x1ff );
            assert( (_nMemberId & 0x40) == 0 );
            // Verify that if METRIC_ITEM is set, we are one of the types supported by
            // SvxUnoConvertToMM.
            assert(!(_nMoreFlags & PropertyMoreFlags::METRIC_ITEM) ||
                ( (aType.getTypeClass() == css::uno::TypeClass_BYTE)
                  || (aType.getTypeClass() == css::uno::TypeClass_SHORT)
                  || (aType.getTypeClass() == css::uno::TypeClass_UNSIGNED_SHORT)
                  || (aType.getTypeClass() == css::uno::TypeClass_LONG)
                  || (aType.getTypeClass() == css::uno::TypeClass_UNSIGNED_LONG)
                ) );
        }
};

struct SfxItemPropertyMapCompare
{
    bool operator() ( const SfxItemPropertyMapEntry * lhs, const SfxItemPropertyMapEntry * rhs ) const
    {
        return lhs->aName < rhs->aName;
    }
};
class SVL_DLLPUBLIC SfxItemPropertyMap
{
    o3tl::sorted_vector< const SfxItemPropertyMapEntry*, SfxItemPropertyMapCompare > m_aMap;
    mutable css::uno::Sequence< css::beans::Property > m_aPropSeq;
public:
    SfxItemPropertyMap( o3tl::span<const SfxItemPropertyMapEntry> pEntries );
    SfxItemPropertyMap( const SfxItemPropertyMap& rSource );
    ~SfxItemPropertyMap();

    const SfxItemPropertyMapEntry* getByName( std::u16string_view rName ) const;
    css::uno::Sequence< css::beans::Property > const & getProperties() const;
    /// @throws css::beans::UnknownPropertyException
    css::beans::Property getPropertyByName( const OUString & rName ) const;
    bool hasPropertyByName( std::u16string_view rName ) const;

    const o3tl::sorted_vector< const SfxItemPropertyMapEntry*, SfxItemPropertyMapCompare >& getPropertyEntries() const { return m_aMap; }
};

class SVL_DLLPUBLIC SfxItemPropertySet final
{
    SfxItemPropertyMap                                        m_aMap;
    mutable css::uno::Reference<css::beans::XPropertySetInfo> m_xInfo;

public:
                            SfxItemPropertySet( o3tl::span<const SfxItemPropertyMapEntry> pMap ) :
                                m_aMap(pMap) {}
                            ~SfxItemPropertySet();

    /// @throws css::uno::RuntimeException
    void getPropertyValue( const SfxItemPropertyMapEntry& rEntry,
                                          const SfxItemSet& rSet,
                                          css::uno::Any& rAny) const;
    /// @throws css::uno::RuntimeException
    /// @throws css::beans::UnknownPropertyException
    void getPropertyValue( const OUString &rName,
                                            const SfxItemSet& rSet,
                                            css::uno::Any& rAny) const;
    /// @throws css::uno::RuntimeException
    /// @throws css::beans::UnknownPropertyException
    css::uno::Any
        getPropertyValue( const OUString &rName,
                                            const SfxItemSet& rSet ) const;
    /// @throws css::uno::RuntimeException
    /// @throws css::lang::IllegalArgumentException
    void                setPropertyValue( const SfxItemPropertyMapEntry& rEntry,
                                          const css::uno::Any& aVal,
                                          SfxItemSet& rSet ) const;
    /// @throws css::uno::RuntimeException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::beans::UnknownPropertyException
    void                  setPropertyValue( const OUString& rPropertyName,
                                            const css::uno::Any& aVal,
                                            SfxItemSet& rSet ) const;

    /// @throws css::beans::UnknownPropertyException
    css::beans::PropertyState
        getPropertyState(const OUString& rName, const SfxItemSet& rSet)const;
    css::beans::PropertyState
        getPropertyState(const SfxItemPropertyMapEntry& rEntry, const SfxItemSet& rSet) const
                                    noexcept;

    css::uno::Reference<css::beans::XPropertySetInfo> const &
        getPropertySetInfo() const;
    const SfxItemPropertyMap& getPropertyMap() const {return m_aMap;}
};

// workaround for incremental linking bugs in MSVC2015
class SAL_DLLPUBLIC_TEMPLATE SfxItemPropertySetInfo_Base : public cppu::WeakImplHelper< css::beans::XPropertySetInfo > {};

class SVL_DLLPUBLIC SfxItemPropertySetInfo final : public SfxItemPropertySetInfo_Base
{
    SfxItemPropertyMap m_aOwnMap;

public:
    SfxItemPropertySetInfo(const SfxItemPropertyMap &rMap );
    SfxItemPropertySetInfo(o3tl::span<const SfxItemPropertyMapEntry> pEntries );
    virtual ~SfxItemPropertySetInfo() override;

    virtual css::uno::Sequence< css::beans::Property > SAL_CALL
        getProperties(  ) override;

    virtual css::beans::Property SAL_CALL
        getPropertyByName( const OUString& aName ) override;

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const OUString& Name ) override;

};

struct SfxItemPropertyMapCompare2
{
    bool operator() ( const SfxItemPropertyMapEntry & lhs, const SfxItemPropertyMapEntry & rhs ) const
    {
        return lhs.aName < rhs.aName;
    }
};

// workaround for incremental linking bugs in MSVC2015
class SAL_DLLPUBLIC_TEMPLATE SfxExtItemPropertySetInfo_Base : public cppu::WeakImplHelper< css::beans::XPropertySetInfo > {};

class SVL_DLLPUBLIC SfxExtItemPropertySetInfo final : public SfxExtItemPropertySetInfo_Base
{
public:
                            SfxExtItemPropertySetInfo(
                                o3tl::span<const SfxItemPropertyMapEntry> pMap,
                                const css::uno::Sequence<css::beans::Property>& rPropSeq );
                            virtual ~SfxExtItemPropertySetInfo() override;

    virtual css::uno::Sequence< css::beans::Property > SAL_CALL
        getProperties(  ) override;

    virtual css::beans::Property SAL_CALL
        getPropertyByName( const OUString& aName ) override;

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const OUString& Name ) override;

private:
    const SfxItemPropertyMapEntry* getByName( std::u16string_view rName ) const;
    o3tl::sorted_vector< SfxItemPropertyMapEntry, SfxItemPropertyMapCompare2 > maMap;
    mutable css::uno::Sequence< css::beans::Property > m_aPropSeq;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
