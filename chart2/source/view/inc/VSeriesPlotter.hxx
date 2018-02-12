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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_VSERIESPLOTTER_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_VSERIESPLOTTER_HXX

#include <memory>
#include "PlotterBase.hxx"
#include "VDataSeries.hxx"
#include "LabelAlignment.hxx"
#include "MinimumAndMaximumSupplier.hxx"
#include "LegendEntryProvider.hxx"
#include <ExplicitCategoriesProvider.hxx>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>

namespace com { namespace sun { namespace star {
    namespace util {
        class XNumberFormatsSupplier;
    }
    namespace chart2 {
        class XColorScheme;
        class XRegressionCurveCalculator;
    }
}}}

namespace chart {

class NumberFormatterWrapper;

class AxesNumberFormats
{
public:
    AxesNumberFormats() {};

    void setFormat( sal_Int32 nFormatKey, sal_Int32 nDimIndex, sal_Int32 nAxisIndex )
    {
        m_aNumberFormatMap[tFullAxisIndex(nDimIndex,nAxisIndex)] = nFormatKey;
    }
    bool hasFormat( sal_Int32 nDimIndex, sal_Int32 nAxisIndex ) const
    {
        return (m_aNumberFormatMap.find(tFullAxisIndex(nDimIndex,nAxisIndex)) !=m_aNumberFormatMap.end());
    }
    sal_Int32 getFormat( sal_Int32 nDimIndex, sal_Int32 nAxisIndex ) const
    {
        tNumberFormatMap::const_iterator aIt = m_aNumberFormatMap.find(tFullAxisIndex(nDimIndex,nAxisIndex));
        if( aIt !=m_aNumberFormatMap.end() )
            return aIt->second;
        return 0;
    }

private:
    typedef std::pair< sal_Int32, sal_Int32 > tFullAxisIndex;
    typedef std::map< tFullAxisIndex, sal_Int32 > tNumberFormatMap;
    tNumberFormatMap m_aNumberFormatMap;
};

/**
 * A list of series that have the same CoordinateSystem. They are used to be
 * plotted maybe in a stacked manner by a plotter.
 */
class VDataSeriesGroup final
{
public:
    VDataSeriesGroup() = delete;
    VDataSeriesGroup( VDataSeries* pSeries );
    ~VDataSeriesGroup();

    void addSeries( VDataSeries* pSeries );//takes ownership of pSeries
    sal_Int32 getSeriesCount() const;
    void deleteSeries();

    sal_Int32    getPointCount() const;
    sal_Int32    getAttachedAxisIndexForFirstSeries() const;

    void getMinimumAndMaximiumX( double& rfMinimum, double& rfMaximum ) const;
    void getMinimumAndMaximiumYInContinuousXRange( double& rfMinY, double& rfMaxY, double fMinX, double fMaxX, sal_Int32 nAxisIndex ) const;

    void calculateYMinAndMaxForCategory( sal_Int32 nCategoryIndex
                                            , bool bSeparateStackingForDifferentSigns
                                            , double& rfMinimumY, double& rfMaximumY, sal_Int32 nAxisIndex );
    void calculateYMinAndMaxForCategoryRange( sal_Int32 nStartCategoryIndex, sal_Int32 nEndCategoryIndex
                                                , bool bSeparateStackingForDifferentSigns
                                                , double& rfMinimumY, double& rfMaximumY, sal_Int32 nAxisIndex );

    std::vector< VDataSeries* >   m_aSeriesVector;

private:
    //cached values
    struct CachedYValues
    {
        CachedYValues();

        bool    m_bValuesDirty;
        double  m_fMinimumY;
        double  m_fMaximumY;
    };

    mutable bool        m_bMaxPointCountDirty;
    mutable sal_Int32   m_nMaxPointCount;
    typedef std::map< sal_Int32, CachedYValues > tCachedYValuesPerAxisIndexMap;
    mutable std::vector< tCachedYValuesPerAxisIndexMap >   m_aListOfCachedYValues;
};

class VSeriesPlotter : public PlotterBase, public MinimumAndMaximumSupplier, public LegendEntryProvider
{
public:
    VSeriesPlotter() = delete;

    virtual ~VSeriesPlotter() override;

