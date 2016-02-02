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

#include <config_features.h>

#include "ChartView.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "NumberFormatterWrapper.hxx"
#include "ViewDefines.hxx"
#include "VDiagram.hxx"
#include "VTitle.hxx"
#include "AbstractShapeFactory.hxx"
#include "VCoordinateSystem.hxx"
#include "VSeriesPlotter.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "TitleHelper.hxx"
#include "LegendHelper.hxx"
#include "VLegend.hxx"
#include "PropertyMapper.hxx"
#include "ChartModelHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "ScaleAutomatism.hxx"
#include "MinimumAndMaximumSupplier.hxx"
#include "ObjectIdentifier.hxx"
#include "DiagramHelper.hxx"
#include "RelativePositionHelper.hxx"
#include "servicenames.hxx"
#include "AxisHelper.hxx"
#include "AxisIndexDefines.hxx"
#include "ControllerLockGuard.hxx"
#include "BaseGFXHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "DateHelper.hxx"
#include "defines.hxx"
#include <unonames.hxx>
#include <GL3DBarChart.hxx>

#include <editeng/frmdiritem.hxx>
#include <rtl/uuid.h>
#include <tools/globname.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/localedatawrapper.hxx>
#include <svx/charthelper.hxx>
#include <svx/svdpage.hxx>
#include <svx/unopage.hxx>
#include <svx/unoshape.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <svx/unofill.hxx>
#include <vcl/openglwin.hxx>
#include <vcl/opengl/OpenGLContext.hxx>

#include <drawinglayer/XShapeDumper.hxx>

#include <time.h>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XShapeGroup.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <svl/languageoptions.hxx>
#include <comphelper/classids.hxx>
#include "servicenames_charttypes.hxx"

#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

#include <osl/conditn.hxx>
#include <osl/time.h>
#include <o3tl/make_unique.hxx>

#include <memory>

namespace chart {

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

namespace {

class theExplicitValueProviderUnoTunnelId  : public rtl::Static<UnoTunnelIdInit, theExplicitValueProviderUnoTunnelId> {};

typedef std::pair< sal_Int32, sal_Int32 > tFullAxisIndex; //first index is the dimension, second index is the axis index that indicates whether this is a main or secondary axis
typedef std::map< VCoordinateSystem*, tFullAxisIndex > tCoordinateSystemMap;

/** This class handles a collection of coordinate systems and is used for
 *  executing some action on all coordinate systems such as
 *  `prepareAutomaticAxisScaling` and `setExplicitScaleAndIncrement`.
 *  Moreover it contains the `aAutoScaling` object that is an instance of
 *  the `ScaleAutomatism` class. The initialization of `aAutoScaling` is
 *  performed in the `SeriesPlotterContainer::initAxisUsageList` method and is
 *  used in the `SeriesPlotterContainer::doAutoScaling` for calculating explicit
 *  scale and increment objects (see `SeriesPlotterContainer::doAutoScaling`).
 */
struct AxisUsage
{
    AxisUsage();
    ~AxisUsage();

    void addCoordinateSystem( VCoordinateSystem* pCooSys, sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );
    ::std::vector< VCoordinateSystem* > getCoordinateSystems( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );
    sal_Int32 getMaxAxisIndexForDimension( sal_Int32 nDimensionIndex );

    void prepareAutomaticAxisScaling( ScaleAutomatism& rScaleAutomatism, sal_Int32 nDimIndex, sal_Int32 nAxisIndex );
    void setExplicitScaleAndIncrement( sal_Int32 nDimIndex, sal_Int32 nAxisIndex, const ExplicitScaleData& rScale, const ExplicitIncrementData& rInc );

    ScaleAutomatism aAutoScaling;

private:
    tCoordinateSystemMap    aCoordinateSystems;
    std::map< sal_Int32, sal_Int32 > aMaxIndexPerDimension;
};

AxisUsage::AxisUsage()
    : aAutoScaling(AxisHelper::createDefaultScale(), Date(Date::SYSTEM))
{
}

AxisUsage::~AxisUsage()
{
    aCoordinateSystems.clear();
}

void AxisUsage::addCoordinateSystem( VCoordinateSystem* pCooSys, sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    if(!pCooSys)
        return;

    tFullAxisIndex aFullAxisIndex( nDimensionIndex, nAxisIndex );
    tCoordinateSystemMap::const_iterator aFound( aCoordinateSystems.find(pCooSys) );

    //use one scale only once for each coordinate system
    //main axis are preferred over secondary axis
    //value scales are preferred
    if(aFound!=aCoordinateSystems.end())
    {
        sal_Int32 nFoundAxisIndex = aFound->second.second;
        if( nFoundAxisIndex < nAxisIndex )
            return;
        sal_Int32 nFoundDimension = aFound->second.first;
        if( nFoundDimension ==1 )
            return;
        if( nFoundDimension < nDimensionIndex )
            return;
    }
    aCoordinateSystems[pCooSys] = aFullAxisIndex;

    //set maximum scale index
    std::map< sal_Int32, sal_Int32 >::const_iterator aIter = aMaxIndexPerDimension.find(nDimensionIndex);
    if( aIter != aMaxIndexPerDimension.end() )
    {
        sal_Int32 nCurrentMaxIndex = aIter->second;
        if( nCurrentMaxIndex < nAxisIndex )
            aMaxIndexPerDimension[nDimensionIndex]=nAxisIndex;
    }
    else
        aMaxIndexPerDimension[nDimensionIndex]=nAxisIndex;
}

::std::vector< VCoordinateSystem* > AxisUsage::getCoordinateSystems( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex )
{
    ::std::vector< VCoordinateSystem* > aRet;

    tCoordinateSystemMap::const_iterator aIter;
    for( aIter = aCoordinateSystems.begin(); aIter!=aCoordinateSystems.end();++aIter )
    {
        if( aIter->second.first != nDimensionIndex )
            continue;
        if( aIter->second.second != nAxisIndex )
            continue;
        aRet.push_back( aIter->first );
    }

    return aRet;
}

sal_Int32 AxisUsage::getMaxAxisIndexForDimension( sal_Int32 nDimensionIndex )
{
    sal_Int32 nRet = -1;
    std::map< sal_Int32, sal_Int32 >::const_iterator aIter = aMaxIndexPerDimension.find(nDimensionIndex);
    if( aIter != aMaxIndexPerDimension.end() )
        nRet = aIter->second;
    return nRet;
}

void AxisUsage::prepareAutomaticAxisScaling( ScaleAutomatism& rScaleAutomatism, sal_Int32 nDimIndex, sal_Int32 nAxisIndex )
{
    std::vector<VCoordinateSystem*> aVCooSysList = getCoordinateSystems(nDimIndex, nAxisIndex);
    for (size_t i = 0, n = aVCooSysList.size(); i < n; ++i)
        aVCooSysList[i]->prepareAutomaticAxisScaling(rScaleAutomatism, nDimIndex, nAxisIndex);
}

void AxisUsage::setExplicitScaleAndIncrement(
    sal_Int32 nDimIndex, sal_Int32 nAxisIndex, const ExplicitScaleData& rScale, const ExplicitIncrementData& rInc )
{
    std::vector<VCoordinateSystem*> aVCooSysList = getCoordinateSystems(nDimIndex, nAxisIndex);
    for (size_t i = 0, n = aVCooSysList.size(); i < n; ++i)
        aVCooSysList[i]->setExplicitScaleAndIncrement(nDimIndex, nAxisIndex, rScale, rInc);
}

typedef std::vector<std::unique_ptr<VSeriesPlotter> > SeriesPlottersType;

/** This class is a container of `SeriesPlotter` objects (such as `PieChart`
 *  instances). It is used for initializing coordinate systems, axes and scales
 *  of all series plotters which belongs to the container.
 */
class SeriesPlotterContainer
{
public:
    explicit SeriesPlotterContainer( std::vector< VCoordinateSystem* >& rVCooSysList );
    ~SeriesPlotterContainer();

    /** It is used to set coordinate systems (`m_rVCooSysList`), this method
     *  is invoked by `ChartView::createShapes2D` before of
     *  `ChartView::impl_createDiagramAndContent`.
     *  Coordinate systems are retrieved through the `XCoordinateSystemContainer`
     *  interface implemented by a diagram object which is provided by the
     *  `ChartModel` object passed to the method (`rChartModel.getFirstDiagram()`).
     *
     *  It is used for creating series plotters and appending them
     *  to `m_aSeriesPlotterList`. The created series plotters are initialized
     *  through data (number formats supplier, color scheme, data series),
     *  extracted from the chart model or the diagram objects. An exception is
     *  the explicit category provider that is retrieved through the
     *  `VCoordinateSystem` object used by the series plotter.
     *
     *  It sets the minimum-maximum supplier for a coordinate system:
     *  this supplier is the series plotter itself which utilizes the given
     *  coordinate system. In fact `VSeriesPlotter` has `MinimumMaximumSupplier`
     *  as one of its base classes.
     *  Hence, for instance, a `PieChart`, which is a series plotter, is
     *  a `MinimumMaximumSupplier`, too.
     */
    void initializeCooSysAndSeriesPlotter( ChartModel& rModel );

    /** This method is invoked by `ChartView::impl_createDiagramAndContent`.
     *  It iterates on every axis of every coordinate systems, and if the axis
     *  is not yet present in `m_aAxisUsageList` it creates a new `AxisUsage`
     *  object and initialize its `aAutoScaling` member to the `ScaleData`
     *  object of the current axis.
     */
    void initAxisUsageList(const Date& rNullDate);

    /**
     * Perform automatic axis scaling and determine the amount and spacing of
     * increments.  It assumes that the caller has determined the size of the
     * largest axis label text object prior to calling this method.
     *
     * The new axis scaling data will be stored in the VCoordinateSystem
     * objects.  The label alignment direction for each axis will also get
     * determined during this process, and stored in VAxis.
     *
     * This method is invoked by `ChartView::impl_createDiagramAndContent`
     * soon after `initAxisUsageList`.
     * It initializes explicit scale and increment objects for all coordinate
     * systems in `m_rVCooSysList`.
     * This action is achieved by iterating on the `m_aAxisUsageList` container,
     * and performing 3 steps:
     *   1- call `VCoordinateSystem::prepareAutomaticAxisScaling` for setting
     *      scaling parameters of the `aAutoScaling` member (a `ScaleAutomatism`
     *      object) for the current `AxisUsage` instance
     *      (see `VCoordinateSystem::prepareAutomaticAxisScaling`);
     *   2- calculate the explicit scale and increment objects
     *      (see ScaleAutomatism::calculateExplicitScaleAndIncrement);
     *   3- set the explicit scale and increment objects for each coordinate
     *      system.
     */
    void doAutoScaling( ChartModel& rModel );

    /**
     * After auto-scaling is performed, call this method to set the explicit
     * scaling and increment data to all relevant VAxis objects.
     */
    void updateScalesAndIncrementsOnAxes();

    /**
     * After auto-scaling is performed, call this method to set the explicit
     * scaling data to all the plotters.
     */
    void setScalesFromCooSysToPlotter();

    void setNumberFormatsFromAxes();
    drawing::Direction3D getPreferredAspectRatio();

    SeriesPlottersType& getSeriesPlotterList() { return m_aSeriesPlotterList; }
    std::vector< VCoordinateSystem* >& getCooSysList() { return m_rVCooSysList; }
    std::vector< LegendEntryProvider* > getLegendEntryProviderList();

    void AdaptScaleOfYAxisWithoutAttachedSeries( ChartModel& rModel );

    bool isCategoryPositionShifted(
        const chart2::ScaleData& rSourceScale, bool bHasComplexCategories ) const;

private:
    /** A vector of series plotters.
     */
    SeriesPlottersType m_aSeriesPlotterList;

    /** A vector of coordinate systems.
     */
    std::vector< VCoordinateSystem* >& m_rVCooSysList;

    /** A map whose key is a `XAxis` interface and the related value is
     *  an object of `AxisUsage` type.
     */
    ::std::map< uno::Reference< XAxis >, AxisUsage > m_aAxisUsageList;

    /**
     * Max axis index of all dimensions.  Currently this can be either 0 or 1
     * since we only support primary and secondary axes per dimension.  The
     * value of 0 means all dimensions have only primary axis, while 1 means
     * at least one dimension has a secondary axis.
     */
    sal_Int32 m_nMaxAxisIndex;

    bool m_bChartTypeUsesShiftedCategoryPositionPerDefault;
    sal_Int32 m_nDefaultDateNumberFormat;
};

SeriesPlotterContainer::SeriesPlotterContainer( std::vector< VCoordinateSystem* >& rVCooSysList )
        : m_rVCooSysList( rVCooSysList )
        , m_nMaxAxisIndex(0)
        , m_bChartTypeUsesShiftedCategoryPositionPerDefault(false)
        , m_nDefaultDateNumberFormat(0)
{
}

SeriesPlotterContainer::~SeriesPlotterContainer()
{
    // - remove plotter from coordinatesystems
    for( size_t nC=0; nC < m_rVCooSysList.size(); nC++)
        m_rVCooSysList[nC]->clearMinimumAndMaximumSupplierList();
}

std::vector< LegendEntryProvider* > SeriesPlotterContainer::getLegendEntryProviderList()
{
    std::vector< LegendEntryProvider* > aRet( m_aSeriesPlotterList.size() );
    sal_Int32 nN = 0;
    for( std::unique_ptr<VSeriesPlotter>& aPlotter : m_aSeriesPlotterList)
        aRet[nN++] = aPlotter.get();
    return aRet;
}

VCoordinateSystem* findInCooSysList( const std::vector< VCoordinateSystem* >& rVCooSysList
                                    , const uno::Reference< XCoordinateSystem >& xCooSys )
{
    for( size_t nC=0; nC < rVCooSysList.size(); nC++)
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[nC];
        if(pVCooSys->getModel()==xCooSys)
            return pVCooSys;
    }
    return nullptr;
}

VCoordinateSystem* lcl_getCooSysForPlotter( const std::vector< VCoordinateSystem* >& rVCooSysList, MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier )
{
    if(!pMinimumAndMaximumSupplier)
        return nullptr;
    for( size_t nC=0; nC < rVCooSysList.size(); nC++)
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[nC];
        if(pVCooSys->hasMinimumAndMaximumSupplier( pMinimumAndMaximumSupplier ))
            return pVCooSys;
    }
    return nullptr;
}

VCoordinateSystem* addCooSysToList( std::vector< VCoordinateSystem* >& rVCooSysList
            , const uno::Reference< XCoordinateSystem >& xCooSys
            , ChartModel& rChartModel )
{
    VCoordinateSystem* pVCooSys = findInCooSysList( rVCooSysList, xCooSys );
    if( !pVCooSys )
    {
        pVCooSys = VCoordinateSystem::createCoordinateSystem(xCooSys );
        if(pVCooSys)
        {
            OUString aCooSysParticle( ObjectIdentifier::createParticleForCoordinateSystem( xCooSys, rChartModel ) );
            pVCooSys->setParticle(aCooSysParticle);

            pVCooSys->setExplicitCategoriesProvider( new ExplicitCategoriesProvider(xCooSys, rChartModel) );

            rVCooSysList.push_back( pVCooSys );
        }
    }
    return pVCooSys;
}

