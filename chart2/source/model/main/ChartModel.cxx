/*************************************************************************
 *
 *  $RCSfile: ChartModel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:30 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "ChartModel.hxx"
#include "ImplChartModel.hxx"
#include "servicenames.hxx"
#include "MediaDescriptorHelper.hxx"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::rtl::OUString;
using ::osl::MutexGuard;

using namespace ::drafts::com::sun::star;
using namespace ::com::sun::star;
using namespace ::apphelper;

//-----------------------------------------------------------------
// ChartModel Constructor and Destructor
//-----------------------------------------------------------------

namespace chart
{

ChartModel::ChartModel(uno::Reference<uno::XComponentContext > const & xContext)
    : m_aLifeTimeManager( this, this )
    , m_bReadOnly( sal_False )
    , m_bModified( sal_False )
    , m_aResource()
    , m_pControllers( NULL )
    , m_xCurrentController( NULL )
    , m_nControllerLockCount(0)
    , m_pImplChartModel( new impl::ImplChartModel( xContext ) )
{
    OSL_TRACE( "ChartModel: CTOR" );
}

ChartModel::~ChartModel()
{
    //@todo

    if ( m_pControllers )
        delete m_pControllers;
}

//-----------------------------------------------------------------
// private methods
//-----------------------------------------------------------------

        ::rtl::OUString ChartModel
::impl_g_getLocation()
{

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return ::rtl::OUString(); //behave passive if already disposed or closed or throw exception @todo?
    //mutex is acquired
    return m_aResource;
}

        ::cppu::OInterfaceContainerHelper* ChartModel
::impl_getControllerContainer() throw(uno::RuntimeException)
{
    if ( m_pControllers )
        return m_pControllers;
    m_pControllers =
        new ::cppu::OInterfaceContainerHelper( m_aModelMutex );
    return m_pControllers;
}

        sal_Bool ChartModel
::impl_isControllerConnected( const uno::Reference< frame::XController >& xController )
{
    try
    {
        if( !m_pControllers )
            return sal_False;

        uno::Sequence< uno::Reference<uno::XInterface> > aSeq = impl_getControllerContainer()->getElements();
        for( sal_Int32 nN = aSeq.getLength(); nN--; )
        {
            if( aSeq[nN] == xController )
                return sal_True;
        }
    }
    catch( uno::Exception )
    {
    }
    return sal_False;
}

            uno::Reference< frame::XController > ChartModel
::impl_getCurrentController() throw(uno::RuntimeException)
{
        //@todo? hold only weak references to controllers

    // get the last active controller of this model
    if( m_xCurrentController.is() )
        return m_xCurrentController;

    // get the first controller of this model
    if( m_pControllers && impl_getControllerContainer()->getLength() )
    {
        uno::Reference<uno::XInterface> xI = impl_getControllerContainer()->getElements()[0];
        return uno::Reference<frame::XController>( xI, uno::UNO_QUERY );
    }

    //return nothing if no controllers are connected at all
    return uno::Reference< frame::XController > ();
}

        void SAL_CALL ChartModel
::impl_notifyModifiedListeners()
        throw( uno::RuntimeException)
{
    ::cppu::OInterfaceContainerHelper* pIC = m_aLifeTimeManager.m_aListenerContainer
        .getContainer( ::getCppuType((const uno::Reference< util::XModifyListener >*)0) );
    if( pIC )
    {
        lang::EventObject aEvent( static_cast< lang::XComponent*>(this) );
        ::cppu::OInterfaceIteratorHelper aIt( *pIC );
        while( aIt.hasMoreElements() )
            (static_cast< util::XModifyListener*>(aIt.next()))->modified( aEvent );
    }
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------

APPHELPER_XSERVICEINFO_IMPL(ChartModel,CHART_MODEL_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > ChartModel
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 2 );
    aSNS.getArray()[ 0 ] = CHART_MODEL_SERVICE_NAME;
    aSNS.getArray()[ 1 ] = ::rtl::OUString::createFromAscii("com.sun.star.document.OfficeDocument");
    //// @todo : add additional services if you support any further
    return aSNS;
}

//-----------------------------------------------------------------
// frame::XModel (required interface)
//-----------------------------------------------------------------

        sal_Bool SAL_CALL ChartModel
::attachResource( const ::rtl::OUString& rURL
        , const uno::Sequence< beans::PropertyValue >& rMediaDescriptor )
        throw(uno::RuntimeException)
{
    /*
    The method attachResource() is used by the frame loader implementations
    to inform the model about its URL and MediaDescriptor.
    */

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return sal_False; //behave passive if already disposed or closed or throw exception @todo?
    //mutex is acquired

    if(m_aResource.getLength()!=0)//we have a resource already //@todo? or is setting a new resource allowed?
        return sal_False;
    m_aResource = rtl::OUString( rURL );
    m_aMediaDescriptor = rMediaDescriptor;

    //@todo ? check rURL ??
    //@todo ? evaluate m_aMediaDescriptor;
    //@todo ? ... ??? --> nothing, this method is only for setting informations

    return sal_True;
}

        ::rtl::OUString SAL_CALL ChartModel
