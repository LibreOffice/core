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

#include "EditBase.hxx"
#include <limitedformats.hxx>


namespace frm
{

class ODateModel
                :public OEditBaseModel
                ,public OLimitedFormats
{
    css::uno::Any      m_aSaveValue;
    bool               m_bDateTimeField;

protected:
    virtual css::uno::Sequence< css::uno::Type> _getTypes() override;

public:
    ODateModel(
        const css::uno::Reference< css::uno::XComponentContext>& _rxFactory
    );
    ODateModel(
        const ODateModel* _pOriginal,
        const css::uno::Reference< css::uno::XComponentContext>& _rxFactory
    );
    virtual ~ODateModel() override;


    // XPersistObject
    virtual OUString SAL_CALL getServiceName() override;

    // css::beans::XPropertySet
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle ) const override;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(css::uno::Any& rConvertedValue, css::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const css::uno::Any& rValue ) override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return u"com.sun.star.form.ODateModel"_ustr; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XPropertySet
    using OEditBaseModel::getFastPropertyValue;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;

protected:
    // OBoundControlModel overridables
    virtual css::uno::Any
                            translateDbColumnToControlValue( ) override;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) override;

    virtual css::uno::Any
                            translateControlValueToExternalValue( ) const override;
    virtual css::uno::Sequence< css::uno::Type >
                            getSupportedBindingTypes() override;
    virtual css::uno::Any   translateExternalValueToControlValue( const css::uno::Any& _rExternalValue ) const override;

    virtual css::uno::Any   translateControlValueToValidatableValue( ) const override;

    virtual css::uno::Any   getDefaultForReset() const override;

    virtual void            resetNoBroadcast() override;

    virtual void            onConnectedDbColumn( const css::uno::Reference< css::uno::XInterface >& _rxForm ) override;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;
};

class ODateControl: public OBoundControl
{
protected:
    virtual css::uno::Sequence< css::uno::Type> _getTypes() override;

public:
    explicit ODateControl(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);
    DECLARE_UNO3_AGG_DEFAULTS(ODateControl, OBoundControl)

// css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName() override
    { return u"com.sun.star.form.ODateControl"_ustr; }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};


}   // namespace frm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
