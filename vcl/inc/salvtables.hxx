/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/toolkit/edit.hxx>
#include <vcl/formatter.hxx>
#include <vcl/toolkit/spinfld.hxx>
#include <vcl/toolkit/fixed.hxx>
#include <vcl/toolkit/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/toolkit/combobox.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/layout.hxx>
#include "messagedialog.hxx"

class SalInstanceBuilder : public weld::Builder
{
protected:
    std::unique_ptr<VclBuilder> m_xBuilder;
    VclPtr<vcl::Window> m_aOwnedToplevel;

public:
    SalInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile,
                       const css::uno::Reference<css::frame::XFrame>& rFrame
                       = css::uno::Reference<css::frame::XFrame>());

    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OString& id) override;

    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString& id) override;

    virtual std::unique_ptr<weld::Assistant> weld_assistant(const OString& id) override;

    virtual std::unique_ptr<weld::Window> create_screenshot_window() override;

    virtual std::unique_ptr<weld::Widget> weld_widget(const OString& id) override;

    virtual std::unique_ptr<weld::Container> weld_container(const OString& id) override;

    virtual std::unique_ptr<weld::Box> weld_box(const OString& id) override;

    virtual std::unique_ptr<weld::Paned> weld_paned(const OString& id) override;

    virtual std::unique_ptr<weld::Frame> weld_frame(const OString& id) override;

    virtual std::unique_ptr<weld::ScrolledWindow> weld_scrolled_window(const OString& id) override;

    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OString& id) override;

    virtual std::unique_ptr<weld::Button> weld_button(const OString& id) override;

    virtual std::unique_ptr<weld::MenuButton> weld_menu_button(const OString& id) override;

    virtual std::unique_ptr<weld::LinkButton> weld_link_button(const OString& id) override;

    virtual std::unique_ptr<weld::ToggleButton> weld_toggle_button(const OString& id) override;

    virtual std::unique_ptr<weld::RadioButton> weld_radio_button(const OString& id) override;

    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OString& id) override;

    virtual std::unique_ptr<weld::Scale> weld_scale(const OString& id) override;

    virtual std::unique_ptr<weld::ProgressBar> weld_progress_bar(const OString& id) override;

    virtual std::unique_ptr<weld::Spinner> weld_spinner(const OString& id) override;

    virtual std::unique_ptr<weld::Image> weld_image(const OString& id) override;

    virtual std::unique_ptr<weld::Calendar> weld_calendar(const OString& id) override;

    virtual std::unique_ptr<weld::Entry> weld_entry(const OString& id) override;

    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OString& id) override;

    virtual std::unique_ptr<weld::MetricSpinButton>
    weld_metric_spin_button(const OString& id, FieldUnit eUnit) override;

    virtual std::unique_ptr<weld::FormattedSpinButton>
    weld_formatted_spin_button(const OString& id) override;

    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OString& id) override;

    virtual std::unique_ptr<weld::EntryTreeView>
    weld_entry_tree_view(const OString& containerid, const OString& entryid,
                         const OString& treeviewid) override;

    virtual std::unique_ptr<weld::TreeView> weld_tree_view(const OString& id) override;

    virtual std::unique_ptr<weld::IconView> weld_icon_view(const OString& id) override;

    virtual std::unique_ptr<weld::Label> weld_label(const OString& id) override;

    virtual std::unique_ptr<weld::TextView> weld_text_view(const OString& id) override;

    virtual std::unique_ptr<weld::Expander> weld_expander(const OString& id) override;

    virtual std::unique_ptr<weld::DrawingArea>
    weld_drawing_area(const OString& id, const a11yref& rA11yImpl = nullptr,
                      FactoryFunction pUITestFactoryFunction = nullptr,
                      void* pUserData = nullptr) override;

    virtual std::unique_ptr<weld::Menu> weld_menu(const OString& id) override;

    virtual std::unique_ptr<weld::Toolbar> weld_toolbar(const OString& id) override;

    virtual std::unique_ptr<weld::SizeGroup> create_size_group() override;

    OString get_current_page_help_id() const;

    virtual ~SalInstanceBuilder() override;
};

class SAL_DLLPUBLIC_RTTI SalInstanceMenu : public weld::Menu
{
private:
    VclPtr<PopupMenu> m_xMenu;

    bool m_bTakeOwnership;
    sal_uInt16 m_nLastId;

    DECL_LINK(SelectMenuHdl, ::Menu*, bool);

public:
    SalInstanceMenu(PopupMenu* pMenu, bool bTakeOwnership);
    virtual OString popup_at_rect(weld::Widget* pParent, const tools::Rectangle& rRect) override;
    virtual void set_sensitive(const OString& rIdent, bool bSensitive) override;
    virtual void set_active(const OString& rIdent, bool bActive) override;
    virtual bool get_active(const OString& rIdent) const override;
    virtual void set_label(const OString& rIdent, const OUString& rLabel) override;
    virtual OUString get_label(const OString& rIdent) const override;
    virtual void set_visible(const OString& rIdent, bool bShow) override;
    virtual void clear() override;
    virtual void insert(int pos, const OUString& rId, const OUString& rStr,
                        const OUString* pIconName, VirtualDevice* pImageSurface,
                        TriState eCheckRadioFalse) override;
    virtual void insert_separator(int pos, const OUString& rId) override;
    virtual void remove(const OString& rId) override;
    virtual int n_children() const override;
    PopupMenu* getMenu() const;
    virtual ~SalInstanceMenu() override;
};

