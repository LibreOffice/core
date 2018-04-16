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

#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <vcl/field.hxx>
#include <vcl/virdev.hxx>

typedef css::uno::Reference<css::accessibility::XAccessible> a11yref;

namespace weld
{
class Container;
class DialogController;

class VCL_DLLPUBLIC Widget
{
protected:
    Link<Widget&, void> m_aFocusInHdl;
    Link<Widget&, void> m_aFocusOutHdl;

    void signal_focus_in() { return m_aFocusInHdl.Call(*this); }
    void signal_focus_out() { return m_aFocusOutHdl.Call(*this); }

public:
    virtual void set_sensitive(bool sensitive) = 0;
    virtual bool get_sensitive() const = 0;
    virtual void set_visible(bool visible) = 0;
    virtual bool get_visible() const = 0;
    virtual void grab_focus() = 0;
    virtual bool has_focus() const = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    void show(bool bShow)
    {
        if (bShow)
            show();
        else
            hide();
    }
    virtual void set_size_request(int nWidth, int nHeight) = 0;
    virtual Size get_preferred_size() const = 0;
    virtual float get_approximate_digit_width() const = 0;
    virtual int get_text_height() const = 0;
    virtual Size get_pixel_size(const OUString& rText) const = 0;
    virtual OString get_buildable_name() const = 0;
    virtual void set_help_id(const OString& rName) = 0;
    virtual OString get_help_id() const = 0;

    virtual void set_grid_left_attach(int nAttach) = 0;
    virtual int get_grid_left_attach() const = 0;
    virtual void set_grid_top_attach(int nAttach) = 0;
    virtual int get_grid_top_attach() const = 0;

    virtual void set_margin_top(int nMargin) = 0;
    virtual void set_margin_bottom(int nMargin) = 0;

    virtual void set_accessible_name(const OUString& rName) = 0;
    virtual OUString get_accessible_name() const = 0;

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) = 0;
    virtual void connect_focus_out(const Link<Widget&, void>& rLink) = 0;

    virtual void grab_add() = 0;
    virtual void grab_remove() = 0;

    virtual Container* weld_parent() const = 0;

    virtual ~Widget() {}
};

class VCL_DLLPUBLIC Container : virtual public Widget
{
public:
    virtual void remove(weld::Widget* pWidget) = 0;
    virtual void add(weld::Widget* pWidget) = 0;
};

class VCL_DLLPUBLIC ScrolledWindow : virtual public Container
{
protected:
    Link<ScrolledWindow&, void> m_aVChangeHdl;

    void signal_vadjustment_changed() { m_aVChangeHdl.Call(*this); }

public:
    virtual void set_user_managed_scrolling() = 0;
    virtual void vadjustment_configure(int value, int lower, int upper, int step_increment,
                                       int page_increment, int page_size)
        = 0;
    virtual int vadjustment_get_value() const = 0;
    virtual void vadjustment_set_value(int value) = 0;
    void connect_vadjustment_changed(const Link<ScrolledWindow&, void>& rLink)
    {
        m_aVChangeHdl = rLink;
    }
};

class VCL_DLLPUBLIC Frame : virtual public Container
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
};

class VCL_DLLPUBLIC Notebook : virtual public Container
{
protected:
    Link<const OString&, bool> m_aLeavePageHdl;
    Link<const OString&, void> m_aEnterPageHdl;

public:
    virtual int get_current_page() const = 0;
    virtual OString get_current_page_ident() const = 0;
    virtual void set_current_page(int nPage) = 0;
    virtual void set_current_page(const OString& rIdent) = 0;
    virtual int get_n_pages() const = 0;
    virtual weld::Container* get_page(const OString& rIdent) const = 0;

    void connect_leave_page(const Link<const OString&, bool>& rLink) { m_aLeavePageHdl = rLink; }

    void connect_enter_page(const Link<const OString&, void>& rLink) { m_aEnterPageHdl = rLink; }
};

class VCL_DLLPUBLIC Window : virtual public Container
{
protected:
    Link<Widget&, bool> m_aHelpRequestHdl;

public:
    virtual void set_title(const OUString& rTitle) = 0;
    virtual OUString get_title() const = 0;
    virtual void set_busy_cursor(bool bBusy) = 0;
    virtual void window_move(int x, int y) = 0;
    virtual bool get_extents_relative_to(Window& rRelative, int& x, int& y, int& width, int& height)
        = 0;

    virtual css::uno::Reference<css::awt::XWindow> GetXWindow() = 0;

    void connect_help(const Link<Widget&, bool>& rLink) { m_aHelpRequestHdl = rLink; }

    virtual void resize_to_request() = 0;
};

