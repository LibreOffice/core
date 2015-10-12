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
#ifndef INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_STOCKCHARTTYPETEMPLATE_HXX
#define INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_STOCKCHARTTYPETEMPLATE_HXX

#include "ChartTypeTemplate.hxx"
#include "OPropertySet.hxx"
#include "MutexContainer.hxx"
#include <comphelper/uno3.hxx>

namespace chart
{

class StockChartTypeTemplate :
        public MutexContainer,
        public ChartTypeTemplate,
        public ::property::OPropertySet
{
public:
    enum StockVariant
    {
        LOW_HI_CLOSE,
        OPEN_LOW_HI_CLOSE,
        VOL_LOW_HI_CLOSE,
        VOL_OPEN_LOW_HI_CLOSE
    };

    /** CTOR

        @param bJapaneseStyle
            If true, the candlesticks are drawn as solid white or black boxes
            depending on rising or falling stock-values.  Otherwise the
            open-value will be drawn as a small line at the left side of a
            straight vertical line, and the close-value on the right hand side.
     */
    explicit StockChartTypeTemplate(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext,
        const OUString & rServiceName,
        StockVariant eVariant,
        bool bJapaneseStyle );
    virtual ~StockChartTypeTemplate();

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
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataInterpreter > SAL_CALL getDataInterpreter()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL applyStyle(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries,
        ::sal_Int32 nChartTypeIndex,
        ::sal_Int32 nSeriesIndex,
        ::sal_Int32 nSeriesCount )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL resetStyles(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ChartTypeTemplate
    virtual sal_Int32 getAxisCountByDimension( sal_Int32 nDimension ) override;

    // ____ ChartTypeTemplate ____
    virtual void createChartTypes(
            const ::com::sun::star::uno::Sequence<
                 ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries > > >& aSeriesSeq,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem > > & rCoordSys,
            const ::com::sun::star::uno::Sequence<
                  ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XChartType > > & aOldChartTypesSeq
            ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >
                getChartTypeForIndex( sal_Int32 nChartTypeIndex ) override;

private:
    // todo: deprecate this variable
    StockVariant  m_eStockVariant;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_MODEL_TEMPLATE_STOCKCHARTTYPETEMPLATE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
