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

    typedef std::map< ::rtl::OUString, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > > tTableMap;//GradientTable, HatchTable etc.
    tTableMap   m_aTableMap;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHART2MODELCONTACT_HXX
#endif
