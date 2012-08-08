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


#include <uiconfiguration/imagemanager.hxx>
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

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <comphelper/sequence.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>
#include <rtl/logfile.hxx>

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

// Image sizes for our toolbars/menus
const sal_Int32 IMAGE_SIZE_NORMAL         = 16;
const sal_Int32 IMAGE_SIZE_LARGE          = 26;
const sal_Int16 MAX_IMAGETYPE_VALUE       = ::com::sun::star::ui::ImageType::COLOR_HIGHCONTRAST|
                                            ::com::sun::star::ui::ImageType::SIZE_LARGE;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_MULTISERVICE        (   ImageManager                        ,
                                            ::cppu::OWeakObject                 ,
                                            SERVICENAME_IMAGEMANAGER            ,
                                            IMPLEMENTATIONNAME_IMAGEMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   ImageManager, {} )

ImageManager::ImageManager( uno::Reference< XMultiServiceFactory > xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_pImpl( new ImageManagerImpl(xServiceManager,false) )
{
}

ImageManager::~ImageManager()
{
    m_pImpl->clear();
}

// XComponent
void SAL_CALL ImageManager::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->dispose(static_cast< OWeakObject* >(this));
}

void SAL_CALL ImageManager::addEventListener( const uno::Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->addEventListener(xListener);
}

void SAL_CALL ImageManager::removeEventListener( const uno::Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_pImpl->removeEventListener(xListener);
}

// Non-UNO methods
void ImageManager::setStorage( const uno::Reference< XStorage >& Storage )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_pImpl->m_aLock );

    m_pImpl->m_xUserConfigStorage = Storage;
    m_pImpl->implts_initialize();
}

// XInitialization
void SAL_CALL ImageManager::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    m_pImpl->initialize(aArguments);
}

// XImageManager
void SAL_CALL ImageManager::reset()
throw (::com::sun::star::uno::RuntimeException)
{

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_pImpl->reset(static_cast< OWeakObject* >(this));
}

Sequence< ::rtl::OUString > SAL_CALL ImageManager::getAllImageNames( ::sal_Int16 nImageType )
throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->getAllImageNames( nImageType );
}

::sal_Bool SAL_CALL ImageManager::hasImage( ::sal_Int16 nImageType, const ::rtl::OUString& aCommandURL )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->hasImage(nImageType,aCommandURL);
}

Sequence< uno::Reference< XGraphic > > SAL_CALL ImageManager::getImages(
    ::sal_Int16 nImageType,
    const Sequence< ::rtl::OUString >& aCommandURLSequence )
throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    return m_pImpl->getImages(nImageType,aCommandURLSequence);
}

void SAL_CALL ImageManager::replaceImages(
    ::sal_Int16 nImageType,
    const Sequence< ::rtl::OUString >& aCommandURLSequence,
    const Sequence< uno::Reference< XGraphic > >& aGraphicsSequence )
throw ( ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->replaceImages(static_cast< OWeakObject* >(this),nImageType,aCommandURLSequence,aGraphicsSequence);
}

void SAL_CALL ImageManager::removeImages( ::sal_Int16 nImageType, const Sequence< ::rtl::OUString >& aCommandURLSequence )
throw ( ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->removeImages(static_cast< OWeakObject* >(this),nImageType,aCommandURLSequence);
}

void SAL_CALL ImageManager::insertImages( ::sal_Int16 nImageType, const Sequence< ::rtl::OUString >& aCommandURLSequence, const Sequence< uno::Reference< XGraphic > >& aGraphicSequence )
throw ( ::com::sun::star::container::ElementExistException,
        ::com::sun::star::lang::IllegalArgumentException,
        ::com::sun::star::lang::IllegalAccessException,
        ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->insertImages(static_cast< OWeakObject* >(this),nImageType,aCommandURLSequence,aGraphicSequence);
}

// XUIConfiguration
void SAL_CALL ImageManager::addConfigurationListener( const uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    m_pImpl->addConfigurationListener(xListener);
}

void SAL_CALL ImageManager::removeConfigurationListener( const uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_pImpl->removeConfigurationListener(xListener);
}

// XUIConfigurationPersistence
void SAL_CALL ImageManager::reload()
throw ( ::com::sun::star::uno::Exception,
        ::com::sun::star::uno::RuntimeException )
{
    m_pImpl->reload(static_cast< OWeakObject* >(this));
}

void SAL_CALL ImageManager::store()
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->store();
}

void SAL_CALL ImageManager::storeToStorage( const uno::Reference< XStorage >& Storage )
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    m_pImpl->storeToStorage(Storage);
}

sal_Bool SAL_CALL ImageManager::isModified()
throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->isModified();
}

sal_Bool SAL_CALL ImageManager::isReadOnly() throw (::com::sun::star::uno::RuntimeException)
{
    return m_pImpl->isReadOnly();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
