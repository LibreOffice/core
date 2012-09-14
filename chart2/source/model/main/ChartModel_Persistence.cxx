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


#include "ChartModel.hxx"
#include "MediaDescriptorHelper.hxx"
#include "ChartDebugTrace.hxx"
#include "macros.hxx"
#include "ChartViewHelper.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"
#include "ThreeDHelper.hxx"

#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <ucbhelper/content.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/cvtgrf.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <vcl/svapp.hxx>

#include <algorithm>
#include <functional>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::osl::MutexGuard;

namespace
{
struct lcl_PropNameEquals : public ::std::unary_function< beans::PropertyValue, bool >
{
    lcl_PropNameEquals( const OUString & rStrToCompareWith ) :
            m_aStr( rStrToCompareWith )
    {}
    bool operator() ( const beans::PropertyValue & rProp )
    {
        return rProp.Name.equals( m_aStr );
    }
private:
    OUString m_aStr;
};

template< typename T >
T lcl_getProperty(
    const Sequence< beans::PropertyValue > & rMediaDescriptor,
    const OUString & rPropName )
{
    T aResult;
    if( rMediaDescriptor.getLength())
    {
        OUString aPropName( rPropName );
        const beans::PropertyValue * pIt = rMediaDescriptor.getConstArray();
        const beans::PropertyValue * pEndIt = pIt +  + rMediaDescriptor.getLength();
        pIt = ::std::find_if( pIt, pEndIt, lcl_PropNameEquals( aPropName ));
        if( pIt != pEndIt )
            (*pIt).Value >>= aResult;
    }
    return aResult;
}

void lcl_addStorageToMediaDescriptor(
    Sequence< beans::PropertyValue > & rOutMD,
    const Reference< embed::XStorage > & xStorage )
{
    rOutMD.realloc( rOutMD.getLength() + 1 );
    rOutMD[rOutMD.getLength() - 1] = beans::PropertyValue(
        C2U("Storage"), -1, uno::makeAny( xStorage ), beans::PropertyState_DIRECT_VALUE );
}

Reference< embed::XStorage > lcl_createStorage(
    const OUString & rURL,
    const Reference< uno::XComponentContext > & xContext,
    const Sequence< beans::PropertyValue > & rMediaDescriptor )
{
    // create new storage
    Reference< embed::XStorage > xStorage;
    if( !xContext.is())
        return xStorage;

    try
    {
        Reference< io::XStream > xStream(
            ::ucbhelper::Content( rURL, Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext()).openStream(),
            uno::UNO_QUERY );

        Reference< lang::XSingleServiceFactory > xStorageFact(
            xContext->getServiceManager()->createInstanceWithContext(
                C2U("com.sun.star.embed.StorageFactory"),
                xContext ),
            uno::UNO_QUERY_THROW );
        Sequence< uno::Any > aStorageArgs( 3 );
        aStorageArgs[0] <<= xStream;
        aStorageArgs[1] <<= embed::ElementModes::READWRITE;
        aStorageArgs[2] <<= rMediaDescriptor;
        xStorage.set(
            xStorageFact->createInstanceWithArguments( aStorageArgs ), uno::UNO_QUERY_THROW );
        OSL_ENSURE( xStorage.is(), "No Storage" );
    }
    catch( const ::com::sun::star::ucb::ContentCreationException & rEx )
    {
        ASSERT_EXCEPTION( rEx );
    }

    return xStorage;
}

} // anonymous namespace

