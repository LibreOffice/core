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

#include <helper/uiconfigelementwrapperbase.hxx>
#include <uielement/menubarmanager.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

namespace framework
{

class MenuBarWrapper final : public UIConfigElementWrapperBase,
                       public css::container::XNameAccess

{
    public:
        MenuBarWrapper(
            const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~MenuBarWrapper() override;

        //  XInterface, XTypeProvider
        virtual css::uno::Any  SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() noexcept override;
        virtual void SAL_CALL release() noexcept override;
        virtual css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

        MenuBarManager* GetMenuBarManager() const { return static_cast< MenuBarManager* >( m_xMenuBarManager.get() ); }

        // XComponent
        virtual void SAL_CALL dispose() override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XUIElement
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getRealInterface() override;

        // XUIElementSettings
        virtual void SAL_CALL updateSettings(  ) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType() override;
        virtual sal_Bool SAL_CALL hasElements() override;

        // XNameAccess
        virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    private:
        virtual void impl_fillNewData() override;
        void fillPopupControllerCache();

        bool                                                            m_bRefreshPopupControllerCache : 1;
        css::uno::Reference< css::lang::XComponent >                    m_xMenuBarManager;
        PopupControllerCache                                            m_aPopupControllerCache;
        css::uno::Reference< css::uno::XComponentContext >              m_xContext;
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
