/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/range/b2irange.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <tools/date.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <utility>
#include <vcl/vclenum.hxx>
#include <vcl/font.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/windowstate.hxx>

#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>

#include <assert.h>
#include <memory>
#include <vector>

namespace com::sun::star::awt
{
class XWindow;
}
namespace com::sun::star::datatransfer::clipboard
{
class XClipboard;
}
namespace com::sun::star::datatransfer::dnd
{
class XDropTarget;
}
namespace com::sun::star::graphic
{
class XGraphic;
}
namespace comphelper
{
class OAccessible;
}
typedef css::uno::Reference<css::accessibility::XAccessibleRelationSet> a11yrelationset;
enum class PointerStyle;
class CommandEvent;
class Formatter;
class InputContext;
class KeyEvent;
class MouseEvent;
class TransferDataContainer;
class OutputDevice;
class VirtualDevice;
struct SystemEnvData;
class Bitmap;

namespace vcl
{
class ILibreOfficeKitNotifier;
typedef OutputDevice RenderContext;
}
namespace tools
{
class JsonWriter;
}

class LOKTrigger;

namespace weld
{
class Container;
class DialogController;
class EntryTreeView;
class IconView;
class MetricSpinButton;
class TreeView;

class VCL_DLLPUBLIC Widget
{
    friend class ::LOKTrigger;

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

    bool signal_command(const CommandEvent& rCEvt) { return m_aCommandHdl.Call(rCEvt); }
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
    virtual bool get_hexpand() const = 0;
    virtual void set_vexpand(bool bExpand) = 0;
    virtual bool get_vexpand() const = 0;

    virtual void set_margin_top(int nMargin) = 0;
    virtual void set_margin_bottom(int nMargin) = 0;
    virtual void set_margin_start(int nMargin) = 0;
    virtual void set_margin_end(int nMargin) = 0;

    virtual int get_margin_top() const = 0;
    virtual int get_margin_bottom() const = 0;
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

    //iterate upwards through the hierarchy starting at this widgets parent,
    //calling func with their helpid until func returns true or we run out of
    //parents
    virtual void help_hierarchy_foreach(const std::function<bool(const OUString&)>& func) = 0;

    virtual OUString strip_mnemonic(const OUString& rLabel) const = 0;

    /* Escapes string contents which are interpreted by the UI.

       Should be overwritten by each VCL implementation to account for the
       string contents which are interpreted by its ui.
    */
    virtual OUString escape_ui_str(const OUString& rLabel) const = 0;

    virtual VclPtr<VirtualDevice> create_virtual_device() const = 0;

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

    virtual void connect_get_property_tree(const Link<tools::JsonWriter&, void>& rLink) = 0;
    virtual void get_property_tree(tools::JsonWriter& rJsonWriter) = 0;

    // render the widget to an output device
    virtual void draw(OutputDevice& rOutput, const Point& rPos, const Size& rSizePixel) = 0;

    virtual ~Widget() {}
};

class VCL_DLLPUBLIC Container : virtual public Widget
{
    Link<Container&, void> m_aContainerFocusChangedHdl;

protected:
    void signal_container_focus_changed() { m_aContainerFocusChangedHdl.Call(*this); }

public:
    // remove from old container and add to new container in one go
    // new container can be null to just remove from old container
    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) = 0;
    // create an XWindow as a child of this container. The XWindow is
    // suitable to contain css::awt::XControl items
    virtual css::uno::Reference<css::awt::XWindow> CreateChildFrame() = 0;
    // rLink is called when the focus transitions from a widget outside the container
    // to a widget inside the container or vice versa
    virtual void connect_container_focus_changed(const Link<Container&, void>& rLink)
    {
        m_aContainerFocusChangedHdl = rLink;
    }
    // causes a child of the container to have the keyboard focus
    virtual void child_grab_focus() = 0;
};

class VCL_DLLPUBLIC Box : virtual public Container
{
public:
    // Moves child to a new position in the list of children
    virtual void reorder_child(weld::Widget* pWidget, int position) = 0;
    // Sort ok/cancel etc buttons in platform order
    virtual void sort_native_button_order() = 0;
};

class VCL_DLLPUBLIC Grid : virtual public Container
{
public:
    virtual void set_child_left_attach(weld::Widget& rWidget, int nAttach) = 0;
    virtual int get_child_left_attach(weld::Widget& rWidget) const = 0;
    virtual void set_child_column_span(weld::Widget& rWidget, int nCols) = 0;
    virtual void set_child_top_attach(weld::Widget& rWidget, int nAttach) = 0;
    virtual int get_child_top_attach(weld::Widget& rWidget) const = 0;
};

class VCL_DLLPUBLIC Paned : virtual public Widget
{
public:
    // set pixel position of divider
    virtual void set_position(int nPos) = 0;
    // get pixel position of divider
    virtual int get_position() const = 0;
};

class VCL_DLLPUBLIC Frame : virtual public Container
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
};