void SeriesPlotterContainer::initializeCooSysAndSeriesPlotter(
              ChartModel& rChartModel )
{
    sal_Int32 nDiagramIndex = 0;//todo if more than one diagram is supported
    uno::Reference< XDiagram > xDiagram( rChartModel.getFirstDiagram() );
    if( !xDiagram.is())
        return;

    uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( static_cast< ::cppu::OWeakObject* >( &rChartModel ), uno::UNO_QUERY );
    if( rChartModel.hasInternalDataProvider() && DiagramHelper::isSupportingDateAxis( xDiagram ) )
            m_nDefaultDateNumberFormat=DiagramHelper::getDateNumberFormat( xNumberFormatsSupplier );

    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
    if(!nDimensionCount)
    {
        //@todo handle mixed dimension
        nDimensionCount = 2;
    }

    bool bSortByXValues = false;
    bool bConnectBars = false;
    bool bGroupBarsPerAxis = true;
    bool bIncludeHiddenCells = true;
    sal_Int32 nStartingAngle = 90;
    sal_Int32 n3DRelativeHeight = 100;
    try
    {
        uno::Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY_THROW );
        xDiaProp->getPropertyValue(CHART_UNONAME_SORT_BY_XVALUES) >>= bSortByXValues;
        xDiaProp->getPropertyValue( "ConnectBars" ) >>= bConnectBars;
        xDiaProp->getPropertyValue( "GroupBarsPerAxis" ) >>= bGroupBarsPerAxis;
        xDiaProp->getPropertyValue( "IncludeHiddenCells" ) >>= bIncludeHiddenCells;
        xDiaProp->getPropertyValue( "StartingAngle" ) >>= nStartingAngle;

        if (nDimensionCount == 3)
        {
            xDiaProp->getPropertyValue( "3DRelativeHeight" ) >>= n3DRelativeHeight;
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    //prepare for autoscaling and shape creation
    // - create plotter for charttypes (for each first scale group at each plotter, as they are independent)
    // - add series to plotter (thus each charttype can provide minimum and maximum values for autoscaling)
    // - add plotter to coordinate systems

    //iterate through all coordinate systems
    uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    OSL_ASSERT( xCooSysContainer.is());
    if( !xCooSysContainer.is())
        return;
    uno::Reference< XColorScheme > xColorScheme( xDiagram->getDefaultColorScheme());
    uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
    sal_Int32 nGlobalSeriesIndex = 0;//for automatic symbols
    for( sal_Int32 nCS = 0; nCS < aCooSysList.getLength(); ++nCS )
    {
        uno::Reference< XCoordinateSystem > xCooSys( aCooSysList[nCS] );
        VCoordinateSystem* pVCooSys = addCooSysToList(m_rVCooSysList,xCooSys,rChartModel);

        //iterate through all chart types in the current coordinate system
        uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        OSL_ASSERT( xChartTypeContainer.is());
        if( !xChartTypeContainer.is() )
            continue;
        uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            uno::Reference< XChartType > xChartType( aChartTypeList[nT] );
            if(3 == nDimensionCount && xChartType->getChartType().equalsIgnoreAsciiCase(CHART2_SERVICE_NAME_CHARTTYPE_PIE))
            {
                uno::Reference< beans::XPropertySet > xPropertySet( xChartType, uno::UNO_QUERY );
                if (xPropertySet.is())
                {
                    try
                    {
                        sal_Int32 n3DRelativeHeightOldValue(100);
                        uno::Any aAny = xPropertySet->getPropertyValue( "3DRelativeHeight" );
                        aAny >>= n3DRelativeHeightOldValue;
                        if (n3DRelativeHeightOldValue != n3DRelativeHeight)
                            xPropertySet->setPropertyValue( "3DRelativeHeight", uno::makeAny(n3DRelativeHeight) );
                    }
                    catch (const uno::Exception&) { }
                }
            }

            if(nT==0)
                m_bChartTypeUsesShiftedCategoryPositionPerDefault = ChartTypeHelper::shiftCategoryPosAtXAxisPerDefault( xChartType );

            bool bExcludingPositioning = DiagramPositioningMode_EXCLUDING == DiagramHelper::getDiagramPositioningMode( xDiagram );
            VSeriesPlotter* pPlotter = VSeriesPlotter::createSeriesPlotter( xChartType, nDimensionCount, bExcludingPositioning );
            if( !pPlotter )
                continue;

            m_aSeriesPlotterList.push_back( std::unique_ptr<VSeriesPlotter>(pPlotter) );
            pPlotter->setNumberFormatsSupplier( xNumberFormatsSupplier );
            pPlotter->setColorScheme( xColorScheme );
            if(pVCooSys)
                pPlotter->setExplicitCategoriesProvider( pVCooSys->getExplicitCategoriesProvider() );
            sal_Int32 nMissingValueTreatment = DiagramHelper::getCorrectedMissingValueTreatment( xDiagram, xChartType );

            if(pVCooSys)
                pVCooSys->addMinimumAndMaximumSupplier(pPlotter);

            uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
            OSL_ASSERT( xDataSeriesContainer.is());
            if( !xDataSeriesContainer.is() )
                continue;

            sal_Int32 zSlot=-1;
            sal_Int32 xSlot=-1;
            sal_Int32 ySlot=-1;
            uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
            for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
            {
                uno::Reference< XDataSeries > xDataSeries( aSeriesList[nS], uno::UNO_QUERY );
                if(!xDataSeries.is())
                    continue;
                if( !bIncludeHiddenCells && !DataSeriesHelper::hasUnhiddenData(xDataSeries) )
                    continue;

                VDataSeries* pSeries = new VDataSeries( xDataSeries );

                pSeries->setGlobalSeriesIndex(nGlobalSeriesIndex);
                nGlobalSeriesIndex++;

                if( bSortByXValues )
                    pSeries->doSortByXValues();

                pSeries->setConnectBars( bConnectBars );
                pSeries->setGroupBarsPerAxis( bGroupBarsPerAxis );
                pSeries->setStartingAngle( nStartingAngle );

                pSeries->setMissingValueTreatment( nMissingValueTreatment );

                OUString aSeriesParticle( ObjectIdentifier::createParticleForSeries( nDiagramIndex, nCS, nT, nS ) );
                pSeries->setParticle(aSeriesParticle);

                OUString aRole( ChartTypeHelper::getRoleOfSequenceForDataLabelNumberFormatDetection( xChartType ) );
                pSeries->setRoleOfSequenceForDataLabelNumberFormatDetection(aRole);

                //ignore secondary axis for charttypes that do not support them
                if( pSeries->getAttachedAxisIndex() != MAIN_AXIS_INDEX &&
                    !ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimensionCount, 1 ) )
                {
                    pSeries->setAttachedAxisIndex(MAIN_AXIS_INDEX);
                }

                StackingDirection eDirection = pSeries->getStackingDirection();
                switch(eDirection)
                {
                    case StackingDirection_NO_STACKING:
                        xSlot++; ySlot=-1;
                        if(zSlot<0)
                            zSlot=0;
                        break;
                    case StackingDirection_Y_STACKING:
                        ySlot++;
                        if(xSlot<0)
                            xSlot=0;
                        if(zSlot<0)
                            zSlot=0;
                        break;
                    case StackingDirection_Z_STACKING:
                        zSlot++; xSlot=-1; ySlot=-1;
                        break;
                    default:
                        // UNO enums have one additional auto-generated case
                        break;
                }
                pPlotter->addSeries( pSeries, zSlot, xSlot, ySlot );
            }
        }
    }

    //transport seriesnames to the coordinatesystems if needed
    if( !m_aSeriesPlotterList.empty() )
    {
        uno::Sequence< OUString > aSeriesNames;
        bool bSeriesNamesInitialized = false;
        for( size_t nC=0; nC < m_rVCooSysList.size(); nC++)
        {
            VCoordinateSystem* pVCooSys = m_rVCooSysList[nC];
            if(!pVCooSys)
                continue;
            if( pVCooSys->needSeriesNamesForAxis() )
            {
                if(!bSeriesNamesInitialized)
                {
                    aSeriesNames = m_aSeriesPlotterList[0]->getSeriesNames();
                    bSeriesNamesInitialized = true;
                }
                pVCooSys->setSeriesNamesForAxis( aSeriesNames );
            }
        }
    }
}

bool SeriesPlotterContainer::isCategoryPositionShifted(
    const chart2::ScaleData& rSourceScale, bool bHasComplexCategories ) const
{
    if (rSourceScale.AxisType == AxisType::CATEGORY && m_bChartTypeUsesShiftedCategoryPositionPerDefault)
        return true;

    if (rSourceScale.AxisType==AxisType::CATEGORY && bHasComplexCategories)
        return true;

    if (rSourceScale.AxisType == AxisType::DATE)
        return true;

    if (rSourceScale.AxisType == AxisType::SERIES)
        return true;

    return false;
}

void SeriesPlotterContainer::initAxisUsageList(const Date& rNullDate)
{
    m_aAxisUsageList.clear();

    // Loop through coordinate systems in the diagram (though for now
    // there should only be one coordinate system per diagram).
    for (size_t i = 0, n = m_rVCooSysList.size(); i < n; ++i)
    {
        VCoordinateSystem* pVCooSys = m_rVCooSysList[i];
        uno::Reference<XCoordinateSystem> xCooSys = pVCooSys->getModel();
        sal_Int32 nDimCount = xCooSys->getDimension();

        for (sal_Int32 nDimIndex = 0; nDimIndex < nDimCount; ++nDimIndex)
        {
            bool bDateAxisAllowed = ChartTypeHelper::isSupportingDateAxis(
                AxisHelper::getChartTypeByIndex(xCooSys, 0), nDimCount, nDimIndex);

            // Each dimension may have primary and secondary axes.
            const sal_Int32 nMaxAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimIndex);
            for (sal_Int32 nAxisIndex = 0; nAxisIndex <= nMaxAxisIndex; ++nAxisIndex)
            {
                uno::Reference<XAxis> xAxis = xCooSys->getAxisByDimension(nDimIndex, nAxisIndex);

                if (!xAxis.is())
                    continue;

                if (m_aAxisUsageList.find(xAxis) == m_aAxisUsageList.end())
                {
                    // Create axis usage object for this axis.

                    chart2::ScaleData aSourceScale = xAxis->getScaleData();
                    ExplicitCategoriesProvider* pCatProvider = pVCooSys->getExplicitCategoriesProvider();
                    if (nDimIndex == 0)
                        AxisHelper::checkDateAxis( aSourceScale, pCatProvider, bDateAxisAllowed );

                    bool bHasComplexCat = pCatProvider && pCatProvider->hasComplexCategories();
                    aSourceScale.ShiftedCategoryPosition = isCategoryPositionShifted(aSourceScale, bHasComplexCat);

                    m_aAxisUsageList[xAxis].aAutoScaling = ScaleAutomatism(aSourceScale, rNullDate);
                }

                AxisUsage& rAxisUsage = m_aAxisUsageList[xAxis];
                rAxisUsage.addCoordinateSystem(pVCooSys, nDimIndex, nAxisIndex);
            }
        }
    }

    // Determine the highest axis index of all dimensions.
    ::std::map< uno::Reference< XAxis >, AxisUsage >::iterator             aAxisIter    = m_aAxisUsageList.begin();
    const ::std::map< uno::Reference< XAxis >, AxisUsage >::const_iterator aAxisEndIter = m_aAxisUsageList.end();
    m_nMaxAxisIndex = 0;
    for (size_t i = 0, n = m_rVCooSysList.size(); i < n; ++i)
    {
        VCoordinateSystem* pVCooSys = m_rVCooSysList[i];
        uno::Reference<XCoordinateSystem> xCooSys = pVCooSys->getModel();
        sal_Int32 nDimCount = xCooSys->getDimension();

        for (sal_Int32 nDimIndex = 0; nDimIndex < nDimCount; ++nDimIndex)
        {
            for (aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; ++aAxisIter)
            {
                sal_Int32 nLocalMax = aAxisIter->second.getMaxAxisIndexForDimension(nDimIndex);
                if (m_nMaxAxisIndex < nLocalMax)
                    m_nMaxAxisIndex = nLocalMax;
            }
        }
    }
}

void SeriesPlotterContainer::setScalesFromCooSysToPlotter()
{
    //set scales to plotter to enable them to provide the preferred scene AspectRatio
    for( std::unique_ptr<VSeriesPlotter>& aPlotter : m_aSeriesPlotterList )
    {
        VSeriesPlotter* pSeriesPlotter = aPlotter.get();
        VCoordinateSystem* pVCooSys = lcl_getCooSysForPlotter( m_rVCooSysList, pSeriesPlotter );
        if(pVCooSys)
        {
            pSeriesPlotter->setScales( pVCooSys->getExplicitScales(0,0), pVCooSys->getPropertySwapXAndYAxis() );
            sal_Int32 nMaxAxisIndex = pVCooSys->getMaximumAxisIndexByDimension(1);//only additional value axis are relevant for series plotter
            for( sal_Int32 nI=1; nI<=nMaxAxisIndex; nI++ )
                pSeriesPlotter->addSecondaryValueScale( pVCooSys->getExplicitScale(1,nI), nI );
        }
    }
}

void SeriesPlotterContainer::setNumberFormatsFromAxes()
{
    //set numberformats to plotter to enable them to display the data labels in the numberformat of the axis
    for( std::unique_ptr<VSeriesPlotter>& aPlotter : m_aSeriesPlotterList )
    {
        VSeriesPlotter* pSeriesPlotter = aPlotter.get();
        VCoordinateSystem* pVCooSys = lcl_getCooSysForPlotter( m_rVCooSysList, pSeriesPlotter );
        if(pVCooSys)
        {
            AxesNumberFormats aAxesNumberFormats;
            uno::Reference< XCoordinateSystem > xCooSys = pVCooSys->getModel();
            sal_Int32 nDimensionCount = xCooSys->getDimension();
            for(sal_Int32 nDimensionIndex=0; nDimensionIndex<nDimensionCount; ++nDimensionIndex)
            {
                const sal_Int32 nMaximumAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
                for(sal_Int32 nAxisIndex=0; nAxisIndex<=nMaximumAxisIndex; ++nAxisIndex)
                {
                    try
                    {
                        Reference< beans::XPropertySet > xAxisProp( xCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex ), uno::UNO_QUERY );
                        if( xAxisProp.is())
                        {
                            sal_Int32 nNumberFormatKey(0);
                            if( xAxisProp->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nNumberFormatKey )
                            {
                                aAxesNumberFormats.setFormat( nNumberFormatKey, nDimensionIndex, nAxisIndex );
                            }
                            else if( nDimensionIndex==0 )
                            {
                                //provide a default date format for date axis with own data
                                aAxesNumberFormats.setFormat( m_nDefaultDateNumberFormat, nDimensionIndex, nAxisIndex );
                            }
                        }
                    }
                    catch( const lang::IndexOutOfBoundsException& e )
                    {
                        ASSERT_EXCEPTION( e );
                    }
                }
            }
            pSeriesPlotter->setAxesNumberFormats( aAxesNumberFormats );
        }
    }
}

void SeriesPlotterContainer::updateScalesAndIncrementsOnAxes()
{
    for( size_t nC=0; nC < m_rVCooSysList.size(); nC++)
        m_rVCooSysList[nC]->updateScalesAndIncrementsOnAxes();
}

void SeriesPlotterContainer::doAutoScaling( ChartModel& rChartModel )
{
    if (m_aSeriesPlotterList.empty() || m_aAxisUsageList.empty())
        // We need these two containers populated to do auto-scaling.  Bail out.
        return;

    ::std::map< uno::Reference< XAxis >, AxisUsage >::iterator             aAxisIter    = m_aAxisUsageList.begin();
    const ::std::map< uno::Reference< XAxis >, AxisUsage >::const_iterator aAxisEndIter = m_aAxisUsageList.end();

    //iterate over the main scales first than secondary axis
    for (sal_Int32 nAxisIndex = 0; nAxisIndex <= m_nMaxAxisIndex; ++nAxisIndex)
    {
        // - first do autoscale for all x and z scales (because they are treated independent)
        for( aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; ++aAxisIter )
        {
            AxisUsage& rAxisUsage = (*aAxisIter).second;

            rAxisUsage.prepareAutomaticAxisScaling(rAxisUsage.aAutoScaling, 0, nAxisIndex);
            rAxisUsage.prepareAutomaticAxisScaling(rAxisUsage.aAutoScaling, 2, nAxisIndex);

            ExplicitScaleData       aExplicitScale;
            ExplicitIncrementData   aExplicitIncrement;
            rAxisUsage.aAutoScaling.calculateExplicitScaleAndIncrement( aExplicitScale, aExplicitIncrement );

            rAxisUsage.setExplicitScaleAndIncrement(0, nAxisIndex, aExplicitScale, aExplicitIncrement);
            rAxisUsage.setExplicitScaleAndIncrement(2, nAxisIndex, aExplicitScale, aExplicitIncrement);
        }

        // - second do autoscale for the dependent y scales (the coordinate systems are prepared with x and z scales already )
        for( aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; ++aAxisIter )
        {
            AxisUsage& rAxisUsage = (*aAxisIter).second;

            rAxisUsage.prepareAutomaticAxisScaling(rAxisUsage.aAutoScaling, 1, nAxisIndex);

            ExplicitScaleData       aExplicitScale;
            ExplicitIncrementData   aExplicitIncrement;
            rAxisUsage.aAutoScaling.calculateExplicitScaleAndIncrement( aExplicitScale, aExplicitIncrement );

            rAxisUsage.setExplicitScaleAndIncrement(0, nAxisIndex, aExplicitScale, aExplicitIncrement);
            rAxisUsage.setExplicitScaleAndIncrement(1, nAxisIndex, aExplicitScale, aExplicitIncrement);
            rAxisUsage.setExplicitScaleAndIncrement(2, nAxisIndex, aExplicitScale, aExplicitIncrement);
        }
    }
    AdaptScaleOfYAxisWithoutAttachedSeries( rChartModel );
}