    /*
    * A new series can be positioned relative to other series in a chart.
    * This positioning has two dimensions. First a series can be placed
    * next to each other on the category axis. This position is indicated by xSlot.
    * Second a series can be stacked on top of another. This position is indicated by ySlot.
    * The positions are counted from 0 on.
    * xSlot < 0                     : append the series to already existing x series
    * xSlot > occupied              : append the series to already existing x series
    *
    * If the xSlot is already occupied the given ySlot decides what should happen:
    * ySlot < -1                    : move all existing series in the xSlot to next slot
    * ySlot == -1                   : stack on top at given x position
    * ySlot == already occupied     : insert at given y and x position
    * ySlot > occupied              : stack on top at given x position
    */
    virtual void addSeries( VDataSeries* pSeries, sal_Int32 zSlot = -1, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );

    /** a value <= 0 for a directions means that this direction can be stretched arbitrary
    */
    virtual css::drawing::Direction3D  getPreferredDiagramAspectRatio() const;

    /** this enables you to handle series on the same x axis with different y axis
    the property AttachedAxisIndex at a dataseries indicates which value scale is to use
    (0==AttachedAxisIndex or a not set AttachedAxisIndex property indicates that this series should be scaled at the main y-axis;
    1==AttachedAxisIndex indicates that the series should be scaled at the first secondary axis if there is any otherwise at the main y axis
    and so on.
    The parameter nAxisIndex matches this DataSeries property 'AttachedAxisIndex'.
    nAxisIndex must be greater than 0. nAxisIndex==1 refers to the first secondary axis.
    )

    @throws css::uno::RuntimeException
    */

    void addSecondaryValueScale( const ExplicitScaleData& rScale, sal_Int32 nAxisIndex );

    // MinimumAndMaximumSupplier

    virtual double getMinimumX() override;
    virtual double getMaximumX() override;

    virtual double getMinimumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex ) override;
    virtual double getMaximumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex ) override;

    virtual double getMinimumZ() override;
    virtual double getMaximumZ() override;

    virtual bool isExpandBorderToIncrementRhythm( sal_Int32 nDimensionIndex ) override;
    virtual bool isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex ) override;
    virtual bool isExpandWideValuesToZero( sal_Int32 nDimensionIndex ) override;
    virtual bool isExpandNarrowValuesTowardZero( sal_Int32 nDimensionIndex ) override;
    virtual bool isSeparateStackingForDifferentSigns( sal_Int32 nDimensionIndex ) override;

    virtual long calculateTimeResolutionOnXAxis() override;
    virtual void setTimeResolutionOnXAxis( long nTimeResolution, const Date& rNullDate ) override;

    void getMinimumAndMaximiumX( double& rfMinimum, double& rfMaximum ) const;
    void getMinimumAndMaximiumYInContinuousXRange( double& rfMinY, double& rfMaxY, double fMinX, double fMaxX, sal_Int32 nAxisIndex ) const;


    // Methods for handling legends and legend entries.

    virtual std::vector< ViewLegendEntry > createLegendEntries(
            const css::awt::Size& rEntryKeyAspectRatio,
            css::chart::ChartLegendExpansion eLegendExpansion,
            const css::uno::Reference< css::beans::XPropertySet >& xTextProperties,
            const css::uno::Reference< css::drawing::XShapes >& xTarget,
            const css::uno::Reference< css::lang::XMultiServiceFactory >& xShapeFactory,
            const css::uno::Reference< css::uno::XComponentContext >& xContext
                ) override;

    virtual LegendSymbolStyle getLegendSymbolStyle();
    virtual css::awt::Size getPreferredLegendKeyAspectRatio() override;

    virtual css::uno::Any getExplicitSymbol( const VDataSeries& rSeries, sal_Int32 nPointIndex/*-1 for series symbol*/ );

    css::uno::Reference< css::drawing::XShape > createLegendSymbolForSeries(
                  const css::awt::Size& rEntryKeyAspectRatio
                , const VDataSeries& rSeries
                , const css::uno::Reference< css::drawing::XShapes >& xTarget
                , const css::uno::Reference< css::lang::XMultiServiceFactory >& xShapeFactory );

    css::uno::Reference< css::drawing::XShape > createLegendSymbolForPoint(
                  const css::awt::Size& rEntryKeyAspectRatio
                , const VDataSeries& rSeries
                , sal_Int32 nPointIndex
                , const css::uno::Reference< css::drawing::XShapes >& xTarget
                , const css::uno::Reference< css::lang::XMultiServiceFactory >& xShapeFactory );

    std::vector< ViewLegendEntry > createLegendEntriesForSeries(
            const css::awt::Size& rEntryKeyAspectRatio,
            const VDataSeries& rSeries,
            const css::uno::Reference< css::beans::XPropertySet >& xTextProperties,
            const css::uno::Reference< css::drawing::XShapes >& xTarget,
            const css::uno::Reference< css::lang::XMultiServiceFactory >& xShapeFactory,
            const css::uno::Reference< css::uno::XComponentContext >& xContext
                );

    std::vector< VDataSeries* > getAllSeries();

    // This method creates a series plotter of the requested type; e.g. : return new PieChart ....
    static VSeriesPlotter* createSeriesPlotter( const css::uno::Reference< css::chart2::XChartType >& xChartTypeModel
                                , sal_Int32 nDimensionCount
                                , bool bExcludingPositioning /*for pie and donut charts labels and exploded segments are excluded from the given size*/);

    sal_Int32 getPointCount() const;

    // Methods for number formats and color schemes

    void setNumberFormatsSupplier( const css::uno::Reference< css::util::XNumberFormatsSupplier > & xNumFmtSupplier );
    void setAxesNumberFormats( const AxesNumberFormats& rAxesNumberFormats ) { m_aAxesNumberFormats = rAxesNumberFormats; };

    void setColorScheme( const css::uno::Reference< css::chart2::XColorScheme >& xColorScheme );

    void setExplicitCategoriesProvider( ExplicitCategoriesProvider* pExplicitCategoriesProvider );

    //get series names for the z axis labels
    css::uno::Sequence< OUString > getSeriesNames() const;

    void setPageReferenceSize( const css::awt::Size & rPageRefSize );
    //better performance for big data
    void setCoordinateSystemResolution( const css::uno::Sequence< sal_Int32 >& rCoordinateSystemResolution );
    bool PointsWereSkipped() const { return m_bPointsWereSkipped;}

    //return the depth for a logic 1
    double  getTransformedDepth() const;

    void    releaseShapes();

    virtual void rearrangeLabelToAvoidOverlapIfRequested( const css::awt::Size& rPageSize );

    bool WantToPlotInFrontOfAxisLine();
    virtual bool shouldSnapRectToUsedArea();

