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
#include "servicenames.hxx"
#include "MediaDescriptorHelper.hxx"
#include "macros.hxx"
#include "DataSourceHelper.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "DisposeHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "ObjectIdentifier.hxx"
#include "PageBackground.hxx"
#include "CloneHelper.hxx"
#include "NameContainer.hxx"
#include "UndoManager.hxx"
#include "ChartView.hxx"
#include <svx/charthelper.hxx>

#include <vcl/openglwin.hxx>

#include <com/sun/star/chart/ChartDataRowSource.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <svl/numuno.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/chart2/XTimeBased.hpp>

#include <svl/zforlist.hxx>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

using namespace ::com::sun::star;
using namespace ::apphelper;
using namespace ::chart::CloneHelper;

namespace
{
const OUString lcl_aGDIMetaFileMIMEType(
    "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"");
const OUString lcl_aGDIMetaFileMIMETypeHighContrast(
    "application/x-openoffice-highcontrast-gdimetafile;windows_formatname=\"GDIMetaFile\"");

} // anonymous namespace

// ChartModel Constructor and Destructor

namespace chart
{

ChartModel::ChartModel(uno::Reference<uno::XComponentContext > const & xContext)
    : m_aLifeTimeManager( this, this )
    , m_bReadOnly( false )
    , m_bModified( false )
    , m_nInLoad(0)
    , m_bUpdateNotificationsPending(false)
    , mbTimeBased(false)
    , mpChartView(nullptr)
    , m_pUndoManager( nullptr )
    , m_aControllers( m_aModelMutex )
    , m_nControllerLockCount(0)
    , m_xContext( xContext )
    , m_aVisualAreaSize( ChartModelHelper::getDefaultPageSize() )
    , m_xDataProvider( nullptr )
    , m_xInternalDataProvider( nullptr )
    , m_xPageBackground( new PageBackground( m_xContext ) )
    , m_xXMLNamespaceMap( createNameContainer( ::cppu::UnoType<OUString>::get(),
                "com.sun.star.xml.NamespaceMap", "com.sun.star.comp.chart.XMLNameSpaceMap" ), uno::UNO_QUERY)
    , mnStart(0)
    , mnEnd(0)
    ,bSet(false)
    , mpOpenGLWindow(nullptr)
{
    OSL_TRACE( "ChartModel: CTOR called" );

    osl_atomic_increment(&m_refCount);
    {
        m_xOldModelAgg.set(
            m_xContext->getServiceManager()->createInstanceWithContext(
            CHART_CHARTAPIWRAPPER_SERVICE_NAME,
            m_xContext ), uno::UNO_QUERY_THROW );
        m_xOldModelAgg->setDelegator( *this );
    }

    {
        ModifyListenerHelper::addListener( m_xPageBackground, this );
        m_xChartTypeManager.set( xContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.chart2.ChartTypeManager", m_xContext ), uno::UNO_QUERY );
    }
    osl_atomic_decrement(&m_refCount);
}

ChartModel::ChartModel( const ChartModel & rOther )
    : impl::ChartModel_Base()
    , m_aLifeTimeManager( this, this )
    , m_bReadOnly( rOther.m_bReadOnly )
    , m_bModified( rOther.m_bModified )
    , m_nInLoad(0)
    , m_bUpdateNotificationsPending(false)
    , mbTimeBased(rOther.mbTimeBased)
    , mpChartView(nullptr)
    , m_aResource( rOther.m_aResource )
    , m_aMediaDescriptor( rOther.m_aMediaDescriptor )
    , m_aControllers( m_aModelMutex )
    , m_nControllerLockCount(0)
    , m_xContext( rOther.m_xContext )
    // @note: the old model aggregate must not be shared with other models if it
    // is, you get mutex deadlocks
    , m_xOldModelAgg( nullptr ) //rOther.m_xOldModelAgg )
    , m_xStorage( nullptr ) //rOther.m_xStorage )
    , m_aVisualAreaSize( rOther.m_aVisualAreaSize )
    , m_aGraphicObjectVector( rOther.m_aGraphicObjectVector )
    , m_xDataProvider( rOther.m_xDataProvider )
    , m_xInternalDataProvider( rOther.m_xInternalDataProvider )
    , mnStart(rOther.mnStart)
    , mnEnd(rOther.mnEnd)
    , bSet(false)
    , mpOpenGLWindow(nullptr)
{
    OSL_TRACE( "ChartModel: Copy-CTOR called" );

    osl_atomic_increment(&m_refCount);
    {
        m_xOldModelAgg.set(
            m_xContext->getServiceManager()->createInstanceWithContext(
            CHART_CHARTAPIWRAPPER_SERVICE_NAME,
            m_xContext ), uno::UNO_QUERY_THROW );
        m_xOldModelAgg->setDelegator( *this );

        Reference< util::XModifyListener > xListener;
        Reference< chart2::XTitle > xNewTitle = CreateRefClone< chart2::XTitle >()( rOther.m_xTitle );
        Reference< chart2::XDiagram > xNewDiagram = CreateRefClone< chart2::XDiagram >()( rOther.m_xDiagram );
        Reference< beans::XPropertySet > xNewPageBackground = CreateRefClone< beans::XPropertySet >()( rOther.m_xPageBackground );
        Reference< chart2::XChartTypeManager > xChartTypeManager = CreateRefClone< chart2::XChartTypeManager >()( rOther.m_xChartTypeManager );
        Reference< container::XNameAccess > xXMLNamespaceMap = CreateRefClone< container::XNameAccess >()( rOther.m_xXMLNamespaceMap );

        {
            MutexGuard aGuard( m_aModelMutex );
            xListener = this;
            m_xTitle = xNewTitle;
            m_xDiagram = xNewDiagram;
            m_xPageBackground = xNewPageBackground;
            m_xChartTypeManager = xChartTypeManager;
            m_xXMLNamespaceMap = xXMLNamespaceMap;
        }

        ModifyListenerHelper::addListener( xNewTitle, xListener );
        ModifyListenerHelper::addListener( xNewDiagram, xListener );
        ModifyListenerHelper::addListener( xNewPageBackground, xListener );
        xListener.clear();
    }
    osl_atomic_decrement(&m_refCount);
}

ChartModel::~ChartModel()
{
    OSL_TRACE( "ChartModel: DTOR called" );
    if( m_xOldModelAgg.is())
        m_xOldModelAgg->setDelegator( nullptr );
}

void SAL_CALL ChartModel::initialize( const Sequence< Any >& /*rArguments*/ )
                throw (uno::Exception, uno::RuntimeException, std::exception)
{
    //#i113722# avoid duplicate creation

    //maybe additional todo?:
    //support argument "EmbeddedObject"?
    //support argument "EmbeddedScriptSupport"?
    //support argument "DocumentRecoverySupport"?
}

// private methods

OUString ChartModel::impl_g_getLocation()
{

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return OUString(); //behave passive if already disposed or closed or throw exception @todo?
    //mutex is acquired
    return m_aResource;
}

bool ChartModel::impl_isControllerConnected( const uno::Reference< frame::XController >& xController )
{
    try
    {
        std::vector< uno::Reference<uno::XInterface> > aSeq = m_aControllers.getElements();
        for( const auto & r : aSeq )
        {
            if( r == xController )
                return true;
        }
    }
    catch (const uno::Exception&)
    {
    }
    return false;
}

uno::Reference< frame::XController > ChartModel::impl_getCurrentController() throw(uno::RuntimeException)
{
        //@todo? hold only weak references to controllers

    // get the last active controller of this model
    if( m_xCurrentController.is() )
        return m_xCurrentController;

    // get the first controller of this model
    if( m_aControllers.getLength() )
    {
        uno::Reference<uno::XInterface> xI = m_aControllers.getElements()[0];
        return uno::Reference<frame::XController>( xI, uno::UNO_QUERY );
    }

    //return nothing if no controllers are connected at all
    return uno::Reference< frame::XController > ();
}

void SAL_CALL ChartModel::impl_notifyCloseListeners()
        throw( uno::RuntimeException)
{
    ::cppu::OInterfaceContainerHelper* pIC = m_aLifeTimeManager.m_aListenerContainer
        .getContainer( cppu::UnoType<util::XCloseListener>::get());
    if( pIC )
    {
        lang::EventObject aEvent( static_cast< lang::XComponent*>(this) );
        ::cppu::OInterfaceIteratorHelper aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            uno::Reference< util::XCloseListener > xListener( aIt.next(), uno::UNO_QUERY );
            if( xListener.is() )
                xListener->notifyClosing( aEvent );
        }
    }
}

void ChartModel::impl_adjustAdditionalShapesPositionAndSize( const awt::Size& aVisualAreaSize )
{
    uno::Reference< beans::XPropertySet > xProperties( static_cast< ::cppu::OWeakObject* >( this ), uno::UNO_QUERY );
    if ( xProperties.is() )
    {
        uno::Reference< drawing::XShapes > xShapes;
        xProperties->getPropertyValue( "AdditionalShapes" ) >>= xShapes;
        if ( xShapes.is() )
        {
            sal_Int32 nCount = xShapes->getCount();
            for ( sal_Int32 i = 0; i < nCount; ++i )
            {
                Reference< drawing::XShape > xShape;
                if ( xShapes->getByIndex( i ) >>= xShape )
                {
                    if ( xShape.is() )
                    {
                        awt::Point aPos( xShape->getPosition() );
                        awt::Size aSize( xShape->getSize() );

                        double fWidth = static_cast< double >( aVisualAreaSize.Width ) / m_aVisualAreaSize.Width;
                        double fHeight = static_cast< double >( aVisualAreaSize.Height ) / m_aVisualAreaSize.Height;

                        aPos.X = static_cast< long >( aPos.X * fWidth );
                        aPos.Y = static_cast< long >( aPos.Y * fHeight );
                        aSize.Width = static_cast< long >( aSize.Width * fWidth );
                        aSize.Height = static_cast< long >( aSize.Height * fHeight );

                        xShape->setPosition( aPos );
                        xShape->setSize( aSize );
                    }
                }
            }
        }
    }
}

// lang::XServiceInfo

OUString SAL_CALL ChartModel::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString ChartModel::getImplementationName_Static()
{
    return OUString(CHART_MODEL_SERVICE_IMPLEMENTATION_NAME);
}

sal_Bool SAL_CALL ChartModel::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ChartModel::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > ChartModel::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSNS( 3 );
    aSNS[0] = CHART_MODEL_SERVICE_NAME;
    aSNS[1] = "com.sun.star.document.OfficeDocument";
    aSNS[2] = "com.sun.star.chart.ChartDocument";
    //// @todo : add additional services if you support any further
    return aSNS;
}

// frame::XModel (required interface)

sal_Bool SAL_CALL ChartModel::attachResource( const OUString& rURL
        , const uno::Sequence< beans::PropertyValue >& rMediaDescriptor )
        throw(uno::RuntimeException, std::exception)
{
    /*
    The method attachResource() is used by the frame loader implementations
    to inform the model about its URL and MediaDescriptor.
    */

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return false; //behave passive if already disposed or closed or throw exception @todo?
    //mutex is acquired

    if(!m_aResource.isEmpty())//we have a resource already //@todo? or is setting a new resource allowed?
        return false;
    m_aResource = rURL;
    m_aMediaDescriptor = rMediaDescriptor;

    //@todo ? check rURL ??
    //@todo ? evaluate m_aMediaDescriptor;
    //@todo ? ... ??? --> nothing, this method is only for setting information

    return true;
}

OUString SAL_CALL ChartModel::getURL() throw(uno::RuntimeException, std::exception)
{
    return impl_g_getLocation();
}

uno::Sequence< beans::PropertyValue > SAL_CALL ChartModel::getArgs() throw(uno::RuntimeException, std::exception)
{
    /*
    The method getArgs() returns a sequence of property values
    that report the resource description according to com.sun.star.document.MediaDescriptor,
    specified on loading or saving with storeAsURL.
    */

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return uno::Sequence< beans::PropertyValue >(); //behave passive if already disposed or closed or throw exception @todo?
    //mutex is acquired

    return m_aMediaDescriptor;
}

void SAL_CALL ChartModel::connectController( const uno::Reference< frame::XController >& xController )
        throw(uno::RuntimeException, std::exception)
{
    //@todo? this method is declared as oneway -> ...?

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return ; //behave passive if already disposed or closed
    //mutex is acquired

    //--add controller
    m_aControllers.addInterface(xController);
}

void SAL_CALL ChartModel::disconnectController( const uno::Reference< frame::XController >& xController )
        throw(uno::RuntimeException, std::exception)
{
    //@todo? this method is declared as oneway -> ...?

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return; //behave passive if already disposed or closed

    //--remove controller
    m_aControllers.removeInterface(xController);

    //case: current controller is disconnected:
    if( m_xCurrentController == xController )
        m_xCurrentController.clear();

    DisposeHelper::DisposeAndClear( m_xRangeHighlighter );
}

void SAL_CALL ChartModel::lockControllers() throw(uno::RuntimeException, std::exception)
{
    /*
    suspends some notifications to the controllers which are used for display updates.

    The calls to lockControllers() and unlockControllers() may be nested
    and even overlapping, but they must be in pairs. While there is at least one lock
    remaining, some notifications for display updates are not broadcasted.
    */

    //@todo? this method is declared as oneway -> ...?

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return; //behave passive if already disposed or closed or throw exception @todo?
    ++m_nControllerLockCount;
}

void SAL_CALL ChartModel::unlockControllers() throw(uno::RuntimeException, std::exception)
{
    /*
    resumes the notifications which were suspended by lockControllers() .

    The calls to lockControllers() and unlockControllers() may be nested
    and even overlapping, but they must be in pairs. While there is at least one lock
    remaining, some notifications for display updates are not broadcasted.
    */

    //@todo? this method is declared as oneway -> ...?

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return; //behave passive if already disposed or closed or throw exception @todo?
    if( m_nControllerLockCount == 0 )
    {
        OSL_TRACE( "ChartModel: unlockControllers called with m_nControllerLockCount == 0" );
        return;
    }
    --m_nControllerLockCount;
    if( m_nControllerLockCount == 0 && m_bUpdateNotificationsPending  )
    {
        aGuard.clear();
        impl_notifyModifiedListeners();
    }
}

sal_Bool SAL_CALL ChartModel::hasControllersLocked() throw(uno::RuntimeException, std::exception)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return false; //behave passive if already disposed or closed or throw exception @todo?
    return ( m_nControllerLockCount != 0 ) ;
}