class SalInstanceWidget : public virtual weld::Widget
{
protected:
    VclPtr<vcl::Window> m_xWidget;
    SalInstanceBuilder* m_pBuilder;

private:
    DECL_LINK(EventListener, VclWindowEvent&, void);
    DECL_LINK(KeyEventListener, VclWindowEvent&, bool);
    DECL_LINK(MouseEventListener, VclSimpleEvent&, void);
    DECL_LINK(MnemonicActivateHdl, vcl::Window&, bool);

    const bool m_bTakeOwnership;
    bool m_bEventListener;
    bool m_bKeyEventListener;
    bool m_bMouseEventListener;
    int m_nBlockNotify;

protected:
    void ensure_event_listener();

    // we want the ability to mark key events as handled, so use this variant
    // for those, we get all keystrokes in this case, so we will need to filter
    // them later
    void ensure_key_listener();

    // we want the ability to know about mouse events that happen in our children
    // so use this variant, we will need to filter them later
    void ensure_mouse_listener();

    virtual void HandleEventListener(VclWindowEvent& rEvent);
    virtual bool HandleKeyEventListener(VclWindowEvent& rEvent);
    virtual void HandleMouseEventListener(VclSimpleEvent& rEvent);

public:
    SalInstanceWidget(vcl::Window* pWidget, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_sensitive(bool sensitive) override;

    virtual bool get_sensitive() const override;

    virtual bool get_visible() const override;

    virtual bool is_visible() const override;

    virtual void set_can_focus(bool bCanFocus) override;

    virtual void grab_focus() override;

    virtual bool has_focus() const override;

    virtual bool is_active() const override;

    virtual void set_has_default(bool has_default) override;

    virtual bool get_has_default() const override;

    virtual void show() override;

    virtual void hide() override;

    virtual void set_size_request(int nWidth, int nHeight) override;

    virtual Size get_size_request() const override;

    virtual Size get_preferred_size() const override;

    virtual float get_approximate_digit_width() const override;

    virtual int get_text_height() const override;

    virtual Size get_pixel_size(const OUString& rText) const override;

    virtual vcl::Font get_font() override;

    virtual OString get_buildable_name() const override;

    virtual void set_help_id(const OString& rId) override;

    virtual OString get_help_id() const override;

    virtual void set_grid_left_attach(int nAttach) override;

    virtual int get_grid_left_attach() const override;

    virtual void set_grid_width(int nCols) override;

    virtual void set_grid_top_attach(int nAttach) override;

    virtual int get_grid_top_attach() const override;

    virtual void set_hexpand(bool bExpand) override;

    virtual bool get_hexpand() const override;

    virtual void set_vexpand(bool bExpand) override;

    virtual bool get_vexpand() const override;

    virtual void set_secondary(bool bSecondary) override;

    virtual void set_margin_top(int nMargin) override;

    virtual void set_margin_bottom(int nMargin) override;

    virtual void set_margin_left(int nMargin) override;

    virtual void set_margin_right(int nMargin) override;

    virtual int get_margin_top() const override;

    virtual int get_margin_bottom() const override;

    virtual int get_margin_left() const override;

    virtual int get_margin_right() const override;

    virtual void set_accessible_name(const OUString& rName) override;

    virtual OUString get_accessible_name() const override;

    virtual OUString get_accessible_description() const override;

    virtual void set_accessible_relation_labeled_by(weld::Widget* pLabel) override;

    virtual void set_accessible_relation_label_for(weld::Widget* pLabeled) override;

    virtual void set_tooltip_text(const OUString& rTip) override;

    virtual OUString get_tooltip_text() const override;

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override;

    virtual void connect_mnemonic_activate(const Link<Widget&, bool>& rLink) override;

    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override;

    virtual void connect_size_allocate(const Link<const Size&, void>& rLink) override;

    virtual void connect_mouse_press(const Link<const MouseEvent&, bool>& rLink) override;

    virtual void connect_mouse_move(const Link<const MouseEvent&, bool>& rLink) override;

    virtual void connect_mouse_release(const Link<const MouseEvent&, bool>& rLink) override;

    virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override;

    virtual void connect_key_release(const Link<const KeyEvent&, bool>& rLink) override;

    virtual bool get_extents_relative_to(const Widget& rRelative, int& x, int& y, int& width,
                                         int& height) const override;

    virtual void grab_add() override;

    virtual bool has_grab() const override;

    virtual void grab_remove() override;

    virtual bool get_direction() const override;

    virtual void set_direction(bool bRTL) override;

    virtual void freeze() override;

    virtual void thaw() override;

    virtual std::unique_ptr<weld::Container> weld_parent() const override;

    virtual ~SalInstanceWidget() override;

    vcl::Window* getWidget() const;

    void disable_notify_events();

    bool notify_events_disabled();

    void enable_notify_events();

    virtual void help_hierarchy_foreach(const std::function<bool(const OString&)>& func) override;

    virtual OUString strip_mnemonic(const OUString& rLabel) const override;

    virtual VclPtr<VirtualDevice> create_virtual_device() const override;

    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> get_drop_target() override;

    virtual void connect_get_property_tree(const Link<tools::JsonWriter&, void>& rLink) override;

    virtual void set_stack_background() override;

    virtual void set_title_background() override;

    virtual void set_toolbar_background() override;

    virtual void set_highlight_background() override;

    virtual void set_background(const Color& rColor) override;

    virtual void draw(OutputDevice& rOutput, const tools::Rectangle& rRect) override;

    SystemWindow* getSystemWindow();
};

class SalInstanceLabel : public SalInstanceWidget, public virtual weld::Label
{
private:
    // Control instead of FixedText so we can also use this for
    // SelectableFixedText which is derived from Edit. We just typically need
    // [G|S]etText which exists in their shared baseclass
    VclPtr<Control> m_xLabel;

public:
    SalInstanceLabel(Control* pLabel, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_label(const OUString& rText) override;

    virtual OUString get_label() const override;

    virtual void set_mnemonic_widget(Widget* pTarget) override;

    virtual void set_label_type(weld::LabelType eType) override;

    virtual void set_font(const vcl::Font& rFont) override;

    virtual void set_font_color(const Color& rColor) override;
};

class SalInstanceContainer : public SalInstanceWidget, public virtual weld::Container
{
protected:
    VclPtr<vcl::Window> m_xContainer;

private:
    void implResetDefault(const vcl::Window* _pWindow);

public:
    SalInstanceContainer(vcl::Window* pContainer, SalInstanceBuilder* pBuilder,
                         bool bTakeOwnership);
    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) override;
    virtual void recursively_unset_default_buttons() override;
    virtual css::uno::Reference<css::awt::XWindow> CreateChildFrame() override;
};

class SalInstanceWindow : public SalInstanceContainer, public virtual weld::Window
{
private:
    VclPtr<vcl::Window> m_xWindow;

