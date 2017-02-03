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
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>

#include <ucbhelper/content.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/cvtgrf.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <algorithm>
#include <functional>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::osl::MutexGuard;

namespace
{
struct lcl_PropNameEquals : public ::std::unary_function< beans::PropertyValue, bool >
{
    explicit lcl_PropNameEquals( const OUString & rStrToCompareWith ) :
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
        const beans::PropertyValue * pIt = rMediaDescriptor.getConstArray();
        const beans::PropertyValue * pEndIt = pIt +  + rMediaDescriptor.getLength();
        pIt = ::std::find_if( pIt, pEndIt, lcl_PropNameEquals( rPropName ));
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
        "Storage", -1, uno::Any( xStorage ), beans::PropertyState_DIRECT_VALUE );
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
            ::ucbhelper::Content( rURL, Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext()).openStream(),
            uno::UNO_QUERY );

        Reference< lang::XSingleServiceFactory > xStorageFact( embed::StorageFactory::create( xContext ) );
        Sequence< uno::Any > aStorageArgs( 3 );
        aStorageArgs[0] <<= xStream;
        aStorageArgs[1] <<= embed::ElementModes::READWRITE;
        aStorageArgs[2] <<= rMediaDescriptor;
        xStorage.set(
            xStorageFact->createInstanceWithArguments( aStorageArgs ), uno::UNO_QUERY_THROW );
        OSL_ENSURE( xStorage.is(), "No Storage" );
    }
    catch(const css::ucb::ContentCreationException& rEx)
    {
        ASSERT_EXCEPTION( rEx );
    }
    catch(const css::ucb::CommandFailedException& rEx)
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
        lcl_getProperty< OUString >( rMediaDescriptor, "FilterName" ) );

    // if FilterName was found, get Filter from factory
    if( !aFilterName.isEmpty() )
    {
        try
        {
            Reference< container::XNameAccess > xFilterFact(
                m_xContext->getServiceManager()->createInstanceWithContext(
                    "com.sun.star.document.FilterFactory", m_xContext ),
                uno::UNO_QUERY_THROW );
            uno::Any aFilterProps( xFilterFact->getByName( aFilterName ));
            Sequence< beans::PropertyValue > aProps;

            if( aFilterProps.hasValue() &&
                (aFilterProps >>= aProps))
            {
                OUString aFilterServiceName(
                    lcl_getProperty< OUString >( aProps, "FilterService" ) );

                if( !aFilterServiceName.isEmpty())
                {
                    xFilter.set(
                        m_xContext->getServiceManager()->createInstanceWithContext(
                            aFilterServiceName, m_xContext ), uno::UNO_QUERY_THROW );
                    SAL_INFO("chart2", "Filter found for service " << aFilterServiceName );
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
        SAL_WARN("chart2", "No FilterName passed in MediaDescriptor" );
        xFilter.set(
            m_xContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.comp.chart2.XMLFilter", m_xContext ),
            uno::UNO_QUERY_THROW );
    }

    return xFilter;
}

// frame::XStorable2

void SAL_CALL ChartModel::storeSelf( const Sequence< beans::PropertyValue >& rMediaDescriptor )
{
    // only some parameters are allowed (see also SfxBaseModel)
    // "VersionComment", "Author", "InteractionHandler", "StatusIndicator"
    // However, they are ignored here.  They would become interesting when
    // charts support a standalone format again.
    impl_store( rMediaDescriptor, m_xStorage );
}

// frame::XStorable (base of XStorable2)
sal_Bool SAL_CALL ChartModel::hasLocation()
{
    //@todo guard
    return !m_aResource.isEmpty();
}

OUString SAL_CALL ChartModel::getLocation()
{
    return impl_g_getLocation();
}

sal_Bool SAL_CALL ChartModel::isReadonly()
{
    //@todo guard
    return m_bReadOnly;
}

void SAL_CALL ChartModel::store()
{
    apphelper::LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall(true)) //start LongLastingCall
        return; //behave passive if already disposed or closed or throw exception @todo?

    OUString aLocation = m_aResource;

    if( aLocation.isEmpty() )
        throw io::IOException( "no location specified", static_cast< ::cppu::OWeakObject* >(this));
    //@todo check whether aLocation is something like private:factory...
    if( m_bReadOnly )
        throw io::IOException( "document is read only", static_cast< ::cppu::OWeakObject* >(this));

    aGuard.clear();

    // store
    impl_store( m_aMediaDescriptor, m_xStorage );
}

void SAL_CALL ChartModel::storeAsURL(
    const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rMediaDescriptor )
{
    apphelper::LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall(true)) //start LongLastingCall
        return; //behave passive if already disposed or closed or throw exception @todo?

    apphelper::MediaDescriptorHelper aMediaDescriptorHelper(rMediaDescriptor);
    uno::Sequence< beans::PropertyValue > aReducedMediaDescriptor(
        aMediaDescriptorHelper.getReducedForModel() );

    m_bReadOnly = false;
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
    const OUString& rURL,
    const uno::Sequence< beans::PropertyValue >& rMediaDescriptor )
{
    apphelper::LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall(true)) //start LongLastingCall
        return; //behave passive if already disposed or closed or throw exception @todo?
    //do not change the internal state of the document here

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
                Reference< io::XStream > xStream(
                    io::TempFile::create(m_xContext), uno::UNO_QUERY_THROW );
                Reference< io::XInputStream > xInputStream( xStream->getInputStream());

                Reference< embed::XStorage > xStorage(
                    ::comphelper::OStorageHelper::GetStorageFromStream( xStream, embed::ElementModes::READWRITE, m_xContext ));
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

    setModified( false );

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
            xPropSet->setPropertyValue(
                "SavedObject",
                uno::Any( aMDHelper.HierarchicalDocumentName ) );
        }
        catch ( const uno::Exception& )
        {
        }
    }
}