::getURL() throw(uno::RuntimeException)
{
    return impl_g_getLocation();
}

        uno::Sequence< beans::PropertyValue > SAL_CALL ChartModel
::getArgs() throw(uno::RuntimeException)
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

        void SAL_CALL ChartModel
::connectController( const uno::Reference< frame::XController >& xController )
        throw(uno::RuntimeException)
{
    //@todo? this method is declared as oneway -> ...?

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return ; //behave passive if already disposed or closed
    //mutex is acquired

    //--add controller
    impl_getControllerContainer()->addInterface(xController);
}

        void SAL_CALL ChartModel
::disconnectController( const uno::Reference< frame::XController >& xController )
        throw(uno::RuntimeException)
{
    //@todo? this method is declared as oneway -> ...?

    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return; //behave passive if already disposed or closed

    //--remove controller
    impl_getControllerContainer()->removeInterface(xController);

    //case: current controller is disconnected:
    if( m_xCurrentController == xController )
        m_xCurrentController = uno::Reference< frame::XController > ();
}

        void SAL_CALL ChartModel
::lockControllers() throw(uno::RuntimeException)
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

        void SAL_CALL ChartModel
::unlockControllers() throw(uno::RuntimeException)
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
    --m_nControllerLockCount;
}

    sal_Bool SAL_CALL ChartModel
::hasControllersLocked() throw(uno::RuntimeException)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        return sal_False; //behave passive if already disposed or closed or throw exception @todo?
    return ( m_nControllerLockCount != 0 ) ;
}

        uno::Reference< frame::XController > SAL_CALL ChartModel
::getCurrentController() throw(uno::RuntimeException)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        throw lang::DisposedException(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "getCurrentController was called on an already disposed or closed model" ) )
                , static_cast< ::cppu::OWeakObject* >(this));

    return impl_getCurrentController();
}

        void SAL_CALL ChartModel
::setCurrentController( const uno::Reference< frame::XController >& xController )
        throw(container::NoSuchElementException, uno::RuntimeException)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        throw lang::DisposedException(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "setCurrentController was called on an already disposed or closed model" ) )
                , static_cast< ::cppu::OWeakObject* >(this));

    //OSL_ENSURE( impl_isControllerConnected(xController), "setCurrentController is called with a Controller which is not connected" );
    if(!impl_isControllerConnected(xController))
        throw container::NoSuchElementException(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "setCurrentController is called with a Controller which is not connected" ) )
                , static_cast< ::cppu::OWeakObject* >(this));

    m_xCurrentController = xController;
}

        uno::Reference< uno::XInterface > SAL_CALL ChartModel
::getCurrentSelection() throw(uno::RuntimeException)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())
        throw lang::DisposedException(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "getCurrentSelection was called on an already disposed or closed model" ) )
                , static_cast< ::cppu::OWeakObject* >(this));


    uno::Reference< uno::XInterface > xReturn=NULL;
    uno::Reference< frame::XController > xController = impl_getCurrentController();

    aGuard.clear();
    if( xController.is() )
    {
        uno::Reference< view::XSelectionSupplier >  xSelectionSupl( xController, uno::UNO_QUERY );
        if ( xSelectionSupl.is() )
        {
            uno::Any aSel = xSelectionSupl->getSelection();
            aSel >>= xReturn ;
        }
    }
    return xReturn;
}


//-----------------------------------------------------------------
// lang::XComponent (base of XModel)
//-----------------------------------------------------------------
        void SAL_CALL ChartModel
::dispose() throw(uno::RuntimeException)
{
    //This object should release all resources and references in the
    //easiest possible manner
    //This object must notify all registered listeners using the method
    //<member>XEventListener::disposing</member>

    //hold no mutex
    if( !m_aLifeTimeManager.dispose() )
        return;

    //--release all resources and references
    //// @todo
}

        void SAL_CALL ChartModel
