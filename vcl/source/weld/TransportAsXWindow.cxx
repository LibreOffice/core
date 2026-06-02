/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld/TransportAsXWindow.hxx>

namespace weld
{
TransportAsXWindow::TransportAsXWindow(weld::Widget* pWeldWidget, weld::Builder* pWeldWidgetBuilder)
    : m_pWeldWidget(pWeldWidget)
    , m_pWeldWidgetBuilder(pWeldWidgetBuilder)
{
}

void TransportAsXWindow::clear()
{
    m_pWeldWidget = nullptr;
    m_pWeldWidgetBuilder = nullptr;
}

void SAL_CALL TransportAsXWindow::setPosSize(sal_Int32, sal_Int32, sal_Int32, sal_Int32, sal_Int16)
{
    throw css::uno::RuntimeException(u"not implemented"_ustr);
}

css::awt::Rectangle SAL_CALL TransportAsXWindow::getPosSize()
{
    throw css::uno::RuntimeException(u"not implemented"_ustr);
}

void SAL_CALL TransportAsXWindow::setVisible(sal_Bool bVisible)
{
    m_pWeldWidget->set_visible(bVisible);
}

void SAL_CALL TransportAsXWindow::setEnable(sal_Bool bSensitive)
{
    m_pWeldWidget->set_sensitive(bSensitive);
}

void SAL_CALL TransportAsXWindow::setFocus() { m_pWeldWidget->grab_focus(); }

void SAL_CALL TransportAsXWindow::addWindowListener(
    const css::uno::Reference<css::awt::XWindowListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aWindowListeners.addInterface(g, rListener);
}

void SAL_CALL TransportAsXWindow::removeWindowListener(
    const css::uno::Reference<css::awt::XWindowListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aWindowListeners.removeInterface(g, rListener);
}

void SAL_CALL
TransportAsXWindow::addFocusListener(const css::uno::Reference<css::awt::XFocusListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aFocusListeners.addInterface(g, rListener);
}

void SAL_CALL TransportAsXWindow::removeFocusListener(
    const css::uno::Reference<css::awt::XFocusListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aFocusListeners.removeInterface(g, rListener);
}

void SAL_CALL
TransportAsXWindow::addKeyListener(const css::uno::Reference<css::awt::XKeyListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aKeyListeners.addInterface(g, rListener);
}

void SAL_CALL
TransportAsXWindow::removeKeyListener(const css::uno::Reference<css::awt::XKeyListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aKeyListeners.removeInterface(g, rListener);
}

void SAL_CALL
TransportAsXWindow::addMouseListener(const css::uno::Reference<css::awt::XMouseListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aMouseListeners.addInterface(g, rListener);
}

void SAL_CALL TransportAsXWindow::removeMouseListener(
    const css::uno::Reference<css::awt::XMouseListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aMouseListeners.removeInterface(g, rListener);
}

void SAL_CALL TransportAsXWindow::addMouseMotionListener(
    const css::uno::Reference<css::awt::XMouseMotionListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aMotionListeners.addInterface(g, rListener);
}

void SAL_CALL TransportAsXWindow::removeMouseMotionListener(
    const css::uno::Reference<css::awt::XMouseMotionListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aMotionListeners.removeInterface(g, rListener);
}

void SAL_CALL
TransportAsXWindow::addPaintListener(const css::uno::Reference<css::awt::XPaintListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aPaintListeners.addInterface(g, rListener);
}

void SAL_CALL TransportAsXWindow::removePaintListener(
    const css::uno::Reference<css::awt::XPaintListener>& rListener)
{
    std::unique_lock g(m_aMutex);
    m_aPaintListeners.removeInterface(g, rListener);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