protected:

    VSeriesPlotter( const css::uno::Reference< css::chart2::XChartType >& xChartTypeModel
                , sal_Int32 nDimensionCount
                , bool bCategoryXAxis=true );

    // Methods for group shapes.

    css::uno::Reference< css::drawing::XShapes >
        getSeriesGroupShape( VDataSeries* pDataSeries
            , const css:: uno::Reference< css::drawing::XShapes >& xTarget );

    //the following group shapes will be created as children of SeriesGroupShape on demand
    //they can be used to assure that some parts of a series shape are always in front of others (e.g. symbols in front of lines)
    //parameter xTarget will be used as parent for the series group shape
    css::uno::Reference< css::drawing::XShapes >
        getSeriesGroupShapeFrontChild( VDataSeries* pDataSeries
            , const css:: uno::Reference< css::drawing::XShapes >& xTarget );
    css::uno::Reference< css::drawing::XShapes >
        getSeriesGroupShapeBackChild( VDataSeries* pDataSeries
            , const css:: uno::Reference< css::drawing::XShapes >& xTarget );

    /// This method creates a 2D group shape for containing all text shapes
    /// needed for this series; the group is added to the text target;
    css::uno::Reference< css::drawing::XShapes >
        getLabelsGroupShape( VDataSeries& rDataSeries
            , const css:: uno::Reference< css::drawing::XShapes >& xTarget );

    css::uno::Reference< css::drawing::XShapes >
        getErrorBarsGroupShape( VDataSeries& rDataSeries
            , const css:: uno::Reference< css::drawing::XShapes >& xTarget, bool bYError );

    /** This method creates a text shape for a label related to a data point
     *  and append it to the root text shape group (xTarget).
     *
     *  @param xTarget
     *      the main root text shape group.
     *  @param rDataSeries
     *      the data series, the data point belongs to.
     *  @param nPointIndex
     *      the index of the data point the label is related to.
     *  @param fValue
     *      the value of the data point.
     *  @param fSumValue
     *      the sum of all data point values in the data series.
     *  @param rScreenPosition2D
     *      the anchor point position for the label.
     *  @param eAlignment
     *      the required alignment of the label.
     *  @param offset
     *      an optional offset depending on the label alignment.
     *  @param nTextWidth
     *      the maximum width of a text label (used for text wrapping).
     *
     *  @return
     *      a reference to the created text shape.
     */
    css::uno::Reference< css::drawing::XShape >
        createDataLabel( const css::uno::Reference< css::drawing::XShapes >& xTarget
                , VDataSeries& rDataSeries
                , sal_Int32 nPointIndex
                , double fValue
                , double fSumValue
                , const css::awt::Point& rScreenPosition2D
                , LabelAlignment eAlignment
                , sal_Int32 nOffset=0
                , sal_Int32 nTextWidth = 0 );

    /// This method returns a text string representation of the passed numeric
    /// value by exploiting a NumberFormatterWrapper object.
    OUString getLabelTextForValue( VDataSeries const & rDataSeries
                , sal_Int32 nPointIndex
                , double fValue
                , bool bAsPercentage );

    /** creates two T-shaped error bars in both directions (up/down or
        left/right depending on the bVertical parameter)

        @param rPos
            logic coordinates

        @param xErrorBarProperties
            the XPropertySet returned by the DataPoint-property "ErrorBarX" or
            "ErrorBarY".

        @param nIndex
            the index of the data point in rData for which the calculation is
            done.

        @param bVertical
            for y-error bars this is true, for x-error-bars it is false.
     */
    void createErrorBar(
          const css::uno::Reference< css::drawing::XShapes >& xTarget
        , const css::drawing::Position3D & rPos
        , const css::uno::Reference< css::beans::XPropertySet > & xErrorBarProperties
        , const VDataSeries& rVDataSeries
        , sal_Int32 nIndex
        , bool bVertical
        , const double* pfScaledLogicX
        );

    void createErrorBar_X( const css::drawing::Position3D& rUnscaledLogicPosition
        , VDataSeries& rVDataSeries, sal_Int32 nPointIndex
        , const css::uno::Reference< css::drawing::XShapes >& xTarget );

    void createErrorBar_Y( const css::drawing::Position3D& rUnscaledLogicPosition
        , VDataSeries& rVDataSeries, sal_Int32 nPointIndex
        , const css::uno::Reference< css::drawing::XShapes >& xTarget
        , double const * pfScaledLogicX );

    void createRegressionCurvesShapes( VDataSeries const & rVDataSeries
        , const css::uno::Reference< css::drawing::XShapes >& xTarget
        , const css::uno::Reference< css::drawing::XShapes >& xEquationTarget
        , bool bMaySkipPointsInRegressionCalculation );

    void createRegressionCurveEquationShapes( const OUString & rEquationCID
        , const css::uno::Reference< css::beans::XPropertySet > & xEquationProperties
        , const css::uno::Reference< css::drawing::XShapes >& xEquationTarget
        , const css::uno::Reference< css::chart2::XRegressionCurveCalculator > & xRegressionCurveCalculator
        , css::awt::Point aDefaultPos );

    static void setMappedProperties(
          const css::uno::Reference< css::drawing::XShape >& xTarget
        , const css::uno::Reference< css::beans::XPropertySet >& xSource
        , const tPropertyNameMap& rMap
        , tPropertyNameValueMap const * pOverwriteMap=nullptr );

    virtual PlottingPositionHelper& getPlottingPositionHelper( sal_Int32 nAxisIndex ) const;//nAxisIndex indicates whether the position belongs to the main axis ( nAxisIndex==0 ) or secondary axis ( nAxisIndex==1 )

    VDataSeries* getFirstSeries() const;

    OUString getCategoryName( sal_Int32 nPointIndex ) const;

