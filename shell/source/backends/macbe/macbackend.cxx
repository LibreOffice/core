/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macbackend.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:56:19 $
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
#include "precompiled_shell.hxx"

#include "macbackend.hxx"
#include "macbelayer.hxx"

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_COMPONENTCHANGEEVENT_HPP_
#include <com/sun/star/configuration/backend/ComponentChangeEvent.hpp>
#endif

#ifndef _UNO_CURRENT_CONTEXT_HXX_
#include <uno/current_context.hxx>
#endif

#define MACBE_INET_COMPONENT    "org.openoffice.Inet"
#define MACBE_PATHS_COMPONENT   "org.openoffice.Office.Paths"

MacOSXBackend::MacOSXBackend(const uno::Reference<uno::XComponentContext>& xContext)
        throw (backend::BackendAccessException) :
        ::cppu::WeakImplHelper2 < backend::XSingleLayerStratum, lang::XServiceInfo > (),
        m_xContext(xContext)
{
}

//------------------------------------------------------------------------------

MacOSXBackend::~MacOSXBackend(void)
{
}

//------------------------------------------------------------------------------

MacOSXBackend* MacOSXBackend::createInstance(const uno::Reference<uno::XComponentContext>& xContext)
{
    return new MacOSXBackend(xContext);
}

// ---------------------------------------------------------------------------------------

uno::Reference<backend::XLayer> SAL_CALL MacOSXBackend::getLayer(const rtl::OUString& aComponent, const rtl::OUString& /*aTimestamp*/)
    throw (backend::BackendAccessException, lang::IllegalArgumentException)
{
    if( aComponent.equalsAscii( MACBE_INET_COMPONENT ) )
    {
        if( ! m_xSystemLayer.is() )
            m_xSystemLayer = new MacOSXLayer( m_xContext );

        return m_xSystemLayer;
    }
    else if( aComponent.equalsAscii( MACBE_PATHS_COMPONENT ) )
    {
        if( ! m_xPathLayer.is() )
            m_xPathLayer = new MacOSXPathLayer( m_xContext );
        return m_xPathLayer;
    }

    return uno::Reference<backend::XLayer>();
}

//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> SAL_CALL
    MacOSXBackend::getUpdatableLayer(const rtl::OUString& /*aComponent*/)
    throw (backend::BackendAccessException,lang::NoSupportException,
           lang::IllegalArgumentException)
{
    throw lang::NoSupportException(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                           "MacOSXBackend: No Update Operation allowed, Read Only access") ),
        *this);

    return NULL;
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL MacOSXBackend::getBackendName(void)
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.configuration.backend.MacOSXBackend");
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL MacOSXBackend::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getBackendName();
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL MacOSXBackend::getBackendServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServiceNameList(2);
    aServiceNameList[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.MacOSXBackend"));
    aServiceNameList[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.PlatformBackend"));

    return aServiceNameList;
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL MacOSXBackend::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getBackendServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;

    return false;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL MacOSXBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getBackendServiceNames();
}

// ---------------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL MacOSXBackend::getSupportedComponents(void)
{
    uno::Sequence<rtl::OUString> aSupportedComponentList(2);
    aSupportedComponentList[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( MACBE_INET_COMPONENT ) );
    aSupportedComponentList[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( MACBE_PATHS_COMPONENT ) );

    return aSupportedComponentList;
}

