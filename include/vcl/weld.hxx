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
#include <tools/time.hxx>
#include <vcl/dllapi.h>
#include <vcl/vclenum.hxx>
#include <vcl/font.hxx>
#include <vcl/menu.hxx>
#include <vcl/uitest/factory.hxx>

#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>

#include <assert.h>
#include <memory>
#include <vector>

namespace com
{
namespace sun
{
namespace star
{
namespace awt
{
class XWindow;
}
namespace datatransfer
{
namespace dnd
{
class XDropTarget;
}
}
namespace graphic
{
class XGraphic;
}
}
}
}

typedef css::uno::Reference<css::accessibility::XAccessible> a11yref;
typedef css::uno::Reference<css::accessibility::XAccessibleRelationSet> a11yrelationset;

enum class PointerStyle;
class SvNumberFormatter;
class KeyEvent;
class MouseEvent;
class OutputDevice;
class VirtualDevice;
struct SystemEnvData;

namespace vcl
{
class ILibreOfficeKitNotifier;
}

namespace weld
{
class Container;
class DialogController;

class VCL_DLLPUBLIC Widget
{
protected:
    Link<Widget&, void> m_aFocusInHdl;
    Link<Widget&, void> m_aFocusOutHdl;
    Link<Widget&, bool> m_aMnemonicActivateHdl;
    Link<const Size&, void> m_aSizeAllocateHdl;
    Link<const KeyEvent&, bool> m_aKeyPressHdl;
    Link<const KeyEvent&, bool> m_aKeyReleaseHdl;
    Link<const MouseEvent&, bool> m_aMousePressHdl;
    Link<const MouseEvent&, bool> m_aMouseMotionHdl;
    Link<const MouseEvent&, bool> m_aMouseReleaseHdl;

public:
    virtual void set_sensitive(bool sensitive) = 0;
    virtual bool get_sensitive() const = 0;
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
    virtual bool get_visible() const = 0; //if this widget visibility is true
    virtual bool is_visible() const = 0; //if this widget visibility and all parents is true
    virtual void set_can_focus(bool bCanFocus) = 0;
    virtual void grab_focus() = 0;
    virtual bool has_focus() const = 0;
    virtual bool is_active() const = 0; //if this widget has the focus within the active window
    virtual void set_has_default(bool has_default) = 0;
    virtual bool get_has_default() const = 0;
    virtual void set_size_request(int nWidth, int nHeight) = 0;
    virtual Size get_size_request() const = 0;
    virtual Size get_preferred_size() const = 0;
    virtual float get_approximate_digit_width() const = 0;
    virtual int get_text_height() const = 0;
    virtual Size get_pixel_size(const OUString& rText) const = 0;
    virtual OString get_buildable_name() const = 0;
    virtual void set_help_id(const OString& rName) = 0;
    virtual OString get_help_id() const = 0;

    virtual void set_grid_left_attach(int nAttach) = 0;
    virtual int get_grid_left_attach() const = 0;
    virtual void set_grid_width(int nCols) = 0;
    virtual void set_grid_top_attach(int nAttach) = 0;
    virtual int get_grid_top_attach() const = 0;

    virtual void set_hexpand(bool bExpand) = 0;
    virtual bool get_hexpand() const = 0;
    virtual void set_vexpand(bool bExpand) = 0;
    virtual bool get_vexpand() const = 0;

    virtual void set_secondary(bool bSecondary) = 0;

    virtual void set_margin_top(int nMargin) = 0;
    virtual void set_margin_bottom(int nMargin) = 0;
    virtual void set_margin_left(int nMargin) = 0;
    virtual void set_margin_right(int nMargin) = 0;

    virtual int get_margin_top() const = 0;
    virtual int get_margin_bottom() const = 0;
    virtual int get_margin_left() const = 0;
    virtual int get_margin_right() const = 0;

    virtual bool get_extents_relative_to(Widget& rRelative, int& x, int& y, int& width, int& height)
        = 0;

    virtual void set_accessible_name(const OUString& rName) = 0;
    virtual OUString get_accessible_name() const = 0;

    virtual OUString get_accessible_description() const = 0;

    virtual void set_accessible_relation_labeled_by(weld::Widget* pLabel) = 0;
    virtual void set_accessible_relation_label_for(weld::Widget* pLabeled) = 0;

    virtual void
    add_extra_accessible_relation(const css::accessibility::AccessibleRelation& rRelation)
        = 0;
    virtual void clear_extra_accessible_relations() = 0;

    virtual void set_tooltip_text(const OUString& rTip) = 0;
    virtual OUString get_tooltip_text() const = 0;

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

    virtual void grab_add() = 0;
    virtual bool has_grab() const = 0;
    virtual void grab_remove() = 0;

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual vcl::Font get_font() = 0;

    //true for rtl, false otherwise
    virtual bool get_direction() const = 0;
    virtual void set_direction(bool bRTL) = 0;

    virtual void freeze() = 0;
    virtual void thaw() = 0;

    virtual std::unique_ptr<Container> weld_parent() const = 0;

    //iterate upwards through the hierarchy starting at this widgets parent,
    //calling func with their helpid until func returns true or we run out of
    //parents
    virtual void help_hierarchy_foreach(const std::function<bool(const OString&)>& func) = 0;

    virtual OUString strip_mnemonic(const OUString& rLabel) const = 0;

    virtual VclPtr<VirtualDevice> create_virtual_device() const = 0;

    //make this widget look like a page in a notebook
    virtual void set_stack_background() = 0;
    //make this widget look like it has a highlighted background
    virtual void set_highlight_background() = 0;

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() = 0;

    virtual ~Widget() {}
};

class VCL_DLLPUBLIC Container : virtual public Widget
{
public:
    // remove and add in one go
    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) = 0;
    // recursively unset has-default on any buttons in the widget hierarchy
    virtual void recursively_unset_default_buttons() = 0;
    // create an XWindow as a child of this container. The XWindow is
    // suitable to contain css::awt::XControl items
    virtual css::uno::Reference<css::awt::XWindow> CreateChildFrame() = 0;
};

class VCL_DLLPUBLIC Box : virtual public Container
{
public:
    // Moves child to a new position in the list of children
    virtual void reorder_child(weld::Widget* pWidget, int position) = 0;
};

