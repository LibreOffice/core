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

#include <com/sun/star/chart2/data/XLabeledDataSequence2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "ModifyListenerHelper.hxx"

namespace com::sun::star::chart2::data { class XDataSequence; }

namespace chart
{

namespace impl
{
typedef cppu::WeakImplHelper<
        css::chart2::data::XLabeledDataSequence2,
        css::lang::XServiceInfo >
    LabeledDataSequence_Base;
}

class LabeledDataSequence final :
        public impl::LabeledDataSequence_Base
{
public:
    explicit LabeledDataSequence();
    explicit LabeledDataSequence(const LabeledDataSequence &);
    explicit LabeledDataSequence(
        css::uno::Reference< css::chart2::data::XDataSequence > xValues );
    explicit LabeledDataSequence(
        css::uno::Reference< css::chart2::data::XDataSequence > xValues,
        css::uno::Reference< css::chart2::data::XDataSequence > xLabels );

    virtual ~LabeledDataSequence() override;

    /// declare XServiceInfo methods
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // ____ XLabeledDataSequence ____
    virtual css::uno::Reference< css::chart2::data::XDataSequence > getValues() override;
    virtual void setValues(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xSequence ) override;
    virtual css::uno::Reference< css::chart2::data::XDataSequence > getLabel() override;
    virtual void setLabel(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xSequence ) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

private:
    css::uno::Reference< css::chart2::data::XDataSequence > m_xData;
    css::uno::Reference< css::chart2::data::XDataSequence > m_xLabel;

    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
