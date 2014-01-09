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

#include <threadhelp/resetableguard.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <services.h>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/XPackageStructureCreator.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManager.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManager2.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <cppuhelper/implbase3.hxx>
#include <rtl/ref.hxx>
#include <vcl/svapp.hxx>

#include <boost/unordered_map.hpp>

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

class ModuleUIConfigurationManagerSupplier : private ThreadHelpBase                     ,   // Struct for right initalization of mutex member! Must be first of baseclasses.
                                             public cppu::WeakImplHelper3<
                                                 css::lang::XServiceInfo  ,
                                                 css::lang::XComponent    ,
                                                 css::ui::XModuleUIConfigurationManagerSupplier >
{
    public:
        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        ModuleUIConfigurationManagerSupplier( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~ModuleUIConfigurationManagerSupplier();

        // XComponent
        virtual void SAL_CALL dispose()
            throw (css::uno::RuntimeException);
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener )
            throw (css::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
            throw (css::uno::RuntimeException);

        // XModuleUIConfigurationManagerSupplier
        virtual css::uno::Reference< css::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager( const OUString& ModuleIdentifier )
            throw (css::container::NoSuchElementException, css::uno::RuntimeException);

    private:
        typedef ::boost::unordered_map< OUString, css::uno::Reference< css::ui::XModuleUIConfigurationManager2 >, OUStringHash, ::std::equal_to< OUString > > ModuleToModuleCfgMgr;

//TODO_AS            void impl_initStorages();

        // private methods
        ModuleToModuleCfgMgr                                                                m_aModuleToModuleUICfgMgrMap;
        bool                                                                                m_bDisposed;
// TODO_AS            bool                                                                                m_bInit;
        OUString                                                                       m_aDefaultConfigURL;
        OUString                                                                       m_aUserConfigURL;
        css::uno::Reference< css::embed::XStorage >                   m_xDefaultCfgRootStorage;
        css::uno::Reference< css::embed::XStorage >                   m_xUserCfgRootStorage;
        css::uno::Reference< css::embed::XTransactedObject >          m_xUserRootCommit;
        css::uno::Reference< css::frame::XModuleManager2 >          m_xModuleMgr;
        css::uno::Reference< css::uno::XComponentContext >            m_xContext;
        ::cppu::OMultiTypeInterfaceContainerHelper                                          m_aListenerContainer;   /// container for ALL Listener
};

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2(   ModuleUIConfigurationManagerSupplier                    ,
                                            ::cppu::OWeakObject                                     ,
                                            "com.sun.star.ui.ModuleUIConfigurationManagerSupplier"  ,
                                            IMPLEMENTATIONNAME_MODULEUICONFIGURATIONMANAGERSUPPLIER
                                        )

DEFINE_INIT_SERVICE                     (   ModuleUIConfigurationManagerSupplier, {} )



ModuleUIConfigurationManagerSupplier::ModuleUIConfigurationManagerSupplier( const Reference< XComponentContext >& xContext ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_bDisposed( false )
//TODO_AS    , m_bInit( false )
    , m_xModuleMgr( ModuleManager::create( xContext ) )
    , m_xContext( xContext )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
{
    try
    {
        // Retrieve known modules and insert them into our boost::unordered_map to speed-up access time.
        Reference< XNameAccess > xNameAccess( m_xModuleMgr, UNO_QUERY_THROW );
        const Sequence< OUString >     aNameSeq   = xNameAccess->getElementNames();
        const OUString*                pNameSeq   = aNameSeq.getConstArray();
        for ( sal_Int32 n = 0; n < aNameSeq.getLength(); n++ )
            m_aModuleToModuleUICfgMgrMap.insert( ModuleToModuleCfgMgr::value_type(  pNameSeq[n], Reference< XModuleUIConfigurationManager2 >() ));
    }
    catch(...)
    {
    }
}

ModuleUIConfigurationManagerSupplier::~ModuleUIConfigurationManagerSupplier()
{
    m_xUserRootCommit.clear();

    // dispose all our module user interface configuration managers
    ModuleToModuleCfgMgr::iterator pIter = m_aModuleToModuleUICfgMgrMap.begin();
    while ( pIter != m_aModuleToModuleUICfgMgrMap.end() )
    {
        Reference< XComponent > xComponent( pIter->second, UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();
        ++pIter;
    }
}

// XComponent
void SAL_CALL ModuleUIConfigurationManagerSupplier::dispose()
throw ( RuntimeException )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        ResetableGuard aGuard( m_aLock );
        m_bDisposed = true;
    }
}

void SAL_CALL ModuleUIConfigurationManagerSupplier::addEventListener( const Reference< XEventListener >& xListener )
throw ( RuntimeException )
{
    {
        ResetableGuard aGuard( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL ModuleUIConfigurationManagerSupplier::removeEventListener( const Reference< XEventListener >& xListener )
throw ( RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

// XModuleUIConfigurationManagerSupplier
Reference< XUIConfigurationManager > SAL_CALL ModuleUIConfigurationManagerSupplier::getUIConfigurationManager( const OUString& sModuleIdentifier )
throw ( NoSuchElementException, RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

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
            Reference< XNameAccess > xCont(m_xModuleMgr, UNO_QUERY);
            xCont->getByName(sModuleIdentifier) >>= lProps;
            for (sal_Int32 i=0; i<lProps.getLength(); ++i)
            {
                if ( lProps[i].Name == "ooSetupFactoryShortName" )
                {
                    lProps[i].Value >>= sShort;
                    break;
                }
            }
        }
        catch( const Exception& )
        {
            sShort = OUString();
        }

        if (sShort.isEmpty())
            throw NoSuchElementException();

        pIter->second = css::ui::ModuleUIConfigurationManager::createDefault(m_xContext, sShort, sModuleIdentifier);
    }

    return pIter->second;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ModuleUIConfigurationManagerSupplier_get_implementation(
        css::uno::XComponentContext * context,
        uno_Sequence * arguments)
{
    assert(arguments != 0 && arguments->nElements == 0); (void) arguments;
    rtl::Reference<ModuleUIConfigurationManagerSupplier> x(
            new ModuleUIConfigurationManagerSupplier(context));
    x->acquire();
    return static_cast<cppu::OWeakObject *>(x.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