class VCL_DLLPUBLIC Notebook : virtual public Widget
{
    friend class ::LOKTrigger;

protected:
    Link<const OUString&, bool> m_aLeavePageHdl;
    Link<const OUString&, void> m_aEnterPageHdl;

public:
    virtual int get_current_page() const = 0;
    virtual int get_page_index(const OUString& rIdent) const = 0;
    virtual OUString get_page_ident(int nPage) const = 0;
    virtual OUString get_current_page_ident() const = 0;
    virtual void set_current_page(int nPage) = 0;
    virtual void set_current_page(const OUString& rIdent) = 0;
    virtual void remove_page(const OUString& rIdent) = 0;
    virtual void insert_page(const OUString& rIdent, const OUString& rLabel, int nPos,
                             const OUString* pIconName = nullptr)
        = 0;
    void append_page(const OUString& rIdent, const OUString& rLabel,
                     const OUString* pIconName = nullptr)
    {
        insert_page(rIdent, rLabel, -1, pIconName);
    }
    virtual void set_tab_label_text(const OUString& rIdent, const OUString& rLabel) = 0;
    virtual OUString get_tab_label_text(const OUString& rIdent) const = 0;
    virtual void set_show_tabs(bool bShow) = 0;
    virtual int get_n_pages() const = 0;
    virtual weld::Container* get_page(const OUString& rIdent) const = 0;

    void connect_leave_page(const Link<const OUString&, bool>& rLink) { m_aLeavePageHdl = rLink; }
    void connect_enter_page(const Link<const OUString&, void>& rLink) { m_aEnterPageHdl = rLink; }
};

class VCL_DLLPUBLIC ScreenShotEntry
{
public:
    ScreenShotEntry(OUString aHelpId, const basegfx::B2IRange& rB2IRange)
        : msHelpId(std::move(aHelpId))
        , maB2IRange(rB2IRange)
    {
    }

    const basegfx::B2IRange& getB2IRange() const { return maB2IRange; }

    const OUString& GetHelpId() const { return msHelpId; }

private:
    OUString msHelpId;
    basegfx::B2IRange maB2IRange;
};

typedef std::vector<ScreenShotEntry> ScreenShotCollection;

class VCL_DLLPUBLIC Window : virtual public Container
{
protected:
    Link<Widget&, bool> m_aHelpRequestHdl;

public:
    virtual void set_title(const OUString& rTitle) = 0;
    virtual OUString get_title() const = 0;
    virtual void window_move(int x, int y) = 0;
    virtual bool get_resizable() const = 0;
    virtual Size get_size() const = 0;
    virtual Point get_position() const = 0;
    virtual AbsoluteScreenPixelRectangle get_monitor_workarea() const = 0;

    // returns whether the widget that has focus is within this Window
    // (its very possible to move this to weld::Container if that becomes
    // desirable)
    virtual bool has_toplevel_focus() const = 0;
    virtual void present() = 0;

    virtual void set_window_state(const OUString& rStr) = 0;
    virtual OUString get_window_state(vcl::WindowDataMask nMask) const = 0;

    virtual css::uno::Reference<css::awt::XWindow> GetXWindow() = 0;

    void connect_help(const Link<Widget&, bool>& rLink) { m_aHelpRequestHdl = rLink; }

    virtual SystemEnvData get_system_data() const = 0;

    virtual void resize_to_request() = 0;

    // collect positions of widgets and their help ids for screenshot purposes
    virtual ScreenShotCollection collect_screenshot_data() = 0;

    // render the widget to an output device
    virtual VclPtr<VirtualDevice> screenshot() = 0;

    virtual const vcl::ILibreOfficeKitNotifier* GetLOKNotifier() = 0;
};

class VCL_DLLPUBLIC WaitObject
{
private:
    weld::Widget* m_pWindow;

public:
    WaitObject(weld::Widget* pWindow)
        : m_pWindow(pWindow)
    {
        if (m_pWindow)
            m_pWindow->set_busy_cursor(true);
    }
    ~WaitObject()
    {
        if (m_pWindow)
            m_pWindow->set_busy_cursor(false);
    }
};

class Button;

class VCL_DLLPUBLIC Dialog : virtual public Window
{
private:
    friend DialogController;
    virtual bool runAsync(std::shared_ptr<DialogController> const& rxOwner,
                          const std::function<void(sal_Int32)>& func)
        = 0;

public:
    virtual void set_modal(bool bModal) = 0;
    virtual bool get_modal() const = 0;

    // center dialog on its parent
    //
    // bTrackGeometryRequests set to true tries to ensure the dialog will end
    // up still centered on its parent windows final size, taking into account
    // that there may currently be pending geometry requests for the parent not
    // yet processed by the underlying toolkit
    //
    // for e.g gtk this will means the dialog is always centered even when
    // resized, calling set_centered_on_parent with false will turn this
    // off again.
    virtual void set_centered_on_parent(bool bTrackGeometryRequests) = 0;

    virtual int run() = 0;
    // Run async without a controller
    // @param self - must point to this, to enforce that the dialog was created/held by a shared_ptr
    virtual bool runAsync(std::shared_ptr<Dialog> const& rxSelf,
                          const std::function<void(sal_Int32)>& func)
        = 0;
    virtual void response(int response) = 0;
    virtual void add_button(const OUString& rText, int response, const OUString& rHelpId = {}) = 0;
    virtual std::unique_ptr<Button> weld_button_for_response(int response) = 0;
    virtual std::unique_ptr<weld::Container> weld_content_area() = 0;

    // with pOld of null, automatically find the old default widget and unset
    // it, otherwise use as hint to the old default
    virtual void change_default_button(weld::Button* pOld, weld::Button* pNew) = 0;
    virtual bool is_default_button(const weld::Button* pCandidate) const = 0;

    virtual inline void set_default_response(int nResponse);