void SeriesPlotterContainer::AdaptScaleOfYAxisWithoutAttachedSeries( ChartModel& rModel )
{
    //issue #i80518#

    ::std::map< uno::Reference< XAxis >, AxisUsage >::iterator             aAxisIter    = m_aAxisUsageList.begin();
    const ::std::map< uno::Reference< XAxis >, AxisUsage >::const_iterator aAxisEndIter = m_aAxisUsageList.end();

    for( sal_Int32 nAxisIndex=0; nAxisIndex<=m_nMaxAxisIndex; nAxisIndex++ )
    {
        for( aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; ++aAxisIter )
        {
            AxisUsage& rAxisUsage = (*aAxisIter).second;
            ::std::vector< VCoordinateSystem* > aVCooSysList_Y = rAxisUsage.getCoordinateSystems( 1, nAxisIndex );
            if( aVCooSysList_Y.empty() )
                continue;

            uno::Reference< XDiagram > xDiagram( rModel.getFirstDiagram() );
            if (!xDiagram.is())
                continue;

            bool bSeriesAttachedToThisAxis = false;
            sal_Int32 nAttachedAxisIndex = -1;
            {
                ::std::vector< Reference< XDataSeries > > aSeriesVector( DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
                ::std::vector< Reference< XDataSeries > >::const_iterator aIter = aSeriesVector.begin();
                for( ; aIter != aSeriesVector.end(); ++aIter )
                {
                    sal_Int32 nCurrentIndex = DataSeriesHelper::getAttachedAxisIndex( *aIter );
                    if( nAxisIndex == nCurrentIndex )
                    {
                        bSeriesAttachedToThisAxis = true;
                        break;
                    }
                    else if( nAttachedAxisIndex<0 || nAttachedAxisIndex>nCurrentIndex )
                        nAttachedAxisIndex=nCurrentIndex;
                }
            }

            if (bSeriesAttachedToThisAxis || nAttachedAxisIndex < 0)
                continue;

            for( size_t nC = 0; nC < aVCooSysList_Y.size(); ++nC )
            {
                aVCooSysList_Y[nC]->prepareAutomaticAxisScaling( rAxisUsage.aAutoScaling, 1, nAttachedAxisIndex );

                ExplicitScaleData aExplicitScaleSource = aVCooSysList_Y[nC]->getExplicitScale( 1,nAttachedAxisIndex );
                ExplicitIncrementData aExplicitIncrementSource = aVCooSysList_Y[nC]->getExplicitIncrement( 1,nAttachedAxisIndex );

                ExplicitScaleData aExplicitScaleDest = aVCooSysList_Y[nC]->getExplicitScale( 1,nAxisIndex );
                ExplicitIncrementData aExplicitIncrementDest = aVCooSysList_Y[nC]->getExplicitIncrement( 1,nAxisIndex );

                aExplicitScaleDest.Orientation = aExplicitScaleSource.Orientation;
                aExplicitScaleDest.Scaling = aExplicitScaleSource.Scaling;
                aExplicitScaleDest.AxisType = aExplicitScaleSource.AxisType;

                aExplicitIncrementDest.BaseValue = aExplicitIncrementSource.BaseValue;

                ScaleData aScale( rAxisUsage.aAutoScaling.getScale() );
                if( !aScale.Minimum.hasValue() )
                {
                    bool bNewMinOK = true;
                    double fMax=0.0;
                    if( aScale.Maximum >>= fMax )
                        bNewMinOK = (aExplicitScaleSource.Minimum <= fMax);
                    if( bNewMinOK )
                        aExplicitScaleDest.Minimum = aExplicitScaleSource.Minimum;
                }
                else
                    aExplicitIncrementDest.BaseValue = aExplicitScaleDest.Minimum;

                if( !aScale.Maximum.hasValue() )
                {
                    bool bNewMaxOK = true;
                    double fMin=0.0;
                    if( aScale.Minimum >>= fMin )
                        bNewMaxOK = (fMin <= aExplicitScaleSource.Maximum);
                    if( bNewMaxOK )
                        aExplicitScaleDest.Maximum = aExplicitScaleSource.Maximum;
                }
                if( !aScale.Origin.hasValue() )
                    aExplicitScaleDest.Origin = aExplicitScaleSource.Origin;

                if( !aScale.IncrementData.Distance.hasValue() )
                    aExplicitIncrementDest.Distance = aExplicitIncrementSource.Distance;

                bool bAutoMinorInterval = true;
                if( aScale.IncrementData.SubIncrements.getLength() )
                    bAutoMinorInterval = !( aScale.IncrementData.SubIncrements[0].IntervalCount.hasValue() );
                if( bAutoMinorInterval )
                {
                    if( !aExplicitIncrementDest.SubIncrements.empty() && !aExplicitIncrementSource.SubIncrements.empty() )
                        aExplicitIncrementDest.SubIncrements[0].IntervalCount =
                            aExplicitIncrementSource.SubIncrements[0].IntervalCount;
                }

                aVCooSysList_Y[nC]->setExplicitScaleAndIncrement( 1, nAxisIndex, aExplicitScaleDest, aExplicitIncrementDest );
            }
        }
    }

    if( AxisHelper::isAxisPositioningEnabled() )
    {
        //correct origin for y main axis (the origin is where the other main axis crosses)
        sal_Int32 nAxisIndex=0;
        sal_Int32 nDimensionIndex=1;
        for( aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; ++aAxisIter )
        {
            AxisUsage& rAxisUsage = (*aAxisIter).second;
            ::std::vector< VCoordinateSystem* > aVCooSysList = rAxisUsage.getCoordinateSystems(nDimensionIndex,nAxisIndex);
            size_t nC;
            for( nC=0; nC < aVCooSysList.size(); nC++)
            {
                ExplicitScaleData aExplicitScale( aVCooSysList[nC]->getExplicitScale( nDimensionIndex, nAxisIndex ) );
                ExplicitIncrementData aExplicitIncrement( aVCooSysList[nC]->getExplicitIncrement( nDimensionIndex, nAxisIndex ) );

                Reference< chart2::XCoordinateSystem > xCooSys( aVCooSysList[nC]->getModel() );
                Reference< XAxis > xAxis( xCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex ) );
                Reference< beans::XPropertySet > xCrossingMainAxis( AxisHelper::getCrossingMainAxis( xAxis, xCooSys ), uno::UNO_QUERY );

                ::com::sun::star::chart::ChartAxisPosition eCrossingMainAxisPos( ::com::sun::star::chart::ChartAxisPosition_ZERO );
                if( xCrossingMainAxis.is() )
                {
                    xCrossingMainAxis->getPropertyValue("CrossoverPosition") >>= eCrossingMainAxisPos;
                    if( ::com::sun::star::chart::ChartAxisPosition_VALUE == eCrossingMainAxisPos )
                    {
                        double fValue = 0.0;
                        xCrossingMainAxis->getPropertyValue("CrossoverValue") >>= fValue;
                        aExplicitScale.Origin = fValue;
                    }
                    else if( ::com::sun::star::chart::ChartAxisPosition_ZERO == eCrossingMainAxisPos )
                        aExplicitScale.Origin = 0.0;
                    else  if( ::com::sun::star::chart::ChartAxisPosition_START == eCrossingMainAxisPos )
                        aExplicitScale.Origin = aExplicitScale.Minimum;
                    else  if( ::com::sun::star::chart::ChartAxisPosition_END == eCrossingMainAxisPos )
                        aExplicitScale.Origin = aExplicitScale.Maximum;
                }

                aVCooSysList[nC]->setExplicitScaleAndIncrement( nDimensionIndex, nAxisIndex, aExplicitScale, aExplicitIncrement );
            }
        }
    }
}

drawing::Direction3D SeriesPlotterContainer::getPreferredAspectRatio()
{
    drawing::Direction3D aPreferredAspectRatio(1.0,1.0,1.0);

    sal_Int32 nPlotterCount=0;
    //get a list of all preferred aspect ratios and combine them
    //first with special demands wins (less or equal zero <-> arbitrary)
    double fx, fy, fz;
    fx = fy = fz = -1.0;
    for( std::unique_ptr<VSeriesPlotter>& aPlotter : m_aSeriesPlotterList )
    {
        drawing::Direction3D aSingleRatio( aPlotter->getPreferredDiagramAspectRatio() );
        if( fx<0 && aSingleRatio.DirectionX>0 )
            fx = aSingleRatio.DirectionX;

        if( fy<0 && aSingleRatio.DirectionY>0 )
        {
            if( fx>0 && aSingleRatio.DirectionX>0 )
                fy = fx*aSingleRatio.DirectionY/aSingleRatio.DirectionX;
            else if( fz>0 && aSingleRatio.DirectionZ>0 )
                fy = fz*aSingleRatio.DirectionY/aSingleRatio.DirectionZ;
            else
                fy = aSingleRatio.DirectionY;
        }

        if( fz<0 && aSingleRatio.DirectionZ>0 )
        {
            if( fx>0 && aSingleRatio.DirectionX>0 )
                fz = fx*aSingleRatio.DirectionZ/aSingleRatio.DirectionX;
            else if( fy>0 && aSingleRatio.DirectionY>0 )
                fz = fy*aSingleRatio.DirectionZ/aSingleRatio.DirectionY;
            else
                fz = aSingleRatio.DirectionZ;
        }

        if( fx>0 && fy>0 && fz>0 )
            break;
        ++nPlotterCount;
    }
    aPreferredAspectRatio = drawing::Direction3D(fx, fy, fz);
    return aPreferredAspectRatio;
}

}

struct CreateShapeParam2D
{
    css::awt::Rectangle maRemainingSpace;

    std::shared_ptr<SeriesPlotterContainer> mpSeriesPlotterContainer;

    std::shared_ptr<VTitle> mpVTitleX;
    std::shared_ptr<VTitle> mpVTitleY;
    std::shared_ptr<VTitle> mpVTitleZ;

    std::shared_ptr<VTitle> mpVTitleSecondX;
    std::shared_ptr<VTitle> mpVTitleSecondY;

    css::uno::Reference<css::drawing::XShape> mxMarkHandles;
    css::uno::Reference<css::drawing::XShape> mxPlotAreaWithAxes;

    css::uno::Reference<css::drawing::XShapes> mxDiagramWithAxesShapes;

    bool mbAutoPosTitleX;
    bool mbAutoPosTitleY;
    bool mbAutoPosTitleZ;

    bool mbAutoPosSecondTitleX;
    bool mbAutoPosSecondTitleY;

    bool mbUseFixedInnerSize;

    CreateShapeParam2D() :
        mbAutoPosTitleX(true),
        mbAutoPosTitleY(true),
        mbAutoPosTitleZ(true),
        mbAutoPosSecondTitleX(true),
        mbAutoPosSecondTitleY(true),
        mbUseFixedInnerSize(false) {}
};

class GL2DRenderer : public IRenderer
{
public:
    explicit GL2DRenderer(ChartView* pView);
    virtual ~GL2DRenderer();

    virtual void update() override;
    virtual void clickedAt(const Point& rPos, sal_uInt16 nButton) override;
    virtual void mouseDragMove(const Point& rBegin, const Point& rEnd, sal_uInt16 nButton) override;
    virtual void scroll(long nDelta) override;
    virtual void contextDestroyed() override;

    const OpenGLWindow* getOpenGLWindow() const;
    void updateOpenGLWindow();
private:
    ChartView* mpView;
    bool mbContextDestroyed;
    VclPtr<OpenGLWindow> mpWindow;
};

GL2DRenderer::GL2DRenderer(ChartView* pView):
    mpView(pView),
    mbContextDestroyed(false),
    mpWindow(mpView->mrChartModel.getOpenGLWindow())
{
}

GL2DRenderer::~GL2DRenderer()
{
    SolarMutexGuard g;
    if(!mbContextDestroyed && mpWindow)
        mpWindow->setRenderer(nullptr);
    mpWindow.reset();
}

void GL2DRenderer::update()
{
    mpView->update();
    mpView->render();
}

void GL2DRenderer::clickedAt(const Point&, sal_uInt16 )
{
}

void GL2DRenderer::mouseDragMove(const Point& , const Point& , sal_uInt16 )
{
}

void GL2DRenderer::scroll(long )
{
}

void GL2DRenderer::contextDestroyed()
{
    mbContextDestroyed = true;
}

const OpenGLWindow* GL2DRenderer::getOpenGLWindow() const
{
    return mpWindow;
}

void GL2DRenderer::updateOpenGLWindow()
{
    if(mbContextDestroyed)
        return;

    OpenGLWindow* pWindow = mpView->mrChartModel.getOpenGLWindow();
    if(pWindow != mpWindow)
    {
        if(mpWindow)
        {
            mpWindow->setRenderer(nullptr);
        }

        if(pWindow)
        {
            pWindow->setRenderer(this);
        }
    }
    mpWindow = pWindow;
}

const uno::Sequence<sal_Int8>& ExplicitValueProvider::getUnoTunnelId()
{
    return theExplicitValueProviderUnoTunnelId::get().getSeq();
}

ExplicitValueProvider* ExplicitValueProvider::getExplicitValueProvider(
        const Reference< uno::XInterface >& xChartView )
{
    ExplicitValueProvider* pExplicitValueProvider=nullptr;

    Reference< lang::XUnoTunnel > xTunnel( xChartView, uno::UNO_QUERY );
    if( xTunnel.is() )
    {
        pExplicitValueProvider = reinterpret_cast<ExplicitValueProvider*>(xTunnel->getSomething(
            ExplicitValueProvider::getUnoTunnelId() ));
    }
    return pExplicitValueProvider;
}

ChartView::ChartView(
        uno::Reference<uno::XComponentContext> const & xContext,
        ChartModel& rModel)
    : m_aMutex()
    , m_xCC(xContext)
    , mrChartModel(rModel)
    , m_xShapeFactory()
    , m_xDrawPage()
    , m_pDrawModelWrapper()
    , m_aListenerContainer( m_aMutex )
    , m_bViewDirty(true)
    , m_bInViewUpdate(false)
    , m_bViewUpdatePending(false)
    , m_bRefreshAddIn(true)
    , m_aPageResolution(1000,1000)
    , m_bPointsWereSkipped(false)
    , m_nScaleXNumerator(1)
    , m_nScaleXDenominator(1)
    , m_nScaleYNumerator(1)
    , m_nScaleYDenominator(1)
    , m_bSdrViewIsInEditMode(false)
    , m_aResultingDiagramRectangleExcludingAxes(0,0,0,0)
    , mp2DRenderer(new GL2DRenderer(this))
{
    init();
}

void ChartView::init()
{
    if( !m_pDrawModelWrapper.get() )
    {
        SolarMutexGuard aSolarGuard;
        m_pDrawModelWrapper = std::shared_ptr< DrawModelWrapper >( new DrawModelWrapper( m_xCC ) );
        m_xShapeFactory = m_pDrawModelWrapper->getShapeFactory();
        m_xDrawPage = m_pDrawModelWrapper->getMainDrawPage();
        StartListening( m_pDrawModelWrapper->getSdrModel() );
    }
}

void SAL_CALL ChartView::initialize( const uno::Sequence< uno::Any >& )
                throw ( uno::Exception, uno::RuntimeException, std::exception)
{
    init();
}

ChartView::~ChartView()
{
    maTimeBased.maTimer.Stop();
    // #i120831#. In ChartView::initialize(), m_xShapeFactory is created from SdrModel::getUnoModel() and indirectly
    //   from SfxBaseModel, it needs call dispose() to make sure SfxBaseModel object is freed correctly.
    uno::Reference< lang::XComponent > xComp( m_xShapeFactory, uno::UNO_QUERY);
    if ( xComp.is() )
        xComp->dispose();

    if( m_pDrawModelWrapper.get() )
    {
        SolarMutexGuard aSolarGuard;
        EndListening( m_pDrawModelWrapper->getSdrModel() );
        m_pDrawModelWrapper.reset();
    }
    m_xDrawPage = nullptr;
    impl_deleteCoordinateSystems();
}

void ChartView::impl_deleteCoordinateSystems()
{
    //delete all coordinate systems
    ::std::vector< VCoordinateSystem* > aVectorToDeleteObjects;
    ::std::swap( aVectorToDeleteObjects, m_aVCooSysList );//#i109770#
    ::std::vector< VCoordinateSystem* >::const_iterator       aIter = aVectorToDeleteObjects.begin();
    const ::std::vector< VCoordinateSystem* >::const_iterator aEnd  = aVectorToDeleteObjects.end();
    for( ; aIter != aEnd; ++aIter )
    {
        delete *aIter;
    }
    aVectorToDeleteObjects.clear();
}

// datatransfer::XTransferable
namespace
{
const OUString lcl_aGDIMetaFileMIMEType(
    "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"" );
const OUString lcl_aGDIMetaFileMIMETypeHighContrast(
    "application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\"" );
} // anonymous namespace

void ChartView::getMetaFile( const uno::Reference< io::XOutputStream >& xOutStream
                           , bool bUseHighContrast )
{
    if( !m_xDrawPage.is() )
        return;

    // creating the graphic exporter
    uno::Reference< drawing::XGraphicExportFilter > xExporter = drawing::GraphicExportFilter::create( m_xCC );

    uno::Sequence< beans::PropertyValue > aProps(3);
    aProps[0].Name = "FilterName";
    aProps[0].Value <<= OUString("SVM");

    aProps[1].Name = "OutputStream";
    aProps[1].Value <<= xOutStream;

    uno::Sequence< beans::PropertyValue > aFilterData(4);
    aFilterData[0].Name = "ExportOnlyBackground";
    aFilterData[0].Value <<= sal_False;
    aFilterData[1].Name = "HighContrast";
    aFilterData[1].Value <<= bUseHighContrast;

    aFilterData[2].Name = "Version";
    const sal_Int32 nVersion = SOFFICE_FILEFORMAT_50;
    aFilterData[2].Value <<= nVersion;

    aFilterData[3].Name = "CurrentPage";
    aFilterData[3].Value <<= uno::Reference< uno::XInterface >( m_xDrawPage, uno::UNO_QUERY );

    //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
    {
        aFilterData.realloc( aFilterData.getLength()+4 );
        aFilterData[4].Name = "ScaleXNumerator";
        aFilterData[4].Value = uno::makeAny( m_nScaleXNumerator );
        aFilterData[5].Name = "ScaleXDenominator";
        aFilterData[5].Value = uno::makeAny( m_nScaleXDenominator );
        aFilterData[6].Name = "ScaleYNumerator";
        aFilterData[6].Value = uno::makeAny( m_nScaleYNumerator );
        aFilterData[7].Name = "ScaleYDenominator";
        aFilterData[7].Value = uno::makeAny( m_nScaleYDenominator );
    }

    aProps[2].Name = "FilterData";
    aProps[2].Value <<= aFilterData;

    xExporter->setSourceDocument( uno::Reference< lang::XComponent >( m_xDrawPage, uno::UNO_QUERY) );
    if( xExporter->filter( aProps ) )
    {
        xOutStream->flush();
        xOutStream->closeOutput();
        uno::Reference< io::XSeekable > xSeekable( xOutStream, uno::UNO_QUERY );
        if( xSeekable.is() )
            xSeekable->seek(0);
    }
}

uno::Any SAL_CALL ChartView::getTransferData( const datatransfer::DataFlavor& aFlavor )
                throw (datatransfer::UnsupportedFlavorException, io::IOException, uno::RuntimeException, std::exception)
{
    bool bHighContrastMetaFile( aFlavor.MimeType.equals(lcl_aGDIMetaFileMIMETypeHighContrast));
    uno::Any aRet;
    if( ! (bHighContrastMetaFile || aFlavor.MimeType.equals(lcl_aGDIMetaFileMIMEType)) )
        return aRet;

    update();

    SvMemoryStream aStream( 1024, 1024 );
    utl::OStreamWrapper* pStreamWrapper = new utl::OStreamWrapper( aStream );

    uno::Reference< io::XOutputStream > xOutStream( pStreamWrapper );
    uno::Reference< io::XInputStream > xInStream( pStreamWrapper );
    uno::Reference< io::XSeekable > xSeekable( pStreamWrapper );

    if( xOutStream.is() )
    {
        this->getMetaFile( xOutStream, bHighContrastMetaFile );

        if( xInStream.is() && xSeekable.is() )
        {
            xSeekable->seek(0);
            sal_Int32 nBytesToRead = xInStream->available();
            uno::Sequence< sal_Int8 > aSeq( nBytesToRead );
            xInStream->readBytes( aSeq, nBytesToRead);
            aRet <<= aSeq;
            xInStream->closeInput();
        }
    }

    return aRet;
}
uno::Sequence< datatransfer::DataFlavor > SAL_CALL ChartView::getTransferDataFlavors()
                throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< datatransfer::DataFlavor > aRet(2);

