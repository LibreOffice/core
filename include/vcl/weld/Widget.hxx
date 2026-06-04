/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/dllapi.h>

#include <assert.h>
#include <memory>

namespace com::sun::star::datatransfer::clipboard
{
class XClipboard;
}
namespace com::sun::star::datatransfer::dnd
{
class XDropTarget;
}
class Color;
class CommandEvent;
class KeyEvent;
class MouseEvent;
class TransferDataContainer;
class OutputDevice;
class VirtualDevice;

namespace vcl
{
class ILibreOfficeKitNotifier;
class Font;
}
template <class reference_type> class VclPtr;

namespace weld
{
class Container;

class VCL_DLLPUBLIC Widget
{
    int m_nBlockNotify = 0;

    Link<const CommandEvent&, bool> m_aCommandHdl;

protected:
    Link<Widget&, void> m_aFocusInHdl;
    Link<Widget&, void> m_aFocusOutHdl;
    Link<Widget&, bool> m_aMnemonicActivateHdl;
    Link<Widget&, void> m_aStyleUpdatedHdl;
    Link<const Size&, void> m_aSizeAllocateHdl;
    Link<const KeyEvent&, bool> m_aKeyPressHdl;
    Link<const KeyEvent&, bool> m_aKeyReleaseHdl;
    Link<const MouseEvent&, bool> m_aMousePressHdl;
    Link<const MouseEvent&, bool> m_aMouseMotionHdl;
    Link<const MouseEvent&, bool> m_aMouseReleaseHdl;

    void disable_notify_events() { ++m_nBlockNotify; }
    bool notify_events_disabled() const { return m_nBlockNotify != 0; }
    void enable_notify_events() { --m_nBlockNotify; }

    virtual bool signal_command(const CommandEvent& rCEvt) { return m_aCommandHdl.Call(rCEvt); }
    void signal_focus_in() { m_aFocusInHdl.Call(*this); }
    void signal_focus_out() { m_aFocusOutHdl.Call(*this); }
    bool signal_mnemonic_activate() { return m_aMnemonicActivateHdl.Call(*this); }
    void signal_style_updated() { m_aStyleUpdatedHdl.Call(*this); }
    void signal_size_allocate(const Size& rSize) { m_aSizeAllocateHdl.Call(rSize); }
    bool signal_key_press(const KeyEvent& rKeyEvent) { return m_aKeyPressHdl.Call(rKeyEvent); }
    bool signal_key_release(const KeyEvent& rKeyEvent) { return m_aKeyReleaseHdl.Call(rKeyEvent); }
    bool signal_mouse_press(const MouseEvent& rMouseEvent)
    {
        return m_aMousePressHdl.Call(rMouseEvent);
    }
    bool signal_mouse_motion(const MouseEvent& rMouseEvent)
    {
        return m_aMouseMotionHdl.Call(rMouseEvent);
    }
    bool signal_mouse_release(const MouseEvent& rMouseEvent)
    {
        return m_aMouseReleaseHdl.Call(rMouseEvent);
    }

public:
    virtual void set_sensitive(bool sensitive) = 0;
    virtual bool get_sensitive() const = 0;

    /* visibility */

    virtual void show() = 0;
    virtual void hide() = 0;

    // This function simply calls show() or hide() but is convenient when the
    // visibility depends on some condition
    void set_visible(bool visible)
    {
        if (visible)
            show();
        else
            hide();
    }

    // return if this widget's visibility is true
    virtual bool get_visible() const = 0;

    // return if this widget's visibility and that of all its parents is true
    virtual bool is_visible() const = 0;

    /* focus */

    // sets if this widget can own the keyboard focus
    virtual void set_can_focus(bool bCanFocus) = 0;

    // causes this widget to have the keyboard focus
    virtual void grab_focus() = 0;

    // returns if this widget has the keyboard focus
    virtual bool has_focus() const = 0;