    // shrink the dialog down to shown just these widgets
    virtual void collapse(weld::Widget& rEdit, weld::Widget* pButton) = 0;
    // undo previous dialog collapse
    virtual void undo_collapse() = 0;

    virtual void SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>& rLink)
        = 0;
};

class VCL_DLLPUBLIC MessageDialog : virtual public Dialog
{
public:
    virtual void set_primary_text(const OUString& rText) = 0;
    virtual OUString get_primary_text() const = 0;
    virtual void set_secondary_text(const OUString& rText) = 0;
    virtual OUString get_secondary_text() const = 0;
    virtual std::unique_ptr<Container> weld_message_area() = 0;
};

inline OUString toId(const void* pValue)
{
    return OUString::number(reinterpret_cast<sal_uIntPtr>(pValue));
}

template <typename T> T fromId(const OUString& rValue)
{
    return reinterpret_cast<T>(rValue.toUInt64());
}

enum class EntryMessageType
{
    Normal,
    Warning,
    Error,
};

class VCL_DLLPUBLIC TreeIter
{
private:
    TreeIter(const TreeIter&) = delete;
    TreeIter& operator=(const TreeIter&) = delete;

public:
    TreeIter() {}
    virtual bool equal(const TreeIter& rOther) const = 0;
    virtual ~TreeIter() {}
};

class VCL_DLLPUBLIC Button : virtual public Widget
{
    friend class ::LOKTrigger;

protected:
    Link<Button&, void> m_aClickHdl;

    void signal_clicked() { m_aClickHdl.Call(*this); }

public:
    virtual void set_label(const OUString& rText) = 0;
    // pDevice, the image for the button, or nullptr to unset
    virtual void set_image(VirtualDevice* pDevice) = 0;
    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) = 0;
    virtual void set_from_icon_name(const OUString& rIconName) = 0;
    virtual OUString get_label() const = 0;
    void clicked() { signal_clicked(); }
    bool is_custom_handler_set() { return m_aClickHdl.IsSet(); }

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;

    /* Sometimes, a widget should behave like a button (activate on click,
       accept keyboard focus, etc), but look entirely different.

       pDevice, the custom look to use, or nullptr to unset.

       Typically doing this is ill advised. Consider using
       set_accessible_name if you do. */
    virtual void set_custom_button(VirtualDevice* pDevice) = 0;

    virtual void connect_clicked(const Link<Button&, void>& rLink) { m_aClickHdl = rLink; }
};

class VCL_DLLPUBLIC Toggleable : virtual public Widget
{
    friend class ::LOKTrigger;

protected:
    Link<Toggleable&, void> m_aToggleHdl;
    TriState m_eSavedValue = TRISTATE_FALSE;

    void signal_toggled()
    {
        if (notify_events_disabled())
            return;
        m_aToggleHdl.Call(*this);
    }

    virtual void do_set_active(bool active) = 0;

public:
    void set_active(bool active)
    {
        disable_notify_events();
        do_set_active(active);
        enable_notify_events();
    }

    virtual bool get_active() const = 0;

    virtual TriState get_state() const
    {
        if (get_active())
            return TRISTATE_TRUE;
        return TRISTATE_FALSE;
    }

    void save_state() { m_eSavedValue = get_state(); }
    TriState get_saved_state() const { return m_eSavedValue; }
    bool get_state_changed_from_saved() const { return m_eSavedValue != get_state(); }

    virtual void connect_toggled(const Link<Toggleable&, void>& rLink) { m_aToggleHdl = rLink; }
};

class VCL_DLLPUBLIC ToggleButton : virtual public Button, virtual public Toggleable
{
    friend class ::LOKTrigger;
};

class VCL_DLLPUBLIC MenuButton : virtual public ToggleButton
{
    friend class ::LOKTrigger;

    Link<const OUString&, void> m_aSelectHdl;

protected:
    void signal_selected(const OUString& rIdent) { m_aSelectHdl.Call(rIdent); }

public:
    void connect_selected(const Link<const OUString&, void>& rLink) { m_aSelectHdl = rLink; }

    virtual void insert_item(int pos, const OUString& rId, const OUString& rStr,
                             const OUString* pIconName, VirtualDevice* pImageSurface,
                             TriState eCheckRadioFalse)
        = 0;
    void append_item(const OUString& rId, const OUString& rStr)
    {
        insert_item(-1, rId, rStr, nullptr, nullptr, TRISTATE_INDET);
    }
    void append_item_check(const OUString& rId, const OUString& rStr)
    {
        insert_item(-1, rId, rStr, nullptr, nullptr, TRISTATE_TRUE);
    }
    void append_item_radio(const OUString& rId, const OUString& rStr)
    {
        insert_item(-1, rId, rStr, nullptr, nullptr, TRISTATE_FALSE);
    }
    void append_item(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert_item(-1, rId, rStr, &rImage, nullptr, TRISTATE_INDET);
    }
    void append_item(const OUString& rId, const OUString& rStr, VirtualDevice& rImage)
    {
        insert_item(-1, rId, rStr, nullptr, &rImage, TRISTATE_INDET);
    }
    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }
    virtual void remove_item(const OUString& rId) = 0;
    virtual void clear() = 0;
    virtual void set_item_sensitive(const OUString& rIdent, bool bSensitive) = 0;
    virtual void set_item_active(const OUString& rIdent, bool bActive) = 0;
    virtual void set_item_label(const OUString& rIdent, const OUString& rLabel) = 0;
    virtual OUString get_item_label(const OUString& rIdent) const = 0;
    virtual void set_item_visible(const OUString& rIdent, bool bVisible) = 0;

    virtual void set_popover(weld::Widget* pPopover) = 0;
};

