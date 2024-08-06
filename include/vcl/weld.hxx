/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_WELD_HXX
#define INCLUDED_VCL_WELD_HXX

#include <basegfx/range/b2irange.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <tools/date.hxx>
#include <tools/fldunit.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <utility>
#include <vcl/vclenum.hxx>
#include <vcl/font.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/uitest/factory.hxx>
#include <vcl/windowstate.hxx>

#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>

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
typedef css::uno::Reference<css::accessibility::XAccessible> a11yref;
typedef css::uno::Reference<css::accessibility::XAccessibleRelationSet> a11yrelationset;
enum class PointerStyle;
class CommandEvent;
class Formatter;
class InputContext;
class KeyEvent;
class MouseEvent;
class SvNumberFormatter;
class TransferDataContainer;
class OutputDevice;
class VirtualDevice;
struct SystemEnvData;

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

class VCL_DLLPUBLIC Widget
{
    friend class ::LOKTrigger;

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

public:
    virtual void set_sensitive(bool sensitive) = 0;
    virtual bool get_sensitive() const = 0;

    /* visibility */

    virtual void show() = 0;
    virtual void hide() = 0;

    // This function simply calls show() or hide() but is convenient when the
    // visibility depends on some condition
    virtual void set_visible(bool visible)
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

    // return if this widget has the keyboard focus within the active window
    // TODO: review if this has any practical difference from has_focus()
    virtual bool is_active() const = 0;

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

    virtual void set_grid_left_attach(int nAttach) = 0;
    virtual int get_grid_left_attach() const = 0;
    virtual void set_grid_width(int nCols) = 0;
    virtual void set_grid_top_attach(int nAttach) = 0;
    virtual int get_grid_top_attach() const = 0;

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

    virtual void grab_add() = 0;
    virtual bool has_grab() const = 0;
    virtual void grab_remove() = 0;

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
       string contents which are interpreted by it's ui.
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

class VCL_DLLPUBLIC Paned : virtual public Container
{
public:
    // set pixel position of divider
    virtual void set_position(int nPos) = 0;
    // get pixel position of divider
    virtual int get_position() const = 0;
};

class VCL_DLLPUBLIC ScrolledWindow : virtual public Container
{
    friend class ::LOKTrigger;

    Link<ScrolledWindow&, void> m_aVChangeHdl;
    Link<ScrolledWindow&, void> m_aHChangeHdl;

protected:
    void signal_vadjustment_changed() { m_aVChangeHdl.Call(*this); }
    void signal_hadjustment_changed() { m_aHChangeHdl.Call(*this); }

public:
    virtual void hadjustment_configure(int value, int lower, int upper, int step_increment,
                                       int page_increment, int page_size)
        = 0;
    virtual int hadjustment_get_value() const = 0;
    virtual void hadjustment_set_value(int value) = 0;
    virtual int hadjustment_get_upper() const = 0;
    virtual void hadjustment_set_upper(int upper) = 0;
    virtual int hadjustment_get_page_size() const = 0;
    virtual void hadjustment_set_page_size(int size) = 0;
    virtual void hadjustment_set_page_increment(int size) = 0;
    virtual void hadjustment_set_step_increment(int size) = 0;
    virtual void set_hpolicy(VclPolicyType eHPolicy) = 0;
    virtual VclPolicyType get_hpolicy() const = 0;
    void connect_hadjustment_changed(const Link<ScrolledWindow&, void>& rLink)
    {
        m_aHChangeHdl = rLink;
    }

    virtual void vadjustment_configure(int value, int lower, int upper, int step_increment,
                                       int page_increment, int page_size)
        = 0;
    virtual int vadjustment_get_value() const = 0;
    virtual void vadjustment_set_value(int value) = 0;
    virtual int vadjustment_get_upper() const = 0;
    virtual void vadjustment_set_upper(int upper) = 0;
    virtual int vadjustment_get_page_size() const = 0;
    virtual void vadjustment_set_page_size(int size) = 0;
    virtual void vadjustment_set_page_increment(int size) = 0;
    virtual void vadjustment_set_step_increment(int size) = 0;
    virtual int vadjustment_get_lower() const = 0;
    virtual void vadjustment_set_lower(int upper) = 0;
    virtual void set_vpolicy(VclPolicyType eVPolicy) = 0;
    virtual VclPolicyType get_vpolicy() const = 0;
    void connect_vadjustment_changed(const Link<ScrolledWindow&, void>& rLink)
    {
        m_aVChangeHdl = rLink;
    }
    virtual int get_scroll_thickness() const = 0;
    virtual void set_scroll_thickness(int nThickness) = 0;

    //trying to use custom color for a scrollbar is generally a bad idea.
    virtual void customize_scrollbars(const Color& rBackgroundColor, const Color& rShadowColor,
                                      const Color& rFaceColor)
        = 0;
};

class Label;

class VCL_DLLPUBLIC Frame : virtual public Container
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual std::unique_ptr<Label> weld_label_widget() const = 0;
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
    virtual void insert_page(const OUString& rIdent, const OUString& rLabel, int nPos) = 0;
    void append_page(const OUString& rIdent, const OUString& rLabel)
    {
        insert_page(rIdent, rLabel, -1);
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
    virtual void set_modal(bool bModal) = 0;
    virtual bool get_modal() const = 0;
    virtual bool get_resizable() const = 0;
    virtual Size get_size() const = 0;
    virtual Point get_position() const = 0;
    virtual AbsoluteScreenPixelRectangle get_monitor_workarea() const = 0;
    // center window on is parent
    //
    // bTrackGeometryRequests set to true tries to ensure the window will end
    // up still centered on its parent windows final size, taking into account
    // that there may currently be pending geometry requests for the parent not
    // yet processed by the underlying toolkit
    //
    // for e.g gtk this will means the window is always centered even when
    // resized, calling set_centered_on_parent with false will turn this
    // off again.
    virtual void set_centered_on_parent(bool bTrackGeometryRequests) = 0;
    // returns whether the widget that has focus is within this Window
    // (its very possible to move this to weld::Container if that becomes
    // desirable)
    virtual bool has_toplevel_focus() const = 0;
    virtual void present() = 0;

    // with pOld of null, automatically find the old default widget and unset
    // it, otherwise use as hint to the old default
    virtual void change_default_widget(weld::Widget* pOld, weld::Widget* pNew) = 0;
    virtual bool is_default_widget(const weld::Widget* pCandidate) const = 0;

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
    virtual int run() = 0;
    // Run async without a controller
    // @param self - must point to this, to enforce that the dialog was created/held by a shared_ptr
    virtual bool runAsync(std::shared_ptr<Dialog> const& rxSelf,
                          const std::function<void(sal_Int32)>& func)
        = 0;
    virtual void response(int response) = 0;
    virtual void add_button(const OUString& rText, int response, const OUString& rHelpId = {}) = 0;
    virtual void set_default_response(int response) = 0;
    virtual Button* weld_widget_for_response(int response) = 0;
    virtual Container* weld_content_area() = 0;

    // shrink the dialog down to shown just these widgets
    virtual void collapse(weld::Widget* pEdit, weld::Widget* pButton) = 0;
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
    virtual Container* weld_message_area() = 0;
};

class VCL_DLLPUBLIC Assistant : virtual public Dialog
{
    Link<const OUString&, bool> m_aJumpPageHdl;

protected:
    bool signal_jump_page(const OUString& rIdent) { return m_aJumpPageHdl.Call(rIdent); }

public:
    virtual int get_current_page() const = 0;
    virtual int get_n_pages() const = 0;
    virtual OUString get_page_ident(int nPage) const = 0;
    virtual OUString get_current_page_ident() const = 0;
    virtual void set_current_page(int nPage) = 0;
    virtual void set_current_page(const OUString& rIdent) = 0;
    // move the page rIdent to position nIndex
    virtual void set_page_index(const OUString& rIdent, int nIndex) = 0;
    virtual void set_page_title(const OUString& rIdent, const OUString& rTitle) = 0;
    virtual OUString get_page_title(const OUString& rIdent) const = 0;
    virtual void set_page_sensitive(const OUString& rIdent, bool bSensitive) = 0;
    virtual weld::Container* append_page(const OUString& rIdent) = 0;

    virtual void set_page_side_help_id(const OUString& rHelpId) = 0;

    virtual void set_page_side_image(const OUString& rImage) = 0;

