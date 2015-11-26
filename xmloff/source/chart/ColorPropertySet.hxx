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
#ifndef XMLOFF_COLORPROPERTYSET_HXX
#define XMLOFF_COLORPROPERTYSET_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

namespace xmloff
{
namespace chart
{

class ColorPropertySet : public ::cppu::WeakImplHelper<
        css::beans::XPropertySet,
        css::beans::XPropertyState >
{
public:
    // if bFillColor == false, the color is a LineColor
    explicit ColorPropertySet( sal_Int32 nColor, bool bFillColor = true );
    virtual ~ColorPropertySet();

protected:
    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
        const OUString& aPropertyName,
        const css::uno::Any& aValue )
        throw (css::beans::UnknownPropertyException,
               css::beans::PropertyVetoException,
               css::lang::IllegalArgumentException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
        const OUString& PropertyName )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(
        const OUString& aPropertyName,
        const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(
        const OUString& aPropertyName,
        const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
        const OUString& PropertyName,
        const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
        const OUString& PropertyName,
        const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;

    // ____ XPropertyState ____
    virtual css::beans::PropertyState SAL_CALL getPropertyState(
        const OUString& PropertyName )
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates(
        const css::uno::Sequence< OUString >& aPropertyName )
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault(
        const OUString& PropertyName )
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault(
        const OUString& aPropertyName )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference< css::beans::XPropertySetInfo > m_xInfo;
    OUString  m_aColorPropName;
    sal_Int32        m_nColor;
    bool             m_bIsFillColor;
    sal_Int32        m_nDefaultColor;
};

} //  namespace chart
} //  namespace xmloff

// XMLOFF_COLORPROPERTYSET_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