uno::Reference< frame::XController > SAL_CALL ChartModel::getCurrentController() throw(uno::RuntimeException, std::exception)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        throw lang::DisposedException(
                "getCurrentController was called on an already disposed or closed model",
                static_cast< ::cppu::OWeakObject* >(this) );

    return impl_getCurrentController();
}

void SAL_CALL ChartModel::setCurrentController( const uno::Reference< frame::XController >& xController )
        throw(container::NoSuchElementException, uno::RuntimeException, std::exception)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        throw lang::DisposedException(
                "setCurrentController was called on an already disposed or closed model",
                static_cast< ::cppu::OWeakObject* >(this) );

    //OSL_ENSURE( impl_isControllerConnected(xController), "setCurrentController is called with a Controller which is not connected" );
    if(!impl_isControllerConnected(xController))
        throw container::NoSuchElementException(
                "setCurrentController is called with a Controller which is not connected",
                static_cast< ::cppu::OWeakObject* >(this) );

    m_xCurrentController = xController;

    DisposeHelper::DisposeAndClear( m_xRangeHighlighter );
}

uno::Reference< uno::XInterface > SAL_CALL ChartModel::getCurrentSelection() throw(uno::RuntimeException, std::exception)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        throw lang::DisposedException(
                "getCurrentSelection was called on an already disposed or closed model",
                static_cast< ::cppu::OWeakObject* >(this) );

    uno::Reference< uno::XInterface > xReturn;
    uno::Reference< frame::XController > xController = impl_getCurrentController();

    aGuard.clear();
    if( xController.is() )
    {
        uno::Reference< view::XSelectionSupplier >  xSelectionSupl( xController, uno::UNO_QUERY );
        if ( xSelectionSupl.is() )
        {
            uno::Any aSel = xSelectionSupl->getSelection();
            OUString aObjectCID;
            if( aSel >>= aObjectCID )
                xReturn.set( ObjectIdentifier::getObjectPropertySet( aObjectCID, Reference< XChartDocument >(this)));
        }
    }
    return xReturn;
}

