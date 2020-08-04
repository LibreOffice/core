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
#pragma once

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <cppuhelper/implbase.hxx>

#include <vector>

typedef std::vector<css::beans::PropertyValue> SbPropertyValueArr_Impl;

typedef ::cppu::WeakImplHelper< css::beans::XPropertySet,
                                css::beans::XPropertyAccess > SbPropertyValuesHelper;


class SbPropertyValues:     public SbPropertyValuesHelper
{
    SbPropertyValueArr_Impl m_aPropVals;
    css::uno::Reference< css::beans::XPropertySetInfo > m_xInfo;

private:
    size_t GetIndex_Impl( const OUString &rPropName ) const;

public:
                            SbPropertyValues();
    virtual                 ~SbPropertyValues() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue(
                                const OUString& aPropertyName,
                                const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener(
                                const OUString& aPropertyName,
                                const css::uno::Reference< css::beans::XPropertyChangeListener >& ) override;
    virtual void SAL_CALL   removePropertyChangeListener(
                                const OUString& aPropertyName,
                                const css::uno::Reference< css::beans::XPropertyChangeListener >& ) override;
    virtual void SAL_CALL   addVetoableChangeListener(
                                const OUString& aPropertyName,
                                const css::uno::Reference< css::beans::XVetoableChangeListener >& ) override;
    virtual void SAL_CALL   removeVetoableChangeListener(
                                const OUString& aPropertyName,
                                const css::uno::Reference< css::beans::XVetoableChangeListener >& ) override;

    // XPropertyAccess
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPropertyValues() override;
    virtual void SAL_CALL setPropertyValues(const css::uno::Sequence< css::beans::PropertyValue >& PropertyValues_) override;
};

class SbxArray;

void RTL_Impl_CreatePropertySet( SbxArray& rPar );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
