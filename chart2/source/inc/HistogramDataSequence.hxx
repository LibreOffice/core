/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "ModifyListenerHelper.hxx"

#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/compbase.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/chart2/data/XTextualDataSequence.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

namespace chart
{
class ModifyEventForwarder;

typedef ::comphelper::WeakComponentImplHelper<
    css::chart2::data::XDataSequence, css::chart2::data::XNumericalDataSequence,
    css::chart2::data::XTextualDataSequence, css::util::XCloneable, css::util::XModifyBroadcaster,
    css::lang::XServiceInfo, css::util::XModifyListener>
    HistogramDataSequence_Base;

/** Lazy XDataSequence that yields either bin labels or bin frequencies derived
    from a raw data source. Recomputes when the source changes. */
class HistogramDataSequence final
    : public HistogramDataSequence_Base,
      public ::comphelper::OMutexAndBroadcastHelper,
      public ::comphelper::OPropertyContainer,
      public ::comphelper::OPropertyArrayUsageHelper<HistogramDataSequence>
{
public:
    using ::comphelper::WeakComponentImplHelperBase::disposing;
    explicit HistogramDataSequence(
        const css::uno::Reference<css::chart2::data::XDataSequence>& xRawData, bool bIsCategory,
        sal_Int32 nFrequencyType = 0, double fBinWidth = 0.0, sal_Int32 nBinCount = 0);
    virtual ~HistogramDataSequence() override;

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    /// declare XInterface methods
    DECLARE_XINTERFACE()

    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

    /// declare property methods
    /// @see css::beans::XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo() override;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
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
    /// @see css::util::XCloneable
    virtual css::uno::Reference<css::util::XCloneable> SAL_CALL createClone() override;

    // XModifyListener
    /// @see css::util::XModifyListener
    virtual void SAL_CALL
    addModifyListener(const css::uno::Reference<css::util::XModifyListener>& aListener) override;
    virtual void SAL_CALL
    removeModifyListener(const css::uno::Reference<css::util::XModifyListener>& aListener) override;
    virtual void SAL_CALL modified(const css::lang::EventObject& aEvent) override;
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

private:
    void ensureCalculated();

    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;
    OUString m_sRole;
    css::uno::Sequence<OUString> mxLabels;
    css::uno::Sequence<css::uno::Any> mxValues;
    css::uno::Reference<css::chart2::data::XDataSequence> m_xRawData;
    bool m_bIsCategory;
    bool m_bIsDirty;

    // Binning parameters captured at construction (from the owning HistogramChartType).
    sal_Int32 m_nFrequencyType;
    double m_fBinWidth;
    sal_Int32 m_nBinCount;
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */