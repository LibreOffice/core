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

#ifndef INCLUDED_FRAMEWORK_INC_UIFACTORY_MENUBARFACTORY_HXX
#define INCLUDED_FRAMEWORK_INC_UIFACTORY_MENUBARFACTORY_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/XUIElementFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <toolkit/awt/vclxmenu.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustring.hxx>

namespace framework
{
typedef ::cppu::WeakImplHelper<
    css::lang::XServiceInfo,
    css::ui::XUIElementFactory> MenuBarFactory_BASE;

    class MenuBarFactory : public MenuBarFactory_BASE
    {
        public:
            MenuBarFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );
            virtual ~MenuBarFactory();

            virtual OUString SAL_CALL getImplementationName()
                throw (css::uno::RuntimeException, std::exception) override
            {
                return OUString("com.sun.star.comp.framework.MenuBarFactory");
            }

            virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
                throw (css::uno::RuntimeException, std::exception) override
            {
                return cppu::supportsService(this, ServiceName);
            }

            virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
                throw (css::uno::RuntimeException, std::exception) override
            {
                css::uno::Sequence< OUString > aSeq { "com.sun.star.ui.UIElementFactory" };
                return aSeq;
            }

            // XUIElementFactory
            virtual css::uno::Reference< css::ui::XUIElement > SAL_CALL createUIElement( const OUString& ResourceURL, const css::uno::Sequence< css::beans::PropertyValue >& Args ) throw ( css::container::NoSuchElementException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;

            static void CreateUIElement(const OUString& ResourceURL
                        ,const css::uno::Sequence< css::beans::PropertyValue >& Args
                        ,const char* _pExtraMode
                        ,const OUString& ResourceType
                        ,const css::uno::Reference< css::ui::XUIElement >& _xMenuBar
                        ,const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

        protected:
            css::uno::Reference< css::uno::XComponentContext >     m_xContext;
    };
}

#endif // INCLUDED_FRAMEWORK_INC_UIFACTORY_MENUBARFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
