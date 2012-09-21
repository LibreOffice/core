/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <boost/unordered_map.hpp>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <uno/dispatcher.h>
#include <uno/mapping.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace cppu;
using namespace osl;
using namespace std;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

using ::rtl::OUString;

#define SERVICENAME "com.sun.star.uno.NamingService"
#define IMPLNAME    "com.sun.star.comp.stoc.NamingService"

namespace stoc_namingservice
{
static rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

static Sequence< OUString > ns_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME));
    return seqNames;
}

static OUString ns_getImplementationName()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(IMPLNAME));
}

struct equalOWString_Impl
{
  sal_Bool operator()(const OUString & s1, const OUString & s2) const
        { return s1 == s2; }
};

struct hashOWString_Impl
{
    size_t operator()(const OUString & rName) const
        { return rName.hashCode(); }
};

typedef boost::unordered_map
<
    OUString,
    Reference<XInterface >,
    hashOWString_Impl,
    equalOWString_Impl
> HashMap_OWString_Interface;

//==================================================================================================
class NamingService_Impl
    : public WeakImplHelper2 < XServiceInfo, XNamingService >
{
    Mutex                               aMutex;
    HashMap_OWString_Interface          aMap;
public:
    NamingService_Impl();
    ~NamingService_Impl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
    throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static()
    {
        OUString aStr( OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME)) );
        return Sequence< OUString >( &aStr, 1 );
    }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRegisteredObject( const ::rtl::OUString& Name ) throw(Exception, RuntimeException);
    virtual void SAL_CALL registerObject( const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Object ) throw(Exception, RuntimeException);
    virtual void SAL_CALL revokeObject( const ::rtl::OUString& Name ) throw(Exception, RuntimeException);
};

//==================================================================================================
static Reference<XInterface> SAL_CALL NamingService_Impl_create(
    SAL_UNUSED_PARAMETER const Reference<XComponentContext> & )
{
    return *new NamingService_Impl();
}

//==================================================================================================
NamingService_Impl::NamingService_Impl()
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}

//==================================================================================================
NamingService_Impl::~NamingService_Impl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

// XServiceInfo
OUString NamingService_Impl::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return ns_getImplementationName();
}

// XServiceInfo
sal_Bool NamingService_Impl::supportsService( const OUString & rServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos] == rServiceName)
            return sal_True;
    }
    return sal_False;
}

// XServiceInfo
Sequence< OUString > NamingService_Impl::getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    return ns_getSupportedServiceNames();
}

// XServiceInfo
Reference< XInterface > NamingService_Impl::getRegisteredObject( const OUString& Name ) throw(Exception, RuntimeException)
{
    Guard< Mutex > aGuard( aMutex );
    Reference< XInterface > xRet;
    HashMap_OWString_Interface::iterator aIt = aMap.find( Name );
    if( aIt != aMap.end() )
        xRet = (*aIt).second;
    return xRet;
}

// XServiceInfo
void NamingService_Impl::registerObject( const OUString& Name, const Reference< XInterface >& Object ) throw(Exception, RuntimeException)
{
    Guard< Mutex > aGuard( aMutex );
    aMap[ Name ] = Object;
}

// XServiceInfo
void NamingService_Impl::revokeObject( const OUString& Name ) throw(Exception, RuntimeException)
{
    Guard< Mutex > aGuard( aMutex );
    aMap.erase( Name );
}

}

using namespace stoc_namingservice;
static struct ImplementationEntry g_entries[] =
{
    {
        NamingService_Impl_create, ns_getImplementationName,
        ns_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL namingservice_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
