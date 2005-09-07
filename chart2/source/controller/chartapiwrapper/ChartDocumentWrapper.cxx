/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartDocumentWrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:00:37 $
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
#include "ChartDocumentWrapper.hxx"
#include "algohelper.hxx"
#include "macros.hxx"
#include "InlineContainer.hxx"
#include "DataSeriesTreeHelper.hxx"

#include "TitleWrapper.hxx"
#include "ChartDataWrapper.hxx"
#include "DiagramWrapper.hxx"
#include "LegendWrapper.hxx"

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
#endif

#include <vector>
#include <algorithm>
#include <functional>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
static const ::rtl::OUString lcl_aImplName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.ChartAPIWrapper" ));

enum eServiceType
{
    SERVICE_NAME_AREA_DIAGRAM = 0,
    SERVICE_NAME_BAR_DIAGRAM,
    SERVICE_NAME_DONUT_DIAGRAM,
    SERVICE_NAME_LINE_DIAGRAM,
    SERVICE_NAME_NET_DIAGRAM,
    SERVICE_NAME_PIE_DIAGRAM,
    SERVICE_NAME_STOCK_DIAGRAM,
    SERVICE_NAME_XY_DIAGRAM,

    SERVICE_NAME_DASH_TABLE,
    SERVICE_NAME_GARDIENT_TABLE,
    SERVICE_NAME_HATCH_TABLE,
    SERVICE_NAME_BITMAP_TABLE,
    SERVICE_NAME_TRANSP_GRADIENT_TABLE,
    SERVICE_NAME_MARKER_TABLE,

    SERVICE_NAME_NAMESPACE_MAP,
    SERVICE_NAME_EXPORT_GRAPHIC_RESOLVER,
    SERVICE_NAME_IMPORT_GRAPHIC_RESOLVER
};

typedef ::std::map< ::rtl::OUString, enum eServiceType > tServiceNameMap;
typedef ::comphelper::MakeMap< ::rtl::OUString, enum eServiceType > tMakeServiceNameMap;

tServiceNameMap & lcl_getStaticServiceNameMap()
{
    static tServiceNameMap aServiceNameMap(
        tMakeServiceNameMap
        ( C2U( "com.sun.star.chart.AreaDiagram" ),                    SERVICE_NAME_AREA_DIAGRAM )
        ( C2U( "com.sun.star.chart.BarDiagram" ),                     SERVICE_NAME_BAR_DIAGRAM )
        ( C2U( "com.sun.star.chart.DonutDiagram" ),                   SERVICE_NAME_DONUT_DIAGRAM )
        ( C2U( "com.sun.star.chart.LineDiagram" ),                    SERVICE_NAME_LINE_DIAGRAM )
        ( C2U( "com.sun.star.chart.NetDiagram" ),                     SERVICE_NAME_NET_DIAGRAM )
        ( C2U( "com.sun.star.chart.PieDiagram" ),                     SERVICE_NAME_PIE_DIAGRAM )
        ( C2U( "com.sun.star.chart.StockDiagram" ),                   SERVICE_NAME_STOCK_DIAGRAM )
        ( C2U( "com.sun.star.chart.XYDiagram" ),                      SERVICE_NAME_XY_DIAGRAM )

        ( C2U( "com.sun.star.drawing.DashTable" ),                    SERVICE_NAME_DASH_TABLE )
        ( C2U( "com.sun.star.drawing.GradientTable" ),                SERVICE_NAME_GARDIENT_TABLE )
        ( C2U( "com.sun.star.drawing.HatchTable" ),                   SERVICE_NAME_HATCH_TABLE )
        ( C2U( "com.sun.star.drawing.BitmapTable" ),                  SERVICE_NAME_BITMAP_TABLE )
        ( C2U( "com.sun.star.drawing.TransparencyGradientTable" ),    SERVICE_NAME_TRANSP_GRADIENT_TABLE )
        ( C2U( "com.sun.star.drawing.MarkerTable" ),                  SERVICE_NAME_MARKER_TABLE )

        ( C2U( "com.sun.star.xml.NamespaceMap" ),                     SERVICE_NAME_NAMESPACE_MAP )
        ( C2U( "com.sun.star.document.ExportGraphicObjectResolver" ), SERVICE_NAME_EXPORT_GRAPHIC_RESOLVER )
        ( C2U( "com.sun.star.document.ImportGraphicObjectResolver" ), SERVICE_NAME_IMPORT_GRAPHIC_RESOLVER )
        );

    return aServiceNameMap;
}

} //  anonymous namespace