class VCL_DLLPUBLIC WaitObject
{
private:
    weld::Window* m_pWindow;

public:
    WaitObject(weld::Window* pWindow)
        : m_pWindow(pWindow)
    {
        m_pWindow->set_busy_cursor(true);
    }
    ~WaitObject() { m_pWindow->set_busy_cursor(false); }
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
    virtual void response(int response) = 0;
    virtual void add_button(const OUString& rText, int response, const OString& rHelpId = OString())
        = 0;
    virtual void set_default_response(int response) = 0;
    virtual Button* get_widget_for_response(int response) = 0;
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

class VCL_DLLPUBLIC ComboBoxText : virtual public Container
{
private:
    OUString m_sSavedValue;

protected:
    Link<ComboBoxText&, void> m_aChangeHdl;
    Link<ComboBoxText&, void> m_aEntryActivateHdl;

    void signal_changed() { m_aChangeHdl.Call(*this); }

public:
    virtual int get_active() const = 0;
    virtual void set_active(int pos) = 0;
    virtual OUString get_active_text() const = 0;
    virtual OUString get_active_id() const = 0;
    virtual void set_active_id(const OUString& rStr) = 0;
    virtual OUString get_text(int pos) const = 0;
    virtual OUString get_id(int pos) const = 0;
    virtual void insert_text(int pos, const OUString& rStr) = 0;
    void append_text(const OUString& rStr) { insert_text(-1, rStr); }
    virtual void insert(int pos, const OUString& rId, const OUString& rStr) = 0;
    void append(const OUString& rId, const OUString& rStr) { insert(-1, rId, rStr); }
    using Container::remove;
    virtual void remove(int pos) = 0;
    virtual int find_text(const OUString& rStr) const = 0;
    virtual int find_id(const OUString& rId) const = 0;
    virtual int get_count() const = 0;
    virtual void make_sorted() = 0;
    virtual void clear() = 0;

    virtual void set_entry_error(bool bError) = 0;

    void connect_changed(const Link<ComboBoxText&, void>& rLink) { m_aChangeHdl = rLink; }

    void set_active(const OUString& rStr) { set_active(find_text(rStr)); }

    virtual void set_entry_text(const OUString& rStr) = 0;
    virtual void select_entry_region(int nStartPos, int nEndPos) = 0;
    virtual bool get_entry_selection_bounds(int& rStartPos, int& rEndPos) = 0;
    virtual void set_entry_completion(bool bEnable) = 0;

    void connect_entry_activate(const Link<ComboBoxText&, void>& rLink)
    {
        m_aEntryActivateHdl = rLink;
    }

    void save_value() { m_sSavedValue = get_active_text(); }

    bool get_value_changed_from_saved() const { return m_sSavedValue != get_active_text(); }
};

class VCL_DLLPUBLIC TreeView : virtual public Container
{
protected:
    Link<TreeView&, void> m_aChangeHdl;
    Link<TreeView&, void> m_aRowActivatedHdl;

    void signal_changed() { m_aChangeHdl.Call(*this); }

    void signal_row_activated() { m_aRowActivatedHdl.Call(*this); }

public:
    virtual void insert_text(const OUString& rText, int pos) = 0;
    virtual void append_text(const OUString& rText) { insert_text(rText, -1); }
    virtual void insert(int pos, const OUString& rId, const OUString& rStr, const OUString& rImage)
        = 0;
    virtual void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(-1, rId, rStr, rImage);
    }
    virtual int n_children() const = 0;
    virtual void select(int pos) = 0;
    using Container::remove;
    virtual void remove(int pos) = 0;
    virtual int find(const OUString& rText) const = 0;
    virtual int find_id(const OUString& rId) const = 0;
    virtual void set_top_entry(int pos) = 0;
    virtual void clear() = 0;
    virtual OUString get_selected() const = 0;
    virtual std::vector<OUString> get_selected_rows() const = 0;
    OUString get_selected_id() const { return get_id(get_selected_index()); }
    virtual int get_selected_index() const = 0;
    virtual OUString get(int pos) const = 0;
    virtual OUString get_id(int pos) const = 0;
    virtual int get_height_rows(int nRows) const = 0;

    virtual void freeze() = 0;
    virtual void thaw() = 0;

    virtual void set_selection_mode(bool bMultiple) = 0;
    virtual int count_selected_rows() const = 0;

    void connect_changed(const Link<TreeView&, void>& rLink) { m_aChangeHdl = rLink; }

    void connect_row_activated(const Link<TreeView&, void>& rLink) { m_aRowActivatedHdl = rLink; }

