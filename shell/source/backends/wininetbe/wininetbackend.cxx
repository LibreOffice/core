/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wininetbackend.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:49:56 $
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

#include "wininetbackend.hxx"
#include "wininetlayer.hxx"

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_COMPONENTCHANGEEVENT_HPP_
#include <com/sun/star/configuration/backend/ComponentChangeEvent.hpp>
#endif

#ifndef _UNO_CURRENT_CONTEXT_HXX_
#include <uno/current_context.hxx>
#endif

#define WININET_DLL_NAME "wininet.dll"

WinInetBackend::WinInetBackend(const uno::Reference<uno::XComponentContext>& xContext)
    throw (backend::BackendAccessException) :
    ::cppu::WeakImplHelper2 < backend::XSingleLayerStratum, lang::XServiceInfo > (),
    m_xContext(xContext)
{
    m_hWinInetDll = LoadLibrary( WININET_DLL_NAME );
}

//------------------------------------------------------------------------------

WinInetBackend::~WinInetBackend(void)
{
    if ( m_hWinInetDll )
        FreeLibrary( m_hWinInetDll );
}

//------------------------------------------------------------------------------

WinInetBackend* WinInetBackend::createInstance(
    const uno::Reference<uno::XComponentContext>& xContext
)
{
    return new WinInetBackend(xContext);
}

// ---------------------------------------------------------------------------------------

uno::Reference<backend::XLayer> SAL_CALL WinInetBackend::getLayer(
        const rtl::OUString& aComponent, const rtl::OUString& aTimestamp)
    throw (backend::BackendAccessException, lang::IllegalArgumentException)
{

    if( aComponent.equals( getSupportedComponents()[0]) )
    {
        if( ! m_xSystemLayer.is() && m_hWinInetDll )
        {
            WinInetLayer::InternetQueryOption_Proc_T lpfnInternetQueryOption =
                reinterpret_cast< WinInetLayer::InternetQueryOption_Proc_T >(
                    GetProcAddress( m_hWinInetDll, "InternetQueryOptionA" ) );

            if( lpfnInternetQueryOption )
                m_xSystemLayer = new WinInetLayer(lpfnInternetQueryOption, m_xContext);
        }

        return m_xSystemLayer;
    }

    return uno::Reference<backend::XLayer>();
}

//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> SAL_CALL
WinInetBackend::getUpdatableLayer(const rtl::OUString& aComponent)
    throw (backend::BackendAccessException,lang::NoSupportException,
           lang::IllegalArgumentException)
{
    throw lang::NoSupportException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
            "WinInetBackend: No Update Operation allowed, Read Only access") ),
        *this) ;

    return NULL;
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL WinInetBackend::getBackendName(void) {
    return rtl::OUString::createFromAscii("com.sun.star.comp.configuration.backend.WinInetBackend") ;
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL WinInetBackend::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getBackendName() ;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL WinInetBackend::getBackendServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServiceNameList(2);
    aServiceNameList[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.WinInetBackend")) ;
    aServiceNameList[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.PlatformBackend")) ;

    return aServiceNameList ;
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL WinInetBackend::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getBackendServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;

    return false;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL WinInetBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getBackendServiceNames() ;
}

// ---------------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL WinInetBackend::getSupportedComponents(void)
{
    uno::Sequence<rtl::OUString> aSupportedComponentList(1);
    aSupportedComponentList[0] = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet" )
    );

    return aSupportedComponentList;
}

