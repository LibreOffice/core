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


#include <stdlib.h>
#include <osl/file.h>
#include <vector>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <osl/module.h>
#include <rtl/ustring.hxx>
#include <uno/environment.h>
#include <uno/mapping.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/shlib.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#define SERVICENAME "com.sun.star.loader.SharedLibrary"
#define IMPLNAME    "com.sun.star.comp.stoc.DLLComponentLoader"


using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::loader;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace osl;
extern rtl_StandardModuleCount g_moduleCount;

namespace stoc_bootstrap
{
Sequence< OUString > loader_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME));
    return seqNames;
}

OUString loader_getImplementationName()
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(IMPLNAME));
}
}

namespace stoc_loader
{
//*************************************************************************
// DllComponentLoader
//*************************************************************************
class DllComponentLoader
    : public WeakImplHelper3< XImplementationLoader,
                              XInitialization,
                              XServiceInfo >
{
public:
    DllComponentLoader( const Reference<XComponentContext> & xCtx );
    ~DllComponentLoader();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XImplementationLoader
    virtual Reference<XInterface> SAL_CALL activate( const OUString& implementationName, const OUString& implementationLoaderUrl, const OUString& locationUrl, const Reference<XRegistryKey>& xKey ) throw(CannotActivateFactoryException, RuntimeException);
    virtual sal_Bool SAL_CALL writeRegistryInfo( const Reference<XRegistryKey>& xKey, const OUString& implementationLoaderUrl, const OUString& locationUrl ) throw(CannotRegisterImplementationException, RuntimeException);

private:
    OUString expand_url( OUString const & url )
        SAL_THROW( (RuntimeException) );

    Reference<XMultiServiceFactory> m_xSMgr;
};

//*************************************************************************
DllComponentLoader::DllComponentLoader( const Reference<XComponentContext> & xCtx )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    m_xSMgr.set( xCtx->getServiceManager(), UNO_QUERY );
}

//*************************************************************************
DllComponentLoader::~DllComponentLoader()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

//*************************************************************************
OUString SAL_CALL DllComponentLoader::getImplementationName(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    return stoc_bootstrap::loader_getImplementationName();
}

//*************************************************************************
sal_Bool SAL_CALL DllComponentLoader::supportsService( const OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL DllComponentLoader::getSupportedServiceNames(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    return stoc_bootstrap::loader_getSupportedServiceNames();
}

//*************************************************************************
void DllComponentLoader::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    OSL_FAIL( "dllcomponentloader::initialize should not be called !" );
//      if( aArgs.getLength() != 1 )
//      {
//          throw IllegalArgumentException();
//      }

//      Reference< XMultiServiceFactory > rServiceManager;

//      if( aArgs.getConstArray()[0].getValueType().getTypeClass() == TypeClass_INTERFACE )
//      {
//          aArgs.getConstArray()[0] >>= rServiceManager;
//      }

//      if( !rServiceManager.is() )
//      {
//          throw IllegalArgumentException();
//      }

//      m_xSMgr = rServiceManager;
}

//==================================================================================================
OUString DllComponentLoader::expand_url( OUString const & url )
    SAL_THROW( (RuntimeException) )
{
    try
    {
        return cppu::bootstrap_expandUri( url );
    }
    catch ( const IllegalArgumentException & e )
    {
        throw RuntimeException( e.Message, e.Context );
    }
}

//*************************************************************************
Reference<XInterface> SAL_CALL DllComponentLoader::activate(
    const OUString & rImplName, const OUString &, const OUString & rLibName,
    const Reference< XRegistryKey > & xKey )

    throw(CannotActivateFactoryException, RuntimeException)
{
    OUString aPrefix;
    if( xKey.is() )
    {
        Reference<XRegistryKey > xActivatorKey = xKey->openKey(
                OUString("/UNO/ACTIVATOR") );
        if (xActivatorKey.is() && xActivatorKey->getValueType() == RegistryValueType_ASCII )
        {
            Reference<XRegistryKey > xPrefixKey = xKey->openKey(
                OUString("/UNO/PREFIX") );
            if( xPrefixKey.is() && xPrefixKey->getValueType() == RegistryValueType_ASCII )
            {
                aPrefix = xPrefixKey->getAsciiValue();
                if( !aPrefix.isEmpty() )
                    aPrefix = aPrefix + "_";
            }
        }
    }

    return loadSharedLibComponentFactory(
        expand_url( rLibName ), OUString(), rImplName, m_xSMgr, xKey, aPrefix );
}


//*************************************************************************
sal_Bool SAL_CALL DllComponentLoader::writeRegistryInfo(
    const Reference< XRegistryKey > & xKey, const OUString &, const OUString & rLibName )

    throw(CannotRegisterImplementationException, RuntimeException)
{
#ifdef DISABLE_DYNLOADING
    (void) xKey;
    (void) rLibName;
    OSL_FAIL( "DllComponentLoader::writeRegistryInfo() should not be called I think?" );
    return sal_False;
#else
    writeSharedLibComponentInfo(
        expand_url( rLibName ), OUString(), m_xSMgr, xKey );
    return sal_True;
#endif
}
}

namespace stoc_bootstrap
{
//*************************************************************************
Reference<XInterface> SAL_CALL DllComponentLoader_CreateInstance( const Reference<XComponentContext> & xCtx ) throw(Exception)
{
    Reference<XInterface> xRet;

    XImplementationLoader *pXLoader = (XImplementationLoader *)new stoc_loader::DllComponentLoader(xCtx);

    if (pXLoader)
    {
        xRet = Reference<XInterface>::query(pXLoader);
    }

    return xRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
