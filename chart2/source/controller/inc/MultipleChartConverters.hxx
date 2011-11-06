/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