    DECL_LINK(HelpHdl, vcl::Window&, bool);

    void override_child_help(vcl::Window* pParent);

    void clear_child_help(vcl::Window* pParent);

public:
    SalInstanceWindow(vcl::Window* pWindow, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_title(const OUString& rTitle) override;

    virtual OUString get_title() const override;

    void help();

    virtual void set_busy_cursor(bool bBusy) override;

    virtual css::uno::Reference<css::awt::XWindow> GetXWindow() override;

    virtual void resize_to_request() override;

    virtual void set_modal(bool bModal) override;

    virtual bool get_modal() const override;

    virtual void window_move(int x, int y) override;

    virtual Size get_size() const override;

    virtual Point get_position() const override;

    virtual tools::Rectangle get_monitor_workarea() const override;

    virtual void set_centered_on_parent(bool /*bTrackGeometryRequests*/) override;

    virtual bool get_resizable() const override;

    virtual bool has_toplevel_focus() const override;

    virtual void present() override;

    virtual void set_window_state(const OString& rStr) override;

    virtual OString get_window_state(WindowStateMask nMask) const override;

    virtual SystemEnvData get_system_data() const override;

    virtual void connect_toplevel_focus_changed(const Link<weld::Widget&, void>& rLink) override;

    virtual void HandleEventListener(VclWindowEvent& rEvent) override;

    virtual weld::ScreenShotCollection collect_screenshot_data() override;

    virtual VclPtr<VirtualDevice> screenshot() override;

    virtual ~SalInstanceWindow() override;
};

class SalInstanceDialog : public SalInstanceWindow, public virtual weld::Dialog
{
private:
    VclPtr<::Dialog> m_xDialog;

    // for calc ref dialog that shrink to range selection widgets and resize back
    VclPtr<vcl::Window> m_xRefEdit;
    std::vector<VclPtr<vcl::Window>> m_aHiddenWidgets; // vector of hidden Controls
    long m_nOldEditWidthReq; // Original width request of the input field
    sal_Int32 m_nOldBorderWidth; // border width for expanded dialog

    DECL_LINK(PopupScreenShotMenuHdl, const CommandEvent&, bool);

public:
    SalInstanceDialog(::Dialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual bool runAsync(std::shared_ptr<weld::DialogController> aOwner,
                          const std::function<void(sal_Int32)>& rEndDialogFn) override;

    virtual bool runAsync(std::shared_ptr<Dialog> const& rxSelf,
                          const std::function<void(sal_Int32)>& rEndDialogFn) override;

    virtual void collapse(weld::Widget* pEdit, weld::Widget* pButton) override;

    virtual void undo_collapse() override;

    virtual void
    SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>& rLink) override;

    virtual int run() override;

    virtual void response(int nResponse) override;

    virtual void add_button(const OUString& rText, int nResponse,
                            const OString& rHelpId = OString()) override;

