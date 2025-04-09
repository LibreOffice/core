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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXTOPWINDOW_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXTOPWINDOW_HXX

#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/XTopWindow3.hpp>

#include <cppuhelper/implbase.hxx>

#include <awt/vclxcontainer.hxx>


class VCLXTopWindow: public cppu::ImplInheritanceHelper<
                        VCLXContainer, css::awt::XTopWindow3, css::awt::XSystemDependentWindowPeer >
{
public:
    VCLXTopWindow();
    virtual ~VCLXTopWindow() override;

    // css::awt::XSystemDependentWindowPeer
    css::uno::Any SAL_CALL getWindowHandle( const css::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) override;

    // css::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& rxListener ) override;
    void SAL_CALL removeTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& rxListener ) override;
    void SAL_CALL toFront() override;
    void SAL_CALL toBack() override;
    void SAL_CALL setMenuBar( const css::uno::Reference< css::awt::XMenuBar >& xMenu ) override;

    // XTopWindow2
    virtual sal_Bool SAL_CALL getIsMaximized() override;
    virtual void SAL_CALL setIsMaximized( sal_Bool _ismaximized ) override;
    virtual sal_Bool SAL_CALL getIsMinimized() override;
    virtual void SAL_CALL setIsMinimized( sal_Bool _isminimized ) override;
    virtual ::sal_Int32 SAL_CALL getDisplay() override;
    virtual void SAL_CALL setDisplay( ::sal_Int32 _display ) override;

    // XTopWindow3
    virtual sal_Bool SAL_CALL getFullScreen() override;
    virtual void SAL_CALL setFullScreen(sal_Bool value) override;

    static void     ImplGetPropertyIds( std::vector< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }
};


#endif // INCLUDED_TOOLKIT_AWT_VCLXTOPWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
