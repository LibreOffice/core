/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartView.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 16:02:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ChartView.hxx"
#include "chartview/DrawModelWrapper.hxx"
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

#include <comphelper/scopeguard.hxx>
#include <boost/bind.hpp>

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
// header for class LocaleDataWrapper
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
// header for class SdrPage
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
// header for class SvxDrawPage
#ifndef _SVX_UNOWPAGE_HXX
#include <svx/unopage.hxx>
#endif
// header for class SvxShape
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
// header for class Application
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <time.h>

#include <com/sun/star/chart/DataLabelPlacement.hpp>

#ifndef _COM_SUN_STAR_CHART2_EXPLICITSUBINCREMENT_HPP_
#include <com/sun/star/chart2/ExplicitSubIncrement.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_STACKINGDIRECTION_HPP_
#include <com/sun/star/chart2/StackingDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEMCONTAINER_HPP_
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPECONTAINER_HPP_
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIESCONTAINER_HPP_
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_RELATIVEPOSITION_HPP_
#include <com/sun/star/chart2/RelativePosition.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_RELATIVESIZE_HPP_
#include <com/sun/star/chart2/RelativeSize.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEGROUP_HPP_
#include <com/sun/star/drawing/XShapeGroup.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif

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

//static
const uno::Sequence<sal_Int8>& ExplicitValueProvider::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

//static
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
    DBG_ASSERT(aArguments.getLength() >= 1,"need 1 argument to initialize the view: xModel");
    if( !(aArguments.getLength() >= 1) )
        return;

    uno::Reference< frame::XModel > xNewChartModel;
    if( !(aArguments[0] >>= xNewChartModel) )
    {
        DBG_ERROR( "need a Reference to frame::XModel as first parameter for view initialization" );
    }
    impl_setChartModel( xNewChartModel );

    if( !m_pDrawModelWrapper.get() )
    {
        // /--
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        m_pDrawModelWrapper = ::boost::shared_ptr< DrawModelWrapper >( new DrawModelWrapper( m_xCC ) );
        m_xShapeFactory = m_pDrawModelWrapper->getShapeFactory();
        m_xDrawPage = m_pDrawModelWrapper->getMainDrawPage();
        StartListening( m_pDrawModelWrapper->getSdrModel(), FALSE /*bPreventDups*/ );
        // \--
    }
}

ChartView::~ChartView()
{
    if( m_pDrawModelWrapper.get() )
        EndListening( m_pDrawModelWrapper->getSdrModel(), FALSE /*bAllDups*/ );
    m_xDrawPage = NULL;
    impl_deleteCoordinateSystems();
}

void ChartView::impl_deleteCoordinateSystems()
{
    //delete all coordinate systems
    ::std::vector< VCoordinateSystem* >::const_iterator       aIter = m_aVCooSysList.begin();
    const ::std::vector< VCoordinateSystem* >::const_iterator aEnd  = m_aVCooSysList.end();
    for( ; aIter != aEnd; aIter++ )
    {
        delete *aIter;
    }
    m_aVCooSysList.clear();
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

    impl_updateView();

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

typedef std::pair< sal_Int32, sal_Int32 > tFullAxisIndex; //first index is the dimension, second index is the axis index that indicates wether this is a main or secondary axis
typedef std::pair< VCoordinateSystem* , tFullAxisIndex > tFullCoordinateSystem;
typedef std::map< VCoordinateSystem*, tFullAxisIndex > tCoordinateSystemMap;

struct AxisUsage
{
    AxisUsage();
    ~AxisUsage();

    void addCoordinateSystem( VCoordinateSystem* pCooSys, sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );
    ::std::vector< VCoordinateSystem* > getCoordinateSystems( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );
    sal_Int32 getMaxAxisIndexForDimension( sal_Int32 nDimensionIndex );
    //tFullAxisIndex getDimensionAndIndexForCooSys( VCoordinateSystem* pCooSys );

    ScaleAutomatism         aScaleAutomatism;

private:
    tCoordinateSystemMap    aCoordinateSystems;
    std::map< sal_Int32, sal_Int32 > aMaxIndexPerDimension;
};

AxisUsage::AxisUsage()
    : aScaleAutomatism(AxisHelper::createDefaultScale())
{
}

AxisUsage::~AxisUsage()
{
    aCoordinateSystems.clear();
}

/*
tFullScaleIndex AxisUsage::getDimensionAndIndexForCooSys( VCoordinateSystem* pCooSys )
{
    tFullScaleIndex aRet(0,0);

    tCoordinateSystemMap::const_iterator aFound( aCoordinateSystems.find(pCooSys) );
    if(aFound!=aCoordinateSystems.end())
        aRet = aFound->second;

    return aRet;
}
*/

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
    void initAxisUsageList();
    void doAutoScaling();
    void updateScalesAndIncrementsOnAxes();
    void setScalesFromCooSysToPlotter();
    void setNumberFormatsFromAxes();
    drawing::Direction3D getPreferredAspectRatio();
    bool shouldKeep2DAspectRatio();

    std::vector< VSeriesPlotter* >& getSeriesPlotterList() { return m_aSeriesPlotterList; }
    std::vector< VCoordinateSystem* >& getCooSysList() { return m_rVCooSysList; }
    std::vector< LegendEntryProvider* > getLegendEntryProviderList();

private:
    std::vector< VSeriesPlotter* > m_aSeriesPlotterList;
    std::vector< VCoordinateSystem* >& m_rVCooSysList;
    ::std::map< uno::Reference< XAxis >, AxisUsage > m_aAxisUsageList;
    sal_Int32 m_nMaxAxisIndex;
};

