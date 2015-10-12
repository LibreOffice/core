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
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <uno/environment.h>
#include <uno/mapping.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/shlib.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star;
using namespace css::uno;
using namespace css::loader;
using namespace css::lang;
using namespace css::registry;
using namespace cppu;
using namespace osl;

namespace {

class DllComponentLoader
    : public WeakImplHelper< XImplementationLoader,
                              XInitialization,
                              XServiceInfo >
{
public:
    explicit DllComponentLoader( const Reference<XComponentContext> & xCtx );
    virtual ~DllComponentLoader();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XImplementationLoader
    virtual Reference<XInterface> SAL_CALL activate( const OUString& implementationName, const OUString& implementationLoaderUrl, const OUString& locationUrl, const Reference<XRegistryKey>& xKey ) throw(CannotActivateFactoryException, RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL writeRegistryInfo( const Reference<XRegistryKey>& xKey, const OUString& implementationLoaderUrl, const OUString& locationUrl ) throw(CannotRegisterImplementationException, RuntimeException, std::exception) override;

private:
    Reference<XMultiServiceFactory> m_xSMgr;
};


DllComponentLoader::DllComponentLoader( const Reference<XComponentContext> & xCtx )
{
    m_xSMgr.set( xCtx->getServiceManager(), UNO_QUERY );
}


DllComponentLoader::~DllComponentLoader() {}


OUString SAL_CALL DllComponentLoader::getImplementationName(  )
    throw(css::uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.stoc.DLLComponentLoader");
}

sal_Bool SAL_CALL DllComponentLoader::supportsService( const OUString& ServiceName )
    throw(css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL DllComponentLoader::getSupportedServiceNames(  )
    throw(css::uno::RuntimeException, std::exception)
{
    Sequence< OUString > seqNames(1);
    seqNames[0] = "com.sun.star.loader.SharedLibrary";
    return seqNames;
}


void DllComponentLoader::initialize( const css::uno::Sequence< css::uno::Any >& )
    throw(css::uno::Exception, css::uno::RuntimeException, std::exception)
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


Reference<XInterface> SAL_CALL DllComponentLoader::activate(
    const OUString & rImplName, const OUString &, const OUString & rLibName,
    const Reference< XRegistryKey > & )

    throw(CannotActivateFactoryException, RuntimeException, std::exception)
{
    return loadSharedLibComponentFactory(
        cppu::bootstrap_expandUri(rLibName), OUString(), rImplName, m_xSMgr,
        css::uno::Reference<css::registry::XRegistryKey>());
}



sal_Bool SAL_CALL DllComponentLoader::writeRegistryInfo(
    const Reference< XRegistryKey > & xKey, const OUString &, const OUString & rLibName )

    throw(CannotRegisterImplementationException, RuntimeException, std::exception)
{
#ifdef DISABLE_DYNLOADING
    (void) xKey;
    (void) rLibName;
    OSL_FAIL( "DllComponentLoader::writeRegistryInfo() should not be called I think?" );
    return sal_False;
#else
    writeSharedLibComponentInfo(
        cppu::bootstrap_expandUri(rLibName), OUString(), m_xSMgr, xKey );
    return sal_True;
#endif
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_stoc_DLLComponentLoader_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new DllComponentLoader(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
