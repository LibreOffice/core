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
#pragma once

#include "OPropertySet.hxx"
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart/X3DDefaultSetter.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "ModifyListenerHelper.hxx"

#include <vector>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::chart2 { class XDataSeries; }
namespace com::sun::star::chart2::data { class XDataSource; }
namespace com::sun::star::uno { class XComponentContext; }

namespace chart
{
class Axis;
class BaseCoordinateSystem;
class ChartType;
class ChartTypeManager;
class ChartTypeTemplate;
class DataSeries;
class Legend;
class DataTable;
class RegressionCurveModel;
enum class StackMode;
class Wall;
enum class ThreeDLookScheme;

enum class DiagramPositioningMode
{
    Auto, Excluding, Including
};


namespace impl
{
typedef ::cppu::WeakImplHelper<
    css::chart2::XDiagram,
    css::lang::XServiceInfo,
    css::chart2::XCoordinateSystemContainer,
    css::chart2::XTitled,
    css::chart::X3DDefaultSetter,
    css::util::XModifyBroadcaster,
    css::util::XModifyListener,
    css::util::XCloneable >
    Diagram_Base;
}

class Diagram
    final
    : public impl::Diagram_Base
    , public ::property::OPropertySet
{
public:
    Diagram( css::uno::Reference< css::uno::XComponentContext > xContext );
    virtual ~Diagram() override;

    /// XServiceInfo declarations
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

    explicit Diagram( const Diagram & rOther );

    // ____ OPropertySet ____
    virtual void GetDefaultValue( sal_Int32 nHandle, css::uno::Any& rAny ) const override;

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    // ____ XPropertySet ____
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;

    // ____ XFastPropertySet ____
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    virtual void SAL_CALL getFastPropertyValue(
        css::uno::Any& rValue, sal_Int32 nHandle ) const override;

    // ____ XDiagram ____
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getWall() override;
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getFloor() override;
    virtual css::uno::Reference< css::chart2::XLegend > SAL_CALL getLegend() override;
    virtual void SAL_CALL setLegend( const css::uno::Reference<
                                     css::chart2::XLegend >& xLegend ) override;
    virtual css::uno::Reference< css::chart2::XColorScheme > SAL_CALL getDefaultColorScheme() override;
    virtual void SAL_CALL setDefaultColorScheme(
        const css::uno::Reference< css::chart2::XColorScheme >& xColorScheme ) override;
    virtual void SAL_CALL setDiagramData(
        const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource,
        const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;

    virtual css::uno::Reference<css::chart2::XDataTable> SAL_CALL getDataTable() override;
    virtual void SAL_CALL setDataTable(const css::uno::Reference<css::chart2::XDataTable>& xDataTable) override;

    // ____ XCoordinateSystemContainer ____
    virtual void SAL_CALL addCoordinateSystem(
        const css::uno::Reference< css::chart2::XCoordinateSystem >& aCoordSys ) override;
    virtual void SAL_CALL removeCoordinateSystem(
        const css::uno::Reference< css::chart2::XCoordinateSystem >& aCoordSys ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::chart2::XCoordinateSystem > > SAL_CALL getCoordinateSystems() override;
    virtual void SAL_CALL setCoordinateSystems(
        const css::uno::Sequence< css::uno::Reference< css::chart2::XCoordinateSystem > >& aCoordinateSystems ) override;

    // ____ XTitled ____
    virtual css::uno::Reference<
        css::chart2::XTitle > SAL_CALL getTitleObject() override;
    virtual void SAL_CALL setTitleObject( const css::uno::Reference<
                                          css::chart2::XTitle >& Title ) override;

    // ____ X3DDefaultSetter ____
    virtual void SAL_CALL set3DSettingsToDefault() override;
    virtual void SAL_CALL setDefaultRotation() override;
    virtual void SAL_CALL setDefaultIllumination() override;

    // ____ XCloneable ____
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone() override;

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    typedef
        std::vector< rtl::Reference< ::chart::BaseCoordinateSystem > >
        tCoordinateSystemContainerType;

    tCoordinateSystemContainerType getBaseCoordinateSystems() const;
    void setCoordinateSystems(
        const std::vector< rtl::Reference< ::chart::BaseCoordinateSystem > >& aCoordinateSystems );

    rtl::Reference< ::chart::Legend > getLegend2() const;
    void setLegend(const rtl::Reference< ::chart::Legend > &);

    void setDataTable(const rtl::Reference<::chart::DataTable>& xNewDataTable);
    rtl::Reference<::chart::DataTable> getDataTableRef() const;

    DiagramPositioningMode getDiagramPositioningMode();

    //returns integer from constant group css::chart::MissingValueTreatment
    sal_Int32 getCorrectedMissingValueTreatment(
            const rtl::Reference< ::chart::ChartType >& xChartType );

    void setGeometry3D( sal_Int32 nNewGeometry );

    sal_Int32 getGeometry3D( bool& rbFound, bool& rbAmbiguous );

    bool isPieOrDonutChart();

    bool isSupportingFloorAndWall();

    /**
    * Move a series forward or backward.
    *
    * @param xDiagram
    *  Reference to the diagram that contains the series.
    *
    * @param xGivenDataSeries
    *  Reference to the series that should be moved.
    *
    * @param bForward
    *  Direction in which the series should be moved.
    *
    * @returns </sal_True> if the series was moved successfully.
    *
    */
    bool moveSeries(
                const rtl::Reference< DataSeries >& xGivenDataSeries,
                bool bForward );

    /**
    * Test if a series can be moved.
    *
    * @param xDiagram
    *  Reference to the diagram that contains the series.
    *
    * @param xGivenDataSeries
    *  Reference to the series that should be tested for moving.
    *
    * @param bForward
    *  Direction of the move to be checked.
    *
    * @returns </sal_True> if the series can be moved.
    *
    */
    bool isSeriesMoveable(
            const rtl::Reference< DataSeries >& xGivenDataSeries,
            bool bForward );

    std::vector< rtl::Reference< ChartType > > getChartTypes();

    rtl::Reference< ChartType > getChartTypeByIndex( sal_Int32 nIndex );

    bool isSupportingDateAxis();

    css::uno::Reference< css::chart2::data::XLabeledDataSequence >
        getCategories();

    void setCategories(
            const css::uno::Reference< css::chart2::data::XLabeledDataSequence >& xCategories,
            bool bSetAxisType = false, // when this flag is true ...
            bool bCategoryAxis = true);// set the AxisType to CATEGORY or back to REALNUMBER

    bool isCategory();

    /** return all data series in this diagram grouped by chart-types
     */
    std::vector<
           std::vector<
               rtl::Reference< ::chart::DataSeries > > >
        getDataSeriesGroups();

    std::vector< rtl::Reference< ::chart::DataSeries > >
        getDataSeries();

    rtl::Reference< ChartType >
        getChartTypeOfSeries( const rtl::Reference< DataSeries >& xSeries );

    rtl::Reference< ::chart::Axis > getAttachedAxis(
        const rtl::Reference< ::chart::DataSeries >& xSeries );

    bool attachSeriesToAxis( bool bMainAxis,
        const rtl::Reference< DataSeries >& xSeries,
        const css::uno::Reference< css::uno::XComponentContext > & xContext,
        bool bAdaptAxes=true );

    /** Replaces all occurrences of xCooSysToReplace in the tree with
        xReplacement in the diagram's tree
     */
    void replaceCoordinateSystem(
        const rtl::Reference< ::chart::BaseCoordinateSystem > & xCooSysToReplace,
        const rtl::Reference< ::chart::BaseCoordinateSystem > & xReplacement );


    /** Returns the dimension found for all chart types in the tree.  If the
        dimension is not unique, 0 is returned.
     */
    sal_Int32 getDimension();

    /** Sets the dimension of the diagram given.

        1. Sets the dimension of all used ChartTypes
        2. Adapts the DataSeriesTree to reflect the new dimension
        3. If new coordinate-systems have to be created, adapts the
           XCoordinateSystemContainer of the diagram.
     */
    void setDimension( sal_Int32 nNewDimensionCount );


    StackMode getStackMode(bool& rbFound, bool& rbAmbiguous);

    /** The stacking mode is only set at the series found inside
        the first chart type.  This is the standard for all current
        templates (the only template that has more than one chart-type and
        allows stacking is bar/line combi, and for this the stacking only
        applies to the first chart type/the bars)
     */
    void setStackMode(StackMode eStackMode);


    /** Sets the "SwapXAndYAxis" property at all coordinate systems found in the
        given diagram.

        "vertical==true" for bar charts, "vertical==false" for column charts
     */
    void setVertical( bool bVertical );

    /** Gets the "SwapXAndYAxis" property at all coordinate systems found in the
        given diagram.

        "vertical==true" for bar charts, "vertical==false" for column charts
    */
    bool getVertical( bool& rbOutFoundResult, bool& rbOutAmbiguousResult );

    struct tTemplateWithServiceName {
        rtl::Reference< ::chart::ChartTypeTemplate > xChartTypeTemplate;
        OUString sServiceName;
    };

    /** tries to find a template in the chart-type manager that matches this
        diagram.

        @return
            A pair containing a template with the correct properties set as
            first entry and the service name of the templates second entry.  If
            no template was found both elements are empty.
     */
    tTemplateWithServiceName
        getTemplate(const rtl::Reference< ::chart::ChartTypeManager > & xChartTypeManager);

    std::vector<rtl::Reference<::chart::RegressionCurveModel> >
        getAllRegressionCurvesNotMeanValueLine();

    double getCameraDistance();
    void setCameraDistance( double fCameraDistance );

    void getRotation(
            sal_Int32& rnHorizontalAngleDegree, sal_Int32& rnVerticalAngleDegree );
    void setRotation(
            sal_Int32 nHorizontalAngleDegree, sal_Int32 nVerticalYAngleDegree );
    void getRotationAngle(
            double& rfXAngleRad, double& rfYAngleRad, double& rfZAngleRad );
    void setRotationAngle(
            double fXAngleRad, double fYAngleRad, double fZAngleRad );

    ThreeDLookScheme detectScheme();
    void setScheme( ThreeDLookScheme aScheme );

    void setDefaultRotation( bool bPieOrDonut );

    void switchRightAngledAxes( bool bRightAngledAxes );

private:
    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent() override;
    using OPropertySet::disposing;

    void fireModifyEvent();

    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    tCoordinateSystemContainerType m_aCoordSystems;

    rtl::Reference<Wall> m_xWall;
    rtl::Reference<Wall> m_xFloor;

    css::uno::Reference<css::chart2::XTitle> m_xTitle;

    rtl::Reference<::chart::Legend> m_xLegend;
    rtl::Reference<::chart::DataTable> m_xDataTable;
    css::uno::Reference<css::chart2::XColorScheme> m_xColorScheme;
    rtl::Reference<ModifyEventForwarder> m_xModifyEventForwarder;

};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
