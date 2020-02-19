#ifndef INCLUDED_VCL_INC_SALVTABLES_HXX
#define INCLUDED_VCL_INC_SALVTABLES_HXX

#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syswin.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/ctrl.hxx>

class SalInstanceBuilder : public weld::Builder
{
private:
    std::unique_ptr<VclBuilder> m_xBuilder;
    VclPtr<vcl::Window> m_aOwnedToplevel;
public:
    SalInstanceBuilder(vcl::Window* pParent, const OUString& rUIRoot, const OUString& rUIFile);

    VclBuilder& get_builder() const;

    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OString &id, bool bTakeOwnership = true) override;

    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OString &id, bool bTakeOwnership = true) override;

    virtual std::unique_ptr<weld::Window> weld_window(const OString &id, bool bTakeOwnership = true) override;

    virtual std::unique_ptr<weld::Widget> weld_widget(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::Container> weld_container(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::Frame> weld_frame(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::ScrolledWindow> weld_scrolled_window(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::Button> weld_button(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::MenuButton> weld_menu_button(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::ToggleButton> weld_toggle_button(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::RadioButton> weld_radio_button(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::Scale> weld_scale(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::ProgressBar> weld_progress_bar(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::Image> weld_image(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::Entry> weld_entry(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::MetricSpinButton> weld_metric_spin_button(const OString& id, FieldUnit eUnit,
                                                                            bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::FormattedSpinButton> weld_formatted_spin_button(const OString& id,
                                                                                  bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::TimeSpinButton> weld_time_spin_button(const OString& id, TimeFieldFormat eFormat,
                                                        bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::EntryTreeView> weld_entry_tree_view(const OString& containerid, const OString& entryid,
                                    const OString& treeviewid, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::TreeView> weld_tree_view(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::Label> weld_label(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::TextView> weld_text_view(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::Expander> weld_expander(const OString &id, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::DrawingArea> weld_drawing_area(const OString &id, const a11yref& rA11yImpl = nullptr,
            FactoryFunction pUITestFactoryFunction = nullptr, void* pUserData = nullptr, bool bTakeOwnership = false) override;

    virtual std::unique_ptr<weld::Menu> weld_menu(const OString &id, bool bTakeOwnership = true) override;

    virtual std::unique_ptr<weld::SizeGroup> create_size_group() override;

    OString get_current_page_help_id();

    virtual ~SalInstanceBuilder() override;
};

class SalInstanceWidget : public virtual weld::Widget
{
protected:
    VclPtr<vcl::Window> m_xWidget;
    SalInstanceBuilder* m_pBuilder;

private:
    DECL_LINK(EventListener, VclWindowEvent&, void);

    const bool m_bTakeOwnership;
    bool m_bEventListener;
    int m_nBlockNotify;

    void ensure_event_listener();

protected:
    virtual void HandleEventListener(VclWindowEvent& rEvent);

public:
    SalInstanceWidget(vcl::Window* pWidget, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual void set_sensitive(bool sensitive) override;

    virtual bool get_sensitive() const override;

    virtual void set_visible(bool visible) override;

    virtual bool get_visible() const override;

    virtual bool is_visible() const override;

    virtual void set_can_focus(bool bCanFocus) override;

    virtual void grab_focus() override;

    virtual bool has_focus() const override;

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

    virtual void set_margin_top(int nMargin) override;

    virtual void set_margin_bottom(int nMargin) override;

    virtual void set_accessible_name(const OUString& rName) override;

    virtual OUString get_accessible_name() const override;

    virtual OUString get_accessible_description() const override;

    virtual void set_tooltip_text(const OUString& rTip) override;

    virtual void connect_focus_in(const Link<Widget&, void>& rLink) override;

    virtual void connect_focus_out(const Link<Widget&, void>& rLink) override;

    virtual void connect_size_allocate(const Link<const Size&, void>& rLink) override;

    virtual void connect_key_press(const Link<const KeyEvent&, bool>& rLink) override;

    virtual void connect_key_release(const Link<const KeyEvent&, bool>& rLink) override;

    virtual bool get_extents_relative_to(Widget& rRelative, int& x, int &y, int& width, int &height) override;

    virtual void grab_add() override;

    virtual bool has_grab() const override;

    virtual void grab_remove() override;

    virtual bool get_direction() const override;

    virtual void set_direction(bool bRTL) override;

    virtual void freeze() override;

    virtual void thaw() override;

    virtual weld::Container* weld_parent() const override;

    virtual ~SalInstanceWidget() override;

    vcl::Window* getWidget();

    void disable_notify_events();

    bool notify_events_disabled();

    void enable_notify_events();

    virtual void help_hierarchy_foreach(const std::function<bool(const OString&)>& func) override;

    virtual OUString strip_mnemonic(const OUString &rLabel) const override;

    SystemWindow* getSystemWindow();
};

class SalInstanceLabel : public SalInstanceWidget, public virtual weld::Label
{
private:
    VclPtr<FixedText> m_xLabel;
public:
    SalInstanceLabel(FixedText* pLabel, SalInstanceBuilder *pBuilder, bool bTakeOwnership);

    virtual void set_label(const OUString& rText) override;

    virtual OUString get_label() const override;

    virtual void set_mnemonic_widget(Widget* pTarget) override;
};

class SalInstanceContainer : public SalInstanceWidget, public virtual weld::Container
{
private:
    VclPtr<vcl::Window> m_xContainer;
public:
    SalInstanceContainer(vcl::Window* pContainer, SalInstanceBuilder* pBuilder, bool bTakeOwnership);
    virtual void move(weld::Widget* pWidget, weld::Container* pNewParent) override;
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

    virtual bool get_resizable() const override;

    virtual bool has_toplevel_focus() const override;

    virtual void set_window_state(const OString& rStr) override;

    virtual OString get_window_state(WindowStateMask nMask) const override;

    virtual SystemEnvData get_system_data() const override;

    virtual ~SalInstanceWindow() override;
};

class SalInstanceDialog : public SalInstanceWindow, public virtual weld::Dialog
{
private:
    VclPtr<::Dialog> m_xDialog;

public:
    SalInstanceDialog(::Dialog* pDialog, SalInstanceBuilder* pBuilder, bool bTakeOwnership);

    virtual bool runAsync(std::shared_ptr<weld::DialogController> aOwner, const std::function<void(sal_Int32)> &rEndDialogFn) override;

    virtual void SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>& rLink) override;

    virtual int run() override;

    virtual void response(int nResponse) override;

    virtual void add_button(const OUString& rText, int nResponse, const OString& rHelpId = OString()) override;

    virtual weld::Button* get_widget_for_response(int nResponse) override;

    virtual void set_default_response(int nResponse) override;

    virtual weld::Container* weld_content_area() override;
};

#endif