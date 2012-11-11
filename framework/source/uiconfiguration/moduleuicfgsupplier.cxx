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

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   ModuleUIConfigurationManagerSupplier                    ,
                                            ::cppu::OWeakObject                                     ,
                                            SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER        ,
                                            IMPLEMENTATIONNAME_MODULEUICONFIGURATIONMANAGERSUPPLIER
                                        )

DEFINE_INIT_SERVICE                     (   ModuleUIConfigurationManagerSupplier, {} )



ModuleUIConfigurationManagerSupplier::ModuleUIConfigurationManagerSupplier( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_bDisposed( false )
//TODO_AS    , m_bInit( false )
    , m_xModuleMgr( ModuleManager::create( comphelper::getComponentContext(xServiceManager) ) )
    , m_xServiceManager( xServiceManager )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManagerSupplier::ModuleUIConfigurationManagerSupplier" );
    try
    {
        // Retrieve known modules and insert them into our boost::unordered_map to speed-up access time.
        Reference< XNameAccess > xNameAccess( m_xModuleMgr, UNO_QUERY_THROW );
        const Sequence< ::rtl::OUString >     aNameSeq   = xNameAccess->getElementNames();
        const ::rtl::OUString*                pNameSeq   = aNameSeq.getConstArray();
        for ( sal_Int32 n = 0; n < aNameSeq.getLength(); n++ )
            m_aModuleToModuleUICfgMgrMap.insert( ModuleToModuleCfgMgr::value_type(  pNameSeq[n], Reference< XUIConfigurationManager >() ));
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
Reference< XUIConfigurationManager > SAL_CALL ModuleUIConfigurationManagerSupplier::getUIConfigurationManager( const ::rtl::OUString& ModuleIdentifier )
throw ( NoSuchElementException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManagerSupplier::getUIConfigurationManager" );
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    ModuleToModuleCfgMgr::iterator pIter = m_aModuleToModuleUICfgMgrMap.find( ModuleIdentifier );
    if ( pIter == m_aModuleToModuleUICfgMgrMap.end() )
        throw NoSuchElementException();
//TODO_AS    impl_initStorages();

    // Create instance on demand
    if ( !pIter->second.is() )
    {
        ::rtl::OUString sShort;
        try
        {
            Sequence< PropertyValue > lProps;
            Reference< XNameAccess > xCont(m_xModuleMgr, UNO_QUERY);
            xCont->getByName(ModuleIdentifier) >>= lProps;
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
            sShort = ::rtl::OUString();
        }

        if (sShort.isEmpty())
            throw NoSuchElementException();
        PropertyValue   aArg;
        Sequence< Any > aArgs( 2 );
        aArg.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleShortName" ));
        aArg.Value <<= sShort;
        aArgs[0] <<= aArg;
        aArg.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleIdentifier" ));
        aArg.Value <<= ModuleIdentifier;
        aArgs[1] <<= aArg;

        pIter->second.set( m_xServiceManager->createInstanceWithArguments(SERVICENAME_MODULEUICONFIGURATIONMANAGER, aArgs ),UNO_QUERY );
    }

    return pIter->second;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