    virtual void set_modal(bool bModal) override;

    virtual bool get_modal() const override;

    virtual weld::Button* weld_widget_for_response(int nResponse) override;

    virtual void set_default_response(int nResponse) override;

    virtual weld::Container* weld_content_area() override;
};

class WeldTextFilter final : public TextFilter
{
private:
    Link<OUString&, bool>& m_rInsertTextHdl;

public:
    WeldTextFilter(Link<OUString&, bool>& rInsertTextHdl);

    virtual OUString filter(const OUString& rText) override;
};

class SalInstanceEntry : public SalInstanceWidget, public virtual weld::Entry
{
private:
    VclPtr<::Edit> m_xEntry;

    DECL_LINK(ChangeHdl, Edit&, void);
    DECL_LINK(CursorListener, VclWindowEvent&, void);
    DECL_LINK(ActivateHdl, Edit&, bool);

    WeldTextFilter m_aTextFilter;

public:
    SalInstanceEntry(::Edit* pEntry, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_text(const OUString& rText) override;

    virtual OUString get_text() const override;

    virtual void set_width_chars(int nChars) override;

    virtual int get_width_chars() const override;

    virtual void set_max_length(int nChars) override;

    virtual void select_region(int nStartPos, int nEndPos) override;

    bool get_selection_bounds(int& rStartPos, int& rEndPos) override;

    virtual void replace_selection(const OUString& rText) override;

    virtual void set_position(int nCursorPos) override;

    virtual int get_position() const override;

    virtual void set_editable(bool bEditable) override;

    virtual bool get_editable() const override;

    virtual void set_overwrite_mode(bool bOn) override;

    virtual bool get_overwrite_mode() const override;

    virtual void set_message_type(weld::EntryMessageType eType) override;

    virtual void set_font(const vcl::Font& rFont) override;

    virtual void set_font_color(const Color& rColor) override;

    virtual void connect_cursor_position(const Link<Entry&, void>& rLink) override;

    virtual void set_placeholder_text(const OUString& rText) override;

    Edit& getEntry();

    void fire_signal_changed();

    virtual void cut_clipboard() override;

    virtual void copy_clipboard() override;

    virtual void paste_clipboard() override;

    virtual void set_alignment(TxtAlign eXAlign) override;

    virtual ~SalInstanceEntry() override;
};

class SalInstanceSpinButton : public SalInstanceEntry, public virtual weld::SpinButton
{
private:
    VclPtr<FormattedField> m_xButton;
    Formatter& m_rFormatter;

    DECL_LINK(UpDownHdl, SpinField&, void);
    DECL_LINK(LoseFocusHdl, Control&, void);
    DECL_LINK(OutputHdl, LinkParamNone*, bool);
    DECL_LINK(InputHdl, sal_Int64*, TriState);
    DECL_LINK(ActivateHdl, Edit&, bool);

    double toField(int nValue) const;

    int fromField(double fValue) const;

public:
    SalInstanceSpinButton(FormattedField* pButton, SalInstanceBuilder* pBuilder,
                          bool bTakeOwnership);

    virtual int get_value() const override;

    virtual void set_value(int value) override;

    virtual void set_range(int min, int max) override;

    virtual void get_range(int& min, int& max) const override;

    virtual void set_increments(int step, int /*page*/) override;

    virtual void get_increments(int& step, int& page) const override;

    virtual void set_digits(unsigned int digits) override;

    // SpinButton may be comprised of multiple subwidgets, consider the lot as
    // one thing for focus
    virtual bool has_focus() const override;

    //off by default for direct SpinButtons, MetricSpinButton enables it
    void SetUseThousandSep();

    virtual unsigned int get_digits() const override;

    virtual ~SalInstanceSpinButton() override;
};

//ComboBox and ListBox have similar apis, ComboBoxes in LibreOffice have an edit box and ListBoxes
//don't. This distinction isn't there in Gtk. Use a template to sort this problem out.
template <class vcl_type>
class SalInstanceComboBox : public SalInstanceContainer, public virtual weld::ComboBox
{
protected:
    // owner for ListBox/ComboBox UserData
    std::vector<std::shared_ptr<OUString>> m_aUserData;
    VclPtr<vcl_type> m_xComboBox;
    ScopedVclPtr<MenuButton> m_xMenuButton;
    OUString m_sMenuButtonRow;

public:
    SalInstanceComboBox(vcl_type* pComboBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership)
        : SalInstanceContainer(pComboBox, pBuilder, bTakeOwnership)
        , m_xComboBox(pComboBox)
    {
    }

    virtual int get_active() const override
    {
        const sal_Int32 nRet = m_xComboBox->GetSelectedEntryPos();
        if (nRet == LISTBOX_ENTRY_NOTFOUND)
            return -1;
        return nRet;
    }

    const OUString* getEntryData(int index) const
    {
        return static_cast<const OUString*>(m_xComboBox->GetEntryData(index));
    }

    // ComboBoxes are comprised of multiple subwidgets, consider the lot as
    // one thing for focus
    virtual bool has_focus() const override
    {
        return m_xWidget->HasChildPathFocus()
               || (m_xMenuButton && (m_xMenuButton->HasFocus() || m_xMenuButton->InPopupMode()));
    }

