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

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include "ModifyListenerHelper.hxx"


#include <vector>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::util::XCloneable,
        css::util::XModifyBroadcaster,
        css::util::XModifyListener,
        css::chart2::data::XDataSource,
        css::chart2::data::XDataSink,
        css::beans::XPropertySet,
        css::beans::XPropertyState >
    ErrorBar_Base;
}

class ErrorBar final : public impl::ErrorBar_Base
{
private:
    OUString maDashName;
    css::drawing::LineDash maLineDash;
    sal_Int32 mnLineWidth;
    css::drawing::LineStyle meLineStyle;
    css::util::Color maLineColor;
    sal_Int16 mnLineTransparence;
    css::drawing::LineJoint meLineJoint;
    bool mbShowPositiveError;
    bool mbShowNegativeError;
    double mfPositiveError;
    double mfNegativeError;
    double mfWeight;
    sal_Int32 meStyle;

public:
    explicit ErrorBar();
    virtual ~ErrorBar() override;

    /// XServiceInfo declarations
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
        getPropertySetInfo() override;
    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XPropertyState
    virtual css::beans::PropertyState getPropertyState( const OUString& rPropName ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyState > getPropertyStates(
            const cpo::uno::Sequence< OUString >& rPropNames ) override;
    virtual void setPropertyToDefault( const OUString& rPropName ) override;
    virtual cpo::uno::Any getPropertyDefault( const OUString& rPropName ) override;

private:
    ErrorBar( const ErrorBar & rOther );

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

    // ____ XDataSink ____
    virtual void setData( const cpo::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > >& aData ) override;

    // ____ XDataSource ____
    virtual cpo::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > getDataSequences() override;

    typedef std::vector< css::uno::Reference<
            css::chart2::data::XLabeledDataSequence > > tDataSequenceContainer;
    tDataSequenceContainer m_aDataSequences;

    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