    aRet[0] = datatransfer::DataFlavor( lcl_aGDIMetaFileMIMEType,
        "GDIMetaFile",
        cppu::UnoType<uno::Sequence< sal_Int8 >>::get() );
    aRet[1] = datatransfer::DataFlavor( lcl_aGDIMetaFileMIMETypeHighContrast,
        "GDIMetaFile",
        cppu::UnoType<uno::Sequence< sal_Int8 >>::get() );

    return aRet;
}
sal_Bool SAL_CALL ChartView::isDataFlavorSupported( const datatransfer::DataFlavor& aFlavor )
                throw (uno::RuntimeException, std::exception)
{
    return ( aFlavor.MimeType.equals(lcl_aGDIMetaFileMIMEType) ||
             aFlavor.MimeType.equals(lcl_aGDIMetaFileMIMETypeHighContrast) );
}

// ____ XUnoTunnel ___
::sal_Int64 SAL_CALL ChartView::getSomething( const uno::Sequence< ::sal_Int8 >& aIdentifier )
        throw( uno::RuntimeException, std::exception)
{
    if( aIdentifier.getLength() == 16 && 0 == memcmp( ExplicitValueProvider::getUnoTunnelId().getConstArray(),
                                                         aIdentifier.getConstArray(), 16 ) )
    {
        ExplicitValueProvider* pProvider = this;
        return reinterpret_cast<sal_Int64>(pProvider);
    }
    return 0;
}

// lang::XServiceInfo

OUString SAL_CALL ChartView::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString ChartView::getImplementationName_Static()
{
    return OUString(CHART_VIEW_SERVICE_IMPLEMENTATION_NAME);
}

sal_Bool SAL_CALL ChartView::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ChartView::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > ChartView::getSupportedServiceNames_Static()
{
    uno::Sequence<OUString> aSNS { CHART_VIEW_SERVICE_NAME };
    return aSNS;
}

::basegfx::B3DHomMatrix createTransformationSceneToScreen(
    const ::basegfx::B2IRectangle& rDiagramRectangleWithoutAxes )
{
    ::basegfx::B3DHomMatrix aM;
    aM.scale(double(rDiagramRectangleWithoutAxes.getWidth())/FIXED_SIZE_FOR_3D_CHART_VOLUME
            , -double(rDiagramRectangleWithoutAxes.getHeight())/FIXED_SIZE_FOR_3D_CHART_VOLUME, 1.0 );
    aM.translate(double(rDiagramRectangleWithoutAxes.getMinX())
        , double(rDiagramRectangleWithoutAxes.getMinY()+rDiagramRectangleWithoutAxes.getHeight()-1), 0);
    return aM;
}

namespace
{

bool lcl_IsPieOrDonut( const uno::Reference< XDiagram >& xDiagram )
{
    //special treatment for pie charts
    //the size is checked after complete creation to get the datalabels into the given space

    //todo: this is just a workaround at the moment for pie and donut labels
    return DiagramHelper::isPieOrDonutChart( xDiagram );
}

void lcl_setDefaultWritingMode( std::shared_ptr< DrawModelWrapper > pDrawModelWrapper, ChartModel& rModel)
{
    //get writing mode from parent document:
    if( SvtLanguageOptions().IsCTLFontEnabled() )
    {
        try
        {
            sal_Int16 nWritingMode=-1;
            uno::Reference< beans::XPropertySet > xParentProps( rModel.getParent(), uno::UNO_QUERY );
            uno::Reference< style::XStyleFamiliesSupplier > xStyleFamiliesSupplier( xParentProps, uno::UNO_QUERY );
            if( xStyleFamiliesSupplier.is() )
            {
                uno::Reference< container::XNameAccess > xStylesFamilies( xStyleFamiliesSupplier->getStyleFamilies() );
                if( xStylesFamilies.is() )
                {
                    if( !xStylesFamilies->hasByName( "PageStyles" ) )
                    {
                        //draw/impress is parent document
                        uno::Reference< lang::XMultiServiceFactory > xFatcory( xParentProps, uno::UNO_QUERY );
                        if( xFatcory.is() )
                        {
                            uno::Reference< beans::XPropertySet > xDrawDefaults( xFatcory->createInstance( "com.sun.star.drawing.Defaults" ), uno::UNO_QUERY );
                            if( xDrawDefaults.is() )
                                xDrawDefaults->getPropertyValue( "WritingMode" ) >>= nWritingMode;
                        }
                    }
                    else
                    {
                        uno::Reference< container::XNameAccess > xPageStyles( xStylesFamilies->getByName( "PageStyles" ), uno::UNO_QUERY );
                        if( xPageStyles.is() )
                        {
                            OUString aPageStyle;

                            uno::Reference< text::XTextDocument > xTextDocument( xParentProps, uno::UNO_QUERY );
                            if( xTextDocument.is() )
                            {
                                //writer is parent document
                                //retrieve the current page style from the text cursor property PageStyleName

                                uno::Reference< text::XTextEmbeddedObjectsSupplier > xTextEmbeddedObjectsSupplier( xTextDocument, uno::UNO_QUERY );
                                if( xTextEmbeddedObjectsSupplier.is() )
                                {
                                    uno::Reference< container::XNameAccess > xEmbeddedObjects( xTextEmbeddedObjectsSupplier->getEmbeddedObjects() );
                                    if( xEmbeddedObjects.is() )
                                    {
                                        uno::Sequence< OUString > aNames( xEmbeddedObjects->getElementNames() );

                                        sal_Int32 nCount = aNames.getLength();
                                        for( sal_Int32 nN=0; nN<nCount; nN++ )
                                        {
                                            uno::Reference< beans::XPropertySet > xEmbeddedProps( xEmbeddedObjects->getByName( aNames[nN] ), uno::UNO_QUERY );
                                            if( xEmbeddedProps.is() )
                                            {
                                                static OUString aChartCLSID = OUString( SvGlobalName( SO3_SCH_CLASSID ).GetHexName());
                                                OUString aCLSID;
                                                xEmbeddedProps->getPropertyValue( "CLSID" ) >>= aCLSID;
                                                if( aCLSID.equals(aChartCLSID) )
                                                {
                                                    uno::Reference< text::XTextContent > xEmbeddedObject( xEmbeddedProps, uno::UNO_QUERY );
                                                    if( xEmbeddedObject.is() )
                                                    {
                                                        uno::Reference< text::XTextRange > xAnchor( xEmbeddedObject->getAnchor() );
                                                        if( xAnchor.is() )
                                                        {
                                                            uno::Reference< beans::XPropertySet > xAnchorProps( xAnchor, uno::UNO_QUERY );
                                                            if( xAnchorProps.is() )
                                                            {
                                                                xAnchorProps->getPropertyValue( "WritingMode" ) >>= nWritingMode;
                                                            }
                                                            uno::Reference< text::XText > xText( xAnchor->getText() );
                                                            if( xText.is() )
                                                            {
                                                                uno::Reference< beans::XPropertySet > xTextCursorProps( xText->createTextCursor(), uno::UNO_QUERY );
                                                                if( xTextCursorProps.is() )
                                                                    xTextCursorProps->getPropertyValue( "PageStyleName" ) >>= aPageStyle;
                                                            }
                                                        }
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                if( aPageStyle.isEmpty() )
                                {
                                    uno::Reference< text::XText > xText( xTextDocument->getText() );
                                    if( xText.is() )
                                    {
                                        uno::Reference< beans::XPropertySet > xTextCursorProps( xText->createTextCursor(), uno::UNO_QUERY );
                                        if( xTextCursorProps.is() )
                                            xTextCursorProps->getPropertyValue( "PageStyleName" ) >>= aPageStyle;
                                    }
                                }
                            }
                            else
                            {
                                //Calc is parent document
                                xParentProps->getPropertyValue( "PageStyle" ) >>= aPageStyle;
                                if(aPageStyle.isEmpty())
                                    aPageStyle = "Default";
                            }
                            if( nWritingMode == -1 || nWritingMode == text::WritingMode2::PAGE )
                            {
                                uno::Reference< beans::XPropertySet > xPageStyle( xPageStyles->getByName( aPageStyle ), uno::UNO_QUERY );
                                if( xPageStyle.is() )
                                    xPageStyle->getPropertyValue( "WritingMode" ) >>= nWritingMode;
                            }
                        }
                    }
                }
            }
            if( nWritingMode != -1 && nWritingMode != text::WritingMode2::PAGE )
            {
                if( pDrawModelWrapper.get() )
                    pDrawModelWrapper->GetItemPool().SetPoolDefaultItem(SvxFrameDirectionItem(static_cast<SvxFrameDirection>(nWritingMode), EE_PARA_WRITINGDIR) );
            }
        }
        catch( const uno::Exception& ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

sal_Int16 lcl_getDefaultWritingModeFromPool( const std::shared_ptr<DrawModelWrapper>& pDrawModelWrapper )
{
    sal_Int16 nWritingMode = text::WritingMode2::LR_TB;
    if(!pDrawModelWrapper)
        return nWritingMode;

    const SfxPoolItem* pItem = &(pDrawModelWrapper->GetItemPool().GetDefaultItem( EE_PARA_WRITINGDIR ));
    if( pItem )
        nWritingMode = static_cast< sal_Int16 >((static_cast< const SvxFrameDirectionItem * >( pItem ))->GetValue());
    return nWritingMode;
}

} //end anonymous namespace

awt::Rectangle ChartView::impl_createDiagramAndContent( const CreateShapeParam2D& rParam, const awt::Size& rPageSize )
{
    //return the used rectangle
    awt::Rectangle aUsedOuterRect(rParam.maRemainingSpace.X, rParam.maRemainingSpace.Y, 0, 0);

    uno::Reference< XDiagram > xDiagram( mrChartModel.getFirstDiagram() );
    if( !xDiagram.is())
        return aUsedOuterRect;

    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
    if(!nDimensionCount)
    {
        //@todo handle mixed dimension
        nDimensionCount = 2;
    }

    basegfx::B2IRectangle aAvailableOuterRect = BaseGFXHelper::makeRectangle(rParam.maRemainingSpace);

    const std::vector< VCoordinateSystem* >& rVCooSysList( rParam.mpSeriesPlotterContainer->getCooSysList() );
    SeriesPlottersType& rSeriesPlotterList = rParam.mpSeriesPlotterContainer->getSeriesPlotterList();

    //create VAxis, so they can give necessary information for automatic scaling
    uno::Reference<chart2::XChartDocument> const xChartDoc(&mrChartModel);
    uno::Reference<util::XNumberFormatsSupplier> const xNumberFormatsSupplier(
            mrChartModel.getNumberFormatsSupplier());
    size_t nC = 0;
    for( nC=0; nC < rVCooSysList.size(); nC++)
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[nC];
        if(3==nDimensionCount)
        {
            uno::Reference<beans::XPropertySet> xSceneProperties( xDiagram, uno::UNO_QUERY );
            CuboidPlanePosition eLeftWallPos( ThreeDHelper::getAutomaticCuboidPlanePositionForStandardLeftWall( xSceneProperties ) );
            CuboidPlanePosition eBackWallPos( ThreeDHelper::getAutomaticCuboidPlanePositionForStandardBackWall( xSceneProperties ) );
            CuboidPlanePosition eBottomPos( ThreeDHelper::getAutomaticCuboidPlanePositionForStandardBottom( xSceneProperties ) );
            pVCooSys->set3DWallPositions( eLeftWallPos, eBackWallPos, eBottomPos );
        }

        pVCooSys->createVAxisList(xChartDoc, rPageSize, rParam.maRemainingSpace);
    }

    // - prepare list of all axis and how they are used
    Date aNullDate = NumberFormatterWrapper( xNumberFormatsSupplier ).getNullDate();
    rParam.mpSeriesPlotterContainer->initAxisUsageList(aNullDate);
    rParam.mpSeriesPlotterContainer->doAutoScaling( mrChartModel );
    rParam.mpSeriesPlotterContainer->setScalesFromCooSysToPlotter();
    rParam.mpSeriesPlotterContainer->setNumberFormatsFromAxes();

    //create shapes

    //aspect ratio
    drawing::Direction3D aPreferredAspectRatio =
        rParam.mpSeriesPlotterContainer->getPreferredAspectRatio();

    uno::Reference< drawing::XShapes > xSeriesTargetInFrontOfAxis(nullptr);
    uno::Reference< drawing::XShapes > xSeriesTargetBehindAxis(nullptr);
    VDiagram aVDiagram(xDiagram, aPreferredAspectRatio, nDimensionCount);
    bool bIsPieOrDonut = lcl_IsPieOrDonut(xDiagram);
    {//create diagram
        aVDiagram.init(rParam.mxDiagramWithAxesShapes, m_xShapeFactory);
        aVDiagram.createShapes(
            awt::Point(rParam.maRemainingSpace.X, rParam.maRemainingSpace.Y),
            awt::Size(rParam.maRemainingSpace.Width, rParam.maRemainingSpace.Height));

        xSeriesTargetInFrontOfAxis = aVDiagram.getCoordinateRegion();
        // It is preferable to use full size than minimum for pie charts
        if (!bIsPieOrDonut && !rParam.mbUseFixedInnerSize)
            aVDiagram.reduceToMimimumSize();
    }

    uno::Reference< drawing::XShapes > xTextTargetShapes =
        AbstractShapeFactory::getOrCreateShapeFactory(m_xShapeFactory)->createGroup2D(rParam.mxDiagramWithAxesShapes);

    // - create axis and grids for all coordinate systems

    //init all coordinate systems
    for( nC=0; nC < rVCooSysList.size(); nC++)
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[nC];
        pVCooSys->initPlottingTargets(xSeriesTargetInFrontOfAxis,xTextTargetShapes,m_xShapeFactory,xSeriesTargetBehindAxis);

        pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
            createTransformationSceneToScreen( aVDiagram.getCurrentRectangle() ) ));

        pVCooSys->initVAxisInList();
    }

    //calculate resulting size respecting axis label layout and fontscaling

    uno::Reference< drawing::XShape > xBoundingShape(rParam.mxDiagramWithAxesShapes, uno::UNO_QUERY);
    ::basegfx::B2IRectangle aConsumedOuterRect;

    //use first coosys only so far; todo: calculate for more than one coosys if we have more in future
    //todo: this is just a workaround at the moment for pie and donut labels
    if( !bIsPieOrDonut && (!rVCooSysList.empty()) )
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[0];
        pVCooSys->createMaximumAxesLabels();

        aConsumedOuterRect = AbstractShapeFactory::getRectangleOfShape(xBoundingShape);
        ::basegfx::B2IRectangle aNewInnerRect( aVDiagram.getCurrentRectangle() );
        if (!rParam.mbUseFixedInnerSize)
            aNewInnerRect = aVDiagram.adjustInnerSize( aConsumedOuterRect );

        pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
            createTransformationSceneToScreen( aNewInnerRect ) ));

        //redo autoscaling to get size and text dependent automatic main increment count
        rParam.mpSeriesPlotterContainer->doAutoScaling( mrChartModel );
        rParam.mpSeriesPlotterContainer->updateScalesAndIncrementsOnAxes();
        rParam.mpSeriesPlotterContainer->setScalesFromCooSysToPlotter();

        pVCooSys->createAxesLabels();

        bool bLessSpaceConsumedThanExpected = false;
        {
            aConsumedOuterRect = AbstractShapeFactory::getRectangleOfShape(xBoundingShape);
            if( aConsumedOuterRect.getMinX() > aAvailableOuterRect.getMinX()
                || aConsumedOuterRect.getMaxX() < aAvailableOuterRect.getMaxX()
                || aConsumedOuterRect.getMinY() > aAvailableOuterRect.getMinY()
                || aConsumedOuterRect.getMinY() < aAvailableOuterRect.getMaxY() )
                bLessSpaceConsumedThanExpected = true;
        }

        if (bLessSpaceConsumedThanExpected && !rParam.mbUseFixedInnerSize)
        {
            aVDiagram.adjustInnerSize( aConsumedOuterRect );
            pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
                createTransformationSceneToScreen( aVDiagram.getCurrentRectangle() ) ));
        }
        pVCooSys->updatePositions();//todo: logically this belongs to the condition above, but it seems also to be necessary to give the axes group shapes the right bounding rects for hit test -  probably caused by bug i106183 -> check again if fixed
    }

    //create axes and grids for the final size
    for( nC=0; nC < rVCooSysList.size(); nC++)
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[nC];

        pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
            createTransformationSceneToScreen( aVDiagram.getCurrentRectangle() ) ));

        pVCooSys->createAxesShapes();
        pVCooSys->createGridShapes();
    }

    // - create data series for all charttypes
    m_bPointsWereSkipped = false;
    for( std::unique_ptr<VSeriesPlotter>& aPlotter : rSeriesPlotterList )
    {
        VSeriesPlotter* pSeriesPlotter = aPlotter.get();
        OUString aCID; //III
        uno::Reference< drawing::XShapes > xSeriesTarget(nullptr);
        if( pSeriesPlotter->WantToPlotInFrontOfAxisLine() )
            xSeriesTarget = xSeriesTargetInFrontOfAxis;
        else
        {
            xSeriesTarget = xSeriesTargetBehindAxis;
            OSL_ENSURE( !bIsPieOrDonut, "not implemented yet! - during a complete recreation this shape is destroyed so no series can be created anymore" );
        }
        pSeriesPlotter->initPlotter( xSeriesTarget,xTextTargetShapes,m_xShapeFactory,aCID );
        pSeriesPlotter->setPageReferenceSize( rPageSize );
        VCoordinateSystem* pVCooSys = lcl_getCooSysForPlotter( rVCooSysList, pSeriesPlotter );
        if(2==nDimensionCount)
            pSeriesPlotter->setTransformationSceneToScreen( pVCooSys->getTransformationSceneToScreen() );
        //better performance for big data
        awt::Size aCoordinateRegionResolution(1000,1000);
        {
            //calculate resolution for coordinate system
            Sequence<sal_Int32> aCoordinateSystemResolution = pVCooSys->getCoordinateSystemResolution( rPageSize, m_aPageResolution );
            pSeriesPlotter->setCoordinateSystemResolution( aCoordinateSystemResolution );
        }

        pSeriesPlotter->createShapes();
        m_bPointsWereSkipped = m_bPointsWereSkipped || pSeriesPlotter->PointsWereSkipped();
    }

    //recreate all with corrected sizes if requested
    if( bIsPieOrDonut )
    {
        m_bPointsWereSkipped = false;

        aConsumedOuterRect = ::basegfx::B2IRectangle( AbstractShapeFactory::getRectangleOfShape(xBoundingShape) );
        ::basegfx::B2IRectangle aNewInnerRect( aVDiagram.getCurrentRectangle() );
        if (!rParam.mbUseFixedInnerSize)
            aNewInnerRect = aVDiagram.adjustInnerSize( aConsumedOuterRect );

        for( std::unique_ptr<VSeriesPlotter>& aPlotter : rSeriesPlotterList )
        {
            aPlotter->releaseShapes();
        }

        //clear and recreate
        AbstractShapeFactory::removeSubShapes( xSeriesTargetInFrontOfAxis ); //xSeriesTargetBehindAxis is a sub shape of xSeriesTargetInFrontOfAxis and will be removed here
        xSeriesTargetBehindAxis.clear();
        AbstractShapeFactory::removeSubShapes( xTextTargetShapes );

        //set new transformation
        for( nC=0; nC < rVCooSysList.size(); nC++)
        {
            VCoordinateSystem* pVCooSys = rVCooSysList[nC];
            pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
                createTransformationSceneToScreen( aNewInnerRect ) ));
        }

        // - create data series for all charttypes
        for( std::unique_ptr<VSeriesPlotter>& aPlotter : rSeriesPlotterList )
        {
            VCoordinateSystem* pVCooSys = lcl_getCooSysForPlotter( rVCooSysList, aPlotter.get() );
            if(2==nDimensionCount)
                aPlotter->setTransformationSceneToScreen( pVCooSys->getTransformationSceneToScreen() );
            aPlotter->createShapes();
            m_bPointsWereSkipped = m_bPointsWereSkipped || aPlotter->PointsWereSkipped();
        }

        for( std::unique_ptr<VSeriesPlotter>& aPlotter : rSeriesPlotterList )
            aPlotter->rearrangeLabelToAvoidOverlapIfRequested(rPageSize);
    }

    if (rParam.mbUseFixedInnerSize)
    {
        aUsedOuterRect = awt::Rectangle( aConsumedOuterRect.getMinX(), aConsumedOuterRect.getMinY(), aConsumedOuterRect.getWidth(), aConsumedOuterRect.getHeight() );
    }
    else
        aUsedOuterRect = rParam.maRemainingSpace;

    bool bSnapRectToUsedArea = false;
    for( std::unique_ptr<VSeriesPlotter>& aPlotter : rSeriesPlotterList )
    {
        bSnapRectToUsedArea = aPlotter->shouldSnapRectToUsedArea();
        if(bSnapRectToUsedArea)
            break;
    }
    if(bSnapRectToUsedArea)
    {
        if (rParam.mbUseFixedInnerSize)
            m_aResultingDiagramRectangleExcludingAxes = getRectangleOfObject( "PlotAreaExcludingAxes" );
        else
        {
            ::basegfx::B2IRectangle aConsumedInnerRect = aVDiagram.getCurrentRectangle();
            m_aResultingDiagramRectangleExcludingAxes = awt::Rectangle( aConsumedInnerRect.getMinX(), aConsumedInnerRect.getMinY(), aConsumedInnerRect.getWidth(), aConsumedInnerRect.getHeight() );
        }
    }
    else
    {
        if (rParam.mbUseFixedInnerSize)
            m_aResultingDiagramRectangleExcludingAxes = rParam.maRemainingSpace;
        else
        {
            ::basegfx::B2IRectangle aConsumedInnerRect = aVDiagram.getCurrentRectangle();
            m_aResultingDiagramRectangleExcludingAxes = awt::Rectangle( aConsumedInnerRect.getMinX(), aConsumedInnerRect.getMinY(), aConsumedInnerRect.getWidth(), aConsumedInnerRect.getHeight() );
        }
    }

    if (rParam.mxMarkHandles.is())
    {
        awt::Point aPos(rParam.maRemainingSpace.X, rParam.maRemainingSpace.Y);
        awt::Size  aSize(rParam.maRemainingSpace.Width, rParam.maRemainingSpace.Height);

        bool bPosSizeExcludeAxesProperty = true;
        uno::Reference< beans::XPropertySet > xDiaProps( xDiagram, uno::UNO_QUERY_THROW );
        if( xDiaProps.is() )
            xDiaProps->getPropertyValue("PosSizeExcludeAxes") >>= bPosSizeExcludeAxesProperty;
        if (rParam.mbUseFixedInnerSize || bPosSizeExcludeAxesProperty)
        {
            aPos = awt::Point( m_aResultingDiagramRectangleExcludingAxes.X, m_aResultingDiagramRectangleExcludingAxes.Y );
            aSize = awt::Size( m_aResultingDiagramRectangleExcludingAxes.Width, m_aResultingDiagramRectangleExcludingAxes.Height );
        }
        rParam.mxMarkHandles->setPosition(aPos);
        rParam.mxMarkHandles->setSize(aSize);
    }

    return aUsedOuterRect;
}

bool ChartView::getExplicitValuesForAxis(
                     uno::Reference< XAxis > xAxis
                     , ExplicitScaleData&  rExplicitScale
                     , ExplicitIncrementData& rExplicitIncrement )
{
    impl_updateView();

    if(!xAxis.is())
        return false;

    uno::Reference< XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis(xAxis, mrChartModel.getFirstDiagram() ) );
    const VCoordinateSystem* pVCooSys = findInCooSysList(m_aVCooSysList,xCooSys);
    if(!pVCooSys)
        return false;

    sal_Int32 nDimensionIndex=-1;
    sal_Int32 nAxisIndex=-1;
    if( AxisHelper::getIndicesForAxis( xAxis, xCooSys, nDimensionIndex, nAxisIndex ) )
    {
        rExplicitScale = pVCooSys->getExplicitScale(nDimensionIndex,nAxisIndex);
        rExplicitIncrement = pVCooSys->getExplicitIncrement(nDimensionIndex,nAxisIndex);
        if( rExplicitScale.ShiftedCategoryPosition )
        {
            //remove 'one' from max
            if( rExplicitScale.AxisType == ::com::sun::star::chart2::AxisType::DATE )
            {
                Date aMaxDate(rExplicitScale.NullDate); aMaxDate += static_cast<long>(::rtl::math::approxFloor(rExplicitScale.Maximum));
                //for explicit scales with shifted categories we need one interval more
                switch( rExplicitScale.TimeResolution )
                {
                case ::com::sun::star::chart::TimeUnit::DAY:
                    --aMaxDate;
                    break;
                case ::com::sun::star::chart::TimeUnit::MONTH:
                    aMaxDate = DateHelper::GetDateSomeMonthsAway(aMaxDate,-1);
                    break;
                case ::com::sun::star::chart::TimeUnit::YEAR:
                    aMaxDate = DateHelper::GetDateSomeYearsAway(aMaxDate,-1);
                    break;
                }
                rExplicitScale.Maximum = aMaxDate - rExplicitScale.NullDate;
            }
            else if( rExplicitScale.AxisType == ::com::sun::star::chart2::AxisType::CATEGORY )
                rExplicitScale.Maximum -= 1.0;
            else if( rExplicitScale.AxisType == ::com::sun::star::chart2::AxisType::SERIES )
                rExplicitScale.Maximum -= 1.0;
        }
        return true;
    }
    return false;
}

SdrPage* ChartView::getSdrPage()
{
    SdrPage* pPage=nullptr;
    Reference< lang::XUnoTunnel> xUnoTunnel(m_xDrawPage,uno::UNO_QUERY);
    if(xUnoTunnel.is())
    {
        SvxDrawPage* pSvxDrawPage = reinterpret_cast<SvxDrawPage*>(xUnoTunnel->getSomething(
            SvxDrawPage::getUnoTunnelId() ));
        if(pSvxDrawPage)
        {
            pPage = pSvxDrawPage->GetSdrPage();
        }
    }
    return pPage;
}

uno::Reference< drawing::XShape > ChartView::getShapeForCID( const OUString& rObjectCID )
{
    SolarMutexGuard aSolarGuard;
    SdrObject* pObj = DrawModelWrapper::getNamedSdrObject( rObjectCID, this->getSdrPage() );
    if( pObj )
        return uno::Reference< drawing::XShape >( pObj->getUnoShape(), uno::UNO_QUERY);
    return nullptr;
}

awt::Rectangle ChartView::getDiagramRectangleExcludingAxes()
{
    impl_updateView();
    return m_aResultingDiagramRectangleExcludingAxes;
}

awt::Rectangle ChartView::getRectangleOfObject( const OUString& rObjectCID, bool bSnapRect )
{
    impl_updateView();

    awt::Rectangle aRet;
    uno::Reference< drawing::XShape > xShape( getShapeForCID(rObjectCID) );
    if(xShape.is())
    {
        //special handling for axis for old api:
        //same special handling for diagram
        ObjectType eObjectType( ObjectIdentifier::getObjectType( rObjectCID ) );
        if( eObjectType == OBJECTTYPE_AXIS || eObjectType == OBJECTTYPE_DIAGRAM )
        {
            SolarMutexGuard aSolarGuard;
            SvxShape* pRoot = SvxShape::getImplementation( xShape );
            if( pRoot )
            {
                SdrObject* pRootSdrObject = pRoot->GetSdrObject();
                if( pRootSdrObject )
                {
                    SdrObjList* pRootList = pRootSdrObject->GetSubList();
                    if( pRootList )
                    {
                        OUString aShapeName = "MarkHandles";
                        if( eObjectType == OBJECTTYPE_DIAGRAM )
                            aShapeName = "PlotAreaIncludingAxes";
                        SdrObject* pShape = DrawModelWrapper::getNamedSdrObject( aShapeName, pRootList );
                        if( pShape )
                            xShape.set( pShape->getUnoShape(), uno::UNO_QUERY);
                    }
                }
            }
        }

        awt::Size aSize( xShape->getSize() );
        awt::Point aPoint( xShape->getPosition() );
        aRet = awt::Rectangle( aPoint.X, aPoint.Y, aSize.Width, aSize.Height );
        if( bSnapRect )
        {
            //for rotated objects the shape size and position differs from the visible rectangle
            SvxShape* pShape = SvxShape::getImplementation( xShape );
            if( pShape )
            {
                SdrObject* pSdrObject = pShape->GetSdrObject();
                if( pSdrObject )
                {
                    Rectangle aSnapRect( pSdrObject->GetSnapRect() );
                    aRet = awt::Rectangle(aSnapRect.Left(),aSnapRect.Top(),aSnapRect.GetWidth(),aSnapRect.GetHeight());
                }
            }
        }
    }
    return aRet;
}

std::shared_ptr< DrawModelWrapper > ChartView::getDrawModelWrapper()
{
    return m_pDrawModelWrapper;
}

namespace
{
inline sal_Int32 lcl_getDiagramTitleSpace()
{
    return 200; //=0,2 cm spacing
}
bool lcl_getPropertySwapXAndYAxis( const uno::Reference< XDiagram >& xDiagram )
{
    bool bSwapXAndY = false;

    uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( xCooSysContainer.is() )
    {
        uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );
        if( aCooSysList.getLength() )
        {
            uno::Reference<beans::XPropertySet> xProp(aCooSysList[0], uno::UNO_QUERY );
            if( xProp.is()) try
            {
                xProp->getPropertyValue( "SwapXAndYAxis" ) >>= bSwapXAndY;
            }
            catch( const uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
    }
    return bSwapXAndY;
}

}

sal_Int32 ExplicitValueProvider::getExplicitNumberFormatKeyForAxis(
                  const Reference< chart2::XAxis >& xAxis
                , const Reference< chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem
                , const Reference<chart2::XChartDocument>& xChartDoc)
{
    return AxisHelper::getExplicitNumberFormatKeyForAxis( xAxis, xCorrespondingCoordinateSystem, xChartDoc
        , true /*bSearchForParallelAxisIfNothingIsFound*/ );
}

sal_Int32 ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel(
        const uno::Reference< beans::XPropertySet >& xSeriesOrPointProp,
        const uno::Reference< XDataSeries >& xSeries,
        sal_Int32 nPointIndex /*-1 for whole series*/,
        const uno::Reference< XDiagram >& xDiagram
        )
{
    sal_Int32 nFormat=0;
    if( !xSeriesOrPointProp.is() )
        return nFormat;

    bool bLinkToSource = true;
    try
    {
        xSeriesOrPointProp->getPropertyValue(CHART_UNONAME_LINK_TO_SRC_NUMFMT) >>= bLinkToSource;
    }
    catch ( const beans::UnknownPropertyException& ) {}

    xSeriesOrPointProp->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nFormat;
    sal_Int32 nOldFormat = nFormat;
    if (bLinkToSource)
    {
        uno::Reference< chart2::XChartType > xChartType( DataSeriesHelper::getChartTypeOfSeries( xSeries, xDiagram ) );

        bool bFormatFound = false;
        if( ChartTypeHelper::shouldLabelNumberFormatKeyBeDetectedFromYAxis( xChartType ) )
        {
            uno::Reference< beans::XPropertySet > xAttachedAxisProps( DiagramHelper::getAttachedAxis( xSeries, xDiagram ), uno::UNO_QUERY );
            if (xAttachedAxisProps.is() && (xAttachedAxisProps->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nFormat))
                bFormatFound = true;
        }
        if( !bFormatFound )
        {
            Reference< chart2::data::XDataSource > xSeriesSource( xSeries, uno::UNO_QUERY );
            OUString aRole( ChartTypeHelper::getRoleOfSequenceForDataLabelNumberFormatDetection( xChartType ) );

            Reference< data::XLabeledDataSequence > xLabeledSequence(
                DataSeriesHelper::getDataSequenceByRole( xSeriesSource, aRole ));
            if( xLabeledSequence.is() )
            {
                Reference< data::XDataSequence > xValues( xLabeledSequence->getValues() );
                if( xValues.is() )
                    nFormat = xValues->getNumberFormatKeyByIndex( nPointIndex );
            }
        }

        if (nFormat >= 0 && nOldFormat != nFormat)
            xSeriesOrPointProp->setPropertyValue(CHART_UNONAME_NUMFMT, uno::makeAny(nFormat));
    }

    if(nFormat<0)
        nFormat=0;
    return nFormat;
}

sal_Int32 ExplicitValueProvider::getExplicitPercentageNumberFormatKeyForDataLabel(
        const uno::Reference< beans::XPropertySet >& xSeriesOrPointProp,
        const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier )
{
    sal_Int32 nFormat=0;
    if( !xSeriesOrPointProp.is() )
        return nFormat;
    if( !(xSeriesOrPointProp->getPropertyValue("PercentageNumberFormat") >>= nFormat) )
    {
        nFormat = DiagramHelper::getPercentNumberFormat( xNumberFormatsSupplier );
    }
    if(nFormat<0)
        nFormat=0;
    return nFormat;
}

awt::Rectangle ExplicitValueProvider::addAxisTitleSizes(
            ChartModel& rModel
            , const Reference< uno::XInterface >& xChartView
            , const awt::Rectangle& rExcludingPositionAndSize )
{
    awt::Rectangle aRet(rExcludingPositionAndSize);

    //add axis title sizes to the diagram size
    uno::Reference< chart2::XTitle > xTitle_Height( TitleHelper::getTitle( TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION, rModel ) );
    uno::Reference< chart2::XTitle > xTitle_Width( TitleHelper::getTitle( TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION, rModel ) );
    uno::Reference< chart2::XTitle > xSecondTitle_Height( TitleHelper::getTitle( TitleHelper::SECONDARY_X_AXIS_TITLE, rModel ) );
    uno::Reference< chart2::XTitle > xSecondTitle_Width( TitleHelper::getTitle( TitleHelper::SECONDARY_Y_AXIS_TITLE, rModel ) );
    if( xTitle_Height.is() || xTitle_Width.is() || xSecondTitle_Height.is() || xSecondTitle_Width.is() )
    {
        ExplicitValueProvider* pExplicitValueProvider = ExplicitValueProvider::getExplicitValueProvider(xChartView);
        if( pExplicitValueProvider )
        {
            //detect whether x axis points into x direction or not
            if( lcl_getPropertySwapXAndYAxis( rModel.getFirstDiagram() ) )
            {
                std::swap( xTitle_Height, xTitle_Width );
                std::swap( xSecondTitle_Height, xSecondTitle_Width );
            }

            sal_Int32 nTitleSpaceWidth = 0;
            sal_Int32 nTitleSpaceHeight = 0;
            sal_Int32 nSecondTitleSpaceWidth = 0;
            sal_Int32 nSecondTitleSpaceHeight = 0;

            if( xTitle_Height.is() )
            {
                OUString aCID_X( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle_Height, rModel ) );
                nTitleSpaceHeight = pExplicitValueProvider->getRectangleOfObject( aCID_X, true ).Height;
                if( nTitleSpaceHeight )
                    nTitleSpaceHeight+=lcl_getDiagramTitleSpace();
            }
            if( xTitle_Width.is() )
            {
                OUString aCID_Y( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle_Width, rModel ) );
                nTitleSpaceWidth = pExplicitValueProvider->getRectangleOfObject( aCID_Y, true ).Width;
                if(nTitleSpaceWidth)
                    nTitleSpaceWidth+=lcl_getDiagramTitleSpace();
            }
            if( xSecondTitle_Height.is() )
            {
                OUString aCID_X( ObjectIdentifier::createClassifiedIdentifierForObject( xSecondTitle_Height, rModel ) );
                nSecondTitleSpaceHeight = pExplicitValueProvider->getRectangleOfObject( aCID_X, true ).Height;
                if( nSecondTitleSpaceHeight )
                    nSecondTitleSpaceHeight+=lcl_getDiagramTitleSpace();
            }
            if( xSecondTitle_Width.is() )
            {
                OUString aCID_Y( ObjectIdentifier::createClassifiedIdentifierForObject( xSecondTitle_Width, rModel ) );
                nSecondTitleSpaceWidth += pExplicitValueProvider->getRectangleOfObject( aCID_Y, true ).Width;
                if( nSecondTitleSpaceWidth )
                    nSecondTitleSpaceWidth+=lcl_getDiagramTitleSpace();
            }

            aRet.X -= nTitleSpaceWidth;
            aRet.Y -= nSecondTitleSpaceHeight;
            aRet.Width += nTitleSpaceWidth + nSecondTitleSpaceWidth;
            aRet.Height += nTitleSpaceHeight + nSecondTitleSpaceHeight;
        }
    }
    return aRet;
}

