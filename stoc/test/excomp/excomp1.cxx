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
#include "precompiled_stoc.hxx"
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/alloc.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>

#include <example/XTest.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace example;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace osl;
using namespace rtl;

#define SERVICENAME1 "example.ExampleComponent1"
#define IMPLNAME1   "example.ExampleComponent1.Impl"

namespace excomp_impl {

//*************************************************************************
// ExampleComponent1Impl
//*************************************************************************
class ExampleComponent1Impl : public WeakImplHelper2< XTest, XServiceInfo >
{
public:
    ExampleComponent1Impl( const Reference<XMultiServiceFactory> & rXSMgr );

    ~ExampleComponent1Impl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XSimpleRegistry
    virtual OUString SAL_CALL getMessage() throw(RuntimeException);

protected:
    Mutex       m_mutex;

    Reference<XMultiServiceFactory> m_xSMgr;
};

//*************************************************************************
ExampleComponent1Impl::ExampleComponent1Impl( const Reference<XMultiServiceFactory> & rXSMgr )
    : m_xSMgr(rXSMgr)
{
}

//*************************************************************************
ExampleComponent1Impl::~ExampleComponent1Impl()
{
}

//*************************************************************************
OUString SAL_CALL ExampleComponent1Impl::getImplementationName(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME1) );
}

//*************************************************************************
sal_Bool SAL_CALL ExampleComponent1Impl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL ExampleComponent1Impl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL ExampleComponent1Impl::getSupportedServiceNames_Static(  )
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME1) );
    return Sequence< OUString >( &aName, 1 );
}

//*************************************************************************
OUString SAL_CALL ExampleComponent1Impl::getMessage() throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString::createFromAscii("Lalelu nur der Mann im Mond schaut zu ...");
}


//*************************************************************************
Reference<XInterface> SAL_CALL ExampleComponent1_CreateInstance( const Reference<XMultiServiceFactory>& rSMgr )
{
    Reference<XInterface> xRet;

    XTest *pXTest = (XTest*) new ExampleComponent1Impl(rSMgr);

    if (pXTest)
    {
        xRet = Reference< XInterface >::query(pXTest);
    }

    return xRet;
}

} // excomp_impl


extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * /* pServiceManager */ , void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            // ExampleComponent1
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME1 "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL =
                ::excomp_impl::ExampleComponent1Impl::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLNAME1 ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME1) ),
            ::excomp_impl::ExampleComponent1_CreateInstance,
            ::excomp_impl::ExampleComponent1Impl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}