::addEventListener( const uno::Reference< lang::XEventListener > & xListener )
        throw(uno::RuntimeException)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed() )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.addInterface( ::getCppuType((const uno::Reference< util::XCloseListener >*)0), xListener );
}

        void SAL_CALL ChartModel
::removeEventListener( const uno::Reference< lang::XEventListener > & xListener )
        throw(uno::RuntimeException)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed() )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.removeInterface( ::getCppuType((const uno::Reference< util::XCloseListener >*)0), xListener );
    return;
}

//-----------------------------------------------------------------
// util::XCloseBroadcaster (base of XCloseable)
//-----------------------------------------------------------------
        void SAL_CALL ChartModel
::addCloseListener( const uno::Reference<   util::XCloseListener > & xListener )
        throw(uno::RuntimeException)
{
    m_aLifeTimeManager.g_addCloseListener( xListener );
}

        void SAL_CALL ChartModel
::removeCloseListener( const uno::Reference< util::XCloseListener > & xListener )
        throw(uno::RuntimeException)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed() )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.removeInterface( ::getCppuType((const uno::Reference< util::XCloseListener >*)0), xListener );
    return;
}

//-----------------------------------------------------------------
// util::XCloseable
//-----------------------------------------------------------------
        void SAL_CALL ChartModel
::close( sal_Bool bDeliverOwnership )
        throw( util::CloseVetoException )
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
    //check wether we self can close
    {
        util::CloseVetoException aVetoException = util::CloseVetoException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "the model itself could not be closed" ) )
                        , static_cast< ::cppu::OWeakObject* >(this));

        if( m_aLifeTimeManager.g_close_isNeedToCancelLongLastingCalls( bDeliverOwnership, aVetoException) )
        {
            ////you can empty this block, if you never start longlasting calls or
            ////if your longlasting calls are per default not cancelable (check how you have constructed your LifeTimeManager)

            sal_Bool bLongLastingCallsAreCanceled = sal_False;
            try
            {
                //try to cancel running longlasting calls
                //// @todo
            }
            catch( uno::Exception )
            {
                //// @todo
                //do not throw anything here!! (without endTryClose)
            }
            //if not successful canceled
            if(!bLongLastingCallsAreCanceled)
            {
                m_aLifeTimeManager.g_close_endTryClose(bDeliverOwnership, sal_True, aVetoException);
                //the exception is thrown in the above method
            }
        }

    }
    m_aLifeTimeManager.g_close_endTryClose_doClose();
}

//-----------------------------------------------------------------
// frame::XStorable (required interface)
//-----------------------------------------------------------------
        sal_Bool SAL_CALL ChartModel
::hasLocation() throw(uno::RuntimeException)
{
    //@todo guard
    return m_aResource.getLength()!=0;
}

        ::rtl::OUString SAL_CALL ChartModel
::getLocation() throw(uno::RuntimeException)
{
    return impl_g_getLocation();
}

        sal_Bool SAL_CALL ChartModel
::isReadonly() throw(uno::RuntimeException)
{
    //@todo guard
    return m_bReadOnly;
}

        void SAL_CALL ChartModel
::store() throw(io::IOException, uno::RuntimeException)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall(sal_True)) //start LongLastingCall
        return; //behave passive if already disposed or closed or throw exception @todo?

    ::rtl::OUString aLocation = m_aResource;

    if( aLocation.getLength() == 0 )
        throw io::IOException(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no location specified" ) )
                            , static_cast< ::cppu::OWeakObject* >(this));
    //@todo check wether aLocation is something like private:factory...
    if( m_bReadOnly )
        throw io::IOException(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "document is read only" ) )
                            , static_cast< ::cppu::OWeakObject* >(this));

    m_bModified = sal_False;
    aGuard.clear();

    //@todo store
}

        void SAL_CALL ChartModel
::storeAsURL( const ::rtl::OUString& rURL
        , const uno::Sequence< beans::PropertyValue >& rMediaDescriptor )
        throw(io::IOException, uno::RuntimeException)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall(sal_True)) //start LongLastingCall
        return; //behave passive if already disposed or closed or throw exception @todo?

    apphelper::MediaDescriptorHelper aMediaDescriptorHelper(rMediaDescriptor);
    m_aMediaDescriptor = aMediaDescriptorHelper.getReducedForModel();
    m_aResource = rURL;

    m_bReadOnly = sal_False;
    m_bModified = sal_False;
    aGuard.clear();

    //@todo saveTo(rURL)
}

        void SAL_CALL ChartModel
