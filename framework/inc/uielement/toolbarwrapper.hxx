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

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ui/XUIFunctionListener.hpp>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>

namespace weld
{
    class Builder;
    class Container;
    class Toolbar;
}

namespace framework
{

class ToolBarWrapper final : public cppu::ImplInheritanceHelper<UIConfigElementWrapperBase,
                                                                css::ui::XUIFunctionListener,
                                                                css::ui::XContextChangeEventListener>
{
    public:
        ToolBarWrapper( const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~ToolBarWrapper() override;

        // XComponent
        virtual void SAL_CALL dispose() override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XUIElement
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getRealInterface() override;

        // XUpdatable
        virtual void SAL_CALL update() override;

        // XUIElementSettings
        virtual void SAL_CALL updateSettings() override;

        // XUIFunctionListener
        virtual void SAL_CALL functionExecute( const OUString& aUIElementName, const OUString& aCommand ) override;

        // XContextChangeEventListener
        virtual void SAL_CALL notifyContextChangeEvent( const css::ui::ContextChangeEventObject& aEvent ) override;

        // XEventListener
        using cppu::OPropertySetHelper::disposing;
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override;

    private:
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any&  aValue ) override;
        virtual void impl_fillNewData() override;

        css::uno::Reference< css::lang::XComponent >            m_xToolBarManager;
        css::uno::Reference< css::uno::XComponentContext >      m_xContext;
        css::uno::Reference< css::ui::XUIElement >              m_xSubElement;

        std::unique_ptr<weld::Builder>                          m_xBuilder;
        std::unique_ptr<weld::Container>                        m_xTopLevel;
        std::unique_ptr<weld::Toolbar>                          m_xWeldedToolbar;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
