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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_MAIN_VLEGEND_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_MAIN_VLEGEND_HXX

#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "ChartModel.hxx"

#include <vector>

namespace chart
{

class LegendEntryProvider;

/**
*/

class VLegend
{
public:
    VLegend( const css::uno::Reference< css::chart2::XLegend > & xLegend,
             const css::uno::Reference< css::uno::XComponentContext > & xContext,
             const std::vector< LegendEntryProvider* >& rLegendEntryProviderList,
             const css::uno::Reference< css::drawing::XShapes >& xTargetPage,
             const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory,
             ChartModel& rModel  );

    void setDefaultWritingMode( sal_Int16 nDefaultWritingMode );

    void createShapes( const css::awt::Size & rAvailableSpace,
                       const css::awt::Size & rPageSize );

    /** Sets the position according to its internal anchor.

        @param rOutAvailableSpace
            is modified by the method, if the legend is in a standard position,
            such that the space allocated by the legend is removed from it.

        @param rReferenceSize
            is used to calculate the offset (default 2%) from the edge.
     */
    void changePosition(
        css::awt::Rectangle & rOutAvailableSpace,
        const css::awt::Size & rReferenceSize );

    static bool isVisible(
        const css::uno::Reference< css::chart2::XLegend > & xLegend );

private:
    css::uno::Reference< css::drawing::XShapes >            m_xTarget;
    css::uno::Reference< css::lang::XMultiServiceFactory>   m_xShapeFactory;
    css::uno::Reference< css::chart2::XLegend >             m_xLegend;
    css::uno::Reference< css::drawing::XShape >             m_xShape;

    ChartModel& mrModel;

    css::uno::Reference< css::uno::XComponentContext >      m_xContext;

    std::vector< LegendEntryProvider* >         m_aLegendEntryProviderList;

    sal_Int16 m_nDefaultWritingMode;//to be used when writing mode is set to page
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