awt::Rectangle ExplicitValueProvider::substractAxisTitleSizes(
            ChartModel& rModel
            , const Reference< uno::XInterface >& xChartView
            , const awt::Rectangle& rPositionAndSizeIncludingTitles )
{
    awt::Rectangle aRet(rPositionAndSizeIncludingTitles);

    //add axis title sizes to the diagram size
    uno::Reference< chart2::XTitle > xTitle_Height( TitleHelper::getTitle( TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION, rModel ) );
    uno::Reference< chart2::XTitle > xTitle_Width( TitleHelper::getTitle( TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION, rModel ) );
    uno::Reference< chart2::XTitle > xSecondTitle_Height( TitleHelper::getTitle( TitleHelper::SECONDARY_X_AXIS_TITLE, rModel ) );
    uno::Reference< chart2::XTitle > xSecondTitle_Width( TitleHelper::getTitle( TitleHelper::SECONDARY_Y_AXIS_TITLE, rModel ) );
    if( xTitle_Height.is() || xTitle_Width.is() || xSecondTitle_Height.is() || xSecondTitle_Width.is() )
    {
        ExplicitValueProvider* pExplicitValueProvider = ExplicitValueProvider::getExplicitValueProvider(xChartView);
        if( pExplicitValueProvider )
        {
            //detect whether x axis points into x direction or not
            if( lcl_getPropertySwapXAndYAxis( rModel.getFirstDiagram() ) )
            {
                std::swap( xTitle_Height, xTitle_Width );
                std::swap( xSecondTitle_Height, xSecondTitle_Width );
            }

            sal_Int32 nTitleSpaceWidth = 0;
            sal_Int32 nTitleSpaceHeight = 0;
            sal_Int32 nSecondTitleSpaceWidth = 0;
            sal_Int32 nSecondTitleSpaceHeight = 0;

            if( xTitle_Height.is() )
            {
                OUString aCID_X( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle_Height, rModel ) );
                nTitleSpaceHeight = pExplicitValueProvider->getRectangleOfObject( aCID_X, true ).Height;
                if( nTitleSpaceHeight )
                    nTitleSpaceHeight+=lcl_getDiagramTitleSpace();
            }
            if( xTitle_Width.is() )
            {
                OUString aCID_Y( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle_Width, rModel ) );
                nTitleSpaceWidth = pExplicitValueProvider->getRectangleOfObject( aCID_Y, true ).Width;
                if(nTitleSpaceWidth)
                    nTitleSpaceWidth+=lcl_getDiagramTitleSpace();
            }
            if( xSecondTitle_Height.is() )
            {
                OUString aCID_X( ObjectIdentifier::createClassifiedIdentifierForObject( xSecondTitle_Height, rModel ) );
                nSecondTitleSpaceHeight = pExplicitValueProvider->getRectangleOfObject( aCID_X, true ).Height;
                if( nSecondTitleSpaceHeight )
                    nSecondTitleSpaceHeight+=lcl_getDiagramTitleSpace();
            }
            if( xSecondTitle_Width.is() )
            {
                OUString aCID_Y( ObjectIdentifier::createClassifiedIdentifierForObject( xSecondTitle_Width, rModel ) );
                nSecondTitleSpaceWidth += pExplicitValueProvider->getRectangleOfObject( aCID_Y, true ).Width;
                if( nSecondTitleSpaceWidth )
                    nSecondTitleSpaceWidth+=lcl_getDiagramTitleSpace();
            }

            aRet.X += nTitleSpaceWidth;
            aRet.Y += nSecondTitleSpaceHeight;
            aRet.Width -= (nTitleSpaceWidth + nSecondTitleSpaceWidth);
            aRet.Height -= (nTitleSpaceHeight + nSecondTitleSpaceHeight);
        }
    }
    return aRet;
}

namespace {

inline double lcl_getPageLayoutDistancePercentage()
{
    return 0.02;
}

bool getAvailablePosAndSizeForDiagram(
    CreateShapeParam2D& rParam, const awt::Size & rPageSize, const uno::Reference<XDiagram>& xDiagram )
{
    rParam.mbUseFixedInnerSize = false;

    //@todo: we need a size dependent on the axis labels
    sal_Int32 nYDistance = static_cast<sal_Int32>(rPageSize.Height*lcl_getPageLayoutDistancePercentage());
    sal_Int32 nXDistance = static_cast<sal_Int32>(rPageSize.Width*lcl_getPageLayoutDistancePercentage());
    rParam.maRemainingSpace.X += nXDistance;
    rParam.maRemainingSpace.Width -= 2*nXDistance;
    rParam.maRemainingSpace.Y += nYDistance;
    rParam.maRemainingSpace.Height -= 2*nYDistance;

    if (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0)
        return false;

    uno::Reference< beans::XPropertySet > xProp(xDiagram, uno::UNO_QUERY);

    bool bPosSizeExcludeAxes = false;
    if( xProp.is() )
        xProp->getPropertyValue( "PosSizeExcludeAxes" ) >>= bPosSizeExcludeAxes;

    //size:
    ::com::sun::star::chart2::RelativeSize aRelativeSize;
    if( xProp.is() && (xProp->getPropertyValue( "RelativeSize" )>>=aRelativeSize) )
    {
        rParam.maRemainingSpace.Height = static_cast<sal_Int32>(aRelativeSize.Secondary*rPageSize.Height);
        rParam.maRemainingSpace.Width = static_cast<sal_Int32>(aRelativeSize.Primary*rPageSize.Width);
        rParam.mbUseFixedInnerSize = bPosSizeExcludeAxes;
    }

    //position:
    chart2::RelativePosition aRelativePosition;
    if( xProp.is() && (xProp->getPropertyValue( "RelativePosition" )>>=aRelativePosition) )
    {
        //@todo decide whether x is primary or secondary

        //the coordinates re relative to the page
        double fX = aRelativePosition.Primary*rPageSize.Width;
        double fY = aRelativePosition.Secondary*rPageSize.Height;

        awt::Point aPos = RelativePositionHelper::getUpperLeftCornerOfAnchoredObject(
            awt::Point(static_cast<sal_Int32>(fX),static_cast<sal_Int32>(fY)),
            awt::Size(rParam.maRemainingSpace.Width, rParam.maRemainingSpace.Height),
            aRelativePosition.Anchor);

        rParam.maRemainingSpace.X = aPos.X;
        rParam.maRemainingSpace.Y = aPos.Y;

        rParam.mbUseFixedInnerSize = bPosSizeExcludeAxes;
    }

    //ensure that the diagram does not lap out right side or out of bottom
    if (rParam.maRemainingSpace.Y + rParam.maRemainingSpace.Height > rPageSize.Height)
        rParam.maRemainingSpace.Height = rPageSize.Height - rParam.maRemainingSpace.Y;

    if (rParam.maRemainingSpace.X + rParam.maRemainingSpace.Width > rPageSize.Width)
        rParam.maRemainingSpace.Width = rPageSize.Width - rParam.maRemainingSpace.X;

    return true;
}

enum TitleAlignment { ALIGN_LEFT, ALIGN_TOP, ALIGN_RIGHT, ALIGN_BOTTOM, ALIGN_Z };

void changePositionOfAxisTitle( VTitle* pVTitle, TitleAlignment eAlignment
                               , awt::Rectangle& rDiagramPlusAxesRect, const awt::Size & rPageSize )
{
    if(!pVTitle)
        return;

    awt::Point aNewPosition(0,0);
    awt::Size aTitleSize = pVTitle->getFinalSize();
    sal_Int32 nYDistance = static_cast<sal_Int32>(rPageSize.Height*lcl_getPageLayoutDistancePercentage());
    sal_Int32 nXDistance = static_cast<sal_Int32>(rPageSize.Width*lcl_getPageLayoutDistancePercentage());
    switch( eAlignment )
    {
    case ALIGN_TOP:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X + rDiagramPlusAxesRect.Width/2
                                    , rDiagramPlusAxesRect.Y - aTitleSize.Height/2  - nYDistance );
        break;
    case ALIGN_BOTTOM:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X + rDiagramPlusAxesRect.Width/2
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height + aTitleSize.Height/2  + nYDistance );
        break;
    case ALIGN_LEFT:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X - aTitleSize.Width/2 - nXDistance
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height/2 );
        break;
    case ALIGN_RIGHT:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X + rDiagramPlusAxesRect.Width + aTitleSize.Width/2 + nXDistance
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height/2 );
        break;
    case ALIGN_Z:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X + rDiagramPlusAxesRect.Width + aTitleSize.Width/2 + nXDistance
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height - aTitleSize.Height/2 );
       break;
    default:
        break;
    }

    sal_Int32 nMaxY = rPageSize.Height - aTitleSize.Height/2;
    sal_Int32 nMaxX = rPageSize.Width - aTitleSize.Width/2;
    sal_Int32 nMinX = aTitleSize.Width/2;
    sal_Int32 nMinY = aTitleSize.Height/2;
    if( aNewPosition.Y > nMaxY )
        aNewPosition.Y = nMaxY;
    if( aNewPosition.X > nMaxX )
        aNewPosition.X = nMaxX;
    if( aNewPosition.Y < nMinY )
        aNewPosition.Y = nMinY;
    if( aNewPosition.X < nMinX )
        aNewPosition.X = nMinX;

    pVTitle->changePosition( aNewPosition );
}

std::shared_ptr<VTitle> lcl_createTitle( TitleHelper::eTitleType eType
                , const uno::Reference< drawing::XShapes>& xPageShapes
                , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
                , ChartModel& rModel
                , awt::Rectangle& rRemainingSpace
                , const awt::Size & rPageSize
                , TitleAlignment eAlignment
                , bool& rbAutoPosition )
{
    std::shared_ptr<VTitle> apVTitle;

    // #i109336# Improve auto positioning in chart
    double fPercentage = lcl_getPageLayoutDistancePercentage();
    sal_Int32 nXDistance = static_cast< sal_Int32 >( rPageSize.Width * fPercentage );
    sal_Int32 nYDistance = static_cast< sal_Int32 >( rPageSize.Height * fPercentage );
    if ( eType == TitleHelper::MAIN_TITLE )
    {
        sal_Int32 nYOffset = 135;  // 1/100 mm
        nYDistance += nYOffset;
    }
    else if ( eType == TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION )
    {
        sal_Int32 nYOffset = 420;  // 1/100 mm
        nYDistance = nYOffset;
    }
    else if ( eType == TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION )
    {
        sal_Int32 nXOffset = 450;  // 1/100 mm
        nXDistance = nXOffset;
    }

    uno::Reference< XTitle > xTitle( TitleHelper::getTitle( eType, rModel ) );
    OUString aCompleteString = TitleHelper::getCompleteString(xTitle);
    if (aCompleteString.isEmpty())
        return apVTitle;

    //create title
    apVTitle.reset(new VTitle(xTitle));
    OUString aCID = ObjectIdentifier::createClassifiedIdentifierForObject(xTitle, rModel);
    apVTitle->init(xPageShapes, xShapeFactory, aCID);
    apVTitle->createShapes(awt::Point(0,0), rPageSize);
    awt::Size aTitleUnrotatedSize = apVTitle->getUnrotatedSize();
    awt::Size aTitleSize = apVTitle->getFinalSize();

    //position
    rbAutoPosition = true;
    awt::Point aNewPosition(0,0);
    chart2::RelativePosition aRelativePosition;
    uno::Reference<beans::XPropertySet> xProp(xTitle, uno::UNO_QUERY);
    if (xProp.is() && (xProp->getPropertyValue("RelativePosition") >>= aRelativePosition))
    {
        rbAutoPosition = false;

        //@todo decide whether x is primary or secondary
        double fX = aRelativePosition.Primary*rPageSize.Width;
        double fY = aRelativePosition.Secondary*rPageSize.Height;

        double fAnglePi = apVTitle->getRotationAnglePi();
        aNewPosition = RelativePositionHelper::getCenterOfAnchoredObject(
                awt::Point(static_cast<sal_Int32>(fX),static_cast<sal_Int32>(fY))
                , aTitleUnrotatedSize, aRelativePosition.Anchor, fAnglePi );
    }
    else //auto position
    {
        switch( eAlignment )
        {
        case ALIGN_TOP:
            aNewPosition = awt::Point( rRemainingSpace.X + rRemainingSpace.Width/2
                                     , rRemainingSpace.Y + aTitleSize.Height/2 + nYDistance );
            break;
        case ALIGN_BOTTOM:
            aNewPosition = awt::Point( rRemainingSpace.X + rRemainingSpace.Width/2
                                     , rRemainingSpace.Y + rRemainingSpace.Height - aTitleSize.Height/2 - nYDistance );
            break;
        case ALIGN_LEFT:
            aNewPosition = awt::Point( rRemainingSpace.X + aTitleSize.Width/2 + nXDistance
                                     , rRemainingSpace.Y + rRemainingSpace.Height/2 );
            break;
        case ALIGN_RIGHT:
            aNewPosition = awt::Point( rRemainingSpace.X + rRemainingSpace.Width - aTitleSize.Width/2 - nXDistance
                                     , rRemainingSpace.Y + rRemainingSpace.Height/2 );
            break;
        default:
            break;

        }
    }
    apVTitle->changePosition( aNewPosition );

    //remaining space
    switch( eAlignment )
    {
        case ALIGN_TOP:
            // Push the remaining space down from top.
            rRemainingSpace.Y += ( aTitleSize.Height + nYDistance );
            rRemainingSpace.Height -= ( aTitleSize.Height + nYDistance );
            break;
        case ALIGN_BOTTOM:
            // Push the remaining space up from bottom.
            rRemainingSpace.Height -= ( aTitleSize.Height + nYDistance );
            break;
        case ALIGN_LEFT:
            // Push the remaining space to the right from left edge.
            rRemainingSpace.X += ( aTitleSize.Width + nXDistance );
            rRemainingSpace.Width -= ( aTitleSize.Width + nXDistance );
            break;
        case ALIGN_RIGHT:
            // Push the remaining space to the left from right edge.
            rRemainingSpace.Width -= ( aTitleSize.Width + nXDistance );
            break;
        default:
            break;
    }

    return apVTitle;
}

bool lcl_createLegend( const uno::Reference< XLegend > & xLegend
                   , const uno::Reference< drawing::XShapes>& xPageShapes
                   , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
                   , const uno::Reference< uno::XComponentContext > & xContext
                   , awt::Rectangle & rRemainingSpace
                   , const awt::Size & rPageSize
                   , ChartModel& rModel
                   , const std::vector< LegendEntryProvider* >& rLegendEntryProviderList
                   , sal_Int16 nDefaultWritingMode )
{
    if (!VLegend::isVisible(xLegend))
        return false;

    VLegend aVLegend( xLegend, xContext, rLegendEntryProviderList,
            xPageShapes, xShapeFactory, rModel);
    aVLegend.setDefaultWritingMode( nDefaultWritingMode );
    aVLegend.createShapes( awt::Size( rRemainingSpace.Width, rRemainingSpace.Height ),
                           rPageSize );
    aVLegend.changePosition( rRemainingSpace, rPageSize );
    return true;
}