class VCL_DLLPUBLIC ScrolledWindow : virtual public Container
{
protected:
    Link<ScrolledWindow&, void> m_aVChangeHdl;
    Link<ScrolledWindow&, void> m_aHChangeHdl;

    void signal_vadjustment_changed() { m_aVChangeHdl.Call(*this); }
    void signal_hadjustment_changed() { m_aHChangeHdl.Call(*this); }

public:
    virtual void set_user_managed_scrolling() = 0;

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
    virtual int get_hscroll_height() const = 0;

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
    virtual int get_vscroll_width() const = 0;
};

class Label;

class VCL_DLLPUBLIC Frame : virtual public Container
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual std::unique_ptr<Label> weld_label_widget() const = 0;
};

class VCL_DLLPUBLIC Notebook : virtual public Container
{
protected:
    Link<const OString&, bool> m_aLeavePageHdl;
    Link<const OString&, void> m_aEnterPageHdl;

public:
    virtual int get_current_page() const = 0;
    virtual OString get_page_ident(int nPage) const = 0;
    virtual OString get_current_page_ident() const = 0;
    virtual void set_current_page(int nPage) = 0;
    virtual void set_current_page(const OString& rIdent) = 0;
    virtual void remove_page(const OString& rIdent) = 0;
    virtual void append_page(const OString& rIdent, const OUString& rLabel) = 0;
    virtual void set_tab_label_text(const OString& rIdent, const OUString& rLabel) = 0;
    virtual OUString get_tab_label_text(const OString& rIdent) const = 0;
    virtual int get_n_pages() const = 0;
    virtual weld::Container* get_page(const OString& rIdent) const = 0;

    void connect_leave_page(const Link<const OString&, bool>& rLink) { m_aLeavePageHdl = rLink; }

    void connect_enter_page(const Link<const OString&, void>& rLink) { m_aEnterPageHdl = rLink; }
};

class VCL_DLLPUBLIC ScreenShotEntry
{
public:
    ScreenShotEntry(const OString& rHelpId, const basegfx::B2IRange& rB2IRange)
        : msHelpId(rHelpId)
        , maB2IRange(rB2IRange)
    {
    }

    const basegfx::B2IRange& getB2IRange() const { return maB2IRange; }

    const OString& GetHelpId() const { return msHelpId; }

private:
    OString msHelpId;
    basegfx::B2IRange maB2IRange;
};

typedef std::vector<ScreenShotEntry> ScreenShotCollection;

class VCL_DLLPUBLIC Window : virtual public Container
{
protected:
    Link<Widget&, bool> m_aHelpRequestHdl;
    Link<Widget&, void> m_aTopLevelFocusChangedHdl;

    void signal_toplevel_focus_changed() { m_aTopLevelFocusChangedHdl.Call(*this); }

public:
    virtual void set_title(const OUString& rTitle) = 0;
    virtual OUString get_title() const = 0;
    virtual void set_busy_cursor(bool bBusy) = 0;
    virtual void window_move(int x, int y) = 0;
    virtual void set_modal(bool bModal) = 0;
    virtual bool get_modal() const = 0;
    virtual bool get_resizable() const = 0;
    virtual Size get_size() const = 0;
    virtual Point get_position() const = 0;
    virtual tools::Rectangle get_monitor_workarea() const = 0;
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
    virtual bool has_toplevel_focus() const = 0;
    virtual void present() = 0;
    virtual void set_window_state(const OString& rStr) = 0;
    virtual OString get_window_state(WindowStateMask nMask) const = 0;

    virtual css::uno::Reference<css::awt::XWindow> GetXWindow() = 0;

    void connect_help(const Link<Widget&, bool>& rLink) { m_aHelpRequestHdl = rLink; }
    virtual void connect_toplevel_focus_changed(const Link<Widget&, void>& rLink)
    {
        m_aTopLevelFocusChangedHdl = rLink;
    }

    virtual SystemEnvData get_system_data() const = 0;

    virtual void resize_to_request() = 0;

    // render the dialog for a screenshot
    virtual void draw(VirtualDevice& rOutput) = 0;
    // collect positions of widgets and their help ids for screenshot purposes
    virtual ScreenShotCollection collect_screenshot_data() = 0;
};

class VCL_DLLPUBLIC WaitObject
{
private:
    weld::Window* m_pWindow;

public:
    WaitObject(weld::Window* pWindow)
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
    virtual bool runAsync(std::shared_ptr<DialogController>,
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
    virtual void add_button(const OUString& rText, int response, const OString& rHelpId = OString())
        = 0;
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

class VCL_DLLPUBLIC AboutDialog : virtual public Dialog
{
public:
    virtual void set_version(const OUString& rVersion) = 0;
    virtual void set_copyright(const OUString& rCopyright) = 0;
    virtual void set_website(const OUString& rURL) = 0;
    virtual void set_website_label(const OUString& rLabel) = 0;
    virtual OUString get_website_label() const = 0;
    virtual void set_logo(const css::uno::Reference<css::graphic::XGraphic>& rImage) = 0;
    virtual void set_background(const css::uno::Reference<css::graphic::XGraphic>& rImage) = 0;
};

class VCL_DLLPUBLIC Assistant : virtual public Dialog
{
protected:
    Link<const OString&, bool> m_aJumpPageHdl;

    bool signal_jump_page(const OString& rIdent) { return m_aJumpPageHdl.Call(rIdent); }

public:
    virtual int get_current_page() const = 0;
    virtual int get_n_pages() const = 0;
    virtual OString get_page_ident(int nPage) const = 0;
    virtual OString get_current_page_ident() const = 0;
    virtual void set_current_page(int nPage) = 0;
    virtual void set_current_page(const OString& rIdent) = 0;
    // move the page rIdent to position nIndex
    virtual void set_page_index(const OString& rIdent, int nIndex) = 0;
    virtual void set_page_title(const OString& rIdent, const OUString& rTitle) = 0;
    virtual OUString get_page_title(const OString& rIdent) const = 0;
    virtual void set_page_sensitive(const OString& rIdent, bool bSensitive) = 0;
    virtual weld::Container* append_page(const OString& rIdent) = 0;

    virtual void set_page_side_help_id(const OString& rHelpId) = 0;

    void connect_jump_page(const Link<const OString&, bool>& rLink) { m_aJumpPageHdl = rLink; }
};

struct VCL_DLLPUBLIC ComboBoxEntry
{
    OUString sString;
    OUString sId;
    OUString sImage;
    ComboBoxEntry(const OUString& rString)
        : sString(rString)
    {
    }
    ComboBoxEntry(const OUString& rString, const OUString& rId)
        : sString(rString)
        , sId(rId)
    {
    }
    ComboBoxEntry(const OUString& rString, const OUString& rId, const OUString& rImage)
        : sString(rString)
        , sId(rId)
        , sImage(rImage)
    {
    }
};

enum class EntryMessageType
{
    Normal,
    Warning,
    Error,
};

/// A widget used to choose from a list of items.
class VCL_DLLPUBLIC ComboBox : virtual public Container
{
private:
    OUString m_sSavedValue;

protected:
    Link<ComboBox&, void> m_aChangeHdl;
    Link<ComboBox&, void> m_aPopupToggledHdl;
    Link<ComboBox&, bool> m_aEntryActivateHdl;
    Link<OUString&, bool> m_aEntryInsertTextHdl;

public:
    void signal_changed() { m_aChangeHdl.Call(*this); }
    virtual void signal_popup_toggled() { m_aPopupToggledHdl.Call(*this); }

    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface)
        = 0;
    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems, bool bKeepExisting)
        = 0;
    void insert_text(int pos, const OUString& rStr)
    {
        insert(pos, rStr, nullptr, nullptr, nullptr);
    }
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