class VCL_DLLPUBLIC CheckButton : virtual public Toggleable
{
protected:
    virtual void do_set_state(TriState eState) = 0;

public:
    // must override Toggleable::get_state to support TRISTATE_INDET
    virtual TriState get_state() const override = 0;

    void set_state(TriState eState)
    {
        disable_notify_events();
        do_set_state(eState);
        enable_notify_events();
    }

    virtual void do_set_active(bool bActive) override final
    {
        do_set_state(bActive ? TRISTATE_TRUE : TRISTATE_FALSE);
    }

    virtual bool get_active() const override final { return get_state() == TRISTATE_TRUE; }

    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_label_wrap(bool wrap) = 0;
};

struct VCL_DLLPUBLIC TriStateEnabled
{
    TriState eState;
    bool bTriStateEnabled;
    TriStateEnabled()
        : eState(TRISTATE_INDET)
        , bTriStateEnabled(true)
    {
    }
    void CheckButtonToggled(CheckButton& rToggle);
};

class VCL_DLLPUBLIC RadioButton : virtual public Toggleable
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_label_wrap(bool wrap) = 0;
};

class VCL_DLLPUBLIC LinkButton : virtual public Widget
{
    friend class ::LOKTrigger;

    Link<LinkButton&, bool> m_aActivateLinkHdl;

protected:
    bool signal_activate_link() { return m_aActivateLinkHdl.Call(*this); }

public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_label_wrap(bool wrap) = 0;
    virtual void set_uri(const OUString& rUri) = 0;
    virtual OUString get_uri() const = 0;

    void connect_activate_link(const Link<LinkButton&, bool>& rLink) { m_aActivateLinkHdl = rLink; }
};

class VCL_DLLPUBLIC Scale : virtual public Widget
{
    Link<Scale&, void> m_aValueChangedHdl;

protected:
    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

public:
    virtual void set_value(int value) = 0;
    virtual int get_value() const = 0;
    virtual void set_range(int min, int max) = 0;

    virtual void set_increments(int step, int page) = 0;
    virtual void get_increments(int& step, int& page) const = 0;

    void connect_value_changed(const Link<Scale&, void>& rLink) { m_aValueChangedHdl = rLink; }
};

class VCL_DLLPUBLIC Spinner : virtual public Widget
{
public:
    virtual void start() = 0;
    virtual void stop() = 0;
};

class VCL_DLLPUBLIC ProgressBar : virtual public Widget
{
public:
    //0-100
    virtual void set_percentage(int value) = 0;
    virtual OUString get_text() const = 0;
    virtual void set_text(const OUString& rText) = 0;
};

class VCL_DLLPUBLIC LevelBar : virtual public Widget
{
public:
    /// Sets LevelBar fill percentage.
    /// @param fPercentage bar's fill percentage, [0.0, 100.0]
    virtual void set_percentage(double fPercentage) = 0;
};

class VCL_DLLPUBLIC Entry : virtual public Widget
{
private:
    OUString m_sSavedValue;

protected:
    Link<Entry&, void> m_aChangeHdl;
    Link<OUString&, bool> m_aInsertTextHdl;
    Link<Entry&, void> m_aCursorPositionHdl;
    Link<Entry&, bool> m_aActivateHdl;

    friend class ::LOKTrigger;

    void signal_changed()
    {
        if (notify_events_disabled())
            return;
        m_aChangeHdl.Call(*this);
    }

    void signal_activated()
    {
        if (notify_events_disabled())
            return;
        m_aActivateHdl.Call(*this);
    }

    void signal_cursor_position()
    {
        if (notify_events_disabled())
            return;
        m_aCursorPositionHdl.Call(*this);
    }

    virtual void do_set_text(const OUString& rText) = 0;
    virtual void do_select_region(int nStartPos, int nEndPos) = 0;
    virtual void do_set_position(int nCursorPos) = 0;

public:
    void set_text(const OUString& rText)
    {
        disable_notify_events();
        do_set_text(rText);
        enable_notify_events();
    }

    virtual OUString get_text() const = 0;
    virtual void set_width_chars(int nChars) = 0;
    virtual int get_width_chars() const = 0;
    // The maximum length of the entry. Use 0 for no maximum
    virtual void set_max_length(int nChars) = 0;

    // nEndPos can be -1 in order to select all text
    void select_region(int nStartPos, int nEndPos)
    {
        disable_notify_events();
        do_select_region(nStartPos, nEndPos);
        enable_notify_events();
    }

