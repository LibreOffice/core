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

#include <uiconfiguration/moduleuicfgsupplier.hxx>
#include <threadhelp/resetableguard.hxx>
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

#include <rtl/logfile.hxx>
#include <cppuhelper/implbase1.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::embed;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::frame;

namespace framework
{

class RootStorageWrapper :  public ::cppu::WeakImplHelper1< com::sun::star::embed::XTransactedObject >
{
    public:
        //  XInterface, XTypeProvider
        RootStorageWrapper( const Reference< XTransactedObject >& xRootCommit ) : m_xRootCommit( xRootCommit ) {}
        virtual ~RootStorageWrapper() {}

        // XTransactedObject
        virtual void SAL_CALL commit() throw ( com::sun::star::io::IOException, com::sun::star::lang::WrappedTargetException )
        {
            m_xRootCommit->commit();
        }

        virtual void SAL_CALL revert() throw ( com::sun::star::io::IOException, com::sun::star::lang::WrappedTargetException )
        {
            m_xRootCommit->revert();
        }

    private:
        Reference< XTransactedObject > m_xRootCommit;
};

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_4                    (    ModuleUIConfigurationManagerSupplier                    ,
                                            OWeakObject                                             ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                          ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                           ),
                                            DIRECT_INTERFACE( css::lang::XComponent                                             ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XModuleUIConfigurationManagerSupplier )
                                        )

DEFINE_XTYPEPROVIDER_4                  (   ModuleUIConfigurationManagerSupplier                                ,
                                            css::lang::XTypeProvider                                            ,
                                            css::lang::XServiceInfo                                             ,
                                            css::lang::XComponent                                               ,
                                            ::com::sun::star::ui::XModuleUIConfigurationManagerSupplier
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2(   ModuleUIConfigurationManagerSupplier                    ,
                                            ::cppu::OWeakObject                                     ,
                                            DECLARE_ASCII("com.sun.star.ui.ModuleUIConfigurationManagerSupplier" ),
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManagerSupplier::ModuleUIConfigurationManagerSupplier" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManagerSupplier::dispose" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManagerSupplier::addEventListener" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManagerSupplier::removeEventListener" );
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

// XModuleUIConfigurationManagerSupplier
Reference< XUIConfigurationManager > SAL_CALL ModuleUIConfigurationManagerSupplier::getUIConfigurationManager( const OUString& sModuleIdentifier )
throw ( NoSuchElementException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManagerSupplier::getUIConfigurationManager" );
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

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