// lang::XComponent (base of XModel)
void SAL_CALL ChartModel::dispose() throw(uno::RuntimeException, std::exception)
{
    Reference< XInterface > xKeepAlive( *this );

    //This object should release all resources and references in the
    //easiest possible manner
    //This object must notify all registered listeners using the method
    //<member>XEventListener::disposing</member>

    //hold no mutex
    if( !m_aLifeTimeManager.dispose() )
        return;

    //--release all resources and references
    //// @todo

    if ( m_xDiagram.is() )
        ModifyListenerHelper::removeListener( m_xDiagram, this );

    m_xDataProvider.clear();
    m_xInternalDataProvider.clear();
    m_xNumberFormatsSupplier.clear();
    DisposeHelper::DisposeAndClear( m_xOwnNumberFormatsSupplier );
    DisposeHelper::DisposeAndClear( m_xChartTypeManager );
    DisposeHelper::DisposeAndClear( m_xDiagram );
    DisposeHelper::DisposeAndClear( m_xTitle );
    DisposeHelper::DisposeAndClear( m_xPageBackground );
    DisposeHelper::DisposeAndClear( m_xXMLNamespaceMap );

    m_xStorage.clear();
        // just clear, don't dispose - we're not the owner

    if ( m_pUndoManager.is() )
        m_pUndoManager->disposing();
    m_pUndoManager.clear();
        // that's important, since the UndoManager implementation delegates its ref counting to ourself.

    if( m_xOldModelAgg.is())  // #i120828#, to release cyclic reference to ChartModel object
        m_xOldModelAgg->setDelegator( nullptr );

    m_aControllers.disposeAndClear( lang::EventObject( static_cast< cppu::OWeakObject * >( this )));
    m_xCurrentController.clear();

    DisposeHelper::DisposeAndClear( m_xRangeHighlighter );

    if( m_xOldModelAgg.is())
        m_xOldModelAgg->setDelegator( nullptr );

    OSL_TRACE( "ChartModel: dispose() called" );
}

