/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unomodule.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 10:46:54 $
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

// System - Includes -----------------------------------------------------

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif

#include "sdmod.hxx"
#include "unomodule.hxx"
#include <sfx2/objface.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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
    return uno::Reference< frame::XDispatch >( new SdUnoModule( rSMgr ) );
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

void SAL_CALL SdUnoModule::addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL SdUnoModule::removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException )
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
REFERENCE< XDISPATCH > SAL_CALL SdUnoModule::queryDispatch( const UNOURL& aURL, const OUSTRING& sTargetFrameName, sal_Int32 eSearchFlags    ) throw( RUNTIMEEXCEPTION )
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

