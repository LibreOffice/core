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
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/chart2/data/XLabeledDataSequence2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com::sun::star::chart2::data { class XDataSequence; }
namespace com::sun::star::util { class XCloneable; }
namespace com::sun::star::util { class XModifyListener; }

namespace chart
{

namespace impl
{
typedef cppu::WeakImplHelper<
        css::chart2::data::XLabeledDataSequence2,
        css::lang::XServiceInfo >
    LabeledDataSequence_Base;
}

class LabeledDataSequence :
        public MutexContainer,
        public impl::LabeledDataSequence_Base
{
public:
    explicit LabeledDataSequence();
    explicit LabeledDataSequence(
        const css::uno::Reference< css::chart2::data::XDataSequence > & rValues );
    explicit LabeledDataSequence(
        const css::uno::Reference< css::chart2::data::XDataSequence > & rValues,
        const css::uno::Reference< css::chart2::data::XDataSequence > & rLabels );

    virtual ~LabeledDataSequence() override;

    /// declare XServiceInfo methods
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

protected:
    // ____ XLabeledDataSequence ____
    virtual css::uno::Reference< css::chart2::data::XDataSequence > SAL_CALL getValues() override;
    virtual void SAL_CALL setValues(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xSequence ) override;
    virtual css::uno::Reference< css::chart2::data::XDataSequence > SAL_CALL getLabel() override;
    virtual void SAL_CALL setLabel(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xSequence ) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

private:
    css::uno::Reference< css::chart2::data::XDataSequence > m_xData;
    css::uno::Reference< css::chart2::data::XDataSequence > m_xLabel;

    css::uno::Reference< css::util::XModifyListener >       m_xModifyEventForwarder;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
