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
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/implbase1.hxx>
#include <svl/itemset.hxx>
#include <svl/svldllapi.h>
#include <vector>
#include <memory>

/// map a property between beans::XPropertySet and SfxPoolItem
struct SfxItemPropertyMapEntry
{
    OUString                            aName; ///< name of property
    sal_uInt16                          nWID;  ///< WhichId of SfxPoolItem
    css::uno::Type                      aType; ///< UNO type of property
    /// flag bitmap, @see css::beans::PropertyAttribute
    long                                nFlags;
    /// "member ID" to tell QueryValue/PutValue which property it is
    /// (when multiple properties map to the same nWID)
    sal_uInt8                           nMemberId;

};

struct SfxItemPropertySimpleEntry
{
    sal_uInt16                          nWID;
    css::uno::Type                      aType;
    long                                nFlags;
    sal_uInt8                           nMemberId;

    SfxItemPropertySimpleEntry()
        : nWID( 0 )
        , nFlags( 0 )
        , nMemberId( 0 )
        {
        }

    SfxItemPropertySimpleEntry(sal_uInt16 _nWID, css::uno::Type const & _rType,
                               long _nFlags)
        : nWID(      _nWID )
        , aType(     _rType )
        , nFlags(    _nFlags )
        , nMemberId( 0 )
        {
        }

    SfxItemPropertySimpleEntry( const SfxItemPropertyMapEntry* pMapEntry )
        : nWID( pMapEntry->nWID )
        , aType( pMapEntry->aType )
        , nFlags( pMapEntry->nFlags )
        , nMemberId( pMapEntry->nMemberId )
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

    const SfxItemPropertySimpleEntry*  getByName( const OUString &rName ) const;
    css::uno::Sequence< css::beans::Property > getProperties() const;
    css::beans::Property getPropertyByName( const OUString & rName ) const
        throw( css::beans::UnknownPropertyException );
    bool hasPropertyByName( const OUString& rName ) const;

    void mergeProperties( const css::uno::Sequence< css::beans::Property >& rPropSeq );
    PropertyEntryVector_t getPropertyEntries() const;
    sal_uInt32 getSize() const;

};

class SVL_DLLPUBLIC SfxItemPropertySet
{
    SfxItemPropertyMap                                        m_aMap;
    mutable css::uno::Reference<css::beans::XPropertySetInfo> m_xInfo;

public:
                            SfxItemPropertySet( const SfxItemPropertyMapEntry *pMap ) :
                                m_aMap(pMap) {}
                            virtual ~SfxItemPropertySet();

    void getPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
                                          const SfxItemSet& rSet,
                                          css::uno::Any& rAny) const
                                          throw(css::uno::RuntimeException);
    void getPropertyValue( const OUString &rName,
                                            const SfxItemSet& rSet,
                                            css::uno::Any& rAny) const
                                            throw(css::uno::RuntimeException,
                                                    css::beans::UnknownPropertyException);
    css::uno::Any
        getPropertyValue( const OUString &rName,
                                            const SfxItemSet& rSet ) const
                                            throw(css::uno::RuntimeException,
                                                    css::beans::UnknownPropertyException);
    void                setPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
                                          const css::uno::Any& aVal,
                                          SfxItemSet& rSet ) const
                                          throw(css::uno::RuntimeException,
                                                  css::lang::IllegalArgumentException);
    void                  setPropertyValue( const OUString& rPropertyName,
                                            const css::uno::Any& aVal,
                                            SfxItemSet& rSet ) const
                                            throw(css::uno::RuntimeException,
                                                    css::lang::IllegalArgumentException,
                                                    css::beans::UnknownPropertyException);

    css::beans::PropertyState
        getPropertyState(const OUString& rName, const SfxItemSet& rSet)const
                                    throw(css::beans::UnknownPropertyException);
    css::beans::PropertyState
        getPropertyState(const SfxItemPropertySimpleEntry& rEntry, const SfxItemSet& rSet) const
                                    throw();

    css::uno::Reference<css::beans::XPropertySetInfo> const &
        getPropertySetInfo() const;
    const SfxItemPropertyMap& getPropertyMap() const {return m_aMap;}
};

struct SfxItemPropertySetInfo_Impl;
class SVL_DLLPUBLIC SfxItemPropertySetInfo : public cppu::WeakImplHelper1<css::beans::XPropertySetInfo>
{
    std::unique_ptr<SfxItemPropertySetInfo_Impl> m_pImpl;

public:
    SfxItemPropertySetInfo(const SfxItemPropertyMap &rMap );
    SfxItemPropertySetInfo(const SfxItemPropertyMapEntry *pEntries );
    virtual ~SfxItemPropertySetInfo();

    virtual css::uno::Sequence< css::beans::Property > SAL_CALL
        getProperties(  )
            throw(css::uno::RuntimeException, std::exception) override;

    virtual css::beans::Property SAL_CALL
        getPropertyByName( const OUString& aName )
            throw(css::beans::UnknownPropertyException,
                    css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const OUString& Name )
            throw(css::uno::RuntimeException, std::exception) override;

};

class SVL_DLLPUBLIC SfxExtItemPropertySetInfo: public cppu::WeakImplHelper1<css::beans::XPropertySetInfo >
{
    SfxItemPropertyMap aExtMap;
public:
                            SfxExtItemPropertySetInfo(
                                const SfxItemPropertyMapEntry *pMap,
                                const css::uno::Sequence<css::beans::Property>& rPropSeq );
                            virtual ~SfxExtItemPropertySetInfo();

    virtual css::uno::Sequence< css::beans::Property > SAL_CALL
        getProperties(  )
            throw(css::uno::RuntimeException, std::exception) override;

    virtual css::beans::Property SAL_CALL
        getPropertyByName( const OUString& aName )
            throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const OUString& Name )
            throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