    // returns true if the selection has nonzero length
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) = 0;
    virtual void replace_selection(const OUString& rText) = 0;

    // nCursorPos can be -1 to set to the end
    void set_position(int nCursorPos)
    {
        disable_notify_events();
        do_set_position(nCursorPos);
        enable_notify_events();
    }

    virtual int get_position() const = 0;
    virtual void set_editable(bool bEditable) = 0;
    virtual bool get_editable() const = 0;
    virtual void set_visibility(bool bVisible) = 0;
    virtual void set_message_type(EntryMessageType eType) = 0;
    virtual void set_placeholder_text(const OUString& rText) = 0;

    virtual void set_overwrite_mode(bool bOn) = 0;
    virtual bool get_overwrite_mode() const = 0;

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;

    /*
       If you want to set a warning or error state, see set_message_type
       instead where, if the toolkit supports it, a specific warning/error
       indicator is shown.

       This explicit text color method exists to support rendering the
       SvNumberformat color feature.
    */
    virtual void set_font_color(const Color& rColor) = 0;

    virtual void connect_changed(const Link<Entry&, void>& rLink) { m_aChangeHdl = rLink; }
    void connect_insert_text(const Link<OUString&, bool>& rLink) { m_aInsertTextHdl = rLink; }
    // callback returns true to indicated no further processing of activate wanted
    void connect_activate(const Link<Entry&, bool>& rLink) { m_aActivateHdl = rLink; }
    virtual void connect_cursor_position(const Link<Entry&, void>& rLink)
    {
        m_aCursorPositionHdl = rLink;
    }

    virtual void cut_clipboard() = 0;
    virtual void copy_clipboard() = 0;
    virtual void paste_clipboard() = 0;

    virtual void set_alignment(TxtAlign eXAlign) = 0;

    void save_value() { m_sSavedValue = get_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_text(); }
};

class VCL_DLLPUBLIC SpinButton : virtual public Entry
{
    friend class ::LOKTrigger;

    Link<SpinButton&, void> m_aValueChangedHdl;
    Link<sal_Int64, OUString> m_aFormatValueHdl;
    Link<const OUString&, std::optional<int>> m_aParseTextHdl;

    // helper methods to convert between sal_Int64 value and
    // floating point number it represents (depending on get_digits())
    double convert_value_to_double(sal_Int64 nValue) const
    {
        return static_cast<double>(nValue) / Power10(get_digits());
    }
    sal_Int64 convert_double_to_value(double fDouble) const
    {
        return basegfx::fround64(fDouble * Power10(get_digits()));
    }

protected:
    void signal_value_changed()
    {
        if (notify_events_disabled())
            return;
        m_aValueChangedHdl.Call(*this);
    }

    /** If a custom value formatter was set via <a>set_value_formatter</a>,
     *  that one gets called to create a text representation of the value
     *  and that one gets returned.
     *  Otherwise, an empty std::optional is returned.
     */
    std::optional<OUString> format_floating_point_value(double fValue)
    {
        if (!m_aFormatValueHdl.IsSet())
            return {};
        const OUString sText = m_aFormatValueHdl.Call(convert_double_to_value(fValue));
        return sText;
    }

    /** If a custom text parser (which parses a value from the given text)
     *  is set and the text can be parsed, this method sets that value
     *  in <a>result</a> and returns <a>TRISTATE_TRUE</a>.
     *  Returns <a>TRISTATE_FALSE</a> if a custom handler is set, but the text
     *  cannot be parsed.
     *  Returns <a>TRISTATE_INDET</a> if no custom input handler is set.
     */
    TriState parse_text(const OUString& rText, double* pResult)
    {
        if (!m_aParseTextHdl.IsSet())
            return TRISTATE_INDET;
        std::optional<int> aValue = m_aParseTextHdl.Call(rText);
        if (!aValue.has_value())
            return TRISTATE_FALSE;

        *pResult = convert_value_to_double(aValue.value());
        return TRISTATE_TRUE;
    }

    // methods to implement in subclasses which use floating point values directly;
    // public methods using sal_Int64 values whose floating point value depends on get_digits()
    // take care of conversion
    virtual void set_floating_point_value(double fValue) = 0;
    virtual double get_floating_point_value() const = 0;
    virtual void set_floating_point_range(double fMin, double fMax) = 0;
    virtual void get_floating_point_range(double& rMin, double& rMax) const = 0;
    virtual void set_floating_point_increments(double fStep, double fPage) = 0;
    virtual void get_floating_point_increments(double& rStep, double& rPage) const = 0;

public:
    void set_value(sal_Int64 value)
    {
        disable_notify_events();
        set_floating_point_value(convert_value_to_double(value));
        enable_notify_events();
    }

    sal_Int64 get_value() const { return convert_double_to_value(get_floating_point_value()); }

    void set_range(sal_Int64 min, sal_Int64 max)
    {
        set_floating_point_range(convert_value_to_double(min), convert_value_to_double(max));
    }

    void get_range(sal_Int64& min, sal_Int64& max) const
    {
        double fMin = 0;
        double fMax = 0;
        get_floating_point_range(fMin, fMax);
        min = convert_double_to_value(fMin);
        max = convert_double_to_value(fMax);
    }

    void set_min(sal_Int64 min)
    {
        sal_Int64 dummy, max;
        get_range(dummy, max);
        set_range(min, max);
    }
    void set_max(sal_Int64 max)
    {
        sal_Int64 min, dummy;
        get_range(min, dummy);
        set_range(min, max);
    }
    sal_Int64 get_min() const
    {
        sal_Int64 min, dummy;
        get_range(min, dummy);
        return min;
    }
    sal_Int64 get_max() const
    {
        sal_Int64 dummy, max;
        get_range(dummy, max);
        return max;
    }

    void set_increments(sal_Int64 step, sal_Int64 page)
    {
        set_floating_point_increments(convert_value_to_double(step), convert_value_to_double(page));
    }

