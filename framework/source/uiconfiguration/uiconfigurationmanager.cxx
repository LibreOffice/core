/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: uiconfigurationmanager.cxx,v $
 * $Revision: 1.19 $
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
#include <uiconfiguration/uiconfigurationmanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include <services.h>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/constitemcontainer.hxx>
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
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

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
DEFINE_XINTERFACE_7                    (    UIConfigurationManager                                                          ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( css::lang::XComponent                                         ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfiguration                  ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationManager           ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationPersistence       ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationStorage           )
                                        )

DEFINE_XTYPEPROVIDER_7                  (   UIConfigurationManager                                  ,
                                            css::lang::XTypeProvider                                ,
                                            css::lang::XServiceInfo                                 ,
                                            css::lang::XComponent                                   ,
                                            ::com::sun::star::ui::XUIConfiguration            ,
                                            ::com::sun::star::ui::XUIConfigurationManager     ,
                                            ::com::sun::star::ui::XUIConfigurationPersistence ,
                                            ::com::sun::star::ui::XUIConfigurationStorage
                                        )

DEFINE_XSERVICEINFO_MULTISERVICE        (   UIConfigurationManager                      ,
                                            ::cppu::OWeakObject                         ,
                                            SERVICENAME_UICONFIGURATIONMANAGER          ,
                                            IMPLEMENTATIONNAME_UICONFIGURATIONMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   UIConfigurationManager, {} )

UIConfigurationManager::UIConfigurationManager( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xServiceManager )
: m_pImpl( new UIConfigurationManagerImpl(xServiceManager,static_cast< OWeakObject* >(this),false) )
{
}

UIConfigurationManager::~UIConfigurationManager()
{
}

// XComponent
void SAL_CALL UIConfigurationManager::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->dispose();
}

void SAL_CALL UIConfigurationManager::addEventListener( const Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{

    m_pImpl->addEventListener(xListener);
}

void SAL_CALL UIConfigurationManager::removeEventListener( const Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->removeEventListener(xListener);
}

// XUIConfigurationManager
void SAL_CALL UIConfigurationManager::addConfigurationListener( const Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->addConfigurationListener(xListener);
}

void SAL_CALL UIConfigurationManager::removeConfigurationListener( const Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->removeConfigurationListener(xListener);
}


void SAL_CALL UIConfigurationManager::reset() throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->reset();
}

Sequence< Sequence< PropertyValue > > SAL_CALL UIConfigurationManager::getUIElementsInfo( sal_Int16 ElementType )
throw ( IllegalArgumentException, RuntimeException )
{
    return m_pImpl->getUIElementsInfo(ElementType);
}

Reference< XIndexContainer > SAL_CALL UIConfigurationManager::createSettings() throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->createSettings();
}

sal_Bool SAL_CALL UIConfigurationManager::hasSettings( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->hasSettings(ResourceURL);
}

Reference< XIndexAccess > SAL_CALL UIConfigurationManager::getSettings( const ::rtl::OUString& ResourceURL, sal_Bool bWriteable )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->getSettings(ResourceURL,bWriteable);
}

void SAL_CALL UIConfigurationManager::replaceSettings( const ::rtl::OUString& ResourceURL, const Reference< ::com::sun::star::container::XIndexAccess >& aNewData )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->replaceSettings(ResourceURL,aNewData);
}

void SAL_CALL UIConfigurationManager::removeSettings( const ::rtl::OUString& ResourceURL )
throw ( NoSuchElementException, IllegalArgumentException, IllegalAccessException, RuntimeException)
{
    m_pImpl->removeSettings(ResourceURL);
}

void SAL_CALL UIConfigurationManager::insertSettings( const ::rtl::OUString& NewResourceURL, const Reference< XIndexAccess >& aNewData )
throw ( ElementExistException, IllegalArgumentException, IllegalAccessException, RuntimeException )
{
    m_pImpl->insertSettings(NewResourceURL,aNewData);
}

Reference< XInterface > SAL_CALL UIConfigurationManager::getImageManager() throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->getImageManager();
}

Reference< XInterface > SAL_CALL UIConfigurationManager::getShortCutManager() throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->getShortCutManager();
}

Reference< XInterface > SAL_CALL UIConfigurationManager::getEventsManager() throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XInterface >();
}

// XUIConfigurationStorage
void SAL_CALL UIConfigurationManager::setStorage( const Reference< XStorage >& Storage ) throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->setStorage(Storage);
}

sal_Bool SAL_CALL UIConfigurationManager::hasStorage() throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->hasStorage();
}

// XUIConfigurationPersistence
void SAL_CALL UIConfigurationManager::reload() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->reload();
}

void SAL_CALL UIConfigurationManager::store() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->store();
}

void SAL_CALL UIConfigurationManager::storeToStorage( const Reference< XStorage >& Storage ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->storeToStorage(Storage);
}

sal_Bool SAL_CALL UIConfigurationManager::isModified() throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->isModified();
}

sal_Bool SAL_CALL UIConfigurationManager::isReadOnly() throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->isReadOnly();
}

} // namespace framework