    void connect_jump_page(const Link<const OUString&, bool>& rLink) { m_aJumpPageHdl = rLink; }
};

inline OUString toId(const void* pValue)
{
    return OUString::number(reinterpret_cast<sal_uIntPtr>(pValue));
}

template <typename T> T fromId(const OUString& rValue)
{
    return reinterpret_cast<T>(rValue.toUInt64());
}

struct VCL_DLLPUBLIC ComboBoxEntry
{
    OUString sString;
    OUString sId;
    OUString sImage;
    ComboBoxEntry(OUString _aString)
        : sString(std::move(_aString))
    {
    }
    ComboBoxEntry(OUString _aString, OUString _aId)
        : sString(std::move(_aString))
        , sId(std::move(_aId))
    {
    }
    ComboBoxEntry(OUString _aString, OUString _aId, OUString _aImage)
        : sString(std::move(_aString))
        , sId(std::move(_aId))
        , sImage(std::move(_aImage))
    {
    }
};

enum class EntryMessageType
{
    Normal,
    Warning,
    Error,
};

class Menu;

/// A widget used to choose from a list of items.
class VCL_DLLPUBLIC ComboBox : virtual public Widget
{
private:
    OUString m_sSavedValue;
    std::vector<OUString> m_aSavedValues;

public:
    // OUString is the id of the row, it may be null to measure the height of a generic line
    typedef std::tuple<vcl::RenderContext&, const tools::Rectangle&, bool, const OUString&>
        render_args;

protected:
    Link<ComboBox&, void> m_aChangeHdl;
    Link<ComboBox&, void> m_aPopupToggledHdl;
    Link<ComboBox&, bool> m_aEntryActivateHdl;
    Link<OUString&, bool> m_aEntryInsertTextHdl;

    friend class ::LOKTrigger;

    void signal_changed() { m_aChangeHdl.Call(*this); }

    virtual void signal_popup_toggled() { m_aPopupToggledHdl.Call(*this); }

    Link<render_args, void> m_aRenderHdl;
    void signal_custom_render(vcl::RenderContext& rDevice, const tools::Rectangle& rRect,
                              bool bSelected, const OUString& rId)
    {
        m_aRenderHdl.Call(render_args(rDevice, rRect, bSelected, rId));
    }

    Link<vcl::RenderContext&, Size> m_aGetSizeHdl;
    Size signal_custom_get_size(vcl::RenderContext& rDevice) { return m_aGetSizeHdl.Call(rDevice); }

public:
    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface)
        = 0;
    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems, bool bKeepExisting)
        = 0;
    void insert(int pos, const weld::ComboBoxEntry& rItem)
    {
        insert(pos, rItem.sString, rItem.sId.isEmpty() ? nullptr : &rItem.sId,
               rItem.sImage.isEmpty() ? nullptr : &rItem.sImage, nullptr);
    }
    void insert_text(int pos, const OUString& rStr)
    {
        insert(pos, rStr, nullptr, nullptr, nullptr);
    }
    void append(const weld::ComboBoxEntry& rItem) { insert(-1, rItem); }
    void append_text(const OUString& rStr) { insert(-1, rStr, nullptr, nullptr, nullptr); }
    void append(const OUString& rId, const OUString& rStr)
    {
        insert(-1, rStr, &rId, nullptr, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(-1, rStr, &rId, &rImage, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr, VirtualDevice& rImage)
    {
        insert(-1, rStr, &rId, nullptr, &rImage);
    }
    void append(int pos, const OUString& rId, const OUString& rStr)
    {
        insert(pos, rStr, &rId, nullptr, nullptr);
    }
    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }

    virtual int get_count() const = 0;
    virtual void make_sorted() = 0;
    virtual void clear() = 0;

    //by index
    virtual int get_active() const = 0;
    virtual void set_active(int pos) = 0;
    virtual void remove(int pos) = 0;

    //by text
    virtual OUString get_active_text() const = 0;
    void set_active_text(const OUString& rStr) { set_active(find_text(rStr)); }
    virtual OUString get_text(int pos) const = 0;
    virtual int find_text(const OUString& rStr) const = 0;
    void remove_text(const OUString& rText) { remove(find_text(rText)); }

    //by id
    virtual OUString get_active_id() const = 0;
    virtual void set_active_id(const OUString& rStr) = 0;
    virtual OUString get_id(int pos) const = 0;
    virtual void set_id(int row, const OUString& rId) = 0;
    virtual int find_id(const OUString& rId) const = 0;
    void remove_id(const OUString& rId) { remove(find_id(rId)); }

    /* m_aChangeHdl is called when the active item is changed. The can be due
       to the user selecting a different item from the list or while typing
       into the entry of a combo box with an entry.

       Use changed_by_direct_pick() to discover whether an item was actually explicitly
       selected, e.g. from the menu.
     */
    void connect_changed(const Link<ComboBox&, void>& rLink) { m_aChangeHdl = rLink; }

    virtual bool changed_by_direct_pick() const = 0;

    virtual void connect_popup_toggled(const Link<ComboBox&, void>& rLink)
    {
        m_aPopupToggledHdl = rLink;
    }

    //entry related
    virtual bool has_entry() const = 0;
    virtual void set_entry_message_type(EntryMessageType eType) = 0;
    virtual void set_entry_text(const OUString& rStr) = 0;
    virtual void set_entry_width_chars(int nChars) = 0;
    virtual void set_entry_max_length(int nChars) = 0;
    virtual void select_entry_region(int nStartPos, int nEndPos) = 0;
    virtual bool get_entry_selection_bounds(int& rStartPos, int& rEndPos) = 0;
    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive = false) = 0;
    virtual void set_entry_placeholder_text(const OUString& rText) = 0;
    virtual void set_entry_editable(bool bEditable) = 0;
    virtual void cut_entry_clipboard() = 0;
    virtual void copy_entry_clipboard() = 0;
    virtual void paste_entry_clipboard() = 0;

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_entry_font(const vcl::Font& rFont) = 0;
    virtual vcl::Font get_entry_font() = 0;

    virtual bool get_popup_shown() const = 0;

    void connect_entry_insert_text(const Link<OUString&, bool>& rLink)
    {
        m_aEntryInsertTextHdl = rLink;
    }

    // callback returns true to indicated no further processing of activate wanted
    void connect_entry_activate(const Link<ComboBox&, bool>& rLink) { m_aEntryActivateHdl = rLink; }

    void save_value() { m_sSavedValue = get_active_text(); }

    void save_values_by_id(const OUString& rId)
    {
        m_aSavedValues.push_back(get_text(find_id(rId)));
    }

    OUString const& get_saved_value() const { return m_sSavedValue; }
    OUString const& get_saved_values(int pos) const { return m_aSavedValues[pos]; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_active_text(); }
    bool get_values_changed_from_saved() const
    {
        return !m_aSavedValues.empty()
               && std::find(m_aSavedValues.begin(), m_aSavedValues.end(), get_active_text())
                      == m_aSavedValues.end();
    }

    void removeSavedValues() { m_aSavedValues.clear(); }

    // for custom rendering a row
    void connect_custom_get_size(const Link<vcl::RenderContext&, Size>& rLink)
    {
        m_aGetSizeHdl = rLink;
    }
    void connect_custom_render(const Link<render_args, void>& rLink) { m_aRenderHdl = rLink; }
    // call set_custom_renderer after setting custom callbacks
    virtual void set_custom_renderer(bool bOn) = 0;
    // create a virtual device compatible with the device passed in render_args wrt alpha
    virtual VclPtr<VirtualDevice> create_render_virtual_device() const = 0;
    // set a sub menu for a entry, only works with custom rendering
    virtual void set_item_menu(const OUString& rIdent, weld::Menu* pMenu) = 0;
    // get the width needed to show the menu launcher in a custom row
    virtual int get_menu_button_width() const = 0;

    // for mru support
    virtual int get_max_mru_count() const = 0;
    virtual void set_max_mru_count(int nCount) = 0;
    virtual OUString get_mru_entries() const = 0;
    virtual void set_mru_entries(const OUString& rEntries) = 0;

    // Backwards compatibility, should be avoided to allow
    // UI consistency.
    virtual void set_max_drop_down_rows(int nRows) = 0;
};