namespace chart
{

Reference< document::XFilter > ChartModel::impl_createFilter(
    const Sequence< beans::PropertyValue > & rMediaDescriptor )
{
    Reference< document::XFilter > xFilter;

    // find FilterName in MediaDescriptor
    OUString aFilterName(
        lcl_getProperty< OUString >( rMediaDescriptor, OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ))));

    // if FilterName was found, get Filter from factory
    if( !aFilterName.isEmpty() )
    {
        try
        {
            Reference< container::XNameAccess > xFilterFact(
                m_xContext->getServiceManager()->createInstanceWithContext(
                    C2U( "com.sun.star.document.FilterFactory" ), m_xContext ),
                uno::UNO_QUERY_THROW );
            uno::Any aFilterProps( xFilterFact->getByName( aFilterName ));
            Sequence< beans::PropertyValue > aProps;

            if( aFilterProps.hasValue() &&
                (aFilterProps >>= aProps))
            {
                OUString aFilterServiceName(
                    lcl_getProperty< OUString >( aProps, OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterService" ))));

                if( !aFilterServiceName.isEmpty())
                {
                    xFilter.set(
                        m_xContext->getServiceManager()->createInstanceWithContext(
                            aFilterServiceName, m_xContext ), uno::UNO_QUERY_THROW );
                    OSL_TRACE( "Filter found for service %s", U2C( aFilterServiceName ));
                }
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
        OSL_ENSURE( xFilter.is(), "Filter not found via factory" );
    }

    // fall-back: create XML-Filter
    if( ! xFilter.is())
    {
        OSL_TRACE( "No FilterName passed in MediaDescriptor" );
        xFilter.set(
            m_xContext->getServiceManager()->createInstanceWithContext(
                C2U("com.sun.star.comp.chart2.XMLFilter"), m_xContext ),
            uno::UNO_QUERY_THROW );
    }

    return xFilter;
}

//-----------------------------------------------------------------
// frame::XStorable2
//-----------------------------------------------------------------

void SAL_CALL ChartModel::storeSelf( const Sequence< beans::PropertyValue >& rMediaDescriptor )
    throw (lang::IllegalArgumentException,
           io::IOException,
           uno::RuntimeException)
{
    // only some parameters are allowed (see also SfxBaseModel)
    // "VersionComment", "Author", "InteractionHandler", "StatusIndicator"
    // However, they are ignored here.  They would become interesting when
    // charts support a standalone format again.
    impl_store( rMediaDescriptor, m_xStorage );
}

//-----------------------------------------------------------------
// frame::XStorable (base of XStorable2)
//-----------------------------------------------------------------
sal_Bool SAL_CALL ChartModel::hasLocation()
    throw(uno::RuntimeException)
{
    //@todo guard
    return !m_aResource.isEmpty();
}

::rtl::OUString SAL_CALL ChartModel::getLocation()
    throw(uno::RuntimeException)
{
    return impl_g_getLocation();
}

sal_Bool SAL_CALL ChartModel::isReadonly()
    throw(uno::RuntimeException)
{
    //@todo guard
    return m_bReadOnly;
}

void SAL_CALL ChartModel::store()
    throw(io::IOException,
          uno::RuntimeException)
{
    apphelper::LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall(sal_True)) //start LongLastingCall
        return; //behave passive if already disposed or closed or throw exception @todo?

    ::rtl::OUString aLocation = m_aResource;

    if( aLocation.isEmpty() )
        throw io::IOException( C2U( "no location specified" ), static_cast< ::cppu::OWeakObject* >(this));
    //@todo check whether aLocation is something like private:factory...
    if( m_bReadOnly )
        throw io::IOException( C2U( "document is read only" ), static_cast< ::cppu::OWeakObject* >(this));

    aGuard.clear();

    // store
    impl_store( m_aMediaDescriptor, m_xStorage );
}

void SAL_CALL ChartModel::storeAsURL(
    const ::rtl::OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rMediaDescriptor )
    throw(io::IOException, uno::RuntimeException)
{
    apphelper::LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall(sal_True)) //start LongLastingCall
        return; //behave passive if already disposed or closed or throw exception @todo?

    apphelper::MediaDescriptorHelper aMediaDescriptorHelper(rMediaDescriptor);
    uno::Sequence< beans::PropertyValue > aReducedMediaDescriptor(
        aMediaDescriptorHelper.getReducedForModel() );

    m_bReadOnly = sal_False;
    aGuard.clear();

    // create new storage
    Reference< embed::XStorage > xStorage( lcl_createStorage( rURL, m_xContext, aReducedMediaDescriptor ));

    if( xStorage.is())
    {
        impl_store( aReducedMediaDescriptor, xStorage );
        attachResource( rURL, aReducedMediaDescriptor );
    }
}

void SAL_CALL ChartModel::storeToURL(
    const ::rtl::OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rMediaDescriptor )
    throw(io::IOException,
          uno::RuntimeException)
{
    apphelper::LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall(sal_True)) //start LongLastingCall
        return; //behave passive if already disposed or closed or throw exception @todo?
    //do not change the internal state of the document here
    //...

    aGuard.clear();

    apphelper::MediaDescriptorHelper aMediaDescriptorHelper(rMediaDescriptor);
    uno::Sequence< beans::PropertyValue > aReducedMediaDescriptor(
        aMediaDescriptorHelper.getReducedForModel() );

    if ( rURL == "private:stream" )
    {
        try
        {
            if( m_xContext.is() && aMediaDescriptorHelper.ISSET_OutputStream )
            {
                Reference< lang::XMultiServiceFactory > xFact( m_xContext->getServiceManager(), uno::UNO_QUERY_THROW );
                Reference< io::XStream > xStream(
                    io::TempFile::create(m_xContext), uno::UNO_QUERY_THROW );
                Reference< io::XInputStream > xInputStream( xStream->getInputStream());

                Reference< embed::XStorage > xStorage(
                    ::comphelper::OStorageHelper::GetStorageFromStream( xStream, embed::ElementModes::READWRITE, xFact ));
                if( xStorage.is())
                {
                    impl_store( aReducedMediaDescriptor, xStorage );

                    Reference< io::XSeekable > xSeekable( xStream, uno::UNO_QUERY_THROW );
                    xSeekable->seek( 0 );
                    ::comphelper::OStorageHelper::CopyInputToOutput( xInputStream, aMediaDescriptorHelper.OutputStream );
                }
            }
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    else
    {
        // create new storage
        Reference< embed::XStorage > xStorage( lcl_createStorage( rURL, m_xContext, aReducedMediaDescriptor ));

        if( xStorage.is())
            impl_store( aReducedMediaDescriptor, xStorage );
    }
}

void ChartModel::impl_store(
    const Sequence< beans::PropertyValue >& rMediaDescriptor,
    const Reference< embed::XStorage > & xStorage )
{
    Reference< document::XFilter > xFilter( impl_createFilter( rMediaDescriptor));
    if( xFilter.is() && xStorage.is())
    {
        Sequence< beans::PropertyValue > aMD( rMediaDescriptor );
        lcl_addStorageToMediaDescriptor( aMD, xStorage );
        try
        {
            Reference< document::XExporter > xExporter( xFilter, uno::UNO_QUERY_THROW );
            xExporter->setSourceDocument( Reference< lang::XComponent >( this ));
            xFilter->filter( aMD );
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    else
    {
        OSL_FAIL( "No filter" );
    }

    setModified( sal_False );

    //#i66865#
    //for data change notification during chart is not loaded:
    //notify parent data provider after saving thus the parent document can store
    //the ranges for which a load and update of the chart will be necessary
    Reference< beans::XPropertySet > xPropSet( m_xParent, uno::UNO_QUERY );
    if ( !hasInternalDataProvider() && xPropSet.is() )
    {
        apphelper::MediaDescriptorHelper aMDHelper(rMediaDescriptor);
        try
        {
            xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "SavedObject" )),
                uno::makeAny( aMDHelper.HierarchicalDocumentName ) );
        }
        catch ( const uno::Exception& )
        {
        }
    }
}

//-----------------------------------------------------------------
// frame::XLoadable
//-----------------------------------------------------------------
void SAL_CALL ChartModel::initNew()
    throw (frame::DoubleInitializationException,
           io::IOException,
           uno::Exception,
           uno::RuntimeException)
{
    lockControllers();
    createInternalDataProvider( sal_False );
    try
    {
        // create default chart
        Reference< chart2::XChartTypeTemplate > xTemplate( impl_createDefaultChartTypeTemplate() );
        if( xTemplate.is())
        {
            try
            {
                Reference< chart2::data::XDataSource > xDataSource( impl_createDefaultData() );
                Sequence< beans::PropertyValue > aParam;

                bool bSupportsCategories = xTemplate->supportsCategories();
                if( bSupportsCategories )
                {
                    aParam.realloc( 1 );
                    aParam[0] = beans::PropertyValue( C2U("HasCategories"), -1, uno::makeAny( true ),
                                                      beans::PropertyState_DIRECT_VALUE );
                }

                Reference< chart2::XDiagram > xDiagram( xTemplate->createDiagramByDataSource( xDataSource, aParam ) );

                setFirstDiagram( xDiagram );

                bool bIsRTL = Application::GetSettings().GetLayoutRTL();
                //reverse x axis for rtl charts
                if( bIsRTL )
                    AxisHelper::setRTLAxisLayout( AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 ) );

                // create and attach legend
                Reference< chart2::XLegend > xLegend(
                    m_xContext->getServiceManager()->createInstanceWithContext(
                        C2U( "com.sun.star.chart2.Legend" ), m_xContext ), uno::UNO_QUERY_THROW );
                Reference< beans::XPropertySet > xLegendProperties( xLegend, uno::UNO_QUERY );
                if( xLegendProperties.is() )
                {
                    xLegendProperties->setPropertyValue( C2U( "FillStyle" ), uno::makeAny( drawing::FillStyle_NONE ));
                    xLegendProperties->setPropertyValue( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_NONE ));
                    xLegendProperties->setPropertyValue( C2U( "LineColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xb3b3b3 ) ));  // gray30
                    xLegendProperties->setPropertyValue( C2U( "FillColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xe6e6e6 ) ) ); // gray10

                    if( bIsRTL )
                        xLegendProperties->setPropertyValue( C2U( "AnchorPosition" ), uno::makeAny( chart2::LegendPosition_LINE_START ));
                }
                if(xDiagram.is())
                    xDiagram->setLegend( xLegend );

                // set simple 3D look
                Reference< beans::XPropertySet > xDiagramProperties( xDiagram, uno::UNO_QUERY );
                if( xDiagramProperties.is() )
                {
                    xDiagramProperties->setPropertyValue( C2U("RightAngledAxes"), uno::makeAny( sal_True ));
                    xDiagramProperties->setPropertyValue( C2U("D3DScenePerspective"), uno::makeAny( drawing::ProjectionMode_PARALLEL ));
                    ThreeDHelper::setScheme( xDiagram, ThreeDLookScheme_Realistic );
                }

                //set some new 'defaults' for wall and floor
                if( xDiagram.is() )
                {
                    Reference< beans::XPropertySet > xWall( xDiagram->getWall() );
                    if( xWall.is() )
                    {
                        xWall->setPropertyValue( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_SOLID ) );
                        xWall->setPropertyValue( C2U( "FillStyle" ), uno::makeAny( drawing::FillStyle_NONE ) );
                        xWall->setPropertyValue( C2U( "LineColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xb3b3b3 ) ) ); // gray30
                        xWall->setPropertyValue( C2U( "FillColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xe6e6e6 ) ) ); // gray10
                    }
                    Reference< beans::XPropertySet > xFloor( xDiagram->getFloor() );
                    if( xFloor.is() )
                    {
                        xFloor->setPropertyValue( C2U( "LineStyle" ), uno::makeAny( drawing::LineStyle_NONE ) );
                        xFloor->setPropertyValue( C2U( "FillStyle" ), uno::makeAny( drawing::FillStyle_SOLID ) );
                        xFloor->setPropertyValue( C2U( "LineColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xb3b3b3 ) ) ); // gray30
                        xFloor->setPropertyValue( C2U( "FillColor" ), uno::makeAny( static_cast< sal_Int32 >( 0xcccccc ) ) ); // gray20
                    }

                }
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
        ChartModelHelper::setIncludeHiddenCells( false, this );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    setModified( sal_False );
    unlockControllers();

#if OSL_DEBUG_LEVEL >= CHART_TRACE_OSL_DEBUG_LEVEL
    OSL_TRACE( "ChartModel::initNew: Showing ChartDocument structure" );
    OSL_TRACE( "----------------------------------------------------" );
    ::chart::debug::ChartDebugTraceDocument( Reference< chart2::XChartDocument >( this ));
#endif
}

void SAL_CALL ChartModel::load(
    const Sequence< beans::PropertyValue >& rMediaDescriptor )
    throw (frame::DoubleInitializationException,
           io::IOException,
           uno::Exception,
           uno::RuntimeException)
{
    Reference< embed::XStorage > xStorage;
    OUString aURL;
    try
    {
        apphelper::MediaDescriptorHelper aMDHelper( rMediaDescriptor );
        if( aMDHelper.ISSET_Storage )
        {
            xStorage = aMDHelper.Storage;
        }
        else if( aMDHelper.ISSET_Stream ||
                 aMDHelper.ISSET_InputStream )
        {
            if( aMDHelper.ISSET_FilterName &&
                (aMDHelper.FilterName.equals( C2U("StarChart 5.0")) ||
                 aMDHelper.FilterName.equals( C2U("StarChart 4.0")) ||
                 aMDHelper.FilterName.equals( C2U("StarChart 3.0")) ))
            {
                attachResource( aMDHelper.URL, rMediaDescriptor );
                impl_load( rMediaDescriptor, 0 ); // cannot create a storage from binary streams, but I do not need the storage here anyhow
                m_bReadOnly = sal_True;
                return;
            }

            Reference< lang::XSingleServiceFactory > xStorageFact(
                m_xContext->getServiceManager()->createInstanceWithContext(
                    C2U("com.sun.star.embed.StorageFactory"),
                    m_xContext ),
                uno::UNO_QUERY_THROW );

            if( aMDHelper.ISSET_Stream )
            {
                // convert XStream to XStorage via the storage factory
                Sequence< uno::Any > aStorageArgs( 2 );
                aStorageArgs[0] <<= aMDHelper.Stream;
                // todo: check if stream is read-only
                aStorageArgs[1] <<= (embed::ElementModes::READ); //WRITE | embed::ElementModes::NOCREATE);

                xStorage.set( xStorageFact->createInstanceWithArguments( aStorageArgs ),
                    uno::UNO_QUERY_THROW );
            }
            else
            {
                OSL_ASSERT( aMDHelper.ISSET_InputStream );
                // convert XInputStream to XStorage via the storage factory
                Sequence< uno::Any > aStorageArgs( 2 );
                aStorageArgs[0] <<= aMDHelper.InputStream;
                aStorageArgs[1] <<= (embed::ElementModes::READ);

                xStorage.set( xStorageFact->createInstanceWithArguments( aStorageArgs ),
                    uno::UNO_QUERY_THROW );
            }
        }

        if( aMDHelper.ISSET_URL )
            aURL = aMDHelper.URL;
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    if( xStorage.is())
    {
        attachResource( aURL, rMediaDescriptor );
        impl_load( rMediaDescriptor, xStorage );
    }
}

void ChartModel::impl_load(
    const Sequence< beans::PropertyValue >& rMediaDescriptor,
    const Reference< embed::XStorage >& xStorage )
{
    {
        MutexGuard aGuard( m_aModelMutex );
        m_nInLoad++;
    }

    Reference< document::XFilter > xFilter( impl_createFilter( rMediaDescriptor ));

    if( xFilter.is())
    {
        Reference< document::XImporter > xImporter( xFilter, uno::UNO_QUERY_THROW );
        xImporter->setTargetDocument( this );
        Sequence< beans::PropertyValue > aMD( rMediaDescriptor );
        lcl_addStorageToMediaDescriptor( aMD, xStorage );

        xFilter->filter( aMD );
        xFilter.clear();
    }
    else
    {
        OSL_FAIL( "loadFromStorage cannot create filter" );
    }

    if( xStorage.is() )
        impl_loadGraphics( xStorage );

    setModified( sal_False );

    // switchToStorage without notifying listeners (which shouldn't exist at
    // this time, anyway)
    m_xStorage = xStorage;

    {
        MutexGuard aGuard( m_aModelMutex );
        m_nInLoad--;
    }
}

void ChartModel::impl_loadGraphics(
    const Reference< embed::XStorage >& xStorage )
{
    try
    {
        const Reference< embed::XStorage >& xGraphicsStorage(
            xStorage->openStorageElement( C2U( "Pictures" ),
                                          embed::ElementModes::READ ) );

        if( xGraphicsStorage.is() )
        {
            const uno::Sequence< ::rtl::OUString > aElementNames(
                xGraphicsStorage->getElementNames() );

            for( int i = 0; i < aElementNames.getLength(); ++i )
            {
                if( xGraphicsStorage->isStreamElement( aElementNames[ i ] ) )
                {
                    uno::Reference< io::XStream > xElementStream(
                        xGraphicsStorage->openStreamElement(
                            aElementNames[ i ],
                            embed::ElementModes::READ ) );

                    if( xElementStream.is() )
                    {
                        std::auto_ptr< SvStream > apIStm(
                            ::utl::UcbStreamHelper::CreateStream(
                                xElementStream, true ) );

                        if( apIStm.get() )
                        {
                            Graphic aGraphic;

                            if( !GraphicConverter::Import(
                                    *apIStm.get(),
                                    aGraphic ) )
                            {
                                m_aGraphicObjectVector.push_back( aGraphic );
                            }
                        }
                    }
                }
            }
        }
    }
    catch ( const uno::Exception& )
    {
    }
}

//-----------------------------------------------------------------
// util::XModifiable
//-----------------------------------------------------------------
void SAL_CALL ChartModel::impl_notifyModifiedListeners()
    throw( uno::RuntimeException)
{
    {
        MutexGuard aGuard( m_aModelMutex );
        m_bUpdateNotificationsPending = false;
    }

    //always notify the view first!
    ChartViewHelper::setViewToDirtyState( this );

    ::cppu::OInterfaceContainerHelper* pIC = m_aLifeTimeManager.m_aListenerContainer
        .getContainer( ::getCppuType((const uno::Reference< util::XModifyListener >*)0) );
    if( pIC )
    {
        lang::EventObject aEvent( static_cast< lang::XComponent*>(this) );
        ::cppu::OInterfaceIteratorHelper aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            uno::Reference< util::XModifyListener > xListener( aIt.next(), uno::UNO_QUERY );
            if( xListener.is() )
                xListener->modified( aEvent );
        }
    }
}

sal_Bool SAL_CALL ChartModel::isModified()
    throw(uno::RuntimeException)
{
    //@todo guard
    return m_bModified;
}

void SAL_CALL ChartModel::setModified( sal_Bool bModified )
    throw(beans::PropertyVetoException,
          uno::RuntimeException)
{
    apphelper::LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())//@todo ? is this a long lasting call??
        return; //behave passive if already disposed or closed or throw exception @todo?
    m_bModified = bModified;

    if( m_nControllerLockCount > 0 )
    {
        m_bUpdateNotificationsPending = true;
        return;//don't call listeners if controllers are locked
    }
    aGuard.clear();

    if(bModified)
        impl_notifyModifiedListeners();
}

//-----------------------------------------------------------------
// util::XModifyBroadcaster (base of XModifiable)
//-----------------------------------------------------------------
void SAL_CALL ChartModel::addModifyListener(
    const uno::Reference< util::XModifyListener >& xListener )
    throw(uno::RuntimeException)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed() )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.addInterface(
        ::getCppuType((const uno::Reference< util::XModifyListener >*)0), xListener );
}

void SAL_CALL ChartModel::removeModifyListener(
    const uno::Reference< util::XModifyListener >& xListener )
    throw(uno::RuntimeException)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed(false) )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.removeInterface(
        ::getCppuType((const uno::Reference< util::XModifyListener >*)0), xListener );
}

//-----------------------------------------------------------------
// util::XModifyListener
//-----------------------------------------------------------------
void SAL_CALL ChartModel::modified( const lang::EventObject& )
    throw (uno::RuntimeException)
{
    if( m_nInLoad == 0 )
        setModified( sal_True );
}

//-----------------------------------------------------------------
// lang::XEventListener (base of util::XModifyListener)
//-----------------------------------------------------------------
void SAL_CALL ChartModel::disposing( const lang::EventObject& )
    throw (uno::RuntimeException)
{
    // child was disposed -- should not happen from outside
}


//-----------------------------------------------------------------
// document::XStorageBasedDocument
//-----------------------------------------------------------------
void SAL_CALL ChartModel::loadFromStorage(
    const Reference< embed::XStorage >& xStorage,
    const Sequence< beans::PropertyValue >& rMediaDescriptor )
    throw (lang::IllegalArgumentException,
           frame::DoubleInitializationException,
           io::IOException,
           uno::Exception,
           uno::RuntimeException)
{
    attachResource( OUString(), rMediaDescriptor );
    impl_load( rMediaDescriptor, xStorage );
}

void SAL_CALL ChartModel::storeToStorage(
    const Reference< embed::XStorage >& xStorage,
    const Sequence< beans::PropertyValue >& rMediaDescriptor )
    throw (lang::IllegalArgumentException,
           io::IOException,
           uno::Exception,
           uno::RuntimeException)
{
    impl_store( rMediaDescriptor, xStorage );
}

void SAL_CALL ChartModel::switchToStorage( const Reference< embed::XStorage >& xStorage )
    throw (lang::IllegalArgumentException,
           io::IOException,
           uno::Exception,
           uno::RuntimeException)
{
    m_xStorage = xStorage;
    impl_notifyStorageChangeListeners();
}

Reference< embed::XStorage > SAL_CALL ChartModel::getDocumentStorage()
    throw (io::IOException,
           uno::Exception,
           uno::RuntimeException)
{
    return m_xStorage;
}

void SAL_CALL ChartModel::impl_notifyStorageChangeListeners()
    throw( uno::RuntimeException)
{
    ::cppu::OInterfaceContainerHelper* pIC = m_aLifeTimeManager.m_aListenerContainer
          .getContainer( ::getCppuType((const uno::Reference< document::XStorageChangeListener >*)0) );
    if( pIC )
    {
        ::cppu::OInterfaceIteratorHelper aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            uno::Reference< document::XStorageChangeListener > xListener( aIt.next(), uno::UNO_QUERY );
            if( xListener.is() )
                xListener->notifyStorageChange( static_cast< ::cppu::OWeakObject* >( this ), m_xStorage );
        }
    }
}

void SAL_CALL ChartModel::addStorageChangeListener( const Reference< document::XStorageChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed() )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.addInterface(
        ::getCppuType((const uno::Reference< document::XStorageChangeListener >*)0), xListener );
}

void SAL_CALL ChartModel::removeStorageChangeListener( const Reference< document::XStorageChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed(false) )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.removeInterface(
        ::getCppuType((const uno::Reference< document::XStorageChangeListener >*)0), xListener );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