void formatPage(
      ChartModel& rChartModel
    , const awt::Size& rPageSize
    , const uno::Reference< drawing::XShapes >& xTarget
    , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
    )
{
    try
    {
        uno::Reference< beans::XPropertySet > xModelPage( rChartModel.getPageBackground());
        if( ! xModelPage.is())
            return;

        if( !xShapeFactory.is() )
            return;

        //format page
        tPropertyNameValueMap aNameValueMap;
        PropertyMapper::getValueMap( aNameValueMap, PropertyMapper::getPropertyNameMapForFillAndLineProperties(), xModelPage );

        OUString aCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, OUString() ) );
        aNameValueMap.insert( tPropertyNameValueMap::value_type( "Name", uno::makeAny( aCID ) ) ); //CID OUString

        tNameSequence aNames;
        tAnySequence aValues;
        PropertyMapper::getMultiPropertyListsFromValueMap( aNames, aValues, aNameValueMap );

        AbstractShapeFactory* pShapeFactory = AbstractShapeFactory::getOrCreateShapeFactory(xShapeFactory);
        pShapeFactory->createRectangle(
            xTarget, rPageSize, awt::Point(0, 0), aNames, aValues);
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void lcl_removeEmptyGroupShapes( const Reference< drawing::XShapes>& xParent )
{
    if(!xParent.is())
        return;
    Reference< drawing::XShapeGroup > xParentGroup( xParent, uno::UNO_QUERY );
    if( !xParentGroup.is() )
    {
        Reference< drawing::XDrawPage > xPage( xParent, uno::UNO_QUERY );
        if( !xPage.is() )
            return;
    }

    //iterate from back!
    for( sal_Int32 nN = xParent->getCount(); nN--; )
    {
        uno::Any aAny = xParent->getByIndex( nN );
        Reference< drawing::XShapes> xShapes(nullptr);
        if( aAny >>= xShapes )
            lcl_removeEmptyGroupShapes( xShapes );
        if( xShapes.is() && xShapes->getCount()==0 )
        {
            //remove empty group shape
            Reference< drawing::XShapeGroup > xGroup( xShapes, uno::UNO_QUERY );
            Reference< drawing::XShape > xShape( xShapes, uno::UNO_QUERY );
            if( xGroup.is() )
                xParent->remove( xShape );
        }
    }
}

}

void ChartView::impl_refreshAddIn()
{
    if( !m_bRefreshAddIn )
        return;

    uno::Reference< beans::XPropertySet > xProp( static_cast< ::cppu::OWeakObject* >( &mrChartModel ), uno::UNO_QUERY );
    if( xProp.is()) try
    {
        uno::Reference< util::XRefreshable > xAddIn;
        xProp->getPropertyValue( "AddIn" ) >>= xAddIn;
        if( xAddIn.is() )
        {
            bool bRefreshAddInAllowed = true;
            xProp->getPropertyValue( "RefreshAddInAllowed" ) >>= bRefreshAddInAllowed;
            if( bRefreshAddInAllowed )
                xAddIn->refresh();
        }
    }
    catch( const uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

/**
 * Is it a real 3D chart with a true 3D scene or a 3D chart in a 2D scene.
 */
bool ChartView::isReal3DChart()
{
    uno::Reference< XDiagram > xDiagram( mrChartModel.getFirstDiagram() );

    return ChartHelper::isGL3DDiagram(xDiagram);
}

static const char* envChartDummyFactory = getenv("CHART_DUMMY_FACTORY");

void ChartView::createShapes()
{
    osl::ResettableMutexGuard aTimedGuard(maTimeMutex);
    if(mrChartModel.isTimeBased())
    {
        maTimeBased.bTimeBased = true;
    }

    //make sure add-in is refreshed after creating the shapes
    const ::comphelper::ScopeGuard aGuard( [this]() { this->impl_refreshAddIn(); } );

    m_aResultingDiagramRectangleExcludingAxes = awt::Rectangle(0,0,0,0);
    impl_deleteCoordinateSystems();
    if( m_pDrawModelWrapper )
    {
        SolarMutexGuard aSolarGuard;
        // #i12587# support for shapes in chart
        m_pDrawModelWrapper->getSdrModel().EnableUndo( false );
        m_pDrawModelWrapper->clearMainDrawPage();
    }

    lcl_setDefaultWritingMode( m_pDrawModelWrapper, mrChartModel );

    awt::Size aPageSize = mrChartModel.getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );

    AbstractShapeFactory* pShapeFactory = AbstractShapeFactory::getOrCreateShapeFactory(m_xShapeFactory);
    if(!mxRootShape.is())
        mxRootShape = pShapeFactory->getOrCreateChartRootShape( m_xDrawPage );

    SdrPage* pPage = ChartView::getSdrPage();
    if(pPage) //it is necessary to use the implementation here as the uno page does not provide a propertyset
        pPage->SetSize(Size(aPageSize.Width,aPageSize.Height));
    else
    {
        OSL_FAIL("could not set page size correctly");
    }
    pShapeFactory->setPageSize(mxRootShape, aPageSize);
    pShapeFactory->clearPage(mxRootShape);

#if HAVE_FEATURE_DESKTOP
    if(isReal3DChart())
    {
        createShapes3D();
        return;
    }
    else
    {
        m_pGL3DPlotter.reset();

        // hide OpenGL window for now in normal charts
        OpenGLWindow* pWindow = mrChartModel.getOpenGLWindow();
        if(pWindow && !envChartDummyFactory)
            pWindow->Show(false);
    }
#endif

    createShapes2D(aPageSize);

    // #i12587# support for shapes in chart
    if ( m_pDrawModelWrapper )
    {
        SolarMutexGuard aSolarGuard;
        m_pDrawModelWrapper->getSdrModel().EnableUndo( true );
    }

    if(maTimeBased.bTimeBased)
    {
        maTimeBased.nFrame++;
    }
}

void ChartView::render()
{
    if(!isReal3DChart())
    {
        AbstractShapeFactory* pShapeFactory = AbstractShapeFactory::getOrCreateShapeFactory(m_xShapeFactory);
        OpenGLWindow* pWindow = mrChartModel.getOpenGLWindow();
        if(pWindow)
            pWindow->setRenderer(mp2DRenderer.get());
        bool bRender = pShapeFactory->preRender(mxRootShape, pWindow);
        if(bRender)
        {
            pShapeFactory->render(mxRootShape, pWindow != mp2DRenderer->getOpenGLWindow());
            pShapeFactory->postRender(pWindow);
        }
    }
}

// util::XEventListener (base of XCloseListener)
void SAL_CALL ChartView::disposing( const lang::EventObject& /* rSource */ )
        throw(uno::RuntimeException, std::exception)
{
}

void ChartView::impl_updateView( bool bCheckLockedCtrler )
{
    if( !m_pDrawModelWrapper )
        return;

    // #i12587# support for shapes in chart
    if ( m_bSdrViewIsInEditMode )
    {
        return;
    }

    if (bCheckLockedCtrler && mrChartModel.hasControllersLocked())
        return;

    if( m_bViewDirty && !m_bInViewUpdate )
    {
        m_bInViewUpdate = true;
        //bool bOldRefreshAddIn = m_bRefreshAddIn;
        //m_bRefreshAddIn = false;
        try
        {
            impl_notifyModeChangeListener("invalid");

            //prepare draw model
            {
                SolarMutexGuard aSolarGuard;
                m_pDrawModelWrapper->lockControllers();
            }

            //create chart view
            {
                OpenGLWindow* pWindow = mrChartModel.getOpenGLWindow();
                if (pWindow && ChartHelper::isGL3DDiagram(mrChartModel.getFirstDiagram()))
                    pWindow->Initialize();

                m_bViewDirty = false;
                m_bViewUpdatePending = false;
                createShapes();

                if( m_bViewDirty )
                {
                    //avoid recursions due to add-in
                    m_bRefreshAddIn = false;
                    m_bViewDirty = false;
                    m_bViewUpdatePending = false;
                    //delete old chart view
                    createShapes();
                    m_bRefreshAddIn = true;
                }
            }

            m_bViewDirty = m_bViewUpdatePending;
            m_bViewUpdatePending = false;
            m_bInViewUpdate = false;
        }
        catch( const uno::Exception& ex)
        {
            m_bViewDirty = m_bViewUpdatePending;
            m_bViewUpdatePending = false;
            m_bInViewUpdate = false;
            ASSERT_EXCEPTION( ex );
        }

        {
            SolarMutexGuard aSolarGuard;
            m_pDrawModelWrapper->unlockControllers();
        }

        impl_notifyModeChangeListener("valid");

        //m_bRefreshAddIn = bOldRefreshAddIn;
    }
}

// ____ XModifyListener ____
void SAL_CALL ChartView::modified( const lang::EventObject& /* aEvent */ )
    throw (uno::RuntimeException, std::exception)
{
    m_bViewDirty = true;
    if( m_bInViewUpdate )
        m_bViewUpdatePending = true;

    impl_notifyModeChangeListener("dirty");
}

//SfxListener
void ChartView::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    //#i77362 change notification for changes on additional shapes are missing
    if( m_bInViewUpdate )
        return;

    // #i12587# support for shapes in chart
    if ( m_bSdrViewIsInEditMode )
    {
        uno::Reference< view::XSelectionSupplier > xSelectionSupplier( mrChartModel.getCurrentController(), uno::UNO_QUERY );
        if ( xSelectionSupplier.is() )
        {
            OUString aSelObjCID;
            uno::Any aSelObj( xSelectionSupplier->getSelection() );
            aSelObj >>= aSelObjCID;
            if ( !aSelObjCID.isEmpty() )
            {
                return;
            }
        }
    }

    const SdrHint* pSdrHint = dynamic_cast< const SdrHint* >(&rHint);
    if( !pSdrHint )
        return;

    bool bShapeChanged = false;
    switch( pSdrHint->GetKind() )
    {
         case HINT_OBJCHG:
            bShapeChanged = true;
            break;
        case HINT_OBJINSERTED:
            bShapeChanged = true;
            break;
        case HINT_OBJREMOVED:
            bShapeChanged = true;
            break;
        case HINT_MODELCLEARED:
            bShapeChanged = true;
            break;
        case HINT_ENDEDIT:
            bShapeChanged = true;
            break;
        default:
            break;
    }

    if(bShapeChanged)
    {
        //#i76053# do not send view modified notifications for changes on the hidden page which contains e.g. the symbols for the dialogs
        if( ChartView::getSdrPage() != pSdrHint->GetPage() )
            bShapeChanged=false;
    }

    if(!bShapeChanged)
        return;

    mrChartModel.setModified(sal_True);
}

void ChartView::impl_notifyModeChangeListener( const OUString& rNewMode )
{
    try
    {
        ::cppu::OInterfaceContainerHelper* pIC = m_aListenerContainer
            .getContainer( cppu::UnoType<util::XModeChangeListener>::get());
        if( pIC )
        {
            util::ModeChangeEvent aEvent( static_cast< uno::XWeak* >( this ), rNewMode );
            ::cppu::OInterfaceIteratorHelper aIt( *pIC );
            while( aIt.hasMoreElements() )
            {
                uno::Reference< util::XModeChangeListener > xListener( aIt.next(), uno::UNO_QUERY );
                if( xListener.is() )
                    xListener->modeChanged( aEvent );
            }
        }
    }
    catch( const uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XModeChangeBroadcaster ____

void SAL_CALL ChartView::addModeChangeListener( const uno::Reference< util::XModeChangeListener >& xListener )
    throw (uno::RuntimeException, std::exception)
{
    m_aListenerContainer.addInterface(
        cppu::UnoType<util::XModeChangeListener>::get(), xListener );
}
void SAL_CALL ChartView::removeModeChangeListener( const uno::Reference< util::XModeChangeListener >& xListener )
    throw (uno::RuntimeException, std::exception)
{
    m_aListenerContainer.removeInterface(
        cppu::UnoType<util::XModeChangeListener>::get(), xListener );
}
void SAL_CALL ChartView::addModeChangeApproveListener( const uno::Reference< util::XModeChangeApproveListener >& /* _rxListener */ )
    throw (lang::NoSupportException, uno::RuntimeException, std::exception)
{

}
void SAL_CALL ChartView::removeModeChangeApproveListener( const uno::Reference< util::XModeChangeApproveListener >& /* _rxListener */ )
    throw (lang::NoSupportException, uno::RuntimeException, std::exception)
{

}

// ____ XUpdatable ____
void SAL_CALL ChartView::update() throw (uno::RuntimeException, std::exception)
{
    impl_updateView();

    //#i100778# migrate all imported or old documents to a plot area sizing exclusive axes (in case the save settings allow for this):
    //Although in general it is a bad idea to change the model from within the view this is exceptionally the best place to do this special conversion.
    //When a view update is requested (what happens for creating the metafile or displaying
    //the chart in edit mode or printing) it is most likely that all necessary information are available - like the underlying spreadsheet data for example.
    //Those data are important for the correct axis label sizes which are needed during conversion.
    if( DiagramHelper::switchDiagramPositioningToExcludingPositioning( mrChartModel, true, false ) )
        impl_updateView();
}

void SAL_CALL ChartView::updateSoft() throw (uno::RuntimeException, std::exception)
{
    update();
}

void SAL_CALL ChartView::updateHard() throw (uno::RuntimeException, std::exception)
{
    impl_updateView(false);
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL ChartView::getPropertySetInfo()
    throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
    return nullptr;
}

void SAL_CALL ChartView::setPropertyValue( const OUString& rPropertyName
                                                     , const Any& rValue )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException
          , lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( rPropertyName == "Resolution" )
    {
        awt::Size aNewResolution;
        if( ! (rValue >>= aNewResolution) )
            throw lang::IllegalArgumentException( "Property 'Resolution' requires value of type awt::Size", nullptr, 0 );

        if( m_aPageResolution.Width!=aNewResolution.Width || m_aPageResolution.Height!=aNewResolution.Height )
        {
            //set modified only when the new resolution is higher and points were skipped before
            bool bSetModified = m_bPointsWereSkipped && (m_aPageResolution.Width<aNewResolution.Width || m_aPageResolution.Height<aNewResolution.Height);

            m_aPageResolution = aNewResolution;

            if( bSetModified )
                this->modified( lang::EventObject(  static_cast< uno::XWeak* >( this )  ) );
        }
    }
    else if( rPropertyName == "ZoomFactors" )
    {
        //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
        uno::Sequence< beans::PropertyValue > aZoomFactors;
        if( ! (rValue >>= aZoomFactors) )
            throw lang::IllegalArgumentException( "Property 'ZoomFactors' requires value of type Sequence< PropertyValue >", nullptr, 0 );

        sal_Int32 nFilterArgs = aZoomFactors.getLength();
        beans::PropertyValue* pDataValues = aZoomFactors.getArray();
        while( nFilterArgs-- )
        {
            if ( pDataValues->Name == "ScaleXNumerator" )
                pDataValues->Value >>= m_nScaleXNumerator;
            else if ( pDataValues->Name == "ScaleXDenominator" )
                pDataValues->Value >>= m_nScaleXDenominator;
            else if ( pDataValues->Name == "ScaleYNumerator" )
                pDataValues->Value >>= m_nScaleYNumerator;
            else if ( pDataValues->Name == "ScaleYDenominator" )
                pDataValues->Value >>= m_nScaleYDenominator;

            pDataValues++;
        }
    }
    else if( rPropertyName == "SdrViewIsInEditMode" )
    {
        //#i77362 change notification for changes on additional shapes are missing
        if( ! (rValue >>= m_bSdrViewIsInEditMode) )
            throw lang::IllegalArgumentException( "Property 'SdrViewIsInEditMode' requires value of type sal_Bool", nullptr, 0 );
    }
    else
        throw beans::UnknownPropertyException( "unknown property was tried to set to chart wizard", nullptr );
}

Any SAL_CALL ChartView::getPropertyValue( const OUString& rPropertyName )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    Any aRet;
    if( rPropertyName == "Resolution" )
    {
        aRet = uno::makeAny( m_aPageResolution );
    }
    else
        throw beans::UnknownPropertyException( "unknown property was tried to get from chart wizard", nullptr );
    return aRet;
}