enum class ColumnToggleType
{
    Check,
    Radio
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

/* Model column indexes are considered to begin at 0, but with special columns
   before index 0. A expander image column (and an additional optional toggle
   button column when enable_toggle_buttons is used). Column index -1 is
   reserved to access those columns.
*/
class VCL_DLLPUBLIC TreeView : virtual public Widget
{
    friend class ::LOKTrigger;

public:
    typedef std::pair<const TreeIter&, int> iter_col;
    typedef std::pair<const TreeIter&, OUString> iter_string;
    // OUString is the id of the row, it may be null to measure the height of a generic line
    typedef std::pair<vcl::RenderContext&, const OUString&> get_size_args;
    typedef std::tuple<vcl::RenderContext&, const tools::Rectangle&, bool, const OUString&>
        render_args;

private:
    OUString m_sSavedValue;

protected:
    Link<TreeView&, void> m_aChangeHdl;
    Link<TreeView&, bool> m_aRowActivatedHdl;
    Link<int, void> m_aColumnClickedHdl;
    Link<const iter_col&, void> m_aRadioToggleHdl;
    Link<const TreeIter&, bool> m_aEditingStartedHdl;
    Link<const iter_string&, bool> m_aEditingDoneHdl;
    // if handler returns false, the expansion of the row is refused
    Link<const TreeIter&, bool> m_aExpandingHdl;
    // if handler returns false, the collapse of the row is refused
    Link<const TreeIter&, bool> m_aCollapsingHdl;
    Link<TreeView&, void> m_aVisibleRangeChangedHdl;
    Link<TreeView&, void> m_aModelChangedHdl;
    // if handler returns true, then menu has been show and event is consumed
    Link<const CommandEvent&, bool> m_aPopupMenuHdl;
    // if handler returns true, drag is disallowed, consumer can change bool
    // arg to false to disable the treeview default dnd icon
    Link<bool&, bool> m_aDragBeginHdl;
    std::function<int(const weld::TreeIter&, const weld::TreeIter&)> m_aCustomSort;

protected:
    void signal_changed() { m_aChangeHdl.Call(*this); }
    bool signal_row_activated() { return m_aRowActivatedHdl.Call(*this); }
    void signal_column_clicked(int nColumn) { m_aColumnClickedHdl.Call(nColumn); }
    bool signal_expanding(const TreeIter& rIter)
    {
        return !m_aExpandingHdl.IsSet() || m_aExpandingHdl.Call(rIter);
    }
    bool signal_collapsing(const TreeIter& rIter)
    {
        return !m_aCollapsingHdl.IsSet() || m_aCollapsingHdl.Call(rIter);
    }
    void signal_visible_range_changed() { m_aVisibleRangeChangedHdl.Call(*this); }
    void signal_model_changed() { m_aModelChangedHdl.Call(*this); }

    void signal_toggled(const iter_col& rIterCol) { m_aRadioToggleHdl.Call(rIterCol); }

    bool signal_editing_started(const TreeIter& rIter) { return m_aEditingStartedHdl.Call(rIter); }

    bool signal_editing_done(const iter_string& rIterText)
    {
        return m_aEditingDoneHdl.Call(rIterText);
    }

    Link<const TreeIter&, OUString> m_aQueryTooltipHdl;
    OUString signal_query_tooltip(const TreeIter& rIter) { return m_aQueryTooltipHdl.Call(rIter); }

    Link<render_args, void> m_aRenderHdl;
    void signal_custom_render(vcl::RenderContext& rDevice, const tools::Rectangle& rRect,
                              bool bSelected, const OUString& rId)
    {
        m_aRenderHdl.Call(render_args(rDevice, rRect, bSelected, rId));
    }

    Link<get_size_args, Size> m_aGetSizeHdl;
    Size signal_custom_get_size(vcl::RenderContext& rDevice, const OUString& rId)
    {
        return m_aGetSizeHdl.Call(get_size_args(rDevice, rId));
    }

public:
    virtual void connect_query_tooltip(const Link<const TreeIter&, OUString>& rLink)
    {
        assert(!m_aQueryTooltipHdl.IsSet() || !rLink.IsSet());
        m_aQueryTooltipHdl = rLink;
    }

    // see 'expanding on-demand node details' for bChildrenOnDemand of true
    virtual void insert(const TreeIter* pParent, int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        bool bChildrenOnDemand, TreeIter* pRet)
        = 0;

    void insert(int nRow, TreeIter* pRet = nullptr)
    {
        insert(nullptr, nRow, nullptr, nullptr, nullptr, nullptr, false, pRet);
    }

    void append(TreeIter* pRet = nullptr) { insert(-1, pRet); }