    // if the widget that has focus is a child, which includes toplevel popup
    // children, of this widget. So an Entry with an active popup (or dialog)
    // has has_child_focus of true, but has_focus of false, while its popup is
    // shown
    virtual bool has_child_focus() const = 0;

    /* size */
    virtual void set_size_request(int nWidth, int nHeight) = 0;
    virtual Size get_size_request() const = 0;
    virtual Size get_preferred_size() const = 0;

    /* measure */
    virtual float get_approximate_digit_width() const = 0;
    virtual int get_text_height() const = 0;
    virtual Size get_pixel_size(const OUString& rText) const = 0;

    // The name of the widget in the GtkBuilder UI definition used to construct it.
    virtual OUString get_buildable_name() const = 0;
    /*
       Typically there is no need to change the buildable name at runtime, changing
       the id in .ui file itself is preferred.

       But for ui-testing purposes it can sometimes be useful to rename
       different widgets, that were loaded from the same .ui, to unique names
       in order to distinguish between them
    */
    virtual void set_buildable_name(const OUString& rName) = 0;

    /*
      The help id of the widget used to identify help for this widget.

      By default the help id of a widget is a path-like sequence of (load-time)
      buildable-names from the widgets UI definition ancestor to this widget,
      e.g. grandparent/parent/widget.

      The default can be overwritten with set_help_id
    */
    virtual OUString get_help_id() const = 0;
    virtual void set_help_id(const OUString& rName) = 0;

    virtual void set_hexpand(bool bExpand) = 0;
    virtual void set_vexpand(bool bExpand) = 0;

    virtual void set_margin_top(int nMargin) = 0;
    virtual void set_margin_bottom(int nMargin) = 0;
    virtual void set_margin_start(int nMargin) = 0;
    virtual void set_margin_end(int nMargin) = 0;

    virtual int get_margin_start() const = 0;
    virtual int get_margin_end() const = 0;

    /*
     * Report the extents of this widget relative to the rRelative target widget.
     *
     * To succeed, both widgets must be realized, and must share a common toplevel.
     *
     * returns false if the relative extents could not be determined, e.g. if
     * either widget was not realized, or there was no common ancestor.
     * Otherwise true.
     */
    virtual bool get_extents_relative_to(const Widget& rRelative, int& x, int& y, int& width,
                                         int& height) const = 0;

    virtual void set_accessible_name(const OUString& rName) = 0;
    virtual void set_accessible_description(const OUString& rDescription) = 0;
    virtual OUString get_accessible_name() const = 0;

    virtual OUString get_accessible_description() const = 0;

    virtual OUString get_accessible_id() const = 0;

    // After this call this widget is only accessibility labelled by pLabel and
    // pLabel only accessibility labels this widget
    virtual void set_accessible_relation_labeled_by(weld::Widget* pLabel) = 0;

    virtual void set_tooltip_text(const OUString& rTip) = 0;
    virtual OUString get_tooltip_text() const = 0;

    virtual void set_cursor_data(void* pData) = 0;

    virtual void connect_command(const Link<const CommandEvent&, bool>& rLink)
    {
        m_aCommandHdl = rLink;
    }

    virtual void connect_focus_in(const Link<Widget&, void>& rLink)
    {
        assert(!m_aFocusInHdl.IsSet() || !rLink.IsSet());
        m_aFocusInHdl = rLink;
    }

    virtual void connect_focus_out(const Link<Widget&, void>& rLink)
    {
        assert(!m_aFocusOutHdl.IsSet() || !rLink.IsSet());
        m_aFocusOutHdl = rLink;
    }

    // rLink is called when the mnemonic for the Widget is called.
    // If rLink returns true the Widget will not automatically gain
    // focus as normally occurs
    virtual void connect_mnemonic_activate(const Link<Widget&, bool>& rLink)
    {
        assert(!m_aMnemonicActivateHdl.IsSet() || !rLink.IsSet());
        m_aMnemonicActivateHdl = rLink;
    }

