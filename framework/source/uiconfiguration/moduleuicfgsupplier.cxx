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

#include <stdtypes.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManager.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManager2.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <unordered_map>

using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::embed;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::frame;
using namespace framework;

namespace {

typedef cppu::WeakComponentImplHelper<
    css::lang::XServiceInfo,
    css::ui::XModuleUIConfigurationManagerSupplier >
        ModuleUIConfigurationManagerSupplier_BASE;

class ModuleUIConfigurationManagerSupplier : private cppu::BaseMutex,
                                             public ModuleUIConfigurationManagerSupplier_BASE
{
public:
    explicit ModuleUIConfigurationManagerSupplier( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~ModuleUIConfigurationManagerSupplier() override;

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.framework.ModuleUIConfigurationManagerSupplier";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.ui.ModuleUIConfigurationManagerSupplier"};
    }

    // XModuleUIConfigurationManagerSupplier
    virtual css::uno::Reference< css::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager( const OUString& ModuleIdentifier ) override;

private:
    virtual void SAL_CALL disposing() final override;

    typedef std::unordered_map< OUString, css::uno::Reference< css::ui::XModuleUIConfigurationManager2 > > ModuleToModuleCfgMgr;

//TODO_AS            void impl_initStorages();

    ModuleToModuleCfgMgr                                                                m_aModuleToModuleUICfgMgrMap;
    css::uno::Reference< css::frame::XModuleManager2 >          m_xModuleMgr;
    css::uno::Reference< css::uno::XComponentContext >            m_xContext;
};

ModuleUIConfigurationManagerSupplier::ModuleUIConfigurationManagerSupplier( const Reference< XComponentContext >& xContext ) :
    ModuleUIConfigurationManagerSupplier_BASE(m_aMutex)
    , m_xModuleMgr( ModuleManager::create( xContext ) )
    , m_xContext( xContext )
{
    try
    {
        // Retrieve known modules and insert them into our unordered_map to speed-up access time.
        Reference< XNameAccess > xNameAccess( m_xModuleMgr, UNO_QUERY_THROW );
        const Sequence< OUString >     aNameSeq   = xNameAccess->getElementNames();
        for ( const OUString& rName : aNameSeq )
            m_aModuleToModuleUICfgMgrMap.emplace( rName, Reference< XModuleUIConfigurationManager2 >() );
    }
    catch(...)
    {
    }
}

ModuleUIConfigurationManagerSupplier::~ModuleUIConfigurationManagerSupplier()
{
    disposing();
}

void SAL_CALL ModuleUIConfigurationManagerSupplier::disposing()
{
    osl::MutexGuard g(rBHelper.rMutex);

    // dispose all our module user interface configuration managers
    for (auto const& elem : m_aModuleToModuleUICfgMgrMap)
    {
        Reference< XComponent > xComponent( elem.second, UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();
    }
    m_aModuleToModuleUICfgMgrMap.clear();
    m_xModuleMgr.clear();
}

// XModuleUIConfigurationManagerSupplier
Reference< XUIConfigurationManager > SAL_CALL ModuleUIConfigurationManagerSupplier::getUIConfigurationManager( const OUString& sModuleIdentifier )
{
    osl::MutexGuard g(rBHelper.rMutex);
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    ModuleToModuleCfgMgr::iterator pIter = m_aModuleToModuleUICfgMgrMap.find( sModuleIdentifier );
    if ( pIter == m_aModuleToModuleUICfgMgrMap.end() )
        throw NoSuchElementException();
//TODO_AS    impl_initStorages();

    // Create instance on demand
    if ( !pIter->second.is() )
    {
        OUString sShort;
        try
        {
            Sequence< PropertyValue > lProps;
            m_xModuleMgr->getByName(sModuleIdentifier) >>= lProps;
            for (PropertyValue const & rProp : std::as_const(lProps))
            {
                if ( rProp.Name == "ooSetupFactoryShortName" )
                {
                    rProp.Value >>= sShort;
                    break;
                }
            }
        }
        catch( const Exception& )
        {
            sShort.clear();
        }

        if (sShort.isEmpty())
            throw NoSuchElementException();

        pIter->second = css::ui::ModuleUIConfigurationManager::createDefault(m_xContext, sShort, sModuleIdentifier);
    }

    return pIter->second;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_ModuleUIConfigurationManagerSupplier_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ModuleUIConfigurationManagerSupplier(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