    void insert(int pos, const OUString& rStr, const OUString* pId, const OUString* pIconName,
                VirtualDevice* pImageSurface)
    {
        insert(nullptr, pos, &rStr, pId, pIconName, pImageSurface, false, nullptr);
    }
    void insert_text(int pos, const OUString& rStr)
    {
        insert(nullptr, pos, &rStr, nullptr, nullptr, nullptr, false, nullptr);
    }
    void append_text(const OUString& rStr)
    {
        insert(nullptr, -1, &rStr, nullptr, nullptr, nullptr, false, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr)
    {
        insert(nullptr, -1, &rStr, &rId, nullptr, nullptr, false, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(nullptr, -1, &rStr, &rId, &rImage, nullptr, false, nullptr);
    }
    void append(const TreeIter* pParent, const OUString& rStr)
    {
        insert(pParent, -1, &rStr, nullptr, nullptr, nullptr, false, nullptr);
    }

    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }

    void connect_changed(const Link<TreeView&, void>& rLink) { m_aChangeHdl = rLink; }

    /* A row is "activated" when the user double clicks a treeview row. It may
       also be emitted when a row is selected and Space or Enter is pressed.

       a return of "true" means the activation has been handled, a "false" propagates
       the activation to the default handler which expands/collapses the row, if possible.
    */
    void connect_row_activated(const Link<TreeView&, bool>& rLink) { m_aRowActivatedHdl = rLink; }

    // Argument is a pair of iter, col describing the toggled node
    void connect_toggled(const Link<const iter_col&, void>& rLink) { m_aRadioToggleHdl = rLink; }

    void connect_column_clicked(const Link<int, void>& rLink) { m_aColumnClickedHdl = rLink; }
    void connect_model_changed(const Link<TreeView&, void>& rLink) { m_aModelChangedHdl = rLink; }

    virtual OUString get_selected_text() const = 0;
    virtual OUString get_selected_id() const = 0;

    // call before inserting any content and connecting to toggle signals,
    // an pre-inserted checkbutton column will exist at the start of every row
    // inserted after this call which can be accessed with col index -1
    virtual void enable_toggle_buttons(ColumnToggleType eType) = 0;

    virtual void set_clicks_to_toggle(int nToggleBehavior) = 0;

    //by index
    virtual int get_selected_index() const = 0;
    //Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    virtual void select(int pos) = 0;
    virtual void unselect(int pos) = 0;
    virtual void remove(int pos) = 0;
    // col index -1 gets the first text column
    virtual OUString get_text(int row, int col = -1) const = 0;
    // col index -1 sets the first text column
    virtual void set_text(int row, const OUString& rText, int col = -1) = 0;
    // col index -1 sets all columns
    virtual void set_sensitive(int row, bool bSensitive, int col = -1) = 0;
    virtual bool get_sensitive(int row, int col) const = 0;
    virtual void set_id(int row, const OUString& rId) = 0;
    // col index -1 sets the expander toggle, enable_toggle_buttons must have been called to create that column
    virtual void set_toggle(int row, TriState eState, int col = -1) = 0;
    // col index -1 gets the expander toggle, enable_toggle_buttons must have been called to create that column
    virtual TriState get_toggle(int row, int col = -1) const = 0;
    // col index -1 sets the expander image
    virtual void set_image(int row, const OUString& rImage, int col = -1) = 0;
    // col index -1 sets the expander image
    virtual void set_image(int row, VirtualDevice& rImage, int col = -1) = 0;
    // col index -1 sets the expander image
    virtual void set_image(int row, const css::uno::Reference<css::graphic::XGraphic>& rImage,
                           int col = -1)
        = 0;
    virtual void set_text_emphasis(int row, bool bOn, int col) = 0;
    virtual bool get_text_emphasis(int row, int col) const = 0;
    virtual void set_text_align(int row, double fAlign, int col) = 0;
    virtual void swap(int pos1, int pos2) = 0;
    virtual std::vector<int> get_selected_rows() const = 0;
    virtual void set_font_color(int pos, const Color& rColor) = 0;
    // scroll to make 'row' visible, this will also expand all parent rows of 'row' as necessary to
    // make 'row' visible
    virtual void scroll_to_row(int row) = 0;
    virtual bool is_selected(int pos) const = 0;
    virtual int get_cursor_index() const = 0;
    virtual void set_cursor(int pos) = 0;

    //by text
    virtual int find_text(const OUString& rText) const = 0;
    //Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    void select_text(const OUString& rText) { select(find_text(rText)); }
    void remove_text(const OUString& rText) { remove(find_text(rText)); }
    std::vector<OUString> get_selected_rows_text() const
    {
        std::vector<int> aRows(get_selected_rows());
        std::vector<OUString> aRet;
        aRet.reserve(aRows.size());
        for (auto a : aRows)
            aRet.push_back(get_text(a));
        return aRet;
    }

    //by id
    virtual OUString get_id(int pos) const = 0;
    virtual int find_id(const OUString& rId) const = 0;
    //Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    void select_id(const OUString& rId) { select(find_id(rId)); }
    void remove_id(const OUString& rText) { remove(find_id(rText)); }

    //via iter
    virtual std::unique_ptr<TreeIter> make_iterator(const TreeIter* pOrig = nullptr) const = 0;
    virtual void copy_iterator(const TreeIter& rSource, TreeIter& rDest) const = 0;
    virtual bool get_selected(TreeIter* pIter) const = 0;
    virtual bool get_cursor(TreeIter* pIter) const = 0;
    virtual void set_cursor(const TreeIter& rIter) = 0;
    virtual bool get_iter_first(TreeIter& rIter) const = 0;
    // set iter to point to next node at the current level
    virtual bool iter_next_sibling(TreeIter& rIter) const = 0;
    // set iter to point to previous node at the current level
    virtual bool iter_previous_sibling(TreeIter& rIter) const = 0;
    // set iter to point to next node, depth first, then sibling
    virtual bool iter_next(TreeIter& rIter) const = 0;
    // set iter to point to previous node, sibling first then depth
    virtual bool iter_previous(TreeIter& rIter) const = 0;
    // set iter to point to first child node
    virtual bool iter_children(TreeIter& rIter) const = 0;
    bool iter_nth_sibling(TreeIter& rIter, int nChild) const
    {
        bool bRet = true;
        for (int i = 0; i < nChild && bRet; ++i)
            bRet = iter_next_sibling(rIter);
        return bRet;
    }
    bool iter_nth_child(TreeIter& rIter, int nChild) const
    {
        if (!iter_children(rIter))
            return false;
        return iter_nth_sibling(rIter, nChild);
    }
    virtual bool iter_parent(TreeIter& rIter) const = 0;
    virtual int get_iter_depth(const TreeIter& rIter) const = 0;
    virtual int get_iter_index_in_parent(const TreeIter& rIter) const = 0;
    /* Compares two paths. If a appears before b in a tree, then -1 is returned.
       If b appears before a , then 1 is returned. If the two nodes are equal,
       then 0 is returned.
    */
    virtual int iter_compare(const TreeIter& a, const TreeIter& b) const = 0;
    virtual bool iter_has_child(const TreeIter& rIter) const = 0;
    // returns the number of direct children rIter has
    virtual int iter_n_children(const TreeIter& rIter) const = 0;
    virtual void remove(const TreeIter& rIter) = 0;
    //Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    virtual void select(const TreeIter& rIter) = 0;
    virtual void unselect(const TreeIter& rIter) = 0;
    //visually indent this row as if it was at get_iter_depth() + nIndentLevel
    virtual void set_extra_row_indent(const TreeIter& rIter, int nIndentLevel) = 0;
    // col index -1 sets the first text column
    virtual void set_text(const TreeIter& rIter, const OUString& rStr, int col = -1) = 0;
    // col index -1 sets all columns
    virtual void set_sensitive(const TreeIter& rIter, bool bSensitive, int col = -1) = 0;
    virtual bool get_sensitive(const TreeIter& rIter, int col) const = 0;
    virtual void set_text_emphasis(const TreeIter& rIter, bool bOn, int col) = 0;
    virtual bool get_text_emphasis(const TreeIter& rIter, int col) const = 0;
    virtual void set_text_align(const TreeIter& rIter, double fAlign, int col) = 0;
    // col index -1 sets the expander toggle, enable_toggle_buttons must have been called to create that column
    virtual void set_toggle(const TreeIter& rIter, TriState bOn, int col = -1) = 0;
    // col index -1 gets the expander toggle, enable_toggle_buttons must have been called to create that column
    virtual TriState get_toggle(const TreeIter& rIter, int col = -1) const = 0;
    // col index -1 gets the first text column
    virtual OUString get_text(const TreeIter& rIter, int col = -1) const = 0;
    virtual void set_id(const TreeIter& rIter, const OUString& rId) = 0;
    virtual OUString get_id(const TreeIter& rIter) const = 0;
    // col index -1 sets the expander image
    virtual void set_image(const TreeIter& rIter, const OUString& rImage, int col = -1) = 0;
    // col index -1 sets the expander image
    virtual void set_image(const TreeIter& rIter, VirtualDevice& rImage, int col = -1) = 0;
    // col index -1 sets the expander image
    virtual void set_image(const TreeIter& rIter,
                           const css::uno::Reference<css::graphic::XGraphic>& rImage, int col = -1)
        = 0;
    virtual void set_font_color(const TreeIter& rIter, const Color& rColor) = 0;
    // scroll to make rIter visible, this will also expand all parent rows of rIter as necessary to
    // make rIter visible
    virtual void scroll_to_row(const TreeIter& rIter) = 0;
    virtual bool is_selected(const TreeIter& rIter) const = 0;

    virtual void move_subtree(TreeIter& rNode, const TreeIter* pNewParent, int nIndexInNewParent)
        = 0;

    // call func on each element until func returns true or we run out of elements
    virtual void all_foreach(const std::function<bool(TreeIter&)>& func) = 0;
    // call func on each selected element until func returns true or we run out of elements
    virtual void selected_foreach(const std::function<bool(TreeIter&)>& func) = 0;
    // call func on each visible element until func returns true or we run out of elements
    virtual void visible_foreach(const std::function<bool(TreeIter&)>& func) = 0;
    // clear the children of pParent (whole tree if nullptr),
    // then add nSourceCount rows under pParent, call func on each row
    // inserted with an arg of the index that this row will be when bulk insert
    // ends.
    //
    // this enables inserting the entries backwards in models where that is faster,
    //
    // pFixedWidths is optional, when present each matching entry col text
    // width will not be measured, and the fixed width used instead. Use
    // sparingly because wider text than the fixed width is clipped and cannot
    // be scrolled into view horizontally.
    virtual void bulk_insert_for_each(int nSourceCount,
                                      const std::function<void(TreeIter&, int nSourceIndex)>& func,
                                      const weld::TreeIter* pParent = nullptr,
                                      const std::vector<int>* pFixedWidths = nullptr)
        = 0;

    /* expanding on-demand node details

    When a node is added with children-on-demand (typically via 'insert' with
    bChildrenOnDemand of true), then initially in reality the
    children-on-demand node is given a 'placeholder' child entry to indicate
    the load-on-demand state.

    The 'placeholder' needs to be there for the expander indicator to be
    drawn/shown even when there are no "real" entries yet. This child doesn't
    exist for the purposes of any of the iterator methods, e.g. iter_has_child
    on an on-demand node which hasn't been expanded yet is false. Likewise the
    rest of the iterator methods skip over or otherwise ignore that node.

    Normal usage is the user clicks on the expander, the expansion mechanism
    removes the 'placeholder' entry (set_children_on_demand(false)) and calls
    any installed expanding-callback (installable via connect_expanding) which
    has the opportunity to populate the node with children.

    If you decide to directly populate the children of an on-demand node
    outside of the expanding-callback then you also need to explicitly remove
    the 'placeholder' with set_children_on_demand(false) otherwise the treeview
    is in an inconsistent state.  */

    virtual bool get_row_expanded(const TreeIter& rIter) const = 0;
    // expand row will first trigger the callback set via connect_expanding before expanding
    virtual void expand_row(const TreeIter& rIter) = 0;
    // collapse row will first trigger the callback set via connect_collapsing before collapsing
    virtual void collapse_row(const TreeIter& rIter) = 0;
    // set the empty node to appear as if it has children, true is equivalent
    // to 'insert' with a bChildrenOnDemand of true. See notes above.
    virtual void set_children_on_demand(const TreeIter& rIter, bool bChildrenOnDemand) = 0;
    // return if the node is configured to be populated on-demand
    virtual bool get_children_on_demand(const TreeIter& rIter) const = 0;
    // set if the expanders are shown or not
    virtual void set_show_expanders(bool bShow) = 0;

    void connect_expanding(const Link<const TreeIter&, bool>& rLink) { m_aExpandingHdl = rLink; }
    void connect_collapsing(const Link<const TreeIter&, bool>& rLink) { m_aCollapsingHdl = rLink; }

    // rStartLink returns true to allow editing, false to disallow
    // rEndLink returns true to accept the edit, false to reject
    virtual void connect_editing(const Link<const TreeIter&, bool>& rStartLink,
                                 const Link<const iter_string&, bool>& rEndLink)
    {
        assert(rStartLink.IsSet() == rEndLink.IsSet() && "should be both on or both off");
        m_aEditingStartedHdl = rStartLink;
        m_aEditingDoneHdl = rEndLink;
    }

    virtual void start_editing(const weld::TreeIter& rEntry) = 0;
    virtual void end_editing() = 0;

    virtual void connect_visible_range_changed(const Link<TreeView&, void>& rLink)
    {
        assert(!m_aVisibleRangeChangedHdl.IsSet() || !rLink.IsSet());
        m_aVisibleRangeChangedHdl = rLink;
    }

    virtual void connect_popup_menu(const Link<const CommandEvent&, bool>& rLink)
    {
        m_aPopupMenuHdl = rLink;
    }

    virtual void enable_drag_source(rtl::Reference<TransferDataContainer>& rTransferable,
                                    sal_uInt8 eDNDConstants)
        = 0;

    void connect_drag_begin(const Link<bool&, bool>& rLink) { m_aDragBeginHdl = rLink; }

    //all of them. Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    void select_all() { unselect(-1); }
    void unselect_all() { select(-1); }

    // return the number of toplevel nodes
    virtual int n_children() const = 0;

    // afterwards, entries will be in default ascending sort order
    virtual void make_sorted() = 0;
    virtual void make_unsorted() = 0;
    virtual bool get_sort_order() const = 0;
    virtual void set_sort_order(bool bAscending) = 0;

    // TRUE ascending, FALSE, descending, INDET, neither (off)
    virtual void set_sort_indicator(TriState eState, int nColumn) = 0;
    virtual TriState get_sort_indicator(int nColumn) const = 0;

    virtual int get_sort_column() const = 0;
    virtual void set_sort_column(int nColumn) = 0;

    virtual void
    set_sort_func(const std::function<int(const weld::TreeIter&, const weld::TreeIter&)>& func)
    {
        m_aCustomSort = func;
    }

    virtual void clear() = 0;
    virtual int get_height_rows(int nRows) const = 0;

    virtual void columns_autosize() = 0;
    virtual void set_column_fixed_widths(const std::vector<int>& rWidths) = 0;
    virtual void set_column_editables(const std::vector<bool>& rEditables) = 0;
    virtual int get_column_width(int nCol) const = 0;
    virtual void set_centered_column(int nCol) = 0;
    virtual OUString get_column_title(int nColumn) const = 0;
    virtual void set_column_title(int nColumn, const OUString& rTitle) = 0;

    int get_checkbox_column_width() const { return get_approximate_digit_width() * 3 + 6; }

    virtual void set_selection_mode(SelectionMode eMode) = 0;
    virtual int count_selected_rows() const = 0;
    // remove the selected nodes
    virtual void remove_selection() = 0;

    // only meaningful is call this from a "changed" callback, true if the change
    // was due to mouse hovering over the entry
    virtual bool changed_by_hover() const = 0;

    virtual void vadjustment_set_value(int value) = 0;
    virtual int vadjustment_get_value() const = 0;

    void save_value() { m_sSavedValue = get_selected_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_selected_text(); }

    // for custom rendering a cell
    void connect_custom_get_size(const Link<get_size_args, Size>& rLink) { m_aGetSizeHdl = rLink; }
    void connect_custom_render(const Link<render_args, void>& rLink) { m_aRenderHdl = rLink; }
    // call set_column_custom_renderer after setting custom callbacks
    virtual void set_column_custom_renderer(int nColumn, bool bEnable) = 0;
    // redraw all rows, typically only useful with custom rendering to redraw due to external
    // state change
    virtual void queue_draw() = 0;

    /* with bDnDMode false simply return the row under the point
     *
     * with bDnDMode true:
     * a) return the row which should be dropped on, which may
     *    be different from the row the mouse is over in some backends where
     *    positioning the mouse on the bottom half of a row indicates to drop
     *    after the row
     * b) dnd highlight the dest row
     */
    virtual bool get_dest_row_at_pos(const Point& rPos, weld::TreeIter* pResult, bool bDnDMode,
                                     bool bAutoScroll = true)
        = 0;
    virtual void unset_drag_dest_row() = 0;
    virtual tools::Rectangle get_row_area(const weld::TreeIter& rIter) const = 0;
    // for dragging and dropping between TreeViews, return the active source
    virtual TreeView* get_drag_source() const = 0;

    using Widget::set_sensitive;
    using Widget::get_sensitive;
};

typedef std::tuple<OUString&, const TreeIter&> encoded_image_query;

class VCL_DLLPUBLIC IconView : virtual public Widget
{
    friend class ::LOKTrigger;

private:
    OUString m_sSavedValue;

protected:
    Link<IconView&, void> m_aSelectionChangeHdl;
    Link<IconView&, bool> m_aItemActivatedHdl;
    Link<const CommandEvent&, bool> m_aCommandHdl;
    Link<const TreeIter&, OUString> m_aQueryTooltipHdl;
    Link<const encoded_image_query&, bool> m_aGetPropertyTreeElemHdl;