void ChartModel::insertDefaultChart()
{
    lockControllers();
    createInternalDataProvider( false );
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
                    aParam[0] = beans::PropertyValue( "HasCategories", -1, uno::Any( true ),
                                                      beans::PropertyState_DIRECT_VALUE );
                }

                Reference< chart2::XDiagram > xDiagram( xTemplate->createDiagramByDataSource( xDataSource, aParam ) );

                setFirstDiagram( xDiagram );

                bool bIsRTL = AllSettings::GetMathLayoutRTL();
                //reverse x axis for rtl charts
                if( bIsRTL )
                    AxisHelper::setRTLAxisLayout( AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 ) );

                // create and attach legend
                Reference< chart2::XLegend > xLegend(
                    m_xContext->getServiceManager()->createInstanceWithContext(
                        "com.sun.star.chart2.Legend", m_xContext ), uno::UNO_QUERY_THROW );
                Reference< beans::XPropertySet > xLegendProperties( xLegend, uno::UNO_QUERY );
                if( xLegendProperties.is() )
                {
                    xLegendProperties->setPropertyValue( "FillStyle", uno::Any( drawing::FillStyle_NONE ));
                    xLegendProperties->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_NONE ));
                    xLegendProperties->setPropertyValue( "LineColor", uno::Any( static_cast< sal_Int32 >( 0xb3b3b3 ) ));  // gray30
                    xLegendProperties->setPropertyValue( "FillColor", uno::Any( static_cast< sal_Int32 >( 0xe6e6e6 ) ) ); // gray10

                    if( bIsRTL )
                        xLegendProperties->setPropertyValue( "AnchorPosition", uno::Any( chart2::LegendPosition_LINE_START ));
                }
                if(xDiagram.is())
                    xDiagram->setLegend( xLegend );

                // set simple 3D look
                Reference< beans::XPropertySet > xDiagramProperties( xDiagram, uno::UNO_QUERY );
                if( xDiagramProperties.is() )
                {
                    xDiagramProperties->setPropertyValue( "RightAngledAxes", uno::Any( true ));
                    xDiagramProperties->setPropertyValue( "D3DScenePerspective", uno::Any( drawing::ProjectionMode_PARALLEL ));
                    ThreeDHelper::setScheme( xDiagram, ThreeDLookScheme_Realistic );
                }

                //set some new 'defaults' for wall and floor
                if( xDiagram.is() )
                {
                    Reference< beans::XPropertySet > xWall( xDiagram->getWall() );
                    if( xWall.is() )
                    {
                        xWall->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_SOLID ) );
                        xWall->setPropertyValue( "FillStyle", uno::Any( drawing::FillStyle_NONE ) );
                        xWall->setPropertyValue( "LineColor", uno::Any( static_cast< sal_Int32 >( 0xb3b3b3 ) ) ); // gray30
                        xWall->setPropertyValue( "FillColor", uno::Any( static_cast< sal_Int32 >( 0xe6e6e6 ) ) ); // gray10
                    }
                    Reference< beans::XPropertySet > xFloor( xDiagram->getFloor() );
                    if( xFloor.is() )
                    {
                        xFloor->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_NONE ) );
                        xFloor->setPropertyValue( "FillStyle", uno::Any( drawing::FillStyle_SOLID ) );
                        xFloor->setPropertyValue( "LineColor", uno::Any( static_cast< sal_Int32 >( 0xb3b3b3 ) ) ); // gray30
                        xFloor->setPropertyValue( "FillColor", uno::Any( static_cast< sal_Int32 >( 0xcccccc ) ) ); // gray20
                    }

                }
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
        ChartModelHelper::setIncludeHiddenCells( false, *this );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    setModified( false );
    unlockControllers();
}

