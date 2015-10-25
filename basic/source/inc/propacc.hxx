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
#ifndef INCLUDED_BASIC_SOURCE_INC_PROPACC_HXX
#define INCLUDED_BASIC_SOURCE_INC_PROPACC_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <cppuhelper/implbase.hxx>

#include <vector>

typedef ::std::vector<css::beans::PropertyValue> SbPropertyValueArr_Impl;

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
    virtual                 ~SbPropertyValues();

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL   setPropertyValue(
                                const OUString& aPropertyName,
                                const css::uno::Any& aValue)
                                throw (css::beans::UnknownPropertyException,
                                css::beans::PropertyVetoException,
                                css::lang::IllegalArgumentException,
                                css::lang::WrappedTargetException,
                                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw(  css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener(
                                const OUString& aPropertyName,
                                const css::uno::Reference< css::beans::XPropertyChangeListener >& )
                                throw (std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener(
                                const OUString& aPropertyName,
                                const css::uno::Reference< css::beans::XPropertyChangeListener >& )
                                throw (std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener(
                                const OUString& aPropertyName,
                                const css::uno::Reference< css::beans::XVetoableChangeListener >& )
                                throw (std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener(
                                const OUString& aPropertyName,
                                const css::uno::Reference< css::beans::XVetoableChangeListener >& )
                                throw (std::exception) override;

    // XPropertyAccess
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPropertyValues() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValues(const css::uno::Sequence< css::beans::PropertyValue >& PropertyValues_) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
};

class StarBASIC;
class SbxArray;

void RTL_Impl_CreatePropertySet( StarBASIC* pBasic, SbxArray& rPar, bool bWrite );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
