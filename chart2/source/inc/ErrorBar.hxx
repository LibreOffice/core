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

#include "MutexContainer.hxx"
#include "charttoolsdllapi.hxx"

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>


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

class ErrorBar final :
        public MutexContainer,
        public impl::ErrorBar_Base
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
    OOO_DLLPUBLIC_CHARTTOOLS explicit ErrorBar();
    virtual ~ErrorBar() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& rPropName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates(
            const css::uno::Sequence< OUString >& rPropNames ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& rPropName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& rPropName ) override;

private:
    ErrorBar( const ErrorBar & rOther );

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    // ____ XDataSink ____
    virtual void SAL_CALL setData( const css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > >& aData ) override;

    // ____ XDataSource ____
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences() override;

    typedef std::vector< css::uno::Reference<
            css::chart2::data::XLabeledDataSequence > > tDataSequenceContainer;
    tDataSequenceContainer m_aDataSequences;

    css::uno::Reference< css::util::XModifyListener > m_xModifyEventForwarder;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