// frame::XLoadable
void SAL_CALL ChartModel::initNew()
{
}

void SAL_CALL ChartModel::load(
    const Sequence< beans::PropertyValue >& rMediaDescriptor )
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
                (aMDHelper.FilterName == "StarChart 5.0" ||
                 aMDHelper.FilterName == "StarChart 4.0" ||
                 aMDHelper.FilterName == "StarChart 3.0" ))
            {
                attachResource( aMDHelper.URL, rMediaDescriptor );
                impl_load( rMediaDescriptor, nullptr ); // cannot create a storage from binary streams, but I do not need the storage here anyhow
                m_bReadOnly = true;
                return;
            }

            Reference< lang::XSingleServiceFactory > xStorageFact( embed::StorageFactory::create(m_xContext) );

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

    setModified( false );

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
            xStorage->openStorageElement( "Pictures",
                                          embed::ElementModes::READ ) );

        if( xGraphicsStorage.is() )
        {
            const uno::Sequence< OUString > aElementNames(
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
                        std::unique_ptr< SvStream > apIStm(
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

// util::XModifiable
void SAL_CALL ChartModel::impl_notifyModifiedListeners()
{
    {
        MutexGuard aGuard( m_aModelMutex );
        m_bUpdateNotificationsPending = false;
    }

    //always notify the view first!
    ChartViewHelper::setViewToDirtyState( this );

    ::cppu::OInterfaceContainerHelper* pIC = m_aLifeTimeManager.m_aListenerContainer
        .getContainer( cppu::UnoType<util::XModifyListener>::get());
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
{
    //@todo guard
    return m_bModified;
}

void SAL_CALL ChartModel::setModified( sal_Bool bModified )
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

// util::XModifyBroadcaster (base of XModifiable)
void SAL_CALL ChartModel::addModifyListener(
    const uno::Reference< util::XModifyListener >& xListener )
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed() )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.addInterface(
        cppu::UnoType<util::XModifyListener>::get(), xListener );
}

void SAL_CALL ChartModel::removeModifyListener(
    const uno::Reference< util::XModifyListener >& xListener )
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed(false) )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.removeInterface(
        cppu::UnoType<util::XModifyListener>::get(), xListener );
}

// util::XModifyListener
void SAL_CALL ChartModel::modified( const lang::EventObject& )
{
    if( m_nInLoad == 0 )
        setModified( true );
}

// lang::XEventListener (base of util::XModifyListener)
void SAL_CALL ChartModel::disposing( const lang::EventObject& )
{
    // child was disposed -- should not happen from outside
}

// document::XStorageBasedDocument
void SAL_CALL ChartModel::loadFromStorage(
    const Reference< embed::XStorage >& xStorage,
    const Sequence< beans::PropertyValue >& rMediaDescriptor )
{
    attachResource( OUString(), rMediaDescriptor );
    impl_load( rMediaDescriptor, xStorage );
}

void SAL_CALL ChartModel::storeToStorage(
    const Reference< embed::XStorage >& xStorage,
    const Sequence< beans::PropertyValue >& rMediaDescriptor )
{
    impl_store( rMediaDescriptor, xStorage );
}

void SAL_CALL ChartModel::switchToStorage( const Reference< embed::XStorage >& xStorage )
{
    m_xStorage = xStorage;
    impl_notifyStorageChangeListeners();
}

Reference< embed::XStorage > SAL_CALL ChartModel::getDocumentStorage()
{
    return m_xStorage;
}

void SAL_CALL ChartModel::impl_notifyStorageChangeListeners()
{
    ::cppu::OInterfaceContainerHelper* pIC = m_aLifeTimeManager.m_aListenerContainer
          .getContainer( cppu::UnoType<document::XStorageChangeListener>::get());
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
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed() )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.addInterface(
        cppu::UnoType<document::XStorageChangeListener>::get(), xListener );
}

void SAL_CALL ChartModel::removeStorageChangeListener( const Reference< document::XStorageChangeListener >& xListener )
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed(false) )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.removeInterface(
        cppu::UnoType<document::XStorageChangeListener>::get(), xListener );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