    virtual OUString get_active_id() const override
    {
        sal_Int32 nPos = m_xComboBox->GetSelectedEntryPos();
        const OUString* pRet;
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            pRet = getEntryData(m_xComboBox->GetSelectedEntryPos());
        else
            pRet = nullptr;
        if (!pRet)
            return OUString();
        return *pRet;
    }

    virtual void set_active_id(const OUString& rStr) override
    {
        for (int i = 0; i < get_count(); ++i)
        {
            const OUString* pId = getEntryData(i);
            if (!pId)
                continue;
            if (*pId == rStr)
                m_xComboBox->SelectEntryPos(i);
        }
    }

    virtual void set_active(int pos) override
    {
        if (pos == -1)
        {
            m_xComboBox->SetNoSelection();
            return;
        }
        m_xComboBox->SelectEntryPos(pos);
    }

    virtual OUString get_text(int pos) const override { return m_xComboBox->GetEntry(pos); }

    virtual OUString get_id(int pos) const override
    {
        const OUString* pRet = getEntryData(pos);
        if (!pRet)
            return OUString();
        return *pRet;
    }

    virtual void set_id(int row, const OUString& rId) override
    {
        m_aUserData.emplace_back(std::make_unique<OUString>(rId));
        m_xComboBox->SetEntryData(row, m_aUserData.back().get());
    }

    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems,
                               bool bKeepExisting) override
    {
        freeze();
        if (!bKeepExisting)
            clear();
        for (const auto& rItem : rItems)
        {
            insert(-1, rItem.sString, rItem.sId.isEmpty() ? nullptr : &rItem.sId,
                   rItem.sImage.isEmpty() ? nullptr : &rItem.sImage, nullptr);
        }
        thaw();
    }

    virtual int get_count() const override { return m_xComboBox->GetEntryCount(); }

    virtual int find_text(const OUString& rStr) const override
    {
        const sal_Int32 nRet = m_xComboBox->GetEntryPos(rStr);
        if (nRet == LISTBOX_ENTRY_NOTFOUND)
            return -1;
        return nRet;
    }

    virtual int find_id(const OUString& rStr) const override
    {
        for (int i = 0; i < get_count(); ++i)
        {
            const OUString* pId = getEntryData(i);
            if (!pId)
                continue;
            if (*pId == rStr)
                return i;
        }
        return -1;
    }

    virtual void clear() override
    {
        m_xComboBox->Clear();
        m_aUserData.clear();
    }

    virtual void make_sorted() override
    {
        m_xComboBox->SetStyle(m_xComboBox->GetStyle() | WB_SORT);
    }

    virtual bool get_popup_shown() const override { return m_xComboBox->IsInDropDown(); }

    virtual void connect_popup_toggled(const Link<ComboBox&, void>& rLink) override
    {
        weld::ComboBox::connect_popup_toggled(rLink);
        ensure_event_listener();
    }

    void call_signal_custom_render(UserDrawEvent* pEvent)
    {
        vcl::RenderContext* pRenderContext = pEvent->GetRenderContext();
        auto nPos = pEvent->GetItemId();
        const tools::Rectangle& rRect = pEvent->GetRect();
        const OUString sId = get_id(nPos);
        signal_custom_render(*pRenderContext, rRect, pEvent->IsSelected(), sId);
        m_xComboBox->DrawEntry(*pEvent); // draw separator

        if (m_xMenuButton && m_xMenuButton->IsVisible() && m_sMenuButtonRow == sId)
        {
            if (m_xMenuButton->GetParent() != pEvent->GetWindow())
                m_xMenuButton->SetParent(pEvent->GetWindow());
            int nButtonWidth = get_menu_button_width();
            m_xMenuButton->SetSizePixel(Size(nButtonWidth, rRect.GetHeight()));
            m_xMenuButton->SetPosPixel(Point(rRect.GetWidth() - nButtonWidth, rRect.getY()));
        }
    }

    VclPtr<VirtualDevice> create_render_virtual_device() const override
    {
        auto xRet = VclPtr<VirtualDevice>::Create();
        xRet->SetBackground(Application::GetSettings().GetStyleSettings().GetFieldColor());
        return xRet;
    }

    virtual void set_item_menu(const OString& rIdent, weld::Menu* pMenu) override
    {
        SalInstanceMenu* pInstanceMenu = dynamic_cast<SalInstanceMenu*>(pMenu);

        PopupMenu* pPopup = pInstanceMenu ? pInstanceMenu->getMenu() : nullptr;

        if (!m_xMenuButton)
            m_xMenuButton
                = VclPtr<MenuButton>::Create(m_xComboBox, WB_FLATBUTTON | WB_NOPOINTERFOCUS);

        m_xMenuButton->SetPopupMenu(pPopup);
        m_xMenuButton->Show(pPopup != nullptr);
        m_sMenuButtonRow = OUString::fromUtf8(rIdent);
    }

    int get_menu_button_width() const override
    {
        const int nButtonWidth = 20;
        return nButtonWidth;
    }

