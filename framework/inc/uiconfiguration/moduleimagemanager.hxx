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

#ifndef INCLUDED_FRAMEWORK_INC_UICONFIGURATION_MODULEIMAGEMANAGER_HXX
#define INCLUDED_FRAMEWORK_INC_UICONFIGURATION_MODULEIMAGEMANAGER_HXX

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <uiconfiguration/imagetype.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <rtl/ustring.hxx>

#include <vcl/image.hxx>
#include <rtl/ref.hxx>

#include <list>
#include <memory>
#include <vector>

namespace framework
{
    class ImageManagerImpl;

    class ModuleImageManager :    public ::cppu::WeakImplHelper< ::com::sun::star::ui::XImageManager>
    {
        public:
            ModuleImageManager( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& xContext );
            virtual ~ModuleImageManager();

            // XComponent
            virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            // XInitialization
            virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

            // XImageManager
            virtual void SAL_CALL reset()
                throw (css::lang::IllegalAccessException,
                       css::uno::RuntimeException,
                       std::exception) override;
            virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAllImageNames( ::sal_Int16 nImageType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL hasImage( ::sal_Int16 nImageType, const OUString& aCommandURL ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > > SAL_CALL getImages( ::sal_Int16 nImageType, const ::com::sun::star::uno::Sequence< OUString >& aCommandURLSequence ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL replaceImages( ::sal_Int16 nImageType, const ::com::sun::star::uno::Sequence< OUString >& aCommandURLSequence, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > >& aGraphicsSequence ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL removeImages( ::sal_Int16 nImageType, const ::com::sun::star::uno::Sequence< OUString >& aResourceURLSequence ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL insertImages( ::sal_Int16 nImageType, const ::com::sun::star::uno::Sequence< OUString >& aCommandURLSequence, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > >& aGraphicSequence ) throw (::com::sun::star::container::ElementExistException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException, std::exception) override;

            // XUIConfiguration
            virtual void SAL_CALL addConfigurationListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL removeConfigurationListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >& Listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

            // XUIConfigurationPersistence
            virtual void SAL_CALL reload() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL store() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual void SAL_CALL storeToStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isModified() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL isReadOnly() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        private:
            ::std::unique_ptr<ImageManagerImpl>                                               m_pImpl;
   };
}

#endif // INCLUDED_FRAMEWORK_INC_UICONFIGURATION_MODULEIMAGEMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