SeriesPlotterContainer::SeriesPlotterContainer( std::vector< VCoordinateSystem* >& rVCooSysList )
        : m_rVCooSysList( rVCooSysList )
        , m_nMaxAxisIndex(0)
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
    for( aPlotterIter = m_aSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; aPlotterIter++ )
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
    sal_Int32 nDiagramIndex = 0;//todo if more than one diagam is supported
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    if( !xDiagram.is())
        return;

    uno::Reference< util::XNumberFormatsSupplier > xNumberFormatsSupplier( xChartModel, uno::UNO_QUERY );

    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
    if(!nDimensionCount)
    {
        //@todo handle mixed dimension
        nDimensionCount = 2;
    }

    sal_Bool bSortByXValues = sal_False;
    sal_Bool bConnectBars = sal_False;
    sal_Bool bGroupBarsPerAxis = sal_True;
    sal_Int32 nStartingAngle = 90;
    try
    {
        uno::Reference< beans::XPropertySet > xDiaProp( xDiagram, uno::UNO_QUERY_THROW );
        xDiaProp->getPropertyValue( C2U( "SortByXValues" ) ) >>= bSortByXValues;
        xDiaProp->getPropertyValue( C2U( "ConnectBars" ) ) >>= bConnectBars;
        xDiaProp->getPropertyValue( C2U( "GroupBarsPerAxis" ) ) >>= bGroupBarsPerAxis;
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

            VSeriesPlotter* pPlotter = VSeriesPlotter::createSeriesPlotter( xChartType, nDimensionCount );
            if( !pPlotter )
                continue;
            m_aSeriesPlotterList.push_back( pPlotter );
            pPlotter->setNumberFormatsSupplier( xNumberFormatsSupplier );
            pPlotter->setColorScheme( xColorScheme );
            pPlotter->setExplicitCategoriesProvider( pVCooSys->getExplicitCategoriesProvider() );

            if(pVCooSys)
                pVCooSys->addMinimumAndMaximumSupplier(pPlotter);

            //------------ add series to plotter and thus prepare him for providing minimum and maximum values
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
                VDataSeries* pSeries = new VDataSeries( xDataSeries );

                pSeries->setGlobalSeriesIndex(nGlobalSeriesIndex);
                nGlobalSeriesIndex++;

                if( bSortByXValues )
                    pSeries->doSortByXValues();

                pSeries->setConnectBars( bConnectBars );
                pSeries->setGroupBarsPerAxis( bGroupBarsPerAxis );
                pSeries->setStartingAngle( nStartingAngle );

                rtl::OUString aSeriesParticle( ObjectIdentifier::createParticleForSeries( nDiagramIndex, nCS, nT, nS ) );
                pSeries->setParticle(aSeriesParticle);

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
    if( m_aSeriesPlotterList.size() )
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

void SeriesPlotterContainer::initAxisUsageList()
{
    m_aAxisUsageList.clear();
    size_t nC;
    for( nC=0; nC < m_rVCooSysList.size(); nC++)
    {
        VCoordinateSystem* pVCooSys = m_rVCooSysList[nC];
        for(sal_Int32 nDimensionIndex=0; nDimensionIndex<3; nDimensionIndex++)
        {
            uno::Reference< XCoordinateSystem > xCooSys = pVCooSys->getModel();
            if( nDimensionIndex >= xCooSys->getDimension() )
                continue;
            const sal_Int32 nMaximumAxisIndex = xCooSys->getMaximumAxisIndexByDimension(nDimensionIndex);
            for(sal_Int32 nAxisIndex=0; nAxisIndex<=nMaximumAxisIndex; ++nAxisIndex)
            {
                uno::Reference< XAxis > xAxis( xCooSys->getAxisByDimension( nDimensionIndex, nAxisIndex ) );
                OSL_ASSERT( xAxis.is());
                if( xAxis.is())
                {
                    if(m_aAxisUsageList.find(xAxis)==m_aAxisUsageList.end())
                        m_aAxisUsageList[xAxis].aScaleAutomatism = ScaleAutomatism(xAxis->getScaleData());
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
        for( aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; aAxisIter++ )
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
    for( aPlotterIter = m_aSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; aPlotterIter++ )
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
    //set numberfarmats to plotter to enable them to display the data labels in the numberfromat of teh axis

    ::std::vector< VSeriesPlotter* >::const_iterator       aPlotterIter = m_aSeriesPlotterList.begin();
    const ::std::vector< VSeriesPlotter* >::const_iterator aPlotterEnd  = m_aSeriesPlotterList.end();
    for( aPlotterIter = m_aSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; aPlotterIter++ )
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
                        }
                    }
                    catch( lang::IndexOutOfBoundsException& e )
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

void SeriesPlotterContainer::doAutoScaling()
{
    //precondition: need a initialized m_aSeriesPlotterList
    //precondition: need a initialized m_aAxisUsageList

    ::std::map< uno::Reference< XAxis >, AxisUsage >::iterator             aAxisIter    = m_aAxisUsageList.begin();
    const ::std::map< uno::Reference< XAxis >, AxisUsage >::const_iterator aAxisEndIter = m_aAxisUsageList.end();

    //iterate over the main scales first than secondary axis
    size_t nC;
    for( sal_Int32 nAxisIndex=0; nAxisIndex<=m_nMaxAxisIndex; nAxisIndex++ )
    {

        // - first do autoscale for all x and z scales (because they are treated independent)
        for( aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; aAxisIter++ )
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
        for( aAxisIter = m_aAxisUsageList.begin(); aAxisIter != aAxisEndIter; aAxisIter++ )
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
        ; aPlotterIter != aPlotterEnd; aPlotterIter++, nPlotterCount++ )
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

bool SeriesPlotterContainer::shouldKeep2DAspectRatio()
{
    bool bOutKeep2DAspectRatio = false;

    //detect wether the aspect ratio needs to be kept
    ::std::vector< VSeriesPlotter* >::const_iterator       aPlotterIter = m_aSeriesPlotterList.begin();
    const ::std::vector< VSeriesPlotter* >::const_iterator aPlotterEnd  = m_aSeriesPlotterList.end();
    for( aPlotterIter = m_aSeriesPlotterList.begin()
        ; aPlotterIter != aPlotterEnd; aPlotterIter++ )
    {
        bOutKeep2DAspectRatio = bOutKeep2DAspectRatio || (*aPlotterIter)->keepAspectRatio();
    }

    return bOutKeep2DAspectRatio;
}

//-----------------------------------------------------

namespace
{

bool lcl_resizeAfterCompleteCreation( const uno::Reference< XDiagram >& xDiagram )
{
    //special treatment for pie charts
    //the size is checked after complete creation to get the datalabels into the given space

    //todo: this is just a workaround at the moment for pie and donut labels
    return DiagramHelper::isPieOrDonutChart( xDiagram );
}

} //end anonymous namespace

//------------ create complete diagram shape (inclusive axis and series)
void ChartView::impl_createDiagramAndContent( SeriesPlotterContainer& rSeriesPlotterContainer
            , const uno::Reference< drawing::XShapes>& xDiagramPlusAxes_Shapes
            , const awt::Point& rAvailablePos
            , const awt::Size& rAvailableSize
            , const awt::Size& rPageSize )
{
//     sal_Int32 nDiagramIndex = 0;//todo if more than one diagam is supported
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( m_xChartModel ) );
    if( !xDiagram.is())
        return;

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
        pVCooSys->createVAxisList( xNumberFormatsSupplier
                                        , rAvailableSize //font reference size
                                        , BaseGFXHelper::B2IRectangleToAWTRectangle( aAvailableOuterRect ) //maximum space for labels
                                        );
    }


    // - prepare list of all axis and how they are used
    rSeriesPlotterContainer.initAxisUsageList();
    rSeriesPlotterContainer.doAutoScaling();
    rSeriesPlotterContainer.setScalesFromCooSysToPlotter();
    rSeriesPlotterContainer.setNumberFormatsFromAxes();

    //---------------------------------------------------------------------
    //create shapes

    //------------ create diagram shapes
    //aspect ratio
    drawing::Direction3D aPreferredAspectRatio(
        rSeriesPlotterContainer.getPreferredAspectRatio() );
//     bool bKeepAspectRatio = rSeriesPlotterContainer.shouldKeep2DAspectRatio();

    uno::Reference< drawing::XShapes > xCoordinateRegionTarget(0);
    VDiagram aVDiagram(xDiagram, aPreferredAspectRatio, nDimensionCount);
    {//create diagram
        aVDiagram.init(xDiagramPlusAxes_Shapes,xDiagramPlusAxes_Shapes,m_xShapeFactory);
        aVDiagram.createShapes(rAvailablePos,rAvailableSize);
        xCoordinateRegionTarget = aVDiagram.getCoordinateRegion();
        aVDiagram.reduceToMimimumSize();
    }

    uno::Reference< drawing::XShapes > xTextTargetShapes( ShapeFactory(m_xShapeFactory).createGroup2D(xDiagramPlusAxes_Shapes) );

    // - create axis and grids for all coordinate systems

    //init all coordinate systems
    for( nC=0; nC < rVCooSysList.size(); nC++)
    {
        VCoordinateSystem* pVCooSys = rVCooSysList[nC];
        pVCooSys->initPlottingTargets(xCoordinateRegionTarget,xTextTargetShapes,m_xShapeFactory);

        pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
            createTransformationSceneToScreen( aVDiagram.getCurrentRectangle() ) ));

        pVCooSys->initVAxisInList();
    }

    //calculate resulting size respecting axis label layout and fontscaling

    //use first coosys only so far; todo: calculate for more than one coosys if we have more in future
    //todo: this is just a workaround at the moment for pie and donut labels
    if( !lcl_resizeAfterCompleteCreation(xDiagram) && rVCooSysList.size() > 0 )
    {
        uno::Reference< drawing::XShape > xBoundingShape( xDiagramPlusAxes_Shapes, uno::UNO_QUERY );

        ::basegfx::B2IRectangle aFirstConsumedOuterRect( ShapeFactory::getRectangleOfShape(xBoundingShape) );

        VCoordinateSystem* pVCooSys = rVCooSysList[0];
        pVCooSys->createMaximumAxesLabels();

        ::basegfx::B2IRectangle aConsumedOuterRect( ShapeFactory::getRectangleOfShape(xBoundingShape) );
        ::basegfx::B2IRectangle aNewInnerRect( aVDiagram.adjustInnerSize( aConsumedOuterRect ) );

        pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
            createTransformationSceneToScreen( aNewInnerRect ) ));

        //redo autoscaling to get size and text dependent automatic main increment count
        rSeriesPlotterContainer.doAutoScaling();
        rSeriesPlotterContainer.updateScalesAndIncrementsOnAxes();
        rSeriesPlotterContainer.setScalesFromCooSysToPlotter();

        pVCooSys->createAxesLabels();

        bool bLessSpaceConsumedThanExpected = false;
        {
            ::basegfx::B2IRectangle aOldRect( aConsumedOuterRect );
            aConsumedOuterRect = ShapeFactory::getRectangleOfShape(xBoundingShape);
            if( aConsumedOuterRect.getMinX() > aAvailableOuterRect.getMinX()
                || aConsumedOuterRect.getMaxX() < aAvailableOuterRect.getMaxX()
                || aConsumedOuterRect.getMinY() > aAvailableOuterRect.getMinY()
                || aConsumedOuterRect.getMinY() < aAvailableOuterRect.getMaxY() )
                bLessSpaceConsumedThanExpected = true;
        }

        if( bLessSpaceConsumedThanExpected )
        {
            aVDiagram.adjustInnerSize( aConsumedOuterRect );
            pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
                createTransformationSceneToScreen( aVDiagram.getCurrentRectangle() ) ));
            pVCooSys->updatePositions();
        }
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
    for( aPlotterIter = rSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; aPlotterIter++ )
    {
        //------------ set transformation to plotter / create series
        VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
        pSeriesPlotter->setPageReferenceSize( rPageSize );
        rtl::OUString aCID; //III
        pSeriesPlotter->initPlotter(xCoordinateRegionTarget,xTextTargetShapes,m_xShapeFactory,aCID);
        pSeriesPlotter->setDiagramReferenceSize( rAvailableSize );
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

    //recreate with corrected sizes if requested
    if( lcl_resizeAfterCompleteCreation(xDiagram) )
    {
        m_bPointsWereSkipped = false;

        uno::Reference< drawing::XShape > xBoundingShape( xDiagramPlusAxes_Shapes, uno::UNO_QUERY );
        ::basegfx::B2IRectangle aConsumedOuterRect( ShapeFactory::getRectangleOfShape(xBoundingShape) );

        ::basegfx::B2IRectangle aNewInnerRect( aVDiagram.adjustInnerSize( aConsumedOuterRect ) );

        for( aPlotterIter = rSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; aPlotterIter++ )
        {
            VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
            pSeriesPlotter->releaseShapes();
        }

        //clear and recreate
        ShapeFactory::removeSubShapes( xCoordinateRegionTarget );
        ShapeFactory::removeSubShapes( xTextTargetShapes );

        //set new transformation
        for( nC=0; nC < rVCooSysList.size(); nC++)
        {
            VCoordinateSystem* pVCooSys = rVCooSysList[nC];
            pVCooSys->setTransformationSceneToScreen( B3DHomMatrixToHomogenMatrix(
                createTransformationSceneToScreen( aNewInnerRect ) ));
        }

        // - create data series for all charttypes
        for( aPlotterIter = rSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; aPlotterIter++ )
        {
            //------------ set transformation to plotter / create series
            VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
            VCoordinateSystem* pVCooSys = lcl_getCooSysForPlotter( rVCooSysList, pSeriesPlotter );
            if(2==nDimensionCount)
                pSeriesPlotter->setTransformationSceneToScreen( pVCooSys->getTransformationSceneToScreen() );
            pSeriesPlotter->createShapes();
            m_bPointsWereSkipped = m_bPointsWereSkipped || pSeriesPlotter->PointsWereSkipped();
        }

        /*
        uno::Reference< drawing::XShape > xDiagramPlusAxes_KeepRatio( xDiagramPlusAxes_Shapes, uno::UNO_QUERY );

        awt::Size aNewSize( rAvailableSize );
        awt::Point aNewPos( rAvailablePos );
        if( bKeepAspectRatio )
        {
            awt::Size aCurrentSize( xDiagramPlusAxes_KeepRatio->getSize());

            aNewSize = ShapeFactory::calculateNewSizeRespectingAspectRatio(
                            rAvailableSize, aCurrentSize );
            aNewPos = ShapeFactory::calculateTopLeftPositionToCenterObject(
                    rAvailablePos, rAvailableSize, aNewSize );
        }

        xDiagramPlusAxes_KeepRatio->setPosition( aNewPos );
        xDiagramPlusAxes_KeepRatio->setSize( aNewSize );
        */
        for( aPlotterIter = rSeriesPlotterList.begin(); aPlotterIter != aPlotterEnd; aPlotterIter++ )
        {
            VSeriesPlotter* pSeriesPlotter = *aPlotterIter;
            pSeriesPlotter->rearrangeLabelToAvoidOverlapIfRequested( rPageSize );
        }
    }
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
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    SdrObject* pObj = DrawModelWrapper::getNamedSdrObject( rObjectCID, this->getSdrPage() );
    if( pObj )
        return uno::Reference< drawing::XShape >( pObj->getUnoShape(), uno::UNO_QUERY);
    return 0;
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
            ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
            SvxShape* pRoot = SvxShape::getImplementation( xShape );
            if( pRoot )
            {
                SdrObject* pRootSdrObject = pRoot->GetSdrObject();
                if( pRootSdrObject )
                {
                    SdrObjList* pRootList = pRootSdrObject->GetSubList();
                    if( pRootList )
                    {
                        SdrObject* pShape = DrawModelWrapper::getNamedSdrObject( C2U("MarkHandles"), pRootList );
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
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
    }
    return bSwapXAndY;
}

}

//static
sal_Int32 ExplicitValueProvider::getExplicitNumberFormatKeyForAxis(
                  const Reference< chart2::XAxis >& xAxis
                , const Reference< chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem
                , const Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier )
{
    sal_Int32 nNumberFormatKey(0);
    Reference< beans::XPropertySet > xProp( xAxis, uno::UNO_QUERY );
    if( xProp.is() && !( xProp->getPropertyValue( C2U( "NumberFormat" ) ) >>= nNumberFormatKey ) )
    {
        bool bPercentFormatSet = false;
        //check wether we have a percent scale -> use percent format
        if( xNumberFormatsSupplier.is() )
        {
            ScaleData aData = xAxis->getScaleData();
            if( aData.AxisType==AxisType::PERCENT )
            {
                sal_Int32 nPercentFormat = ExplicitValueProvider::getPercentNumberFormat( xNumberFormatsSupplier );
                if( nPercentFormat != -1 )
                {
                    nNumberFormatKey = nPercentFormat;
                    bPercentFormatSet = true;
                }
            }
        }

        if( !bPercentFormatSet )
        {
            typedef ::std::map< sal_Int32, sal_Int32 > tNumberformatFrequency;
            tNumberformatFrequency aKeyMap;

            try
            {
                Reference< XChartTypeContainer > xCTCnt( xCorrespondingCoordinateSystem, uno::UNO_QUERY_THROW );
                if( xCTCnt.is() )
                {
                    sal_Int32 nDimensionIndex = 1;
                    sal_Int32 nAxisIndex = 0;
                    AxisHelper::getIndicesForAxis( xAxis, xCorrespondingCoordinateSystem, nDimensionIndex, nAxisIndex );
                    ::rtl::OUString aRoleToMatch;
                    if( nDimensionIndex == 0 )
                        aRoleToMatch = C2U("values-x");
                    Sequence< Reference< XChartType > > aChartTypes( xCTCnt->getChartTypes());
                    for( sal_Int32 nCTIdx=0; nCTIdx<aChartTypes.getLength(); ++nCTIdx )
                    {
                        if( nDimensionIndex != 0 )
                            aRoleToMatch = aChartTypes[nCTIdx]->getRoleOfSequenceForSeriesLabel();
                        Reference< XDataSeriesContainer > xDSCnt( aChartTypes[nCTIdx], uno::UNO_QUERY_THROW );
                        Sequence< Reference< XDataSeries > > aDataSeriesSeq( xDSCnt->getDataSeries());
                        for( sal_Int32 nSeriesIdx=0; nSeriesIdx<aDataSeriesSeq.getLength(); ++nSeriesIdx )
                        {
                            Reference< chart2::XDataSeries > xDataSeries(aDataSeriesSeq[nSeriesIdx]);
                            Reference< data::XDataSource > xSource( xDataSeries, uno::UNO_QUERY_THROW );

                            if( nDimensionIndex == 1 )
                            {
                                //only take those series into accoutn that are attached to this axis
                                sal_Int32 nAttachedAxisIndex = DataSeriesHelper::getAttachedAxisIndex(xDataSeries);
                                if( nAttachedAxisIndex != nAxisIndex )
                                    continue;
                            }

                            Sequence< Reference< data::XLabeledDataSequence > > aLabeledSeq( xSource->getDataSequences());
                            for( sal_Int32 nLSeqIdx=0; nLSeqIdx<aLabeledSeq.getLength(); ++nLSeqIdx )
                            {
                                if(!aLabeledSeq[nLSeqIdx].is())
                                    continue;
                                Reference< data::XDataSequence > xSeq( aLabeledSeq[nLSeqIdx]->getValues());
                                OSL_ASSERT( xSeq.is());
                                Reference< beans::XPropertySet > xSeqProp( xSeq, uno::UNO_QUERY );
                                ::rtl::OUString aRole;
                                bool bTakeIntoAccount =
                                    ( xSeqProp.is() && (aRoleToMatch.getLength() > 0) &&
                                    (xSeqProp->getPropertyValue(C2U("Role")) >>= aRole ) &&
                                    aRole.equals( aRoleToMatch ));

                                if( bTakeIntoAccount )
                                {
                                    sal_Int32 nKey = xSeq->getNumberFormatKeyByIndex( -1 );
                                    // initialize the value
                                    if( aKeyMap.find( nKey ) == aKeyMap.end())
                                        aKeyMap[ nKey ] = 0;
                                    // increase frequency
                                    aKeyMap[ nKey ] = (aKeyMap[ nKey ] + 1);
                                }
                            }
                        }
                    }
                }
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }

            if( ! aKeyMap.empty())
            {
                sal_Int32 nMaxFreq = 0;
                // find most frequent key
                for( tNumberformatFrequency::const_iterator aIt = aKeyMap.begin();
                    aIt != aKeyMap.end(); ++aIt )
                {
                    OSL_TRACE( "NumberFormatKey %d appears %d times", (*aIt).first, (*aIt).second );
                    // all values must at least be 1
                    if( (*aIt).second > nMaxFreq )
                    {
                        nNumberFormatKey = (*aIt).first;
                        nMaxFreq = (*aIt).second;
                    }
                }
            }
        }
    }
    return nNumberFormatKey;
}

//static
sal_Int32 ExplicitValueProvider::getPercentNumberFormat( const Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier )
{
    sal_Int32 nRet=-1;
    Reference< util::XNumberFormats > xNumberFormats( xNumberFormatsSupplier->getNumberFormats() );
    if( xNumberFormats.is() )
    {
        sal_Bool bCreate = sal_True;
        const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
        Sequence<sal_Int32> aKeySeq = xNumberFormats->queryKeys( util::NumberFormat::PERCENT,
            rLocaleDataWrapper.getLocale(), bCreate );
        if( aKeySeq.getLength() )
        {
            nRet = aKeySeq[0];
        }
    }
    return nRet;
}


sal_Int32 ExplicitValueProvider::getExplicitNumberFormatKeyForLabel(
        const uno::Reference< beans::XPropertySet >& xSeriesOrPointProp,
        const uno::Reference< XDataSeries >& xSeries,
        sal_Int32 nPointIndex /*-1 for whole series*/,
        const uno::Reference< beans::XPropertySet >& xAttachedAxisProps
        )
{
    sal_Int32 nFormat=0;
    if( !xSeriesOrPointProp.is() )
        return nFormat;
    rtl::OUString aPropName( C2U( "NumberFormat" ) );
    if( !(xSeriesOrPointProp->getPropertyValue(aPropName) >>= nFormat) )
    {
        if( xAttachedAxisProps.is() && !( xAttachedAxisProps->getPropertyValue( aPropName ) >>= nFormat ) )
        {
            Reference< chart2::data::XDataSource > xSeriesSource( xSeries, uno::UNO_QUERY );
            Reference< data::XLabeledDataSequence > xLabeledSequence(
                DataSeriesHelper::getDataSequenceByRole( xSeriesSource, C2U("values-y"), false ));
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

sal_Int32 ExplicitValueProvider::getExplicitPercentageNumberFormatKeyForLabel(
        const uno::Reference< beans::XPropertySet >& xSeriesOrPointProp,
        const uno::Reference< util::XNumberFormatsSupplier >& xNumberFormatsSupplier )
{
    sal_Int32 nFormat=0;
    if( !xSeriesOrPointProp.is() )
        return nFormat;
    if( !(xSeriesOrPointProp->getPropertyValue(C2U( "PercentageNumberFormat" )) >>= nFormat) )
    {
        nFormat = ExplicitValueProvider::getPercentNumberFormat( xNumberFormatsSupplier );
    }
    if(nFormat<0)
        nFormat=0;
    return nFormat;
}

//static
awt::Rectangle ExplicitValueProvider::calculateDiagramPositionAndSizeInclusiveTitle(
            const Reference< frame::XModel >& xChartModel
            , const Reference< uno::XInterface >& xChartView
            , const awt::Rectangle& rExclusivePositionAndSize )
{
    awt::Rectangle aRet(rExclusivePositionAndSize);

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
            //detect wether x axis points into x direction or not
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

double lcl_getPageLayoutDistancePercentage()
{
    return 0.02;
}

bool getAvailablePosAndSizeForDiagram(
    awt::Point& rOutPos, awt::Size& rOutAvailableDiagramSize
    , const awt::Rectangle& rSpaceLeft
    , const awt::Size & rPageSize
    , const uno::Reference< XDiagram > & xDiagram
    , VTitle* pXTitle, VTitle* pYTitle
    , VTitle* pSecondXTitle, VTitle* pSecondYTitle )
{
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

    bool bMakeRoomForTitle = false;

    //size:
    ::com::sun::star::chart2::RelativeSize aRelativeSize;
    if( xProp.is() && (xProp->getPropertyValue( C2U( "RelativeSize" ) )>>=aRelativeSize) )
    {
        rOutAvailableDiagramSize.Height = static_cast<sal_Int32>(aRelativeSize.Secondary*rPageSize.Height);
        rOutAvailableDiagramSize.Width = static_cast<sal_Int32>(aRelativeSize.Primary*rPageSize.Width);
        bMakeRoomForTitle = true;
    }
    else
        rOutAvailableDiagramSize = awt::Size(aRemainingSpace.Width,aRemainingSpace.Height);

    //position:
    chart2::RelativePosition aRelativePosition;
    if( xProp.is() && (xProp->getPropertyValue( C2U( "RelativePosition" ) )>>=aRelativePosition) )
    {
        //@todo decide wether x is primary or secondary

        //the coordinates re relative to the page
        double fX = aRelativePosition.Primary*rPageSize.Width;
        double fY = aRelativePosition.Secondary*rPageSize.Height;

        rOutPos = RelativePositionHelper::getUpperLeftCornerOfAnchoredObject(
                    awt::Point(static_cast<sal_Int32>(fX),static_cast<sal_Int32>(fY))
                    , rOutAvailableDiagramSize, aRelativePosition.Anchor );
        bMakeRoomForTitle = true;
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

    if( bMakeRoomForTitle )
    {
        sal_Int32 nTitleSpaceWidth = 0;
        sal_Int32 nTitleSpaceHeight = 0;
        sal_Int32 nSecondTitleSpaceWidth = 0;
        sal_Int32 nSecondTitleSpaceHeight = 0;
        {
            //todo detect wether x axis points into x direction or not
            //detect wether x axis points into x direction or not
            if( lcl_getPropertySwapXAndYAxis( xDiagram ) )
            {
                std::swap( pXTitle, pYTitle );
                std::swap( pSecondXTitle, pSecondYTitle );
            }

            if( pXTitle )
            {
                nTitleSpaceHeight = pXTitle->getFinalSize().Height;
                if(nTitleSpaceHeight)
                    nTitleSpaceHeight+=lcl_getDiagramTitleSpace();
            }
            if( pYTitle )
            {
                nTitleSpaceWidth = pYTitle->getFinalSize().Width;
                if(nTitleSpaceWidth)
                    nTitleSpaceWidth+=lcl_getDiagramTitleSpace();
            }
            if( pSecondXTitle)
            {
                nSecondTitleSpaceHeight += pSecondXTitle->getFinalSize().Height;
                if(nSecondTitleSpaceHeight)
                    nSecondTitleSpaceHeight+=lcl_getDiagramTitleSpace();
            }
            if( pSecondYTitle)
            {
                nSecondTitleSpaceWidth += pSecondYTitle->getFinalSize().Width;
                if(nSecondTitleSpaceWidth)
                    nSecondTitleSpaceWidth+=lcl_getDiagramTitleSpace();
            }
        }
        rOutAvailableDiagramSize.Height -= nTitleSpaceHeight + nSecondTitleSpaceHeight;
        rOutAvailableDiagramSize.Width -= nTitleSpaceWidth + nSecondTitleSpaceWidth;
        rOutPos.X += nTitleSpaceWidth;
        rOutPos.Y += nSecondTitleSpaceHeight;
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

    pVTitle->changePosition( aNewPosition );
}

std::auto_ptr<VTitle> lcl_createTitle( const uno::Reference< XTitle >& xTitle
                , const uno::Reference< drawing::XShapes>& xPageShapes
                , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
                , const uno::Reference< frame::XModel >& xChartModel
                , awt::Rectangle& rRemainingSpace
                , const awt::Size & rPageSize
                , TitleAlignment eAlignment
                , bool& rbAutoPosition )
{
    std::auto_ptr<VTitle> apVTitle;
    if(xTitle.is())
    {
        rtl::OUString aCompleteString( TitleHelper::getCompleteString( xTitle ) );
        if( aCompleteString.getLength()==0 )
            return apVTitle;//don't create empty titles as the resulting diagram position is wrong then

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
        sal_Int32 nYDistance = static_cast<sal_Int32>(rPageSize.Height*lcl_getPageLayoutDistancePercentage());
        sal_Int32 nXDistance = static_cast<sal_Int32>(rPageSize.Width*lcl_getPageLayoutDistancePercentage());
        chart2::RelativePosition aRelativePosition;
        uno::Reference< beans::XPropertySet > xProp(xTitle, uno::UNO_QUERY);
        if( xProp.is() && (xProp->getPropertyValue( C2U( "RelativePosition" ) )>>=aRelativePosition) )
        {
            rbAutoPosition = false;

            //@todo decide wether x is primary or secondary
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
    return apVTitle;
}

bool lcl_createLegend( const uno::Reference< XLegend > & xLegend
                   , const uno::Reference< drawing::XShapes>& xPageShapes
                   , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
                   , const uno::Reference< uno::XComponentContext > & xContext
                   , awt::Rectangle & rRemainingSpace
                   , const awt::Size & rPageSize
                   , const uno::Reference< frame::XModel > & xModel
                   , const std::vector< LegendEntryProvider* >& rLegendEntryProviderList )
{
    if( VLegend::isVisible( xLegend ))
    {
        VLegend aVLegend( xLegend, xContext, rLegendEntryProviderList );
        aVLegend.init( xPageShapes, xShapeFactory, xModel );
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
    catch( uno::Exception & ex )
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
    bool bRet = false;
    /*
    uno::Reference< beans::XPropertySet > xProp( m_xChartModel, uno::UNO_QUERY );
    if( xProp.is()) try
    {
        uno::Reference< util::XRefreshable > xAddIn;
        xProp->getPropertyValue( C2U( "AddIn" ) ) >>= xAddIn;
        if( xAddIn.is() )
        {
            rtl::OUString aBaseDiagram;
            xProp->getPropertyValue( C2U( "BaseDiagram" ) ) >>= aBaseDiagram;
            if(aBaseDiagram.getLength())
                bRet = true;
        }
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
    */
    return bRet;
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
    catch( uno::Exception& e )
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

    impl_deleteCoordinateSystems();
    if( m_pDrawModelWrapper )
    {
        // /--
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        m_pDrawModelWrapper->clearMainDrawPage();
        // \--
    }

    awt::Size aPageSize = ChartModelHelper::getPageSize( m_xChartModel );

    uno::Reference<drawing::XShapes> xPageShapes( ShapeFactory(m_xShapeFactory)
        .getOrCreateChartRootShape( m_xDrawPage ) );

    SdrPage* pPage = ChartView::getSdrPage();
    if(pPage) //it is neccessary to use the implementation here as the uno page does not provide a propertyset
        pPage->SetSize(Size(aPageSize.Width,aPageSize.Height));
    else
    {
        DBG_ERROR("could not set page size correctly");
    }

    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());

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
        uno::Reference< drawing::XShape > xDiagramPlusAxes_MarkHandles( ShapeFactory(m_xShapeFactory).createInvisibleRectangle(
                    xDiagramPlusAxesPlusMarkHandlesGroup_Shapes, awt::Size(0,0) ) );
        ShapeFactory::setShapeName( xDiagramPlusAxes_MarkHandles, C2U("MarkHandles") );
        uno::Reference< drawing::XShapes > xDiagramPlusAxes_Shapes( ShapeFactory(m_xShapeFactory).createGroup2D(xDiagramPlusAxesPlusMarkHandlesGroup_Shapes ) );

        //------------ create some titles
        std::auto_ptr<VTitle> apVTitle(0);
        bool bAutoPositionDummy = true;

        //------------ create main title shape
        lcl_createTitle( TitleHelper::getTitle( TitleHelper::MAIN_TITLE, m_xChartModel ), xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, ALIGN_TOP, bAutoPositionDummy );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        //------------ create sub title shape
        lcl_createTitle( TitleHelper::getTitle( TitleHelper::SUB_TITLE, m_xChartModel ), xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, ALIGN_TOP, bAutoPositionDummy );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;


        //------------ prepare series to give input to the legend (create categories and symbols etc.)
        SeriesPlotterContainer aSeriesPlotterContainer( m_aVCooSysList );
        aSeriesPlotterContainer.initializeCooSysAndSeriesPlotter( m_xChartModel );

        //------------ create legend
        lcl_createLegend( LegendHelper::getLegend( m_xChartModel ), xPageShapes, m_xShapeFactory, m_xCC
                    , aRemainingSpace, aPageSize, m_xChartModel, aSeriesPlotterContainer.getLegendEntryProviderList() );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
        sal_Int32 nDimension = DiagramHelper::getDimension( xDiagram );

        //------------ create x axis title
        bool bAutoPosition_XTitle = true;
        std::auto_ptr<VTitle> apVTitle_X;
        if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 0 ) )
            apVTitle_X = lcl_createTitle( TitleHelper::getTitle( TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION, m_xChartModel ), xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, ALIGN_BOTTOM, bAutoPosition_XTitle );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        //------------ create y axis title
        bool bAutoPosition_YTitle = true;
        std::auto_ptr<VTitle> apVTitle_Y;
        if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 1 ) )
            apVTitle_Y = lcl_createTitle( TitleHelper::getTitle( TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION, m_xChartModel ), xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, ALIGN_LEFT, bAutoPosition_YTitle );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        //------------ create z axis title
        bool bAutoPosition_ZTitle = true;
        std::auto_ptr<VTitle> apVTitle_Z;
        if( ChartTypeHelper::isSupportingMainAxis( xChartType, nDimension, 2 ) )
            apVTitle_Z = lcl_createTitle( TitleHelper::getTitle( TitleHelper::Z_AXIS_TITLE, m_xChartModel ), xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, ALIGN_RIGHT, bAutoPosition_ZTitle );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        bool bDummy = false;
        bool bIsVertical = DiagramHelper::getVertical( xDiagram, bDummy, bDummy );

        //------------ create secondary x axis title
        bool bAutoPosition_SecondXTitle = true;
        std::auto_ptr<VTitle> apVTitle_SecondX;
        if( ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimension, 0 ) )
            apVTitle_SecondX = lcl_createTitle( TitleHelper::getTitle( TitleHelper::SECONDARY_X_AXIS_TITLE, m_xChartModel ), xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, bIsVertical? ALIGN_RIGHT : ALIGN_TOP, bAutoPosition_SecondXTitle );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        //------------ create secondary y axis title
        bool bAutoPosition_SecondYTitle = true;
        std::auto_ptr<VTitle> apVTitle_SecondY;
        if( ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimension, 1 ) )
            apVTitle_SecondY = lcl_createTitle( TitleHelper::getTitle( TitleHelper::SECONDARY_Y_AXIS_TITLE, m_xChartModel ), xPageShapes, m_xShapeFactory, m_xChartModel
                    , aRemainingSpace, aPageSize, bIsVertical? ALIGN_TOP : ALIGN_RIGHT, bAutoPosition_SecondYTitle );
        if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
            return;

        //------------ create complete diagram shape (inclusive axis and series)
        awt::Point aAvailablePosDia;
        awt::Size  aAvailableSizeForDiagram;
        if( getAvailablePosAndSizeForDiagram( aAvailablePosDia, aAvailableSizeForDiagram, aRemainingSpace, aPageSize, ChartModelHelper::findDiagram( m_xChartModel )
                , apVTitle_X.get(), apVTitle_Y.get(), apVTitle_SecondX.get(), apVTitle_SecondY.get() ) )
        {
            impl_createDiagramAndContent( aSeriesPlotterContainer
                        , xDiagramPlusAxes_Shapes
                        , aAvailablePosDia ,aAvailableSizeForDiagram, aPageSize );

            if(xDiagramPlusAxes_MarkHandles.is())
            {
                xDiagramPlusAxes_MarkHandles->setPosition( aAvailablePosDia );
                xDiagramPlusAxes_MarkHandles->setSize( aAvailableSizeForDiagram );
            }

            //correct axis title position
            awt::Rectangle aDiagramPlusAxesRect(aAvailablePosDia.X,aAvailablePosDia.Y,aAvailableSizeForDiagram.Width,aAvailableSizeForDiagram.Height);
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
                // /--
                ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
                m_pDrawModelWrapper->lockControllers();
                m_pDrawModelWrapper->updateTablesFromChartModel( m_xChartModel );
                // \--
            }

            //create chart view
            {
                /*
                ::vos::OGuard aGuard( Application::GetSolarMutex());
                while( m_bViewDirty )
                {
                    createShapes();
                    m_bViewDirty = m_bViewUpdatePending;
                    m_bViewUpdatePending = false;
                    m_bInViewUpdate = false;
                }
                */

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
        catch( uno::Exception& ex)
        {
            m_bViewDirty = m_bViewUpdatePending;
            m_bViewUpdatePending = false;
            m_bInViewUpdate = false;
            ASSERT_EXCEPTION( ex );
        }

        {
            // /--
            ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
            m_pDrawModelWrapper->unlockControllers();
            // \--
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
    if( m_bSdrViewIsInEditMode )
        return;

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
        default:
            break;
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
                (static_cast< util::XModeChangeListener*>(aIt.next()))->modeChanged( aEvent );
        }
    }
    catch( uno::Exception& ex)
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
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL ChartView::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    OSL_ENSURE(false,"not implemented");
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
            if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ScaleXNumerator" ) ) )
                pDataValues->Value >>= m_nScaleXNumerator;
            else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ScaleXDenominator" ) ) )
                pDataValues->Value >>= m_nScaleXDenominator;
            else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ScaleYNumerator" ) ) )
                pDataValues->Value >>= m_nScaleYNumerator;
            else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ScaleYDenominator" ) ) )
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
    OSL_ENSURE(false,"not implemented");
}
void SAL_CALL ChartView::removePropertyChangeListener(
    const ::rtl::OUString& /* aPropertyName */, const Reference< beans::XPropertyChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_ENSURE(false,"not implemented");
}

void SAL_CALL ChartView::addVetoableChangeListener( const ::rtl::OUString& /* PropertyName */, const Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_ENSURE(false,"not implemented");
}

void SAL_CALL ChartView::removeVetoableChangeListener( const ::rtl::OUString& /* PropertyName */, const Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_ENSURE(false,"not implemented");
}


//.............................................................................
} //namespace chart
//.............................................................................
