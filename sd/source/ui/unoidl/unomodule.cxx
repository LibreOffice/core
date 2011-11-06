/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

// System - Includes -----------------------------------------------------
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

#include "sdmod.hxx"
#include "unomodule.hxx"
#include <sfx2/objface.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

::rtl::OUString SAL_CALL SdUnoModule_getImplementationName() throw( uno::RuntimeException )
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Draw.DrawingModule" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL SdUnoModule_getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSeq( 1 );
    aSeq[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ModuleDispatcher"));
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SdUnoModule_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    return uno::Reference< uno::XInterface >( static_cast< cppu::OWeakObject* >( new SdUnoModule( rSMgr ) ) );
}

    // XNotifyingDispatch
void SAL_CALL SdUnoModule::dispatchWithNotification( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    // there is no guarantee, that we are holded alive during this method!
    // May the outside dispatch container will be updated by a CONTEXT_CHANGED
    // asynchronous ...
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xThis(static_cast< ::com::sun::star::frame::XNotifyingDispatch* >(this));

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SdDLL::Init();
    const SfxSlot* pSlot = SD_MOD()->GetInterface()->GetSlot( aURL.Complete );

    sal_Int16 aState = ::com::sun::star::frame::DispatchResultState::DONTKNOW;
    if ( !pSlot )
        aState = ::com::sun::star::frame::DispatchResultState::FAILURE;
    else
    {
        SfxRequest aReq( pSlot, aArgs, SFX_CALLMODE_SYNCHRON, SD_MOD()->GetPool() );
        const SfxPoolItem* pResult = SD_MOD()->ExecuteSlot( aReq );
        if ( pResult )
            aState = ::com::sun::star::frame::DispatchResultState::SUCCESS;
        else
            aState = ::com::sun::star::frame::DispatchResultState::FAILURE;
    }

    if ( xListener.is() )
    {
        xListener->dispatchFinished(
            ::com::sun::star::frame::DispatchResultEvent(
                    xThis, aState, ::com::sun::star::uno::Any()));
    }
}
    // XDispatch
void SAL_CALL SdUnoModule::dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw( ::com::sun::star::uno::RuntimeException )
{
    dispatchWithNotification(aURL, aArgs, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >());
}

void SAL_CALL SdUnoModule::addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > &, const ::com::sun::star::util::URL&) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL SdUnoModule::removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > &, const ::com::sun::star::util::URL&) throw( ::com::sun::star::uno::RuntimeException )
{
}

SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL SdUnoModule::queryDispatches( const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescripts ) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Int32 nCount = seqDescripts.getLength();
    SEQUENCE< REFERENCE< XDISPATCH > > lDispatcher( nCount );

    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch( seqDescripts[i].FeatureURL  ,
                                        seqDescripts[i].FrameName   ,
                                        seqDescripts[i].SearchFlags );
    }

    return lDispatcher;
}

// XDispatchProvider
REFERENCE< XDISPATCH > SAL_CALL SdUnoModule::queryDispatch( const UNOURL& aURL, const OUSTRING&, sal_Int32 ) throw( RUNTIMEEXCEPTION )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SdDLL::Init();
    const SfxSlot* pSlot = SD_MOD()->GetInterface()->GetSlot( aURL.Complete );

    REFERENCE< XDISPATCH > xSlot;
    if ( pSlot )
        xSlot = this;

    return xSlot;
}

// XServiceInfo
::rtl::OUString SAL_CALL SdUnoModule::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return SdUnoModule_getImplementationName();
}

sal_Bool SAL_CALL SdUnoModule::supportsService( const ::rtl::OUString& sServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    UNOSEQUENCE< UNOOUSTRING >  seqServiceNames =   getSupportedServiceNames();
    const UNOOUSTRING*          pArray          =   seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SdUnoModule::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return SdUnoModule_getSupportedServiceNames();
}