    void CallHandleEventListener(VclWindowEvent& rEvent)
    {
        if (rEvent.GetId() == VclEventId::DropdownPreOpen
            || rEvent.GetId() == VclEventId::DropdownClose)
        {
            signal_popup_toggled();
            return;
        }
        SalInstanceContainer::HandleEventListener(rEvent);
    }
};

class SalInstanceComboBoxWithoutEdit : public SalInstanceComboBox<ListBox>
{
private:
    DECL_LINK(SelectHdl, ListBox&, void);

public:
    SalInstanceComboBoxWithoutEdit(ListBox* pListBox, SalInstanceBuilder* pBuilder,
                                   bool bTakeOwnership);

    virtual OUString get_active_text() const override;

    virtual void remove(int pos) override;

    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;

    virtual void insert_separator(int pos, const OUString& /*rId*/) override;

    virtual bool has_entry() const override;

    virtual bool changed_by_direct_pick() const override;

    virtual void set_entry_message_type(weld::EntryMessageType /*eType*/) override;

    virtual void set_entry_text(const OUString& /*rText*/) override;

    virtual void select_entry_region(int /*nStartPos*/, int /*nEndPos*/) override;

    virtual bool get_entry_selection_bounds(int& /*rStartPos*/, int& /*rEndPos*/) override;

    virtual void set_entry_width_chars(int /*nChars*/) override;

    virtual void set_entry_max_length(int /*nChars*/) override;

    virtual void set_entry_completion(bool, bool bCaseSensitive = false) override;

    virtual void set_entry_placeholder_text(const OUString&) override;

    virtual void set_entry_editable(bool bEditable) override;

    virtual void cut_entry_clipboard() override;

    virtual void copy_entry_clipboard() override;

    virtual void paste_entry_clipboard() override;

    virtual void set_entry_font(const vcl::Font&) override;

    virtual vcl::Font get_entry_font() override;

    virtual void set_custom_renderer(bool bOn) override;

    virtual int get_max_mru_count() const override;

    virtual void set_max_mru_count(int) override;

    virtual OUString get_mru_entries() const override;

    virtual void set_mru_entries(const OUString&) override;

    virtual void HandleEventListener(VclWindowEvent& rEvent) override;

    virtual ~SalInstanceComboBoxWithoutEdit() override;
};

class SalInstanceComboBoxWithEdit : public SalInstanceComboBox<ComboBox>
{
private:
    DECL_LINK(ChangeHdl, Edit&, void);
    DECL_LINK(EntryActivateHdl, Edit&, bool);
    DECL_LINK(SelectHdl, ::ComboBox&, void);
    DECL_LINK(UserDrawHdl, UserDrawEvent*, void);
    WeldTextFilter m_aTextFilter;
    bool m_bInSelect;

public:
    SalInstanceComboBoxWithEdit(::ComboBox* pComboBox, SalInstanceBuilder* pBuilder,
                                bool bTakeOwnership);

    virtual bool has_entry() const override;

    virtual bool changed_by_direct_pick() const override;

    virtual void set_entry_message_type(weld::EntryMessageType eType) override;

    virtual OUString get_active_text() const override;

    virtual void remove(int pos) override;

    virtual void insert(int pos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;

    virtual void insert_separator(int pos, const OUString& /*rId*/) override;

    virtual void set_entry_text(const OUString& rText) override;

    virtual void set_entry_width_chars(int nChars) override;

    virtual void set_entry_max_length(int nChars) override;

    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive = false) override;

    virtual void set_entry_placeholder_text(const OUString& rText) override;

    virtual void set_entry_editable(bool bEditable) override;

    virtual void cut_entry_clipboard() override;

    virtual void copy_entry_clipboard() override;

    virtual void paste_entry_clipboard() override;

    virtual void select_entry_region(int nStartPos, int nEndPos) override;

    virtual bool get_entry_selection_bounds(int& rStartPos, int& rEndPos) override;

    virtual void set_entry_font(const vcl::Font& rFont) override;

    virtual vcl::Font get_entry_font() override;

    virtual void set_custom_renderer(bool bOn) override;

    virtual int get_max_mru_count() const override;

    virtual void set_max_mru_count(int nCount) override;

    virtual OUString get_mru_entries() const override;

    virtual void set_mru_entries(const OUString& rEntries) override;

    virtual void HandleEventListener(VclWindowEvent& rEvent) override;

    virtual ~SalInstanceComboBoxWithEdit() override;
};

class SalInstanceButton : public SalInstanceContainer, public virtual weld::Button
{
private:
    VclPtr<::Button> m_xButton;
    Link<::Button*, void> const m_aOldClickHdl;