    void select(const OUString& rText) { select(find(rText)); }
    void select_id(const OUString& rId) { select(find_id(rId)); }

    void remove(const OUString& rText) { remove(find(rText)); }
};

class VCL_DLLPUBLIC Button : virtual public Container
{
protected:
    Link<Button&, void> m_aClickHdl;

    void signal_clicked() { m_aClickHdl.Call(*this); }

public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
    virtual void clicked() = 0;

    void connect_clicked(const Link<Button&, void>& rLink) { m_aClickHdl = rLink; }
};

class VCL_DLLPUBLIC MenuButton : virtual public Button
{
protected:
    Link<const OString&, void> m_aSelectHdl;

    void signal_selected(const OString& rIdent) { m_aSelectHdl.Call(rIdent); }

public:
    void connect_selected(const Link<const OString&, void>& rLink) { m_aSelectHdl = rLink; }

    virtual void set_active(const OString& rIdent, bool bActive) = 0;
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

    void save_state() { m_eSavedValue = get_state(); }

    TriState get_saved_state() const { return m_eSavedValue; }

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

    bool get_state_changed_from_saved() const { return m_eSavedValue != get_state(); }

    void connect_toggled(const Link<ToggleButton&, void>& rLink) { m_aToggleHdl = rLink; }
};

class VCL_DLLPUBLIC CheckButton : virtual public ToggleButton
{
};

class VCL_DLLPUBLIC RadioButton : virtual public ToggleButton
{
};

class VCL_DLLPUBLIC Entry : virtual public Widget
{
private:
    OUString m_sSavedValue;

protected:
    Link<Entry&, void> m_aChangeHdl;
    Link<OUString&, bool> m_aInsertTextHdl;

    void signal_changed() { m_aChangeHdl.Call(*this); }

    void signal_insert_text(OUString& rString);

public:
    virtual void set_text(const OUString& rText) = 0;
    virtual OUString get_text() const = 0;
    virtual void set_width_chars(int nChars) = 0;
    virtual void set_max_length(int nChars) = 0;
    virtual void select_region(int nStartPos, int nEndPos) = 0;
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) = 0;
    virtual void set_position(int nCursorPos) = 0;
    virtual void set_editable(bool bEditable) = 0;

    virtual vcl::Font get_font() = 0;
    virtual void set_font(const vcl::Font& rFont) = 0;

    void connect_changed(const Link<Entry&, void>& rLink) { m_aChangeHdl = rLink; }

    void connect_insert_text(const Link<OUString&, bool>& rLink) { m_aInsertTextHdl = rLink; }

    void save_value() { m_sSavedValue = get_text(); }

    bool get_value_changed_from_saved() const { return m_sSavedValue != get_text(); }
};

class VCL_DLLPUBLIC SpinButton : virtual public Entry
{
protected:
    Link<SpinButton&, void> m_aValueChangedHdl;
    Link<SpinButton&, void> m_aOutputHdl;

    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

    bool signal_output()
    {
        if (!m_aOutputHdl.IsSet())
            return false;
        m_aOutputHdl.Call(*this);
        return true;
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
    virtual void set_increments(int step, int page) = 0;
    virtual void get_increments(int& step, int& page) const = 0;
    virtual void set_digits(unsigned int digits) = 0;
    virtual unsigned int get_digits() const = 0;

    void connect_value_changed(const Link<SpinButton&, void>& rLink) { m_aValueChangedHdl = rLink; }

    void connect_output(const Link<SpinButton&, void>& rLink) { m_aOutputHdl = rLink; }

    int normalize(int nValue) const { return (nValue * Power10(get_digits())); }

    int denormalize(int nValue) const;

    static unsigned int Power10(unsigned int n);
};

class VCL_DLLPUBLIC MetricSpinButton
{
protected:
    FieldUnit m_eSrcUnit;
    std::unique_ptr<weld::SpinButton> m_xSpinButton;
    Link<MetricSpinButton&, void> m_aValueChangedHdl;

    DECL_LINK(spin_button_value_changed, weld::SpinButton&, void);
    DECL_LINK(spin_button_output, weld::SpinButton&, void);

    void signal_value_changed() { m_aValueChangedHdl.Call(*this); }

    int ConvertValue(int nValue, FieldUnit eInUnit, FieldUnit eOutUnit) const;
    OUString format_number(int nValue) const;
    void update_width_chars();

public:
    MetricSpinButton(SpinButton* pSpinButton, FieldUnit eSrcUnit)
        : m_eSrcUnit(eSrcUnit)
        , m_xSpinButton(pSpinButton)
    {
        update_width_chars();
        m_xSpinButton->connect_output(LINK(this, MetricSpinButton, spin_button_output));
        m_xSpinButton->connect_value_changed(
            LINK(this, MetricSpinButton, spin_button_value_changed));
    }