    void signal_selection_changed() { m_aSelectionChangeHdl.Call(*this); }
    bool signal_item_activated() { return m_aItemActivatedHdl.Call(*this); }
    OUString signal_query_tooltip(const TreeIter& rIter) const
    {
        return m_aQueryTooltipHdl.Call(rIter);
    }

public:
    virtual int get_item_width() const = 0;
    virtual void set_item_width(int width) = 0;

    virtual void insert(int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, TreeIter* pRet)
        = 0;

    virtual void insert(int pos, const OUString* pStr, const OUString* pId,
                        const VirtualDevice* pIcon, TreeIter* pRet)
        = 0;

    virtual void insert_separator(int pos, const OUString* pId) = 0;

    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(-1, &rStr, &rId, &rImage, nullptr);
    }

    void append(const OUString& rId, const OUString& rStr, const VirtualDevice* pImage)
    {
        insert(-1, &rStr, &rId, pImage, nullptr);
    }

    void append_separator(const OUString& rId) { insert_separator(-1, &rId); }

    void connect_selection_changed(const Link<IconView&, void>& rLink)
    {
        m_aSelectionChangeHdl = rLink;
    }

    /* A row is "activated" when the user double clicks a treeview row. It may
       also be emitted when a row is selected and Space or Enter is pressed.

       a return of "true" means the activation has been handled, a "false" propagates
       the activation to the default handler which expands/collapses the row, if possible.
    */
    void connect_item_activated(const Link<IconView&, bool>& rLink) { m_aItemActivatedHdl = rLink; }

    void connect_command(const Link<const CommandEvent&, bool>& rLink) { m_aCommandHdl = rLink; }

    virtual void connect_query_tooltip(const Link<const TreeIter&, OUString>& rLink)
    {
        m_aQueryTooltipHdl = rLink;
    }

    // 0: OUString, 1: TreeIter, returns true if supported
    virtual void connect_get_image(const Link<const encoded_image_query&, bool>& rLink)
    {
        m_aGetPropertyTreeElemHdl = rLink;
    }

    virtual OUString get_selected_id() const = 0;

    virtual void clear() = 0;

    virtual int count_selected_items() const = 0;

    virtual OUString get_selected_text() const = 0;

    //by index. Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    virtual void select(int pos) = 0;
    virtual void unselect(int pos) = 0;

    //via iter
    virtual std::unique_ptr<TreeIter> make_iterator(const TreeIter* pOrig = nullptr) const = 0;
    virtual bool get_selected(TreeIter* pIter) const = 0;
    virtual bool get_cursor(TreeIter* pIter) const = 0;
    virtual void set_cursor(const TreeIter& rIter) = 0;
    virtual bool get_iter_first(TreeIter& rIter) const = 0;
    virtual OUString get_id(const TreeIter& rIter) const = 0;
    virtual OUString get_text(const TreeIter& rIter) const = 0;
    virtual void scroll_to_item(const TreeIter& rIter) = 0;

    // call func on each selected element until func returns true or we run out of elements
    virtual void selected_foreach(const std::function<bool(TreeIter&)>& func) = 0;

    //all of them. Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    void select_all() { unselect(-1); }
    void unselect_all() { select(-1); }

    // return the number of toplevel nodes
    virtual int n_children() const = 0;

    void save_value() { m_sSavedValue = get_selected_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_selected_text(); }
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

    void connect_clicked(const Link<Button&, void>& rLink) { m_aClickHdl = rLink; }
};

class VCL_DLLPUBLIC Toggleable : virtual public Widget
{
    friend class ::LOKTrigger;

protected:
    Link<Toggleable&, void> m_aToggleHdl;
    TriState m_eSavedValue = TRISTATE_FALSE;

    void signal_toggled() { m_aToggleHdl.Call(*this); }

public:
    virtual void set_active(bool active) = 0;
    virtual bool get_active() const = 0;

    virtual void set_inconsistent(bool inconsistent) = 0;
    virtual bool get_inconsistent() const = 0;

