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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARWRAPPER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARWRAPPER_HXX

#include <helper/uiconfigelementwrapperbase.hxx>
#include <uielement/menubarmanager.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

namespace framework
{

class MenuBarWrapper : public UIConfigElementWrapperBase,
                       public css::container::XNameAccess

{
    public:
        MenuBarWrapper(
            const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~MenuBarWrapper();

        //  XInterface, XTypeProvider

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        MenuBarManager* GetMenuBarManager() const { return static_cast< MenuBarManager* >( m_xMenuBarManager.get() ); }

        // XComponent
        virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XUIElement
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getRealInterface() throw (css::uno::RuntimeException, std::exception) override;

        // XUIElementSettings
        virtual void SAL_CALL updateSettings(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException, std::exception) override;

        // XNameAccess
        virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (css::uno::RuntimeException, std::exception) override;

    private:
        virtual void impl_fillNewData() override;
        void fillPopupControllerCache();

        bool                                                            m_bRefreshPopupControllerCache : 1;
        css::uno::Reference< css::lang::XComponent >                    m_xMenuBarManager;
        PopupControllerCache                                            m_aPopupControllerCache;
        css::uno::Reference< css::uno::XComponentContext >              m_xContext;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_MENUBARWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