    FieldUnit get_unit() const { return m_eSrcUnit; }

    void set_unit(FieldUnit eUnit)
    {
        m_eSrcUnit = eUnit;
        update_width_chars();
    }

    void set_value(int nValue, FieldUnit eValueUnit)
    {
        m_xSpinButton->set_value(ConvertValue(nValue, eValueUnit, m_eSrcUnit));
    }

    int get_value(FieldUnit eDestUnit) const
    {
        int nValue = m_xSpinButton->get_value();
        return ConvertValue(nValue, m_eSrcUnit, eDestUnit);
    }

    void set_range(int min, int max, FieldUnit eValueUnit)
    {
        min = ConvertValue(min, eValueUnit, m_eSrcUnit);
        max = ConvertValue(max, eValueUnit, m_eSrcUnit);
        m_xSpinButton->set_range(min, max);
        update_width_chars();
    }

    void get_range(int& min, int& max, FieldUnit eDestUnit) const
    {
        m_xSpinButton->get_range(min, max);
        min = ConvertValue(min, m_eSrcUnit, eDestUnit);
        max = ConvertValue(max, m_eSrcUnit, eDestUnit);
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

    void set_increments(int step, int page, FieldUnit eValueUnit)
    {
        step = ConvertValue(step, eValueUnit, m_eSrcUnit);
        page = ConvertValue(page, eValueUnit, m_eSrcUnit);
        m_xSpinButton->set_increments(step, page);
    }

    void get_increments(int& step, int& page, FieldUnit eDestUnit) const
    {
        m_xSpinButton->get_increments(step, page);
        step = ConvertValue(step, m_eSrcUnit, eDestUnit);
        page = ConvertValue(page, m_eSrcUnit, eDestUnit);
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
    void show(bool bShow = true) { m_xSpinButton->show(bShow); }
    void hide() { m_xSpinButton->hide(); }
    void set_digits(unsigned int digits) { m_xSpinButton->set_digits(digits); }
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
    const weld::SpinButton* get_widget() const { return m_xSpinButton.get(); }
};

class VCL_DLLPUBLIC Label : virtual public Widget
{
public:
    virtual void set_label(const OUString& rText) = 0;
    virtual OUString get_label() const = 0;
};

class VCL_DLLPUBLIC TextView : virtual public Container
{
public:
    virtual void set_text(const OUString& rText) = 0;
    virtual OUString get_text() const = 0;
    virtual void select_region(int nStartPos, int nEndPos) = 0;
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) = 0;
    virtual void set_editable(bool bEditable) = 0;
    int get_height_rows(int nRows) const
    {
        //can improve this if needed
        return get_text_height() * nRows;
    }
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
    Link<const Size&, void> m_aSizeAllocateHdl;
    Link<const MouseEvent&, void> m_aMousePressHdl;
    Link<const MouseEvent&, void> m_aMouseMotionHdl;
    Link<const MouseEvent&, void> m_aMouseReleaseHdl;
    Link<const KeyEvent&, bool> m_aKeyPressHdl;
    Link<const KeyEvent&, bool> m_aKeyReleaseHdl;

public:
    void connect_draw(const Link<draw_args, void>& rLink) { m_aDrawHdl = rLink; }
    void connect_size_allocate(const Link<const Size&, void>& rLink) { m_aSizeAllocateHdl = rLink; }
    void connect_mouse_press(const Link<const MouseEvent&, void>& rLink)
    {
        m_aMousePressHdl = rLink;
    }
    void connect_mouse_move(const Link<const MouseEvent&, void>& rLink)
    {
        m_aMouseMotionHdl = rLink;
    }
    void connect_mouse_release(const Link<const MouseEvent&, void>& rLink)
    {
        m_aMouseReleaseHdl = rLink;
    }
    void connect_key_press(const Link<const KeyEvent&, bool>& rLink) { m_aKeyPressHdl = rLink; }
    void connect_key_release(const Link<const KeyEvent&, bool>& rLink) { m_aKeyReleaseHdl = rLink; }
    virtual void queue_draw() = 0;
    virtual void queue_draw_area(int x, int y, int width, int height) = 0;
    virtual a11yref get_accessible_parent() = 0;
};

class VCL_DLLPUBLIC Menu
{
public:
    virtual OString popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect) = 0;
    virtual void set_sensitive(const OString& rIdent, bool bSensitive) = 0;
    virtual void set_active(const OString& rIdent, bool bActive) = 0;
    virtual void show(const OString& rIdent, bool bShow) = 0;
    virtual ~Menu() {}
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
        m_sHelpRoot = m_sHelpRoot + OString('/');
    }
    virtual MessageDialog* weld_message_dialog(const OString& id, bool bTakeOwnership = true) = 0;
    virtual Dialog* weld_dialog(const OString& id, bool bTakeOwnership = true) = 0;
    virtual Window* weld_window(const OString& id, bool bTakeOwnership = true) = 0;
    virtual Widget* weld_widget(const OString& id, bool bTakeOwnership = false) = 0;
    virtual Container* weld_container(const OString& id, bool bTakeOwnership = false) = 0;
    virtual Button* weld_button(const OString& id, bool bTakeOwnership = false) = 0;
    virtual MenuButton* weld_menu_button(const OString& id, bool bTakeOwnership = false) = 0;
    virtual Frame* weld_frame(const OString& id, bool bTakeOwnership = false) = 0;
    virtual ScrolledWindow* weld_scrolled_window(const OString& id, bool bTakeOwnership = false)
        = 0;
    virtual Notebook* weld_notebook(const OString& id, bool bTakeOwnership = false) = 0;
    virtual RadioButton* weld_radio_button(const OString& id, bool bTakeOwnership = false) = 0;
    virtual CheckButton* weld_check_button(const OString& id, bool bTakeOwnership = false) = 0;
    virtual SpinButton* weld_spin_button(const OString& id, bool bTakeOwnership = false) = 0;
    MetricSpinButton* weld_metric_spin_button(const OString& id, FieldUnit eUnit,
                                              bool bTakeOwnership = false)
    {
        return new MetricSpinButton(weld_spin_button(id, bTakeOwnership), eUnit);
    }
    virtual ComboBoxText* weld_combo_box_text(const OString& id, bool bTakeOwnership = false) = 0;
    virtual TreeView* weld_tree_view(const OString& id, bool bTakeOwnership = false) = 0;
    virtual Label* weld_label(const OString& id, bool bTakeOwnership = false) = 0;
    virtual TextView* weld_text_view(const OString& id, bool bTakeOwnership = false) = 0;
    virtual Expander* weld_expander(const OString& id, bool bTakeOwnership = false) = 0;
    virtual Entry* weld_entry(const OString& id, bool bTakeOwnership = false) = 0;
    virtual DrawingArea* weld_drawing_area(const OString& id, const a11yref& rA11yImpl = nullptr,
                                           FactoryFunction pUITestFactoryFunction = nullptr,
                                           void* pUserData = nullptr, bool bTakeOwnership = false)
        = 0;
    virtual Menu* weld_menu(const OString& id, bool bTakeOwnership = true) = 0;
    virtual ~Builder() {}
};