    void connect_changed(const Link<ComboBox&, void>& rLink) { m_aChangeHdl = rLink; }
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

    virtual bool get_popup_shown() const = 0;

    void connect_entry_insert_text(const Link<OUString&, bool>& rLink)
    {
        m_aEntryInsertTextHdl = rLink;
    }

    // callback returns true to indicated no further processing of activate wanted
    void connect_entry_activate(const Link<ComboBox&, bool>& rLink) { m_aEntryActivateHdl = rLink; }

    void save_value() { m_sSavedValue = get_active_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_active_text(); }
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

class VCL_DLLPUBLIC TreeView : virtual public Container
{
private:
    OUString m_sSavedValue;

protected:
    Link<TreeView&, void> m_aChangeHdl;
    Link<TreeView&, bool> m_aRowActivatedHdl;
    Link<int, void> m_aColumnClickedHdl;
    Link<const std::pair<int, int>&, void> m_aRadioToggleHdl;
    Link<const TreeIter&, bool> m_aEditingStartedHdl;
    Link<const std::pair<const TreeIter&, OUString>&, bool> m_aEditingDoneHdl;
    // if handler returns false, the expansion of the row is refused
    Link<const TreeIter&, bool> m_aExpandingHdl;
    Link<TreeView&, void> m_aVisibleRangeChangedHdl;
    Link<TreeView&, void> m_aModelChangedHdl;
    Link<const CommandEvent&, bool> m_aPopupMenuHdl;
    std::function<int(const weld::TreeIter&, const weld::TreeIter&)> m_aCustomSort;

    std::vector<int> m_aRadioIndexes;

    void signal_changed() { m_aChangeHdl.Call(*this); }
    bool signal_row_activated() { return m_aRowActivatedHdl.Call(*this); }
    void signal_column_clicked(int nColumn) { m_aColumnClickedHdl.Call(nColumn); }
    bool signal_expanding(const TreeIter& rIter)
    {
        return !m_aExpandingHdl.IsSet() || m_aExpandingHdl.Call(rIter);
    }
    void signal_visible_range_changed() { m_aVisibleRangeChangedHdl.Call(*this); }
    void signal_model_changed() { m_aModelChangedHdl.Call(*this); }

    // arg is pair<row,col>
    void signal_toggled(const std::pair<int, int>& rRowCol) { m_aRadioToggleHdl.Call(rRowCol); }

    bool signal_editing_started(const TreeIter& rIter) { return m_aEditingStartedHdl.Call(rIter); }

    bool signal_editing_done(const std::pair<const TreeIter&, OUString>& rIterText)
    {
        return m_aEditingDoneHdl.Call(rIterText);
    }

public:
    virtual void insert(const TreeIter* pParent, int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        const OUString* pExpanderName, bool bChildrenOnDemand, TreeIter* pRet)
        = 0;

    void insert(int nRow, TreeIter* pRet = nullptr)
    {
        insert(nullptr, nRow, nullptr, nullptr, nullptr, nullptr, nullptr, false, pRet);
    }

    void append(TreeIter* pRet = nullptr) { insert(-1, pRet); }

