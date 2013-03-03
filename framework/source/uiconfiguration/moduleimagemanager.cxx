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


#include <rtl/logfile.hxx>
#include <uiconfiguration/moduleimagemanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include <xml/imagesconfiguration.hxx>
#include <uiconfiguration/graphicnameaccess.hxx>
#include <services.h>
#include "imagemanagerimpl.hxx"

#include "properties.h"

#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <comphelper/sequence.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::graphic::XGraphic;
using namespace ::com::sun::star;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ui;

namespace framework
{
ModuleImageManager::ModuleImageManager( uno::Reference< XMultiServiceFactory > xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_pImpl( new ImageManagerImpl(comphelper::getComponentContext(xServiceManager),static_cast< OWeakObject* >(this),true) )
{
}

ModuleImageManager::~ModuleImageManager()
{
}

// XComponent
void SAL_CALL ModuleImageManager::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->dispose();
}

void SAL_CALL ModuleImageManager::addEventListener( const uno::Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->addEventListener(xListener);
}

void SAL_CALL ModuleImageManager::removeEventListener( const uno::Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_pImpl->removeEventListener(xListener);
}

// XInitialization
void SAL_CALL ModuleImageManager::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    m_pImpl->initialize(aArguments);
}

// XImageManager
void SAL_CALL ModuleImageManager::reset()
throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->reset();
}

Sequence< ::rtl::OUString > SAL_CALL ModuleImageManager::getAllImageNames( ::sal_Int16 nImageType )
throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->getAllImageNames( nImageType );
}

::sal_Bool SAL_CALL ModuleImageManager::hasImage( ::sal_Int16 nImageType, const ::rtl::OUString& aCommandURL )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->hasImage(nImageType,aCommandURL);
}

Sequence< uno::Reference< XGraphic > > SAL_CALL ModuleImageManager::getImages(
    ::sal_Int16 nImageType,
    const Sequence< ::rtl::OUString >& aCommandURLSequence )
throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "framework: ModuleImageManager::getImages" );
    return m_pImpl->getImages(nImageType,aCommandURLSequence);
}

void SAL_CALL ModuleImageManager::replaceImages(
    ::sal_Int16 nImageType,
    const Sequence< ::rtl::OUString >& aCommandURLSequence,
    const Sequence< uno::Reference< XGraphic > >& aGraphicsSequence )
throw ( ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->replaceImages(nImageType,aCommandURLSequence,aGraphicsSequence);
}

void SAL_CALL ModuleImageManager::removeImages( ::sal_Int16 nImageType, const Sequence< ::rtl::OUString >& aCommandURLSequence )
throw ( ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->removeImages(nImageType,aCommandURLSequence);
}

void SAL_CALL ModuleImageManager::insertImages( ::sal_Int16 nImageType, const Sequence< ::rtl::OUString >& aCommandURLSequence, const Sequence< uno::Reference< XGraphic > >& aGraphicSequence )
throw ( ::com::sun::star::container::ElementExistException,
        ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->insertImages(nImageType,aCommandURLSequence,aGraphicSequence);
}

// XUIConfiguration
void SAL_CALL ModuleImageManager::addConfigurationListener( const uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->addConfigurationListener(xListener);
}

void SAL_CALL ModuleImageManager::removeConfigurationListener( const uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->removeConfigurationListener(xListener);
}

// XUIConfigurationPersistence
void SAL_CALL ModuleImageManager::reload()
throw ( ::com::sun::star::uno::Exception,
        ::com::sun::star::uno::RuntimeException )
{
    m_pImpl->reload();
}

void SAL_CALL ModuleImageManager::store()
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->store();
}

void SAL_CALL ModuleImageManager::storeToStorage( const uno::Reference< XStorage >& Storage )
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->storeToStorage(Storage);
}

sal_Bool SAL_CALL ModuleImageManager::isModified()
throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->isModified();
}

sal_Bool SAL_CALL ModuleImageManager::isReadOnly() throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->isReadOnly();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