    void get_increments(sal_Int64& step, sal_Int64& page) const
    {
        double fStep = 0;
        double fPage = 0;
        get_floating_point_increments(fStep, fPage);
        step = convert_double_to_value(fStep);
        page = convert_double_to_value(fPage);
    }

    virtual void set_digits(unsigned int digits) = 0;
    virtual unsigned int get_digits() const = 0;

    void connect_value_changed(const Link<SpinButton&, void>& rLink) { m_aValueChangedHdl = rLink; }

    /** Set a value formatter that receives the value as a parameter and returns the
     *  text representation to display in the SpinButton.
     */
    void set_value_formatter(const Link<sal_Int64, OUString>& rLink) { m_aFormatValueHdl = rLink; }

    /** Set a parser that receives the text as a parameter and returns the value
     *  parsed from the text, or an empty std::optional if a value cannot be
     *  parsed from the text.
     */
    void set_text_parser(const Link<const OUString&, std::optional<int>>& rLink)
    {
        m_aParseTextHdl = rLink;
    }

    sal_Int64 normalize(sal_Int64 nValue) const { return (nValue * Power10(get_digits())); }

    sal_Int64 denormalize(sal_Int64 nValue) const;

    static unsigned int Power10(unsigned int n);
};

class EntryFormatter;

// Similar to a SpinButton, but input and output formatting and range/value
// are managed by a more complex Formatter which can support doubles.
class VCL_DLLPUBLIC FormattedSpinButton : virtual public Entry
{
    friend class ::LOKTrigger;

    Link<FormattedSpinButton&, void> m_aValueChangedHdl;

protected:
    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

public:
    virtual Formatter& GetFormatter() = 0;
    // does not take ownership, and so must be deregistered if pFormatter
    // is destroyed
    virtual void SetFormatter(weld::EntryFormatter* pFormatter) = 0;

    void connect_value_changed(const Link<FormattedSpinButton&, void>& rLink)
    {
        m_aValueChangedHdl = rLink;
    }

private:
    friend class EntryFormatter;
    virtual void sync_range_from_formatter() = 0;
    virtual void sync_value_from_formatter() = 0;
    virtual void sync_increments_from_formatter() = 0;
};

class VCL_DLLPUBLIC Image : virtual public Widget
{
public:
    virtual void set_from_icon_name(const OUString& rIconName) = 0;
    virtual void set_image(VirtualDevice* pDevice) = 0;
    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) = 0;
};

class VCL_DLLPUBLIC Calendar : virtual public Widget
{
    friend class ::LOKTrigger;

    Link<Calendar&, void> m_aSelectedHdl;
    Link<Calendar&, void> m_aActivatedHdl;

protected:
    void signal_selected()
    {
        if (notify_events_disabled())
            return;
        m_aSelectedHdl.Call(*this);
    }

    void signal_activated()
    {
        if (notify_events_disabled())
            return;
        m_aActivatedHdl.Call(*this);
    }

public:
    void connect_selected(const Link<Calendar&, void>& rLink) { m_aSelectedHdl = rLink; }
    void connect_activated(const Link<Calendar&, void>& rLink) { m_aActivatedHdl = rLink; }

    virtual void set_date(const Date& rDate) = 0;
    virtual Date get_date() const = 0;
};

enum class LabelType
{
    Normal,
    Warning,
    Error,
    Title, // this is intended to be used against the background set by set_title_background
};

class VCL_DLLPUBLIC Label : virtual public Widget
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_mnemonic_widget(Widget* pTarget) = 0;
    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;
    virtual void set_label_type(LabelType eType) = 0;
    /*
       If you want to set a warning or error state, see set_label_type
       instead.
    */
    virtual void set_font_color(const Color& rColor) = 0;
};

class VCL_DLLPUBLIC Expander : virtual public Widget
{
    Link<Expander&, void> m_aExpandedHdl;

protected:
    void signal_expanded() { m_aExpandedHdl.Call(*this); }

public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual bool get_expanded() const = 0;
    virtual void set_expanded(bool bExpand) = 0;

    void connect_expanded(const Link<Expander&, void>& rLink) { m_aExpandedHdl = rLink; }
};

class VCL_DLLPUBLIC DrawingArea : virtual public Widget
{
public:
    typedef std::pair<vcl::RenderContext&, const tools::Rectangle&> draw_args;

protected:
    Link<draw_args, void> m_aDrawHdl;
    Link<Widget&, tools::Rectangle> m_aGetFocusRectHdl;
    Link<tools::Rectangle&, OUString> m_aQueryTooltipHdl;
    // if handler returns true, drag is disallowed
    Link<DrawingArea&, bool> m_aDragBeginHdl;
    // return position of cursor, fill OUString& with surrounding text
    Link<OUString&, int> m_aGetSurroundingHdl;
    // attempt to delete the range, return true if successful
    Link<const Selection&, bool> m_aDeleteSurroundingHdl;

    OUString signal_query_tooltip(tools::Rectangle& rHelpArea)
    {
        return m_aQueryTooltipHdl.Call(rHelpArea);
    }

    int signal_im_context_get_surrounding(OUString& rSurroundingText)
    {
        if (!m_aGetSurroundingHdl.IsSet())
            return -1;
        return m_aGetSurroundingHdl.Call(rSurroundingText);
    }

