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



#include <cppuhelper/implbase3.hxx> // "3" implementing three interfaces
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

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
    Reference< XComponentContext > const & xContext )
    SAL_THROW( () );

static Sequence< OUString > getSupportedServiceNames_MyService2Impl()
{
    Sequence<OUString> names(1);
    names[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("my_module.MyService2"));
    return names;
}

static OUString getImplementationName_MyService2Impl()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
                         "my_module.my_sc_implementation.MyService2") );
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

// XInitialization implemention
void MyService2Impl::initialize( Sequence< Any > const & args )
    throw (Exception)
{
    if (args.getLength() != 1)
    {
        throw lang::IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                          "give a string instanciating this component!") ),
            // resolve to XInterface reference:
            static_cast< ::cppu::OWeakObject * >(this),
            0 ); // argument pos
    }
    if (! (args[ 0 ] >>= m_sData))
    {
        throw lang::IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                          "no string given as argument!") ),
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
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "called methodOne() of MyService2 implementation: ") ) + m_sData;
}

OUString MyService2Impl::methodTwo( )
    throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "called methodTwo() of MyService2 implementation: ") ) + m_sData;
}

// XServiceInfo implementation
OUString MyService2Impl::getImplementationName()
    throw (RuntimeException)
{
    // unique implementation name
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
                         "my_module.my_sc_implementation.MyService2") );
}

sal_Bool MyService2Impl::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    // this object only supports one service, so the test is simple
    return serviceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                         "my_module.MyService2") );
}

Sequence< OUString > MyService2Impl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return getSupportedServiceNames_MyService2Impl();
}

Reference< XInterface > SAL_CALL create_MyService2Impl(
    Reference< XComponentContext > const & xContext )
    SAL_THROW( () )
{
    return static_cast< ::cppu::OWeakObject * >( new MyService2Impl( xContext ) );
}

}

/* shared lib exports implemented without helpers in service_impl1.cxx */
namespace my_sc_impl
{
static struct ::cppu::ImplementationEntry s_component_entries [] =
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
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    sal_Char const ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// This method not longer necessary since OOo 3.4 where the component registration was
// was changed to passive component registration. For more details see
// http://wiki.services.openoffice.org/wiki/Passive_Component_Registration
//
// sal_Bool SAL_CALL component_writeInfo(
//     lang::XMultiServiceFactory * xMgr, registry::XRegistryKey * xRegistry )
// {
//     return ::cppu::component_writeInfoHelper(
//         xMgr, xRegistry, ::my_sc_impl::s_component_entries );
// }


SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    sal_Char const * implName, lang::XMultiServiceFactory * xMgr,
    registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_getFactoryHelper(
        implName, xMgr, xRegistry, ::my_sc_impl::s_component_entries );
}

}


