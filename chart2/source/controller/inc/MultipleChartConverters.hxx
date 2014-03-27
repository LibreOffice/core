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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_MULTIPLECHARTCONVERTERS_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_MULTIPLECHARTCONVERTERS_HXX

#include "MultipleItemConverter.hxx"
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <memory>

class SdrModel;

namespace chart
{
namespace wrapper
{

class AllAxisItemConverter : public ::comphelper::MultipleItemConverter
{
public:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    AllAxisItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual ~AllAxisItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const SAL_OVERRIDE;
};

class AllGridItemConverter : public ::comphelper::MultipleItemConverter
{
public:
    AllGridItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory );
    virtual ~AllGridItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const SAL_OVERRIDE;
};

class AllDataLabelItemConverter : public ::comphelper::MultipleItemConverter
{
public:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    AllDataLabelItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual ~AllDataLabelItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const SAL_OVERRIDE;
};

class AllTitleItemConverter : public ::comphelper::MultipleItemConverter
{
public:
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    AllTitleItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    virtual ~AllTitleItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const SAL_OVERRIDE;
};

class AllSeriesStatisticsConverter : public ::comphelper::MultipleItemConverter
{
public:
    AllSeriesStatisticsConverter(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool );
    virtual ~AllSeriesStatisticsConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const SAL_OVERRIDE;
};

} //  namespace wrapper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_INC_MULTIPLECHARTCONVERTERS_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
