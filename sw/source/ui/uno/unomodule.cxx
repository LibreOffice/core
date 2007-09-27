/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unomodule.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:43:28 $
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
#include "precompiled_sw.hxx"
// System - Includes -----------------------------------------------------

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif

#include "swmodule.hxx"
#include "swdll.hxx"
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

::rtl::OUString SAL_CALL SwUnoModule_getImplementationName() throw( uno::RuntimeException )
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Writer.WriterModule" ) );
}

uno::Sequence< rtl::OUString > SAL_CALL SwUnoModule_getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSeq( 1 );
    aSeq[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.ModuleDispatcher"));
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL SwUnoModule_createInstance(
                const uno::Reference< lang::XMultiServiceFactory > & rSMgr )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    return uno::Reference< uno::XInterface >( dynamic_cast< frame::XDispatch * >(new SwUnoModule( rSMgr )), uno::UNO_QUERY );
}

    // XNotifyingDispatch
void SAL_CALL SwUnoModule::dispatchWithNotification( const util::URL& aURL, const uno::Sequence< beans::PropertyValue >& aArgs, const uno::Reference< frame::XDispatchResultListener >& xListener ) throw (uno::RuntimeException)
{
    // there is no guarantee, that we are holded alive during this method!
    // May the outside dispatch container will be updated by a CONTEXT_CHANGED
    // asynchronous ...
    uno::Reference< uno::XInterface > xThis(static_cast< frame::XNotifyingDispatch* >(this));

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SwDLL::Init();
    const SfxSlot* pSlot = SW_MOD()->GetInterface()->GetSlot( aURL.Complete );

    sal_Int16 aState = frame::DispatchResultState::DONTKNOW;
    if ( !pSlot )
        aState = frame::DispatchResultState::FAILURE;
    else
    {
        SfxRequest aReq( pSlot, aArgs, SFX_CALLMODE_SYNCHRON, SW_MOD()->GetPool() );
        const SfxPoolItem* pResult = SW_MOD()->ExecuteSlot( aReq );
        if ( pResult )
            aState = frame::DispatchResultState::SUCCESS;
        else
            aState = frame::DispatchResultState::FAILURE;
    }

    if ( xListener.is() )
    {
        xListener->dispatchFinished(
            frame::DispatchResultEvent(
                    xThis, aState, uno::Any()));
    }
}

    // XDispatch
void SAL_CALL SwUnoModule::dispatch( const util::URL& aURL, const uno::Sequence< beans::PropertyValue >& aArgs ) throw( uno::RuntimeException )
{
    dispatchWithNotification(aURL, aArgs, uno::Reference< frame::XDispatchResultListener >());
}

void SAL_CALL SwUnoModule::addStatusListener(
    const uno::Reference< frame::XStatusListener > & /*xControl*/,
    const util::URL& /*aURL*/)
    throw( uno::RuntimeException )
{
}

void SAL_CALL SwUnoModule::removeStatusListener(
    const uno::Reference< frame::XStatusListener > & /*xControl*/,
    const util::URL& /*aURL*/) throw( uno::RuntimeException )
{
}

SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL SwUnoModule::queryDispatches(
    const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescripts ) throw( uno::RuntimeException )
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
REFERENCE< XDISPATCH > SAL_CALL SwUnoModule::queryDispatch(
    const UNOURL& aURL, const OUSTRING& /*sTargetFrameName*/,
    sal_Int32 /*eSearchFlags*/    ) throw( uno::RuntimeException )
{
    REFERENCE< XDISPATCH > xReturn;

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SwDLL::Init();
    const SfxSlot* pSlot = SW_MOD()->GetInterface()->GetSlot( aURL.Complete );
    if ( pSlot )
        xReturn = REFERENCE< XDISPATCH >(static_cast< XDISPATCH* >(this), uno::UNO_QUERY);

    return xReturn;
}

// XServiceInfo
::rtl::OUString SAL_CALL SwUnoModule::getImplementationName(  ) throw(uno::RuntimeException)
{
    return SwUnoModule_getImplementationName();
}

sal_Bool SAL_CALL SwUnoModule::supportsService( const ::rtl::OUString& sServiceName ) throw(uno::RuntimeException)
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

uno::Sequence< ::rtl::OUString > SAL_CALL SwUnoModule::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return SwUnoModule_getSupportedServiceNames();
}

