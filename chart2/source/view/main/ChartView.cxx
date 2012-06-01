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

#include "ChartView.hxx"
#include "chartview/DrawModelWrapper.hxx"
#include "NumberFormatterWrapper.hxx"
#include "ViewDefines.hxx"
#include "VDiagram.hxx"
#include "VTitle.hxx"
#include "ShapeFactory.hxx"
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

#include <rtl/uuid.h>
#include <comphelper/scopeguard.hxx>
#include <comphelper/servicehelper.hxx>
#include <boost/bind.hpp>
#include <unotools/streamwrap.hxx>
// header for class LocaleDataWrapper
#include <unotools/localedatawrapper.hxx>
// header for class SdrPage
#include <svx/svdpage.hxx>
// header for class SvxDrawPage
#include <svx/unopage.hxx>
// header for class SvxShape
#include <svx/unoshape.hxx>
// header for class Application
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <svx/unofill.hxx>

#include <drawinglayer/XShapeDumper.hxx>

#include <time.h>

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
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XShapeGroup.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/text/XText.hpp>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <svl/languageoptions.hxx>
#include <sot/clsids.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/oustringostreaminserter.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using rtl::OUString;

namespace
{
    class theExplicitValueProviderUnoTunnelId  : public rtl::Static< UnoTunnelIdInit, theExplicitValueProviderUnoTunnelId > {};
}

const uno::Sequence<sal_Int8>& ExplicitValueProvider::getUnoTunnelId()
{
    return theExplicitValueProviderUnoTunnelId::get().getSeq();
}

ExplicitValueProvider* ExplicitValueProvider::getExplicitValueProvider(
        const Reference< uno::XInterface >& xChartView )
{
    ExplicitValueProvider* pExplicitValueProvider=0;

    Reference< lang::XUnoTunnel > xTunnel( xChartView, uno::UNO_QUERY );
    if( xTunnel.is() )
    {
        pExplicitValueProvider = reinterpret_cast<ExplicitValueProvider*>(xTunnel->getSomething(
            ExplicitValueProvider::getUnoTunnelId() ));
    }
    return pExplicitValueProvider;
}

ChartView::ChartView(
        uno::Reference<uno::XComponentContext> const & xContext)
    : m_aMutex()
    , m_xCC(xContext)
    , m_xChartModel()
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
    , m_bSdrViewIsInEditMode(sal_False)
    , m_aResultingDiagramRectangleExcludingAxes(0,0,0,0)
{
}

void ChartView::impl_setChartModel( const uno::Reference< frame::XModel >& xChartModel )
{
    if( m_xChartModel != xChartModel )
    {
        m_xChartModel = xChartModel;
        m_bViewDirty = true;
    }
}

void SAL_CALL ChartView::initialize( const uno::Sequence< uno::Any >& aArguments )
                throw ( uno::Exception, uno::RuntimeException)
{
    OSL_PRECOND(aArguments.getLength() >= 1,"need 1 argument to initialize the view: xModel");
    if( !(aArguments.getLength() >= 1) )
        return;

    uno::Reference< frame::XModel > xNewChartModel;
    if( !(aArguments[0] >>= xNewChartModel) )
    {
        OSL_FAIL( "need a Reference to frame::XModel as first parameter for view initialization" );
    }
    impl_setChartModel( xNewChartModel );

    if( !m_pDrawModelWrapper.get() )
    {
        SolarMutexGuard aSolarGuard;
        m_pDrawModelWrapper = ::boost::shared_ptr< DrawModelWrapper >( new DrawModelWrapper( m_xCC ) );
        m_xShapeFactory = m_pDrawModelWrapper->getShapeFactory();
        m_xDrawPage = m_pDrawModelWrapper->getMainDrawPage();
        StartListening( m_pDrawModelWrapper->getSdrModel(), false /*bPreventDups*/ );
    }
}

ChartView::~ChartView()
{
    if( m_pDrawModelWrapper.get() )
    {
        EndListening( m_pDrawModelWrapper->getSdrModel(), false /*bAllDups*/ );
        SolarMutexGuard aSolarGuard;
        m_pDrawModelWrapper.reset();
    }
    m_xDrawPage = NULL;
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


//-----------------------------------------------------------------
// datatransfer::XTransferable
namespace
{
const rtl::OUString lcl_aGDIMetaFileMIMEType(
    RTL_CONSTASCII_USTRINGPARAM("application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\""));
const rtl::OUString lcl_aGDIMetaFileMIMETypeHighContrast(
    RTL_CONSTASCII_USTRINGPARAM("application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\""));
} // anonymous namespace

void ChartView::getMetaFile( const uno::Reference< io::XOutputStream >& xOutStream
                           , bool bUseHighContrast )
{
    if( !m_xDrawPage.is() )
        return;

    uno::Reference< lang::XMultiServiceFactory > xFactory( m_xCC->getServiceManager(), uno::UNO_QUERY );
    if( !xFactory.is() )
        return;

    // creating the graphic exporter
    uno::Reference< document::XExporter > xExporter( xFactory->createInstance(
            C2U("com.sun.star.drawing.GraphicExportFilter")), uno::UNO_QUERY);
    uno::Reference< document::XFilter > xFilter( xExporter, uno::UNO_QUERY );

    if( !xExporter.is() || !xFilter.is() )
        return;

    uno::Sequence< beans::PropertyValue > aProps(3);
    aProps[0].Name = C2U("FilterName");
    aProps[0].Value <<= C2U("SVM");

    aProps[1].Name = C2U("OutputStream");
    aProps[1].Value <<= xOutStream;

    uno::Sequence< beans::PropertyValue > aFilterData(4);
    aFilterData[0].Name = C2U("ExportOnlyBackground");
    aFilterData[0].Value <<= sal_False;
    aFilterData[1].Name = C2U("HighContrast");
    aFilterData[1].Value <<= bUseHighContrast;

    aFilterData[2].Name = C2U("Version");
    const sal_Int32 nVersion = SOFFICE_FILEFORMAT_50;
    aFilterData[2].Value <<= nVersion;

    aFilterData[3].Name = C2U("CurrentPage");
    aFilterData[3].Value <<= uno::Reference< uno::XInterface >( m_xDrawPage, uno::UNO_QUERY );

    //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
    {
        aFilterData.realloc( aFilterData.getLength()+4 );
        aFilterData[4].Name = C2U("ScaleXNumerator");
        aFilterData[4].Value = uno::makeAny( m_nScaleXNumerator );
        aFilterData[5].Name = C2U("ScaleXDenominator");
        aFilterData[5].Value = uno::makeAny( m_nScaleXDenominator );
        aFilterData[6].Name = C2U("ScaleYNumerator");
        aFilterData[6].Value = uno::makeAny( m_nScaleYNumerator );
        aFilterData[7].Name = C2U("ScaleYDenominator");
        aFilterData[7].Value = uno::makeAny( m_nScaleYDenominator );
    }

    aProps[2].Name = C2U("FilterData");
    aProps[2].Value <<= aFilterData;

    xExporter->setSourceDocument( uno::Reference< lang::XComponent >( m_xDrawPage, uno::UNO_QUERY) );
    if( xFilter->filter( aProps ) )
    {
        xOutStream->flush();
        xOutStream->closeOutput();
        uno::Reference< io::XSeekable > xSeekable( xOutStream, uno::UNO_QUERY );
        if( xSeekable.is() )
            xSeekable->seek(0);
    }
}

uno::Any SAL_CALL ChartView::getTransferData( const datatransfer::DataFlavor& aFlavor )
                throw (datatransfer::UnsupportedFlavorException, io::IOException, uno::RuntimeException)
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
                throw (uno::RuntimeException)
{
    uno::Sequence< datatransfer::DataFlavor > aRet(2);

    aRet[0] = datatransfer::DataFlavor( lcl_aGDIMetaFileMIMEType,
        C2U( "GDIMetaFile" ),
        ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );
    aRet[1] = datatransfer::DataFlavor( lcl_aGDIMetaFileMIMETypeHighContrast,
        C2U( "GDIMetaFile" ),
        ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );

    return aRet;
}
::sal_Bool SAL_CALL ChartView::isDataFlavorSupported( const datatransfer::DataFlavor& aFlavor )
                throw (uno::RuntimeException)
{
    return ( aFlavor.MimeType.equals(lcl_aGDIMetaFileMIMEType) ||
             aFlavor.MimeType.equals(lcl_aGDIMetaFileMIMETypeHighContrast) );
}

//-----------------------------------------------------------------
// ____ XUnoTunnel ___
::sal_Int64 SAL_CALL ChartView::getSomething( const uno::Sequence< ::sal_Int8 >& aIdentifier )
        throw( uno::RuntimeException)
{
    if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( ExplicitValueProvider::getUnoTunnelId().getConstArray(),
                                                         aIdentifier.getConstArray(), 16 ) )
    {
        ExplicitValueProvider* pProvider = this;
        return reinterpret_cast<sal_Int64>(pProvider);
    }
    return 0;
}

//-----------------------------------------------------------------
// lang::XServiceInfo

APPHELPER_XSERVICEINFO_IMPL(ChartView,CHART_VIEW_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > ChartView
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART_VIEW_SERVICE_NAME;
    return aSNS;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------

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

VCoordinateSystem* findInCooSysList( const std::vector< VCoordinateSystem* >& rVCooSysList
                                    , const uno::Reference< XCoordinateSystem >& xCooSys )
{
    for( size_t nC=0; nC < rVCooSysList.size(); nC++)
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[nC];
        if(pVCooSys->getModel()==xCooSys)
            return pVCooSys;
    }
    return NULL;
}

VCoordinateSystem* addCooSysToList( std::vector< VCoordinateSystem* >& rVCooSysList
            , const uno::Reference< XCoordinateSystem >& xCooSys
            , const uno::Reference< frame::XModel >& xChartModel )
{
    VCoordinateSystem* pVCooSys = findInCooSysList( rVCooSysList, xCooSys );
    if( !pVCooSys )
    {
        pVCooSys = VCoordinateSystem::createCoordinateSystem(xCooSys );
        if(pVCooSys)
        {
            rtl::OUString aCooSysParticle( ObjectIdentifier::createParticleForCoordinateSystem( xCooSys, xChartModel ) );
            pVCooSys->setParticle(aCooSysParticle);

            pVCooSys->setExplicitCategoriesProvider( new ExplicitCategoriesProvider(xCooSys,xChartModel) );

            rVCooSysList.push_back( pVCooSys );
        }
    }
    return pVCooSys;
}

VCoordinateSystem* lcl_getCooSysForPlotter( const std::vector< VCoordinateSystem* >& rVCooSysList, MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier )
{
    if(!pMinimumAndMaximumSupplier)
        return 0;
    for( size_t nC=0; nC < rVCooSysList.size(); nC++)
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[nC];
        if(pVCooSys->hasMinimumAndMaximumSupplier( pMinimumAndMaximumSupplier ))
            return pVCooSys;
    }
    return 0;
}

typedef std::pair< sal_Int32, sal_Int32 > tFullAxisIndex; //first index is the dimension, second index is the axis index that indicates whether this is a main or secondary axis
typedef std::map< VCoordinateSystem*, tFullAxisIndex > tCoordinateSystemMap;

struct AxisUsage
{
    AxisUsage();
    ~AxisUsage();

    void addCoordinateSystem( VCoordinateSystem* pCooSys, sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );
    ::std::vector< VCoordinateSystem* > getCoordinateSystems( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );
    sal_Int32 getMaxAxisIndexForDimension( sal_Int32 nDimensionIndex );