::storeToURL( const ::rtl::OUString& rURL
        , const uno::Sequence< beans::PropertyValue >& rMediaDescriptor )
        throw(io::IOException, uno::RuntimeException)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall(sal_True)) //start LongLastingCall
        return; //behave passive if already disposed or closed or throw exception @todo?
    //do not change the internal state of the document here
    //...

    aGuard.clear();
    //@todo saveTo(rURL)
}

//-----------------------------------------------------------------
// util::XModifiable (required interface)
//-----------------------------------------------------------------
        sal_Bool SAL_CALL ChartModel
::isModified() throw(uno::RuntimeException)
{
    //@todo guard
    return m_bModified;
}

        void SAL_CALL ChartModel
::setModified( sal_Bool bModified )
        throw(beans::PropertyVetoException, uno::RuntimeException)
{
    LifeTimeGuard aGuard(m_aLifeTimeManager);
    if(!aGuard.startApiCall())//@todo ? is this a long lasting call??
        return; //behave passive if already disposed or closed or throw exception @todo?
    m_bModified = bModified;
    aGuard.clear();

    if(bModified)
        impl_notifyModifiedListeners();
}

//-----------------------------------------------------------------
// util::XModifyBroadcaster (base of XModifiable)
//-----------------------------------------------------------------
        void SAL_CALL ChartModel
::addModifyListener( const uno::Reference< util::XModifyListener >& xListener )
        throw(uno::RuntimeException)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed() )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.addInterface( ::getCppuType((const uno::Reference< util::XModifyListener >*)0), xListener );
}

        void SAL_CALL ChartModel
::removeModifyListener( const uno::Reference< util::XModifyListener >& xListener )
        throw(uno::RuntimeException)
{
    if( m_aLifeTimeManager.impl_isDisposedOrClosed() )
        return; //behave passive if already disposed or closed

    m_aLifeTimeManager.m_aListenerContainer.removeInterface( ::getCppuType((const uno::Reference< util::XModifyListener >*)0), xListener );
}
/*
//-----------------------------------------------------------------
// view::XPrintable (optional interface)
//-----------------------------------------------------------------
        uno::Sequence< beans::PropertyValue > SAL_CALL ChartModel
::getPrinter() throw(uno::RuntimeException)
{
    //@todo  guard
    return m_aPrinterOptions;
}

        void SAL_CALL ChartModel
::setPrinter( const uno::Sequence< beans::PropertyValue >& rPrinter )
        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    //@todo  guard
    m_aPrinterOptions = rPrinter;
}

        void SAL_CALL ChartModel
::print( const uno::Sequence< beans::PropertyValue >& rOptions )
        throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    //@todo
}

//-----------------------------------------------------------------
// document::XEventBroadcaster (optional interface)
//-----------------------------------------------------------------

        void SAL_CALL ChartModel
::addEventListener( const uno::Reference< document::XEventListener >& xListener )
        throw(uno::RuntimeException)
{
    //@todo
}

        void SAL_CALL ChartModel
::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
        throw(uno::RuntimeException)
{
    //@todo
}

//-----------------------------------------------------------------
// document::XEventsSupplier (optional interface)
//-----------------------------------------------------------------

        uno::Reference< container::XNameReplace > SAL_CALL ChartModel
::getEvents() throw(uno::RuntimeException)
{
    //@todo
}

//-----------------------------------------------------------------
// document::XDocumentInfoSupplier (optional interface)
//-----------------------------------------------------------------

        uno::Reference< document::XDocumentInfo > SAL_CALL ChartModel
::getDocumentInfo() throw(uno::RuntimeException)
{
    //@todo
}

//-----------------------------------------------------------------
// document::XViewDataSupplier (optional interface)
//-----------------------------------------------------------------

        uno::Reference< container::XIndexAccess > SAL_CALL ChartModel
::getViewData() throw(uno::RuntimeException)
{
    //@todo
}

        void SAL_CALL ChartModel
::setViewData( const uno::Reference< container::XIndexAccess >& xData )
        throw(uno::RuntimeException)
{
    //m_bModified = sal_True;//@todo? is this really correct
    //@todo
}
*/

//-----------------------------------------------------------------
// ::com::sun::star::style::XStyleFamiliesSupplier
//-----------------------------------------------------------------
::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameAccess > SAL_CALL ChartModel
    ::getStyleFamilies()
        throw (uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    return m_pImplChartModel->GetStyleFamilies();
    // \--
}

