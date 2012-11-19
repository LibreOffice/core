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

#include <sal/types.h>
#include <osl/time.h>
#include <rtl/ustring.hxx>
#include <uno/environment.h>
#include <cppu/macros.hxx>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/unload.h>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;
using namespace ::cppu;

#define IMPLNAME1 "com.sun.star.comp.sal.UnloadingTest1"
#define SERVICENAME1 "com.sun.star.UnloadingTest1"
#define IMPLNAME2 "com.sun.star.comp.sal.UnloadingTest2"
#define SERVICENAME2 "com.sun.star.UnloadingTest2"
#define IMPLNAME3 "com.sun.star.comp.sal.UnloadingTest3"
#define SERVICENAME3 "com.sun.star.UnloadingTest3"
#define IMPLNAME4 "com.sun.star.comp.sal.OneInstanceTest"
#define SERVICENAME4 "com.sun.star.OneInstanceTest"


// Unloading Support ----------------------------------------------
rtl_StandardModuleCount globalModuleCount= MODULE_COUNT_INIT;
//rtl_StandardModuleCount globalModuleCount= { {rtl_moduleCount_acquire,rtl_moduleCount_release}, rtl_moduleCount_canUnload,0,{0,0}}; //, 0, {0, 0}};
// Services -------------------------------------------------------
class TestService: public WeakImplHelper1<XServiceInfo>
{
OUString m_implName;
OUString m_serviceName;
public:
    TestService( OUString implName, OUString serviceName);
    ~TestService();
    virtual OUString SAL_CALL getImplementationName(  )  throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (RuntimeException);
    virtual Sequence<OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);
};

TestService::TestService( OUString implName, OUString serviceName):
            m_implName(implName),m_serviceName(serviceName)
{   // Library unloading support
    globalModuleCount.modCnt.acquire( &globalModuleCount.modCnt);
}

TestService::~TestService()
{   // Library unloading support
    globalModuleCount.modCnt.release( &globalModuleCount.modCnt);
}

OUString SAL_CALL TestService::getImplementationName(  )  throw (RuntimeException)
{
    return m_implName;
}
sal_Bool SAL_CALL TestService::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    return ServiceName.equals( m_serviceName);
}
Sequence<OUString > SAL_CALL TestService::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return Sequence<OUString>( &m_serviceName, 1);
}


// Creator functions for Services -------------------------------------------------
static Reference<XInterface> SAL_CALL test1_createInstance(const Reference<XMultiServiceFactory> & rSMgr)
        throw (RuntimeException)
{
    return Reference<XInterface>( static_cast<XWeak*>( new TestService(
        OUString(  IMPLNAME1),
        OUString(  SERVICENAME1) )), UNO_QUERY);
}
static Reference<XInterface> SAL_CALL test2_createInstance(const Reference<XMultiServiceFactory> & rSMgr)
        throw (RuntimeException)
{
    return Reference<XInterface>( static_cast<XWeak*>( new TestService(
        OUString(  IMPLNAME2),
        OUString(  SERVICENAME2) )), UNO_QUERY);
}
static Reference<XInterface> SAL_CALL test3_createInstance(const Reference<XMultiServiceFactory> & rSMgr)
        throw (RuntimeException)
{
    return Reference<XInterface>( static_cast<XWeak*>( new TestService(
        OUString(  IMPLNAME3),
        OUString(  SERVICENAME3) )), UNO_QUERY);
}
static Reference<XInterface> SAL_CALL test4_createInstance(const Reference<XMultiServiceFactory> & rSMgr)
        throw (RuntimeException)
{
    return Reference<XInterface>( static_cast<XWeak*>( new TestService(
        OUString(  IMPLNAME4),
        OUString(  SERVICENAME4) )), UNO_QUERY);
}


// Standard UNO library interface -------------------------------------------------
extern "C" {
    sal_Bool SAL_CALL component_writeInfo(void * pServiceManager, void * pRegistryKey) throw()
    {
        if (pRegistryKey)
        {
            try
            {
                Reference< XRegistryKey > xNewKey(
                    reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                        OUString( "/" IMPLNAME1 "/UNO/SERVICES" ) ) );

                xNewKey->createKey( OUString(  SERVICENAME1));

                xNewKey=
                    reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                        OUString( "/" IMPLNAME2 "/UNO/SERVICES" ) );

                xNewKey->createKey(OUString(  SERVICENAME2));
                xNewKey=
                    reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                        OUString( "/" IMPLNAME3 "/UNO/SERVICES" )   );

                xNewKey->createKey(OUString(  SERVICENAME3));

                xNewKey=
                    reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                        OUString( "/" IMPLNAME4 "/UNO/SERVICES" )   );

                xNewKey->createKey(OUString(  SERVICENAME4));
                return sal_True;
            }
            catch (const InvalidRegistryException &)
            {
                OSL_FAIL( "### InvalidRegistryException!" );
            }
        }
        return sal_False;
    }

    SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey) throw()
    {
        void * pRet = 0;


        OUString implname1(  IMPLNAME1 );
        OUString serviceName1(  SERVICENAME1 );
        OUString implname2(  IMPLNAME2 );
        OUString serviceName2(  SERVICENAME2 );
        OUString implname3(  IMPLNAME3 );
        OUString serviceName3(  SERVICENAME3 );
        OUString implname4(  IMPLNAME4 );
        OUString serviceName4(  SERVICENAME4 );

        if (implname1.equals( OUString::createFromAscii(pImplName)))
        {
            Reference<XMultiServiceFactory> mgr= reinterpret_cast<XMultiServiceFactory *>(pServiceManager);
            Reference<XSingleServiceFactory> xFactory( createSingleFactory(
                reinterpret_cast<XMultiServiceFactory *>(pServiceManager),
                implname1,
                test1_createInstance,
                Sequence<OUString>( &serviceName1, 1),
                &globalModuleCount.modCnt
                ));

            if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        else if( implname2.equals( OUString::createFromAscii(pImplName)))
        {
            Reference<XMultiServiceFactory> mgr= reinterpret_cast<XMultiServiceFactory *>(pServiceManager);
            Reference<XSingleServiceFactory> xFactory( createSingleFactory(
                reinterpret_cast<XMultiServiceFactory *>(pServiceManager),
                implname2,
                test2_createInstance,
                Sequence<OUString>( &serviceName2, 1),
                &globalModuleCount.modCnt
                ));

            if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        else if( implname3.equals( OUString::createFromAscii(pImplName)))
        {
            Reference<XMultiServiceFactory> mgr= reinterpret_cast<XMultiServiceFactory *>(pServiceManager);
            Reference<XSingleServiceFactory> xFactory( createSingleFactory(
                reinterpret_cast<XMultiServiceFactory *>(pServiceManager),
                implname3,
                test3_createInstance,
                Sequence<OUString>( &serviceName3, 1),
                &globalModuleCount.modCnt
                ));

            if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        else if( implname4.equals( OUString::createFromAscii(pImplName)))
        {
            Reference<XMultiServiceFactory> mgr= reinterpret_cast<XMultiServiceFactory *>(pServiceManager);
            Reference<XSingleServiceFactory> xFactory( createOneInstanceFactory(
                reinterpret_cast<XMultiServiceFactory *>(pServiceManager),
                implname3,
                test4_createInstance,
                Sequence<OUString>( &serviceName3, 1),
                &globalModuleCount.modCnt
                ));

            if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        return pRet;
    }

    SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_canUnload( TimeValue* libUnused)
    {
        return globalModuleCount.canUnload( &globalModuleCount, libUnused);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