    TriState get_state() const
    {
        if (get_inconsistent())
            return TRISTATE_INDET;
        else if (get_active())
            return TRISTATE_TRUE;
        return TRISTATE_FALSE;
    }

    void set_state(TriState eState)
    {
        switch (eState)
        {
            case TRISTATE_INDET:
                set_inconsistent(true);
                break;
            case TRISTATE_TRUE:
                set_inconsistent(false);
                set_active(true);
                break;
            case TRISTATE_FALSE:
                set_inconsistent(false);
                set_active(false);
                break;
        }
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

struct VCL_DLLPUBLIC TriStateEnabled
{
    TriState eState;
    bool bTriStateEnabled;
    TriStateEnabled()
        : eState(TRISTATE_INDET)
        , bTriStateEnabled(true)
    {
    }
    void ButtonToggled(Toggleable& rToggle);
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

// Similar to a MenuButton except it is split into two parts, a toggle
// button at the start and a menubutton at the end
class VCL_DLLPUBLIC MenuToggleButton : virtual public MenuButton
{
};

class VCL_DLLPUBLIC CheckButton : virtual public Toggleable
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_label_wrap(bool wrap) = 0;
};

class VCL_DLLPUBLIC RadioButton : virtual public CheckButton
{
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

    void signal_changed() { m_aChangeHdl.Call(*this); }
    void signal_cursor_position() { m_aCursorPositionHdl.Call(*this); }

public:
    virtual void set_text(const OUString& rText) = 0;
    virtual OUString get_text() const = 0;
    virtual void set_width_chars(int nChars) = 0;
    virtual int get_width_chars() const = 0;
    // The maximum length of the entry. Use 0 for no maximum
    virtual void set_max_length(int nChars) = 0;
    // nEndPos can be -1 in order to select all text
    virtual void select_region(int nStartPos, int nEndPos) = 0;
    // returns true if the selection has nonzero length
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) = 0;
    virtual void replace_selection(const OUString& rText) = 0;
    // nCursorPos can be -1 to set to the end
    virtual void set_position(int nCursorPos) = 0;
    virtual int get_position() const = 0;
    virtual void set_editable(bool bEditable) = 0;
    virtual bool get_editable() const = 0;
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
    Link<SpinButton&, void> m_aOutputHdl;
    Link<int*, bool> m_aInputHdl;

protected:
    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

    bool signal_output()
    {
        if (!m_aOutputHdl.IsSet())
            return false;
        m_aOutputHdl.Call(*this);
        return true;
    }

    TriState signal_input(int* result)
    {
        if (!m_aInputHdl.IsSet())
            return TRISTATE_INDET;
        return m_aInputHdl.Call(result) ? TRISTATE_TRUE : TRISTATE_FALSE;
    }

public:
    virtual void set_value(sal_Int64 value) = 0;
    virtual sal_Int64 get_value() const = 0;
    virtual void set_range(sal_Int64 min, sal_Int64 max) = 0;
    virtual void get_range(sal_Int64& min, sal_Int64& max) const = 0;
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
    virtual void set_increments(int step, int page) = 0;
    virtual void get_increments(int& step, int& page) const = 0;
    virtual void set_digits(unsigned int digits) = 0;
    virtual unsigned int get_digits() const = 0;

    void connect_value_changed(const Link<SpinButton&, void>& rLink) { m_aValueChangedHdl = rLink; }

    void connect_output(const Link<SpinButton&, void>& rLink) { m_aOutputHdl = rLink; }
    void connect_input(const Link<int*, bool>& rLink) { m_aInputHdl = rLink; }

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
    void signal_selected() { m_aSelectedHdl.Call(*this); }
    void signal_activated() { m_aActivatedHdl.Call(*this); }

public:
    void connect_selected(const Link<Calendar&, void>& rLink) { m_aSelectedHdl = rLink; }
    void connect_activated(const Link<Calendar&, void>& rLink) { m_aActivatedHdl = rLink; }

    virtual void set_date(const Date& rDate) = 0;
    virtual Date get_date() const = 0;
};

// an entry + treeview pair, where the entry autocompletes from the
// treeview list, and selecting something in the list sets the
// entry to that text, i.e. a visually exploded ComboBox
class VCL_DLLPUBLIC EntryTreeView : virtual public ComboBox
{
private:
    DECL_DLLPRIVATE_LINK(ClickHdl, weld::TreeView&, void);
    DECL_DLLPRIVATE_LINK(ModifyHdl, weld::Entry&, void);
    void EntryModifyHdl(const weld::Entry& rEntry);

protected:
    std::unique_ptr<Entry> m_xEntry;
    std::unique_ptr<TreeView> m_xTreeView;

public:
    EntryTreeView(std::unique_ptr<Entry> xEntry, std::unique_ptr<TreeView> xTreeView);

    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems,
                               bool bKeepExisting) override
    {
        m_xTreeView->freeze();
        if (!bKeepExisting)
            m_xTreeView->clear();
        for (const auto& rItem : rItems)
            append(rItem);
        m_xTreeView->thaw();
    }

    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override
    {
        m_xTreeView->insert(pos, rStr, pId, pIconName, pImageSurface);
    }

    virtual int get_count() const override { return m_xTreeView->n_children(); }
    virtual void clear() override { m_xTreeView->clear(); }

    //by index
    virtual int get_active() const override { return m_xTreeView->get_selected_index(); }
    virtual void set_active(int pos) override
    {
        m_xTreeView->set_cursor(pos);
        m_xTreeView->select(pos);
        m_xEntry->set_text(m_xTreeView->get_selected_text());
    }
    virtual void remove(int pos) override { m_xTreeView->remove(pos); }

    //by text
    virtual OUString get_active_text() const override { return m_xEntry->get_text(); }
    virtual OUString get_text(int pos) const override { return m_xTreeView->get_text(pos); }
    virtual int find_text(const OUString& rStr) const override
    {
        return m_xTreeView->find_text(rStr);
    }

    //by id
    virtual OUString get_active_id() const override { return m_xTreeView->get_selected_id(); }
    virtual void set_active_id(const OUString& rStr) override
    {
        m_xTreeView->select_id(rStr);
        m_xEntry->set_text(m_xTreeView->get_selected_text());
    }
    virtual OUString get_id(int pos) const override { return m_xTreeView->get_id(pos); }
    virtual void set_id(int pos, const OUString& rId) override { m_xTreeView->set_id(pos, rId); }
    virtual int find_id(const OUString& rId) const override { return m_xTreeView->find_id(rId); }

    //entry related
    virtual bool has_entry() const override { return true; }
    virtual void set_entry_message_type(EntryMessageType eType) override
    {
        m_xEntry->set_message_type(eType);
    }
    virtual void set_entry_text(const OUString& rStr) override { m_xEntry->set_text(rStr); }
    virtual void set_entry_width_chars(int nChars) override { m_xEntry->set_width_chars(nChars); }
    virtual void set_entry_max_length(int nChars) override { m_xEntry->set_max_length(nChars); }
    virtual void select_entry_region(int nStartPos, int nEndPos) override
    {
        m_xEntry->select_region(nStartPos, nEndPos);
    }
    //if no text was selected, both rStartPos and rEndPos will be identical
    //and false will be returned
    virtual bool get_entry_selection_bounds(int& rStartPos, int& rEndPos) override
    {
        return m_xEntry->get_selection_bounds(rStartPos, rEndPos);
    }
    void connect_row_activated(const Link<TreeView&, bool>& rLink)
    {
        m_xTreeView->connect_row_activated(rLink);
    }

    virtual bool get_popup_shown() const override { return false; }

    void set_height_request_by_rows(int nRows);
};

class VCL_DLLPUBLIC MetricSpinButton final
{
    FieldUnit m_eSrcUnit;
    std::unique_ptr<weld::SpinButton> m_xSpinButton;
    Link<MetricSpinButton&, void> m_aValueChangedHdl;

    DECL_LINK(spin_button_value_changed, weld::SpinButton&, void);
    DECL_LINK(spin_button_output, weld::SpinButton&, void);
    DECL_LINK(spin_button_input, int* result, bool);

    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

    sal_Int64 ConvertValue(sal_Int64 nValue, FieldUnit eInUnit, FieldUnit eOutUnit) const;
    OUString format_number(sal_Int64 nValue) const;
    void update_width_chars();

public:
    MetricSpinButton(std::unique_ptr<SpinButton> pSpinButton, FieldUnit eSrcUnit)
        : m_eSrcUnit(eSrcUnit)
        , m_xSpinButton(std::move(pSpinButton))
    {
        update_width_chars();
        m_xSpinButton->connect_output(LINK(this, MetricSpinButton, spin_button_output));
        m_xSpinButton->connect_input(LINK(this, MetricSpinButton, spin_button_input));
        m_xSpinButton->connect_value_changed(
            LINK(this, MetricSpinButton, spin_button_value_changed));
        spin_button_output(*m_xSpinButton);
    }

