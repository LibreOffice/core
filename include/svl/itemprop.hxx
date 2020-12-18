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
#include <vector>
#include <memory>
#include <string_view>

// values from com/sun/star/beans/PropertyAttribute
#define PROPERTY_NONE 0

/// map a property between beans::XPropertySet and SfxPoolItem
struct SfxItemPropertyMapEntry
{
    std::u16string_view                 aName; ///< name of property
    css::uno::Type                      aType; ///< UNO type of property
    sal_uInt16                          nWID;  ///< WhichId of SfxPoolItem
    /// flag bitmap, @see css::beans::PropertyAttribute
    sal_Int16                           nFlags;
    /// "member ID" to tell QueryValue/PutValue which property it is
    /// (when multiple properties map to the same nWID)
    sal_uInt8                           nMemberId;
    PropertyMoreFlags                   nMoreFlags;

    SfxItemPropertyMapEntry(std::u16string_view _aName, sal_uInt16 _nWID, css::uno::Type const & _rType,
                               sal_Int16 _nFlags, sal_uInt8 const _nMemberId, PropertyMoreFlags _nMoreFlags = PropertyMoreFlags::NONE)
        : aName(      _aName )
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

struct SfxItemPropertySimpleEntry
{
    css::uno::Type                      aType;
    sal_uInt16                          nWID;
    /// flag bitmap, @see css::beans::PropertyAttribute
    sal_Int16                           nFlags;
    sal_uInt8                           nMemberId;
    PropertyMoreFlags                   nMoreFlags = PropertyMoreFlags::NONE;

    SfxItemPropertySimpleEntry()
        : nWID( 0 )
        , nFlags( 0 )
        , nMemberId( 0 )
        {
        }

    SfxItemPropertySimpleEntry(sal_uInt16 _nWID, css::uno::Type const & _rType,
                               sal_Int16 _nFlags)
        : aType(     _rType )
        , nWID(      _nWID )
        , nFlags(    _nFlags )
        , nMemberId( 0 )
        {
            assert(_nFlags <= 0x1ff );
        }

    SfxItemPropertySimpleEntry( const SfxItemPropertyMapEntry* pMapEntry )
        : aType( pMapEntry->aType )
        , nWID( pMapEntry->nWID )
        , nFlags( pMapEntry->nFlags )
        , nMemberId( pMapEntry->nMemberId )
        , nMoreFlags( pMapEntry->nMoreFlags )
        {
        }

};
struct SfxItemPropertyNamedEntry : public SfxItemPropertySimpleEntry
{
    OUString sName;
    SfxItemPropertyNamedEntry( const OUString& rName, const SfxItemPropertySimpleEntry& rSimpleEntry)
        : SfxItemPropertySimpleEntry( rSimpleEntry )
        , sName( rName )
{
}

};
typedef std::vector< SfxItemPropertyNamedEntry > PropertyEntryVector_t;
class SfxItemPropertyMap_Impl;
class SVL_DLLPUBLIC SfxItemPropertyMap
{
    std::unique_ptr<SfxItemPropertyMap_Impl> m_pImpl;
public:
    SfxItemPropertyMap( const SfxItemPropertyMapEntry* pEntries );
    SfxItemPropertyMap( const SfxItemPropertyMap& rSource );
    ~SfxItemPropertyMap();

    const SfxItemPropertySimpleEntry*  getByName( std::u16string_view rName ) const;
    css::uno::Sequence< css::beans::Property > const & getProperties() const;
    /// @throws css::beans::UnknownPropertyException
    css::beans::Property getPropertyByName( const OUString & rName ) const;
    bool hasPropertyByName( std::u16string_view rName ) const;

    void mergeProperties( const css::uno::Sequence< css::beans::Property >& rPropSeq );
    PropertyEntryVector_t getPropertyEntries() const;
    sal_uInt32 getSize() const;

};

class SVL_DLLPUBLIC SfxItemPropertySet final
{
    SfxItemPropertyMap                                        m_aMap;
    mutable css::uno::Reference<css::beans::XPropertySetInfo> m_xInfo;

public:
                            SfxItemPropertySet( const SfxItemPropertyMapEntry *pMap ) :
                                m_aMap(pMap) {}
                            ~SfxItemPropertySet();

    /// @throws css::uno::RuntimeException
    void getPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
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
    void                setPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
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
        getPropertyState(const SfxItemPropertySimpleEntry& rEntry, const SfxItemSet& rSet) const
                                    throw();

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
    SfxItemPropertySetInfo(const SfxItemPropertyMapEntry *pEntries );
    virtual ~SfxItemPropertySetInfo() override;

    virtual css::uno::Sequence< css::beans::Property > SAL_CALL
        getProperties(  ) override;

    virtual css::beans::Property SAL_CALL
        getPropertyByName( const OUString& aName ) override;

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const OUString& Name ) override;

};

// workaround for incremental linking bugs in MSVC2015
class SAL_DLLPUBLIC_TEMPLATE SfxExtItemPropertySetInfo_Base : public cppu::WeakImplHelper< css::beans::XPropertySetInfo > {};

class SVL_DLLPUBLIC SfxExtItemPropertySetInfo final : public SfxExtItemPropertySetInfo_Base
{
    SfxItemPropertyMap aExtMap;
public:
                            SfxExtItemPropertySetInfo(
                                const SfxItemPropertyMapEntry *pMap,
                                const css::uno::Sequence<css::beans::Property>& rPropSeq );
                            virtual ~SfxExtItemPropertySetInfo() override;

    virtual css::uno::Sequence< css::beans::Property > SAL_CALL
        getProperties(  ) override;

    virtual css::beans::Property SAL_CALL
        getPropertyByName( const OUString& aName ) override;

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const OUString& Name ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
