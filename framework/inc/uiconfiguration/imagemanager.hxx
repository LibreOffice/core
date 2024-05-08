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

#pragma once

#include <memory>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustring.hxx>

namespace framework
{
    class ImageManagerImpl;
    class ImageManager final : public ::cppu::WeakImplHelper< css::ui::XImageManager, css::lang::XServiceInfo>
    {
        public:
            ImageManager( const css::uno::Reference< css::uno::XComponentContext >& rxContext, bool bForModule );
            virtual ~ImageManager() override;

            virtual OUString SAL_CALL getImplementationName() override
            {
                return u"com.sun.star.comp.framework.ImageManager"_ustr;
            }

            virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
            {
                return cppu::supportsService(this, ServiceName);
            }

            virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
            {
                css::uno::Sequence< OUString > aSeq { u"com.sun.star.ui.ImageManager"_ustr };
                return aSeq;
            }

            // XComponent
            virtual void SAL_CALL dispose() override;
            virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
            virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

            // XInitialization
            virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

            // XImageManager
            virtual void SAL_CALL reset() override;
            virtual css::uno::Sequence< OUString > SAL_CALL getAllImageNames( ::sal_Int16 nImageType ) override;
            virtual sal_Bool SAL_CALL hasImage( ::sal_Int16 nImageType, const OUString& aCommandURL ) override;
            virtual css::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > > SAL_CALL getImages( ::sal_Int16 nImageType, const css::uno::Sequence< OUString >& aCommandURLSequence ) override;
            virtual void SAL_CALL replaceImages( ::sal_Int16 nImageType, const css::uno::Sequence< OUString >& aCommandURLSequence, const css::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > >& aGraphicsSequence ) override;
            virtual void SAL_CALL removeImages( ::sal_Int16 nImageType, const css::uno::Sequence< OUString >& aResourceURLSequence ) override;
            virtual void SAL_CALL insertImages( ::sal_Int16 nImageType, const css::uno::Sequence< OUString >& aCommandURLSequence, const css::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > >& aGraphicSequence ) override;

            // XUIConfiguration
            virtual void SAL_CALL addConfigurationListener( const css::uno::Reference< css::ui::XUIConfigurationListener >& Listener ) override;
            virtual void SAL_CALL removeConfigurationListener( const css::uno::Reference< css::ui::XUIConfigurationListener >& Listener ) override;

            // XUIConfigurationPersistence
            virtual void SAL_CALL reload() override;
            virtual void SAL_CALL store() override;
            virtual void SAL_CALL storeToStorage( const css::uno::Reference< css::embed::XStorage >& Storage ) override;
            virtual sal_Bool SAL_CALL isModified() override;
            virtual sal_Bool SAL_CALL isReadOnly() override;

            // Non-UNO methods
            /// @throws css::uno::RuntimeException
            void setStorage( const css::uno::Reference< css::embed::XStorage >& Storage );

        private:
            ::std::unique_ptr<ImageManagerImpl> m_pImpl;
   };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
