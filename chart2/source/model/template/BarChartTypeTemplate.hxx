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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_BARCHARTTYPETEMPLATE_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_BARCHARTTYPETEMPLATE_HXX

#include "OPropertySet.hxx"
#include "MutexContainer.hxx"
#include <comphelper/uno3.hxx>

#include "ChartTypeTemplate.hxx"
#include "StackMode.hxx"

namespace chart
{

class BarChartTypeTemplate :
        public MutexContainer,
        public ChartTypeTemplate,
        public ::property::OPropertySet
{
public:
    enum BarDirection
    {
        HORIZONTAL,
        VERTICAL
    };

    explicit BarChartTypeTemplate(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext,
        const OUString & rServiceName,
        StackMode eStackMode,
        BarDirection eDirection,
        sal_Int32 nDim = 2 );
    virtual ~BarChartTypeTemplate();

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException) override;
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XChartTypeTemplate ____
    virtual sal_Bool SAL_CALL matchesTemplate(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram,
        sal_Bool bAdaptProperties )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType > SAL_CALL
        getChartTypeForNewSeries( const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartType > >& aFormerlyUsedChartTypes )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL applyStyle(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries,
        ::sal_Int32 nChartTypeGroupIndex,
        ::sal_Int32 nSeriesIndex,
        ::sal_Int32 nSeriesCount )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL resetStyles(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ ChartTypeTemplate ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >
                getChartTypeForIndex( sal_Int32 nChartTypeIndex ) override;
    virtual sal_Int32 getDimension() const override;
    virtual StackMode getStackMode( sal_Int32 nChartTypeIndex ) const override;
    virtual bool isSwapXAndY() const override;

    virtual void createCoordinateSystems(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystemContainer > & xCooSysCnt ) override;

private:
    StackMode          m_eStackMode;
    BarDirection       m_eBarDirection;
    sal_Int32          m_nDim;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_BARCHARTTYPETEMPLATE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
