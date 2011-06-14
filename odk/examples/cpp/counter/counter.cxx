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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
