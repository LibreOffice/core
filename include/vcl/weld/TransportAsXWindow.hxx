/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/compbase.hxx>
#include <vcl/dllapi.h>
#include <vcl/weld/Widget.hxx>

#include <com/sun/star/awt/XWindow.hpp>

namespace weld
{
class Builder;

typedef comphelper::WeakComponentImplHelper<css::awt::XWindow> TransportAsXWindow_Base;

class VCL_DLLPUBLIC TransportAsXWindow : public TransportAsXWindow_Base
{
private:
    weld::Widget* m_pWeldWidget;
    weld::Builder* m_pWeldWidgetBuilder;

    comphelper::OInterfaceContainerHelper4<css::awt::XWindowListener> m_aWindowListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XKeyListener> m_aKeyListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XFocusListener> m_aFocusListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XMouseListener> m_aMouseListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XMouseMotionListener> m_aMotionListeners;
    comphelper::OInterfaceContainerHelper4<css::awt::XPaintListener> m_aPaintListeners;

public:
    TransportAsXWindow(weld::Widget* pWeldWidget, weld::Builder* pWeldWidgetBuilder = nullptr);

    weld::Widget* getWidget() const { return m_pWeldWidget; }

    weld::Builder* getBuilder() const { return m_pWeldWidgetBuilder; }

    virtual void clear();

    // css::awt::XWindow
    void SAL_CALL setPosSize(sal_Int32, sal_Int32, sal_Int32, sal_Int32, sal_Int16) override;

    css::awt::Rectangle SAL_CALL getPosSize() override;

    void SAL_CALL setVisible(sal_Bool bVisible) override;

    void SAL_CALL setEnable(sal_Bool bSensitive) override;

    void SAL_CALL setFocus() override;

    void SAL_CALL
    addWindowListener(const css::uno::Reference<css::awt::XWindowListener>& rListener) override;

    void SAL_CALL
    removeWindowListener(const css::uno::Reference<css::awt::XWindowListener>& rListener) override;

    void SAL_CALL
    addFocusListener(const css::uno::Reference<css::awt::XFocusListener>& rListener) override;

    void SAL_CALL
    removeFocusListener(const css::uno::Reference<css::awt::XFocusListener>& rListener) override;

    void SAL_CALL
    addKeyListener(const css::uno::Reference<css::awt::XKeyListener>& rListener) override;

    void SAL_CALL
    removeKeyListener(const css::uno::Reference<css::awt::XKeyListener>& rListener) override;

    void SAL_CALL
    addMouseListener(const css::uno::Reference<css::awt::XMouseListener>& rListener) override;

    void SAL_CALL
    removeMouseListener(const css::uno::Reference<css::awt::XMouseListener>& rListener) override;

    void SAL_CALL addMouseMotionListener(
        const css::uno::Reference<css::awt::XMouseMotionListener>& rListener) override;

    void SAL_CALL removeMouseMotionListener(
        const css::uno::Reference<css::awt::XMouseMotionListener>& rListener) override;

    void SAL_CALL
    addPaintListener(const css::uno::Reference<css::awt::XPaintListener>& rListener) override;

    void SAL_CALL
    removePaintListener(const css::uno::Reference<css::awt::XPaintListener>& rListener) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
