/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#include <QtInstanceBox.hxx>
#include <QtInstanceCheckButton.hxx>
#include <QtInstanceComboBox.hxx>
#include <QtInstanceDrawingArea.hxx>
#include <QtInstanceEntry.hxx>
#include <QtInstanceExpander.hxx>
#include <QtInstanceFormattedSpinButton.hxx>
#include <QtInstanceFrame.hxx>
#include <QtInstanceGrid.hxx>
#include <QtInstanceImage.hxx>
#include <QtInstanceLabel.hxx>
#include <QtInstanceLevelBar.hxx>
#include <QtInstanceLinkButton.hxx>
#include <QtInstanceIconView.hxx>
#include <QtInstanceMenu.hxx>
#include <QtInstanceMenuButton.hxx>
#include <QtInstanceMessageDialog.hxx>
#include <QtInstanceNotebook.hxx>
#include <QtInstancePopover.hxx>
#include <QtInstanceProgressBar.hxx>
#include <QtInstanceRadioButton.hxx>
#include <QtInstanceScale.hxx>
#include <QtInstanceScrolledWindow.hxx>
#include <QtInstanceSpinButton.hxx>
#include <QtInstanceTextView.hxx>
#include <QtInstanceToggleButton.hxx>
#include <QtInstanceToolbar.hxx>
#include <QtInstanceTreeView.hxx>

#include <QtWidgets/QToolButton>

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
        u"cui/ui/aboutdialog.ui"_ustr,
        u"cui/ui/insertrowcolumn.ui"_ustr,
        u"cui/ui/javastartparametersdialog.ui"_ustr,
        u"cui/ui/namedialog.ui"_ustr,
        u"cui/ui/numberdialog.ui"_ustr,
        u"cui/ui/objecttitledescdialog.ui"_ustr,
        u"cui/ui/optlingupage.ui"_ustr,
        u"cui/ui/optnewdictionarydialog.ui"_ustr,
        u"cui/ui/password.ui"_ustr,
        u"cui/ui/pastespecial.ui"_ustr,
        u"cui/ui/querydeletedictionarydialog.ui"_ustr,
        u"cui/ui/qrcodegen.ui"_ustr,
        u"cui/ui/querydialog.ui"_ustr,
        u"cui/ui/securityoptionsdialog.ui"_ustr,
        u"cui/ui/spelloptionsdialog.ui"_ustr,
        u"cui/ui/splitcellsdialog.ui"_ustr,
        u"cui/ui/tipofthedaydialog.ui"_ustr,
        u"cui/ui/toolbarmodedialog.ui"_ustr,
        u"cui/ui/zoomdialog.ui"_ustr,
        u"dbaccess/ui/savedialog.ui"_ustr,
        u"dbaccess/ui/tabledesignsavemodifieddialog.ui"_ustr,
        u"modules/scalc/ui/inputstringdialog.ui"_ustr,
        u"modules/scalc/ui/insertcells.ui"_ustr,
        u"modules/scalc/ui/movecopysheet.ui"_ustr,
        u"modules/scalc/ui/optdlg.ui"_ustr,
        u"modules/scalc/ui/selectsource.ui"_ustr,
        u"modules/scalc/ui/showsheetdialog.ui"_ustr,
        u"modules/schart/ui/insertaxisdlg.ui"_ustr,
        u"modules/simpress/ui/presentationdialog.ui"_ustr,
        u"modules/smath/ui/alignmentdialog.ui"_ustr,
        u"modules/smath/ui/fontsizedialog.ui"_ustr,
        u"modules/smath/ui/savedefaultsdialog.ui"_ustr,
        u"modules/smath/ui/smathsettings.ui"_ustr,
        u"modules/swriter/ui/authenticationsettingsdialog.ui"_ustr,
        u"modules/swriter/ui/columnwidth.ui"_ustr,
        u"modules/swriter/ui/endnotepage.ui"_ustr,
        u"modules/swriter/ui/footendnotedialog.ui"_ustr,
        u"modules/swriter/ui/footnotepage.ui"_ustr,
        u"modules/swriter/ui/inforeadonlydialog.ui"_ustr,
        u"modules/swriter/ui/insertbreak.ui"_ustr,
        u"modules/swriter/ui/inserttable.ui"_ustr,
        u"modules/swriter/ui/printoptionspage.ui"_ustr,
        u"modules/swriter/ui/renameobjectdialog.ui"_ustr,
        u"modules/swriter/ui/rowheight.ui"_ustr,
        u"modules/swriter/ui/splittable.ui"_ustr,
        u"modules/swriter/ui/wordcount.ui"_ustr,
        u"sfx/ui/helpmanual.ui"_ustr,
        u"sfx/ui/inputdialog.ui"_ustr,
        u"sfx/ui/licensedialog.ui"_ustr,
        u"sfx/ui/password.ui"_ustr,
        u"sfx/ui/printeroptionsdialog.ui"_ustr,
        u"sfx/ui/querysavedialog.ui"_ustr,
        u"sfx/ui/safemodequerydialog.ui"_ustr,
        u"svt/ui/printersetupdialog.ui"_ustr,
        u"svt/ui/restartdialog.ui"_ustr,
        u"svx/ui/compressgraphicdialog.ui"_ustr,
        u"svx/ui/fontworkgallerydialog.ui"_ustr,
        u"svx/ui/deletefooterdialog.ui"_ustr,
        u"svx/ui/deleteheaderdialog.ui"_ustr,
        u"svx/ui/gotopagedialog.ui"_ustr,
        u"svx/ui/safemodedialog.ui"_ustr,
        u"vcl/ui/openlockedquerybox.ui"_ustr,
        u"vcl/ui/printerdevicepage.ui"_ustr,
        u"vcl/ui/printerpaperpage.ui"_ustr,
        u"vcl/ui/printerpropertiesdialog.ui"_ustr,
        u"vcl/ui/printprogressdialog.ui"_ustr,
        u"writerperfect/ui/exportepub.ui"_ustr,
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