void SAL_CALL ChartModel::addEventListener( const uno::Reference< lang::XEventListener > & xListener )
        throw(uno::RuntimeException, std::exception)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed() )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.addInterface( cppu::UnoType<lang::XEventListener>::get(), xListener );
}

void SAL_CALL ChartModel::removeEventListener( const uno::Reference< lang::XEventListener > & xListener )
        throw(uno::RuntimeException, std::exception)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed(false) )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.removeInterface( cppu::UnoType<lang::XEventListener>::get(), xListener );
    return;
}

// util::XCloseBroadcaster (base of XCloseable)
void SAL_CALL ChartModel::addCloseListener( const uno::Reference<   util::XCloseListener > & xListener )
        throw(uno::RuntimeException, std::exception)
{
    m_aLifeTimeManager.g_addCloseListener( xListener );
}

void SAL_CALL ChartModel::removeCloseListener( const uno::Reference< util::XCloseListener > & xListener )
        throw(uno::RuntimeException, std::exception)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed(false) )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.removeInterface( cppu::UnoType<util::XCloseListener>::get(), xListener );
    return;
}

// util::XCloseable
void SAL_CALL ChartModel::close( sal_Bool bDeliverOwnership )
            throw( util::CloseVetoException,
                   uno::RuntimeException, std::exception )
{
    //hold no mutex

    if( !m_aLifeTimeManager.g_close_startTryClose( bDeliverOwnership ) )
        return;
    //no mutex is acquired

    // At the end of this method may we must dispose ourself ...
    // and may nobody from outside hold a reference to us ...
    // then it's a good idea to do that by ourself.
    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >(this) );

    //the listeners have had no veto
    //check whether we self can close
    {
        util::CloseVetoException aVetoException = util::CloseVetoException(
                        "the model itself could not be closed",
                        static_cast< ::cppu::OWeakObject* >(this) );

        if( m_aLifeTimeManager.g_close_isNeedToCancelLongLastingCalls( bDeliverOwnership, aVetoException ) )
        {
            ////you can empty this block, if you never start longlasting calls or
            ////if your longlasting calls are per default not cancelable (check how you have constructed your LifeTimeManager)

            bool bLongLastingCallsAreCanceled = false;
            try
            {
                //try to cancel running longlasting calls
                //// @todo
            }
            catch (const uno::Exception&)
            {
                //// @todo
                //do not throw anything here!! (without endTryClose)
            }
            //if not successful canceled
            if(!bLongLastingCallsAreCanceled)
            {
                m_aLifeTimeManager.g_close_endTryClose( bDeliverOwnership, true );
                throw aVetoException;
            }
        }

    }
    m_aLifeTimeManager.g_close_endTryClose_doClose();

    // BM @todo: is it ok to call the listeners here?
    impl_notifyCloseListeners();
}

// lang::XTypeProvider
uno::Sequence< uno::Type > SAL_CALL ChartModel::getTypes()
        throw (uno::RuntimeException, std::exception)
{
    uno::Reference< lang::XTypeProvider > xAggTypeProvider;
    if( (m_xOldModelAgg->queryAggregation( cppu::UnoType<decltype(xAggTypeProvider)>::get()) >>= xAggTypeProvider)
        && xAggTypeProvider.is())
    {
        uno::Sequence< uno::Type > aOwnTypes( impl::ChartModel_Base::getTypes());
        uno::Sequence< uno::Type > aAggTypes( xAggTypeProvider->getTypes());
        uno::Sequence< uno::Type > aResult( aOwnTypes.getLength() + aAggTypes.getLength());
        sal_Int32 i=0;
        for( ;i<aOwnTypes.getLength(); ++i )
            aResult[i] = aOwnTypes[i];
        for( sal_Int32 j=0; i<aResult.getLength(); ++j, ++i)
            aResult[i] = aAggTypes[j];
        return aResult;
    }

    return impl::ChartModel_Base::getTypes();
}

// document::XDocumentPropertiesSupplier
uno::Reference< document::XDocumentProperties > SAL_CALL
        ChartModel::getDocumentProperties() throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aModelMutex );
    if ( !m_xDocumentProperties.is() )
    {
        m_xDocumentProperties.set( document::DocumentProperties::create( ::comphelper::getProcessComponentContext() ) );
    }
    return m_xDocumentProperties;
}

// document::XDocumentPropertiesSupplier
Reference< document::XUndoManager > SAL_CALL ChartModel::getUndoManager(  ) throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aModelMutex );
    if ( !m_pUndoManager.is() )
        m_pUndoManager.set( new UndoManager( *this, m_aModelMutex ) );
    return m_pUndoManager.get();
}

// chart2::XChartDocument

uno::Reference< chart2::XDiagram > SAL_CALL ChartModel::getFirstDiagram()
            throw (uno::RuntimeException, std::exception)
{
    MutexGuard aGuard( m_aModelMutex );
    return m_xDiagram;
}

void SAL_CALL ChartModel::setFirstDiagram( const uno::Reference< chart2::XDiagram >& xDiagram )
            throw (uno::RuntimeException, std::exception)
{
    Reference< chart2::XDiagram > xOldDiagram;
    Reference< util::XModifyListener > xListener;
    {
        MutexGuard aGuard( m_aModelMutex );
        if( xDiagram == m_xDiagram )
            return;
        xOldDiagram = m_xDiagram;
        m_xDiagram = xDiagram;
        xListener = this;
    }
    //don't keep the mutex locked while calling out
    ModifyListenerHelper::removeListener( xOldDiagram, xListener );
    ModifyListenerHelper::addListener( xDiagram, xListener );
    setModified( true );
}