class VCL_DLLPUBLIC DialogController
{
private:
    virtual Dialog* getDialog() = 0;
    const Dialog* getConstDialog() const
    {
        return const_cast<DialogController*>(this)->getDialog();
    }

public:
    short run() { return getDialog()->run(); }
    static bool runAsync(const std::shared_ptr<DialogController>& rController,
                         const std::function<void(sal_Int32)>&);
    void set_title(const OUString& rTitle) { getDialog()->set_title(rTitle); }
    OUString get_title() const { return getConstDialog()->get_title(); }
    void set_help_id(const OString& rHelpId) { getDialog()->set_help_id(rHelpId); }
    OString get_help_id() const { return getConstDialog()->get_help_id(); }
    virtual ~DialogController();
};

class VCL_DLLPUBLIC GenericDialogController : public DialogController
{
private:
    virtual Dialog* getDialog() override;

protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::shared_ptr<weld::Dialog> m_xDialog;

public:
    GenericDialogController(weld::Widget* pParent, const OUString& rUIFile,
                            const OString& rDialogId);
    virtual ~GenericDialogController() override;
};

class VCL_DLLPUBLIC MessageDialogController : public DialogController
{
private:
    virtual Dialog* getDialog() override;

protected:
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::MessageDialog> m_xDialog;
    std::unique_ptr<weld::Container> m_xContentArea;
    std::unique_ptr<weld::Widget> m_xRelocate;
    std::unique_ptr<weld::Container> m_xOrigParent;

public:
    MessageDialogController(weld::Widget* pParent, const OUString& rUIFile,
                            const OString& rDialogId, const OString& rRelocateId = OString());
    virtual ~MessageDialogController() override;
    void set_primary_text(const OUString& rText) { m_xDialog->set_primary_text(rText); }
    OUString get_primary_text() const { return m_xDialog->get_primary_text(); }
    void set_default_response(int response) { m_xDialog->set_default_response(response); }
};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