    ScaleAutomatism         aScaleAutomatism;

private:
    tCoordinateSystemMap    aCoordinateSystems;
    std::map< sal_Int32, sal_Int32 > aMaxIndexPerDimension;
};

AxisUsage::AxisUsage()
    : aScaleAutomatism(AxisHelper::createDefaultScale(),Date( Date::SYSTEM ))
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

//-----------------------------------------------------

class SeriesPlotterContainer
{
public:
    SeriesPlotterContainer( std::vector< VCoordinateSystem* >& rVCooSysList );
    ~SeriesPlotterContainer();

    void initializeCooSysAndSeriesPlotter( const uno::Reference< frame::XModel >& xChartModel );
    void initAxisUsageList(const Date& rNullDate);
    void doAutoScaling( const uno::Reference< frame::XModel >& xChartModel );
    void updateScalesAndIncrementsOnAxes();
    void setScalesFromCooSysToPlotter();
    void setNumberFormatsFromAxes();
    drawing::Direction3D getPreferredAspectRatio();

    std::vector< VSeriesPlotter* >& getSeriesPlotterList() { return m_aSeriesPlotterList; }
    std::vector< VCoordinateSystem* >& getCooSysList() { return m_rVCooSysList; }
    std::vector< LegendEntryProvider* > getLegendEntryProviderList();

    void AdaptScaleOfYAxisWithoutAttachedSeries( const uno::Reference< frame::XModel >& xChartModel );

private:
    std::vector< VSeriesPlotter* > m_aSeriesPlotterList;
    std::vector< VCoordinateSystem* >& m_rVCooSysList;
    ::std::map< uno::Reference< XAxis >, AxisUsage > m_aAxisUsageList;
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
    // - delete all plotter
    ::std::vector< VSeriesPlotter* >::const_iterator       aPlotterIter = m_aSeriesPlotterList.begin();
    const ::std::vector< VSeriesPlotter* >::const_iterator aPlotterEnd  = m_aSeriesPlotterList.end();
    for( aPlotterIter = m_aSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; ++aPlotterIter )
        delete *aPlotterIter;
    m_aSeriesPlotterList.clear();
}

std::vector< LegendEntryProvider* > SeriesPlotterContainer::getLegendEntryProviderList()
{
    std::vector< LegendEntryProvider* > aRet( m_aSeriesPlotterList.size() );
    ::std::vector< VSeriesPlotter* >::const_iterator       aPlotterIter = m_aSeriesPlotterList.begin();
    const ::std::vector< VSeriesPlotter* >::const_iterator aPlotterEnd  = m_aSeriesPlotterList.end();
    sal_Int32 nN = 0;
    for( aPlotterIter = m_aSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; ++aPlotterIter, nN++ )
        aRet[nN] = *aPlotterIter;
    return aRet;
}

void SeriesPlotterContainer::initializeCooSysAndSeriesPlotter(
              const uno::Reference< frame::XModel >& xChartModel )
{
    //------------ get model series from model
    sal_Int32 nDiagramIndex = 0;//todo if more than one diagram is supported
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    if( !xDiagram.is())
        return;

    uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( xChartModel, uno::UNO_QUERY );
    uno::Reference< chart2::XChartDocument > xChartDoc( xChartModel, uno::UNO_QUERY );
    if( xChartDoc.is() && xChartDoc->hasInternalDataProvider()
        && DiagramHelper::isSupportingDateAxis( xDiagram ) )
            m_nDefaultDateNumberFormat=DiagramHelper::getDateNumberFormat( xNumberFormatsSupplier );

    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
    if(!nDimensionCount)
    {
        //@todo handle mixed dimension
        nDimensionCount = 2;
    }

    sal_Bool bSortByXValues = sal_False;
    sal_Bool bConnectBars = sal_False;
    sal_Bool bGroupBarsPerAxis = sal_True;
    sal_Bool bIncludeHiddenCells = sal_True;
    sal_Int32 nStartingAngle = 90;
    try
    {
        uno::Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY_THROW );
        xDiaProp->getPropertyValue( C2U( "SortByXValues" ) ) >>= bSortByXValues;
        xDiaProp->getPropertyValue( C2U( "ConnectBars" ) ) >>= bConnectBars;
        xDiaProp->getPropertyValue( C2U( "GroupBarsPerAxis" ) ) >>= bGroupBarsPerAxis;
        xDiaProp->getPropertyValue( C2U( "IncludeHiddenCells" ) ) >>= bIncludeHiddenCells;
        xDiaProp->getPropertyValue( C2U( "StartingAngle" ) ) >>= nStartingAngle;
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    //---------------------------------------------------------------------
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
        VCoordinateSystem* pVCooSys = addCooSysToList(m_rVCooSysList,xCooSys,xChartModel);

        //iterate through all chart types in the current coordinate system
        uno::Reference< XChartTypeContainer > xChartTypeContainer( xCooSys, uno::UNO_QUERY );
        OSL_ASSERT( xChartTypeContainer.is());
        if( !xChartTypeContainer.is() )
            continue;
        uno::Sequence< uno::Reference< XChartType > > aChartTypeList( xChartTypeContainer->getChartTypes() );
        for( sal_Int32 nT = 0; nT < aChartTypeList.getLength(); ++nT )
        {
            uno::Reference< XChartType > xChartType( aChartTypeList[nT] );

            if(nT==0)
                m_bChartTypeUsesShiftedCategoryPositionPerDefault = ChartTypeHelper::shiftCategoryPosAtXAxisPerDefault( xChartType );

            bool bExcludingPositioning = DiagramPositioningMode_EXCLUDING == DiagramHelper::getDiagramPositioningMode( xDiagram );
            VSeriesPlotter* pPlotter = VSeriesPlotter::createSeriesPlotter( xChartType, nDimensionCount, bExcludingPositioning );
            if( !pPlotter )
                continue;
            m_aSeriesPlotterList.push_back( pPlotter );
            pPlotter->setNumberFormatsSupplier( xNumberFormatsSupplier );
            pPlotter->setColorScheme( xColorScheme );
            if(pVCooSys)
                pPlotter->setExplicitCategoriesProvider( pVCooSys->getExplicitCategoriesProvider() );
            sal_Int32 nMissingValueTreatment = DiagramHelper::getCorrectedMissingValueTreatment( xDiagram, xChartType );

            if(pVCooSys)
                pVCooSys->addMinimumAndMaximumSupplier(pPlotter);

            //------------ add series to plotter and thus prepare him(it) for providing minimum and maximum values
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

                rtl::OUString aSeriesParticle( ObjectIdentifier::createParticleForSeries( nDiagramIndex, nCS, nT, nS ) );
                pSeries->setParticle(aSeriesParticle);

                OUString aRole( ChartTypeHelper::getRoleOfSequenceForDataLabelNumberFormatDetection( xChartType ) );
                pSeries->setRoleOfSequenceForDataLabelNumberFormatDetection(aRole);

                //ignore secondary axis for charttypes that do not suppoert them
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
        uno::Sequence< rtl::OUString > aSeriesNames;
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
                    VSeriesPlotter* pSeriesPlotter = m_aSeriesPlotterList[0];
                    if( pSeriesPlotter )
                        aSeriesNames = pSeriesPlotter->getSeriesNames();
                    bSeriesNamesInitialized = true;
                }
                pVCooSys->setSeriesNamesForAxis( aSeriesNames );
            }
        }
    }
}

void SeriesPlotterContainer::initAxisUsageList(const Date& rNullDate)
{
    m_aAxisUsageList.clear();
    size_t nC;
    for( nC=0; nC < m_rVCooSysList.size(); nC++)
    {
        VCoordinateSystem* pVCooSys = m_rVCooSysList[nC];
        for(sal_Int32 nDimensionIndex=0; nDimensionIndex<3; nDimensionIndex++)
        {
            uno::Reference< XCoordinateSystem > xCooSys = pVCooSys->getModel();
            sal_Int32 nDimensionCount = xCooSys->getDimension();
            if( nDimensionIndex >= nDimensionCount )
                continue;
            bool bChartTypeAllowsDateAxis = ChartTypeHelper::isSupportingDateAxis(  AxisHelper::getChartTypeByIndex( xCooSys, 0 ), nDimensionCount, nDimensionIndex );
            const sal_Int32 nMaximumAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
            for(sal_Int32 nAxisIndex=0; nAxisIndex<=nMaximumAxisIndex; ++nAxisIndex)
            {
                uno::Reference< XAxis > xAxis( xCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex ) );
                OSL_ASSERT( xAxis.is());
                if( xAxis.is())
                {
                    if(m_aAxisUsageList.find(xAxis)==m_aAxisUsageList.end())
                    {
                        chart2::ScaleData aSourceScale = xAxis->getScaleData();
                        ExplicitCategoriesProvider* pExplicitCategoriesProvider = pVCooSys->getExplicitCategoriesProvider();
                        if( nDimensionIndex==0 )
                            AxisHelper::checkDateAxis( aSourceScale, pExplicitCategoriesProvider, bChartTypeAllowsDateAxis );
                        if( (aSourceScale.AxisType == AxisType::CATEGORY && m_bChartTypeUsesShiftedCategoryPositionPerDefault)
                            || (aSourceScale.AxisType==AxisType::CATEGORY && pExplicitCategoriesProvider && pExplicitCategoriesProvider->hasComplexCategories() )
                            || aSourceScale.AxisType == AxisType::DATE
                            || aSourceScale.AxisType == AxisType::SERIES )
                            aSourceScale.ShiftedCategoryPosition = true;
                        else
                            aSourceScale.ShiftedCategoryPosition = false;
                        m_aAxisUsageList[xAxis].aScaleAutomatism = ScaleAutomatism(aSourceScale,rNullDate);
                    }
                    AxisUsage& rAxisUsage = m_aAxisUsageList[xAxis];
                    rAxisUsage.addCoordinateSystem(pVCooSys,nDimensionIndex,nAxisIndex);
                }
            }
        }
    }

    ::std::map< uno::Reference< XAxis >, AxisUsage >::iterator             aAxisIter    = m_aAxisUsageList.begin();
    const ::std::map< uno::Reference< XAxis >, AxisUsage >::const_iterator aAxisEndIter = m_aAxisUsageList.end();

    //init m_nMaxAxisIndex
    m_nMaxAxisIndex = 0;
    for(sal_Int32 nDimensionIndex=0; nDimensionIndex<3; nDimensionIndex++)
    {
        for( aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; ++aAxisIter )
        {
            sal_Int32 nLocalMax = aAxisIter->second.getMaxAxisIndexForDimension( nDimensionIndex );
            if( m_nMaxAxisIndex < nLocalMax )
                m_nMaxAxisIndex = nLocalMax;
        }
    }
}