Reference< chart2::data::XDataSource > ChartModel::impl_createDefaultData()
{
    Reference< chart2::data::XDataSource > xDataSource;
    if( hasInternalDataProvider() )
    {
        uno::Reference< lang::XInitialization > xIni(m_xInternalDataProvider,uno::UNO_QUERY);
        if( xIni.is() )
        {
            //init internal dataprovider
            {
                uno::Sequence< uno::Any > aArgs(1);
                beans::NamedValue aParam( "CreateDefaultData" ,uno::makeAny(true) );
                aArgs[0] <<= aParam;
                xIni->initialize(aArgs);
            }
            //create data
            uno::Sequence< beans::PropertyValue > aArgs( 4 );
            aArgs[0] = beans::PropertyValue(
                OUString( "CellRangeRepresentation" ), -1,
                uno::makeAny( OUString("all") ), beans::PropertyState_DIRECT_VALUE );
            aArgs[1] = beans::PropertyValue(
                "HasCategories",
                -1,
                uno::makeAny( true ),
                beans::PropertyState_DIRECT_VALUE );
            aArgs[2] = beans::PropertyValue(
                "FirstCellAsLabel",
                -1,
                uno::makeAny( true ),
                beans::PropertyState_DIRECT_VALUE );
            aArgs[3] = beans::PropertyValue(
                "DataRowSource",
                -1,
                uno::makeAny( css::chart::ChartDataRowSource_COLUMNS ),
                beans::PropertyState_DIRECT_VALUE );
            xDataSource = m_xInternalDataProvider->createDataSource( aArgs );
        }
    }
    return xDataSource;
}

void SAL_CALL ChartModel::createInternalDataProvider( sal_Bool bCloneExistingData )
            throw (util::CloseVetoException, uno::RuntimeException, std::exception)
{
    // don't lock the mutex, because this call calls out to code that tries to
    // lock the solar mutex. On the other hand, a paint locks the solar mutex
    // and calls to the model lock the model's mutex => deadlock
    // @todo: lock a separate mutex in the InternalData class
    if( !hasInternalDataProvider() )
    {
        if( bCloneExistingData )
            m_xInternalDataProvider = ChartModelHelper::createInternalDataProvider( this, true );
        else
            m_xInternalDataProvider = ChartModelHelper::createInternalDataProvider( Reference<XChartDocument>(), true );
        m_xDataProvider.set( m_xInternalDataProvider );
    }
    setModified( true );
}

sal_Bool SAL_CALL ChartModel::hasInternalDataProvider()
    throw (uno::RuntimeException, std::exception)
{
    return m_xDataProvider.is() && m_xInternalDataProvider.is();
}

uno::Reference< chart2::data::XDataProvider > SAL_CALL ChartModel::getDataProvider()
            throw (uno::RuntimeException, std::exception)
{
    MutexGuard aGuard( m_aModelMutex );
    return m_xDataProvider;
}

// ____ XDataReceiver ____

void SAL_CALL ChartModel::attachDataProvider( const uno::Reference< chart2::data::XDataProvider >& xDataProvider )
            throw (uno::RuntimeException, std::exception)
{
    {
        MutexGuard aGuard( m_aModelMutex );
        uno::Reference< beans::XPropertySet > xProp( xDataProvider, uno::UNO_QUERY );
        if( xProp.is() )
        {
            try
            {
                bool bIncludeHiddenCells = ChartModelHelper::isIncludeHiddenCells( Reference< frame::XModel >(this) );
                xProp->setPropertyValue("IncludeHiddenCells", uno::makeAny(bIncludeHiddenCells));
            }
            catch (const beans::UnknownPropertyException&)
            {
            }
        }

        m_xDataProvider.set( xDataProvider );
        m_xInternalDataProvider.clear();

        //the numberformatter is kept independent of the data provider!
    }
    setModified( true );
}

void SAL_CALL ChartModel::attachNumberFormatsSupplier( const uno::Reference< util::XNumberFormatsSupplier >& xNewSupplier )
            throw (uno::RuntimeException, std::exception)
{
    {
        MutexGuard aGuard( m_aModelMutex );
        if( xNewSupplier==m_xNumberFormatsSupplier )
            return;
        if( xNewSupplier==m_xOwnNumberFormatsSupplier )
            return;
        if( m_xOwnNumberFormatsSupplier.is() && xNewSupplier.is() )
        {
            //@todo
            //merge missing numberformats from own to new formatter
        }
        else if( !xNewSupplier.is() )
        {
            if( m_xNumberFormatsSupplier.is() )
            {
                //@todo
                //merge missing numberformats from old numberformatter to own numberformatter
                //create own numberformatter if necessary
            }
        }

        m_xNumberFormatsSupplier.set( xNewSupplier );
        m_xOwnNumberFormatsSupplier.clear();
    }
    setModified( true );
}

void SAL_CALL ChartModel::setArguments( const Sequence< beans::PropertyValue >& aArguments )
            throw (lang::IllegalArgumentException,
                   uno::RuntimeException, std::exception)
{
    {
        MutexGuard aGuard( m_aModelMutex );
        if( !m_xDataProvider.is() )
            return;
        lockControllers();

        try
        {
            Reference< chart2::data::XDataSource > xDataSource( m_xDataProvider->createDataSource( aArguments ) );
            if( xDataSource.is() )
            {
                Reference< chart2::XDiagram > xDia( getFirstDiagram() );
                if( !xDia.is() )
                {
                    Reference< chart2::XChartTypeTemplate > xTemplate( impl_createDefaultChartTypeTemplate() );
                    if( xTemplate.is())
                        setFirstDiagram( xTemplate->createDiagramByDataSource( xDataSource, aArguments ) );
                }
                else
                    xDia->setDiagramData( xDataSource, aArguments );
            }
        }
        catch (const lang::IllegalArgumentException&)
        {
            throw;
        }
        catch (const uno::Exception& ex)
        {
            ASSERT_EXCEPTION( ex );
        }
        unlockControllers();
    }
    setModified( true );
}

Sequence< OUString > SAL_CALL ChartModel::getUsedRangeRepresentations()
            throw (uno::RuntimeException, std::exception)
{
    return DataSourceHelper::getUsedDataRanges( Reference< frame::XModel >(this));
}

Reference< chart2::data::XDataSource > SAL_CALL ChartModel::getUsedData()
            throw (uno::RuntimeException, std::exception)
{
    return DataSourceHelper::getUsedData( Reference< chart2::XChartDocument >(this));
}

Reference< chart2::data::XRangeHighlighter > SAL_CALL ChartModel::getRangeHighlighter()
            throw (uno::RuntimeException, std::exception)
{
    if( ! m_xRangeHighlighter.is())
    {
        uno::Reference< view::XSelectionSupplier > xSelSupp( this->getCurrentController(), uno::UNO_QUERY );
        if( xSelSupp.is() )
            m_xRangeHighlighter.set( ChartModelHelper::createRangeHighlighter( xSelSupp ));
    }
    return m_xRangeHighlighter;
}

