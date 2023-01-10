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

#include <com/sun/star/embed/XHatchWindow.hpp>

#include <toolkit/awt/vclxwindow.hxx>

class SvResizeWindow;
typedef cppu::ImplInheritanceHelper< VCLXWindow, css::embed::XHatchWindow> VCLXHatchWindow_Base;
class VCLXHatchWindow : public VCLXHatchWindow_Base
{
    css::uno::Reference< css::embed::XHatchWindowController > m_xController;
    css::awt::Size aHatchBorderSize;
    VclPtr<SvResizeWindow> pHatchWindow;

public:
    VCLXHatchWindow();
    virtual ~VCLXHatchWindow() override;

    void initializeWindow( const css::uno::Reference< css::awt::XWindowPeer >& xParent,
                const css::awt::Rectangle& aBounds,
                const css::awt::Size& aSize );

    void QueryObjAreaPixel( tools::Rectangle & );
    void RequestObjAreaPixel( const tools::Rectangle & );
    void InplaceDeactivate();
    void Activated();
    void Deactivated();

    // XHatchWindow
    virtual void SAL_CALL setController( const css::uno::Reference< css::embed::XHatchWindowController >& xController ) override;
    virtual css::awt::Size SAL_CALL getHatchBorderSize() override;
    virtual void SAL_CALL setHatchBorderSize( const css::awt::Size& _hatchbordersize ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