namespace chart
{
namespace wrapper
{

ChartDocumentWrapper::ChartDocumentWrapper(
    const Reference< uno::XComponentContext > & xContext ) :
        m_xContext( xContext ),
        m_bIsDisposed( false )
{
    // note: do not listen at m_xModel.  This is the owner of the object and
    // calls dispose() itself
}

ChartDocumentWrapper::~ChartDocumentWrapper()
{
    stopAllComponentListening();
}

::osl::Mutex & ChartDocumentWrapper::GetMutex() const
{
    return m_aMutex;
}

// ____ XInterface (for new interfaces) ____
uno::Any SAL_CALL ChartDocumentWrapper::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException)
{
    if( m_xDelegator.is())
        return m_xDelegator->queryInterface( aType );
    else
        return queryAggregation( aType );
}

// ____ chart::XChartDocument (old API wrapper) ____
Reference< drawing::XShape > SAL_CALL ChartDocumentWrapper::getTitle()
    throw (uno::RuntimeException)
{
    if( ! m_xTitle.is() &&
        m_xChartDoc.is())
    {
        try
        {
            uno::Reference< chart2::XTitled > xTitled( m_xChartDoc, uno::UNO_QUERY_THROW );
            m_xTitle = new TitleWrapper( xTitled->getTitle(), m_xContext, GetMutex() );
            Reference< lang::XComponent > xComp( m_xTitle, uno::UNO_QUERY_THROW );
            startComponentListening( xComp );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return m_xTitle;
}

Reference< drawing::XShape > SAL_CALL ChartDocumentWrapper::getSubTitle()
    throw (uno::RuntimeException)
{
    if( ! m_xSubTitle.is() &&
        m_xChartDoc.is() )
    {
        try
        {
            uno::Reference< chart2::XTitled > xTitled( m_xChartDoc->getDiagram(), uno::UNO_QUERY_THROW );
            m_xSubTitle = new TitleWrapper( xTitled->getTitle(), m_xContext, GetMutex() );
            Reference< lang::XComponent > xComp( m_xSubTitle, uno::UNO_QUERY_THROW );
            startComponentListening( xComp );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return m_xSubTitle;
}

Reference< drawing::XShape > SAL_CALL ChartDocumentWrapper::getLegend()
    throw (uno::RuntimeException)
{
    if( ! m_xLegend.is())
    {
        m_xLegend = new LegendWrapper( m_xChartDoc, m_xContext, GetMutex() );
        Reference< lang::XComponent > xComp( m_xLegend, uno::UNO_QUERY );
        if( xComp.is())
            startComponentListening( xComp );
    }

    return m_xLegend;
}

Reference< beans::XPropertySet > SAL_CALL ChartDocumentWrapper::getArea()
    throw (uno::RuntimeException)
{
    return m_xChartDoc->getPageBackground();
}

Reference< XDiagram > SAL_CALL ChartDocumentWrapper::getDiagram()
    throw (uno::RuntimeException)
{
    if( ! m_xDiagram.is() &&
        m_xChartDoc.is() )
    {
        try
        {
            m_xDiagram = new DiagramWrapper(
                m_xChartDoc->getDiagram(),
                m_xContext, GetMutex() );
            Reference< lang::XComponent > xComp( m_xDiagram, uno::UNO_QUERY );
            startComponentListening( xComp );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return m_xDiagram;
}

void SAL_CALL ChartDocumentWrapper::setDiagram( const Reference< XDiagram >& xDiagram )
    throw (uno::RuntimeException)
{
    if( xDiagram.is() &&
        xDiagram != m_xDiagram )
    {
        // set new wrapped diagram at new chart.  This requires the old
        // diagram given as parameter to implement the new interface.  If
        // this is not possible throw an exception
        Reference< chart2::XDiagramProvider > xNewDiaProvider( xDiagram, uno::UNO_QUERY_THROW );
        Reference< chart2::XDiagram > xNewDia( xNewDiaProvider->getDiagram());

        try
        {
            // stop listening and dispose old diagram
            if( m_xDiagram.is())
            {
                Reference< lang::XComponent > xComp( m_xDiagram, uno::UNO_QUERY_THROW );
                stopComponentListening( xComp );
                xComp->dispose();
            }

            // set the new diagram and start listening
            m_xChartDoc->setDiagram( xNewDia );
            m_xDiagram = xDiagram;
            Reference< lang::XComponent > xComp( m_xDiagram, uno::UNO_QUERY_THROW );
            startComponentListening( xComp );
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

Reference< XChartData > SAL_CALL ChartDocumentWrapper::getData()
    throw (uno::RuntimeException)
{
    if( ! m_xChartData.is())
    {
        m_xChartData = new ChartDataWrapper( m_xChartDoc, m_xContext, GetMutex());
    }

    return m_xChartData;
}

void SAL_CALL ChartDocumentWrapper::attachData( const Reference< XChartData >& xData )
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "not implemented" );
}

// ____ XModel ____
sal_Bool SAL_CALL ChartDocumentWrapper::attachResource(
    const ::rtl::OUString& URL,
    const Sequence< beans::PropertyValue >& Arguments )
    throw (uno::RuntimeException)
{
    return m_xModel->attachResource( URL, Arguments );
}

::rtl::OUString SAL_CALL ChartDocumentWrapper::getURL()
    throw (uno::RuntimeException)
{
    return m_xModel->getURL();
}

Sequence< beans::PropertyValue > SAL_CALL ChartDocumentWrapper::getArgs()
    throw (uno::RuntimeException)
{
    return m_xModel->getArgs();
}

void SAL_CALL ChartDocumentWrapper::connectController( const Reference< frame::XController >& Controller )
    throw (uno::RuntimeException)
{
    m_xModel->connectController( Controller );
}

void SAL_CALL ChartDocumentWrapper::disconnectController(
    const Reference< frame::XController >& Controller )
    throw (uno::RuntimeException)
{
    m_xModel->disconnectController( Controller );
}

void SAL_CALL ChartDocumentWrapper::lockControllers()
    throw (uno::RuntimeException)
{
    m_xModel->lockControllers();
}

void SAL_CALL ChartDocumentWrapper::unlockControllers()
    throw (uno::RuntimeException)
{
    m_xModel->unlockControllers();
}

sal_Bool SAL_CALL ChartDocumentWrapper::hasControllersLocked()
    throw (uno::RuntimeException)
{
    return m_xModel->hasControllersLocked();
}

Reference< frame::XController > SAL_CALL ChartDocumentWrapper::getCurrentController()
    throw (uno::RuntimeException)
{
    return Reference< frame::XController >();
}

void SAL_CALL ChartDocumentWrapper::setCurrentController(
    const Reference< frame::XController >& Controller )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    m_xModel->setCurrentController( Controller );
}

Reference< uno::XInterface > SAL_CALL ChartDocumentWrapper::getCurrentSelection()
    throw (uno::RuntimeException)
{
    return m_xModel->getCurrentSelection();
}


// ____ XComponent ____
void SAL_CALL ChartDocumentWrapper::dispose()
    throw (uno::RuntimeException)
{
    if( m_bIsDisposed )
        throw lang::DisposedException(
            C2U("ChartDocumentWrapper is disposed" ),
            static_cast< ::cppu::OWeakObject* >( this ));

    m_bIsDisposed = true;

    try
    {
        m_xModel->dispose();
    }
    catch( lang::DisposedException )
    {
        // this is ok, don't panic
    }
    catch( uno::Exception &ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    Reference< lang::XComponent > xComp;
    if( m_xTitle.is())
    {
        try
        {
            xComp.set( m_xTitle, uno::UNO_QUERY );
            if( xComp.is())
            {
                stopComponentListening( xComp );
                xComp->dispose();
            }
            m_xTitle = NULL;
        }
        catch( uno::Exception &ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    if( m_xSubTitle.is())
    {
        try
        {
            xComp.set( m_xSubTitle, uno::UNO_QUERY );
            if( xComp.is())
            {
                stopComponentListening( xComp );
                xComp->dispose();
            }
            m_xSubTitle = NULL;
        }
        catch( uno::Exception &ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    if( m_xDiagram.is())
    {
        try
        {
            xComp.set( m_xDiagram, uno::UNO_QUERY );
            if( xComp.is())
            {
                stopComponentListening( xComp );
                xComp->dispose();
            }
            m_xDiagram = NULL;
        }
        catch( uno::Exception &ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

void SAL_CALL ChartDocumentWrapper::addEventListener( const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_xModel->addEventListener( xListener );
}

void SAL_CALL ChartDocumentWrapper::removeEventListener( const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_xModel->removeEventListener( aListener );
}


// ____ XNumberFormatsSupplier ____
uno::Reference< beans::XPropertySet > SAL_CALL ChartDocumentWrapper::getNumberFormatSettings()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not yet implemented" );
    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< util::XNumberFormats > SAL_CALL ChartDocumentWrapper::getNumberFormats()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not yet implemented" );
    return uno::Reference< util::XNumberFormats >();
}


// ____ XDrawPageSupplier ____
uno::Reference< drawing::XDrawPage > SAL_CALL ChartDocumentWrapper::getDrawPage()
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "Not yet implemented" );
    return uno::Reference< drawing::XDrawPage >();
}


// ____ XMultiServiceFactory ____
uno::Reference< uno::XInterface > SAL_CALL ChartDocumentWrapper::createInstance(
    const ::rtl::OUString& aServiceSpecifier )
    throw (uno::Exception,
           uno::RuntimeException)
{
    uno::Reference< uno::XInterface > xResult;
    bool bServiceFound = false;
    tServiceNameMap & rMap = lcl_getStaticServiceNameMap();

    tServiceNameMap::const_iterator aIt( rMap.find( aServiceSpecifier ));
    if( aIt != rMap.end())
    {
        bool bCreateDiagram = false;
        uno::Reference< lang::XMultiServiceFactory > xManagerFact(
            m_xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
        uno::Reference< chart2::XChartTypeTemplate > xTemplate;

        switch( (*aIt).second )
        {
            case SERVICE_NAME_AREA_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance(
                            C2U( "com.sun.star.chart2.template.Area" )), uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_BAR_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance(
                            C2U( "com.sun.star.chart2.template.Bar" )), uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_DONUT_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance(
                            C2U( "com.sun.star.chart2.template.Ring" )), uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_LINE_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance(
                            C2U( "com.sun.star.chart2.template.Line" )), uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_NET_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance(
                            C2U( "com.sun.star.chart2.template.Net" )), uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_PIE_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance(
                            C2U( "com.sun.star.chart2.template.Pie" )), uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_STOCK_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance(
                            C2U( "com.sun.star.chart2.template.StockLowHighClose" )), uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;
            case SERVICE_NAME_XY_DIAGRAM:
                if( xManagerFact.is())
                {
                    xTemplate.set(
                        xManagerFact->createInstance(
                            C2U( "com.sun.star.chart2.template.ScatterSymbol" )), uno::UNO_QUERY );
                    bCreateDiagram = true;
                }
                break;

            case SERVICE_NAME_DASH_TABLE:
//              xDashTable = SvxUnoDashTable_createInstance( m_pModel );
                break;
            case SERVICE_NAME_GARDIENT_TABLE:
//              xGradientTable = SvxUnoGradientTable_createInstance( m_pModel );
                break;
            case SERVICE_NAME_HATCH_TABLE:
//              xHatchTable = SvxUnoHatchTable_createInstance( m_pModel );
                break;
            case SERVICE_NAME_BITMAP_TABLE:
//              xBitmapTable = SvxUnoBitmapTable_createInstance( m_pModel );
                break;
            case SERVICE_NAME_TRANSP_GRADIENT_TABLE:
//              xTransparencyGradientTable = SvxUnoTransGradientTable_createInstance( m_pModel );
                break;
            case SERVICE_NAME_MARKER_TABLE:
//                  xMarkerTable = SvxUnoMarkerTable_createInstance( m_pModel );
                break;

            case SERVICE_NAME_NAMESPACE_MAP:
//                 static sal_uInt16 aWhichIds[] = { SCHATTR_USER_DEFINED_ATTR, 0 };
//                 xResult = svx::NamespaceMap_createInstance( aWhichIds, &m_pModel->GetPool() );
                break;
            case SERVICE_NAME_EXPORT_GRAPHIC_RESOLVER:
//                 xResult = static_cast< ::cppu::OWeakObject * >( new SvXMLGraphicHelper( GRAPHICHELPER_MODE_WRITE ));
                break;
            case SERVICE_NAME_IMPORT_GRAPHIC_RESOLVER:
//                 xResult = static_cast< ::cppu::OWeakObject * >( new SvXMLGraphicHelper( GRAPHICHELPER_MODE_READ ));
                break;
        }

        if( bCreateDiagram && xTemplate.is() )
        {
            try
            {
                uno::Sequence< uno::Reference< chart2::XDataSeries > > aSeriesSeq(
                    helper::DataSeriesTreeHelper::getDataSeriesFromDiagram(
                        m_xChartDoc->getDiagram() ));

                uno::Reference< chart2::XDiagram > xDia(
                    xTemplate->createDiagram( aSeriesSeq ));
                xResult = static_cast< ::cppu::OWeakObject* >(
                    new DiagramWrapper( xDia, m_xContext, GetMutex()));
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }

        bServiceFound = true;
    }
    else
    {
        // try to create a shape
        try
        {
//             xResult = SvxUnoDrawMSFactory::createInstance( aServiceSpecifier );
            bServiceFound = true;
        }
        catch( const uno::Exception& aEx )
        {
            // couldn't create shape
        }
    }

    // finally, try to create an addin
    if( ! bServiceFound )
    {
//         uno::Reference< util::XRefreshable > xAddIn = GetAddInCollection().GetAddInByName( aServiceSpecifier );
//         if( xAddIn.is())
//         {
//             xResult = xAddIn;
            bServiceFound = true;
//         }
    }

    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL ChartDocumentWrapper::createInstanceWithArguments(
    const ::rtl::OUString& ServiceSpecifier,
    const uno::Sequence< uno::Any >& Arguments )
    throw (uno::Exception,
           uno::RuntimeException)
{
    OSL_ENSURE( Arguments.getLength(), "createInstanceWithArguments: Warning: Arguments are ignored" );

    return createInstance( ServiceSpecifier );
}

uno::Sequence< ::rtl::OUString > SAL_CALL ChartDocumentWrapper::getAvailableServiceNames()
    throw (uno::RuntimeException)
{
    tServiceNameMap & rMap = lcl_getStaticServiceNameMap();
    uno::Sequence< ::rtl::OUString > aResult( rMap.size());

    ::std::transform( rMap.begin(), rMap.end(),
                      aResult.getArray(),
                      ::std::select1st< tServiceNameMap::value_type >() );

    return aResult;

//         // shapes
// //         uno::Sequence< OUString > aDrawServices( SvxUnoDrawMSFactory::getAvailableServiceNames() );
// //         const OUString * pArr = aDrawServices.getConstArray();
// //         aServices.insert( aServices.end(), pArr, pArr + aDrawServices.getLength() );
//     }


    // add-ins
 //    uno::Sequence< OUString > aAddIns( GetAddInCollection().GetAddInNames() );
//     pArr = aAddIns.getConstArray();
//     aServices.insert( aServices.end(), pArr, pArr + aAddIns.getLength() );

//     return helper::VectorToSequence( aServices );
}

// ____ XAggregation ____
void SAL_CALL ChartDocumentWrapper::setDelegator(
    const uno::Reference< uno::XInterface >& rDelegator )
    throw (uno::RuntimeException)
{
    m_xDelegator = rDelegator;
    m_xModel.set( m_xDelegator, uno::UNO_QUERY );
    m_xChartDoc.set( m_xDelegator, uno::UNO_QUERY );
}

uno::Any SAL_CALL ChartDocumentWrapper::queryAggregation( const uno::Type& aType )
    throw (uno::RuntimeException)
{
    return impl::ChartDocumentWrapper_Base::queryInterface( aType );
}

// ____ ::utl::OEventListenerAdapter ____
void ChartDocumentWrapper::_disposing( const lang::EventObject& _rSource )
{
    if( _rSource.Source == m_xTitle )
        m_xTitle = NULL;
    if( _rSource.Source == m_xSubTitle )
        m_xSubTitle = NULL;
    if( _rSource.Source == m_xDiagram )
        m_xDiagram = NULL;
}

// ================================================================================

uno::Sequence< ::rtl::OUString > ChartDocumentWrapper::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartDocument" );
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartDocumentWrapper" );
    aServices[ 2 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 3 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ChartDocumentWrapper, lcl_aImplName );

} //  namespace wrapper
} //  namespace chart
