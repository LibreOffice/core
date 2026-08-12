/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

            virtual OUString getImplementationName() override
            {
                return u"com.sun.star.comp.framework.ImageManager"_ustr;
            }

            virtual bool supportsService(OUString const & ServiceName) override
            {
                return cppu::supportsService(this, ServiceName);
            }

            virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override
            {
                cpo::uno::Sequence< OUString > aSeq { u"com.sun.star.ui.ImageManager"_ustr };
                return aSeq;
            }

            // XComponent
            virtual void dispose() override;
            virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
            virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

            // XInitialization
            virtual void initialize( const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;

            // XImageManager
            virtual void reset() override;
            virtual cpo::uno::Sequence< OUString > getAllImageNames( ::sal_Int16 nImageType ) override;
            virtual bool hasImage( ::sal_Int16 nImageType, const OUString& aCommandURL ) override;
            virtual cpo::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > > getImages( ::sal_Int16 nImageType, const cpo::uno::Sequence< OUString >& aCommandURLSequence ) override;
            virtual void replaceImages( ::sal_Int16 nImageType, const cpo::uno::Sequence< OUString >& aCommandURLSequence, const cpo::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > >& aGraphicsSequence ) override;
            virtual void removeImages( ::sal_Int16 nImageType, const cpo::uno::Sequence< OUString >& aResourceURLSequence ) override;
            virtual void insertImages( ::sal_Int16 nImageType, const cpo::uno::Sequence< OUString >& aCommandURLSequence, const cpo::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > >& aGraphicSequence ) override;

            // XUIConfiguration
            virtual void addConfigurationListener( const css::uno::Reference< css::ui::XUIConfigurationListener >& Listener ) override;
            virtual void removeConfigurationListener( const css::uno::Reference< css::ui::XUIConfigurationListener >& Listener ) override;

            // XUIConfigurationPersistence
            virtual void reload() override;
            virtual void store() override;
            virtual void storeToStorage( const css::uno::Reference< css::embed::XStorage >& Storage ) override;
            virtual bool isModified() override;
            virtual bool isReadOnly() override;

            // Non-UNO methods
            /// @throws css::uno::RuntimeException
            void setStorage( const css::uno::Reference< css::embed::XStorage >& Storage );

        private:
            ::std::unique_ptr<ImageManagerImpl> m_pImpl;
   };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