Reference< chart2::XChartTypeTemplate > ChartModel::impl_createDefaultChartTypeTemplate()
{
    Reference< chart2::XChartTypeTemplate > xTemplate;
    Reference< lang::XMultiServiceFactory > xFact( m_xChartTypeManager, uno::UNO_QUERY );
    if( xFact.is() )
        xTemplate.set( xFact->createInstance( "com.sun.star.chart2.template.Column" ), uno::UNO_QUERY );
    return xTemplate;
}

void SAL_CALL ChartModel::setChartTypeManager( const uno::Reference< chart2::XChartTypeManager >& xNewManager )
            throw (uno::RuntimeException, std::exception)
{
    {
        MutexGuard aGuard( m_aModelMutex );
        m_xChartTypeManager = xNewManager;
    }
    setModified( true );
}

uno::Reference< chart2::XChartTypeManager > SAL_CALL ChartModel::getChartTypeManager()
            throw (uno::RuntimeException, std::exception)
{
    MutexGuard aGuard( m_aModelMutex );
    return m_xChartTypeManager;
}

uno::Reference< beans::XPropertySet > SAL_CALL ChartModel::getPageBackground()
    throw (uno::RuntimeException, std::exception)
{
    MutexGuard aGuard( m_aModelMutex );
    return m_xPageBackground;
}

void SAL_CALL ChartModel::createDefaultChart()
    throw (css::uno::RuntimeException, std::exception)
{
    insertDefaultChart();
}

sal_Bool SAL_CALL ChartModel::isOpenGLChart()
    throw (css::uno::RuntimeException, std::exception)
{
    return ChartHelper::isGL3DDiagram(m_xDiagram);
}

// ____ XTitled ____
uno::Reference< chart2::XTitle > SAL_CALL ChartModel::getTitleObject()
    throw (uno::RuntimeException, std::exception)
{
    MutexGuard aGuard( m_aModelMutex );
    return m_xTitle;
}

void SAL_CALL ChartModel::setTitleObject( const uno::Reference< chart2::XTitle >& xTitle )
    throw (uno::RuntimeException, std::exception)
{
    {
        MutexGuard aGuard( m_aModelMutex );
        if( m_xTitle.is() )
            ModifyListenerHelper::removeListener( m_xTitle, this );
        m_xTitle = xTitle;
        ModifyListenerHelper::addListener( m_xTitle, this );
    }
    setModified( true );
}

// ____ XInterface (for old API wrapper) ____
uno::Any SAL_CALL ChartModel::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException, std::exception)
{
    uno::Any aResult( impl::ChartModel_Base::queryInterface( aType ));

    if( ! aResult.hasValue())
    {
        // try old API wrapper
        try
        {
            if( m_xOldModelAgg.is())
                aResult = m_xOldModelAgg->queryAggregation( aType );
        }
        catch (const uno::Exception& ex)
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return aResult;
}

// ____ XCloneable ____
Reference< util::XCloneable > SAL_CALL ChartModel::createClone()
    throw (uno::RuntimeException, std::exception)
{
    return Reference< util::XCloneable >( new ChartModel( *this ));
}

// ____ XVisualObject ____
void SAL_CALL ChartModel::setVisualAreaSize( ::sal_Int64 nAspect, const awt::Size& aSize )
    throw (lang::IllegalArgumentException,
           embed::WrongStateException,
           uno::Exception,
           uno::RuntimeException, std::exception)
{
    if( nAspect == embed::Aspects::MSOLE_CONTENT )
    {
        ControllerLockGuard aLockGuard( *this );
        bool bChanged =
            (m_aVisualAreaSize.Width != aSize.Width ||
             m_aVisualAreaSize.Height != aSize.Height);

        // #i12587# support for shapes in chart
        if ( bChanged )
        {
            impl_adjustAdditionalShapesPositionAndSize( aSize );
        }

        m_aVisualAreaSize = aSize;
        if( bChanged )
            setModified( true );
    }
    else
    {
        OSL_FAIL( "setVisualAreaSize: Aspect not implemented yet.");
    }
}

awt::Size SAL_CALL ChartModel::getVisualAreaSize( ::sal_Int64 nAspect )
    throw (lang::IllegalArgumentException,
           embed::WrongStateException,
           uno::Exception,
           uno::RuntimeException, std::exception)
{
    OSL_ENSURE( nAspect == embed::Aspects::MSOLE_CONTENT,
                "No aspects other than content are supported" );
    (void)(nAspect); // avoid warning in non-debug builds
    // other possible aspects are MSOLE_THUMBNAIL, MSOLE_ICON and MSOLE_DOCPRINT

    return m_aVisualAreaSize;
}

embed::VisualRepresentation SAL_CALL ChartModel::getPreferredVisualRepresentation( ::sal_Int64 nAspect )
    throw (lang::IllegalArgumentException,
           embed::WrongStateException,
           uno::Exception,
           uno::RuntimeException, std::exception)
{
    OSL_ENSURE( nAspect == embed::Aspects::MSOLE_CONTENT,
                "No aspects other than content are supported" );
    (void)(nAspect); // avoid warning in non-debug builds

    embed::VisualRepresentation aResult;

    try
    {
        Sequence< sal_Int8 > aMetafile;

        //get view from old api wrapper
        Reference< datatransfer::XTransferable > xTransferable(
            this->createInstance( CHART_VIEW_SERVICE_NAME ), uno::UNO_QUERY );
        if( xTransferable.is() )
        {
            datatransfer::DataFlavor aDataFlavor( lcl_aGDIMetaFileMIMEType,
                    "GDIMetaFile",
                    cppu::UnoType<uno::Sequence< sal_Int8 >>::get() );

            uno::Any aData( xTransferable->getTransferData( aDataFlavor ) );
            aData >>= aMetafile;
        }

        aResult.Flavor.MimeType = lcl_aGDIMetaFileMIMEType;
        aResult.Flavor.DataType = cppu::UnoType<decltype(aMetafile)>::get();

        aResult.Data <<= aMetafile;
    }
    catch (const uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );
    }

    return aResult;
}

::sal_Int32 SAL_CALL ChartModel::getMapUnit( ::sal_Int64 nAspect )
    throw (uno::Exception,
           uno::RuntimeException, std::exception)
{
    OSL_ENSURE( nAspect == embed::Aspects::MSOLE_CONTENT,
                "No aspects other than content are supported" );
    (void)(nAspect); // avoid warning in non-debug builds
    return embed::EmbedMapUnits::ONE_100TH_MM;
}

