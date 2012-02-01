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



/*************************************************************************
 *************************************************************************
 *
 * service implementation:   foo.Counter
 * exported interfaces:      foo.XCounter
 *
 * simple example component implementing a counter
 *
 *************************************************************************
 *************************************************************************/

#include <stdio.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/queryinterface.hxx> // helper for queryInterface() impl
#include <cppuhelper/factory.hxx> // helper for component factory
// generated c++ interfaces
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <foo/XCountable.hpp>

#define SERVICENAME "foo.Counter"
#define IMPLNAME    "com.sun.star.comp.example.cpp.Counter"

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::foo;


//========================================================================
class MyCounterImpl
    : public XCountable
    , public XServiceInfo
{
    // to obtain other services if needed
    Reference< XMultiServiceFactory > m_xServiceManager;

    sal_Int32 m_nRefCount;
    sal_Int32 m_nCount;

public:
    MyCounterImpl( const Reference< XMultiServiceFactory > & xServiceManager )
        : m_xServiceManager( xServiceManager ), m_nRefCount( 0 )
        { printf( "< MyCounterImpl ctor called >\n" ); }
    ~MyCounterImpl()
        { printf( "< MyCounterImpl dtor called >\n" ); }

    // XInterface implementation
    virtual void SAL_CALL acquire() throw ()
        { ++m_nRefCount; }
    virtual void SAL_CALL release() throw ()
        { if (! --m_nRefCount) delete this; }
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw (RuntimeException)
        { return cppu::queryInterface(rType,
                                      static_cast< XInterface* >( static_cast< XServiceInfo* >( this ) ),
                                      static_cast< XCountable* >( this ),
                                      static_cast< XServiceInfo* >( this ) ); }

    // XServiceInfo implementation
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XCountable implementation
    virtual sal_Int32 SAL_CALL getCount() throw (RuntimeException)
        { return m_nCount; }
    virtual void SAL_CALL setCount( sal_Int32 nCount ) throw (RuntimeException)
        { m_nCount = nCount; }
    virtual sal_Int32 SAL_CALL increment() throw (RuntimeException)
        { return (++m_nCount); }
    virtual sal_Int32 SAL_CALL decrement() throw (RuntimeException)
        { return (--m_nCount); }
};

//*************************************************************************
OUString SAL_CALL MyCounterImpl::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}

//*************************************************************************
sal_Bool SAL_CALL MyCounterImpl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL MyCounterImpl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL MyCounterImpl::getSupportedServiceNames_Static(  )
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
}




/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL MyCounterImpl_create(
    const Reference< XMultiServiceFactory > & xMgr )
{
    return Reference<XInterface>(static_cast<XCountable*>(new MyCounterImpl(xMgr)));
}


//#########################################################################
//#### EXPORTED ###########################################################
//#########################################################################


/**
 * Gives the environment this component belongs to.
 */
extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/**
 * This function creates an implementation section in the registry and another subkey
 *
 * for each supported service.
 * @param pServiceManager   the service manager
 * @param pRegistryKey      the registry key
 */
// extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(void * pServiceManager, void * pRegistryKey)
// {
//  sal_Bool result = sal_False;

//  if (pRegistryKey)
//  {
//      try
//      {
//          Reference< XRegistryKey > xNewKey(
//              reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
//                  OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

//          const Sequence< OUString > & rSNL =
//              MyCounterImpl::getSupportedServiceNames_Static();
//          const OUString * pArray = rSNL.getConstArray();
//          for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
//              xNewKey->createKey( pArray[nPos] );

//          return sal_True;
//      }
//      catch (InvalidRegistryException &)
//      {
//          // we should not ignore exceptions
//      }
//  }
//  return result;
// }

/**
 * This function is called to get service factories for an implementation.
 *
 * @param pImplName       name of implementation
 * @param pServiceManager a service manager, need for component creation
 * @param pRegistryKey    the registry key for this component, need for persistent data
 * @return a component factory
 */
extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey)
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            MyCounterImpl_create,
            MyCounterImpl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
