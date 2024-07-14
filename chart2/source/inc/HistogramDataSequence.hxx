/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

// helper classes
#include <comphelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer2.hxx>
#include <comphelper/proparrhlp.hxx>
#include <rtl/ref.hxx>

#include "ModifyListenerHelper.hxx"

// interfaces and types
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifiable.hpp>

namespace chart
{
typedef ::comphelper::WeakComponentImplHelper<
    css::chart2::data::XDataSequence, css::chart2::data::XNumericalDataSequence,
    css::chart2::data::XTextualDataSequence, css::util::XCloneable, css::util::XModifyBroadcaster,
    css::lang::XServiceInfo>
    HistogramDataSequence_Base;

class HistogramDataSequence final
    : public HistogramDataSequence_Base,
      public ::comphelper::OPropertyContainer2,
      public ::comphelper::OPropertyArrayUsageHelper<HistogramDataSequence>
{
public:
    HistogramDataSequence();
    virtual ~HistogramDataSequence() override;

    /// declare XServiceInfo methods
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    /// merge XInterface implementations
    DECLARE_XINTERFACE()

    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

    // XPropertySet
    /// @see css::beans::XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo() override;
    /// @see ::comphelper::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& getInfoHelper() override;
    /// @see ::comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

    // XDataSequence
    css::uno::Sequence<css::uno::Any> SAL_CALL getData() override;
    OUString SAL_CALL getSourceRangeRepresentation() override;
    css::uno::Sequence<OUString>
        SAL_CALL generateLabel(css::chart2::data::LabelOrigin nLabelOrigin) override;
    sal_Int32 SAL_CALL getNumberFormatKeyByIndex(sal_Int32 nIndex) override;

    // XNumericalDataSequence
    /// @see css::chart::data::XNumericalDataSequence
    virtual css::uno::Sequence<double> SAL_CALL getNumericalData() override;

    // XTextualDataSequence
    /// @see css::chart::data::XTextualDataSequence
    virtual css::uno::Sequence<OUString> SAL_CALL getTextualData() override;

    // XCloneable
    virtual css::uno::Reference<css::util::XCloneable> SAL_CALL createClone() override;

    // XModifyBroadcaster
    virtual void SAL_CALL
    addModifyListener(const css::uno::Reference<css::util::XModifyListener>& aListener) override;
    virtual void SAL_CALL
    removeModifyListener(const css::uno::Reference<css::util::XModifyListener>& aListener) override;

    void setLabels(css::uno::Sequence<OUString> const& xLabels) { mxLabels = xLabels; }

    void setValues(css::uno::Sequence<double> const& xValues) { mxValues = xValues; }

private:
    rtl::Reference<chart::ModifyEventForwarder> m_xModifyEventForwarder;
    OUString m_sRole;
    css::uno::Sequence<OUString> mxLabels;
    css::uno::Sequence<double> mxValues;
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