//-----------------------------------------------------------------
// chart2::XModelDataProvider
//-----------------------------------------------------------------
//         Sequence< Reference< chart2::XDataSeries > > SAL_CALL ChartModel
// ::getDataSeries()
//         throw (uno::RuntimeException)
// {
//     OSL_ASSERT( m_pImplChartModel.get() != 0 );
//     // /--
//     MutexGuard aGuard( m_aModelMutex );
//     return m_pImplChartModel->GetDataSeries();
//     // \--
// }

//-----------------------------------------------------------------
// chart2::XChartDocument
//-----------------------------------------------------------------

        uno::Reference< chart2::XDiagram > SAL_CALL ChartModel
::getDiagram()
            throw (uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    try
    {
        return m_pImplChartModel->GetDiagram( 0 );
    }
    catch( container::NoSuchElementException )
    {
    }

    return uno::Reference< chart2::XDiagram >();
    // \--
}

        void SAL_CALL ChartModel
::setDiagram( const uno::Reference< chart2::XDiagram >& xDiagram )
            throw (lang::IllegalArgumentException,
                   uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    m_pImplChartModel->RemoveAllDiagrams();
    m_pImplChartModel->AppendDiagram( xDiagram );
    // \--
}

        void SAL_CALL ChartModel
::attachDataProvider( const uno::Reference< chart2::XDataProvider >& xProvider )
            throw (uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    m_pImplChartModel->SetDataProvider( xProvider );
    // \--
}

        void SAL_CALL ChartModel
::setRangeRepresentation( const ::rtl::OUString& aRangeRepresentation )
            throw (lang::IllegalArgumentException,
                   uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    m_pImplChartModel->SetRangeRepresentation( aRangeRepresentation );
    // \--
}

        void SAL_CALL ChartModel
::setChartTypeManager( const uno::Reference< chart2::XChartTypeManager >& xNewManager )
            throw (uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    m_pImplChartModel->SetChartTypeManager( xNewManager );
    // \--
}

        uno::Reference< chart2::XChartTypeManager > SAL_CALL ChartModel
::getChartTypeManager()
            throw (uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    return m_pImplChartModel->GetChartTypeManager();
    // \--
}

        void SAL_CALL ChartModel
::setChartTypeTemplate( const uno::Reference< chart2::XChartTypeTemplate >& xNewTemplate )
        throw (uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    m_pImplChartModel->SetChartTypeTemplate( xNewTemplate );
    // \--
}

        uno::Reference< chart2::XChartTypeTemplate > SAL_CALL ChartModel
::getChartTypeTemplate()
        throw (uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    return m_pImplChartModel->GetChartTypeTemplate();
    // \--
}

//         void SAL_CALL ChartModel
// ::setSplitLayoutContainer( const uno::Reference< layout::XSplitLayoutContainer >& xLayoutCnt )
//         throw (uno::RuntimeException)
// {
//     OSL_ASSERT( m_pImplChartModel.get() != 0 );
//     // /--
//     MutexGuard aGuard( m_aModelMutex );
//     m_pImplChartModel->SetSplitLayoutContainer( xLayoutCnt );
//     // \--
// }

//         uno::Reference< layout::XSplitLayoutContainer > SAL_CALL ChartModel
// ::getSplitLayoutContainer()
//         throw (uno::RuntimeException)
// {
//     OSL_ASSERT( m_pImplChartModel.get() != 0 );
//     // /--
//     MutexGuard aGuard( m_aModelMutex );
//     return m_pImplChartModel->GetSplitLayoutContainer();
//     // \--
// }



// ____ XTitled ____
uno::Reference< chart2::XTitle > SAL_CALL ChartModel::getTitle()
    throw (uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    return m_pImplChartModel->GetTitle();
    // \--
}

void SAL_CALL ChartModel::setTitle(
    const uno::Reference<
    chart2::XTitle >& Title )
    throw (uno::RuntimeException)
{
    OSL_ASSERT( m_pImplChartModel.get() != 0 );
    // /--
    MutexGuard aGuard( m_aModelMutex );
    m_pImplChartModel->SetTitle( Title );
    // \--
}

// ____ XInterface (for old API wrapper) ____
uno::Any SAL_CALL ChartModel::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException)
{
    // return old API wrapper
    if( aType ==
        ::getCppuType( (uno::Reference< ::com::sun::star::chart::XChartDocument > *)0 ))
    {
        return uno::makeAny( m_pImplChartModel->GetOldChartDocument( this ));
    }

    return impl::ChartModel_Base::queryInterface( aType );
}


}  // namespace chart