// ____ datatransfer::XTransferable ____
uno::Any SAL_CALL ChartModel::getTransferData( const datatransfer::DataFlavor& aFlavor )
    throw (datatransfer::UnsupportedFlavorException,
           io::IOException,
           uno::RuntimeException, std::exception)
{
    uno::Any aResult;
    if( this->isDataFlavorSupported( aFlavor ))
    {
        try
        {
            //get view from old api wrapper
            Reference< datatransfer::XTransferable > xTransferable(
                this->createInstance( CHART_VIEW_SERVICE_NAME ), uno::UNO_QUERY );
            if( xTransferable.is() &&
                xTransferable->isDataFlavorSupported( aFlavor ))
            {
                aResult = xTransferable->getTransferData( aFlavor );
            }
        }
        catch (const uno::Exception& ex)
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    else
    {
        throw datatransfer::UnsupportedFlavorException(
            aFlavor.MimeType, static_cast< ::cppu::OWeakObject* >( this ));
    }

    return aResult;
}

Sequence< datatransfer::DataFlavor > SAL_CALL ChartModel::getTransferDataFlavors()
    throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< datatransfer::DataFlavor > aRet(1);

    aRet[0] = datatransfer::DataFlavor( lcl_aGDIMetaFileMIMETypeHighContrast,
        "GDIMetaFile",
        cppu::UnoType<uno::Sequence< sal_Int8 >>::get() );

    return aRet;
}

sal_Bool SAL_CALL ChartModel::isDataFlavorSupported( const datatransfer::DataFlavor& aFlavor )
    throw (uno::RuntimeException, std::exception)
{
    return aFlavor.MimeType.equals(lcl_aGDIMetaFileMIMETypeHighContrast);
}

namespace
{
enum eServiceType
{
    SERVICE_DASH_TABLE,
    SERVICE_GARDIENT_TABLE,
    SERVICE_HATCH_TABLE,
    SERVICE_BITMAP_TABLE,
    SERVICE_TRANSP_GRADIENT_TABLE,
    SERVICE_MARKER_TABLE,
    SERVICE_NAMESPACE_MAP
};

typedef ::std::map< OUString, enum eServiceType > tServiceNameMap;

tServiceNameMap & lcl_getStaticServiceNameMap()
{
    static tServiceNameMap aServiceNameMap{
        {"com.sun.star.drawing.DashTable",                    SERVICE_DASH_TABLE},
        {"com.sun.star.drawing.GradientTable",                SERVICE_GARDIENT_TABLE},
        {"com.sun.star.drawing.HatchTable",                   SERVICE_HATCH_TABLE},
        {"com.sun.star.drawing.BitmapTable",                  SERVICE_BITMAP_TABLE},
        {"com.sun.star.drawing.TransparencyGradientTable",    SERVICE_TRANSP_GRADIENT_TABLE},
        {"com.sun.star.drawing.MarkerTable",                  SERVICE_MARKER_TABLE},
        {"com.sun.star.xml.NamespaceMap",                     SERVICE_NAMESPACE_MAP}};
    return aServiceNameMap;
}
}
// ____ XMultiServiceFactory ____
Reference< uno::XInterface > SAL_CALL ChartModel::createInstance( const OUString& rServiceSpecifier )
            throw( uno::Exception, uno::RuntimeException, std::exception )
{
    uno::Reference< uno::XInterface > xResult;
    tServiceNameMap & rMap = lcl_getStaticServiceNameMap();

    tServiceNameMap::const_iterator aIt( rMap.find( rServiceSpecifier ));
    if( aIt != rMap.end())
    {
        switch( (*aIt).second )
        {
            case SERVICE_DASH_TABLE:
            case SERVICE_GARDIENT_TABLE:
            case SERVICE_HATCH_TABLE:
            case SERVICE_BITMAP_TABLE:
            case SERVICE_TRANSP_GRADIENT_TABLE:
            case SERVICE_MARKER_TABLE:
                {
                    if(!mpChartView)
                    {
                        mpChartView = new ChartView( m_xContext, *this);
                        xChartView = static_cast< ::cppu::OWeakObject* >( mpChartView );
                    }
                    return mpChartView->createInstance( rServiceSpecifier );
                }
                break;
            case SERVICE_NAMESPACE_MAP:
                return Reference< uno::XInterface >( m_xXMLNamespaceMap );
        }
    }
    else if(rServiceSpecifier == CHART_VIEW_SERVICE_NAME)
    {
        if(!mpChartView)
        {
            mpChartView = new ChartView( m_xContext, *this);
            xChartView = static_cast< ::cppu::OWeakObject* >( mpChartView );
        }

        return static_cast< ::cppu::OWeakObject* >( mpChartView );
    }
    else
    {
        if( m_xOldModelAgg.is() )
        {
            Any aAny = m_xOldModelAgg->queryAggregation( cppu::UnoType<lang::XMultiServiceFactory>::get());
            uno::Reference< lang::XMultiServiceFactory > xOldModelFactory;
            if( (aAny >>= xOldModelFactory) && xOldModelFactory.is() )
            {
                return xOldModelFactory->createInstance( rServiceSpecifier );
            }
        }
    }
    return nullptr;
}

Reference< uno::XInterface > SAL_CALL ChartModel::createInstanceWithArguments(
            const OUString& rServiceSpecifier , const Sequence< Any >& Arguments )
            throw( uno::Exception, uno::RuntimeException, std::exception )
{
    OSL_ENSURE( Arguments.getLength(), "createInstanceWithArguments: Warning: Arguments are ignored" );
    (void)(Arguments); // avoid warning in non-debug builds
    return createInstance( rServiceSpecifier );
}

Sequence< OUString > SAL_CALL ChartModel::getAvailableServiceNames()
            throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aResult;

    if( m_xOldModelAgg.is())
    {
        Any aAny = m_xOldModelAgg->queryAggregation( cppu::UnoType<lang::XMultiServiceFactory>::get());
        uno::Reference< lang::XMultiServiceFactory > xOldModelFactory;
        if( (aAny >>= xOldModelFactory) && xOldModelFactory.is() )
        {
            return xOldModelFactory->getAvailableServiceNames();
        }
    }
    return aResult;
}