    virtual void connect_size_allocate(const Link<const Size&, void>& rLink)
    {
        assert(!m_aSizeAllocateHdl.IsSet() || !rLink.IsSet());
        m_aSizeAllocateHdl = rLink;
    }

    virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink)
    {
        assert(!m_aKeyPressHdl.IsSet() || !rLink.IsSet());
        m_aKeyPressHdl = rLink;
    }

    virtual void connect_key_release(const Link<const KeyEvent&, bool>& rLink)
    {
        assert(!m_aKeyReleaseHdl.IsSet() || !rLink.IsSet());
        m_aKeyReleaseHdl = rLink;
    }

    virtual void connect_mouse_press(const Link<const MouseEvent&, bool>& rLink)
    {
        assert(!m_aMousePressHdl.IsSet() || !rLink.IsSet());
        m_aMousePressHdl = rLink;
    }

    virtual void connect_mouse_move(const Link<const MouseEvent&, bool>& rLink)
    {
        assert(!m_aMouseMotionHdl.IsSet() || !rLink.IsSet());
        m_aMouseMotionHdl = rLink;
    }

    virtual void connect_mouse_release(const Link<const MouseEvent&, bool>& rLink)
    {
        assert(!m_aMouseReleaseHdl.IsSet() || !rLink.IsSet());
        m_aMouseReleaseHdl = rLink;
    }

    virtual void connect_style_updated(const Link<Widget&, void>& rLink)
    {
        assert(!m_aStyleUpdatedHdl.IsSet() || !rLink.IsSet());
        m_aStyleUpdatedHdl = rLink;
    }

    virtual void grab_mouse() = 0;
    virtual bool has_mouse_grab() const = 0;
    virtual void release_mouse() = 0;

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual vcl::Font get_font() = 0;

    //true for rtl, false otherwise
    virtual bool get_direction() const = 0;
    virtual void set_direction(bool bRTL) = 0;

    /* Increases the freeze count on widget.

       If the freeze count is non-zero, emission of the widget's notifications
       is stopped. The notifications are queued until the freeze count is
       decreased to zero. Duplicate notifications may be squashed together.
    */
    virtual void freeze() = 0;

    /* Reverts the effect of a previous call to freeze.

       The freeze count is decreased on the widget and when it reaches zero,
       queued notifications are emitted.
    */
    virtual void thaw() = 0;

    /* push/pop busy mouse cursor state

      bBusy of true to push a busy state onto the stack and false
      to pop it off, calls to this should balance.

      see weld::WaitObject */
    virtual void set_busy_cursor(bool bBusy) = 0;

    virtual void queue_resize() = 0;

    virtual std::unique_ptr<Container> weld_parent() const = 0;

    virtual OUString strip_mnemonic(const OUString& rLabel) const = 0;

    /* Escapes string contents which are interpreted by the UI.

       Should be overwritten by each VCL implementation to account for the
       string contents which are interpreted by its ui.
    */
    virtual OUString escape_ui_str(const OUString& rLabel) const = 0;

    virtual ScopedVclPtr<VirtualDevice> create_virtual_device() const = 0;

    //do something transient to attract the attention of the user to the widget
    virtual void call_attention_to() = 0;

    //make this widget look like a page in a notebook
    virtual void set_stack_background() = 0;
    //make this widget look like it has a highlighted background
    virtual void set_highlight_background() = 0;
    //make this widget suitable as parent for a title
    virtual void set_title_background() = 0;
    //make this widget suitable for use in a toolbar
    virtual void set_toolbar_background() = 0;
    //trying to use a custom color for a background is generally a bad idea. If your need
    //fits one of the above categories then that's a somewhat better choice
    virtual void set_background(const Color& rBackColor) = 0;
    // reset to default background
    virtual void set_background() = 0;

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() = 0;
    virtual css::uno::Reference<css::datatransfer::clipboard::XClipboard> get_clipboard() const = 0;

    // render the widget to an output device
    virtual void draw(OutputDevice& rOutput, const Point& rPos, const Size& rSizePixel) = 0;

    virtual ~Widget() {}
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