    void insert(int pos, const OUString& rStr, const OUString* pId, const OUString* pIconName,
                VirtualDevice* pImageSurface)
    {
        insert(nullptr, pos, &rStr, pId, pIconName, pImageSurface, nullptr, false, nullptr);
    }
    void insert_text(int pos, const OUString& rStr)
    {
        insert(nullptr, pos, &rStr, nullptr, nullptr, nullptr, nullptr, false, nullptr);
    }
    void append_text(const OUString& rStr)
    {
        insert(nullptr, -1, &rStr, nullptr, nullptr, nullptr, nullptr, false, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr)
    {
        insert(nullptr, -1, &rStr, &rId, nullptr, nullptr, nullptr, false, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(nullptr, -1, &rStr, &rId, &rImage, nullptr, nullptr, false, nullptr);
    }
    void append(const TreeIter* pParent, const OUString& rId, const OUString& rStr,
                const OUString& rImage)
    {
        insert(pParent, -1, &rStr, &rId, &rImage, nullptr, nullptr, false, nullptr);
    }
    void append(const TreeIter* pParent, const OUString& rStr)
    {
        insert(pParent, -1, &rStr, nullptr, nullptr, nullptr, nullptr, false, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr, VirtualDevice& rImage)
    {
        insert(nullptr, -1, &rStr, &rId, nullptr, &rImage, nullptr, false, nullptr);
    }

    void connect_changed(const Link<TreeView&, void>& rLink) { m_aChangeHdl = rLink; }

    /* A row is "activated" when the user double clicks a treeview row. It may
       also be emitted when a row is selected and Space or Enter is pressed.

       a return of "true" means the activation has been handled, a "false" propagates
       the activation to the default handler which expands/collapses the row, if possible.
    */
    void connect_row_activated(const Link<TreeView&, bool>& rLink) { m_aRowActivatedHdl = rLink; }

    // Argument is a pair of row, col describing the node in non-tree mode.
    // If in tree mode, then retrieve the toggled node with get_cursor
    void connect_toggled(const Link<const std::pair<int, int>&, void>& rLink)
    {
        m_aRadioToggleHdl = rLink;
    }

    void connect_column_clicked(const Link<int, void>& rLink) { m_aColumnClickedHdl = rLink; }
    void connect_model_changed(const Link<TreeView&, void>& rLink) { m_aModelChangedHdl = rLink; }

    virtual OUString get_selected_text() const = 0;
    virtual OUString get_selected_id() const = 0;

    //by index
    virtual int get_selected_index() const = 0;
    virtual void select(int pos) = 0;
    virtual void unselect(int pos) = 0;
    virtual void remove(int pos) = 0;
    virtual OUString get_text(int row, int col = -1) const = 0;
    virtual void set_text(int row, const OUString& rText, int col = -1) = 0;
    virtual void set_sensitive(int row, bool bSensitive, int col = -1) = 0;
    virtual void set_id(int row, const OUString& rId) = 0;
    virtual void set_toggle(int row, TriState eState, int col) = 0;
    virtual TriState get_toggle(int row, int col) const = 0;
    virtual void set_image(int row, const OUString& rImage, int col = -1) = 0;
    virtual void set_image(int row, VirtualDevice& rImage, int col = -1) = 0;
    virtual void set_image(int row, const css::uno::Reference<css::graphic::XGraphic>& rImage,
                           int col = -1)
        = 0;
    virtual void set_text_emphasis(int row, bool bOn, int col) = 0;
    virtual bool get_text_emphasis(int row, int col) const = 0;
    virtual void swap(int pos1, int pos2) = 0;
    virtual std::vector<int> get_selected_rows() const = 0;
    virtual void set_font_color(int pos, const Color& rColor) const = 0;
    virtual void scroll_to_row(int pos) = 0;
    virtual bool is_selected(int pos) const = 0;
    virtual int get_cursor_index() const = 0;
    virtual void set_cursor(int pos) = 0;

    //by text
    virtual int find_text(const OUString& rText) const = 0;
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
    // set iter to point to next node, depth first, then sibling
    virtual bool iter_next(TreeIter& rIter) const = 0;
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
    virtual void remove(const TreeIter& rIter) = 0;
    virtual void select(const TreeIter& rIter) = 0;
    virtual void unselect(const TreeIter& rIter) = 0;
    virtual bool get_row_expanded(const TreeIter& rIter) const = 0;
    virtual void expand_row(const TreeIter& rIter) = 0;
    virtual void collapse_row(const TreeIter& rIter) = 0;
    virtual void set_text(const TreeIter& rIter, const OUString& rStr, int col = -1) = 0;
    virtual void set_sensitive(const TreeIter& rIter, bool bSensitive, int col = -1) = 0;
    virtual void set_text_emphasis(const TreeIter& rIter, bool bOn, int col) = 0;
    virtual bool get_text_emphasis(const TreeIter& rIter, int col) const = 0;
    virtual void set_toggle(const TreeIter& rIter, TriState bOn, int col) = 0;
    virtual TriState get_toggle(const TreeIter& rIter, int col) const = 0;
    virtual OUString get_text(const TreeIter& rIter, int col = -1) const = 0;
    virtual void set_id(const TreeIter& rIter, const OUString& rId) = 0;
    virtual OUString get_id(const TreeIter& rIter) const = 0;
    virtual void set_image(const TreeIter& rIter, const OUString& rImage, int col = -1) = 0;
    virtual void set_image(const TreeIter& rIter, VirtualDevice& rImage, int col = -1) = 0;
    virtual void set_image(const TreeIter& rIter,
                           const css::uno::Reference<css::graphic::XGraphic>& rImage, int col = -1)
        = 0;
    virtual void set_font_color(const TreeIter& rIter, const Color& rColor) const = 0;
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
    // clear the tree, then add nSourceCount rows, call func on each row
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
                                      const std::vector<int>* pFixedWidths = nullptr)
        = 0;

    void connect_expanding(const Link<const TreeIter&, bool>& rLink) { m_aExpandingHdl = rLink; }

    // rStartLink returns true to allow editing, false to disallow
    // rEndLink returns true to accept the edit, false to reject
    virtual void
    connect_editing(const Link<const TreeIter&, bool>& rStartLink,
                    const Link<const std::pair<const TreeIter&, OUString>&, bool>& rEndLink)
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

    //all of them
    void select_all() { unselect(-1); }
    void unselect_all() { select(-1); }

    // return the number of toplevel nodes
    virtual int n_children() const = 0;

    virtual void make_sorted() = 0;
    virtual void make_unsorted() = 0;
    virtual bool get_sort_order() const = 0;
    virtual void set_sort_order(bool bAscending) = 0;

    // TRUE ascending, FALSE, descending, INDET, neither (off)
    virtual void set_sort_indicator(TriState eState, int nColumn = -1) = 0;
    virtual TriState get_sort_indicator(int nColumn = -1) const = 0;

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
    virtual int get_column_width(int nCol) const = 0;
    virtual void set_centered_column(int nCol) = 0;
    virtual OUString get_column_title(int nColumn) const = 0;
    virtual void set_column_title(int nColumn, const OUString& rTitle) = 0;

    int get_checkbox_column_width() const { return get_approximate_digit_width() * 3 + 6; }

    virtual void set_selection_mode(SelectionMode eMode) = 0;
    virtual int count_selected_rows() const = 0;
    // remove the selected nodes
    virtual void remove_selection() = 0;

    void set_toggle_columns_as_radio(const std::vector<int>& rCols) { m_aRadioIndexes = rCols; }

    void save_value() { m_sSavedValue = get_selected_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_selected_text(); }

    // for dnd
    virtual bool get_dest_row_at_pos(const Point& rPos, weld::TreeIter* pResult) = 0;
    // for dragging and dropping between TreeViews, return the active source
    virtual TreeView* get_drag_source() const = 0;

    using Widget::set_sensitive;
};

class VCL_DLLPUBLIC IconView : virtual public Container
{
private:
    OUString m_sSavedValue;

protected:
    Link<IconView&, void> m_aSelectionChangeHdl;
    Link<IconView&, bool> m_aItemActivatedHdl;

    void signal_selection_changed() { m_aSelectionChangeHdl.Call(*this); }
    bool signal_item_activated() { return m_aItemActivatedHdl.Call(*this); }

public:
    virtual void insert(int pos, const OUString* pStr, const OUString* pId,
                        const OUString* pIconName, TreeIter* pRet)
        = 0;

    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(-1, &rStr, &rId, &rImage, nullptr);
    }

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

    virtual OUString get_selected_id() const = 0;

    virtual void clear() = 0;

    virtual int count_selected_items() const = 0;

    virtual OUString get_selected_text() const = 0;

    //by index
    virtual void select(int pos) = 0;
    virtual void unselect(int pos) = 0;

    //via iter
    virtual std::unique_ptr<TreeIter> make_iterator(const TreeIter* pOrig = nullptr) const = 0;
    virtual bool get_selected(TreeIter* pIter) const = 0;
    virtual bool get_cursor(TreeIter* pIter) const = 0;
    virtual void set_cursor(const TreeIter& rIter) = 0;
    virtual bool get_iter_first(TreeIter& rIter) const = 0;
    virtual OUString get_id(const TreeIter& rIter) const = 0;
    virtual void scroll_to_item(const TreeIter& rIter) = 0;

    // call func on each selected element until func returns true or we run out of elements
    virtual void selected_foreach(const std::function<bool(TreeIter&)>& func) = 0;

    //all of them
    void select_all() { unselect(-1); }
    void unselect_all() { select(-1); }

    // return the number of toplevel nodes
    virtual int n_children() const = 0;

    void save_value() { m_sSavedValue = get_selected_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_selected_text(); }
};

class VCL_DLLPUBLIC Button : virtual public Container
{
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
    virtual void set_label_line_wrap(bool wrap) = 0;
    void clicked() { signal_clicked(); }