    DECL_LINK(ClickHdl, ::Button*, void);

public:
    SalInstanceButton(::Button* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_label(const OUString& rText) override;

    virtual void set_image(VirtualDevice* pDevice) override;

    virtual void set_image(const css::uno::Reference<css::graphic::XGraphic>& rImage) override;

    virtual void set_from_icon_name(const OUString& rIconName) override;

    virtual void set_label_line_wrap(bool wrap) override;

    virtual OUString get_label() const override;

    virtual ~SalInstanceButton() override;
};

class SalInstanceNotebook : public SalInstanceContainer, public virtual weld::Notebook
{
private:
    VclPtr<TabControl> m_xNotebook;
    mutable std::vector<std::shared_ptr<SalInstanceContainer>> m_aPages;
    std::map<OString, std::pair<VclPtr<TabPage>, VclPtr<VclGrid>>> m_aAddedPages;

    DECL_LINK(DeactivatePageHdl, TabControl*, bool);
    DECL_LINK(ActivatePageHdl, TabControl*, void);

public:
    SalInstanceNotebook(TabControl* pNotebook, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual int get_current_page() const override;

    virtual int get_page_index(const OString& rIdent) const override;

    virtual OString get_page_ident(int nPage) const override;

    virtual OString get_current_page_ident() const override;

    virtual weld::Container* get_page(const OString& rIdent) const override;

    virtual void set_current_page(int nPage) override;

    virtual void set_current_page(const OString& rIdent) override;

    virtual void remove_page(const OString& rIdent) override;

    virtual void insert_page(const OString& rIdent, const OUString& rLabel, int nPos) override;

    virtual int get_n_pages() const override;

    virtual OUString get_tab_label_text(const OString& rIdent) const override;

    virtual void set_tab_label_text(const OString& rIdent, const OUString& rText) override;

    virtual ~SalInstanceNotebook() override;
};

class SalInstanceMessageDialog : public SalInstanceDialog, public virtual weld::MessageDialog
{
protected:
    VclPtr<::MessageDialog> m_xMessageDialog;

public:
    SalInstanceMessageDialog(::MessageDialog* pDialog, SalInstanceBuilder* pBuilder,
                             bool bTakeOwnership);

    virtual void set_primary_text(const OUString& rText) override;

    virtual OUString get_primary_text() const override;

    virtual void set_secondary_text(const OUString& rText) override;

    virtual OUString get_secondary_text() const override;

    virtual weld::Container* weld_message_area() override;
};

class SalInstanceCheckButton : public SalInstanceButton, public virtual weld::CheckButton
{
private:
    VclPtr<CheckBox> m_xCheckButton;

    DECL_LINK(ToggleHdl, CheckBox&, void);

public:
    SalInstanceCheckButton(CheckBox* pButton, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_active(bool active) override;

    virtual bool get_active() const override;

    virtual void set_inconsistent(bool inconsistent) override;

    virtual bool get_inconsistent() const override;

    virtual ~SalInstanceCheckButton() override;
};

class SalInstanceDrawingArea : public SalInstanceWidget, public virtual weld::DrawingArea
{
private:
    VclPtr<VclDrawingArea> m_xDrawingArea;

    typedef std::pair<vcl::RenderContext&, const tools::Rectangle&> target_and_area;
    DECL_LINK(PaintHdl, target_and_area, void);
    DECL_LINK(ResizeHdl, const Size&, void);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_LINK(MouseMoveHdl, const MouseEvent&, bool);
    DECL_LINK(MouseReleaseHdl, const MouseEvent&, bool);
    DECL_LINK(KeyPressHdl, const KeyEvent&, bool);
    DECL_LINK(KeyReleaseHdl, const KeyEvent&, bool);
    DECL_LINK(StyleUpdatedHdl, VclDrawingArea&, void);
    DECL_LINK(CommandHdl, const CommandEvent&, bool);
    DECL_LINK(QueryTooltipHdl, tools::Rectangle&, OUString);
    DECL_LINK(GetSurroundingHdl, OUString&, int);
    DECL_LINK(StartDragHdl, VclDrawingArea*, bool);

    // SalInstanceWidget has a generic listener for all these
    // events, ignore the ones we have specializations for
    // in VclDrawingArea
    virtual void HandleEventListener(VclWindowEvent& rEvent) override;

    virtual void HandleMouseEventListener(VclSimpleEvent& rEvent) override;

    virtual bool HandleKeyEventListener(VclWindowEvent& /*rEvent*/) override;

public:
    SalInstanceDrawingArea(VclDrawingArea* pDrawingArea, SalInstanceBuilder* pBuilder,
                           const a11yref& rAlly, FactoryFunction pUITestFactoryFunction,
                           void* pUserData, bool bTakeOwnership);

    virtual void queue_draw() override;

    virtual void queue_draw_area(int x, int y, int width, int height) override;

    virtual void queue_resize() override;

    virtual void connect_size_allocate(const Link<const Size&, void>& rLink) override;

    virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override;

    virtual void connect_key_release(const Link<const KeyEvent&, bool>& rLink) override;

    virtual void set_cursor(PointerStyle ePointerStyle) override;

    virtual void set_input_context(const InputContext& rInputContext) override;

    virtual void im_context_set_cursor_location(const tools::Rectangle& rCursorRect,
                                                int nExtTextInputWidth) override;

    virtual a11yref get_accessible_parent() override;

    virtual a11yrelationset get_accessible_relation_set() override;

    virtual Point get_accessible_location() override;

