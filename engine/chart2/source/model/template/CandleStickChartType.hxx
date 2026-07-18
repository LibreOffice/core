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

#include <ChartType.hxx>

namespace chart
{

class CandleStickChartType final : public ChartType
{
public:
    explicit CandleStickChartType();
    virtual ~CandleStickChartType() override;

    virtual OUString
        getImplementationName() override;
    virtual bool
        supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString >
        getSupportedServiceNames() override;

    virtual rtl::Reference<ChartType> cloneChartType() const override;

private:
    explicit CandleStickChartType( const CandleStickChartType & rOther );

    // ____ XChartType ____
    virtual OUString getChartType() override;
    virtual cpo::uno::Sequence< OUString >
        getSupportedMandatoryRoles() override;
    virtual cpo::uno::Sequence< OUString >
        getSupportedOptionalRoles() override;
    virtual OUString getRoleOfSequenceForSeriesLabel() override;

    // ____ OPropertySet ____
    virtual void GetDefaultValue( sal_Int32 nHandle, cpo::uno::Any& rAny ) const override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & getInfoHelper() override;

    // ____ OPropertySet ____
    virtual void setFastPropertyValue_NoBroadcast
        ( sal_Int32 nHandle,
          const cpo::uno::Any& rValue ) override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
        getPropertySetInfo() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > createClone() override;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