    void connect_clicked(const Link<Button&, void>& rLink) { m_aClickHdl = rLink; }
};

class VCL_DLLPUBLIC ToggleButton : virtual public Button
{
protected:
    Link<ToggleButton&, void> m_aToggleHdl;
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

    virtual void connect_toggled(const Link<ToggleButton&, void>& rLink) { m_aToggleHdl = rLink; }
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
    void ButtonToggled(ToggleButton& rToggle);
};

class VCL_DLLPUBLIC MenuButton : virtual public ToggleButton
{
protected:
    Link<const OString&, void> m_aSelectHdl;

    void signal_selected(const OString& rIdent) { m_aSelectHdl.Call(rIdent); }

public:
    void connect_selected(const Link<const OString&, void>& rLink) { m_aSelectHdl = rLink; }

    virtual void insert_item(int pos, const OUString& rId, const OUString& rStr,
                             const OUString* pIconName, VirtualDevice* pImageSufface, bool bCheck)
        = 0;
    void append_item(const OUString& rId, const OUString& rStr)
    {
        insert_item(-1, rId, rStr, nullptr, nullptr, false);
    }
    void append_item_check(const OUString& rId, const OUString& rStr)
    {
        insert_item(-1, rId, rStr, nullptr, nullptr, true);
    }
    void append_item(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert_item(-1, rId, rStr, &rImage, nullptr, false);
    }
    void append_item(const OUString& rId, const OUString& rStr, VirtualDevice& rImage)
    {
        insert_item(-1, rId, rStr, nullptr, &rImage, false);
    }
    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }
    virtual void remove_item(const OString& rId) = 0;
    virtual void clear() = 0;
    virtual void set_item_sensitive(const OString& rIdent, bool bSensitive) = 0;
    virtual void set_item_active(const OString& rIdent, bool bActive) = 0;
    virtual void set_item_label(const OString& rIdent, const OUString& rLabel) = 0;
    virtual OUString get_item_label(const OString& rIdent) const = 0;
    virtual void set_item_help_id(const OString& rIdent, const OString& rHelpId) = 0;
    virtual void set_item_visible(const OString& rIdent, bool bVisible) = 0;
    virtual OString get_item_help_id(const OString& rIdent) const = 0;

    virtual void set_popover(weld::Widget* pPopover) = 0;
};

class VCL_DLLPUBLIC CheckButton : virtual public ToggleButton
{
};

class VCL_DLLPUBLIC RadioButton : virtual public ToggleButton
{
};

class VCL_DLLPUBLIC LinkButton : virtual public Container
{
protected:
    Link<LinkButton&, bool> m_aActivateLinkHdl;

    bool signal_activate_link() { return m_aActivateLinkHdl.Call(*this); }

public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_uri(const OUString& rUri) = 0;
    virtual OUString get_uri() const = 0;

    void connect_activate_link(const Link<LinkButton&, bool>& rLink) { m_aActivateLinkHdl = rLink; }
};

class VCL_DLLPUBLIC Scale : virtual public Widget
{
protected:
    Link<Scale&, void> m_aValueChangedHdl;

    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

public:
    virtual void set_value(int value) = 0;
    virtual int get_value() const = 0;
    virtual void set_range(int min, int max) = 0;
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

class VCL_DLLPUBLIC Entry : virtual public Widget
{
private:
    OUString m_sSavedValue;

protected:
    Link<Entry&, void> m_aChangeHdl;
    Link<OUString&, bool> m_aInsertTextHdl;
    Link<Entry&, void> m_aCursorPositionHdl;
    Link<Entry&, bool> m_aActivateHdl;

    void signal_changed() { m_aChangeHdl.Call(*this); }
    void signal_cursor_position() { m_aCursorPositionHdl.Call(*this); }
    void signal_insert_text(OUString& rString);

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

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;

    void connect_changed(const Link<Entry&, void>& rLink) { m_aChangeHdl = rLink; }
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