void SeriesPlotterContainer::setScalesFromCooSysToPlotter()
{
    //set scales to plotter to enable them to provide the preferred scene AspectRatio
    ::std::vector< VSeriesPlotter* >::const_iterator       aPlotterIter = m_aSeriesPlotterList.begin();
    const ::std::vector< VSeriesPlotter* >::const_iterator aPlotterEnd  = m_aSeriesPlotterList.end();
    for( aPlotterIter = m_aSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; ++aPlotterIter )
    {
        VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
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

    ::std::vector< VSeriesPlotter* >::const_iterator       aPlotterIter = m_aSeriesPlotterList.begin();
    const ::std::vector< VSeriesPlotter* >::const_iterator aPlotterEnd  = m_aSeriesPlotterList.end();
    for( aPlotterIter = m_aSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; ++aPlotterIter )
    {
        VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
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
                            if( xAxisProp->getPropertyValue( C2U( "NumberFormat" ) ) >>= nNumberFormatKey )
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

void SeriesPlotterContainer::doAutoScaling( const uno::Reference< frame::XModel >& xChartModel )
{
    //precondition: need a initialized m_aSeriesPlotterList
    //precondition: need a initialized m_aAxisUsageList

    ::std::map< uno::Reference< XAxis >, AxisUsage >::iterator             aAxisIter    = m_aAxisUsageList.begin();
    const ::std::map< uno::Reference< XAxis >, AxisUsage >::const_iterator aAxisEndIter = m_aAxisUsageList.end();

    //iterate over the main scales first than secondary axis
    size_t nC;
    sal_Int32 nAxisIndex=0;
    for( nAxisIndex=0; nAxisIndex<=m_nMaxAxisIndex; nAxisIndex++ )
    {

        // - first do autoscale for all x and z scales (because they are treated independent)
        for( aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; ++aAxisIter )
        {
            AxisUsage& rAxisUsage = (*aAxisIter).second;
            ::std::vector< VCoordinateSystem* > aVCooSysList_X = rAxisUsage.getCoordinateSystems(0,nAxisIndex);
            ::std::vector< VCoordinateSystem* > aVCooSysList_Z = rAxisUsage.getCoordinateSystems(2,nAxisIndex);

            for( nC=0; nC < aVCooSysList_X.size(); nC++)
                aVCooSysList_X[nC]->prepareScaleAutomatismForDimensionAndIndex(rAxisUsage.aScaleAutomatism,0,nAxisIndex);
            for( nC=0; nC < aVCooSysList_Z.size(); nC++)
                aVCooSysList_Z[nC]->prepareScaleAutomatismForDimensionAndIndex(rAxisUsage.aScaleAutomatism,2,nAxisIndex);

            ExplicitScaleData       aExplicitScale;
            ExplicitIncrementData   aExplicitIncrement;
            rAxisUsage.aScaleAutomatism.calculateExplicitScaleAndIncrement( aExplicitScale, aExplicitIncrement );

            for( nC=0; nC < aVCooSysList_X.size(); nC++)
                aVCooSysList_X[nC]->setExplicitScaleAndIncrement( 0, nAxisIndex, aExplicitScale, aExplicitIncrement );
            for( nC=0; nC < aVCooSysList_Z.size(); nC++)
                aVCooSysList_Z[nC]->setExplicitScaleAndIncrement( 2, nAxisIndex, aExplicitScale, aExplicitIncrement );
        }

        // - second do autoscale for the dependent y scales (the coordinate systems are prepared with x and z scales already )
        for( aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; ++aAxisIter )
        {
            AxisUsage& rAxisUsage = (*aAxisIter).second;
            ::std::vector< VCoordinateSystem* > aVCooSysList_X = rAxisUsage.getCoordinateSystems(0,nAxisIndex);
            ::std::vector< VCoordinateSystem* > aVCooSysList_Y = rAxisUsage.getCoordinateSystems(1,nAxisIndex);
            ::std::vector< VCoordinateSystem* > aVCooSysList_Z = rAxisUsage.getCoordinateSystems(2,nAxisIndex);

            if(!aVCooSysList_Y.size())
                continue;

            for( nC=0; nC < aVCooSysList_Y.size(); nC++)
                aVCooSysList_Y[nC]->prepareScaleAutomatismForDimensionAndIndex(rAxisUsage.aScaleAutomatism,1,nAxisIndex);

            ExplicitScaleData       aExplicitScale;
            ExplicitIncrementData   aExplicitIncrement;
            rAxisUsage.aScaleAutomatism.calculateExplicitScaleAndIncrement( aExplicitScale, aExplicitIncrement );

            for( nC=0; nC < aVCooSysList_X.size(); nC++)
                aVCooSysList_X[nC]->setExplicitScaleAndIncrement( 0, nAxisIndex, aExplicitScale, aExplicitIncrement );
            for( nC=0; nC < aVCooSysList_Y.size(); nC++)
                aVCooSysList_Y[nC]->setExplicitScaleAndIncrement( 1, nAxisIndex, aExplicitScale, aExplicitIncrement );
            for( nC=0; nC < aVCooSysList_Z.size(); nC++)
                aVCooSysList_Z[nC]->setExplicitScaleAndIncrement( 2, nAxisIndex, aExplicitScale, aExplicitIncrement );
        }
    }
    AdaptScaleOfYAxisWithoutAttachedSeries( xChartModel );
}

void SeriesPlotterContainer::AdaptScaleOfYAxisWithoutAttachedSeries( const uno::Reference< frame::XModel >& xChartModel )
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
            if( !aVCooSysList_Y.size() )
                continue;

            uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
            if( xDiagram.is() )
            {
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

                if( !bSeriesAttachedToThisAxis && nAttachedAxisIndex >= 0 )
                {
                    for( size_t nC = 0; nC < aVCooSysList_Y.size(); ++nC )
                    {
                        aVCooSysList_Y[nC]->prepareScaleAutomatismForDimensionAndIndex( rAxisUsage.aScaleAutomatism, 1, nAttachedAxisIndex );

                        ExplicitScaleData aExplicitScaleSource = aVCooSysList_Y[nC]->getExplicitScale( 1,nAttachedAxisIndex );
                        ExplicitIncrementData aExplicitIncrementSource = aVCooSysList_Y[nC]->getExplicitIncrement( 1,nAttachedAxisIndex );

                        ExplicitScaleData aExplicitScaleDest = aVCooSysList_Y[nC]->getExplicitScale( 1,nAxisIndex );;
                        ExplicitIncrementData aExplicitIncrementDest = aVCooSysList_Y[nC]->getExplicitIncrement( 1,nAxisIndex );;

                        aExplicitScaleDest.Orientation = aExplicitScaleSource.Orientation;
                        aExplicitScaleDest.Scaling = aExplicitScaleSource.Scaling;
                        aExplicitScaleDest.AxisType = aExplicitScaleSource.AxisType;

                        aExplicitIncrementDest.BaseValue = aExplicitIncrementSource.BaseValue;

                        ScaleData aScale( rAxisUsage.aScaleAutomatism.getScale() );
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
                    xCrossingMainAxis->getPropertyValue(C2U( "CrossoverPosition" )) >>= eCrossingMainAxisPos;
                    if( ::com::sun::star::chart::ChartAxisPosition_VALUE == eCrossingMainAxisPos )
                    {
                        double fValue = 0.0;
                        xCrossingMainAxis->getPropertyValue(C2U( "CrossoverValue" )) >>= fValue;
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
    ::std::vector< VSeriesPlotter* >::const_iterator       aPlotterIter = m_aSeriesPlotterList.begin();
    const ::std::vector< VSeriesPlotter* >::const_iterator aPlotterEnd  = m_aSeriesPlotterList.end();
    for( aPlotterIter = m_aSeriesPlotterList.begin(), nPlotterCount=0
        ; aPlotterIter != aPlotterEnd; ++aPlotterIter, ++nPlotterCount )
    {
        drawing::Direction3D aSingleRatio( (*aPlotterIter)->getPreferredDiagramAspectRatio() );
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
    }
    aPreferredAspectRatio = drawing::Direction3D(fx, fy, fz);
    return aPreferredAspectRatio;
}

//-----------------------------------------------------

namespace
{

bool lcl_IsPieOrDonut( const uno::Reference< XDiagram >& xDiagram )
{
    //special treatment for pie charts
    //the size is checked after complete creation to get the datalabels into the given space

    //todo: this is just a workaround at the moment for pie and donut labels
    return DiagramHelper::isPieOrDonutChart( xDiagram );
}

void lcl_setDefaultWritingMode( ::boost::shared_ptr< DrawModelWrapper > pDrawModelWrapper, const Reference< frame::XModel >& xChartModel )
{
    //get writing mode from parent document:
    if( SvtLanguageOptions().IsCTLFontEnabled() )
    {
        try
        {
            uno::Reference< container::XChild > xChild( xChartModel, uno::UNO_QUERY );
            sal_Int16 nWritingMode=-1;
            if ( xChild.is() )
            {
                uno::Reference< beans::XPropertySet > xParentProps( xChild->getParent(), uno::UNO_QUERY );
                uno::Reference< style::XStyleFamiliesSupplier > xStyleFamiliesSupplier( xParentProps, uno::UNO_QUERY );
                if( xStyleFamiliesSupplier.is() )
                {
                    uno::Reference< container::XNameAccess > xStylesFamilies( xStyleFamiliesSupplier->getStyleFamilies() );
                    if( xStylesFamilies.is() )
                    {
                        if( !xStylesFamilies->hasByName( C2U("PageStyles") ) )
                        {
                            //draw/impress is parent document
                            uno::Reference< lang::XMultiServiceFactory > xFatcory( xParentProps, uno::UNO_QUERY );
                            if( xFatcory.is() )
                            {
                                uno::Reference< beans::XPropertySet > xDrawDefaults( xFatcory->createInstance( C2U( "com.sun.star.drawing.Defaults" ) ), uno::UNO_QUERY );
                                if( xDrawDefaults.is() )
                                    xDrawDefaults->getPropertyValue( C2U("WritingMode") ) >>= nWritingMode;
                            }
                        }
                        else
                        {
                            uno::Reference< container::XNameAccess > xPageStyles( xStylesFamilies->getByName( C2U("PageStyles") ), uno::UNO_QUERY );
                            if( xPageStyles.is() )
                            {
                                rtl::OUString aPageStyle;

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
                                            uno::Sequence< rtl::OUString > aNames( xEmbeddedObjects->getElementNames() );

                                            sal_Int32 nCount = aNames.getLength();
                                            for( sal_Int32 nN=0; nN<nCount; nN++ )
                                            {
                                                uno::Reference< beans::XPropertySet > xEmbeddedProps( xEmbeddedObjects->getByName( aNames[nN] ), uno::UNO_QUERY );
                                                if( xEmbeddedProps.is() )
                                                {
                                                    static rtl::OUString aChartCLSID = rtl::OUString( SvGlobalName( SO3_SCH_CLASSID ).GetHexName());
                                                    rtl::OUString aCLSID;
                                                    xEmbeddedProps->getPropertyValue( C2U("CLSID") ) >>= aCLSID;
                                                    if( aCLSID.equals(aChartCLSID) )
                                                    {
                                                        uno::Reference< frame::XModel > xModel;
                                                        xEmbeddedProps->getPropertyValue( C2U("Model") ) >>= xModel;
                                                        if( xModel == xChartModel )
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
                                                                        xAnchorProps->getPropertyValue( C2U("WritingMode") ) >>= nWritingMode;
                                                                    }
                                                                    uno::Reference< text::XText > xText( xAnchor->getText() );
                                                                    if( xText.is() )
                                                                    {
                                                                        uno::Reference< beans::XPropertySet > xTextCursorProps( xText->createTextCursor(), uno::UNO_QUERY );
                                                                        if( xTextCursorProps.is() )
                                                                            xTextCursorProps->getPropertyValue( C2U("PageStyleName") ) >>= aPageStyle;
                                                                    }
                                                                }
                                                            }
                                                            break;
                                                        }
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
                                                xTextCursorProps->getPropertyValue( C2U("PageStyleName") ) >>= aPageStyle;
                                        }
                                    }
                                }
                                else
                                {
                                    //Calc is parent document
                                    xParentProps->getPropertyValue( C2U("PageStyle") ) >>= aPageStyle;
                                    if(aPageStyle.isEmpty())
                                        aPageStyle = C2U("Default");
                                }
                                if( nWritingMode == -1 || nWritingMode == text::WritingMode2::PAGE )
                                {
                                    uno::Reference< beans::XPropertySet > xPageStyle( xPageStyles->getByName( aPageStyle ), uno::UNO_QUERY );
                                    if( xPageStyle.is() )
                                        xPageStyle->getPropertyValue( C2U("WritingMode") ) >>= nWritingMode;
                                }
                            }
                        }
                    }
                }
            }
            if( nWritingMode != -1 && nWritingMode != text::WritingMode2::PAGE )
            {
                if( pDrawModelWrapper.get() )
                    pDrawModelWrapper->GetItemPool().SetPoolDefaultItem(SfxInt32Item(EE_PARA_WRITINGDIR, nWritingMode) );
            }
        }
        catch( const uno::Exception& ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

sal_Int16 lcl_getDefaultWritingModeFromPool( ::boost::shared_ptr< DrawModelWrapper > pDrawModelWrapper )
{
    sal_Int16 nWritingMode = text::WritingMode2::LR_TB;
    if( pDrawModelWrapper.get() )
    {
        const SfxPoolItem* pItem = &(pDrawModelWrapper->GetItemPool().GetDefaultItem( EE_PARA_WRITINGDIR ));
        if( pItem )
            nWritingMode = static_cast< sal_Int16 >((static_cast< const SfxInt32Item * >( pItem ))->GetValue());
    }
    return nWritingMode;
}

} //end anonymous namespace

//------------ create complete diagram shape (inclusive axis and series)

awt::Rectangle ChartView::impl_createDiagramAndContent( SeriesPlotterContainer& rSeriesPlotterContainer
            , const uno::Reference< drawing::XShapes>& xDiagramPlusAxes_Shapes
            , const awt::Point& rAvailablePos
            , const awt::Size& rAvailableSize
            , const awt::Size& rPageSize
            , bool bUseFixedInnerSize
            , const uno::Reference< drawing::XShape>& xDiagram_MarkHandles /*needs to be resized to fit the result*/
            )
{
    //return the used rectangle
    awt::Rectangle aUsedOuterRect( rAvailablePos.X, rAvailablePos.Y, 0, 0 );

//     sal_Int32 nDiagramIndex = 0;//todo if more than one diagam is supported
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( m_xChartModel ) );
    if( !xDiagram.is())
        return aUsedOuterRect;

    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
    if(!nDimensionCount)
    {
        //@todo handle mixed dimension
        nDimensionCount = 2;
    }

    ::basegfx::B2IRectangle aAvailableOuterRect( BaseGFXHelper::makeRectangle(rAvailablePos,rAvailableSize) );

    const std::vector< VCoordinateSystem* >& rVCooSysList( rSeriesPlotterContainer.getCooSysList() );
    const std::vector< VSeriesPlotter* >& rSeriesPlotterList( rSeriesPlotterContainer.getSeriesPlotterList() );

    //create VAxis, so they can give necessary information for automatic scaling
    uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( m_xChartModel, uno::UNO_QUERY );
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
        pVCooSys->createVAxisList( xNumberFormatsSupplier
                                        , rPageSize //font reference size
                                        , BaseGFXHelper::B2IRectangleToAWTRectangle( aAvailableOuterRect ) //maximum space for labels
                                        );
    }


    // - prepare list of all axis and how they are used
    Date aNullDate = NumberFormatterWrapper( uno::Reference< util::XNumberFormatsSupplier >( m_xChartModel, uno::UNO_QUERY ) ).getNullDate();
    rSeriesPlotterContainer.initAxisUsageList(aNullDate);
    rSeriesPlotterContainer.doAutoScaling( m_xChartModel );
    rSeriesPlotterContainer.setScalesFromCooSysToPlotter();
    rSeriesPlotterContainer.setNumberFormatsFromAxes();

    //---------------------------------------------------------------------
    //create shapes

    //------------ create diagram shapes
    //aspect ratio
    drawing::Direction3D aPreferredAspectRatio(
        rSeriesPlotterContainer.getPreferredAspectRatio() );

    uno::Reference< drawing::XShapes > xSeriesTargetInFrontOfAxis(0);
    uno::Reference< drawing::XShapes > xSeriesTargetBehindAxis(0);
    VDiagram aVDiagram(xDiagram, aPreferredAspectRatio, nDimensionCount);
    {//create diagram
        aVDiagram.init(xDiagramPlusAxes_Shapes,xDiagramPlusAxes_Shapes,m_xShapeFactory);
        aVDiagram.createShapes(rAvailablePos,rAvailableSize);
        xSeriesTargetInFrontOfAxis = aVDiagram.getCoordinateRegion();
        if( !bUseFixedInnerSize )
            aVDiagram.reduceToMimimumSize();
    }

    uno::Reference< drawing::XShapes > xTextTargetShapes( ShapeFactory(m_xShapeFactory).createGroup2D(xDiagramPlusAxes_Shapes) );

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

    uno::Reference< drawing::XShape > xBoundingShape( xDiagramPlusAxes_Shapes, uno::UNO_QUERY );
    ::basegfx::B2IRectangle aConsumedOuterRect;

    //use first coosys only so far; todo: calculate for more than one coosys if we have more in future
    //todo: this is just a workaround at the moment for pie and donut labels
    bool bIsPieOrDonut = lcl_IsPieOrDonut(xDiagram);
    if( !bIsPieOrDonut && (!rVCooSysList.empty()) )
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[0];
        pVCooSys->createMaximumAxesLabels();

        aConsumedOuterRect = ::basegfx::B2IRectangle( ShapeFactory::getRectangleOfShape(xBoundingShape) );
        ::basegfx::B2IRectangle aNewInnerRect( aVDiagram.getCurrentRectangle() );
        if( !bUseFixedInnerSize )
            aNewInnerRect = aVDiagram.adjustInnerSize( aConsumedOuterRect );

        pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
            createTransformationSceneToScreen( aNewInnerRect ) ));

        //redo autoscaling to get size and text dependent automatic main increment count
        rSeriesPlotterContainer.doAutoScaling( m_xChartModel );
        rSeriesPlotterContainer.updateScalesAndIncrementsOnAxes();
        rSeriesPlotterContainer.setScalesFromCooSysToPlotter();

        pVCooSys->createAxesLabels();

        bool bLessSpaceConsumedThanExpected = false;
        {
            aConsumedOuterRect = ShapeFactory::getRectangleOfShape(xBoundingShape);
            if( aConsumedOuterRect.getMinX() > aAvailableOuterRect.getMinX()
                || aConsumedOuterRect.getMaxX() < aAvailableOuterRect.getMaxX()
                || aConsumedOuterRect.getMinY() > aAvailableOuterRect.getMinY()
                || aConsumedOuterRect.getMinY() < aAvailableOuterRect.getMaxY() )
                bLessSpaceConsumedThanExpected = true;
        }

        if( bLessSpaceConsumedThanExpected && !bUseFixedInnerSize )
        {
            aVDiagram.adjustInnerSize( aConsumedOuterRect );
            pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
                createTransformationSceneToScreen( aVDiagram.getCurrentRectangle() ) ));
        }
        pVCooSys->updatePositions();//todo: logically this belongs to the condition above, but it seems also to be neccessary to give the axes group shapes the right bounding rects for hit test -  probably caused by bug i106183 -> check again if fixed
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
    ::std::vector< VSeriesPlotter* >::const_iterator       aPlotterIter = rSeriesPlotterList.begin();
    const ::std::vector< VSeriesPlotter* >::const_iterator aPlotterEnd  = rSeriesPlotterList.end();
    for( aPlotterIter = rSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; ++aPlotterIter )
    {
        //------------ set transformation to plotter / create series
        VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
        rtl::OUString aCID; //III
        uno::Reference< drawing::XShapes > xSeriesTarget(0);
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
        //
        pSeriesPlotter->createShapes();
        m_bPointsWereSkipped = m_bPointsWereSkipped || pSeriesPlotter->PointsWereSkipped();
    }

    //recreate all with corrected sizes if requested
    if( bIsPieOrDonut )
    {
        m_bPointsWereSkipped = false;

        aConsumedOuterRect = ::basegfx::B2IRectangle( ShapeFactory::getRectangleOfShape(xBoundingShape) );
        ::basegfx::B2IRectangle aNewInnerRect( aVDiagram.getCurrentRectangle() );
        if( !bUseFixedInnerSize )
            aNewInnerRect = aVDiagram.adjustInnerSize( aConsumedOuterRect );

        for( aPlotterIter = rSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; ++aPlotterIter )
        {
            VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
            pSeriesPlotter->releaseShapes();
        }

        //clear and recreate
        ShapeFactory::removeSubShapes( xSeriesTargetInFrontOfAxis ); //xSeriesTargetBehindAxis is a sub shape of xSeriesTargetInFrontOfAxis and will be removed here
        xSeriesTargetBehindAxis.clear();
        ShapeFactory::removeSubShapes( xTextTargetShapes );

        //set new transformation
        for( nC=0; nC < rVCooSysList.size(); nC++)
        {
            VCoordinateSystem* pVCooSys = rVCooSysList[nC];
            pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
                createTransformationSceneToScreen( aNewInnerRect ) ));
        }

        // - create data series for all charttypes
        for( aPlotterIter = rSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; ++aPlotterIter )
        {
            //------------ set transformation to plotter / create series
            VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
            VCoordinateSystem* pVCooSys = lcl_getCooSysForPlotter( rVCooSysList, pSeriesPlotter );
            if(2==nDimensionCount)
                pSeriesPlotter->setTransformationSceneToScreen( pVCooSys->getTransformationSceneToScreen() );
            pSeriesPlotter->createShapes();
            m_bPointsWereSkipped = m_bPointsWereSkipped || pSeriesPlotter->PointsWereSkipped();
        }

        for( aPlotterIter = rSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; ++aPlotterIter )
        {
            VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
            pSeriesPlotter->rearrangeLabelToAvoidOverlapIfRequested( rPageSize );
        }
    }

    if( bUseFixedInnerSize )
    {
        aUsedOuterRect = awt::Rectangle( aConsumedOuterRect.getMinX(), aConsumedOuterRect.getMinY(), aConsumedOuterRect.getWidth(), aConsumedOuterRect.getHeight() );
    }
    else
        aUsedOuterRect = awt::Rectangle( rAvailablePos.X, rAvailablePos.Y, rAvailableSize.Width, rAvailableSize.Height );

    bool bSnapRectToUsedArea = false;
    for( aPlotterIter = rSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; ++aPlotterIter )
    {
        VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
        bSnapRectToUsedArea = pSeriesPlotter->shouldSnapRectToUsedArea();
        if(bSnapRectToUsedArea)
            break;
    }
    if(bSnapRectToUsedArea)
    {
        if( bUseFixedInnerSize )
            m_aResultingDiagramRectangleExcludingAxes = getRectangleOfObject( C2U("PlotAreaExcludingAxes") );
        else
        {
            ::basegfx::B2IRectangle aConsumedInnerRect = aVDiagram.getCurrentRectangle();
            m_aResultingDiagramRectangleExcludingAxes = awt::Rectangle( aConsumedInnerRect.getMinX(), aConsumedInnerRect.getMinY(), aConsumedInnerRect.getWidth(), aConsumedInnerRect.getHeight() );
        }
    }
    else
    {
        if( bUseFixedInnerSize )
            m_aResultingDiagramRectangleExcludingAxes = awt::Rectangle( rAvailablePos.X, rAvailablePos.Y, rAvailableSize.Width, rAvailableSize.Height );
        else
        {
            ::basegfx::B2IRectangle aConsumedInnerRect = aVDiagram.getCurrentRectangle();
            m_aResultingDiagramRectangleExcludingAxes = awt::Rectangle( aConsumedInnerRect.getMinX(), aConsumedInnerRect.getMinY(), aConsumedInnerRect.getWidth(), aConsumedInnerRect.getHeight() );
        }
    }

    if( xDiagram_MarkHandles.is() )
    {
        awt::Point aPos(rAvailablePos);
        awt::Size  aSize(rAvailableSize);
        bool bPosSizeExcludeAxesProperty = true;
        uno::Reference< beans::XPropertySet > xDiaProps( xDiagram, uno::UNO_QUERY_THROW );
        if( xDiaProps.is() )
            xDiaProps->getPropertyValue(C2U("PosSizeExcludeAxes")) >>= bPosSizeExcludeAxesProperty;
        if( bUseFixedInnerSize || bPosSizeExcludeAxesProperty )
        {
            aPos = awt::Point( m_aResultingDiagramRectangleExcludingAxes.X, m_aResultingDiagramRectangleExcludingAxes.Y );
            aSize = awt::Size( m_aResultingDiagramRectangleExcludingAxes.Width, m_aResultingDiagramRectangleExcludingAxes.Height );
        }
        xDiagram_MarkHandles->setPosition( aPos );
        xDiagram_MarkHandles->setSize( aSize );
    }

    return aUsedOuterRect;
}

