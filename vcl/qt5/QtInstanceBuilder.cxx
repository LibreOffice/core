/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceBuilder.hxx>

#include <unordered_set>

#include <QtBuilder.hxx>
#include <QtInstanceMessageDialog.hxx>

QtInstanceBuilder::QtInstanceBuilder(QWidget* pParent, std::u16string_view sUIRoot,
                                     const OUString& rUIFile)
    : m_xBuilder(std::make_unique<QtBuilder>(pParent, sUIRoot, rUIFile))
{
}

QtInstanceBuilder::~QtInstanceBuilder() {}

bool QtInstanceBuilder::IsUIFileSupported(const OUString& rUIFile)
{
    // set of supported UI files
    //
    // The idea is to implement functionality needed for a specific UI file/dialog
    // in QtInstanceBuilder, then add it to the set of supported UI files here.
    // This allows looking at one .ui file at a time and only having to implement
    // what is relevant for that particular one, without having to implement the full
    // weld API at once.
    static std::unordered_set<OUString> aSupportedUIFiles = {
        u"modules/swriter/ui/inforeadonlydialog.ui"_ustr,
        u"sfx/ui/licensedialog.ui"_ustr,
        u"sfx/ui/querysavedialog.ui"_ustr,
    };

    return aSupportedUIFiles.contains(rUIFile);
}

std::unique_ptr<weld::MessageDialog> QtInstanceBuilder::weld_message_dialog(const OUString& id)
{
    QMessageBox* pMessageBox = m_xBuilder->get<QMessageBox>(id);
    std::unique_ptr<weld::MessageDialog> xRet(
        pMessageBox ? std::make_unique<QtInstanceMessageDialog>(pMessageBox) : nullptr);
    return xRet;
}

std::unique_ptr<weld::Dialog> QtInstanceBuilder::weld_dialog(const OUString& rId)
{
    QDialog* pDialog = m_xBuilder->get<QDialog>(rId);
    std::unique_ptr<weld::Dialog> xRet(pDialog ? std::make_unique<QtInstanceDialog>(pDialog)
                                               : nullptr);
    return xRet;
}

std::unique_ptr<weld::Assistant> QtInstanceBuilder::weld_assistant(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Window> QtInstanceBuilder::create_screenshot_window()
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Widget> QtInstanceBuilder::weld_widget(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Container> QtInstanceBuilder::weld_container(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Box> QtInstanceBuilder::weld_box(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Paned> QtInstanceBuilder::weld_paned(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Frame> QtInstanceBuilder::weld_frame(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::ScrolledWindow> QtInstanceBuilder::weld_scrolled_window(const OUString&, bool)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Notebook> QtInstanceBuilder::weld_notebook(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Button> QtInstanceBuilder::weld_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::MenuButton> QtInstanceBuilder::weld_menu_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::MenuToggleButton> QtInstanceBuilder::weld_menu_toggle_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::LinkButton> QtInstanceBuilder::weld_link_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::ToggleButton> QtInstanceBuilder::weld_toggle_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::RadioButton> QtInstanceBuilder::weld_radio_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::CheckButton> QtInstanceBuilder::weld_check_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Scale> QtInstanceBuilder::weld_scale(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::ProgressBar> QtInstanceBuilder::weld_progress_bar(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::LevelBar> QtInstanceBuilder::weld_level_bar(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Spinner> QtInstanceBuilder::weld_spinner(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Image> QtInstanceBuilder::weld_image(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Calendar> QtInstanceBuilder::weld_calendar(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Entry> QtInstanceBuilder::weld_entry(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::SpinButton> QtInstanceBuilder::weld_spin_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::MetricSpinButton> QtInstanceBuilder::weld_metric_spin_button(const OUString&,
                                                                                   FieldUnit)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::FormattedSpinButton>
QtInstanceBuilder::weld_formatted_spin_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::ComboBox> QtInstanceBuilder::weld_combo_box(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::EntryTreeView>
QtInstanceBuilder::weld_entry_tree_view(const OUString&, const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::TreeView> QtInstanceBuilder::weld_tree_view(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::IconView> QtInstanceBuilder::weld_icon_view(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Label> QtInstanceBuilder::weld_label(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::TextView> QtInstanceBuilder::weld_text_view(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Expander> QtInstanceBuilder::weld_expander(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::DrawingArea>
QtInstanceBuilder::weld_drawing_area(const OUString&, const a11yref&, FactoryFunction, void*)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Menu> QtInstanceBuilder::weld_menu(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Popover> QtInstanceBuilder::weld_popover(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Toolbar> QtInstanceBuilder::weld_toolbar(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Scrollbar> QtInstanceBuilder::weld_scrollbar(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::SizeGroup> QtInstanceBuilder::create_size_group()
{
    assert(false && "Not implemented yet");
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
