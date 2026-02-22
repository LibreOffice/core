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
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <utility>

#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>

#include <assert.h>
#include <functional>
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
namespace rtl
{
template <class reference_type> class Reference;
}
typedef css::uno::Reference<css::accessibility::XAccessibleRelationSet> a11yrelationset;
enum class PointerStyle;
enum class VclSizeGroupMode;
class Color;
class CommandEvent;
class Date;
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
class Font;
enum class WindowDataMask;
}
template <class reference_type> class VclPtr;

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

class VCL_DLLPUBLIC Frame : virtual public Container
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
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

enum class Placement
{
    Under,
    End
};

class VCL_DLLPUBLIC SizeGroup
{
public:
    virtual void add_widget(weld::Widget* pWidget) = 0;
    // the default mode is VclSizeGroupMode::Horizontal
    virtual void set_mode(VclSizeGroupMode eMode) = 0;
    virtual ~SizeGroup() {}
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