Reference< util::XNumberFormatsSupplier > const & ChartModel::getNumberFormatsSupplier()
{
    if( !m_xNumberFormatsSupplier.is() )
    {
        if( !m_xOwnNumberFormatsSupplier.is() )
        {
            m_apSvNumberFormatter.reset( new SvNumberFormatter( m_xContext, LANGUAGE_SYSTEM ) );
            m_xOwnNumberFormatsSupplier = new SvNumberFormatsSupplierObj( m_apSvNumberFormatter.get() );
            //pOwnNumberFormatter->ChangeStandardPrec( 15 ); todo?
        }
        m_xNumberFormatsSupplier = m_xOwnNumberFormatsSupplier;
    }
    return m_xNumberFormatsSupplier;
}

// ____ XUnoTunnel ___
::sal_Int64 SAL_CALL ChartModel::getSomething( const Sequence< ::sal_Int8 >& aIdentifier )
        throw( uno::RuntimeException, std::exception)
{
    if( aIdentifier.getLength() == 16 && 0 == memcmp( SvNumberFormatsSupplierObj::getUnoTunnelId().getConstArray(),
                                                         aIdentifier.getConstArray(), 16 ) )
    {
        Reference< lang::XUnoTunnel > xTunnel( getNumberFormatsSupplier(), uno::UNO_QUERY );
        if( xTunnel.is() )
            return xTunnel->getSomething( aIdentifier );
    }
    return 0;
}

// ____ XNumberFormatsSupplier ____
uno::Reference< beans::XPropertySet > SAL_CALL ChartModel::getNumberFormatSettings()
    throw (uno::RuntimeException, std::exception)
{
    Reference< util::XNumberFormatsSupplier > xSupplier( getNumberFormatsSupplier() );
    if( xSupplier.is() )
        return xSupplier->getNumberFormatSettings();
    return uno::Reference< beans::XPropertySet >();
}

uno::Reference< util::XNumberFormats > SAL_CALL ChartModel::getNumberFormats()
    throw (uno::RuntimeException, std::exception)
{
    Reference< util::XNumberFormatsSupplier > xSupplier( getNumberFormatsSupplier() );
    if( xSupplier.is() )
        return xSupplier->getNumberFormats();
    return uno::Reference< util::XNumberFormats >();
}

// ____ XChild ____
Reference< uno::XInterface > SAL_CALL ChartModel::getParent()
    throw (uno::RuntimeException, std::exception)
{
    return Reference< uno::XInterface >(m_xParent,uno::UNO_QUERY);
}

void SAL_CALL ChartModel::setParent( const Reference< uno::XInterface >& Parent )
    throw (lang::NoSupportException,
           uno::RuntimeException, std::exception)
{
    if( Parent != m_xParent )
        m_xParent.set( Parent, uno::UNO_QUERY );
}

// ____ XDataSource ____
uno::Sequence< Reference< chart2::data::XLabeledDataSequence > > SAL_CALL ChartModel::getDataSequences()
    throw (uno::RuntimeException, std::exception)
{
    Reference< chart2::data::XDataSource > xSource(
        DataSourceHelper::getUsedData( uno::Reference< frame::XModel >(this) ) );
    if( xSource.is())
        return xSource->getDataSequences();

    return uno::Sequence< Reference< chart2::data::XLabeledDataSequence > >();
}

//XDumper
OUString SAL_CALL ChartModel::dump()
    throw (uno::RuntimeException, std::exception)
{
    uno::Reference< qa::XDumper > xDumper(
            this->createInstance( CHART_VIEW_SERVICE_NAME ), uno::UNO_QUERY );
    if (xDumper.is())
        return xDumper->dump();

    return OUString();
}

void ChartModel::setTimeBased(bool bTimeBased)
{
    mbTimeBased = bTimeBased;
    uno::Sequence<Reference< chart2::data::XLabeledDataSequence > >
        xDataSequences = getDataSequences();
    sal_Int32 n = xDataSequences.getLength();
    for(sal_Int32 i = 0; i < n; ++i)
    {
        uno::Reference< chart2::XTimeBased > xTimeBased(xDataSequences[i]->getValues(), uno::UNO_QUERY);
        SAL_WARN_IF(!xTimeBased.is(), "chart2", "does not support time based charting");
        if(xTimeBased.is())
        {
            uno::Reference< beans::XPropertySet > xPropSet(xTimeBased, uno::UNO_QUERY_THROW);
            xPropSet->setPropertyValue("TimeBased", uno::makeAny(bTimeBased));
        }
    }
}

void ChartModel::getNextTimePoint()
{
    uno::Sequence< Reference< chart2::data::XLabeledDataSequence > > xDataSequences = getDataSequences();
    sal_Int32 n = xDataSequences.getLength();
    for(sal_Int32 i = 0; i < n; ++i)
    {
        uno::Reference< chart2::XTimeBased > xTimeBased(xDataSequences[i]->getValues(), uno::UNO_QUERY);
        SAL_WARN_IF(!xTimeBased.is(), "chart2", "does not support time based charting");
        if(xTimeBased.is())
        {
            if(!bSet)
                xTimeBased->setRange(mnStart, mnEnd);
            xTimeBased->switchToNext(true);
        }
    }
    bSet = true;
}

void ChartModel::setTimeBasedRange(sal_Int32 nStart, sal_Int32 nEnd)
{
    bSet = false;
    mnStart = nStart;
    mnEnd = nEnd;
    mbTimeBased = true;
}

void ChartModel::setWindow( const sal_uInt64 nWindowPtr )
    throw (uno::RuntimeException, std::exception)
{
    OpenGLWindow* pWindow = reinterpret_cast<OpenGLWindow*>(nWindowPtr);
    mpOpenGLWindow = pWindow;
}

void ChartModel::update()
    throw (uno::RuntimeException, std::exception)
{
    if(!mpChartView)
    {
        mpChartView = new ChartView( m_xContext, *this);
        xChartView = static_cast< ::cppu::OWeakObject* >( mpChartView );
    }
    if(mpChartView)
    {
        mpChartView->setViewDirty();
        mpChartView->update();
        mpChartView->updateOpenGLWindow();
    }
}

}  // namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_ChartModel_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::ChartModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
