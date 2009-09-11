/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: moduleuiconfigurationmanager.cxx,v $
 * $Revision: 1.19.208.2 $
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
#include "precompiled_framework.hxx"
#include <uiconfiguration/moduleuiconfigurationmanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include <services.h>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/uielementtypenames.hxx>
#include <xml/menuconfiguration.hxx>
#include <xml/toolboxconfiguration.hxx>
#include <uiconfigurationmanagerimpl.hxx>

#ifndef __FRAMEWORK_XML_STATUSBARCONFIGURATION_HXX_
#include <xml/statusbarconfiguration.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XStream.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/sequenceashashmap.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::embed;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace ::com::sun::star::ui;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_8                    (    ModuleUIConfigurationManager                                                    ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( css::lang::XComponent                                         ),
                                            DIRECT_INTERFACE( css::lang::XInitialization                                    ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfiguration                  ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationManager           ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XModuleUIConfigurationManager     ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationPersistence       )
                                        )

DEFINE_XTYPEPROVIDER_8                  (   ModuleUIConfigurationManager                                ,
                                            css::lang::XTypeProvider                                    ,
                                            css::lang::XServiceInfo                                     ,
                                            css::lang::XComponent                                       ,
                                            css::lang::XInitialization                                  ,
                                            ::com::sun::star::ui::XUIConfiguration                ,
                                            ::com::sun::star::ui::XUIConfigurationManager         ,
                                            ::com::sun::star::ui::XModuleUIConfigurationManager   ,
                                            ::com::sun::star::ui::XUIConfigurationPersistence
                                        )

DEFINE_XSERVICEINFO_MULTISERVICE        (   ModuleUIConfigurationManager                    ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_MODULEUICONFIGURATIONMANAGER        ,
                                            IMPLEMENTATIONNAME_MODULEUICONFIGURATIONMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   ModuleUIConfigurationManager, {} )


ModuleUIConfigurationManager::ModuleUIConfigurationManager( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xServiceManager )
: m_pImpl( new UIConfigurationManagerImpl(xServiceManager,static_cast< OWeakObject* >(this),true) )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::ModuleUIConfigurationManager" );
}

ModuleUIConfigurationManager::~ModuleUIConfigurationManager()
{
}

// XComponent
void SAL_CALL ModuleUIConfigurationManager::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::dispose" );
    m_pImpl->dispose();
}

void SAL_CALL ModuleUIConfigurationManager::addEventListener( const Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::addEventListener" );
    m_pImpl->addEventListener(xListener);
}

void SAL_CALL ModuleUIConfigurationManager::removeEventListener( const Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::removeEventListener" );
    m_pImpl->removeEventListener(xListener);
}

// XInitialization
void SAL_CALL ModuleUIConfigurationManager::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::initialize" );
    m_pImpl->initialize(aArguments);
}

// XUIConfiguration
void SAL_CALL ModuleUIConfigurationManager::addConfigurationListener( const Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::addConfigurationListener" );
    m_pImpl->addConfigurationListener(xListener);
}

void SAL_CALL ModuleUIConfigurationManager::removeConfigurationListener( const Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::removeConfigurationListener" );
    m_pImpl->removeConfigurationListener(xListener);
}


// XUIConfigurationManager
void SAL_CALL ModuleUIConfigurationManager::reset() throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::reset" );
    m_pImpl->reset();
}

Sequence< Sequence< PropertyValue > > SAL_CALL ModuleUIConfigurationManager::getUIElementsInfo( sal_Int16 ElementType )
throw ( IllegalArgumentException, RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::getUIElementsInfo" );
    return m_pImpl->getUIElementsInfo(ElementType);
}

Reference< XIndexContainer > SAL_CALL ModuleUIConfigurationManager::createSettings() throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::createSettings" );

    // Creates an empty item container which can be filled from outside
    return m_pImpl->createSettings();
}

sal_Bool SAL_CALL ModuleUIConfigurationManager::hasSettings( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::hasSettings" );
    return m_pImpl->hasSettings(ResourceURL);
}

Reference< XIndexAccess > SAL_CALL ModuleUIConfigurationManager::getSettings( const ::rtl::OUString& ResourceURL, sal_Bool bWriteable )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::getSettings" );
    return m_pImpl->getSettings(ResourceURL,bWriteable);
}

void SAL_CALL ModuleUIConfigurationManager::replaceSettings( const ::rtl::OUString& ResourceURL, const Reference< ::com::sun::star::container::XIndexAccess >& aNewData )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::replaceSettings" );
    return m_pImpl->replaceSettings(ResourceURL,aNewData);
}

void SAL_CALL ModuleUIConfigurationManager::removeSettings( const ::rtl::OUString& ResourceURL )
throw ( NoSuchElementException, IllegalArgumentException, IllegalAccessException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::removeSettings" );
    m_pImpl->removeSettings(ResourceURL);
}

void SAL_CALL ModuleUIConfigurationManager::insertSettings( const ::rtl::OUString& NewResourceURL, const Reference< XIndexAccess >& aNewData )
throw ( ElementExistException, IllegalArgumentException, IllegalAccessException, RuntimeException )
{
    m_pImpl->insertSettings(NewResourceURL,aNewData);
}

Reference< XInterface > SAL_CALL ModuleUIConfigurationManager::getImageManager() throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::getImageManager" );
    return m_pImpl->getImageManager();
}

Reference< XInterface > SAL_CALL ModuleUIConfigurationManager::getShortCutManager() throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::getShortCutManager" );
    return m_pImpl->getShortCutManager();
}

Reference< XInterface > SAL_CALL ModuleUIConfigurationManager::getEventsManager() throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::getEventsManager" );
    return Reference< XInterface >();
}

// XModuleUIConfigurationManager
sal_Bool SAL_CALL ModuleUIConfigurationManager::isDefaultSettings( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::isDefaultSettings" );
    return m_pImpl->isDefaultSettings(ResourceURL);
}

Reference< XIndexAccess > SAL_CALL ModuleUIConfigurationManager::getDefaultSettings( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::getDefaultSettings" );
    return m_pImpl->getDefaultSettings(ResourceURL);
}

// XUIConfigurationPersistence
void SAL_CALL ModuleUIConfigurationManager::reload() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::reload" );
    m_pImpl->reload();
}

void SAL_CALL ModuleUIConfigurationManager::store() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::store" );
    m_pImpl->store();
}

void SAL_CALL ModuleUIConfigurationManager::storeToStorage( const Reference< XStorage >& Storage ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::storeToStorage" );
    m_pImpl->storeToStorage(Storage);
}

sal_Bool SAL_CALL ModuleUIConfigurationManager::isModified() throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::isModified" );
    return m_pImpl->isModified();
}

sal_Bool SAL_CALL ModuleUIConfigurationManager::isReadOnly() throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ModuleUIConfigurationManager::isReadOnly" );
    return m_pImpl->isReadOnly();
}

} // namespace framework