    bool signal_im_context_delete_surrounding(const Selection& rRange)
    {
        return m_aDeleteSurroundingHdl.Call(rRange);
    }

public:
    void connect_draw(const Link<draw_args, void>& rLink) { m_aDrawHdl = rLink; }
    void connect_focus_rect(const Link<Widget&, tools::Rectangle>& rLink)
    {
        m_aGetFocusRectHdl = rLink;
    }
    void connect_query_tooltip(const Link<tools::Rectangle&, OUString>& rLink)
    {
        m_aQueryTooltipHdl = rLink;
    }
    void connect_drag_begin(const Link<DrawingArea&, bool>& rLink) { m_aDragBeginHdl = rLink; }
    void connect_im_context_get_surrounding(const Link<OUString&, int>& rLink)
    {
        m_aGetSurroundingHdl = rLink;
    }
    void connect_im_context_delete_surrounding(const Link<const Selection&, bool>& rLink)
    {
        m_aDeleteSurroundingHdl = rLink;
    }
    virtual void queue_draw() = 0;
    virtual void queue_draw_area(int x, int y, int width, int height) = 0;

    virtual void enable_drag_source(rtl::Reference<TransferDataContainer>& rTransferable,
                                    sal_uInt8 eDNDConstants)
        = 0;

    virtual void set_cursor(PointerStyle ePointerStyle) = 0;

    virtual Point get_pointer_position() const = 0;

    virtual void set_input_context(const InputContext& rInputContext) = 0;
    virtual void im_context_set_cursor_location(const tools::Rectangle& rCursorRect,
                                                int nExtTextInputWidth)
        = 0;

    // use return here just to generate matching VirtualDevices
    virtual OutputDevice& get_ref_device() = 0;

    virtual rtl::Reference<comphelper::OAccessible> get_accessible_parent() = 0;
    virtual a11yrelationset get_accessible_relation_set() = 0;
    virtual AbsoluteScreenPixelPoint get_accessible_location_on_screen() = 0;

private:
    friend class ::LOKTrigger;

    virtual void click(const Point&) = 0;

    virtual void dblclick(const Point&){};

    virtual void mouse_up(const Point&){};

    virtual void mouse_down(const Point&){};

    virtual void mouse_move(const Point&){};
};

enum class Placement
{
    Under,
    End
};

class VCL_DLLPUBLIC Menu
{
    friend class ::LOKTrigger;

    Link<const OUString&, void> m_aActivateHdl;

protected:
    void signal_activate(const OUString& rIdent) { m_aActivateHdl.Call(rIdent); }

public:
    virtual OUString popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                                   Placement ePlace = Placement::Under)
        = 0;

    void connect_activate(const Link<const OUString&, void>& rLink) { m_aActivateHdl = rLink; }

    virtual void set_sensitive(const OUString& rIdent, bool bSensitive) = 0;
    virtual bool get_sensitive(const OUString& rIdent) const = 0;
    virtual void set_label(const OUString& rIdent, const OUString& rLabel) = 0;
    virtual OUString get_label(const OUString& rIdent) const = 0;
    virtual void set_active(const OUString& rIdent, bool bActive) = 0;
    virtual bool get_active(const OUString& rIdent) const = 0;
    virtual void set_visible(const OUString& rIdent, bool bVisible) = 0;

    virtual void insert(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        const css::uno::Reference<css::graphic::XGraphic>& rImage,
                        TriState eCheckRadioFalse)
        = 0;

    virtual void set_item_help_id(const OUString& rIdent, const OUString& rHelpId) = 0;
    virtual void remove(const OUString& rId) = 0;

    virtual void clear() = 0;

    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }

    void append(const OUString& rId, const OUString& rStr)
    {
        insert(-1, rId, rStr, nullptr, nullptr, nullptr, TRISTATE_INDET);
    }
    void append_check(const OUString& rId, const OUString& rStr)
    {
        insert(-1, rId, rStr, nullptr, nullptr, nullptr, TRISTATE_TRUE);
    }
    void append_radio(const OUString& rId, const OUString& rStr)
    {
        insert(-1, rId, rStr, nullptr, nullptr, nullptr, TRISTATE_FALSE);
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(-1, rId, rStr, &rImage, nullptr, nullptr, TRISTATE_INDET);
    }
    void append(const OUString& rId, const OUString& rStr, VirtualDevice& rImage)
    {
        insert(-1, rId, rStr, nullptr, &rImage, nullptr, TRISTATE_INDET);
    }

    // return the number of toplevel nodes
    virtual int n_children() const = 0;

    virtual OUString get_id(int pos) const = 0;

    virtual ~Menu() {}
};

class VCL_DLLPUBLIC Popover : virtual public Container
{
    friend class ::LOKTrigger;

private:
    Link<weld::Popover&, void> m_aCloseHdl;

protected:
    void signal_closed() { m_aCloseHdl.Call(*this); }

public:
    virtual void popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect,
                               Placement ePlace = Placement::Under)
        = 0;
    virtual void popdown() = 0;

    virtual void resize_to_request() = 0;

    void connect_closed(const Link<weld::Popover&, void>& rLink) { m_aCloseHdl = rLink; }
};

class VCL_DLLPUBLIC Toolbar : virtual public Widget
{
    Link<const OUString&, void> m_aClickHdl;
    Link<const OUString&, void> m_aToggleMenuHdl;

protected:
    friend class ::LOKTrigger;

