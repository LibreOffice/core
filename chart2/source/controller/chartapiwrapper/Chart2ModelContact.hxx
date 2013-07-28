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
#ifndef CHART_CHART2MODELCONTACT_HXX
#define CHART_CHART2MODELCONTACT_HXX

#include <chartview/ExplicitScaleValues.hxx>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XTitle.hpp>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <map>

namespace chart
{
class ExplicitValueProvider;

namespace wrapper
{

class Chart2ModelContact
{
public:
    Chart2ModelContact( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::uno::XComponentContext >& xContext );
    virtual ~Chart2ModelContact();

public:
    void setModel( const ::com::sun::star::uno::Reference<
                       ::com::sun::star::frame::XModel >& xChartModel );
    void clear();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel > getChartModel() const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartDocument > getChart2Document() const;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDiagram > getChart2Diagram() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > getDrawPage();

    /** get the current values calculated for an axis in the current view in
        case properties are 'auto'.
     */
    sal_Bool getExplicitValuesForAxis(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XAxis > & xAxis,
        ExplicitScaleData &  rOutExplicitScale,
        ExplicitIncrementData & rOutExplicitIncrement );

    sal_Int32 getExplicitNumberFormatKeyForAxis(
            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis );

    sal_Int32 getExplicitNumberFormatKeyForSeries(
            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries );

    /** Returns the size of the page in logic coordinates.  This value is used
        for setting an appropriate "ReferencePageSize" for FontHeights.
     */
    ::com::sun::star::awt::Size GetPageSize() const;

    /** calculates the current axes title sizes and substract that space them from the given recangle
     */
    ::com::sun::star::awt::Rectangle SubstractAxisTitleSizes( const ::com::sun::star::awt::Rectangle& rPositionRect );

    /** Returns the position and size of the diagram in logic coordinates (100th mm) including
        the space used for axes including axes titles.
     */
    ::com::sun::star::awt::Rectangle GetDiagramRectangleIncludingTitle() const;

    /** Returns the position and size of the diagram in logic coordinates (100th mm) including
        the space used for axes excluding axes titles.
     */
    ::com::sun::star::awt::Rectangle GetDiagramRectangleIncludingAxes() const;

    /** Returns the position and size of the diagram in logic coordinates (100th mm) excluding
        the space used for axes (inner plot area).
     */
    ::com::sun::star::awt::Rectangle GetDiagramRectangleExcludingAxes() const;

    /** Returns the size of the object in logic coordinates.
     */
    ::com::sun::star::awt::Size GetLegendSize() const;

    /** Returns the position of the object in logic coordinates.
     */
    ::com::sun::star::awt::Point GetLegendPosition() const;

    /** Returns the size of the object in logic coordinates.
     */
    ::com::sun::star::awt::Size GetTitleSize( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XTitle > & xTitle ) const;

    /** Returns the position of the object in logic coordinates.
     */
    ::com::sun::star::awt::Point GetTitlePosition( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XTitle > & xTitle ) const;

    /** Returns the size of the object in logic coordinates.
     */
    ::com::sun::star::awt::Size GetAxisSize( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XAxis > & xAxis ) const;

    /** Returns the position of the object in logic coordinates.
     */
    ::com::sun::star::awt::Point GetAxisPosition( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::chart2::XAxis > & xAxis ) const;

private: //methods
    ExplicitValueProvider* getExplicitValueProvider() const;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XUnoTunnel > getChartView() const;

public: //member
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;

private: //member
    ::com::sun::star::uno::WeakReference<
        ::com::sun::star::frame::XModel >   m_xChartModel;

    mutable ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XUnoTunnel >        m_xChartView;

    typedef std::map< OUString, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > > tTableMap;//GradientTable, HatchTable etc.
    tTableMap   m_aTableMap;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHART2MODELCONTACT_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