    virtual void enable_drag_source(rtl::Reference<TransferDataContainer>& rHelper,
                                    sal_uInt8 eDNDConstants) override;

    virtual ~SalInstanceDrawingArea() override;

    virtual OutputDevice& get_ref_device() override;

    virtual void click(const Point& rPos) override;
};

class SalInstanceToolbar : public SalInstanceWidget, public virtual weld::Toolbar
{
private:
    VclPtr<ToolBox> m_xToolBox;
    std::map<sal_uInt16, VclPtr<vcl::Window>> m_aFloats;
    std::map<sal_uInt16, VclPtr<PopupMenu>> m_aMenus;

    OString m_sStartShowIdent;

    DECL_LINK(ClickHdl, ToolBox*, void);
    DECL_LINK(DropdownClick, ToolBox*, void);
    DECL_LINK(MenuToggleListener, VclWindowEvent&, void);

public:
    SalInstanceToolbar(ToolBox* pToolBox, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_item_sensitive(const OString& rIdent, bool bSensitive) override;

    virtual bool get_item_sensitive(const OString& rIdent) const override;

    virtual void set_item_visible(const OString& rIdent, bool bVisible) override;

    virtual void set_item_help_id(const OString& rIdent, const OString& rHelpId) override;

    virtual bool get_item_visible(const OString& rIdent) const override;

    virtual void set_item_active(const OString& rIdent, bool bActive) override;

    virtual bool get_item_active(const OString& rIdent) const override;

    void set_menu_item_active(const OString& rIdent, bool bActive) override;

    bool get_menu_item_active(const OString& rIdent) const override;

    virtual void set_item_popover(const OString& rIdent, weld::Widget* pPopover) override;

    virtual void set_item_menu(const OString& rIdent, weld::Menu* pMenu) override;

    virtual void insert_separator(int pos, const OUString& /*rId*/) override;

    virtual int get_n_items() const override;

    virtual OString get_item_ident(int nIndex) const override;

    virtual void set_item_ident(int nIndex, const OString& rIdent) override;

    virtual void set_item_label(int nIndex, const OUString& rLabel) override;

    virtual OUString get_item_label(const OString& rIdent) const override;

    virtual void set_item_label(const OString& rIdent, const OUString& rLabel) override;

    virtual void set_item_icon_name(const OString& rIdent, const OUString& rIconName) override;

    virtual void set_item_image(const OString& rIdent,
                                const css::uno::Reference<css::graphic::XGraphic>& rIcon) override;

    virtual void set_item_image(const OString& rIdent, VirtualDevice* pDevice) override;

    virtual void set_item_image(int nIndex,
                                const css::uno::Reference<css::graphic::XGraphic>& rIcon) override;

    virtual void set_item_tooltip_text(int nIndex, const OUString& rTip) override;

    virtual void set_item_tooltip_text(const OString& rIdent, const OUString& rTip) override;

    virtual OUString get_item_tooltip_text(const OString& rIdent) const override;

    virtual vcl::ImageType get_icon_size() const override;

    virtual void set_icon_size(vcl::ImageType eType) override;

    virtual sal_uInt16 get_modifier_state() const override;

    int get_drop_index(const Point& rPoint) const override;

    virtual ~SalInstanceToolbar() override;
};

class SalInstanceTextView : public SalInstanceContainer, public virtual weld::TextView
{
private:
    VclPtr<VclMultiLineEdit> m_xTextView;
    Link<ScrollBar*, void> m_aOrigVScrollHdl;

    DECL_LINK(ChangeHdl, Edit&, void);
    DECL_LINK(VscrollHdl, ScrollBar*, void);
    DECL_LINK(CursorListener, VclWindowEvent&, void);

public:
    SalInstanceTextView(VclMultiLineEdit* pTextView, SalInstanceBuilder* pBuilder,
                        bool bTakeOwnership);

    virtual void set_text(const OUString& rText) override;

    virtual void replace_selection(const OUString& rText) override;

    virtual OUString get_text() const override;

    bool get_selection_bounds(int& rStartPos, int& rEndPos) override;

    virtual void select_region(int nStartPos, int nEndPos) override;

    virtual void set_editable(bool bEditable) override;
    virtual bool get_editable() const override;
    virtual void set_max_length(int nChars) override;

    virtual void set_monospace(bool bMonospace) override;

    virtual void set_font_color(const Color& rColor) override;

    virtual void connect_cursor_position(const Link<TextView&, void>& rLink) override;

    virtual bool can_move_cursor_with_up() const override;

    virtual bool can_move_cursor_with_down() const override;

    virtual void cut_clipboard() override;

    virtual void copy_clipboard() override;

    virtual void paste_clipboard() override;

    virtual void set_alignment(TxtAlign eXAlign) override;

    virtual int vadjustment_get_value() const override;

    virtual void vadjustment_set_value(int value) override;

    virtual int vadjustment_get_upper() const override;

    virtual int vadjustment_get_lower() const override;

    virtual int vadjustment_get_page_size() const override;

    virtual bool has_focus() const override;

    virtual ~SalInstanceTextView() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
