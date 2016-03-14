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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_NETCHARTTYPE_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_NETCHARTTYPE_HXX

#include "ChartType.hxx"

namespace chart
{

class NetChartType_Base : public ChartType
{
public:
    explicit NetChartType_Base( css::uno::Reference< css::uno::XComponentContext > const & xContext );
    virtual ~NetChartType_Base();

protected:
    explicit NetChartType_Base( const NetChartType_Base & rOther );

    // ____ XChartType ____
    virtual css::uno::Reference< css::chart2::XCoordinateSystem > SAL_CALL
        createCoordinateSystem( ::sal_Int32 DimensionCount )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override;

    // ____ OPropertySet ____
    virtual css::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(css::beans::UnknownPropertyException) override;

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;
};

class NetChartType : public NetChartType_Base
{
public:
    explicit NetChartType( css::uno::Reference< css::uno::XComponentContext > const & xContext );
    virtual ~NetChartType();

    virtual OUString SAL_CALL
        getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL
        supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) override;
    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString >
        getSupportedServiceNames_Static();

protected:
    explicit NetChartType( const NetChartType & rOther );

    // ____ XChartType ____
    virtual OUString SAL_CALL getChartType()
        throw (css::uno::RuntimeException, std::exception) override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone()
        throw (css::uno::RuntimeException, std::exception) override;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_NETCHARTTYPE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
