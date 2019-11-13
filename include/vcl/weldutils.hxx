/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_WELDUTILS_HXX
#define INCLUDED_VCL_WELDUTILS_HXX

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/compbase.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>

namespace weld
{
class Toolbar;

class VCL_DLLPUBLIC ToolbarUnoDispatcher
{
private:
    css::uno::Reference<css::frame::XFrame> m_xFrame;
    DECL_LINK(SelectHdl, const OString&, void);

public:
    // fill in the label and icons for actions and dispatch the action on item click
    ToolbarUnoDispatcher(Toolbar& rToolbar, const css::uno::Reference<css::frame::XFrame>& rFrame);
};

typedef cppu::WeakComponentImplHelper<css::awt::XWindow> TransportAsXWindow_Base;

class VCL_DLLPUBLIC TransportAsXWindow : public TransportAsXWindow_Base
{
private:
    osl::Mutex m_aHelperMtx;
    weld::Widget* m_pWeldWidget;

    comphelper::OInterfaceContainerHelper2 m_aWindowListeners;
    comphelper::OInterfaceContainerHelper2 m_aKeyListeners;
    comphelper::OInterfaceContainerHelper2 m_aFocusListeners;
    comphelper::OInterfaceContainerHelper2 m_aMouseListeners;
    comphelper::OInterfaceContainerHelper2 m_aMotionListeners;
    comphelper::OInterfaceContainerHelper2 m_aPaintListeners;

public:
    TransportAsXWindow(weld::Widget* pWeldWidget)
        : TransportAsXWindow_Base(m_aHelperMtx)
        , m_pWeldWidget(pWeldWidget)
        , m_aWindowListeners(m_aHelperMtx)
        , m_aKeyListeners(m_aHelperMtx)
        , m_aFocusListeners(m_aHelperMtx)
        , m_aMouseListeners(m_aHelperMtx)
        , m_aMotionListeners(m_aHelperMtx)
        , m_aPaintListeners(m_aHelperMtx)
    {
    }

    weld::Widget* getWidget() const { return m_pWeldWidget; }

    virtual void clear() { m_pWeldWidget = nullptr; }

    // css::awt::XWindow
    void SAL_CALL setPosSize(sal_Int32, sal_Int32, sal_Int32, sal_Int32, sal_Int16) override
    {
        throw css::uno::RuntimeException("not implemented");
    }

    css::awt::Rectangle SAL_CALL getPosSize() override
    {
        throw css::uno::RuntimeException("not implemented");
    }

    void SAL_CALL setVisible(sal_Bool bVisible) override { m_pWeldWidget->set_visible(bVisible); }

    void SAL_CALL setEnable(sal_Bool bSensitive) override
    {
        m_pWeldWidget->set_sensitive(bSensitive);
    }

    void SAL_CALL setFocus() override { m_pWeldWidget->grab_focus(); }

    void SAL_CALL
    addWindowListener(const css::uno::Reference<css::awt::XWindowListener>& rListener) override
    {
        m_aWindowListeners.addInterface(rListener);
    }

    void SAL_CALL
    removeWindowListener(const css::uno::Reference<css::awt::XWindowListener>& rListener) override
    {
        m_aWindowListeners.removeInterface(rListener);
    }

    void SAL_CALL
    addFocusListener(const css::uno::Reference<css::awt::XFocusListener>& rListener) override
    {
        m_aFocusListeners.addInterface(rListener);
    }

    void SAL_CALL
    removeFocusListener(const css::uno::Reference<css::awt::XFocusListener>& rListener) override
    {
        m_aFocusListeners.removeInterface(rListener);
    }

    void SAL_CALL
    addKeyListener(const css::uno::Reference<css::awt::XKeyListener>& rListener) override
    {
        m_aKeyListeners.addInterface(rListener);
    }

    void SAL_CALL
    removeKeyListener(const css::uno::Reference<css::awt::XKeyListener>& rListener) override
    {
        m_aKeyListeners.removeInterface(rListener);
    }

    void SAL_CALL
    addMouseListener(const css::uno::Reference<css::awt::XMouseListener>& rListener) override
    {
        m_aMouseListeners.addInterface(rListener);
    }

    void SAL_CALL
    removeMouseListener(const css::uno::Reference<css::awt::XMouseListener>& rListener) override
    {
        m_aMouseListeners.removeInterface(rListener);
    }

    void SAL_CALL addMouseMotionListener(
        const css::uno::Reference<css::awt::XMouseMotionListener>& rListener) override
    {
        m_aMotionListeners.addInterface(rListener);
    }

    void SAL_CALL removeMouseMotionListener(
        const css::uno::Reference<css::awt::XMouseMotionListener>& rListener) override
    {
        m_aMotionListeners.removeInterface(rListener);
    }

    void SAL_CALL
    addPaintListener(const css::uno::Reference<css::awt::XPaintListener>& rListener) override
    {
        m_aPaintListeners.addInterface(rListener);
    }

    void SAL_CALL
    removePaintListener(const css::uno::Reference<css::awt::XPaintListener>& rListener) override
    {
        m_aPaintListeners.removeInterface(rListener);
    }
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
