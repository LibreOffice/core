/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "OPropertySet.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/XDataPointCustomLabelField.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include "ModifyListenerHelper.hxx"
#include "PropertyHelper.hxx"

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper<
    css::chart2::XDataPointCustomLabelField, // inherits from XFormattedString2
    css::lang::XServiceInfo,
    css::util::XCloneable,
    css::util::XModifyBroadcaster,
    css::util::XModifyListener >
    FormattedString_Base;
}

class FormattedString final :
    public impl::FormattedString_Base,
    public ::property::OPropertySet
{
public:
    explicit FormattedString();
    virtual ~FormattedString() override;

    /// declare XServiceInfo methods
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

    virtual void setPropertyValue(const OUString& p1, const cpo::uno::Any& p2) override
        { ::property::OPropertySet::setPropertyValue(p1, p2); }
    virtual cpo::uno::Any getPropertyValue(const OUString& p1) override
        { return ::property::OPropertySet::getPropertyValue(p1); }
    virtual void addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
        { ::property::OPropertySet::addPropertyChangeListener(p1, p2); }
    virtual void removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
        { ::property::OPropertySet::removePropertyChangeListener(p1, p2); }
    virtual void addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
        { ::property::OPropertySet::addVetoableChangeListener(p1, p2); }
    virtual void removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
        { ::property::OPropertySet::removeVetoableChangeListener(p1, p2); }

    explicit FormattedString( const FormattedString & rOther );

    // ____ XFormattedString ____
    virtual OUString getString() override;
    virtual void setString( const OUString& String ) override;

    // ____ XDataPointCustomLabelField ____
    virtual css::chart2::DataPointCustomLabelFieldType getFieldType() override;
    virtual void
        setFieldType( const css::chart2::DataPointCustomLabelFieldType FieldType ) override;
    virtual OUString getGuid() override;
    void setGuid( const OUString& guid ) override;
    virtual bool getDataLabelsRange() override;
    virtual void setDataLabelsRange( bool dataLabelsRange ) override;
    virtual OUString getCellRange() override;
    virtual void setCellRange( const OUString& cellRange ) override;

    // ____ OPropertySet ____
    virtual void GetDefaultValue( sal_Int32 nHandle, cpo::uno::Any& rAny ) const override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
        getPropertySetInfo() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // ____ XModifyListener ____
    virtual void modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void disposing(
        const css::lang::EventObject& Source ) override;

private:
    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    void fireModifyEvent();

    // ____ XFormattedString ____
    OUString m_aString;

    // ____ XDataPointCustomLabelField ____
    css::chart2::DataPointCustomLabelFieldType m_aType;
    OUString m_aGuid;
    OUString m_aCellRange;
    bool m_bDataLabelsRange;

    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;
};

const ::chart::tPropertyValueMap & StaticFormattedStringDefaults();

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