//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------

sal_Bool ChartView::getExplicitValuesForAxis(
                     uno::Reference< XAxis > xAxis
                     , ExplicitScaleData&  rExplicitScale
                     , ExplicitIncrementData& rExplicitIncrement )
{
    impl_updateView();

    if(!xAxis.is())
        return sal_False;

    uno::Reference< XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemOfAxis(xAxis,ChartModelHelper::findDiagram( m_xChartModel )  ) );
    const VCoordinateSystem* pVCooSys = findInCooSysList(m_aVCooSysList,xCooSys);
    if(!pVCooSys)
        return sal_False;

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
                    aMaxDate--;break;
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
        return sal_True;
    }
    return sal_False;
}

SdrPage* ChartView::getSdrPage()
{
    SdrPage* pPage=0;
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

uno::Reference< drawing::XShape > ChartView::getShapeForCID( const rtl::OUString& rObjectCID )
{
    SolarMutexGuard aSolarGuard;
    SdrObject* pObj = DrawModelWrapper::getNamedSdrObject( rObjectCID, this->getSdrPage() );
    if( pObj )
        return uno::Reference< drawing::XShape >( pObj->getUnoShape(), uno::UNO_QUERY);
    return 0;
}

awt::Rectangle ChartView::getDiagramRectangleExcludingAxes()
{
    impl_updateView();
    return m_aResultingDiagramRectangleExcludingAxes;
}

awt::Rectangle ChartView::getRectangleOfObject( const rtl::OUString& rObjectCID, bool bSnapRect )
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
                        OUString aShapeName = C2U("MarkHandles");
                        if( eObjectType == OBJECTTYPE_DIAGRAM )
                            aShapeName = C2U("PlotAreaIncludingAxes");
                        SdrObject* pShape = DrawModelWrapper::getNamedSdrObject( aShapeName, pRootList );
                        if( pShape )
                            xShape = uno::Reference< drawing::XShape >( pShape->getUnoShape(), uno::UNO_QUERY);
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

::boost::shared_ptr< DrawModelWrapper > ChartView::getDrawModelWrapper()
{
    return m_pDrawModelWrapper;
}

namespace
{
sal_Int32 lcl_getDiagramTitleSpace()
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
                xProp->getPropertyValue( C2U( "SwapXAndYAxis" ) ) >>= bSwapXAndY;
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
                , const Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier )
{
    return AxisHelper::getExplicitNumberFormatKeyForAxis( xAxis, xCorrespondingCoordinateSystem, xNumberFormatsSupplier
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

    rtl::OUString aPropName( C2U( "NumberFormat" ) );
    if( !(xSeriesOrPointProp->getPropertyValue(aPropName) >>= nFormat) )
    {
        uno::Reference< chart2::XChartType > xChartType( DataSeriesHelper::getChartTypeOfSeries( xSeries, xDiagram ) );

        bool bFormatFound = false;
        if( ChartTypeHelper::shouldLabelNumberFormatKeyBeDetectedFromYAxis( xChartType ) )
        {
            uno::Reference< beans::XPropertySet > xAttachedAxisProps( DiagramHelper::getAttachedAxis( xSeries, xDiagram ), uno::UNO_QUERY );
            if( xAttachedAxisProps.is() && ( xAttachedAxisProps->getPropertyValue( aPropName ) >>= nFormat ) )
                bFormatFound = true;
        }
        if( !bFormatFound )
        {
            Reference< chart2::data::XDataSource > xSeriesSource( xSeries, uno::UNO_QUERY );
            OUString aRole( ChartTypeHelper::getRoleOfSequenceForDataLabelNumberFormatDetection( xChartType ) );

            Reference< data::XLabeledDataSequence > xLabeledSequence(
                DataSeriesHelper::getDataSequenceByRole( xSeriesSource, aRole, false ));
            if( xLabeledSequence.is() )
            {
                Reference< data::XDataSequence > xValues( xLabeledSequence->getValues() );
                if( xValues.is() )
                    nFormat = xValues->getNumberFormatKeyByIndex( nPointIndex );
            }
        }
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
    if( !(xSeriesOrPointProp->getPropertyValue(C2U( "PercentageNumberFormat" )) >>= nFormat) )
    {
        nFormat = DiagramHelper::getPercentNumberFormat( xNumberFormatsSupplier );
    }
    if(nFormat<0)
        nFormat=0;
    return nFormat;
}

awt::Rectangle ExplicitValueProvider::addAxisTitleSizes(
            const Reference< frame::XModel >& xChartModel
            , const Reference< uno::XInterface >& xChartView
            , const awt::Rectangle& rExcludingPositionAndSize )
{
    awt::Rectangle aRet(rExcludingPositionAndSize);

    //add axis title sizes to the diagram size
    uno::Reference< chart2::XTitle > xTitle_Height( TitleHelper::getTitle( TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION, xChartModel ) );
    uno::Reference< chart2::XTitle > xTitle_Width( TitleHelper::getTitle( TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION, xChartModel ) );
    uno::Reference< chart2::XTitle > xSecondTitle_Height( TitleHelper::getTitle( TitleHelper::SECONDARY_X_AXIS_TITLE, xChartModel ) );
    uno::Reference< chart2::XTitle > xSecondTitle_Width( TitleHelper::getTitle( TitleHelper::SECONDARY_Y_AXIS_TITLE, xChartModel ) );
    if( xTitle_Height.is() || xTitle_Width.is() || xSecondTitle_Height.is() || xSecondTitle_Width.is() )
    {
        ExplicitValueProvider* pExplicitValueProvider = ExplicitValueProvider::getExplicitValueProvider(xChartView);
        if( pExplicitValueProvider )
        {
            //detect whether x axis points into x direction or not
            if( lcl_getPropertySwapXAndYAxis( ChartModelHelper::findDiagram( xChartModel ) ) )
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
                rtl::OUString aCID_X( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle_Height, xChartModel ) );
                nTitleSpaceHeight = pExplicitValueProvider->getRectangleOfObject( aCID_X, true ).Height;
                if( nTitleSpaceHeight )
                    nTitleSpaceHeight+=lcl_getDiagramTitleSpace();
            }
            if( xTitle_Width.is() )
            {
                rtl::OUString aCID_Y( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle_Width, xChartModel ) );
                nTitleSpaceWidth = pExplicitValueProvider->getRectangleOfObject( aCID_Y, true ).Width;
                if(nTitleSpaceWidth)
                    nTitleSpaceWidth+=lcl_getDiagramTitleSpace();
            }
            if( xSecondTitle_Height.is() )
            {
                rtl::OUString aCID_X( ObjectIdentifier::createClassifiedIdentifierForObject( xSecondTitle_Height, xChartModel ) );
                nSecondTitleSpaceHeight = pExplicitValueProvider->getRectangleOfObject( aCID_X, true ).Height;
                if( nSecondTitleSpaceHeight )
                    nSecondTitleSpaceHeight+=lcl_getDiagramTitleSpace();
            }
            if( xSecondTitle_Width.is() )
            {
                rtl::OUString aCID_Y( ObjectIdentifier::createClassifiedIdentifierForObject( xSecondTitle_Width, xChartModel ) );
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
            const Reference< frame::XModel >& xChartModel
            , const Reference< uno::XInterface >& xChartView
            , const awt::Rectangle& rPositionAndSizeIncludingTitles )
{
    awt::Rectangle aRet(rPositionAndSizeIncludingTitles);

    //add axis title sizes to the diagram size
    uno::Reference< chart2::XTitle > xTitle_Height( TitleHelper::getTitle( TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION, xChartModel ) );
    uno::Reference< chart2::XTitle > xTitle_Width( TitleHelper::getTitle( TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION, xChartModel ) );
    uno::Reference< chart2::XTitle > xSecondTitle_Height( TitleHelper::getTitle( TitleHelper::SECONDARY_X_AXIS_TITLE, xChartModel ) );
    uno::Reference< chart2::XTitle > xSecondTitle_Width( TitleHelper::getTitle( TitleHelper::SECONDARY_Y_AXIS_TITLE, xChartModel ) );
    if( xTitle_Height.is() || xTitle_Width.is() || xSecondTitle_Height.is() || xSecondTitle_Width.is() )
    {
        ExplicitValueProvider* pExplicitValueProvider = ExplicitValueProvider::getExplicitValueProvider(xChartView);
        if( pExplicitValueProvider )
        {
            //detect whether x axis points into x direction or not
            if( lcl_getPropertySwapXAndYAxis( ChartModelHelper::findDiagram( xChartModel ) ) )
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
                rtl::OUString aCID_X( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle_Height, xChartModel ) );
                nTitleSpaceHeight = pExplicitValueProvider->getRectangleOfObject( aCID_X, true ).Height;
                if( nTitleSpaceHeight )
                    nTitleSpaceHeight+=lcl_getDiagramTitleSpace();
            }
            if( xTitle_Width.is() )
            {
                rtl::OUString aCID_Y( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle_Width, xChartModel ) );
                nTitleSpaceWidth = pExplicitValueProvider->getRectangleOfObject( aCID_Y, true ).Width;
                if(nTitleSpaceWidth)
                    nTitleSpaceWidth+=lcl_getDiagramTitleSpace();
            }
            if( xSecondTitle_Height.is() )
            {
                rtl::OUString aCID_X( ObjectIdentifier::createClassifiedIdentifierForObject( xSecondTitle_Height, xChartModel ) );
                nSecondTitleSpaceHeight = pExplicitValueProvider->getRectangleOfObject( aCID_X, true ).Height;
                if( nSecondTitleSpaceHeight )
                    nSecondTitleSpaceHeight+=lcl_getDiagramTitleSpace();
            }
            if( xSecondTitle_Width.is() )
            {
                rtl::OUString aCID_Y( ObjectIdentifier::createClassifiedIdentifierForObject( xSecondTitle_Width, xChartModel ) );
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

double lcl_getPageLayoutDistancePercentage()
{
    return 0.02;
}

bool getAvailablePosAndSizeForDiagram(
    awt::Point& rOutPos, awt::Size& rOutAvailableDiagramSize
    , const awt::Rectangle& rSpaceLeft
    , const awt::Size & rPageSize
    , const uno::Reference< XDiagram > & xDiagram
    , bool& bUseFixedInnerSize )
{
    bUseFixedInnerSize = false;

    //@todo: we need a size dependent on the axis labels
    awt::Rectangle aRemainingSpace(rSpaceLeft);
    {
        sal_Int32 nYDistance = static_cast<sal_Int32>(rPageSize.Height*lcl_getPageLayoutDistancePercentage());
        sal_Int32 nXDistance = static_cast<sal_Int32>(rPageSize.Width*lcl_getPageLayoutDistancePercentage());
        aRemainingSpace.X+=nXDistance;
        aRemainingSpace.Width-=2*nXDistance;
        aRemainingSpace.Y+=nYDistance;
        aRemainingSpace.Height-=2*nYDistance;
    }
    if(aRemainingSpace.Width <= 0 || aRemainingSpace.Height <= 0 )
        return false;

    uno::Reference< beans::XPropertySet > xProp(xDiagram, uno::UNO_QUERY);

    bool bPosSizeExcludeAxes = false;
    if( xProp.is() )
        xProp->getPropertyValue( C2U( "PosSizeExcludeAxes" ) ) >>= bPosSizeExcludeAxes;

    //size:
    ::com::sun::star::chart2::RelativeSize aRelativeSize;
    if( xProp.is() && (xProp->getPropertyValue( C2U( "RelativeSize" ) )>>=aRelativeSize) )
    {
        rOutAvailableDiagramSize.Height = static_cast<sal_Int32>(aRelativeSize.Secondary*rPageSize.Height);
        rOutAvailableDiagramSize.Width = static_cast<sal_Int32>(aRelativeSize.Primary*rPageSize.Width);
        bUseFixedInnerSize = bPosSizeExcludeAxes;
    }
    else
        rOutAvailableDiagramSize = awt::Size(aRemainingSpace.Width,aRemainingSpace.Height);

    //position:
    chart2::RelativePosition aRelativePosition;
    if( xProp.is() && (xProp->getPropertyValue( C2U( "RelativePosition" ) )>>=aRelativePosition) )
    {
        //@todo decide whether x is primary or secondary

        //the coordinates re relative to the page
        double fX = aRelativePosition.Primary*rPageSize.Width;
        double fY = aRelativePosition.Secondary*rPageSize.Height;

        rOutPos = RelativePositionHelper::getUpperLeftCornerOfAnchoredObject(
                    awt::Point(static_cast<sal_Int32>(fX),static_cast<sal_Int32>(fY))
                    , rOutAvailableDiagramSize, aRelativePosition.Anchor );
        bUseFixedInnerSize = bPosSizeExcludeAxes;
    }
    else
        rOutPos = awt::Point(aRemainingSpace.X,aRemainingSpace.Y);

    //ensure that the diagram does not lap out right side or out of bottom
    {
        if( rOutPos.Y + rOutAvailableDiagramSize.Height > rPageSize.Height )
            rOutAvailableDiagramSize.Height = rPageSize.Height - rOutPos.Y;
        if( rOutPos.X + rOutAvailableDiagramSize.Width > rPageSize.Width )
            rOutAvailableDiagramSize.Width = rPageSize.Width - rOutPos.X;
    }

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

SAL_WNODEPRECATED_DECLARATIONS_PUSH
std::auto_ptr<VTitle> lcl_createTitle( TitleHelper::eTitleType eType
                , const uno::Reference< drawing::XShapes>& xPageShapes
                , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
                , const uno::Reference< frame::XModel >& xChartModel
                , awt::Rectangle& rRemainingSpace
                , const awt::Size & rPageSize
                , TitleAlignment eAlignment
                , bool& rbAutoPosition )
{
    std::auto_ptr<VTitle> apVTitle;

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

    uno::Reference< XTitle > xTitle( TitleHelper::getTitle( eType, xChartModel ) );
    rtl::OUString aCompleteString( TitleHelper::getCompleteString( xTitle ) );
    if( !aCompleteString.isEmpty() )
    {
        //create title
        apVTitle = std::auto_ptr<VTitle>(new VTitle(xTitle));
        rtl::OUString aCID( ObjectIdentifier::createClassifiedIdentifierForObject( xTitle, xChartModel ) );
        apVTitle->init(xPageShapes,xShapeFactory,aCID);
        apVTitle->createShapes( awt::Point(0,0), rPageSize );
        awt::Size aTitleUnrotatedSize = apVTitle->getUnrotatedSize();
        awt::Size aTitleSize = apVTitle->getFinalSize();

        //position
        rbAutoPosition=true;
        awt::Point aNewPosition(0,0);
        chart2::RelativePosition aRelativePosition;
        uno::Reference< beans::XPropertySet > xProp(xTitle, uno::UNO_QUERY);
        if( xProp.is() && (xProp->getPropertyValue( C2U( "RelativePosition" ) )>>=aRelativePosition) )
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
                rRemainingSpace.Y += ( aTitleSize.Height + nYDistance );
                rRemainingSpace.Height -= ( aTitleSize.Height + nYDistance );
                break;
            case ALIGN_BOTTOM:
                rRemainingSpace.Height -= ( aTitleSize.Height + nYDistance );
                break;
            case ALIGN_LEFT:
                rRemainingSpace.X += ( aTitleSize.Width + nXDistance );
                rRemainingSpace.Width -= ( aTitleSize.Width + nXDistance );
                break;
            case ALIGN_RIGHT:
                rRemainingSpace.Width -= ( aTitleSize.Width + nXDistance );
                break;
            default:
                break;
        }
    }
    else
    {
        // #i109336# Improve auto positioning in chart
        switch ( eAlignment )
        {
            case ALIGN_TOP:
                {
                    rRemainingSpace.Y += nYDistance;
                    rRemainingSpace.Height -= nYDistance;
                }
                break;
            case ALIGN_BOTTOM:
                {
                    rRemainingSpace.Height -= nYDistance;
                }
                break;
            case ALIGN_LEFT:
                {
                    rRemainingSpace.X += nXDistance;
                    rRemainingSpace.Width -= nXDistance;
                }
                break;
            case ALIGN_RIGHT:
                {
                    rRemainingSpace.Width -= nXDistance;
                }
                break;
            default:
                break;
        }
    }
    return apVTitle;
}
SAL_WNODEPRECATED_DECLARATIONS_POP

bool lcl_createLegend( const uno::Reference< XLegend > & xLegend
                   , const uno::Reference< drawing::XShapes>& xPageShapes
                   , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
                   , const uno::Reference< uno::XComponentContext > & xContext
                   , awt::Rectangle & rRemainingSpace
                   , const awt::Size & rPageSize
                   , const uno::Reference< frame::XModel > & xModel
                   , const std::vector< LegendEntryProvider* >& rLegendEntryProviderList
                   , sal_Int16 nDefaultWritingMode )
{
    if( VLegend::isVisible( xLegend ))
    {
        VLegend aVLegend( xLegend, xContext, rLegendEntryProviderList );
        aVLegend.init( xPageShapes, xShapeFactory, xModel );
        aVLegend.setDefaultWritingMode( nDefaultWritingMode );
        aVLegend.createShapes( awt::Size( rRemainingSpace.Width, rRemainingSpace.Height ),
                               rPageSize );
        aVLegend.changePosition( rRemainingSpace, rPageSize );
        return true;
    }
    return false;
}

void formatPage(
      const uno::Reference< frame::XModel > & xModel
    , const awt::Size rPageSize
    , const uno::Reference< drawing::XShapes >& xTarget
    , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
    )
{
    try
    {
        uno::Reference< XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
        OSL_ASSERT( xChartDoc.is());
        if( ! xChartDoc.is())
            return;
        uno::Reference< beans::XPropertySet > xModelPage( xChartDoc->getPageBackground());
        if( ! xModelPage.is())
            return;


        if( !xShapeFactory.is() )
            return;

        uno::Reference< beans::XPropertySet > xPageProp;
        // create a shape for the background
        {
            uno::Reference< drawing::XShape > xShape(
                xShapeFactory->createInstance(
                    C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );
            if( xTarget.is() &&
                xShape.is())
            {
                xTarget->add( xShape );
                xShape->setSize( rPageSize );
                xPageProp.set( xShape, uno::UNO_QUERY );
                if( xPageProp.is())
                {
                    xPageProp->setPropertyValue( C2U("LineStyle"), uno::makeAny( drawing::LineStyle_NONE ));
                }
            }
        }

        //format page
        if( xPageProp.is())
        {
            tPropertyNameValueMap aNameValueMap;
            PropertyMapper::getValueMap( aNameValueMap, PropertyMapper::getPropertyNameMapForFillAndLineProperties(), xModelPage );

            rtl::OUString aCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, rtl::OUString() ) );
            aNameValueMap.insert( tPropertyNameValueMap::value_type( C2U("Name"), uno::makeAny( aCID ) ) ); //CID rtl::OUString

            tNameSequence aNames;
            tAnySequence aValues;
            PropertyMapper::getMultiPropertyListsFromValueMap( aNames, aValues, aNameValueMap );
            PropertyMapper::setMultiProperties( aNames, aValues, xPageProp );
        }
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
        Reference< drawing::XShapes> xShapes(0);
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

bool ChartView::impl_AddInDrawsAllByItself()
{
    return false;
}

void ChartView::impl_refreshAddIn()
{
    if( !m_bRefreshAddIn )
        return;

    uno::Reference< beans::XPropertySet > xProp( m_xChartModel, uno::UNO_QUERY );
    if( xProp.is()) try
    {
        uno::Reference< util::XRefreshable > xAddIn;
        xProp->getPropertyValue( C2U( "AddIn" ) ) >>= xAddIn;
        if( xAddIn.is() )
        {
            sal_Bool bRefreshAddInAllowed = sal_True;
            xProp->getPropertyValue( C2U( "RefreshAddInAllowed" ) ) >>= bRefreshAddInAllowed;
            if( bRefreshAddInAllowed )
                xAddIn->refresh();
        }
    }
    catch( const uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

void ChartView::createShapes()
{
#if OSL_DEBUG_LEVEL > 0
    clock_t nStart = clock();
    OSL_TRACE( "\nPPPPPPPPP>>>>>>>>>>>> chart view :: createShapes()" );
#endif

    //make sure add-in is refreshed after creating the shapes
    const ::comphelper::ScopeGuard aGuard( boost::bind( &ChartView::impl_refreshAddIn, this ) );
    if( impl_AddInDrawsAllByItself() )
        return;

    m_aResultingDiagramRectangleExcludingAxes = awt::Rectangle(0,0,0,0);
    impl_deleteCoordinateSystems();
    if( m_pDrawModelWrapper )
    {
        SolarMutexGuard aSolarGuard;
        // #i12587# support for shapes in chart
        m_pDrawModelWrapper->getSdrModel().EnableUndo( sal_False );
        m_pDrawModelWrapper->clearMainDrawPage();
    }

    lcl_setDefaultWritingMode( m_pDrawModelWrapper, m_xChartModel );

    awt::Size aPageSize = ChartModelHelper::getPageSize( m_xChartModel );

    uno::Reference<drawing::XShapes> xPageShapes( ShapeFactory(m_xShapeFactory)
        .getOrCreateChartRootShape( m_xDrawPage ) );

    SdrPage* pPage = ChartView::getSdrPage();
    if(pPage) //it is neccessary to use the implementation here as the uno page does not provide a propertyset
        pPage->SetSize(Size(aPageSize.Width,aPageSize.Height));
    else
    {
        OSL_FAIL("could not set page size correctly");
    }

    {
        SolarMutexGuard aSolarGuard;

        //------------ apply fill properties to page
        // todo: it would be nicer to just pass the page m_xDrawPage and format it,
        // but the draw page does not support XPropertySet
        formatPage( m_xChartModel, aPageSize, xPageShapes, m_xShapeFactory );

        //sal_Int32 nYDistance = static_cast<sal_Int32>(aPageSize.Height*lcl_getPageLayoutDistancePercentage());
        awt::Rectangle aRemainingSpace( 0, 0, aPageSize.Width, aPageSize.Height );

        //create the group shape for diagram and axes first to have title and legends on top of it
        uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( m_xChartModel ) );
        rtl::OUString aDiagramCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM, rtl::OUString::valueOf( sal_Int32(0) ) ) );//todo: other index if more than one diagram is possible
        uno::Reference< drawing::XShapes > xDiagramPlusAxesPlusMarkHandlesGroup_Shapes( ShapeFactory(m_xShapeFactory).createGroup2D(xPageShapes,aDiagramCID) );

        uno::Reference< drawing::XShape > xDiagram_MarkHandles( ShapeFactory(m_xShapeFactory).createInvisibleRectangle(
                    xDiagramPlusAxesPlusMarkHandlesGroup_Shapes, awt::Size(0,0) ) );
        ShapeFactory::setShapeName( xDiagram_MarkHandles, C2U("MarkHandles") );

        uno::Reference< drawing::XShape > xDiagram_OuterRect( ShapeFactory(m_xShapeFactory).createInvisibleRectangle(
                    xDiagramPlusAxesPlusMarkHandlesGroup_Shapes, awt::Size(0,0) ) );
        ShapeFactory::setShapeName( xDiagram_OuterRect, C2U("PlotAreaIncludingAxes") );

        uno::Reference< drawing::XShapes > xDiagramPlusAxes_Shapes( ShapeFactory(m_xShapeFactory).createGroup2D(xDiagramPlusAxesPlusMarkHandlesGroup_Shapes ) );

        //------------ create some titles
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr<VTitle> apVTitle(0);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        bool bAutoPositionDummy = true;

        //------------ create main title shape
        lcl_createTitle( TitleHelper::MAIN_TITLE, xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, ALIGN_TOP, bAutoPositionDummy );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        //------------ create sub title shape
        lcl_createTitle( TitleHelper::SUB_TITLE, xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, ALIGN_TOP, bAutoPositionDummy );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;


        //------------ prepare series to give input to the legend (create categories and symbols etc.)
        SeriesPlotterContainer aSeriesPlotterContainer( m_aVCooSysList );
        aSeriesPlotterContainer.initializeCooSysAndSeriesPlotter( m_xChartModel );

        //------------ create legend
        lcl_createLegend( LegendHelper::getLegend( m_xChartModel ), xPageShapes, m_xShapeFactory, m_xCC
                    , aRemainingSpace, aPageSize, m_xChartModel, aSeriesPlotterContainer.getLegendEntryProviderList()
                    , lcl_getDefaultWritingModeFromPool( m_pDrawModelWrapper ) );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
        sal_Int32 nDimension = DiagramHelper::getDimension( xDiagram );

        //------------ create x axis title
        bool bAutoPosition_XTitle = true;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr<VTitle> apVTitle_X;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 0 ) )
            apVTitle_X = lcl_createTitle( TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION, xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, ALIGN_BOTTOM, bAutoPosition_XTitle );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        //------------ create y axis title
        bool bAutoPosition_YTitle = true;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr<VTitle> apVTitle_Y;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 1 ) )
            apVTitle_Y = lcl_createTitle( TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION, xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, ALIGN_LEFT, bAutoPosition_YTitle );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        //------------ create z axis title
        bool bAutoPosition_ZTitle = true;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr<VTitle> apVTitle_Z;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 2 ) )
            apVTitle_Z = lcl_createTitle( TitleHelper::Z_AXIS_TITLE, xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, ALIGN_RIGHT, bAutoPosition_ZTitle );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        bool bDummy = false;
        bool bIsVertical = DiagramHelper::getVertical( xDiagram, bDummy, bDummy );

        //------------ create secondary x axis title
        bool bAutoPosition_SecondXTitle = true;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr<VTitle> apVTitle_SecondX;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if( ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimension, 0 ) )
            apVTitle_SecondX = lcl_createTitle( TitleHelper::SECONDARY_X_AXIS_TITLE, xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, bIsVertical? ALIGN_RIGHT : ALIGN_TOP, bAutoPosition_SecondXTitle );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        //------------ create secondary y axis title
        bool bAutoPosition_SecondYTitle = true;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr<VTitle> apVTitle_SecondY;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if( ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimension, 1 ) )
            apVTitle_SecondY = lcl_createTitle( TitleHelper::SECONDARY_Y_AXIS_TITLE, xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, bIsVertical? ALIGN_TOP : ALIGN_RIGHT, bAutoPosition_SecondYTitle );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        //------------ create complete diagram shape (inclusive axis and series)
        awt::Point aAvailablePosDia;
        awt::Size  aAvailableSizeForDiagram;
        bool bUseFixedInnerSize = false;
        if( getAvailablePosAndSizeForDiagram( aAvailablePosDia, aAvailableSizeForDiagram, aRemainingSpace, aPageSize
            , ChartModelHelper::findDiagram( m_xChartModel ), bUseFixedInnerSize ) )
        {
            awt::Rectangle aUsedOuterRect = impl_createDiagramAndContent( aSeriesPlotterContainer
                        , xDiagramPlusAxes_Shapes
                        , aAvailablePosDia ,aAvailableSizeForDiagram, aPageSize, bUseFixedInnerSize, xDiagram_MarkHandles );

            if( xDiagram_OuterRect.is() )
            {
                xDiagram_OuterRect->setPosition( awt::Point( aUsedOuterRect.X, aUsedOuterRect.Y ) );
                xDiagram_OuterRect->setSize( awt::Size( aUsedOuterRect.Width, aUsedOuterRect.Height ) );
            }

            //correct axis title position
            awt::Rectangle aDiagramPlusAxesRect( aUsedOuterRect );
            if(bAutoPosition_XTitle)
                changePositionOfAxisTitle( apVTitle_X.get(), ALIGN_BOTTOM, aDiagramPlusAxesRect, aPageSize );
            if(bAutoPosition_YTitle)
                changePositionOfAxisTitle( apVTitle_Y.get(), ALIGN_LEFT, aDiagramPlusAxesRect, aPageSize );
            if(bAutoPosition_ZTitle)
                changePositionOfAxisTitle( apVTitle_Z.get(), ALIGN_Z, aDiagramPlusAxesRect, aPageSize );
            if(bAutoPosition_SecondXTitle)
                changePositionOfAxisTitle( apVTitle_SecondX.get(), bIsVertical? ALIGN_RIGHT : ALIGN_TOP, aDiagramPlusAxesRect, aPageSize );
            if(bAutoPosition_SecondYTitle)
                changePositionOfAxisTitle( apVTitle_SecondY.get(), bIsVertical? ALIGN_TOP : ALIGN_RIGHT, aDiagramPlusAxesRect, aPageSize );
        }

        //cleanup: remove all empty group shapes to avoid grey border lines:
        lcl_removeEmptyGroupShapes( xPageShapes );
    }

    // #i12587# support for shapes in chart
    if ( m_pDrawModelWrapper )
    {
        SolarMutexGuard aSolarGuard;
        m_pDrawModelWrapper->getSdrModel().EnableUndo( true );
    }

