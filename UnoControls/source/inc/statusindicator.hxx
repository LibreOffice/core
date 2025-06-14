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

#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <rtl/ref.hxx>
#include <tools/color.hxx>
#include <basecontainercontrol.hxx>
#include <toolkit/controls/unocontrols.hxx>

namespace com::sun::star::awt { class XFixedText; }

namespace unocontrols {

class ProgressBar;

constexpr auto  STATUSINDICATOR_FREEBORDER = 5; // border around and between the controls
constexpr auto STATUSINDICATOR_DEFAULT_WIDTH = 300;
constexpr auto STATUSINDICATOR_DEFAULT_HEIGHT = 25;
constexpr sal_Int32 STATUSINDICATOR_BACKGROUNDCOLOR = sal_Int32(COL_LIGHTGRAY);
constexpr sal_Int32 STATUSINDICATOR_LINECOLOR_BRIGHT = sal_Int32(COL_WHITE);
constexpr sal_Int32 STATUSINDICATOR_LINECOLOR_SHADOW = sal_Int32(COL_BLACK);

using StatusIndicator_BASE = cppu::ImplInheritanceHelper<BaseContainerControl,
                                                         css::awt::XLayoutConstrains,
                                                         css::task::XStatusIndicator>;
class StatusIndicator final : public StatusIndicator_BASE
{
public:
    StatusIndicator( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual ~StatusIndicator() override;

    //  XStatusIndicator

    virtual void SAL_CALL start(
        const OUString&  sText   ,
        sal_Int32 nRange
    ) override;

    virtual void SAL_CALL end() override;

    virtual void SAL_CALL reset() override;

    virtual void SAL_CALL setText( const OUString& sText ) override;

    virtual void SAL_CALL setValue( sal_Int32 nValue ) override;

    //  XLayoutConstrains

    virtual css::awt::Size SAL_CALL getMinimumSize() override;

    virtual css::awt::Size SAL_CALL getPreferredSize() override;

    virtual css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& aNewSize ) override;

    //  XControl

    virtual void SAL_CALL createPeer(
        const   css::uno::Reference< css::awt::XToolkit >&    xToolkit    ,
        const   css::uno::Reference< css::awt::XWindowPeer >& xParent
    ) override;

    virtual sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& xModel ) override;

    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel() override;

    //  XComponent

    virtual void SAL_CALL dispose() override;

    //  XWindow

    virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) override;

private:
    virtual css::awt::WindowDescriptor impl_getWindowDescriptor(
        const css::uno::Reference< css::awt::XWindowPeer >& xParentPeer
    ) override;

    virtual void impl_paint (
        sal_Int32 nX,
        sal_Int32 nY,
        const css::uno::Reference< css::awt::XGraphics > & rGraphics
    ) override;

    virtual void impl_recalcLayout( const css::awt::WindowEvent& aEvent ) override;

    rtl::Reference< UnoFixedTextControl >         m_xText;
    rtl::Reference<ProgressBar>                   m_xProgressBar;

};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