    void save_value() { m_sSavedValue = get_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_text(); }
};

class VCL_DLLPUBLIC SpinButton : virtual public Entry
{
protected:
    Link<SpinButton&, void> m_aValueChangedHdl;
    Link<SpinButton&, void> m_aOutputHdl;
    Link<int*, bool> m_aInputHdl;

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
    virtual void set_value(int value) = 0;
    virtual int get_value() const = 0;
    virtual void set_range(int min, int max) = 0;
    virtual void get_range(int& min, int& max) const = 0;
    void set_min(int min)
    {
        int dummy, max;
        get_range(dummy, max);
        set_range(min, max);
    }
    void set_max(int max)
    {
        int min, dummy;
        get_range(min, dummy);
        set_range(min, max);
    }
    int get_min() const
    {
        int min, dummy;
        get_range(min, dummy);
        return min;
    }
    int get_max() const
    {
        int dummy, max;
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

    int normalize(int nValue) const { return (nValue * Power10(get_digits())); }

    int denormalize(int nValue) const;

    static unsigned int Power10(unsigned int n);
};

class VCL_DLLPUBLIC FormattedSpinButton : virtual public Entry
{
protected:
    Link<FormattedSpinButton&, void> m_aValueChangedHdl;

    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

public:
    virtual void set_value(double value) = 0;
    virtual double get_value() const = 0;
    virtual void set_range(double min, double max) = 0;
    virtual void get_range(double& min, double& max) const = 0;

    void set_max(double max)
    {
        double min, dummy;
        get_range(min, dummy);
        set_range(min, max);
    }

    virtual void set_formatter(SvNumberFormatter* pFormatter) = 0;
    virtual sal_Int32 get_format_key() const = 0;
    virtual void set_format_key(sal_Int32 nFormatKey) = 0;

    void connect_value_changed(const Link<FormattedSpinButton&, void>& rLink)
    {
        m_aValueChangedHdl = rLink;
    }
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
protected:
    Link<Calendar&, void> m_aSelectedHdl;
    Link<Calendar&, void> m_aActivatedHdl;

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
        {
            m_xTreeView->insert(-1, rItem.sString, rItem.sId.isEmpty() ? nullptr : &rItem.sId,
                                rItem.sImage.isEmpty() ? nullptr : &rItem.sImage, nullptr);
        }
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
    //if not text was selected, both rStartPos and rEndPos will be identical
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

    int ConvertValue(int nValue, FieldUnit eInUnit, FieldUnit eOutUnit) const;
    OUString format_number(int nValue) const;
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

    int convert_value_to(int nValue, FieldUnit eValueUnit) const
    {
        return ConvertValue(nValue, m_eSrcUnit, eValueUnit);
    }

    int convert_value_from(int nValue, FieldUnit eValueUnit) const
    {
        return ConvertValue(nValue, eValueUnit, m_eSrcUnit);
    }

    void set_value(int nValue, FieldUnit eValueUnit)
    {
        m_xSpinButton->set_value(convert_value_from(nValue, eValueUnit));
    }

    int get_value(FieldUnit eDestUnit) const
    {
        return convert_value_to(m_xSpinButton->get_value(), eDestUnit);
    }

    // typically you only need to call this if set_text (e.g. with "") was
    // previously called to display some arbitrary text instead of the
    // formatted value and now you want to show it as formatted again
    void reformat() { spin_button_output(*m_xSpinButton); }

    void set_range(int min, int max, FieldUnit eValueUnit)
    {
        min = convert_value_from(min, eValueUnit);
        max = convert_value_from(max, eValueUnit);
        m_xSpinButton->set_range(min, max);
        update_width_chars();
    }

    void get_range(int& min, int& max, FieldUnit eDestUnit) const
    {
        m_xSpinButton->get_range(min, max);
        min = convert_value_to(min, eDestUnit);
        max = convert_value_to(max, eDestUnit);
    }

    void set_min(int min, FieldUnit eValueUnit)
    {
        int dummy, max;
        get_range(dummy, max, eValueUnit);
        set_range(min, max, eValueUnit);
    }

    void set_max(int max, FieldUnit eValueUnit)
    {
        int min, dummy;
        get_range(min, dummy, eValueUnit);
        set_range(min, max, eValueUnit);
    }

    int get_min(FieldUnit eValueUnit) const
    {
        int min, dummy;
        get_range(min, dummy, eValueUnit);
        return min;
    }

    int get_max(FieldUnit eValueUnit) const
    {
        int dummy, max;
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

    int normalize(int nValue) const { return m_xSpinButton->normalize(nValue); }
    int denormalize(int nValue) const { return m_xSpinButton->denormalize(nValue); }
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
    void set_help_id(const OString& rName) { m_xSpinButton->set_help_id(rName); }
    void set_position(int nCursorPos) { m_xSpinButton->set_position(nCursorPos); }
    weld::SpinButton& get_widget() { return *m_xSpinButton; }
};

class VCL_DLLPUBLIC TimeSpinButton final
{
    TimeFieldFormat const m_eFormat;
    std::unique_ptr<weld::SpinButton> m_xSpinButton;
    Link<TimeSpinButton&, void> m_aValueChangedHdl;

    DECL_LINK(spin_button_value_changed, weld::SpinButton&, void);
    DECL_LINK(spin_button_output, weld::SpinButton&, void);
    DECL_LINK(spin_button_input, int* result, bool);
    DECL_LINK(spin_button_cursor_position, weld::Entry&, void);

    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

    static tools::Time ConvertValue(int nValue);
    static int ConvertValue(const tools::Time& rTime);
    OUString format_number(int nValue) const;
    void update_width_chars();

public:
    TimeSpinButton(std::unique_ptr<SpinButton> pSpinButton, TimeFieldFormat eFormat)
        : m_eFormat(eFormat)
        , m_xSpinButton(std::move(pSpinButton))
    {
        update_width_chars();
        m_xSpinButton->connect_output(LINK(this, TimeSpinButton, spin_button_output));
        m_xSpinButton->connect_input(LINK(this, TimeSpinButton, spin_button_input));
        m_xSpinButton->connect_value_changed(LINK(this, TimeSpinButton, spin_button_value_changed));
        m_xSpinButton->connect_cursor_position(
            LINK(this, TimeSpinButton, spin_button_cursor_position));
        spin_button_output(*m_xSpinButton);
    }

    void set_value(const tools::Time& rTime) { m_xSpinButton->set_value(ConvertValue(rTime)); }

