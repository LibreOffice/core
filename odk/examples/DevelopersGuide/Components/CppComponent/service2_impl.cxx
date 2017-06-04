/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <uno/lbnames.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <my_module/XSomething.hpp>


using namespace ::rtl; // for OUString
using namespace ::com::sun::star; // for odk interfaces
using namespace ::com::sun::star::uno; // for basic types


namespace my_sc_impl
{

extern Sequence< OUString > SAL_CALL  getSupportedServiceNames_MyService1Impl();
extern OUString SAL_CALL getImplementationName_MyService1Impl();
extern Reference< XInterface > SAL_CALL create_MyService1Impl(
    Reference< XComponentContext > const & xContext );

static Sequence< OUString > getSupportedServiceNames_MyService2Impl()
{
    Sequence<OUString> names(1);
    names[0] = "my_module.MyService2";
    return names;
}

static OUString getImplementationName_MyService2Impl()
{
    return OUString("my_module.my_sc_implementation.MyService2");
}

class MyService2Impl : public ::cppu::WeakImplHelper3<
      ::my_module::XSomething, lang::XServiceInfo, lang::XInitialization >
{
    OUString m_sData;
    // it's good practise to store the context for further use when you use
    // other UNO API's in your implementation
    Reference< XComponentContext > m_xContext;
public:
    inline MyService2Impl(Reference< XComponentContext > const & xContext) throw ()
        : m_xContext(xContext)
        {}

    virtual ~MyService2Impl() {}

    // focus on three given interfaces,
    // no need to implement XInterface, XTypeProvider, XWeak

    // XInitialization will be called upon
    // createInstanceWithArguments[AndContext]()
    virtual void SAL_CALL initialize( Sequence< Any > const & args )
        throw (Exception);
    // XSomething
    virtual OUString SAL_CALL methodOne( OUString const & str )
        throw (RuntimeException);
    virtual OUString SAL_CALL methodTwo( )
        throw (RuntimeException);
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
};

// XInitialization implementation
void MyService2Impl::initialize( Sequence< Any > const & args )
    throw (Exception)
{
    if (args.getLength() != 1)
    {
        throw lang::IllegalArgumentException(
            OUString(
                          "give a string instantiating this component!"),
            // resolve to XInterface reference:
            static_cast< ::cppu::OWeakObject * >(this),
            0 ); // argument pos
    }
    if (! (args[ 0 ] >>= m_sData))
    {
        throw lang::IllegalArgumentException(
            OUString(
                          "no string given as argument!"),
            // resolve to XInterface reference:
            static_cast< ::cppu::OWeakObject * >(this),
            0 ); // argument pos
    }
}

// XSomething implementation
OUString MyService2Impl::methodOne( OUString const & str )
    throw (RuntimeException)
{
    m_sData = str;
    return OUString( "called methodOne() of MyService2 implementation: " ) + m_sData;
}

OUString MyService2Impl::methodTwo( )
    throw (RuntimeException)
{
    return OUString( "called methodTwo() of MyService2 implementation: " ) + m_sData;
}

// XServiceInfo implementation
OUString MyService2Impl::getImplementationName()
    throw (RuntimeException)
{
    // unique implementation name
    return OUString("my_module.my_sc_implementation.MyService2");
}

sal_Bool MyService2Impl::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    return cppu::supportsService(this, serviceName);
}

Sequence< OUString > MyService2Impl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return getSupportedServiceNames_MyService2Impl();
}

Reference< XInterface > SAL_CALL create_MyService2Impl(
    Reference< XComponentContext > const & xContext )
{
    return static_cast< ::cppu::OWeakObject * >( new MyService2Impl( xContext ) );
}

}

/* shared lib exports implemented without helpers in service_impl1.cxx */
namespace my_sc_impl
{
static const struct ::cppu::ImplementationEntry s_component_entries [] =
{
    {
        create_MyService1Impl, getImplementationName_MyService1Impl,
        getSupportedServiceNames_MyService1Impl,
        ::cppu::createSingleComponentFactory,
        0, 0
    },
    {
        create_MyService2Impl, getImplementationName_MyService2Impl,
        getSupportedServiceNames_MyService2Impl,
        ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    sal_Char const * implName, lang::XMultiServiceFactory * xMgr,
    registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_getFactoryHelper(
        implName, xMgr, xRegistry, ::my_sc_impl::s_component_entries );
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    char const ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