    void signal_clicked(const OUString& rIdent) { m_aClickHdl.Call(rIdent); }
    void signal_toggle_menu(const OUString& rIdent) { m_aToggleMenuHdl.Call(rIdent); }

public:
    virtual void set_item_sensitive(const OUString& rIdent, bool bSensitive) = 0;
    virtual bool get_item_sensitive(const OUString& rIdent) const = 0;
    virtual void set_item_active(const OUString& rIdent, bool bActive) = 0;
    virtual bool get_item_active(const OUString& rIdent) const = 0;
    virtual void set_menu_item_active(const OUString& rIdent, bool bActive) = 0;
    virtual bool get_menu_item_active(const OUString& rIdent) const = 0;
    virtual void set_item_menu(const OUString& rIdent, weld::Menu* pMenu) = 0;
    virtual void set_item_popover(const OUString& rIdent, weld::Widget* pPopover) = 0;
    virtual void set_item_visible(const OUString& rIdent, bool bVisible) = 0;
    virtual void set_item_help_id(const OUString& rIdent, const OUString& rHelpId) = 0;
    virtual bool get_item_visible(const OUString& rIdent) const = 0;
    virtual void set_item_label(const OUString& rIdent, const OUString& rLabel) = 0;
    virtual OUString get_item_label(const OUString& rIdent) const = 0;
    virtual void set_item_tooltip_text(const OUString& rIdent, const OUString& rTip) = 0;
    virtual OUString get_item_tooltip_text(const OUString& rIdent) const = 0;
    virtual void set_item_icon_name(const OUString& rIdent, const OUString& rIconName) = 0;
    virtual void set_item_image_mirrored(const OUString& rIdent, bool bMirrored) = 0;
    virtual void set_item_image(const OUString& rIdent,
                                const css::uno::Reference<css::graphic::XGraphic>& rIcon)
        = 0;
    virtual void set_item_image(const OUString& rIdent, VirtualDevice* pDevice) = 0;

    virtual void insert_item(int pos, const OUString& rId) = 0;
    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }

    virtual int get_n_items() const = 0;
    virtual OUString get_item_ident(int nIndex) const = 0;
    virtual void set_item_ident(int nIndex, const OUString& rIdent) = 0;
    virtual void set_item_label(int nIndex, const OUString& rLabel) = 0;
    virtual void set_item_image(int nIndex,
                                const css::uno::Reference<css::graphic::XGraphic>& rIcon)
        = 0;
    virtual void set_item_tooltip_text(int nIndex, const OUString& rTip) = 0;
    virtual void set_item_accessible_name(int nIndex, const OUString& rName) = 0;
    virtual void set_item_accessible_name(const OUString& rIdent, const OUString& rName) = 0;

    virtual vcl::ImageType get_icon_size() const = 0;
    virtual void set_icon_size(vcl::ImageType eType) = 0;

    // return what modifiers are held
    virtual sal_uInt16 get_modifier_state() const = 0;

    // This function returns the position a new item should be inserted if dnd
    // is dropped at rPoint
    virtual int get_drop_index(const Point& rPoint) const = 0;

    void connect_clicked(const Link<const OUString&, void>& rLink) { m_aClickHdl = rLink; }
    void connect_menu_toggled(const Link<const OUString&, void>& rLink)
    {
        m_aToggleMenuHdl = rLink;
    }
};

class VCL_DLLPUBLIC Scrollbar : virtual public Widget
{
    Link<Scrollbar&, void> m_aValueChangeHdl;

protected:
    void signal_adjustment_value_changed() { m_aValueChangeHdl.Call(*this); }

public:
    virtual void adjustment_configure(int value, int lower, int upper, int step_increment,
                                      int page_increment, int page_size)
        = 0;
    virtual int adjustment_get_value() const = 0;
    virtual void adjustment_set_value(int value) = 0;
    virtual int adjustment_get_upper() const = 0;
    virtual void adjustment_set_upper(int upper) = 0;
    virtual int adjustment_get_page_size() const = 0;
    virtual void adjustment_set_page_size(int size) = 0;
    virtual int adjustment_get_page_increment() const = 0;
    virtual void adjustment_set_page_increment(int size) = 0;
    virtual int adjustment_get_step_increment() const = 0;
    virtual void adjustment_set_step_increment(int size) = 0;
    virtual int adjustment_get_lower() const = 0;
    virtual void adjustment_set_lower(int lower) = 0;

    virtual int get_scroll_thickness() const = 0;
    virtual void set_scroll_thickness(int nThickness) = 0;
    virtual void set_scroll_swap_arrows(bool bSwap) = 0;

    virtual ScrollType get_scroll_type() const = 0;

    void connect_adjustment_value_changed(const Link<Scrollbar&, void>& rLink)
    {
        m_aValueChangeHdl = rLink;
    }
};

class VCL_DLLPUBLIC ColorChooserDialog : virtual public Dialog
{
public:
    virtual void set_color(const Color& rColor) = 0;
    virtual Color get_color() const = 0;
};

class VCL_DLLPUBLIC SizeGroup
{
public:
    virtual void add_widget(weld::Widget* pWidget) = 0;
    // the default mode is VclSizeGroupMode::Horizontal
    virtual void set_mode(VclSizeGroupMode eMode) = 0;
    virtual ~SizeGroup() {}
};

void Dialog::set_default_response(int nResponse)
{
    std::unique_ptr<weld::Button> pButton = weld_button_for_response(nResponse);
    change_default_button(nullptr, pButton.get());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