void SAL_CALL ChartView::addPropertyChangeListener(
    const OUString& /* aPropertyName */, const Reference< beans::XPropertyChangeListener >& /* xListener */ )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL ChartView::removePropertyChangeListener(
    const OUString& /* aPropertyName */, const Reference< beans::XPropertyChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ChartView::addVetoableChangeListener( const OUString& /* PropertyName */, const Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ChartView::removeVetoableChangeListener( const OUString& /* PropertyName */, const Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

// ____ XMultiServiceFactory ____

Reference< uno::XInterface > ChartView::createInstance( const OUString& aServiceSpecifier )
    throw (uno::Exception, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;

    SdrModel* pModel = ( m_pDrawModelWrapper ? &m_pDrawModelWrapper->getSdrModel() : nullptr );
    if ( pModel )
    {
        if ( aServiceSpecifier == "com.sun.star.drawing.DashTable" )
        {
            if ( !m_xDashTable.is() )
            {
                m_xDashTable = SvxUnoDashTable_createInstance( pModel );
            }
            return m_xDashTable;
        }
        else if ( aServiceSpecifier == "com.sun.star.drawing.GradientTable" )
        {
            if ( !m_xGradientTable.is() )
            {
                m_xGradientTable = SvxUnoGradientTable_createInstance( pModel );
            }
            return m_xGradientTable;
        }
        else if ( aServiceSpecifier == "com.sun.star.drawing.HatchTable" )
        {
            if ( !m_xHatchTable.is() )
            {
                m_xHatchTable = SvxUnoHatchTable_createInstance( pModel );
            }
            return m_xHatchTable;
        }
        else if ( aServiceSpecifier == "com.sun.star.drawing.BitmapTable" )
        {
            if ( !m_xBitmapTable.is() )
            {
                m_xBitmapTable = SvxUnoBitmapTable_createInstance( pModel );
            }
            return m_xBitmapTable;
        }
        else if ( aServiceSpecifier == "com.sun.star.drawing.TransparencyGradientTable" )
        {
            if ( !m_xTransGradientTable.is() )
            {
                m_xTransGradientTable = SvxUnoTransGradientTable_createInstance( pModel );
            }
            return m_xTransGradientTable;
        }
        else if ( aServiceSpecifier == "com.sun.star.drawing.MarkerTable" )
        {
            if ( !m_xMarkerTable.is() )
            {
                m_xMarkerTable = SvxUnoMarkerTable_createInstance( pModel );
            }
            return m_xMarkerTable;
        }
    }

    return nullptr;
}

Reference< uno::XInterface > ChartView::createInstanceWithArguments( const OUString& ServiceSpecifier, const uno::Sequence< uno::Any >& Arguments )
    throw (uno::Exception, uno::RuntimeException, std::exception)
{
    OSL_ENSURE( Arguments.getLength(), "ChartView::createInstanceWithArguments: arguments are ignored" );
    (void) Arguments; // avoid warning
    return createInstance( ServiceSpecifier );
}

uno::Sequence< OUString > ChartView::getAvailableServiceNames() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aServiceNames( 6 );

    aServiceNames[0] = "com.sun.star.drawing.DashTable";
    aServiceNames[1] = "com.sun.star.drawing.GradientTable";
    aServiceNames[2] = "com.sun.star.drawing.HatchTable";
    aServiceNames[3] = "com.sun.star.drawing.BitmapTable";
    aServiceNames[4] = "com.sun.star.drawing.TransparencyGradientTable";
    aServiceNames[5] = "com.sun.star.drawing.MarkerTable";

    return aServiceNames;
}

OUString ChartView::dump() throw (uno::RuntimeException, std::exception)
{
#if HAVE_FEATURE_DESKTOP
    // Used for unit tests and in chartcontroller only, no need to drag in this when cross-compiling
    // for non-desktop
    impl_updateView();
    uno::Reference< drawing::XShapes > xShapes( m_xDrawPage, uno::UNO_QUERY_THROW );
    sal_Int32 n = xShapes->getCount();
    OUStringBuffer aBuffer;
    for(sal_Int32 i = 0; i < n; ++i)
    {
        uno::Reference< drawing::XShapes > xShape(xShapes->getByIndex(i), uno::UNO_QUERY);
        if(xShape.is())
        {
            XShapeDumper dumper;
            OUString aString = XShapeDumper::dump(mxRootShape);
            aBuffer.append(aString);
        }
        else
        {
            uno::Reference< drawing::XShape > xSingleShape(xShapes->getByIndex(i), uno::UNO_QUERY);
            if(!xSingleShape.is())
                continue;
            XShapeDumper dumper;
            OUString aString = XShapeDumper::dump(xSingleShape);
            aBuffer.append(aString);
        }
        aBuffer.append("\n\n");
    }

    return aBuffer.makeStringAndClear();
#else
    return OUString();
#endif
}

void ChartView::setViewDirty()
{
    osl::ResettableMutexGuard aGuard(maTimeMutex);
    m_bViewDirty = true;
}

IMPL_LINK_NOARG_TYPED(ChartView, UpdateTimeBased, Timer *, void)
{
    setViewDirty();
    update();
}

void ChartView::createShapes2D( const awt::Size& rPageSize )
{
    AbstractShapeFactory* pShapeFactory = AbstractShapeFactory::getOrCreateShapeFactory(m_xShapeFactory);

    SolarMutexGuard aSolarGuard;

    // todo: it would be nicer to just pass the page m_xDrawPage and format it,
    // but the draw page does not support XPropertySet
    formatPage( mrChartModel, rPageSize, mxRootShape, m_xShapeFactory );

    CreateShapeParam2D aParam;
    aParam.maRemainingSpace.X = 0;
    aParam.maRemainingSpace.Y = 0;
    aParam.maRemainingSpace.Width = rPageSize.Width;
    aParam.maRemainingSpace.Height = rPageSize.Height;

    //create the group shape for diagram and axes first to have title and legends on top of it
    uno::Reference< XDiagram > xDiagram( mrChartModel.getFirstDiagram() );
    OUString aDiagramCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM, OUString::number( 0 ) ) );//todo: other index if more than one diagram is possible
    uno::Reference< drawing::XShapes > xDiagramPlusAxesPlusMarkHandlesGroup_Shapes(
            pShapeFactory->createGroup2D(mxRootShape,aDiagramCID) );

    aParam.mxMarkHandles = pShapeFactory->createInvisibleRectangle(
        xDiagramPlusAxesPlusMarkHandlesGroup_Shapes, awt::Size(0,0));
    AbstractShapeFactory::setShapeName(aParam.mxMarkHandles, "MarkHandles");

    aParam.mxPlotAreaWithAxes = pShapeFactory->createInvisibleRectangle(
        xDiagramPlusAxesPlusMarkHandlesGroup_Shapes, awt::Size(0, 0));
    AbstractShapeFactory::setShapeName(aParam.mxPlotAreaWithAxes, "PlotAreaIncludingAxes");

    aParam.mxDiagramWithAxesShapes = pShapeFactory->createGroup2D(xDiagramPlusAxesPlusMarkHandlesGroup_Shapes);

    bool bAutoPositionDummy = true;

    lcl_createTitle(
        TitleHelper::MAIN_TITLE, mxRootShape, m_xShapeFactory, mrChartModel,
        aParam.maRemainingSpace, rPageSize, ALIGN_TOP, bAutoPositionDummy);
    if (aParam.maRemainingSpace.Width <= 0 || aParam.maRemainingSpace.Height <= 0)
        return;

    lcl_createTitle(
        TitleHelper::SUB_TITLE, mxRootShape, m_xShapeFactory, mrChartModel,
        aParam.maRemainingSpace, rPageSize, ALIGN_TOP, bAutoPositionDummy );
    if (aParam.maRemainingSpace.Width <= 0|| aParam.maRemainingSpace.Height <= 0)
        return;

    aParam.mpSeriesPlotterContainer.reset(new SeriesPlotterContainer(m_aVCooSysList));
    aParam.mpSeriesPlotterContainer->initializeCooSysAndSeriesPlotter( mrChartModel );
    if(maTimeBased.bTimeBased && maTimeBased.nFrame != 0)
    {
        SeriesPlottersType& rSeriesPlotter = aParam.mpSeriesPlotterContainer->getSeriesPlotterList();
        size_t n = rSeriesPlotter.size();
        for(size_t i = 0; i < n; ++i)
        {
            std::vector<VDataSeries*> aAllNewDataSeries = rSeriesPlotter[i]->getAllSeries();
            std::vector< VDataSeries* >& rAllOldDataSeries =
                maTimeBased.m_aDataSeriesList[i];
            size_t m = std::min(aAllNewDataSeries.size(), rAllOldDataSeries.size());
            for(size_t j = 0; j < m; ++j)
            {
                aAllNewDataSeries[j]->setOldTimeBased(
                        rAllOldDataSeries[j], (maTimeBased.nFrame % 60)/60.0);
            }
        }
    }

    lcl_createLegend(
        LegendHelper::getLegend( mrChartModel ), mxRootShape, m_xShapeFactory, m_xCC,
        aParam.maRemainingSpace, rPageSize, mrChartModel, aParam.mpSeriesPlotterContainer->getLegendEntryProviderList(),
        lcl_getDefaultWritingModeFromPool( m_pDrawModelWrapper ) );
    if (aParam.maRemainingSpace.Width <= 0 || aParam.maRemainingSpace.Height <= 0)
        return;

    if (!createAxisTitleShapes2D(aParam, rPageSize))
        return;

    bool bDummy = false;
    bool bIsVertical = DiagramHelper::getVertical(xDiagram, bDummy, bDummy);

    if (getAvailablePosAndSizeForDiagram(aParam, rPageSize, mrChartModel.getFirstDiagram()))
    {
        awt::Rectangle aUsedOuterRect = impl_createDiagramAndContent(aParam, rPageSize);

        if (aParam.mxPlotAreaWithAxes.is())
        {
            aParam.mxPlotAreaWithAxes->setPosition(awt::Point(aUsedOuterRect.X, aUsedOuterRect.Y));
            aParam.mxPlotAreaWithAxes->setSize(awt::Size(aUsedOuterRect.Width, aUsedOuterRect.Height));
        }

        //correct axis title position
        awt::Rectangle aDiagramPlusAxesRect( aUsedOuterRect );
        if (aParam.mbAutoPosTitleX)
            changePositionOfAxisTitle(aParam.mpVTitleX.get(), ALIGN_BOTTOM, aDiagramPlusAxesRect, rPageSize);
        if (aParam.mbAutoPosTitleY)
            changePositionOfAxisTitle(aParam.mpVTitleY.get(), ALIGN_LEFT, aDiagramPlusAxesRect, rPageSize);
        if (aParam.mbAutoPosTitleZ)
            changePositionOfAxisTitle(aParam.mpVTitleZ.get(), ALIGN_Z, aDiagramPlusAxesRect, rPageSize);
        if (aParam.mbAutoPosSecondTitleX)
            changePositionOfAxisTitle(aParam.mpVTitleSecondX.get(), bIsVertical? ALIGN_RIGHT : ALIGN_TOP, aDiagramPlusAxesRect, rPageSize);
        if (aParam.mbAutoPosSecondTitleY)
            changePositionOfAxisTitle(aParam.mpVTitleSecondY.get(), bIsVertical? ALIGN_TOP : ALIGN_RIGHT, aDiagramPlusAxesRect, rPageSize);
    }

    //cleanup: remove all empty group shapes to avoid grey border lines:
    lcl_removeEmptyGroupShapes( mxRootShape );

    render();

    if(maTimeBased.bTimeBased && maTimeBased.nFrame % 60 == 0)
    {
        // create copy of the data for next frame
        SeriesPlottersType& rSeriesPlotter = aParam.mpSeriesPlotterContainer->getSeriesPlotterList();
        size_t n = rSeriesPlotter.size();
        maTimeBased.m_aDataSeriesList.clear();
        maTimeBased.m_aDataSeriesList.resize(n);
        for(size_t i = 0; i < n; ++i)
        {
            std::vector<VDataSeries*> aAllNewDataSeries = rSeriesPlotter[i]->getAllSeries();
            std::vector<VDataSeries*>& rAllOldDataSeries = maTimeBased.m_aDataSeriesList[i];
            size_t m = aAllNewDataSeries.size();
            for(size_t j = 0; j < m; ++j)
            {
                rAllOldDataSeries.push_back( aAllNewDataSeries[j]->
                        createCopyForTimeBased() );
            }
        }

        if(maTimeBased.eMode != MANUAL)
        {
            mrChartModel.setTimeBased(true);
            mrChartModel.getNextTimePoint();
        }
        else
            maTimeBased.maTimer.Stop();
    }

    if(maTimeBased.bTimeBased && maTimeBased.eMode != MANUAL && !maTimeBased.maTimer.IsActive())
    {
        maTimeBased.maTimer.SetTimeout(15);
        maTimeBased.maTimer.SetTimeoutHdl(LINK(this, ChartView, UpdateTimeBased));
        maTimeBased.maTimer.Start();
    }
}

bool ChartView::createAxisTitleShapes2D( CreateShapeParam2D& rParam, const css::awt::Size& rPageSize )
{
    uno::Reference<XDiagram> xDiagram = mrChartModel.getFirstDiagram();

    Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
    sal_Int32 nDimension = DiagramHelper::getDimension( xDiagram );

    if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 0 ) )
        rParam.mpVTitleX = lcl_createTitle( TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION, mxRootShape, m_xShapeFactory, mrChartModel
                , rParam.maRemainingSpace, rPageSize, ALIGN_BOTTOM, rParam.mbAutoPosTitleX );
    if (rParam.maRemainingSpace.Width <= 0 ||rParam.maRemainingSpace.Height <= 0)
        return false;

    if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 1 ) )
        rParam.mpVTitleY = lcl_createTitle( TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION, mxRootShape, m_xShapeFactory, mrChartModel
                , rParam.maRemainingSpace, rPageSize, ALIGN_LEFT, rParam.mbAutoPosTitleY );
    if (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0)
        return false;

    if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 2 ) )
        rParam.mpVTitleZ = lcl_createTitle( TitleHelper::Z_AXIS_TITLE, mxRootShape, m_xShapeFactory, mrChartModel
                , rParam.maRemainingSpace, rPageSize, ALIGN_RIGHT, rParam.mbAutoPosTitleZ );
    if (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0)
        return false;

    bool bDummy = false;
    bool bIsVertical = DiagramHelper::getVertical( xDiagram, bDummy, bDummy );

    if( ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimension, 0 ) )
        rParam.mpVTitleSecondX = lcl_createTitle( TitleHelper::SECONDARY_X_AXIS_TITLE, mxRootShape, m_xShapeFactory, mrChartModel
                , rParam.maRemainingSpace, rPageSize, bIsVertical? ALIGN_RIGHT : ALIGN_TOP, rParam.mbAutoPosSecondTitleX );
    if (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0)
        return false;

    if( ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimension, 1 ) )
        rParam.mpVTitleSecondY = lcl_createTitle( TitleHelper::SECONDARY_Y_AXIS_TITLE, mxRootShape, m_xShapeFactory, mrChartModel
                , rParam.maRemainingSpace, rPageSize, bIsVertical? ALIGN_TOP : ALIGN_RIGHT, rParam.mbAutoPosSecondTitleY );
    if (rParam.maRemainingSpace.Width <= 0 || rParam.maRemainingSpace.Height <= 0)
        return false;

    return true;
}

void ChartView::createShapes3D()
{
    OpenGLWindow* pWindow = mrChartModel.getOpenGLWindow();
    if(!pWindow)
        return;

    if( pWindow->GetSizePixel().Width() == 0 || pWindow->GetSizePixel().Height() == 0 )
    {
        awt::Size aPageSize = mrChartModel.getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );
        Size aSize = pWindow->LogicToPixel( Size(aPageSize.Width,aPageSize.Height), MapUnit(MAP_100TH_MM) );
        pWindow->SetSizePixel(aSize);
    }
    pWindow->Show();
    uno::Reference< XDiagram > xDiagram( mrChartModel.getFirstDiagram() );
    uno::Reference< XCoordinateSystemContainer > xCooSysContainer( xDiagram, uno::UNO_QUERY );
    if( !xCooSysContainer.is())
        return;

    uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysList( xCooSysContainer->getCoordinateSystems() );

    if (aCooSysList.getLength() != 1)
        // Supporting multiple coordinates in a truly 3D chart (which implies
        // it's a Cartesian coordinate system) is a bit of a challenge, if not
        // impossible.
        return;

    uno::Reference<XCoordinateSystem> xCooSys( aCooSysList[0] );

    //iterate through all chart types in the current coordinate system
    uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
    OSL_ASSERT( xChartTypeContainer.is());
    if( !xChartTypeContainer.is() )
        return;

    uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
    if (aChartTypeList.getLength() != 1)
        // Likewise, we can't really support multiple chart types here.
        return;

    uno::Reference< XChartType > xChartType( aChartTypeList[0] );

    if (!m_pGL3DPlotter)
    {
        m_pGL3DPlotter.reset(new GL3DBarChart(xChartType, pWindow));
    }
    else
    {
        GL3DBarChart* pChart = dynamic_cast<GL3DBarChart*>(m_pGL3DPlotter.get());
        if (pChart)
            pChart->setOpenGLWindow(pWindow);
    }

    uno::Reference< XDataSeriesContainer > xDataSeriesContainer( xChartType, uno::UNO_QUERY );
    OSL_ASSERT( xDataSeriesContainer.is());
    if( !xDataSeriesContainer.is() )
        return;

    std::vector<std::unique_ptr<VDataSeries> > aDataSeries;
    uno::Sequence< uno::Reference< XDataSeries > > aSeriesList( xDataSeriesContainer->getDataSeries() );
    for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
    {
        uno::Reference< XDataSeries > xDataSeries( aSeriesList[nS], uno::UNO_QUERY );
        if(!xDataSeries.is())
            continue;

        aDataSeries.push_back(o3tl::make_unique<VDataSeries>(xDataSeries));
    }

    std::unique_ptr<ExplicitCategoriesProvider> pCatProvider(new ExplicitCategoriesProvider(xCooSys, mrChartModel));

    m_pGL3DPlotter->create3DShapes(aDataSeries, *pCatProvider);

    m_pGL3DPlotter->render();
}

void ChartView::updateOpenGLWindow()
{
    if(!isReal3DChart())
        mp2DRenderer->updateOpenGLWindow();
}

} //namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_ChartView_get_implementation(css::uno::XComponentContext *context,
                                                         css::uno::Sequence<css::uno::Any> const &)
{
    ::chart::ChartModel *pChartModel = new ::chart::ChartModel(context);
    return cppu::acquire(new ::chart::ChartView(context, *pChartModel));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
