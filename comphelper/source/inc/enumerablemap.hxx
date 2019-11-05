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

#include <comphelper/componentbase.hxx>
#include <comphelper/anycompare.hxx>
#include <cppuhelper/compbase3.hxx>

#include <com/sun/star/beans/Pair.hpp>
#include <com/sun/star/container/XEnumerableMap.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <map>
#include <memory>

namespace comphelper
{
class MapEnumerator;

typedef std::map<css::uno::Any, css::uno::Any, LessPredicateAdapter> KeyedValues;
struct MapData
{
    css::uno::Type m_aKeyType;
    css::uno::Type m_aValueType;
    std::unique_ptr<KeyedValues> m_pValues;
    std::shared_ptr<IKeyPredicateLess> m_pKeyCompare;
    bool m_bMutable;
    std::vector<MapEnumerator*> m_aModListeners;

    MapData()
        : m_bMutable(true)
    {
    }

    MapData(const MapData& _source)
        : m_aKeyType(_source.m_aKeyType)
        , m_aValueType(_source.m_aValueType)
        , m_pValues(new KeyedValues(*_source.m_pValues))
        , m_pKeyCompare(_source.m_pKeyCompare)
        , m_bMutable(false)
        , m_aModListeners()
    {
    }

private:
    MapData& operator=(const MapData& _source) = delete;
};

// EnumerableMap

typedef ::cppu::WeakAggComponentImplHelper3<css::lang::XInitialization,
                                            css::container::XEnumerableMap, css::lang::XServiceInfo>
    Map_IFace;

class EnumerableMap : public Map_IFace, public ComponentBase
{
protected:
    EnumerableMap();
    virtual ~EnumerableMap() override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& aArguments) override;

    // XEnumerableMap
    virtual css::uno::Reference<css::container::XEnumeration>
        SAL_CALL createKeyEnumeration(sal_Bool Isolated) override;
    virtual css::uno::Reference<css::container::XEnumeration>
        SAL_CALL createValueEnumeration(sal_Bool Isolated) override;
    virtual css::uno::Reference<css::container::XEnumeration>
        SAL_CALL createElementEnumeration(sal_Bool Isolated) override;

    // XMap
    virtual css::uno::Type SAL_CALL getKeyType() override;
    virtual css::uno::Type SAL_CALL getValueType() override;
    virtual void SAL_CALL clear() override;
    virtual sal_Bool SAL_CALL containsKey(const css::uno::Any& _key) override;
    virtual sal_Bool SAL_CALL containsValue(const css::uno::Any& _value) override;
    virtual css::uno::Any SAL_CALL get(const css::uno::Any& _key) override;
    virtual css::uno::Any SAL_CALL put(const css::uno::Any& _key,
                                       const css::uno::Any& _value) override;
    virtual css::uno::Any SAL_CALL remove(const css::uno::Any& _key) override;

    // XElementAccess (base of XMap)
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

public:
    // XServiceInfo, static version (used for component registration)
    static OUString getImplementationName_static();
    static css::uno::Sequence<OUString> getSupportedServiceNames_static();
    static css::uno::Reference<css::uno::XInterface>
    Create(const css::uno::Reference<css::uno::XComponentContext>&);

private:
    void impl_initValues_throw(
        const css::uno::Sequence<css::beans::Pair<css::uno::Any, css::uno::Any>>& _initialValues);

    /// throws an IllegalTypeException if the given value is not compatible with our ValueType
    void impl_checkValue_throw(const css::uno::Any& _value) const;
    void impl_checkKey_throw(const css::uno::Any& _key) const;
    void impl_checkNaN_throw(const css::uno::Any& _keyOrValue,
                             const css::uno::Type& _keyOrValueType) const;
    void impl_checkMutable_throw() const;

private:
    ::osl::Mutex m_aMutex;
    MapData m_aData;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
