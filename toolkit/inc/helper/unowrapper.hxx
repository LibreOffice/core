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

#ifndef INCLUDED_TOOLKIT_INC_HELPER_UNOWRAPPER_HXX
#define INCLUDED_TOOLKIT_INC_HELPER_UNOWRAPPER_HXX

#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>

#include <vcl/toolkit/unowrap.hxx>
#include <vcl/window.hxx>


class UnoWrapper final : public UnoWrapperBase
{
private:
    css::uno::Reference< css::awt::XToolkit>  mxToolkit;

public:
    UnoWrapper( const css::uno::Reference< css::awt::XToolkit>& rxToolkit );

    virtual void        Destroy() override;

    // Toolkit
    virtual css::uno::Reference< css::awt::XToolkit> GetVCLToolkit() override;

    // Graphics
    virtual css::uno::Reference< css::awt::XGraphics> CreateGraphics( OutputDevice* pOutDev ) override;
    virtual void        ReleaseAllGraphics( OutputDevice* pOutDev ) override;

    // Window
    virtual css::uno::Reference< css::awt::XVclWindowPeer> GetWindowInterface( vcl::Window* pWindow ) override;
    virtual void        SetWindowInterface( vcl::Window* pWindow, const css::uno::Reference< css::awt::XVclWindowPeer> & xIFace ) override;
    virtual VclPtr<vcl::Window> GetWindow(const css::uno::Reference<css::awt::XWindow>& rxWindow) override;

    // Menu
    virtual css::uno::Reference<css::awt::XPopupMenu> CreateMenuInterface( PopupMenu* pPopupMenu ) override;

    void                WindowDestroyed( vcl::Window* pWindow ) override;

private:
    ~UnoWrapper();
};

#endif // INCLUDED_TOOLKIT_INC_HELPER_UNOWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
