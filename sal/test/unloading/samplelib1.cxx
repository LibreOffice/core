/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_sal.hxx"

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
        OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME1)),
        OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME1)) )), UNO_QUERY);
}
static Reference<XInterface> SAL_CALL test2_createInstance(const Reference<XMultiServiceFactory> & rSMgr)
        throw (RuntimeException)
{
    return Reference<XInterface>( static_cast<XWeak*>( new TestService(
        OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME2)),
        OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME2)) )), UNO_QUERY);
}
static Reference<XInterface> SAL_CALL test3_createInstance(const Reference<XMultiServiceFactory> & rSMgr)
        throw (RuntimeException)
{
    return Reference<XInterface>( static_cast<XWeak*>( new TestService(
        OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME3)),
        OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME3)) )), UNO_QUERY);
}
static Reference<XInterface> SAL_CALL test4_createInstance(const Reference<XMultiServiceFactory> & rSMgr)
        throw (RuntimeException)
{
    return Reference<XInterface>( static_cast<XWeak*>( new TestService(
        OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME4)),
        OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME4)) )), UNO_QUERY);
}


// Standard UNO library interface -------------------------------------------------
extern "C" {
    void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv){
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo(void * pServiceManager, void * pRegistryKey) throw()
    {
        if (pRegistryKey)
        {
            try
            {
                Reference< XRegistryKey > xNewKey(
                    reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                        OUString(RTL_CONSTASCII_USTRINGPARAM( "/" IMPLNAME1 "/UNO/SERVICES" )) ) );

                xNewKey->createKey( OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME1)));

                xNewKey=
                    reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                        OUString(RTL_CONSTASCII_USTRINGPARAM( "/" IMPLNAME2 "/UNO/SERVICES" )) );

                xNewKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME2)));
                xNewKey=
                    reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                        OUString(RTL_CONSTASCII_USTRINGPARAM( "/" IMPLNAME3 "/UNO/SERVICES" ))   );

                xNewKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME3)));

                xNewKey=
                    reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                        OUString(RTL_CONSTASCII_USTRINGPARAM( "/" IMPLNAME4 "/UNO/SERVICES" ))   );

                xNewKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME4)));
                return sal_True;
            }
            catch (InvalidRegistryException &)
            {
                OSL_FAIL( "### InvalidRegistryException!" );
            }
        }
        return sal_False;
    }

    void * SAL_CALL component_getFactory(const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey) throw()
    {
        void * pRet = 0;


        OUString implname1( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME1) );
        OUString serviceName1( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME1) );
        OUString implname2( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME2) );
        OUString serviceName2( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME2) );
        OUString implname3( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME3) );
        OUString serviceName3( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME3) );
        OUString implname4( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME4) );
        OUString serviceName4( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME4) );

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

    sal_Bool component_canUnload( TimeValue* libUnused)
    {
        return globalModuleCount.canUnload( &globalModuleCount, libUnused);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