#if OSL_DEBUG_LEVEL > 0
    clock_t nEnd = clock();
    double fDuration =(double(nEnd-nStart)*1000.0)/double(CLOCKS_PER_SEC);

    OSL_TRACE( "\nPPPPPPPPP<<<<<<<<<<<< chart view :: createShapes():: needed %f msec", fDuration );
#endif
}

//-----------------------------------------------------------------
// util::XEventListener (base of XCloseListener)
//-----------------------------------------------------------------
void SAL_CALL ChartView::disposing( const lang::EventObject& /* rSource */ )
        throw(uno::RuntimeException)
{
    impl_setChartModel( 0 );
}

void ChartView::impl_updateView()
{
    if( !m_xChartModel.is() || !m_pDrawModelWrapper )
        return;

    // #i12587# support for shapes in chart
    if ( m_bSdrViewIsInEditMode )
    {
        return;
    }

    if( m_bViewDirty && !m_bInViewUpdate )
    {
        m_bInViewUpdate = true;
        //bool bOldRefreshAddIn = m_bRefreshAddIn;
        //m_bRefreshAddIn = false;
        try
        {
            impl_notifyModeChangeListener(C2U("invalid"));

            //prepare draw model
            {
                SolarMutexGuard aSolarGuard;
                m_pDrawModelWrapper->lockControllers();
            }

            //create chart view
            {
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

        impl_notifyModeChangeListener(C2U("valid"));

        //m_bRefreshAddIn = bOldRefreshAddIn;
    }
}

// ____ XModifyListener ____
void SAL_CALL ChartView::modified( const lang::EventObject& /* aEvent */ )
    throw (uno::RuntimeException)
{
    m_bViewDirty = sal_True;
    if( m_bInViewUpdate )
        m_bViewUpdatePending = true;

    impl_notifyModeChangeListener(C2U("dirty"));
}

//SfxListener
void ChartView::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    //#i77362 change notification for changes on additional shapes are missing
    if( m_bInViewUpdate )
        return;

    // #i12587# support for shapes in chart
    if ( m_bSdrViewIsInEditMode && m_xChartModel.is() )
    {
        uno::Reference< view::XSelectionSupplier > xSelectionSupplier( m_xChartModel->getCurrentController(), uno::UNO_QUERY );
        if ( xSelectionSupplier.is() )
        {
            ::rtl::OUString aSelObjCID;
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

    Reference< util::XModifiable > xModifiable( m_xChartModel, uno::UNO_QUERY );
    if( xModifiable.is() )
        xModifiable->setModified( sal_True );
}

void ChartView::impl_notifyModeChangeListener( const rtl::OUString& rNewMode )
{
    try
    {
        ::cppu::OInterfaceContainerHelper* pIC = m_aListenerContainer
            .getContainer( ::getCppuType((const uno::Reference< util::XModeChangeListener >*)0) );
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
    throw (uno::RuntimeException)
{
    m_aListenerContainer.addInterface(
        ::getCppuType((const uno::Reference< util::XModeChangeListener >*)0), xListener );
}
void SAL_CALL ChartView::removeModeChangeListener( const uno::Reference< util::XModeChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aListenerContainer.removeInterface(
        ::getCppuType((const uno::Reference< util::XModeChangeListener >*)0), xListener );
}
void SAL_CALL ChartView::addModeChangeApproveListener( const uno::Reference< util::XModeChangeApproveListener >& /* _rxListener */ )
    throw (lang::NoSupportException, uno::RuntimeException)
{

}
void SAL_CALL ChartView::removeModeChangeApproveListener( const uno::Reference< util::XModeChangeApproveListener >& /* _rxListener */ )
    throw (lang::NoSupportException, uno::RuntimeException)
{

}

// ____ XUpdatable ____
void SAL_CALL ChartView::update() throw (uno::RuntimeException)
{
    impl_updateView();

    //#i100778# migrate all imported or old documents to a plot area sizing exclusive axes (in case the save settings allow for this):
    //Although in general it is a bad idea to change the model from within the view this is exceptionally the best place to do this special conversion.
    //When a view update is requested (what happens for creating the metafile or displaying
    //the chart in edit mode or printing) it is most likely that all necessary informations are available - like the underlying spreadsheet data for example.
    //Those data is important for the correct axis lable sizes which are needed during conversion.
    if( DiagramHelper::switchDiagramPositioningToExcludingPositioning( m_xChartModel, true, false ) )
        impl_updateView();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL ChartView::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
    return 0;
}

void SAL_CALL ChartView::setPropertyValue( const ::rtl::OUString& rPropertyName
                                                     , const Any& rValue )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException
          , lang::WrappedTargetException, uno::RuntimeException)
{
    if( rPropertyName.equals(C2U("Resolution")) )
    {
        awt::Size aNewResolution;
        if( ! (rValue >>= aNewResolution) )
            throw lang::IllegalArgumentException( C2U("Property 'Resolution' requires value of type awt::Size"), 0, 0 );

        if( m_aPageResolution.Width!=aNewResolution.Width || m_aPageResolution.Height!=aNewResolution.Height )
        {
            //set modified only when the new resolution is higher and points were skipped before
            bool bSetModified = m_bPointsWereSkipped && (m_aPageResolution.Width<aNewResolution.Width || m_aPageResolution.Height<aNewResolution.Height);

            m_aPageResolution = aNewResolution;

            if( bSetModified )
                this->modified( lang::EventObject(  static_cast< uno::XWeak* >( this )  ) );
        }
    }
    else if( rPropertyName.equals(C2U("ZoomFactors")) )
    {
        //#i75867# poor quality of ole's alternative view with 3D scenes and zoomfactors besides 100%
        uno::Sequence< beans::PropertyValue > aZoomFactors;
        if( ! (rValue >>= aZoomFactors) )
            throw lang::IllegalArgumentException( C2U("Property 'ZoomFactors' requires value of type Sequence< PropertyValue >"), 0, 0 );

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
    else if( rPropertyName.equals(C2U("SdrViewIsInEditMode")) )
    {
        //#i77362 change notification for changes on additional shapes are missing
        if( ! (rValue >>= m_bSdrViewIsInEditMode) )
            throw lang::IllegalArgumentException( C2U("Property 'SdrViewIsInEditMode' requires value of type sal_Bool"), 0, 0 );
    }
    else
        throw beans::UnknownPropertyException( C2U("unknown property was tried to set to chart wizard"), 0 );
}

Any SAL_CALL ChartView::getPropertyValue( const ::rtl::OUString& rPropertyName )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;
    if( rPropertyName.equals(C2U("Resolution")) )
    {
        aRet = uno::makeAny( m_aPageResolution );
    }
    else
        throw beans::UnknownPropertyException( C2U("unknown property was tried to get from chart wizard"), 0 );
    return aRet;
}

void SAL_CALL ChartView::addPropertyChangeListener(
    const ::rtl::OUString& /* aPropertyName */, const Reference< beans::XPropertyChangeListener >& /* xListener */ )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL ChartView::removePropertyChangeListener(
    const ::rtl::OUString& /* aPropertyName */, const Reference< beans::XPropertyChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ChartView::addVetoableChangeListener( const ::rtl::OUString& /* PropertyName */, const Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ChartView::removeVetoableChangeListener( const ::rtl::OUString& /* PropertyName */, const Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

// ____ XMultiServiceFactory ____

Reference< uno::XInterface > ChartView::createInstance( const ::rtl::OUString& aServiceSpecifier )
    throw (uno::Exception, uno::RuntimeException)
{
    SdrModel* pModel = ( m_pDrawModelWrapper ? &m_pDrawModelWrapper->getSdrModel() : NULL );
    if ( pModel )
    {
        if ( aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.DashTable" ) ) == 0 )
        {
            if ( !m_xDashTable.is() )
            {
                m_xDashTable = SvxUnoDashTable_createInstance( pModel );
            }
            return m_xDashTable;
        }
        else if ( aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.GradientTable" ) ) == 0 )
        {
            if ( !m_xGradientTable.is() )
            {
                m_xGradientTable = SvxUnoGradientTable_createInstance( pModel );
            }
            return m_xGradientTable;
        }
        else if ( aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.HatchTable" ) ) == 0 )
        {
            if ( !m_xHatchTable.is() )
            {
                m_xHatchTable = SvxUnoHatchTable_createInstance( pModel );
            }
            return m_xHatchTable;
        }
        else if ( aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.BitmapTable" ) ) == 0 )
        {
            if ( !m_xBitmapTable.is() )
            {
                m_xBitmapTable = SvxUnoBitmapTable_createInstance( pModel );
            }
            return m_xBitmapTable;
        }
        else if ( aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.TransparencyGradientTable" ) ) == 0 )
        {
            if ( !m_xTransGradientTable.is() )
            {
                m_xTransGradientTable = SvxUnoTransGradientTable_createInstance( pModel );
            }
            return m_xTransGradientTable;
        }
        else if ( aServiceSpecifier.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.drawing.MarkerTable" ) ) == 0 )
        {
            if ( !m_xMarkerTable.is() )
            {
                m_xMarkerTable = SvxUnoMarkerTable_createInstance( pModel );
            }
            return m_xMarkerTable;
        }
    }

    return 0;
}

Reference< uno::XInterface > ChartView::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const uno::Sequence< uno::Any >& Arguments )
    throw (uno::Exception, uno::RuntimeException)
{
    OSL_ENSURE( Arguments.getLength(), "ChartView::createInstanceWithArguments: arguments are ignored" );
    (void) Arguments; // avoid warning
    return createInstance( ServiceSpecifier );
}

uno::Sequence< ::rtl::OUString > ChartView::getAvailableServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServiceNames( 6 );

    aServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DashTable" ) );
    aServiceNames[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.GradientTable" ) );
    aServiceNames[2] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.HatchTable" ) );
    aServiceNames[3] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.BitmapTable" ) );
    aServiceNames[4] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.TransparencyGradientTable" ) );
    aServiceNames[5] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.MarkerTable" ) );

    return aServiceNames;
}

/* ----------------------
    goes to drawinglayer/XShapeDumper.cxx
    ----------------------
namespace {

#define DEBUG_DUMPER 0

int writeCallback(void* pContext, const char* sBuffer, int nLen)
{
    rtl::OStringBuffer* pBuffer = static_cast<rtl::OStringBuffer*>(pContext);
    pBuffer->append(sBuffer);
    return nLen;
}

int closeCallback(void* )
{
    return 0;
}

void dumpPositionAsAttribute(const awt::Point& rPoint, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionX"), "%" SAL_PRIdINT32, rPoint.X);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("positionY"), "%" SAL_PRIdINT32, rPoint.Y);
}

void dumpSizeAsAttribute(const awt::Size& rSize, xmlTextWriterPtr xmlWriter)
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("sizeX"), "%" SAL_PRIdINT32, rSize.Width);
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("sizeY"), "%" SAL_PRIdINT32, rSize.Height);
}

void dumpShapeDescriptorAsAttribute( uno::Reference< drawing::XShapeDescriptor > xDescr, xmlTextWriterPtr xmlWriter )
{
    xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST("type"), "%s", rtl::OUStringToOString(xDescr->getShapeType(), RTL_TEXTENCODING_UTF8).getStr());
}

void dumpXShapes( uno::Reference< drawing::XShapes > xShapes, xmlTextWriterPtr xmlWriter );

void dumpXShape( uno::Reference< drawing::XShape > xShape, xmlTextWriterPtr xmlWriter  )
{
    xmlTextWriterStartElement( xmlWriter, BAD_CAST( "XShape" ) );

    dumpPositionAsAttribute(xShape->getPosition(), xmlWriter);
    dumpSizeAsAttribute(xShape->getSize(), xmlWriter);
    uno::Reference< drawing::XShapeDescriptor > xDescr(xShape, uno::UNO_QUERY_THROW);
    dumpShapeDescriptorAsAttribute(xDescr, xmlWriter);

    uno::Reference< lang::XServiceInfo > xServiceInfo( xShape, uno::UNO_QUERY_THROW );
    uno::Sequence< rtl::OUString > aServiceNames = xServiceInfo->getSupportedServiceNames();

    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY_THROW);
    uno::Any aAny = xPropSet->getPropertyValue("Name");
    rtl::OUString aName;
    if (aAny >>= aName)
    {
        if (!aName.isEmpty())
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("name"), "%s", rtl::OUStringToOString(aName, RTL_TEXTENCODING_UTF8).getStr());
    }
    if (xServiceInfo->supportsService("com.sun.star.drawing.Text"))
    {
        uno::Reference< text::XText > xText(xShape, uno::UNO_QUERY_THROW);
        rtl::OUString aText = xText->getString();
        if(!aText.isEmpty())
            xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("text"), "%s", rtl::OUStringToOString(aText, RTL_TEXTENCODING_UTF8).getStr());
    }
    else if(xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"))
    {
        uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY_THROW);
        dumpXShapes(xShapes, xmlWriter);
    }
#if DEBUG_DUMPER
    sal_Int32 nServices = aServiceNames.getLength();
    for (sal_Int32 i = 0; i < nServices; ++i)
    {
        xmlTextWriterStartElement(xmlWriter, BAD_CAST( "ServiceName" ));
        xmlTextWriterWriteFormatAttribute(xmlWriter, BAD_CAST( "name" ), "%s", rtl::OUStringToOString(aServiceNames[i], RTL_TEXTENCODING_UTF8).getStr());
        xmlTextWriterEndElement( xmlWriter );
    }
#endif

    xmlTextWriterEndElement( xmlWriter );
}

void dumpXShapes( uno::Reference< drawing::XShapes > xShapes, xmlTextWriterPtr xmlWriter )
{
    xmlTextWriterStartElement( xmlWriter, BAD_CAST( "XShapes" ) );
    uno::Reference< container::XIndexAccess > xIA( xShapes, uno::UNO_QUERY_THROW);
    sal_Int32 nLength = xIA->getCount();
    for (sal_Int32 i = 0; i < nLength; ++i)
    {
        uno::Reference< drawing::XShape > xShape( xIA->getByIndex( i ), uno::UNO_QUERY_THROW );
        dumpXShape( xShape, xmlWriter );
    }

    xmlTextWriterEndElement( xmlWriter );
}

}
*/

rtl::OUString ChartView::dump() throw (uno::RuntimeException)
{
    impl_updateView();
    uno::Reference<drawing::XShapes> xPageShapes( ShapeFactory(m_xShapeFactory)
        .getOrCreateChartRootShape( m_xDrawPage ) );

    if (!xPageShapes.is())
        return rtl::OUString();
    else
    {
        XShapeDumper dumper;
        return dumper.dump(xPageShapes);
    }

}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
