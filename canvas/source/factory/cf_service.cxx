/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cf_service.cxx,v $
 * $Revision: 1.12 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"
#include "osl/mutex.hxx"
#include "osl/process.h"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase3.hxx"
#include "vcl/configsettings.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/container/XContentEnumerationAccess.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"

#include <vector>
#include <algorithm>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define ARLEN(x) (sizeof (x) / sizeof *(x))


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace {

OUString SAL_CALL getImplName()
{
    return OUSTR("com.sun.star.comp.rendering.CanvasFactory");
}

Sequence<OUString> SAL_CALL getSuppServices()
{
    OUString name = OUSTR("com.sun.star.rendering.CanvasFactory");
    return Sequence<OUString>(&name, 1);
}

//==============================================================================
class CanvasFactory
    : public ::cppu::WeakImplHelper3< lang::XServiceInfo,
                                      lang::XMultiComponentFactory,
                                      lang::XMultiServiceFactory >
{
    ::osl::Mutex m_mutex;
    Reference<XComponentContext> m_xContext;
    Sequence<OUString> m_services;
    OUString m_serviceName;
    Reference<lang::XSingleComponentFactory> m_xFactory;

    Reference<XInterface> use(
        Reference<lang::XSingleComponentFactory> const & xFactory,
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xContext );
    Reference<XInterface> lookupAndUse(
        OUString const & serviceName, Sequence<Any> const & args,
        Reference<XComponentContext> const & xContext );

public:
    virtual ~CanvasFactory();
    CanvasFactory( Reference<XComponentContext> const & xContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

    // XMultiComponentFactory
    virtual Sequence<OUString> SAL_CALL getAvailableServiceNames()
        throw (RuntimeException);
    virtual Reference<XInterface> SAL_CALL createInstanceWithContext(
        OUString const & name,
        Reference<XComponentContext> const & xContext ) throw (Exception);
    virtual Reference<XInterface> SAL_CALL
    createInstanceWithArgumentsAndContext(
        OUString const & name,
        Sequence<Any> const & args,
        Reference<XComponentContext> const & xContext ) throw (Exception);

    // XMultiServiceFactory
    virtual Reference<XInterface> SAL_CALL createInstance(
        OUString const & name )
        throw (Exception);
    virtual Reference<XInterface> SAL_CALL createInstanceWithArguments(
        OUString const & name, Sequence<Any> const & args )
        throw (Exception);
//     virtual Sequence<OUString> SAL_CALL getAvailableServiceNames()
//         throw (RuntimeException);
};

CanvasFactory::CanvasFactory(
    Reference<XComponentContext> const & xContext )
    : m_xContext(xContext)
{
    ::rtl::OUString preferredServices;

    try
    {
        // read out configuration for preferred services:
        Reference<lang::XMultiServiceFactory> xConfigProvider(
            m_xContext->getServiceManager()->createInstanceWithContext(
                OUSTR("com.sun.star.configuration.ConfigurationProvider"),
                m_xContext ), UNO_QUERY_THROW );

        Any propValue(
            makeAny( beans::PropertyValue(
                         OUSTR("nodepath"), -1,
                         makeAny( OUSTR("/org.openoffice.VCL/Settings/Canvas") ),
                         beans::PropertyState_DIRECT_VALUE ) ) );

        Reference<container::XNameAccess> xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                OUSTR("com.sun.star.configuration.ConfigurationAccess"),
                Sequence<Any>( &propValue, 1 ) ), UNO_QUERY );

        if (xNameAccess.is())
            xNameAccess->getByName( OUSTR("PreferredServices") ) >>= preferredServices;
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception & exc) {
        (void) exc;
    }

    ::std::vector< ::rtl::OUString > services;
    sal_Int32 tokenPos = 0;
    do
    {
        ::rtl::OUString oneService = preferredServices.getToken( 0, ';', tokenPos );
        if ( oneService.getLength() )
            services.push_back( oneService );
    }
    while ( tokenPos > 0 );
    m_services.realloc( services.size() );
    ::std::copy( services.begin(), services.end(), m_services.getArray() );

    // append the usual preferred ones:
    sal_Int32 pos = m_services.getLength();
#if defined WNT
    m_services.realloc( pos + 6 );
    m_services[ pos++ ] = OUSTR("com.sun.star.rendering.DX9Canvas");
    m_services[ pos++ ] = OUSTR("com.sun.star.rendering.DXCanvas");
    m_services[ pos++ ] = OUSTR("com.sun.star.rendering.GLCanvas");
#else
    m_services.realloc( pos + 4 );
    m_services[ pos++ ] = OUSTR("com.sun.star.rendering.GLCanvas");
#endif
    m_services[ pos++ ] = OUSTR("com.sun.star.rendering.CairoCanvas");
    m_services[ pos++ ] = OUSTR("com.sun.star.rendering.JavaCanvas");
    m_services[ pos   ] = OUSTR("com.sun.star.rendering.VCLCanvas");
}

CanvasFactory::~CanvasFactory()
{
}

//------------------------------------------------------------------------------
Reference<XInterface> create( Reference<XComponentContext> const & xContext )
{
    return static_cast< ::cppu::OWeakObject * >(
        new CanvasFactory( xContext ) );
}

