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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAWINDOW_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAWINDOW_HXX
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/word/XWindow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbawindowbase.hxx>

typedef cppu::ImplInheritanceHelper< VbaWindowBase, ov::word::XWindow > WindowImpl_BASE;

class SwVbaWindow : public WindowImpl_BASE
{
public:
    /// @throws css::uno::RuntimeException
    SwVbaWindow(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XModel >& xModel,
        const css::uno::Reference< css::frame::XController >& xController );

    // Attributes
    virtual css::uno::Any SAL_CALL getView() override;
    virtual void SAL_CALL setView( const css::uno::Any& _view ) override;
    virtual css::uno::Any SAL_CALL getWindowState() override;
    virtual void SAL_CALL setWindowState( const css::uno::Any& _windowstate ) override;
    // Methods
    virtual void SAL_CALL Activate(  ) override;
    virtual void SAL_CALL Close( const css::uno::Any& SaveChanges, const css::uno::Any& RouteDocument ) override;
    virtual css::uno::Any SAL_CALL Panes( const css::uno::Any& aIndex ) override;
    virtual css::uno::Any SAL_CALL ActivePane() override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
