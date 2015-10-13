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

#ifndef INCLUDED_FRAMEWORK_INC_UIELEMENT_STATUSBARWRAPPER_HXX
#define INCLUDED_FRAMEWORK_INC_UIELEMENT_STATUSBARWRAPPER_HXX

#include <helper/uiconfigelementwrapperbase.hxx>
#include <uielement/statusbarmanager.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace framework
{

class StatusBarWrapper : public UIConfigElementWrapperBase
{
    public:
        StatusBarWrapper(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~StatusBarWrapper();

        // XComponent
        virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XUIElement
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getRealInterface() throw (css::uno::RuntimeException, std::exception) override;

        // XUIElementSettings
        virtual void SAL_CALL updateSettings() throw (css::uno::RuntimeException, std::exception) override;

    private:
        css::uno::Reference< css::lang::XComponent >             m_xStatusBarManager;
        css::uno::Reference< css::uno::XComponentContext >       m_xContext;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_UIELEMENT_STATUSBARWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