// XServiceInfo
//______________________________________________________________________________
OUString CanvasFactory::getImplementationName() throw (RuntimeException)
{
    return getImplName();
}

//______________________________________________________________________________
sal_Bool CanvasFactory::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    return serviceName.equals(getSuppServices()[0]);
}

//______________________________________________________________________________
Sequence<OUString> CanvasFactory::getSupportedServiceNames()
    throw (RuntimeException)
{
    return getSuppServices();
}

// XMultiComponentFactory
//______________________________________________________________________________
Sequence<OUString> CanvasFactory::getAvailableServiceNames()
    throw (RuntimeException)
{
    return m_services;
}

//______________________________________________________________________________
Reference<XInterface> CanvasFactory::createInstanceWithContext(
    OUString const & name, Reference<XComponentContext> const & xContext )
    throw (Exception)
{
    return createInstanceWithArgumentsAndContext(
        name, Sequence<Any>(), xContext );
}

//______________________________________________________________________________
Reference<XInterface> CanvasFactory::use(
    Reference<lang::XSingleComponentFactory> const & xFactory,
    Sequence<Any> const & args, Reference<XComponentContext> const & xContext )
{
    try {
        return xFactory->createInstanceWithArgumentsAndContext(args, xContext);
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception &) {
        return Reference<XInterface>();
    }
}

//______________________________________________________________________________
Reference<XInterface> CanvasFactory::lookupAndUse(
    OUString const & serviceName, Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext )
{
    {
        // try to reuse:
        ::osl::ClearableMutexGuard guard(m_mutex);
        if (m_serviceName.equals(serviceName)) {
            Reference<lang::XSingleComponentFactory> xFac(m_xFactory);
            guard.clear();
            Reference<XInterface> xCanvas( use( xFac, args, xContext ) );
            if (xCanvas.is())
                return xCanvas;
        }
    }

    Reference<container::XContentEnumerationAccess> xEnumAccess(
        // use service manager of factory:
        m_xContext->getServiceManager(), UNO_QUERY_THROW );
    Reference<container::XEnumeration> xEnum(
        xEnumAccess->createContentEnumeration( serviceName ) );
    if (xEnum.is()) {
        while (xEnum->hasMoreElements()) {
            Reference<lang::XSingleComponentFactory> xFactory(
                xEnum->nextElement(), UNO_QUERY );
            OSL_ASSERT( xFactory.is() );
            if (xFactory.is()) {
                Reference<XInterface> xCanvas(
                    use( xFactory, args, xContext ) );
                if (xCanvas.is()) {
                    ::osl::MutexGuard guard(m_mutex);
                    // init for reuse:
                    m_xFactory.set(xFactory);
                    m_serviceName = serviceName;
                    return xCanvas;
                }
            }
        }
    }

    return Reference<XInterface>();
}

//______________________________________________________________________________
Reference<XInterface> CanvasFactory::createInstanceWithArgumentsAndContext(
    OUString const & preferredOne, Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext ) throw (Exception)
{
    // preferred one overrides previously used one:
    if (preferredOne.getLength() > 0) {
        Reference<XInterface> xCanvas(
            lookupAndUse( preferredOne, args, xContext ) );
        if (xCanvas.is())
            return xCanvas;
    }

    {
        // try to reuse previously installed factory:
        ::osl::ClearableMutexGuard guard(m_mutex);
        if (m_xFactory.is()) {
            Reference<lang::XSingleComponentFactory> xFac(m_xFactory);
            guard.clear();
            Reference<XInterface> xCanvas( use( xFac, args, xContext ) );
            if (xCanvas.is())
                return xCanvas;
        }
    }

    // try configured ones:
    OUString const * pservices = m_services.getConstArray();
    sal_Int32 pos = 0, len = m_services.getLength();
    for ( ; pos < len; ++pos ) {
        Reference<XInterface> xCanvas(
            lookupAndUse( pservices[pos], args, xContext ) );
        if (xCanvas.is())
            return xCanvas;
    }

    OSL_ENSURE( 0, "### no canvas available!?" );
    return Reference<XInterface>();
}

// XMultiServiceFactory
//______________________________________________________________________________
Reference<XInterface> CanvasFactory::createInstance( OUString const & name )
    throw (Exception)
{
    return createInstanceWithArgumentsAndContext(
        name, Sequence<Any>(), m_xContext );
}

//______________________________________________________________________________
Reference<XInterface> CanvasFactory::createInstanceWithArguments(
    OUString const & name, Sequence<Any> const & args ) throw (Exception)
{
    return createInstanceWithArgumentsAndContext(
        name, args, m_xContext );
}

const ::cppu::ImplementationEntry s_entries [] = {
    {
        create,
        getImplName,
        getSuppServices,
        ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

} // anon namespace

extern "C" {

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo(
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return ::cppu::component_writeInfoHelper(
        pServiceManager, pRegistryKey, s_entries );
}

void * SAL_CALL component_getFactory(
    sal_Char const * pImplName,
    lang::XMultiServiceFactory * pServiceManager,
    registry::XRegistryKey * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, s_entries );
}

}

