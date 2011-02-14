/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CHART2_MULTIPLECHARTCONVERTERS_HXX
#define CHART2_MULTIPLECHARTCONVERTERS_HXX

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
    AllAxisItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    virtual ~AllAxisItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const;
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
    virtual const sal_uInt16 * GetWhichPairs() const;
};

class AllDataLabelItemConverter : public ::comphelper::MultipleItemConverter
{
public:
    AllDataLabelItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    virtual ~AllDataLabelItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const;
};

class AllTitleItemConverter : public ::comphelper::MultipleItemConverter
{
public:
    AllTitleItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xChartModel,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize =
            ::std::auto_ptr< ::com::sun::star::awt::Size >() );
    virtual ~AllTitleItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const;
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
    virtual const sal_uInt16 * GetWhichPairs() const;
};

} //  namespace wrapper
} //  namespace chart

// CHART2_MULTIPLECHARTCONVERTERS_HXX
#endif