protected:
    PlottingPositionHelper*    m_pMainPosHelper;

    css::uno::Reference< css::chart2::XChartType >    m_xChartTypeModel;
    css::uno::Reference< css::beans::XPropertySet >   m_xChartTypeModelProps;

    std::vector< std::vector< VDataSeriesGroup > >  m_aZSlots;

    bool                                m_bCategoryXAxis;//true->xvalues are indices (this would not be necessary if series for category chart wouldn't have x-values)
    long m_nTimeResolution;
    Date m_aNullDate;

    std::unique_ptr< NumberFormatterWrapper > m_apNumberFormatterWrapper;
    AxesNumberFormats                         m_aAxesNumberFormats;//direct numberformats on axes, if empty ask the data series instead

    css::uno::Reference< css::chart2::XColorScheme >    m_xColorScheme;

    ExplicitCategoriesProvider*    m_pExplicitCategoriesProvider;

    //better performance for big data
    css::uno::Sequence< sal_Int32 >    m_aCoordinateSystemResolution;
    bool m_bPointsWereSkipped;

private:
    typedef std::map< sal_Int32 , ExplicitScaleData > tSecondaryValueScales;
    tSecondaryValueScales   m_aSecondaryValueScales;

    typedef std::map< sal_Int32 , PlottingPositionHelper* > tSecondaryPosHelperMap;
    mutable tSecondaryPosHelperMap   m_aSecondaryPosHelperMap;
    css::awt::Size      m_aPageReferenceSize;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