    tools::Time get_value() const { return ConvertValue(m_xSpinButton->get_value()); }

    void connect_value_changed(const Link<TimeSpinButton&, void>& rLink)
    {
        m_aValueChangedHdl = rLink;
    }

    void set_sensitive(bool sensitive) { m_xSpinButton->set_sensitive(sensitive); }
    bool get_sensitive() const { return m_xSpinButton->get_sensitive(); }
    void set_visible(bool bShow) { m_xSpinButton->set_visible(bShow); }
    bool get_visible() const { return m_xSpinButton->get_visible(); }
    void grab_focus() { m_xSpinButton->grab_focus(); }
    bool has_focus() const { return m_xSpinButton->has_focus(); }
    void show() { m_xSpinButton->show(); }
    void hide() { m_xSpinButton->hide(); }
    void save_value() { m_xSpinButton->save_value(); }
    bool get_value_changed_from_saved() const
    {
        return m_xSpinButton->get_value_changed_from_saved();
    }
    void set_position(int nCursorPos) { m_xSpinButton->set_position(nCursorPos); }
    void set_text(const OUString& rText) { m_xSpinButton->set_text(rText); }
    OUString get_text() const { return m_xSpinButton->get_text(); }
    weld::SpinButton& get_widget() { return *m_xSpinButton; }
};

class VCL_DLLPUBLIC Label : virtual public Widget
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void set_mnemonic_widget(Widget* pTarget) = 0;
    virtual void set_message_type(EntryMessageType eType) = 0;
    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;
};

class VCL_DLLPUBLIC TextView : virtual public Container
{
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
    virtual void select_region(int nStartPos, int nEndPos) = 0;
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) = 0;
    virtual void replace_selection(const OUString& rText) = 0;
    virtual void set_editable(bool bEditable) = 0;
    virtual void set_monospace(bool bMonospace) = 0;
    int get_height_rows(int nRows) const
    {
        //can improve this if needed
        return get_text_height() * nRows;
    }

    void save_value() { m_sSavedValue = get_text(); }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_text(); }

    void connect_changed(const Link<TextView&, void>& rLink) { m_aChangeHdl = rLink; }
    virtual void connect_cursor_position(const Link<TextView&, void>& rLink)
    {
        m_aCursorPositionHdl = rLink;
    }

    virtual int vadjustment_get_value() const = 0;
    virtual int vadjustment_get_upper() const = 0;
    virtual int vadjustment_get_lower() const = 0;
    virtual int vadjustment_get_page_size() const = 0;
    virtual void vadjustment_set_value(int value) = 0;
    void connect_vadjustment_changed(const Link<TextView&, void>& rLink) { m_aVChangeHdl = rLink; }
};

class VCL_DLLPUBLIC Expander : virtual public Container
{
protected:
    Link<Expander&, void> m_aExpandedHdl;

    void signal_expanded() { m_aExpandedHdl.Call(*this); }

public:
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
    Link<Widget&, void> m_aStyleUpdatedHdl;
    Link<const CommandEvent&, bool> m_aCommandHdl;
    Link<Widget&, tools::Rectangle> m_aGetFocusRectHdl;
    Link<tools::Rectangle&, OUString> m_aQueryTooltipHdl;

    OUString signal_query_tooltip(tools::Rectangle& rHelpArea)
    {
        return m_aQueryTooltipHdl.Call(rHelpArea);
    }

public:
    void connect_draw(const Link<draw_args, void>& rLink) { m_aDrawHdl = rLink; }
    void connect_style_updated(const Link<Widget&, void>& rLink) { m_aStyleUpdatedHdl = rLink; }
    void connect_command(const Link<const CommandEvent&, bool>& rLink) { m_aCommandHdl = rLink; }
    void connect_focus_rect(const Link<Widget&, tools::Rectangle>& rLink)
    {
        m_aGetFocusRectHdl = rLink;
    }
    void connect_query_tooltip(const Link<tools::Rectangle&, OUString>& rLink)
    {
        m_aQueryTooltipHdl = rLink;
    }
    virtual void queue_draw() = 0;
    virtual void queue_draw_area(int x, int y, int width, int height) = 0;
    virtual void queue_resize() = 0;

    virtual void set_cursor(PointerStyle ePointerStyle) = 0;

    // use return here just to generate matching VirtualDevices
    virtual OutputDevice& get_ref_device() = 0;

    virtual a11yref get_accessible_parent() = 0;
    virtual a11yrelationset get_accessible_relation_set() = 0;
    virtual Point get_accessible_location() = 0;
};

class VCL_DLLPUBLIC Menu
{
protected:
    Link<const OString&, void> m_aActivateHdl;

    void signal_activate(const OString& rIdent) { m_aActivateHdl.Call(rIdent); }

public:
    virtual OString popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect) = 0;

    void connect_activate(const Link<const OString&, void>& rLink) { m_aActivateHdl = rLink; }

    virtual void set_sensitive(const OString& rIdent, bool bSensitive) = 0;
    virtual void set_label(const OString& rIdent, const OUString& rLabel) = 0;
    virtual void set_active(const OString& rIdent, bool bActive) = 0;
    virtual bool get_active(const OString& rIdent) const = 0;
    virtual void set_visible(const OString& rIdent, bool bVisible) = 0;

    virtual void insert(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSufface, bool bCheck)
        = 0;

    virtual void clear() = 0;

    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }

    void append(const OUString& rId, const OUString& rStr)
    {
        insert(-1, rId, rStr, nullptr, nullptr, false);
    }
    void append_check(const OUString& rId, const OUString& rStr)
    {
        insert(-1, rId, rStr, nullptr, nullptr, true);
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(-1, rId, rStr, &rImage, nullptr, false);
    }
    void append(const OUString& rId, const OUString& rStr, VirtualDevice& rImage)
    {
        insert(-1, rId, rStr, nullptr, &rImage, false);
    }

    virtual ~Menu() {}
};

class VCL_DLLPUBLIC Toolbar : virtual public Widget
{
protected:
    Link<const OString&, void> m_aClickHdl;

