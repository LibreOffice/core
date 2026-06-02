/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/compbase.hxx>
#include <vcl/dllapi.h>
#include <vcl/timer.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld/weld.hxx>

enum class SelectionOptions;

namespace com::sun::star::frame
{
class XDispatch;
}
namespace com::sun::star::frame
{
class XFrame;
}

namespace tools
{
class Time;
}

namespace vcl
{
class Window;
}

namespace weld
{
class Builder;
class TextWidget;
class TreeIter;
class TreeView;
class Window;

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
    TransportAsXWindow(weld::Widget* pWeldWidget, weld::Builder* pWeldWidgetBuilder = nullptr)
        : m_pWeldWidget(pWeldWidget)
        , m_pWeldWidgetBuilder(pWeldWidgetBuilder)
    {
    }

    weld::Widget* getWidget() const { return m_pWeldWidget; }

    weld::Builder* getBuilder() const { return m_pWeldWidgetBuilder; }

    virtual void clear()
    {
        m_pWeldWidget = nullptr;
        m_pWeldWidgetBuilder = nullptr;
    }

    // css::awt::XWindow
    void SAL_CALL setPosSize(sal_Int32, sal_Int32, sal_Int32, sal_Int32, sal_Int16) override
    {
        throw css::uno::RuntimeException(u"not implemented"_ustr);
    }

    css::awt::Rectangle SAL_CALL getPosSize() override
    {
        throw css::uno::RuntimeException(u"not implemented"_ustr);
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
        std::unique_lock g(m_aMutex);
        m_aWindowListeners.addInterface(g, rListener);
    }

    void SAL_CALL
    removeWindowListener(const css::uno::Reference<css::awt::XWindowListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aWindowListeners.removeInterface(g, rListener);
    }

    void SAL_CALL
    addFocusListener(const css::uno::Reference<css::awt::XFocusListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aFocusListeners.addInterface(g, rListener);
    }

    void SAL_CALL
    removeFocusListener(const css::uno::Reference<css::awt::XFocusListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aFocusListeners.removeInterface(g, rListener);
    }

    void SAL_CALL
    addKeyListener(const css::uno::Reference<css::awt::XKeyListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aKeyListeners.addInterface(g, rListener);
    }

    void SAL_CALL
    removeKeyListener(const css::uno::Reference<css::awt::XKeyListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aKeyListeners.removeInterface(g, rListener);
    }

    void SAL_CALL
    addMouseListener(const css::uno::Reference<css::awt::XMouseListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aMouseListeners.addInterface(g, rListener);
    }

    void SAL_CALL
    removeMouseListener(const css::uno::Reference<css::awt::XMouseListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aMouseListeners.removeInterface(g, rListener);
    }

    void SAL_CALL addMouseMotionListener(
        const css::uno::Reference<css::awt::XMouseMotionListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aMotionListeners.addInterface(g, rListener);
    }

    void SAL_CALL removeMouseMotionListener(
        const css::uno::Reference<css::awt::XMouseMotionListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aMotionListeners.removeInterface(g, rListener);
    }

    void SAL_CALL
    addPaintListener(const css::uno::Reference<css::awt::XPaintListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aPaintListeners.addInterface(g, rListener);
    }

    void SAL_CALL
    removePaintListener(const css::uno::Reference<css::awt::XPaintListener>& rListener) override
    {
        std::unique_lock g(m_aMutex);
        m_aPaintListeners.removeInterface(g, rListener);
    }
};

// don't export to avoid duplicate WeakImplHelper definitions with MSVC
class SAL_DLLPUBLIC_TEMPLATE WidgetStatusListener_Base
    : public cppu::WeakImplHelper<css::frame::XStatusListener>
{
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) WidgetStatusListener final : public WidgetStatusListener_Base
{
public:
    WidgetStatusListener(weld::Widget* widget, const OUString& rCommand);

private:
    weld::Widget* mWidget; /** The widget on which actions are performed */

    /** Dispatcher. Need to keep a reference to it as long as this StatusListener exists. */
    css::uno::Reference<css::frame::XDispatch> mxDispatch;
    css::util::URL maCommandURL;
    css::uno::Reference<css::frame::XFrame> mxFrame;

public:
    SAL_DLLPRIVATE void SAL_CALL
    statusChanged(const css::frame::FeatureStateEvent& rEvent) override;

    SAL_DLLPRIVATE void SAL_CALL disposing(const css::lang::EventObject& /*Source*/) override;

    const css::uno::Reference<css::frame::XFrame>& getFrame() const { return mxFrame; }

    void startListening();

    void dispose();
};

class VCL_DLLPUBLIC ButtonPressRepeater final
    : public std::enable_shared_from_this<ButtonPressRepeater>
{
private:
    weld::Button& m_rButton;
    AutoTimer m_aRepeat;
    const Link<Button&, void> m_aLink;
    const Link<const CommandEvent&, void> m_aContextLink;
    bool m_bModKey;

    DECL_DLLPRIVATE_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_DLLPRIVATE_LINK(MouseReleaseHdl, const MouseEvent&, bool);
    DECL_DLLPRIVATE_LINK(RepeatTimerHdl, Timer*, void);

public:
    ButtonPressRepeater(weld::Button& rButton, const Link<Button&, void>& rLink,
                        const Link<const CommandEvent&, void>& rContextLink
                        = Link<const CommandEvent&, void>());
    void Stop() { m_aRepeat.Stop(); }
    bool IsModKeyPressed() const { return m_bModKey; }
};

/*
  If a TreeView is used as a list, rather than a tree, and DnD should just
  reorder rows, then this can be used to implement that.

  Because the TreeView doesn't want or need subnodes, the drop target can be
  simply visually indicated as being between rows (the issue of a final drop
  location of a child of the drop target doesn't arise), and the meaning of
  what a drop before or after the last row should do is unambiguous.
*/
class VCL_DLLPUBLIC ReorderingDropTarget : public DropTargetHelper
{
protected:
    weld::TreeView& m_rTreeView;

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override;
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override;

public:
    ReorderingDropTarget(weld::TreeView& rTreeView);
};

// get the row the iterator is on
VCL_DLLPUBLIC size_t GetAbsPos(const weld::TreeView& rTreeView, const weld::TreeIter& rIter);

// an entry is visible if all parents are expanded
VCL_DLLPUBLIC bool IsEntryVisible(const weld::TreeView& rTreeView, const weld::TreeIter& rIter);

// A Parent's Children are turned into Children of the Parent which comes next in hierarchy
VCL_DLLPUBLIC void RemoveParentKeepChildren(weld::TreeView& rTreeView,
                                            const weld::TreeIter& rParent);

// return the weld::Window of the SalFrame rOutWin is in, and convert rRect
// from relative to rOutWin to relative to that weld::Window suitable for use
// with popup_at_rect
VCL_DLLPUBLIC weld::Window* GetPopupParent(vcl::Window& rOutWin, tools::Rectangle& rRect);

// Use Application::GetDefaultDevice to set the PointFont rFont to the OutputDevice
VCL_DLLPUBLIC void SetPointFont(OutputDevice& rDevice, const vcl::Font& rFont,
                                bool bUseDeviceDPI = false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
