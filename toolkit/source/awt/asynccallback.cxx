/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asynccallback.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-27 11:43:50 $
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
#include "precompiled_toolkit.hxx"

#include "vcl/svapp.hxx"
#include "vos/mutex.hxx"
#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/implbase2.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/awt/XRequestCallback.hpp"


// component helper namespace
namespace comp_AsyncCallback {

namespace css = ::com::sun::star;

// component and service helper functions:
::rtl::OUString SAL_CALL _getImplementationName();
css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames();
css::uno::Reference< css::uno::XInterface > SAL_CALL _create( css::uno::Reference< css::uno::XComponentContext > const & context );

} // closing component helper namespace



/// anonymous implementation namespace
namespace {

namespace css = ::com::sun::star;

class AsyncCallback:
    public ::cppu::WeakImplHelper2<
        css::lang::XServiceInfo,
        css::awt::XRequestCallback>
{
public:
    explicit AsyncCallback(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::awt::XRequestCallback:
    virtual void SAL_CALL addCallback(const css::uno::Reference< css::awt::XCallback > & xCallback, const ::com::sun::star::uno::Any & aData) throw (css::uno::RuntimeException);

private:

    struct CallbackData
    {
        CallbackData( const css::uno::Reference< css::awt::XCallback >& rCallback, const css::uno::Any& rAny ) :
            xCallback( rCallback ), aData( rAny ) {}

        css::uno::Reference< css::awt::XCallback > xCallback;
        css::uno::Any                              aData;
    };

    DECL_STATIC_LINK( AsyncCallback, Notify_Impl, CallbackData* );

    AsyncCallback(AsyncCallback &); // not defined
    void operator =(AsyncCallback &); // not defined

    virtual ~AsyncCallback() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

AsyncCallback::AsyncCallback(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL AsyncCallback::getImplementationName() throw (css::uno::RuntimeException)
{
    return comp_AsyncCallback::_getImplementationName();
}

::sal_Bool SAL_CALL AsyncCallback::supportsService(::rtl::OUString const & serviceName) throw (css::uno::RuntimeException)
{
    const css::uno::Sequence< ::rtl::OUString > serviceNames = comp_AsyncCallback::_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL AsyncCallback::getSupportedServiceNames() throw (css::uno::RuntimeException)
{
    return comp_AsyncCallback::_getSupportedServiceNames();
}

// ::com::sun::star::awt::XRequestCallback:
void SAL_CALL AsyncCallback::addCallback(const css::uno::Reference< css::awt::XCallback > & xCallback, const ::com::sun::star::uno::Any & aData) throw (css::uno::RuntimeException)
{
    if ( Application::IsInMain() )
    {
        osl::Guard< vos::IMutex > aSolarGuard( Application::GetSolarMutex() );
        CallbackData* pCallbackData = new CallbackData( xCallback, aData );
        Application::PostUserEvent( STATIC_LINK( this, AsyncCallback, Notify_Impl ), pCallbackData );
    }
}

// private asynchronous link to call reference to the callback object
IMPL_STATIC_LINK_NOINSTANCE( AsyncCallback, Notify_Impl, CallbackData*, pCallbackData )
{
    try
    {
        // Asynchronous execution
        // Check pointer and reference before!
        if ( pCallbackData && pCallbackData->xCallback.is() )
            pCallbackData->xCallback->notify( pCallbackData->aData );
    }
    catch ( css::uno::Exception& )
    {
    }

    delete pCallbackData;
    return 0;
}

} // closing anonymous implementation namespace



// component helper namespace
namespace comp_AsyncCallback {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.awt.comp.AsyncCallback"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.awt.AsyncCallback"));
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new AsyncCallback(context));
}

} // closing component helper namespace

static ::cppu::ImplementationEntry const entries[] = {
    { &comp_AsyncCallback::_create,
      &comp_AsyncCallback::_getImplementationName,
      &comp_AsyncCallback::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

void * SAL_CALL comp_AsyncCallback_component_getFactory(
    const char * implName, void * serviceManager, void * registryKey)
{
    return ::cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

sal_Bool SAL_CALL comp_AsyncCallback_component_writeInfo(
    void * serviceManager, void * registryKey)
{
    return ::cppu::component_writeInfoHelper(serviceManager, registryKey, entries);
}