    void signal_clicked(const OString& rIdent) { m_aClickHdl.Call(rIdent); }

public:
    virtual void set_item_sensitive(const OString& rIdent, bool bSensitive) = 0;
    virtual bool get_item_sensitive(const OString& rIdent) const = 0;
    virtual void set_item_active(const OString& rIdent, bool bActive) = 0;
    virtual bool get_item_active(const OString& rIdent) const = 0;
    virtual void set_item_menu(const OString& rIdent, weld::Menu* pMenu) = 0;
    virtual void set_item_popover(const OString& rIdent, weld::Widget* pPopover) = 0;

    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }

    virtual int get_n_items() const = 0;
    virtual OString get_item_ident(int nIndex) const = 0;
    virtual void set_item_label(int nIndex, const OUString& rLabel) = 0;
    virtual void set_item_icon(int nIndex, const css::uno::Reference<css::graphic::XGraphic>& rIcon)
        = 0;
    virtual void set_item_tooltip_text(int nIndex, const OUString& rTip) = 0;

    virtual vcl::ImageType get_icon_size() const = 0;

    void connect_clicked(const Link<const OString&, void>& rLink) { m_aClickHdl = rLink; }
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
private:
    OString m_sHelpRoot;

public:
    Builder(const OUString& rUIFile)
        : m_sHelpRoot(OUStringToOString(rUIFile, RTL_TEXTENCODING_UTF8))
    {
        sal_Int32 nIdx = m_sHelpRoot.lastIndexOf('.');
        if (nIdx != -1)
            m_sHelpRoot = m_sHelpRoot.copy(0, nIdx);
        m_sHelpRoot += OString('/');
    }
    virtual std::unique_ptr<MessageDialog> weld_message_dialog(const OString& id,
                                                               bool bTakeOwnership = true)
        = 0;
    virtual std::unique_ptr<Dialog> weld_dialog(const OString& id, bool bTakeOwnership = true) = 0;
    virtual std::unique_ptr<AboutDialog> weld_about_dialog(const OString& id,
                                                           bool bTakeOwnership = true)
        = 0;
    virtual std::unique_ptr<Assistant> weld_assistant(const OString& id, bool bTakeOwnership = true)
        = 0;
    virtual std::unique_ptr<Window> weld_window(const OString& id, bool bTakeOwnership = true) = 0;
    virtual std::unique_ptr<Widget> weld_widget(const OString& id, bool bTakeOwnership = false) = 0;
    virtual std::unique_ptr<Container> weld_container(const OString& id,
                                                      bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<Box> weld_box(const OString& id, bool bTakeOwnership = false) = 0;
    virtual std::unique_ptr<Button> weld_button(const OString& id, bool bTakeOwnership = false) = 0;
    virtual std::unique_ptr<MenuButton> weld_menu_button(const OString& id,
                                                         bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<Frame> weld_frame(const OString& id, bool bTakeOwnership = false) = 0;
    virtual std::unique_ptr<ScrolledWindow> weld_scrolled_window(const OString& id,
                                                                 bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<Notebook> weld_notebook(const OString& id, bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<ToggleButton> weld_toggle_button(const OString& id,
                                                             bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<RadioButton> weld_radio_button(const OString& id,
                                                           bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<CheckButton> weld_check_button(const OString& id,
                                                           bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<LinkButton> weld_link_button(const OString& id,
                                                         bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<SpinButton> weld_spin_button(const OString& id,
                                                         bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<MetricSpinButton>
    weld_metric_spin_button(const OString& id, FieldUnit eUnit, bool bTakeOwnership = false) = 0;
    virtual std::unique_ptr<FormattedSpinButton>
    weld_formatted_spin_button(const OString& id, bool bTakeOwnership = false) = 0;
    virtual std::unique_ptr<TimeSpinButton>
    weld_time_spin_button(const OString& id, TimeFieldFormat eFormat, bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<ComboBox> weld_combo_box(const OString& id, bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<TreeView> weld_tree_view(const OString& id, bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<IconView> weld_icon_view(const OString& id, bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<Label> weld_label(const OString& id, bool bTakeOwnership = false) = 0;
    virtual std::unique_ptr<TextView> weld_text_view(const OString& id, bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<Expander> weld_expander(const OString& id, bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<Entry> weld_entry(const OString& id, bool bTakeOwnership = false) = 0;
    virtual std::unique_ptr<Scale> weld_scale(const OString& id, bool bTakeOwnership = false) = 0;
    virtual std::unique_ptr<ProgressBar> weld_progress_bar(const OString& id,
                                                           bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<Spinner> weld_spinner(const OString& id, bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<Image> weld_image(const OString& id, bool bTakeOwnership = false) = 0;
    virtual std::unique_ptr<Calendar> weld_calendar(const OString& id, bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<DrawingArea>
    weld_drawing_area(const OString& id, const a11yref& rA11yImpl = nullptr,
                      FactoryFunction pUITestFactoryFunction = nullptr, void* pUserData = nullptr,
                      bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<EntryTreeView>
    weld_entry_tree_view(const OString& containerid, const OString& entryid,
                         const OString& treeviewid, bool bTakeOwnership = false)
        = 0;
    virtual std::unique_ptr<Menu> weld_menu(const OString& id, bool bTakeOwnership = true) = 0;
    virtual std::unique_ptr<Toolbar> weld_toolbar(const OString& id, bool bTakeOwnership = true)
        = 0;
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
    void set_help_id(const OString& rHelpId) { getDialog()->set_help_id(rHelpId); }
    OString get_help_id() const { return getConstDialog()->get_help_id(); }
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
                            const OString& rDialogId, bool bMobile = false);
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
                            const OString& rDialogId, const OString& rRelocateId = OString());
    virtual Dialog* getDialog() override;
    virtual ~MessageDialogController() override;
    void set_primary_text(const OUString& rText) { m_xDialog->set_primary_text(rText); }
    OUString get_primary_text() const { return m_xDialog->get_primary_text(); }
    void set_default_response(int nResponse) { m_xDialog->set_default_response(nResponse); }
};

class VCL_DLLPUBLIC AssistantController : public DialogController
{
protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Assistant> m_xAssistant;

public:
    AssistantController(weld::Widget* pParent, const OUString& rUIFile, const OString& rDialogId);
    virtual Dialog* getDialog() override;
    virtual ~AssistantController() override;
};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
