/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtBuilder.hxx"

#include <string_view>

#include <QtWidgets/QWidget>

#include <rtl/ustring.hxx>
#include <vcl/weld.hxx>

class QtInstanceBuilder : public weld::Builder
{
private:
    std::unique_ptr<QtBuilder> m_xBuilder;

public:
    QtInstanceBuilder(QWidget* pParent, std::u16string_view sUIRoot, const OUString& rUIFile);
    ~QtInstanceBuilder();

    static bool IsUIFileSupported(const OUString& rUIFile);

    virtual std::unique_ptr<weld::MessageDialog> weld_message_dialog(const OUString& id) override;
    virtual std::unique_ptr<weld::Dialog> weld_dialog(const OUString& rId) override;
    virtual std::unique_ptr<weld::Assistant> weld_assistant(const OUString&) override;
    virtual std::unique_ptr<weld::Window> create_screenshot_window() override;
    virtual std::unique_ptr<weld::Widget> weld_widget(const OUString& rId) override;
    virtual std::unique_ptr<weld::Container> weld_container(const OUString& rId) override;
    virtual std::unique_ptr<weld::Box> weld_box(const OUString&) override;
    virtual std::unique_ptr<weld::Grid> weld_grid(const OUString& rId) override;
    virtual std::unique_ptr<weld::Paned> weld_paned(const OUString&) override;
    virtual std::unique_ptr<weld::Frame> weld_frame(const OUString& rId) override;
    virtual std::unique_ptr<weld::ScrolledWindow>
    weld_scrolled_window(const OUString& rId, bool bUserManagedScrolling = false) override;
    virtual std::unique_ptr<weld::Notebook> weld_notebook(const OUString& rId) override;
    virtual std::unique_ptr<weld::Button> weld_button(const OUString& rId) override;
    virtual std::unique_ptr<weld::MenuButton> weld_menu_button(const OUString& rId) override;
    virtual std::unique_ptr<weld::MenuToggleButton>
    weld_menu_toggle_button(const OUString&) override;
    virtual std::unique_ptr<weld::LinkButton> weld_link_button(const OUString& rId) override;
    virtual std::unique_ptr<weld::ToggleButton> weld_toggle_button(const OUString& rId) override;
    virtual std::unique_ptr<weld::RadioButton> weld_radio_button(const OUString& rId) override;
    virtual std::unique_ptr<weld::CheckButton> weld_check_button(const OUString& rId) override;
    virtual std::unique_ptr<weld::Scale> weld_scale(const OUString& rId) override;
    virtual std::unique_ptr<weld::ProgressBar> weld_progress_bar(const OUString& rId) override;
    virtual std::unique_ptr<weld::LevelBar> weld_level_bar(const OUString& rId) override;
    virtual std::unique_ptr<weld::Spinner> weld_spinner(const OUString&) override;
    virtual std::unique_ptr<weld::Image> weld_image(const OUString& rId) override;
    virtual std::unique_ptr<weld::Calendar> weld_calendar(const OUString&) override;
    virtual std::unique_ptr<weld::Entry> weld_entry(const OUString& rId) override;
    virtual std::unique_ptr<weld::SpinButton> weld_spin_button(const OUString& rId) override;
    virtual std::unique_ptr<weld::MetricSpinButton> weld_metric_spin_button(const OUString&,
                                                                            FieldUnit) override;
    virtual std::unique_ptr<weld::FormattedSpinButton>
    weld_formatted_spin_button(const OUString&) override;
    virtual std::unique_ptr<weld::ComboBox> weld_combo_box(const OUString& rId) override;
    virtual std::unique_ptr<weld::EntryTreeView>
    weld_entry_tree_view(const OUString&, const OUString&, const OUString&) override;
    virtual std::unique_ptr<weld::TreeView> weld_tree_view(const OUString& rId) override;
    virtual std::unique_ptr<weld::IconView> weld_icon_view(const OUString& rId) override;
    virtual std::unique_ptr<weld::Label> weld_label(const OUString& rId) override;
    virtual std::unique_ptr<weld::TextView> weld_text_view(const OUString& rId) override;
    virtual std::unique_ptr<weld::Expander> weld_expander(const OUString& rId) override;
    virtual std::unique_ptr<weld::DrawingArea>
    weld_drawing_area(const OUString&, const a11yref& rA11yImpl = nullptr,
                      FactoryFunction pUITestFactoryFunction = nullptr,
                      void* pUserData = nullptr) override;
    virtual std::unique_ptr<weld::Menu> weld_menu(const OUString& rId) override;
    virtual std::unique_ptr<weld::Popover> weld_popover(const OUString&) override;
    virtual std::unique_ptr<weld::Toolbar> weld_toolbar(const OUString&) override;
    virtual std::unique_ptr<weld::Scrollbar> weld_scrollbar(const OUString&) override;
    virtual std::unique_ptr<weld::SizeGroup> create_size_group() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