std::unique_ptr<weld::Widget> QtInstanceBuilder::weld_widget(const OUString& rId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
    std::unique_ptr<weld::Widget> xRet(pWidget ? std::make_unique<QtInstanceWidget>(pWidget)
                                               : nullptr);
    return xRet;
}

std::unique_ptr<weld::Container> QtInstanceBuilder::weld_container(const OUString& rId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
    if (!pWidget)
        return nullptr;

    assert(pWidget->layout() && "no layout");
    return std::make_unique<QtInstanceContainer>(pWidget);
}

std::unique_ptr<weld::Box> QtInstanceBuilder::weld_box(const OUString& rId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
    if (!pWidget)
        return nullptr;

    assert(qobject_cast<QBoxLayout*>(pWidget->layout()) && "widget doesn't have a box layout");
    return std::make_unique<QtInstanceBox>(pWidget);
}

std::unique_ptr<weld::Grid> QtInstanceBuilder::weld_grid(const OUString& rId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
    if (!pWidget)
        return nullptr;

    assert(qobject_cast<QGridLayout*>(pWidget->layout()) && "no grid layout");
    return std::make_unique<QtInstanceGrid>(pWidget);
}

std::unique_ptr<weld::Paned> QtInstanceBuilder::weld_paned(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Frame> QtInstanceBuilder::weld_frame(const OUString& rId)
{
    QGroupBox* pGroupBox = m_xBuilder->get<QGroupBox>(rId);
    std::unique_ptr<weld::Frame> xRet(pGroupBox ? std::make_unique<QtInstanceFrame>(pGroupBox)
                                                : nullptr);
    return xRet;
}

std::unique_ptr<weld::ScrolledWindow> QtInstanceBuilder::weld_scrolled_window(const OUString& rId,
                                                                              bool)
{
    QScrollArea* pScrollArea = m_xBuilder->get<QScrollArea>(rId);
    std::unique_ptr<weld::ScrolledWindow> xRet(
        pScrollArea ? std::make_unique<QtInstanceScrolledWindow>(pScrollArea) : nullptr);
    return xRet;
}

std::unique_ptr<weld::Notebook> QtInstanceBuilder::weld_notebook(const OUString& rId)
{
    QTabWidget* pTabWidget = m_xBuilder->get<QTabWidget>(rId);
    std::unique_ptr<weld::Notebook> xRet(
        pTabWidget ? std::make_unique<QtInstanceNotebook>(pTabWidget) : nullptr);
    return xRet;
}

std::unique_ptr<weld::Button> QtInstanceBuilder::weld_button(const OUString& rId)
{
    QPushButton* pButton = m_xBuilder->get<QPushButton>(rId);
    std::unique_ptr<weld::Button> xRet(pButton ? std::make_unique<QtInstanceButton>(pButton)
                                               : nullptr);
    return xRet;
}

std::unique_ptr<weld::MenuButton> QtInstanceBuilder::weld_menu_button(const OUString& rId)
{
    QToolButton* pButton = m_xBuilder->get<QToolButton>(rId);
    std::unique_ptr<weld::MenuButton> xRet(pButton ? std::make_unique<QtInstanceMenuButton>(pButton)
                                                   : nullptr);
    return xRet;
}

std::unique_ptr<weld::MenuToggleButton> QtInstanceBuilder::weld_menu_toggle_button(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::LinkButton> QtInstanceBuilder::weld_link_button(const OUString& rId)
{
    QtHyperlinkLabel* pLabel = m_xBuilder->get<QtHyperlinkLabel>(rId);
    std::unique_ptr<weld::LinkButton> xRet(pLabel ? std::make_unique<QtInstanceLinkButton>(pLabel)
                                                  : nullptr);
    return xRet;
}

std::unique_ptr<weld::ToggleButton> QtInstanceBuilder::weld_toggle_button(const OUString& rId)
{
    QAbstractButton* pButton = m_xBuilder->get<QAbstractButton>(rId);
    std::unique_ptr<weld::ToggleButton> xRet(
        pButton ? std::make_unique<QtInstanceToggleButton>(pButton) : nullptr);
    return xRet;
}

std::unique_ptr<weld::RadioButton> QtInstanceBuilder::weld_radio_button(const OUString& rId)
{
    QRadioButton* pRadioButton = m_xBuilder->get<QRadioButton>(rId);
    std::unique_ptr<weld::RadioButton> xRet(
        pRadioButton ? std::make_unique<QtInstanceRadioButton>(pRadioButton) : nullptr);
    return xRet;
}

std::unique_ptr<weld::CheckButton> QtInstanceBuilder::weld_check_button(const OUString& rId)
{
    QCheckBox* pCheckBox = m_xBuilder->get<QCheckBox>(rId);
    std::unique_ptr<weld::CheckButton> xRet(
        pCheckBox ? std::make_unique<QtInstanceCheckButton>(pCheckBox) : nullptr);
    return xRet;
}

std::unique_ptr<weld::Scale> QtInstanceBuilder::weld_scale(const OUString& rId)
{
    QSlider* pSlider = m_xBuilder->get<QSlider>(rId);
    std::unique_ptr<weld::Scale> xRet(pSlider ? std::make_unique<QtInstanceScale>(pSlider)
                                              : nullptr);
    return xRet;
}

std::unique_ptr<weld::ProgressBar> QtInstanceBuilder::weld_progress_bar(const OUString& rId)
{
    QProgressBar* pProgressBar = m_xBuilder->get<QProgressBar>(rId);
    std::unique_ptr<weld::ProgressBar> xRet(
        pProgressBar ? std::make_unique<QtInstanceProgressBar>(pProgressBar) : nullptr);
    return xRet;
}

std::unique_ptr<weld::LevelBar> QtInstanceBuilder::weld_level_bar(const OUString& rId)
{
    QProgressBar* pProgressBar = m_xBuilder->get<QProgressBar>(rId);
    std::unique_ptr<weld::LevelBar> xRet(
        pProgressBar ? std::make_unique<QtInstanceLevelBar>(pProgressBar) : nullptr);
    return xRet;
}

std::unique_ptr<weld::Spinner> QtInstanceBuilder::weld_spinner(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Image> QtInstanceBuilder::weld_image(const OUString& rId)
{
    QLabel* pLabel = m_xBuilder->get<QLabel>(rId);
    std::unique_ptr<weld::Image> xRet(pLabel ? std::make_unique<QtInstanceImage>(pLabel) : nullptr);
    return xRet;
}

std::unique_ptr<weld::Calendar> QtInstanceBuilder::weld_calendar(const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::Entry> QtInstanceBuilder::weld_entry(const OUString& rId)
{
    QLineEdit* pLineEdit = m_xBuilder->get<QLineEdit>(rId);
    std::unique_ptr<weld::Entry> xRet(pLineEdit ? std::make_unique<QtInstanceEntry>(pLineEdit)
                                                : nullptr);
    return xRet;
}

std::unique_ptr<weld::SpinButton> QtInstanceBuilder::weld_spin_button(const OUString& rId)
{
    QtDoubleSpinBox* pSpinBox = m_xBuilder->get<QtDoubleSpinBox>(rId);
    std::unique_ptr<weld::SpinButton> xRet(
        pSpinBox ? std::make_unique<QtInstanceSpinButton>(pSpinBox) : nullptr);
    return xRet;
}

std::unique_ptr<weld::MetricSpinButton>
QtInstanceBuilder::weld_metric_spin_button(const OUString& rId, FieldUnit eUnit)
{
    return std::make_unique<weld::MetricSpinButton>(weld_spin_button(rId), eUnit);
}

std::unique_ptr<weld::FormattedSpinButton>
QtInstanceBuilder::weld_formatted_spin_button(const OUString& rId)
{
    QtDoubleSpinBox* pSpinBox = m_xBuilder->get<QtDoubleSpinBox>(rId);
    std::unique_ptr<weld::FormattedSpinButton> xRet(
        pSpinBox ? std::make_unique<QtInstanceFormattedSpinButton>(pSpinBox) : nullptr);
    return xRet;
}

std::unique_ptr<weld::ComboBox> QtInstanceBuilder::weld_combo_box(const OUString& rId)
{
    QComboBox* pComboBox = m_xBuilder->get<QComboBox>(rId);
    std::unique_ptr<weld::ComboBox> xRet(pComboBox ? std::make_unique<QtInstanceComboBox>(pComboBox)
                                                   : nullptr);
    return xRet;
}

std::unique_ptr<weld::EntryTreeView>
QtInstanceBuilder::weld_entry_tree_view(const OUString&, const OUString&, const OUString&)
{
    assert(false && "Not implemented yet");
    return nullptr;
}

std::unique_ptr<weld::TreeView> QtInstanceBuilder::weld_tree_view(const OUString& rId)
{
    QTreeView* pTreeView = m_xBuilder->get<QTreeView>(rId);
    std::unique_ptr<weld::TreeView> xRet(pTreeView ? std::make_unique<QtInstanceTreeView>(pTreeView)
                                                   : nullptr);
    return xRet;
}

std::unique_ptr<weld::IconView> QtInstanceBuilder::weld_icon_view(const OUString& rId)
{
    QListView* pListView = m_xBuilder->get<QListView>(rId);
    std::unique_ptr<weld::IconView> xRet(pListView ? std::make_unique<QtInstanceIconView>(pListView)
                                                   : nullptr);
    return xRet;
}

std::unique_ptr<weld::Label> QtInstanceBuilder::weld_label(const OUString& rId)
{
    QLabel* pLabel = m_xBuilder->get<QLabel>(rId);
    std::unique_ptr<weld::Label> xRet(pLabel ? std::make_unique<QtInstanceLabel>(pLabel) : nullptr);
    return xRet;
}

std::unique_ptr<weld::TextView> QtInstanceBuilder::weld_text_view(const OUString& rId)
{
    QPlainTextEdit* pTextEdit = m_xBuilder->get<QPlainTextEdit>(rId);
    std::unique_ptr<weld::TextView> xRet(pTextEdit ? std::make_unique<QtInstanceTextView>(pTextEdit)
                                                   : nullptr);
    return xRet;
}

std::unique_ptr<weld::Expander> QtInstanceBuilder::weld_expander(const OUString& rId)
{
    QtExpander* pExpander = m_xBuilder->get<QtExpander>(rId);
    std::unique_ptr<weld::Expander> xRet(pExpander ? std::make_unique<QtInstanceExpander>(pExpander)
                                                   : nullptr);
    return xRet;
}

std::unique_ptr<weld::DrawingArea>
QtInstanceBuilder::weld_drawing_area(const OUString& rId, const a11yref&, FactoryFunction, void*)
{
    QLabel* pLabel = m_xBuilder->get<QLabel>(rId);
    std::unique_ptr<weld::DrawingArea> xRet(pLabel ? std::make_unique<QtInstanceDrawingArea>(pLabel)
                                                   : nullptr);
    return xRet;
}

std::unique_ptr<weld::Menu> QtInstanceBuilder::weld_menu(const OUString& rId)
{
    QMenu* pMenu = m_xBuilder->get<QMenu>(rId);
    std::unique_ptr<weld::Menu> xRet(pMenu ? std::make_unique<QtInstanceMenu>(pMenu) : nullptr);
    return xRet;
}

std::unique_ptr<weld::Popover> QtInstanceBuilder::weld_popover(const OUString& rId)
{
    QWidget* pWidget = m_xBuilder->get<QWidget>(rId);
    std::unique_ptr<weld::Popover> xRet(pWidget ? std::make_unique<QtInstancePopover>(pWidget)
                                                : nullptr);
    return xRet;
}

std::unique_ptr<weld::Toolbar> QtInstanceBuilder::weld_toolbar(const OUString& rId)
{
    QToolBar* pToolBar = m_xBuilder->get<QToolBar>(rId);
    std::unique_ptr<weld::Toolbar> xRet(pToolBar ? std::make_unique<QtInstanceToolbar>(pToolBar)
                                                 : nullptr);
    return xRet;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
