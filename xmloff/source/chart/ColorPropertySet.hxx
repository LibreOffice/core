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
#include <tools/color.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>

namespace xmloff::chart
{

class ColorPropertySet : public ::cppu::WeakImplHelper<
        css::beans::XPropertySet,
        css::beans::XPropertyState >
{
public:
    explicit ColorPropertySet( ::Color nColor );
    virtual ~ColorPropertySet() override;

protected:
    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(
        const OUString& aPropertyName,
        const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
        const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener(
        const OUString& aPropertyName,
        const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener(
        const OUString& aPropertyName,
        const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener(
        const OUString& PropertyName,
        const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener(
        const OUString& PropertyName,
        const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // ____ XPropertyState ____
    virtual css::beans::PropertyState SAL_CALL getPropertyState(
        const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates(
        const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault(
        const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault(
        const OUString& aPropertyName ) override;

private:
    css::uno::Reference< css::beans::XPropertySetInfo > m_xInfo;
    static constexpr OUStringLiteral g_aColorPropName = u"FillColor";
    Color     m_nColor;
    Color     m_nDefaultColor;
};

} //  namespace xmloff::chart

// XMLOFF_COLORPROPERTYSET_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
