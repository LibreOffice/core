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
#ifndef CHART_CHARTTYPETEMPLATE_HXX
#define CHART_CHARTTYPETEMPLATE_HXX

#include <cppuhelper/implbase2.hxx>
#include "ServiceMacros.hxx"
#include "DataInterpreter.hxx"
#include "StackMode.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XLegend.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>

#include <utility>

namespace chart
{

/** For creating diagrams and modifying existing diagrams.  A base class that
    implements XChartTypeTemplate and offers some tooling for classes that
    derive from this class.

    createDiagramByDataSource

    This does the following steps using some virtual helper-methods, that may be
    overloaded by derived classes:

    * creates an XDiagram via service-factory.

    * convert the given XDataSource to a sequence of XDataSeries using the
      method createDataSeries().  In this class the DataInterpreter helper class
      is used to create a standard interpretation (just y-values).

    * call applyDefaultStyle() for all XDataSeries in order to apply default
      styles.  In this class the series get the system-wide default colors as
      "Color" property.

    * call applyStyle() for applying chart-type specific styles to all series.
      The default implementation is empty.

    * call createCoordinateSystems() and apply them to the diagram.  As
      default one cartesian system with Scales using a linear Scaling is
      created.

    * createChartTypes() is called in order to define the structure of the
      diagram.  For details see comment of this function.  As default this
      method creates a tree where all series appear in one branch with the chart
      type determined by getChartTypeForNewSeries().  The stacking is determined
      via the method getStackMode().

    * create an XLegend via the global service factory, set it at the diagram.
 */
class ChartTypeTemplate : public ::cppu::WeakImplHelper2<
        ::com::sun::star::chart2::XChartTypeTemplate,
        ::com::sun::star::lang::XServiceName >
{
public:
    explicit ChartTypeTemplate(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext,
        const OUString & rServiceName );
    virtual ~ChartTypeTemplate();

    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
//  APPHELPER_SERVICE_FACTORY_HELPER( ChartTypeTemplate )

protected:
    // ____ XChartTypeTemplate ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram > SAL_CALL createDiagramByDataSource(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xDataSource,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
        throw (::com::sun::star::uno::RuntimeException);
    /// denotes if the chart needs categories at the first scale
    virtual sal_Bool SAL_CALL supportsCategories()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL changeDiagram(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL changeDiagramData(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xDataSource,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL matchesTemplate(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDiagram >& xDiagram,
        ::sal_Bool bAdaptProperties )
        throw (::com::sun::star::uno::RuntimeException);
    // still abstract: getChartTypeForNewSeries()
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataInterpreter > SAL_CALL getDataInterpreter()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL applyStyle(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries,
        ::sal_Int32 nChartTypeIndex,
        ::sal_Int32 nSeriesIndex,
        ::sal_Int32 nSeriesCount )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resetStyles(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL applyStyles(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

    // Methods to overload for automatic creation

    /// returns 2 by default.  Supported are 2 and 3
    virtual sal_Int32 getDimension() const;

    /** returns StackMode_NONE by default.  This is a global flag used for all
        series of a specific chart type.  If percent stacking is supported, the
        percent stacking mode is retrieved from the first chart type (index 0)

        @param nChartTypeIndex denotes the index of the charttype in means
            defined by the template creation order, i.e., 0 means the first
            chart type that a template creates.
     */
    virtual StackMode getStackMode( sal_Int32 nChartTypeIndex ) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >
                getChartTypeForIndex( sal_Int32 nChartTypeIndex ) = 0;

    virtual bool isSwapXAndY() const;

    // Methods for creating the diagram piecewise

    /** Allows derived classes to manipulate the diagramas whole, like changing
        the wall color. The default implementation is empty. It is called by
        FillDiagram which is called by createDiagramByDataSource and
        changeDiagram
     */
    virtual void adaptDiagram(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram );

    /** Creates a 2d or 3d cartesian coordinate system with mathematically
        oriented, linear scales with auto-min/max.  If the given
        CoordinateSystemContainer is not empty, those coordinate system should
        be reused.

        <p>The dimension depends on the value returned by getDimension().</p>
     */
    virtual void createCoordinateSystems(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystemContainer > & xOutCooSysCnt );

    /** Sets categories at the scales of dimension 0 and the percent stacking at
        the scales of dimension 1 of all given coordinate systems.

        <p>Called by FillDiagram.</p>
     */
    virtual void adaptScales(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XCoordinateSystem > > & aCooSysSeq,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > & xCategories );

    /** create a data series tree, that fits the requirements of the chart type.

        <p>As default, this creates a tree with the following structure:</p>

        <pre>
          root
           |
           +-- chart type (determined by getChartTypeForNewSeries())
                   |
                   +-- category (DiscreteStackableScaleGroup using scale 0)
                          |
                          +-- values (ContinuousStackableScaleGroup using scale 1)
                                |
                                +-- series 0
                                |
                                +-- series 1
                                |
                                ...
                                |
                                +.. series n-1
        </pre>

        <p>If there are less than two scales available the returned tree is
        empty.</p>
     */
    virtual void createChartTypes(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Reference<
                        ::com::sun::star::chart2::XDataSeries > > > & aSeriesSeq,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem > > & rCoordSys,
            const ::com::sun::star::uno::Sequence<
                      ::com::sun::star::uno::Reference<
                          ::com::sun::star::chart2::XChartType > > & aOldChartTypesSeq
            );

    /** create axes and add them to the given container. If there are already
        compatible axes in the container these should be maintained.

        <p>As default, this method creates as many axes as there are dimensions
        in the given first coordinate system.  Each of the axis
        represents one of the dimensions of the coordinate systems. If there are series
        requesting asecondary axes a secondary y axes is added</p>
     */
    virtual void createAxes(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XCoordinateSystem > > & rCoordSys );

    /** Give the number of requested axis per dimension here.  Default is one
        axis for each dimension
     */
    virtual sal_Int32 getAxisCountByDimension( sal_Int32 nDimension );

    /** adapt properties of exsisting axes and remove superfluous axes
    */
    virtual void adaptAxes(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XCoordinateSystem > > & rCoordSys );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        GetComponentContext() const;

    static void copyPropertiesFromOldToNewCoordianteSystem(
                    const ::com::sun::star::uno::Sequence<
                      ::com::sun::star::uno::Reference<
                          ::com::sun::star::chart2::XChartType > > & rOldChartTypesSeq,
                    const ::com::sun::star::uno::Reference<
                          ::com::sun::star::chart2::XChartType > & xNewChartType );

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
    mutable ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataInterpreter > m_xDataInterpreter;

private:
    const OUString m_aServiceName;

private:
    /** modifies the given diagram
     */
    void FillDiagram( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::chart2::XDiagram > & xDiagram,
                      const ::com::sun::star::uno::Sequence<
                          ::com::sun::star::uno::Sequence<
                              ::com::sun::star::uno::Reference<
                                  ::com::sun::star::chart2::XDataSeries > > > & aSeriesSeq,
                      ::com::sun::star::uno::Reference<
                          ::com::sun::star::chart2::data::XLabeledDataSequence > xCategories,
                      const ::com::sun::star::uno::Sequence<
                              ::com::sun::star::uno::Reference<
                                  ::com::sun::star::chart2::XChartType > > & aOldChartTypesSeq,
                      bool bCreate );
};

} //  namespace chart

// CHART_CHARTTYPETEMPLATE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