    static OUString MetricToString(FieldUnit rUnit);

    FieldUnit get_unit() const { return m_eSrcUnit; }

    void set_unit(FieldUnit eUnit);

    sal_Int64 convert_value_to(sal_Int64 nValue, FieldUnit eValueUnit) const
    {
        return ConvertValue(nValue, m_eSrcUnit, eValueUnit);
    }

    sal_Int64 convert_value_from(sal_Int64 nValue, FieldUnit eValueUnit) const
    {
        return ConvertValue(nValue, eValueUnit, m_eSrcUnit);
    }

    void set_value(sal_Int64 nValue, FieldUnit eValueUnit)
    {
        m_xSpinButton->set_value(convert_value_from(nValue, eValueUnit));
    }

    sal_Int64 get_value(FieldUnit eDestUnit) const
    {
        return convert_value_to(m_xSpinButton->get_value(), eDestUnit);
    }

    // typically you only need to call this if set_text (e.g. with "") was
    // previously called to display some arbitrary text instead of the
    // formatted value and now you want to show it as formatted again
    void reformat() { spin_button_output(*m_xSpinButton); }

    void set_range(sal_Int64 min, sal_Int64 max, FieldUnit eValueUnit)
    {
        min = convert_value_from(min, eValueUnit);
        max = convert_value_from(max, eValueUnit);
        m_xSpinButton->set_range(min, max);
        update_width_chars();
    }

    void get_range(sal_Int64& min, sal_Int64& max, FieldUnit eDestUnit) const
    {
        m_xSpinButton->get_range(min, max);
        min = convert_value_to(min, eDestUnit);
        max = convert_value_to(max, eDestUnit);
    }

    void set_min(sal_Int64 min, FieldUnit eValueUnit)
    {
        sal_Int64 dummy, max;
        get_range(dummy, max, eValueUnit);
        set_range(min, max, eValueUnit);
    }

    void set_max(sal_Int64 max, FieldUnit eValueUnit)
    {
        sal_Int64 min, dummy;
        get_range(min, dummy, eValueUnit);
        set_range(min, max, eValueUnit);
    }

    sal_Int64 get_min(FieldUnit eValueUnit) const
    {
        sal_Int64 min, dummy;
        get_range(min, dummy, eValueUnit);
        return min;
    }

    sal_Int64 get_max(FieldUnit eValueUnit) const
    {
        sal_Int64 dummy, max;
        get_range(dummy, max, eValueUnit);
        return max;
    }

    void set_increments(int step, int page, FieldUnit eValueUnit)
    {
        step = convert_value_from(step, eValueUnit);
        page = convert_value_from(page, eValueUnit);
        m_xSpinButton->set_increments(step, page);
    }

    void get_increments(int& step, int& page, FieldUnit eDestUnit) const
    {
        m_xSpinButton->get_increments(step, page);
        step = convert_value_to(step, eDestUnit);
        page = convert_value_to(page, eDestUnit);
    }

    void connect_value_changed(const Link<MetricSpinButton&, void>& rLink)
    {
        m_aValueChangedHdl = rLink;
    }

    sal_Int64 normalize(sal_Int64 nValue) const { return m_xSpinButton->normalize(nValue); }
    sal_Int64 denormalize(sal_Int64 nValue) const { return m_xSpinButton->denormalize(nValue); }
    void set_sensitive(bool sensitive) { m_xSpinButton->set_sensitive(sensitive); }
    bool get_sensitive() const { return m_xSpinButton->get_sensitive(); }
    bool get_visible() const { return m_xSpinButton->get_visible(); }
    void grab_focus() { m_xSpinButton->grab_focus(); }
    bool has_focus() const { return m_xSpinButton->has_focus(); }
    void show() { m_xSpinButton->show(); }
    void set_visible(bool bShow) { m_xSpinButton->set_visible(bShow); }
    void hide() { m_xSpinButton->hide(); }
    void set_digits(unsigned int digits);
    void set_accessible_name(const OUString& rName) { m_xSpinButton->set_accessible_name(rName); }
    unsigned int get_digits() const { return m_xSpinButton->get_digits(); }
    void save_value() { m_xSpinButton->save_value(); }
    bool get_value_changed_from_saved() const
    {
        return m_xSpinButton->get_value_changed_from_saved();
    }
    void set_text(const OUString& rText) { m_xSpinButton->set_text(rText); }
    OUString get_text() const { return m_xSpinButton->get_text(); }
    void set_size_request(int nWidth, int nHeight)
    {
        m_xSpinButton->set_size_request(nWidth, nHeight);
    }
    Size get_size_request() const { return m_xSpinButton->get_size_request(); }
    Size get_preferred_size() const { return m_xSpinButton->get_preferred_size(); }
    void connect_focus_in(const Link<Widget&, void>& rLink)
    {
        m_xSpinButton->connect_focus_in(rLink);
    }
    void connect_focus_out(const Link<Widget&, void>& rLink)
    {
        m_xSpinButton->connect_focus_out(rLink);
    }
    OUString get_buildable_name() const { return m_xSpinButton->get_buildable_name(); }
    void set_help_id(const OUString& rName) { m_xSpinButton->set_help_id(rName); }
    void set_position(int nCursorPos) { m_xSpinButton->set_position(nCursorPos); }
    // set the width of the underlying widget in characters, this setting is
    // invalidated when changing the units, range or digits, so to have effect
    // must come after changing those values
    void set_width_chars(int nChars) { m_xSpinButton->set_width_chars(nChars); }
    int get_width_chars() const { return m_xSpinButton->get_width_chars(); }
    weld::SpinButton& get_widget() { return *m_xSpinButton; }
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

class VCL_DLLPUBLIC TextView : virtual public Widget
{
    friend class ::LOKTrigger;

private:
    OUString m_sSavedValue;

protected:
    Link<TextView&, void> m_aChangeHdl;
    Link<TextView&, void> m_aVChangeHdl;
    Link<TextView&, void> m_aCursorPositionHdl;

    void signal_changed() { m_aChangeHdl.Call(*this); }
    void signal_cursor_position() { m_aCursorPositionHdl.Call(*this); }
    void signal_vadjustment_changed() { m_aVChangeHdl.Call(*this); }

public:
    virtual void set_text(const OUString& rText) = 0;
    virtual OUString get_text() const = 0;
    // if nStartPos or nEndPos is -1 the max available text pos will be used
    virtual void select_region(int nStartPos, int nEndPos) = 0;
    // returns true if the selection has nonzero length
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) = 0;
    virtual void replace_selection(const OUString& rText) = 0;
    virtual void set_editable(bool bEditable) = 0;
    virtual bool get_editable() const = 0;
    virtual void set_monospace(bool bMonospace) = 0;
    // The maximum length of the entry. Use 0 for no maximum
    virtual void set_max_length(int nChars) = 0;
    int get_height_rows(int nRows) const
    {
        //can improve this if needed
        return get_text_height() * nRows;
    }

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;

    /*
       Typically you want to avoid the temptation of customizing
       font colors
    */
    virtual void set_font_color(const Color& rColor) = 0;

    void save_value() { m_sSavedValue = get_text(); }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_text(); }

    void connect_changed(const Link<TextView&, void>& rLink) { m_aChangeHdl = rLink; }
    virtual void connect_cursor_position(const Link<TextView&, void>& rLink)
    {
        m_aCursorPositionHdl = rLink;
    }

    // returns true if pressing up would move the cursor
    // doesn't matter if that move is to a previous line or to the start of the
    // current line just so long as the cursor would move
    virtual bool can_move_cursor_with_up() const = 0;

    // returns true if pressing down would move the cursor
    // doesn't matter if that move is to a next line or to the end of the
    // current line just so long as the cursor would move
    virtual bool can_move_cursor_with_down() const = 0;

    virtual void cut_clipboard() = 0;
    virtual void copy_clipboard() = 0;
    virtual void paste_clipboard() = 0;

    virtual void set_alignment(TxtAlign eXAlign) = 0;

    virtual int vadjustment_get_value() const = 0;
    virtual int vadjustment_get_upper() const = 0;
    virtual int vadjustment_get_lower() const = 0;
    virtual int vadjustment_get_page_size() const = 0;
    virtual void vadjustment_set_value(int value) = 0;
    void connect_vadjustment_changed(const Link<TextView&, void>& rLink) { m_aVChangeHdl = rLink; }
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
    Link<const CommandEvent&, bool> m_aCommandHdl;
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
    void connect_command(const Link<const CommandEvent&, bool>& rLink) { m_aCommandHdl = rLink; }
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

