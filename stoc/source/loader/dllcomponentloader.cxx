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


#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/shlib.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace com::sun::star::registry { class XRegistryKey; }

using namespace com::sun::star;
using namespace css::uno;
using namespace css::loader;
using namespace css::lang;
using namespace css::registry;
using namespace cppu;

namespace {

class DllComponentLoader
    : public WeakImplHelper< XImplementationLoader,
                              XInitialization,
                              XServiceInfo >
{
public:
    explicit DllComponentLoader( const Reference<XComponentContext> & xCtx );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XImplementationLoader
    virtual Reference<XInterface> SAL_CALL activate( const OUString& implementationName, const OUString& implementationLoaderUrl, const OUString& locationUrl, const Reference<XRegistryKey>& xKey ) override;
    virtual sal_Bool SAL_CALL writeRegistryInfo( const Reference<XRegistryKey>& xKey, const OUString& implementationLoaderUrl, const OUString& locationUrl ) override;

private:
    Reference<XMultiServiceFactory> m_xSMgr;
};


DllComponentLoader::DllComponentLoader( const Reference<XComponentContext> & xCtx )
{
    m_xSMgr.set( xCtx->getServiceManager(), UNO_QUERY );
}

OUString SAL_CALL DllComponentLoader::getImplementationName(  )
{
    return u"com.sun.star.comp.stoc.DLLComponentLoader"_ustr;
}

sal_Bool SAL_CALL DllComponentLoader::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL DllComponentLoader::getSupportedServiceNames(  )
{
    return { u"com.sun.star.loader.SharedLibrary"_ustr };
}


void DllComponentLoader::initialize( const css::uno::Sequence< css::uno::Any >& )
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
{
    return loadSharedLibComponentFactory(
        cppu::bootstrap_expandUri(rLibName), OUString(), rImplName, m_xSMgr,
        css::uno::Reference<css::registry::XRegistryKey>());
}


sal_Bool SAL_CALL DllComponentLoader::writeRegistryInfo(
    const Reference< XRegistryKey > & xKey, const OUString &, const OUString & rLibName )
{
#ifdef DISABLE_DYNLOADING
    (void) xKey;
    (void) rLibName;
    OSL_FAIL( "DllComponentLoader::writeRegistryInfo() should not be called I think?" );
    return sal_False;
#else
    writeSharedLibComponentInfo(
        cppu::bootstrap_expandUri(rLibName), OUString(), m_xSMgr, xKey );
    return true;
#endif
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_stoc_DLLComponentLoader_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new DllComponentLoader(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