    virtual a11yref get_accessible_parent() = 0;
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
    Link<Scrollbar&, void> m_aChangeHdl;

protected:
    void signal_adjustment_changed() { m_aChangeHdl.Call(*this); }

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
    virtual void adjustment_set_lower(int upper) = 0;

    virtual int get_scroll_thickness() const = 0;
    virtual void set_scroll_thickness(int nThickness) = 0;
    virtual void set_scroll_swap_arrows(bool bSwap) = 0;

    virtual ScrollType get_scroll_type() const = 0;

    void connect_adjustment_changed(const Link<Scrollbar&, void>& rLink) { m_aChangeHdl = rLink; }
};

class VCL_DLLPUBLIC SizeGroup
{
public:
    virtual void add_widget(weld::Widget* pWidget) = 0;
    // the default mode is VclSizeGroupMode::Horizontal
    virtual void set_mode(VclSizeGroupMode eMode) = 0;
    virtual ~SizeGroup() {}
};

class VCL_DLLPUBLIC Builder
{
public:
    virtual std::unique_ptr<MessageDialog> weld_message_dialog(const OUString& id) = 0;
    virtual std::unique_ptr<Dialog> weld_dialog(const OUString& id) = 0;
    virtual std::unique_ptr<Assistant> weld_assistant(const OUString& id) = 0;
    virtual std::unique_ptr<Widget> weld_widget(const OUString& id) = 0;
    virtual std::unique_ptr<Container> weld_container(const OUString& id) = 0;
    virtual std::unique_ptr<Box> weld_box(const OUString& id) = 0;
    virtual std::unique_ptr<Paned> weld_paned(const OUString& id) = 0;
    virtual std::unique_ptr<Button> weld_button(const OUString& id) = 0;
    virtual std::unique_ptr<MenuButton> weld_menu_button(const OUString& id) = 0;
    virtual std::unique_ptr<MenuToggleButton> weld_menu_toggle_button(const OUString& id) = 0;
    virtual std::unique_ptr<Frame> weld_frame(const OUString& id) = 0;
    /* bUserManagedScrolling of true means that the automatic scrolling of the window is disabled
       and the owner must specifically listen to adjustment changes and react appropriately to them.
    */
    virtual std::unique_ptr<ScrolledWindow> weld_scrolled_window(const OUString& id,
                                                                 bool bUserManagedScrolling = false)
        = 0;
    virtual std::unique_ptr<Notebook> weld_notebook(const OUString& id) = 0;
    virtual std::unique_ptr<ToggleButton> weld_toggle_button(const OUString& id) = 0;
    virtual std::unique_ptr<RadioButton> weld_radio_button(const OUString& id) = 0;
    virtual std::unique_ptr<CheckButton> weld_check_button(const OUString& id) = 0;
    virtual std::unique_ptr<LinkButton> weld_link_button(const OUString& id) = 0;
    virtual std::unique_ptr<SpinButton> weld_spin_button(const OUString& id) = 0;
    virtual std::unique_ptr<MetricSpinButton> weld_metric_spin_button(const OUString& id,
                                                                      FieldUnit eUnit)
        = 0;
    virtual std::unique_ptr<FormattedSpinButton> weld_formatted_spin_button(const OUString& id) = 0;
    virtual std::unique_ptr<ComboBox> weld_combo_box(const OUString& id) = 0;
    virtual std::unique_ptr<TreeView> weld_tree_view(const OUString& id) = 0;
    virtual std::unique_ptr<IconView> weld_icon_view(const OUString& id) = 0;
    virtual std::unique_ptr<Label> weld_label(const OUString& id) = 0;
    virtual std::unique_ptr<TextView> weld_text_view(const OUString& id) = 0;
    virtual std::unique_ptr<Expander> weld_expander(const OUString& id) = 0;
    virtual std::unique_ptr<Entry> weld_entry(const OUString& id) = 0;
    virtual std::unique_ptr<Scale> weld_scale(const OUString& id) = 0;
    virtual std::unique_ptr<ProgressBar> weld_progress_bar(const OUString& id) = 0;
    virtual std::unique_ptr<LevelBar> weld_level_bar(const OUString& id) = 0;
    virtual std::unique_ptr<Spinner> weld_spinner(const OUString& id) = 0;
    virtual std::unique_ptr<Image> weld_image(const OUString& id) = 0;
    virtual std::unique_ptr<Calendar> weld_calendar(const OUString& id) = 0;
    virtual std::unique_ptr<DrawingArea>
    weld_drawing_area(const OUString& id, const a11yref& rA11yImpl = nullptr,
                      FactoryFunction pUITestFactoryFunction = nullptr, void* pUserData = nullptr)
        = 0;
    virtual std::unique_ptr<EntryTreeView> weld_entry_tree_view(const OUString& containerid,
                                                                const OUString& entryid,
                                                                const OUString& treeviewid)
        = 0;
    virtual std::unique_ptr<Menu> weld_menu(const OUString& id) = 0;
    virtual std::unique_ptr<Popover> weld_popover(const OUString& id) = 0;
    virtual std::unique_ptr<Toolbar> weld_toolbar(const OUString& id) = 0;
    virtual std::unique_ptr<Scrollbar> weld_scrollbar(const OUString& id) = 0;
    virtual std::unique_ptr<SizeGroup> create_size_group() = 0;
    /* return a Dialog suitable to take a screenshot of containing the contents of the .ui file.

       If the toplevel element is a dialog, that will be returned
       If the toplevel is not a dialog, a dialog will be created and the contents of the .ui
       inserted into it
    */
    virtual std::unique_ptr<Window> create_screenshot_window() = 0;
    virtual ~Builder() {}
};

class VCL_DLLPUBLIC DialogController : public std::enable_shared_from_this<DialogController>
{
public:
    virtual Dialog* getDialog() = 0;
    const Dialog* getConstDialog() const
    {
        return const_cast<DialogController*>(this)->getDialog();
    }
    virtual short run() { return getDialog()->run(); }
    static bool runAsync(const std::shared_ptr<DialogController>& rController,
                         const std::function<void(sal_Int32)>&);
    void set_title(const OUString& rTitle) { getDialog()->set_title(rTitle); }
    OUString get_title() const { return getConstDialog()->get_title(); }
    void set_help_id(const OUString& rHelpId) { getDialog()->set_help_id(rHelpId); }
    OUString get_help_id() const { return getConstDialog()->get_help_id(); }
    void response(int nResponse) { getDialog()->response(nResponse); }
    virtual ~DialogController() COVERITY_NOEXCEPT_FALSE;
};

class VCL_DLLPUBLIC GenericDialogController : public DialogController
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::shared_ptr<weld::Dialog> m_xDialog;

public:
    GenericDialogController(weld::Widget* pParent, const OUString& rUIFile,
                            const OUString& rDialogId, bool bMobile = false);
    virtual Dialog* getDialog() override;
    virtual ~GenericDialogController() COVERITY_NOEXCEPT_FALSE override;
};

class VCL_DLLPUBLIC MessageDialogController : public DialogController
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::MessageDialog> m_xDialog;
    std::unique_ptr<weld::Container> m_xContentArea;
    std::unique_ptr<weld::Widget> m_xRelocate;
    std::unique_ptr<weld::Container> m_xOrigParent;

public:
    /* @param rRelocateId - optional argument of the name of a widget in the .ui file
                            which should be relocated into the content area of the dialog.

                            e.g. a checkbox for a "Never show this again" option.

                            This results in the named widget relocating to the same container
                            as the messages.  This enables aligning the extra widget with the
                            message labels in the content area container which doesn't
                            explicitly exist in the ui description, but is only implied.
    */
    MessageDialogController(weld::Widget* pParent, const OUString& rUIFile,
                            const OUString& rDialogId, const OUString& rRelocateId = {});
    virtual Dialog* getDialog() override;
    virtual ~MessageDialogController() override;
    void set_primary_text(const OUString& rText) { m_xDialog->set_primary_text(rText); }
    OUString get_primary_text() const { return m_xDialog->get_primary_text(); }
    void set_secondary_text(const OUString& rText) { m_xDialog->set_secondary_text(rText); }
    OUString get_secondary_text() const { return m_xDialog->get_secondary_text(); }
    void set_default_response(int nResponse) { m_xDialog->set_default_response(nResponse); }
};

class VCL_DLLPUBLIC AssistantController : public DialogController
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Assistant> m_xAssistant;

public:
    SAL_DLLPRIVATE AssistantController(weld::Widget* pParent, const OUString& rUIFile,
                                       const OUString& rDialogId);
    virtual Dialog* getDialog() override;
    SAL_DLLPRIVATE virtual ~AssistantController() override;
};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
